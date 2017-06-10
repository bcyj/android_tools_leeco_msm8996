/* Copyright (c) 2011, 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef ACDB_LOADER_H

#define ANC_IIR_COEFFS_REG_INT_BITS 1
#define ANC_IIR_COEFFS_REG_FRAC_BITS 7
#define ANC_IIR_COEFFS_MAX_VAL 255

#define TAIKO_ANC_NUM_IIR_FF_A_COEFFS 7
#define TAIKO_ANC_NUM_IIR_FF_B_COEFFS 8
#define TAIKO_ANC_NUM_IIR_FF_COEFFS (TAIKO_ANC_NUM_IIR_FF_A_COEFFS + TAIKO_ANC_NUM_IIR_FF_B_COEFFS)

#define TAIKO_ANC_NUM_IIR_FB_A_COEFFS 6
#define TAIKO_ANC_NUM_IIR_FB_B_COEFFS 7
#define TAIKO_ANC_NUM_IIR_FB_COEFFS (TAIKO_ANC_NUM_IIR_FB_A_COEFFS + TAIKO_ANC_NUM_IIR_FB_B_COEFFS)

#define TAIKO_ANC_NUM_LPF_FF_A_COEFFS 1
#define TAIKO_ANC_NUM_LPF_FF_B_COEFFS 1
#define TAIKO_ANC_NUM_LPF_FF_COEFFS (TAIKO_ANC_NUM_LPF_FF_A_COEFFS + TAIKO_ANC_NUM_LPF_FF_B_COEFFS)

#define TAIKO_ANC_NUM_LPF_FB_A_COEFFS 1
#define TAIKO_ANC_NUM_LPF_FB_B_COEFFS 1
#define TAIKO_ANC_NUM_LPF_FB_COEFFS (TAIKO_ANC_NUM_LPF_FB_A_COEFFS + TAIKO_ANC_NUM_LPF_FB_B_COEFFS)

enum taiko_codec_anc_input_device {
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

struct adie_codec_taiko_db_anc_cfg {
  uint8_t default_enable;          //check if the anc channel is enabled or not
  uint8_t anc_feedback_enable;     //CDC_CLK_ANC_CLK_EN_CTL
  /* CDC_ANC_CTL  -- used with anc_lr_mix_output_channel, if true and 1,
  * set it for CDC_ANC1_CTL, if true and 2, set it for CDC_ANC2_CTL, if false,
   set to false
   */
  uint8_t anc_lr_mix_enable;
  uint8_t smlpf_enable;                     //CDC_ANC_CTL
  uint8_t ff_in_enable;                     //CDC_ANC_CTL
  uint8_t hybrid_enable;                    //CDC_ANC_CTL
  uint8_t ff_out_enable;                    //CDC_ANC_CTL
  uint8_t dcflt_enable;                     //CDC_ANC_CTL
  uint8_t adaptive;                         //not sure
  uint8_t adaptive_gain_enable;             //CDC_ANC_B1_CTL
  uint8_t flex_enable;                      //CDC_ANC_B2_CTL
  uint8_t dmic_x2_enable;                   //CDC_ANC_B2_CTL
  uint8_t anc_lr_mix_output_channel;   /*only used if anc_lr_mix_enable = true
					1 = channel 1, 2 = channel 2 */
  int8_t  anc_ff_shift;                     //CDC_ANC_SHIFT // -4 -> 6
  int8_t  anc_fb_shift;                     //CDC_ANC_SHIFT // -4 -> 6
  uint8_t padding;
  int32_t anc_ff_coeff[15];                 //CDC_ANC_IIR_B3_CTL // IIR coeffs: A[2 - 8] B[1-8] - Q3.13 (signed)
  int32_t anc_gain;                         //iir coefficient calculation
  int32_t anc_ff_lpf_shift[2];              //CDC_ANC_LPF_B1_CTL // 0 -> 12
  int32_t anc_fb_coeff[13];                 //CDC_ANC_IIR_B3_CTL // IIR coeffs: A[2 - 7] B[1-7] - Q3.13 (signed)
  int32_t anc_gain_default;                 //not sure what this is for..
  int32_t anc_fb_lpf_shift[2];              //CDC_ANC_LPF_B1_CTL // 0 -> 12
  enum taiko_codec_anc_input_device    input_device;  //CDC_ANC_CTL_ADC_DMIC_SEL & CDC_CONN_ANC_B1_CTL
  uint32_t  smlpf_shift;                      //CDC_ANC_SMLPF_CTL
  uint32_t  dcflt_shift;                      //CDC_ANC_DCFLT_CTL
  int32_t   adaptive_gain;                    //CDC_ANC_GAIN_CTL
};
#endif /* ACDB_LOADER_H */
