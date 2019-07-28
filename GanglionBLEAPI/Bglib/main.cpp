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

#define UART_TIMEOUT 1000
#define MAX_ATTEMPTS 3000


// not thread safe but in brainflow these functions could not be called in parallel
namespace GanglionLibNative
{
    int cmp_bdaddr (bd_addr first, bd_addr second);
    void convert_to_lower (char *str);
    void remove_colon (char *str);
    void output (uint8 len1, uint8 *data1, uint16 len2, uint8 *data2);
    bd_addr str_to_bdaddr (char *lower_str);

    int exit_code = (int)CustomExitCodesNative::SYNC_ERROR;
    char uart_port[1024];
    bd_addr connect_addr;

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
        char *mac_addr = (char *)param;
        convert_to_lower (mac_addr);
        remove_colon (mac_addr);
        connect_addr = str_to_bdaddr (mac_addr);

        ble_cmd_gap_discover (gap_discover_observation);

        int i = 0;
        for (int i = 0; i < MAX_ATTEMPTS, exit_code == (int)CustomExitCodesNative::SYNC_ERROR; i++)
        {
            if (read_message (UART_TIMEOUT) > 0)
                break;
        }

        ble_cmd_gap_end_procedure ();
        usleep (500000);
        if (exit_code != (int)CustomExitCodesNative::STATUS_OK)
        {
            return exit_code;
        }
        return (int)CustomExitCodesNative::STATUS_OK;
    }

    void ble_evt_gap_scan_response (const struct ble_msg_gap_scan_response_evt_t *msg)
    {
        for (int i = 0; i < 6; i++)
            printf (msg->sender.addr[i]);
        printf ("\n");
    }

    int cmp_bdaddr (bd_addr first, bd_addr second)
    {
        int i;
        for (i = 0; i < sizeof (bd_addr); i++)
        {
            if (first.addr[i] != second.addr[i])
                return 1;
        }
        return 0;
    }

    void convert_to_lower (char *str)
    {
        for (int i = 0; str[i]; i++)
        {
            if (str[i] != ':')
                str[i] = tolower (str[i]);
        }
    }

    void remove_colon (char *str)
    {
        int count = 0;
        for (int i = 0; str[i]; i++)
            if (str[i] != ':')
                str[count++] = str[i];
        str[count] = '\0';
    }

    bd_addr str_to_bdaddr (char *lower_str)
    {
        bd_addr res;
        uint8 val = 0;
        count = 0;
        for (int i = 0; lower_str[i]; i++)
        {
            if (i % 2 == 0)
            {
                if ((str[i] >= 'a') && (str[i] <= 'z'))
                    val = 16 * (str[i] - 'a');
                else
                    val = 16 * (str[i] - '0');
            }
            else
            {
                if ((str[i] >= 'a') && (str[i] <= 'z'))
                    val += (str[i] - 'a');
                else
                    val += (str[i] - '0');
                res.addr[count++] = val;
            }
        }
        return res;
    }

    void output (uint8 len1, uint8 *data1, uint16 len2, uint8 *data2)
    {
    }
}
