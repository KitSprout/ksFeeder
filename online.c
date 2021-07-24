/**
 *      __            ____
 *     / /__ _  __   / __/                      __  
 *    / //_/(_)/ /_ / /  ___   ____ ___  __ __ / /_ 
 *   / ,<  / // __/_\ \ / _ \ / __// _ \/ // // __/ 
 *  /_/|_|/_/ \__//___// .__//_/   \___/\_,_/ \__/  
 *                    /_/   github.com/KitSprout    
 * 
 *  @file    online.c
 *  @author  KitSprout
 *  @brief   mcu firmware:
 *           https://github.com/Hom-Wang/NRF52833-DK_kTerminal
 *           commit f4948c44c10bc662ad909b7e2c704aba21a50ab6
 * 
 */

/* Includes --------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <conio.h>

#include "serial.h"

#include "kslog.h"
#include "kscsv.h"
#include "kserial.h"
#include "ksfeeder.h"

/* Define ----------------------------------------------------------------------------------*/

#define DEFAULT_FREQ                                    (50)

/* Macro -----------------------------------------------------------------------------------*/
/* Typedef ---------------------------------------------------------------------------------*/
/* Variables -------------------------------------------------------------------------------*/
/* Prototypes ------------------------------------------------------------------------------*/

static void ksraw_update(ksraw_t *praw, kserial_packet_t *pk, int dt);
static int ksfeed_serial(ksraw_t *raw, kscsv_t *csv, int frequency);

/* Functions -------------------------------------------------------------------------------*/

/**
 *  @brief  run_online
 */
int run_online(char *comport, int freq)
{
    kscsv_t csv = {0};
    ksraw_t raw = {0};

    raw.raw.mode = ONLINE_MODE;

    s.port = strtoul(&comport[3], NULL, 10);
    // open serial port
    if (s.port == 0)
    {
        // port auto select
        if (serial_get_portlist(&s.portlist) != KS_OK)
        {
            klogd("\n  scan comport error\n");
            return KS_ERROR;
        }
        s.port = s.portlist.port[0];
    }
    if (serial_open(&s) != KS_OK)
    {
        return KS_ERROR;
    }
    klogd("serial open ... COM%d", s.port);
    uint32_t id = 0;
    if (kscmd_check_device(&id) != KS_OK)
    {
        klogd(" ... device not found\n");
        return KS_ERROR;
    }
    klogd(", id=0x%04X\n", id);

    // run
    ksfeed_serial(&raw, &csv, DEFAULT_FREQ);

    // close serial port and free portlist
    serial_close(&s);
    serial_free_portlist(&s.portlist);

    return 0;
}

static char getKey(void)
{
    char ch = 0;
    if (_kbhit())
    {
        ch = _getch();
    }
    return ch;
}

#define GYR_SENSITIVY       (3.14159 / 180.0 / 16.4)    // to rad/s, ±2000dps
#define ACC_SENSITIVY       (9.81 / 8192.0)             // to m/s^2, ±4g
#define MAG_SENSITIVY       (1.0 / 6.6)                 // to uT, ±500uT
static void ksraw_update(ksraw_t *praw, kserial_packet_t *pk, int dt)
{
    // [0]sec [1]msc [2]grx [3]gry [4]grz [5]arx [6]ary [7]arz [8]mrx [9]mry [10]mrz [11]t
    praw->raw.index++;
    praw->raw.dt = dt * 1e-3;
    if (pk->lens > 11)
    {
        praw->raw.t = ((int16_t*)pk->data)[11] / 132.48 + 25;
    }
    for (int i = 0; i < 3; i++)
    {
        praw->raw.gr[i] = ((int16_t*)pk->data)[i + 2] * GYR_SENSITIVY;
        praw->raw.ar[i] = ((int16_t*)pk->data)[i + 5] * ACC_SENSITIVY;
        praw->raw.mr[i] = ((int16_t*)pk->data)[i + 8] * MAG_SENSITIVY;
        praw->raw.g[i]  = praw->raw.gr[i];
        praw->raw.a[i]  = praw->raw.ar[i];
        praw->raw.m[i]  = praw->raw.mr[i];
    }
}

static int ksfeed_serial(ksraw_t *raw, kscsv_t *csv, int frequency)
{
    int loop = KS_TRUE;

    int16_t bytes[KSERIAL_MAX_PACKET_LENS >> 1];
    uint32_t index = 0, count = 0, total = 0;
    kserial_packet_t pk = {
        .param = {0},
        .type = 0,
        .lens = 0,
        .nbyte = 0,
        .data = bytes
    };

    int ts = 0, tn = 0, dt = 0;
    float packetFreq;

    kscmd_set_updaterate(DEFAULT_FREQ);
    kscmd_set_mode(1);

    while (loop)
    {
        kserial_read_continuous(&pk, &index, &count, &total);
        if (count != 0)
        {
            ts = tn;
            tn = ((int16_t*)pk.data)[0] * 1000 + ((int16_t*)pk.data)[1];
            dt = tn - ts;
            packetFreq = 1000.0 / dt;
#if 1
            ksraw_update(raw, &pk, dt);
            ksfeeder(raw->raw.index, &raw->raw);
#endif
#if 0
            klogc("[%6d][%3d][%s][%02X:%02X][%4dHz] ", total, count, KS_TYPE_STRING[pk.type], pk.param[0], pk.param[1], (int32_t)packetFreq);
            for (int i = 0; i < pk.lens; i++)
            {
                switch (pk.type)
                {
                    case KS_I8:     klogc(KS_TYPE_FORMATE[pk.type], ((int8_t*)pk.data)[i]);    break;
                    case KS_U8:     klogc(KS_TYPE_FORMATE[pk.type], ((uint8_t*)pk.data)[i]);   break;
                    case KS_I16:    klogc(KS_TYPE_FORMATE[pk.type], ((int16_t*)pk.data)[i]);   break;
                    case KS_U16:    klogc(KS_TYPE_FORMATE[pk.type], ((uint16_t*)pk.data)[i]);  break;
                    case KS_I32:    klogc(KS_TYPE_FORMATE[pk.type], ((int32_t*)pk.data)[i]);   break;
                    case KS_U32:    klogc(KS_TYPE_FORMATE[pk.type], ((uint32_t*)pk.data)[i]);  break;
                    case KS_I64:    klogc(KS_TYPE_FORMATE[pk.type], ((int64_t*)pk.data)[i]);   break;
                    case KS_U64:    klogc(KS_TYPE_FORMATE[pk.type], ((uint64_t*)pk.data)[i]);  break;
                    case KS_F32:    klogc(KS_TYPE_FORMATE[pk.type], ((float*)pk.data)[i]);     break;
                    case KS_F64:    klogc(KS_TYPE_FORMATE[pk.type], ((double*)pk.data)[i]);    break;
                }
                if (i != (pk.lens - 1))
                {
                    klogc(",");
                }
            }
            klogc("\n");
            klogc(NULL);
#endif

        }
        switch (getKey())
        {
            case 17:    // ctrl + q
            {
                klogd("\n  >> exit\n");
                loop = KS_FALSE;
                break;
            }
            case 19:    // ctrl + S
            {
                klogd("\n  >> save file\n");
                kserial_delay(100);
                break;
            }
        }
    }

    kscmd_set_mode(0);

    return KS_OK;
}

/*************************************** END OF FILE ****************************************/
