/*============================================================================

   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __MM_CAMERA_INTERFACE_H__
#define __MM_CAMERA_INTERFACE_H__
#include "camera.h"

typedef enum {
  MM_CAMERA_SUCCESS,
  MM_CAMERA_ERR_GENERAL,
  MM_CAMERA_ERR_NO_MEMORY,
  MM_CAMERA_ERR_NOT_SUPPORTED,
  MM_CAMERA_ERR_INVALID_INPUT,
  MM_CAMERA_ERR_INVALID_OPERATION, /* 5 */
  MM_CAMERA_ERR_ENCODE,
  MM_CAMERA_ERR_BUFFER_REG,
  MM_CAMERA_ERR_PMEM_ALLOC,
  MM_CAMERA_ERR_CAPTURE_FAILED,
  MM_CAMERA_ERR_CAPTURE_TIMEOUT, /* 10 */
}mm_camera_status_t;

typedef enum {
  CAMERA_MODE_STREAM_NONE,
  CAMERA_MODE_STREAM_VIDEO,
  CAMERA_MODE_STREAM_ZSL,
  CAMERA_MODE_STREAM_PREVIEW,
}mm_camera_stream_mode_t;

/* Add enumenrations at the bottom */
typedef enum {
  /* 1st 32 params*/
  CAMERA_PARM_PICT_SIZE,
  CAMERA_PARM_ZOOM_RATIO,
  CAMERA_PARM_HISTOGRAM,
  CAMERA_PARM_DIMENSION,
  CAMERA_PARM_FPS,
  CAMERA_PARM_FPS_MODE, /*5*/
  CAMERA_PARM_EFFECT,
  CAMERA_PARM_EXPOSURE_COMPENSATION,
  CAMERA_PARM_EXPOSURE,
  CAMERA_PARM_SHARPNESS,
  CAMERA_PARM_CONTRAST, /*10*/
  CAMERA_PARM_SATURATION,
  CAMERA_PARM_BRIGHTNESS,
  CAMERA_PARM_WHITE_BALANCE,
  CAMERA_PARM_LED_MODE,
  CAMERA_PARM_ANTIBANDING, /*15*/
  CAMERA_PARM_ROLLOFF,
  CAMERA_PARM_CONTINUOUS_AF,
  CAMERA_PARM_FOCUS_RECT,
  CAMERA_PARM_AEC_ROI,
  CAMERA_PARM_AF_ROI, /*20*/
  CAMERA_PARM_HJR,
  CAMERA_PARM_ISO,
  CAMERA_PARM_BL_DETECTION,
  CAMERA_PARM_SNOW_DETECTION,
  CAMERA_PARM_BESTSHOT_MODE, /*25*/
  CAMERA_PARM_ZOOM,
  CAMERA_PARM_VIDEO_DIS,
  CAMERA_PARM_VIDEO_ROT,
  CAMERA_PARM_SCE_FACTOR,
  CAMERA_PARM_FD, /*30*/
  CAMERA_PARM_MODE,
  /* 2nd 32 bits */
  CAMERA_PARM_3D_FRAME_FORMAT,
  CAMERA_PARM_CAMERA_ID,
  CAMERA_PARM_CAMERA_INFO,
  CAMERA_PARM_PREVIEW_SIZE, /*35*/
  CAMERA_PARM_QUERY_FALSH4SNAP,
  CAMERA_PARM_FOCUS_DISTANCES,
  CAMERA_PARM_BUFFER_INFO,
  CAMERA_PARM_JPEG_ROTATION,
  CAMERA_PARM_JPEG_MAINIMG_QUALITY, /* 40 */
  CAMERA_PARM_JPEG_THUMB_QUALITY,
  CAMERA_PARM_ZSL_ENABLE,
  CAMERA_PARM_FOCAL_LENGTH,
  CAMERA_PARM_HORIZONTAL_VIEW_ANGLE,
  CAMERA_PARM_VERTICAL_VIEW_ANGLE, /* 45 */
  CAMERA_PARM_MCE,
  CAMERA_PARM_RESET_LENS_TO_INFINITY,
  CAMERA_PARM_SNAPSHOTDATA,
  CAMERA_PARM_HFR,
  CAMERA_PARM_REDEYE_REDUCTION, /* 50 */
  CAMERA_PARM_WAVELET_DENOISE,
  CAMERA_PARM_3D_DISPLAY_DISTANCE,
  CAMERA_PARM_3D_VIEW_ANGLE,
  CAMERA_PARM_PREVIEW_FORMAT,
  CAMERA_PARM_HFR_SIZE, /* 55 */
  CAMERA_PARM_3D_EFFECT,
  CAMERA_PARM_3D_MANUAL_CONV_RANGE,
  CAMERA_PARM_3D_MANUAL_CONV_VALUE,
  CAMERA_PARM_ENABLE_3D_MANUAL_CONVERGENCE,
  CAMERA_PARM_HDR,  /* 60 */
  CAMERA_PARM_AEC_MTR_AREA,
  CAMERA_PARM_ASD_ENABLE,
  CAMERA_PARM_AEC_LOCK,
  CAMERA_PARM_LIVESHOT_MAIN,
  CAMERA_PARM_AWB_LOCK, /* 65 */
  CAMERA_PARM_RECORDING_HINT,
  CAMERA_PARM_HFR_SKIP,
} camera_parm_type_t;

typedef enum {
  CAMERA_OPS_STREAMING_PREVIEW,
  CAMERA_OPS_STREAMING_ZSL,
  CAMERA_OPS_STREAMING_VIDEO,
  CAMERA_OPS_CAPTURE, /*not supported*/
  CAMERA_OPS_FOCUS,
  CAMERA_OPS_GET_PICTURE, /*5*/
  CAMERA_OPS_PREPARE_SNAPSHOT,
  CAMERA_OPS_SNAPSHOT,
  CAMERA_OPS_LIVESHOT,
  CAMERA_OPS_RAW_SNAPSHOT,
  CAMERA_OPS_VIDEO_RECORDING, /*10*/
  CAMERA_OPS_REGISTER_BUFFER,
  CAMERA_OPS_UNREGISTER_BUFFER,
  CAMERA_OPS_CAPTURE_AND_ENCODE,
  CAMERA_OPS_RAW_CAPTURE,
  CAMERA_OPS_ENCODE, /*15*/
  CAMERA_OPS_ZSL_STREAMING_CB,
}mm_camera_ops_type_t;

typedef enum {
  CAMERA_MODE_NONE,
  CAMERA_MODE_VIDEO,
  CAMERA_MODE_ZSL,
}mm_camera_mode_t;


typedef struct {
  /* used for querying the tables from mm_camera*/
  mm_camera_status_t (*mm_camera_query_parms) (camera_parm_type_t parm_type,
    void** pp_values, uint32_t* p_count);

  /* set a parm’s current value */
  mm_camera_status_t (*mm_camera_set_parm) (camera_parm_type_t parm_type,
    void* p_value);

  /* get a parm’s current value */
  mm_camera_status_t(*mm_camera_get_parm) (camera_parm_type_t parm_type,
    void* p_value);

  /* check if the parm is supported */
  int8_t (*mm_camera_is_supported) (camera_parm_type_t parm_type);

  /* check if the sub parm is supported */
  int8_t (*mm_camera_is_parm_supported) (camera_parm_type_t parm_type,
   void* sub_parm);

}mm_camera_config;

typedef struct {
  /* init the functionality */
  mm_camera_status_t (*mm_camera_init) (mm_camera_ops_type_t ops_type,
    void* parm1, void* parm2);

  /* start the functionality */
  mm_camera_status_t (*mm_camera_start) (mm_camera_ops_type_t ops_type,
    void* parm1, void* parm2);

  /* stop the functionality */
  mm_camera_status_t(*mm_camera_stop) (mm_camera_ops_type_t ops_type,
    void* parm1, void* parm2);

  /* init the functionality */
  mm_camera_status_t (*mm_camera_deinit) (mm_camera_ops_type_t ops_type,
    void* parm1, void* parm2);

  /* check if the ops is supported */
  int8_t (*mm_camera_is_supported) (mm_camera_ops_type_t ops_type);

} mm_camera_ops;

typedef enum {
  FRAME_READY,
  SNAPSHOT_DONE,
  SNAPSHOT_FAILED,
  JPEG_ENC_DONE,
  JPEG_ENC_FAILED,
} mm_camera_event_type;

typedef enum {
  MM_CAMERA_YCBCR_420,
  MM_CAMERA_YCRCB_420
}mm_camera_format_type;

#ifndef LIVESHOT_STATUS
#define LIVESHOT_STATUS
typedef enum {
  LIVESHOT_SUCCESS,
  LIVESHOT_ENCODE_ERROR,
  LIVESHOT_UNKNOWN_ERROR,
}liveshot_status;
#endif // LIVESHOT_STATUS

typedef struct {
  uint8_t* ptr;
  uint32_t filled_size;
  uint32_t size;
  int32_t fd;
  uint32_t offset;
}mm_camera_buffer_t;

typedef struct {
  mm_camera_event_type event_type;
  union {
   mm_camera_buffer_t* encoded_frame;
   struct msm_frame* preview_frame;
   struct msm_frame* yuv_frames[2]; /*0 postview, 1 main img*/
   struct msm_frame* raw_frame;
  }event_data;
} mm_camera_event;

typedef struct {
  int8_t (*on_event)(mm_camera_event* evt);
  void (*video_frame_cb) (struct msm_frame *);
  void (*preview_frame_cb) (struct msm_frame *);
  void (*on_error_event) (camera_error_type err);
  void (*camstats_cb) (camstats_type, camera_preview_histogram_info*);
  void (*jpegfragment_cb)(uint8_t *, uint32_t);
  void (*on_jpeg_event)(uint32_t status);
  void (*on_liveshot_event)(liveshot_status status, uint32_t jpeg_size);
} mm_camera_notify;

/* Initializes the config interface */
mm_camera_status_t mm_camera_init(mm_camera_config * configIntf,
  mm_camera_notify* notifyIntf, mm_camera_ops* opsIntf,
  uint8_t dyn_device_query);

/* De-initializes the config interface */
mm_camera_status_t mm_camera_deinit();

/* For destroying the object */
mm_camera_status_t mm_camera_destroy();

mm_camera_status_t mm_camera_exec();

mm_camera_status_t mm_camera_get_camera_info(camera_info_t* p_cam_info,
  int* p_num_cameras);

mm_camera_status_t camera_ops_init (mm_camera_ops_type_t ops_type,
  void* parm1, void* parm2);

uint8_t get_device_id();

mm_camera_notify* get_notify_obj();
typedef struct {
  uint32_t picture_width;
  uint32_t picture_height;
  uint32_t postview_width;
  uint32_t postview_height;
  uint32_t thumbnail_width;
  uint32_t thumbnail_height;
  int num_captures;
}capture_params_t;

typedef struct {
  uint32_t CbOffset;
  uint32_t CrOffset;
}yv12_format_parms_t;

typedef struct {
  uint32_t picture_width;
  uint32_t picture_height;
  uint32_t preview_width;
  uint32_t preview_height;
  uint8_t  useExternalBuffers;
}zsl_params_t;

typedef struct {
  int num_captures;
  uint32_t thumbnail_width;
  uint32_t thumbnail_height;
}zsl_capture_params_t;

typedef struct {
  uint32_t raw_picture_width;
  uint32_t raw_picture_height;
  int num_captures;
}raw_capture_params_t;

#ifndef DISABLE_JPEG_ENCODING
typedef struct {
  exif_tags_info_t* exif_data;
  int exif_numEntries;
  mm_camera_buffer_t* p_output_buffer;
  uint8_t buffer_count;
  uint32_t rotation;
  uint32_t quality;
  int y_offset;
  int cbcr_offset;
  /* bitmask for the images to be encoded. if capture_and_encode
   * option is selected, all the images will be encoded irrespective
   * of bitmask.
   */
  uint8_t encodeBitMask;
  uint32_t output_picture_width;
  uint32_t output_picture_height;
  int format3d;
}encode_params_t;
#endif /* DISABLE_JPEG_ENCODING */
#endif /*__MM_CAMERA_INTERFACE_H__*/
