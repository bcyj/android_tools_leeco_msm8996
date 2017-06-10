/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __CHROMA_SUBSAMPLE_H__
#define __CHROMA_SUBSAMPLE_H__

#ifndef VFE_2X
/*  Chroma Subsample Config Command */
typedef struct VFE_ChromaSubsampleConfigCmdType {
  /* Chroma Subsample Selection */
  uint32_t    hCositedPhase                             : 1;
  uint32_t    vCositedPhase                             : 1;
  uint32_t    hCosited                                  : 1;
  uint32_t    vCosited                                  : 1;
  uint32_t    hsubSampleEnable                          : 1;
  uint32_t    vsubSampleEnable                          : 1;
  uint32_t    cropEnable                                : 1;
  uint32_t    /* reserved */                            :25;

  uint32_t    cropWidthLastPixel                        :12;
  uint32_t    /* reserved */                            : 4;
  uint32_t    cropWidthFirstPixel                       :12;
  uint32_t    /* reserved */                            : 4;

  uint32_t    cropHeightLastLine                        :12;
  uint32_t    /* reserved */                            : 4;
  uint32_t    cropHeightFirstLine                       :12;
  uint32_t    /* reserved */                            : 4;
} __attribute__((packed, aligned(4))) VFE_ChromaSubsampleConfigCmdType;
#else
/*  Chroma Subsample Config Command */
typedef struct VFE_ChromaSubsampleConfigCmdType {
  /* Chroma Subsample Selection */
  uint32_t    hCosited                                  : 1;
  uint32_t    vCosited                                  : 1;
  uint32_t    hsubSampleEnable                          : 1;
  uint32_t    vsubSampleEnable                          : 1;
  uint32_t    /* reserved */                            :28;
} __attribute__((packed, aligned(4))) VFE_ChromaSubsampleConfigCmdType;
#endif

typedef struct {
 VFE_ChromaSubsampleConfigCmdType Chroma_ss_cmd;
 vfe_module_ops_t ops;
 int8_t css_enable;
 int8_t css_update;
 cam_format_t main_format;
}chroma_ss_mod_t;

vfe_status_t vfe_chroma_subsample_ops_init(void *mod);
vfe_status_t vfe_chroma_subsample_ops_deinit(void *mod);
vfe_status_t vfe_chroma_subsample_config(int mod_id, void *mod_cs,
  void *vparm);
vfe_status_t vfe_chroma_subsample_enable(int mod_id, void *mod,
  void  *parm, int8_t enable, int8_t hw_write);
vfe_status_t vfe_chroma_subsample_init(int mod_id, void *mod,
  void *parm);
vfe_status_t vfe_chroma_subsample_deinit(int mod_id, void *mod,
  void *parm);
vfe_status_t vfe_chroma_subsample_plugin_update(int module_id, void *mod,
  void *vparams);
#endif //__CHROMA_SUBSAMPLE_H__
