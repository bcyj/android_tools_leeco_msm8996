/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __EZTUNE_DIAGNOSTICS_H__
#define __EZTUNE_DIAGNOSTICS_H__
#include <inttypes.h>
#include "eztune_vfe_diagnostics.h"

typedef struct {
  float focal_length;
  float pixel_size;
  float f_number;
  float total_foc_dist;
  float hor_view_angle;
  float ver_view_angle;
} ez_sensor_lens_spec_t;

typedef struct {
  uint16_t fullsize_width;
  uint16_t fullsize_height;
  uint16_t qtrsize_width;
  uint16_t qtrsize_height;
  uint16_t pixelsperLine;
  uint16_t sensor_type;
  uint16_t format;
  uint32_t pixelclock_freq;
  ez_sensor_lens_spec_t lens_spec;
}ez_sensor_params_t;

typedef struct {
  uint16_t disp_width;
  uint16_t disp_height;
  uint16_t pict_height;
  uint16_t pict_width;
  uint16_t curr_width;
  uint16_t curr_height;
}ez_config_params_t;

typedef struct {
  int32_t enable;
  int32_t lock;
  int32_t ymin_pct;
  int32_t white_stat_on;
  int32_t decision;
  int32_t outdoor_pos_grcount;
  int32_t indoor_pos_grcount;
  int32_t color_temp;
  int32_t is_compact_cluster;
  float ave_rg_ratio;
  float ave_bg_ratio;
  float sgw_rg_ratio;
  float sgw_bg_ratio;
  float white_rg_ratio;
  float white_bg_ratio;
  float shifted_D50RG;
  float shifted_D50BG;
  float prev_wbrgain;
  float prev_wbggain;
  float prev_wbbgain;
  float snap_wbrgain;
  float snap_wbggain;
  float snap_wbbgain;
} ez_awb_params_t;

typedef struct {
  int32_t enable;
  int32_t luma;
  int32_t lock;
  int32_t expindex;
  int32_t luxindex;
  int32_t touch_ROIluma;
  int32_t test_enable;
  int32_t force_snaplinecount;
  int32_t force_prevlinecount;
  int32_t prev_forceexp;
  int32_t snap_forceexp;
  int32_t prev_linecount;
  int32_t snap_linecount;
  float   prev_realgain;
  float   prev_exposuretime;
  float   snap_realgain;
  float   snap_exposuretime;
  float   force_snapgain;
  float   force_prevgain;
  int32_t antibanding_enable;
}ez_aec_params_t;

typedef struct {
  int32_t enable;
  int32_t peakpos_index;
  int32_t tracing_index;
  int32_t tracing_stats[50];
  int32_t tracing_pos[50];
} ez_af_params_t;

typedef struct {
  uint8_t linearenable;
  uint8_t linearstepsize;
  uint8_t ringenable;
  uint8_t ringstepsize;
  uint8_t deffocenable;
  uint8_t movfocenable;
  uint8_t movfocdirection;
  uint8_t movfocsteps;
} ez_af_tuning_params_t;

typedef struct {
  ez_awb_params_t awb_params;
  ez_aec_params_t aec_params;
  ez_af_params_t af_params;
} ez_3a_params_t;

typedef struct {
  int32_t flicker_detect;
  int32_t flicker_freq;
  int32_t status;
  int32_t std_width;
  int32_t multiple_peak_algo;
  int32_t actual_peaks;
} ez_afd_params_t;

typedef struct {
  uint32_t bls_detected;
  uint32_t bls_histbcklit_detected;
  uint32_t bls_severity;
  uint32_t bls_mixlightcase;
  uint32_t sns_luma_thres;
  uint32_t sns_ymaxingray;
  uint32_t sns_min_sample_thres;
  uint32_t sns_extr_sample_thres;
} ez_asd_params_t;

typedef struct {
  ez_afd_params_t afd_params;
  ez_asd_params_t asd_params;
} ez_pp_params_t;

typedef enum {
  EZ_AF_LOADPARAMS,
  EZ_AF_LINEARTEST_ENABLE,
  EZ_AF_RINGTEST_ENABLE,
  EZ_AF_DEFFOCUSTEST_ENABLE,
  EZ_AF_MOVFOCUSTEST_ENABLE,
} aftuning_optype_t;

typedef enum {
  EZ_STATUS,
  EZ_AEC_ENABLE,
  EZ_AEC_TESTENABLE,
  EZ_AEC_LOCK,
  EZ_AEC_FORCESNAPEXPOSURE,
  EZ_AEC_FORCESNAPLINECOUNT,
  EZ_AEC_FORCESNAPGAIN,
  EZ_AEC_FORCEPREVEXPOSURE,
  EZ_AEC_FORCEPREVLINECOUNT,
  EZ_AEC_FORCEPREVGAIN,
  EZ_AWB_ENABLE,
  EZ_AWB_CONTROLENABLE,
  EZ_AF_ENABLE,
  EZ_AEC_ABENABLE,
  EZ_RELOAD_CHROMATIX,
}isp_set_optype_t;

typedef enum {
  EZ_MISC_PREVIEW_RESOLUTION,
  EZ_MISC_SNAPSHOT_RESOLUTION,
  EZ_MISC_CURRENT_RESOLUTION,
  EZ_MISC_SENSOR_FORMAT,
  EZ_MISC_SENSOR_TYPE,
  EZ_MISC_SENSOR_FULLWIDTH,
  EZ_MISC_SENSOR_FULLHEIGHT,
  EZ_MISC_SENSOR_QTRWIDTH,
  EZ_MISC_SENSOR_QTRHEIGHT,
  EZ_MISC_SENSOR_PIXELCLKFREQ,
  EZ_MISC_SENSOR_PIXELSPERLINE,
  EZ_MISC_SENSOR_LENSSPEC,
}miscoptype_t;

typedef enum {
  EZ_MCTL_ISP_AEC_CMD,
  EZ_MCTL_ISP_AWB_CMD,
  EZ_MCTL_ISP_AF_CMD,
  EZ_MCTL_ISP_ASD_CMD,
  EZ_MCTL_ISP_AFD_CMD,
  EZ_MCTL_ISP_DEFAULT,
}isp_diagtype_t;
#endif /* __EZTUNE_DIAGNOSTICS_H__ */
