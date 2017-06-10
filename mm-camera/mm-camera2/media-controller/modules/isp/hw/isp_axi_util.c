/*============================================================================
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/videodev2.h>

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_axi.h"
#include "isp_hw.h"
#include "isp.h"
#include "isp_log.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef ENABLE_ISP_AXI_U_LOGGING
  #undef CDBG
  #define CDBG ALOGE
#endif

/** isp_axi_util_find_stream:
 *
 *    @axi:
 *    @session_id:
 *    @stream_id:
 *
 **/
isp_axi_stream_t *isp_axi_util_find_stream(isp_axi_t *axi, uint32_t session_id,
  uint32_t stream_id)
{
  int i;

  for (i = 0; i < ISP_AXI_STREAM_MAX; i++) {
    ISP_DBG(ISP_MOD_COM,"%s: stream_state = %d, session_id = %d, "
         "stream_id = %d, in_sess_id = %d, in_stream_id = %d",
         __func__, axi->streams[i].state,
         axi->streams[i].cfg.stream_param.session_id,
         axi->streams[i].cfg.stream_param.stream_id, session_id, stream_id);

    if (axi->streams[i].state != ISP_AXI_STREAM_STATE_INVALID &&
        axi->streams[i].cfg.stream_param.session_id == session_id &&
        axi->streams[i].cfg.stream_param.stream_id == stream_id)
      return &axi->streams[i];
  }

  return NULL;
}

/** isp_axi_util_find_active_video_stream:
 *
 *    @axi:
 *    @session_id:
 *
 **/
isp_axi_stream_t *isp_axi_util_find_active_video_stream(isp_axi_t *axi, uint32_t session_id)
{
  int i;

  for (i = 0; i < ISP_AXI_STREAM_MAX; i++) {
    ISP_DBG(ISP_MOD_COM,"%s: stream_state = %d, session_id = %d, "
         "stream_id = %d, in_sess_id = %d",
         __func__, axi->streams[i].state,
         axi->streams[i].cfg.stream_param.session_id,
         axi->streams[i].cfg.stream_param.stream_id, session_id);

    if (axi->streams[i].state == ISP_AXI_STREAM_STATE_ACTIVE &&
        axi->streams[i].cfg.stream_param.session_id == session_id &&
        axi->streams[i].cfg.stream_param.stream_type == CAM_STREAM_TYPE_VIDEO)
      return &axi->streams[i];
  }

  return NULL;
}
/** isp_axi_util_find_stream_handle:
 *
 *    @axi:
 *    @handle:
 *
 **/
isp_axi_stream_t *isp_axi_util_find_stream_handle(isp_axi_t *axi,
  uint32_t handle)
{
  int i;

  for (i = 0; i < ISP_AXI_STREAM_MAX; i++) {
    if (axi->streams[i].axi_stream_handle == handle &&
        axi->streams[i].state != ISP_AXI_STREAM_STATE_INVALID) {
      return &axi->streams[i];
    }
  }

  return NULL;
}

/** isp_axi_util_cam_fmt_to_v4l2_fmt:
 *
 *    @fmt:
 *    @uv_subsample:
 *
 **/
uint32_t isp_axi_util_cam_fmt_to_v4l2_fmt(
  cam_format_t fmt,
  uint32_t uv_subsample)
{
  switch (fmt) {
  case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
    return V4L2_PIX_FMT_YUYV;

  case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
    return V4L2_PIX_FMT_YVYU;

  case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
    return V4L2_PIX_FMT_UYVY;

  case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
    return V4L2_PIX_FMT_VYUY;

  case CAM_FORMAT_YUV_420_NV12_VENUS:
  case CAM_FORMAT_YUV_420_NV12:
    return (uv_subsample) ? V4L2_PIX_FMT_NV14 : V4L2_PIX_FMT_NV12;

  case CAM_FORMAT_YUV_420_NV21:
    return (uv_subsample) ? V4L2_PIX_FMT_NV41 : V4L2_PIX_FMT_NV21;

  case CAM_FORMAT_YUV_420_NV21_ADRENO:
    return (uv_subsample) ? V4L2_PIX_FMT_NV41 : V4L2_PIX_FMT_NV21;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
    return V4L2_PIX_FMT_SBGGR8;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
    return V4L2_PIX_FMT_SGBRG8;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
    return V4L2_PIX_FMT_SGRBG8;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
    return V4L2_PIX_FMT_SRGGB8;

  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
    return V4L2_PIX_FMT_QBGGR8;

  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
    return V4L2_PIX_FMT_QGBRG8;

  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
    return V4L2_PIX_FMT_QGRBG8;

  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
    return V4L2_PIX_FMT_QRGGB8;

  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
    return V4L2_PIX_FMT_SBGGR10;

  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
    return V4L2_PIX_FMT_SGBRG10;

  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
    return V4L2_PIX_FMT_SGRBG10;

  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
    return V4L2_PIX_FMT_SRGGB10;

  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
    return V4L2_PIX_FMT_QBGGR10;

  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
    return V4L2_PIX_FMT_QGBRG10;

  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
    return V4L2_PIX_FMT_QGRBG10;

  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
    return V4L2_PIX_FMT_QRGGB10;

  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
    return V4L2_PIX_FMT_SBGGR12;

  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
    return V4L2_PIX_FMT_SGBRG12;

  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
    return V4L2_PIX_FMT_SGRBG12;

  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
    return V4L2_PIX_FMT_SRGGB12;

  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
    return V4L2_PIX_FMT_QBGGR12;

  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
    return V4L2_PIX_FMT_QGBRG12;

  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
    return V4L2_PIX_FMT_QGRBG12;

  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
    return V4L2_PIX_FMT_QRGGB12;

  case CAM_FORMAT_YUV_420_YV12:
    return V4L2_PIX_FMT_YVU420;

  case CAM_FORMAT_YUV_422_NV16:
    return (uv_subsample) ? V4L2_PIX_FMT_NV14 : V4L2_PIX_FMT_NV16;

  case CAM_FORMAT_YUV_422_NV61:
    return (uv_subsample) ? V4L2_PIX_FMT_NV41 : V4L2_PIX_FMT_NV61;

  case CAM_FORMAT_JPEG_RAW_8BIT:
    return V4L2_PIX_FMT_JPEG;

  case CAM_FORMAT_META_RAW_8BIT:
  case CAM_FORMAT_META_RAW_10BIT:
    return V4L2_PIX_FMT_META;

  default:
    return 0;
  }
}

/** isp_axi_util_calculate_output_width:
 *
 *    @stream:
 *
 **/
static uint32_t isp_axi_util_calculate_output_width(isp_axi_stream_t *stream)
{
  if (stream->cfg.ispif_out_info.is_split) {
    if (stream->cfg.isp_out_info.stripe_id == ISP_STRIPE_LEFT)
      return stream->cfg.isp_out_info.left_output_width;
    else
      return stream->cfg.isp_out_info.right_output_width;
  }

  return stream->cfg.stream_param.width; /*Include padding*/
}

/** isp_axi_util_calculate_plane_addr_offset:
 *
 *    @stream:
 *
 **/
static uint32_t isp_axi_util_calculate_plane_addr_offset(
  isp_axi_stream_t *stream)
{
  if (stream->cfg.ispif_out_info.is_split) {
    if (stream->cfg.isp_out_info.stripe_id == ISP_STRIPE_LEFT)
      return 0;
    else
      return stream->cfg.isp_out_info.left_output_width;
  }

  return 0;
}

/** isp_axi_util_fill_plane_info:
 *
 *    @axi:
 *    @planes:
 *    @stream:
 *
 **/
int isp_axi_util_fill_plane_info(isp_axi_t *axi,
  struct msm_vfe_axi_plane_cfg *planes, isp_axi_stream_t *stream)
{
  int i;
  uint32_t calculated_output_width =
    isp_axi_util_calculate_output_width(stream);
  uint32_t calculated_plane_addr_offset =
    isp_axi_util_calculate_plane_addr_offset(stream);

  switch (stream->cfg.stream_param.fmt) {
  case CAM_FORMAT_YUV_420_NV12_VENUS:
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV21:
  case CAM_FORMAT_YUV_420_NV21_ADRENO:
  case CAM_FORMAT_YUV_422_NV16:
  case CAM_FORMAT_YUV_422_NV61: {
    /* two planes */
    for (i = 0; i < 2; i++) {
      planes[i].output_width = calculated_output_width; /*Include padding*/
      planes[i].output_height = stream->cfg.stream_param.height;
      planes[i].output_stride = stream->cfg.stream_param.planes.strides[i];
      planes[i].output_scan_lines = stream->cfg.stream_param.planes.scanline[i];
      planes[i].plane_addr_offset = calculated_plane_addr_offset;

      if (stream->cfg.isp_output_interface == ISP_INTF_RDI0 ||
          stream->cfg.isp_output_interface == ISP_INTF_RDI1 ||
          stream->cfg.isp_output_interface == ISP_INTF_RDI2) {
        planes[i].csid_src =
          stream->cfg.isp_output_interface - 1; /*RDI 0-2*/
      } else {
        planes[i].csid_src = 0;
      }

      if (stream->cfg.stream_param.num_cids > 1) {
        planes[i].rdi_cid =
          stream->cfg.stream_param.cid[i];/*CID 1-16*/
      } else {
        planes[i].rdi_cid =
          stream->cfg.stream_param.cid[0];/*CID 1-16*/
      }

      if (i == 0) {
        planes[i].output_plane_format = Y_PLANE; /*Y/Cb/Cr/CbCr*/
      } else {
        if (stream->cfg.stream_param.fmt == CAM_FORMAT_YUV_420_NV12 ||
            stream->cfg.stream_param.fmt == CAM_FORMAT_YUV_420_NV12_VENUS ||
            stream->cfg.stream_param.fmt == CAM_FORMAT_YUV_422_NV16) {
          planes[i].output_plane_format = CBCR_PLANE;
        } else {
          planes[i].output_plane_format = CRCB_PLANE;
        }

        if (stream->cfg.stream_param.fmt == CAM_FORMAT_YUV_420_NV12 ||
            stream->cfg.stream_param.fmt == CAM_FORMAT_YUV_420_NV12_VENUS ||
            stream->cfg.stream_param.fmt == CAM_FORMAT_YUV_420_NV21)
          planes[i].output_height /= 2;

        if (stream->cfg.need_uv_subsample) {
          planes[i].output_width  /= 2;
          planes[i].output_height /= 2;
          planes[i].output_stride /= 2;
          planes[i].plane_addr_offset /= 2;
        }
      }
    }
  }
    break;

  case CAM_FORMAT_YUV_420_YV12: {
    /* three panes */
    for (i = 0; i < 3; i++) {
      planes[i].output_width = calculated_output_width; /*Include padding*/
      planes[i].output_height = stream->cfg.stream_param.height;
      planes[i].output_stride =
        stream->cfg.stream_param.planes.strides[i];
      planes[i].output_scan_lines =
        stream->cfg.stream_param.planes.scanline[i];
      planes[i].plane_addr_offset = calculated_plane_addr_offset;

      if (stream->cfg.isp_output_interface == ISP_INTF_RDI0 ||
        stream->cfg.isp_output_interface == ISP_INTF_RDI1 ||
        stream->cfg.isp_output_interface == ISP_INTF_RDI2)
        planes[i].csid_src =
          stream->cfg.isp_output_interface - 1; /*RDI 0-2*/
      else
        planes[i].csid_src = 0;

      if (stream->cfg.stream_param.num_cids > 2)
        planes[i].rdi_cid =
          stream->cfg.stream_param.cid[i];/*CID 1-16*/
      else
        planes[i].rdi_cid =
          stream->cfg.stream_param.cid[0];/*CID 1-16*/

      if (i == 0) {
        planes[i].output_plane_format = Y_PLANE; /*Y/Cb/Cr/CbCr*/
      } else if (i == 1) {
        planes[i].output_plane_format = CB_PLANE;
      } else {
        planes[i].output_plane_format = CR_PLANE;
      }

      if (i != 0 && stream->cfg.need_uv_subsample) {
        planes[i].output_width  /= 2;
        planes[i].output_height /= 2;
        planes[i].plane_addr_offset /= 2;
      }
    }
  }
    break;

  default: {
    /* single plane */
    planes[0].output_width = calculated_output_width; /*Include padding*/
    planes[0].output_height = stream->cfg.stream_param.height;
    planes[0].output_stride = stream->cfg.stream_param.planes.strides[0];
    planes[0].output_scan_lines = stream->cfg.stream_param.planes.scanline[0];
    planes[0].plane_addr_offset = calculated_plane_addr_offset;

    if (stream->cfg.isp_output_interface == ISP_INTF_RDI0 ||
      stream->cfg.isp_output_interface == ISP_INTF_RDI1 ||
      stream->cfg.isp_output_interface == ISP_INTF_RDI2) {
      planes[0].csid_src =
        stream->cfg.isp_output_interface - 1; /*RDI 0-2*/
    } else {
      /* this will be for camif dump such as qcom raw, idea raw */
      planes[0].csid_src = ISP_INTF_PIX;
    }

    planes[0].rdi_cid = stream->cfg.stream_param.cid[0];/*CID 1-16*/
    planes[0].output_plane_format = Y_PLANE; /* define for opaque? */

    CDBG_HIGH("%s: AXI dump: single plane: isp_out_intf %d, rdi_cid %d, fmt = %d, "
     "W = %d, H = %d, stride = %d, scanline = %d\n",
     __func__, stream->cfg.isp_output_interface,
     planes[0].rdi_cid, stream->cfg.stream_param.fmt,
     planes[0].output_width, planes[0].output_height,
     planes[0].output_stride, planes[0].output_scan_lines);
    break;
  }
  }

  return 0;
}

/** isp_axi_util_subscribe_v4l2_event:
 *
 *    @axi:
 *    @event_type:
 *    @subscribe:
 *
 **/
int isp_axi_util_subscribe_v4l2_event(isp_axi_t *axi, uint32_t event_type,
  boolean subscribe)
{
  int rc = 0;
  struct v4l2_event_subscription sub;

  CDBG_ERROR("%s: event_type = 0x%x, is_subscribe = %d",  __func__, event_type,
    subscribe);

  memset(&sub, 0, sizeof(sub));
  sub.type = event_type;

  if(axi->fd < 0)
  {
    CDBG_ERROR("%s: error, isp_hw->fd is incorrect", __func__);
  }

  if (subscribe) {
    rc = ioctl(axi->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  } else {
    rc = ioctl(axi->fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
  }

  if (rc < 0)
    CDBG_ERROR("%s: error, event_type = 0x%x, is_subscribe = %d", __func__,
      event_type, subscribe);

  return rc;
}
