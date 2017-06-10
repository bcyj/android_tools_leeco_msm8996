/*============================================================================
   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __MSM_CAM_MMAP_H__
#define __MSM_CAM_MMAP_H__

#include <sys/types.h>
#include <stdlib.h>
#include <inttypes.h>

struct file;
struct inode;
struct vm_area_struct;
struct ion_allocation_data;
struct ion_fd_data;

uint8_t *do_mmap(uint32_t size, int *pmemFd);
int do_munmap (int pmem_fd, void *addr, size_t size);
void cam_dump_yuv(void *addr, uint32_t size, char *filename);
void cam_dump_yuv2(void *yaddr, void *cbcraddr, uint32_t size, char *filename);
uint8_t *do_mmap_ion(int ion_fd, struct ion_allocation_data *alloc,
		     struct ion_fd_data *ion_info_fd, int *mapFd);
int do_munmap_ion (int ion_fd,
		   struct ion_fd_data *ion_info_fd, void *addr, size_t size);
#endif /* __MSM_CAM_MMAP_H__ */
