#ifndef ADC_SERVICE_H
#define ADC_SERVICE_H
/**
  @file qmi_adc_service_v01.h
  
  @brief This is the public header file which defines the adc service Data structures.

  This header file defines the types and structures that were defined in 
  adc. It contains the constant values defined, enums, structures,
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
   It was generated on: Fri Apr 29 2011
   From IDL File: qmi_adc_service_v01.idl */

/** @defgroup adc_qmi_consts Constant values defined in the IDL */
/** @defgroup adc_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup adc_qmi_enums Enumerated types used in QMI messages */
/** @defgroup adc_qmi_messages Structures sent as QMI messages */
/** @defgroup adc_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup adc_qmi_accessor Accessor for QMI service object */
/** @defgroup adc_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup adc_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define ADC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define ADC_V01_IDL_MINOR_VERS 0x00
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define ADC_V01_IDL_TOOL_VERS 0x02
/** Maximum Defined Message ID */
#define ADC_V01_MAX_MESSAGE_ID 0x0009;
/** 
    @} 
  */


/** @addtogroup adc_qmi_consts 
    @{ 
  */
#define RESPONSE_ARRAY_V01 5
#define ADC_CH_NAME_LENGTH_V01 40
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Request Message; Gets the properties for an analog input. */
typedef struct {

  /* Mandatory */
  char adc_channel_name[ADC_CH_NAME_LENGTH_V01 + 1];
}adc_get_input_properties_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup adc_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t nDeviceIdx;

  uint32_t nChannelIdx;
}AdcInputPropertiesType_v01;  /* Type */
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Response Message; Gets the properties for an analog input. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  new field */
  uint8_t resp_value_valid;  /**< Must be set to true if resp_value is being passed */
  AdcInputPropertiesType_v01 resp_value;
}adc_get_input_properties_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup adc_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t nDeviceIdx;

  /* !< Device index.   */
  uint32_t nChannelIdx;
}AdcRequestParametersType_v01;  /* Type */
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Request Message; Reads an analog input */
typedef struct {

  /* Mandatory */
  AdcRequestParametersType_v01 adc_param;
}adc_request_conversion_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Response Message; Reads an analog input */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}adc_request_conversion_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Request Message; Calibrates an analog input */
typedef struct {

  /* Mandatory */
  AdcRequestParametersType_v01 adc_param;
}adc_request_recalibration_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Response Message; Calibrates an analog input */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}adc_request_recalibration_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup adc_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t nDeviceIdx;

  /* !< Device index.   */
  uint32_t nChannelIdx;

  /* !< Channel index.  */
  uint32_t eFormat;

  /* !< Format of the result type.  */
  uint32_t nNumBatches;

  /* !< Number of batches to make; or set to 0 for
                                infinite batch requests.  */
  uint32_t nDurationUs;

  /* !< Time to continuously sample in microseconds per
                                batch; or set = 0 to use nNumConversions.  */
  uint32_t nNumConversions;

  /* !< Number of conversions to sample per batch;
                                or set = 0 to fill the buffer.  */
  uint32_t nPeriodUs;

  /* !< Period of time in microseconds to wait between
                                each conversion result.  */
  uint8_t isTimeStampRqd;
}AdcBatchParametersType_v01;  /* Type */
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Request Message; Starts multiple conversions on a channel */
typedef struct {

  /* Mandatory */
  AdcBatchParametersType_v01 adc_param;
}adc_request_batch_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup adc_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t nToken;
}AdcBatchStatusType_v01;  /* Type */
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Response Message; Starts multiple conversions on a channel */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  uint8_t adcBatchStatus_valid;  /**< Must be set to true if adcBatchStatus is being passed */
  AdcBatchStatusType_v01 adcBatchStatus;
}adc_request_batch_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Request Message; Stops the batch request with the specified token */
typedef struct {

  /* Mandatory */
  uint32_t nToken;
}adc_stop_batch_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Response Message; Stops the batch request with the specified token */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}adc_stop_batch_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Request Message; Notification to send asynchronous messages to the client. */
typedef struct {

  /* Optional */
  uint8_t req_valid;  /**< Must be set to true if req is being passed */
  uint32_t req;
  /**<   Enable asynchronous notification for the client. */
}adc_event_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Response Message; Notification to send asynchronous messages to the client. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}adc_event_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup adc_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t eStatus;
  /**<   Status of the conversion.  */

  uint32_t nToken;
  /**<   Token that identifies this conversion.  */

  uint32_t nDeviceIdx;
  /**<   Device index for this conversion.    */

  uint32_t nChannelIdx;
  /**<   Channel index for this conversion.   */

  int32_t nPhysical;
  /**<   Result in physical units (units depend on the BSP).  */

  uint32_t nPercent;
  /**<   Result as the percentage of reference voltage used
                             for conversion (0 = 0%, 65535 = 100%).  */

  uint32_t nMicrovolts;
  /**<   Result in microvolts.       */

  uint32_t nReserved;
  /**<   Reserved for internal use.  */
}AdcResultType_v01;  /* Type */
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Indication Message; This is an indication that the adc conversion is complete. */
typedef struct {

  /* Mandatory */
  /*  Memory Full Information */
  AdcResultType_v01 adcResult;
}adc_conversion_complete_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup adc_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t eStatus;

  uint32_t nToken;

  uint32_t nSamples;

  uint32_t aSamples_len;  /**< Must be set to # of elements in aSamples */
  int32_t aSamples[8192];
}AdcBatchResultType_v01;  /* Type */
/**
    @}
  */

/** @addtogroup adc_qmi_messages
    @{
  */
/** Indication Message; This is an indication that the batch data is ready. */
typedef struct {

  /* Mandatory */
  /*  Memory Full Information */
  AdcBatchResultType_v01 adcBatchPayload;
}adc_batch_data_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup adc_qmi_msg_ids
    @{
  */
#define QMI_ADC_GET_ADC_INPUT_PROPERTIES_REQ_V01 0x0001
#define QMI_ADC_GET_ADC_INPUT_PROPERTIES_RESP_V01 0x0001
#define QMI_ADC_REQUEST_CONVERSION_REQ_V01 0x0002
#define QMI_ADC_REQUEST_CONVERSION_RESP_V01 0x0002
#define QMI_ADC_REQUEST_RECALIBRATION_REQ_V01 0x0003
#define QMI_ADC_REQUEST_RECALIBRATION_RESP_V01 0x0003
#define QMI_ADC_REQUEST_BATCH_REQ_V01 0x0004
#define QMI_ADC_REQUEST_BATCH_RESP_V01 0x0004
#define QMI_ADC_STOP_BATCH_REQ_V01 0x0005
#define QMI_ADC_STOP_BATCH_RESP_V01 0x0005
#define QMI_ADC_EVENT_REPORT_REQ_V01 0x0006
#define QMI_ADC_EVENT_REPORT_RESP_V01 0x0007
#define QMI_ADC_CONVERSION_COMPLETE_IND_V01 0x0008
#define QMI_ADC_BATCH_DATA_IND_V01 0x0009
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro adc_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type adc_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define adc_get_service_object_v01( ) \
          adc_get_service_object_internal_v01( \
            ADC_V01_IDL_MAJOR_VERS, ADC_V01_IDL_MINOR_VERS, \
            ADC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

