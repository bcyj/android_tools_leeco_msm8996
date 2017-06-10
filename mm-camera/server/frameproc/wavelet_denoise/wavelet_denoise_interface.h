#ifndef WAVELET_DENOISE_INTERFACE_H
#define WAVELET_DENOISE_INTERFACE_H
/*======================================================================
Copyright (C) 2010-2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
====================================================================== */
/*============================================================================
                        INCLUDE FILES
============================================================================*/

#include <stdint.h>
//#include "frameproc.h"

/*============================================================================
  STRUCTURE DECLARATIONS
  ============================================================================*/

#define SEGMENT_LINE_SIZE 320
#define NUMBER_OF_SETS 6
#define CAMERA_WAVELET_DENOISING_DATA_SIZE 24
#define WAVELET_DENOISING_DATA_SIZE_BY_6 4

typedef enum
{
  DENOISE_H2V2,
  DENOISE_H2V1,
  DENOISE_H1V2,
  DENOISE_H1V1,
  DENOISE_SUBSAMPLE_MAX
}denoise_subsampling_t;

typedef enum
{
  DENOISE_SEMI_PLANAR,
  DENOISE_PLANAR,
  DENOISE_PLANE_MAX
}denoise_plane_t;

typedef enum
{
  DENOISE_YCBCR_PLANE,
  DENOISE_CBCR_ONLY,
  DENOISE_STREAMLINE_YCBCR,
  DENOISE_STREAMLINED_CBCR,
} denois_process_plane_t;

typedef struct
{
  /* This is the average noise value */
  uint32_t referencePatchAverageValue;
  /* This is the array of noise profile data */
  uint32_t  referenceNoiseProfileData[CAMERA_WAVELET_DENOISING_DATA_SIZE];
}CameraDenoisingType;

typedef enum
{
  IDLE = 0,
  ABORTING,
  DENOISE,
  STATE_MAX
} denoise_state_t;


/* denoise_lib_t type */
typedef struct denoise_lib_t
{
  uint32_t              width;
  uint32_t              height;
  denoise_subsampling_t subsampling;
  uint32_t              lines_to_pad;
  denoise_state_t       state;
  CameraDenoisingType* Y_noiseProfileData_Q20;
  CameraDenoisingType* Chroma_noiseProfileData_Q20;
  uint8_t* plumaplane;
  uint8_t* pchromaplane1;
  uint8_t* pchromaplane2;
  denoise_plane_t       denoiseplane;
  uint32_t              segmentLineSize; /*piece by piece segment line size */
  int                   process_planes;
  int                   y_complexity;
  int                   cbcr_complexity;
} denoise_lib_t;


/*=========================
===================================================
                      FUNCTION DECLARATIONS
====================================================================*/

/*===========================================================================

Function            : camera_denoising_manager

Description         : Top level camera denoising function

Input parameter(s)  : denoise_lib_t

Output parameter(s) : TRUE/FALSE

Return Value        : None

Side Effects        : None

=========================================================================== */

int camera_denoising_manager(denoise_lib_t* p_denoise);


#endif /* WAVELET_DENOISE_INTERFACE_H */
