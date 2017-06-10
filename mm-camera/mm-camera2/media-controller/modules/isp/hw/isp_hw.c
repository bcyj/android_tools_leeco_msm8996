/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#define ATRACE_TAG ATRACE_TAG_ALWAYS
#include <cutils/trace.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/videodev2.h>

#include "camera_dbg.h"
#include "cam_types.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "mct_bus.h"
#include "modules.h"
#include "isp_def.h"
#include "isp_event.h"
#include "isp_def.h"
#include "isp_ops.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "isp_axi.h"
#include "isp_hw.h"
#include "isp.h"
#include "q3a_stats_hw.h"
#include "isp_log.h"
#include "server_debug.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef ENABLE_ISP_HW_LOGGING
  #undef CDBG
  #define CDBG ALOGE
#endif


/*to enable isp hw dump, use these two define*/
/*#define ISP_REG_DUMP*/
/*#define ISP_DMI_DUMP*/

/* 10% margin for vfe line blanking */
#define ISP_LINE_TIME_MARGIN 0.1

extern void *isp_hw_create_pipeline(int fd, uint32_t isp_version, int dev_idx,
  void *buf_mgr);
extern void *isp_hw_create_axi(int fd, uint32_t isp_version, int dev_idx,
  void *buf_mgr);
static int isp_hw_subscribe_pipeline_event(isp_hw_t *isp_hw,
  boolean subscribe);
static int isp_hw_proc_action_divert_ack(isp_hw_t *isp_hw,
  isp_hw_buf_divert_ack_t *ack);
static int isp_hw_action(void *ctrl, uint32_t action_code, void *action_data,
  uint32_t action_data_size);
static int isp_hw_proc_ihist_la_trigger_update(isp_hw_t *isp_hw,
  q3a_ihist_stats_t *ihist_stats, uint32_t in_params_size);
static int isp_hw_proc_awb_trigger_update(isp_hw_t *isp_hw,
  stats_update_t *stats_update, uint32_t in_params_size);
static int isp_hw_proc_aec_trigger_update(isp_hw_t *isp_hw,
  float *aec_gain, uint32_t in_params_size);
static int isp_hw_set_crop_factor(isp_hw_t *isp_hw,
  isp_hw_set_crop_factor_t *crop_factor, uint32_t size);
static int isp_hw_get_roi_map(isp_hw_t *isp_hw, void *out_params,
  uint32_t out_params_size);
static int isp_hw_subscribe_one_v4l2_event(isp_hw_t *isp_hw,
  uint32_t event_type, boolean subscribe);
/** isp_hw_find_session:
 *
 *    @isp_hw: isp hw
 *    @session_id: session id
 *
 *  Find a session by given session id
 *
 *  Return to hw session. If not found return NULL
 **/
static isp_hw_session_t *isp_hw_find_session(isp_hw_t *isp_hw,
  uint32_t session_id)
{
  int i;

  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (isp_hw->session[i].isp_hw &&
        isp_hw->session[i].session_id == session_id) {
      return &isp_hw->session[i];
    }
  }

  return NULL;
}

/** isp_hw_find_stream_in_session:
 *
 *    @session: hw session
 *    @stream_id: stream id
 *
 *  Find a stream by given hw session and stream id
 *
 *  Return hw stream. If not found return NULL
 **/
static isp_hw_stream_t *isp_hw_find_stream_in_session(
  isp_hw_session_t *session, uint32_t stream_id)
{
  int i;
  isp_hw_stream_t *stream;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    stream = &session->streams[i];

    if (stream->state == ISP_HW_STREAM_STATE_NULL) {
      continue;
    } else if (stream->cfg.stream_id == stream_id) {
      return stream;
    }
  }

  return NULL;
}

/** isp_hw_notify_buf_divert_event:
 *
 *    @isp_hw: isp hw
 *    @isp_event_data: isp event data
 *
 *  Returns 0 for success and negative error on failure
 **/
static int isp_hw_notify_buf_divert_event(isp_hw_t *isp_hw,
  struct msm_isp_event_data *isp_event_data)
{
  int rc = 0;
  isp_frame_divert_notify_t event;
  isp_hw_buf_divert_ack_t ack;

  memset(&event,  0,  sizeof(isp_frame_divert_notify_t));
  event.isp_event_data = isp_event_data;

  /* clear the native stream_id bit to restore isp stream_id */
  isp_event_data->u.buf_done.stream_id &= ~ISP_NATIVE_BUF_BIT;

  rc = isp_hw->notify_ops->notify(
    (void*)isp_hw->notify_ops->parent, isp_hw->init_params.dev_idx,
    ISP_HW_NOTIFY_BUF_DIVERT, &event,
    sizeof(isp_frame_divert_notify_t));

  return rc;
}

/** isp_hw_fill_stream_plane_info:
 *
 *    @planes: plane info
 *    @stream: hw stream
 *
 *    Clear and initialize planes info
 **/
static void isp_hw_fill_stream_plane_info(isp_plane_info_t *planes,
  isp_hw_stream_t *stream)
{
  int i;
  cam_stream_buf_plane_info_t *plane_info;

  memset(planes, 0, sizeof(isp_plane_info_t));
  plane_info = &stream->cfg.stream_info.buf_planes;
  planes->num_planes =
    stream->cfg.stream_info.buf_planes.plane_info.num_planes;

  for (i = 0; i < planes->num_planes; i++) {
    planes->strides[i] = plane_info->plane_info.mp[i].stride;
    planes->scanline[i] = plane_info->plane_info.mp[i].scanline;
    ISP_DBG(ISP_MOD_COM,"%s: fmt = %d, plane idx = %d, width = 0x%x, stride = 0x%x, scanline = 0x%x\n",
      __func__, stream->cfg.stream_info.fmt, i,
      stream->cfg.stream_info.dim.width, planes->strides[i],
      planes->scanline[i]);
  }
}

/** isp_hw_get_axi_src_type:
 *
 *    @isp_hw: isp hw
 *    @stream: hw stream
 *    @is_encoder:
 *
 *    Get stream format
 *
 *    Return stream format
 **/
static enum msm_vfe_axi_stream_src isp_hw_get_axi_src_type(isp_hw_t *isp_hw,
  isp_hw_stream_t *stream, uint8_t is_encoder)
{
  switch (stream->cfg.stream_info.fmt) {
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
    return CAMIF_RAW;

  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:
    return IDEAL_RAW;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
  case CAM_FORMAT_JPEG_RAW_8BIT:
    return RDI_INTF_0;

  case CAM_FORMAT_META_RAW_8BIT:
  case CAM_FORMAT_META_RAW_10BIT:
    return RDI_INTF_1;

  default: {
    if (is_encoder)
      return PIX_ENCODER;
    else
      return PIX_VIEWFINDER;
  }
    break;
  }

  return VFE_AXI_SRC_MAX;
}

/** isp_hw_find_primary_cid:
 *
 *    @sensor_cfg: sensor config
 *    @sensor_cap:
 *
 *    Return
 **/
uint32_t isp_hw_find_primary_cid(sensor_out_info_t *sensor_cfg,
  sensor_src_port_cap_t *sensor_cap)
{
  int i;

  if(SENSOR_CID_CH_MAX < sensor_cap->num_cid_ch) {
    CDBG_ERROR("%s:%d error out of range", __func__, __LINE__);
    return(0);
  }

  for(i = 0; i < sensor_cap->num_cid_ch; i++)
    if (sensor_cfg->fmt == sensor_cap->sensor_cid_ch[i].fmt)
      break;

  if(i == sensor_cap->num_cid_ch) {
    CDBG_ERROR("%s:%d error cannot find primary sensor format", __func__,
      __LINE__);
    return(0);
  }

  return(i);
}

/** isp_hw_camif_cfg:
 *
 *    @isp_hw: isp hw
 *    @stream: stream hw
 *    @axi_path:
 *
 *  Returns 0 for success and negative error on failure
 **/
static int isp_hw_camif_cfg(isp_hw_t *isp_hw, isp_hw_stream_t *stream,
  enum msm_vfe_axi_stream_src axi_path)
{
  int rc = 0;
  isp_pix_camif_cfg_t camif_cfg;
  isp_hw_session_t *session;
  uint32_t primary_cid_idx;
  int input_src = 0;
  int rdi_id = 0;;

  primary_cid_idx = isp_hw_find_primary_cid(&stream->cfg.sensor_out_info,
    &stream->cfg.sink_cap);

  if (SENSOR_CID_CH_MAX - 1 < primary_cid_idx) {
    CDBG_ERROR("%s:%d error out of range\n", __func__, __LINE__);
    return -1;
  }

  memset(&camif_cfg,  0,  sizeof(camif_cfg));
  camif_cfg.sensor_out_info = stream->cfg.sensor_out_info;
  camif_cfg.ispif_out_info = stream->cfg.ispif_out_info;
  camif_cfg.sensor_output_fmt =
    stream->cfg.sink_cap.sensor_cid_ch[primary_cid_idx].fmt;
  camif_cfg.is_bayer_sensor =
    stream->cfg.sink_cap.sensor_cid_ch[primary_cid_idx].is_bayer_sensor;
  camif_cfg.axi_path = axi_path;

  if (axi_path >= VFE_AXI_SRC_MAX) {
    CDBG_ERROR("%s: Invalid axi_path %d\n", __func__, axi_path);
    return -1;
  }

  if (axi_path < RDI_INTF_0) {
    input_src = VFE_PIX_0;
  } else {
    rdi_id = axi_path - RDI_INTF_0;
    input_src = rdi_id + VFE_RAW_0;
  }

  session = isp_hw_find_session(isp_hw,
    isp_hw->pipeline.session_id[input_src]);

  if (!session) {
    CDBG_ERROR("%s: can not find session\n", __func__);
    return -1;
  }

  camif_cfg.hfr_mode = session->hfr_param.hfr_mode;
  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_CAMIF_CFG, (void *)&camif_cfg, sizeof(camif_cfg));

  if (rc < 0) {
    CDBG_ERROR("%s: PIX stream cfg error = %d\n", __func__, rc);
    return rc;
  }

  return rc;
}
/** isp_hw_dump_camif_cfg:
 *
 *    @pix_cfg:
 *
 *    Logs pix config
 **/
static void isp_hw_camif_dump_cfg(struct msm_vfe_pix_cfg *pix_cfg)
{
  ISP_DBG(ISP_MOD_COM,"%s: =====dump Camif cfg for PIX interface====\n", __func__);
  ISP_DBG(ISP_MOD_COM,"%s: camif input type = %d\n", __func__,
    pix_cfg->camif_cfg.camif_input);
  ISP_DBG(ISP_MOD_COM,"%s: camif pixel pattern = %d\n", __func__, pix_cfg->pixel_pattern);
  ISP_DBG(ISP_MOD_COM,"%s: camif input_format= %d\n", __func__, pix_cfg->input_format);

  ISP_DBG(ISP_MOD_COM,"%s: camif first_pix = %d\n", __func__, pix_cfg->camif_cfg.first_pixel);
  ISP_DBG(ISP_MOD_COM,"%s: camif last_pix = %d\n", __func__, pix_cfg->camif_cfg.last_pixel);
  ISP_DBG(ISP_MOD_COM,"%s: camif first_line = %d\n", __func__, pix_cfg->camif_cfg.first_line);
  ISP_DBG(ISP_MOD_COM,"%s: camif last_line = %d\n", __func__, pix_cfg->camif_cfg.first_line);
  ISP_DBG(ISP_MOD_COM,"%s: camif pixels per line = %d\n",
    __func__,  pix_cfg->camif_cfg.pixels_per_line);
  ISP_DBG(ISP_MOD_COM,"%s: camif lines per frame = %d\n", __func__,
    pix_cfg->camif_cfg.lines_per_frame);
}

/** isp_hw_input_cfg:
 *
 *    @isp_hw: isp hw
 *    @intf: isp interface type
 *
 *  Returns 0 for success and negative error on failure
 **/
static int isp_hw_input_cfg(isp_hw_t *isp_hw, isp_intf_type_t intf)
{
  int rc = 0;
  struct msm_vfe_input_cfg input_cfg;

  if (intf == ISP_INTF_PIX) {
    isp_camif_cfg_t *camif_cfg = &isp_hw->input.camif_cfg;
    struct msm_vfe_pix_cfg *pix_cfg = &input_cfg.d.pix_cfg;

    /*one interface only allow one ref count*/
    if (camif_cfg->ref_count)
      return rc;

    camif_cfg->ref_count++;

    input_cfg.input_src = VFE_PIX_0;
    input_cfg.input_pix_clk = camif_cfg->pixel_clock;

    /* For meta rdi streaming the pipeline is not used.
     * Do we see issue with vfe clock when enabling RDI dumping?
     * Remove the hard coded 320 clock. We will re-evaluate
     * it when hitting issue in vHDR testing.
     */

    /*input_cfg.input_pix_clk = ISP_PIX_CLK_320MHZ;*/
    pix_cfg->input_mux = CAMIF;
    pix_cfg->pixel_pattern = camif_cfg->pixel_pattern;
    pix_cfg->camif_cfg.camif_input = CAMIF_MIPI_INPUT;
    pix_cfg->camif_cfg.first_line = camif_cfg->first_line;
    pix_cfg->camif_cfg.last_line = camif_cfg->last_line;
    pix_cfg->camif_cfg.first_pixel = camif_cfg->first_pixel;
    pix_cfg->camif_cfg.last_pixel = camif_cfg->last_pixel;
    pix_cfg->camif_cfg.lines_per_frame = camif_cfg->lines_per_frame;
    pix_cfg->camif_cfg.pixels_per_line = camif_cfg->pixels_per_line;
    pix_cfg->input_format = camif_cfg->input_format;
    isp_hw_camif_dump_cfg(pix_cfg);
  } else if (intf < ISP_INTF_MAX) {
    int rdi = intf - ISP_INTF_RDI0;

    if (isp_hw->input.rdi_cfg[rdi].ref_count)
      return rc;

    isp_hw->input.rdi_cfg[rdi].ref_count++;

    input_cfg.input_src = VFE_RAW_0 + rdi;
    input_cfg.input_pix_clk = isp_hw->input.rdi_cfg[rdi].pixel_clock;
    input_cfg.d.rdi_cfg.cid = isp_hw->input.rdi_cfg[rdi].cid;
    input_cfg.d.rdi_cfg.frame_based = isp_hw->input.rdi_cfg[rdi].frame_based;
  } else {
    rc = -1;
    return rc;
  }


  rc = ioctl(isp_hw->fd, VIDIOC_MSM_ISP_INPUT_CFG, &input_cfg);
  if (rc < 0) {
    CDBG_ERROR("%s: input cfg error = %d\n", __func__, rc);
  }

  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d", __func__, rc);

  return rc;
}

/** isp_input_set_parms:
 *
 *    @isp_hw: isp hw
 *    @stream: stream hw
 *    @intf: isp interface
 *
 *  Returns 0 for success and negative error on failure
 **/
static int isp_input_set_parms(isp_hw_t *isp_hw,
  isp_hw_stream_t *stream, isp_intf_type_t intf)
{
  int rc = 0;

  uint32_t primary_cid_idx = isp_hw_find_primary_cid(
    &stream->cfg.sensor_out_info, &stream->cfg.sink_cap);

  if (SENSOR_CID_CH_MAX - 1 < primary_cid_idx) {
    CDBG_ERROR("%s:%d error out of range\n", __func__, __LINE__);
    return -1;
  }

  if (intf == ISP_INTF_PIX) {
    sensor_out_info_t *sensor_info = &stream->cfg.sensor_out_info;

    if (stream->cfg.ispif_out_info.is_split) {
      if (stream->cfg.isp_out_info.stripe_id == ISP_STRIPE_LEFT) {
        isp_hw->input.camif_cfg.first_line =
          sensor_info->request_crop.first_line;
        isp_hw->input.camif_cfg.last_line =
          sensor_info->request_crop.last_line;
        isp_hw->input.camif_cfg.first_pixel =
          sensor_info->request_crop.first_pixel;
        isp_hw->input.camif_cfg.last_pixel =
          stream->cfg.ispif_out_info.right_stripe_offset +
          stream->cfg.ispif_out_info.overlap - 1;
        isp_hw->input.camif_cfg.pixels_per_line =
          stream->cfg.ispif_out_info.right_stripe_offset +
          stream->cfg.ispif_out_info.overlap;
        isp_hw->input.camif_cfg.lines_per_frame =
          sensor_info->dim_output.height;
      } else {
        isp_hw->input.camif_cfg.first_line =
          sensor_info->request_crop.first_line;
        isp_hw->input.camif_cfg.last_line =
          sensor_info->request_crop.last_line;
        isp_hw->input.camif_cfg.first_pixel = 0;
        isp_hw->input.camif_cfg.last_pixel =
          sensor_info->request_crop.last_pixel -
          stream->cfg.ispif_out_info.right_stripe_offset;
        isp_hw->input.camif_cfg.pixels_per_line =
          sensor_info->dim_output.width -
          stream->cfg.ispif_out_info.right_stripe_offset;
        isp_hw->input.camif_cfg.lines_per_frame =
          sensor_info->dim_output.height;
      }
      isp_hw->input.camif_cfg.pixel_clock =
        isp_hw->input.camif_cfg.pixels_per_line *
        (1 + ISP_LINE_TIME_MARGIN) *
        sensor_info->vt_pixel_clk / sensor_info->ll_pck;
    } else {
      isp_hw->input.camif_cfg.first_line =
        sensor_info->request_crop.first_line;
      isp_hw->input.camif_cfg.last_line = sensor_info->request_crop.last_line;
      isp_hw->input.camif_cfg.first_pixel =
        sensor_info->request_crop.first_pixel;
      isp_hw->input.camif_cfg.last_pixel =
        sensor_info->request_crop.last_pixel;
      isp_hw->input.camif_cfg.pixels_per_line = sensor_info->dim_output.width;
      isp_hw->input.camif_cfg.lines_per_frame = sensor_info->dim_output.height;
      isp_hw->input.camif_cfg.pixel_clock = sensor_info->op_pixel_clk;
    }

    isp_hw->input.camif_cfg.pixel_pattern =
      isp_fmt_to_pix_pattern(
        stream->cfg.sink_cap.sensor_cid_ch[primary_cid_idx].fmt);
    isp_hw->input.camif_cfg.input_format =
       isp_cam_fmt_to_v4l2_fmt(stream->cfg.sink_cap.sensor_cid_ch[primary_cid_idx].fmt, 0);
  } else if (intf < ISP_INTF_MAX) {
    int rdi = intf - ISP_INTF_RDI0;
    if (stream->cfg.meta_ch_idx_mask == 0) {
      /* this is normal RDI dump stream */
      sensor_out_info_t *sensor_info = &stream->cfg.sensor_out_info;
      isp_hw->input.rdi_cfg[rdi].cid =
        stream->cfg.sink_cap.sensor_cid_ch[primary_cid_idx].cid;
      isp_hw->input.rdi_cfg[rdi].frame_based = 1;
    } else {
      int meta_cid_idx = 0;
      sensor_out_info_t *sensor_info = &stream->cfg.sensor_out_info;
      if (stream->cfg.meta_ch_idx_mask != 1) {
        CDBG_ERROR("%s: now we only supprt one meta data, mask = 0x%x\n",
          __func__, stream->cfg.meta_ch_idx_mask);
        return -1;
      }
      meta_cid_idx = stream->cfg.sink_cap.num_cid_ch;
      isp_hw->input.rdi_cfg[rdi].cid =
        stream->cfg.sink_cap.sensor_cid_ch[meta_cid_idx].cid;
      isp_hw->input.rdi_cfg[rdi].frame_based = 1;
    }
    /* in the case that both pipeline straem and RDI
     * coexisting we take pipeline stream's clock.
     */
    if (isp_hw->input.camif_cfg.pixel_clock == 0) {
        isp_hw->input.rdi_cfg[rdi].pixel_clock =
        stream->cfg.sensor_out_info.op_pixel_clk;
    }
  } else {
    rc = -1;
  }

  return rc;
}

/** isp_hw_input_uncfg:
 *
 *    @isp_hw: isp hw
 *    @intf: isp interface type
 *
 **/
static void isp_hw_input_uncfg(isp_hw_t *isp_hw, isp_intf_type_t intf)
{
  if (intf == ISP_INTF_PIX) {
    if (isp_hw->input.camif_cfg.ref_count)
      isp_hw->input.camif_cfg.ref_count--;
  } else if (intf < ISP_INTF_MAX) {
    int rdi = intf - ISP_INTF_RDI0;

    if (isp_hw->input.rdi_cfg[rdi].ref_count)
      isp_hw->input.rdi_cfg[rdi].ref_count--;
  }
}

/** isp_hw_set_stream_hfr_skip_pattern:
 *
 *    @session: hw session
 *    @stream: hw stream
 *    @stream_cfg:
 *
 *    Init hfr skip pattern
 **/
static void isp_hw_set_stream_hfr_skip_pattern(isp_hw_session_t *session,
  isp_hw_stream_t *stream, isp_hwif_output_cfg_t *stream_cfg)
{
  stream_cfg->frame_skip_period = 32; /* 32 frames period */
  stream_cfg->frame_skip_pattern = stream->cfg.skip_pattern;
}

/** isp_hw_sel_axi_path:
 *
 *    @session: hw session
 *    @stream: hw stream
 *    @stream_cfg:
 *
 *    Map isp interface to axi
 **/
static void isp_hw_sel_axi_path(isp_hw_t *isp_hw,
  isp_hw_stream_t *stream, isp_hwif_output_cfg_t *stream_cfg)
{
  stream_cfg->isp_output_interface = ISP_INTF_PIX;

  if (stream->cfg.use_pix) {
    stream_cfg->axi_path =
      isp_hw_get_axi_src_type(isp_hw, stream, stream->cfg.is_encoder);
  } else {
    CDBG_HIGH("%s: meta_idx_mask = 0x%x, identity = 0x%x, "
      "hw_stream_id = %d, vfe_output_mask = 0x%x\n",
      __func__, stream->cfg.meta_ch_idx_mask,
      stream->cfg.stream_info.identity,
      stream->cfg.stream_id, stream->cfg.vfe_output_mask);

    if (stream->cfg.vfe_output_mask == (1 << ISP_INTF_RDI0)) {
      stream_cfg->isp_output_interface = ISP_INTF_RDI0;
      stream_cfg->axi_path = RDI_INTF_0;
    } else if (stream->cfg.vfe_output_mask == (1 << ISP_INTF_RDI1)) {
      stream_cfg->isp_output_interface = ISP_INTF_RDI1;
      stream_cfg->axi_path = RDI_INTF_1;
    } else if (stream->cfg.vfe_output_mask == (1 << ISP_INTF_RDI2)) {
      stream_cfg->isp_output_interface = ISP_INTF_RDI2;
      stream_cfg->axi_path = RDI_INTF_2;
    } else {
      CDBG_ERROR("%s: Invalid vfe output mask: 0x%8x\n", __func__,
        stream->cfg.vfe_output_mask);
    }
    stream_cfg->stream_param.frame_base = 1;
  }
}

/** isp_hw_get_meta_idx_from_mask:
 *
 *    @meta_ch_idx_mask:
 *
 **/
static int isp_hw_get_meta_idx_from_mask(uint32_t meta_ch_idx_mask)
{
  if (meta_ch_idx_mask <= 0) {
    return -1;
  } else if (meta_ch_idx_mask == 1) {
    return 0;
  } else {
    CDBG_ERROR("%s: only support one meta data now, mask = 0x%x\n",
      __func__, meta_ch_idx_mask);
  }

  return -100;
}

/** isp_hw_fill_stream_cfg:
 *
 *    @isp_hw: isp hw
 *    @session: hw session
 *    @stream: hw stream
 *    @stream_cfg:
 *
 **/
static void isp_hw_fill_output_cfg(isp_hw_t *isp_hw, isp_hw_session_t *session,
  isp_hw_stream_t *stream, isp_hwif_output_cfg_t *output_cfg)
{
  int k;

  isp_hw_set_stream_hfr_skip_pattern(session, stream, output_cfg);

  output_cfg->need_divert = stream->cfg.need_divert;
  output_cfg->need_uv_subsample = stream->cfg.need_uv_subsample;
  output_cfg->ispif_out_info = stream->cfg.ispif_out_info;
  output_cfg->isp_out_info = stream->cfg.isp_out_info;
  output_cfg->use_native_buf = stream->cfg.use_native_buf;
  output_cfg->stream_param.stream_type = stream->cfg.stream_info.stream_type;
  output_cfg->stream_param.session_id = session->session_id;
  output_cfg->stream_param.stream_id = stream->cfg.stream_id;
  output_cfg->stream_param.num_burst = stream->cfg.stream_info.num_burst;
  output_cfg->stream_param.sensor_skip_cnt =
    stream->cfg.sensor_out_info.num_frames_skip;
  output_cfg->stream_param.streaming_mode =
    stream->cfg.stream_info.streaming_mode;
  output_cfg->frame_skip_pattern = stream->cfg.skip_pattern;

  if (stream->cfg.meta_ch_idx_mask == 0) {
    /* non meta data stream. */
    output_cfg->stream_param.num_cids =
      stream->cfg.sink_cap.num_cid_ch - stream->cfg.sink_cap.num_meta_ch;
    for (k = 0; k < output_cfg->stream_param.num_cids; k++) {
      output_cfg->stream_param.cid[k] =
        stream->cfg.sink_cap.sensor_cid_ch[k].cid;
    }
    output_cfg->stream_param.width = stream->cfg.stream_info.dim.width;
    output_cfg->stream_param.height = stream->cfg.stream_info.dim.height;
    output_cfg->stream_param.fmt = stream->cfg.stream_info.fmt;
  } else {
    int meta_idx = isp_hw_get_meta_idx_from_mask(stream->cfg.meta_ch_idx_mask);
    int meta_cid_start_idx =
      stream->cfg.sink_cap.num_cid_ch - stream->cfg.sink_cap.num_meta_ch;

    /* one meta stream has only one cid. */
    output_cfg->stream_param.num_cids = 1;
    output_cfg->stream_param.cid[0] =
      stream->cfg.sink_cap.sensor_cid_ch[meta_cid_start_idx + meta_idx].cid;
    output_cfg->stream_param.width = stream->cfg.stream_info.dim.width;
    output_cfg->stream_param.height = stream->cfg.stream_info.dim.height;
    output_cfg->stream_param.fmt = stream->cfg.stream_info.fmt;
  }

  output_cfg->ion_fd = session->ion_fd;
  output_cfg->vt_enable = session->vt_enable;

  switch(GET_ISP_MAIN_VERSION(isp_hw->init_params.isp_version)){
  case ISP_VERSION_32:
    output_cfg->burst_len = VFE32_BURST_LEN;
    break;
  case ISP_VERSION_40: {
    if(isp_hw->init_params.cap.hw_ver == ISP_MSM8916 ||
       isp_hw->init_params.cap.hw_ver == ISP_MSM8939 ) {
         output_cfg->burst_len = VFE40_BURST_LEN_2;
    } else {
         output_cfg->burst_len = VFE40_BURST_LEN_1;
    }
  }
    break;
  case ISP_VERSION_44:
    output_cfg->burst_len = VFE44_BURST_LEN;
    break;
  default:
    output_cfg->burst_len = VFE40_BURST_LEN_1;
    break;
  }
  isp_hw_fill_stream_plane_info(&output_cfg->stream_param.planes, stream);
  isp_hw_sel_axi_path(isp_hw, stream, output_cfg);
}

/** isp_hw_stream_config:
 *
 *    @isp_hw: isp hw
 *    @session: hw session
 *    @stream: hw stream
 *
 *    Stream config
 **/
static int isp_hw_stream_config(isp_hw_t *isp_hw,
  isp_hw_session_t *session, isp_hw_stream_t *stream)
{
  isp_hwif_output_cfg_t output_cfg;
  int i, k, rc = 0;
  uint32_t encoder_len = 0, tmp_len;
  isp_hw_stream_t *encoder_stream = NULL;
  int ion_fd = 0;

  if (session->active_stream > 0 && stream->cfg.use_pix) {
    /* do not allow reconfig pipeline when it's running */
    return 0;
  }

  ion_fd = session->ion_fd;
  for (i = 0; i < VFE_SRC_MAX; i ++) {
    if (stream->cfg.vfe_output_mask & (1 << i)) {
      isp_hw->pipeline.session_id[i] = session->session_id;
    }
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_ION_FD, (void *)&ion_fd, sizeof(int));

  if (stream->state == ISP_HW_STREAM_STATE_NULL) {
    CDBG_ERROR("%s: stream in invalid state\n", __func__);
    return -1;
  }

  memset(&output_cfg, 0, sizeof(output_cfg));

  /*fill in stream info to stream cfg*/
  isp_hw_fill_output_cfg(isp_hw, session, stream, &output_cfg);
  /* config pix stream*/
  if (stream->cfg.use_pix) {
    isp_hw_camif_cfg(isp_hw, stream, output_cfg.axi_path);

    rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
      ISP_PIX_SET_STREAM_CFG, (void *)&output_cfg, sizeof(output_cfg));

    if (rc < 0) {
      CDBG_ERROR("%s: PIX stream cfg error = %d\n", __func__, rc);
      return rc;
    }
  }

  /*fill in Camif/RDI param to isp_hw*/
  rc = isp_input_set_parms(isp_hw, stream,
    output_cfg.isp_output_interface);

  /* cmaif/Camif input cfg need to sit here, input format need to set to kernel
     before VIDIOC_MSM_ISP_REQUEST_STREAM (ISP_AXI_SET_STREAM_CFG)*/
  rc = isp_hw_input_cfg(isp_hw, output_cfg.isp_output_interface);

  /*config axi stream, create/request axi stream to kernel*/
  rc = isp_axi_set_params(isp_hw->axi.private_data, ISP_AXI_SET_STREAM_CFG,
    (void *)&output_cfg, sizeof(output_cfg));

  if (rc < 0) {
    CDBG_ERROR("%s: AXI stream cfg error = %d\n", __func__, rc);
    return rc;
  }

  return rc;
}

/** isp_hw_count_num_pix_streams:
 *
 *    @isp_hw: isp hw
 *    @params: start/stop properties
 *
 *    Return count of pix streams.
 **/
static int isp_hw_count_num_pix_streams(isp_hw_t *isp_hw,
  start_stop_stream_t *params)
{
  int i, cnt = 0;
  isp_hw_stream_t *stream = NULL;
  isp_hw_session_t *session = NULL;
  session = isp_hw_find_session(isp_hw, params->session_id);

  if (!session) {
    CDBG_ERROR("%s: cannot find session with sessionid = %d\n",
               __func__, params->session_id);
    return 0;
  }

  for (i = 0; i < params->num_streams; i++) {
    stream = isp_hw_find_stream_in_session(session,
      params->stream_ids[i]);
    if (stream) {
      if (stream->cfg.use_pix)
        cnt++;
    }
  }

  return cnt;
}

/** isp_hw_subscribe_one_v4l2_event
 *    @isp_hw:
 *    @event_type:
 *    @subscribe:
 *
 **/
static int isp_hw_subscribe_one_v4l2_event(isp_hw_t *isp_hw,
  uint32_t event_type, boolean subscribe)
{
  int rc = 0;
  struct v4l2_event_subscription sub;

  ISP_DBG(ISP_MOD_COM,"%s: event_type = 0x%x, is_subscribe = %d", __func__, event_type,
    subscribe);

  memset(&sub, 0, sizeof(sub));
  sub.type = event_type;

  if (isp_hw->fd < 0) {
    CDBG_ERROR("%s: error, isp_hw->fd is incorrect", __func__);
  }

  if (subscribe)
    rc = ioctl(isp_hw->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  else
    rc = ioctl(isp_hw->fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);

  if (rc < 0)
    CDBG_ERROR("%s: error, event_type = 0x%x, is_subscribe = %d", __func__,
      event_type, subscribe);

  return rc;
}

/** isp_hw_proc_action_stream_start:
 *
 *    @isp_hw: isp hw
 *    @params: start/stop properties
 *
 **/
static int isp_hw_proc_action_stream_start(isp_hw_t *isp_hw,
  start_stop_stream_t *start_params)
{
  int i, rc = 0;
  int use_pix = 0;
  isp_hw_stream_t *stream = NULL;
  isp_hw_session_t *session = NULL;
  isp_session_t *current_session = NULL;
  isp_t *isp = (isp_t*)isp_hw->notify_ops->parent;
  start_stop_stream_t pipeline_start_param;
  uint32_t streamids[ISP_MAX_STREAMS];
  int input_start_mask = 0;
  int existing_pix_cnt = isp_hw->pipeline.num_active_streams;

  if (isp_hw->hw_state == ISP_HW_STATE_STREAM_STOPPING) {
    CDBG_ERROR("%s: cannot start stream while stopping streaming\n",
               __func__);
    return -1;
  }

  memset(&pipeline_start_param,  0,  sizeof(pipeline_start_param));
  pipeline_start_param.session_id = start_params->session_id;

  session = isp_hw_find_session(isp_hw, start_params->session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session with sessionid = %d\n",
               __func__, start_params->session_id);
    return -1;
  }

  current_session = isp_util_find_session(isp, start_params->session_id);
  if (!current_session) {
    CDBG_ERROR("%s: cannot find session (%d)\n",
               __func__, start_params->session_id);
  }
  for (i = 0; i < start_params->num_streams; i++) {
    stream = isp_hw_find_stream_in_session(session,
      start_params->stream_ids[i]);
    if (stream) {
      if (stream->cfg.use_pix) {
        isp_hw->pipeline.num_active_streams++;
        streamids[pipeline_start_param.num_streams++] = stream->cfg.stream_id;
      }
      input_start_mask |= stream->cfg.vfe_output_mask;
    }
  }

  if (existing_pix_cnt == 0 && isp_hw->pipeline.num_active_streams > 0) {
    ISP_DBG(ISP_MOD_COM,"%s: stream id %d type %d fmt %d subscribe pipeline related events\n",
         __func__, stream->cfg.stream_id, stream->cfg.stream_info.stream_type,
         stream->cfg.stream_info.fmt);
    isp_hw_subscribe_pipeline_event(isp_hw, TRUE);
  }
  /*rc = isp_hw_subscribe_one_v4l2_event(isp_hw, ISP_EVENT_SOF, TRUE);*/


  /* Start Pix*/
  pipeline_start_param.stream_ids = streamids;
  pipeline_start_param.fast_aec_mode = start_params->fast_aec_mode;

  switch(GET_ISP_MAIN_VERSION(isp_hw->init_params.isp_version)){
  case ISP_VERSION_32:
      ((isp_pipeline_t *)isp_hw->pipeline.private_data)->stats_burst_len =
         VFE32_STATS_BURST_LEN;
    break;
  case ISP_VERSION_40: {
    if(isp_hw->init_params.cap.hw_ver == ISP_MSM8916 ||
       isp_hw->init_params.cap.hw_ver == ISP_MSM8939 ) {
      ((isp_pipeline_t *)isp_hw->pipeline.private_data)->stats_burst_len =
         VFE40_STATS_BURST_LEN_2;
    } else {
      ((isp_pipeline_t *)isp_hw->pipeline.private_data)->stats_burst_len =
         VFE40_STATS_BURST_LEN_1;
      }
    }
    break;
  case ISP_VERSION_44:
      ((isp_pipeline_t *)isp_hw->pipeline.private_data)->stats_burst_len =
         VFE44_STATS_BURST_LEN;
    break;
  default:
      ((isp_pipeline_t *)isp_hw->pipeline.private_data)->stats_burst_len =
         VFE40_STATS_BURST_LEN_1;
    break;
  }

  if (pipeline_start_param.num_streams) {
    rc = isp_pipeline_action(isp_hw->pipeline.private_data,
      ISP_PIX_ACTION_CODE_STREAM_START, (void *)&pipeline_start_param,
      sizeof(start_stop_stream_t));

    if (rc < 0) {
      CDBG_ERROR("%s: PIX start error = %d\n", __func__, rc);
      return rc;
    }
  }

  for (i = 0; i < ISP_INTF_MAX; i++) {
    if (input_start_mask & (1 << i)) {
      isp_hw->sof_subscribe_ref_cnt[i] ++;
      if (isp_hw->sof_subscribe_ref_cnt[i] == 1) {
        rc = isp_hw_subscribe_one_v4l2_event(isp_hw, ISP_EVENT_SOF + i, TRUE);
        rc = isp_hw_subscribe_one_v4l2_event(isp_hw, ISP_EVENT_REG_UPDATE + i, TRUE);
      }
    }
  }

  ISP_DBG(ISP_MOD_COM,"%s: PIX STREAM_START done\n", __func__);

  if (current_session != NULL) {
    start_params->frame_id = current_session->sof_id[0];
  }
  /* start AXI */
  rc = isp_axi_action(isp_hw->axi.private_data,
    ISP_AXI_ACTION_CODE_STREAM_START, (void *)start_params,
    sizeof(start_stop_stream_t));

  if (rc < 0) {
    CDBG_ERROR("%s: AXI start error = %d\n", __func__, rc);
    return rc;
  }

  isp_hw->hw_state = ISP_HW_STATE_ACTIVE;
  isp_hw->num_active_streams += start_params->num_streams;

  return rc;
}

/** isp_hw_proc_action_stream_stop:
 *
 *    @isp_hw: isp hw
 *    @params: start/stop properties
 *
 **/
static int isp_hw_proc_action_stream_stop(isp_hw_t *isp_hw,
  start_stop_stream_t *stop_params)
{
  int rc = 0;
  int i, use_pix = 0;
  isp_hw_stream_t *stream = NULL;
  isp_hw_session_t *session = NULL;
  start_stop_stream_t pipeline_stop_param;
  uint32_t streamids[ISP_MAX_STREAMS];
  int input_stop_mask = 0;
  int existing_pix_cnt = isp_hw->pipeline.num_active_streams;

  ISP_DBG(ISP_MOD_COM,"%s: E, sessionid = %d\n", __func__, stop_params->session_id);

  if (isp_hw->hw_state != ISP_HW_STATE_ACTIVE) {
    CDBG_ERROR("%s: Invalid state\n", __func__);
    return -1;
  }

  session = isp_hw_find_session(isp_hw, stop_params->session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session with sessionid = %d\n",
               __func__, stop_params->session_id);
    return -1;
  }

  memset(&pipeline_stop_param,  0,  sizeof(pipeline_stop_param));
  pipeline_stop_param.session_id = stop_params->session_id;

  for (i = 0; i < stop_params->num_streams; i++) {
    stream = isp_hw_find_stream_in_session(session,
      stop_params->stream_ids[i]);
    if (stream) {
      if (stream->cfg.use_pix) {
        isp_hw->pipeline.num_active_streams--;
        streamids[pipeline_stop_param.num_streams++] = stream->cfg.stream_id;
      }

      input_stop_mask |= stream->cfg.vfe_output_mask;
    }
  }
  if (existing_pix_cnt > 0 && isp_hw->pipeline.num_active_streams == 0) {
    ISP_DBG(ISP_MOD_COM,"%s: subscribe pipeline related events\n", __func__);
    isp_hw_subscribe_pipeline_event(isp_hw, FALSE);
  }
  /*rc = isp_hw_subscribe_one_v4l2_event(isp_hw, ISP_EVENT_SOF, FALSE);*/

  pipeline_stop_param.stream_ids = streamids;

  /* stop AXI */
  rc = isp_axi_action(isp_hw->axi.private_data,
    ISP_AXI_ACTION_CODE_STREAM_STOP, (void *)stop_params,
    sizeof(start_stop_stream_t));

  if (rc < 0) {
    CDBG_ERROR("%s: AXI stop error = %d\n", __func__, rc);
    return rc;
  }

  if (pipeline_stop_param.num_streams) {
    /* stop the PIX */
    rc = isp_pipeline_action(isp_hw->pipeline.private_data,
      ISP_PIX_ACTION_CODE_STREAM_STOP, (void *)&pipeline_stop_param,
      sizeof(start_stop_stream_t));

    if (rc < 0) {
      CDBG_ERROR("%s: AXI stop error = %d\n", __func__, rc);
      return rc;
    }
  }

  for (i = 0; i < ISP_INTF_MAX; i++) {
    if (input_stop_mask & (1 << i)) {
      isp_hw->sof_subscribe_ref_cnt[i] --;
      if (isp_hw->sof_subscribe_ref_cnt[i] == 0) {
        isp_hw_subscribe_one_v4l2_event(isp_hw, ISP_EVENT_SOF + i, FALSE);
        isp_hw_subscribe_one_v4l2_event(isp_hw, ISP_EVENT_REG_UPDATE + i, FALSE);
      }
    }
  }

  isp_hw->num_active_streams -= stop_params->num_streams;
  if (!isp_hw->num_active_streams) {
    /* no more stream active */
    isp_hw->hw_state = ISP_HW_STATE_IDLE;
  }

  ISP_DBG(ISP_MOD_COM,"%s: X, sessionid = %d, active_streams = %d, hw_state = %d\n", __func__,
    stop_params->session_id, isp_hw->num_active_streams, isp_hw->hw_state);

  return rc;
}

/** isp_hw_proc_action_divert_ack:
 *
 *    @isp_hw: isp hw
 *    @ack:
 *
 **/
static int isp_hw_proc_action_divert_ack(isp_hw_t *isp_hw,
  isp_hw_buf_divert_ack_t *ack)
{
  return isp_axi_action(isp_hw->axi.private_data,
    ISP_AXI_ACTION_CODE_STREAM_DIVERT_ACK, (void *)ack,
    sizeof(isp_hw_buf_divert_ack_t));
}

/** isp_hw_proc_action_set_saved_params
 *    @isp_hw: isp_hw root
 *    @saved_params: isp saved params
 *
 *  Sets saved params to pix.
 *
 **/
static int isp_hw_proc_set_saved_params(isp_hw_t *isp_hw,
  isp_saved_params_t *saved_params, uint32_t in_params_size)
{

  int rc = 0;

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_SAVED_PARAMS, (void *)saved_params,
    sizeof(isp_saved_params_t));

  return rc;
}


/** isp_hw_proc_set_uv_subsample
 *
 *    @isp_hw:
 *    @enable:
 *    @in_params_size:
 *
 **/
static int isp_hw_proc_set_uv_subsample(isp_hw_t *isp_hw,
  void *enable, uint32_t in_params_size)
{
  int rc = 0;
  isp_pipeline_t *pipeline = isp_hw->pipeline.private_data;

  /*update scaler and fov*/
  rc = isp_pipeline_set_params((void *)pipeline,
    ISP_PIX_SET_UV_SUBSAMPLE, enable, in_params_size);

  if (rc < 0) {
    CDBG_ERROR("%s: pipeline set params error = %d\n", __func__, rc);
    return rc;
  }

  /* axi hw update*/
  rc = isp_axi_set_params(isp_hw->axi.private_data,
    ISP_AXI_SET_STREAM_UPDATE,
    (void *)pipeline->pix_params.cfg_and_3a_params.cfg.outputs,
    sizeof(isp_hwif_output_cfg_t) * ISP_PIX_PATH_MAX);

  return rc;
}

/** isp_hw_proc_save_aec_params
 *    @isp_hw: isp_hw root
 *    @saved_params: isp saved params
 *
 *  Sets saved params to pix.
 *
 **/
static int isp_hw_proc_save_aec_params(isp_hw_t *isp_hw,
  stats_update_t *saved_params, uint32_t in_params_size)
{
  int rc = 0;

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SAVE_AEC_PARAMS, (void *)saved_params,
    sizeof(isp_saved_params_t));

  return rc;
}

/** isp_hw_proc_save_aec_params
 *
 *  @isp_hw: isp_hw root
 *  @saved_params: isp saved params
 *
 *  Sets saved params to pix.
 *
 *  Return 0 on success and negative error on failure
 **/
static int isp_hw_proc_save_flash_params(isp_hw_t *isp_hw,
  isp_flash_params_t *flash_params, uint32_t in_params_size)
{
  int rc = 0;

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_FLASH_PARAMS, (void *)flash_params,
    sizeof(isp_flash_params_t));

  return rc;
}

/** isp_hw_proc_save_asd_params
 *    @isp_hw: isp_hw root
 *    @saved_params: isp saved params
 *
 *  Sets saved params to pix.
 *
 **/
static int isp_hw_proc_save_asd_params(isp_hw_t *isp_hw,
  stats_update_t *saved_params, uint32_t in_params_size)
{
  int rc = 0;

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SAVE_ASD_PARAMS, (void *)saved_params, sizeof(isp_saved_params_t));

  return rc;
}

/** isp_hw_proc_init_pix
 *    @isp_hw: isp_hw root
 *
 **/
static int isp_hw_proc_init_pix(isp_hw_t *isp_hw)
{
  int rc = 0;

  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);

  if (isp_hw->pipeline.ref_cnt > 0) {
    isp_hw->pipeline.ref_cnt++;
    return rc;
  }

  isp_hw->pipeline.private_data = isp_hw_create_pipeline(isp_hw->fd,
    isp_hw->init_params.isp_version, isp_hw->init_params.dev_idx,
    isp_hw->init_params.buf_mgr);

  if (isp_hw->pipeline.private_data) {
    rc = isp_pipeline_init(isp_hw->pipeline.private_data, NULL, isp_hw);
    ISP_DBG(ISP_MOD_COM,"%s: isp_pipeline_init ret = %d\n", __func__, rc);
  }

  isp_hw->pipeline.ref_cnt++;

  return rc;
}

/** isp_hw_proc_destroy_pix
 *    @isp_hw: isp_hw root
 *
 **/
static int isp_hw_proc_destroy_pix(isp_hw_t *isp_hw)
{
  int rc = 0;

  isp_hw->pipeline.ref_cnt--;
  if (isp_hw->pipeline.ref_cnt == 0) {
    rc = isp_pipeline_destroy(isp_hw->pipeline.private_data);
    memset(&isp_hw->pipeline, 0,  sizeof(isp_hw_pipeline_t));
  }

  return rc;
}

/** isp_hw_proc_init_axi
 *    @isp_hw: isp_hw root
 *
 **/
static int isp_hw_proc_init_axi(isp_hw_t *isp_hw)
{
  int rc = 0;

  if (isp_hw->axi.ref_cnt > 0) {
    isp_hw->axi.ref_cnt++;
    return rc;
  }

  isp_hw->axi.private_data = isp_hw_create_axi(isp_hw->fd,
    isp_hw->init_params.isp_version, isp_hw->init_params.dev_idx,
    isp_hw->init_params.buf_mgr);

  if (isp_hw->axi.private_data) {
    rc = isp_axi_init(isp_hw->axi.private_data, NULL, isp_hw);
  }

  isp_hw->axi.ref_cnt++;

  return rc;
}

/** isp_hw_proc_destroy_axi
 *    @isp_hw: isp_hw root
 *
 **/
static int isp_hw_proc_destroy_axi(isp_hw_t *isp_hw)
{
  int rc = 0;

  isp_hw->axi.ref_cnt--;

  if (isp_hw->axi.ref_cnt == 0) {
    rc = isp_axi_destroy(isp_hw->axi.private_data);
    memset(&isp_hw->axi, 0,  sizeof(isp_hw_axi_t));
  }

  return rc;
}

/** isp_hw_proc_hw_update
 *    @ctrl:
 *
 **/
int isp_hw_proc_hw_update(void *ctrl, struct msm_isp_event_data *sof)
{
  int rc = 0;
  isp_hw_t *isp_hw = ctrl;
  isp_pipeline_t *pipeline = isp_hw->pipeline.private_data;

  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];
  uint32_t hw_frame_id;

  if (isp_hw->hw_update_skip) {
    isp_hw->hw_update_skip--;
    return 0;
  }

  /* axi hw update*/
  rc = isp_axi_action(isp_hw->axi.private_data, ISP_AXI_ACTION_CODE_HW_UPDATE,
    NULL, 0);

  if (rc < 0) {
    CDBG_ERROR("%s: ISP axi HW update action error = %d\n",  __func__, rc);
  }
  /* pipeline hw update */
  rc = isp_pipeline_action(isp_hw->pipeline.private_data,
    ISP_PIX_ACTION_CODE_HW_UPDATE, NULL, 0);

  if (rc < 0) {
    CDBG_ERROR("%s: ISP pipeline HW update action error = %d\n",
      __func__, rc);
  }

  isp_hw->dump_info.meta_data.awb_gain =
    pipeline->pix_params.cfg_and_3a_params.trigger_input.stats_update.awb_update.gain;
  isp_hw->dump_info.meta_data.color_temp =
    pipeline->pix_params.cfg_and_3a_params.trigger_input.stats_update.awb_update.color_temp;
  isp_hw->dump_info.meta_data.lux_idx = FLOAT_TO_Q(8,
    pipeline->pix_params.cfg_and_3a_params.trigger_input.stats_update.aec_update.lux_idx);

  return rc;
}

int isp_hw_proc_hw_request_reg_update(  isp_hw_t *isp_hw ,void *session_ptr)
{
  boolean                      ret = TRUE;
  int                          hw_id;
  int                          rc = 0;
  uint32_t                     frame_id;
  enum msm_vfe_input_src       frame_src = VFE_PIX_0;
  isp_session_t *session = (isp_session_t *)session_ptr;
  if (session == NULL) {
    CDBG_ERROR("%s: Session is NULL\n", __func__);
    return -1;
  }
  rc = ioctl(isp_hw->fd, VIDIOC_MSM_ISP_REG_UPDATE_CMD,
       &frame_src);
  if (rc < 0) {
     CDBG_ERROR("failed: ret %d", ret);
     ret = FALSE;
  }
  return ret;
}


/** isp_hw_proc_action
 *    @ctrl:
 *    @action_code:
 *    @action_data:
 *    @action_data_size:
 *    @previous_ret_code:
 *
 **/
int isp_hw_proc_action(void *ctrl,  uint32_t action_code, void *action_data,
  uint32_t action_data_size,  int previous_ret_code)
{
  int rc = 0;
  isp_hw_t *isp_hw = ctrl;

  switch (action_code) {
  case ISP_HW_ACTION_CODE_STREAM_START: {
    rc = isp_hw_proc_action_stream_start(isp_hw,
      (start_stop_stream_t *)action_data);
  }
    break;

  case ISP_HW_ACTION_CODE_STREAM_STOP: {
    rc = isp_hw_proc_action_stream_stop(isp_hw,
      (start_stop_stream_t *)action_data);
  }
    break;

  case ISP_HW_ACTION_CODE_BUF_DIVERT_ACK: {
    rc = isp_hw_proc_action_divert_ack(isp_hw,
      (isp_hw_buf_divert_ack_t *)action_data);
  }
    break;

  case ISP_HW_ACTION_CODE_STREAM_START_ACK:
  case ISP_HW_ACTION_CODE_STREAM_STOP_ACK: {
    rc = previous_ret_code;
  }
    break;

  default: {
  }
    break;
  }

  return rc;
}


/** isp_hw_subscribe_pipeline_event
 *    @isp_hw:
 *    @subscribe:
 *
 **/
static int isp_hw_subscribe_pipeline_event(isp_hw_t *isp_hw, boolean subscribe)
{
  int rc = 0, i;

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    rc = isp_hw_subscribe_one_v4l2_event(isp_hw, ISP_EVENT_STATS_NOTIFY + i,
      subscribe);
  }

  rc = isp_hw_subscribe_one_v4l2_event(isp_hw, ISP_EVENT_COMP_STATS_NOTIFY,
    subscribe);

  return rc;
}

/** isp_hw_proc_init_meta_dump
 *    @isp_hw:
 *
 *
 *
 **/
static int isp_hw_proc_init_meta_dump(isp_hw_t *isp_hw)
{
  int rc = 0;

  /*init isp_reg_dump entry: after isp hw init & isp pipeline init*/
  isp_hw_read_info *read_info;
  read_info = &isp_hw->dump_info.vfe_read_info[0];

  isp_hw->dump_info.meta_data.isp_version = isp_hw->init_params.isp_version;
  /*init reg dump entry*/
  read_info[ISP_META_REGISTER_DUMP].read_type = VFE_READ;
  read_info[ISP_META_REGISTER_DUMP].read_lengh =
    isp_hw->init_params.cap.num_register * sizeof(uint32_t);

  isp_hw->dump_info.meta_data.meta_entry[ISP_META_REGISTER_DUMP].dump_type
    = ISP_META_REGISTER_DUMP;
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_REGISTER_DUMP].len =
    isp_hw->init_params.cap.num_register * sizeof(uint32_t);
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_REGISTER_DUMP].isp_meta_dump
    = malloc(isp_hw->init_params.cap.num_register * sizeof(uint32_t));

  /*init rolloff tbl entry*/
  rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
    ISP_PIX_GET_ROLLOFF_TABLE_SIZE, NULL, 0,
    (void*)&read_info[ISP_META_ROLLOFF_TBL], sizeof(isp_hw_read_info));
  if (rc < 0) {
    CDBG_ERROR("%s: get ISP_PIX_GET_ROLLOFF_TABLE_SIZE error\n", __func__);
    return rc;
  }
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_ROLLOFF_TBL].dump_type =
    ISP_META_ROLLOFF_TBL;
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_ROLLOFF_TBL].len =
    read_info[ISP_META_ROLLOFF_TBL].read_lengh;
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_ROLLOFF_TBL].isp_meta_dump
    = malloc(read_info[ISP_META_ROLLOFF_TBL].read_lengh);

  /*init GAMMA tbl entry*/
  rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
    ISP_PIX_GET_GAMMA_TABLE_SIZE, NULL, 0,
    (void*)&read_info[ISP_META_GAMMA_TBL], sizeof(isp_hw_read_info));
  if (rc < 0) {
    CDBG_ERROR("%s: get ISP_PIX_GET_GAMMA_TABLE_SIZE error\n", __func__);
    return rc;
  }
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_GAMMA_TBL].dump_type =
    ISP_META_GAMMA_TBL;
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_GAMMA_TBL].len =
    read_info[ISP_META_GAMMA_TBL].read_lengh;
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_GAMMA_TBL].isp_meta_dump
    = malloc(read_info[ISP_META_GAMMA_TBL].read_lengh);

  /*init LINEARIZATION tbl entry*/
  rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
    ISP_PIX_GET_LINEARIZATION_TABLE_SIZE, NULL, 0,
    (void*)&read_info[ISP_META_LINEARIZATION_TBL], sizeof(isp_hw_read_info));
  if (rc < 0) {
    CDBG_ERROR("%s: get ISP_PIX_GET_LINEARIZATION_TABLE_SIZE error\n", __func__);
    return rc;
  }
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_LINEARIZATION_TBL].dump_type =
    ISP_META_LINEARIZATION_TBL;
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_LINEARIZATION_TBL].len =
    read_info[ISP_META_LINEARIZATION_TBL].read_lengh;
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_LINEARIZATION_TBL].isp_meta_dump
    = malloc(read_info[ISP_META_LINEARIZATION_TBL].read_lengh);

  /*init Luma Adaptation tbl entry*/
  rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
    ISP_PIX_GET_LA_TABLE_SIZE, NULL, 0,
    (void*)&read_info[ISP_META_LA_TBL], sizeof(isp_hw_read_info));
  if (rc < 0) {
    CDBG_ERROR("%s: get ISP_PIX_GET_LA_TABLE_SIZE error\n", __func__);
    return rc;
  }
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_LA_TBL].dump_type = ISP_META_LA_TBL;
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_LA_TBL].len =
    read_info[ISP_META_LA_TBL].read_lengh;
  isp_hw->dump_info.meta_data.meta_entry[ISP_META_LA_TBL].isp_meta_dump
    = malloc(read_info[ISP_META_LA_TBL].read_lengh);

  ISP_DBG(ISP_MOD_COM,"%s: num_reg = %d\n", __func__, isp_hw->init_params.cap.num_register);
  ISP_DBG(ISP_MOD_COM,"%s: tbl size: rolloff %d, gamma %d, linear %d, luma_adap %d\n",
    __func__, read_info[ISP_META_ROLLOFF_TBL].read_lengh,
    read_info[ISP_META_GAMMA_TBL].read_lengh,
    read_info[ISP_META_LINEARIZATION_TBL].read_lengh,
    read_info[ISP_META_LA_TBL].read_lengh);

  return rc;
}

/** isp_hw_proc_init
 *    @ctrl:
 *    @in_params:
 *    @notify_ops
 *
 **/
int isp_hw_proc_init(void *ctrl, void *in_params,
      isp_notify_ops_t *notify_ops)
{
  int rc = 0;
  isp_hw_t *isp_hw = ctrl;

  switch (isp_hw->hw_state) {
  case ISP_HW_STATE_DEV_OPEN: {
    isp_hw_init_params_t *init_params = (isp_hw_init_params_t *)in_params;

    isp_hw->notify_ops = notify_ops;
    isp_hw->hw_state = ISP_HW_STATE_IDLE;
    isp_hw->init_params = *init_params;

    /* init PIX. */
    rc = isp_hw_proc_init_pix(isp_hw);

    /* INIT AXI */
    if (rc == 0) {
      isp_hw_proc_init_axi(isp_hw);
      isp_hw_proc_init_meta_dump(isp_hw);
      isp_hw->isp_diag.has_user_dianostics = 0;
    }
  }
    break;

  default: {
  }
    break;
  }

  return rc;
}

/** isp_hw_proc_destroy
 *    @ctrl:
 *
 **/
int isp_hw_proc_destroy(void *ctrl)
{
  int rc = 0;
  isp_hw_t *isp_hw = ctrl;

  isp_hw_proc_destroy_pix(isp_hw);
  isp_hw_proc_destroy_axi(isp_hw);

  isp_hw->isp_diag.has_user_dianostics = 0;
  return rc;
}
/** isp_hw_read_reg_dump
 *    @isp_hw: isp hardware object
 *
 *    read register dump when receive SOF evt will be the most
 *    acurate timing to dump the register
 *
 *   Return none
 **/
int isp_hw_read_reg_dump(isp_hw_t *isp_hw, void *dump_entry, uint32_t read_type, uint32_t read_len)
{
  int rc = 0;
  uint32_t i;

  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd;

  memset(&cfg_cmd, 0, sizeof(cfg_cmd));
  memset(&reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));
  memset(dump_entry, 0, read_len);

  cfg_cmd.cfg_data = dump_entry;
  cfg_cmd.cmd_len = read_len;
  cfg_cmd.cfg_cmd = (void *)&reg_cfg_cmd;
  cfg_cmd.num_cfg = 1;

  reg_cfg_cmd.u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd.cmd_type = read_type;
  reg_cfg_cmd.u.rw_info.len = read_len;
  reg_cfg_cmd.u.rw_info.reg_offset = 0;
  rc = ioctl(isp_hw->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: isp red register error = %d\n", __func__, rc);
    return rc;
  }

#ifdef ISP_REG_DUMP
  uint32_t *reg_dump;
  reg_dump = (uint32_t *)dump_entry;

  /*dump*/
  /* i%4: 4 register per line
     i*16/4: *16 = to be HEX, /4 = 4 byte per register*/
  for (i=0; i < ISP40_NUM_REG_DUMP; i++) {
     if (i % 4  == 0) {
       ISP_DBG(ISP_MOD_COM,"%s: 0x%x: %08x  %08x  %08x  %08x\n", __func__, (i * 16) / 4,
         reg_dump[i], reg_dump[i+1], reg_dump[i+2], reg_dump[i+3]);
     }
  }
#endif

  return rc;
}

/** isp_hw_read_dmi_dump
 *    @isp_hw: isp hardware object
 *
 *    read register dump when receive SOF evt will be the most
 *    acurate timing to dump the register
 *
 *   Return none
 **/
int isp_hw_read_dmi_dump(isp_hw_t *isp_hw, void *dump_entry, isp_hw_read_info *dmi_read_info)
{
  int rc = 0;
  uint32_t i;

  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd;

  memset(&cfg_cmd, 0, sizeof(cfg_cmd));
  memset(&reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));
  memset(dump_entry, 0, dmi_read_info->read_lengh);

  rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
    ISP_PIX_GET_DMI_DUMP, (void*)dmi_read_info, sizeof(isp_hw_read_info),
    dump_entry, dmi_read_info->read_lengh);
  if (rc < 0) {
    CDBG_ERROR("%s: get ISP_PIX_GET_DMI_DUMP error\n", __func__);
    return rc;
  }

#ifdef ISP_DMI_DUMP
  uint32_t *reg_dump;
  reg_dump = (uint32_t *)dump_entry;

  /* read_lengh = by words (4 bytes)*/
  for (i=0; i < (dmi_read_info->read_lengh / sizeof(uint32_t)); i++) {
    ISP_DBG(ISP_MOD_COM,"%s: DMI table[%d] %08x\n", __func__, i, reg_dump[i]);
  }
#endif

  return rc;
}

/** isp_hw_prepare_meta_entry
 *    @isp_hw: isp hardware object
 *
 *
 *   Return none
 **/
int isp_hw_prepare_meta_entry(isp_hw_t *isp_hw)
{
  int i, rc;
  void *meta_entry;
  uint32_t read_lengh;
  uint32_t read_type;
  uint32_t read_bank;

  /*regular case it could be ISP_META_MAX entries
    but now only support reg_dump, rolloff, and gamma*/
  isp_hw->dump_info.meta_data.num_entry = 3;
    /*dump reg*/
  meta_entry = isp_hw->dump_info.meta_data.meta_entry[ISP_META_REGISTER_DUMP].isp_meta_dump;
  read_lengh = isp_hw->dump_info.vfe_read_info[ISP_META_REGISTER_DUMP].read_lengh;
  read_type =  isp_hw->dump_info.vfe_read_info[ISP_META_REGISTER_DUMP].read_type;

  rc = isp_hw_read_reg_dump(isp_hw, meta_entry, read_type, read_lengh);
  if (rc < 0) {
    CDBG_ERROR("%s: read register error, rc = %d\n", __func__, rc);
    return rc;
  }

  /*Dump DMI Rolloff table*/
  meta_entry =
    isp_hw->dump_info.meta_data.meta_entry[ISP_META_ROLLOFF_TBL].isp_meta_dump;

  rc = isp_hw_read_dmi_dump(isp_hw, meta_entry,
    &isp_hw->dump_info.vfe_read_info[ISP_META_ROLLOFF_TBL]);
  if (rc < 0) {
    CDBG_ERROR("%s: read isp_hw_read_dmi_dump error, rc = %d\n", __func__, rc);
    return rc;
  }
  isp_hw->dump_info.vfe_read_info[ISP_META_ROLLOFF_TBL].bank_idx ^= 1;

  /*Dump DMI Gamma table*/
  meta_entry =
    isp_hw->dump_info.meta_data.meta_entry[ISP_META_GAMMA_TBL].isp_meta_dump;
  rc = isp_hw_read_dmi_dump(isp_hw, meta_entry,
    &isp_hw->dump_info.vfe_read_info[ISP_META_GAMMA_TBL]);
  if (rc < 0) {
    CDBG_ERROR("%s: read isp_hw_read_dmi_dump error, rc = %d\n", __func__, rc);
    return rc;
  }
  isp_hw->dump_info.vfe_read_info[ISP_META_GAMMA_TBL].bank_idx ^= 1;

  return rc;
}

/** isp_hw_prepare_vfe_diag
 *    @isp_hw: isp hardware object
 *
 *
 *   Return none
 **/
int isp_hw_prepare_vfe_diag(isp_hw_t *isp_hw,
  cam_streaming_mode_t streaming_mode)
{
  int i, rc;
  vfe_diagnostics_t *vfe_diag;

  vfe_diag = &isp_hw->isp_diag.user_vfe_diagnostics;
  memset(vfe_diag, 0, sizeof(vfe_diagnostics_t *));

  rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
    ISP_PIX_GET_VFE_DIAG_INFO, &streaming_mode, sizeof(streaming_mode),
    vfe_diag, sizeof(vfe_diagnostics_t));
  if (rc < 0) {
    CDBG_ERROR("%s: get ISP_PIX_GET_VFE_DIAG_INFO error\n", __func__);
    return rc;
  }

#ifdef ISP_VFE_DIAG_DUMP
  uint32_t *reg_dump;
  reg_dump = (uint32_t *)vfe_diag;
#endif

  return rc;
}

/** isp_hw_get_current_rolloff_data
 *    @isp_hw: isp hardware object
 *
 *
 *   Return none
 **/
int isp_hw_get_current_rolloff_data(isp_hw_t *isp_hw,
  tintless_mesh_rolloff_array_t *rolloff)
{
  int rc;

  memset(rolloff, 0, sizeof(tintless_mesh_rolloff_array_t));

  rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
    ISP_PIX_GET_TINTLESS_RO, NULL, 0,
    rolloff, sizeof(tintless_mesh_rolloff_array_t));
  if (rc < 0) {
    CDBG_ERROR("%s: get ISP_PIX_GET_TINTLESS_RO error\n", __func__);
    return rc;
  }

  return rc;
}

/** isp_hw_send_info_to_metadata
 *    @isp_hw: isp hardware object
 *
 *  To send the ISP specific info to metadata stream. Note that
 *  only gamma table is supported as of now. This could be
 *  expanded
 *
 *  Return none
 **/
void isp_hw_send_info_to_metadata(isp_hw_t *isp_hw, int input_src)
{
  int rc;
  isp_pipeline_t *pipeline = isp_hw->pipeline.private_data;
  isp_gamma_t *p_gamma = &pipeline->cur_pix_params.cfg_and_3a_params.cfg.gamma_rgb;
  mct_bus_msg_t bus_msg;

  if (sizeof(mct_bus_msg_isp_gamma_t) >=
      (sizeof(isp_gamma_t))) {
    bus_msg.type = MCT_BUS_MSG_SET_ISP_GAMMA_INFO;
    bus_msg.size = sizeof(isp_gamma_t);
    bus_msg.msg = (void *)p_gamma;
    bus_msg.sessionid = isp_hw->pipeline.session_id[input_src];

    /* notify ptr is isp_core_hw_notify */
    rc = isp_hw->notify_ops->notify((void*)isp_hw->notify_ops->parent,
      isp_hw->init_params.dev_idx, ISP_HW_NOTIFY_METADATA_INFO, &bus_msg,
      sizeof(mct_bus_msg_t));

    if (rc < 0)
      CDBG_ERROR("%s: ISP_INFO to bus error = %d\n", __func__, rc);

  } else {
    CDBG_ERROR("%s:%d] Cannot post ISP_INFO bus message", __func__, __LINE__);
  }
}

/** isp_hw_update_params_at_sof
 *    @isp_hw: isp hardware object
 *    @sof: sof event data object
 *
 *  Fetch saved params and make a cached copy of it. Then
 *  trigger update to modules for both VFE keeping same copy of
 *  params for given frame id.
 *
 *  Return 0 success and negative value on failure
 **/
int isp_hw_proc_update_params_at_sof(isp_hw_t *isp_hw,
  struct msm_isp_event_data *sof)
{
  isp_hw_pending_update_params_t pending_update_params;
  stats_update_t stats_update;
  isp_hw_set_crop_factor_t * crop_factor = NULL;
  isp_hw_zoom_param_t *zoom_params =NULL;
  int rc = 0;
  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);
  memset(&stats_update, 0, sizeof(stats_update_t));
  /* We are updating pipeline here - hence using PIX */

  pending_update_params = isp_hw->thread_hw.pending_update_params;

  /* Fetch cached params for trigger update. This is to avoid sending
     different params to two VFEs. Both VFEs will use single cached
     params copy. If there is trigger update in between 2 VFE hw update
     calls, cached params won't be changed thus protect against split
     screen behaviour */

  if (pending_update_params.hw_update_params.uv_subsample_update) {
    rc = isp_hw_proc_set_uv_subsample(isp_hw,
      (void *)&pending_update_params.hw_update_params.uv_subsample_enable,
      sizeof(pending_update_params.hw_update_params.uv_subsample_enable));
  if (rc < 0)
    CDBG_ERROR("%s: uv submsapling error = %d\n", __func__, rc);
  }

  if (pending_update_params.hw_update_params.zoom_update == TRUE) {
    /* set Crop factor update to hw */
    crop_factor =
      &pending_update_params.hw_update_params.zoom_factor;
    zoom_params = &crop_factor->hw_zoom_parm;
    /* FOV crop is returned from set crop factor */
    rc = isp_hw_set_crop_factor(isp_hw, crop_factor,
           sizeof(isp_hw_set_crop_factor_t));
    /* get ROI for zoom */
    rc = isp_hw_get_roi_map(isp_hw, zoom_params->entry,
           sizeof(isp_hw_zoom_param_entry_t));
    if (rc < 0) {
      CDBG_ERROR("%s: isp_hw_get_roi_map failed error = %d\n", __func__, rc);
    } else if (zoom_params != NULL) {
      /* send to isp to calculate & save zoom params*/
      zoom_params->frame_id = sof->frame_id;
      /* We are updating pipeline here - hence using PIX */
      zoom_params->session_id = isp_hw->pipeline.session_id[VFE_PIX_0];
      rc = isp_hw->notify_ops->notify((void*)isp_hw->notify_ops->parent,
        isp_hw->init_params.dev_idx, ISP_HW_NOTIFY_ZOOM_ROI_PARAMS,
        zoom_params, sizeof(isp_hw_zoom_param_t *));
      if (rc < 0)
        CDBG_ERROR("%s:ISP_HW_NOTIFY_ZOOM_ROI_PARAMS failed, hw_idx = %d, "
          "session_id = %d, frame_id = %d error = %d\n",
          __func__, isp_hw->init_params.dev_idx,
          zoom_params->session_id, sof->frame_id, rc);
    }
  }

  stats_update.aec_update =
    pending_update_params.hw_update_params.aec_stats_update;
  stats_update.flag = pending_update_params.hw_update_params.stats_flag;
  if(stats_update.flag & STATS_UPDATE_AEC) {
    rc = isp_hw_proc_save_aec_params(isp_hw, &stats_update,
           sizeof(stats_update));
    if (rc < 0)
      CDBG_ERROR("%s: AEC trigger update error = %d\n", __func__, rc);
  }

  rc = isp_hw_proc_save_flash_params(isp_hw,
    &pending_update_params.hw_update_params.flash_params,
    sizeof(pending_update_params.hw_update_params.flash_params));
  if (rc < 0) {
      CDBG_ERROR("%s: isp_hw_proc_save_flash_params error = %d\n", __func__,
        rc);
  }

  stats_update.awb_update =
    pending_update_params.hw_update_params.awb_stats_update;
  if(stats_update.flag & STATS_UPDATE_AWB) {
    rc = isp_hw_proc_awb_trigger_update(isp_hw, &stats_update,
      sizeof(stats_update));
    if (rc < 0)
      CDBG_ERROR("%s: AWB trigger update error = %d\n", __func__, rc);
  }

  rc = isp_hw_proc_aec_trigger_update(isp_hw,
  &pending_update_params.hw_update_params.dig_gain,
    sizeof(pending_update_params.hw_update_params.dig_gain));
  if (rc < 0)
    CDBG_ERROR("%s: Digital gain update error = %d\n", __func__, rc);

  if(pending_update_params.hw_update_params.ihist_update) {
    rc = isp_hw_proc_ihist_la_trigger_update(isp_hw,
      &pending_update_params.hw_update_params.ihist_stats,
      sizeof(pending_update_params.hw_update_params.ihist_stats));
    if (rc < 0)
      CDBG_ERROR("%s: IHIST trigger update error = %d\n", __func__, rc);
  }
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_sof_update
 *
 *  @ctrl: handle to isp_hw_t
 *
 *  Write MM_ISP_CMD_HW_UPDATE cmd to pipe to schedule hw
 *  thread. hw thread upon receiving this cmd peforms hw
 *  update
 *
 *  Return 0 on success and negative error on failure
 **/
static int isp_sof_update(void *ctrl, struct msm_isp_event_data *sof_parm)
{
  int len, rc = 0;
  isp_hw_t *isp_hw = ctrl;
  uint32_t cmd = MM_ISP_CMD_SOF_UPDATE;
  pthread_mutex_lock(&isp_hw->thread_hw.cmd_mutex);
  pthread_mutex_lock(&isp_hw->thread_hw.busy_mutex);

  /* If hw update in progress, drop current hw update */
  if(!isp_hw->thread_hw.thread_busy) {
    isp_hw->thread_hw.thread_busy = TRUE;
    isp_hw->thread_hw.sof_parm = *sof_parm;
    memset(&isp_hw->thread_hw.pending_update_params, 0,
           sizeof(isp_hw_pending_update_params_t));

    isp_hw->thread_hw.pending_update_params.session_id = isp_hw->pipeline.session_id[VFE_PIX_0];
    isp_hw->thread_hw.pending_update_params.frame_id = isp_hw->thread_hw.sof_parm.frame_id;
    isp_hw->thread_hw.pending_update_params.dev_idx = isp_hw->init_params.dev_idx;
    pthread_mutex_unlock(&isp_hw->thread_hw.busy_mutex);

    rc = isp_hw->notify_ops->notify((void*)isp_hw->notify_ops->parent,
      isp_hw->init_params.dev_idx, ISP_HW_NOTIFY_FETCH_HW_UPDATE_PARAMS,
      &isp_hw->thread_hw.pending_update_params, sizeof(isp_hw_pending_update_params_t));

    len = write(isp_hw->thread_hw.pipe_fds[1], &cmd, sizeof(cmd));

    if (len != sizeof(cmd)) {
      /* we got error here */
      pthread_mutex_unlock(&isp_hw->thread_hw.cmd_mutex);
      return -EPIPE;
    }

    sem_wait(&isp_hw->thread_hw.sig_sem);
    rc = isp_hw->thread_hw.return_code;
  } else {
    pthread_mutex_unlock(&isp_hw->thread_hw.busy_mutex);
  }

  pthread_mutex_unlock(&isp_hw->thread_hw.cmd_mutex);

  return rc;
}

/** isp_hw_proc_subdev_event
 *    @isp_hw:
 *    @thread_data:
 *
 **/
void isp_hw_proc_subdev_event(isp_hw_t *isp_hw, isp_thread_t *thread_data)
{
  int rc, input_src = 0;
  struct v4l2_event v4l2_event;
  struct msm_isp_event_data *isp_event_data = NULL;

  /* HW is in overflow recovery. Don't process any more events*/
  if (isp_hw->is_overflow) {
    return 0;
  }

  memset(&v4l2_event, 0, sizeof(v4l2_event));
  rc = ioctl(isp_hw->fd, VIDIOC_DQEVENT, &v4l2_event);

  if (rc >= 0) {
    isp_event_data = (struct msm_isp_event_data *)v4l2_event.u.data;

    if ((v4l2_event.type & 0xFFFFFF00) >= ISP_EVENT_BUF_DIVERT) {
      input_src = v4l2_event.type & 0xFF;
      v4l2_event.type &= ~input_src;
    }

    switch(v4l2_event.type) {
    case ISP_EVENT_STATS_NOTIFY:{
      isp_pipeline_stats_parse_t stats_event;
      mct_event_stats_t mct_stats_event;
      isp_pipeline_t *pipeline = isp_hw->pipeline.private_data;
      mct_event_stats_isp_t *isp_stats_event = NULL;
      isp_stats_config_t * stats_cfg =
        &pipeline->pix_params.cfg_and_3a_params.cfg.stats_cfg;
      isp_stats_config_t temp_stats_config;

      if (isp_hw->pipeline.num_active_streams == 0)
        break;

      stats_event.raw_stats_event =
        (struct msm_isp_event_data *)v4l2_event.u.data;
      stats_event.parsed_stats_event = &mct_stats_event.u.isp_stats;
      memset(&mct_stats_event, 0 , sizeof(mct_event_stats_t));

      /* Parse stats*/
      rc = isp_pipeline_action(pipeline,
        ISP_PIX_ACTION_CODE_STATS_PARSE, (void *)&stats_event,
        sizeof(stats_event));

      /* Save current stats config to send to 3A */
      temp_stats_config = *stats_cfg;
      isp_stats_event = stats_event.parsed_stats_event;
      isp_stats_event->stats_config.stats_mask = temp_stats_config.stats_mask;
      isp_stats_event->stats_config.af_config = &temp_stats_config.af_config;
      isp_stats_event->stats_config.awb_config = &temp_stats_config.awb_config;
      isp_stats_event->stats_config.aec_config = &temp_stats_config.aec_config;

      /* Notify 3A*/
      if (stats_event.parsed_stats_event->stats_mask == 0) {
        //no stats to be reported
        ISP_DBG(ISP_MOD_COM,"%s: Stats mask 0. No stats to be reported. Skip "\
          "sending stats event to 3A\n", __func__);
      } else {
        mct_stats_event.type = MCT_EVENT_STATS_ISP_STATS;
        rc = isp_hw->notify_ops->notify((void*)isp_hw->notify_ops->parent,
          isp_hw->init_params.dev_idx, ISP_HW_NOTIFY_STATS,
          &stats_event, sizeof(isp_pipeline_stats_parse_t));
      }
    }
      break;

    case ISP_EVENT_SOF:{
      isp_pipeline_t *pipeline = isp_hw->pipeline.private_data;
      isp_stats_udpate_t *stats_update =
        &pipeline->cur_pix_params.cfg_and_3a_params.trigger_input.stats_update;
      cam_streaming_mode_t streaming_mode =
        pipeline->cur_pix_params.cfg_and_3a_params.cfg.streaming_mode;
      isp_pipeline_curr_rolloff_t curr_rolloff;
      mct_bus_msg_t bus_msg;
      mct_bus_msg_isp_sof_t sof_event;
      tintless_mesh_rolloff_array_t rolloff;
      struct msm_isp_event_data *sof =
        (struct msm_isp_event_data *)v4l2_event.u.data;

      if (thread_data->wake_up_at_sof) {
        ISP_DBG(ISP_MOD_COM,"%s: wake up waiting thread\n", __func__);
        thread_data->wake_up_at_sof = FALSE;
        sem_post(&thread_data->sig_sem);
      }
      ATRACE_INT("Camera:SOF", 1);
      ATRACE_INT("Camera:SOF", 0);
      /*when got SOF event means got reg update IRQ, just applied the hw setting*/
      if (isp_hw->isp_diag.has_user_dianostics) {
        isp_hw_prepare_vfe_diag(isp_hw, streaming_mode);
      }
      if (GET_ISP_MAIN_VERSION(isp_hw->init_params.isp_version)
        == ISP_VERSION_40) {
        uint32_t metadump_enable = 0;
        char value[PROPERTY_VALUE_MAX];
        property_get("persist.camera.dumpmetadata", value, "0");
        metadump_enable = atoi(value);
        if (metadump_enable) {
           isp_hw_prepare_meta_entry(isp_hw);
        }
      }
      if(pipeline->pix_params.cfg_and_3a_params.cfg.tintless_data->stats_support_type
        == ISP_TINTLESS_STATS_TYPE_BG) {
        isp_hw_get_current_rolloff_data(isp_hw, &rolloff);
        curr_rolloff.session_id = isp_hw->pipeline.session_id[input_src];
        curr_rolloff.rolloff = &rolloff;
        rc = isp_hw->notify_ops->notify((void*)isp_hw->notify_ops->parent,
           isp_hw->init_params.dev_idx, ISP_HW_NOTIFY_CUR_ROLLOFF, &curr_rolloff,
          sizeof(isp_pipeline_curr_rolloff_t));

        if (rc < 0)
          CDBG_ERROR("%s:GET _ROLLOFF error = %d\n", __func__, rc);
      }

      if (sizeof(mct_bus_msg_isp_stats_awb_metadata_t) >=
          sizeof(stats_update->awb_update)) {
        bus_msg.type = MCT_BUS_MSG_SET_ISP_STATS_AWB_INFO;
        bus_msg.size = sizeof(stats_update->awb_update);
        bus_msg.msg = (void *)&stats_update->awb_update;
        bus_msg.sessionid = isp_hw->pipeline.session_id[input_src];
        /* notify ptr is isp_core_hw_notify */
        rc = isp_hw->notify_ops->notify((void*)isp_hw->notify_ops->parent,
          isp_hw->init_params.dev_idx, ISP_HW_NOTIFY_STATS_AWB_INFO, &bus_msg,
          sizeof(mct_bus_msg_t));

        if (rc < 0)
          CDBG_ERROR("%s: STATS_AWB_INFO to bus error = %d\n", __func__, rc);
      } else {
        CDBG_ERROR("%s: Can not post STATS_AWB_INFO bus message, because\n\
          dst buf size is %d src buf size is %d\n", __func__,
          sizeof(mct_bus_msg_isp_stats_awb_metadata_t),
          sizeof(stats_update->awb_update));
      }

      if (input_src == VFE_PIX_0) {
        /* send ISP info */
        isp_hw_send_info_to_metadata(isp_hw, input_src);
      }

      bus_msg.type = MCT_BUS_MSG_ISP_SOF;
      bus_msg.msg = (void *)&sof_event;
      sof_event.frame_id = sof->frame_id;
      sof_event.timestamp = sof->timestamp;
      sof_event.mono_timestamp = sof->mono_timestamp;
      bus_msg.sessionid = isp_hw->pipeline.session_id[input_src];

      ISP_DBG(ISP_MOD_COM,"%s: SOF input intf = %d\n", __func__, sof->input_intf);
      if (sof_event.frame_id > 0) {
         if (sof->input_intf == VFE_PIX_0) {
            /*SOF update contain
              1. HW reg_update
              2. trigger update parm*/
            rc = isp_sof_update(isp_hw, sof);
            if (rc < 0)
              CDBG_ERROR("%s: HW update failed error = %d\n", __func__, rc);
          }

         /* notify ptr is isp_core_hw_notify */
         rc = isp_hw->notify_ops->notify(
           (void*)isp_hw->notify_ops->parent, isp_hw->init_params.dev_idx,
           ISP_HW_NOTIFY_CAMIF_SOF, &bus_msg, sizeof(mct_bus_msg_t));

         if (rc < 0) {
           CDBG_ERROR("%s: SOF to media bus error = %d\n", __func__, rc);
         }
       }
      }
      break;

    case ISP_EVENT_REG_UPDATE: {
      isp_session_t *current_session = NULL;
      isp_t *isp = (isp_t *)isp_hw->notify_ops->parent;
      uint32_t frame_id;
      struct msm_isp_event_data *reg_update_sof =
        (struct msm_isp_event_data *)v4l2_event.u.data;
      current_session =
        isp_util_find_session(isp, isp_hw->pipeline.session_id[VFE_PIX_0]);
      if (!current_session) {
          CDBG_ERROR("%s: can not find session\n", __func__);
          return;
      }
      if (isp_hw->pipeline.num_active_streams == 0) {
        CDBG_ERROR("%s: no hw stream , skip sending REG_UPDATE!\n", __func__);
        break;
      }
      pthread_mutex_lock(&current_session->state_mutex);
      current_session->reg_update_info.reg_update_state =
        ISP_REG_UPDATE_STATE_CONSUMED;
      pthread_mutex_unlock(&current_session->state_mutex);
    }
      break;

    case ISP_EVENT_BUF_DIVERT: {
      /* is_skip_pproc is TRUE if LowPowerMode is enable,So ISP will do buf_done.*/
        isp_event_data->is_skip_pproc = isp_util_is_lowpowermode_feature_enable(
           (void *)isp_hw->notify_ops->parent,
           isp_event_data->u.buf_done.session_id);

        mct_bus_msg_meta_valid meta_valid;
        mct_bus_msg_t bus_msg;
        bus_msg.type = MCT_BUS_MSG_META_VALID;
        bus_msg.msg = (void *)&meta_valid;
        bus_msg.sessionid = isp_event_data->u.buf_done.session_id;
        meta_valid.frame_id = isp_event_data->frame_id;
        /* notify ptr is isp_core_hw_notify */
        rc = isp_hw->notify_ops->notify(
           (void *)isp_hw->notify_ops->parent, isp_hw->init_params.dev_idx,
           ISP_HW_NOTIFY_META_VALID, &bus_msg, sizeof(mct_bus_msg_t));
        rc = isp_hw_notify_buf_divert_event(isp_hw, isp_event_data);
      }
      break;
    case ISP_EVENT_WM_BUS_OVERFLOW: {
      struct msm_isp_event_data *error_event =
        (struct msm_isp_event_data *)v4l2_event.u.data;
      isp_t *isp = (isp_t*)isp_hw->notify_ops->parent;
      int j = 0;
      isp_hw_session_t *session = NULL;
      if (error_event->u.error_info.error_mask == (1 << ISP_WM_BUS_OVERFLOW)) {
        CDBG_ERROR("%s Overflow detected. Notify ISPIF to reset \n", __func__);
        isp_hw->is_overflow = 1;
        for (j = 0; j < ISP_MAX_SESSIONS; j++) {
          session = &isp_hw->session[j];
          if (session && session->session_id) {
            mct_bus_msg_t bus_msg;
            uint8_t is_frame_id_reset = 1;
            bus_msg.type = MCT_BUS_MSG_WM_BUS_OVERFLOW_RECOVERY;
            bus_msg.msg = (void *)&is_frame_id_reset;
            bus_msg.size = sizeof(uint8_t);
            bus_msg.sessionid = session->session_id;
            if (TRUE != mct_module_post_bus_msg(isp->module, &bus_msg))
            CDBG_ERROR("%s: MCT_BUS_MSG_WM_BUS_OVERFLOW_RECOVERY to bus error\n", __func__);

            rc = isp_util_wm_bus_overflow_recovery(isp,isp_hw, session);
            if (rc < 0) {
              CDBG_ERROR("%s Error doing WM BUS OVERFLOW Recovery \n", __func__);
              return;
            }
          }
        }
      } else {
        mct_bus_msg_t bus_msg;
        bus_msg.type = MCT_BUS_MSG_SEND_HW_ERROR;
        bus_msg.msg = NULL;
        bus_msg.size = 0;
        bus_msg.sessionid = isp_hw->pipeline.session_id;
        if (TRUE != mct_module_post_bus_msg(isp->module, &bus_msg))
         CDBG_ERROR("%s: MCT_BUS_MSG_ERROR_MESSAGE to bus error\n", __func__);
      }
    }
      break;
    default: {
    }
      break;
    }
  } else {
    CDBG_HIGH("%s: VIDIOC_DQEVENT type failed\n", __func__);
  }

  return;
}

/** isp_hw_reserve_session
 *    @isp_hw:
 *    @session_id:
 *
 **/
static isp_hw_session_t *isp_hw_reserve_session(isp_hw_t *isp_hw, uint32_t session_id)
{
  int i;
  isp_hw_session_t *session = NULL;

  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (isp_hw->session[i].isp_hw == NULL && session == NULL) {
      session = &isp_hw->session[i];
    } else if (isp_hw->session[i].isp_hw != NULL &&
               isp_hw->session[i].session_id == session_id) {
      /* find a matching session */
      return &isp_hw->session[i];
    }
  }

  if (session && session->isp_hw == NULL){
    session->isp_hw = isp_hw;
    session->session_id = session_id;
    session->started = 0;

    return session;
  } else {
    return session;
  }
}

/** isp_hw_proc_stream_config
 *    @isp_hw:
 *    @session_id:
 *
 **/
static int isp_hw_proc_stream_config(isp_hw_t *isp_hw,
  isp_hw_stream_cfg_t *in_params, uint32_t in_params_size)
{
  int i, rc = 0;
  isp_hw_session_t *session;

  if (sizeof(isp_hw_stream_cfg_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  if (in_params->num_streams == 0)
    goto end;

  session = isp_hw_reserve_session(isp_hw,  in_params->session_id);

  if (!session) {
    /* no more free session */
    CDBG_ERROR("%s: no more free session\n", __func__);
    return -1;
  }

  session->ion_fd = in_params->ion_fd;
  session->hfr_param = in_params->hfr_param;
  session->vt_enable = in_params->vt_enable;

  /* in case RDI is configured after pix stream we
   * cannot overwrite the camif clock, so we set the
   * clk to be zero to start. vfe pipeline stream
   * always overwrite the clock but RDI stream will not do that.
   */
  isp_hw->input.camif_cfg.pixel_clock = 0;

  for (i = 0; i < in_params->num_streams; i++) {
    if (session->streams[i].state >= ISP_HW_STREAM_STATE_CFG)
      continue;

    session->streams[i].cfg = in_params->entries[i];
    session->streams[i].state = ISP_HW_STREAM_STATE_CFG;

    if (session->streams[i].cfg.use_pix)
      session->use_pipeline = TRUE;

    ISP_DBG(ISP_MOD_COM,"%s: i = %d, sessid = %d, streamid = %d, straem = %p\n", __func__, i,
      in_params->session_id, session->streams[i].cfg.stream_id,
      &session->streams[i]);

    rc = isp_hw_stream_config(isp_hw, session, &session->streams[i]);

    if (rc < 0) {
      CDBG_ERROR("%s: pipeline/axi config error = %d\n", __func__, rc);
      break;
    }
  }

end:

  if (rc < 0)
    CDBG_ERROR("%s: stream config error = %d\n", __func__, rc);

  return rc;
}

/** isp_hw_get_stream_cnt
 *    @session:
 *
 **/
static int isp_hw_get_stream_cnt(isp_hw_session_t *session)
{
  int cnt = 0, i;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (session->streams[i].state != ISP_HW_STREAM_STATE_NULL)
      cnt++;
  }

  return cnt;
}

/** isp_hw_proc_ihist_la_trigger_update
 *    @isp_hw:
 *    @ihist_stats
 *    @in_params_size
 *
 **/
static int isp_hw_proc_ihist_la_trigger_update(isp_hw_t *isp_hw,
  q3a_ihist_stats_t *ihist_stats, uint32_t in_params_size)
{
   int rc = 0;

   if (ihist_stats != 0) {
      rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
        ISP_PIX_SET_IHIST_LA_TRIGGER_UPDATE, (void *)ihist_stats,
        sizeof(q3a_ihist_stats_t));
   }

   return rc;
}

/** isp_hw_proc_stream_unconfig
 *    @isp_hw:
 *    @ihist_stats
 *    @in_params_size
 *
 **/
static int isp_hw_proc_stream_unconfig(isp_hw_t *isp_hw,
  isp_hw_stream_uncfg_t *in_params, uint32_t in_params_size)
{
  int i, k, rc = 0;
  int stream_cnt = 0;
  isp_hw_session_t *session;
  int use_pix = 0;
  isp_hw_stream_uncfg_t stream_uncfg;
  isp_hwif_output_cfg_t stream_cfg;

  if (sizeof(isp_hw_stream_uncfg_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  session = isp_hw_find_session(isp_hw, in_params->session_id);

  if (!session)
    return -EAGAIN;

  for (i = 0; i < in_params->num_streams; i++) {
    isp_hw_stream_t *stream = NULL;
    for (k = 0; k < ISP_MAX_STREAMS; k++) {
      if (session->streams[k].state != ISP_HW_STREAM_STATE_NULL &&
          session->streams[k].cfg.stream_id == in_params->stream_ids[i]) {
        /* found the stream */
        stream = &session->streams[k];
        isp_hw_sel_axi_path(isp_hw, stream, &stream_cfg);
        isp_hw_input_uncfg(isp_hw, stream_cfg.isp_output_interface);

        stream_uncfg.num_streams = 1;
        stream_uncfg.session_id = session->session_id;
        stream_uncfg.stream_ids[0] = stream->cfg.stream_id;
        rc = isp_axi_set_params(isp_hw->axi.private_data,
          ISP_AXI_SET_STREAM_UNCFG, (void *)&stream_uncfg,
          sizeof(stream_uncfg));

        if (rc < 0) {
          CDBG_ERROR("%s: AXI stream uncfg error = %d\n", __func__, rc);
          return rc;
        }

        if (stream->cfg.use_pix) {
          rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
            ISP_PIX_SET_STREAM_UNCFG, (void *)&stream_uncfg,
            sizeof(stream_uncfg));

          if (rc < 0) {
            CDBG_ERROR("%s: PIX stream uncfg error = %d\n", __func__, rc);
            return rc;
          }
        }
        memset(stream, 0, sizeof(*stream));
      }
    }
  }

  stream_cnt = isp_hw_get_stream_cnt(session);

  if (!stream_cnt) {
    /* no stream in session release the session */
    memset(session,  0,  sizeof(isp_hw_session_t));
  }

  return 0;
}

/** isp_hw_proc_set_effect
 *    @isp: isp root
 *    @isp_sink_port: isp sink port
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @effect: effect data
 *
 *  Sets effect parameter to pipeline.
 **/
static int isp_hw_proc_set_effect(isp_hw_t *isp_hw, int32_t *effect,
  uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(int32_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_EFFECT, (void *)effect, in_params_size);

  return rc;
}

/** isp_hw_proc_set_sharpness
 *    @isp: isp root
 *    @isp_sink_port: isp sink port
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @sharpness: sharpness value
 *
 *  Sets sharpness parameter to pipeline.
 **/
static int isp_hw_proc_set_sharpness(
  isp_hw_t *isp_hw,
  int32_t *sharpness,
  uint32_t in_params_size)
{
  int rc = 0;
  if (sizeof(int32_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }
  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_SHARPNESS, (void *)sharpness, in_params_size);
  return rc;
}

/** isp_hw_proc_set_saturation
 *    @isp_hw: isp hw
 *    @saturation: saturation data
 *    @in_param_size: size of the data
 *
 *  Sets effect parameter to pipeline.
 **/
static int isp_hw_proc_set_saturation(isp_hw_t *isp_hw,
  isp_saturation_setting_t *saturation, uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(isp_saturation_setting_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_SATURATION, (void *)saturation, in_params_size);

  return rc;
}

/** isp_hw_proc_set_bracketing_data
 *    @isp: isp root
 *    @isp_sink_port: isp sink port
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @bracketing_data: bracketing data
 *
 *  Sets bracketing data parameter to pipeline.
 **/
static int isp_hw_proc_set_bracketing_data(isp_hw_t *isp_hw,
  mct_bracket_ctrl_t *bracketing_data, uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(mct_bracket_ctrl_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_BRACKETING_DATA, (void *)bracketing_data, in_params_size);

  return rc;
}

/** isp_hw_proc_set_bestshot
 *    @isp: isp root
 *    @isp_sink_port: isp sink port
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @bestshot: bestshot data
 *
 *  Sets bestshot parameter to pipeline.
 **/
static int isp_hw_proc_set_bestshot(isp_hw_t *isp_hw,
  cam_scene_mode_type *bestshot, uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(cam_scene_mode_type) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_BESTSHOT, (void *)bestshot, in_params_size);

  return rc;
}

/** isp_hw_proc_get_rolloff
 *    @isp: isp_hw root
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @rolloff_table: rolloff table data
 *
 *  Gets the rolloff table.
 **/
static int isp_hw_proc_get_rolloff(isp_hw_t *isp_hw,
  mct_event_stats_isp_rolloff_t *rolloff_table, uint32_t out_params_size)
{
  int rc = 0;

  if (sizeof(mct_event_stats_isp_rolloff_t) != out_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
    ISP_PIX_GET_ROLLOFF_TABLE, NULL, 0, (void*)rolloff_table, out_params_size);

  return rc;
}

/** isp_hw_proc_get_cds_trigger_val
 *    @isp: isp_hw root
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @isp_uv_subsample_t: rolloff table data
 *
 *  Gets the uv subsampling hystersis trigger points.
 *
 *  Return 0 success and negative value for failure.
 **/
static int isp_hw_proc_get_cds_trigger_val(isp_hw_t *isp_hw, void *in_params,
  uint32_t in_params_size, isp_uv_subsample_t *uv_sbsampling_ctrl,
  uint32_t out_params_size)
{
  int rc = 0;
  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  if (sizeof(isp_uv_subsample_t) != out_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }
  rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
      ISP_PIX_GET_CDS_TRIGGER_VAL, in_params, in_params_size,
      (void*)uv_sbsampling_ctrl, out_params_size);
  ISP_DBG(ISP_MOD_COM,"%s: X rc = %d\n", __func__, rc);
  return rc;
}

/** isp_hw_proc_set_contrast
 *    @isp: isp root
 *    @isp_sink_port: isp sink port
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @contrast: contrast data
 *
 *  Sets effect parameter to pipeline.
 **/
static int isp_hw_proc_set_contrast(isp_hw_t *isp_hw, int32_t *contrast,
  uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(int32_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_CONTRAST, (void *)contrast, sizeof(in_params_size));

  return rc;
}

/** isp_hw_proc_set_sce
 *    @isp: isp root
 *    @isp_sink_port: isp sink port
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @sce_factor: sce_factor data
 *
 *  Sets sce_factor parameter to pipeline.
 **/
static int isp_hw_proc_set_sce(isp_hw_t *isp_hw, int32_t *sce_factor,
  uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(cam_scene_mode_type) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_SCE, (void *)sce_factor, sizeof(in_params_size));

  return rc;
}

/** isp_hw_proc_set_recording_hint
*    @isp_hw: isp hw
*    @recording_hint: recording hint
*    @in_params_size: parameter size
*
*  Sets recording hint parameter to pipeline.
**/
static int isp_hw_proc_set_recording_hint(isp_hw_t *isp_hw,
  int32_t *recording_hint, uint32_t in_params_size)
{
  int rc = 0;

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_RECORDING_HINT, (void *)recording_hint,
    sizeof(in_params_size));

  return rc;
}

/** isp_hw_proc_set_vhdr
 *    @isp: isp root
 *    @isp_sink_port: isp sink port
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @sce_factor: sce_factor data
 *
 *  Sets sce_factor parameter to pipeline.
 **/
static int isp_hw_proc_set_vhdr(isp_hw_t *isp_hw, uint32_t *vhdr_enable,
  uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(cam_scene_mode_type) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_VHDR, (void *)vhdr_enable, sizeof(in_params_size));

  return rc;
}

/** isp_hw_proc_set_tintless
 *
 *    @isp_hw: isp_hw root
 *    @is_tintless_supported:
 *    @param_size:
 *
 *  Sets tintless support parameter.
 *
 **/
static int isp_hw_proc_set_tintless(isp_hw_t *isp_hw, isp_tintless_data_t *tintless_data,
  uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(*tintless_data) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
         ISP_PIX_SET_TINTLESS, (void *)tintless_data,
         sizeof(in_params_size));

  return rc;
}


/** isp_hw_proc_set_module_trigger
 *    @isp: isp root
 *    @trg_enable
 *    @in_parms_size
 *  Sets module trigger
 *  enable/diable parameter to pipeline.
 **/
static int isp_hw_proc_set_module_trigger(isp_hw_t *isp_hw, isp_mod_trigger_t *tgr_enable,
  uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(isp_mod_trigger_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_MOD_TRIGGER, (void *)tgr_enable, sizeof(in_params_size));

  return rc;
}

/** isp_hw_proc_set_module_enable
 *    @isp: isp root
 *    @trg_enable
 *    @in_parms_size
 *  Sets module trigger
 *  enable/diable parameter to pipeline.
 **/
static int isp_hw_proc_set_module_enable(isp_hw_t *isp_hw, isp_mod_trigger_t *tgr_enable,
  uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(isp_mod_trigger_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_MOD_ENABLE, (void *)tgr_enable, sizeof(in_params_size));

  return rc;
}

/** isp_hw_proc_set_module_trigger
 *    @isp: isp root
 *    @isp_sink_port: isp sink port
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @sce_factor: sce_factor data
 *
 *  Sets sce_factor parameter to pipeline.
 **/
static int isp_hw_proc_set_isp_diagnostics(isp_hw_t *isp_hw, int32_t *diag_enable,
  uint32_t in_params_size)
{
  if (sizeof(int32_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  isp_hw->isp_diag.has_user_dianostics = *diag_enable;
  return 0;
}
/** isp_hw_proc_set_af_rolloff_params
 *    @isp_hw: isp root
 *    @in_params:
 *    @in_params_size:
 *
 **/
static int isp_hw_proc_set_af_rolloff_params(isp_hw_t *isp_hw,
  af_rolloff_info_t *in_params, uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(af_rolloff_info_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch. Expecting %d Received %d\n", __func__,
      sizeof(af_rolloff_info_t), in_params_size);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_AF_ROLLOFF_PARAMS, (void *)in_params, sizeof(in_params_size));

  return rc;
}

/** isp_hw_proc_set_chromatix
 *    @isp_hw: isp root
 *    @in_params:
 *    @in_params_size:
 *
 **/
static int isp_hw_proc_set_chromatix(isp_hw_t *isp_hw,
  modulesChromatix_t *in_params, uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(modulesChromatix_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch. Expecting %d Received %d\n", __func__,
      sizeof(modulesChromatix_t), in_params_size);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_CHROMATIX, (void *)in_params, sizeof(in_params_size));

  return rc;
}

/** isp_hw_proc_set_chromatix
 *    @isp_hw: isp root
 *    @in_params:
 *    @in_params_size:
 *
 **/
static int isp_hw_proc_set_reload_chromatix(isp_hw_t *isp_hw,
  modulesChromatix_t *in_params, uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(modulesChromatix_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch. Expecting %d Received %d\n", __func__,
      sizeof(modulesChromatix_t), in_params_size);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_RELOAD_CHROMATIX, (void *)in_params, sizeof(in_params_size));

  return rc;
}

/** isp_hw_proc_set_chromatix
 *    @isp_hw: isp root
 *    @in_params:
 *    @in_params_size:
 *
 **/
static int isp_hw_proc_stats_config_update(isp_hw_t *isp_hw,
  stats_config_t *stats_config, uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(stats_config_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
        ISP_PIX_SET_STATS_CFG_UPDATE, (void *)stats_config,
        sizeof(stats_config_t));
  return rc;
}

/** isp_hw_proc_sensor_lens_position_trigger_update
 *    @isp_hw: isp root
 *    @lens_position_update:
 *    @in_params_size:
 *
 **/
static int isp_hw_proc_sensor_lens_position_trigger_update(isp_hw_t *isp_hw,
  lens_position_update_isp_t *lens_position_update, uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(lens_position_update_isp_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_SENSOR_LENS_POSITION_TRIGGER_UPDATE,
    (void *)lens_position_update, sizeof(lens_position_update_isp_t));

  return rc;
}

/** isp_hw_proc_awb_trigger_update
 *    @isp_hw: isp root
 *    @stats_update:
 *    @in_params_size:
 *
 **/
static int isp_hw_proc_awb_trigger_update(isp_hw_t *isp_hw,
  stats_update_t *stats_update, uint32_t in_params_size)
{
  int rc = 0;

  ISP_DBG(ISP_MOD_COM,"%s: flag = 0x%x\n", __func__, stats_update->flag);

  if (sizeof(stats_update_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_AWB_TRIGGER_UPDATE, (void *)stats_update,
    sizeof(stats_update_t));

  return rc;
}

/** isp_hw_proc_aec_trigger_update
 *    @isp_hw: isp root
 *    @aec_gain:
 *    @in_params_size:
 *
 **/
static int isp_hw_proc_aec_trigger_update(isp_hw_t *isp_hw, float *aec_gain,
  uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(float) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_AEC_TRIGGER_UPDATE, (void *)aec_gain, sizeof(float));

  return rc;
};

static int isp_hw_proc_flash_mode(isp_hw_t *isp_hw,
  cam_flash_mode_t *flash_mode, uint32_t in_params_size)
{
  int rc = 0;

  if (sizeof(cam_flash_mode_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
        ISP_PIX_SET_FLASH_MODE, (void *)flash_mode,
        sizeof(float));

  return rc;
};

/** isp_hw_proc_aec_trigger_update
 *    @isp_hw: isp root
 *    @aec_gain:
 *    @in_params_size:
 *
 **/
static int isp_hw_save_mapped_buf(isp_hw_t *isp_hw,
  isp_mapped_buf_params_t *in_params, uint32_t in_params_size)
{
  int rc = 0;
  isp_hw_session_t *session;
  isp_hw_stream_t *stream;

  if (NULL == in_params) {
    return -1;
  }

  session = isp_hw_find_session(isp_hw, in_params->session_id);
  if (NULL == session) {
    CDBG_ERROR("%s: Session could not be found! \n", __func__);
    return -1;
  }

  stream = isp_hw_find_stream_in_session(session, in_params->stream_id);
  if (NULL == stream) {
    CDBG_ERROR("%s: Stream could not be found! \n", __func__);
    return -1;
  }

  stream->cfg.map_buf = *in_params;
  ISP_DBG(ISP_MOD_COM,"%s: sessid = %d, streamid = %d, num_buf = %d, use_native = %d\n",
    __func__, stream->cfg.map_buf.session_id, stream->cfg.map_buf.stream_id,
    stream->cfg.map_buf.num_bufs, stream->cfg.map_buf.use_native_buf);

  return rc;
}

/** isp_hw_set_stats_config
 *    @isp_hw: isp root
 *    @aec_gain:
 *    @in_params_size:
 *
 **/
static int isp_hw_set_stats_config(
  isp_hw_t *isp_hw,
  isp_stats_config_t *stats_cfg,
  uint32_t in_params_size)
{
  int rc = 0;

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_STATS_CFG, (void *)stats_cfg, sizeof(isp_stats_config_t));

  return rc;
}

/** isp_hw_set_frame_skip
 *    @isp_hw: isp root
 *    @frame_skip:
 *    @in_params_size:
 *
 **/
static int isp_hw_set_frame_skip(isp_hw_t *isp_hw,
  isp_param_frame_skip_pattern_t *frame_skip, uint32_t in_params_size)
{
  int rc = 0;

  rc = isp_axi_set_params(isp_hw->axi.private_data,
    ISP_AXI_SET_PARAM_FRAME_SKIP, frame_skip, in_params_size);

  return rc;
}

/** isp_hw_set_crop_factor
 *    @isp_hw: isp root
 *    @crop_factor:
 *    @size:
 *
 **/
static int isp_hw_set_crop_factor(isp_hw_t *isp_hw,
  isp_hw_set_crop_factor_t *crop_factor, uint32_t size)
{
  int rc = 0;

  isp_hw_session_t *session =
    isp_hw_find_session(isp_hw, crop_factor->session_id);

  if (!session) {
    CDBG_ERROR("%s: error - no session (%d) found\n",
      __func__, crop_factor->session_id);
    return -1;
  }

  if (session->use_pipeline == FALSE) {
    CDBG_ERROR("%s: this session does not use pipeline\n", __func__);
    return -1;
  }

  rc = isp_pipeline_set_params(isp_hw->pipeline.private_data,
    ISP_PIX_SET_PARAM_CROP_FACTOR, (void *)crop_factor, sizeof(*crop_factor));

  return rc;
}

/** isp_hw_proc_set_params
 *
 *    @ctrl: isp root
 *    @params_id:
 *    @in_params:
 *    @in_params_size:
 *
 **/
int isp_hw_proc_set_params(void *ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size)
{
  int rc = 0;
  isp_hw_t *isp_hw = ctrl;
  uint32_t pix_params_id = ISP_PIX_SET_PARAM_INVALID;
  uint32_t axi_params_id = 0;

  ISP_DBG(ISP_MOD_COM,"%s: E, id = %d\n", __func__, params_id);

  switch (params_id) {
  case ISP_HW_SET_PARAM_STREAM_CFG: {
    rc = isp_hw_proc_stream_config(isp_hw, (isp_hw_stream_cfg_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_STREAM_UNCFG: {
    rc = isp_hw_proc_stream_unconfig(isp_hw, (isp_hw_stream_uncfg_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_IHIST_LA_TRIGGER_UPDATE: {
    pthread_mutex_lock(&isp_hw->thread_hw.busy_mutex);
    /* If hw update in progress, drop current trigger update */
    if(!isp_hw->thread_hw.thread_busy) {
      pthread_mutex_unlock(&isp_hw->thread_hw.busy_mutex);
      rc = isp_hw_proc_ihist_la_trigger_update(isp_hw,
        (q3a_ihist_stats_t *)in_params, in_params_size);
    } else {
      pthread_mutex_unlock(&isp_hw->thread_hw.busy_mutex);
    }
  }
    break;

  case ISP_HW_SET_PARAM_CONTRAST: {
    rc = isp_hw_proc_set_contrast(isp_hw, (int32_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_EFFECT: {
    rc = isp_hw_proc_set_effect(isp_hw, (int32_t *)in_params, in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_SATURATION: {
    rc = isp_hw_proc_set_saturation(isp_hw,
      (isp_saturation_setting_t *)in_params, in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_SHARPNESS:
    rc = isp_hw_proc_set_sharpness(isp_hw, (int32_t *)in_params,
      in_params_size);
    break;

  case ISP_HW_SET_PARAM_AF_ROLLOFF_PARAMS: {
    rc = isp_hw_proc_set_af_rolloff_params(isp_hw,
      (af_rolloff_info_t *)in_params, in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_CHROMATIX: {
    rc = isp_hw_proc_set_chromatix(isp_hw, (modulesChromatix_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_RELOAD_CHROMATIX: {
    rc = isp_hw_proc_set_reload_chromatix(isp_hw,
           (modulesChromatix_t *)in_params, in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_MAPPED_BUF: {
    rc = isp_hw_save_mapped_buf(isp_hw, (isp_mapped_buf_params_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_STATS_CFG_UPDATE: {
    rc = isp_hw_proc_stats_config_update(isp_hw, (stats_config_t *)in_params,
           in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_SENSOR_LENS_POSITION_TRIGGER_UPDATE: {
    pthread_mutex_lock(&isp_hw->thread_hw.busy_mutex);

    /* If hw update in progress, drop current trigger update */
    if(!isp_hw->thread_hw.thread_busy) {
      pthread_mutex_unlock(&isp_hw->thread_hw.busy_mutex);
      rc = isp_hw_proc_sensor_lens_position_trigger_update(isp_hw,
        (lens_position_update_isp_t *)in_params, in_params_size);
    } else {
      pthread_mutex_unlock(&isp_hw->thread_hw.busy_mutex);
    }
  }
    break;

  case ISP_HW_SET_PARAM_AWB_TRIGGER_UPDATE: {
    pthread_mutex_lock(&isp_hw->thread_hw.busy_mutex);
    /* If hw update in progress, drop current trigger update */
    if(!isp_hw->thread_hw.thread_busy) {
      pthread_mutex_unlock(&isp_hw->thread_hw.busy_mutex);
      rc = isp_hw_proc_awb_trigger_update(isp_hw, (stats_update_t *)in_params,
        in_params_size);
    } else {
      pthread_mutex_unlock(&isp_hw->thread_hw.busy_mutex);
    }
  }
    break;

  case ISP_HW_SET_PARAM_AEC_TRIGGER_UPDATE: {
    pthread_mutex_lock(&isp_hw->thread_hw.busy_mutex);
    /* If hw update in progress, drop current trigger update */
    if(!isp_hw->thread_hw.thread_busy) {
      pthread_mutex_unlock(&isp_hw->thread_hw.busy_mutex);
      rc = isp_hw_proc_aec_trigger_update(isp_hw, (float *)in_params,
        in_params_size);
    } else {
      pthread_mutex_unlock(&isp_hw->thread_hw.busy_mutex);
    }
  }
    break;

  case ISP_HW_SET_FLASH_MODE: {
    pthread_mutex_lock(&isp_hw->thread_hw.busy_mutex);
    /* If hw update in progress, drop current trigger update */
    if(!isp_hw->thread_hw.thread_busy) {
      pthread_mutex_unlock(&isp_hw->thread_hw.busy_mutex);
      rc = isp_hw_proc_flash_mode(isp_hw, (cam_flash_mode_t *)in_params,
        in_params_size);
    } else {
      pthread_mutex_unlock(&isp_hw->thread_hw.busy_mutex);
    }
  }
    break;

  case ISP_HW_SET_PARAM_STATS_CFG: {
    rc = isp_hw_set_stats_config(isp_hw, (isp_stats_config_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_SCE: {
    rc = isp_hw_proc_set_sce(isp_hw, (int32_t *)in_params, in_params_size);
  }
    break;

  case ISP_HW_SET_RECORDING_HINT: {
    rc = isp_hw_proc_set_recording_hint(isp_hw, (int32_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_BRACKETING_DATA: {
    rc = isp_hw_proc_set_bracketing_data(isp_hw,
      (mct_bracket_ctrl_t *)in_params, in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_BEST_SHOT: {
    rc = isp_hw_proc_set_bestshot(isp_hw, (cam_scene_mode_type *)in_params,
      in_params_size);
  }
    break;


  case ISP_HW_SET_PARAM_SAVE_AEC_PARAMS: {
    rc = isp_hw_proc_save_aec_params(isp_hw, (stats_update_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_SET_UV_SUBSAMPLE: {
    rc = isp_hw_proc_set_uv_subsample(isp_hw,
      in_params, in_params_size);
  }
     break;

  case ISP_HW_SET_PARAM_HW_UPDATE_SKIP: {
    isp_hw->hw_update_skip = *((uint32_t *)in_params);
  }
    break;

  case ISP_HW_SET_PARAM_SAVE_ASD_PARAMS: {
    rc = isp_hw_proc_save_asd_params(isp_hw, (stats_update_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_SET_SAVED_PARAMS: {
    rc = isp_hw_proc_set_saved_params(isp_hw,
      (isp_saved_params_t *)in_params, in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_FRAME_SKIP: {
    rc = isp_hw_set_frame_skip(isp_hw,
      (isp_param_frame_skip_pattern_t *)in_params, in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_CROP_FACTOR: {
    rc = isp_hw_set_crop_factor(isp_hw, (isp_hw_set_crop_factor_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_VHDR: {
    rc = isp_hw_proc_set_vhdr(isp_hw, (uint32_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_PARAM_TINTLESS: {
    rc = isp_hw_proc_set_tintless(isp_hw, (isp_tintless_data_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_ISP_DIAGNOSTICS: {
    rc = isp_hw_proc_set_isp_diagnostics(isp_hw, (int32_t*)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_ISP_MODULE_TRIGGER: {
    rc = isp_hw_proc_set_module_trigger(isp_hw, (isp_mod_trigger_t*)in_params,
      in_params_size);
  }
    break;

  case ISP_HW_SET_ISP_MODULE_ENABLE: {
    rc = isp_hw_proc_set_module_enable(isp_hw, (isp_mod_trigger_t*)in_params,
      in_params_size);
  }
    break;

  default: {
  }
    break;
  }

  return rc;
}

/** isp_hw_get_la_gamma_config
 *
 *    @isp_hw: isp root
 *    @out_params:
 *    @out_params_size:
 *
 **/
static int isp_hw_get_la_gamma_config(
  isp_hw_t *isp_hw, void *out_params,
  uint32_t out_params_size)
{
  int rc = 0;

  if (sizeof(mct_isp_table_t) != out_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  if (isp_hw->pipeline.private_data) {
    rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
      ISP_PIX_GET_LA_GAMMA_TBLS, NULL, 0,
      out_params, out_params_size);
  } else {
    CDBG_ERROR("%s: no pipeline running. Error!!!!\n",  __func__);
    /* memset to make it nop(num = 0) */
    memset(out_params, 0, out_params_size);
  }

  return rc;

}

/** isp_hw_get_cs_rs_config
 *
 *    @isp_hw: isp root
 *    @cs_rs_cfg:
 *
 **/
static int isp_hw_get_cs_rs_config(isp_hw_t *isp_hw,
  isp_cs_rs_config_t *cs_rs_cfg)
{
  int rc = 0;
  isp_hw_session_t *session =
    isp_hw_find_session(isp_hw, cs_rs_cfg->session_id);

  if (!session || session->use_pipeline == FALSE) {
    CDBG_ERROR("%s: not using pipeline. error\n", __func__);
    return -1;
  }

  rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
    ISP_PIX_GET_CS_RS_CONFIG, NULL, 0,
    cs_rs_cfg, sizeof(isp_cs_rs_config_t));
  return rc;
}

/** isp_hw_get_roi_map
 *
 *    @isp_hw: isp root
 *    @out_params:
 *    @out_params_size:
 *
 **/
static int isp_hw_get_roi_map(isp_hw_t *isp_hw, void *out_params,
  uint32_t out_params_size)
{
  int rc = 0;

  if (sizeof(isp_hw_zoom_param_entry_t) != out_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return 0;
  }

  if (isp_hw->pipeline.private_data) {
    rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
      ISP_PIX_GET_ROI_MAP, NULL, 0,
      out_params, out_params_size);
  } else {
    CDBG_ERROR("%s: no pipeline running. Error!!!!\n",  __func__);
    /* memset to make it nop(num = 0) */
    memset(out_params, 0, out_params_size);
  }

  return rc;
}

/** isp_hw_get_rolloff_grid_info
 *
 *    @isp_hw: isp root
 *    @grid_info:
 *    @out_params_size:
 *
 **/
static int isp_hw_get_rolloff_grid_info(isp_hw_t *isp_hw, uint32_t *grid_info,
  uint32_t out_params_size)
{
  int rc = 0;

  if (isp_hw->pipeline.private_data) {
     rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
       ISP_PIX_GET_ROLLOFF_GRID_INFO, NULL, 0, grid_info, sizeof(uint32_t));
  } else {
    CDBG_ERROR("%s: no pipeline running, error!\n", __func__);
    memset(grid_info, 0, out_params_size);
  }

  return rc;
}

/** isp_hw_get_fov
 *
 *    @isp_hw: isp root
 *    @out_params:
 *    @out_params_size:
 *
 **/
static int isp_hw_get_fov(isp_hw_t *isp_hw, void *out_params,
  uint32_t out_params_size)
{
  int rc = 0;

  if (sizeof(isp_hw_zoom_param_t) != out_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return 0;
  }

  if (isp_hw->pipeline.private_data) {
    rc = isp_pipeline_get_params(isp_hw->pipeline.private_data,
      ISP_PIX_GET_FOV, NULL, 0,
      out_params, out_params_size);
  } else {        0,
    CDBG_ERROR("%s: no pipeline running. Error!!!!\n",  __func__);
    /* memset to make it nop(num = 0) */
    memset(out_params, 0, out_params_size);
  }

  return rc;
}

/** isp_hw_proc_get_params
 *
 *    @ctrl: isp root
 *    @params_id:
 *    @in_params:
 *    @in_params_size:
 *    @out_params:
 *    @out_params_size:
 *
 **/
int isp_hw_proc_get_params(void *ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size, void *out_params, uint32_t out_params_size)
{
  int rc = 0;
  isp_hw_t *isp_hw = ctrl;

  switch (params_id) {
  case ISP_HW_GET_CS_RS_CONFIG: {
    rc = isp_hw_get_cs_rs_config(isp_hw, (isp_cs_rs_config_t *)out_params);
  }
    break;

  case ISP_HW_GET_PARAM_LA_GAMMA_TBLS: {
    rc = isp_hw_get_la_gamma_config(isp_hw, out_params, out_params_size);
  }
    break;

  case ISP_HW_GET_FOV_CROP: {
    rc = isp_hw_get_fov(isp_hw, out_params, out_params_size);
  }
    break;

  case ISP_HW_GET_ROI_MAP: {
    rc = isp_hw_get_roi_map(isp_hw, out_params, out_params_size);
  }
    break;

  case ISP_HW_GET_ROLLOFF_GRID_INFO: {
    rc = isp_hw_get_rolloff_grid_info(isp_hw, (uint32_t *)out_params, out_params_size);
  }
    break;

  case ISP_HW_GET_CAPABILITY: {
  }
    break;

  case ISP_HW_GET_PARAM_ROLLOFF_TABLE: {
    rc = isp_hw_proc_get_rolloff(isp_hw,
      (mct_event_stats_isp_rolloff_t *)out_params, out_params_size);
  }
    break;

  case ISP_HW_GET_PARAM_CDS_TRIGER_VAL: {
    rc = isp_hw_proc_get_cds_trigger_val(isp_hw, in_params, in_params_size,
    (isp_uv_subsample_t *)out_params, out_params_size);
  }
    break;

  default: {

  }
    break;
  }

  return rc;
}

/** isp_hw_init
 *
 *    @ctrl: isp root
 *    @out_params_size:
 *    @notify_ops:
 *
 **/
static int isp_hw_init(void *ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  int len, rc = 0;
  isp_hw_t *isp_hw = ctrl;
  isp_pipe_notify_ops_init_t init_data;
  uint32_t cmd = MM_ISP_CMD_NOTIFY_OPS_INIT;

  init_data.in_params = in_params;
  init_data.notify_ops = notify_ops;

  pthread_mutex_lock(&isp_hw->thread_poll.cmd_mutex);

  isp_hw->thread_poll.init_cmd = &init_data;

  len = write(isp_hw->thread_poll.pipe_fds[1], &cmd, sizeof(cmd));
  if (len != sizeof(cmd)) {
    /* we got error here */
    pthread_mutex_unlock(&isp_hw->thread_poll.cmd_mutex);
    return -EPIPE;
  }

  sem_wait(&isp_hw->thread_poll.sig_sem);
  rc = isp_hw->thread_poll.return_code;

  pthread_mutex_unlock(&isp_hw->thread_poll.cmd_mutex);

  return rc;
}

/** isp_hw_destroy_thread
 *
 *    @thread: information about the thread to be destroyed
 *
 *  Destroys isp hw thread
 **/
static int isp_hw_destroy_thread(isp_thread_t *thread)
{
  int len;
  uint32_t cmd = MM_ISP_CMD_DESTROY;

  pthread_mutex_lock(&thread->cmd_mutex);

  if (thread->return_code == -EPIPE) {
    CDBG_ERROR("%s: Pipe read error\n", __func__);
    pthread_mutex_unlock(&thread->cmd_mutex);
    return -EPIPE;
  }

  len = write(thread->pipe_fds[1], &cmd, sizeof(cmd));
  if (len != sizeof(cmd)) {
    /* we got error here */
    pthread_mutex_unlock(&thread->cmd_mutex);
    CDBG_ERROR("%s: Pipe write error\n", __func__);
    return -EPIPE;
  }

  /* we do not wait on sem but join to wait the thread to die. */
  if (pthread_join(thread->pid, NULL) != 0)
    ISP_DBG(ISP_MOD_COM,"%s: pthread dead already\n", __func__);

  sem_destroy(&thread->sig_sem);

  pthread_mutex_destroy(&thread->busy_mutex);
  pthread_mutex_unlock(&thread->cmd_mutex);
  pthread_mutex_destroy(&thread->cmd_mutex);

  return 0;
}

/** isp_hw_destroy
 *
 *    @ctrl:
 *
 *  Destroys isp hw
 **/
static int isp_hw_destroy(void *ctrl)
{
  int i, len, rc = 0;
  isp_hw_t *isp_hw = ctrl;
  struct msm_vfe_smmu_attach_cmd cmd;

  rc = isp_hw_destroy_thread(&isp_hw->thread_poll);
  if(rc < 0)
    goto EXIT;

  rc = isp_sem_thread_stop(&isp_hw->thread_stream);
  if(rc < 0)
    goto EXIT;

  rc = isp_hw_destroy_thread(&isp_hw->thread_hw);
  if(rc < 0)
    goto EXIT;

  /* free the memory */
  for (i = 0; i < ISP_META_MAX; i++) {
    free(isp_hw->dump_info.meta_data.meta_entry[i].isp_meta_dump);
  }
  memset(&cmd, 0, sizeof(cmd));

  cmd.security_mode = NON_SECURE_MODE;
  cmd.iommu_attach_mode = IOMMU_DETACH;
  rc = ioctl(isp_hw->fd, VIDIOC_MSM_ISP_SMMU_ATTACH, &cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: isp smmu detach error = %d\n", __func__, rc);
    return -1;
  }
  rc = isp_hw_subscribe_one_v4l2_event(isp_hw, ISP_EVENT_WM_BUS_OVERFLOW, FALSE);
  if (rc < 0) {
    CDBG_ERROR("%s: isp de-subscribe event error = %d\n", __func__, rc);
    return -1;
  }
  if (isp_hw->fd > 0) {
    close(isp_hw->fd);
    isp_hw->fd = 0;
  }
  free (isp_hw);

  return rc;

EXIT:
  isp_hw_proc_destroy(ctrl);

  return rc;
}

static int isp_hw_proc_halt(isp_hw_t *isp_hw)
{
  int rc = 0;
  rc = isp_axi_action(isp_hw->axi.private_data, ISP_AXI_ACTION_CODE_HALT,
                      NULL, 0);

  return rc;
}

static int isp_hw_proc_reset(isp_hw_t *isp_hw, void *action_data,
  uint32_t action_data_size)
{
  int rc = 0;
  rc = isp_axi_action(isp_hw->axi.private_data, ISP_AXI_ACTION_CODE_RESET,
                      action_data, action_data_size);

  return rc;
}

static int isp_hw_proc_restart(isp_hw_t *isp_hw)
{
  int rc = 0;
  rc = isp_axi_action(isp_hw->axi.private_data, ISP_AXI_ACTION_CODE_RESTART,
                      NULL, 0);

  return rc;
}

/** isp_hw_set_params
 *
 *    @ctrl:
 *    @params_id:
 *    @in_params:
 *    @in_params_size:
 *
 *
 **/
static int isp_hw_set_params(void *ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size)
{
  int len, rc = 0;
  isp_hw_t *isp_hw = ctrl;

  if (params_id == ISP_HW_SET_PARAM_OVERFLOW_DETECTED) {
    pthread_mutex_lock(&isp_hw->overflow_mutex);
    isp_hw->is_overflow = *((uint32_t *)in_params);
    if (isp_hw->is_overflow) {
      sem_init(&isp_hw->reset_done, 0, 0);
    } else {
      sem_post(&isp_hw->reset_done);
    }
    pthread_mutex_unlock(&isp_hw->overflow_mutex);
  } else {
    isp_pipe_set_params_t set_params;
    uint32_t cmd = MM_ISP_CMD_SET_PARAMS;
    set_params.in_params = in_params;
    set_params.in_params_size = in_params_size;
    set_params.params_id = params_id;

    pthread_mutex_lock(&isp_hw->thread_hw.cmd_mutex);
    isp_hw->thread_hw.set_param_cmd = &set_params;
    len = write(isp_hw->thread_hw.pipe_fds[1], &cmd, sizeof(cmd));
    if (len != sizeof(cmd)) {
      /* we got error here */
      pthread_mutex_unlock(&isp_hw->thread_hw.cmd_mutex);
      return -EPIPE;
    }
    sem_wait(&isp_hw->thread_hw.sig_sem);
    rc = isp_hw->thread_hw.return_code;
    pthread_mutex_unlock(&isp_hw->thread_hw.cmd_mutex);
  }

  return rc;
}

/** isp_hw_get_params
 *
 *    @ctrl:
 *    @params_id:
 *    @in_params:
 *    @in_params_size:
 *    @out_params:
 *    @out_params_size:
 *
 *
 **/
static int isp_hw_get_params(void *ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size, void *out_params, uint32_t out_params_size)
{
  int len, rc = 0;
  isp_hw_t *isp_hw = ctrl;
  isp_pipe_get_params_t get_params;
  uint32_t cmd = MM_ISP_CMD_GET_PARAMS;

  get_params.in_params = in_params;
  get_params.in_params_size = in_params_size;
  get_params.params_id = params_id;
  get_params.out_params = out_params;
  get_params.out_params_size = out_params_size;
  pthread_mutex_lock(&isp_hw->thread_hw.cmd_mutex);
  isp_hw->thread_hw.get_param_cmd = &get_params;
  len = write(isp_hw->thread_hw.pipe_fds[1], &cmd, sizeof(cmd));
  if (len != sizeof(cmd)) {
    /* we got error here */
    pthread_mutex_unlock(&isp_hw->thread_hw.cmd_mutex);
    return -EPIPE;
  }
  sem_wait(&isp_hw->thread_hw.sig_sem);
  rc = isp_hw->thread_hw.return_code;
  pthread_mutex_unlock(&isp_hw->thread_hw.cmd_mutex);
  return rc;
}

/** isp_hw_open
 *
 *    @isp_hw:
 *    @dev_name:
 *
 **/
static int isp_hw_open(isp_hw_t *isp_hw, char *dev_name)
{
  int rc = 0;
  struct msm_vfe_smmu_attach_cmd cmd;

  memset(&cmd, 0, sizeof(cmd));
  switch (isp_hw->hw_state) {
  case ISP_HW_STATE_INVALID: {
    isp_hw->fd = open(dev_name, O_RDWR | O_NONBLOCK);

    if ((isp_hw->fd) >= MAX_FD_PER_PROCESS) {
      dump_list_of_daemon_fd();
      isp_hw->fd = -1;
      return -1;
    }
    if (isp_hw->fd <= 0) {
      CDBG_ERROR("%s: cannot open '%s'\n", __func__, dev_name);
      isp_hw->fd = 0;
      return -1;
    }

    cmd.security_mode = NON_SECURE_MODE;
    cmd.iommu_attach_mode = IOMMU_ATTACH;
    rc = ioctl(isp_hw->fd, VIDIOC_MSM_ISP_SMMU_ATTACH, &cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: isp smmu attach error = %d\n", __func__, rc);
      return -1;
    }
    rc = isp_hw_subscribe_one_v4l2_event(isp_hw, ISP_EVENT_WM_BUS_OVERFLOW, TRUE);
    if (rc < 0) {
      CDBG_ERROR("%s: isp subscribe event error = %d\n", __func__, rc);
      return -1;
    }
    /* hw opened, set state to idle */
    isp_hw->hw_state = ISP_HW_STATE_DEV_OPEN;
  }
    break;

  default: {
    /* cannot open twice, nop */
    rc = -EAGAIN;
  }
    break;
  }

  if (rc == 0) {
    /* fork the hw thread to poll on vfe subdev and pipe */
    rc = isp_thread_start(&isp_hw->thread_poll, isp_hw, isp_hw->fd);
  }

  if(rc == 0) {
    /* fork the hw thread for stream on/off */
    rc = isp_sem_thread_start(&isp_hw->thread_stream, isp_hw);
  }

  if(rc == 0) {
    /* fork the hw thread for set/get config and hw update */
    rc = isp_thread_start(&isp_hw->thread_hw, isp_hw, 0);
  }

  return rc;
}

/** isp_hw_sem_thread_action
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
static int isp_hw_sem_thread_action(void *ctrl, isp_thread_t *thread,
  void *action)
{
  int rc = 0;

  pthread_mutex_lock(&thread->cmd_mutex);
  thread->action_cmd = action;
  thread->cmd = MM_ISP_CMD_ACTION;
  sem_post(&thread->thread_wait_sem);

  sem_wait(&thread->sig_sem);
  rc = thread->return_code;
  pthread_mutex_unlock(&thread->cmd_mutex);

  return rc;
}

/** isp_hw_pipe_thread_action
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
static int isp_hw_pipe_thread_action(void *ctrl, isp_thread_t *thread,
  void *action)
{
  int len, rc = 0;
  uint32_t cmd = MM_ISP_CMD_ACTION;

  pthread_mutex_lock(&thread->cmd_mutex);

  thread->action_cmd = action;
  thread->cmd = MM_ISP_CMD_ACTION;

  len = write(thread->pipe_fds[1], &cmd, sizeof(cmd));
  if (len != sizeof(cmd)) {
    pthread_mutex_unlock(&thread->cmd_mutex);
    CDBG_ERROR("%s: write to pipe error\n", __func__);
    return -EPIPE;
  }

  sem_wait(&thread->sig_sem);
  rc = thread->return_code;

  pthread_mutex_unlock(&thread->cmd_mutex);

  return rc;
}

/** isp_hw_action
 *
 *    @ctrl:
 *    @action_code:
 *    @action_data:
 *    @action_data_size:
 *
 **/
static int isp_hw_action(void *ctrl, uint32_t action_code, void *action_data,
  uint32_t action_data_size)
{
  int len, rc = 0;
  isp_hw_t *isp_hw = ctrl;
  isp_pipe_action_t action;
  uint32_t cmd = MM_ISP_CMD_ACTION;
  isp_thread_t *thread;

  switch(action_code) {
  case ISP_HW_ACTION_CODE_STREAM_START:
  case ISP_HW_ACTION_CODE_STREAM_STOP: {
    start_stop_stream_t *param = action_data;

    if (sizeof(start_stop_stream_t) != action_data_size) {
      CDBG_ERROR("%s: error, action_code = %d, data size mismatch\n",
        __func__, action_code);
      return -1;
    }

    if (param->wait_for_sof) {
      ISP_DBG(ISP_MOD_COM,"%s: stream on thread wait for sof...\n", __func__);
      thread = &isp_hw->thread_poll;
      action.action_code = ISP_HW_ACTION_CODE_WAKE_UP_AT_SOF;
      action.data = NULL;
      action.data_size = 0;
      rc = isp_hw_pipe_thread_action(ctrl, thread, &action);
      param->wait_for_sof = 0;

      thread = &isp_hw->thread_stream;
      action.action_code = action_code;
      action.data = action_data;
      action.data_size = action_data_size;
      rc = isp_hw_sem_thread_action(ctrl, thread, &action);
    } else {
      action.action_code = action_code;
      action.data = action_data;
      action.data_size = action_data_size;

      if(action_code == ISP_HW_ACTION_CODE_STREAM_START) {
         if (isp_hw->pipeline.num_active_streams == 0) {
           CDBG_HIGH("%s: first start, hw thread for streamon/ack\n",
             __func__);
           thread = &isp_hw->thread_hw;
           isp_hw->use_hw_thread_for_ack = TRUE;
           rc = isp_hw_pipe_thread_action(ctrl, thread, &action);
         } else {
           CDBG_HIGH("%s: runtime start, stream thread for streamon/ack\n",
             __func__);
           thread = &isp_hw->thread_stream;
           isp_hw->use_hw_thread_for_ack = FALSE;
           rc = isp_hw_sem_thread_action(ctrl, thread, &action);
         }
      } else {
        int cnt = isp_hw_count_num_pix_streams(isp_hw, param);

        CDBG_HIGH("%s: last pix stop, pix_cnt = %d, cnt = %d\n",
          __func__, isp_hw->pipeline.num_active_streams, cnt);

        if (isp_hw->pipeline.num_active_streams <= cnt &&
            isp_hw->pipeline.num_active_streams > 0) {
          thread = &isp_hw->thread_hw;
          isp_hw->use_hw_thread_for_ack = TRUE;
          rc = isp_hw_pipe_thread_action(ctrl, thread, &action);
        } else {
          isp_hw->use_hw_thread_for_ack = FALSE;
          thread = &isp_hw->thread_stream;
          rc = isp_hw_sem_thread_action(ctrl, thread, &action);
        }
      }
    }
  }
    break;

  case ISP_HW_ACTION_CODE_STREAM_START_ACK:
  case ISP_HW_ACTION_CODE_STREAM_STOP_ACK: {
    ISP_DBG(ISP_MOD_COM,"%s: Sending start/stop ack \n",__func__);
    action.action_code = action_code;
    action.data = action_data;
    action.data_size = action_data_size;

    if (isp_hw->use_hw_thread_for_ack) {
      CDBG_HIGH("%s: hw_thread for ack\n",__func__);
      thread = &isp_hw->thread_hw;
      isp_hw->use_hw_thread_for_ack = FALSE;
      rc = isp_hw_pipe_thread_action(ctrl, thread, &action);
    } else {
       CDBG_HIGH("%s: stream_thread for ack\n",__func__);
      thread = &isp_hw->thread_stream;
      rc = isp_hw_sem_thread_action(ctrl, thread, &action);
    }
  }
    break;
  case ISP_HW_ACTION_CODE_HALT: // This needs to be executed immediately in the calling thread
    rc = isp_hw_proc_halt(isp_hw);
    return rc;
  case ISP_HW_ACTION_CODE_RESET:
    rc = isp_hw_proc_reset(isp_hw, action_data, action_data_size);
    return rc;
  case ISP_HW_ACTION_CODE_RESTART:
    rc = isp_hw_proc_restart(isp_hw);
    return rc;
  default: {
    /* use hw polling thread and pipe for rest of commands */
    thread = &isp_hw->thread_poll;

    action.action_code = action_code;
    action.data = action_data;
    action.data_size = action_data_size;
    rc = isp_hw_pipe_thread_action(ctrl, thread, &action);
  }
    break;
  }

  return rc;
}

/** isp_hw_create
 *
 *    @dev_name:
 *
 **/
isp_ops_t *isp_hw_create(char *dev_name)
{
  int sd_num;
  int rc = 0, dev_fd = 0;
  isp_hw_t *isp_hw = NULL;

  isp_hw = malloc(sizeof(isp_hw_t));
  if (isp_hw == NULL) {
    /* no mem */
    CDBG_ERROR("%s: no mem", __func__);
    return NULL;
  }

  memset(isp_hw, 0, sizeof(isp_hw_t));
  isp_hw->hw_state = ISP_HW_STATE_INVALID;
  isp_hw->hw_ops.ctrl = isp_hw;
  isp_hw->hw_ops.init = isp_hw_init;
  isp_hw->hw_ops.destroy = isp_hw_destroy;
  isp_hw->hw_ops.set_params = isp_hw_set_params;
  isp_hw->hw_ops.get_params = isp_hw_get_params;
  isp_hw->hw_ops.action = isp_hw_action;

  /* open ISP and fork thread for event polling and cmd processing */
  rc = isp_hw_open(isp_hw, dev_name);
  if (rc < 0) {
    goto error;
  }

  return &isp_hw->hw_ops;

error:
  isp_hw_destroy(isp_hw->hw_ops.ctrl);

  return NULL;
}

/** isp_hw_query_caps
 *
 *    @dev_name:
 *    @isp_version:
 *    @cap:
 *    @id:
 *
 **/
int isp_hw_query_caps(const char *dev_name, uint32_t *isp_version,
  isp_hw_cap_t *cap, int id)
{
  int rc = 0;
  int fd = 0;
  uint32_t ver = 0;
  uint32_t soc_hw_ver = 0;
  uint32_t soc_hw_ver_minor = 0;
  struct msm_vfe_cfg_cmd2 cmd2;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd;
  unsigned long max_clk_rate = 0;
  unsigned long wm_ub_size = VFE40_UB_SIZE_24KB;
  uint32_t ub_policy = VFE40_UB_SLICING_POLICY_DEFAULT;

  fd = open(dev_name, O_RDWR | O_NONBLOCK);
  if (fd >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    fd = -1;
    return -1;
  }
  if (fd < 0) {
    ISP_DBG(ISP_MOD_COM,"%s: cannot open '%s'\n", __func__, dev_name);
    return -1;
  }

  memset(&cmd2, 0, sizeof(cmd2));
  memset(&reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));

  cmd2.cfg_cmd = (void *)&reg_cfg_cmd;
  cmd2.cfg_data = (void *)&ver;
  cmd2.cmd_len = sizeof(ver);
  cmd2.num_cfg = 1;

  reg_cfg_cmd.u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd.cmd_type = VFE_READ;
  reg_cfg_cmd.u.rw_info.len = sizeof(ver);
  reg_cfg_cmd.u.rw_info.reg_offset = 0;

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cmd2);
  if (rc < 0) {
    CDBG_ERROR("%s: isp version query error = %d\n", __func__, rc);
    goto end;
  }

  /* Assign HW Device details */
  cap->hw_ver = ver;

  memset(&cmd2, 0, sizeof(cmd2));
  memset(&reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));

  cmd2.cfg_cmd = (void *)&reg_cfg_cmd;
  cmd2.cfg_data = (void *)&max_clk_rate;
  cmd2.cmd_len = sizeof(max_clk_rate);
  cmd2.num_cfg = 1;

  reg_cfg_cmd.cmd_type = GET_MAX_CLK_RATE;

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cmd2);
  if (rc < 0) {
    CDBG_ERROR("%s: max rate query error = %d\n", __func__, rc);
    goto end;
  }

  cap->stats_mask = 0;
  cap->isp_info.max_pix_clk = max_clk_rate;
  cap->isp_info.longshot_max_bandwidth = 0;
  cap->isp_info.zsl_max_bandwidth = 0;

  switch (ver) {
  case ISP_MSM8974_V1:
  case ISP_MSM8974_V2:
  case ISP_MSM8974_V3: {
    if (ver == ISP_MSM8974_V1) {
      *isp_version = SET_ISP_VERSION(ISP_VERSION_40, ISP_REVISION_V1);
    } else if (ver == ISP_MSM8974_V2) {
      *isp_version = SET_ISP_VERSION(ISP_VERSION_40, ISP_REVISION_V2);
    } else if (ver == ISP_MSM8974_V3) {
      *isp_version = SET_ISP_VERSION(ISP_VERSION_40, ISP_REVISION_V3);
    }

    cap->num_pix = 1;
    cap->num_rdi = 3;
    cap->num_wms = 7;
    cap->num_register = ISP40_NUM_REG_DUMP;
    cap->stats_mask |= (1 << MSM_ISP_STATS_AWB);
    cap->stats_mask |= (1 << MSM_ISP_STATS_RS);
    cap->stats_mask |= (1 << MSM_ISP_STATS_CS);
    cap->stats_mask |= (1 << MSM_ISP_STATS_IHIST);
    cap->stats_mask |= (1 << MSM_ISP_STATS_BG);
    cap->stats_mask |= (1 << MSM_ISP_STATS_BE);
    cap->stats_mask |= (1 << MSM_ISP_STATS_BF);
    cap->stats_mask |= (1 << MSM_ISP_STATS_BHIST);
    cap->isp_info.id = id;

    if (id == 0) {
      cap->isp_info.max_resolution = 5376 * 4032;
    } else {
      cap->isp_info.max_resolution = 4288 * 3216;
    }

    cap->isp_info.ver = *isp_version;
    cap->isp_info.max_scaler_out_width = 2816;
    cap->isp_info.max_scaler_out_height = 2252;
  }
    break;

  case ISP_MSM8226_V1:
  case ISP_MSM8226_V2:
  case ISP_MSM8916:
  case ISP_MSM8939: {
    *isp_version = SET_ISP_VERSION(ISP_VERSION_40, ISP_REVISION_V2);
    /* Limit ZSL+Longshot FPS to 4.5 for 8916 and 3.9 for 8939
       at full resolution and ZSL only to 7.5. These values are
       arrived at by analysing BUS characteristics*/
    if (ver == ISP_MSM8916) {
      cap->isp_info.max_resolution = 4288 * 3216;
      cap->isp_info.max_scaler_out_width = 2112;
      cap->isp_info.max_scaler_out_height = 1188;
      cap->isp_info.longshot_max_bandwidth =
        cap->isp_info.max_resolution * ISP_LIMIT_FPS_4p5;
      cap->isp_info.zsl_max_bandwidth =
        cap->isp_info.max_resolution * ISP_LIMIT_FPS_7p5;
    } else if (ver == ISP_MSM8939) {
      cap->isp_info.max_resolution = 5376 * 4032;
      cap->isp_info.max_scaler_out_width = 2816;
      cap->isp_info.max_scaler_out_height = 1600;
      cap->isp_info.longshot_max_bandwidth =
        cap->isp_info.max_resolution * ISP_LIMIT_FPS_3p9;
    } else {
      cap->isp_info.max_scaler_out_width = 2816;
      cap->isp_info.max_scaler_out_height = 2252;
    }
    cap->num_pix = 1;
    cap->num_rdi = 3;
    cap->num_wms = 7;
    cap->num_register = ISP40_NUM_REG_DUMP;
    cap->stats_mask |= (1 << MSM_ISP_STATS_AWB);
    cap->stats_mask |= (1 << MSM_ISP_STATS_RS);
    cap->stats_mask |= (1 << MSM_ISP_STATS_CS);
    cap->stats_mask |= (1 << MSM_ISP_STATS_IHIST);
    cap->stats_mask |= (1 << MSM_ISP_STATS_BG);
    cap->stats_mask |= (1 << MSM_ISP_STATS_BE);
    cap->stats_mask |= (1 << MSM_ISP_STATS_BF);
    cap->stats_mask |= (1 << MSM_ISP_STATS_BHIST);
    cap->isp_info.id = 0;
    cap->isp_info.ver = *isp_version;
  }
    break;

  case ISP_MSM8930:
  case ISP_MSM8960V2:
  case ISP_MSM8610:
  case ISP_MSM8960V1:
  case ISP_MSM8909: {
    if ((ver == ISP_MSM8930) || (ver == ISP_MSM8610)) {
      *isp_version = SET_ISP_VERSION(ISP_VERSION_32, ISP_REVISION_V2);
    } else if (ver == ISP_MSM8960V1) {
      *isp_version = SET_ISP_VERSION(ISP_VERSION_32, ISP_REVISION_V1);
    } else if (ver == ISP_MSM8909) {
      *isp_version = SET_ISP_VERSION(ISP_VERSION_32, ISP_REVISION_V3);
    } else {
      *isp_version = SET_ISP_VERSION(ISP_VERSION_32, ISP_REVISION_V2);
    }

    if (ver == ISP_MSM8610) {
      cap->isp_info.max_scaler_out_width = 1056;
      cap->isp_info.max_scaler_out_height = 594;
    } else if(ver == ISP_MSM8909) {
      cap->isp_info.max_resolution = 3264 * 2448;
      cap->isp_info.max_scaler_out_width = 1408;
      cap->isp_info.max_scaler_out_height = 792;
      cap->isp_info.longshot_max_bandwidth =
        cap->isp_info.max_resolution * ISP_LIMIT_FPS_3p0;
	  cap->isp_info.use_pix_for_SOC = 1;
    } else {
      cap->isp_info.max_scaler_out_width = 2112;
      cap->isp_info.max_scaler_out_height = 1188;
    }

    cap->num_pix = 1;
    cap->num_rdi = 3;
    cap->num_wms = 7;
    cap->num_register = ISP32_NUM_REG_DUMP;
    cap->stats_mask |= (1 << MSM_ISP_STATS_AWB);
    cap->stats_mask |= (1 << MSM_ISP_STATS_RS);
    cap->stats_mask |= (1 << MSM_ISP_STATS_CS);
    cap->stats_mask |= (1 << MSM_ISP_STATS_IHIST);
    cap->stats_mask |= (1 << MSM_ISP_STATS_BG);
    cap->stats_mask |= (1 << MSM_ISP_STATS_BE);
    cap->stats_mask |= (1 << MSM_ISP_STATS_BF);
    cap->stats_mask |= (1 << MSM_ISP_STATS_BHIST);
    cap->stats_mask |= (1 << MSM_ISP_STATS_AEC);
    cap->stats_mask |= (1 << MSM_ISP_STATS_AF);
    cap->isp_info.id = id;
    cap->isp_info.max_resolution = 4736 * 3552;
    cap->isp_info.ver = *isp_version;
  }
    break;

  default: {
    *isp_version = 0;
    cap->num_pix = 0;
    cap->num_rdi = 0;
    cap->num_wms = 0;
  }
    break;
  }

  switch(GET_ISP_MAIN_VERSION(*isp_version)){
  case ISP_VERSION_32:
    if (ver == ISP_MSM8909)
      wm_ub_size = VFE32_UB_SIZE_32KB;
    else
      wm_ub_size = VFE32_UB_SIZE;
    break;
  case ISP_VERSION_40:
    if (ver == ISP_MSM8916) {
      wm_ub_size = VFE40_UB_SIZE_32KB;
      ub_policy = VFE40_UB_SLICING_POLICY_DEFAULT;
    } else if (ver == ISP_MSM8939) {
      wm_ub_size = VFE40_UB_SIZE_48KB;
      ub_policy = VFE40_UB_SLICING_POLICY_EQUAL;
    }
    break;
  case ISP_VERSION_44:
    wm_ub_size = VFE44_UB_SIZE;
    break;
  default:
    wm_ub_size = VFE40_UB_SIZE_24KB;
    break;
  }

  /* Send UB Size info to kernel */
  memset(&cmd2, 0, sizeof(cmd2));
  memset(&reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));

  cmd2.cfg_cmd = (void *)&reg_cfg_cmd;
  cmd2.cfg_data = (void *)&wm_ub_size;
  cmd2.cmd_len = sizeof(wm_ub_size);
  cmd2.num_cfg = 1;

  reg_cfg_cmd.cmd_type = SET_WM_UB_SIZE;

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cmd2);
  if (rc < 0) {
    CDBG_ERROR("%s: set wm ub size error = %d\n", __func__, rc);
    goto end;
  }
  memset(&cmd2, 0, sizeof(cmd2));
  memset(&reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));
  reg_cfg_cmd.cmd_type = SET_UB_POLICY;
  cmd2.cfg_cmd = (void *)&reg_cfg_cmd;
  cmd2.cfg_data = (void *)&ub_policy;
  cmd2.cmd_len = sizeof(ub_policy);
  cmd2.num_cfg = 1;


  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cmd2);
  if (rc < 0) {
    CDBG_ERROR("%s: set ub policy size error = %d\n", __func__, rc);
    goto end;
  }


end:
  close(fd);
  return rc;
}
