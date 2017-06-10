#ifndef SNS_SMGR_RESTRICTED_SVC_SERVICE_01_H
#define SNS_SMGR_RESTRICTED_SVC_SERVICE_01_H
/**
  @file sns_smgr_restricted_api_v01.h

  @brief This is the public header file which defines the SNS_SMGR_RESTRICTED_SVC service Data structures.

  This header file defines the types and structures that were defined in
  SNS_SMGR_RESTRICTED_SVC. It contains the constant values defined, enums, structures,
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
  
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.6 
   It was generated on: Tue Mar  4 2014 (Spin 0)
   From IDL File: sns_smgr_restricted_api_v01.idl */

/** @defgroup SNS_SMGR_RESTRICTED_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_SMGR_RESTRICTED_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_SMGR_RESTRICTED_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_SMGR_RESTRICTED_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_SMGR_RESTRICTED_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_SMGR_RESTRICTED_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_SMGR_RESTRICTED_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_SMGR_RESTRICTED_SVC_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SNS_SMGR_RESTRICTED_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SMGR_RESTRICTED_SVC_V01_IDL_MINOR_VERS 0x03
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SMGR_RESTRICTED_SVC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_SMGR_RESTRICTED_SVC_V01_MAX_MESSAGE_ID 0x0021
/**
    @}
  */


/** @addtogroup SNS_SMGR_RESTRICTED_SVC_qmi_consts 
    @{ 
  */

/**  Status of Driver Access Framework, used in sns_smgr_driver_access_resp_msg.
  

 The request was processed and serviced successfully  */
#define SNS_SMGR_DRIVER_ACCESS_STATUS_SUCCESS_V01 0

/**  Invalid UUID provided in the request  */
#define SNS_SMGR_DRIVER_ACCESS_INVALID_UUID_V01 1

/**  Invalid parameter or message in the request  */
#define SNS_SMGR_DRIVER_ACCESS_INVALID_PARAM_V01 2

/**  Request is unsupported by the DD  */
#define SNS_SMGR_DRIVER_ACCESS_INVALID_REQ_V01 3

/**  Unspecified error in the SMGR  */
#define SNS_SMGR_DRIVER_ACCESS_SMGR_FAILURE_V01 4

/**  Unspecified error in the driver  */
#define SNS_SMGR_DRIVER_ACCESS_DD_FAILURE_V01 5

/**  Unspecified error  */
#define SNS_SMGR_DRIVER_ACCESS_FAIL_V01 6

/**  The request is pending completion  */
#define SNS_SMGR_DRIVER_ACCESS_PENDING_V01 7

/**  =============== Driver Access Framework Constants ===============
 Length of the UUID byte array  */
#define SNS_SMGR_UUID_LENGTH_V01 16

/**  Maximum size (in bytes) of the Driver Access message  */
#define SNS_SMGR_MAX_DAF_MESSAGE_SIZE_V01 256
/**
    @}
  */

/** @addtogroup SNS_SMGR_RESTRICTED_SVC_qmi_messages
    @{
  */
/** Request Message; This command allows a client to pass in an arbitrary message to the
    desired device driver. */
typedef struct {

  /* Mandatory */
  uint8_t Uuid[SNS_SMGR_UUID_LENGTH_V01];
  /**<   The UUID of the device driver the request is to be forwarded to.
  */

  /* Mandatory */
  uint32_t RequestId;
  /**<   The request identifier specifies a particular request. Furthermore,
       this field tells the driver how the RequestMsg is encoded so it may be
       properly decoded. This field is only understood by the Sensor1 client
       and the driver.
  */

  /* Optional */
  uint8_t RequestMsg_valid;  /**< Must be set to true if RequestMsg is being passed */
  uint32_t RequestMsg_len;  /**< Must be set to # of elements in RequestMsg */
  uint8_t RequestMsg[SNS_SMGR_MAX_DAF_MESSAGE_SIZE_V01];
  /**<   An opaque message to be delivered to the device driver. The device
       driver will parse this.
  */

  /* Optional */
  uint8_t TransactionId_valid;  /**< Must be set to true if TransactionId is being passed */
  uint8_t TransactionId;
  /**<   A unique identifier assigned by the client to distinguish this
       transaction from other concurrent transactions.
       IMPORTANT: It is up to the device driver to support concurrent
       transactions.
  */
}sns_smgr_driver_access_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_RESTRICTED_SVC_qmi_messages
    @{
  */
/** Response Message; This command allows a client to pass in an arbitrary message to the
    desired device driver. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;

  /* Optional */
  uint8_t ResponseStatus_valid;  /**< Must be set to true if ResponseStatus is being passed */
  uint8_t ResponseStatus;
  /**<   An error code generated by the SMGR.
      - SNS_SMGR_DRIVER_ACCESS_STATUS_SUCCESS - The request was processed
          and serviced successfully
      - SNS_SMGR_DRIVER_ACCESS_INVALID_UUID - Invalid UUID provided in the
          request
      - SNS_SMGR_DRIVER_ACCESS_INVALID_PARAM - Invalid parameter or message
          in the request
      - SNS_SMGR_DRIVER_ACCESS_INVALID_REQ - Request is unsupported by the
          DD
      - SNS_SMGR_DRIVER_ACCESS_SMGR_FAILURE - Unspecified error in the SMGR
      - SNS_SMGR_DRIVER_ACCESS_DD_FAILURE - Unspecified error in the driver
      - SNS_SMGR_DRIVER_ACCESS_FAIL - Unspecified error
      - SNS_SMGR_DRIVER_ACCESS_PENDING - The request is pending completion
      - All other values defined as SNS_SMGR_DRIVER_ACCESS_XXXX style are
          reserved for future use
  */

  /* Optional */
  uint8_t ResponseMsg_valid;  /**< Must be set to true if ResponseMsg is being passed */
  uint32_t ResponseMsg_len;  /**< Must be set to # of elements in ResponseMsg */
  uint8_t ResponseMsg[SNS_SMGR_MAX_DAF_MESSAGE_SIZE_V01];
  /**<   An opaque message to be delivered to the client. The client will parse
       this within the context of the request that the client last sent.
  */

  /* Optional */
  uint8_t TransactionId_valid;  /**< Must be set to true if TransactionId is being passed */
  uint8_t TransactionId;
  /**<   A unique identifier assigned by the client to distinguish this
       transaction from other concurrent transactions.
       IMPORTANT: It is up to the device driver to support concurrent
       transactions.
  */
}sns_smgr_driver_access_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_RESTRICTED_SVC_qmi_messages
    @{
  */
/** Indication Message; Driver Access Framework Message sent to the client from the driver */
typedef struct {

  /* Mandatory */
  uint8_t IndicationId;
  /**<   The indication identifier specifies a particular request. Furthermore,
       this field tells the Sensor1 client how the IndicationMsg is encoded
       so it may be properly decoded. This field is only understood by the
       Sensor1 client and the driver.
  */

  /* Optional */
  uint8_t IndicationMsg_valid;  /**< Must be set to true if IndicationMsg is being passed */
  uint32_t IndicationMsg_len;  /**< Must be set to # of elements in IndicationMsg */
  uint8_t IndicationMsg[SNS_SMGR_MAX_DAF_MESSAGE_SIZE_V01];
  /**<   An opaque message to be delivered to the client. The client will parse
       this according to the IndicationId.
  */

  /* Optional */
  uint8_t TransactionId_valid;  /**< Must be set to true if TransactionId is being passed */
  uint8_t TransactionId;
  /**<   A unique identifier assigned by the client to distinguish this
       transaction from other concurrent transactions.
       IMPORTANT: It is up to the device driver to support concurrent
       transactions.
  */
}sns_smgr_driver_access_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup SNS_SMGR_RESTRICTED_SVC_qmi_msg_ids
    @{
  */
#define SNS_SMGR_RESTRICTED_CANCEL_REQ_V01 0x0000
#define SNS_SMGR_RESTRICTED_CANCEL_RESP_V01 0x0000
#define SNS_SMGR_RESTRICTED_VERSION_REQ_V01 0x0001
#define SNS_SMGR_RESTRICTED_VERSION_RESP_V01 0x0001
#define SNS_SMGR_DRIVER_ACCESS_REQ_V01 0x0020
#define SNS_SMGR_DRIVER_ACCESS_RESP_V01 0x0020
#define SNS_SMGR_DRIVER_ACCESS_IND_V01 0x0021
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_SMGR_RESTRICTED_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_SMGR_RESTRICTED_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_SMGR_RESTRICTED_SVC_get_service_object_v01( ) \
          SNS_SMGR_RESTRICTED_SVC_get_service_object_internal_v01( \
            SNS_SMGR_RESTRICTED_SVC_V01_IDL_MAJOR_VERS, SNS_SMGR_RESTRICTED_SVC_V01_IDL_MINOR_VERS, \
            SNS_SMGR_RESTRICTED_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

