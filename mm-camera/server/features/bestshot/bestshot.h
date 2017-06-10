/*============================================================================

   Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __BESTSHOT_H__
#define __BESTSHOT_H__

#include "tgtcommon.h"
#include "sensor_interface.h"

/* Used only for best shot mode */
typedef enum {
  BESTSHOT_GAMMA_TABLE_NORMAL = 0,
  BESTSHOT_GAMMA_TABLE_OUTDOOR,
  BESTSHOT_GAMMA_TABLE_LOWLIGHT,
  BESTSHOT_GAMMA_TABLE_BACKLIGHT,
  BESTSHOT_GAMMA_TABLE_MAX
} bestshot_gamma_table_type;

typedef enum {
  BESTSHOT_ISO_AUTO = 0,
  BESTSHOT_ISO_100,
  BESTSHOT_ISO_200,
  BESTSHOT_ISO_400,
  BESTSHOT_ISO_800,
  BESTSHOT_ISO_MAX,
} bestshot_iso_mode_type;

typedef enum {
  BESTSHOT_EV_DEFAULT = 0,
  BESTSHOT_EV_LO,
  BESTSHOT_EV_HI,
  BESTSHOT_EV_MAX
} bestshot_exposure_comp_type;

typedef enum {
  BESTSHOT_AEC_FRAME_AVERAGE = 0,
  BESTSHOT_AEC_CENTER_WEIGHTED,
  BESTSHOT_AEC_SPOT_METERING,
  BESTSHOT_AEC_MAX
} bestshot_auto_exposure_mode_type;

typedef enum {
  BESTSHOT_AF_MODE_NORMAL,
  BESTSHOT_AF_MODE_MACRO,
  BESTSHOT_AF_MODE_INF,
  BESTSHOT_AF_MODE_AUTO,
  BESTSHOT_AF_MODE_MAX
} bestshot_af_mode_type;

typedef enum {
  BESTSHOT_WB_AUTO = 0,
  BESTSHOT_WB_OUTDOOR_AWB,
  BESTSHOT_WB_TUNGSTON, /* TUNGSTON == INCANDESCENT */
  BESTSHOT_WB_D50,
  BESTSHOT_WB_SGW_AWB,
  BESTSHOT_WB_AUTO_OR_FLASH,
  BESTSHOT_WB_FLUORESCENT,
  BESTSHOT_WB_INDOOR_AWB,
  BESTSHOT_WB_MAX
} bestshot_wb_type;

typedef enum {
  BESTSHOT_COLOR_CORRECTION_NORMAL = 0,
  BESTSHOT_COLOR_CORRECTION_LOW_LIGHT,
  BESTSHOT_COLOR_CORRECTION_MAX
} bestshot_color_correction_type;

typedef enum {
  BESTSHOT_COLOR_CONVERSION_NORMAL = 0,
  BESTSHOT_COLOR_CONVERSION_SUNSET,
  BESTSHOT_COLOR_CONVERSION_SKIN_TONE,
  BESTSHOT_COLOR_CONVERSION_SATURATED,
  BESTSHOT_COLOR_CONVERSION_MAX
} bestshot_color_conversion_type;

typedef enum {
  BESTSHOT_SETTING_CURRENT = 0,
  BESTSHOT_SETTING_ON,
  BESTSHOT_SETTING_OFF,
  BESTSHOT_SETTING_MAX
} bestshot_setting_type;

typedef enum {
  BESTSHOT_SPECIAL_WB_ADJ_NONE = 0,
  BESTSHOT_SPECIAL_WB_ADJ_SNOW_BLUE_GAIN,
  BESTSHOT_SPECIAL_WB_ADJ_BEACH_BLUE_GAIN,
  BESTSHOT_SPECIAL_WB_ADJ_MAX
} bestshot_special_wb_adj_type;

typedef enum {
  BESTSHOT_SOFT_FOCUS_OFF = 0,
  BESTSHOT_SOFT_FOCUS_ON
} bestshot_soft_focus_type;

typedef enum {
  BESTSHOT_FPS_AUTO,
  BESTSHOT_FPS_FIXED_MIN,
  BESTSHOT_FPS_FIXED_MAX,
  BESTSHOT_FPS_MAX
} bestshot_fps_mode_type;

typedef struct {
  camera_bestshot_mode_type        mode;
  sensor_load_chromatix_t          chromatix_type;
  bestshot_gamma_table_type        gamma_table;
  bestshot_iso_mode_type           iso_mode;
  bestshot_exposure_comp_type      exp_comp;
  bestshot_auto_exposure_mode_type aec_mode;
  bestshot_af_mode_type            af_mode;
  bestshot_wb_type                 awb;
  bestshot_color_correction_type   color_corr;
  bestshot_color_conversion_type   color_conv;
  bestshot_setting_type            hjr_setting;
  bestshot_setting_type            luma_adaptation;
  bestshot_special_wb_adj_type     special_wb_adj;
  bestshot_soft_focus_type         soft_focus;
  bestshot_fps_mode_type           fps_mode;
} camera_bestshot_config_t;

typedef struct {
  int8_t                         hjr_enabled;
  camera_iso_mode_type           iso_mode;
  camera_auto_exposure_mode_type aec_mode;
  camera_antibanding_type        antibanding;
  int32_t                        af_mode_value;
  int32_t                        WB_value;
  int32_t                        effect_value;
  int32_t                        saturation_value;
  int32_t                        contrast_value;
  fps_mode_t                     fps_mode; /* ?? */
} bestshot_defaultmode_info_t;

typedef struct {
  cam_parm_info_t parm;
  int8_t bestshot_mode_enabled;
} bestshot_mode_info_t;

typedef struct {
  bestshot_defaultmode_info_t   bestshotdefModeInfo;
  bestshot_mode_info_t   bestshotModeInfo;
  float soft_focus_dgr;
} bestshot_ctrl_t;

int bestshot_set_mode(void *cctrl, bestshot_ctrl_t *bestshot, uint8_t parm);
void bestshot_init(bestshot_ctrl_t * bestshot);
gamma_table_t bestshot_select_gamma_table(camera_bestshot_mode_type type);
int bestshot_reconfig_mode(void *cctrl, bestshot_ctrl_t *bestshot);

#endif /* __BESTSHOT_H__ */
