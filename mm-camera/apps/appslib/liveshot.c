
#include<stdio.h>
#include "cam_mmap.h"
#include "liveshot.h"
#include <pthread.h>
#include "camera_dbg.h"
#include "mm_camera_interface.h"

#define DEF_THUMB_W 320
#define DEF_THUMB_H 320
#define Q12  0x00001000

static uint32_t width;
static uint32_t height;
static exif_tags_info_t* exif_data;
static int exif_numEntries;
static uint8_t* frame_buf;
static struct ion_allocation_data frame_ion_alloc;
static struct ion_fd_data frame_fd_data;
static int ion_dev_fd;
static int frame_buf_fd = -1;
static uint32_t frame_buf_size;
static pthread_t liveshot_thread_id;
static uint8_t* out_buffer;
static uint32_t outbuffer_size;
static uint8_t combine_jpeg_buffer;
static jpege_obj_t jpeg_encoder;
static exif_info_obj_t exif_info;
static uint8_t jpeg_encoder_initialized;
static uint8_t* out_buffer_temp;
static jpege_img_data_t liveshot_img_info;
static jpege_src_t jpege_source;
static jpege_dst_t jpege_dest;
static jpege_cfg_t jpege_config;
static pthread_cond_t  liveshot_thread_cond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t liveshot_thread_mutex = PTHREAD_MUTEX_INITIALIZER;

static int8_t liveshot_jpeg_encoder_encode(void);
static void liveshot_jpeg_encoder_deinit(void);
void liveshot_jpege_output_produced_handler(void *p_user_data, void *p_arg,
  jpeg_buffer_t buffer);
int liveshot_jpege_output_produced_handler2(void *p_user_data, void *p_arg,
  jpeg_buffer_t buffer, uint8_t last_flag);
void live_shot_jpege_event_handler(void *p_user_data, jpeg_event_t event, void *p_arg);

typedef enum {
    LIVESHOT_STATE_STOPPED = 0,
    LIVESHOT_STATE_INIT,
    LIVESHOT_STATE_CAPTURE_DONE,
    LIVESHOT_STATE_INPROGRESS,
    LIVESHOT_STATE_ENCODE_DONE,
    LIVESHOT_STATE_ENCODE_ERROR,
    LIVESHOT_STATE_CANCEL
}liveshot_state;
static liveshot_state state = LIVESHOT_STATE_STOPPED;

/*===========================================================================
FUNCTION      set_liveshot_params

DESCRIPTION
===========================================================================*/
int8_t set_liveshot_params(uint32_t a_width, uint32_t a_height, exif_tags_info_t *a_exif_data,
                         int a_exif_numEntries, uint8_t* a_out_buffer, uint32_t a_outbuffer_size)
{
  CDBG("set_liveshot_params() w=%u h=%u\n",a_width, a_height);
  pthread_mutex_lock(&liveshot_thread_mutex);
  if (LIVESHOT_STATE_STOPPED != state) {
     CDBG("set_liveshot_params() failed, previous liveshot running ");
     pthread_mutex_unlock(&liveshot_thread_mutex);
     return FALSE;
  }
  width = a_width;
  height = a_height;
  exif_data = a_exif_data;
  exif_numEntries = a_exif_numEntries;
  out_buffer = a_out_buffer;
  out_buffer_temp = out_buffer;
  outbuffer_size = a_outbuffer_size;
  state = LIVESHOT_STATE_INIT;
  pthread_mutex_unlock(&liveshot_thread_mutex);
  return TRUE;
}

/*===========================================================================
FUNCTION      cancel_liveshot

DESCRIPTION
===========================================================================*/
int8_t cancel_liveshot() {
  pthread_mutex_lock(&liveshot_thread_mutex);
  CDBG("cancel_liveshot(), state %d\n",state);
  if (LIVESHOT_STATE_STOPPED == state) {
    pthread_mutex_unlock(&liveshot_thread_mutex);
    return TRUE;
  }
  if (LIVESHOT_STATE_INPROGRESS == state) {
    state = LIVESHOT_STATE_CANCEL;
    pthread_cond_signal(&liveshot_thread_cond);
  }
  pthread_mutex_unlock(&liveshot_thread_mutex);
  if (liveshot_thread_id >= 0) {
      CDBG("cancel_liveshot(), wait for thread exit\n");
      pthread_join(liveshot_thread_id, NULL);
  }
  CDBG("cancel_liveshot(), exit\n");
  return TRUE;
}

/*===========================================================================
FUNCTION      liveshot_thread

DESCRIPTION
===========================================================================*/
static void *liveshot_thread(void *data)
{
  int result = TRUE;

  CDBG("liveshot_thread(), start\n");
  pthread_mutex_lock(&liveshot_thread_mutex);
  CDBG("liveshot_thread(), state %d\n",state);
  if (state == LIVESHOT_STATE_CANCEL) {
    goto liveshot_thread_error;
  }

#ifndef DISABLE_JPEG_ENCODING
  int rc = jpege_init(&jpeg_encoder, live_shot_jpege_event_handler, NULL);
  if (JPEGERR_SUCCESS != rc) {
    /* Give error callback*/
    CDBG("liveshot_thread(), init failed\n");
    result = FALSE;
    goto liveshot_thread_error;
  }

  jpeg_encoder_initialized = 1;
#endif /* DISABLE_JPEG_ENCODING */
  pthread_mutex_unlock(&liveshot_thread_mutex);

  result = liveshot_jpeg_encoder_encode();
  if (FALSE == result) {
    /* Give error callback*/
    CDBG("liveshot_thread(), encode failed\n");
    result = FALSE;
    goto liveshot_thread_error;
  }

  pthread_mutex_lock(&liveshot_thread_mutex);
  if (LIVESHOT_STATE_CANCEL == state) {
    goto liveshot_thread_error;
  }
  state = LIVESHOT_STATE_INPROGRESS;

  /* wait for completion of encoding */
  pthread_cond_wait(&liveshot_thread_cond, &liveshot_thread_mutex);

liveshot_thread_error:
  CDBG("liveshot_thread(), result %d state %d",result, state);
  liveshot_jpeg_encoder_deinit();

  if ((FALSE == result) || (LIVESHOT_STATE_ENCODE_ERROR == state)) {
    /* Give error callback */
    if (get_notify_obj()->on_liveshot_event)
      get_notify_obj()->on_liveshot_event(LIVESHOT_ENCODE_ERROR, 0);
  }else if(LIVESHOT_STATE_ENCODE_DONE == state) {
    if(get_notify_obj()->on_liveshot_event)
      get_notify_obj()->on_liveshot_event(LIVESHOT_SUCCESS,
        out_buffer_temp - out_buffer);
  }
  state = LIVESHOT_STATE_STOPPED;
  pthread_mutex_unlock(&liveshot_thread_mutex);
  CDBG("liveshot_thread(), exit ");
  return NULL;
}

/*===========================================================================
FUNCTION      set_liveshot_frame

DESCRIPTION
===========================================================================*/
void set_liveshot_frame(struct msm_frame* liveshot_frame)
{
  int result = TRUE;
  pthread_mutex_lock(&liveshot_thread_mutex);
  CDBG("set_liveshot_frame(), state %d\n",state);
  liveshot_frame->path &= ~OUTPUT_TYPE_L;

  if (LIVESHOT_STATE_CANCEL == state) {
    goto setframe_exit;
  }else if (LIVESHOT_STATE_INIT != state) {
    CDBG("set_liveshot_frame() liveshot params not set");
    pthread_mutex_unlock(&liveshot_thread_mutex);
    return;
  }
  state = LIVESHOT_STATE_CAPTURE_DONE;
  pthread_mutex_unlock(&liveshot_thread_mutex);
#ifdef USE_ION
    ion_dev_fd = open("/dev/ion", O_RDONLY | O_SYNC);
    if (ion_dev_fd < 0) {
      CDBG_ERROR("Ion dev open failed\n");
      CDBG_ERROR("Error is %s\n", strerror(errno));
      result = FALSE;
      goto setframe_exit;
    }
#endif
  frame_buf_size = width * height * 3/2;
  frame_buf_fd = -1;
#ifdef USE_ION
  frame_ion_alloc.len = width * height * 3/2;
  frame_ion_alloc.flags = 0;
  frame_ion_alloc.heap_mask = (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
  frame_ion_alloc.align = 4096;
  frame_buf = do_mmap_ion(ion_dev_fd,
        &(frame_ion_alloc),
        &(frame_fd_data),
        (int *)&(frame_buf_fd));
#else
  frame_buf = (uint8_t*) do_mmap(frame_buf_size, &frame_buf_fd);
#endif
  if (NULL == frame_buf) {
      /* Give error callback*/
      CDBG("start_liveshot(), frame_buf is NULL\n");
      result = FALSE;
      goto setframe_exit;
  }

  /* Copy the main buffer and startsnapshot in different thread*/
  int size = width * height;
  memcpy(frame_buf, (uint8_t *)liveshot_frame->buffer, size);
  memcpy(frame_buf+size, (uint8_t *)liveshot_frame->buffer+liveshot_frame->planar1_off, size/2);
  int rc = pthread_create(&liveshot_thread_id, NULL, liveshot_thread, NULL);
  if (rc < 0) {
      /* Give error callback*/
      CDBG("start_liveshot(), pthread_create failed\n");
      result = FALSE;
      goto setframe_exit;
  }

setframe_exit:
  CDBG("set_liveshot_frame(), result %d state %d",result, state);
  pthread_mutex_lock(&liveshot_thread_mutex);
  if ((FALSE == result) || (LIVESHOT_STATE_CANCEL == state)) {
      liveshot_jpeg_encoder_deinit();
      if (LIVESHOT_STATE_CANCEL != state) {
        /* Give error callback */
        if(get_notify_obj()->on_liveshot_event)
          get_notify_obj()->on_liveshot_event(LIVESHOT_ENCODE_ERROR, 0);
      }
      state = LIVESHOT_STATE_STOPPED;
  }
  pthread_mutex_unlock(&liveshot_thread_mutex);
  CDBG("set_liveshot_frame() exit");

}

/*===========================================================================
FUNCTION      liveshot_thread_ready_signal

DESCRIPTION
===========================================================================*/
static void liveshot_thread_ready_signal(liveshot_state a_state)
{
  CDBG("liveshot_thread_ready_signal() is ready, call pthread_cond_signal state %d\n",a_state);
  pthread_mutex_lock(&liveshot_thread_mutex);
  if (LIVESHOT_STATE_CANCEL != state) {
    state = a_state;
  }
  pthread_cond_signal(&liveshot_thread_cond);
  pthread_mutex_unlock(&liveshot_thread_mutex);
}

/*===========================================================================
FUNCTION      live_shot_jpege_event_handler

DESCRIPTION
===========================================================================*/
void live_shot_jpege_event_handler(void *p_user_data, jpeg_event_t event, void *p_arg)
{
  CDBG("jpege_event_handler is called with event %d!\n", event);
  if (event == JPEG_EVENT_DONE) {
    liveshot_thread_ready_signal(LIVESHOT_STATE_ENCODE_DONE);
  } else {
    liveshot_thread_ready_signal(LIVESHOT_STATE_ENCODE_ERROR);
  }
}

/*===========================================================================
FUNCTION      liveshot_jpege_output_produced_handler

DESCRIPTION
===========================================================================*/
void liveshot_jpege_output_produced_handler(void *p_user_data, void *p_arg,
  jpeg_buffer_t buffer)
{
  uint32_t buf_size;
  uint8_t *buf_ptr;

#ifndef DISABLE_JPEG_ENCODING
  jpeg_buffer_get_actual_size(buffer, &buf_size);
  jpeg_buffer_get_addr(buffer, &buf_ptr);
  CDBG("liveshot_jpege_output_produced_handler buf_ptr %p buf_size %d",buf_ptr, buf_size);

  if (buf_ptr && buf_size && (out_buffer_temp < (out_buffer + outbuffer_size - buf_size))) {
      memcpy(out_buffer_temp, buf_ptr, buf_size);
      out_buffer_temp += buf_size;
  } else {
      CDBG("liveshot_jpege_output_produced_handler copy failed");
  }
#endif /* DISABLE_JPEG_ENCODING */
}

#if !defined(_TARGET_7x2x_) && !defined(_TARGET_7x27A_) 
/*===========================================================================
FUNCTION      liveshot_jpege_output_produced_handler2

DESCRIPTION
===========================================================================*/
int liveshot_jpege_output_produced_handler2(void *p_user_data, void *p_arg,
  jpeg_buffer_t buffer, uint8_t last_flag)
{
  uint32_t buf_size;
  uint8_t *buf_ptr;
  int rv;

#ifndef DISABLE_JPEG_ENCODING
  jpeg_buffer_get_actual_size(buffer, &buf_size);
  jpeg_buffer_get_addr(buffer, &buf_ptr);
  CDBG("liveshot_jpege_output_produced_handler2 buf_ptr %p buf_size %d",buf_ptr, buf_size);

  if (buf_ptr && buf_size && (out_buffer_temp < (out_buffer + outbuffer_size - buf_size))) {
      memcpy(out_buffer_temp, buf_ptr, buf_size);
      out_buffer_temp += buf_size;
  } else {
      CDBG("liveshot_jpege_output_produced_handler2 copy failed");
  }
  rv = jpeg_buffer_set_actual_size(buffer, 0);
  if(rv == JPEGERR_SUCCESS){
      rv = jpege_enqueue_output_buffer(
              jpeg_encoder,
              &buffer, 1);
  }
#endif /* DISABLE_JPEG_ENCODING */
  return rv;
}
#endif

/*===========================================================================
FUNCTION      liveshot_jpeg_encoder_deinit

DESCRIPTION
===========================================================================*/
static void liveshot_jpeg_encoder_deinit(void)
{
  CDBG("liveshot_jpeg_encoder_deinit S");

  if (jpeg_encoder_initialized) {
    jpeg_encoder_initialized = 0;
#ifndef DISABLE_JPEG_ENCODING
    jpege_abort(jpeg_encoder);
    jpeg_buffer_destroy(&liveshot_img_info.p_fragments[0].color.yuv.luma_buf);
    jpeg_buffer_destroy(&liveshot_img_info.p_fragments[0].color.yuv.chroma_buf);
    jpeg_buffer_destroy(&jpege_dest.buffers[0]);
    jpeg_buffer_destroy(&jpege_dest.buffers[1]);
    exif_destroy(&exif_info);
    jpege_destroy(&jpeg_encoder);
#endif /* DISABLE_JPEG_ENCODING */
  }

  int rc = 0;
  if (frame_buf) {
#ifdef USE_ION
        do_munmap_ion(ion_dev_fd,
                    &(frame_fd_data),
                    frame_buf,
                    frame_ion_alloc.len);
#else
    rc = do_munmap(frame_buf_fd, frame_buf, frame_buf_size);
#endif
    CDBG("do_munmap liveshot buffer return value: %d\n", rc);
    frame_buf = 0;
    frame_buf_size = 0;
  }
#ifdef USE_ION
  close(ion_dev_fd);
#endif
  CDBG("liveshot_jpeg_encoder_deinit E");
}

/*===========================================================================
FUNCTION      liveshot_jpeg_encoder_encode

DESCRIPTION
===========================================================================*/
static int8_t liveshot_jpeg_encoder_encode(void)
{
  int rc = 0;

  pthread_mutex_lock(&liveshot_thread_mutex);

#ifndef DISABLE_JPEG_ENCODING
  exif_init(&exif_info);
  memset(&liveshot_img_info, 0, sizeof(jpege_img_data_t));
  memset(&jpege_source, 0, sizeof(jpege_src_t));
  memset(&jpege_dest, 0, sizeof(jpege_dst_t));


  if ((rc = jpeg_buffer_init(&liveshot_img_info.p_fragments[0].color.yuv.luma_buf)) ||
    (rc = jpeg_buffer_init(&liveshot_img_info.p_fragments[0].color.yuv.chroma_buf))
    || (rc = jpeg_buffer_init(&jpege_dest.buffers[0]))
    || (rc = jpeg_buffer_init(&jpege_dest.buffers[1]))) {
    CDBG("liveshot_jpeg_encoder_encode jpeg_buffer_init failed: %d\n", rc);
    pthread_mutex_unlock(&liveshot_thread_mutex);
    return FALSE;
  }
  jpege_dest.buffer_cnt = 2;
  if ((rc = jpeg_buffer_allocate(jpege_dest.buffers[0], 64*1024, 0)) ||
    (rc = jpeg_buffer_allocate(jpege_dest.buffers[1], 64*1024, 0))) {
    CDBG("liveshot_jpeg_encoder_encode jpeg_buffer_allocate failed: %d\n", rc);
    pthread_mutex_unlock(&liveshot_thread_mutex);
    jpege_dest.buffer_cnt = 0;
    return FALSE;
  }

#if !defined(_TARGET_7x2x_) && !defined(_TARGET_7x27A_)
  jpege_dest.p_buffer = &jpege_dest.buffers[0];
#endif

  CDBG("liveshot_jpeg_encoder_encode size %dx%d %u\n",width, height, frame_buf_size);
  liveshot_img_info.width = width;
  liveshot_img_info.height = height;
  liveshot_img_info.fragment_cnt = 1;
#if defined(_TARGET_7x2x_) || defined(_TARGET_7x27A_)
  liveshot_img_info.color_format = YCRCBLP_H2V2;
#else
  liveshot_img_info.color_format = YCBCRLP_H2V2;
#endif
  liveshot_img_info.p_fragments[0].width = width;
  liveshot_img_info.p_fragments[0].height = CEILING16(height);
  jpeg_buffer_reset(liveshot_img_info.p_fragments[0].color.yuv.luma_buf);
  jpeg_buffer_reset(liveshot_img_info.p_fragments[0].color.yuv.chroma_buf);

  rc =
    jpeg_buffer_use_external_buffer(
            liveshot_img_info.p_fragments[0].color.yuv.luma_buf,
            frame_buf, frame_buf_size,
            frame_buf_fd);

  if (rc == JPEGERR_EFAILED) {
    CDBG("liveshot_jpeg_encoder_encode jpeg_buffer_use_external_buffer Snapshot pmem failed...\n");
    pthread_mutex_unlock(&liveshot_thread_mutex);
    return FALSE;
  }
  int cbcroffset = liveshot_img_info.width * liveshot_img_info.height;

  CDBG("liveshot_jpeg_encoder_encode: cbcroffset %d",cbcroffset);
  jpeg_buffer_attach_existing(liveshot_img_info.p_fragments[0].color.yuv.chroma_buf,
    liveshot_img_info.p_fragments[0].color.yuv.luma_buf,
    cbcroffset);
  jpeg_buffer_set_actual_size(liveshot_img_info.p_fragments[0].color.yuv.luma_buf, liveshot_img_info.width * liveshot_img_info.height);
  jpeg_buffer_set_actual_size(liveshot_img_info.p_fragments[0].color.yuv.chroma_buf, liveshot_img_info.width * liveshot_img_info.height / 2);

  /*  Set Source */
  jpege_source.p_thumbnail = jpege_source.p_main = &liveshot_img_info;

  rc = jpege_set_source(jpeg_encoder, &jpege_source);
  if (rc) {
    CDBG("liveshot_jpeg_encoder_encode jpege_set_source failed: %d\n", rc);
    pthread_mutex_unlock(&liveshot_thread_mutex);
    return FALSE;
  }

#if defined(_TARGET_7x2x_) || defined(_TARGET_7x27A_)
  jpege_dest.p_output_handler = (jpege_output_handler_t) liveshot_jpege_output_produced_handler;
#else
  jpege_dest.p_output_handler = liveshot_jpege_output_produced_handler2;
#endif

  jpege_dest.buffer_cnt = 2;
  rc = jpege_set_destination(jpeg_encoder, &jpege_dest);
  if (rc) {
    CDBG("liveshot_jpeg_encoder_encode jpege_set_desination failed: %d\n", rc);
    pthread_mutex_unlock(&liveshot_thread_mutex);
    return FALSE;
  }
  /*  Get default configuration */
  jpege_get_default_config(&jpege_config);
  jpege_config.thumbnail_present = 1;
  jpege_config.preference = JPEG_ENCODER_PREF_SOFTWARE_ONLY;
  jpege_config.main_cfg.quality = 85;
  jpege_config.thumbnail_cfg.quality = 75; /* to fit 64K APP1 marker length*/

  /* Upscaler information  for main image */
  jpege_config.main_cfg.scale_cfg.enable = FALSE;

  /* Scaler information  for thumbnail */
  jpege_config.thumbnail_cfg.scale_cfg.enable = TRUE;

  jpege_config.thumbnail_cfg.scale_cfg.input_width = width;
  jpege_config.thumbnail_cfg.scale_cfg.input_height = height;

  jpege_config.thumbnail_cfg.scale_cfg.h_offset = 0;
  jpege_config.thumbnail_cfg.scale_cfg.v_offset = 0;

  jpege_config.thumbnail_cfg.scale_cfg.output_width = DEF_THUMB_W;
  int thumb_h = ((float)DEF_THUMB_W * (float)height / (float)width);
  CDBG("liveshot_jpeg_encoder_encode thumb_h %d ",thumb_h);
  jpege_config.thumbnail_cfg.scale_cfg.output_height = thumb_h;

  jpege_config.main_cfg.rotation_degree_clk = 0;
  jpege_config.thumbnail_cfg.rotation_degree_clk = 0;

  int i = 0;
  if( exif_data != NULL) {
    for(i = 0; i < exif_numEntries; i++) {
      rc = exif_set_tag(exif_info, exif_data[i].tag_id,
                         &(exif_data[i].tag_entry));
      if (rc) {
        CDBG("liveshot_jpeg_encoder_encode exif_set_tag failed: %d\n", rc);
        pthread_mutex_unlock(&liveshot_thread_mutex);
        return FALSE;
      }
    }
  }
  /*  Start encoder */
  rc = jpege_start(jpeg_encoder, &jpege_config, &exif_info);
  if (rc) {
    CDBG("jpege_start failed: %d\n", rc);
    pthread_mutex_unlock(&liveshot_thread_mutex);
    return FALSE;
  }
#endif /* DISABLE_JPEG_ENCODING */

  pthread_mutex_unlock(&liveshot_thread_mutex);
  return TRUE;
}

