/**********************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __HDR_H__
#define __HDR_H__

/**
 * CONSTANTS and MACROS
 **/

#ifdef HDR_LIB_GHOSTBUSTER
#define MAX_HDR_FRAMES 3
#define HDR_COMP_LIB_NAME "libmmcamera_hdr_gb_lib.so"
#else
#define MAX_HDR_FRAMES 2
#define HDR_COMP_LIB_NAME "libmmcamera_hdr_lib.so"
#endif

/** hdr_mode_t
 *   SINGLE_FRAME: single frame HDR mode
 *   MULTI_FRAME: multiframe HDR mode
 *
 *   HDR modes
 **/
typedef enum {
  SINGLE_FRAME,
  MULTI_FRAME,
} hdr_mode_t;

/** hdr_crop_t
 *    @start_x: start X
 *    @start_y: start Y
 *    @width: width
 *    @height: height
 *
 *   HDR modes
 **/
typedef struct {
  unsigned int start_x;
  unsigned int start_y;
  unsigned int width;
  unsigned int height;
} hdr_crop_t;

/** QHDR_GAMMA_TABLE
 *
 *   datastructure passed needs to be of type img_gamma_t
 **/
#define QHDR_GAMMA_TABLE     (QIMG_HDR_PARAM_OFF +  1)

/** QHDR_ANALYZE_IMAGE
 *
 *   datastructure passed needs to be of type int
 **/
#define QHDR_ANALYZE_IMAGE   (QIMG_HDR_PARAM_OFF +  2)

/** QHDR_MODE
 *
 *   datastructure passed needs to be of type hdr_mode_t
 **/
#define QHDR_MODE            (QIMG_HDR_PARAM_OFF +  3)

/** QHDR_OUT_INDEX
 *
 *   datastructure passed needs to be of type int
 **/
#define QHDR_OUT_INDEX       (QIMG_HDR_PARAM_OFF +  4)

/** QHDR_OUT_CROP
 *
 *   datastructure passed needs to be of type hdr_crop_t
 **/
#define QHDR_OUT_CROP        (QIMG_HDR_PARAM_OFF +  5)

/** QHDR_HDR_CHROMATIX
 *
 *   datastructure passed needs to be of type fd_chromatix_hdr_t
 **/
#define QHDR_HDR_CHROMATIX   (QIMG_HDR_PARAM_OFF +  6)

#endif //__HDR_H__
