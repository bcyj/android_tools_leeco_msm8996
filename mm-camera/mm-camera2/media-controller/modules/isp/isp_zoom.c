/*============================================================================
Copyright (c) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/media.h>

#include "camera_dbg.h"
#include "cam_types.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "isp_def.h"
#include "isp_ops.h"
#include "isp_hw.h"
#include "isp.h"
#include "isp_util.h"
#include "isp_buf_mgr.h"
#include "isp_resource_mgr.h"
#include "isp_log.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef ENABLE_ZOOM_LOGGING
  #undef CDBG
  #define CDBG ALOGE
#endif

static const uint32_t isp_zoom_table_def[ZOOM_TABLE_MAX_DEF] = {
  4096, 4191, 4289, 4389, 4492,
  4597, 4705, 4815, 4927, 5042,
  5160, 5281, 5404, 5531, 5660,
  5792, 5928, 6066, 6208, 6353,
  6501, 6653, 6809, 6968, 7131,
  7298, 7468, 7643, 7822, 8004,
  8192, 8383, 8579, 8779, 8985,
  9195, 9410, 9630, 9855, 10085,
  10321, 10562, 10809, 11062, 11320,
  11585, 11856, 12133, 12416, 12706,
  13003, 13307, 13619, 13937, 14263,
  14596, 14937, 15286, 15644, 16009,
  16384, 16766, 17158, 17559, 17970,
  18390, 18820, 19260, 19710, 20171,
  20642, 21125, 21618, 22124, 22641,
  23170, 23712, 24266, 24833, 25413,
  26007, 26615, 27238, 27874, 28526,
  29192, 29875, 30573, 31288, 32019,
  32768, 33533, 34317, 35119, 35940,
  36780, 37640, 38520, 39420, 40342,
  41285, 42250, 43237, 44248, 45282,
  46340, 47424, 48532, 49666, 50827,
  52015, 53231, 54476, 55749, 57052,
  58385, 59750, 61147, 62576, 64039,
  65536, 67067, 68635, 70239, 71881,
  73561, 75281, 77040, 78841, 80684,
  82570, 84500, 86475, 88496, 90565,
  92681, 94848, 97065, 99334, 101655,
  104031, 106463, 108952, 111498, 114104,
  116771, 119501, 122294, 125152, 128078,
  131072, 134135, 137270, 140479, 143763,
  147123, 150562, 154081, 157682, 161368,
  165140, 169000, 172950, 176993, 181130,
  185363, 189696, 194130, 198668, 203311,
  208063, 212927, 217904, 222997, 228209,
  233543, 239002, 244589, 250305, 256156,
  262144, 999999
};

/** isp_zoom_calc_num_fovs:
 *
 *    @isp_version:
 *
 **/
static int isp_zoom_calc_num_fovs(uint32_t isp_version)
{
  switch (GET_ISP_MAIN_VERSION(isp_version)) {
	case ISP_VERSION_40:
	  return 2;

  case ISP_VERSION_32:
    return 1;

  default:
    return 1;
  }
}

/** zoom_init:
 *
 *    @zoom:
 *
 **/
static void zoom_init(isp_zoom_t *zoom)
{
  int i;
  int32_t minimum_value; /* Minimum allowed value */
  int32_t maximum_value; /* Maximum allowed value */

  minimum_value = 0;
  maximum_value = 0;

  /* Compute min and max zoom values:
   * now that we know the resize factor, figure out which entry in our zoom
   * table equals this zoom factor. This entry actually will correspond to
   * NO zoom, since it will tell the VFE to crop the entire image. Entry
   * min_decimation (often 0) in the table, is the smallest amount we
   * can crop, which is max zoom. */
  for (i = 0; i < zoom->zoom_data.zoom_table_size; i++)
    if (zoom->zoom_data.resize_factor < zoom->zoom_data.zoom_table[i + 1])
      break;

  /* if value not found, handle gracefully */
  if (i == zoom->zoom_data.zoom_table_size)
    i = 0;

  /* define which zoom entry defines no zoom. */
  maximum_value = i;

  /* define which zoom entry defines no zoom. */
  minimum_value = 0;

  /* setup ui values */
  zoom->zoom_data.minimum_value = 0;

  zoom->zoom_data.maximum_value = (maximum_value - minimum_value);

  /* if we always want to have 10 zoom steps (a good feature until the UI can
   * handle press and hold zooming where say, 60 steps (4x), zooming does
   * not require 60 key presses, then lets do the following
   */
  if (zoom->zoom_data.maximum_value > (MAX_ZOOM_STEPS - 1)) {
    zoom->zoom_data.zoom_step_size =
      (zoom->zoom_data.maximum_value / (MAX_ZOOM_STEPS - 1));
    zoom->zoom_data.maximum_value = MAX_ZOOM_STEPS - 1;
  } else {
    zoom->zoom_data.zoom_step_size = 1;
  }

  /* If we have say computed 34 zoom steps, but we want to have
   * 10 steps in the UI, then we move 34/(10-1), or 3 steps in the zoom
   * table per click. However, that would take us to 3*9, 27, not 34.
   *
   * 0 3 6 9 12 15 18 21 24 27
   *
   * So we need to bump each value by bump number, specifically,
   *
   * 0 1 2 3  4  5  6  6  7  7
   *
   * which will give us
   *
   * 0 4 8 12 16 20 24 27 31 34
   *
   * This bit of code sets up that bump table */
  if ((maximum_value - minimum_value) > (MAX_ZOOM_STEPS - 1)) {
    int32_t num;

    num = ((maximum_value - minimum_value) % (MAX_ZOOM_STEPS - 1));
    for (i = 0; i < MAX_ZOOM_STEPS; i++) {
      zoom->zoom_data.zoom_table_bump[i] =
        ((i << 4) / (MAX_ZOOM_STEPS - 1)) * num;
      zoom->zoom_data.zoom_table_bump[i] >>= 4;
    }
  } else {
    for (i = 0; i < MAX_ZOOM_STEPS; i++)
      zoom->zoom_data.zoom_table_bump[i] = 0;
  }
  zoom->zoom_data.minimum_value = 0;
}

/** isp_zoom_set_scaling_single_fov:
 *
 *    @hw_zoom_parm:
 *
 **/
static int isp_zoom_set_scaling_single_fov(
  isp_zoom_session_t *session, isp_hw_zoom_param_t *hw_zoom_parm)
{
  isp_zoom_t *pzoom = session->pzoom;
  /* output2 is the main scaler, i.e. the encoder in single fov isp */
  uint32_t output1Height = hw_zoom_parm->entry[1].dim.height;
  uint32_t output1Width  = hw_zoom_parm->entry[1].dim.width;
  uint32_t output2Height = hw_zoom_parm->entry[0].dim.height;
  uint32_t output2Width  = hw_zoom_parm->entry[0].dim.width;
  uint32_t output2_aspect_ratio = 0;
  uint32_t input_aspect_ratio = 0;
  crop_window_info_t *crop_win = &hw_zoom_parm->entry[0].crop_win;
  isp_zoom_scaling_param_t *zoom_scaling_params = &session->zoom_scaling;

  zoom_scaling_params->num = ISP_ZOOM_MAX_ENTRY_NUM;
  isp_zoom_scaling_param_entry_t *output2 = &zoom_scaling_params->entry[0];
  isp_zoom_scaling_param_entry_t *output1 = &zoom_scaling_params->entry[1];

  output2->stream_id = hw_zoom_parm->entry[0].stream_id;
  output2->roi_map_info = hw_zoom_parm->entry[0].roi_map_info;
  output1->stream_id = hw_zoom_parm->entry[1].stream_id;
  output1->roi_map_info = hw_zoom_parm->entry[1].roi_map_info;

  if ((crop_win->crop_out_x < output2Width)||
      (crop_win->crop_out_y < output2Height)) {
    output2->in_height = output2Height;
    output2->out_height = output2Height;
    output1->in_height = output1Height;
    output1->out_height = output1Height;

    output2->in_width = output2Width;
    output2->out_width = output2Width;
    output1->in_width = output1Width;
    output1->out_width = output1Width;

    /* if fall below then crop.info is needed.  Otherwise VFE
       handles zoom itself. */
    if (crop_win->crop_out_x < output2Width)
       output2->in_width = crop_win->crop_out_x;

    output1->in_width =
      output2->in_width * output1Width / output2Width;
    if (crop_win->crop_out_y < output2Height ) {
      output2->in_height = crop_win->crop_out_y;
      output1->in_height =
        output2->in_height * output1Height / output2Height;
    }
    /* Now update the scaler based on the crop values */
    /* MDP requires that Width, Height, Width/2 and Height/2
     * are even numbers to avoid shaky zoom */
    output1->in_width  = CEILING4(output1->in_width);
    output1->in_height = CEILING4(output1->in_height);
    output2->in_width  = CEILING4(output2->in_width);
    output2->in_height = CEILING4(output2->in_height);
  } else {   /* no crop.info  */
    /* default to no crop.info always. */
    output2->in_height = 0;
    output2->out_height = 0;
    output1->in_height = 0;
    output1->out_height = 0;
    output2->in_width = 0;
    output2->out_width = 0;
    output1->in_width = 0;
    output1->out_width = 0;
  }

  return 0;
}

/** isp_zoom_scaling_double_fov_proc:
 *
 *    @pzoom:
 *    @session:
 *    @entry:
 *    @output:
 *
 **/
static int isp_zoom_scaling_double_fov_proc(isp_zoom_t *pzoom,
  isp_zoom_session_t *session, isp_hw_zoom_param_entry_t *entry,
  isp_zoom_scaling_param_entry_t *output)
{
  int rc = 0;
  uint32_t outputHeight = entry->dim.height;
  uint32_t outputWidth  = entry->dim.width;
  uint32_t output2_aspect_ratio = 0;
  uint32_t input_aspect_ratio = 0;
  crop_window_info_t *crop_win = &entry->crop_win;

  output->roi_map_info = entry->roi_map_info;
  output->stream_id = entry->stream_id;

  if (crop_win->crop_out_x == 0) {
    output->in_height = 0;
    output->out_height = 0;
    output->in_width = 0;
    output->out_width = 0;
  } else {
    output->in_height = outputHeight;  /* ISP out height */
    output->out_height = crop_win->crop_out_y; /* crop height */
    output->in_width = outputWidth;    /* isp out width */
    output->out_width = crop_win->crop_out_x; /* crop width */
    output->stream_id = entry->stream_id;
  }

  return 0;
}

/** isp_zoom_set_scaling_double_fov:
 *
 *    @session:
 *    @zoom_info:
 *
 **/
static int isp_zoom_set_scaling_double_fov(isp_zoom_session_t *session,
  isp_hw_zoom_param_t *zoom_info)
{
  int i, rc = 0;
  isp_zoom_t *pzoom = session->pzoom;
  isp_zoom_scaling_param_t *output = &session->zoom_scaling;

  output->num = 0;

  for (i = 0; i < zoom_info->num; i++) {
    rc = isp_zoom_scaling_double_fov_proc(pzoom,
    session, &zoom_info->entry[i], &output->entry[i]);

    if (rc) {
      CDBG_ERROR("%s: process entry %d error = %d\n", __func__, i, rc);
      return rc;
    }

    output->num++;
  }

  return rc;
}

/** isp_zoom_get_scaling_param:
 *
 *    @session:
 *    @scaling_param:
 *
 **/
int isp_zoom_get_scaling_param(isp_zoom_session_t *session,
  isp_zoom_scaling_param_t *scaling_param)
{
  *scaling_param = session->zoom_scaling;
  return 0;
}

/** isp_set_zoom_scaling_parm:
 *
 *    @session:
 *    @hw_zoom_parm:
 *
 **/
int isp_set_zoom_scaling_parm(isp_zoom_session_t *session,
  isp_hw_zoom_param_t *hw_zoom_parm)
{
  isp_zoom_t *pzoom = session->pzoom;

  ISP_DBG(ISP_MOD_COM,"%s: num_fov = %d\n", __func__, session->num_fovs);

  if (session->num_fovs == 1) {
    return isp_zoom_set_scaling_single_fov(session, hw_zoom_parm);
  } else if (session->num_fovs == 2) {
    return isp_zoom_set_scaling_double_fov(session, hw_zoom_parm);
  } else {
    CDBG_ERROR("%s: not supported\n", __func__);
    return -1;
  }
}

uint32_t isp_zoom_calc_dim(isp_zoom_session_t *session, uint32_t dim,
  uint32_t crop_factor)
{
  isp_zoom_t *pzoom = session->pzoom;
  return(dim * pzoom->zoom_data.zoom_table[0] / crop_factor);
}

/** isp_zoom_get_crop_factor:
 *
 *    @session:
 *    @zoom_val:
 *    @crop_factor:
 *
 **/
int isp_zoom_get_crop_factor(isp_zoom_session_t *session, int zoom_val,
  uint32_t *crop_factor)
{
  isp_zoom_t *pzoom = session->pzoom;
  int idx;

  session->zoom_val = zoom_val;
  idx = pzoom->zoom_data.zoom_step_size * session->zoom_val +
    pzoom->zoom_data.zoom_table_bump[session->zoom_val];

  /* This is the meat of the zoom algorithm. This is where we use a table
   * to determine how many times greater than the smallest crop window
   * (which provides maximum zoom) should we crop. If our crop is the
   * smallest crop permissible (low table entry), we get maximum zoom, if
   * we are high in the LUT, we are then a big multiple of min crop window,
   * i.e. big crop window, i.e. little zooming. */
  *crop_factor = pzoom->zoom_data.zoom_table[idx];

  return 0;
}

/** isp_zoom_open_session:
 *
 *    @pzoom:
 *    @session_id:
 *
 **/
isp_zoom_session_t *isp_zoom_open_session(isp_zoom_t *pzoom,
  uint32_t session_id)
{
  int i;

  /* here we assuem isp already locked the mutex
   * so that no mutex needed in zoom component */
  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (pzoom->sessions[i].pzoom == NULL) {
      memset(&pzoom->sessions[i], 0, sizeof(pzoom->sessions[i]));

      pzoom->sessions[i].session_id = session_id;
      pzoom->sessions[i].pzoom = pzoom;
      pzoom->sessions[i].num_fovs =
        isp_zoom_calc_num_fovs(pzoom->zoom_data.isp_version);

      /* 1X is the default */
      pzoom->sessions[i].zoom_val = pzoom->zoom_data.minimum_value;

      return &pzoom->sessions[i];
    }
  }

  CDBG_ERROR("%s: no zoom session available, error\n", __func__);

  return NULL;
}

/** isp_zoom_close_session:
 *
 *    @session:
 *
 **/
void isp_zoom_close_session(isp_zoom_session_t *session)
{
  if (session->pzoom == NULL) {
    CDBG_ERROR("%s: invalid session\n", __func__);
    return;
  }

  /* zero out old zoom sessin data */
  memset(session, 0, sizeof(*session));
}

/** isp_zoom_get_max_zoom:
 *
 *    @pzoom:
 *
 *    Get maximum zoom value
 *
 *    Return maximum zoom value
 **/
uint32_t isp_zoom_get_max_zoom(isp_zoom_t *pzoom)
{
  return pzoom->zoom_data.maximum_value;
}

/** isp_zoom_get_min_zoom:
 *
 *    @pzoom:
 *
 **/
uint32_t isp_zoom_get_min_zoom(isp_zoom_t *pzoom)
{
  return pzoom->zoom_data.minimum_value;
}

/** isp_zoom_get_ratio_table:
 *
 *    @pzoom:
 *    @num: zoom steps
 *    @buf: pointer to zoom table
 *
 **/
int isp_zoom_get_ratio_table(isp_zoom_t *pzoom, int32_t *num, int *buf)
{
  int i, rc = 0;
  int *pos = buf;

  if (*num > MAX_ZOOM_STEPS)
    *num = MAX_ZOOM_STEPS;

  for (i=0; i < *num; i++) {
    *pos = (pzoom->zoom_data.zoom_table[i]*100) /
      pzoom->zoom_data.zoom_table[0];

    pos++;
  }

  return 0;
}

/** isp_zoom_create:
 *
 *    @isp_version:
 *
 **/
isp_zoom_t *isp_zoom_create(uint32_t isp_version)
{
  isp_zoom_t *pzoom = malloc(sizeof(isp_zoom_t));

  if (!pzoom) {
    CDBG_ERROR("%s: cannot malloc for zoom struct\n", __func__);
    return NULL;
  }

  memset(pzoom, 0, sizeof(isp_zoom_t));
  pzoom->zoom_data.isp_version = isp_version;
  pzoom->zoom_data.zoom_table = (uint32_t *)&isp_zoom_table_def[0];
  pzoom->zoom_data.zoom_table_size = ZOOM_TABLE_MAX_DEF;
  pzoom->zoom_data.resize_factor = Q12*4;

  zoom_init(pzoom);

  return pzoom;
}

/** isp_zoom_destroy:
 *
 *    @pzoom:
 *
 **/
void isp_zoom_destroy(isp_zoom_t *pzoom)
{
  if (pzoom)
    free(pzoom);
}
