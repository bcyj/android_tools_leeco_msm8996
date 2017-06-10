/* pp_buf_mgr.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdint.h>
#include <sys/time.h>
#include <linux/media.h>
#include <media/msmb_generic_buf_mgr.h>
#include "camera_dbg.h"
#include "pp_buf_mgr.h"
#include "server_debug.h"

/* macros for unpacking identity */
#define BUF_MGR_GET_STREAM_ID(identity) ((identity) & 0xFFFF)
#define BUF_MGR_GET_SESSION_ID(identity) (((identity) & 0xFFFF0000) >> 16)

/** pp_buf_mgr_t: buffer manager structure
 *
 *  @fd: fd to communicate with buffer manager subdevice
 *  @mutex: mutex to ensure shared resources are locked for
 *        concurrency
 *  @ref_count: ref count for open and close
 *
 *  buffer manager struct that has resources specific to buffer
 *  manager client **/
typedef struct _pp_buf_mgr_t {
  int32_t         fd;
  pthread_mutex_t mutex;
  uint32_t        ref_count;
} pp_buf_mgr_t;

/* Have only one instance of buffer manager*/
static pp_buf_mgr_t g_buf_mgr;

/** pp_buf_mgr_open_subdev: buffer manager subdevice open
 *
 *  @buf_mgr: buf mgr instance
 *
 *  find buffer manager subdev node and open it
 *
 *  Return: TRUE on SUCCESS
 *          FALSE on failure **/
static boolean pp_buf_mgr_open_subdev(pp_buf_mgr_t *buf_mgr)
{
  struct media_device_info mdev_info;
  int32_t num_media_devices = 0;
  char dev_name[32];
  char subdev_name[32];
  int32_t dev_fd = 0, ioctl_ret;
  boolean ret = FALSE;
  uint32_t i = 0;

  CDBG("%s:%d Enter\n", __func__, __LINE__);
  if (!buf_mgr) {
    CDBG_ERROR("%s:%d buf mgr NULL\n", __func__, __LINE__);
    return FALSE;
  }
  while (1) {
    int32_t num_entities = 1;
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd >= MAX_FD_PER_PROCESS) {
      dump_list_of_daemon_fd();
      dev_fd = -1;
    }
    if (dev_fd < 0) {
      CDBG("%s:%d Done enumerating media devices\n", __func__, __LINE__);
      break;
    }
    num_media_devices++;
    ioctl_ret = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (ioctl_ret < 0) {
      CDBG("%s:%d Done enumerating media devices\n", __func__, __LINE__);
      close(dev_fd);
      break;
    }

    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model) != 0)) {
      close(dev_fd);
      continue;
    }
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities++;
      CDBG("%s:%d entity id %d", __func__, __LINE__, entity.id);
      ioctl_ret = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (ioctl_ret < 0) {
        CDBG("%s:%d Done enumerating media entities\n", __func__, __LINE__);
        break;
      }
      CDBG("%s:%d entity name %s type %d group id %d\n", __func__, __LINE__,
        entity.name, entity.type, entity.group_id);
      if (entity.type == MEDIA_ENT_T_V4L2_SUBDEV &&
          entity.group_id == MSM_CAMERA_SUBDEV_BUF_MNGR) {
        snprintf(subdev_name, sizeof(dev_name), "/dev/%s", entity.name);
        CDBG("%s: subdev_name:%s\n", __func__, subdev_name);
        buf_mgr->fd = open(subdev_name, O_RDWR);
        if (buf_mgr->fd >= MAX_FD_PER_PROCESS) {
          dump_list_of_daemon_fd();
          buf_mgr->fd = -1;
        }
        if (buf_mgr->fd < 0) {
          CDBG("%s:Open subdev failed\n", __func__);
          continue;
        }
        CDBG("%s:Open subdev Success\n", __func__);
        ret = TRUE;
        break;
      }
    }
    close(dev_fd);
  }
  return ret;
}

/** pp_buf_mgr_match_buf_index: buffer manager subdevice open
 *
 *  @data1: fd to communicate with buffer manager subdevice
 *  @data2: mutex to ensure shared resources are locked for
 *        concurrency
 *
 *  buffer manager struct that has resources specific to buffer
 *  manager client **/
static boolean pp_buf_mgr_match_buf_index(void *data1, void *data2)
{
  mct_stream_map_buf_t *list_buf = (mct_stream_map_buf_t *)data1;
  uint32_t *user_buf_index = (uint32_t *)data2;

  /* Validate input parameters */
  if (!list_buf || !user_buf_index) {
    CDBG_ERROR("%s:%d failed: invalid input params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Match list buffer index with user buffer index */
  if (list_buf->buf_index == *user_buf_index) {
    return TRUE;
  }

ERROR:
  return FALSE;
}

/** pp_buf_mgr_open: buffer manager open
 *
 *  On first call, create buffer manager instance, call function
 *  to open buffer manager subdevice and increment ref count.
 *
 *  Return: buffer manager instance on SUCCESS
 *          NULL on failure **/
void *pp_buf_mgr_open(void)
{
  boolean rc = TRUE;

  /* Open subdev if this is the first call */
  if (!g_buf_mgr.ref_count) {
    rc = pp_buf_mgr_open_subdev(&g_buf_mgr);
    if (rc == FALSE || g_buf_mgr.fd < 0) {
      CDBG_ERROR("%s:%d failed: to open subdev rc %d fd %d \n", __func__,
        __LINE__, rc, g_buf_mgr.fd);
      goto ERROR;
    }
  }

  /* Increment ref count */
  g_buf_mgr.ref_count++;
  return &g_buf_mgr;
ERROR:
  return NULL;
}

/** pp_buf_mgr_close: buffer manager close
 *
 *  @buf_mgr: buf mgr instance
 *
 *  Decrement buffer manager ref count. If ref count is zero,
 *  close sub device and free memory
 *
 *  Return: TRUE on SUCCESS
 *          FALSE on failure **/
boolean pp_buf_mgr_close(void *v_buf_mgr)
{
  pp_buf_mgr_t *buf_mgr = (pp_buf_mgr_t *)v_buf_mgr;

  /* Validate input parameters */
  if (!buf_mgr || !buf_mgr->ref_count) {
    CDBG_ERROR("%s:%d invalid params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Decrement refcount and close fd if ref count is 0 */
  if (--buf_mgr->ref_count == 0) {
    close(buf_mgr->fd);
    buf_mgr->fd = 0;
  }

  return TRUE;
ERROR:
  return FALSE;
}

/** pp_buf_mgr_get_buf: get buf API to acquire buffer
 *
 *  @v_buf_mgr: buf mgr instance
 *  @stream_info: stream info handle
 *
 *  Get buffer from kernel buffer manager and match its index
 *  from stream info's img buf list to extract other params like
 *  fd, vaddr
 *
 *  Return: SUCCESS - stream map struct representing buffer
 *          FAILURE - NULL **/
mct_stream_map_buf_t *pp_buf_mgr_get_buf(void *v_buf_mgr,
  mct_stream_info_t *stream_info)
{
  int32_t                   ret = 0;
  pp_buf_mgr_t             *buf_mgr = (pp_buf_mgr_t *)v_buf_mgr;
  struct msm_buf_mngr_info  buffer;
  mct_list_t               *img_buf_list = NULL;

  /* Validate input parameters */
  if (!buf_mgr || !stream_info) {
    CDBG_ERROR("%s:%d invalid params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Validate buffer manager parameters */
  if (buf_mgr->fd < 0 || !buf_mgr->ref_count) {
    CDBG_ERROR("%s:%d invalid buf mgr params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Validate stream info parameters */
  if (!stream_info->img_buffer_list) {
    CDBG_ERROR("%s:%d invalid steam info params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Get buffer from buf mgr node */
  buffer.session_id = BUF_MGR_GET_SESSION_ID(stream_info->identity);
  buffer.stream_id = BUF_MGR_GET_STREAM_ID(stream_info->identity);
  ret = ioctl(buf_mgr->fd, VIDIOC_MSM_BUF_MNGR_GET_BUF, &buffer);
  if (ret < 0) {
    CDBG("%s:%d failed: to get buf from buf mgr", __func__, __LINE__);
    goto ERROR;
  }

  /* Match vb2 buffer index with stream info buf list */
  img_buf_list = mct_list_find_custom(stream_info->img_buffer_list,
    &buffer.index, pp_buf_mgr_match_buf_index);
  if (!img_buf_list || !img_buf_list->data) {
    CDBG_ERROR("%s:%d failed: to match kernel buf index with stream buf list\n",
      __func__, __LINE__);
    pp_buf_mgr_put_buf(v_buf_mgr, stream_info->identity,
        buffer.index, buffer.frame_id, buffer.timestamp);
    goto ERROR;
  }

  return img_buf_list->data;
ERROR:
  return NULL;
}

/** pp_buf_mgr_put_buf: put buf API to release buffer to buf mgr
 *  without doing buf done on HAL
 *
 *  @v_buf_mgr: buf mgr instance
 *  @stream_info: stream info handle
 *
 *  Invoke put buf on buffer manager node
 *
 *  Return: SUCCESS - TRUE
 *          FAILURE - FALSE **/
boolean pp_buf_mgr_put_buf(void *v_buf_mgr, uint32_t identity,
  uint32_t buff_idx, uint32_t frameid, struct timeval timestamp)
{
  int                       ret = 0;
  pp_buf_mgr_t             *buf_mgr = (pp_buf_mgr_t *)v_buf_mgr;
  struct msm_buf_mngr_info  buff;

  /* Validate input parameters */
  if (!buf_mgr) {
    CDBG_ERROR("%s:%d invalid params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Validate buffer manager parameters */
  if (buf_mgr->fd < 0 || !buf_mgr->ref_count) {
    CDBG_ERROR("%s:%d invalid buf mgr params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Perform ioctl for put buf on buf mgr node */
  buff.index = buff_idx;
  buff.session_id = BUF_MGR_GET_SESSION_ID(identity);
  buff.stream_id = BUF_MGR_GET_STREAM_ID(identity);
  buff.frame_id = frameid;
  buff.timestamp = timestamp;
  ret = ioctl(buf_mgr->fd, VIDIOC_MSM_BUF_MNGR_PUT_BUF, &buff);
  if (ret < 0) {
    CDBG_ERROR("%s:%d failed: to put buf on kernel buf mgr node", __func__,
      __LINE__);
    goto ERROR;
  }
  return TRUE;

ERROR:
  return FALSE;
}

/** pp_buf_mgr_buf_done: buf done API to release buffer to buf
 *  mgr + buf done on HAL
 *
 *  @v_buf_mgr: buf mgr instance
 *  @stream_info: stream info handle
 *
 *  Invoke buf done on buffer manager node
 *
 *  Return: SUCCESS - TRUE
 *          FAILURE - FALSE **/
boolean pp_buf_mgr_buf_done(void *v_buf_mgr, uint32_t identity,
  uint32_t buff_idx, uint32_t frameid, struct timeval timestamp)
{
  int                       ret = 0;
  pp_buf_mgr_t             *buf_mgr = (pp_buf_mgr_t *)v_buf_mgr;
  struct msm_buf_mngr_info  buff;

  /* Validate input parameters */
  if (!buf_mgr) {
    CDBG_ERROR("%s:%d invalid params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Validate buffer manager parameters */
  if (buf_mgr->fd < 0 || !buf_mgr->ref_count) {
    CDBG_ERROR("%s:%d invalid buf mgr params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Perform ioctl for buf done on buf mgr node */
  buff.index = buff_idx;
  buff.session_id = BUF_MGR_GET_SESSION_ID(identity);
  buff.stream_id = BUF_MGR_GET_STREAM_ID(identity);
  buff.frame_id = frameid;
  buff.timestamp = timestamp;
  ret = ioctl(buf_mgr->fd, VIDIOC_MSM_BUF_MNGR_BUF_DONE, &buff);
  if (ret < 0) {
    CDBG_ERROR("%s:%d failed: to buf done on kernel buf mgr node", __func__,
      __LINE__);
    goto ERROR;
  }
  return TRUE;

ERROR:
  return FALSE;
}

/** pp_buf_mgr_get_vaddr: get virtual address from buffer index
 *  and associated stream info
 *
 *  @v_buf_mgr: [INPUT] buf mgr instance
 *  @buff_idx: [INPUT] buf index
 *  @stream_info: [INPUT] stream info handle
 *  @vaddr: [OUTPUT] address of virtual address
 *
 *  Get virtual address from buffer index and associated stream
 *  info
 *
 *  Return: SUCCESS - TRUE
 *          FAILURE - FALSE **/
boolean pp_buf_mgr_get_vaddr(void *v_buf_mgr, uint32_t buff_idx,
  mct_stream_info_t *stream_info, unsigned long *vaddr)
{
  boolean              rc = TRUE;
  pp_buf_mgr_t         *buf_mgr = (pp_buf_mgr_t *)v_buf_mgr;
  mct_list_t           *img_buf_list = NULL;
  mct_stream_map_buf_t *img_buf = NULL;

  /* Validate input parameters */
  if (!buf_mgr || !stream_info || !vaddr) {
    CDBG_ERROR("%s:%d invalid params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Validate buffer manager parameters */
  if (buf_mgr->fd < 0 || !buf_mgr->ref_count) {
    CDBG_ERROR("%s:%d invalid buf mgr params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Validate stream info parameters */
  if (!stream_info->img_buffer_list) {
    CDBG_ERROR("%s:%d invalid steam info params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Match vb2 buffer index with stream info buf list */
  img_buf_list = mct_list_find_custom(stream_info->img_buffer_list,
    &buff_idx, pp_buf_mgr_match_buf_index);
  if (!img_buf_list || !img_buf_list->data) {
    CDBG_ERROR("%s:%d failed: to match kernel buf index with stream buf list\n",
      __func__, __LINE__);
    goto ERROR;
  }

  img_buf = (mct_stream_map_buf_t *)img_buf_list->data;

  /* Fill virtual address */
  *vaddr = (unsigned long)img_buf->buf_planes[0].buf;

  return TRUE;
ERROR:
  return rc;
}
