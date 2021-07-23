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
#include "online.h"
#include "offline.h"

/* Define ----------------------------------------------------------------------------------*/

#ifndef DEFAULE_LOGFILE
#define DEFAULE_LOGFILE     "default.csv"
#endif

/* Macro -----------------------------------------------------------------------------------*/
/* Typedef ---------------------------------------------------------------------------------*/

typedef enum
{
    VERSION_MODE = 0,
    ONLINE_MODE,
    OFFLINE_MODE_DEFAULT,
    OFFLINE_MODE_ASSIGN

} mode_t;

/* Variables -------------------------------------------------------------------------------*/
/* Prototypes ------------------------------------------------------------------------------*/
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
            return OFFLINE_MODE_ASSIGN;
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
            char ver[1024] = {"v1.0.0"};
            klogd("  >> version\n");
            klogd("  %s\n", ver);
            break;
        }
        case ONLINE_MODE:
        {
            // klogd(">> Online Mode ... %s\n", argv[1]);
            int freq = 50;
            if (argv[1] != NULL)
            {
                freq = strtoul(argv[2], NULL, 10);
            }
            run_online(argv[1], freq);
            break;
        }
        case OFFLINE_MODE_DEFAULT:
        {
            // klogd(">> Offline Mode ... %s\n", DEFAULE_LOGFILE);
            run_offline(DEFAULE_LOGFILE, NULL);
            break;
        }
        case OFFLINE_MODE_ASSIGN:
        {
            // klogd(">> Offline Mode ... %s\n", argv[1]);
            int range[2] = {0};
            getrange(argv[2], range);
            run_offline(argv[1], range);
            break;
        }
    }
    return 0;
}

/*************************************** END OF FILE ****************************************/
