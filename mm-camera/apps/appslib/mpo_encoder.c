/*============================================================================

   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <mpo_encoder.h>
#include "camera_dbg.h"
#include <sys/system_properties.h>
#include "mm_camera_interface.h"

#define JPEGE_FRAGMENT_SIZE (64*1024)

#ifdef MPO_DBG
  #ifdef _ANDROID_
    #undef CDBG
    #undef LOG_NIDEBUG
    #undef LOG_TAG
    #define LOG_NIDEBUG 0
    #define LOG_TAG "mm-camera mpo"
    #include <utils/Log.h>
    #define CDBG(fmt, args...) LOGE(fmt, ##args)
    #endif
#endif

#define MPO_DEFAULT_MAINIMAGE_QUALITY 75
#define MPO_DEFAULT_THUMBNAIL_QUALITY 75


static pthread_mutex_t mpoe_mutex = PTHREAD_MUTEX_INITIALIZER;
static int mpo_encoder_initialized;
static mpoe_obj_t encoder;
static mpoe_src_t source[2];
static mpoe_dst_t dest;
static mpoe_cfg_t config[2];
static mpoe_img_cfg_t tn_cfg, main_cfg[2];
static mpoe_img_data_t main_img_info[2], tn_img_info[2];
static uint8_t use_pmem;
static jpeg_buffer_t buffers[2];
static exif_info_obj_t exif_info[2];
static mpo_info_obj_t mpo_info;
static mpo_index_obj_t mpo_index;
static mpo_attribute_obj_t mpo_attribute[2];
static exif_tag_entry_t mpo_data;
static mpoe_app2_data_t  app2_data[2];
static int state;
static int jpegRotation;

void (*mmcamera_jpegfragment_callback)(uint8_t * buff_ptr, uint32_t buff_size);
void (*mmcamera_jpeg_callback)(jpeg_event_t);


void mpo_encoder_event_handler(void *p_user_data, jpeg_event_t event,
    void *p_arg) {
  uint32_t buf_size;
  uint8_t *buf_ptr = NULL;
  int mainimg_fd, thumbnail_fd;

  CDBG("jpege_event_handler is called with event %d!\n", event);
  if (event == JPEG_EVENT_DONE) {

    jpeg_buffer_t thumbnail_buffer, snapshot_buffer;

    snapshot_buffer = main_img_info[0].p_fragments[0].color.yuv.luma_buf;
    mainimg_fd = jpeg_buffer_get_pmem_fd(snapshot_buffer);
    jpeg_buffer_get_actual_size(snapshot_buffer, &buf_size);
    jpeg_buffer_get_addr(snapshot_buffer, &buf_ptr);

    snapshot_buffer = main_img_info[1].p_fragments[0].color.yuv.luma_buf;
    mainimg_fd = jpeg_buffer_get_pmem_fd(snapshot_buffer);
    jpeg_buffer_get_actual_size(snapshot_buffer, &buf_size);
    jpeg_buffer_get_addr(snapshot_buffer, &buf_ptr);

  }
  if (mmcamera_jpeg_callback)
    mmcamera_jpeg_callback(event);

}

int mpo_encoder_output_handler(void *p_user_data, void *p_arg,
    jpeg_buffer_t buffer, uint8_t last_buf_flag)
{
  uint32_t buf_size;
  uint8_t *buf_ptr;
  int rv;
  jpeg_buffer_get_actual_size(buffer, &buf_size);
  jpeg_buffer_get_addr(buffer, &buf_ptr);

  if (mmcamera_jpegfragment_callback)
    mmcamera_jpegfragment_callback(buf_ptr, buf_size);

  rv = jpeg_buffer_set_actual_size(buffer, 0);
  if (rv == JPEGERR_SUCCESS) {
    rv = mpoe_enqueue_output_buffer(encoder, &buffer, 1);
  }
  return rv;
}

void mpo_encoder_set_api_info(uint8_t* main_buf) {
  uint32_t buf_size;
  uint8_t *buf_ptr;
  int i = 0;
  int rc;

  // Open output file and update
  jpeg_buffer_init(&app2_data[0].data_buf);
  jpeg_buffer_init(&app2_data[1].data_buf);
  for (i = 0; i < 2; i++) {
    rc = jpeg_buffer_allocate(app2_data[i].data_buf, 1000, false);
  }

  // Get App2 header
  mpoe_get_app2_header(encoder, &app2_data[0], 2);

  // Update all App2 header
  for (i = 0; i < 2; i++) {
    jpeg_buffer_get_addr(app2_data[i].data_buf, &buf_ptr);
    jpeg_buffer_get_actual_size(app2_data[i].data_buf, &buf_size);
    memcpy(main_buf + app2_data[i].start_offset, buf_ptr, buf_size);
  }

}

void mpo_encoder_join(){
  int rc;
  int i;

  pthread_mutex_lock(&mpoe_mutex);
  if (mpo_encoder_initialized) {
    mpo_encoder_initialized = 0;
    pthread_mutex_destroy(&mpoe_mutex);
    mpoe_abort(encoder);

    // Clean up allocated data buffers
    jpeg_buffer_destroy(&app2_data[0].data_buf);
    jpeg_buffer_destroy(&app2_data[1].data_buf);

    rc = mpo_delete_index_tag(mpo_info, MPOTAGID_NUMBER_OF_IMAGES);
    rc = mpo_delete_index_tag(mpo_info, MPOTAGID_MP_ENTRY);
    rc = mpo_delete_attribute_tag(mpo_info, MPOTAGID_MP_INDIVIDUAL_NUM, 0);
    rc = mpo_delete_attribute_tag(mpo_info, MPOTAGID_MP_INDIVIDUAL_NUM, 1);

    // Destroy mpo info
    mpo_info_destroy(&(mpo_info));

    // Clean up allocated source buffers
    jpeg_buffer_destroy(&main_img_info[0].p_fragments[0].color.yuv.luma_buf);
    jpeg_buffer_destroy(&main_img_info[0].p_fragments[0].color.yuv.chroma_buf);
    jpeg_buffer_destroy(&main_img_info[1].p_fragments[0].color.yuv.luma_buf);
    jpeg_buffer_destroy(&main_img_info[1].p_fragments[0].color.yuv.chroma_buf);
    jpeg_buffer_destroy(&tn_img_info[0].p_fragments[0].color.yuv.luma_buf);
    jpeg_buffer_destroy(&tn_img_info[0].p_fragments[0].color.yuv.chroma_buf);
    jpeg_buffer_destroy(&tn_img_info[1].p_fragments[0].color.yuv.luma_buf);
    jpeg_buffer_destroy(&tn_img_info[1].p_fragments[0].color.yuv.chroma_buf);
    jpeg_buffer_destroy(&buffers[0]);
    jpeg_buffer_destroy(&buffers[1]);
    for (i = 0; i < 2; i++) {
      jpeg_buffer_destroy(&app2_data[i].data_buf);
    }
    exif_destroy(&exif_info[0]);
    exif_destroy(&exif_info[1]);
    mpoe_destroy(&encoder);
  }
  pthread_mutex_unlock(&mpoe_mutex);
  return;
}

#define SET_ZERO(variable) memset(&variable, 0, sizeof(variable))

int8_t mpo_encoder_init() {
  SET_ZERO(mpoe_mutex);
  SET_ZERO(encoder);
  SET_ZERO(source);
  SET_ZERO(dest);
  SET_ZERO(config);
  SET_ZERO(tn_cfg);
  SET_ZERO(main_cfg);
  SET_ZERO(main_img_info);
  SET_ZERO(tn_img_info);
  SET_ZERO(buffers);
  SET_ZERO(exif_info);
  SET_ZERO(mpo_info);
  SET_ZERO(mpo_index);
  SET_ZERO(mpo_attribute);
  SET_ZERO(mpo_data);
  SET_ZERO(app2_data);
  state = 0;

  pthread_mutex_lock(&mpoe_mutex);
  mpo_encoder_initialized = 1;
  pthread_mutex_unlock(&mpoe_mutex);
  return TRUE;
}

int8_t mpo_encoder_encode(const cam_ctrl_dimension_t * dimension,
        const uint8_t * thumbnail_buf, int thumbnail_fd,
        const uint8_t * snapshot_buf, int snapshot_fd,
        common_crop_t *scaling_params, exif_tags_info_t *exif_data,
        int exif_numEntries, const int32_t a_cbcroffset,
        cam_point_t* main_crop_offset, cam_point_t* thumb_crop_offset,
        int zsl_enable){


  int buf_size, cbcroffset;
  int hw_encode = false;
  int rc, i;

  pthread_mutex_lock(&mpoe_mutex);

  rc = mpoe_init(&encoder, &mpo_encoder_event_handler, (void *) NULL);
  if (JPEG_FAILED(rc)) {
    CDBG_ERROR("encoder_test: mpoe_init failed\n");
    pthread_mutex_unlock(&mpoe_mutex);
    return FALSE;
  }
  rc = mpoe_get_default_config(&config[0]);
  rc = mpoe_get_default_config(&config[1]);

  config[0].preference = MPO_ENCODER_PREF_SOFTWARE_ONLY;
  config[1].preference = MPO_ENCODER_PREF_SOFTWARE_ONLY;

  if((rc = jpeg_buffer_init(&main_img_info[0].
        p_fragments[0].color.yuv.luma_buf)) ||
      (rc = jpeg_buffer_init(&main_img_info[0].
        p_fragments[0].color.yuv.chroma_buf)) ||
      (rc = jpeg_buffer_init(&main_img_info[1].
        p_fragments[0].color.yuv.luma_buf)) ||
      (rc = jpeg_buffer_init(&main_img_info[1].
        p_fragments[0].color.yuv.chroma_buf)) ||
      (rc = jpeg_buffer_init(&tn_img_info[0].
        p_fragments[0].color.yuv.luma_buf)) ||
      (rc = jpeg_buffer_init(&tn_img_info[0].
        p_fragments[0].color.yuv.chroma_buf)) ||
      (rc = jpeg_buffer_init(&tn_img_info[1].
        p_fragments[0].color.yuv.luma_buf)) ||
      (rc = jpeg_buffer_init(&tn_img_info[1].
        p_fragments[0].color.yuv.chroma_buf)) ||
      (rc = jpeg_buffer_init(&buffers[0])) ||
      (rc = jpeg_buffer_init(&buffers[1]))) {

    CDBG_ERROR("jpeg_buffer_init failed: %d\n", rc);
    pthread_mutex_unlock(&mpoe_mutex);
    dest.buffer_cnt = 0;
    return FALSE;
  }

  // Set source information (main)
  main_img_info[0].color_format           = YCRCBLP_H2V2;
  main_img_info[0].width                  = dimension->orig_picture_dx;
  main_img_info[0].height                 = dimension->orig_picture_dy;
  main_img_info[0].fragment_cnt           = 1;
  main_img_info[0].p_fragments[0].width   = dimension->orig_picture_dx;
  main_img_info[0].p_fragments[0].height  = dimension->orig_picture_dy;

  //TODO change to use get buffer offset kind method
  buf_size = dimension->orig_picture_dx * dimension->orig_picture_dy * 2;
  //set the source buffers
  rc = jpeg_buffer_use_external_buffer(
      main_img_info[0].p_fragments[0].color.yuv.luma_buf,
      (uint8_t *) snapshot_buf, buf_size, snapshot_fd);


  if (rc == JPEGERR_EFAILED) {
    CDBG_ERROR("jpeg_buffer_use_external_buffer Snapshot pmem failed...\n");
    pthread_mutex_unlock(&mpoe_mutex);
    return FALSE;
  }

  cbcroffset = PAD_TO_WORD(main_img_info[0].width *
      main_img_info[0].height);

  jpeg_buffer_attach_existing(
      main_img_info[0].p_fragments[0].color.yuv.chroma_buf,
      main_img_info[0].p_fragments[0].color.yuv.luma_buf, cbcroffset);
  jpeg_buffer_set_actual_size(
      main_img_info[0].p_fragments[0].color.yuv.luma_buf,
      main_img_info[0].width * main_img_info[0].height);
  jpeg_buffer_set_actual_size(
      main_img_info[0].p_fragments[0].color.yuv.chroma_buf,
      main_img_info[0].width * main_img_info[0].height / 2);



  tn_img_info[0].color_format = YCRCBLP_H2V2;
  tn_img_info[0].width = dimension->orig_picture_dx;
  tn_img_info[0].height = dimension->orig_picture_dy;
  tn_img_info[0].fragment_cnt = 1;
  tn_img_info[0].p_fragments[0].width = dimension->orig_picture_dx;
  tn_img_info[0].p_fragments[0].height = dimension->orig_picture_dy;

  //TODO change to use get buffer offset kind method
  buf_size = dimension->orig_picture_dx * dimension->orig_picture_dy * 2;
  //set the source buffers
  rc = jpeg_buffer_use_external_buffer(
      tn_img_info[0].p_fragments[0].color.yuv.luma_buf,
      (uint8_t *) thumbnail_buf, buf_size, thumbnail_fd);

  if (rc == JPEGERR_EFAILED) {
    CDBG_ERROR("jpeg_buffer_use_external_buffer Snapshot pmem failed...\n");
    pthread_mutex_unlock(&mpoe_mutex);
    return FALSE;
  }

  cbcroffset = PAD_TO_WORD(tn_img_info[0].width *
      tn_img_info[0].height);

  jpeg_buffer_attach_existing(
      tn_img_info[0].p_fragments[0].color.yuv.chroma_buf,
      tn_img_info[0].p_fragments[0].color.yuv.luma_buf, cbcroffset);
  jpeg_buffer_set_actual_size(tn_img_info[0].p_fragments[0].color.yuv.luma_buf,
      tn_img_info[0].width * tn_img_info[0].height);
  jpeg_buffer_set_actual_size(
      tn_img_info[0].p_fragments[0].color.yuv.chroma_buf,
      tn_img_info[0].width * tn_img_info[0].height / 2);


  //setting scaling scaling
  config[0].main_cfg.rotation_degree_clk = 0;
  config[0].main_cfg.scale_cfg.enable = true;
  config[0].main_cfg.scale_cfg.h_offset = 0;
  config[0].main_cfg.scale_cfg.v_offset = 0;
  config[0].main_cfg.scale_cfg.input_width = main_img_info[0].width / 2;
  config[0].main_cfg.scale_cfg.input_height = main_img_info[0].height;
  config[0].main_cfg.scale_cfg.output_width = main_img_info[0].width / 2;
  config[0].main_cfg.scale_cfg.output_height = main_img_info[0].height;

  //setting scaling scaling
  config[0].thumbnail_cfg.rotation_degree_clk = 0;
  config[0].thumbnail_cfg.scale_cfg.enable = true;
  config[0].thumbnail_cfg.scale_cfg.h_offset = 0;
  config[0].thumbnail_cfg.scale_cfg.v_offset = 0;
  config[0].thumbnail_cfg.scale_cfg.input_width = tn_img_info[0].width / 2;
  config[0].thumbnail_cfg.scale_cfg.input_height = tn_img_info[0].height;
  config[0].thumbnail_cfg.scale_cfg.output_width = scaling_params->out1_w;
  config[0].thumbnail_cfg.scale_cfg.output_height = scaling_params->out1_h;
  config[0].thumbnail_present = TRUE;

  source[0].p_main = &main_img_info[0];
  source[0].p_thumbnail = &tn_img_info[0];
  source[0].image_attribute.type = MULTI_VIEW_DISPARITY;


  // Set source information (main)
  main_img_info[1].color_format = YCRCBLP_H2V2;
  main_img_info[1].width = dimension->orig_picture_dx;
  main_img_info[1].height = dimension->orig_picture_dy;
  main_img_info[1].fragment_cnt = 1;
  main_img_info[1].p_fragments[0].width = dimension->orig_picture_dx;
  main_img_info[1].p_fragments[0].height = dimension->orig_picture_dy;

  //TODO change to use get buffer offset kind method
  buf_size = dimension->orig_picture_dx * dimension->orig_picture_dy * 2;
  //set the source buffers
  rc = jpeg_buffer_use_external_buffer(
      main_img_info[1].p_fragments[0].color.yuv.luma_buf,
      (uint8_t *) snapshot_buf, buf_size, snapshot_fd);

  if (rc == JPEGERR_EFAILED) {
    CDBG_ERROR("jpeg_buffer_use_external_buffer Snapshot pmem failed...\n");
    pthread_mutex_unlock(&mpoe_mutex);
    return FALSE;
  }

  cbcroffset = PAD_TO_WORD(main_img_info[1].width *
      main_img_info[1].height);
  jpeg_buffer_attach_existing(
      main_img_info[1].p_fragments[0].color.yuv.chroma_buf,
      main_img_info[1].p_fragments[0].color.yuv.luma_buf, cbcroffset);
  jpeg_buffer_set_actual_size(
      main_img_info[1].p_fragments[0].color.yuv.luma_buf,
      main_img_info[1].width * main_img_info[1].height);
  jpeg_buffer_set_actual_size(
      main_img_info[1].p_fragments[0].color.yuv.chroma_buf,
      main_img_info[1].width * main_img_info[1].height / 2);


  tn_img_info[1].color_format = YCRCBLP_H2V2;
  tn_img_info[1].width = dimension->orig_picture_dx;
  tn_img_info[1].height = dimension->orig_picture_dy;
  tn_img_info[1].fragment_cnt = 1;
  tn_img_info[1].p_fragments[0].width = dimension->orig_picture_dx;
  tn_img_info[1].p_fragments[0].height = dimension->orig_picture_dy;

  //TODO change to use get buffer offset kind method
  buf_size = dimension->orig_picture_dx * dimension->orig_picture_dy * 2;
  //set the source buffers
  rc = jpeg_buffer_use_external_buffer(
      tn_img_info[1].p_fragments[0].color.yuv.luma_buf,
      (uint8_t *) thumbnail_buf, buf_size, thumbnail_fd);


  if (rc == JPEGERR_EFAILED) {
    CDBG_ERROR("jpeg_buffer_use_external_buffer Snapshot pmem failed...\n");
    pthread_mutex_unlock(&mpoe_mutex);
    return FALSE;
  }

  cbcroffset = PAD_TO_WORD(tn_img_info[1].width *
      tn_img_info[1].height);

  jpeg_buffer_attach_existing(
      tn_img_info[1].p_fragments[0].color.yuv.chroma_buf,
      tn_img_info[1].p_fragments[0].color.yuv.luma_buf, cbcroffset);
  jpeg_buffer_set_actual_size(tn_img_info[1].p_fragments[0].color.yuv.luma_buf,
      tn_img_info[1].width * tn_img_info[1].height);
  jpeg_buffer_set_actual_size(
      tn_img_info[1].p_fragments[0].color.yuv.chroma_buf,
      tn_img_info[1].width * tn_img_info[1].height / 2);


  //setting scaling scaling
  config[1].main_cfg.rotation_degree_clk = 0;
  config[1].main_cfg.scale_cfg.enable = true;
  config[1].main_cfg.scale_cfg.h_offset = main_img_info[1].width/2;
  config[1].main_cfg.scale_cfg.v_offset = 0;
  config[1].main_cfg.scale_cfg.input_width = main_img_info[1].width/2;
  config[1].main_cfg.scale_cfg.input_height = main_img_info[1].height;
  config[1].main_cfg.scale_cfg.output_width = main_img_info[1].width/2;
  config[1].main_cfg.scale_cfg.output_height = main_img_info[1].height;

  //setting scaling scaling
  config[1].thumbnail_cfg.rotation_degree_clk = 0;
  config[1].thumbnail_cfg.scale_cfg.enable = true;
  config[1].thumbnail_cfg.scale_cfg.h_offset = tn_img_info[1].width/2;
  config[1].thumbnail_cfg.scale_cfg.v_offset = 0;
  config[1].thumbnail_cfg.scale_cfg.input_width = tn_img_info[1].width/2;
  config[1].thumbnail_cfg.scale_cfg.input_height = tn_img_info[1].height;
  config[1].thumbnail_cfg.scale_cfg.output_width = scaling_params->out1_w;

  config[1].thumbnail_cfg.scale_cfg.output_height = scaling_params->out1_h;
  config[1].thumbnail_present = TRUE;

  source[1].p_main = &main_img_info[1];
  source[1].p_thumbnail = &tn_img_info[1];
  source[1].image_attribute.type = MULTI_VIEW_DISPARITY;

  rc = mpoe_set_source(encoder, &source[0], 2);

  //destination
  dest.buffer_cnt = 1;
  dest.p_buffer = &buffers[0];
  /*  Allocate 2 ping-pong buffers on the heap for jpeg encoder outputs */
  //TODO make it ping pong
  if ((rc = jpeg_buffer_allocate(buffers[0], JPEGE_FRAGMENT_SIZE, 0)) ||
      (rc = jpeg_buffer_allocate(buffers[1], JPEGE_FRAGMENT_SIZE, 0))) {
    CDBG_ERROR("jpeg_buffer_allocate failed: %d\n", rc);
    pthread_mutex_unlock(&mpoe_mutex);
    return FALSE;
  }

  if (JPEG_FAILED(rc)) {
    CDBG_ERROR("encoder_test: mpoe_set_source failed\n");
    pthread_mutex_unlock(&mpoe_mutex);
    return FALSE;
  }

  dest.p_output_handler = mpo_encoder_output_handler;

  rc = mpoe_set_destination(encoder, &dest);
  if (JPEG_FAILED(rc)) {
    CDBG_ERROR("encoder_test: mpoe_set_destination failed\n");
    pthread_mutex_unlock(&mpoe_mutex);
    return FALSE;
  }

  uint8_t default_value = 0;


  rc = mpo_info_init(&(mpo_info));
  mpo_data.type = EXIF_LONG;
  mpo_data.count = 1;
  mpo_data.data._long = 2;
  rc = mpo_set_index_tag(mpo_info, MPOTAGID_NUMBER_OF_IMAGES, &mpo_data);
  if (JPEG_FAILED(rc)) {
    CDBG_ERROR("encoder_test: mpo_set_tag failed\n");
    return false;
  }

  mpo_data.type = EXIF_UNDEFINED;
  mpo_data.count = 16 * 2;
  mpo_data.data._undefined = &default_value;
  rc = mpo_set_index_tag(mpo_info, MPOTAGID_MP_ENTRY, &mpo_data);
  if (JPEG_FAILED(rc)) {
    CDBG_ERROR("encoder_test: mpo_set_tag failed\n");
    return false;
  }

  mpo_data.type = EXIF_LONG;
  mpo_data.count = 1;
  mpo_data.data._long = 1;
  rc = mpo_set_attribute_tag(mpo_info, MPOTAGID_MP_INDIVIDUAL_NUM, &mpo_data,
          0);
  if (JPEG_FAILED(rc)) {
    CDBG_ERROR("encoder_test: mpo_set_tag failed\n");
    return false;
  }

  mpo_data.type = EXIF_LONG;
  mpo_data.count = 1;
  mpo_data.data._long = 2;
  rc = mpo_set_attribute_tag(mpo_info, MPOTAGID_MP_INDIVIDUAL_NUM, &mpo_data,
          1);
  if (JPEG_FAILED(rc)) {
    CDBG_ERROR("encoder_test: mpo_set_tag failed\n");
    return false;
  }


  if((rc = exif_init(&exif_info[0])) ||
    (rc = exif_init(&exif_info[1])) ){
    CDBG_ERROR("exif_init failed ");
    return false;
  }

  if( exif_data != NULL) {
    for (i = 0; i < exif_numEntries; i++) {
      rc = exif_set_tag(exif_info[0], exif_data[i].tag_id,
        &(exif_data[i].tag_entry));
      rc = exif_set_tag(exif_info[1], exif_data[i].tag_id,
        &(exif_data[i].tag_entry));
      if (rc) {
        CDBG_ERROR("exif_set_tag failed: %d\n", rc);
        pthread_mutex_unlock(&mpoe_mutex);
        return FALSE;
      }
    }
  }

  rc = mpoe_start(encoder, &config[0], &exif_info[0], &mpo_info, 2);
  if (JPEG_FAILED(rc)){
    CDBG_ERROR("encoder_test: mpoe_start failed\n");
    pthread_mutex_unlock(&mpoe_mutex);
    return false;
  }

  pthread_mutex_unlock(&mpoe_mutex);
  return TRUE;
}


