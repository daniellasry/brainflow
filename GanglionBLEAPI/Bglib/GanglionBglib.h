#pragma once

typedef enum
{
    action_none = 0,
    action_scan,
    action_connect,
    action_info
} actions;

typedef enum
{
    state_disconnected = 0,
    state_connecting,
    state_connected,
    state_finding_services,
    state_finding_attributes,
    state_listening_measurements,
    state_finish,
    state_last
} states;


class GanglionBglib
{

public:
    GanglionBglib (char *port_name, char *mac_addr);
    int open_ganglion_native (void *param);
    int open_ganglion_mac_addr_native (void *param);
    int stop_stream_native (void *param);
    int start_stream_native (void *param);
    int close_ganglion_native (void *param);
    int get_data_native (void *param);

private:
    states state;
    char port_name[1024];
    char mac_addr[1024];
};
