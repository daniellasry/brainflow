#pragma once

#include <ctype.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

#include "cmd_def.h"
#include "uart.h"

#define UART_TIMEOUT 1000

enum class State : int
{
    none = 0,
    init_called = 1,
    initial_connection = 2,
    open_called = 3,
    config_called = 4,
    close_called = 5,
    get_data_called = 6
};

void output (uint8 len1, uint8 *data1, uint16 len2, uint8 *data2);
int read_message (int timeout_ms);
std::string get_dongle_port ();
int open_ble_dev ();
int wait_for_callback (int num_sec);
int reset_ble_dev ();
