#ifndef IMSA_SERVICE_01_H
#define IMSA_SERVICE_01_H
/**
  @file ip_multimedia_subsystem_application_v01.h

  @brief This is the public header file which defines the imsa service Data structures.

  This header file defines the types and structures that were defined in
  imsa. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All rights reserved.
  Qualcomm Technologies Proprietary and Confidential.


  $Header: //source/qcom/qct/interfaces/qmi/imsa/main/latest/api/ip_multimedia_subsystem_application_v01.h#32 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.5 
   It was generated on: Thu Apr 30 2015 (Spin 0)
   From IDL File: ip_multimedia_subsystem_application_v01.idl */

/** @defgroup imsa_qmi_consts Constant values defined in the IDL */
/** @defgroup imsa_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup imsa_qmi_enums Enumerated types used in QMI messages */
/** @defgroup imsa_qmi_messages Structures sent as QMI messages */
/** @defgroup imsa_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup imsa_qmi_accessor Accessor for QMI service object */
/** @defgroup imsa_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup imsa_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define IMSA_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define IMSA_V01_IDL_MINOR_VERS 0x13
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define IMSA_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define IMSA_V01_MAX_MESSAGE_ID 0x002E
/**
    @}
  */


/** @addtogroup imsa_qmi_consts 
    @{ 
  */
#define IMSA_HO_FAILURE_CAUSE_CODE_STR_LEN_V01 125
#define IMSA_SUBSCRIPTION_FAILURE_ERROR_STR_LEN_V01 125
#define IMSA_REGISTRATION_FAILURE_ERROR_STR_LEN_V01 255
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_SERVICE_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_NO_SERVICE_V01 = 0, 
  IMSA_LIMITED_SERVICE_V01 = 1, /**<  IMS is in limited service  */
  IMSA_FULL_SERVICE_V01 = 2, /**<  IMS is in full service  */
  IMSA_SERVICE_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_service_status_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_SERVICE_RAT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_WLAN_V01 = 0, /**<  IMS service is registered on WLAN \n  */
  IMSA_WWAN_V01 = 1, /**<  IMS service is registered on WWAN \n  */
  IMSA_IWLAN_V01 = 2, /**<  IMS service is registered on interworking WLAN  */
  IMSA_SERVICE_RAT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_service_rat_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_IMS_REGISTRATION_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_STATUS_NOT_REGISTERED_V01 = 0, /**<  Not registered for IMS \n  */
  IMSA_STATUS_REGISTERING_V01 = 1, /**<  Registering for IMS \n  */
  IMSA_STATUS_REGISTERED_V01 = 2, /**<  Registered for IMS  */
  IMSA_IMS_REGISTRATION_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_ims_registration_status_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_RAT_HANDOVER_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_STATUS_RAT_HO_SUCCESS_V01 = 0, /**<  RAT handover was successful \n  */
  IMSA_STATUS_RAT_HO_FAILURE_V01 = 1, /**<  RAT handover failed  */
  IMSA_STATUS_RAT_HO_NOT_TRIGGERED_V01 = 2, /**<  RAT handover could not be triggered  */
  IMSA_RAT_HANDOVER_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_rat_handover_status_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_PDP_FAILURE_ERROR_CODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_PDP_STATUS_OTHER_FAILURE_V01 = 0, /**<  Generic failure reason for other than specified \n  */
  IMSA_PDP_STATUS_OPTION_UNSUBSCRIBED_V01 = 1, /**<  Option is unsubscribed \n  */
  IMSA_PDP_STATUS_UNKNOWN_PDP_V01 = 2, /**<  PDP was unknown  */
  IMSA_PDP_STATUS_REASON_NOT_SPECIFIED_V01 = 3, /**<  Reason not specified \n */
  IMSA_PDP_STATUS_CONNECTION_BRINGUP_FAILURE_V01 = 4, /**<  Connection bring-up failure  */
  IMSA_PDP_FAILURE_ERROR_CODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_pdp_failure_error_code_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_ACS_FAILURE_STATUS_CODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_ACS_FAILURE_NO_EXISTING_ACS_DATA_V01 = 0, /**<  ACS failure; there is no earlier APCS success data, and
                       the ISIM parameters are not available \n  */
  IMSA_ACS_FAILURE_ERROR_CODE_RECEIVED_V01 = 1, /**<  Error code received from the network for an
       ACS configuration retrieval request  */
  IMSA_ACS_FAILURE_STATUS_CODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_acs_failure_status_code_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_SUBSCRIPTION_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_SUBSCRIPTION_TYPE_REG_V01 = 0, /**<  Register subscription\n  */
  IMSA_SUBSCRIPTION_TYPE_MWI_V01 = 1, /**<  MWI subscription  */
  IMSA_SUBSCRIPTION_TYPE_CONFERENCE_V01 = 2, /**<  Conference subscription  */
  IMSA_SUBSCRIPTION_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_subscription_type_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_NETWORK_PROVISIONING_STATUS_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_NETWORK_PROV_OUT_OF_SYNC_V01 = 0, /**<  Network is out of sync. This is an 
       indication sent when the IMS UE detects the network provisioning status through 
       SIP messages. When the network is out of sync with the UE, DM resync 
       may be triggered by the OEM based on this indication.
   */
  IMSA_NETWORK_PROVISIONING_STATUS_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_network_provisioning_status_type_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_aggregates
    @{
  */
/**  IMS service is not available 
 */
typedef struct {

  imsa_rat_handover_status_enum_v01 rat_ho_status;
  /**<   RAT handover status. */

  imsa_service_rat_enum_v01 source_rat;
  /**<   Source RAT information; IWLAN/WWAN. */

  imsa_service_rat_enum_v01 target_rat;
  /**<   Target RAT information; IWLAN/WWAN. */

  char cause_code[IMSA_HO_FAILURE_CAUSE_CODE_STR_LEN_V01 + 1];
  /**<   Handover failure cause code string
       when the status is IMSA_STATUS_RAT_HO_FAILURE. */
}imsa_rat_handover_status_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Gets the registration status for various IMS services for the
             requesting control point. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_registration_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Gets the registration status for various IMS services for the
             requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  IMS Registration Status (Deprecated) */
  uint8_t ims_registered_valid;  /**< Must be set to true if ims_registered is being passed */
  uint8_t ims_registered;
  /**<   Values: \n
       - TRUE  -- UE is registered on the IMS network \n
       - FALSE -- UE is not registered on the IMS network

       Note: This TLV is deprecated and
             the TLV New IMS Registration Status is used instead.
   */

  /* Optional */
  /*  IMS Registration Error Code */
  uint8_t ims_registration_failure_error_code_valid;  /**< Must be set to true if ims_registration_failure_error_code is being passed */
  uint16_t ims_registration_failure_error_code;
  /**<   IMS registration error code. An error code is returned when the IMS
       registration status is IMSA_STATUS_NOT_REGISTERED. \n
       Values: \n
       - 3xx -- Redirection responses \n
       - 4xx -- Client failure responses \n
       - 5xx -- Server failure responses \n
       - 6xx -- Global failure responses
  */

  /* Optional */
  /*  New IMS Registration Status */
  uint8_t ims_reg_status_valid;  /**< Must be set to true if ims_reg_status is being passed */
  imsa_ims_registration_status_enum_v01 ims_reg_status;
  /**<   IMS registration status. Values: \n
      - IMSA_STATUS_NOT_REGISTERED (0) --  Not registered for IMS \n 
      - IMSA_STATUS_REGISTERING (1) --  Registering for IMS \n 
      - IMSA_STATUS_REGISTERED (2) --  Registered for IMS 
 */

  /* Optional */
  /*  Registration Error String */
  uint8_t registration_error_string_valid;  /**< Must be set to true if registration_error_string is being passed */
  char registration_error_string[IMSA_REGISTRATION_FAILURE_ERROR_STR_LEN_V01 + 1];
  /**<   Registration failure error string
       when the ims_registered value is FALSE. */

  /* Optional */
  /*  Registration Network */
  uint8_t registration_network_valid;  /**< Must be set to true if registration_network is being passed */
  imsa_service_rat_enum_v01 registration_network;
  /**<   IMS registration network. Network is returned when IMS 
       registration is being attempted or is successful	\n
       Values: \n
       - IMSA_WLAN = 0 \n
       - IMSA_WWAN = 1 \n
       - IMSA_IWLAN = 2
   */
}imsa_get_registration_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Gets the service status for various IMS services for the
             requesting control point. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_service_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Gets the service status for various IMS services for the
             requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  SMS Service Status */
  uint8_t sms_service_status_valid;  /**< Must be set to true if sms_service_status is being passed */
  imsa_service_status_enum_v01 sms_service_status;
  /**<   Values: \n
       - 0 -- IMS SMS service is not available \n
       - 1 -- IMS SMS is in limited service \n
       - 2 -- IMS SMS is in full service
  */

  /* Optional */
  /*  VoIP Service Status */
  uint8_t voip_service_status_valid;  /**< Must be set to true if voip_service_status is being passed */
  imsa_service_status_enum_v01 voip_service_status;
  /**<   Values: \n
       - 0 -- IMS VoIP service is not available \n
       - 2 -- IMS VoIP is in full service
  */

  /* Optional */
  /*  VT Service Status */
  uint8_t vt_service_status_valid;  /**< Must be set to true if vt_service_status is being passed */
  imsa_service_status_enum_v01 vt_service_status;
  /**<   Values: \n
       - 0 -- IMS VT service is not available \n
       - 2 -- IMS VT is in full service
  */

  /* Optional */
  /*  SMS RAT */
  uint8_t sms_service_rat_valid;  /**< Must be set to true if sms_service_rat is being passed */
  imsa_service_rat_enum_v01 sms_service_rat;
  /**<   SMS service RAT. Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  VoIP RAT */
  uint8_t voip_service_rat_valid;  /**< Must be set to true if voip_service_rat is being passed */
  imsa_service_rat_enum_v01 voip_service_rat;
  /**<   VoIP service RAT. Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  VT RAT */
  uint8_t vt_service_rat_valid;  /**< Must be set to true if vt_service_rat is being passed */
  imsa_service_rat_enum_v01 vt_service_rat;
  /**<   VT service RAT. Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  UT Service Status */
  uint8_t ut_service_status_valid;  /**< Must be set to true if ut_service_status is being passed */
  imsa_service_status_enum_v01 ut_service_status;
  /**<   UT service status. Values: \n
       - 0 -- IMS UT service is not available \n
       - 2 -- IMS UT is in full service
  */

  /* Optional */
  /*  UT RAT */
  uint8_t ut_service_rat_valid;  /**< Must be set to true if ut_service_rat is being passed */
  imsa_service_rat_enum_v01 ut_service_rat;
  /**<   UT service RAT. Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  VS Service Status */
  uint8_t vs_service_status_valid;  /**< Must be set to true if vs_service_status is being passed */
  imsa_service_status_enum_v01 vs_service_status;
  /**<   VS service status. Values: \n
       - 0 -- IMS UT service is not available \n
       - 2 -- IMS UT is in full service
  */

  /* Optional */
  /*  VS RAT */
  uint8_t vs_service_rat_valid;  /**< Must be set to true if vs_service_rat is being passed */
  imsa_service_rat_enum_v01 vs_service_rat;
  /**<   Video Share service RAT. Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */
}imsa_get_service_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Gets the PDP status.  */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_pdp_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Gets the PDP status.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  IMS PDP Connection State */
  uint8_t is_ims_pdp_connected_valid;  /**< Must be set to true if is_ims_pdp_connected is being passed */
  uint8_t is_ims_pdp_connected;
  /**<   IMS PDP connection state information.\n
     Values:\n
       - TRUE  -- IMS PDP is connected \n
       - FALSE -- IMS PDP is not connected
    */

  /* Optional */
  /*  IMS PDP Failure Error Code */
  uint8_t ims_pdp_failure_error_code_valid;  /**< Must be set to true if ims_pdp_failure_error_code is being passed */
  imsa_pdp_failure_error_code_enum_v01 ims_pdp_failure_error_code;
  /**<   IMS PDP connection failure error reason code when the
 IMS PDP Connection State TLV is FALSE (that is, IMS PDP is not in
 the Connected state). Values:\n
      - IMSA_PDP_STATUS_OTHER_FAILURE (0) --  Generic failure reason for other than specified \n 
      - IMSA_PDP_STATUS_OPTION_UNSUBSCRIBED (1) --  Option is unsubscribed \n 
      - IMSA_PDP_STATUS_UNKNOWN_PDP (2) --  PDP was unknown 
      - IMSA_PDP_STATUS_REASON_NOT_SPECIFIED (3) --  Reason not specified \n
      - IMSA_PDP_STATUS_CONNECTION_BRINGUP_FAILURE (4) --  Connection bring-up failure 
 */
}imsa_get_pdp_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Gets the LTE attach parameters.  */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_lte_attach_parameters_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Gets the LTE attach parameters.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  LTE Attach Parameters  */
  uint8_t operator_reserved_pco_valid;  /**< Must be set to true if operator_reserved_pco is being passed */
  uint8_t operator_reserved_pco;
  /**<   Operator-reserved PCO value for subscription status. Preconditions: \n
       - The operator will always have an operator-reserved PCO container in the Attach response. \n
       - Applications should query to get the API only when the PDP is connected.
      */
}imsa_get_lte_attach_parameters_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Gets the RTP statistics. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_rtp_statistics_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Gets the RTP statistics. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}imsa_get_rtp_statistics_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Sets the registration state for different IMS service indications
             for the requesting control point. */
typedef struct {

  /* Optional */
  /*  Registration Status Configuration */
  uint8_t reg_status_config_valid;  /**< Must be set to true if reg_status_config is being passed */
  uint8_t reg_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Service Status Configuration */
  uint8_t service_status_config_valid;  /**< Must be set to true if service_status_config is being passed */
  uint8_t service_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  RAT Handover Status Configuration */
  uint8_t rat_handover_config_valid;  /**< Must be set to true if rat_handover_config is being passed */
  uint8_t rat_handover_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  PDP Status Configuration */
  uint8_t pdp_status_config_valid;  /**< Must be set to true if pdp_status_config is being passed */
  uint8_t pdp_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  ACS Retrieval Status Configuration */
  uint8_t acs_status_config_valid;  /**< Must be set to true if acs_status_config is being passed */
  uint8_t acs_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  LTE Attach Parameters Configuration */
  uint8_t lte_attach_params_config_valid;  /**< Must be set to true if lte_attach_params_config is being passed */
  uint8_t lte_attach_params_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Subscription Status Configuration */
  uint8_t subscription_status_config_valid;  /**< Must be set to true if subscription_status_config is being passed */
  uint8_t subscription_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
    */

  /* Optional */
  /*  Network Provisioning Status Configuration */
  uint8_t network_provisioning_status_config_valid;  /**< Must be set to true if network_provisioning_status_config is being passed */
  uint8_t network_provisioning_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
    */
}imsa_ind_reg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Sets the registration state for different IMS service indications
             for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}imsa_ind_reg_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent when the IMS service registration status changes. */
typedef struct {

  /* Mandatory */
  /*  IMS Registration Status (Deprecated) */
  uint8_t ims_registered;
  /**<   Values: \n
       - TRUE  -- UE is registered on the IMS network \n
       - FALSE -- UE is not registered on the IMS network \n
       Note: This TLV is deprecated and the
             new TLV New IMS Registration Status is used instead.
   */

  /* Optional */
  /*  IMS Registration Error Code */
  uint8_t ims_registration_failure_error_code_valid;  /**< Must be set to true if ims_registration_failure_error_code is being passed */
  uint16_t ims_registration_failure_error_code;
  /**<   IMS registration error code. An error code is returned when the IMS
       registration status is IMSA_STATUS_NOT_REGISTERED.\n
       Values: \n
       - 3xx -- Redirection responses \n
       - 4xx -- Client failure responses \n
       - 5xx -- Server failure responses \n
       - 6xx -- Global failure responses
  */

  /* Optional */
  /*  New IMS Registration Status */
  uint8_t ims_reg_status_valid;  /**< Must be set to true if ims_reg_status is being passed */
  imsa_ims_registration_status_enum_v01 ims_reg_status;
  /**<   IMS registration status. Values: \n
      - IMSA_STATUS_NOT_REGISTERED (0) --  Not registered for IMS \n 
      - IMSA_STATUS_REGISTERING (1) --  Registering for IMS \n 
      - IMSA_STATUS_REGISTERED (2) --  Registered for IMS 
 */

  /* Optional */
  /*  Registration Error String */
  uint8_t registration_error_string_valid;  /**< Must be set to true if registration_error_string is being passed */
  char registration_error_string[IMSA_REGISTRATION_FAILURE_ERROR_STR_LEN_V01 + 1];
  /**<   Registration failure error string
       when the ims_registered is FALSE. */

  /* Optional */
  /*  Registration Network */
  uint8_t registration_network_valid;  /**< Must be set to true if registration_network is being passed */
  imsa_service_rat_enum_v01 registration_network;
  /**<   IMS registration network. Network is returned when IMS 
       registration is being attempted or is successful	\n
       Values: \n
       - IMSA_WLAN = 0 \n
       - IMSA_WWAN = 1 \n
       - IMSA_IWLAN = 2
   */
}imsa_registration_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent when the service status of an IMS service changes. */
typedef struct {

  /* Optional */
  /*  SMS Service Status */
  uint8_t sms_service_status_valid;  /**< Must be set to true if sms_service_status is being passed */
  imsa_service_status_enum_v01 sms_service_status;
  /**<   Values: \n
       - 0 -- IMS SMS service is not available \n
       - 1 -- IMS SMS is in limited service \n
       - 2 -- IMS SMS is in full service
   */

  /* Optional */
  /*  VoIP Service Status */
  uint8_t voip_service_status_valid;  /**< Must be set to true if voip_service_status is being passed */
  imsa_service_status_enum_v01 voip_service_status;
  /**<   Values: \n
       - 0 -- IMS VoIP service is not available \n
       - 2 -- IMS VoIP is in full service
    */

  /* Optional */
  /*  VT Service Status */
  uint8_t vt_service_status_valid;  /**< Must be set to true if vt_service_status is being passed */
  imsa_service_status_enum_v01 vt_service_status;
  /**<   Values: \n
       - 0 -- IMS VT service is not available \n
       - 2 -- IMS VT is in full service
  */

  /* Optional */
  /*  SMS RAT */
  uint8_t sms_service_rat_valid;  /**< Must be set to true if sms_service_rat is being passed */
  imsa_service_rat_enum_v01 sms_service_rat;
  /**<   SMS service RAT. Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  VoIP RAT */
  uint8_t voip_service_rat_valid;  /**< Must be set to true if voip_service_rat is being passed */
  imsa_service_rat_enum_v01 voip_service_rat;
  /**<   VoIP service RAT. Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  VT RAT */
  uint8_t vt_service_rat_valid;  /**< Must be set to true if vt_service_rat is being passed */
  imsa_service_rat_enum_v01 vt_service_rat;
  /**<   VT service RAT. Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  UT Service Status */
  uint8_t ut_service_status_valid;  /**< Must be set to true if ut_service_status is being passed */
  imsa_service_status_enum_v01 ut_service_status;
  /**<   UT service status. Values: \n
       - 0 -- IMS UT service is not available \n
       - 2 -- IMS UT is in full service
  */

  /* Optional */
  /*  UT RAT */
  uint8_t ut_service_rat_valid;  /**< Must be set to true if ut_service_rat is being passed */
  imsa_service_rat_enum_v01 ut_service_rat;
  /**<   UT service RAT. Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  VS Service Status */
  uint8_t vs_service_status_valid;  /**< Must be set to true if vs_service_status is being passed */
  imsa_service_status_enum_v01 vs_service_status;
  /**<   VS service status. Values: \n
       - 0 -- IMS UT service is not available \n
       - 2 -- IMS UT is in full service
  */

  /* Optional */
  /*  VS RAT */
  uint8_t vs_service_rat_valid;  /**< Must be set to true if vs_service_rat is being passed */
  imsa_service_rat_enum_v01 vs_service_rat;
  /**<   Video Share service RAT. Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */
}imsa_service_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent when the status of a RAT handover changes. */
typedef struct {

  /* Optional */
  /*  RAT Handover Status Information */
  uint8_t rat_ho_status_info_valid;  /**< Must be set to true if rat_ho_status_info is being passed */
  imsa_rat_handover_status_info_v01 rat_ho_status_info;
}imsa_rat_handover_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent when the status of a PDP connection changes. */
typedef struct {

  /* Mandatory */
  /*  IMS PDP Connection State */
  uint8_t is_ims_pdp_connected;
  /**<   IMS PDP connection state information.\n
     Values:\n
       - TRUE  -- IMS PDP is connected \n
       - FALSE -- IMS PDP is not connected
    */

  /* Optional */
  /*  IMS PDP Failure Error Code */
  uint8_t ims_pdp_failure_error_code_valid;  /**< Must be set to true if ims_pdp_failure_error_code is being passed */
  imsa_pdp_failure_error_code_enum_v01 ims_pdp_failure_error_code;
  /**<   IMS PDP connection failure error reason code when the
 IMS PDP Connection State TLV is FALSE (that is, IMS PDP is not in
 the Connected state). Values:\n
      - IMSA_PDP_STATUS_OTHER_FAILURE (0) --  Generic failure reason for other than specified \n 
      - IMSA_PDP_STATUS_OPTION_UNSUBSCRIBED (1) --  Option is unsubscribed \n 
      - IMSA_PDP_STATUS_UNKNOWN_PDP (2) --  PDP was unknown 
      - IMSA_PDP_STATUS_REASON_NOT_SPECIFIED (3) --  Reason not specified \n
      - IMSA_PDP_STATUS_CONNECTION_BRINGUP_FAILURE (4) --  Connection bring-up failure 
 */
}imsa_pdp_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent with the ACS retrieval status.	This indication
             currently sends the status only when the ACS retrieval fails. */
typedef struct {

  /* Mandatory */
  /*  ACS Failure Status Code */
  imsa_acs_failure_status_code_enum_v01 acs_failure_status_code;
  /**<   ACS failure status code when the ACS retrieval procedure fails.
 Values:\n
      - IMSA_ACS_FAILURE_NO_EXISTING_ACS_DATA (0) --  ACS failure; there is no earlier APCS success data, and
                       the ISIM parameters are not available \n 
      - IMSA_ACS_FAILURE_ERROR_CODE_RECEIVED (1) --  Error code received from the network for an
       ACS configuration retrieval request 
 */

  /* Optional */
  /*  ACS Failure Error Code Received */
  uint8_t acs_failure_error_code_valid;  /**< Must be set to true if acs_failure_error_code is being passed */
  uint16_t acs_failure_error_code;
  /**<   ACS failure error code received from the network for	a
       configuration retrieval request when the
       ACS retrieval procedure fails.
  */
}imsa_acs_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent when LTE attach parameters change. */
typedef struct {

  /* Mandatory */
  /*  LTE Attach Parameters  */
  uint8_t operator_reserved_pco;
  /**<   Operator-reserved PCO value for subscription status.
    */
}imsa_lte_attach_parameters_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent with the subscription status.  */
typedef struct {

  /* Mandatory */
  /*  Subscription Type */
  imsa_subscription_type_enum_v01 subscription_type;
  /**<   Subscription type. \n
 Values: \n
      - IMSA_SUBSCRIPTION_TYPE_REG (0) --  Register subscription\n 
      - IMSA_SUBSCRIPTION_TYPE_MWI (1) --  MWI subscription 
      - IMSA_SUBSCRIPTION_TYPE_CONFERENCE (2) --  Conference subscription 
 */

  /* Mandatory */
  /*  Subscription Status */
  uint8_t subscription_status;
  /**<   Subscription state information.\n
     Values:\n
       - TRUE  -- Subscription Success \n
       - FALSE -- Subscription Failure
    */

  /* Optional */
  /*  Subscription Error String */
  uint8_t subscription_error_string_valid;  /**< Must be set to true if subscription_error_string is being passed */
  char subscription_error_string[IMSA_SUBSCRIPTION_FAILURE_ERROR_STR_LEN_V01 + 1];
  /**<   Subscription failure error string
       when the subscription_status is FALSE. */

  /* Optional */
  /*  Subscription Error Code Received */
  uint8_t subscription_error_code_valid;  /**< Must be set to true if subscription_error_code is being passed */
  uint16_t subscription_error_code;
  /**<   Subscription error code received from the network 
       when the subscription_status is FALSE.*/
}imsa_subscription_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent with the network provisioning status.  */
typedef struct {

  /* Mandatory */
  /*  IMS Network Provisioning Status */
  imsa_network_provisioning_status_type_enum_v01 network_provisioning_status;
  /**<   IMS network provisioning status information. \n
 Values:\n
      - IMSA_NETWORK_PROV_OUT_OF_SYNC (0) --  Network is out of sync. This is an 
       indication sent when the IMS UE detects the network provisioning status through 
       SIP messages. When the network is out of sync with the UE, DM resync 
       may be triggered by the OEM based on this indication.
  
 */
}imsa_network_provisioning_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent when the RTP Statistics request is queried */
typedef struct {

  /* Optional */
  /*  RTP Statistics Parameter */
  uint8_t total_rx_expected_rtp_pkt_count_valid;  /**< Must be set to true if total_rx_expected_rtp_pkt_count is being passed */
  uint64_t total_rx_expected_rtp_pkt_count;
  /**<   Total number of packets expected on Rx 
       from the beginning of the call until the time of the request,
       measured across all opened sessions.
    */

  /* Optional */
  /*  RTP Statistics Parameter */
  uint8_t total_rx_rtp_pkt_loss_count_valid;  /**< Must be set to true if total_rx_rtp_pkt_loss_count is being passed */
  uint64_t total_rx_rtp_pkt_loss_count;
  /**<   Total number of packets lost on Rx 
       from the beginning of the call until the time of the request,
       measured across all the opened sessions.
    */
}imsa_rtp_statistics_ind_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_QMI_IMSA_ACS_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_GET_LTE_ATTACH_PARAMETERS_V01 
//#define REMOVE_QMI_IMSA_GET_PDP_STATUS_V01 
//#define REMOVE_QMI_IMSA_GET_REGISTRATION_STATUS_V01 
//#define REMOVE_QMI_IMSA_GET_RTP_STATISTICS_V01 
//#define REMOVE_QMI_IMSA_GET_SERVICE_STATUS_V01 
//#define REMOVE_QMI_IMSA_GET_SUPPORTED_FIELDS_V01 
//#define REMOVE_QMI_IMSA_GET_SUPPORTED_MSGS_V01 
//#define REMOVE_QMI_IMSA_IND_REG_V01 
//#define REMOVE_QMI_IMSA_LTE_ATTACH_PARAMETERS_IND_V01 
//#define REMOVE_QMI_IMSA_NETWORK_PROVISIONING_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_PDP_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_RAT_HANDOVER_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_REGISTRATION_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_RTP_STATISTICS_IND_V01 
//#define REMOVE_QMI_IMSA_SERVICE_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_SUBSCRIPTION_STATUS_IND_V01 

/*Service Message Definition*/
/** @addtogroup imsa_qmi_msg_ids
    @{
  */
#define QMI_IMSA_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_IMSA_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_IMSA_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_IMSA_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_IMSA_GET_REGISTRATION_STATUS_REQ_V01 0x0020
#define QMI_IMSA_GET_REGISTRATION_STATUS_RSP_V01 0x0020
#define QMI_IMSA_GET_SERVICE_STATUS_REQ_V01 0x0021
#define QMI_IMSA_GET_SERVICE_STATUS_RSP_V01 0x0021
#define QMI_IMSA_IND_REG_REQ_V01 0x0022
#define QMI_IMSA_IND_REG_RSP_V01 0x0022
#define QMI_IMSA_REGISTRATION_STATUS_IND_V01 0x0023
#define QMI_IMSA_SERVICE_STATUS_IND_V01 0x0024
#define QMI_IMSA_RAT_HANDOVER_STATUS_IND_V01 0x0025
#define QMI_IMSA_PDP_STATUS_IND_V01 0x0026
#define QMI_IMSA_ACS_STATUS_IND_V01 0x0027
#define QMI_IMSA_GET_PDP_STATUS_REQ_V01 0x0028
#define QMI_IMSA_GET_PDP_STATUS_RSP_V01 0x0028
#define QMI_IMSA_LTE_ATTACH_PARAMETERS_IND_V01 0x0029
#define QMI_IMSA_GET_LTE_ATTACH_PARAMETERS_REQ_V01 0x002A
#define QMI_IMSA_GET_LTE_ATTACH_PARAMETERS_RSP_V01 0x002A
#define QMI_IMSA_SUBSCRIPTION_STATUS_IND_V01 0x002B
#define QMI_IMSA_NETWORK_PROVISIONING_STATUS_IND_V01 0x002C
#define QMI_IMSA_GET_RTP_STATISTICS_REQ_V01 0x002D
#define QMI_IMSA_GET_RTP_STATISTICS_RSP_V01 0x002D
#define QMI_IMSA_RTP_STATISTICS_IND_V01 0x002E
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro imsa_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type imsa_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define imsa_get_service_object_v01( ) \
          imsa_get_service_object_internal_v01( \
            IMSA_V01_IDL_MAJOR_VERS, IMSA_V01_IDL_MINOR_VERS, \
            IMSA_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

