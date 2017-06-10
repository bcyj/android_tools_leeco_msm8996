/* mct_bus.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MCT_BUS_H__
#define __MCT_BUS_H__

#include "media_controller.h"
#include "mct_queue.h"
#include "mct_event_stats.h"

/* mct_bus_msg_type_t:
 *
 */
typedef enum {
  MCT_BUS_MSG_ISPIF_SOF,
  MCT_BUS_MSG_ISP_SOF,      /* mct_bus_msg_isp_sof_t */
  MCT_BUS_MSG_ISP_STATS_AF, /* mct_bus_msg_isp_stats_af_t */
  MCT_BUS_MSG_ISP_STREAM_CROP, /* mct_bus_msg_stream_crop_t */
  MCT_BUS_MSG_Q3A_AF_STATUS,/* mct_bus_msg_isp_stats_af_t */
  MCT_BUS_MSG_FACE_INFO,    /* cam_face_detection_data_t */
  MCT_BUS_MSG_PREPARE_HW_DONE, /* cam_prep_snapshot_state_t */
  MCT_BUS_MSG_ZSL_TAKE_PICT_DONE,  /* cam_frame_idx_range_t */
  MCT_BUS_MSG_HIST_STATS_INFO, /* cam_hist_stats_t */
  MCT_BUS_MSG_SET_SENSOR_INFO, /*mct_bus_msg_sensor_metadata_t*/
  MCT_BUS_MSG_SET_STATS_AEC_INFO, /*mct_bus_msg_stats_aec_metadata_t*/
  MCT_BUS_MSG_SET_ISP_STATS_AWB_INFO, /*mct_bus_msg_isp_stats_awb_metadata_t*/
  MCT_BUS_MSG_SET_ISP_GAMMA_INFO, /*mct_bus_msg_isp_gamma_t*/
  MCT_BUS_MSG_SET_AEC_STATE, /*int32_t*/
  MCT_BUS_MSG_SET_AEC_PRECAPTURE_ID, /*int32_t*/
  MCT_BUS_MSG_SET_AEC_RESET, /* NULL */
  MCT_BUS_MSG_SET_AF_STATE, /*int32_t*/
  MCT_BUS_MSG_UPDATE_AF_FOCUS_POS, /* cam_focus_pos_info_t */
  MCT_BUS_MSG_SET_AF_TRIGGER_ID, /*int32_t*/
  MCT_BUS_MSG_SET_AF_ROI, /*cam_area_t*/
  MCT_BUS_MSG_SET_AF_FOCUS_INFO, /* cam_focus_distances_info_t */
  MCT_BUS_MSG_SET_AF_LENS_STATE, /* cam_af_lens_state_t */
  MCT_BUS_MSG_SET_AF_MODE,
  MCT_BUS_MSG_ERROR_MESSAGE,  /*mct_bus_msg_error_message_t*/
  MCT_BUS_MSG_AE_INFO,        /* cam_ae_params_t */
  MCT_BUS_MSG_AWB_INFO,       /* cam_awb_params_t */
  MCT_BUS_MSG_AE_EXIF_DEBUG_INFO,
  MCT_BUS_MSG_AWB_EXIF_DEBUG_INFO,
  MCT_BUS_MSG_AF_EXIF_DEBUG_INFO,
  MCT_BUS_MSG_ASD_EXIF_DEBUG_INFO,
  MCT_BUS_MSG_STATS_EXIF_DEBUG_INFO,
  MCT_BUS_MSG_SENSOR_INFO,    /*cam_sensor_params_t*/
  MCT_BUS_MSG_SENSOR_STARTING,/*NULL*/
  MCT_BUS_MSG_SENSOR_STOPPING,/*NULL*/
  MCT_BUS_MSG_NOTIFY_KERNEL,
  MCT_BUS_MSG_SEND_HW_ERROR,  /*NULL*/
  MCT_BUS_MSG_SENSOR_AF_STATUS,
  MCT_BUS_MSG_AUTO_SCENE_DECISION, /*mct_bus_msg_asd_decision_t*/
  MCT_BUS_MSG_ASD_HDR_SCENE_STATUS, /* mct_bus_msg_asd_hdr_status_t */
  MCT_BUS_MSG_META_VALID, /* mct_bus_msg_meta_valid */
  MCT_BUS_MSG_FRAME_INVALID, /* mct_bus_msg_frame_invalid */
  MCT_BUS_MSG_AE_EZTUNING_INFO, /* ae_eztuning_params_t */
  MCT_BUS_MSG_AWB_EZTUNING_INFO,/* awb_eztuning_params_t */
  MCT_BUS_MSG_AF_EZTUNING_INFO, /* af_eztuning_params_t */
  MCT_BUS_MSG_AF_MOBICAT_INFO,
  MCT_BUS_MSG_ISP_CHROMATIX_LITE,
  MCT_BUS_MSG_PP_CHROMATIX_LITE,
  MCT_BUS_MSG_ISP_META,
  MCT_BUS_MSG_SENSOR_META,
  MCT_BUS_MSG_REPROCESS_STAGE_DONE, /* NULL */
  MCT_BUS_MSG_PREPARE_HDR_ZSL_DONE, /* cam_prep_snapshot_state_t */
  MCT_BUS_MSG_SEND_EZTUNE_EVT,
  MCT_BUS_MSG_PP_SET_META,
  MCT_BUS_MSG_WM_BUS_OVERFLOW_RECOVERY,
  MCT_BUS_MSG_MAX
}mct_bus_msg_type_t;

#define MAX_STREAMS_NUM 8

// Use ISP SOF to send metadata when preview stream is present
// because stats and face data should be present for current frame.
// Otherwise use MCT_BUS_MSG_META_VALID (send on ISP buf divert).
#define SEND_METADATA_AT_SOF_MASK   (1 << CAM_STREAM_TYPE_PREVIEW)

typedef enum {
  ISP_META_REGISTER_DUMP,
  ISP_META_ROLLOFF_TBL,
  ISP_META_GAMMA_TBL,
  ISP_META_LINEARIZATION_TBL, /*not use for now*/
  ISP_META_LA_TBL, /*not use for now*/
  ISP_META_MAX,
} isp_meta_dump_t;

typedef struct {
  uint32_t dump_type; /*reg dump, DMI 16bit, DMI 32 bit, DMI 64 bit*/
  uint32_t len;
  uint32_t start_addr;
  void *isp_meta_dump;
} isp_meta_entry_t;

typedef struct {
  uint32_t isp_version; /* VFE40/32... */
  awb_gain_t awb_gain; /*r_gain, g_gain, b_gain*/
  uint32_t color_temp;
  uint32_t lux_idx;
  uint32_t num_entry;
  isp_meta_entry_t meta_entry[ISP_META_MAX]; /* reg dump, dmi tbl dump*/
} isp_meta_t;

/** _mct_bus_msg_isp_sof:
 *    @frame_id:    frame id
 *    @timestamp:   timestamp
 *    @num_streams: number of streams associated with sof
 *    @streamids:   stream id list.
 *
 *  This structure defines the sof bus message
 **/
typedef struct _mct_bus_msg_isp_sof {
  unsigned int frame_id;
  struct timeval timestamp;
  struct timeval mono_timestamp;
  int num_streams;
  uint32_t streamids[MAX_STREAMS_NUM];
  uint32_t prev_sent_streamids[MAX_STREAMS_NUM];
} mct_bus_msg_isp_sof_t;

typedef struct __mct_bus_msg_meta_valid {
  uint32_t frame_id;
  uint32_t meta_valid;
} mct_bus_msg_meta_valid;

typedef struct {
  void *chromatix_ptr;
  void *common_chromatix_ptr;
} mct_bus_msg_sensor_metadata_t;

typedef struct {
  char private_data[128];
} mct_bus_msg_stats_aec_metadata_t;

typedef struct {
  char private_data[400];
} mct_bus_msg_isp_stats_awb_metadata_t;

#define GAMA_DATA_SIZE (128*3)
typedef struct {
  char private_data[GAMA_DATA_SIZE];
} mct_bus_msg_isp_gamma_t;

typedef enum {
  MCT_ERROR_MSG_NOT_USED,
  MCT_ERROR_MSG_RSTART_VFE_STREAMING, /* restart VFE straeming */
  MCT_ERROR_MSG_RESUME_VFE_STREAMING, /* resume pending VFE streaming */
}mct_bus_msg_error_message_t;

/* mct_bus_msg_isp_stats_af_t:
 *
 */
typedef struct _mct_bus_msg_isp_stats_af {
  unsigned int frame_id;
  struct timeval timestamp;
} mct_bus_msg_isp_stats_af_t;

typedef struct {
  uint32_t frame_id;
  struct timeval timestamp;
  uint32_t session_id;
  uint32_t stream_id;
  uint32_t x_map; /* x map on input */
  uint32_t y_map; /* y map on input */
  uint32_t width_map;
  uint32_t height_map;
  uint32_t x; /* left */
  uint32_t y; /* top */
  uint32_t crop_out_x; /* width */
  uint32_t crop_out_y; /* height */
} mct_bus_msg_stream_crop_t;

/** mct_bus_msg_af_status:
  *  AF status that needs to be sent to HAL
  *
  *  @focus_state: current focus status
  *
  *  @focus_distance: focus distance
  **/
typedef struct _mct_bus_msg_af_status {
  cam_autofocus_state_t focus_state;
  cam_focus_distances_info_t f_distance;
  int32_t focus_pos;
} mct_bus_msg_af_status_t;

/** ASD decision to be sent to HAL
 * scene - S_NORMAL <= scene < S_MAX
 */
typedef struct {
  cam_auto_scene_t scene;
} mct_bus_msg_asd_decision_t;

typedef struct _mct_bus_msg_asd_hdr_status {
  uint32_t is_hdr_scene;
  float    hdr_confidence;
} mct_bus_msg_asd_hdr_status_t;

/* mct_bus_msg_t:
 *
 */
typedef struct _mct_bus_msg {
  unsigned int sessionid;
  mct_bus_msg_type_t type;
  unsigned int size;
  void *msg;
  uint32_t thread_wait_time;
} mct_bus_msg_t;

typedef boolean (*post_msg_to_bus_func)
  (mct_bus_t *bus, mct_bus_msg_t *msg);

struct _mct_bus {
  unsigned int session_id;
  mct_queue_t     *bus_queue;

  /*This flag set to 1 when message pushed to Q */
  int bus_cmd_q_flag;

  pthread_mutex_t bus_msg_q_lock;
  pthread_mutex_t *mct_mutex;
  pthread_cond_t  *mct_cond;
  post_msg_to_bus_func post_msg_to_bus;
  pthread_mutex_t bus_sof_msg_lock;
  pthread_cond_t  bus_sof_msg_cond;
  pthread_mutex_t bus_sof_init_lock;
  pthread_cond_t  bus_sof_init_cond;
  pthread_t       bus_sof_tid;
  int             thread_run;
  mct_bus_msg_type_t msg_to_send_metadata;
  uint32_t        thread_wait_time;
};

void mct_bus_queue_flush(mct_bus_t *bus);
mct_bus_t *mct_bus_create(unsigned int session);
void mct_bus_destroy(mct_bus_t *bus);

#endif /* __MCT_BUS_H__ */
