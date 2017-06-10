/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  comm.h : Interface to communicate over an external device.
 *
 */
#ifndef COMM_H
#define COMM_H

#include "common.h"

enum transport_type
{
    UART,
    SDIO,
    HSIC,
    USB,
    GENERIC
};

/*Invalid handle for com port*/
#ifndef WINDOWSPC
#define INVALID_HANDLE_VALUE  -1
#endif

typedef struct
{
    const char *port_name;

    /* handle to COM port */
    #if defined(WINDOWSPC)
        HANDLE port_handle;
    #else
        int port_handle;
    #endif

    byte *rx_buffer;
    byte *tx_buffer;
    byte *misc_buffer;

    size_t rx_buffer_size;
    size_t tx_buffer_size;
    size_t misc_buffer_size;

    enum protocol_type protocol_name;
    void *protocol_priv;
} port_comm_struct;

extern port_comm_struct port_comm;
extern int default_rx_timeout;
extern long long default_rx_usecs;

boolean connect_port ();
boolean disconnect_port ();
boolean configure_port ();
boolean rx_data (byte *buffer, size_t buffer_length, size_t *bytes_read, boolean read_to_buf_len);
boolean rx_data_timeout (byte *buffer, size_t buffer_length, size_t *bytes_read, boolean read_to_buf_len, int rx_timeout);
boolean tx_data (byte *buffer, size_t buffer_length, size_t *bytes_written);
boolean connect_and_configure_port ();
void free_buffers ();
void free_buffers_and_disconnect ();

#endif
