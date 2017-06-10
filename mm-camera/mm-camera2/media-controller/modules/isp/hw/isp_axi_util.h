/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_AXI_UTIL_H__
#define __ISP_AXI_UTIL_H__

isp_axi_stream_t *isp_axi_util_find_stream(isp_axi_t *axi, uint32_t session_id,
  uint32_t stream_id);
isp_axi_stream_t *isp_axi_util_find_active_video_stream(isp_axi_t *axi,
  uint32_t session_id);
isp_axi_stream_t *isp_axi_util_find_stream_handle(isp_axi_t *axi,
  uint32_t handle);
uint32_t isp_axi_util_cam_fmt_to_v4l2_fmt(cam_format_t fmt,
  uint32_t uv_subsample);
int isp_axi_util_fill_plane_info(isp_axi_t *axi,
  struct msm_vfe_axi_plane_cfg *planes, isp_axi_stream_t *stream);
int isp_axi_util_subscribe_v4l2_event(isp_axi_t *axi, uint32_t event_type,
  boolean subscribe);

#endif /* __ISP_AXI_UTIL_H__ */
