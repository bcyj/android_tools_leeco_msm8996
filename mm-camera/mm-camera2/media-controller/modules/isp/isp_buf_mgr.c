/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "isp_buf_mgr.h"
#include "mct_stream.h"
#include "isp_log.h"
#include "server_debug.h"
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#if 0
#undef CDBG
#define CDBG ALOGE
#endif

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

#define ISP_BUFQ_HANDLE_SHIFT 16
#define ISP_BUFQ_INDEX(handle) ((handle) & 0xFFFF)
#define ISP_GET_BUFQ(mgr, handle)  \
  ( (ISP_BUFQ_INDEX(handle) < ISP_MAX_NUM_BUF_QUEUE) ? \
  (&mgr->bufq[ISP_BUFQ_INDEX(handle)]) : NULL )

typedef struct {
  isp_frame_buffer_t *isp_map_buf;
  int cnt;
} find_map_buf_t;

static int isp_queue_buf_int(isp_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx, uint32_t dirty_buf, int vfe_fd);

/** isp_do_mmap_ion
 *
 * DESCRIPTION:
 *
 **/
static uint8_t *isp_do_mmap_ion(int ion_fd, struct ion_allocation_data *alloc,
  struct ion_fd_data *ion_info_fd, int *mapFd)
{
  void *ret; /* returned virtual address */
  int rc = 0;
  struct ion_handle_data handle_data;

  /* to make it page size aligned */
  alloc->len = (alloc->len + 4095) & (~4095);
  rc = ioctl(ion_fd, ION_IOC_ALLOC, alloc);
  if (rc < 0) {
    CDBG_ERROR("%s: ION allocation failed\n", __func__);
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

  return ret;

ION_MAP_FAILED:
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);

ION_ALLOC_FAILED:
  return NULL;
}

/** isp_do_munmap_ion
 *
 * DESCRIPTION:
 *
 **/
static int isp_do_munmap_ion (int ion_fd, struct ion_fd_data *ion_info_fd,
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

/** isp_init_native_buffer
 *
 * DESCRIPTION:
 *
 **/
int isp_init_native_buffer(isp_frame_buffer_t *buf, int buf_idx,
  int ion_fd, cam_frame_len_offset_t *len_offset, int cached)
{
  int current_fd = -1;
  uint32_t i;
  unsigned long current_addr = 0;

  memset(buf, 0, sizeof(isp_frame_buffer_t));
  buf->buffer.m.planes = &buf->planes[0];

  /* now we only use contigous memory.
   * We could use per plane memory */
  buf->ion_alloc[0].len = len_offset->frame_len;
  if (cached) {
    buf->ion_alloc[0].flags = ION_FLAG_CACHED;
  } else {
    buf->ion_alloc[0].flags = 0;
  }

  buf->ion_alloc[0].heap_id_mask = 0x1 << ION_IOMMU_HEAP_ID;
  buf->ion_alloc[0].align = 4096;
  current_addr = (unsigned long) isp_do_mmap_ion(ion_fd,
    &(buf->ion_alloc[0]), &(buf->fd_data[0]), &current_fd);
  if (current_addr == 0) {
    CDBG_ERROR("%s: ION allocation no mem\n", __func__);
    return -1;
  }
  buf->vaddr = (void *)current_addr;

  for (i = 0; i < len_offset->num_planes; i++) {
    buf->buffer.m.planes[i].m.userptr = current_fd;
    buf->buffer.m.planes[i].data_offset = len_offset->mp[i].offset;
    buf->buffer.m.planes[i].length = len_offset->mp[i].len;
    buf->addr[0] = current_addr;
  }

  buf->buffer.length = len_offset->num_planes;
  buf->buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  buf->buffer.index = buf_idx;
  buf->buffer.memory = V4L2_MEMORY_USERPTR;
  buf->fd = current_fd;
  buf->cached = cached;
  return 0;
}

/** isp_do_cache_inv_ion
 *
 * DESCRIPTION:
 *
 **/
int isp_do_cache_inv_ion(int ion_fd, isp_frame_buffer_t *image_buf)
{
  struct ion_flush_data cache_inv_data;
  struct ion_custom_data custom_data;
  int ret = 0;

  memset(&cache_inv_data, 0, sizeof(cache_inv_data));
  memset(&custom_data, 0, sizeof(custom_data));
  cache_inv_data.vaddr = image_buf->vaddr;
  cache_inv_data.fd = image_buf->fd;
  cache_inv_data.handle = image_buf->ion_alloc[0].handle;
  cache_inv_data.length = image_buf->ion_alloc[0].len;
  custom_data.cmd = ION_IOC_CLEAN_INV_CACHES;
  custom_data.arg = (unsigned long)&cache_inv_data;

  ret = ioctl(ion_fd, ION_IOC_CUSTOM, &custom_data);

  return ret;
}

/** isp_deinit_native_buffer
 *
 * DESCRIPTION:
 *
 **/
void isp_deinit_native_buffer(isp_frame_buffer_t *buf, int ion_fd)
{
  int i;

  if (buf->fd <= 0)
    return;
  for (i = 0; i < (int)buf->buffer.length; i++) {
    if (buf->ion_alloc[i].len != 0) {
      isp_do_munmap_ion(ion_fd, &(buf->fd_data[0]),
        (void *)buf->addr[i], buf->ion_alloc[i].len);
    }
  }
  memset(buf, 0, sizeof(isp_frame_buffer_t));
}

/** isp_generate_new_bufq_handle
 *
 * DESCRIPTION:
 *
 **/
static int isp_generate_new_bufq_handle(isp_buf_mgr_t *buf_mgr, int index)
{
  buf_mgr->bufq_handle_count &= 0x0000FFFF;
  if (buf_mgr->bufq_handle_count == 0)
    buf_mgr->bufq_handle_count = 1;
  return buf_mgr->bufq_handle_count++ << ISP_BUFQ_HANDLE_SHIFT | index;
}

/** isp_get_bufq_handle
 *
 * DESCRIPTION:
 *
 **/
static uint32_t isp_get_bufq_handle(isp_buf_mgr_t *buf_mgr)
{
  int i;

  for (i = 0; i < ISP_MAX_NUM_BUF_QUEUE; i++) {
    if (!buf_mgr->bufq[i].used)
      break;
  }

  if (i == ISP_MAX_NUM_BUF_QUEUE) {
    CDBG_ERROR("%s: No free buf queue\n", __func__);
    return 0;
  }

  pthread_mutex_lock(&buf_mgr->bufq[i].mutex);
  buf_mgr->bufq[i].used = 1;
  buf_mgr->bufq[i].user_bufq_handle = isp_generate_new_bufq_handle(buf_mgr, i);
  pthread_mutex_unlock(&buf_mgr->bufq[i].mutex);
  return buf_mgr->bufq[i].user_bufq_handle;
}

/** isp_free_bufq_handle
 *
 * DESCRIPTION:
 *
 **/
static void isp_free_bufq_handle(isp_buf_mgr_t *buf_mgr, isp_bufq_t *bufq)
{
  bufq->user_bufq_handle = 0;
  bufq->kernel_bufq_handle = 0;
  bufq->session_id = 0;
  bufq->stream_id = 0;
  bufq->current_num_buffer = 0;
  bufq->total_num_buffer = 0;
  bufq->open_cnt = 0;
  bufq->use_native_buf = 0;
  bufq->buf_type = MAX_ISP_BUF_TYPE;
  buf_mgr->bufq_handle_count--;
  memset(bufq->image_buf, 0, sizeof(isp_frame_buffer_t)*ISP_MAX_IMG_BUF);
  bufq->used = 0;
  memset(bufq->vfe_fds, 0, sizeof(int)*2) ;
  bufq->num_vfe_fds = 0;
}

/** isp_init_hal_buffer
 *
 * DESCRIPTION:
 *
 **/
static boolean isp_init_hal_buffer(void *data, void *user_data)
{
  uint32_t i;
  find_map_buf_t *map_buf = (find_map_buf_t *)user_data;
  mct_stream_map_buf_t *img_buf = (mct_stream_map_buf_t *)data;
  struct v4l2_buffer *v4l2_buf = NULL;

  v4l2_buf = &map_buf->isp_map_buf[map_buf->cnt].buffer;
  map_buf->isp_map_buf[map_buf->cnt].vaddr = img_buf->buf_planes[0].buf;
  v4l2_buf->m.planes = map_buf->isp_map_buf[map_buf->cnt].planes;

  for (i = 0; i < img_buf->num_planes; i++) {
    v4l2_buf->m.planes[i].m.userptr = img_buf->buf_planes[i].fd;
    v4l2_buf->m.planes[i].data_offset = img_buf->buf_planes[i].offset;
    v4l2_buf->m.planes[i].length = img_buf->buf_planes[i].size;
    map_buf->isp_map_buf[map_buf->cnt].addr[i] =
      (unsigned long)img_buf->buf_planes[i].buf;
  }

  v4l2_buf->length = img_buf->num_planes;
  v4l2_buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  v4l2_buf->index = img_buf->buf_index;
   v4l2_buf->memory = V4L2_MEMORY_USERPTR;
  map_buf->isp_map_buf[map_buf->cnt].is_reg = FALSE;
  map_buf->cnt++;

  return TRUE;
}

/** isp_validate_buf_request
 *
 * DESCRIPTION:
 *
 **/
static int isp_validate_buf_request(isp_buf_mgr_t *buf_mgr,
    isp_buf_request_t *buf_request, int bufq_handle)
{
  int i;
  isp_bufq_t *bufq = ISP_GET_BUFQ(buf_mgr, bufq_handle);
  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }
  ISP_DBG(ISP_MOD_COM,"%s: bufq %p, bufq_handle %x, stream_id %d,"
    "img_buf_list %p, buf_mgr %p\n", __func__, bufq, bufq_handle,
    buf_request->stream_id, buf_request->img_buf_list, buf_mgr);

  if (!buf_request->use_native_buf && !buf_request->img_buf_list) {
    CDBG_ERROR("%s: No mct buf list\n", __func__);
    return -1;
  }

  if (!buf_request->use_native_buf) {
    find_map_buf_t map_buf;
    /*Initially num_buffer would be zero, but when HAL updates
      buf_list dynamically after streamon, if MCT sends only
      delta of buf list need to pupolate ISP image_buf array
      only for additional buffers*/
    pthread_mutex_lock(&bufq->mutex);
    map_buf.cnt = bufq->current_num_buffer;
    map_buf.isp_map_buf = bufq->image_buf;
    mct_list_traverse(buf_request->img_buf_list,
      isp_init_hal_buffer, (void *)&map_buf);
    pthread_mutex_unlock(&bufq->mutex);
    buf_request->current_num_buf = map_buf.cnt;
    ISP_DBG(ISP_MOD_COM,"%s: old count %d new count %d\n", __func__, bufq->current_num_buffer,
      buf_request->current_num_buf);
  }

  if (buf_request->current_num_buf == 0 ||
      buf_request->current_num_buf > ISP_MAX_IMG_BUF) {
    CDBG_ERROR("%s: Invalid number of buffer\n", __func__);
    return -1;
  }

  pthread_mutex_lock(&bufq->mutex);
  buf_request->buf_handle = bufq_handle;
  bufq->session_id = buf_request->session_id;
  bufq->stream_id = buf_request->stream_id;
  bufq->current_num_buffer = buf_request->current_num_buf;
  if (buf_request->total_num_buf > 0)
    bufq->total_num_buffer = buf_request->total_num_buf;
  else if (bufq->total_num_buffer == 0) {
    /*If HAL has not passed num_bufs information, use the
      count of buffers passed in im_buf_list as total bufs*/
    bufq->total_num_buffer = bufq->current_num_buffer;
  }
  bufq->buf_type = buf_request->buf_type;
  bufq->use_native_buf = buf_request->use_native_buf;

  CDBG_HIGH("%s: total %d current buffer count %d for stream %d\n",
    __func__, bufq->total_num_buffer, bufq->current_num_buffer,
    bufq->stream_id);
  pthread_mutex_unlock(&bufq->mutex);
  /* If totalnumber of buffer are less than or equal to zero,
  with stream buffer configuration*/
  assert(bufq->total_num_buffer > 0);
  return 0;
}

/** isp_request_kernel_bufq
 *
 * DESCRIPTION:
 *
 **/
static int isp_request_kernel_bufq(isp_buf_mgr_t *buf_mgr,
  isp_bufq_t *bufq, int vfe_fd)
{
  int rc = 0;
  struct msm_isp_buf_request bufq_request;
  bufq_request.session_id = bufq->session_id;
  bufq_request.stream_id = bufq->stream_id;
  bufq_request.num_buf = bufq->total_num_buffer;
  bufq_request.buf_type = bufq->buf_type;

  ISP_DBG(ISP_MOD_COM,"%s: stream_id %d total_num_buf %d current %d\n", __func__,
    bufq_request.stream_id, bufq_request.num_buf, bufq->current_num_buffer);
  rc = ioctl(vfe_fd, VIDIOC_MSM_ISP_REQUEST_BUF, &bufq_request);
  if (rc < 0 || !bufq_request.handle) {
    CDBG_ERROR("%s: kernel request buf failed\n", __func__);
    rc = -1;
    goto end;
  }

  bufq->kernel_bufq_handle = bufq_request.handle;

end:
  return rc;
}

/** isp_release_kernel_bufq
 *
 * DESCRIPTION:
 *
 **/
static void isp_release_kernel_bufq(isp_buf_mgr_t *buf_mgr,
  isp_bufq_t *bufq, int vfe_fd)
{
  struct msm_isp_buf_request bufq_release;
  bufq_release.handle = bufq->kernel_bufq_handle;
  ioctl(vfe_fd, VIDIOC_MSM_ISP_RELEASE_BUF, &bufq_release);

  return;
}


/** isp_queue_buf_list_update
 *    @buf_mgr: buf manager
 *    @bufq: buffer queue
 *    @vfe_fd : hw id
 *
 *  HAL has updated buffer list. Now we have to update and
 *  register new buffers to kernel.
 *
 *  This function is called in MCT thread context
 *
 *  Returns 0 for success and negative error for failure
 **/
static int isp_queue_buf_list_update(isp_buf_mgr_t *buf_mgr, isp_bufq_t *bufq,
  int vfe_fd)
{
  int rc = 0, i;
  uint32_t dirty_buf = 0;

  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  /*if vfe_fd is zero update buffer list to any of available hw*/
  pthread_mutex_lock(&bufq->mutex);
  if (vfe_fd <= 0) {
    if (bufq->vfe_fds[0] > 0)
      vfe_fd = bufq->vfe_fds[0];
    else if (bufq->vfe_fds[1] > 0)
      vfe_fd= bufq->vfe_fds[1];
    else {
      CDBG_ERROR("%s:#%d vfe_fd %d, vfe_fd[0] %d vfe_fd[1] %d \n",
        __func__, __LINE__, vfe_fd, bufq->vfe_fds[0], bufq->vfe_fds[1]);

      pthread_mutex_unlock(&bufq->mutex);
      return -1;
    }
  }

  /*Re-iterate through entire image_buf array and enque the newly added
    buffers to kernel*/
  for (i = 0; i < bufq->current_num_buffer; i++) {
    ISP_DBG(ISP_MOD_COM,"%s: buffer %lx buf_idx %d is reg = %d\n", __func__,
      bufq->image_buf[i].buffer.m.userptr, i, bufq->image_buf[i].is_reg);
    if (!bufq->image_buf[i].is_reg) {
      rc = isp_queue_buf_int(buf_mgr, bufq->user_bufq_handle, i, dirty_buf,
        vfe_fd);
      if (rc < 0) {
        CDBG_ERROR("%s: buffer enque to kernel failed rc = %d\n", __func__, rc);
        rc = -1;
        goto end;
      }
    }
  }
end:
  pthread_mutex_unlock(&bufq->mutex);
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d\n", __func__, rc);
  return rc;
}

/** isp_queue_buf_all
 *    @buf_mgr: buf manager
 *    @bufq: buffer queue
 *    @vfe_fd : hw id
 *
 *  Queue all buffers to kernel
 *
 * Returns 0 for success and negative error for failure
 *
 **/
static int isp_queue_buf_all(isp_buf_mgr_t *buf_mgr,
  isp_bufq_t *bufq, int vfe_fd)
{
  int rc, i;
  uint32_t dirty_buf = 0;

  for (i = 0; i < bufq->current_num_buffer; i++) {
    rc = isp_queue_buf_int(buf_mgr, bufq->user_bufq_handle, i, dirty_buf,
      vfe_fd);
    if (rc < 0)
      return rc;
  }

  return 0;
}

/** isp_copy_planes_from_v4l2_buffer
 *    @qbuf_buf: target isp qbuf buffer
 *    @v4l2_buf: source v4l2 buffer
 *
 * Copies planes info from V4L2 buffer to isp queue buffer
 *
 * Return: None
 *
 **/
static void isp_copy_planes_from_v4l2_buffer(
  struct msm_isp_qbuf_buffer *qbuf_buf, const struct v4l2_buffer *v4l2_buf)
{
  unsigned int i;
  qbuf_buf->num_planes = v4l2_buf->length;
  for (i = 0; i < qbuf_buf->num_planes; i++) {
    qbuf_buf->planes[i].addr = v4l2_buf->m.planes[i].m.userptr;
    qbuf_buf->planes[i].offset = v4l2_buf->m.planes[i].data_offset;
  }
}

/** isp_queue_buf_int
 *
 * DESCRIPTION:
 *
 **/
static int isp_queue_buf_int(isp_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx, uint32_t dirty_buf, int vfe_fd)
{
  int rc;
  struct msm_isp_qbuf_info qbuf_info;
  isp_bufq_t *bufq = ISP_GET_BUFQ(buf_mgr, bufq_handle);

  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }

  if (bufq->image_buf[buf_idx].cached) {
    /* if the buffer is cached it needs to be invalidated first */
    isp_do_cache_inv_ion(buf_mgr->ion_fd, &bufq->image_buf[buf_idx]);
  }

  qbuf_info.handle = bufq->kernel_bufq_handle;
  qbuf_info.buf_idx = buf_idx;
  isp_copy_planes_from_v4l2_buffer(&qbuf_info.buffer,
    &(bufq->image_buf[buf_idx].buffer));
  qbuf_info.dirty_buf = dirty_buf;

  rc = ioctl(vfe_fd, VIDIOC_MSM_ISP_ENQUEUE_BUF, &qbuf_info);
  if (rc < 0) {
    CDBG_ERROR("%s: queue buf to kernel failed\n", __func__);
    return -1;
  }

  bufq->image_buf[buf_idx].is_reg = TRUE;
  return 0;
}

/** isp_queue_buf
 *
 * DESCRIPTION:
 *
 **/
int isp_queue_buf(isp_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx, uint32_t dirty_buf, int vfe_fd)
{
  int rc = 0;
  struct msm_isp_qbuf_info qbuf_info;
  isp_bufq_t *bufq = ISP_GET_BUFQ(buf_mgr, bufq_handle);

  if (!bufq) {
    CDBG_ERROR("%s: error, bufq is NULL\n", __func__);
    return -1;
  }
  /* Lock since bufq is accessed and updated here*/
  pthread_mutex_lock(&bufq->mutex);
  if (vfe_fd <= 0) {
    if (bufq->vfe_fds[0] > 0)
      vfe_fd = bufq->vfe_fds[0];
    else if (bufq->vfe_fds[1] > 0)
      vfe_fd= bufq->vfe_fds[1];
    else {
      CDBG_ERROR("%s: error, do not have VFE fd\n", __func__);
      rc = -1;
      goto end;
    }
  }

  rc = isp_queue_buf_int(buf_mgr, bufq_handle, buf_idx, dirty_buf, vfe_fd);

end:
  pthread_mutex_unlock(&bufq->mutex);
  return rc;
}

/** isp_get_buf_addr
 *
 * DESCRIPTION:
 *
 **/
void *isp_get_buf_addr(isp_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx)
{
  int rc = 0;
  isp_bufq_t *bufq = ISP_GET_BUFQ(buf_mgr, bufq_handle);

  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return NULL;
  }

  pthread_mutex_lock(&bufq->mutex);
  if (bufq->used && bufq->total_num_buffer > buf_idx) {
    pthread_mutex_unlock(&bufq->mutex);
    return bufq->image_buf[buf_idx].vaddr;
  }
  pthread_mutex_unlock(&bufq->mutex);

  return NULL;
}

/** isp_get_buf_by_idx
 *
 * DESCRIPTION:
 *
 **/
isp_frame_buffer_t *isp_get_buf_by_idx(isp_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx)
{
  int rc = 0;
  isp_bufq_t *bufq = ISP_GET_BUFQ(buf_mgr, bufq_handle);
  isp_frame_buffer_t *image_buf = NULL;

  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return NULL;
  }

  pthread_mutex_lock(&bufq->mutex);
  if (bufq->used && bufq->total_num_buffer > buf_idx) {
    image_buf = &bufq->image_buf[buf_idx];
  }
  pthread_mutex_unlock(&bufq->mutex);

  return image_buf;
}

/** isp_find_matched_bufq_handle
 *
 * DESCRIPTION:
 *
 **/
uint32_t isp_find_matched_bufq_handle(isp_buf_mgr_t *buf_mgr,
  uint32_t session_id, uint32_t stream_id)
{
  int i;
  isp_bufq_t *tmp_bufq;
  uint32_t bufq_handle = 0;

  pthread_mutex_lock(&buf_mgr->mutex);
  for (i = 0; i < ISP_MAX_NUM_BUF_QUEUE; i++) {
    tmp_bufq = &buf_mgr->bufq[i];
    if(tmp_bufq->used &&
       tmp_bufq->session_id == session_id &&
       tmp_bufq->stream_id == stream_id){
      bufq_handle = tmp_bufq->user_bufq_handle;
      break;
    }
  }
  pthread_mutex_unlock(&buf_mgr->mutex);

  return bufq_handle;
}

/** isp_save_vfe_fd
 *
 * DESCRIPTION:
 *
 **/
static int isp_save_vfe_fd(isp_bufq_t *bufq, int vfe_fd)
{
  int rc = 0;
  if (bufq->vfe_fds[0] == 0 || bufq->vfe_fds[0] == vfe_fd)
    bufq->vfe_fds[0] = vfe_fd;
  else if (bufq->vfe_fds[1] == 0 || bufq->vfe_fds[1] == vfe_fd)
    bufq->vfe_fds[1] = vfe_fd;
  else {
    rc = -1;
    CDBG_ERROR("%s:#%d vfe_fd %d, vfe_fd[0] %d vfe_fd[1] %d \n",
      __func__, __LINE__, vfe_fd, bufq->vfe_fds[0], bufq->vfe_fds[1]);
    goto end;
  }
  bufq->num_vfe_fds++;

end:
  return rc;
}

/** isp_remove_vfe_fd
 *
 * DESCRIPTION:
 *
 **/
static int isp_remove_vfe_fd(isp_bufq_t *bufq, int vfe_fd)
{
  int rc = 0;
  if (vfe_fd <= 0) {
    CDBG_ERROR("%s: invalid vfe_fd = %d\n", __func__, vfe_fd);
    return -1;
  }
  if (bufq->vfe_fds[0] == vfe_fd)
    bufq->vfe_fds[0] = 0;
  else if (bufq->vfe_fds[1] == vfe_fd)
    bufq->vfe_fds[1] = 0;
  else {
    rc = -1;
    CDBG_ERROR("%s:#%d vfe_fd %d, vfe_fd[0] %d vfe_fd[1] %d \n",
      __func__, __LINE__, vfe_fd, bufq->vfe_fds[0], bufq->vfe_fds[1]);
    goto end;
  }

  bufq->num_vfe_fds--;

end:
  return rc;
}

/** isp_register_buf_list_update
 *    @buf_mgr: buf manager
 *    @bufq_handle: buffer queue
 *    @buf_request: cotain info about buf_mgr and bufq
 *    @vfe_fd : hw id
 *
 *  HAL has updated buffer list. Now we have to update and
 *  register new buffers to kernel.
 *
 *  This function runs in MCT thread context
 *
 *  Returns 0 for success and negative error for failure
 **/
int isp_register_buf_list_update(isp_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, isp_buf_request_t *buf_request, int vfe_fd)
{
  int i = 0;
  isp_bufq_t *bufq;
  int rc = 0;

  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  bufq = ISP_GET_BUFQ(buf_mgr, bufq_handle);
  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }

  /*Validate buf request and populate info such as updated num_buffer*/
  rc = isp_validate_buf_request(buf_mgr, buf_request, bufq_handle);
  if (rc < 0) {
    CDBG_ERROR("%s: validate_buf failed\n", __func__);
    goto queue_buf_error;
  }

  /*Enqueue additional buffers to kernel*/
  rc = isp_queue_buf_list_update(buf_mgr, bufq, vfe_fd);
  if (rc < 0) {
    CDBG_ERROR("%s: cannot enqueue additional bufs\n", __func__);
    goto queue_buf_error;
  }

queue_buf_error:
  ISP_DBG(ISP_MOD_COM,"%s: X,rc = %d\n", __func__, rc);
  return rc;
}

/** isp_register_buf
 *    @buf_mgr: buf manager
 *    @bufq_handle: buffer queue
 *    @vfe_fd : hw id
 *
 *  Buffer list is received , here we register new buffers to
 *  kernel.
 *
 *  Returns 0 for success and negative error for failure
 **/
int isp_register_buf(isp_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int vfe_fd)
{
  int i = 0;
  isp_bufq_t *bufq;

  int rc = 0;

  bufq = ISP_GET_BUFQ(buf_mgr, bufq_handle);
  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }

  pthread_mutex_lock(&bufq->mutex);
  if (isp_save_vfe_fd(bufq, vfe_fd) < 0) {
    CDBG_ERROR("%s: cannot save vfe_fd %d with handle 0x%x\n",
      __func__, vfe_fd, bufq_handle);
    pthread_mutex_unlock(&bufq->mutex);
    return -1;
  }

  if (bufq->num_vfe_fds > 1) {
    pthread_mutex_unlock(&bufq->mutex);
    return 0;
  }

  if (bufq->use_native_buf)
    bufq->stream_id = bufq->stream_id | ISP_NATIVE_BUF_BIT;


  /*Request kernel buffer queue*/
  rc = isp_request_kernel_bufq(buf_mgr, bufq, vfe_fd);
  if (rc < 0)
    goto request_buf_error;

  /*Enqueue buffer to kernel*/
  rc = isp_queue_buf_all(buf_mgr, bufq, vfe_fd);
  if (rc < 0)
    goto queue_buf_error;

  pthread_mutex_unlock(&bufq->mutex);
  return 0;

queue_buf_error:
  isp_release_kernel_bufq(buf_mgr, bufq, vfe_fd);

request_buf_error:
  pthread_mutex_unlock(&bufq->mutex);

  return rc;
}

/** isp_request_buf
 *
 * DESCRIPTION:
 *
 **/
int isp_request_buf(isp_buf_mgr_t *buf_mgr, isp_buf_request_t *buf_request)
{
  int i = 0;
  isp_bufq_t *bufq;
  uint32_t bufq_handle;
    int rc = 0;

  bufq_handle = isp_find_matched_bufq_handle(buf_mgr,
    buf_request->session_id, buf_request->stream_id);
  if (bufq_handle > 0) {
    /* buf queue already requested. */
    buf_request->buf_handle = bufq_handle;
    return 0;
  }

  pthread_mutex_lock(&buf_mgr->mutex);
  /*Get new userspace buffer queue from buf mgr*/
  bufq_handle = isp_get_bufq_handle(buf_mgr);
  /* already set the used bit in bufq, no lock is needed */
  pthread_mutex_unlock(&buf_mgr->mutex);
  if (bufq_handle == 0) {
    return -1;
  }

  /*Validate buf request and populate info*/
  rc = isp_validate_buf_request(buf_mgr,
    buf_request, bufq_handle);

  if (rc < 0)
    return rc;

  /*Allocate buffer for native buffer or
    populate buf info from HAL mapped buffer */
  bufq = ISP_GET_BUFQ(buf_mgr, bufq_handle);
  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }

  pthread_mutex_lock(&bufq->mutex);
  if (bufq->use_native_buf) {
    for (i = 0; i < buf_request->total_num_buf; i++) {
      rc = isp_init_native_buffer(&bufq->image_buf[i],
        i, buf_mgr->ion_fd, &buf_request->buf_info, buf_request->cached);
      if (rc < 0) {
        pthread_mutex_unlock(&bufq->mutex);
        goto native_buf_error;
      }
    }
  }
  pthread_mutex_unlock(&bufq->mutex);
  return 0;

native_buf_error:
  pthread_mutex_lock(&bufq->mutex);
  if (bufq->use_native_buf) {
    for (i--; i >= 0; i--)
      isp_deinit_native_buffer(&bufq->image_buf[i], buf_mgr->ion_fd);
  }
  pthread_mutex_unlock(&bufq->mutex);

  pthread_mutex_lock(&buf_mgr->mutex);
  isp_free_bufq_handle(buf_mgr, bufq);
  pthread_mutex_unlock(&buf_mgr->mutex);

  return rc;
}

/** isp_unregister_buf
 *
 * DESCRIPTION:
 *
 **/
int isp_unregister_buf(isp_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int vfe_fd)
{
  isp_bufq_t *bufq = ISP_GET_BUFQ(buf_mgr, bufq_handle);

  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }
  /* Lock since bufq is accessed and updated here*/
  pthread_mutex_lock(&bufq->mutex);
  if (!bufq->used) {
    pthread_mutex_unlock(&bufq->mutex);
    return -1;
  }

  isp_remove_vfe_fd(bufq, vfe_fd);

  if (bufq->num_vfe_fds > 0) {
    pthread_mutex_unlock(&bufq->mutex);
    return 1;
  }

  isp_release_kernel_bufq(buf_mgr, bufq, vfe_fd);
  pthread_mutex_unlock(&bufq->mutex);

  return 0;
}

/** isp_release_buf
 *
 * DESCRIPTION:
 *
 **/
void isp_release_buf(isp_buf_mgr_t *buf_mgr, uint32_t bufq_handle)
{
  int i;
  isp_bufq_t *bufq = ISP_GET_BUFQ(buf_mgr, bufq_handle);

  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return;
  }

  if (!bufq->used)
    return;

  pthread_mutex_lock(&bufq->mutex);
  if (bufq->use_native_buf) {
    for (i = 0; i < bufq->total_num_buffer; i++)
      isp_deinit_native_buffer(&bufq->image_buf[i], buf_mgr->ion_fd);
  }
  pthread_mutex_unlock(&bufq->mutex);

  pthread_mutex_lock(&buf_mgr->mutex);
  isp_free_bufq_handle(buf_mgr, bufq);
  pthread_mutex_unlock(&buf_mgr->mutex);

}

/** isp_open_buf_mgr
 *
 * DESCRIPTION:
 *
 **/
int isp_open_buf_mgr(isp_buf_mgr_t *buf_mgr)
{
  pthread_mutex_lock(&buf_mgr->mutex);
  if (buf_mgr->use_cnt++) {
    pthread_mutex_unlock(&buf_mgr->mutex);
    return 0;
  }

  buf_mgr->ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
  if (buf_mgr->ion_fd >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    buf_mgr->ion_fd = -1;
    pthread_mutex_unlock(&buf_mgr->mutex);
    return -1;
  }

  if (buf_mgr->ion_fd < 0) {
    CDBG_ERROR("%s: ion open failed\n", __func__);
    buf_mgr->use_cnt = 0;
    pthread_mutex_unlock(&buf_mgr->mutex);
    return -1;
  }
  pthread_mutex_unlock(&buf_mgr->mutex);

  return 0;
}

/** isp_close_buf_mgr
 *
 * DESCRIPTION:
 *
 **/
void isp_close_buf_mgr(isp_buf_mgr_t *buf_mgr)
{
  pthread_mutex_lock(&buf_mgr->mutex);
  if (--buf_mgr->use_cnt) {
    pthread_mutex_unlock(&buf_mgr->mutex);
    return;
  }

  if (buf_mgr->ion_fd) {
    close(buf_mgr->ion_fd);
    buf_mgr->ion_fd = 0;
  }
  pthread_mutex_unlock(&buf_mgr->mutex);

  return;
}

/** isp_init_buf_mgr
 *
 * DESCRIPTION:
 *
 **/
int isp_init_buf_mgr(isp_buf_mgr_t *buf_mgr)
{
  int i;

  if (buf_mgr->use_cnt++)
    return 0;

  memset(buf_mgr, 0, sizeof(isp_buf_mgr_t));
  memset(buf_mgr->bufq, 0, sizeof(isp_bufq_t) * ISP_MAX_NUM_BUF_QUEUE);

  pthread_mutex_init(&buf_mgr->mutex, NULL);
  pthread_mutex_init(&buf_mgr->req_mutex, NULL);

  for (i = 0; i < ISP_MAX_NUM_BUF_QUEUE; i++)
    pthread_mutex_init(&buf_mgr->bufq[i].mutex, NULL);

  return 0;
}

/** isp_deinit_buf_mgr
 *
 * DESCRIPTION:
 *
 **/
void isp_deinit_buf_mgr(isp_buf_mgr_t *buf_mgr)
{
  int i;

  if (--buf_mgr->use_cnt)
    return;

  for (i = 0; i < ISP_MAX_NUM_BUF_QUEUE; i++)
    pthread_mutex_destroy(&buf_mgr->bufq[i].mutex);

  pthread_mutex_destroy(&buf_mgr->req_mutex);
  pthread_mutex_destroy(&buf_mgr->mutex);
}

/** isp_open_ion
 *
 * DESCRIPTION:
 *
 **/
int isp_open_ion(void)
{
  int fd;

  fd = open("/dev/ion", O_RDONLY | O_SYNC);
  if (fd >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    fd = -1;
    return -1;
  }
  return fd;
}

/** isp_close_ion
 *
 * DESCRIPTION:
 *
 **/
void isp_close_ion(int ion_fd)
{
  if (ion_fd > 0)
    close(ion_fd);
}
