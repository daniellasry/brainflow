#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cmd_def.h"
#include "uart.h"

#include "GanglionNativeInterface.h"

#include <iostream>

#define UART_TIMEOUT 1000
#define MAX_ATTEMPTS 10


void output (uint8 len1, uint8 *data1, uint16 len2, uint8 *data2);
int read_message (int timeout_ms);

int exit_code = (int)GanglionLibNative::SYNC_ERROR;
char uart_port[1024];
bd_addr connect_addr;

namespace GanglionLibNative
{
    int initialize (void *param)
    {
        strcpy (uart_port, (char *)param);
        bglib_output = output;
        exit_code = (int)CustomExitCodesNative::SYNC_ERROR;
        if (uart_open (uart_port))
        {
            return (int)CustomExitCodesNative::GANGLION_NOT_FOUND_ERROR;
        }
        // Reset dongle to get it into known state
        ble_cmd_system_reset (0);
        uart_close ();
        do
        {
            usleep (500000); // 0.5s
        } while (uart_open (uart_port));
        return (int)CustomExitCodesNative::STATUS_OK;
    }

    int open_ganglion_native (void *param)
    {
        exit_code = (int)CustomExitCodesNative::SYNC_ERROR;
        ble_cmd_gap_discover (gap_discover_observation);
        for (int i = 0; (i < MAX_ATTEMPTS) && (exit_code == (int)CustomExitCodesNative::SYNC_ERROR);
             i++)
        {
            if (read_message (UART_TIMEOUT) > 0)
                break;
        }
        if (exit_code != (int)CustomExitCodesNative::STATUS_OK)
        {
#ifdef DEBUG
            std::cout << "Failed to find Ganglion" << std::endl;
#endif
            return exit_code;
        }
        ble_cmd_gap_end_procedure ();
        // send command to connect
        ble_cmd_gap_connect_direct (&connect_addr, gap_address_type_random, 40, 60, 100, 0);
        // wait for callback to be triggered
        for (int i = 0; (i < MAX_ATTEMPTS) && (exit_code == (int)CustomExitCodesNative::SYNC_ERROR);
             i++)
        {
            if (read_message (UART_TIMEOUT) > 0)
                break;
        }

        return exit_code;
        return (int)CustomExitCodesNative::NOT_IMPLEMENTED_ERROR;
    }

    int open_ganglion_mac_addr_native (void *param)
    {
        exit_code = (int)CustomExitCodesNative::SYNC_ERROR;
        char *mac_addr = (char *)param;
        // convert string mac addr to bd_addr struct
        for (int i = 0; i < strlen (mac_addr); i++)
        {
            mac_addr[i] = tolower (mac_addr[i]);
        }
        short unsigned int addr[6];
        if (sscanf (mac_addr, "%02hx:%02hx:%02hx:%02hx:%02hx:%02hx", &addr[5], &addr[4], &addr[3],
                &addr[2], &addr[1], &addr[0]) == 6)
        {
            for (int i = 0; i < 6; i++)
            {
                connect_addr.addr[i] = addr[i];
            }
        }
        else
        {
            return (int)CustomExitCodesNative::INVALID_MAC_ADDR_ERROR;
        }
        // send command to connect
        ble_cmd_gap_connect_direct (&connect_addr, gap_address_type_random, 40, 60, 100, 0);
        // wait for callback to be triggered
        for (int i = 0; (i < MAX_ATTEMPTS) && (exit_code == (int)CustomExitCodesNative::SYNC_ERROR);
             i++)
        {
            if (read_message (UART_TIMEOUT) > 0)
                break;
        }

        return exit_code;
    }

    int stop_stream_native (void *param)
    {
        return (int)CustomExitCodesNative::NOT_IMPLEMENTED_ERROR;
    }

    int start_stream_native (void *param)
    {
        return (int)CustomExitCodesNative::NOT_IMPLEMENTED_ERROR;
    }

    int close_ganglion_native (void *param)
    {
        return (int)CustomExitCodesNative::NOT_IMPLEMENTED_ERROR;
    }

    int get_data_native (void *param)
    {
        return (int)CustomExitCodesNative::NOT_IMPLEMENTED_ERROR;
    }

} // GanglionLibNative

void output (uint8 len1, uint8 *data1, uint16 len2, uint8 *data2)
{
    if (uart_tx (len1, data1) || uart_tx (len2, data2))
    {
        exit_code = (int)GanglionLibNative::GANGLION_NOT_FOUND_ERROR;
#ifdef DEBUG
        std::cout << "failed to write to uart" << std::endl;
#endif
    }
}

// reads messages and calls required callbacks
int read_message (int timeout_ms)
{
    unsigned char data[256]; // enough for BLE
    struct ble_header hdr;
    int r;

    r = uart_rx (sizeof (hdr), (unsigned char *)&hdr, UART_TIMEOUT);
    if (!r)
    {
#ifdef DEBUG
        std::cout << "read_message timeout" << std::endl;
#endif
        return -1; // timeout
    }
    else if (r < 0)
    {
        exit_code = (int)GanglionLibNative::PORT_OPEN_ERROR;
        return 1; // fails to read
    }
#ifdef DEBUG
    std::cout << "read_message read smth" << std::endl;
#endif
    if (hdr.lolen)
    {
        r = uart_rx (hdr.lolen, data, UART_TIMEOUT);
        if (r <= 0)
        {
            exit_code = (int)GanglionLibNative::PORT_OPEN_ERROR;
            return 1; // fails to read
        }
    }

    const struct ble_msg *msg = ble_get_msg_hdr (hdr);

    if (!msg)
    {
        exit_code = (int)GanglionLibNative::GENERAL_ERROR;
        return 1;
    }

    msg->handler (data);

    return 0;
}
