/**
 *      __            ____
 *     / /__ _  __   / __/                      __  
 *    / //_/(_)/ /_ / /  ___   ____ ___  __ __ / /_ 
 *   / ,<  / // __/_\ \ / _ \ / __// _ \/ // // __/ 
 *  /_/|_|/_/ \__//___// .__//_/   \___/\_,_/ \__/  
 *                    /_/   github.com/KitSprout    
 * 
 *  @file    offline.c
 *  @author  KitSprout
 *  @brief   
 * 
 */

/* Includes --------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <conio.h>

#include "serial.h"

#include "kslog.h"
#include "kscsv.h"
#include "kserial.h"
#include "ksentry.h"

/* Define ----------------------------------------------------------------------------------*/
/* Macro -----------------------------------------------------------------------------------*/
/* Typedef ---------------------------------------------------------------------------------*/
/* Variables -------------------------------------------------------------------------------*/
/* Prototypes ------------------------------------------------------------------------------*/

static void ksraw_update(int index, ksraw_t *raw, kscsv_t *csv);
static int ksfeed_serial(int frequency);

/* Functions -------------------------------------------------------------------------------*/

/**
 *  @brief  run_online
 */
int run_online(char *comport, int freq)
{
    ksraw_t raw;

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
    klogd("serial open ... COM%d\n", s.port);

    // run
    ksfeed_serial(100);

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

static void ksraw_update(int index, ksraw_t *praw, kscsv_t *pcsv)
{
    praw->raw.index++;

}

static int ksfeed_serial(int frequency)
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

    uint32_t ts = 0, tn = 0;
    float packetFreq;

    // set command

    while (loop)
    {
        kserial_read_continuous(&pk, &index, &count, &total);
        if (count != 0)
        {
            ts = tn;
            tn = ((int16_t*)pk.data)[0] * 1000 + ((int16_t*)pk.data)[1];
            packetFreq = 1000.0 / (tn - ts);

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
    return KS_OK;
}

/*************************************** END OF FILE ****************************************/
