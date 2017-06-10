#ifndef TEST_SERVICE_H
#define TEST_SERVICE_H
/**
  @file test_service_v01.h

  @brief This is the public header file which defines the test service Data structures.

  This header file defines the types and structures that were defined in
  test. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

  $Header: //source/qcom/qct/interfaces/qmi/test/main/latest/api/test_service_v01.h#2 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 4.3
   It was generated on: Tue Dec 13 2011
   From IDL File: test_service_v01.idl */

/** @defgroup test_qmi_consts Constant values defined in the IDL */
/** @defgroup test_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup test_qmi_enums Enumerated types used in QMI messages */
/** @defgroup test_qmi_messages Structures sent as QMI messages */
/** @defgroup test_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup test_qmi_accessor Accessor for QMI service object */
/** @defgroup test_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup test_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define TEST_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define TEST_V01_IDL_MINOR_VERS 0x04
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define TEST_V01_IDL_TOOL_VERS 0x04
/** Maximum Defined Message ID */
#define TEST_V01_MAX_MESSAGE_ID 0x0026;
/**
    @}
  */


/** @addtogroup test_qmi_consts
    @{
  */

/**  Maximum size for a data TLV */
#define TEST_MED_DATA_SIZE_V01 8192

/**  Maximum size of a large data TLV, below a full 64k to accommodate
 response, client_name, and service_name TLVs. */
#define TEST_LARGE_MAX_DATA_SIZE_V01 65000

/**  Maximum length of a client or service name */
#define TEST_MAX_NAME_SIZE_V01 255
/**
    @}
  */

/** @addtogroup test_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t name_len;  /**< Must be set to # of elements in name */
  char name[TEST_MAX_NAME_SIZE_V01];
  /**<   In a request, the optional name to identify a client.
         In a response or an indication, the optional name to identify a
         service.  */
}test_name_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup test_qmi_messages
    @{
  */
/** Request Message; Tests a basic message passing between the client and the service. */
typedef struct {

  /* Mandatory */
  /*  Ping */
  char ping[4];
  /**<   Simple ping request.  */

  /* Optional */
  /*  Optional Client Name */
  uint8_t client_name_valid;  /**< Must be set to true if client_name is being passed */
  test_name_type_v01 client_name;
}test_ping_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup test_qmi_messages
    @{
  */
/** Response Message; Tests a basic message passing between the client and the service. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Pong */
  uint8_t pong_valid;  /**< Must be set to true if pong is being passed */
  char pong[4];
  /**<   Simple pong response.  */

  /* Optional */
  /*  Optional Service Name */
  uint8_t service_name_valid;  /**< Must be set to true if service_name is being passed */
  test_name_type_v01 service_name;
}test_ping_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup test_qmi_messages
    @{
  */
/** Indication Message; Tests a basic message passing between the client and the service. */
typedef struct {

  /* Mandatory */
  /*  Hello Indication */
  char indication[5];
  /**<   Simple hello indication.  */

  /* Optional */
  /*  Optional Service Name */
  uint8_t service_name_valid;  /**< Must be set to true if service_name is being passed */
  test_name_type_v01 service_name;
}test_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup test_qmi_messages
    @{
  */
/** Request Message; Tests variably sized messages. */
typedef struct {

  /* Mandatory */
  /*  Ping Data */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[TEST_MED_DATA_SIZE_V01];
  /**<   Variably sized data request.  */

  /* Optional */
  /*  Optional Client Name */
  uint8_t client_name_valid;  /**< Must be set to true if client_name is being passed */
  test_name_type_v01 client_name;
}test_data_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup test_qmi_messages
    @{
  */
/** Response Message; Tests variably sized messages. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Response Data */
  uint8_t data_valid;  /**< Must be set to true if data is being passed */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[TEST_MED_DATA_SIZE_V01];
  /**<   Variably sized data response.  */

  /* Optional */
  /*  Optional Service Name */
  uint8_t service_name_valid;  /**< Must be set to true if service_name is being passed */
  test_name_type_v01 service_name;
}test_data_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup test_qmi_messages
    @{
  */
/** Request Message; Tests large variably sized messages. */
typedef struct {

  /* Mandatory */
  /*  Ping Data */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[TEST_LARGE_MAX_DATA_SIZE_V01];
  /**<   Variably sized data request.  */

  /* Optional */
  /*  Optional Client Name */
  uint8_t client_name_valid;  /**< Must be set to true if client_name is being passed */
  test_name_type_v01 client_name;
}test_large_data_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup test_qmi_messages
    @{
  */
/** Response Message; Tests large variably sized messages. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Response Data */
  uint8_t data_valid;  /**< Must be set to true if data is being passed */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[TEST_LARGE_MAX_DATA_SIZE_V01];
  /**<   Variably sized data response.  */

  /* Optional */
  /*  Optional Service Name */
  uint8_t service_name_valid;  /**< Must be set to true if service_name is being passed */
  test_name_type_v01 service_name;
}test_large_data_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup test_qmi_messages
    @{
  */
/** Request Message; Registers for variably sized indications. */
typedef struct {

  /* Optional */
  /*  Number of Indications */
  uint8_t num_inds_valid;  /**< Must be set to true if num_inds is being passed */
  uint16_t num_inds;
  /**<   Number of indications to send.  */

  /* Optional */
  /*  Size of Indications */
  uint8_t ind_size_valid;  /**< Must be set to true if ind_size is being passed */
  uint16_t ind_size;
  /**<   Maximum value is TEST_MED_DATA_SIZE.  */

  /* Optional */
  /*  Delay Between Indications */
  uint8_t ms_delay_valid;  /**< Must be set to true if ms_delay is being passed */
  uint16_t ms_delay;
  /**<   Delay (in milliseconds) for the service to wait between sending indications.  */

  /* Optional */
  /*  Indication Delayed Start */
  uint8_t num_reqs_delay_valid;  /**< Must be set to true if num_reqs_delay is being passed */
  uint16_t num_reqs_delay;
  /**<
      Tells the service to wait until ind_num_reqs requests have been received
      before sending any indications.
	 */

  /* Optional */
  /*  Optional Client Name */
  uint8_t client_name_valid;  /**< Must be set to true if client_name is being passed */
  test_name_type_v01 client_name;
}test_data_ind_reg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup test_qmi_messages
    @{
  */
/** Response Message; Registers for variably sized indications. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /*  Optional Service Name */
  uint8_t service_name_valid;  /**< Must be set to true if service_name is being passed */
  test_name_type_v01 service_name;
}test_data_ind_reg_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup test_qmi_messages
    @{
  */
/** Indication Message; Tests sending variably sized indications. */
typedef struct {

  /* Mandatory */
  /*  Indication Data */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[TEST_MED_DATA_SIZE_V01];
  /**<   Variably sized data indication.  */

  /* Optional */
  /*  Optional Service Name */
  uint8_t service_name_valid;  /**< Must be set to true if service_name is being passed */
  test_name_type_v01 service_name;

  /* Optional */
  /*  Optional Checksum Value */
  uint8_t sum_valid;  /**< Must be set to true if sum is being passed */
  uint32_t sum;
  /**<   Optional checksum value to validate the data.  */
}test_data_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup test_qmi_messages
    @{
  */
/** Request Message; Queries the service for its name. */
typedef struct {

  /* Optional */
  /*  Optional Client Name */
  uint8_t client_name_valid;  /**< Must be set to true if client_name is being passed */
  test_name_type_v01 client_name;
}test_get_service_name_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup test_qmi_messages
    @{
  */
/** Response Message; Queries the service for its name. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /* Service Name */
  uint8_t service_name_valid;  /**< Must be set to true if service_name is being passed */
  test_name_type_v01 service_name;
}test_get_service_name_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * test_null_req_msg is empty
 * typedef struct {
 * }test_null_req_msg_v01;
 */

/** @addtogroup test_qmi_messages
    @{
  */
/** Response Message; Sends an empty request message. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */

  /* Optional */
  /* Service Name */
  uint8_t service_name_valid;  /**< Must be set to true if service_name is being passed */
  test_name_type_v01 service_name;
}test_null_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * test_null_ind_msg is empty
 * typedef struct {
 * }test_null_ind_msg_v01;
 */

/*Service Message Definition*/
/** @addtogroup test_qmi_msg_ids
    @{
  */
#define QMI_TEST_REQ_V01 0x0020
#define QMI_TEST_RESP_V01 0x0020
#define QMI_TEST_IND_V01 0x0020
#define QMI_TEST_DATA_REQ_V01 0x0021
#define QMI_TEST_DATA_RESP_V01 0x0021
#define QMI_TEST_LARGE_DATA_REQ_V01 0x0022
#define QMI_TEST_LARGE_DATA_RESP_V01 0x0022
#define QMI_TEST_DATA_IND_REG_REQ_V01 0x0023
#define QMI_TEST_DATA_IND_REG_RESP_V01 0x0023
#define QMI_TEST_DATA_IND_V01 0x0024
#define QMI_TEST_GET_SERVICE_NAME_REQ_V01 0x0025
#define QMI_TEST_GET_SERVICE_NAME_RESP_V01 0x0025
#define QMI_TEST_NULL_REQ_V01 0x0026
#define QMI_TEST_NULL_RESP_V01 0x0026
#define QMI_TEST_NULL_IND_V01 0x0026
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro test_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type test_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define test_get_service_object_v01( ) \
          test_get_service_object_internal_v01( \
            TEST_V01_IDL_MAJOR_VERS, TEST_V01_IDL_MINOR_VERS, \
            TEST_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

