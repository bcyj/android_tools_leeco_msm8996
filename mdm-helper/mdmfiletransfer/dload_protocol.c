/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  dload_protocol.c : Implements the DMSS-DL "DLOAD" protocol
 *
 */
#include "dload_protocol.h"
#include "common.h"
#include "comm.h"
#include "log.h"
#include "hdlc.h"

static const unsigned int NUM_PING_RETRIES = 10;

static size_t DLOAD_RX_BUFFER_LENGTH = 4096;
static size_t DLOAD_TX_BUFFER_LENGTH = 4096;
static size_t DLOAD_MISC_BUFFER_LENGTH;

boolean dload_init_buffers ()
{
    port_comm.rx_buffer = (byte *) malloc (DLOAD_RX_BUFFER_LENGTH * sizeof (byte));
    if (NULL == port_comm.rx_buffer) {
        logmsg (LOG_ERROR, "Failed to allocate dload rx buffer");
        goto error_dload_init_buffers;
    }
    port_comm.rx_buffer_size = DLOAD_RX_BUFFER_LENGTH * sizeof (byte);

    port_comm.tx_buffer = (byte *) malloc (DLOAD_TX_BUFFER_LENGTH * sizeof (byte));
    if (NULL == port_comm.tx_buffer) {
        logmsg (LOG_ERROR, "Failed to allocate dload tx buffer");
        goto error_dload_init_buffers;
    }
    port_comm.tx_buffer_size = DLOAD_TX_BUFFER_LENGTH * sizeof (byte);

    DLOAD_MISC_BUFFER_LENGTH = (DLOAD_TX_BUFFER_LENGTH * 2) + 4;
    port_comm.misc_buffer = (byte *) malloc (DLOAD_MISC_BUFFER_LENGTH * sizeof (byte));
    if (NULL == port_comm.misc_buffer) {
        logmsg (LOG_ERROR, "Failed to allocate dload misc buffer");
        goto error_dload_init_buffers;
    }
    port_comm.misc_buffer_size = DLOAD_MISC_BUFFER_LENGTH * sizeof (byte);

    return TRUE;

error_dload_init_buffers:
    free_buffers ();
    return FALSE;
}

static boolean dload_rx_data (byte* buffer, size_t buffer_length, size_t *bytes_read)
{
    boolean func_status      = FALSE;
    size_t  bytes_read_in  = 0, data_size = 0;
    int i = 0;

    do {
        data_size = 0;
        if (FALSE == rx_data (buffer + bytes_read_in, buffer_length - bytes_read_in, &data_size, FALSE)) {
            logmsg (LOG_ERROR, "Reading data failed");
            func_status = FALSE;
            goto return_dload_rx_data;
        }
        logmsg (LOG_ERROR, "Read %d bytes", data_size);
        //for (i = bytes_read_in; i < bytes_read_in+data_size; i++) logmsg (LOG_ERROR, "Byte[%d]: %02X", i, buffer[i]);
        bytes_read_in += data_size;
        logmsg (LOG_ERROR, "Total read in: %d bytes", bytes_read_in);
    } while (buffer[bytes_read_in-1] != AHDLC_FLAG && bytes_read_in > 1 && bytes_read_in < buffer_length && data_size > 0);

    if (buffer[bytes_read_in-1] != AHDLC_FLAG) {
        logmsg (LOG_ERROR, "Trailing HDLC flag not found");
        func_status = FALSE;
    }
    else
        func_status = TRUE;

return_dload_rx_data:
    if (FALSE == func_status) {
        if (NULL != bytes_read)
            *bytes_read = 0;
    }
    else {
        if (NULL != bytes_read)
            *bytes_read = bytes_read_in;
    }
    return func_status;
}

static boolean send_pkt_param_req ()
{
    byte buffer[] = {DLOAD_CMD_PARAM_REQ};

    if (FALSE == encode_and_send (buffer, DLOAD_CMD_PARAM_REQ_SIZE, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
        logmsg (LOG_ERROR, "Failed to send Parameter request packet");
        return FALSE;
    }
    return TRUE;
}

static boolean send_pkt_mem_dbg_query ()
{
    byte buffer[] = {DLOAD_CMD_MEM_DBG_QUERY};

    if (FALSE == encode_and_send (buffer, DLOAD_CMD_MDM_DBG_QUERY_SIZE, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
        logmsg (LOG_ERROR, "Failed to send Mem Dbg Query packet");
        return FALSE;
    }
    return TRUE;
}

static boolean send_pkt_mem_read_req (int address, int length)
{
    byte buffer[DLOAD_CMD_MEM_READ_REQ_SIZE];

    buffer[0] = DLOAD_CMD_MEM_READ_REQ;

    buffer[1] = (address >> 24) & 0xFF;
    buffer[2] = (address >> 16) & 0xFF;
    buffer[3] = (address >> 8) & 0xFF;
    buffer[4] = (address)      & 0xFF;

    buffer[5] = (length >> 8) & 0xFF;
    buffer[6] = (length)      & 0xFF;

    if (FALSE == encode_and_send (buffer, DLOAD_CMD_MEM_READ_REQ_SIZE, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
        logmsg (LOG_ERROR, "Failed to send Mem read request packet");
        return FALSE;
    }
    return TRUE;
}

static boolean send_pkt_unframed_mem_read_req (int address, int length)
{
    byte buffer[DLOAD_CMD_UNFRAMED_MEM_READ_REQ_SIZE];

    buffer[0] = DLOAD_CMD_MEM_UNFRAMED_READ_REQ;

    buffer[1] = buffer[2] = buffer[3] = 0;

    buffer[4] = (address >> 24) & 0xFF;
    buffer[5] = (address >> 16) & 0xFF;
    buffer[6] = (address >> 8)  & 0xFF;
    buffer[7] = (address)       & 0xFF;

    buffer[8]  = (length >> 24) & 0xFF;
    buffer[9]  = (length >> 16) & 0xFF;
    buffer[10] = (length >> 8)  & 0xFF;
    buffer[11] = (length)       & 0xFF;


    if (FALSE == encode_and_send (buffer, DLOAD_CMD_UNFRAMED_MEM_READ_REQ_SIZE, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
        logmsg (LOG_ERROR, "Failed to send Unframed mem read request packet");
        return FALSE;
    }
    return TRUE;
}

static boolean send_pkt_go (int address)
{
    byte buffer[DLOAD_CMD_GO_SIZE];

    buffer[0] = DLOAD_CMD_GO;

    buffer[1] = (address >> 24) & 0xFF;
    buffer[2] = (address >> 16) & 0xFF;
    buffer[3] = (address >> 8) & 0xFF;
    buffer[4] = (address)      & 0xFF;

    if (FALSE == encode_and_send (buffer, DLOAD_CMD_GO_SIZE, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
        logmsg (LOG_ERROR, "Failed to send Go command packet");
        return FALSE;
    }
    return TRUE;
}

static boolean send_pkt_reset ()
{
    byte buffer[] = {DLOAD_CMD_RESET};

    if (FALSE == encode_and_send (buffer, DLOAD_CMD_RESET_SIZE, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
        logmsg (LOG_ERROR, "Failed to send RESET packet");
        return FALSE;
    }
    return TRUE;
}

static boolean send_pkt_nop ()
{
    byte buffer[] = {DLOAD_CMD_NOP};

    if (FALSE == encode_and_send (buffer, DLOAD_CMD_NOP_SIZE, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
        logmsg (LOG_ERROR, "Failed to send NOP packet");
        return FALSE;
    }
    return TRUE;
}

static boolean read_pkt_ack ()
{
    size_t data_size;

    if (FALSE == rx_data(port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size, FALSE)) {
        logmsg (LOG_ERROR, "Reading ACK failed");
        return FALSE;
    }

    if (FALSE == hdlc_decode (port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size)) {
        logmsg (LOG_ERROR, "HDLC decoding failed");
        return FALSE;
    }

    if (port_comm.rx_buffer[0] != DLOAD_CMD_ACK) {
        logmsg (LOG_ERROR, "ACK was not received, received 0x%02X instead", port_comm.rx_buffer[0]);
        if (port_comm.rx_buffer[0] == DLOAD_CMD_NAK) {
            logmsg (LOG_ERROR, "NAK received with error code %02X %02X", port_comm.rx_buffer[1], port_comm.rx_buffer[2]);
        }
        return FALSE;
    }
    return TRUE;
}

static boolean read_pkt_mem_dbg_info_resp ()
{
    size_t data_size;

    if (FALSE == rx_data(port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size, FALSE)) {
        logmsg (LOG_ERROR, "Read failed");
        return FALSE;
    }
    /* Send hdlc_decode the received number of bytes and get back the decoded size */
    if (FALSE == hdlc_decode_skip_opening (port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size, TRUE)) {
        logmsg (LOG_ERROR, "HDLC decoding failed");
        return FALSE;
    }
    if (port_comm.rx_buffer[0] != DLOAD_CMD_MEM_DBG_INFO_RESP) {
        logmsg (LOG_ERROR, "Memory debug Info response was not received");
        return FALSE;
    }
    return TRUE;
}

static boolean read_mem_read_resp ()
{
    size_t data_size;
    unsigned int i;

    if (FALSE == dload_rx_data (port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size)) {
        logmsg (LOG_ERROR, "Read failed");
        return FALSE;
    }

    //logmsg (LOG_ERROR, "data_size: %d", data_size);
    /*
    for (i = 0; i < 10; i++)
        if (i < 10)
            logmsg (LOG_ERROR, "buf[%02d]: %02x", i, port_comm.rx_buffer[i]);
        else
            logmsg (LOG_ERROR, "buf[%02d]: %c", i, port_comm.rx_buffer[i]);
    */
    //logmsg (LOG_ERROR, "buf[0]: %02x", m_comm->recv_buffer[0]);
    //logmsg (LOG_ERROR, "buf[1]: %02x, buf[2]: %02x", m_comm->recv_buffer[1], m_comm->recv_buffer[2]);

    /* Send hdlc_decode the received number of bytes and get back the decoded size */
    if (FALSE == hdlc_decode_skip_opening (port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size, TRUE)) {
        logmsg (LOG_ERROR, "HDLC decoding failed");
        return FALSE;
    }
    if (port_comm.rx_buffer[0] != DLOAD_CMD_MEM_READ_RESP) {
        logmsg (LOG_ERROR, "Memory read response was not received, received 0x%02X instead", port_comm.rx_buffer[0]);
        return FALSE;
    }
    return TRUE;
}

static boolean read_unframed_mem_read_resp ()
{
    size_t data_size;
    //unsigned int i;

    if (FALSE == rx_data (port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size, FALSE)) {
        logmsg (LOG_ERROR, "Read failed");
        return FALSE;
    }

    if (port_comm.rx_buffer[0] != DLOAD_CMD_MEM_UNFRAMED_READ_RESP) {
        logmsg (LOG_ERROR, "Memory unframed read response was not received");
        /*dbg (ERROR, "data_size: %d", data_size);
        for (i = 0; i < 10; i++)
            dbg (ERROR, "buf[%02d]: %02x", i, m_comm->recv_buffer[i]);*/
        return FALSE;
    }
    return TRUE;
}

static boolean read_pkt_param_resp (boolean skip_opening_byte_check)
{
    size_t data_size;

    if (FALSE == rx_data (port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size, FALSE)) {
        logmsg (LOG_ERROR, "Read failed");
        return FALSE;
    }
    /* Send hdlc_decode the received number of bytes and get back the decoded size */
    if (FALSE == hdlc_decode_skip_opening (port_comm.rx_buffer, port_comm.rx_buffer_size, &data_size, skip_opening_byte_check)) {
        logmsg (LOG_ERROR, "HDLC decoding failed");
        return FALSE;
    }
    if (port_comm.rx_buffer[0] != DLOAD_CMD_PARAM_RESP) {
        logmsg (LOG_ERROR, "Parameter response was not received, received %02X instead", port_comm.rx_buffer[0]);
        return FALSE;
    }
    return TRUE;
}

boolean dload_ping_target ()
{
    unsigned int i;
    for (i = 0; i < NUM_PING_RETRIES; i++) {
        logmsg (LOG_INFO, "Sending NOP");
        if (FALSE == send_pkt_nop ()) {
            logmsg (LOG_ERROR, "Sending NOP failed");
            return FALSE;
        }
        logmsg (LOG_INFO, "Waiting for ACK");
        if (FALSE == read_pkt_ack ()) {
            logmsg (LOG_ERROR, "ACK not received");
        }
        else {
            logmsg (LOG_INFO, "ACK received");
            break;
        }
    }
    if (i == NUM_PING_RETRIES)
        return FALSE;
    return TRUE;
}

boolean collect_ram_dumps ()
{
    unsigned int regions_list_data_size, protocol_version;
    unsigned char *regions_list_data = NULL, *filename, *description, *read_response_data;
    int save_preference, base_address, current_base_address, length, current_length, read_response_address, read_response_length;
    unsigned int i = 0;
    int retval;
    FILE* ram_dump_file = NULL;

    if (FALSE == send_pkt_param_req()) {
        logmsg (LOG_ERROR, "Sending param request packet failed");
        goto error_collect_ram_dumps;
    }
    if (FALSE == read_pkt_param_resp (TRUE)) {
        logmsg (LOG_ERROR, "Reading param response packet failed");
        goto error_collect_ram_dumps;
    }
    protocol_version = port_comm.rx_buffer[1];
    logmsg (LOG_INFO, "Protocol version: %d", protocol_version);

    if (FALSE == send_pkt_mem_dbg_query()) {
        logmsg (LOG_ERROR, "Sending Mem dbg packet failed");
        goto error_collect_ram_dumps;
    }
    if (FALSE == read_pkt_mem_dbg_info_resp()) {
        logmsg (LOG_ERROR, "Error reading Mem dbg info response");
        goto error_collect_ram_dumps;
    }

    /* Get the 'Length' field stored in byte 1 and 2 of the decoded buffer */
    regions_list_data_size = port_comm.rx_buffer[1] << 8 | port_comm.rx_buffer[2];

    /* Since we will use recv_buffer for actually fetching the data, copy out the file table first */
    regions_list_data = (unsigned char *) malloc(regions_list_data_size * sizeof(unsigned char));
    if (NULL==regions_list_data) {
        logmsg (LOG_ERROR, "Memory allocation failure: %s", strerror(errno));
        goto error_collect_ram_dumps;
    }

    memcpy(regions_list_data, &(port_comm.rx_buffer[3]), regions_list_data_size * sizeof(unsigned char));
    while (i < regions_list_data_size) {
        save_preference = regions_list_data[i++];
        base_address = regions_list_data[i] << 24 | regions_list_data[i+1] << 16 | regions_list_data[i+2] << 8 | regions_list_data[i+3];
        i += 4;
        length = regions_list_data[i] << 24 | regions_list_data[i+1] << 16 | regions_list_data[i+2] << 8 | regions_list_data[i+3];
        i += 4;

        description = &regions_list_data[i];
        i += 1 + strlen((char *)&(regions_list_data[i]));
        filename = &regions_list_data[i];
        i += 1 + strlen((char *)&(regions_list_data[i]));
        logmsg (LOG_INFO, "Filename: \"%s\", Desc: %s, Save Pref: %d, Base address: 0x%08x, Length: %d bytes", filename, description, save_preference, base_address, length);
        current_length = length;
        current_base_address = base_address;

        //if (length < 196608) {
        logmsg (LOG_EVENT,"Saving \"%s\"", filename);
        ram_dump_file = open_file ((const char *)filename, FALSE);
        if (NULL == ram_dump_file) {
            logmsg (LOG_ERROR, "Could not open file for saving");
            goto error_collect_ram_dumps;
        }
        while (current_length != 0) {
            if (protocol_version >= PROTOCOL_VERSION_UNFRAMED_MEMORY_READ) {
                if (FALSE == send_pkt_unframed_mem_read_req (current_base_address, min(MAX_UNFRAMED_READ_LENGTH, current_length))) {
                    logmsg (LOG_ERROR, "Sending memory read packet failed");
                    goto error_collect_ram_dumps;
                }
                if (FALSE == read_unframed_mem_read_resp ()) {
                    logmsg (LOG_ERROR, "Reading memory read response packet failed");
                    logmsg (LOG_ERROR, "Only completed reading %d of %d bytes", length-current_length, length);
                    if (FALSE == dload_ping_target ()) {
                        logmsg (LOG_ERROR, "Target not responsive");
                        goto error_collect_ram_dumps;
                    }
                    else {
                        logmsg (LOG_STATUS, "Target responsive again. Continuing.");
                        continue;
                    }
                }
                read_response_address = port_comm.rx_buffer[4] << 24 | port_comm.rx_buffer[5] << 16 | port_comm.rx_buffer[6] << 8 | port_comm.rx_buffer[7];
                read_response_length = port_comm.rx_buffer[8] << 24 | port_comm.rx_buffer[9] << 16 | port_comm.rx_buffer[10] << 8 | port_comm.rx_buffer[11];
                read_response_data = &port_comm.rx_buffer[12];

                if (read_response_address != current_base_address) {
                    logmsg (LOG_ERROR, "Received memory response address does not match request address. Retrying.");
                    continue;
                }
            }
            if (protocol_version < PROTOCOL_VERSION_UNFRAMED_MEMORY_READ) {
                /*(logmsg (LOG_ERROR, "Framed reads are currently not supported");
                close_file (ram_dump_file);
                if (NULL != regions_list_data) {
                    free (regions_list_data);
                    regions_list_data = NULL;
                }
                return FALSE;*/
                logmsg (LOG_ERROR, "Sending framed mem read request for %d bytes, address %08X", min(MAX_FRAMED_READ_LENGTH, current_length), current_base_address);
                if (FALSE == send_pkt_mem_read_req (current_base_address, min(MAX_FRAMED_READ_LENGTH, current_length))) {
                    logmsg (LOG_ERROR, "Sending memory read packet failed");
                    goto error_collect_ram_dumps;
                }
                logmsg (LOG_INFO, "Waiting for read response");
                if (FALSE == read_mem_read_resp ()) {
                    logmsg (LOG_ERROR, "Reading memory read response packet failed");
                    logmsg (LOG_ERROR, "Only completed reading %d of %d bytes", length-current_length, length);
                    if (FALSE == dload_ping_target ()) {
                        logmsg (LOG_ERROR, "Target not responsive");
                        goto error_collect_ram_dumps;
                    }
                    else {
                        logmsg (LOG_STATUS, "Target responsive again. Continuing.");
                        continue;
                    }
                }
                read_response_address = port_comm.rx_buffer[1] << 24 | port_comm.rx_buffer[2] << 16 | port_comm.rx_buffer[3] << 8 | port_comm.rx_buffer[4];
                //read_response_length = port_comm.rx_buffer[5] << 24 | port_comm.rx_buffer[6] << 16 | port_comm.rx_buffer[7] << 8 | port_comm.rx_buffer[8];
                read_response_length = port_comm.rx_buffer[5] << 8 | port_comm.rx_buffer[6];
                read_response_data = &port_comm.rx_buffer[7];;

                if (read_response_address != current_base_address) {
                    logmsg (LOG_ERROR, "Received memory response address %08X does not match request address %08X. Retrying.", read_response_address, current_base_address);
                    continue;
                }
            }

            logmsg (LOG_INFO, "Writing data to file");
            retval = fwrite (read_response_data, sizeof(unsigned char), read_response_length, ram_dump_file);
            if (retval<0) {
                logmsg (LOG_ERROR, "file write failed: %s", strerror(errno));
                goto error_collect_ram_dumps;
            }
            if (retval != read_response_length) {
                logmsg (LOG_WARN, "Wrote only %d of %d bytes", retval, read_response_length);
            }
            current_base_address += read_response_length;
            current_length -= read_response_length;
        }
        logmsg (LOG_STATUS, "Received file \"%s\"", filename);
        close_file (ram_dump_file);
        ram_dump_file = NULL;
        //}//
    }

    if (NULL != regions_list_data) {
        free (regions_list_data);
        regions_list_data = NULL;
    }
/*
    if (FALSE == send_pkt_reset ()) {
        logmsg (LOG_ERROR, "Sending RESET failed");
        return FALSE;
    }
    if (FALSE == read_pkt_ack ()) {
        logmsg (LOG_ERROR, "ACK not received");
    }
*/
    logmsg (LOG_STATUS, "RAM dump collection completed");
    return TRUE;

error_collect_ram_dumps:
    if (NULL != regions_list_data) {
        free (regions_list_data);
        regions_list_data = NULL;
    }
    if (NULL != ram_dump_file) {
        close_file(ram_dump_file);
        ram_dump_file = NULL;
    }
    return FALSE;
}

static boolean parse_hex_file (const char* filename, boolean read_address_only, int* hex_transfer_address)
{
    size_t num_bytes;
    char start_code;
    int byte_count, address, record_type, checksum;
    char eol;
    FILE* fp = NULL;
    byte* packet = NULL;
    byte line_data[0xFF] = { 0 };
    int i = 0,
        line_number = 0,
        max_write_size = 0,
        packet_index = 0,
        packet_size = 0,
        transfer_address = 0,
        packet_ok = 0,
        max_write_retry = 10,
        write_attempt = 0,
        retval = FALSE;

    logmsg (LOG_INFO, "Opening file %s", filename);
    fp = fopen (filename, "r");

    if (NULL == fp) {
        logmsg (LOG_ERROR, "Failed to open hex file");
        return FALSE;
    }

    if (read_address_only == FALSE) {

        if (FALSE == send_pkt_param_req()) {
            logmsg (LOG_ERROR, "Sending param request packet failed");
            goto free_file;
        }
        if (FALSE == read_pkt_param_resp (TRUE)) {
            logmsg (LOG_ERROR, "Reading param response packet failed");
            goto free_file;
        }
        max_write_size = (port_comm.rx_buffer[3] << 8) | port_comm.rx_buffer[4];

        if (FALSE == max_write_size) {
            logmsg (LOG_ERROR, "Could not retrieve max write size from target");
            goto free_file;
        }
        if (max_write_size < DLOAD_WRITE_SIZE_THRESHOLD) {
            logmsg (LOG_INFO, "Setting max_write_size to WRITE_SIZE_THRESHOLD:%d for better upload rate ", DLOAD_WRITE_SIZE_THRESHOLD);
            max_write_size = DLOAD_WRITE_SIZE_THRESHOLD;
        }
        packet_size = (7 + DLOAD_WRITE_SIZE_THRESHOLD);
        packet = (unsigned char*) malloc (packet_size * sizeof(unsigned char));
        if (NULL == packet) {
            logmsg (LOG_ERROR, "Memory allocation failure: %s", strerror(errno));
            goto free_file;
        }
        packet_index = 7;
        transfer_address = *hex_transfer_address;
    }

    do {
        num_bytes = fscanf (fp, "%c%2X%4X%2X", &start_code, &byte_count, &address, &record_type);
        if (num_bytes == 0) {
            logmsg (LOG_ERROR, "Unable to parse file data");
            goto free_packet;
        }
        else if (num_bytes == EOF) {
            logmsg (LOG_ERROR, "Unexpected EOF");
            goto free_packet;
        }
        logmsg (LOG_INFO, "read %c,%2X,%4X,%2X", start_code, byte_count, address, record_type);

        if (start_code != ':') {
            logmsg (LOG_ERROR, "Start code : not found");
            goto free_packet;
        }
        if (byte_count > DLOAD_RX_BUFFER_LENGTH) {
            logmsg (LOG_ERROR, "Byte count is larger than can fit in recv buffer");
            goto free_packet;
        }
        //dbg (INFO, "start_code %d, byte_count %d, address %d, record_type %d", start_code, byte_count, address, record_type);

        for (i = 0; i < byte_count; i++) {
            num_bytes = fscanf (fp, "%2X", ((int *)(&(line_data[i]))));
            if (num_bytes == 0) {
                logmsg (LOG_ERROR, "Unable to parse file data");
                goto free_packet;
            }
            else if (num_bytes == EOF) {
                logmsg (LOG_ERROR, "Unexpected EOF");
                goto free_packet;
            }
        }

#ifdef WINDOWSPC
        num_bytes = fscanf (fp, "%2X%c", &checksum, &eol);
#else
        num_bytes = fscanf (fp, "%2X\n", &checksum);
#endif
        if (num_bytes == 0) {
            logmsg (LOG_ERROR, "Unable to parse file data");
            goto free_packet;
        }
        else if (num_bytes == EOF) {
            logmsg (LOG_ERROR, "Unexpected EOF");
            goto free_packet;
        }
        line_number++;

        /* A fully formed line has been read. Process here */
        if (read_address_only == TRUE) {
            if (record_type == 5 && byte_count == 4 && address == 0) {
                *hex_transfer_address = ((line_data[0]) << 24) | ((line_data[1]) << 16) | ((line_data[2]) << 8) | (line_data[3]);
                logmsg (LOG_EVENT, "Hex transfer address is %08X, found at line %d", *hex_transfer_address, line_number);
                retval = TRUE;
                goto free_packet;
            }
        }
        else {
            if (record_type == 0 || record_type == 1) {
                if (packet_index + byte_count > packet_size || record_type == 1) {

                    packet[0] = DLOAD_CMD_WRITE_32BIT;
                    packet[1] = (transfer_address >> 24) & 0x000000FF;
                    packet[2] = (transfer_address >> 16) & 0x000000FF;
                    packet[3] = (transfer_address >>  8) & 0x000000FF;
                    packet[4] = transfer_address & 0x000000FF;
                    packet[5] = ((packet_index - 7) >> 8) & 0x00FF;
                    packet[6] = (packet_index - 7) & 0x00FF;
                    //packet[5] = (1536 >> 8) & 0x00FF;
                    //packet[6] = 1536 & 0x00FF;

                    write_attempt = 0;
                    do {
                        if (FALSE == encode_and_send (packet, packet_index, port_comm.misc_buffer, port_comm.misc_buffer_size)) {
                            logmsg (LOG_ERROR, "Failed to send data packet");
                            goto free_packet;
                        }
                        write_attempt++;
                    } while (TRUE != read_pkt_ack() && write_attempt < max_write_retry);

                    if (write_attempt == max_write_retry) {
                        logmsg (LOG_ERROR, "Failed to read ack");
                        goto free_packet;
                    }
                    logmsg (LOG_STATUS, "Sent uptil line %d", line_number);
                    transfer_address = transfer_address + (packet_index - 7);
                    packet_index = 7;
                }
                if (record_type != 1) {
                    memcpy (&packet[packet_index], line_data, byte_count);
                    packet_index += byte_count;
                }
            }
        }
    } while (record_type != 1);
    retval = TRUE;

free_packet:
    if (NULL != packet) {
        free (packet);
        packet = NULL;
    }
free_file:
    fclose(fp);

    return retval;
}

boolean transfer_hex_file (const char* filename)
{
    int hex_transfer_address = -1;
    if (FALSE == parse_hex_file (filename, TRUE, &hex_transfer_address) || hex_transfer_address == -1) {
        logmsg (LOG_ERROR, "Failed to read address from hex file");
        return FALSE;
    }
    else {
        if (FALSE == parse_hex_file (filename, FALSE, &hex_transfer_address)) {
            logmsg (LOG_ERROR, "Failed to transfer hex file");
            return FALSE;
        }
        if (FALSE == send_pkt_go (hex_transfer_address)) {
            logmsg (LOG_ERROR, "Sending go command failed");
            return FALSE;
        }
        if (FALSE == read_pkt_ack ()) {
            logmsg (LOG_ERROR, "ACK not received");
        }
    }
    logmsg (LOG_STATUS, "HEX file transferred successfully");
    return TRUE;
}
