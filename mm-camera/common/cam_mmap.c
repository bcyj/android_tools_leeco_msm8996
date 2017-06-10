/*============================================================================
   Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#define ATRACE_TAG ATRACE_TAG_CAMERA
#include <cutils/trace.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include "camera.h"
#include "camera_dbg.h"
#include <sys/time.h>
#include "cam_mmap.h"


#define MM_CAMERA_PROFILE 1
/*===========================================================================
 * FUNCTION    - do_mmap -
 *
 * DESCRIPTION:  retured virtual addresss
 *==========================================================================*/
uint8_t *do_mmap(uint32_t size, int *pmemFd)
{
  void *ret; /* returned virtual address */
  int  pmem_fd = open("/dev/pmem_adsp", O_RDWR|O_SYNC);

  if (pmem_fd < 0) {
    CDBG("do_mmap: Open device /dev/pmem_adsp failed!\n");
    return NULL;
  }

  /* to make it page size aligned */
  size = (size + 4095) & (~4095);

  ret = mmap(NULL,
    size,
    PROT_READ  | PROT_WRITE,
    MAP_SHARED,
    pmem_fd,
    0);

  if (ret == MAP_FAILED) {
    CDBG("do_mmap: pmem mmap() failed: %s (%d)\n", strerror(errno), errno);
    return NULL;
  }

  CDBG("do_mmap: pmem mmap fd %d ptr %p len %u\n", pmem_fd, ret, size);

  *pmemFd = pmem_fd;
  return(uint8_t *)ret;
}

/*===========================================================================
 * FUNCTION    - do_munmap -
 *
 * DESCRIPTION:
 *==========================================================================*/
int do_munmap(int pmem_fd, void *addr, size_t size)
{
  int rc;

  size = (size + 4095) & (~4095);

  CDBG("munmapped size = %d, virt_addr = 0x%p\n",
    size, addr);

  rc = (munmap(addr, size));

  close(pmem_fd);

  CDBG("do_mmap: pmem munmap fd %d ptr %p len %u rc %d\n", pmem_fd, addr,
    size, rc);

  return rc;
}
#ifdef USE_ION
uint8_t *do_mmap_ion(int ion_fd, struct ion_allocation_data *alloc,
  struct ion_fd_data *ion_info_fd, int *mapFd)
{
  ATRACE_BEGIN("Camera:alloc");
  void *ret; /* returned virtual address */
  int rc = 0;
  struct ion_handle_data handle_data;

  /* to make it page size aligned */
  alloc->len = (alloc->len + 4095) & (~4095);
  ALOGD("%s : alloc: E size=%d", __func__, alloc->len);
#ifdef TARGET_7x27A
  alloc->flags = ION_HEAP(CAMERA_ION_HEAP_ID);
#endif
  rc = ioctl(ion_fd, ION_IOC_ALLOC, alloc);
  if (rc < 0) {
    CDBG_ERROR("ION allocation failed\n");
    goto ION_ALLOC_FAILED;
  }

  ion_info_fd->handle = alloc->handle;
  rc = ioctl(ion_fd, ION_IOC_SHARE, ion_info_fd);
  if (rc < 0) {
    CDBG_ERROR("ION map failed %s\n", strerror(errno));
    goto ION_MAP_FAILED;
  }
  *mapFd = ion_info_fd->fd;
  ret = mmap(NULL,
    alloc->len,
    PROT_READ  | PROT_WRITE,
    MAP_SHARED,
    *mapFd,
    0);

  if (ret == MAP_FAILED) {
    CDBG_ERROR("ION_MMAP_FAILED: %s (%d)\n", strerror(errno), errno);
    goto ION_MAP_FAILED;
  }

  ATRACE_END();
  ALOGD("%s : alloc: X", __func__);
  return ret;

ION_MAP_FAILED:
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
ION_ALLOC_FAILED:
  ATRACE_END();
  return NULL;
}

int do_munmap_ion (int ion_fd, struct ion_fd_data *ion_info_fd,
                   void *addr, size_t size)
{
  int rc = 0;
  rc = munmap(addr, size);
  close(ion_info_fd->fd);

  struct ion_handle_data handle_data;
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
  return rc;
}
#endif
/*============================================================
   FUNCTION cam_dump_yuv
   DESCRIPTION this function dumps the yuv 4:2:0 frame with
               Y and CbCr in continuous in memory
==============================================================*/
void cam_dump_yuv(void *addr, uint32_t size, char *filename)
{
  int file_fd = open(filename, O_RDWR | O_CREAT, 0777);

  if (file_fd < 0)
    CDBG_HIGH("%s: cannot open file\n", __func__);
  else
    write(file_fd, addr, size * 3/2);

  close(file_fd);
}

/*============================================================
   FUNCTION cam_dump_yuv2
   DESCRIPTION this function dumps the yuv 4:2:0 frame with
               Y and CbCr in non-continuous memory
==============================================================*/
void cam_dump_yuv2(void *yaddr, void *cbcraddr, uint32_t size,
                   char *filename)
{
  int file_fd = open(filename, O_RDWR | O_CREAT, 0777);

  if (file_fd < 0)
    CDBG_HIGH("%s: cannot open file\n", __func__);
  else {
    write(file_fd, yaddr, size);
    write(file_fd, cbcraddr, size/2);
  }

  close(file_fd);
}

void mmcamera_util_profile(const char *str)
{
  #if (MM_CAMERA_PROFILE)
  struct timespec cur_time;

  clock_gettime(CLOCK_REALTIME, &cur_time);
  CDBG_HIGH("PROFILE %s: %ld.%09ld\n", str,
    cur_time.tv_sec, cur_time.tv_nsec);
  #endif
}
