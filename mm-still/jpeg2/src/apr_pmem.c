/*****************************************************************************
* Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential. *
*****************************************************************************/

#include "apr_pmem.h"
#include <linux/msm_ion.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <linux/types.h>
#include <linux/android_pmem.h>
#include "adsp_dbg.h"

#include "remote.h"

#ifndef PMEM_GET_PHYS
#define PMEM_GET_PHYS   _IOW('p', 1, unsigned int)
#endif //PMEM_GET_PHYS

#pragma weak  remote_register_buf

static void register_buf(void* buf, int size, int fd) {
  if(remote_register_buf) {
      remote_register_buf(buf, size, fd);
   }
}


/*===========================================================================

Function            : apr_pmem_alloc_ion

Description         : Allocates a physically contiguous memory in ion CP_HEAP

Input parameter(s)  : size Size of the block in bytes

Output parameter(s) : Ion structure to the allocated memory block; otherwise,
          NULL if theallocation failed.

=========================================================================== */

void apr_pmem_alloc_ion_uncached(struct mmap_info_ion *pmapion, uint32_t ionheapid) {
  struct ion_handle_data handle_data;
  struct ion_allocation_data alloc;
  struct ion_client *ion_client;
  struct ion_handle *ion_handle;
  int32_t rc;
  int32_t ion_fd;
  uint32_t ret;
  int32_t  p_pmem_fd;
  struct ion_fd_data ion_info_fd;
  struct ion_fd_data test_fd;
  struct ion_custom_data data;

  ion_fd = open("/dev/ion", O_RDONLY);
  if (ion_fd < 0) {
    CDBG_ERROR("\n apr_pmem_alloc_ion : Open ion device failed");
    pmapion->pVirtualAddr = NULL;
    return;
  }

  alloc.len = pmapion->bufsize;
  alloc.align = 4096;
  alloc.heap_mask = ionheapid;
  if (ionheapid == ION_HEAP(ION_CP_MM_HEAP_ID)) {
    alloc.flags = ION_SECURE;
  } else {
    alloc.flags = 0;
  }

  rc = ioctl(ion_fd, ION_IOC_ALLOC, &alloc);

  if (rc < 0) {
    handle_data.handle =  (pmapion->ion_info_fd).handle;
    CDBG_ERROR("\n apr_pmem_alloc_ion : ION alloc length %d %d", rc,
      alloc.len);
    close(ion_fd);
    pmapion->pVirtualAddr = NULL;
    return;
  } else {
    pmapion->ion_info_fd.handle = alloc.handle;
    rc = ioctl(ion_fd, ION_IOC_SHARE, &(pmapion->ion_info_fd));

    if (rc < 0) {
      CDBG_ERROR("\n apr_pmem_alloc_ion : ION map call failed %d", rc);
      handle_data.handle = pmapion->ion_info_fd.handle;
      ioctl(ion_fd, ION_IOC_FREE, &handle_data);
      close(ion_fd);
      pmapion->pVirtualAddr = NULL;
      return;
    } else {
      p_pmem_fd = pmapion->ion_info_fd.fd;
      ret = (uint32_t)mmap(NULL,
                           alloc.len,
                           PROT_READ  | PROT_WRITE,
                           MAP_SHARED,
                           p_pmem_fd,
                           0);
      if (ret == (uint32_t)MAP_FAILED) {
        CDBG_ERROR("\n apr_pmem_alloc_ion : mmap call failed %d", rc);
        handle_data.handle = (pmapion->ion_info_fd).handle;
        ioctl(ion_fd, ION_IOC_FREE, &handle_data);
        close(ion_fd);
        pmapion->pVirtualAddr = NULL;
        return;
      } else {
        CDBG_ERROR("\n Ion allocation success virtaddr : %u fd %u",
                   (uint32_t)ret,
                   (uint32_t)p_pmem_fd);
        data.cmd = p_pmem_fd;
        pmapion->pVirtualAddr = (void *)ret;
        pmapion->ion_fd = ion_fd;
      }
    }
  }
  return;
}

/*===========================================================================

Function            : apr_pmem_alloc_ion_cached

Description         : Allocates a physically contiguous memory in ion CP_HEAP

Input parameter(s)  : size Size of the block in bytes

Output parameter(s) : Ion structure to the allocated memory block; otherwise,
          NULL if theallocation failed.

=========================================================================== */

void apr_pmem_alloc_ion_cached(struct mmap_info_ion *pmapion, uint32_t ionheapid) {
  struct ion_handle_data handle_data;
  struct ion_allocation_data alloc;
  struct ion_client *ion_client;
  struct ion_handle *ion_handle;
  int32_t rc;
  int32_t ion_fd;
  uint32_t ret;
  int32_t  p_pmem_fd;
  struct ion_fd_data ion_info_fd;
  struct ion_fd_data test_fd;
  struct ion_custom_data data;

  ion_fd = open("/dev/ion", O_RDONLY);
  if (ion_fd < 0) {
    CDBG_ERROR("\n apr_pmem_alloc_ion : Open ion device failed");
    pmapion->pVirtualAddr = NULL;
    return;
  }

  alloc.len = pmapion->bufsize;
  alloc.align = 4096;
  alloc.heap_mask = ionheapid;
  if (ionheapid == ION_HEAP(ION_CP_MM_HEAP_ID)) {
    alloc.flags = ION_SECURE | ION_FLAG_CACHED;
  } else {
    alloc.flags =  ION_FLAG_CACHED;
  }

  rc = ioctl(ion_fd, ION_IOC_ALLOC, &alloc);

  if (rc < 0) {
    handle_data.handle =  (pmapion->ion_info_fd).
      handle;
    CDBG_ERROR("\n apr_pmem_alloc_ion : ION alloc length %d %d", rc,
      alloc.len);
    close(ion_fd);
    pmapion->pVirtualAddr = NULL;
    return;
  } else {
    pmapion->ion_info_fd.handle = alloc.handle;
    rc = ioctl(ion_fd, ION_IOC_SHARE, &(pmapion->ion_info_fd));

    if (rc < 0) {
      CDBG_ERROR("\n apr_pmem_alloc_ion : ION map call failed %d", rc);
      handle_data.handle = pmapion->ion_info_fd.handle;
      ioctl(ion_fd, ION_IOC_FREE, &handle_data);
      close(ion_fd);
      pmapion->pVirtualAddr = NULL;
      return;
    } else {
      p_pmem_fd = pmapion->ion_info_fd.fd;
      ret = (uint32_t)mmap(NULL,
        alloc.len,
        PROT_READ  | PROT_WRITE,
        MAP_SHARED,
        p_pmem_fd,
        0);
      if (ret == (uint32_t)MAP_FAILED) {
        CDBG_ERROR("\n apr_pmem_alloc_ion : mmap call failed %d", rc);
        handle_data.handle = (pmapion->ion_info_fd).handle;
        ioctl(ion_fd, ION_IOC_FREE, &handle_data);
        close(ion_fd);
        pmapion->pVirtualAddr = NULL;
        return;
      } else {
        CDBG_ERROR("\n Ion allocation success virtaddr : %u fd %u",
          (uint32_t)ret,
          (uint32_t)p_pmem_fd);
        data.cmd = p_pmem_fd;
        pmapion->pVirtualAddr = (void *)ret;
        pmapion->ion_fd = ion_fd;
      }
    }
  }
   register_buf(pmapion->pVirtualAddr, alloc.len, p_pmem_fd);
   return;
}

/*===========================================================================

Function            : apr_pmem_free_ion

Description         : Free ion memory

Input parameter(s)  : Ion structure to the allocated memory block

Output parameter(s) : None

=========================================================================== */
void apr_pmem_free_ion(struct mmap_info_ion mapion) {
  int32_t rc = 0;
  struct ion_handle_data handle_data;
  uint32_t bufsize;

  if (mapion.pVirtualAddr == NULL) {
    return;
  }
  register_buf(mapion.pVirtualAddr, mapion.bufsize, -1);
  bufsize = (mapion.bufsize + 4095) & (~4095);
  rc = munmap(mapion.pVirtualAddr, bufsize);
  close(mapion.ion_info_fd.fd);

  handle_data.handle = mapion.ion_info_fd.handle;
  ioctl(mapion.ion_fd, ION_IOC_FREE, &handle_data);
  close(mapion.ion_fd);
}





