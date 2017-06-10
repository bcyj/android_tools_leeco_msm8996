/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/

#ifndef __WD_COMP_H__
#define __WD_COMP_H__

#include "denoise.h"
#include "img_comp_priv.h"
#include "chromatix.h"
#include "wavelet_denoise_lib.h"
#include "sw_wnr_chromatix.h"

/** wd_comp_t
 *   @b: base image component
 *   @lines_to_pad: number of lines to pad the image
 *   @p_noiseprof: pointer to the noise profile
 *   @mode: wavelet denoise mode
 *   @y_complexity: luma complexity
 *   @cbcr_complexity: chroma complexity
 *   @gamma_set: flag to indicate whether the gamma is set
 *   @lowlight_gamma_set: flag to indicate whether the low light
 *                      gamma is set
 *   @lowlight_gamma: lowlight gamma table
 *   @current_gamma: gamma table of the current frame
 *   @info_3a: 3A info passed by the client
 *   @p_chromatix: chromatix pointer passed by the client
 *   @p_sw_wnr_chromatix: Sw wnr specific chromatix tuning
 *     Contain affinity table and segment dividers.
 *   @mapiondsp: DSP ION buffer mapping data (buffer)
 *   @mapiondspheap: DSP ION buffer mapping data (heap)
 *   @mapiondspnoiseprofile: DSP ION buffer mapping data (noise profile)
 *   @dspInitFlag: Is DSP ION buffer initialized
 *   @mapiongpu: GPU ION buffer mapping data
 *   @gpuInitFlag: Is GPU ION buffer initialized
 *   @denoiseCtrl: denoise library control structure
 *
 *   @early_cb_enabled: Indicates if early callback enabled for
 *                    burst mode
 *   Wavelet denoise component structure
 **/
typedef struct {
  /*base component*/
  img_component_t b;
  CameraDenoisingType *p_noiseprof;
  wd_mode_t mode;
  uint8_t gamma_set;
  uint8_t lowlight_gamma_set;
  img_gamma_t lowlight_gamma;
  img_gamma_t current_gamma;
  wd_3a_info_t info_3a;
  chromatix_parms_type *p_chromatix;
  sw_wnr_chromatix_t *p_sw_wnr_chromatix;
  img_mmap_info_ion mapiondsp;
  img_mmap_info_ion mapiondspheap;
  img_mmap_info_ion mapiondspnoiseprofile;
  uint32_t dspInitFlag;
  img_mmap_info_ion mapiongpu;
  uint32_t gpuInitFlag;
  uint8_t early_cb_enabled;
  denoise_lib_t denoiseCtrl;
} wd_comp_t;

#endif //__WD_COMP_H__

