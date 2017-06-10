/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

   This file implements the logic to interact with the mctl device node.
   The functionalities of these modules are:

   1. Control the mctl device node.
   2. Provide the daemon process with an interface to interact
      with the mctl device node.
============================================================================*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include "camera_dbg.h"
#include "camera.h"
#include "cam_mmap.h"
#include "mctl.h"
#include "mctl_pp.h"
#include "config_proc.h"
#include "mctl_pp_node.h"

#if 0
#undef CDBG
#define CDBG LOGE
#endif

static void mctl_pp_node_data_notify(int mctl_fd)
{
//  CDBG("%s: Data path : Fd %d ", __func__, mctl_fd);
}

static void mctl_pp_node_evt_notify(int mctl_fd)
{
//  CDBG("%s: Evt path : Fd %d ", __func__, mctl_fd);
}

void mctl_pp_node_proc_evt(mctl_pp_node_obj_t *pp_node,
                           struct pollfd *poll_fds)
{
  if ((poll_fds->revents & POLLPRI)) {
    /* Process MCTL node ctrl events */
    mctl_pp_node_evt_notify(poll_fds[0].fd);
    return;
  }
  if ((poll_fds[0].revents & POLLIN) &&
      (poll_fds[0].revents & POLLRDNORM)) {
    /* Process MCTL node data events */
    mctl_pp_node_data_notify(poll_fds[0].fd);
  }

}

int mctl_pp_node_open(mctl_pp_node_obj_t *pp_node, char *dev_name)
{
  int rc = 0;
  struct v4l2_event_subscription sub;

  pp_node->fd = open(dev_name, O_RDWR);
  if (pp_node->fd <= 0) {
    CDBG_ERROR("%s Error opening mctl device node ", __func__);
    return -EINVAL;
  }

  sub.type = V4L2_EVENT_ALL;
  rc = ioctl(pp_node->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  if (rc < 0)
    CDBG_ERROR("error: ioctl VIDIOC_SUBSCRIBE_EVENT failed : %s\n",
      strerror(errno));

  return rc;
}

void mctl_pp_node_close(mctl_pp_node_obj_t *pp_node)
{
  int rc = 0;
  struct v4l2_event_subscription sub;

  sub.type = V4L2_EVENT_ALL;
  rc = ioctl(pp_node->fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
  if (rc < 0)
    CDBG_ERROR("%s mctl pp node  unsubscribe event failed: %s ",
                                  __func__, strerror(errno));

  if (pp_node->fd)
      close(pp_node->fd);
  pp_node->fd = -1;
}

static uint32_t mctl_pp_get_v4l2_fmt(cam_format_t fmt,
                                     uint8_t *num_planes)
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

static int32_t mctl_pp_node_set_fmt(mctl_pp_node_obj_t *myobj,
  cam_stream_info_def_t *strm_info)
{
  int32_t rc = 0;

  myobj->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  myobj->fmt.fmt.pix_mp.width = strm_info->width;
  myobj->fmt.fmt.pix_mp.height= strm_info->height;
  myobj->fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
  myobj->fmt.fmt.pix_mp.pixelformat =
    mctl_pp_get_v4l2_fmt(strm_info->format,
    &(myobj->fmt.fmt.pix_mp.num_planes));

  rc = ioctl(myobj->fd, VIDIOC_S_FMT, &myobj->fmt);
  if (rc < 0)
    CDBG_ERROR("%s: ioctl VIDIOC_S_FMT failed: rc=%d\n", __func__, rc);

  CDBG("%s: fd=%d, VIDIOC_S_FMT  rc=%d\n", __func__, myobj->fd, rc);
  return rc;
}

static int mctl_pp_node_set_ext_mode(mctl_pp_node_obj_t *myobj)
{
  int rc = 0;
  struct v4l2_streamparm s_parm;
  s_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  s_parm.parm.capture.extendedmode = myobj->strm_info.image_mode;

  rc = ioctl(myobj->fd, VIDIOC_S_PARM, &s_parm);

  CDBG("%s: Set extended_mode=%d rc = %d\n", __func__,
    s_parm.parm.capture.extendedmode, rc);
  return rc;
}

static int mctl_pp_node_get_inst_handle(mctl_pp_node_obj_t *myobj)
{
  int rc = 0;
  uint32_t inst_handle;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;

  v4l2_ioctl.id = MSM_V4L2_PID_INST_HANDLE;
  v4l2_ioctl.ioctl_ptr = &inst_handle;
  v4l2_ioctl.len = sizeof(inst_handle);
  rc = ioctl(myobj->fd, MSM_CAM_V4L2_IOCTL_PRIVATE_G_CTRL, &v4l2_ioctl);
  if (rc) {
    CDBG_ERROR("%s Error getting mctl pp inst handle", __func__);
    return rc;
  }

  myobj->strm_info.inst_handle = inst_handle;
  CDBG("%s: inst handle = %x rc = %d\n", __func__,
    myobj->strm_info.inst_handle, rc);
  return rc;
}

static int mctl_pp_node_qbuf(mctl_pp_node_obj_t *myobj, int idx)
{
    int32_t i, rc = 0;
    struct v4l2_buffer buffer;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buffer.memory = V4L2_MEMORY_USERPTR;
    buffer.index = idx;
    buffer.m.planes = myobj->frames[idx].planes;
    buffer.length = myobj->frames[idx].num_planes;

    CDBG("%s: fd = %d, frame idx=%d, num planes %d\n",
                 __func__, myobj->fd, idx, buffer.length);

    rc = ioctl(myobj->fd, VIDIOC_QBUF, &buffer);
    if (rc < 0) {
        CDBG_ERROR("%s: VIDIOC_QBUF error = %d\n", __func__, rc);
        return rc;
    }
    return rc;
}

static int mctl_pp_node_dqbuf(mctl_pp_node_obj_t *myobj)
{
    int32_t i, rc = 0;
    struct v4l2_buffer buffer;
    struct v4l2_plane planes[VIDEO_MAX_PLANES];

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buffer.memory = V4L2_MEMORY_USERPTR;
    buffer.m.planes = planes;
    buffer.length = myobj->fmt.fmt.pix_mp.num_planes;

    rc = ioctl(myobj->fd, VIDIOC_QBUF, &buffer);
    if (rc < 0) {
        CDBG_ERROR("%s: VIDIOC_QBUF error = %d\n", __func__, rc);
        return rc;
    }

    CDBG("%s: fd = %d, frame idx=%d, num planes %d\n",
                 __func__, myobj->fd, buffer.index, buffer.length);
    return rc;
}


static int mctl_pp_node_reg_buf(mctl_pp_node_obj_t *myobj)
{
  int i, rc = 0, j;
  struct v4l2_requestbuffers bufreq;
  int image_type;

  bufreq.count = myobj->buf_count;
  bufreq.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  bufreq.memory = V4L2_MEMORY_USERPTR;
  CDBG("%s: calling VIDIOC_REQBUFS - fd=%d, num_buf=%d, type=%d, memory=%d\n",
    __func__, myobj->fd, bufreq.count, bufreq.type, bufreq.memory);

  rc = ioctl(myobj->fd, VIDIOC_REQBUFS, &bufreq);
  if (rc < 0) {
    CDBG_ERROR("%s: fd=%d, ioctl VIDIOC_REQBUFS failed: rc=%d\n", __func__,
      myobj->fd, rc);
    goto end;
  }
  CDBG("%s: stream fd=%d, ioctl VIDIOC_REQBUFS: memtype = %d, num_frames = %d, rc=%d\n",
    __func__, myobj->fd, bufreq.memory, bufreq.count, rc);

  /* Queue all the buffers into the driver */
  for (i = 0; i < myobj->buf_count ; i++) {
    rc = mctl_pp_node_qbuf(myobj, i);
    if (rc < 0) {
      CDBG_ERROR("%s: VIDIOC_QBUF rc = %d\n", __func__, rc);
    }
  }
end:
  return rc;
}

static int mctl_pp_node_unreg_buf(mctl_pp_node_obj_t *myobj)
{
    struct v4l2_requestbuffers bufreq;
    int32_t i, rc = 0;

    bufreq.count = 0;
    bufreq.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    bufreq.memory = V4L2_MEMORY_USERPTR;
    rc = ioctl(myobj->fd, VIDIOC_REQBUFS, &bufreq);
    if (rc < 0) {
        CDBG_ERROR("%s: fd=%d, VIDIOC_REQBUFS failed, rc=%d\n",
              __func__, myobj->fd, rc);
        return rc;
    }
    CDBG("%s: fd=%d, rc=%d\n", __func__, myobj->fd, rc);
    return rc;
}

static int mctl_pp_node_get_frame_len(cam_format_t fmt_type,
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
    case CAMERA_YUV_422_YUYV:
      *num_planes = 1;
      plane[0] = PAD_TO_WORD(width * height);
      size = plane[0];
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

static int mctl_pp_node_alloc_buf(mctl_pp_node_obj_t *myobj,
  struct img_plane_info *plane_info,
  mctl_pp_node_buffer_alloc_t *buf, int ion_dev_fd)
{
  int i, j, rc = 0;
  uint32_t frame_len = 0;
  uint8_t num_planes = 0;
  uint32_t planes[VIDEO_MAX_PLANES];

  memset(planes, 0, sizeof(planes));

  for (i = 0; i < plane_info->num_planes; i++) {
	  frame_len += plane_info->plane[i].size;
	  planes[i] = plane_info->plane[i].size;
	  num_planes++;
  }
  CDBG("%s Calculated frame length = %d Dim = %d x %d", __func__, frame_len,
    myobj->strm_info.width, myobj->strm_info.height);

  for (i = 0; i < myobj->buf_count; i++) {
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
        do_munmap_ion(ion_dev_fd, &(buf[j].fd_data), buf[j].buf,
          buf[j].ion_alloc.len);
#else
        do_munmap(buf[j].fd, buf[j].buf, buf[j].buf_size);
#endif
      }
      return -1;
    }

    CDBG("%s: buf_idx=%d, vAddr=0x%x, fd=%d, handle=0x%x, len=0x%x", __func__,
      i, (unsigned int)buf[i].buf, buf[i].fd,
      (unsigned int)buf[i].ion_alloc.handle, buf[i].ion_alloc.len);

    myobj->frames[i].num_planes = num_planes;
    /* Plane 0 needs to be set seperately. Set other planes
     * in a loop. */
    myobj->frames[i].planes[0].length = planes[0];
    myobj->frames[i].planes[0].m.userptr = buf[i].fd;
    myobj->frames[i].planes[0].data_offset = 0;
    myobj->frames[i].planes[0].reserved[0] = 0;

    for (j = 1; j < num_planes; j++) {
      myobj->frames[i].planes[j].length = planes[j];
      myobj->frames[i].planes[j].m.userptr = buf[i].fd;
      myobj->frames[i].planes[j].data_offset = 0;
      myobj->frames[i].planes[j].reserved[0] =
        myobj->frames[i].planes[j-1].reserved[0] +
        myobj->frames[i].planes[j-1].length;
    }
    CDBG("%s Prepared plane info for %d planes: length %d %d fd %d ", __func__,
      myobj->frames[i].num_planes, myobj->frames[i].planes[0].length,
      myobj->frames[i].planes[1].length,
      (int)myobj->frames[i].planes[0].m.userptr);
  }
  return rc;
}

int mctl_pp_node_get_buffer_count(mctl_pp_node_obj_t *myobj)
{
  return myobj->buf_count;
}

int mctl_pp_node_get_buffer_info(mctl_pp_node_obj_t *myobj,
                                mctl_pp_local_buf_info_t *buf_data)
{
  int i, rc = 0;

  if (!myobj->buf_alloc[0].fd)
    return -EINVAL;

  CDBG("%s Filling buffer info as :", __func__);
  for (i = 0; i < myobj->buf_count; i++) {
    buf_data[i].fd = myobj->buf_alloc[i].fd;
    buf_data[i].local_vaddr = myobj->buf_alloc[i].buf;
    CDBG("%s Buffer idx %d fd %d Vaddr = %x", __func__, i,
         buf_data[i].fd, (int)buf_data[i].local_vaddr);
  }
  return rc;
}

static void mctl_pp_node_dealloc_buf(mctl_pp_node_obj_t *myobj,
                                     mctl_pp_node_buffer_alloc_t *buf,
                                     int ion_dev_fd)
{
  int i, j;
  uint32_t frame_len;
  uint8_t num_planes = 0;
  uint32_t planes[VIDEO_MAX_PLANES];

  for (i = 0; i < myobj->buf_count; i++) {
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

static int mctl_pp_node_streamon(mctl_pp_node_obj_t *myobj)
{
  int rc = 0;
  enum v4l2_buf_type buf_type;

  buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  rc = ioctl(myobj->fd, VIDIOC_STREAMON, &buf_type);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_STREAMON failed: rc=%d\n",
               __func__, rc);
    return rc;
  }

  CDBG("%s: fd=%d, VIDIOC_STREAMON rc = %d\n", __func__, myobj->fd, rc);
  return rc;
}

static int mctl_pp_node_streamoff(mctl_pp_node_obj_t *myobj)
{
  int rc = 0;
  enum v4l2_buf_type buf_type;

  buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  rc = ioctl(myobj->fd, VIDIOC_STREAMOFF, &buf_type);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_STREAMOFF failed: rc=%d\n",
               __func__, rc);
    return rc;
  }

  CDBG("%s: fd=%d, VIDIOC_STREAMOFF rc = %d\n", __func__, myobj->fd, rc);
  return rc;
}

static int mctl_pp_node_set_plane_info(mctl_pp_node_obj_t *myobj,
  struct img_plane_info *info)
{
  int rc = 0;
  struct v4l2_control control;

  memset(&control, 0, sizeof(control));
  control.id = MSM_V4L2_PID_PP_PLANE_INFO;
  control.value = (int32_t)info;
  rc = ioctl(myobj->fd, VIDIOC_S_CTRL, &control);

  CDBG("%s: fd=%d, VIDIOC_S_CTRL id=0x%x rc = %d\n", __func__, myobj->fd,
    control.id, rc);
  return rc;
}

int mctl_pp_node_prepare(void *ctrl, mctl_pp_node_obj_t *pp_node, int ion_dev_fd)
{
  int rc = 0;
  struct img_plane_info plane_info;

  rc = mctl_pp_node_set_fmt(pp_node, &pp_node->strm_info);
  if (rc < 0) {
    CDBG_ERROR("%s Error setting format ", __func__);
    return rc;
  }

  rc = mctl_pp_node_set_ext_mode(pp_node);
  if (rc < 0) {
    CDBG_ERROR("%s Error setting image mode ", __func__);
    return rc;
  }

  rc = mctl_pp_node_get_inst_handle(pp_node);
  if (rc < 0) {
    CDBG_ERROR("%s Error getting inst handle ", __func__);
    return rc;
  }

  plane_info.width = pp_node->fmt.fmt.pix_mp.width;
  plane_info.height = pp_node->fmt.fmt.pix_mp.height;
  plane_info.pixelformat = pp_node->fmt.fmt.pix_mp.pixelformat;
  plane_info.buffer_type = pp_node->fmt.type;
  plane_info.ext_mode = pp_node->strm_info.image_mode;
  plane_info.num_planes = pp_node->fmt.fmt.pix_mp.num_planes;
  rc = config_plane_info((void *)ctrl, (void *)&plane_info);
  if (rc < 0) {
    CDBG_ERROR("%s Error getting plane info ", __func__);
    return rc;
  }

  rc = mctl_pp_node_set_plane_info(pp_node, &plane_info);
  if (rc < 0) {
    CDBG_ERROR("%s Error setting plane info ", __func__);
    return rc;
  }

  memset(&pp_node->buf_alloc, 0, sizeof(pp_node->buf_alloc));
  pp_node->buf_count = MCTL_PP_VIDEO_BUFFER_CNT;
  rc = mctl_pp_node_alloc_buf(pp_node, &plane_info, pp_node->buf_alloc, ion_dev_fd);
  if (rc < 0) {
    CDBG_ERROR("%s Error allocating buffers ", __func__);
    return rc;
  }

  rc = mctl_pp_node_reg_buf(pp_node);
  if (rc < 0) {
    CDBG_ERROR("%s Error registering buffers with mctl pp node", __func__);
    return rc;
  }

  rc = mctl_pp_node_streamon(pp_node);
  if (rc < 0) {
    CDBG_ERROR("%s Error calling STREAMON on mctl pp node", __func__);
    return rc;
  }

  return rc;
}

int mctl_pp_node_release(mctl_pp_node_obj_t *pp_node, int ion_dev_fd)
{
  int rc = 0;

  CDBG("%s E ", __func__);

  rc = mctl_pp_node_streamoff(pp_node);
  if (rc < 0) {
    CDBG_ERROR("%s Error calling STREAMOFF on mctl pp node", __func__);
    return rc;
  }

  rc = mctl_pp_node_unreg_buf(pp_node);
  if (rc < 0) {
    CDBG_ERROR("%s Error registering buffers with mctl pp node", __func__);
    return rc;
  }

  mctl_pp_node_dealloc_buf(pp_node, pp_node->buf_alloc, ion_dev_fd);

  return rc;
}
