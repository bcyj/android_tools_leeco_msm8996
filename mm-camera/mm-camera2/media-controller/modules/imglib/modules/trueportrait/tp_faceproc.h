/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#ifndef __IMGLIB_TEST_H__
#define __IMGLIB_TEST_H__

#include "img_common.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "faceproc.h"
#include "fd_chromatix.h"

/**
 * CONSTANTS and MACROS
 **/
#define MAX_TEST_FRAMES 10

/** imglib_data_t
 *   @frame: array of frames
 *   @addr: address of the frames allocated
 *   @mem_cnt: count of the buffers allocated
 *   @width: width of the frame
 *   @height: height of the frame
 *   @input_fn: input file name
 *   @out_fn: output file name
 *   @ss: subsampling type
 *   @in_count: input frame count
 *   @mutex: pointer to mutex variable
 *   @cond: pointer to condition variable
 *   @p_comp: pointer to the component ops
 *   @p_core_ops: pointer to the core ops
 *   @comp: component structure
 *   @core_ops: core ops structure
 *   @stride: stride of the image
 *   @scanline: scanline of the image
 *   @algo_index: algorithm index
 *
 *   Image test app structure
 **/
typedef struct {
  img_frame_t frame[MAX_TEST_FRAMES];
  uint8_t *addr[MAX_TEST_FRAMES];
  int mem_cnt;
  int width;
  int height;
  char *input_fn;
  char *out_fn;
  img_subsampling_t ss;
  int in_count;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  img_component_ops_t *p_comp;
  img_core_ops_t *p_core_ops;
  img_component_ops_t comp;
  img_core_ops_t core_ops;
  int stride;
  int scanline;
  int algo_index;
} imglib_data_t;

/** faceproc_tp_t
 *   @base: base test app structure
 *   @mode: Faceproc denoise mode
 *   @config: faceproc configuration
 *   @result: faceproc result
 *
 *   Faceproc test app structure
 **/
typedef struct {
  imglib_data_t *base;
  faceproc_mode_t mode;
  faceproc_config_t config;
  faceproc_result_t result;
  fd_chromatix_t *test_chromatix;
} faceproc_tp_t;

/**
 * Function: faceproc_test_execute
 *
 * Description: execute FaceProc test case
 *
 * Input parameters:
 *   p_faceproc - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/

int faceproc_test_execute(faceproc_tp_t *p_test);

#endif //__IMGLIB_TEST_H__
