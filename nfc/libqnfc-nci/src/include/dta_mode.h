/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/**
 * This header contains helper methods for the DTA mode of the stack.
 */

#ifndef DTA_MODE_H
#define DTA_MODE_H

#include "../../hal/int/nfc_brcm_defs.h"
#include "dta_flag.h"

static char* nci_pmid_to_string(UINT8 pmid) {
  switch (pmid)
    {
      // NCI standard parameters:
    case NFC_PMID_TOTAL_DURATION:
      return "NFC_PMID_TOTAL_DURATION";
    case NFC_PMID_CON_DEVICES_LIMIT:
      return "NFC_PMID_CON_DEVICES_LIMIT";
    case NFC_PMID_PA_BAILOUT:
      return "NFC_PMID_PA_BAILOUT";
    case NFC_PMID_PB_AFI:
      return "NFC_PMID_PB_AFI";
    case NFC_PMID_PB_BAILOUT:
      return "NFC_PMID_PB_BAILOUT";
    case NFC_PMID_PB_ATTRIB_PARAM1:
      return "NFC_PMID_PB_ATTRIB_PARAM1";
    case NFC_PMID_PF_BIT_RATE:
      return "NFC_PMID_PF_BIT_RATE";
    case NFC_PMID_PB_H_INFO:
      return "NFC_PMID_PB_H_INFO";
    case NFC_PMID_BITR_NFC_DEP:
      return "NFC_PMID_BITR_NFC_DEP";
    case NFC_PMID_ATR_REQ_GEN_BYTES:
      return "NFC_PMID_ATR_REQ_GEN_BYTES";
    case NFC_PMID_ATR_REQ_CONFIG:
      return "NFC_PMID_ATR_REQ_CONFIG";
    case NFC_PMID_LA_HIST_BY:
      return "NFC_PMID_LA_HIST_BY";
    case NFC_PMID_LA_NFCID1:
      return "NFC_PMID_LA_NFCID1";
    case NFC_PMID_PI_BIT_RATE:
      return "NFC_PMID_PI_BIT_RATE";
    case NFC_PMID_LA_BIT_FRAME_SDD:
      return "NFC_PMID_LA_BIT_FRAME_SDD";
    case NFC_PMID_LA_PLATFORM_CONFIG:
      return "NFC_PMID_LA_PLATFORM_CONFIG";
    case NFC_PMID_LA_SEL_INFO:
      return "NFC_PMID_LA_SEL_INFO";
    case NFC_PMID_LI_BIT_RATE:
      return "NFC_PMID_LI_BIT_RATE";
    case NFC_PMID_LB_SENSB_INFO:  // same as NFC_PMID_LB_PROTOCOL
      return "NFC_PMID_LB_SENSB_INFO / NFC_PMID_LB_PROTOCOL";
    case NFC_PMID_LB_H_INFO:
      return "NFC_PMID_LB_H_INFO";
    case NFC_PMID_LB_NFCID0:
      return "NFC_PMID_LB_NFCID0";
    case NFC_PMID_LB_APPDATA:
      return "NFC_PMID_LB_APPDATA";
    case NFC_PMID_LB_SFGI:
      return "NFC_PMID_LB_SFGI";
    case NFC_PMID_LB_ADC_FO:
      return "NFC_PMID_LB_ADC_FO";
    case NFC_PMID_LF_T3T_ID1:
      return "NFC_PMID_LF_T3T_ID1";
    case NFC_PMID_LF_T3T_ID2:
      return "NFC_PMID_LF_T3T_ID2";
    case NFC_PMID_LF_T3T_ID3:
      return "NFC_PMID_LF_T3T_ID3";
    case NFC_PMID_LF_T3T_ID4:
      return "NFC_PMID_LF_T3T_ID4";
    case NFC_PMID_LF_T3T_ID5:
      return "NFC_PMID_LF_T3T_ID5";
    case NFC_PMID_LF_T3T_ID6:
      return "NFC_PMID_LF_T3T_ID6";
    case NFC_PMID_LF_T3T_ID7:
      return "NFC_PMID_LF_T3T_ID7";
    case NFC_PMID_LF_T3T_ID8:
      return "NFC_PMID_LF_T3T_ID8";
    case NFC_PMID_LF_T3T_ID9:
      return "NFC_PMID_LF_T3T_ID9";
    case NFC_PMID_LF_T3T_ID10:
      return "NFC_PMID_LF_T3T_ID10";
    case NFC_PMID_LF_T3T_ID11:
      return "NFC_PMID_LF_T3T_ID11";
    case NFC_PMID_LF_T3T_ID12:
      return "NFC_PMID_LF_T3T_ID12";
    case NFC_PMID_LF_T3T_ID13:
      return "NFC_PMID_LF_T3T_ID13";
    case NFC_PMID_LF_T3T_ID14:
      return "NFC_PMID_LF_T3T_ID14";
    case NFC_PMID_LF_T3T_ID15:
      return "NFC_PMID_LF_T3T_ID15";
    case NFC_PMID_LF_T3T_ID16:
      return "NFC_PMID_LF_T3T_ID16";
    case NFC_PMID_LF_PROTOCOL:
      return "NFC_PMID_LF_PROTOCOL";
    case NFC_PMID_LF_T3T_PMM:
      return "NFC_PMID_LF_T3T_PMM";
    case NFC_PMID_LF_T3T_MAX:
      return "NFC_PMID_LF_T3T_MAX";
    case NFC_PMID_LF_T3T_FLAGS2:
      return "NFC_PMID_LF_T3T_FLAGS2";
    case NFC_PMID_FWI:
      return "NFC_PMID_FWI";
    case NFC_PMID_LF_CON_BITR_F:
      return "NFC_PMID_LF_CON_BITR_F";
    case NFC_PMID_WT:
      return "NFC_PMID_WT";
    case NFC_PMID_ATR_RES_GEN_BYTES:
      return "NFC_PMID_ATR_RES_GEN_BYTES";
    case NFC_PMID_ATR_RSP_CONFIG:
      return "NFC_PMID_ATR_RSP_CONFIG";
    case NFC_PMID_RF_FIELD_INFO:
      return "NFC_PMID_RF_FIELD_INFO";
    case NFC_PMID_NFC_DEP_OP:
      return "NFC_PMID_NFC_DEP_OP";
      //      case NFC_PARAM_ID_RF_EE_ACTION:    // these two have no actual NCI param value defined
      //return "NFC_PARAM_ID_RF_EE_ACTION";
      //      case NFC_PARAM_ID_ISO_DEP_OP:
      //      return "NFC_PARAM_ID_ISO_DEP_OP";

      // Broadcom Proprietary parameters:

    /*case NCI_PARAM_ID_LA_FSDI:
      return "NCI_PARAM_ID_LA_FSDI (brcm)";
    case NCI_PARAM_ID_LB_FSDI:
      return "NCI_PARAM_ID_LB_FSDI (brcm)";
    case NCI_PARAM_ID_HOST_LISTEN_MASK:
      return "NCI_PARAM_ID_HOST_LISTEN_MASK (brcm)";
    case NCI_PARAM_ID_CHIP_TYPE:
      return "NCI_PARAM_ID_CHIP_TYPE (brcm)";
    case NCI_PARAM_ID_PA_ANTICOLL:
      return "NCI_PARAM_ID_PA_ANTICOLL (brcm)";
    case NCI_PARAM_ID_CONTINUE_MODE:
      return "NCI_PARAM_ID_CONTINUE_MODE (brcm)";
    case NCI_PARAM_ID_LBP:
      return "NCI_PARAM_ID_LBP (brcm)";
    case NCI_PARAM_ID_T1T_RDR_ONLY:
      return "NCI_PARAM_ID_T1T_RDR_ONLY (brcm)";
    case NCI_PARAM_ID_LA_SENS_RES:
      return "NCI_PARAM_ID_LA_SENS_RES (brcm)";
    case NCI_PARAM_ID_PWR_SETTING_BITMAP:
      return "NCI_PARAM_ID_PWR_SETTING_BITMAP (brcm)";
    case NCI_PARAM_ID_WI_NTF_ENABLE:
      return "NCI_PARAM_ID_WI_NTF_ENABLE (brcm)";
    case NCI_PARAM_ID_LN_BITRATE:
      return "NCI_PARAM_ID_LN_BITRATE (brcm)";
    case NCI_PARAM_ID_LF_BITRATE:
      return "NCI_PARAM_ID_LF_BITRATE (brcm)";
    case NCI_PARAM_ID_SWP_BITRATE_MASK:
      return "NCI_PARAM_ID_SWP_BITRATE_MASK (brcm)";
    case NCI_PARAM_ID_KOVIO:
      return "NCI_PARAM_ID_KOVIO (brcm)";
    case NCI_PARAM_ID_UICC_NTF_TO:
      return "NCI_PARAM_ID_UICC_NTF_TO (brcm)";
    case NCI_PARAM_ID_NFCDEP:
      return "NCI_PARAM_ID_NFCDEP (brcm)";
    case NCI_PARAM_ID_CLF_REGS_CFG:
      return "NCI_PARAM_ID_CLF_REGS_CFG (brcm)";
    case NCI_PARAM_ID_NFCDEP_TRANS_TIME:
      return "NCI_PARAM_ID_NFCDEP_TRANS_TIME (brcm)";
    case NCI_PARAM_ID_CREDIT_TIMER:
      return "NCI_PARAM_ID_CREDIT_TIMER (brcm)";
    case NCI_PARAM_ID_CORRUPT_RX:
      return "NCI_PARAM_ID_CORRUPT_RX (brcm)";
    case NCI_PARAM_ID_ISODEP:
      return "NCI_PARAM_ID_ISODEP (brcm)";
    case NCI_PARAM_ID_LF_CONFIG:
      return "NCI_PARAM_ID_LF_CONFIG (brcm)";
    case NCI_PARAM_ID_I93_DATARATE:
      return "NCI_PARAM_ID_I93_DATARATE (brcm)";
    case NCI_PARAM_ID_CREDITS_THRESHOLD:
      return "NCI_PARAM_ID_CREDITS_THRESHOLD (brcm)";
    case NCI_PARAM_ID_TAGSNIFF_CFG:
      return "NCI_PARAM_ID_TAGSNIFF_CFG (brcm)";
    case NCI_PARAM_ID_PA_FSDI:
      return "NCI_PARAM_ID_PA_FSDI (brcm)";
    case NCI_PARAM_ID_PB_FSDI:
      return "NCI_PARAM_ID_PB_FSDI (brcm)";
    case NCI_PARAM_ID_FRAME_INTF_RETXN:
      return "NCI_PARAM_ID_FRAME_INTF_RETXN (brcm)";
    case NCI_PARAM_ID_UICC_RDR_PRIORITY:
      return "NCI_PARAM_ID_UICC_RDR_PRIORITY (brcm)";
    case NCI_PARAM_ID_GUARD_TIME:
      return "NCI_PARAM_ID_GUARD_TIME (brcm)";
    case NCI_PARAM_ID_STDCONFIG:
      return "NCI_PARAM_ID_STDCONFIG (brcm)";
    case NCI_PARAM_ID_PROPCFG:
      return "NCI_PARAM_ID_PROPCFG (brcm)";
    case NCI_PARAM_ID_MAXTRY2ACTIVATE:
      return "NCI_PARAM_ID_MAXTRY2ACTIVATE (brcm)";
    case NCI_PARAM_ID_SWPCFG:
      return "NCI_PARAM_ID_SWPCFG (brcm)";
    case NCI_PARAM_ID_CLF_LPM_CFG:
      return "NCI_PARAM_ID_CLF_LPM_CFG (brcm)";
    case NCI_PARAM_ID_DCLB:
      return "NCI_PARAM_ID_DCLB (brcm)";
    case NCI_PARAM_ID_ACT_ORDER:
      return "NCI_PARAM_ID_ACT_ORDER (brcm)";
    case NCI_PARAM_ID_DEP_DELAY_ACT:
      return "NCI_PARAM_ID_DEP_DELAY_ACT (brcm)";
    case NCI_PARAM_ID_DH_PARITY_CRC_CTL:
      return "NCI_PARAM_ID_DH_PARITY_CRC_CTL (brcm)";
    case NCI_PARAM_ID_PREINIT_DSP_CFG:
      return "NCI_PARAM_ID_PREINIT_DSP_CFG (brcm)";
    case NCI_PARAM_ID_FW_WORKAROUND:
      return "NCI_PARAM_ID_FW_WORKAROUND (brcm)";
    case NCI_PARAM_ID_RFU_CONFIG:
      return "NCI_PARAM_ID_RFU_CONFIG (brcm)";
    case NCI_PARAM_ID_EMVCO_ENABLE:
      return "NCI_PARAM_ID_EMVCO_ENABLE (brcm)";
    case NCI_PARAM_ID_ANTDRIVER_PARAM:
      return "NCI_PARAM_ID_ANTDRIVER_PARAM (brcm)";
    case NCI_PARAM_ID_PLL325_CFG_PARAM:
      return "NCI_PARAM_ID_PLL325_CFG_PARAM (brcm)";
    case NCI_PARAM_ID_OPNLP_ADPLL_ENABLE:
      return "NCI_PARAM_ID_OPNLP_ADPLL_ENABLE (brcm)";
    case NCI_PARAM_ID_CONFORMANCE_MODE:
      return "NCI_PARAM_ID_CONFORMANCE_MODE (brcm)";
    case NCI_PARAM_ID_LPO_ON_OFF_ENABLE:
      return "NCI_PARAM_ID_LPO_ON_OFF_ENABLE (brcm)";
    case NCI_PARAM_ID_FORCE_VANT:
      return "NCI_PARAM_ID_FORCE_VANT (brcm)";
    case NCI_PARAM_ID_COEX_CONFIG:
      return "NCI_PARAM_ID_COEX_CONFIG (brcm)";
    case NCI_PARAM_ID_INTEL_MODE:
      return "NCI_PARAM_ID_INTEL_MODE (brcm)";
    case NCI_PARAM_ID_AID:
      return "NCI_PARAM_ID_AID (brcm)";*/

    default:
      return ("???? UNKNOWN NCI PARAMETER ID");
    }
}

#endif // DTA_MODE_H
