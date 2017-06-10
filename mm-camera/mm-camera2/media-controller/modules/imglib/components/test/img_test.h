/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __IMGLIB_TEST_H__
#define __IMGLIB_TEST_H__

#include "img_common.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "hdr.h"
#include "denoise.h"
#include "faceproc.h"
#include "cac.h"
#include "fd_chromatix.h"

/**
 * CONSTANTS and MACROS
 **/
#define MAX_FILENAME_LEN 256
#define MAX_TEST_FRAMES 10

/** imglib_test_t
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
} imglib_test_t;

/** frameproc_test_t
 *   @base: base test app structure
 *
 *   Frameproc test app structure
 **/
typedef struct {
  imglib_test_t *base;
} frameproc_test_t;

/** hdr_test_t
 *   @base: base test app structure
 *   @mode: HDR mode
 *   @analyse: flag to indicate if the analysis is supported
 *   @gamma: gamma table
 *   @out_index: index of the output buffer
 *
 *   HDR test app structure
 **/
typedef struct {
  imglib_test_t *base;
  hdr_mode_t mode;
  int analyse;
  img_gamma_t gamma;
  int out_index;
} hdr_test_t;

/** denoise_test_t
 *   @base: base test app structure
 *   @mode: Wavelet denoise mode
 *   @gamma: gamma table
 *   @low_gamma: lowlight gamma table
 *   @info_3a: 3A information
 *
 *   Denoise test app structure
 **/
typedef struct {
  imglib_test_t *base;
  wd_mode_t mode;
  img_gamma_t gamma;
  img_gamma_t low_gamma;
  wd_3a_info_t info_3a;
} denoise_test_t;

/** cac_test_t
 *   @base: base test app structure
 *   @r_gamma: R - gamma table- 1024 entires
 *   @g_gamma: G - gamma table- 1024 entires
 *   @b_gamma: B - gamma table -1024 entries
 *   @chromatix_info: Chromatix Info
 *   @chroma_order - CBCR or CRCB
 *
 *   CAC test app structure
 **/
typedef struct {
  imglib_test_t *base;
  img_gamma_t r_gamma;
  img_gamma_t g_gamma;
  img_gamma_t b_gamma;
  cac_chromatix_info_t chromatix_info;
  cac_3a_info_t info_3a;
  cac_chroma_order chroma_order;
} cac_test_t;

/** faceproc_test_t
 *   @base: base test app structure
 *   @mode: Faceproc denoise mode
 *   @config: faceproc configuration
 *   @result: faceproc result
 *
 *   Faceproc test app structure
 **/
typedef struct {
  imglib_test_t *base;
  faceproc_mode_t mode;
  faceproc_config_t config;
  faceproc_result_t result;
  fd_chromatix_t *test_chromatix;
} faceproc_test_t;


/** img_test_init
 *   @p_test: pointer to the imagelib testapp
 *
 *   Imagelib test app initialization
 **/
int img_test_init(imglib_test_t *p_test);

/** img_test_fill_buffer
 *   @p_test: pointer to the imagelib testapp
 *   @index: index of the buffer to be filled
 *   @analysis: indicates if the buffer is for analysis
 *
 *   Fill the buffers for imglib test application
 **/
int img_test_fill_buffer(imglib_test_t *p_test, int index, int analysis);

/** img_test_read
 *   @p_test: pointer to the imagelib testapp
 *   @filename: name of the file from which the buffer should be
 *            read
 *   @index: index of the buffer
 *
 *   Read the image and fill the buffer for imglib test
 *   application
 **/
int img_test_read(imglib_test_t *p_test, char *filename, int index);

/** img_test_write
 *   @p_test: pointer to the imagelib testapp
 *   @filename: name of the file from which the image should be
 *            written
 *   @index: index of the buffer
 *
 *   Read the image and fill the buffer for imglib test
 *   application
 **/
int img_test_write(imglib_test_t *p_test, char *filename, int index);

/** hdr_test_execute
 *   @p_test: pointer to the imagelib testapp
 *
 *   Executes the HDR test application
 **/
int hdr_test_execute(hdr_test_t *p_test);

/** denoise_test_execute
 *   @p_test: pointer to the imagelib testapp
 *
 *   Executes the denoise test application
 **/
int denoise_test_execute(denoise_test_t *p_test);

/** faceproc_test_execute
 *   @p_test: pointer to the imagelib testapp
 *
 *   Executes the faceproc test application
 **/
int faceproc_test_execute(faceproc_test_t *p_test);

/** cac_test_execute
 *   @p_test: pointer to the imagelib testapp
 *
 *   Executes the faceproc test application
 **/
int cac_test_execute(cac_test_t *p_test);



#endif //__IMGLIB_TEST_H__
