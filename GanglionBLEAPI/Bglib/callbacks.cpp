#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "GanglionNativeInterface.h"
#include "cmd_def.h"
#include "uart.h"

#include <iostream>

extern int exit_code;
extern bd_addr connect_addr;

// majority of callbacks are stubs in stubs.cpp file
void ble_evt_gap_scan_response (const struct ble_msg_gap_scan_response_evt_t *msg)
{
    // TODO: IMPLEMENT IT?
}

void ble_evt_connection_status (const struct ble_msg_connection_status_evt_t *msg)
{
    // New connection
    if (msg->flags & connection_connected)
    {
        exit_code = (int)GanglionLibNative::STATUS_OK;
#ifdef DEBUG
        std::cerr << "connected" << std::endl;
#endif
    }
}

void ble_evt_connection_disconnected (const struct ble_msg_connection_disconnected_evt_t *msg)
{
    ble_cmd_gap_connect_direct (&connect_addr, gap_address_type_random, 40, 60, 100, 0);
#ifdef DEBUG
    std::cout << "reconnect" << std::endl;
#endif
}
