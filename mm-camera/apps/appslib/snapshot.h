/*============================================================================

   Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef MM_CAMERA_SNAPSHOT_H
#define MM_CAMERA_SNAPSHOT_H
#include "camera.h"
#ifndef DISABLE_JPEG_ENCODING
#include "jpeg_encoder.h"
#endif
#include "exif.h"
#include "camera_defs_i.h"


typedef enum {
  SNAPSHOT_TYPE_RAW_CAPTURE,
  SNAPSHOT_TYPE_CAPTURE_ONLY,
  SNAPSHOT_TYPE_CAPTURE_AND_ENCODE,
  SNAPSHOT_TYPE_ZSL,
}snapshot_type_t;

mm_camera_status_t snapshot_create(void **,
  mm_camera_notify* , int );
mm_camera_status_t snapshot_set_capture_parms(void* ,
  capture_params_t* );
mm_camera_status_t snapshot_set_raw_capture_parms(void* ,
  raw_capture_params_t* );
#ifndef DISABLE_JPEG_ENCODING
mm_camera_status_t snapshot_set_encode_parms(void* , encode_params_t* );
#endif
mm_camera_status_t snapshot_init(void* , snapshot_type_t);
mm_camera_status_t snapshot_delete(void* );
mm_camera_status_t snapshot_start(void* );
mm_camera_status_t snapshot_cancel(void* );
mm_camera_status_t snapshot_add_buffers(void* ,struct msm_pmem_info* );
#ifndef DISABLE_JPEG_ENCODING
mm_camera_status_t snapshot_start_encode(void* );
#endif
mm_camera_status_t snapshot_set_zsl_streaming_parms(void* , zsl_params_t* );
mm_camera_status_t snapshot_set_zsl_capture_parms(void* handle,
  zsl_capture_params_t* , snapshot_type_t);
#endif //MM_CAMERA_SNAPSHOT_H
