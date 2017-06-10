/**********************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __HDR_LIB_H__
#define __HDR_LIB_H__

#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/*============================================================================
  MACRO DEFINITION
============================================================================*/

#if !defined ABS
  #define  ABS(x) ((x)>0 ? (x) : -(x))
#endif

#if !defined MAX
  #define  MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#endif

#if !defined MIN
  #define  MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif

#define FRAME_1 1
#define FRAME_2 2

typedef enum {
  HDR_NO_MEMORY = -2,
  HDR_ERROR,
  HDR_SUCESS,
} hdr_return_t;

/*Sub sample format*/
typedef enum {
  HDR_H2V2 = 0,
  HDR_H2V1 = 1,
  HDR_H1V2 = 2,
  HDR_H1V1 = 3,
  HDR_SUBSAMPLE_MAX
}subsample_format_type;

/*============================================================================
  Type DECLARATIONS
============================================================================*/

typedef enum {
  GAMMA_TBL_ALL = 0,
  GAMMA_TBL_R,
  GAMMA_TBL_G,
  GAMMA_TBL_B,
} hdr_gamma_tbl_type_t;

typedef struct {
  hdr_gamma_tbl_type_t gamma_t;
  uint32_t entry;
  void *gamma_tbl;
} hdr_gamma_table_struct_t;


typedef struct {
  uint32_t mCalculatedExposureRatioG;
  uint32_t *mpRedInverseGammatable;
  uint32_t *mpGreenInverseGammatable;
  uint32_t *mpBlueInverseGammatable;
  uint32_t *mpRedNewGammatable;
  uint32_t *mpGreenNewGammatable;
  uint32_t *mpBlueNewGammatable;
} hdr_param_t;

typedef enum {
  YCRCB,
  YCBCR
} hdr_chroma_order_t;

typedef enum {
  Y,
  CR,
  CB
} hdr_pixel_component_type_t;


typedef struct {
  void *buffer;
  uint32_t y_offset;
  uint32_t cbcr_offset;
  uint32_t output_flag;  /* tell algorithm to write back*/
} hdr_frame_type_t;

typedef struct {
  uint32_t *pH1;
  uint32_t *pH2;
  uint32_t *pH3;
  uint32_t *pH4;
  uint32_t *pH5;
  uint32_t *pH6;
  uint32_t *pH7;
  uint32_t *pH8;
  uint32_t *pV1;
  uint32_t *pV2;
  uint32_t *pV3;
  uint32_t *pV4;
  uint32_t *pV5;
  uint32_t *pV6;
  uint32_t *pV7;
  uint32_t *pV8;
}hdr_projection_t;

typedef struct {
  int32_t vert[49];
  int32_t hori[49];
}hdr_motion_vector_t;


typedef struct {
  //Longer Exposure Time for multiframe HDR OR
  //Normal frame for single frame hdr
  void  *pHdrBuffer1Y;
  void  *pHdrBuffer1C;
  void  *pHdrBuffer1R;
  void  *pHdrBuffer1G;
  void  *pHdrBuffer1B;
  //Shorter Exposure Time for multiframe HDR
  //Not used for single frame HDR
  void  *pHdrBuffer2Y;
  void  *pHdrBuffer2C;
  void  *pHdrBuffer2R;
  void  *pHdrBuffer2G;
  void  *pHdrBuffer2B;
  //Temp buffers circular
  void  *pHdrBuffer2RC;
  void  *pHdrBuffer2GC;
  void  *pHdrBuffer2BC;
  uint32_t imageWidth;
  uint32_t imageHeight;
  subsample_format_type subSampleFormat;
  uint32_t thumbMainIndicator;
  uint32_t maxLag;
  uint32_t calculatedExposureRatioG;
  uint32_t *pRedGammaInvTable;
  uint32_t *pGreenGammaInvTable;
  uint32_t *pBlueGammaInvTable;
  uint32_t *pRedGammaTable;
  uint32_t *pGreenGammaTable;
  uint32_t *pBlueGammaTable;
  uint8_t frameid;
  hdr_projection_t *pProjection;
  hdr_motion_vector_t *pCorrectedMv;
  uint32_t segmentOutputHeight;
  uint8_t *pMask;
  uint8_t *pMarker;
  void *pVoidHistogramArray;
  uint32_t startingLine;
  uint32_t endingLine;
  uint32_t startingRange;
  uint32_t endingRange;
  hdr_chroma_order_t chromaOrder;
  hdr_pixel_component_type_t pixelComponent;
  uint32_t contrastControl;
} hdr_config_t;


/*===========================================================================

Function           : hdrSingleFrameCore

Description        : Entry point for single frame HDR algorithm
1. Create chroma preprocess thread and run it in background
2. Calculate luma histogram
3. Intra Filter luma histogram, contrast control, and inter filter histogram
4. Wait for preprocess thread to join
5. Create and dispatch threads to perform adaptive equalization

Input parameter(s) : Pointer to the image configuration struture (pIn).
store Return status

Output parameter(s): pIn->pHdrBuffer1Y -> HDR output Y
pIn->pHdrBuffer1C -> HDR output CbCr
pReturnStatus containing return code

Return Value       : none

Side Effects       : None

=========================================================================== */

void hdrSingleFrameCore (hdr_config_t *pIn, hdr_return_t *pReturnStatus);

/*===========================================================================

Function           : hdrTwoFrameCore

Description        : Entry point for 2 frame HDR algorithm

Input parameter(s) : Pointer to the image configuration struture (pIn).
structure to store Return status (pReturnStatus)

Output parameter(s): pIn->pHdrBuffer1Y -> HDR output Y
pIn->pHdrBuffer1C -> HDR output CbCr
pReturnStatus containing return code

Return Value       : none

Side Effects       : None

=========================================================================== */

void hdrTwoFrameCore (hdr_config_t *pIn, hdr_return_t *pReturnStatus);


#endif //__HDR_LIB_H__

