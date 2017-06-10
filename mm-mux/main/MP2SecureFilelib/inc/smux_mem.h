/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                 SecureMux

GENERAL DESCRIPTION
 ION manipulation functions for the SecureMux interface.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2013 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

                            EDIT HISTORY FOR MODULE

$Header:

when       who    what, where, why
--------   ---    ----------------------------------------------------------
09/30/2013  yb    Created
===========================================================================*/
#ifndef SMUX_MEM_H_
#define SMUX_MEM_H_

#include <stdio.h>
#include <stdlib.h>
#include "comdef.h"
#include <ctype.h>
#include <linux/msm_ion.h>

// Data structures
struct smux_ion_info {
   int32_t ion_fd;
   int32_t ifd_data_fd;
   struct ion_handle_data ion_alloc_handle;
   unsigned char * ion_sbuffer;
   uint32_t sbuf_len;
};

/* functions declaration */
#ifdef __cplusplus
extern "C" {
#endif
/* Shared buffer allocation */
int32_t smux_ION_memalloc(struct smux_ion_info *handle,
                                uint32_t size);
/* Shared buffer deallocation */
int32_t smux_ion_dealloc(struct smux_ion_info *handle);

#ifdef __cplusplus
}
#endif

#endif /* SMUX_MEM_H_ */

