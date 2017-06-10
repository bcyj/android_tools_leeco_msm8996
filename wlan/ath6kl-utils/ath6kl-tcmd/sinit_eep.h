/*
* Copyright (c) 2011-2012 Qualcomm Atheros Inc. All Rights Reserved.
* Qualcomm Atheros Proprietary and Confidential.
*/

#ifndef _AR6003_EEPROM_STRUCT_H_
#define _AR6003_EEPROM_STRUCT_H_

#include <stdint.h>

#define PREPACK
#define POSTPACK __attribute__ ((packed))

#define AR6003_EEP_VER_MINOR_MASK    0xFFF
#define AR6003_EEP_MINOR_VER5	     0x5
#define AR6003_EEP_MINOR_VER10	      0xA
#define AR6K_EEPROM_SIZE_PRIOR_VER4   1024
#define AR6K_EEPROM_SIZE_LARGEST      2048

#define AR6003_NUM_5G_CAL_PIERS      8
#define AR6003_NUM_2G_CAL_PIERS      3
#define AR6003_NUM_5G_CAL_PIERS_EXPANDED      32
#define AR6003_NUM_2G_CAL_PIERS_EXPANDED      16
#define AR6003_BCHAN_UNUSED	     0xFF

#define AR6003_BOARDFLAGSEXT_PSAT_CAL_GEN_EEPROM    0x04
#define AR6003_BOARDFLAGSEXT_PSAT_CAL_ABS	    0x08

#define FREQ2FBIN(x,y) ((y) ? ((x) - 2300) : (((x) - 4800) / 5))
#define FBIN2FREQ(x,y) ((y) ? ((x) + 2300) : (((x) * 5) + 4800))
#define AR6003_MAX_CHAINS	     1
#define AR6003_EEPROM_MODAL_SPURS    5
#define AR6003_NUM_ALPHATHERM_CHAN_PIERS	4
#define AR6003_NUM_ALPHATHERM_TEMP_PIERS	4
#define AR6003_NUM_5G_20_TARGET_POWERS	8
#define AR6003_NUM_5G_40_TARGET_POWERS	8
#define AR6003_NUM_2G_CCK_TARGET_POWERS 2
#define AR6003_NUM_2G_20_TARGET_POWERS	3
#define AR6003_NUM_2G_40_TARGET_POWERS	3
#define AR6003_NUM_CTLS			21
#define AR6003_NUM_BAND_EDGES	     8
#define AR6003_NUM_PD_GAINS	     4
#define NUM_ATECAL_CHANS_2G  4
#define NUM_ATECAL_CHANS_5G  10
#define AR6003_PD_GAIN_ICEPTS	     5
#define STICKY_REG_TBL_SIZE_MAX 8
#define MAX_CUSTDATA_BYTES 16

typedef enum {
    WLAN_11A_CAPABILITY   = 1,
    WLAN_11G_CAPABILITY   = 2,
    WLAN_11AG_CAPABILITY  = 3,
}WALN_CAPABILITY;

typedef enum {
    TGTPWR_LEG_6_24 = 0,
    TGTPWR_LEG_36,
    TGTPWR_LEG_48,
    TGTPWR_LEG_54,
    TGTPWR_LEG_LAST
} TGTPWR_LEG;

typedef enum {
    TGTPWR_HT_0 = 0,
    TGTPWR_HT_1,
    TGTPWR_HT_2,
    TGTPWR_HT_3,
    TGTPWR_HT_4,
    TGTPWR_HT_5,
    TGTPWR_HT_6,
    TGTPWR_HT_7,
    TGTPWR_HT_LAST
} TGTPWR_HT;

typedef PREPACK struct Ar6003_spurChanStruct {
    uint8_t   spurChan;			/*spur mitigation enabled in the channel bitmap, 1 bit 1 channel*/
    uint8_t   spurABChoose;		 /*choose which is master spur ,spur_A or spur_B*/
    uint8_t   spurA_PrimSecChoose; /*choose primary and secondary spur which produce by spur_A*/
    uint8_t   spurB_PrimSecChoose; /*choose primary and secondary spur which produce by spur_B*/
} POSTPACK AR6003_SPUR_CHAN;

typedef union Ar6003_obdbStruct {
    uint16_t value;
    struct g_bits {
	uint16_t paloff   :  3,
		 qam	  :  3,
		 psk	  :  3,
		 cck	  :  3,
		 db	  :  3,
		 unused   :  1;
    } g_obdb;
    struct a_bits {
	uint16_t d2b	  :  3,
		 d3b	  :  3,
		 d4b	  :  3,
		 ob	  :  3,
		 unused   :  4;
    } a_obdb;
} POSTPACK AR6003_OBDB;

typedef PREPACK struct Ar6003_BaseEepHeader {
    uint32_t  length;							// 4 B
    uint16_t  checksum;							// 2 B
    uint16_t  version;							// 2 B
    uint16_t  opCapFlags;						// 2 B
    uint16_t  boardFlags;						// 2 B
    uint16_t  regDmn[2];						// 4 B
    uint16_t  subSystemId;						// 2 B
    uint16_t  blueToothOptions;						// 2 B
    uint16_t  binBuildNumber; // major(4):minor(5):build(7)		// 2 B
    uint8_t   macAddr[6];						// 6 B
    uint8_t   bdAddr[6];						// 6 B
    uint8_t   rxMask;							// 1 B
    uint8_t   txMask;							// 1 B
    int8_t    cckOfdmDelta;						// 1 B
    uint8_t   custData[MAX_CUSTDATA_BYTES];				// 16 B
    uint8_t   reserved[4];						// 4 B reserved for custData. Sometimes custData exceed the max bytes
    uint8_t   spurBaseA;						// 1 B
    uint8_t   spurBaseB;						// 1 B
    uint8_t   spurRssiThresh;						// 1 B
    uint8_t   HTCapFlags;						// 1 B
    uint8_t   boardFlagsExt;						// 1 B
    uint8_t   futureBase[6];						// 6 B
} POSTPACK AR6003_BASE_EEP_HEADER;					// 68 B

// Notes:
// - IQ CAL is done by sw for every reset, here we store the default values in case IQ CAL
//   is not done for whatever reason.
typedef PREPACK struct Ar6003_ModalEepHeader {
    uint32_t  antCtrlChain[AR6003_MAX_CHAINS];			       // 4B, "chn_b0_switch_table", AntE only
    uint32_t  antCtrlCom1;					       // 4B, "com1_switch_table", AntA, B, C, D
    uint32_t  antCtrlCom2;					       // 4B, "com2_switch_table"
    AR6003_OBDB ob_db;						       // 2B, a number of ob, db fields
    uint8_t   anaXtalConfig[6];						// 6B, tbd?: for xtal/synth related fields, tobe replaced with a struct
    int8_t    antennaGainCh[AR6003_MAX_CHAINS];				// 1B, affect antenna gain calculation
    uint8_t   switchSettling;					       // 1B, "time_switch_settling"
    uint8_t   swSettleHt40;					       // 1B, "time_switch_settling" for HT40
    uint8_t   xatten1Db[AR6003_MAX_CHAINS];			       // 1B, "xatten1_db_0"
    uint8_t   xatten1Margin[AR6003_MAX_CHAINS];				// 1B, "xatten1_margin_0"
    uint8_t   xatten1Hyst[AR6003_MAX_CHAINS];			       // 1B, "xatten1_hyst_margin_0"
    uint8_t   xatten2Db[AR6003_MAX_CHAINS];			       // 1B, "xatten2_db_0"
    uint8_t   xatten2Margin[AR6003_MAX_CHAINS];				// 1B, "xatten2_margin_0"
    uint8_t   xatten2Hyst[AR6003_MAX_CHAINS];			       // 1B, "xatten2_hyst_margin_0"
    int8_t    adcDesiredSize;					       // 1B, "adc_desired_size"
    uint8_t   txEndToXpaOff;					       // 1B, "tx_end_to_xpaa_off", "tx_end_to_xpab_off"
    uint8_t   txEndToRxOn;					       // 1B, "tx_end_to_a2_rx_on"
    uint8_t   txFrameToXpaOn;					       // 1B, "tx_frame_to_xpaa_on", "tx_frame_to_xpab_on"
    uint8_t   txFrameToDataStart;				       // 1B, "tx_frame_to_tx_d_start"
    uint8_t   txFrameToPaOn;					       // 1B, "tx_frame_to_pa_on"
    uint8_t   thresh62;							// 1B, "cf_thresh62"
    int8_t    noiseFloorThreshCh[AR6003_MAX_CHAINS];		       // 1B, tbd?: what to set?
    int8_t    rxIqCalICh[AR6003_MAX_CHAINS];			       // 1B, "rx_iqcorr_q_i_coff_0"
    int8_t    rxIqCalQCh[AR6003_MAX_CHAINS];			       // 1B, "rx_iqcorr_q_q_coff_0"
    uint8_t   xpaBiasLvl;					       // 1B, "an_top_xpabiaslvl"
    uint8_t   ht40PowerIncForPdadc;				       // 1B, affect tx power in HT40
    uint8_t   enableXpa;					       // 1B, "bb_enable_xpab | bb_enable_xpaa"
    AR6003_SPUR_CHAN spurChans[AR6003_EEPROM_MODAL_SPURS];	       // 4 * 5 = 20 B, tbd?:
    uint8_t   alphaTherm;					       // 1B, "alpha_therm"
    uint8_t   alphaThermPalOn;					       // 1B, "alpha_therm_pal_on"
    uint8_t   alphaVolt;					       // 1B, "alpha_volt"
    uint8_t   alphaVoltPalOn;					       // 1B, "alpha_volt_pal_on"
    int8_t    adcDesiredSizeHT40;				       // 1B, "adc_desired_size" for HT40
    uint8_t   paprdQuickDrop;					       // 1B, "bb_paprd_trainer_cntl3_cf_paprd_quick_drop"
    uint8_t   txGainTbl_0;	     // bb_tx_gain_table_0
    uint8_t   txGainTblDelta[16];    // bb_tx_gain_table_x (_1 to _32, 4 bits per delta) for PAL OFF??
    uint8_t   reserved[3];
    uint32_t  antCtrlExtCommon1;
    uint32_t  antCtrlExtCommon2;
    uint8_t   alphaThermChans[AR6003_NUM_ALPHATHERM_CHAN_PIERS];	// 4B, "alpha therm channels"
    uint8_t   alphaThermTbl[AR6003_NUM_ALPHATHERM_CHAN_PIERS][AR6003_NUM_ALPHATHERM_TEMP_PIERS];      // 16B, "alpha_therm_tbl" lowtemp midtemp1 midtemp2 hightemp
    uint8_t   areg_lvlctr;						// 1 B
    uint8_t   pareg_lvlctr;						// 1 B
    uint8_t   txGainTblMax;						// 1 B
    uint8_t   cckInitRfGain;						// 1 B
    uint16_t  paprdMinCorrThreshold;					// 2 B
    uint8_t   an_rxrf_bias1_pwd_ic25vga5g;				// 1 B
    uint8_t   futureModal[5];						// 5 B
} POSTPACK AR6003_MODAL_EEP_HEADER;					// 128 B

typedef PREPACK struct Ar6003_calDataPerFreq {
    uint8_t pwrPdg[AR6003_NUM_PD_GAINS][AR6003_PD_GAIN_ICEPTS];        // 20 B
    uint8_t vpdPdg[AR6003_NUM_PD_GAINS][AR6003_PD_GAIN_ICEPTS];        // 20 B
} POSTPACK AR6003_CAL_DATA_PER_FREQ;				       // 40 B

typedef PREPACK struct Ar6003_calDataPerFreqOlpc {
    int8_t  olpcGainDelta;					       // 1B, "olpc_gain_delta"
    uint8_t thermCalVal;					       // 1B, "therm_cal_value"
    uint8_t voltCalVal;							// 1B, "volt_cal_value"
    int8_t  olpcGainDeltaPALOn;						// 1B, "olpc_gain_delta_pal_on"
} POSTPACK AR6003_CAL_DATA_PER_FREQ_OLPC;			       // 4 B

typedef PREPACK struct Ar6003_calDataPerFreqOlpcExpanded {
    AR6003_CAL_DATA_PER_FREQ_OLPC olpcBasic;				// 4B
    int16_t  olpcGainDelta_t10;						// 2B  "untruncated olpc gain delta 0.1dB resolution"
    uint8_t  desiredScaleCck_t10;					// 1B, "desired_scale_cck"
    uint8_t  desiredScale6M_t10;					// 1B
    uint8_t  desiredScale36M_t10;					// 1B
    uint8_t  desiredScale54M_t10;					// 1B
    uint8_t  desiredScaleMCS0HT20_t10;					// 1B
    uint8_t  desiredScaleMCS7HT20_t10;					// 1B
    uint8_t  desiredScaleMCS0HT40_t10;					// 1B
    uint8_t  desiredScaleMCS7HT40_t10;					// 1B
} POSTPACK AR6003_CAL_DATA_PER_FREQ_OLPC_EXPANDED;			// 14 B

typedef PREPACK struct Ar6003_calDataPerFreqOlpcATEDelta {
    int16_t olpcGainDelta_t10;					       // 2B
} POSTPACK AR6003_CAL_DATA_PER_FREQ_OLPC_FE_DELTA;		       // 2B

typedef PREPACK struct Ar6003_CalTargetPowerLegacy {
    uint8_t  bChannel;
    uint8_t  tPow2x[4];
} POSTPACK AR6003_CAL_TARGET_POWER_LEG;     // 5B

typedef PREPACK struct Ar6003_CalTargetPowerHt {
    uint8_t  bChannel;
    uint8_t  tPow2x[8];
} POSTPACK AR6003_CAL_TARGET_POWER_HT;

typedef PREPACK struct Ar6003_CalCtlEdges {
    uint8_t  bChannel;
    uint8_t  tPower :6,
	     flag   :2;
} POSTPACK AR6003_CAL_CTL_EDGES;

typedef PREPACK struct Ar6003_CalCtlData {
    AR6003_CAL_CTL_EDGES  ctlEdges[AR6003_MAX_CHAINS][AR6003_NUM_BAND_EDGES];			  // 2 * 8 = 16 B
} POSTPACK AR6003_CAL_CTL_DATA;  // 16 B

typedef PREPACK struct Ar6003_CalDataPerChip {
    int16_t	thermAdcScaledGain;   // 2B, "therm_adc_scaled_gain"
    int8_t	thermAdcOffset;       // 1B, "therm_adc_offset"
    uint8_t	xtalCapOutDac;	      // 1B, "xtal_capoutdac"
    uint8_t	xtalCapInDac;	      // 1B, "xtal_capindac"
    uint8_t	refBiasTrim;	      // 1B, tbd?: what field?
} POSTPACK AR6003_CAL_DATA_PER_CHIP;  // 6 B

typedef PREPACK struct ateProvided {
    uint8_t GoldenTherm;
    uint8_t GoldenVolt;
    uint8_t pcDac;
    uint8_t vtestLCodeGolden;
    int8_t  vtestLVoltGolden;
    uint8_t vtestHCodeGolden;
    int8_t  vtestHVoltGolden;
    uint8_t numCalChans5G;
    uint8_t numCalChans2G;			      // 1B
    uint8_t calFreqPier5G[NUM_ATECAL_CHANS_5G];       // 10B
    uint8_t calFreqPier2G[NUM_ATECAL_CHANS_2G];       // 4B
    uint8_t ateZeroCalFlag;			      // 1B
} POSTPACK AR6003_ATE_PROVIDED_DATA;		      // 24B

typedef PREPACK struct stickyRegTbl {
	uint32_t address;
	uint32_t value;
} POSTPACK AR6003_STICKY_REG_TABLE;		// 8B

#define PSAT_AN_TXRF3_RDIV2G_LSB		    0	     // an_txrf3_rdiv2g;
#define PSAT_AN_TXRF3_RDIV2G_MASK		    0x3      // an_txrf3_rdiv2g;
#define PSAT_AN_TXRF3_PDPREDIST2G_LSB		    2	     // an_txrf3_pdpredist2g;
#define PSAT_AN_TXRF3_PDPREDIST2G_MASK		    0x1      // an_txrf3_pdpredist2g;
#define PSAT_AN_RXTX2_MXRGAIN_LSB		    3	     // an_rxtx2_mxrgain;
#define PSAT_AN_RXTX2_MXRGAIN_MASK		    0x3      // an_rxtx2_mxrgain;
#define PSAT_AN_RXRF_BIAS1_PWD_IC25MXR2GH_LSB	    5	     // an_rxrf_bias1_pwd_ic25mxr2gh;
#define PSAT_AN_RXRF_BIAS1_PWD_IC25MXR2GH_MASK	    0x7      // an_rxrf_bias1_pwd_ic25mxr2gh;
#define PSAT_AN_BIAS2_PWD_IC25RXRF_LSB		    8	     // an_bias2_pwd_ic25rxrf;
#define PSAT_AN_BIAS2_PWD_IC25RXRF_MASK		0x7      // an_bias2_pwd_ic25rxrf;
#define PSAT_AN_BB1_I2V_CURR2X_LSB		    11	     // an_bb1_i2v_curr2x;
#define PSAT_AN_BB1_I2V_CURR2X_MASK		    0x1      // an_bb1_i2v_curr2x;
#define PSAT_AN_TXRF6_CAPDIV2G_LSB		    12	     // an_txrf6_capdiv2g;
#define PSAT_AN_TXRF6_CAPDIV2G_MASK		    0xF      // an_txrf6_capdiv2g;

typedef PREPACK struct ar6kPSATCAL {
    int16_t    psat_t10;
    int16_t    ofdmCwDelta_t10;
    int16_t    cmacOlpcPsatDeltaGu_t10;
    int16_t    olpcGainDeltaGu_t10;
    uint8_t    olpcPcdac;
    uint8_t    psatPcdac;
    uint16_t   psatTuneParms;
    uint16_t   psatTuneParmsAlt1;
} POSTPACK AR6003_PSAT_CAL_PARMS;

typedef PREPACK struct ar6kEeprom {
    // base
    AR6003_BASE_EEP_HEADER	 baseEepHeader;							// 68 B
    // modal
    AR6003_MODAL_EEP_HEADER	 modalHeader[2];					       // 256 B

    // CAL section
    uint8_t			 calFreqPier5G[AR6003_NUM_5G_CAL_PIERS];		       // 8 B
    uint8_t			 calFreqPier2G[AR6003_NUM_2G_CAL_PIERS];		       // 3 B
    uint8_t			 padding1;						       // 1 B
    AR6003_CAL_DATA_PER_FREQ_OLPC calPierData5G[AR6003_NUM_5G_CAL_PIERS];		       // 4 * 8 = 32 B
    AR6003_CAL_DATA_PER_FREQ_OLPC calPierData2G[AR6003_NUM_2G_CAL_PIERS];		       // 4 * 3 = 12 B
    // ATE CAL data from OTP
    AR6003_CAL_DATA_PER_CHIP	 calPerChip;						       // 6 B
    // future expansion of CAL data
    AR6003_PSAT_CAL_PARMS	 psatCAL;						       // 14B

    // target power
    AR6003_CAL_TARGET_POWER_LEG  calTargetPower5G[AR6003_NUM_5G_20_TARGET_POWERS];	       // 5 * 8 = 40 B
    AR6003_CAL_TARGET_POWER_HT	 calTargetPower5GHT20[AR6003_NUM_5G_20_TARGET_POWERS];	       // 9 * 8 = 72 B
    AR6003_CAL_TARGET_POWER_HT	 calTargetPower5GHT40[AR6003_NUM_5G_40_TARGET_POWERS];	       // 9 * 8 = 72 B
    AR6003_CAL_TARGET_POWER_LEG  calTargetPowerCck[AR6003_NUM_2G_CCK_TARGET_POWERS];	       // 5 * 2 = 10 B
    AR6003_CAL_TARGET_POWER_LEG  calTargetPower2G[AR6003_NUM_2G_20_TARGET_POWERS];	       // 5 * 3 = 15 B
    AR6003_CAL_TARGET_POWER_HT	 calTargetPower2GHT20[AR6003_NUM_2G_20_TARGET_POWERS];	       // 9 * 3 = 27 B
    AR6003_CAL_TARGET_POWER_HT	 calTargetPower2GHT40[AR6003_NUM_2G_40_TARGET_POWERS];	       // 9 * 3 = 27 B	  // 263B
    uint8_t			 psatTuneParmsAlt4_H;					       // 1 B		  // 264B
    // CTL
    uint8_t			 ctlIndex[AR6003_NUM_CTLS];				       // 21 B
    uint8_t			 psatTuneParmsAlt4_L;					       // 1 B
    uint16_t			 psatTuneParmsAlt2;					       // 2 B
    AR6003_CAL_CTL_DATA		ctlData[AR6003_NUM_CTLS];				// 16 * 21 = 336 B  // 360B
											       // total: 1024B
    uint8_t			 calFreqPier5GExpanded[AR6003_NUM_5G_CAL_PIERS_EXPANDED];      // 32 B
    uint8_t			 calFreqPier2GExpanded[AR6003_NUM_2G_CAL_PIERS_EXPANDED];      // 16 B
    AR6003_CAL_DATA_PER_FREQ_OLPC_EXPANDED calPierData5GExpanded[AR6003_NUM_5G_CAL_PIERS_EXPANDED]; // 14 * 32 = 448 B
    AR6003_CAL_DATA_PER_FREQ_OLPC_EXPANDED calPierData2GExpanded[AR6003_NUM_2G_CAL_PIERS_EXPANDED]; // 14 * 16 = 224 B

    AR6003_ATE_PROVIDED_DATA	 ateProvidedData;					       // 24B

    int8_t			 calTgtPwrBkoff2GLeg[TGTPWR_LEG_LAST];			       // 4B
    int8_t			 calTgtPwrBkoff2GHT20[TGTPWR_HT_LAST];			       // 8B
    int8_t			 calTgtPwrBkoff2GHT40[TGTPWR_HT_LAST];			       // 8B

    uint16_t			 psatTuneParmsAlt3;					       // 2B
    uint16_t			 checksumExpanded;					       // 2B
											       // total: 768B
    AR6003_CAL_DATA_PER_FREQ_OLPC_FE_DELTA calDataFEDelta5G[AR6003_NUM_5G_CAL_PIERS_EXPANDED];			// 2 * 32 =  64 B
    AR6003_CAL_DATA_PER_FREQ_OLPC_FE_DELTA calDataFEDelta2G[AR6003_NUM_2G_CAL_PIERS_EXPANDED];			// 2 * 16 = 32 B
    uint16_t			psatTuneParmsAlt7;						 // 2B
    uint16_t			psatTuneParmsAlt8;						 // 2B
    uint16_t			psatTuneParmsAlt9;						 // 2B
    int16_t			cmacOlpcPsatDeltaGuMid_t10;					 // 2B
    int16_t			cmacOlpcPsatDeltaGuHigh_t10;					 // 2B
    uint16_t			psatTuneParmsAltSet[10];					 // 20B
    int8_t			psatTuneParmsOlpcPsatCmacDelta[20];				 // 20B
    uint8_t			padding5[42];							 // 42B
	  //uint32_t		       pllClkModa;
    AR6003_STICKY_REG_TABLE stickyRegTable[STICKY_REG_TBL_SIZE_MAX];				 // 64B
    //uint8_t			  stickyRegTableSize;
    uint16_t			psatTuneParmsAlt5;						 // 2B
    uint16_t			psatTuneParmsAlt6;						 // 2B
    //uint16_t			  checksum0CAL;
												 // total: 256B
} POSTPACK AR6003_EEPROM;									 // 1024 + 768 + 256 = 2048B

#endif //_AR6003_EEPROM_STRUCT_H_
