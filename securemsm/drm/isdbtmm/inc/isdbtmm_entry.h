#ifndef __TZ_ISDBTMM_H_
#define __TZ_ISDBTMM_H_
/*===========================================================================
  Copyright (c) 2012 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

  $Header: //source/qcom/qct/core/pkg/bootloaders/rel/1.0/boot_images/core/securemsm/trustzone/tzosapps/tzdrm/isdbtmm/core/shared/inc/isdbtmm_entry.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/16/12   ib     Initial Version.

===========================================================================*/

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
/** @addtogroup tz_isdbtmm
  @} */
#include "comdef.h"
#include "common_log.h"
#include "stdlib.h"
#include "app_main.h"

#define TZ_ISDBTMM_MAX_DATA_LEN          20000
#define TZ_ISDBTMM_BUFF_SIZE             15000  //TZ_ISDBTMM_BUFF_SIZE must be smaller than TZ_ISDBTMM_MAX_DATA_LEN
#define TZ_ISDBTMM_MAX_NAME_LEN          256



#define ISDBTMM_CREATE_CMD(x)     (SVC_ISDBTMM_ID | x)

/**
  Commands for TZ isdbtmm
 */
typedef enum
{
  TZ_ISDBTMM_CMD_INVALID              = ISDBTMM_CREATE_CMD(0x00000000),
  TZ_ISDBTMM_CMD_START,
  TZ_ISDBTMM_CMD_SFS_OPEN,
  TZ_ISDBTMM_CMD_SFS_CLOSE,
  TZ_ISDBTMM_CMD_SFS_REMOVE,
  TZ_ISDBTMM_CMD_SFS_READ,
  TZ_ISDBTMM_CMD_SFS_WRITE,
  TZ_ISDBTMM_CMD_SFS_MKDIR,
  TZ_ISDBTMM_CMD_SFS_GET_SIZE,
  TZ_ISDBTMM_CMD_SFS_SEEK,
  TZ_ISDBTMM_CMD_SFS_RMDIR,
  TZ_ISDBTMM_CMD_END,
  TZ_ISDBTMM_CMD_UNKNOWN              = ISDBTMM_CREATE_CMD(0x7FFFFFFF)
} tz_isdbtmm_cmd_type;



/** Command structure for isdbt sfs open
*/
typedef struct tz_isdbt_sfs_open_req_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  uint8                  fileName[TZ_ISDBTMM_MAX_NAME_LEN];   // Name of the file to open in the predefined SFS folder
  int32                  mode;                        // File mode
} __attribute__ ((packed)) tz_isdbtmm_sfs_open_req_t;

typedef struct tz_isdbtmm_sfs_open_rsp_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  /** Returned file descriptor; >0 if succesfull or -1 if failed  */
  int32                     ret;
} __attribute__ ((packed)) tz_isdbtmm_sfs_open_rsp_t;


/** Command structure for isdbt sfs close
*/
typedef struct tz_isdbt_sfs_close_req_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  int                    fileDesc;   // Descriptor of the file to be closed
} __attribute__ ((packed)) tz_isdbtmm_sfs_close_req_t;

typedef struct tz_isdbtmm_sfs_close_rsp_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  /** Returns 0 if succesfull or -1 if failed  */
  int32                     ret;
} __attribute__ ((packed)) tz_isdbtmm_sfs_close_rsp_t;


/** Command structure for isdbt sfs remove file
*/
typedef struct tz_isdbt_sfs_remove_req_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  uint8                  fileName[TZ_ISDBTMM_MAX_NAME_LEN];   // Name of the file to be deleted from the predefined SFS folder
} __attribute__ ((packed)) tz_isdbtmm_sfs_remove_req_t;

typedef struct tz_isdbtmm_sfs_remove_rsp_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  /** Returns 0 if succesfull or -1 if failed  */
  int32                     ret;
} __attribute__ ((packed)) tz_isdbtmm_sfs_remove_rsp_t;


/** Command structure for isdbt sfs remove directory
*/
typedef struct tz_isdbt_sfs_rmdir_req_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  uint8                  dirName[TZ_ISDBTMM_MAX_NAME_LEN];   // Name of the directory to be deleted from the SFS
} __attribute__ ((packed)) tz_isdbtmm_sfs_rmdir_req_t;

typedef struct tz_isdbtmm_sfs_rmdir_rsp_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  /** Returns 0 if succesfull or -1 if failed  */
  int32                     ret;
} __attribute__ ((packed)) tz_isdbtmm_sfs_rmdir_rsp_t;


/** Command structure for isdbt sfs read
*/
typedef struct tz_isdbt_sfs_read_req_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  int                    fileDesc;   // Descriptor of the file to read from
  uint32                 count;      // number of bytes to read
} __attribute__ ((packed)) tz_isdbtmm_sfs_read_req_t;

typedef struct tz_isdbtmm_sfs_read_rsp_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  /** Returns the number of bytes read or -1 if failed  */
  int32                     ret;
  /** Chunk of up to TZ_ISDBTMM_BUFF_SIZE bytes of data read from the file */
  uint8                     buf[TZ_ISDBTMM_BUFF_SIZE];  // Read data buffer.
} __attribute__ ((packed)) tz_isdbtmm_sfs_read_rsp_t;


/** Command structure for isdbt sfs write
*/
typedef struct tz_isdbt_sfs_write_req_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  int                    fileDesc;        // Descriptor of the file to write to
  uint8                  buf[TZ_ISDBTMM_BUFF_SIZE];  // Write data buffer.
  uint32                 count;           // number of bytes to write
} __attribute__ ((packed)) tz_isdbtmm_sfs_write_req_t;

typedef struct tz_isdbtmm_sfs_write_rsp_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  /** Returns the number of bytes written or -1 if failed  */
  int32                     ret;
} __attribute__ ((packed)) tz_isdbtmm_sfs_write_rsp_t;


/** Command structure for isdbt sfs make dir
*/
typedef struct tz_isdbt_sfs_mkdir_req_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  uint8                  dirName[TZ_ISDBTMM_MAX_NAME_LEN];   // Name of the new folder to be created
} __attribute__ ((packed)) tz_isdbtmm_sfs_mkdir_req_t;

typedef struct tz_isdbtmm_sfs_mkdir_rsp_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  /** Returns 0 if succesfull or -1 if failed  */
  int32                     ret;
} __attribute__ ((packed)) tz_isdbtmm_sfs_mkdir_rsp_t;


/** Command structure for isdbt sfs get file size
*/
typedef struct tz_isdbt_sfs_get_size_req_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  int                    fileDesc;        // Descriptor of the file
} __attribute__ ((packed)) tz_isdbtmm_sfs_get_size_req_t;

typedef struct tz_isdbtmm_sfs_get_size_rsp_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  /** Returned file size                        */
  uint32					size;
  /** Returns 0 if succesfull or -1 if failed   */
  int32                     ret;
} __attribute__ ((packed)) tz_isdbtmm_sfs_get_size_rsp_t;


/** Command structure for isdbt sfs seek
*/
typedef struct tz_isdbt_sfs_seek_req_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  int                    fileDesc;   // Descriptor of the file
  int32                  offset   ;  // File offset to seek in bytes.
  int                    whence;     // Indicates start, end, or current position
} __attribute__ ((packed)) tz_isdbtmm_sfs_seek_req_t;

typedef struct tz_isdbtmm_sfs_seek_rsp_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  /** Returns the current location if successful, or -1 if an error occurred  */
  int32                     ret;
} __attribute__ ((packed)) tz_isdbtmm_sfs_seek_rsp_t;


/** Command structure for isdbt create sfs root
*/
typedef struct tz_isdbt_sfs_create_root_req_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
} __attribute__ ((packed)) tz_isdbtmm_sfs_create_root_req_t;

typedef struct tz_isdbtmm_sfs_create_root_rsp_s
{
  /** First 4 bytes should always be command id */
  tz_isdbtmm_cmd_type    cmd_id;
  /** Returns 0 if succesfull or -1 if failed   */
  int32                     ret;
} __attribute__ ((packed)) tz_isdbtmm_sfs_create_root_rsp_t;



#endif /* __TZ_ISDBTMM_H_ */

