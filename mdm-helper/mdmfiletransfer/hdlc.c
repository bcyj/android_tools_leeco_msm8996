/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  hdlc.c : Encode asynchronous HDLC protocol packets as described
 *  by both the QUALCOMM download & SDIC (diagnostic) protocol documents
 *
 */
#include "hdlc.h"
#include "crc.h"
#include "common.h"
#include "log.h"
#include "comm.h"

/* --------------------------------------------------------------------------- */
/* Definitions */
/* --------------------------------------------------------------------------- */

/* Async HDLC defines */
const unsigned char AHDLC_FLAG = 0x7e;
const unsigned char AHDLC_ESCAPE = 0x7d;
const unsigned char AHDLC_ESC_M = 0x20;

/*===========================================================================
 *  METHOD:
 *  hdlc_encode
 *
 *  DESCRIPTION:
 *  HDLC encode the given buffer returning the results in an allocated buffer
 *
 *  PARAMETERS:
 *  packet           -   Pointer to the data
 *  write_size       -   Size of the input buffer
 *  encoded_buffer   -   Pointer to the encoded buffer
 *
 *  RETURN VALUE:
 *  int              -   size of the encoded buffer
 *  ===========================================================================*/
boolean hdlc_encode (byte *packet, size_t write_size, byte *encoded_buffer, size_t encoding_buffer_length, size_t *encoded_length)
{
    unsigned short crc;
    unsigned int encode_index = 0;
    unsigned int decodeIndex;
    char byteOrderedCRC[2];
    unsigned int c;
    boolean func_status = FALSE;


    if (encoding_buffer_length < ((write_size * 2) + 4)) {
        logmsg (LOG_ERROR, "Encoding buffer not large enough");
        func_status = FALSE;
        goto return_hdlc_encode;
    }

    crc = calculate_crc (packet, write_size * 8);
    encoded_buffer[encode_index++] = AHDLC_FLAG;

    /* Add data, escaping when necessary */
    decodeIndex = 0;
    while (decodeIndex < write_size) {
        char value = packet[decodeIndex++];
        if (value == AHDLC_FLAG || value == AHDLC_ESCAPE) {
            value ^= AHDLC_ESC_M;
            encoded_buffer[encode_index++] = AHDLC_ESCAPE;
        }
        encoded_buffer[encode_index++] = value;
    }

    /* Byte order crc */
    byteOrderedCRC[0] = (char)(crc & 0x00ff);
    byteOrderedCRC[1] = (char)(crc >> 8);

    /* Add crc */
    c = 0;
    while (c < 2) {
        unsigned char value = byteOrderedCRC[c++];
        if (value == AHDLC_FLAG || value == AHDLC_ESCAPE) {
            value ^= AHDLC_ESC_M;
            encoded_buffer[encode_index++] = AHDLC_ESCAPE;
        }

        encoded_buffer[encode_index++] = value;
    }

    /* Add trailing flag */
    encoded_buffer[encode_index++] = AHDLC_FLAG;
    func_status = TRUE;

return_hdlc_encode:
    if (NULL != encoded_length)
        *encoded_length = encode_index;
    return func_status;
}

boolean hdlc_decode (byte *buffer, size_t length, size_t *decoded_length)
{
    return hdlc_decode_skip_opening (buffer, length, decoded_length, FALSE);
}

boolean hdlc_decode_skip_opening (byte *buffer, size_t length, size_t *decoded_length, boolean skip_opening_byte_check)
{
    unsigned int buffer_index, buffer_copying_index;
    boolean func_status = FALSE;

    buffer_index = buffer_copying_index = 0;

    if (FALSE == skip_opening_byte_check) {
        if (buffer[buffer_index] != AHDLC_FLAG) {
            logmsg (LOG_ERROR, "Opening HDLC flag not found. Found %02x instead", buffer[buffer_index]);
            func_status = FALSE;
            goto return_hdlc_decode_skip_opening;
        }
        buffer_index++;
    }

    while (buffer_index < length) {
        if (buffer[buffer_index] == AHDLC_ESCAPE) {
            buffer_index++;
            if (buffer[buffer_index] == 0x5E)
                buffer[buffer_copying_index++] = AHDLC_FLAG;
            else if (buffer[buffer_index] == 0x5D)
                buffer[buffer_copying_index++] = AHDLC_ESCAPE;
            else {
                logmsg (LOG_ERROR, "Unknown HDLC escape sequence");
                func_status = FALSE;
                goto return_hdlc_decode_skip_opening;
            }
            buffer_index++;
        }
        else if (buffer[buffer_index] == AHDLC_FLAG) {
            func_status = TRUE;
            break;
        }
        else {
            buffer[buffer_copying_index++] = buffer[buffer_index++];
        }
    }

return_hdlc_decode_skip_opening:
    if (NULL != decoded_length) {
        if (TRUE == func_status)
            *decoded_length = buffer_copying_index;
        else
            *decoded_length = 0;
    }
    return func_status;
}

boolean encode_and_send(byte* buffer, size_t buffer_length, byte* encoding_buffer, size_t encoding_buffer_length)
{
    size_t encode_size;

    if (FALSE == hdlc_encode (buffer, buffer_length, encoding_buffer, encoding_buffer_length, &encode_size))
        return FALSE;

    if (FALSE == tx_data (encoding_buffer, encode_size, NULL)) {
        logmsg (LOG_ERROR, "Data transfer failed");
        return FALSE;
    }

    return TRUE;
}
