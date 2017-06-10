#ifndef WAVELET_DENOISE_H
#define WAVELET_DENOISE_H
/*======================================================================
Copyright (C) 2010-2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
====================================================================== */

/*============================================================================
                        INCLUDE FILES
============================================================================*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "camera.h"
#include "camera_dbg.h"

#if(FRAME_PROC_DENOISE_DEBUG)
#ifdef _ANDROID_
#include <utils/Log.h>
#endif
#undef LOG_NIDEBUG
#undef LOG_TAG
#define LOG_NIDEBUG 0
#define LOG_TAG "mm-camera-DENOISE"
#define CDBG_DENOISE(fmt, args...) LOGE(fmt, ##args)
#else
#define CDBG_DENOISE(fmt, args...) do{}while(0)
#endif

/*============================================================================
  DEFINITIONS and CONSTANTS
  ============================================================================*/

#define GAMMA_TABLE_ENTRIES 64

#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif
#ifndef min
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif
#define NUMBER_OF_LEVELS 4
#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define TRUE   1   /* Boolean true value. */
#define FALSE  0   /* Boolean false value. */

#define  ON   1    /* On value. */
#define  OFF  0    /* Off value. */

#ifndef NULL
#define NULL  0
#endif

#define CLAMP255(x) ( ((x) & 0xFFFFFF00) ? ((~(x) >> 31) & 0xFF) : (x))

#define parameter_q15_factor 15
#define parameter_q15_roundoff (1 << (parameter_q15_factor-1))

#define parameter_q20_roundoff (1<< (20-1))
#define parameter_q21_roundoff (1<< (21-1))

#undef Q14
#define Q14  14
#define Q15  15
#define Q16  16
#define Q20 20

#define Q14_ROUNDOFF  (1 << (Q14 - 1))
#define Q15_ROUNDOFF  (1 << (Q15 - 1))
#define Q16_ROUNDOFF  (1 << (Q16 - 1))

#define Q14_OPT(x)  (((x) + Q14_ROUNDOFF) >> Q14)
#define Q15_OPT(x)  (((x) + Q15_ROUNDOFF) >> Q15)

/*============================================================================
  STRUCTURE DECLARATIONS
  ============================================================================*/

typedef struct
{
  uint32_t table_size;
  uint32_t entry;
  uint16_t* gamma_tbl;
} wavelet_gamma_table_struct;

typedef struct denoise_input_t
{
  double D_new;
  short int current_gamma[GAMMA_TABLE_ENTRIES];
  chromatix_parms_type* chromatix;
  int gamma_table_size;
} denoise_input_t;

/* denoise object type */
typedef struct denoise_t
{
  uint32_t              width;
  uint32_t              height;
  int                   subsampling;
  uint32_t              lines_to_pad;
  uint32_t              src_handle;
  float                 denoise_scale_Y;
  float                 denoise_scale_chroma;
  uint8_t               gain_trigger;
  denoise_state_t       state;
  CameraDenoisingType* Y_noiseProfileData_Q20;
  CameraDenoisingType* Chroma_noiseProfileData_Q20;
  struct msm_pp_frame* frame;
  uint32_t              segmentLineSize; /*piece by piece segment line size */
  int process_planes;
  int y_complexity;
  int cbcr_complexity;
  denoise_input_t       input;
} denoise_t;


/*=========================
===================================================
                      FUNCTION DECLARATIONS
====================================================================*/

/*===========================================================================
 * FUNCTION    - wavelet_denoise_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int wavelet_denoise_set_params(frame_proc_t* frameCtrl, frame_proc_set_wd_data_t* data);

/*===========================================================================

FUNCTION:  wavelet_denoise_calibrate

RETURN VALUE:
 1 - success
  0 - failure
  ============================================================================*/
int wavelet_denoise_calibrate(void* Ctrl, denoise_t* wdCtrl);


/*===========================================================================

FUNCTION:  wavelet_denoise_process

RETURN VALUE:
 1 - success
 0 - failure
============================================================================*/

int32_t wavelet_denoise_process(void* Ctrl, denoise_t* wdCtrl);


/*===========================================================================
 * FUNCTION    - wavelet_denoise_execute -
 *
 * DESCRIPTION:
 *==========================================================================*/
int wavelet_denoise_execute(frame_proc_t* frameCtrl);

/*===========================================================================
 * FUNCTION    - wavelet_denoise_exit -
 *
 * DESCRIPTION:
 *==========================================================================*/
int wavelet_denoise_exit(frame_proc_t* frameCtrl);

/*===========================================================================

FUNCTION:  wavelet_denoise_init

RETURN VALUE:
 1 - success
 0 - failure
============================================================================*/

int wavelet_denoise_init(frame_proc_t* frameCtrl);

/*===========================================================================
FUNCTION:  interpolate_noise_profile
============================================================================*/
void interpolate_noise_profile(ReferenceNoiseProfile_type* noise_profile,
                               int patch_index, double calibration_level,
                               denoise_t* wdCtrl, double gain);


/*===========================================================================

FUNCTION:  wavelet_denoise_calibrate

RETURN VALUE:
 1 - success
  0 - failure
  ============================================================================*/
int wavelet_denoise_calibrate(void* Ctrl, denoise_t* wdCtrl);


/*===========================================================================

FUNCTION:  wavelet_denoise_process

RETURN VALUE:
 1 - success
 0 - failure
============================================================================*/
int32_t wavelet_denoise_process(void* Ctrl, denoise_t* wdCtrl);

#endif /* WAVELET_DENOISE_H */
