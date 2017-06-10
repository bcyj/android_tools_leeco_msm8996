/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include <sys/mman.h>
#include <fcntl.h>
#include <linux/android_pmem.h>
#include <string.h>
#include <errno.h>

#include <linux/msm_ion.h>
#include "jpege_app_util_mmap.h"
#include "jpeg_hw_dbg.h"

/*===========================================================================
 * FUNCTION    - do_mmap -
 *
 * DESCRIPTION:  retured virtual addresss
 *==========================================================================*/
void *do_mmap (long size, int *p_pmem_fd, jpeg_pmem_region_type type,
               int *ion_fd, struct ion_allocation_data *alloc,
               struct ion_fd_data *ion_info_fd)
{
#ifdef USE_ION
  void *ret; /* returned virtual address */
  int rc = 0;
  struct ion_handle_data handle_data;

  *ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
  if(*ion_fd < 0) {
    JPEG_HW_PR_ERR("Ion open failed\n");
    goto ION_ALLOC_FAILED;
  }

  /* to make it page size aligned */
  alloc->len = (alloc->len + 4095) & (~4095);
  rc = ioctl(*ion_fd, ION_IOC_ALLOC, alloc);
  if (rc < 0) {
    JPEG_HW_PR_ERR("ION allocation failed\n");
    goto ION_ALLOC_FAILED;
  }

  ion_info_fd->handle = alloc->handle;
  rc = ioctl(*ion_fd, ION_IOC_SHARE, ion_info_fd);
  if (rc < 0) {
    JPEG_HW_PR_ERR("ION map failed %s\n", strerror(errno));
    goto ION_MAP_FAILED;
  }
  *p_pmem_fd = ion_info_fd->fd;
  ret = mmap(NULL,
    alloc->len,
    PROT_READ  | PROT_WRITE,
    MAP_SHARED,
    *p_pmem_fd,
    0);

  if (ret == MAP_FAILED) {
    JPEG_HW_PR_ERR("ION_MMAP_FAILED: %s (%d)\n", strerror(errno), errno);
    goto ION_MAP_FAILED;
  }

  return ret;

ION_MAP_FAILED:
  handle_data.handle = ion_info_fd->handle;
  ioctl(*ion_fd, ION_IOC_FREE, &handle_data);
ION_ALLOC_FAILED:
  return NULL;

#else
  void *addr = NULL;
  char* pmem_region;

  switch(type) {
  case GEMINI_PMEM_SMIPOOL:
    pmem_region = "/dev/pmem_smipool";
    break;
  default:
  case GEMINI_PMEM_ADSP:
    pmem_region = "/dev/pmem_adsp";
    break;
  }

  int pmem_fd = open (pmem_region, O_RDWR|O_SYNC);

  JPEG_HW_DBG ("%s:%d] Open device %s!\n", __func__, __LINE__, pmem_region);
  if (pmem_fd < 0) {
    JPEG_HW_PR_ERR ("%s:%d] Open device %s failed!\n", __func__, __LINE__,
      pmem_region);
    return NULL;
  }

  /*
   * to make it page size aligned
   */
  size = (size + 4095) & (~4095);

  addr = mmap (NULL,
    size, PROT_READ | PROT_WRITE, MAP_SHARED, pmem_fd, 0);

  if (addr == MAP_FAILED) {
    JPEG_HW_PR_ERR ("%s:%d] failed: %s (%d)\n", __func__, __LINE__,
    strerror (errno), errno);
    addr = NULL;
  }

  JPEG_HW_PR_ERR ("%s:%d] pmem_fd %d addr %p size %ld\n", __func__, __LINE__,
    pmem_fd, addr, size);

  *p_pmem_fd = pmem_fd;
  return addr;
#endif
}

/*===========================================================================
 * FUNCTION    - do_munmap -
 *
 * DESCRIPTION:
 *==========================================================================*/
int do_munmap (int pmem_fd, void *addr, size_t size,
               int ion_fd, struct ion_fd_data *ion_info_fd)
{
#ifdef USE_ION
  int rc = 0;
  size = (size + 4095) & (~4095);
  rc = munmap(addr, size);

  close(ion_info_fd->fd);
  struct ion_handle_data handle_data;
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);

  close(ion_fd);
  return rc;
#else
  int rc = 0;
  size = (size + 4095) & (~4095);

  if (NULL != addr) {
    rc = (munmap (addr, size));
  }
  close (pmem_fd);

  JPEG_HW_DBG ("%s:%d] pmem_fd %d addr %p size %u rc %d\n", __func__, __LINE__,
    pmem_fd, addr, size, rc);

  return rc;
#endif
}
