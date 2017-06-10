/*===========================================================================
 *  FILE:
 *  hdlc.h
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
 *  hdlc.h : Encode asynchronous HDLC protocol packets as described
 *  by both the QUALCOMM download & SDIC (diagnostic) protocol documents
 * ==========================================================================================
 *   $Header: //source/qcom/qct/core/storage/tools/kickstart/hdlc.h#3 $
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
#ifndef HDLC_H
#define HDLC_H

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

int hdlc_encode (char *packet, unsigned short write_size, char *encoded_buffer);
#endif
