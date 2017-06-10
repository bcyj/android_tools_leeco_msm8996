#ifndef CXM_SERVICE_H
#define CXM_SERVICE_H
/**
  @file coexistence_manager_v01.h
  
  @brief This is the public header file which defines the cxm service Data structures.

  This header file defines the types and structures that were defined in 
  cxm. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header: //source/qcom/qct/modem/arch/qmi_cxm/coexistence_manager_v01.h#3 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 2.8
   It was generated on: Wed Oct 26 2011
   From IDL File: coexistence_manager_v01.idl */

/** @defgroup cxm_qmi_consts Constant values defined in the IDL */
/** @defgroup cxm_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup cxm_qmi_enums Enumerated types used in QMI messages */
/** @defgroup cxm_qmi_messages Structures sent as QMI messages */
/** @defgroup cxm_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup cxm_qmi_accessor Accessor for QMI service object */
/** @defgroup cxm_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup cxm_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define CXM_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define CXM_V01_IDL_MINOR_VERS 0x00
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define CXM_V01_IDL_TOOL_VERS 0x02
/** Maximum Defined Message ID */
#define CXM_V01_MAX_MESSAGE_ID 0x0027;
/** 
    @} 
  */


/** @addtogroup cxm_qmi_consts 
    @{ 
  */
#define CXM_LTE_ML1_FRAME_TIMING_REPORT_LIST_MAX_V01 6
/**
    @}
  */

/** @addtogroup cxm_qmi_enums
    @{
  */
typedef enum {
  CXM_LTE_BANDWIDTH_IDX_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CXM_LTE_BW_IDX_NRB_6_V01 = 0, /**<  1.4MHz bandwidth  */
  CXM_LTE_BW_IDX_NRB_15_V01 = 1, /**<  3MHz bandwidth  */
  CXM_LTE_BW_IDX_NRB_25_V01 = 2, /**<  5MHz bandwidth  */
  CXM_LTE_BW_IDX_NRB_50_V01 = 3, /**<  10MHz bandwidth  */
  CXM_LTE_BW_IDX_NRB_75_V01 = 4, /**<  15MHz bandwidth 
 20MHz bandwidth  */
  CXM_LTE_BW_IDX_NRB_100_V01 = 5, 
  CXM_LTE_BANDWIDTH_IDX_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cxm_lte_bandwidth_idx_e_v01;
/**
    @}
  */

/** @addtogroup cxm_qmi_enums
    @{
  */
typedef enum {
  CXM_LTE_L1_FRAME_STRUCT_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CXM_LTE_L1_FRAME_STRUCTURE_FS1_V01 = 0, /**<  Frame structure 1 (generic frame structure) 
 Frame structure 2 (alternative frame structure)  */
  CXM_LTE_L1_FRAME_STRUCTURE_FS2_V01 = 1, 
  CXM_LTE_L1_FRAME_STRUCT_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cxm_lte_l1_frame_struct_e_v01;
/**
    @}
  */

/** @addtogroup cxm_qmi_enums
    @{
  */
typedef enum {
  CXM_LTE_L1_TDD_UL_DL_CFG_INDEX_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CXM_LTE_L1_TDD_UL_DL_CFG_0_V01 = 0, 
  CXM_LTE_L1_TDD_UL_DL_CFG_1_V01 = 1, 
  CXM_LTE_L1_TDD_UL_DL_CFG_2_V01 = 2, 
  CXM_LTE_L1_TDD_UL_DL_CFG_3_V01 = 3, 
  CXM_LTE_L1_TDD_UL_DL_CFG_4_V01 = 4, 
  CXM_LTE_L1_TDD_UL_DL_CFG_5_V01 = 5, 
  CXM_LTE_L1_TDD_UL_DL_CFG_6_V01 = 6, 
  CXM_LTE_L1_TDD_UL_DL_CFG_UNKNOWN_WITH_UNKNOWN_10MS_FRAME_V01 = 7, 
  CXM_LTE_L1_TDD_UL_DL_CFG_UNKONWN_WITH_KNOWN_10MS_FRAME_V01 = 8, 
  CXM_LTE_L1_TDD_UL_DL_CFG_INDEX_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cxm_lte_l1_tdd_ul_dl_cfg_index_e_v01;
/**
    @}
  */

/** @addtogroup cxm_qmi_enums
    @{
  */
typedef enum {
  CXM_LTE_ML1_COEX_CHANNEL_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CXM_LTE_ML1_COEX_RESERVED_V01 = 0, 
  CXM_LTE_ML1_COEX_PUCCH_V01 = 1, 
  CXM_LTE_ML1_COEX_PUSCH_V01 = 2, 
  CXM_LTE_ML1_COEX_PRACH_V01 = 3, 
  CXM_LTE_ML1_COEX_SRS_V01 = 4, 
  CXM_LTE_ML1_COEX_SRS_PUCCH_V01 = 5, 
  CXM_LTE_ML1_COEX_SRS_PUSCH_V01 = 6, 
  CXM_LTE_ML1_COEX_CHANNEL_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cxm_lte_ml1_coex_channel_e_v01;
/**
    @}
  */

/** @addtogroup cxm_qmi_enums
    @{
  */
typedef enum {
  CXM_LTE_L1_TDD_SPECIAL_SUBFRAME_PATTERN_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CXM_LTE_L1_TDD_SPECIAL_SUBFRAME_PATTERN_0_V01 = 0, 
  CXM_LTE_L1_TDD_SPECIAL_SUBFRAME_PATTERN_1_V01 = 1, 
  CXM_LTE_L1_TDD_SPECIAL_SUBFRAME_PATTERN_2_V01 = 2, 
  CXM_LTE_L1_TDD_SPECIAL_SUBFRAME_PATTERN_3_V01 = 3, 
  CXM_LTE_L1_TDD_SPECIAL_SUBFRAME_PATTERN_4_V01 = 4, 
  CXM_LTE_L1_TDD_SPECIAL_SUBFRAME_PATTERN_5_V01 = 5, 
  CXM_LTE_L1_TDD_SPECIAL_SUBFRAME_PATTERN_6_V01 = 6, 
  CXM_LTE_L1_TDD_SPECIAL_SUBFRAME_PATTERN_7_V01 = 7, 
  CXM_LTE_L1_TDD_SPECIAL_SUBFRAME_PATTERN_8_V01 = 8, 
  CXM_LTE_L1_TDD_SPECIAL_SUBFRAME_PATTERN_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cxm_lte_l1_tdd_special_subframe_pattern_e_v01;
/**
    @}
  */

/** @addtogroup cxm_qmi_enums
    @{
  */
typedef enum {
  CXM_LTE_L1_CP_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CXM_LTE_L1_CP_MODE_NORMAL_V01 = 0, /**<  Normal CP  */
  CXM_LTE_L1_CP_MODE_EXTENDED_V01 = 1, /**<  Extended CP 
 Long CP for MBSFN  */
  CXM_LTE_L1_CP_MODE_EXTENDED_MBSFN_V01 = 2, 
  CXM_LTE_L1_CP_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cxm_lte_l1_cp_e_v01;
/**
    @}
  */

/** @addtogroup cxm_qmi_enums
    @{
  */
typedef enum {
  CXM_LTE_CPHY_UL_CYCLIC_PREFIX_LENGTH_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CXM_LTE_CPHY_UL_CYCLIC_PREFIX_LENGTH_1_V01 = 0, 
  CXM_LTE_CPHY_UL_CYCLIC_PREFIX_LENGTH_2_V01 = 1, 
  CXM_LTE_CPHY_UL_CYCLIC_PREFIX_LENGTH_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cxm_lte_cphy_ul_cyclic_prefix_length_e_v01;
/**
    @}
  */

/** @addtogroup cxm_qmi_enums
    @{
  */
typedef enum {
  CXM_LTE_ML1_COEX_REPORT_TYPE_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CXM_LTE_ML1_COEX_PERIODIC_REPORT_V01 = 0, /**<  Periodic Report  */
  CXM_LTE_ML1_COEX_HO_REPORT_V01 = 1, /**<  Handoff report 
 TA report  */
  CXM_LTE_ML1_COEX_TA_REPORT_V01 = 2, 
  CXM_LTE_ML1_COEX_REPORT_TYPE_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cxm_lte_ml1_coex_report_type_e_v01;
/**
    @}
  */

/** @addtogroup cxm_qmi_messages
    @{
  */
/** Response Message;  */
typedef struct {

  /* Mandatory */
  /*  Placeholder for responses that are never actually sent by the service
 Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}cxm_unused_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup cxm_qmi_messages
    @{
  */
/** Request Message; This message activates the coexistence service. */
typedef struct {

  /* Mandatory */
  /*  Data Plane Enable */
  uint8_t data_plane_enable;
  /**<   Values: \n
       - 0 -- Disable Data Plane \n
       - 1 -- Enable Data Plane
   */
}cxm_activate_req_msg_v01;  /* Message */
/**
    @}
  */

/*
 * cxm_deactivate_req_msg is empty
 * typedef struct {
 * }cxm_deactivate_req_msg_v01;
 */

/*
 * cxm_state_req_msg is empty
 * typedef struct {
 * }cxm_state_req_msg_v01;
 */

/** @addtogroup cxm_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t is_connected;
  /**<   Values: \n
       - 0 -- Not Connected \n
       - 1 -- Connected
   */

  uint8_t is_high_priority;
  /**<   Values: \n
       - 0 -- Low Priority \n
       - 1 -- High Priority
   */

  uint16_t mask;
  /**<   Bit-mask where each bit indicates whether one of the following fields has \n
       been populated with a valid value.  Values: \n
       0x0001 - start_time \n
       0x0002 - end_time \n
       0x0004 - dl_earfcn \n
       0x0008 - ul_earfcn \n
       0x0010 - dl_bandwidth \n
       0x0020 - ul_bandwidth \n
       0x0040 - frame_structure \n
       0x0080 - tdd_config \n
       0x0100 - SSP
       0x0200   - DL CP
       0x0400   - UL CP 
   */

  uint32_t start_time;
  /**<   Event Start time   */

  uint32_t end_time;
  /**<   Event End time   */

  uint16_t dl_earfcn;
  /**<   DL EARFCN  */

  uint16_t ul_earfcn;
  /**<   UL EARFCN  */

  cxm_lte_bandwidth_idx_e_v01 dl_bandwidth;
  /**<   DL Bandwidth 
      Values: \n
      0 = 1.4MHz bandwidth (CXM_LTE_BW_IDX_NRB_6) \n
      1 = 3MHz bandwidth (CXM_LTE_BW_IDX_NRB_15) \n
      2 = 5MHz bandwidth (CXM_LTE_BW_IDX_NRB_25) \n
      3 = 10MHz bandwidth (CXM_LTE_BW_IDX_NRB_50) \n
      4 = 15MHz bandwidth (CXM_LTE_BW_IDX_NRB_75) \n
      5 = 20MHz bandwidth (CXM_LTE_BW_IDX_NRB_100)
   */

  cxm_lte_bandwidth_idx_e_v01 ul_bandwidth;
  /**<   UL Bandwidth 
      Values: \n
      0 = 1.4MHz bandwidth (CXM_LTE_BW_IDX_NRB_6) \n
      1 = 3MHz bandwidth (CXM_LTE_BW_IDX_NRB_15) \n
      2 = 5MHz bandwidth (CXM_LTE_BW_IDX_NRB_25) \n
      3 = 10MHz bandwidth (CXM_LTE_BW_IDX_NRB_50) \n
      4 = 15MHz bandwidth (CXM_LTE_BW_IDX_NRB_75) \n
      5 = 20MHz bandwidth (CXM_LTE_BW_IDX_NRB_100)
   */

  cxm_lte_l1_frame_struct_e_v01 frame_structure;
  /**<   Frame Structure FDD or TDD as per 36.201 Section 4.2.1 
       values:
       0 = Frame structure 1 (generic frame structure, CXM_LTE_L1_FRAME_STRUCTURE_FS1)
       1 = Frame structure 2 (alternative frame structure, CXM_LTE_L1_FRAME_STRUCTURE_FS2)
   */

  cxm_lte_l1_tdd_ul_dl_cfg_index_e_v01 tdd_config;
  /**<   TDD configuration as per 36.211, table 4.2.2, valid only for TDD frame structure.
       Values: \n
       0-6 = UL/DL Config 0-6 (CXM_LTE_L1_TDD_UL_DL_CFG_0 - CXM_LTE_L1_TDD_UL_DL_CFG_6) \n
       7 = Unknown Config, Unknown 10ms frame (CXM_LTE_L1_TDD_UL_DL_CFG_UNKNOWN_WITH_UNKNOWN_10MS_FRAME) \n
       8 = Unknown Config, Known 10ms frame (CXM_LTE_L1_TDD_UL_DL_CFG_UNKONWN_WITH_KNOWN_10MS_FRAME)
   */

  cxm_lte_l1_tdd_special_subframe_pattern_e_v01 ssp;

  cxm_lte_l1_cp_e_v01 dl_cp;

  cxm_lte_cphy_ul_cyclic_prefix_length_e_v01 ul_cp;
}cxm_lte_ml1_state_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup cxm_qmi_messages
    @{
  */
/** Indication Message; This message retrieves the current state of the CxM service. */
typedef struct {

  /* Mandatory */
  /*  LTE ML1 State */
  cxm_lte_ml1_state_s_v01 lte_ml1_state;
}cxm_state_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup cxm_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t snr_is_valid;
  /**<   SNR validity 
       Values: \n
       0 = SNR Invalid \n
       1 = SNR Valid
   */

  uint8_t subframe;
  /**<   Subframe number (0-9)  */

  int32_t snr;
  /**<   SNR in dBm format  */
}cxm_lte_ml1_coex_snr_info_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup cxm_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t subframe;
  /**<   Subframe number (0-9)  */

  uint8_t tx_is_scheduled;
  /**<   If Tx scheduled in subframe 
       Values: \n
       0 = TX not scheduled in subframe \n
       1 = TX is scheduled in subframe
   */

  cxm_lte_ml1_coex_channel_e_v01 channel_type;
  /**<   Channel type
       Values:
       0 = Reserved (CXM_LTE_ML1_COEX_RESERVED)
       1 = PUCCH (CXM_LTE_ML1_COEX_PUCCH)
       2 = PUSCH (CXM_LTE_ML1_COEX_PUSCH)
       3 = PRACH (CXM_LTE_ML1_COEX_PRACH)
       4 = SRS (CXM_LTE_ML1_COEX_SRS)
       5 = PUCCH (CXM_LTE_ML1_COEX_SRS_PUCCH)
       6 = PUSCH (CXM_LTE_ML1_COEX_SRS_PUSCH)
   */

  uint8_t priority;
  /**<   Tx Priority.  Valid values are 0-7.  */

  int8_t slot0_power;
  /**<   Tx Power in slot 1    */

  uint8_t slot0_first_rb;
  /**<   First PRB allocation in slot 1    */

  uint8_t slot0_last_rb;
  /**<   Last PRB allocation in slot 1    */

  int8_t slot1_power;
  /**<   Tx Power in slot 2    */

  uint8_t slot1_first_rb;
  /**<   First PRB allocation in slot 2    */

  uint8_t slot1_last_rb;
  /**<   Last PRB allocation in slot 2    */

  int8_t srs_power;
  /**<   SRS power    */

  uint8_t srs_first_rb;
  /**<   First PRB allocation for SRS    */

  uint8_t srs_last_rb;
  /**<   Last PRB allocation for SRS    */
}cxm_lte_ml1_coex_tx_event_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup cxm_qmi_messages
    @{
  */
/** Indication Message; This message notifies the client of an upcoming TX event. */
typedef struct {

  /* Optional */
  /*  LTE ML1 TX Event Info */
  uint8_t lte_ml1_tx_event_info_valid;  /**< Must be set to true if lte_ml1_tx_event_info is being passed */
  cxm_lte_ml1_coex_tx_event_s_v01 lte_ml1_tx_event_info;

  /* Optional */
  /*  LTE ML1 SNR Info */
  uint8_t lte_ml1_snr_info_valid;  /**< Must be set to true if lte_ml1_snr_info is being passed */
  cxm_lte_ml1_coex_snr_info_s_v01 lte_ml1_snr_info;
}cxm_notify_event_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup cxm_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t dl_subframe;
  /**<   DL subframe number (0-9)  */

  uint32_t dl_frame_time;
  /**<   DL frame timing in terms of UNIV STMR  */

  uint8_t ul_subframe;
  /**<   UL subframe number (0-9)  */

  uint32_t ul_frame_time;
  /**<   UL frame timing in terms of UNIV STMR  */
}cxm_lte_ml1_coex_ustmr_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup cxm_qmi_aggregates
    @{
  */
typedef struct {

  cxm_lte_ml1_coex_report_type_e_v01 report_type;
  /**<   Report Type   */

  cxm_lte_ml1_coex_ustmr_s_v01 timing;
  /**<   timing report information  */
}cxm_lte_ml1_coex_frame_timing_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup cxm_qmi_messages
    @{
  */
/** Indication Message; This message notifies the client of the current frame timing. */
typedef struct {

  /* Mandatory */
  /*  LTE ML1 Frame Timing Report */
  uint32_t frame_timing_report_list_len;  /**< Must be set to # of elements in frame_timing_report_list */
  cxm_lte_ml1_coex_frame_timing_s_v01 frame_timing_report_list[CXM_LTE_ML1_FRAME_TIMING_REPORT_LIST_MAX_V01];
}cxm_frame_timing_ind_msg_v01;  /* Message */
/**
    @}
  */

/*
 * cxm_stmr_read_req_msg is empty
 * typedef struct {
 * }cxm_stmr_read_req_msg_v01;
 */

/** @addtogroup cxm_qmi_messages
    @{
  */
/** Response Message; This message coordinates the reading of the universal  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}cxm_stmr_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * cxm_stmr_read_complete_req_msg is empty
 * typedef struct {
 * }cxm_stmr_read_complete_req_msg_v01;
 */

/** @addtogroup cxm_qmi_aggregates
    @{
  */
typedef struct {

  int8_t backoff_mtpl;
  /**<   Backoff max. transmit power limit    */

  uint8_t priority_threshold;
  /**<   Threshold at/below which  backoff will be applied.  Valid values are 0-7.  */
}cxm_lte_phr_backoff_req_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup cxm_qmi_aggregates
    @{
  */
typedef struct {

  int8_t backoff_mtpl;
  /**<   Backoff max. transmit power limit    */

  uint8_t priority_threshold;
  /**<   Threshold at/below which backoff will be applied.  Valid values are 0-7.    */

  uint32_t starting_time;
  /**<   Time in terms of Univ. STMR from which the backoff will be applied  */

  uint8_t num_of_subframes;
  /**<   No. of subframes from starting time for which this request is valid    */
}cxm_lte_phr_less_backoff_req_s_v01;  /* Type */
/**
    @}
  */

/** @addtogroup cxm_qmi_messages
    @{
  */
/** Request Message; This message requests the service to initiate a Power Headroom
               backoff */
typedef struct {

  /* Optional */
  /*  PHR Backoff Request */
  uint8_t phr_backoff_valid;  /**< Must be set to true if phr_backoff is being passed */
  cxm_lte_phr_backoff_req_s_v01 phr_backoff;

  /* Optional */
  /*  PHR Less Backoff Request */
  uint8_t phr_less_backoff_valid;  /**< Must be set to true if phr_less_backoff is being passed */
  cxm_lte_phr_less_backoff_req_s_v01 phr_less_backoff;
}cxm_phr_backoff_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup cxm_qmi_messages
    @{
  */
/** Response Message; This message requests the service to initiate a Power Headroom
               backoff */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}cxm_phr_backoff_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup cxm_qmi_msg_ids
    @{
  */
#define QMI_CXM_ACTIVATE_REQ_MSG_V01 0x0020
#define QMI_CXM_ACTIVATE_RESP_MSG_V01 0x0020
#define QMI_CXM_DEACTIVATE_REQ_MSG_V01 0x0021
#define QMI_CXM_DEACTIVATE_RESP_MSG_V01 0x0021
#define QMI_CXM_STATE_REQ_MSG_V01 0x0022
#define QMI_CXM_STATE_RESP_MSG_V01 0x0022
#define QMI_CXM_STATE_IND_MSG_V01 0x0022
#define QMI_CXM_NOTIFY_EVENT_IND_V01 0x0023
#define QMI_CXM_FRAME_TIMING_IND_V01 0x0024
#define QMI_CXM_STMR_READ_REQ_MSG_V01 0x0025
#define QMI_CXM_STMR_READ_RESP_MSG_V01 0x0025
#define QMI_CXM_STMR_READ_COMPLETE_REQ_MSG_V01 0x0026
#define QMI_CXM_STMR_READ_COMPLETE_RESP_MSG_V01 0x0026
#define QMI_CXM_PHR_BACKOFF_REQ_MSG_V01 0x0027
#define QMI_CXM_PHR_BACKOFF_RESP_MSG_V01 0x0027
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro cxm_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type cxm_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define cxm_get_service_object_v01( ) \
          cxm_get_service_object_internal_v01( \
            CXM_V01_IDL_MAJOR_VERS, CXM_V01_IDL_MINOR_VERS, \
            CXM_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

