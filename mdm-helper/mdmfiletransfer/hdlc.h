/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  hdlc.h : Encode asynchronous HDLC protocol packets as described
 *  by both the QUALCOMM download & SDIC (diagnostic) protocol documents
 *
 */
#ifndef HDLC_H
#define HDLC_H

#include "common.h"

/* --------------------------------------------------------------------------- */
/* Definitions */
/* --------------------------------------------------------------------------- */

/*=========================================================================*/
/* Prototypes */
/******************************************************************************
* Name: hdlc_encode
*
* Description:
*    This function encodes the input buffer with HDLC algorithm
*
* Arguments:
*    packet           -   Pointer to the data
*    write_size       -   Size of the input buffer
*    encoded_buffer   -   Pointer to the encode buffer
*
*
* Returns:
*    int              -   size of the encoded buffer
******************************************************************************/

/* overhead is 1 leading flag + 1 trailing flag + 2 CRC bytes + 2 for possible escaping the CRC */
/* the value is meant to indicate what is extra on top of (size of actual packet * 2) */
#define HDLC_OVERHEAD_BYTES (1+1+2)

extern const unsigned char AHDLC_FLAG;

boolean hdlc_encode (byte *packet, size_t write_size, byte *encoded_buffer, size_t encoding_buffer_length, size_t *encoded_length);
boolean hdlc_decode (byte *buffer, size_t length, size_t *decoded_length);
boolean hdlc_decode_skip_opening (byte *buffer, size_t length, size_t *decoded_length, boolean skip_opening_byte_check);
boolean encode_and_send(byte* buffer, size_t buffer_length, byte* encoding_buffer, size_t encoding_buffer_length);

#endif
