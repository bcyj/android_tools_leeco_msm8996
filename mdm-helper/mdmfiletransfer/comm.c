/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  comm.c : Implementations of functions to communicate over an external device.
 *
 */
#include "comm.h"
#include "log.h"

#define MAX_READ 4096

port_comm_struct port_comm = {NULL, INVALID_HANDLE_VALUE, NULL, NULL, NULL, 0, 0, 0, DLOAD_PROTOCOL, NULL};
int default_rx_timeout = 0;
long long default_rx_usecs = 100000LL;

void free_buffers ()
{
    if (NULL != port_comm.rx_buffer) {
        free (port_comm.rx_buffer);
        port_comm.rx_buffer = NULL;
    }
    if (NULL != port_comm.tx_buffer) {
        free (port_comm.tx_buffer);
        port_comm.tx_buffer = NULL;
    }
    if (NULL != port_comm.misc_buffer) {
        free (port_comm.misc_buffer);
        port_comm.misc_buffer = NULL;
    }
    if (NULL != port_comm.protocol_priv) {
        free (port_comm.protocol_priv);
        port_comm.protocol_priv = NULL;
    }
}

boolean connect_and_configure_port ()
{
    if (FALSE == connect_port())
        return FALSE;
    if (FALSE == configure_port ()) {
        disconnect_port ();
        return FALSE;
    }
    return TRUE;
}

boolean connect_port ()
{
    if (NULL == port_comm.port_name) {
        logmsg (LOG_ERROR, "Port name is NULL");
        return FALSE;
    }

    if (INVALID_HANDLE_VALUE != port_comm.port_handle && FALSE == disconnect_port()) {
        logmsg (LOG_ERROR, "Could not close previous port handle");
        return FALSE;
    }

    #ifdef WINDOWSPC
        port_comm.port_handle = CreateFileA(port_comm.port_name,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_EXISTING,
                                0,              // FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING,
                                NULL);
    #else
        port_comm.port_handle = open (port_comm.port_name, O_RDWR | O_SYNC);
    #endif

    if (INVALID_HANDLE_VALUE == port_comm.port_handle) {
        logmsg (LOG_ERROR, "Opening port failed");
        return FALSE;
    }

    logmsg (LOG_STATUS, "Port opened successfully");
    return TRUE;
}

boolean disconnect_port ()
{
    if (INVALID_HANDLE_VALUE == port_comm.port_handle)
        return FALSE;

    #ifdef WINDOWSPC
        CloseHandle(port_comm.port_handle);
    #else
        if (close (port_comm.port_handle) == -1)
            return FALSE;
    #endif

    port_comm.port_handle = INVALID_HANDLE_VALUE;

    return TRUE;
}

void clear_port_comm ()
{
    port_comm.misc_buffer = NULL;
    port_comm.misc_buffer_size = 0;
    port_comm.rx_buffer = NULL;
    port_comm.rx_buffer_size = 0;
    port_comm.tx_buffer = NULL;
    port_comm.tx_buffer_size = 0;

    port_comm.port_handle = INVALID_HANDLE_VALUE;
    port_comm.protocol_name = DLOAD_PROTOCOL;
    port_comm.protocol_priv = NULL;
    /* port_comm.port_name = NULL; */
}

void free_buffers_and_disconnect ()
{
    disconnect_port ();
    free_buffers ();
    clear_port_comm ();
}

boolean rx_data (byte *buffer, size_t buffer_length, size_t *bytes_read, boolean read_to_buf_len)
{
    return rx_data_timeout (buffer, buffer_length, bytes_read, read_to_buf_len, default_rx_timeout);
}

#ifdef WINDOWSPC
boolean configure_port ()
{
    COMMTIMEOUTS timeouts;

    if (INVALID_HANDLE_VALUE == port_comm.port_handle) {
        logmsg (LOG_ERROR, "Port handle is invalid");
        return FALSE;
    }

    if (SAHARA_PROTOCOL == port_comm.protocol_name) {
        if (default_rx_timeout > 0) {
            timeouts.ReadIntervalTimeout        = 10000;
            timeouts.ReadTotalTimeoutMultiplier = 10000;
            timeouts.ReadTotalTimeoutConstant   = 10000;
        }
        else {
            timeouts.ReadIntervalTimeout        = 10000;
            timeouts.ReadTotalTimeoutMultiplier = 0;
            timeouts.ReadTotalTimeoutConstant   = 0;
        }
        timeouts.WriteTotalTimeoutMultiplier= 10000;
        timeouts.WriteTotalTimeoutConstant  = 10000;
    }
    else /*if (DLOAD_PROTOCOL == port_comm.protocol_name)*/ {
        timeouts.ReadIntervalTimeout        = 10;
        timeouts.ReadTotalTimeoutMultiplier = 1;
        timeouts.ReadTotalTimeoutConstant   = 1;
        timeouts.WriteTotalTimeoutMultiplier= 1;
        timeouts.WriteTotalTimeoutConstant  = 10;
    }
    if(!SetCommTimeouts(port_comm.port_handle, &timeouts))
    {
        logmsg (LOG_ERROR, "Error setting com timeouts");
        return FALSE;
    }

    logmsg (LOG_STATUS, "Configured port successfully");
    return TRUE;
}

boolean rx_data_timeout (byte *buffer, size_t buffer_length, size_t *bytes_read, boolean read_to_buf_len, int rx_timeout)
{
    DWORD retval = 0;
    size_t bytes_read_in = 0;
    boolean func_status = FALSE;

    if (!ReadFile(port_comm.port_handle, buffer+bytes_read_in, buffer_length-bytes_read_in, &retval, NULL))
    {
        logmsg (LOG_ERROR, "Error occurred while reading from COM port");
        func_status = FALSE;
        bytes_read_in = 0;
        goto return_rx_data;
    }
    bytes_read_in = retval;
    func_status = TRUE;

return_rx_data:
    if (NULL != bytes_read)
        *bytes_read = bytes_read_in;
    return func_status;
}

boolean tx_data (byte *buffer, size_t buffer_length, size_t *bytes_written)
{
    DWORD retval;
    boolean func_status = FALSE;

    if (!WriteFile (port_comm.port_handle, buffer, buffer_length, &retval, NULL))
    {
        logmsg (LOG_ERROR, "Error occurred while writing to COM port");
        func_status = FALSE;
        goto return_tx_data;
    }
    else
    {
        if(buffer_length != retval)
        {
            logmsg (LOG_ERROR, "USB only wrote %i bytes of %i\n", retval, buffer_length);
            func_status = FALSE;
            goto return_tx_data;
        }
    }
    func_status = TRUE;

return_tx_data:
    if (NULL != bytes_written)
        *bytes_written = retval;
    return func_status;
}

#else

boolean configure_port ()
{
    struct termios termios_settings;

    if (INVALID_HANDLE_VALUE == port_comm.port_handle)
        return FALSE;

    if (strstr(port_comm.port_name, "/dev/ttyS") || strstr(port_comm.port_name, "/dev/ttyHSL") || strstr(port_comm.port_name, "/dev/ttyHS")) {
        logmsg (LOG_INFO, "Configuring UART");
        memset (&termios_settings, 0, sizeof(termios_settings));
        termios_settings.c_iflag = 0;
        termios_settings.c_oflag = 0;
        termios_settings.c_cflag = CS8|CREAD|CLOCAL|CRTSCTS;           // 8n1, see termios.h for more information
        termios_settings.c_lflag = 0;
        termios_settings.c_cc[VMIN] = 1;
        termios_settings.c_cc[VTIME] = 5;
        cfsetospeed (&termios_settings, B4000000);                 // B4000000 baud
        cfsetispeed (&termios_settings, B4000000);                 // B4000000 baud
        tcsetattr(port_comm.port_handle, TCSANOW, &termios_settings);
    }

    if (-1 == tcgetattr (port_comm.port_handle, &termios_settings)){
        logmsg(LOG_ERROR,"tcgetattr failed");
        return FALSE;
    }
    cfmakeraw (&termios_settings);
    termios_settings.c_cflag |= CREAD | CLOCAL;

    if (-1 == tcsetattr (port_comm.port_handle, TCSANOW, &termios_settings)){
        logmsg(LOG_ERROR,"tcsetattr failed");
        return FALSE;
    }

    return TRUE;
}

boolean rx_data_timeout (byte *buffer, size_t buffer_length, size_t *bytes_read, boolean read_to_buf_len, int rx_timeout)
{
    fd_set rfds;
    struct timeval tv;
    int retval = 0;
    size_t bytes_read_in = 0;
    boolean func_status = FALSE;

    do {
        /*Init read file descriptor */
        FD_ZERO (&rfds);
        FD_SET (port_comm.port_handle, &rfds);

        /* time out initializtion. */
        tv.tv_sec  = rx_timeout >= 0 ? rx_timeout : 0;
        tv.tv_usec = default_rx_usecs;

        retval = select (port_comm.port_handle + 1, &rfds, NULL, NULL, ((rx_timeout >= 0) ? (&tv) : (NULL)));
        if (0 == retval) {
            logmsg (LOG_ERROR, "Timeout Occured, No response or command came from the target !!!");
            func_status = FALSE;
            goto return_rx_data;
        }
        retval = read (port_comm.port_handle, buffer+bytes_read_in, min((buffer_length - bytes_read_in), MAX_READ));
        if (retval < 0) {
            if (errno != EAGAIN) {
                logmsg (LOG_ERROR, "Read/Write File descriptor returned error: %s, error code %d", strerror (errno), retval);
                func_status = FALSE;
                goto return_rx_data;
            }
        }
        else if (retval == 0) {
            logmsg (LOG_ERROR, "Zero length packet received or hardware connection went off");
        }

        if (retval < 0) {
            logmsg (LOG_ERROR, "select returned error: %s", strerror (errno));
            func_status = FALSE;
            goto return_rx_data;
        }

        bytes_read_in += retval;
    } while (TRUE == read_to_buf_len && bytes_read_in < buffer_length);
    func_status = TRUE;

return_rx_data:
    if (NULL != bytes_read)
        *bytes_read = bytes_read_in;
    return func_status;
}

boolean tx_data (byte *buffer, size_t buffer_length, size_t *bytes_written)
{
    size_t bytes_written_out = 0;
    int retval;
    boolean func_status = FALSE;

    tcflush (port_comm.port_handle, TCIFLUSH);
    while (bytes_written_out < buffer_length) {
        retval = write (port_comm.port_handle, buffer+bytes_written_out, buffer_length-bytes_written_out);
        if (retval < 0) {
            logmsg (LOG_ERROR, "Write returned failure %d", retval);
            func_status = FALSE;
            goto return_tx_data;
        }
        bytes_written_out += retval;
    }
    func_status = TRUE;

return_tx_data:
    if (NULL != bytes_written)
        *bytes_written = bytes_written_out;
    return func_status;
}

#endif
