#ifndef PDC_SERVICE_01_H
#define PDC_SERVICE_01_H
/**
  @file persistent_device_configuration_v01.h
  
  @brief This is the public header file which defines the pdc service Data structures.

  This header file defines the types and structures that were defined in 
  pdc. It contains the constant values defined, enums, structures,
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
  Qualcomm Technologies Proprietary and Confidential.


  $Header: //components/rel/qmimsgs.mpss/3.4.1/pdc/api/persistent_device_configuration_v01.h#2 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It was generated on: Tue Jul 23 2013 (Spin 2)
   From IDL File: persistent_device_configuration_v01.idl */

/** @defgroup pdc_qmi_consts Constant values defined in the IDL */
/** @defgroup pdc_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup pdc_qmi_enums Enumerated types used in QMI messages */
/** @defgroup pdc_qmi_messages Structures sent as QMI messages */
/** @defgroup pdc_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup pdc_qmi_accessor Accessor for QMI service object */
/** @defgroup pdc_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup pdc_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define PDC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define PDC_V01_IDL_MINOR_VERS 0x05
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define PDC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define PDC_V01_MAX_MESSAGE_ID 0x002B;
/** 
    @} 
  */


/** @addtogroup pdc_qmi_consts 
    @{ 
  */
#define PDC_CONFIG_ID_SIZE_MAX_V01 124
#define PDC_CONFIG_DESC_SIZE_MAX_V01 255
#define PDC_CONFIG_LIST_SIZE_MAX_V01 25
#define PDC_CONFIG_FRAME_SIZE_MAX_V01 32768
/**
    @}
  */

/** @addtogroup pdc_qmi_enums
    @{
  */
typedef enum {
  PDC_CONFIG_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  PDC_CONFIG_TYPE_MODEM_PLATFORM_V01 = 0x00, /**<  Modem platform configuration type  */
  PDC_CONFIG_TYPE_MODEM_SW_V01 = 0x01, /**<  Modem software configuration type  */
  PDC_CONFIG_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}pdc_config_type_enum_v01;
/**
    @}
  */

/** @addtogroup pdc_qmi_aggregates
    @{
  */
typedef struct {

  /*  Configuration Type */
  pdc_config_type_enum_v01 config_type;
  /**<   Type of configuration. Values: \n
      - PDC_CONFIG_TYPE_MODEM_PLATFORM (0x00) --  Modem platform configuration type 
      - PDC_CONFIG_TYPE_MODEM_SW (0x01) --  Modem software configuration type 

 Note: All other values are reserved, and the service responds with 
 QMI_ERR_NOT_SUPPORTED.
 */

  /*  Configuration ID */
  uint32_t config_id_len;  /**< Must be set to # of elements in config_id */
  uint8_t config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
  /**<   Unique ID for the configuration.
  */
}pdc_config_info_req_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup pdc_qmi_aggregates
    @{
  */
typedef struct {

  /*  Configuration Type */
  pdc_config_type_enum_v01 config_type;
  /**<   Type of configuration. Values: \n
      - PDC_CONFIG_TYPE_MODEM_PLATFORM (0x00) --  Modem platform configuration type 
      - PDC_CONFIG_TYPE_MODEM_SW (0x01) --  Modem software configuration type 

 Note: Clients are to ignore configuration types that are invalid or 
 unknown.
 */

  /*  Configuration ID */
  uint32_t config_id_len;  /**< Must be set to # of elements in config_id */
  uint8_t config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
  /**<   Unique ID for the configuration.
  */
}pdc_config_info_resp_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup pdc_qmi_aggregates
    @{
  */
typedef struct {

  /*  Configuration Type */
  pdc_config_type_enum_v01 config_type;
  /**<   Type of configuration. Values: \n
      - PDC_CONFIG_TYPE_MODEM_PLATFORM (0x00) --  Modem platform configuration type 
      - PDC_CONFIG_TYPE_MODEM_SW (0x01) --  Modem software configuration type 

 Note: All other values are reserved, and the service responds with 
 QMI_ERR_NOT_SUPPORTED.
 */

  /*  Configuration ID */
  uint32_t config_id_len;  /**< Must be set to # of elements in config_id */
  uint8_t config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
  /**<   Unique ID for the configuration.
  */

  /*  Total Configuration Size */
  uint32_t total_config_size;
  /**<   Total size of the configuration.
  */

  /*  Configuration Data Frame */
  uint32_t config_frame_len;  /**< Must be set to # of elements in config_frame */
  uint8_t config_frame[PDC_CONFIG_FRAME_SIZE_MAX_V01];
  /**<   Next frame of the configuration data to be stored.
  */
}pdc_load_config_info_type_v01;  /* Type */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}pdc_reset_req_msg_v01;

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Response Message; Resets the PDC state variables of the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described 
                          in the error codes section of each message 
                          definition.
  */
}pdc_reset_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Request Message; Maintains control point registration for service indications. */
typedef struct {

  /* Optional */
  /*  Component Configuration */
  uint8_t reg_config_change_valid;  /**< Must be set to true if reg_config_change is being passed */
  uint8_t reg_config_change;
  /**<   Controls the reporting of QMI_PDC_CONFIG_CHANGE_IND. Values: \n
       - 0x00 -- Disable (default)  \n
       - 0x01 -- Enable
   */
}pdc_indication_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Response Message; Maintains control point registration for service indications. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described 
                          in the error codes section of each message 
                          definition.
  */
}pdc_indication_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Indication Message; Indication that a configuration has changed. */
typedef struct {

  /* Mandatory */
  /*  New Configuration */
  pdc_config_info_resp_type_v01 new_config_info;
}pdc_config_change_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Request Message; Gets the active configuration. */
typedef struct {

  /* Mandatory */
  /*  Configuration Type */
  pdc_config_type_enum_v01 config_type;
  /**<   Type of configuration. Values: \n
      - PDC_CONFIG_TYPE_MODEM_PLATFORM (0x00) --  Modem platform configuration type 
      - PDC_CONFIG_TYPE_MODEM_SW (0x01) --  Modem software configuration type 

 Note: All other values are reserved, and the service responds with 
 QMI_ERR_NOT_SUPPORTED.
 */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token used to identify the indication sent when the request is complete.
  */
}pdc_get_selected_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Response Message; Gets the active configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described 
                          in the error codes section of each message 
                          definition.
  */
}pdc_get_selected_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Indication Message; Indication with the read selected configuration result. */
typedef struct {

  /* Mandatory */
  /*  Indication Error Code */
  qmi_error_type_v01 error;
  /**<   Error code. Values: 
       \begin{itemize1}
       \item 0x0000 -- QMI_ERR_NONE              -- Success 
       \item 0x0003 -- QMI_ERR_INTERNAL          -- Internal error 
       \item 0x0010 -- QMI_ERR_NOT_ PROVISIONED   -- Specified configuration was 
                                                     not found
       \vspace{-0.18in}
       \end{itemize1}
  */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token passed in the request that generated this indication.
  */

  /* Optional */
  /*  Active Configuration ID */
  uint8_t active_config_id_valid;  /**< Must be set to true if active_config_id is being passed */
  uint32_t active_config_id_len;  /**< Must be set to # of elements in active_config_id */
  uint8_t active_config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
  /**<   Unique ID for the active configuration.
  */

  /* Optional */
  /*  Pending Configuration ID */
  uint8_t pending_config_id_valid;  /**< Must be set to true if pending_config_id is being passed */
  uint32_t pending_config_id_len;  /**< Must be set to # of elements in pending_config_id */
  uint8_t pending_config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
  /**<   Unique ID for the pending configuration.
  */
}pdc_get_selected_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Request Message; Sets an available configuration for the device. */
typedef struct {

  /* Mandatory */
  /*  New Configuration */
  pdc_config_info_req_type_v01 new_config_info;

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token used to identify the indication sent when the request is complete.
  */
}pdc_set_selected_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Response Message; Sets an available configuration for the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described 
                          in the error codes section of each message 
                          definition.
  */
}pdc_set_selected_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Indication Message; Indication with the set selected configuration result.  */
typedef struct {

  /* Mandatory */
  /*  Indication Error Code */
  qmi_error_type_v01 error;
  /**<   Error code. Values: 
       \begin{itemize1}
       \item 0x0000 -- QMI_ERR_NONE        -- Success 
       \item 0x0003 -- QMI_ERR_INTERNAL    -- Internal error 
       \item 0x0029 -- QMI_ERR_INVALID_ID  -- Specified argument already exists
       \vspace{-0.18in}
       \end{itemize1}
  */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token passed in the request that generated.
  */
}pdc_set_selected_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Request Message; Lists the configurations currently available on the device. */
typedef struct {

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token used to identify the indication sent when the request is complete.
  */

  /* Optional */
  /*  List Configuration Type */
  uint8_t config_type_valid;  /**< Must be set to true if config_type is being passed */
  pdc_config_type_enum_v01 config_type;
  /**<   Type of configuration. Values: \n
      - PDC_CONFIG_TYPE_MODEM_PLATFORM (0x00) --  Modem platform configuration type 
      - PDC_CONFIG_TYPE_MODEM_SW (0x01) --  Modem software configuration type 

 Note: All other values are reserved, and the service responds with 
 QMI_ERR_NOT_SUPPORTED.
 */
}pdc_list_configs_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Response Message; Lists the configurations currently available on the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described 
                          in the error codes section of each message 
                          definition.
  */
}pdc_list_configs_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Indication Message; Indication with the list configuration result. */
typedef struct {

  /* Mandatory */
  /*  Indication Error Code */
  qmi_error_type_v01 error;
  /**<   Error code. Values: 
       \begin{itemize1}
       \item 0x0000 -- QMI_ERR_NONE               -- Success 
       \item 0x0003 -- QMI_ERR_INTERNAL           -- Internal error
       \vspace{-0.18in}
       \end{itemize1}
  */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token passed in the request that generated.
  */

  /* Optional */
  /*  Configuration List */
  uint8_t config_list_valid;  /**< Must be set to true if config_list is being passed */
  uint32_t config_list_len;  /**< Must be set to # of elements in config_list */
  pdc_config_info_resp_type_v01 config_list[PDC_CONFIG_LIST_SIZE_MAX_V01];
}pdc_list_configs_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Request Message; Deletes a configuration from the device. */
typedef struct {

  /* Mandatory */
  /*  Configuration Type */
  pdc_config_type_enum_v01 config_type;
  /**<   Type of configuration. Values: \n
      - PDC_CONFIG_TYPE_MODEM_PLATFORM (0x00) --  Modem platform configuration type 
      - PDC_CONFIG_TYPE_MODEM_SW (0x01) --  Modem software configuration type 

 Note: All other values are reserved, and the service responds with 
 QMI_ERR_NOT_SUPPORTED.
 */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token used to identify the indication sent when the request is complete.
  */

  /* Optional */
  /*  Configuration ID */
  uint8_t config_id_valid;  /**< Must be set to true if config_id is being passed */
  uint32_t config_id_len;  /**< Must be set to # of elements in config_id */
  uint8_t config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
  /**<   Unique ID for the configuration.
  */
}pdc_delete_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Response Message; Deletes a configuration from the device. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described 
                          in the error codes section of each message 
                          definition.
  */
}pdc_delete_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Indication Message; Indication with the delete configuration result.  */
typedef struct {

  /* Mandatory */
  /*  Indication Error Code */
  qmi_error_type_v01 error;
  /**<   Error code. Values: 
       \begin{itemize1}
       \item 0x0000 -- QMI_ERR_NONE        -- Success 
       \item 0x0003 -- QMI_ERR_INTERNAL    -- Internal error 
       \item 0x0029 -- QMI_ERR_INVALID_ID  -- Specified argument already exists
       \vspace{-0.18in}
       \end{itemize1}
  */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token passed in the request that generated.
  */
}pdc_delete_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Request Message; Loads the specified configuration to device memory. */
typedef struct {

  /* Mandatory */
  /*  Load Configuration */
  pdc_load_config_info_type_v01 load_config_info;

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token used to identify the indication sent when the request is complete.
  */
}pdc_load_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Response Message; Loads the specified configuration to device memory. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described 
                          in the error codes section of each message 
                          definition.
  */

  /* Optional */
  /*  Frame Data Reset */
  uint8_t frame_data_reset_valid;  /**< Must be set to true if frame_data_reset is being passed */
  uint8_t frame_data_reset;
  /**<   Frame data has been reset due to an error. Value:\n
       - 0x01 -- Frame data reset
   */
}pdc_load_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Indication Message; Indication with the load configuration result.  */
typedef struct {

  /* Mandatory */
  /*  Indication Error Code */
  qmi_error_type_v01 error;
  /**<   Error code. Values: 
       \begin{itemize1}
       \item 0x0000 -- QMI_ERR_NONE         -- Success 
       \item 0x0002 -- QMI_ERR_NO_MEMORY    -- Insufficient memory to store 
                                               the configuration 
       \item 0x0003 -- QMI_ERR_INTERNAL     -- Internal error 
       \item 0x0022 -- QMI_ERR_ AUTHENTICATION_FAILED -- Configuration 
                                                         authentication failed 
       \item 0x0029 -- QMI_ERR_INVALID_ID   -- Specified argument already 
                                               exists 
       \item 0x0030 -- QMI_ERR_INVALID_ARG  -- Sum of the frame data is  
                                               greater than the total
       \vspace{-0.18in}
       \end{itemize1}
  */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token passed in the request that generated.
  */

  /* Optional */
  /*  Received Configuration Size */
  uint8_t received_config_size_valid;  /**< Must be set to true if received_config_size is being passed */
  uint32_t received_config_size;
  /**<   Total size of the configuration data received.
  */

  /* Optional */
  /*  Remaining Configuration Size */
  uint8_t remaining_config_size_valid;  /**< Must be set to true if remaining_config_size is being passed */
  uint32_t remaining_config_size;
  /**<   Total size of the configuration data remaining.
  */

  /* Optional */
  /*  Frame Data Reset */
  uint8_t frame_data_reset_valid;  /**< Must be set to true if frame_data_reset is being passed */
  uint8_t frame_data_reset;
  /**<   Frame data has been reset due to an error. Value:\n 
       - 0x01 -- Frame data reset
   */
}pdc_load_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Request Message; Activates a pending configuration for the component. */
typedef struct {

  /* Mandatory */
  /*  Configuration Type */
  pdc_config_type_enum_v01 config_type;
  /**<   Type of configuration. Values: \n
      - PDC_CONFIG_TYPE_MODEM_PLATFORM (0x00) --  Modem platform configuration type 
      - PDC_CONFIG_TYPE_MODEM_SW (0x01) --  Modem software configuration type 

 Note: All other values are reserved, and the service responds with 
 QMI_ERR_NOT_SUPPORTED.
 */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token used to identify the indication sent when the request is complete.
  */
}pdc_activate_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Response Message; Activates a pending configuration for the component. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described 
                          in the error codes section of each message 
                          definition.
  */
}pdc_activate_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Indication Message; Indication with the activate configuration result.  */
typedef struct {

  /* Mandatory */
  /*  Indication Error Code */
  qmi_error_type_v01 error;
  /**<   Error code. Values: 
       \begin{itemize1}
       \item 0x0000 -- QMI_ERR_NONE             -- Success 
       \item 0x0003 -- QMI_ERR_INTERNAL         -- Internal error 
       \item 0x0010 -- QMI_ERR_NOT_ PROVISIONED -- Specified configuration was
                                                   not found
       \vspace{-0.18in}
       \end{itemize1}
  */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token passed in the request that generated.
  */
}pdc_activate_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Request Message; Queries additional information for a configuration. */
typedef struct {

  /* Mandatory */
  /*  Configuration ID */
  pdc_config_info_req_type_v01 new_config_info;

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token used to identify the indication sent when the request is complete.
  */
}pdc_get_config_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Response Message; Queries additional information for a configuration. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described 
                          in the error codes section of each message 
                          definition.
  */
}pdc_get_config_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Indication Message; Indication with the read configuration information result.  */
typedef struct {

  /* Mandatory */
  /*  Indication Error Code */
  qmi_error_type_v01 error;
  /**<   Error code. Values: 
       \begin{itemize1}
       \item 0x0000 -- QMI_ERR_NONE        -- Success 
       \item 0x0003 -- QMI_ERR_INTERNAL    -- Internal error 
       \item 0x0029 -- QMI_ERR_INVALID_ID  -- Specified argument already exists
       \vspace{-0.18in}
       \end{itemize1}
  */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token passed in the request that generated.
  */

  /* Optional */
  /*  Configuration Size */
  uint8_t config_size_valid;  /**< Must be set to true if config_size is being passed */
  uint32_t config_size;
  /**<   Size of the configuration in memory.
  */

  /* Optional */
  /*  Configuration Description */
  uint8_t config_desc_valid;  /**< Must be set to true if config_desc is being passed */
  uint32_t config_desc_len;  /**< Must be set to # of elements in config_desc */
  uint8_t config_desc[PDC_CONFIG_DESC_SIZE_MAX_V01];
  /**<   ASCII string containing the description of the configuration read from 
       memory.
  */

  /* Optional */
  /*  Configuration Version */
  uint8_t config_version_valid;  /**< Must be set to true if config_version is being passed */
  uint32_t config_version;
  /**<   Version of the configuration in memory.
  */
}pdc_get_config_info_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Request Message; Queries the maximum and current sizes for each configuration memory
           store. */
typedef struct {

  /* Mandatory */
  /*  Configuration Type */
  pdc_config_type_enum_v01 config_type;
  /**<   Type of configuration. Values: \n
      - PDC_CONFIG_TYPE_MODEM_PLATFORM (0x00) --  Modem platform configuration type 
      - PDC_CONFIG_TYPE_MODEM_SW (0x01) --  Modem software configuration type 

 Note: All other values are reserved, and the service responds with 
 QMI_ERR_NOT_SUPPORTED.
 */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token used to identify the indication sent when the request is complete.
  */
}pdc_get_config_limits_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Response Message; Queries the maximum and current sizes for each configuration memory
           store. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described 
                          in the error codes section of each message 
                          definition.
  */
}pdc_get_config_limits_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Indication Message; Indication with the read configuration limits result.  */
typedef struct {

  /* Mandatory */
  /*  Indication Error Code */
  qmi_error_type_v01 error;
  /**<   Error code. Values: 
       \begin{itemize1}
       \item 0x0000 -- QMI_ERR_NONE               -- Success 
       \item 0x0003 -- QMI_ERR_INTERNAL           -- Internal error
       \vspace{-0.18in}
       \end{itemize1}
  */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token passed in the request that generated.
  */

  /* Optional */
  /*  Maximum Configuration Size */
  uint8_t max_config_size_valid;  /**< Must be set to true if max_config_size is being passed */
  uint64_t max_config_size;
  /**<   Maximum size of the configurations in memory.
  */

  /* Optional */
  /*  Current Configuration Size */
  uint8_t curr_config_size_valid;  /**< Must be set to true if curr_config_size is being passed */
  uint64_t curr_config_size;
  /**<   Current size of the configurations in memory.
  */
}pdc_get_config_limits_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Request Message; Gets the default configuration information for a specified
           configuration type currently embedded with the loaded image. */
typedef struct {

  /* Mandatory */
  /*  Configuration Type */
  pdc_config_type_enum_v01 config_type;
  /**<   Type of configuration. Values: \n
      - PDC_CONFIG_TYPE_MODEM_PLATFORM (0x00) --  Modem platform configuration type 
      - PDC_CONFIG_TYPE_MODEM_SW (0x01) --  Modem software configuration type 

 Note: All other values are reserved, and the service responds with 
 QMI_ERR_NOT_SUPPORTED.
 */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token used to identify the indication sent when the request is complete.
  */
}pdc_get_default_config_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Response Message; Gets the default configuration information for a specified
           configuration type currently embedded with the loaded image. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described 
                          in the error codes section of each message 
                          definition.
  */
}pdc_get_default_config_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Indication Message; Indication with the default configuration result information. */
typedef struct {

  /* Mandatory */
  /*  Indication Error Code */
  qmi_error_type_v01 error;
  /**<   Error code. Values: 
       \begin{itemize1}
       \item 0x0000 -- QMI_ERR_NONE               -- Success 
       \item 0x0003 -- QMI_ERR_INTERNAL           -- Internal error
       \vspace{-0.18in}
       \end{itemize1}
  */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token passed in the request that generated.
  */

  /* Optional */
  /*  Configuration Version */
  uint8_t config_version_valid;  /**< Must be set to true if config_version is being passed */
  uint32_t config_version;
  /**<   Version of the configuration in memory.
  */

  /* Optional */
  /*  Configuration Size */
  uint8_t config_size_valid;  /**< Must be set to true if config_size is being passed */
  uint32_t config_size;
  /**<   Size of the configuration in memory.
  */

  /* Optional */
  /*  Configuration Description */
  uint8_t config_desc_valid;  /**< Must be set to true if config_desc is being passed */
  uint32_t config_desc_len;  /**< Must be set to # of elements in config_desc */
  uint8_t config_desc[PDC_CONFIG_DESC_SIZE_MAX_V01];
  /**<   ASCII string containing the description of the configuration read from 
       memory.
  */
}pdc_get_default_config_info_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Request Message; Deactivates an active configuration for the component. */
typedef struct {

  /* Mandatory */
  /*  Configuration Type */
  pdc_config_type_enum_v01 config_type;
  /**<   Type of configuration. Values: \n
      - PDC_CONFIG_TYPE_MODEM_PLATFORM (0x00) --  Modem platform configuration type 
      - PDC_CONFIG_TYPE_MODEM_SW (0x01) --  Modem software configuration type 

 Note: All other values are reserved, and the service responds with 
 QMI_ERR_NOT_SUPPORTED.
 */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token used to identify the indication sent when the request is complete.
  */
}pdc_deactivate_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Response Message; Deactivates an active configuration for the component. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:\n
       qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE.\n
       qmi_error_type  -- Error code. Possible error code values are described 
                          in the error codes section of each message 
                          definition.
  */
}pdc_deactivate_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup pdc_qmi_messages
    @{
  */
/** Indication Message; Indication with the deactivate configuration result.  */
typedef struct {

  /* Mandatory */
  /*  Indication Error Code */
  qmi_error_type_v01 error;
  /**<   Error code. Values: 
       \begin{itemize1}
       \item 0x0000 -- QMI_ERR_NONE             -- Success 
       \item 0x0003 -- QMI_ERR_INTERNAL         -- Internal error 
       \item 0x0010 -- QMI_ERR_NOT_ PROVISIONED -- Specified configuration was
                                                   not found
       \vspace{-0.18in}
       \end{itemize1}
  */

  /* Optional */
  /*  Indication Token */
  uint8_t ind_token_valid;  /**< Must be set to true if ind_token is being passed */
  uint32_t ind_token;
  /**<   Token passed in the request that generated.
  */
}pdc_deactivate_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup pdc_qmi_msg_ids
    @{
  */
#define QMI_PDC_RESET_REQ_V01 0x0000
#define QMI_PDC_RESET_RESP_V01 0x0000
#define QMI_PDC_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_PDC_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_PDC_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_PDC_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_PDC_INDICATION_REGISTER_REQ_V01 0x0020
#define QMI_PDC_INDICATION_REGISTER_RESP_V01 0x0020
#define QMI_PDC_CONFIG_CHANGE_IND_V01 0x0021
#define QMI_PDC_GET_SELECTED_CONFIG_REQ_V01 0x0022
#define QMI_PDC_GET_SELECTED_CONFIG_RESP_V01 0x0022
#define QMI_PDC_GET_SELECTED_CONFIG_IND_V01 0x0022
#define QMI_PDC_SET_SELECTED_CONFIG_REQ_V01 0x0023
#define QMI_PDC_SET_SELECTED_CONFIG_RESP_V01 0x0023
#define QMI_PDC_SET_SELECTED_CONFIG_IND_V01 0x0023
#define QMI_PDC_LIST_CONFIGS_REQ_V01 0x0024
#define QMI_PDC_LIST_CONFIGS_RESP_V01 0x0024
#define QMI_PDC_LIST_CONFIGS_IND_V01 0x0024
#define QMI_PDC_DELETE_CONFIG_REQ_V01 0x0025
#define QMI_PDC_DELETE_CONFIG_RESP_V01 0x0025
#define QMI_PDC_DELETE_CONFIG_IND_V01 0x0025
#define QMI_PDC_LOAD_CONFIG_REQ_V01 0x0026
#define QMI_PDC_LOAD_CONFIG_RESP_V01 0x0026
#define QMI_PDC_LOAD_CONFIG_IND_V01 0x0026
#define QMI_PDC_ACTIVATE_CONFIG_REQ_V01 0x0027
#define QMI_PDC_ACTIVATE_CONFIG_RESP_V01 0x0027
#define QMI_PDC_ACTIVATE_CONFIG_IND_V01 0x0027
#define QMI_PDC_GET_CONFIG_INFO_REQ_V01 0x0028
#define QMI_PDC_GET_CONFIG_INFO_RESP_V01 0x0028
#define QMI_PDC_GET_CONFIG_INFO_IND_V01 0x0028
#define QMI_PDC_GET_CONFIG_LIMITS_REQ_V01 0x0029
#define QMI_PDC_GET_CONFIG_LIMITS_RESP_V01 0x0029
#define QMI_PDC_GET_CONFIG_LIMITS_IND_V01 0x0029
#define QMI_PDC_GET_DEFAULT_CONFIG_INFO_REQ_V01 0x002A
#define QMI_PDC_GET_DEFAULT_CONFIG_INFO_RESP_V01 0x002A
#define QMI_PDC_GET_DEFAULT_CONFIG_INFO_IND_V01 0x002A
#define QMI_PDC_DEACTIVATE_CONFIG_REQ_V01 0x002B
#define QMI_PDC_DEACTIVATE_CONFIG_RESP_V01 0x002B
#define QMI_PDC_DEACTIVATE_CONFIG_IND_V01 0x002B
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro pdc_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type pdc_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define pdc_get_service_object_v01( ) \
          pdc_get_service_object_internal_v01( \
            PDC_V01_IDL_MAJOR_VERS, PDC_V01_IDL_MINOR_VERS, \
            PDC_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

