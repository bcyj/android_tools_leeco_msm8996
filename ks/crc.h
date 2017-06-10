/*===========================================================================
 *  FILE:
 *  CRC.h
 *
 *  DESCRIPTION:
 *  16-bit LSB CRC computation/verification
 *
 *  PUBLIC CLASSES AND METHODS:
 *  set_crc()
 *  check_crc()
 *  calculate_crc()
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
 *  CRC.h : 16-bit LSB CRC computation/verification
 * ==========================================================================================
 *   $Header: //source/qcom/qct/core/storage/tools/kickstart/CRC.h#3 $
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

/* --------------------------------------------------------------------------- */
/* Pragmas */
/* --------------------------------------------------------------------------- */
#pragma once
/* --------------------------------------------------------------------------- */
/* Definitions */
/* --------------------------------------------------------------------------- */

#define CRC_16_L_POLYNOMIAL 0x8408
#define CRC_16_L_SEED       0xFFFF
#define CRC_16_L_OK        0x0F47
#define CRC_SIZE            = 2;
#define CRC_TABLE_SIZE 256
/*=========================================================================*/
/* Prototypes */
/*=========================================================================*/

/* Calculate and append a 16-bit CRC to given data, the calculated CRC */
void set_crc (char *buffer, unsigned long length);

/* Check a CRC value for the given data, length includes the 2 byte CRC */
/* value at the end of the buffer */
int check_crc (char *buffer, unsigned long length);

/* Calculate a CRC value for the given data */
unsigned short calculate_crc (char *pBuf, unsigned long bitLen);
