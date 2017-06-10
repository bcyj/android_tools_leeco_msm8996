/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  streaming_protocol.c : Implements the Streaming download protocol
 *
 */
#include "streaming_protocol.h"
#include "common.h"
#include "comm.h"
#include "log.h"
#include "hdlc.h"

static const unsigned int NUM_PING_RETRIES = 10;

static size_t STREAMING_RX_BUFFER_LENGTH = 4096;
static size_t STREAMING_TX_BUFFER_LENGTH = 4096;
static size_t STREAMING_MISC_BUFFER_LENGTH;
static size_t streaming_max_write_length;

static const char* streaming_err_codes[] = {
    "Illegal reason (do not use)",
    "Reserved",
    "Invalid destination address",
    "Invalid length",
    "Unexpected end of packet",
    "Invalid command",
    "Reserved",
    "Operation failed",
    "Wrong Flash intelligent ID",
    "Bad programming voltage",
    "Write-verify failed",
    "Reserved",
    "Incorrect security code",
    "Cannot power down phone",
    "NAND Flash programming not supported",
    "Command out of sequence",
    "Close did not succeed",
    "Incompatible feature bits",
    "Out of space",
    "Invalid Security mode",
    "Multi-image NAND not supported",
    "Power off command not supported",
    "Illegal reason (do not use)",
};

static const char* streaming_command_names[] = {
    "illegal",
    "Hello",
    "Hello response",
    "Read",
    "Read data",
    "Simple write",
    "Wrote okay",
    "Stream write",
    "Block written",
    "NOP",
    "NOP response",
    "Reset",
    "Reset ACK",
    "Error",
    "Log",
    "Unlock",
    "Unlocked",
    "Power off",
    "Powering down",
    "Open",
    "Opened",
    "Close",
    "Closed",
    "Security mode",
    "Security mode received",
    "Partition table",
    "Partition table received",
    "Open multi-image",
    "Opened multi-image",
    "Erase Flash",
    "Flash erased",
    "Get ECC state",
    "Current ECC state",
    "Set ECC",
    "Set ECC response"
};

boolean streaming_init_buffers ()
{
    port_comm.rx_buffer = (byte *) malloc (STREAMING_RX_BUFFER_LENGTH * sizeof (byte));
    if (NULL == port_comm.rx_buffer) {
        logmsg (LOG_ERROR, "Failed to allocate streaming rx buffer");
        goto error_streaming_init_buffers;
    }
    port_comm.rx_buffer_size = STREAMING_RX_BUFFER_LENGTH * sizeof (byte);

    port_comm.tx_buffer = (byte *) malloc (STREAMING_TX_BUFFER_LENGTH * sizeof (byte));
    if (NULL == port_comm.tx_buffer) {
        logmsg (LOG_ERROR, "Failed to allocate streaming tx buffer");
        goto error_streaming_init_buffers;
    }
    port_comm.tx_buffer_size = STREAMING_TX_BUFFER_LENGTH * sizeof (byte);

    STREAMING_MISC_BUFFER_LENGTH = (STREAMING_TX_BUFFER_LENGTH * 2) + 4;
    port_comm.misc_buffer = (byte *) malloc (STREAMING_MISC_BUFFER_LENGTH * sizeof (byte));
    if (NULL == port_comm.misc_buffer) {
        logmsg (LOG_ERROR, "Failed to allocate streaming misc buffer");
        goto error_streaming_init_buffers;
    }
    port_comm.misc_buffer_size = STREAMING_MISC_BUFFER_LENGTH * sizeof (byte);
    streaming_max_write_length = (port_comm.misc_buffer_size - HDLC_OVERHEAD_BYTES) / 2
                                 - 1 /* for command */
                                 - 4 /* for address */;

    return TRUE;

error_streaming_init_buffers:
    free_buffers ();
    return FALSE;
}

/* This function is meant to read data, strip out any state-independent packets such as 
log messages, and return a pointer to the first other response packet if any */

static boolean streaming_rx_data (byte* buffer, size_t buffer_length, size_t *response_size, byte **response_buffer)
{
    size_t  total_data_size  = 0, data_size = 0;
    boolean func_status      = FALSE;
    byte*   response_pointer = NULL;
    size_t  response_length  = 0;
    byte*   current_pointer  = NULL;
    size_t  current_packet_start = 0, current_packet_end = 1;

    do {
        if (FALSE == rx_data (buffer + total_data_size, buffer_length - total_data_size, &data_size, FALSE)) {
            logmsg (LOG_ERROR, "Reading data failed");
            func_status = FALSE;
            goto return_streaming_rx_data;
        }
        total_data_size += data_size;
    } while (buffer[data_size-1] != AHDLC_FLAG && buffer_length > 0);
    /* We've now read as much as we could - process it, one painful byte at a time */

    if (total_data_size < 1 || buffer[total_data_size - 1] != AHDLC_FLAG || buffer[0] != AHDLC_FLAG) {
        logmsg (LOG_ERROR, "Invalid data received");
        func_status = FALSE;
        goto return_streaming_rx_data;
    }

    current_packet_start = 0;

    do {

        if (buffer[current_packet_start] != AHDLC_FLAG) {
            logmsg (LOG_ERROR, "HDLC flag not found at beginning of packet");
            func_status = FALSE;
            goto return_streaming_rx_data;
        }
        for (current_packet_end = current_packet_start + 1; current_packet_end < total_data_size && buffer[current_packet_end] != AHDLC_FLAG; current_packet_end++);
        if (buffer[current_packet_end] != AHDLC_FLAG) {
            logmsg (LOG_ERROR, "HDLC flag not found at end of packet");
            func_status = FALSE;
            goto return_streaming_rx_data;
        }
        current_packet_end++;

        if (FALSE == hdlc_decode (&buffer[current_packet_start], current_packet_end - current_packet_start, &data_size)) {
            logmsg (LOG_ERROR, "HDLC decoding failed");
            func_status = FALSE;
            goto return_streaming_rx_data;
        }

        /* process this packet */
        if (buffer[current_packet_start] == STREAMING_CMD_LOG) {
            buffer[current_packet_start + data_size] = '\0';
            logmsg (LOG_STATUS, "Log message: %s", &buffer[current_packet_start + 1]);
        }
        else {
            logmsg (LOG_INFO, "Received packet of type 0x%02X", buffer[current_packet_start]);
            if (response_pointer == NULL) {
                response_pointer = &buffer[current_packet_start];
                response_length = data_size;
                func_status = TRUE;
            }
            else {
                logmsg (LOG_ERROR, "Multiple responses received");
                logmsg (LOG_STATUS, "Packet is of type 0x%02X", buffer[current_packet_start]);
                func_status = FALSE;
                goto return_streaming_rx_data;
            }
        }

        current_packet_start = current_packet_end;
    } while (current_packet_start < total_data_size);

    if (response_pointer == NULL) {
        logmsg (LOG_WARN, "No response received, checking again");
        func_status = streaming_rx_data (buffer, buffer_length, &response_length, &response_pointer);
    }

return_streaming_rx_data:
    if (FALSE == func_status) {
        if (NULL != response_size)
            *response_size = 0;
        if (NULL != response_buffer)
            *response_buffer = NULL;
    }
    else {
        if (NULL != response_size)
            *response_size = response_length;
        if (NULL != response_buffer)
            *response_buffer = response_pointer;
    }
    return func_status;
}

static boolean send_pkt_hello ()
{
    byte buffer[STREAMING_CMD_HELLO_SIZE];
    unsigned long features;
    const char* magic_number_host = "QCOM fast download protocol host";

    buffer[0] = STREAMING_CMD_HELLO;
    memcpy (&buffer[1], magic_number_host, min(32, strlen(magic_number_host)));
    buffer[33] = 0x02;
    buffer[34] = 0x02;

    features = FEATURE_UNCOMPRESSED_DLOAD;
    buffer[35] = features & 0xFF;

    buffer[35] = (features)       & 0xFF;
    buffer[36] = (features >> 8)  & 0xFF;
    buffer[37] = (features >> 16) & 0xFF;
    buffer[38] = (features >> 24) & 0xFF;

    if (FALSE == encode_and_send (buffer, STREAMING_CMD_HELLO_SIZE, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
        logmsg (LOG_ERROR, "Failed to send Hello packet");
        return FALSE;
    }
    return TRUE;
}

static boolean send_file_stream_write (const char* filename, int address)
{
    byte* buffer = port_comm.tx_buffer;
    FILE* input_file = NULL;
    boolean func_status = FALSE;
    size_t response_bytes_read = 0, file_bytes_read = 0, total_file_bytes_read = 0;
    byte *response_buffer = NULL;
    int response_address = 0;
    //size_t file_size = 0;
    int MAX_WRITE_RETRY = 10;
    int write_attempts = 0;
    int mismatch_count = 0;
        boolean write_success  = FALSE;

    input_file = fopen (filename, "rb");

    if (NULL == input_file) {
        logmsg (LOG_ERROR, "Failed to open input file");
        func_status = FALSE;
        goto return_send_file_stream_write;
    }

    streaming_max_write_length = 1024;
    logmsg (LOG_STATUS, "streaming max length is %d", streaming_max_write_length);
    do {
        buffer[0] = STREAMING_CMD_STREAM_WRITE;
        buffer[1] = ((address + total_file_bytes_read))        & 0xFF;
        buffer[2] = ((address + total_file_bytes_read)  >> 8)  & 0xFF;
        buffer[3] = ((address + total_file_bytes_read)  >> 16) & 0xFF;
        buffer[4] = ((address + total_file_bytes_read)  >> 24) & 0xFF;

        fseek (input_file, total_file_bytes_read, SEEK_SET);
        file_bytes_read = fread (&buffer[5], sizeof(byte), streaming_max_write_length, input_file);
        if (file_bytes_read < streaming_max_write_length && 0 == feof (input_file)) {
            logmsg (LOG_ERROR, "Error reading from file");
            func_status = FALSE;
            goto return_send_file_stream_write;
        }
        logmsg (LOG_INFO, "Read %d bytes from file %s", file_bytes_read, filename);

        write_success = FALSE;
        if (FALSE == encode_and_send (buffer, 5 + file_bytes_read, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
            logmsg (LOG_ERROR, "Failed to send Stream Write packet");
            func_status = FALSE;
            goto return_send_file_stream_write;
        }
        logmsg (LOG_INFO, "Sent data packet address: %08X", address + total_file_bytes_read);
        write_attempts++;

        response_bytes_read = 0;
        response_buffer = NULL;

        if (FALSE == streaming_rx_data (port_comm.rx_buffer, port_comm.rx_buffer_size, &response_bytes_read, &response_buffer) || response_buffer == NULL) {
            logmsg (LOG_ERROR, "Reading response failed");
            if (write_attempts < MAX_WRITE_RETRY)
                continue;
            func_status = FALSE;
            goto return_send_file_stream_write;
        }
        write_attempts = 0;

        if (response_bytes_read < STREAMING_CMD_STREAM_WRITE_RESP_SIZE) {
            logmsg (LOG_ERROR, "Data length less than expected");
            func_status = FALSE;
            goto return_send_file_stream_write;
        }

        if (response_buffer[0] != STREAMING_CMD_STREAM_WRITE_RESP) {
            logmsg (LOG_ERROR, "Invalid response received");
            func_status = FALSE;
            goto return_send_file_stream_write;
        }
        response_address = response_buffer[4] << 24 | response_buffer[3] << 16 | response_buffer[2] << 8 | response_buffer[1];
        logmsg (LOG_INFO, "Write RSP address: %08X", response_address);
        if (response_address != address + total_file_bytes_read) {
            logmsg (LOG_ERROR, "Address mismatch. Resending.");
            mismatch_count++;
            continue;
        }

        write_success = TRUE;
        total_file_bytes_read += file_bytes_read;

        logmsg (LOG_STATUS, "total transmistted bytes %d\n",total_file_bytes_read);
    } while (0 == feof (input_file) || write_success == FALSE);

    func_status = TRUE;

return_send_file_stream_write:
    if (NULL != input_file)
        fclose (input_file);
    if (mismatch_count > 0)
	    logmsg (LOG_ERROR, "mismatch_count : %d", mismatch_count);
    return func_status;
}

static boolean send_pkt_secure ()
{
    byte buffer[] = {STREAMING_CMD_SECURITY_MODE};

    if (FALSE == encode_and_send (buffer, STREAMING_CMD_SECURITY_MODE_SIZE, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
        logmsg (LOG_ERROR, "Failed to send Secure packet");
        return FALSE;
    }

    return TRUE;
}

static boolean send_pkt_open_multi_image (byte partition_id)
{
    byte buffer[] = {STREAMING_CMD_OPEN_MULTI_IMAGE, partition_id};

    if (FALSE == encode_and_send (buffer, STREAMING_CMD_OPEN_MULTI_IMAGE_SIZE, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
        logmsg (LOG_ERROR, "Failed to send Open multi packet");
        return FALSE;
    }

    return TRUE;
}

static boolean send_pkt_close ()
{
    byte buffer[] = {STREAMING_CMD_CLOSE};

    if (FALSE == encode_and_send (buffer, STREAMING_CMD_CLOSE_SIZE, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
        logmsg (LOG_ERROR, "Failed to send Close packet");
        return FALSE;
    }

    return TRUE;
}

static boolean send_pkt_reset ()
{
    byte buffer[] = {STREAMING_CMD_RESET};

    if (FALSE == encode_and_send (buffer, STREAMING_CMD_RESET_SIZE, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
        logmsg (LOG_ERROR, "Failed to send Reset packet");
        return FALSE;
    }

    return TRUE;
}

static boolean send_pkt_nop ()
{
    byte buffer[] = {STREAMING_CMD_NOP};

    if (FALSE == encode_and_send (buffer, STREAMING_CMD_NOP_SIZE, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
        logmsg (LOG_ERROR, "Failed to send NOP packet");
        return FALSE;
    }

    return TRUE;
}

static boolean read_hello_resp ()
{
    size_t data_size = 0;
    byte *data_buffer = NULL;
    unsigned int i = 0;
    const char* magic_number_target = "QCOM fast download protocol targ";
    size_t hello_max_write_length;

    if (FALSE == streaming_rx_data (port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size, &data_buffer) || data_buffer == NULL) {
        logmsg (LOG_ERROR, "Read failed");
        return FALSE;
    }

    if (data_size < STREAMING_CMD_HELLO_RESPONSE_SIZE) {
        logmsg (LOG_ERROR, "Data length less than expected");
        return FALSE;
    }

    if (data_buffer[0] != STREAMING_CMD_HELLO_RESPONSE) {
        logmsg (LOG_ERROR, "Invalid response received");
        return FALSE;
    }
    if (0 != memcmp (&data_buffer[1], magic_number_target, strlen (magic_number_target))) {
        logmsg (LOG_ERROR, "Invalid magic string received");
        return FALSE;
    }

    hello_max_write_length = data_buffer[35]       |
                             data_buffer[36] << 8  |
                             data_buffer[37] << 16 |
                             data_buffer[38] << 24;

    logmsg (LOG_STATUS, "Max write length in hello packet: %d", hello_max_write_length);
    streaming_max_write_length = min (streaming_max_write_length, hello_max_write_length);
    logmsg (LOG_STATUS, "Set streaming max length to %d", streaming_max_write_length);

    return TRUE;
}

static boolean read_secure_resp ()
{
    size_t data_size = 0;
    byte *data_buffer = NULL;

    if (FALSE == streaming_rx_data (port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size, &data_buffer) || data_buffer == NULL) {
        logmsg (LOG_ERROR, "Read failed");
        return FALSE;
    }

    if (data_size < STREAMING_CMD_SECURITY_MODE_RESP_SIZE) {
        logmsg (LOG_ERROR, "Data length less than expected");
        return FALSE;
    }

    if (data_buffer[0] != STREAMING_CMD_SECURITY_MODE_RESP) {
        logmsg (LOG_ERROR, "Invalid response received");
        return FALSE;
    }

    return TRUE;
}

static boolean read_open_multi_image_resp ()
{
    size_t data_size = 0;
    byte *data_buffer = NULL;

    if (FALSE == streaming_rx_data (port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size, &data_buffer) || data_buffer == NULL) {
        logmsg (LOG_ERROR, "Read failed");
        return FALSE;
    }

    if (data_size < STREAMING_CMD_OPEN_MULTI_IMAGE_RESP_SIZE) {
        logmsg (LOG_ERROR, "Data length less than expected");
        return FALSE;
    }

    if (data_buffer[0] != STREAMING_CMD_OPEN_MULTI_IMAGE_RESP) {
        logmsg (LOG_ERROR, "Invalid response received");
        return FALSE;
    }

    return TRUE;
}

static boolean read_close_resp ()
{
    size_t data_size = 0;
    byte *data_buffer = NULL;

    if (FALSE == streaming_rx_data (port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size, &data_buffer) || data_buffer == NULL) {
        logmsg (LOG_ERROR, "Read failed");
        return FALSE;
    }

    if (data_size < STREAMING_CMD_CLOSE_RESP_SIZE) {
        logmsg (LOG_ERROR, "Data length less than expected");
        return FALSE;
    }

    if (data_buffer[0] != STREAMING_CMD_CLOSE_RESP) {
        logmsg (LOG_ERROR, "Invalid response received");
        return FALSE;
    }

    return TRUE;
}

static boolean read_reset_resp ()
{
    size_t data_size = 0;
    byte *data_buffer = NULL;

    if (FALSE == streaming_rx_data (port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size, &data_buffer) || data_buffer == NULL) {
        logmsg (LOG_ERROR, "Read failed");
        return FALSE;
    }

    if (data_size < STREAMING_CMD_RESET_RESP_SIZE) {
        logmsg (LOG_ERROR, "Data length less than expected");
        return FALSE;
    }

    if (data_buffer[0] != STREAMING_CMD_RESET_RESP) {
        logmsg (LOG_ERROR, "Invalid response received");
        return FALSE;
    }

    return TRUE;
}

boolean streaming_ping_target ()
{
    unsigned int i;
    for (i = 0; i < NUM_PING_RETRIES; i++) {
        if (FALSE == send_pkt_hello ()) {
            logmsg (LOG_ERROR, "Sending NOP failed");
            return FALSE;
        }
        if (FALSE == read_hello_resp ()) {
            logmsg (LOG_ERROR, "NOP response not received");
        }
        else
            break;
    }
    return TRUE;
}

boolean begin_streaming_emmc (const char* filename)
{
    if (TRUE == send_pkt_secure () && TRUE == read_secure_resp () && TRUE == send_pkt_open_multi_image (0x21) && TRUE == read_open_multi_image_resp ()) {
        if (TRUE == send_file_stream_write (filename, 0) && TRUE == send_pkt_close () && TRUE == read_close_resp () && TRUE == send_pkt_reset () && TRUE == read_reset_resp()) {
            logmsg (LOG_STATUS, "Completed streaming sequence for emmc");
            return TRUE;
        }
    }
    return FALSE;
}

boolean begin_streaming_nor (const char* filename)
{
    if (TRUE == send_file_stream_write (filename, 0) && TRUE == send_pkt_reset ()) {
        read_reset_resp ();
        logmsg (LOG_STATUS, "Completed streaming sequence for nor");
        return TRUE;
    }
    return FALSE;
}
