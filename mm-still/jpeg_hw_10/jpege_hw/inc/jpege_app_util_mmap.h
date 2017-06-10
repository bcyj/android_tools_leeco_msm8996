/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __TEST_UTIL_MMAP_H__
#define __TEST_UTIL_MMAP_H__
#include <linux/msm_ion.h>
typedef enum {
  JPEG_PMEM_ADSP,
  JPEG_PMEM_SMIPOOL,
}jpeg_pmem_region_type;

/*===========================================================================
 * FUNCTION    - do_mmap -
 *
 * DESCRIPTION:  retured virtual addresss
 *==========================================================================*/
void *do_mmap (long size, int *p_pmem_fd, jpeg_pmem_region_type type,
               int *ion_fd, struct ion_allocation_data *alloc,
           struct ion_fd_data *ion_info_fd);

/*===========================================================================
 * FUNCTION    - do_munmap -
 *
 * DESCRIPTION:
 *==========================================================================*/
int do_munmap (int pmem_fd, void *addr, size_t size,
               int ion_fd, struct ion_fd_data *ion_info_fd);
#endif /* __TEST_UTIL_MMAP_H__ */
