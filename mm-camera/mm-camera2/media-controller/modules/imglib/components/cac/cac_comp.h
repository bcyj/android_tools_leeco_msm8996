/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __CAC_COMP_H__
#define __CAC_COMP_H__

#include "cac.h"
#include "img_comp_priv.h"
#include "chromatix.h"
#include "cac_interface.h"


/** cac_comp_t
 *   @b: base component
 *   @r_gamma: The R gamma table
 *   @g_gamma: The G gamma table
 *   @b_gamma: The B gamma table
 *   @chromatix_info: chromatix info from the chromatix header
 *   @info_3a: Awb gain info
 *   @chroma_order : CBCR or CRCB
 *   @p_y_buffer: Y buffer ptr
 *   @p_c_buffer: Chroma buffer ptr
 *   @mode : Indicates if the component is executed in
 *       syncronous or asynchronous mode
 **/
typedef struct {
  /*base component*/
  img_component_t b;
  img_gamma_t r_gamma;
  img_gamma_t g_gamma;
  img_gamma_t b_gamma;
  cac_chromatix_info_t chromatix_info;
  cac_3a_info_t info_3a;
  cac_chroma_order chroma_order;
  uint8_t *p_y_buffer;
  uint8_t *p_c_buffer;
} cac_comp_t;

#endif
