#ifndef DPM_SERVICE_01_H
#define DPM_SERVICE_01_H
/**
  @file data_port_mapper_v01.h

  @brief This is the public header file which defines the dpm service Data structures.

  This header file defines the types and structures that were defined in
  dpm. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

  $Header: //source/qcom/qct/interfaces/qmi/dpm/main/latest/api/data_port_mapper_v01.h#1 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.5
   It was generated on: Tue Sep 24 2013 (Spin 0)
   From IDL File: data_port_mapper_v01.idl */

/** @defgroup dpm_qmi_consts Constant values defined in the IDL */
/** @defgroup dpm_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup dpm_qmi_enums Enumerated types used in QMI messages */
/** @defgroup dpm_qmi_messages Structures sent as QMI messages */
/** @defgroup dpm_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup dpm_qmi_accessor Accessor for QMI service object */
/** @defgroup dpm_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"
#include "data_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup dpm_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define DPM_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define DPM_V01_IDL_MINOR_VERS 0x00
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define DPM_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define DPM_V01_MAX_MESSAGE_ID 0x0021
/**
    @}
  */


/** @addtogroup dpm_qmi_consts
    @{
  */
#define QMI_DPM_PORT_MAX_NUM_V01 32
#define QMI_DPM_PORT_NAME_MAX_V01 32
/**
    @}
  */

/** @addtogroup dpm_qmi_aggregates
    @{
  */
typedef struct {

  char port_name[QMI_DPM_PORT_NAME_MAX_V01 + 1];
  /**<   String literal that will be converted to a port number.
  */

  data_ep_id_type_v01 default_ep_id;
  /**<   Identifies the default peripheral end point associated with the
       control port.
  */
}ctl_port_map_details_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dpm_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t consumer_pipe_num;
  /**<   Consumer pipe number for the hardware accelerated port.
  */

  uint32_t producer_pipe_num;
  /**<   Producer pipe number for the hardware accelerated port.
  */
}dpm_ep_pair_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dpm_qmi_aggregates
    @{
  */
typedef struct {

  data_ep_id_type_v01 ep_id;
  /**<   Identifies the peripheral end point associated with the hardware
       data port.
  */

  dpm_ep_pair_type_v01 hardware_ep_pair;
  /**<   Provided only if the data path of the RmNet instance is accelerated
       through hardware block. Conveys both consumer and producer pipes of
       hardware accelerated port facing the peripheral.
  */
}hardware_accl_port_details_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dpm_qmi_aggregates
    @{
  */
typedef struct {

  data_ep_id_type_v01 ep_id;
  /**<   Identifies the peripheral end point associated with the software
       data port.
  */

  char port_name[QMI_DPM_PORT_NAME_MAX_V01 + 1];
  /**<   String literal that will be converted to a port number.
  */
}software_data_port_map_details_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dpm_qmi_messages
    @{
  */
/** Request Message; Opens the control and data port associated with the data
           source/sink on modem. */
typedef struct {

  /* Optional */
  /*  Control Port List */
  uint8_t control_port_list_valid;  /**< Must be set to true if control_port_list is being passed */
  uint32_t control_port_list_len;  /**< Must be set to # of elements in control_port_list */
  ctl_port_map_details_v01 control_port_list[QMI_DPM_PORT_MAX_NUM_V01];
  /**<   Provides the configuration details specific to the control port.
  */

  /* Optional */
  /*  Hardware Data Port List */
  uint8_t hardware_data_port_list_valid;  /**< Must be set to true if hardware_data_port_list is being passed */
  uint32_t hardware_data_port_list_len;  /**< Must be set to # of elements in hardware_data_port_list */
  hardware_accl_port_details_v01 hardware_data_port_list[QMI_DPM_PORT_MAX_NUM_V01];
  /**<   Provides the configuration details specific to hardware accelerated port.
  */

  /* Optional */
  /*  Software Data Port List */
  uint8_t software_data_port_list_valid;  /**< Must be set to true if software_data_port_list is being passed */
  uint32_t software_data_port_list_len;  /**< Must be set to # of elements in software_data_port_list */
  software_data_port_map_details_v01 software_data_port_list[QMI_DPM_PORT_MAX_NUM_V01];
  /**<   Provides the configuration details specific to the software data port.
  */
}dpm_open_port_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dpm_qmi_messages
    @{
  */
/** Response Message; Opens the control and data port associated with the data
           source/sink on modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}dpm_open_port_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dpm_qmi_aggregates
    @{
  */
typedef struct {

  char port_name[QMI_DPM_PORT_NAME_MAX_V01 + 1];
  /**<   String literal that will be converted to a port number.
  */
}ctl_port_name_v01;  /* Type */
/**
    @}
  */

/** @addtogroup dpm_qmi_messages
    @{
  */
/** Request Message; Close the control and data port associated with the data
           source/sink on modem. */
typedef struct {

  /* Optional */
  /*  Control Port List */
  uint8_t control_port_list_valid;  /**< Must be set to true if control_port_list is being passed */
  uint32_t control_port_list_len;  /**< Must be set to # of elements in control_port_list */
  ctl_port_name_v01 control_port_list[QMI_DPM_PORT_MAX_NUM_V01];
  /**<   Provides the name of the control port to be closed
  */

  /* Optional */
  /*  Data Port List */
  uint8_t data_port_list_valid;  /**< Must be set to true if data_port_list is being passed */
  uint32_t data_port_list_len;  /**< Must be set to # of elements in data_port_list */
  data_ep_id_type_v01 data_port_list[QMI_DPM_PORT_MAX_NUM_V01];
  /**<   Provides the end point IDs associated with the data ports to be closed.
  */
}dpm_close_port_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup dpm_qmi_messages
    @{
  */
/** Response Message; Close the control and data port associated with the data
           source/sink on modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}dpm_close_port_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup dpm_qmi_msg_ids
    @{
  */
#define QMI_DPM_OPEN_PORT_REQ_V01 0x0020
#define QMI_DPM_OPEN_PORT_RESP_V01 0x0020
#define QMI_DPM_CLOSE_PORT_REQ_V01 0x0021
#define QMI_DPM_CLOSE_PORT_RESP_V01 0x0021
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro dpm_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type dpm_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define dpm_get_service_object_v01( ) \
          dpm_get_service_object_internal_v01( \
            DPM_V01_IDL_MAJOR_VERS, DPM_V01_IDL_MINOR_VERS, \
            DPM_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

