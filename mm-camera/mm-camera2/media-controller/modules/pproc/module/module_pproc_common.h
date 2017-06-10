/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __MODULE_PPROC_COMMON_H__
#define __MODULE_PPROC_COMMON_H__

#include <pthread.h>
#include "modules.h"
#include "mct_stream.h"
#include "mct_port.h"
#include "pproc_caps.h"
#include "pproc_interface.h"
#include "pproc_common_buff_mgr.h"
#include "cam_types.h"
#include "cpp.h"

#define PPROC_SUCCESS 0
#define PPROC_FAILURE -1
#define PPROC_ERROR_IO -2
#define PPROC_ERROR_NOMEM -3
#define PPROC_ERROR_INVAL -4

#define STR_SIZE_OF_SRC 3
#define STR_SIZE_OF_SINK 4

#define PPROC_MAX_PLANES 3
#define PPROC_WDN_FILTER_LEVEL 4

typedef enum {
  QUERY_PPROC_CAPABILITIES,
} module_pproc_common_query_type_t;

typedef struct _module_pproc_common_query_caps {
  unsigned int                     sessionid;
  /* TODO: common query type */
  module_pproc_common_query_type_t type;
  pproc_caps_t                     caps;
  void                            *query_buf;
} module_pproc_common_query_caps_t;

/** _pproc_common_event_type:
 *    PPROC_EVENT_MCT_EVENT: MCT Module event
 *    PPROC_EVENT_PRIVATE: pproc mod & submods priv event.
 *
 *  Event type
 **/
typedef enum _pproc_common_event_type {
  PPROC_EVENT_MCT_EVENT,
  PPROC_EVENT_PRIVATE,
} pproc_common_event_type_t;

typedef enum _pproc_event_type {
  PPROC_EVENT_TYPE_MCT_EVENT,
  PPROC_EVENT_TYPE_PRIVATE,
} pproc_event_type_t;

/** _pproc_priv_event_type:
 *    PPROC_PRIV_EVENT_CREATE_TOPOLOGY: create link.
 *
 *  private event enums for pproc mod & submods
 **/
typedef enum _pproc_priv_event_type {
  PPROC_PRIV_EVENT_CREATE_TOPOLOGY,
  PPROC_PRIV_EVENT_DELETE_TOPOLOGY,
  PPROC_PRIV_EVENT_DISPATCH_MCT_EVENT_DWS,
  PPROC_PRIV_EVENT_DISPATCH_MCT_EVENT_UPS,
} pproc_priv_event_type_t;

/** _pproc_priv_event:
 *    @type: pproc private event type.
 *    @data: pproc private event data.
 *
 *  pproc private event object for pproc mod & submods
 **/
typedef struct _pproc_priv_event {
  pproc_priv_event_type_t type;
  void                   *data;
} pproc_priv_event_t;

typedef struct _pproc_event_tplgy_link_traverse_obj {
  uint32_t frame_consumer_ref_count;
  mct_event_t event;
} pproc_event_link_traverse_obj_t;

/** _pproc_common_module_event:
 *    @type: mct_event or private event.
 *    @u.mct_event: mct event pointer.
 *    @pproc_event: pproc private event.
 *
 *  event object for pproc mod & submods
 **/
typedef struct _pproc_event {
  pproc_event_type_t type;
  union {
    mct_event_t *mct_event;
    pproc_priv_event_t pproc_event;
  } u;
} pproc_event_t;

typedef boolean (* module_pproc_common_event_func_t)
  (mct_module_t *module, mct_port_t *port, void *data);

typedef enum _module_pproc_asf_region {
  PPROC_ASF_LOW_LIGHT,
  PPROC_ASF_LOW_LIGHT_INTERPOLATE,
  PPROC_ASF_NORMAL_LIGHT,
  PPROC_ASF_BRIGHT_LIGHT_INTERPOLATE,
  PPROC_ASF_BRIGHT_LIGHT,
  PPROC_ASF_MAX_LIGHT
} module_pproc_asf_region_t;

typedef struct _module_pproc_aec_trigger_params {
  float lowlight_trigger_start;
  float lowlight_trigger_end;
  float brightlight_trigger_start;
  float brightlight_trigger_end;
  float aec_trigger_input;
} module_pproc_aec_trigger_params_t;

typedef struct _module_pproc_chromatix_denoise_params {
  double noise_profile[PPROC_MAX_PLANES][PPROC_WDN_FILTER_LEVEL];
  double weight[PPROC_MAX_PLANES][PPROC_WDN_FILTER_LEVEL];
  double denoise_ratio[PPROC_MAX_PLANES][PPROC_WDN_FILTER_LEVEL];
  double edge_softness[PPROC_MAX_PLANES][PPROC_WDN_FILTER_LEVEL];
} module_pproc_chromatix_denoise_params_t;

typedef struct _module_pproc_common_session_params {
  uint32_t            sessionid;
  double             sharpness;
  cam_denoise_param_t denoise_params;
  cam_effect_mode_type effect;
  cpp_asf_mode         asf_mode;
} module_pproc_common_session_params_t;

typedef struct _module_pproc_common_ctrl {
  /* container pproc module */
  mct_module_t                    *container_mod;
  pproc_buff_mgr_client_t         *buff_mgr_client;
  module_pproc_common_event_func_t pproc_event_func;
  /* library handle */
  pproc_interface_t               *pproc_iface;
  mct_list_t                      *session_params;
  pthread_mutex_t                  mutex;
} module_pproc_common_ctrl_t;

typedef struct {
  uint32_t identity;
  uint32_t src_width;
  uint32_t src_height;
  uint32_t src_stride;
  uint32_t src_scanline;
  uint32_t dst_width;
  uint32_t dst_height;
  uint32_t dst_stride;
  uint32_t dst_scanline;
  mct_bus_msg_stream_crop_t stream_crop;
  is_update_t is_crop;
  pproc_interface_plane_fmt plane_fmt;
  uint32_t flip;
  module_pproc_chromatix_denoise_params_t chrmatix_denoise_params;
  cam_denoise_param_t denoise_params;
  pproc_interface_plane_fmt out_plane_fmt;
  pproc_interface_plane_fmt in_plane_fmt;
  mct_stream_info_t *stream_info;
} module_pproc_common_frame_params_t;

typedef struct {
  cam_streaming_mode_t  streaming_mode;
  mct_list_t           *frame_params;
  modulesChromatix_t chromatix;
  struct cpp_asf_info asf_info;
  float aec_trigger_lux_idx;
  float aec_trigger_real_gain;
} module_pproc_common_port_private_t;

#define MODULE_PPROC_COMMON_CTRL_CAST(x) ((module_pproc_common_ctrl_t *)(x))

#define module_pproc_common_set_container(x,d) \
  MODULE_PPROC_COMMON_CTRL_CAST(x)->container_mod = d

#define module_pproc_common_get_container(x) \
  MODULE_PPROC_COMMON_CTRL_CAST(x)->container_mod

#define MODULE_PPROC_COMMON_SUBMOD_EVENT_FUNC(x, y, z) \
  (MODULE_PPROC_COMMON_CTRL_CAST(x)->pproc_event_func(x, y, z))

int32_t pproc_common_load_library(pproc_interface_t *,const char *);
int32_t pproc_common_unload_library(pproc_interface_t *);
mct_port_t *module_pproc_common_find_port_using_identity(mct_module_t *module,
  uint32_t *identity);
boolean module_pproc_common_process_control_event(mct_module_t *module,
  mct_event_control_t *event, uint32_t identity, mct_port_t *port);
boolean module_pproc_common_process_module_event(mct_module_t *module,
  mct_event_module_t *event, uint32_t identity, mct_port_t *port);
boolean module_pproc_common_process_event(mct_module_t *module,
  mct_event_t *event);
void module_pproc_common_set_mod(mct_module_t *module,
  unsigned int module_type, unsigned int identity);
boolean module_pproc_common_query_mod(mct_module_t *module, void *query_buf,
  unsigned int sessionid);
boolean module_pproc_destroy_port(void *data, void *user_data);
boolean module_pproc_common_create_default_ports(mct_module_t *mod);
boolean module_pproc_common_start_session(mct_module_t *module,
  unsigned int sessionid);
boolean module_pproc_common_stop_session(mct_module_t *module,
  unsigned int sessionid);
mct_port_t *module_pproc_common_request_new_port(void *stream_info,
  mct_port_direction_t direction, mct_module_t *module,
  void *peer_caps);
mct_module_t *module_pproc_common_create_submod(const char *name);
boolean module_pproc_common_find_identity(void *data1, void *data2);
boolean module_pproc_common_match_mod_params_by_session(void *list_data,
  void *user_data);
void module_pproc_common_init_wnr_params(
  module_pproc_common_frame_params_t *frame_params);

#endif /* __PORT_PPROC_COMMON_H__ */
