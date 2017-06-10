#ifndef ATP_SERVICE_01_H
#define ATP_SERVICE_01_H
/**
  @file application_traffic_pairing_v01.h

  @brief This is the public header file which defines the atp service Data structures.

  This header file defines the types and structures that were defined in
  atp. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.6
   It was generated on: Fri Jan 10 2014 (Spin 0)
   From IDL File: application_traffic_pairing_v01.idl */

/** @defgroup atp_qmi_consts Constant values defined in the IDL */
/** @defgroup atp_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup atp_qmi_enums Enumerated types used in QMI messages */
/** @defgroup atp_qmi_messages Structures sent as QMI messages */
/** @defgroup atp_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup atp_qmi_accessor Accessor for QMI service object */
/** @defgroup atp_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup atp_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define ATP_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define ATP_V01_IDL_MINOR_VERS 0x00
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define ATP_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define ATP_V01_MAX_MESSAGE_ID 0x0025
/**
    @}
  */


/** @addtogroup atp_qmi_consts
    @{
  */
#define QMI_ATP_IPV6_ADDR_LEN_V01 16
#define QMI_ATP_IPV4_ADDR_LEN_V01 4
#define QMI_ATP_MAX_PACKAGE_NAME_LEN_V01 256
#define QMI_ATP_MAX_APN_NAME_LEN_V01 255
#define QMI_ATP_MAX_SERVICE_INFO_V01 256
#define QMI_ATP_MAX_HASH_VALUE_V01 64
#define QMI_ATP_MAX_UID_V01 64
#define QMI_ATP_MAX_PACKAGE_INFO_V01 1024
#define QMI_ATP_MAX_APN_INFO_V01 8
#define QMI_ATP_MAX_ENTRIES_PER_IND_V01 50
/**
    @}
  */

/** @addtogroup atp_qmi_messages
    @{
  */
/** Request Message; Sets the registration state for different QMI_ATP indications
           for the requesting control point. */
typedef struct {

  /* Optional */
  /*  ATP Activation Status */
  uint8_t report_activation_status_valid;  /**< Must be set to true if report_activation_status is being passed */
  uint8_t report_activation_status;
  /**<   Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report activation status
  */

  /* Optional */
  /*  Policy Information Change */
  uint8_t report_policy_info_valid;  /**< Must be set to true if report_policy_info is being passed */
  uint8_t report_policy_info;
  /**<   Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report policy information
  */
}atp_indication_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup atp_qmi_messages
    @{
  */
/** Response Message; Sets the registration state for different QMI_ATP indications
           for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  ATP Activation Status */
  uint8_t activation_status_valid;  /**< Must be set to true if activation_status is being passed */
  uint8_t activation_status;
  /**<   Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report activation status
  */

  /* Optional */
  /*  Policy Information Change */
  uint8_t policy_info_valid;  /**< Must be set to true if policy_info is being passed */
  uint8_t policy_info;
  /**<   Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report policy information
  */
}atp_indication_register_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}atp_activation_status_query_req_msg_v01;

/** @addtogroup atp_qmi_messages
    @{
  */
/** Response Message; Query for the ATP activation status */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  ATP Activation Status */
  uint8_t is_activated_valid;  /**< Must be set to true if is_activated is being passed */
  uint8_t is_activated;
  /**<   Values:  \n
       - 0 -- Deactivated \n
       - 1 -- Activated
  */
}atp_activation_status_query_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup atp_qmi_messages
    @{
  */
/** Indication Message; Indicates the current ATP activation status */
typedef struct {

  /* Mandatory */
  /*  ATP Activation Status */
  uint8_t is_activated;
  /**<   Values:  \n
       - 0 -- Deactivated \n
       - 1 -- Activated
  */
}atp_activation_status_ind_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}atp_policy_info_req_msg_v01;

/** @addtogroup atp_qmi_messages
    @{
  */
/** Response Message; Query for the ATP Policy Information */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}atp_policy_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup atp_qmi_aggregates
    @{
  */
typedef struct {

  char apn[QMI_ATP_MAX_APN_NAME_LEN_V01 + 1];
  /**<   APN name string.
   */

  uint32_t service_id;
  /**<   Service ID.
   */

  char package_name[QMI_ATP_MAX_PACKAGE_NAME_LEN_V01 + 1];
  /**<   Package name string.
  */

  uint32_t framework_uid_len;  /**< Must be set to # of elements in framework_uid */
  uint32_t framework_uid[QMI_ATP_MAX_UID_V01];
  /**<   Framework UIDs.
    */

  uint32_t hash_values_len;  /**< Must be set to # of elements in hash_values */
  uint32_t hash_values[QMI_ATP_MAX_HASH_VALUE_V01];
  /**<   Hash value array.
  */

  uint32_t max_ack_timeout;
  /**<   Timeout value for each filter report, if no ack is received from
       NW in the duration, socket call shall be returned.
  */
}atp_policy_info_entry_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup atp_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t list_id;
  /**<   Unique ID for this policy information report (same for all segments).
       list_id - 0x00 is reserved - Do not use.
  */

  uint32_t total_list_entries;
  /**<   Total number of entries in the list received from the network
       e.g. If the list is a total of 500 entries and is being sent in
       multiple indications, total_list_entries is set to 500 in all
       indications.
  */

  uint32_t policy_info_len;  /**< Must be set to # of elements in policy_info */
  atp_policy_info_entry_type_v01 policy_info[QMI_ATP_MAX_ENTRIES_PER_IND_V01];
  /**<   ATP policy information segments from the network
  */
}atp_policy_list_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup atp_qmi_messages
    @{
  */
/** Indication Message; Indicates the change in ATP in policy information */
typedef struct {

  /* Optional */
  /*  ATP Policy Information from Network */
  uint8_t atp_policy_list_valid;  /**< Must be set to true if atp_policy_list is being passed */
  atp_policy_list_type_v01 atp_policy_list;
  /**<   Policy information based on each package from the network.
  */
}atp_policy_info_change_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup atp_qmi_enums
    @{
  */
typedef enum {
  ATP_FILTER_ACTION_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  ATP_FILTER_ACTION_ADD_V01 = 0x01,
  ATP_FILTER_ACTION_REMOVE_V01 = 0x02,
  ATP_FILTER_ACTION_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}atp_filter_action_enum_type_v01;
/**
    @}
  */

/** @addtogroup atp_qmi_enums
    @{
  */
typedef enum {
  ATP_FILTER_PROTOCOL_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  ATP_FILTER_PROTOCOL_UDP_V01 = 0,
  ATP_FILTER_PROTOCOL_TCP_V01 = 1,
  ATP_FILTER_PROTOCOL_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}atp_filter_protocol_enum_type_v01;
/**
    @}
  */

/** @addtogroup atp_qmi_enums
    @{
  */
typedef enum {
  ATP_FILTER_ADDR_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  ATP_FILTER_ADDR_V4_V01 = 0,
  ATP_FILTER_ADDR_V6_V01 = 1,
  ATP_FILTER_ADDR_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}atp_filter_addr_enum_type_v01;
/**
    @}
  */

/** @addtogroup atp_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t src_ipv6_addr[QMI_ATP_IPV6_ADDR_LEN_V01];
  /**<   IPv6 address (in network byte order); this is
       an 8-element array of 16-bit numbers, each
       of which is in big-endian format.
  */

  uint8_t src_ipv6_prefix_length;
  /**<   IPv6 prefix length in number of bits; it can
       take a value between 0 and 128.
  */
}atp_filter_report_src_ipv6_addr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup atp_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t dest_ipv6_addr[QMI_ATP_IPV6_ADDR_LEN_V01];
  /**<   IPv6 address (in network byte order); this is
       an 8-element array of 16-bit numbers, each
       of which is in big-endian format.
  */

  uint8_t dest_ipv6_prefix_length;
  /**<   IPv6 prefix length in number of bits; it can
       take a value between 0 and 128.
  */
}atp_filter_report_dest_ipv6_addr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup atp_qmi_messages
    @{
  */
/** Request Message; Request the modem for filter report */
typedef struct {

  /* Mandatory */
  /*  Report ID */
  uint32_t report_id;
  /**<   A track ID generated by control point, will be returned
       in the corresponding indication.
   */

  /* Optional */
  /*  Filter Report Action  */
  uint8_t action_valid;  /**< Must be set to true if action is being passed */
  atp_filter_action_enum_type_v01 action;
  /**<   Filter action.
   */

  /* Optional */
  /*  Service ID */
  uint8_t service_id_valid;  /**< Must be set to true if service_id is being passed */
  uint32_t service_id;
  /**<   Service ID.
   */

  /* Optional */
  /*  Package Name */
  uint8_t package_name_valid;  /**< Must be set to true if package_name is being passed */
  char package_name[QMI_ATP_MAX_PACKAGE_NAME_LEN_V01 + 1];
  /**<   Package name.
   */

  /* Optional */
  /*  Hash Values */
  uint8_t hash_values_valid;  /**< Must be set to true if hash_values is being passed */
  uint32_t hash_values_len;  /**< Must be set to # of elements in hash_values */
  uint32_t hash_values[QMI_ATP_MAX_HASH_VALUE_V01];
  /**<   Hash value array.
   */

  /* Optional */
  /*  IP Type */
  uint8_t ip_type_valid;  /**< Must be set to true if ip_type is being passed */
  atp_filter_addr_enum_type_v01 ip_type;
  /**<   IP type: v4 or v6.
  */

  /* Optional */
  /*  Destination IPv4 Address */
  uint8_t dest_ipv4_addr_valid;  /**< Must be set to true if dest_ipv4_addr is being passed */
  uint8_t dest_ipv4_addr[QMI_ATP_IPV4_ADDR_LEN_V01];
  /**<   Destination IPv4 address.
  */

  /* Optional */
  /*  Destination IPv6 Address */
  uint8_t dest_ipv6_addr_valid;  /**< Must be set to true if dest_ipv6_addr is being passed */
  atp_filter_report_dest_ipv6_addr_type_v01 dest_ipv6_addr;
  /**<   Destination IPv6 address.
  */

  /* Optional */
  /*  Destination Port */
  uint8_t dest_port_valid;  /**< Must be set to true if dest_port is being passed */
  uint16_t dest_port;
  /**<   Destination port.
  */

  /* Optional */
  /*  Source IPv4 Address */
  uint8_t src_ipv4_addr_valid;  /**< Must be set to true if src_ipv4_addr is being passed */
  uint8_t src_ipv4_addr[QMI_ATP_IPV4_ADDR_LEN_V01];
  /**<   Source IPv4 address.
    */

  /* Optional */
  /*  Source IPv6 Address */
  uint8_t src_ipv6_addr_valid;  /**< Must be set to true if src_ipv6_addr is being passed */
  atp_filter_report_src_ipv6_addr_type_v01 src_ipv6_addr;
  /**<   Source IPv6 address.
    */

  /* Optional */
  /*  Source Port */
  uint8_t src_port_valid;  /**< Must be set to true if src_port is being passed */
  uint16_t src_port;
  /**<   Source port.
    */

  /* Optional */
  /*  Protocol Type */
  uint8_t protocol_valid;  /**< Must be set to true if protocol is being passed */
  atp_filter_protocol_enum_type_v01 protocol;
  /**<   Protocol type.
    */
}atp_send_filter_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup atp_qmi_messages
    @{
  */
/** Response Message; Request the modem for filter report */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}atp_send_filter_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup atp_qmi_enums
    @{
  */
typedef enum {
  ATP_FILTER_REPORT_RESP_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  ATP_FILTER_REPORT_ACK_V01 = 0,
  ATP_FILTER_REPORT_NAK_V01 = 1,
  ATP_FILTER_REPORT_RESP_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}atp_filter_report_resp_enum_type_v01;
/**
    @}
  */

/** @addtogroup atp_qmi_messages
    @{
  */
/** Indication Message; Indicates the result of filter report request */
typedef struct {

  /* Mandatory */
  /*  Report ID */
  uint32_t report_id;
  /**<   Report ID.
  */

  /* Optional */
  /*  Filter Report Response */
  uint8_t filter_report_resp_valid;  /**< Must be set to true if filter_report_resp is being passed */
  atp_filter_report_resp_enum_type_v01 filter_report_resp;
  /**<   Network returned filter report response.
  */
}atp_send_filter_report_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup atp_qmi_msg_ids
    @{
  */
#define QMI_ATP_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_ATP_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_ATP_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_ATP_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_ATP_INDICATION_REGISTER_REQ_V01 0x0020
#define QMI_ATP_INDICATION_REGISTER_RESP_V01 0x0020
#define QMI_ATP_ACTIVATION_STATUS_QUERY_REQ_V01 0x0021
#define QMI_ATP_ACTIVATION_STATUS_QUERY_RESP_V01 0x0021
#define QMI_ATP_ACTIVATION_STATUS_IND_V01 0x0022
#define QMI_ATP_POLICY_INFO_QUERY_REQ_V01 0x0023
#define QMI_ATP_POLICY_INFO_QUERY_RESP_V01 0x0023
#define QMI_ATP_POLICY_INFO_CHANGE_IND_V01 0x0024
#define QMI_ATP_SEND_FILTER_REPORT_REQ_V01 0x0025
#define QMI_ATP_SEND_FILTER_REPORT_RESP_V01 0x0025
#define QMI_ATP_SEND_FILTER_REPORT_IND_V01 0x0025
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro atp_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type atp_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define atp_get_service_object_v01( ) \
          atp_get_service_object_internal_v01( \
            ATP_V01_IDL_MAJOR_VERS, ATP_V01_IDL_MINOR_VERS, \
            ATP_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

