/**********************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __MODULE_AFS_H__
#define __MODULE_AFS_H__

#include "img_common.h"
#include "module_imglib_common.h"
#include "camera_dbg.h"
#include "af_algo_tuning.h"

#define IMGLIB_MOD_NAME "afs"
#define MAX_AFS_STATIC_PORTS 2
#define MAX_NUM_FRAMES 20
#define MAX_NUM_AFS_FRAMES 4

/** afs_client_t
 *   @mutex: client lock
 *   @frame_algo_mutex: frame algo lock
 *   @frame_algo_cond: signal to notify frame_algo is done
 *   @identity: MCT session/stream identity
 *   @state: state of AFS detection client
 *   @p_sinkport: sink port associated with the client
 *   @p_srcport: source port associated with the client
 *   @stream_info: pointer to hold the stream info
 *   @main_dim: variable to store the sensor dimension
 *   @out_trans_info: translation from camif -> preview
 *   @port: pointer to the port which is associated with the
 *        client
 *   @streamon: false to indicate of streamon is issued
 *   @p_mod: module pointer
 *   @frame_skip_cnt: frame skip count
 *   @current_count: current count
 *   @p_frame: Frame buffers
 *   @p_map_buf: Buffers from MCT
 *   @buffer_cnt: Number of frames in current stream
 *   @buf_idx: current buffer index
 *   @crop_info: crop info
 *   @active: flag to indicate whether the client is active
 *   @camif_trans_info: translation for camif
 *   @sync: synchronous handling of frames
 *   @processing: flag to indicate whether PAAF is in progress
 *   @out_dim: output dimension
 *   @video_mode: video mode
 *   @num_skip: number of frames to skip
 *   @cropped_window: dimension of cropped frame
 *   @frame_crop: flag to indicate if frame crop is enabled
 *   @buf_allocation_done: flag to indicate whether buf for
 *     frame is allocated
 *   @frame_dim: dimension of the frame passed to frame algo
 *
 *   afs client structure
 **/
typedef struct {
  pthread_mutex_t mutex;
  pthread_mutex_t frame_algo_mutex;
  pthread_cond_t  frame_algo_cond;
  uint32_t identity;
  imglib_state_t state;
  mct_port_t *p_sinkport;
  mct_port_t *p_srcport;
  mct_stream_info_t *stream_info;
  img_size_t main_dim;
  img_trans_info_t out_trans_info;
  mct_port_t *port;
  int8_t streamon;
  void *p_mod;
  uint32_t frame_skip_cnt;
  uint32_t current_count;
  img_frame_t p_frame[MAX_NUM_AFS_FRAMES];
  mct_stream_map_buf_t p_map_buf[MAX_NUM_FRAMES];
  img_rect_t crop_info;
  uint32_t buffer_cnt;
  int buf_idx;
  int8_t active;
  mct_imglib_af_config_t cur_af_cfg;
  img_trans_info_t camif_trans_info;
  boolean use_af_tuning_trans;
  img_trans_info_t af_tuning_trans_info;
  int frame_id;
  int32_t sync;
  int32_t processing;
  img_size_t out_dim;
  int32_t video_mode;
  img_rect_t roi;
  int num_skip;
  img_rect_t cropped_window;
  boolean frame_crop;
  boolean buf_allocation_done;
  img_size_t frame_dim;
} afs_client_t;

/** afs_session_params_t
 *   @session_id: Session id for which parameters are stored
 *   @valid_params: Valid parameters Yes/No
 *
 *   Structure which holds session based parameters
 **/
typedef struct {
  uint32_t session_id;
  boolean valid_params;
  struct {
  } param;
} afs_session_params_t;

/** module_afs_t
 *   @client_cnt: Variable to hold the number of afs
 *              clients
 *   @module_type: Hold last updated module type
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @lib_ref_count: reference count for afs library access
 *   @afs_client: list of afs clients
 *   @session_parms: List of session based parameters
 *   @msg_thread: message thread
 *
 *   afs module structure
 **/
typedef struct {
  int client_cnt;
  mct_module_type_t module_type;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int lib_ref_count;
  mct_list_t *afs_client;
  mct_list_t *session_parms;
  mod_imglib_msg_th_t msg_thread;
} module_afs_t;

/**
 * afs client APIs
 **/
int module_afs_client_create(mct_module_t *p_mct_mod, mct_port_t *p_port,
  uint32_t identity, mct_stream_info_t *stream_info);

int module_afs_client_start(afs_client_t *p_client);

int module_afs_client_stop(afs_client_t *p_client);

int module_afs_load();

void module_afs_unload();

void module_afs_client_destroy(afs_client_t *p_client);

int module_afs_client_map_buffers(afs_client_t *p_client);

int module_afs_client_handle_buffer(afs_client_t *p_client,
  uint32_t buf_idx, uint32_t frame_id,
  int32_t *p_frame_idx, isp_buf_divert_t *isp_buf);

int module_afs_client_unmap_buffers(afs_client_t *p_client);

int module_afs_client_handle_ctrl_parm(afs_client_t *p_client,
  mct_event_control_parm_t *param);

int module_afs_client_set_scale_ratio(afs_client_t *p_client,
  mct_bus_msg_stream_crop_t *stream_crop);

int module_afs_handle_streamon(module_afs_t *p_mod,
  afs_client_t *p_client);

int module_afs_handle_streamoff(module_afs_t *p_mod,
  afs_client_t *p_client);

void module_afs_client_process(void *p_userdata, void *data);

void module_afs_client_update_cfg(afs_client_t *p_client);

#endif //__MODULE_afs_H__
