/**********************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/


#ifndef __HDR_CHROMATIX_H__
#define __HDR_CHROMATIX_H__

/** hdr_chromatix_t
*   @enable: flag to enable hdr
*   @hdr_contrast_control: contrast_control_Q4
*   @hdr_chromaSat_wgt : Chroma desaturation control overall
*   @hdr_chromaSat_clamp :chroma desaturation in bright parts
*   @hdr_chromaSat_shift : chroma desaturation in dark parts
*
*   hdr chromatix header
**/
typedef struct {
  int8_t enable;
  /* HDR tuning parameters*/
  uint32_t hdr_contrast_control;
  /*0: Disable Chroma DeSaturation control overall,
  10: Max chroma de-saturation control. Range 0 to 10*/
  uint8_t hdr_chromaSat_wgt;
  /*0: Disable Chroma Saturation control in bright parts,
  40: Smaller the value, more the chroma de-saturation in Bright parts
  100: Bigger the value, lesser the chroma de-saturation in Bright parts*/
  uint8_t hdr_chromaSat_clamp;
  /*0: Disable Chroma Saturation control in dark parts,
  0: Smaller the value, lesser the chroma de-saturation in darker parts
  30: Bigger the value, more the chroma de-saturation in darker parts*/
  uint8_t hdr_chromaSat_shift;
}hdr_chromatix_t;

#endif //__HDR_CHROMATIX_H__
