/*===========================================================================
 *  FILE:
 *  dload_protocol.h
 *
 *  DESCRIPTION:
 *  Declaration of Dload protocol specific structures and functions
 *
 *  PUBLIC METHODS:
 *  dload_max_write_bytes()
 *  dload_image()
 *  dload_exec()
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
 *  dload_protocol.h : Declaration of Dload protocol specific structures and functions
 * ==========================================================================================
 *   $Header: //source/qcom/qct/core/storage/tools/kickstart/dload_protocol.h#4 $
 *   $DateTime: 2010/04/14 11:44:40 $
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
#ifndef DLOAD_PROTOCOL_H
#define DLOAD_PROTOCOL_H

#include "comm.h"
#include "hdlc.h"
#include "common_protocol_defs.h"
#include "kickstart_log.h"

/* --------------------------------------------------------------------------- */
/* The header part of the boot image that would be downloaded using DLOAD protocol. */
/* This header is used for getting the image destination address */
struct old_mi_boot_image_header_type {
    enum image_type image_id;                   /* Identifies the type of image this header
                                                 * represents (OEM SBL, AMSS, Apps boot loader,
                                                 * etc.). */
    unsigned int header_vsn_num;                /* Header version number. */
    unsigned int image_src;                     /* Location of image in flash: Address of
                                                * image in NOR or page/sector offset to image
                                                * from page/sector 0 in NAND/SUPERAND. */
    unsigned int image_dest_ptr;                /* Pointer to location to store image in RAM.
                                                 * Also, entry point at which image execution
                                                 * begins. */
    unsigned int image_size;                    /* Size of complete image in bytes */
    unsigned int code_size;                     /* Size of code region of image in bytes */
    unsigned int signature_ptr;                 /* Pointer to images attestation signature */
    unsigned int signature_size;                /* Size of the attestation signature in
                                                 * bytes */
    unsigned int cert_chain_ptr;                /* Pointer to the chain of attestation
                                                 * certificates associated with the image. */
    unsigned int cert_chain_size;               /* Size of the attestation chain in bytes */
};

struct mi_boot_image_header_type {
	unsigned int  codeword;            /* Codeword/magic number defining flash type
								information. */
	unsigned int  magic;               /* Magic number */
	enum image_type  image_id;        /* image content */
	unsigned int  RESERVED_1;          /* RESERVED */
	unsigned int  RESERVED_2;          /* RESERVED */
	unsigned int  image_src;             /* Location of RPM_SBL in flash or e-hostdl in RAM. This is given in
								byte offset from beginning of flash/RAM.  */
	unsigned int image_dest_ptr;        /* Pointer to location to store RPM_SBL/e-hostdl in RAM.
								Also, entry point at which execution begins.
								*/
	unsigned int  image_size;      /* Size of RPM_SBL image in bytes */
	unsigned int  code_size;       /* Size of code region in RPM_SBL image in bytes */
	unsigned int signature_ptr;         /* Pointer to images attestation signature */
	unsigned int  signature_size;        /* Size of the attestation signature in
								bytes */
	unsigned int cert_chain_ptr;  /* Pointer to the chain of attestation
								certificates associated with the image. */
	unsigned int  cert_chain_size; /* Size of the attestation chain in bytes */

	unsigned int  oem_root_cert_sel;  /* Root certificate to use for authentication.
								Only used if SECURE_BOOT1 table_sel fuse is
								OEM_TABLE. 1 indicates the first root
								certificate in the chain should be used, etc */
	unsigned int  oem_num_root_certs; /* Number of root certificates in image.
								Only used if SECURE_BOOT1 table_sel fuse is
								OEM_TABLE. Denotes the number of certificates
								OEM has provisioned                          */

	unsigned int  RESERVED_5;          /* RESERVED */
	unsigned int  RESERVED_6;          /* RESERVED */
	unsigned int  RESERVED_7;          /* RESERVED */
	unsigned int  RESERVED_8;          /* RESERVED */
	unsigned int  RESERVED_9;          /* RESERVED */
};

/******************************************************************************
* Name: dload_image
*
* Description:
*    This function starts the DMSS protocol based download for the given input file
*
* Arguments:
*    m_comm           -   Pointer to the com_state structure
*    dload_filename   -   Dload file name
*
*
* Returns:
*    int              -   SUCESS/EFAILED
*
******************************************************************************/
int dload_image (struct com_state *m_comm, char *dload_filename, int noclosedevnode);

/******************************************************************************
* Name: dload_max_write_bytes
*
* Description:
*    This function gets the maximum write size the device supports
*
* Arguments:
*    m_comm           -   Pointer to the com structure
*
* Returns:
*    int              -   Maximum write size
*
******************************************************************************/
int  dload_max_write_bytes (struct com_state *m_comm);

/******************************************************************************
* Name: dload_exec
*
* Description:
*    This function triggers the DMSS "GO" command after uploading the image
*
* Arguments:
*    m_comm           -   Pointer to the com_state structure
*    address          -   Destination address to jump to
*
* Returns:
*    int              -   SUCCESS/EFAILED
*
******************************************************************************/
int dload_exec (struct com_state *port, unsigned long address);

#endif
