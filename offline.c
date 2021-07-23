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
#include "ksentry.h"

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

char RelatePath[1024] = {LOGOUT_RELATE_PATH};
char OutputFileTag[1024] = {LOGOUT_FILE_TAG};

/* Prototypes ------------------------------------------------------------------------------*/
/* Functions -------------------------------------------------------------------------------*/

/**
 *  @brief  run_offline
 */
int run_offline(char *filename, int *range)
{
    kscsv_t csv = {0};
    int start = 0, end = 0;

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
        end = range[1];
        if (end > csv.raw.size)
        {
            end = csv.raw.size;
        }
    }
    else
    {
        end = csv.raw.size;
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
    char *LOG[] = {"gx","gy","gz","mx","my","mz","dt","mbx","mby","mbz"};
    if (kscsv_create(&csv, RelatePath, OutputFileTag, LOG, sizeof(LOG) >> 2) != KS_OK)
    {
        klogd("create csv failed !!!\n");
        return -1;
    }

    // feed
    ksraw_t raw;
    // for (uint32_t i = 0; i < csv.raw.size; i++)
    for (uint32_t i = start; i < end; i++)
    {
# if 1
        float sn     = csv.raw.sn[i];
        float ygc[3] = {csv.raw.g[0][i], csv.raw.g[1][i], csv.raw.g[2][i]};     // calibrated gyr
        float yac[3] = {csv.raw.a[0][i], csv.raw.a[1][i], csv.raw.a[2][i]};     // calibrated acc
        float ymc[3] = {csv.raw.m[0][i], csv.raw.m[1][i], csv.raw.m[2][i]};     // calibrated mag
        float ymb[3] = {csv.raw.mb[0][i], csv.raw.mb[1][i], csv.raw.mb[2][i]};  // mag hardiron
        float ym[3]  = {ymc[0] + ymb[0], ymc[1] + ymb[1], ymc[2] + ymb[2]};     // uncalibrated mag
        float dt     = csv.raw.dt[i] * 1e-9;                                    // dt in second
        // process
        // ksupdate(i, &raw);
        ksentry(i, &raw);
#endif
#if 1
        // tag: sn,gx,gy,gz,mx,my,mz,mbx,mby,mbz,dt
        kscsv_write(&csv, "%.0f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f",
            sn,
            ygc[0], ygc[1], ygc[2],
            ymc[0], ymc[1], ymc[2],
            ymb[0], ymb[1], ymb[2],
            dt);
#endif
    }

    // close csv
    return kscsv_close(&csv);
}

/*************************************** END OF FILE ****************************************/
