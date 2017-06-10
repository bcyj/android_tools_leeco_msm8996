#ifndef IMSDCM_SERVICE_01_H
#define IMSDCM_SERVICE_01_H
/**
  @file ip_multimedia_subsystem_dcm_v01.h

  @brief This is the public header file which defines the imsdcm service Data structures.

  This header file defines the types and structures that were defined in
  imsdcm. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2
   It requires encode/decode library version 5 or later
   It was generated on: Mon Sep 16 2013 (Spin 0)
   From IDL File: ip_multimedia_subsystem_dcm_v01.idl */

/** @defgroup imsdcm_qmi_consts Constant values defined in the IDL */
/** @defgroup imsdcm_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup imsdcm_qmi_enums Enumerated types used in QMI messages */
/** @defgroup imsdcm_qmi_messages Structures sent as QMI messages */
/** @defgroup imsdcm_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup imsdcm_qmi_accessor Accessor for QMI service object */
/** @defgroup imsdcm_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup imsdcm_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define IMSDCM_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define IMSDCM_V01_IDL_MINOR_VERS 0x03
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define IMSDCM_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define IMSDCM_V01_MAX_MESSAGE_ID 0x0027;
/**
    @}
  */


/** @addtogroup imsdcm_qmi_consts
    @{
  */

/**  Maximum bytes needed to store an IP address(IPv4/IPv6)  */
#define IMS_DCM_APN_STRING_LEN_MAX_V01 100

/**  Enumeration for all the IMS supported Radio Access Technology. */
#define IMS_DCM_IP_ADDR_STRING_LEN_MAX_V01 40
/**
    @}
  */

/** @addtogroup imsdcm_qmi_enums
    @{
  */
typedef enum {
  IMS_DCM_RAT_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_DCM_RAT_EHRPD_V01 = 0, /**<  CDMA eHRPD \n  */
  IMS_DCM_RAT_LTE_V01 = 1, /**<  LTE \n  */
  IMS_DCM_RAT_EPC_V01 = 2, /**<  EPC \n  */
  IMS_DCM_RAT_WLAN_V01 = 3, /**<  WLAN  */
  IMS_DCM_RAT_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_dcm_rat_type_v01;
/**
    @}
  */

/** @addtogroup imsdcm_qmi_enums
    @{
  */
typedef enum {
  IMS_DCM_APN_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_DCM_APN_IMS_V01 = 0, /**<  IMS \n  */
  IMS_DCM_APN_INTERNET_V01 = 1, /**<  Internet \n  */
  IMS_DCM_APN_EMERGENCY_V01 = 2, /**<  Emergency \n  */
  IMS_DCM_APN_RCS_V01 = 3, /**<  RCS \n  */
  IMS_DCM_APN_UT_V01 = 4, /**<  UT \n  */
  IMS_DCM_APN_WLAN_V01 = 5, /**<  WLAN  */
  IMS_DCM_APN_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_dcm_apn_type_v01;
/**
    @}
  */

/** @addtogroup imsdcm_qmi_enums
    @{
  */
typedef enum {
  IMS_DCM_IP_ADDRESS_FAMILY_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_DCM_IPV4_V01 = 0, /**<  IPv4 address \n  */
  IMS_DCM_IPV6_V01 = 1, /**<  IPv6 address  */
  IMS_DCM_IP_ADDRESS_FAMILY_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_dcm_ip_address_family_type_v01;
/**
    @}
  */

/** @addtogroup imsdcm_qmi_enums
    @{
  */
typedef enum {
  IMS_DCM_WIFI_ATTACH_STATUS_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_DCM_WIFI_STATE_ATTACH_TO_WIFI_V01 = 0, /**<  IMS attached to WiFi \n  */
  IMS_DCM_WIFI_STATE_DETACH_FROM_WIFI_V01 = 1, /**<  IMS not attached to WiFi  */
  IMS_DCM_WIFI_ATTACH_STATUS_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_dcm_wifi_attach_status_v01;
/**
    @}
  */

/** @addtogroup imsdcm_qmi_enums
    @{
  */
typedef enum {
  IMS_DCM_WIFI_QUALITY_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_DCM_WIFI_QUALITY_GOOD_V01 = 0, /**<  Where RSSI value is beyond the threshold \n  */
  IMS_DCM_WIFI_QUALITY_BAD_V01 = 1, /**<  Where RSSI value is below the threshold  */
  IMS_DCM_WIFI_QUALITY_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_dcm_wifi_quality_v01;
/**
    @}
  */

/** @addtogroup imsdcm_qmi_aggregates
    @{
  */
typedef struct {

  ims_dcm_ip_address_family_type_v01 family_type;
  /**<   IP address type. Values: \n
      - IMS_DCM_IPV4 (0) --  IPv4 address \n
      - IMS_DCM_IPV6 (1) --  IPv6 address
 */

  uint32_t ipaddr_len;  /**< Must be set to # of elements in ipaddr */
  char ipaddr[IMS_DCM_IP_ADDR_STRING_LEN_MAX_V01];
  /**<   Buffer containing the IP address of the interface.
  For IPv4, this confirms to dot-decimal notation.
  For IPv6, this confirms to RFC 5952.
  */
}imsdcm_addr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t port;
  /**<   Modem end point port. */

  ims_dcm_ip_address_family_type_v01 family_type;
  /**<   IP address type. Values: \n
      - IMS_DCM_IPV4 (0) --  IPv4 address \n
      - IMS_DCM_IPV6 (1) --  IPv6 address
 */

  uint32_t ipaddr_len;  /**< Must be set to # of elements in ipaddr */
  char ipaddr[IMS_DCM_IP_ADDR_STRING_LEN_MAX_V01];
  /**<   Buffer containing the IP address of the interface.
       For IPv4, this confirms to dot-decimal notation.
       For IPv6, this confirms to RFC 5952.
  */
}imsdcm_link_addr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_aggregates
    @{
  */
typedef struct {

  char apn_name[IMS_DCM_APN_STRING_LEN_MAX_V01 + 1];
  /**<   APN name. */

  ims_dcm_apn_type_v01 apn_type;
  /**<   APN type. Values:\n
      - IMS_DCM_APN_IMS (0) --  IMS \n
      - IMS_DCM_APN_INTERNET (1) --  Internet \n
      - IMS_DCM_APN_EMERGENCY (2) --  Emergency \n
      - IMS_DCM_APN_RCS (3) --  RCS \n
      - IMS_DCM_APN_UT (4) --  UT \n
      - IMS_DCM_APN_WLAN (5) --  WLAN
 */

  ims_dcm_rat_type_v01 rat_type;
  /**<   Radio access technology. Values:\n
      - IMS_DCM_RAT_EHRPD (0) --  CDMA eHRPD \n
      - IMS_DCM_RAT_LTE (1) --  LTE \n
      - IMS_DCM_RAT_EPC (2) --  EPC \n
      - IMS_DCM_RAT_WLAN (3) --  WLAN
 */

  ims_dcm_ip_address_family_type_v01 family_type;
  /**<   IP address family. Values:\n
      - IMS_DCM_IPV4 (0) --  IPv4 address \n
      - IMS_DCM_IPV6 (1) --  IPv6 address
 */

  int32_t profile_num;
  /**<   Data services (DS) profile number */
}ims_dcm_pdp_information_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Request Message; Brings up the PDN. */
typedef struct {

  /* Mandatory */
  /*  PDP Information */
  ims_dcm_pdp_information_v01 pdp_info;

  /* Optional */
  /*  PDP Request Sequence Number */
  uint8_t pdp_req_seq_no_valid;  /**< Must be set to true if pdp_req_seq_no is being passed */
  uint32_t pdp_req_seq_no;
  /**<   Cookie to help the client uniquely identify this specific PDP request.
  */
}ims_dcm_pdp_activate_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Response Message; Brings up the PDN. */
typedef struct {

  /* Mandatory */
  /*  Result Code     */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
       - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       - qmi_error_type  -- Error code. Possible error code values are
                            described in the error codes section of each
                            message definition.
                            Only if processing was successful, will an
                            indication be sent which will hold information
                            about the result of processing.
  */
}ims_dcm_pdp_activate_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Indication Message; Informs the client about the IP address obtained as a result of
	PDP activation. */
typedef struct {

  /* Mandatory */
  /*  PDP ID */
  uint8_t pdp_id;
  /**<   PDP ID to uniquely identify the interface. 0 indicates a failure. More details
       about the reason for failure can be obtained from the pdp_error field.
  */

  /* Mandatory */
  /*  PDP Error */
  qmi_response_type_v01 pdp_error;
  /**<   PDP Error Information
       Indicates the reason for the PDP activation failure.
      - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      - qmi_error_type -- Error code. Possible error code values are
      - QMI_ERR_NONE -- Success case
      - QMI_ERR_INVALID_PROFILE -- Invalid ds profile specified \n
      - QMI_ERR_INVALID_PDP_TYPE -- Unsupported PDP type specified \n

  */

  /* Optional */
  /*  PDP Request Sequence Number */
  uint8_t pdp_req_seq_no_valid;  /**< Must be set to true if pdp_req_seq_no is being passed */
  uint32_t pdp_req_seq_no;
  /**<   Cookie to help the client uniquely identify this specific PDP request.
  */

  /* Optional */
  /*  Address Information */
  uint8_t addr_info_valid;  /**< Must be set to true if addr_info is being passed */
  imsdcm_addr_type_v01 addr_info;
  /**<   \n
    IP address obtained as a result of PDN bring-up.
  */
}ims_dcm_pdp_activate_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Request Message; Brings down the previously activated PDP. */
typedef struct {

  /* Mandatory */
  /*  PDP ID */
  uint8_t pdp_id;
  /**<   PDP ID that uniquely identifies this specific PDP request. */
}ims_dcm_pdp_deactivate_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Response Message; Brings down the previously activated PDP. */
typedef struct {

  /* Mandatory */
  /*  Result Code     */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
      - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      - qmi_error_type  -- Error code. Possible error code values are
                           described in the error codes section of each
                           message definition
    */

  /* Optional */
  /*  PDP ID */
  uint8_t pdp_id_valid;  /**< Must be set to true if pdp_id is being passed */
  uint8_t pdp_id;
  /**<   PDP ID that uniquely identifies this specific PDP. */
}ims_dcm_pdp_deactivate_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Request Message; Obtains the IP address associated
             with the specified PDP ID. */
typedef struct {

  /* Mandatory */
  /*  PDP ID */
  uint8_t pdp_id;
  /**<   PDP ID that uniquely identifies this specific PDP request. */
}ims_dcm_pdp_get_ip_address_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Response Message; Obtains the IP address associated
             with the specified PDP ID. */
typedef struct {

  /* Mandatory */
  /*  Result Code     */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
      - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      - qmi_error_type  -- Error code. Possible error code values are
                           described in the error codes section of each
                           message definition.
   */
}ims_dcm_pdp_get_ip_address_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Indication Message; Informs the client about the IP address obtained as a result of the query. */
typedef struct {

  /* Mandatory */
  /*  PDP ID */
  uint8_t pdp_id;
  /**<   PDP ID that uniquely identifies the interface.
  */

  /* Optional */
  /*  Address Information */
  uint8_t addr_info_valid;  /**< Must be set to true if addr_info is being passed */
  imsdcm_addr_type_v01 addr_info;
  /**<   \n
    IP address obtained as a result of PDN bring-up.
  */
}ims_dcm_pdp_get_ip_address_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Request Message; Sets the modem local link address. */
typedef struct {

  /* Mandatory */
  /*  PDP Information */
  imsdcm_link_addr_type_v01 link_local_ip_addr;
  /**<   \n
     Request to update the link local IP.
  */
}ims_dcm_modem_link_add_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Response Message; Sets the modem local link address. */
typedef struct {

  /* Mandatory */
  /*  Result Code     */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
       - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       - qmi_error_type  -- Error code. Possible error code values are
                            described in the error codes section of each
                            message definition.
                            If processing was successful, an
                            indication is sent that holds information
                            about the processing result.
  */
}ims_dcm_modem_link_add_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Indication Message; Informs the client about the change in IP address. */
typedef struct {

  /* Mandatory */
  /*  PDP ID */
  uint8_t pdp_id;
  /**<   PDP ID that uniquely identifies the interface.
  */

  /* Optional */
  /*  Address Information */
  uint8_t addr_info_valid;  /**< Must be set to true if addr_info is being passed */
  imsdcm_addr_type_v01 addr_info;
  /**<   \n
  IP address obtained as a result of PDN bring-up.
  */
}ims_dcm_address_change_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Request Message; Start WiFi quality measurments. */
typedef struct {

  /* Mandatory */
  /*  PDP ID */
  uint8_t pdp_id;
  /**<   PDP ID that uniquely identifies the interface.
  */

  /* Mandatory */
  /*  RSSI Threshold Value  */
  uint32_t rssi_threshold_add;
  /**<   RSSI threshold value to be compared while \n
       offloading from LTE to WiFi (units in dbm) */

  /* Mandatory */
  /*  Max RSSI Tolerance Value  */
  uint32_t rssi_threshold_drop;
  /**<   Max RSSI tolerance value to be used to decide \n
       the timer value (close/far) when average is \n
       - Gaining towards iRSSIThresholdAdd OR \n
       - Falling towards iRSSIThresholdDrop (units in dbm)*/

  /* Mandatory */
  /*  RSSI Threshold Close Value  */
  uint32_t rssi_threshold_close;
  /**<   RSSI threshold close value to be compared with the difference between(average RSSI \n
     - iRSSIThresholdAdd/iRSSIThresholdDrop) (units in dbm) to choose the right timer  */

  /* Mandatory */
  /*  RSSI Timer Close Value  */
  uint32_t rssi_timer_close;
  /**<   Timer value to be set when average RSSI \n
       value is far from iRSSIThresholdAdd/iRSSIThresholdDrop (units in seconds)	 */

  /* Mandatory */
  /*  RSSI Timer Far Value  */
  uint32_t rssi_timer_far;
  /**<   Timer value to be set when average RSSI \n
       is above iRSSIThresholdAdd (Next scan cycle units in seconds)	 */

  /* Mandatory */
  /*  RSSI Average Timer on Attach Value  */
  uint32_t rssi_avg_timer_on_attach;
  /**<   Timer value to be set when average RSSI \n
       is above iRSSIThresholdAdd (Next scan cycle units in seconds)   */

  /* Mandatory */
  /*  RSSI Average Timer on Camp Value  */
  uint32_t rssi_avg_timer_on_camp;
  /**<   Timer value to be set when average RSSI \n
       is above iRSSIThresholdAdd (Next scan cycle units in seconds)   */

  /* Mandatory */
  /*  RSSI Sampling Timer Value  */
  uint32_t rssi_sampling_timer;
  /**<   Timer value to be set when average RSSI \n
       is above iRSSIThresholdAdd (Next scan cycle units in seconds)   */

  /* Mandatory */
  /*  RSSI Average Internal Value  */
  uint32_t rssi_average_interval;
  /**<   Timer value to be set when average RSSI \n
       is above iRSSIThresholdAdd (Next scan cycle units in seconds)   */

  /* Mandatory */
  /*  Current Attach Status */
  ims_dcm_wifi_attach_status_v01 curattachstatus;
  /**<   Current attach status.
  */
}ims_dcm_get_wifi_quality_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Response Message; Start WiFi quality measurments. */
typedef struct {

  /* Mandatory */
  /*  Result Code     */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
       - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       - qmi_error_type  -- Error code. Possible error code values are
                            described in the error codes section of each
                            message definition.
                            Only if processing was successful, will an
                            indication be sent which will hold information
                            about the result of processing.
  */

  /* Optional */
  /*  PDP ID */
  uint8_t pdp_id_valid;  /**< Must be set to true if pdp_id is being passed */
  uint8_t pdp_id;
  /**<   PDP ID that uniquely identifies the interface.
  */
}ims_dcm_get_wifi_quality_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Indication Message; Informs the client about the WiFi quality report. */
typedef struct {

  /* Mandatory */
  /*  PDP ID */
  uint8_t pdp_id;
  /**<   PDP ID to uniquely identify the interface. 0 indicates a failure. More details
       about the reason for failure can be obtained from the pdp_error field.
  */

  /* Mandatory */
  /*  WiFi Quality Report */
  ims_dcm_wifi_quality_v01 qualityreport;
  /**<   WiFi quality information
       Indicates the quality report.
  */
}ims_dcm_get_wifi_quality_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Request Message; Stops WiFi quality measurments. */
typedef struct {

  /* Mandatory */
  /*  PDP ID */
  uint8_t pdp_id;
  /**<   PDP ID that uniquely identifies this specific PDP request. */
}ims_dcm_stop_wifi_quality_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Response Message; Stops WiFi quality measurments. */
typedef struct {

  /* Mandatory */
  /*  Result Code     */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
      - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      - qmi_error_type  -- Error code. Possible error code values are
                           described in the error codes section of each
                           message definition
    */

  /* Optional */
  /*  PDP ID */
  uint8_t pdp_id_valid;  /**< Must be set to true if pdp_id is being passed */
  uint8_t pdp_id;
  /**<   PDP ID that uniquely identifies this specific PDP. */
}ims_dcm_stop_wifi_quality_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Request Message; Update WiFi quality measurments. */
typedef struct {

  /* Mandatory */
  /*  PDP ID */
  uint8_t pdp_id;
  /**<   PDP ID that uniquely identifies this specific PDP request. */

  /* Mandatory */
  /*  PDP ID */
  ims_dcm_wifi_attach_status_v01 attachstatus;
  /**<   Attach status.   */
}ims_dcm_update_wifi_quality_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsdcm_qmi_messages
    @{
  */
/** Response Message; Update WiFi quality measurments. */
typedef struct {

  /* Mandatory */
  /*  Result Code     */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
      - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      - qmi_error_type  -- Error code. Possible error code values are
                           described in the error codes section of each
                           message definition
    */

  /* Optional */
  /*  PDP ID */
  uint8_t pdp_id_valid;  /**< Must be set to true if pdp_id is being passed */
  uint8_t pdp_id;
  /**<   PDP ID that uniquely identifies this specific PDP. */
}ims_dcm_update_wifi_quality_rsp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup imsdcm_qmi_msg_ids
    @{
  */
#define QMI_IMS_DCM_PDP_ACTIVATE_REQ_V01 0x0020
#define QMI_IMS_DCM_PDP_ACTIVATE_RSP_V01 0x0020
#define QMI_IMS_DCM_PDP_ACTIVATE_IND_V01 0x0020
#define QMI_IMS_DCM_PDP_DEACTIVATE_REQ_V01 0x0021
#define QMI_IMS_DCM_PDP_DEACTIVATE_RSP_V01 0x0021
#define QMI_IMS_DCM_GET_IP_ADDRESS_REQ_V01 0x0022
#define QMI_IMS_DCM_GET_IP_ADDRESS_RSP_V01 0x0022
#define QMI_IMS_DCM_GET_IP_ADDRESS_IND_V01 0x0022
#define QMI_IMS_DCM_LINK_ADDR_REQ_V01 0x0023
#define QMI_IMS_DCM_LINK_ADDR_RSP_V01 0x0023
#define QMI_IMS_DCM_ADDRESS_CHANGE_IND_V01 0x0024
#define QMI_IMS_DCM_GET_WIFI_QUALITY_REQ_V01 0x0025
#define QMI_IMS_DCM_GET_WIFI_QUALITY_RSP_V01 0x0025
#define QMI_IMS_DCM_WIFI_QUALITY_IND_V01 0x0025
#define QMI_IMS_DCM_STOP_WIFI_QUALITY_REQ_V01 0x0026
#define QMI_IMS_DCM_STOP_WIFI_QUALITY_RSP_V01 0x0026
#define QMI_IMS_DCM_UPDATE_WIFI_QUALITY_REQ_V01 0x0027
#define QMI_IMS_DCM_UPDATE_WIFI_QUALITY_RSP_V01 0x0027
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro imsdcm_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type imsdcm_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define imsdcm_get_service_object_v01( ) \
          imsdcm_get_service_object_internal_v01( \
            IMSDCM_V01_IDL_MAJOR_VERS, IMSDCM_V01_IDL_MINOR_VERS, \
            IMSDCM_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif
