/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __MODULE_PPROC_H__
#define __MODULE_PPROC_H__

#include "modules.h"
#include "mct_list.h"
#include "module_pproc_common.h"

typedef boolean (* module_pproc_reserve_submod_func)
  (mct_module_t *module, void *stream_info);

typedef boolean (* module_pproc_unreserve_submod_func)
  (mct_module_t *module, unsigned int identity);

typedef boolean (* module_pproc_link_submod_func)
  (mct_module_t *module, unsigned int identity, mct_port_t *peer);

typedef boolean (* module_pproc_unlink_submod_func)
  (mct_module_t *module, unsigned int identity, mct_port_t *peer);

typedef boolean	(* module_pproc_event_func)
  (mct_module_t *module, mct_port_t *port, void *data);

typedef struct _mct_pproc_init_name {
  const char        *name;
  mct_module_init   init_mod;
  mct_module_deinit deinit_mod;
  mct_module_t      *mod;
} mct_pproc_init_name_t;

typedef enum _pproc_mod_t {
  PPROC_CPP,
  PPROC_C2D,
  PPROC_VPE,
  PPROC_SWI,
  PPROC_MAX
} pproc_mod_t;

typedef struct {
  uint32_t first_pixel;
  uint32_t first_line;
  uint32_t width;
  uint32_t height;
} crop_info_t;

typedef struct {
  uint32_t    input_width;
  uint32_t    input_height;
  crop_info_t zoom_info;
  uint8_t     rotation;
  uint16_t    mirror;
  double      h_scale_ratio;
  double      v_scale_ratio;
} module_pproc_input_t;

/** pproc_submod_t:
 *    @mod: Mct module object of pproc submodule
 *
 **/
typedef struct {
  mct_module_t *mod;
} pproc_submod_t;

/** _pproc_stream_topology:
 *    @mod: Mct module object of pproc submodule
 *
 **/
typedef struct _pproc_stream_topology {
  unsigned int identity;
  mct_list_t *mod_list;
} pproc_stream_topology_t;

typedef struct _pproc_link_traverse {
  mct_list_t *links;
  mct_list_t **out_evnt_list;
  uint32_t identity;
  mct_list_traverse_func traverse_func;
  pproc_event_link_traverse_obj_t incoming_traverse_evnt;
} pproc_link_traverse_t;

/** _module_pproc_ctrl:
 *    @active_sub_mod_list:     detected submodule list.
 *    @stream_topology_list:    active stream topology list.
 *    @buff_mgr_client:         client to generic buffer
 *                              manager.
 *    @reserve_submod:          fn_ptr to reserve submods and
 *                              build topology.
 *    @unreserve_submod:        fn_ptr to unreserve submods.
 *    @link_submod:             fn_ptr to link submods.
 *    @unlink_submod:           fn_ptr to unlink submods.
 *    @pproc_common_event_func: fn_ptr to post public or private
 *                              events to any pproc module or
 *                              submod.
 *
 * pproc modules common control structure
 **/
typedef struct _module_pproc_ctrl {
  mct_list_t                        *active_sub_mod_list;
  mct_list_t                        *stream_topology_list;
  module_pproc_reserve_submod_func   reserve_submod;
  module_pproc_unreserve_submod_func unreserve_submod;
  module_pproc_link_submod_func      link_submod;
  module_pproc_unlink_submod_func    unlink_submod;
  module_pproc_common_event_func_t   pproc_event_func;
} module_pproc_ctrl_t;

#define MODULE_PPROC_PRIVATE_DATA(x) ((x)->module_private)

#define MODULE_PPROC_CTRL_CAST(x) ((module_pproc_ctrl_t *)(x))

#define module_pproc_reserve_submod(x,y)  \
  MODULE_PPROC_CTRL_CAST(MODULE_PPROC_PRIVATE_DATA(x))->reserve_submod(x,y)

#define module_pproc_unreserve_submod(x,y)  \
  MODULE_PPROC_CTRL_CAST(MODULE_PPROC_PRIVATE_DATA(x))->unreserve_submod(x,y)

#define module_pproc_link_submod(x,y,z)  \
  MODULE_PPROC_CTRL_CAST(MODULE_PPROC_PRIVATE_DATA(x))->link_submod(x,y,z)

#define module_pproc_unlink_submod(x,y,z)  \
  MODULE_PPROC_CTRL_CAST(MODULE_PPROC_PRIVATE_DATA(x))->unlink_submod(x,y,z)

#define module_pproc_event_handler_func(x,y,z)  \
  MODULE_PPROC_CTRL_CAST(MODULE_PPROC_PRIVATE_DATA(x))->pproc_event_func(x,y,z)

#endif
