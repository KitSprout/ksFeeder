/**
 *      __            ____
 *     / /__ _  __   / __/                      __  
 *    / //_/(_)/ /_ / /  ___   ____ ___  __ __ / /_ 
 *   / ,<  / // __/_\ \ / _ \ / __// _ \/ // // __/ 
 *  /_/|_|/_/ \__//___// .__//_/   \___/\_,_/ \__/  
 *                    /_/   github.com/KitSprout    
 * 
 *  @file    main.c
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

#ifndef DEFAULT_ONLINE_UPDATE_RATE
#define DEFAULT_ONLINE_UPDATE_RATE                      (50)
#endif

/* Macro -----------------------------------------------------------------------------------*/
/* Typedef ---------------------------------------------------------------------------------*/
/* Variables -------------------------------------------------------------------------------*/

const char KSFEEDER_VERSION[] = KSFEEDER_VERSION_DEFINE;

/* Prototypes ------------------------------------------------------------------------------*/

int run_online(char *comport, int updaterate, int save);
int run_offline(char *filename, int *range);

/* Functions -------------------------------------------------------------------------------*/

/**
 *  @brief  check_input
 */
int check_input(char *mode)
{
    if (mode != NULL)
    {
        char port[256] = {0};
        strcpy(port, mode);
        port[3] = '\0';
        if ((strcmp("com", port) == 0) || (strcmp("COM", port) == 0))
        {
            return ONLINE_MODE;
        }
        else
        {
            return OFFLINE_MODE;
        }
    }
    else
    {
        return VERSION_MODE;
    }
}

/**
 *  @brief  getrange
 */
int getrange(const char *string, int *range)
{
    if (string == NULL)
    {
        return KS_ERROR;
    }

    int idx = 0;
    for (int i = 0; i < strlen(string); i++)
    {
        if ((string[i] == ':') || (string[i] == ','))
        {
            idx = i;
        }
    }
    char buf[256] = {0};
    strcpy(buf, string);
    buf[idx] = '\0';
    range[0] = strtoul(buf, NULL, 10);
    range[1] = strtoul(&buf[idx+1], NULL, 10);
    if (range[0] <= 0)
    {
        range[0] = 1;
    }
    if (range[1] < range[0])
    {
        range[0] = 1;
        range[1] = 1;
    }
    return KS_OK;
}

/**
 *  @brief  main
 */
int main(int argc, char **argv)
{
    int mode = check_input(argv[1]);

    switch (mode)
    {
        case VERSION_MODE:
        {
            klogd("\n");
            klogd("  >> version\n");
            klogd("  v%s\n", KSFEEDER_VERSION);
            break;
        }
        case ONLINE_MODE:
        {
            klogd(">> Online Mode ... %s\n", argv[1]);
            int updaterate = DEFAULT_ONLINE_UPDATE_RATE;
            int savelog = KS_FALSE;
            if (argv[1] != NULL)
            {
                updaterate = strtoul(argv[2], NULL, 10);
            }
            if (argv[2] != NULL)
            {
                savelog = KS_TRUE;
                klogd("savelog = KS_TRUE\n");
            }
            run_online(argv[1], updaterate, savelog);
            break;
        }
        case OFFLINE_MODE:
        {
            // klogd(">> Offline Mode ... %s\n", argv[1]);
            int range[2] = {0};
            if (getrange(argv[2], range) != KS_OK)
            {
                run_offline(argv[1], NULL);
            }
            else
            {
                run_offline(argv[1], range);
            }
            break;
        }
    }
    return 0;
}

/*************************************** END OF FILE ****************************************/
