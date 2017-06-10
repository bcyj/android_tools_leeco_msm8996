/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __IMG_COMP_REGISTRY_H__
#define __IMG_COMP_REGISTRY_H__

#include "img_comp_factory_priv.h"

/** g_img_registry
 *
 *   Global Registry for the components available in the target
 **/
const img_comp_reg_t g_img_registry[] =
{
  {
    IMG_COMP_DENOISE,
    "qcom.wavelet",
    {
      wd_comp_create,
      wd_comp_load,
      wd_comp_unload,
      NULL,
    },
  },
  {
    IMG_COMP_HDR,
    "qcom.hdr",
    {
      hdr_comp_create,
      hdr_comp_load,
      hdr_comp_unload,
      NULL,
    },
  },
  {
    IMG_COMP_FACE_PROC,
    "qcom.faceproc",
    {
      faceproc_comp_create,
      faceproc_comp_load,
      faceproc_comp_unload,
      NULL,
    },
  },
  {
    IMG_COMP_CAC,
    "qcom.cac",
    {
      cac_comp_create,
      cac_comp_load,
      cac_comp_unload,
      NULL,
    },
  },
  {
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    {
      frameproc_comp_create,
      frameproc_comp_load,
      frameproc_comp_unload,
      NULL,
    },
  },
};

#define IMG_COMP_REG_SIZE sizeof(g_img_registry)/sizeof(g_img_registry[0])

#endif //__IMG_COMP_REGISTRY_H__
