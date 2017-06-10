/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __VFE_UTIL_COMMON_H__
#define __VFE_UTIL_COMMON_H__

#include <unistd.h>
#include <math.h>
#include <stdint.h>
#include <media/msm_isp.h>
#include <sys/time.h>
#include <media/msm_camera.h>
#include "vfe_interface.h"
#include "vfe_tgtcommon.h"
#include "chromatix.h"
#include "camera_dbg.h"

/*TODO: Need to add enums for CMD_GENERAL
as of now the CMD_GENERAL is added from camera.h thru tgtcommon.h
*/

/* TODO: Remove this once kernel change mainlines. */
#ifndef VFE_CMD_MODULE_CFG
#define VFE_CMD_MODULE_CFG VFE_CMD_EZTUNE_CFG
#endif

#define Q14   0x00004000
#define Q13   0x00002000
#define Q12   0x00001000
#define Q12_2 0x00002000
#define Q12_4 0x00004000
#define Q11   0x00000800
#define Q10   0x00000400
#define Q8    0x00000100
#define Q7    0x00000080
#define Q6    0x00000040
#define Q4    0x00000010

#define PREV 0
#define SNAP 1

/*bit for each module. used to identify which all modules are updated*/
#define VFE_MOD_LINEARIZATION    (1<<0)
#define VFE_MOD_ROLLOFF          (1<<1)
#define VFE_MOD_DEMUX            (1<<2)
#define VFE_MOD_DEMOSAIC         (1<<3)
#define VFE_MOD_BPC              (1<<4)
#define VFE_MOD_ABF              (1<<5)
#define VFE_MOD_ASF              (1<<6)
// module not applicable for 7x27a
#define VFE_MOD_COLOR_CONV       (1<<7)
#define VFE_MOD_COLOR_CORRECT    (1<<8)
#define VFE_MOD_CHROMA_SS        (1<<9)
// module not applicable for 7x27a
#define VFE_MOD_CHROMA_SUPPRESS (1<<10)
#define VFE_MOD_LA              (1<<11)
#define VFE_MOD_MCE             (1<<12)
#define VFE_MOD_SCE             (1<<13)
#define VFE_MOD_CLF             (1<<14)
#define VFE_MOD_WB              (1<<15)
#define VFE_MOD_GAMMA           (1<<16)
#define VFE_MOD_AWB_STATS       (1<<17)
#define VFE_MOD_AEC_STATS       (1<<18)
#define VFE_MOD_AF_STATS        (1<<19)
#define VFE_MOD_FOV             (1<<20)
#define VFE_MOD_SCALER          (1<<21)
#define VFE_MOD_BCC             (1<<22)
#define VFE_MOD_CLAMP           (1<<23)
#define VFE_MOD_FRAME_SKIP      (1<<24)
#define VFE_MOD_IHIST_STATS     (1<<25)
#define VFE_MOD_RSCS_STATS      (1<<26)
#define VFE_MOD_AEC_AWB_STATS   (1<<27)
#define VFE_MOD_BHIST_STATS     (1<<28)
#define VFE_MOD_SCALER_ENC      (1<<29)
#define VFE_MOD_FOV_ENC         (1<<30)
#define VFE_MOD_BE_STATS        (1<<31)

//for 7x27a
#define VFE_MOD_COLOR_PROC       (1<<7)
//for 7x27a
#define VFE_MOD_ACTIVE_CROP     (1<<10)
#define VFE_MOD_COUNT 27

#define FLOAT_TO_Q(exp, f) \
  ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))

/* Return v1 * ratio + v2 * ( 1.0 - ratio ) */
#define LINEAR_INTERPOLATION(v1, v2, ratio) \
  ((v2) + ((ratio) * ((v1) - (v2))))

#define LINEAR_INTERPOLATION_INT(v1, v2, ratio) \
  (roundf((v2) + ((ratio) * ((v1) - (v2)))))

#define MIN(x,y) (((x)<(y)) ? (x) : (y))
#define MAX(x,y) (((x)>(y)) ? (x) : (y))

#define CameraExp(x) (exp(x))
#define CameraSquareRoot(x) (sqrt(x))

#define Clamp(x, t1, t2) (((x) < (t1))? (t1): ((x) > (t2))? (t2): (x))

#define MIRED(x) (1000000 / (x))

#define F_EQUAL(a, b) \
  ( fabs(a-b) < 1e-4 )

#define GET_INTERPOLATION_RATIO(ct, s, e) (1.0 - ((ct) - (s))/((e) - (s)))

#define MATCH(v1, v2, th) ((abs(v2-v1) <= (th)))

#define CALC_CCT_TRIGGER_MIRED(out, in) ({ \
  out.mired_start = MIRED(in.CCT_start); \
  out.mired_end = MIRED(in.CCT_end); \
})

#define TBL_INTERPOLATE(in1, in2, out, ratio, size, i) ({\
  for (i=0; i<size; i++) \
    out[i] = LINEAR_INTERPOLATION(in1[i], in2[i], ratio); })

#define TBL_INTERPOLATE_INT(in1, in2, out, ratio, size, i) ({\
  for (i=0; i<size; i++) \
    out[i] = LINEAR_INTERPOLATION_INT(in1[i], in2[i], ratio); })

#define CEIL_LOG2(n) ({ \
  int32_t val = 0, n1 = n; \
  if (n <= 1) \
    val = 0; \
  else { \
    while (n1 > 1) { \
      val++; \
      n1 >>= 1; \
    }\
  } \
  val;})

#define MATRIX_INVERSE_3x3(MatIn, MatOut) ({\
  typeof (MatOut[0]) __det; \
  if (MatIn == NULL || MatOut == NULL) \
    return FALSE; \
  __det = MatIn[0]*(MatIn[4]*MatIn[8]-MatIn[5]*MatIn[7]) + \
          MatIn[1]*(MatIn[5]*MatIn[6]-MatIn[3]*MatIn[8]) + \
          MatIn[2]*(MatIn[3]*MatIn[7]-MatIn[4]*MatIn[6]); \
  if (__det == 0) \
    return FALSE; \
  MatOut[0] = (MatIn[4]*MatIn[8] - MatIn[5]*MatIn[7]) / __det; \
  MatOut[1] = (MatIn[2]*MatIn[7] - MatIn[1]*MatIn[8]) / __det; \
  MatOut[2] = (MatIn[1]*MatIn[5] - MatIn[2]*MatIn[4]) / __det; \
  MatOut[3] = (MatIn[5]*MatIn[6] - MatIn[3]*MatIn[8]) / __det; \
  MatOut[4] = (MatIn[0]*MatIn[8] - MatIn[2]*MatIn[6]) / __det; \
  MatOut[5] = (MatIn[2]*MatIn[3] - MatIn[0]*MatIn[5]) / __det; \
  MatOut[6] = (MatIn[3]*MatIn[7] - MatIn[4]*MatIn[6]) / __det; \
  MatOut[7] = (MatIn[1]*MatIn[6] - MatIn[0]*MatIn[7]) / __det; \
  MatOut[8] = (MatIn[0]*MatIn[4] - MatIn[1]*MatIn[3]) / __det; \
})


#define COPY_MATRIX(IN, OUT, M, N) ({ \
  int i, j; \
  for (i=0; i<M; i++) \
    for (j=0; j<N; j++) \
      OUT[i][j] = IN[i][j]; })

/* IN1 MxN  IN2 NxL*/
#define MATRIX_MULT(IN1, IN2, OUT, M, N, L) ({ \
  int i, j, k; \
  for (i=0; i<M; i++) \
    for (j=0; j<L; j++) { \
      OUT[i][j] = 0; \
      for (k=0; k<N; k++) \
        OUT[i][j] += (IN1[i][k] * IN2[k][j]); \
    } })

#define IS_UNITY_MATRIX(IN, N) ({\
  int i, j, ret = TRUE; \
  for (i=0; i<N; i++) \
     for (j=0; j<N; j++) \
       if (((i == j) && (IN[i][j] != 1)) \
         || ((i != j) && (IN[i][j] != 0))) { \
         ret = FALSE; \
         break; \
       } \
   ret;})

#define SET_UNITY_MATRIX(IN, N) ({\
  int i, j;\
  for (i=0; i<N; i++) \
     for (j=0; j<N; j++) { \
       if (i == j) \
         IN[i][j] = 1; \
       else \
         IN[i][j] = 0; \
     }})

#define IS_SNAP_MODE(parms) ((parms->vfe_op_mode == VFE_OP_MODE_SNAPSHOT) \
  || (parms->vfe_op_mode == VFE_OP_MODE_RAW_SNAPSHOT) \
  || (parms->vfe_op_mode == VFE_OP_MODE_JPEG_SNAPSHOT))

#define IS_BAYER_FORMAT(parms) (parms->sensor_parms.vfe_snsr_fmt == SENSOR_BAYER)

#define IS_MANUAL_WB(parms) (parms->wb != CAMERA_WB_AUTO)

#define VALIDATE_TST_VEC(ip, op, range, str)({\
  if (abs(ip - op) > range) {\
    CDBG_ERROR("%s: Mismatch, %s, diff: %d, ip: %08x, op : %08x\n",\
    __func__, str, abs(ip - op), ip, op);\
  }\
})

#define VALIDATE_TST_LUT(ip, op, range, str, index)({\
    if (abs(ip - op) > range) {\
      CDBG_ERROR("%s: Mismatch, %s, Index :%d, diff: %d, ip: %08x, op : %08x\n",\
       __func__, str, index, abs(ip - op), ip, op);\
    }\
})

#define CUBIC_F(fs, fc0, fc1, fc2, fc3) ({ \
  double fs3, fs2;\
  fs2 = fs * fs; \
  fs3 = fs * fs2; \
  fc0 = 0.5 * (-fs3 + (2.0 * fs2) - fs); \
  fc1 = 0.5 * ((3.0 * fs3) - (5.0 * fs2) + 2.0); \
  fc2 = 0.5 * ((-3.0 * fs3) + (4.0 * fs2) + fs); \
  fc3 = 0.5 * (fs3 - fs2); \
})

typedef enum {
  CAM_MODE_2D = 0,
  CAM_MODE_3D,
  CAM_MODE_2D_HFR,
  CAM_MODE_ZSL,
  CAM_MODE_MAX,
} cam_mode_t;

typedef enum {
  MODE_OF_OPERATION_CONTINUOUS,
  MODE_OF_OPERATION_SNAPSHOT,
  MODE_OF_OPERATION_VIDEO,
  MODE_OF_OPERATION_RAW_SNAPSHOT,
  MODE_OF_OPERATION_ZSL,
  MODE_OF_OPERATION_JPEG_SNAPSHOT,
  LAST_MODE_OF_OPERATION_ENUM
} VFE_ModeOfOperationType;

typedef enum {
  OPERATION_CONTINUOUS,
  OPERATION_SNAPSHOT,
}VFE2X_ModeOfOperationType;

typedef enum {
  TRIGGER_LOWLIGHT,
  TRIGGER_NORMAL,
  TRIGGER_OUTDOOR,
} trigger_lighting_t;

typedef struct {
  float ratio;
  trigger_lighting_t lighting;
} trigger_ratio_t;

typedef enum VFE_EffectsType {
  VFE_DEFAULT,
  VFE_SEPIA,
  VFE_MONO,
  VFE_NEGATIVE,
  VFE_AQUA,
  VFE_POSTERIZE,
  VFE_BLACKBOARD,
  VFE_WHITEBOARD,
  VFE_SOLARIZE,
} VFE_EffectsType;

typedef enum awb_cct_type {
  AWB_CCT_TYPE_D65,
  AWB_CCT_TYPE_D65_TL84,
  AWB_CCT_TYPE_TL84,
  AWB_CCT_TYPE_TL84_A,
  AWB_CCT_TYPE_A,
  AWB_CCT_TYPE_MAX,
}awb_cct_type;

typedef struct{
  float mired_start;
  float mired_end;
}cct_trigger_type;

typedef struct {
  cct_trigger_type trigger_A;
  cct_trigger_type trigger_d65;
  float mired_color_temp;
}cct_trigger_info;

typedef enum {
  VFE_AEC_LOWLIGHT,
  VFE_AEC_NORMAL,
  VFE_AEC_BRIGHT,
}vfe_aec_type_t;

typedef struct {
  int32_t id;
  uint16_t length;
  void *value;
} vfe_hw_cmd_t;

/*===========================================================================
 *  rolloff Interface info
 *==========================================================================*/
typedef enum
{
  VFE_ROLLOFF_TL84_LIGHT,   /* Flourescent  */
  VFE_ROLLOFF_A_LIGHT,      /* Incandescent */
  VFE_ROLLOFF_D65_LIGHT,    /* Day Light    */
  VFE_ROLLOFF_LOW_LIGHT,    /* Low Light    */
  VFE_ROLLOFF_LED_FLASH,    /* Led Flash    */
  VFE_ROLLOFF_STROBE_FLASH, /* Stribe Flash */
  VFE_ROLLOFF_MAX_LIGHT,
  VFE_ROLLOFF_INVALID_LIGHT = VFE_ROLLOFF_MAX_LIGHT
}rolloff_light_type;

typedef enum {
  VFE_ROLLOFF_CH_R,
  VFE_ROLLOFF_CH_GR,
  VFE_ROLLOFF_CH_GB,
  VFE_ROLLOFF_CH_B,
  VFE_ROLLOFF_CH_MAX
}rolloff_color_channels;

typedef enum {
  OUT_PREVIEW,
  OUT_ENCODER,
  OUT_MAX,
}output_type;

typedef struct {
  mesh_rolloff_array_type left[VFE_ROLLOFF_MAX_LIGHT];
  mesh_rolloff_array_type right[VFE_ROLLOFF_MAX_LIGHT];
}vfe_rolloff_info_t;

typedef struct {
  uint32_t support_3d;
}vfe_stereocam_info_t;

typedef struct {
  float total_dig_gain;
  float cc_gain_adj;
  float wb_gain_adj;
}vfe_aec_gain_adj_t;

typedef struct {
vfe_zoom_info_t y;
vfe_zoom_info_t cbcr;
}vfe_crop_info;

typedef struct {
  int camfd;
  uint32_t vfe_version;
  uint32_t vfe_op_mode;
  uint32_t output1w;
  uint32_t output1h;
  uint32_t output2w;
  uint32_t output2h;
  cam_format_t enc_format;
  uint32_t eztune_status;
  camera_hfr_mode_t vfe_hfr_mode;
  cam_mode_t cam_mode;
  chromatix_parms_type *chroma3a;
  VFE_ModuleCfgPacked *moduleCfg;
  vfe_sensor_params_t sensor_parms;
  camera_size_t vfe_input_win;
  pixel_crop_info_t demosaic_op_params;
  pixel_crop_info_t scaler_op_params[OUT_MAX];
  vfe_crop_info crop_ops_params[OUT_MAX];
  vfe_aec_parms_t aec_params;
  vfe_af_params_t af_params;
  vfe_awb_params_t awb_params;
  vfe_asd_params_t asd_params;
  vfe_stats_rs_cs_params rs_cs_params;
  vfe_zoom_info_t crop_info;
  vfe_sharpness_info_t sharpness_info;
  vfe_rolloff_info_t rolloff_info;
  vfe_stereocam_info_t stereocam_info;
  vfe_effects_params_t effects_params;
  vfe_spl_effects_type prev_spl_effect;
  vfe_aec_gain_adj_t aec_gain_adj;
  camera_bestshot_mode_type bs_mode;
  vfe_flash_parms_t flash_params;
#ifdef VFE_2X
  vfe_active_region_t active_crop_info;
#endif
  uint32_t update;
  config3a_wb_t wb;
  float digital_gain;
  int awb_gain_update;
  uint32_t current_config;
  uint32_t stats_config;
#ifndef VFE_2X
  uint32_t color_mod_config;
#endif
  int use_default_config;
  vfe_op_mode_t prev_mode;
  uint8_t color_modules_disable;
  int use_cv_for_hue_sat;
  int demosaic_wb_not_present;
  int use_cc_for_dig_gain;
  uint32_t crop_factor;
  int enable_mobicat;
}vfe_params_t;

typedef struct {
  int (*set_param) (void *vfe_client, int type, void *parm_in,
    void *parm_out);
  int (*get_param) (void *vfe_client, int parm_type, void *parm,
    int parm_len);
  int (*process) (void *vfe_client, int event, void *parm);
  int (*destroy) (void *vfe_client);
  int (*parse_stats)(void *client,
  int isp_started, stats_type_t stats_type, void *stats, void *stats_output);
}vfe_ops_t;

vfe_status_t vfe_util_write_hw_cmd(int fd,int type, void *pCmdData,
  unsigned int messageSize, int cmd_id);
awb_cct_type vfe_util_get_awb_cct_type(cct_trigger_info* trigger,
  vfe_params_t* parms);
int8_t vfe_util_aec_check_settled(vfe_aec_parms_t* aec_out);
trigger_ratio_t vfe_util_get_aec_ratio2(tuning_control_type tunning,
  trigger_point_type *outdoor_trigger, trigger_point_type *lowlight_trigger,
  vfe_params_t* parms);
float vfe_util_get_aec_ratio(tuning_control_type tunning,
  trigger_point_type *trigger, vfe_params_t* parms);
float vfe_util_calc_interpolation_weight(float value,
  float start, float end);
uint32_t vfe_util_calculate_shift_bits(uint32_t pixels_in_ae_rgn);

/* Logging */
//#define ENABLE_VFE_LOGGING
//#define ENABLE_LINEAR_LOGGING
//#define ENABLE_BL_LOGGING
//#define ENABLE_DEMUX_LOGGING
//#define ENABLE_ABF_LOGGING
//#define ENABLE_CLF_LOGGING
//#define ENABLE_CC_LOGGING
//#define ENABLE_GAMMA_LOGGING
//#define ENABLE_LA_LOGGING
//#define ENABLE_MCE_LOGGING
//#define ENABLE_SCE_LOGGING
//#define ENABLE_ASF_LOGGING
//#define ENABLE_AWB_STATS_LOGGING
//#define ENABLE_AEC_STATS_LOGGING
//#define ENABLE_AF_STATS_LOGGING
//#define ENABLE_WB_LOGGING
//#define ENABLE_CV_LOGGING
//#define ENABLE_TEST_VECTOR_LOGGING
//#define ENABLE_ASF_LOGGING
//#define ENABLE_BPC_LOGGING
//#define ENABLE_BCC_LOGGING
//#define ENABLE_LINEAR_LOGGING
//#define ENABLE_PCA_LOGGING
//#define ENABLE_CHROMA_SUPP_LOGGING
//#define ENABLE_DEMOSAIC_LOGGING
//#define ENABLE_FOV_LOGGING
//#define ENABLE_SCALER_LOGGING
//#define ENABLE_MESH_ROLLOFF_LOGGING
//#define ENABLE_ROLLOFF_LOGGING
//#define ENABLE_IHIST_STATS_LOGGING
//#define ENABLE_RS_CS_STATS_LOGGING
//#define ENABLE_STATS_PARSER_LOGGING
//#define ENABLE_BAYER_GRID_LOGGING
//#define ENABLE_BAYER_HIST_LOGGING
//#define ENABLE_BAYER_FOCUS_LOGGING

#endif //__VFE_UTIL_COMMON_H__
