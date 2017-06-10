/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#ifndef QOMX_JPEG_ENC_TEST_H
#define QOMX_JPEG_ENC_TEST_H

#include <sys/types.h>
#include <fcntl.h>
#include <linux/android_pmem.h>
#include <linux/ioctl.h>
#include <stdio.h>
#include <stdlib.h>

/*KHRONOS header files*/
#include "OMX_Types.h"
#include "OMX_Index.h"
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "QOMX_JpegExtensions.h"
#include "buffer_test.h"
#include "qomx_core.h"
#include "qexif.h"

/**
 *  MACROS and CONSTANTS
 **/
#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define PAD_TO_WORD(a)(((a)+3)&~3)

#define MAX_EXIF_ENTRIES 10
#define MAX_INSTANCES 3
#define MAX_TEST_BUFFERS 5
#define MAX_FN_LEN 256


/** omx_test_image_t:
 *  @file_name: image filename
 *  @width: image width
 *  @height: image height
 *  @quality: image quality
 *  @eColorFormat: openmax color format
 *  @chroma_wt: factor to be multiplied for the chroma buffer
 *
 *  Represents the image class
 **/
typedef struct {
  char *file_name;
  uint32_t width;
  uint32_t height;
  uint32_t quality;
  int eColorFormat;
  float chroma_wt;
} omx_test_image_t;

/** omx_test_scale_cfg_t:
 *  @input_width: input width
 *  @input_height: input height
 *  @h_offset: left cordinate
 *  @v_offset: top cordinate
 *  @output_width: output width
 *  @output_height: output height
 *  @enable: flag to represent if scale is enabled
 *
 *  Represents the image scale configuration
 **/
typedef struct {
  uint32_t input_width;
  uint32_t input_height;
  uint32_t h_offset;
  uint32_t v_offset;
  uint32_t output_width;
  uint32_t output_height;
  uint8_t enable;
} omx_test_scale_cfg_t;

/** omx_enc_test_args_t:
 *  @main: main image info
 *  @thumbnail: thumbnail info
 *  @output_file: output filenmae
 *  @rotation: image rotation
 *  @encode_thumbnail: flag to indicate if thumbnail needs to be
 *                   encoded
 *  @abort_time: abort time
 *  @use_pmem: flag to indicate if ion needs to be used
 *  @main_scale_cfg: main image scale config
 *  @tn_scale_cfg: thumbnail scale config
 *  @instance_cnt: encoder instance count
 *  @burst_count: burst capture count
 *
 *  Represents the test arguments passed from the user
 **/
typedef struct {
  omx_test_image_t main;
  omx_test_image_t thumbnail;
  char *output_file;
  int16_t rotation;
  uint8_t  encode_thumbnail;
  uint32_t abort_time;
  uint8_t use_pmem;
  omx_test_scale_cfg_t main_scale_cfg;
  omx_test_scale_cfg_t tn_scale_cfg;
  int instance_cnt;
  int burst_count;
} omx_enc_test_args_t;

/** omx_enc_test_args_t:
 *  @main: main image info
 *  @thumbnail: thumbnail info
 *  @output_file: output filename
 *  @rotation: image rotation
 *  @encode_thumbnail: flag to indicate if thumbnail needs to be
 *                   encoded
 *  @abort_time: abort time
 *  @use_pmem: flag to indicate if ion needs to be used
 *  @main_scale_cfg: main image scale config
 *  @tn_scale_cfg: thumbnail scale config
 *  @instance_cnt: encoder instance count
 *  @frame_info: YUV input frame extension
 *  @callbacks: OMX callbacks
 *  @encoding: flag to indicate if encoding is in progress
 *  @usePadding: check if image is padded
 *  @lock: mutex variable
 *  @cond: condition variable
 *  @thread_id: thread id
 *  @p_in_buffers: input buffer header
 *  @p_out_buffers: output buffer header
 *  @in_buffer: input buffer
 *  @out_buffer: output buffer
 *  @buf_count: number of captures. (for burst mode)
 *  @ebd_count: EBD count
 *  @fbd_count: FBD count
 *  @p_handle: OMX handle
 *  @inputPort: input port
 *  @outputPort: output port
 *  @total_size: total size of the buffer
 *  @error_flag: check if error is occured
 *  @state_change_pending: flag to indicate of state change is
 *                       pending
 *  @last_error: last error
 *
 *  Represents the encoder test object
 **/
typedef struct {
  omx_test_image_t main;
  omx_test_image_t thumbnail;
  char output_file[MAX_TEST_BUFFERS][MAX_FN_LEN];
  int16_t rotation;
  uint8_t  encode_thumbnail;
  uint32_t abort_time;
  uint8_t use_pmem;
  omx_test_scale_cfg_t main_scale_cfg;
  omx_test_scale_cfg_t tn_scale_cfg;

  QOMX_YUV_FRAME_INFO frame_info;
  OMX_CALLBACKTYPE callbacks;

  int encoding;
  int usePadding;
  pthread_mutex_t lock;
  pthread_cond_t cond;
  pthread_t thread_id;

  OMX_BUFFERHEADERTYPE *p_in_buffers[MAX_TEST_BUFFERS];
  OMX_BUFFERHEADERTYPE *p_thumb_buf[MAX_TEST_BUFFERS];
  OMX_BUFFERHEADERTYPE *p_out_buffers[MAX_TEST_BUFFERS];
  buffer_test_t in_buffer[MAX_TEST_BUFFERS];
  buffer_test_t out_buffer[MAX_TEST_BUFFERS];
  int buf_count;
  int ebd_count;
  int total_ebd_count;
  int fbd_count;

  OMX_HANDLETYPE p_handle;
  OMX_PARAM_PORTDEFINITIONTYPE *inputPort;
  OMX_PARAM_PORTDEFINITIONTYPE *outputPort;
  OMX_PARAM_PORTDEFINITIONTYPE *thumbPort;
  OMX_U32 total_size;
  OMX_BOOL error_flag;
  OMX_BOOL state_change_pending;
  OMX_ERRORTYPE last_error;
  uint64_t encode_time;
  OMX_BOOL aborted;
  pthread_t abort_thread_id;
  pthread_mutex_t abort_mutx;
  pthread_cond_t  abort_cond;
  OMX_STATETYPE  omx_state;
} omx_enc_test_t;

#endif //QOMX_JPEG_ENC_TEST_H
