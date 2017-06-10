#ifndef RMTFS_SERVICE_01_H
#define RMTFS_SERVICE_01_H
/**
  @file remote_storage_v01.h

  @brief This is the public header file which defines the rmtfs service Data structures.

  This header file defines the types and structures that were defined in
  rmtfs. It contains the constant values defined, enums, structures,
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

  $Header: //source/qcom/qct/interfaces/qmi/rmtfs/main/latest/api/remote_storage_v01.h#5 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2
   It was generated on: Wed Jul 24 2013 (Spin 2)
   From IDL File: remote_storage_v01.idl */

/** @defgroup rmtfs_qmi_consts Constant values defined in the IDL */
/** @defgroup rmtfs_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup rmtfs_qmi_enums Enumerated types used in QMI messages */
/** @defgroup rmtfs_qmi_messages Structures sent as QMI messages */
/** @defgroup rmtfs_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup rmtfs_qmi_accessor Accessor for QMI service object */
/** @defgroup rmtfs_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup rmtfs_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define RMTFS_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define RMTFS_V01_IDL_MINOR_VERS 0x02
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define RMTFS_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define RMTFS_V01_MAX_MESSAGE_ID 0x0006;
/**
    @}
  */


/** @addtogroup rmtfs_qmi_consts
    @{
  */

/**  Max file path size for open request */
#define RMTFS_MAX_FILE_PATH_V01 255

/**  Max iovec entries for read or write iovec requests */
#define RMTFS_MAX_IOVEC_ENTRIES_V01 255

/**  Max caller ID's that can be specified in force sync indication */
#define RMTFS_MAX_CALLER_ID_V01 10
/**
    @}
  */

/** @addtogroup rmtfs_qmi_messages
    @{
  */
/** Request Message; Allows the client to send an open request to the server and receive a
           response to the request. */
typedef struct {

  /* Mandatory */
  /*  Path to the Partition */
  char path[RMTFS_MAX_FILE_PATH_V01 + 1];
  /**<   Path to the partition (on the eMMC device) that the modem requests to open. */
}rmtfs_open_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rmtfs_qmi_messages
    @{
  */
/** Response Message; Allows the client to send an open request to the server and receive a
           response to the request. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Client ID */
  uint8_t caller_id_valid;  /**< Must be set to true if caller_id is being passed */
  uint32_t caller_id;
  /**<   Caller ID. */
}rmtfs_open_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rmtfs_qmi_messages
    @{
  */
/** Request Message; Allows the client to send a close request to the server and receive
           a response to the request. */
typedef struct {

  /* Mandatory */
  /*  Client ID */
  uint32_t caller_id;
  /**<   Caller ID.*/
}rmtfs_close_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rmtfs_qmi_messages
    @{
  */
/** Response Message; Allows the client to send a close request to the server and receive
           a response to the request. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}rmtfs_close_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rmtfs_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t sector_addr;
  /**<   Sector address to access. */

  uint32_t data_phy_addr_offset;
  /**<   Physical address offset of the data. */

  uint32_t num_sector;
  /**<   Number of sectors from which to read or write. */
}rmtfs_iovec_desc_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup rmtfs_qmi_enums
    @{
  */
typedef enum {
  RMTFS_DIRECTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  RMTFS_DIRECTION_READ_V01 = 0x00,
  RMTFS_DIRECTION_WRITE_V01 = 0x01,
  RMTFS_DIRECTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}rmtfs_direction_enum_v01;
/**
    @}
  */

/** @addtogroup rmtfs_qmi_messages
    @{
  */
/** Request Message; Allows the client to send a write IOVEC request to the server and
           receive a response to the request. */
typedef struct {

  /* Mandatory */
  /*  Client ID */
  uint32_t caller_id;
  /**<   Caller ID. */

  /* Mandatory */
  /*  Direction of Operation */
  rmtfs_direction_enum_v01 direction;
  /**<   Indicates the direction of data transfer. Values: \n
       - 0x00 -- Data is read from the sector \n
       - 0x01 -- Data is written to the sector */

  /* Mandatory */
  /*  IOVEC Structure Array */
  uint32_t iovec_struct_len;  /**< Must be set to # of elements in iovec_struct */
  rmtfs_iovec_desc_type_v01 iovec_struct[RMTFS_MAX_IOVEC_ENTRIES_V01];

  /* Mandatory */
  /*  Force Sync Indicator */
  uint8_t is_force_sync;
  /**<   Indicates whether a force sync is to be initiated. Values: \n
       - 0x00 -- Force sync is to be initiated \n
       - 0x01 -- Force sync is not to be initiated */
}rmtfs_rw_iovec_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rmtfs_qmi_messages
    @{
  */
/** Response Message; Allows the client to send a write IOVEC request to the server and
           receive a response to the request. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}rmtfs_rw_iovec_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rmtfs_qmi_messages
    @{
  */
/** Request Message; Allows the client to send an allocation buffer request to the server
           and receive a response to the request. */
typedef struct {

  /* Mandatory */
  /*  Client ID */
  uint32_t caller_id;
  /**<   Caller ID. */

  /* Mandatory */
  /*  Buffer Size */
  uint32_t buff_size;
  /**<   Size of the buffer to be allocated. */
}rmtfs_alloc_buff_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rmtfs_qmi_messages
    @{
  */
/** Response Message; Allows the client to send an allocation buffer request to the server
           and receive a response to the request. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Buffer Address */
  uint8_t buff_address_valid;  /**< Must be set to true if buff_address is being passed */
  uint64_t buff_address;
  /**<   Address of the buffer that is allocated. */
}rmtfs_alloc_buff_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rmtfs_qmi_messages
    @{
  */
/** Request Message; Allows the client to get the device error for the last data request. */
typedef struct {

  /* Mandatory */
  /*  Client ID */
  uint32_t caller_id;
  /**<   Caller ID. */
}rmtfs_get_dev_error_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rmtfs_qmi_enums
    @{
  */
typedef enum {
  RMTFS_DEV_ERROR_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  RMTFS_DEV_ERROR_NONE_V01 = 0,
  RMTFS_DEV_ERROR_DEVICE_NOT_FOUND_V01 = 1,
  RMTFS_DEV_ERROR_PARTITION_NOT_FOUND_V01 = 2,
  RMTS_DEV_ERROR_RW_FAILURE_V01 = 3,
  RMTS_DEV_ERROR_PARAM_ERROR_V01 = 4,
  RMTS_DEV_ERROR_OTHER_V01 = 5,
  RMTFS_DEV_ERROR_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}rmtfs_dev_error_enum_v01;
/**
    @}
  */

/** @addtogroup rmtfs_qmi_messages
    @{
  */
/** Response Message; Allows the client to get the device error for the last data request. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Device Error  */
  uint8_t status_valid;  /**< Must be set to true if status is being passed */
  rmtfs_dev_error_enum_v01 status;
  /**<   Indicates the device error that occurred. Values: \n
       - 0 -- No error occurred \n
       - 1 -- Device was not found \n
       - 2 -- Partition was not found \n
       - 3 -- Read/write error occurred \n
       - 4 -- Parameter error occurred \n
       - 5 -- Other unspecified error occurred */
}rmtfs_get_dev_error_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rmtfs_qmi_messages
    @{
  */
/** Indication Message; Allows the server to indicate a force sync to the client. */
typedef struct {

  /* Mandatory */
  /*  Caller ID */
  uint32_t caller_id_len;  /**< Must be set to # of elements in caller_id */
  uint32_t caller_id[RMTFS_MAX_CALLER_ID_V01];
  /**<   List of the caller IDs for which the force sync is to be issued. */
}rmtfs_force_sync_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup rmtfs_qmi_msg_ids
    @{
  */
#define QMI_RMTFS_OPEN_REQ_V01 0x0001
#define QMI_RMTFS_OPEN_RESP_V01 0x0001
#define QMI_RMTFS_CLOSE_REQ_V01 0x0002
#define QMI_RMTFS_CLOSE_RESP_V01 0x0002
#define QMI_RMTFS_RW_IOVEC_REQ_V01 0x0003
#define QMI_RMTFS_RW_IOVEC_RESP_V01 0x0003
#define QMI_RMTFS_ALLOC_BUFF_REQ_V01 0x0004
#define QMI_RMTFS_ALLOC_BUFF_RESP_V01 0x0004
#define QMI_RMTFS_GET_DEV_ERROR_REQ_V01 0x0005
#define QMI_RMTFS_GET_DEV_ERROR_RESP_V01 0x0005
#define QMI_RMTFS_FORCE_SYNC_IND_V01 0x0006
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro rmtfs_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type rmtfs_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define rmtfs_get_service_object_v01( ) \
          rmtfs_get_service_object_internal_v01( \
            RMTFS_V01_IDL_MAJOR_VERS, RMTFS_V01_IDL_MINOR_VERS, \
            RMTFS_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

