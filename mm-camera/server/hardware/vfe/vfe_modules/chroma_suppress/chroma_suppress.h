/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __CHROMA_SUPPRESS_H__
#define __CHROMA_SUPPRESS_H__

#define V32_CHROMA_SUP_OFF 0x000003E8

#define Clamp(x, t1, t2) (((x) < (t1))? (t1): ((x) > (t2))? (t2): (x))
/* Chroma Suppression Config */
typedef struct VFE_ChromaSuppress_ConfigCmdType {
  /* Chroma Suppress 0 Config */
  uint32_t                      ySup1                  : 8;
  uint32_t                      ySup2                  : 8;
  uint32_t                      ySup3                  : 8;
  uint32_t                      ySup4                  : 8;

  /* Chroma Suppress 1 Config */
  uint32_t                      ySupM1                 : 7;
  uint32_t                     /* reserved  */         : 1;
  uint32_t                      ySupM3                 : 7;
  uint32_t                     /* reserved  */         : 1;

  uint32_t                      ySupS1                 : 3;
  uint32_t                     /* reserved  */         : 1;
  uint32_t                      ySupS3                 : 3;
  uint32_t                     /* reserved  */         : 1;

  uint32_t                      chromaSuppressEn       : 1;
  uint32_t                     /* reserved  */         : 3;
  /* reserved for MCE enable */
  uint32_t                      /* reserved  */        : 1;
  uint32_t                      /* reserved  */        : 3;

  /* Chroma Suppress 2 Config */
  uint32_t                      cSup1                  : 8;
  uint32_t                      cSup2                  : 8;
  uint32_t                      cSupM1                 : 7;
  uint32_t                     /* reserved  */         : 1;
  uint32_t                      cSupS1                 : 3;
  uint32_t                     /* reserved  */         : 1;
  /* reserved for QK */
  uint32_t                      /* reserved  */        : 4;
}__attribute__((packed, aligned(4))) VFE_ChromaSuppress_ConfigCmdType;

typedef struct {
  VFE_ChromaSuppress_ConfigCmdType chroma_supp_video_cmd;
  VFE_ChromaSuppress_ConfigCmdType chroma_supp_snap_cmd;
  vfe_module_ops_t ops;
  int hw_enable;
  int8_t chroma_supp_enable;
  int8_t chroma_supp_trigger;
  int8_t chroma_supp_update;
  vfe_op_mode_t prev_mode;
  float prev_aec_ratio;
}chroma_supp_mod_t;

vfe_status_t vfe_chroma_suppression_ops_init(void *mod);
vfe_status_t vfe_chroma_suppression_ops_deinit(void *mod);
vfe_status_t vfe_chroma_suppression_init(int mod_id, void *mod,
  void *parms);
vfe_status_t vfe_chroma_suppression_deinit(int mod_id, void *mod,
  void *parms);
vfe_status_t vfe_chroma_suppression_config(int mod_id, void *mod,
  void *parm);
vfe_status_t vfe_chroma_suppression_enable(int mod_id, void *mod,
  void *parm, int8_t enable, int8_t hw_write);
vfe_status_t vfe_chroma_suppression_update(int mod_id, void *mod,
  void *parm);
vfe_status_t vfe_chroma_suppresion_trigger_update(int mod_id, void *mod,
  void *parm);
vfe_status_t vfe_chroma_suppression_trigger_enable(int mod_id, void *mod,
  void *parm, int enable);
vfe_status_t vfe_chroma_suppression_test_vector_validate(
  int mod_id,void *test_input, void *test_output);
vfe_status_t vfe_chroma_suppress_plugin_update(int module_id, void *mod,
  void *vparams);
#endif //__CHROMA_SUPPRESS_H__

