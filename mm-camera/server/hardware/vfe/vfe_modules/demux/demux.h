/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __DEMUX_H__
#define __DEMUX_H__

#define DEMUX_GAIN(x) FLOAT_TO_Q(7 ,(x))

typedef enum {
  VFE_INPUT_FORMAT_FIRST_ROW_BAYER_RGRG_PATTERN,
  VFE_INPUT_FORMAT_FIRST_ROW_BAYER_GRGR_PATTERN,
  VFE_INPUT_FORMAT_FIRST_ROW_BAYER_BGBG_PATTERN,
  VFE_INPUT_FORMAT_FIRST_ROW_BAYER_GBGB_PATTERN,
  VFE_INPUT_FORMAT_FIRST_ROW_422_YCBYCR_PATTERN,
  VFE_INPUT_FORMAT_FIRST_ROW_422_YCRYCB_PATTERN,
  VFE_INPUT_FORMAT_FIRST_ROW_422_CBYCRY_PATTERN,
  VFE_INPUT_FORMAT_FIRST_ROW_422_CRYCBY_PATTERN,
  VFE_LAST_INPUT_FORMAT_FIRST_ROW_PATTERN_ENUM
    = VFE_INPUT_FORMAT_FIRST_ROW_422_CRYCBY_PATTERN
} VFE_InputFormatFirstRowPatternType;

typedef enum {
  VFE_DISABLE_CHROMA_COSITING_WITH_LUMA_SAMPLES,
  VFE_ENABLE_CHROMA_COSITING_WITH_LUMA_SAMPLES,
  VFE_LAST_ENABLE_CHROMA_COSITING_WITH_LUMA_SAMPLES_ENUM =
    VFE_ENABLE_CHROMA_COSITING_WITH_LUMA_SAMPLES,/* For count purposes */
} VFE_ChromaCositingForYCbCrInputEnableType;

#ifndef VFE_2X
typedef struct VFE_DemuxConfigCmdType {
  /*  period  */
  uint32_t  period         : 3;
  uint32_t  /* reserved */ :29;
  /* Demux Gain 0 Config */
  uint32_t  ch0EvenGain    :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch0OddGain     :10;
  uint32_t  /* reserved */ : 6;
  /* Demux Gain 1 Config */
  uint32_t  ch1Gain        :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch2Gain        :10;
  uint32_t  /* reserved */ : 6;
#ifdef VFE_40
  /* Demux Gain 0 Config */
  uint32_t  R_ch0EvenGain  :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  R_ch0OddGain   :10;
  uint32_t  /* reserved */ : 6;
  /* Demux Gain 1 Config */
  uint32_t  R_ch1Gain      :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  R_ch2Gain      :10;
  uint32_t  /* reserved */ : 6;
#endif
  /* Demux Gain 1 Config */
  uint32_t  evenCfg        :32;
  /* Demux Gain 1 Config */
  uint32_t  oddCfg         :32;
} __attribute__((packed, aligned(4))) VFE_DemuxConfigCmdType;
#else
typedef struct VFE_DemuxConfigCmdType {
  /* Input Format Selection */
  VFE_InputFormatFirstRowPatternType inputFormatFirstRowPattern:3;
  VFE_ChromaCositingForYCbCrInputEnableType
    chromaCositingForYCbCrInputs:1;
  unsigned int /* reserved */ :28;
  /* Demux Gain 0 Config */
  uint32_t  ch0EvenGain    :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch0OddGain     :10;
  uint32_t  /* reserved */ : 6;
  /* Demux Gain 1 Config */
  uint32_t  ch2Gain        :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch1Gain        :10;
  uint32_t  /* reserved */ : 6;
} __attribute__((packed, aligned(4))) VFE_DemuxConfigCmdType;
#endif

typedef struct VFE_DemuxGainCfgCmdType {
  uint32_t  ch0EvenGain    :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch0OddGain     :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch1Gain        :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch2Gain        :10;
  uint32_t  /* reserved */ : 6;
} __attribute__((packed, aligned(4))) VFE_DemuxGainCfgCmdType;

typedef struct {
  VFE_DemuxConfigCmdType VFE_PreviewDemuxConfigCmd;
  VFE_DemuxConfigCmdType VFE_SnapDemuxConfigCmd;
  VFE_DemuxGainCfgCmdType VFE_PreviewRImageGainConfigCmd;
  VFE_DemuxGainCfgCmdType VFE_SnapRImageGainConfigCmd;
  chromatix_channel_balance_gains_type gain[2]; /* 0 preview 1 snap*/
  chromatix_channel_balance_gains_type r_gain[2]; /* 0 preview 1 snap*/
  vfe_module_ops_t ops;
  float dig_gain[2];
  int8_t enable;
  int8_t update;
  vfe_op_mode_t mode;
  int trigger_enable;
  int reload_params;
  int hw_enable;
}demux_mod_t;

vfe_status_t vfe_demux_ops_init(void *mod);
vfe_status_t vfe_demux_ops_deinit(void *mod);
vfe_status_t vfe_demux_set_gains(demux_mod_t *mod, vfe_params_t* parms,
  float g_odd_gain, float g_even_gain, float b_gain, float r_gain);
vfe_status_t vfe_demux_config(int mod_id, void *mod, void *parms);
vfe_status_t vfe_demux_enable(int mod_id, void *mod, void *parms,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_demux_init(int mod_id, void *mod, void *parms);
vfe_status_t vfe_demux_update(int mod_id, void *mod, void *parms);
vfe_status_t vfe_demux_trigger_update(int mod_id, void *mod, void *parms);
vfe_status_t vfe_demux_trigger_enable(int mod_id, void *mod,
  void *params, int enable);
vfe_status_t vfe_demux_reload_params(int mod_id, void *mod, void *parms);
vfe_status_t vfe_demux_tv_vaidate(int mod_id, void *in, void *out);
vfe_status_t vfe_demux_deinit(int mod_id, void *mod, void *params);
vfe_status_t vfe_demux_plugin_update(int module_id, void *mod,
  void *vparams);
#endif //__DEMUX_H__
