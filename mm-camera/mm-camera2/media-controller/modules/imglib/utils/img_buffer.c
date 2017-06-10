/*******************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#include <sys/ioctl.h>
#include "img_buffer.h"
#include <linux/msm_ion.h>

/** buffer_allocate:
 *
 *  Arguments:
 *     @p_buffer: Image buffer
 *
 *  Return:
 *     buffer address
 *
 *  Description:
 *      allocates ION buffer
 *
 **/
void* buffer_allocate(img_buffer_t *p_buffer)
{
  void *l_buffer = NULL;

  int lrc = 0;
  struct ion_handle_data lhandle_data;

   p_buffer->alloc.len = p_buffer->size;
   p_buffer->alloc.align = 4096;
   p_buffer->alloc.flags = ION_FLAG_CACHED;
   p_buffer->alloc.heap_id_mask = 0x1 << ION_IOMMU_HEAP_ID;

   p_buffer->ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
   if(p_buffer->ion_fd < 0) {
    IDBG_ERROR("%s :Ion open failed\n", __func__);
    goto ION_ALLOC_FAILED;
  }

  /* Make it page size aligned */
  p_buffer->alloc.len = (p_buffer->alloc.len + 4095) & (~4095);
  lrc = ioctl(p_buffer->ion_fd, ION_IOC_ALLOC, p_buffer->alloc);
  if (lrc < 0) {
    IDBG_ERROR("%s :ION allocation failed\n", __func__);
    goto ION_ALLOC_FAILED;
  }

  p_buffer->ion_info_fd.handle = p_buffer->alloc.handle;
  lrc = ioctl(p_buffer->ion_fd, ION_IOC_SHARE,
    p_buffer->ion_info_fd);
  if (lrc < 0) {
    IDBG_ERROR("%s :ION map failed %s\n", __func__, strerror(errno));
    goto ION_MAP_FAILED;
  }

  p_buffer->fd = p_buffer->ion_info_fd.fd;

  l_buffer = mmap(NULL, p_buffer->alloc.len, PROT_READ | PROT_WRITE,
    MAP_SHARED, p_buffer->fd, 0);

  if (l_buffer == MAP_FAILED) {
    IDBG_ERROR("%s :ION_MMAP_FAILED: %s (%d)\n", __func__,
      strerror(errno), errno);
    goto ION_MAP_FAILED;
  }

  return l_buffer;

ION_MAP_FAILED:
  lhandle_data.handle = p_buffer->ion_info_fd.handle;
  ioctl(p_buffer->ion_fd, ION_IOC_FREE, &lhandle_data);
  return NULL;
ION_ALLOC_FAILED:
  return NULL;
}

/** buffer_deallocate:
 *
 *  Arguments:
 *     @p_buffer: Image buffer
 *
 *  Return:
 *     Upon successful completion, returns 0. Otherwise, it returns -1
 *
 *  Description:
 *      deallocates ION buffer
 *
 **/
int buffer_deallocate(img_buffer_t *p_buffer)
{
  int lrc = 0;
  int lsize = (p_buffer->size + 4095) & (~4095);

  struct ion_handle_data lhandle_data;
  lrc = munmap(p_buffer->addr, lsize);

  close(p_buffer->ion_info_fd.fd);

  lhandle_data.handle = p_buffer->ion_info_fd.handle;
  ioctl(p_buffer->ion_fd, ION_IOC_FREE, &lhandle_data);

  close(p_buffer->ion_fd);
  return lrc;
}
