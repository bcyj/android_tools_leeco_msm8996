/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#ifndef __MODULE_IMGLIB_BASE_H__
#define __MODULE_IMGLIB_BASE_H__

#include "img_common.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "module_imgbase.h"
#include "module_imglib_common.h"
#include "camera_dbg.h"
#include "modules.h"
#include "mct_pipeline.h"
#include <sys/sysinfo.h>

//512MB
#define RAM_SIZE_THRESHOLD_FOR_AOST 536870912

#define MAX_IMGLIB_BASE_STATIC_PORTS 5
#define MAX_IMGLIB_BASE_MAX_STREAM 3

#if !defined ABS
  #define  ABS(x) ((x)>0 ? (x) : -(x))
#endif

#define FACE_TILT_CUTOFF_FOR_TP 45

/** imgbase_stream_info_t
 *   @identity: MCT session/stream identity
 *   @p_sinkport: sink port associated with the client
 *   @p_srcport: source port associated with the client
 *   @stream_info: stream information
 *
 *   IMGLIB_BASE stream structure
 **/
typedef struct {
  uint32_t identity;
  mct_port_t *p_sinkport;
  mct_port_t *p_srcport;
  mct_stream_info_t *stream_info;
} imgbase_stream_t;

/** imgbase_client_t
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @comp: component ops structure
 *   @frame: frame info from the module
 *   @state: state of face detection client
 *   @frame: array of image frames
 *   @parent_mod: pointer to the parent module
 *   @stream_on: Flag to indicate whether streamon is called
 *   @p_mod: pointer to the module
 *   @mode: IMBLIB mode
 *   @cur_buf_cnt: current buffer count
 *   @caps: imglib capabilities
 *   @current_meta: current meta data
 *   @stream_parm_q: stream param queue
 *   @frame_id: frame id
 *   @p_current_meta:  pointer to current meta
 *   @cur_index: current stream index
 *   @rate_control: control the rate of frames
 *   @prev_time: previous time
 *   @first_frame: flag to indicate whether first frame is
 *               received
 *   @exp_frame_delay: expected frame delay
 *   @ion_fd: ION file descriptor
 *   @dis_enable: digital image stabilization enable flag set from HAL
 *   @is_update_valid: image stabilization valid data flag
 *   @is_update: image stabilization data
 *   @stream_crop_valid: isp_output_dim_stream_info_valid
 *   @stream_crop: stream crop data
 *   @isp_output_dim_stream_info_valid: isp output dim stream info valid flag
 *   @isp_output_dim_stream_info: isp output dim stream info
 *   @divert_mask: current divert mask if base module is after cpp
 *
 *   IMGLIB_BASE client structure
 **/
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  img_component_ops_t comp;
  int state;
  mct_module_t *parent_mod;
  uint8_t stream_on;
  void *p_mod;
  img_comp_mode_t mode;
  int cur_buf_cnt;
  img_caps_t caps;
  img_meta_t current_meta;
  img_queue_t stream_parm_q;
  int frame_id;
  img_meta_t *p_current_meta;
  imgbase_stream_t stream[MAX_IMGLIB_BASE_MAX_STREAM];
  int32_t stream_cnt;
  int32_t cur_index;
  int32_t rate_control;
  struct timespec prev_time;
  int32_t first_frame;
  uint64_t exp_frame_delay;
  int32_t ion_fd;
  boolean dis_enable;
  boolean is_update_valid;
  is_update_t is_update;
  boolean stream_crop_valid;
  mct_bus_msg_stream_crop_t stream_crop;
  boolean isp_output_dim_stream_info_valid;
  mct_stream_info_t isp_output_dim_stream_info;
  uint32_t divert_mask;
} imgbase_client_t;

/** module_imgbase_params_t
 *   @imgbase_query_mod: function pointer for module query
 *   @imgbase_client_init_params: function ptr for init params
 *   @imgbase_client_share_stream: called during caps reserve to
 *                            ensure that client supports the
 *                            particular stream
 *   @imgbase_client_created: function called when client is
 *                          created
 *   @imgbase_client_destroy: function called before client is
 *                          destroyed
 *   @imgbase_client_process_done: function to indicate after
 *                               the frame is processed
 *
 *   module parameters for imglib base
 **/
typedef struct {
  int32_t (*imgbase_query_mod)(mct_pipeline_cap_t *);
  boolean (*imgbase_client_init_params)(img_init_params_t *p_params);
  boolean (*imgbase_client_stream_supported)(imgbase_client_t *p_client,
    mct_stream_info_t *stream_info);
  int32_t (*imgbase_client_created)(imgbase_client_t *);
  int32_t (*imgbase_client_destroy)(imgbase_client_t *);
  int32_t (*imgbase_client_process_done)(imgbase_client_t *, img_frame_t *);
  boolean (*imgbase_crop_support)();
} module_imgbase_params_t;

/** module_imgbase_t
 *   @imgbase_client_cnt: Variable to hold the number of IMGLIB_BASE
 *              clients
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @comp: core operation structure
 *   @lib_ref_count: reference count for imgbase library access
 *   @imgbase_client: List of IMGLIB_BASE clients
 *   @msg_thread: message thread
 *   @parent: pointer to the parent module
 *   @caps: Capabilities for imaging component
 *   @subdevfd: Buffer manager subdev FD
 *   @modparams: module parameters
 *   @name: name of the module
 *
 *   IMGLIB_BASE module structure
 **/
typedef struct {
  int imgbase_client_cnt;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  img_core_ops_t core_ops;
  int lib_ref_count;
  mct_list_t *imgbase_client;
  mod_imglib_msg_th_t msg_thread;
  mct_module_t *parent_mod;
  uint32_t extra_buf;
  uint32_t feature_mask;
  void *mod_private;
  img_caps_t caps;
  int subdevfd;
  module_imgbase_params_t modparams;
  const char *name;
} module_imgbase_t;

/**
 * Macro: mod_imgbase_send_event
 *
 * Description: This macro is used for sending an event between
 *            the modules
 *
 * Arguments:
 *   @id: identity
 *   @is_upstream: flag to indicate whether its upstream event
 *   @evt_type: event type
 *   @evt_data: event payload
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
#define mod_imgbase_send_event(id, is_upstream, evt_type, evt_data) ({ \
  boolean rc = TRUE; \
  mct_port_t *p_port; \
  mct_event_t event; \
  memset(&event, 0x0, sizeof(mct_event_t)); \
  event.type = MCT_EVENT_MODULE_EVENT; \
  event.identity = id; \
  if (is_upstream) { \
    event.direction = MCT_EVENT_UPSTREAM; \
    p_port = p_stream->p_sinkport; \
  } else { \
    event.direction = MCT_EVENT_DOWNSTREAM; \
    p_port = p_stream->p_srcport; \
  } \
  event.u.module_event.type = evt_type; \
  event.u.module_event.module_event_data = &evt_data; \
  rc =  mct_port_send_event_to_peer(p_port, &event); \
  rc; \
})

/*IMGLIB_BASE client APIs*/

/**
 * Function: module_imgbase_deinit
 *
 * Description: This function is used to free the imgbase module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_imgbase_deinit(mct_module_t *p_mct_mod);

/** module_imgbase_init:
 *
 *  Arguments:
 *  @name - name of the module
 *  @comp_role: imaging component role
 *  @comp_name: imaging component name
 *  @mod_private: derived structure pointer
 *  @p_caps: imaging capability
 *  @lib_name: library name
 *  @feature_mask: feature mask of imaging algo
 *  @p_modparams: module parameters
 *
 * Description: This function is used to initialize the imgbase
 * module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_imgbase_init(const char *name,
  img_comp_role_t comp_role,
  char *comp_name,
  void *mod_private,
  img_caps_t *p_caps,
  char *lib_name,
  uint32_t feature_mask,
  module_imgbase_params_t *p_modparams);

/** Function: module_imgbase_client_create
 *
 * Description: This function is used to create the IMGLIB_BASE client
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @p_port: mct port pointer
 *   @identity: identity of the stream
 *   @stream_info: stream information
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_create(mct_module_t *p_mct_mod,
  mct_port_t *p_port,
  uint32_t identity,
  mct_stream_info_t *stream_info);

/**
 * Function: module_imgbase_client_destroy
 *
 * Description: This function is used to destroy the imgbase client
 *
 * Arguments:
 *   @p_client: imgbase client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
void module_imgbase_client_destroy(imgbase_client_t *p_client);

/**
 * Function: module_imgbase_client_stop
 *
 * Description: This function is used to stop the IMGLIB_BASE
 *              client
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_stop(imgbase_client_t *p_client);

/**
 * Function: module_imgbase_client_start
 *
 * Description: This function is used to start the IMGLIB_BASE
 *              client
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_start(imgbase_client_t *p_client);

/**
 * Function: module_imgbase_client_handle_buffer
 *
 * Description: This function is used to start the IMGLIB_BASE
 *              client
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @p_buf_divert: Buffer divert structure
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_handle_buffer(imgbase_client_t *p_client,
  isp_buf_divert_t *p_buf_divert);

/** module_imgbase_set_parent:
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_imgbase_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent);

#endif //__MODULE_IMGLIB_BASE_H__
