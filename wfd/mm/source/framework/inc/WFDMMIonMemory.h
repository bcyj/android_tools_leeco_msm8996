#ifndef __WFD_MM_ION_HEADER_H__
#define __WFD_MM_ION_HEADER_H__
/* =======================================================================
                              WFDMMIonMemory.h
DESCRIPTION
Header file for WFDIonMemory.cpp file

Copyright (c) 2012-2013 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                             Edit History
$Header:
$DateTime:
$Change:$
========================================================================== */
#include "MMDebugMsg.h"
#include "OMX_Types.h"
#include <sys/mman.h>
#include <linux/msm_ion.h>

/*!*
 *@brief ION memory buffer structure.
*/
struct buffer
{
    void *start;                /* Ptr to start of buffer */
    size_t length;              /* Buffer length          */
    int fd;                     /* Memory descriptor      */
    int offset;                 /* Buffer start offset    */
    int index;                  /* Buffer index           */
    int bytesused;              /* Filled data length     */
    ion_user_handle_t handle;  /* IO memory handle       */
};
/* @brief        Allocate memory on ION pool */
int allocate_ion_mem(unsigned int size, ion_user_handle_t *handle, int ionfd, int heap_id, OMX_BOOL secure_memory);

#endif //__WFD_MM_ION_HEADER_H__
