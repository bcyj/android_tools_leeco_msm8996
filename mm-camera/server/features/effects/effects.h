/**********************************************************************
* Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#ifndef __EFFECTCONFIG_H__
#define __EFFECTCONFIG_H__

#include "tgtcommon.h"
#include "sensor_interface.h"

typedef enum {
  CAMERA_HISTOGRAM_OFF,
  CAMERA_HISTOGRAM_Y,
  CAMERA_HISTOGRAM_RGB,
  CAMERA_HISTOGRAM_YRGB
} effects_histogram_mode_t;

/* Reflection type is used for CAMERA_PARM_REFLECT.
 * It is only used for preview.
 * For sensor that is mounted facing the phone user: use mirror reflection.
 *
 * For sensor that is mounted facing away from the phone user: use no
 * reflection.
 *
 * For sensor that is mounted normally facing away from the phone user, but can 
 * be rotated so that the sensor is facing the phone user: 
 *
 *   if the normal preview display is not up side down after roation, then use
 *   no reflection when the sensor is facing away from the user and use mirror
 *   reflection when the sensor is facing the user.
 *
 *   if the normal preview display is up side down after roation, then use
 *   reflection when the sensor is facing the user. */
typedef enum {
  CAMERA_NO_REFLECT,
  CAMERA_MIRROR_REFLECT,
  CAMERA_WATER_REFLECT,
  CAMERA_MAX_REFLECT
} effects_reflect_val_t;

typedef cam_parm_info_t effects_nightshot_t; 
typedef cam_parm_info_t effects_reflect_t; 
typedef cam_parm_info_t effects_antibanding_t; 
typedef cam_parm_info_t effects_red_eye_reduction_t; 
typedef cam_parm_info_t effects_brightness_t; 
typedef cam_parm_info_t effects_contrast_t; 
typedef cam_parm_info_t effects_sharpness_t; 
typedef cam_parm_info_t effects_saturation_t; 
typedef cam_parm_info_t effects_iso_t; 
typedef cam_parm_info_t effects_preview_fps_t; 
typedef cam_parm_info_t effects_histogram_t; 
typedef cam_parm_info_t effects_ev_compensation_t; 
typedef cam_parm_info_t effects_hue_t;

typedef struct {
  cam_parm_info_t parm;

  int8_t spl_effect_solarize_on;
  int8_t spl_effects_enabled;
  int8_t spl_effects_gamma_updated;
} effects_special_effects_t;

typedef struct {
  float c00;
  float c01;
  float c02;

  float c10;
  float c11;
  float c12;

  float c20;
  float c21;
  float c22;
} effects_matrix_3x3_t;

typedef struct {
  effects_nightshot_t         nightshotInfo;
  effects_reflect_t           reflectInfo;
  effects_antibanding_t       antibandingInfo;
  effects_red_eye_reduction_t redEyeReductionInfo;
  effects_brightness_t        brightnessInfo;
  effects_contrast_t          contrastInfo;
  effects_special_effects_t   specialEffectsInfo;
  effects_sharpness_t         sharpnessInfo;
  effects_hue_t               hueInfo;
  effects_saturation_t        saturationInfo;
  effects_iso_t               isoInfo;
  effects_preview_fps_t       previewFpsInfo;
  effects_histogram_t         histogramInfo;

  int8_t                      camera_la_enable;
  int16_t                     camera_la_detect;

  matrix_2x2_t                color_conversion_matrix;
  matrix_3x3_t                color_correction_matrix;

  int init;
} effects_ctrl_t; 

void effects_init(effects_ctrl_t *effect);
int8_t effects_set_special_effect(void *ctrl, int32_t parm);
int8_t effects_set_contrast(void *ctrlBlk, int32_t parm);
int8_t effects_set_hue(void *ctrlBlk, int32_t hue);
int8_t effects_set_saturation(void *ctrlBlk, int32_t saturation);
//gamma_table_t effects_select_gamma_table(sensor_mode_t op_mode);
#endif /* __EFFECTCONFIG_H__ */
