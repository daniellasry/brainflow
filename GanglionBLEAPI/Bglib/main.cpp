//
// Bluegiga’s Bluetooth Smart Demo Application
// Contact: support@bluegiga.com.
//
// This is free software distributed under the terms of the MIT license
// reproduced below.
//
// Copyright (c) 2012, Bluegiga Technologies
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
// EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//

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
bd_addr str_to_bdaddr (char *str);
int read_message (int timeout_ms);
void print_bdaddr (bd_addr bdaddr);

int exit_code = (int)GanglionLibNative::SYNC_ERROR;
char uart_port[1024];
bd_addr connect_addr;

namespace GanglionLibNative
{
    int initialize (void *param)
    {
        std::cout << "param is " << (char *)param << std::endl;
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

    // TODO: IMPLEMENT
    int open_ganglion_native (void *param)
    {
        return (int)CustomExitCodesNative::NOT_IMPLEMENTED_ERROR;
    }

    int open_ganglion_mac_addr_native (void *param)
    {
        exit_code = (int)CustomExitCodesNative::SYNC_ERROR;
        char *mac_addr = (char *)param;
        connect_addr = str_to_bdaddr (mac_addr);
        print_bdaddr (connect_addr);

        ble_cmd_gap_connect_direct (&connect_addr, gap_address_type_random, 40, 60, 100, 0);

        int i = 0;
        for (int i = 0; (i < MAX_ATTEMPTS) && (exit_code == (int)CustomExitCodesNative::SYNC_ERROR);
             i++)
        {
            usleep (500000);
            if (read_message (UART_TIMEOUT) > 0)
                break;
        }

        return exit_code;
    }

} // GanglionLibNative

bd_addr str_to_bdaddr (char *str)
{
    bd_addr res;
    short unsigned int addr[6];
    if (sscanf (str, "%02hx:%02hx:%02hx:%02hx:%02hx:%02hx", &addr[5], &addr[4], &addr[3], &addr[2],
            &addr[1], &addr[0]) == 6)
    {

        for (int i = 0; i < 6; i++)
        {
            res.addr[i] = addr[i];
        }
    }
    else
    {
        std::cout << "huj" << std::endl;
    }
    return res;
}

void output (uint8 len1, uint8 *data1, uint16 len2, uint8 *data2)
{
}

int read_message (int timeout_ms)
{
    unsigned char data[256]; // enough for BLE
    struct ble_header hdr;
    int r;

    r = uart_rx (sizeof (hdr), (unsigned char *)&hdr, UART_TIMEOUT);
    if (!r)
    {
        return -1; // timeout
    }
    else if (r < 0)
    {
        printf ("ERROR: Reading header failed. Error code:%d\n", r);
        return 1;
    }

    if (hdr.lolen)
    {
        r = uart_rx (hdr.lolen, data, UART_TIMEOUT);
        if (r <= 0)
        {
            printf ("ERROR: Reading data failed. Error code:%d\n", r);
            return 1;
        }
    }

    const struct ble_msg *msg = ble_get_msg_hdr (hdr);

#ifdef DEBUG
    print_raw_packet (&hdr, data);
#endif

    if (!msg)
    {
        printf ("ERROR: Unknown message received\n");
        exit (1);
    }

    msg->handler (data);

    return 0;
}

void ble_evt_gap_scan_response (const struct ble_msg_gap_scan_response_evt_t *msg)
{
    for (int i = 0; i < 6; i++)
        printf (" %d ", msg->sender.addr[i]);
    printf ("\n");
}

void ble_evt_connection_status (const struct ble_msg_connection_status_evt_t *msg)
{
    // New connection
    if (msg->flags & connection_connected)
    {
        // exit_code = (int)GanglionLibNative::STATUS_OK;
        printf ("Connected\n");
        fflush (stdout);
    }
}

void ble_evt_connection_disconnected (const struct ble_msg_connection_disconnected_evt_t *msg)
{
    std::cout << "reconnect" << std::endl;
    ble_cmd_gap_connect_direct (&connect_addr, gap_address_type_random, 40, 60, 100, 0);
}

void print_bdaddr (bd_addr bdaddr)
{
    printf ("%02x:%02x:%02x:%02x:%02x:%02x", bdaddr.addr[5], bdaddr.addr[4], bdaddr.addr[3],
        bdaddr.addr[2], bdaddr.addr[1], bdaddr.addr[0]);
    fflush (stdout);
}