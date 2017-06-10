/* af_fdprio.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef AF_FDPRIO_H_
#define AF_FDPRIO_H_

typedef enum {
  AF_FDPRIO_CMD_INIT,
  AF_FDPRIO_CMD_PROC_COUNTERS,
  AF_FDPRIO_CMD_PROC_FD_ROI,
  AF_FDPRIO_CMD_PROC_FD_MAX
} af_fdprio_cmd_t;

typedef struct {
  int32_t   face_id;
  uint32_t  frame_count;
} stability_data_t;

/** af_fdprio_face_info_t
 */
typedef struct {
  boolean is_valid;
  int32_t face_id;
  int32_t ctr_x;
  int32_t ctr_y;
  int32_t top_left_x;
  int32_t top_left_y;
  uint32_t loc_stability_cnt;
  uint32_t size_stability_cnt;
} af_fdprio_face_info_t;


/** af_fdprio_t
 *
 *  @noface_cnt: how many consequent frames no faces are detected
 *
 *  @faces_detects: how many faces were detected in the latest frame
 *
 *  @info_pos: where to write the next biggest face info
 *
 *  @biggest_face_info: history for the biggest face detected
 *
 *  @face_info: info for the detected faces in the latest frame
 *              Must not be NULL if the command is AF_FDPRIO_CMD_PROC_FD_ROI
 *
 */
typedef struct {
  boolean fd_enabled;
  uint8_t noface_cnt;
  uint8_t faces_detected;
  cam_rect_t cur_confgd_roi;
  uint32_t current_frame_id;
  uint32_t last_processed_frame_id;
  uint32_t camif_width;
  uint32_t camif_height;
  boolean trigger_search;
  af_fdprio_face_info_t active_face;
  mct_face_info_t   *pface_info;
  q3a_thread_data_t *thread_data;
  af_algo_tune_parms_t *tuning_info;
  int preview_fps;
} af_fdprio_t;


/* API function(s) */
boolean af_fdprio_process(af_fdprio_t *af_fdprio_data, af_fdprio_cmd_t cmd);

#endif /* AF_FDPRIO_H_ */
