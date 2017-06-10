/* Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef ACDB_LOADER_H

#define TABLA_ANC_NUM_IIR_FF_A_COEFFS 7
#define TABLA_ANC_NUM_IIR_FF_B_COEFFS 8
#define TABLA_ANC_NUM_IIR_FF_COEFFS (TABLA_ANC_NUM_IIR_FF_A_COEFFS + TABLA_ANC_NUM_IIR_FF_B_COEFFS)

#define TABLA_ANC_NUM_IIR_FB_A_COEFFS 6
#define TABLA_ANC_NUM_IIR_FB_B_COEFFS 7
#define TABLA_ANC_NUM_IIR_FB_COEFFS (TABLA_ANC_NUM_IIR_FB_A_COEFFS + TABLA_ANC_NUM_IIR_FB_B_COEFFS)

#define TABLA_ANC_NUM_LPF_FF_A_COEFFS 1
#define TABLA_ANC_NUM_LPF_FF_B_COEFFS 1
#define TABLA_ANC_NUM_LPF_FF_COEFFS (TABLA_ANC_NUM_LPF_FF_A_COEFFS + TABLA_ANC_NUM_LPF_FF_B_COEFFS)

#define TABLA_ANC_NUM_LPF_FB_A_COEFFS 1
#define TABLA_ANC_NUM_LPF_FB_B_COEFFS 1
#define TABLA_ANC_NUM_LPF_FB_COEFFS (TABLA_ANC_NUM_LPF_FB_A_COEFFS + TABLA_ANC_NUM_LPF_FB_B_COEFFS)

enum tabla_codec_anc_input_device {
	ADIE_CODEC_ADC1 = 0,
	ADIE_CODEC_ADC2,
	ADIE_CODEC_ADC3,
	ADIE_CODEC_ADC4,
	ADIE_CODEC_ADC5,
	ADIE_CODEC_ADC6,
	ADIE_CODEC_ADC7,
	ADIE_CODEC_DMIC1,
	ADIE_CODEC_DMIC2,
	ADIE_CODEC_DMIC3,
	ADIE_CODEC_DMIC4,
	ADIE_CODEC_DMIC5,
	ADIE_CODEC_DMIC6,
	ADIE_CODEC_INPUT_DEVICE_MAX = 0x7FFFFFFF
};

struct adie_codec_tabla_db_anc_cfg {
  uint8_t default_enable;
  uint8_t anc_feedback_enable;
  uint8_t anc_lr_mix_enable;
  uint8_t smlpf_enable;
  uint8_t lr_swap;
  uint8_t ff_in_enable;
  uint8_t hybrid_enable;
  uint8_t ff_out_enable;
  uint8_t dcflt_enable;
  uint8_t adaptive;
  uint8_t padding[3];
  uint8_t anc_lr_mix_output_channel;
  uint8_t anc_ff_shift;   //range: 0 to 15
  uint8_t anc_fb_shift;   //range: 0 to 15
  int32_t anc_ff_coeff[15];
  int32_t anc_gain;
  int32_t anc_ff_lpf_coeff[2];
  int32_t anc_fb_coeff[13];
  int32_t anc_gain_default;
  int32_t anc_fb_lpf_coeff[2];
  uint32_t input_device;
  uint32_t smlpf_shift;
  uint32_t dcflt_shift;
};

#endif /* ACDB_LOADER_H */

