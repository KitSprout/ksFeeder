/**
 *      __            ____
 *     / /__ _  __   / __/                      __  
 *    / //_/(_)/ /_ / /  ___   ____ ___  __ __ / /_ 
 *   / ,<  / // __/_\ \ / _ \ / __// _ \/ // // __/ 
 *  /_/|_|/_/ \__//___// .__//_/   \___/\_,_/ \__/  
 *                    /_/   github.com/KitSprout    
 * 
 *  @file    ksfeeder.h
 *  @author  KitSprout
 *  @brief   
 * 
 */

/* Define to prevent recursive inclusion ---------------------------------------------------*/
#ifndef __KSFEEDER_H
#define __KSFEEDER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes --------------------------------------------------------------------------------*/
#include <stdint.h>
#include "kscsv.h"

/* Define ----------------------------------------------------------------------------------*/

#ifndef KSFEEDER_VERSION_DEFINE
#define KSFEEDER_VERSION_DEFINE                         "1.0.0"
#endif

/* Macro -----------------------------------------------------------------------------------*/
/* Typedef ---------------------------------------------------------------------------------*/

typedef enum
{
    VERSION_MODE = 0,
    ONLINE_MODE,
    OFFLINE_MODE

} ksraw_mode_t;

typedef struct
{
    float sn;
    float ts;
    float dt;
    float g[3], gr[3], gb[3];
    float a[3], ar[3], ab[3];
    float m[3], mr[3], mb[3];
    float p, t;
    float ag[3], al[3];
    float rv[4], rva[4], rvm[4];
    int index;

} ksraw_tag_t;

typedef struct
{
    union {
        float mem[KSCSV_IDX_TOTAL];
        ksraw_tag_t raw;
    };
    int mode;

} ksraw_t;

/* Extern ----------------------------------------------------------------------------------*/
/* Functions -------------------------------------------------------------------------------*/

int ksfeeder(int index, ksraw_tag_t *raw);

#ifdef __cplusplus
}
#endif

#endif

/*************************************** END OF FILE ****************************************/
