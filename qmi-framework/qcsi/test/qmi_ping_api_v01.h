#ifndef PING_SERVICE_H
#define PING_SERVICE_H
/**
  @file qmi_ping_api_v01.h

  @brief This is the public header file which defines the ping service Data structures.

  This header file defines the types and structures that were defined in
  ping. It contains the constant values defined, enums, structures,
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

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 2.5
   It was generated on: Fri May 13 2011
   From IDL File: qmi_ping_api_v01.idl */

/** @defgroup ping_qmi_consts Constant values defined in the IDL */
/** @defgroup ping_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup ping_qmi_enums Enumerated types used in QMI messages */
/** @defgroup ping_qmi_messages Structures sent as QMI messages */
/** @defgroup ping_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup ping_qmi_accessor Accessor for QMI service object */
/** @defgroup ping_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup ping_qmi_version
    @{
  */ 
/** Major Version Number of the IDL used to generate this file */
#define PING_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define PING_V01_IDL_MINOR_VERS 0x03
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define PING_V01_IDL_TOOL_VERS 0x02
/** Maximum Defined Message ID */
#define PING_V01_MAX_MESSAGE_ID 0x0006;
/**
    @}
  */


/** @addtogroup ping_qmi_consts
    @{
  */

/**  Maximum size for a data TLV */
#define PING_MAX_DATA_SIZE_V01 8192

/**  Maximum length of a client or service name */
#define PING_MAX_NAME_SIZE_V01 255
/**
    @}
  */

/** @addtogroup ping_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t name_len;  /**< Must be set to # of elements in name */
  char name[PING_MAX_NAME_SIZE_V01];
}ping_name_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ping_qmi_messages
    @{
  */
/** Request Message; This message tests basic message passing between
	client and service */
typedef struct {

  /* Mandatory */
  /*  Ping */
  char ping[4];
  /**<   Simple 'ping' request  */

  /* Optional */
  /*  Optional Client Name */
  uint8_t client_name_valid;  /**< Must be set to true if client_name is being passed */
  ping_name_type_v01 client_name;
  /**<   Optional name to identify clients  */
}ping_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ping_qmi_messages
    @{
  */
/** Response Message; This message tests basic message passing between
	client and service */
typedef struct {

  /* Mandatory */
  /*  Pong */
  char pong[4];
  /**<   Simple 'pong' response  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Optional Service Name */
  uint8_t service_name_valid;  /**< Must be set to true if service_name is being passed */
  ping_name_type_v01 service_name;
  /**<   Optional name to identify service  */
}ping_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ping_qmi_messages
    @{
  */
/** Indication Message; This message tests basic message passing between
	client and service */
typedef struct {

  /* Mandatory */
  /*  Hello indication */
  char indication[5];
  /**<   Simple 'hello' indication  */

  /* Optional */
  /*  Optional Service Name */
  uint8_t service_name_valid;  /**< Must be set to true if service_name is being passed */
  ping_name_type_v01 service_name;
  /**<   Optional name to identify service  */
}ping_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ping_qmi_messages
    @{
  */
/** Request Message; This message tests variably sized messages */
typedef struct {

  /* Mandatory */
  /*  Ping Data */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[PING_MAX_DATA_SIZE_V01];
  /**<   Variable sized data request  */

  /* Optional */
  /*  Optional Client Name */
  uint8_t client_name_valid;  /**< Must be set to true if client_name is being passed */
  ping_name_type_v01 client_name;
  /**<   Optional name to identify clients  */
}ping_data_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ping_qmi_messages
    @{
  */
/** Response Message; This message tests variably sized messages */
typedef struct {

  /* Mandatory */
  /*  Response Data */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[PING_MAX_DATA_SIZE_V01];
  /**<   Variable sized data response  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Optional Service Name */
  uint8_t service_name_valid;  /**< Must be set to true if service_name is being passed */
  ping_name_type_v01 service_name;
  /**<   Optional name to identify service  */
}ping_data_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ping_qmi_messages
    @{
  */
/** Request Message; This message registers for variable sized indications */
typedef struct {

  /* Optional */
  /*  Number of Indications */
  uint8_t num_inds_valid;  /**< Must be set to true if num_inds is being passed */
  uint16_t num_inds;
  /**<   Number of indications to send */

  /* Optional */
  /*  Size of Indications */
  uint8_t ind_size_valid;  /**< Must be set to true if ind_size is being passed */
  uint16_t ind_size;
  /**<   Max value 65000 */

  /* Optional */
  /*  Delay between Indications */
  uint8_t ind_delay_valid;  /**< Must be set to true if ind_delay is being passed */
  uint16_t ind_delay;
  /**<   Delay (in milliseconds) for the service to wait before sending indications  */

  /* Optional */
  /*  Indication Delayed start */
  uint8_t num_reqs_valid;  /**< Must be set to true if num_reqs is being passed */
  uint16_t num_reqs;
  /**<
      Tells the server to wait until ind_num_reqs requests have been
	  received before sending the indications described in the other
	  message fields.
	 */

  /* Optional */
  /*  Optional Client Name */
  uint8_t client_name_valid;  /**< Must be set to true if client_name is being passed */
  ping_name_type_v01 client_name;
  /**<   Optional name to identify clients  */
}ping_data_ind_reg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ping_qmi_messages
    @{
  */
/** Response Message; This message registers for variable sized indications */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Optional Service Name */
  uint8_t service_name_valid;  /**< Must be set to true if service_name is being passed */
  ping_name_type_v01 service_name;
  /**<   Optional name to identify service  */
}ping_data_ind_reg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ping_qmi_messages
    @{
  */
/** Indication Message; This message tests sending variably sized indications */
typedef struct {

  /* Mandatory */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[PING_MAX_DATA_SIZE_V01];
  /**<   Variable sized data indication  */

  /* Optional */
  /*  Optional Service Name */
  uint8_t service_name_valid;  /**< Must be set to true if service_name is being passed */
  ping_name_type_v01 service_name;
  /**<   Optional name to identify service  */
}ping_data_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ping_qmi_messages
    @{
  */
/** Request Message; This message queries the service for it's name */
typedef struct {

  /* Optional */
  /*  Optional Client Name */
  uint8_t client_name_valid;  /**< Must be set to true if client_name is being passed */
  ping_name_type_v01 client_name;
  /**<   Optional name to identify clients  */
}ping_get_service_name_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ping_qmi_messages
    @{
  */
/** Response Message; This message queries the service for it's name */
typedef struct {

  /* Mandatory */
  /* Service Name */
  ping_name_type_v01 service_name;
  /**<   Name to identify service  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}ping_get_service_name_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * ping_null_req_msg is empty
 * typedef struct {
 * }ping_null_req_msg_v01;
 */

/** @addtogroup ping_qmi_messages
    @{
  */
/** Response Message; This message sends an empty request message */
typedef struct {

  /* Mandatory */
  /* Service Name */
  ping_name_type_v01 service_name;
  /**<   Name to identify service  */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}ping_null_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * ping_null_ind_msg is empty
 * typedef struct {
 * }ping_null_ind_msg_v01;
 */

/*Service Message Definition*/
/** @addtogroup ping_qmi_msg_ids
    @{
  */
#define QMI_PING_REQ_V01 0x0001
#define QMI_PING_RESP_V01 0x0001
#define QMI_PING_IND_V01 0x0001
#define QMI_PING_DATA_REQ_V01 0x0002
#define QMI_PING_DATA_RESP_V01 0x0002
#define QMI_PING_DATA_IND_REG_REQ_V01 0x0003
#define QMI_PING_DATA_IND_REG_RESP_V01 0x0003
#define QMI_PING_DATA_IND_V01 0x0004
#define QMI_PING_GET_SERVICE_NAME_REQ_V01 0x0005
#define QMI_PING_GET_SERVICE_NAME_RESP_V01 0x0005
#define QMI_PING_NULL_REQ_V01 0x0006
#define QMI_PING_NULL_RESP_V01 0x0006
#define QMI_PING_NULL_IND_V01 0x0006
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro ping_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type ping_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define ping_get_service_object_v01( ) \
          ping_get_service_object_internal_v01( \
            PING_V01_IDL_MAJOR_VERS, PING_V01_IDL_MINOR_VERS, \
            PING_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

