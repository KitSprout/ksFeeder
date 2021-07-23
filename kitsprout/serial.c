/**
 *      __            ____
 *     / /__ _  __   / __/                      __  
 *    / //_/(_)/ /_ / /  ___   ____ ___  __ __ / /_ 
 *   / ,<  / // __/_\ \ / _ \ / __// _ \/ // // __/ 
 *  /_/|_|/_/ \__//___// .__//_/   \___/\_,_/ \__/  
 *                    /_/   github.com/KitSprout    
 * 
 *  @file    serial.c
 *  @author  KitSprout
 *  @brief   
 * 
 */

/* Includes --------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "kslog.h"
#include "serial.h"

/* Define ----------------------------------------------------------------------------------*/

#define SERIAL_MAX_PORT_NUM                             (255U)
#define SERIAL_STRING_BUFFER_SIZE                       (64U)
#define COMPORT_REG_SUBKEY                              "HARDWARE\\DEVICEMAP\\SERIALCOMM\\"

/* Macro -----------------------------------------------------------------------------------*/
/* Typedef ---------------------------------------------------------------------------------*/
/* Variables -------------------------------------------------------------------------------*/

const char SERIAL_PARITY_STRING[][8] =
{
    "NONE", "ODD", "EVEN", "MARK", "SPACE"
};

const char SERIAL_PARITY_STRING_SHORT[] =
{
    'N', 'O', 'E', '_', '_'
};

const char SERIAL_FLOWCTRL_STRING[][8] =
{
    "NONE", "XONXOFF", "RTSCTS", "DTRDSR"
};

const int SERIAL_BAUDRATE_LIST[BAUDRATE_LIST_MAX_LENS] =
{
    9600,
    19200,
    38400,
    115200,
    230400,
    250000,
    // 256000,
    460800,
    // 500000,
    // 512000,
    921600,
    1000000,
};

// portlist_t portlist;
serial_t s = DEFAULT_SERIAL_CONFIG;

/* Prototypes ------------------------------------------------------------------------------*/

static int get_dcb_config( const serial_t *serial, serial_config_t *config, DCB *dcb );
static int set_dcb_config( const serial_t *serial, serial_config_t *config, DCB *dcb );

/* Functions -------------------------------------------------------------------------------*/

// uint32_t openSerialPort( void )
// {
//     if (s.isconnected == KS_OPEN)
//     {
//         return KS_BUSY;
//     }
//     // check comport
//     if (s.portlist.num == 0)
//     {
//         klogd("  not available port\n");
//         return KS_ERROR;
//     }
//     // open serial port
//     if (serial_open(&s) != KS_OK)
//     {
//         klogd("\n  open serial error (COM%d)\n", s.port);
//         return KS_ERROR;
//     }

//     return KS_OK;
// }

void serial_delay(uint32_t ms)
{
    Sleep(ms);
}

int serial_open(serial_t *serial)
{
    serial_config_t serialcfg;
    DCB dcbcfg;
    DWORD desired_access;
    char portname[SERIAL_STRING_BUFFER_SIZE];

    if (serial->isconnected == KS_OPEN)
    {
        return KS_BUSY;
    }
    // check max comport number
    if (serial->port >= SERIAL_MAX_PORT_NUM)
    {
        klogd("  max port number\n");
        return KS_ERROR;
    }
    // check available comport
    if (serial->portlist.num < 0)
    {
        if (serial_get_portlist(&serial->portlist) != KS_OK)
        {
            klogd("  scan comport error\n");
        }
    }
    if (serial->portlist.num == 0)
    {
        klogd("  not available port\n");
        return KS_ERROR;
    }

    // TODO: check baudrate
    // TODO: check databits
    // TODO: check stopbits
    // TODO: check rts, cts, dtr, dsr, xonoff

    // update port name
    sprintf(serial->name, "COM%d", serial->port);
    sprintf(portname, "\\\\.\\%s", serial->name);

    // open comport
    desired_access = GENERIC_READ | GENERIC_WRITE;
    serial->handle = CreateFileA(portname, desired_access, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (serial->handle == INVALID_HANDLE_VALUE)
    {
        klogd("  open comport failed (%s)\n", serial->name);
        return KS_ERROR;
    }

#if 0
    // event setting
    if (!SetCommMask(serial->handle, EV_RXCHAR | EV_ERR))
    {
        klogd("  set comport event failed\n");
        CloseHandle(serial->handle);
        return KS_ERROR;
    }
#endif

#if 1
    // rx, tx buffer size setting
    if (!SetupComm(serial->handle, serial->rxbuffersize, serial->txbuffersize))
    {
        klogd("  set comport buffer size failed\n");
        CloseHandle(serial->handle);
        return KS_ERROR;
    }
#endif

    // timeout setting
    serial->timeouts.ReadIntervalTimeout          = MAXDWORD;
    serial->timeouts.ReadTotalTimeoutMultiplier   = 0;
    serial->timeouts.ReadTotalTimeoutConstant     = 0;
    serial->timeouts.WriteTotalTimeoutMultiplier  = 0;
    serial->timeouts.WriteTotalTimeoutConstant    = 0;
    if (!SetCommTimeouts(serial->handle, &serial->timeouts))
    {
        klogd("  set comport timeout failed\n");
        CloseHandle(serial->handle);
        return KS_ERROR;
    }

    get_dcb_config(serial, &serialcfg, &dcbcfg);
    dcbcfg.fBinary         = TRUE;
    dcbcfg.fDsrSensitivity = FALSE;
    dcbcfg.fErrorChar      = FALSE;
    dcbcfg.fNull           = FALSE;
    dcbcfg.fAbortOnError   = FALSE;
    set_dcb_config(serial, &serial->cfg, &dcbcfg);
    serial->isconnected = KS_OPEN;

    return KS_OK;
}

int serial_close(serial_t *serial)
{
    if (serial->isconnected != KS_CLOSE)
    {
        CloseHandle(serial->handle);
        serial->isconnected = KS_CLOSE;
        return KS_OK;
    }
    return KS_ERROR;
}

void serial_set_baudrate(serial_t *serial, int baudrate)
{
    serial_close(serial);
    serial->cfg.baudrate = baudrate;
    serial_open(serial);
}

int serial_get_portlist(serial_portlist_t *list)
{
    HKEY hkey;
    LONG retval;
    DWORD type;
    DWORD index = 0;
    TCHAR info[SERIAL_STRING_BUFFER_SIZE];
    TCHAR name[SERIAL_STRING_BUFFER_SIZE];

    /* http://winapi.freetechsecrets.com/win32/WIN32RegOpenKeyEx.htm */
    retval = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, // handle of open key
        COMPORT_REG_SUBKEY, // address of name of subkey to open
        0,                  // reserved
        KEY_QUERY_VALUE,    // security access mask
        &hkey               // address of handle of open key
    );
    if (retval != ERROR_SUCCESS)
    {
        klogd("  open registry key failed\n");
        return KS_ERROR;
    }

    /* http://winapi.freetechsecrets.com/win32/WIN32RegQueryInfoKey.htm */
    retval = RegQueryInfoKey(
        hkey,   // handle of key to query
        NULL,   // address of buffer for class string
        NULL,   // address of size of class string buffer
        NULL,   // reserved
        NULL,   // address of buffer for number of subkeys
        NULL,   // address of buffer for longest subkey name length
        NULL,   // address of buffer for longest class string length
        NULL,   // address of buffer for number of value entries
        NULL,   // address of buffer for longest value name length
        NULL,   // address of buffer for longest value data length
        NULL,   // address of buffer for security descriptor length
        NULL    // address of buffer for last write time
    );
    if (retval != ERROR_SUCCESS)
    {
        klogd("  get registry key information failed\n");
        RegCloseKey(hkey);
        return KS_ERROR;
    }

    serial_free_portlist(list);

    while (1)
    {
        DWORD infobuffersize = SERIAL_STRING_BUFFER_SIZE;
        DWORD namebuffersize = SERIAL_STRING_BUFFER_SIZE;

        /* http://winapi.freetechsecrets.com/win32/WIN32RegEnumValue.htm */
        retval = RegEnumValue(
            hkey,               // handle of key to query
            index,              // index of value to query
            info,               // address of buffer for value string
            &infobuffersize,    // address for size of value buffer
            NULL,               // reserved
            &type,              // address of buffer for type code
            (LPBYTE)name,       // address of buffer for value data
            &namebuffersize     // address for size of data buffer
        );
        if (retval != ERROR_SUCCESS)
        {
            break;
        }

        if (type == REG_SZ)
        {
            name[namebuffersize] = '\0';
            info[infobuffersize] = '\0';
            list->info[list->num] = malloc(sizeof(char) * infobuffersize + 1);
            memcpy(list->info[list->num], info, infobuffersize + 1);
            sscanf(name, "COM%d", &list->port[list->num]);
            list->num++;
        }
        index++;
    }

    RegCloseKey(hkey);

    return KS_OK;
}

void serial_free_portlist(serial_portlist_t *list)
{
    for (int i = 0; i < list->num; i++)
    {
        free(list->info[i]);
        list->info[i] = NULL;
    }
    list->num = 0;
}

int serial_send_byte(const serial_t *serial, const uint8_t sdata)
{
    DWORD n;
    WriteFile(serial->handle, &sdata, 1, &n, NULL);
    if (n < 0)
    {
        return KS_ERROR;
    }
    return KS_OK;
}

uint8_t serial_recv_byte(const serial_t *serial)
{
    DWORD n;
    uint8_t rdata;
    ReadFile(serial->handle, &rdata, 1, &n, NULL);
    return rdata;
}

int serial_send_data( const serial_t *serial, const uint8_t *sdata, uint32_t lens )
{
    DWORD n;
    if (WriteFile(serial->handle, sdata, lens, &n, NULL))
    {
        return (n);
    }
    return (-1);
}

int serial_recv_data(const serial_t *serial, uint8_t *rdata, uint32_t lens)
{
    DWORD n;
    ReadFile(serial->handle, rdata, lens, &n, NULL);
    return (n);
}

void serial_flush(const serial_t *serial)
{
    PurgeComm(serial->handle, PURGE_RXCLEAR | PURGE_RXABORT);
    PurgeComm(serial->handle, PURGE_TXCLEAR | PURGE_TXABORT);
}

void serial_flush_rx(const serial_t *serial)
{
    PurgeComm(serial->handle, PURGE_RXCLEAR | PURGE_RXABORT);
}

void serial_flush_tx(const serial_t *serial)
{
    PurgeComm(serial->handle, PURGE_TXCLEAR | PURGE_TXABORT);
}

static int get_dcb_config(const serial_t *serial, serial_config_t *config, DCB *dcb)
{
    if (!GetCommState(serial->handle, dcb))
    {
        klogd("  get comport dcb setting failed\n");
        return KS_ERROR;
    }

    config->baudrate = dcb->BaudRate;
    config->databits = dcb->ByteSize;

    if (dcb->fParity)
    {
        switch (dcb->Parity)
        {
            case NOPARITY:    config->parity = SERIAL_PARITY_NONE;    break;
            case ODDPARITY:   config->parity = SERIAL_PARITY_ODD;     break;
            case EVENPARITY:  config->parity = SERIAL_PARITY_EVEN;    break;
            case MARKPARITY:  config->parity = SERIAL_PARITY_MARK;    break;
            case SPACEPARITY: config->parity = SERIAL_PARITY_SPACE;   break;
            // default:          config->parity = SERIAL_PARITY_INVALID; break;
            default:          config->parity = SERIAL_PARITY_NONE; break;
        }
    }
    else
    {
        config->parity = SERIAL_PARITY_NONE;
    }

    switch (dcb->StopBits)
    {
        case ONESTOPBIT:  config->stopbits =  1;  break;
        case TWOSTOPBITS: config->stopbits =  2;  break;
        default:          config->stopbits = -1;  break;
    }

    switch (dcb->fRtsControl)
    {
        case RTS_CONTROL_DISABLE:   config->rts = SERIAL_RTS_OFF;           break;
        case RTS_CONTROL_ENABLE:    config->rts = SERIAL_RTS_ON;            break;
        case RTS_CONTROL_HANDSHAKE: config->rts = SERIAL_RTS_FLOW_CONTROL;  break;
        default:                    config->rts = SERIAL_RTS_INVALID;       break;
    }

    config->cts = (dcb->fOutxCtsFlow) ? SERIAL_CTS_FLOW_CONTROL : SERIAL_CTS_IGNORE;

    switch (dcb->fDtrControl)
    {
        case DTR_CONTROL_DISABLE:   config->dtr = SERIAL_DTR_OFF;           break;
        case DTR_CONTROL_ENABLE:    config->dtr = SERIAL_DTR_ON;            break;
        case DTR_CONTROL_HANDSHAKE: config->dtr = SERIAL_DTR_FLOW_CONTROL;  break;
        default:                    config->dtr = -1;                       break;
    }

    config->dsr = (dcb->fOutxDsrFlow) ? SERIAL_DSR_FLOW_CONTROL : SERIAL_DSR_IGNORE;

    if (dcb->fInX)
    {
        config->xonoff = (dcb->fOutX) ? SERIAL_XONXOFF_INOUT : SERIAL_XONXOFF_IN;
    }
    else
    {
        config->xonoff = (dcb->fOutX) ? SERIAL_XONXOFF_OUT : SERIAL_XONXOFF_DISABLED;
    }

    config->flowctrl = SERIAL_FLOWCONTROL_NONE;

    return KS_OK;
}

static int set_dcb_config(const serial_t *serial, serial_config_t *config, DCB *dcb)
{
    dcb->BaudRate = config->baudrate;
    dcb->ByteSize = config->databits;

    switch (config->parity)
    {
        case SERIAL_PARITY_NONE:  dcb->Parity = NOPARITY;     break;
        case SERIAL_PARITY_ODD:   dcb->Parity = ODDPARITY;    break;
        case SERIAL_PARITY_EVEN:  dcb->Parity = EVENPARITY;   break;
        case SERIAL_PARITY_MARK:  dcb->Parity = MARKPARITY;   break;
        case SERIAL_PARITY_SPACE: dcb->Parity = SPACEPARITY;  break;
        default:
        {
            klogd("  invalid parity setting\n");
            return KS_ERROR;
        }
    }

    switch (config->stopbits)
    {
        case 1:   dcb->StopBits = ONESTOPBIT;   break;
        case 2:   dcb->StopBits = TWOSTOPBITS;  break;
        default:
        {
            klogd("  invalid stopbits setting\n");
            return KS_ERROR;
        }
    }

    switch (config->rts)
    {
        case SERIAL_RTS_OFF:          dcb->fRtsControl = RTS_CONTROL_DISABLE;   break;
        case SERIAL_RTS_ON:           dcb->fRtsControl = RTS_CONTROL_ENABLE;    break;
        case SERIAL_RTS_FLOW_CONTROL: dcb->fRtsControl = RTS_CONTROL_HANDSHAKE; break;
        default:
        {
            klogd("  invalid rts setting\n");
            return KS_ERROR;
        }
    }

    switch (config->cts)
    {
        case SERIAL_CTS_IGNORE:       dcb->fOutxCtsFlow = FALSE;  break;
        case SERIAL_CTS_FLOW_CONTROL: dcb->fOutxCtsFlow = TRUE;   break;
        default:
        {
            klogd("  invalid cts setting\n");
            return KS_ERROR;
        }
    }

    switch (config->dtr)
    {
        case SERIAL_DTR_OFF:          dcb->fDtrControl = DTR_CONTROL_DISABLE;   break;
        case SERIAL_DTR_ON:           dcb->fDtrControl = DTR_CONTROL_ENABLE;    break;
        case SERIAL_DTR_FLOW_CONTROL: dcb->fDtrControl = DTR_CONTROL_HANDSHAKE; break;
        default:
        {
            klogd("  invalid dtr setting\n");
            return KS_ERROR;
        }
    }

    switch (config->dsr)
    {
        case SERIAL_DSR_IGNORE:       dcb->fOutxDsrFlow = FALSE;  break;
        case SERIAL_DSR_FLOW_CONTROL: dcb->fOutxDsrFlow = TRUE;   break;
        default:
        {
            klogd("  invalid dsr setting\n");
            return KS_ERROR;
        }
    }

    switch (config->xonoff)
    {
        case SERIAL_XONXOFF_DISABLED:   dcb->fInX = FALSE;  dcb->fOutX = FALSE;   break;
        case SERIAL_XONXOFF_IN:         dcb->fInX = TRUE;   dcb->fOutX = FALSE;   break;
        case SERIAL_XONXOFF_OUT:        dcb->fInX = FALSE;  dcb->fOutX = TRUE;    break;
        case SERIAL_XONXOFF_INOUT:      dcb->fInX = TRUE;   dcb->fOutX = TRUE;    break;
        default:
        {
            klogd("  invalid xon/xoff setting\n");
            return KS_ERROR;
        }
    }

    if (!SetCommState(serial->handle, dcb))
    {
        klogd("  set comport dcb setting failed\n");
        CloseHandle(serial->handle);
        return KS_ERROR;
    }

    return KS_OK;
}

int serial_show_portlist(serial_portlist_t *list, char *string)
{
    int lens = 0;
    for (int i = 0; i < list->num; i++)
    {
        lens += sprintf(&string[lens], "  [%d] COM%d\t(%s)\n", i + 1, list->port[i], list->info[i]);
    }
    return lens;
}

/*************************************** END OF FILE ****************************************/
