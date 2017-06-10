/*============================================================================
Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <linux/videodev2.h>
#include <media/msmb_camera.h>
#include <media/msmb_ispif.h>
#include <media/msmb_isp.h>
#include "mct_module.h"
#include "module_sensor.h"
#include "modules.h"
#include "camera_dbg.h"
#include "mct_stream.h"
#include "mct_pipeline.h"

#undef CDBG
#define CDBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

#if 0
#undef CDBG
#define CDBG ALOGE
#endif

#define PAD_TO_WORD(a)               (((a)+3)&~3)
#define PAD_TO_2K(a)                 (((a)+2047)&~2047)
#define PAD_TO_4K(a)                 (((a)+4095)&~4095)
#define PAD_TO_8K(a)                 (((a)+8191)&~8191)

static uint32_t stream_id = 1;
static uint32_t session_id = 1;
static uint32_t identity = (1 << 16 | 1 );

static void fill_stream_info(mct_stream_info_t *stream_info)
{
  memset(stream_info, 0, sizeof(mct_stream_info_t));
  stream_info->identity = identity;
  stream_info->stream_type = CAM_STREAM_TYPE_PREVIEW;
  stream_info->fmt = CAM_FORMAT_YUV_420_NV12;
  stream_info->dim.width = 640;
  stream_info->dim.height = 480;
  stream_info->buf_planes;
  stream_info->streaming_mode = CAM_STREAMING_MODE_CONTINUOUS;
  stream_info->num_burst = 0;
  stream_info->buf_planes.plane_info.num_planes = 2;
  stream_info->buf_planes.plane_info.mp[0].stride =  stream_info->dim.width;
  stream_info->buf_planes.plane_info.mp[0].scanline= stream_info->dim.height;
  stream_info->buf_planes.plane_info.mp[0].offset_x = 0;
  stream_info->buf_planes.plane_info.mp[0].offset_y = 0;
  stream_info->buf_planes.plane_info.mp[0].len = PAD_TO_WORD(640 * 480);
  stream_info->buf_planes.plane_info.mp[0].offset = 0;
  stream_info->buf_planes.plane_info.mp[1].offset_x = 0;
  stream_info->buf_planes.plane_info.mp[1].offset_y = 0;
  stream_info->buf_planes.plane_info.mp[0].stride =  stream_info->dim.width/2;
  stream_info->buf_planes.plane_info.mp[0].scanline= stream_info->dim.height;
  stream_info->buf_planes.plane_info.mp[1].len = PAD_TO_WORD(640 * 480 / 2);
  stream_info->buf_planes.plane_info.mp[1].offset = 0;
  stream_info->buf_planes.plane_info.frame_len =
    PAD_TO_4K(stream_info->buf_planes.plane_info.mp[0].len +
              stream_info->buf_planes.plane_info.mp[1].len);
}
static void fill_sensor_cap(sensor_src_port_cap_t *sensor_cap,
                            sensor_src_port_cap_t *ispif_sink_cap)
{
  sensor_cap->num_cid_ch = 1;
  sensor_cap->session_id = session_id;
  sensor_cap->sensor_cid_ch[0].cid = 0;
  sensor_cap->sensor_cid_ch[0].csid = 0;
  sensor_cap->sensor_cid_ch[0].csid_version = CSID_VERSION_V30;
  sensor_cap->sensor_cid_ch[0].fmt = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG;
  *ispif_sink_cap = *sensor_cap;
}
static void fill_ispif_src_cap(sensor_src_port_cap_t *ispif_sink_cap,
                               ispif_src_port_caps_t *ispif_src_cap)
{
  ispif_src_cap->sensor_cap = *ispif_sink_cap;
  ispif_src_cap->input_type = ISP_INPUT_ISPIF;
  ispif_src_cap->use_pix = 1;
}
static boolean reserve_ports(
  mct_port_t *iface_sink_port,
  mct_port_t *iface_src_port,
  mct_port_t *isp_sink_port)
{
  boolean rc = FALSE;
  mct_stream_info_t stream_info;
  sensor_src_port_cap_t sensor_cap;
  ispif_src_port_caps_t ispif_src_cap;
  sensor_src_port_cap_t ispif_sink_cap;

  memset(&stream_info, 0, sizeof(stream_info));
  memset(&sensor_cap, 0, sizeof(sensor_cap));
  memset(&ispif_src_cap, 0, sizeof(ispif_src_cap));
  memset(&ispif_sink_cap, 0, sizeof(ispif_sink_cap));
  fill_stream_info(&stream_info);
  fill_sensor_cap(&sensor_cap, &ispif_sink_cap);
  fill_ispif_src_cap(&ispif_sink_cap, &ispif_src_cap);

  rc = iface_sink_port->check_caps_reserve(iface_sink_port,
                                           (void *)&sensor_cap,
                                           &stream_info);
  if (rc != TRUE )
    goto end;
  rc = iface_sink_port->ext_link(identity, iface_sink_port, NULL);
  if (rc != TRUE )
    goto end;
  rc = iface_src_port->check_caps_reserve(iface_src_port,
                                             (void *)&sensor_cap,
                                             &stream_info);
  rc = isp_sink_port->check_caps_reserve(isp_sink_port,
                                             (void *)&ispif_src_cap,
                                             &stream_info);
  rc = iface_src_port->ext_link(identity,
                               iface_src_port, isp_sink_port );
  rc = isp_sink_port->ext_link(identity,
                               isp_sink_port, iface_src_port);
end:
  return rc;
}

static int config_isp(
  mct_port_t *iface_sink_port,
  mct_port_t *iface_src_port,
  mct_port_t *isp_sink_port)
{
  boolean rc = FALSE;
  sensor_out_info_t *sensor_cfg;
  mct_event_t *event = NULL;

  event = malloc(sizeof(mct_event_t));
  if (!event) {
    CDBG("%s: no memory\n",  __func__);
    return -1;
  }
  sensor_cfg = malloc(sizeof(sensor_out_info_t));
  if (!sensor_cfg) {
    CDBG("%s: no memory\n",  __func__);
    free (event);
    return -1;
  }
  memset(event, 0, sizeof(mct_event_t));
  memset(sensor_cfg, 0, sizeof(sensor_out_info_t));
  sensor_cfg->mode = CAMERA_MODE_2D_B;
  sensor_cfg->dim_output.width = 1984;
  sensor_cfg->dim_output.height = 1508;
  sensor_cfg->request_crop.first_line = 0;
  sensor_cfg->request_crop.last_line = 1507;
  sensor_cfg->request_crop.first_pixel = 0;
  sensor_cfg->request_crop.last_pixel = 1985;
  sensor_cfg->op_pixel_clk = 264000000; /* HZ */
  sensor_cfg->num_frames_skip = 1;
  sensor_cfg->max_gain = 16;
  sensor_cfg->max_linecount = 57888;
  sensor_cfg->vt_pixel_clk = 330000000;
  sensor_cfg->ll_pck = 4480;
  sensor_cfg->fl_lines = 2412;
  sensor_cfg->pixel_sum_factor = 1;

  event->identity = identity;
  event->direction = MCT_EVENT_DOWNSTREAM;
  event->type = MCT_EVENT_MODULE_EVENT;
  event->u.module_event.type = MCT_EVENT_MODULE_SET_STREAM_CONFIG;
  event->u.module_event.module_event_data = (void *)sensor_cfg;
  rc = iface_sink_port->event_func(iface_sink_port, event);
  free (event);
  free (sensor_cfg);
  return rc;
}

static int start_stream(
  mct_port_t *iface_sink_port,
  mct_port_t *iface_src_port,
  mct_port_t *isp_sink_port)
{
  boolean rc = FALSE;
  mct_event_t *event;

  event = malloc(sizeof(mct_event_t));
  if (!event) {
    CDBG("%s: no memory\n",  __func__);
    return -1;
  }
  memset(event, 0, sizeof(mct_event_t));
  event->identity = identity;
  event->type = MCT_EVENT_CONTROL_CMD;
  event->u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMON;
  rc = iface_sink_port->event_func(iface_sink_port, event);
  free (event);
  return rc;
}


int test_isp_init_modules()
{
  boolean rc = FALSE;
  mct_module_t *ispif_module = NULL;
  mct_module_t *isp_module = NULL;
  mct_port_t *iface_sink_port = NULL;
  mct_port_t *iface_src_port = NULL;
  mct_port_t *isp_sink_port = NULL;
  mct_list_t *srcports, *sinkports;

  mct_event_t event;
  uint32_t i = 0;

  CDBG("%s TEST_ISP: E", __func__);
  ispif_module = module_iface_init("iface");
  CDBG("%s TEST_ISP: module_iface_init = %p", __func__, ispif_module);
  if (ispif_module== NULL) {
    CDBG_ERROR("%s: ispif_module = NULL\n",  __func__);
    exit(1);
  }
  rc = ispif_module->start_session(ispif_module, session_id);
  if (rc == FALSE) {
    CDBG_ERROR("%s: TEST_ISP: ispif_module start error = %d\n",  __func__, rc);
    exit(1);
  }
  isp_module = module_isp_init("isp");
  CDBG("%s TEST_ISP: module_isp_init = %p", __func__, isp_module);
  if (isp_module== NULL) {
    CDBG_ERROR("%s: isp_module = NULL\n",  __func__);
    exit(1);
  }
  rc = isp_module->start_session(isp_module, session_id);
  if (rc == FALSE) {
    CDBG_ERROR("%s: TEST_ISP: isp_module start error = %d\n",  __func__, rc);
    exit(1);
  }
  sinkports = MCT_MODULE_SINKPORTS(ispif_module);
  CDBG("%s TEST_ISP: ispif MCT_MODULE_SINKPORTS = %p", __func__, sinkports);
  iface_sink_port = (mct_port_t *)sinkports->data;
  srcports = MCT_MODULE_SRCPORTS(ispif_module);
  CDBG("%s TEST_ISP: ispif MCT_MODULE_SRCPORTS = %p", __func__, srcports);
  iface_src_port = (mct_port_t *)srcports->data;
  sinkports = MCT_MODULE_SINKPORTS(isp_module);
  CDBG("%s TEST_ISP: isp MCT_MODULE_SRCPORTS = %p", __func__, sinkports);
  isp_sink_port = (mct_port_t *)sinkports->data;
  if (iface_sink_port == NULL || iface_sink_port->direction != MCT_PORT_SINK) {
    CDBG_ERROR("%s: no iface sink port %p\n",  __func__,  iface_sink_port);
    exit(1);
  }
  if (iface_src_port == NULL || iface_src_port->direction != MCT_PORT_SRC) {
    CDBG_ERROR("%s: no iface src port %p\n",  __func__,  iface_src_port);
    exit(1);
  }
  if (isp_sink_port == NULL || isp_sink_port->direction != MCT_PORT_SINK) {
    CDBG_ERROR("%s: no isp sink port %p\n",  __func__,  isp_sink_port);
    exit(1);
  }
  CDBG("%s TEST_ISP: reserve port", __func__);
  rc = reserve_ports(iface_sink_port, iface_src_port, isp_sink_port);
  CDBG("%s TEST_ISP: reserve port, rc = %d", __func__, rc);
  rc = config_isp(iface_sink_port, iface_src_port, isp_sink_port);
  CDBG("%s TEST_ISP: config_isp, rc = %d", __func__, rc);
  rc = start_stream(iface_sink_port, iface_src_port, isp_sink_port);
  CDBG("%s TEST_ISP: start_stream, rc = %d", __func__, rc);
  CDBG("%s TEST_ISP: X, rc = %d", __func__, rc);
  return 0;
}

