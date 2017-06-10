/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <math.h>
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif
#include <stdlib.h>
#include "camera_dbg.h"
#include "live_snapshot.h"
#include "mctl.h"
#include "cam_mmap.h"
#include "config_proc.h"

void liveshot_init(liveshot_ctrl_t *liveshot_ctrl, char *node_name)
{
  CDBG("%s: E\n", __func__);
  liveshot_ctrl->total_mctl_frames = 2;

  snprintf(liveshot_ctrl->buffer_info.node_name, MAX_DEV_NAME_LEN, "/dev/%s",
    node_name);
  liveshot_ctrl->buffer_info.main_node_fd = 0;
  CDBG("%s: X\n", __func__);
}

static uint32_t liveshot_get_v4l2_fmt(cam_format_t fmt, uint8_t *num_planes)
{
  uint32_t val;
  switch(fmt) {
    case CAMERA_YUV_420_NV12:
      val = V4L2_PIX_FMT_NV12;
      *num_planes = 2;
      break;
    case CAMERA_YUV_420_NV21:
      val = V4L2_PIX_FMT_NV21;
      *num_planes = 2;
      break;
    case CAMERA_YUV_422_NV16:
      val = V4L2_PIX_FMT_NV16;
      *num_planes = 2;
      break;
    case CAMERA_YUV_422_NV61:
      val = V4L2_PIX_FMT_NV61;
      *num_planes = 2;
      break;
    case CAMERA_BAYER_SBGGR10:
    case CAMERA_RDI:
      val= V4L2_PIX_FMT_SBGGR10;
      *num_planes = 1;
      break;
    default:
      val = 0;
      *num_planes = 0;
      break;
  }
  CDBG("%s: fmt: %d num_planes: %d\n", __func__, fmt, (int)num_planes);
  return val;
}

static int liveshot_get_frame_len(cam_format_t fmt_type,
                                      camera_mode_t mode,
                                      int width, int height,
                                      int image_mode,
                                      uint8_t *num_planes,
                                      uint32_t plane[])
{
    uint32_t size;
    *num_planes = 0;

    switch (fmt_type) {
    case CAMERA_YUV_420_NV12:
    case CAMERA_YUV_420_NV21:
        *num_planes = 2;
        if(CAMERA_MODE_3D == mode) {
            size = (uint32_t)(PAD_TO_2K(width*height)*3/2);
            plane[0] = PAD_TO_WORD(width*height);
        } else {
            if (image_mode == MSM_V4L2_EXT_CAPTURE_MODE_VIDEO) {
                plane[0] = PAD_TO_4K(width * height);
                plane[1] = PAD_TO_4K(width * height/2);
            } else if (image_mode == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW) {
                plane[0] = PAD_TO_WORD(width * height);
                plane[1] = PAD_TO_WORD(width * height/2);
            } else {
                plane[0] = PAD_TO_WORD(width * CEILING16(height));
                plane[1] = PAD_TO_WORD(width * CEILING16(height)/2);
            }
            size = plane[0] + plane[1];
        }
        break;
    default:
        CDBG_HIGH("%s: format %d not supported.\n",
            __func__, fmt_type);
        size = 0;
        break;
    }
    CDBG("%s: fmt=%d, image_type=%d, width=%d, height=%d, frame_len=%d\n",
        __func__, fmt_type, image_mode, width, height, size);
    return size;
}
static int liveshot_alloc_buf(int count, liveshot_buffer_alloc_t *buf, liveshot_frame_t *frames,
                         cam_format_t format, int width, int height,
                         int image_mode, int ion_dev_fd,
                         cam_frame_len_offset_t* frame_offset)
{
  int i, j, rc = 0;
  uint32_t frame_len;
  uint8_t num_planes = 0;
  uint32_t planes[VIDEO_MAX_PLANES];
  int y_off, cbcr_off;
  CDBG("%s: E\n",__func__);

  memset(planes, 0, sizeof(planes));
  num_planes = 2;
  planes[0] = frame_offset->mp[0].len;
  planes[1] = frame_offset->mp[1].len;
  frame_len = frame_offset->frame_len;
  y_off = frame_offset->mp[0].offset;
  cbcr_off = frame_offset->mp[1].offset;

  for (i = 0; i < count; i++) {
    buf[i].buf_size = frame_len;
#ifdef USE_ION
    buf[i].ion_alloc.len = buf[i].buf_size;
    buf[i].ion_alloc.flags = ION_FLAG_CACHED;
    buf[i].ion_alloc.heap_mask = (0x1<<CAMERA_ION_HEAP_ID|0x1<<ION_IOMMU_HEAP_ID);
    buf[i].ion_alloc.align = 4096;
    buf[i].buf = do_mmap_ion(ion_dev_fd, &(buf[i].ion_alloc),
      &(buf[i].fd_data), &(buf[i].fd));
#else
    buf[i].buf = do_mmap(buf[i].buf_size, (int*)&(buf[i].fd));
#endif
    if (!(buf[i].buf)) {
      CDBG_ERROR("%s do_mmap failed\n", __func__);
      /* Deallocate the buffers allocated so far */
      for (j = 0; j < i; j++) {
#ifdef USE_ION
        do_munmap_ion(ion_dev_fd, &(buf[j].fd_data),
          buf[j].buf, buf[j].ion_alloc.len);
#else
        do_munmap(buf[j].fd, buf[j].buf, buf[j].buf_size);
#endif
      }
      return -1;
    }
    CDBG("%s Alloc buffer index %d buf = 0x%p, fd = %d\n", __func__, i,
      buf[i].buf, buf[i].fd);

    frames[i].num_planes = num_planes;
    /* Plane 0 needs to be set seperately. Set other planes
     * in a loop. */
    frames[i].planes[0].length = planes[0];
    frames[i].planes[0].m.userptr = buf[i].fd;
    frames[i].planes[0].data_offset = y_off;
    frames[i].planes[0].reserved[0] = 0;

    for (j = 1; j < num_planes; j++) {
      frames[i].planes[j].length = planes[j];
      frames[i].planes[j].m.userptr = buf[i].fd;
      frames[i].planes[j].data_offset = cbcr_off;
      frames[i].planes[j].reserved[0] =
        frames[i].planes[j-1].reserved[0] +
        frames[i].planes[j-1].length;
    }
    CDBG("%s Prepared plane info for %d planes: length %d %d fd %d ", __func__,
      frames[i].num_planes, frames[i].planes[0].length,
      frames[i].planes[1].length,
      (int)frames[i].planes[0].m.userptr);
  }
  CDBG("%s: X\n",__func__);
  return rc;
}

static void liveshot_release_buf(int count, liveshot_buffer_alloc_t *buf, int ion_dev_fd)
{
  int i;
  CDBG("%s: E\n", __func__);

  for (i = 0; i < count; i++) {
#ifdef USE_ION
    do_munmap_ion(ion_dev_fd,
                  &(buf[i].fd_data),
                  buf[i].buf, buf[i].ion_alloc.len);
#else
    do_munmap(buf[i].fd, buf[i].buf, buf[i].buf_size);
#endif
    buf[i].fd = -1;
    buf[i].buf = 0;
    buf[i].buf_size = 0;
  }

  CDBG("%s: X\n", __func__);
}

static int setup_mctl_inst(mctl_config_ctrl_t *ctrl,
                           struct v4l2_format *fmt,
                           int mctl_fd, int output_id,
                           cam_format_t format, int image_mode)
{
  int rc = 0;
  struct v4l2_streamparm s_parm;
  struct img_plane_info plane_info;
  struct v4l2_control control;
  CDBG("%s: E\n", __func__);

  fmt->fmt.pix_mp.width =
    ctrl->curr_output_info.output[output_id].image_width +
    ctrl->curr_output_info.output[output_id].extra_pad_width;

  fmt->fmt.pix_mp.height =
    ctrl->curr_output_info.output[output_id].image_height +
    ctrl->curr_output_info.output[output_id].extra_pad_height;

  fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  fmt->fmt.pix_mp.field = V4L2_FIELD_NONE;
  fmt->fmt.pix_mp.pixelformat =
    liveshot_get_v4l2_fmt(format, &(fmt->fmt.pix_mp.num_planes));

  rc = ioctl(mctl_fd, VIDIOC_S_FMT, fmt);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_S_FMT failed: rc=%d\n", __func__, rc);
    goto ERROR;
  }

  s_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  s_parm.parm.capture.extendedmode = image_mode;

  rc = ioctl(mctl_fd, VIDIOC_S_PARM, &s_parm);
  if (rc < 0) {
    CDBG_ERROR("%s Error setting image mode ", __func__);
    goto ERROR;
  }

  plane_info.width = fmt->fmt.pix_mp.width;
  CDBG("%s: plane width: %d\n", __func__, plane_info.width);
  plane_info.height = fmt->fmt.pix_mp.height;
  CDBG("%s: plane height: %d\n", __func__, plane_info.height);
  plane_info.pixelformat = fmt->fmt.pix_mp.pixelformat;
  CDBG("%s: plane pixel format: %d\n", __func__, plane_info.pixelformat);
  plane_info.buffer_type = fmt->type;
  CDBG("%s: plane buffer_type: %d\n", __func__, plane_info.buffer_type);
  plane_info.ext_mode = image_mode;
  CDBG("%s: plane ext_mode: %d\n", __func__, plane_info.ext_mode);
  plane_info.num_planes = fmt->fmt.pix_mp.num_planes;
  CDBG("%s: plane num_planes: %d\n", __func__, plane_info.num_planes);

  rc = config_plane_info((void *)ctrl, (void *)&plane_info);
  if (rc < 0) {
    CDBG_ERROR("%s Error getting plane info ", __func__);
    goto ERROR;
  }

  memset(&control, 0, sizeof(control));
  control.id = MSM_V4L2_PID_PP_PLANE_INFO;
  control.value = (int32_t)&plane_info;

  rc = ioctl(mctl_fd, VIDIOC_S_CTRL, &control);
  if (rc < 0) {
    CDBG_ERROR("%s Error setting image mode ", __func__);
    goto ERROR;
  }
  CDBG("%s: X\n", __func__);
  return 0;
ERROR:
  return -EINVAL;
}

static int liveshot_qbuf(int mctl_fd, int idx, liveshot_frame_t *frames)
{
  int32_t i, rc = 0;
  struct v4l2_buffer buffer;

  CDBG("%s: E\n", __func__);
  memset(&buffer, 0, sizeof(buffer));
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  buffer.memory = V4L2_MEMORY_USERPTR;
  buffer.index = idx;
  buffer.m.planes = frames[idx].planes;
  buffer.length = frames[idx].num_planes;

  CDBG("%s: fd = %d, frame idx=%d, num planes %d\n",
      __func__, mctl_fd, idx, buffer.length);

  rc = ioctl(mctl_fd, VIDIOC_QBUF, &buffer);
  if (rc < 0) {
     CDBG_ERROR("%s: VIDIOC_QBUF error = %d\n", __func__, rc);
     return rc;
  }

  CDBG("%s: X\n", __func__);
  return rc;
}
static int liveshot_reg_buf(int count, int mctl_fd, liveshot_frame_t *frames)
{
  int i, rc = 0, j;
  struct v4l2_requestbuffers bufreq;
  int image_type;
  CDBG("%s: E\n", __func__);

  bufreq.count = count;
  bufreq.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  bufreq.memory = V4L2_MEMORY_USERPTR;
  CDBG("%s: calling VIDIOC_REQBUFS - fd=%d, num_buf=%d, type=%d, memory=%d\n",
    __func__, mctl_fd, bufreq.count, bufreq.type, bufreq.memory);

  rc = ioctl(mctl_fd, VIDIOC_REQBUFS, &bufreq);
  if (rc < 0) {
    CDBG_ERROR("%s: fd=%d, ioctl VIDIOC_REQBUFS failed: rc=%d\n", __func__,
      mctl_fd, rc);
    goto end;
  }
  CDBG("%s: stream fd=%d, ioctl VIDIOC_REQBUFS: num_frames = %d, rc=%d\n",
     __func__, mctl_fd, bufreq.count, rc);

  /* Queue all the buffers into the driver */
  for (i = 0; i < count ; i++) {
    rc = liveshot_qbuf(mctl_fd, i, frames);
    if (rc < 0)
      CDBG_ERROR("%s: VIDIOC_QBUF rc = %d\n", __func__, rc);
  }

  CDBG("%s: X\n", __func__);
end:
  return rc;
}

static int liveshot_unreg_buf(int mctl_fd)
{
  struct v4l2_requestbuffers bufreq;
  int32_t i, rc = 0;
  CDBG("%s: E\n", __func__);

  bufreq.count = 0;
  bufreq.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  bufreq.memory = V4L2_MEMORY_USERPTR;

  rc = ioctl(mctl_fd, VIDIOC_REQBUFS, &bufreq);
  if (rc < 0) {
     CDBG_ERROR("%s: fd=%d, VIDIOC_REQBUFS failed, rc=%d\n",
            __func__, mctl_fd, rc);
     return rc;
  }
  CDBG("%s: fd=%d, rc=%d\n", __func__, mctl_fd, rc);

  CDBG("%s: X\n", __func__);
  return rc;
}
static int liveshot_streamon(int mctl_fd)
{
  int rc = 0;
  enum v4l2_buf_type buf_type;
  CDBG("%s: E\n", __func__);

  buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

  rc = ioctl(mctl_fd, VIDIOC_STREAMON, &buf_type);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_STREAMON failed: rc=%d\n",
               __func__, rc);
    return rc;
  }
  CDBG("%s: fd=%d, VIDIOC_STREAMON rc = %d\n", __func__, mctl_fd, rc);

  CDBG("%s: X\n", __func__);
  return rc;
}

static int liveshot_streamoff(int mctl_fd)
{
  int rc = 0;
  enum v4l2_buf_type buf_type;
  CDBG("%s: E\n", __func__);

  buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

  rc = ioctl(mctl_fd, VIDIOC_STREAMOFF, &buf_type);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_STREAMOFF failed: rc=%d\n",
               __func__, rc);
    return rc;
  }

  CDBG("%s: fd=%d, VIDIOC_STREAMOFF rc = %d\n", __func__, mctl_fd, rc);

  CDBG("%s: X\n", __func__);
  return rc;
}

static int prepare_liveshot_frame(mctl_config_ctrl_t *ctrl)
{
  int rc = 0;
  struct v4l2_event_subscription sub;
  int vfe_main_output_number;
  liveshot_buffer_t *liveshot_buffer = &ctrl->liveshotCtrl.buffer_info;
  CDBG("%s: E\n", __func__);

  if (liveshot_buffer->main_node_fd > 0) {
    CDBG_ERROR("%s: Mctl node already opened\n", __func__);
    goto ERROR;
  }

  liveshot_buffer->main_node_fd = open(liveshot_buffer->node_name, O_RDWR);
  if (liveshot_buffer->main_node_fd <= 0) {
    CDBG_ERROR("%s Error opening mctl device node ", __func__);
    goto ERROR;
  }

  vfe_main_output_number = PRIMARY;
  liveshot_buffer->main_image_mode =
    ctrl->curr_output_info.output[vfe_main_output_number].stream_type;

  if (setup_mctl_inst(ctrl, &liveshot_buffer->main_fmt,
                           liveshot_buffer->main_node_fd, vfe_main_output_number,
                           liveshot_buffer->main_format,
                           liveshot_buffer->main_image_mode) < 0) {
    CDBG_ERROR("%s: Could not setup indtance\n", __func__);
    goto ERROR1;
  }

  liveshot_buffer->buf_count = ctrl->liveshotCtrl.total_mctl_frames;

  rc = liveshot_alloc_buf(liveshot_buffer->buf_count, liveshot_buffer->main_buf,
                     liveshot_buffer->main_frames, liveshot_buffer->main_format,
                     liveshot_buffer->main_fmt.fmt.pix_mp.width,
                     liveshot_buffer->main_fmt.fmt.pix_mp.height,
                     liveshot_buffer->main_image_mode, ctrl->ion_dev_fd,
                     &ctrl->dimInfo.picture_frame_offset);
  if (rc < 0) {
    CDBG_ERROR("%s Error allocating buffers ", __func__);
    goto ERROR2;
  }

  rc = liveshot_reg_buf(liveshot_buffer->buf_count, liveshot_buffer->main_node_fd,
                   liveshot_buffer->main_frames);
  if (rc < 0) {
    CDBG_ERROR("%s: Could not register buffer\n", __func__);
    goto ERROR3;
  }

  rc = liveshot_streamon(liveshot_buffer->main_node_fd);
  if (rc < 0) {
    CDBG_ERROR("%s: Streamon failed\n", __func__);
    goto ERROR4;
  }

  CDBG("%s: X\n", __func__);

  return 0;
ERROR4:
  liveshot_unreg_buf(liveshot_buffer->main_node_fd);
ERROR3:
  liveshot_release_buf(liveshot_buffer->buf_count, liveshot_buffer->main_buf,
                  ctrl->ion_dev_fd);
ERROR2:
#ifdef USE_ION
  close(ctrl->ion_dev_fd);
#endif
ERROR1:
  close(liveshot_buffer->main_node_fd);
  liveshot_buffer->main_node_fd = 0;
ERROR:
  return -EINVAL;
}

static int destroy_liveshot_frame(mctl_config_ctrl_t *ctrl)
{
  liveshot_buffer_t *liveshot_buffer = &ctrl->liveshotCtrl.buffer_info;
  CDBG("%s: E\n", __func__);

  liveshot_streamoff(liveshot_buffer->main_node_fd);
  liveshot_unreg_buf(liveshot_buffer->main_node_fd);
  liveshot_release_buf(liveshot_buffer->buf_count, liveshot_buffer->main_buf,
                  ctrl->ion_dev_fd);

  close(liveshot_buffer->main_node_fd);
  liveshot_buffer->main_node_fd = 0;

  CDBG("%s: X\n", __func__);
  return 0;
}

int32_t prepare_liveshot(void *ctrlblk)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)ctrlblk;
  CDBG("%s: E\n", __func__);
  if (ctrl->liveshotCtrl.total_mctl_frames > 0) {
    prepare_liveshot_frame(ctrl);
  }
  CDBG("%s: X\n", __func__);
  return 0;
}

int32_t destroy_liveshot(void *ctrlblk)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)ctrlblk;
  CDBG("%s: E\n", __func__);
  if (ctrl->liveshotCtrl.total_mctl_frames > 0) {
    destroy_liveshot_frame(ctrl);
  }
  CDBG("%s: X\n", __func__);
  return 0;
}
