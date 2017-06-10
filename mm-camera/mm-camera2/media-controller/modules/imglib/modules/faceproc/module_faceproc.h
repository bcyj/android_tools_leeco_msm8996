/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __MODULE_FACEPROC_H__
#define __MODULE_FACEPROC_H__

#include "img_common.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "faceproc.h"
#include "module_imglib_common.h"
#include "camera_dbg.h"
#include "fd_chromatix.h"

#define IMGLIB_MOD_NAME "faceproc"
#define MAX_FD_STATIC_PORTS 1
#define MAX_NUM_FRAMES 20
#define MAX_NUM_FD_FRAMES 4
#define MOD_FACEPROC_SYNC TRUE

#define MAX_FD_WIDTH 1920
#define MAX_FD_HEIGHT 1088

#define FACEPROC_STAB_HISTORY 10

/** faceproc_history_entry_t
 *   @x: X coordinate
 *   @y: Y coordinate
 *
 *   Structure which holds face coordinates
 **/
typedef struct {
  uint32_t x;
  uint32_t y;
} faceproc_history_entry_t;

/** faceproc_history_state_t
 *   @FD_STAB_STATE_STABLE: State is stable old coordinates will be used
 *   @FD_STAB_STATE_UNSTABLE: State is unstable also old coordinates will be used
 *   @FD_STAB_STATE_STABILIZE: Stabilizing move to new position when is done it will
 *     switch tos table state
 *
 *   Enum for face stabilization states
 **/
typedef enum {
  FD_STAB_STATE_STABLE,
  FD_STAB_STATE_UNSTABLE,
  FD_STAB_STATE_STABILIZE,
} faceproc_history_state_t;

/** faceproc_history_holder_t
 *   @index: Current active face index in the history
 *   @faces_inside: Faces inside the history
 *   @state_count: Count frames per curernt state
 *   @max_state_count: Max state can be active (current valid only for unstable state)
 *   @state: Current state
 *   @stable_entry: Current stable entry
 *   @stable_refer: Stable reference if stabilization is used with reference
 *   @entry: Stabilization history entries
 *
 *   Structure which holds face entry stabilization parameters
 **/
typedef struct {
  uint32_t index;
  uint32_t faces_inside;
  uint32_t state_count;
  uint32_t max_state_count;
  faceproc_history_state_t state;
  faceproc_history_entry_t stable_entry;
  faceproc_history_entry_t stable_refer;
  faceproc_history_entry_t entry[FACEPROC_STAB_HISTORY];
} faceproc_history_holder_t;

/** faceproc_faces_history_t
 *   @id: FAce unique ID
 *   @face_size: Face size history holder
 *   @face_position: Face position history holder
 *   @mouth_position: Mouth position history holder
 *   @smile_degree: Smile degree history holder
 *
 *   Structure which holds face detection stabilization history parameters
 **/
typedef struct {
    uint32_t id;
    faceproc_history_holder_t face_size;
    faceproc_history_holder_t face_position;
    faceproc_history_holder_t mouth_position;
    faceproc_history_holder_t smile_degree;
} faceproc_faces_history_t;

/** faceproc_stabilization_t
 *   @faces_detected: Faces detected in the history
 *   @faces: Array of faces history for stabilization
 *
 *   Structure which holds face detection stabilization parameters
 **/
typedef struct {
  uint32_t detected_faces;
  faceproc_faces_history_t faces[MAX_FACES_TO_DETECT * 2]; // We should have double history to handle the sorting
} faceproc_stabilization_t;

/** faceproc_client_t
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @threadid: pthread id if the client is running is async
 *            mode
 *   @comp: component ops structure
 *   @identity: MCT session/stream identity
 *   @buffer_info: buffer info from the module
 *   @mode: facial processing mode
 *   @config: structure to hold the facial processing
 *          configuration
 *   @result: structure to hold the result of face detection
 *   @state: state of face detection client
 *   @is_ready: flag for synchronizing the thread start
 *   @status: integer to hold the asynchronous results
 *   @sync: synchronous/asynchrnous mode
 *   @p_sinkport: sink port associated with the client
 *   @p_srcport: source port associated with the client
 *   @stream_info: pointer to hold the stream info
 *   @fd_active_index: active buffer index for faceproc
 *   @main_dim: variable to store the sensor dimension
 *   @port: pointer to the port which is associated with the
 *        client
 *   @zoom_val: zoom value provided by the UI
 *   @prev_face_count: face count for the previous iteration
 *   @buf_idx: current buffer index
 *   @active: flag to indicate whether the client is active
 *   @streamon: false to indicate of streamon is issued
 *   @p_mod: module pointer
 *   @mutex: mutex to synchronize the result
 *   @frame_skip_cnt: frame skip count
 *   @current_count: current count
 *   @crop_info: crop info
 *   @p_fd_chromatix: FD chromatix pointer
 *   @stabilization: Face detection stabilization function
 *   @camif_trans_info: translation w.r.t camif dimension
 *
 *   Faceproc client structure
 **/
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t threadid;
  img_component_ops_t comp;
  uint32_t identity;
  mod_img_buffer_info_t buffer_info;
  faceproc_mode_t mode;
  faceproc_config_t config;
  faceproc_result_t result[2];
  imglib_state_t state;
  int is_ready;
  int status;
  int sync;
  mct_port_t *p_sinkport;
  mct_port_t *p_srcport;
  mct_stream_info_t *stream_info;
  int fd_active_index;
  img_size_t main_dim;
  img_trans_info_t out_trans_info;
  mct_port_t *port;
  int zoom_val;
  int8_t prev_face_count;
  int buf_idx;
  int8_t active;
  int8_t streamon;
  void *p_mod;
  pthread_mutex_t result_mutex;
  uint32_t frame_skip_cnt;
  uint32_t current_count;
  img_rect_t crop_info;
  fd_chromatix_t *p_fd_chromatix;
  faceproc_stabilization_t stabilization;
  img_trans_info_t camif_trans_info;
} faceproc_client_t;

/** faceproc_session_params_t
 *   @session_id: Session id for which parameters are stored
 *   @valid_params: Valid parameters Yes/No
 *   @fd_enable: Face detection enable
 *   @fr_enable: Face recognition enable
 *   @zoom_val: zoom value provided by the UI
 *
 *   Structure which holds session based parameters
 **/
typedef struct {
  uint32_t session_id;
  boolean valid_params;
  struct {
    boolean fd_enable;
    boolean fr_enable;
    boolean zoom_val;
  } param;
} faceproc_session_params_t;

/** module_faceproc_t
 *   @client_cnt: Variable to hold the number of faceproc
 *              clients
 *   @module_type: Hold last updated module type
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @threadid: pthread id if the client is running is async
 *            mode
 *   @comp: core operation structure
 *   @lib_ref_count: reference count for faceproc library access
 *   @fp_client: list of faceproc clients
 *   @session_parms: List of session based parameters
 *   @msg_thread: message thread
 *   @active: flag to indicate whether the client is active
 *
 *   Faceproc module structure
 **/
typedef struct {
  int client_cnt;
  mct_module_type_t module_type;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  img_core_ops_t core_ops;
  int lib_ref_count;
  mct_list_t *fp_client;
  mct_list_t *session_parms;
  mod_imglib_msg_th_t msg_thread;
  int8_t active;
} module_faceproc_t;

/**
 * FACEPROC client APIs
 **/
int module_faceproc_client_create(mct_module_t *p_mct_mod, mct_port_t *p_port,
  uint32_t identity, mct_stream_info_t *stream_info);

int module_faceproc_client_start(faceproc_client_t *p_client);

int module_faceproc_client_stop(faceproc_client_t *p_client);

void module_faceproc_client_destroy(faceproc_client_t *p_client);

int module_faceproc_client_set_mode(faceproc_client_t *p_client,
  faceproc_mode_t mode);

int module_faceproc_client_map_buffers(faceproc_client_t *p_client);

int module_faceproc_client_handle_buffer(faceproc_client_t *p_client,
  uint32_t buf_idx, uint32_t frame_id);

int module_faceproc_client_process_buffers(faceproc_client_t *p_client,
    uint32_t frame_id);

int module_faceproc_client_unmap_buffers(faceproc_client_t *p_client);

int module_faceproc_client_handle_ctrl_parm(faceproc_client_t *p_client,
  mct_event_control_parm_t *param);

int module_faceproc_client_set_scale_ratio(faceproc_client_t *p_client,
  mct_bus_msg_stream_crop_t *stream_crop);

int module_faceproc_handle_streamon(module_faceproc_t *p_mod,
  faceproc_client_t *p_client);

int module_faceproc_handle_streamoff(module_faceproc_t *p_mod,
  faceproc_client_t *p_client);

int module_faceproc_faces_stabilization(faceproc_client_t *p_client,
  faceproc_result_t *p_result);

#endif //__MODULE_FACEPROC_H__
