/*============================================================================

   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <jpeg_encoder.h>
#include "camera_dbg.h"
#ifdef _ANDROID_
#include <sys/system_properties.h>
#endif
#include "mm_camera_interface.h"

#ifdef USE_GEMINI
#include "gemini_lib.h"
#include "gemini_inline.h"
#endif

#ifdef JPG_DBG
  #ifdef _ANDROID_
    #undef CDBG
    #undef LOG_NIDEBUG
    #undef LOG_TAG
    #define LOG_NIDEBUG 0
    #define LOG_TAG "mm-camera jpeg"
    #include <utils/Log.h>
    #define CDBG(fmt, args...) LOGE(fmt, ##args)
    #endif
#endif

#define JPEG_DEFAULT_MAINIMAGE_QUALITY 75
#define JPEG_DEFAULT_THUMBNAIL_QUALITY 75

int is_encoding = 0;
pthread_mutex_t jpege_mutex = PTHREAD_MUTEX_INITIALIZER;

int rc;
jpege_src_t jpege_source;
jpege_dst_t jpege_dest;
jpege_cfg_t jpege_config;
jpege_img_data_t main_img_info, tn_img_info;
jpeg_buffer_t temp;
jpege_obj_t jpeg_encoder;
exif_info_obj_t exif_info;
exif_tag_entry_t sample_tag;
struct timeval tdBefore, tdAfter;
struct timezone tz;
static uint32_t jpegMainimageQuality = JPEG_DEFAULT_MAINIMAGE_QUALITY;
static uint32_t jpegThumbnailQuality = JPEG_DEFAULT_THUMBNAIL_QUALITY;
static uint32_t jpegRotation = 0;
static int8_t usethumbnail = 1;
static int8_t use_thumbnail_padding = 0;
static uint32_t phy_offset;
#ifdef HW_ENCODE
static uint8_t hw_encode = true;
#else
static uint8_t hw_encode = false;
#endif
void (*mmcamera_jpegfragment_callback)(uint8_t * buff_ptr, uint32_t buff_size);
void (*mmcamera_jpeg_callback)(jpeg_event_t);
static int8_t is_3dmode = 0;
static cam_3d_frame_format_t img_format_3d;

#define JPEGE_FRAGMENT_SIZE (64*1024)

/*===========================================================================
FUNCTION      jpege_event_handler

DESCRIPTION   Set physical offset for the buffer
===========================================================================*/
void jpege_set_phy_offset(uint32_t a_phy_offset)
{
  phy_offset = a_phy_offset;
}

/*===========================================================================
FUNCTION      jpege_event_handler

DESCRIPTION   Handler function for jpeg encoder events
===========================================================================*/
inline void jpege_use_thumb_padding(uint8_t a_use_thumb_padding)
{
  use_thumbnail_padding = a_use_thumb_padding;
}

/*===========================================================================
FUNCTION      jpege_event_handler

DESCRIPTION   Handler function for jpeg encoder events
===========================================================================*/
void jpege_event_handler(void *p_user_data, jpeg_event_t event, void *p_arg)
{
  uint32_t buf_size;
  uint8_t *buf_ptr = NULL;
  int mainimg_fd, thumbnail_fd;

  CDBG("jpege_event_handler is called with event %d!\n", event);
  if (event == JPEG_EVENT_DONE) {

    jpeg_buffer_t thumbnail_buffer, snapshot_buffer;

    thumbnail_buffer = tn_img_info.p_fragments[0].color.yuv.luma_buf;
    thumbnail_fd = jpeg_buffer_get_pmem_fd(thumbnail_buffer);
    jpeg_buffer_get_actual_size(thumbnail_buffer, &buf_size);
    jpeg_buffer_get_addr(thumbnail_buffer, &buf_ptr);

    snapshot_buffer = main_img_info.p_fragments[0].color.yuv.luma_buf;
    mainimg_fd = jpeg_buffer_get_pmem_fd(snapshot_buffer);
    jpeg_buffer_get_actual_size(snapshot_buffer, &buf_size);
    jpeg_buffer_get_addr(snapshot_buffer, &buf_ptr);

#if 0
    gettimeofday(&tdAfter, &tz);
    CDBG("Profiling: JPEG encoding latency %ld microseconds\n",
      1000000 * (tdAfter.tv_sec - tdBefore.tv_sec) + tdAfter.tv_usec -
      tdBefore.tv_usec);
#endif
    mmcamera_util_profile("encoder done");
  }
  if(mmcamera_jpeg_callback)
    mmcamera_jpeg_callback(event);

}

/*===========================================================================
FUNCTION      jpege_output_produced_handler

DESCRIPTION   Handler function for when jpeg encoder has output produced
===========================================================================*/
void jpege_output_produced_handler(void *p_user_data, void *p_arg,
  jpeg_buffer_t buffer)
{
  uint32_t buf_size;
  uint8_t *buf_ptr;

  /*  The mutex is to prevent the very rare case where the file writing is */
  /*  so slow that the next ping-pong output is delivered before the */
  /*  current one is finished writing, in which case the writing of the new */
  /*  buffer will be performed after the first one finishes (because of the lock) */

  jpeg_buffer_get_actual_size(buffer, &buf_size);
  jpeg_buffer_get_addr(buffer, &buf_ptr);

  if(mmcamera_jpegfragment_callback)
    mmcamera_jpegfragment_callback(buf_ptr, buf_size);
}

#if !defined(_TARGET_7x2x_) && !defined(_TARGET_7x27A_)
/*===========================================================================
FUNCTION      jpege_output_produced_handler2

DESCRIPTION   Handler function for when jpeg encoder has output produced
===========================================================================*/
int jpege_output_produced_handler2(void *p_user_data, void *p_arg,
  jpeg_buffer_t buffer, uint8_t last_buf_flag)
{
  uint32_t buf_size;
  uint8_t *buf_ptr;
  int rv;

  /*  The mutex is to prevent the very rare case where the file writing is */
  /*  so slow that the next ping-pong output is delivered before the */
  /*  current one is finished writing, in which case the writing of the new */
  /*  buffer will be performed after the first one finishes (because of the lock) */
  jpeg_buffer_get_actual_size(buffer, &buf_size);
  jpeg_buffer_get_addr(buffer, &buf_ptr);

  if(mmcamera_jpegfragment_callback)
    mmcamera_jpegfragment_callback(buf_ptr, buf_size);

  rv = jpeg_buffer_set_actual_size(buffer, 0);
  if(rv == JPEGERR_SUCCESS){
      rv = jpege_enqueue_output_buffer(
          jpeg_encoder,
          &buffer, 1);
  }
  return rv;
}
#endif

static int jpeg_encoder_initialized = 0;

void jpeg_encoder_set_3D_info(cam_3d_frame_format_t format)
{
  pthread_mutex_lock(&jpege_mutex);
  is_3dmode = 1;
  img_format_3d = format;
  pthread_mutex_unlock(&jpege_mutex);
}

int8_t jpeg_encoder_init()
{
  pthread_mutex_lock(&jpege_mutex);
  #ifdef HW_ENCODE
  static uint8_t hw_encode = true;
  #else
  static uint8_t hw_encode = false;
  #endif
  is_3dmode = 0;

  /*  Initialize jpeg encoder */
  rc = jpege_init(&jpeg_encoder, jpege_event_handler, NULL);
  if (rc) {
    CDBG("jpege_init failed: %d\n", rc);
    pthread_mutex_unlock(&jpege_mutex);
    return FALSE;
  }

  jpeg_encoder_initialized = 1;
  phy_offset = 0;
  pthread_mutex_unlock(&jpege_mutex);

  return TRUE;
}

void jpeg_encoder_join(void)
{
  pthread_mutex_lock(&jpege_mutex);
  if (jpeg_encoder_initialized) {
    jpeg_encoder_initialized = 0;
    pthread_mutex_destroy(&jpege_mutex);
    jpege_abort(jpeg_encoder);
    jpeg_buffer_destroy(&temp);
    if (usethumbnail) {
        jpeg_buffer_destroy(&tn_img_info.p_fragments[0].color.yuv.luma_buf);
        jpeg_buffer_destroy(&tn_img_info.p_fragments[0].color.yuv.chroma_buf);
    }
    jpeg_buffer_destroy(&main_img_info.p_fragments[0].color.yuv.luma_buf);
    jpeg_buffer_destroy(&main_img_info.p_fragments[0].color.yuv.chroma_buf);
    jpeg_buffer_destroy(&jpege_dest.buffers[0]);
    jpeg_buffer_destroy(&jpege_dest.buffers[1]);
    exif_destroy(&exif_info);
    jpege_destroy(&jpeg_encoder);
  }
  is_3dmode = 0;
  pthread_mutex_unlock(&jpege_mutex);
}
/* This function returns the Yoffset and CbCr offset requirements for the Jpeg encoding*/
int8_t jpeg_encoder_get_buffer_offset(uint32_t width, uint32_t height,
                                      uint32_t* p_y_offset, uint32_t* p_cbcr_offset,
                                      uint32_t* p_buf_size)
{
  CDBG("jpeg_encoder_get_buffer_offset");
  if ((NULL == p_y_offset) || (NULL == p_cbcr_offset)) {
    return FALSE;
  }

  if (hw_encode) {
    int cbcr_offset = 0;
    uint32_t actual_size = width*height;
    uint32_t padded_size = width * CEILING16(height);
    *p_y_offset = 0;
    *p_cbcr_offset = padded_size;
    if ((jpegRotation == 90) || (jpegRotation == 180)) {
      *p_y_offset += padded_size - actual_size;
      *p_cbcr_offset += ((padded_size - actual_size) >> 1);
    }
    *p_buf_size = padded_size * 3/2;
  } else {
    *p_y_offset = 0;
    *p_cbcr_offset = PAD_TO_WORD(width*height);
    *p_buf_size = *p_cbcr_offset * 3/2;
  }
  return TRUE;
}

int8_t jpeg_encoder_encode(const cam_ctrl_dimension_t * dimension, const
  uint8_t * thumbnail_buf,
  int thumbnail_fd, const uint8_t * snapshot_buf, int snapshot_fd,
  common_crop_t *scaling_params, exif_tags_info_t *exif_data, int exif_numEntries,
  const int32_t a_cbcroffset,
  cam_point_t* main_crop_offset,
  cam_point_t* thumb_crop_offset,int zsl_enable)
{
  int buf_size = 0;
  int ret = 0;
  int i = 0;
  int cbcroffset = 0;
  int actual_size = 0, padded_size = 0;
  usethumbnail = thumbnail_buf ? 1 : 0;
  int w_scale_factor = (is_3dmode && img_format_3d == SIDE_BY_SIDE_FULL) ? 2 : 1;
  jpege_get_default_config(&jpege_config);
  if(jpegRotation && zsl_enable) {
     jpege_config.preference = JPEG_ENCODER_PREF_SOFTWARE_ONLY;
  }
  else {
      #ifdef HW_ENCODE
      jpege_config.preference = JPEG_ENCODER_PREF_HW_ACCELERATED_PREFERRED;
      #else
      jpege_config.preference = JPEG_ENCODER_PREF_SOFTWARE_ONLY;
      #endif
  }

  pthread_mutex_lock(&jpege_mutex);
  mmcamera_util_profile("encoder configure");

  /*  Do not allow snapshot if the previous one is not done */
  /*  Alternately we can queue the snapshot to be done after the one in progress is completed, */
  /*  but it involves more complex logic */
  if (is_encoding) {
    CDBG("Previous Jpeg Encoding is not done!\n");
    pthread_mutex_unlock(&jpege_mutex);
    return FALSE;
  }
  CDBG("jpeg_encoder_encode: thumbnail_fd = %d snapshot_fd = %d usethumbnail %d\n",
    thumbnail_fd, snapshot_fd, usethumbnail);

  gettimeofday(&tdBefore, &tz);
  /*  Initialize exif info */
  exif_init(&exif_info);
  /*  Zero out supporting structures */
  memset(&main_img_info, 0, sizeof(jpege_img_data_t));
  memset(&tn_img_info, 0, sizeof(jpege_img_data_t));
  memset(&jpege_source, 0, sizeof(jpege_src_t));
  memset(&jpege_dest, 0, sizeof(jpege_dst_t));

  /*  Initialize JPEG buffers */
  jpege_dest.buffer_cnt = 2;
  if ((rc = jpeg_buffer_init(&temp)) ||
    (usethumbnail && (rc = jpeg_buffer_init(&tn_img_info.p_fragments[0].color.yuv.luma_buf))) ||
    (usethumbnail && (rc = jpeg_buffer_init(&tn_img_info.p_fragments[0].color.yuv.chroma_buf))) ||
    (rc = jpeg_buffer_init(&main_img_info.p_fragments[0].color.yuv.luma_buf)) ||
    (rc = jpeg_buffer_init(&main_img_info.p_fragments[0].color.yuv.chroma_buf))
    || (rc = jpeg_buffer_init(&jpege_dest.buffers[0]))
    || (rc = jpeg_buffer_init(&jpege_dest.buffers[1]))) {
    CDBG_ERROR("jpeg_buffer_init failed: %d\n", rc);
    pthread_mutex_unlock(&jpege_mutex);
    jpege_dest.buffer_cnt = 0;
    return FALSE;
  }
#if !defined(_TARGET_7x2x_) && !defined(_TARGET_7x27A_)
  jpege_dest.p_buffer = &jpege_dest.buffers[0];
#endif

#if defined(_TARGET_7x27A_)
  /*  Allocate 2 ping-pong buffers on the heap for jpeg encoder outputs */
  if ((rc = jpeg_buffer_allocate(jpege_dest.buffers[0], JPEGE_FRAGMENT_SIZE, 1)) ||
    (rc = jpeg_buffer_allocate(jpege_dest.buffers[1], JPEGE_FRAGMENT_SIZE, 1))) {
    CDBG("jpeg_buffer_allocate failed: %d\n", rc);
    pthread_mutex_unlock(&jpege_mutex);
    return FALSE;
  }
#else
  /*  Allocate 2 ping-pong buffers on the heap for jpeg encoder outputs */
  if ((rc = jpeg_buffer_allocate(jpege_dest.buffers[0], JPEGE_FRAGMENT_SIZE, 0)) ||
    (rc = jpeg_buffer_allocate(jpege_dest.buffers[1], JPEGE_FRAGMENT_SIZE, 0))) {
    CDBG("jpeg_buffer_allocate failed: %d\n", rc);
    pthread_mutex_unlock(&jpege_mutex);
    return FALSE;
  }
#endif


  if (usethumbnail) {
    tn_img_info.width = dimension->thumbnail_width * w_scale_factor;
    tn_img_info.height = dimension->thumbnail_height;
    buf_size = tn_img_info.width * tn_img_info.height * 2;
    tn_img_info.fragment_cnt = 1;
    if (dimension->thumb_format == CAMERA_YUV_422_NV61)
      tn_img_info.color_format = YCRCBLP_H2V1;
    else
      tn_img_info.color_format = YCRCBLP_H2V2;
    tn_img_info.p_fragments[0].width = tn_img_info.width;
    tn_img_info.p_fragments[0].height = CEILING16(dimension->thumbnail_height);
    jpeg_buffer_reset(tn_img_info.p_fragments[0].color.yuv.luma_buf);
    jpeg_buffer_reset(tn_img_info.p_fragments[0].color.yuv.chroma_buf);

    rc = jpeg_buffer_use_external_buffer(
              tn_img_info.p_fragments[0].color.yuv.luma_buf,
              (uint8_t *)thumbnail_buf, buf_size,
              thumbnail_fd);

    if (rc == JPEGERR_EFAILED) {
      CDBG_ERROR("jpeg_buffer_use_external_buffer Thumbnail pmem failed...\n");
      pthread_mutex_unlock(&jpege_mutex);
      return FALSE;
    }

    cbcroffset = PAD_TO_WORD(tn_img_info.width * tn_img_info.height);
    if (hw_encode) {
      actual_size = dimension->thumbnail_width * dimension->thumbnail_height;
      padded_size = dimension->thumbnail_width *
        CEILING16(dimension->thumbnail_height);
      cbcroffset = padded_size;
   }

    // The chroma plane in YUV4:2:0 semiplanar is at the end of the luma plane,
    // so we attach the chroma buf to the luma buffer, which we've allocated to
    // be large enough to hold the entire YUV image.
    //
    jpeg_buffer_attach_existing(tn_img_info.p_fragments[0].color.yuv.chroma_buf,
      tn_img_info.p_fragments[0].color.yuv.luma_buf,
      cbcroffset);
    jpeg_buffer_set_actual_size(tn_img_info.p_fragments[0].color.yuv.luma_buf,
      tn_img_info.width * tn_img_info.height);
    jpeg_buffer_set_actual_size(
      tn_img_info.p_fragments[0].color.yuv.chroma_buf, tn_img_info.width *
      tn_img_info.height / 2);

    if (hw_encode && !zsl_enable) {
      if ((jpegRotation == 90) || (jpegRotation == 180)) {
        jpeg_buffer_set_start_offset(tn_img_info.p_fragments[0].color.yuv.luma_buf, (padded_size - actual_size));
        jpeg_buffer_set_start_offset(tn_img_info.p_fragments[0].color.yuv.chroma_buf, ((padded_size - actual_size) >> 1));
      }
    }
  }

  CDBG("jpeg_encoder_encode size %dx%d\n",dimension->orig_picture_dx,dimension->orig_picture_dy);
  main_img_info.width = dimension->orig_picture_dx * w_scale_factor;
  main_img_info.height = dimension->orig_picture_dy;
  buf_size = main_img_info.width * main_img_info.height * 2;
  main_img_info.fragment_cnt = 1;
  if (dimension->main_img_format == CAMERA_YUV_422_NV61) 
    main_img_info.color_format = YCRCBLP_H2V1;
  else
    main_img_info.color_format = YCRCBLP_H2V2;
  main_img_info.p_fragments[0].width = main_img_info.width;
  main_img_info.p_fragments[0].height = CEILING16(main_img_info.height);
  jpeg_buffer_reset(main_img_info.p_fragments[0].color.yuv.luma_buf);
  jpeg_buffer_reset(main_img_info.p_fragments[0].color.yuv.chroma_buf);

  rc =
    jpeg_buffer_use_external_buffer(
            main_img_info.p_fragments[0].color.yuv.luma_buf,
            (uint8_t *)snapshot_buf, buf_size,
            snapshot_fd);

  if (rc == JPEGERR_EFAILED) {
    CDBG("jpeg_buffer_use_external_buffer Snapshot pmem failed...\n");
    pthread_mutex_unlock(&jpege_mutex);
    return FALSE;
  }
  cbcroffset = PAD_TO_WORD(main_img_info.width * main_img_info.height);
  actual_size = 0;
  padded_size = 0;
  if (a_cbcroffset >= 0) {
    cbcroffset = a_cbcroffset;
  } else {
    if (hw_encode) {
      actual_size = dimension->orig_picture_dx * dimension->orig_picture_dy;
      padded_size = dimension->orig_picture_dx * CEILING16(dimension->orig_picture_dy);
      cbcroffset = padded_size;
    }
  }

  CDBG("jpeg_encoder_encode: cbcroffset %d",cbcroffset);
  jpeg_buffer_attach_existing(main_img_info.p_fragments[0].color.yuv.chroma_buf,
    main_img_info.p_fragments[0].color.yuv.luma_buf,
    cbcroffset);
  jpeg_buffer_set_actual_size(main_img_info.p_fragments[0].color.yuv.luma_buf, main_img_info.width * main_img_info.height);
  jpeg_buffer_set_actual_size(main_img_info.p_fragments[0].color.yuv.chroma_buf, main_img_info.width * main_img_info.height / 2);

  if (hw_encode && !zsl_enable) {
    if ((jpegRotation == 90) || (jpegRotation == 180)) {
      jpeg_buffer_set_start_offset(main_img_info.p_fragments[0].color.yuv.luma_buf, (padded_size - actual_size));
      jpeg_buffer_set_start_offset(main_img_info.p_fragments[0].color.yuv.chroma_buf, ((padded_size - actual_size) >> 1));
    }
  }

#if !defined(_TARGET_7x2x_) && !defined(_TARGET_7x27A_)
  jpeg_buffer_set_phy_offset(main_img_info.p_fragments[0].color.yuv.luma_buf, phy_offset);
#endif

  /*  Set Source */
  jpege_source.p_main = &main_img_info;
  if (usethumbnail) {
    jpege_source.p_thumbnail = &tn_img_info;
    CDBG("fragment_cnt: thumb %d \n", jpege_source.p_thumbnail->fragment_cnt);
  }

  CDBG("fragment_cnt: main %d \n", jpege_source.p_main->fragment_cnt);

  rc = jpege_set_source(jpeg_encoder, &jpege_source);
  if (rc) {
    CDBG("jpege_set_source failed: %d\n", rc);
    pthread_mutex_unlock(&jpege_mutex);
    return FALSE;
  }

#if defined(_TARGET_7x2x_) || defined(_TARGET_7x27A_)
  jpege_dest.p_output_handler = (jpege_output_handler_t) jpege_output_produced_handler;
#else
  jpege_dest.p_output_handler = jpege_output_produced_handler2;
#endif

  jpege_dest.buffer_cnt = 2;
  rc = jpege_set_destination(jpeg_encoder, &jpege_dest);
  if (rc) {
    CDBG("jpege_set_desination failed: %d\n", rc);
    pthread_mutex_unlock(&jpege_mutex);
    return FALSE;
  }
  /*  Get default configuration */
  jpege_config.thumbnail_present = usethumbnail;
  CDBG("%s: preference %d ", __func__, jpege_config.preference);
  jpege_config.main_cfg.quality = jpegMainimageQuality;
  jpege_config.thumbnail_cfg.quality = jpegThumbnailQuality;

  CDBG("Scaling params thumb in1_w %d in1_h %d out1_w %d out1_h %d "
       "main_img in2_w %d in2_h %d out2_w %d out2_h %d\n",
         scaling_params->in1_w, scaling_params->in1_h,
         scaling_params->out1_w, scaling_params->out1_h,
         scaling_params->in2_w, scaling_params->in2_h,
         scaling_params->out2_w, scaling_params->out2_h);

  if(scaling_params->in2_w && scaling_params->in2_h &&
     scaling_params->out2_w && scaling_params->out2_h) {

    if(jpegRotation)
      jpege_config.preference = JPEG_ENCODER_PREF_SOFTWARE_ONLY;

    /* Scaler information  for main image */
    jpege_config.main_cfg.scale_cfg.enable = TRUE;

    jpege_config.main_cfg.scale_cfg.input_width = CEILING2(scaling_params->in2_w);
    jpege_config.main_cfg.scale_cfg.input_height = CEILING2(scaling_params->in2_h);

    if (main_crop_offset) {
      jpege_config.main_cfg.scale_cfg.h_offset = main_crop_offset->x;
      jpege_config.main_cfg.scale_cfg.v_offset = main_crop_offset->y;
    } else {
      jpege_config.main_cfg.scale_cfg.h_offset = 0;
      jpege_config.main_cfg.scale_cfg.v_offset = 0;
    }

    jpege_config.main_cfg.scale_cfg.output_width = scaling_params->out2_w;
    jpege_config.main_cfg.scale_cfg.output_height = scaling_params->out2_h;
  } else {
    CDBG("There is no scaling information for JPEG main image scaling.");
  }

  if(scaling_params->in1_w  && scaling_params->in1_h &&
     scaling_params->out1_w && scaling_params->out1_h) {
    /* Scaler information  for thumbnail */
    jpege_config.thumbnail_cfg.scale_cfg.enable = TRUE;

    jpege_config.thumbnail_cfg.scale_cfg.input_width = CEILING2(scaling_params->in1_w);
    jpege_config.thumbnail_cfg.scale_cfg.input_height = CEILING2(scaling_params->in1_h);

    if (thumb_crop_offset) {
      jpege_config.thumbnail_cfg.scale_cfg.h_offset = thumb_crop_offset->x;
      jpege_config.thumbnail_cfg.scale_cfg.v_offset = thumb_crop_offset->y;
    } else {
      jpege_config.thumbnail_cfg.scale_cfg.h_offset = 0;
      jpege_config.thumbnail_cfg.scale_cfg.v_offset = 0;
    }

    jpege_config.thumbnail_cfg.scale_cfg.output_width = scaling_params->out1_w;
    jpege_config.thumbnail_cfg.scale_cfg.output_height = scaling_params->out1_h;
  } else {
    CDBG("There is no scaling information for JPEG thumbnail upscaling.");
  }

  /* Set rotation based on the mode selected */
  CDBG(" Setting Jpeg Rotation mode to %d ", jpegRotation );
  jpege_config.main_cfg.rotation_degree_clk = jpegRotation;
  jpege_config.thumbnail_cfg.rotation_degree_clk = jpegRotation;

  if( exif_data != NULL) {
    for(i = 0; i < exif_numEntries; i++) {
      rc = exif_set_tag(exif_info, exif_data[i].tag_id,
                         &(exif_data[i].tag_entry));
      if (rc) {
        CDBG("exif_set_tag failed: %d\n", rc);
        pthread_mutex_unlock(&jpege_mutex);
        return FALSE;
      }
    }
  }

#if 0 /* Enable when JPS/MPO is ready */
  /* 3D config */
  CDBG("%s: is_3dmode %d ", __func__, is_3dmode );
  if (is_3dmode) {
    jps_cfg_3d_t cfg_3d;
    if (jpege_config.main_cfg.scale_cfg.enable ||
      (jpege_config.main_cfg.rotation_degree_clk > 0)) {
      CDBG("%s: img_format_3d %d ", __func__, img_format_3d );
      return FALSE;
    }
    CDBG("%s: img_format_3d %d ", __func__, img_format_3d );

    switch (img_format_3d) {
    case TOP_DOWN_HALF:
      cfg_3d.layout = OVER_UNDER;
      cfg_3d.width_flag = FULL_WIDTH;
      cfg_3d.height_flag = HALF_HEIGHT;
      cfg_3d.field_order = LEFT_FIELD_FIRST;
      cfg_3d.separation = 0;
      break;
    case TOP_DOWN_FULL:
      cfg_3d.layout = OVER_UNDER;
      cfg_3d.width_flag = FULL_WIDTH;
      cfg_3d.height_flag = FULL_HEIGHT;
      cfg_3d.field_order = LEFT_FIELD_FIRST;
      cfg_3d.separation = 0;
      break;
    case SIDE_BY_SIDE_HALF:
      cfg_3d.layout = SIDE_BY_SIDE;
      cfg_3d.width_flag = HALF_WIDTH;
      cfg_3d.height_flag = FULL_HEIGHT;
      cfg_3d.field_order = LEFT_FIELD_FIRST;
      cfg_3d.separation = 0;
      break;
    default:
    case SIDE_BY_SIDE_FULL:
      cfg_3d.layout = SIDE_BY_SIDE;
      cfg_3d.width_flag = FULL_WIDTH;
      cfg_3d.height_flag = FULL_HEIGHT;
      cfg_3d.field_order = LEFT_FIELD_FIRST;
      cfg_3d.separation = 0;
      break;
    }

    rc = jpse_config_3d(jpeg_encoder, cfg_3d);
    if (rc) {
     CDBG_ERROR("%s: jpse_config_3d failed: %d\n", __func__, rc);
      pthread_mutex_unlock(&jpege_mutex);
      return FALSE;
    }
  }
#endif

  /*  Start encoder */
  if( jpege_config.main_cfg.scale_cfg.enable) {
    mmcamera_util_profile("SW encoder starting encoding");
  } else {
    mmcamera_util_profile("HW encoder starting encoding");
  }
  rc = jpege_start(jpeg_encoder, &jpege_config, &exif_info);
  if (rc) {
    CDBG("jpege_start failed: %d\n", rc);
    pthread_mutex_unlock(&jpege_mutex);
    return FALSE;
  }

  pthread_mutex_unlock(&jpege_mutex);
  return TRUE;
}

int8_t jpeg_encoder_setMainImageQuality(uint32_t quality)
{
  pthread_mutex_lock(&jpege_mutex);
  CDBG(" jpeg_encoder_setMainImageQuality current main inage quality %d ," \
       " new quality : %d\n", jpegMainimageQuality, quality);
  if (quality <= 100)
    jpegMainimageQuality = quality;
  pthread_mutex_unlock(&jpege_mutex);
  return TRUE;
}

int8_t jpeg_encoder_setThumbnailQuality(uint32_t quality)
{
  pthread_mutex_lock(&jpege_mutex);
  CDBG(" jpeg_encoder_setThumbnailQuality current thumbnail quality %d ," \
       " new quality : %d\n", jpegThumbnailQuality, quality);
  if (quality <= 100)
    jpegThumbnailQuality = quality;
  pthread_mutex_unlock(&jpege_mutex);
  return TRUE;
}

int8_t jpeg_encoder_setRotation(int rotation)
{
  pthread_mutex_lock(&jpege_mutex);
  /* Set rotation configuration */
  switch(rotation)
  {
      case 0:
      case 90:
      case 180:
      case 270:
          jpegRotation = rotation;
          break;
      default:
          /* Invalid rotation mode, set to default */
          CDBG(" Setting Default rotation mode ");
          jpegRotation = 0;
          break;
  }
  pthread_mutex_unlock(&jpege_mutex);
  return TRUE;
}

int8_t jpeg_encoder_hw_inline_config(void)
{
  CDBG("%s", __func__);
#ifdef USE_GEMINI
  gemini_config(MSM_GEMINI_MODE_REALTIME_ENCODE);
#endif
  return 0;
}

int8_t jpeg_encoder_hw_inline_wait_done(void)
{
   CDBG("%s", __func__);
#ifdef USE_GEMINI
   gemini_lib_inline_wait_done();
#endif
   return 0;
}
