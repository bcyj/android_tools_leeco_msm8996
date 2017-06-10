/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <sys/mman.h>
#include <fcntl.h>
#include <linux/android_pmem.h>
#include <string.h>
#include <errno.h>

#include <linux/msm_ion.h>
#include "gemini_app_util_mmap.h"
#include "../src/gemini_dbg.h"
#include <sys/ioctl.h>

/*===========================================================================
 * FUNCTION    - do_mmap_ion -
 *
 * DESCRIPTION:  This version of mmap is used for ION. The API supports
 *               cached or uncached memory
 *==========================================================================*/
void *do_mmap_ion(long size, int *p_pmem_fd,
  int *ion_fd, struct ion_allocation_data *alloc,
  struct ion_fd_data *ion_info_fd, int8_t cached)
{
#ifdef USE_ION
  void *vaddr = NULL; /* returned virtual address */
  int rc = 0;
  struct ion_handle_data handle_data;
  int flags = O_RDONLY;

  if (!cached) {
    flags |= O_SYNC;
    alloc->flags = 0;
  } else {
    alloc->flags = ION_FLAG_CACHED;
  }

  struct ion_flush_data cache_inv_data;
  memset(&cache_inv_data, 0, sizeof(struct ion_flush_data));

  *ion_fd = open("/dev/ion", flags);
  if(*ion_fd < 0) {
    GMN_PR_ERR("%s:%d] Ion open failed", __func__, __LINE__);
    goto ION_ALLOC_FAILED;
  }

  /* to make it page size aligned */
  alloc->heap_mask = 0x1 << ION_IOMMU_HEAP_ID;
  alloc->align = 4096;

  alloc->len = (alloc->len + 4095) & (~4095);
  rc = ioctl(*ion_fd, ION_IOC_ALLOC, alloc);
  if (rc < 0) {
    GMN_PR_ERR("%s:%d] ION allocation failed", __func__, __LINE__);
    goto ION_ALLOC_FAILED;
  }

  ion_info_fd->handle = alloc->handle;
  rc = ioctl(*ion_fd, ION_IOC_SHARE, ion_info_fd);
  if (rc < 0) {
    GMN_PR_ERR("%s:%d] ION map failed %s", __func__, __LINE__, strerror(errno));
    goto ION_MAP_FAILED;
  }

  *p_pmem_fd = ion_info_fd->fd;
  vaddr = mmap(NULL,
    alloc->len,
    PROT_READ  | PROT_WRITE,
    MAP_SHARED,
    *p_pmem_fd,
    0);

  if (vaddr == MAP_FAILED) {
    GMN_PR_ERR("%s:%d] ION_MMAP_FAILED: %s (%d)",
      __func__, __LINE__, strerror(errno), errno);
    goto ION_MAP_FAILED;
  }

  if (cached) {
    cache_inv_data.vaddr = vaddr;
    cache_inv_data.fd = ion_info_fd->fd;
    cache_inv_data.handle =  ion_info_fd->handle;
    cache_inv_data.length = alloc->len;
    if(*ion_fd > 0) {
      if(ioctl(*ion_fd, ION_IOC_INV_CACHES, &cache_inv_data) < 0)
            GMN_PR_ERR("%s:%d] Cache Invalidate failed", __func__, __LINE__);
    }
  }

  return vaddr;

ION_MAP_FAILED:
  handle_data.handle = ion_info_fd->handle;
  ioctl(*ion_fd, ION_IOC_FREE, &handle_data);
ION_ALLOC_FAILED:
  if (*ion_fd > 0)
    close(*ion_fd);
  return NULL;
#endif
}

/*===========================================================================
 * FUNCTION    - do_mmap -
 *
 * DESCRIPTION:  retured virtual addresss
 *==========================================================================*/
void *do_mmap (long size, int *p_pmem_fd, gemini_pmem_region_type type,
               int *ion_fd, struct ion_allocation_data *alloc,
               struct ion_fd_data *ion_info_fd)
{
#ifdef USE_ION
  void *ret; /* returned virtual address */
  int rc = 0;
  struct ion_handle_data handle_data;

  struct ion_flush_data cache_inv_data;
  struct ion_custom_data custom_data;
  memset(&cache_inv_data, 0, sizeof(struct ion_flush_data));

  *ion_fd = open("/dev/ion", O_RDONLY);
  if(*ion_fd < 0) {
    GMN_PR_ERR("Ion open failed\n");
    goto ION_ALLOC_FAILED;
  }

  /* to make it page size aligned */
  alloc->len = (alloc->len + 4095) & (~4095);
  rc = ioctl(*ion_fd, ION_IOC_ALLOC, alloc);
  if (rc < 0) {
    GMN_PR_ERR("ION allocation failed\n");
    goto ION_ALLOC_FAILED;
  }

  ion_info_fd->handle = alloc->handle;
  rc = ioctl(*ion_fd, ION_IOC_SHARE, ion_info_fd);
  if (rc < 0) {
    GMN_PR_ERR("ION map failed %s\n", strerror(errno));
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
    GMN_PR_ERR("ION_MMAP_FAILED: %s (%d)\n", strerror(errno), errno);
    goto ION_MAP_FAILED;
  }

  cache_inv_data.vaddr = ret;
  cache_inv_data.fd = ion_info_fd->fd;
  cache_inv_data.handle =  ion_info_fd->handle;
  cache_inv_data.length = alloc->len;
  custom_data.cmd = ION_IOC_INV_CACHES;
  custom_data.arg = (unsigned long)&cache_inv_data;
  if(*ion_fd > 0) {
    if(ioctl(*ion_fd, ION_IOC_CUSTOM, &custom_data) < 0)
          GMN_PR_ERR("%s: Cache Invalidate failed\n", __func__);
  }

  return ret;

ION_MAP_FAILED:
  handle_data.handle = ion_info_fd->handle;
  ioctl(*ion_fd, ION_IOC_FREE, &handle_data);
ION_ALLOC_FAILED:
  if (*ion_fd > 0)
    close(*ion_fd);
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

  GMN_DBG ("%s:%d] Open device %s!\n", __func__, __LINE__, pmem_region);
  if (pmem_fd < 0) {
    GMN_DBG ("%s:%d] Open device %s failed!\n", __func__, __LINE__,
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
    GMN_DBG ("%s:%d] failed: %s (%d)\n", __func__, __LINE__,
    strerror (errno), errno);
    addr = NULL;
  }

  GMN_DBG ("%s:%d] pmem_fd %d addr %p size %ld\n", __func__, __LINE__,
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

  GMN_DBG ("%s:%d] pmem_fd %d addr %p size %u rc %d\n", __func__, __LINE__,
    pmem_fd, addr, size, rc);

  return rc;
#endif
}

/*===========================================================================
 * FUNCTION - do_clearcache -
 *
 * DESCRIPTION:
 *==========================================================================*/
int do_clearcache(int pmem_fd,
  void *addr,
  size_t size,
  int ion_fd,
  struct ion_fd_data *ion_info_fd)
{
#ifdef USE_ION
  int rc = 0;
  struct ion_flush_data ion_flash =
  {
    .handle = ion_info_fd->handle,
    .vaddr = addr,
    .offset = 0,
    .length = (size + (getpagesize() - 1)) & (~(getpagesize() - 1))
  };
  rc = ioctl(ion_fd, ION_IOC_INV_CACHES, &ion_flash);
  GMN_DBG ("%s:%d] ion_fd %d addr %p size %u\n", __func__,
      __LINE__, ion_fd, addr, size);
  return rc;
#else
  int rc = 0;

  struct pmem_addr pmemAddr =
  {
    .vaddr = (unsigned long)addr,
    .offset = 0,
    .length = (size + (getpagesize() - 1)) & (~(getpagesize() - 1))
  };
  rc = ioctl(pmem_fd, PMEM_INV_CACHES, &pmemAddr);

  GMN_DBG ("%s:%d] pmem_fd %d addr %p size %u\n",
      __func__, __LINE__, pmem_fd, addr, size);
  return rc;
#endif /* USE_ION */
}


