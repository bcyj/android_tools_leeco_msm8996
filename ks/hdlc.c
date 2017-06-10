/*===========================================================================
 *  FILE:
 *  hdlc.c
 *
 *  DESCRIPTION:
 *  Encode asynchronous HDLC protocol packets as described
 *  by both the QUALCOMM download & SDIC (diagnostic) protocol documents
 *
 *  PUBLIC METHODS:
 *  hdlc_encode()
 *
 *  Copyright (C) 2012 Qualcomm Technologies, Inc. All rights reserved.
 *                  Qualcomm Technologies Proprietary/GTDR
 *
 *  All data and information contained in or disclosed by this document is
 *  confidential and proprietary information of Qualcomm Technologies, Inc. and all
 *  rights therein are expressly reserved.  By accepting this material the
 *  recipient agrees that this material and the information contained therein
 *  is held in confidence and in trust and will not be used, copied, reproduced
 *  in whole or in part, nor its contents revealed in any manner to others
 *  without the express written permission of Qualcomm Technologies, Inc.
 *  ===========================================================================
 *
 *
 *  hdlc.c : Encode asynchronous HDLC protocol packets as described
 *  by both the QUALCOMM download & SDIC (diagnostic) protocol documents
 * ==========================================================================================
 *   $Header: //source/qcom/qct/core/storage/tools/kickstart/hdlc.c#3 $
 *   $DateTime: 2010/04/13 18:00:17 $
 *   $Author: niting $
 *
 *  Edit History:
 *  YYYY-MM-DD		who		why
 *  -----------------------------------------------------------------------------
 *
 *  Copyright 2012 by Qualcomm Technologies, Inc.  All Rights Reserved.
 *
 *==========================================================================================
 */

/* ----------------------------------------------------------------------------- */
/* Include Files */
#include "hdlc.h"
#include "crc.h"
#include <stdio.h>

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
int hdlc_encode (char *packet, unsigned short write_size, char *encoded_buffer)
{
    /* Compute crc */
    unsigned short crc = calculate_crc (packet, write_size * 8);

    /* Add leading flag */
    unsigned int encode_index = 0;

    encoded_buffer[encode_index++] = AHDLC_FLAG;

    /* Add data, escaping when necessary */
    unsigned int decodeIndex = 0;
    while (decodeIndex < write_size) {
        char value = packet[decodeIndex++];
        if (value == AHDLC_FLAG || value == AHDLC_ESCAPE) {
            value ^= AHDLC_ESC_M;
            encoded_buffer[encode_index++] = AHDLC_ESCAPE;
        }
        encoded_buffer[encode_index++] = value;
    }

    /* Byte order crc */
    char byteOrderedCRC[2];
    byteOrderedCRC[0] = (char)(crc & 0x00ff);
    byteOrderedCRC[1] = (char)(crc >> 8);

    /* Add crc */
    unsigned int c = 0;
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
    return encode_index;
}
