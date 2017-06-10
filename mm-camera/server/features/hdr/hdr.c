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
#include "hdr.h"
#include "mctl.h"
#include "cam_mmap.h"
#include "config_proc.h"
#define GET_TO_INDEX 12

static float aec_ev_compensation_one_over_six_table[] =
{ /* Must be consistent with the table in aec.c*/
  0.2500,    /* 2^EV_Comp = 2^-12/6 */
  0.2806,    /* 2^EV_Comp = 2^-11/6 */
  0.3150,    /* 2^EV_Comp = 2^-10/6 */
  0.3536,    /* 2^EV_Comp = 2^-9/6  */
  0.3969,    /* 2^EV_Comp = 2^-8/6  */
  0.4454,    /* 2^EV_Comp = 2^-7/6  */
  0.5000,    /* 2^EV_Comp = 2^-6/6  */
  0.5612,    /* 2^EV_Comp = 2^-5/6  */
  0.6299,    /* 2^EV_Comp = 2^-4/6  */
  0.7071,    /* 2^EV_Comp = 2^-3/6  */
  0.7937,    /* 2^EV_Comp = 2^-2/6  */
  0.8909,    /* 2^EV_Comp = 2^-1/6  */
  1.0000,    /* 2^EV_Comp = 2^0     */
  1.1225,    /* 2^EV_Comp = 2^1/6   */
  1.2599,    /* 2^EV_Comp = 2^2/6   */
  1.4142,    /* 2^EV_Comp = 2^3/6   */
  1.5874,    /* 2^EV_Comp = 2^4/6   */
  1.7818,    /* 2^EV_Comp = 2^5/6   */
  2.0000,    /* 2^EV_Comp = 2^6/6   */
  2.2449,    /* 2^EV_Comp = 2^7/6   */
  2.5198,    /* 2^EV_Comp = 2^8/6   */
  2.8284,    /* 2^EV_Comp = 2^9/6   */
  3.1748,    /* 2^EV_Comp = 2^10/6  */
  3.5636,    /* 2^EV_Comp = 2^11/6  */
  4.0000     /* 2^EV_Comp = 2^12/6  */
};

void hdr_init(hdr_ctrl_t *hdr_ctrl, char *node_name)
{
  hdr_ctrl->current_snapshot_count = 0;
  hdr_ctrl->hdr_main_divert_count = 0;
  hdr_ctrl->hdr_thumb_divert_count = 0;
  hdr_ctrl->exp_bracketing_enable = 0;
  hdr_ctrl->frm_index = 0;
  hdr_ctrl->hdr_enable = 0;
  hdr_ctrl->total_frames = 1;
  hdr_ctrl->total_hal_frames = 1;
  hdr_ctrl->total_mctl_frames = 0;
  hdr_ctrl->algo_exp_tbl[0] = 1; /*HDR_EXPOSURE_x1 */
  hdr_ctrl->algo_exp_tbl[1] = 0.5; /* HDR_EXPOSURE_xPT5 */
  hdr_ctrl->algo_exp_tbl[2] = 2; /* HDR_EXPOSURE_x2 */

  snprintf(hdr_ctrl->buffer_info.node_name, MAX_DEV_NAME_LEN, "/dev/%s",
    node_name);
  hdr_ctrl->buffer_info.thumb_node_fd = 0;
  hdr_ctrl->buffer_info.main_node_fd = 0;
}

/*===========================================================================

Function           : map_user_exp

Description        : Map user exposure compensation to hdr exposure
Input parameter(s) : user Exposure Compensation


Return Value       : null

=========================================================================== */

static float map_user_exp(int user_exp)
{
 return aec_ev_compensation_one_over_six_table[user_exp + GET_TO_INDEX];
}

static void  hdr_sort_exp(float* exp_vals, int exp_num)
{
  int i, j, n;
  float temp;
  /*1. find 1.0x exp, could have multi 1.0x*/
  n = 0;
  for (i = 1; i< exp_num; i++) {
    if (*(exp_vals+i) == 1.0 ) { /*swap to the tip*/
      temp = *(exp_vals+n);
      *(exp_vals+n) = *(exp_vals+i);
      *(exp_vals+i) = temp;
      n++;
    }
  }
  if (*(exp_vals+n) == 1.0 ) {
    n++;
  }
  /*2. sort the rest by assending order*/
  for (i = n; i <exp_num; i++) {
    for (j = i+1; j < exp_num; j++) {
      if (*(exp_vals+i) > *(exp_vals+j)) { /*swap*/
        temp = *(exp_vals+i);
        *(exp_vals+i) = *(exp_vals+j);
        *(exp_vals+j) = temp;
      }
    }
  }
  return;
}

int32_t hdr_get(hdr_parm_t type, void *cctrl)
{
  mctl_config_ctrl_t * ctrl = (mctl_config_ctrl_t *)cctrl;
  hdr_ctrl_t *hdr_ctrl = &ctrl->hdrCtrl;
  if (hdr_ctrl == NULL) {
    CDBG_HIGH("%s invalid parmeter", __func__);
    return 0;
  }
  char *val, *exp_value, *prev_value;
  uint32_t i;
  switch (type) {
  case HDR_PARM_ALGO_ID:
    break;
  case HDR_PARM_FRM_USER:
    exp_value = (char *) hdr_ctrl->user_exp_values;
    i = 0;
    val = strtok_r(exp_value,",", &prev_value);
    while (val != NULL){
      hdr_ctrl->algo_exp_tbl[i++] = map_user_exp(atoi(val));
      if(i >= hdr_ctrl->total_frames )
        break;
      val = strtok_r(NULL, ",", &prev_value);
    }
    break;
  case HDR_PARM_FRM_EXP:
    {
      uint32_t i;
      hdr_ctrl->total_frames = ctrl->frame_proc_ctrl.intf.output.hdr_d.max_num_hdr_frames;
      hdr_ctrl->total_mctl_frames = hdr_ctrl->total_frames
                                         - hdr_ctrl->total_hal_frames;
      for (i = 0; i < hdr_ctrl->total_frames; i++) {
        hdr_ctrl->algo_exp_tbl[i] = ctrl->frame_proc_ctrl.intf.output.hdr_d.exp_values[i];
      }
    }

    break;
  case HDR_PARM_HW_INFO:
    break;
  default:
    break;
  }
  for (i =0; i < hdr_ctrl->total_frames; i++) {
    CDBG("%s: hdr_ctrl->algo_exp_tbl[%d] =%f\n",
         __func__, i, hdr_ctrl->algo_exp_tbl[i]);
  }
  hdr_sort_exp(hdr_ctrl->algo_exp_tbl, hdr_ctrl->total_frames);
  for (i =0; i < hdr_ctrl->total_frames; i++) {
    CDBG("%s: post hdr_ctrl->algo_exp_tbl[%d] =%f\n",
         __func__, i, hdr_ctrl->algo_exp_tbl[i]);
  }
  return 1;
}

static uint32_t hdr_get_v4l2_fmt(cam_format_t fmt, uint8_t *num_planes)
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
    case CAMERA_YUV_422_YUYV:
      val=V4L2_PIX_FMT_YUYV;
      *num_planes = 1;
      break;
    default:
      val = 0;
      *num_planes = 0;
      break;
  }
  return val;
}

static int hdr_alloc_buf(int count, hdr_buffer_alloc_t *buf, hdr_frame_t *frames,
                         cam_format_t format, int width, int height,
                         int image_mode, int ion_dev_fd,
                         cam_frame_len_offset_t* frame_offset)
{
  int i, j, rc = 0;
  uint32_t frame_len;
  uint8_t num_planes = 0;
  uint32_t planes[VIDEO_MAX_PLANES];
  int y_off, cbcr_off;

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
  return rc;
}

static void hdr_release_buf(int count, hdr_buffer_alloc_t *buf, int ion_dev_fd)
{
  int i;
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
  fmt->fmt.pix_mp.width =
    ctrl->curr_output_info.output[output_id].image_width +
    ctrl->curr_output_info.output[output_id].extra_pad_width;
  fmt->fmt.pix_mp.height =
    ctrl->curr_output_info.output[output_id].image_height +
    ctrl->curr_output_info.output[output_id].extra_pad_height;
  fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  fmt->fmt.pix_mp.field = V4L2_FIELD_NONE;
  fmt->fmt.pix_mp.pixelformat =
    hdr_get_v4l2_fmt(format, &(fmt->fmt.pix_mp.num_planes));
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
  plane_info.height = fmt->fmt.pix_mp.height;
  plane_info.pixelformat = fmt->fmt.pix_mp.pixelformat;
  plane_info.buffer_type = fmt->type;
  plane_info.ext_mode = image_mode;
  plane_info.num_planes = fmt->fmt.pix_mp.num_planes;
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
  return 0;
ERROR:
  return -EINVAL;
}

static int hdr_qbuf(int mctl_fd, int idx, hdr_frame_t *frames)
{
    int32_t i, rc = 0;
    struct v4l2_buffer buffer;

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
    return rc;
}

static int hdr_reg_buf(int count, int mctl_fd, hdr_frame_t *frames)
{
  int i, rc = 0, j;
  struct v4l2_requestbuffers bufreq;
  int image_type;

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
  CDBG("%s: stream fd=%d, ioctl VIDIOC_REQBUFS: memtype = %d, num_frames = %d, rc=%d\n",
    __func__, mctl_fd, bufreq.memory, bufreq.count, rc);

  /* Queue all the buffers into the driver */
  for (i = 0; i < count ; i++) {
    rc = hdr_qbuf(mctl_fd, i, frames);
    if (rc < 0) {
      CDBG_ERROR("%s: VIDIOC_QBUF rc = %d\n", __func__, rc);
    }
  }
end:
  return rc;
}

static int hdr_unreg_buf(int mctl_fd)
{
    struct v4l2_requestbuffers bufreq;
    int32_t i, rc = 0;

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
    return rc;
}

static int hdr_streamon(int mctl_fd)
{
  int rc = 0;
  enum v4l2_buf_type buf_type;

  buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  rc = ioctl(mctl_fd, VIDIOC_STREAMON, &buf_type);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_STREAMON failed: rc=%d\n",
               __func__, rc);
    return rc;
  }

  CDBG("%s: fd=%d, VIDIOC_STREAMON rc = %d\n", __func__, mctl_fd, rc);
  return rc;
}

static int hdr_streamoff(int mctl_fd)
{
  int rc = 0;
  enum v4l2_buf_type buf_type;

  buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  rc = ioctl(mctl_fd, VIDIOC_STREAMOFF, &buf_type);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_STREAMOFF failed: rc=%d\n",
               __func__, rc);
    return rc;
  }

  CDBG("%s: fd=%d, VIDIOC_STREAMOFF rc = %d\n", __func__, mctl_fd, rc);
  return rc;
}

static void hdr_reg_pp_node_buf(mctl_config_ctrl_t *ctrl, int image_mode,
                               int buf_count, hdr_buffer_alloc_t *buf)
{
  int i;
  uint32_t idx;
  mctl_pp_local_buf_info_t *buf_data;

  CDBG("%s Storing buffer info for image mode %d ", __func__, image_mode);
  buf_data = ctrl->video_ctrl.mctl_buf_info[image_mode].local_frame_info;
  for (i = 0; i < buf_count; i++) {
    buf_data[i].fd = buf[i].fd;
    buf_data[i].local_vaddr = buf[i].buf;
    CDBG("%s Buffer idx %d fd %d Vaddr = %x", __func__, i,
         buf_data[i].fd, (int)buf_data[i].local_vaddr);
  }
}

static void hdr_unreg_pp_node_buf(mctl_config_ctrl_t *ctrl, int image_mode)
{
  mctl_pp_buf_info_t *buf_info;

  CDBG("%s Removing buffer info for image mode %d ", __func__, image_mode);
  buf_info = &(ctrl->video_ctrl.mctl_buf_info[image_mode]);
  memset(buf_info, 0, sizeof(mctl_pp_buf_info_t));
}

static int prepare_hdr_frame(mctl_config_ctrl_t *ctrl)
{
  int rc = 0;
  struct v4l2_event_subscription sub;
  int vfe_main_output_number;
  hdr_buffer_t *hdr_buffer = &ctrl->hdrCtrl.buffer_info;
  if (hdr_buffer->main_node_fd > 0 || hdr_buffer->thumb_node_fd > 0) {
    CDBG_ERROR("%s: Mctl node already opened\n", __func__);
    goto ERROR;
  }
  hdr_buffer->main_node_fd = open(hdr_buffer->node_name, O_RDWR);
  if (hdr_buffer->main_node_fd <= 0) {
    CDBG_ERROR("%s Error opening mctl device node ", __func__);
    goto ERROR;
  }
  hdr_buffer->thumb_node_fd = open(hdr_buffer->node_name, O_RDWR);
  if (hdr_buffer->thumb_node_fd <= 0) {
    CDBG_ERROR("%s Error opening mctl device node ", __func__);
    goto ERROR1;
  }

  if (ctrl->dimInfo.thumbnail_width > ctrl->dimInfo.picture_width)
    vfe_main_output_number = SECONDARY;
  else
    vfe_main_output_number = PRIMARY;
  hdr_buffer->thumb_image_mode =
    ctrl->curr_output_info.output[!vfe_main_output_number].stream_type;
  hdr_buffer->thumb_format = ctrl->dimInfo.thumb_format;
  hdr_buffer->main_image_mode =
    ctrl->curr_output_info.output[vfe_main_output_number].stream_type;
  hdr_buffer->main_format = ctrl->dimInfo.thumb_format;

  if (setup_mctl_inst(ctrl,&hdr_buffer->thumb_fmt,
                           hdr_buffer->thumb_node_fd, !vfe_main_output_number,
                           hdr_buffer->thumb_format,
                           hdr_buffer->thumb_image_mode) < 0) {
    CDBG_ERROR("%s: Could not setup indtance\n", __func__);
    goto ERROR2;
  }
  if (setup_mctl_inst(ctrl, &hdr_buffer->main_fmt,
                           hdr_buffer->main_node_fd, vfe_main_output_number,
                           hdr_buffer->main_format,
                           hdr_buffer->main_image_mode) < 0) {
    CDBG_ERROR("%s: Could not setup indtance\n", __func__);
    goto ERROR2;
  }

  hdr_buffer->buf_count = ctrl->hdrCtrl.total_mctl_frames;
  rc = hdr_alloc_buf(hdr_buffer->buf_count, hdr_buffer->thumb_buf,
                     hdr_buffer->thumb_frames, hdr_buffer->thumb_format,
                     hdr_buffer->thumb_fmt.fmt.pix_mp.width,
                     hdr_buffer->thumb_fmt.fmt.pix_mp.height,
                     hdr_buffer->thumb_image_mode, ctrl->ion_dev_fd,
                     &ctrl->dimInfo.thumb_frame_offset);
  if (rc < 0) {
    CDBG_ERROR("%s Error allocating buffers ", __func__);
    goto ERROR3;
  }
  rc = hdr_alloc_buf(hdr_buffer->buf_count, hdr_buffer->main_buf,
                     hdr_buffer->main_frames, hdr_buffer->main_format,
                     hdr_buffer->main_fmt.fmt.pix_mp.width,
                     hdr_buffer->main_fmt.fmt.pix_mp.height,
                     hdr_buffer->main_image_mode, ctrl->ion_dev_fd,
                     &ctrl->dimInfo.picture_frame_offset);
  if (rc < 0) {
    CDBG_ERROR("%s Error allocating buffers ", __func__);
    goto ERROR4;
  }

  rc = hdr_reg_buf(hdr_buffer->buf_count, hdr_buffer->thumb_node_fd,
                   hdr_buffer->thumb_frames);
  if (rc < 0) {
    CDBG_ERROR("%s: Could not register buffer\n", __func__);
    goto ERROR5;
  }
  rc = hdr_reg_buf(hdr_buffer->buf_count, hdr_buffer->main_node_fd,
                   hdr_buffer->main_frames);
  if (rc < 0) {
    CDBG_ERROR("%s: Could not register buffer\n", __func__);
    goto ERROR6;
  }

  rc = hdr_streamon(hdr_buffer->thumb_node_fd);
  if (rc < 0) {
    CDBG_ERROR("%s: Streamon failed\n", __func__);
    goto ERROR7;
  }
  rc = hdr_streamon(hdr_buffer->main_node_fd);
  if (rc < 0) {
    CDBG_ERROR("%s: Streamon failed\n", __func__);
    goto ERROR8;
  }

  hdr_reg_pp_node_buf(ctrl, hdr_buffer->thumb_image_mode,
                      hdr_buffer->buf_count, hdr_buffer->thumb_buf);
  hdr_reg_pp_node_buf(ctrl, hdr_buffer->main_image_mode,
                      hdr_buffer->buf_count, hdr_buffer->main_buf);

  return 0;

ERROR8:
  hdr_streamoff(hdr_buffer->thumb_node_fd);
ERROR7:
  hdr_unreg_buf(hdr_buffer->main_node_fd);
ERROR6:
  hdr_unreg_buf(hdr_buffer->thumb_node_fd);
ERROR5:
  hdr_release_buf(hdr_buffer->buf_count, hdr_buffer->main_buf,
                  ctrl->ion_dev_fd);
ERROR4:
  hdr_release_buf(hdr_buffer->buf_count, hdr_buffer->thumb_buf,
                  ctrl->ion_dev_fd);
ERROR3:
#ifdef USE_ION
  close(ctrl->ion_dev_fd);
#endif
ERROR2:
  close(hdr_buffer->thumb_node_fd);
  hdr_buffer->thumb_node_fd = 0;
ERROR1:
  close(hdr_buffer->main_node_fd);
  hdr_buffer->main_node_fd = 0;
ERROR:
  return -EINVAL;
}

static int destroy_hdr_frame(mctl_config_ctrl_t *ctrl)
{
  hdr_buffer_t *hdr_buffer = &ctrl->hdrCtrl.buffer_info;
  hdr_streamoff(hdr_buffer->main_node_fd);
  hdr_streamoff(hdr_buffer->thumb_node_fd);
  hdr_unreg_buf(hdr_buffer->main_node_fd);
  hdr_unreg_buf(hdr_buffer->thumb_node_fd);
  hdr_release_buf(hdr_buffer->buf_count, hdr_buffer->main_buf,
                  ctrl->ion_dev_fd);
  hdr_release_buf(hdr_buffer->buf_count, hdr_buffer->thumb_buf,
                  ctrl->ion_dev_fd);
  hdr_unreg_pp_node_buf(ctrl, hdr_buffer->main_image_mode);
  hdr_unreg_pp_node_buf(ctrl, hdr_buffer->thumb_image_mode);

  close(hdr_buffer->thumb_node_fd);
  hdr_buffer->thumb_node_fd = 0;
  close(hdr_buffer->main_node_fd);
  hdr_buffer->main_node_fd = 0;
  return 0;
}

int32_t prepare_hdr(void *ctrlblk)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)ctrlblk;
  hdr_buffer_t *hdr_buffer = &ctrl->hdrCtrl.buffer_info;
  ctrl->hdrCtrl.frm_index = 0;
  ctrl->hdrCtrl.current_snapshot_count = 0;
  ctrl->hdrCtrl.hdr_main_divert_count = 0;
  ctrl->hdrCtrl.hdr_thumb_divert_count = 0;
  if (ctrl->hdrCtrl.total_mctl_frames > 0) {
    prepare_hdr_frame(ctrl);
  }
  return 0;
}

int32_t destroy_hdr(void *ctrlblk)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)ctrlblk;
  hdr_buffer_t *hdr_buffer = &ctrl->hdrCtrl.buffer_info;
  ctrl->hdrCtrl.frm_index = 0;
  if (ctrl->hdrCtrl.total_mctl_frames > 0) {
    destroy_hdr_frame(ctrl);
  }
  return 0;
}

#define MAX_IDEAL_HDR_GAIN  4.0
int32_t hdr_calc_sensor_gain_upon_sof(void *cctrl)
{
  int32_t rc = 0;
  uint32_t exp_tbl_num_entries, line_count;
  uint16_t fix_fps_exp_idx;
  float gain, temp_gain;
  mctl_config_ctrl_t * ctrl = (mctl_config_ctrl_t *)cctrl;
  sensor_set_t sensor_set;
  chromatix_parms_type *pchromatix;
  sensor_get_t sensor_get;
  uint32_t min_line_cnt;

  if (ctrl->hdrCtrl.frm_index < (int)ctrl->hdrCtrl.total_frames ) {

    if(ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
      ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
      SENSOR_GET_CHROMATIX_PTR, &sensor_get, sizeof(sensor_get))) {
      CDBG("%s Error getting chromatix ptr from sensor ", __func__);
      rc =-1;
      goto __end;
    }

    pchromatix = sensor_get.data.chromatix_ptr;
    min_line_cnt = pchromatix->chromatix_exposure_table.
      exposure_entries[0].line_count;

    sensor_get.data.aec_info.op_mode = SENSOR_MODE_SNAPSHOT;
    rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
            ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
            SENSOR_GET_SENSOR_MAX_AEC_INFO,
            &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
      CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
     goto __end;
    }
    line_count = ctrl->stats_proc_ctrl.intf.output.aec_d.snap.line_count ;
    gain = ctrl->stats_proc_ctrl.intf.output.aec_d.snap.real_gain;
    CDBG("%s: orig line_cnt =%d, gain=%f, frm_idx=%d, ajust =%f, total_frame=%d",
                __func__, line_count, gain, ctrl->hdrCtrl.frm_index,
               ctrl->hdrCtrl.algo_exp_tbl[ctrl->hdrCtrl.frm_index],
               ctrl->hdrCtrl.total_frames);
    if (ctrl->hdrCtrl.algo_exp_tbl[ctrl->hdrCtrl.frm_index] < 1.0 ) {
      /*reduce the exposure; reduce gain first*/
      temp_gain = gain * ctrl->hdrCtrl.algo_exp_tbl[ctrl->hdrCtrl.frm_index];
      if (temp_gain < 1.0) {
        line_count = (uint32_t) (line_count * gain *
                   ctrl->hdrCtrl.algo_exp_tbl[ctrl->hdrCtrl.frm_index]);
        gain = 1.0;
        if (line_count < min_line_cnt) {
          line_count = min_line_cnt;  /*min exposure already*/
        }
      } else {
        gain = temp_gain;
      }
    } else if (ctrl->hdrCtrl.algo_exp_tbl[ctrl->hdrCtrl.frm_index] > 1.0) {
      /*increase the exposure; increase linecount first*/
      line_count = line_count * ctrl->hdrCtrl.algo_exp_tbl[ctrl->hdrCtrl.frm_index];
    }

    if (gain > MAX_IDEAL_HDR_GAIN) { /*limited gain to 4 */
      line_count = line_count * gain /MAX_IDEAL_HDR_GAIN;
      gain = MAX_IDEAL_HDR_GAIN;
    }
    if (line_count > sensor_get.data.aec_info.max_linecount) {
      gain = line_count * gain / sensor_get.data.aec_info.max_linecount;
      line_count = sensor_get.data.aec_info.max_linecount;

      if (gain > sensor_get.data.aec_info.max_gain) {
        gain = sensor_get.data.aec_info.max_gain; /*max exposure, no room to increase*/
      }
    }

    CDBG("%s: post line_cnt =%d, gain=%f", __func__, line_count, gain);
    sensor_set.data.aec_data.gain = gain;
    sensor_set.data.aec_data.linecount = line_count;
    rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
      ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
      SENSOR_SET_EXPOSURE, &sensor_set, NULL);

    if (rc < 0)
      CDBG_ERROR("%s Sensor gain update failed ", __func__);
  }
  __end:
  ctrl->hdrCtrl.frm_index++;
  return rc;
}
