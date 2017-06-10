#ifndef NAS_SERVICE_01_H
#define NAS_SERVICE_01_H
/**
  @file network_access_service_v01.h
  
  @brief This is the public header file which defines the nas service Data structures.

  This header file defines the types and structures that were defined in 
  nas. It contains the constant values defined, enums, structures,
  messages, and service message IDs (in that order) Structures that were 
  defined in the IDL as messages contain mandatory elements, optional 
  elements, a combination of mandatory and optional elements (mandatory 
  always come before optionals in the structure), or nothing (null message)
   
  An optional element in a message is preceded by a uint8_t value that must be
  set to true if the element is going to be included. When decoding a received
  message, the uint8_t values will be set to true or false by the decode
  routine, and should be checked before accessing the values that they
  correspond to. 
   
  Variable sized arrays are defined as static sized arrays with an unsigned
  integer (32 bit) preceding it that must be set to the number of elements
  in the array that are valid. For Example:
   
  uint32_t test_opaque_len;
  uint8_t test_opaque[16];
   
  If only 4 elements are added to test_opaque[] then test_opaque_len must be
  set to 4 before sending the message.  When decoding, the _len value is set 
  by the decode routine and should be checked so that the correct number of 
  elements in the array will be accessed. 

*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2006-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/nas/main/latest/api/network_access_service_v01.h#94 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It was generated on: Wed Jul 31 2013 (Spin 0)
   From IDL File: network_access_service_v01.idl */

/** @defgroup nas_qmi_consts Constant values defined in the IDL */
/** @defgroup nas_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup nas_qmi_enums Enumerated types used in QMI messages */
/** @defgroup nas_qmi_messages Structures sent as QMI messages */
/** @defgroup nas_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup nas_qmi_accessor Accessor for QMI service object */
/** @defgroup nas_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup nas_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define NAS_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define NAS_V01_IDL_MINOR_VERS 0x5A
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define NAS_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define NAS_V01_MAX_MESSAGE_ID 0x0085;
/** 
    @} 
  */


/** @addtogroup nas_qmi_consts 
    @{ 
  */

/**  Constants used for various array max lengths */
#define NAS_SIG_STRENGTH_LIST_MAX_V01 2
#define NAS_SIG_STRENGTH_THRESHOLD_LIST_MAX_V01 5
#define NAS_ECIO_THRESHOLD_LIST_MAX_V01 10
#define NAS_SINR_THRESHOLD_LIST_MAX_V01 5
#define NAS_SPC_MAX_V01 6
#define NAS_MCC_MNC_MAX_V01 3
#define NAS_RSSI_LIST_MAX_V01 7
#define NAS_ECIO_LIST_MAX_V01 6
#define NAS_ERROR_RATE_LIST_MAX_V01 16
#define NAS_3GPP_NETWORK_INFO_LIST_MAX_V01 40
#define NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01 85
#define NAS_STATIC_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01 40
#define NAS_3GPP_FORBIDDEN_NETWORKS_LIST_MAX_V01 64
#define NAS_IS_856_MAX_LEN_V01 16
#define NAS_RF_BAND_INFO_LIST_MAX_V01 16
#define NAS_SO_LIST_MAX_V01 32
#define NAS_NETWORK_DESCRIPTION_MAX_V01 255
#define NAS_RADIO_IF_LIST_MAX_V01 255
#define NAS_DATA_CAPABILITIES_LIST_MAX_V01 10
#define NAS_ROAMING_INDICATOR_LIST_MAX_V01 2
#define NAS_ACQ_ORDER_LIST_MAX_V01 10
#define NAS_SERVICE_PROVIDER_NAME_MAX_V01 16
#define NAS_OPERATOR_PLMN_LIST_MAX_V01 255
#define NAS_PLMN_NETWORK_NAME_LIST_MAX_V01 64
#define NAS_LONG_NAME_MAX_V01 255
#define NAS_SHORT_NAME_MAX_V01 255
#define NAS_PLMN_NAME_MAX_V01 255
#define QMI_NAS_UATI_LENGTH_V01 16
#define QMI_NAS_REQUEST_SIG_INFO_RSSI_BIT_V01 0
#define QMI_NAS_REQUEST_SIG_INFO_ECIO_BIT_V01 1
#define QMI_NAS_REQUEST_SIG_INFO_IO_BIT_V01 2
#define QMI_NAS_REQUEST_SIG_INFO_SINR_BIT_V01 3
#define QMI_NAS_REQUEST_SIG_INFO_ERROR_RATE_BIT_V01 4
#define QMI_NAS_REQUEST_SIG_INFO_RSRQ_BIT_V01 5
#define QMI_NAS_RAT_UMTS_BIT_V01 15
#define QMI_NAS_RAT_LTE_BIT_V01 14
#define QMI_NAS_RAT_GSM_BIT_V01 7
#define QMI_NAS_RAT_GSM_COMPACT_BIT_V01 6
#define QMI_NAS_RAT_NOT_AVAILABLE_V01 0
#define QMI_NAS_PROTOCOL_SUBTYPE_2_PHYSICAL_LAYER_BIT_V01 0
#define QMI_NAS_PROTOCOL_SUBTYPE_CCMAC_BIT_V01 1
#define QMI_NAS_PROTOCOL_SUBTYPE_ACMAC_BIT_V01 2
#define QMI_NAS_PROTOCOL_SUBTYPE_FTCMAC_BIT_V01 3
#define QMI_NAS_PROTOCOL_SUBTYPE_3_RTCMAC_BIT_V01 4
#define QMI_NAS_PROTOCOL_SUBTYPE_1_RTCMAC_BIT_V01 5
#define QMI_NAS_PROTOCOL_SUBTYPE_IDLE_BIT_V01 6
#define QMI_NAS_PROTOCOL_SUBTYPE_GEN_MULTI_DISC_PORT_BIT_V01 7
#define QMI_NAS_BROADCAST_SUBTYPE_GENERIC_BIT_V01 0
#define QMI_NAS_APP_SUBTYPE_MULTIFLOW_BIT_V01 0
#define QMI_NAS_APP_SUBTYPE_ENHANCED_MULTIFLOW_BIT_V01 1
#define QMI_NAS_AKEY_LEN_V01 26
#define NAS_MAX_LTE_NGBR_WCDMA_NUM_FREQS_V01 2
#define NAS_MAX_LTE_NGBR_WCDMA_NUM_CELLS_V01 8
#define NAS_MAX_LTE_NGBR_GSM_NUM_FREQS_V01 2
#define NAS_MAX_LTE_NGBR_GSM_NUM_CELLS_V01 8
#define NAS_MAX_LTE_NGBR_NUM_FREQS_V01 3
#define NAS_MAX_LTE_NGBR_NUM_CELLS_V01 8
#define QMI_NAS_RAT_MODE_PREF_CDMA2000_1X_BIT_V01 0
#define QMI_NAS_RAT_MODE_PREF_CDMA2000_HRPD_BIT_V01 1
#define QMI_NAS_RAT_MODE_PREF_GSM_BIT_V01 2
#define QMI_NAS_RAT_MODE_PREF_UMTS_BIT_V01 3
#define QMI_NAS_RAT_MODE_PREF_LTE_BIT_V01 4
#define QMI_NAS_RAT_MODE_PREF_TDSCDMA_BIT_V01 5
#define QMI_NAS_DDTM_ACTION_SUPPRESS_L2ACK_BIT_V01 0
#define QMI_NAS_DDTM_ACTION_SUPPRESS_REG_BIT_V01 1
#define QMI_NAS_DDTM_ACTION_IGNORE_SO_PAGES_BIT_V01 2
#define QMI_NAS_DDTM_ACTION_SUPPRESS_MO_DBM_BIT_V01 3
#define QMI_NAS_NETWORK_IN_USE_STATUS_BITS_V01 0x03
#define QMI_NAS_NETWORK_IN_USE_STATUS_UNKNOWN_V01 0
#define QMI_NAS_NETWORK_IN_USE_STATUS_CURRENT_SERVING_V01 1
#define QMI_NAS_NETWORK_IN_USE_STATUS_AVAILABLE_V01 2
#define QMI_NAS_NETWORK_ROAMING_STATUS_BITS_V01 0x0C
#define QMI_NAS_NETWORK_ROAMING_STATUS_UNKNOWN_V01 0
#define QMI_NAS_NETWORK_ROAMING_STATUS_HOME_V01 1
#define QMI_NAS_NETWORK_ROAMING_STATUS_ROAM_V01 2
#define QMI_NAS_NETWORK_FORBIDDEN_STATUS_BITS_V01 0x30
#define QMI_NAS_NETWORK_FORBIDDEN_STATUS_UNKNOWN_V01 0
#define QMI_NAS_NETWORK_FORBIDDEN_STATUS_FORBIDDEN_V01 1
#define QMI_NAS_NETWORK_FORBIDDEN_STATUS_NOT_FORBIDDEN_V01 2
#define QMI_NAS_NETWORK_PREFERRED_STATUS_BITS_V01 0xC0
#define QMI_NAS_NETWORK_PREFERRED_STATUS_UNKNOWN_V01 0
#define QMI_NAS_NETWORK_PREFERRED_STATUS_PREFERRED_V01 1
#define QMI_NAS_NETWORK_PREFERRED_STATUS_NOT_PREFERRED_V01 2
#define QMI_NAS_SCM_EXT_IND_BIT_V01 0x80
#define QMI_NAS_SCM_EXT_IND_BAND_CLASS_1_POINT_4_V01 1
#define QMI_NAS_SCM_EXT_IND_OTHER_BAND_V01 0
#define QMI_NAS_SCM_MODE_BIT_V01 0x40
#define QMI_NAS_SCM_MODE_DUAL_V01 1
#define QMI_NAS_SCM_MODE_CDMA_ONLY_V01 0
#define QMI_NAS_SCM_SLOTTED_BIT_V01 0x20
#define QMI_NAS_SCM_MEID_CONFIGURED_BIT_V01 0x10
#define QMI_NAS_SCM_25_MHZ_BANDWIDTH_BIT_V01 0x08
#define QMI_NAS_SCM_TRANSMISSION_BIT_V01 0x04
#define QMI_NAS_SCM_TRANSMISSION_DISCONTINUOUS_V01 1
#define QMI_NAS_SCM_TRANSMISSION_CONTINUOUS_V01 0
#define QMI_NAS_SCM_POWER_CLASS_BIT_V01 0x03
#define QMI_NAS_SCM_POWER_CLASS_I_V01 0
#define QMI_NAS_SCM_POWER_CLASS_II_V01 1
#define QMI_NAS_SCM_POWER_CLASS_III_V01 2
#define QMI_NAS_SCM_POWER_CLASS_RESERVED_V01 3
#define NAS_MAX_NAM_NAME_LEN_V01 12
#define NAS_MAX_3GPP2_SUBS_INFO_DIR_NUM_LEN_V01 10
#define MDN_MAX_LEN_V01 15
#define NAS_MAX_3GPP2_HOME_SID_NID_NUM_V01 20
#define NAS_MCC_LEN_V01 3
#define NAS_IMSI_11_12_LEN_V01 2
#define NAS_IMSI_MIN1_LEN_V01 7
#define NAS_IMSI_MIN2_LEN_V01 3
#define NAS_PLMN_LEN_V01 3
#define NAS_NMR_MAX_NUM_V01 6
#define NAS_UMTS_MAX_MONITORED_CELL_SET_NUM_V01 24
#define NAS_UMTS_GERAN_MAX_NBR_CELL_SET_NUM_V01 8
#define NAS_UMTS_LTE_MAX_NBR_CELL_SET_NUM_V01 32
#define NAS_UMTS_MAX_ACTIVE_CELL_SET_NUM_V01 10
#define NAS_SPN_LEN_MAX_V01 16
#define NAS_MAX_SVC_STAT_V01 10
#define NAS_MAX_SVC_DOMAIN_V01 10
#define NAS_MAX_SVC_CAPA_V01 10
#define NAS_MAX_REG_REJ_V01 10
#define NAS_MAX_ROAM_V01 10
#define NAS_MAX_FORBIDDEN_SYS_V01 10
#define NAS_MAX_SYS_ID_V01 10
#define NAS_MAX_LAC_V01 10
#define NAS_MAX_TAC_V01 10
#define NAS_MAX_CELL_INFO_V01 10
#define NAS_MAX_BS_LOC_V01 10
#define NAS_MAX_PKT_ZONE_V01 10
#define NAS_SIG_STR_THRESHOLD_LIST_MAX_V01 16
#define NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01 32
#define NAS_CDMA_POSITION_INFO_MAX_V01 10
#define NAS_TECHNOLOGY_PREF_BITMASK_3GPP2_V01 0x01
#define NAS_TECHNOLOGY_PREF_BITMASK_3GPP_V01 0x02
#define NAS_TECHNOLOGY_PREF_BITMASK_ANALOG_V01 0x04
#define NAS_TECHNOLOGY_PREF_BITMASK_DIGITAL_V01 0x08
#define NAS_TECHNOLOGY_PREF_BITMASK_HDR_V01 0x10
#define NAS_TECHNOLOGY_PREF_BITMASK_LTE_V01 0x20
#define NAS_GET_PLMN_ID_MAX_LEN_V01 32
#define NAS_TDS_MAX_NBR_CELL_NUM_V01 8
#define NAS_LTE_EMBMS_MAX_MBSFN_AREA_V01 8
#define NAS_CSG_NAME_MAX_V01 48
#define NAS_IMS_REG_STATUS_MAX_V01 64
#define NAS_MAX_CDMA_SYSTEMS_AVOIDED_V01 10
#define NAS_LTE_BAND_PRIORITY_LIST_MAX_V01 128
#define NAS_TMGI_IDENTIFIER_LEN_V01 6
#define NAS_TMGI_BEARER_INFO_MAX_V01 32
#define NAS_MAX_BUILTIN_OPLMN_ENTRIES_V01 500
#define NAS_ALT_LANG_NAME_LEN_MAX_V01 64
#define NAS_ALT_LANG_MAX_V01 10
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_TRI_STATE_BOOLEAN_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_TRI_FALSE_V01 = 0, /**<  Status: FALSE \n  */
  NAS_TRI_TRUE_V01 = 1, /**<  Status: TRUE  \n  */
  NAS_TRI_UNKNOWN_V01 = 2, /**<  Status: Unknown   */
  NAS_TRI_STATE_BOOLEAN_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_tri_state_boolean_type_v01;
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_reset_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Resets the NAS service state variables of the requesting
              control point.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_reset_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Aborts a previously issued QMI_NAS command. */
typedef struct {

  /* Mandatory */
  /*  TX_ID */
  uint16_t tx_id;
  /**<   Transaction ID of the request to be aborted.
  */
}nas_abort_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Aborts a previously issued QMI_NAS command. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_abort_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t report_signal_strength;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report
  */

  uint32_t report_signal_strength_threshold_list_len;  /**< Must be set to # of elements in report_signal_strength_threshold_list */
  int8_t report_signal_strength_threshold_list[NAS_SIG_STRENGTH_THRESHOLD_LIST_MAX_V01];
  /**<   A sequence of thresholds delimiting signal strength Var bands. 
       Each threshold specifies the signal strength (in dBm) at which
       an event report indication, including the current signal
       strength, will be sent to the requesting control point. 
       Threshold is a signed 1 byte value. Valid values: -128 dBm
       to +127 dBm.
  */
}nas_signal_stregth_indicator_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t report_rssi;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report
  */

  uint8_t rssi_delta;
  /**<   RSSI delta (in dBm) at which an event report indication,
       including the current RSSI, will be sent to the requesting
       control point. RSSI delta is an unsigned 1 byte value.
  */
}nas_rssi_indicator_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t report_ecio;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report
  */

  uint8_t ecio_delta;
  /**<   ECIO delta at which an event report indication,
       ecio_delta including the current ECIO, will be sent to the
       requesting control point. ECIO delta is an unsigned 1 byte
       value that increments in negative 0.5 dB, e.g., ecio_delta of
       2 means a change of -1 dB.
  */
}nas_ecio_indicator_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t report_io;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report
  */

  uint8_t io_delta;
  /**<   IO delta (in dBm) at which an event report indication,
       io_delta including the current IO, will be sent to the
       requesting control point. IO delta is an unsigned 1 byte value.
  */
}nas_io_indicator_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t report_sinr;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report
  */

  uint8_t sinr_delta;
  /**<   SINR delta level at which an event report indication,
       sinr_delta including the current SINR, will be sent to the
       requesting control point. SINR delta level is an unsigned
       1 byte value.
  */
}nas_sinr_indicator_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t report_rsrq;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report
  */

  uint8_t rsrq_delta;
  /**<   RSRQ delta level at which an event report indication, including the
      current RSRQ, will be sent to the requesting control point. 
      RSRQ delta level is an unsigned 1 byte value.
  */
}nas_rsrq_indicator_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t report_ecio;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report
  */

  uint32_t threshold_list_len;  /**< Must be set to # of elements in threshold_list */
  int16_t threshold_list[NAS_ECIO_THRESHOLD_LIST_MAX_V01];
  /**<  
      A sequence of thresholds delimiting ECIO event reporting bands.
      Every time a new ECIO value crosses a threshold value, an event
      report indication message with the new ECIO value is sent to the
      requesting control point. For this field: \n

      - Each threshold value is a signed 2 byte value \n
      - Maximum number of threshold values is 10           \n
      - At least one value must be specified (if report_ecio is set)
   */
}nas_ecio_indicator_threshold_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t report_sinr;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report
  */

  /*  sinr threshold list */
  uint32_t threshold_list_len;  /**< Must be set to # of elements in threshold_list */
  uint8_t threshold_list[NAS_SINR_THRESHOLD_LIST_MAX_V01];
  /**<  
   A sequence of thresholds delimiting SINR event reporting bands.
   Every time a new SINR value crosses a threshold value, an event
   report indication message with the new SINR value is sent to the
   requesting control point. For this field: \n

   - Each threshold value will be an unsigned 1 byte value \n
   - Maximum number of threshold values is 5              \n
   - At least one value must be specified (if report_sinr is set)
 */
}nas_sinr_indicator_threshold_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t report_lte_rsrp;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report
  */

  uint8_t lte_rsrp_delta;
  /**<   LTE RSRP delta level at which an event report indication, including the
       current RSRP, will be sent to the requesting control point. LTE RSRP 
       delta level is an unsigned 1 byte value, representing the delta in dB.
  */
}nas_rsrp_indicator_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t report_lte_snr;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report
  */

  uint16_t lte_snr_delta;
  /**<   LTE SNR delta level at which an event report indication, including the
       current SNR, will be sent to the requesting control point. LTE SNR delta 
       level is an unsigned 2 byte value, representing the delta in units 
       of 0.1 dB, e.g., lte_snr_delta of 3 means a change 0.3 dB.
  */
}nas_snr_indicator_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets the NAS state reporting conditions for the requesting
              control point. (Deprecated) */
typedef struct {

  /* Optional */
  /*  Signal Strength Indicator */
  uint8_t signal_strength_valid;  /**< Must be set to true if signal_strength is being passed */
  nas_signal_stregth_indicator_type_v01 signal_strength;

  /* Optional */
  /*  RF Band Information */
  uint8_t report_rf_band_info_valid;  /**< Must be set to true if report_rf_band_info is being passed */
  uint8_t report_rf_band_info;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report
  */

  /* Optional */
  /*  Registration Reject Reason** */
  uint8_t report_reg_reject_valid;  /**< Must be set to true if report_reg_reject is being passed */
  uint8_t report_reg_reject;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report
  */

  /* Optional */
  /*  RSSI Indicator */
  uint8_t rssi_indicator_valid;  /**< Must be set to true if rssi_indicator is being passed */
  nas_rssi_indicator_type_v01 rssi_indicator;

  /* Optional */
  /*  ECIO Indicator */
  uint8_t ecio_indicator_valid;  /**< Must be set to true if ecio_indicator is being passed */
  nas_ecio_indicator_type_v01 ecio_indicator;

  /* Optional */
  /*  IO Indicator* */
  uint8_t io_indicator_valid;  /**< Must be set to true if io_indicator is being passed */
  nas_io_indicator_type_v01 io_indicator;

  /* Optional */
  /*  SINR Indicator* */
  uint8_t sinr_indicator_valid;  /**< Must be set to true if sinr_indicator is being passed */
  nas_sinr_indicator_type_v01 sinr_indicator;

  /* Optional */
  /*  Error Rate Indicator */
  uint8_t report_error_rate_valid;  /**< Must be set to true if report_error_rate is being passed */
  uint8_t report_error_rate;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report
  */

  /* Optional */
  /*  RSRQ Indicator* */
  uint8_t rsrq_indicator_valid;  /**< Must be set to true if rsrq_indicator is being passed */
  nas_rsrq_indicator_type_v01 rsrq_indicator;

  /* Optional */
  /*  ECIO Threshold */
  uint8_t ecio_threshold_indicator_valid;  /**< Must be set to true if ecio_threshold_indicator is being passed */
  nas_ecio_indicator_threshold_type_v01 ecio_threshold_indicator;

  /* Optional */
  /*  SINR Threshold */
  uint8_t sinr_threshold_indicator_valid;  /**< Must be set to true if sinr_threshold_indicator is being passed */
  nas_sinr_indicator_threshold_type_v01 sinr_threshold_indicator;

  /* Optional */
  /*  LTE SNR Delta */
  uint8_t lte_snr_delta_indicator_valid;  /**< Must be set to true if lte_snr_delta_indicator is being passed */
  nas_snr_indicator_type_v01 lte_snr_delta_indicator;

  /* Optional */
  /*  RSRP Delta */
  uint8_t lte_rsrp_delta_indicator_valid;  /**< Must be set to true if lte_rsrp_delta_indicator is being passed */
  nas_rsrp_indicator_type_v01 lte_rsrp_delta_indicator;
}nas_set_event_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets the NAS state reporting conditions for the requesting
              control point. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_set_event_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_RADIO_IF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_RADIO_IF_NO_SVC_V01 = 0x00, 
  NAS_RADIO_IF_CDMA_1X_V01 = 0x01, 
  NAS_RADIO_IF_CDMA_1XEVDO_V01 = 0x02, 
  NAS_RADIO_IF_AMPS_V01 = 0x03, 
  NAS_RADIO_IF_GSM_V01 = 0x04, 
  NAS_RADIO_IF_UMTS_V01 = 0x05, 
  NAS_RADIO_IF_WLAN_V01 = 0x06, 
  NAS_RADIO_IF_GPS_V01 = 0x07, 
  NAS_RADIO_IF_LTE_V01 = 0x08, 
  NAS_RADIO_IF_TDSCDMA_V01 = 0x09, 
  NAS_RADIO_IF_NO_CHANGE_V01 = -1, 
  NAS_RADIO_IF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_radio_if_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  int8_t sig_strength;
  /**<   Received signal strength in dBm:                           \n
       - For CDMA and UMTS, this indicates forward link pilot Ec  \n
       - For GSM, this indicates received signal strength \n
       - For LTE, this indicates the total received wideband power observed by the UE
    */

  nas_radio_if_enum_v01 radio_if;
  /**<   Radio interface technology of the signal being measured. Values: \n
       -0x00 -- RADIO_IF_NO_SVC      -- None (no service) \n
       -0x01 -- RADIO_IF_CDMA_1X     -- 
        cdma2000\textsuperscript{\textregistered} 1X             \n
       -0x02 -- RADIO_IF_CDMA_1XEVDO -- 
        cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n 
       -0x03 -- RADIO_IF_AMPS        -- AMPS \n
       -0x04 -- RADIO_IF_GSM         -- GSM \n
       -0x05 -- RADIO_IF_UMTS        -- UMTS \n
       -0x08 -- RADIO_IF_LTE         -- LTE
  */
}nas_signal_strength_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_ACTIVE_BAND_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_ACTIVE_BAND_BC_0_V01 = 0, 
  NAS_ACTIVE_BAND_BC_1_V01 = 1, 
  NAS_ACTIVE_BAND_BC_3_V01 = 3, 
  NAS_ACTIVE_BAND_BC_4_V01 = 4, 
  NAS_ACTIVE_BAND_BC_5_V01 = 5, 
  NAS_ACTIVE_BAND_BC_6_V01 = 6, 
  NAS_ACTIVE_BAND_BC_7_V01 = 7, 
  NAS_ACTIVE_BAND_BC_8_V01 = 8, 
  NAS_ACTIVE_BAND_BC_9_V01 = 9, 
  NAS_ACTIVE_BAND_BC_10_V01 = 10, 
  NAS_ACTIVE_BAND_BC_11_V01 = 11, 
  NAS_ACTIVE_BAND_BC_12_V01 = 12, 
  NAS_ACTIVE_BAND_BC_13_V01 = 13, 
  NAS_ACTIVE_BAND_BC_14_V01 = 14, 
  NAS_ACTIVE_BAND_BC_15_V01 = 15, 
  NAS_ACTIVE_BAND_BC_16_V01 = 16, 
  NAS_ACTIVE_BAND_BC_17_V01 = 17, 
  NAS_ACTIVE_BAND_BC_18_V01 = 18, 
  NAS_ACTIVE_BAND_BC_19_V01 = 19, 
  NAS_ACTIVE_BAND_GSM_450_V01 = 40, 
  NAS_ACTIVE_BAND_GSM_480_V01 = 41, 
  NAS_ACTIVE_BAND_GSM_750_V01 = 42, 
  NAS_ACTIVE_BAND_GSM_850_V01 = 43, 
  NAS_ACTIVE_BAND_GSM_900_EXTENDED_V01 = 44, 
  NAS_ACTIVE_BAND_GSM_900_PRIMARY_V01 = 45, 
  NAS_ACTIVE_BAND_GSM_900_RAILWAYS_V01 = 46, 
  NAS_ACTIVE_BAND_GSM_1800_V01 = 47, 
  NAS_ACTIVE_BAND_GSM_1900_V01 = 48, 
  NAS_ACTIVE_BAND_WCDMA_2100_V01 = 80, 
  NAS_ACTIVE_BAND_WCDMA_PCS_1900_V01 = 81, 
  NAS_ACTIVE_BAND_WCDMA_DCS_1800_V01 = 82, 
  NAS_ACTIVE_BAND_WCDMA_1700_US_V01 = 83, 
  NAS_ACTIVE_BAND_WCDMA_850_V01 = 84, 
  NAS_ACTIVE_BAND_WCDMA_800_V01 = 85, 
  NAS_ACTIVE_BAND_WCDMA_2600_V01 = 86, 
  NAS_ACTIVE_BAND_WCDMA_900_V01 = 87, 
  NAS_ACTIVE_BAND_WCDMA_1700_JAPAN_V01 = 88, 
  NAS_ACTIVE_BAND_WCDMA_1500_JAPAN_V01 = 90, 
  NAS_ACTIVE_BAND_WCDMA_850_JAPAN_V01 = 91, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_1_V01 = 120, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_2_V01 = 121, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_3_V01 = 122, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_4_V01 = 123, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_5_V01 = 124, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_6_V01 = 125, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_7_V01 = 126, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_8_V01 = 127, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_9_V01 = 128, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_10_V01 = 129, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_11_V01 = 130, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_12_V01 = 131, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_13_V01 = 132, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_14_V01 = 133, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_17_V01 = 134, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_33_V01 = 135, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_34_V01 = 136, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_35_V01 = 137, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_36_V01 = 138, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_37_V01 = 139, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_38_V01 = 140, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_39_V01 = 141, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_40_V01 = 142, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_18_V01 = 143, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_19_V01 = 144, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_20_V01 = 145, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_21_V01 = 146, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_24_V01 = 147, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_25_V01 = 148, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_41_V01 = 149, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_42_V01 = 150, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_43_V01 = 151, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_23_V01 = 152, 
  NAS_ACTIVE_BAND_E_UTRA_OPERATING_BAND_26_V01 = 153, 
  NAS_ACTIVE_BAND_TDSCDMA_BAND_A_V01 = 200, 
  NAS_ACTIVE_BAND_TDSCDMA_BAND_B_V01 = 201, 
  NAS_ACTIVE_BAND_TDSCDMA_BAND_C_V01 = 202, 
  NAS_ACTIVE_BAND_TDSCDMA_BAND_D_V01 = 203, 
  NAS_ACTIVE_BAND_TDSCDMA_BAND_E_V01 = 204, 
  NAS_ACTIVE_BAND_TDSCDMA_BAND_F_V01 = 205, 
  NAS_ACTIVE_BAND_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_active_band_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_radio_if_enum_v01 radio_if;
  /**<   Radio interface currently in use. Values:  \n
       - 0x01 -- cdma2000\textsuperscript{\textregistered} 1X             \n
       - 0x02 -- cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - 0x03 -- AMPS \n
       - 0x04 -- GSM \n
       - 0x05 -- UMTS \n
       - 0x08 -- LTE \n
       - 0x09 -- TD-SCDMA
  */

  nas_active_band_enum_v01 active_band;
  /**<   Active band class (see Table @latexonly\ref{tbl:bandClass}@endlatexonly 
      for details). Values: \n
      - 00 to 39   -- CDMA band classes  \n
      - 40 to 79   -- GSM band classes   \n
      - 80 to 91   -- WCDMA band classes \n
      - 120 to 153 -- LTE band classes   \n
      - 200 to 205 -- TD-SCDMA band classes
  */

  uint16_t active_channel;
  /**<   Active channel. If the channel is not relevant to the
      technology, a value of 0 will be returned.
 */
}nas_rf_band_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_NETWORK_SERVICE_DOMAIN_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_CIRCUIT_SWITCHED_V01 = 0x01, 
  NAS_PACKET_SWITCHED_V01 = 0x02, 
  NAS_CIRCUIT_AND_PACKET_SWITCHED_V01 = 0x03, 
  NAS_NETWORK_SERVICE_DOMAIN_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_network_service_domain_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_network_service_domain_enum_v01 service_domain;
  /**<   Network service domain that was rejected. Possible values: \n
       - 1 -- CIRCUIT_SWITCHED \n
       - 2 -- PACKET_SWITCHED \n
       - 3 -- CIRCUIT_AND_PACKET_ SWITCHED
  */

  uint16_t reject_cause;
  /**<   Reject cause; refer to \hyperref[S5]{[S5]} Sections 10.5.3.6 and 10.5.5.14, 
       and \hyperref[S16]{[S16]} Section 9.9.3.9.
  */
}nas_registration_reject_reason_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t rssi;
  /**<   RSSI represented as a positive value; control points need to
       convert this to negative to get actual value in dBm: \n
       - For CDMA and UMTS, this indicates forward link pilot Ec \n
       - For GSM, this indicates received signal strength
  */

  nas_radio_if_enum_v01 radio_if;
  /**<   Radio interface technology of the signal being measured. Values:  \n
       -0x00 -- RADIO_IF_NO_SVC      -- None (no service) \n
       -0x01 -- RADIO_IF_CDMA_1X     -- 
        cdma2000\textsuperscript{\textregistered} 1X             \n
       -0x02 -- RADIO_IF_CDMA_1XEVDO -- 
        cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       -0x03 -- RADIO_IF_AMPS        -- AMPS \n
       -0x04 -- RADIO_IF_GSM         -- GSM \n
       -0x05 -- RADIO_IF_UMTS        -- UMTS \n
       -0x08 -- RADIO_IF_LTE         -- LTE
  */
}nas_rssi_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t ecio;
  /**<   ECIO value representing negative 0.5 dB increments, i.e., 
       2 means -1 dB (14 means -7 dB, 63 means -31.5 dB).
  */

  nas_radio_if_enum_v01 radio_if;
  /**<   Radio interface technology of the signal being measured. Values:  \n
       -0x00 -- RADIO_IF_NO_SVC      -- None (no service) \n
       -0x01 -- RADIO_IF_CDMA_1X     -- 
        cdma2000\textsuperscript{\textregistered} 1X             \n
       -0x02 -- RADIO_IF_CDMA_1XEVDO -- 
        cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       -0x03 -- RADIO_IF_AMPS        -- AMPS \n
       -0x04 -- RADIO_IF_GSM         -- GSM \n
       -0x05 -- RADIO_IF_UMTS        -- UMTS 
  */
}nas_ecio_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t error_rate;
  /**<   Error rate value corresponds to the RAT that is currently registered. \n
         For CDMA, the error rate reported is Frame Error Rate: \n
         - Valid error rate values between 1 and 10000 are returned to indicate 
            percentage, e.g., a value of 300 means the error rate is 3% \n
         - A value of 0xFFFF indicates that the error rate is unknown or 
            unavailable \n
         For HDR, the error rate reported is Packet Error Rate: \n
         - Valid error rate values between 1 and 10000 are returned to indicate 
            percentage, e.g., a value of 300 means the error rate is 3% \n
         - A value of 0xFFFF indicates that the error rate is unknown or 
            unavailable \n
         For GSM, the error rate reported is Bit Error Rate: \n
         - Valid values are 0, 100, 200, 300, 400, 500, 600, and 700 \n
         - The reported value divided by 100 gives the error rate as an RxQual 
            value as defined in \hyperref[S13]{[S13]} Section 8.2.4, e.g., a value 
            of 300 represents an RxQual value of 3 \n
         - A value of 25500 indicates No Data \n
         For WCDMA, the error rate reported is Block Error Rate (BLER): \n
         - Valid values are 1 to 10000 \n
         - The reported value divided by 100 provides the error rate in 
            percentages, e.g., a value of 300 represents a BLER of 3% \n
         - A value of 0 indicates No Data
  */

  nas_radio_if_enum_v01 radio_if;
  /**<   Radio interface technology of the signal being measured. Values:  \n
       -0x00 -- RADIO_IF_NO_SVC      -- None (no service) \n
       -0x01 -- RADIO_IF_CDMA_1X     -- 
        cdma2000\textsuperscript{\textregistered} 1X             \n
       -0x02 -- RADIO_IF_CDMA_1XEVDO -- 
        cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       -0x03 -- RADIO_IF_AMPS        -- AMPS \n
       -0x04 -- RADIO_IF_GSM         -- GSM \n
       -0x05 -- RADIO_IF_UMTS        -- UMTS
  */
}nas_error_rate_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  int8_t rsrq;
  /**<   RSRQ value in dB (signed integer value).
       Range: -3 to -20 (-3 means -3 dB, -20 means -20 dB).
  */

  uint8_t radio_if;
  /**<   Radio interface technology of the signal being measured. Values: \n
       - 0x08 -- LTE
  */
}nas_rsrq_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SINR_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SINR_LEVEL_0_V01 = 0x00, 
  NAS_SINR_LEVEL_1_V01 = 0x01, 
  NAS_SINR_LEVEL_2_V01 = 0x02, 
  NAS_SINR_LEVEL_3_V01 = 0x03, 
  NAS_SINR_LEVEL_4_V01 = 0x04, 
  NAS_SINR_LEVEL_5_V01 = 0x05, 
  NAS_SINR_LEVEL_6_V01 = 0x06, 
  NAS_SINR_LEVEL_7_V01 = 0x07, 
  NAS_SINR_LEVEL_8_V01 = 0x08, 
  NAS_SINR_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_sinr_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Indicates the NAS state change. (Deprecated) */
typedef struct {

  /* Optional */
  /*  Signal Strength */
  uint8_t signal_strength_valid;  /**< Must be set to true if signal_strength is being passed */
  nas_signal_strength_type_v01 signal_strength;

  /* Optional */
  /*  RF Band Information List */
  uint8_t rf_band_info_list_valid;  /**< Must be set to true if rf_band_info_list is being passed */
  uint32_t rf_band_info_list_len;  /**< Must be set to # of elements in rf_band_info_list */
  nas_rf_band_info_type_v01 rf_band_info_list[NAS_RADIO_IF_LIST_MAX_V01];

  /* Optional */
  /*  Registration Reject Reason** */
  uint8_t registration_reject_reason_valid;  /**< Must be set to true if registration_reject_reason is being passed */
  nas_registration_reject_reason_type_v01 registration_reject_reason;

  /* Optional */
  /*  RSSI */
  uint8_t rssi_valid;  /**< Must be set to true if rssi is being passed */
  nas_rssi_type_v01 rssi;

  /* Optional */
  /*  ECIO */
  uint8_t ecio_valid;  /**< Must be set to true if ecio is being passed */
  nas_ecio_type_v01 ecio;

  /* Optional */
  /*  IO* */
  uint8_t io_valid;  /**< Must be set to true if io is being passed */
  int32_t io;
  /**<   Received IO in dBm. IO is only applicable for 1xEV-DO.
  */

  /* Optional */
  /*  SINR* */
  uint8_t sinr_valid;  /**< Must be set to true if sinr is being passed */
  nas_sinr_enum_v01 sinr;
  /**<   SINR level. SINR is only applicable for 1xEV-DO. 
       Valid levels are 0 to 8, where the maximum value for:        \n
       - 0x00 -- SINR_LEVEL_0 is -9 dB     \n
       - 0x01 -- SINR_LEVEL_1 is -6 dB     \n
       - 0x02 -- SINR_LEVEL_2 is -4.5 dB   \n
       - 0x03 -- SINR_LEVEL_3 is -3 dB     \n
       - 0x04 -- SINR_LEVEL_4 is -2 dB     \n
       - 0x05 -- SINR_LEVEL_5 is +1 dB     \n
       - 0x06 -- SINR_LEVEL_6 is +3 dB     \n
       - 0x07 -- SINR_LEVEL_7 is +6 dB     \n
       - 0x08 -- SINR_LEVEL_8 is +9 dB
  */

  /* Optional */
  /*  Error Rate */
  uint8_t error_rate_valid;  /**< Must be set to true if error_rate is being passed */
  nas_error_rate_type_v01 error_rate;

  /* Optional */
  /*  RSRQ** */
  uint8_t rsrq_valid;  /**< Must be set to true if rsrq is being passed */
  nas_rsrq_type_v01 rsrq;

  /* Optional */
  /*  LTE SNR */
  uint8_t snr_valid;  /**< Must be set to true if snr is being passed */
  int16_t snr;
  /**<   
     LTE SNR level as a scaled integer in units of 0.1 dB; 
     e.g., -16 dB has a value of -160 and 24.6 dB has a value of 246.
      */

  /* Optional */
  /*  LTE RSRP */
  uint8_t rsrp_valid;  /**< Must be set to true if rsrp is being passed */
  int16_t rsrp;
  /**<  
     Current LTE RSRP in dBm as measured by L1. 
     Range: -44 to -140 (-44 means -44 dBm, -140 means -140 dBm).
  */
}nas_event_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t reg_network_reject;
  /**<   Controls the reporting of QMI_NAS_NETWORK_REJECT_IND. Values: \n
       - 0x00 -- Disable (default value) \n
       - 0x01 -- Enable
  */

  uint8_t suppress_sys_info;
  /**<   Controls the reporting of QMI_NAS_SYS_INFO_IND when only the reject_cause 
       field has changed. Values: \n
       - 0x00 -- Do not suppress (default value) \n
       - 0x01 -- Suppress
  */
}nas_reg_network_reject_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets the registration state for different
              QMI_NAS indications for the requesting control point.
              \label{idl:indicationRegister} */
typedef struct {

  /* Optional */
  /*  System Selection Preference */
  uint8_t reg_sys_sel_pref_valid;  /**< Must be set to true if reg_sys_sel_pref is being passed */
  uint8_t reg_sys_sel_pref;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  DDTM Events */
  uint8_t reg_ddtm_events_valid;  /**< Must be set to true if reg_ddtm_events is being passed */
  uint8_t reg_ddtm_events;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Serving System Events */
  uint8_t req_serving_system_valid;  /**< Must be set to true if req_serving_system is being passed */
  uint8_t req_serving_system;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Dual Standby Preference */
  uint8_t dual_standby_pref_valid;  /**< Must be set to true if dual_standby_pref is being passed */
  uint8_t dual_standby_pref;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Subscription Info */
  uint8_t subscription_info_valid;  /**< Must be set to true if subscription_info is being passed */
  uint8_t subscription_info;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  Network Time */
  uint8_t reg_network_time_valid;  /**< Must be set to true if reg_network_time is being passed */
  uint8_t reg_network_time;
  /**<    Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  Sys Info */
  uint8_t sys_info_valid;  /**< Must be set to true if sys_info is being passed */
  uint8_t sys_info;
  /**<    Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  Signal Strength  */
  uint8_t sig_info_valid;  /**< Must be set to true if sig_info is being passed */
  uint8_t sig_info;
  /**<    Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  Error Rate */
  uint8_t err_rate_valid;  /**< Must be set to true if err_rate is being passed */
  uint8_t err_rate;
  /**<    Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  HDR New UATI Assigned  */
  uint8_t reg_hdr_uati_valid;  /**< Must be set to true if reg_hdr_uati is being passed */
  uint8_t reg_hdr_uati;
  /**<   Controls the reporting of QMI_NAS_HDR_UATI_UPDATE_IND. Values: \n
       - 0x00 -- Disable (default value) \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  HDR Session Closed */
  uint8_t reg_hdr_session_close_valid;  /**< Must be set to true if reg_hdr_session_close is being passed */
  uint8_t reg_hdr_session_close;
  /**<   Controls the reporting of QMI_NAS_HDR_SESSION_CLOSE_IND. Values: \n  
       - 0x00 -- Disable (default value) \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  Managed Roaming */
  uint8_t reg_managed_roaming_valid;  /**< Must be set to true if reg_managed_roaming is being passed */
  uint8_t reg_managed_roaming;
  /**<   Controls the reporting of QMI_NAS_MANAGED_ROAMING_IND. Values: \n
       - 0x00 -- Disable (default value) \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Current PLMN Name */
  uint8_t reg_current_plmn_name_valid;  /**< Must be set to true if reg_current_plmn_name is being passed */
  uint8_t reg_current_plmn_name;
  /**<   Controls the reporting of QMI_NAS_CURRENT_PLMN_NAME_IND. Values: \n
       - 0x00 -- Disable (default value) \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  eMBMS Status */
  uint8_t reg_embms_status_valid;  /**< Must be set to true if reg_embms_status is being passed */
  uint8_t reg_embms_status;
  /**<   Controls the reporting of QMI_NAS_EMBMS_STATUS_IND. Values: \n
       - 0x00 -- Disable (default value) \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  RF Band Information */
  uint8_t reg_rf_band_info_valid;  /**< Must be set to true if reg_rf_band_info is being passed */
  uint8_t reg_rf_band_info;
  /**<   Controls the reporting of QMI_NAS_RF_BAND_INFO_IND. Values: \n
       - 0x00 -- Disable (default value) \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Network Reject Information */
  uint8_t network_reject_valid;  /**< Must be set to true if network_reject is being passed */
  nas_reg_network_reject_v01 network_reject;

  /* Optional */
  /*  Operator Name Data */
  uint8_t reg_operator_name_data_valid;  /**< Must be set to true if reg_operator_name_data is being passed */
  uint8_t reg_operator_name_data;
  /**<   Controls the reporting of QMI_NAS_OPERATOR_NAME_DATA_IND. Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable (default value) 
  */

  /* Optional */
  /*  CSP PLMN Mode Bit */
  uint8_t reg_csp_plmn_mode_bit_valid;  /**< Must be set to true if reg_csp_plmn_mode_bit is being passed */
  uint8_t reg_csp_plmn_mode_bit;
  /**<   Controls the reporting of QMI_NAS_CSP_PLMN_MODE_BIT_IND. Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable (default value) 
  */

  /* Optional */
  /*  RTRE Configuration */
  uint8_t reg_rtre_cfg_valid;  /**< Must be set to true if reg_rtre_cfg is being passed */
  uint8_t reg_rtre_cfg;
  /**<   Controls the reporting of QMI_NAS_RTRE_CONFIG_IND. Values: \n
       - 0x00 -- Disable (default value) \n
       - 0x01 -- Enable
   */

  /* Optional */
  /*  IMS Preference Status */
  uint8_t reg_ims_pref_status_valid;  /**< Must be set to true if reg_ims_pref_status is being passed */
  uint8_t reg_ims_pref_status;
  /**<   Controls the reporting of QMI_NAS_IMS_PREF_STATUS_IND. Values: \n
       - 0x00 -- Disable (default value) \n
       - 0x01 -- Enable
   */

  /* Optional */
  /*  E911 State Ready Status */
  uint8_t reg_e911_state_ready_status_valid;  /**< Must be set to true if reg_e911_state_ready_status is being passed */
  uint8_t reg_e911_state_ready_status;
  /**<   Controls the reporting of QMI_NAS_E911_STATE_READY_IND. Values: \n
       - 0x00 -- Disable (default value) \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  LTE SIB16 Network Time */
  uint8_t reg_lte_sib16_network_time_valid;  /**< Must be set to true if reg_lte_sib16_network_time is being passed */
  uint8_t reg_lte_sib16_network_time;
  /**<   Controls the reporting of QMI_NAS_LTE_SIB16_NETWORK_TIME_IND. Values: \n
       - 0x00 -- Disable (default value) \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  LTE Physical Carrier Aggregation Information */
  uint8_t reg_lte_cphy_ca_valid;  /**< Must be set to true if reg_lte_cphy_ca is being passed */
  uint8_t reg_lte_cphy_ca;
  /**<   Controls the reporting of QMI_NAS_LTE_CPHY_CA_IND. Values: \n
       - 0x00 -- Disable (default value) \n
       - 0x01 -- Enable
  */
}nas_indication_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets the registration state for different
              QMI_NAS indications for the requesting control point.
              \label{idl:indicationRegister} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_indication_register_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef uint16_t nas_get_sig_str_req_mask_type_v01;
#define QMI_NAS_REQUEST_SIG_INFO_RSSI_MASK_V01 ((nas_get_sig_str_req_mask_type_v01)0x01) 
#define QMI_NAS_REQUEST_SIG_INFO_ECIO_MASk_V01 ((nas_get_sig_str_req_mask_type_v01)0x02) 
#define QMI_NAS_REQUEST_SIG_INFO_IO_MASK_V01 ((nas_get_sig_str_req_mask_type_v01)0x04) 
#define QMI_NAS_REQUEST_SIG_INFO_SINR_MASK_V01 ((nas_get_sig_str_req_mask_type_v01)0x08) 
#define QMI_NAS_REQUEST_SIG_INFO_ERROR_RATE_MASK_V01 ((nas_get_sig_str_req_mask_type_v01)0x10) 
#define QMI_NAS_REQUEST_SIG_INFO_RSRQ_MASK_V01 ((nas_get_sig_str_req_mask_type_v01)0x20) 
#define QMI_NAS_REQUEST_SIG_INFO_LTE_SNR_MASK_V01 ((nas_get_sig_str_req_mask_type_v01)0x40) 
#define QMI_NAS_REQUEST_SIG_INFO_LTE_RSRP_MASK_V01 ((nas_get_sig_str_req_mask_type_v01)0x80) 
/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Queries the current signal strength as measured by the device.
              (Deprecated) */
typedef struct {

  /* Optional */
  /*  Request Mask */
  uint8_t request_mask_valid;  /**< Must be set to true if request_mask is being passed */
  nas_get_sig_str_req_mask_type_v01 request_mask;
  /**<   Request additional signal information for: \n
       Bit 0 (0x01) -- QMI_NAS_REQUEST_SIG_INFO_ RSSI_MASK; values: \n
       - 0 -- Do not request additional information for RSSI \n
       - 1 -- Request additional information for RSSI

       Bit 1 (0x02) -- QMI_NAS_REQUEST_SIG_INFO_ ECIO_MASK; values: \n
       - 0 -- Do not request additional information for ECIO \n
       - 1 -- Request additional information for ECIO

       Bit 2 (0x04) -- QMI_NAS_REQUEST_SIG_INFO_ IO_MASK; values: \n
       - 0 -- Do not request additional information for IO \n
       - 1 -- Request additional information for IO

       Bit 3 (0x08) -- QMI_NAS_REQUEST_SIG_INFO_ SINR_MASK; values: \n
       - 0 -- Do not request additional information for SINR \n
       - 1 -- Request additional information for SINR
   
       Bit 4 (0x10) -- QMI_NAS_REQUEST_SIG_INFO_ ERROR_RATE_MASK; values: \n
       - 0 -- Do not request additional information for Error Rate \n
       - 1 -- Request additional information for Error Rate

       Bit 5 (0x20) -- QMI_NAS_REQUEST_SIG_INFO_ RSRQ_MASK; values: \n
       - 0 -- Do not request additional information for RSRQ \n
       - 1 -- Request additional information for RSRQ

       Bit 6 (0x40) -- QMI_NAS_REQUEST_SIG_INFO_ LTE_SNR_MASK; values: \n
       - 0 -- Do not request additional information for LTE SNR \n
       - 1 -- Request additional information for LTE SNR 

       Bit 7 (0x80) -- QMI_NAS_REQUEST_SIG_INFO_ LTE_RSRP_MASK; values: \n
       - 0 -- Do not request additional information for LTE RSRP \n
       - 1 -- Request additional information for LTE RSRP 
  */
}nas_get_signal_strength_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  int8_t sig_strength;
  /**<   Received signal strength in dBm:                            \n
       - For CDMA and UMTS, this indicates forward link pilot Ec   \n
       - For GSM, this indicates received signal strength
  */

  nas_radio_if_enum_v01 radio_if;
  /**<   Radio interface technology of the signal being measured. Values: \n
       -0x01 -- RADIO_IF_CDMA_1X         -- 
        cdma2000\textsuperscript{\textregistered} 1X                 \n
       -0x02 -- RADIO_IF_CDMA_1XEVDO     -- 
        cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO)
  */
}nas_signal_strength_list_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the current signal strength as measured by the device.
              (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Signal Strength */
  nas_signal_strength_type_v01 signal_strength;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
   */

  /* Optional */
  /*  Signal Strength List */
  uint8_t signal_strength_list_valid;  /**< Must be set to true if signal_strength_list is being passed */
  uint32_t signal_strength_list_len;  /**< Must be set to # of elements in signal_strength_list */
  nas_signal_strength_list_type_v01 signal_strength_list[NAS_SIG_STRENGTH_LIST_MAX_V01];

  /* Optional */
  /*  RSSI List */
  uint8_t rssi_valid;  /**< Must be set to true if rssi is being passed */
  uint32_t rssi_len;  /**< Must be set to # of elements in rssi */
  nas_rssi_type_v01 rssi[NAS_RSSI_LIST_MAX_V01];

  /* Optional */
  /*  ECIO List */
  uint8_t ecio_valid;  /**< Must be set to true if ecio is being passed */
  uint32_t ecio_len;  /**< Must be set to # of elements in ecio */
  nas_ecio_type_v01 ecio[NAS_ECIO_LIST_MAX_V01];

  /* Optional */
  /*  IO */
  uint8_t io_valid;  /**< Must be set to true if io is being passed */
  uint32_t io;
  /**<   Received IO in dBm. IO is only applicable for 1xEV-DO.
  */

  /* Optional */
  /*  SINR */
  uint8_t sinr_valid;  /**< Must be set to true if sinr is being passed */
  nas_sinr_enum_v01 sinr;
  /**<   SINR level. SINR is only applicable for 1xEV-DO.
       Valid levels are 0 to 8, where the maximum value for:        \n
       - 0x00 -- SINR_LEVEL_0 is -9 dB     \n
       - 0x01 -- SINR_LEVEL_1 is -6 dB     \n
       - 0x02 -- SINR_LEVEL_2 is -4.5 dB   \n
       - 0x03 -- SINR_LEVEL_3 is -3 dB     \n
       - 0x04 -- SINR_LEVEL_4 is -2 dB     \n
       - 0x05 -- SINR_LEVEL_5 is +1 dB     \n
       - 0x06 -- SINR_LEVEL_6 is +3 dB     \n
       - 0x07 -- SINR_LEVEL_7 is +6 dB     \n
       - 0x08 -- SINR_LEVEL_8 is +9 dB
  */

  /* Optional */
  /*  Error Rate List */
  uint8_t error_rate_valid;  /**< Must be set to true if error_rate is being passed */
  uint32_t error_rate_len;  /**< Must be set to # of elements in error_rate */
  nas_error_rate_type_v01 error_rate[NAS_ERROR_RATE_LIST_MAX_V01];

  /* Optional */
  /*  RSRQ */
  uint8_t rsrq_valid;  /**< Must be set to true if rsrq is being passed */
  nas_rsrq_type_v01 rsrq;

  /* Optional */
  /*  LTE SNR */
  uint8_t snr_valid;  /**< Must be set to true if snr is being passed */
  int16_t snr;
  /**<   LTE SNR level as a scaled integer in units of 0.1 dB; 
       e.g., -16 dB has a value of -160 and 24.6 dB has a value of 246.
       LTE SNR is included only when the current serving system is LTE.
      */

  /* Optional */
  /*  LTE RSRP */
  uint8_t lte_rsrp_valid;  /**< Must be set to true if lte_rsrp is being passed */
  int16_t lte_rsrp;
  /**<   Current LTE RSRP in dBm as measured by L1. 
       Range: -44 to -140 (-44 means -44 dBm, -140 means -140 dBm).
       LTE RSRP is included only if the current serving system is LTE.
    */
}nas_get_signal_strength_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef uint8_t nas_network_type_mask_type_v01;
#define NAS_NETWORK_TYPE_GSM_ONLY_V01 ((nas_network_type_mask_type_v01)0x01) 
#define NAS_NETWORK_TYPE_WCDMA_ONLY_V01 ((nas_network_type_mask_type_v01)0x02) 
#define NAS_NETWORK_TYPE_LTE_ONLY_V01 ((nas_network_type_mask_type_v01)0x04) 
#define NAS_NETWORK_TYPE_TDSCDMA_ONLY_V01 ((nas_network_type_mask_type_v01)0x08) 
/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_NW_SCAN_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SCAN_TYPE_PLMN_V01 = 0x00, 
  NAS_SCAN_TYPE_CSG_V01 = 0x01, 
  NAS_NW_SCAN_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_nw_scan_type_enum_v01;
/**
    @}
  */

typedef uint64_t nas_band_pref_mask_type_v01;
#define QMI_NAS_BAND_CLASS_0_A_SYSTEM_V01 ((nas_band_pref_mask_type_v01)0x000000000000001ull) 
#define QMI_NAS_BAND_CLASS_0_B_AB_GSM850_V01 ((nas_band_pref_mask_type_v01)0x000000000000002ull) 
#define QMI_NAS_BAND_CLASS_1_ALL_BLOCKS_V01 ((nas_band_pref_mask_type_v01)0x000000000000004ull) 
#define QMI_NAS_BAND_CLASS_2_PLACEHOLDER_V01 ((nas_band_pref_mask_type_v01)0x000000000000008ull) 
#define QMI_NAS_BAND_CLASS_3_A_SYSTEM_V01 ((nas_band_pref_mask_type_v01)0x000000000000010ull) 
#define QMI_NAS_BAND_CLASS_4_ALL_BLOCKS_V01 ((nas_band_pref_mask_type_v01)0x000000000000020ull) 
#define QMI_NAS_BAND_CLASS_5_ALL_BLOCKS_V01 ((nas_band_pref_mask_type_v01)0x000000000000040ull) 
#define QMI_NAS_GSM_DCS_1800_BAND_V01 ((nas_band_pref_mask_type_v01)0x000000000000080ull) 
#define QMI_NAS_E_GSM_900_BAND_V01 ((nas_band_pref_mask_type_v01)0x000000000000100ull) 
#define QMI_NAS_P_GSM_900_BAND_V01 ((nas_band_pref_mask_type_v01)0x000000000000200ull) 
#define QMI_NAS_BAND_CLASS_6_V01 ((nas_band_pref_mask_type_v01)0x000000000000400ull) 
#define QMI_NAS_BAND_CLASS_7_V01 ((nas_band_pref_mask_type_v01)0x000000000000800ull) 
#define QMI_NAS_BAND_CLASS_8_V01 ((nas_band_pref_mask_type_v01)0x000000000001000ull) 
#define QMI_NAS_BAND_CLASS_9_V01 ((nas_band_pref_mask_type_v01)0x000000000002000ull) 
#define QMI_NAS_BAND_CLASS_10_V01 ((nas_band_pref_mask_type_v01)0x000000000004000ull) 
#define QMI_NAS_BAND_CLASS_11_V01 ((nas_band_pref_mask_type_v01)0x000000000008000ull) 
#define QMI_NAS_GSM_BAND_450_V01 ((nas_band_pref_mask_type_v01)0x000000000010000ull) 
#define QMI_NAS_GSM_BAND_480_V01 ((nas_band_pref_mask_type_v01)0x000000000020000ull) 
#define QMI_NAS_GSM_BAND_750_V01 ((nas_band_pref_mask_type_v01)0x000000000040000ull) 
#define QMI_NAS_GSM_BAND_850_V01 ((nas_band_pref_mask_type_v01)0x000000000080000ull) 
#define QMI_NAS_GSM_BAND_RAILWAYS_900_BAND_V01 ((nas_band_pref_mask_type_v01)0x000000000100000ull) 
#define QMI_NAS_GSM_BAND_PCS_1900_BAND_V01 ((nas_band_pref_mask_type_v01)0x000000000200000ull) 
#define QMI_NAS_WCDMA_EU_J_CH_IMT_2100_BAND_V01 ((nas_band_pref_mask_type_v01)0x000000000400000ull) 
#define QMI_NAS_WCDMA_US_PCS_1900_BAND_V01 ((nas_band_pref_mask_type_v01)0x000000000800000ull) 
#define QMI_NAS_EU_CH_DCS_1800_BAND_V01 ((nas_band_pref_mask_type_v01)0x000000001000000ull) 
#define QMI_NAS_WCDMA_US_1700_BAND_V01 ((nas_band_pref_mask_type_v01)0x000000002000000ull) 
#define QMI_NAS_WCDMA_US_850_BAND_V01 ((nas_band_pref_mask_type_v01)0x000000004000000ull) 
#define QMI_NAS_WCDMA_JAPAN_800_BAND_V01 ((nas_band_pref_mask_type_v01)0x000000008000000ull) 
#define QMI_NAS_BAND_CLASS_12_V01 ((nas_band_pref_mask_type_v01)0x000000010000000ull) 
#define QMI_NAS_BAND_CLASS_14_V01 ((nas_band_pref_mask_type_v01)0x000000020000000ull) 
#define QMI_NAS_RESERVED_V01 ((nas_band_pref_mask_type_v01)0x000000040000000ull) 
#define QMI_NAS_BAND_CLASS_15_V01 ((nas_band_pref_mask_type_v01)0x000000080000000ull) 
#define QMI_NAS_WCDMA_EU_2600_BAND_V01 ((nas_band_pref_mask_type_v01)0x001000000000000ull) 
#define QMI_NAS_WCDMA_EU_J_900_BAND_V01 ((nas_band_pref_mask_type_v01)0x002000000000000ull) 
#define QMI_NAS_WCDMA_J_1700_BAND_V01 ((nas_band_pref_mask_type_v01)0x004000000000000ull) 
#define QMI_NAS_BAND_CLASS_16_V01 ((nas_band_pref_mask_type_v01)0x100000000000000ull) 
#define QMI_NAS_BAND_CLASS_17_V01 ((nas_band_pref_mask_type_v01)0x200000000000000ull) 
#define QMI_NAS_BAND_CLASS_18_V01 ((nas_band_pref_mask_type_v01)0x400000000000000ull) 
#define QMI_NAS_BAND_CLASS_19_V01 ((nas_band_pref_mask_type_v01)0x800000000000000ull) 
typedef uint64_t lte_band_pref_mask_type_v01;
#define E_UTRA_OPERATING_BAND_1_V01 ((lte_band_pref_mask_type_v01)0x000000000000001ull) 
#define E_UTRA_OPERATING_BAND_2_V01 ((lte_band_pref_mask_type_v01)0x000000000000002ull) 
#define E_UTRA_OPERATING_BAND_3_V01 ((lte_band_pref_mask_type_v01)0x000000000000004ull) 
#define E_UTRA_OPERATING_BAND_4_V01 ((lte_band_pref_mask_type_v01)0x000000000000008ull) 
#define E_UTRA_OPERATING_BAND_5_V01 ((lte_band_pref_mask_type_v01)0x000000000000010ull) 
#define E_UTRA_OPERATING_BAND_6_V01 ((lte_band_pref_mask_type_v01)0x000000000000020ull) 
#define E_UTRA_OPERATING_BAND_7_V01 ((lte_band_pref_mask_type_v01)0x000000000000040ull) 
#define E_UTRA_OPERATING_BAND_8_V01 ((lte_band_pref_mask_type_v01)0x000000000000080ull) 
#define E_UTRA_OPERATING_BAND_9_V01 ((lte_band_pref_mask_type_v01)0x000000000000100ull) 
#define E_UTRA_OPERATING_BAND_10_V01 ((lte_band_pref_mask_type_v01)0x000000000000200ull) 
#define E_UTRA_OPERATING_BAND_11_V01 ((lte_band_pref_mask_type_v01)0x000000000000400ull) 
#define E_UTRA_OPERATING_BAND_12_V01 ((lte_band_pref_mask_type_v01)0x000000000000800ull) 
#define E_UTRA_OPERATING_BAND_13_V01 ((lte_band_pref_mask_type_v01)0x000000000001000ull) 
#define E_UTRA_OPERATING_BAND_14_V01 ((lte_band_pref_mask_type_v01)0x000000000002000ull) 
#define E_UTRA_OPERATING_BAND_17_V01 ((lte_band_pref_mask_type_v01)0x000000000010000ull) 
#define E_UTRA_OPERATING_BAND_18_V01 ((lte_band_pref_mask_type_v01)0x000000000020000ull) 
#define E_UTRA_OPERATING_BAND_19_V01 ((lte_band_pref_mask_type_v01)0x000000000040000ull) 
#define E_UTRA_OPERATING_BAND_20_V01 ((lte_band_pref_mask_type_v01)0x000000000080000ull) 
#define E_UTRA_OPERATING_BAND_21_V01 ((lte_band_pref_mask_type_v01)0x000000000100000ull) 
#define E_UTRA_OPERATING_BAND_23_V01 ((lte_band_pref_mask_type_v01)0x000000000400000ull) 
#define E_UTRA_OPERATING_BAND_24_V01 ((lte_band_pref_mask_type_v01)0x000000000800000ull) 
#define E_UTRA_OPERATING_BAND_25_V01 ((lte_band_pref_mask_type_v01)0x000000001000000ull) 
#define E_UTRA_OPERATING_BAND_26_V01 ((lte_band_pref_mask_type_v01)0x000000002000000ull) 
#define E_UTRA_OPERATING_BAND_28_V01 ((lte_band_pref_mask_type_v01)0x000000010000000ull) 
#define E_UTRA_OPERATING_BAND_29_V01 ((lte_band_pref_mask_type_v01)0x000000020000000ull) 
#define E_UTRA_OPERATING_BAND_33_V01 ((lte_band_pref_mask_type_v01)0x000000100000000ull) 
#define E_UTRA_OPERATING_BAND_34_V01 ((lte_band_pref_mask_type_v01)0x000000200000000ull) 
#define E_UTRA_OPERATING_BAND_35_V01 ((lte_band_pref_mask_type_v01)0x000000400000000ull) 
#define E_UTRA_OPERATING_BAND_36_V01 ((lte_band_pref_mask_type_v01)0x000000800000000ull) 
#define E_UTRA_OPERATING_BAND_37_V01 ((lte_band_pref_mask_type_v01)0x000001000000000ull) 
#define E_UTRA_OPERATING_BAND_38_V01 ((lte_band_pref_mask_type_v01)0x000002000000000ull) 
#define E_UTRA_OPERATING_BAND_39_V01 ((lte_band_pref_mask_type_v01)0x000004000000000ull) 
#define E_UTRA_OPERATING_BAND_40_V01 ((lte_band_pref_mask_type_v01)0x000008000000000ull) 
#define E_UTRA_OPERATING_BAND_41_V01 ((lte_band_pref_mask_type_v01)0x000010000000000ull) 
#define E_UTRA_OPERATING_BAND_42_V01 ((lte_band_pref_mask_type_v01)0x000020000000000ull) 
#define E_UTRA_OPERATING_BAND_43_V01 ((lte_band_pref_mask_type_v01)0x000040000000000ull) 
typedef uint64_t nas_tdscdma_band_pref_mask_type_v01;
#define NAS_TDSCDMA_BAND_A_V01 ((nas_tdscdma_band_pref_mask_type_v01)0x01ull) /**<  TD-SCDMA Band A  */
#define NAS_TDSCDMA_BAND_B_V01 ((nas_tdscdma_band_pref_mask_type_v01)0x02ull) /**<  TD-SCDMA Band B  */
#define NAS_TDSCDMA_BAND_C_V01 ((nas_tdscdma_band_pref_mask_type_v01)0x04ull) /**<  TD-SCDMA Band C  */
#define NAS_TDSCDMA_BAND_D_V01 ((nas_tdscdma_band_pref_mask_type_v01)0x08ull) /**<  TD-SCDMA Band D  */
#define NAS_TDSCDMA_BAND_E_V01 ((nas_tdscdma_band_pref_mask_type_v01)0x10ull) /**<  TD-SCDMA Band E  */
#define NAS_TDSCDMA_BAND_F_V01 ((nas_tdscdma_band_pref_mask_type_v01)0x20ull) /**<  TD-SCDMA Band F  */
/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Performs a scan for visible networks. */
typedef struct {

  /* Optional */
  /*  Network Type */
  uint8_t network_type_valid;  /**< Must be set to true if network_type is being passed */
  nas_network_type_mask_type_v01 network_type;
  /**<   Bitmask representing the network type to scan. Values: \n
       - Bit 0 -- GSM \n
       - Bit 1 -- UMTS \n
       - Bit 2 -- LTE \n
       - Bit 3 -- TD-SCDMA \n
       Any combination of the bit positions can be used.  If the mask is
       sent with no bits set, the scan is performed using the currently 
       set preference.
    */

  /* Optional */
  /*  Scan Type */
  uint8_t scan_type_valid;  /**< Must be set to true if scan_type is being passed */
  nas_nw_scan_type_enum_v01 scan_type;
  /**<   Network scan type. Values: \n
       - 0x00 -- NAS_SCAN_TYPE_PLMN -- PLMN (default) \n
       - 0x01 -- NAS_SCAN_TYPE_CSG -- Closed subscriber group
    */

  /* Optional */
  /*  Band Preference */
  uint8_t band_pref_valid;  /**< Must be set to true if band_pref is being passed */
  nas_band_pref_mask_type_v01 band_pref;
  /**<   Bitmask representing the band preference to be scanned.  
       See Table @latexonly\ref{tbl:bandPreference}@endlatexonly 
       for details.   
  */

  /* Optional */
  /*  LTE Band Preference */
  uint8_t lte_band_pref_valid;  /**< Must be set to true if lte_band_pref is being passed */
  lte_band_pref_mask_type_v01 lte_band_pref;
  /**<   Bitmask representing the LTE band preference to be scanned. 
       See Table @latexonly\ref{tbl:lteBandPreference}@endlatexonly 
       for details.  
  */

  /* Optional */
  /*  TDSCDMA Band Preference */
  uint8_t tdscdma_band_pref_valid;  /**< Must be set to true if tdscdma_band_pref is being passed */
  nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref;
  /**<   Bitmask representing the TD-SCDMA band preference to be set. Values: \n
      - NAS_TDSCDMA_BAND_A (0x01) --  TD-SCDMA Band A 
      - NAS_TDSCDMA_BAND_B (0x02) --  TD-SCDMA Band B 
      - NAS_TDSCDMA_BAND_C (0x04) --  TD-SCDMA Band C 
      - NAS_TDSCDMA_BAND_D (0x08) --  TD-SCDMA Band D 
      - NAS_TDSCDMA_BAND_E (0x10) --  TD-SCDMA Band E 
      - NAS_TDSCDMA_BAND_F (0x20) --  TD-SCDMA Band F 

 \vspace{3pt}
 All other bits are reserved and must be set to 0.
 */
}nas_perform_network_scan_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SCAN_RESULT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SCAN_SUCCESS_V01 = 0x00, 
  NAS_SCAN_AS_ABORT_V01 = 0x01, 
  NAS_SCAN_REJ_IN_RLF_V01 = 0x02, 
  NAS_SCAN_RESULT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_scan_result_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mobile_country_code;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.
  */

  uint16_t mobile_network_code;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.
  */

  uint8_t network_status;
  /**<   Status of the network identified by MCC and MNC preceding it.
       The status is encoded in a bitmapped value as follows: \n
       Bits 0-1 -- QMI_NAS_NETWORK_IN_USE_ STATUS_BITS    -- In-use status       \n
       - 0 -- QMI_NAS_NETWORK_IN_USE_STATUS_ UNKNOWN          -- Unknown         \n
       - 1 -- QMI_NAS_NETWORK_IN_USE_STATUS_ CURRENT_SERVING  -- Current serving \n
       - 2 -- QMI_NAS_NETWORK_IN_USE_STATUS_ AVAILABLE        -- Available
       
       Bits 2-3 -- QMI_NAS_NETWORK_ROAMING_ STATUS_BITS   -- Roaming status      \n
       - 0 -- QMI_NAS_NETWORK_ROAMING_ STATUS_UNKNOWN         -- Unknown         \n
       - 1 -- QMI_NAS_NETWORK_ROAMING_ STATUS_HOME            -- Home            \n
       - 2 -- QMI_NAS_NETWORK_ROAMING_ STATUS_ROAM            -- Roam

       Bits 4-5 -- QMI_NAS_NETWORK_FORBIDDEN_ STATUS_BITS -- Forbidden status    \n
       - 0 -- QMI_NAS_NETWORK_FORBIDDEN_ STATUS_UNKNOWN       -- Unknown         \n
       - 1 -- QMI_NAS_NETWORK_FORBIDDEN_ STATUS_FORBIDDEN     -- Forbidden       \n
       - 2 -- QMI_NAS_NETWORK_FORBIDDEN_ STATUS_NOT_FORBIDDEN -- Not forbidden

       Bits 6-7 -- QMI_NAS_NETWORK_PREFERRED_ STATUS_BITS -- Preferred status    \n
       - 0 -- QMI_NAS_NETWORK_PREFERRED_ STATUS_UNKNOWN       -- Unknown         \n
       - 1 -- QMI_NAS_NETWORK_PREFERRED_ STATUS_PREFERRED     -- Preferred       \n
       - 2 -- QMI_NAS_NETWORK_PREFERRED_ STATUS_NOT_PREFERRED -- Not preferred
  */

  char network_description[NAS_NETWORK_DESCRIPTION_MAX_V01 + 1];
  /**<   An optional string containing the network name or description.
  */
}nas_3gpp_network_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mcc;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.
  */

  uint16_t mnc;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.
  */

  uint8_t rat;
  /**<   Radio access technology. Values: \n
       - 0x04 -- GERAN \n
       - 0x05 -- UMTS \n
       - 0x08 -- LTE \n
       - 0x09 -- TD-SCDMA 
  */
}nas_network_radio_access_technology_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mcc;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.
  */

  uint16_t mnc;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.
  */

  /*  MNC PCS digit include status */
  uint8_t mnc_includes_pcs_digit;
  /**<   This field is used to interpret the length of the corresponding
       MNC reported in the TLVs (in this table) with an mnc or 
       mobile_network_code field. Values: \n

       - TRUE  -- MNC is a three-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 090  \n
       - FALSE -- MNC is a two-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 90
  */
}nas_mnc_pcs_digit_include_status_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t id;
  /**<   Closed subscriber group identifier.
  */

  uint32_t name_len;  /**< Must be set to # of elements in name */
  uint16_t name[NAS_CSG_NAME_MAX_V01];
  /**<   Home Node B (HNB) or Home eNode B (HeNB) name in UTF-16.
       The network name is not guaranteed to be NULL terminated.
  */
}nas_csg_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_CSG_LIST_CAT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_CSG_LIST_CAT_UNKNOWN_V01 = 0, /**<  Unknown CSG list.  */
  NAS_CSG_LIST_CAT_ALLOWED_V01 = 1, /**<  Allowed CSG list.  */
  NAS_CSG_LIST_CAT_OPERATOR_V01 = 2, /**<  Operator CSG list.  */
  NAS_CSG_LIST_CAT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_csg_list_cat_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mcc;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.
  */

  uint16_t mnc;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.
  */

  nas_csg_list_cat_enum_v01 csg_list_cat;
  /**<   Closed subscriber group category. Values: \n
       - 0 -- NAS_CSG_LIST_CAT_UNKNOWN -- Unknown CSG list \n
       - 1 -- NAS_CSG_LIST_CAT_ALLOWED -- Allowed CSG list \n
       - 2 -- NAS_CSG_LIST_CAT_OPERATOR -- Operator CSG list
  */

  nas_csg_info_type_v01 csg_info;
  /**<   Closed subscriber group information.
  */
}nas_csg_nw_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mcc;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.
  */

  uint16_t mnc;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.
  */

  uint32_t csg_id;
  /**<   Closed subscriber group identifier.
  */

  uint32_t signal_strength;
  /**<   Signal strength information.
  */
}nas_csg_nw_signal_strength_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Performs a scan for visible networks. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. 
 Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */

  /* Optional */
  /*  3GPP Network Information** */
  uint8_t nas_3gpp_network_info_valid;  /**< Must be set to true if nas_3gpp_network_info is being passed */
  uint32_t nas_3gpp_network_info_len;  /**< Must be set to # of elements in nas_3gpp_network_info */
  nas_3gpp_network_info_type_v01 nas_3gpp_network_info[NAS_3GPP_NETWORK_INFO_LIST_MAX_V01];

  /* Optional */
  /*  Network Radio Access Technology** */
  uint8_t nas_network_radio_access_technology_valid;  /**< Must be set to true if nas_network_radio_access_technology is being passed */
  uint32_t nas_network_radio_access_technology_len;  /**< Must be set to # of elements in nas_network_radio_access_technology */
  nas_network_radio_access_technology_type_v01 nas_network_radio_access_technology[NAS_3GPP_NETWORK_INFO_LIST_MAX_V01];

  /* Optional */
  /*  MNC PCS Digit Include Status */
  uint8_t mnc_includes_pcs_digit_valid;  /**< Must be set to true if mnc_includes_pcs_digit is being passed */
  uint32_t mnc_includes_pcs_digit_len;  /**< Must be set to # of elements in mnc_includes_pcs_digit */
  nas_mnc_pcs_digit_include_status_type_v01 mnc_includes_pcs_digit[NAS_3GPP_NETWORK_INFO_LIST_MAX_V01];

  /* Optional */
  /*  Network Scan Result */
  uint8_t scan_result_valid;  /**< Must be set to true if scan_result is being passed */
  nas_scan_result_enum_v01 scan_result;
  /**<   Indicates the status of the network scan. Values: \n
       - 0x00 -- NAS_SCAN_SUCCESS -- Network scan was successful \n
       - 0x01 -- NAS_SCAN_AS_ABORT -- Network scan was aborted   \n
       - 0x02 -- NAS_SCAN_REJ_IN_RLF -- Network scan did not complete due 
                 to a radio link failure recovery in progress
  */

  /* Optional */
  /*  CSG Information */
  uint8_t csg_info_valid;  /**< Must be set to true if csg_info is being passed */
  uint32_t csg_info_len;  /**< Must be set to # of elements in csg_info */
  nas_csg_nw_info_type_v01 csg_info[NAS_3GPP_NETWORK_INFO_LIST_MAX_V01];

  /* Optional */
  /*  CSG Signal Strength Information */
  uint8_t csg_sig_info_valid;  /**< Must be set to true if csg_sig_info is being passed */
  uint32_t csg_sig_info_len;  /**< Must be set to # of elements in csg_sig_info */
  nas_csg_nw_signal_strength_info_type_v01 csg_sig_info[NAS_3GPP_NETWORK_INFO_LIST_MAX_V01];
}nas_perform_network_scan_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mobile_country_code;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.
  */

  uint16_t mobile_network_code;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.
  */

  nas_radio_if_enum_v01 radio_access_technology;
  /**<   Radio access technology for which to register. Values: \n
        -0x04 -- RADIO_IF_GSM -- GSM \n
        -0x05 -- RADIO_IF_UMTS -- UMTS \n 
        -0x08 -- RADIO_IF_LTE -- LTE \n
        -  -1 -- RADIO_IF_NO_CHANGE -- No change in the mode preference
  */
}nas_manual_network_register_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_REGISTER_ACTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_AUTO_REGISTER_V01 = 0x01, 
  NAS_MANUAL_REGISTER_V01 = 0x02, 
  NAS_REGISTER_ACTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_register_action_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_CHANGE_DURATION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_POWER_CYCLE_V01 = 0x00, 
  NAS_PERMANENT_V01 = 0x01, 
  NAS_CHANGE_DURATION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_change_duration_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Initiates a network registration. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Register Action */
  nas_register_action_enum_v01 register_action;
  /**<    Specifies one of the following actions: \n
        - 0x01 -- NAS_AUTO_REGISTER -- Device registers according
          to its provisioning; optional TLVs supplied with the command
          are ignored \n
        - 0x02 -- NAS_MANUAL_REGISTER -- Device registers to a specified
          network; the optional Manual Network Register Information TLV must also
          be included for the command to process successfully;
          supported only for 3GPP
  */

  /* Optional */
  /*  Manual Network Register Information** */
  uint8_t manual_network_register_info_valid;  /**< Must be set to true if manual_network_register_info is being passed */
  nas_manual_network_register_info_type_v01 manual_network_register_info;

  /* Optional */
  /*  Change Duration** */
  uint8_t change_duration_valid;  /**< Must be set to true if change_duration is being passed */
  nas_change_duration_enum_v01 change_duration;
  /**<    Duration of the change. Values: \n
        - 0x00 -- Power cycle -- Remains active until the next device power cycle \n
        - 0x01 -- Permanent -- Remains active through power cycles until changed by the client \n
        Note: The device will use "0x00 -- Power cycle" as the default value 
              if the TLV is omitted.
  */

  /* Optional */
  /*  MNC PCS Digit Include Status */
  uint8_t mnc_includes_pcs_digit_valid;  /**< Must be set to true if mnc_includes_pcs_digit is being passed */
  uint8_t mnc_includes_pcs_digit;
  /**<    This TLV applies to the MNC field of the manual_network_register_info 
        data structure. Values: \n
        - TRUE  -- MNC is a three-digit value \n
        - FALSE -- MNC is a two-digit value

        If this TLV is not included in the case of a manual register option, 
        the value of the MNC value specified in manual_network_register_info 
        is interpreted as follows: \n
        - If the MNC value is less than 100, the MNC value provided is 
          interpreted as a two-digit value. \n
        - If the MNC value is greater than or equal to 100, the MNC value 
          provided is interpreted as a three-digit value.
  */
}nas_initiate_network_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Initiates a network registration. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_initiate_network_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_PS_ATTACH_ACTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_PS_ACTION_ATTACH_V01 = 0x01, 
  NAS_PS_ACTION_DETACH_V01 = 0x02, 
  NAS_PS_ATTACH_ACTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_ps_attach_action_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Initiates a domain attach or detach action. (Deprecated) */
typedef struct {

  /* Optional */
  /*  PS Attach Action** */
  uint8_t ps_attach_action_valid;  /**< Must be set to true if ps_attach_action is being passed */
  nas_ps_attach_action_enum_v01 ps_attach_action;
  /**<   Initiates a packet domain attach or detach action. Values: \n
       - 0x01 -- PS_ACTION_ATTACH -- Initiates an immediate packet domain attach action \n
       - 0x02 -- PS_ACTION_DETACH -- Initiates an immediate packet domain detach action 
  */
}nas_initiate_attach_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Initiates a domain attach or detach action. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_initiate_attach_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_serving_system_req_msg_v01;

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_REGISTRATION_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_NOT_REGISTERED_V01 = 0x00, 
  NAS_REGISTERED_V01 = 0x01, 
  NAS_NOT_REGISTERED_SEARCHING_V01 = 0x02, 
  NAS_REGISTRATION_DENIED_V01 = 0x03, 
  NAS_REGISTRATION_UNKNOWN_V01 = 0x04, 
  NAS_REGISTRATION_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_registration_state_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_CS_ATTACH_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_CS_UNKNOWN_V01 = 0x00, 
  NAS_CS_ATTACHED_V01 = 0x01, 
  NAS_CS_DETACHED_V01 = 0x02, 
  NAS_CS_ATTACH_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_cs_attach_state_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_PS_ATTACH_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_PS_UNKNOWN_V01 = 0x00, 
  NAS_PS_ATTACHED_V01 = 0x01, 
  NAS_PS_DETACHED_V01 = 0x02, 
  NAS_PS_ATTACH_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_ps_attach_state_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SELECTED_NETWORK_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SELECTED_NETWORK_UNKNOWN_V01 = 0x00, 
  NAS_SELECTED_NETWORK_3GPP2_V01 = 0x01, 
  NAS_SELECTED_NETWORK_3GPP_V01 = 0x02, 
  NAS_SELECTED_NETWORK_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_selected_network_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_registration_state_enum_v01 registration_state;
  /**<   Registration state of the mobile. Values: \n
       - 0x00 -- NOT_REGISTERED        -- Not registered; mobile is not currently 
                                          searching for a new network to provide 
                                          service \n
       - 0x01 -- REGISTERED            -- Registered with a network \n
       - 0x02 -- NOT_REGISTERED_SEARCHING -- Not registered, but mobile is currently 
                                             searching for a new network to provide 
                                             service \n
       - 0x03 -- REGISTRATION_DENIED   -- Registration denied by the visible 
                                          network \n
       - 0x04 -- REGISTRATION_UNKNOWN  -- Registration state is unknown
  */

  nas_cs_attach_state_enum_v01 cs_attach_state;
  /**<   Circuit-switched domain attach state of the mobile. Values: \n
       - 0x00 -- CS_UNKNOWN  -- Unknown or not applicable \n
       - 0x01 -- CS_ATTACHED -- Attached \n
       - 0x02 -- CS_DETACHED -- Detached
  */

  nas_ps_attach_state_enum_v01 ps_attach_state;
  /**<   Packet-switched domain attach state of the mobile. Values: \n
       - 0x00 -- PS_UNKNOWN  -- Unknown or not applicable \n
       - 0x01 -- PS_ATTACHED -- Attached \n
       - 0x02 -- PS_DETACHED -- Detached
  */

  nas_selected_network_enum_v01 selected_network;
  /**<   Type of selected radio access network. Values: \n
       - 0x00 -- SELECTED_NETWORK_UNKNOWN -- Unknown \n
       - 0x01 -- SELECTED_NETWORK_3GPP2   -- 3GPP2 network \n
       - 0x02 -- SELECTED_NETWORK_3GPP    -- 3GPP network
  */

  uint32_t radio_if_len;  /**< Must be set to # of elements in radio_if */
  nas_radio_if_enum_v01 radio_if[NAS_RADIO_IF_LIST_MAX_V01];
  /**<   Radio interface currently in use. Values: \n
       -0x00 -- RADIO_IF_NO_SVC      -- None (no service) \n
       -0x01 -- RADIO_IF_CDMA_1X     -- 
        cdma2000\textsuperscript{\textregistered} 1X             \n
       -0x02 -- RADIO_IF_CDMA_1XEVDO -- 
        cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       -0x03 -- RADIO_IF_AMPS        -- AMPS \n
       -0x04 -- RADIO_IF_GSM         -- GSM \n
       -0x05 -- RADIO_IF_UMTS        -- UMTS \n
       -0x08 -- RADIO_IF_LTE         -- LTE 
  */
}nas_serving_system_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mobile_country_code;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.*/

  uint16_t mobile_network_code;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.*/

  char network_description[NAS_NETWORK_DESCRIPTION_MAX_V01 + 1];
  /**<   An optional string containing the network name or description.*/
}nas_plmn_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t sid;
  /**<   System ID. */

  uint16_t nid;
  /**<   Network ID.*/
}nas_cdma_system_id_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t base_id;
  /**<   Base station identification number.*/

  int32_t base_lat;
  /**<   
    Base station latitude in units of 0.25 sec, expressed as a two's
    complement signed number with positive numbers signifying North
    latitudes.
  */

  int32_t base_long;
  /**<   
    Base station longitude in units of 0.25 sec, expressed as a two's
    complement signed number with positive numbers signifying East
    longitude.
  */
}nas_cdma_base_station_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_ROAMING_INDICATOR_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_ROAMING_IND_ON_V01 = 0x00, 
  NAS_ROAMING_IND_OFF_V01 = 0x01, 
  NAS_ROAMING_IND_FLASHING_V01 = 0x02, 
  NAS_ROAMING_INDICATOR_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_roaming_indicator_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_radio_if_enum_v01 radio_if;
  /**<   Radio interface currently in use. Values: \n
       -0x01 -- RADIO_IF_CDMA_1X     -- 
        cdma2000\textsuperscript{\textregistered} 1X             \n
       -0x02 -- RADIO_IF_CDMA_1XEVDO -- 
        cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       -0x03 -- RADIO_IF_AMPS        -- AMPS \n
       -0x04 -- RADIO_IF_GSM         -- GSM \n
       -0x05 -- RADIO_IF_UMTS        -- UMTS \n
       -0x08 -- RADIO_IF_LTE         -- LTE
  */

  nas_roaming_indicator_enum_v01 roaming_indicator;
  /**<   
    Roaming indicator. Values: \n
    -0x00 -- ROAMING_IND_ON                       -- Roaming \n
    -0x01 -- ROAMING_IND_OFF                      -- Home

    Values from 2 onward are applicable only for 3GPP2. Refer to 
    \hyperref[S4]{[S4]} for the meanings of these values.
  */
}nas_roaming_indicator_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t lp_sec;
  /**<   Number of leap seconds since the start of CDMA system time.
  */

  int8_t ltm_offset;
  /**<    Offset of local time from system time in units of 30 min. The value in
        this field conveys the offset as an 8-bit two's complement number.
  */

  uint8_t daylt_savings;
  /**<   Daylight saving indicator. Values: \n
       - 0x00 -- OFF (daylight saving not in effect) \n
       - 0x01 -- ON (daylight saving in effect)
  */
}nas_3gpp_time_zone_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_DATA_CAPABILITES_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_DATA_CAPABILITIES_GPRS_V01 = 0x01, 
  NAS_DATA_CAPABILITIES_EDGE_V01 = 0x02, 
  NAS_DATA_CAPABILITIES_HSDPA_V01 = 0x03, 
  NAS_DATA_CAPABILITIES_HSUPA_V01 = 0x04, 
  NAS_DATA_CAPABILITIES_WCDMA_V01 = 0x05, 
  NAS_DATA_CAPABILITIES_CDMA_V01 = 0x06, 
  NAS_DATA_CAPABILITIES_EVDO_REV_O_V01 = 0x07, 
  NAS_DATA_CAPABILITIES_EVDO_REV_A_V01 = 0x08, 
  NAS_DATA_CAPABILITIES_GSM_V01 = 0x09, 
  NAS_DATA_CAPABILITIES_EVDO_REV_B_V01 = 0x0A, 
  NAS_DATA_CAPABILITIES_LTE_V01 = 0x0B, 
  NAS_DATA_CAPABILITIES_HSDPA_PLUS_V01 = 0x0C, 
  NAS_DATA_CAPABILITIES_DC_HSDPA_PLUS_V01 = 0x0D, 
  NAS_DATA_CAPABILITES_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_data_capabilites_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t srv_status;
  /**<   Service status. Values: \n
       - 0x00 -- No service \n
       - 0x01 -- Limited service \n
       - 0x02 -- Service available \n
       - 0x03 -- Limited regional service \n
       - 0x04 -- MS in power save or deep sleep
  */

  uint8_t srv_capability;
  /**<   System's service capability. Values: \n
      - 0x00 -- No service \n
      - 0x01 -- Circuit-switched only \n
      - 0x02 -- Packet-switched only \n
      - 0x03 -- Circuit-switched and-packet switched \n
      - 0x04 -- MS found the right system but not yet registered/attached 
  */

  uint8_t hdr_srv_status;
  /**<   HDR service status. Values: \n
      - 0x00 -- No service \n
      - 0x01 -- Limited service \n
      - 0x02 -- Service available \n
      - 0x03 -- Limited regional service \n
      - 0x04 -- MS in power save or deep sleep
  */

  uint8_t hdr_hybrid;
  /**<   HDR hybrid information. Values: \n
      - 0x00 -- System is not hybrid \n
      - 0x01 -- System is hybrid
  */

  uint8_t is_sys_forbidden;
  /**<   Forbidden system information. Values: \n
      - 0x00 -- System is not a forbidden system \n
      - 0x01 -- System is a forbidden system
  */
}nas_ss_detailed_service_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  MCC */
  uint16_t mcc;
  /**<   Mobile country code. 
  */

  /*  imsi_11_12 */
  uint8_t imsi_11_12;
  /**<   IMSI_11_12. 
  */
}nas_cdma_system_id_ext_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_HDR_PERSONALITY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_HDR_PERSONALITY_UNKNOWN_V01 = 0x00, 
  NAS_HDR_PERSONALITY_HRPD_V01 = 0x01, 
  NAS_HDR_PERSONALITY_EHRPD_V01 = 0x02, 
  NAS_HDR_PERSONALITY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_hdr_personality_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_CELL_ACCESS_STATUS_E_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_CELL_ACCESS_NORMAL_ONLY_V01 = 0x00, /**<  Cell access is allowed for normal calls only \n   */
  NAS_CELL_ACCESS_EMERGENCY_ONLY_V01 = 0x01, /**<  Cell access is allowed for emergency calls only \n   */
  NAS_CELL_ACCESS_NO_CALLS_V01 = 0x02, /**<  Cell access is not allowed for any call type \n   */
  NAS_CELL_ACCESS_ALL_CALLS_V01 = 0x03, /**<  Cell access is allowed for all call types \n   */
  NAS_CELL_ACCESS_UNKNOWN_V01 = -1, /**<  Cell access type is unknown   */
  NAS_CELL_ACCESS_STATUS_E_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_cell_access_status_e_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_cell_access_status_e_type_v01 cs_bar_status;
  /**<   
     Call barring status for circuit-switched calls. Values: \n
    - 0x00 -- NAS_CELL_ACCESS_NORMAL_ONLY     -- Cell access is allowed for normal calls only \n
    - 0x01 -- NAS_CELL_ACCESS_EMERGENCY_ ONLY -- Cell access is allowed for emergency calls only \n
    - 0x02 -- NAS_CELL_ACCESS_NO_CALLS        -- Cell access is not allowed for any call type \n
    - 0x03 -- NAS_CELL_ACCESS_ALL_CALLS       -- Cell access is allowed for all call types \n
    -   -1 -- NAS_CELL_ACCESS_UNKNOWN         -- Cell access type is unknown
  */

  nas_cell_access_status_e_type_v01 ps_bar_status;
  /**<  
     Call barring status for packet-switched calls. Values: \n
    - 0x00 -- NAS_CELL_ACCESS_NORMAL_ONLY     -- Cell access is allowed for normal calls only \n
    - 0x01 -- NAS_CELL_ACCESS_EMERGENCY_ ONLY -- Cell access is allowed for emergency calls only \n
    - 0x02 -- NAS_CELL_ACCESS_NO_CALLS        -- Cell access is not allowed for any call type \n
    - 0x03 -- NAS_CELL_ACCESS_ALL_CALLS       -- Cell access is allowed for all call types \n
    -   -1 -- NAS_CELL_ACCESS_UNKNOWN         -- Cell access type is unknown
  */
}nas_gw_sys_info3_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_HS_SUPPORT_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SYS_HS_IND_HSDPA_HSUPA_UNSUPP_CELL_V01 = 0x00, 
  SYS_HS_IND_HSDPA_SUPP_CELL_V01 = 0x01, 
  SYS_HS_IND_HSUPA_SUPP_CELL_V01 = 0x02, 
  SYS_HS_IND_HSDPA_HSUPA_SUPP_CELL_V01 = 0x03, 
  SYS_HS_IND_HSDPAPLUS_SUPP_CELL_V01 = 0x04, 
  SYS_HS_IND_HSDPAPLUS_HSUPA_SUPP_CELL_V01 = 0x05, 
  SYS_HS_IND_DC_HSDPAPLUS_SUPP_CELL_V01 = 0x06, 
  SYS_HS_IND_DC_HSDPAPLUS_HSUPA_SUPP_CELL_V01 = 0x07, 
  SYS_HS_IND_HSDPAPLUS_64QAM_HSUPA_SUPP_CELL_V01 = 0x08, 
  SYS_HS_IND_HSDPAPLUS_64QAM_SUPP_CELL_V01 = 0x09, 
  NAS_HS_SUPPORT_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_hs_support_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries information regarding the system that currently 
              provides service. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Serving System */
  nas_serving_system_type_v01 serving_system;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */

  /* Optional */
  /*  Roaming Indicator Value */
  uint8_t roaming_indicator_valid;  /**< Must be set to true if roaming_indicator is being passed */
  nas_roaming_indicator_enum_v01 roaming_indicator;
  /**<   
    Roaming indicator. Values: \n
    -0x00 -- ROAMING_IND_ON                       -- Roaming \n
    -0x01 -- ROAMING_IND_OFF                      -- Home    \n
    -0x02 and above -- Operator-defined values
  */

  /* Optional */
  /*  Data Service Capability */
  uint8_t data_capabilities_valid;  /**< Must be set to true if data_capabilities is being passed */
  uint32_t data_capabilities_len;  /**< Must be set to # of elements in data_capabilities */
  nas_data_capabilites_enum_v01 data_capabilities[NAS_DATA_CAPABILITIES_LIST_MAX_V01];
  /**<   List of data capabilities (each is 1 byte) of the current
       serving system. Possible values: \n
       -0x01 -- DATA_CAPABILITIES_GPRS       -- GPRS \n
       -0x02 -- DATA_CAPABILITIES_EDGE       -- EDGE \n
       -0x03 -- DATA_CAPABILITIES_HSDPA      -- HSDPA \n
       -0x04 -- DATA_CAPABILITIES_HSUPA      -- HSUPA \n
       -0x05 -- DATA_CAPABILITIES_WCDMA      -- WCDMA \n
       -0x06 -- DATA_CAPABILITIES_CDMA       -- CDMA \n
       -0x07 -- DATA_CAPABILITIES_EVDO_REV_O -- EV-DO REV 0  \n
       -0x08 -- DATA_CAPABILITIES_EVDO_REV_A -- EV-DO REV A  \n
       -0x09 -- DATA_CAPABILITIES_GSM        -- GSM          \n
       -0x0A -- DATA_CAPABILITIES_EVDO_REV_B -- EV-DO REV B  \n
       -0x0B -- DATA_CAPABILITIES_LTE        -- LTE          \n
       -0x0C -- DATA_CAPABILITIES_HSDPA_PLUS -- HSDPA+       \n
       -0x0D -- DATA_CAPABILITIES_DC_HSDPA_ PLUS -- DC-HSDPA+
  */

  /* Optional */
  /*  Current PLMN */
  uint8_t current_plmn_valid;  /**< Must be set to true if current_plmn is being passed */
  nas_plmn_type_v01 current_plmn;

  /* Optional */
  /*  CDMA System ID */
  uint8_t cdma_system_id_valid;  /**< Must be set to true if cdma_system_id is being passed */
  nas_cdma_system_id_type_v01 cdma_system_id;

  /* Optional */
  /*  CDMA Base Station Information */
  uint8_t cdma_base_station_info_valid;  /**< Must be set to true if cdma_base_station_info is being passed */
  nas_cdma_base_station_info_type_v01 cdma_base_station_info;

  /* Optional */
  /*  Roaming Indicator List */
  uint8_t roaming_indicator_list_valid;  /**< Must be set to true if roaming_indicator_list is being passed */
  uint32_t roaming_indicator_list_len;  /**< Must be set to # of elements in roaming_indicator_list */
  nas_roaming_indicator_type_v01 roaming_indicator_list[NAS_ROAMING_INDICATOR_LIST_MAX_V01];

  /* Optional */
  /*  Default Roaming Indicator */
  uint8_t def_roam_ind_valid;  /**< Must be set to true if def_roam_ind is being passed */
  nas_roaming_indicator_enum_v01 def_roam_ind;
  /**<   
    Roaming indicator. Values: \n
    -0x00 -- ROAMING_IND_ON                       -- Roaming \n
    -0x01 -- ROAMING_IND_OFF                      -- Home

    Values from 2 onward are applicable only for 3GPP2. Refer to 
    \hyperref[S4]{[S4]} for the meanings of these values.
  */

  /* Optional */
  /*  3GGP2 Time Zone */
  uint8_t nas_3gpp_time_zone_valid;  /**< Must be set to true if nas_3gpp_time_zone is being passed */
  nas_3gpp_time_zone_type_v01 nas_3gpp_time_zone;

  /* Optional */
  /*  CDMA P_Rev in Use */
  uint8_t p_rev_in_use_valid;  /**< Must be set to true if p_rev_in_use is being passed */
  uint8_t p_rev_in_use;
  /**<   P_Rev that is currently in use.*/

  /* Optional */
  /*  3GPP Time Zone */
  uint8_t time_zone_valid;  /**< Must be set to true if time_zone is being passed */
  int8_t time_zone;
  /**<   Offset from Universal time, i.e., difference between local
       time and Universal time, in increments of 15 min (signed value).
  */

  /* Optional */
  /*   3GPP Network Daylight Saving Adjustment */
  uint8_t adj_valid;  /**< Must be set to true if adj is being passed */
  uint8_t adj;
  /**<   3GPP network daylight saving adjustment. Values: \n
       - 0x00 -- No adjustment for Daylight Saving Time \n
       - 0x01 -- 1 hr adjustment for Daylight Saving Time \n
       - 0x02 -- 2 hr adjustment for Daylight Saving Time
  */

  /* Optional */
  /*  3GPP Location Area Code */
  uint8_t lac_valid;  /**< Must be set to true if lac is being passed */
  uint16_t lac;
  /**<   Location area code.
  */

  /* Optional */
  /*  3GPP Cell ID */
  uint8_t cell_id_valid;  /**< Must be set to true if cell_id is being passed */
  uint32_t cell_id;
  /**<   3GPP cell ID.
  */

  /* Optional */
  /*  3GPP2 Concurrent Service Info */
  uint8_t ccs_valid;  /**< Must be set to true if ccs is being passed */
  uint8_t ccs;
  /**<   3GPP2 concurrent service information. Values: \n
        - 0x00 -- Concurrent service not available \n
        - 0x01 -- Concurrent service available
  */

  /* Optional */
  /*  3GPP2 PRL Indicator */
  uint8_t prl_ind_valid;  /**< Must be set to true if prl_ind is being passed */
  uint8_t prl_ind;
  /**<   3GPP2 PRL indicator. Values: \n
       - 0x00 -- System not in PRL \n
       - 0x01 -- System is in PRL
  */

  /* Optional */
  /*  Dual Transfer Mode Indication (GSM Only) */
  uint8_t dtm_ind_valid;  /**< Must be set to true if dtm_ind is being passed */
  uint8_t dtm_ind;
  /**<   Dual Transfer mode indication. Values: \n
      - 0x00 -- DTM not supported \n
      - 0x01 -- DTM supported 
  */

  /* Optional */
  /*  Detailed Service Information */
  uint8_t detailed_service_info_valid;  /**< Must be set to true if detailed_service_info is being passed */
  nas_ss_detailed_service_info_type_v01 detailed_service_info;

  /* Optional */
  /*  CDMA System Info */
  uint8_t cdma_system_id_ext_valid;  /**< Must be set to true if cdma_system_id_ext is being passed */
  nas_cdma_system_id_ext_type_v01 cdma_system_id_ext;

  /* Optional */
  /*  HDR Personality */
  uint8_t hdr_personality_valid;  /**< Must be set to true if hdr_personality is being passed */
  nas_hdr_personality_enum_v01 hdr_personality;
  /**<   HDR personality information. Values: \n
      - 0x00 -- Unknown \n
      - 0x01 -- HRPD \n
      - 0x02 -- eHRPD
  */

  /* Optional */
  /*  TAC Information for LTE */
  uint8_t tac_valid;  /**< Must be set to true if tac is being passed */
  uint16_t tac;
  /**<   Tracking area code information for LTE.
  */

  /* Optional */
  /*  Call Barring Status */
  uint8_t call_barring_status_valid;  /**< Must be set to true if call_barring_status is being passed */
  nas_gw_sys_info3_type_v01 call_barring_status;

  /* Optional */
  /*  UMTS Primary Scrambling Code */
  uint8_t umts_psc_valid;  /**< Must be set to true if umts_psc is being passed */
  uint16_t umts_psc;
  /**<   Primary scrambling code.
  */

  /* Optional */
  /*  MNC PCS Digit Include Status */
  uint8_t mnc_includes_pcs_digit_valid;  /**< Must be set to true if mnc_includes_pcs_digit is being passed */
  nas_mnc_pcs_digit_include_status_type_v01 mnc_includes_pcs_digit;

  /* Optional */
  /*  HS Call Status */
  uint8_t hs_call_status_valid;  /**< Must be set to true if hs_call_status is being passed */
  nas_hs_support_enum_type_v01 hs_call_status;
  /**<  
      Call status on high speed (only applicable for WCDMA). Values: \n
      - 0x00 -- SYS_HS_IND_HSDPA_HSUPA_ UNSUPP_CELL    -- HSDPA and HSUPA are unsupported \n
      - 0x01 -- SYS_HS_IND_HSDPA_SUPP_CELL             -- HSDPA is supported \n
      - 0x02 -- SYS_HS_IND_HSUPA_SUPP_CELL             -- HSUPA is supported \n
      - 0x03 -- SYS_HS_IND_HSDPA_HSUPA_SUPP_ CELL      -- HSDPA and HSUPA are supported \n
      - 0x04 -- SYS_HS_IND_HSDPAPLUS_SUPP_ CELL        -- HSDPA+ is supported \n
      - 0x05 -- SYS_HS_IND_HSDPAPLUS_HSUPA_ SUPP_CELL  -- HSDPA+ and HSUPA are supported \n
      - 0x06 -- SYS_HS_IND_DC_HSDPAPLUS_SUPP_ CELL     -- Dual-cell HSDPA+ is supported \n
      - 0x07 -- SYS_HS_IND_DC_HSDPAPLUS_ HSUPA_SUPP_CELL    -- Dual-cell HSDPA+ and HSUPA are supported \n 
      - 0x08 -- SYS_HS_IND_HSDPAPLUS_64QAM_ HSUPA_SUPP_CELL -- Dual-cell HSDPA+, 64 QAM, and HSUPA are supported \n
      - 0x09 -- SYS_HS_IND_HSDPAPLUS_64QAM_ SUPP_CELL       -- Dual-cell HSDPA+ and 64 QAM are supported
    */
}nas_get_serving_system_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t year;
  /**<   Year.*/

  uint8_t month;
  /**<   Month.*/

  uint8_t day;
  /**<   Day.*/

  uint8_t hour;
  /**<   Hour.*/

  uint8_t minute;
  /**<   Minute.*/

  uint8_t second;
  /**<   Second.*/

  int8_t time_zone;
  /**<   Offset from Universal time, i.e., difference between local time
       and Universal time, in increments of 15 min (signed value).
  */
}nas_universal_time_and_local_time_zone_3gpp_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the current serving system registration
              state and/or radio technology. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Serving System */
  nas_serving_system_type_v01 serving_system;

  /* Optional */
  /*  Roaming Indicator Value */
  uint8_t roaming_indicator_valid;  /**< Must be set to true if roaming_indicator is being passed */
  nas_roaming_indicator_enum_v01 roaming_indicator;
  /**<   
    Roaming indicator. Values: \n
    - 0x00 -- ROAMING_IND_ON       -- Roaming   \n
    - 0x01 -- ROAMING_IND_OFF      -- Home      \n
    - 0x02 -- ROAMING_IND_FLASHING -- Flashing  \n
    - 0x03 and above -- Operator-defined values
  */

  /* Optional */
  /*  Data Service Capability */
  uint8_t data_capabilities_valid;  /**< Must be set to true if data_capabilities is being passed */
  uint32_t data_capabilities_len;  /**< Must be set to # of elements in data_capabilities */
  nas_data_capabilites_enum_v01 data_capabilities[NAS_DATA_CAPABILITIES_LIST_MAX_V01];
  /**<   List of data capabilities (each is 1 byte) of the current
       serving system. Possible values: \n
       -0x01 -- DATA_CAPABILITIES_GPRS       -- GPRS \n
       -0x02 -- DATA_CAPABILITIES_EDGE       -- EDGE \n
       -0x03 -- DATA_CAPABILITIES_HSDPA      -- HSDPA \n
       -0x04 -- DATA_CAPABILITIES_HSUPA      -- HSUPA \n
       -0x05 -- DATA_CAPABILITIES_WCDMA      -- WCDMA \n
       -0x06 -- DATA_CAPABILITIES_CDMA       -- CDMA \n
       -0x07 -- DATA_CAPABILITIES_EVDO_REV_O -- EV-DO REV 0  \n
       -0x08 -- DATA_CAPABILITIES_EVDO_REV_A -- EV-DO REV A  \n
       -0x09 -- DATA_CAPABILITIES_GSM        -- GSM          \n
       -0x0A -- DATA_CAPABILITIES_EVDO_REV_B -- EV-DO REV B  \n
       -0x0B -- DATA_CAPABILITIES_LTE        -- LTE          \n
       -0x0C -- DATA_CAPABILITIES_HSDPA_PLUS -- HSDPA+       \n
       -0x0D -- DATA_CAPABILITIES_DC_HSDPA_ PLUS -- DC-HSDPA+
  */

  /* Optional */
  /*  Current PLMN */
  uint8_t current_plmn_valid;  /**< Must be set to true if current_plmn is being passed */
  nas_plmn_type_v01 current_plmn;

  /* Optional */
  /*  CDMA System ID */
  uint8_t cdma_system_id_valid;  /**< Must be set to true if cdma_system_id is being passed */
  nas_cdma_system_id_type_v01 cdma_system_id;

  /* Optional */
  /*  CDMA Base Station Information */
  uint8_t cdma_base_station_info_valid;  /**< Must be set to true if cdma_base_station_info is being passed */
  nas_cdma_base_station_info_type_v01 cdma_base_station_info;

  /* Optional */
  /*  Roaming Indicator List */
  uint8_t roaming_indicator_list_valid;  /**< Must be set to true if roaming_indicator_list is being passed */
  uint32_t roaming_indicator_list_len;  /**< Must be set to # of elements in roaming_indicator_list */
  nas_roaming_indicator_type_v01 roaming_indicator_list[NAS_ROAMING_INDICATOR_LIST_MAX_V01];

  /* Optional */
  /*  Default Roaming Indicator */
  uint8_t def_roam_ind_valid;  /**< Must be set to true if def_roam_ind is being passed */
  nas_roaming_indicator_enum_v01 def_roam_ind;
  /**<   
    Roaming indicator. Values: \n
    -0x00 -- ROAMING_IND_ON                       -- Roaming \n
    -0x01 -- ROAMING_IND_OFF                      -- Home

    Values from 2 onward are applicable only for 3GPP2. Refer to 
    \hyperref[S4]{[S4]} for the meanings of these values.
  */

  /* Optional */
  /*  3GGP2 Time Zone */
  uint8_t nas_3gpp_time_zone_valid;  /**< Must be set to true if nas_3gpp_time_zone is being passed */
  nas_3gpp_time_zone_type_v01 nas_3gpp_time_zone;

  /* Optional */
  /*  CDMA P_Rev in Use */
  uint8_t p_rev_in_use_valid;  /**< Must be set to true if p_rev_in_use is being passed */
  uint8_t p_rev_in_use;
  /**<   P_Rev that is currently in use.
  */

  /* Optional */
  /*  3GPP PLMN Name Flag */
  uint8_t plmn_description_changed_valid;  /**< Must be set to true if plmn_description_changed is being passed */
  uint8_t plmn_description_changed;
  /**<  
       Flag indicating that the 3GPP EONS network description changed. Values: \n
       -0x01 -- PLMN name changed
  */

  /* Optional */
  /*  3GPP Time Zone */
  uint8_t time_zone_valid;  /**< Must be set to true if time_zone is being passed */
  int8_t time_zone;
  /**<   Offset from Universal time, i.e., difference between local
       time and Universal time, in increments of 15 min (signed value).
  */

  /* Optional */
  /*   3GPP Network Daylight Saving Adjustment */
  uint8_t adj_valid;  /**< Must be set to true if adj is being passed */
  uint8_t adj;
  /**<   3GPP network daylight saving adjustment. Values: \n
       - 0x00 -- No adjustment for Daylight Saving Time \n
       - 0x01 -- 1 hr adjustment for Daylight Saving Time \n
       - 0x02 -- 2 hr adjustment for Daylight Saving Time
  */

  /* Optional */
  /*  3GPP Universal Time and Local Time Zone */
  uint8_t universal_time_and_local_time_3gpp_zone_valid;  /**< Must be set to true if universal_time_and_local_time_3gpp_zone is being passed */
  nas_universal_time_and_local_time_zone_3gpp_type_v01 universal_time_and_local_time_3gpp_zone;

  /* Optional */
  /*  3GPP Location Area Code */
  uint8_t lac_valid;  /**< Must be set to true if lac is being passed */
  uint16_t lac;
  /**<   Location area code.
  */

  /* Optional */
  /*  3GPP Cell ID */
  uint8_t cell_id_valid;  /**< Must be set to true if cell_id is being passed */
  uint32_t cell_id;
  /**<   3GPP cell ID.
  */

  /* Optional */
  /*  3GPP2 Concurrent Service Info */
  uint8_t ccs_valid;  /**< Must be set to true if ccs is being passed */
  uint8_t ccs;
  /**<   3GPP2 concurrent service information. Values: \n
        - 0x00 -- Concurrent service not available \n
        - 0x01 -- Concurrent service available
  */

  /* Optional */
  /*  3GPP2 PRL Indicator */
  uint8_t prl_ind_valid;  /**< Must be set to true if prl_ind is being passed */
  uint8_t prl_ind;
  /**<   3GPP2 PRL indicator. Values: \n
       - 0x00 -- System not in PRL \n
       - 0x01 -- System is in PRL
  */

  /* Optional */
  /*  Dual Transfer Mode Indication (GSM Only) */
  uint8_t dtm_ind_valid;  /**< Must be set to true if dtm_ind is being passed */
  uint8_t dtm_ind;
  /**<   Dual Transfer mode indication. Values: \n
      - 0x00 -- DTM not supported \n
      - 0x01 -- DTM supported 
  */

  /* Optional */
  /*  Detailed Service Information */
  uint8_t detailed_service_info_valid;  /**< Must be set to true if detailed_service_info is being passed */
  nas_ss_detailed_service_info_type_v01 detailed_service_info;

  /* Optional */
  /*  CDMA System Info Ext */
  uint8_t cdma_system_id_ext_valid;  /**< Must be set to true if cdma_system_id_ext is being passed */
  nas_cdma_system_id_ext_type_v01 cdma_system_id_ext;

  /* Optional */
  /*  HDR Personality */
  uint8_t hdr_personality_valid;  /**< Must be set to true if hdr_personality is being passed */
  nas_hdr_personality_enum_v01 hdr_personality;
  /**<   HDR personality information. Values: \n
      - 0x00 -- Unknown \n
      - 0x01 -- HRPD \n
      - 0x02 -- eHRPD
  */

  /* Optional */
  /*  TAC Information for LTE */
  uint8_t tac_valid;  /**< Must be set to true if tac is being passed */
  uint16_t tac;
  /**<   Tracking area code information for LTE.
  */

  /* Optional */
  /*  Call Barring Status */
  uint8_t call_barring_status_valid;  /**< Must be set to true if call_barring_status is being passed */
  nas_gw_sys_info3_type_v01 call_barring_status;

  /* Optional */
  /*  PLMN Change Status */
  uint8_t srv_sys_no_change_valid;  /**< Must be set to true if srv_sys_no_change is being passed */
  uint8_t srv_sys_no_change;
  /**<   Flag used to notify clients that a request to select a network ended 
      with no change in the PLMN. Values: \n
      - 0x01 -- No change in serving system information
  */

  /* Optional */
  /*  UMTS Primary Scrambling Code */
  uint8_t umts_psc_valid;  /**< Must be set to true if umts_psc is being passed */
  uint16_t umts_psc;
  /**<   Primary scrambling code.
  */

  /* Optional */
  /*  MNC PCS Digit Include Status */
  uint8_t mnc_includes_pcs_digit_valid;  /**< Must be set to true if mnc_includes_pcs_digit is being passed */
  nas_mnc_pcs_digit_include_status_type_v01 mnc_includes_pcs_digit;

  /* Optional */
  /*  HS Call Status */
  uint8_t hs_call_status_valid;  /**< Must be set to true if hs_call_status is being passed */
  nas_hs_support_enum_type_v01 hs_call_status;
  /**<  
      Call status on high speed (only applicable for WCDMA). Values: \n
      - 0x00 -- SYS_HS_IND_HSDPA_HSUPA_ UNSUPP_CELL    -- HSDPA and HSUPA are unsupported \n
      - 0x01 -- SYS_HS_IND_HSDPA_SUPP_CELL             -- HSDPA is supported \n
      - 0x02 -- SYS_HS_IND_HSUPA_SUPP_CELL             -- HSUPA is supported \n
      - 0x03 -- SYS_HS_IND_HSDPA_HSUPA_SUPP_ CELL      -- HSDPA and HSUPA are supported \n
      - 0x04 -- SYS_HS_IND_HSDPAPLUS_SUPP_ CELL        -- HSDPA+ is supported \n
      - 0x05 -- SYS_HS_IND_HSDPAPLUS_HSUPA_ SUPP_CELL  -- HSDPA+ and HSUPA are supported \n
      - 0x06 -- SYS_HS_IND_DC_HSDPAPLUS_SUPP_ CELL     -- Dual-cell HSDPA+ is supported \n
      - 0x07 -- SYS_HS_IND_DC_HSDPAPLUS_ HSUPA_SUPP_CELL    -- Dual-cell HSDPA+ and HSUPA are supported \n 
      - 0x08 -- SYS_HS_IND_HSDPAPLUS_64QAM_ HSUPA_SUPP_CELL -- Dual-cell HSDPA+, 64 QAM, and HSUPA are supported \n
      - 0x09 -- SYS_HS_IND_HSDPAPLUS_64QAM_ SUPP_CELL       -- Dual-cell HSDPA+ and 64 QAM are supported
    */
}nas_serving_system_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_home_network_req_msg_v01;

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_NETWORK_DESC_DISPLAY_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_NETWORK_DESC_DISP_FALSE_V01 = 0x00, 
  NAS_NETWORK_DESC_DISP_TRUE_V01 = 0x01, 
  NAS_NETWORK_DESC_DISP_UNKOWN_V01 = 0xFF, 
  NAS_NETWORK_DESC_DISP_UNKNOWN_V01 = 0xFF, 
  NAS_NETWORK_DESC_DISPLAY_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_network_desc_display_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_NETWORK_DESC_ENCODING_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_NETWORK_DESC_ENCODING_OCTECT_UNSPECIFIED_V01 = 0x00, 
  NAS_NETWORK_DESC_ENCODING_7_BIT_ASCII_V01 = 0x02, 
  NAS_NETWORK_DESC_ENCODING_UNICODE_V01 = 0x04, 
  NAS_NETWORK_DESC_ENCODING_GSM_7_BIT_DEFAULT_V01 = 0x09, 
  NAS_NETWORK_DESC_ENCODING_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_network_desc_encoding_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  MCC */
  uint16_t mcc;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.
  */

  /*  MNC */
  uint16_t mnc;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.
  */
}nas_plmn_id_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  MCC and MNC */
  nas_plmn_id_type_v01 mcc_mnc;

  /*  Network name display status */
  nas_network_desc_display_enum_type_v01 network_desc_display;
  /**<   
      Whether the network name is to be conditionally displayed: \n
      - 0x00 -- Do not display \n
      - 0x01 -- Display \n
      - 0xFF -- Unknown \n
      Note: This value is ignored if the network_description_len
            is zero.
  */

  /*  Network description encoding */
  nas_network_desc_encoding_type_v01 network_desc_encoding;
  /**<   Encoding of the network description. 
      Refer to \hyperref[S4]{[S4]} Table 9.1.1 for list of all defined values. 
      Common (but not all) values include: \n
      - 0x00 -- Octet, unspecified \n
      - 0x02 -- 7-bit ASCII \n
      - 0x04 -- Unicode (refer to \hyperref[S10]{[S10]}) \n
      - 0x09 -- GSM 7-bit default (refer to \hyperref[S8]{[S8]}) \n
      Note: This value is ignored if the network_description_len
            is zero. If the encoding type is not recognized the 
            network_description is ignored.
   */

  /*  Network description 
 Length of network description string that follows. 
      If the network name is unknown or not included, the length
      is 0.
   */
  uint32_t network_desc_len;  /**< Must be set to # of elements in network_desc */
  uint8_t network_desc[NAS_NETWORK_DESCRIPTION_MAX_V01];
  /**<   Length of network description string that follows. 
       If the network name is unknown or not included, the length
       is 0.
  */
}nas_3gpp2_home_network_ext_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t is_3gpp_network;
  /**<   TRUE if TLV 0x01 corresponds to a 3GPP network; otherwise FALSE. */

  uint8_t mnc_includes_pcs_digit;
  /**<   This field is used to interpret the length of the mobile_network_code 
       reported in TLV 0x01. Values: \n

       - TRUE  -- MNC is a three-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 090  \n
       - FALSE -- MNC is a two-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 90 \n
       Note: This value is ignored if is_3gpp_network is FALSE.
  */
}nas_3gpp_mcs_digit_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves information about the home network of the device. */
typedef struct {

  /* Mandatory */
  /*  Home Network */
  nas_plmn_type_v01 home_network;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. 
 Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */

  /* Optional */
  /*  Home System ID */
  uint8_t home_system_id_valid;  /**< Must be set to true if home_system_id is being passed */
  nas_cdma_system_id_type_v01 home_system_id;

  /* Optional */
  /*  3GPP2 Home Network Ext */
  uint8_t nas_3gpp2_home_network_ext_valid;  /**< Must be set to true if nas_3gpp2_home_network_ext is being passed */
  nas_3gpp2_home_network_ext_type_v01 nas_3gpp2_home_network_ext;

  /* Optional */
  /*  3GPP Home Network MNC (includes PCS digit status) */
  uint8_t nas_3gpp_mcs_include_digit_valid;  /**< Must be set to true if nas_3gpp_mcs_include_digit is being passed */
  nas_3gpp_mcs_digit_type_v01 nas_3gpp_mcs_include_digit;
}nas_get_home_network_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_preferred_networks_req_msg_v01;

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mobile_country_code;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.
  */

  uint16_t mobile_network_code;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.
  */

  uint16_t radio_access_technology;
  /**<    RAT as a bitmask (bit count begins from zero). Values: \n
        - Bit 15 -- UMTS \n
        - Bit 14 -- LTE \n
        - Bit 7 -- GSM \n
        - Bit 6 -- GSM compact \n
        - All bits set to 0 -- No access technology is available from the device
  */
}nas_3gpp_preferred_networks_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the list of preferred networks from the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  3GPP Preferred Networks** */
  uint8_t nas_3gpp_preferred_networks_valid;  /**< Must be set to true if nas_3gpp_preferred_networks is being passed */
  uint32_t nas_3gpp_preferred_networks_len;  /**< Must be set to # of elements in nas_3gpp_preferred_networks */
  nas_3gpp_preferred_networks_type_v01 nas_3gpp_preferred_networks[NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01];

  /* Optional */
  /*  Static 3GPP Preferred Networks** */
  uint8_t static_3gpp_preferred_networks_valid;  /**< Must be set to true if static_3gpp_preferred_networks is being passed */
  uint32_t static_3gpp_preferred_networks_len;  /**< Must be set to # of elements in static_3gpp_preferred_networks */
  nas_3gpp_preferred_networks_type_v01 static_3gpp_preferred_networks[NAS_STATIC_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01];

  /* Optional */
  /*  3GPP Preferred Networks MNC (includes PCS digit status) */
  uint8_t nas_3gpp_mnc_includes_pcs_digit_valid;  /**< Must be set to true if nas_3gpp_mnc_includes_pcs_digit is being passed */
  uint32_t nas_3gpp_mnc_includes_pcs_digit_len;  /**< Must be set to # of elements in nas_3gpp_mnc_includes_pcs_digit */
  nas_mnc_pcs_digit_include_status_type_v01 nas_3gpp_mnc_includes_pcs_digit[NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01];

  /* Optional */
  /*  Static 3GPP Preferred Networks MNC (includes PCS digit status) */
  uint8_t static_3gpp_mnc_includes_pcs_digit_valid;  /**< Must be set to true if static_3gpp_mnc_includes_pcs_digit is being passed */
  uint32_t static_3gpp_mnc_includes_pcs_digit_len;  /**< Must be set to # of elements in static_3gpp_mnc_includes_pcs_digit */
  nas_mnc_pcs_digit_include_status_type_v01 static_3gpp_mnc_includes_pcs_digit[NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01];
}nas_get_preferred_networks_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Writes the specified list of preferred networks to the device. */
typedef struct {

  /* Optional */
  /*  3GPP Preferred Networks** */
  uint8_t nas_3gpp_preferred_networks_valid;  /**< Must be set to true if nas_3gpp_preferred_networks is being passed */
  uint32_t nas_3gpp_preferred_networks_len;  /**< Must be set to # of elements in nas_3gpp_preferred_networks */
  nas_3gpp_preferred_networks_type_v01 nas_3gpp_preferred_networks[NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01];

  /* Optional */
  /*  3GPP Preferred Networks MNC (includes PCS digit status) */
  uint8_t nas_3gpp_mnc_includes_pcs_digit_valid;  /**< Must be set to true if nas_3gpp_mnc_includes_pcs_digit is being passed */
  uint32_t nas_3gpp_mnc_includes_pcs_digit_len;  /**< Must be set to # of elements in nas_3gpp_mnc_includes_pcs_digit */
  nas_mnc_pcs_digit_include_status_type_v01 nas_3gpp_mnc_includes_pcs_digit[NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01];

  /* Optional */
  /*  Clear Previous Preferred Networks List */
  uint8_t clear_prev_preferred_networks_valid;  /**< Must be set to true if clear_prev_preferred_networks is being passed */
  uint8_t clear_prev_preferred_networks;
  /**<   Indicates whether to add padding to the incoming preferred networks list 
       and to fully clear out the previous preferred networks list.
    */
}nas_set_preferred_networks_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Writes the specified list of preferred networks to the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_set_preferred_networks_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_forbidden_networks_req_msg_v01;

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mobile_country_code;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.
  */

  uint16_t mobile_network_code;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.
  */
}nas_3gpp_forbidden_networks_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the list of forbidden networks from the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  3GPP Forbidden Networks** */
  uint8_t nas_3gpp_forbidden_networks_valid;  /**< Must be set to true if nas_3gpp_forbidden_networks is being passed */
  uint32_t nas_3gpp_forbidden_networks_len;  /**< Must be set to # of elements in nas_3gpp_forbidden_networks */
  nas_3gpp_forbidden_networks_type_v01 nas_3gpp_forbidden_networks[NAS_3GPP_FORBIDDEN_NETWORKS_LIST_MAX_V01];
}nas_get_forbidden_networks_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Writes the specified list of forbidden networks to the device. */
typedef struct {

  /* Optional */
  /*  3GPP Forbidden Networks** */
  uint8_t nas_3gpp_forbidden_networks_valid;  /**< Must be set to true if nas_3gpp_forbidden_networks is being passed */
  uint32_t nas_3gpp_forbidden_networks_len;  /**< Must be set to # of elements in nas_3gpp_forbidden_networks */
  nas_3gpp_forbidden_networks_type_v01 nas_3gpp_forbidden_networks[NAS_3GPP_FORBIDDEN_NETWORKS_LIST_MAX_V01];
}nas_set_forbidden_networks_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Writes the specified list of forbidden networks to the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_set_forbidden_networks_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_accolc_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the Access Overload Class (ACCOLC) of the device. */
typedef struct {

  /* Mandatory */
  /*  Access Overload Class */
  uint8_t accolc;
  /**<   An 8-bit integer representation of the ACCOLC. 
       Range: 0 to 15 (0x00 to 0x0F).
  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_get_accolc_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  char spc[NAS_SPC_MAX_V01];
  /**<   Service programming code in ASCII format (digits 0 to 9 only).*/

  uint8_t accolc;
  /**<   An 8-bit integer representation of the ACCOLC. 
       Range: 0 to 15 (0x00 to 0x0F).
  */
}nas_accolc_set_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets the ACCOLC of the device. */
typedef struct {

  /* Mandatory */
  /*  Access Overload Class */
  nas_accolc_set_type_v01 accolc_set;
}nas_set_accolc_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets the ACCOLC of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_set_accolc_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_device_config_req_msg_v01;

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t reg_home_sid;
  /**<   Register on home system. Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  uint8_t reg_foreign_sid;
  /**<   Register on foreign system. Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  uint8_t reg_foreign_nid;
  /**<   Register on foreign network. Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */
}nas_registration_parameters_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t state;
  /**<   HDR custom configuration for session control protocol. Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable; enable may only be specified if Force HDR Revision 
                 is set to Disable
  */

  uint32_t protocol_mask;
  /**<   Protocol subtype bitmask. Values: \n
       - Bit 0 -- Subtype 2 physical layer \n
       - Bit 1 -- Enhanced CCMAC \n
       - Bit 2 -- Enhanced ACMAC \n
       - Bit 3 -- Enhanced FTCMAC \n
       - Bit 4 -- Subtype 3 RTCMAC \n
       - Bit 5 -- Subtype 1 RTCMAC \n
       - Bit 6 -- Enhanced idle \n
       - Bit 7 -- Generic multimode-capable disc port

       \vspace{3pt}
       All unlisted bits are reserved for future use and are ignored.
  */

  uint32_t broadcast_mask;
  /**<   Broadcast subtype bitmask. Values: \n
       - Bit 0 -- Generic broadcast enabled

       \vspace{3pt}
       All unlisted bits are reserved for future use and are ignored.
  */

  uint32_t application_mask;
  /**<   Application subtype bitmask. Values: \n
       - Bit 0 -- SN multiflow packet application \n
       - Bit 1 -- SN enhanced multiflow packet application

       \vspace{3pt}
       All unlisted bits are reserved for future use and are ignored.
  */
}nas_hdr_scp_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_ROAM_CONFIG_PREF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_ROAM_CONFIG_PREF_AUTO_V01 = 0x00, 
  NAS_ROAM_CONFIG_PREF_HOME_ONLY_V01 = 0x01, 
  NAS_ROAM_CONFIG_PREF_ROAM_ONLY_V01 = 0x02, 
  NAS_ROAM_CONFIG_PREF_HOME_AND_AFFILIATE_V01 = 0x03, 
  NAS_ROAM_CONFIG_PREF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_roam_config_pref_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_FORCE_HDRSCP_CONFIG_AT_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_HDR_REV0_PROTOCOLS_ONLY_V01 = 0x00, 
  NAS_HDR_REVA_PROTOCOLS_MFPA_V01 = 0x01, 
  NAS_HDR_REVA_PROTOCOLS_MFPA_EMPA_V01 = 0x02, 
  NAS_HDR_REVB_PROTOCOLS_MMPA_V01 = 0x03, 
  NAS_HDR_REVA_PROTOCOLS_EHRPD_V01 = 0x04, 
  NAS_HDR_REVB_PROTOCOLS_EHRPD_V01 = 0x05, 
  NAS_FORCE_HDRSCP_CONFIG_AT_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_force_hdrscp_config_at_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the network-related configuration setting of the 
              device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. 
 Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */

  /* Optional */
  /*  Slot Cycle Index* */
  uint8_t sci_valid;  /**< Must be set to true if sci is being passed */
  uint8_t sci;
  /**<   Slot cycle index (refer to \hyperref[S4]{[S4]} Section 6.6.2.1).
  */

  /* Optional */
  /*  Station Class Mark* */
  uint8_t scm_valid;  /**< Must be set to true if scm is being passed */
  uint8_t scm;
  /**<   Station class mark (refer to \hyperref[S4]{[S4]} Section 6.3.3).
  */

  /* Optional */
  /*  Registration Parameters* */
  uint8_t registration_parameters_valid;  /**< Must be set to true if registration_parameters is being passed */
  nas_registration_parameters_type_v01 registration_parameters;

  /* Optional */
  /*  Force HDR Revision* */
  uint8_t force_rev0_valid;  /**< Must be set to true if force_rev0 is being passed */
  uint8_t force_rev0;
  /**<   Force Rev0. Values: \n
       - 0x00 -- Disabled \n
       - 0x01 -- Enabled \n
    Note: This TLV is now DISCONTINUED, and is present here as a
          placeholder only for existing clients referencing this TLV.
  */

  /* Optional */
  /*  HDR SCP Custom Config* */
  uint8_t hdr_scp_config_valid;  /**< Must be set to true if hdr_scp_config is being passed */
  nas_hdr_scp_config_type_v01 hdr_scp_config;
  /**<   \n
    Note: This TLV is now DISCONTINUED, and is present here as a
          placeholder only for existing clients referencing this TLV.
  */

  /* Optional */
  /*  Roam Preference* */
  uint8_t roam_pref_valid;  /**< Must be set to true if roam_pref is being passed */
  nas_roam_config_pref_enum_v01 roam_pref;
  /**<   Roaming preference. Values: \n
       - 0x00 -- ROAM_CONFIG_PREF_AUTO                -- Acquire systems regardless of roaming status \n
       - 0x01 -- ROAM_CONFIG_PREF_HOME_ONLY           -- Acquire home systems only \n
       - 0x02 -- ROAM_CONFIG_PREF_ROAM_ONLY           -- Acquire nonhome systems only \n
       - 0x03 -- ROAM_CONFIG_PREF_HOME_AND_ AFFILIATE -- Acquire home and affiliated roaming systems only
  */

  /* Optional */
  /*  Force HDR SCP AT Config */
  uint8_t force_hdrscp_config_at_valid;  /**< Must be set to true if force_hdrscp_config_at is being passed */
  nas_force_hdrscp_config_at_enum_type_v01 force_hdrscp_config_at;
  /**<   
      Values: \n
        -0x00 -- HDR Rev0 Protocols only  \n
        -0x01 -- HDR RevA Protocols with MFPA \n
        -0x02 -- HDR RevA Protocols with MFPA and EMPA \n
        -0x03 -- HDR RevB Protocols with MMPA \n
        -0x04 -- HDR RevA Protocols with eHRPD \n
        -0x05 -- HDR RevB Protocols with eHRPD
  */
}nas_get_device_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets network-related configuration settings of the device. */
typedef struct {

  /* Optional */
  /*  Service Programming Code* */
  uint8_t spc_valid;  /**< Must be set to true if spc is being passed */
  char spc[NAS_SPC_MAX_V01];
  /**<   Service programming code in ASCII format (digits 0 to 9 only).
  */

  /* Optional */
  /*  Force HDR Revision* */
  uint8_t force_hdr_rev0_valid;  /**< Must be set to true if force_hdr_rev0 is being passed */
  uint8_t force_hdr_rev0;
  /**<   Force Rev0. Values: \n
    - 0x00 -- Disable \n
    - 0x01 -- Enable; enable may only be specified if HDR SCP Custom Config
              state is set to Disable \n
    Note: This TLV is now DISCONTINUED, and is present here as a
          placeholder only for existing clients referencing this TLV.
  */

  /* Optional */
  /*  HDR SCP Custom Config* */
  uint8_t hdr_scp_config_valid;  /**< Must be set to true if hdr_scp_config is being passed */
  nas_hdr_scp_config_type_v01 hdr_scp_config;
  /**<   \n
    Note: This TLV is now DISCONTINUED, and is present here as a
          placeholder only for existing clients referencing this TLV.
  */

  /* Optional */
  /*  Roam Preference* */
  uint8_t roam_pref_valid;  /**< Must be set to true if roam_pref is being passed */
  nas_roam_config_pref_enum_v01 roam_pref;
  /**<   Roaming preference. Values: \n
       - 0x00 -- ROAM_CONFIG_PREF_AUTO                -- Acquire systems regardless of roaming status \n
       - 0x01 -- ROAM_CONFIG_PREF_HOME_ONLY           -- Acquire home systems only \n
       - 0x02 -- ROAM_CONFIG_PREF_ROAM_ONLY           -- Acquire nonhome systems only \n
       - 0x03 -- ROAM_CONFIG_PREF_HOME_AND_ AFFILIATE -- Acquire home and affiliated roaming systems only
  */
}nas_set_device_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets network-related configuration settings of the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_set_device_config_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_rf_band_info_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries radio band/channel information regarding the
              system currently providing service.  */
typedef struct {

  /* Mandatory */
  /*  RF Band Information List */
  uint32_t rf_band_info_list_len;  /**< Must be set to # of elements in rf_band_info_list */
  nas_rf_band_info_type_v01 rf_band_info_list[NAS_RF_BAND_INFO_LIST_MAX_V01];

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_get_rf_band_info_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_an_aaa_status_req_msg_v01;

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_AN_AAA_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_AAA_STATUS_FAILED_V01 = 0x00, 
  NAS_AAA_STATUS_SUCCESS_V01 = 0x01, 
  NAS_AAA_STATUS_NO_REQUEST_V01 = 0x02, 
  NAS_AN_AAA_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_an_aaa_status_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the status of the last AN-AAA authentication
              request for the current 1xEV-DO session. */
typedef struct {

  /* Mandatory */
  /*  AN-AAA Authentication Status */
  nas_an_aaa_status_enum_v01 an_aaa_status;
  /**<   Status of the last AN-AAA authentication request, if any, for
       the current 1xEV-DO session. Values: \n
       - 0 -- AAA_STATUS_FAILED     -- Authentication failed \n
       - 1 -- AAA_STATUS_SUCCESS    -- Authentication success \n
       - 2 -- AAA_STATUS_NO_REQUEST -- No authentication requested 
  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_get_an_aaa_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_PRL_PREF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_PRL_PREF_A_SIDE_ONLY_V01 = 0x0001, 
  NAS_PRL_PREF_B_SIDE_ONLY_V01 = 0x0002, 
  NAS_PRL_PREF_ANY_V01 = 0x3FFF, 
  NAS_PRL_PREF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_prl_pref_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_ROAM_PREF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_ROAMING_PREF_OFF_V01 = 0x01, 
  NAS_ROAMING_PREF_NOT_OFF_V01 = 0x02, 
  NAS_ROAMING_PREF_NOT_FLASING_V01 = 0x03, 
  NAS_ROAMING_PREF_ANY_V01 = 0xFF, 
  NAS_ROAM_PREF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_roam_pref_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_NET_SEL_PREF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_NET_SEL_PREF_AUTOMATIC_V01 = 0x00, 
  NAS_NET_SEL_PREF_MANUAL_V01 = 0x01, 
  NAS_NET_SEL_PREF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_net_sel_pref_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SRV_DOMAIN_PREF_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_SRV_DOMAIN_PREF_CS_ONLY_V01 = 0x00, 
  QMI_SRV_DOMAIN_PREF_PS_ONLY_V01 = 0x01, 
  QMI_SRV_DOMAIN_PREF_CS_PS_V01 = 0x02, 
  QMI_SRV_DOMAIN_PREF_PS_ATTACH_V01 = 0x03, 
  QMI_SRV_DOMAIN_PREF_PS_DETACH_V01 = 0x04, 
  QMI_SRV_DOMAIN_PREF_PS_DETACH_NO_PREF_CHANGE_V01 = 0x05, 
  NAS_SRV_DOMAIN_PREF_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_srv_domain_pref_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_GW_ACQ_ORDER_PREF_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01 = 0x00, 
  NAS_GW_ACQ_ORDER_PREF_GSM_WCDMA_V01 = 0x01, 
  NAS_GW_ACQ_ORDER_PREF_WCDMA_GSM_V01 = 0x02, 
  NAS_GW_ACQ_ORDER_PREF_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_gw_acq_order_pref_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SRV_REG_RESTRICTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01 = 0x00, 
  NAS_SRV_REG_RESTRICTION_CAMPED_ONLY_V01 = 0x01, 
  NAS_SRV_REG_RESTRICTION_LIMITED_V01 = 0x02, 
  NAS_SRV_REG_RESTRICTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_srv_reg_restriction_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_USAGE_PREF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_USAGE_UNKNOWN_V01 = 0, /**<  Unknown \n  */
  NAS_USAGE_VOICE_CENTRIC_V01 = 1, /**<  Voice centric \n  */
  NAS_USAGE_DATA_CENTRIC_V01 = 2, /**<  Data centric  */
  NAS_USAGE_PREF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_usage_pref_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_net_sel_pref_enum_v01 net_sel_pref;
  /**<   Specifies one of the following actions: \n
       - 0x00 -- NAS_NET_SEL_PREF_AUTOMATIC -- 
                 Device registers according to its provisioning; 
                 mcc and mnc fields must also contain valid values 
                 if Radio Access Technology (TLV 0x22) is present. 
                 Otherwise, mcc and mnc are ignored. \n
       - 0x01 -- NAS_NET_SEL_PREF_MANUAL -- 
                 Device registers to specified network; 
                 mcc and mnc fields must also contain valid values.
       
       \vspace{3pt}
       All other values are reserved.
  */

  uint16_t mcc;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.
  */

  uint16_t mnc;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.
  */
}nas_net_sel_pref_type_v01;  /* Type */
/**
    @}
  */

typedef uint16_t mode_pref_mask_type_v01;
#define QMI_NAS_RAT_MODE_PREF_CDMA2000_1X_V01 ((mode_pref_mask_type_v01)0x01) 
#define QMI_NAS_RAT_MODE_PREF_CDMA2000_HRPD_V01 ((mode_pref_mask_type_v01)0x02) 
#define QMI_NAS_RAT_MODE_PREF_GSM_V01 ((mode_pref_mask_type_v01)0x04) 
#define QMI_NAS_RAT_MODE_PREF_UMTS_V01 ((mode_pref_mask_type_v01)0x08) 
#define QMI_NAS_RAT_MODE_PREF_LTE_V01 ((mode_pref_mask_type_v01)0x10) 
#define QMI_NAS_RAT_MODE_PREF_TDSCDMA_V01 ((mode_pref_mask_type_v01)0x20) 
/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mcc;
  /**<   A 16-bit integer representation of CSG MCC. Range: 0 to 999.
  */

  uint16_t mnc;
  /**<   A 16-bit integer representation of CSG MNC. Range: 0 to 999.
  */

  uint8_t mnc_includes_pcs_digit;
  /**<   This field is used to interpret the length of the corresponding
       MNC reported in the TLVs (in this table) with an mnc or 
       mobile_network_code field. Values: \n

       - TRUE  -- MNC is a three-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 090  \n
       - FALSE -- MNC is a two-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 90
  */

  uint32_t id;
  /**<   Closed subscriber group identifier.
  */

  nas_radio_if_enum_v01 rat;
  /**<   Radio interface technology of the CSG network. Values: \n
       -0x04 -- RADIO_IF_GSM         -- GSM \n
       -0x05 -- RADIO_IF_UMTS        -- UMTS \n
       -0x08 -- RADIO_IF_LTE         -- LTE \n
       -0x09 -- RADIO_IF_TDSCDMA     -- TDS
  */
}nas_csg_nw_iden_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets the different system selection preferences of the device.
              \label{idl:setSysSelPref} */
typedef struct {

  /* Optional */
  /*  Emergency Mode */
  uint8_t emergency_mode_valid;  /**< Must be set to true if emergency_mode is being passed */
  uint8_t emergency_mode;
  /**<   Values: \n
       - 0x00 -- OFF (normal) \n
       - 0x01 -- ON (emergency)
  */

  /* Optional */
  /*  Mode Preference */
  uint8_t mode_pref_valid;  /**< Must be set to true if mode_pref is being passed */
  mode_pref_mask_type_v01 mode_pref;
  /**<   Bitmask representing the radio technology mode preference to be set. 
       Values: \n
       - Bit 0 (0x01) -- QMI_NAS_RAT_MODE_PREF_ CDMA2000_1X    -- 
         cdma2000\textsuperscript{\textregistered} 1X             \n
       - Bit 1 (0x02) -- QMI_NAS_RAT_MODE_PREF_ CDMA2000_HRPD  -- 
         cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - Bit 2 (0x04) -- QMI_NAS_RAT_MODE_PREF_ GSM            -- GSM \n
       - Bit 3 (0x08) -- QMI_NAS_RAT_MODE_PREF_ UMTS           -- UMTS \n
       - Bit 4 (0x10) -- QMI_NAS_RAT_MODE_PREF_ LTE            -- LTE \n
       - Bit 5 (0x20) -- QMI_NAS_RAT_MODE_PREF_ TDSCDMA        -- TD-SCDMA

       \vspace{3pt}
       All unlisted bits are reserved for future use and the service point
       ignores them if used.
  */

  /* Optional */
  /*  Band Preference */
  uint8_t band_pref_valid;  /**< Must be set to true if band_pref is being passed */
  nas_band_pref_mask_type_v01 band_pref;
  /**<   Bitmask representing the band preference to be set.  
       See Table @latexonly\ref{tbl:bandPreference}@endlatexonly 
       for details.   
  */

  /* Optional */
  /*  CDMA PRL Preference */
  uint8_t prl_pref_valid;  /**< Must be set to true if prl_pref is being passed */
  nas_prl_pref_enum_v01 prl_pref;
  /**<   PRL preference to be set for band class 0 (BC0) prl_pref. Values: \n
       - 0x0001 -- PRL_PREF_A_SIDE_ONLY -- Acquire available system only on the A side \n
       - 0x0002 -- PRL_PREF_B_SIDE_ONLY -- Acquire available system only on the B side \n
       - 0x3FFF -- PRL_PREF_ANY         -- Acquire any available systems
  */

  /* Optional */
  /*  Roaming Preference */
  uint8_t roam_pref_valid;  /**< Must be set to true if roam_pref is being passed */
  nas_roam_pref_enum_v01 roam_pref;
  /**<   Roaming preference to be set. Values: \n
       - 0x01 -- ROAMING_PREF_OFF         -- Acquire only systems for which the roaming indicator is off \n
       - 0x02 -- ROAMING_PREF_NOT_OFF     -- Acquire a system as long as its roaming indicator is not off \n
       - 0x03 -- ROAMING_PREF_NOT_FLASING -- Acquire only systems for which the roaming indicator is off or solid on, i.e., not flashing; CDMA only \n
       - 0xFF -- ROAMING_PREF_ANY         -- Acquire systems, regardless of their roaming indicator
  */

  /* Optional */
  /*  LTE Band Preference */
  uint8_t lte_band_pref_valid;  /**< Must be set to true if lte_band_pref is being passed */
  lte_band_pref_mask_type_v01 lte_band_pref;
  /**<   Bitmask representing the LTE band preference to be set. 
       See Table @latexonly\ref{tbl:lteBandPreference}@endlatexonly 
       for details.  
  */

  /* Optional */
  /*  Network Selection Preference */
  uint8_t net_sel_pref_valid;  /**< Must be set to true if net_sel_pref is being passed */
  nas_net_sel_pref_type_v01 net_sel_pref;

  /* Optional */
  /*  Change Duration */
  uint8_t change_duration_valid;  /**< Must be set to true if change_duration is being passed */
  nas_change_duration_enum_v01 change_duration;
  /**<    Duration of the change. Values: \n
        - 0x00 -- Power cycle -- Remains active until the next device power cycle \n
        - 0x01 -- Permanent -- Remains active through power cycles until changed by the client \n
        Note: The device will use "0x01 -- Permanent" as the default value 
              if the TLV is omitted.
  */

  /* Optional */
  /*  Service Domain */
  uint8_t srv_domain_pref_valid;  /**< Must be set to true if srv_domain_pref is being passed */
  nas_srv_domain_pref_enum_type_v01 srv_domain_pref;
  /**<    Service domain preference. Values: \n
       - 0x00  -- QMI_SRV_DOMAIN_PREF_CS_ONLY -- Circuit-switched only \n
       - 0x01  -- QMI_SRV_DOMAIN_PREF_PS_ONLY -- Packet-switched only  \n
       - 0x02  -- QMI_SRV_DOMAIN_PREF_CS_PS   -- Circuit-switched and packet-switched \n
       - 0x03  -- QMI_SRV_DOMAIN_PREF_PS_ ATTACH -- Packet-switched attach \n
       - 0x04  -- QMI_SRV_DOMAIN_PREF_PS_ DETACH -- Packet-switched detach \n
       - 0x05  -- QMI_SRV_DOMAIN_PREF_PS_ DETACH_NO_PREF_CHANGE -- Packet-switched 
                  detach with no change in the service domain preference
  */

  /* Optional */
  /*  GSM/WCDMA Acquisition Order */
  uint8_t gw_acq_order_pref_valid;  /**< Must be set to true if gw_acq_order_pref is being passed */
  nas_gw_acq_order_pref_enum_type_v01 gw_acq_order_pref;
  /**<    GSM/WCDMA acquisition order preference. Values: \n
       - 0x00 -- NAS_GW_ACQ_ORDER_PREF_ AUTOMATIC -- Automatic \n
       - 0x01 -- NAS_GW_ACQ_ORDER_PREF_ GSM_WCDMA -- GSM then WCDMA \n
       - 0x02 -- NAS_GW_ACQ_ORDER_PREF_ WCDMA_GSM -- WCDMA then GSM
  */

  /* Optional */
  /* MNC PCS Digit Include Status */
  uint8_t mnc_includes_pcs_digit_valid;  /**< Must be set to true if mnc_includes_pcs_digit is being passed */
  uint8_t mnc_includes_pcs_digit;
  /**<   This field is used to interpret the length of the corresponding
       MNC reported in the Network Selection Preference TLV (0x16). Values: \n

       - TRUE  -- MNC is a three-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 090  \n
       - FALSE -- MNC is a two-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 90
  */

  /* Optional */
  /*  TDSCDMA Band Preference */
  uint8_t tdscdma_band_pref_valid;  /**< Must be set to true if tdscdma_band_pref is being passed */
  nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref;
  /**<   Bitmask representing the TD-SCDMA band preference to be set. Values: \n
       - 0x01 -- NAS_TDSCDMA_BAND_A  -- TD-SCDMA Band A \n
       - 0x02 -- NAS_TDSCDMA_BAND_B  -- TD-SCDMA Band B \n
       - 0x04 -- NAS_TDSCDMA_BAND_C  -- TD-SCDMA Band C \n
       - 0x08 -- NAS_TDSCDMA_BAND_D  -- TD-SCDMA Band D \n
       - 0x10 -- NAS_TDSCDMA_BAND_E  -- TD-SCDMA Band E \n
       - 0x20 -- NAS_TDSCDMA_BAND_F  -- TD-SCDMA Band F

       \vspace{3pt}
       All other bits are reserved.
  */

  /* Optional */
  /*  Acquisition Order Preference */
  uint8_t acq_order_valid;  /**< Must be set to true if acq_order is being passed */
  uint32_t acq_order_len;  /**< Must be set to # of elements in acq_order */
  nas_radio_if_enum_v01 acq_order[NAS_ACQ_ORDER_LIST_MAX_V01];
  /**<   Acquisition order preference to be set. Values: \n
    - 0x01 -- NAS_RADIO_IF_CDMA_1X     -- 
      cdma2000\textsuperscript{\textregistered} 1X             \n
    - 0x02 -- NAS_RADIO_IF_CDMA_1XEVDO -- 
      cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
    - 0x04 -- NAS_RADIO_IF_GSM         -- GSM \n
    - 0x05 -- NAS_RADIO_IF_UMTS        -- UMTS \n
    - 0x08 -- NAS_RADIO_IF_LTE         -- LTE \n
    - 0x09 -- NAS_RADIO_IF_TDSCDMA     -- TD-SCDMA
  */

  /* Optional */
  /*  Network Selection Registration Restriction Preference */
  uint8_t srv_reg_restriction_valid;  /**< Must be set to true if srv_reg_restriction is being passed */
  nas_srv_reg_restriction_enum_v01 srv_reg_restriction;
  /**<   Registration restriction preference. Specifies one of the following 
       modifiers to net_sel_pref: \n
    - 0x00 -- NAS_SRV_REG_RESTRICTION_ UNRESTRICTED -- Device follows the normal 
              registration process \n
    - 0x01 -- NAS_SRV_REG_RESTRICTION_ CAMPED_ONLY -- Device camps on the network 
              according to its provisioning, but does not register \n
    - 0x02 -- NAS_SRV_REG_RESTRICTION_ LIMITED -- Device selects the network for 
              limited service

    \vspace{3pt}
    All other values are reserved.
   */

  /* Optional */
  /*  CSG ID */
  uint8_t csg_info_valid;  /**< Must be set to true if csg_info is being passed */
  nas_csg_nw_iden_type_v01 csg_info;

  /* Optional */
  /*  Usage Preference */
  uint8_t usage_setting_valid;  /**< Must be set to true if usage_setting is being passed */
  nas_usage_pref_enum_v01 usage_setting;
  /**<   Modem usage preference to be set. Values: \n
      - NAS_USAGE_VOICE_CENTRIC (1) --  Voice centric \n 
      - NAS_USAGE_DATA_CENTRIC (2) --  Data centric 
 */

  /* Optional */
  /*  Radio Access Technology */
  uint8_t rat_valid;  /**< Must be set to true if rat is being passed */
  nas_radio_if_enum_v01 rat;
  /**<   Radio access technology for the corresponding PLMN ID in the Network 
       Selection Preference TLV (0x16). If this TLV is present and the 
       net_sel_pref field is set to automatic, the provided MCC, MNC, and RAT 
       are searched for first. If they are not found, the selection falls 
       back to automatic. This TLV can also be used with the net_sel_pref 
       field set to manual to indicate the RAT of the specified MCC and MNC. \n
       Values: \n
    - 0x04 -- NAS_RADIO_IF_GSM         -- GSM \n
    - 0x05 -- NAS_RADIO_IF_UMTS        -- UMTS \n
    - 0x08 -- NAS_RADIO_IF_LTE         -- LTE \n
    - 0x09 -- NAS_RADIO_IF_TDSCDMA     -- TD-SCDMA
  */
}nas_set_system_selection_preference_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets the different system selection preferences of the device.
              \label{idl:setSysSelPref} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_set_system_selection_preference_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_system_selection_preference_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the different system selection preferences of the 
              device.
              \label{idl:getSysSelPref} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Emergency Mode */
  uint8_t emergency_mode_valid;  /**< Must be set to true if emergency_mode is being passed */
  uint8_t emergency_mode;
  /**<   Values: \n
       - 0x00 -- OFF (normal) \n
       - 0x01 -- ON (emergency)
  */

  /* Optional */
  /*  Mode Preference */
  uint8_t mode_pref_valid;  /**< Must be set to true if mode_pref is being passed */
  mode_pref_mask_type_v01 mode_pref;
  /**<   Bitmask representing the radio technology mode preference to be set. 
       Values: \n
       - Bit 0 (0x01) -- QMI_NAS_RAT_MODE_PREF_ CDMA2000_1X    -- 
         cdma2000\textsuperscript{\textregistered} 1X             \n
       - Bit 1 (0x02) -- QMI_NAS_RAT_MODE_PREF_ CDMA2000_HRPD  -- 
         cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - Bit 2 (0x04) -- QMI_NAS_RAT_MODE_PREF_ GSM            -- GSM \n
       - Bit 3 (0x08) -- QMI_NAS_RAT_MODE_PREF_ UMTS           -- UMTS \n
       - Bit 4 (0x10) -- QMI_NAS_RAT_MODE_PREF_ LTE            -- LTE \n
       - Bit 5 (0x20) -- QMI_NAS_RAT_MODE_PREF_ TDSCDMA        -- TD-SCDMA

       \vspace{3pt}
       All unlisted bits are reserved for future use and the service point
       ignores them if used.
  */

  /* Optional */
  /*  Band Preference */
  uint8_t band_pref_valid;  /**< Must be set to true if band_pref is being passed */
  nas_band_pref_mask_type_v01 band_pref;
  /**<   Bitmask representing the band preference to be set. 
       See Table @latexonly\ref{tbl:bandPreference}@endlatexonly 
       for details.   
  */

  /* Optional */
  /*  CDMA PRL Preference */
  uint8_t prl_pref_valid;  /**< Must be set to true if prl_pref is being passed */
  nas_prl_pref_enum_v01 prl_pref;
  /**<   PRL preference to be set for band class 0 (BC0) prl_pref. Values: \n
       - 0x0001 -- PRL_PREF_A_SIDE_ONLY -- Acquire available system only on the A side \n
       - 0x0002 -- PRL_PREF_B_SIDE_ONLY -- Acquire available system only on the B side \n
       - 0x3FFF -- PRL_PREF_ANY         -- Acquire any available systems
  */

  /* Optional */
  /*  Roaming Preference */
  uint8_t roam_pref_valid;  /**< Must be set to true if roam_pref is being passed */
  nas_roam_pref_enum_v01 roam_pref;
  /**<   Roaming preference to be set. Values: \n
       - 0x01 -- ROAMING_PREF_OFF         -- Acquire only systems for which the roaming indicator is off \n
       - 0x02 -- ROAMING_PREF_NOT_OFF     -- Acquire a system as long as its roaming indicator is not off \n
       - 0x03 -- ROAMING_PREF_NOT_FLASING -- Acquire only systems for which the roaming indicator is off or solid on, i.e., not flashing; CDMA only \n
       - 0xFF -- ROAMING_PREF_ANY         -- Acquire systems, regardless of their roaming indicator
  */

  /* Optional */
  /*  LTE Band Preference */
  uint8_t band_pref_ext_valid;  /**< Must be set to true if band_pref_ext is being passed */
  uint64_t band_pref_ext;
  /**<   Bitmask representing the LTE band preference to be set. Values: \n
         - Bit 0  -- E-UTRA Operating Band 1 \n
         - Bit 1  -- E-UTRA Operating Band 2 \n
         - Bit 2  -- E-UTRA Operating Band 3 \n
         - Bit 3  -- E-UTRA Operating Band 4 \n
         - Bit 4  -- E-UTRA Operating Band 5 \n
         - Bit 5  -- E-UTRA Operating Band 6 \n
         - Bit 6  -- E-UTRA Operating Band 7 \n
         - Bit 7  -- E-UTRA Operating Band 8 \n
         - Bit 8  -- E-UTRA Operating Band 9 \n
         - Bit 9  -- E-UTRA Operating Band 10 \n
         - Bit 10 -- E-UTRA Operating Band 11 \n
         - Bit 11 -- E-UTRA Operating Band 12 \n
         - Bit 12 -- E-UTRA Operating Band 13 \n
         - Bit 13 -- E-UTRA Operating Band 14 \n
         - Bit 16 -- E-UTRA Operating Band 17 \n
         - Bit 17 -- E-UTRA Operating Band 18 \n
         - Bit 18 -- E-UTRA Operating Band 19 \n
         - Bit 19 -- E-UTRA Operating Band 20 \n
         - Bit 20 -- E-UTRA Operating Band 21 \n
         - Bit 23 -- E-UTRA Operating Band 24 \n
         - Bit 24 -- E-UTRA Operating Band 25 \n
         - Bit 32 -- E-UTRA Operating Band 33 \n
         - Bit 33 -- E-UTRA Operating Band 34 \n
         - Bit 34 -- E-UTRA Operating Band 35 \n
         - Bit 35 -- E-UTRA Operating Band 36 \n
         - Bit 36 -- E-UTRA Operating Band 37 \n
         - Bit 37 -- E-UTRA Operating Band 38 \n
         - Bit 38 -- E-UTRA Operating Band 39 \n
         - Bit 39 -- E-UTRA Operating Band 40 \n
         - Bit 40 -- E-UTRA Operating Band 41 \n
         - Bit 41 -- E-UTRA Operating Band 42 \n
         - Bit 42 -- E-UTRA Operating Band 43

         \vspace{3pt}
         All other bits are reserved.
  */

  /* Optional */
  /*  Network Selection Preference */
  uint8_t net_sel_pref_valid;  /**< Must be set to true if net_sel_pref is being passed */
  nas_net_sel_pref_enum_v01 net_sel_pref;
  /**<   Network selection preference. Values: \n
       - 0x00 -- Automatic network selection \n
       - 0x01 -- Manual network selection
  */

  /* Optional */
  /*  Service Domain Preference */
  uint8_t srv_domain_pref_valid;  /**< Must be set to true if srv_domain_pref is being passed */
  nas_srv_domain_pref_enum_type_v01 srv_domain_pref;
  /**<   Service domain preference. Values: \n
       - 0x00  -- QMI_SRV_DOMAIN_PREF_CS_ONLY -- Circuit-switched only \n
       - 0x01  -- QMI_SRV_DOMAIN_PREF_PS_ONLY -- Packet-switched only  \n
       - 0x02  -- QMI_SRV_DOMAIN_PREF_CS_PS   -- Circuit-switched and packet-switched
  */

  /* Optional */
  /*  GSM/WCDMA Acquisition Order Preference */
  uint8_t gw_acq_order_pref_valid;  /**< Must be set to true if gw_acq_order_pref is being passed */
  nas_gw_acq_order_pref_enum_type_v01 gw_acq_order_pref;
  /**<   GSM/WCDMA acquisition order preference. Values: \n
       - 0x00 -- NAS_GW_ACQ_ORDER_PREF_ AUTOMATIC -- Automatic \n
       - 0x01 -- NAS_GW_ACQ_ORDER_PREF_ GSM_WCDMA -- GSM then WCDMA \n
       - 0x02 -- NAS_GW_ACQ_ORDER_PREF_ WCDMA_GSM -- WCDMA then GSM
    */

  /* Optional */
  /*  TDSCDMA Band Preference */
  uint8_t tdscdma_band_pref_valid;  /**< Must be set to true if tdscdma_band_pref is being passed */
  nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref;
  /**<   Bitmask representing the TD-SCDMA band preference to be set. Values: \n
       - 0x01 -- NAS_TDSCDMA_BAND_A  -- TD-SCDMA Band A \n
       - 0x02 -- NAS_TDSCDMA_BAND_B  -- TD-SCDMA Band B \n
       - 0x04 -- NAS_TDSCDMA_BAND_C  -- TD-SCDMA Band C \n
       - 0x08 -- NAS_TDSCDMA_BAND_D  -- TD-SCDMA Band D \n
       - 0x10 -- NAS_TDSCDMA_BAND_E  -- TD-SCDMA Band E \n
       - 0x20 -- NAS_TDSCDMA_BAND_F  -- TD-SCDMA Band F

       \vspace{3pt}
       All other bits are reserved.
  */

  /* Optional */
  /*  Manual Network Selection PLMN */
  uint8_t manual_net_sel_plmn_valid;  /**< Must be set to true if manual_net_sel_plmn is being passed */
  nas_mnc_pcs_digit_include_status_type_v01 manual_net_sel_plmn;

  /* Optional */
  /*  Acquisition Order Preference */
  uint8_t acq_order_valid;  /**< Must be set to true if acq_order is being passed */
  uint32_t acq_order_len;  /**< Must be set to # of elements in acq_order */
  nas_radio_if_enum_v01 acq_order[NAS_ACQ_ORDER_LIST_MAX_V01];
  /**<   Acquisition order preference to be set. Values: \n
    - 0x01 -- NAS_RADIO_IF_CDMA_1X     -- 
      cdma2000\textsuperscript{\textregistered} 1X             \n
    - 0x02 -- NAS_RADIO_IF_CDMA_1XEVDO -- 
      cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
    - 0x04 -- NAS_RADIO_IF_GSM         -- GSM \n
    - 0x05 -- NAS_RADIO_IF_UMTS        -- UMTS \n
    - 0x08 -- NAS_RADIO_IF_LTE         -- LTE \n
    - 0x09 -- NAS_RADIO_IF_TDSCDMA     -- TD-SCDMA
  */

  /* Optional */
  /*  Network Selection Registration Restriction Preference */
  uint8_t srv_reg_restriction_valid;  /**< Must be set to true if srv_reg_restriction is being passed */
  nas_srv_reg_restriction_enum_v01 srv_reg_restriction;
  /**<   Registration restriction preference. Specifies one of the following 
       modifiers to net_sel_pref: \n
    - 0x00 -- NAS_SRV_REG_RESTRICTION_ UNRESTRICTED -- Device follows the normal 
              registration process \n
    - 0x01 -- NAS_SRV_REG_RESTRICTION_ CAMPED_ONLY -- Device camps on the network 
              according to its provisioning, but does not register \n
    - 0x02 -- NAS_SRV_REG_RESTRICTION_ LIMITED -- Device selects the network for 
              limited service

    \vspace{3pt}
    All other values are reserved.
   */

  /* Optional */
  /*  CSG ID */
  uint8_t csg_info_valid;  /**< Must be set to true if csg_info is being passed */
  nas_csg_nw_iden_type_v01 csg_info;

  /* Optional */
  /*  Usage Preference */
  uint8_t usage_setting_valid;  /**< Must be set to true if usage_setting is being passed */
  nas_usage_pref_enum_v01 usage_setting;
  /**<   Modem usage preference to be set. Values: \n
      - NAS_USAGE_UNKNOWN (0) --  Unknown \n 
      - NAS_USAGE_VOICE_CENTRIC (1) --  Voice centric \n 
      - NAS_USAGE_DATA_CENTRIC (2) --  Data centric 
 */
}nas_get_system_selection_preference_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Queries the different system selection preferences of the 
              device.
              \label{idl:getSysSelPref} */
typedef struct {

  /* Optional */
  /*  Emergency Mode */
  uint8_t emergency_mode_valid;  /**< Must be set to true if emergency_mode is being passed */
  uint8_t emergency_mode;
  /**<   Values: \n
       - 0x00 -- OFF (normal) \n
       - 0x01 -- ON (emergency)
  */

  /* Optional */
  /*  Mode Preference */
  uint8_t mode_pref_valid;  /**< Must be set to true if mode_pref is being passed */
  mode_pref_mask_type_v01 mode_pref;
  /**<   Bitmask representing the radio technology mode preference to be set. 
       Values: \n
       - Bit 0 (0x01) -- QMI_NAS_RAT_MODE_PREF_ CDMA2000_1X    -- 
         cdma2000\textsuperscript{\textregistered} 1X \n
       - Bit 1 (0x02) -- QMI_NAS_RAT_MODE_PREF_ CDMA2000_HRPD  -- 
         cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - Bit 2 (0x04) -- QMI_NAS_RAT_MODE_PREF_ GSM            -- GSM \n
       - Bit 3 (0x08) -- QMI_NAS_RAT_MODE_PREF_ UMTS           -- UMTS \n
       - Bit 4 (0x10) -- QMI_NAS_RAT_MODE_PREF_ LTE            -- LTE \n
       - Bit 5 (0x20) -- QMI_NAS_RAT_MODE_PREF_ TDSCDMA        -- TD-SCDMA

       \vspace{3pt}
       All unlisted bits are reserved for future use.
  */

  /* Optional */
  /*  Band Preference */
  uint8_t band_pref_valid;  /**< Must be set to true if band_pref is being passed */
  nas_band_pref_mask_type_v01 band_pref;
  /**<   Bitmask representing the band preference to be set. 
       See Table @latexonly\ref{tbl:bandPreference}@endlatexonly 
       for details.   
  */

  /* Optional */
  /*  CDMA PRL Preference */
  uint8_t prl_pref_valid;  /**< Must be set to true if prl_pref is being passed */
  nas_prl_pref_enum_v01 prl_pref;
  /**<   PRL preference to be set for band class 0 (BC0) prl_pref. Values: \n
       - 0x0001 -- PRL_PREF_A_SIDE_ONLY -- Acquire available system only on the A side \n
       - 0x0002 -- PRL_PREF_B_SIDE_ONLY -- Acquire available system only on the B side \n
       - 0x3FFF -- PRL_PREF_ANY         -- Acquire any available systems
  */

  /* Optional */
  /*  Roaming Preference */
  uint8_t roam_pref_valid;  /**< Must be set to true if roam_pref is being passed */
  nas_roam_pref_enum_v01 roam_pref;
  /**<   Roaming preference to be set. Values: \n
       - 0x01 -- ROAMING_PREF_OFF         -- Acquire only systems for which the roaming indicator is off \n
       - 0x02 -- ROAMING_PREF_NOT_OFF     -- Acquire a system as long as its roaming indicator is not off \n
       - 0x03 -- ROAMING_PREF_NOT_FLASING -- Acquire only systems for which the roaming indicator is off or solid on, i.e., not flashing; CDMA only \n
       - 0xFF -- ROAMING_PREF_ANY         -- Acquire systems, regardless of their roaming indicator
  */

  /* Optional */
  /*  LTE Band Preference */
  uint8_t lte_band_pref_valid;  /**< Must be set to true if lte_band_pref is being passed */
  lte_band_pref_mask_type_v01 lte_band_pref;
  /**<   Bitmask representing the LTE band preference to be set. 
       See Table @latexonly\ref{tbl:lteBandPreference}@endlatexonly 
       for details.  
  */

  /* Optional */
  /*  Network Selection Preference */
  uint8_t net_sel_pref_valid;  /**< Must be set to true if net_sel_pref is being passed */
  nas_net_sel_pref_enum_v01 net_sel_pref;
  /**<   Network selection preference. Values: \n
       - 0x00 -- Automatic network selection \n
       - 0x01 -- Manual network selection
  */

  /* Optional */
  /*  Service Domain Preference */
  uint8_t srv_domain_pref_valid;  /**< Must be set to true if srv_domain_pref is being passed */
  nas_srv_domain_pref_enum_type_v01 srv_domain_pref;
  /**<   Service domain preference. Values: \n
       - 0x00  -- QMI_SRV_DOMAIN_PREF_CS_ONLY -- Circuit-switched only \n
       - 0x01  -- QMI_SRV_DOMAIN_PREF_PS_ONLY -- Packet-switched only  \n
       - 0x02  -- QMI_SRV_DOMAIN_PREF_CS_PS   -- Circuit-switched and packet-switched
  */

  /* Optional */
  /*  GSM/WCDMA Acquisition Order Preference */
  uint8_t gw_acq_order_pref_valid;  /**< Must be set to true if gw_acq_order_pref is being passed */
  nas_gw_acq_order_pref_enum_type_v01 gw_acq_order_pref;
  /**<   GSM/WCDMA acquisition order preference. Values: \n
       - 0x00 -- NAS_GW_ACQ_ORDER_PREF_ AUTOMATIC -- Automatic \n
       - 0x01 -- NAS_GW_ACQ_ORDER_PREF_ GSM_WCDMA -- GSM then WCDMA \n
       - 0x02 -- NAS_GW_ACQ_ORDER_PREF_ WCDMA_GSM -- WCDMA then GSM
   */

  /* Optional */
  /*  TDSCDMA Band Preference */
  uint8_t tdscdma_band_pref_valid;  /**< Must be set to true if tdscdma_band_pref is being passed */
  nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref;
  /**<   Bitmask representing the TD-SCDMA band preference to be set. Values: \n
       - 0x01 -- NAS_TDSCDMA_BAND_A  -- TD-SCDMA Band A \n
       - 0x02 -- NAS_TDSCDMA_BAND_B  -- TD-SCDMA Band B \n
       - 0x04 -- NAS_TDSCDMA_BAND_C  -- TD-SCDMA Band C \n
       - 0x08 -- NAS_TDSCDMA_BAND_D  -- TD-SCDMA Band D \n
       - 0x10 -- NAS_TDSCDMA_BAND_E  -- TD-SCDMA Band E \n
       - 0x20 -- NAS_TDSCDMA_BAND_F  -- TD-SCDMA Band F

       \vspace{3pt}
       All other bits are reserved.
  */

  /* Optional */
  /*  Manual Network Selection PLMN */
  uint8_t manual_net_sel_plmn_valid;  /**< Must be set to true if manual_net_sel_plmn is being passed */
  nas_mnc_pcs_digit_include_status_type_v01 manual_net_sel_plmn;

  /* Optional */
  /*  Acquisition Order Preference */
  uint8_t acq_order_valid;  /**< Must be set to true if acq_order is being passed */
  uint32_t acq_order_len;  /**< Must be set to # of elements in acq_order */
  nas_radio_if_enum_v01 acq_order[NAS_ACQ_ORDER_LIST_MAX_V01];
  /**<   Acquisition order preference to be set. Values: \n
    - 0x01 -- NAS_RADIO_IF_CDMA_1X     -- 
      cdma2000\textsuperscript{\textregistered} 1X             \n
    - 0x02 -- NAS_RADIO_IF_CDMA_1XEVDO -- 
      cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
    - 0x04 -- NAS_RADIO_IF_GSM         -- GSM \n
    - 0x05 -- NAS_RADIO_IF_UMTS        -- UMTS \n
    - 0x08 -- NAS_RADIO_IF_LTE         -- LTE \n
    - 0x09 -- NAS_RADIO_IF_TDSCDMA     -- TD-SCDMA
  */

  /* Optional */
  /*  Network Selection Registration Restriction Preference */
  uint8_t srv_reg_restriction_valid;  /**< Must be set to true if srv_reg_restriction is being passed */
  nas_srv_reg_restriction_enum_v01 srv_reg_restriction;
  /**<   Registration restriction preference. Specifies one of the following 
       modifiers to net_sel_pref: \n
    - 0x00 -- NAS_SRV_REG_RESTRICTION_ UNRESTRICTED -- Device follows the normal 
              registration process \n
    - 0x01 -- NAS_SRV_REG_RESTRICTION_ CAMPED_ONLY -- Device camps on the network 
              according to its provisioning, but does not register \n
    - 0x02 -- NAS_SRV_REG_RESTRICTION_ LIMITED -- Device selects the network for 
              limited service

    \vspace{3pt}
    All other values are reserved.
   */

  /* Optional */
  /*  CSG ID */
  uint8_t csg_info_valid;  /**< Must be set to true if csg_info is being passed */
  nas_csg_nw_iden_type_v01 csg_info;

  /* Optional */
  /*  Usage Preference */
  uint8_t usage_setting_valid;  /**< Must be set to true if usage_setting is being passed */
  nas_usage_pref_enum_v01 usage_setting;
  /**<   Usage preference to be set. Values: \n
      - NAS_USAGE_UNKNOWN (0) --  Unknown \n 
      - NAS_USAGE_VOICE_CENTRIC (1) --  Voice centric \n 
      - NAS_USAGE_DATA_CENTRIC (2) --  Data centric 
 */
}nas_system_selection_preference_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_DDTM_PREF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_DDTM_PREF_OFF_V01 = 0x00, 
  NAS_DDTM_PREF_ON_V01 = 0x01, 
  NAS_DDTM_PREF_NO_CHANGE_V01 = 0x02, 
  NAS_DDTM_PREF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_ddtm_pref_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SO_LIST_ACTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SO_LIST_ACTION_ADD_V01 = 0x00, 
  NAS_SO_LIST_ACTION_REPLACE_V01 = 0x01, 
  NAS_SO_LIST_ACTION_DELETE_V01 = 0x02, 
  NAS_SO_LIST_ACTION_NO_CHANGE_V01 = 0x03, 
  NAS_SO_LIST_ACTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_so_list_action_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_ddtm_pref_enum_v01 ddtm_pref;
  /**<   DDTM preference setting. Values: \n
       - 0x00 -- DDTM_PREF_OFF       -- Disable DDTM \n
       - 0x01 -- DDTM_PREF_ON        -- Enable DDTM \n
       - 0x02 -- DDTM_PREF_NO_CHANGE -- Do not change DDTM preference
  */

  uint16_t ddtm_action;
  /**<   Bitmask (with each bit specifying action) representing what
       combined DDTM actions should take place. Values: \n
       - Bit 0 -- QMI_NAS_DDTM_ACTION_ SUPPRESS_L2ACK_BIT  -- Do not send L2 ACK on 1X \n
       - Bit 1 -- QMI_NAS_DDTM_ACTION_ SUPPRESS_REG_BIT    -- Suppress 1X registrations \n
       - Bit 2 -- QMI_NAS_DDTM_ACTION_IGNORE_ SO_PAGES_BIT -- Ignore 1X pages with specified service options \n
       - Bit 3 -- QMI_NAS_DDTM_ACTION_ SUPPRESS_MO_DBM_BIT -- Block MO SMS and DBM

  To enable all masks, a value of 0x3FFF must be sent in this field.
  */

  nas_so_list_action_enum_v01 so_list_action;
  /**<   Action to be taken with the specified SO list in the SO field. Values: \n
       - 0x00 -- SO_LIST_ACTION_ADD       -- Add the specified SOs to the current DDTM SO list \n
       - 0x01 -- SO_LIST_ACTION_REPLACE   -- Replace the current DDTM SO list \n
       - 0x02 -- SO_LIST_ACTION_DELETE    -- Delete the specified SOs from the DDTM SO list \n
       - 0x03 -- SO_LIST_ACTION_NO_CHANGE -- No change in the DDTM SO list
  */

  uint32_t so_len;  /**< Must be set to # of elements in so */
  uint16_t so[NAS_SO_LIST_MAX_V01];
  /**<   Service option for which SO pages are ignored
       when DDTM status is ON. Refer to \hyperref[S4]{[S4]} Table 3.1-1 for 
       standard SO number assignments. To ignore all SO pages, a value of 0xFFFF
       must be specified.
  */
}nas_ddtm_preference_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets the Data Dedicated Transmission Mode (DDTM) preference 
              for the device. */
typedef struct {

  /* Mandatory */
  /*  DDTM Preference */
  nas_ddtm_preference_type_v01 ddtm_preference;
}nas_set_ddtm_preference_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets the Data Dedicated Transmission Mode (DDTM) preference 
              for the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_set_ddtm_preference_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_CURR_DDTM_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_CURRENT_DDTM_STATUS_DISABLED_V01 = 0x00, 
  NAS_CURRENT_DDTM_STATUS_ENABLED_V01 = 0x01, 
  NAS_CURR_DDTM_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_curr_ddtm_status_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_curr_ddtm_status_enum_v01 curr_ddtm_status;
  /**<   Current DDTM status. Values: \n
       - 0x00 -- CURRENT_DDTM_STATUS_DISABLED \n
       - 0x01 -- CURRENT_DDTM_STATUS_ENABLED
  */

  nas_ddtm_pref_enum_v01 ddtm_pref;
  /**<   DDTM preference setting. Values: \n
       - 0x00 -- DDTM_PREF_OFF       -- Disable DDTM \n
       - 0x01 -- DDTM_PREF_ON        -- Enable DDTM
  */

  uint16_t ddtm_action;
  /**<   Bitmask (with each bit specifying action) representing what
       combined DDTM actions should take place. Values: \n
       - Bit 0 -- QMI_NAS_DDTM_ACTION_ SUPPRESS_ L2ACK_BIT  -- Do not send L2 ACK on 1X \n
       - Bit 1 -- QMI_NAS_DDTM_ACTION_ SUPPRESS_ REG_BIT    -- Suppress 1X registrations \n
       - Bit 2 -- QMI_NAS_DDTM_ACTION_IGNORE_ SO_ PAGES_BIT -- Ignore 1X pages with specified service options \n
       - Bit 3 -- QMI_NAS_DDTM_ACTION_ SUPPRESS_ MO_DBM_BIT -- Block MO SMS and DBM \n
       To enable all masks, a value of 0x3FFF must be sent in this field
  */

  nas_so_list_action_enum_v01 so_list_action;
  /**<   Action to be taken with the specified SO list in the SO field. Values: \n
       - 0x00 -- SO_LIST_ACTION_ADD       -- Add the specified SOs to the current DDTM SO list \n
       - 0x01 -- SO_LIST_ACTION_REPLACE   -- Replace the current DDTM SO list \n
       - 0x02 -- SO_LIST_ACTION_DELETE    -- Delete the specified SOs from the DDTM SO list \n
       - 0x03 -- SO_LIST_ACTION_NO_CHANGE -- No change in the DDTM SO list
  */

  uint32_t so_len;  /**< Must be set to # of elements in so */
  uint16_t so[NAS_SO_LIST_MAX_V01];
  /**<   Service option for which SO pages are ignored
       when DDTM status is ON. Refer to \hyperref[S4]{[S4]} Table 3.1-1 for 
       standard SO number assignments. To ignore all SO pages, a value of 0xFFFF
       must be specified.
  */
}nas_ddtm_settings_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Provides the DDTM status of the device. */
typedef struct {

  /* Mandatory */
  /*  DDTM Settings */
  nas_ddtm_settings_type_v01 ddtm_settings;
}nas_ddtm_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_operator_name_data_req_msg_v01;

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t display_cond;
  /**<   Display condition*/

  uint32_t spn_len;  /**< Must be set to # of elements in spn */
  uint8_t spn[NAS_SERVICE_PROVIDER_NAME_MAX_V01];
  /**<    Service provider name string must use: \n
        - The SMS default 7-bit coded alphabet as defined in 
          \hyperref[S8]{[S8]} with bit 8 set to 9 \n
        - One UCS2 code option defined in \hyperref[S9]{[S9]} Annex B
  */
}nas_service_provider_name_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  char mcc[NAS_MCC_MNC_MAX_V01];
  /**<   MCC in ASCII string (a value of D in any of the digits is to be used
       to indicate a "wild" value for that corresponding digit).
  */

  char mnc[NAS_MCC_MNC_MAX_V01];
  /**<   MNC in ASCII string (a value of D in any of the digits is to be used
       to indicate a "wild" value for that corresponding digit; digit 3 in MNC
       is optional and when not present, will be set as ASCII F).
  */

  uint16_t lac1;
  /**<   Location area code 1.*/

  uint16_t lac2;
  /**<   Location area code 2.*/

  uint8_t pnn_rec_id;
  /**<   PLMN network name record identifier.*/
}nas_operator_plmn_list_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_CODING_SCHEME_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_CODING_SCHEME_CELL_BROADCAST_GSM_V01 = 0x00, 
  NAS_CODING_SCHEME_UCS2_V01 = 0x01, 
  NAS_CODING_SCHEME_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_coding_scheme_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_COUNTRY_INITIALS_ADD_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_COUNTRY_INITIALS_DO_NOT_ADD_V01 = 0x00, 
  NAS_COUNTRY_INITIALS_ADD_V01 = 0x01, 
  NAS_COUNTRY_INITIALS_UNSPEFICIED_V01 = 0xFF, 
  NAS_COUNTRY_INITIALS_ADD_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_country_initials_add_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SPARE_BITS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SPARE_BITS_8_V01 = 0x01, 
  NAS_SPARE_BITS_7_TO_8_V01 = 0x02, 
  NAS_SPARE_BITS_6_TO_8_V01 = 0x03, 
  NAS_SPARE_BITS_5_TO_8_V01 = 0x04, 
  NAS_SPARE_BITS_4_TO_8_V01 = 0x05, 
  NAS_SPARE_BITS_3_TO_8_V01 = 0x06, 
  NAS_SPARE_BITS_2_TO_8_V01 = 0x07, 
  NAS_SPARE_BITS_UNKNOWN_V01 = 0x00, 
  NAS_SPARE_BITS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_spare_bits_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_coding_scheme_enum_v01 coding_scheme;
  /**<   Coding scheme. Values: \n
       - 0x00 -- NAS_CODING_SCHEME_ CELL_BROADCAST_GSM -- Cell broadcast data 
                 coding scheme, GSM default alphabet, language unspecified; 
                 defined in \hyperref[S8]{[S8]} \n
       - 0x01 -- NAS_CODING_SCHEME_ UCS2 -- UCS2 (16 bit) \hyperref[S10]{[S10]}
  */

  nas_country_initials_add_enum_v01 ci;
  /**<   Country's initials. Values: \n
       - 0x00 -- COUNTRY_INITIALS_ DO_NOT_ADD -- MS does not add the letters 
                 for the country's initials to the text string \n
       - 0x01 -- COUNTRY_INITIALS_ADD -- MS adds the letters for the 
                 country's initials and a separator, e.g., a space, to the text 
                 string
  */

  nas_spare_bits_enum_v01 long_name_spare_bits;
  /**<   Values: \n
       - 0x01 -- SPARE_BITS_8       -- Bit 8 is spare and set to 0 in octet n                       \n
       - 0x02 -- SPARE_BITS_7_TO_8  -- Bits 7 and 8 are spare and set to 0 in octet n               \n
       - 0x03 -- SPARE_BITS_6_TO_8  -- Bits 6 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x04 -- SPARE_BITS_5_TO_8  -- Bits 5 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x05 -- SPARE_BITS_4_TO_8  -- Bits 4 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x06 -- SPARE_BITS_3_TO_8  -- Bits 3 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x07 -- SPARE_BITS_2_TO_8  -- Bits 2 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x00 -- SPARE_BITS_UNKNOWN -- Carries no information about the number of spare bits in octet n
  */

  nas_spare_bits_enum_v01 short_name_spare_bits;
  /**<   Values: \n
       - 0x01 -- SPARE_BITS_8       -- Bit 8 is spare and set to 0 in octet n                       \n
       - 0x02 -- SPARE_BITS_7_TO_8  -- Bits 7 and 8 are spare and set to 0 in octet n               \n
       - 0x03 -- SPARE_BITS_6_TO_8  -- Bits 6 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x04 -- SPARE_BITS_5_TO_8  -- Bits 5 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x05 -- SPARE_BITS_4_TO_8  -- Bits 4 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x06 -- SPARE_BITS_3_TO_8  -- Bits 3 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x07 -- SPARE_BITS_2_TO_8  -- Bits 2 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x00 -- SPARE_BITS_UNKNOWN -- Carries no information about the number of spare bits in octet n
  */

  uint32_t long_name_len;  /**< Must be set to # of elements in long_name */
  uint8_t long_name[NAS_LONG_NAME_MAX_V01];
  /**<   Long name string in coding_scheme.*/

  uint32_t short_name_len;  /**< Must be set to # of elements in short_name */
  uint8_t short_name[NAS_SHORT_NAME_MAX_V01];
  /**<   Short name string in coding_scheme.*/
}nas_plmn_network_name_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves operator name data from multiple sources. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. 
 Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
   */

  /* Optional */
  /*  Service Provider Name (refer to \hyperref[S7]{[S7]} Section 4.2.12) */
  uint8_t service_provider_name_valid;  /**< Must be set to true if service_provider_name is being passed */
  nas_service_provider_name_type_v01 service_provider_name;

  /* Optional */
  /*  Operator PLMN List (refer to \hyperref[S7]{[S7]} Section 4.2.59) */
  uint8_t operator_plmn_list_valid;  /**< Must be set to true if operator_plmn_list is being passed */
  uint32_t operator_plmn_list_len;  /**< Must be set to # of elements in operator_plmn_list */
  nas_operator_plmn_list_type_v01 operator_plmn_list[NAS_OPERATOR_PLMN_LIST_MAX_V01];

  /* Optional */
  /*  PLMN Network Name (refer to \hyperref[S5]{[S5]} Section 10.5.3.5a) */
  uint8_t plmn_network_name_valid;  /**< Must be set to true if plmn_network_name is being passed */
  uint32_t plmn_network_name_len;  /**< Must be set to # of elements in plmn_network_name */
  nas_plmn_network_name_type_v01 plmn_network_name[NAS_PLMN_NETWORK_NAME_LIST_MAX_V01];

  /* Optional */
  /*  Operator Name String (refer to \hyperref[S11]{[S11]} Section B.4.1.2) */
  uint8_t plmn_name_valid;  /**< Must be set to true if plmn_name is being passed */
  char plmn_name[NAS_PLMN_NAME_MAX_V01 + 1];
  /**<   PLMN name must be coded in a default 7-bit alphabet with b8 set to 0.*/

  /* Optional */
  /*  NITZ Information */
  uint8_t nitz_information_valid;  /**< Must be set to true if nitz_information is being passed */
  nas_plmn_network_name_type_v01 nitz_information;
}nas_get_operator_name_data_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in operator name data, which is obtained 
              from multiple sources. (Deprecated) */
typedef struct {

  /* Optional */
  /*  Service Provider Name (refer to \hyperref[S7]{[S7]} Section 4.2.12) */
  uint8_t service_provider_name_valid;  /**< Must be set to true if service_provider_name is being passed */
  nas_service_provider_name_type_v01 service_provider_name;

  /* Optional */
  /*  Operator PLMN List (refer to \hyperref[S7]{[S7]} Section 4.2.59) */
  uint8_t operator_plmn_list_valid;  /**< Must be set to true if operator_plmn_list is being passed */
  uint32_t operator_plmn_list_len;  /**< Must be set to # of elements in operator_plmn_list */
  nas_operator_plmn_list_type_v01 operator_plmn_list[NAS_OPERATOR_PLMN_LIST_MAX_V01];

  /* Optional */
  /*  PLMN Network Name (refer to \hyperref[S5]{[S5]} Section 10.5.3.5a) */
  uint8_t plmn_network_name_valid;  /**< Must be set to true if plmn_network_name is being passed */
  uint32_t plmn_network_name_len;  /**< Must be set to # of elements in plmn_network_name */
  nas_plmn_network_name_type_v01 plmn_network_name[NAS_PLMN_NETWORK_NAME_LIST_MAX_V01];

  /* Optional */
  /*  Operator Name String (refer to \hyperref[S11]{[S11]} Section B.4.1.2) */
  uint8_t plmn_name_valid;  /**< Must be set to true if plmn_name is being passed */
  char plmn_name[NAS_PLMN_NAME_MAX_V01 + 1];
  /**<   PLMN name must be coded in a default 7-bit alphabet with b8 set to 0*/

  /* Optional */
  /*  NITZ Information */
  uint8_t nitz_information_valid;  /**< Must be set to true if nitz_information is being passed */
  nas_plmn_network_name_type_v01 nitz_information;
}nas_operator_name_data_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_csp_plmn_mode_bit_req_msg_v01;

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_PLMN_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_PLMN_MODE_DO_NOT_RESTRICT_V01 = 0x00, 
  NAS_PLMN_MODE_RESTRICT_V01 = 0x01, 
  NAS_PLMN_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_plmn_mode_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the PLMN MODE bit data from the Customer Service 
              Profile (CSP). */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  PLMN Mode (refer to \hyperref[S11]{[S11]} Section 4.7.1) */
  uint8_t plmn_mode_valid;  /**< Must be set to true if plmn_mode is being passed */
  nas_plmn_mode_enum_v01 plmn_mode;
  /**<   Values: \n
       - 0x00 -- PLMN_MODE_DO_NOT_RESTRICT -- Do not restrict menu options for manual PLMN selection \n
       - 0x01 -- PLMN_MODE_RESTRICT        -- Restrict menu options for manual PLMN selection
  */
}nas_get_csp_plmn_mode_bit_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Provides any change in the PLMN MODE bit in the CSP. */
typedef struct {

  /* Optional */
  /*  PLMN Mode (refer to \hyperref[S11]{[S11]} Section 4.7.1) */
  uint8_t plmn_mode_valid;  /**< Must be set to true if plmn_mode is being passed */
  nas_plmn_mode_enum_v01 plmn_mode;
  /**<   Values: \n
       - 0x00 -- PLMN_MODE_DO_NOT_RESTRICT -- Do not restrict menu options for manual PLMN selection \n
       - 0x01 -- PLMN_MODE_RESTRICT        -- Restrict menu options for manual PLMN selection
  */
}nas_csp_plmn_mode_bit_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Updates the A-KEY. (Discontinued) */
typedef struct {

  /* Mandatory */
  /*  AKEY */
  uint8_t akey[26];
  /**<   AKEY value + checksum value in ASCII (first 20 bytes are the AKEY value,
       last 6 bytes are the checksum).
  */
}nas_update_akey_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Updates the A-KEY. (Discontinued) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_update_akey_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef uint32_t get_3gpp2_info_mask_enum_type_v01;
#define QMI_NAS_GET_3GPP2_SUBS_INFO_NAM_NAME_V01 ((get_3gpp2_info_mask_enum_type_v01)0x01) 
#define QMI_NAS_GET_3GPP2_SUBS_INFO_DIR_NUM_V01 ((get_3gpp2_info_mask_enum_type_v01)0x02) 
#define QMI_NAS_GET_3GPP2_SUBS_INFO_HOME_SID_IND_V01 ((get_3gpp2_info_mask_enum_type_v01)0x04) 
#define QMI_NAS_GET_3GPP2_SUBS_INFO_MIN_BASED_IMSI_V01 ((get_3gpp2_info_mask_enum_type_v01)0x08) 
#define QMI_NAS_GET_3GPP2_SUBS_INFO_TRUE_IMSI_V01 ((get_3gpp2_info_mask_enum_type_v01)0x10) 
#define QMI_NAS_GET_3GPP2_SUBS_INFO_CDMA_CHANNEL_V01 ((get_3gpp2_info_mask_enum_type_v01)0x20) 
#define QMI_NAS_GET_3GPP2_SUBS_INFO_MDN_V01 ((get_3gpp2_info_mask_enum_type_v01)0x40) 
/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Retrieves 3GPP2 subscription-related information. */
typedef struct {

  /* Mandatory */
  /*  NAM ID */
  uint8_t nam_id;
  /**<   NAM ID of the information to be retrieved. The index starts from 0. 
       A nam_id of 0xFF is used to retrieve information of current NAM.
  */

  /* Optional */
  /*  Get 3GPP2 Info Bitmask */
  uint8_t get_3gpp2_info_mask_valid;  /**< Must be set to true if get_3gpp2_info_mask is being passed */
  get_3gpp2_info_mask_enum_type_v01 get_3gpp2_info_mask;
  /**<   Bitmasks included in this field decide which optional TLVs are to be 
       included in the response message. If this TLV is not included, all 
       available information is sent as part of the response message. \n \vspace{-.12in}
       
       The bitmask enum value, bitmask enum member name, and TLV that is 
       included are: \n

       - 0x01 -- QMI_NAS_GET_3GPP2_SUBS_INFO_ NAM_NAME       -- NAM Name \n
       - 0x02 -- QMI_NAS_GET_3GPP2_SUBS_INFO_ DIR_NUM        -- Directory Number \n
       - 0x04 -- QMI_NAS_GET_3GPP2_SUBS_INFO_ HOME_SID_IND   -- Home SID/NID \n
       - 0x08 -- QMI_NAS_GET_3GPP2_SUBS_INFO_ MIN_BASED_IMSI -- MIN-based IMSI \n
       - 0x10 -- QMI_NAS_GET_3GPP2_SUBS_INFO_ TRUE_IMSI      -- True IMSI \n
       - 0x20 -- QMI_NAS_GET_3GPP2_SUBS_INFO_ CDMA_CHANNEL   -- CDMA Channel \n
       - 0x40 -- QMI_NAS_GET_3GPP2_SUBS_INFO_MDN             -- Mobile Directory Number

       \vspace{3pt}
       All other bits are reserved for future use.
  */
}nas_get_3gpp2_subscription_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t sid;
  /**<   System ID. */

  uint16_t nid;
  /**<   Network ID. */
}nas_3gpp2_home_sid_nid_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  MCC_M */
  char mcc_m[NAS_MCC_LEN_V01];
  /**<   ASCII character representation of MCC_M; 
       example: 000, 123, etc.
  */

  /*  IMSI_M_11_12 */
  char imsi_m_11_12[NAS_IMSI_11_12_LEN_V01];
  /**<   ASCII character representation of IMSI_M_11_12 value;
       example: 00, 01, etc.
  */

  /*  IMSI_M_S1 */
  char imsi_m_s1[NAS_IMSI_MIN1_LEN_V01];
  /**<   ASCII character representation of IMSI_M_S1 value;
       example: 0123456.
  */

  /*  IMSI_M_S2 */
  char imsi_m_s2[NAS_IMSI_MIN2_LEN_V01];
  /**<   ASCII character representation of IMSI_M_S2 value;
       example: 012.
  */
}nas_3gpp2_min_based_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  MCC_T */
  char mcc_t[NAS_MCC_LEN_V01];
  /**<   ASCII character representation of MCC_T;
       example: 000, 123, etc.
  */

  /*  IMSI_T_11_12 */
  char imsi_t_11_12[NAS_IMSI_11_12_LEN_V01];
  /**<   ASCII character representation of IMSI_T_11_12 value;
       example: 00, 01, etc.
  */

  /*  IMSI_T_S1 */
  char imsi_t_s1[NAS_IMSI_MIN1_LEN_V01];
  /**<   ASCII character representation of IMSI_T_S1 value;
       example: 0123456.
  */

  /*  IMSI_T_S2 */
  char imsi_t_s2[NAS_IMSI_MIN2_LEN_V01];
  /**<   ASCII character representation of IMSI_T_S2 value;
       example: 012.
  */

  /*  IMSI_T_ADDR_NUM */
  uint8_t imsi_t_addr_num;
  /**<   Value of IMSI_T_ADDR_NUM.
  */
}nas_3gpp2_true_imsi_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t pri_ch_a;
  /**<   A Channel number for the primary carrier. */

  uint16_t pri_ch_b;
  /**<   B Channel number for the primary carrier. */

  uint16_t sec_ch_a;
  /**<   A Channel number for the secondary carrier. */

  uint16_t sec_ch_b;
  /**<   B Channel number for the secondary carrier. */
}nas_cdma_channel_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves 3GPP2 subscription-related information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  NAM Name (information retrieved from NV_NAME_NAM_I) */
  uint8_t nam_name_valid;  /**< Must be set to true if nam_name is being passed */
  uint32_t nam_name_len;  /**< Must be set to # of elements in nam_name */
  char nam_name[NAS_MAX_NAM_NAME_LEN_V01];
  /**<   Name information in ASCII. The maximum length of nam_name is 12.
  */

  /* Optional */
  /*  Directory Number (information retrieved from NV_DIR_NUMBER_I) */
  uint8_t dir_num_valid;  /**< Must be set to true if dir_num is being passed */
  uint32_t dir_num_len;  /**< Must be set to # of elements in dir_num */
  char dir_num[NAS_MAX_3GPP2_SUBS_INFO_DIR_NUM_LEN_V01];
  /**<   Directory number in ASCII characters.
  */

  /* Optional */
  /*  Home SID/NID (information retrieved from NV_HOME_SID_NID_I) */
  uint8_t cdma_sys_id_valid;  /**< Must be set to true if cdma_sys_id is being passed */
  uint32_t cdma_sys_id_len;  /**< Must be set to # of elements in cdma_sys_id */
  nas_3gpp2_home_sid_nid_info_type_v01 cdma_sys_id[NAS_MAX_3GPP2_HOME_SID_NID_NUM_V01];

  /* Optional */
  /*  MIN-based IMSI (information retrieved from NV_IMSI_MCC_I, NV_IMSI_11_12_I, NV_MIN1_I, and NV_MIN2_I) */
  uint8_t min_based_info_valid;  /**< Must be set to true if min_based_info is being passed */
  nas_3gpp2_min_based_info_type_v01 min_based_info;

  /* Optional */
  /*  True IMSI (information retrieved from NV_IMSI_T_MCC_I, NV_IMSI_T_11_12_I, NV_IMSI_T_S1_I, NV_IMSI_T_S2_I, and NV_IMSI_T_ADDR_NUM_I) */
  uint8_t true_imsi_valid;  /**< Must be set to true if true_imsi is being passed */
  nas_3gpp2_true_imsi_info_type_v01 true_imsi;

  /* Optional */
  /*  CDMA Channel (information retrieved from NV_PCDMACH_I and NV_SCDMACH_I) */
  uint8_t cdma_channel_info_valid;  /**< Must be set to true if cdma_channel_info is being passed */
  nas_cdma_channel_info_type_v01 cdma_channel_info;

  /* Optional */
  /*  Mobile Directory Number (information retrieved from NV_DIR_NUMBER_PCS_I) */
  uint8_t mdn_valid;  /**< Must be set to true if mdn is being passed */
  uint32_t mdn_len;  /**< Must be set to # of elements in mdn */
  char mdn[MDN_MAX_LEN_V01];
  /**<   Mobile directory number represented in ASCII format with a maximum 
       length of 15 characters. Valid values for individual characters in the 
       MDN are digits 0 through 9, and special characters * and #.
  */
}nas_get_3gpp2_subscription_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Writes 3GPP2 subscription-related information. */
typedef struct {

  /* Mandatory */
  /*  NAM ID */
  uint8_t nam_id;
  /**<   NAM ID of the information to be written. The index starts from 0. 
       A nam_id of 0xFF is used to write information to current NAM.
  */

  /* Optional */
  /*  Directory Number (information written to NV_DIR_NUMBER_I) */
  uint8_t dir_num_valid;  /**< Must be set to true if dir_num is being passed */
  uint32_t dir_num_len;  /**< Must be set to # of elements in dir_num */
  char dir_num[NAS_MAX_3GPP2_SUBS_INFO_DIR_NUM_LEN_V01];
  /**<   Directory number in ASCII characters.
  */

  /* Optional */
  /*  Home SID/NID (information written to NV_HOME_SID_NID_I) */
  uint8_t cdma_sys_id_valid;  /**< Must be set to true if cdma_sys_id is being passed */
  uint32_t cdma_sys_id_len;  /**< Must be set to # of elements in cdma_sys_id */
  nas_3gpp2_home_sid_nid_info_type_v01 cdma_sys_id[NAS_MAX_3GPP2_HOME_SID_NID_NUM_V01];

  /* Optional */
  /*  MIN-based IMSI (information written to NV_IMSI_MCC_I, NV_IMSI_11_12_I, NV_MIN1_I, and NV_MIN2_I) */
  uint8_t min_based_info_valid;  /**< Must be set to true if min_based_info is being passed */
  nas_3gpp2_min_based_info_type_v01 min_based_info;

  /* Optional */
  /*  True IMSI (information written to NV_IMSI_T_MCC_I, NV_IMSI_T_11_12_I, NV_IMSI_T_S1_I, NV_IMSI_T_S2_I, and NV_IMSI_T_ADDR_NUM_I) */
  uint8_t true_imsi_valid;  /**< Must be set to true if true_imsi is being passed */
  nas_3gpp2_true_imsi_info_type_v01 true_imsi;

  /* Optional */
  /*  CDMA Channel (information written to NV_PCDMACH_I and NV_SCDMACH_I) */
  uint8_t cdma_channel_info_valid;  /**< Must be set to true if cdma_channel_info is being passed */
  nas_cdma_channel_info_type_v01 cdma_channel_info;

  /* Optional */
  /*  NAM Name (information written to NV_NAME_NAM_I) */
  uint8_t nam_name_valid;  /**< Must be set to true if nam_name is being passed */
  uint32_t nam_name_len;  /**< Must be set to # of elements in nam_name */
  char nam_name[NAS_MAX_NAM_NAME_LEN_V01];
  /**<   Name information in ASCII. The maximum length of nam_name is 12.
  */

  /* Optional */
  /*  Mobile Directory Number (information written to NV_DIR_NUMBER_PCS_I) */
  uint8_t mdn_valid;  /**< Must be set to true if mdn is being passed */
  uint32_t mdn_len;  /**< Must be set to # of elements in mdn */
  char mdn[MDN_MAX_LEN_V01];
  /**<   Mobile directory number represented in ASCII format with a maximum 
       length of 15 characters. Valid values for individual characters in the 
       MDN are digits 0 through 9, and special characters * and #.
  */

  /* Optional */
  /*  Service Programming Code */
  uint8_t spc_valid;  /**< Must be set to true if spc is being passed */
  char spc[NAS_SPC_MAX_V01];
  /**<   Service programming code in ASCII format (digits 0 to 9 only). 
       This TLV is required when any of the following TLVs are present: 
       Directory Number, Home SID/NID, MIN-based IMSI, CDMA Channel, 
       or Mobile Directory.
  */
}nas_set_3gpp2_subscription_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Writes 3GPP2 subscription-related information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_set_3gpp2_subscription_info_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_mob_cai_rev_req_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves Mobile CAI revision information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  CAI revision (information retrieved from NV_MOB_CAI_REV_I) */
  uint8_t cai_rev_valid;  /**< Must be set to true if cai_rev is being passed */
  uint8_t cai_rev;
  /**<   CAI revision. Values: \n
       - 0x01 -- P_REV_JSTD008 \n
       - 0x03 -- P_REV_IS95A \n
       - 0x04 -- P_REV_IS95B \n
       - 0x06 -- P_REV_IS2000 \n
       - 0x07 -- P_REV_IS2000_REL_A    \n
       - 0x08 -- P_REV_IS2000_REL_B    \n
       - 0x09 -- P_REV_IS2000_REL_C    \n
       - 0x0A -- P_REV_IS2000_REL_C_MI \n
       - 0x0B -- P_REV_IS2000_REL_D
  */
}nas_get_mob_cai_rev_resp_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_rtre_config_req_v01;

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_RTRE_CFG_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_RTRE_CFG_RUIM_ONLY_V01 = 0x01, 
  NAS_RTRE_CFG_INTERNAL_SETTINGS_ONLY_V01 = 0x02, 
  NAS_RTRE_CFG_RUIM_IF_AVAIL_V01 = 0x03, 
  NAS_RTRE_CFG_GSM_ON_1X_V01 = 0x04, 
  NAS_RTRE_CFG_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_rtre_cfg_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves current RTRE configuration information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Current RTRE Configuration */
  uint8_t rtre_cfg_valid;  /**< Must be set to true if rtre_cfg is being passed */
  nas_rtre_cfg_enum_v01 rtre_cfg;
  /**<   Values: \n
       -0x01 -- R-UIM only \n
       -0x02 -- Internal settings only \n
       -0x04 -- GSM on 1X
  */

  /* Optional */
  /*  RTRE Configuration Preference */
  uint8_t rtre_cfg_pref_valid;  /**< Must be set to true if rtre_cfg_pref is being passed */
  nas_rtre_cfg_enum_v01 rtre_cfg_pref;
  /**<   Values: \n
       -0x01 -- R-UIM only \n
       -0x02 -- Internal settings only \n
       -0x03 -- Use R-UIM if available \n
       -0x04 -- GSM on 1X
  */
}nas_get_rtre_config_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets RTRE configuration preference. */
typedef struct {

  /* Mandatory */
  /*  RTRE Configuration Preference */
  nas_rtre_cfg_enum_v01 rtre_cfg_pref;
  /**<   Values: \n
       -0x01 -- R-UIM only \n
       -0x02 -- Internal settings only \n
       -0x03 -- Use R-UIM if available \n
       -0x04 -- GSM on 1X (deprecated; will be converted to "Internal settings only" when used)
  */

  /* Optional */
  /*  Service Programming Code */
  uint8_t spc_valid;  /**< Must be set to true if spc is being passed */
  char spc[NAS_SPC_MAX_V01];
  /**<   Service programming code in ASCII format (digits 0 to 9 only). 
       This TLV is required when the RTRE Configuration Preference TLV 
       is present.
  */
}nas_set_rtre_config_req_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets RTRE configuration preference. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_set_rtre_config_resp_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_cell_location_info_req_msg_v01;

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Cell id */
  uint32_t nmr_cell_id;
  /**<   Cell ID (0xFFFFFFFF indicates cell ID information is not present).
  */

  /*  PLMN */
  char nmr_plmn[NAS_PLMN_LEN_V01];
  /**<   MCC/MNC information coded as octet 3, 4, and 5 in 
       \hyperref[S5]{[S5]} Section 10.5.1.3. 
       (This field is ignored when nmr_cell_id is not present.)
  */

  /*  LAC */
  uint16_t nmr_lac;
  /**<   Location area code. (This field is ignored when nmr_cell_id is not present.)
  */

  /*  ARFCN */
  uint16_t nmr_arfcn;
  /**<   Absolute RF channel number.
  */

  /*  BSIC */
  uint8_t nmr_bsic;
  /**<   Base station identity code.
  */

  /*  Rx Lev */
  uint16_t nmr_rx_lev;
  /**<   Cell Rx measurement. Values range between 0 and 63, which is 
       mapped to a measured signal level: \n

       - Rxlev 0 is a signal strength less than -110 dBm \n
       - Rxlev 1 is -110 dBm to -109 dBm    \n
       - Rxlev 2 is -109 dBm to -108 dBm    \n
       - ...                                \n
       - Rxlev 62 is -49 dBm to -48 dBm     \n
       - Rxlev 63 is greater than -48 dBm
  */
}nas_nmr_cell_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Cell id */
  uint32_t cell_id;
  /**<   Cell ID (0xFFFFFFFF indicates cell ID information is not present).
  */

  /*  PLMN */
  char plmn[NAS_PLMN_LEN_V01];
  /**<   MCC/MNC information coded as octet 3, 4, and 5 in 
       \hyperref[S5]{[S5]} Section 10.5.1.3. 
       (This field is ignored when cell_id is not present.)
  */

  /*  LAC */
  uint16_t lac;
  /**<   Location area code. (This field is ignored when cell_id is not present.)
  */

  /*  ARFCN */
  uint16_t arfcn;
  /**<   Absolute RF channel number.
  */

  /*  BSIC */
  uint8_t bsic;
  /**<   Base station identity code.
  */

  /*  Timing Advance */
  uint32_t timing_advance;
  /**<   Measured delay (in bit periods; 1 bit period = 48/13 microsecond) of 
       an access burst transmission on the RACH or PRACH to the expected signal 
       from an MS at zero distance under static channel conditions.
       (0xFFFFFFFF indicates timing advance information is not present.)
  */

  /*  Rx Lev */
  uint16_t rx_lev;
  /**<   Serving cell Rx measurement. Values range between 0 and 63, which is 
       mapped to a measured signal level: \n

       - Rxlev 0 is a signal strength less than -110 dBm \n
       - Rxlev 1 is -110 dBm to -109 dBm    \n
       - Rxlev 2 is -109 dBm to -108 dBm    \n
       - ...                               \n
       - Rxlev 62 is -49 dBm to -48 dBm     \n
       - Rxlev 63 is greater than -48 dBm
  */

  /*  Neighbor cell information  */
  uint32_t nmr_cell_info_len;  /**< Must be set to # of elements in nmr_cell_info */
  nas_nmr_cell_info_type_v01 nmr_cell_info[NAS_NMR_MAX_NUM_V01];
  /**<   Contains information only if neighbors are present; 
       includes: \n
       - nmr_cell_id \n
       - nmr_plmn \n
       - nmr_lac \n
       - nmr_arfcn \n
       - nmr_bsic \n
       - nmr_rx_lev
  */
}nas_geran_cell_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  UARFCN */
  uint16_t umts_uarfcn;
  /**<   UTRA absolute RF channel number.
  */

  /*  PSC */
  uint16_t umts_psc;
  /**<   Primary scrambling code.
  */

  /*  RSCP */
  int16_t umts_rscp;
  /**<   Received signal code power; the received power on one code measured in 
       dBm on the primary CPICH channel of the neighbor/monitored cell.
  */

  /*  Ec/Io */
  int16_t umts_ecio;
  /**<   ECIO; the received energy per chip divided by the power density in the 
       band measured in dBm on the primary CPICH channel of the 
       neighbor/monitored cell.
  */
}nas_umts_monitored_cell_set_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  UARFCN */
  uint16_t geran_arfcn;
  /**<   Absolute RF channel number.
  */

  /*  BSIC NCC */
  uint8_t geran_bsic_ncc;
  /**<   Base station identity code network color code
       (0xFF indicates information is not present).
  */

  /*  BSIC BCC */
  uint8_t geran_bsic_bcc;
  /**<   Base station identity code base station color code
       (0xFF indicates information is not present).
  */

  /*  GERAN RSSI */
  int16_t geran_rssi;
  /**<   Received signal strength indicator.
  */
}nas_umts_geran_nbr_cell_set_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  EARFCN */
  uint16_t earfcn;
  /**<   E-UTRA absolute RF channel number of the detected cell.   */

  /*  Physical Cell id */
  uint16_t pci;
  /**<   Physical cell ID of the detected cell. 
       Range is defined in \hyperref[S18]{[S18]}. */

  /*  RSRP */
  float rsrp;
  /**<   Current received signal strength indication (in dBm) of the detected 
       cell. 
  */

  /*  RSRQ */
  float rsrq;
  /**<   Current reference signal received quality (in dB) of the detected cell. */

  /*  RX Level */
  int16_t srxlev;
  /**<   Cell selection Rx level (Srxlev) value of the detected cell in linear 
       scale. (This field is only valid when wcdma_rrc_state is not 
       NAS_WCDMA_RRC_STATE_CELL_FACH or NAS_WCDMA_RRC_STATE_CELL_DCH.) 
  */

  /*  Cell is TDD */
  uint8_t cell_is_tdd;
  /**<   TRUE if the cell is TDD; FALSE if the cell is FDD. */
}nas_umts_lte_nbr_cell_set_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Cell id */
  uint16_t cell_id;
  /**<   Cell ID (0xFFFFFFFF indicates cell ID information is not present).
  */

  /*  PLMN */
  char plmn[NAS_PLMN_LEN_V01];
  /**<   MCC/MNC information coded as octet 3, 4, and 5 in 
       \hyperref[S5]{[S5]} Section 10.5.1.3. 
  */

  /*  LAC */
  uint16_t lac;
  /**<   Location area code.
  */

  /*  UARFCN */
  uint16_t uarfcn;
  /**<   UTRA absolute RF channel number.
  */

  /*  PSC */
  uint16_t psc;
  /**<   Primary scrambling code.
  */

  /*  RSCP */
  int16_t rscp;
  /**<   Received signal code power; the received power on one code measured in
       dBm on the primary CPICH channel of the serving cell.
  */

  /*  Ec/Io */
  int16_t ecio;
  /**<   ECIO; the received energy per chip divided by the power density in the
       band measured in dBm on the primary CPICH channel of the serving cell.
  */

  /*  UMTS Monitored Cell info set */
  uint32_t umts_monitored_cell_len;  /**< Must be set to # of elements in umts_monitored_cell */
  nas_umts_monitored_cell_set_info_type_v01 umts_monitored_cell[NAS_UMTS_MAX_MONITORED_CELL_SET_NUM_V01];

  /*  GERAN Neighbor cell info set */
  uint32_t umts_geran_nbr_cell_len;  /**< Must be set to # of elements in umts_geran_nbr_cell */
  nas_umts_geran_nbr_cell_set_info_type_v01 umts_geran_nbr_cell[NAS_UMTS_GERAN_MAX_NBR_CELL_SET_NUM_V01];
}nas_umts_cell_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t sid;
  /**<   System ID. */

  uint16_t nid;
  /**<   Network ID. */

  uint16_t base_id;
  /**<   Base station ID. */

  uint16_t refpn;
  /**<   Reference PN. */

  uint32_t base_lat;
  /**<   Latitude of the current base station in units of 0.25 sec. */

  uint32_t base_long;
  /**<   Longitude of the current base station in units of 0.25 sec. */
}nas_cdma_cell_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t pci;
  /**<   Physical cell ID. Range: 0 to 503. */

  int16_t rsrq;
  /**<   Current RSRQ in 1/10 dB as measured by L1.  
    Range: -200 to -30 (e.g., -200 means -20.0 dB). */

  int16_t rsrp;
  /**<   Current RSRP in 1/10 dBm as measured by L1. 
    Range: -1400 to -440 (e.g., -440 means -44.0 dBm). */

  int16_t rssi;
  /**<   Current RSSI in 1/10 dBm as measured by L1.
    Range: -1200 to 0 (e.g., -440 means -44.0 dBm). */

  int16_t srxlev;
  /**<   Cell selection Rx level (Srxlev) value. Range: -128 to 128. 
    (This field is only valid when ue_in_idle is TRUE.) */
}nas_lte_ngbr_cell_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t ue_in_idle;
  /**<   TRUE if the UE is in Idle mode; otherwise FALSE. */

  uint8_t plmn[NAS_PLMN_LEN_V01];
  /**<   PLMN ID coded as octet 3, 4, and 5 in 
     \hyperref[S5]{[S5]} Section 10.5.1.3. */

  uint16_t tac;
  /**<   Tracking area code. */

  uint32_t global_cell_id;
  /**<   Global cell ID in the system information block. */

  uint16_t earfcn;
  /**<   E-UTRA absolute radio frequency channel number of the serving cell. 
     Range: 0 to 65535. */

  uint16_t serving_cell_id;
  /**<   LTE serving cell ID. Range: 0 to 503. This is the cell ID of the 
    serving cell and can be found in the cell list. */

  uint8_t cell_resel_priority;
  /**<   Priority for serving frequency. Range: 0 to 7. (This field is only 
    valid when ue_in_idle is TRUE.) */

  uint8_t s_non_intra_search;
  /**<   S non-intra search threshold to control non-intrafrequency searches. 
    Range: 0 to 31. (This field is only valid when ue_in_idle is TRUE.) */

  uint8_t thresh_serving_low;
  /**<   Serving cell low threshold. Range: 0 to 31. (This field is only 
    valid when ue_in_idle is TRUE.) */

  uint8_t s_intra_search;
  /**<   S intra search threshold. Range: 0 to 31. The current cell 
    measurement must fall below this threshold to consider intrafrequency 
    for reselection. (This field is only valid when ue_in_idle is TRUE.) */

  uint32_t cells_len;  /**< Must be set to # of elements in cells */
  nas_lte_ngbr_cell_type_v01 cells[NAS_MAX_LTE_NGBR_NUM_CELLS_V01];
}nas_lte_intra_freq_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t earfcn;
  /**<   E-UTRA absolute radio frequency channel number. Range: 0 to 65535. */

  uint8_t threshX_low;
  /**<   Cell Srxlev low threshold. Range: 0 to 31.  
    When the serving cell does not exceed thresh_serving_low, 
    the value of an evaluated cell must be smaller than this value to be 
    considered for reselection. */

  uint8_t threshX_high;
  /**<   Cell Srxlev high threshold. Range: 0 to 31. 
    When the serving cell exceeds thresh_serving_low, 
    the value of an evaluated cell must be greater than this value to be 
    considered for reselection. */

  uint8_t cell_resel_priority;
  /**<   Cell reselection priority. Range: 0 to 7. (This field is only valid 
    when ue_in_idle is TRUE.) */

  uint32_t cells_len;  /**< Must be set to # of elements in cells */
  nas_lte_ngbr_cell_type_v01 cells[NAS_MAX_LTE_NGBR_NUM_CELLS_V01];
}nas_lte_inter_freq_freqs_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t ue_in_idle;
  /**<   TRUE if the UE is in Idle mode; otherwise FALSE. */

  uint32_t freqs_len;  /**< Must be set to # of elements in freqs */
  nas_lte_inter_freq_freqs_type_v01 freqs[NAS_MAX_LTE_NGBR_NUM_FREQS_V01];
}nas_lte_inter_freq_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t arfcn;
  /**<   GSM frequency being reported. Range: 0 to 1023. */

  uint8_t band_1900;
  /**<   Band indicator for the GSM ARFCN (this field is only valid if arfcn 
    is in the overlapping region). If TRUE and the cell is in the overlapping 
    region, the ARFCN is on the 1900 band. If FALSE, it is on the 1800 band. */

  uint8_t cell_id_valid;
  /**<   Flag indicating whether the base station identity code ID is valid. */

  uint8_t bsic_id;
  /**<   Base station identity code ID, including base station color code and
    network color code. The lower 6 bits can be set to any value. */

  int16_t rssi;
  /**<   Measured RSSI value in 1/10 dB. 
    Range: -2000 to 0  (e.g., -800 means -80.0 dB). */

  int16_t srxlev;
  /**<   Cell selection Rx level (Srxlev) value. Range: -128 to 128. 
    (This field is only valid when ue_in_idle is TRUE.) */
}nas_lte_ngbr_gsm_cell_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t cell_resel_priority;
  /**<   Priority of this frequency group. Range: 0 to 7. (This field is only 
    valid when ue_in_idle is TRUE.) */

  uint8_t thresh_gsm_high;
  /**<   Reselection threshold for high priority layers. Range: 0 to 31. 
    (This field is only valid when ue_in_idle is TRUE.) */

  uint8_t thresh_gsm_low;
  /**<   Reselection threshold for low priority layers. Range: 0 to 31. 
    (This field is only valid when ue_in_idle is TRUE.) */

  uint8_t ncc_permitted;
  /**<    Bitmask specifying whether a neighbor with a specific network color
    code is to be reported. Range: 0 to 255. Bit n set to 1 means a neighbor 
    with NCC n must be included in the report. This flag is synonymous with a
    blacklist in other RATs. (This field is only valid when ue_in_idle is
    TRUE.) */

  uint32_t cells_len;  /**< Must be set to # of elements in cells */
  nas_lte_ngbr_gsm_cell_type_v01 cells[NAS_MAX_LTE_NGBR_GSM_NUM_CELLS_V01];
}nas_lte_ngbr_gsm_freq_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t ue_in_idle;
  /**<   TRUE if the UE is in Idle mode; otherwise FALSE. */

  uint32_t freqs_len;  /**< Must be set to # of elements in freqs */
  nas_lte_ngbr_gsm_freq_type_v01 freqs[NAS_MAX_LTE_NGBR_GSM_NUM_FREQS_V01];
}nas_lte_ngbr_gsm_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t psc;
  /**<   Primary scrambling code. Range: 0 to 511. */

  int16_t cpich_rscp;
  /**<   Absolute power level (in 1/10 dBm) of the common pilot channel as 
    received by the UE. Range: -1200 to -250 (e.g., -250 means -25.0 dBm). 
    Defined in \hyperref[S14]{[S14]}. */

  int16_t cpich_ecno;
  /**<   CPICH Ec/No; ratio (in 1/10 dB) of the received energy per PN chip for 
    the CPICH to the total received power spectral density at the UE antenna 
    connector. Range: -500 to 0 (e.g., -25 means -2.5 dB). Defined in 
    \hyperref[S14]{[S14]}. */

  int16_t srxlev;
  /**<   Cell selection Rx level (Srxlev) value. Range: -128 to 128. 
    (This field is only valid when ue_in_idle is TRUE.) */
}nas_lte_ngbr_wcdma_cell_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t uarfcn;
  /**<   WCDMA layer frequency. Range: 0 to 16383. */

  uint8_t cell_resel_priority;
  /**<   Cell reselection priority. Range: 0 to 7. (This field is only 
    valid when ue_in_idle is TRUE.) */

  uint16_t thresh_Xhigh;
  /**<   Reselection low threshold. Range: 0 to 31. (This field is only 
    valid when ue_in_idle is TRUE.) */

  uint16_t thresh_Xlow;
  /**<   Reselection high threshold. Range: 0 to 31. (This field is only 
    valid when ue_in_idle is TRUE.) */

  uint32_t cells_len;  /**< Must be set to # of elements in cells */
  nas_lte_ngbr_wcdma_cell_type_v01 cells[NAS_MAX_LTE_NGBR_WCDMA_NUM_CELLS_V01];
}nas_lte_ngbr_wcdma_freq_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t ue_in_idle;
  /**<   TRUE if the UE is in Idle mode; otherwise FALSE. */

  uint32_t freqs_len;  /**< Must be set to # of elements in freqs */
  nas_lte_ngbr_wcdma_freq_type_v01 freqs[NAS_MAX_LTE_NGBR_WCDMA_NUM_FREQS_V01];
}nas_lte_ngbr_wcdma_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_WCDMA_RRC_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_WCDMA_RRC_STATE_DISCONNECTED_V01 = 0x00, 
  NAS_WCDMA_RRC_STATE_CELL_PCH_V01 = 0x01, 
  NAS_WCDMA_RRC_STATE_URA_PCH_V01 = 0x02, 
  NAS_WCDMA_RRC_STATE_CELL_FACH_V01 = 0x03, 
  NAS_WCDMA_RRC_STATE_CELL_DCH_V01 = 0x04, 
  NAS_WCDMA_RRC_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_wcdma_rrc_state_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  WCDMA RRC state */
  nas_wcdma_rrc_state_enum_v01 wcdma_rrc_state;
  /**<   WCDMA RRC states. Values: \n
        -0x00 -- NAS_WCDMA_RRC_STATE_DISCONNECTED -- WCDMA RRC state is IDLE
                defined in \hyperref[S17]{[S17]} \n
        -0x01 -- NAS_WCDMA_RRC_STATE_CELL_PCH -- WCDMA RRC state is CELL_PCH
                defined in \hyperref[S17]{[S17]} \n
        -0x02 -- NAS_WCDMA_RRC_STATE_URA_PCH -- WCDMA RRC state is URA_PCH
                defined in \hyperref[S17]{[S17]} \n
        -0x03 -- NAS_WCDMA_RRC_STATE_CELL_FACH -- WCDMA RRC state is CELL_FACH
                defined in \hyperref[S17]{[S17]} \n
        -0x04 -- NAS_WCDMA_RRC_STATE_CELL_DCH -- WCDMA RRC state is CELL_DCH
                defined in \hyperref[S17]{[S17]}
  */

  uint32_t umts_lte_nbr_cell_len;  /**< Must be set to # of elements in umts_lte_nbr_cell */
  nas_umts_lte_nbr_cell_set_info_type_v01 umts_lte_nbr_cell[NAS_UMTS_LTE_MAX_NBR_CELL_SET_NUM_V01];
}nas_wcdma_ngbr_lte_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  RX Power 0 */
  float rx0_agc;
  /**<   Rx power 0 in dB.
  */

  /*  RX Power 1 */
  float rx1_agc;
  /**<   Rx power 1 in dB.
  */
}nas_rx_power_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Timing Advance Value */
  uint16_t g_ta;
  /**<   Range of the UE from the base station in steps.
  */

  /*  Channel Frequency Number */
  uint16_t g_bcch;
  /**<   Channel number assigned to the frequency.
  */
}nas_gsm_cell_info_ext_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  WCDMA Power */
  float w_agc;
  /**<   Power in dB.
  */

  /*  WCDMA TX Power */
  float w_txagc;
  /**<   Tx power in dB.
  */

  /*  DL Bler */
  uint16_t w_dl_bler;
  /**<   Downlink block error rate percentage.
  */
}nas_wcdma_cell_info_ext_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  PSC */
  uint16_t psc;
  /**<   Primary scrambling code. */

  uint32_t cell_id;
  /**<   Cell ID. */

  /*  RSCP */
  int16_t rscp;
  /**<   Received signal code power; the received power on one code measured in
       dBm on the primary CPICH channel of the active set cell.
  */

  /*  Ec/Io */
  int16_t ecio;
  /**<   ECIO; the received energy per chip divided by the power density in the
       band measured in dBm on the primary CPICH channel of the active set 
       cell.
  */

  /*  UARFCN */
  uint16_t uarfcn;
  /**<   UTRA absolute RF channel number.
  */
}nas_wcdma_active_set_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Cell ID */
  uint32_t cell_id;
  /**<   Cell ID (0xFFFFFFFF indicates cell ID information is not present).
  */

  /*  PLMN */
  char plmn[NAS_PLMN_LEN_V01];
  /**<   MCC/MNC information coded as octet 3, 4, and 5 in 
       \hyperref[S5]{[S5]} Section 10.5.1.3. 
  */

  /*  LAC */
  uint16_t lac;
  /**<   Location area code.
  */

  /*  UARFCN */
  uint16_t uarfcn;
  /**<   UTRA absolute RF channel number.
  */

  /*  PSC */
  uint16_t psc;
  /**<   Primary scrambling code.
  */

  /*  RAC   */
  uint16_t rac;
  /**<   Routing area code. */
}nas_wcdma_active_set_reference_rl_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  UARFCN */
  uint16_t umts_uarfcn;
  /**<   UTRA absolute RF channel number.
  */

  /*  PSC */
  uint16_t umts_psc;
  /**<   Primary scrambling code.
  */

  /*  RSCP */
  int16_t umts_rscp;
  /**<   Received signal code power; the received power on one code measured in 
       dBm on the primary CPICH channel of the neighbor/monitored cell.
  */

  /*  ECIO */
  int16_t umts_ecio;
  /**<   ECIO; the received energy per chip divided by the power density in the 
       band measured in dBm on the primary CPICH channel of the 
       neighbor/monitored cell.
  */

  /*  Squal */
  int16_t umts_squal;
  /**<   Squal; Cell Selection quality value in dB
  */

  /*  Srxlev */
  int16_t umts_srxlev;
  /**<   Srxlev; Cell Selection RX level value in dB
  */

  /*  Rank */
  int16_t umts_rank;
  /**<   Rank of the cell 
   */

  /*  Set */
  uint8_t umts_set;
  /**<   Set of the cell
  */
}nas_umts_monitored_cell_set_ext_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_WCDMA_L1_SF_E_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_WCDMA_L1_SF_4_V01 = 0x00, 
  NAS_WCDMA_L1_SF_8_V01 = 0x01, 
  NAS_WCDMA_L1_SF_16_V01 = 0x02, 
  NAS_WCDMA_L1_SF_32_V01 = 0x03, 
  NAS_WCDMA_L1_SF_64_V01 = 0x04, 
  NAS_WCDMA_L1_SF_128_V01 = 0x05, 
  NAS_WCDMA_L1_SF_256_V01 = 0x06, 
  NAS_WCDMA_L1_SF_512_V01 = 0x07, 
  NAS_WCDMA_L1_NUM_SF_V01 = 0x08, 
  NAS_WCDMA_L1_SF_E_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_wcdma_l1_sf_e_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_WCDMA_L1_DL_PHYCHAN_E_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_WCDMA_L1_DL_PHYCHAN_PCCPCH_S_V01 = 0x00, 
  NAS_WCDMA_L1_DL_PHYCHAN_PCCPCH_N_V01 = 0x01, 
  NAS_WCDMA_L1_DL_PHYCHAN_SCCPCH0_V01 = 0x02, 
  NAS_WCDMA_L1_DL_PHYCHAN_SCCPCH1_V01 = 0x03, 
  NAS_WCDMA_L1_DL_PHYCHAN_PICH_V01 = 0x04, 
  NAS_WCDMA_L1_DL_PHYCHAN_AICH_V01 = 0x05, 
  NAS_WCDMA_L1_DL_PHYCHAN_HS_RACH_AICH_V01 = 0x06, 
  NAS_WCDMA_L1_DL_PHYCHAN_DPCH_V01 = 0x07, 
  NAS_WCDMA_L1_DL_PHYCHAN_HS_RACH_FDPCH_V01 = 0x08, 
  NAS_WCDMA_L1_DL_PHYCHAN_FDPCH_V01 = 0x09, 
  NAS_WCDMA_L1_DL_PHYCHAN_PDSCH_V01 = 0x0A, 
  NAS_WCDMA_L1_NUM_DL_PHYCHAN_V01 = 0x0B, 
  NAS_WCDMA_L1_DL_PHYCHAN_NOCHAN_V01 = 0x0C, 
  NAS_WCDMA_L1_DL_PHYCHAN_E_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_wcdma_l1_dl_phychan_e_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  UARFCN */
  uint16_t geran_arfcn;
  /**<   Absolute RF channel number.
  */

  /*  BSIC NCC */
  uint8_t geran_bsic_ncc;
  /**<   Base station identity code network color code
       (0xFF indicates information is not present).
  */

  /*  BSIC BCC */
  uint8_t geran_bsic_bcc;
  /**<   Base station identity code base station color code
       (0xFF indicates information is not present).
  */

  /*  GERAN RSSI */
  int16_t geran_rssi;
  /**<   Received signal strength indicator.
  */

  /*  GERAN_RANK */
  int16_t geran_rank;
  /**<   Rank of the cell 
  */
}nas_umts_geran_nbr_cell_set_ext_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Cell ID */
  uint16_t cell_id;
  /**<   Cell ID (0xFFFFFFFF indicates cell ID information is not present).
  */

  /*  PLMN */
  char plmn[NAS_PLMN_LEN_V01];
  /**<   MCC/MNC information coded as octet 3, 4, and 5 in 
       \hyperref[S5]{[S5]} Section 10.5.1.3. 
  */

  /*  LAC */
  uint16_t lac;
  /**<   Location area code.
  */

  /*  UARFCN */
  uint16_t uarfcn;
  /**<   UTRA absolute RF channel number.
  */

  /*  PSC */
  uint16_t psc;
  /**<   Primary scrambling code.
  */

  /*  RSCP */
  int16_t rscp;
  /**<   Received signal code power; the received power on one code measured in
       dBm on the primary CPICH channel of the serving cell.
  */

  /*  ECIO */
  int16_t ecio;
  /**<   ECIO; the received energy per chip divided by the power density in the
       band measured in dBm on the primary CPICH channel of the serving cell.
  */

  /*  Squal  */
  int16_t squal;
  /**<   Squal; Cell Selection quality value in dB
  */

  /*  Srxlev */
  int16_t srxlev;
  /**<   Srxlev; Cell Selection RX level value in dB
  */

  /*  UMTS Monitored Cell Information Set */
  uint32_t umts_monitored_ext_ext_cell_len;  /**< Must be set to # of elements in umts_monitored_ext_ext_cell */
  nas_umts_monitored_cell_set_ext_info_type_v01 umts_monitored_ext_ext_cell[NAS_UMTS_MAX_MONITORED_CELL_SET_NUM_V01];

  /*  GERAN Neighbor Cell Information Set */
  uint32_t umts_geran_ext_nbr_cell_len;  /**< Must be set to # of elements in umts_geran_ext_nbr_cell */
  nas_umts_geran_nbr_cell_set_ext_info_type_v01 umts_geran_ext_nbr_cell[NAS_UMTS_GERAN_MAX_NBR_CELL_SET_NUM_V01];
}nas_umts_cell_ext_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  PSC */
  uint16_t psc;
  /**<   Primary scrambling code. */

  /*  Cell ID */
  uint32_t cell_id;
  /**<   Cell ID. */

  /*  RSCP */
  int16_t rscp;
  /**<   Received signal code power; the received power on one code measured in
       dBm on the primary CPICH channel of the active set cell.
  */

  /*  ECIO */
  int16_t ecio;
  /**<   ECIO; the received energy per chip divided by the power density in the
       band measured in dBm on the primary CPICH channel of the active set 
       cell.
  */

  /*  UARFCN */
  uint16_t uarfcn;
  /**<   UTRA absolute RF channel number.
  */

  /*  Spreading Factor of the Channel */
  nas_wcdma_l1_sf_e_type_v01 sf;
  /**<    Spreading factor of the channel. Values: \n
	   - 0x00 -- NAS_WCDMA_L1_SF_4 \n
	   - 0x01 -- NAS_WCDMA_L1_SF_8 \n
	   - 0x02 -- NAS_WCDMA_L1_SF_16 \n
	   - 0x03 -- NAS_WCDMA_L1_SF_32 \n
	   - 0x04 -- NAS_WCDMA_L1_SF_64 \n
	   - 0x05 -- NAS_WCDMA_L1_SF_128 \n
	   - 0x06 -- NAS_WCDMA_L1_SF_256 \n
	   - 0x07 -- NAS_WCDMA_L1_SF_512 \n
	   - 0x08 -- NAS_WCDMA_L1_NUM_SF
  */

  /*  Physical Channel Type FDPCH/DPCH */
  nas_wcdma_l1_dl_phychan_e_type_v01 phy_chan_type;
  /**<    Physical channel type. Values: \n
	   - 0x00 -- NAS_WCDMA_L1_DL_PHYCHAN_PCCPCH_S \n
	   - 0x01 -- NAS_WCDMA_L1_DL_PHYCHAN_PCCPCH_N \n
	   - 0x02 -- NAS_WCDMA_L1_DL_PHYCHAN_SCCPCH0 \n
	   - 0x03 -- NAS_WCDMA_L1_DL_PHYCHAN_SCCPCH1 \n
	   - 0x04 -- NAS_WCDMA_L1_DL_PHYCHAN_PICH \n
	   - 0x05 -- NAS_WCDMA_L1_DL_PHYCHAN_AICH \n
	   - 0x06 -- NAS_WCDMA_L1_DL_PHYCHAN_HS_RACH_AICH \n
	   - 0x07 -- NAS_WCDMA_L1_DL_PHYCHAN_DPCH \n
	   - 0x08 -- NAS_WCDMA_L1_DL_PHYCHAN_HS_RACH_FDPCH \n
	   - 0x09 -- NAS_WCDMA_L1_DL_PHYCHAN_FDPCH \n
	   - 0x0A -- NAS_WCDMA_L1_DL_PHYCHAN_PDSCH \n
	   - 0x0B -- NAS_WCDMA_L1_NUM_DL_PHYCHAN \n
	   - 0x0C -- NAS_WCDMA_L1_DL_PHYCHAN_NOCHAN
  */

  /*  Slot Format for the Channel  */
  uint8_t slot_format;
  /**<   
      Indicates slot format. Values range between 0 and 6 as per 25.211.
  */

  /*  Indicates CM On or Not */
  uint8_t is_compressed_mode_on;
  /**<   
      Indicates whether the compressed mode is ON or OFF. 
  */
}nas_wcdma_active_set_ext_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Cell ID */
  uint32_t nmr_cell_id;
  /**<   Cell ID (0xFFFFFFFF indicates cell ID information is not present).
  */

  /*  PLMN */
  char nmr_plmn[NAS_PLMN_LEN_V01];
  /**<   MCC/MNC information coded as octet 3, 4, and 5 in 
       \hyperref[S5]{[S5]} Section 10.5.1.3. 
       (This field is ignored when nmr_cell_id is not present.)
  */

  /*  LAC */
  uint16_t nmr_lac;
  /**<   Location area code. (This field is ignored when nmr_cell_id is not present.)
  */

  /*  ARFCN */
  uint16_t nmr_arfcn;
  /**<   Absolute RF channel number.
  */

  /*  BSIC */
  uint8_t nmr_bsic;
  /**<   Base station identity code.
  */

  /*  Rx Lev */
  uint16_t nmr_rx_lev;
  /**<   Cell Rx measurement. Values range between 0 and 63, which is 
       mapped to a measured signal level: \n

       - Rxlev 0 is a signal strength less than -110 dBm \n
       - Rxlev 1 is -110 dBm to -109 dBm    \n
       - Rxlev 2 is -109 dBm to -108 dBm    \n
       - ...                                \n
       - Rxlev 62 is -49 dBm to -48 dBm     \n
       - Rxlev 63 is greater than -48 dBm
  */

  /*  Nmr_c1 */
  int32_t nmr_c1;
  /**<   Nmr_c1 as defined in 45.008 6.4, default: 0
  */

  /*  Nmr_c2 */
  int32_t nmr_c2;
  /**<   Nmr_c2 as defined in 45.008 6.4, default: 0
  */

  /*  Nmr_c31 */
  int32_t nmr_c31;
  /**<   Nmr_c31 as defined in 45.008 10.1.2, default: 0
  */

  /*  Nmr_c32 */
  int32_t nmr_c32;
  /**<   Nmr_c32 as defined in 45.008 10.1.2, default: 0
  */
}nas_nmr_cell_info_ext_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Cell ID */
  uint32_t cell_id;
  /**<   Cell ID (0xFFFFFFFF indicates cell ID information is not present).
  */

  /*  PLMN */
  char plmn[NAS_PLMN_LEN_V01];
  /**<   MCC/MNC information coded as octet 3, 4, and 5 in 
       \hyperref[S5]{[S5]} Section 10.5.1.3. 
       (This field is ignored when cell_id is not present.)
  */

  /*  LAC */
  uint16_t lac;
  /**<   Location area code. (This field is ignored when cell_id is not present.)
  */

  /*  ARFCN */
  uint16_t arfcn;
  /**<   Absolute RF channel number.
  */

  /*  BSIC */
  uint8_t bsic;
  /**<   Base station identity code.
  */

  /*  Timing Advance */
  uint32_t timing_advance;
  /**<   Measured delay (in bit periods; 1 bit period = 48/13 microsecond) of 
       an access burst transmission on the RACH or PRACH to the expected signal 
       from an MS at zero distance under static channel conditions.
       (0xFFFFFFFF indicates timing advance information is not present.)
  */

  /*  Rx Lev */
  uint16_t rx_lev;
  /**<   Serving cell Rx measurement. Values range between 0 and 63, which is 
       mapped to a measured signal level: \n

       - Rxlev 0 is a signal strength less than -110 dBm \n
       - Rxlev 1 is -110 dBm to -109 dBm    \n
       - Rxlev 2 is -109 dBm to -108 dBm    \n
       - ...                               \n
       - Rxlev 62 is -49 dBm to -48 dBm     \n
       - Rxlev 63 is greater than -48 dBm
  */

  /*  Neighbor Cell Information  */
  uint32_t nmr_cell_info_len;  /**< Must be set to # of elements in nmr_cell_info */
  nas_nmr_cell_info_ext_type_v01 nmr_cell_info[NAS_NMR_MAX_NUM_V01];
  /**<   Contains information only if neighbors are present; 
       includes: \n
       - nmr_cell_id \n
       - nmr_plmn \n
       - nmr_lac \n
       - nmr_arfcn \n
       - nmr_bsic \n
       - nmr_rx_lev \n
       - nmr_c1 \n
       - nmr_c2 \n
       - nmr_c31 \n
       - nmr_c32 \n
  */
}nas_geran_cell_info_ext_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t pbcch_present;
  /**<   Presence of PBCCH in cell:
       - 0 -- No.
       - 1 -- Yes.
       - 0xff -- Invalid. */

  uint8_t gprs_rxlev_access_min;
  /**<   Range 0..63, 0xff - invalid, Reference 45.008
  */

  uint8_t gprs_ms_txpwr_max_cch;
  /**<   Range 0..31, 0xff - invalid, Reference 45.005, 45.008
  */
}nas_geran_eng_mode_scell_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves cell location-related information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  GERAN Info */
  uint8_t geran_info_valid;  /**< Must be set to true if geran_info is being passed */
  nas_geran_cell_info_type_v01 geran_info;

  /* Optional */
  /*  UMTS Info */
  uint8_t umts_info_valid;  /**< Must be set to true if umts_info is being passed */
  nas_umts_cell_info_type_v01 umts_info;

  /* Optional */
  /*  CDMA Info */
  uint8_t cdma_info_valid;  /**< Must be set to true if cdma_info is being passed */
  nas_cdma_cell_info_type_v01 cdma_info;

  /* Optional */
  /*  LTE Info - Intrafrequency */
  uint8_t lte_intra_valid;  /**< Must be set to true if lte_intra is being passed */
  nas_lte_intra_freq_type_v01 lte_intra;

  /* Optional */
  /*  LTE Info - Interfrequency */
  uint8_t lte_inter_valid;  /**< Must be set to true if lte_inter is being passed */
  nas_lte_inter_freq_type_v01 lte_inter;

  /* Optional */
  /*  LTE Info - Neighboring GSM */
  uint8_t lte_gsm_valid;  /**< Must be set to true if lte_gsm is being passed */
  nas_lte_ngbr_gsm_type_v01 lte_gsm;

  /* Optional */
  /*  LTE Info - Neighboring WCDMA */
  uint8_t lte_wcdma_valid;  /**< Must be set to true if lte_wcdma is being passed */
  nas_lte_ngbr_wcdma_type_v01 lte_wcdma;

  /* Optional */
  /*  UMTS Cell ID */
  uint8_t umts_cell_id_valid;  /**< Must be set to true if umts_cell_id is being passed */
  uint32_t umts_cell_id;
  /**<   Cell ID (0xFFFFFFFF indicates cell ID information is not present).
  */

  /* Optional */
  /*  WCDMA Info - LTE Neighbor Cell Info Set */
  uint8_t wcdma_lte_valid;  /**< Must be set to true if wcdma_lte is being passed */
  nas_wcdma_ngbr_lte_type_v01 wcdma_lte;

  /* Optional */
  /*  CDMA Rx Info */
  uint8_t cdma_rx_power_valid;  /**< Must be set to true if cdma_rx_power is being passed */
  nas_rx_power_type_v01 cdma_rx_power;

  /* Optional */
  /*  HDR Rx Info */
  uint8_t hdr_rx_power_valid;  /**< Must be set to true if hdr_rx_power is being passed */
  nas_rx_power_type_v01 hdr_rx_power;

  /* Optional */
  /*  GSM Cell Info Ext */
  uint8_t gsm_info_ext_valid;  /**< Must be set to true if gsm_info_ext is being passed */
  nas_gsm_cell_info_ext_type_v01 gsm_info_ext;

  /* Optional */
  /*  WCDMA Cell Info Ext */
  uint8_t wcdma_info_ext_valid;  /**< Must be set to true if wcdma_info_ext is being passed */
  nas_wcdma_cell_info_ext_type_v01 wcdma_info_ext;

  /* Optional */
  /*  WCDMA GSM Neighbor Cell Ext */
  uint8_t gncell_bcch_valid;  /**< Must be set to true if gncell_bcch is being passed */
  uint32_t gncell_bcch_len;  /**< Must be set to # of elements in gncell_bcch */
  uint16_t gncell_bcch[NAS_UMTS_GERAN_MAX_NBR_CELL_SET_NUM_V01];
  /**<   Channel number assigned to the frequency for the neighboring
       GSM cells.
  */

  /* Optional */
  /*  LTE Info - Timing Advance */
  uint8_t timing_advance_valid;  /**< Must be set to true if timing_advance is being passed */
  int32_t timing_advance;
  /**<   Timing advance of the LTE cell in micro seconds. (0xFFFFFFFF indicates 
       timing advance information is not present.) */

  /* Optional */
  /*  WCDMA Info - Active Set */
  uint8_t cells_valid;  /**< Must be set to true if cells is being passed */
  uint32_t cells_len;  /**< Must be set to # of elements in cells */
  nas_wcdma_active_set_info_type_v01 cells[NAS_UMTS_MAX_ACTIVE_CELL_SET_NUM_V01];

  /* Optional */
  /*  WCDMA Info - Active Set Reference Radio Link */
  uint8_t wcdma_active_set_reference_rl_valid;  /**< Must be set to true if wcdma_active_set_reference_rl is being passed */
  nas_wcdma_active_set_reference_rl_type_v01 wcdma_active_set_reference_rl;

  /* Optional */
  /*  Extended GERAN Info */
  uint8_t geran_info_ext_valid;  /**< Must be set to true if geran_info_ext is being passed */
  nas_geran_cell_info_ext_type_v01 geran_info_ext;

  /* Optional */
  /*  UMTS Extended Info */
  uint8_t umts_ext_info_valid;  /**< Must be set to true if umts_ext_info is being passed */
  nas_umts_cell_ext_info_type_v01 umts_ext_info;

  /* Optional */
  /*  Extended WCDMA Info - Active Set */
  uint8_t wcdma_active_set_cells_valid;  /**< Must be set to true if wcdma_active_set_cells is being passed */
  uint32_t wcdma_active_set_cells_len;  /**< Must be set to # of elements in wcdma_active_set_cells */
  nas_wcdma_active_set_ext_info_type_v01 wcdma_active_set_cells[NAS_UMTS_MAX_ACTIVE_CELL_SET_NUM_V01];

  /* Optional */
  /*  Scell GERAN Config */
  uint8_t scell_geran_config_valid;  /**< Must be set to true if scell_geran_config is being passed */
  nas_geran_eng_mode_scell_config_type_v01 scell_geran_config;

  /* Optional */
  /*  Current L1 Timeslot */
  uint8_t current_l1_ts_valid;  /**< Must be set to true if current_l1_ts is being passed */
  uint8_t current_l1_ts;
  /**<   timeslot number, Range : 0 to 7
  */
}nas_get_cell_location_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Queries the operator name for a specified network.
              \label{idl:getPlmnName} */
typedef struct {

  /* Mandatory */
  /*  PLMN  */
  nas_plmn_id_type_v01 plmn;

  /* Optional */
  /*  Suppress SIM Error */
  uint8_t suppress_sim_error_valid;  /**< Must be set to true if suppress_sim_error is being passed */
  uint8_t suppress_sim_error;
  /**<   Suppress the QMI_NAS_SIM_NOT_INITIALIZED error, so to allow network name 
       retrieval even when the SIM is not initialized. Values: \n
       - FALSE -- SIM initialization is checked; an error is returned if the SIM 
                  is not available (default value) \n
       - TRUE  -- SIM initialization is not checked; if the SIM is not available, 
                  retrieving the name from the SIM files is skipped
  */

  /* Optional */
  /*  MNC PCS Digit Include Status */
  uint8_t mnc_includes_pcs_digit_valid;  /**< Must be set to true if mnc_includes_pcs_digit is being passed */
  uint8_t mnc_includes_pcs_digit;
  /**<   This field is used to interpret the length of the corresponding 
       MNC reported in the PLMN TLV (0x01). Values: \n

       - TRUE  -- MNC is a three-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 090  \n
       - FALSE -- MNC is a two-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 90 \n \vspace{-.12in}

       If this TLV is not present, an MNC smaller than 100 is assumed to be 
       a two-digit value, and an MNC greater than or equal to 100 is 
       assumed to be a three-digit value.
  */

  /* Optional */
  /*  Always Send PLMN Name */
  uint8_t always_send_plmn_name_valid;  /**< Must be set to true if always_send_plmn_name is being passed */
  uint8_t always_send_plmn_name;
  /**<   Indicates that the client wants to receive the PLMN name regardless
       of the EF display condition. Values: \n

       - FALSE -- EF SPN PLMN display condition is looked at before attempting 
                  to retrieve the name \n
       - TRUE  -- PLMN name is returned regardless of the EF SPN PLMN display 
                  condition.
  */

  /* Optional */
  /*  Use Static Table Only */
  uint8_t use_static_table_only_valid;  /**< Must be set to true if use_static_table_only is being passed */
  uint8_t use_static_table_only;
  /**<   Indicates that the client wants to receive the network name only from 
       the SE.13 GSM Mobile Network Codes and Names Static Table. Values: \n

       - FALSE -- Normal procedure is followed when determining the network 
                  name (default value) \n
       - TRUE  -- SIM initialization state and the EF SPN PLMN display 
                  condition are ignored; the network name is read directly 
                  from the table
  */

  /* Optional */
  /*  CSG ID */
  uint8_t csg_id_valid;  /**< Must be set to true if csg_id is being passed */
  uint32_t csg_id;
  /**<   Closed subscriber group identifier.
  */

  /* Optional */
  /*  Radio Access Technology */
  uint8_t rat_valid;  /**< Must be set to true if rat is being passed */
  nas_radio_if_enum_v01 rat;
  /**<   Radio access technology. Values: \n
    - 0x04 -- NAS_RADIO_IF_GSM         -- GSM \n
    - 0x05 -- NAS_RADIO_IF_UMTS        -- UMTS \n
    - 0x08 -- NAS_RADIO_IF_LTE         -- LTE \n
    - 0x09 -- NAS_RADIO_IF_TDSCDMA     -- TD-SCDMA
  */

  /* Optional */
  /*  Send All Information */
  uint8_t send_all_information_valid;  /**< Must be set to true if send_all_information is being passed */
  uint8_t send_all_information;
  /**<   Indicates that the client wants to receive all available information, 
       including display byte information, without the modem influencing the 
       name sent. Values: \n       
       - FALSE -- Follow the normal procedure (default value) \n
       - TRUE  -- Send all available information 
  */
}nas_get_plmn_name_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  SPN coding scheme */
  nas_coding_scheme_enum_v01 spn_enc;
  /**<  
        Coding scheme for the service provider name. Values: \n
        -0x00 -- NAS_CODING_SCHEME_ CELL_BROADCAST_GSM -- SMS default 7-bit coded 
                 alphabet as defined in \hyperref[S8]{[S8]} with bit 8 set to 0 \n
        -0x01 -- NAS_CODING_SCHEME_ UCS2 -- UCS2 (16 bit, little-endian) 
                 \hyperref[S8]{[S8]} \n
        Note: This value is ignored if spn_len is zero.
  */

  /*  SPN */
  uint32_t spn_len;  /**< Must be set to # of elements in spn */
  char spn[NAS_SPN_LEN_MAX_V01];
  /**<  
     Service provider name string.
  */

  /*  PLMN short name encoding scheme */
  nas_coding_scheme_enum_v01 plmn_short_name_enc;
  /**<  
        Coding scheme for plmn_short_name. Values: \n
        -0x00 -- NAS_CODING_SCHEME_ CELL_BROADCAST_GSM -- SMS default 7-bit coded 
                 alphabet as defined in \hyperref[S8]{[S8]} with bit 8 set to 0 \n
        -0x01 -- NAS_CODING_SCHEME_ UCS2 -- UCS2 (16 bit, little-endian) 
                 \hyperref[S8]{[S8]} \n
        Note: This value is ignored if plmn_short_name_len is zero.
  */

  /*  PLMN short name country initial include status */
  nas_country_initials_add_enum_v01 plmn_short_name_ci;
  /**<  
        Indicates whether the country initials are to be added to the 
        plmn_short_name. Values: \n
        -0x00 -- Do not add the letters for the country's initials to the name \n
        -0x01 -- Add the country's initials and a text string to the name \n
        -0xFF -- Not specified \n
        Note: This value is ignored if plmn_short_name_len is zero.
  */

  /*  PLMN short spare bits */
  nas_spare_bits_enum_v01 plmn_short_spare_bits;
  /**<   Values: \n
       -0x01 -- Bit 8 is spare and set to 0 in octet n                       \n
       -0x02 -- Bits 7 and 8 are spare and set to 0 in octet n               \n
       -0x03 -- Bits 6 to 8 (inclusive) are spare and set to 0 in octet n    \n
       -0x04 -- Bits 5 to 8 (inclusive) are spare and set to 0 in octet n    \n
       -0x05 -- Bits 4 to 8 (inclusive) are spare and set to 0 in octet n    \n
       -0x06 -- Bits 3 to 8 (inclusive) are spare and set to 0 in octet n    \n
       -0x07 -- Bits 2 to 8 (inclusive) are spare and set to 0 in octet n    \n
       -0x00 -- Carries no information about the number of spare bits in octet n    \n
       Note: This value is ignored if plmn_short_name_len is zero.
  */

  /*  PLMN short name */
  uint32_t plmn_short_name_len;  /**< Must be set to # of elements in plmn_short_name */
  char plmn_short_name[NAS_PLMN_NAME_MAX_V01];
  /**<   PLMN short name. If no short name is available for the specified PLMN ID, 
       MCC and MNC values are included in ASCII format with the MCC followed
       by the MNC within double quotes. For example, for an MCC of 123 and an 
       MNC of 678, the ASCII string "123678" is returned when the short name 
       is not available.  
  */

  /*  PLMN long name encoding scheme */
  nas_coding_scheme_enum_v01 plmn_long_name_enc;
  /**<  
        Coding scheme for plmn_long_name. Values: \n
        -0x00 -- NAS_CODING_SCHEME_ CELL_BROADCAST_GSM -- SMS default 7-bit coded 
                 alphabet as defined in \hyperref[S8]{[S8]} with bit 8 set to 0 \n
        -0x01 -- NAS_CODING_SCHEME_ UCS2 -- UCS2 (16 bit, little-endian) 
                 \hyperref[S8]{[S8]} \n
        Note: This value is ignored if plmn_long_name_len is zero.
  */

  /*  PLMN long name country initial include status */
  nas_country_initials_add_enum_v01 plmn_long_name_ci;
  /**<  
        Indicates whether the country initials are to be added to the 
        plmn_long_name. Values: \n
        -0x00 -- Do not add the letters for the country's initials to the name \n
        -0x01 -- Add the country's initials and a text string to the name \n
        -0xFF -- Not specified \n
        Note: This value is ignored if plmn_long_name_len is zero.
  */

  /*  PLMN long spare bits  */
  nas_spare_bits_enum_v01 plmn_long_spare_bits;
  /**<   Values: \n
       -0x01 -- Bit 8 is spare and set to 0 in octet n                       \n
       -0x02 -- Bits 7 and 8 are spare and set to 0 in octet n               \n
       -0x03 -- Bits 6 to 8 (inclusive) are spare and set to 0 in octet n    \n
       -0x04 -- Bits 5 to 8 (inclusive) are spare and set to 0 in octet n    \n
       -0x05 -- Bits 4 to 8 (inclusive) are spare and set to 0 in octet n    \n
       -0x06 -- Bits 3 to 8 (inclusive) are spare and set to 0 in octet n    \n
       -0x07 -- Bits 2 to 8 (inclusive) are spare and set to 0 in octet n    \n
       -0x00 -- Carries no information about the number of spare bits in octet n    \n
       Note: This value is ignored if plmn_long_name_len is zero.
  */

  uint32_t plmn_long_name_len;  /**< Must be set to # of elements in plmn_long_name */
  char plmn_long_name[NAS_PLMN_NAME_MAX_V01];
  /**<   PLMN long name. If no long name is available for the specified PLMN ID, 
       MCC and MNC values are included in ASCII format with the MCC followed
       by the MNC within double quotes. For example, for an MCC of 123 and an 
       MNC of 678, the ASCII string "123678" is returned when the long name 
       is not available.  
  */
}nas_3gpp_eons_plmn_name_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_tri_state_boolean_type_v01 is_spn_set;
  /**<   Whether the SPN display bit is set. Values: \n
      - NAS_TRI_FALSE (0) --  Status: FALSE \n 
      - NAS_TRI_TRUE (1) --  Status: TRUE  \n 
      - NAS_TRI_UNKNOWN (2) --  Status: Unknown  
 */

  nas_tri_state_boolean_type_v01 is_plmn_set;
  /**<   Whether the PLMN display bit is set. Values: \n
      - NAS_TRI_FALSE (0) --  Status: FALSE \n 
      - NAS_TRI_TRUE (1) --  Status: TRUE  \n 
      - NAS_TRI_UNKNOWN (2) --  Status: Unknown  
 */
}nas_display_bit_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_LANG_ID_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_LANG_ID_UNKNOWN_V01 = 0x00, /**<  Unknown Language ID  */
  NAS_LANG_ID_ZH_TRAD_V01 = 0x01, /**<  Chinese Traditional  */
  NAS_LANG_ID_ZH_SIMP_V01 = 0x02, /**<  Chinese Simplified  */
  NAS_LANG_ID_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_lang_id_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t plmn_long_name_len;  /**< Must be set to # of elements in plmn_long_name */
  uint16_t plmn_long_name[NAS_ALT_LANG_NAME_LEN_MAX_V01];
  /**<   PLMN long name, in UCS2 (16 bit, little-endian) encoded format.
  */

  uint32_t plmn_short_name_len;  /**< Must be set to # of elements in plmn_short_name */
  uint16_t plmn_short_name[NAS_ALT_LANG_NAME_LEN_MAX_V01];
  /**<   PLMN short name, in UCS2 (16 bit, little-endian) encoded format.
  */

  nas_lang_id_enum_v01 lang_id;
  /**<   Language ID for the PLMN long and short names
      - NAS_LANG_ID_UNKNOWN (0x00) --  Unknown Language ID 
      - NAS_LANG_ID_ZH_TRAD (0x01) --  Chinese Traditional 
      - NAS_LANG_ID_ZH_SIMP (0x02) --  Chinese Simplified 
 */
}nas_lang_plmn_names_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the operator name for a specified network.
              \label{idl:getPlmnName} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  3GPP EONS PLMN Name  */
  uint8_t eons_plmn_name_3gpp_valid;  /**< Must be set to true if eons_plmn_name_3gpp is being passed */
  nas_3gpp_eons_plmn_name_type_v01 eons_plmn_name_3gpp;

  /* Optional */
  /*  Display Bit Information */
  uint8_t eons_display_bit_info_valid;  /**< Must be set to true if eons_display_bit_info is being passed */
  nas_display_bit_type_v01 eons_display_bit_info;

  /* Optional */
  /*  Network Information */
  uint8_t is_home_network_valid;  /**< Must be set to true if is_home_network is being passed */
  nas_tri_state_boolean_type_v01 is_home_network;
  /**<   Whether the network is the home network. Values: \n
      - NAS_TRI_FALSE (0) --  Status: FALSE \n 
      - NAS_TRI_TRUE (1) --  Status: TRUE  \n 
      - NAS_TRI_UNKNOWN (2) --  Status: Unknown  
 */

  /* Optional */
  /*  3GPP EONS PLMN Name with Language ID */
  uint8_t lang_plmn_names_valid;  /**< Must be set to true if lang_plmn_names is being passed */
  uint32_t lang_plmn_names_len;  /**< Must be set to # of elements in lang_plmn_names */
  nas_lang_plmn_names_type_v01 lang_plmn_names[NAS_ALT_LANG_MAX_V01];
  /**<   List of additional PLMN names, with their language identifier.
  */
}nas_get_plmn_name_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SUBS_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_PRIMARY_SUBSCRIPTION_V01 = 0x00, /**<  Primary subscription \n  */
  NAS_SECONDARY_SUBSCRIPTION_V01 = 0x01, /**<  Secondary subscription \n  */
  NAS_TERTIARY_SUBSCRIPTION_V01 = 0x02, /**<  Tertiary subscription  */
  NAS_SUBS_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_subs_type_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Binds the current control point to a specific subscription.  */
typedef struct {

  /* Mandatory */
  /*  Subscription Type */
  nas_subs_type_enum_v01 subs_type;
  /**<   Values: \n
      - NAS_PRIMARY_SUBSCRIPTION (0x00) --  Primary subscription \n 
      - NAS_SECONDARY_SUBSCRIPTION (0x01) --  Secondary subscription \n 
      - NAS_TERTIARY_SUBSCRIPTION (0x02) --  Tertiary subscription 
 */
}nas_bind_subscription_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Binds the current control point to a specific subscription.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_bind_subscription_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_STANDBY_PREF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SINGLE_STANDBY_V01 = 0x01, 
  NAS_DUAL_STANDBY_WITH_TUNE_AWAY_V01 = 0x02, 
  NAS_DUAL_STANDBY_WITHOUT_TUNE_AWAY_V01 = 0x04, 
  NAS_AUTOMATIC_WITH_TUNE_AWAY_V01 = 0x05, 
  NAS_AUTOMATIC_WITHOUT_TUNE_AWAY_V01 = 0x06, 
  NAS_TRIPLE_STANDBY_V01 = 0x07, 
  NAS_STANDBY_PREF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_standby_pref_enum_v01;
/**
    @}
  */

typedef uint64_t nas_active_subs_mask_type_v01;
#define QMI_NAS_ACTIVE_SUB_PRIMARY_V01 ((nas_active_subs_mask_type_v01)0x01ull) 
#define QMI_NAS_ACTIVE_SUB_SECONDARY_V01 ((nas_active_subs_mask_type_v01)0x02ull) 
#define QMI_NAS_ACTIVE_SUB_TERTIARY_V01 ((nas_active_subs_mask_type_v01)0x04ull) 
/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Configures dual standby preference. */
typedef struct {

  /* Optional */
  /*  Standby Preference */
  uint8_t standby_pref_valid;  /**< Must be set to true if standby_pref is being passed */
  nas_standby_pref_enum_v01 standby_pref;
  /**<  
        Values: \n
        -0x05 -- Automatic mode with tune away where applicable \n
        -0x06 -- Automatic mode without tune away

        \vspace{3pt}
        All other values are reserved.
  */

  /* Optional */
  /*  Priority Subs */
  uint8_t priority_subs_valid;  /**< Must be set to true if priority_subs is being passed */
  nas_subs_type_enum_v01 priority_subs;
  /**<   Subscription to give priority when listening to the paging channel during
 standby. Values: \n
      - NAS_PRIMARY_SUBSCRIPTION (0x00) --  Primary subscription \n 
      - NAS_SECONDARY_SUBSCRIPTION (0x01) --  Secondary subscription \n 
      - NAS_TERTIARY_SUBSCRIPTION (0x02) --  Tertiary subscription 

 \vspace{3pt}
 All other values are reserved.
 */

  /* Optional */
  /*  Default Data Subs */
  uint8_t default_data_subs_valid;  /**< Must be set to true if default_data_subs is being passed */
  nas_subs_type_enum_v01 default_data_subs;
  /**<   Default data subscription. Values: \n
      - NAS_PRIMARY_SUBSCRIPTION (0x00) --  Primary subscription \n 
      - NAS_SECONDARY_SUBSCRIPTION (0x01) --  Secondary subscription \n 
      - NAS_TERTIARY_SUBSCRIPTION (0x02) --  Tertiary subscription 

 \vspace{3pt}
 All other values are reserved.
 */

  /* Optional */
  /*  Default Voice Subs */
  uint8_t default_voice_subs_valid;  /**< Must be set to true if default_voice_subs is being passed */
  nas_subs_type_enum_v01 default_voice_subs;
  /**<   Default voice subscription. Values: \n
      - NAS_PRIMARY_SUBSCRIPTION (0x00) --  Primary subscription \n 
      - NAS_SECONDARY_SUBSCRIPTION (0x01) --  Secondary subscription \n 
      - NAS_TERTIARY_SUBSCRIPTION (0x02) --  Tertiary subscription 

 \vspace{3pt}
 All other values are reserved.
 */

  /* Optional */
  /*  Active Subs Mask */
  uint8_t active_subs_mask_valid;  /**< Must be set to true if active_subs_mask is being passed */
  nas_active_subs_mask_type_v01 active_subs_mask;
  /**<   Bitmask representing the active subscriptions to be set. If a value 
       of 0 is sent, there are no active subscriptions.
       Values: \n
       - Bit 0 (0x01) -- QMI_NAS_ACTIVE_SUB_ PRIMARY   -- Primary subscription \n
       - Bit 1 (0x02) -- QMI_NAS_ACTIVE_SUB_ SECONDARY -- Secondary subscription \n
       - Bit 2 (0x04) -- QMI_NAS_ACTIVE_SUB_ TERTIARY  -- Tertiary subscription

       \vspace{3pt}
       All unlisted bits are reserved for future use and the service point
       ignores them if used.
  */
}nas_set_dual_standby_pref_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Configures dual standby preference. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_set_dual_standby_pref_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Standby preference */
  nas_standby_pref_enum_v01 standby_pref;
  /**<   Values: \n
       -0x01 -- Single standby \n
       -0x02 -- Dual standby with tune away \n
       -0x04 -- Dual standby without tune away \n
       -0x05 -- Automatic mode with tune away where applicable \n
       -0x06 -- Automatic mode without tune away \n
       -0x07 -- Triple standby

       \vspace{3pt}
       All other values are reserved.
  */

  /*  Priority subs */
  nas_subs_type_enum_v01 priority_subs;
  /**<   Subscription to give priority when listening to the paging channel during
 dual standby. Values: \n
      - NAS_PRIMARY_SUBSCRIPTION (0x00) --  Primary subscription \n 
      - NAS_SECONDARY_SUBSCRIPTION (0x01) --  Secondary subscription \n 
      - NAS_TERTIARY_SUBSCRIPTION (0x02) --  Tertiary subscription 

 \vspace{3pt}
 All other values are reserved.
 */

  /*  Active subs */
  nas_subs_type_enum_v01 active_subs;
  /**<   Subscription to enable when "standby_pref is 0x01 -- Single standby". 
 Values: \n
      - NAS_PRIMARY_SUBSCRIPTION (0x00) --  Primary subscription \n 
      - NAS_SECONDARY_SUBSCRIPTION (0x01) --  Secondary subscription \n 
      - NAS_TERTIARY_SUBSCRIPTION (0x02) --  Tertiary subscription 

 \vspace{3pt}
 All other values are reserved.
 */

  /*  Default data subs */
  nas_subs_type_enum_v01 default_data_subs;
  /**<   Default data subscription. Values: \n
      - NAS_PRIMARY_SUBSCRIPTION (0x00) --  Primary subscription \n 
      - NAS_SECONDARY_SUBSCRIPTION (0x01) --  Secondary subscription \n 
      - NAS_TERTIARY_SUBSCRIPTION (0x02) --  Tertiary subscription 

 \vspace{3pt}
 All other values are reserved.
 */
}nas_standby_pref_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Informs the control point of any changes in dual standby
             subscription. */
typedef struct {

  /* Optional */
  /*  Standby Preference */
  uint8_t standby_pref_valid;  /**< Must be set to true if standby_pref is being passed */
  nas_standby_pref_type_v01 standby_pref;

  /* Optional */
  /*  Default Voice Subs */
  uint8_t default_voice_subs_valid;  /**< Must be set to true if default_voice_subs is being passed */
  nas_subs_type_enum_v01 default_voice_subs;
  /**<   Default voice subscription. Values: \n
      - NAS_PRIMARY_SUBSCRIPTION (0x00) --  Primary subscription \n 
      - NAS_SECONDARY_SUBSCRIPTION (0x01) --  Secondary subscription \n 
      - NAS_TERTIARY_SUBSCRIPTION (0x02) --  Tertiary subscription 

 \vspace{3pt}
 All other values are reserved.
 */

  /* Optional */
  /*  Active Subs Mask */
  uint8_t active_subs_mask_valid;  /**< Must be set to true if active_subs_mask is being passed */
  nas_active_subs_mask_type_v01 active_subs_mask;
  /**<   Bitmask representing the active subscriptions in the device. If a value 
       of 0 is sent, there are no active subscriptions.
       Values: \n
       - Bit 0 (0x01) -- QMI_NAS_ACTIVE_SUB_ PRIMARY   -- Primary subscription \n
       - Bit 1 (0x02) -- QMI_NAS_ACTIVE_SUB_ SECONDARY -- Secondary subscription \n
       - Bit 2 (0x04) -- QMI_NAS_ACTIVE_SUB_ TERTIARY  -- Tertiary subscription

       \vspace{3pt}
       All unlisted bits are reserved for future use and the service point
       ignores them if used.
  */
}nas_dual_standby_pref_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_IS_PRIORITY_SUBS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_PRIORITY_SUBSCRIPTION_FALSE_V01 = 0x00, 
  NAS_PRIORITY_SUBSCRIPTION_TRUE_V01 = 0x01, 
  NAS_IS_PRIORITY_SUBS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_is_priority_subs_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_ACTIVE_SUBS_INFO_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SUBSCRIPTION_NOT_ACTIVE_V01 = 0x00, 
  NAS_SUBSCRIPTION_ACTIVE_V01 = 0x01, 
  NAS_ACTIVE_SUBS_INFO_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_active_subs_info_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Indicates any change in the subscription information. */
typedef struct {

  /* Optional */
  /*  Priority Subscription Info */
  uint8_t is_priority_subs_valid;  /**< Must be set to true if is_priority_subs is being passed */
  nas_is_priority_subs_enum_v01 is_priority_subs;
  /**<  
      Information on whether the subscription is a priority subscription 
      in cases of dual standby. Values: \n
      -0x00 -- Not a priority subscription \n
      -0x01 -- Priority subscription
 */

  /* Optional */
  /*  Active Subscription Info */
  uint8_t is_active_valid;  /**< Must be set to true if is_active is being passed */
  nas_active_subs_info_enum_v01 is_active;
  /**<  
      Information on whether the subscription is active. Values: \n
      -0x00 -- Not active \n
      -0x01 -- Active
 */

  /* Optional */
  /*  Default Data Subscription Info */
  uint8_t is_default_data_subs_valid;  /**< Must be set to true if is_default_data_subs is being passed */
  uint8_t is_default_data_subs;
  /**<  
      Information on whether the subscription is the default data
      subscription in cases of dual standby. Values: \n
      -0x00 -- Not a default data subscription \n
      -0x01 -- Default data subscription
 */

  /* Optional */
  /*  Voice System ID */
  uint8_t voice_system_id_valid;  /**< Must be set to true if voice_system_id is being passed */
  uint32_t voice_system_id;
  /**<   Voice system ID.
 */
}nas_subscription_info_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_mode_pref_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the mode preference. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Mode Preference for idx0 */
  uint8_t idx0_mode_pref_valid;  /**< Must be set to true if idx0_mode_pref is being passed */
  mode_pref_mask_type_v01 idx0_mode_pref;
  /**<   Bitmask representing the radio technology mode preference set in
      NV (idx0). Values: \n
       - Bit 0 (0x01) -- QMI_NAS_RAT_MODE_PREF_ CDMA2000_1X    -- 
         cdma2000\textsuperscript{\textregistered} 1X             \n
       - Bit 1 (0x02) -- QMI_NAS_RAT_MODE_PREF_ CDMA2000_HRPD  -- 
         cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - Bit 2 (0x04) -- QMI_NAS_RAT_MODE_PREF_ GSM            -- GSM \n
       - Bit 3 (0x08) -- QMI_NAS_RAT_MODE_PREF_ UMTS           -- UMTS \n
       - Bit 4 (0x10) -- QMI_NAS_RAT_MODE_PREF_ LTE            -- LTE \n
       - Bit 5 (0x20) -- QMI_NAS_RAT_MODE_PREF_ TDSCDMA        -- TD-SCDMA
  */

  /* Optional */
  /*  Mode Preference for idx1 */
  uint8_t idx1_mode_pref_valid;  /**< Must be set to true if idx1_mode_pref is being passed */
  mode_pref_mask_type_v01 idx1_mode_pref;
  /**<   Bitmask representing the radio technology mode preference set in 
      NV (idx1). Values: \n
       - Bit 0 (0x01) -- QMI_NAS_RAT_MODE_PREF_ CDMA2000_1X    -- 
         cdma2000\textsuperscript{\textregistered} 1X             \n
       - Bit 1 (0x02) -- QMI_NAS_RAT_MODE_PREF_ CDMA2000_HRPD  -- 
         cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - Bit 2 (0x04) -- QMI_NAS_RAT_MODE_PREF_ GSM            -- GSM \n
       - Bit 3 (0x08) -- QMI_NAS_RAT_MODE_PREF_ UMTS           -- UMTS \n
       - Bit 4 (0x10) -- QMI_NAS_RAT_MODE_PREF_ LTE            -- LTE \n
       - Bit 5 (0x20) -- QMI_NAS_RAT_MODE_PREF_ TDSCDMA        -- TD-SCDMA
 */

  /* Optional */
  /*  Mode Preference for idx2 */
  uint8_t idx2_mode_pref_valid;  /**< Must be set to true if idx2_mode_pref is being passed */
  mode_pref_mask_type_v01 idx2_mode_pref;
  /**<   Bitmask representing the radio technology mode preference set in 
      NV (idx2). Values: \n
       - Bit 0 (0x01) -- QMI_NAS_RAT_MODE_PREF_ CDMA2000_1X    -- 
         cdma2000\textsuperscript{\textregistered} 1X             \n
       - Bit 1 (0x02) -- QMI_NAS_RAT_MODE_PREF_ CDMA2000_HRPD  -- 
         cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       - Bit 2 (0x04) -- QMI_NAS_RAT_MODE_PREF_ GSM            -- GSM \n
       - Bit 3 (0x08) -- QMI_NAS_RAT_MODE_PREF_ UMTS           -- UMTS \n
       - Bit 4 (0x10) -- QMI_NAS_RAT_MODE_PREF_ LTE            -- LTE \n
       - Bit 5 (0x20) -- QMI_NAS_RAT_MODE_PREF_ TDSCDMA        -- TD-SCDMA
 */
}nas_get_mode_pref_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_ACTIVE_TECHNOLOGY_DURATION_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_ACTIVE_TECHNOLOGY_DURATION_PERMANENT_V01 = 0x00, 
  NAS_ACTIVE_TECHNOLOGY_DURATION_PWR_CYCLE_V01 = 0x01, 
  NAS_ACTIVE_TECHNOLOGY_DURATION_1C_ENC_PC_V01 = 0x02, 
  NAS_ACTIVE_TECHNOLOGY_DURATION_1C_TUENC_ST_PC_V01 = 0x03, 
  NAS_ACTIVE_TECHNOLOGY_DURATION_1C_TUENC_INTERNAL1_V01 = 0x04, 
  NAS_ACTIVE_TECHNOLOGY_DURATION_1C_TUENC_INTERNAL2_V01 = 0x05, 
  NAS_ACTIVE_TECHNOLOGY_DURATION_1C_TUENC_INTERNAL3_V01 = 0x06, 
  NAS_ACTIVE_TECHNOLOGY_DURATION_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_active_technology_duration_enum_type_v01;
/**
    @}
  */

typedef uint16_t nas_persistent_technology_pref_mask_type_v01;
#define NAS_PERSISTENT_TECH_PREF_3GPP2_V01 ((nas_persistent_technology_pref_mask_type_v01)0x01) 
#define NAS_PERSISTENT_TECH_PREF_3GPP_V01 ((nas_persistent_technology_pref_mask_type_v01)0x02) 
#define NAS_PERSISTENT_TECH_PREF_ANALOG_V01 ((nas_persistent_technology_pref_mask_type_v01)0x04) 
#define NAS_PERSISTENT_TECH_PREF_DIGITAL_V01 ((nas_persistent_technology_pref_mask_type_v01)0x08) 
#define NAS_PERSISTENT_TECH_PREF_HDR_V01 ((nas_persistent_technology_pref_mask_type_v01)0x10) 
#define NAS_PERSISTENT_TECH_PREF_LTE_V01 ((nas_persistent_technology_pref_mask_type_v01)0x20) 
/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Technology preference */
  nas_persistent_technology_pref_mask_type_v01 technology_pref;
  /**<  
        Bitmask representing the radio technology preference set. 
        No bits set indicates to the device to automatically
        determine the technology to use. Values: \n
        - Bit 0 -- Technology is 3GPP2 \n
        - Bit 1 -- Technology is 3GPP

        Any combination of the following may be returned: \n
        - Bit 2 -- Analog -- AMPS if 3GPP2, GSM if 3GPP \n
        - Bit 3 -- Digital -- CDMA if 3GPP2, WCDMA if 3GPP \n
        - Bit 4 -- HDR \n
        - Bit 5 -- LTE \n
        - Bits 6 to 15 -- Reserved

        Note: Bits 0 and 1 are exclusive; only one may be set at a time.
              All unlisted bits are reserved for future use and are ignored.
  */

  /*  Duration */
  nas_active_technology_duration_enum_type_v01 duration;
  /**<  
      Preference duration. Values: \n
      -0x00 -- Permanent   -- Preference is used permanently \n
      -0x01 -- Power cycle -- Preference is used until the next device power cycle
  */
}nas_technology_pref_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets the technology preference. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Technology Preference */
  nas_technology_pref_type_v01 technology_pref;
}nas_set_technology_preference_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets the technology preference. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_set_technology_preference_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_technology_preference_req_type_v01;

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Technology preference */
  nas_persistent_technology_pref_mask_type_v01 technology_pref;
  /**<  
        Bitmask representing the radio technology preference set. 
        No bits set indicates to the device to automatically
        determine the technology to use. Values: \n
        - Bit 0 -- Technology is 3GPP2 \n
        - Bit 1 -- Technology is 3GPP

        Any combination of the following may be returned: \n
        - Bit 2 -- Analog -- AMPS if 3GPP2, GSM if 3GPP \n
        - Bit 3 -- Digital -- CDMA if 3GPP2, WCDMA if 3GPP \n
        - Bit 4 -- HDR \n
        - Bit 5 -- LTE \n
        - Bits 6 to 15 -- Reserved

        Note: Bits 0 and 1 are exclusive; only one may be set at a time.
              All unlisted bits are reserved for future use and are ignored.
  */

  /*  Duration */
  nas_active_technology_duration_enum_type_v01 duration;
  /**<  
      Duration of the active preference. Values: \n
      -0x00 -- Permanent -- Preference is used permanently \n
      -0x01 -- Power cycle -- Preference is used until the next device power cycle \n
      -0x02 -- 1 call -- Until the end of the next call or a power cycle \n
      -0x03 -- 1 call or time -- Until the end of the next call, a specified time, 
                                 or a power cycle \n
      -0x04-0x06 -- Internal 1 call -- Until the end of the next call 

  */
}nas_active_technology_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the technology preference. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Active Technology Preference */
  nas_active_technology_type_v01 active_technology_pref;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Persistent Technology Preference */
  uint8_t persistent_technology_pref_valid;  /**< Must be set to true if persistent_technology_pref is being passed */
  nas_persistent_technology_pref_mask_type_v01 persistent_technology_pref;
  /**<    Bitmask representing the radio technology preference set. 
        No bits set indicates to the device to automatically
        determine the technology to use. Values: \n
        - Bit 0 -- Technology is 3GPP2 \n
        - Bit 1 -- Technology is 3GPP

        Any combination of the following may be returned: \n
        - Bit 2 -- Analog -- AMPS if 3GPP2, GSM if 3GPP \n
        - Bit 3 -- Digital -- CDMA if 3GPP2, WCDMA if 3GPP \n
        - Bit 4 -- HDR \n
        - Bit 5 -- LTE \n
        - Bits 6 to 15 -- Reserved

        Note: Bits 0 and 1 are exclusive; only one may be set at a time.
              All unlisted bits are reserved for future use and are ignored.       
    */
}nas_get_technology_preference_resp_type_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_network_system_preference_req_v01;

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NETWORK_SYS_PREF_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_NETWORK_SYSTEM_PREFERENCE_AUTOMATIC_V01 = 0x00, 
  NAS_NETWORK_SYSTEM_PREFERENCE_AUTO_A_V01 = 0x01, 
  NAS_NETWORK_SYSTEM_PREFERENCE_AUTO_B_V01 = 0x02, 
  NETWORK_SYS_PREF_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}network_sys_pref_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the network system preference. */
typedef struct {

  /* Mandatory */
  /*  System Preference */
  network_sys_pref_enum_type_v01 system_pref;
  /**<  
      Duration of the active preference. Values: \n
      -0x00 -- Automatic \n
      -0x01 -- Auto A \n
      -0x02 -- Auto B
  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_get_network_system_preference_resp_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Year */
  uint16_t year;
  /**<   Year.
  */

  /*  Month */
  uint8_t month;
  /**<   Month. 1 is January and 12 is December.
  */

  /*  Day */
  uint8_t day;
  /**<   Day. Range: 1 to 31.
  */

  /*  Hour */
  uint8_t hour;
  /**<   Hour. Range: 0 to 59.
  */

  /*  Minute */
  uint8_t minute;
  /**<   Minute. Range: 0 to 59.
  */

  /*  Second */
  uint8_t second;
  /**<   Second. Range: 0 to 59.
  */

  /*  Day of the week */
  uint8_t day_of_week;
  /**<   Day of the week. 0 is Monday and 6 is Sunday.
  */
}nas_julian_time_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Indicates a time change reported by the network. */
typedef struct {

  /* Mandatory */
  /*  Universal Time */
  nas_julian_time_type_v01 universal_time;

  /* Optional */
  /*  Time Zone */
  uint8_t time_zone_valid;  /**< Must be set to true if time_zone is being passed */
  int8_t time_zone;
  /**<   Offset from Universal time, i.e., the difference between local time
       and Universal time, in increments of 15 min (signed value).
  */

  /* Optional */
  /*  Daylight Saving Adjustment */
  uint8_t daylt_sav_adj_valid;  /**< Must be set to true if daylt_sav_adj is being passed */
  uint8_t daylt_sav_adj;
  /**<   Daylight saving adjustment in hours. Possible values: 0, 1, and 2. This 
       TLV is ignored if radio_if is NAS_RADIO_IF_CDMA_1XEVDO.
  */

  /* Optional */
  /*  Radio Interface */
  uint8_t radio_if_valid;  /**< Must be set to true if radio_if is being passed */
  nas_radio_if_enum_v01 radio_if;
  /**<  
    Radio interface from which to get the information. Values: \n
    - 0x01 -- NAS_RADIO_IF_CDMA_1X     -- 
      cdma2000\textsuperscript{\textregistered} 1X             \n
    - 0x02 -- NAS_RADIO_IF_CDMA_1XEVDO -- 
      cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
    - 0x04 -- NAS_RADIO_IF_GSM         -- GSM \n
    - 0x05 -- NAS_RADIO_IF_UMTS        -- UMTS \n
    - 0x08 -- NAS_RADIO_IF_LTE         -- LTE \n
    - 0x09 -- NAS_RADIO_IF_TDSCDMA     -- TD-SCDMA
  */
}nas_network_time_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_sys_info_req_msg_v01;

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_CUR_IDLE_DIGITAL_MODE_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SYS_MODE_NO_SRV_V01 = 0x00, 
  NAS_SYS_MODE_AMPS_V01 = 0x01, 
  NAS_SYS_MODE_CDMA_V01 = 0x02, 
  NAS_SYS_MODE_GSM_V01 = 0x03, 
  NAS_SYS_MODE_HDR_V01 = 0x04, 
  NAS_SYS_MODE_WCDMA_V01 = 0x05, 
  NAS_SYS_MODE_GPS_V01 = 0x06, 
  NAS_SYS_MODE_GW_V01 = 0x07, 
  NAS_SYS_MODE_WLAN_V01 = 0x08, 
  NAS_SYS_MODE_LTE_V01 = 0x09, 
  NAS_SYS_MODE_GWL_V01 = 0x0A, 
  NAS_CUR_IDLE_DIGITAL_MODE_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_cur_idle_digital_mode_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SYS_SRV_STATUS_NO_SRV_V01 = 0, 
  NAS_SYS_SRV_STATUS_LIMITED_V01 = 1, 
  NAS_SYS_SRV_STATUS_SRV_V01 = 2, 
  NAS_SYS_SRV_STATUS_LIMITED_REGIONAL_V01 = 3, 
  NAS_SYS_SRV_STATUS_PWR_SAVE_V01 = 4, 
  NAS_SERVICE_STATUS_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_service_status_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_TRUE_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SYS_SRV_STATUS_NO_SRV_V01 = 0, 
  SYS_SRV_STATUS_LIMITED_V01 = 1, 
  SYS_SRV_STATUS_SRV_V01 = 2, 
  SYS_SRV_STATUS_LIMITED_REGIONAL_V01 = 3, 
  SYS_SRV_STATUS_PWR_SAVE_V01 = 4, 
  NAS_TRUE_SERVICE_STATUS_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_true_service_status_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SYS_SRV_DOMAIN_NO_SRV_V01 = 0, 
  SYS_SRV_DOMAIN_CS_ONLY_V01 = 1, 
  SYS_SRV_DOMAIN_PS_ONLY_V01 = 2, 
  SYS_SRV_DOMAIN_CS_PS_V01 = 3, 
  SYS_SRV_DOMAIN_CAMPED_V01 = 4, 
  NAS_SERVICE_DOMAIN_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_service_domain_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_HDR_PERSONALITY_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SYS_PERSONALITY_NONE_V01 = 0x00, 
  NAS_SYS_PERSONALITY_HRPD_V01 = 0x02, 
  NAS_SYS_PERSONALITY_EHRPD_V01 = 0x03, 
  NAS_HDR_PERSONALITY_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_hdr_personality_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_HDR_ACTIVE_PROT_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SYS_ACTIVE_PROT_NONE_V01 = 0x00, 
  NAS_SYS_ACTIVE_PROT_HDR_REL0_V01 = 0x02, 
  NAS_SYS_ACTIVE_PROT_HDR_RELA_V01 = 0x03, 
  NAS_SYS_ACTIVE_PROT_HDR_RELB_V01 = 0x04, 
  NAS_HDR_ACTIVE_PROT_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_hdr_active_prot_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_ROAM_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SYS_ROAM_STATUS_OFF_V01 = 0x00, 
  NAS_SYS_ROAM_STATUS_ON_V01 = 0x01, 
  NAS_SYS_ROAM_STATUS_BLINK_V01 = 0x02, 
  NAS_SYS_ROAM_STATUS_OUT_OF_NEIGHBORHOOD_V01 = 0x03, 
  NAS_SYS_ROAM_STATUS_OUT_OF_BLDG_V01 = 0x04, 
  NAS_SYS_ROAM_STATUS_PREF_SYS_V01 = 0x05, 
  NAS_SYS_ROAM_STATUS_AVAIL_SYS_V01 = 0x06, 
  NAS_SYS_ROAM_STATUS_ALLIANCE_PARTNER_V01 = 0x07, 
  NAS_SYS_ROAM_STATUS_PREMIUM_PARTNER_V01 = 0x08, 
  NAS_SYS_ROAM_STATUS_FULL_SVC_V01 = 0x09, 
  NAS_SYS_ROAM_STATUS_PARTIAL_SVC_V01 = 0x0A, 
  NAS_SYS_ROAM_STATUS_BANNER_ON_V01 = 0x0B, 
  NAS_SYS_ROAM_STATUS_BANNER_OFF_V01 = 0x0C, 
  NAS_ROAM_STATUS_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_roam_status_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_EUTRA_CELL_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_EUTRA_CELL_PRESENT_V01 = 0x00, 
  NAS_EUTRA_CELL_NOT_PRESENT_V01 = 0x01, 
  NAS_EUTRA_CELL_PRESENCE_UNKNOWN_V01 = 0x02, 
  NAS_EUTRA_CELL_DETECTION_UNSUPPORTED_V01 = 0x03, 
  NAS_EUTRA_CELL_STATUS_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_eutra_cell_status_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Service Domain */
  nas_service_domain_enum_type_v01 reject_srv_domain;
  /**<  
      Type of service domain in which the registration is rejected. Values: \n
      - 0x00 -- SYS_SRV_DOMAIN_NO_SRV  -- No service \n
      - 0x01 -- SYS_SRV_DOMAIN_CS_ONLY -- Circuit-switched only \n
      - 0x02 -- SYS_SRV_DOMAIN_PS_ONLY -- Packet-switched only \n
      - 0x03 -- SYS_SRV_DOMAIN_CS_PS   -- Circuit-switched and packet-switched \n
      - 0x04 -- SYS_SRV_DOMAIN_CAMPED  -- Camped
      */

  /*  Registration Rejection Cause */
  uint8_t rej_cause;
  /**<  
      Reject cause values sent are specified in 
      \hyperref[S5]{[S5]} Sections 10.5.3.6 and 10.5.5.14, and 
      \hyperref[S16]{[S16]} Section 9.9.3.9.
  */
}nas_reg_reject_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Service Status of the System */
  nas_service_status_enum_type_v01 srv_status;
  /**<   
       Service status of the system. Values: \n
       - 0x00 -- SYS_SRV_STATUS_NO_SRV  -- No service \n
       - 0x01 -- SYS_SRV_STATUS_LIMITED -- Limited service \n
       - 0x02 -- SYS_SRV_STATUS_SRV     -- Service \n
       - 0x03 -- SYS_SRV_STATUS_LIMITED_ REGIONAL -- Limited regional service \n
       - 0x04 -- SYS_SRV_STATUS_PWR_SAVE          -- Power save
  */

  /*  Is this RAT the preferred data path */
  uint8_t is_pref_data_path;
  /**<  
       Whether the RAT is the preferred data path: \n
       - 0x00 -- Not preferred \n
       - 0x01 -- Preferred
  */
}nas_3gpp2_srv_status_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Service Status of the System */
  nas_service_status_enum_type_v01 srv_status;
  /**<   
       Service status of the system. Values: \n
       - 0x00 -- SYS_SRV_STATUS_NO_SRV  -- No service \n
       - 0x01 -- SYS_SRV_STATUS_LIMITED -- Limited service \n
       - 0x02 -- SYS_SRV_STATUS_SRV     -- Service \n
       - 0x03 -- SYS_SRV_STATUS_LIMITED_ REGIONAL -- Limited regional service \n
       - 0x04 -- SYS_SRV_STATUS_PWR_SAVE          -- Power save
  */

  /*  True Service Status of the System (not applicable to CDMA/HDR) */
  nas_true_service_status_enum_type_v01 true_srv_status;
  /**<  
      True service status of the system (not applicable to CDMA/HDR). Values: \n
      - 0x00 -- SYS_SRV_STATUS_NO_SRV  -- No service \n
      - 0x01 -- SYS_SRV_STATUS_LIMITED -- Limited service \n
      - 0x02 -- SYS_SRV_STATUS_SRV     -- Service \n
      - 0x03 -- SYS_SRV_STATUS_LIMITED_ REGIONAL -- Limited regional service \n
      - 0x04 -- SYS_SRV_STATUS_PWR_SAVE          -- Power save
  */

  /*  Is this RAT the Preferred Data Path */
  uint8_t is_pref_data_path;
  /**<  
       Whether the RAT is the preferred data path: \n
       - 0x00 -- Not preferred \n
       - 0x01 -- Preferred
    */
}nas_3gpp_srv_status_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_CELL_BROADCAST_CAP_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_CELL_BROADCAST_CAP_UNKNOWN_V01 = 0x00, /**<  CB capability information not known.  */
  NAS_CELL_BROADCAST_CAP_OFF_V01 = 0x01, /**<  CB capability OFF     */
  NAS_CELL_BROADCAST_CAP_ON_V01 = 0x02, /**<  CB capability ON     */
  NAS_CELL_BROADCAST_CAP_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_cell_broadcast_cap_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  MCC */
  char mcc[NAS_MCC_MNC_MAX_V01];
  /**<  
      MCC digits in ASCII characters. 

      For CDMA, the MCC wildcard value is returned as \{`3', 0xFF, 0xFF\}.
  */

  /*  MNC */
  char mnc[NAS_MCC_MNC_MAX_V01];
  /**<  
      MNC digits in ASCII characters. For this field: \n
      - Unused byte is set to 0xFF   \n
      - In the case of two-digit MNC values, the third (unused) digit 
        is set to 0xFF. For example, 15 (a two-digit MNC) is reported 
        using the byte stream 0x31 0x35 0xFF.

      For CDMA, the MNC wildcard value is returned as \{`7', 0xFF, 0xFF\}. 
  */
}nas_common_network_id_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Is the Service Domain Valid */
  uint8_t srv_domain_valid;
  /**<   
      Indicates whether the service domain is valid.
  */

  /*  Service Domain */
  nas_service_domain_enum_type_v01 srv_domain;
  /**<  
      Service domain registered on the system. Values:  \n
      - 0x00 -- SYS_SRV_DOMAIN_NO_SRV  -- No service \n
      - 0x01 -- SYS_SRV_DOMAIN_CS_ONLY -- Circuit-switched only \n
      - 0x02 -- SYS_SRV_DOMAIN_PS_ONLY -- Packet-switched only \n
      - 0x03 -- SYS_SRV_DOMAIN_CS_PS   -- Circuit-switched and packet-switched \n
      - 0x04 -- SYS_SRV_DOMAIN_CAMPED  -- Camped
      */

  /*  Is the Service Capability Valid */
  uint8_t srv_capability_valid;
  /**<   
      Indicates whether the service capability is valid. 
  */

  /*  Service Capability */
  nas_service_domain_enum_type_v01 srv_capability;
  /**<  
      Current system's service capability. Values: \n
      - 0x00 -- SYS_SRV_DOMAIN_NO_SRV  -- No service \n
      - 0x01 -- SYS_SRV_DOMAIN_CS_ONLY -- Circuit-switched only \n
      - 0x02 -- SYS_SRV_DOMAIN_PS_ONLY -- Packet-switched only \n
      - 0x03 -- SYS_SRV_DOMAIN_CS_PS   -- Circuit-switched and packet-switched \n
      - 0x04 -- SYS_SRV_DOMAIN_CAMPED  -- Camped
        */

  /*  Is the Roaming Status Valid */
  uint8_t roam_status_valid;
  /**<   
      Indicates whether the roaming status is valid. 
  */

  /*  Current Roaming Status */
  nas_roam_status_enum_type_v01 roam_status;
  /**<  
      Current roaming status. Values: \n
      - 0x00 -- SYS_ROAM_STATUS_OFF   -- Off \n
      - 0x01 -- SYS_ROAM_STATUS_ON    -- On  \n
      - 0x02 -- SYS_ROAM_STATUS_BLINK -- Blinking \n
      - 0x03 -- SYS_ROAM_STATUS_OUT_OF_ NEIGHBORHOOD -- Out of the neighborhood \n
      - 0x04 -- SYS_ROAM_STATUS_OUT_OF_BLDG          -- Out of the building \n
      - 0x05 -- SYS_ROAM_STATUS_PREF_SYS          -- Preferred system \n
      - 0x06 -- SYS_ROAM_STATUS_AVAIL_SYS         -- Available system \n
      - 0x07 -- SYS_ROAM_STATUS_ALLIANCE_ PARTNER -- Alliance partner \n
      - 0x08 -- SYS_ROAM_STATUS_PREMIUM_ PARTNER  -- Premium partner \n
      - 0x09 -- SYS_ROAM_STATUS_FULL_SVC          -- Full service \n
      - 0x0A -- SYS_ROAM_STATUS_PARTIAL_SVC       -- Partial service \n
      - 0x0B -- SYS_ROAM_STATUS_BANNER_ON         -- Banner is on \n
      - 0x0C -- SYS_ROAM_STATUS_BANNER_OFF        -- Banner is off \n
        Remainder of the values are per \hyperref[S4]{[S4]}.

        Values from 0x02 onward are only applicable for 3GPP2.
  */

  /*  Is the Forbidden System Valid */
  uint8_t is_sys_forbidden_valid;
  /**<   
      Indicates whether the forbidden system is valid. 
  */

  /*  Indicates Whether the System is Forbidden */
  uint8_t is_sys_forbidden;
  /**<   
      Whether the system is forbidden: \n
      - 0x00 -- Not forbidden \n
      - 0x01 -- Forbidden
  */
}nas_common_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Is the P_Rev in Use Valid */
  uint8_t p_rev_in_use_valid;
  /**<   
      Indicates whether the P_Rev in use is valid. 
  */

  /*  P_Rev in Use */
  uint8_t p_rev_in_use;
  /**<  
     The lesser of the base station P_Rev and mobile P_Rev 
     (only applicable for CDMA).
  */

  /*  Is the Base Station P_Rev Valid */
  uint8_t bs_p_rev_valid;
  /**<   
      Indicates whether the base station P_Rev is valid. 
  */

  /*  bs_p_rev  */
  uint8_t bs_p_rev;
  /**<  
    Base station P_Rev (only applicable for CDMA).
  */

  /*  Is the Supported CCS Valid */
  uint8_t ccs_supported_valid;
  /**<   
      Indicates whether the supported concurrent service is valid. 
  */

  /*  Is CCS Supported  */
  uint8_t ccs_supported;
  /**<  
      Whether concurrent service is supported (only applicable for CDMA): \n
      - 0x00 -- Not supported \n
      - 0x01 -- Supported

  */

  /*  Is the CDMA System ID Valid */
  uint8_t cdma_sys_id_valid;
  /**<   
      Indicates whether the CDMA system ID is valid. 
  */

  /*  CDMA System ID */
  nas_cdma_system_id_type_v01 cdma_sys_id;
  /**<  
     CDMA system ID; includes: \n
     - SID -- System ID \n
     - NID -- Network ID 
  */

  /*  Is the Base Station Information Valid */
  uint8_t bs_info_valid;
  /**<   
      Indicates whether the base station information is valid. 
  */

  /*  Base Station Information */
  nas_cdma_base_station_info_type_v01 bs_info;
  /**<  
   Base station information; includes: \n
   - Base station ID \n
   - Base station latitude \n
   - Base station longitude
  */

  /*  Is the 3GPP2 Packet Zone Valid */
  uint8_t packet_zone_valid;
  /**<   
      Indicates whether the packet zone is valid. 
  */

  /*  3GPP2 Packet Zone */
  uint16_t packet_zone;
  /**<  
    Packet zone (8-bit). 0xFFFF indicates no packet zone. 
    (Only applicable for CDMA.)
  */

  /*  Is the Network ID Valid */
  uint8_t network_id_valid;
  /**<   
      Indicates whether the network ID is valid. 
  */

  /*  Network Name */
  nas_common_network_id_type_v01 network_id;
  /**<  
    Network ID consists of MCC and MNC.
  */
}nas_cdma_only_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Is the HDR Personality Valid */
  uint8_t hdr_personality_valid;
  /**<   
      Indicates whether the HDR personality is valid. 
  */

  /*  HDR Personality */
  nas_hdr_personality_enum_type_v01 hdr_personality;
  /**<  
      HDR personality information (only applicable for HDR). Values: \n
      - 0x00 -- SYS_PERSONALITY_NONE -- None \n
      - 0x02 -- SYS_PERSONALITY_HRPD -- HRPD \n
      - 0x03 -- SYS_PERSONALITY_EHRPD -- eHRPD
  */

  /*  Is the HDR Active Protocol Revision Information Valid */
  uint8_t hdr_active_prot_valid;
  /**<   
      Indicates whether the HDR active protocol revision information is valid.
  */

  /*  HDR Active Protocol Revision Information  */
  nas_hdr_active_prot_enum_type_v01 hdr_active_prot;
  /**<  
      HDR active protocol revision information (only applicable for HDR). 
      Values: \n
      - 0x00 -- SYS_ACTIVE_PROT_NONE -- None           \n
      - 0x02 -- SYS_ACTIVE_PROT_HDR_REL0 -- HDR Rel 0  \n
      - 0x03 -- SYS_ACTIVE_PROT_HDR_RELA -- HDR Rel A  \n
      - 0x04 -- SYS_ACTIVE_PROT_HDR_RELB -- HDR Rel B
  */

  /*  Is the IS-856 System ID Valid */
  uint8_t is856_sys_id_valid;
  /**<   
      Indicates whether the IS-856 system ID is valid. 
  */

  /*  IS 856 */
  uint8_t is856_sys_id[NAS_IS_856_MAX_LEN_V01];
  /**<  
      IS-856 system ID (only applicable for HDR).
    */
}nas_hdr_only_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Is the EGPRS Support Valid */
  uint8_t egprs_supp_valid;
  /**<   
      Indicates whether EGPRS support is valid. 
  */

  /*  EGPRS indication  */
  uint8_t egprs_supp;
  /**<  
      EGPRS support indication (only applicable for GSM). Values: \n
      - 0x00 -- SYS_EGPRS_SUPPORT_NOT_AVAIL -- Not available \n
      - 0x01 -- SYS_EGPRS_SUPPORT_AVAIL -- Available
  */

  /*  Is the DTM Support Valid */
  uint8_t dtm_supp_valid;
  /**<   
      Indicates whether Dual Transfer mode support is valid. 
  */

  /*  DTM support status */
  uint8_t dtm_supp;
  /**<  
      Dual Transfer mode support indication (only applicable for GSM). Values: \n
      - 0x00 -- SYS_DTM_SUPPORT_NOT_AVAIL -- Not available  \n
      - 0x01 -- SYS_DTM_SUPPORT_AVAIL -- Available
  */
}nas_gsm_only_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Is the HS Call Status Valid */
  uint8_t hs_call_status_valid;
  /**<   
      Indicates whether the high-speed call status is valid. 
  */

  /*  HS Call Status */
  nas_hs_support_enum_type_v01 hs_call_status;
  /**<  
      Call status on high speed (only applicable for WCDMA). Values: \n
      - 0x00 -- SYS_HS_IND_HSDPA_HSUPA_ UNSUPP_CELL    -- HSDPA and HSUPA are unsupported \n
      - 0x01 -- SYS_HS_IND_HSDPA_SUPP_CELL             -- HSDPA is supported \n
      - 0x02 -- SYS_HS_IND_HSUPA_SUPP_CELL             -- HSUPA is supported \n
      - 0x03 -- SYS_HS_IND_HSDPA_HSUPA_SUPP_ CELL      -- HSDPA and HSUPA are supported \n
      - 0x04 -- SYS_HS_IND_HSDPAPLUS_SUPP_ CELL        -- HSDPA+ is supported \n
      - 0x05 -- SYS_HS_IND_HSDPAPLUS_HSUPA_ SUPP_CELL  -- HSDPA+ and HSUPA are supported \n
      - 0x06 -- SYS_HS_IND_DC_HSDPAPLUS_SUPP_ CELL     -- Dual-cell HSDPA+ is supported \n
      - 0x07 -- SYS_HS_IND_DC_HSDPAPLUS_ HSUPA_SUPP_CELL    -- Dual-cell HSDPA+ and HSUPA are supported \n 
      - 0x08 -- SYS_HS_IND_HSDPAPLUS_64QAM_ HSUPA_SUPP_CELL -- Dual-cell HSDPA+, 64 QAM, and HSUPA are supported \n
      - 0x09 -- SYS_HS_IND_HSDPAPLUS_64QAM_ SUPP_CELL       -- Dual-cell HSDPA+ and 64 QAM are supported
  */

  /*  Is the HS Service Indication Valid */
  uint8_t hs_ind_valid;
  /**<   
      Indicates whether the high-speed service indication is valid. 
  */

  /*   HS service indication */
  nas_hs_support_enum_type_v01 hs_ind;
  /**<  
      High-speed service indication (only applicable for WCDMA). Values: \n
      - 0x00 -- SYS_HS_IND_HSDPA_HSUPA_ UNSUPP_CELL    -- HSDPA and HSUPA are unsupported \n
      - 0x01 -- SYS_HS_IND_HSDPA_SUPP_CELL             -- HSDPA is supported \n
      - 0x02 -- SYS_HS_IND_HSUPA_SUPP_CELL             -- HSUPA is supported \n
      - 0x03 -- SYS_HS_IND_HSDPA_HSUPA_SUPP_ CELL      -- HSDPA and HSUPA are supported \n
      - 0x04 -- SYS_HS_IND_HSDPAPLUS_SUPP_ CELL        -- HSDPA+ is supported \n
      - 0x05 -- SYS_HS_IND_HSDPAPLUS_HSUPA_ SUPP_CELL  -- HSDPA+ and HSUPA are supported \n
      - 0x06 -- SYS_HS_IND_DC_HSDPAPLUS_SUPP_ CELL     -- Dual-cell HSDPA+ is supported \n
      - 0x07 -- SYS_HS_IND_DC_HSDPAPLUS_ HSUPA_SUPP_CELL    -- Dual-cell HSDPA+ and HSUPA are supported \n 
      - 0x08 -- SYS_HS_IND_HSDPAPLUS_64QAM_ HSUPA_SUPP_CELL -- Dual-cell HSDPA+, 64 QAM, and HSUPA are supported \n
      - 0x09 -- SYS_HS_IND_HSDPAPLUS_64QAM_ SUPP_CELL       -- Dual-cell HSDPA+ and 64 QAM are supported
  */

  /*  Is the PSC Valid */
  uint8_t psc_valid;
  /**<   
      Indicates whether the primary scrambling code is valid. 
  */

  /*  Primary Scrambling Code (PSC) */
  uint16_t psc;
  /**<  
    Primary scrambling code.
    */
}nas_wcdma_only_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Is the TAC Valid */
  uint8_t tac_valid;
  /**<   
      Indicates whether the tracking area code is valid. 
  */

  /*  Tracking Area Code */
  uint16_t tac;
  /**<  
     Tracking area code (only applicable for LTE).
    */
}nas_lte_only_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Is the System PRL Match Valid */
  uint8_t is_sys_prl_match_valid;
  /**<   
      Indicates whether the system PRL match is valid. 
  */

  /*  Indicates if the system is in PRL  */
  uint8_t is_sys_prl_match;
  /**<  
    Indicates whether the system is in a PRL (only applies to CDMA/HDR). 
    Values: \n
    - 0x00 -- System is not in a PRL \n
    - 0x01 -- System is in a PRL

    If the system is not in a PRL, roam_status carries the value from the 
    default roaming indicator in the PRL. \n
    If the system is in a PRL, roam_status is set to the value based on the 
    standard specification.
  */
}nas_cdma_hdr_only_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Is the LAC Valid */
  uint8_t lac_valid;
  /**<   
      Indicates whether the location area code is valid. 
  */

  /*  Location Area Code  */
  uint16_t lac;
  /**<  
    Location area code (only applicable for 3GPP).
  */

  /*  Is the Cell ID Valid */
  uint8_t cell_id_valid;
  /**<   
      Indicates whether the cell ID is valid. 
  */

  /*  Cell ID */
  uint32_t cell_id;
  /**<  
    Cell ID.
  */

  /*  Is the Registration Reject Information Valid */
  uint8_t reg_reject_info_valid;
  /**<   
      Indicates whether the registration reject information is valid. 
  */

  /*  Registration Reject Info */
  nas_reg_reject_info_type_v01 reg_reject_info;

  /*  Is the Network ID Valid */
  uint8_t network_id_valid;
  /**<   
      Indicates whether the network ID is valid. 
  */

  /*  Network name */
  nas_common_network_id_type_v01 network_id;
  /**<  
    Network ID consists of MCC and MNC.
  */
}nas_3gpp_only_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_common_sys_info_type_v01 common_sys_info;

  nas_cdma_hdr_only_sys_info_type_v01 cdma_hdr_only_sys_info;

  nas_cdma_only_sys_info_type_v01 cdma_specific_sys_info;
}nas_cdma_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_common_sys_info_type_v01 common_sys_info;

  nas_cdma_hdr_only_sys_info_type_v01 cdma_hdr_only_sys_info;

  nas_hdr_only_sys_info_type_v01 hdr_specific_sys_info;
}nas_hdr_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_common_sys_info_type_v01 common_sys_info;

  nas_3gpp_only_sys_info_type_v01 threegpp_specific_sys_info;

  nas_gsm_only_sys_info_type_v01 gsm_specific_sys_info;
}nas_gsm_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_common_sys_info_type_v01 common_sys_info;

  nas_3gpp_only_sys_info_type_v01 threegpp_specific_sys_info;

  nas_wcdma_only_sys_info_type_v01 wcdma_specific_sys_info;
}nas_wcdma_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_common_sys_info_type_v01 common_sys_info;

  nas_3gpp_only_sys_info_type_v01 threegpp_specific_sys_info;

  nas_lte_only_sys_info_type_v01 lte_specific_sys_info;
}nas_lte_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Is the HS Call Status Valid */
  uint8_t hs_call_status_valid;
  /**<   
    Indicates whether the high-speed call status is valid. 
  */

  /*  HS Call Status */
  nas_hs_support_enum_type_v01 hs_call_status;
  /**<  
    Call status on high speed (only applicable for WCDMA). Values: \n
    - 0x00 -- SYS_HS_IND_HSDPA_HSUPA_ UNSUPP_CELL    -- HSDPA and HSUPA are unsupported \n
    - 0x01 -- SYS_HS_IND_HSDPA_SUPP_CELL             -- HSDPA is supported \n
    - 0x02 -- SYS_HS_IND_HSUPA_SUPP_CELL             -- HSUPA is supported \n
    - 0x03 -- SYS_HS_IND_HSDPA_HSUPA_SUPP_ CELL      -- HSDPA and HSUPA are supported \n
    - 0x04 -- SYS_HS_IND_HSDPAPLUS_SUPP_ CELL        -- HSDPA+ is supported \n
    - 0x05 -- SYS_HS_IND_HSDPAPLUS_HSUPA_ SUPP_CELL  -- HSDPA+ and HSUPA are supported \n
    - 0x06 -- SYS_HS_IND_DC_HSDPAPLUS_SUPP_ CELL     -- Dual-cell HSDPA+ is supported \n
    - 0x07 -- SYS_HS_IND_DC_HSDPAPLUS_ HSUPA_SUPP_CELL    -- Dual-cell HSDPA+ and HSUPA are supported \n 
    - 0x08 -- SYS_HS_IND_HSDPAPLUS_64QAM_ HSUPA_SUPP_CELL -- Dual-cell HSDPA+, 64 QAM, and HSUPA are supported \n
    - 0x09 -- SYS_HS_IND_HSDPAPLUS_64QAM_ SUPP_CELL       -- Dual-cell HSDPA+ and 64 QAM are supported
  */

  /*  Is the HS Service Indication Valid */
  uint8_t hs_ind_valid;
  /**<   
    Indicates whether the high-speed service indication is valid. 
  */

  /*   HS service indication */
  nas_hs_support_enum_type_v01 hs_ind;
  /**<  
    High-speed service indication (only applicable for WCDMA). Values: \n
    - 0x00 -- SYS_HS_IND_HSDPA_HSUPA_ UNSUPP_CELL    -- HSDPA and HSUPA are unsupported \n
    - 0x01 -- SYS_HS_IND_HSDPA_SUPP_CELL             -- HSDPA is supported \n
    - 0x02 -- SYS_HS_IND_HSUPA_SUPP_CELL             -- HSUPA is supported \n
    - 0x03 -- SYS_HS_IND_HSDPA_HSUPA_SUPP_ CELL      -- HSDPA and HSUPA are supported \n
    - 0x04 -- SYS_HS_IND_HSDPAPLUS_SUPP_ CELL        -- HSDPA+ is supported \n
    - 0x05 -- SYS_HS_IND_HSDPAPLUS_HSUPA_ SUPP_CELL  -- HSDPA+ and HSUPA are supported \n
    - 0x06 -- SYS_HS_IND_DC_HSDPAPLUS_SUPP_ CELL     -- Dual-cell HSDPA+ is supported \n
    - 0x07 -- SYS_HS_IND_DC_HSDPAPLUS_ HSUPA_SUPP_CELL    -- Dual-cell HSDPA+ and HSUPA are supported \n 
    - 0x08 -- SYS_HS_IND_HSDPAPLUS_64QAM_ HSUPA_SUPP_CELL -- Dual-cell HSDPA+, 64 QAM, and HSUPA are supported \n
    - 0x09 -- SYS_HS_IND_HSDPAPLUS_64QAM_ SUPP_CELL       -- Dual-cell HSDPA+ and 64 QAM are supported
  */

  /*  Is the Cell Parameter ID Valid */
  uint8_t cell_parameter_id_valid;
  /**<   
    Indicates whether the cell parameter ID is valid. 
  */

  /*  Cell Parameter ID */
  uint16_t cell_parameter_id;
  /**<  
    Cell parameter ID.
    */

  /*  Is the Cell Broadcast Capability Valid */
  uint8_t cell_broadcast_cap_valid;
  /**<   
    Indicates whether the cell broadcast capability is valid. 
  */

  nas_cell_broadcast_cap_enum_type_v01 cell_broadcast_cap;
  /**<  
    Cell broadcast capability of the serving system. Values: \n
    - 0x00 -- NAS_CELL_BROADCAST_CAP_ UNKNOWN  -- Cell broadcast support is unknown \n
    - 0x01 -- NAS_CELL_BROADCAST_CAP_OFF       -- Cell broadcast is not supported \n
    - 0x02 -- NAS_CELL_BROADCAST_CAP_ON        -- Cell broadcast is supported
  */

  /*  Is the CS Bar Status Valid */
  uint8_t cs_bar_status_valid;
  /**<   
    Indicates whether the circuit-switched call barring status is valid. 
  */

  nas_cell_access_status_e_type_v01 cs_bar_status;
  /**<  
    Call barring status for circuit-switched calls. Values: \n
    - 0x00 -- NAS_CELL_ACCESS_NORMAL_ONLY     -- Cell access is allowed for normal calls only \n
    - 0x01 -- NAS_CELL_ACCESS_EMERGENCY_ ONLY -- Cell access is allowed for emergency calls only \n
    - 0x02 -- NAS_CELL_ACCESS_NO_CALLS        -- Cell access is not allowed for any call type \n
    - 0x03 -- NAS_CELL_ACCESS_ALL_CALLS       -- Cell access is allowed for all call types \n
    -   -1 -- NAS_CELL_ACCESS_UNKNOWN         -- Cell access type is unknown
  */

  /*  Is the PS Bar Status Valid */
  uint8_t ps_bar_status_valid;
  /**<   
    Indicates whether the packet-switched call barring status is valid. 
  */

  nas_cell_access_status_e_type_v01 ps_bar_status;
  /**<  
     Call barring status for packet-switched calls. Values: \n
    - 0x00 -- NAS_CELL_ACCESS_NORMAL_ONLY     -- Cell access is allowed for normal calls only \n
    - 0x01 -- NAS_CELL_ACCESS_EMERGENCY_ ONLY -- Cell access is allowed for emergency calls only \n
    - 0x02 -- NAS_CELL_ACCESS_NO_CALLS        -- Cell access is not allowed for any call type \n
    - 0x03 -- NAS_CELL_ACCESS_ALL_CALLS       -- Cell access is allowed for all call types \n
    -   -1 -- NAS_CELL_ACCESS_UNKNOWN         -- Cell access type is unknown
  */

  /*  Is the Cipher Domain Valid */
  uint8_t cipher_domain_valid;
  /**<   
    Indicates whether the cipher domain is valid. 
  */

  nas_service_domain_enum_type_v01 cipher_domain;
  /**<  
    Ciphering on the service domain. Values: \n
    - 0x00 -- SYS_SRV_DOMAIN_NO_SRV  -- No service \n
    - 0x01 -- SYS_SRV_DOMAIN_CS_ONLY -- Circuit-switched only \n
    - 0x02 -- SYS_SRV_DOMAIN_PS_ONLY -- Packet-switched only \n
    - 0x03 -- SYS_SRV_DOMAIN_CS_PS   -- Circuit-switched and packet-switched
  */
}nas_tdscdma_only_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_common_sys_info_type_v01 common_sys_info;

  nas_3gpp_only_sys_info_type_v01 threegpp_specific_sys_info;

  nas_tdscdma_only_sys_info_type_v01 tdscdma_specific_sys_info;
}nas_tdscdma_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t geo_sys_idx;
  /**<  
    System table index referencing the beginning of the geo in which
    the current serving system is present. When the system index 
    is not known, 0xFFFF is used.
  */

  uint16_t reg_prd;
  /**<  
    Registration period after the CDMA system is acquired. 
    When the CDMA registration period is not valid, 0xFFFF is used. 
  */
}nas_cdma_sys_info2_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t geo_sys_idx;
  /**<  
    System table index referencing the beginning of the geo in which
    the current serving system is present. When the system index
    is not known, 0xFFFF is used.
  */
}nas_hdr_sys_info2_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t geo_sys_idx;
  /**<  
    System table index referencing the beginning of the geo in which
    the current serving system is present. When the system index 
    is not known, 0xFFFF is used.
  */

  nas_cell_broadcast_cap_enum_type_v01 cell_broadcast_cap;
  /**<  
    Cell broadcast capability of the serving system. Values: \n
    - 0x00 -- NAS_CELL_BROADCAST_CAP_ UNKNOWN -- Cell broadcast support is unknown \n
    - 0x01 -- NAS_CELL_BROADCAST_CAP_OFF      -- Cell broadcast is not supported \n
    - 0x02 -- NAS_CELL_BROADCAST_CAP_ON       -- Cell broadcast is supported
  */
}nas_gsm_sys_info2_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t geo_sys_idx;
  /**<  
    System table index referencing the beginning of the geo in which
    the current serving system is present. When the system index 
    is not known, 0xFFFF is used.
  */

  nas_cell_broadcast_cap_enum_type_v01 cell_broadcast_cap;
  /**<  
    Cell broadcast capability of the serving system. Values: \n
    - 0x00 -- NAS_CELL_BROADCAST_CAP_ UNKNOWN  -- Cell broadcast support is unknown \n
    - 0x01 -- NAS_CELL_BROADCAST_CAP_OFF       -- Cell broadcast is not supported \n
    - 0x02 -- NAS_CELL_BROADCAST_CAP_ON        -- Cell broadcast is supported
  */
}nas_wcdma_sys_info2_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t geo_sys_idx;
  /**<  
    System table index referencing the beginning of the geo in which
    the current serving system is present. When the system index 
    is not known, 0xFFFF is used.
  */
}nas_lte_sys_info2_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SIM_REJ_INFO_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SIM_NOT_AVAILABLE_V01 = 0, /**<  SIM is not available.  */
  NAS_SIM_AVAILABLE_V01 = 1, /**<  SIM is available.  */
  NAS_SIM_CS_INVALID_V01 = 2, /**<  SIM has been marked by the network as invalid for CS services.  */
  NAS_SIM_PS_INVALID_V01 = 3, /**<  SIM has been marked by the network as invalid for PS services.  */
  NAS_SIM_CS_PS_INVALID_V01 = 4, /**<  SIM has been marked by the network as invalid for CS and PS services.  */
  NAS_SIM_REJ_INFO_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_sim_rej_info_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_LTE_VOICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_LTE_VOICE_STATUS_NO_VOICE_V01 = 0, /**<  Data centric devices: No voice, stay on LTE  */
  NAS_LTE_VOICE_STATUS_IMS_V01 = 1, /**<  Voice is supported over IMS network  */
  NAS_LTE_VOICE_STATUS_1X_V01 = 2, /**<  Voice is supported over 1X network  */
  NAS_LTE_VOICE_STATUS_3GPP_V01 = 3, /**<  Voice is supported over 3GPP network  */
  NAS_LTE_VOICE_STATUS_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_lte_voice_status_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_POSSIBLE_REG_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_POSSIBLE_REG_DOMAIN_NA_V01 = 0, 
  NAS_POSSIBLE_REG_DOMAIN_CS_ONLY_V01 = 1, 
  NAS_POSSIBLE_REG_DOMAIN_PS_ONLY_V01 = 2, 
  NAS_POSSIBLE_REG_DOMAIN_CS_PS_V01 = 3, 
  NAS_POSSIBLE_REG_DOMAIN_LIMITED_SERVICE_V01 = 4, 
  NAS_POSSIBLE_REG_DOMAIN_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_possible_reg_domain_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SMS_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SMS_STATUS_NO_SMS_V01 = 0, /**<  Data centric devices: No sms, stay on network  */
  NAS_SMS_STATUS_IMS_V01 = 1, /**<  SMS is supported over IMS network  */
  NAS_SMS_STATUS_1X_V01 = 2, /**<  SMS is supported over 1X network  */
  NAS_SMS_STATUS_3GPP_V01 = 3, /**<  SMS is supported over 3GPP network  */
  NAS_SMS_STATUS_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_sms_status_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Provides the system information.
               \label{idl:getSysInfo} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  CDMA Service Status Info */
  uint8_t cdma_srv_status_info_valid;  /**< Must be set to true if cdma_srv_status_info is being passed */
  nas_3gpp2_srv_status_info_type_v01 cdma_srv_status_info;

  /* Optional */
  /*  HDR Service Status Info */
  uint8_t hdr_srv_status_info_valid;  /**< Must be set to true if hdr_srv_status_info is being passed */
  nas_3gpp2_srv_status_info_type_v01 hdr_srv_status_info;

  /* Optional */
  /*  GSM Service Status Info */
  uint8_t gsm_srv_status_info_valid;  /**< Must be set to true if gsm_srv_status_info is being passed */
  nas_3gpp_srv_status_info_type_v01 gsm_srv_status_info;

  /* Optional */
  /*  WCDMA Service Status Info */
  uint8_t wcdma_srv_status_info_valid;  /**< Must be set to true if wcdma_srv_status_info is being passed */
  nas_3gpp_srv_status_info_type_v01 wcdma_srv_status_info;

  /* Optional */
  /*  LTE Service Status Info */
  uint8_t lte_srv_status_info_valid;  /**< Must be set to true if lte_srv_status_info is being passed */
  nas_3gpp_srv_status_info_type_v01 lte_srv_status_info;

  /* Optional */
  /*  CDMA System Info */
  uint8_t cdma_sys_info_valid;  /**< Must be set to true if cdma_sys_info is being passed */
  nas_cdma_sys_info_type_v01 cdma_sys_info;

  /* Optional */
  /*  HDR System Info */
  uint8_t hdr_sys_info_valid;  /**< Must be set to true if hdr_sys_info is being passed */
  nas_hdr_sys_info_type_v01 hdr_sys_info;

  /* Optional */
  /*  GSM System Info */
  uint8_t gsm_sys_info_valid;  /**< Must be set to true if gsm_sys_info is being passed */
  nas_gsm_sys_info_type_v01 gsm_sys_info;

  /* Optional */
  /*  WCDMA System Info */
  uint8_t wcdma_sys_info_valid;  /**< Must be set to true if wcdma_sys_info is being passed */
  nas_wcdma_sys_info_type_v01 wcdma_sys_info;

  /* Optional */
  /*  LTE System Info */
  uint8_t lte_sys_info_valid;  /**< Must be set to true if lte_sys_info is being passed */
  nas_lte_sys_info_type_v01 lte_sys_info;

  /* Optional */
  /*  Additional CDMA System Info */
  uint8_t cdma_sys_info2_valid;  /**< Must be set to true if cdma_sys_info2 is being passed */
  nas_cdma_sys_info2_type_v01 cdma_sys_info2;

  /* Optional */
  /*  Additional HDR System Info */
  uint8_t hdr_sys_info2_valid;  /**< Must be set to true if hdr_sys_info2 is being passed */
  nas_hdr_sys_info2_type_v01 hdr_sys_info2;

  /* Optional */
  /*  Additional GSM System Info */
  uint8_t gsm_sys_info2_valid;  /**< Must be set to true if gsm_sys_info2 is being passed */
  nas_gsm_sys_info2_type_v01 gsm_sys_info2;

  /* Optional */
  /*  Additional WCDMA System Info */
  uint8_t wcdma_sys_info2_valid;  /**< Must be set to true if wcdma_sys_info2 is being passed */
  nas_wcdma_sys_info2_type_v01 wcdma_sys_info2;

  /* Optional */
  /*  Additional LTE System Info */
  uint8_t lte_sys_info2_valid;  /**< Must be set to true if lte_sys_info2 is being passed */
  nas_lte_sys_info2_type_v01 lte_sys_info2;

  /* Optional */
  /*  GSM Call Barring System Info */
  uint8_t gsm_sys_info3_valid;  /**< Must be set to true if gsm_sys_info3 is being passed */
  nas_gw_sys_info3_type_v01 gsm_sys_info3;

  /* Optional */
  /*  WCDMA Call Barring System Info */
  uint8_t wcdma_sys_info3_valid;  /**< Must be set to true if wcdma_sys_info3 is being passed */
  nas_gw_sys_info3_type_v01 wcdma_sys_info3;

  /* Optional */
  /*  LTE Voice Support Sys Info */
  uint8_t voice_support_on_lte_valid;  /**< Must be set to true if voice_support_on_lte is being passed */
  uint8_t voice_support_on_lte;
  /**<  
    Indicates voice support status on LTE. Values: \n
    - 0x00 -- Voice is not supported \n
    - 1x01 -- Voice is supported
   */

  /* Optional */
  /*  GSM Cipher Domain Sys Info */
  uint8_t gsm_cipher_domain_valid;  /**< Must be set to true if gsm_cipher_domain is being passed */
  nas_service_domain_enum_type_v01 gsm_cipher_domain;
  /**<  
    Ciphering on the service domain. Values: \n
    - 0x00 -- SYS_SRV_DOMAIN_NO_SRV  -- No service \n
    - 0x01 -- SYS_SRV_DOMAIN_CS_ONLY -- Circuit-switched only \n
    - 0x02 -- SYS_SRV_DOMAIN_PS_ONLY -- Packet-switched only \n
    - 0x03 -- SYS_SRV_DOMAIN_CS_PS   -- Circuit-switched and packet-switched
   */

  /* Optional */
  /*  WCDMA Cipher Domain Sys Info */
  uint8_t wcdma_cipher_domain_valid;  /**< Must be set to true if wcdma_cipher_domain is being passed */
  nas_service_domain_enum_type_v01 wcdma_cipher_domain;
  /**<  
    Ciphering on the service domain. Values: \n
    - 0x00 -- SYS_SRV_DOMAIN_NO_SRV  -- No service \n
    - 0x01 -- SYS_SRV_DOMAIN_CS_ONLY -- Circuit-switched only \n
    - 0x02 -- SYS_SRV_DOMAIN_PS_ONLY -- Packet-switched only \n
    - 0x03 -- SYS_SRV_DOMAIN_CS_PS   -- Circuit-switched and packet-switched
   */

  /* Optional */
  /*  TDSCDMA Service Status Info */
  uint8_t tdscdma_srv_status_info_valid;  /**< Must be set to true if tdscdma_srv_status_info is being passed */
  nas_3gpp_srv_status_info_type_v01 tdscdma_srv_status_info;

  /* Optional */
  /*  TDSCDMA System Info */
  uint8_t tdscdma_sys_info_valid;  /**< Must be set to true if tdscdma_sys_info is being passed */
  nas_tdscdma_sys_info_type_v01 tdscdma_sys_info;

  /* Optional */
  /*  LTE eMBMS Coverage Info */
  uint8_t lte_embms_coverage_valid;  /**< Must be set to true if lte_embms_coverage is being passed */
  uint8_t lte_embms_coverage;
  /**<  
    Values: \n
    - TRUE  -- Current LTE system supports eMBMS \n
    - FALSE -- Current LTE system does not support eMBMS
  */

  /* Optional */
  /*  SIM Reject Information */
  uint8_t sim_rej_info_valid;  /**< Must be set to true if sim_rej_info is being passed */
  nas_sim_rej_info_enum_type_v01 sim_rej_info;
  /**<  
    Current reject state information of the SIM. Values: \n
    - 0 -- NAS_SIM_NOT_AVAILABLE -- SIM is not available \n
    - 1 -- NAS_SIM_AVAILABLE     -- SIM is available     \n
    - 2 -- NAS_SIM_CS_INVALID    -- SIM has been marked by the network as 
                                    invalid for circuit-switched services \n
    - 3 -- NAS_SIM_PS_INVALID    -- SIM has been marked by the network as 
                                    invalid for packet-switched services  \n
    - 4 -- NAS_SIM_CS_PS_INVALID -- SIM has been marked by the network as 
                                    invalid for circuit-switched and 
                                    packet-switched services
  */

  /* Optional */
  /*  WCDMA EUTRA Status Information */
  uint8_t wcdma_eutra_status_valid;  /**< Must be set to true if wcdma_eutra_status is being passed */
  nas_eutra_cell_status_enum_type_v01 wcdma_eutra_status;
  /**<  
    E-UTRA detection status. Values: \n
    - 0 -- NAS_EUTRA_CELL_PRESENT           -- E-UTRA cell is detected            \n
    - 1 -- NAS_EUTRA_CELL_NOT_PRESENT       -- E-UTRA cell is not detected        \n
    - 2 -- NAS_EUTRA_CELL_PRESENCE_ UNKNOWN -- E-UTRA cell information is unknown 
                                               due to a state transition          \n
    - 3 -- NAS_EUTRA_CELL_DETECTION_ UNSUPPORTED -- E-UTRA detection is not supported
  */

  /* Optional */
  /*  IMS Voice Support Status on LTE */
  uint8_t lte_ims_voice_avail_valid;  /**< Must be set to true if lte_ims_voice_avail is being passed */
  uint8_t lte_ims_voice_avail;
  /**<   
    Values: \n
    - 0x00 -- Support is not available \n
    - 0x01 -- Support is available
    */

  /* Optional */
  /*  LTE Voice Domain */
  uint8_t lte_voice_status_valid;  /**< Must be set to true if lte_voice_status is being passed */
  nas_lte_voice_status_enum_type_v01 lte_voice_status;
  /**<  
    LTE voice domain. Values: \n
    - 0 -- NAS_DOMAIN_SEL_DOMAIN_NO_VOICE -- Data-centric devices: 
                                             No voice, stay on LTE \n
    - 1 -- NAS_DOMAIN_SEL_DOMAIN_IMS      -- Voice is supported over the IMS network \n
    - 2 -- NAS_DOMAIN_SEL_DOMAIN_1X       -- Voice is supported over the 1X network \n
    - 3 -- NAS_DOMAIN_SEL_DOMAIN_3GPP     -- Voice is supported over the 3GPP network 
  */

  /* Optional */
  /*  CDMA Reg Zone ID */
  uint8_t cdma_reg_zone_valid;  /**< Must be set to true if cdma_reg_zone is being passed */
  uint16_t cdma_reg_zone;
  /**<  
    CDMA registration zone ID.
  */

  /* Optional */
  /*  GSM RAC */
  uint8_t gsm_rac_valid;  /**< Must be set to true if gsm_rac is being passed */
  uint8_t gsm_rac;
  /**<  
    GSM routing area code.
  */

  /* Optional */
  /*  WCDMA RAC */
  uint8_t wcdma_rac_valid;  /**< Must be set to true if wcdma_rac is being passed */
  uint8_t wcdma_rac;
  /**<  
    WCDMA routing area code.
  */

  /* Optional */
  /*  CDMA Resolved Mobile Country Code */
  uint8_t cdma_mcc_resolved_via_sid_lookup_valid;  /**< Must be set to true if cdma_mcc_resolved_via_sid_lookup is being passed */
  uint16_t cdma_mcc_resolved_via_sid_lookup;
  /**<  
    MCC derived by looking up the IFAST SID conflict table and configured 
    SID-MCC table (static and NV) with the SID received from the network as the 
    key. If the lookup is not successful, 0xFFFF is used. \n
    Note: This MCC value is determined solely from the SID and may differ from 
    the MCC value sent by the network.
  */

  /* Optional */
  /*  Network Selection Registration Restriction */
  uint8_t srv_reg_restriction_valid;  /**< Must be set to true if srv_reg_restriction is being passed */
  nas_srv_reg_restriction_enum_v01 srv_reg_restriction;
  /**<  
    Registration restriction. Values: \n
    - 0x00 -- NAS_SRV_REG_RESTRICTION_ UNRESTRICTED -- Device follows the normal 
              registration process \n
    - 0x01 -- NAS_SRV_REG_RESTRICTION_ CAMPED_ONLY -- Device follows the camp-only 
              registration process

    \vspace{3pt}
    All other values are reserved.
   */

  /* Optional */
  /*  TDSCDMA Registration Domain */
  uint8_t tdscdma_reg_domain_valid;  /**< Must be set to true if tdscdma_reg_domain is being passed */
  nas_possible_reg_domain_enum_type_v01 tdscdma_reg_domain;
  /**<  
    TD-SCDMA registration domain. Values: \n
    - 0 -- NAS_POSSIBLE_REG_DOMAIN_NA        -- Not applicable because the UE 
           is not in Camp Only mode \n
    - 1 -- NAS_POSSIBLE_REG_DOMAIN_CS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide CS service only \n
    - 2 -- NAS_POSSIBLE_REG_DOMAIN_PS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide PS service only \n
    - 3 -- NAS_POSSIBLE_REG_DOMAIN_CS_PS     -- UE is in Camp Only mode and the 
           PLMN can provide CS and PS service \n
    - 4 -- NAS_POSSIBLE_REG_DOMAIN_ LIMITED_SERVICE -- UE is in Camp Only mode, but 
           the PLMN cannot provide any service
  */

  /* Optional */
  /*  LTE Registration Domain */
  uint8_t lte_reg_domain_valid;  /**< Must be set to true if lte_reg_domain is being passed */
  nas_possible_reg_domain_enum_type_v01 lte_reg_domain;
  /**<  
    LTE registration domain. Values: \n
    - 0 -- NAS_POSSIBLE_REG_DOMAIN_NA        -- Not applicable because the UE 
           is not in Camp Only mode \n
    - 1 -- NAS_POSSIBLE_REG_DOMAIN_CS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide CS service only \n
    - 2 -- NAS_POSSIBLE_REG_DOMAIN_PS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide PS service only \n
    - 3 -- NAS_POSSIBLE_REG_DOMAIN_CS_PS     -- UE is in Camp Only mode and the 
           PLMN can provide CS and PS service \n
    - 4 -- NAS_POSSIBLE_REG_DOMAIN_ LIMITED_SERVICE -- UE is in Camp Only mode, but 
           the PLMN cannot provide any service
  */

  /* Optional */
  /*  WCDMA Registration Domain */
  uint8_t wcdma_reg_domain_valid;  /**< Must be set to true if wcdma_reg_domain is being passed */
  nas_possible_reg_domain_enum_type_v01 wcdma_reg_domain;
  /**<  
    WCDMA registration domain. Values: \n
    - 0 -- NAS_POSSIBLE_REG_DOMAIN_NA        -- Not applicable because the UE 
           is not in Camp Only mode \n
    - 1 -- NAS_POSSIBLE_REG_DOMAIN_CS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide CS service only \n
    - 2 -- NAS_POSSIBLE_REG_DOMAIN_PS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide PS service only \n
    - 3 -- NAS_POSSIBLE_REG_DOMAIN_CS_PS     -- UE is in Camp Only mode and the 
           PLMN can provide CS and PS service \n
    - 4 -- NAS_POSSIBLE_REG_DOMAIN_ LIMITED_SERVICE -- UE is in Camp Only mode, but 
           the PLMN cannot provide any service
  */

  /* Optional */
  /*  GSM Registration Domain */
  uint8_t gsm_reg_domain_valid;  /**< Must be set to true if gsm_reg_domain is being passed */
  nas_possible_reg_domain_enum_type_v01 gsm_reg_domain;
  /**<  
    GSM registration domain. Values: \n
    - 0 -- NAS_POSSIBLE_REG_DOMAIN_NA        -- Not applicable because the UE 
           is not in Camp Only mode \n
    - 1 -- NAS_POSSIBLE_REG_DOMAIN_CS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide CS service only \n
    - 2 -- NAS_POSSIBLE_REG_DOMAIN_PS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide PS service only \n
    - 3 -- NAS_POSSIBLE_REG_DOMAIN_CS_PS     -- UE is in Camp Only mode and the 
           PLMN can provide CS and PS service \n
    - 4 -- NAS_POSSIBLE_REG_DOMAIN_ LIMITED_SERVICE -- UE is in Camp Only mode, but 
           the PLMN cannot provide any service
  */

  /* Optional */
  /*  LTE eMBMS Coverage Info Trace ID */
  uint8_t lte_embms_coverage_trace_id_valid;  /**< Must be set to true if lte_embms_coverage_trace_id is being passed */
  int16_t lte_embms_coverage_trace_id;
  /**<   
    LTE eMBMS coverage information trace ID. Values: \n
    - 0 to 32768 -- Valid trace ID \n
    - -1 -- Trace ID is not used
  */

  /* Optional */
  /*  WCDMA CSG Information */
  uint8_t wcdma_csg_info_valid;  /**< Must be set to true if wcdma_csg_info is being passed */
  nas_csg_info_type_v01 wcdma_csg_info;

  /* Optional */
  /*  HDR Voice Domain */
  uint8_t hdr_voice_status_valid;  /**< Must be set to true if hdr_voice_status is being passed */
  nas_lte_voice_status_enum_type_v01 hdr_voice_status;
  /**<  
    HDR voice domain. Values: \n
    - 0 -- NAS_DOMAIN_SEL_DOMAIN_NO_VOICE -- Data-centric devices: 
                                             No voice, stay on HDR \n
    - 1 -- NAS_DOMAIN_SEL_DOMAIN_IMS      -- Voice is supported over the IMS network \n
    - 2 -- NAS_DOMAIN_SEL_DOMAIN_1X       -- Voice is supported over the 1X network
  */

  /* Optional */
  /*  HDR SMS Domain */
  uint8_t hdr_sms_status_valid;  /**< Must be set to true if hdr_sms_status is being passed */
  nas_sms_status_enum_type_v01 hdr_sms_status;
  /**<  
    HDR SMS domain. Values: \n
    - 0 -- NAS_SMS_STATUS_NO_SMS -- Data-centric devices: 
                                    No SMS, stay on HDR \n
    - 1 -- NAS_SMS_STATUS_IMS    -- SMS is supported over the IMS network \n
    - 2 -- NAS_SMS_STATUS_1X     -- SMS is supported over the 1X network
  */

  /* Optional */
  /*  LTE SMS Domain */
  uint8_t lte_sms_status_valid;  /**< Must be set to true if lte_sms_status is being passed */
  nas_sms_status_enum_type_v01 lte_sms_status;
  /**<  
    LTE SMS domain. Values: \n
    - 0 -- NAS_SMS_STATUS_NO_SMS -- Data-centric devices: 
                                    No SMS, stay on LTE \n
    - 1 -- NAS_SMS_STATUS_IMS    -- SMS is supported over the IMS network \n
    - 2 -- NAS_SMS_STATUS_1X     -- SMS is supported over the 1X network \n
    - 3 -- NAS_SMS_STATUS_3GPP   -- SMS is supported over the 3GPP network 
  */

  /* Optional */
  /*  LTE Emergency Bearer Support */
  uint8_t lte_is_eb_supported_valid;  /**< Must be set to true if lte_is_eb_supported is being passed */
  nas_tri_state_boolean_type_v01 lte_is_eb_supported;
  /**<  
 Whether LTE emergency bearer is supported. Values: \n
      - NAS_TRI_FALSE (0) --  Status: FALSE \n 
      - NAS_TRI_TRUE (1) --  Status: TRUE  \n 
      - NAS_TRI_UNKNOWN (2) --  Status: Unknown  

 \vspace{3pt}
 The TLV status is NAS_TRI_UNKNOWN for scenarios where information is not 
 available from the lower layers; e.g., if the UE powers up while acquiring 
 service or in the middle of an attach procedure.
 */

  /* Optional */
  /*  GSM Voice Domain */
  uint8_t gsm_voice_status_valid;  /**< Must be set to true if gsm_voice_status is being passed */
  nas_lte_voice_status_enum_type_v01 gsm_voice_status;
  /**<  
    GSM voice domain. Values: \n
    - 0 -- NAS_DOMAIN_SEL_DOMAIN_NO_VOICE -- Data-centric devices: 
                                             No voice, stay on GSM \n
    - 1 -- NAS_DOMAIN_SEL_DOMAIN_IMS      -- Voice is supported over the IMS network \n
    - 2 -- NAS_DOMAIN_SEL_DOMAIN_1X       -- Voice is supported over the 1X network
  */

  /* Optional */
  /*  GSM SMS Domain */
  uint8_t gsm_sms_status_valid;  /**< Must be set to true if gsm_sms_status is being passed */
  nas_sms_status_enum_type_v01 gsm_sms_status;
  /**<  
    GSM SMS domain. Values: \n
    - 0 -- NAS_SMS_STATUS_NO_SMS -- Data-centric devices: 
                                    No SMS, stay on GSM \n
    - 1 -- NAS_SMS_STATUS_IMS    -- SMS is supported over the IMS network \n
    - 2 -- NAS_SMS_STATUS_1X     -- SMS is supported over the 1X network
  */

  /* Optional */
  /*  WCDMA Voice Domain */
  uint8_t wcdma_voice_status_valid;  /**< Must be set to true if wcdma_voice_status is being passed */
  nas_lte_voice_status_enum_type_v01 wcdma_voice_status;
  /**<  
    WCDMA voice domain. Values: \n
    - 0 -- NAS_DOMAIN_SEL_DOMAIN_NO_VOICE -- Data-centric devices: 
                                             No voice, stay on WCDMA \n
    - 1 -- NAS_DOMAIN_SEL_DOMAIN_IMS      -- Voice is supported over the IMS network \n
    - 2 -- NAS_DOMAIN_SEL_DOMAIN_1X       -- Voice is supported over the 1X network
  */

  /* Optional */
  /*  WCDMA SMS Domain */
  uint8_t wcdma_sms_status_valid;  /**< Must be set to true if wcdma_sms_status is being passed */
  nas_sms_status_enum_type_v01 wcdma_sms_status;
  /**<  
    WCDMA SMS domain. Values: \n
    - 0 -- NAS_SMS_STATUS_NO_SMS -- Data-centric devices: 
                                    No SMS, stay on WCDMA \n
    - 1 -- NAS_SMS_STATUS_IMS    -- SMS is supported over the IMS network \n
    - 2 -- NAS_SMS_STATUS_1X     -- SMS is supported over the 1X network
  */

  /* Optional */
  /*  LTE Emergency Access Barred */
  uint8_t emergency_access_barred_valid;  /**< Must be set to true if emergency_access_barred is being passed */
  nas_tri_state_boolean_type_v01 emergency_access_barred;
  /**<  
 Whether LTE emergency access is barred on the current system. Values: \n
      - NAS_TRI_FALSE (0) --  Status: FALSE \n 
      - NAS_TRI_TRUE (1) --  Status: TRUE  \n 
      - NAS_TRI_UNKNOWN (2) --  Status: Unknown  

 \vspace{3pt}
 The TLV status is NAS_TRI_UNKNOWN for scenarios where information is not 
 available from the lower layers; e.g., if the UE powers up while acquiring 
 service or in the middle of an attach procedure.
 */

  /* Optional */
  /*  CDMA Voice Domain */
  uint8_t cdma_voice_status_valid;  /**< Must be set to true if cdma_voice_status is being passed */
  nas_lte_voice_status_enum_type_v01 cdma_voice_status;
  /**<  
    CDMA voice domain. Values: \n
    - 0 -- NAS_DOMAIN_SEL_DOMAIN_NO_VOICE -- Data-centric devices: 
                                             No voice, stay on CDMA \n
    - 1 -- NAS_DOMAIN_SEL_DOMAIN_IMS      -- Voice is supported over the IMS network \n
    - 2 -- NAS_DOMAIN_SEL_DOMAIN_1X       -- Voice is supported over the 1X network
  */

  /* Optional */
  /*  CDMA SMS Domain */
  uint8_t cdma_sms_status_valid;  /**< Must be set to true if cdma_sms_status is being passed */
  nas_sms_status_enum_type_v01 cdma_sms_status;
  /**<  
    CDMA SMS domain. Values: \n
    - 0 -- NAS_SMS_STATUS_NO_SMS -- Data-centric devices: 
                                    No SMS, stay on CDMA \n
    - 1 -- NAS_SMS_STATUS_IMS    -- SMS is supported over the IMS network \n
    - 2 -- NAS_SMS_STATUS_1X     -- SMS is supported over the 1X network
  */

  /* Optional */
  /*  TDSCDMA Voice Domain */
  uint8_t tdscdma_voice_status_valid;  /**< Must be set to true if tdscdma_voice_status is being passed */
  nas_lte_voice_status_enum_type_v01 tdscdma_voice_status;
  /**<  
    TD-SCDMA voice domain. Values: \n
    - 0 -- NAS_DOMAIN_SEL_DOMAIN_NO_VOICE -- Data-centric devices: 
                                             No voice, stay on TD-SCDMA \n
    - 1 -- NAS_DOMAIN_SEL_DOMAIN_IMS      -- Voice is supported over the IMS network \n
    - 2 -- NAS_DOMAIN_SEL_DOMAIN_1X       -- Voice is supported over the 1X network
  */

  /* Optional */
  /*  TDSCDMA SMS Domain */
  uint8_t tdscdma_sms_status_valid;  /**< Must be set to true if tdscdma_sms_status is being passed */
  nas_sms_status_enum_type_v01 tdscdma_sms_status;
  /**<  
    TD-SCDMA SMS domain. Values: \n
    - 0 -- NAS_SMS_STATUS_NO_SMS -- Data-centric devices: 
                                    No SMS, stay on TD-SCDMA \n
    - 1 -- NAS_SMS_STATUS_IMS    -- SMS is supported over the IMS network \n
    - 2 -- NAS_SMS_STATUS_1X     -- SMS is supported over the 1X network
  */

  /* Optional */
  /*  LTE CSG Information */
  uint8_t lte_csg_info_valid;  /**< Must be set to true if lte_csg_info is being passed */
  nas_csg_info_type_v01 lte_csg_info;

  /* Optional */
  /*  LTE Cell Access Status Info */
  uint8_t lte_cell_status_valid;  /**< Must be set to true if lte_cell_status is being passed */
  nas_cell_access_status_e_type_v01 lte_cell_status;
  /**<  
 Cell access status for LTE calls. Values: \n
      - NAS_CELL_ACCESS_NORMAL_ONLY (0x00) --  Cell access is allowed for normal calls only \n  
      - NAS_CELL_ACCESS_EMERGENCY_ONLY (0x01) --  Cell access is allowed for emergency calls only \n  
      - NAS_CELL_ACCESS_NO_CALLS (0x02) --  Cell access is not allowed for any call type \n  
      - NAS_CELL_ACCESS_ALL_CALLS (0x03) --  Cell access is allowed for all call types \n  
      - NAS_CELL_ACCESS_UNKNOWN (-1) --  Cell access type is unknown  
 */

  /* Optional */
  /*  HDR Subnet Mask Length */
  uint8_t hdr_subnet_mask_len_valid;  /**< Must be set to true if hdr_subnet_mask_len is being passed */
  uint8_t hdr_subnet_mask_len;
  /**<  
      HDR Subnet Mask Length.
  */
}nas_get_sys_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the system information.   
             \label{idl:sysInfoInd} */
typedef struct {

  /* Optional */
  /*  CDMA Service Status Info */
  uint8_t cdma_srv_status_info_valid;  /**< Must be set to true if cdma_srv_status_info is being passed */
  nas_3gpp2_srv_status_info_type_v01 cdma_srv_status_info;

  /* Optional */
  /*  HDR Service Status Info */
  uint8_t hdr_srv_status_info_valid;  /**< Must be set to true if hdr_srv_status_info is being passed */
  nas_3gpp2_srv_status_info_type_v01 hdr_srv_status_info;

  /* Optional */
  /*  GSM Service Status Info */
  uint8_t gsm_srv_status_info_valid;  /**< Must be set to true if gsm_srv_status_info is being passed */
  nas_3gpp_srv_status_info_type_v01 gsm_srv_status_info;

  /* Optional */
  /*  WCDMA Service Status Info */
  uint8_t wcdma_srv_status_info_valid;  /**< Must be set to true if wcdma_srv_status_info is being passed */
  nas_3gpp_srv_status_info_type_v01 wcdma_srv_status_info;

  /* Optional */
  /*  LTE Service Status Info */
  uint8_t lte_srv_status_info_valid;  /**< Must be set to true if lte_srv_status_info is being passed */
  nas_3gpp_srv_status_info_type_v01 lte_srv_status_info;

  /* Optional */
  /*  CDMA System Info */
  uint8_t cdma_sys_info_valid;  /**< Must be set to true if cdma_sys_info is being passed */
  nas_cdma_sys_info_type_v01 cdma_sys_info;

  /* Optional */
  /*  HDR System Info */
  uint8_t hdr_sys_info_valid;  /**< Must be set to true if hdr_sys_info is being passed */
  nas_hdr_sys_info_type_v01 hdr_sys_info;

  /* Optional */
  /*  GSM System Info */
  uint8_t gsm_sys_info_valid;  /**< Must be set to true if gsm_sys_info is being passed */
  nas_gsm_sys_info_type_v01 gsm_sys_info;

  /* Optional */
  /*  WCDMA System Info */
  uint8_t wcdma_sys_info_valid;  /**< Must be set to true if wcdma_sys_info is being passed */
  nas_wcdma_sys_info_type_v01 wcdma_sys_info;

  /* Optional */
  /*  LTE System Info */
  uint8_t lte_sys_info_valid;  /**< Must be set to true if lte_sys_info is being passed */
  nas_lte_sys_info_type_v01 lte_sys_info;

  /* Optional */
  /*  Additional CDMA System Info */
  uint8_t cdma_sys_info2_valid;  /**< Must be set to true if cdma_sys_info2 is being passed */
  nas_cdma_sys_info2_type_v01 cdma_sys_info2;

  /* Optional */
  /*  Additional HDR System Info */
  uint8_t hdr_sys_info2_valid;  /**< Must be set to true if hdr_sys_info2 is being passed */
  nas_hdr_sys_info2_type_v01 hdr_sys_info2;

  /* Optional */
  /*  Additional GSM System Info */
  uint8_t gsm_sys_info2_valid;  /**< Must be set to true if gsm_sys_info2 is being passed */
  nas_gsm_sys_info2_type_v01 gsm_sys_info2;

  /* Optional */
  /*  Additional WCDMA System Info */
  uint8_t wcdma_sys_info2_valid;  /**< Must be set to true if wcdma_sys_info2 is being passed */
  nas_wcdma_sys_info2_type_v01 wcdma_sys_info2;

  /* Optional */
  /*  Additional LTE System Info */
  uint8_t lte_sys_info2_valid;  /**< Must be set to true if lte_sys_info2 is being passed */
  nas_lte_sys_info2_type_v01 lte_sys_info2;

  /* Optional */
  /*  GSM Call Barring System Info */
  uint8_t gsm_sys_info3_valid;  /**< Must be set to true if gsm_sys_info3 is being passed */
  nas_gw_sys_info3_type_v01 gsm_sys_info3;

  /* Optional */
  /*  WCDMA Call Barring System Info */
  uint8_t wcdma_sys_info3_valid;  /**< Must be set to true if wcdma_sys_info3 is being passed */
  nas_gw_sys_info3_type_v01 wcdma_sys_info3;

  /* Optional */
  /*  LTE Voice Support Sys Info */
  uint8_t voice_support_on_lte_valid;  /**< Must be set to true if voice_support_on_lte is being passed */
  uint8_t voice_support_on_lte;
  /**<  
    Indicates voice support status on LTE. Values: \n
    - 0x00 -- Voice is not supported \n
    - 1x01 -- Voice is supported
   */

  /* Optional */
  /*  GSM Cipher Domain Sys Info */
  uint8_t gsm_cipher_domain_valid;  /**< Must be set to true if gsm_cipher_domain is being passed */
  nas_service_domain_enum_type_v01 gsm_cipher_domain;
  /**<  
    Ciphering on the service domain. Values: \n
    - 0x00 -- SYS_SRV_DOMAIN_NO_SRV  -- No service \n
    - 0x01 -- SYS_SRV_DOMAIN_CS_ONLY -- Circuit-switched only \n
    - 0x02 -- SYS_SRV_DOMAIN_PS_ONLY -- Packet-switched only \n
    - 0x03 -- SYS_SRV_DOMAIN_CS_PS   -- Circuit-switched and packet-switched
   */

  /* Optional */
  /*  WCDMA Cipher Domain Sys Info */
  uint8_t wcdma_cipher_domain_valid;  /**< Must be set to true if wcdma_cipher_domain is being passed */
  nas_service_domain_enum_type_v01 wcdma_cipher_domain;
  /**<  
    Ciphering on the service domain. Values: \n
    - 0x00 -- SYS_SRV_DOMAIN_NO_SRV  -- No service \n
    - 0x01 -- SYS_SRV_DOMAIN_CS_ONLY -- Circuit-switched only \n
    - 0x02 -- SYS_SRV_DOMAIN_PS_ONLY -- Packet-switched only \n
    - 0x03 -- SYS_SRV_DOMAIN_CS_PS   -- Circuit-switched and packet-switched
   */

  /* Optional */
  /*  System Info No Change */
  uint8_t sys_info_no_change_valid;  /**< Must be set to true if sys_info_no_change is being passed */
  uint8_t sys_info_no_change;
  /**<   
    Flag used to notify clients that a request to select a network ended 
    with no change in the PLMN. Values: \n
    - 0x01 -- No change in system information
  */

  /* Optional */
  /*  TDSCDMA Service Status Info */
  uint8_t tdscdma_srv_status_info_valid;  /**< Must be set to true if tdscdma_srv_status_info is being passed */
  nas_3gpp_srv_status_info_type_v01 tdscdma_srv_status_info;

  /* Optional */
  /*  TDSCDMA System Info */
  uint8_t tdscdma_sys_info_valid;  /**< Must be set to true if tdscdma_sys_info is being passed */
  nas_tdscdma_sys_info_type_v01 tdscdma_sys_info;

  /* Optional */
  /*  LTE eMBMS Coverage Info */
  uint8_t lte_embms_coverage_valid;  /**< Must be set to true if lte_embms_coverage is being passed */
  uint8_t lte_embms_coverage;
  /**<  
    Values: \n
    - TRUE  -- Current LTE system supports eMBMBS \n
    - FALSE -- Current LTE system does not support eMBMBS
 */

  /* Optional */
  /*  SIM Reject information */
  uint8_t sim_rej_info_valid;  /**< Must be set to true if sim_rej_info is being passed */
  nas_sim_rej_info_enum_type_v01 sim_rej_info;
  /**<  
    Current reject state information of the SIM. Values: \n
    - 0 -- NAS_SIM_NOT_AVAILABLE -- SIM is not available \n
    - 1 -- NAS_SIM_AVAILABLE     -- SIM is available     \n
    - 2 -- NAS_SIM_CS_INVALID    -- SIM has been marked by the network as 
                                    invalid for circuit-switched services \n
    - 3 -- NAS_SIM_PS_INVALID    -- SIM has been marked by the network as 
                                    invalid for packet-switched services  \n
    - 4 -- NAS_SIM_CS_PS_INVALID -- SIM has been marked by the network as 
                                    invalid for circuit-switched and 
                                    packet-switched services
  */

  /* Optional */
  /*  WCDMA EUTRA Status Information */
  uint8_t wcdma_eutra_status_valid;  /**< Must be set to true if wcdma_eutra_status is being passed */
  nas_eutra_cell_status_enum_type_v01 wcdma_eutra_status;
  /**<  
     E-UTRA detection status. Values: \n
    - 0 -- NAS_EUTRA_CELL_PRESENT           -- E-UTRA cell is detected            \n
    - 1 -- NAS_EUTRA_CELL_NOT_PRESENT       -- E-UTRA cell is not detected        \n
    - 2 -- NAS_EUTRA_CELL_PRESENCE_ UNKNOWN -- E-UTRA cell information is unknown 
                                               due to a state transition          \n
    - 3 -- NAS_EUTRA_CELL_DETECTION_ UNSUPPORTED -- E-UTRA detection is not supported
  */

  /* Optional */
  /*  IMS Voice Support Status on LTE */
  uint8_t lte_ims_voice_avail_valid;  /**< Must be set to true if lte_ims_voice_avail is being passed */
  uint8_t lte_ims_voice_avail;
  /**<  
    Values: \n
    - 0x00 -- Support is not available \n
    - 0x01 -- Support is available
  */

  /* Optional */
  /*  LTE Voice Domain */
  uint8_t lte_voice_status_valid;  /**< Must be set to true if lte_voice_status is being passed */
  nas_lte_voice_status_enum_type_v01 lte_voice_status;
  /**<  
    LTE voice domain. Values: \n
    - 0 -- NAS_DOMAIN_SEL_DOMAIN_NO_VOICE -- Data-centric devices: 
                                             No voice, stay on LTE \n
    - 1 -- NAS_DOMAIN_SEL_DOMAIN_IMS      -- Voice is supported over the IMS network \n
    - 2 -- NAS_DOMAIN_SEL_DOMAIN_1X       -- Voice is supported over the 1X network \n
    - 3 -- NAS_DOMAIN_SEL_DOMAIN_3GPP     -- Voice is supported over the 3GPP network 
  */

  /* Optional */
  /*  CDMA Reg Zone ID */
  uint8_t cdma_reg_zone_valid;  /**< Must be set to true if cdma_reg_zone is being passed */
  uint16_t cdma_reg_zone;
  /**<  
    CDMA registration zone ID.
  */

  /* Optional */
  /*  GSM RAC */
  uint8_t gsm_rac_valid;  /**< Must be set to true if gsm_rac is being passed */
  uint8_t gsm_rac;
  /**<  
    GSM routing area code.
  */

  /* Optional */
  /*  WCDMA RAC */
  uint8_t wcdma_rac_valid;  /**< Must be set to true if wcdma_rac is being passed */
  uint8_t wcdma_rac;
  /**<  
    WCDMA routing area code.
  */

  /* Optional */
  /*  CDMA Resolved Mobile Country Code */
  uint8_t cdma_mcc_resolved_via_sid_lookup_valid;  /**< Must be set to true if cdma_mcc_resolved_via_sid_lookup is being passed */
  uint16_t cdma_mcc_resolved_via_sid_lookup;
  /**<  
    MCC derived by looking up the IFAST SID conflict table and configured 
    SID-MCC table (static and NV) with the SID received from the network as the 
    key. If the lookup is not successful, 0xFFFF is used. \n
    Note: This MCC value is determined solely from the SID and may differ from 
    the MCC value sent by the network.

    (This field requires version 1.35 or later.)
  */

  /* Optional */
  /*  Network Selection Registration Restriction */
  uint8_t srv_reg_restriction_valid;  /**< Must be set to true if srv_reg_restriction is being passed */
  nas_srv_reg_restriction_enum_v01 srv_reg_restriction;
  /**<  
     Registration restriction. Values: \n
    - 0x00 -- NAS_SRV_REG_RESTRICTION_ UNRESTRICTED -- Device follows the normal 
              registration process \n
    - 0x01 -- NAS_SRV_REG_RESTRICTION_ CAMPED_ONLY -- Device follows the camp-only 
              registration process

    \vspace{3pt}
    All other values are reserved.

    (This field requires version 1.35 or later.)
  */

  /* Optional */
  /*  TDSCDMA Registration Domain */
  uint8_t tdscdma_reg_domain_valid;  /**< Must be set to true if tdscdma_reg_domain is being passed */
  nas_possible_reg_domain_enum_type_v01 tdscdma_reg_domain;
  /**<  
    TD-SCDMA registration domain. Values: \n
    - 0 -- NAS_POSSIBLE_REG_DOMAIN_NA        -- Not applicable because the UE 
           is not in Camp Only mode \n
    - 1 -- NAS_POSSIBLE_REG_DOMAIN_CS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide CS service only \n
    - 2 -- NAS_POSSIBLE_REG_DOMAIN_PS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide PS service only \n
    - 3 -- NAS_POSSIBLE_REG_DOMAIN_CS_PS     -- UE is in Camp Only mode and the 
           PLMN can provide CS and PS service \n
    - 4 -- NAS_POSSIBLE_REG_DOMAIN_ LIMITED_SERVICE -- UE is in Camp Only mode, but 
           the PLMN cannot provide any service

    \vspace{3pt}
   (This field requires version 1.35 or later.)
  */

  /* Optional */
  /*  LTE Registration Domain */
  uint8_t lte_reg_domain_valid;  /**< Must be set to true if lte_reg_domain is being passed */
  nas_possible_reg_domain_enum_type_v01 lte_reg_domain;
  /**<  
    LTE registration domain. Values: \n
    - 0 -- NAS_POSSIBLE_REG_DOMAIN_NA        -- Not applicable because the UE 
           is not in Camp Only mode \n
    - 1 -- NAS_POSSIBLE_REG_DOMAIN_CS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide CS service only \n
    - 2 -- NAS_POSSIBLE_REG_DOMAIN_PS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide PS service only \n
    - 3 -- NAS_POSSIBLE_REG_DOMAIN_CS_PS     -- UE is in Camp Only mode and the 
           PLMN can provide CS and PS service \n
    - 4 -- NAS_POSSIBLE_REG_DOMAIN_ LIMITED_SERVICE -- UE is in Camp Only mode, but 
           the PLMN cannot provide any service

    \vspace{3pt}
   (This field requires version 1.35 or later.)
  */

  /* Optional */
  /*  WCDMA Registration Domain */
  uint8_t wcdma_reg_domain_valid;  /**< Must be set to true if wcdma_reg_domain is being passed */
  nas_possible_reg_domain_enum_type_v01 wcdma_reg_domain;
  /**<  
    WCDMA registration domain. Values: \n
    - 0 -- NAS_POSSIBLE_REG_DOMAIN_NA        -- Not applicable because the UE 
           is not in Camp Only mode \n
    - 1 -- NAS_POSSIBLE_REG_DOMAIN_CS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide CS service only \n
    - 2 -- NAS_POSSIBLE_REG_DOMAIN_PS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide PS service only \n
    - 3 -- NAS_POSSIBLE_REG_DOMAIN_CS_PS     -- UE is in Camp Only mode and the 
           PLMN can provide CS and PS service \n
    - 4 -- NAS_POSSIBLE_REG_DOMAIN_ LIMITED_SERVICE -- UE is in Camp Only mode, but 
           the PLMN cannot provide any service

    \vspace{3pt}
   (This field requires version 1.35 or later.)
  */

  /* Optional */
  /*  GSM Registration Domain */
  uint8_t gsm_reg_domain_valid;  /**< Must be set to true if gsm_reg_domain is being passed */
  nas_possible_reg_domain_enum_type_v01 gsm_reg_domain;
  /**<  
    GSM registration domain. Values: \n
    - 0 -- NAS_POSSIBLE_REG_DOMAIN_NA        -- Not applicable because the UE 
           is not in Camp Only mode \n
    - 1 -- NAS_POSSIBLE_REG_DOMAIN_CS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide CS service only \n
    - 2 -- NAS_POSSIBLE_REG_DOMAIN_PS_ONLY   -- UE is in Camp Only mode and the 
           PLMN can provide PS service only \n
    - 3 -- NAS_POSSIBLE_REG_DOMAIN_CS_PS     -- UE is in Camp Only mode and the 
           PLMN can provide CS and PS service \n
    - 4 -- NAS_POSSIBLE_REG_DOMAIN_ LIMITED_SERVICE -- UE is in Camp Only mode, but 
           the PLMN cannot provide any service

    \vspace{3pt}
   (This field requires version 1.35 or later.)
  */

  /* Optional */
  /*  LTE eMBMS Coverage Info Trace ID */
  uint8_t lte_embms_coverage_trace_id_valid;  /**< Must be set to true if lte_embms_coverage_trace_id is being passed */
  int16_t lte_embms_coverage_trace_id;
  /**<   
    LTE eMBMS coverage information trace ID. Values: \n 
    - 0 to 32768 -- Valid trace ID \n
    - -1 -- Trace ID is not used  
  */

  /* Optional */
  /*  WCDMA CSG Information */
  uint8_t wcdma_csg_info_valid;  /**< Must be set to true if wcdma_csg_info is being passed */
  nas_csg_info_type_v01 wcdma_csg_info;

  /* Optional */
  /*  HDR Voice Domain */
  uint8_t hdr_voice_status_valid;  /**< Must be set to true if hdr_voice_status is being passed */
  nas_lte_voice_status_enum_type_v01 hdr_voice_status;
  /**<  
    HDR voice domain. Values: \n
    - 0 -- NAS_DOMAIN_SEL_DOMAIN_NO_VOICE -- Data-centric devices: 
                                             No voice, stay on HDR \n
    - 1 -- NAS_DOMAIN_SEL_DOMAIN_IMS      -- Voice is supported over the IMS network \n
    - 2 -- NAS_DOMAIN_SEL_DOMAIN_1X       -- Voice is supported over the 1X network
  */

  /* Optional */
  /*  HDR SMS Domain */
  uint8_t hdr_sms_status_valid;  /**< Must be set to true if hdr_sms_status is being passed */
  nas_sms_status_enum_type_v01 hdr_sms_status;
  /**<  
    HDR SMS domain. Values: \n
    - 0 -- NAS_SMS_STATUS_NO_SMS -- Data-centric devices: 
                                    No SMS, stay on HDR \n
    - 1 -- NAS_SMS_STATUS_IMS    -- SMS is supported over the IMS network \n
    - 2 -- NAS_SMS_STATUS_1X     -- SMS is supported over the 1X network
  */

  /* Optional */
  /*  LTE SMS Domain */
  uint8_t lte_sms_status_valid;  /**< Must be set to true if lte_sms_status is being passed */
  nas_sms_status_enum_type_v01 lte_sms_status;
  /**<  
    LTE SMS domain. Values: \n
    - 0 -- NAS_SMS_STATUS_NO_SMS -- Data-centric devices: 
                                    No SMS, stay on LTE \n
    - 1 -- NAS_SMS_STATUS_IMS    -- SMS is supported over the IMS network \n
    - 2 -- NAS_SMS_STATUS_1X     -- SMS is supported over the 1X network \n
    - 3 -- NAS_SMS_STATUS_3GPP   -- SMS is supported over the 3GPP network 
  */

  /* Optional */
  /*  LTE Emergency Bearer Support */
  uint8_t lte_is_eb_supported_valid;  /**< Must be set to true if lte_is_eb_supported is being passed */
  nas_tri_state_boolean_type_v01 lte_is_eb_supported;
  /**<  
 Whether LTE emergency bearer is supported. Values: \n
      - NAS_TRI_FALSE (0) --  Status: FALSE \n 
      - NAS_TRI_TRUE (1) --  Status: TRUE  \n 
      - NAS_TRI_UNKNOWN (2) --  Status: Unknown  

 \vspace{3pt}
 The TLV status is NAS_TRI_UNKNOWN for scenarios where information is not 
 available from the lower layers; e.g., if the UE powers up while acquiring 
 service or in the middle of an attach procedure.
 */

  /* Optional */
  /*  GSM Voice Domain */
  uint8_t gsm_voice_status_valid;  /**< Must be set to true if gsm_voice_status is being passed */
  nas_lte_voice_status_enum_type_v01 gsm_voice_status;
  /**<  
    GSM voice domain. Values: \n
    - 0 -- NAS_DOMAIN_SEL_DOMAIN_NO_VOICE -- Data-centric devices: 
                                             No voice, stay on GSM \n
    - 1 -- NAS_DOMAIN_SEL_DOMAIN_IMS      -- Voice is supported over the IMS network \n
    - 2 -- NAS_DOMAIN_SEL_DOMAIN_1X       -- Voice is supported over the 1X network
  */

  /* Optional */
  /*  GSM SMS Domain */
  uint8_t gsm_sms_status_valid;  /**< Must be set to true if gsm_sms_status is being passed */
  nas_sms_status_enum_type_v01 gsm_sms_status;
  /**<  
    GSM SMS domain. Values: \n
    - 0 -- NAS_SMS_STATUS_NO_SMS -- Data-centric devices: 
                                    No SMS, stay on GSM \n
    - 1 -- NAS_SMS_STATUS_IMS    -- SMS is supported over the IMS network \n
    - 2 -- NAS_SMS_STATUS_1X     -- SMS is supported over the 1X network
  */

  /* Optional */
  /*  WCDMA Voice Domain */
  uint8_t wcdma_voice_status_valid;  /**< Must be set to true if wcdma_voice_status is being passed */
  nas_lte_voice_status_enum_type_v01 wcdma_voice_status;
  /**<  
    WCDMA voice domain. Values: \n
    - 0 -- NAS_DOMAIN_SEL_DOMAIN_NO_VOICE -- Data-centric devices: 
                                             No voice, stay on WCDMA \n
    - 1 -- NAS_DOMAIN_SEL_DOMAIN_IMS      -- Voice is supported over the IMS network \n
    - 2 -- NAS_DOMAIN_SEL_DOMAIN_1X       -- Voice is supported over the 1X network
  */

  /* Optional */
  /*  WCDMA SMS Domain */
  uint8_t wcdma_sms_status_valid;  /**< Must be set to true if wcdma_sms_status is being passed */
  nas_sms_status_enum_type_v01 wcdma_sms_status;
  /**<  
    WCDMA SMS domain. Values: \n
    - 0 -- NAS_SMS_STATUS_NO_SMS -- Data-centric devices: 
                                    No SMS, stay on WCDMA \n
    - 1 -- NAS_SMS_STATUS_IMS    -- SMS is supported over the IMS network \n
    - 2 -- NAS_SMS_STATUS_1X     -- SMS is supported over the 1X network
  */

  /* Optional */
  /*  LTE Emergency Access Barred */
  uint8_t emergency_access_barred_valid;  /**< Must be set to true if emergency_access_barred is being passed */
  nas_tri_state_boolean_type_v01 emergency_access_barred;
  /**<  
 Whether LTE emergency access is barred on the current system. Values: \n
      - NAS_TRI_FALSE (0) --  Status: FALSE \n 
      - NAS_TRI_TRUE (1) --  Status: TRUE  \n 
      - NAS_TRI_UNKNOWN (2) --  Status: Unknown  

 \vspace{3pt}
 The TLV status is NAS_TRI_UNKNOWN for scenarios where information is not 
 available from the lower layers; e.g., if the UE powers up while acquiring 
 service or in the middle of an attach procedure.
 */

  /* Optional */
  /*  CDMA Voice Domain */
  uint8_t cdma_voice_status_valid;  /**< Must be set to true if cdma_voice_status is being passed */
  nas_lte_voice_status_enum_type_v01 cdma_voice_status;
  /**<  
    CDMA voice domain. Values: \n
    - 0 -- NAS_DOMAIN_SEL_DOMAIN_NO_VOICE -- Data-centric devices: 
                                             No voice, stay on CDMA \n
    - 1 -- NAS_DOMAIN_SEL_DOMAIN_IMS      -- Voice is supported over the IMS network \n
    - 2 -- NAS_DOMAIN_SEL_DOMAIN_1X       -- Voice is supported over the 1X network
  */

  /* Optional */
  /*  CDMA SMS Domain */
  uint8_t cdma_sms_status_valid;  /**< Must be set to true if cdma_sms_status is being passed */
  nas_sms_status_enum_type_v01 cdma_sms_status;
  /**<  
    CDMA SMS domain. Values: \n
    - 0 -- NAS_SMS_STATUS_NO_SMS -- Data-centric devices: 
                                    No SMS, stay on CDMA \n
    - 1 -- NAS_SMS_STATUS_IMS    -- SMS is supported over the IMS network \n
    - 2 -- NAS_SMS_STATUS_1X     -- SMS is supported over the 1X network
  */

  /* Optional */
  /*  TDSCDMA Voice Domain */
  uint8_t tdscdma_voice_status_valid;  /**< Must be set to true if tdscdma_voice_status is being passed */
  nas_lte_voice_status_enum_type_v01 tdscdma_voice_status;
  /**<  
    TD-SCDMA voice domain. Values: \n
    - 0 -- NAS_DOMAIN_SEL_DOMAIN_NO_VOICE -- Data-centric devices: 
                                             No voice, stay on TD-SCDMA \n
    - 1 -- NAS_DOMAIN_SEL_DOMAIN_IMS      -- Voice is supported over the IMS network \n
    - 2 -- NAS_DOMAIN_SEL_DOMAIN_1X       -- Voice is supported over the 1X network
  */

  /* Optional */
  /*  TDSCDMA SMS Domain */
  uint8_t tdscdma_sms_status_valid;  /**< Must be set to true if tdscdma_sms_status is being passed */
  nas_sms_status_enum_type_v01 tdscdma_sms_status;
  /**<  
    TD-SCDMA SMS domain. Values: \n
    - 0 -- NAS_SMS_STATUS_NO_SMS -- Data-centric devices: 
                                    No SMS, stay on TD-SCDMA \n
    - 1 -- NAS_SMS_STATUS_IMS    -- SMS is supported over the IMS network \n
    - 2 -- NAS_SMS_STATUS_1X     -- SMS is supported over the 1X network
  */

  /* Optional */
  /*  LTE CSG Information */
  uint8_t lte_csg_info_valid;  /**< Must be set to true if lte_csg_info is being passed */
  nas_csg_info_type_v01 lte_csg_info;

  /* Optional */
  /*  LTE Cell Access Status Info */
  uint8_t lte_cell_status_valid;  /**< Must be set to true if lte_cell_status is being passed */
  nas_cell_access_status_e_type_v01 lte_cell_status;
  /**<  
 Cell access status for LTE calls. Values: \n
      - NAS_CELL_ACCESS_NORMAL_ONLY (0x00) --  Cell access is allowed for normal calls only \n  
      - NAS_CELL_ACCESS_EMERGENCY_ONLY (0x01) --  Cell access is allowed for emergency calls only \n  
      - NAS_CELL_ACCESS_NO_CALLS (0x02) --  Cell access is not allowed for any call type \n  
      - NAS_CELL_ACCESS_ALL_CALLS (0x03) --  Cell access is allowed for all call types \n  
      - NAS_CELL_ACCESS_UNKNOWN (-1) --  Cell access type is unknown  
 */

  /* Optional */
  /*  HDR Subnet Mask Length */
  uint8_t hdr_subnet_mask_len_valid;  /**< Must be set to true if hdr_subnet_mask_len is being passed */
  uint8_t hdr_subnet_mask_len;
  /**<  
      HDR Subnet Mask Length.
  */
}nas_sys_info_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_sig_info_req_msg_v01;

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  RSSI */
  int8_t rssi;
  /**<  
      RSSI in dBm (signed value); a value of -125 dBm or lower is
      used to indicate No Signal: \n 
      - For CDMA, this indicates forward link pilot Power (AGC) + Ec/Io \n
      - For UMTS, this indicates forward link pilot Ec   \n
      - For GSM, this indicates received signal strength
  */

  /*  ECIO */
  int16_t ecio;
  /**<  
      ECIO value representing negative 0.5 dB increments, i.e., 
      2 means -1 dB (14 means -7 dB, 63 means -31.5 dB).
  */
}nas_common_sig_info_param_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_common_sig_info_param_type_v01 common_sig_str;

  /*  SINR */
  nas_sinr_enum_v01 sinr;
  /**<   
     SINR level. SINR is only applicable for 1xEV-DO. 
     Valid levels are 0 to 8, where the maximum value for:        \n
     - 0x00 -- SINR_LEVEL_0 is -9 dB     \n
     - 0x01 -- SINR_LEVEL_1 is -6 dB     \n
     - 0x02 -- SINR_LEVEL_2 is -4.5 dB   \n
     - 0x03 -- SINR_LEVEL_3 is -3 dB     \n
     - 0x04 -- SINR_LEVEL_4 is -2 dB     \n
     - 0x05 -- SINR_LEVEL_5 is +1 dB     \n
     - 0x06 -- SINR_LEVEL_6 is +3 dB     \n
     - 0x07 -- SINR_LEVEL_7 is +6 dB     \n
     - 0x08 -- SINR_LEVEL_8 is +9 dB
 */

  /*  IO */
  int32_t io;
  /**<  
     Received IO in dBm. IO is only applicable for 1xEV-DO. 
 */
}nas_hdr_sig_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  RSSI */
  int8_t rssi;
  /**<  
     RSSI in dBm (signed value); a value of -125 dBm or lower is
     used to indicate No Signal: \n 
     - For CDMA and UMTS, this indicates forward link pilot Ec   \n
     - For GSM, this indicates received signal strength
 */

  /*  RSRQ */
  int8_t rsrq;
  /**<  
     RSRQ value in dB (signed integer value) as measured by L1. 
     Range: -3 to -20 (-3 means -3 dB, -20 means -20 dB). 
 */

  /*  RSRP */
  int16_t rsrp;
  /**<  
     Current RSRP in dBm as measured by L1. 
     Range: -44 to -140 (-44 means -44 dBm, -140 means -140 dBm).
 */

  /*  SNR */
  int16_t snr;
  /**<   
     SNR level as a scaled integer in units of 0.1 dB; 
     e.g., -16 dB has a value of -160 and 24.6 dB has a value of 246.
 */
}nas_lte_sig_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  RSSI */
  float rssi;
  /**<  
      Measured RSSI in dBm. 
  */

  /*  RSCP */
  float rscp;
  /**<  
      Measured RSCP in dBm.  
  */

  /*  ECIO */
  float ecio;
  /**<  
      Measured ECIO in dB.
  */

  /*  SINR */
  float sinr;
  /**<   
      Measured SINR in dB. -15 dB is sent to clients if the actual SINR is 
      less than -15 dB.
  */
}nas_tdscdma_sig_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries information regarding the signal strength.
               \label{idl:getSigInfo} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  CDMA Signal Strength Info */
  uint8_t cdma_sig_info_valid;  /**< Must be set to true if cdma_sig_info is being passed */
  nas_common_sig_info_param_type_v01 cdma_sig_info;

  /* Optional */
  /*  HDR Signal Strength Info */
  uint8_t hdr_sig_info_valid;  /**< Must be set to true if hdr_sig_info is being passed */
  nas_hdr_sig_info_type_v01 hdr_sig_info;

  /* Optional */
  /*  GSM Signal Strength Info */
  uint8_t gsm_sig_info_valid;  /**< Must be set to true if gsm_sig_info is being passed */
  int8_t gsm_sig_info;
  /**<  
    GSM signal strength is the RSSI in dBm (signed value). 
    A value of -125 dBm or lower is used to indicate No Signal.
  */

  /* Optional */
  /*  WCDMA Signal Strength Info */
  uint8_t wcdma_sig_info_valid;  /**< Must be set to true if wcdma_sig_info is being passed */
  nas_common_sig_info_param_type_v01 wcdma_sig_info;

  /* Optional */
  /*  LTE Signal Strength Info */
  uint8_t lte_sig_info_valid;  /**< Must be set to true if lte_sig_info is being passed */
  nas_lte_sig_info_type_v01 lte_sig_info;

  /* Optional */
  /*  TDSCDMA Signal Strength Info */
  uint8_t rscp_valid;  /**< Must be set to true if rscp is being passed */
  int8_t rscp;
  /**<  
    RSCP of the Primary Common Control Physical Channel (PCCPCH) in dBm. 
    Measurement range: -120 dBm to -25 dBm. 
  */

  /* Optional */
  /*  TDSCDMA Signal Strength Info Extended */
  uint8_t tdscdma_sig_info_valid;  /**< Must be set to true if tdscdma_sig_info is being passed */
  nas_tdscdma_sig_info_type_v01 tdscdma_sig_info;
}nas_get_sig_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_LTE_SIG_RPT_RATE_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_LTE_SIG_RPT_RATE_DEFAULT_V01 = 0, 
  NAS_LTE_SIG_RPT_RATE_1_SEC_V01 = 1, 
  NAS_LTE_SIG_RPT_RATE_2_SEC_V01 = 2, 
  NAS_LTE_SIG_RPT_RATE_3_SEC_V01 = 3, 
  NAS_LTE_SIG_RPT_RATE_4_SEC_V01 = 4, 
  NAS_LTE_SIG_RPT_RATE_5_SEC_V01 = 5, 
  NAS_LTE_SIG_RPT_RATE_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_lte_sig_rpt_rate_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_LTE_SIG_AVG_PRD_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_LTE_SIG_AVG_PRD_DEFAULT_V01 = 0, 
  NAS_LTE_SIG_AVG_PRD_1_SEC_V01 = 1, 
  NAS_LTE_SIG_AVG_PRD_2_SEC_V01 = 2, 
  NAS_LTE_SIG_AVG_PRD_3_SEC_V01 = 3, 
  NAS_LTE_SIG_AVG_PRD_4_SEC_V01 = 4, 
  NAS_LTE_SIG_AVG_PRD_5_SEC_V01 = 5, 
  NAS_LTE_SIG_AVG_PRD_6_SEC_V01 = 6, 
  NAS_LTE_SIG_AVG_PRD_7_SEC_V01 = 7, 
  NAS_LTE_SIG_AVG_PRD_8_SEC_V01 = 8, 
  NAS_LTE_SIG_AVG_PRD_9_SEC_V01 = 9, 
  NAS_LTE_SIG_AVG_PRD_10_SEC_V01 = 10, 
  NAS_LTE_SIG_AVG_PRD_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_lte_sig_avg_prd_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Report rate */
  nas_lte_sig_rpt_rate_enum_type_v01 rpt_rate;
  /**<  
      Rate on how often the LTE signal must be checked for reporting. Values: \n
      - 0 -- Report using the default configuration \n
      - 1 -- Report every 1 sec \n
      - 2 -- Report every 2 sec \n
      - 3 -- Report every 3 sec \n
      - 4 -- Report every 4 sec \n
      - 5 -- Report every 5 sec 
   */

  /*  Averaging period */
  nas_lte_sig_avg_prd_enum_type_v01 avg_period;
  /**<  
      Averaging period to be used for the LTE signal. Values: \n
      - 0  -- Average using the default configuration \n
      - 1  -- Average over 1 sec \n
      - 2  -- Average over 2 sec \n
      - 3  -- Average over 3 sec \n
      - 4  -- Average over 4 sec \n
      - 5  -- Average over 5 sec \n
      - 6  -- Average over 6 sec \n
      - 7  -- Average over 7 sec \n
      - 8  -- Average over 8 sec \n
      - 9  -- Average over 9 sec \n
      - 10 -- Average over 10 sec
   */
}nas_lte_sig_rpt_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets the signal strength reporting thresholds. (Deprecated) */
typedef struct {

  /* Optional */
  /*  RSSI Threshold List  */
  uint8_t rssi_threshold_list_valid;  /**< Must be set to true if rssi_threshold_list is being passed */
  uint32_t rssi_threshold_list_len;  /**< Must be set to # of elements in rssi_threshold_list */
  int8_t rssi_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX_V01];
  /**<  
      RSSI in 1 dBm. A value of -125 dBm or lower is used to indicate 
      No Signal. RSSI values have the following ranges (in dBm): \n
      - CDMA:  -105 to -21 \n
      - HDR:   -118 to -13 \n
      - GSM:   -111 to -48 \n
      - WCDMA: -121 to 0   \n
      - LTE:   -120 to 0   \n
      The threshold values specified here are used for all RATs. The 
      maximum number of threshold values is 16, each a signed byte 
      value.                                                      \n \vspace{-.12in}

      For CDMA and UMTS, this threshold setting results in the 
      forward link pilot Ec values to be reported as part of the rssi
      field in TLV corresponding to the RAT in the QMI_NAS_SIG_INFO_IND
      indication.                                                 \n \vspace{-.12in}

      For GSM, this threshold setting results in the received 
      signal strength to be reported as part of the GSM Signal Strength Info 
      TLV in the QMI_NAS_SIG_INFO_IND indication.                 \n \vspace{-.12in}

      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  ECIO Threshold List */
  uint8_t ecio_threshold_list_valid;  /**< Must be set to true if ecio_threshold_list is being passed */
  uint32_t ecio_threshold_list_len;  /**< Must be set to # of elements in ecio_threshold_list */
  int16_t ecio_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX_V01];
  /**<  
      A sequence of thresholds delimiting ECIO event reporting bands.
      Every time a new ECIO value crosses a threshold value, an event
      report indication message with the new ECIO value is sent to the
      requesting control point. For this field: \n

      - Each ECIO threshold value is a signed 2 byte value \n
      - Each ECIO threshold value increments in negative 0.5 dB, 
        e.g., an ECIO threshold value of 2 means -1 dB. \n
      - Maximum number of threshold values is 16        \n
      - At least one value must be specified (if report_ecio is set) \n
      - Threshold values specified here are used for all RATs
  */

  /* Optional */
  /*  HDR SINR Threshold List */
  uint8_t hdr_sinr_threshold_list_valid;  /**< Must be set to true if hdr_sinr_threshold_list is being passed */
  uint32_t hdr_sinr_threshold_list_len;  /**< Must be set to # of elements in hdr_sinr_threshold_list */
  uint8_t hdr_sinr_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX_V01];
  /**<  
      A sequence of thresholds delimiting SINR event reporting bands.
      Every time a new SINR value crosses a threshold value, an event
      report indication message with the new SINR value is sent to the
      requesting control point. For this field: \n
      
      - SINR is reported only for HDR \n
      - Each SINR threshold value is an unsigned 1 byte value \n
      - Maximum number of threshold values is 16              \n
      - At least one value must be specified (if report_sinr is set)
   */

  /* Optional */
  /*  LTE SNR Threshold List */
  uint8_t lte_snr_threshold_list_valid;  /**< Must be set to true if lte_snr_threshold_list is being passed */
  uint32_t lte_snr_threshold_list_len;  /**< Must be set to # of elements in lte_snr_threshold_list */
  int16_t lte_snr_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX_V01];
  /**<  
      A sequence of thresholds delimiting SNR event reporting bands.
      Every time a new SNR value crosses a threshold value, an event
      report indication message with the new snr value is sent to the
      requesting control point. For this field: \n

      - For LTE, each SNR threshold value is a signed 2 byte value  \n
      - Maximum number of threshold values is 16                    \n 
      - At least one value must be specified (if report_snr is set) \n
      - SNR level as a scaled integer in units of 0.1 dB; 
        e.g., -16 dB has a value of -160 and 24.6 dB has a value of 246
  
   */

  /* Optional */
  /*  IO Threshold List */
  uint8_t io_threshold_list_valid;  /**< Must be set to true if io_threshold_list is being passed */
  uint32_t io_threshold_list_len;  /**< Must be set to # of elements in io_threshold_list */
  int32_t io_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX_V01];
  /**<  
      A sequence of thresholds delimiting IO event reporting bands.
      Every time a new IO value crosses a threshold value, an event
      report indication message with the new IO value is sent to the
      requesting control point. For this field: \n
      
      - IO is applicable only for HDR \n
      - Each IO threshold value is a signed 4 byte value \n
      - Maximum number of threshold values is 16         \n
      - At least one value must be specified
 */

  /* Optional */
  /*  RSRQ Threshold List */
  uint8_t lte_rsrq_threshold_list_valid;  /**< Must be set to true if lte_rsrq_threshold_list is being passed */
  uint32_t lte_rsrq_threshold_list_len;  /**< Must be set to # of elements in lte_rsrq_threshold_list */
  int8_t lte_rsrq_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX_V01];
  /**<  
      A sequence of thresholds delimiting current RSRQ event reporting bands.
      Every time a new RSRQ value crosses a specified threshold value, an
      event report indication message with the new RSRQ value is sent
      to the requesting control point. For this field: \n

      - RSRQ values are applicable only for LTE \n
      - RSRQ values are measured in dBm, with a range of -20 dBm to -3 dBm \n
      - Each RSRQ threshold value is a signed byte value \n
      - Maximum number of threshold values is 16         \n
      - At least one value must be specified
 */

  /* Optional */
  /*  RSRP Threshold List */
  uint8_t lte_rsrp_threshold_list_valid;  /**< Must be set to true if lte_rsrp_threshold_list is being passed */
  uint32_t lte_rsrp_threshold_list_len;  /**< Must be set to # of elements in lte_rsrp_threshold_list */
  int16_t lte_rsrp_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX_V01];
  /**<  
      A sequence of thresholds delimiting current RSRP event reporting bands.
      Every time a new RSRP value crosses a specified threshold value, an
      event report indication message with the new RSRP value is sent
      to the requesting control point. For this field: \n

      - RSRP values are applicable only for LTE \n
      - RSRP values are measured in dBm, with a range of -44 dBm to -140 dBm \n
      - Each RSRP threshold value is a signed 2 byte value \n
      - Maximum number of threshold values is 16           \n
      - At least one value must be specified
    */

  /* Optional */
  /*  LTE Signal Report Config */
  uint8_t lte_sig_rpt_config_valid;  /**< Must be set to true if lte_sig_rpt_config is being passed */
  nas_lte_sig_rpt_config_type_v01 lte_sig_rpt_config;

  /* Optional */
  /*  RSCP Threshold List  */
  uint8_t rscp_threshold_list_valid;  /**< Must be set to true if rscp_threshold_list is being passed */
  uint32_t rscp_threshold_list_len;  /**< Must be set to # of elements in rscp_threshold_list */
  int8_t rscp_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX_V01];
  /**<  
      RSCP in 1 dBm. The threshold values specified here are used for all RATs.
   */

  /* Optional */
  /*  TDSCDMA SINR Threshold List  */
  uint8_t tds_sinr_threshold_list_valid;  /**< Must be set to true if tds_sinr_threshold_list is being passed */
  uint32_t tds_sinr_threshold_list_len;  /**< Must be set to # of elements in tds_sinr_threshold_list */
  float tds_sinr_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX_V01];
  /**<  
      Array of SINR thresholds (in dB) used by TD-SCDMA; maximum of 16 values.
  */
}nas_config_sig_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets the signal strength reporting thresholds. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_config_sig_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Provides any change in signal strength status.
             \label{idl:sigInfoInd} */
typedef struct {

  /* Optional */
  /*  CDMA Signal Strength Info */
  uint8_t cdma_sig_info_valid;  /**< Must be set to true if cdma_sig_info is being passed */
  nas_common_sig_info_param_type_v01 cdma_sig_info;

  /* Optional */
  /*  HDR Signal Strength Info */
  uint8_t hdr_sig_info_valid;  /**< Must be set to true if hdr_sig_info is being passed */
  nas_hdr_sig_info_type_v01 hdr_sig_info;

  /* Optional */
  /*  GSM Signal Strength Info */
  uint8_t gsm_sig_info_valid;  /**< Must be set to true if gsm_sig_info is being passed */
  int8_t gsm_sig_info;
  /**<  
    GSM signal strength is the RSSI in dBm (signed value). 
    A value of -125 dBm or lower is used to indicate No Signal.
    */

  /* Optional */
  /*  WCDMA Signal Strength Info */
  uint8_t wcdma_sig_info_valid;  /**< Must be set to true if wcdma_sig_info is being passed */
  nas_common_sig_info_param_type_v01 wcdma_sig_info;

  /* Optional */
  /*  LTE Signal Strength Info */
  uint8_t lte_sig_info_valid;  /**< Must be set to true if lte_sig_info is being passed */
  nas_lte_sig_info_type_v01 lte_sig_info;

  /* Optional */
  /*  TDSCDMA Signal Strength Info */
  uint8_t rscp_valid;  /**< Must be set to true if rscp is being passed */
  int8_t rscp;
  /**<  
    RSCP of the PCCPCH in dBm. 
    Measurement range: -120 dBm to -25 dBm. 
  */

  /* Optional */
  /*  TDSCDMA Signal Strength Info Extended */
  uint8_t tdscdma_sig_info_valid;  /**< Must be set to true if tdscdma_sig_info is being passed */
  nas_tdscdma_sig_info_type_v01 tdscdma_sig_info;
}nas_sig_info_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_err_rate_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the current error rate information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  CDMA Frame Error Rate */
  uint8_t cdma_frame_err_rate_valid;  /**< Must be set to true if cdma_frame_err_rate is being passed */
  uint16_t cdma_frame_err_rate;
  /**<  
      Valid error rate values between 1 and 10000 are returned to 
      indicate the percentage, e.g., a value of 300 means the error rate is 3%. 
      A value of 0xFFFF indicates that the error rate is unknown/unavailable.
  */

  /* Optional */
  /*  HDR Packet Error Rate */
  uint8_t hdr_packet_err_rate_valid;  /**< Must be set to true if hdr_packet_err_rate is being passed */
  uint16_t hdr_packet_err_rate;
  /**<  
      Valid error rate values between 1 and 10000 are returned to 
      indicate the percentage, e.g., a value of 300 means the error rate is 3%. 
      A value of 0xFFFF indicates that the error rate is unknown/unavailable. 
  */

  /* Optional */
  /*  GSM Bit Error Rate */
  uint8_t gsm_bit_err_rate_valid;  /**< Must be set to true if gsm_bit_err_rate is being passed */
  uint8_t gsm_bit_err_rate;
  /**<  
      GSM bit error rate represented as an RxQual metric as defined in 
      \hyperref[S13]{[S13]} Section 8.2.4. Valid values: 0 to 7. 
      A value of 0xFF indicates No Data.
  */

  /* Optional */
  /*  WCDMA Block Error Rate */
  uint8_t wcdma_block_err_rate_valid;  /**< Must be set to true if wcdma_block_err_rate is being passed */
  uint8_t wcdma_block_err_rate;
  /**<  
      Valid error rate values between 1 and 100 are returned to 
      indicate the percentage value. A value of 0xFF indicates
      that the error rate is unknown/unavailable.
  */

  /* Optional */
  /*  TDSCDMA Block Error Rate */
  uint8_t tdscdma_block_err_rate_valid;  /**< Must be set to true if tdscdma_block_err_rate is being passed */
  uint8_t tdscdma_block_err_rate;
  /**<  
      Percentage of blocks that had errors. A value of 0xFF indicates 
      that the error rate is unknown/unavailable.
  */
}nas_get_err_rate_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Provides RAT-specific error rate information.
             \label{idl:errRateInd} */
typedef struct {

  /* Optional */
  /*  CDMA Frame Error Rate */
  uint8_t cdma_frame_err_rate_valid;  /**< Must be set to true if cdma_frame_err_rate is being passed */
  uint16_t cdma_frame_err_rate;
  /**<  
      Valid error rate values between 1 and 10000 are returned to 
      indicate the percentage, e.g., a value of 300 means the error rate is 3%. 
      A value of 0xFFFF indicates that the error rate is unknown/unavailable.
  */

  /* Optional */
  /*  HDR Packet Error Rate */
  uint8_t hdr_packet_err_rate_valid;  /**< Must be set to true if hdr_packet_err_rate is being passed */
  uint16_t hdr_packet_err_rate;
  /**<  
      Valid error rate values between 1 and 10000 are returned to 
      indicate the percentage, e.g., a value of 300 means the error rate is 3%. 
      A value of 0xFFFF indicates that the error rate is unknown/unavailable.
  */

  /* Optional */
  /*  GSM Bit Error Rate */
  uint8_t gsm_bit_err_rate_valid;  /**< Must be set to true if gsm_bit_err_rate is being passed */
  uint8_t gsm_bit_err_rate;
  /**<  
      GSM bit error rate represented as an RxQual metric as defined in 
      \hyperref[S13]{[S13]} Section 8.2.4. Valid values: 0 to 7. 
      A value of 0xFF indicates No Data.
  */

  /* Optional */
  /*  WCDMA Block Error Rate */
  uint8_t wcdma_block_err_rate_valid;  /**< Must be set to true if wcdma_block_err_rate is being passed */
  uint8_t wcdma_block_err_rate;
  /**<  
      Valid error rate values between 1 and 100 are returned to 
      indicate the percentage value. A value of 0xFF indicates
      that the error rate is unknown/unavailable.
  */

  /* Optional */
  /*  TDSCDMA Block Error Rate */
  uint8_t tdscdma_block_err_rate_valid;  /**< Must be set to true if tdscdma_block_err_rate is being passed */
  uint8_t tdscdma_block_err_rate;
  /**<  
      Percentage of blocks that had errors. A value of 0xFF indicates 
      that the error rate is unknown/unavailable.
  */
}nas_err_rate_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_HDR_SESSION_CLOSE_REASON_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_HDR_CLOSE_REASON_NEW_NETWORK_V01 = 0x00, 
  NAS_HDR_CLOSE_REASON_UATI_FAIL_V01 = 0x01, 
  NAS_HDR_CLOSE_REASON_KA_EXP_V01 = 0x02, 
  NAS_HDR_CLOSE_REASON_DEACTIVATE_V01 = 0x03, 
  NAS_HDR_CLOSE_REASON_REPLY_V01 = 0x04, 
  NAS_HDR_CLOSE_REASON_CONN_OPEN_FAIL_V01 = 0x05, 
  NAS_HDR_CLOSE_REASON_CFG_MSG_FAIL_V01 = 0x06, 
  NAS_HDR_CLOSE_REASON_CFG_RSP_EXP_V01 = 0x07, 
  NAS_HDR_CLOSE_REASON_PROT_NEG_FAIL_V01 = 0x08, 
  NAS_HDR_CLOSE_REASON_AN_INIT_EXP_V01 = 0x09, 
  NAS_HDR_CLOSE_REASON_QUICK_FAILURE_V01 = 0x0A, 
  NAS_HDR_CLOSE_REASON_CONN_OPEN_DENY_V01 = 0x0B, 
  NAS_HDR_CLOSE_REASON_SILENT_DEACTIVATE_V01 = 0x0C, 
  NAS_HDR_CLOSE_REASON_NEW_ESN_V01 = 0x0D, 
  NAS_HDR_CLOSE_REASON_AN_GAUP_FAIL_V01 = 0x0E, 
  NAS_HDR_CLOSE_REASON_PERSONALITY_INDEX_INVALID_V01 = 0x0F, 
  NAS_HDR_CLOSE_REASON_NOT_MAINT_UATI_V01 = 0x10, 
  NAS_HDR_CLOSE_REASON_NEW_NAI_V01 = 0x11, 
  NAS_HDR_CLOSE_REASON_EHRPD_CREDENTIALS_CHANGED_V01 = 0x12, 
  NAS_HDR_SESSION_CLOSE_REASON_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_hdr_session_close_reason_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Indicates when an HDR session has closed and returns a 
               close reason. */
typedef struct {

  /* Mandatory */
  /*  HDR Session Close Reason */
  nas_hdr_session_close_reason_type_v01 close_reason;
  /**<   HDR session close reason (see Table @latexonly\ref{tbl:closeReason}@endlatexonly 
       for details). */
}nas_hdr_session_close_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Indicates when an HDR unique access terminal identifier has been 
             updated and returns its new value. */
typedef struct {

  /* Mandatory */
  /*  HDR UATI */
  uint8_t uati[QMI_NAS_UATI_LENGTH_V01];
  /**<  
      A 128-bit address that includes the access terminal identifier 
      and subnet ID.
    */
}nas_hdr_uati_update_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Retrieves the current HDR protocol subtype. */
typedef struct {

  /* Mandatory */
  /*  Protocol */
  uint32_t protocol;
  /**<   HDR protocol for which the subtype is requested (refer to 
       \hyperref[S15]{[S15]} Table 2.5.4-1).
  */
}nas_get_hdr_subtype_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the current HDR protocol subtype. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Protocol Subtype */
  uint8_t subtype_valid;  /**< Must be set to true if subtype is being passed */
  uint16_t subtype;
  /**<   Current HDR protocol subtype (refer to 
       \hyperref[S15]{[S15]} Table 6.4.7.1-1). Values: \n
       - 0x0000 -- Default \n
       - 0x0000 to 0XFFFD -- Protocol subtypes \n
       - 0xFFFE -- Hardlink \n
       - 0xFFFF -- Indicates that the input protocol ID is not valid
   */
}nas_get_hdr_subtype_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_hdr_color_code_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the HDR color code value. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Color Code Value */
  uint8_t color_code_valid;  /**< Must be set to true if color_code is being passed */
  uint8_t color_code;
  /**<   Color code corresponding to the sector to which the AT is sending the 
       access probe (refer to \hyperref[S15]{[S15]} Section 7.11.6.2.1).
  */
}nas_get_hdr_color_code_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_current_acq_sys_mode_req_msg_v01;

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SYS_MODE_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SYS_MODE_NO_SERVICE_V01 = 0x00, 
  NAS_SYS_MODE_ACQUIRING_V01 = 0x01, 
  NAS_SYS_MODE_INSERVICE_V01 = 0x02, 
  NAS_SYS_MODE_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_sys_mode_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the current acquisition system mode. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Current System Mode for CDMA 1X */
  uint8_t cdma_valid;  /**< Must be set to true if cdma is being passed */
  nas_sys_mode_type_v01 cdma;
  /**<   Radio interface system mode. Values: \n
       - 0x00 -- NAS_SYS_MODE_NO_SERVICE -- No service \n
       - 0x01 -- NAS_SYS_MODE_ACQUIRING  -- Acquiring service \n
       - 0x02 -- NAS_SYS_MODE_INSERVICE  -- In service
  */

  /* Optional */
  /*  Current System Mode for CDMA 1xEV-DO */
  uint8_t cdma_evdo_valid;  /**< Must be set to true if cdma_evdo is being passed */
  nas_sys_mode_type_v01 cdma_evdo;
  /**<   Radio interface system mode. Values: \n
       - 0x00 -- NAS_SYS_MODE_NO_SERVICE -- No service \n
       - 0x01 -- NAS_SYS_MODE_ACQUIRING  -- Acquiring service \n
       - 0x02 -- NAS_SYS_MODE_INSERVICE  -- In service
  */

  /* Optional */
  /*  Current System Mode for GSM */
  uint8_t gsm_valid;  /**< Must be set to true if gsm is being passed */
  nas_sys_mode_type_v01 gsm;
  /**<   Radio interface system mode. Values: \n
       - 0x00 -- NAS_SYS_MODE_NO_SERVICE -- No service \n
       - 0x01 -- NAS_SYS_MODE_ACQUIRING  -- Acquiring service \n
       - 0x02 -- NAS_SYS_MODE_INSERVICE  -- In service
  */

  /* Optional */
  /*  Current System Mode for UMTS */
  uint8_t umts_valid;  /**< Must be set to true if umts is being passed */
  nas_sys_mode_type_v01 umts;
  /**<   Radio interface system mode. Values: \n
       - 0x00 -- NAS_SYS_MODE_NO_SERVICE -- No service \n
       - 0x01 -- NAS_SYS_MODE_ACQUIRING  -- Acquiring service \n
       - 0x02 -- NAS_SYS_MODE_INSERVICE  -- In service
  */

  /* Optional */
  /*  Current System Mode for LTE */
  uint8_t lte_valid;  /**< Must be set to true if lte is being passed */
  nas_sys_mode_type_v01 lte;
  /**<   Radio interface system mode. Values: \n
       - 0x00 -- NAS_SYS_MODE_NO_SERVICE -- No service \n
       - 0x01 -- NAS_SYS_MODE_ACQUIRING  -- Acquiring service \n
       - 0x02 -- NAS_SYS_MODE_INSERVICE  -- In service
  */

  /* Optional */
  /*  Current System Mode for TDSCDMA */
  uint8_t tdscdma_valid;  /**< Must be set to true if tdscdma is being passed */
  nas_sys_mode_type_v01 tdscdma;
  /**<   Radio interface system mode. Values: \n
       - 0x00 -- NAS_SYS_MODE_NO_SERVICE -- No service \n
       - 0x01 -- NAS_SYS_MODE_ACQUIRING  -- Acquiring service \n
       - 0x02 -- NAS_SYS_MODE_INSERVICE  -- In service
  */
}nas_get_current_acq_sys_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Radio interface */
  nas_radio_if_enum_v01 radio_if;
  /**<   Radio interface for which to set the Rx diversity. Values: \n
    - 0x01 -- NAS_RADIO_IF_CDMA_1X     -- 
      cdma2000\textsuperscript{\textregistered} 1X             \n
    - 0x02 -- NAS_RADIO_IF_CDMA_1XEVDO -- 
      cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
    - 0x04 -- NAS_RADIO_IF_GSM         -- GSM \n
    - 0x05 -- NAS_RADIO_IF_UMTS        -- UMTS \n
    - 0x08 -- NAS_RADIO_IF_LTE         -- LTE
  */

  /*  Rx chain setting bitmask */
  uint8_t rx_chain_bitmask;
  /**<   Rx chain setting bitmask. Values: \n
    - Bit 0 -- Rx chain 0 setting; 0 is disable, 1 is enable \n
    - Bit 1 -- Rx chain 1 setting; 0 is disable, 1 is enable \n
    - All other bits are set to zero
   */
}nas_set_rx_diversity_req_param_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets the Rx diversity. */
typedef struct {

  /* Mandatory */
  /*  Rx Diversity Setting */
  nas_set_rx_diversity_req_param_type_v01 req_param;
}nas_set_rx_diversity_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets the Rx diversity. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_set_rx_diversity_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Retrieves the detailed Tx/Rx information. */
typedef struct {

  /* Mandatory */
  /*  Radio Interface */
  nas_radio_if_enum_v01 radio_if;
  /**<  
    Radio interface from which to get the information. Values: \n
    - 0x01 -- NAS_RADIO_IF_CDMA_1X     -- 
      cdma2000\textsuperscript{\textregistered} 1X             \n
    - 0x02 -- NAS_RADIO_IF_CDMA_1XEVDO -- 
      cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
    - 0x04 -- NAS_RADIO_IF_GSM         -- GSM \n
    - 0x05 -- NAS_RADIO_IF_UMTS        -- UMTS \n
    - 0x08 -- NAS_RADIO_IF_LTE         -- LTE
  */
}nas_get_tx_rx_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t is_radio_tuned;
  /**<   Whether Rx is tuned to a channel: \n
       - 0x00 -- Not tuned \n
       - 0x01 -- Tuned \n
       If the radio is tuned, instantaneous values are set for the signal 
       information fields below. If the radio is not tuned, or is delayed or 
       invalid, the values are set depending on each technology.
   */

  int32_t rx_pwr;
  /**<   Rx power value in 1/10 dbm resolution. */

  int32_t ecio;
  /**<   ECIO in 1/10 dB; valid for CDMA, HDR, GSM, WCDMA, and LTE. */

  int32_t rscp;
  /**<   Received signal code power in 1/10 dbm; valid for WCDMA. */

  int32_t rsrp;
  /**<   Current reference signal received power in 1/10 dbm; valid for LTE. */

  uint32_t phase;
  /**<   Phase in 1/100 degrees; valid for LTE. When the phase is unknown, 
       0xFFFFFFFF is used. 
  */
}nas_rx_chain_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t is_in_traffic;
  /**<   Whether the device is in traffic. The tx_pwr field is only 
       meaningful when in the device is in traffic. If it is not in traffic, 
       tx_pwr is invalid.
   */

  int32_t tx_pwr;
  /**<   Tx power value in 1/10 dbm. */
}nas_tx_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the detailed Tx/Rx information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Rx Chain 0 Info */
  uint8_t rx_chain_0_valid;  /**< Must be set to true if rx_chain_0 is being passed */
  nas_rx_chain_info_type_v01 rx_chain_0;

  /* Optional */
  /*  Rx Chain 1 Info */
  uint8_t rx_chain_1_valid;  /**< Must be set to true if rx_chain_1 is being passed */
  nas_rx_chain_info_type_v01 rx_chain_1;

  /* Optional */
  /*  Tx Info */
  uint8_t tx_valid;  /**< Must be set to true if tx is being passed */
  nas_tx_info_type_v01 tx;
}nas_get_tx_rx_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Service Programming Code */
  char spc[NAS_SPC_MAX_V01];
  /**<   Service programming code in ASCII format (digits 0 to 9 only).
  */

  /*  AKEY */
  uint8_t akey[QMI_NAS_AKEY_LEN_V01];
  /**<   AKEY value + checksum value in ASCII (first 20 bytes are the AKEY value,
       last 6 bytes are the checksum).
  */
}nas_akey_with_spc_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Updates the A-KEY (extended).
              \label{idl:updateAkeyExt} */
typedef struct {

  /* Mandatory */
  /*  AKEY with SPC */
  nas_akey_with_spc_type_v01 akey_with_spc;
}nas_update_akey_ext_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Updates the A-KEY (extended).
              \label{idl:updateAkeyExt} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_update_akey_ext_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Indicates whether managed roaming is enabled. */
typedef struct {

  /* Optional */
  /*  Radio Interface */
  uint8_t radio_if_valid;  /**< Must be set to true if radio_if is being passed */
  nas_radio_if_enum_v01 radio_if;
  /**<  
    Radio interface from which to get the information. Values: \n
    - 0x01 -- NAS_RADIO_IF_CDMA_1X     -- 
      cdma2000\textsuperscript{\textregistered} 1X             \n
    - 0x02 -- NAS_RADIO_IF_CDMA_1XEVDO -- 
      cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
    - 0x04 -- NAS_RADIO_IF_GSM         -- GSM \n
    - 0x05 -- NAS_RADIO_IF_UMTS        -- UMTS \n
    - 0x08 -- NAS_RADIO_IF_LTE         -- LTE \n
    - 0x09 -- NAS_RADIO_IF_TDSCDMA     -- TD-SCDMA
  */
}nas_managed_roaming_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_dual_standby_pref_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves dual standby preference. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Standby Preference */
  uint8_t standby_pref_valid;  /**< Must be set to true if standby_pref is being passed */
  nas_standby_pref_enum_v01 standby_pref;
  /**<   Values: \n
       -0x01 -- Single standby \n
       -0x02 -- Dual standby with tune away \n
       -0x04 -- Dual standby without tune away \n
       -0x05 -- Automatic mode with tune away where applicable \n
       -0x06 -- Automatic mode without tune away \n
       -0x07 -- Triple standby
  */

  /* Optional */
  /*  Priority Subs */
  uint8_t priority_subs_valid;  /**< Must be set to true if priority_subs is being passed */
  nas_subs_type_enum_v01 priority_subs;
  /**<   Subscription to give priority when listening to the paging channel during
 dual standby. Values: \n
      - NAS_PRIMARY_SUBSCRIPTION (0x00) --  Primary subscription \n 
      - NAS_SECONDARY_SUBSCRIPTION (0x01) --  Secondary subscription \n 
      - NAS_TERTIARY_SUBSCRIPTION (0x02) --  Tertiary subscription 
 */

  /* Optional */
  /*  Active Subs */
  uint8_t active_subs_valid;  /**< Must be set to true if active_subs is being passed */
  nas_subs_type_enum_v01 active_subs;
  /**<   Subscription to enable when "standby_pref is 0x01 -- Single standby". 
 Values: \n
      - NAS_PRIMARY_SUBSCRIPTION (0x00) --  Primary subscription \n 
      - NAS_SECONDARY_SUBSCRIPTION (0x01) --  Secondary subscription \n 
      - NAS_TERTIARY_SUBSCRIPTION (0x02) --  Tertiary subscription 
 */

  /* Optional */
  /*  Default Data Subs */
  uint8_t default_data_subs_valid;  /**< Must be set to true if default_data_subs is being passed */
  nas_subs_type_enum_v01 default_data_subs;
  /**<   Default data subscription. Values: \n
      - NAS_PRIMARY_SUBSCRIPTION (0x00) --  Primary subscription \n 
      - NAS_SECONDARY_SUBSCRIPTION (0x01) --  Secondary subscription \n 
      - NAS_TERTIARY_SUBSCRIPTION (0x02) --  Tertiary subscription 
 */

  /* Optional */
  /*  Default Voice Subs */
  uint8_t default_voice_subs_valid;  /**< Must be set to true if default_voice_subs is being passed */
  nas_subs_type_enum_v01 default_voice_subs;
  /**<   Default voice subscription. Values: \n
      - NAS_PRIMARY_SUBSCRIPTION (0x00) --  Primary subscription \n 
      - NAS_SECONDARY_SUBSCRIPTION (0x01) --  Secondary subscription \n 
      - NAS_TERTIARY_SUBSCRIPTION (0x02) --  Tertiary subscription 

 \vspace{3pt}
 All other values are reserved.
 */

  /* Optional */
  /*  Active Subs Mask */
  uint8_t active_subs_mask_valid;  /**< Must be set to true if active_subs_mask is being passed */
  nas_active_subs_mask_type_v01 active_subs_mask;
  /**<   Bitmask representing the active subscriptions in the device. If a value 
       of 0 is sent, there are no active subscriptions.
       Values: \n
       - Bit 0 (0x01) -- QMI_NAS_ACTIVE_SUB_ PRIMARY   -- Primary subscription \n
       - Bit 1 (0x02) -- QMI_NAS_ACTIVE_SUB_ SECONDARY -- Secondary subscription \n
       - Bit 2 (0x04) -- QMI_NAS_ACTIVE_SUB_ TERTIARY  -- Tertiary subscription

       \vspace{3pt}
       All unlisted bits are reserved for future use and the service point
       ignores them if used.
  */
}nas_get_dual_standby_pref_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_detach_lte_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Detaches the current LTE system. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_detach_lte_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  MCC */
  uint16_t mcc;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.
  */

  /*  MNC */
  uint16_t mnc;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.
  */

  /*  MNC PCS digit include status */
  uint8_t mnc_includes_pcs_digit;
  /**<   This field is used to interpret the length of the corresponding
       MNC reported in this TLV. Values: \n

       - TRUE  -- MNC is a three-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 090  \n
       - FALSE -- MNC is a two-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 90
  */
}nas_plmn_id_ext_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Blocks the LTE PLMN. */
typedef struct {

  /* Mandatory */
  /*  PLMN */
  nas_plmn_id_ext_type_v01 plmn;

  /* Optional */
  /*  Blocking Interval Absolute Time */
  uint8_t blocking_interval_abs_valid;  /**< Must be set to true if blocking_interval_abs is being passed */
  uint32_t blocking_interval_abs;
  /**<   Blocking interval in absolute time (in milliseconds).
  */

  /* Optional */
  /*  Blocking Interval T3204 Multiplier */
  uint8_t blocking_interval_mult_valid;  /**< Must be set to true if blocking_interval_mult is being passed */
  float blocking_interval_mult;
  /**<   Blocking time as a multiplier of T3204.
  */
}nas_block_lte_plmn_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Blocks the LTE PLMN. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_block_lte_plmn_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Unblocks the LTE PLMN. */
typedef struct {

  /* Mandatory */
  /*  PLMN */
  nas_plmn_id_ext_type_v01 plmn;
}nas_unblock_lte_plmn_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Unblocks the LTE PLMN. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_unblock_lte_plmn_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_reset_lte_plmn_blocking_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Resets all previous LTE PLMN blocking operations. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_reset_lte_plmn_blocking_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  SPN coding scheme */
  nas_coding_scheme_enum_v01 spn_enc;
  /**<  
        Coding scheme for the service provider name. Values: \n
        - 0x00 -- NAS_CODING_SCHEME_ CELL_BROADCAST_GSM -- SMS default 7-bit coded 
                  alphabet as defined in \hyperref[S8]{[S8]} with bit 8 set to 0 \n
        - 0x01 -- NAS_CODING_SCHEME_ UCS2 -- UCS2 (16 bit, little-endian) 
                  \hyperref[S8]{[S8]} \n
        Note: This value is ignored if spn_len is zero.
  */

  /*  SPN */
  uint32_t spn_len;  /**< Must be set to # of elements in spn */
  uint8_t spn[NAS_SPN_LEN_MAX_V01];
  /**<  
     Service provider name string.
  */
}nas_spn_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  PLMN name encoding scheme */
  nas_coding_scheme_enum_v01 plmn_name_enc;
  /**<  
        Coding scheme for plmn_name. Values: \n
        - 0x00 -- NAS_CODING_SCHEME_ CELL_BROADCAST_GSM -- SMS default 7-bit coded 
                  alphabet as defined in \hyperref[S8]{[S8]} with bit 8 set to 0 \n
        - 0x01 -- NAS_CODING_SCHEME_ UCS2 -- UCS2 (16 bit, little-endian) 
                  \hyperref[S8]{[S8]} \n
        Note: This value is ignored if plmn_name_len is zero.
  */

  /*  PLMN name country initial include status */
  nas_country_initials_add_enum_v01 plmn_name_ci;
  /**<  
        Indicates whether the country initials are to be added to the plmn_name. 
        Values: \n
        - 0x00 -- Do not add the letters for the country's initials to the name \n
        - 0x01 -- Add the country's initials and a text string to the name \n
        - 0xFF -- Not specified \n
        Note: This value is ignored if plmn_name_len is zero.
  */

  /*  PLMN spare bits  */
  nas_spare_bits_enum_v01 plmn_spare_bits;
  /**<   Values: \n
       - 0x01 -- SPARE_BITS_8       -- Bit 8 is spare and set to 0 in octet n                       \n
       - 0x02 -- SPARE_BITS_7_TO_8  -- Bits 7 and 8 are spare and set to 0 in octet n               \n               
       - 0x03 -- SPARE_BITS_6_TO_8  -- Bits 6 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x04 -- SPARE_BITS_5_TO_8  -- Bits 5 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x05 -- SPARE_BITS_4_TO_8  -- Bits 4 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x06 -- SPARE_BITS_3_TO_8  -- Bits 3 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x07 -- SPARE_BITS_2_TO_8  -- Bits 2 to 8 (inclusive) are spare and set to 0 in octet n    \n
       - 0x00 -- SPARE_BITS_UNKNOWN -- Carries no information about the number of spare bits in octet n    \n
       Note: This value is ignored if plmn_name_len is zero.
  */

  uint32_t plmn_name_len;  /**< Must be set to # of elements in plmn_name */
  uint8_t plmn_name[NAS_PLMN_NAME_MAX_V01];
  /**<   PLMN name.
  */
}nas_plmn_name_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Indicates the current SPN and PLMN name information.
             \label{idl:currPlmnNameInd} */
typedef struct {

  /* Optional */
  /*  PLMN ID */
  uint8_t plmn_id_valid;  /**< Must be set to true if plmn_id is being passed */
  nas_plmn_id_ext_type_v01 plmn_id;

  /* Optional */
  /*  Service Provider Name */
  uint8_t spn_valid;  /**< Must be set to true if spn is being passed */
  nas_spn_type_v01 spn;

  /* Optional */
  /*  Short Name for Network */
  uint8_t short_name_valid;  /**< Must be set to true if short_name is being passed */
  nas_plmn_name_type_v01 short_name;

  /* Optional */
  /*  Long Name for Network */
  uint8_t long_name_valid;  /**< Must be set to true if long_name is being passed */
  nas_plmn_name_type_v01 long_name;

  /* Optional */
  /*  CSG ID for Network */
  uint8_t csg_id_valid;  /**< Must be set to true if csg_id is being passed */
  uint32_t csg_id;
  /**<   Closed subscriber group identifier; included only when the network 
       is a CSG network.
  */

  /* Optional */
  /*  Display Bit Information */
  uint8_t eons_display_bit_info_valid;  /**< Must be set to true if eons_display_bit_info is being passed */
  nas_display_bit_type_v01 eons_display_bit_info;

  /* Optional */
  /*  Network Information */
  uint8_t is_home_network_valid;  /**< Must be set to true if is_home_network is being passed */
  nas_tri_state_boolean_type_v01 is_home_network;
  /**<   Whether the network is the home network. Values: \n
      - NAS_TRI_FALSE (0) --  Status: FALSE \n 
      - NAS_TRI_TRUE (1) --  Status: TRUE  \n 
      - NAS_TRI_UNKNOWN (2) --  Status: Unknown  
 */

  /* Optional */
  /*  Radio Access Technology */
  uint8_t rat_valid;  /**< Must be set to true if rat is being passed */
  nas_radio_if_enum_v01 rat;
  /**<   Radio access technology. Values: \n
    - 0x04 -- NAS_RADIO_IF_GSM         -- GSM \n
    - 0x05 -- NAS_RADIO_IF_UMTS        -- UMTS \n
    - 0x08 -- NAS_RADIO_IF_LTE         -- LTE \n
    - 0x09 -- NAS_RADIO_IF_TDSCDMA     -- TD-SCDMA
   */

  /* Optional */
  /*  3GPP EONS PLMN Name with Language ID */
  uint8_t lang_plmn_names_valid;  /**< Must be set to true if lang_plmn_names is being passed */
  uint32_t lang_plmn_names_len;  /**< Must be set to # of elements in lang_plmn_names */
  nas_lang_plmn_names_type_v01 lang_plmn_names[NAS_ALT_LANG_MAX_V01];
  /**<   List of additional PLMN names, with their language identifier.
  */
}nas_current_plmn_name_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Requests the UE to enable/disable eMBMS. */
typedef struct {

  /* Mandatory */
  /*  Config Request */
  uint8_t enable;
  /**<   Enable/disable eMBMS. Values: \n
       - TRUE  -- Enable \n
       - FALSE -- Disable
  */

  /* Optional */
  /*  Trace ID */
  uint8_t trace_id_valid;  /**< Must be set to true if trace_id is being passed */
  int16_t trace_id;
  /**<   Trace ID.  Values: \n
       - 0 to 32768 -- Valid trace ID \n
       - -1 -- Trace ID is not used
  */
}nas_config_embms_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Requests the UE to enable/disable eMBMS. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Trace ID */
  uint8_t trace_id_valid;  /**< Must be set to true if trace_id is being passed */
  int16_t trace_id;
  /**<   Trace ID.  Values: \n
       - 0 to 32768 -- Valid trace ID \n
       - -1 -- Trace ID is not used
  */
}nas_config_embms_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_embms_status_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the eMBMS status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  eMBMS Status */
  uint8_t enabled_valid;  /**< Must be set to true if enabled is being passed */
  uint8_t enabled;
  /**<   eMBMS status. Values: \n
       - TRUE  -- Enabled \n
       - FALSE -- Disabled
  */

  /* Optional */
  /*  Trace ID */
  uint8_t trace_id_valid;  /**< Must be set to true if trace_id is being passed */
  int16_t trace_id;
  /**<   Trace ID.  Values: \n
       - 0 to 32768 -- Valid trace ID \n
       - -1 -- Trace ID is not used
  */
}nas_get_embms_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Reports the UE's current eMBMS status change. */
typedef struct {

  /* Mandatory */
  /*  eMBMS Status */
  uint8_t enabled;
  /**<   eMBMS status. Values: \n
       - TRUE  -- Enabled \n
       - FALSE -- Disabled
  */

  /* Optional */
  /*  Trace ID */
  uint8_t trace_id_valid;  /**< Must be set to true if trace_id is being passed */
  int16_t trace_id;
  /**<   Trace ID.  Values: \n
       - 0 to 32768 -- Valid trace ID \n
       - -1 -- Trace ID is not used
  */
}nas_embms_status_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_cdma_position_info_req_msg_v01;

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_CDMA_PILOT_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_CDMA_PILOT_CURR_ACT_PLT_V01 = 0x00, 
  NAS_CDMA_PILOT_NEIGHBOR_PLT_V01 = 0x01, 
  NAS_CDMA_PILOT_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_cdma_pilot_type_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_cdma_pilot_type_enum_v01 pilot_type;
  /**<   Pilot information type. Values: \n
       - 0x00 -- NAS_CDMA_PILOT_CURR_ACT_PLT -- Current active pilot information \n
       - 0x01 -- NAS_CDMA_PILOT_NEIGHBOR_PLT -- Neighbor pilot information
   */

  uint16_t sid;
  /**<   System ID. Range: 0 to 32767. 
   */

  uint16_t nid;
  /**<   Network ID. Range: 0 to 65535.
  */

  uint16_t base_id;
  /**<   Base station ID. */

  uint16_t pilot_pn;
  /**<   Pilot PN sequence offset index. Range: 0 to 511.
  */

  uint16_t pilot_strength;
  /**<   Strength of the pilot (in dB). Range: 0 to 64.
  */

  uint32_t base_lat;
  /**<   Latitude of the current base station in units of 0.25 sec.
  */

  uint32_t base_long;
  /**<   Longitude of the current base station in units of 0.25 sec.
  */

  uint64_t time_stamp;
  /**<   Time (in milliseconds) from the start of GPS time when the measurement 
       was taken.
  */
}nas_cdma_bs_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t ue_in_idle;
  /**<   CDMA Idle state. TRUE if the UE is in Idle mode; otherwise FALSE.
  */

  /*  CDMA base station info */
  uint32_t bs_len;  /**< Must be set to # of elements in bs */
  nas_cdma_bs_info_type_v01 bs[NAS_CDMA_POSITION_INFO_MAX_V01];
}nas_cdma_position_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the current CDMA base station position information for
              active and neighbor's position information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CDMA Position Info */
  uint8_t info_valid;  /**< Must be set to true if info is being passed */
  nas_cdma_position_info_type_v01 info;
}nas_get_cdma_position_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Reports current RF band information. */
typedef struct {

  /* Mandatory */
  /*  RF Band Information */
  nas_rf_band_info_type_v01 rf_band_info;
}nas_rf_band_info_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_force_network_search_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Forces a network search procedure. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}nas_force_network_search_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Reports network reject information. */
typedef struct {

  /* Mandatory */
  /*  Radio Interface */
  nas_radio_if_enum_v01 radio_if;
  /**<  
    Radio interface from which to get the information. Values: \n
    - 0x04 -- NAS_RADIO_IF_GSM         -- GSM \n
    - 0x05 -- NAS_RADIO_IF_UMTS        -- UMTS \n
    - 0x08 -- NAS_RADIO_IF_LTE         -- LTE \n
    - 0x09 -- NAS_RADIO_IF_TDSCDMA     -- TD-SCDMA
  */

  /* Mandatory */
  /*  Service Domain */
  nas_service_domain_enum_type_v01 reject_srv_domain;
  /**<  
    Type of service domain in which the registration is rejected. Values: \n
    - 0x00 -- SYS_SRV_DOMAIN_NO_SRV  -- No service \n
    - 0x01 -- SYS_SRV_DOMAIN_CS_ONLY -- Circuit-switched only \n
    - 0x02 -- SYS_SRV_DOMAIN_PS_ONLY -- Packet-switched only \n
    - 0x03 -- SYS_SRV_DOMAIN_CS_PS   -- Circuit-switched and packet-switched \n
    - 0x04 -- SYS_SRV_DOMAIN_CAMPED  -- Camped
      */

  /* Mandatory */
  /*  Registration Rejection Cause */
  uint8_t rej_cause;
  /**<  
    Reject cause values sent are specified in 
    \hyperref[S5]{[S5]} Sections 10.5.3.6 and 10.5.5.14, and 
    \hyperref[S16]{[S16]} Section 9.9.3.9.
  */

  /* Optional */
  /*  PLMN ID  */
  uint8_t plmn_id_valid;  /**< Must be set to true if plmn_id is being passed */
  nas_mnc_pcs_digit_include_status_type_v01 plmn_id;

  /* Optional */
  /*  CSG ID */
  uint8_t csg_id_valid;  /**< Must be set to true if csg_id is being passed */
  uint32_t csg_id;
  /**<   Closed subscriber group identifier.
  */
}nas_network_reject_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_managed_roaming_config_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the current managed roaming configuration information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Managed Roaming Configuration */
  uint8_t managed_roaming_supported_valid;  /**< Must be set to true if managed_roaming_supported is being passed */
  uint8_t managed_roaming_supported;
  /**<   Managed roaming support status (corresponds to NV item 
       NV_MGRF_SUPPORTED_I). Values: \n
       - 0 -- Not supported \n
       - 1 -- Supported
  */
}nas_get_managed_roaming_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Reports a change in the RTRE configuration status.  */
typedef struct {

  /* Optional */
  /*  Current RTRE Configuration */
  uint8_t rtre_cfg_valid;  /**< Must be set to true if rtre_cfg is being passed */
  nas_rtre_cfg_enum_v01 rtre_cfg;
  /**<   Values: \n 
       - 0x01 -- R-UIM only \n 
       - 0x02 -- Internal settings only \n 
       - 0x04 -- GSM on 1X
  */

  /* Optional */
  /*  RTRE Configuration Preference */
  uint8_t rtre_cfg_pref_valid;  /**< Must be set to true if rtre_cfg_pref is being passed */
  nas_rtre_cfg_enum_v01 rtre_cfg_pref;
  /**<   Values: \n 
       - 0x01 -- R-UIM only \n 
       - 0x02 -- Internal settings only \n 
       - 0x03 -- Use R-UIM if available \n 
       - 0x04 -- GSM on 1X
  */
}nas_rtre_cfg_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_centralized_eons_support_status_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the modem support status for centralized EONS. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Centralized EONS Support Status */
  uint8_t centralized_eons_supported_valid;  /**< Must be set to true if centralized_eons_supported is being passed */
  uint8_t centralized_eons_supported;
  /**<   Centralized EONS support status. Values: \n
       - 0 -- Not supported \n
       - 1 -- Supported
  */
}nas_get_centralized_eons_support_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets the signal strength reporting thresholds. 
               \label{idl:configSigInfo2} */
typedef struct {

  /* Optional */
  /*  CDMA RSSI Threshold List  */
  uint8_t cdma_rssi_threshold_list_valid;  /**< Must be set to true if cdma_rssi_threshold_list is being passed */
  uint32_t cdma_rssi_threshold_list_len;  /**< Must be set to # of elements in cdma_rssi_threshold_list */
  int16_t cdma_rssi_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of RSSI thresholds (in units of 0.1 dBm); maximum of 32 values. 
      Range for RSSI values: -105 to -21 (in dBm). \n
      For example, to set thresholds at -95 dBm and -80 dBm, the threshold 
      list values are {-950, -800}. \n
      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  CDMA RSSI Delta  */
  uint8_t cdma_rssi_delta_valid;  /**< Must be set to true if cdma_rssi_delta is being passed */
  uint16_t cdma_rssi_delta;
  /**<  
      RSSI delta (in units of 0.1 dBm). \n
      For example, to set a delta of 10 dBm, the delta value must be set 
      to 100. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  CDMA ECIO Threshold List */
  uint8_t cdma_ecio_threshold_list_valid;  /**< Must be set to true if cdma_ecio_threshold_list is being passed */
  uint32_t cdma_ecio_threshold_list_len;  /**< Must be set to # of elements in cdma_ecio_threshold_list */
  int16_t cdma_ecio_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of ECIO thresholds (in units of 0.1 dB); maximum of 32 values. 
      Range for ECIO values: -31.5 to 0 (in dB). \n
      For example, to set thresholds at -20 dB and -15.5 dB, the threshold 
      list values are {-400, -310}. \n
      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  CDMA ECIO Delta */
  uint8_t cdma_ecio_delta_valid;  /**< Must be set to true if cdma_ecio_delta is being passed */
  uint16_t cdma_ecio_delta;
  /**<  
      ECIO delta (in units of 0.1 dB). \n
      For example, to set a delta of 10 dB, the delta value must be set 
      to 100. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  HDR RSSI Threshold List  */
  uint8_t hdr_rssi_threshold_list_valid;  /**< Must be set to true if hdr_rssi_threshold_list is being passed */
  uint32_t hdr_rssi_threshold_list_len;  /**< Must be set to # of elements in hdr_rssi_threshold_list */
  int16_t hdr_rssi_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of RSSI thresholds (in units of 0.1 dBm); maximum of 32 values. 
      Range for RSSI values: -118 to -13 (in dBm). \n
      For example, to set thresholds at -20 dBm and -15 dBm, the threshold 
      list values are {-200, -150}. \n
      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  HDR RSSI Delta  */
  uint8_t hdr_rssi_delta_valid;  /**< Must be set to true if hdr_rssi_delta is being passed */
  uint16_t hdr_rssi_delta;
  /**<  
      RSSI delta (in units of 0.1 dBm). \n
      For example, to set a delta of 10 dBm, the delta value must be set 
      to 100. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  HDR ECIO Threshold List */
  uint8_t hdr_ecio_threshold_list_valid;  /**< Must be set to true if hdr_ecio_threshold_list is being passed */
  uint32_t hdr_ecio_threshold_list_len;  /**< Must be set to # of elements in hdr_ecio_threshold_list */
  int16_t hdr_ecio_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of ECIO thresholds (in units of 0.1 dB); maximum of 32 values. 
      Range for ECIO values: -31.5 to 0 (in dB). \n
      For example, to set thresholds at -20 dB and -15.5 dB, the threshold 
      list values are {-400, -310}. \n
      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  HDR ECIO Delta */
  uint8_t hdr_ecio_delta_valid;  /**< Must be set to true if hdr_ecio_delta is being passed */
  uint16_t hdr_ecio_delta;
  /**<  
      ECIO delta (in units of 0.1 dB). \n
      For example, to set a delta of 10 dB, the delta value must be set 
      to 100. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  HDR SINR Threshold List */
  uint8_t hdr_sinr_threshold_list_valid;  /**< Must be set to true if hdr_sinr_threshold_list is being passed */
  uint32_t hdr_sinr_threshold_list_len;  /**< Must be set to # of elements in hdr_sinr_threshold_list */
  uint16_t hdr_sinr_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of SINR level thresholds (in units of 1); maximum of 32 values. 
      Valid levels are 0 to 8, where the maximum value for: \n
      - 0x00 -- SINR_LEVEL_0 is -9 dB   \n
      - 0x01 -- SINR_LEVEL_1 is -6 dB   \n
      - 0x02 -- SINR_LEVEL_2 is -4.5 dB \n
      - 0x03 -- SINR_LEVEL_3 is -3 dB   \n
      - 0x04 -- SINR_LEVEL_4 is -2 dB   \n
      - 0x05 -- SINR_LEVEL_5 is +1 dB   \n
      - 0x06 -- SINR_LEVEL_6 is +3 dB   \n
      - 0x07 -- SINR_LEVEL_7 is +6 dB   \n
      - 0x08 -- SINR_LEVEL_8 is +9 dB
 */

  /* Optional */
  /*  HDR SINR Delta */
  uint8_t hdr_sinr_delta_valid;  /**< Must be set to true if hdr_sinr_delta is being passed */
  uint16_t hdr_sinr_delta;
  /**<  
      SINR delta (in units of 1 SINR level). \n
      For example, to set a delta of 1 SINR level, the delta value must be 
      set to 1. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  HDR IO Threshold List */
  uint8_t hdr_io_threshold_list_valid;  /**< Must be set to true if hdr_io_threshold_list is being passed */
  uint32_t hdr_io_threshold_list_len;  /**< Must be set to # of elements in hdr_io_threshold_list */
  int16_t hdr_io_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of IO thresholds (in units of 0.1 dBm); maximum of 32 values. 
      Range for IO values: -128 to -13 (in dBm). \n
      For example, to set thresholds at -111 dBm and -73 dBm, the threshold 
      list values are {-1110, -730}. \n
      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  HDR IO Delta */
  uint8_t hdr_io_delta_valid;  /**< Must be set to true if hdr_io_delta is being passed */
  uint16_t hdr_io_delta;
  /**<  
      IO delta (in units of 0.1 dBm). \n
      For example, to set a delta of 10 dBm, the delta value must be set 
      to 100. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  GSM RSSI Threshold List  */
  uint8_t gsm_rssi_threshold_list_valid;  /**< Must be set to true if gsm_rssi_threshold_list is being passed */
  uint32_t gsm_rssi_threshold_list_len;  /**< Must be set to # of elements in gsm_rssi_threshold_list */
  int16_t gsm_rssi_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of RSSI thresholds (in units of 0.1 dBm); maximum of 32 values. 
      Range for RSSI values: -111 to -48 (in dBm). \n
      For example, to set thresholds at -95 dBm and -80 dBm, the threshold 
      list values are {-950, -800}. \n
      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  GSM RSSI Delta  */
  uint8_t gsm_rssi_delta_valid;  /**< Must be set to true if gsm_rssi_delta is being passed */
  uint16_t gsm_rssi_delta;
  /**<  
      RSSI delta (in units of 0.1 dBm). \n
      For example, to set a delta of 10 dBm, the delta value must be set 
      to 100. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  WCDMA RSSI Threshold List  */
  uint8_t wcdma_rssi_threshold_list_valid;  /**< Must be set to true if wcdma_rssi_threshold_list is being passed */
  uint32_t wcdma_rssi_threshold_list_len;  /**< Must be set to # of elements in wcdma_rssi_threshold_list */
  int16_t wcdma_rssi_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of RSSI thresholds (in units of 0.1 dBm); maximum of 32 values. 
      Range for RSSI values: -121 to 0 (in dBm). \n
      For example, to set thresholds at -20 dBm and -15 dBm, the threshold 
      list values are {-200, -150}. \n
      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  WCDMA RSSI Delta  */
  uint8_t wcdma_rssi_delta_valid;  /**< Must be set to true if wcdma_rssi_delta is being passed */
  uint16_t wcdma_rssi_delta;
  /**<  
      RSSI delta (in units of 0.1 dBm). \n
      For example, to set a delta of 10 dBm, the delta value must be set 
      to 100. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  WCDMA ECIO Threshold List */
  uint8_t wcdma_ecio_threshold_list_valid;  /**< Must be set to true if wcdma_ecio_threshold_list is being passed */
  uint32_t wcdma_ecio_threshold_list_len;  /**< Must be set to # of elements in wcdma_ecio_threshold_list */
  int16_t wcdma_ecio_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of ECIO thresholds (in units of 0.1 dB); maximum of 32 values. 
      Range for ECIO values: -31.5 to 0 (in dB). \n
      For example, to set thresholds at -20 dB and -15.5 dB, the threshold 
      list values are {-400, -310}. \n
      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  WCDMA ECIO Delta */
  uint8_t wcdma_ecio_delta_valid;  /**< Must be set to true if wcdma_ecio_delta is being passed */
  uint16_t wcdma_ecio_delta;
  /**<  
      ECIO delta (in units of 0.1 dB). \n
      For example, to set a delta of 10 dB, the delta value must be set 
      to 100. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  LTE RSSI Threshold List  */
  uint8_t lte_rssi_threshold_list_valid;  /**< Must be set to true if lte_rssi_threshold_list is being passed */
  uint32_t lte_rssi_threshold_list_len;  /**< Must be set to # of elements in lte_rssi_threshold_list */
  int16_t lte_rssi_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of RSSI thresholds (in units of 0.1 dBm); maximum of 32 values. 
      Range for RSSI values: -120 to 0 (in dBm). \n
      For example, to set thresholds at -20 dBm and -15 dBm, the threshold 
      list values are {-200, -150}. \n
      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  LTE RSSI Delta  */
  uint8_t lte_rssi_delta_valid;  /**< Must be set to true if lte_rssi_delta is being passed */
  uint16_t lte_rssi_delta;
  /**<  
      RSSI delta (in units of 0.1 dBm).
      For example, to set a delta of 10 dBm, the delta value must be set 
      to 100. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  LTE SNR Threshold List */
  uint8_t lte_snr_threshold_list_valid;  /**< Must be set to true if lte_snr_threshold_list is being passed */
  uint32_t lte_snr_threshold_list_len;  /**< Must be set to # of elements in lte_snr_threshold_list */
  int16_t lte_snr_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of SNR thresholds (in units of 0.1 dB); maximum of 32 values. 
      Range for SNR values: -20 to 30 (in dB). \n
      For example, to set thresholds at -19.8 dB and 23 dB, the threshold 
      list values are {-198, 230}. \n
      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  LTE SNR Delta */
  uint8_t lte_snr_delta_valid;  /**< Must be set to true if lte_snr_delta is being passed */
  uint16_t lte_snr_delta;
  /**<  
      SNR delta (in units of 0.1 dBm). \n
      For example, to set a delta of 10 dBm, the delta value must be set 
      to 100. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  LTE RSRQ Threshold List */
  uint8_t lte_rsrq_threshold_list_valid;  /**< Must be set to true if lte_rsrq_threshold_list is being passed */
  uint32_t lte_rsrq_threshold_list_len;  /**< Must be set to # of elements in lte_rsrq_threshold_list */
  int16_t lte_rsrq_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of RSRQ thresholds (in units of 0.1 dBm); maximum of 32 values. 
      Range for RSRQ values: -20 to -3 (in dBm). \n
      For example, to set thresholds at -11 dBm and -6 dBm, the threshold 
      list values are {-110, -60}. \n
      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  LTE RSRQ Delta */
  uint8_t lte_rsrq_delta_valid;  /**< Must be set to true if lte_rsrq_delta is being passed */
  uint16_t lte_rsrq_delta;
  /**<  
      RSRQ delta (in units of 0.1 dBm). \n
      For example, to set a delta of 10 dBm, the delta value must be set 
      to 100. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  LTE RSRP Threshold List */
  uint8_t lte_rsrp_threshold_list_valid;  /**< Must be set to true if lte_rsrp_threshold_list is being passed */
  uint32_t lte_rsrp_threshold_list_len;  /**< Must be set to # of elements in lte_rsrp_threshold_list */
  int16_t lte_rsrp_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of RSRP thresholds (in units of 0.1 dBm); maximum of 32 values. 
      Range for RSRP values: -140 to -44 (in dBm). \n
      For example, to set thresholds at -125 dBm and -64 dBm, the threshold 
      list values are {-1250, -640}. \n
      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  LTE RSRP Delta */
  uint8_t lte_rsrp_delta_valid;  /**< Must be set to true if lte_rsrp_delta is being passed */
  uint16_t lte_rsrp_delta;
  /**<  
      RSRP delta (in units of 0.1 dBm). \n
      For example, to set a delta of 10 dBm, the delta value must be set 
      to 100. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  LTE Signal Report Config */
  uint8_t lte_sig_rpt_config_valid;  /**< Must be set to true if lte_sig_rpt_config is being passed */
  nas_lte_sig_rpt_config_type_v01 lte_sig_rpt_config;

  /* Optional */
  /*  TDSCDMA RSCP Threshold List  */
  uint8_t tdscdma_rscp_threshold_list_valid;  /**< Must be set to true if tdscdma_rscp_threshold_list is being passed */
  uint32_t tdscdma_rscp_threshold_list_len;  /**< Must be set to # of elements in tdscdma_rscp_threshold_list */
  int16_t tdscdma_rscp_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of RSCP thresholds (in units of 0.1 dBm); maximum of 32 values. 
      Range for RSCP values: -120 to -25 (in dBm). \n
      For example, to set thresholds at -95 dBm and -80 dBm, the threshold 
      list values would be {-950, -800}. \n
      The range is based on the latest releases and may change over time.
 */

  /* Optional */
  /*  TDSCDMA RSCP Delta */
  uint8_t tdscdma_rscp_delta_valid;  /**< Must be set to true if tdscdma_rscp_delta is being passed */
  uint16_t tdscdma_rscp_delta;
  /**<  
      RSCP delta (in units of 0.1 dBm). \n
      For example, to set a delta of 10 dBm, the delta value must be set 
      to 100. A value of 0 is rejected with a QMI_ERR_INVALID_ARG error.
 */

  /* Optional */
  /*  TDSCDMA RSSI Threshold List  */
  uint8_t tds_rssi_threshold_list_valid;  /**< Must be set to true if tds_rssi_threshold_list is being passed */
  uint32_t tds_rssi_threshold_list_len;  /**< Must be set to # of elements in tds_rssi_threshold_list */
  float tds_rssi_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of RSSI thresholds (in dBm) used by TD-SCDMA; maximum of 32 values.
  */

  /* Optional */
  /*  TDSCDMA RSSI Delta */
  uint8_t tdscdma_rssi_delta_valid;  /**< Must be set to true if tdscdma_rssi_delta is being passed */
  float tdscdma_rssi_delta;
  /**<  
      RSSI delta (in dBm) used by TD-SCDMA.
 */

  /* Optional */
  /*  TDSCDMA ECIO Threshold List  */
  uint8_t tds_ecio_threshold_list_valid;  /**< Must be set to true if tds_ecio_threshold_list is being passed */
  uint32_t tds_ecio_threshold_list_len;  /**< Must be set to # of elements in tds_ecio_threshold_list */
  float tds_ecio_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of ECIO thresholds (in dB) used by TD-SCDMA; maximum of 32 values.
  */

  /* Optional */
  /*  TDSCDMA ECIO Delta */
  uint8_t tdscdma_ecio_delta_valid;  /**< Must be set to true if tdscdma_ecio_delta is being passed */
  float tdscdma_ecio_delta;
  /**<  
      ECIO delta (in dB) used by TD-SCDMA.
 */

  /* Optional */
  /*  TDSCDMA SINR Threshold List  */
  uint8_t tds_sinr_threshold_list_valid;  /**< Must be set to true if tds_sinr_threshold_list is being passed */
  uint32_t tds_sinr_threshold_list_len;  /**< Must be set to # of elements in tds_sinr_threshold_list */
  float tds_sinr_threshold_list[NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01];
  /**<  
      Array of SINR thresholds (in dB) used by TD-SCDMA; maximum of 32 values.
 */

  /* Optional */
  /*  TDSCDMA SINR Delta */
  uint8_t tdscdma_sinr_delta_valid;  /**< Must be set to true if tdscdma_sinr_delta is being passed */
  float tdscdma_sinr_delta;
  /**<  
      SINR delta (in dB) used by TD-SCDMA.
 */
}nas_config_sig_info2_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets the signal strength reporting thresholds. 
               \label{idl:configSigInfo2} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}nas_config_sig_info2_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_tds_cell_and_position_info_req_msg_v01;

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  PLMN ID */
  nas_plmn_id_ext_type_v01 plmn;

  /*  LAC */
  uint16_t lac;
  /**<   Location area code. (This field is ignored when cell_id is not present.)
  */

  /*  UARFCN */
  uint16_t uarfcn;
  /**<   Absolute RF channel number.
  */

  /*  Cell id */
  uint32_t cell_id;
  /**<   Cell ID (0xFFFFFFFF indicates cell ID information is not present).
  */

  /*  Cell parameter id */
  uint8_t cell_parameter_id;
  /**<   Cell parameter ID.
  */

  /*  Pathloss */
  uint8_t pathloss;
  /**<   Path loss in units of 1 dB.
  */

  /*  Timing advance */
  float timing_advance;
  /**<   Measured delay (in seconds) of an access burst transmission on the RACH 
       or PRACH to the expected signal from an MS at zero distance under static 
       channel conditions. 
  */

  /*  RSCP */
  float rscp;
  /**<   Received signal code power in dBm.
  */
}nas_tds_cell_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  UARFCN */
  uint16_t uarfcn;
  /**<   Absolute RF channel number.
  */

  /*  Cell parameter id */
  uint8_t cell_parameter_id;
  /**<   Cell parameter ID.
  */

  /*  RSCP */
  float rscp;
  /**<    Received signal code power in dBm.
  */
}nas_tds_nbr_cell_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the cell information and neighbor cell information 
               for TD-SCDMA. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  TDSCDMA Cell Info  */
  uint8_t tds_cell_info_valid;  /**< Must be set to true if tds_cell_info is being passed */
  nas_tds_cell_info_type_v01 tds_cell_info;

  /* Optional */
  /*  TDSCDMA Neighbor Cell Info  */
  uint8_t tds_nbr_cell_info_valid;  /**< Must be set to true if tds_nbr_cell_info is being passed */
  uint32_t tds_nbr_cell_info_len;  /**< Must be set to # of elements in tds_nbr_cell_info */
  nas_tds_nbr_cell_info_type_v01 tds_nbr_cell_info[NAS_TDS_MAX_NBR_CELL_NUM_V01];
}nas_get_tds_cell_and_position_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets the periodic search timer configuration for a 
              home operator-specific BPLMN search to LTE. */
typedef struct {

  /* Mandatory */
  /*  TDSCDMA Neighbor Cell Periodic Search Timer */
  uint16_t timer_value;
  /**<  
      TD-SCDMA search timer value (in minutes). \n
      0 indicates an immediate search and the timer is disabled. \n
      0xFFFF is used to disable the timer without any search.
 */
}nas_set_hplmn_irat_search_timer_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets the periodic search timer configuration for a 
              home operator-specific BPLMN search to LTE. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}nas_set_hplmn_irat_search_timer_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Retrieves the current signal quality at L1 for each MBSFN area. */
typedef struct {

  /* Optional */
  /*  Trace ID */
  uint8_t trace_id_valid;  /**< Must be set to true if trace_id is being passed */
  int16_t trace_id;
  /**<   Trace ID.  Values: \n
       - 0 to 32768 -- Valid trace ID \n
       - -1 -- Trace ID is not used
  */
}nas_get_embms_sig_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  MBSFN Area ID */
  uint8_t area_id;
  /**<   Multicast Broadcast Single Frequency Network (MBSFN) area ID. 
    Values: 0 to 255.
  */

  /*  SNR */
  float snr;
  /**<   Average SNR of the serving cell over the last measurement period in
    decibels.
   */

  /*  Signal Level */
  int8_t signal_level;
  /**<   Signal level of the serving cell over the last measurement period. 
    Range: 0 to 5.
   */
}nas_lte_cphy_mbsfn_area_signal_strength_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the current signal quality at L1 for each MBSFN area. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Trace ID */
  uint8_t trace_id_valid;  /**< Must be set to true if trace_id is being passed */
  int16_t trace_id;
  /**<   Trace ID.  Values: \n
       - 0 to 32768 -- Valid trace ID \n
       - -1 -- Trace ID is not used
  */

  /* Optional */
  /*  Signal Quality */
  uint8_t sig_list_valid;  /**< Must be set to true if sig_list is being passed */
  uint32_t sig_list_len;  /**< Must be set to # of elements in sig_list */
  nas_lte_cphy_mbsfn_area_signal_strength_type_v01 sig_list[NAS_LTE_EMBMS_MAX_MBSFN_AREA_V01];
}nas_get_embms_sig_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef uint64_t nas_limit_sys_info_ind_mask_type_v01;
#define NAS_LIMIT_BY_SRV_STATUS_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x01ull) 
#define NAS_LIMIT_BY_SRV_DOMAIN_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x02ull) 
#define NAS_LIMIT_BY_PLMN_ID_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x04ull) 
#define NAS_LIMIT_BY_SID_NID_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x08ull) 
#define NAS_LIMIT_BY_ROAM_STATUS_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x10ull) 
#define NAS_LIMIT_BY_SRV_CAPABILITY_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x20ull) 
#define NAS_LIMIT_BY_PACKET_ZONE_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x40ull) 
#define NAS_LIMIT_BY_IS856_SYS_ID_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x80ull) 
#define NAS_LIMIT_BY_CELL_ID_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x100ull) 
#define NAS_LIMIT_BY_LAC_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x200ull) 
#define NAS_LIMIT_BY_RAC_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x400ull) 
#define NAS_LIMIT_BY_TAC_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x800ull) 
#define NAS_LIMIT_BY_HS_CALL_STATUS_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x1000ull) 
#define NAS_LIMIT_BY_HS_IND_V01 ((nas_limit_sys_info_ind_mask_type_v01)0x2000ull) 
/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Limits the reporting of QMI_NAS_SYS_INFO_IND to only when 
              certain fields have changed. */
typedef struct {

  /* Mandatory */
  /*  Limit Sys Info Change Reporting */
  nas_limit_sys_info_ind_mask_type_v01 limit_sys_info_chg_rpt;
  /**<   Bitmasks included in this TLV limit the reporting of QMI_NAS_SYS_INFO_IND 
       to when those values change. If a value of 0 is sent, QMI_NAS_SYS_INFO_IND 
       reporting is as if no limit is set. Values: \n
      
       - 0x01 -- NAS_LIMIT_BY_SRV_STATUS  -- Limit by srv_status changes \n
       - 0x02 -- NAS_LIMIT_BY_SRV_DOMAIN  -- Limit by srv_domain changes \n
       - 0x04 -- NAS_LIMIT_BY_PLMN_ID     -- Limit by mcc/mnc \n
       - 0x08 -- NAS_LIMIT_BY_SID_NID     -- Limit by sid/nid \n
       - 0x10 -- NAS_LIMIT_BY_ROAM_STATUS -- Limit by roam_status \n
       - 0x20 --  NAS_LIMIT_BY_SRV_CAPABILITY -- Limit by srv_capability changes \n
       - 0x40 --  NAS_LIMIT_BY_PACKET_ZONE -- Limit by packet_zone changes \n
       - 0x80 --  NAS_LIMIT_BY_IS856_SYS_ID -- Limit by IS856_SYS_ID changes \n
       - 0x100 --  NAS_LIMIT_BY_CELL_ID -- Limit by CELL_ID changes \n
       - 0x200 --  NAS_LIMIT_BY_LAC -- Limit by LAC changes \n
       - 0x400 --  NAS_LIMIT_BY_RAC -- Limit by RAC changes \n
       - 0x800 --  NAS_LIMIT_BY_TAC -- Limit by TAC changes \n
       - 0x1000 --  NAS_LIMIT_BY_HS_CALL_STATUS -- Limit by hs_call_status \n
       - 0x2000 --  NAS_LIMIT_BY_HS_IND -- Limit by hs_ind

       \vspace{3pt}
       All other bits are reserved for future use.
  */
}nas_limit_sys_info_ind_reporting_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Limits the reporting of QMI_NAS_SYS_INFO_IND to only when 
              certain fields have changed. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}nas_limit_sys_info_ind_reporting_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_sys_info_ind_reporting_limit_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the limitations set on the reporting of 
              QMI_NAS_SYS_INFO_IND. */
typedef struct {

  /* Mandatory */
  /*  Limit Sys Info Change Reporting */
  nas_limit_sys_info_ind_mask_type_v01 limit_sys_info_chg_rpt;
  /**<   Bitmasks included in this TLV indicate the limits set on 
       QMI_NAS_SYS_INFO_IND reporting.  If a value of 0 is sent, 
       QMI_NAS_SYS_INFO_IND is reporting with no limitations. Values: \n
      
       - 0x01 -- NAS_LIMIT_BY_SRV_STATUS  -- Limit by srv_status changes \n
       - 0x02 -- NAS_LIMIT_BY_SRV_DOMAIN  -- Limit by srv_domain changes \n
       - 0x04 -- NAS_LIMIT_BY_PLMN_ID     -- Limit by mcc/mnc \n
       - 0x08 -- NAS_LIMIT_BY_SID_NID     -- Limit by sid/nid \n
       - 0x10 -- NAS_LIMIT_BY_ROAM_STATUS -- Limit by roam_status \n
       - 0x20 -- NAS_LIMIT_BY_SRV_CAPABILITY -- Limit by srv_capability changes \n
       - 0x40 -- NAS_LIMIT_BY_PACKET_ZONE -- Limit by packet_zone changes \n
       - 0x80 -- NAS_LIMIT_BY_IS856_SYS_ID -- Limit by IS856_SYS_ID changes \n
       - 0x100 -- NAS_LIMIT_BY_CELL_ID -- Limit by CELL_ID changes \n
       - 0x200 -- NAS_LIMIT_BY_LAC -- Limit by LAC changes \n
       - 0x400 -- NAS_LIMIT_BY_RAC -- Limit by RAC changes \n
       - 0x800 -- NAS_LIMIT_BY_TAC -- Limit by TAC changes \n
       - 0x1000 -- NAS_LIMIT_BY_HS_CALL_STATUS -- Limit by hs_call_status \n
       - 0x2000 -- NAS_LIMIT_BY_HS_IND -- Limit by hs_ind

       \vspace{3pt}
       All other bits are reserved for future use.
  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}nas_get_sys_info_ind_reporting_limit_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_CALL_TYPE_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_CALL_TYPE_E_VOICE_V01 = 0, 
  NAS_CALL_TYPE_E_SMS_V01 = 1, 
  NAS_CALL_TYPE_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_call_type_enum_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  IMS Preferred Call Types */
  nas_call_type_enum_type_v01 call_type;
  /**<   Call type for which IMS is preferred. Values: \n 
       - 0x00 -- CALL_TYPE_E_VOICE -- Voice \n 
       - 0x01 -- CALL_TYPE_E_SMS -- SMS
  */

  /*  Is IMS Registered */
  uint8_t is_registered;
  /**<   Whether IMS is registered. Values: \n
       - 0 -- Not registered \n
       - 1 -- Registered
  */
}nas_ims_registration_state_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Updates the IMS registration status. */
typedef struct {

  /* Mandatory */
  /*  Radio Access Technology */
  nas_radio_if_enum_v01 sys_mode;
  /**<   Radio interface system mode. Values: \n 
       - 0x02 -- RADIO_IF_CDMA_1XEVDO -- 
         cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       -0x04 -- RADIO_IF_GSM         -- GSM \n
       -0x05 -- RADIO_IF_UMTS        -- UMTS \n
       -0x08 -- RADIO_IF_LTE         -- LTE
  */

  /* Mandatory */
  /*  IMS Registration State */
  uint32_t registration_state_len;  /**< Must be set to # of elements in registration_state */
  nas_ims_registration_state_type_v01 registration_state[NAS_IMS_REG_STATUS_MAX_V01];
}nas_update_ims_status_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Updates the IMS registration status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}nas_update_ims_status_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_ims_pref_status_req_msg_v01;

typedef uint64_t nas_call_type_mask_type_v01;
#define NAS_CALL_TYPE_B_VOICE_V01 ((nas_call_type_mask_type_v01)0x01ull) 
#define NAS_CALL_TYPE_B_SMS_V01 ((nas_call_type_mask_type_v01)0x02ull) 
/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Current radio access technology */
  nas_radio_if_enum_v01 sys_mode;
  /**<   Radio interface system mode. Values: \n 
       - 0x02 -- RADIO_IF_CDMA_1XEVDO -- 
         cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
       -0x04 -- RADIO_IF_GSM         -- GSM \n
       -0x05 -- RADIO_IF_UMTS        -- UMTS \n
       -0x08 -- RADIO_IF_LTE         -- LTE
  */

  nas_call_type_mask_type_v01 ims_pref_call_type;
  /**<   Bitmask representing the IMS preferred call type. 
       Bits for call types preferring IMS must be set to 1.
       Otherwise, the bits must be set to 0. \n
       Values: \n
       - Bit 0 (0x01) -- NAS_CALL_TYPE_B_VOICE -- Voice \n
       - Bit 1 (0x02) -- NAS_CALL_TYPE_B_SMS   -- SMS
  */
}nas_ims_pref_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the IMS preference status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  IMS Preference Information */
  uint8_t ims_pref_valid;  /**< Must be set to true if ims_pref is being passed */
  nas_ims_pref_type_v01 ims_pref;
}nas_get_ims_pref_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Reports a change in the IMS preference.  */
typedef struct {

  /* Mandatory */
  /*  IMS Preference Information */
  nas_ims_pref_type_v01 ims_pref;
}nas_ims_pref_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Configures whether QMI_NAS_CURRENT_PLMN_NAME_IND returns the 
              modem-determined name or all available information. */
typedef struct {

  /* Mandatory */
  /*  Current PLMN Name Ind Send All Information */
  uint8_t send_all_information;
  /**<   Indicates that QMI_NAS_CURRENT_PLMN_NAME_IND is to contain all available
       names, regardless of display condition. Values: \n
       - 0x00 -- FALSE value) \n
       - 0x01 -- TRUE
  */
}nas_config_plmn_name_ind_reporting_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Configures whether QMI_NAS_CURRENT_PLMN_NAME_IND returns the 
              modem-determined name or all available information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}nas_config_plmn_name_ind_reporting_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_AVOID_SYS_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_AVOID_SYS_USERZONE_V01 = 0x00, /**<  Avoid an idle system if the mobile station has a user zone currently 
       selected \n   */
  NAS_AVOID_SYS_IDLE_V01 = 0x01, /**<  Avoid an idle system \n  */
  NAS_AVOID_SYS_CLR_LIST_V01 = 0x02, /**<  Clear all avoid system lists  */
  NAS_AVOID_SYS_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_avoid_sys_type_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Facilitates avoiding a CDMA system and clearing the avoided 
              systems list. */
typedef struct {

  /* Mandatory */
  /*  Avoid System Information */
  nas_avoid_sys_type_v01 avoid_type;
  /**<   Avoid system type. Values: \n 
      - NAS_AVOID_SYS_USERZONE (0x00) --  Avoid an idle system if the mobile station has a user zone currently 
       selected \n  
      - NAS_AVOID_SYS_IDLE (0x01) --  Avoid an idle system \n 
      - NAS_AVOID_SYS_CLR_LIST (0x02) --  Clear all avoid system lists 
 */
}nas_cdma_avoid_system_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Facilitates avoiding a CDMA system and clearing the avoided 
              systems list. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}nas_cdma_avoid_system_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_cdma_avoid_system_list_req_msg_v01;

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t sid;
  /**<   System ID. */

  uint16_t nid;
  /**<   Network ID. */

  uint16_t mnc;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.
  */

  uint16_t mcc;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.
  */
}nas_cdma_avoid_sys_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the list of previously avoided CDMA systems. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */

  /* Optional */
  /*  Avoided Systems List */
  uint8_t nam1_systems_valid;  /**< Must be set to true if nam1_systems is being passed */
  uint32_t nam1_systems_len;  /**< Must be set to # of elements in nam1_systems */
  nas_cdma_avoid_sys_info_type_v01 nam1_systems[NAS_MAX_CDMA_SYSTEMS_AVOIDED_V01];
}nas_get_cdma_avoid_system_list_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets the HPLMN search timer in the modem. */
typedef struct {

  /* Mandatory */
  /*  HPLMN Search Timer */
  uint32_t timer_value;
  /**<   HPLMN search timer (in minutes). 
       A timer value of 0xFFFFFFFF means use the SIM-defined timer.
  */
}nas_set_hplmn_search_timer_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets the HPLMN search timer in the modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */
}nas_set_hplmn_search_timer_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_hplmn_search_timer_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the HPLMN search timer. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  HPLMN Search Timer */
  uint8_t timer_value_valid;  /**< Must be set to true if timer_value is being passed */
  uint32_t timer_value;
  /**<   HPLMN search timer (in minutes).
  */
}nas_get_hplmn_search_timer_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_subscription_info_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Queries the current subscription information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */

  /* Optional */
  /*  Priority Subscription Info */
  uint8_t is_priority_subs_valid;  /**< Must be set to true if is_priority_subs is being passed */
  nas_is_priority_subs_enum_v01 is_priority_subs;
  /**<  
       Information on whether the subscription is a priority subscription 
       in cases of dual standby. Values: \n
       -0x00 -- Not a priority subscription \n
       -0x01 -- Priority subscription
  */

  /* Optional */
  /*  Active Subscription Info */
  uint8_t is_active_valid;  /**< Must be set to true if is_active is being passed */
  nas_active_subs_info_enum_v01 is_active;
  /**<  
       Information on whether the subscription is active. Values: \n
       -0x00 -- Not active \n
       -0x01 -- Active
  */

  /* Optional */
  /*  Default Data Subscription Info */
  uint8_t is_default_data_subs_valid;  /**< Must be set to true if is_default_data_subs is being passed */
  uint8_t is_default_data_subs;
  /**<  
       Information on whether the subscription is the default data
       subscription in cases of dual standby. Values: \n
       -0x00 -- Not a default data subscription \n
       -0x01 -- Default data subscription
  */

  /* Optional */
  /*  Voice System ID */
  uint8_t voice_system_id_valid;  /**< Must be set to true if voice_system_id is being passed */
  uint32_t voice_system_id;
  /**<   Voice system ID.
  */
}nas_get_subscription_info_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_network_time_req_msg_v01;

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  nas_julian_time_type_v01 universal_time;
  /**<   Universal time. */

  int8_t time_zone;
  /**<   Offset from Universal time, i.e., the difference between local time
       and Universal time, in increments of 15 min (signed value).
  */

  uint8_t daylt_sav_adj;
  /**<   Daylight saving adjustment in hours. Possible values: 0, 1, and 2. This 
       field is ignored if radio_if is NAS_RADIO_IF_CDMA_1XEVDO.
  */

  /*  Radio Interface */
  nas_radio_if_enum_v01 radio_if;
  /**<  
    Radio interface from which the information comes. Values: \n
    - 0x01 -- NAS_RADIO_IF_CDMA_1X     -- 
      cdma2000\textsuperscript{\textregistered} 1X             \n
    - 0x02 -- NAS_RADIO_IF_CDMA_1XEVDO -- 
      cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n
    - 0x04 -- NAS_RADIO_IF_GSM         -- GSM \n
    - 0x05 -- NAS_RADIO_IF_UMTS        -- UMTS \n
    - 0x08 -- NAS_RADIO_IF_LTE         -- LTE \n
    - 0x09 -- NAS_RADIO_IF_TDSCDMA     -- TD-SCDMA
  */
}nas_network_time_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the latest time change reported by the network. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */

  /* Optional */
  /*  3GPP2 Time Information */
  uint8_t nas_3gpp2_time_valid;  /**< Must be set to true if nas_3gpp2_time is being passed */
  nas_network_time_type_v01 nas_3gpp2_time;

  /* Optional */
  /*  3GPP Time Information */
  uint8_t nas_3gpp_time_valid;  /**< Must be set to true if nas_3gpp_time is being passed */
  nas_network_time_type_v01 nas_3gpp_time;
}nas_get_network_time_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_lte_sib16_network_time_req_msg_v01;

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  Year */
  uint16_t year;
  /**<   Year.
  */

  /*  Month */
  uint8_t month;
  /**<   Month. 1 is January and 12 is December.
  */

  /*  Day */
  uint8_t day;
  /**<   Day. Range: 1 to 31.
  */

  /*  Hour */
  uint8_t hour;
  /**<   Hour. Range: 0 to 23.
  */

  /*  Minute */
  uint8_t minute;
  /**<   Minute. Range: 0 to 59.
  */

  /*  Second */
  uint8_t second;
  /**<   Second. Range: 0 to 59.
  */

  /*  MilliSecond */
  uint16_t millisecond;
  /**<   Millisecond. Range: 0 to 999.
  */

  /*  Day of the week */
  uint8_t day_of_week;
  /**<   Day of the week. 0 is Monday and 6 is Sunday.
  */
}nas_lte_sib16_julian_time_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the LTE network time from the UE. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */

  /* Optional */
  /*  LTE SIB16 Coverage Status */
  uint8_t lte_sib16_acquired_valid;  /**< Must be set to true if lte_sib16_acquired is being passed */
  nas_tri_state_boolean_type_v01 lte_sib16_acquired;
  /**<  
 Whether LTE SIB16 is acquired. Values: \n
      - NAS_TRI_FALSE (0) --  Status: FALSE \n 
      - NAS_TRI_TRUE (1) --  Status: TRUE  \n 
      - NAS_TRI_UNKNOWN (2) --  Status: Unknown  
 */

  /* Optional */
  /*  Universal Time */
  uint8_t universal_time_valid;  /**< Must be set to true if universal_time is being passed */
  nas_lte_sib16_julian_time_type_v01 universal_time;

  /* Optional */
  /*  Absolute Time */
  uint8_t abs_time_valid;  /**< Must be set to true if abs_time is being passed */
  uint64_t abs_time;
  /**<   Absolute time in milliseconds since \n
       Jan 6, 1980 00:00:00 hr.
  */

  /* Optional */
  /*  Leap Second */
  uint8_t leap_sec_valid;  /**< Must be set to true if leap_sec is being passed */
  int8_t leap_sec;
  /**<   Leap second.
  */

  /* Optional */
  /*  Time Zone */
  uint8_t time_zone_valid;  /**< Must be set to true if time_zone is being passed */
  int8_t time_zone;
  /**<   Offset from Universal time, i.e., the difference between local time
       and Universal time, in increments of 15 min (signed value).
  */

  /* Optional */
  /*  Daylight Saving Adjustment */
  uint8_t daylt_sav_adj_valid;  /**< Must be set to true if daylt_sav_adj is being passed */
  uint8_t daylt_sav_adj;
  /**<   Daylight saving adjustment in hours. Possible values: 0, 1, and 2.
  */
}nas_get_lte_sib16_network_time_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Indicates an LTE time change reported by the network.  */
typedef struct {

  /* Optional */
  /*  LTE SIB16 Coverage Status */
  uint8_t lte_sib16_acquired_valid;  /**< Must be set to true if lte_sib16_acquired is being passed */
  nas_tri_state_boolean_type_v01 lte_sib16_acquired;
  /**<  
 Whether LTE SIB16 is acquired. Values: \n
      - NAS_TRI_FALSE (0) --  Status: FALSE \n 
      - NAS_TRI_TRUE (1) --  Status: TRUE  \n 
      - NAS_TRI_UNKNOWN (2) --  Status: Unknown  
 */

  /* Optional */
  /*  Universal Time */
  uint8_t universal_time_valid;  /**< Must be set to true if universal_time is being passed */
  nas_lte_sib16_julian_time_type_v01 universal_time;

  /* Optional */
  /*  Absolute Time */
  uint8_t abs_time_valid;  /**< Must be set to true if abs_time is being passed */
  uint64_t abs_time;
  /**<   Absolute time in milliseconds since \n
       Jan 6, 1980 00:00:00 hr.
  */

  /* Optional */
  /*  Leap Second */
  uint8_t leap_sec_valid;  /**< Must be set to true if leap_sec is being passed */
  int8_t leap_sec;
  /**<   Leap second.
  */

  /* Optional */
  /*  Time Zone */
  uint8_t time_zone_valid;  /**< Must be set to true if time_zone is being passed */
  int8_t time_zone;
  /**<   Offset from Universal time, i.e., the difference between local time
       and Universal time, in increments of 15 min (signed value).
  */

  /* Optional */
  /*  Daylight Saving Adjustment */
  uint8_t daylt_sav_adj_valid;  /**< Must be set to true if daylt_sav_adj is being passed */
  uint8_t daylt_sav_adj;
  /**<   Daylight saving adjustment in hours. Possible values: 0, 1, and 2.
  */
}nas_lte_sib16_network_time_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets the priority for LTE bands. */
typedef struct {

  /* Mandatory */
  /*  LTE Band Priority List */
  uint32_t band_priority_list_len;  /**< Must be set to # of elements in band_priority_list */
  nas_active_band_enum_v01 band_priority_list[NAS_LTE_BAND_PRIORITY_LIST_MAX_V01];
  /**<   Priority list for LTE bands 
       (see Table @latexonly\ref{tbl:bandClass}@endlatexonly for details). 
       Values: \n
       - 120 to 153 -- LTE band classes
  */
}nas_set_lte_band_priority_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets the priority for LTE bands. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */
}nas_set_lte_band_priority_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Retrieves the current signal quality at L1 for each MBSFN area. */
typedef struct {

  /* Optional */
  /*  Trace ID */
  uint8_t trace_id_valid;  /**< Must be set to true if trace_id is being passed */
  int16_t trace_id;
  /**<   Trace ID.  Values: \n
       - 0 to 32768 -- Valid trace ID \n
       - -1 -- Trace ID is not used
  */
}nas_get_embms_sig_ext_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  MRB ID */
  uint8_t mrb_id;
  /**<   Multicast radio bearer ID for the session.
  */

  /*  Session ID Valid */
  uint8_t session_id_valid;
  /**<   Indicates whether session ID information is available.
  */

  /*  Session ID */
  uint8_t session_id;
  /**<   Session ID for the session; valid only when session_id_valid is TRUE.
  */

  /*  TMGI Identifier */
  uint8_t tmgi_identifier[NAS_TMGI_IDENTIFIER_LEN_V01];
  /**<   TMGI identifier, consisting of service ID + PLMN ID.
  */
}nas_embms_tmgi_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  MBSFN Area ID */
  uint8_t area_id;
  /**<   Multicast Broadcast Single Frequency Network (MBSFN) area ID. 
    Values: 0 to 255.
  */

  /*  SNR */
  float snr;
  /**<   Average SNR of the serving cell over the last measurement period in
    decibels.
   */

  /*  Excess SNR */
  float excess_snr;
  /**<   Excess SNR of the serving cell over the last measurement period in
       decibels.
   */

  /*  Active TMGI sessions */
  uint32_t tmgi_info_len;  /**< Must be set to # of elements in tmgi_info */
  nas_embms_tmgi_type_v01 tmgi_info[NAS_TMGI_BEARER_INFO_MAX_V01];
  /**<   Array of the active TMGI sessions.
  */
}nas_lte_embms_signal_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Retrieves the current signal quality at L1 for each MBSFN area. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
 Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */

  /* Optional */
  /*  Trace ID */
  uint8_t trace_id_valid;  /**< Must be set to true if trace_id is being passed */
  int16_t trace_id;
  /**<   Trace ID.  Values: \n
       - 0 to 32768 -- Valid trace ID \n
       - -1 -- Trace ID is not used
  */

  /* Optional */
  /*  Signal Quality and TMGI */
  uint8_t snr_and_tmgi_list_valid;  /**< Must be set to true if snr_and_tmgi_list is being passed */
  uint32_t snr_and_tmgi_list_len;  /**< Must be set to # of elements in snr_and_tmgi_list */
  nas_lte_embms_signal_type_v01 snr_and_tmgi_list[NAS_LTE_EMBMS_MAX_MBSFN_AREA_V01];
}nas_get_embms_sig_ext_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SCELL_STATE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_LTE_CPHY_SCELL_STATE_DECONFIGURED_V01 = 0x00, 
  NAS_LTE_CPHY_SCELL_STATE_CONFIGURED_DEACTIVATED_V01 = 0x01, 
  NAS_LTE_CPHY_SCELL_STATE_CONFIGURED_ACTIVATED_V01 = 0x02, 
  NAS_SCELL_STATE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_scell_state_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t pci;
  /**<   Physical cell ID of the Scell. Range: 0 to 503.
  */

  uint16_t freq;
  /**<   Absolute cell's frequency. Range: 0 to 65535.
  */

  nas_scell_state_enum_v01 scell_state;
  /**<   Scell states . Values: \n
       - 0x00 -- NAS_LTE_CPHY_SCELL_STATE_ DECONFIGURED -- 
                 Deconfigured \n
       - 0x01 -- NAS_LTE_CPHY_SCELL_STATE_ CONFIGURED_DEACTIVATED -- 
                 Configured and deactivated \n
       - 0x02 -- NAS_LTE_CPHY_SCELL_STATE_ CONFIGURED_ACTIVATED -- 
                 Configured and activated
  */
}nas_lte_cphy_ca_indicator_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Indicates a carrier aggregation event has occurred.  */
typedef struct {

  /* Mandatory */
  /*  Physical Carrier Aggregation of Scell Indicator Type */
  nas_lte_cphy_ca_indicator_type_v01 cphy_ca;
}nas_lte_cphy_ca_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}nas_get_lte_band_priority_list_req_msg_v01;

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Gets the list of priority LTE bands. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */

  /* Optional */
  /*  LTE Band Priority List */
  uint8_t configured_band_priority_list_valid;  /**< Must be set to true if configured_band_priority_list is being passed */
  uint32_t configured_band_priority_list_len;  /**< Must be set to # of elements in configured_band_priority_list */
  nas_active_band_enum_v01 configured_band_priority_list[NAS_LTE_BAND_PRIORITY_LIST_MAX_V01];
  /**<   List of user-configured LTE bands, ordered by priority. The ordering of this list overrides the ordering 
       of any bands it shares with supported_band_priority_list.
	   (see Table @latexonly\ref{tbl:bandClass}@endlatexonly for details). 
	   Values: \n
	   - 120 to 153 -- LTE band classes
  */

  /* Optional */
  /*  LTE Supported Band Priority List */
  uint8_t supported_band_priority_list_valid;  /**< Must be set to true if supported_band_priority_list is being passed */
  uint32_t supported_band_priority_list_len;  /**< Must be set to # of elements in supported_band_priority_list */
  nas_active_band_enum_v01 supported_band_priority_list[NAS_LTE_BAND_PRIORITY_LIST_MAX_V01];
  /**<   List of LTE bands supported by the device, ordered by priority.
	   (see Table @latexonly\ref{tbl:bandClass}@endlatexonly for details). 
	   Values: \n
	   - 120 to 153 -- LTE band classes
  */
}nas_get_lte_band_priority_list_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t plmn[3];
  /**<   PLMN. */

  uint16_t access_tech;
  /**<   Access Technology identifier. */
}nas_oplmn_entry_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  /*  List ID */
  uint32_t list_id;
  /**<   Unique ID for OPLMN list being sent
  */

  /*  Total List Entries */
  uint32_t total_list_entries;
  /**<   Total number of OPLMN entries in the list 
       e.g. If the list is a total of 500 entries and is being sent in 
	   multiple requests, total_list_entries is set to 500 in all requests.
  */

  uint32_t oplmn_len;  /**< Must be set to # of elements in oplmn */
  nas_oplmn_entry_type_v01 oplmn[NAS_MAX_BUILTIN_OPLMN_ENTRIES_V01];
  /**<   OPLMN List. refer to \hyperref[S7]{[S7]} Section 4.2.53 for coding  of plmn 
     and Access Technology identifier.
  */
}nas_oplmn_list_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Sets the built-in PLMN list. */
typedef struct {

  /* Optional */
  /*  OPLMN List */
  uint8_t oplmn_list_valid;  /**< Must be set to true if oplmn_list is being passed */
  nas_oplmn_list_type_v01 oplmn_list;
  /**<   OPLMN List */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token used to identify the indication sent when the request is complete.
  */
}nas_set_builtin_plmn_list_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Sets the built-in PLMN list. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */
}nas_set_builtin_plmn_list_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Sets the built-in PLMN list. */
typedef struct {

  /* Mandatory */
  /*  Indication Error Code */
  qmi_error_type_v01 error;
  /**<   Error code. Values: 
        0x0000 -- QMI_ERR_NONE                -- Success 
        0x0002 -- QMI_ERR_NO_MEMORY           -- Insufficient memory to store 
                                                 the list 
		0x0003 -- QMI_ERR_INTERNAL            -- Internal error
		0x002D -- QMI_ERR_INVALID_DATA_FORMAT -- Data format invalid								   
  */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;

  /* Optional */
  /*  Received List Entry Count */
  uint8_t received_list_entry_count_valid;  /**< Must be set to true if received_list_entry_count is being passed */
  uint32_t received_list_entry_count;
  /**<   Total number of PLMN entries received so far
  */

  /* Optional */
  /*  Remaining List Entry Count */
  uint8_t remaining_list_entry_count_valid;  /**< Must be set to true if remaining_list_entry_count is being passed */
  uint32_t remaining_list_entry_count;
  /**<   Total number of PLMN entries still expected to complete the list
  */
}nas_set_builtin_plmn_list_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Request Message; Performs the network scan and gives results incrementally. */
typedef struct {

  /* Optional */
  /*  Network Type */
  uint8_t network_type_valid;  /**< Must be set to true if network_type is being passed */
  nas_network_type_mask_type_v01 network_type;
  /**<   Bit mask representing the network type to scan. Values: \n
       - Bit 0 -- GSM \n
       - Bit 1 -- UMTS \n
       - Bit 2 -- LTE \n
       - Bit 3 -- TD-SCDMA \n
       Any combination of the bit positions can be used.  If the mask is
       sent with no bits set, the scan is performed using the currently 
       set preference.
    */

  /* Optional */
  /*  Scan Type */
  uint8_t scan_type_valid;  /**< Must be set to true if scan_type is being passed */
  nas_nw_scan_type_enum_v01 scan_type;
  /**<   Network scan type. Values: \n
       - 0x00 -- NAS_SCAN_TYPE_PLMN -- PLMN (default) \n
       - 0x01 -- NAS_SCAN_TYPE_CSG -- Closed subscriber group
    */

  /* Optional */
  /*  Band Preference */
  uint8_t band_pref_valid;  /**< Must be set to true if band_pref is being passed */
  nas_band_pref_mask_type_v01 band_pref;
  /**<   Bit mask representing the band preference to be scanned.  
       See Table @latexonly\ref{tbl:bandPreference}@endlatexonly 
       for details.   
  */

  /* Optional */
  /*  LTE Band Preference */
  uint8_t lte_band_pref_valid;  /**< Must be set to true if lte_band_pref is being passed */
  lte_band_pref_mask_type_v01 lte_band_pref;
  /**<   Bit mask representing the LTE band preference to be scanned. 
       See Table @latexonly\ref{tbl:lteBandPreference}@endlatexonly 
       for details.  
  */

  /* Optional */
  /*  TDSCDMA Band Preference */
  uint8_t tdscdma_band_pref_valid;  /**< Must be set to true if tdscdma_band_pref is being passed */
  nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref;
  /**<   Bit mask representing the TD-SCDMA band preference to be set. Values: \n
      - NAS_TDSCDMA_BAND_A (0x01) --  TD-SCDMA Band A 
      - NAS_TDSCDMA_BAND_B (0x02) --  TD-SCDMA Band B 
      - NAS_TDSCDMA_BAND_C (0x04) --  TD-SCDMA Band C 
      - NAS_TDSCDMA_BAND_D (0x08) --  TD-SCDMA Band D 
      - NAS_TDSCDMA_BAND_E (0x10) --  TD-SCDMA Band E 
      - NAS_TDSCDMA_BAND_F (0x20) --  TD-SCDMA Band F 

 \vspace{3pt}
 All other bits are reserved and must be set to 0.
 */
}nas_perform_incremental_network_scan_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Response Message; Performs the network scan and gives results incrementally. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. 
 Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */
}nas_perform_incremental_network_scan_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup nas_qmi_enums
    @{
  */
typedef enum {
  NAS_SCAN_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_SCAN_STATUS_COMPLETE_V01 = 0x00, /**<  Network scan successful and complete \n  */
  NAS_SCAN_STATUS_PARTIAL_V01 = 0x01, /**<  Network scan partial \n  */
  NAS_SCAN_STATUS_ABORT_V01 = 0x02, /**<  Network scan was aborted \n  */
  NAS_SCAN_STATUS_REJ_IN_RLF_V01 = 0x03, /**<   Network scan did not complete due 
        to a radio link failure recovery in progress \n  */
  NAS_SCAN_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_scan_status_enum_v01;
/**
    @}
  */

/** @addtogroup nas_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mobile_country_code;
  /**<   A 16-bit integer representation of MCC. Range: 0 to 999.
  */

  uint16_t mobile_network_code;
  /**<   A 16-bit integer representation of MNC. Range: 0 to 999.
  */

  uint8_t network_status;
  /**<   Status of the network identified by MCC and MNC preceding it.
       The status is encoded in a bitmapped value as follows: \n
       Bits 0-1 -- QMI_NAS_NETWORK_IN_USE_ STATUS_BITS    -- In-use status       \n
       - 0 -- QMI_NAS_NETWORK_IN_USE_STATUS_ UNKNOWN          -- Unknown         \n
       - 1 -- QMI_NAS_NETWORK_IN_USE_STATUS_ CURRENT_SERVING  -- Current serving \n
       - 2 -- QMI_NAS_NETWORK_IN_USE_STATUS_ AVAILABLE        -- Available
       
       Bits 2-3 -- QMI_NAS_NETWORK_ROAMING_ STATUS_BITS   -- Roaming status      \n
       - 0 -- QMI_NAS_NETWORK_ROAMING_ STATUS_UNKNOWN         -- Unknown         \n
       - 1 -- QMI_NAS_NETWORK_ROAMING_ STATUS_HOME            -- Home            \n
       - 2 -- QMI_NAS_NETWORK_ROAMING_ STATUS_ROAM            -- Roam

       Bits 4-5 -- QMI_NAS_NETWORK_FORBIDDEN_ STATUS_BITS -- Forbidden status    \n
       - 0 -- QMI_NAS_NETWORK_FORBIDDEN_ STATUS_UNKNOWN       -- Unknown         \n
       - 1 -- QMI_NAS_NETWORK_FORBIDDEN_ STATUS_FORBIDDEN     -- Forbidden       \n
       - 2 -- QMI_NAS_NETWORK_FORBIDDEN_ STATUS_NOT_FORBIDDEN -- Not forbidden

       Bits 6-7 -- QMI_NAS_NETWORK_PREFERRED_ STATUS_BITS -- Preferred status    \n
       - 0 -- QMI_NAS_NETWORK_PREFERRED_ STATUS_UNKNOWN       -- Unknown         \n
       - 1 -- QMI_NAS_NETWORK_PREFERRED_ STATUS_PREFERRED     -- Preferred       \n
       - 2 -- QMI_NAS_NETWORK_PREFERRED_ STATUS_NOT_PREFERRED -- Not preferred
  */

  uint8_t rat;
  /**<   Radio Access Technology. Values: \n
       - 0x04 -- GERAN \n
       - 0x05 -- UMTS \n
       - 0x08 -- LTE \n
       - 0x09 -- TD-SCDMA 
  */

  /*  MNC PCS Digit Include Status */
  uint8_t mnc_includes_pcs_digit;
  /**<   This field is used to interpret the length of the corresponding
       MNC reported in the TLVs (in this table) with an MNC or 
       mobile_network_code field. Values: \n

       - TRUE  -- MNC is a three-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 090  \n
       - FALSE -- MNC is a two-digit value; e.g., a reported value of 
                  90 corresponds to an MNC value of 90
  */

  char network_description[NAS_NETWORK_DESCRIPTION_MAX_V01 + 1];
  /**<   An optional string containing the network name or description.
  */
}nas_3gpp_network_scan_result_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup nas_qmi_messages
    @{
  */
/** Indication Message; Performs the network scan and gives results incrementally. */
typedef struct {

  /* Mandatory */
  /*  Network Scan Status */
  nas_scan_status_enum_v01 scan_status;
  /**<   Indicates the status of the network scan. Values: \n
      - NAS_SCAN_STATUS_COMPLETE (0x00) --  Network scan successful and complete \n 
      - NAS_SCAN_STATUS_PARTIAL (0x01) --  Network scan partial \n 
      - NAS_SCAN_STATUS_ABORT (0x02) --  Network scan was aborted \n 
      - NAS_SCAN_STATUS_REJ_IN_RLF (0x03) --   Network scan did not complete due 
        to a radio link failure recovery in progress \n 
 */

  /* Optional */
  /*  3gpp Network Scan Information */
  uint8_t nas_network_scan_info_valid;  /**< Must be set to true if nas_network_scan_info is being passed */
  uint32_t nas_network_scan_info_len;  /**< Must be set to # of elements in nas_network_scan_info */
  nas_3gpp_network_scan_result_type_v01 nas_network_scan_info[NAS_3GPP_NETWORK_INFO_LIST_MAX_V01];

  /* Optional */
  /*  CSG Information */
  uint8_t csg_info_valid;  /**< Must be set to true if csg_info is being passed */
  uint32_t csg_info_len;  /**< Must be set to # of elements in csg_info */
  nas_csg_nw_info_type_v01 csg_info[NAS_3GPP_NETWORK_INFO_LIST_MAX_V01];
}nas_perform_incremental_network_scan_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup nas_qmi_msg_ids
    @{
  */
#define QMI_NAS_RESET_REQ_MSG_V01 0x0000
#define QMI_NAS_RESET_RESP_MSG_V01 0x0000
#define QMI_NAS_ABORT_REQ_MSG_V01 0x0001
#define QMI_NAS_ABORT_RESP_MSG_V01 0x0001
#define QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01 0x0002
#define QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01 0x0002
#define QMI_NAS_EVENT_REPORT_IND_MSG_V01 0x0002
#define QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01 0x0003
#define QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01 0x0003
#define QMI_NAS_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_NAS_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_NAS_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_NAS_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_NAS_GET_SIGNAL_STRENGTH_REQ_MSG_V01 0x0020
#define QMI_NAS_GET_SIGNAL_STRENGTH_RESP_MSG_V01 0x0020
#define QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01 0x0021
#define QMI_NAS_PERFORM_NETWORK_SCAN_RESP_MSG_V01 0x0021
#define QMI_NAS_INITIATE_NETWORK_REGISTER_REQ_MSG_V01 0x0022
#define QMI_NAS_INITIATE_NETWORK_REGISTER_RESP_MSG_V01 0x0022
#define QMI_NAS_INITIATE_ATTACH_REQ_MSG_V01 0x0023
#define QMI_NAS_INITIATE_ATTACH_RESP_MSG_V01 0x0023
#define QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01 0x0024
#define QMI_NAS_GET_SERVING_SYSTEM_RESP_MSG_V01 0x0024
#define QMI_NAS_SERVING_SYSTEM_IND_MSG_V01 0x0024
#define QMI_NAS_GET_HOME_NETWORK_REQ_MSG_V01 0x0025
#define QMI_NAS_GET_HOME_NETWORK_RESP_MSG_V01 0x0025
#define QMI_NAS_GET_PREFERRED_NETWORKS_REQ_MSG_V01 0x0026
#define QMI_NAS_GET_PREFERRED_NETWORKS_RESP_MSG_V01 0x0026
#define QMI_NAS_SET_PREFERRED_NETWORKS_REQ_MSG_V01 0x0027
#define QMI_NAS_SET_PREFERRED_NETWORKS_RESP_MSG_V01 0x0027
#define QMI_NAS_GET_FORBIDDEN_NETWORKS_REQ_MSG_V01 0x0028
#define QMI_NAS_GET_FORBIDDEN_NETWORKS_RESP_MSG_V01 0x0028
#define QMI_NAS_SET_FORBIDDEN_NETWORKS_REQ_MSG_V01 0x0029
#define QMI_NAS_SET_FORBIDDEN_NETWORKS_RESP_MSG_V01 0x0029
#define QMI_NAS_SET_TECHNOLOGY_PREFERENCE_REQ_V01 0x002A
#define QMI_NAS_SET_TECHNOLOGY_PREFERENCE_RESP_V01 0x002A
#define QMI_NAS_GET_TECHNOLOGY_PREFERENCE_REQ_V01 0x002B
#define QMI_NAS_GET_TECHNOLOGY_PREFERENCE_RESP_V01 0x002B
#define QMI_NAS_GET_ACCOLC_REQ_MSG_V01 0x002C
#define QMI_NAS_GET_ACCOLC_RESP_MSG_V01 0x002C
#define QMI_NAS_SET_ACCOLC_REQ_MSG_V01 0x002D
#define QMI_NAS_SET_ACCOLC_RESP_MSG_V01 0x002D
#define QMI_NAS_GET_NETWORK_SYSTEM_PREFERENCE_REQ_V01 0x002E
#define QMI_NAS_GET_NETWORK_SYSTEM_PREFERENCE_RESP_V01 0x002E
#define QMI_NAS_GET_DEVICE_CONFIG_REQ_MSG_V01 0x002F
#define QMI_NAS_GET_DEVICE_CONFIG_RESP_MSG_V01 0x002F
#define QMI_NAS_SET_DEVICE_CONFIG_REQ_MSG_V01 0x0030
#define QMI_NAS_SET_DEVICE_CONFIG_RESP_MSG_V01 0x0030
#define QMI_NAS_GET_RF_BAND_INFO_REQ_MSG_V01 0x0031
#define QMI_NAS_GET_RF_BAND_INFO_RESP_MSG_V01 0x0031
#define QMI_NAS_GET_AN_AAA_STATUS_REQ_MSG_V01 0x0032
#define QMI_NAS_GET_AN_AAA_STATUS_RESP_MSG_V01 0x0032
#define QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01 0x0033
#define QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01 0x0033
#define QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01 0x0034
#define QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01 0x0034
#define QMI_NAS_SYSTEM_SELECTION_PREFERENCE_IND_MSG_V01 0x0034
#define QMI_NAS_SET_DDTM_PREFERENCE_REQ_MSG_V01 0x0037
#define QMI_NAS_SET_DDTM_PREFERENCE_RESP_MSG_V01 0x0037
#define QMI_NAS_DDTM_IND_MSG_V01 0x0038
#define QMI_NAS_GET_OPERATOR_NAME_DATA_REQ_MSG_V01 0x0039
#define QMI_NAS_GET_OPERATOR_NAME_DATA_RESP_MSG_V01 0x0039
#define QMI_NAS_OPERATOR_NAME_DATA_IND_MSG_V01 0x003A
#define QMI_NAS_GET_CSP_PLMN_MODE_BIT_REQ_MSG_V01 0x003B
#define QMI_NAS_GET_CSP_PLMN_MODE_BIT_RESP_MSG_V01 0x003B
#define QMI_NAS_CSP_PLMN_MODE_BIT_IND_MSG_V01 0x003C
#define QMI_NAS_UPDATE_AKEY_REQ_MSG_V01 0x003D
#define QMI_NAS_UPDATE_AKEY_RESP_MSG_V01 0x003D
#define QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01 0x003E
#define QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_RESP_MSG_V01 0x003E
#define QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01 0x003F
#define QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO_RESP_MSG_V01 0x003F
#define QMI_NAS_GET_MOB_CAI_REV_REQ_MSG_V01 0x0040
#define QMI_NAS_GET_MOB_CAI_REV_RESP_MSG_V01 0x0040
#define QMI_NAS_GET_RTRE_CONFIG_REQ_MSG_V01 0x0041
#define QMI_NAS_GET_RTRE_CONFIG_RESP_MSG_V01 0x0041
#define QMI_NAS_SET_RTRE_CONFIG_REQ_MSG_V01 0x0042
#define QMI_NAS_SET_RTRE_CONFIG_RESP_MSG_V01 0x0042
#define QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01 0x0043
#define QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01 0x0043
#define QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01 0x0044
#define QMI_NAS_GET_PLMN_NAME_RESP_MSG_V01 0x0044
#define QMI_NAS_BIND_SUBSCRIPTION_REQ_MSG_V01 0x0045
#define QMI_NAS_BIND_SUBSCRIPTION_RESP_MSG_V01 0x0045
#define QMI_NAS_MANAGED_ROAMING_IND_MSG_V01 0x0046
#define QMI_NAS_DUAL_STANDBY_PREF_IND_MSG_V01 0x0047
#define QMI_NAS_SUBSCRIPTION_INFO_IND_MSG_V01 0x0048
#define QMI_NAS_GET_MODE_PREF_REQ_MSG_V01 0x0049
#define QMI_NAS_GET_MODE_PREF_RESP_MSG_V01 0x0049
#define QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01 0x004B
#define QMI_NAS_DUAL_STANDBY_PREF_RESP_MSG_V01 0x004B
#define QMI_NAS_NETWORK_TIME_IND_MSG_V01 0x004C
#define QMI_NAS_GET_SYS_INFO_REQ_MSG_V01 0x004D
#define QMI_NAS_GET_SYS_INFO_RESP_MSG_V01 0x004D
#define QMI_NAS_SYS_INFO_IND_MSG_V01 0x004E
#define QMI_NAS_GET_SIG_INFO_REQ_MSG_V01 0x004F
#define QMI_NAS_GET_SIG_INFO_RESP_MSG_V01 0x004F
#define QMI_NAS_CONFIG_SIG_INFO_REQ_MSG_V01 0x0050
#define QMI_NAS_CONFIG_SIG_INFO_RESP_MSG_V01 0x0050
#define QMI_NAS_SIG_INFO_IND_MSG_V01 0x0051
#define QMI_NAS_GET_ERR_RATE_REQ_MSG_V01 0x0052
#define QMI_NAS_GET_ERR_RATE_RESP_MSG_V01 0x0052
#define QMI_NAS_ERR_RATE_IND_MSG_V01 0x0053
#define QMI_NAS_HDR_SESSION_CLOSE_IND_MSG_V01 0x0054
#define QMI_NAS_HDR_UATI_UPDATE_IND_MSG_V01 0x0055
#define QMI_NAS_GET_HDR_SUBTYPE_REQ_MSG_V01 0x0056
#define QMI_NAS_GET_HDR_SUBTYPE_RESP_MSG_V01 0x0056
#define QMI_NAS_GET_HDR_COLOR_CODE_REQ_MSG_V01 0x0057
#define QMI_NAS_GET_HDR_COLOR_CODE_RESP_MSG_V01 0x0057
#define QMI_NAS_GET_CURRENT_ACQ_SYS_MODE_REQ_MSG_V01 0x0058
#define QMI_NAS_GET_CURRENT_ACQ_SYS_MODE_RESP_MSG_V01 0x0058
#define QMI_NAS_SET_RX_DIVERSITY_REQ_MSG_V01 0x0059
#define QMI_NAS_SET_RX_DIVERSITY_RESP_MSG_V01 0x0059
#define QMI_NAS_GET_TX_RX_INFO_REQ_MSG_V01 0x005A
#define QMI_NAS_GET_TX_RX_INFO_RESP_MSG_V01 0x005A
#define QMI_NAS_UPDATE_AKEY_EXT_REQ_MSG_V01 0x005B
#define QMI_NAS_UPDATE_AKEY_EXT_RESP_V01 0x005B
#define QMI_NAS_GET_DUAL_STANDBY_PREF_REQ_MSG_V01 0x005C
#define QMI_NAS_GET_DUAL_STANDBY_PREF_RESP_MSG_V01 0x005C
#define QMI_NAS_DETACH_LTE_REQ_MSG_V01 0x005D
#define QMI_NAS_DETACH_LTE_RESP_MSG_V01 0x005D
#define QMI_NAS_BLOCK_LTE_PLMN_REQ_MSG_V01 0x005E
#define QMI_NAS_BLOCK_LTE_PLMN_RESP_MSG_V01 0x005E
#define QMI_NAS_UNBLOCK_LTE_PLMN_REQ_MSG_V01 0x005F
#define QMI_NAS_UNBLOCK_LTE_PLMN_RESP_MSG_V01 0x005F
#define QMI_NAS_RESET_LTE_PLMN_BLOCKING_REQ_MSG_V01 0x0060
#define QMI_NAS_RESET_LTE_PLMN_BLOCKING_RESP_MSG_V01 0x0060
#define QMI_NAS_CURRENT_PLMN_NAME_IND_V01 0x0061
#define QMI_NAS_CONFIG_EMBMS_REQ_MSG_V01 0x0062
#define QMI_NAS_CONFIG_EMBMS_RESP_MSG_V01 0x0062
#define QMI_NAS_GET_EMBMS_STATUS_REQ_MSG_V01 0x0063
#define QMI_NAS_GET_EMBMS_STATUS_RESP_MSG_V01 0x0063
#define QMI_NAS_EMBMS_STATUS_IND_V01 0x0064
#define QMI_NAS_GET_CDMA_POSITION_INFO_REQ_MSG_V01 0x0065
#define QMI_NAS_GET_CDMA_POSITION_INFO_RESP_MSG_V01 0x0065
#define QMI_NAS_RF_BAND_INFO_IND_V01 0x0066
#define QMI_NAS_FORCE_NETWORK_SEARCH_REQ_MSG_V01 0x0067
#define QMI_NAS_FORCE_NETWORK_SEARCH_RESP_MSG_V01 0x0067
#define QMI_NAS_NETWORK_REJECT_IND_V01 0x0068
#define QMI_NAS_GET_MANAGED_ROAMING_CONFIG_REQ_MSG_V01 0x0069
#define QMI_NAS_GET_MANAGED_ROAMING_CONFIG_RESP_MSG_V01 0x0069
#define QMI_NAS_RTRE_CONFIG_IND_V01 0x006A
#define QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01 0x006B
#define QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01 0x006B
#define QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01 0x006C
#define QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01 0x006C
#define QMI_NAS_GET_TDS_CELL_AND_POSITION_INFO_REQ_MSG_V01 0x006D
#define QMI_NAS_GET_TDS_CELL_AND_POSITION_INFO_RESP_MSG_V01 0x006D
#define QMI_NAS_SET_HPLMN_IRAT_SEARCH_TIMER_REQ_MSG_V01 0x006E
#define QMI_NAS_SET_HPLMN_IRAT_SEARCH_TIMER_RESP_MSG_V01 0x006E
#define QMI_NAS_GET_EMBMS_SIG_REQ_MSG_V01 0x006F
#define QMI_NAS_GET_EMBMS_SIG_RESP_MSG_V01 0x006F
#define QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_REQ_MSG_V01 0x0070
#define QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_RESP_MSG_V01 0x0070
#define QMI_NAS_GET_SYS_INFO_IND_REPORTING_LIMIT_REQ_MSG_V01 0x0071
#define QMI_NAS_GET_SYS_INFO_IND_REPORTING_LIMIT_RESP_MSG_V01 0x0071
#define QMI_NAS_UPDATE_IMS_STATUS_REQ_MSG_V01 0x0072
#define QMI_NAS_UPDATE_IMS_STATUS_RESP_MSG_V01 0x0072
#define QMI_NAS_GET_IMS_PREF_STATUS_REQ_MSG_V01 0x0073
#define QMI_NAS_GET_IMS_PREF_STATUS_RESP_MSG_V01 0x0073
#define QMI_NAS_IMS_PREF_STATUS_IND_V01 0x0074
#define QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01 0x0075
#define QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01 0x0075
#define QMI_NAS_CDMA_AVOID_SYSTEM_REQ_MSG_V01 0x0076
#define QMI_NAS_CDMA_AVOID_SYSTEM_RESP_MSG_V01 0x0076
#define QMI_NAS_GET_CDMA_AVOID_SYSTEM_LIST_REQ_MSG_V01 0x0077
#define QMI_NAS_GET_CDMA_AVOID_SYSTEM_LIST_RESP_MSG_V01 0x0077
#define QMI_NAS_SET_HPLMN_SEARCH_TIME_REQ_MSG_V01 0x0078
#define QMI_NAS_SET_HPLMN_SEARCH_TIME_RESP_MSG_V01 0x0078
#define QMI_NAS_GET_HPLMN_SEARCH_TIME_REQ_MSG_V01 0x0079
#define QMI_NAS_GET_HPLMN_SEARCH_TIME_RESP_MSG_V01 0x0079
#define QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01 0x007C
#define QMI_NAS_GET_SUBSCRIPTION_INFO_RESP_MSG_V01 0x007C
#define QMI_NAS_GET_NETWORK_TIME_REQ_MSG_V01 0x007D
#define QMI_NAS_GET_NETWORK_TIME_RESP_MSG_V01 0x007D
#define QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_REQ_MSG_V01 0x007E
#define QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_RESP_MSG_V01 0x007E
#define QMI_NAS_LTE_SIB16_NETWORK_TIME_IND_V01 0x007F
#define QMI_NAS_SET_LTE_BAND_PRIORITY_REQ_MSG_V01 0x0080
#define QMI_NAS_SET_LTE_BAND_PRIORITY_RESP_MSG_V01 0x0080
#define QMI_NAS_GET_EMBMS_SIG_EXT_REQ_MSG_V01 0x0081
#define QMI_NAS_GET_EMBMS_SIG_EXT_RESP_MSG_V01 0x0081
#define QMI_NAS_LTE_CPHY_CA_IND_V01 0x0082
#define QMI_NAS_GET_LTE_BAND_PRIORITY_LIST_REQ_MSG_V01 0x0083
#define QMI_NAS_GET_LTE_BAND_PRIORITY_LIST_RESP_MSG_V01 0x0083
#define QMI_NAS_SET_BUILTIN_PLMN_LIST_REQ_MSG_V01 0x0084
#define QMI_NAS_SET_BUILTIN_PLMN_LIST_RESP_MSG_V01 0x0084
#define QMI_NAS_SET_BUILTIN_PLMN_LIST_IND_MSG_V01 0x0084
#define QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_REQ_MSG_V01 0x0085
#define QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_RESP_MSG_V01 0x0085
#define QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_IND_MSG_V01 0x0085
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro nas_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type nas_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define nas_get_service_object_v01( ) \
          nas_get_service_object_internal_v01( \
            NAS_V01_IDL_MAJOR_VERS, NAS_V01_IDL_MINOR_VERS, \
            NAS_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

