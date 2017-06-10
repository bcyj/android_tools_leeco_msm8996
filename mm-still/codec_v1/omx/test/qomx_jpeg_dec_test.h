/*******************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#ifndef QOMX_JPEG_DEC_TEST_H
#define QOMX_JPEG_DEC_TEST_H

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
#define MAX_TEST_BUFFERS 1
#define MAX_FN_LEN 256
#define MAX_DECODE_B2B_COUNT 1

typedef enum {
  QOMX_DEC_STATE_IDLE,
  QOMX_DEC_STATE_ACTIVE,
  QOMX_DEC_STATE_RECONFIG_OUT_PORT,
} qomx_dec_state_t;

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

/** omx_dec_test_args_t:
 *  @in_file_name: input filename
 *  @eColorFormat: output color format
 *  @width: image width
 *  @height: image height
 *  @output_file: output filename
 *  @rotation: image rotation
 *  @abort_time: abort time
 *  @use_pmem: flag to indicate whether to use pmem buffers
 *  @scale_cfg: image scale config
 *  @instance_cnt: decoder instance count
 *  @b2b_count: back to back decoding count
 *  @scale_factor: scale factor
 *
 *  Represents the test arguments passed from the user
 **/
typedef struct {
  char *in_file_name;
  int eColorFormat;
  uint32_t width;
  uint32_t height;
  char *output_file;
  int16_t rotation;
  uint32_t abort_time;
  uint8_t use_pmem;
  omx_test_scale_cfg_t scale_cfg;
  int instance_cnt;
  int b2b_count;
  float scale_factor;
} omx_dec_test_args_t;

/** omx_dec_test_t:
 *  @in_file_name: input filename
 *  @eColorFormat: output color format
 *  @output_file: output filename
 *  @rotation: image rotation
 *  @abort_time: abort time
 *  @use_pmem: flag to indicate whether to use pmem buffers
 *  @scale_cfg: image scale config
 *  @b2b_count: back 2 back count
 *  @scale_factor: scale factor
 *  @callbacks: OMX callbacks
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
 *  @state: decoding client state
 *  @p_handle: OMX handle
 *  @inputPort: input port
 *  @outputPort: output port
 *  @total_size: total size of the buffer
 *  @error_flag: check if error is occured
 *  @state_change_pending: flag to indicate of state change is
 *                       pending
 *  @last_error: last error
 *
 *  Represents the decoder test object
 **/
typedef struct {
  char *in_file_name;
  int eColorFormat;
  uint32_t width;
  uint32_t height;
  char output_file[MAX_TEST_BUFFERS][MAX_FN_LEN];
  int16_t rotation;
  uint32_t abort_time;
  uint8_t use_pmem;
  omx_test_scale_cfg_t scale_cfg;
  OMX_CALLBACKTYPE callbacks;
  float scale_factor;
  int b2b_count;

  pthread_mutex_t lock;
  pthread_cond_t cond;
  pthread_t thread_id;

  OMX_BUFFERHEADERTYPE *p_in_buffers[MAX_TEST_BUFFERS];
  OMX_BUFFERHEADERTYPE *p_out_buffers[MAX_TEST_BUFFERS];
  buffer_test_t in_buffer[MAX_TEST_BUFFERS];
  buffer_test_t out_buffer[MAX_TEST_BUFFERS];
  int buf_count;
  int ebd_count;
  int fbd_count;
  qomx_dec_state_t state;

  OMX_HANDLETYPE p_handle;
  OMX_PARAM_PORTDEFINITIONTYPE *inputPort;
  OMX_PARAM_PORTDEFINITIONTYPE *outputPort;
  OMX_BOOL error_flag;
  OMX_U32 pending_event_type;
  OMX_BOOL event_pending;
  OMX_ERRORTYPE last_error;
  uint64_t decode_time;
} omx_dec_test_t;

#endif //QOMX_JPEG_DEC_TEST_H
