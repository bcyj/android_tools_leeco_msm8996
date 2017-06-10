/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  crc.h : 16-bit LSB CRC computation/verification
 *
 */
#ifndef CRC_H
#define CRC_H

#include "common.h"

/* --------------------------------------------------------------------------- */
/* Definitions */
/* --------------------------------------------------------------------------- */

#define CRC_16_L_POLYNOMIAL 0x8408
#define CRC_16_L_SEED       0xFFFF
#define CRC_16_L_OK         0x0F47
#define CRC_SIZE            2;
#define CRC_TABLE_SIZE 256
/*=========================================================================*/
/* Prototypes */
/*=========================================================================*/

/* Calculate and append a 16-bit CRC to given data, the calculated CRC */
void set_crc (byte *buffer, size_t length);

/* Check a CRC value for the given data, length includes the 2 byte CRC */
/* value at the end of the buffer */
boolean check_crc (byte *buffer, size_t length);

/* Calculate a CRC value for the given data */
unsigned short calculate_crc (byte *pBuf, size_t bitLen);

#endif