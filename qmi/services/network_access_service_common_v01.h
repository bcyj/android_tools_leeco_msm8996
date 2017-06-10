#ifndef NETWORK_ACCESS_SERVICE_COMMON_SERVICE_01_H
#define NETWORK_ACCESS_SERVICE_COMMON_SERVICE_01_H
/**
  @file network_access_service_common_v01.h

  @brief This is the public header file which defines the network_access_service_common service Data structures.

  This header file defines the types and structures that were defined in
  network_access_service_common. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2013-2015 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13 
   It was generated on: Wed Oct 22 2014 (Spin 0)
   From IDL File: network_access_service_common_v01.idl */

/** @defgroup network_access_service_common_qmi_consts Constant values defined in the IDL */
/** @defgroup network_access_service_common_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup network_access_service_common_qmi_enums Enumerated types used in QMI messages */
/** @defgroup network_access_service_common_qmi_messages Structures sent as QMI messages */
/** @defgroup network_access_service_common_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup network_access_service_common_qmi_accessor Accessor for QMI service object */
/** @defgroup network_access_service_common_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup network_access_service_common_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define NETWORK_ACCESS_SERVICE_COMMON_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define NETWORK_ACCESS_SERVICE_COMMON_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define NETWORK_ACCESS_SERVICE_COMMON_V01_IDL_TOOL_VERS 0x06

/**
    @}
  */

/** @addtogroup network_access_service_common_qmi_enums
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

/** @addtogroup network_access_service_common_qmi_enums
    @{
  */
typedef enum {
  NAS_RADIO_IF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_RADIO_IF_NO_SVC_V01 = 0x00, /**<  None (no service) \n  */
  NAS_RADIO_IF_CDMA_1X_V01 = 0x01, /**<  cdma2000\textsuperscript{\textregistered} 1X \n  */
  NAS_RADIO_IF_CDMA_1XEVDO_V01 = 0x02, /**<  cdma2000\textsuperscript{\textregistered} HRPD (1xEV-DO) \n  */
  NAS_RADIO_IF_AMPS_V01 = 0x03, /**<  AMPS \n  */
  NAS_RADIO_IF_GSM_V01 = 0x04, /**<  GSM \n  */
  NAS_RADIO_IF_UMTS_V01 = 0x05, /**<  UMTS \n  */
  NAS_RADIO_IF_WLAN_V01 = 0x06, /**<  WLAN \n  */
  NAS_RADIO_IF_GPS_V01 = 0x07, /**<  GPS \n  */
  NAS_RADIO_IF_LTE_V01 = 0x08, /**<  LTE \n  */
  NAS_RADIO_IF_TDSCDMA_V01 = 0x09, /**<  TD-SCDMA \n  */
  NAS_RADIO_IF_NO_CHANGE_V01 = -1, /**<  No change  */
  NAS_RADIO_IF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_radio_if_enum_v01;
/**
    @}
  */

/* Conditional compilation tags for message removal */ 

/*Extern Definition of Type Table Object*/
/*THIS IS AN INTERNAL OBJECT AND SHOULD ONLY*/
/*BE ACCESSED BY AUTOGENERATED FILES*/
extern const qmi_idl_type_table_object network_access_service_common_qmi_idl_type_table_object_v01;


#ifdef __cplusplus
}
#endif
#endif

