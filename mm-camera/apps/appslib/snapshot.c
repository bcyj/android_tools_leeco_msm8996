/*============================================================================
   Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include "camera_dbg.h"
#include "camera.h"
#include "cam_mmap.h"
#include "cam_fifo.h"
#include "jpeg_encoder.h"
#include "mm_camera_interface.h"
#include "snapshot.h"
#include <time.h>

#ifdef _TARGET_8660_
#include "mpo_encoder.h"
#endif

#define ACTIVE_SNAPSHOT_BUFFERS 3
#define MAX_EXIF_TABLE_ENTRIES 14
#define DBG_DUMP_JPEG_IMG 0
#define DBG_DUMP_ZSL_YUV_FRAME 1
#define BUFF_SIZE_50 50
#define BUFF_SIZE_128 128

#define GET_REL_TIME_IN_MS(x,y) ( ((y.tv_sec - x.tv_sec) * 1000) \
  + (y.tv_nsec - x.tv_nsec)/1000000)

#define ZSL_DELAY 200 /* 200ms delay */
#ifndef ABS
#define ABS(x) (((x) < 0) ? -(x) : (x))
#endif

static pthread_cond_t  zsl_thread_cond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t zsl_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  snapshot_state_cond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t snapshot_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  encode_thread_cond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t encode_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  encode_wait_cond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t encode_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct timezone tz;

static const int32_t SNAPSHOT_THRESHOLD_TIME_LOW = -500;
static const int32_t SNAPSHOT_THRESHOLD_TIME_HIGH = 50;

typedef enum {
  SNAPSHOT_STATE_CAPTURE_WAIT,
  SNAPSHOT_STATE_CAPTURE_DONE,
  SNAPSHOT_STATE_CAPTURE_ERROR,
  SNAPSHOT_STATE_CAPTURE_DONE_ALL,
  SNAPSHOT_STATE_CAPTURE_CANCEL
}snapshot_capture_state_t;

typedef enum {
  SNAPSHOT_STATE_ENCODE_IDLE,
  SNAPSHOT_STATE_ENCODE_WAIT,
  SNAPSHOT_STATE_ENCODE_DONE,
  SNAPSHOT_STATE_ENCODE_ERROR,
  SNAPSHOT_STATE_ENCODE_DONE_ALL,
  SNAPSHOT_STATE_ENCODE_CANCEL
}snapshot_encode_state_t;

typedef enum {
  SNAPSHOT_STATE_IDLE = 0,
  SNAPSHOT_STATE_INIT,
  SNAPSHOT_STATE_CAPTURE,
  SNAPSHOT_STATE_STOPPED,
  SNAPSHOT_STATE_DEINIT
}snapshot_state_t;

typedef struct {
  struct msm_frame img;
  uint32_t offset;
  uint32_t size;
  common_crop_t frame_crop_info;
  struct timeval time_stamp;
}snapshot_frame_info_t;

typedef struct {
  pthread_t zsl_thread_id;
  int8_t zsl_thread_exit;
  int8_t zsl_thread_started;
  int8_t start_capture;
  struct timeval time_stamp;
  zsl_params_t zsl_str_parms;
  zsl_capture_params_t zsl_capture_params;
  snapshot_type_t snapshot_type;
}snapshot_zsl_info_t;

typedef struct {
  capture_params_t capture_params;
  int8_t zsl_enable;
#ifndef DISABLE_JPEG_ENCODING
  encode_params_t encode_params;
#endif
  raw_capture_params_t raw_capture_params;
  snapshot_type_t snapshot_type;
  snapshot_state_t state;
  snapshot_frame_info_t snapshot_frame[MAX_SNAPSHOT_BUFFERS];
  snapshot_frame_info_t thumb_frame[MAX_SNAPSHOT_BUFFERS];
  uint32_t out_buffer_offset;
  int capture_complete_cnt;
  int encode_complete_cnt;
  mm_camera_status_t last_error;
  struct fifo_queue encode_thumb_queue;
  struct fifo_queue encode_queue;
  struct fifo_queue capture_queue;
  struct fifo_queue capture_thumb_queue;
  int8_t encode_thread_exit;
  pthread_t encode_thread_id;
  common_crop_t crop_info;
  snapshot_zsl_info_t zsl_info;
  snapshot_capture_state_t capture_state;
  snapshot_encode_state_t encode_state;
  struct timespec capture_start_time;
  int picfd;
  mm_camera_notify* notifyIntf;
  int num_captures;
  int current_buffer_count;
  int current_thumb_buffer_count;
  int8_t use_external_buffers;
  int camfd;
  struct msm_frame* p_capturedFrame[MAX_SNAPSHOT_BUFFERS];
  int capturedFrameCount;
  int terminatefd[2];
  uint8_t out_buffer_index;
  int ion_dev_fd;
}snapshot_context_t;

static mm_camera_status_t snapshot_delete_buffers(snapshot_context_t* context);
static mm_camera_status_t snapshot_register_buffers(snapshot_context_t* context,
  int8_t unregister);
static mm_camera_status_t snapshot_thumbnail_register_buffers(
  snapshot_context_t* context, int8_t unregister);
static mm_camera_status_t snapshot_create_thumbnail_buffers(snapshot_context_t*
  context);
static mm_camera_status_t snapshot_create_snapshot_buffers(snapshot_context_t* context);
static void* zsl_snapshot_thread_func(void *data);
#ifndef DISABLE_JPEG_ENCODING
extern void (*mmcamera_jpegfragment_callback)(uint8_t *buff_ptr,
  uint32_t buff_size);
extern void (*mmcamera_jpeg_callback)(jpeg_event_t status);
#endif

static mm_camera_status_t snapshot_issue_cmd(cam_ctrl_type type,
  uint16_t length, void *value, uint32_t timeout_ms, int camfd);
#ifndef DISABLE_JPEG_ENCODING
#ifdef _TARGET_7x2x_
static mm_camera_status_t snapshot_jpeg_encode(snapshot_context_t* context,
  struct msm_frame* main_frame, struct msm_frame* thumb_frame);
#else
static mm_camera_status_t snapshot_jpeg_encode(snapshot_context_t* context,
  struct msm_frame* main_frame);
#endif
#endif
static mm_camera_status_t snapshot_get_picture(snapshot_context_t* context,
  struct msm_frame* captFrame);
static mm_camera_status_t snapshot_release_picture_frame(snapshot_context_t* context,
  struct msm_frame* captFrame);
static mm_camera_status_t snapshot_start_multishot(snapshot_context_t* context);
static mm_camera_status_t snapshot_start_raw_snapshot(snapshot_context_t* context);
#ifndef DISABLE_JPEG_ENCODING
void snapshot_jpeg_cb(jpeg_event_t event);
void snapshot_jpeg_fragment_cb(uint8_t *, uint32_t);
static void* encode_thread_func(void *data);
#endif

static snapshot_context_t* g_snapshot_context = NULL;

int8_t (*encode_init)();
void (*encode_join)();
int8_t (*encoder_encode)(const cam_ctrl_dimension_t *,
  const uint8_t *, int,
  const uint8_t *, int,
  common_crop_t *, exif_tags_info_t *,
  int, const int32_t,
  cam_point_t *, cam_point_t *, int);

/*===========================================================================
FUNCTION      snapshot_create

DESCRIPTION
===========================================================================*/
mm_camera_status_t snapshot_create(void **handle,
  mm_camera_notify* notifyIntf, int camfd)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  pthread_mutex_lock(&snapshot_state_mutex);
  if (NULL != g_snapshot_context) {
    pthread_mutex_unlock(&snapshot_state_mutex);
    CDBG("%s: snapshot already created", __func__);
    status = MM_CAMERA_ERR_INVALID_OPERATION;
    goto snapshot_create_end;
  }
  pthread_mutex_unlock(&snapshot_state_mutex);
  g_snapshot_context = (snapshot_context_t*)malloc(sizeof(snapshot_context_t));
  if (NULL == g_snapshot_context) {
    status = MM_CAMERA_ERR_NO_MEMORY;
    CDBG("%s: g_snapshot_context is NULL", __func__);
    goto snapshot_create_end;
  }
  memset(g_snapshot_context, 0, sizeof(snapshot_context_t));
  g_snapshot_context->notifyIntf = notifyIntf;
  g_snapshot_context->encode_thread_id = -1;
  g_snapshot_context->picfd = -1;
  g_snapshot_context->current_buffer_count = 0;
  g_snapshot_context->current_thumb_buffer_count = 0;
  g_snapshot_context->use_external_buffers = true;
  g_snapshot_context->camfd = camfd;

  pthread_mutex_init(&g_snapshot_context->capture_queue.mut, NULL);
  pthread_cond_init(&g_snapshot_context->capture_queue.wait, NULL);
  g_snapshot_context->capture_queue.name = "snapshot_capture_q";

  pthread_mutex_init(&g_snapshot_context->capture_thumb_queue.mut, NULL);
  pthread_cond_init(&g_snapshot_context->capture_thumb_queue.wait, NULL);
  g_snapshot_context->capture_thumb_queue.name = "snapshot_capture_thumb_q";

  pthread_mutex_init(&g_snapshot_context->encode_queue.mut, NULL);
  pthread_cond_init(&g_snapshot_context->encode_queue.wait, NULL);
  g_snapshot_context->encode_queue.name = "snapshot_encode_q";

  pthread_mutex_init(&g_snapshot_context->encode_thumb_queue.mut, NULL);
  pthread_cond_init(&g_snapshot_context->encode_thumb_queue.wait, NULL);
  g_snapshot_context->encode_thumb_queue.name = "snapshot_encode_thumb_q";

  if (pipe(g_snapshot_context->terminatefd)< 0) {
    CDBG_ERROR("%s: pipe creation failed\n", __func__);
    goto snapshot_create_end;
  }
#ifndef DISABLE_JPEG_ENCODING
  mmcamera_jpegfragment_callback = snapshot_jpeg_fragment_cb;
  mmcamera_jpeg_callback = snapshot_jpeg_cb;
#endif
snapshot_create_end:
  *handle = (void *)g_snapshot_context;
  CDBG("%s: exit status %d", __func__, status);
  return status;
}

/*===========================================================================
FUNCTION      snapshot_set_zsl_streaming_parms

DESCRIPTION
===========================================================================*/
mm_camera_status_t snapshot_set_zsl_streaming_parms(void* handle,
  zsl_params_t* p_params)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  CDBG("%s: ", __func__);
  snapshot_context_t* context = (snapshot_context_t*)handle;
  context->zsl_info.zsl_str_parms = *p_params;
  CDBG("%s, Dim = %dx%d, %dx%d\n",
       __func__,
       context->zsl_info.zsl_str_parms.picture_height,
       context->zsl_info.zsl_str_parms.picture_width,
       context->zsl_info.zsl_str_parms.preview_height,
       context->zsl_info.zsl_str_parms.preview_width);
  return status;
}

/*===========================================================================
FUNCTION      snapshot_set_zsl_capture_parms

DESCRIPTION
===========================================================================*/
mm_camera_status_t snapshot_set_zsl_capture_parms(void* handle,
  zsl_capture_params_t* p_capture_params, snapshot_type_t type)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  snapshot_context_t* context = (snapshot_context_t*)handle;
  CDBG("%s, state %d \n", __func__, context->state);
  pthread_mutex_lock(&snapshot_state_mutex);
  if (SNAPSHOT_TYPE_ZSL != context->snapshot_type) {
    pthread_mutex_unlock(&snapshot_state_mutex);
    status = MM_CAMERA_ERR_INVALID_OPERATION;
    goto snapshot_set_zsl_capture_parms_end;
  }
  pthread_mutex_unlock(&snapshot_state_mutex);
  CDBG("%s: thumbnail w=%u h=%u\n", __func__,
    p_capture_params->thumbnail_width,
    p_capture_params->thumbnail_height);
  context->zsl_info.zsl_capture_params = *p_capture_params;
  context->zsl_info.snapshot_type = type;
snapshot_set_zsl_capture_parms_end:
  CDBG("%s: exit status %d", __func__, status);
  return status;
}

/*===========================================================================
FUNCTION      snapshot_set_capture_parms

DESCRIPTION
===========================================================================*/
mm_camera_status_t snapshot_set_capture_parms(void* handle, capture_params_t*
  p_capture_params)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  snapshot_context_t* context = (snapshot_context_t*)handle;
  CDBG("%s, enter\n", __func__);
  pthread_mutex_lock(&snapshot_state_mutex);
  if ( (SNAPSHOT_STATE_INIT != context->state)
    || (SNAPSHOT_TYPE_RAW_CAPTURE == context->snapshot_type)) {
    pthread_mutex_unlock(&snapshot_state_mutex);
    status = MM_CAMERA_ERR_INVALID_OPERATION;
    goto snapshot_set_capture_parms_end;
  }
  pthread_mutex_unlock(&snapshot_state_mutex);
  CDBG("%s: w=%u h=%u\n", __func__,
    p_capture_params->picture_width,
    p_capture_params->picture_height);
  context->capture_params = *p_capture_params;
snapshot_set_capture_parms_end:
  CDBG("%s: exit status %d", __func__, status);
  return status;
}

/*===========================================================================
FUNCTION      snapshot_set_raw_capture_parms

DESCRIPTION
===========================================================================*/
mm_camera_status_t snapshot_set_raw_capture_parms(void* handle,
  raw_capture_params_t* p_raw_capture_params)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  snapshot_context_t* context = (snapshot_context_t*)handle;
  pthread_mutex_lock(&snapshot_state_mutex);
  if ( (SNAPSHOT_STATE_INIT != context->state)
    || (SNAPSHOT_TYPE_RAW_CAPTURE != context->snapshot_type)) {
    pthread_mutex_unlock(&snapshot_state_mutex);
    status = MM_CAMERA_ERR_INVALID_OPERATION;
    goto snapshot_set_raw_capture_parms_end;
  }
  pthread_mutex_unlock(&snapshot_state_mutex);
  context->raw_capture_params = *p_raw_capture_params;
snapshot_set_raw_capture_parms_end:
  CDBG("%s: exit status %d", __func__, status);
  return status;
}

/*===========================================================================
FUNCTION      snapshot_set_encode_parms

DESCRIPTION
===========================================================================*/
#ifndef DISABLE_JPEG_ENCODING
mm_camera_status_t snapshot_set_encode_parms(void* handle, encode_params_t*
  p_encode_params)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  snapshot_context_t* context = (snapshot_context_t*)handle;
  pthread_mutex_lock(&snapshot_state_mutex);
  if ((SNAPSHOT_TYPE_ZSL != context->snapshot_type) &&
    ((SNAPSHOT_STATE_INIT != context->state)
    || (SNAPSHOT_TYPE_RAW_CAPTURE == context->snapshot_type))) {
    pthread_mutex_unlock(&snapshot_state_mutex);
    status = MM_CAMERA_ERR_INVALID_OPERATION;
    goto snapshot_set_encode_parms_end;
  }
  pthread_mutex_unlock(&snapshot_state_mutex);
  context->encode_params = *p_encode_params;
snapshot_set_encode_parms_end:
  CDBG("%s: exit status %d", __func__, status);
  return status;
}
#endif

/*===========================================================================
FUNCTION      snapshot_init

DESCRIPTION
===========================================================================*/
mm_camera_status_t snapshot_init(void* handle, snapshot_type_t type)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  snapshot_context_t* context = (snapshot_context_t*)handle;
  CDBG("%s, enter \n", __func__);
  pthread_mutex_lock(&snapshot_state_mutex);
  context->snapshot_type = type;
  context->state = SNAPSHOT_STATE_INIT;
  pthread_mutex_unlock(&snapshot_state_mutex);

  if(SNAPSHOT_TYPE_ZSL == type) {
    context->capture_params.picture_width =
      context->zsl_info.zsl_str_parms.picture_width;
    context->capture_params.picture_height =
      context->zsl_info.zsl_str_parms.picture_height;
    context->capture_params.thumbnail_height =
      context->zsl_info.zsl_str_parms.preview_height;
    context->capture_params.thumbnail_width =
      context->zsl_info.zsl_str_parms.preview_width;

    g_snapshot_context->zsl_enable = true;
    if (FALSE == context->zsl_info.zsl_str_parms.useExternalBuffers) {
#ifdef USE_ION
      context->ion_dev_fd= open("/dev/ion", O_RDONLY | O_SYNC);
      if (context->ion_dev_fd < 0) {
        CDBG_ERROR("Ion dev open failed\n");
        CDBG_ERROR("Error is %s\n", strerror(errno));
        goto snapshot_init_end;
      }
#endif
      status = snapshot_create_snapshot_buffers(context);
      if (MM_CAMERA_SUCCESS != status) {
        CDBG("%s snapshot_create_snapshot_buffers failed", __func__);
        goto snapshot_init_end;
      }
      status = snapshot_create_thumbnail_buffers(context);
      if (MM_CAMERA_SUCCESS != status) {
        CDBG("%s snapshot_create_thumbnail_buffers failed", __func__);
        goto snapshot_init_end;
      }
      status = snapshot_register_buffers(context, false);
      if (MM_CAMERA_SUCCESS != status) {
        CDBG("%s snapshot_register_buffers failed", __func__);
        goto snapshot_init_end;
      }
      status = snapshot_thumbnail_register_buffers(context, false);
      if (MM_CAMERA_SUCCESS != status) {
        CDBG("%s snapshot_thumbnail_register_buffers failed", __func__);
        goto snapshot_init_end;
      }
    }

    /*create ZSL capture thread*/

    pthread_mutex_lock(&zsl_thread_mutex);
    g_snapshot_context->zsl_info.zsl_thread_exit = false;
    g_snapshot_context->zsl_info.zsl_thread_started = false;
    if ((pthread_create(&g_snapshot_context->zsl_info.zsl_thread_id, NULL,
      zsl_snapshot_thread_func, (void *)g_snapshot_context)) != 0) {
      CDBG("%s ZSL thread thread creation failed", __func__);
      pthread_mutex_unlock(&zsl_thread_mutex);
      goto snapshot_init_end;
    }
    if (!g_snapshot_context->zsl_info.zsl_thread_started) {
      CDBG("%s wait for ZSL snapshot thread to start", __func__);
      pthread_cond_wait(&zsl_thread_cond, &zsl_thread_mutex);
    }
    CDBG("%s ZSL snapshot thread started", __func__);
    pthread_mutex_unlock(&zsl_thread_mutex);
  }
snapshot_init_end:
  CDBG("%s Exit", __func__);
  return status;
}

/*===========================================================================
FUNCTION      snapshot_delete

DESCRIPTION
===========================================================================*/
mm_camera_status_t snapshot_delete(void* handle)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int ret = 0;
  int i = 0;
  char exit = 'y';

  CDBG("%s enter %p\n",__func__, handle);
  if (NULL == handle) {
    status = MM_CAMERA_ERR_INVALID_INPUT;
    goto snapshot_deinit_end;
  }
  snapshot_context_t* context = (snapshot_context_t*)handle;

  pthread_mutex_lock(&snapshot_state_mutex);
  if (SNAPSHOT_STATE_DEINIT == context->state) {
    CDBG_HIGH("%s: Already in deinit state", __func__);
    status = MM_CAMERA_ERR_INVALID_OPERATION;
    pthread_mutex_unlock(&snapshot_state_mutex);
    return status;
  }
  context->state = SNAPSHOT_STATE_DEINIT;
  pthread_mutex_unlock(&snapshot_state_mutex);

  /* unblock polling */
  if (context->picfd > 0) {
    ret = ioctl(context->picfd, MSM_CAM_IOCTL_UNBLOCK_POLL_PIC_FRAME);
    if (ret < 0) {
      CDBG_ERROR("%s: MSM_CAM_IOCTL_UNBLOCK_POLL_PIC_FRAME failed (%s).",
        __func__, strerror(errno));
    }
  }

  /* wait for encoder thread to exit */
  CDBG("%s context->encode_thread_id %d state %d", __func__,
    (int)context->encode_thread_id, context->encode_state);
  pthread_mutex_lock(&encode_thread_mutex);
  if (context->encode_thread_id >= 0) {
    pthread_mutex_unlock(&encode_thread_mutex);
    if (SNAPSHOT_STATE_ENCODE_WAIT == context->encode_state) {
      context->encode_state = SNAPSHOT_STATE_ENCODE_CANCEL;
      context->encode_thread_exit = true;
      pthread_cond_signal(&encode_thread_cond);
    }
    /* Signal encode queues */
    CDBG("%s: Signal encode queue", __func__);
    signal_queue(&context->encode_queue);
    signal_queue(&context->encode_thumb_queue);
    CDBG("%s: wait for thread to exit", __func__);
    ret = pthread_join(context->encode_thread_id, NULL);
    CDBG("%s: encoder thread exited", __func__);
  } else
    pthread_mutex_unlock(&encode_thread_mutex);

  pthread_mutex_lock(&snapshot_state_mutex);
  context->capture_state = SNAPSHOT_STATE_CAPTURE_CANCEL;
  pthread_mutex_unlock(&snapshot_state_mutex);

  pthread_mutex_lock(&zsl_thread_mutex);
  context->zsl_info.zsl_thread_exit = true;
  pthread_mutex_unlock(&zsl_thread_mutex);
  pthread_cond_signal(&zsl_thread_cond);

  ret = write(context->terminatefd[1], &exit, sizeof(exit));
  if (ret < 0)
    CDBG("%s: Snapshot thread terminate failed\n", __func__);

  if (context->zsl_enable) {
    pthread_mutex_lock(&zsl_thread_mutex);
    if (context->zsl_info.zsl_thread_id >= 0) {
      pthread_mutex_unlock(&zsl_thread_mutex);
      ret = pthread_join(context->zsl_info.zsl_thread_id, NULL);
    } else
      pthread_mutex_unlock(&zsl_thread_mutex);
  }

  /* signal all the mutex*/
  signal_queue(&context->capture_queue);
  signal_queue(&context->capture_thumb_queue);

  flush_and_destroy_queue(&context->capture_queue);
  flush_and_destroy_queue(&context->capture_thumb_queue);
  flush_and_destroy_queue(&context->encode_queue);
  flush_and_destroy_queue(&context->encode_thumb_queue);

  for (i=0; i < context->capturedFrameCount; i++) {
    if (context->p_capturedFrame[i]) {
      free(context->p_capturedFrame[i]);
      context->p_capturedFrame[i] = NULL;
    }
  }
  /* unregister and clear buffers*/
  if (context->zsl_enable &&
    (FALSE == context->zsl_info.zsl_str_parms.useExternalBuffers)) {
    pthread_mutex_lock(&zsl_thread_mutex);
    status = snapshot_register_buffers(context, true);
    if (MM_CAMERA_SUCCESS != status) {
      CDBG("%s snapshot_unregister_buffers failed", __func__);
    }

    status = snapshot_thumbnail_register_buffers(context, true);
    if (MM_CAMERA_SUCCESS != status) {
      CDBG("%s snapshot_thumbnail_unregister_buffers failed", __func__);
    }
    status = snapshot_delete_buffers(context);
    if (MM_CAMERA_SUCCESS != status) {
      CDBG("%s snapshot_delete_snapshot_buffers failed", __func__);
    }
#ifdef USE_ION
  close(context->ion_dev_fd);
#endif
    pthread_mutex_unlock(&zsl_thread_mutex);
  }

  if (g_snapshot_context->terminatefd[0] > 0) {
    close(g_snapshot_context->terminatefd[0]);
    g_snapshot_context->terminatefd[0] = -1;
  }
  if (g_snapshot_context->terminatefd[1] > 0) {
    close(g_snapshot_context->terminatefd[1]);
    g_snapshot_context->terminatefd[1] = -1;
  }

snapshot_deinit_end:
  CDBG("%s exit", __func__);
  if (handle) {
    free (handle);
  }
  g_snapshot_context = NULL;
  return status;
}

/*===========================================================================
FUNCTION      snapshot_start

DESCRIPTION
===========================================================================*/
mm_camera_status_t snapshot_start(void* handle)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  snapshot_context_t* context = (snapshot_context_t*)handle;

  CDBG("%s enter", __func__);
  /* check if encoder is in progress */
  mmcamera_util_profile("enter snapshot_start");
  pthread_mutex_lock(&encode_thread_mutex);
  if (context->encode_thread_id > 0) {
    CDBG("%s: encode in progress", __func__);
    status = MM_CAMERA_ERR_INVALID_OPERATION;
    pthread_mutex_unlock(&encode_thread_mutex);
    goto snapshot_start_end;
  }
  pthread_mutex_unlock(&encode_thread_mutex);

  if (context->zsl_enable) {
    pthread_mutex_lock(&zsl_thread_mutex);
    /* check if ZSL capture is in progress */
    if (context->zsl_info.start_capture) {
      CDBG("%s: capture in progress", __func__);
      status = MM_CAMERA_ERR_INVALID_OPERATION;
      pthread_mutex_unlock(&zsl_thread_mutex);
      goto snapshot_start_end;
    }
    pthread_mutex_unlock(&zsl_thread_mutex);
  }

  pthread_mutex_lock(&snapshot_state_mutex);
  /* check if the state is proper*/
  if ((SNAPSHOT_TYPE_ZSL != context->snapshot_type) &&
    (SNAPSHOT_STATE_INIT != context->state)) {
    status = MM_CAMERA_ERR_INVALID_OPERATION;
     CDBG("%s failed %d", __func__, status);
    pthread_mutex_unlock(&snapshot_state_mutex);
    goto snapshot_start_end;
  }
  context->capturedFrameCount = 0;
  context->state = SNAPSHOT_STATE_CAPTURE;
  context->capture_complete_cnt = 0;

  /* encoder is not running. Hence protection not required */
  context->encode_complete_cnt = 0;
  context->out_buffer_index = 0;
  context->encode_state = SNAPSHOT_STATE_ENCODE_IDLE;

  if ((SNAPSHOT_TYPE_CAPTURE_AND_ENCODE == context->snapshot_type)
    || (SNAPSHOT_TYPE_ZSL == context->snapshot_type)) {
    /* create encode thread */
#ifndef DISABLE_JPEG_ENCODING
    if ((pthread_create(&context->encode_thread_id, NULL,
      encode_thread_func, (void *)context)) != 0) {
      CDBG("%s Encode thread creation failed", __func__);
      status = MM_CAMERA_ERR_NO_MEMORY;
      goto snapshot_start_end;
    }
#endif
    CDBG("%s wait for encode thread to start", __func__);
    pthread_mutex_lock(&encode_thread_mutex);
    if (context->encode_state == SNAPSHOT_STATE_ENCODE_IDLE) {
      pthread_cond_wait(&encode_thread_cond, &encode_thread_mutex);
    }
    pthread_mutex_unlock(&encode_thread_mutex);
  }
  CDBG("%s: encoder thread started", __func__);
  pthread_mutex_unlock(&snapshot_state_mutex);
  if ((SNAPSHOT_TYPE_CAPTURE_AND_ENCODE == context->snapshot_type)
    || (SNAPSHOT_TYPE_CAPTURE_ONLY == context->snapshot_type)){
    status = snapshot_start_multishot(context);
  } else if(SNAPSHOT_TYPE_ZSL == context->snapshot_type){
    pthread_mutex_lock(&zsl_thread_mutex);
    context->zsl_info.start_capture = true;
    pthread_mutex_unlock(&zsl_thread_mutex);
    clock_gettime(CLOCK_REALTIME, &context->capture_start_time);
  } else {
    status = snapshot_start_raw_snapshot(context);
  }

snapshot_start_end:
  CDBG("%s: Exit %d", __func__, status);
  mmcamera_util_profile("snapshot_start: Exit");
  return status;
}

/*===========================================================================
FUNCTION      snapshot_start_encode

DESCRIPTION
===========================================================================*/
#ifndef DISABLE_JPEG_ENCODING
mm_camera_status_t snapshot_start_encode(void* handle)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  snapshot_context_t* context = (snapshot_context_t*)handle;
  int index = 0;
  CDBG("%s: type %d zsl_type %d [S]", __func__, context->snapshot_type,
    context->zsl_info.snapshot_type);
  if ((SNAPSHOT_TYPE_CAPTURE_ONLY != context->snapshot_type)
    && ((SNAPSHOT_TYPE_ZSL == context->snapshot_type)
    && (SNAPSHOT_TYPE_CAPTURE_ONLY != context->zsl_info.snapshot_type))) {
    status = MM_CAMERA_ERR_INVALID_OPERATION;
    goto snapshot_start_encode_end;
  }
  if (SNAPSHOT_TYPE_CAPTURE_ONLY == context->snapshot_type) {
    add_frame(&context->encode_queue,
      context->p_capturedFrame[0]);
    context->capturedFrameCount = 0;
    context->p_capturedFrame[0] = NULL;
     /* create encode thread */
    if ((pthread_create(&context->encode_thread_id, NULL,
      encode_thread_func, (void *)context)) != 0) {
      CDBG("%s Encode thread creation failed", __func__);
      status = MM_CAMERA_ERR_NO_MEMORY;
      goto snapshot_start_encode_end;
    }
  } else if (SNAPSHOT_TYPE_ZSL == context->snapshot_type) {
    CDBG("%s captured frame count %d encode bitmask %u", __func__,
      context->capturedFrameCount, context->encode_params.encodeBitMask);
    index = 0;
    context->num_captures = 0;
    do {
      if (context->encode_params.encodeBitMask & (0x1<<index)) {
        context->num_captures++;
        add_frame(&context->encode_queue, context->p_capturedFrame[index]);
      } else {
        add_frame(&context->capture_queue, context->p_capturedFrame[index]);
      }
      context->p_capturedFrame[index] = NULL;
      index++;
    } while (index < context->capturedFrameCount);
    context->capturedFrameCount = 0;
    signal_queue(&context->encode_queue);
  }
snapshot_start_encode_end:
  CDBG("%s: [E] %d", __func__, status);
  return status;
}
#endif //DISABLE_JPEG_ENCODING
/*===========================================================================
FUNCTION      snapshot_start_multishot

DESCRIPTION
===========================================================================*/
mm_camera_status_t snapshot_start_multishot(snapshot_context_t* context)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int i = 0;
  int retval = 0;
  struct msm_frame *postviewFrame = NULL;
  struct msm_frame *mainFrame = NULL;
  struct msm_frame *releaseFrame, *tempFrame;
  int index = -1;
  int8_t is_thumb = false;
  fd_set fds;
  int entry = 0;
  uint32_t numn, denm ;
  rat_t exp_val, exposureTime;
  snapshotData_info_t snapshot_buf;
  struct timeval timeout;
  context->picfd = -1;

  int num_captures = context->capture_params.num_captures * 2;
  CDBG("%s enter num_captures %d state %d\n", __func__, num_captures,
    context->state);
  pthread_mutex_lock(&snapshot_state_mutex);
  if (SNAPSHOT_STATE_DEINIT == context->state) {
    CDBG_HIGH("%s snapshot_start_snapshot failed", __func__);
    pthread_mutex_unlock(&snapshot_state_mutex);
    goto snapshot_start_multishot_end;
  }
  context->capture_state = SNAPSHOT_STATE_CAPTURE_WAIT;
  context->capturedFrameCount = 0;
  pthread_mutex_unlock(&snapshot_state_mutex);

  /* update params for multishot*/

  mmcamera_util_profile("send snap cmd ");
  status = snapshot_issue_cmd(CAMERA_START_SNAPSHOT, 0, NULL, 5000,
    context->camfd);

  mmcamera_util_profile("post send snap cmd ");
  if (MM_CAMERA_SUCCESS != status) {
    CDBG_HIGH("%s snapshot_start_snapshot failed", __func__);
    goto snapshot_start_multishot_end;
  }

  char device[MAX_DEV_NAME_LEN];
  snprintf(device, MAX_DEV_NAME_LEN, MSM_CAMERA_PIC, get_device_id());
  context->picfd = open(device, O_RDWR);
  if (context->picfd < 0) {
    CDBG("%s picfd open failed", __func__);
    status = MM_CAMERA_ERR_CAPTURE_FAILED;
    goto snapshot_start_multishot_end;
  }

  /* Get the frame */
  do {
    timeout.tv_usec = 0;
    timeout.tv_sec = 6;
    FD_ZERO(&fds);
    FD_SET(context->picfd, &fds);
    FD_SET(context->terminatefd[0], &fds);
    int nfds = MAX(context->picfd, context->terminatefd[0]);
    CDBG("%s get frame", __func__);

    retval = select(nfds + 1, &fds, NULL, NULL, &timeout);
    if (FD_ISSET(context->terminatefd[0], &fds)) {
      CDBG("%s terminate the snapshot thread", __func__);
      status = MM_CAMERA_ERR_CAPTURE_FAILED;
      close(context->picfd);
      CDBG("%s Exiting %d",__func__, status);
      return status;
    }
    if (retval == 0) {
      status = MM_CAMERA_ERR_CAPTURE_TIMEOUT;
      CDBG("%s timeout while polling", __func__);
      goto snapshot_start_multishot_end;
    } else if (retval < 0) {
      CDBG("%s SELECT ERROR %s \n", strerror(errno), __func__);
      status = MM_CAMERA_ERR_CAPTURE_FAILED;
      goto snapshot_start_multishot_end;
    }

    tempFrame = (struct msm_frame *)malloc(sizeof(struct msm_frame)
      + sizeof(common_crop_t));
    if (!tempFrame) {
      CDBG("%s alloc failed \n", __func__);
      status = MM_CAMERA_ERR_NO_MEMORY;
      goto snapshot_start_multishot_end;
    }
    memset(tempFrame, 0x0, sizeof(struct msm_frame)
      + sizeof(common_crop_t));
    tempFrame->cropinfo = (uint8_t *)tempFrame + sizeof(struct msm_frame);
    tempFrame->croplen = sizeof(common_crop_t);

    status = snapshot_get_picture(context, tempFrame);
    if (MM_CAMERA_SUCCESS != status) {
      CDBG("%s snapshot_get_picture failed", __func__);
      free(tempFrame);
      goto snapshot_start_multishot_end;
    }

    CDBG("%s snapshot_get_picture %d", __func__, tempFrame->path);

    is_thumb = (OUTPUT_TYPE_T == tempFrame->path);
    if (is_thumb) {
      postviewFrame = tempFrame;
    } else if (tempFrame->path == OUTPUT_TYPE_S) {
      mainFrame = tempFrame;
    } else {
      status = MM_CAMERA_ERR_CAPTURE_FAILED;
      goto snapshot_start_multishot_end;
    }

    context->capture_complete_cnt++;

    if (!(context->capture_complete_cnt & 0x1)) { /*capture completed*/
      pthread_mutex_lock(&snapshot_state_mutex);
      context->capture_state = SNAPSHOT_STATE_CAPTURE_DONE;
      pthread_mutex_unlock(&snapshot_state_mutex);

      if (context->notifyIntf->on_event) {
        mm_camera_event event;
        event.event_type = SNAPSHOT_DONE;
        event.event_data.yuv_frames[0] = postviewFrame;
        event.event_data.yuv_frames[1] = mainFrame;
        context->notifyIntf->on_event(&event);
      }

      /* Adding ExifTag for Exposure time -
       * We are not getting correct exposure time by the time we are sending
       * ImageEncodeParams for encode in HAL.We are getting correct exposuretime
       * after SNAPSHOT_DONE,so adding exif tag for exposure time here after
       * SNAPSHOT_DONE and before sending the encode params for jpeg encoding
       */

      status = snapshot_issue_cmd(CAMERA_GET_PARM_SNAPSHOTDATA,
          sizeof(snapshotData_info_t),(void *)&snapshot_buf, 5000,context->camfd);
      if (MM_CAMERA_SUCCESS == status) {
        denm = 100000;
        numn = (snapshot_buf.exposure_time) * (denm);
        rat_t exp_val = {numn,denm};
        memcpy(&exposureTime, &exp_val, sizeof(exp_val));
#ifndef DISABLE_JPEG_ENCODING
        entry = context->encode_params.exif_numEntries;
        if(entry == MAX_EXIF_TABLE_ENTRIES) {
          CDBG_HIGH("%s snapshot_exif_tag_entries_exceeded_limit", __func__);
          goto snapshot_start_multishot_end;
        }
        context->encode_params.exif_data[entry].tag_id =  EXIFTAGID_EXPOSURE_TIME;
        context->encode_params.exif_data[entry].tag_entry.type = EXIF_RATIONAL;
        context->encode_params.exif_data[entry].tag_entry.count = 1;
        context->encode_params.exif_data[entry].tag_entry.copy = 1;
        context->encode_params.exif_data[entry].tag_entry.data._rat = exposureTime;
        context->encode_params.exif_numEntries += 1;

        /* Only in case of "ISO mode=Auto" iso_speed needs to be queried from
         * aec or else HAL would have already filled in those Default settings
         * values from App
         */

        for(i=0; i < context->encode_params.exif_numEntries; i++){
           if(context->encode_params.exif_data[i].tag_id == EXIFTAGID_ISO_SPEED_RATING){
             if(context->encode_params.exif_data[i].tag_entry.data._short == 0){
                context->encode_params.exif_data[i].tag_entry.data._short =
                     snapshot_buf.iso_speed;
             }
             break;
           }
        }
#endif //DISABLE_JPEG_ENCODING
        CDBG("%s: %d: snapshot_buf.iso_speed: %d",__func__,__LINE__,snapshot_buf.iso_speed);
      } else if (MM_CAMERA_ERR_NOT_SUPPORTED == status) {
        CDBG("%s snapshot_get_parm_snapshotdata not supported", __func__);
        status = MM_CAMERA_SUCCESS;
      } else {
        CDBG_HIGH("%s snapshot_get_parm_snapshotdata failed", __func__);
        goto snapshot_start_multishot_end;
      }

      if (SNAPSHOT_TYPE_CAPTURE_AND_ENCODE == context->snapshot_type) {
        mmcamera_util_profile("Snapshot enQ for encoding");
#ifndef DISABLE_JPEG_ENCODING
        add_frame(&context->encode_queue, mainFrame);
#ifdef _TARGET_7x2x_
        add_frame(&context->encode_thumb_queue, postviewFrame);
#endif
        signal_queue(&context->encode_queue);
#endif
      } else { /* capture only */
        /* store the captured frame */
        context->p_capturedFrame[context->capturedFrameCount] = mainFrame;
        context->capturedFrameCount++;
      }
#ifndef _TARGET_7x2x_
      if(postviewFrame) {
        free(postviewFrame);
        postviewFrame = NULL;
      }
#endif

      /* release the next buffers to the kernel*/
      releaseFrame = get_frame(&context->capture_queue);
      if (releaseFrame) {
        status = snapshot_release_picture_frame(context, releaseFrame);
        if (MM_CAMERA_SUCCESS != status) {
          CDBG("%s snapshot_release_picture_frame failed", __func__);
          goto snapshot_start_multishot_end;
        }
        free(releaseFrame);
      }

      /* release the next buffers to the kernel*/
      releaseFrame = get_frame(&context->capture_thumb_queue);
      if (releaseFrame) {
        status = snapshot_release_picture_frame(context, releaseFrame);
        if (MM_CAMERA_SUCCESS != status) {
          CDBG("%s snapshot_release_picture_frame failed", __func__);
          context->capture_state = SNAPSHOT_STATE_CAPTURE_ERROR;
          goto snapshot_start_multishot_end;
        }
        free(releaseFrame);
      }
      pthread_mutex_lock(&snapshot_state_mutex);
      context->capture_state = SNAPSHOT_STATE_CAPTURE_DONE;
      pthread_mutex_unlock(&snapshot_state_mutex);
    }
    CDBG("%s capture_complete_cnt %d num_capt %d state %d", __func__,
      context->capture_complete_cnt, num_captures, context->capture_state);

    pthread_mutex_lock(&snapshot_state_mutex);
    if (SNAPSHOT_STATE_CAPTURE_CANCEL == context->capture_state) {
      CDBG("%s: cancel issued", __func__);
      pthread_mutex_unlock(&snapshot_state_mutex);
      goto snapshot_start_multishot_end;
    }
    pthread_mutex_unlock(&snapshot_state_mutex);
  } while (context->capture_complete_cnt < num_captures);

snapshot_start_multishot_end:

  pthread_mutex_lock(&snapshot_state_mutex);
  if ((MM_CAMERA_SUCCESS != status) &&
      (SNAPSHOT_STATE_CAPTURE_CANCEL != context->capture_state)) {
    context->capture_state = SNAPSHOT_STATE_CAPTURE_ERROR;
  } else if (SNAPSHOT_STATE_CAPTURE_CANCEL != context->capture_state)
    context->capture_state = SNAPSHOT_STATE_CAPTURE_DONE_ALL;
  pthread_mutex_unlock(&snapshot_state_mutex);

  pthread_mutex_lock(&snapshot_state_mutex);
  if (SNAPSHOT_STATE_CAPTURE_ERROR == context->capture_state) {
    pthread_mutex_unlock(&snapshot_state_mutex);
    /* send callback to user*/
    if (context->notifyIntf->on_event) {
      mm_camera_event event;
      event.event_type = SNAPSHOT_FAILED;
      event.event_data.yuv_frames[0] = NULL;
      event.event_data.yuv_frames[1] = NULL;
      context->notifyIntf->on_event(&event);
    }
  } else
    pthread_mutex_unlock(&snapshot_state_mutex);

  if (context->picfd > 0) {
    close(context->picfd);
    context->picfd = -1;
  }
  CDBG("%s Exit %d",__func__, status);
  return status;
}

/*===========================================================================
FUNCTION      snapshot_start_raw_snapshot

DESCRIPTION
===========================================================================*/
mm_camera_status_t snapshot_start_raw_snapshot(snapshot_context_t* context)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int retval = 0;
  struct msm_frame* tempFrame;
  struct msm_frame* releaseFrame;
  fd_set fds;
  struct timeval timeout;
  context->picfd = -1;

  int num_captures = 1; /*only one capture*/
  CDBG("%s enter num_captures %d\n", __func__, num_captures);
  pthread_mutex_lock(&snapshot_state_mutex);
  context->capture_state = SNAPSHOT_STATE_CAPTURE_WAIT;
  pthread_mutex_unlock(&snapshot_state_mutex);

  /* update params for multishot*/
  status = snapshot_issue_cmd(CAMERA_START_RAW_SNAPSHOT, 0, NULL, 1000,
    context->camfd);
  if (MM_CAMERA_SUCCESS != status) {
    CDBG("%s snapshot_start_snapshot failed", __func__);
    goto snapshot_start_raw_snapshot_end;
  }

  char device[MAX_DEV_NAME_LEN];
  snprintf(device, MAX_DEV_NAME_LEN, MSM_CAMERA_PIC, get_device_id());
  context->picfd = open(device, O_RDWR);
  if (context->picfd < 0) {
    CDBG("%s picfd open failed", __func__);
    status = MM_CAMERA_ERR_CAPTURE_FAILED;
    goto snapshot_start_raw_snapshot_end;
  }

  /* Get the frame */
  do {
    timeout.tv_usec = 0;
    timeout.tv_sec = 6;
    FD_ZERO(&fds);
    FD_SET(context->picfd, &fds);
    CDBG("%s get frame", __func__);

    pthread_mutex_lock(&snapshot_state_mutex);
    context->capture_state = SNAPSHOT_STATE_CAPTURE_WAIT;
    pthread_mutex_unlock(&snapshot_state_mutex);

    retval = select(context->picfd + 1, &fds, NULL, NULL, &timeout);
    if (retval == 0) {
      status = MM_CAMERA_ERR_CAPTURE_TIMEOUT;
      CDBG("%s timeout while polling", __func__);
      goto snapshot_start_raw_snapshot_end;
    } else if (retval < 0) {
      CDBG("%s SELECT ERROR %s \n", strerror(errno), __func__);
      status = MM_CAMERA_ERR_CAPTURE_FAILED;
      goto snapshot_start_raw_snapshot_end;
    }

    tempFrame = (struct msm_frame *)malloc(sizeof(struct msm_frame)
      + sizeof(common_crop_t));
    if (!tempFrame) {
      CDBG("%s alloc failed \n", __func__);
      status = MM_CAMERA_ERR_NO_MEMORY;
      goto snapshot_start_raw_snapshot_end;
    }
    memset(tempFrame, 0x0, sizeof(struct msm_frame)
      + sizeof(common_crop_t));
    tempFrame->cropinfo = (uint8_t *)tempFrame + sizeof(struct msm_frame);
    tempFrame->croplen = sizeof(common_crop_t);

    context->capture_state = SNAPSHOT_STATE_CAPTURE_DONE;
    status = snapshot_get_picture(context, tempFrame);
    if (MM_CAMERA_SUCCESS != status) {
      CDBG("%s snapshot_get_picture failed", __func__);
      goto snapshot_start_raw_snapshot_end;
    }

    context->capture_complete_cnt++;

    if (context->notifyIntf->on_event) {
      mm_camera_event event;
      event.event_type = SNAPSHOT_DONE;
      event.event_data.raw_frame = tempFrame;
      context->notifyIntf->on_event(&event);
    }

    /* release the frame */
    free(tempFrame);
    CDBG("%s capture_complete_cnt %d num_capt %d state %d", __func__,
      context->capture_complete_cnt, num_captures, context->capture_state);

    pthread_mutex_lock(&snapshot_state_mutex);
    if (SNAPSHOT_STATE_CAPTURE_CANCEL == context->capture_state) {
      CDBG("%s: cancel issued", __func__);
      pthread_mutex_unlock(&snapshot_state_mutex);
      goto snapshot_start_raw_snapshot_end;
    }
    context->capture_state = SNAPSHOT_STATE_CAPTURE_DONE;
    pthread_mutex_unlock(&snapshot_state_mutex);
  } while (context->capture_complete_cnt < num_captures);

snapshot_start_raw_snapshot_end:

  pthread_mutex_lock(&snapshot_state_mutex);
  if ((MM_CAMERA_SUCCESS != status) &&
    (SNAPSHOT_STATE_CAPTURE_CANCEL != context->capture_state)) {
    context->capture_state = SNAPSHOT_STATE_CAPTURE_ERROR;
  } else if (SNAPSHOT_STATE_CAPTURE_CANCEL != context->capture_state)
    context->capture_state = SNAPSHOT_STATE_CAPTURE_DONE_ALL;
  pthread_mutex_unlock(&snapshot_state_mutex);

  pthread_mutex_lock(&snapshot_state_mutex);
  if (SNAPSHOT_STATE_CAPTURE_ERROR == context->capture_state) {
    pthread_mutex_unlock(&snapshot_state_mutex);
    /* send callback to user*/
    if (context->notifyIntf->on_event) {
      mm_camera_event event;
      event.event_type = SNAPSHOT_FAILED;
      event.event_data.raw_frame = NULL;
      context->notifyIntf->on_event(&event);
    }
  } else
    pthread_mutex_unlock(&snapshot_state_mutex);

  if (context->picfd > 0) {
    close(context->picfd);
    context->picfd = -1;
  }
  CDBG("%s Exit %d",__func__, status);
  return status;
}

/*===========================================================================
FUNCTION      snapshot_cancel

DESCRIPTION
===========================================================================*/
mm_camera_status_t snapshot_cancel(void* handle)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  char exit = 'y';
  CDBG("%s: [S] handle %p", __func__, handle);
  snapshot_context_t* context = (snapshot_context_t*)handle;
  if (!handle) {
    /* snapshot is already cancelled */
    goto snapshot_cancel_end;
  }
  int retval;

  status = snapshot_issue_cmd(CAMERA_STOP_SNAPSHOT, 0, NULL, 1000,
    context->camfd);
  if (MM_CAMERA_SUCCESS != status) {
    goto snapshot_cancel_end;
  }

  if (context->picfd > 0) {
    retval = ioctl(context->picfd, MSM_CAM_IOCTL_UNBLOCK_POLL_PIC_FRAME);
    if (retval < 0) {
      CDBG("%s: MSM_CAM_IOCTL_UNBLOCK_POLL_FRAME failed (%s).",
        __func__, strerror(errno));
    }
  }
  pthread_mutex_lock(&snapshot_state_mutex);
  context->capture_state = SNAPSHOT_STATE_CAPTURE_CANCEL;
  pthread_mutex_unlock(&snapshot_state_mutex);

  retval = write(context->terminatefd[1], &exit, sizeof(exit));
  if (retval < 0)
    CDBG("%s: Snapshot thread terminate failed\n", __func__);

snapshot_cancel_end:
  if (context)
    status = snapshot_issue_cmd(CAMERA_POSTPROC_ABORT, 0, NULL, 1000,
      context->camfd);
  CDBG("%s Exit %d",__func__, status);
  return status;
}

/*===========================================================================
 * FUNCTION    - snapshot_add_buffers -
 *
 * DESCRIPTION:
 *==========================================================================*/
mm_camera_status_t snapshot_add_buffers(void* handle ,
  struct msm_pmem_info* pmemBuf)
{
  snapshot_context_t* context = (snapshot_context_t*)handle;
  int ret;
  int i = 0;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  struct fifo_queue* queue = NULL;
  snapshot_frame_info_t* frame = NULL;

  switch(pmemBuf->type) {
    case MSM_PMEM_RAW_MAINIMG:
    case MSM_PMEM_MAINIMG_VPE:
    case MSM_PMEM_MAINIMG: {
      i = context->current_buffer_count;
      CDBG("%s: add snapshot buffer %d", __func__, i);
      context->snapshot_frame[i].img.path = OUTPUT_TYPE_S;
      queue = &context->capture_queue;
      frame = &context->snapshot_frame[i];
      context->current_buffer_count++;
      break;
    }
    case MSM_PMEM_THUMBNAIL_VPE:
    case MSM_PMEM_THUMBNAIL: {
      i = context->current_thumb_buffer_count;
      CDBG("%s: add thumb buffer %d", __func__, i);
      context->thumb_frame[i].img.path = OUTPUT_TYPE_T;
      queue = &context->capture_thumb_queue;
      frame = &context->thumb_frame[i];
      context->current_thumb_buffer_count++;
      break;
    }
    default: {
      status = MM_CAMERA_ERR_INVALID_OPERATION;
      goto snapshot_add_buffers_end;
    }
  }

  frame->img.fd = pmemBuf->fd;
  frame->img.buffer = (unsigned long)pmemBuf->vaddr;
  frame->img.planar0_off = pmemBuf->planar0_off;
  frame->offset = pmemBuf->offset;
  frame->img.planar1_off = pmemBuf->planar1_off;

  if (i >= ACTIVE_SNAPSHOT_BUFFERS) { /* push to  Q*/
    CDBG ("%s: add_frame[%d] to %s", __func__, i, queue->name);
    struct msm_frame* tempFrame = (struct msm_frame *)malloc(
      sizeof(struct msm_frame)
      + sizeof(common_crop_t));
    if (!tempFrame) {
      CDBG("%s alloc failed \n", __func__);
      status = MM_CAMERA_ERR_NO_MEMORY;
      goto snapshot_add_buffers_end;
    }
    memset(tempFrame, 0x0, sizeof(struct msm_frame)
      + sizeof(common_crop_t));
    *tempFrame = frame->img;
    tempFrame->cropinfo = (uint8_t *)tempFrame + sizeof(struct msm_frame);
    tempFrame->croplen = sizeof(common_crop_t);
    add_frame(queue, tempFrame);
  }

snapshot_add_buffers_end:
  CDBG("%s: Exit %d", __func__, status);
  return status;
}

/*===========================================================================
FUNCTION      snapshot_issue_cmd

DESCRIPTION
===========================================================================*/
mm_camera_status_t snapshot_issue_cmd(cam_ctrl_type type,
  uint16_t length, void *value, uint32_t timeout_ms, int camfd)
{
  struct msm_ctrl_cmd ctrlCmd;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int32_t ioctl_cmd = MSM_CAM_IOCTL_CTRL_COMMAND;
  int resp_fd = camfd;
  int8_t is_blocking_cmd = TRUE;

  switch(type) {
    case CAMERA_STOP_SNAPSHOT:
      ioctl_cmd = MSM_CAM_IOCTL_CTRL_COMMAND_2;
      resp_fd = -1;
      is_blocking_cmd = FALSE;
      break;
    default:
      /* take default value */
      break;
  }

  ctrlCmd.timeout_ms = timeout_ms;
  ctrlCmd.type       = (uint16_t)type;
  ctrlCmd.length     = length;
  ctrlCmd.resp_fd    = resp_fd;
  ctrlCmd.value = value;

  if (ioctl(camfd, ioctl_cmd, &ctrlCmd) < 0) {
    CDBG_ERROR("%s: error (%s): type %d, length %d, status %d",
      __FUNCTION__, strerror(errno), type, length, ctrlCmd.status);
    return MM_CAMERA_ERR_GENERAL;
  }
  if (!is_blocking_cmd)
    ctrlCmd.status = CAM_CTRL_SUCCESS;

  CDBG("%s: succeeded type %d ctrl_status %d,", __func__, type,
    ctrlCmd.status);
  if (CAM_CTRL_SUCCESS == ctrlCmd.status)
    status = MM_CAMERA_SUCCESS;
  else if (CAM_CTRL_INVALID_PARM == ctrlCmd.status)
    status = MM_CAMERA_ERR_INVALID_OPERATION;
  else if (CAM_CTRL_NOT_SUPPORTED == ctrlCmd.status)
    status = MM_CAMERA_ERR_NOT_SUPPORTED;
  else
    status = MM_CAMERA_ERR_GENERAL;
  return status;
}

/*===========================================================================
 * FUNCTION    - snapshot_release_picture_frame -
 *
 * DESCRIPTION:
 *==========================================================================*/
static mm_camera_status_t snapshot_release_picture_frame(snapshot_context_t*
  context, struct msm_frame* captFrame)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  CDBG("%s enter 0x%x\n", __func__, (uint32_t)captFrame->buffer);

  if ((ret = ioctl(context->picfd,
    MSM_CAM_IOCTL_RELEASE_PIC_BUFFER, captFrame)) < 0) {
    CDBG("%s: MSM_CAM_IOCTL_RELEASE_PIC_BUFFER failed.%d \n", __func__,
      ret);
  }

  CDBG("%s Exit", __func__);
  return status;
}

/*===========================================================================
 * FUNCTION    - snapshot_get_picture -
 *
 * DESCRIPTION:
 *==========================================================================*/
mm_camera_status_t snapshot_get_picture(snapshot_context_t* context,
  struct msm_frame* captFrame)
{
  int ret = TRUE;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  CDBG("%s: enter\n", __func__);

  if ((ret = ioctl(context->picfd, MSM_CAM_IOCTL_GET_PICTURE,
    captFrame)) < 0) {
    CDBG("%s: MSM_CAM_IOCTL_GET_PICTURE failed. %d \n", __func__, ret);
      status = MM_CAMERA_ERR_CAPTURE_FAILED;
  }
  CDBG("%s: buffer %d vaddr %p \n", __func__, captFrame->path,
    (void *)captFrame->buffer);
  CDBG("%s: crop info %d %d %d %d %d %d %d %d\n", __func__,
    context->crop_info.in1_h,
    context->crop_info.in1_w,
    context->crop_info.out1_w,
    context->crop_info.out1_h,
    context->crop_info.in2_h,
    context->crop_info.in2_w,
    context->crop_info.out2_w,
    context->crop_info.out2_h);

  CDBG("%s: Exit %d", __func__, status);
  return status;
}

/*===========================================================================
 * FUNCTION    - snapshot_get_offset -
 *
 * DESCRIPTION:
 *==========================================================================*/
uint32_t snapshot_get_offset(snapshot_context_t* context, unsigned long vaddr)
{
  int i=0;

  for (i=0; i< MAX_SNAPSHOT_BUFFERS; i++) {
    if (context->snapshot_frame[i].img.buffer == vaddr) {
      CDBG("%s: buffer offset %d", __func__,
        context->snapshot_frame[i].offset);
      return context->snapshot_frame[i].offset;
    }
  }
  CDBG_ERROR("%s: No matching buffer found", __func__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - snapshot_jpeg_encode -
 *
 * DESCRIPTION:
 *==========================================================================*/
#ifndef DISABLE_JPEG_ENCODING
#ifdef _TARGET_7x2x_
mm_camera_status_t snapshot_jpeg_encode(snapshot_context_t* context,
  struct msm_frame* main_frame, struct msm_frame* thumb_frame)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  cam_ctrl_dimension_t dimension;
  uint8_t *snapshot_buf;
  uint32_t snapshot_fd;
  uint8_t *thumbnail_buf;
  uint32_t thumbnail_fd;
  common_crop_t *main_crop;
  common_crop_t *thumb_crop;
  common_crop_t crop_info;
  cam_point_t main_crop_offset;
  cam_point_t *p_main_crop_offset = NULL;
  cam_point_t thumb_crop_offset;
  cam_point_t *p_thumb_crop_offset = NULL;
  uint32_t picture_width, picture_height, thumb_width, thumb_height;

  CDBG("%s: main %p\n",__func__, main_frame);

  mmcamera_util_profile("snap config encoding");
  context->out_buffer_offset = 0;
  CDBG("3d formatted %d", g_snapshot_context->encode_params.format3d);
  int rc = 1;

  if(g_snapshot_context->encode_params.format3d){
#ifdef _TARGET_8660_
    encode_init = mpo_encoder_init;
    encode_join = mpo_encoder_join;
    encoder_encode = mpo_encoder_encode;
#else
    encode_init = NULL;
    encode_join = NULL;
#endif
  } else {
    encode_init = jpeg_encoder_init;
    encode_join = jpeg_encoder_join;
    encoder_encode = jpeg_encoder_encode;
  }

  encode_init();

  if (!rc) {
    CDBG("%s: jpeg_encoding_init failed.\n",__func__);
    status = MM_CAMERA_ERR_ENCODE;

    encode_join();

    goto snapshot_jpeg_encode_end;
  }

  memset(&dimension, 0, sizeof(cam_ctrl_dimension_t));
  picture_width = (context->zsl_enable) ?
    context->zsl_info.zsl_str_parms.picture_width :
    context->capture_params.picture_width;
  picture_height = (context->zsl_enable) ?
    context->zsl_info.zsl_str_parms.picture_height :
    context->capture_params.picture_height;
  thumb_width = (context->zsl_enable) ?
    context->zsl_info.zsl_capture_params.thumbnail_width :
    context->capture_params.thumbnail_width;
  thumb_height = (context->zsl_enable) ?
    context->zsl_info.zsl_capture_params.thumbnail_height :
    context->capture_params.thumbnail_height;

  if (g_snapshot_context->encode_params.format3d)
    dimension.orig_picture_dx = picture_width*2;
  else
    dimension.orig_picture_dx = picture_width;

  dimension.orig_picture_dy = picture_height;
  thumb_crop = (common_crop_t *)thumb_frame->cropinfo;
  dimension.thumbnail_width = (thumb_crop->in1_w) ? thumb_crop->in1_w :
    thumb_width;
  dimension.thumbnail_height = (thumb_crop->in1_h) ? thumb_crop->in1_h :
    thumb_height;
  snapshot_buf = (uint8_t *)main_frame->buffer;
  snapshot_fd = main_frame->fd;
  thumbnail_buf = (uint8_t *)thumb_frame->buffer;
  thumbnail_fd = thumb_frame->fd;

  /* update crop info */
  main_crop = (common_crop_t *)main_frame->cropinfo;
  crop_info.in2_w = main_crop->in2_w;
  crop_info.in2_h = main_crop->in2_h;
  crop_info.out2_w = main_crop->out2_w;
  crop_info.out2_h = main_crop->out2_h;
  if (context->encode_params.output_picture_width &&
    context->encode_params.output_picture_height) {
    CDBG("%s: downscale main image %dx%d", __func__,
      context->encode_params.output_picture_width,
      context->encode_params.output_picture_height);
    crop_info.out2_w = context->encode_params.output_picture_width;
    crop_info.out2_h = context->encode_params.output_picture_height;
    if (!crop_info.in2_w)
      crop_info.in2_w = dimension.orig_picture_dx;
    if (!crop_info.in2_h)
      crop_info.in2_h = dimension.orig_picture_dy;

    CDBG("%s: thumb dimension %dx%d", __func__,
      thumb_width,
      thumb_height);
    if ((thumb_width > context->encode_params.output_picture_width)
      || (thumb_height > context->encode_params.output_picture_height)) {
      thumb_width = context->encode_params.output_picture_width;
      thumb_height = context->encode_params.output_picture_height;
    }
  }
  crop_info.in1_w = (thumb_crop->in1_w) ? thumb_crop->in1_w :
    thumb_width;
  crop_info.in1_h = (thumb_crop->in1_h) ? thumb_crop->in1_h :
    thumb_height;
  crop_info.out1_w = thumb_crop->out1_w;
  crop_info.out1_h = thumb_crop->out1_h;
  if (main_crop->in2_w || main_crop->in2_h) {
    p_main_crop_offset = &main_crop_offset;
    p_main_crop_offset->x = (dimension.orig_picture_dx - crop_info.in2_w)/2;
    p_main_crop_offset->y = (dimension.orig_picture_dy - crop_info.in2_h)/2;
  }
  if (thumb_crop->in1_w || thumb_crop->in1_h) {
    p_thumb_crop_offset = &thumb_crop_offset;
    p_thumb_crop_offset->x = 0;
    p_thumb_crop_offset->y = 0;
  }

#ifdef DUMP_JPEG_INPUT
  static int count;
  char fn[BUFF_SIZE_50];
  int size = dimension.orig_picture_dx * dimension.orig_picture_dy * 1.5;
  snprintf(fn, BUFF_SIZE_50, "/data/multishot%d.yuv", count);
  FILE* fp = fopen(fn, "w+");
  CDBG("%s: fp %p size %d", __func__, fp, size);
  if (fp) {
    fwrite(snapshot_buf, 1, size, fp);
    fclose(fp);
    count++;
  }
#endif

  mmcamera_util_profile("snap starting encoding");
  if((thumb_width ==0) || (thumb_height == 0)) {
      thumbnail_buf = NULL;
      thumbnail_fd = 0;
  }

  if(!g_snapshot_context->encode_params.format3d) {
    jpege_set_phy_offset(snapshot_get_offset(context, main_frame->buffer));
  }

  encoder_encode(&dimension,
    thumbnail_buf,
    thumbnail_fd,
    snapshot_buf,
    snapshot_fd,
    &crop_info,
    context->encode_params.exif_data,
    context->encode_params.exif_numEntries,
    -1,
    p_main_crop_offset,
    p_thumb_crop_offset);

  if (!rc) {
    status = MM_CAMERA_ERR_ENCODE;
  }

snapshot_jpeg_encode_end:
  CDBG("%s: Exit %d", __func__, status);
  return status;
}
#else
mm_camera_status_t snapshot_jpeg_encode(snapshot_context_t* context,
  struct msm_frame* main_frame)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  cam_ctrl_dimension_t dimension;
  uint8_t *snapshot_buf;
  uint32_t snapshot_fd;
  uint8_t *thumbnail_buf;
  uint32_t thumbnail_fd;
  common_crop_t *main_crop;
  common_crop_t crop_info;
  cam_point_t main_crop_offset;
  cam_point_t *p_main_crop_offset = NULL;
  uint32_t picture_width, picture_height, thumb_width, thumb_height;

  CDBG("%s: main %p\n",__func__, main_frame);

  mmcamera_util_profile("snap config encoding");
  context->out_buffer_offset = 0;
  CDBG("3d formatted %d", g_snapshot_context->encode_params.format3d);
  int rc = 1;

  encode_init = jpeg_encoder_init;
  encode_join = jpeg_encoder_join;
  encoder_encode = jpeg_encoder_encode;
  if(g_snapshot_context->encode_params.format3d){
#ifdef _TARGET_8660_
    encode_init = mpo_encoder_init;
    encode_join = mpo_encoder_join;
    encoder_encode = mpo_encoder_encode;
#endif
  }

  encode_init();

  if (!rc) {
    CDBG("%s: jpeg_encoding_init failed.\n",__func__);
    status = MM_CAMERA_ERR_ENCODE;

    encode_join();

    goto snapshot_jpeg_encode_end;
  }

  memset(&dimension, 0, sizeof(cam_ctrl_dimension_t));
  picture_width = (context->zsl_enable) ?
    context->zsl_info.zsl_str_parms.picture_width :
    context->capture_params.picture_width;
  picture_height = (context->zsl_enable) ?
    context->zsl_info.zsl_str_parms.picture_height :
    context->capture_params.picture_height;
  thumb_width = (context->zsl_enable) ?
    context->zsl_info.zsl_capture_params.thumbnail_width :
    context->capture_params.thumbnail_width;
  thumb_height = (context->zsl_enable) ?
    context->zsl_info.zsl_capture_params.thumbnail_height :
    context->capture_params.thumbnail_height;

  if (g_snapshot_context->encode_params.format3d)
    dimension.orig_picture_dx = picture_width*2;
  else
    dimension.orig_picture_dx = picture_width;
  dimension.orig_picture_dy = picture_height;
  dimension.thumbnail_width = picture_width;
  dimension.thumbnail_height = picture_height;
  snapshot_buf = (uint8_t *)main_frame->buffer;
  snapshot_fd = main_frame->fd;
  thumbnail_buf = (uint8_t *)main_frame->buffer;
  thumbnail_fd = main_frame->fd;

  /* update crop info */
  main_crop = (common_crop_t *)main_frame->cropinfo;
  crop_info.in2_w = main_crop->in2_w;
  crop_info.in2_h = main_crop->in2_h;
  crop_info.out2_w = main_crop->out2_w;
  crop_info.out2_h = main_crop->out2_h;
  if (context->encode_params.output_picture_width &&
    context->encode_params.output_picture_height) {
    CDBG("%s: downscale main image %dx%d", __func__,
      context->encode_params.output_picture_width,
      context->encode_params.output_picture_height);
    crop_info.out2_w = context->encode_params.output_picture_width;
    crop_info.out2_h = context->encode_params.output_picture_height;
    if (!crop_info.in2_w)
      crop_info.in2_w = dimension.orig_picture_dx;
    if (!crop_info.in2_h)
      crop_info.in2_h = dimension.orig_picture_dy;

    if (!(((crop_info.in2_w < crop_info.out2_w)&&(crop_info.in2_h < crop_info.out2_h)) || ((crop_info.in2_w > crop_info.out2_w)&&(crop_info.in2_h > crop_info.out2_h))))
    {
      crop_info.in2_h = crop_info.out2_h;
      CDBG("%s: input height correction\n",  __func__);
    }

    CDBG("%s: thumb dimension %dx%d", __func__,
      thumb_width,
      thumb_height);
    if ((thumb_width > context->encode_params.output_picture_width)
      || (thumb_height > context->encode_params.output_picture_height)) {
      thumb_width = context->encode_params.output_picture_width;
      thumb_height = context->encode_params.output_picture_height;
    }
  }
  crop_info.in1_w = (main_crop->in2_w) ? main_crop->in2_w :
    picture_width;
  crop_info.in1_h = (main_crop->in2_h) ? main_crop->in2_h :
    picture_height;
  crop_info.out1_w = thumb_width;
  crop_info.out1_h = thumb_height;
  if (main_crop->in2_w || main_crop->in2_h) {
    p_main_crop_offset = &main_crop_offset;
    p_main_crop_offset->x = (dimension.orig_picture_dx - crop_info.in2_w)/2;
    p_main_crop_offset->y = (dimension.orig_picture_dy - crop_info.in2_h)/2;
  }

#ifdef DUMP_JPEG_INPUT
  static int count;
  char fn[BUFF_SIZE_50];
  int size = dimension.orig_picture_dx * dimension.orig_picture_dy;
  snprintf(fn, BUFF_SIZE_50, "/data/multishot%d.yuv", count);
  FILE* fp = fopen(fn, "w+");
  CDBG("%s: fp %p size %d", __func__, fp, size);
  if (fp) {
    fwrite(snapshot_buf, 1, size, fp);
    fwrite(snapshot_buf + main_frame->planar1_off, 1, size/2 , fp);
    fclose(fp);
    count++;
  }
#endif

  mmcamera_util_profile("snap starting encoding");
  if((thumb_width ==0) || (thumb_height == 0)) {
      thumbnail_buf = NULL;
      thumbnail_fd = 0;
  }

  if(!g_snapshot_context->encode_params.format3d) {
    jpege_set_phy_offset(snapshot_get_offset(context, main_frame->buffer));
  }

  encoder_encode(&dimension,
    thumbnail_buf,
    thumbnail_fd,
    snapshot_buf,
    snapshot_fd,
    &crop_info,
    context->encode_params.exif_data,
    context->encode_params.exif_numEntries,
    -1,
    p_main_crop_offset,
    p_main_crop_offset,
    context->zsl_enable);

  if (!rc) {
    status = MM_CAMERA_ERR_ENCODE;
  }

snapshot_jpeg_encode_end:
  CDBG("%s: Exit %d", __func__, status);
  return status;
}
#endif //7x27
#endif //DISABLE_JPEG_ENCODING
/*===========================================================================
 * FUNCTION    - snapshot_jpeg_cb -
 *
 * DESCRIPTION:
 *==========================================================================*/
#ifndef DISABLE_JPEG_ENCODING
void snapshot_jpeg_cb(jpeg_event_t event)
{
  CDBG("%s enter\n", __func__);
#if DBG_DUMP_JPEG_IMG
  static int count;
  char fn[BUFF_SIZE_50];
  snprintf(fn, BUFF_SIZE_50, "/data/multishot/multishot%d.jpg", count);
  FILE* fp = fopen(fn, "w+");
  CDBG("%s: fp %p size %d", __func__, fp,
    g_snapshot_context->out_buffer_offset);
  if (fp) {
    fwrite(g_snapshot_context->start_params.output_buffer.ptr,
      1, g_snapshot_context->out_buffer_offset, fp);
    fclose(fp);
    count ++;
  }
#endif


#ifdef _TARGET_8660_
  if(JPEG_EVENT_DONE == event && g_snapshot_context->encode_params.format3d){
      mpo_encoder_set_api_info(
          g_snapshot_context->encode_params.
          p_output_buffer[g_snapshot_context->out_buffer_index].ptr);
  }
#endif
  if(event == JPEG_EVENT_THUMBNAIL_DROPPED){
        CDBG("Thumbnail dropped: Size can be greater than 64k");
        return;
  }

   pthread_mutex_lock(&encode_thread_mutex);
   if((g_snapshot_context != NULL) &&
           g_snapshot_context->encode_state != SNAPSHOT_STATE_ENCODE_CANCEL){
       g_snapshot_context->encode_state = (JPEG_EVENT_DONE == event) ?
         SNAPSHOT_STATE_ENCODE_DONE : SNAPSHOT_STATE_ENCODE_ERROR;
   }
   pthread_cond_signal(&encode_thread_cond);
   pthread_mutex_unlock(&encode_thread_mutex);
}
#endif //DISABLE_JPEG_ENCODING

/*===========================================================================
 * FUNCTION    - snapshot_jpeg_fragment_cb -
 *
 * DESCRIPTION:
 *==========================================================================*/
#ifndef DISABLE_JPEG_ENCODING
void snapshot_jpeg_fragment_cb(uint8_t *ptr, uint32_t size)
{
   CDBG("%s: offset %d size %d buf_index %d", __func__,
     g_snapshot_context->out_buffer_offset, size,
     g_snapshot_context->out_buffer_index);
   CDBG("%s, size = %d\n", __func__,
     g_snapshot_context->encode_params.
     p_output_buffer[g_snapshot_context->out_buffer_index].size);

   if (g_snapshot_context->out_buffer_offset + size
     <= g_snapshot_context->encode_params.
     p_output_buffer[g_snapshot_context->out_buffer_index].size) {
     CDBG("%s, out_buffer_offset = %d\n", __func__,
       g_snapshot_context->out_buffer_offset);
     memcpy(g_snapshot_context->encode_params.
       p_output_buffer[g_snapshot_context->out_buffer_index].ptr
       + g_snapshot_context->out_buffer_offset, ptr, size);
       g_snapshot_context->out_buffer_offset += size;
   }
}
#endif //DISABLE_JPEG_ENCODING

/*===========================================================================
 * FUNCTION    - snapshot_zsl_capture_bestframe -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t snapshot_zsl_capture_bestframe(snapshot_context_t* context,
  struct msm_frame** pp_MainFrame, struct msm_frame** pp_ThumbFrame)
{
  int i=0;
  struct msm_frame *tempMFrame = NULL;
  struct msm_frame *tempTNFrame = NULL;
  struct msm_frame *temp;
  void* iterator = NULL;
  int index = 0;
  int32_t reltime = 0;
  uint32_t shortest_time = 0xffffffff, temptime;

  CDBG("%s: Enter", __func__);
  if ((context->capture_queue.num_of_frames !=
    context->capture_thumb_queue.num_of_frames) ||
    !context->capture_queue.num_of_frames) {
    CDBG_ERROR("%s: Error cannot find frame", __func__);
    return FALSE;
  }

  temp = begin(&iterator, &context->capture_queue);
  for(i=0; i<context->capture_queue.num_of_frames; i++ ) {
    if (temp == NULL) {
      CDBG_ERROR("%s: begin return NULL\n", __func__);
      return FALSE;
    }
    reltime = GET_REL_TIME_IN_MS(context->capture_start_time,
      temp->ts);
    CDBG("%s: reltime %d", __func__, reltime);
    temptime = ABS((reltime - ZSL_DELAY));
    CDBG("%s: temptime %d", __func__, temptime);
    if (temptime < shortest_time) {
      shortest_time = temptime;
      index = i;
    }
    temp = next(&iterator);
  }

  /* circulate all the buffer till the index */
  for (i=0; i<index; i++) {
    tempMFrame = get_frame(&context->capture_queue);
    tempTNFrame = get_frame(&context->capture_thumb_queue);
    if (!tempMFrame || !tempTNFrame) { /* Give the oldest frame for now*/
      CDBG("%s: Error", __func__);
    }
    /* Add back the frame to the capture queue */
    add_frame(&context->capture_queue, tempMFrame);
    add_frame(&context->capture_thumb_queue, tempTNFrame);
  }

  *pp_MainFrame = get_frame(&context->capture_queue);
  *pp_ThumbFrame = get_frame(&context->capture_thumb_queue);
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - snapshot_zsl_capture_oldest_frame -
 *
 * DESCRIPTION:
 *==========================================================================*/
void snapshot_zsl_capture_oldest_frame(snapshot_context_t* context,
  struct msm_frame** pp_MainFrame, struct msm_frame** pp_ThumbFrame)
{
  *pp_MainFrame = get_frame(&context->capture_queue);
  *pp_ThumbFrame = get_frame(&context->capture_thumb_queue);
}

/*===========================================================================
 * FUNCTION    - zsl_docapture -
 *
 * DESCRIPTION:
 *==========================================================================*/
void zsl_docapture(snapshot_context_t* context)
{
  int32_t reltime = 0;
  int i = 0;
  int num_frames = 0;
  int num_thumb_frames = 0;
  struct msm_frame *p_MainFrame = NULL;
  struct msm_frame *p_ThumbFrame = NULL;
  int num_captures =
    context->zsl_info.zsl_capture_params.num_captures;

  num_frames = context->capture_queue.num_of_frames;
  num_thumb_frames = context->capture_thumb_queue.num_of_frames;
  CDBG("%s captureQ frame count %d %d num_captures %d",
    __func__,num_frames,
    context->capture_thumb_queue.num_of_frames,
    num_captures);

  /* Search for the best frame to encode, based on the threshold */
  if (num_captures <= 1) {
    snapshot_zsl_capture_oldest_frame(context, &p_MainFrame, &p_ThumbFrame);
    context->capture_state = SNAPSHOT_STATE_CAPTURE_DONE;

    if (context->notifyIntf->on_event) {
      mm_camera_event event;
      event.event_type = SNAPSHOT_DONE;
      event.event_data.yuv_frames[0] = p_ThumbFrame;
      event.event_data.yuv_frames[1] = p_MainFrame;
      context->notifyIntf->on_event(&event);
    }

    if (SNAPSHOT_TYPE_CAPTURE_AND_ENCODE ==
      context->zsl_info.snapshot_type) {
      add_frame(&context->encode_queue, p_MainFrame);
      signal_queue(&context->encode_queue);
    } else {
      context->p_capturedFrame[context->capturedFrameCount] = p_MainFrame;
      CDBG("%s: capturedFrame[%d] %p buffer %p", __func__,
        context->capturedFrameCount, p_MainFrame, (void *)p_MainFrame->buffer);
      context->capturedFrameCount++;
    }

    add_frame(&context->capture_thumb_queue, p_ThumbFrame);
    context->capture_complete_cnt++;
  } else { /* burst mode */
    for (i=0; ((i<num_captures) &&
      (i < num_frames) &&
      (i< num_thumb_frames)); i++) {
       p_MainFrame = get_frame(&context->capture_queue);
       p_ThumbFrame = get_frame(&context->capture_thumb_queue);
       context->capture_state = SNAPSHOT_STATE_CAPTURE_DONE;

       if (context->notifyIntf->on_event) {
         mm_camera_event event;
         event.event_type = SNAPSHOT_DONE;
         event.event_data.yuv_frames[0] = p_ThumbFrame;
         event.event_data.yuv_frames[1] = p_MainFrame;
         context->notifyIntf->on_event(&event);
       }

       if (SNAPSHOT_TYPE_CAPTURE_AND_ENCODE ==
         context->zsl_info.snapshot_type) {
         add_frame(&context->encode_queue, p_MainFrame);
         signal_queue(&context->encode_queue);
       } else {
        context->p_capturedFrame[context->capturedFrameCount] = p_MainFrame;
        CDBG("%s: capturedFrame[%d] %p buffer %p", __func__,
          context->capturedFrameCount,
          p_MainFrame, (void *)p_MainFrame->buffer);
        context->capturedFrameCount++;
       }

       add_frame(&context->capture_thumb_queue, p_ThumbFrame);
       context->capture_complete_cnt++;
    }
  }
  /* stop capture*/
  CDBG("%s capture_complete_cnt %d", __func__, context->capture_complete_cnt);
  if (context->capture_complete_cnt >= context->num_captures) {

    pthread_mutex_lock(&snapshot_state_mutex);
    context->capture_state = SNAPSHOT_STATE_CAPTURE_DONE_ALL;
    context->capture_complete_cnt = 0;
    pthread_mutex_unlock(&snapshot_state_mutex);

    /* Stop streaming until, snapshot is done */
    CDBG("%s encode state before wait %d", __func__, context->encode_state);
    int ret = pthread_join(context->encode_thread_id, NULL);
    CDBG("%s after wait for encoding %d", __func__, ret);
    pthread_mutex_lock(&zsl_thread_mutex);
    context->zsl_info.start_capture = false; /* stop capture */
    context->encode_thread_id = -1;
    pthread_mutex_unlock(&zsl_thread_mutex);
  }
}


/*===========================================================================
FUNCTION      zsl_snapshot_thread_func

DESCRIPTION
===========================================================================*/
void *zsl_snapshot_thread_func(void *data)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  snapshot_context_t* context = (snapshot_context_t *)data;
  int retval = 0;
  struct msm_frame* releaseFrame;
  struct msm_frame* tempFrame;
  fd_set fds;
  int8_t is_thumb = false;
  uint8_t stream_count = 0;
  int i = 0;
  struct timeval timeout;

  CDBG("%s enter", __func__);
  pthread_mutex_lock(&zsl_thread_mutex);
  g_snapshot_context->zsl_info.zsl_thread_started = true;
  pthread_mutex_unlock(&zsl_thread_mutex);
  /* Signal main thread*/
  pthread_cond_signal(&zsl_thread_cond);

  context->picfd = -1;
  char device[MAX_DEV_NAME_LEN];
  snprintf(device, MAX_DEV_NAME_LEN, MSM_CAMERA_PIC, get_device_id());
  context->picfd = open(device, O_RDWR);

  if (context->picfd < 0) {
    CDBG("%s picfd open failed", __func__);
    goto zsl_snapshot_thread_func_end;
  }
  context->capture_state = SNAPSHOT_STATE_CAPTURE_WAIT;

  /* Get the frame */
  do {
    CDBG("%s get frame", __func__);

    do {
      timeout.tv_usec = 0;
      timeout.tv_sec = 6;
      FD_ZERO(&fds);
      FD_SET(context->picfd, &fds);
      FD_SET(context->terminatefd[0], &fds);
      int nfds = MAX(context->picfd, context->terminatefd[0]);

      retval = select(nfds + 1, &fds, NULL, NULL, &timeout);
      if (FD_ISSET(context->terminatefd[0], &fds)) {
        CDBG("%s terminate the ZSL thread", __func__);
        context->zsl_info.zsl_thread_exit = true;
        break;
      }
      if (retval == 0) {
        status = MM_CAMERA_ERR_CAPTURE_TIMEOUT;
        CDBG("%s timeout while polling", __func__);
        /* give timout callback ?*/
        continue;
      } else if (retval < 0) {
        CDBG("%s SELECT ERROR %s \n", strerror(errno), __func__);
        status = MM_CAMERA_ERR_CAPTURE_FAILED;
        goto zsl_snapshot_thread_func_end;
      }

      tempFrame = (struct msm_frame *)malloc(sizeof(struct msm_frame)
        + sizeof(common_crop_t));
      if (!tempFrame) {
        CDBG("%s alloc failed \n", __func__);
        status = MM_CAMERA_ERR_NO_MEMORY;
        goto zsl_snapshot_thread_func_end;
      }
      memset(tempFrame, 0x0, sizeof(struct msm_frame)
        + sizeof(common_crop_t));
      tempFrame->cropinfo = (uint8_t *)tempFrame + sizeof(struct msm_frame);
      tempFrame->croplen = sizeof(common_crop_t);

      status = snapshot_get_picture(context, tempFrame);
      if (MM_CAMERA_SUCCESS != status) {
        CDBG("%s snapshot_get_picture failed", __func__);
        status = MM_CAMERA_ERR_CAPTURE_FAILED;
        free(tempFrame);
        goto zsl_snapshot_thread_func_end;
      }

      CDBG("%s snapshot_get_picture %d", __func__, tempFrame->path);
#if DBG_DUMP_ZSL_YUV_FRAME
     static int frameCnt = 0;
     if (frameCnt >= 0 && frameCnt <= 10) {
       char buf[BUFF_SIZE_128];
       snprintf(buf, BUFF_SIZE_128, "/data/zsl/zsl%d.yuv", frameCnt);
       int file_fd = open(buf, O_RDWR | O_CREAT, 0777);

       if (file_fd < 0) {
         CDBG("cannot open file\n");
       }

       write(file_fd, (const void *)tempFrame->buffer,
         tempFrame->planar1_off * 3 / 2);
       close(file_fd);
       frameCnt++;
     }
#endif
      clock_gettime(CLOCK_REALTIME, &tempFrame->ts);
      is_thumb = (OUTPUT_TYPE_T == tempFrame->path);
      stream_count = (stream_count+1)%2;
      if (is_thumb) {
        add_frame(&context->capture_thumb_queue, tempFrame);
      } else { /* Main Image */
        add_frame(&context->capture_queue, tempFrame);
      }
      CDBG("%s: stream_count %d", __func__, stream_count);
    } while (stream_count);

    if (context->zsl_info.zsl_thread_exit) {
      CDBG("%s: ZSL thread exiting", __func__);
      break;
    }

    if (context->zsl_info.start_capture) {
      CDBG("%s encoding started capture cnt %d", __func__,
        context->capture_complete_cnt);
      context->capture_state = SNAPSHOT_STATE_CAPTURE_WAIT;

      /* Check the appropriate frame to capture */
      CDBG("%s captureQ count %d", __func__,
        context->capture_queue.num_of_frames);
      zsl_docapture(context);
    }

    /* release the next postview buffer to the kernel*/
    releaseFrame = get_frame(&context->capture_thumb_queue);
    if (releaseFrame) {
      status = snapshot_release_picture_frame(context, releaseFrame);
      if (MM_CAMERA_SUCCESS != status) {
        CDBG("%s snapshot_release_picture_frame failed", __func__);
        goto zsl_snapshot_thread_func_end;
      }
      free(releaseFrame);
    }
      /* release the next buffer to the kernel*/
    releaseFrame = get_frame(&context->capture_queue);
    if (releaseFrame) {
      status = snapshot_release_picture_frame(context, releaseFrame);
      if (MM_CAMERA_SUCCESS != status) {
        CDBG("%s snapshot_release_picture_frame failed", __func__);
        goto zsl_snapshot_thread_func_end;
      }
      free(releaseFrame);
    }
  } while (!context->zsl_info.zsl_thread_exit);

zsl_snapshot_thread_func_end:
  if (context->picfd > 0) {
    close(context->picfd);
    context->picfd = -1;
  }
  CDBG("%s Exit %d",__func__, status);
  return NULL;
}

/*===========================================================================
FUNCTION      encode_func

DESCRIPTION
===========================================================================*/
#ifndef DISABLE_JPEG_ENCODING
void* encode_thread_func(void *data)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  snapshot_context_t* context = (snapshot_context_t *)data;
  struct msm_frame* main_frame = NULL;
  struct msm_frame* thumb_frame = NULL;
  context->num_captures = (!context->zsl_enable) ?
    context->capture_params.num_captures :
    context->zsl_info.zsl_capture_params.num_captures;

  CDBG("%s enter %d", __func__, context->num_captures);
  /* Signal main thread*/
  pthread_mutex_lock(&encode_thread_mutex);
  context->encode_state = SNAPSHOT_STATE_ENCODE_WAIT;
  pthread_cond_signal(&encode_thread_cond);
  pthread_mutex_unlock(&encode_thread_mutex);

  do {
    CDBG("%s: encode loop", __func__);
    if (context->encode_thread_exit) {
      CDBG("%s encode_thread_exit", __func__);
      goto encode_func_end;
    }
    /* wait for main image*/
    wait_queue(&context->encode_queue);
    CDBG("%s: wait complete", __func__);
    if (context->encode_thread_exit) {
      CDBG("%s encode_thread_exit", __func__);
      goto encode_func_end;
    }

    main_frame = get_frame(&context->encode_queue);
#ifdef _TARGET_7x2x_
    thumb_frame = get_frame(&context->encode_thumb_queue);
    if (thumb_frame) {
      CDBG("%s got postview frame %p", __func__, (uint8_t *)thumb_frame->buffer);
    } else {
      CDBG_ERROR("%s: cannot get postview image", __func__);
      goto encode_func_end;
    }
#endif
    mmcamera_util_profile("snap got the image");
    if (main_frame) {
      CDBG("%s got frame %p", __func__, (uint8_t *)main_frame->buffer);
    } else {
      CDBG_ERROR("%s: cannot get main image", __func__);
      goto encode_func_end;
    }
    pthread_mutex_lock(&encode_thread_mutex);
    if (context->encode_thread_exit) {
      pthread_mutex_unlock(&encode_thread_mutex);
      CDBG("%s encode_thread_exit", __func__);
      goto encode_func_end;
    }
#ifdef _TARGET_7x2x_
    status = snapshot_jpeg_encode(context, main_frame, thumb_frame);
#else
    status = snapshot_jpeg_encode(context, main_frame);
#endif
    if (MM_CAMERA_SUCCESS != status) {
      CDBG_ERROR("%s snapshot_jpeg_encode failed", __func__);
      context->encode_state = SNAPSHOT_STATE_ENCODE_ERROR;
      pthread_mutex_unlock(&encode_thread_mutex);
      goto encode_func_end;
    }

    /* wait for completion of encoding */
    pthread_cond_wait(&encode_thread_cond, &encode_thread_mutex);
    pthread_mutex_unlock(&encode_thread_mutex);

    CDBG("%s after pthread_cond_wait encode state %d",__func__,
      context->encode_state);
    if (context->notifyIntf->on_event) {
      mm_camera_event event;
      if (SNAPSHOT_STATE_ENCODE_DONE == context->encode_state) {
        event.event_type = JPEG_ENC_DONE;
        event.event_data.encoded_frame =
          &(context->encode_params.p_output_buffer[context->out_buffer_index]);
        event.event_data.encoded_frame->filled_size =
          context->out_buffer_offset;
        context->notifyIntf->on_event(&event);
      }
    }

  encode_join = jpeg_encoder_join;
  if(g_snapshot_context->encode_params.format3d){
#ifdef _TARGET_8660_
    encode_join = mpo_encoder_join;
#endif
  }
    encode_join();

    context->out_buffer_index++;

    pthread_mutex_lock(&encode_thread_mutex);
    context->encode_complete_cnt++;
    pthread_mutex_unlock(&encode_thread_mutex);

    /*release should be called by capture thread*/
    add_frame(&context->capture_queue, main_frame);
#ifdef _TARGET_7x2x_
    add_frame(&context->capture_thumb_queue,thumb_frame);
#endif

    CDBG("%s enc %d capt %d num_capt %d", __func__,
      context->encode_complete_cnt,
      context->capture_complete_cnt, context->num_captures);
  } while ((context->encode_complete_cnt < context->num_captures)
    && (!context->encode_thread_exit)
    && (SNAPSHOT_STATE_ENCODE_CANCEL != context->encode_state));

  pthread_mutex_lock(&encode_thread_mutex);
  if ((SNAPSHOT_STATE_ENCODE_CANCEL != context->encode_state)
    && ((SNAPSHOT_STATE_ENCODE_ERROR != context->encode_state)))
    context->encode_state = SNAPSHOT_STATE_ENCODE_DONE_ALL;
  pthread_mutex_unlock(&encode_thread_mutex);

encode_func_end:
  if (context->notifyIntf->on_event) {
    mm_camera_event event;
    if (SNAPSHOT_STATE_ENCODE_ERROR == context->encode_state) {
      event.event_type = JPEG_ENC_FAILED;
      event.event_data.encoded_frame = NULL;
      context->notifyIntf->on_event(&event);
    }
  }
  CDBG("%s exit", __func__);
  return NULL;
}
#endif //DISABLE_JPEG_ENCODING

/*===========================================================================
 * FUNCTION    - snapshot_delete_buffers -
 *
 * DESCRIPTION:
 *==========================================================================*/
static mm_camera_status_t snapshot_delete_buffers(snapshot_context_t* context)
{
  int i = 0;
  int rc = 0;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int num_buffers = (context->zsl_enable) ? MAX_SNAPSHOT_BUFFERS :
    context->num_captures;
  CDBG("%s enter %d\n", __func__, num_buffers);

  for (i = 0; i < num_buffers; i++ ) {
    /* snapshot buffers */
    if ((0 <= context->snapshot_frame[i].img.fd)
        && (NULL != (void *)(context->snapshot_frame[i].img.buffer))) {
#ifdef USE_ION
      rc = do_munmap_ion(context->ion_dev_fd,
                    &(context->snapshot_frame[i].img.fd_data),
                    (void *)context->snapshot_frame[i].img.buffer,
                    context->snapshot_frame[i].img.ion_alloc.len);
#else
      rc = do_munmap(context->snapshot_frame[i].img.fd,
        (void *)(context->snapshot_frame[i].img.buffer),
        context->snapshot_frame[i].size);
#endif
      CDBG("%s: dommap main image status rc %d i %d", __func__, rc, i);
      if (0 > rc) {
        CDBG("%s: snapshot dealloc failed %d", __func__, rc);
      }
    }
    /* thumbnail buffers */
    if ((0 <= context->thumb_frame[i].img.fd)
        && (NULL != (void *)(context->thumb_frame[i].img.buffer))) {
#ifdef USE_ION
      rc = do_munmap_ion(context->ion_dev_fd,
                    &(context->thumb_frame[i].img.fd_data),
                    (void *)context->thumb_frame[i].img.buffer,
                    context->thumb_frame[i].img.ion_alloc.len);
#else
      rc = do_munmap(context->thumb_frame[i].img.fd,
        (void *)(context->thumb_frame[i].img.buffer),
        context->thumb_frame[i].size);
#endif
      CDBG("%s: dommap thumb status rc %d i %d", __func__, rc, i);
      if (0 > rc) {
        CDBG("%s: snapshot dealloc failed %d", __func__, rc);
      }
    }

  }
  CDBG("%s: Exit %d", __func__, status);
  return status;
}
/*===========================================================================
 * FUNCTION    - snapshot_create_snapshot_buffers -
 *
 * DESCRIPTION:
 *==========================================================================*/
mm_camera_status_t snapshot_create_snapshot_buffers(
  snapshot_context_t* context)
{
  int i = 0;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int num_buffers = (context->zsl_enable) ? MAX_SNAPSHOT_BUFFERS :
    context->num_captures;
  uint32_t width = context->zsl_info.zsl_str_parms.picture_width;
  uint32_t height = context->zsl_info.zsl_str_parms.picture_height;
  CDBG("%s: enter num buffers %d\n", __func__, num_buffers);

  CDBG("%s: res %dx%d\n", __func__, width, height);

  for (i = 0; i < num_buffers; i++ ) {
    context->snapshot_frame[i].size = width *
      CEILING16(height) * 3/2;
#ifdef USE_ION
    context->snapshot_frame[i].img.ion_alloc.len = width *
      CEILING16(height) * 3/2;;
    context->snapshot_frame[i].img.ion_alloc.flags = 0;
    context->snapshot_frame[i].img.ion_alloc.heap_mask =
      (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
    context->snapshot_frame[i].img.ion_alloc.align = 4096;
    context->snapshot_frame[i].img.buffer = (unsigned long)
      do_mmap_ion(context->ion_dev_fd,
      &(context->snapshot_frame[i].img.ion_alloc),
      &(context->snapshot_frame[i].img.fd_data),
      &(context->snapshot_frame[i].img.fd));
#else
    context->snapshot_frame[i].img.buffer = (unsigned long)do_mmap(
      context->snapshot_frame[i].size,
      &(context->snapshot_frame[i].img.fd));
#endif
    context->snapshot_frame[i].img.cropinfo =
      &(context->snapshot_frame[i].frame_crop_info);
    CDBG("%s: dommap status ptr %lx i %d", __func__,
      context->snapshot_frame[i].img.buffer, i);
    if (!context->snapshot_frame[i].img.buffer) {
      status = MM_CAMERA_ERR_PMEM_ALLOC;
      context->snapshot_frame[i].size = 0;
      break;
    }
  }
  CDBG("%s: Exit %d", __func__, status);
  return status;
}

/*===========================================================================
 * FUNCTION    - snapshot_create_thumbnail_buffers -
 *
 * DESCRIPTION:
 *==========================================================================*/
mm_camera_status_t snapshot_create_thumbnail_buffers(snapshot_context_t*
  context)
{
  int i = 0;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int num_buffers = (context->zsl_enable) ? MAX_SNAPSHOT_BUFFERS :
    context->num_captures;
  uint32_t width = context->zsl_info.zsl_str_parms.preview_width;
  uint32_t height = context->zsl_info.zsl_str_parms.preview_height;
  CDBG("%s enter wxh %dx%d\n", __func__, width, height);

  if ( !width || !height) {
    return status;
  }
  for (i = 0; i < num_buffers; i++ ) {
    context->thumb_frame[i].size = width *
      CEILING16(height) * 3/2;
#ifdef USE_ION
    context->thumb_frame[i].img.ion_alloc.len = width *
      CEILING16(height) * 3/2;;
    context->thumb_frame[i].img.ion_alloc.flags = 0;
    context->thumb_frame[i].img.ion_alloc.heap_mask =
      (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
    context->thumb_frame[i].img.ion_alloc.align = 4096;
    context->thumb_frame[i].img.buffer = (unsigned long)
      do_mmap_ion(context->ion_dev_fd,
      &(context->thumb_frame[i].img.ion_alloc),
      &(context->thumb_frame[i].img.fd_data),
      &(context->thumb_frame[i].img.fd));
#else
    context->thumb_frame[i].img.buffer =
      (unsigned long)do_mmap(context->thumb_frame[i].size,
      &(context->thumb_frame[i].img.fd));
#endif
    context->thumb_frame[i].img.cropinfo =
      &(context->thumb_frame[i].frame_crop_info);
    CDBG("%s: dommap status ptr %lx i %d", __func__,
      context->thumb_frame[i].img.buffer, i);
    if (!context->thumb_frame[i].img.buffer) {
      status = MM_CAMERA_ERR_PMEM_ALLOC;
      context->thumb_frame[i].size = 0;
      break;
    }
  }
  CDBG("%s: Exit %d", __func__, status);
  return status;
}

/*===========================================================================
 * FUNCTION    - snapshot_thumbnail_register_buffers -
 *
 * DESCRIPTION:
 *==========================================================================*/
mm_camera_status_t snapshot_thumbnail_register_buffers(
  snapshot_context_t* context, int8_t unregister)
{
  struct msm_pmem_info pmemBuf;
  uint32_t ioctl_cmd;
  int ret;
  int i = 0;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int num_buffers = (context->zsl_enable) ? MAX_SNAPSHOT_BUFFERS :
    context->num_captures;
  CDBG("%s enter\n", __func__);

  pmemBuf.type = MSM_PMEM_THUMBNAIL;
  uint32_t width = context->zsl_info.zsl_str_parms.preview_width;
  uint32_t height = context->zsl_info.zsl_str_parms.preview_height;

  for (i = 0; i < num_buffers; i++ ) {
    pmemBuf.fd = context->thumb_frame[i].img.fd;
    pmemBuf.vaddr = (void *)context->thumb_frame[i].img.buffer;
    pmemBuf.planar0_off = 0;
    pmemBuf.active = (i < ACTIVE_SNAPSHOT_BUFFERS);
    pmemBuf.offset = context->thumb_frame[i].offset;
    pmemBuf.planar1_off = PAD_TO_WORD(width*height);
    pmemBuf.planar2_off = 0;
    pmemBuf.len = PAD_TO_WORD(pmemBuf.planar1_off * 3/2);
    CDBG ("%s: pmemBuf fd %d vaddr %p yoff %d cbcroff %d offset %d len %d\n",
      __func__, pmemBuf.fd, pmemBuf.vaddr, pmemBuf.planar0_off, pmemBuf.planar1_off,
      pmemBuf.offset, pmemBuf.len);

    ioctl_cmd = unregister ? MSM_CAM_IOCTL_UNREGISTER_PMEM :
      MSM_CAM_IOCTL_REGISTER_PMEM;

    if ((ret = ioctl(context->camfd, ioctl_cmd, &pmemBuf)) < 0) {
      CDBG ("%s : ioctl failed... ioctl return value is %d \n", __func__, ret);
      status = MM_CAMERA_ERR_BUFFER_REG;
    }

    if (!unregister) {
      CDBG ("%s: Populate frame[%d]", __func__, i);
      if (i >= ACTIVE_SNAPSHOT_BUFFERS) { /* push to capture thumb Q*/
        CDBG ("%s: add_frame[%d]", __func__, i);
        add_frame(&context->capture_thumb_queue,
          &(context->thumb_frame[i].img));
      }

      context->thumb_frame[i].img.planar1_off = pmemBuf.planar1_off;
      context->thumb_frame[i].img.path = OUTPUT_TYPE_T;
      context->thumb_frame[i].img.planar0_off = pmemBuf.planar0_off;
      context->thumb_frame[i].img.planar2_off = pmemBuf.planar2_off;
    }
  }

snapshot_thumbnail_register_buffers_end:
  CDBG("%s: Exit %d", __func__, status);
  return status;
}

/*===========================================================================
 * FUNCTION    - snapshot_register_buffers -
 *
 * DESCRIPTION:
 *==========================================================================*/
mm_camera_status_t snapshot_register_buffers(snapshot_context_t* context,
  int8_t unregister)
{
  struct msm_pmem_info pmemBuf;
  uint32_t ioctl_cmd;
  int ret;
  int i = 0;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int num_buffers = (context->zsl_enable) ? MAX_SNAPSHOT_BUFFERS :
    context->num_captures;
  CDBG("%s enter unregister %d\n", __func__, unregister);

  pmemBuf.type = MSM_PMEM_MAINIMG;
  uint32_t width = context->zsl_info.zsl_str_parms.picture_width;
  uint32_t height = context->zsl_info.zsl_str_parms.picture_height;
  uint32_t y_off, cbcr_off, len;
#ifndef DISABLE_JPEG_ENCODING
  jpeg_encoder_get_buffer_offset(width, height, &y_off, &cbcr_off, &len );
#endif
  for (i = 0; i < num_buffers; i++ ) {
    pmemBuf.fd = context->snapshot_frame[i].img.fd;
    pmemBuf.vaddr = (void *)context->snapshot_frame[i].img.buffer;
    pmemBuf.planar0_off = y_off;
    pmemBuf.active = i < ACTIVE_SNAPSHOT_BUFFERS;
    pmemBuf.offset = context->snapshot_frame[i].offset;
    pmemBuf.planar1_off = cbcr_off;
    pmemBuf.planar2_off = y_off;
    pmemBuf.len = len;
    CDBG("%s: pmemBuf fd %d vaddr %p yoff %d cbcroff %d offset %d len %d\n",
      __func__, pmemBuf.fd, pmemBuf.vaddr, pmemBuf.planar0_off, pmemBuf.planar1_off,
      pmemBuf.offset, pmemBuf.len);
    /* Add frame to queue */
    if (!unregister) {
      CDBG ("%s: Populate frame[%d]", __func__, i);
      context->snapshot_frame[i].img.planar1_off = pmemBuf.planar1_off;
      context->snapshot_frame[i].img.path = OUTPUT_TYPE_S;
      context->snapshot_frame[i].img.planar0_off = pmemBuf.planar0_off;
      context->snapshot_frame[i].img.planar2_off = pmemBuf.planar2_off;

      if (i >= ACTIVE_SNAPSHOT_BUFFERS) { /* push to capture Q*/
        CDBG ("%s: add_frame[%d]", __func__, i);
        add_frame(&context->capture_queue, &(context->snapshot_frame[i].img));
      }
    }

    ioctl_cmd = unregister ? MSM_CAM_IOCTL_UNREGISTER_PMEM :
      MSM_CAM_IOCTL_REGISTER_PMEM;
    if ((ret = ioctl(context->camfd, ioctl_cmd, &pmemBuf)) < 0) {
      CDBG ("%s: snapshot_thumbnail_register_buffers ioctl failed %d \n",
        __func__, ret);
      status = MM_CAMERA_ERR_BUFFER_REG;;
     }
  }

snapshot_register_buffers_end:
  CDBG("%s: Exit %d", __func__, status);
  return status;
}

