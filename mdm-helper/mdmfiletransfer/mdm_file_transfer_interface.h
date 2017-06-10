/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  mdm_file_transfer_interface.h : Interface to dload ram dump collection and
 *  streaming download
 *
 */
#ifndef MDM_FILE_TRANSFER_INTERFACE_H
#define MDM_FILE_TRANSFER_INTERFACE_H

int get_dload_status(char *);
int save_ram_dumps(char *, char *);
int program_nor_image(char *, char *, char *);
#endif
