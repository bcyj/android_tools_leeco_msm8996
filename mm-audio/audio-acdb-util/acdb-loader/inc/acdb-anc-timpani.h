/* Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef ACDB_LOADER_H

#define ANC_NUM_IIR_FF_B_COEFFS 8
#define ANC_NUM_IIR_FF_A_COEFFS 7
#define ANC_NUM_IIR_FB_B_COEFFS 7
#define ANC_NUM_IIR_FB_A_COEFFS 6
#define ANC_NUM_LPF_FF_B_COEFFS 3
#define ANC_NUM_LPF_FF_A_COEFFS 2
#define ANC_NUM_LPF_FB_B_COEFFS 3
#define ANC_NUM_LPF_FB_A_COEFFS 2
#define ANC_IIR_COEFFS_REG_INT_BITS 1
#define ANC_IIR_COEFFS_REG_FRAC_BITS 7
#define ANC_IIR_COEFFS_MAX_VAL 255
#define ANC_LPF_COEFFS_REG_INT_BITS 0
#define ANC_LPF_COEFFS_REG_FRAC_BITS 11
#define ANC_LPF_COEFFS_MAX_VAL 2047

enum adie_codec_device_port_id {
	ADIE_CODEC_DEV_PORT_ID_NONE = 0,
	ADIE_CODEC_DEV_PORT_ID_EAR,		 /**< Handset ear */
	ADIE_CODEC_DEV_PORT_ID_HPH,			/**< Headphone */
	ADIE_CODEC_DEV_PORT_ID_HPH_CLASSD,      /**< Headphone Class D */
	ADIE_CODEC_DEV_PORT_ID_LINE_OUT,		       /**< Line analog output*/
	ADIE_CODEC_DEV_PORT_ID_RX_AUX,		       /**< Auxiliary analog output */
	ADIE_CODEC_DEV_PORT_ID_DMICA,		/**< Digital Microphone 1 and 2 */
	ADIE_CODEC_DEV_PORT_ID_AMIC1,			      /**< Analog Microphone 1 */
	ADIE_CODEC_DEV_PORT_ID_DMICB,		/**< Digital Microphone 3 and 4 */
	ADIE_CODEC_DEV_PORT_ID_AMIC2,			      /**< Analog Microphone 2 */
	ADIE_CODEC_DEV_PORT_ID_TX_AUX,			   /**< Auxiliary analog input */
	ADIE_CODEC_DEV_PORT_ID_LINE_IN,			/**< Line analog input */
	ADIE_CODEC_DEV_PORT_ID_NUM,
	ADIE_CODEC_DEV_PORT_ID_MAX = 0x7FFFFFFF
};

struct adie_codec_anc_chnl_config {
	enum adie_codec_device_port_id     eANC_InputDevice;
	int		       bANC_InputLRSwap;
	int32_t			 iANC_Gain;
	int32_t			 iANC_Gain_Default;
};

struct adie_codec_db_anc_cfg {
	int		       bDefaultEnbl;
	int		       bANC_FeedBackEnbl;
	int		       bANC_LRMixEnbl;
	enum adie_codec_anc_channel       eANC_LRMixOutputChannel;
	int32_t			 iANC_FFCoeff[15];
	int32_t			 iANC_FBCoeff[13];
	uint32_t			iANC_FF_Shift;
	uint32_t			iANC_FB_Shift;
	int32_t			 iANC_FF_LPFCoeff[5];
	int32_t			 iANC_FB_LPFCoeff[5];
	uint32_t			iANC_AGCTargetAmplitude;
	struct adie_codec_anc_chnl_config
	ancChnlConfig[ADIE_CODEC_ANC_CHANNEL_NUM];
};

#define ANC_REG_ADDR(anc_ch)\
{\
  TIMPANI_A_CDC_##anc_ch##_CTL1,\
  TIMPANI_A_CDC_##anc_ch##_CTL2,\
  TIMPANI_A_CDC_##anc_ch##_FF_FB_SHIFT,\
  TIMPANI_A_CDC_##anc_ch##_RX_NS,\
  TIMPANI_A_CDC_##anc_ch##_IIR_COEFF_PTR,\
  TIMPANI_A_CDC_##anc_ch##_IIR_COEFF_MSB,\
  TIMPANI_A_CDC_##anc_ch##_IIR_COEFF_LSB,\
  TIMPANI_A_CDC_##anc_ch##_IIR_COEFF_CTL,\
  TIMPANI_A_CDC_##anc_ch##_LPF_COEFF_PTR,\
  TIMPANI_A_CDC_##anc_ch##_LPF_COEFF_MSB,\
  TIMPANI_A_CDC_##anc_ch##_LPF_COEFF_LSB,\
  TIMPANI_A_CDC_##anc_ch##_SCALE_PTR,\
  TIMPANI_A_CDC_##anc_ch##_SCALE,\
}

struct AdieCodecANCRegAddr {
  uint8_t TIMPANI_CDC_ANC_CTL1;
  uint8_t TIMPANI_CDC_ANC_CTL2;
  uint8_t TIMPANI_CDC_ANC_LPF_GAIN;
  uint8_t TIMPANI_CDC_ANC_NS;
  uint8_t TIMPANI_CDC_ANC_IIR_PTR;
  uint8_t TIMPANI_CDC_ANC_IIR_MSB;
  uint8_t TIMPANI_CDC_ANC_IIR_LSB;
  uint8_t TIMPANI_CDC_ANC_IIR_CTL;
  uint8_t TIMPANI_CDC_ANC_LPF_PTR;
  uint8_t TIMPANI_CDC_ANC_LPF_MSB;
  uint8_t TIMPANI_CDC_ANC_LPF_LSB;
  uint8_t TIMPANI_CDC_ANC_SCALE_PTR;
  uint8_t TIMPANI_CDC_ANC_SCALE;
};

static struct AdieCodecANCRegAddr ancRegAddr[] = {
  ANC_REG_ADDR(ANC1),
  ANC_REG_ADDR(ANC2)
};

#endif /* ACDB_LOADER_H */

