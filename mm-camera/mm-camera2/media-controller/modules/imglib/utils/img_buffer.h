/*******************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#ifndef __IMG_BUFFER_H__
#define __IMG_BUFFER_H__

#include <stdio.h>
#include <linux/msm_ion.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <linux/android_pmem.h>
#include <fcntl.h>

/** img_buffer_t:
 *
 *  Arguments:
 *     @ion_info_fd: ION fd information
 *     @alloc: ION allocation data
 *     @fd: buffer fd
 *     @size: buffer sizes
 *     @ion_fd: ION fd
 *     @addr: virtual address
 *
 *  Description:
 *      Buffer structure
 *
 **/
typedef struct  {
  struct ion_fd_data ion_info_fd;
  struct ion_allocation_data alloc;
  int fd;
  long size;
  int ion_fd;
  uint8_t *addr;
} img_buffer_t;

/** buffer_allocate:
 *
 *  Arguments:
 *     @p_buffer: image buffer
 *
 *  Return:
 *     buffer address
 *
 *  Description:
 *      allocates ION buffer
 *
 **/
void* buffer_allocate(img_buffer_t *p_buffer);

/** buffer_deallocate:
 *
 *  Arguments:
 *     @p_buffer: image buffer
 *
 *  Return:
 *     Upon successful completion, returns 0. Otherwise, it returns -1
 *
 *  Description:
 *      deallocates ION buffer
 *
 **/
int buffer_deallocate(img_buffer_t *p_buffer);

#endif //__IMG_BUFFER_H__

