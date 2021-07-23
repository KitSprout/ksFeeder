/**
 *      __            ____
 *     / /__ _  __   / __/                      __  
 *    / //_/(_)/ /_ / /  ___   ____ ___  __ __ / /_ 
 *   / ,<  / // __/_\ \ / _ \ / __// _ \/ // // __/ 
 *  /_/|_|/_/ \__//___// .__//_/   \___/\_,_/ \__/  
 *                    /_/   github.com/KitSprout    
 * 
 *  @file    serial.h
 *  @author  KitSprout
 *  @brief   
 * 
 */

/* Define to prevent recursive inclusion ---------------------------------------------------*/
#ifndef __SERIAL_H
#define __SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes --------------------------------------------------------------------------------*/
#include <windows.h>
#include <stdint.h>

#include "kstatus.h"

/* Define ----------------------------------------------------------------------------------*/

#define BAUDRATE_LIST_MAX_LENS                          (9U)
#define COMPORT_LIST_MAX_LENS                           (16U)

#define DEFAULT_COMPORT                                 (1U)
#define DEFAULT_BAUDRATE                                (115200U)
#define DEFAULT_DATABITS                                (8U)
#define DEFAULT_STOPBITS                                (1U)
#define DEFAULT_PARITY                                  (SERIAL_PARITY_NONE)
#define DEFAULT_FLOWCTRL                                (SERIAL_FLOWCONTROL_NONE)

#define DEFAULT_SERIAL_CONFIG { \
    .port           = DEFAULT_COMPORT, \
    .txbuffersize   = 8 * 1024, \
    .rxbuffersize   = 16 * 1024, \
    .cfg = { \
        .baudrate   = DEFAULT_BAUDRATE, \
        .databits   = DEFAULT_DATABITS, \
        .stopbits   = DEFAULT_STOPBITS, \
        .parity     = DEFAULT_PARITY, \
        .flowctrl   = DEFAULT_FLOWCTRL, \
    }, \
    .portlist = { \
        .num = -1, \
        .port = {0}, \
        .info = {NULL} \
    } \
}

/* Macro -----------------------------------------------------------------------------------*/
/* Typedef ---------------------------------------------------------------------------------*/

typedef enum
{
    // SERIAL_PARITY_INVALID   = -1,   // invalid
    SERIAL_PARITY_NONE      =  0,   // parity none
    SERIAL_PARITY_ODD       =  1,   // parity odd
    SERIAL_PARITY_EVEN      =  2,   // parity even
    SERIAL_PARITY_MARK      =  3,   // parity mark
    SERIAL_PARITY_SPACE     =  4    // parity space

} serial_parity_t;

typedef enum
{
    SERIAL_FLOWCONTROL_NONE     = 0,    // flow control none
    SERIAL_FLOWCONTROL_XONXOFF  = 1,    // flow control xon/xoff
    SERIAL_FLOWCONTROL_RTSCTS   = 2,    // flow control rts/cts
    SERIAL_FLOWCONTROL_DTRDSR   = 3     // flow control dtr/dsr

} serial_flowcontrol_t;

typedef enum
{
    SERIAL_RTS_INVALID      = -1,
    SERIAL_RTS_OFF          =  0,   // rts off
    SERIAL_RTS_ON           =  1,   // rts on
    SERIAL_RTS_FLOW_CONTROL =  2    // rts flow control

} serial_rts_t;

typedef enum
{
    SERIAL_CTS_INVALID      = -1,
    SERIAL_CTS_IGNORE       =  0,   // cts ignored
    SERIAL_CTS_FLOW_CONTROL =  1    // cts flow control

} serial_cts_t;

typedef enum
{
	SERIAL_DTR_INVALID      = -1,
	SERIAL_DTR_OFF          =  0,   // dtr off
	SERIAL_DTR_ON           =  1,   // dtr on
	SERIAL_DTR_FLOW_CONTROL =  2    // dtr flow control

} serial_dtr_t;

typedef enum
{
    SERIAL_DSR_INVALID      = -1,
    SERIAL_DSR_IGNORE       =  0,   // dsr ignored
    SERIAL_DSR_FLOW_CONTROL =  1    // dsr flow control

} serial_dsr_t;

typedef enum
{
    SERIAL_XONXOFF_INVALID  = -1,
    SERIAL_XONXOFF_DISABLED =  0,   // xon/xoff disabled
    SERIAL_XONXOFF_IN       =  1,   // xon/xoff input only
    SERIAL_XONXOFF_OUT      =  2,   // xon/xoff output only
    SERIAL_XONXOFF_INOUT    =  3    // xon/xoff input and output

} serial_xonxoff_t;

typedef struct
{
    int baudrate;
    int databits;
    int stopbits;
    serial_parity_t parity;
    serial_flowcontrol_t flowctrl;
    serial_rts_t rts;
    serial_cts_t cts;
    serial_dtr_t dtr;
    serial_dsr_t dsr;
    serial_xonxoff_t xonoff;

} serial_config_t;

typedef struct
{
    int num;
    int port[COMPORT_LIST_MAX_LENS];
    char *info[COMPORT_LIST_MAX_LENS];

} serial_portlist_t;

typedef struct
{
    int port;

    char name[8];

    uint32_t isconnected;
    uint32_t txbuffersize;
    uint32_t rxbuffersize;

    HANDLE handle;
    COMMTIMEOUTS timeouts;

    serial_config_t cfg;
    serial_portlist_t portlist;

} serial_t;

/* Extern ----------------------------------------------------------------------------------*/

extern const char SERIAL_PARITY_STRING[][8];
extern const char SERIAL_PARITY_STRING_SHORT[];
extern const char SERIAL_FLOWCTRL_STRING[][8];
extern const int SERIAL_BAUDRATE_LIST[BAUDRATE_LIST_MAX_LENS];

extern serial_t s;

/* Functions -------------------------------------------------------------------------------*/

uint32_t    openSerialPort(void);

void        serial_delay(uint32_t ms);

int         serial_open(serial_t *serial);
int         serial_close(serial_t *serial);
void        serial_set_baudrate(serial_t *serial, int baudrate);
int         serial_get_portlist(serial_portlist_t *list);
void        serial_free_portlist(serial_portlist_t *list);

int         serial_send_byte(const serial_t *serial, const uint8_t sdata);
uint8_t     serial_recv_byte(const serial_t *serial);
int         serial_send_data(const serial_t *serial, const uint8_t *sdata, uint32_t lens);
int         serial_recv_data(const serial_t *serial, uint8_t *rdata, uint32_t lens);

void        serial_flush(const serial_t *serial);
void        serial_flush_rx(const serial_t *serial);
void        serial_flush_tx(const serial_t *serial);

int         serial_show_portlist(serial_portlist_t *list, char *string);

#ifdef __cplusplus
}
#endif

#endif

/*************************************** END OF FILE ****************************************/
