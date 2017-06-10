#ifndef WMS_SERVICE_01_H
#define WMS_SERVICE_01_H
/**
  @file wireless_messaging_service_v01.h
  
  @brief This is the public header file which defines the wms service Data structures.

  This header file defines the types and structures that were defined in 
  wms. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2009-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/wms/main/latest/api/wireless_messaging_service_v01.h#25 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It was generated on: Thu Jul 25 2013 (Spin 1)
   From IDL File: wireless_messaging_service_v01.idl */

/** @defgroup wms_qmi_consts Constant values defined in the IDL */
/** @defgroup wms_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup wms_qmi_enums Enumerated types used in QMI messages */
/** @defgroup wms_qmi_messages Structures sent as QMI messages */
/** @defgroup wms_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup wms_qmi_accessor Accessor for QMI service object */
/** @defgroup wms_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup wms_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define WMS_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define WMS_V01_IDL_MINOR_VERS 0x14
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define WMS_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define WMS_V01_MAX_MESSAGE_ID 0x005F;
/** 
    @} 
  */


/** @addtogroup wms_qmi_consts 
    @{ 
  */
#define WMS_MESSAGE_LENGTH_MAX_V01 255
#define WMS_MESSAGE_TUPLE_MAX_V01 255
#define WMS_ROUTE_TUPLE_MAX_V01 10
#define WMS_ETWS_MESSAGE_LENGTH_MAX_V01 1252
#define WMS_SMSC_ADDRESS_LENGTH_MAX_V01 11
#define WMS_DEST_ADDRESS_LENGTH_MAX_V01 12
#define WMS_ALPHA_ID_LENGTH_MAX_V01 255
#define WMS_ADDRESS_DIGIT_MAX_V01 21
#define WMS_ADDRESS_TYPE_MAX_V01 3
#define WMS_3GPP_BROADCAST_CONFIG_MAX_V01 50
#define WMS_3GPP2_BROADCAST_CONFIG_MAX_V01 50
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_reset_req_msg_v01;

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Resets the WMS service state variables of the requesting control
             point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_reset_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sets the WMS event reporting conditions for the control point. */
typedef struct {

  /* Optional */
  /*  New MT Message Indicator */
  uint8_t report_mt_message_valid;  /**< Must be set to true if report_mt_message is being passed */
  uint8_t report_mt_message;
  /**<   Report new MT messages. Values:\n
         - 0x00 -- Disable \n
         - 0x01 -- Enable 
    */

  /* Optional */
  /*  MO SMS Call Control Information */
  uint8_t report_call_control_info_valid;  /**< Must be set to true if report_call_control_info is being passed */
  uint8_t report_call_control_info;
  /**<   Report MO SMS call control information. Values: \n
         - 0x00 -- Disable \n
         - 0x01 -- Enable 
    */

  /* Optional */
  /*  MWI Message Indicator */
  uint8_t report_mwi_message_valid;  /**< Must be set to true if report_mwi_message is being passed */
  uint8_t report_mwi_message;
  /**<   Report new MWI messages. Values: \n
         - 0x00 -- Disable \n
         - 0x01 -- Enable 
    */
}wms_set_event_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sets the WMS event reporting conditions for the control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_set_event_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_STORAGE_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_STORAGE_TYPE_UIM_V01 = 0x00, 
  WMS_STORAGE_TYPE_NV_V01 = 0x01, 
  WMS_STORAGE_TYPE_NONE_V01 = -1, 
  WMS_STORAGE_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_storage_type_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_storage_type_enum_v01 storage_type;
  /**<   Memory storage. Values: \n
         - 0x00 -- STORAGE_TYPE_UIM \n
         - 0x01 -- STORAGE_TYPE_NV
    */

  uint32_t storage_index;
  /**<   MT message index*/
}wms_mt_message_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_ACK_INDICATOR_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_ACK_INDICATOR_SEND_ACK_V01 = 0x00, 
  WMS_ACK_INDICATOR_DO_NOT_SEND_ACK_V01 = 0x01, 
  WMS_ACK_INDICATOR_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_ack_indicator_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_MESSAGE_FORMAT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_MESSAGE_FORMAT_CDMA_V01 = 0x00, 
  WMS_MESSAGE_FORMAT_GW_PP_V01 = 0x06, 
  WMS_MESSAGE_FORMAT_GW_BC_V01 = 0x07, 
  WMS_MESSAGE_FORMAT_MWI_V01 = 0x08, 
  WMS_MESSAGE_FORMAT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_message_format_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_ack_indicator_enum_v01 ack_indicator;
  /**<   Parameter to indicate if ACK needs to be
         sent by the control point. Values: \n
         - 0x00 -- ACK_INDICATOR_SEND_ ACK -- Send ACK \n
         - 0x01 -- ACK_INDICATOR_DO_NOT_ SEND_ACK -- Do not send ACK 
    */

  uint32_t transaction_id;
  /**<   Transaction ID of the message*/

  wms_message_format_enum_v01 format;
  /**<   Message format. Values: \n
         - 0x00 -- MESSAGE_FORMAT_CDMA -- CDMA \n
         - 0x02 to 0x05 -- Reserved \n
         - 0x06 -- MESSAGE_FORMAT_GW_PP -- GW_PP \n
         - 0x07 -- MESSAGE_FORMAT_GW_ BC -- GW_BC 
    */

  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[WMS_MESSAGE_LENGTH_MAX_V01];
  /**<   Raw message data*/
}wms_transfer_route_mt_message_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_MESSAGE_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_MESSAGE_MODE_CDMA_V01 = 0x00, 
  WMS_MESSAGE_MODE_GW_V01 = 0x01, 
  WMS_MESSAGE_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_message_mode_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_ETWS_NOTIFICATION_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_ETWS_NOTIFICATION_TYPE_PRIMARY_V01 = 0x00, 
  WMS_ETWS_NOTIFICATION_TYPE_SECONDARY_GSM_V01 = 0x01, 
  WMS_ETWS_NOTIFICATION_TYPE_SECONDARY_UMTS_V01 = 0x02, 
  WMS_ETWS_NOTIFICATION_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_etws_notification_type_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_etws_notification_type_enum_v01 notification_type;
  /**<   Notification Type. Values: \n
         - 0x00 -- Primary \n
         - 0x01 -- Secondary GSM \n 
         - 0x02 -- Secondary UMTS 
    */

  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[WMS_ETWS_MESSAGE_LENGTH_MAX_V01];
  /**<   Raw message data*/
}wms_etws_message_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t mobile_country_code;
  /**<   16-bit integer representation of the MCC. Values: \n
       - 0 to 999
  */

  uint16_t mobile_network_code;
  /**<   16-bit integer representation of the MNC. Values:\n
       - 0 to 999
  */
}wms_etws_plmn_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[WMS_SMSC_ADDRESS_LENGTH_MAX_V01];
  /**<   SMSC Address*/
}wms_mt_message_smsc_address_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_MO_CONTROL_INFO_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_MO_CONTROL_DISALLOW_V01 = 0x00, /**<  Disallow the MO message \n  */
  WMS_MO_CONTROL_ALLOW_V01 = 0x01, /**<  Allow the MO message with no modification \n  */
  WMS_MO_CONTROL_ALLOW_BUT_MODIFIED_V01 = 0x02, /**<  Allow the MO message with modification  */
  WMS_MO_CONTROL_INFO_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_mo_control_info_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_mo_control_info_enum_v01 mo_control_type;
  /**<   MO SMS control. Values: \n
      - WMS_MO_CONTROL_DISALLOW (0x00) --  Disallow the MO message \n 
      - WMS_MO_CONTROL_ALLOW (0x01) --  Allow the MO message with no modification \n 
      - WMS_MO_CONTROL_ALLOW_BUT_MODIFIED (0x02) --  Allow the MO message with modification 
 */

  uint32_t alpha_id_len;  /**< Must be set to # of elements in alpha_id */
  uint8_t alpha_id[WMS_ALPHA_ID_LENGTH_MAX_V01];
  /**<   Alpha ID. */
}wms_call_control_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Indication Message; Indicates a QMI_WMS event. */
typedef struct {

  /* Optional */
  /*  MT Message */
  uint8_t mt_message_valid;  /**< Must be set to true if mt_message is being passed */
  wms_mt_message_type_v01 mt_message;

  /* Optional */
  /*  Transfer Route MT Message */
  uint8_t transfer_route_mt_message_valid;  /**< Must be set to true if transfer_route_mt_message is being passed */
  wms_transfer_route_mt_message_type_v01 transfer_route_mt_message;

  /* Optional */
  /*  Message Mode */
  uint8_t message_mode_valid;  /**< Must be set to true if message_mode is being passed */
  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Values: \n
         - 0x00 -- MESSAGE_MODE_CDMA -- CDMA \n
         - 0x01 -- MESSAGE_MODE_GW -- GW
    */

  /* Optional */
  /*  ETWS Message */
  uint8_t etws_message_valid;  /**< Must be set to true if etws_message is being passed */
  wms_etws_message_type_v01 etws_message;

  /* Optional */
  /*  ETWS PLMN Information */
  uint8_t etws_plmn_info_valid;  /**< Must be set to true if etws_plmn_info is being passed */
  wms_etws_plmn_info_type_v01 etws_plmn_info;

  /* Optional */
  /*  SMSC Address */
  uint8_t mt_message_smsc_address_valid;  /**< Must be set to true if mt_message_smsc_address is being passed */
  wms_mt_message_smsc_address_type_v01 mt_message_smsc_address;

  /* Optional */
  /*  SMS on IMS */
  uint8_t sms_on_ims_valid;  /**< Must be set to true if sms_on_ims is being passed */
  uint8_t sms_on_ims;
  /**<   Indicates whether the message is received from IMS. Values: \n
         - 0x00 -- Message is not received from IMS \n
         - 0x01 -- Message is received from IMS \n
         - 0x02 to 0xFF -- Reserved \n
         Note: In minor version 9, the implementation was changed in such
               a way that this TLV may be included at times when it
               previously may not have been included.
    */

  /* Optional */
  /*  Call Control Result */
  uint8_t call_control_info_valid;  /**< Must be set to true if call_control_info is being passed */
  wms_call_control_info_type_v01 call_control_info;
}wms_event_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_message_format_enum_v01 format;
  /**<   Message format. Values: \n 
         - 0x00 -- MESSAGE_FORMAT_CDMA -- CDMA \n 
         - 0x02 to 0x05 -- Reserved \n 
         - 0x06 -- MESSAGE_FORMAT_GW_PP -- GW_PP
    */

  uint32_t raw_message_len;  /**< Must be set to # of elements in raw_message */
  uint8_t raw_message[WMS_MESSAGE_LENGTH_MAX_V01];
  /**<   Raw message data*/
}wms_send_raw_message_data_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_SO_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_SO_AUTO_V01 = 0x00, 
  WMS_SO_6_V01 = 0x06, 
  WMS_SO_14_V01 = 0x0E, 
  WMS_SO_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_so_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t force_on_dc;
  /**<   Force the message to be sent on the CDMA dedicated channel. Values: \n
         - 0x00 -- Do not care about the channel on which the message is sent \n
         - 0x01 -- Request to send the message over the dedicated channel
    */

  wms_so_enum_v01 so;
  /**<   Service option. Values: \n
         - 0x00 -- SO_AUTO -- AUTO (choose the best service option while setting
           up the DC) \n
         - 0x06 -- SO_6 -- Service option 6 \n
         - 0x0E -- SO_14 -- Service option 14
    */
}wms_force_on_dc_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_FOLLOW_ON_DC_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_FOLLOW_ON_DC_ON_V01 = 0x01, 
  WMS_FOLLOW_ON_DC_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_follow_on_dc_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_RETRY_MESSAGE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_MESSAGE_IS_A_RETRY_V01 = 0x01, 
  WMS_RETRY_MESSAGE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_retry_message_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sends a new message in its raw format. */
typedef struct {

  /* Mandatory */
  /*  Raw Message Data */
  wms_send_raw_message_data_type_v01 raw_message_data;

  /* Optional */
  /*  Force on DC* */
  uint8_t force_on_dc_valid;  /**< Must be set to true if force_on_dc is being passed */
  wms_force_on_dc_type_v01 force_on_dc;

  /* Optional */
  /*  Follow on DC* */
  uint8_t follow_on_dc_valid;  /**< Must be set to true if follow_on_dc is being passed */
  wms_follow_on_dc_enum_v01 follow_on_dc;
  /**<   Flag to request to not disconnect the CDMA
         dedicated channel after the send operation is
         completed; this TLV can be included if
         more messages are expected to follow. Values: \n       
         - 0x01 -- FOLLOW_ON_DC_ON -- On (do not disconnect the DC after
         the send operation) \n 
         Any value other than 0x01 in this field is
         treated as an absence of this TLV.
    */

  /* Optional */
  /*  Link Control** */
  uint8_t link_timer_valid;  /**< Must be set to true if link_timer is being passed */
  uint8_t link_timer;
  /**<   Keeps the GW SMS link open for the specified number of seconds;
         can be enabled if more messages are expected to follow
    */

  /* Optional */
  /*  SMS on IMS */
  uint8_t sms_on_ims_valid;  /**< Must be set to true if sms_on_ims is being passed */
  uint8_t sms_on_ims;
  /**<   Indicates whether the message is to be sent on IMS. Values: \n
         - 0x00 -- Message is not to be sent on IMS \n
         - 0x01 -- Message is to be sent on IMS \n
         - 0x02 to 0xFF -- Reserved \n
         Note: In minor version 9, the implementation was changed  in such a way that
               inclusion of this TLV may affect the SMS routing differently.
         
    */

  /* Optional */
  /*  Retry Message */
  uint8_t retry_message_valid;  /**< Must be set to true if retry_message is being passed */
  wms_retry_message_enum_v01 retry_message;
  /**<   Indicates this message is a retry message. Values: \n
         - 0x01 -- WMS_MESSAGE_IS_A_ RETRY -- Message is a retry message	\n
          Note: Any value other than 0x01 in this field is treated as an absence
          of this TLV.
    */

  /* Optional */
  /*  Retry Message ID */
  uint8_t retry_message_id_valid;  /**< Must be set to true if retry_message_id is being passed */
  uint32_t retry_message_id;
  /**<   Message ID to be used in the retry message. 
         The message ID specified here is used instead of the messsage ID
		 encoded in the raw message. \n
		 Note: This TLV is valid only if the Retry Message TLV is specified and
		 set to 0x01.
    */

  /* Optional */
  /*  Link Control Enabling Information** */
  uint8_t link_enable_mode_valid;  /**< Must be set to true if link_enable_mode is being passed */
  uint8_t link_enable_mode;
  /**<   Indicates whether to keep the link control enabled, until the option is
         modified by the client. Values: \n
         - 0x00 -- Enable link control once so that the lower layer keeps the
                   link up for a specified time until the next MO SMS is requested
                   or the timer expires \n
         - 0x01 -- Always enable link control \n
         Note: This TLV is valid only if the Link Control TLV is specified 
         and is set to a valid timer value.
    */
}wms_raw_send_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_RP_CAUSE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_RP_CAUSE_UNASSIGNED_NUMBER_V01 = 0x01, 
  WMS_RP_CAUSE_OPERATOR_DETERMINED_BARRING_V01 = 0x08, 
  WMS_RP_CAUSE_CALL_BARRED_V01 = 0x0A, 
  WMS_RP_CAUSE_RESERVED_V01 = 0x0B, 
  WMS_RP_CAUSE_SMS_TRANSFER_REJECTED_V01 = 0x15, 
  WMS_RP_CAUSE_MEMORY_CAP_EXCEEDED_V01 = 0x16, 
  WMS_RP_CAUSE_DESTINATION_OUT_OF_ORDER_V01 = 0x1B, 
  WMS_RP_CAUSE_UNIDENTIFIED_SUBSCRIBER_V01 = 0x1C, 
  WMS_RP_CAUSE_FACILITY_REJECTED_V01 = 0x1D, 
  WMS_RP_CAUSE_UNKNOWN_SUBSCRIBER_V01 = 0x1E, 
  WMS_RP_CAUSE_NETWORK_OUT_OF_ORDER_V01 = 0x26, 
  WMS_RP_CAUSE_TEMPORARY_FAILURE_V01 = 0x29, 
  WMS_RP_CAUSE_CONGESTION_V01 = 0x2A, 
  WMS_RP_CAUSE_RESOURCES_UNAVAILABLE_V01 = 0x2F, 
  WMS_RP_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED_V01 = 0x32, 
  WMS_RP_CAUSE_REQUESTED_FACILITY_NOT_IMPLEMENTED_V01 = 0x45, 
  WMS_RP_CAUSE_INVALID_SMS_TRANSFER_REFERENCE_VALUE_V01 = 0x51, 
  WMS_RP_CAUSE_SEMANTICALLY_INCORRECT_MESSAGE_V01 = 0x5F, 
  WMS_RP_CAUSE_INVALID_MANDATORY_INFO_V01 = 0x60, 
  WMS_RP_CAUSE_MESSAGE_TYPE_NOT_IMPLEMENTED_V01 = 0x61, 
  WMS_RP_CAUSE_MESSAGE_NOT_COMPATABLE_WITH_SMS_V01 = 0x62, 
  WMS_RP_CAUSE_INFO_ELEMENT_NOT_IMPLEMENTED_V01 = 0x63, 
  WMS_RP_CAUSE_PROTOCOL_ERROR_V01 = 0x6F, 
  WMS_RP_CAUSE_INTERWORKING_V01 = 0x7F, 
  WMS_RP_CAUSE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_rp_cause_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_TP_CAUSE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_TP_CAUSE_TELE_INTERWORKING_NOT_SUPPORTED_V01 = -128, 
  WMS_TP_CAUSE_SHORT_MESSAGE_TYPE_0_NOT_SUPPORTED_V01 = -127, 
  WMS_TP_CAUSE_SHORT_MESSAGE_CANNOT_BE_REPLACED_V01 = -126, 
  WMS_TP_CAUSE_UNSPECIFIED_PID_ERROR_V01 = -113, 
  WMS_TP_CAUSE_DCS_NOT_SUPPORTED_V01 = -112, 
  WMS_TP_CAUSE_MESSAGE_CLASS_NOT_SUPPORTED_V01 = -111, 
  WMS_TP_CAUSE_UNSPECIFIED_DCS_ERROR_V01 = -97, 
  WMS_TP_CAUSE_COMMAND_CANNOT_BE_ACTIONED_V01 = -96, 
  WMS_TP_CAUSE_COMMAND_UNSUPPORTED_V01 = -95, 
  WMS_TP_CAUSE_UNSPECIFIED_COMMAND_ERROR_V01 = -81, 
  WMS_TP_CAUSE_TPDU_NOT_SUPPORTED_V01 = -80, 
  WMS_TP_CAUSE_SC_BUSY_V01 = -64, 
  WMS_TP_CAUSE_NO_SC_SUBSCRIPTION_V01 = -63, 
  WMS_TP_CAUSE_SC_SYS_FAILURE_V01 = -62, 
  WMS_TP_CAUSE_INVALID_SME_ADDRESS_V01 = -61, 
  WMS_TP_CAUSE_DESTINATION_SME_BARRED_V01 = -60, 
  WMS_TP_CAUSE_SM_REJECTED_OR_DUPLICATE_V01 = -59, 
  WMS_TP_CAUSE_TP_VPF_NOT_SUPPORTED_V01 = -58, 
  WMS_TP_CAUSE_TP_VP_NOT_SUPPORTED_V01 = -57, 
  WMS_TP_CAUSE_SIM_SMS_STORAGE_FULL_V01 = -48, 
  WMS_TP_CAUSE_NO_SMS_STORAGE_CAP_IN_SIM_V01 = -47, 
  WMS_TP_CAUSE_MS_ERROR_V01 = -46, 
  WMS_TP_CAUSE_MEMORY_CAP_EXCEEDED_V01 = -45, 
  WMS_TP_CAUSE_SIM_APP_TOOLKIT_BUSY_V01 = -44, 
  WMS_TP_CAUSE_SIM_DATA_DOWNLOAD_ERROR_V01 = -43, 
  WMS_TP_CAUSE_UNSPECIFIED_ERROR_V01 = -1, 
  WMS_TP_CAUSE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_tp_cause_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_rp_cause_enum_v01 rp_cause;
  /**<   GW RP cause per \hyperref[S5]{[S5]} Section 8.2.5.4;
         see Table @latexonly \ref{tbl:GWRPCauseCodes} @endlatexonly
         for more information.
    */

  wms_tp_cause_enum_v01 tp_cause;
  /**<   GW TP cause per \hyperref[S2]{[S2]} Section 9.2.3.22; see Table
          @latexonly \ref{tbl:GWTPCauseCodes} @endlatexonly for more 
          information.
    */
}wms_gw_cause_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_ERROR_CLASS_SEND_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_ERROR_CLASS_TEMPORARY_V01 = 0x00, 
  WMS_ERROR_CLASS_PERMANENT_V01 = 0x01, 
  WMS_ERROR_CLASS_SEND_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_error_class_send_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_TL_CAUSE_CODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_TL_CAUSE_CODE_ADDR_VACANT_V01 = 0x00, 
  WMS_TL_CAUSE_CODE_ADDR_TRANSLATION_FAILURE_V01 = 0x01, 
  WMS_TL_CAUSE_CODE_NETWORK_RESOURCE_SHORTAGE_V01 = 0x02, 
  WMS_TL_CAUSE_CODE_NETWORK_FAILURE_V01 = 0x03, 
  WMS_TL_CAUSE_CODE_INVALID_TELESERVICE_ID_V01 = 0x04, 
  WMS_TL_CAUSE_CODE_NETWORK_OTHER_V01 = 0x05, 
  WMS_TL_CAUSE_CODE_NO_PAGE_RESPONSE_V01 = 0x20, 
  WMS_TL_CAUSE_CODE_DEST_BUSY_V01 = 0x21, 
  WMS_TL_CAUSE_CODE_NO_ACK_V01 = 0x22, 
  WMS_TL_CAUSE_CODE_DEST_RESOURCE_SHORTAGE_V01 = 0x23, 
  WMS_TL_CAUSE_CODE_SMS_DELIVERY_POSTPONED_V01 = 0x24, 
  WMS_TL_CAUSE_CODE_DEST_OUT_OF_SERV_V01 = 0x25, 
  WMS_TL_CAUSE_CODE_DEST_NOT_AT_ADDR_V01 = 0x26, 
  WMS_TL_CAUSE_CODE_DEST_OTHER_V01 = 0x27, 
  WMS_TL_CAUSE_CODE_RADIO_IF_RESOURCE_SHORTAGE_V01 = 0x40, 
  WMS_TL_CAUSE_CODE_RADIO_IF_INCOMPATABILITY_V01 = 0x41, 
  WMS_TL_CAUSE_CODE_RADIO_IF_OTHER_V01 = 0x42, 
  WMS_TL_CAUSE_CODE_ENCODING_V01 = 0x60, 
  WMS_TL_CAUSE_CODE_SMS_ORIG_DENIED_V01 = 0x61, 
  WMS_TL_CAUSE_CODE_SMS_TERM_DENIED_V01 = 0x62, 
  WMS_TL_CAUSE_CODE_SUPP_SERV_NOT_SUPP_V01 = 0x63, 
  WMS_TL_CAUSE_CODE_SMS_NOT_SUPP_V01 = 0x64, 
  WMS_TL_CAUSE_CODE_MISSING_EXPECTED_PARAM_V01 = 0x65, 
  WMS_TL_CAUSE_CODE_MISSING_MAND_PARAM_V01 = 0x66, 
  WMS_TL_CAUSE_CODE_UNRECOGNIZED_PARAM_VAL_V01 = 0x67, 
  WMS_TL_CAUSE_CODE_UNEXPECTED_PARAM_VAL_V01 = 0x68, 
  WMS_TL_CAUSE_CODE_USER_DATA_SIZE_ERR_V01 = 0x69, 
  WMS_TL_CAUSE_CODE_GENERAL_OTHER_V01 = 0x6A, 
  WMS_TL_CAUSE_CODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_tl_cause_code_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_MESSAGE_DELIVERY_FAILURE_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_MESSAGE_DELIVERY_FAILURE_TEMPORARY_V01 = 0x00, 
  WMS_MESSAGE_DELIVERY_FAILURE_PERMANENT_V01 = 0x01, 
  WMS_MESSAGE_DELIVERY_FAILURE_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_message_delivery_failure_type_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_MESSAGE_DELIVERY_FAILURE_CAUSE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_MESSAGE_BLOCKED_DUE_TO_CALL_CONTROL_V01 = 0x00, 
  WMS_MESSAGE_DELIVERY_FAILURE_CAUSE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_message_delivery_failure_cause_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t alpha_id_len;  /**< Must be set to # of elements in alpha_id */
  uint8_t alpha_id[WMS_ALPHA_ID_LENGTH_MAX_V01];
  /**<   Alpha ID.*/
}wms_call_control_modified_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sends a new message in its raw format. */
typedef struct {

  /* Mandatory */
  /*  Message ID */
  uint16_t message_id;
  /**<   WMS message ID*/

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Cause Code* */
  uint8_t cause_code_valid;  /**< Must be set to true if cause_code is being passed */
  wms_tl_cause_code_enum_v01 cause_code;
  /**<   WMS cause code per \hyperref[S4]{[S4]} Section 6.5.2.125; see
         Table @latexonly \ref{tbl:WMSCauseCodes} @endlatexonly for more 
         information
    */

  /* Optional */
  /*  Error Class* */
  uint8_t error_class_valid;  /**< Must be set to true if error_class is being passed */
  wms_error_class_send_enum_v01 error_class;
  /**<   Error class. Values: \n
       - 0x00 -- ERROR_CLASS_ TEMPORARY \n
       - 0x01 -- ERROR_CLASS_ PERMANENT 
  */

  /* Optional */
  /*  GW Cause Info** */
  uint8_t gw_cause_info_valid;  /**< Must be set to true if gw_cause_info is being passed */
  wms_gw_cause_info_type_v01 gw_cause_info;

  /* Optional */
  /*  Message Delivery Failure Type */
  uint8_t message_delivery_failure_type_valid;  /**< Must be set to true if message_delivery_failure_type is being passed */
  wms_message_delivery_failure_type_enum_v01 message_delivery_failure_type;
  /**<   Message delivery failure type. Values: \n
       - 0x00 -- WMS_MESSAGE_ DELIVERY_FAILURE_TEMPORARY \n
       - 0x01 -- WMS_MESSAGE_ DELIVERY_FAILURE_PERMANENT
  */

  /* Optional */
  /*  Message Delivery Failure Cause */
  uint8_t message_delivery_failure_cause_valid;  /**< Must be set to true if message_delivery_failure_cause is being passed */
  wms_message_delivery_failure_cause_enum_v01 message_delivery_failure_cause;
  /**<   Message delivery failure cause.  Values: \n
       - 0x00 -- WMS_MESSAGE_ BLOCKED_DUE_TO_CALL_ CONTROL
    */

  /* Optional */
  /*  Call Control Modified Information** */
  uint8_t call_control_modified_info_valid;  /**< Must be set to true if call_control_modified_info is being passed */
  wms_call_control_modified_info_type_v01 call_control_modified_info;
}wms_raw_send_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_storage_type_enum_v01 storage_type;
  /**<   Memory storage. Values: \n
         - 0x00 -- STORAGE_TYPE_UIM -- UIM \n
         - 0x01 -- STORAGE_TYPE_NV  -- NV
    */

  wms_message_format_enum_v01 format;
  /**<   Message format. Values: \n
         - 0x00 -- MESSAGE_FORMAT_CDMA  -- CDMA \n
         - 0x02 to 0x05 -- Reserved \n
         - 0x06 -- MESSAGE_FORMAT_GW_PP -- GW_PP
    */

  uint32_t raw_message_len;  /**< Must be set to # of elements in raw_message */
  uint8_t raw_message[WMS_MESSAGE_LENGTH_MAX_V01];
  /**<   Raw message buffer*/
}wms_raw_message_write_data_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_MESSAGE_TAG_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_TAG_TYPE_MT_READ_V01 = 0x00, 
  WMS_TAG_TYPE_MT_NOT_READ_V01 = 0x01, 
  WMS_TAG_TYPE_MO_SENT_V01 = 0x02, 
  WMS_TAG_TYPE_MO_NOT_SENT_V01 = 0x03, 
  WMS_MESSAGE_TAG_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_message_tag_type_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Writes a new message given in its raw format. */
typedef struct {

  /* Mandatory */
  /*  Raw Message Write Data */
  wms_raw_message_write_data_type_v01 raw_message_write_data;

  /* Optional */
  /*  Message Tag */
  uint8_t tag_type_valid;  /**< Must be set to true if tag_type is being passed */
  wms_message_tag_type_enum_v01 tag_type;
  /**<   Message tag. Values: \n
         - 0x00 -- TAG_TYPE_MT_READ \n 
         - 0x01 -- TAG_TYPE_MT_NOT_READ \n 
         - 0x02 -- TAG_TYPE_MO_SENT \n 
         - 0x03 -- TAG_TYPE_MO_NOT_SENT  
    */
}wms_raw_write_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Writes a new message given in its raw format. */
typedef struct {

  /* Mandatory */
  /*  Message Memory Storage Identification */
  uint32_t storage_index;
  /**<   Memory index*/

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_raw_write_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_storage_type_enum_v01 storage_type;
  /**<   Memory storage. Values: \n 
         - 0x00 -- STORAGE_TYPE_UIM -- UIM \n
         - 0x01 -- STORAGE_TYPE_NV  -- NV
    */

  uint32_t storage_index;
  /**<   Memory index*/
}wms_message_memory_storage_identification_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Reads a message from the device memory storage and returns the
             message in its raw format. */
typedef struct {

  /* Mandatory */
  /*  Message Memory Storage Identification */
  wms_message_memory_storage_identification_type_v01 message_memory_storage_identification;

  /* Optional */
  /*  Message Mode */
  uint8_t message_mode_valid;  /**< Must be set to true if message_mode is being passed */
  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Values: \n 
         - 0x00 -- MESSAGE_MODE_CDMA -- CDMA \n 
         - 0x01 -- MESSAGE_MODE_GW   -- GW
    */

  /* Optional */
  /*  SMS on IMS */
  uint8_t sms_on_ims_valid;  /**< Must be set to true if sms_on_ims is being passed */
  uint8_t sms_on_ims;
  /**<   Indicates whether the message is to be read from IMS. Values: \n 
         - 0x00 -- Message is not to be read from IMS \n 
         - 0x01 -- Message is to be read from IMS \n 
         - 0x02 to 0xFF -- Reserved \n
         Note: This TLV is deprecated from minor version 9.
    */
}wms_raw_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_message_tag_type_enum_v01 tag_type;
  /**<   Message tag. Value: \n 
         - 0x00 -- TAG_TYPE_MT_READ \n 
         - 0x01 -- TAG_TYPE_MT_NOT_READ \n 
         - 0x02 -- TAG_TYPE_MO_SENT \n 
         - 0x03 -- TAG_TYPE_MO_NOT_SENT 
    */

  wms_message_format_enum_v01 format;
  /**<   Message format. Value: \n 
         - 0x00 -- MESSAGE_FORMAT_CDMA  -- CDMA \n 
         - 0x02 to 0x05 -- Reserved \n 
         - 0x06 -- MESSAGE_FORMAT_GW_PP -- GW_PP \n 
         - 0x08 - MESSAGE_FORMAT_MWI    -- MWI
    */

  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[WMS_MESSAGE_LENGTH_MAX_V01];
  /**<   Raw message data*/
}wms_read_raw_message_data_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Reads a message from the device memory storage and returns the
             message in its raw format. */
typedef struct {

  /* Mandatory */
  /*  Raw Message Data */
  wms_read_raw_message_data_type_v01 raw_message_data;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_raw_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_storage_type_enum_v01 storage_type;
  /**<   Memory storage. Values: \n 
         - 0x00 -- STORAGE_TYPE_UIM \n 
         - 0x01 -- STORAGE_TYPE_NV
    */

  uint32_t storage_index;
  /**<   Memory index*/

  wms_message_tag_type_enum_v01 tag_type;
  /**<   Message tag. Values: \n 
         - 0x00 -- TAG_TYPE_MT_READ \n 
         - 0x01 -- TAG_TYPE_MT_NOT_READ \n 
         - 0x02 -- TAG_TYPE_MO_SENT \n 
         - 0x03 -- TAG_TYPE_MO_NOT_SENT
    */
}wms_message_tag_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Modifies the metadata tag of a message in the MSM
             device storage. */
typedef struct {

  /* Mandatory */
  /*  WMS Message Tag */
  wms_message_tag_type_v01 wms_message_tag;

  /* Optional */
  /*  Message Mode */
  uint8_t message_mode_valid;  /**< Must be set to true if message_mode is being passed */
  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Values: \n 
         - 0x00 -- MESSAGE_MODE_CDMA -- CDMA \n 
         - 0x01 -- MESSAGE_MODE_GW -- GW
    */
}wms_modify_tag_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Modifies the metadata tag of a message in the MSM
             device storage. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_modify_tag_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Deletes the message in a specified memory location. */
typedef struct {

  /* Mandatory */
  /*  Memory Storage */
  wms_storage_type_enum_v01 storage_type;
  /**<   Memory storage. Values: \n 
         - 0x00 -- STORAGE_TYPE_UIM \n 
         - 0x01 -- STORAGE_TYPE_NV
    */

  /* Optional */
  /*  Memory Index */
  uint8_t index_valid;  /**< Must be set to true if index is being passed */
  uint32_t index;
  /**<   Indicates the storage index of the message of
 interest*/

  /* Optional */
  /*  Message Tag */
  uint8_t tag_type_valid;  /**< Must be set to true if tag_type is being passed */
  wms_message_tag_type_enum_v01 tag_type;
  /**<   Message tag. Values: \n
         - 0x00 -- TAG_TYPE_MT_READ \n 
         - 0x01 -- TAG_TYPE_MT_NOT_READ \n 
         - 0x02 -- TAG_TYPE_MO_SENT \n 
         - 0x03 -- TAG_TYPE_MO_NOT_SENT  
    */

  /* Optional */
  /*  Message Mode */
  uint8_t message_mode_valid;  /**< Must be set to true if message_mode is being passed */
  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Values: \n 
         - 0x00 -- MESSAGE_MODE_CDMA -- CDMA \n 
         - 0x01 -- MESSAGE_MODE_GW -- GW
    */
}wms_delete_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Deletes the message in a specified memory location. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_delete_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_message_protocol_req_msg_v01;

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_MESSAGE_PROTOCOL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_MESSAGE_PROTOCOL_CDMA_V01 = 0x00, 
  WMS_MESSAGE_PROTOCOL_WCDMA_V01 = 0x01, 
  WMS_MESSAGE_PROTOCOL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_message_protocol_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Queries the message protocol currently in use for the WMS client. */
typedef struct {

  /* Mandatory */
  /*  Message Protocol */
  wms_message_protocol_enum_v01 message_protocol;
  /**<   WMS message protocol. Values: \n 
         - 0x00 -- MESSAGE_PROTOCOL_ CDMA \n 
         - 0x01 -- MESSAGE_PROTOCOL_ WCDMA
    */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_get_message_protocol_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Requests a list of WMS message indices and meta information
             within the specified memory storage, matching a specified
             message tag. */
typedef struct {

  /* Mandatory */
  /*  Requested Memory Storage */
  wms_storage_type_enum_v01 storage_type;
  /**<   Memory storage. Values: \n 
         - 0x00 -- STORAGE_TYPE_UIM \n 
         - 0x01 -- STORAGE_TYPE_NV
    */

  /* Optional */
  /*  Requested Tag */
  uint8_t tag_type_valid;  /**< Must be set to true if tag_type is being passed */
  wms_message_tag_type_enum_v01 tag_type;
  /**<   Message tag. Values: \n 
         - 0x00 -- TAG_TYPE_MT_READ \n 
         - 0x01 -- TAG_TYPE_MT_NOT_READ \n 
         - 0x02 -- TAG_TYPE_MO_SENT \n 
         - 0x03 -- TAG_TYPE_MO_NOT_SENT
    */

  /* Optional */
  /*  Message Mode */
  uint8_t message_mode_valid;  /**< Must be set to true if message_mode is being passed */
  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Values: \n 
         - 0x00 -- MESSAGE_MODE_CDMA -- CDMA \n 
         - 0x01 -- MESSAGE_MODE_GW -- GW
    */
}wms_list_messages_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t message_index;
  /**<   Message index of each matched message*/

  wms_message_tag_type_enum_v01 tag_type;
  /**<   Message tag. Values: \n 
         - 0x00 -- TAG_TYPE_MT_READ \n 
         - 0x01 -- TAG_TYPE_MT_NOT_READ \n 
         - 0x02 -- TAG_TYPE_MO_SENT \n 
         - 0x03 -- TAG_TYPE_MO_NOT_SENT
    */
}wms_message_tuple_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Requests a list of WMS message indices and meta information
             within the specified memory storage, matching a specified
             message tag. */
typedef struct {

  /* Mandatory */
  /*  Message List */
  uint32_t message_tuple_len;  /**< Must be set to # of elements in message_tuple */
  wms_message_tuple_type_v01 message_tuple[WMS_MESSAGE_TUPLE_MAX_V01];

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_list_messages_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_MESSAGE_CLASS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_MESSAGE_CLASS_0_V01 = 0x00, 
  WMS_MESSAGE_CLASS_1_V01 = 0x01, 
  WMS_MESSAGE_CLASS_2_V01 = 0x02, 
  WMS_MESSAGE_CLASS_3_V01 = 0x03, 
  WMS_MESSAGE_CLASS_NONE_V01 = 0x04, 
  WMS_MESSAGE_CLASS_CDMA_V01 = 0x05, 
  WMS_MESSAGE_CLASS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_message_class_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_RECEIPT_ACTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_DISCARD_V01 = 0x00, 
  WMS_STORE_AND_NOTIFY_V01 = 0x01, 
  WMS_TRANSFER_ONLY_V01 = 0x02, 
  WMS_TRANSFER_AND_ACK_V01 = 0x03, 
  WMS_UNKNOWN_V01 = -1, 
  WMS_RECEIPT_ACTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_receipt_action_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_MESSAGE_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_MESSAGE_TYPE_POINT_TO_POINT_V01 = 0x00, 
  WMS_MESSAGE_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_message_type_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_message_type_enum_v01 message_type;
  /**<   Message type matching this route. Values: \n 
       - 0x00 -- MESSAGE_TYPE_POINT_ TO_POINT - Point-to-Point
  */

  wms_message_class_enum_v01 message_class;
  /**<   Message class. Values: \n 
       - 0x00 -- MESSAGE_CLASS_0    -- \n Class 0 \n 
       - 0x01 -- MESSAGE_CLASS_1    -- \n Class 1 \n 
       - 0x02 -- MESSAGE_CLASS_2    -- \n Class 2 \n 
       - 0x03 -- MESSAGE_CLASS_3    -- \n Class 3 \n 
       - 0x04 -- MESSAGE_CLASS_NONE -- Class None \n 
       - 0x05 -- MESSAGE_CLASS_CDMA -- Class CDMA
  */

  wms_storage_type_enum_v01 route_storage;
  /**<   If the action is store, where to store the incoming
       message. Values: \n 
       - 0x00 -- STORAGE_TYPE_UIM \n 
       - 0x01 -- STORAGE_TYPE_NV \n 
       -   -1 -- STORAGE_TYPE_NONE
  */

  wms_receipt_action_enum_v01 receipt_action;
  /**<   Action to be taken on receipt of a message
       matching the specified type and class for this route. Values: \n  
       - 0x00 -- DISCARD          -- Incoming messages for this route are
                                     discarded by the WMS service without 
                                     notifying QMI_WMS clients \n 
       - 0x01 -- STORE_AND_NOTIFY -- Incoming messages for this route are
                                     stored to the specified device 
                                     memory, and new message notifications
                                     are sent to registered clients \n 
       - 0x02 -- TRANSFER_ONLY    -- Incoming messages for this route are
                                     transferred to the client, and the
                                     client is expected to send ACK to
                                     the network \n 
       - 0x03 -- TRANSFER_AND_ACK -- Incoming messages for this route are
                                     transferred to the client, and ACK is
                                     sent to the network
  */
}wms_set_route_list_tuple_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_TRANSFER_IND_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_TRANSFER_IND_SIM_V01 = 0x00, 
  WMS_TRANSFER_IND_CLIENT_V01 = 0x01, 
  WMS_TRANSFER_IND_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_transfer_ind_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sets the action performed upon WMS message receipt for the
             specified message routes. It also sets the action performed
             upon WMS receipt of status reports.
             \label{idl:setRoutes} */
typedef struct {

  /* Mandatory */
  /*  Route List */
  uint32_t route_list_tuple_len;  /**< Must be set to # of elements in route_list_tuple */
  wms_set_route_list_tuple_type_v01 route_list_tuple[WMS_ROUTE_TUPLE_MAX_V01];

  /* Optional */
  /*  Transfer Status Report** */
  uint8_t transfer_ind_valid;  /**< Must be set to true if transfer_ind is being passed */
  wms_transfer_ind_enum_v01 transfer_ind;
  /**<   Values: \n
       - 0x01 -- TRANSFER_IND_CLIENT -- Status reports are transferred to 
         the client
 */
}wms_set_routes_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sets the action performed upon WMS message receipt for the
             specified message routes. It also sets the action performed
             upon WMS receipt of status reports.
             \label{idl:setRoutes} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_set_routes_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_routes_req_msg_v01;

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_message_type_enum_v01 route_type;
  /**<   Message type matching this route. Values: \n 
       - 0x00 -- MESSAGE_TYPE_POINT_ TO_POINT -- Point-to-Point
  */

  wms_message_class_enum_v01 route_class;
  /**<   Message class. Values: \n 
       - 0x00 -- MESSAGE_CLASS_0    -- \n Class 0 \n 
       - 0x01 -- MESSAGE_CLASS_1    -- \n Class 1 \n 
       - 0x02 -- MESSAGE_CLASS_2    -- \n Class 2 \n 
       - 0x03 -- MESSAGE_CLASS_3    -- \n Class 3 \n 
       - 0x04 -- MESSAGE_CLASS_NONE -- Class None \n 
       - 0x05 -- MESSAGE_CLASS_CDMA -- Class CDMA
  */

  wms_storage_type_enum_v01 route_memory;
  /**<   Memory storage. Values: \n 
       - 0x00 -- STORAGE_TYPE_UIM \n 
       - 0x01 -- STORAGE_TYPE_NV \n 
       -   -1 -- STORAGE_TYPE_NONE
  */

  wms_receipt_action_enum_v01 route_value;
  /**<   Route value. Values: \n 
       - 0x00 -- DISCARD          -- Incoming messages for this route are
                                     discarded by the WMS service, and no
                                     notification is sent to clients \n 
       - 0x01 -- STORE_AND_NOTIFY -- Incoming messages for this route are
                                     stored to the specified device 
                                     memory, and new message notifications
                                     are sent to registered clients \n 
       - 0x02 -- TRANSFER_ONLY    -- Incoming messages for this route are
                                     transferred to the client, and the
                                     client is expected to send ACK to
                                     the network \n 
       - 0x03 -- TRANSFER_AND_ACK -- Incoming messages for this route are
                                     transferred to the client, and ACK is
                                     sent to the network \n 
       -   -1 -- UNKNOWN          -- Incoming messages for this route are
                                     handled in a way that is unknown or
                                     unsupported by QMI_WMS
  */
}wms_get_route_list_tuple_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Queries the currently configured action performed upon WMS
             message receipt for the specified message routes. It also
             queries the action performed upon WMS receipt of status reports. */
typedef struct {

  /* Mandatory */
  /*  Route List */
  uint32_t route_list_len;  /**< Must be set to # of elements in route_list */
  wms_get_route_list_tuple_type_v01 route_list[WMS_ROUTE_TUPLE_MAX_V01];

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Transfer Status Report** */
  uint8_t transfer_ind_valid;  /**< Must be set to true if transfer_ind is being passed */
  wms_transfer_ind_enum_v01 transfer_ind;
  /**<   Values: \n
         - 0x00 -- TRANSFER_IND_SIM    -- Status reports are stored on the
                                          SIM if a matching MO record is found
                                          on the SIM; otherwise, status reports
                                          are transferred to the client \n 
         - 0x01 -- TRANSFER_IND_CLIENT -- Status reports are transferred
                                          to the client
    */
}wms_get_routes_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_smsc_address_req_msg_v01;

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  char smsc_address_type[WMS_ADDRESS_TYPE_MAX_V01];
  /**<   Type of SMSC address given in ASCII digits
       (must be three digits long, with leading zeros used as
       placeholders)
  */

  uint32_t smsc_address_digits_len;  /**< Must be set to # of elements in smsc_address_digits */
  char smsc_address_digits[WMS_ADDRESS_DIGIT_MAX_V01];
  /**<   Address of the SMSC given in ASCII digits; 
       can be prefixed with + (maximum 20 digits, not
       including the +)
  */
}wms_smsc_address_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Queries the currently configured SMSC address. */
typedef struct {

  /* Mandatory */
  /*  SMSC Address */
  wms_smsc_address_type_v01 smsc_address;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_get_smsc_address_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sets the SMSC address used when storing or saving SMS messages. */
typedef struct {

  /* Mandatory */
  /*  SMSC Address */
  char smsc_address_digits[WMS_ADDRESS_DIGIT_MAX_V01 + 1];
  /**<   NULL-terminated string containing the address of the SMSC, 
       given in ASCII digits; can be prefixed with + 
       (maximum 20 digits, not including the +)
  */

  /* Optional */
  /* SMSC Address Type */
  uint8_t smsc_address_type_valid;  /**< Must be set to true if smsc_address_type is being passed */
  char smsc_address_type[WMS_ADDRESS_TYPE_MAX_V01 + 1];
  /**<   NULL-terminated string containing the type of SMSC address, 
       given in ASCII digits (maximum three digits)
  */

  /* Optional */
  /* SMSC Address Index */
  uint8_t index_valid;  /**< Must be set to true if index is being passed */
  uint8_t index;
  /**<   Indicates the record index where the SMSC address needs to 
       be written
  */
}wms_set_smsc_address_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sets the SMSC address used when storing or saving SMS messages. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_set_smsc_address_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Queries the maximum number of messages that can be stored per
             memory storage, as well as the number of slots currently
             available. */
typedef struct {

  /* Mandatory */
  /*  Memory Store */
  wms_storage_type_enum_v01 storage_type;
  /**<   Memory storage. Values: \n 
       - 0x00 -- STORAGE_TYPE_UIM \n 
       - 0x01 -- STORAGE_TYPE_NV
  */

  /* Optional */
  /*  Message Mode */
  uint8_t message_mode_valid;  /**< Must be set to true if message_mode is being passed */
  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Values: \n 
         - 0x00 -- MESSAGE_MODE_CDMA \n
         - 0x01 -- MESSAGE_MODE_GW
    */
}wms_get_store_max_size_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Queries the maximum number of messages that can be stored per
             memory storage, as well as the number of slots currently
             available. */
typedef struct {

  /* Mandatory */
  /*  Memory Store Size */
  uint32_t mem_store_max_size;
  /**<   Maximum number of messages for this memory storage*/

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Memory Available */
  uint8_t free_slots_valid;  /**< Must be set to true if free_slots is being passed */
  uint32_t free_slots;
  /**<   Number of slots currently available for this memory storage*/
}wms_get_store_max_size_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t transaction_id;
  /**<   Transaction ID of the message for which ACK is to be sent.
  */

  wms_message_protocol_enum_v01 message_protocol;
  /**<   WMS message protocol. Values: \n 
         - 0x00 -- MESSAGE_PROTOCOL_ CDMA \n 
         - 0x01 -- MESSAGE_PROTOCOL_ WCDMA
    */

  uint8_t success;
  /**<   Indicates whether the MT message processed successfully. Values: \n
       - 0x00 -- Failure \n 
       - 0x01 -- Success
  */
}wms_ack_information_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_ERROR_CLASS_3GPP2_FAILURE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_ERROR_CLASS_3GPP2_FAILURE_TEMPORARY_V01 = 0x02, 
  WMS_ERROR_CLASS_3GPP2_FAILURE_PERMANENT_V01 = 0x03, 
  WMS_ERROR_CLASS_3GPP2_FAILURE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_error_class_3gpp2_failure_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_TL_CAUSE_CODE_SEND_ACK_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_TL_CAUSE_CODE_ADDR_VACANT_ACK_V01 = 0x00, 
  WMS_TL_CAUSE_CODE_ADDR_TRANSLATION_FAILURE_ACK_V01 = 0x01, 
  WMS_TL_CAUSE_CODE_NETWORK_RESOURCE_SHORTAGE_ACK_V01 = 0x02, 
  WMS_TL_CAUSE_CODE_NETWORK_FAILURE_ACK_V01 = 0x03, 
  WMS_TL_CAUSE_CODE_INVALID_TELESERVICE_ID_ACK_V01 = 0x04, 
  WMS_TL_CAUSE_CODE_NETWORK_OTHER_ACK_V01 = 0x05, 
  WMS_TL_CAUSE_CODE_NO_PAGE_RESPONSE_ACK_V01 = 0x20, 
  WMS_TL_CAUSE_CODE_DEST_BUSY_ACK_V01 = 0x21, 
  WMS_TL_CAUSE_CODE_NO_ACK_ACK_V01 = 0x22, 
  WMS_TL_CAUSE_CODE_DEST_RESOURCE_SHORTAGE_ACK_V01 = 0x23, 
  WMS_TL_CAUSE_CODE_SMS_DELIVERY_POSTPONED_ACK_V01 = 0x24, 
  WMS_TL_CAUSE_CODE_DEST_OUT_OF_SERV_ACK_V01 = 0x25, 
  WMS_TL_CAUSE_CODE_DEST_NOT_AT_ADDR_ACK_V01 = 0x26, 
  WMS_TL_CAUSE_CODE_DEST_OTHER_ACK_V01 = 0x27, 
  WMS_TL_CAUSE_CODE_RADIO_IF_RESOURCE_SHORTAGE_ACK_V01 = 0x40, 
  WMS_TL_CAUSE_CODE_RADIO_IF_INCOMPATABILITY_ACK_V01 = 0x41, 
  WMS_TL_CAUSE_CODE_RADIO_IF_OTHER_ACK_V01 = 0x42, 
  WMS_TL_CAUSE_CODE_ENCODING_ACK_V01 = 0x60, 
  WMS_TL_CAUSE_CODE_SMS_ORIG_DENIED_ACK_V01 = 0x61, 
  WMS_TL_CAUSE_CODE_SMS_TERM_DENIED_ACK_V01 = 0x62, 
  WMS_TL_CAUSE_CODE_SUPP_SERV_NOT_SUPP_ACK_V01 = 0x63, 
  WMS_TL_CAUSE_CODE_SMS_NOT_SUPP_ACK_V01 = 0x64, 
  WMS_TL_CAUSE_CODE_MISSING_EXPECTED_PARAM_ACK_V01 = 0x65, 
  WMS_TL_CAUSE_CODE_MISSING_MAND_PARAM_ACK_V01 = 0x66, 
  WMS_TL_CAUSE_CODE_UNRECOGNIZED_PARAM_VAL_ACK_V01 = 0x67, 
  WMS_TL_CAUSE_CODE_UNEXPECTED_PARAM_VAL_ACK_V01 = 0x68, 
  WMS_TL_CAUSE_CODE_USER_DATA_SIZE_ERR_ACK_V01 = 0x69, 
  WMS_TL_CAUSE_CODE_GENERAL_OTHER_ACK_V01 = 0x6A, 
  WMS_TL_CAUSE_CODE_SEND_ACK_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_tl_cause_code_send_ack_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_error_class_3gpp2_failure_enum_v01 error_class;
  /**<   Error class. Values: \n 
       - 0x02 -- ERROR_CLASS_3GPP2_ FAILURE_TEMPORARY \n 
       - 0x03 -- ERROR_CLASS_3GPP2_ FAILURE_PERMANENT
  */

  wms_tl_cause_code_send_ack_enum_v01 tl_status;
  /**<   WMS transport layer status conveying the CDMA
       cause code per \hyperref[S1]{[S1]} Section 3.4.3.6; see Table
       @latexonly \ref{tbl:WMSCauseCodes} @endlatexonly for more 
       information.
  */
}wms_3gpp2_failure_information_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_RP_CAUSE_SEND_ACK_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_RP_CAUSE_UNASSIGNED_NUMBER_ACK_V01 = 0x01, 
  WMS_RP_CAUSE_OPERATOR_DETERMINED_BARRING_ACK_V01 = 0x08, 
  WMS_RP_CAUSE_CALL_BARRED_ACK_V01 = 0x0A, 
  WMS_RP_CAUSE_RESERVED_ACK_V01 = 0x0B, 
  WMS_RP_CAUSE_SMS_TRANSFER_REJECTED_ACK_V01 = 0x15, 
  WMS_RP_CAUSE_MEMORY_CAP_EXCEEDED_ACK_V01 = 0x16, 
  WMS_RP_CAUSE_DESTINATION_OUT_OF_ORDER_ACK_V01 = 0x1B, 
  WMS_RP_CAUSE_UNIDENTIFIED_SUBSCRIBER_ACK_V01 = 0x1C, 
  WMS_RP_CAUSE_FACILITY_REJECTED_ACK_V01 = 0x1D, 
  WMS_RP_CAUSE_UNKNOWN_SUBSCRIBER_ACK_V01 = 0x1E, 
  WMS_RP_CAUSE_NETWORK_OUT_OF_ORDER_ACK_V01 = 0x26, 
  WMS_RP_CAUSE_TEMPORARY_FAILURE_ACK_V01 = 0x29, 
  WMS_RP_CAUSE_CONGESTION_ACK_V01 = 0x2A, 
  WMS_RP_CAUSE_RESOURCES_UNAVAILABLE_ACK_V01 = 0x2F, 
  WMS_RP_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED_ACK_V01 = 0x32, 
  WMS_RP_CAUSE_REQUESTED_FACILITY_NOT_IMPLEMENTED_ACK_V01 = 0x45, 
  WMS_RP_CAUSE_INVALID_SMS_TRANSFER_REFERENCE_VALUE_ACK_V01 = 0x51, 
  WMS_RP_CAUSE_SEMANTICALLY_INCORRECT_MESSAGE_ACK_V01 = 0x5F, 
  WMS_RP_CAUSE_INVALID_MANDATORY_INFO_ACK_V01 = 0x60, 
  WMS_RP_CAUSE_MESSAGE_TYPE_NOT_IMPLEMENTED_ACK_V01 = 0x61, 
  WMS_RP_CAUSE_MESSAGE_NOT_COMPATABLE_WITH_SMS_ACK_V01 = 0x62, 
  WMS_RP_CAUSE_INFO_ELEMENT_NOT_IMPLEMENTED_ACK_V01 = 0x63, 
  WMS_RP_CAUSE_PROTOCOL_ERROR_ACK_V01 = 0x6F, 
  WMS_RP_CAUSE_INTERWORKING_ACK_V01 = 0x7F, 
  WMS_RP_CAUSE_SEND_ACK_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_rp_cause_send_ack_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_rp_cause_send_ack_enum_v01 rp_cause;
  /**<   GW RP cause per \hyperref[S5]{[S5]}
       Section 8.2.5.4; see Table @latexonly \ref{tbl:GWRPCauseCodes}
       @endlatexonly for more information.
  */

  wms_tp_cause_enum_v01 tp_cause;
  /**<   GW TP cause per \hyperref[S2]{[S2]} Section 9.2.3.22; see Table
         @latexonly \ref{tbl:GWTPCauseCodes} @endlatexonly for more information.
    */
}wms_3gpp_failure_information_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sends an ACK to the network for transfer-only routes. */
typedef struct {

  /* Mandatory */
  /*  ACK Information */
  wms_ack_information_type_v01 ack_information;

  /* Optional */
  /*  3GPP2 Failure Information* */
  uint8_t wms_3gpp2_failure_information_valid;  /**< Must be set to true if wms_3gpp2_failure_information is being passed */
  wms_3gpp2_failure_information_type_v01 wms_3gpp2_failure_information;

  /* Optional */
  /*  3GPP Failure Information** */
  uint8_t wms_3gpp_failure_information_valid;  /**< Must be set to true if wms_3gpp_failure_information is being passed */
  wms_3gpp_failure_information_type_v01 wms_3gpp_failure_information;

  /* Optional */
  /*  SMS on IMS */
  uint8_t sms_on_ims_valid;  /**< Must be set to true if sms_on_ims is being passed */
  uint8_t sms_on_ims;
  /**<   Indicates whether ACK is to be sent on IMS. Values: \n 
       - 0x00 -- ACK is not to be sent on IMS \n 
       - 0x01 -- ACK is to be sent on IMS \n 
       - 0x02 to 0xFF -- Reserved \n
         Note: In minor version 9, the implementation was changed in such a way
               that inclusion of this TLV may affect the SMS routing differently.
  */
}wms_send_ack_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_ACK_FAILURE_CAUSE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_ACK_FAILURE_NO_NETWORK_RESPONSE_V01 = 0x00, 
  WMS_ACK_FAILURE_NETWORK_RELEASED_LINK_V01 = 0x01, 
  WMS_ACK_FAILURE_ACK_NOT_SENT_V01 = 0x02, 
  WMS_ACK_FAILURE_CAUSE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_ack_failure_cause_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sends an ACK to the network for transfer-only routes. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  ACK Failure Cause */
  uint8_t failure_cause_valid;  /**< Must be set to true if failure_cause is being passed */
  wms_ack_failure_cause_enum_v01 failure_cause;
  /**<   ACK failure cause. Values: \n 
     - 0x00 -- ACK_FAILURE_NO_ NETWORK_ RESPONSE \n 
     - 0x01 -- ACK_FAILURE_NETWORK_ RELEASED_LINK \n 
     - 0x02 -- ACK_FAILURE_ACK_ NOT_SENT*/
}wms_send_ack_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Configures the retry period.
             \label{idl:setRetryPeriod} */
typedef struct {

  /* Mandatory */
  /*  Retry Period */
  uint32_t retry_period;
  /**<   Retry period in seconds up to which the WMS retries
       to send a message before giving up; if
       retry_period is 0 sec, retry is not attempted
  */
}wms_set_retry_period_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Configures the retry period.
             \label{idl:setRetryPeriod} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */
}wms_set_retry_period_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Configures the retry interval.
             \label{idl:setRetryInterval} */
typedef struct {

  /* Mandatory */
  /*  Retry Interval */
  uint32_t retry_interval;
  /**<   Retry interval in seconds specifying the interval
       between WMS retry attempts
  */
}wms_set_retry_interval_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Configures the retry interval.
             \label{idl:setRetryInterval} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_set_retry_interval_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Configures the CDMA dedicated channel autodisconnect timer.
             \label{idl:setDcDisconnectTimer} */
typedef struct {

  /* Mandatory */
  /*  DC Auto Disconnect Timer */
  uint32_t dc_auto_disconn_timer;
  /**<   Timeout period in seconds; a value of 0
       means that the autodisconnect is disabled
  */
}wms_set_dc_disconnect_timer_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Configures the CDMA dedicated channel autodisconnect timer.
             \label{idl:setDcDisconnectTimer} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_set_dc_disconnect_timer_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Indicates whether the client has storage available
             for new SMS messages.
             @latexonly \\ \textbf{Note:} The client must set itself as the 
             primary client of QMI\_WMS in order for this request to be 
             successful. This can be done using the 
             QMI\_WMS\_SET\_PRIMARY\_CLIENT request. @endlatexonly
             \label{idl:setMemoryStatus} */
typedef struct {

  /* Mandatory */
  /*  Memory Status Information */
  uint8_t memory_available;
  /**<   Memory availability. Values: \n 
       - 0x00 -- Memory is not available \n 
       - 0x01 -- Memory is available
  */
}wms_set_memory_status_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Indicates whether the client has storage available
             for new SMS messages.
             @latexonly \\ \textbf{Note:} The client must set itself as the 
             primary client of QMI\_WMS in order for this request to be 
             successful. This can be done using the 
             QMI\_WMS\_SET\_PRIMARY\_CLIENT request. @endlatexonly
             \label{idl:setMemoryStatus} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_set_memory_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Values: \n 
         - 0x00 -- MESSAGE_MODE_CDMA -- CDMA \n 
         - 0x01 -- MESSAGE_MODE_GW   -- GW
    */

  uint8_t bc_activate;
  /**<   Broadcast activation. Values: \n 
       - 0x00 -- Disable broadcast \n 
       - 0x01 -- Activate broadcast
  */
}wms_broadcast_activation_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Enables or disables the reception of broadcast SMS messages. */
typedef struct {

  /* Mandatory */
  /*  Broadcast Activation Information */
  wms_broadcast_activation_info_type_v01 broadcast_activation_info;

  /* Optional */
  /*  Broadcast Filtering Information */
  uint8_t activate_all_valid;  /**< Must be set to true if activate_all is being passed */
  uint8_t activate_all;
  /**<   Indicates whether to accept 3GPP2 broadcast SMS messages for all service 
       categories or to accept 3GPP cell broadcast SMS messages without 
       additional language preference filtering. Values: \n
       - 0x00 -- Filter 3GPP2 broadcast messages based on service categories 
                 and 3GPP cell broadcast messages based on language preferences \n
       - 0x01 -- Ignore service categories or language preferences 
  */
}wms_set_broadcast_activation_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Enables or disables the reception of broadcast SMS messages. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */
}wms_set_broadcast_activation_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t from_service_id;
  /**<   Starting point of the range of CBM message identifiers; message IDs are
       defined in \hyperref[S6]{[S6]} Section 9.4.1.2.2 for GSM and
       \hyperref[S6]{[S6]} Section 9.4.4.2.2 for UMTS.
  */

  uint16_t to_service_id;
  /**<   Ending point of the range of CBM message identifiers; message IDs are
       defined in \hyperref[S6]{[S6]} Section 9.4.1.2.2 for GSM and
       \hyperref[S6]{[S6]} Section 9.4.4.2.2 for UMTS.
  */

  uint8_t selected;
  /**<   Range of CBM message identifiers indicated by from_service_id and
       to_service_id. Values: \n 
       - 0x00 -- Not selected \n 
       - 0x01 -- Selected
  */
}wms_3gpp_broadcast_config_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_LANGUAGE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_LANGUAGE_UNKNOWN_V01 = 0x00, 
  WMS_LANGUAGE_ENGLISH_V01 = 0x01, 
  WMS_LANGUAGE_FRENCH_V01 = 0x02, 
  WMS_LANGUAGE_SPANISH_V01 = 0x03, 
  WMS_LANGUAGE_JAPANESE_V01 = 0x04, 
  WMS_LANGUAGE_KOREAN_V01 = 0x05, 
  WMS_LANGUAGE_CHINESE_V01 = 0x06, 
  WMS_LANGUAGE_HEBREW_V01 = 0x07, 
  WMS_LANGUAGE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_language_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_SERVICE_CATEGORY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_SERVICE_CAT_UNKNOWN_V01 = 0x00, 
  WMS_SERVICE_CAT_EMERGENCY_BROADCAST_V01 = 0x01, 
  WMS_SERVICE_CAT_ADMINISTRATIVE_V01 = 0x02, 
  WMS_SERVICE_CAT_MAINTENANCE_V01 = 0x03, 
  WMS_SERVICE_CAT_GENERAL_NEWS_LOCAL_V01 = 0x04, 
  WMS_SERVICE_CAT_GENERAL_NEWS_REGIONAL_V01 = 0x05, 
  WMS_SERVICE_CAT_GENERAL_NEWS_NATIONAL_V01 = 0x06, 
  WMS_SERVICE_CAT_GENERAL_NEWS_INTERNATIONAL_V01 = 0x07, 
  WMS_SERVICE_CAT_BUSINESS_NEWS_LOCAL_V01 = 0x08, 
  WMS_SERVICE_CAT_BUSINESS_NEWS_REGIONAL_V01 = 0x09, 
  WMS_SERVICE_CAT_BUSINESS_NEWS_NATIONAL_V01 = 0x0A, 
  WMS_SERVICE_CAT_BUSINESS_NEWS_INTERNATIONAL_V01 = 0x0B, 
  WMS_SERVICE_CAT_SPORTS_NEWS_LOCAL_V01 = 0x0C, 
  WMS_SERVICE_CAT_SPORTS_NEWS_REGIONAL_V01 = 0x0D, 
  WMS_SERVICE_CAT_SPORTS_NEWS_NATIONAL_V01 = 0x0E, 
  WMS_SERVICE_CAT_SPORTS_NEWS_INTERNATIONAL_V01 = 0x0F, 
  WMS_SERVICE_CAT_ENTERTAINMENT_NEWS_LOCAL_V01 = 0x10, 
  WMS_SERVICE_CAT_ENTERTAINMENT_NEWS_REGIONAL_V01 = 0x11, 
  WMS_SERVICE_CAT_ENTERTAINMENT_NEWS_NATIONAL_V01 = 0x12, 
  WMS_SERVICE_CAT_ENTERTAINMENT_NEWS_INTERNATIONAL_V01 = 0x13, 
  WMS_SERVICE_CAT_LOCAL_WEATHER_V01 = 0x14, 
  WMS_SERVICE_CAT_TRAFFIC_REPORTS_V01 = 0x15, 
  WMS_SERVICE_CAT_LOCAL_FLIGHT_SCHEDULES_V01 = 0x16, 
  WMS_SERVICE_CAT_RESTAURANTS_V01 = 0x17, 
  WMS_SERVICE_CAT_LODGINGS_V01 = 0x18, 
  WMS_SERVICE_CAT_RETAIL_DIRECTORY_V01 = 0x19, 
  WMS_SERVICE_CAT_ADVERTISEMENTS_V01 = 0x1A, 
  WMS_SERVICE_CAT_STOCK_QUOTES_V01 = 0x1B, 
  WMS_SERVICE_CAT_EMPLOYMENT_OPPORTUNITIES_V01 = 0x1C, 
  WMS_SERVICE_CAT_MEDICAL_V01 = 0x1D, 
  WMS_SERVICE_CAT_TECHNOLOGY_NEWS_V01 = 0x1E, 
  WMS_SERVICE_CAT_MULTI_CAT_V01 = 0x1F, 
  WMS_SERVICE_CAT_CATPT_V01 = 0x20, 
  WMS_SERVICE_CAT_PRESIDENTIAL_LEVEL_ALERT_V01 = 0x1000, 
  WMS_SERVICE_CAT_EXTREME_THREAT_TO_LIFE_AND_PROPERTY_V01 = 0x1001, 
  WMS_SERVICE_CAT_SEVERE_THREAT_TO_LIFE_AND_PROPERTY_V01 = 0x1002, 
  WMS_SERVICE_CAT_AMBER_CHILD_ABDUCTION_EMERGENCY_V01 = 0x1003, 
  WMS_SERVICE_CAT_CMAS_TEST_MESSAGE_V01 = 0x1004, 
  WMS_SERVICE_CATEGORY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_service_category_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_service_category_enum_v01 service_category;
  /**<   Service category assignments, as defined in \hyperref[S7]{[S7]}
       Section 9.3; see Table @latexonly \ref{tbl:ServiceCategories}
       @endlatexonly for more information.
  */

  wms_language_enum_v01 language;
  /**<   Language indicator value assignments, as defined in
       \hyperref[S7]{[S7]} Section 9.2.
       Values: \n 
        - 0x00 -- LANGUAGE_UNKNOWN  -- Unknown or unspecified \n 
        - 0x01 -- LANGUAGE_ENGLISH  -- English \n 
        - 0x02 -- LANGUAGE_FRENCH   -- French \n 
        - 0x03 -- LANGUAGE_SPANISH  -- Spanish \n 
        - 0x04 -- LANGUAGE_JAPANESE -- Japanese \n 
        - 0x05 -- LANGUAGE_KOREAN   -- Korean \n 
        - 0x06 -- LANGUAGE_CHINESE  -- Chinese \n 
        - 0x07 -- LANGUAGE_HEBREW   -- Hebrew
  */

  uint8_t selected;
  /**<   Specified service_category and language. Values: \n 
       - 0x00 -- Not selected \n 
       - 0x01 -- Selected
  */
}wms_3gpp2_broadcast_config_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sets the broadcast SMS configuration. */
typedef struct {

  /* Mandatory */
  /*  Broadcast Configuration Information */
  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Values: \n 
         - 0x00 -- MESSAGE_MODE_CDMA -- CDMA \n 
         - 0x01 -- MESSAGE_MODE_GW   -- GW
    */

  /* Optional */
  /*  3GPP Broadcast Configuration Information** */
  uint8_t wms_3gpp_broadcast_config_info_valid;  /**< Must be set to true if wms_3gpp_broadcast_config_info is being passed */
  uint32_t wms_3gpp_broadcast_config_info_len;  /**< Must be set to # of elements in wms_3gpp_broadcast_config_info */
  wms_3gpp_broadcast_config_info_type_v01 wms_3gpp_broadcast_config_info[WMS_3GPP_BROADCAST_CONFIG_MAX_V01];

  /* Optional */
  /*  3GPP2 Broadcast Configuration Information* */
  uint8_t wms_3gpp2_broadcast_config_info_valid;  /**< Must be set to true if wms_3gpp2_broadcast_config_info is being passed */
  uint32_t wms_3gpp2_broadcast_config_info_len;  /**< Must be set to # of elements in wms_3gpp2_broadcast_config_info */
  wms_3gpp2_broadcast_config_info_type_v01 wms_3gpp2_broadcast_config_info[WMS_3GPP2_BROADCAST_CONFIG_MAX_V01];
}wms_set_broadcast_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sets the broadcast SMS configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_set_broadcast_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Gets the current broadcast SMS configuration. */
typedef struct {

  /* Mandatory */
  /*  Broadcast Configuration Information */
  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Values: \n 
         - 0x00 -- MESSAGE_MODE_CDMA -- CDMA \n 
         - 0x01 -- MESSAGE_MODE_GW   -- GW
    */
}wms_get_broadcast_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t activated_ind;
  /**<   Broadcast SMS. Values: \n 
       - 0x00 -- Deactivated \n 
       - 0x01 -- Activated
  */

  uint32_t wms_3gpp_broadcast_config_info_len;  /**< Must be set to # of elements in wms_3gpp_broadcast_config_info */
  wms_3gpp_broadcast_config_info_type_v01 wms_3gpp_broadcast_config_info[WMS_3GPP_BROADCAST_CONFIG_MAX_V01];
}wms_3gpp_broadcast_info_config2_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t activated_ind;
  /**<   Broadcast SMS. Values: \n 
       - 0x00 -- Deactivated \n 
       - 0x01 -- Activated
  */

  uint32_t wms_3gpp2_broadcast_config_info_len;  /**< Must be set to # of elements in wms_3gpp2_broadcast_config_info */
  wms_3gpp2_broadcast_config_info_type_v01 wms_3gpp2_broadcast_config_info[WMS_3GPP2_BROADCAST_CONFIG_MAX_V01];
}wms_3gpp2_broadcast_info_config2_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Gets the current broadcast SMS configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  3GPP Broadcast Configuration Information** */
  uint8_t wms_3gpp_broadcast_info_valid;  /**< Must be set to true if wms_3gpp_broadcast_info is being passed */
  wms_3gpp_broadcast_info_config2_type_v01 wms_3gpp_broadcast_info;

  /* Optional */
  /*  3GPP2 Broadcast Configuration Information* */
  uint8_t wms_3gpp2_broadcast_info_valid;  /**< Must be set to true if wms_3gpp2_broadcast_info is being passed */
  wms_3gpp2_broadcast_info_config2_type_v01 wms_3gpp2_broadcast_info;
}wms_get_broadcast_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_storage_type_enum_v01 storage_type;
  /**<   Memory storage. Values: \n 
       - 0x00 -- STORAGE_TYPE_UIM \n 
       - 0x01 -- STORAGE_TYPE_NV
  */

  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Values: \n 
         - 0x00 -- MESSAGE_MODE_CDMA -- CDMA \n 
         - 0x01 -- MESSAGE_MODE_GW   -- GW
    */
}wms_memory_full_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Indication Message; Indicates that the SMS storage is full. */
typedef struct {

  /* Mandatory */
  /*  Memory Full Information */
  wms_memory_full_info_type_v01 memory_full_info;
}wms_memory_full_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_domain_pref_req_msg_v01;

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_DOMAIN_PREF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_DOMAIN_PREF_CS_V01 = 0x00, 
  WMS_DOMAIN_PREF_PS_V01 = 0x01, 
  WMS_DOMAIN_PREF_CS_ONLY_V01 = 0x02, 
  WMS_DOMAIN_PREF_PS_ONLY_V01 = 0x03, 
  WMS_DOMAIN_PREF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_domain_pref_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Queries the GW domain preference. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Domain Pref */
  wms_domain_pref_enum_v01 domain_pref;
  /**<   GW domain preference. Values: \n 
       - 0x00 -- DOMAIN_PREF_CS      -- CS preferred \n 
       - 0x01 -- DOMAIN_PREF_PS      -- PS preferred \n 
       - 0x02 -- DOMAIN_PREF_CS_ONLY -- CS only \n 
       - 0x03 -- DOMAIN_PREF_PS_ONLY -- PS only
  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_get_domain_pref_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sets the GW domain preference. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Domain Pref */
  wms_domain_pref_enum_v01 domain_pref;
  /**<   GW domain preference. Values: \n 
       - 0x00 -- DOMAIN_PREF_CS      -- CS preferred \n 
       - 0x01 -- DOMAIN_PREF_PS      -- PS preferred \n 
       - 0x02 -- DOMAIN_PREF_CS_ONLY -- CS only \n 
       - 0x03 -- DOMAIN_PREF_PS_ONLY -- PS only
  */
}wms_set_domain_pref_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sets the GW domain preference. (Deprecated) */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
}wms_set_domain_pref_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_storage_type_enum_v01 storage_type;
  /**<   Memory storage. Values: \n 
       - 0x00 -- STORAGE_TYPE_UIM \n 
       - 0x01 -- STORAGE_TYPE_NV
  */

  uint32_t storage_index;
  /**<   Memory Index*/

  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Value: \n 
         - 0x00 -- MESSAGE_MODE_CDMA -- CDMA \n 
         - 0x01 -- MESSAGE_MODE_GW   -- GW
    */
}wms_message_memory_storage_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sends a message from a memory store. */
typedef struct {

  /* Mandatory */
  /*  Message Memory Storage Information */
  wms_message_memory_storage_info_type_v01 message_memory_storage_info;

  /* Optional */
  /*  SMS on IMS */
  uint8_t sms_on_ims_valid;  /**< Must be set to true if sms_on_ims is being passed */
  uint8_t sms_on_ims;
  /**<   Indicates whether the message is to be sent on IMS. Values: \n 
       - 0x00 -- Message is not to be sent on IMS \n 
       - 0x01 -- Message is to be sent on IMS \n 
       - 0x02 to 0xFF -- Reserved \n
         Note: In minor version 9, the implementation was changed in such a way
               that inclusion of this TLV may affect the SMS routing differently.
  */
}wms_send_from_mem_store_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sends a message from a memory store. */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Message ID */
  uint8_t message_id_valid;  /**< Must be set to true if message_id is being passed */
  uint16_t message_id;
  /**<   WMS message ID*/

  /* Optional */
  /*  Cause Code* */
  uint8_t cause_code_valid;  /**< Must be set to true if cause_code is being passed */
  wms_tl_cause_code_enum_v01 cause_code;
  /**<   WMS cause code per \hyperref[S4]{[S4]} Section 6.5.2.125; see Table
       @latexonly \ref{tbl:WMSCauseCodes} @endlatexonly for more information.
  */

  /* Optional */
  /*  Error Class* */
  uint8_t error_class_valid;  /**< Must be set to true if error_class is being passed */
  wms_error_class_send_enum_v01 error_class;
  /**<   Error class. Values: \n 
       - 0x00 -- ERROR_CLASS_TEMPORARY \n 
       - 0x01 -- ERROR_CLASS_PERMANENT
  */

  /* Optional */
  /*  GW Cause Info** */
  uint8_t gw_cause_info_valid;  /**< Must be set to true if gw_cause_info is being passed */
  wms_gw_cause_info_type_v01 gw_cause_info;

  /* Optional */
  /*  Message Delivery Failure Type */
  uint8_t message_delivery_failure_type_valid;  /**< Must be set to true if message_delivery_failure_type is being passed */
  wms_message_delivery_failure_type_enum_v01 message_delivery_failure_type;
  /**<   Message delivery failure type. Values: \n 
     - 0x00 -- WMS_MESSAGE_ DELIVERY_FAILURE_TEMPORARY \n 
     - 0x01 -- WMS_MESSAGE_ DELIVERY_ FAILURE_PERMANENT*/
}wms_send_from_mem_store_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_message_waiting_req_msg_v01;

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMI_WMS_MESSAGE_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_MWI_MESSAGE_TYPE_VOICEMAIL_V01 = 0x00, 
  WMS_MWI_MESSAGE_TYPE_FAX_V01 = 0x01, 
  WMS_MWI_MESSAGE_TYPE_EMAIL_V01 = 0x02, 
  WMS_MWI_MESSAGE_TYPE_OTHER_V01 = 0x03, 
  WMS_MWI_MESSAGE_TYPE_VIDEOMAIL_V01 = 0x04, 
  WMI_WMS_MESSAGE_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wmi_wms_message_type_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wmi_wms_message_type_enum_v01 message_type;
  /**<   Message type. Values: \n 
         - 0x00 -- MWI_MESSAGE_TYPE_ VOICEMAIL -- Voicemail \n 
         - 0x01 -- MWI_MESSAGE_TYPE_FAX        -- Fax \n 
         - 0x02 -- MWI_MESSAGE_TYPE_ EMAIL     -- Email \n 
         - 0x03 -- MWI_MESSAGE_TYPE_ OTHER     -- Other \n 
         - 0x04 -- MWI_MESSAGE_TYPE_ VIDEOMAIL -- Videomail
    */

  uint8_t active_ind;
  /**<   Indicates whether the indication is active. Values: \n 
         - 0x00 -- Inactive \n 
         - 0x01 -- Active
    */

  uint8_t message_count;
  /**<   Number of messages. */
}wms_message_waiting_information_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Gets the message waiting information. */
typedef struct {

  /* Mandatory */
  /*  Message Waiting Information */
  uint32_t message_waiting_info_len;  /**< Must be set to # of elements in message_waiting_info */
  wms_message_waiting_information_type_v01 message_waiting_info[WMS_MESSAGE_TUPLE_MAX_V01];

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}wms_get_message_waiting_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the message waiting information. */
typedef struct {

  /* Mandatory */
  /*  Message Waiting Information */
  uint32_t message_waiting_info_len;  /**< Must be set to # of elements in message_waiting_info */
  wms_message_waiting_information_type_v01 message_waiting_info[WMS_MESSAGE_TUPLE_MAX_V01];
}wms_message_waiting_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Allows the client to set or unset itself as the primary client of 
             QMI_WMS.
             \label{idl:setPrimaryClient} */
typedef struct {

  /* Mandatory */
  /*  Primary Client Information */
  uint8_t primary_client;
  /**<   Indicates whether the client is set as the primary client. Values: \n
         - 0x00 -- FALSE \n 
         - 0x01 -- TRUE
    */
}wms_set_primary_client_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Allows the client to set or unset itself as the primary client of 
             QMI_WMS.
             \label{idl:setPrimaryClient} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}wms_set_primary_client_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the SMSC address used by QMI_WMS. */
typedef struct {

  /* Mandatory */
  /*  SMSC Address */
  wms_smsc_address_type_v01 smsc_address;
}wms_smsc_address_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sets the registration state for different
             QMI_WMS indications for the requesting control point. */
typedef struct {

  /* Optional */
  /*  Transport Layer Information Events */
  uint8_t reg_transport_layer_info_events_valid;  /**< Must be set to true if reg_transport_layer_info_events is being passed */
  uint8_t reg_transport_layer_info_events;
  /**<   Values: \n 
       - 0x00 -- Disable \n 
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Transport NW Reg Information Events */
  uint8_t reg_transport_nw_reg_info_events_valid;  /**< Must be set to true if reg_transport_nw_reg_info_events is being passed */
  uint8_t reg_transport_nw_reg_info_events;
  /**<   Values: \n 
       - 0x00 -- Disable \n 
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Call Status Information Events */
  uint8_t reg_call_status_info_events_valid;  /**< Must be set to true if reg_call_status_info_events is being passed */
  uint8_t reg_call_status_info_events;
  /**<   Values: \n 
       - 0x00 -- Disable \n 
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Service Ready Events */
  uint8_t reg_service_ready_events_valid;  /**< Must be set to true if reg_service_ready_events is being passed */
  uint8_t reg_service_ready_events;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Broadcast Config Events */
  uint8_t reg_broadcast_config_events_valid;  /**< Must be set to true if reg_broadcast_config_events is being passed */
  uint8_t reg_broadcast_config_events;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */
}wms_indication_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sets the registration state for different
             QMI_WMS indications for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}wms_indication_register_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_transport_layer_req_msg_v01;

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_TRANSPORT_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_TRANSPORT_TYPE_IMS_V01 = 0x00, 
  WMS_TRANSPORT_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_transport_type_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_TRANSPORT_CAPABILITY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_TRANSPORT_CAP_CDMA_V01 = 0x00, 
  WMS_TRANSPORT_CAP_GW_V01 = 0x01, 
  WMS_TRANSPORT_CAPABILITY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_transport_capability_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_aggregates
    @{
  */
typedef struct {

  wms_transport_type_enum_v01 transport_type;
  /**<   Transport type. Values: \n 
         - 0x00 -- IMS
    */

  wms_transport_capability_enum_v01 transport_cap;
  /**<   Transport capability. Values: \n 
         - 0x00 -- CDMA \n 
         - 0x01 -- GW
    */
}wms_transport_layer_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Gets the transport layer information.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Transport Layer Registration Information */
  uint8_t registered_ind_valid;  /**< Must be set to true if registered_ind is being passed */
  uint8_t registered_ind;
  /**<   Indicates whether a transport layer is registered. Values: \n  
       - 0x00 -- Transport layer is not registered \n 
       - 0x01 -- Transport layer is registered
  */

  /* Optional */
  /*  Transport Layer Information */
  uint8_t transport_layer_info_valid;  /**< Must be set to true if transport_layer_info is being passed */
  wms_transport_layer_info_type_v01 transport_layer_info;
}wms_get_transport_layer_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the transport layer information. */
typedef struct {

  /* Mandatory */
  /*  Transport Layer Registration Information */
  uint8_t registered_ind;
  /**<   Indicates whether a transport layer is registered. Values: \n 
         - 0x00 -- Transport layer is not registered \n 
         - 0x01 -- Transport layer is registered
    */

  /* Optional */
  /*  Transport Layer Information */
  uint8_t transport_layer_info_valid;  /**< Must be set to true if transport_layer_info is being passed */
  wms_transport_layer_info_type_v01 transport_layer_info;
}wms_transport_layer_info_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_transport_nw_reg_req_msg_v01;

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_TRANSPORT_NW_REG_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_TRANSPORT_NW_REG_STATUS_NO_SERVICE_V01 = 0x00, 
  WMS_TRANSPORT_NW_REG_STATUS_IN_PROGRESS_V01 = 0x01, 
  WMS_TRANSPORT_NW_REG_STATUS_FAILED_V01 = 0x02, 
  WMS_TRANSPORT_NW_REG_STATUS_LIMITED_SERVICE_V01 = 0x03, 
  WMS_TRANSPORT_NW_REG_STATUS_FULL_SERVICE_V01 = 0x04, 
  WMS_TRANSPORT_NW_REG_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_transport_nw_reg_status_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Gets the transport network registration information.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Transport Network Registration Information */
  uint8_t transport_nw_reg_status_valid;  /**< Must be set to true if transport_nw_reg_status is being passed */
  wms_transport_nw_reg_status_enum_v01 transport_nw_reg_status;
  /**<   Transport layer network registration status. Values: \n  
       - 0x00 -- No service \n 
       - 0x01 -- In process \n 
       - 0x02 -- Failed \n 
       - 0x03 -- Limited service \n 
       - 0x04 -- Full service
  */
}wms_get_transport_nw_reg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the transport network registration information. */
typedef struct {

  /* Mandatory */
  /*  Transport Network Registration Information */
  wms_transport_nw_reg_status_enum_v01 transport_nw_reg_status;
  /**<   Transport layer network registration status. Values: \n  
       - 0x00 -- No service \n 
       - 0x01 -- In process \n 
       - 0x02 -- Failed \n 
       - 0x03 -- Limited service \n 
       - 0x04 -- Full service
  */
}wms_transport_nw_reg_info_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_SUBSCRIPTION_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_PRIMARY_SUBSCRIPTION_V01 = 0x00, 
  WMS_SECONDARY_SUBSCRIPTION_V01 = 0x01, 
  WMS_TERTIARY_SUBSCRIPTION_V01 = 0x02, 
  WMS_SUBSCRIPTION_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_subscription_type_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Binds the current control point to a specific subscription.
             \label{idl:bindSubscription} */
typedef struct {

  /* Mandatory */
  /*  Subscription Type */
  wms_subscription_type_enum_v01 subs_type;
  /**<   Values: \n 
         - 0x00 -- Primary subscription \n 
         - 0x01 -- Secondary subscription \n
         - 0x02 -- Tertiary subscription \n
         - 0x03 to 0xFF -- Reserved
    */
}wms_bind_subscription_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Binds the current control point to a specific subscription.
             \label{idl:bindSubscription} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}wms_bind_subscription_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_indication_register_req_msg_v01;

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Gets the registration state for different
             QMI_WMS indications for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Transport Layer Information Events */
  uint8_t reg_transport_layer_info_events_valid;  /**< Must be set to true if reg_transport_layer_info_events is being passed */
  uint8_t reg_transport_layer_info_events;
  /**<   Values: \n 
       - 0x00 -- Disable \n 
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Transport NW Reg Information Events */
  uint8_t reg_transport_nw_reg_info_events_valid;  /**< Must be set to true if reg_transport_nw_reg_info_events is being passed */
  uint8_t reg_transport_nw_reg_info_events;
  /**<   Values: \n 
       - 0x00 -- Disable \n 
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Call Status Information Events */
  uint8_t reg_call_status_info_events_valid;  /**< Must be set to true if reg_call_status_info_events is being passed */
  uint8_t reg_call_status_info_events;
  /**<   Values: \n 
       - 0x00 -- Disable \n 
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Service Ready Events */
  uint8_t reg_service_ready_events_valid;  /**< Must be set to true if reg_service_ready_events is being passed */
  uint8_t reg_service_ready_events;
  /**<   Values: \n 
       - 0x00 -- Disable \n 
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Broadcast Config Events */
  uint8_t reg_broadcast_config_events_valid;  /**< Must be set to true if reg_broadcast_config_events is being passed */
  uint8_t reg_broadcast_config_events;
  /**<   Values: \n 
       - 0x00 -- Disable \n 
       - 0x01 -- Enable
  */
}wms_get_indication_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Reads the SMS parameters from EF-SMSP. */
typedef struct {

  /* Mandatory */
  /*  Message Mode */
  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Values: \n 
         - 0x01 -- MESSAGE_MODE_GW -- GW
      */
}wms_get_sms_parameters_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_PID_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_PID_DEFAULT_V01 = 0x00, 
  WMS_PID_IMPLICIT_V01 = 0x20, 
  WMS_PID_TELEX_V01 = 0x21, 
  WMS_PID_G3_FAX_V01 = 0x22, 
  WMS_PID_G4_FAX_V01 = 0x23, 
  WMS_PID_VOICE_PHONE_V01 = 0x24, 
  WMS_PID_ERMES_V01 = 0x25, 
  WMS_PID_NAT_PAGING_V01 = 0x26, 
  WMS_PID_VIDEOTEX_V01 = 0x27, 
  WMS_PID_TELETEX_UNSPEC_V01 = 0x28, 
  WMS_PID_TELETEX_PSPDN_V01 = 0x29, 
  WMS_PID_TELETEX_CSPDN_V01 = 0x2a, 
  WMS_PID_TELETEX_PSTN_V01 = 0x2b, 
  WMS_PID_TELETEX_ISDN_V01 = 0x2c, 
  WMS_PID_UCI_V01 = 0x2d, 
  WMS_PID_MSG_HANDLING_V01 = 0x30, 
  WMS_PID_X400_V01 = 0x31, 
  WMS_PID_INTERNET_EMAIL_V01 = 0x32, 
  WMS_PID_SC_SPECIFIC_1_V01 = 0x38, 
  WMS_PID_SC_SPECIFIC_2_V01 = 0x39, 
  WMS_PID_SC_SPECIFIC_3_V01 = 0x3a, 
  WMS_PID_SC_SPECIFIC_4_V01 = 0x3b, 
  WMS_PID_SC_SPECIFIC_5_V01 = 0x3c, 
  WMS_PID_SC_SPECIFIC_6_V01 = 0x3d, 
  WMS_PID_SC_SPECIFIC_7_V01 = 0x3e, 
  WMS_PID_GSM_UMTS_V01 = 0x3f, 
  WMS_PID_SM_TYPE_0_V01 = 0x40, 
  WMS_PID_REPLACE_SM_1_V01 = 0x41, 
  WMS_PID_REPLACE_SM_2_V01 = 0x42, 
  WMS_PID_REPLACE_SM_3_V01 = 0x43, 
  WMS_PID_REPLACE_SM_4_V01 = 0x44, 
  WMS_PID_REPLACE_SM_5_V01 = 0x45, 
  WMS_PID_REPLACE_SM_6_V01 = 0x46, 
  WMS_PID_REPLACE_SM_7_V01 = 0x47, 
  WMS_PID_RETURN_CALL_V01 = 0x5f, 
  WMS_PID_ANSI136_R_DATA_V01 = 0x7c, 
  WMS_PID_ME_DATA_DOWNLOAD_V01 = 0x7d, 
  WMS_PID_ME_DEPERSONALIZE_V01 = 0x7e, 
  WMS_PID_SIM_DATA_DOWNLOAD_V01 = 0x7f, 
  WMS_PID_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_pid_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Reads the SMS parameters from EF-SMSP. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Destination Address */
  uint8_t dest_addr_valid;  /**< Must be set to true if dest_addr is being passed */
  uint32_t dest_addr_len;  /**< Must be set to # of elements in dest_addr */
  uint8_t dest_addr[WMS_DEST_ADDRESS_LENGTH_MAX_V01];
  /**<   Destination address as defined in \hyperref[S2]{[S2]}
       Section 9.2.3.8.
  */

  /* Optional */
  /*  Protocol Identifier Data */
  uint8_t pid_valid;  /**< Must be set to true if pid is being passed */
  wms_pid_enum_v01 pid;
  /**<   Protocol Identifier Data (PID) per \hyperref[S2]{[S2]}
       Section 9.2.3.9; see
       Table @latexonly \ref{tbl:ProtoIdent} @endlatexonly for
       more information.
  */

  /* Optional */
  /*  Data Coding Scheme */
  uint8_t dcs_valid;  /**< Must be set to true if dcs is being passed */
  uint8_t dcs;
  /**<   SMS data coding scheme as defined in \hyperref[S8]{[S8]} Section 4.
  */

  /* Optional */
  /*  Validity Period */
  uint8_t validity_valid;  /**< Must be set to true if validity is being passed */
  uint8_t validity;
  /**<   Relative validity period as defined in \hyperref[S2]{[S2]}
       Section 9.2.3.12.1.
  */
}wms_get_sms_parameters_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Writes the SMS parameters to EF-SMSP. */
typedef struct {

  /* Mandatory */
  /*  Message Mode */
  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Values: \n 
           - 0x01 -- MESSAGE_MODE_GW -- GW
     */

  /* Optional */
  /*  Destination Address */
  uint8_t dest_addr_valid;  /**< Must be set to true if dest_addr is being passed */
  uint32_t dest_addr_len;  /**< Must be set to # of elements in dest_addr */
  uint8_t dest_addr[WMS_DEST_ADDRESS_LENGTH_MAX_V01];
  /**<   Destination address as defined in \hyperref[S2]{[S2]} Section 9.2.3.8.
    */

  /* Optional */
  /*  Protocol Identifier Data */
  uint8_t pid_valid;  /**< Must be set to true if pid is being passed */
  wms_pid_enum_v01 pid;
  /**<   Protocol Identifier Data (PID) per \hyperref[S2]{[S2]} Section 9.2.3.9; see Table 
         @latexonly \ref{tbl:ProtoIdent} @endlatexonly for more information.
    */

  /* Optional */
  /*  Data Coding Scheme */
  uint8_t dcs_valid;  /**< Must be set to true if dcs is being passed */
  uint8_t dcs;
  /**<   SMS data coding scheme as defined in \hyperref[S8]{[S8]} Section 4.
    */

  /* Optional */
  /*  Validity Period */
  uint8_t validity_valid;  /**< Must be set to true if validity is being passed */
  uint8_t validity;
  /**<   Relative validity period as defined in \hyperref[S2]{[S2]} Section 9.2.3.12.1.
    */
}wms_set_sms_parameters_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Writes the SMS parameters to EF-SMSP. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}wms_set_sms_parameters_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_CALL_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_CALL_STATUS_INCOMING_V01 = 0x00, 
  WMS_CALL_STATUS_CONNECTED_V01 = 0x01, 
  WMS_CALL_STATUS_ABORTED_V01 = 0x02, 
  WMS_CALL_STATUS_DISCONNECTED_V01 = 0x03, 
  WMS_CALL_STATUS_CONNECTING_V01 = 0x04, 
  WMS_CALL_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_call_status_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Indication Message; Indicates a change in the SMS call status. */
typedef struct {

  /* Mandatory */
  /*  SMS Call Status Information */
  wms_call_status_enum_v01 call_status;
  /**<   Indicates the status of the SMS call. Values: \n 
       - 0x00 -- Incoming \n 
       - 0x01 -- Connected \n 
       - 0x02 -- Aborted \n 
       - 0x03 -- Disconnected \n 
       - 0x04 -- Connecting
    */
}wms_call_status_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_domain_pref_config_req_msg_v01;

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_LTE_DOMAIN_PREF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_LTE_DOMAIN_PREF_NONE_V01 = 0x00, 
  WMS_LTE_DOMAIN_PREF_IMS_V01 = 0x01, 
  WMS_LTE_DOMAIN_PREF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_lte_domain_pref_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Queries the domain preference configuration.
             \label{idl:getDomainPrefConfig} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  LTE Domain Preference */
  uint8_t lte_domain_pref_valid;  /**< Must be set to true if lte_domain_pref is being passed */
  wms_lte_domain_pref_enum_v01 lte_domain_pref;
  /**<   LTE domain preference. Values: \n
       - 0x00 -- WMS_LTE_DOMAIN_PREF_ NONE \n
       - 0x01 -- WMS_LTE_DOMAIN_PREF_ IMS
  */

  /* Optional */
  /*  GW Domain Preference */
  uint8_t gw_domain_pref_valid;  /**< Must be set to true if gw_domain_pref is being passed */
  wms_domain_pref_enum_v01 gw_domain_pref;
  /**<   GW domain preference. Values: \n
       - 0x00 -- DOMAIN_PREF_CS -- CS preferred \n
       - 0x01 -- DOMAIN_PREF_PS -- PS preferred \n
       - 0x02 -- DOMAIN_PREF_CS_ONLY -- CS only \n
       - 0x03 -- DOMAIN_PREF_PS_ONLY -- PS only
  */
}wms_get_domain_pref_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sets the domain preference configuration.
             \label{idl:setDomainPrefConfig} */
typedef struct {

  /* Optional */
  /*  LTE Domain Preference */
  uint8_t lte_domain_pref_valid;  /**< Must be set to true if lte_domain_pref is being passed */
  wms_lte_domain_pref_enum_v01 lte_domain_pref;
  /**<   LTE domain preference. Values: \n
       - 0x00 -- WMS_LTE_DOMAIN_PREF_ NONE \n
       - 0x01 -- WMS_LTE_DOMAIN_PREF_ IMS
  */

  /* Optional */
  /*  GW Domain Preference */
  uint8_t gw_domain_pref_valid;  /**< Must be set to true if gw_domain_pref is being passed */
  wms_domain_pref_enum_v01 gw_domain_pref;
  /**<   GW domain preference. Values: \n
       - 0x00 -- DOMAIN_PREF_CS -- CS preferred	\n
       - 0x01 -- DOMAIN_PREF_PS -- PS preferred	\n
       - 0x02 -- DOMAIN_PREF_CS_ONLY -- CS only	\n
       - 0x03 -- DOMAIN_PREF_PS_ONLY -- PS only
  */
}wms_set_domain_pref_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sets the domain preference configuration.
             \label{idl:setDomainPrefConfig} */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  LTE Domain Preference Status */
  uint8_t lte_domain_pref_outcome_valid;  /**< Must be set to true if lte_domain_pref_outcome is being passed */
  qmi_error_type_v01 lte_domain_pref_outcome;
  /**<   Error code; possible error code values are described in the error codes 
       section of each message definition
  */

  /* Optional */
  /*  GW Domain Preference Status */
  uint8_t gw_domain_pref_outcome_valid;  /**< Must be set to true if gw_domain_pref_outcome is being passed */
  qmi_error_type_v01 gw_domain_pref_outcome;
  /**<   Error code; possible error code values are described in the error 
       codes section of each message definition.
  */
}wms_set_domain_pref_config_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_retry_period_req_msg_v01;

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Queries the retry period. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Retry Period */
  uint8_t retry_period_valid;  /**< Must be set to true if retry_period is being passed */
  uint32_t retry_period;
  /**<   WMS attempts to send a message up to the retry period in seconds before
       giving up. If retry_period is 0 sec, the retry is not attempted.
  */
}wms_get_retry_period_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_retry_interval_req_msg_v01;

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Queries the retry interval. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Retry Interval */
  uint8_t retry_interval_valid;  /**< Must be set to true if retry_interval is being passed */
  uint32_t retry_interval;
  /**<   Retry interval in seconds specifying the interval between WMS
       retry attempts.
  */
}wms_get_retry_interval_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_dc_disconnect_timer_req_msg_v01;

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Queries the CDMA dedicated channel autodisconnect timer. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  DC Auto Disconnect Timer */
  uint8_t dc_auto_disconn_timer_valid;  /**< Must be set to true if dc_auto_disconn_timer is being passed */
  uint32_t dc_auto_disconn_timer;
  /**<   Timeout period in seconds. A value of 0 means that the autodisconnect
       is disabled.
  */
}wms_get_dc_disconnect_timer_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_memory_status_req_msg_v01;

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Queries the client-set memory status for new SMS messages. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Memory Status Information */
  uint8_t memory_available_valid;  /**< Must be set to true if memory_available is being passed */
  uint8_t memory_available;
  /**<   Memory availability. Values: \n 
       - 0x00 -- Memory is not available \n 
       - 0x01 -- Memory is available
  */
}wms_get_memory_status_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_primary_client_req_msg_v01;

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Queries whether the client has set itself as the primary client of 
             QMI_WMS. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Primary Client Information */
  uint8_t primary_client_valid;  /**< Must be set to true if primary_client is being passed */
  uint8_t primary_client;
  /**<   Indicates whether the client is set as the primary client. Values: \n
       - 0x00 -- FALSE \n 
       - 0x01 -- TRUE
  */
}wms_get_primary_client_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_subscription_binding_req_msg_v01;

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Queries the specific subscription to which the control point is bound. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Subscription Type */
  uint8_t subs_type_valid;  /**< Must be set to true if subs_type is being passed */
  wms_subscription_type_enum_v01 subs_type;
  /**<   Subscription type. Values: \n 
       - 0x00 -- Primary subscription \n 
       - 0x01 -- Secondary subscription \n 
       - 0x02 -- Tertiary subscription \n
       - 0x03 to 0xFF -- Reserved
  */
}wms_get_subscription_binding_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sends a new message asynchronously in its raw format. */
typedef struct {

  /* Mandatory */
  /*  Raw Message Data */
  wms_send_raw_message_data_type_v01 raw_message_data;

  /* Optional */
  /*  Force on DC* */
  uint8_t force_on_dc_valid;  /**< Must be set to true if force_on_dc is being passed */
  wms_force_on_dc_type_v01 force_on_dc;

  /* Optional */
  /*  Follow on DC* */
  uint8_t follow_on_dc_valid;  /**< Must be set to true if follow_on_dc is being passed */
  wms_follow_on_dc_enum_v01 follow_on_dc;
  /**<   Flag to request not to disconnect the CDMA dedicated channel after
         the send operation is complete. This TLV can be included if more
         messages are expected to follow. Values: \n       
         - 0x01 -- FOLLOW_ON_DC_ON -- On (do not disconnect the DC after
           the send operation) \n 
         Any value other than 0x01 in this field is treated as an absence
         of this TLV.
    */

  /* Optional */
  /*  Link Control** */
  uint8_t link_timer_valid;  /**< Must be set to true if link_timer is being passed */
  uint8_t link_timer;
  /**<   Keeps the GW SMS link open for the specified number of seconds.
         Can be enabled if more messages are expected to follow.
    */

  /* Optional */
  /*  SMS on IMS */
  uint8_t sms_on_ims_valid;  /**< Must be set to true if sms_on_ims is being passed */
  uint8_t sms_on_ims;
  /**<   Indicates whether the message is to be sent on IMS. Values: \n
         - 0x00 -- Message is not to be sent on IMS \n
         - 0x01 -- Message is to be sent on IMS \n
         - 0x02 to 0xFF -- Reserved \n
         Note: In minor version 9, the implementation was changed in such a way
               that inclusion of this TLV may affect the SMS routing differently.
    */

  /* Optional */
  /*  Retry Message */
  uint8_t retry_message_valid;  /**< Must be set to true if retry_message is being passed */
  wms_retry_message_enum_v01 retry_message;
  /**<   Indicates this message is a retry message. Values: \n
         - 0x01 -- WMS_MESSAGE_IS_A_ RETRY -- Message is a retry message	\n
          Note: Any value other than 0x01 in this field is treated as an
          absence of this TLV.
    */

  /* Optional */
  /*  Retry Message ID */
  uint8_t retry_message_id_valid;  /**< Must be set to true if retry_message_id is being passed */
  uint32_t retry_message_id;
  /**<   Message ID to be used in the retry message. The message ID
         specified here is used instead of the messsage ID encoded in the
         raw message. \n
		 Note: This TLV is only meaningful if the Retry Message TLV is
         specified and set to 0x01.
    */

  /* Optional */
  /*  User Data */
  uint8_t user_data_valid;  /**< Must be set to true if user_data is being passed */
  uint32_t user_data;
  /**<   Enables the control point to associate the request with the
         corresponding indication. The control point might send numerous
         requests. This TLV will help the control point to identify the
         request for which the received indication belongs.
    */

  /* Optional */
  /*  Link Control Enabling Information** */
  uint8_t link_enable_mode_valid;  /**< Must be set to true if link_enable_mode is being passed */
  uint8_t link_enable_mode;
  /**<   Indicates whether to keep the link control enabled, until the option is
         modified by the client. Values: \n
         - 0x00 -- Enable link control once so that the lower layer keeps the
                   link up for a specified time until the next MO SMS is requested
                   or the timer expires \n
         - 0x01 -- Always enable link control \n
         Note: This TLV is valid only if the Link Control TLV is specified and 
         is set to a valid timer value..
    */
}wms_async_raw_send_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sends a new message asynchronously in its raw format. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_async_raw_send_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Indication Message; Asynchronous result of QMI_WMS_ASYNC_RAW_SEND_REQ. */
typedef struct {

  /* Mandatory */
  /*  Send Status */
  qmi_error_type_v01 send_status;
  /**<   Send status. Values: \n
    -QMI_ERR_NONE -- No error in the request \n
    -QMI_ERR_CAUSE_CODE -- SMS cause code: For CDMA, refer to 
                           \hyperref[S4]{[S4]}
                           Section 6.5.2.125; 
                           for GW, refer to \hyperref[S3]{[S3]}
                           Section 3.2.5 \n
    -QMI_ERR_MESSAGE_DELIVERY_ FAILURE -- Message could not be delivered \n
    -QMI_ERR_NO_MEMORY -- Device could not allocate memory to
                          formulate a response
    */

  /* Optional */
  /*  Message ID */
  uint8_t message_id_valid;  /**< Must be set to true if message_id is being passed */
  uint16_t message_id;
  /**<   Unique ID assigned by WMS for non-retry messages.
    */

  /* Optional */
  /*  Cause Code* */
  uint8_t cause_code_valid;  /**< Must be set to true if cause_code is being passed */
  wms_tl_cause_code_enum_v01 cause_code;
  /**<   WMS cause code per \hyperref[S4]{[S4]} Section 6.5.2.125; see
         Table @latexonly \ref{tbl:WMSCauseCodes} @endlatexonly for more
         information.
    */

  /* Optional */
  /*  Error Class* */
  uint8_t error_class_valid;  /**< Must be set to true if error_class is being passed */
  wms_error_class_send_enum_v01 error_class;
  /**<   Error class. Values: \n
         - 0x00 -- ERROR_CLASS_ TEMPORARY \n
         - 0x01 -- ERROR_CLASS_ PERMANENT 
    */

  /* Optional */
  /*  GW Cause Info** */
  uint8_t gw_cause_info_valid;  /**< Must be set to true if gw_cause_info is being passed */
  wms_gw_cause_info_type_v01 gw_cause_info;

  /* Optional */
  /*  Message Delivery Failure Type */
  uint8_t message_delivery_failure_type_valid;  /**< Must be set to true if message_delivery_failure_type is being passed */
  wms_message_delivery_failure_type_enum_v01 message_delivery_failure_type;
  /**<   Message delivery failure type. Values: \n
         - 0x00 -- WMS_MESSAGE_ DELIVERY_FAILURE_TEMPORARY \n
         - 0x01 -- WMS_MESSAGE_ DELIVERY_FAILURE_PERMANENT
    */

  /* Optional */
  /*  Message Delivery Failure Cause */
  uint8_t message_delivery_failure_cause_valid;  /**< Must be set to true if message_delivery_failure_cause is being passed */
  wms_message_delivery_failure_cause_enum_v01 message_delivery_failure_cause;
  /**<   Message delivery failure cause. Values: \n
        - 0x00 -- WMS_MESSAGE_ BLOCKED_DUE_TO_CALL_ CONTROL
    */

  /* Optional */
  /*  Call Control Modified Information */
  uint8_t call_control_modified_info_valid;  /**< Must be set to true if call_control_modified_info is being passed */
  wms_call_control_modified_info_type_v01 call_control_modified_info;

  /* Optional */
  /*  User Data */
  uint8_t user_data_valid;  /**< Must be set to true if user_data is being passed */
  uint32_t user_data;
  /**<   Identifies the request associated with this indication. 
    */
}wms_async_raw_send_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sends an ACK asynchronously to the network for transfer-only routes. */
typedef struct {

  /* Mandatory */
  /*  ACK Information */
  wms_ack_information_type_v01 ack_information;

  /* Optional */
  /*  3GPP2 Failure Information* */
  uint8_t wms_3gpp2_failure_information_valid;  /**< Must be set to true if wms_3gpp2_failure_information is being passed */
  wms_3gpp2_failure_information_type_v01 wms_3gpp2_failure_information;

  /* Optional */
  /*  3GPP Failure Information** */
  uint8_t wms_3gpp_failure_information_valid;  /**< Must be set to true if wms_3gpp_failure_information is being passed */
  wms_3gpp_failure_information_type_v01 wms_3gpp_failure_information;

  /* Optional */
  /*  SMS on IMS */
  uint8_t sms_on_ims_valid;  /**< Must be set to true if sms_on_ims is being passed */
  uint8_t sms_on_ims;
  /**<   Indicates whether ACK is to be sent on IMS. Values: \n 
         - 0x00 -- ACK is not to be sent on IMS \n 
         - 0x01 -- ACK is to be sent on IMS \n 
         - 0x02 to 0xFF -- Reserved \n 
         Note: In minor version 9, the implementation was changed in such a way
               that inclusion of this TLV may affect the SMS routing differently.
    */

  /* Optional */
  /*  User Data */
  uint8_t user_data_valid;  /**< Must be set to true if user_data is being passed */
  uint32_t user_data;
  /**<   Enables the control point to associate the ACK request with the
         corresponding indication. The control point might send numerous
         requests. This TLV will help the control point identify the
         request for which the received indication belongs.
    */
}wms_async_send_ack_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sends an ACK asynchronously to the network for transfer-only routes. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_async_send_ack_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Indication Message; Asynchronous result of QMI_WMS_ASYNC_SEND_ACK. */
typedef struct {

  /* Mandatory */
  /*  ACK Status */
  qmi_error_type_v01 ack_status;
  /**<   ACK status. Values: \n
    - QMI_ERR_NONE -- No error in the request \n            
    - QMI_ERR_MALFORMED_MSG -- Message was not formulated correctly 
                              by the control point or the message was
                              corrupted during transmission \n
    - QMI_ERR_NO_MEMORY -- Device could not allocate memory to 
                          formulate a response \n
    - QMI_ERR_ACK_NOT_SENT -- ACK could not be sent
    */

  /* Optional */
  /*  ACK Failure Cause */
  uint8_t failure_cause_valid;  /**< Must be set to true if failure_cause is being passed */
  wms_ack_failure_cause_enum_v01 failure_cause;
  /**<   ACK failure cause. Values: \n
         - 0x00 -- ACK_FAILURE_NO_ NETWORK_RESPONSE \n
         - 0x01 -- ACK_FAILURE_NETWORK_ RELEASED_LINK \n
         - 0x02 -- ACK_FAILURE_ACK_NOT_ SENT
    */

  /* Optional */
  /*  User Data */
  uint8_t user_data_valid;  /**< Must be set to true if user_data is being passed */
  uint32_t user_data;
  /**<   Identifies the ACK request associated with this indication.
    */
}wms_async_send_ack_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sends a message asynchronously from a memory store. */
typedef struct {

  /* Mandatory */
  /*  Message Memory Storage Information */
  wms_message_memory_storage_info_type_v01 message_memory_storage_info;

  /* Optional */
  /*  SMS on IMS */
  uint8_t sms_on_ims_valid;  /**< Must be set to true if sms_on_ims is being passed */
  uint8_t sms_on_ims;
  /**<   Indicates whether the message is to be sent on IMS. Values: \n 
         - 0x00 -- Message is not to be sent on IMS \n 
         - 0x01 -- Message is to be sent on IMS \n 
         - 0x02 to 0xFF -- Reserved \n 
         Note: In minor version 9, the implementation was changed in such a way
               that inclusion of this TLV may affect the SMS routing differently.
               
    */

  /* Optional */
  /*  User Data */
  uint8_t user_data_valid;  /**< Must be set to true if user_data is being passed */
  uint32_t user_data;
  /**<   Enables the control point to associate the send request with the
         corresponding indication. The control point might send numerous
         requests. This TLV will help the control point identify the
         request for which the received indication belongs.
    */
}wms_async_send_from_mem_store_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sends a message asynchronously from a memory store. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_async_send_from_mem_store_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Indication Message; Asynchronous result of QMI_WMS_ASYNC_SEND_FROM_MEM_STORE. */
typedef struct {

  /* Mandatory */
  /*  Send Status */
  qmi_error_type_v01 send_status;
  /**<   Send status. Values: \n
    -QMI_ERR_NONE -- No error in the request \n         
    -QMI_ERR_CAUSE_CODE -- SMS cause code: For CDMA, refer to 
                           \hyperref[S4]{[S4]} 
                           Section 6.5.2.125; 
                           for GW, refer to 
                           \hyperref[S3]{[S3]}
                           Section 3.2.5 \n
    -QMI_ERR_MESSAGE_DELIVERY_ FAILURE -- Message could not be delivered \n
    -QMI_ERR_NO_MEMORY -- Device could not allocate memory to
                          formulate a response  
    */

  /* Optional */
  /*  Message ID */
  uint8_t message_id_valid;  /**< Must be set to true if message_id is being passed */
  uint16_t message_id;
  /**<   WMS message ID.
    */

  /* Optional */
  /*  Cause Code* */
  uint8_t cause_code_valid;  /**< Must be set to true if cause_code is being passed */
  wms_tl_cause_code_enum_v01 cause_code;
  /**<   WMS cause code per \hyperref[S4]{[S4]} Section 6.5.2.125; see
         Table \ref{tbl:WMSCauseCodes} for more information.
    */

  /* Optional */
  /*  Error Class* */
  uint8_t error_class_valid;  /**< Must be set to true if error_class is being passed */
  wms_error_class_send_enum_v01 error_class;
  /**<   Error class. Values: \n 
         - 0x00 -- ERROR_CLASS_ TEMPORARY \n 
         - 0x01 -- ERROR_CLASS_ PERMANENT
    */

  /* Optional */
  /*  GW Cause Info** */
  uint8_t gw_cause_info_valid;  /**< Must be set to true if gw_cause_info is being passed */
  wms_gw_cause_info_type_v01 gw_cause_info;

  /* Optional */
  /*  Message Delivery Failure Type */
  uint8_t message_delivery_failure_type_valid;  /**< Must be set to true if message_delivery_failure_type is being passed */
  wms_message_delivery_failure_type_enum_v01 message_delivery_failure_type;
  /**<   Message delivery failure type. Values: \n 
       - 0x00 -- WMS_MESSAGE_ DELIVERY_FAILURE_ TEMPORARY \n
       - 0x01 -- WMS_MESSAGE_ DELIVERY_FAILURE_ PERMANENT
    */

  /* Optional */
  /*  Message Delivery Failure Cause */
  uint8_t message_delivery_failure_cause_valid;  /**< Must be set to true if message_delivery_failure_cause is being passed */
  wms_message_delivery_failure_cause_enum_v01 message_delivery_failure_cause;
  /**<   Message delivery failure cause. Values: \n
        - 0x00 -- WMS_MESSAGE_ BLOCKED_DUE_TO_CALL_ CONTROL
    */

  /* Optional */
  /*  Call Control Modified Information */
  uint8_t call_control_modified_info_valid;  /**< Must be set to true if call_control_modified_info is being passed */
  wms_call_control_modified_info_type_v01 call_control_modified_info;

  /* Optional */
  /*  User Data */
  uint8_t user_data_valid;  /**< Must be set to true if user_data is being passed */
  uint32_t user_data;
  /**<   Identifies the request associated with this indication.
    */
}wms_async_send_from_mem_store_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}wms_get_service_ready_status_req_msg_v01;

/** @addtogroup wms_qmi_enums
    @{
  */
typedef enum {
  WMS_SERVICE_READY_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WMS_SERVICE_READY_STATUS_NONE_V01 = 0x00, 
  WMS_SERVICE_READY_STATUS_3GPP_V01 = 0x01, 
  WMS_SERVICE_READY_STATUS_3GPP2_V01 = 0x02, 
  WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01 = 0x03, 
  WMS_SERVICE_READY_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wms_service_ready_status_enum_v01;
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Gets the service ready status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Service Ready Events Registration Information */
  uint8_t registered_ind_valid;  /**< Must be set to true if registered_ind is being passed */
  uint8_t registered_ind;
  /**<   Indicates whether service ready events are registered. Values: \n
       - 0x00 -- Service ready events are not registered \n
       - 0x01 -- Service ready events are registered
  */

  /* Optional */
  /*  SMS Service Ready Status Information */
  uint8_t ready_status_valid;  /**< Must be set to true if ready_status is being passed */
  wms_service_ready_status_enum_v01 ready_status;
  /**<   Indicates whether Service is ready to handle 3GPP2/3GPP SMS requests.
       Values: \n
       - 0x00 -- SMS Service is not ready \n 
       - 0x01 -- 3GPP SMS Service is ready \n 
       - 0x02 -- 3GPP2 SMS Service is ready \n 
       - 0x03 -- Both 3GPP and 3GPP2 SMS Service are ready \n
       Note: All other values are reserved and should be ignored by clients.
  */
}wms_get_service_ready_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Indication Message; Indicates whether the SMS service is ready. */
typedef struct {

  /* Mandatory */
  /*  SMS Service Ready Status Information */
  wms_service_ready_status_enum_v01 ready_status;
  /**<   Indicates which service is ready. Values: \n
       - 0x00 -- SMS Service is not ready \n
       - 0x01 -- 3GPP SMS Service is ready \n
       - 0x02 -- 3GPP2 SMS Service is ready \n
       - 0x03 -- Both 3GPP and 3GPP2 SMS Service is ready \n
       Note: All other values are reserved and should be ignored by clients.
  */
}wms_service_ready_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Indication Message; Indicates when broadcast configuration has been changed. */
typedef struct {

  /* Mandatory */
  /*  Broadcast Configuration Information */
  wms_message_mode_enum_v01 message_mode;
  /**<   Message mode. Values: \n
       - 0x00 -- MESSAGE_MODE_CDMA -- CDMA \n
       - 0x01 -- MESSAGE_MODE_GW   -- GW
  */

  /* Optional */
  /*  3GPP Broadcast Configuration Information* */
  uint8_t wms_3gpp_broadcast_info_valid;  /**< Must be set to true if wms_3gpp_broadcast_info is being passed */
  wms_3gpp_broadcast_info_config2_type_v01 wms_3gpp_broadcast_info;

  /* Optional */
  /*  3GPP2 Broadcast Configuration Information* */
  uint8_t wms_3gpp2_broadcast_info_valid;  /**< Must be set to true if wms_3gpp2_broadcast_info is being passed */
  wms_3gpp2_broadcast_info_config2_type_v01 wms_3gpp2_broadcast_info;
}wms_broadcast_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Request Message; Sets the message waiting information. */
typedef struct {

  /* Mandatory */
  /*  Message Waiting Information */
  uint32_t message_waiting_info_len;  /**< Must be set to # of elements in message_waiting_info */
  wms_message_waiting_information_type_v01 message_waiting_info[WMS_MESSAGE_TUPLE_MAX_V01];
}wms_set_message_waiting_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup wms_qmi_messages
    @{
  */
/** Response Message; Sets the message waiting information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}wms_set_message_waiting_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup wms_qmi_msg_ids
    @{
  */
#define QMI_WMS_RESET_REQ_V01 0x0000
#define QMI_WMS_RESET_RESP_V01 0x0000
#define QMI_WMS_SET_EVENT_REPORT_REQ_V01 0x0001
#define QMI_WMS_SET_EVENT_REPORT_RESP_V01 0x0001
#define QMI_WMS_EVENT_REPORT_IND_V01 0x0001
#define QMI_WMS_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_WMS_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_WMS_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_WMS_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_WMS_RAW_SEND_REQ_V01 0x0020
#define QMI_WMS_RAW_SEND_RESP_V01 0x0020
#define QMI_WMS_RAW_WRITE_REQ_V01 0x0021
#define QMI_WMS_RAW_WRITE_RESP_V01 0x0021
#define QMI_WMS_RAW_READ_REQ_V01 0x0022
#define QMI_WMS_RAW_RESP_MSG_V01 0x0022
#define QMI_WMS_MODIFY_TAG_REQ_V01 0x0023
#define QMI_WMS_MODIFY_TAG_RESP_V01 0x0023
#define QMI_WMS_DELETE_REQ_V01 0x0024
#define QMI_WMS_DELETE_RESP_V01 0x0024
#define QMI_WMS_GET_MESSAGE_PROTOCOL_REQ_V01 0x0030
#define QMI_WMS_GET_MESSAGE_PROTOCOL_RESP_V01 0x0030
#define QMI_WMS_LIST_MESSAGES_REQ_V01 0x0031
#define QMI_WMS_LIST_MESSAGES_RESP_V01 0x0031
#define QMI_WMS_SET_ROUTES_REQ_V01 0x0032
#define QMI_WMS_SET_ROUTES_RESP_V01 0x0032
#define QMI_WMS_GET_ROUTES_REQ_V01 0x0033
#define QMI_WMS_GET_ROUTES_RESP_V01 0x0033
#define QMI_WMS_GET_SMSC_ADDRESS_REQ_V01 0x0034
#define QMI_WMS_GET_SMSC_ADDRESS_RESP_V01 0x0034
#define QMI_WMS_SET_SMSC_ADDRESS_REQ_V01 0x0035
#define QMI_WMS_SET_SMSC_ADDRESS_RESP_V01 0x0035
#define QMI_WMS_GET_STORE_MAX_SIZE_REQ_V01 0x0036
#define QMI_WMS_GET_STORE_MAX_SIZE_RESP_V01 0x0036
#define QMI_WMS_SEND_ACK_REQ_V01 0x0037
#define QMI_WMS_SEND_ACK_RESP_V01 0x0037
#define QMI_WMS_SET_RETRY_PERIOD_REQ_V01 0x0038
#define QMI_WMS_SET_RETRY_PERIOD_RESP_V01 0x0038
#define QMI_WMS_SET_RETRY_INTERVAL_REQ_V01 0x0039
#define QMI_WMS_SET_RETRY_INTERVAL_RESP_V01 0x0039
#define QMI_WMS_SET_DC_DISCONNECT_TIMER_REQ_V01 0x003A
#define QMI_WMS_SET_DC_DISCONNECT_TIMER_RESP_V01 0x003A
#define QMI_WMS_SET_MEMORY_STATUS_REQ_V01 0x003B
#define QMI_WMS_SET_MEMORY_STATUS_RESP_V01 0x003B
#define QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01 0x003C
#define QMI_WMS_SET_BROADCAST_ACTIVATION_RESP_V01 0x003C
#define QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01 0x003D
#define QMI_WMS_SET_BROADCAST_CONFIG_RESP_V01 0x003D
#define QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01 0x003E
#define QMI_WMS_GET_BROADCAST_CONFIG_RESP_V01 0x003E
#define QMI_WMS_MEMORY_FULL_IND_V01 0x003F
#define QMI_WMS_GET_DOMAIN_PREF_REQ_V01 0x0040
#define QMI_WMS_GET_DOMAIN_PREF_RESP_V01 0x0040
#define QMI_WMS_SET_DOMAIN_PREF_REQ_V01 0x0041
#define QMI_WMS_SET_DOMAIN_PREF_RESP_V01 0x0041
#define QMI_WMS_SEND_FROM_MEM_STORE_REQ_V01 0x0042
#define QMI_WMS_SEND_FROM_MEM_STORE_RESP_V01 0x0042
#define QMI_WMS_GET_MESSAGE_WAITING_REQ_V01 0x0043
#define QMI_WMS_GET_MESSAGE_WAITING_RESP_V01 0x0043
#define QMI_WMS_MESSAGE_WAITING_IND_V01 0x0044
#define QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01 0x0045
#define QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01 0x0045
#define QMI_WMS_SMSC_ADDRESS_IND_V01 0x0046
#define QMI_WMS_INDICATION_REGISTER_REQ_V01 0x0047
#define QMI_WMS_INDICATION_REGISTER_RESP_V01 0x0047
#define QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01 0x0048
#define QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01 0x0048
#define QMI_WMS_TRANSPORT_LAYER_INFO_IND_V01 0x0049
#define QMI_WMS_GET_TRANSPORT_NW_REG_INFO_REQ_V01 0x004A
#define QMI_WMS_GET_TRANSPORT_NW_REG_INFO_RESP_V01 0x004A
#define QMI_WMS_TRANSPORT_NW_REG_INFO_IND_V01 0x004B
#define QMI_WMS_BIND_SUBSCRIPTION_REQ_V01 0x004C
#define QMI_WMS_BIND_SUBSCRIPTION_RESP_V01 0x004C
#define QMI_WMS_GET_INDICATION_REGISTER_REQ_V01 0x004D
#define QMI_WMS_GET_INDICATION_REGISTER_RESP_V01 0x004D
#define QMI_WMS_GET_SMS_PARAMETERS_REQ_V01 0x004E
#define QMI_WMS_GET_SMS_PARAMETERS_RESP_V01 0x004E
#define QMI_WMS_SET_SMS_PARAMETERS_REQ_V01 0x004F
#define QMI_WMS_SET_SMS_PARAMETERS_RESP_V01 0x004F
#define QMI_WMS_CALL_STATUS_IND_V01 0x0050
#define QMI_WMS_GET_DOMAIN_PREF_CONFIG_REQ_V01 0x0051
#define QMI_WMS_GET_DOMAIN_PREF_CONFIG_RESP_V01 0x0051
#define QMI_WMS_SET_DOMAIN_PREF_CONFIG_REQ_V01 0x0052
#define QMI_WMS_SET_DOMAIN_PREF_CONFIG_RESP_V01 0x0052
#define QMI_WMS_GET_RETRY_PERIOD_REQ_V01 0x0053
#define QMI_WMS_GET_RETRY_PERIOD_RESP_V01 0x0053
#define QMI_WMS_GET_RETRY_INTERVAL_REQ_V01 0x0054
#define QMI_WMS_GET_RETRY_INTERVAL_RESP_V01 0x0054
#define QMI_WMS_GET_DC_DISCONNECT_TIMER_REQ_V01 0x0055
#define QMI_WMS_GET_DC_DISCONNECT_TIMER_RESP_V01 0x0055
#define QMI_WMS_GET_MEMORY_STATUS_REQ_V01 0x0056
#define QMI_WMS_GET_MEMORY_STATUS_RESP_V01 0x0056
#define QMI_WMS_GET_PRIMARY_CLIENT_REQ_V01 0x0057
#define QMI_WMS_GET_PRIMARY_CLIENT_RESP_V01 0x0057
#define QMI_WMS_GET_SUBSCRIPTION_BINDING_REQ_V01 0x0058
#define QMI_WMS_GET_SUBSCRIPTION_BINDING_RESP_V01 0x0058
#define QMI_WMS_ASYNC_RAW_SEND_REQ_V01 0x0059
#define QMI_WMS_ASYNC_RAW_SEND_RESP_V01 0x0059
#define QMI_WMS_ASYNC_RAW_SEND_IND_V01 0x0059
#define QMI_WMS_ASYNC_SEND_ACK_REQ_V01 0x005A
#define QMI_WMS_ASYNC_SEND_ACK_RESP_V01 0x005A
#define QMI_WMS_ASYNC_SEND_ACK_IND_V01 0x005A
#define QMI_WMS_ASYNC_SEND_FROM_MEM_STORE_REQ_V01 0x005B
#define QMI_WMS_ASYNC_SEND_FROM_MEM_STORE_RESP_V01 0x005B
#define QMI_WMS_ASYNC_SEND_FROM_MEM_STORE_IND_V01 0x005B
#define QMI_WMS_GET_SERVICE_READY_STATUS_REQ_V01 0x005C
#define QMI_WMS_GET_SERVICE_READY_STATUS_RESP_V01 0x005C
#define QMI_WMS_SERVICE_READY_IND_V01 0x005D
#define QMI_WMS_BROADCAST_CONFIG_IND_V01 0x005E
#define QMI_WMS_SET_MESSAGE_WAITING_REQ_V01 0x005F
#define QMI_WMS_SET_MESSAGE_WAITING_RESP_V01 0x005F
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro wms_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type wms_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define wms_get_service_object_v01( ) \
          wms_get_service_object_internal_v01( \
            WMS_V01_IDL_MAJOR_VERS, WMS_V01_IDL_MINOR_VERS, \
            WMS_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

