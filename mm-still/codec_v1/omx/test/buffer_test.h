/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#ifndef __QBUFFER_H__
#define __QBUFFER_H__


#include <stdio.h>
#include <linux/msm_ion.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <linux/android_pmem.h>
#include <fcntl.h>
#include "QIDbg.h"


typedef struct  {
  struct ion_fd_data ion_info_fd;
  struct ion_allocation_data alloc;
  int p_pmem_fd;
  long size;
  int ion_fd;
  uint8_t *addr;
} buffer_test_t;

/** buffer_allocate:
 *
 *  Arguments:
 *     @p_buffer: ION buffer
 *
 *  Return:
 *     buffer address
 *
 *  Description:
 *      allocates ION buffer
 *
 **/
void* buffer_allocate(buffer_test_t *p_buffer);

/** buffer_deallocate:
 *
 *  Arguments:
 *     @p_buffer: ION buffer
 *
 *  Return:
 *     buffer address
 *
 *  Description:
 *      deallocates ION buffer
 *
 **/
int buffer_deallocate(buffer_test_t *p_buffer);

#endif

