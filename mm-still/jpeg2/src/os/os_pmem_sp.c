/*========================================================================

*//** @file os_pmem.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009,2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/11/09   vma     Removed 2^n restriction on PMEM (clp2)
05/12/09   vma     Created file.

========================================================================== */

#include "os_pmem.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include <string.h>
#include <linux/msm_ion.h>

#define USE_ION

#define ALIGN(x, a) {x = (x + a - 1) & (~(a-1));}

int os_pmem_fd_open(pmem_fd_t *p_pmem_fd, int *ion_fd, struct ion_allocation_data *alloc,
  struct ion_fd_data *ion_info_fd)
{
#ifdef USE_ION
  int rc = 0;
  struct ion_handle_data handle_data;
  *ion_fd = open("/dev/ion", O_RDONLY);
  if(*ion_fd < 0) {
    JPEG_DBG_HIGH("Ion open failed\n");
    return JPEGERR_EFAILED;
  }

  /* to make it page size aligned */
  alloc->len = (alloc->len + 4095) & (~4095);
  rc = ioctl(*ion_fd, ION_IOC_ALLOC, alloc);
  if (rc < 0) {
    JPEG_DBG_HIGH("ION allocation failed\n");
    return JPEGERR_EFAILED;
  }

  ion_info_fd->handle = alloc->handle;
  rc = ioctl(*ion_fd, ION_IOC_SHARE, ion_info_fd);
  if (rc < 0) {
    JPEG_DBG_HIGH("ION map failed %s\n", strerror(errno));
    return JPEGERR_EFAILED;
  }
  *p_pmem_fd = ion_info_fd->fd;
  return JPEGERR_SUCCESS;
#else
    if (!p_pmem_fd)
        return JPEGERR_EFAILED;
    char* pmem_region;
#ifdef SMIPOOL_AVAILABLE
    pmem_region = "/dev/pmem_smipool";
#else
    pmem_region = "/dev/pmem_adsp";
#endif

    JPEG_DBG_LOW("%s: open pmem_region %s", __func__, pmem_region);
    *p_pmem_fd = open(pmem_region, O_RDWR|O_SYNC);
    if (*p_pmem_fd < 0)
    {
        JPEG_DBG_HIGH("os_pmem_fd_open: failed to open pmem (%d - %s)\n",
                       errno, strerror(errno));
        return JPEGERR_EFAILED;
    }
    return JPEGERR_SUCCESS;
#endif
}

int os_pmem_fd_close(pmem_fd_t *p_pmem_fd, int ion_fd,
   struct ion_fd_data *ion_info_fd)
{
#ifdef USE_ION
  if (!p_pmem_fd || *p_pmem_fd < 0)
    return JPEGERR_EFAILED;
  close(ion_info_fd->fd);
  struct ion_handle_data handle_data;
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
  close(ion_fd);
  *p_pmem_fd = OS_PMEM_FD_INIT_VALUE;
  return JPEGERR_SUCCESS;
#else
    if (!p_pmem_fd || *p_pmem_fd < 0)
        return JPEGERR_EFAILED;

    (void)close(*p_pmem_fd);
    *p_pmem_fd = OS_PMEM_FD_INIT_VALUE;
    return JPEGERR_SUCCESS;
#endif
}

int os_pmem_allocate(pmem_fd_t pmem_fd, uint32_t size, uint8_t **p_vaddr)
{
    if (pmem_fd <= 0 || !p_vaddr)
        return JPEGERR_EFAILED;

    ALIGN(size, PAGESIZE);
    *p_vaddr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                    MAP_SHARED, pmem_fd, 0);
    if (*p_vaddr == MAP_FAILED)
    {
       return JPEGERR_EMALLOC;
    }
    return JPEGERR_SUCCESS;
}

int os_pmem_free(pmem_fd_t pmem_fd, uint32_t size, uint8_t *vaddr)
{
    if (pmem_fd <= 0 || !vaddr)
        return JPEGERR_EFAILED;

    ALIGN(size, PAGESIZE);
    return munmap(vaddr, size);
}

int os_pmem_get_phy_addr(pmem_fd_t pmem_fd, uint8_t **p_paddr)
{
#ifdef USE_ION
    return JPEGERR_SUCCESS;
#else
    if (pmem_fd <= 0 || !p_paddr)
        return JPEGERR_EFAILED;

    if (ioctl(pmem_fd, PMEM_GET_PHYS, p_paddr) < 0)
        return JPEGERR_EFAILED;

    return JPEGERR_SUCCESS;
#endif
}
