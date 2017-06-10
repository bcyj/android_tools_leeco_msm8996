/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/

#ifndef __WAVELET_DENOISE_LIB_H__
#define __WAVELET_DENOISE_LIB_H__

/**
 * CONSTANTS and MACROS
 **/
#define NUMBER_OF_LEVELS 4

 /* Lines required for padding (2*(2^n)-2)
  * for number of levels < 4 we add 64 lines to make it integer number of MCU
  * row for number of levels> 4 we add 2*2^n which is multiple of MCU rows*/
#define LINES_TO_PAD \
  ((NUMBER_OF_LEVELS < 4) ? 64 : (2 * (1 << (NUMBER_OF_LEVELS + 1))))


#define MAX_LEVEL 4

#define FILTER_LENGTH 5
#define FILTER_HALF_LENGTH 2

#define CLAMP255(x) ( ((x) & 0xFFFFFF00) ? ((~(x) >> 31) & 0xFF) : (x))

#define parameter_q15_factor 15
#define parameter_q15_roundoff (1 << (parameter_q15_factor-1))

#define WEIGHT_LUMA 6554 // (uint32_t)(0.2*(1<<parameter_q15_factor)+0.5)
#define WEIGHT_CHROMA 0  // (uint32_t)(0.0*(1<<parameter_q15_factor)+0.5)

#define parameter_q20_roundoff (1<< (20-1))
#define parameter_q21_roundoff (1<< (21-1))
#define Q14 14
#define Q15 15
#define Q16 16
#define Q20 20

#define Q14_ROUNDOFF  (1 << (Q14 - 1))
#define Q15_ROUNDOFF  (1 << (Q15 - 1))
#define Q16_ROUNDOFF  (1 << (Q16 - 1))

#define Q14_OPT(x)  (((x) + Q14_ROUNDOFF) >> Q14)
#define Q15_OPT(x)  (((x) + Q15_ROUNDOFF) >> Q15)

#define SEGMENT_LINE_SIZE 320
#define WAVELET_DENOISING_DATA_SIZE_BY_6 4
#define NUM_NOISE_PROFILE_DATA 24

#define MAX_SEGMENTS 8
#define MAX_SAMPLES  3

typedef struct
{
  /* This is the average noise value */
  uint32_t referencePatchAverageValue;
  /* This is the array of noise profile data */
  uint32_t  referenceNoiseProfileData[NUM_NOISE_PROFILE_DATA];
  /* Fine tuning flag , set it to true if the below fine tuning parameters are available in chromatix, otherwise set it to false*/
  /* The lib will use default parameters if this flag is false */
  uint32_t fineTuningFlag;
  /* This is the array of luma edge weight, Read luma_weight[level]  from chromatix, and do (uint32_t)(luma_weight[i]*(1<<15)+0.5)  for each level */
  uint32_t lumaEdgeWeight[WAVELET_DENOISING_DATA_SIZE_BY_6];
  /* This is the array of luma edge limit factor, Read denoise_edge_softness_y[level] from chromatix and do 2 * denoise_edge_softness_y[i] for each level*/
  uint32_t lumaEdgeLimitFactor[WAVELET_DENOISING_DATA_SIZE_BY_6];
  /* This is the array of chroma edge weight, Read chroma_weight[level]  from chromatix, and do (uint32_t)(chroma_weight[i]*(1<<15)+0.5)  for each level */
  uint32_t chromaEdgeWeight[WAVELET_DENOISING_DATA_SIZE_BY_6];
  /* This is the array of chroma edge limit factor, Read denoise_edge_softness_chroma[level] from chromatix and do 2 * denoise_edge_softness_chroma[i] for each level */
  uint32_t chromaEdgeLimitFactor[WAVELET_DENOISING_DATA_SIZE_BY_6];
  /* This is luma epsilon threshold, epsilon_y  from chromatix ,if chromatix is floating like 0.8, then do (uint32_t)(255 * 0.8  + 0.5)?*/
  uint32_t lumaEpsilonThreshold;
  /* This is chroma  epsilon threshold, epsilon_chroma from chromatix, if chromatix is floating like 0.8, then do (uint32_t)(255 * 0.8  + 0.5)?*/
  uint32_t chromaEpsilonThreshold;
} CameraDenoisingType;


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

typedef enum
{
  IDLE = 0,
  ABORTING,
  DENOISE,
  STATE_MAX
} denoise_state_t;

/* ION mapped buffer type */
typedef struct {
  uint8_t *vAddr;
  size_t size;
} denoise_mapped_buffer_t;

/* ION mapped memory type */
typedef struct {
  uint32_t useGPU;
  uint32_t useDSP;
  denoise_mapped_buffer_t gpuBuf;
  denoise_mapped_buffer_t dspBuf;
  denoise_mapped_buffer_t dspheapBuf;
  denoise_mapped_buffer_t dspnoiseprofileBuf;
  denoise_mapped_buffer_t arm[3];
  denoise_mapped_buffer_t chroma_interleave;
} denoise_mapped_memory_t;

//Core type
typedef enum
{
  CORE_DSP,
  CORE_GPU,
  CORE_ARM,
  CORE_ANY
}
core_type_t;

/* denoise_alloc_prop_item_t type */
typedef struct denoise_alloc_prop_item_t
{
  size_t                bufferSize;
  size_t                heapSize;
  uint32_t              enabled;
} denoise_alloc_prop_item_t;

/* denoise_alloc_prop_t type */
typedef struct denoise_alloc_prop_t
{
  denoise_alloc_prop_item_t dsp;
  denoise_alloc_prop_item_t gpu;
  uint32_t                  smmuOnly;
} denoise_alloc_prop_t;

/** notify_early_cb+ *
 *  Callback function to indicate WNR progress+ **/
 typedef int (*notify_cb_t) (void *handle);

/* denoise_lib_t type */
typedef struct denoise_lib_t
{
  uint32_t              Y_width;
  uint32_t              Y_height;
  uint32_t              Chroma_width;
  uint32_t              Chroma_height;
  uint32_t              lines_to_pad;
  denoise_state_t       state;
  CameraDenoisingType* noiseProfileData_Q20;
  uint8_t* plumaplane;
  uint8_t* pchromaplane1;
  uint8_t* pchromaplane2;
  denoise_plane_t       denoiseplane;
  uint32_t              LumaSegmentLineSize;   /* piece by piece Y segment line size */
  uint32_t              ChromaSegmentLineSize; /* piece by piece Cb Cr segment line size */
  int                   process_planes;
  int                   y_complexity;
  int                   cbcr_complexity;
  denoise_mapped_memory_t ion;
  core_type_t           core_aff[MAX_SAMPLES][MAX_SEGMENTS];
  uint8_t early_cb_enabled;
  void* app_data;
  notify_cb_t notify_early_cb;
} denoise_lib_t;

/*===========================================================================

Function            : camera_denoising_get_alloc_prop

Description         : Top level camera denoising get properties function

Input parameter(s)  : pdenoise, pprop

Output parameter(s) : TRUE/FALSE

Return Value        : None

Side Effects        : None

=========================================================================== */
int camera_denoising_get_alloc_prop(const denoise_lib_t *pdenoise,
  denoise_alloc_prop_t* pprop);

/*===========================================================================

Function            : camera_denoising_manager

Description         : Top level camera denoising function

Input parameter(s)  : denoise_lib_t

Output parameter(s) : TRUE/FALSE

Return Value        : None

Side Effects        : None

=========================================================================== */
int camera_denoising_manager(denoise_lib_t* p_denoise);

#endif //__WAVELET_DENOISE_LIB_H__

