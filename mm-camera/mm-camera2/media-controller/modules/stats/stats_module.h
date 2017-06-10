/* stats_module .h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __STATS_MODULE_H__
#define __STATS_MODULE_H__

/* DEBUG macro section for STATS module */

#define  Q3A_MODULE_DEBUG         0

#define  Q3A_PORT_DEBUG           0

#define  Q3A_THREAD_DEBUG         0

#include "mct_module.h"
#include "aec.h"
#include "awb.h"
#include "af.h"
#include "asd.h"
#include "is_interface.h"

typedef enum {
  STATS_MASK_AEC     = (1 << 0),
  STATS_MASK_AWB     = (1 << 1),
  STATS_MASK_AF      = (1 << 2),
  STATS_MASK_ASD     = (1 << 3),
  STATS_MASK_AFD     = (1 << 4),
} stats_mask_type;

/** stats_set_param_t
 *
 *  This enumeration represents the types of set parameter for stats
 *
 **/
typedef enum {
  STATS_SET_Q3A_PARAM,
  STATS_SET_ASD_PARAM,
  STATS_SET_AFD_PARAM,
  STATS_SET_IS_PARAM,
  STATS_SET_GYRO_PARAM,
  STATS_SET_COMMON_PARAM,
} stats_set_param_t;

/** stats_get_param_t
 *
 *  This enumeration represents the types of get parameter for stats
 *
 **/
typedef enum {
  STATS_GET_Q3A_PARAM,
  STATS_GET_ASD_PARAM,
  STATS_GET_AFD_PARAM,
  STATS_GET_EIS_PARAM,
  STATS_GET_GYRO_PARAM,
} stats_get_param_t;

/** q3a_set_param_t
 *
 *  This enumeration represents the types of set parameter for q3a
 *
 **/
typedef enum {
  Q3A_SET_AEC_PARAM,
  Q3A_SET_AWB_PARAM,
  Q3A_SET_AF_PARAM,
  Q3A_ALL_SET_PARAM,
} q3a_set_param_t;

/** q3a_get_param_t
 *
 *  This enumeration represents the types of get parameter for q3a
 *
 **/
typedef enum {
  Q3A_GET_AEC_PARAM,
  Q3A_GET_AWB_PARAM,
  Q3A_GET_AF_PARAM,
} q3a_get_param_t;

/** q3a_all_set_param_t
 *
 *  This enumeration represents the types of set parameter applys to all q3a modules
 *
 **/
typedef enum {
  Q3A_ALL_SET_BESTSHOT_MODE,
  Q3A_ALL_SET_EZTUNE_RUNNIG,
  Q3A_ALL_SET_HAL_VERSION,
} q3a_all_set_param_t;

/** q3a_all_set_param_type
 *    @type:          Indicates the type of the parameter to be processed
 *    @bestshot_mode: Indicates the type of best mode, refer to cam_scene_mode_type
 *
 *  This structure is used to indicate the type of parameter which applys to all q3a module
 *
 **/
typedef struct {
  q3a_all_set_param_t type;
  union {
    int32_t bestshot_mode;
    int     ez_runnig;
    int32_t hal_version;
  } u;
} q3a_all_set_param_type;

/** q3a_set_params_type
 *    @type:                   Indicates the type of the parameter to be processed
 *    @aec_set_parameter_t:    the structure for aec parameter
 *    @awb_set_parameter_t:    the structure for awb parameter
 *    @af_set_parameter_t:     the structure for af parameter
 *    @q3a_all_set_param_type: the structure for parameter applys to all q3a module
 *
 *  This structure is used to indicate the type of parameter set to q3a submodules
 *
 **/
typedef struct {
  q3a_set_param_t type;
  union {
    aec_set_parameter_t     aec_param;
    awb_set_parameter_t     awb_param;
    af_set_parameter_t      af_param;
    q3a_all_set_param_type  q3a_all_param;
  } u;
} q3a_set_params_type;

/** stats_common_set_param_type
 *
 *  This enumeration represents the types of common set parameter for stats module
 *
 **/
typedef enum {
  COMMON_SET_PARAM_BESTSHOT,
  COMMON_SET_PARAM_VIDEO_HDR,
  COMMON_SET_PARAM_STATS_DEBUG_MASK,
  COMMON_SET_PARAM_SUPER_EVT,
  COMMON_SET_PARAM_META_MODE,
  COMMON_SET_CAPTURE_INTENT,
  COMMON_SET_CROP_REGION,
  COMMON_SET_PARAM_ALGO_OPTIMIZATIONS_MASK,
} stats_common_set_param_type;

/** stats_common_set_param_type
 *
 *  This enumeration represents the types of capture intents
 *
 **/
typedef enum {
  STATS_INTENT_UNKNOWN,
  STATS_INTENT_PREVIEW,
  STATS_INTENT_STILL_CAPTURE,
  STATS_INTENT_VIDEO_RECORD,
  STATS_INTENT_VIDEO_SNAPSHOT,
  STATS_INTENT_ZERO_SHUTTER_LAG,
  STATS_INTENT_MAX,
} stats_capture_intent_t;

/** stats_common_set_parameter_t
 *    @type:                  Indicates the type of the parameter to be processed
 *    @bestshot_mode:         Indicates the type of best mode, refer to cam_scene_mode_type
 *    @video_hdr:             Indicates the type of video_hdr
 *    @stats_debug_mask:      Indicates debug mask got from hal, this mask will be passed
 *                            to mm-camera-core to control debug feature, like log
 *
 *  This structure is used to indicate the type of common parameter set to stats module
 *
 **/
typedef struct {
  stats_common_set_param_type type;

  union {
    int                    bestshot_mode;
    int32_t                video_hdr;
    uint32_t               stats_debug_mask;
    unsigned int           current_frame_id;
    uint8_t                meta_mode;
    stats_capture_intent_t capture_type;
    cam_crop_region_t      crop_region;
    uint32_t               algo_opt_mask;
  } u;
} stats_common_set_parameter_t;

/** stats_set_params_type
 *    @type:                          Indicates the type of the parameter to be processed
 *    @q3a_set_params_type:           the structure for q3a parameter
 *    @asd_set_parameter_t:           the structure for asd parameter
 *    @cam_antibanding_mode_type:     the type of cam antibanding mode
 *    @is_set_params_type:            is parameter
 *    @stats_common_set_parameter_t:  the structure for common stats
 *
 *  This structure is used to indicate the type of parameter set to stats submodules
 *
 **/
typedef struct {
  stats_set_param_t param_type;
  union {
    q3a_set_params_type          q3a_param;
    asd_set_parameter_t          asd_param;
    cam_antibanding_mode_type    afd_param;
    is_set_parameter_t           is_param;
    stats_common_set_parameter_t common_param;
  } u;
} stats_set_params_type;


/** stats_module_ack_key_t
 *    @identity:    Indicates the identity
 *    @buf_idx:     indicates the index of buffer
 *    @channel_id:  indicates the id of channel
 *
 *  This structure is used to indicate the ack key
 *
 **/
typedef struct _stats_module_ack_key_t {
  uint32_t identity;
  int      buf_idx;
  int      channel_id;
} stats_module_ack_key_t;

/** stats_port_cap_type
 *
 *  This enumeration represents the types of port's capacity
 *
 **/
typedef enum {
  MCT_PORT_CAP_STATS_AEC = (1 << 0),
  MCT_PORT_CAP_STATS_AWB = (1 << 1),
  MCT_PORT_CAP_STATS_AF = (1 << 2),
  MCT_PORT_CAP_STATS_AFD = (1 << 3),
  MCT_PORT_CAP_STATS_ASD = (1 << 4),
  MCT_PORT_CAP_STATS_HIST = (1 << 5),
  MCT_PORT_CAP_STATS_BHIST = (1 << 6),
  MCT_PORT_CAP_STATS_RS_CS = (1 << 7),
  MCT_PORT_CAP_STATS_GYRO = (1 << 8),
  MCT_PORT_CAP_STATS_IS = (1 << 9),
  MCT_PORT_CAP_STATS_Q3A_VIRTUAL = (1 << 10), /*virtual cap for q3a module*/
} stats_port_cap_type;

/** stats_mode_change_event_data
 *    @reserved_id:       The reserved ID
 *    @stream_type:       The type of the stream
 *
 * This is event data for mode change event
 *
 **/
typedef struct _stats_mode_change_event_data {
    unsigned int reserved_id;
    unsigned int  stream_type;
} stats_mode_change_event_data;

/*Data sturcture for awb ends*/

mct_module_t* stats_module_init(const char *name);
void          stats_module_deinit(mct_module_t *mod);

#endif /* __STATS_MODULE_H__ */
