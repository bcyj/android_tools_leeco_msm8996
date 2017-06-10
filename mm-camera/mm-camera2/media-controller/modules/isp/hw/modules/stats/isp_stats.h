/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __ISP_STATS_H__
#define __ISP_STATS_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "isp_buf_mgr.h"
#include "q3a_stats.h"

#define ISP_STATS_MAX_BUFS         8

typedef enum {
  ISP_STATS_STREAM_STATE_NOTUSED,  /* not used yet */
  ISP_STATS_STREAM_STATE_INITTED,  /* initted */
  ISP_STATS_STREAM_STATE_REGGED,   /* configured and buffer regged */
  ISP_STATS_STREAM_STATE_ACTIVE,   /* active */
  ISP_STATS_STREAM_STATE_STOPPING,
  ISP_STATS_STREAM_STATE_MAX,
} isp_stats_stream_state_t;

typedef struct {
  enum msm_isp_stats_type stats_type;
  void *reg_cmd;
  void *current_reg_cmd;
  void *new_reg_cmd;
  uint8_t hw_update_pending;
  uint8_t trigger_enable; /* enable trigger update feature flag */
  uint8_t enable;         /* enable flag from PIX */
  uint8_t is_used;        /* is this entry used or not */
  isp_stats_stream_state_t state;
  uint32_t stream_handle;
  uint32_t session_id;
  uint8_t is_first;
  cam_hfr_mode_t hfr_mode;
  int comp_flag;
  uint8_t num_bufs;
  int fd;
  int ion_fd;
  uint32_t buf_len;
  uint32_t buf_handle;
  uint32_t buf_offset;
  uint32_t is_ispif_split;
  uint32_t num_left_rgns;
  uint32_t num_right_rgns;
  isp_ops_t ops;                /* ops for stats cfg and stream mgt */
  isp_notify_ops_t *notify_ops; /* notify ops to stats module */
  void *parsed_stats_buf;
  void *saved_stats_buf;        /* saved prevous stats */
  boolean has_saved_stats;      /* has saved  stats */
  boolean need_to_do_fullsize_cfg;
  boolean is_tintless_stats_configured;
  boolean is_fullsize_stats;
  boolean is_current_stats_fullsize;
  boolean is_new_stats_full_size;
  boolean roi_config_skip_stats;
  boolean tinltess_cofig_stats;
  boolean new_stats_config;
  uint32_t len_parsed_stats_buf;
  uint32_t hnum;
  uint32_t vnum;
  void *private;
  int dev_idx;
  void *buf_mgr;
  uint8_t skip_stats;
}isp_stats_entry_t;

typedef enum {
  ISP_STATS_SET_INVALID,       /* not used enum */
  ISP_STATS_SET_ENABLE,        /* reserve module */
  ISP_STATS_SET_CONFIG,        /* config module */
  ISP_STATS_SET_TRIGGER_ENABLE,/* trigger update reg settings */
  ISP_STATS_SET_TRIGGER_UPDATE, /* trigger update reg settings */
  ISP_STATS_SET_STREAM_CFG,    /* isp_hw_pix_setting_params_t */
  ISP_STATS_SET_STREAM_UNCFG,  /* isp_pix_trigger_update_input_t */
  ISP_STATS_SET_MAX_NUM        /* max set param num */
} isp_stats_set_param_id_t;

typedef enum {
  ISP_STATS_GET_INVALID,       /* not used enum */
  ISP_STATS_GET_ENABLE,        /* enable module */
  ISP_STATS_GET_STREAM_HANDLE, /* fetch stream state */
  ISP_STATS_GET_STREAM_STATE,  /* fetch stream state */
  ISP_STATS_GET_PARSED_STATS,  /* parse the stats */
  ISP_STATS_GET_RS_CONFIG,     /* uint32_t */
  ISP_STATS_GET_CS_CONFIG,     /* uint32_t */
  ISP_STATS_GET_MAX_NUM        /* max get param num */
} isp_stats_get_param_id_t;

typedef enum {
  ISP_STATS_ACTION_INVALID,       /* invalid action code */
  ISP_STATS_ACTION_STREAM_BUF_CFG,  /* buf config */
  ISP_STATS_ACTION_STREAM_BUF_UNCFG,  /* buf unconfig */
  ISP_STATS_ACTION_STREAM_START,  /* do hw reg update */
  ISP_STATS_ACTION_STREAM_STOP,   /* do hw reg update */
  ISP_STATS_ACTION_HW_CFG_UPDATE, /* update stats module hw configuration */
  ISP_STATS_ACTION_STATS_PARSE,
  ISP_STATS_ACTION_RESET,         /* reset stats sub modules */
  ISP_STATS_ACTION_MAX_NUM,       /* max num action codes */
}isp_stats_action_code_t;

typedef struct {
  int fd;
  uint32_t stats_max_mask; /* what the hw can support */
  uint32_t stats_cfg_mask; /* what statistics asked to use */
  uint32_t stats_enable_mask; /* what are enabled */
  uint32_t session_id;
  isp_ops_t *stats_ops[MSM_ISP_STATS_MAX];
  uint8_t trigger_enable; /* enable trigger update feature flag */
  uint8_t enable;         /* enable flag from PIX */
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  void *buf_mgr;
  int dev_idx;
  uint32_t stats_burst_len;
  pthread_mutex_t  parse_stats_mutex;
  uint32_t isp_version;
}isp_stats_mod_t;

int isp_stats_enqueue_buf(isp_stats_entry_t *entry, int buf_idx);
int isp_stats_config_stats_stream(isp_stats_entry_t *entry, int num_bufs);
int isp_stats_unconfig_stats_stream(isp_stats_entry_t *entry);
int isp_stats_start_streams(isp_stats_mod_t *mod,
  uint32_t start_mask);
int isp_stats_stop_streams(
  isp_stats_mod_t *mod,
  uint32_t stats_mask);
int isp_stats_parse(isp_stats_mod_t *mod,
  isp_pipeline_stats_parse_t *action_data);
void isp_stats_reset(isp_stats_entry_t *entry);
int isp_stats_do_reset(isp_stats_mod_t *mod);
#endif /*__ISP_STATS_H__*/
