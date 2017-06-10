/*============================================================================
Copyright (c) 2013, 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_TINTLESS_H__
#define __ISP_TINTLESS_H__

#include "isp_def.h"
#include "tintless_interface.h"
#include "q3a_stats_hw.h"

typedef enum {
  ISP_TINTLESS_STATS_TYPE_INVALID,
  ISP_TINTLESS_STATS_TYPE_BE,
  ISP_TINTLESS_STATS_TYPE_BG,
  ISP_TINTLESS_STATS_TYPE_MAX,
} isp_tintless_stats_type_t;

typedef struct {
  uint32_t isp_version;
  tintless_version_t version;           // tintless version for core
  uint8_t is_supported;
  uint8_t is_enabled;
  isp_tintless_stats_type_t stats_support_type;
} isp_tintless_data_t;

typedef struct {
  pthread_t pid;
  sem_t wait_sem; /* thread waits on this semphore */
  uint8_t thread_exit;
  pthread_mutex_t  lock_mutex;
  uint8_t algo_requested;                      // 1 - need processing, 0 skip it

  uint32_t in_frame_id;
  tintless_stats_t mod_stats;
} isp_tintless_thread_data_t;

typedef struct {
  void *tintless;
  uint32_t session_id;

  tintless_mesh_rolloff_array_t mesh_fixed;   // fixed table to tintless
  tintless_mesh_rolloff_array_t mesh_hw;      // table going to hw
  tintless_mesh_rolloff_array_t curr_rolloff_hw;  // table for current pca rolloff
  uint32_t frame_id;
  uint32_t ready_frame_id;

  uint8_t update_stats;                       // flag for updating stats
  uint8_t update_chromatix;                   // flag for update chromatix tintless params
  uint8_t update_mesh;                        // flag for updating mesh rolloff

  void *tinstance;                            // pointer for tintless to maintain session info for multiple VFE
  isp_tintless_thread_data_t thread_data;
} isp_tintless_session_t;

typedef struct {
  isp_tintless_data_t tintless_data;
  isp_tintless_session_t sessions[ISP_MAX_SESSIONS];
} isp_tintless_t;

typedef struct {
  uint32_t session_id;
  void *notify_data;
  uint32_t notify_data_size;
} isp_tintless_notify_data_t;

typedef struct {
  tintless_mesh_config_t mesh_cfg;
  tintless_mesh_rolloff_array_t mesh_fixed;   // fixed table to tintless
  tintless_mesh_rolloff_array_t mesh_hw;      // table going to hw
  tintless_mesh_rolloff_array_t mesh_led;     // led mesh table from chromatix
} isp_tintless_mesh_config_t;

isp_tintless_t *isp_tintless_create(uint32_t isp_version);
void isp_tintless_destroy(isp_tintless_t *tintless);
isp_tintless_session_t *isp_tintless_open_session(isp_tintless_t *tintless,
  uint32_t session_id);
void isp_tintless_close_session(isp_tintless_session_t *session);
void isp_tintless_set_param(isp_tintless_session_t *session, uint8_t *enabled);
int isp_tintless_be_config(isp_tintless_session_t *session,
  isp_tintless_notify_data_t *tintless_data);
int isp_tintless_bg_config(isp_tintless_session_t *session,
  isp_tintless_notify_data_t *tintless_data);
int isp_tintless_chroma_config(isp_tintless_session_t *session,
  void *chromatix_ptr);
int isp_tintless_rolloff_config(isp_tintless_session_t *session,
  isp_tintless_notify_data_t *tintless_data);
int isp_tintless_trigger_update(isp_tintless_session_t *session,
  void *be_stats, int type, int hfr_mode);
int isp_tintless_get_table(isp_tintless_session_t *session,
  isp_tintless_notify_data_t *tintless_data_out);

#endif /* __ISP_TINTLESS_H__ */
