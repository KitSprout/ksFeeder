/**
 *      __            ____
 *     / /__ _  __   / __/                      __  
 *    / //_/(_)/ /_ / /  ___   ____ ___  __ __ / /_ 
 *   / ,<  / // __/_\ \ / _ \ / __// _ \/ // // __/ 
 *  /_/|_|/_/ \__//___// .__//_/   \___/\_,_/ \__/  
 *                    /_/   github.com/KitSprout    
 * 
 *  @file    ksentry.h
 *  @author  KitSprout
 *  @brief   
 * 
 */

/* Define to prevent recursive inclusion ---------------------------------------------------*/
#ifndef __KSENTRY_H
#define __KSENTRY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes --------------------------------------------------------------------------------*/
#include <stdint.h>

/* Define ----------------------------------------------------------------------------------*/
/* Macro -----------------------------------------------------------------------------------*/
/* Typedef ---------------------------------------------------------------------------------*/

typedef struct
{
    double sn;
    double ts;
    double dt;
    double g[3], gr[3], gb[3];
    double a[3], ar[3], ab[3];
    double m[3], mr[3], mb[3];
    double p, t;
    double ag[3], al[3];
    double rv[4], rva[4], rvm[4];

} ksraw_t;

/* Extern ----------------------------------------------------------------------------------*/
/* Functions -------------------------------------------------------------------------------*/

int ksentry(int index, ksraw_t *raw);
// int ksupdate(int index, ksraw_t *raw);

#ifdef __cplusplus
}
#endif

#endif

/*************************************** END OF FILE ****************************************/
