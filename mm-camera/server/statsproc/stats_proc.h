/*============================================================================
  Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/
#ifndef __STATS_PROC_H__
#define __STATS_PROC_H__

#ifdef _ANDROID_
#include <utils/Log.h>
#endif
#include "stats_proc_interface.h"
#include "camera_dbg.h"

#define STATS_PROC_DEBUG     0

#define AEC_DEBUG_HIGH       0
#define AEC_DEBUG_LOW        0

#define AWB_DEBUG_HIGH       0
#define AWB_DEBUG_LOW        0

#define AF_DEBUG_HIGH        0
#define AF_DEBUG_LOW         0

#define ASD_DEBUG_HIGH       0
#define ASD_DEBUG_LOW        0

#define AFD_DEBUG_HIGH       0
#define AFD_DEBUG_LOW        0

#define STATS_PROC_DIS       0

#undef CDBG_HIGH
#if(STATS_PROC_DEBUG)
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-STATSPROC"
  #define CDBG_STATS_PROC(fmt, args...) LOGE(fmt, ##args)
  #define CDBG_HIGH(fmt, args...) LOGE(fmt, ##args)
#else
  #define CDBG_STATS_PROC(fmt, args...) do{}while(0)
  #define CDBG_HIGH(fmt, args...) do{}while(0)
#endif

#define MAX_INSTANCES 8

#define AEC_SETTLE_MAX_FRAME_CNT 15

#define STATS_PROC_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define STATS_PROC_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define FLOAT_TO_Q(exp, f) \
  ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))
#define STATS_PROC_CAP_UINT8(a) (((a<0)||(a>255)) ? ((a<0)?0:255):a);
#define STATS_PROC_CAP(a, b, c) (((a < b)||(a  > c)) ? (( a < b)? b: c):a);

/********************************
     STATS_PROC MAIN Contol Block
*********************************/
typedef struct {
  stats_proc_roi_info_t      fd_roi;

  /* AEC 3A Shared data */
  int      prev_exp_index;
  uint32_t luma_settled_cnt;
  uint32_t cur_af_luma;
  /* AWB 3A Shared data */
  uint32_t grey_world_stats;
  uint32_t awb_asd_sync_flag;
  int      stat_sample_decision[64];
  int      stat_index_mapping[64];
  /* AWB 3A Shared data */
  uint32_t backlight_luma_target_offset;
  uint32_t snow_or_cloudy_luma_target_offset;
  /* AFD 3A Shared data */
  int afd_enable;
  int afd_monitor;
  stats_proc_antibanding_type  afd_atb;
  int afd_exec_once;
  int afd_status;  /* -1 means not sure, 0 means no flicker; 60, or 50. */
  int hjr_af_enabled;
  /* EZ TUNE Shared data */
  int eztune_enable;
  /* MOBICAT Shared data */
  int mobicat_enable;

  /* Externally shared data */
  stats_proc_aec_data_t    aec_ext;
  stats_proc_awb_data_t    awb_ext;
  stats_proc_af_data_t     af_ext;
  stats_proc_asd_data_t    asd_ext;
  stats_proc_dis_data_t    dis_ext;
  stats_proc_afd_data_t    afd_ext;
} stats_proc_shared_data_t;

typedef struct {
  uint32_t handle;
  stats_proc_interface_input_t input;
  stats_proc_shared_data_t share;
  uint32_t obj_idx_mask;
  uint8_t client_idx;
  uint8_t my_comp_id;
  uint32_t vfe_version;
  mctl_ops_t *ops;
} stats_proc_t;

#define STATSPROC_MAX_CLIENT_NUM    2

typedef struct {
  pthread_mutex_t mutex;
  uint32_t statsproc_handle_cnt;
  stats_proc_t client[STATSPROC_MAX_CLIENT_NUM];
} statsproc_comp_root_t;

/********************************
     AEC Interface APIs
*********************************/
int  aec_init(stats_proc_t *sproc);
int  aec_get_params(stats_proc_t *sproc, stats_proc_get_aec_data_t *get_aec);
int  aec_set_params(stats_proc_t *sproc, stats_proc_set_aec_data_t *set_aec);
int  aec_process(stats_proc_t *sproc);
void aec_deinit(stats_proc_t *sproc);
void aec_chromatix_reload(stats_proc_t *sproc);

/********************************
     AWB Interface APIs
*********************************/
int  awb_init(stats_proc_t *sproc);
int  awb_get_params(stats_proc_t *sproc, stats_proc_get_awb_data_t *get_awb);
int  awb_set_params(stats_proc_t *sproc, stats_proc_set_awb_data_t *set_awb);
int  awb_process(stats_proc_t *sproc);
void awb_deinit(stats_proc_t *sproc);
void awb_chromatix_reload(stats_proc_t *sproc);

/********************************
     AF Interface APIs
*********************************/
int  af_init(stats_proc_t *sproc);
int  af_get_params(stats_proc_t *sproc, stats_proc_get_af_data_t *get_af);
int  af_set_params(stats_proc_t *sproc, stats_proc_set_af_data_t *set_af);
int  af_process(stats_proc_t *sproc);
void af_deinit(stats_proc_t *sproc);
void af_chromatix_reload(stats_proc_t *sproc);

/********************************
     AF Interface APIs
*********************************/
int  asd_init(stats_proc_t *sproc);
int  asd_get_params(stats_proc_t *sproc, stats_proc_get_asd_data_t *get_asd);
int  asd_set_params(stats_proc_t *sproc, stats_proc_set_asd_data_t *set_asd);
int  asd_process(stats_proc_t *sproc);
void asd_deinit(stats_proc_t *sproc);
void af_deinit(stats_proc_t *sproc);

/*******************************
 IS APIs
*******************************/
int  is_if_init(stats_proc_t *sproc);
void is_if_deinit(stats_proc_t *sproc);
int  is_process(stats_proc_t *sproc, uint32_t frame_id);

/*******************************
 DIS APIs
*******************************/
int  dis_if_init(stats_proc_t *sproc);
int  dis_get_params(stats_proc_t *sproc, stats_proc_get_dis_data_t *get_dis);
int  dis_set_params(stats_proc_t *sproc, stats_proc_set_dis_data_t *set_dis);
int  dis_process(stats_proc_t *sproc);
void dis_if_deinit(stats_proc_t *sproc);

/*******************************
 EIS APIs
*******************************/
int  eis_if_init(stats_proc_t *sproc);
void eis_if_deinit(stats_proc_t *sproc);
int  eis_process(stats_proc_t *sproc);
int  eis_set_params(stats_proc_t *sproc, stats_proc_set_dis_data_t *data);

/*******************************
 AFD APIs
*******************************/
int  afd_init(stats_proc_t *sproc);
int  afd_get_params(stats_proc_t *sproc, stats_proc_get_afd_data_t *get_afd);
int  afd_set_params(stats_proc_t *sproc, stats_proc_set_afd_data_t *set_afd);
int  afd_process(stats_proc_t *sproc);
void afd_deinit(stats_proc_t *sproc);
#endif /* __STATS_PROC_H__ */
