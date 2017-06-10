/**********************************************************************
* Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#ifndef __AEC_H__
#define __AEC_H__

#ifdef _ANDROID_
#include <utils/Log.h>
#endif
#include "stats_proc_interface.h"
#include "camera_dbg.h"
#include "aec_slow_conv.h"

#if(AEC_DEBUG_HIGH)
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-AEC"
  #define CDBG_AEC(fmt, args...) LOGE(fmt, ##args)
  #undef CDBG_HIGH
  #define CDBG_HIGH(fmt, args...) LOGE(fmt, ##args)
#endif
#if(AEC_DEBUG_LOW)
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-AEC"
  #define CDBG_AEC(fmt, args...) LOGE(fmt, ##args)
#else
  #define CDBG_AEC(fmt, args...) do{}while(0)
#endif
#define INTERESTED_SUB_REGN_MAX_NUM      9
#define MAX_LUMA_ARRAY_INDEX             9
#define MOTION_ARRAY_SIZE                5
#define LED_FRAME_SKIP_CNT               3
#define LUMA_SETTLED_BEFORE_AF_CNT       2

#define AEC_Q8     0x00000100
#define USE_AEC_LED_ROI                  0
#define W1 0.533
#define W2 2.4

static float aec_center_weighted_adv[2][256] = { /* Q8 format */
  /* 4:3 Aspect Ratio */
  {
  /* Row 1 */
  0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f,
  0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f,
  /* Row 2 */
  0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.03947f, 0.05263f, 0.05263f,
  0.05263f, 0.05263f, 0.03947f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f,
  /* Row 3 */
  0.02632f, 0.02632f, 0.02632f, 0.03947f, 0.05263f, 0.05263f, 0.07895f, 0.10526f,
  0.10526f, 0.07895f, 0.05263f, 0.05263f, 0.03947f, 0.02632f, 0.02632f, 0.02632f,
  /* Row 4 */
  0.02632f, 0.02632f, 0.03947f, 0.05263f, 0.07895f, 0.10526f, 0.10526f, 0.15789f,
  0.15789f, 0.10526f, 0.10526f, 0.07895f, 0.05263f, 0.03947f, 0.02632f, 0.02632f,
  /* Row 5 */
  0.02632f, 0.02632f, 0.05263f, 0.10526f, 0.10526f, 0.15789f, 0.21053f, 0.31579f,
  0.31579f, 0.21053f, 0.15789f, 0.10526f, 0.10526f, 0.05263f, 0.02632f, 0.02632f,
  /* Row 6 */
  0.02632f, 0.03947f, 0.05263f, 0.10526f, 0.21053f, 0.31579f, 0.42105f, 0.63158f,
  0.63158f, 0.42105f, 0.31579f, 0.21053f, 0.10526f, 0.05263f, 0.03947f, 0.02632f,
  /* Row 7 */
  0.02632f, 0.05263f, 0.10526f, 0.15789f, 0.21053f, 0.42105f, 0.84211f, 0.89474f,
  0.89474f, 0.84211f, 0.42105f, 0.21053f, 0.15789f, 0.10526f, 0.05263f, 0.02632f,
  /* Row 8 */
  0.02632f, 0.05263f, 0.10526f, 0.21053f, 0.42105f, 0.63158f, 0.84211f, 1.00000f,
  1.00000f, 0.84211f, 0.63158f, 0.42105f, 0.21053f, 0.10526f, 0.05263f, 0.02632f,
  /* Row 9 */
  0.02632f, 0.05263f, 0.10526f, 0.21053f, 0.42105f, 0.63158f, 0.84211f, 1.00000f,
  1.00000f, 0.84211f, 0.63158f, 0.42105f, 0.21053f, 0.10526f, 0.05263f, 0.02632f,
  /* Row 10 */
  0.02632f, 0.05263f, 0.10526f, 0.15789f, 0.21053f, 0.42105f, 0.84211f, 0.89474f,
  0.89474f, 0.84211f, 0.42105f, 0.21053f, 0.15789f, 0.10526f, 0.05263f, 0.02632f,
  /* Row 11 */
  0.02632f, 0.03947f, 0.05263f, 0.10526f, 0.21053f, 0.42105f, 0.63158f, 0.84211f,
  0.84211f, 0.63158f, 0.42105f, 0.21053f, 0.10526f, 0.05263f, 0.03947f, 0.02632f,
  /* Row 12 */
  0.02632f, 0.02632f, 0.05263f, 0.10526f, 0.21053f, 0.21053f, 0.31579f, 0.42105f,
  0.42105f, 0.31579f, 0.21053f, 0.21053f, 0.10526f, 0.05263f, 0.02632f, 0.02632f,
  /* Row 13 */
  0.02632f, 0.02632f, 0.05263f, 0.07895f, 0.10526f, 0.15789f, 0.21053f, 0.21053f,
  0.21053f, 0.21053f, 0.15789f, 0.10526f, 0.07895f, 0.05263f, 0.02632f, 0.02632f,
  /* Row 14 */
  0.02632f, 0.02632f, 0.02632f, 0.03947f, 0.05263f, 0.10526f, 0.15789f, 0.21053f,
  0.21053f, 0.15789f, 0.10526f, 0.05263f, 0.03947f, 0.02632f, 0.02632f, 0.02632f,
  /* Row 15 */
  0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.03947f, 0.05263f, 0.07895f, 0.10526f,
  0.10526f, 0.07895f, 0.05263f, 0.03947f, 0.02632f, 0.02632f, 0.02632f, 0.02632f,
  /* Row 16 */
  0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.03947f, 0.03947f,
  0.03947f, 0.03947f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f
  },
  /* 16.:9 Aspect Ratio */
  {
  /* Row 1 */
  0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f,
  0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f,
  /* Row 2 */
  0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.03947f, 0.05263f, 0.05263f,
  0.05263f, 0.05263f, 0.03947f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f,
  /* Row 3 */
  0.02632f, 0.02632f, 0.02632f, 0.03947f, 0.05263f, 0.05263f, 0.07895f, 0.10526f,
  0.10526f, 0.07895f, 0.05263f, 0.05263f, 0.03947f, 0.02632f, 0.02632f, 0.02632f,
  /* Row 4 */
  0.02632f, 0.02632f, 0.03947f, 0.05263f, 0.07895f, 0.10526f, 0.10526f, 0.15789f,
  0.15789f, 0.10526f, 0.10526f, 0.07895f, 0.05263f, 0.03947f, 0.02632f, 0.02632f,
  /* Row 5 */
  0.02632f, 0.02632f, 0.05263f, 0.10526f, 0.10526f, 0.15789f, 0.21053f, 0.31579f,
  0.31579f, 0.21053f, 0.15789f, 0.10526f, 0.10526f, 0.05263f, 0.02632f, 0.02632f,
  /* Row 6 */
  0.02632f, 0.03947f, 0.05263f, 0.10526f, 0.21053f, 0.31579f, 0.42105f, 0.63158f,
  0.63158f, 0.42105f, 0.31579f, 0.21053f, 0.10526f, 0.05263f, 0.03947f, 0.02632f,
  /* Row 7 */
  0.02632f, 0.05263f, 0.10526f, 0.15789f, 0.21053f, 0.42105f, 0.84211f, 0.89474f,
  0.89474f, 0.84211f, 0.42105f, 0.21053f, 0.15789f, 0.10526f, 0.05263f, 0.02632f,
  /* Row 8 */
  0.02632f, 0.05263f, 0.10526f, 0.21053f, 0.42105f, 0.63158f, 0.84211f, 1.00000f,
  1.00000f, 0.84211f, 0.63158f, 0.42105f, 0.21053f, 0.10526f, 0.05263f, 0.02632f,
  /* Row 9 */
  0.02632f, 0.05263f, 0.10526f, 0.21053f, 0.42105f, 0.63158f, 0.84211f, 1.00000f,
  1.00000f, 0.84211f, 0.63158f, 0.42105f, 0.21053f, 0.10526f, 0.05263f, 0.02632f,
  /* Row 10 */
  0.02632f, 0.05263f, 0.10526f, 0.15789f, 0.21053f, 0.42105f, 0.84211f, 0.89474f,
  0.89474f, 0.84211f, 0.42105f, 0.21053f, 0.15789f, 0.10526f, 0.05263f, 0.02632f,
  /* Row 11 */
  0.02632f, 0.03947f, 0.05263f, 0.10526f, 0.21053f, 0.42105f, 0.63158f, 0.84211f,
  0.84211f, 0.63158f, 0.42105f, 0.21053f, 0.10526f, 0.05263f, 0.03947f, 0.02632f,
  /* Row 12 */
  0.02632f, 0.02632f, 0.05263f, 0.10526f, 0.21053f, 0.21053f, 0.31579f, 0.42105f,
  0.42105f, 0.31579f, 0.21053f, 0.21053f, 0.10526f, 0.05263f, 0.02632f, 0.02632f,
  /* Row 13 */
  0.02632f, 0.02632f, 0.05263f, 0.07895f, 0.10526f, 0.15789f, 0.21053f, 0.21053f,
  0.21053f, 0.21053f, 0.15789f, 0.10526f, 0.07895f, 0.05263f, 0.02632f, 0.02632f,
  /* Row 14 */
  0.02632f, 0.02632f, 0.02632f, 0.03947f, 0.05263f, 0.10526f, 0.15789f, 0.21053f,
  0.21053f, 0.15789f, 0.10526f, 0.05263f, 0.03947f, 0.02632f, 0.02632f, 0.02632f,
  /* Row 15 */
  0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.03947f, 0.05263f, 0.07895f, 0.10526f,
  0.10526f, 0.07895f, 0.05263f, 0.03947f, 0.02632f, 0.02632f, 0.02632f, 0.02632f,
  /* Row 16 */
  0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.03947f, 0.03947f,
  0.03947f, 0.03947f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f, 0.02632f
  }
};

static float aec_spot_metering_adv[2][256] = {
  { /* 4:3 Aspect Ratio */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 1 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 2 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 3 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 4 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 5 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 6 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 7 */
/* Row-wise, start of the inner region */
  0, 0, 0, 0, 0, 0, 0, W2, W2, 0, 0, 0, 0, 0, 0, 0, /* row 8 */
  0, 0, 0, 0, 0, 0, 0, W2, W2, 0, 0, 0, 0, 0, 0, 0, /* row 9 */
/* Row-wise, end of the inner region */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 10 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 11 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 12 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 13 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 14 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 15 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  /* row 16 */
  },
  { /* 16:9 Aspect Ratio */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 1 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 2 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 3 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 4 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 5 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 6 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 7 */
  /* Row-wise, start of the inner region */
  0, 0, 0, 0, 0, 0, 0, W2, W2, 0, 0, 0, 0, 0, 0, 0, /* row 8 */
  0, 0, 0, 0, 0, 0, 0, W2, W2, 0, 0, 0, 0, 0, 0, 0, /* row 9 */
  /* Row-wise, end of the inner region */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 10 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 11 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 12 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 13 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 14 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* row 15 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  /* row 16 */
  }
};

static float aec_center_weighted_8x8[64] = {/* Q8 format */
  W1, W1, W1, W1, W1, W1, W1, W1,
  W1, W1, W1, W1, W1, W1, W1, W1,
/* Row-wise, start of the inner region */
  W1, W1, W2, W2, W2, W2, W1, W1,
  W1, W1, W2, W2, W2, W2, W1, W1,
  W1, W1, W2, W2, W2, W2, W1, W1,
  W1, W1, W2, W2, W2, W2, W1, W1,
/* Row-wise, end of the inner region */
  W1, W1, W1, W1, W1, W1, W1, W1,
  W1, W1, W1, W1, W1, W1, W1, W1
};

static float aec_center_weighted_4x4[16] = {/* Q8 format */
  W1, W1, W1, W1,
/* Row-wise, start of the inner region */
  W1, W2, W2, W1,
  W1, W2, W2, W1,
/* Row-wise, end of the inner region */
  W1, W1, W1, W1,
};
/* Spot-Metering tables: Formula to apply to each entry:
 * ((total num entries 64 or 256) * (weight for rgn)) / num entries in rgn
 * = ((64*0.0)/60)=0.0 [ for outer rg with 48 pts and 40% of the weighting]
 * =  ((64*1.0) / 4)  = 16.0     [ for inner region with 16 points and 60%
 * of the weighting] Verify these nubers by (60 * 0.0) + (16 * 4) = 64*/
static float aec_spot_metering_8x8[64] = {/* Q8 format */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0, /* Row-wise, start of the inner region */
  0, 0, 0, 1, 1, 0, 0, 0, /* Row-wise, end of the inner region */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};


static float aec_spot_metering_4x4[16] = { /* Q8 format */
  0, 0, 0, 0,
  0, 1, 1, 0, /* Row-wise, start of the inner region */
  0, 1, 1, 0, /* Row-wise, end of the inner region */
  0, 0, 0, 0
};

typedef enum {
  STROBE_OFF,
  STROBE_PRE_ON,
  STROBE_CHECK_READY,
  STROBE_PRE_FLASH,
} strobe_internal_state_t;

typedef enum {
  AEC_SETTLED = 1,
  AEC_SETTLE_WAIT,
  AEC_FLASH_SETTLE_WAIT,
} aec_settle_t;

typedef struct {
  int enable;
  uint32_t number; /* todo rename ask marvin */
  int index[INTERESTED_SUB_REGN_MAX_NUM];
} aec_sub_region_of_interest_t;

typedef struct {
  /*stats when pre-flash is on */
  int   luma_on;

  /*stats when pre-flash is off */
  int   luma_off;
  int   index_off;
  float lux_idx_off;

  int   exp_index_adjustment;
  uint32_t linecount_off;
  float real_gain_off;
  float strb_gain;
} aec_strobe_flash_est_t;

typedef struct {
  int   luma_on_100;
  int   index_on_100;
  int   luma_off;
  int   index_off;
  float lux_idx_off;
} aec_led_flash_est_t;

typedef struct {
  int min_numerator_val;
  int min_denominator_val;
  int max_numerator_val;
  int max_denominator_val;
  int step_numerator_val;
  int step_denominator_val;
  int default_numerator_val;
  int default_denominator_val;
  int val_to_get_index;
  float *ev_ptr;    /* EV Compensation table */
} aec_exposure_compensation_t;

typedef struct {
  uint32_t   new_line_count;
  uint32_t hjr_af_line_count;
  float new_sensor_gain;
  float hjr_af_gain;
} aec_hjr_data_t;

typedef struct {
  float mtn;
  int motion_up;
  int frame_cnt;
} aec_m_iso_test_t;

typedef struct {
  int cur_num;
  int fr_cnt; /*for timing*/
} aec_ROI_test_t;

typedef struct {
  int                            iso;
  camera_auto_exposure_mode_type metering_type;
  camera_bestshot_mode_type      curr_mode;
  int                            exp_comp_val;
} aec_bestshot_data_t;

typedef struct { /* Flash/LED Params */
  int strb_frame_cnt_wait;
  int led_compensation_disabled;
  aec_strobe_flash_est_t strb_est;
  aec_led_flash_est_t led_est;
  int strobe_mode;
  strobe_internal_state_t strb_int_state;
} aec_flash_t;

typedef struct { /* Luma Params */
  int lux_index;
  int array[MAX_LUMA_ARRAY_INDEX + 1];
  int index;

  uint32_t target;  /*luma target compensated for indoor-outdoor only*/
  uint32_t comp_target;
  uint32_t tolerance;      /* Tolerance + and - the target */
  uint32_t previous_sumArray[256];
} aec_luma_data_t;

typedef struct { /* Motion  Params */
  int status; /* todo revisit never set to ON */
  float iso_snap_real_gain;
  float tmp_val;
  float val; /* todo change name to preview motion */
  float snap_val;
  int apply;
  int frame;
  float array[MOTION_ARRAY_SIZE];
} aec_motion_t;

typedef struct {
  int      test_enable;
  int      test_roi;
  int      test_motion;
  int      disable;
  int      lock_output;

  int      force_exp;
  uint32_t force_linecount;
  float    force_gain;

  int      force_snapshot_exp;
  uint32_t force_snap_linecount;
  float    force_snap_gain;

  float    stored_gain;
  uint32_t stored_line_count;
} aec_ez_tune_t;

typedef struct {
  int exp_increase;
  int af_hjr_frame_skip_count;
  int snap_est_lock;
  int update_hw;
  int next_update_frame_cnt; /* todo: should be renaned done: need to revist */

  float *bias_table;
  float *prev_bias_table;
  camera_auto_exposure_mode_type prev_metering_type;
  uint32_t settle_frame_cnt;
  uint32_t sensor_update_ok;
  uint32_t redeye_reduction;

  int outdoor_bright_rgn_discard_thld; /* todo revisit dark rgn variables*/
  float dark_reduction0;
  float dark_reduction1;
  float dark_reduction2;
  float dark_reduction3;
  float dark_rgn_discard_thld;
  float dark_rgn_TH_HI_outdoor_index_ratio;
  float dark_rgn_TH_HI_for_outdoor;

  aec_exposure_compensation_t exp_comp;

  /*compensated indoor for EV & brightness*/
  int default_luma_target_compensated;
  int outdoor_luma_target_compensated;

  /* ROI Params*/
  aec_sub_region_of_interest_t sub_roi;
  stats_proc_interested_region_t roi;
  uint32_t fd_adjusted_luma;

  int hjr_af_lock_cnt;

  int reach_target_before; /* todo rename */
  int frame_in_current_fps; /* todo rename */
  int frame_skip;           /* todo revisit not used correctly */
  uint32_t num_exp_tbl_for_min_fps; /* todo revisit not used correctly */
  float motion; /* todo revisit should be set to 1? */
  uint32_t min_fps; /* todo revisit not used correctly */
  uint32_t fps;

  int asd_luma_offset; /* ASD params */
  int afr_frame_rate;  /* AFR params */
  int fast_conv_speed; /* Q8 Number: Fast Convergence Params */
  exposure_entry_type *exp_tbl_ptr;
  unsigned short valid_entries;
  int exp_comp_val;
  aec_luma_data_t luma;   /* Luma Params */
  aec_motion_t mtn;
  stats_proc_antibanding_type antibanding;  /* AFD  Params */
  aec_slow_conv_t        slow_c;
  aec_hjr_data_t         hjr_data;  /* HJR */
  aec_flash_t            flash;
  aec_m_iso_test_t       m_iso_test;
  aec_ROI_test_t         roi_test;
  aec_bestshot_data_t    bestshot_d;
  aec_ez_tune_t          eztune; /* EZTune Set Params */
  stats_proc_mtr_area_t  aec_mtr_area;
  int                    aec_lock;
  int                    current_luma_target;
  int                    led_off_SY[256];
  float                  led_low_bias_table[256];
} aec_t;

int aec_process_preview_and_video(stats_proc_t *sproc, aec_t *aec);
int aec_process_snapshot(stats_proc_t *sproc, aec_t *aec);

/* UTIL MISC APIS*/
int aec_init_data(stats_proc_t *sproc, aec_t *aec);
int aec_load_chromatix(stats_proc_t *sproc, aec_t *aec);
void aec_preview_antibanding(stats_proc_t *sproc, aec_t *aec);
void aec_adjust_exp_settings_for_led(stats_proc_t *sproc, aec_t *aec);
void aec_fast_conv_config(stats_proc_t *sproc, aec_t *aec);
void aec_est_strobe_flash_for_snapshot(stats_proc_t *sproc, aec_t *aec);
void aec_strobe_flash_store_est(stats_proc_t *sproc, aec_t *aec,
  strobe_internal_state_t strb_state);
void aec_set_full_frame_exp(stats_proc_t *sproc, aec_t *aec);
int aec_strobe_enable_check(void);
int aec_use_strobe(stats_proc_t *sproc, aec_t *aec);

/* UTIL SET APIs */
int aec_set_exp_metering_mode(stats_proc_t *sproc, aec_t *aec,
  camera_auto_exposure_mode_type aec_metering);
int aec_set_iso_mode(stats_proc_t *sproc,  aec_t *aec, int iso);
int aec_set_antibanding(stats_proc_t *sproc,  aec_t *aec,
  const stats_proc_antibanding_type antibanding);
int aec_set_antibanding_status(stats_proc_t *sproc,  aec_t *aec,
  const int status);
int aec_set_brightness_level(stats_proc_t *sproc,  aec_t *aec, int brightness);
int aec_set_exposure_compensation(stats_proc_t *sproc,  aec_t *aec,
  uint32_t aec_exp_comp);
int aec_set_fps_mode(stats_proc_t *sproc,  aec_t *aec, stats_proc_fps_mode_type fps_mode);
int aec_set_parm_fps(stats_proc_t *sproc,  aec_t *aec, int fps);
int aec_set_hjr(stats_proc_t *sproc, aec_t *aec);
int aec_set_for_hjr_af(stats_proc_t *sproc,  aec_t *aec, int af_hjr);
int aec_set_ROI(stats_proc_t *sproc,  aec_t *aec, stats_proc_interested_region_t aec_roi);
int aec_reset_LED(stats_proc_t *sproc,  aec_t *aec);
int aec_store_LED_est_stats(stats_proc_t *sproc,  aec_t *aec,
  uint32_t led_state);
int aec_set_strobe_mode(stats_proc_t *sproc,  aec_t *aec,
  int strobe_mode);
int aec_prepare_snapshot(stats_proc_t *sproc, aec_t *aec);
int aec_set_bestshot_mode(stats_proc_t *sproc, aec_t *aec,
  camera_bestshot_mode_type new_mode);
float aec_calc_sensitivity(stats_proc_t *sproc, float gain,
  uint32_t line_count);
void aec_update_exp_idx(stats_proc_t *sproc, aec_t *aec);

/* UTIL GET APIs */
int aec_get_settled_cnt(stats_proc_t *sproc,  aec_t *aec);
int aec_get_LED_over_exp_check(stats_proc_t *sproc, aec_t *aec);
int aec_get_strobe(stats_proc_t *sproc, aec_t *aec);
int aec_get_flash_for_snapshot(stats_proc_t *sproc,  aec_t *aec);
float aec_get_preview_fps(stats_proc_t *sproc, aec_t *aec);
float aec_get_preview_exp_time(stats_proc_t *sproc,  aec_t *aec);
void aec_util_calculate_led_low_bias_table(stats_proc_t *sproc, aec_t *aec);
int aec_calculate_current_luma(stats_proc_t *sproc, aec_t *aec);
/* SLOW CONVERGENCE */
void aec_slow_convergence(stats_proc_t *sproc, aec_t *aec,
    uint32_t current_vfe_luma);
#endif /* __AEC_H__ */
