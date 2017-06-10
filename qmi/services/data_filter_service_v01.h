#ifndef DFS_SERVICE_01_H
#define DFS_SERVICE_01_H
/**
  @file data_filter_service_v01.h

  @brief This is the public header file which defines the dfs service Data structures.

  This header file defines the types and structures that were defined in
  dfs. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All rights reserved.
  Qualcomm Technologies Proprietary and Confidential.



  $Header: //source/qcom/qct/interfaces/qmi/dfs/main/latest/api/data_filter_service_v01.h#12 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.5 
   It was generated on: Fri Jun 19 2015 (Spin 0)
   From IDL File: data_filter_service_v01.idl */

/** @defgroup dfs_qmi_consts Constant values defined in the IDL */
/** @defgroup dfs_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup dfs_qmi_enums Enumerated types used in QMI messages */
/** @defgroup dfs_qmi_messages Structures sent as QMI messages */
/** @defgroup dfs_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup dfs_qmi_accessor Accessor for QMI service object */
/** @defgroup dfs_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"
#include "data_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup dfs_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define DFS_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define DFS_V01_IDL_MINOR_VERS 0x09
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define DFS_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define DFS_V01_MAX_MESSAGE_ID 0x0036
/**
    @}
  */


/** @addtogroup dfs_qmi_consts
    @{
  */
#define QMI_DFS_IPV6_ADDR_LEN_V01 16
#define QMI_DFS_MAX_FILTERS_V01 255
#define QMI_DFS_MAX_ALLOCATED_SOCKETS_V01 255
/**
    @}
  */

/** @addtogroup dfs_qmi_enums
    @{
  */
typedef enum {
  DFS_IP_FAMILY_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DFS_IP_FAMILY_IPV4_V01 = 0x04, /**<  IPv4\n  */
  DFS_IP_FAMILY_IPV6_V01 = 0x06, /**<  IPv6  */
  DFS_IP_FAMILY_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dfs_ip_family_enum_v01;
/**
    @}
  */

/** @addtogroup dfs_qmi_enums
    @{
  */
typedef enum {
  DFS_BIND_SUBSCRIPTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DFS_DEFAULT_SUBS_V01 = 0x0000, /**<  Default data subscription \n  */
  DFS_PRIMARY_SUBS_V01 = 0x0001, /**<  Primary\n  */
  DFS_SECONDARY_SUBS_V01 = 0x0002, /**<  Secondary\n  */
  DFS_TERTIARY_SUBS_V01 = 0x0003, /**<  Tertiary\n  */
  DFS_DONT_CARE_SUBS_V01 = 0x00FF, /**<  Default value used in the absence of
       explicit binding  */
  DFS_BIND_SUBSCRIPTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dfs_bind_subscription_enum_v01;
/**
    @}
  */

typedef uint64_t dfs_ipv4_filter_mask_v01;
#define QMI_DFS_IPV4_FILTER_MASK_NONE_V01 ((dfs_ipv4_filter_mask_v01)0x0000000000000000ull) /**<  No parameters  */
#define QMI_DFS_IPV4_FILTER_MASK_SRC_ADDR_V01 ((dfs_ipv4_filter_mask_v01)0x0000000000000001ull) /**<  IPv4 source address  */
#define QMI_DFS_IPV4_FILTER_MASK_DEST_ADDR_V01 ((dfs_ipv4_filter_mask_v01)0x0000000000000002ull) /**<  IPv4 destination address  */
#define QMI_DFS_IPV4_FILTER_MASK_TOS_V01 ((dfs_ipv4_filter_mask_v01)0x0000000000000004ull) /**<  IPv4 type of service  */
typedef uint64_t dfs_ipv6_filter_mask_v01;
#define QMI_DFS_IPV6_FILTER_MASK_NONE_V01 ((dfs_ipv6_filter_mask_v01)0x0000000000000000ull) /**<  No parameters  */
#define QMI_DFS_IPV6_FILTER_MASK_SRC_ADDR_V01 ((dfs_ipv6_filter_mask_v01)0x0000000000000001ull) /**<  IPv6 source address  */
#define QMI_DFS_IPV6_FILTER_MASK_DEST_ADDR_V01 ((dfs_ipv6_filter_mask_v01)0x0000000000000002ull) /**<  IPv6 destination address  */
#define QMI_DFS_IPV6_FILTER_MASK_TRAFFIC_CLASS_V01 ((dfs_ipv6_filter_mask_v01)0x0000000000000004ull) /**<  IPv6 traffic class  */
#define QMI_DFS_IPV6_FILTER_MASK_FLOW_LABEL_V01 ((dfs_ipv6_filter_mask_v01)0x0000000000000008ull) /**<  IPv6 flow label  */
/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ipv4_addr;
  /**<   IPv4 address.
   */

  uint32_t subnet_mask;
  /**<   IPv4 subnet mask.
   */
}dfs_ipv4_addr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t val;
  /**<   Type of service value. */

  uint8_t mask;
  /**<   Type of service mask. */
}dfs_ipv4_tos_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  dfs_ipv4_filter_mask_v01 valid_params;
  /**<   The bits set in this mask denote which parameters contain valid values. Values: \n
      - QMI_DFS_IPV4_FILTER_MASK_NONE (0x0000000000000000) --  No parameters 
      - QMI_DFS_IPV4_FILTER_MASK_SRC_ADDR (0x0000000000000001) --  IPv4 source address 
      - QMI_DFS_IPV4_FILTER_MASK_DEST_ADDR (0x0000000000000002) --  IPv4 destination address 
      - QMI_DFS_IPV4_FILTER_MASK_TOS (0x0000000000000004) --  IPv4 type of service 
 */

  dfs_ipv4_addr_type_v01 src_addr;
  /**<   IPv4 source address.
   */

  dfs_ipv4_addr_type_v01 dest_addr;
  /**<   IPv4 destination address.
   */

  dfs_ipv4_tos_type_v01 tos;
  /**<   IPv4 type of service.
   */
}dfs_ipv4_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t val;
  /**<   Traffic class value. */

  uint8_t mask;
  /**<   Traffic class mask. */
}dfs_ipv6_trf_cls_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t ipv6_address[QMI_DFS_IPV6_ADDR_LEN_V01];
  /**<   IPv6 address.
   */

  uint8_t prefix_len;
  /**<   IPv6 address prefix length.
   */
}dfs_ipv6_addr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  dfs_ipv6_filter_mask_v01 valid_params;
  /**<   The bits set in this mask denote which parameters contain valid values. Values: \n
      - QMI_DFS_IPV6_FILTER_MASK_NONE (0x0000000000000000) --  No parameters 
      - QMI_DFS_IPV6_FILTER_MASK_SRC_ADDR (0x0000000000000001) --  IPv6 source address 
      - QMI_DFS_IPV6_FILTER_MASK_DEST_ADDR (0x0000000000000002) --  IPv6 destination address 
      - QMI_DFS_IPV6_FILTER_MASK_TRAFFIC_CLASS (0x0000000000000004) --  IPv6 traffic class 
      - QMI_DFS_IPV6_FILTER_MASK_FLOW_LABEL (0x0000000000000008) --  IPv6 flow label 
 */

  dfs_ipv6_addr_type_v01 src_addr;
  /**<   IPv6 source address.
   */

  dfs_ipv6_addr_type_v01 dest_addr;
  /**<   IPv6 destination address.
   */

  dfs_ipv6_trf_cls_type_v01 trf_cls;
  /**<   IPv6 traffic class.
   */

  uint32_t flow_label;
  /**<   IPv6 flow label.
   */
}dfs_ipv6_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  dfs_ip_family_enum_v01 ip_version;
  /**<   Depending on the IP version set, either the IPv4 or the IPv6 information is valid. Values:\n
      - DFS_IP_FAMILY_IPV4 (0x04) --  IPv4\n 
      - DFS_IP_FAMILY_IPV6 (0x06) --  IPv6 
 */

  dfs_ipv4_info_type_v01 v4_info;
  /**<   Filter parameters for IPv4.
   */

  dfs_ipv6_info_type_v01 v6_info;
  /**<   Filter parameters for IPv6.
   */
}dfs_ip_header_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t port;
  /**<   Port.
  */

  uint16_t range;
  /**<   Range.
   */
}dfs_port_type_v01;  /* Type */
/**
    @}
  */

typedef uint64_t dfs_port_info_filter_mask_v01;
#define QMI_DFS_PORT_INFO_FILTER_MASK_NONE_V01 ((dfs_port_info_filter_mask_v01)0x0000000000000000ull) /**<  No parameters  */
#define QMI_DFS_PORT_INFO_FILTER_MASK_SRC_PORT_V01 ((dfs_port_info_filter_mask_v01)0x0000000000000001ull) /**<  Source port  */
#define QMI_DFS_PORT_INFO_FILTER_MASK_DEST_PORT_V01 ((dfs_port_info_filter_mask_v01)0x0000000000000002ull) /**<  Destination port  */
/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  dfs_port_info_filter_mask_v01 valid_params;
  /**<   The bits set in this mask denote which parameters contain valid values. Values: \n
      - QMI_DFS_PORT_INFO_FILTER_MASK_NONE (0x0000000000000000) --  No parameters 
      - QMI_DFS_PORT_INFO_FILTER_MASK_SRC_PORT (0x0000000000000001) --  Source port 
      - QMI_DFS_PORT_INFO_FILTER_MASK_DEST_PORT (0x0000000000000002) --  Destination port 
 */

  dfs_port_type_v01 src_port_info;
  /**<   Source port information.
  */

  dfs_port_type_v01 dest_port_info;
  /**<   Destination port information.
   */
}dfs_port_info_type_v01;  /* Type */
/**
    @}
  */

typedef uint64_t dfs_icmp_filter_mask_v01;
#define QMI_DFS_ICMP_FILTER_MASK_NONE_V01 ((dfs_icmp_filter_mask_v01)0x0000000000000000ull) /**<  No parameters  */
#define QMI_DFS_ICMP_FILTER_MASK_MSG_TYPE_V01 ((dfs_icmp_filter_mask_v01)0x0000000000000001ull) /**<  Message type  */
#define QMI_DFS_ICMP_FILTER_MASK_MSG_CODE_V01 ((dfs_icmp_filter_mask_v01)0x0000000000000002ull) /**<  Message code  */
/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  dfs_icmp_filter_mask_v01 valid_params;
  /**<   The bits set in this mask denote which parameters contain valid values. Values: \n
      - QMI_DFS_ICMP_FILTER_MASK_NONE (0x0000000000000000) --  No parameters 
      - QMI_DFS_ICMP_FILTER_MASK_MSG_TYPE (0x0000000000000001) --  Message type 
      - QMI_DFS_ICMP_FILTER_MASK_MSG_CODE (0x0000000000000002) --  Message code 
 */

  uint8_t type;
  /**<   ICMP type.
  */

  uint8_t code;
  /**<   ICMP code.
   */
}dfs_icmp_info_type_v01;  /* Type */
/**
    @}
  */

typedef uint64_t dfs_ipsec_filter_mask_v01;
#define QMI_DFS_IPSEC_FILTER_MASK_NONE_V01 ((dfs_ipsec_filter_mask_v01)0x0000000000000000ull) /**<  No parameters  */
#define QMI_DFS_IPSEC_FILTER_MASK_SPI_V01 ((dfs_ipsec_filter_mask_v01)0x0000000000000001ull) /**<  Security parameter index  */
/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  dfs_ipsec_filter_mask_v01 valid_params;
  /**<   The bits set in this mask denote which parameters contain valid values. Values: \n
      - QMI_DFS_IPSEC_FILTER_MASK_NONE (0x0000000000000000) --  No parameters 
      - QMI_DFS_IPSEC_FILTER_MASK_SPI (0x0000000000000001) --  Security parameter index 
 */

  uint32_t spi;
  /**<   Security parameter index for IPSec.
   */
}dfs_ipsec_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dfs_qmi_enums
    @{
  */
typedef enum {
  DFS_XPORT_PROTOCOL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DFS_PROTO_NONE_V01 = 0x00, /**<  No transport protocol\n  */
  DFS_PROTO_ICMP_V01 = 0x01, /**<  Internet Control Messaging Protocol\n  */
  DFS_PROTO_TCP_V01 = 0x06, /**<  Transmission Control Protocol\n  */
  DFS_PROTO_UDP_V01 = 0x11, /**<  User Datagram Protocol\n  */
  DFS_PROTO_ESP_V01 = 0x32, /**<  Encapsulating Security Payload protocol  */
  DFS_PROTO_AH_V01 = 0x33, /**<  Authentication Header protocol  */
  DFS_PROTO_ICMP6_V01 = 0x3A, /**<  ICMPv6 protocol  */
  DFS_XPORT_PROTOCOL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dfs_xport_protocol_enum_v01;
/**
    @}
  */

/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  dfs_xport_protocol_enum_v01 xport_protocol;
  /**<   Depending on the value in xport_protocol, only one field of icmp_info, 
 tcp_info, udp_info, esp_info, or ah_info is valid. DFS_PROTO_NONE 
 implies that no transport level protocol parameters are valid. Values:\n
      - DFS_PROTO_NONE (0x00) --  No transport protocol\n 
      - DFS_PROTO_ICMP (0x01) --  Internet Control Messaging Protocol\n 
      - DFS_PROTO_TCP (0x06) --  Transmission Control Protocol\n 
      - DFS_PROTO_UDP (0x11) --  User Datagram Protocol\n 
      - DFS_PROTO_ESP (0x32) --  Encapsulating Security Payload protocol 
      - DFS_PROTO_AH (0x33) --  Authentication Header protocol 
      - DFS_PROTO_ICMP6 (0x3A) --  ICMPv6 protocol 
 */

  dfs_port_info_type_v01 tcp_info;
  /**<   Filter parameters for TCP.
   */

  dfs_port_info_type_v01 udp_info;
  /**<   Filter parameters for UDP.
   */

  dfs_icmp_info_type_v01 icmp_info;
  /**<   Filter parameters for ICMP.
   */

  dfs_ipsec_info_type_v01 esp_info;
  /**<   Filter parameters for ESP.
   */

  dfs_ipsec_info_type_v01 ah_info;
  /**<   Filter parameters for AH.
    */
}dfs_xport_header_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  dfs_ip_header_type_v01 ip_info;
  /**<   Internet protocol filter parameters.
  */

  dfs_xport_header_type_v01 xport_info;
  /**<   Transport level protocol filter parameters.
   */
}dfs_filter_rule_type_v01;  /* Type */
/**
    @}
  */

typedef uint64_t dfs_filter_param_error_mask_v01;
#define QMI_DFS_FILTER_PARAM_NONE_V01 ((dfs_filter_param_error_mask_v01)0x0000000000000000ull) /**<  No errors \n  */
#define QMI_DFS_FILTER_PARAM_IP_VERSION_V01 ((dfs_filter_param_error_mask_v01)0x0000000000000001ull) /**<  IP version \n  */
#define QMI_DFS_FILTER_PARAM_IPV4_SRC_ADDR_V01 ((dfs_filter_param_error_mask_v01)0x0000000000000002ull) /**<  IPv4 source address \n  */
#define QMI_DFS_FILTER_PARAM_IPV4_DEST_ADDR_V01 ((dfs_filter_param_error_mask_v01)0x0000000000000004ull) /**<  IPv4 destination address \n  */
#define QMI_DFS_FILTER_PARAM_IPV4_TOS_V01 ((dfs_filter_param_error_mask_v01)0x0000000000000008ull) /**<  IPv4 type of service \n  */
#define QMI_DFS_FILTER_PARAM_IPV6_SRC_ADDR_V01 ((dfs_filter_param_error_mask_v01)0x0000000000000010ull) /**<  IPv6 source address \n  */
#define QMI_DFS_FILTER_PARAM_IPV6_DEST_ADDR_V01 ((dfs_filter_param_error_mask_v01)0x0000000000000020ull) /**<  IPv6 destination address \n  */
#define QMI_DFS_FILTER_PARAM_IPV6_TRF_CLS_V01 ((dfs_filter_param_error_mask_v01)0x0000000000000040ull) /**<  IPv6 traffic class \n  */
#define QMI_DFS_FILTER_PARAM_IPV6_FLOW_LABEL_V01 ((dfs_filter_param_error_mask_v01)0x0000000000000080ull) /**<  IPv6 flow label \n  */
#define QMI_DFS_FILTER_PARAM_XPORT_PROT_V01 ((dfs_filter_param_error_mask_v01)0x0000000000000100ull) /**<  Transport protocol \n  */
#define QMI_DFS_FILTER_PARAM_TCP_SRC_PORT_V01 ((dfs_filter_param_error_mask_v01)0x0000000000000200ull) /**<  TCP source port \n  */
#define QMI_DFS_FILTER_PARAM_TCP_DEST_PORT_V01 ((dfs_filter_param_error_mask_v01)0x0000000000000400ull) /**<  TCP destination port \n  */
#define QMI_DFS_FILTER_PARAM_UDP_SRC_PORT_V01 ((dfs_filter_param_error_mask_v01)0x0000000000000800ull) /**<  UDP source port \n  */
#define QMI_DFS_FILTER_PARAM_UDP_DEST_PORT_V01 ((dfs_filter_param_error_mask_v01)0x0000000000001000ull) /**<  UDP destination port \n  */
#define QMI_DFS_FILTER_PARAM_ICMP_TYPE_V01 ((dfs_filter_param_error_mask_v01)0x0000000000002000ull) /**<  ICMP type \n  */
#define QMI_DFS_FILTER_PARAM_ICMP_CODE_V01 ((dfs_filter_param_error_mask_v01)0x0000000000004000ull) /**<  ICMP code \n  */
#define QMI_DFS_FILTER_PARAM_ESP_SPI_V01 ((dfs_filter_param_error_mask_v01)0x0000000000008000ull) /**<  Encapsulating SPI \n  */
#define QMI_DFS_FILTER_PARAM_AH_SPI_V01 ((dfs_filter_param_error_mask_v01)0x0000000000010000ull) /**<  Authentication header SPI  */
/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Queries the filter capability available. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dfs_get_filter_capability_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t max_filters_supported;
  /**<   Maximum number of filters that can be added.
   */

  uint32_t max_filters_supported_per_add;
  /**<   Maximum number of filter rules that can be specified per filter add request.
   */
}dfs_filter_capability_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Queries the filter capability available. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Maximum Media Offload Filters */
  uint8_t max_media_offload_filters_valid;  /**< Must be set to true if max_media_offload_filters is being passed */
  uint32_t max_media_offload_filters;
  /**<    Maximum number of filters that can be added for media offloading.
  */

  /* Optional */
  /*  Maximum PDN Sharing Filters */
  uint8_t max_pdn_sharing_filters_valid;  /**< Must be set to true if max_pdn_sharing_filters is being passed */
  dfs_filter_capability_type_v01 max_pdn_sharing_filters;
  /**<    \n(Maximum number of filters that can be added for PDN sharing, in total,
        and per filter add request.)
  */

  /* Optional */
  /*  Maximum Powersave Filters */
  uint8_t max_powersave_filters_valid;  /**< Must be set to true if max_powersave_filters is being passed */
  dfs_filter_capability_type_v01 max_powersave_filters;
  /**<   \n(Maximum number of filters that can be added for powersave, in total,
        and per filter add requests.)
  */

  /* Optional */
  /*  Maximum Low Latency Filters */
  uint8_t max_low_latency_filters_valid;  /**< Must be set to true if max_low_latency_filters is being passed */
  dfs_filter_capability_type_v01 max_low_latency_filters;
  /**<    \n(Maximum number of filters that can be added for low latency, in 
        total, and per filter add requests.)
  */
}dfs_get_filter_capability_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Binds a control point to a data port and IP preference. */
typedef struct {

  /* Optional */
  /*  Binding Data Port */
  uint8_t data_port_valid;  /**< Must be set to true if data_port is being passed */
  uint16_t data_port;
  /**<   The SIO data port to which the client binds.
  */

  /* Optional */
  /*  IP Family Preference */
  uint8_t ip_preference_valid;  /**< Must be set to true if ip_preference is being passed */
  dfs_ip_family_enum_v01 ip_preference;
  /**<   Values: \n
      - DFS_IP_FAMILY_IPV4 (0x04) --  IPv4\n
      - DFS_IP_FAMILY_IPV6 (0x06) --  IPv6
 */

  /* Optional */
  /*  Peripheral Endpoint ID */
  uint8_t ep_id_valid;  /**< Must be set to true if ep_id is being passed */
  data_ep_id_type_v01 ep_id;
  /**<   \n(The peripheral endpoint (physical data channel) to which
       the client binds.)
  */

  /* Optional */
  /*  Mux ID */
  uint8_t mux_id_valid;  /**< Must be set to true if mux_id is being passed */
  uint8_t mux_id;
  /**<   The mux ID of the logical data channel to which
       the client binds; default value is 0.
  */

  /* Optional */
  /*  Bind Subscription */
  uint8_t bind_subs_valid;  /**< Must be set to true if bind_subs is being passed */
  dfs_bind_subscription_enum_v01 bind_subs;
  /**<   Subscription to which to bind. Values: \n
      - DFS_DEFAULT_SUBS (0x0000) --  Default data subscription \n
      - DFS_PRIMARY_SUBS (0x0001) --  Primary\n
      - DFS_SECONDARY_SUBS (0x0002) --  Secondary\n
      - DFS_TERTIARY_SUBS (0x0003) --  Tertiary\n
      - DFS_DONT_CARE_SUBS (0x00FF) --  Default value used in the absence of
       explicit binding 
 */
}dfs_bind_client_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Binds a control point to a data port and IP preference. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}dfs_bind_client_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Gets the control point parameters. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dfs_get_client_binding_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Gets the control point parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Bound Data Port */
  uint8_t data_port_valid;  /**< Must be set to true if data_port is being passed */
  uint16_t data_port;
  /**<   The SIO data port to which the client is bound.
  */

  /* Optional */
  /*  IP Family Preference */
  uint8_t ip_preference_valid;  /**< Must be set to true if ip_preference is being passed */
  dfs_ip_family_enum_v01 ip_preference;
  /**<   Values: \n
      - DFS_IP_FAMILY_IPV4 (0x04) --  IPv4\n
      - DFS_IP_FAMILY_IPV6 (0x06) --  IPv6
 */

  /* Optional */
  /*  Bound Peripheral Endpoint ID */
  uint8_t bound_ep_id_valid;  /**< Must be set to true if bound_ep_id is being passed */
  data_ep_id_type_v01 bound_ep_id;
  /**<   \n(The peripheral endpoint (physical data channel) to which
       the client is bound.)
  */

  /* Optional */
  /*  Bound Mux ID */
  uint8_t bound_mux_id_valid;  /**< Must be set to true if bound_mux_id is being passed */
  uint8_t bound_mux_id;
  /**<   The mux ID of the logical data channel to which
       the client is bound.
  */

  /* Optional */
  /*  Bound Subscription  */
  uint8_t bound_subs_valid;  /**< Must be set to true if bound_subs is being passed */
  dfs_bind_subscription_enum_v01 bound_subs;
  /**<   Subscription to which is bound. Values: \n
      - DFS_DEFAULT_SUBS (0x0000) --  Default data subscription \n
      - DFS_PRIMARY_SUBS (0x0001) --  Primary\n
      - DFS_SECONDARY_SUBS (0x0002) --  Secondary\n
      - DFS_TERTIARY_SUBS (0x0003) --  Tertiary\n
      - DFS_DONT_CARE_SUBS (0x00FF) --  Default value used in the absence of
       explicit binding 
 */
}dfs_get_client_binding_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Adds a media offload filter. */
typedef struct {

  /* Optional */
  /*  Filter ID */
  uint8_t filter_id_valid;  /**< Must be set to true if filter_id is being passed */
  uint8_t filter_id;
  /**<   Filter ID.
   */

  /* Optional */
  /*  IPv4 Destination Address */
  uint8_t ipv4_dest_address_valid;  /**< Must be set to true if ipv4_dest_address is being passed */
  uint32_t ipv4_dest_address;
  /**<   IPv4 destination address.
   */

  /* Optional */
  /*  IPv6 Destination Address */
  uint8_t ipv6_dest_address_valid;  /**< Must be set to true if ipv6_dest_address is being passed */
  dfs_ipv6_addr_type_v01 ipv6_dest_address;
  /**<   IPv6 destination address.
   */

  /* Optional */
  /*  Transport Level Protocol */
  uint8_t xport_protocol_valid;  /**< Must be set to true if xport_protocol is being passed */
  dfs_xport_protocol_enum_v01 xport_protocol;
  /**<   Transport protocol, Values:\n
      - DFS_PROTO_UDP (0x11) --  User Datagram Protocol\n
 */

  /* Optional */
  /*  UDP Destination Port */
  uint8_t udp_dest_port_valid;  /**< Must be set to true if udp_dest_port is being passed */
  uint16_t udp_dest_port;
  /**<   Destination port for UDP.
   */
}dfs_add_media_offload_filter_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Adds a media offload filter. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */

  /* Optional */
  /*  Filter Handle */
  uint8_t filter_handle_valid;  /**< Must be set to true if filter_handle is being passed */
  uint32_t filter_handle;
  /**<   Handle to the filter that was added. */
}dfs_add_media_offload_filter_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Removes a media offload filter that has been previously added. */
typedef struct {

  /* Mandatory */
  /*  Filter Handle */
  uint32_t filter_handle;
  /**<   Handle to the filter to be removed. */
}dfs_remove_media_offload_filter_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Removes a media offload filter that has been previously added. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */
}dfs_remove_media_offload_filter_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Queries the statistics for a filter handle for the media offload client. */
typedef struct {

  /* Mandatory */
  /*  Filter Handle */
  uint32_t filter_handle;
  /**<   Handle to the filter for which statistics information is required. */
}dfs_get_media_offload_statistics_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Queries the statistics for a filter handle for the media offload client. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Bytes Sent */
  uint8_t bytes_sent_valid;  /**< Must be set to true if bytes_sent is being passed */
  uint64_t bytes_sent;
  /**<   Bytes sent.
   */

  /* Optional */
  /*  Bytes Received */
  uint8_t bytes_received_valid;  /**< Must be set to true if bytes_received is being passed */
  uint64_t bytes_received;
  /**<   Bytes received.
   */

  /* Optional */
  /*  Packets Sent */
  uint8_t packets_sent_valid;  /**< Must be set to true if packets_sent is being passed */
  uint32_t packets_sent;
  /**<   Packets sent.
   */

  /* Optional */
  /*  Packets Received */
  uint8_t packets_received_valid;  /**< Must be set to true if packets_received is being passed */
  uint32_t packets_received;
  /**<   Packets received.
     */
}dfs_get_media_offload_statistics_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Adds a list of filters for PDN sharing. */
typedef struct {

  /* Optional */
  /*  Filter Rules */
  uint8_t filter_rules_valid;  /**< Must be set to true if filter_rules is being passed */
  uint32_t filter_rules_len;  /**< Must be set to # of elements in filter_rules */
  dfs_filter_rule_type_v01 filter_rules[QMI_DFS_MAX_FILTERS_V01];
  /**<   \n (List of filter rules.)
   */
}dfs_add_pdn_sharing_filters_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Adds a list of filters for PDN sharing. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */

  /* Optional */
  /*  Filter Handles */
  uint8_t filter_handles_valid;  /**< Must be set to true if filter_handles is being passed */
  uint32_t filter_handles_len;  /**< Must be set to # of elements in filter_handles */
  uint32_t filter_handles[QMI_DFS_MAX_FILTERS_V01];
  /**<   List of handles that uniquely identifies filter rules that have been
       added. This TLV is returned when there is no error.
   */

  /* Optional */
  /*  Filter Rule Error */
  uint8_t filter_rule_error_valid;  /**< Must be set to true if filter_rule_error is being passed */
  uint32_t filter_rule_error_len;  /**< Must be set to # of elements in filter_rule_error */
  dfs_filter_param_error_mask_v01 filter_rule_error[QMI_DFS_MAX_FILTERS_V01];
  /**<   List of error masks that contain errors for each filter rule. This TLV
 is returned when the error is QMI_ERR_ INVALID_ARG. Values:\n
      - QMI_DFS_FILTER_PARAM_NONE (0x0000000000000000) --  No errors \n
      - QMI_DFS_FILTER_PARAM_IP_VERSION (0x0000000000000001) --  IP version \n
      - QMI_DFS_FILTER_PARAM_IPV4_SRC_ADDR (0x0000000000000002) --  IPv4 source address \n
      - QMI_DFS_FILTER_PARAM_IPV4_DEST_ADDR (0x0000000000000004) --  IPv4 destination address \n
      - QMI_DFS_FILTER_PARAM_IPV4_TOS (0x0000000000000008) --  IPv4 type of service \n
      - QMI_DFS_FILTER_PARAM_IPV6_SRC_ADDR (0x0000000000000010) --  IPv6 source address \n
      - QMI_DFS_FILTER_PARAM_IPV6_DEST_ADDR (0x0000000000000020) --  IPv6 destination address \n
      - QMI_DFS_FILTER_PARAM_IPV6_TRF_CLS (0x0000000000000040) --  IPv6 traffic class \n
      - QMI_DFS_FILTER_PARAM_IPV6_FLOW_LABEL (0x0000000000000080) --  IPv6 flow label \n
      - QMI_DFS_FILTER_PARAM_XPORT_PROT (0x0000000000000100) --  Transport protocol \n
      - QMI_DFS_FILTER_PARAM_TCP_SRC_PORT (0x0000000000000200) --  TCP source port \n
      - QMI_DFS_FILTER_PARAM_TCP_DEST_PORT (0x0000000000000400) --  TCP destination port \n
      - QMI_DFS_FILTER_PARAM_UDP_SRC_PORT (0x0000000000000800) --  UDP source port \n
      - QMI_DFS_FILTER_PARAM_UDP_DEST_PORT (0x0000000000001000) --  UDP destination port \n
      - QMI_DFS_FILTER_PARAM_ICMP_TYPE (0x0000000000002000) --  ICMP type \n
      - QMI_DFS_FILTER_PARAM_ICMP_CODE (0x0000000000004000) --  ICMP code \n
      - QMI_DFS_FILTER_PARAM_ESP_SPI (0x0000000000008000) --  Encapsulating SPI \n
      - QMI_DFS_FILTER_PARAM_AH_SPI (0x0000000000010000) --  Authentication header SPI
 */
}dfs_add_pdn_sharing_filters_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Removes all filters associated with previously added handles. */
typedef struct {

  /* Mandatory */
  /*  Filter Handles */
  uint32_t filter_handles_len;  /**< Must be set to # of elements in filter_handles */
  uint32_t filter_handles[QMI_DFS_MAX_FILTERS_V01];
  /**<   List of handles to the filter rules to remove. */
}dfs_remove_filters_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Removes all filters associated with previously added handles. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */
}dfs_remove_filters_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Adds a list of filters for powersave. */
typedef struct {

  /* Optional */
  /*  Filter Rules */
  uint8_t filter_rules_valid;  /**< Must be set to true if filter_rules is being passed */
  uint32_t filter_rules_len;  /**< Must be set to # of elements in filter_rules */
  dfs_filter_rule_type_v01 filter_rules[QMI_DFS_MAX_FILTERS_V01];
  /**<   \n (List of filter rules.) */
}dfs_add_powersave_filters_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Adds a list of filters for powersave. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */

  /* Optional */
  /*  Filter Handles */
  uint8_t filter_handles_valid;  /**< Must be set to true if filter_handles is being passed */
  uint32_t filter_handles_len;  /**< Must be set to # of elements in filter_handles */
  uint32_t filter_handles[QMI_DFS_MAX_FILTERS_V01];
  /**<   List of handles that uniquely identifies filter rules that have been added.
       This TLV is returned when there is no error.
   */

  /* Optional */
  /*  Filter Rule Error */
  uint8_t filter_rule_error_valid;  /**< Must be set to true if filter_rule_error is being passed */
  uint32_t filter_rule_error_len;  /**< Must be set to # of elements in filter_rule_error */
  dfs_filter_param_error_mask_v01 filter_rule_error[QMI_DFS_MAX_FILTERS_V01];
  /**<   List of error masks that contain errors for each filter rule. This TLV
 is returned when the error is QMI_ERR_INVALID_ARG. Values:\n
      - QMI_DFS_FILTER_PARAM_NONE (0x0000000000000000) --  No errors \n
      - QMI_DFS_FILTER_PARAM_IP_VERSION (0x0000000000000001) --  IP version \n
      - QMI_DFS_FILTER_PARAM_IPV4_SRC_ADDR (0x0000000000000002) --  IPv4 source address \n
      - QMI_DFS_FILTER_PARAM_IPV4_DEST_ADDR (0x0000000000000004) --  IPv4 destination address \n
      - QMI_DFS_FILTER_PARAM_IPV4_TOS (0x0000000000000008) --  IPv4 type of service \n
      - QMI_DFS_FILTER_PARAM_IPV6_SRC_ADDR (0x0000000000000010) --  IPv6 source address \n
      - QMI_DFS_FILTER_PARAM_IPV6_DEST_ADDR (0x0000000000000020) --  IPv6 destination address \n
      - QMI_DFS_FILTER_PARAM_IPV6_TRF_CLS (0x0000000000000040) --  IPv6 traffic class \n
      - QMI_DFS_FILTER_PARAM_IPV6_FLOW_LABEL (0x0000000000000080) --  IPv6 flow label \n
      - QMI_DFS_FILTER_PARAM_XPORT_PROT (0x0000000000000100) --  Transport protocol \n
      - QMI_DFS_FILTER_PARAM_TCP_SRC_PORT (0x0000000000000200) --  TCP source port \n
      - QMI_DFS_FILTER_PARAM_TCP_DEST_PORT (0x0000000000000400) --  TCP destination port \n
      - QMI_DFS_FILTER_PARAM_UDP_SRC_PORT (0x0000000000000800) --  UDP source port \n
      - QMI_DFS_FILTER_PARAM_UDP_DEST_PORT (0x0000000000001000) --  UDP destination port \n
      - QMI_DFS_FILTER_PARAM_ICMP_TYPE (0x0000000000002000) --  ICMP type \n
      - QMI_DFS_FILTER_PARAM_ICMP_CODE (0x0000000000004000) --  ICMP code \n
      - QMI_DFS_FILTER_PARAM_ESP_SPI (0x0000000000008000) --  Encapsulating SPI \n
      - QMI_DFS_FILTER_PARAM_AH_SPI (0x0000000000010000) --  Authentication header SPI
 */
}dfs_add_powersave_filters_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Enables or disables powersave filtering. */
typedef struct {

  /* Mandatory */
  /*  Powersave Filter Mode */
  uint8_t powersave_filter_mode;
  /**<   Values: \n
       - 0 -- Disable powersave filtering \n
       - 1 -- Enable powersave filtering
  */
}dfs_set_powersave_filter_mode_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Enables or disables powersave filtering. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */
}dfs_set_powersave_filter_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Queries the current powersave filtering mode. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dfs_get_powersave_filter_mode_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Queries the current powersave filtering mode. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */

  /* Optional */
  /*  Powersave Filter Mode */
  uint8_t powersave_filter_mode_valid;  /**< Must be set to true if powersave_filter_mode is being passed */
  uint8_t powersave_filter_mode;
  /**<   Values: \n
       - 0 -- Powersave filtering is disabled \n
       - 1 -- Powersave filtering is enabled
  */
}dfs_get_powersave_filter_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Enables or disables the automatic exiting of powersave filtering. */
typedef struct {

  /* Mandatory */
  /*  Autoexit Powersave Filter Mode */
  uint8_t autoexit_powersave_filter_mode;
  /**<   Values: \n
       - 0 -- Do not autoexit powersave filtering (default value) \n
       - 1 -- Autoexit powersave filtering
  */
}dfs_set_autoexit_powersave_filter_mode_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Enables or disables the automatic exiting of powersave filtering. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */
}dfs_set_autoexit_powersave_filter_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Sets the registration state for different QMI_DFS indications
           for the requesting control point. */
typedef struct {

  /* Optional */
  /*  Powersave Filter Mode Change */
  uint8_t report_powersave_filter_mode_change_valid;  /**< Must be set to true if report_powersave_filter_mode_change is being passed */
  uint8_t report_powersave_filter_mode_change;
  /**<   Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report powersave filter mode change
  */

  /* Optional */
  /*  Low Latency Traffic */
  uint8_t report_low_latency_traffic_valid;  /**< Must be set to true if report_low_latency_traffic is being passed */
  uint8_t report_low_latency_traffic;
  /**<   Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report low latency traffic
  */

  /* Optional */
  /*  Report Reverse IP Transport Filters Update */
  uint8_t report_reverse_ip_transport_filters_update_valid;  /**< Must be set to true if report_reverse_ip_transport_filters_update is being passed */
  uint8_t report_reverse_ip_transport_filters_update;
  /**<   Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report reverse IP transport filters update
  */

  /* Optional */
  /*  Remote Socket Handling */
  uint8_t remote_socket_handling_valid;  /**< Must be set to true if remote_socket_handling is being passed */
  uint8_t remote_socket_handling;
  /**<   Values:  \n
       - 0 -- Do not report \n
       - 1 -- Report remote socket handling related indications
  */
}dfs_indication_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Sets the registration state for different QMI_DFS indications
           for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}dfs_indication_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Indication Message; Indicates that the powersave filtering mode for the PDN has changed. */
typedef struct {

  /* Mandatory */
  /*  Powersave Filter Mode */
  uint8_t powersave_filter_mode;
  /**<   Values: \n
        - 0 -- Powersave filtering is disabled \n
        - 1 -- Powersave filtering is enabled
   */
}dfs_powersave_filter_mode_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Removes all previously added filter rules for powersave filtering. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dfs_remove_all_powersave_filters_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Removes all previously added filter rules for powersave filtering. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */
}dfs_remove_all_powersave_filters_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_enums
    @{
  */
typedef enum {
  DFS_LOW_LATENCY_FILTER_DIRECTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DFS_LOW_LATENCY_FILTER_DIRECTION_DOWNLINK_V01 = 0x00, /**<  Low latency traffic detection filter to be installed for the downlink 
       data direction  */
  DFS_LOW_LATENCY_FILTER_DIRECTION_UPLINK_V01 = 0x01, /**<  Low latency traffic detection filter to be installed for the uplink 
       data direction  */
  DFS_LOW_LATENCY_FILTER_DIRECTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dfs_low_latency_filter_direction_enum_v01;
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Adds a list of filters for low latency traffic detection. */
typedef struct {

  /* Optional */
  /*  Filter Rules */
  uint8_t filter_rules_valid;  /**< Must be set to true if filter_rules is being passed */
  uint32_t filter_rules_len;  /**< Must be set to # of elements in filter_rules */
  dfs_filter_rule_type_v01 filter_rules[QMI_DFS_MAX_FILTERS_V01];
  /**<   \n (List of filter rules.) */

  /* Optional */
  /*  Packet Inter-arrival Time */
  uint8_t pkt_inter_arrival_time_valid;  /**< Must be set to true if pkt_inter_arrival_time is being passed */
  uint32_t pkt_inter_arrival_time_len;  /**< Must be set to # of elements in pkt_inter_arrival_time */
  uint32_t pkt_inter_arrival_time[QMI_DFS_MAX_FILTERS_V01];
  /**<   Inter-arrival time, in milliseconds, between successive packets for each 
       filter rule. This TLV determines the duration of the timer that is run 
       on the device to detect the end of such packet traffic.
   */

  /* Optional */
  /*  Filter Direction */
  uint8_t filter_direction_valid;  /**< Must be set to true if filter_direction is being passed */
  dfs_low_latency_filter_direction_enum_v01 filter_direction;
  /**<   Data traffic direction in which the filter rules in the Filter Rules
 TLV must be installed. The default value is 
 DFS_LOW_LATENCY_FILTER_DIRECTION_UPLINK. Values : \n
      - DFS_LOW_LATENCY_FILTER_DIRECTION_DOWNLINK (0x00) --  Low latency traffic detection filter to be installed for the downlink 
       data direction
      - DFS_LOW_LATENCY_FILTER_DIRECTION_UPLINK (0x01) --  Low latency traffic detection filter to be installed for the uplink 
       data direction
 */
}dfs_add_low_latency_filters_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Adds a list of filters for low latency traffic detection. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */

  /* Optional */
  /*  Filter Handles */
  uint8_t filter_handles_valid;  /**< Must be set to true if filter_handles is being passed */
  uint32_t filter_handles_len;  /**< Must be set to # of elements in filter_handles */
  uint32_t filter_handles[QMI_DFS_MAX_FILTERS_V01];
  /**<   List of handles to the filter rules that have been added. 
       This TLV is returned when there is no error.
   */

  /* Optional */
  /*  Filter Rule Error */
  uint8_t filter_rule_error_valid;  /**< Must be set to true if filter_rule_error is being passed */
  uint32_t filter_rule_error_len;  /**< Must be set to # of elements in filter_rule_error */
  dfs_filter_param_error_mask_v01 filter_rule_error[QMI_DFS_MAX_FILTERS_V01];
  /**<   List of error masks that contain errors for each filter rule. This TLV
       is returned when the error is QMI_ERR_INVALID_ARG.
   */
}dfs_add_low_latency_filters_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Indication Message; Informs the Terminal Equipment (TE) about the start 
           or stop of low latency traffic. */
typedef struct {

  /* Mandatory */
  /*  Traffic Start */
  uint8_t traffic_start;
  /**<   Indicates whether the low latency traffic has started or has
       ended as detected on the device. Values : \n
       - 0 -- Low latency traffic stop \n
       - 1 -- Low latency traffic start
  */

  /* Mandatory */
  /*  Filter Handle */
  uint32_t filter_handle;
  /**<   Indicates which specific filter in the list of low latency filters was hit.
       This is sent for both the start and end of the low latency traffic.
  */
}dfs_low_latency_traffic_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Retrieves a list of filters for the reverse IP transport connection. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dfs_get_reverse_ip_transport_filters_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Retrieves a list of filters for the reverse IP transport connection. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */

  /* Optional */
  /*  Filter Rules */
  uint8_t filter_rules_valid;  /**< Must be set to true if filter_rules is being passed */
  uint32_t filter_rules_len;  /**< Must be set to # of elements in filter_rules */
  dfs_filter_rule_type_v01 filter_rules[QMI_DFS_MAX_FILTERS_V01];
  /**<   \n (List of filter rules.) */
}dfs_get_reverse_ip_transport_filters_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_enums
    @{
  */
typedef enum {
  DFS_REVERSE_IP_TRANSPORT_FILTERS_ACTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DFS_REVERSE_IP_TRANSPORT_FILTERS_ADDED_V01 = 0, /**<  Reverse IP Transport filters were added  */
  DFS_REVERSE_IP_TRANSPORT_FILTERS_DELETED_V01 = 1, /**<  Reverse IP Transport filters were deleted  */
  DFS_REVERSE_IP_TRANSPORT_FILTERS_ACTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dfs_reverse_ip_transport_filters_action_enum_v01;
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Indication Message; Informs the TE from about changes in 
           the reverse IP transport connection filters. */
typedef struct {

  /* Mandatory */
  /*  Filter Action */
  dfs_reverse_ip_transport_filters_action_enum_v01 filter_action;
  /**<   Indicates whether the filters are to be added or deleted. Values: \n
      - DFS_REVERSE_IP_TRANSPORT_FILTERS_ADDED (0) --  Reverse IP Transport filters were added 
      - DFS_REVERSE_IP_TRANSPORT_FILTERS_DELETED (1) --  Reverse IP Transport filters were deleted 
 */

  /* Optional */
  /*  Filter Rules */
  uint8_t filter_rules_valid;  /**< Must be set to true if filter_rules is being passed */
  uint32_t filter_rules_len;  /**< Must be set to # of elements in filter_rules */
  dfs_filter_rule_type_v01 filter_rules[QMI_DFS_MAX_FILTERS_V01];
  /**<   \n (List of filter rules.) */
}dfs_reverse_ip_transport_filters_updated_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  dfs_ip_family_enum_v01 ip_family;
  /**<   IP Family of the socket. Values :\n
      - DFS_IP_FAMILY_IPV4 (0x04) --  IPv4\n 
      - DFS_IP_FAMILY_IPV6 (0x06) --  IPv6 
 */

  dfs_xport_protocol_enum_v01 xport_prot;
  /**<   Transport protocol of the socket. Values :\n
      - DFS_PROTO_TCP (0x06) --  Transmission Control Protocol\n 
      - DFS_PROTO_UDP (0x11) --  User Datagram Protocol\n  
 */

  uint16_t port_no;
  /**<   Port number. Will be 0 if specific port number is not needed
  */
}dfs_request_socket_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Indication Message; Informs the TE that one or more sockets need to be allocated. */
typedef struct {

  /* Mandatory */
  /*  Request Socket List */
  uint32_t request_socket_list_len;  /**< Must be set to # of elements in request_socket_list */
  dfs_request_socket_info_type_v01 request_socket_list[QMI_DFS_MAX_ALLOCATED_SOCKETS_V01];
  /**<   List of sockets with qualifying attributes that are needed by the modem. 
  */
}dfs_remote_socket_request_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_enums
    @{
  */
typedef enum {
  DFS_REMOTE_SOCKET_ALLOC_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  DFS_REMOTE_SOCKET_ALLOC_SUCCESS_V01 = 0x0000, /**<  Allocation successful \n  */
  DFS_REMOTE_SOCKET_ALLOC_IN_USE_FAILURE_V01 = 0x0001, /**<  Requested port is in use \n  */
  DFS_REMOTE_SOCKET_ALLOC_GENERAL_FAILURE_V01 = 0x0002, /**<  Other failure seen during remote socket allocation \n  */
  DFS_REMOTE_SOCKET_ALLOC_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}dfs_remote_socket_alloc_status_enum_v01;
/**
    @}
  */

/** @addtogroup dfs_qmi_aggregates
    @{
  */
typedef struct {

  dfs_remote_socket_alloc_status_enum_v01 status;
  /**<   Status while processing the socket allocation. If it is
       DFS_REMOTE_SOCKET_ALLOC_SUCCESS, the socket_handle and socket_info
       will be valid.
  */

  uint32_t socket_handle;
  /**<   Handle to the socket created on the TE. Uniquely identifies the tuple of 
  ip_family, xport_prot and port_no. 
  */

  uint8_t is_ephemeral;
  /**<   Whether request from modem was for ephemeral port or specific port. 
       Values:\n
       - 0 -- Is not ephemeral port \n
       - 1 -- Is ephemeral port
  */

  dfs_request_socket_info_type_v01 socket_info;
  /**<   Qualifying attributes of the socket. port_no will contain a non zero value.
  */
}dfs_allocated_socket_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Sends a list of allocated sockets with other qualifying information. */
typedef struct {

  /* Mandatory */
  /*  Socket List */
  uint32_t socket_list_len;  /**< Must be set to # of elements in socket_list */
  dfs_allocated_socket_info_type_v01 socket_list[QMI_DFS_MAX_ALLOCATED_SOCKETS_V01];
  /**<   List of sockets allocated on the TE for use by the modem.
    */
}dfs_remote_socket_allocated_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Sends a list of allocated sockets with other qualifying information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */
}dfs_remote_socket_allocated_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Indication Message; Informs the TE that one or many sockets need to be released. */
typedef struct {

  /* Optional */
  /*  Socket Handles */
  uint8_t socket_handles_valid;  /**< Must be set to true if socket_handles is being passed */
  uint32_t socket_handles_len;  /**< Must be set to # of elements in socket_handles */
  uint32_t socket_handles[QMI_DFS_MAX_ALLOCATED_SOCKETS_V01];
  /**<   List of sockets identified by socket handles that need to be released. 
       If not specified, all socket handles allocated by the TE for the modem
       can be released.
  */
}dfs_remote_socket_release_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Indication Message; Informs the TE about options to be set for an allocated socket. */
typedef struct {

  /* Mandatory */
  /*  Socket Handle */
  uint32_t socket_handle;
  /**<   Socket handle identifying a previously allocated socket. 
  */

  /* Optional */
  /*  Is UDP Encapsulated */
  uint8_t is_udp_encaps_valid;  /**< Must be set to true if is_udp_encaps is being passed */
  uint8_t is_udp_encaps;
  /**<   Values : \n. 
       - 0 -- Socket is not UDP Encapsulated \n
       - 1 -- Socket is UDP Encapsulated
  */
}dfs_remote_socket_set_option_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Request Message; Queries the capability available. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}dfs_get_capability_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup dfs_qmi_messages
    @{
  */
/** Response Message; Queries the capability available. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Remote Socket Capability */
  uint8_t remote_socket_capability_valid;  /**< Must be set to true if remote_socket_capability is being passed */
  uint8_t remote_socket_capability;
  /**<    Values : \n.
        - 0 -- FALSE = Remote Socket Capability not supported \n
        - 1 -- TRUE = Remote Socket Capability supported
  */
}dfs_get_capability_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */
//#define REMOVE_QMI_DFS_ADD_LOW_LATENCY_FILTERS_V01
//#define REMOVE_QMI_DFS_ADD_MEDIA_OFFLOAD_FILTER_V01
//#define REMOVE_QMI_DFS_ADD_PDN_SHARING_FILTERS_V01
//#define REMOVE_QMI_DFS_ADD_POWERSAVE_FILTERS_V01
//#define REMOVE_QMI_DFS_BIND_CLIENT_V01
//#define REMOVE_QMI_DFS_GET_CAPABILITY_V01 
//#define REMOVE_QMI_DFS_GET_CLIENT_BINDING_V01
//#define REMOVE_QMI_DFS_GET_FILTER_CAPABILITY_V01
//#define REMOVE_QMI_DFS_GET_MEDIA_OFFLOAD_STATISTICS_V01
//#define REMOVE_QMI_DFS_GET_POWERSAVE_FILTER_MODE_V01
//#define REMOVE_QMI_DFS_GET_REVERSE_IP_TRANSPORT_FILTERS_V01
//#define REMOVE_QMI_DFS_INDICATION_REGISTER_V01
//#define REMOVE_QMI_DFS_LOW_LATENCY_TRAFFIC_IND_V01
//#define REMOVE_QMI_DFS_POWERSAVE_FILTER_MODE_IND_V01
//#define REMOVE_QMI_DFS_REMOTE_SOCKET_ALLOCATED_V01 
//#define REMOVE_QMI_DFS_REMOTE_SOCKET_RELEASE_IND_V01 
//#define REMOVE_QMI_DFS_REMOTE_SOCKET_REQUEST_IND_V01 
//#define REMOVE_QMI_DFS_REMOTE_SOCKET_SET_OPTION_IND_V01 
//#define REMOVE_QMI_DFS_REMOVE_ALL_POWERSAVE_FILTERS_V01
//#define REMOVE_QMI_DFS_REMOVE_FILTERS_V01
//#define REMOVE_QMI_DFS_REMOVE_MEDIA_OFFLOAD_FILTER_V01
//#define REMOVE_QMI_DFS_REVERSE_IP_TRANSPORT_FILTERS_UPDATED_IND_V01
//#define REMOVE_QMI_DFS_SET_AUTOEXIT_POWERSAVE_FILTER_MODE_V01
//#define REMOVE_QMI_DFS_SET_POWERSAVE_FILTER_MODE_V01

/*Service Message Definition*/
/** @addtogroup dfs_qmi_msg_ids
    @{
  */
#define QMI_DFS_INDICATION_REGISTER_REQ_V01 0x0003
#define QMI_DFS_INDICATION_REGISTER_RESP_V01 0x0003
#define QMI_DFS_GET_FILTER_CAPABILITY_REQ_V01 0x0020
#define QMI_DFS_GET_FILTER_CAPABILITY_RESP_V01 0x0020
#define QMI_DFS_BIND_CLIENT_REQ_V01 0x0021
#define QMI_DFS_BIND_CLIENT_RESP_V01 0x0021
#define QMI_DFS_GET_CLIENT_BINDING_REQ_V01 0x0022
#define QMI_DFS_GET_CLIENT_BINDING_RESP_V01 0x0022
#define QMI_DFS_ADD_MEDIA_OFFLOAD_FILTER_REQ_V01 0x0023
#define QMI_DFS_ADD_MEDIA_OFFLOAD_FILTER_RESP_V01 0x0023
#define QMI_DFS_REMOVE_MEDIA_OFFLOAD_FILTER_REQ_V01 0x0024
#define QMI_DFS_REMOVE_MEDIA_OFFLOAD_FILTER_RESP_V01 0x0024
#define QMI_DFS_GET_MEDIA_OFFLOAD_STATISTICS_REQ_V01 0x0025
#define QMI_DFS_GET_MEDIA_OFFLOAD_STATISTICS_RESP_V01 0x0025
#define QMI_DFS_ADD_PDN_SHARING_FILTERS_REQ_V01 0x0026
#define QMI_DFS_ADD_PDN_SHARING_FILTERS_RESP_V01 0x0026
#define QMI_DFS_REMOVE_FILTERS_REQ_V01 0x0027
#define QMI_DFS_REMOVE_FILTERS_RESP_V01 0x0027
#define QMI_DFS_ADD_POWERSAVE_FILTERS_REQ_V01 0x0028
#define QMI_DFS_ADD_POWERSAVE_FILTERS_RESP_V01 0x0028
#define QMI_DFS_SET_POWERSAVE_FILTER_MODE_REQ_V01 0x0029
#define QMI_DFS_SET_POWERSAVE_FILTER_MODE_RESP_V01 0x0029
#define QMI_DFS_GET_POWERSAVE_FILTER_MODE_REQ_V01 0x002A
#define QMI_DFS_GET_POWERSAVE_FILTER_MODE_RESP_V01 0x002A
#define QMI_DFS_SET_AUTOEXIT_POWERSAVE_FILTER_MODE_REQ_V01 0x002B
#define QMI_DFS_SET_AUTOEXIT_POWERSAVE_FILTER_MODE_RESP_V01 0x002B
#define QMI_DFS_POWERSAVE_FILTER_MODE_IND_V01 0x002C
#define QMI_DFS_REMOVE_ALL_POWERSAVE_FILTERS_REQ_V01 0x002D
#define QMI_DFS_REMOVE_ALL_POWERSAVE_FILTERS_RESP_V01 0x002D
#define QMI_DFS_ADD_LOW_LATENCY_FILTERS_REQ_V01 0x002E
#define QMI_DFS_ADD_LOW_LATENCY_FILTERS_RESP_V01 0x002E
#define QMI_DFS_LOW_LATENCY_TRAFFIC_IND_V01 0x002F
#define QMI_DFS_GET_REVERSE_IP_TRANSPORT_FILTERS_REQ_V01 0x0030
#define QMI_DFS_GET_REVERSE_IP_TRANSPORT_FILTERS_RESP_V01 0x0030
#define QMI_DFS_REVERSE_IP_TRANSPORT_FILTERS_UPDATED_IND_V01 0x0031
#define QMI_DFS_REMOTE_SOCKET_REQUEST_IND_V01 0x0032
#define QMI_DFS_REMOTE_SOCKET_ALLOCATED_REQ_V01 0x0033
#define QMI_DFS_REMOTE_SOCKET_ALLOCATED_RESP_V01 0x0033
#define QMI_DFS_REMOTE_SOCKET_RELEASE_IND_V01 0x0034
#define QMI_DFS_REMOTE_SOCKET_SET_OPTION_IND_V01 0x0035
#define QMI_DFS_GET_CAPABILITY_REQ_V01 0x0036
#define QMI_DFS_GET_CAPABILITY_RESP_V01 0x0036
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro dfs_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type dfs_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define dfs_get_service_object_v01( ) \
          dfs_get_service_object_internal_v01( \
            DFS_V01_IDL_MAJOR_VERS, DFS_V01_IDL_MINOR_VERS, \
            DFS_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

