/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __TEST_UTIL_MMAP_H__
#define __TEST_UTIL_MMAP_H__
#include <linux/msm_ion.h>
typedef enum {
  GEMINI_PMEM_ADSP,
  GEMINI_PMEM_SMIPOOL,
}gemini_pmem_region_type;

/*===========================================================================
 * FUNCTION    - do_mmap -
 *
 * DESCRIPTION:  retured virtual addresss
 *==========================================================================*/
void *do_mmap (long size, int *p_pmem_fd, gemini_pmem_region_type type,
               int *ion_fd, struct ion_allocation_data *alloc,
		       struct ion_fd_data *ion_info_fd);

/*===========================================================================
 * FUNCTION    - do_munmap -
 *
 * DESCRIPTION:
 *==========================================================================*/
int do_munmap (int pmem_fd, void *addr, size_t size,
               int ion_fd, struct ion_fd_data *ion_info_fd);

/*===========================================================================
 * FUNCTION    - do_mmap_ion -
 *
 * DESCRIPTION:  This version of mmap is used for ION. The API supports
 *               cached or uncached memory
 *==========================================================================*/
void *do_mmap_ion(long size, int *p_pmem_fd,
  int *ion_fd, struct ion_allocation_data *alloc,
  struct ion_fd_data *ion_info_fd, int8_t cached);

/*===========================================================================
 * FUNCTION    - do_clearcache -
 *
 * DESCRIPTION:
 *==========================================================================*/
int do_clearcache(int pmem_fd, void *addr, size_t size,
               int ion_fd, struct ion_fd_data *ion_info_fd);
#endif /* __TEST_UTIL_MMAP_H__ */
