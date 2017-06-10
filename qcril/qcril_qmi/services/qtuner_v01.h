#ifndef QTUNER_SERVICE_H
#define QTUNER_SERVICE_H
/**
  @file qtuner_v01.h

  @brief This is the public header file which defines the Qtuner service Data structures.

  This header file defines the types and structures that were defined in
  Qtuner. It contains the constant values defined, enums, structures,
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

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 5.5
   It was generated on: Fri Sep 28 2012
   From IDL File: qtuner_v01.idl */

/** @defgroup Qtuner_qmi_consts Constant values defined in the IDL */
/** @defgroup Qtuner_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup Qtuner_qmi_enums Enumerated types used in QMI messages */
/** @defgroup Qtuner_qmi_messages Structures sent as QMI messages */
/** @defgroup Qtuner_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup Qtuner_qmi_accessor Accessor for QMI service object */
/** @defgroup Qtuner_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup Qtuner_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define QTUNER_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define QTUNER_V01_IDL_MINOR_VERS 0x00
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define QTUNER_V01_IDL_TOOL_VERS 0x05
/** Maximum Defined Message ID */
#define QTUNER_V01_MAX_MESSAGE_ID 0x0022;
/**
    @}
  */


/** @addtogroup Qtuner_qmi_consts
    @{
  */
#define Qtuner_FREE_SPACE_SCENARIO_V01 0
#define Qtuner_CONCURRENT_SCENARIOS_MAX_V01 32
#define Qtuner_OEM_STR_LENGTH_V01 64
/**
    @}
  */

/** @addtogroup Qtuner_qmi_messages
    @{
  */
/** Request Message;  */
typedef struct {

  /* Mandatory */
  /*  Array of scenario numbers from APPS */
  uint32_t scenarios_len;  /**< Must be set to # of elements in scenarios */
  uint32_t scenarios[Qtuner_CONCURRENT_SCENARIOS_MAX_V01];
  /**<   Qtuner Scenario numbers detected in APPS */
}Qtuner_set_scenario_req_v01;  /* Message */
/**
    @}
  */

/*
 * Qtuner_set_scenario_resp is empty
 * typedef struct {
 * }Qtuner_set_scenario_resp_v01;
 */

/*
 * Qtuner_get_rfm_scenarios_req is empty
 * typedef struct {
 * }Qtuner_get_rfm_scenarios_req_v01;
 */

/** @addtogroup Qtuner_qmi_messages
    @{
  */
/** Response Message;  */
typedef struct {

  /* Optional */
  uint8_t active_scenarios_valid;  /**< Must be set to true if active_scenarios is being passed */
  uint32_t active_scenarios_len;  /**< Must be set to # of elements in active_scenarios */
  uint32_t active_scenarios[Qtuner_CONCURRENT_SCENARIOS_MAX_V01];
}Qtuner_get_rfm_scenarios_resp_v01;  /* Message */
/**
    @}
  */

/*
 * Qtuner_get_provisioned_table_revision_req is empty
 * typedef struct {
 * }Qtuner_get_provisioned_table_revision_req_v01;
 */

/** @addtogroup Qtuner_qmi_messages
    @{
  */
/**  Message;  */
typedef struct {

  /* Optional */
  uint8_t provisioned_table_revision_valid;  /**< Must be set to true if provisioned_table_revision is being passed */
  uint32_t provisioned_table_revision;

  /* Optional */
  uint8_t provisioned_table_OEM_valid;  /**< Must be set to true if provisioned_table_OEM is being passed */
  uint32_t provisioned_table_OEM_len;  /**< Must be set to # of elements in provisioned_table_OEM */
  uint16_t provisioned_table_OEM[Qtuner_OEM_STR_LENGTH_V01];
}Qtuner_get_provisioned_table_revision_resp_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup Qtuner_qmi_msg_ids
    @{
  */
#define QMI_Qtuner_SET_RFM_SCENARIO_REQ_V01 0x0020
#define QMI_Qtuner_SET_RFM_SCENARIO_RESP_V01 0x0020
#define QMI_Qtuner_GET_RFM_SCENARIO_REQ_V01 0x0021
#define QMI_Qtuner_GET_RFM_SCENARIO_RESP_V01 0x0021
#define QMI_Qtuner_GET_PROVISIONED_TABLE_REVISION_REQ_V01 0x0022
#define QMI_Qtuner_GET_PROVISIONED_TABLE_REVISION_RESP_V01 0x0022
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro Qtuner_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type Qtuner_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define Qtuner_get_service_object_v01( ) \
          Qtuner_get_service_object_internal_v01( \
            QTUNER_V01_IDL_MAJOR_VERS, QTUNER_V01_IDL_MINOR_VERS, \
            QTUNER_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

