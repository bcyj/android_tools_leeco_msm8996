#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H
/**
  @file time_service_v01.h
  
  @brief This is the public header file which defines the time service Data structures.

  This header file defines the types and structures that were defined in 
  time. It contains the constant values defined, enums, structures,
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

  $Header: //source/qcom/qct/core/pkg/mpss/rel/1.0/modem_proc/core/services/time_qmi/src/time_service_v01.h#2 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 2.8
   It was generated on: Wed Sep 28 2011
   From IDL File: time_service_v01.idl */

/** @defgroup time_qmi_consts Constant values defined in the IDL */
/** @defgroup time_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup time_qmi_enums Enumerated types used in QMI messages */
/** @defgroup time_qmi_messages Structures sent as QMI messages */
/** @defgroup time_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup time_qmi_accessor Accessor for QMI service object */
/** @defgroup time_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup time_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define TIME_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define TIME_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define TIME_V01_IDL_TOOL_VERS 0x02
/** Maximum Defined Message ID */
#define TIME_V01_MAX_MESSAGE_ID 0x0031;
/** 
    @} 
  */


/** @addtogroup time_qmi_consts 
    @{ 
  */
/**
    @}
  */

/** @addtogroup time_qmi_enums
    @{
  */
typedef enum {
  TIME_QMI_TIME_BASES_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_TIME_BASE_RTC_V01 = 0, /**<  Real time clock timebase. */
  QMI_TIME_BASE_TOD_V01 = 1, /**<  Proxy base for number of bases. */
  QMI_TIME_BASE_USER_V01 = 2, /**<  User timebase.  */
  QMI_TIME_BASE_SECURE_V01 = 3, /**<  Secure timebase.  */
  QMI_TIME_BASE_DRM_V01 = 4, /**<  Digital rights management timebase.  */
  QMI_TIME_BASE_USER_UTC_V01 = 5, /**<  Universal Time Coordinated user timebase.     */
  QMI_TIME_BASE_USER_TZ_DL_V01 = 6, /**<  Global time zone user timebase.  */
  QMI_TIME_BASE_GPS_V01 = 7, /**<  Base for GPS time. \n
                   @note1hang When QMI_TIME_BASE_GSTK is modified, changes are also 
                   reflected on QMI_TIME_BASE_TOD.  */
  QMI_TIME_BASE_1X_V01 = 8, /**<  Base for 1X time. \n
                   @note1hang When QMI_TIME_BASE_1X is modified, changes are also 
                   reflected on QMI_TIME_BASE_TOD.  */
  QMI_TIME_BASE_HDR_V01 = 9, /**<  Base for HDR time. \n
                   @note1hang When QMI_TIME_BASE_HDR is modified, changes are also 
                   reflected on QMI_TIME_BASE_TOD.  */
  QMI_TIME_BASE_WCDMA_V01 = 10, /**<  Base for WCDMA time. \n
                   @note1hang When QMI_TIME_BASE_WCDMA is modified, changes are also 
                   reflected on QMI_TIME_BASE_TOD.  */
  QMI_TIME_BASE_MFLO_V01 = 11, /**<  Base for MediaFLO time. \n
                   @note1hang When QMI_TIME_BASE_MFLO is modified, changes are also 
                   reflected on QMI_TIME_BASE_TOD.  */
  QMI_TIME_BASE_INVALID_V01 = 0x10000000, 
  TIME_QMI_TIME_BASES_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}time_qmi_time_bases_type_v01;
/**
    @}
  */

/** @addtogroup time_qmi_messages
    @{
  */
/** Request Message; This sets the generic offset specified by the base 
              using TIME service.  */
typedef struct {

  /* Mandatory */
  /*  Base of which the offset is to be changed */
  time_qmi_time_bases_type_v01 base;
  /**<   - 0 - RTC
       - 1 - TOD
       - 2 - 1X
       - 3 - HDR
       - 4 - GPS
     */

  /* Mandatory */
  /*  Offset value to be set on the remote proc */
  uint64_t generic_offset;
  /**<   Current time = RTC value at bootup + generic offset + Uptime. 
    Therefore generic offset = 
    Number of msec elapsed through 01/06/1980 - RTC offset at bootup - uptime          
     */
}time_set_genoff_offset_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup time_qmi_messages
    @{
  */
/** Response Message; This sets the generic offset specified by the base 
              using TIME service.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}time_set_genoff_offset_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup time_qmi_messages
    @{
  */
/** Request Message; This gets the TIME service specified generic offset value.  */
typedef struct {

  /* Mandatory */
  /*  Base of which the offset is to be changed */
  time_qmi_time_bases_type_v01 base;
  /**<   - 0 - RTC
       - 1 - TOD
       - 2 - 1X
       - 3 - HDR
       - 4 - GPS
        */
}time_get_genoff_offset_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup time_qmi_messages
    @{
  */
/** Response Message; This gets the TIME service specified generic offset value.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Mandatory */
  /*  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  
 Base of which the offset is to be changed */
  time_qmi_time_bases_type_v01 base;
  /**<   - 0 - RTC
       - 1 - TOD
       - 2 - 1X
       - 3 - HDR
       - 4 - GPS
       */

  /* Mandatory */
  /*  Offset value to be set on the remote proc */
  uint64_t generic_offset;
  /**<   Current time = RTC value at bootup + generic offset + Uptime. 
    Therefore generic offset = 
    Number of msec elapsed through 01/06/1980 - RTC offset at bootup - uptime          
     */
}time_get_genoff_offset_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup time_qmi_messages
    @{
  */
/** Request Message; This sets the leap seconds on the modem.  */
typedef struct {

  /* Mandatory */
  /*  Value of leap seconds to be set */
  uint8_t leap_sec_set_value;
}time_set_leap_sec_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup time_qmi_messages
    @{
  */
/** Response Message; This sets the leap seconds on the modem.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}time_set_leap_sec_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * time_get_leap_sec_req_msg is empty
 * typedef struct {
 * }time_get_leap_sec_req_msg_v01;
 */

/** @addtogroup time_qmi_messages
    @{
  */
/** Response Message; This sets the leap seconds on the modem.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Mandatory */
  /*  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  
 Leap sec value to be set on the remote proc */
  uint8_t leap_second;
}time_get_leap_sec_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup time_qmi_messages
    @{
  */
/** Request Message; This turns off the specified indication */
typedef struct {

  /* Mandatory */
  /*  Indication to be turned off */
  int32_t msg_id;
}time_turn_off_ind_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup time_qmi_messages
    @{
  */
/** Response Message; This turns off the specified indication */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}time_turn_off_ind_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup time_qmi_messages
    @{
  */
/** Request Message; This turns on the specified indication */
typedef struct {

  /* Mandatory */
  /*  Indication to be turned on */
  int32_t msg_id;
}time_turn_on_ind_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup time_qmi_messages
    @{
  */
/** Response Message; This turns on the specified indication */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}time_turn_on_ind_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup time_qmi_messages
    @{
  */
/** Indication Message; This turns on the specified indication */
typedef struct {

  /* Mandatory */
  /*  Base to be set */
  time_qmi_time_bases_type_v01 base;
  /**<   - 0 - RTC
       - 1 - TOD
       - 2 - 1X
       - 3 - HDR
       - 4 - GPS
   */

  /* Mandatory */
  /*  Offset value to be sent over to the remote proc */
  uint64_t offset;
  /**<   Number of msec elapsed through 01/06/1980       
   */
}time_update_indication_message_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup time_qmi_msg_ids
    @{
  */
#define QMI_TIME_GENOFF_SET_REQ_MSG_V01 0x0020
#define QMI_TIME_GENOFF_SET_RESP_MSG_V01 0x0020
#define QMI_TIME_GENOFF_GET_REQ_MSG_V01 0x0021
#define QMI_TIME_GENOFF_GET_RESP_MSG_V01 0x0021
#define QMI_TIME_TURN_OFF_IND_REQ_MSG_V01 0x0022
#define QMI_TIME_TURN_OFF_IND_RESP_MSG_V01 0x0022
#define QMI_TIME_TURN_ON_IND_REQ_MSG_V01 0x0023
#define QMI_TIME_TURN_ON_IND_RESP_MSG_V01 0x0023
#define QMI_TIME_LEAP_SEC_SET_REQ_MSG_V01 0x0024
#define QMI_TIME_LEAP_SEC_SET_RESP_MSG_V01 0x0024
#define QMI_TIME_LEAP_SEC_GET_REQ_MSG_V01 0x0025
#define QMI_TIME_LEAP_SEC_GET_RESP_MSG_V01 0x0025
#define QMI_TIME_ATS_RTC_UPDATE_IND_MSG_V01 0x0026
#define QMI_TIME_ATS_TOD_UPDATE_IND_MSG_V01 0x0027
#define QMI_TIME_ATS_USER_UPDATE_IND_MSG_V01 0x0028
#define QMI_TIME_ATS_SECURE_UPDATE_IND_MSG_V01 0x0029
#define QMI_TIME_ATS_DRM_UPDATE_IND_MSG_V01 0x002A
#define QMI_TIME_ATS_USER_UTC_UPDATE_IND_MSG_V01 0x002B
#define QMI_TIME_ATS_USER_TZ_DL_UPDATE_IND_MSG_V01 0x002C
#define QMI_TIME_ATS_GPS_UPDATE_IND_MSG_V01 0x002D
#define QMI_TIME_ATS_1X_UPDATE_IND_MSG_V01 0x002E
#define QMI_TIME_ATS_HDR_UPDATE_IND_MSG_V01 0x002F
#define QMI_TIME_ATS_WCDMA_UPDATE_IND_MSG_V01 0x0030
#define QMI_TIME_ATS_BREW_UPDATE_IND_MSG_V01 0x0031
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro time_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type time_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define time_get_service_object_v01( ) \
          time_get_service_object_internal_v01( \
            TIME_V01_IDL_MAJOR_VERS, TIME_V01_IDL_MINOR_VERS, \
            TIME_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

