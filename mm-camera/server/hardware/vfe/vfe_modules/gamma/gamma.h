/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __GAMMA_H__
#define __GAMMA_H__

#ifndef VFE_2X
#define VFE_GAMMA_NUM_ENTRIES         64
#else
#define VFE_GAMMA_NUM_ENTRIES         256
#endif
#define GAMMA_TABLE_CHROMATICS_SIZE   1024

typedef enum {
  VFE_GAMMA_LINEAR_MAPPING_OF_10_BIT_GAMMA_TABLE,
  VFE_GAMMA_PIECEWISE_LINEAR_MAPPING_OF_10_BIT_GAMMA_TABLE,
  VFE_LAST_GAMMA_MAPPING_MODE_ENUM
    = VFE_GAMMA_PIECEWISE_LINEAR_MAPPING_OF_10_BIT_GAMMA_TABLE
} VFE_GammaMappingModeType;

#ifndef VFE_2X
typedef struct VFE_GammaLutSelect {
  /* LUT Bank Select Config */
  uint32_t      ch0BankSelect               : 1;
  uint32_t      ch1BankSelect               : 1;
  uint32_t      ch2BankSelect               : 1;
  uint32_t     /* reserved */               :29;
}__attribute__((packed, aligned(4))) VFE_GammaLutSelect;

typedef struct VFE_GammaTable {
  int16_t table[VFE_GAMMA_NUM_ENTRIES];
} VFE_GammaTable;

typedef struct VFE_GammaConfigCmdType {
  VFE_GammaLutSelect LutSel;
  VFE_GammaTable Gamatbl;
} VFE_GammaConfigCmdType;
#else
typedef struct {
  uint32_t VFE_GammaTableEntryByte0:8;
  uint32_t VFE_GammaTableEntryByte1:8;
  uint32_t VFE_GammaTableEntryByte2:8;
  uint32_t VFE_GammaTableEntryByte3:8;
} __attribute__ ((packed, aligned(4))) vfe_gammaentry_t;

typedef struct VFE_GammaTable {
  uint32_t table[VFE_GAMMA_NUM_ENTRIES];
} VFE_GammaTable;

typedef struct VFE_Y_GammaTable {
  uint8_t table[1024];
} VFE_Y_GammaTable;

typedef struct VFE_GammaConfigCmdType {
  /* Gamma Selection */
  VFE_GammaMappingModeType gammaMappingMode:1;
  unsigned int /* reserved */ :31;
  /* Gamma Entries 0-255 */
  VFE_GammaTable Gamatbl;
}__attribute__((packed, aligned(4))) VFE_GammaConfigCmdType;

typedef struct VFE_Y_GammaConfigCmdType {
  /* Gamma Selection */
  VFE_GammaMappingModeType gammaMappingMode:1;
  unsigned int /* reserved */ :31;
  /* Gamma Entries 0-255 */
  vfe_gammaentry_t Gamatbl[256];
}__attribute__((packed, aligned(4))) VFE_Y_GammaConfigCmdType;

#endif

typedef enum {
  GAMMA_TABLE_DEFAULT = 0,
  GAMMA_TABLE_OUTDOOR,
  GAMMA_TABLE_LOWLIGHT,
  GAMMA_TABLE_BACKLIGHT,
  GAMMA_TABLE_SOLARIZE,
  GAMMA_TABLE_POSTERIZE,
  GAMMA_TABLE_WHITE_BOARD,
  GAMMA_TABLE_BLACK_BOARD,
  GAMMA_TABLE_INVALID
} vfe_gamma_table_t;

typedef struct {
  int8_t enable;
  int8_t trigger_update;
  int8_t update;
  VFE_GammaConfigCmdType VFE_GammaCfgCmd;
#ifdef VFE_2X
  VFE_Y_GammaConfigCmdType VFE_Y_GammaCfgCmd;
#endif
  uint8_t gamma_table[2][GAMMA_TABLE_SIZE];
  uint8_t* p_gamma_table[2]; /* 0 - preview 1 - snapshot */
  uint8_t solarize_gamma_table[GAMMA_TABLE_SIZE];
  vfe_op_mode_t cur_mode;
  vfe_module_ops_t ops;
  trigger_ratio_t gamma_ratio;
  int32_t contrast;
  vfe_gamma_table_t gamma_table_type;
  uint32_t backlight_severity;
  int enable_backlight_compensation;
  int trigger_enable;
  int reload_params;
  int hw_enable;
  uint32_t vfe_reconfig;
} gamma_mod_t;

vfe_status_t vfe_gamma_ops_init(void *mod);
vfe_status_t vfe_gamma_ops_deinit(void *mod);
vfe_status_t vfe_gamma_init(int mod_id, void *mod, void *parms);
vfe_status_t vfe_gamma_enable(int mod_id, void *mod, void *parms,
  int8_t g_enable, int8_t hw_write);
vfe_status_t vfe_gamma_update(int mod_id, void *mod, void *parms);
vfe_status_t vfe_gamma_config(int mod_id, void *mod_g, void *vparms);
vfe_status_t vfe_gamma_trigger_update(int mod_id, void *mod, void *parms);
vfe_status_t vfe_gamma_set_table(gamma_mod_t* mod, vfe_params_t *parms,
  vfe_gamma_table_t gamma_table_type);
vfe_status_t vfe_gamma_get_table(gamma_mod_t* mod, vfe_params_t *parms,
  uint32_t *size, int16_t **pp_out_table);
vfe_status_t vfe_gamma_set_contrast(int mod_id, void *mod_g, void *vparms,
  int32_t contrast);
vfe_status_t vfe_gamma_set_spl_effect(int mod_id, void *mod_g, void *vparms,
  vfe_spl_effects_type type);
vfe_status_t vfe_gamma_set_bestshot(int mod_id, void *mod, void *parms,
  camera_bestshot_mode_type mode);
vfe_status_t vfe_gamma_trigger_enable(int mod_id, void *mod, void *parms,
  int enable);
vfe_status_t vfe_gamma_reload_params(int mod_id, void *mod, void *parms);
vfe_status_t vfe_gamma_tv_validate(int mod_id, void *input,
  void* output);
vfe_status_t vfe_gamma_deinit(int mod_id, void *mod, void *parms);
vfe_status_t vfe_gamma_plugin_update(int module_id, void *mod,
  void *vparams);
#endif //__GAMMA_H__
