/* afd.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AFD_H__
#define __AFD_H__

#include "q3a_stats.h"
#include "aec.h"

#define CAFD_DELAY 120

typedef enum {
  AFD_STATUS_OFF = 0,
  AFD_STATUS_REGULAR_EXPOSURE_TABLE,   /* 1 */
  AFD_STATUS_60HZ_EXPOSURE_TABLE,      /* 2 */
  AFD_STATUS_50HZ_EXPOSURE_TABLE,      /* 3 */
  AFD_STATUS_50HZ_AUTO_EXPOSURE_TABLE, /* 4 */
  AFD_STATUS_60HZ_AUTO_EXPOSURE_TABLE, /* 5 */
} afd_status_t;

typedef enum {
#if 1
  AFD_TYPE_OFF,
  AFD_TYPE_60HZ,
  AFD_TYPE_50HZ,
  AFD_TYPE_AUTO,
  AFD_TYPE_AUTO_50HZ,
  AFD_TYPE_AUTO_60HZ,
  AFD_TYPE_MAX,
#else
  CAMERA_ANTIBANDING_OFF,
  CAMERA_ANTIBANDING_60HZ,
  CAMERA_ANTIBANDING_50HZ,
  CAMERA_ANTIBANDING_AUTO,
  CAMERA_ANTIBANDING_AUTO_50HZ,
  CAMERA_ANTIBANDING_AUTO_60HZ,
  CAMERA_MAX_ANTIBANDING,
#endif
} afd_type_t;

typedef enum {
  AFD_STATE_INIT = 0,
  AFD_STATE_ACTIVE,
  AFD_STATE_INACTIVE,
  AFD_STATE_INVALID_STATE,
  AFD_STATE_OFF,
  AFD_STATE_ON,
  AFD_STATE_BUSY,
  AFD_STATE_DONE,
} afd_state_t;

typedef struct {
  boolean trigger;
  int frame_skip_cnt;
  int status;
  int conti_afd_delay;
} afd_conti_t;

typedef enum {
  AFD_SET_PARAM_INIT_CHROMATIX   = 1,
  AFD_SET_AEC_PARAM,
  AFD_SET_SENSOR_PARAM,
  AFD_SET_AF_PARAM,
  AFD_SET_ENABLE,
  AFD_SET_RESET,
  AFD_SET_STATS_DEBUG_MASK,
  AFD_SET_PARAM_MAX
} afd_set_parameter_type;

typedef struct {
  void *chromatix;
} afd_set_parameter_init_t;

typedef struct {
  boolean afd_enable;
  afd_type_t afd_mode;
} afd_set_enable_t;

/* Below should come from AEC update */
typedef struct {
  boolean aec_settled;

  int  max_line_cnt;

  uint32_t preview_fps;
  uint32_t max_preview_fps;
  uint32_t cur_line_cnt;
  float    band_50hz_gap;
  afd_type_t  aec_atb;
  uint32_t preview_linesPerFrame;
  uint32_t sen_dim_height;

  boolean  af_fixed_lens;

  boolean  af_active;
  boolean  cont_af_enabled;
} afd_data_from_aec_and_af_t;

typedef struct _afd_set_parameter {
  afd_set_parameter_type type;

  union {
    afd_set_parameter_init_t   init_param;
    afd_set_enable_t           set_enable;
    afd_data_from_aec_and_af_t aec_af_data;
    uint32_t                   stats_debug_mask;
  } u;
} afd_set_parameter_t;

typedef enum {
  AFD_GET_PARAMS,
  AFD_GET_MAX
} afd_get_parameter_type;

typedef struct {
  unsigned int afd_todo_params;
} afd_get_params_t;

typedef struct _afd_get_parameter {
  afd_get_parameter_type type;

  union {
    afd_get_params_t afd_params;
  } u;
} afd_get_parameter_t;

typedef struct {
  afd_data_from_aec_and_af_t aec_af_data;
} afd_process_data_t;

/** asd_output_data
 *
 **/
typedef struct {
  boolean     afd_enable;
  boolean     afd_monitor;
  boolean     afd_exec_once;
  boolean     flicker_detect; /* afd->flicker */
  boolean     eztune_enabled;

  int flicker_freq;
  int actual_peaks;
  int multiple_peak_algo;     /* afd->num_peaks */
  int std_width;

  afd_state_t  afd_state;
  afd_status_t afd_status;
  afd_type_t   afd_atb;
} afd_output_data_t;

void *afd_init(void);
int afd_set_parameters(afd_set_parameter_t *param,
  void *afd,afd_output_data_t *output);
boolean afd_get_parameters(afd_get_parameter_t *param, void *afd);
boolean afd_process(stats_t *stat, void *afd, afd_output_data_t *output);
void afd_destroy(void *afd);

#endif /* __AFD_H__ */
