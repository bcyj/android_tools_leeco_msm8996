/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/

#ifndef __MODULE_WNR_H__
#define __MODULE_WNR_H__

#include "img_common.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "denoise.h"
#include "module_imglib_common.h"
#include "camera_dbg.h"
#include "modules.h"
#include "mct_pipeline.h"
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#define IMGLIB_MOD_NAME "wnr"
#define MAX_NUM_FRAMES 1
#define MAX_WNR_STATIC_PORTS 5

#define MODULE_WNR_DEBUG 1

#define MODULE_WNR_MIN_NUM_PP_BUFS 1

#define MODULE_WNR_META_QUEUE_DEPTH 15

/** wnr_metadata_info_t
 *   @meta_stream_handle: meta data stream I
 *   @meta_stream_handle:buf index to meta data buffer.
 *   wnr metadata info structure
 **/
typedef struct {
  uint32_t meta_stream_handle;
  uint8_t meta_buf_index;
} wnr_metadata_info_t;

/** wnr_client_t
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @comp: component ops structure
 *   @identity: MCT session/stream identity
 *   @state: state of wnr client
 *   @p_sinkport: sink port associated with the client
 *   @p_srcport: source port associated with the client
 *   @stream_info - stream info for the reprocess stream
 *   @input_stream_info - stream info of the input(snapshot) stream
 *   @p_buf_divert_data - input/process buffer holder
 *   @ session_meta - stream meta data holder
 *   @ cam_denoise_param - current session params
 *   @frame: array of image frames
 *   @parent_mod: pointer to the parent module
 *   @p_mod: pointer to the module
 *   @chromatix_param: Holds Chromatix parameters
 *   @stream_off: Flag to indicate whether streamoff is called
 *   @meta_buf_q: List containing metadata info needed for process
 *   @stream_off_mutex: mutex for synchronizing stream off
 *   @stream_off_cond: condition for synchronizing stream off
 *   @early_cb_enabled: Early callback flag for long shot
 *
 *   wnr client structure
 **/
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  img_component_ops_t comp;
  uint32_t identity;
  int state;
  mct_port_t *p_sinkport;
  mct_port_t *p_srcport;
  mct_stream_info_t *stream_info;
  mct_stream_info_t input_stream_info;
  isp_buf_divert_t *p_buf_divert_data;
  mct_stream_session_metadata_info session_meta;
  cam_denoise_param_t cam_denoise_param;
  img_frame_t frame[MAX_NUM_FRAMES];
  mct_module_t *parent_mod;
  void *p_mod;
  int8_t stream_off;
  modulesChromatix_t chromatix_param;
  stats_get_data_t stats_get;
  mct_queue_t meta_buf_q;
  img_debug_info_t debug_info;
  uint8_t early_cb_enabled;
} wnr_client_t;


/** module_wnr_t
 *   @wnr_client_cnt: Variable to hold the number of wnr
 *              clients
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @lib_singleton_mutex: mutex to serialize library
 *   @core_ops: core operation structure
 *   @lib_ref_count: reference count for wnr library access
 *   @wnr_client: List of wnr clients
 *   @msg_thread: message thread
 *   @parent_mod: pointer to the parent module
 *
 *   wnr module structure
 **/
typedef struct {
  int wnr_client_cnt;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_mutex_t lib_singleton_mutex;
  img_core_ops_t core_ops;
  int lib_ref_count;
  mct_list_t *wnr_client;
  mod_imglib_msg_th_t msg_thread;
  mct_module_t *parent_mod;
} module_wnr_t;


/*WNR client APIs*/
int module_wnr_client_create(mct_module_t *p_mct_mod, mct_port_t *p_port,
  uint32_t identity, mct_stream_info_t *stream_info);

void module_wnr_client_destroy(wnr_client_t *p_client);

int module_wnr_client_stop(wnr_client_t *p_client);

int module_wnr_client_exec(wnr_client_t *p_client);

void module_wnr_client_buffers_allocate(void *userdata, void *data);

void module_wnr_client_divert_exec(void *userdata, void *data);
mct_port_t* module_wnr_find_port_with_identity(mct_module_t *module,
  mct_port_direction_t dir, uint32_t identity , wnr_client_t * p_client);

int module_wnr_client_set_meta_info(wnr_client_t *p_client,
    wnr_metadata_info_t *metadata_info);

int module_wnr_client_get_meta_info(wnr_client_t *p_client,
    wnr_metadata_info_t *metadata_info);

int module_wnr_client_clear_meta_info(wnr_client_t *p_client);

#endif //__MODULE_DENOISE_H__

