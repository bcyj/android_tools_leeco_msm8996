/*==============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
 *============================================================================*/
#include <sys/ioctl.h>
#include <stdint.h>
#include <string.h>
#include "camera_dbg.h"
#include "zoom.h"
#include "mctl.h"

#if 0
#undef CDBG
#define CDBG LOGE
#endif

static const uint32_t camera_zoom_table_def[ZOOM_TABLE_MAX_DEF] = {
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

void zoom_reset_scaler(void *scal)
{
  zoom_scaling_params_t *scaling = (zoom_scaling_params_t *) scal;

  /* update zoom scaling settings:
   * Reset scaler values */
  scaling->input1_width = 0;
  scaling->output1_width = 0;
  scaling->input1_height = 0;
  scaling->output1_height = 0;
  scaling->input2_width = 0;
  scaling->output2_width = 0;
  scaling->input2_height = 0;
  scaling->output2_height = 0;
}

/*===========================================================================
FUNCTION      zoom_calculate

DESCRIPTION
===========================================================================*/
static int zoom_get_crop_factor(
  zoom_ctrl_t *pme,
  int32_t current_value,
  uint32_t *crop_factor)
{
  /* This is the meat of the zoom algorithm. This is where we use a table
   * to determine how many times greater than the smallest crop window
   * (which provides maximum zoom) should we crop. If our crop is the
   * smallest crop permissible (low table entry), we get maximum zoom, if
   * we are high in the LUT, we are then a big multiple of min crop window,
   * i.e. big crop window, i.e. little zooming. */
  pme->cam_parm_zoom.current_value = current_value;
  *crop_factor =
    pme->zoomInfo.zoomTable[pme->zoom_step_size *
    pme->cam_parm_zoom.current_value +
    pme->zoomInfo.zoomTableBump[pme->cam_parm_zoom.current_value]];
    CDBG("%s: crop_factor = %d", __func__, *crop_factor);
  return 0;
}

static void zoom_calc_resize_factor(zoom_ctrl_t *zctrl)
{
  zctrl->resize_factor = Q12*4;
}

/*===========================================================================
FUNCTION        zoom_init

DESCRIPTION
===========================================================================*/
static void zoom_init(zoom_ctrl_t *zoom)
{
  int i;
  int32_t minimum_value; /* Minimum allowed value */
  int32_t maximum_value; /* Maximum allowed value */
  int32_t step_value;    /* step value */
  int32_t default_value; /* Default value */
  int32_t current_value; /* Current value */

  minimum_value = 0;
  maximum_value = 0;
  default_value = 0;
  current_value = 0;
  step_value = 1;

  /* Compute min and max zoom values:
   * now that we know the resize factor, figure out which entry in our zoom
   * table equals this zoom factor. This entry actually will correspond to
   * NO zoom, since it will tell the VFE to crop the entire image. Entry
   * min_decimation (often 0) in the table, is the smallest amount we
   * can crop, which is max zoom. */
  for (i = 0; i < zoom->zoomInfo.zoom_table_size; i++)
    if (zoom->resize_factor < zoom->zoomInfo.zoomTable[i + 1])
      break;

  /* if value not found, handle gracefully */
  if (i == zoom->zoomInfo.zoom_table_size)
    i = 0;

  /* define which zoom entry defines no zoom. */
  maximum_value = i;
  CDBG("%s: maximum_value=%d", __func__, i);

  /* define which zoom entry defines no zoom. */
  minimum_value = 0;

  /* setup ui values */
  zoom->cam_parm_zoom.minimum_value = 0;

  zoom->cam_parm_zoom.maximum_value =
    (maximum_value - minimum_value);

  /* if we always want to have 10 zoom steps (a good feature until the UI can
   * handle press and hold zooming where say, 60 steps (4x), zooming does
   * not require 60 key presses, then lets do the following
   */
  if (zoom->cam_parm_zoom.maximum_value > (MAX_ZOOM_STEPS - 1)) {
    zoom->zoom_step_size =
      (zoom->cam_parm_zoom.maximum_value / (MAX_ZOOM_STEPS - 1));

    zoom->cam_parm_zoom.maximum_value = MAX_ZOOM_STEPS - 1;
  } else
    zoom->zoom_step_size = 1;

  zoom->cam_parm_zoom.current_value = zoom->cam_parm_zoom.minimum_value;
  zoom->cam_parm_zoom.default_value = zoom->cam_parm_zoom.current_value;
  zoom->cam_parm_zoom.step_value = 1;

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

      zoom->zoomInfo.zoomTableBump[i] = ((i << 4) / (MAX_ZOOM_STEPS - 1)) * num;
      zoom->zoomInfo.zoomTableBump[i] >>= 4;
    }
  } else
    for (i = 0; i < MAX_ZOOM_STEPS; i++)
      zoom->zoomInfo.zoomTableBump[i] = 0;

  zoom->cam_parm_zoom.minimum_value = 0;

  zoom->cam_parm_zoom.current_value = zoom->cam_parm_zoom.minimum_value;
  zoom->cam_parm_zoom.default_value = zoom->cam_parm_zoom.current_value;

  zoom->cam_parm_zoom.step_value = 1;
  zoom->cam_parm_zoom.current_value = -1;
  CDBG("after zoom_init, cam_parm_zoom.maximum_value = %d\n",
    zoom->cam_parm_zoom.maximum_value);
}

/*===========================================================================
FUNCTION      zoom_init_ctrl

DESCRIPTION
===========================================================================*/
int zoom_init_ctrl(zoom_ctrl_t *zctrl)
{
  int rc = 0;

  if (!zctrl) {
    CDBG_ERROR("%s: Invalid input", __func__);
    return -EINVAL;
  }

  zctrl->cam_parm_zoom.minimum_value = 0;
  zctrl->cam_parm_zoom.maximum_value = 0;
  zctrl->cam_parm_zoom.default_value = 0;
  zctrl->cam_parm_zoom.current_value = 0;
  zctrl->cam_parm_zoom.step_value = 1;
  if(zctrl->zoomInfo.ext_zoom_table) {
	/* if plugin push zoom table to us */
    zctrl->zoomInfo.zoomTable = zctrl->zoomInfo.ext_zoom_table;
  } else {
	zctrl->zoomInfo.zoomTable = (uint32_t *)&camera_zoom_table_def[0];
	zctrl->zoomInfo.zoom_table_size = ZOOM_TABLE_MAX_DEF;
  }
  zoom_calc_resize_factor(zctrl);
  zoom_init(zctrl);

  return rc;
}

/*===========================================================================
 * FUNCTION    - zoom_vfe -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int zoom_get_scaling_2d(
  zoom_ctrl_t *zctrl,
  vfe_zoom_crop_info_t *crop_info,
  zoom_scaling_params_t *output)
{
  uint32_t output1Height = crop_info->output1h;
  uint32_t output1Width  = crop_info->output1w;
  uint32_t output2Height = crop_info->output2h;
  uint32_t output2Width  = crop_info->output2w;
  uint32_t output2_aspect_ratio = 0;
  uint32_t input_aspect_ratio = 0;
  output->update_flag = 1;
   /* if any dimension needs crop.info  then first init them to
      actual output size, this is to ensure no entries are zero
      to begin with. */
  CDBG("%s: x= %d, y = %d, width = %d, height = %d",
    __func__, crop_info->crop_win.x, crop_info->crop_win.y,
    crop_info->crop_win.crop_out_x, crop_info->crop_win.crop_out_y);
  if ((crop_info->crop_win.crop_out_x < output2Width)||
    (crop_info->crop_win.crop_out_y < output2Height)) {
    output->input2_height = output2Height;
    output->output2_height = output2Height;
    output->input1_height = output1Height;
    output->output1_height = output1Height;

    output->input2_width = output2Width;
    output->output2_width = output2Width;
    output->input1_width = output1Width;
    output->output1_width = output1Width;

    /* if fall below then crop.info is needed.  Otherwise VFE
       handles zoom itself. */
    if (crop_info->crop_win.crop_out_x < output2Width)
       output->input2_width = crop_info->crop_win.crop_out_x;

    //if (zctrl->zoomscaling.input2_width < (output2Width/4))
    //  zctrl->zoomscaling.input2_width = (output2Width/4);

    output->input1_width =
    output->input2_width * output1Width / output2Width;
    if (crop_info->crop_win.crop_out_y < output2Height ) {
      output->input2_height = crop_info->crop_win.crop_out_y;
      output->input1_height =
      output->input2_height * output1Height / output2Height;
    }
    /* Now update the scaler based on the crop values */
    if (output->update_flag) {
      /* MDP requires that Width, Height, Width/2 and Height/2
       * are even numbers to avoid shaky zoom */
      output->input1_width  =
        CEILING4(output->input1_width);
      output->input1_height =
        CEILING4(output->input1_height);
      output->input2_width  =
        CEILING4(output->input2_width);
      output->input2_height =
        CEILING4(output->input2_height);
      CDBG("zoomscaling: input1_width=%d, input1_height=%d,"
        "output1_width=%d, output1_height=%d\n",
        output->input1_width, output->input1_height,
        output->output1_width, output->output1_height);
      CDBG("zoomscaling: input2_width=%d, input2_height=%d,"
        "output2_width=%d, output2_height=%d\n",
        output->input2_width, output->input2_height,
        output->output2_width, output->output2_height);
    }
  } else {   /* no crop.info  */
    /* default to no crop.info always. */
    output->input2_height = 0;
    output->output2_height = 0;
    output->input1_height = 0;
    output->output1_height = 0;
    output->input2_width = 0;
    output->output2_width = 0;
    output->input1_width = 0;
    output->output1_width = 0;
  }
  return 0;
}

/*===========================================================================
 * FUNCTION    - zoom_proc -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int zoom_proc(zoom_ctrl_t *pme, vfe_zoom_crop_info_t *crop_info, zoom_scaling_params_t *output)
{
  int rc = 0;

  memset(output, 0, sizeof(zoom_scaling_params_t));
  rc = zoom_get_scaling_2d(pme, crop_info, output);
  pme->zoomscaling = *output;
  return rc;
}

/*===========================================================================
FUNCTION      zoom_calc_ratios

DESCRIPTION
===========================================================================*/
int zoom_calc_ratios(zoom_ctrl_t *pme, void *ctrlCmdBlk)
{
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)ctrlCmdBlk;

  int16_t usr_max_zoom = (int16_t) (ctrlCmd->length / sizeof(int16_t));
  int16_t *value = (int16_t *)ctrlCmd->value;

  CDBG("usr_max_zoom = %d", usr_max_zoom);

  if (usr_max_zoom > MAX_ZOOM_STEPS) {
    CDBG_HIGH("%s failed: number of zoom steps %d are more "
      "than driver can support\n", __func__, usr_max_zoom);
    return -1;
  }

  int16_t i;
  for (i=0; i<usr_max_zoom; i++) {
    *value = (pme->zoomInfo.zoomTable[i] * 100) / pme->zoomInfo.zoomTable[0];

    CDBG("ZoomRatio[%d] = %d", i, *value);
    value++;
  }

  return 0;
}

int zoom_process(zoom_ctrl_t *pme, zoom_process_cmd_t type, void *parm)
{
  int rc = 0;

  switch(type) {
  case ZOOM_PROC_CMD_ZOOM_RATIOS:
    rc =  zoom_calc_ratios(pme, parm);
    break;
  default:
    rc = -1;
    break;
  }
  return rc;
}

int zoom_get_parms(zoom_ctrl_t *pme, zoom_parm_type_t parm_type,
  void *parm_in, void* parm_out)
{
  int rc = -1;
  switch(parm_type) {
  case ZOOM_PARM_GET_CROP_FACTOR: {
    int32_t zoom_value = *((int32_t *)parm_in);
    uint32_t *crop_factor = (uint32_t *)parm_out;
    rc = zoom_get_crop_factor(pme, zoom_value, crop_factor);
    break;
  }
  case ZOOM_PARM_GET_SCALING_INFO: {
    vfe_zoom_crop_info_t *crop_info = (vfe_zoom_crop_info_t *)parm_in;
    zoom_scaling_params_t *output = (zoom_scaling_params_t *)parm_out;
    rc = zoom_proc(pme, crop_info, output);
    rc = 0;
    break;
  }
  default:
    break;
  }
  return rc;
}

int zoom_ctrl_save_plugin_table(zoom_ctrl_t *pme, int num_entries, uint32_t *zoom_table_ptr)
{
  pme->zoomInfo.ext_zoom_table = (uint32_t *)malloc(num_entries*sizeof(uint32_t));
  if (!pme->zoomInfo.ext_zoom_table) {
	CDBG_ERROR("%s: cannot alloc mem for external zoom table",  __func__);
	return -ENOMEM;
  }
  memcpy(pme->zoomInfo.ext_zoom_table, zoom_table_ptr, num_entries*sizeof(uint32_t));
  pme->zoomInfo.zoom_table_size = num_entries;
  return 0;
}
void zoom_ctrl_deinit(zoom_ctrl_t *pme)
{
  if(pme->zoomInfo.ext_zoom_table) {
	free (pme->zoomInfo.ext_zoom_table);
	pme->zoomInfo.ext_zoom_table = NULL;
  }
  pme->zoomInfo.zoom_table_size = 0;
}

