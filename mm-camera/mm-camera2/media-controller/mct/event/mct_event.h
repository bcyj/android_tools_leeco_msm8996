/* mct_event.h
 *                                                           .
 * Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MCT_EVENT_H__
#define __MCT_EVENT_H___

#include "media_controller.h"
#include "mct_event_stats.h"

/* 1st level event type */
typedef enum _mct_event_type {
  MCT_EVENT_INVALID      = 0,
  MCT_EVENT_CONTROL_CMD  = 1, /* Control command from Media Controller */
  MCT_EVENT_MODULE_EVENT = 2, /* Event among modules                   */
  MCT_EVENT_MAX   = 3
} mct_event_type;

/** _mct_event_direction
 *    @MCT_EVENT_UPSTREAM: upstream event;
 *    @MCT_EVENT_DOWNSTREAM: downstream event;
 *    @MCT_EVENT_BOTH: both upstream and downstream event, this is rare
 *                     case only used by a few modules.
 **/
typedef enum _mct_event_direction {
  MCT_EVENT_UPSTREAM,
  MCT_EVENT_DOWNSTREAM,
  MCT_EVENT_BOTH,
  MCT_EVENT_NONE
} mct_event_direction;

#if 0
#define NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS  9
#define NUM_AUTOFOCUS_HORIZONTAL_GRID 9
#define NUM_AUTOFOCUS_VERTICAL_GRID 9
#define AUTOFOCUS_STATS_BUFFER_MAX_ENTRIES 1056
#endif
/** _mct_event_module_type
 *
 *
 *
 * 2nd level event type with type of struct mct_event_module_t
 **/
typedef enum _mct_event_module_type {
  MCT_EVENT_MODULE_SET_STREAM_CONFIG,
  MCT_EVENT_MODULE_SET_STREAM_CONFIG_FOR_FLASH,
  MCT_EVENT_MODULE_IFACE_SET_STREAM_CONFIG,
  MCT_EVENT_MODULE_SET_AF_ROLLOFF_PARAMS,
  MCT_EVENT_MODULE_USE_NATIVE_BUFFER,   /* NULL payload */
  MCT_EVENT_MODULE_DIVERT_BUF_TO_STATS, /* NULL payload */
  MCT_EVENT_MODULE_SET_CHROMATIX_PTR,
  MCT_EVENT_MODULE_GET_CHROMATIX_PTR,
  MCT_EVENT_MODULE_SET_RELOAD_CHROMATIX, /* EZTuning reload chromatix*/
  MCT_EVENT_MODULE_STATS_UPDATE_ISP,
  MCT_EVENT_MODULE_STATS_UPDATE, /*shall be replaced by individual update*/
  MCT_EVENT_MODULE_STATS_AEC_UPDATE,
  MCT_EVENT_MODULE_STATS_AWB_UPDATE,
  MCT_EVENT_MODULE_STATS_AF_UPDATE,
  MCT_EVENT_MODULE_SENSOR_LENS_POSITION_UPDATE,
  MCT_EVENT_MODULE_STATS_CONFIG_UPDATE,
  MCT_EVENT_MODULE_SET_FLASH_MODE, /* cam_flash_mode_t */
  MCT_EVENT_MODULE_STATS_AFD_UPDATE,
  MCT_EVENT_MODULE_STATS_ASD_UPDATE, /* 10 */
  MCT_EVENT_MODULE_STATS_GYRO_STATS,
  MCT_EVENT_MODULE_STATS_DATA,
  MCT_EVENT_MODULE_STATS_GET_DATA,
  MCT_EVENT_MODULE_3A_GET_CUR_FPS,
  MCT_EVENT_MODULE_SOF,
  MCT_EVENT_MODULE_SET_DIGITAL_GAIN, /* float * */
  MCT_EVENT_MODULE_STATS_GET_THREAD_OBJECT,
  MCT_EVENT_MODULE_SET_AF_TUNE_PTR, /* af_algo_tune_parms_t * */
  MCT_EVENT_MODULE_SET_RELOAD_AFTUNE, /*EZTuning reload aftune*/
  MCT_EVENT_MODULE_SOF_NOTIFY, /* mct_bus_msg_isp_sof_t * */
  MCT_EVENT_MODULE_BUF_DIVERT, /* 20 */ /* isp_buf_divert_t * */
  MCT_EVENT_MODULE_BUF_DIVERT_ACK, /* isp_buf_divert_ack_t * */
  MCT_EVENT_MODULE_FRAME_IND,
  MCT_EVENT_MODULE_FRAME_DONE,
  MCT_EVENT_MODULE_ISP_DIS_CONFIG, /* isp_dis_config_info_t */
  MCT_EVENT_MODULE_STREAM_CROP, /* mct_bus_msg_stream_crop_t */
  MCT_EVENT_MODULE_ISP_OUTPUT_DIM, /* mct_stream_info_t */
  MCT_EVENT_MODULE_ISPIF_OUTPUT_INFO, /* ispif_output_info_t */
  MCT_EVENT_MODULE_GET_GYRO_DATA, /* mct_event_gyro_data_t */
  MCT_EVENT_MODULE_STATS_DIS_UPDATE,
  MCT_EVENT_MODULE_ISP_SKIP_PATTERN, /* isp_stream_skip_pattern_t */
  MCT_EVENT_MODULE_STATS_POST_TO_BUS,
  MCT_EVENT_MODULE_FACE_INFO, /* mct_face_info_t */
  MCT_EVENT_MODULE_MODE_CHANGE,
  MCT_EVENT_MODULE_PPROC_GET_AEC_UPDATE,
  MCT_EVENT_MODULE_PPROC_GET_AWB_UPDATE,
  MCT_EVENT_MODULE_PPROC_DIVERT_INFO, /* pproc_divert_info_t */
  MCT_EVENT_MODULE_META_CHANNEL_DIVERT, /* meta_channel_buf_divert_request_t */
  MCT_EVENT_MODULE_ISP_PRIVATE_EVENT,
  MCT_EVENT_MODULE_GET_ISP_TABLES, /* mct_isp_table_t */
  MCT_EVENT_MODULE_ISP_AWB_UPDATE, /* awb_update_t */
  MCT_EVENT_MODULE_ISP_GAMMA_UPDATE, /* pointer to uint16_t table[64]; */
  MCT_EVENT_MODULE_ISP_STREAMON_DONE, /* no payload */
  MCT_EVENT_MODULE_SENSOR_META_CONFIG,  /* sensor_meta_t */
  MCT_EVENT_MODULE_ISP_META_CONFIG,     /* sensor_meta_t */
  MCT_EVENT_MODULE_ISP_CHANGE_OP_PIX_CLK, /* uint32_t */
  MCT_EVENT_MODULE_ISP_NO_RESOURCE, /* no payload */
  MCT_EVENT_MODULE_PREVIEW_STREAM_ID, /* int */
  MCT_EVENT_MODULE_SENSOR_ROLLOFF, /* mct_event_stats_isp_rolloff_t */
  MCT_EVENT_MODULE_SET_LIVESHOT_CHROMATIX_PTR,
    /* modules_liveshot_Chromatix_t */
  MCT_EVENT_MODULE_SET_FAST_AEC_CONVERGE_MODE, /* mct_fast_aec_mode_t */
  MCT_EVENT_MODULE_FAST_AEC_CONVERGE_ACK,
  MCT_EVENT_MODULE_QUERY_DIVERT_TYPE, /* uint32_t divert_mask */
  MCT_EVENT_MODULE_PP_SUBMOD_POST_TO_BUS, /* mct_bus_msg_t */
  MCT_EVENT_MODULE_PPROC_SET_OUTPUT_BUFF,
  MCT_EVENT_MODULE_LED_STATE_TIMEOUT,
  MCT_EVENT_MODULE_LED_AF_UPDATE,
  MCT_EVENT_MODULE_IMGLIB_AF_CONFIG, /* mct_imglib_af_config_t */
  MCT_EVENT_MODULE_IMGLIB_AF_OUTPUT, /* mct_imglib_af_output_t */
  MCT_EVENT_MODULE_SENSOR_BRACKET_CTRL, /* mct_bracket_ctrl_t */
  MCT_EVENT_MODULE_GET_AF_SW_STATS_FILTER_TYPE, /* mct_imglib_af_output_t */
  MCT_EVENT_MODULE_HFR_MODE_NOTIFY,
  MCT_EVENT_MODULE_GET_AEC_LUX_INDEX,
  MCT_EVENT_MODULE_START_STOP_STATS_THREADS,
  MCT_EVENT_MODULE_ISP_STATS_INFO, /* mct_stats_info_t */
  MCT_EVENT_MODULE_ISPIF_RESET,
  MCT_EVENT_MODULE_ISP_RESTART,
  MCT_EVENT_MODULE_SENSOR_UPDATE_FPS,
  MCT_EVENT_MODULE_MAX
} mct_event_module_type_t;

typedef struct {
  cam_intf_parm_type_t type;
  void *parm_data;
} mct_event_control_parm_t;

/** _mct_event_control_type
 *    @MCT_EVENT_CONTROL_TEST:
 *    @MCT_EVENT_CONTROL_STREAMON:
 *    @MCT_EVENT_CONTROL_STREAMOFF:
 *
 * 2nd level event type with type of struct mct_event_control_t
 **/
typedef enum _mct_event_control_type {
  MCT_EVENT_CONTROL_TEST,
  MCT_EVENT_CONTROL_STREAMON,
  MCT_EVENT_CONTROL_STREAMOFF,
  MCT_EVENT_CONTROL_STREAMON_FOR_FLASH,
  MCT_EVENT_CONTROL_STREAMOFF_FOR_FLASH,

  MCT_EVENT_CONTROL_SET_PARM,
  MCT_EVENT_CONTROL_GET_PARM,
  MCT_EVENT_CONTROL_SOF,
  MCT_EVENT_CONTROL_DO_AF,
  MCT_EVENT_CONTROL_CANCEL_AF,
  MCT_EVENT_CONTROL_PREPARE_SNAPSHOT,
  MCT_EVENT_CONTROL_PARM_STREAM_BUF,
  MCT_EVENT_CONTROL_START_ZSL_SNAPSHOT,
  MCT_EVENT_CONTROL_STOP_ZSL_SNAPSHOT,
  MCT_EVENT_CONTROL_UPDATE_BUF_INFO,
  MCT_EVENT_CONTROL_DEL_OFFLINE_STREAM,
  MCT_EVENT_CONTROL_MAX
} mct_event_control_type_t;

typedef enum _mct_ctrl_set_pram_type {
  MCT_CTRL_SET_SENSOR_PARAM,
  MCT_CTRL_SET_ISP_PARAM,
  MCT_CTRL_SET_STATS_PARAM,
  MCT_CTRL_SET_FRAME_PARAM,
}mct_ctrl_set_pram_t;

typedef struct _mct_ctrl_prarm_set_type {
  mct_ctrl_set_pram_t type;
  uint32_t            size;
  void                *param_set_data;
} mct_ctrl_prarm_set_t;

typedef struct _mct_event_module {
  mct_event_module_type_t type;
  uint32_t current_frame_id;
  void *module_event_data;
} mct_event_module_t;

typedef struct _mct_event_control {
  mct_event_control_type_t type;
  uint32_t current_frame_id;
  void *control_event_data;
} mct_event_control_t;

/** _mct_event:
 *    @type: first level of event type
 *    @identity:   0x0000 0000 (session/stream index)
 *    @direction:  upstream or downstream
 *    @data:       event data type of first level type
 *    @timestamp:  system clock timestamp
 *
 * Event source can be Pipeline, Stream or
 * any of the Modules, event Ports
 **/
struct _mct_event {
  mct_event_type       type;
  uint32_t             identity;
  mct_event_direction  direction;

  union {
    mct_event_control_t ctrl_event;
    mct_event_module_t  module_event;
  } u;

  signed long long     timestamp;
};

#define MCT_EVENT_CAST(obj)      ((mct_event_t *)(obj))
#define MCT_EVENT_DIRECTION(obj) (MCT_EVENT_CAST(obj)->direction)

mct_event_t *mct_event_package_event(/* FIX me */);

#endif /* __MCT_EVENT_H___*/
