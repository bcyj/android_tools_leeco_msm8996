/* af_port.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __PORT_AEC_H__
#define __PORT_AEC_H__

#include <mct_stream.h>
#include "q3a_thread.h"
#include "modules.h"

/* Every AEC sink port ONLY corresponds to ONE session */

typedef enum {
  AEC_PORT_STATE_CREATED,
  AEC_PORT_STATE_RESERVED,
  AEC_PORT_STATE_LINKED,
  AEC_PORT_STATE_UNLINKED,
  AEC_PORT_STATE_UNRESERVED
} aec_port_state_t;

/** _aec_port_private:
 *    @reserved_id:           TODO
 *    @stream_type:           TODO
 *    @vfe_out_width:         TODO
 *    @vfe_out_height:        TODO
 *    @cur_sof_id:            TODO
 *    @state:                 TODO
 *    @aec_update_data:       TODO
 *    @aec_update_flag:       TODO
 *    @aec_object:            session index
 *    @thread_data:           TODO
 *    @stream_info:           a copy, not a reference, can this be a const ptr
 *    @aec_get_data:          TODO
 *    @video_hdr:             TODO
 *    @aec_state:             TODO
 *    @aec_last_state:        TODO
 *    @locked_from_hal:       TODO
 *    @aec_enabled:           TODO
 *    @aec_precap_start:      TODO
 *    @aec_precap_for_af:     TODO
 *    @force_prep_snap_done:  TODO
 *    @aec_precap_trigger_id: TODO
 *    @iso_speed:             TODO
 *    @max_sensor_delay:      TODO
 *    @in_zsl_capture:        TODO
 *    @aec_roi:               TODO
 *    @est_state:             TODO
 *
 * Each aec moduld object should be used ONLY for one Bayer
 * session/stream set - use this structure to store session
 * and stream indices information.
 **/
typedef struct _aec_port_private {
  unsigned int        reserved_id;
  cam_stream_type_t   stream_type;
  int                 vfe_out_width;
  int                 vfe_out_height;
  uint32_t            cur_sof_id;
  uint32_t            cur_stats_id;
  aec_port_state_t    state;
  stats_update_t      aec_update_data;
  stats_update_t      aec_data_copy;
  boolean             aec_update_flag;
  boolean             need_to_send_est;
  aec_object_t        aec_object;
  q3a_thread_data_t   *thread_data;
  mct_stream_info_t   stream_info;
  aec_get_t           aec_get_data;

  int32_t                  video_hdr;
  enum sensor_stats_type   video_hdr_stats_type;

  uint8_t             aec_state;
  cam_ae_state_t      aec_last_state;
  boolean             locked_from_hal;
  boolean             in_longshot_mode;
  boolean             aec_enabled;
  boolean             aec_precap_start;
  boolean             aec_precap_for_af;
  boolean             force_prep_snap_done;
  int32_t             aec_precap_trigger_id;
  int32_t             iso_speed;
  uint32_t            max_sensor_delay;
  boolean             in_zsl_capture;
  int                 preview_width;
  int                 preview_height;
  char                aec_debug_data_array[AEC_DEBUG_DATA_SIZE];
  uint32_t            aec_debug_data_size;
  /* HAL 3*/
  cam_area_t          aec_roi;
  aec_led_est_state_t est_state;
  uint32_t            dual_led_calibration_cnt;
} aec_port_private_t;

void    aec_port_deinit(mct_port_t *port);
boolean aec_port_find_identity(mct_port_t *port, unsigned int identity);
boolean aec_port_init(mct_port_t *port, unsigned int *session_id);

#endif /* __PORT_AEC_H__ */
