/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "vfe_util_common.h"
#include "camera_dbg.h"

#ifdef ENABLE_VFE_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    -  vfe_util_calculate_ceil_log_2 -
 *
 * DESCRIPTION: calculate the y = ceil(log2(x)),
 * if x == 0, then y =0;
 * if 2^n <= x < 2^(n+1), then y= n+1;
 * .
 *==========================================================================*/
static uint32_t vfe_util_calculate_ceil_log_2(uint32_t pixels_in_ae_rgn)
{
  uint32_t val = 0;
  while (pixels_in_ae_rgn) {
    val++;
    pixels_in_ae_rgn = pixels_in_ae_rgn >>1;
  }
  return val;
} /* vfe_util_calculate_ceil_log_2 */

/*===========================================================================
FUNCTION      vfe_util_write_hw_cmd

DESCRIPTION
===========================================================================*/
vfe_status_t vfe_util_write_hw_cmd(int fd, int type, void *pCmdData,
  unsigned int messageSize, int cmd_id)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd cfgCmd;
  vfe_hw_cmd_t cmd;

  cmd.id = cmd_id;
  cmd.length = messageSize;
  cmd.value = pCmdData;

  cfgCmd.cmd_type = type;
  cfgCmd.length = sizeof(vfe_hw_cmd_t);
  cfgCmd.value = &cmd;

  if ((rc = ioctl(fd, MSM_CAM_IOCTL_CONFIG_VFE, &cfgCmd)) < 0) {
    CDBG_ERROR("%s: MSM_CAM_IOCTL_CONFIG_VFE failed...%d %s\n", __func__, rc,
      strerror(errno));
    return VFE_ERROR_GENERAL;
  }

  CDBG("%s: type = %d, Cmd = %d, length = %d\n",
    __func__, cfgCmd.cmd_type, cmd_id, messageSize);

  return VFE_SUCCESS;
} /* vfe_util_write_hw_cmd */

/*===========================================================================
 * FUNCTION    - vfe_util_get_awb_cct_type -
 *
 * DESCRIPTION:
 *==========================================================================*/
awb_cct_type vfe_util_get_awb_cct_type(cct_trigger_info* trigger,
  vfe_params_t* parms)
{
  chromatix_parms_type *p_chromatix = parms->chroma3a;
  awb_cct_type cct_type = AWB_CCT_TYPE_TL84;

  CDBG("%s: CCT %f D65 %f %f A %f %f", __func__,
    trigger->mired_color_temp,
    trigger->trigger_d65.mired_end,
    trigger->trigger_d65.mired_start,
    trigger->trigger_A.mired_start,
    trigger->trigger_A.mired_end);
  if (trigger->mired_color_temp <= trigger->trigger_d65.mired_end)
    cct_type = AWB_CCT_TYPE_D65;
  else if ((trigger->mired_color_temp > trigger->trigger_d65.mired_end)
   && (trigger->mired_color_temp <= trigger->trigger_d65.mired_start))
    cct_type = AWB_CCT_TYPE_D65_TL84;
  else if ((trigger->mired_color_temp >= trigger->trigger_A.mired_start)
   && (trigger->mired_color_temp < trigger->trigger_A.mired_end))
    cct_type = AWB_CCT_TYPE_TL84_A;
  else if (trigger->mired_color_temp >= trigger->trigger_A.mired_end)
    cct_type = AWB_CCT_TYPE_A;
  /* else its TL84*/
  return cct_type;
} /*vfe_util_get_awb_cct_type*/


/*===========================================================================
 * FUNCTION    - vfe_util_aec_check_settled -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t vfe_util_aec_check_settled (vfe_aec_parms_t* aec_params)
{
#ifdef FEATURE_VFE_TEST_VECTOR
  return 1;
#endif
  /* check whether aec is settled or not */
  CDBG("%s: cur %d tar %d exp %d exp_val %d", __func__,
    aec_params->cur_luma, aec_params->target_luma,
    aec_params->exp_index, aec_params->exp_tbl_val);
  if ((aec_params->cur_luma >= (0.8f * aec_params->target_luma)
    && aec_params->cur_luma <= (1.2f * aec_params->target_luma))
    || (aec_params->exp_index >= aec_params->exp_tbl_val - 1))
    return 1;
  else
    return 0;
} /* vfe_trigger_aec_check_settled */

/*===========================================================================
 * FUNCTION    - vfe_util_calc_interpolation_weight -
 *
 * DESCRIPTION:
 *==========================================================================*/
float vfe_util_calc_interpolation_weight(float value,
  float start, float end)
{
  if (start != end) {
    if (value  <= start)
      return 0.0;
    else if (value  >= end)
      return 1.0;
    else
      return(value  - start) / (end - start);
  } else {
    CDBG("Trigger Warning: same value %f\n", start);
    return 0.0;
  }
} /* vfe_trigger_calc_interpolation_weight */

/*===========================================================================
 * FUNCTION    - vfe_util_get_aec_ratio2 -
 *
 * DESCRIPTION:  Get trigger ratio based on outdoor trigger & lowlight
 *    trigger Please note that rt.ratio means the weight of the normal light.
 *==========================================================================*/
trigger_ratio_t vfe_util_get_aec_ratio2(tuning_control_type tuning,
  trigger_point_type *outdoor_trigger, trigger_point_type *lowlight_trigger,
  vfe_params_t* parms)
{
  float real_gain;
  trigger_ratio_t rt;
  vfe_aec_parms_t* aec_out = &(parms->aec_params);
  int8_t is_snap_mode = IS_SNAP_MODE(parms);

  rt.ratio = 0.0;
  rt.lighting = TRIGGER_NORMAL;

  CDBG("lux_idx %f, current_real_gain %f\n",
    aec_out->lux_idx, aec_out->cur_real_gain);

  switch (tuning) {
    case 0: /* 0 is Lux Index based */
      if (aec_out->lux_idx < outdoor_trigger->lux_index_start) {
        rt.ratio = vfe_util_calc_interpolation_weight(
          aec_out->lux_idx, outdoor_trigger->lux_index_end,
          outdoor_trigger->lux_index_start);
        rt.lighting = TRIGGER_OUTDOOR;
      } else if (aec_out->lux_idx > lowlight_trigger->lux_index_start) {
        rt.ratio = 1.0 - vfe_util_calc_interpolation_weight(
          aec_out->lux_idx, lowlight_trigger->lux_index_start,
          lowlight_trigger->lux_index_end);
        rt.lighting = TRIGGER_LOWLIGHT;
      } else {
        rt.ratio = 1.0;
        rt.lighting = TRIGGER_NORMAL;
      }
      break;

    case 1: /* 1 is Gain Based */
      if (is_snap_mode)
        real_gain = aec_out->snapshot_real_gain;
      else
        real_gain = aec_out->cur_real_gain;

      if (real_gain < outdoor_trigger->gain_start) {
        rt.ratio = vfe_util_calc_interpolation_weight(real_gain,
          outdoor_trigger->gain_end, outdoor_trigger->gain_start);
        rt.lighting = TRIGGER_OUTDOOR;
      } else if (real_gain > lowlight_trigger->gain_start) {
        rt.ratio = 1.0 - vfe_util_calc_interpolation_weight(real_gain,
          lowlight_trigger->gain_start, lowlight_trigger->gain_end);
        rt.lighting = TRIGGER_LOWLIGHT;
      } else {
        rt.ratio = 1.0;
        rt.lighting = TRIGGER_NORMAL;
      }
      break;

    default:
      CDBG_ERROR("get_trigger_ratio: tunning type %d is not supported.\n",
        tuning);
      break;
  }
  return rt;
} /* vfe_util_get_ratio2 */

/*===========================================================================
 * FUNCTION    - vfe_util_get_aec_ratio -
 *
 * DESCRIPTION:  get trigger ratio based on lowlight trigger.
 *               Please note that ratio is the weight of the normal light.
 *==========================================================================*/
float vfe_util_get_aec_ratio(tuning_control_type tunning,
  trigger_point_type *trigger, vfe_params_t* parms)
{
  float ratio = 0.0, real_gain;
  vfe_aec_parms_t* aec_out = &(parms->aec_params);
  int8_t is_snap_mode = IS_SNAP_MODE(parms);

  switch (tunning) {
    case 0: /* 0 is Lux Index based */
      ratio = 1.0 - vfe_util_calc_interpolation_weight(
        aec_out->lux_idx,
        trigger->lux_index_start, trigger->lux_index_end);
      break;

    case 1:  /* 1 is Gain Based */
      if (is_snap_mode)
        real_gain = aec_out->snapshot_real_gain;
      else
        real_gain = aec_out->cur_real_gain;

      ratio = 1.0 - vfe_util_calc_interpolation_weight(real_gain,
        trigger->gain_start, trigger->gain_end);
      break;

    default:
      CDBG_ERROR("get_trigger_ratio: tunning type %d is not supported.\n",
        tunning);
      break;
  }
  if (ratio < 0)
    ratio = 0;
  else if (ratio > 1.0) {
    ratio = 1.0;
  }
  return ratio;
} /* vfe_util_get_aec_ratio */

/*===========================================================================
 * FUNCTION    -  vfe_util_calculate_shift_bits -
 *
 * DESCRIPTION: Calculate the shift bits.
 *   AE_SHIFT_BITS = ceil(log2[(ae_rgn_width*ae_rgn_height)<<8])-16
 *     = ceil(log2[ae_rgn_width*ae_rgn_height]-8);
 *  if (log2[ae_rgn_width*ae_rgn_height] ==
 *    ceil(log2[ae_rgn_width*ae_rgn_height])) AE_SHIFT_BITS++;
 *   if (AE_SHIFT_BITS < 0) AE_SHIFT_BITS = 0;
 *==========================================================================*/
uint32_t vfe_util_calculate_shift_bits(uint32_t pixels_in_ae_rgn)
{
  uint32_t log2_val;
  uint32_t shift_bits;

  log2_val = vfe_util_calculate_ceil_log_2(pixels_in_ae_rgn);
  if (log2_val > 8) {
    shift_bits = log2_val - 8;
  } else {
    shift_bits = 0;
  }
  return shift_bits;
} /* vfe_util_calculate_shift_bits */
