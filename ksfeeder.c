/**
 *      __            ____
 *     / /__ _  __   / __/                      __  
 *    / //_/(_)/ /_ / /  ___   ____ ___  __ __ / /_ 
 *   / ,<  / // __/_\ \ / _ \ / __// _ \/ // // __/ 
 *  /_/|_|/_/ \__//___// .__//_/   \___/\_,_/ \__/  
 *                    /_/   github.com/KitSprout    
 * 
 *  @file    ksentry.c
 *  @author  KitSprout
 *  @brief   
 * 
 */

/* Includes --------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

#include "kslog.h"
#include "ksfeeder.h"

/* Define ----------------------------------------------------------------------------------*/
/* Macro -----------------------------------------------------------------------------------*/
/* Typedef ---------------------------------------------------------------------------------*/
/* Variables -------------------------------------------------------------------------------*/

static int isinited = KS_FALSE;

/* Prototypes ------------------------------------------------------------------------------*/
/* Functions -------------------------------------------------------------------------------*/

/**
 *  @brief  ksfeeder
 */
int ksfeeder(int index, ksraw_tag_t *raw)
{
    if (!isinited)
    {
        if (raw->dt > 1)
        {
            // klogd("[%6d] error dt \n", index);
            raw->index--;
            return KS_FALSE;
        }
        isinited = KS_TRUE;
        klogd("\n[%6d] init ... \n", index);

        // user init process
        // ...
    }

#if 1
    klogd("[%6d][%.0fHz] ", index, 1.0 / raw->dt);
    klogd("[dt] %.3f [g] %10.6f,%10.6f,%10.6f [a] %8.4f,%8.4f,%8.4f [m] %7.2f,%7.2f,%7.2f [t] %.3f\n",
        raw->dt,
        raw->g[0], raw->g[1], raw->g[2],
        raw->a[0], raw->a[1], raw->a[2],
        raw->m[0], raw->m[1], raw->m[2],
        raw->t);
#endif
    // user run process
    // ...

    return KS_TRUE;
}

/*************************************** END OF FILE ****************************************/
