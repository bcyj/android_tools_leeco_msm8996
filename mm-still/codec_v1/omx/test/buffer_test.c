/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#include "buffer_test.h"
#include <linux/msm_ion.h>

/** buffer_allocate:
 *
 *  Arguments:
 *     @p_buffer: ION buffer
 *
 *  Return:
 *     buffer address
 *
 *  Description:
 *      allocates ION buffer
 *
 **/
void* buffer_allocate(buffer_test_t *p_buffer)
{
  void *l_buffer = NULL;

  int lrc = 0;
  struct ion_handle_data lhandle_data;

   p_buffer->alloc.len = p_buffer->size;
   p_buffer->alloc.align = 4096;
   p_buffer->alloc.flags = 0;
   p_buffer->alloc.heap_id_mask = 0x1 << ION_IOMMU_HEAP_ID;

   p_buffer->ion_fd = open("/dev/ion", O_RDONLY);
   if(p_buffer->ion_fd < 0) {
    QIDBG_ERROR("%s :Ion open failed", __func__);
    goto ION_ALLOC_FAILED;
  }

  /* Make it page size aligned */
  p_buffer->alloc.len = (p_buffer->alloc.len + 4095) & (~4095);
  lrc = ioctl(p_buffer->ion_fd, ION_IOC_ALLOC, &p_buffer->alloc);
  if (lrc < 0) {
    QIDBG_ERROR("%s :ION allocation failed len %d", __func__,
      p_buffer->alloc.len);
    goto ION_ALLOC_FAILED;
  }

  p_buffer->ion_info_fd.handle = p_buffer->alloc.handle;
  lrc = ioctl(p_buffer->ion_fd, ION_IOC_SHARE,
    &p_buffer->ion_info_fd);
  if (lrc < 0) {
    QIDBG_ERROR("%s :ION map failed %s", __func__, strerror(errno));
    goto ION_MAP_FAILED;
  }

  p_buffer->p_pmem_fd = p_buffer->ion_info_fd.fd;

  l_buffer = mmap(NULL, p_buffer->alloc.len, PROT_READ  | PROT_WRITE,
    MAP_SHARED,p_buffer->p_pmem_fd, 0);

  if (l_buffer == MAP_FAILED) {
    QIDBG_ERROR("%s :ION_MMAP_FAILED: %s (%d)", __func__,
      strerror(errno), errno);
    goto ION_MAP_FAILED;
  }
  QIDBG_ERROR("%s:%d] fd %d", __func__, __LINE__, p_buffer->p_pmem_fd);

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
 *     @p_buffer: ION buffer
 *
 *  Return:
 *     buffer address
 *
 *  Description:
 *      deallocates ION buffer
 *
 **/
int buffer_deallocate(buffer_test_t *p_buffer)
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


