#ifndef SAP_SERVICE_H
#define SAP_SERVICE_H
/**
  @file service_access_proxy_v01.h

  @brief This is the public header file which defines the sap service Data structures.

  This header file defines the types and structures that were defined in
  sap. It contains the constant values defined, enums, structures,
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

  $Header: //source/qcom/qct/interfaces/qmi/sap/main/latest/api/service_access_proxy_v01.h#2 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 4.3
   It was generated on: Fri Feb 10 2012
   From IDL File: service_access_proxy_v01.idl */

/** @defgroup sap_qmi_consts Constant values defined in the IDL */
/** @defgroup sap_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup sap_qmi_enums Enumerated types used in QMI messages */
/** @defgroup sap_qmi_messages Structures sent as QMI messages */
/** @defgroup sap_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup sap_qmi_accessor Accessor for QMI service object */
/** @defgroup sap_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sap_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SAP_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SAP_V01_IDL_MINOR_VERS 0x00
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SAP_V01_IDL_TOOL_VERS 0x04
/** Maximum Defined Message ID */
#define SAP_V01_MAX_MESSAGE_ID 0x0023;
/**
    @}
  */


/** @addtogroup sap_qmi_consts
    @{
  */
/**
    @}
  */

/** @addtogroup sap_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t service_id;
  /**<   Service ID.  */

  uint32_t major_vers;
  /**<   Major version.  */

  uint32_t max_msg_len;
  /**<   Maximum message length.  */
}sap_service_obj_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sap_qmi_messages
    @{
  */
/** Request Message; This message registers a service with QMI_SAP. */
typedef struct {

  /* Mandatory */
  /*  Service Object */
  sap_service_obj_type_v01 service_obj;
}sap_register_service_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sap_qmi_messages
    @{
  */
/** Response Message; This message registers a service with QMI_SAP. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}sap_register_service_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sap_qmi_messages
    @{
  */
/** Request Message; This message deregisters a service from QMI_SAP. */
typedef struct {

  /* Mandatory */
  /*  Service Object */
  sap_service_obj_type_v01 service_obj;
}sap_deregister_service_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sap_qmi_messages
    @{
  */
/** Response Message; This message deregisters a service from QMI_SAP. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.  */
}sap_deregister_service_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sap_qmi_messages
    @{
  */
/** Indication Message; This message is sent when a client connects via QMUX. */
typedef struct {

  /* Mandatory */
  /*  Service ID */
  uint32_t service_id;
  /**<   Service ID.   */

  /* Mandatory */
  /*  Client ID */
  uint8_t client_id;
  /**<   Client ID.  */
}sap_client_connect_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sap_qmi_messages
    @{
  */
/** Indication Message; This message is sent when a QMUX client disconnects. */
typedef struct {

  /* Mandatory */
  /*  Service ID */
  uint32_t service_id;
  /**<   Service ID.  */

  /* Mandatory */
  /*  Client ID */
  uint8_t client_id;
  /**<   Client ID.  */
}sap_client_disconnect_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup sap_qmi_msg_ids
    @{
  */
#define QMI_SAP_REGISTER_SERVICE_REQ_V01 0x0020
#define QMI_SAP_REGISTER_SERVICE_RESP_V01 0x0020
#define QMI_SAP_DEREGISTER_SERVICE_REQ_V01 0x0021
#define QMI_SAP_DEREGISTER_SERVICE_RESP_V01 0x0021
#define QMI_SAP_CLIENT_CONNECT_IND_V01 0x0022
#define QMI_SAP_CLIENT_DISCONNECT_IND_V01 0x0023
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro sap_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type sap_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define sap_get_service_object_v01( ) \
          sap_get_service_object_internal_v01( \
            SAP_V01_IDL_MAJOR_VERS, SAP_V01_IDL_MINOR_VERS, \
            SAP_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

