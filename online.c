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
#include <time.h>

#include "serial.h"

#include "kslog.h"
#include "kscsv.h"
#include "kserial.h"
#include "ksfeeder.h"

/* Define ----------------------------------------------------------------------------------*/

#ifndef ONLINE_LOGOUT_RELATE_PATH
#define ONLINE_LOGOUT_RELATE_PATH                       "log/"
#endif

/* Macro -----------------------------------------------------------------------------------*/
/* Typedef ---------------------------------------------------------------------------------*/
/* Variables -------------------------------------------------------------------------------*/

static char *LOG[] = {"sn","dt","gx","gy","gz","ax","ay","az","mx","my","mz"};

/* Prototypes ------------------------------------------------------------------------------*/

static int datetimestring(char *datestring);
static void ksraw_update(ksraw_t *praw, kserial_packet_t *pk, int dt);
static int ksfeed_serial(ksraw_t *raw, kscsv_t *csv, int updaterate, int save);

/* Functions -------------------------------------------------------------------------------*/

/**
 *  @brief  run_online
 */
int run_online(char *comport, int updaterate, int save)
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
    kscmd_set_mode(0);
    kserial_delay(100);
    uint32_t id = 0;
    if (kscmd_check_device(&id) != KS_OK)
    {
        klogd(" ... device not found\n");
        return KS_ERROR;
    }
    klogd(", id=0x%04X\n", id);

    if (save == KS_TRUE)
    {
        // create csv
        char filename[256] = {0};
        char datestring[256] = {0};
        datetimestring(datestring);
        sprintf(filename, "%sLOG_SERIAL_%s.csv", ONLINE_LOGOUT_RELATE_PATH, datestring);
        if (kscsv_create(&csv, filename, NULL, LOG, sizeof(LOG) >> 2) != KS_OK)
        {
            klogd("create csv failed !!!\n");
            return -1;
        }
    }

    // run
    ksfeed_serial(&raw, &csv, updaterate, save);

    // close serial port and free portlist
    serial_close(&s);
    serial_free_portlist(&s.portlist);

    return 0;
}

static int datetimestring(char *datestring)
{
    time_t t = time(NULL);
    struct tm ts = *localtime(&t);
    int lens;
    int datetime[6] = {0};
    datetime[0] = ts.tm_year + 1900;
    datetime[1] = ts.tm_mon + 1;
    datetime[2] = ts.tm_mday;
    datetime[3] = ts.tm_hour;
    datetime[4] = ts.tm_min;
    datetime[5] = ts.tm_sec;
    lens = sprintf(datestring, "%04d%02d%02d_%02d%02d%02d",
        datetime[0], datetime[1], datetime[2], datetime[3], datetime[4], datetime[5]);
    return lens;
}

static char getkey(void)
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

static int ksfeed_serial(ksraw_t *raw, kscsv_t *csv, int updaterate, int save)
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

    kscmd_set_updaterate(updaterate);
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
#if 1
            if ((raw->raw.index > 0) && (save == KS_TRUE))
            {
                // tag: sn,dt,gx,gy,gz,ax,ay,az,mx,my,mz
                kscsv_write(csv, "%d,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f",
                    raw->raw.index, raw->raw.dt,
                    raw->raw.g[0], raw->raw.g[1], raw->raw.g[2],
                    raw->raw.a[0], raw->raw.a[1], raw->raw.a[2],
                    raw->raw.m[0], raw->raw.m[1], raw->raw.m[2]);
            }
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
        switch (getkey())
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
