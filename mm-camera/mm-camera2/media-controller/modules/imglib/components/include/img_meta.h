/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#ifndef __IMG_META_H__
#define __IMG_META_H__

#include <unistd.h>
#include <math.h>
#include "img_common.h"

#define MAX_FACES_SUPPORTED_BY_TP 5
/**
 * CONSTANTS and MACROS
 **/

/** img_aec_info_t
*   @real_gain: real gain of the frame
*   @linecount: line count for the frame
*   @exp_time: exposure time
*   @lux_idx: lux index of the frame
*
*    AEC info
**/
typedef struct {
  float real_gain;
  uint32_t linecount;
  float exp_time;
  float lux_idx;
} img_aec_info_t;

/** img_awb_info_t
*   @r_gain: Red channel gain
*   @g_gain: green channel gain
*   @b_gain: Blue channel gain
*   @color_temp: Color temperature
*
*    AWB info
**/
typedef struct {
  float r_gain;
  float g_gain;
  float b_gain;
  uint32_t color_temp;
} img_awb_info_t;

/** img_af_info_t
*
*    AF info
**/
typedef struct {
  uint32_t dummy;
} img_af_info_t;

/** img_meta_type_t
*    IMG_META_R_GAMMA: R gamma table
*    IMG_META_G_GAMMA: G gamma table
*    IMG_META_B_GAMMA: B gamma table
*    IMG_META_AEC_INFO: AEC info
*    IMG_META_AWB_INFO: AWB info
*    IMG_META_AF_INFO: AF info
*    IMG_META_OUTPUT_ROI: Output ROI
*    IMG_META_ZOOM_FACTOR: zoom factor
*    IMG_META_OUTPUT_DIM: output dimension
*
*    Meta type
**/
typedef enum {
  IMG_META_R_GAMMA,
  IMG_META_G_GAMMA,
  IMG_META_B_GAMMA,
  IMG_META_AEC_INFO,
  IMG_META_AWB_INFO,
  IMG_META_AF_INFO,
  IMG_META_OUTPUT_ROI,
  IMG_META_ZOOM_FACTOR,
  IMG_META_OUTPUT_DIM,
  IMG_META_MAX,
} img_meta_type_t;

/** img_fd_info_t
*   @faceROIx: face start poistion x
*   @faceROIy: face start poistion y
*   @faceROIWidth: face width
*   @faceROIHeight: face height
*
*    face detection info
**/
typedef struct {
  uint32_t faceROIx[MAX_FACES_SUPPORTED_BY_TP];
  uint32_t faceROIy[MAX_FACES_SUPPORTED_BY_TP];
  uint32_t faceROIWidth[MAX_FACES_SUPPORTED_BY_TP];
  uint32_t faceROIHeight[MAX_FACES_SUPPORTED_BY_TP];
} img_fd_info_t;

/** img_meta_t
*    @valid: check if meta is valid
*    @gamma_R: R gamma table
*    @gamma_G: G gamma table
*    @gamma_B: B gamma table
*    @aec_info: AEC information
*    @awb_info: AWB information
*    @af_info: AF information
*    @output_crop: output crop info
*    @zoom_factor: zoom factor 1x-6x
*    @output_dim: output dimension
*    @valid_faces_detected: number of valid faces for Tp
*    @rotation: rotation requested for snapshot frame
*    @fd_frame_dim: frame dimension on which fd is applied
*
*    Imaging metadata structure
**/
typedef struct {
  int valid[IMG_META_MAX];

  img_gamma_t gamma_R;
  img_gamma_t gamma_G;
  img_gamma_t gamma_B;
  img_aec_info_t aec_info;
  img_awb_info_t awb_info;
  img_af_info_t af_info;
  img_rect_t output_crop;
  float zoom_factor;
  img_size_t output_dim;
  uint8_t valid_faces_detected;
  img_fd_info_t fd_info;
  int32_t rotation;
  img_size_t fd_frame_dim;
} img_meta_t;


/** img_frame_bundle_t
 *   @p_input: input frame pointers
 *   @p_output: output frame pointers
 *   @p_meta: meta pointer
 *
 *   Imaging frame bundle
 **/
typedef struct {
  img_frame_t *p_input[IMG_MAX_INPUT_FRAME];
  img_frame_t *p_output[IMG_MAX_OUTPUT_FRAME];
  img_meta_t *p_meta[IMG_MAX_META_FRAME];
} img_frame_bundle_t;

/** img_msg_type_t
 *   @IMG_MSG_BUNDLE: frame bundle
 *   @IMG_MSG_FRAME: image frame
 *   @IMG_MSG_META: meta frame
 *
 *   Imaging message type
 **/
typedef enum {
  IMG_MSG_BUNDLE,
  IMG_MSG_FRAME,
  IMG_MSG_META,
  IMG_MSG_NONE,
} img_msg_type_t;

/** img_msg_t
 *   @p_input: input frame pointers
 *   @p_output: output frame pointers
 *   @p_meta: meta pointer
 *
 *   Imaging message information
 **/
typedef struct {
  img_msg_type_t type;
  union {
    img_frame_bundle_t bundle;
    img_frame_t *p_frame;
    img_meta_t *p_meta;
  };
} img_msg_t;

/**
 * Function: img_get_meta
 *
 * Description: This macro is to get the meta value from
 *               metadata if present
 *
 * Arguments:
 *   @p_meta : meta buffer
 *   @type: meta type
 *
 * Return values:
 *     meta buffer value pointer
 *
 * Notes: none
 **/
void *img_get_meta(img_meta_t *p_meta, img_meta_type_t type);

#endif //__IMG_META_H__
