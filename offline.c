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
#include <string.h>

#include "kslog.h"
#include "kscsv.h"
#include "ksfeeder.h"

/* Define ----------------------------------------------------------------------------------*/

#ifndef LOGOUT_RELATE_PATH
#define LOGOUT_RELATE_PATH     "output/"
#endif

#ifndef LOGOUT_FILE_TAG
#define LOGOUT_FILE_TAG         "_output"
#endif

/* Macro -----------------------------------------------------------------------------------*/
/* Typedef ---------------------------------------------------------------------------------*/
/* Variables -------------------------------------------------------------------------------*/

static char RelatePath[1024] = {LOGOUT_RELATE_PATH};
static char OutputFileTag[1024] = {LOGOUT_FILE_TAG};

/* Prototypes ------------------------------------------------------------------------------*/

static void ksraw_update(int index, ksraw_t *raw, kscsv_t *csv);
static int ksfeed_csv(ksraw_t *raw, kscsv_t *csv, int start, int stop);

/* Functions -------------------------------------------------------------------------------*/

/**
 *  @brief  run_offline
 */
int run_offline(char *filename, int *range)
{
    kscsv_t csv = {0};
    ksraw_t raw = {0};

    int start = 0, stop = 0;

    raw.raw.mode = OFFLINE_MODE;

    // read csv
    if (kscsv_open(&csv, filename) != KS_OK)
    {
        klogd("\nopen csv failed !!!\n");
        return -1;
    }
#if 0
    kscsv_info(&csv);
#endif
    kscsv_read(&csv, -1);

    if (range != NULL)
    {
        start = range[0] - 1;
        stop = range[1];
        if (stop > csv.raw.size)
        {
            stop = csv.raw.size;
        }
    }
    else
    {
        stop = csv.raw.size;
    }
    // if (relatepath != NULL)
    // {
    //     strcpy(RelatePath, relatepath);
    // }
    // if (filetag != NULL)
    // {
    //     strcpy(OutputFileTag, filetag);
    // }
    // create csv
    char *LOG[] = {"sn","dt","gx","gy","gz","mx","my","mz","mbx","mby","mbz"};
    if (kscsv_create(&csv, RelatePath, OutputFileTag, LOG, sizeof(LOG) >> 2) != KS_OK)
    {
        klogd("create csv failed !!!\n");
        return -1;
    }

    // feed
    ksfeed_csv(&raw, &csv, start, stop);

    // close csv
    return kscsv_close(&csv);
}

static void ksraw_update(int index, ksraw_t *praw, kscsv_t *pcsv)
{
    praw->raw.index++;
    praw->raw.t = 25.0f;
    for (int i = 0; i < pcsv->tagcnt; i++)
    {
        int tag = pcsv->tags[i];
        if ((tag > KSCSV_IDX_TS) && (tag < KSCSV_IDX_UNKNOWN))
        {
            if (tag != KSCSV_IDX_DT)
            {
                praw->mem[tag] = (float)((double*)pcsv->mem[tag])[index];
            }
            else
            {
                praw->mem[tag] = (float)((double*)pcsv->mem[tag])[index] * 1e-9;
            }
        }
    }
}

static int ksfeed_csv(ksraw_t *raw, kscsv_t *csv, int start, int stop)
{
    // for (int i = 0; i < csv.raw.size; i++)
    for (int i = start; i < stop; i++)
    {
# if 1
        // process
        ksraw_update(i, raw, csv);
        ksfeeder(i + 1, &raw->raw);
#endif
#if 1
        // tag: sn,dt,gx,gy,gz,mx,my,mz,mbx,mby,mbz
        kscsv_write(csv, "%.0f,%.0f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f",
            csv->raw.sn[i], csv->raw.dt[i],
            csv->raw.g[0][i], csv->raw.g[1][i], csv->raw.g[2][i],
            csv->raw.m[0][i], csv->raw.m[1][i], csv->raw.m[2][i],
            csv->raw.mb[0][i], csv->raw.mb[1][i], csv->raw.mb[2][i]);
#endif
    }
    return KS_OK;
}

/*************************************** END OF FILE ****************************************/
