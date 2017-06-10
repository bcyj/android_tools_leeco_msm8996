/***************************************************************************
* Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/

#ifndef __MODULE_IMGLIB_DEFS_H__
#define __MODULE_IMGLIB_DEFS_H__

#include "module_imglib.h"
#include "camera_dbg.h"
#include "modules.h"

/** module_imglib_topology_t
 *   @feature_mask: Stream type which this topology will be used
 *   @port_events_mask: port event mask is used to select extra functionality
 *     in imglib for port events. Supported masks:
 *       MODULE_IMGLIB_PORT_USE_INT_BUFS - This mask will enable internally allocated
 *       buffers to be used for buffer divert event. buffer gave reference count
 *       for all parallel topologies.
 *   @stream_type: Array of modules used as topology building blocks
 *   @modules: Array holding parallel topologies and their module configuration.
 *   @session_params: Array of session parameters which need
 *     to be stored for this topology
 *
 *   Imagelib topology structure
 **/
typedef struct {
  uint32_t feature_mask;
  uint32_t port_events_mask;
  cam_stream_type_t stream_type;
  mct_module_init_name_t modules[MODULE_IMGLIB_MAX_PAR_TOPO][MODULE_IMGLIB_MAX_TOPO_MOD];
  cam_intf_parm_type_t session_params[MODULE_IMGLIB_MAX_STORED_PARAMS];
} module_imglib_topology_t;

/* Section for defining imglib internal topologies.
 *
 * Module definitions - Every module in the topology
 * need to be defined with 3 fields:
 *
 * - name: Module name
 * - init_mod: Module init function
 * - deinit_mod: Module deinit function
 *
 * Example:
 *
 * #define MOD_IMGLIB_EXAMPLE { \
 * .name = "imglib_example", \
 * .init_mod = module_example_init, \
 * .deinit_mod = module_example_deinit, } \
 *
 * Topology Definitions
 *  Internal topologies will be created per port. If topology is not
 *  available, port will only accept and redirect the events.
 *
 * 4 Fields:
 *
 * - feature_mask - Optional - If mask is not present it will not be used.
 *
 * - port_events_mask - Port event mask. This feature is used if we want to enable
 *   extra handling for ports events. Supported masks:
 *     1. MODULE_IMGLIB_PORT_USE_INT_BUFS - Imglib will use
 *     internal memory for buffer divert events, on every event internal copy
 *     of the buffer will be used with reference counting for parallel topologies.
 *
 * - stream_type - Mandatory - Stream type for which topology will be used.
 *
 * - modules - Mandatory - Array of internal topology modules, if more then one list
 *    of topology is set they will work in parallel and they will be connected
 *    to one port, every event will be sent to all parallel topologies.
 *    How they are connected:
 *    first in the list will be connected with sink port of imglib,
 *    last in the list with source port of imglib,
 *    all modules will be connected with manner first->second->third...
 *
 * - stored_params: Session parameters to be stored/restored,
 *     use CAM_INTF_PARM_MAX delimiter.
 *
 * Topology selection is done based on stream type and feature mask,
 * if feature mask is not set it will not be used.
 * If there are topologies for same stream type but different masks
 * the topology with more features matching will be selected.
 *
 * Example:
 * Only one module in the topology:
 *
 *  { .stream_type = CAM_STREAM_TYPE_PREVIEW, \
 *     .modules[0] = {MOD_IMGLIB_EXAMPLE}, \
 *  }, \
 *
 * How it looks inside imglib module:
 *  ------------- ------------
 *  ||->MOD_IMGLIB_EXAMPLE->||
 *  --------------------------
 *
 * With more modules in topology:
 *
 *
 *  { .stream_type = CAM_STREAM_TYPE_PREVIEW, \
 *     .modules[0] = {MOD_IMGLIB_EXAMPLE1, MOD_IMGLIB_EXAMPLE2}, \
 *  }, \
 *
 * Inside imglib module:
 *  ------------- ----------------------------------
 *  ||->MOD_IMGLIB_EXAMPLE1->MOD_IMGLIB_EXAMPLE2->||
 *  ------------------------------------------------
 *
 * Parralel toplologies:
 *
 *  { .stream_type = CAM_STREAM_TYPE_PREVIEW, \
 *     .modules[0] = {MOD_IMGLIB_EXAMPLE1, MOD_IMGLIB_EXAMPLE2}, \
 *     .modules[1] = {MOD_IMGLIB_EXAMPLE3, MOD_IMGLIB_EXAMPLE4}, \
 *  }, \
 *
 *
 * Inside imglib module, both topologies are connected to the
 * same port in imaging lib:
 *  ------------- ----------------------------------
 *  ||->MOD_IMGLIB_EXAMPLE1->MOD_IMGLIB_EXAMPLE2->||
 *    ->MOD_IMGLIB_EXAMPLE3->MOD_IMGLIB_EXAMPLE4->
 *  ------------------------------------------------
 */

/* Modules definition */
#define MOD_IMGLIB_FACEPROC { \
  .name = "imglib_faceproc", \
  .init_mod = module_faceproc_init, \
  .deinit_mod = module_faceproc_deinit, } \

#define MOD_IMGLIB_DENOISE { \
  .name = "imglib_denoise", \
  .init_mod = NULL, \
  .deinit_mod = NULL, } \

#ifdef CAMERA_FEATURE_WNR_SW

#define MOD_IMGLIB_HDR { \
  .name = "hdr", \
  .init_mod = NULL, \
  .deinit_mod = NULL, } \

#else

#define MOD_IMGLIB_HDR { \
  .name = "hdr", \
  .init_mod = module_hdr_init, \
  .deinit_mod = module_hdr_deinit, } \

#endif

#define MOD_IMGLIB_CAC { \
  .name = "imglib_cac", \
  .init_mod = NULL, \
  .deinit_mod = NULL, } \

#define MOD_IMGLIB_CHROMA_FLASH { \
  .name = "imglib_chroma_flash", \
  .init_mod = module_chroma_flash_init, \
  .deinit_mod = module_chroma_flash_deinit, } \

#define MOD_IMGLIB_OPTIZOOM { \
  .name = "imglib_optizoom", \
  .init_mod = module_optizoom_init, \
  .deinit_mod = module_optizoom_deinit, } \

#define MOD_IMGLIB_UBIFOCUS { \
  .name = "imglib_ubifocus", \
  .init_mod = module_ubifocus_init, \
  .deinit_mod = module_ubifocus_deinit, } \

#define MOD_IMGLIB_REFOCUS { \
  .name = "imglib_refocus", \
  .init_mod = module_refocus_init, \
  .deinit_mod = module_refocus_deinit, } \

#define MOD_IMGLIB_FSSR { \
  .name = "imglib_fssr", \
  .init_mod = module_fssr_init, \
  .deinit_mod = module_fssr_deinit, } \

#define MOD_IMGLIB_AFS { \
  .name = "imglib_afs", \
  .init_mod = module_afs_init, \
  .deinit_mod = module_afs_deinit, } \

#define MOD_IMGLIB_TRUEPORTRAIT { \
  .name = "imglib_trueportrait", \
  .init_mod = module_trueportrait_init, \
  .deinit_mod = module_trueportrait_deinit, } \

#define MOD_IMGLIB_MULTI_TOUCH_FOCUS { \
  .name = "imglib_multitouchfocus", \
  .init_mod = module_multitouch_focus_init, \
  .deinit_mod = module_multitouch_focus_deinit, } \


/* Topology definitions */
#define MOD_IMGLIB_TOPOLOGY_REGISTER(t) \
  static module_imglib_topology_t (t)[] = { \
    { .stream_type = CAM_STREAM_TYPE_PREVIEW, \
      .modules[0] = {MOD_IMGLIB_AFS, MOD_IMGLIB_FACEPROC}, \
      .session_params = {CAM_INTF_PARM_FD, CAM_INTF_PARM_MAX}, \
    }, \
    { .feature_mask = CAM_QCOM_FEATURE_DENOISE2D, \
      .stream_type = CAM_STREAM_TYPE_SNAPSHOT, \
      .modules[0] = {MOD_IMGLIB_DENOISE}, \
      .session_params = {CAM_INTF_PARM_WAVELET_DENOISE, CAM_INTF_PARM_MAX}, \
    }, \
    { .feature_mask = CAM_QCOM_FEATURE_HDR, \
      .stream_type = CAM_STREAM_TYPE_OFFLINE_PROC, \
      .modules[0] = {MOD_IMGLIB_HDR}, \
      .session_params = {CAM_INTF_PARM_HDR, CAM_INTF_PARM_MAX}, \
    }, \
    { .feature_mask = CAM_QCOM_FEATURE_REGISTER_FACE, \
      .stream_type = CAM_STREAM_TYPE_OFFLINE_PROC, \
      .modules[0] = {MOD_IMGLIB_FACEPROC}, \
      .session_params = {CAM_INTF_PARM_FD, CAM_INTF_PARM_MAX}, \
    }, \
    { .feature_mask = CAM_QCOM_FEATURE_CHROMA_FLASH, \
      .stream_type = CAM_STREAM_TYPE_OFFLINE_PROC, \
      .modules[0] = {MOD_IMGLIB_CHROMA_FLASH}, \
      .session_params = {CAM_INTF_PARM_MAX}, \
    }, \
    { .feature_mask = CAM_QCOM_FEATURE_OPTIZOOM, \
      .stream_type = CAM_STREAM_TYPE_OFFLINE_PROC, \
      .modules[0] = {MOD_IMGLIB_OPTIZOOM}, \
      .session_params = {CAM_INTF_PARM_MAX}, \
    }, \
    { .feature_mask = CAM_QCOM_FEATURE_UBIFOCUS, \
      .stream_type = CAM_STREAM_TYPE_OFFLINE_PROC, \
      .modules[0] = {MOD_IMGLIB_UBIFOCUS}, \
      .session_params = {CAM_INTF_PARM_MAX}, \
    }, \
    { .feature_mask = CAM_QCOM_FEATURE_TRUEPORTRAIT, \
      .stream_type = CAM_STREAM_TYPE_OFFLINE_PROC, \
      .modules[0] = {MOD_IMGLIB_TRUEPORTRAIT}, \
      .session_params = {CAM_INTF_PARM_MAX}, \
    }, \
    { .feature_mask = CAM_QCOM_FEATURE_FSSR, \
      .stream_type = CAM_STREAM_TYPE_OFFLINE_PROC, \
      .modules[0] = {MOD_IMGLIB_FSSR}, \
      .session_params = {CAM_INTF_PARM_MAX}, \
    }, \
    { .feature_mask = CAM_QCOM_FEATURE_MULTI_TOUCH_FOCUS, \
      .stream_type = CAM_STREAM_TYPE_OFFLINE_PROC, \
      .modules[0] = {MOD_IMGLIB_MULTI_TOUCH_FOCUS}, \
      .session_params = {CAM_INTF_PARM_MULTI_TOUCH_FOCUS_BRACKETING, CAM_INTF_PARM_MAX}, \
    }, \
    { .feature_mask = CAM_QCOM_FEATURE_REFOCUS, \
      .stream_type = CAM_STREAM_TYPE_OFFLINE_PROC, \
      .modules[0] = {MOD_IMGLIB_REFOCUS}, \
      .session_params = {CAM_INTF_PARM_MAX}, \
    }, \
  } \

#endif //__MODULE_IMGLIB_DEFS_H__
