/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#include <linux/media.h>
#include "mct_module.h"
#include "module_cac.h"
#include "mct_stream.h"
#include "pthread.h"
#include "chromatix.h"

//#define USE_CHROMATIX
//#define USE_RNR_HYSTERISIS

/**
 * Function: module_cac2_config_get_rnr_scaling_factor
 *
 * Description: This function calculates the scaling factor for
 * the current resolution wrt the max resolution (camif o/p)
 *
 * Arguments:
 *   @p_client: cac client
 *
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static float module_cac2_config_get_rnr_scaling_factor(cac_client_t *p_client)
{
  float center_x, center_y, max_radius, current_radius;
  float default_scale_factor, new_scale_factor;
  uint32_t max_width, max_height, current_width, current_height;

  // CAMIF output window size - 4208x3120
  max_width=4208; max_height=3120;
  default_scale_factor = 16.0f;
  center_x = (max_width-1)*0.5f;
  center_y = (max_height-1)*0.5f;
   max_radius = (float)p_client->cac2_cfg_info.rnr_chromatix_info.lut_size - 1;

  // Current snapshot resolution
  current_width  = p_client->frame[0].info.width;
  current_height = p_client->frame[0].info.height;
  center_x = (current_width-1)* 0.5f;
  center_y = (current_height-1)* 0.5f;
  current_radius = (float)(sqrt( center_x * center_x + center_y * center_y )) /
    default_scale_factor;
  new_scale_factor = default_scale_factor * current_radius / max_radius;

  return new_scale_factor;
}

/* Function: module_cac2_config_rnr_hysterisis
 *
 * Description: Helper function for rnr hysterisis
 *
 * Arguments:
 *   @trigger_pt_values: array of 4 tigger pt values
 *...@rnr_hysterisis_info hysterisis info for RNR
 *   @trend: hysterisis trend
 *   @trigger: can be gain or lux value
 *
 * Return values:
 *     Sampling factor
 *
 * Notes: none
 **/
static int module_cac2_config_rnr_hysterisis(float *trigger_pt_values,
  hysterisis_info_t *rnr_hysterisis_info, float trigger, hysterisis_trend trend)
{

  int sampling_factor = 2;

  if (!trigger_pt_values) {
    IDBG_ERROR("%s %d: Null parameter for trigger_pt_values",
      __func__, __LINE__);
    return 0;
  }
  float trigger_ptA = trigger_pt_values[0];
  float trigger_ptB = trigger_pt_values[1];
  float trigger_ptC = trigger_pt_values[2];
  float trigger_ptD = trigger_pt_values[3];

  IDBG_MED("%s %d: trigger %f TpointA %f, TpointB %f, Tpointc %f, TpointD %f",
    __func__, __LINE__, trigger, trigger_ptA, trigger_ptB, trigger_ptC,
    trigger_ptD);

  if(trigger < trigger_ptA) {
   sampling_factor = 2;
   rnr_hysterisis_info->norml_hyst_enabled = FALSE;
   rnr_hysterisis_info->lowl_hyst_enabled = FALSE;
   IDBG_MED("%s %d: trigger < trigger_ptA, sampling Factor = %d",
     __func__,__LINE__, sampling_factor);
  } else if (trigger > trigger_ptD) {
    sampling_factor = 8;
    rnr_hysterisis_info->norml_hyst_enabled = FALSE;
    rnr_hysterisis_info->lowl_hyst_enabled = FALSE;
    IDBG_MED("%s %d: trigger > trigger_ptD, sampling Factor = %d",
      __func__, __LINE__, sampling_factor);
  } else if ((trigger > trigger_ptB) && (trigger < trigger_ptC)) {
    sampling_factor = 4;
    rnr_hysterisis_info->norml_hyst_enabled = FALSE;
    rnr_hysterisis_info->lowl_hyst_enabled = FALSE;
    IDBG_MED("%s %d: trigger > trigger_ptB && trigger < trigger_ptC,"
      "sampling Factor = %d", __func__, __LINE__, sampling_factor);
  } else {
    if(trigger >= trigger_ptA && trigger <= trigger_ptB) {
      if(rnr_hysterisis_info->norml_hyst_enabled) {
        if((trend == HYSTERISIS_TREND_DOWNWARD) ||
          (trend == HYSTERISIS_TREND_UPWARD)) {
          sampling_factor = rnr_hysterisis_info->prev_sampling_factor;
          IDBG_MED("%s %d: Normal light Hysterisis, trend %d"
            "sampling Factor = %d", __func__, __LINE__, trend, sampling_factor);
        }
      } else {
        switch(trend) {
        case HYSTERISIS_TREND_UPWARD:
          sampling_factor = 2;
          break;
        case HYSTERISIS_TREND_DOWNWARD:
          sampling_factor = 4;
          break;
        case HYSTERISIS_TREND_NONE:
        default:
          IDBG_ERROR("%s %d: trend NONE Invalid case,default sampling factor 2",
           __func__, __LINE__);
        break;
       }
       rnr_hysterisis_info->norml_hyst_enabled = TRUE;
       rnr_hysterisis_info->lowl_hyst_enabled = FALSE;
       IDBG_MED("%s %d: trend %d Normal Light Hysterisis enabled,"
         "sampling Factor = %d", __func__, __LINE__, trend, sampling_factor);
      }
    } else if(trigger >= trigger_ptC && trigger <= trigger_ptD) {
      if(rnr_hysterisis_info->lowl_hyst_enabled) {
        if((trend == HYSTERISIS_TREND_DOWNWARD) ||
          (trend == HYSTERISIS_TREND_UPWARD)) {
          sampling_factor = rnr_hysterisis_info->prev_sampling_factor;
          IDBG_MED("%s %d: Low light Hysterisis, trend %d sampling Factor = %d",
            __func__, __LINE__, trend, sampling_factor);
        }
      } else {
        switch(trend) {
        case HYSTERISIS_TREND_UPWARD:
          sampling_factor = 4;
          break;
        case HYSTERISIS_TREND_DOWNWARD:
          sampling_factor = 8;
          break;
        case HYSTERISIS_TREND_NONE:
        default:
          IDBG_ERROR("%s %d: trend None Invalid case, default sampling factor 2",
           __func__, __LINE__);
        break;
        }
        rnr_hysterisis_info->lowl_hyst_enabled = TRUE;
        rnr_hysterisis_info->norml_hyst_enabled = FALSE;

        IDBG_MED("%s %d: trend %d lowlHysterisis enabled,sampling Factor = %d",
          __func__, __LINE__, trend, sampling_factor);
      }
    }
  }
  return sampling_factor;
}
/**
 * Function: module_cac2_config_get_sampling_factor
 *
 * Description: This function calculates the sampling factor using hysterisis
 *
 * Arguments:
 *   @p_client: CAC Client
 *   @chromatix_rnr: RNR structure from chromatix
 *   @trigger: current trigger -can be gain or lux value
 *
 * Return values:
 * None
 *
 * Notes: none
 **/
#ifdef USE_RNR_HYSTERISIS
static void module_cac2_config_get_sampling_factor(cac_client_t *p_client,
  chromatix_RNR_type *chromatix_rnr, float trigger)
{
  float hyst_trigger_pt_values[4];

  if(chromatix_rnr->control_RNR  == 0) { //lux based
    //If LUX did not change, use prev sampling factor
    if(trigger == p_client->cac2_cfg_info.rnr_hysterisis_info.prev_lux_value) {
      p_client->cac2_cfg_info.rnr_chromatix_info.sampling_factor =
      p_client->cac2_cfg_info.rnr_hysterisis_info.prev_sampling_factor;
    } else {
      hyst_trigger_pt_values[0] =
        chromatix_rnr->normallight_hysteresis_point.lux_index_A;
      hyst_trigger_pt_values[1] =
        chromatix_rnr->normallight_hysteresis_point.lux_index_B;
      hyst_trigger_pt_values[2] =
        chromatix_rnr->lowlight_hysteresis_point.lux_index_A;
      hyst_trigger_pt_values[3] =
        chromatix_rnr->lowlight_hysteresis_point.lux_index_B;

      if(trigger > p_client->cac2_cfg_info.rnr_hysterisis_info.prev_lux_value) {
        p_client->cac2_cfg_info.rnr_hysterisis_info.lux_trend =
          HYSTERISIS_TREND_UPWARD;
      } else {
        p_client->cac2_cfg_info.rnr_hysterisis_info.lux_trend =
          HYSTERISIS_TREND_DOWNWARD;
      }
      p_client->cac2_cfg_info.rnr_chromatix_info.sampling_factor =
        module_cac2_config_rnr_hysterisis(hyst_trigger_pt_values,
        &p_client->cac2_cfg_info.rnr_hysterisis_info,trigger,
        p_client->cac2_cfg_info.rnr_hysterisis_info.lux_trend);
    }
  } else {//gain based
    //If gain did not change, use prev sampling factor
    if(trigger == p_client->cac2_cfg_info.rnr_hysterisis_info.prev_gain_value) {
      p_client->cac2_cfg_info.rnr_chromatix_info.sampling_factor =
      p_client->cac2_cfg_info.rnr_hysterisis_info.prev_sampling_factor;
    } else {
      hyst_trigger_pt_values[0] =
        chromatix_rnr->normallight_hysteresis_point.gain_A;
      hyst_trigger_pt_values[1] =
        chromatix_rnr->normallight_hysteresis_point.gain_B;
      hyst_trigger_pt_values[2] =
        chromatix_rnr->lowlight_hysteresis_point.gain_A;
      hyst_trigger_pt_values[3] =
        chromatix_rnr->lowlight_hysteresis_point.gain_B;

      if(trigger > p_client->cac2_cfg_info.rnr_hysterisis_info.prev_gain_value) {
        p_client->cac2_cfg_info.rnr_hysterisis_info.gain_trend =
          HYSTERISIS_TREND_UPWARD;
      } else {
        p_client->cac2_cfg_info.rnr_hysterisis_info.gain_trend =
          HYSTERISIS_TREND_DOWNWARD;
      }
      p_client->cac2_cfg_info.rnr_chromatix_info.sampling_factor =
        module_cac2_config_rnr_hysterisis(hyst_trigger_pt_values,
        &p_client->cac2_cfg_info.rnr_hysterisis_info, trigger,
        p_client->cac2_cfg_info.rnr_hysterisis_info.gain_trend);
      }
    }
  return;
}
#endif
/**
 * Function: module_cac2_config_get_rnr_params
 *
 * Description: This function is to update the RNR parameters
 * for offline usecase for RNR from chromatix header
 *
 * Arguments:
 *   @p_client: cac client
 *   @chromatix: chromatix pointer
 *   @native_buf: flag to indicate if its a native buffer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_cac2_config_get_rnr_params(cac_client_t *p_client,
  chromatix_parms_type *chromatix, float lux_idx, float gain)
{
  int rc = IMG_SUCCESS;
  int i =0;

#ifdef USE_CHROMATIX
  chromatix_RNR_type *chromatix_rnr;
  int regionStart = 0 , regionEnd = 0;
  float  trigger, interp_ratio = 0;
  float trigger_start[MAX_LIGHT_TYPES_FOR_SPATIAL];
  float trigger_end[MAX_LIGHT_TYPES_FOR_SPATIAL];

  //Set RnR params
  chromatix_rnr =
    &(chromatix->chromatix_post_processing.chromatix_radial_noise_reduction);
  //RNR enabled or disabled
  p_client->cac2_cfg_info.rnr_enable_flag = chromatix_rnr->rnr_en;
  trigger = (chromatix_rnr->control_RNR == 0)? (float)lux_idx : (float)gain;
  IDBG_HIGH("%s: trigger = %f, rnr_enable_flag = %d\n", __func__, trigger,
    p_client->cac2_cfg_info.rnr_enable_flag);
  for ( i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i ++ )
  {
    if (chromatix_rnr->control_RNR  == 0 ) { //lux index
      trigger_start[i] =
        chromatix_rnr->rnr_data[i].rnr_trigger.lux_index_start;
      trigger_end[i] =
        chromatix_rnr->rnr_data[i].rnr_trigger.lux_index_end;
    } else {
      trigger_start[i] = chromatix_rnr->rnr_data[i].rnr_trigger.gain_start ;
      trigger_end[i] = chromatix_rnr->rnr_data[i].rnr_trigger.gain_end;
    }
  }

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
    if ((trigger_start[i]>=trigger_end[i])) {
      IDBG_ERROR("%s %d: strigger start %f is <= to trigger end %f", __func__,
        __LINE__, trigger_start[i], trigger_end[i]);
    }
    if ( i == ( MAX_LIGHT_TYPES_FOR_SPATIAL - 1 )) {
      regionStart = MAX_LIGHT_TYPES_FOR_SPATIAL-1 ;
      regionEnd = MAX_LIGHT_TYPES_FOR_SPATIAL-1 ;
      break;
    }
    if (trigger <= trigger_start[i]) {
      regionStart = i;
      regionEnd = i;
      interp_ratio = 0.0;
      break;
    } else if (trigger < trigger_end[i] ) {
      regionStart = i;
      regionEnd = i+1;
      interp_ratio = (float) (trigger - trigger_start[i])/
        (float)(trigger_end[i] - trigger_start[i]);
      break;
    }
  }
  if (interp_ratio < 1.0) {
    IDBG_MED("%s %d: Interpolation ratio %f < 1.0", __func__, __LINE__,
      interp_ratio);
  }
  IDBG_MED("%s %d: regionStart %d, regionEnd %d", __func__, __LINE__,
    regionStart, regionEnd);

  p_client->cac2_cfg_info.rnr_chromatix_info.lut_size = chromatix_rnr->lut_size;

  p_client->cac2_cfg_info.rnr_chromatix_info.scale_factor =
    module_cac2_config_get_rnr_scaling_factor(p_client);
  IDBG_MED("%s :scaling factor %f", __func__,
    p_client->cac2_cfg_info.rnr_chromatix_info.scale_factor);

  for (i =0; i < chromatix_rnr->lut_size; i++) {
    p_client->cac2_cfg_info.rnr_chromatix_info.sigma_lut[i] =
      chromatix_rnr->sigma_lut[i];
  }
#ifdef DEBUG_SIGMA_TABLES
  for (i =0; i < chromatix_rnr->lut_size; i++) {
    IDBG_MED("%s:sigma_lut[%d]: %f", __func__, i,
      p_client->cac2_cfg_info.rnr_chromatix_info.sigma_lut[i]);
  }
#endif
  if(chromatix_rnr->rnr_data[regionStart].center_noise_weight != 0) {
    p_client->cac2_cfg_info.rnr_chromatix_info.center_noise_weight =
      chromatix_rnr->rnr_data[regionStart].center_noise_weight;
  } else {
    p_client->cac2_cfg_info.rnr_chromatix_info.center_noise_weight = 1.0f;
  }
  if(regionStart != regionEnd) {
    p_client->cac2_cfg_info.rnr_chromatix_info.center_noise_sigma =
      BILINEAR_INTERPOLATION(
      chromatix_rnr->rnr_data[regionStart].center_noise_sigma,
      chromatix_rnr->rnr_data[regionEnd].center_noise_sigma, interp_ratio);
  } else {
    p_client->cac2_cfg_info.rnr_chromatix_info.center_noise_sigma =
      chromatix_rnr->rnr_data[regionStart].center_noise_sigma;
  }
  IDBG_MED("%s: center_noise_sigma = %f\n", __func__,
    p_client->cac2_cfg_info.rnr_chromatix_info.center_noise_sigma);

#ifdef USE_RNR_HYSTERISIS
   //Hysterisis only in burst mode
  if((p_client->cac2_cfg_info.rnr_hysterisis_info.prev_lux_value != 0) ||
    (p_client->cac2_cfg_info.rnr_hysterisis_info.prev_gain_value != 0)) {
      module_cac2_config_get_sampling_factor(p_client, chromatix_rnr, trigger);
    //If hysterisis fails, get sampling factor from chromatix.
    if(p_client->cac2_cfg_info.rnr_chromatix_info.sampling_factor == 0){
      p_client->cac2_cfg_info.rnr_chromatix_info.sampling_factor =
        chromatix_rnr->rnr_data[regionStart].sampling_factor;
    }
  } else {
    p_client->cac2_cfg_info.rnr_chromatix_info.sampling_factor =
    chromatix_rnr->rnr_data[regionStart].sampling_factor;
  }
    //Save current gain, lux and sampling factor  values as prev values
    p_client->cac2_cfg_info.rnr_hysterisis_info.prev_lux_value = lux_idx;
    p_client->cac2_cfg_info.rnr_hysterisis_info.prev_gain_value = gain;
    p_client->cac2_cfg_info.rnr_hysterisis_info.prev_sampling_factor =
      p_client->cac2_cfg_info.rnr_chromatix_info.sampling_factor;
#else
  IDBG_MED("%s:%d: Hysterisis not enabled", __func__, __LINE__);
   p_client->cac2_cfg_info.rnr_chromatix_info.sampling_factor =
   chromatix_rnr->rnr_data[regionStart].sampling_factor;
#endif
    p_client->cac2_cfg_info.rnr_chromatix_info.weight_order =
      chromatix_rnr->rnr_data[regionStart].weight_order;
#else
  p_client->cac2_cfg_info.rnr_enable_flag = TRUE;
  p_client->cac2_cfg_info.rnr_chromatix_info.sampling_factor = 2;
  p_client->cac2_cfg_info.rnr_chromatix_info.scale_factor = 16.0f;
  for(i = 0; i < RNR_LUT_SIZE; i++) {
    p_client->cac2_cfg_info.rnr_chromatix_info.sigma_lut[i] =
      sigma_lut_in[i];
  }
  p_client->cac2_cfg_info.rnr_chromatix_info.lut_size = RNR_LUT_SIZE;
  p_client->cac2_cfg_info.rnr_chromatix_info.center_noise_sigma = 2.0f;
  p_client->cac2_cfg_info.rnr_chromatix_info.center_noise_weight = 1.0f;
  p_client->cac2_cfg_info.rnr_chromatix_info.weight_order = 2.0f;
#endif
  return rc;
}



/**
 * Function: module_cac2_config_get_cac_params
 *
 * Description: This function is to update the CAC parameters
 * for offline usecase for cac v2 from chromatix header
 *
 * Arguments:
 *   @p_client: cac client
 *   @chromatix: chromatix pointer
 *   @native_buf: flag to indicate if its a native buffer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_cac2_config_get_cac_params(cac_client_t *p_client,
  chromatix_parms_type *chromatix, float lux_idx, float gain)
{
 int rc = IMG_SUCCESS;

#ifdef USE_CHROMATIX
 chromatix_CAC2_type *chromatix_cac2;
 int regionStart = 0 , regionEnd = 0, i = 0;
 float  trigger, interp_ratio = 0 ;
 float trigger_start[MAX_LIGHT_TYPES_FOR_SPATIAL];
 float trigger_end[MAX_LIGHT_TYPES_FOR_SPATIAL];

  IDBG_ERROR("%s:chromatix ptr %p",__func__,chromatix);
  chromatix_cac2 = &(chromatix->chromatix_post_processing.chromatix_CAC2_data);
  trigger = (chromatix_cac2->control_CAC2 == 0)? (float)lux_idx : (float)gain;
  IDBG_ERROR("%s:lux_idx %f, gain %f chromatix_cac2->control_CAC2 %d",
    __func__, lux_idx, gain,chromatix_cac2->control_CAC2);
  IDBG_ERROR("%s:Trigger %f",__func__, trigger);
  for ( i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i ++ )
  {
    if ( chromatix_cac2->control_CAC2 == 0 ) { //lux index
      trigger_start[i] =
        chromatix_cac2->cac2_data[i].cac2_trigger.lux_index_start;
      trigger_end[i] =
        chromatix_cac2->cac2_data[i].cac2_trigger.lux_index_end;
    } else {
      trigger_start[i] = chromatix_cac2->cac2_data[i].cac2_trigger.gain_start ;
      trigger_end[i] = chromatix_cac2->cac2_data[i].cac2_trigger.gain_end;
    }
  }

 for(i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i ++ ) {
    IDBG_ERROR("%s:triger_start[%d] = %f, trigger_end[%d] = %f", __func__,
    i,trigger_start[i],i,trigger_end[i]);
  }
  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++ ) {
    if ((trigger_start[i]>=trigger_end[i])) {
      IDBG_ERROR("%s %d: strigger start %f is <= to trigger end %f", __func__,
        __LINE__, trigger_start[i], trigger_end[i]);
    }
    if ( i == ( MAX_LIGHT_TYPES_FOR_SPATIAL - 1 )) {
      regionStart = MAX_LIGHT_TYPES_FOR_SPATIAL-1 ;
      regionEnd = MAX_LIGHT_TYPES_FOR_SPATIAL-1 ;
      break;
    }
    if (trigger <= trigger_start[i]) {
      regionStart = i;
      regionEnd = i;
      interp_ratio = 0.0;
      break;
    } else if (trigger < trigger_end[i] ) {
      regionStart = i;
      regionEnd = i+1;
      interp_ratio = (float) (trigger - trigger_start[i])/
        (float)(trigger_end[i] - trigger_start[i]);
      break;
    }
  }
  if (interp_ratio < 1.0) {
    IDBG_MED("%s %d: Interpolation ratio %f < 1.0", __func__, __LINE__,
      interp_ratio);
  }
  IDBG_HIGH("%s %d: region start %d regionend %d", __func__, __LINE__,
    regionStart, regionEnd);
  if(regionStart != regionEnd){
  p_client->cac2_cfg_info.cac_chromatix_info.detection_th1 =
      Round(BILINEAR_INTERPOLATION (
      chromatix_cac2->cac2_data[regionStart].Detection_TH1,
      chromatix_cac2->cac2_data[regionEnd].Detection_TH1, interp_ratio));
  p_client->cac2_cfg_info.cac_chromatix_info.detection_th2 =
      Round(BILINEAR_INTERPOLATION (
      chromatix_cac2->cac2_data[regionStart].Detection_TH2,
      chromatix_cac2->cac2_data[regionEnd].Detection_TH2, interp_ratio));
  p_client->cac2_cfg_info.cac_chromatix_info.detection_th3 =
      Round(BILINEAR_INTERPOLATION (
      chromatix_cac2->cac2_data[regionStart].Detection_TH3,
      chromatix_cac2->cac2_data[regionEnd].Detection_TH3, interp_ratio));
  p_client->cac2_cfg_info.cac_chromatix_info.verification_th1 =
      Round(BILINEAR_INTERPOLATION(
    chromatix_cac2->cac2_data[regionStart].Verification_TH1,
      chromatix_cac2->cac2_data[regionEnd].Verification_TH1, interp_ratio));
  p_client->cac2_cfg_info.cac_chromatix_info.correction_strength =
      Round(BILINEAR_INTERPOLATION(
    chromatix_cac2->cac2_data[regionStart].Correction_Strength,
      chromatix_cac2->cac2_data[regionEnd].Correction_Strength, interp_ratio));
  } else {
    p_client->cac2_cfg_info.cac_chromatix_info.detection_th1 =
      chromatix_cac2->cac2_data[regionStart].Detection_TH1;
    p_client->cac2_cfg_info.cac_chromatix_info.detection_th2 =
      chromatix_cac2->cac2_data[regionStart].Detection_TH2;
    p_client->cac2_cfg_info.cac_chromatix_info.detection_th3 =
      chromatix_cac2->cac2_data[regionStart].Detection_TH3;
    p_client->cac2_cfg_info.cac_chromatix_info.verification_th1 =
      chromatix_cac2->cac2_data[regionStart].Verification_TH1;
    p_client->cac2_cfg_info.cac_chromatix_info.correction_strength =
      chromatix_cac2->cac2_data[regionStart].Correction_Strength;
  }
#else
  p_client->cac2_cfg_info.cac_chromatix_info.detection_th1 = 30;
  p_client->cac2_cfg_info.cac_chromatix_info.detection_th2 = 15;
  p_client->cac2_cfg_info.cac_chromatix_info.detection_th3 = 240;
  p_client->cac2_cfg_info.cac_chromatix_info.verification_th1 = 10;
  p_client->cac2_cfg_info.cac_chromatix_info.correction_strength = 32;

#endif
  IDBG_ERROR("%s: detection_th1 = %d\n", __func__,
    p_client->cac2_cfg_info.cac_chromatix_info.detection_th1);
  return rc;
}
/**
 * Function: module_cac2_config_update_offline_params
 *
 * Description: This function is to update the CAC parameters
 * for offline usecase for cac v2.
 *
 * Arguments:
 *   @p_client: cac client
 *   @pframe: frame pointer
 *   @native_buf: flag to indicate if its a native buffer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_cac2_config_update_offline_params(cac_client_t *p_client)
{
  int status = IMG_SUCCESS;
  cam_metadata_info_t *metadata_buff;
  mct_stream_session_metadata_info* session_meta;
  chromatix_parms_type *chromatix = NULL;
  awb_update_t awb_update_val;
  awb_update_t *p_awb_update = &awb_update_val;
  stats_get_data_t* stats_get_data = NULL;
  float lux_idx;
  float gain;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  metadata_buff = mct_module_get_buffer_ptr(
    p_client->stream_info->parm_buf.reprocess.meta_buf_index,
    p_client->parent_mod,
    IMGLIB_SESSIONID(p_client->identity),
    p_client->stream_info->parm_buf.reprocess.meta_stream_handle);

  if (!metadata_buff) {
    IDBG_ERROR("%s:%d] Invalid metadata buffer", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }
  session_meta =
    (mct_stream_session_metadata_info *)&metadata_buff->private_metadata;
  if (!(session_meta->sensor_data.chromatix_ptr
    && session_meta->sensor_data.common_chromatix_ptr)) {
    IDBG_ERROR("%s:%d] Invalid chromatix pointer", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  chromatix = session_meta->sensor_data.chromatix_ptr;
  //Get lux and gain

  stats_get_data =
    (stats_get_data_t*)&session_meta->stats_aec_data.private_data;
  lux_idx = stats_get_data->aec_get.lux_idx;
  gain = stats_get_data->aec_get.real_gain[0];


  IDBG_HIGH("%s:lux_idx %f, gain %f, cac2_enable_flag = %d", __func__,
    lux_idx, gain, p_client->cac2_cfg_info.cac2_enable_flag);
  //Check if CAC is valid
  if (p_client->cac2_cfg_info.cac2_enable_flag) {
    status = module_cac2_config_get_cac_params(p_client, chromatix,
      lux_idx, gain);
    if (status) {
       IDBG_ERROR("%s %d: Error getting CAC params", __func__, __LINE__);
       p_client->cac2_cfg_info.cac2_enable_flag = FALSE;
    }

  }
  status = module_cac2_config_get_rnr_params(p_client, chromatix,
      lux_idx, gain);
  if (status) {
    IDBG_ERROR("%s %d: Error getting RNR params", __func__, __LINE__);
      p_client->cac2_cfg_info.rnr_enable_flag = FALSE;
  }

#ifdef DEBUG_SIGMA_TBL
  int i = 0;
  for (i= 0; i < RNR_LUT_SIZE; i++) {
    ALOGE("sigma_lut[%d] %d", i,
      p_client->cac2_cfg_info.rnr_chromatix_info.sigma_lut[i]);
  }
#endif

  return status;
}

/**
 * Function: module_cac_v2_config_client
 *
 * Description: This function configures the cac v2 component
 *
 * Arguments:
 *   @p_client: cac client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_v2_config_client(cac_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;

  IDBG_MED("%s:%d] type %d", __func__, __LINE__,
    p_client->stream_info->stream_type);

  if (CAM_STREAM_TYPE_OFFLINE_PROC == p_client->stream_info->stream_type) {
    rc = module_cac2_config_update_offline_params(p_client);
  }

  //Fill in the CAC/RNR data
  p_client->cac2_cfg_info.chroma_order = CAC_CHROMA_ORDER_CRCB;

  //Set the component to be executed in syncromous mode
  p_client->mode = IMG_SYNC_MODE;

  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_ENABLED,
    (void *)&(p_client->cac2_cfg_info.cac2_enable_flag));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] QCAC_ENABLED rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_CHROMATIX_INFO,
    (void *)&(p_client->cac2_cfg_info.cac_chromatix_info));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] QCAC_CHROMATIX_INFO rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QRNR_ENABLED,
    (void *)&(p_client->cac2_cfg_info.rnr_enable_flag));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] QRNR_ENABLED rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QRNR_CHROMATIX_INFO,
    (void *)&(p_client->cac2_cfg_info.rnr_chromatix_info));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] QRNR_CHROMATIX_INFO rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_CHROMA_ORDER,
    (void *)&(p_client->cac2_cfg_info.chroma_order));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Cannot set Chroma Order rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_MODE,
    (void*)&(p_client->mode));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] QIMG_PARAM_MODE rc %d", __func__, __LINE__, rc);
    return rc;
  }

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return rc;
}
