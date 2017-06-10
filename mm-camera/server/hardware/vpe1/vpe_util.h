/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef mm_vpe_util_h
#define mm_vpe_util_h

/* This file serves the interface for the low layer VPE code to
   kernel. */

#include "vpe_reg_pack.h"
#include "vpe_proc.h"

#define MM_VPE_SCALE_COEFF_NUM     32

/*--------------------------------------------*/
/* This part is the interface to the kernel.  */
/*--------------------------------------------*/
/*
 * VPE_input_plane_config_type
 *
 * Define the parameters for input plane
 */

/*
 for now give src_image_width = stride plane0, 1  (same)
 how does it determine over-fetched or repeated?
 */

typedef struct {
  mm_vpe_src_format_packed     src_format;
  uint32_t                     src_unpack_pattern1;
  mm_vpe_src_image_size_packed src_buffer_size;
  mm_vpe_src_ystride1_packed   src_ystride1;
  mm_vpe_src_size_packed       src_roi_size;
  mm_vpe_src_xy_packed         src_roi_offset;
} mm_vpe_input_plane_config_type;

/*
 * VPE_output_plane_config_type
 *
 * Define the parameters for output plane
 */
typedef struct {
  mm_vpe_out_format_packed    out_format;
  uint32_t                    out_pack_pattern1;
  mm_vpe_out_ystride1_packed  out_ystride1;
  mm_vpe_out_size_packed      out_roi_size;
  mm_vpe_out_xy_packed        out_roi_offset;
} mm_vpe_output_plane_config_type;

enum {
  MM_VPE_SCALE_PT2TOPT4,
  MM_VPE_SCALE_PT4TOPT6,
  MM_VPE_SCALE_PT6TOPT8,
  MM_VPE_SCALE_PT8TO8,
  MM_VPE_SCALE_MAX,
} MM_VPE_SCALE_ENUM;

typedef struct {
  uint32_t   offset;
  int32_t    coef[MM_VPE_SCALE_COEFF_NUM*2];
} mm_vpe_scale_coef_cfg_type;


#define MM_VPE_RAW_INPUT_MAX_WIDTH          1408
#define MM_VPE_RAW_INPUT_MAX_HEIGHT         792
#define MM_VPE_FRAME_MAX_WIDTH              1280
#define MM_VPE_FRAME_MAX_HEIGHT             720
#define MM_VPE_FRAME_MIN_WIDTH              32
#define MM_VPE_FRAME_MIN_HEIGHT             32
/*
 * color_format_type
 *
*/
typedef enum {
  MM_COLOR_FORMAT_Y_CRCB_NV12_COSITE, /*<4:2:0, Pseudo planar, Cr in MSB, NV12*/
  MM_COLOR_FORMAT_Y_CRCB_NV12_OFFSITE,/*<4:2:0, Pseudo planar, Cr in MSB, NV12*/
  MM_COLOR_FORMAT_Y_CBCR_NV21_COSITE, /*<4:2:0, Pseudo planar, Cb in MSB, NV21*/
  MM_COLOR_FORMAT_Y_CBCR_NV21_OFFSITE,/*<4:2:0, Pseudo planar, Cb in MSB, NV21*/
} MM_COLOR_FORMAT_ENUM;

/*
 * frame_dimension_type
 * Note for destination frame offset will most likely to be 0.
 * Src offset x/y pair are used.
 */
typedef struct {
  uint32_t frame_offset_x;   /**< offset_x of ROI, in pixel */
  uint32_t frame_offset_y;   /**< offset_y of ROI, in pixel */
  uint32_t frame_width;      /**< Width of ROI, in pixel */
  uint32_t frame_height;     /**< Height of ROI, in pixel */
} mm_frame_dimension_type;

/*
 * buffer_dimension
 */
typedef struct {   /* stride is in the unit of bytes. */
  uint32_t stride_0; /* y plane */
  uint32_t stride_1; /* cbcr plane */
  uint32_t stride_2;
} mm_buffer_dimension_type;

/*
 * vpe_surface_attr_type
 *
 * Defines the attributes for memory which will be used as a VPE
 * frame buffer.
*/
typedef struct {
  MM_COLOR_FORMAT_ENUM     color_format;
  mm_buffer_dimension_type buffer_dimension;
  mm_frame_dimension_type  frame_dimension;
} mm_vpe_surface_attr_type;

/*
 * SCALE_POLYPHASE_RANGE_ENUM
 *
 */
typedef enum {
  MM_VPE_SCALE_P2_TO_P4 = 0, /*< Down-scale: 0.25x ~ 0.4x   >D0 */
  MM_VPE_SCALE_P4_TO_P6,     /*< Down-scale: 0.4x ~ 0.6x    >D1 */
  MM_VPE_SCALE_P6_TO_P8,     /*< Down-scale: 0.6x ~ 0.8x    >D2 */
  MM_VPE_SCALE_P8_TO_20      /*< Down-scale: 0.8x ~ 1.0x, Up-scale: 1.0x ~ 20.0x > U1 */
} MM_SCALE_POLYPHASE_RANGE_ENUM;

/*
 * VPE_ScaleConfigType
 *
 * Define the scaling configuration.
 */
typedef struct {
  uint32_t                       bEnPolyPhaseX;     /* Else M/N phase-cntl. */
  uint32_t                       bEnPolyPhaseY;
  uint32_t                       scale_phase_step_x;
  uint32_t                       scale_phase_step_y;
  uint32_t                       scale_phase_init_x;
  uint32_t                       scale_phase_init_y;
  MM_SCALE_POLYPHASE_RANGE_ENUM     ePolyPhaseRangeX;  /* N/A for M/N. */
  MM_SCALE_POLYPHASE_RANGE_ENUM     ePolyPhaseRangeY;
} mm_vpe_scale_config_type;

/*
 * VPE_ScaleAttrType
 *
 * Define the parameters for scaling.
 */
typedef struct {
  int16_t *piCoeffC0;    /* Phase K-1, 10-bit signed. */
  int16_t *piCoeffC1;    /* Phase K,   10-bit signed. */
  int16_t *piCoeffC2;    /* Phase K+1, 10-bit signed. */
  int16_t *piCoeffC3;    /* Phase K+2, 10-bit signed. */
} mm_vpe_scale_polyphase_filter_type;

/*===========================================================================
                           FUNCTION DECLARATIONS
==========================================================================*/
/* MRR-WARN */
void mm_vpe_init_scale_table(mm_vpe_scale_coef_cfg_type*  pvpecmd,
  MM_VPE_SCALE_TABLE_ENUM index);

int mm_vpe_util_sendcmd(int fd, void *pCmdData, unsigned int messageSize, int cmd_id);

#endif /* vpe_api_h */
