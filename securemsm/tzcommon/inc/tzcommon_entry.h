#ifndef __TZ_SERVICE_H_
#define __TZ_SERVICE_H_
/*===========================================================================
  Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

  $Header: //source/qcom/qct/core/pkg/modem/mp/arm11/rel/1.0/modem_proc/core/securemsm/playready/tz/common/shared/inc/tz_playready.h#9 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/08/14    rk     Removed unwanted structs.
05/07/14    rk     Changed stat struct to work on both 32bit and 64bit builds
05/01/14    rk     Made changes to cmd structures to support both 32bit and 64bit
04/03/14    tp     Formatted by running "atyle --style=allman" command.
02/28/14    wt     Added support for cputils mem tag
02/06/14    rk     Separated file/time command IDs to fs_msg.h/time_msg.h
11/14/13    rz     Added support for opendir, readdir
10/04/13    cz     Added support of returning partition free size
06/06/13    rk     Added support for file rename operation
05/21/13    dm     Added support for file sync operation
04/04/13    dm     Added changes to support internal decrypt testing for QSAPPS
06/07/12    cz     Added a version control service
01/30/12    dm     Replaced protect_mem() with content protection and added new decrypt API.
01/19/12   chm     Added support for QSECOM.
12/22/11    kr     Update for CR#326083.
12/14/11    dm     Modified the value for TZ_CM_CMD_UNKNOWN as in TZ side
11/17/11    cz     Added TZ_PR_CMD_FILE_CHOWN_CHMOD, and fixed CR 313052
09/19/11    cz     Splited tz_playready.h
02/08/11    vs     Added Provisioning APIs
05/11/11    cz     Added a path in tz_prov_provision_rsp_t to support chmod/chown
04/28/10   chm     Added support for decryption using TZBSP Crypto Driver.
04/28/11   chm     Added support for Memory protection API's.
03/24/11   jct     Added testdir request and response structures
03/03/11   jct     Added fs and time command id's
02/09/11   ssm     Initial Version.

===========================================================================*/

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
/** @addtogroup tz_playready
  @} */
#include "comdef.h"
#include "common_log.h"
#include "stdlib.h"
#include "app_main.h"

/* Size of PR license or challenge */

#define TZ_CM_MAX_NAME_LEN          256   /* Fixed. Don't increase the size of TZ_CM_MAX_NAME_LEN*/
#define TZ_CM_MAX_DATA_LEN          20000
#define TZ_CM_PROV_PKG_SIZE         10240  //TZ_CM_PROV_PKG_SIZE must be smaller than TZ_CM_MAX_DATA_LEN

#define TZCOMMON_CREATE_CMD(x)  (SVC_TZCOMMMON_ID | x)


typedef struct tzStat {
    uint64              st_dev;          /* ID of device containing file */
    unsigned char       __pad0[4];
    uint32              __st_ino;
    unsigned int        st_mode;         /* protection */
    unsigned int        st_nlink;        /* number of hard links */
    uint32              st_uid;          /* user ID of owner */
    uint32              st_gid;          /* group ID of owner */
    uint64              st_rdev;         /* device ID (if special file) */
    unsigned char       __pad3[4];
    int64               st_size;         /* total size, in bytes */
    uint32	        st_blksize;      /* blocksize for filesystem I/O */
    uint64              st_blocks;       /* number of blocks allocated */
    uint32              st_atime;        /* time of last access */
    uint32              st_atime_nsec;
    uint32              st_mtime;        /* time of last modification */
    uint32              st_mtime_nsec;
    uint32              st_ctime;        /* time of last status change */
    uint32              st_ctime_nsec;
    uint64              st_ino;          /* inode number */
}__attribute__ ((packed)) tzStat_t;

typedef struct tzDirent
{
    uint64            d_ino;
    int64             d_off;
    unsigned short    d_reclen;
    unsigned char     d_type;
    char              d_name[TZ_CM_MAX_NAME_LEN];
} __attribute__ ((packed)) tzDirent_t;

/**
  Error Codes for FS
 */
typedef enum
{
    E_FS_SUCCESS         =  0,
    E_FS_FAILURE         = -1,
    E_FS_INVALID_ARG     = -2,
    E_FS_DIR_NOT_EXIST   = -3,
    E_FS_PATH_TOO_LONG   = -4,
} tz_common_error_codes;

/**
  Commands for :
  1) TZ Services requested by HLOS
  2) HLOS services requested by TZ
 */
#define SEC_UI_FIRST_COMMAND_ID  TZCOMMON_CREATE_CMD(0x00000501)

typedef enum
{
    /* HLOS to TZ commands -
    ** Following commands represent services that HLOS could request from TZ.
    ** This is the traditional use case where HLOS will be the client and TZ will service the following requests.
    */
    TZ_CM_CMD_INVALID           = TZCOMMON_CREATE_CMD(0x00000000),
    TZ_CM_CMD_UNUSED1,                                           /**< Initialize the shared buffer */
    TZ_CM_CMD_UNUSED2,                                           /**< Initialize the logging shared buf */
    TZ_CM_CMD_UNUSED3,                                    /**< Protect content data memory */
    TZ_CM_CMD_REGISTER_LISTENER,
    TZ_CM_CMD_EXEC_TEST_START   = TZCOMMON_CREATE_CMD(0x00000101),
    TZ_CM_CMD_EXEC_TEST,
    TZ_CM_CMD_EXEC_TEST_END,
    TZ_CM_CMD_CP_TAG_MEM        = TZCOMMON_CREATE_CMD(0x00000151),   /**< cp_utils_tag_mem - DO NOT CHANGE */

    // TZCOMMON_CREATE_CMD(0x00000201) is used by FS Service in fs_msg.h
    // TZCOMMON_CREATE_CMD(0x00000301) is used by TIME Service in time_msg.h

    /* TZ to HLOS commands -
    ** HLOS gets the TZ version
    */
    TZ_CM_CMD_VERSION_START     = TZCOMMON_CREATE_CMD(0x00000401),
    TZ_CM_CMD_VERSION_GET_VER,
    TZ_CM_CMD_VERSION_END,
    TZ_CM_CMD_CPCHECK_TOGGLE,   /**< turns on/off the content protection feature on TZ */
    TZ_CM_CMD_GET_CAPABILITY,
    TZ_CM_CMD_PROV_CHECK,
    /* TZ to HLOS Secure UI listener commands
    */
    SEC_UI_CMD_GET_SCREEN_PROPERTIES     = SEC_UI_FIRST_COMMAND_ID,
    SEC_UI_CMD_START_SECURE_DISPLAY,
    SEC_UI_CMD_STOP_SECURE_DISPLAY,
    SEC_UI_CMD_DISPLAY_SECURE_BUFF,
    SEC_UI_CMD_GET_SECURE_DISPLAY_STATUS,
    SEC_UI_CMD_PROTECT_NEXT_BUFFER,
    SEC_UI_CMD_INIT_DONE,
    SEC_UI_CMD_START_SECURE_TOUCH,
    SEC_UI_CMD_STOP_SECURE_TOUCH,
    SEC_UI_CMD_WAIT_FOR_TOUCH_EVENT,
    SEC_UI_CMD_HLOS_RELEASE,
    SEC_UI_LAST_COMMAND_ID,

    TZ_CM_CMD_UNKNOWN           = TZCOMMON_CREATE_CMD(0x7FFFFFFF)
} tz_common_cmd_type;

/** Command structure for initializing shared buffers (SB_OUT
    and SB_LOG)
*/

typedef struct tz_exec_test_req_s
{
    /** First 4 bytes should always be command id */
    tz_common_cmd_type        cmd_id;
    /** test module */
    uint32                    module;
} __attribute__ ((packed)) tz_exec_test_req_t;

typedef struct tz_exec_test_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_common_cmd_type        cmd_id;
    /** Results of the tests; 0 - success; otherwise failure */
    int8                      resultVector[50];
    /** Error messages for failed tests */
    uint8                     errorLogs[TZ_CM_MAX_DATA_LEN];
    /** Number of tests executed */
    int32                     ret;
} __attribute__ ((packed)) tz_exec_test_rsp_t;


typedef struct tz_unknown_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_common_cmd_type       cmd_id;
} __attribute__ ((packed)) tz_unknown_rsp_t;


/** Command structure for getting tzapps version
*/
typedef struct tz_qsappsver_get_ver_req_s
{
    /** First 4 bytes are always command id */
    tz_common_cmd_type          cmd_id;
} __attribute__ ((packed)) tz_qsappsver_get_ver_req_t;


typedef struct tz_qsappsver_get_ver_rsp_s
{
    /** First 4 bytes are always command id */
    tz_common_cmd_type          cmd_id;
    /** Version of tz apps */
    uint32                      version;
    /**<-- Return value for maintenance */
    int32                   ret;
} __attribute__ ((packed)) tz_qsappsver_get_ver_rsp_t;


/** Command structure to set the DRM CP flag
*/
typedef struct tz_qsapp_cpcheck_req_s
{
    /** First 4 bytes should always be command id */
    tz_common_cmd_type          cmd_id;
    /*flag for CP turning on/off*/
    uint8                       bContentProtection;
} __attribute__ ((packed)) tz_qsapp_cpcheck_req_t;

typedef struct tz_qsapp_cpcheck_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_common_cmd_type          cmd_id;
    int32                       ret;
} __attribute__ ((packed)) tz_qsapp_cpcheck_rsp_t;

/** Command structure for getting tzapps version
*/
typedef struct tz_qsapps_get_capability_req_s
{
  /** First 4 bytes are always command id */
  tz_common_cmd_type          cmd_id;
} __attribute__ ((packed)) tz_qsapps_get_capability_req_t;


typedef struct tz_qsapps_get_capability_rsp_s
{
  /** First 4 bytes are always command id */
  tz_common_cmd_type          cmd_id;
  /** Version of tz apps */
  uint32                      TZCapability;
  /**<-- Return value for maintenance */
  int32                       ret;
} __attribute__ ((packed)) tz_qsapps_get_capability_rsp_t;
/** Command structure for dir_open
*/
typedef struct tz_dir_open_req_s
{
    /** First 4 bytes are always command id */
    tz_common_cmd_type      cmd_id;
    /** Pointer to directory name with complete path */
    const char              pathname[TZ_CM_MAX_NAME_LEN];
} __attribute__ ((packed)) tz_dir_open_req_t;

typedef struct tz_dir_open_rsp_s
{
    /** First 4 bytes are always command id */
    tz_common_cmd_type      cmd_id;
    /** Pointer to directory stream */
    uint64                  pdir;
    /** Success/failure value */
    int                     ret;
} __attribute__ ((packed)) tz_dir_open_rsp_t;


/** Command structure for dir_read
*/
typedef struct tz_dir_read_req_s
{
    /** First 4 bytes are always command id */
    tz_common_cmd_type      cmd_id;
    /** Pointer to directory stream */
    uint64                  pdir;
} __attribute__ ((packed)) tz_dir_read_req_t;

typedef struct tz_dir_read_rsp_s
{
    /** First 4 bytes are always command id */
    tz_common_cmd_type      cmd_id;
    /** Directory entry struct */
    struct tzDirent          pdirent;
    /** Success/failure value */
    int                     ret;
} __attribute__ ((packed)) tz_dir_read_rsp_t;

/** Command structure for dir_close
*/
typedef struct tz_dir_close_req_s
{
    /** First 4 bytes are always command id */
    tz_common_cmd_type      cmd_id;
    /** Pointer to directory stream */
    uint64                   pdir;
} __attribute__ ((packed)) tz_dir_close_req_t;

typedef struct tz_dir_close_rsp_s
{
    /** First 4 bytes are always command id */
    tz_common_cmd_type      cmd_id;
    /** Success/failure value */
    int                     ret;
} __attribute__ ((packed)) tz_dir_close_rsp_t;

/** Command structure to set cp mem tag
*/
typedef struct tz_cp_tag_mem_req_s
{
    /** First 4 bytes should always be command id */
    uint32                          cmd_id;
    uint32                          tag;
    tz_buf_array_s_t                secBufferHandle;
    uint32                          secBuffLength;
} __attribute__ ((packed)) tz_cp_tag_mem_req_t;

typedef struct tz_cp_tag_mem_rsp_s
{
    /** First 4 bytes should always be command id */
    uint32                          cmd_id;
    long                            ret;
} __attribute__ ((packed)) tz_cp_tag_mem_rsp_t;


#endif /* __TZ_SERVICE_H_ */
