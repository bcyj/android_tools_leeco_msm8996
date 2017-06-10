#ifndef RFSA_SERVICE_01_H
#define RFSA_SERVICE_01_H
/**
  @file remote_filesystem_access_v01.h

  @brief This is the public header file which defines the rfsa service Data structures.

  This header file defines the types and structures that were defined in
  rfsa. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/rfsa/main/latest/api/remote_filesystem_access_v01.h#7 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2
   It was generated on: Wed Jul 24 2013 (Spin 4)
   From IDL File: remote_filesystem_access_v01.idl */

/** @defgroup rfsa_qmi_consts Constant values defined in the IDL */
/** @defgroup rfsa_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup rfsa_qmi_enums Enumerated types used in QMI messages */
/** @defgroup rfsa_qmi_messages Structures sent as QMI messages */
/** @defgroup rfsa_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup rfsa_qmi_accessor Accessor for QMI service object */
/** @defgroup rfsa_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup rfsa_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define RFSA_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define RFSA_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define RFSA_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define RFSA_V01_MAX_MESSAGE_ID 0x0026;
/**
    @}
  */


/** @addtogroup rfsa_qmi_consts
    @{
  */

/**  Maximum file path size for open request. */
#define RFSA_MAX_FILE_PATH_V01 255

/**  Maximum IOVEC entries for read or write IOVEC requests. */
#define RFSA_MAX_IOVEC_ENTRIES_V01 50
/**
    @}
  */

/** @addtogroup rfsa_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t count;
  /**<   Size of the actual read. */

  uint64_t buffer;
  /**<   Physical address of the shared memory used to store the data. */
}rfsa_file_content_v01;  /* Type */
/**
    @}
  */

/**  Enumeration for the type of access to check */
typedef uint64_t rfsa_access_flag_mask_v01;
#define RFSA_ACCESS_FLAG_READ_V01 ((rfsa_access_flag_mask_v01)0x01ull)
#define RFSA_ACCESS_FLAG_WRITE_V01 ((rfsa_access_flag_mask_v01)0x02ull)
#define RFSA_ACCESS_FLAG_APPEND_V01 ((rfsa_access_flag_mask_v01)0x04ull)
#define RFSA_ACCESS_FLAG_CREATE_V01 ((rfsa_access_flag_mask_v01)0x08ull)
#define RFSA_ACCESS_FLAG_TRUNC_V01 ((rfsa_access_flag_mask_v01)0x10ull)
/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Request Message; Gets information about a file.  It also checks if the file
              has the correct permission set for opening. */
typedef struct {

  /* Mandatory */
  /*  File Path */
  char filename[RFSA_MAX_FILE_PATH_V01 + 1];
  /**<   File path, including directory remapping tag.  The path and tag used
        for the filename argument is in a URL-like form, e.g.,
        TAG://rootpath/filename. */
}rfsa_file_stat_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Response Message; Gets information about a file.  It also checks if the file
              has the correct permission set for opening. */
typedef struct {

  /* Mandatory */
  /*  QMI Result Code */
  qmi_response_type_v01 resp;

  /* Mandatory */
  /*  File Access Permission Flags */
  rfsa_access_flag_mask_v01 flags;
  /**<   Indicates file access type to be used when opening the file. Values: \n
      - 0x01 -- File can be opened for a read \n
      - 0x02 -- File can be opened for a write \n
      - 0x08 -- File does not exist, it can be created \n
      - 0x10 -- File does exist and it can be truncated */

  /* Optional */
  /*  Size of File */
  uint8_t size_valid;  /**< Must be set to true if size is being passed */
  uint32_t size;
  /**<   Size, in bytes, of the file. */
}rfsa_file_stat_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Request Message; Creates an empty file container or truncates an existing file
              before a write operation. */
typedef struct {

  /* Mandatory */
  /*  File Path */
  char filename[RFSA_MAX_FILE_PATH_V01 + 1];
  /**<   File path, including directory remapping tag. The path and tag used for
       the filename argument is in a URL-like form, e.g.,
	   TAG://rootpath/filename. */

  /* Mandatory */
  /*  File Access Permission Flags */
  rfsa_access_flag_mask_v01 flags;
  /**<   Indicates file access type to be used when opening the file. Values: \n
      - 0x08 -- File does not exist, it is created \n
      - 0x10 -- File exists and is truncated \n */
}rfsa_file_create_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Response Message; Creates an empty file container or truncates an existing file
              before a write operation. */
typedef struct {

  /* Mandatory */
  /*  QMI Result Code */
  qmi_response_type_v01 resp;
}rfsa_file_create_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Request Message; Client requests the server to look up the shared memory buffer
              allocated to the specified client and returns the address. */
typedef struct {

  /* Mandatory */
  /*  Client ID */
  uint32_t client_id;
  /**<   Unique identifier for the client. */

  /* Mandatory */
  /*  Size of the Buffer to Request */
  uint32_t size;
  /**<   Size, in bytes, of the requested buffer. */
}rfsa_get_buff_addr_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Response Message; Client requests the server to look up the shared memory buffer
              allocated to the specified client and returns the address. */
typedef struct {

  /* Mandatory */
  /*  QMI Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Buffer Address */
  uint8_t address_valid;  /**< Must be set to true if address is being passed */
  uint64_t address;
  /**<   Physical address of the shared memory buffer. */
}rfsa_get_buff_addr_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Request Message; Client releases the buffer that the server allocated from the
              shared memory buffer. */
typedef struct {

  /* Mandatory */
  /*  Client ID */
  uint32_t client_id;
  /**<   Unique client identifier. */

  /* Mandatory */
  /*  Buffer Address */
  uint64_t address;
  /**<   Physical address of the shared memory buffer. */
}rfsa_release_buff_addr_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Response Message; Client releases the buffer that the server allocated from the
              shared memory buffer. */
typedef struct {

  /* Mandatory */
  /*  QMI Result Code */
  qmi_response_type_v01 resp;
}rfsa_release_buff_addr_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Request Message; Reads data from a file.  */
typedef struct {

  /* Mandatory */
  /*  Client ID */
  uint32_t client_id;
  /**<   Unique client identifier. */

  /* Mandatory */
  /*  File Path */
  char filename[RFSA_MAX_FILE_PATH_V01 + 1];
  /**<   File path, including directory remapping tag.  The path and tag used
        for the filename argument is in a URL-like form, e.g.,
        TAG://rootpath/filename. */

  /* Mandatory */
  /*  File Offset to Start */
  uint32_t offset;
  /**<   Offset, in bytes, to the start of the read. */

  /* Mandatory */
  /*  Size to Read */
  uint32_t size;
  /**<   Number of bytes to read from the file. */
}rfsa_file_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Response Message; Reads data from a file.  */
typedef struct {

  /* Mandatory */
  /*  QMI Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Information About the Actual Read */
  uint8_t data_valid;  /**< Must be set to true if data is being passed */
  rfsa_file_content_v01 data;
}rfsa_file_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rfsa_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t file_offset;
  /**<   File offset to access. */

  uint32_t buff_addr_offset;
  /**<   Data buffer's offset address. */

  uint32_t size;
  /**<   Number of bytes to read from the file. */
}rfsa_iovec_desc_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Request Message; Reads data from a file in multiple pieces.  */
typedef struct {

  /* Mandatory */
  /*  Client ID */
  uint32_t client_id;
  /**<   Unique client identifier. */

  /* Mandatory */
  /*  File Path */
  char filename[RFSA_MAX_FILE_PATH_V01 + 1];
  /**<   File path, including directory remapping tag.  The path and tag used
        for the filename argument is in a URL-like form, e.g.,
        TAG://rootpath/filename. */

  /* Mandatory */
  /*  IOVEC Structure Array */
  uint32_t iovec_struct_len;  /**< Must be set to # of elements in iovec_struct */
  rfsa_iovec_desc_type_v01 iovec_struct[RFSA_MAX_IOVEC_ENTRIES_V01];
}rfsa_iovec_file_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Response Message; Reads data from a file in multiple pieces.  */
typedef struct {

  /* Mandatory */
  /*  QMI result code */
  qmi_response_type_v01 resp;
}rfsa_iovec_file_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Request Message; Writes data to a file in multiple pieces. */
typedef struct {

  /* Mandatory */
  /*  Client ID */
  uint32_t client_id;
  /**<   Unique client identifier. */

  /* Mandatory */
  /*  File Path */
  char filename[RFSA_MAX_FILE_PATH_V01 + 1];
  /**<   File path, including directory remapping tag.  The path and tag used
        for the filename argument is in a URL-like form, e.g.,
        TAG://rootpath/filename. */

  /* Mandatory */
  /*  IOVEC structure array */
  uint32_t iovec_struct_len;  /**< Must be set to # of elements in iovec_struct */
  rfsa_iovec_desc_type_v01 iovec_struct[RFSA_MAX_IOVEC_ENTRIES_V01];
}rfsa_iovec_file_write_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup rfsa_qmi_messages
    @{
  */
/** Response Message; Writes data to a file in multiple pieces. */
typedef struct {

  /* Mandatory */
  /*  QMI Result Code */
  qmi_response_type_v01 resp;
}rfsa_iovec_file_write_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup rfsa_qmi_msg_ids
    @{
  */
#define QMI_RFSA_FILE_STAT_REQ_MSG_V01 0x0020
#define QMI_RFSA_FILE_STAT_RESP_MSG_V01 0x0020
#define QMI_RFSA_FILE_CREATE_REQ_MSG_V01 0x0021
#define QMI_RFSA_FILE_CREATE_RESP_MSG_V01 0x0021
#define QMI_RFSA_FILE_READ_REQ_MSG_V01 0x0022
#define QMI_RFSA_FILE_READ_RESP_MSG_V01 0x0022
#define QMI_RFSA_GET_BUFF_ADDR_REQ_MSG_V01 0x0023
#define QMI_RFSA_GET_BUFF_ADDR_RESP_MSG_V01 0x0023
#define QMI_RFSA_RELEASE_BUFF_ADDR_REQ_MSG_V01 0x0024
#define QMI_RFSA_RELEASE_BUFF_ADDR_RESP_MSG_V01 0x0024
#define QMI_RFSA_IOVEC_FILE_READ_REQ_MSG_V01 0x0025
#define QMI_RFSA_IOVEC_FILE_READ_RESP_MSG_V01 0x0025
#define QMI_RFSA_IOVEC_FILE_WRITE_REQ_MSG_V01 0x0026
#define QMI_RFSA_IOVEC_FILE_WRITE_RESP_MSG_V01 0x0026
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro rfsa_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type rfsa_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define rfsa_get_service_object_v01( ) \
          rfsa_get_service_object_internal_v01( \
            RFSA_V01_IDL_MAJOR_VERS, RFSA_V01_IDL_MINOR_VERS, \
            RFSA_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

