#ifndef SNS_SMGR_INTERNAL_SVC_SERVICE_01_H
#define SNS_SMGR_INTERNAL_SVC_SERVICE_01_H
/**
  @file sns_smgr_internal_api_v01.h
  
  @brief This is the public header file which defines the SNS_SMGR_INTERNAL_SVC service Data structures.

  This header file defines the types and structures that were defined in 
  SNS_SMGR_INTERNAL_SVC. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It requires encode/decode library version 4 or later
   It was generated on: Fri May 24 2013 (Spin 0)
   From IDL File: sns_smgr_internal_api_v01.idl */

/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_consts Constant values defined in the IDL */
/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_enums Enumerated types used in QMI messages */
/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_messages Structures sent as QMI messages */
/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_accessor Accessor for QMI service object */
/** @defgroup SNS_SMGR_INTERNAL_SVC_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "sns_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define SNS_SMGR_INTERNAL_SVC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SNS_SMGR_INTERNAL_SVC_V01_IDL_MINOR_VERS 0x03
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SNS_SMGR_INTERNAL_SVC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SNS_SMGR_INTERNAL_SVC_V01_MAX_MESSAGE_ID 0x0021;
/** 
    @} 
  */


/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_consts 
    @{ 
  */

/**  register HW MD interupt in sns_smgr_reg_hw_md_int_req_msg  */
#define SNS_SMGR_REG_HW_MD_INT_ADD_V01 0

/**  un-register HW MD interupt in sns_smgr_reg_hw_md_int_req_msg  */
#define SNS_SMGR_REG_HW_MD_INT_DEL_V01 1

/**  identify source module as SAM on apps processor  */
#define SNS_MODULE_APPS_SAM_V01 1

/**  identify source module as SAM on DSPS/ADSP processor  */
#define SNS_MODULE_DSPS_SAM_V01 13

/**  successful to register HW MD interupt in sns_smgr_reg_hw_md_int_resp_msg 
     or sns_smgr_reg_hw_md_int_ind_msg  */
#define SNS_SMGR_REG_HW_MD_INT_ENABLED_V01 0

/**  failed  to register HW MD interupt in sns_smgr_reg_hw_md_int_resp_msg 
    or sns_smgr_reg_hw_md_int_ind_msg */
#define SNS_SMGR_REG_HW_MD_INT_DISABLED_V01 1

/**  HW MD interupt happened in sns_smgr_reg_hw_md_int_ind_msg  */
#define SNS_SMGR_REG_HW_MD_INT_OCCURRED_V01 2

/**  Access successful  */
#define SNS_SMGR_INTERNAL_DEV_ACCESS_RESP_SUCCESS_V01 0

/**  Device error  */
#define SNS_SMGR_INTERNAL_DEV_ACCESS_RESP_ERR_DEVICE_V01 1

/**  Invalid address  */
#define SNS_SMGR_INTERNAL_DEV_ACCESS_RESP_ERR_ADDR_V01 2

/**  Maximum number of bytes to be read/written in a single request  */
#define SNS_SMGR_INTERNAL_MAX_DATA_LEN_V01 512
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Request Message; This command requests to register HW Motion Detection Interrupt.
    SMGR will turn on MD  int if all accel requests are  from  clients who registered 
    MD int and  report ID from both requests match; 
    SMGR will suspend reports by these requests and turn on MD int until MD int happens.
    Client can register for HW MD interupt first and request accel data with same report ID later ;
    Or request for accel data first, then register for HW MD interrupt later with the same report ID.
    The MD requests is deleted when the client explicitly deregisters or when MD interrupt occurs and 
    SMGR notifies client. 
    If associated streaming is canceled, it doesn’t imply canceling the HW MD registering request.
    How SMGR should treat the HW MD registering message if it does not have an associated streaming 
    but there is a streaming by other client. Still SMGR save and maintains the HW MD registering requests?
    SMGR will maintain the request, and treat this request the same as other client such as QMD.
    Previous MD request is deleted if a new one has the same source module and report ID. */
typedef struct {

  /* Mandatory */
  uint8_t ReportId;
  /**<   ID assigned by client to distinguish client's reports */

  /* Mandatory */
  uint8_t Action;
  /**<   SNS_SMGR_REG_HW_MD_INT_ADD =0 ,Add;
       SNS_SMGR_REG_HW_MD_INT_DEL =1, Delete */

  /* Optional */
  uint8_t SrcModule_valid;  /**< Must be set to true if SrcModule is being passed */
  uint8_t SrcModule;
  /**<   Added in @VERSION 1.2
       This field is used to subscribe to Motion Detect interrupt. If the
       field is used, it accepts only the values below. All other
       values are rejected (indicated as DISABLED in the corresponding
       response message).

       If the field is enabled and it is a valid value (from the list
       below), the response will indicated ENABLED (provided that
       interrupt is supported.). When the interrupt is un-subscribed after
       an ENABLED response, an indication is sent to the client.

       SNS_MODULE_APPS_SAM =1,  SAM on apps;
       SNS_MODULE_DSPS_SAM =13, SAM on DSPS/ADSP */
}sns_smgr_reg_hw_md_int_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Response Message; This command requests to register HW Motion Detection Interrupt.
    SMGR will turn on MD  int if all accel requests are  from  clients who registered 
    MD int and  report ID from both requests match; 
    SMGR will suspend reports by these requests and turn on MD int until MD int happens.
    Client can register for HW MD interupt first and request accel data with same report ID later ;
    Or request for accel data first, then register for HW MD interrupt later with the same report ID.
    The MD requests is deleted when the client explicitly deregisters or when MD interrupt occurs and 
    SMGR notifies client. 
    If associated streaming is canceled, it doesn’t imply canceling the HW MD registering request.
    How SMGR should treat the HW MD registering message if it does not have an associated streaming 
    but there is a streaming by other client. Still SMGR save and maintains the HW MD registering requests?
    SMGR will maintain the request, and treat this request the same as other client such as QMD.
    Previous MD request is deleted if a new one has the same source module and report ID. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;

  /* Mandatory */
  uint8_t ReportId;
  /**<   ID assigned by client to distinguish client's reports */

  /* Mandatory */
  uint8_t result;
  /**<   SNS_SMGR_REG_HW_MD_INT_ENABLED =0 , success;
       SNS_SMGR_REG_HW_MD_INT_DISABLED =1, fail */
}sns_smgr_reg_hw_md_int_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** indication Message; This command requests to register HW Motion Detection Interrupt.
    SMGR will turn on MD  int if all accel requests are  from  clients who registered 
    MD int and  report ID from both requests match; 
    SMGR will suspend reports by these requests and turn on MD int until MD int happens.
    Client can register for HW MD interupt first and request accel data with same report ID later ;
    Or request for accel data first, then register for HW MD interrupt later with the same report ID.
    The MD requests is deleted when the client explicitly deregisters or when MD interrupt occurs and 
    SMGR notifies client. 
    If associated streaming is canceled, it doesn’t imply canceling the HW MD registering request.
    How SMGR should treat the HW MD registering message if it does not have an associated streaming 
    but there is a streaming by other client. Still SMGR save and maintains the HW MD registering requests?
    SMGR will maintain the request, and treat this request the same as other client such as QMD.
    Previous MD request is deleted if a new one has the same source module and report ID. */
typedef struct {

  /* Mandatory */
  uint8_t ReportId;
  /**<   ID assigned by client to distinguish client's reports */

  /* Mandatory */
  uint8_t indication;
  /**<   see #define SNS_SMGR_REG_HW_MD_INT_XXX_XXX  */
}sns_smgr_reg_hw_md_int_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Request Message; This command allows the client to access I/O registers in sensor
           hardware. All requests are processed synchronously. */
typedef struct {

  /* Mandatory */
  uint8_t SensorId;
  /**<   Desired Sensor ID associated with the request */

  /* Mandatory */
  uint64_t Addr;
  /**<   Address of device register. Assumes both client and service know
       the actual length of the address in question.
   */

  /* Mandatory */
  uint32_t Bytes_len;
  /**<   Number of data bytes to read */
}sns_smgr_internal_dev_access_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Response Message; This command allows the client to access I/O registers in sensor
           hardware. All requests are processed synchronously. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;

  /* Optional */
  uint8_t Result_valid;  /**< Must be set to true if Result is being passed */
  uint16_t Result;
  /**<  
    Lower byte (LSB) defines the status of the associated request:
    - SNS_SMGR_INTERNAL_DEV_ACCESS_RESP_SUCCESS - the request has been accepted
    - SNS_SMGR_INTERNAL_DEV_ACCESS_RESP_ERR_DEVICE - generic I/O error
    - SNS_SMGR_INTERNAL_DEV_ACCESS_RESP_ERR_ADDR   - invalid device address

    Upper byte (MSB) defines any (optional) additional error code information.
  */

  /* Optional */
  uint8_t SensorId_valid;  /**< Must be set to true if SensorId is being passed */
  uint8_t SensorId;
  /**<   Sensor ID associated with the request message */

  /* Optional */
  uint8_t Addr_valid;  /**< Must be set to true if Addr is being passed */
  uint64_t Addr;
  /**<   Address of device register */

  /* Optional */
  uint8_t Bytes_valid;  /**< Must be set to true if Bytes is being passed */
  uint32_t Bytes_len;  /**< Must be set to # of elements in Bytes */
  uint8_t Bytes[SNS_SMGR_INTERNAL_MAX_DATA_LEN_V01];
  /**<   Data bytes read from hardware register */
}sns_smgr_internal_dev_access_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Request Message; This command allows the client to access I/O registers in sensor
           hardware. All requests are processed synchronously. */
typedef struct {

  /* Mandatory */
  uint8_t SensorId;
  /**<   Desired Sensor ID associated with the request */

  /* Mandatory */
  uint64_t Addr;
  /**<   Address of device register. Assumes both client and service know
       the actual length of the address in question.
   */

  /* Mandatory */
  uint32_t Bytes_len;  /**< Must be set to # of elements in Bytes */
  uint32_t Bytes[SNS_SMGR_INTERNAL_MAX_DATA_LEN_V01];
  /**<   Data bytes to be written */
}sns_smgr_internal_dev_access_write_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_messages
    @{
  */
/** Response Message; This command allows the client to access I/O registers in sensor
           hardware. All requests are processed synchronously. */
typedef struct {

  /* Mandatory */
  sns_common_resp_s_v01 Resp;

  /* Optional */
  uint8_t Result_valid;  /**< Must be set to true if Result is being passed */
  uint16_t Result;
  /**<  
    Lower byte (LSB) defines the status of the associated request:
    - SNS_SMGR_INTERNAL_DEV_ACCESS_RESP_SUCCESS - the request has been accepted
    - SNS_SMGR_INTERNAL_DEV_ACCESS_RESP_ERR_DEVICE - generic I/O error
    - SNS_SMGR_INTERNAL_DEV_ACCESS_RESP_ERR_ADDR   - invalid device address

    Upper byte (MSB) defines any (optional) additional error code information.
  */

  /* Optional */
  uint8_t SensorId_valid;  /**< Must be set to true if SensorId is being passed */
  uint8_t SensorId;
  /**<   Sensor ID associated with the request message */
}sns_smgr_internal_dev_access_write_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup SNS_SMGR_INTERNAL_SVC_qmi_msg_ids
    @{
  */
#define SNS_SMGR_INTERNAL_CANCEL_REQ_V01 0x0001
#define SNS_SMGR_INTERNAL_CANCEL_RESP_V01 0x0001
#define SNS_SMGR_INTERNAL_VERSION_REQ_V01 0x0002
#define SNS_SMGR_INTERNAL_VERSION_RESP_V01 0x0002
#define SNS_SMGR_REG_HW_MD_INT_REQ_V01 0x0003
#define SNS_SMGR_REG_HW_MD_INT_RESP_V01 0x0003
#define SNS_SMGR_REG_HW_MD_INT_IND_V01 0x0004
#define SNS_SMGR_INTERNAL_DEV_ACCESS_READ_REQ_V01 0x0020
#define SNS_SMGR_INTERNAL_DEV_ACCESS_READ_RESP_V01 0x0020
#define SNS_SMGR_INTERNAL_DEV_ACCESS_WRITE_REQ_V01 0x0021
#define SNS_SMGR_INTERNAL_DEV_ACCESS_WRITE_RESP_V01 0x0021
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro SNS_SMGR_INTERNAL_SVC_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type SNS_SMGR_INTERNAL_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define SNS_SMGR_INTERNAL_SVC_get_service_object_v01( ) \
          SNS_SMGR_INTERNAL_SVC_get_service_object_internal_v01( \
            SNS_SMGR_INTERNAL_SVC_V01_IDL_MAJOR_VERS, SNS_SMGR_INTERNAL_SVC_V01_IDL_MINOR_VERS, \
            SNS_SMGR_INTERNAL_SVC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

