#ifndef __DRMPROV_ENTRY_H_
#define __DRMPROV_ENTRY_H_
/*===========================================================================
Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved..
Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

  $Header:

when        who     what, where, why
--------   ---     ----------------------------------------------------------
05/01/14   rk      Changed long to uint32
04/14/14   dm      Deprecated DIAG provisioning APIs
04/03/14   tp      Formatted by running "atyle --style=allman" command.
02/28/14   dm      Removed the usage of macro SINGLE_TIME_PROV
10/31/13   wt      Added structs for oem unwrap save key API and changed IPC structs names
09/30/13   wt      Added structs for encap save key and unwrap save key prov API.
01/17/13   rk      Modified generic prov APIs to handle data larger than TZ_PR_PROV_PKG_SIZE
12/03/12   cz      Move drmprov apis to the drmprov_clnt.h file
08/08/12   rk      Initial Version.

===========================================================================*/

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
/** @addtogroup tz_playready
  @} */
#include "comdef.h"
#include "stdlib.h"
#include "app_main.h"

#define DRMPROV_CREATE_CMD(x)     (SVC_DRMPROV_ID | x)

/**
  Commands for :
  1) TZ Services requested by HLOS
  2) HLOS services requested by TZ
 */
typedef enum
{
    /* HLOS to DRMPROV commands -
    ** Following commands represent services that HLOS could request from TZ.
    ** This is the traditional use case where HLOS will be the client and TZ will service the following requests.
    */
    DRMPROV_CMD_INVALID           = DRMPROV_CREATE_CMD(0x00000000),
    DRMPROV_CMD_SAVE_KEY,               /**< generic cmd of prvisioning keys using sfs */
    DRMPROV_CMD_VERIFY_KEY,             /**< generic cmd of verification of provisioned keys using sfs */
    DRMPROV_CMD_UNUSED1,                /**< deprecated diag provisioning command */
    DRMPROV_CMD_UNUSED2,                /**< deprecated diag provisioning command */
    DRMPROV_CMD_FINALIZE_KEY_PROV,
    DRMPROV_CMD_SAVE_IPC_WRAPPED_KEYS,  /**< generic cmd of prvisioning wrapped keys using sfs */
    DRMPROV_CMD_ENCAP_SAVE_KEY,         /**< generic cmd of provisioned encapsulated key */
    DRMPROV_CMD_OEM_UNWRAP_AND_SAVE_KEY,/**< generic cmd of provisioned oem wrapped key */
    DRMPROV_CMD_UNKNOWN           = DRMPROV_CREATE_CMD(0x7FFFFFFF)
} drmprov_cmd_type;

/** Command structure for saving generic drm prov keys
*/
typedef struct tz_drm_save_key_req_s
{
    /** First 4 bytes should always be command id */
    drmprov_cmd_type            pr_cmd;
    /** Length of the feature name */
    uint32                    feature_name_len;
    /** Length of the file name */
    uint32                    file_name_len;
    /** Length of the data */
    uint32                    msg_len;
    /** feature name */
    uint8                     feature_name[TZ_CM_MAX_NAME_LEN];
    /** file name */
    uint8                     file_name[TZ_CM_MAX_NAME_LEN];
    /** data for the keys */
    uint8                     msg_data[TZ_CM_PROV_PKG_SIZE];
    /** Indicates whether file to be created with O_TRUNC */
    uint8                     file_truncation_flag;
} __attribute__ ((packed)) tz_drm_save_key_req_t;

typedef struct tz_drm_save_key_rsp_s
{
    /** First 4 bytes should always be command id */
    drmprov_cmd_type            pr_cmd;
    /** Path to the created file */
    uint8                     prt_path[TZ_CM_MAX_NAME_LEN];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                       ret;
} __attribute__ ((packed)) tz_drm_save_key_rsp_t;

/** Command structure for verifying generic drm prov keys
*/
typedef struct tz_drm_verify_key_req_s
{
    /** First 4 bytes should always be command id */
    drmprov_cmd_type          pr_cmd;
    /** Length of the feature name */
    uint32                    feature_name_len;
    /** Length of the file name */
    uint32                    file_name_len;
    /** Length of the data */
    uint32                    msg_len;
    /** feature name */
    uint8                     feature_name[TZ_CM_MAX_NAME_LEN];
    /** file name */
    uint8                     file_name[TZ_CM_MAX_NAME_LEN];
    /** data for the keys */
    uint8                     msg_data[TZ_CM_PROV_PKG_SIZE];
} __attribute__ ((packed)) tz_drm_verify_key_req_t;

typedef struct tz_drm_verify_key_rsp_s
{
    /** First 4 bytes should always be command id */
    drmprov_cmd_type          pr_cmd;
    /** Path to the created file */
    uint8                     prt_path[TZ_CM_MAX_NAME_LEN];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_drm_verify_key_rsp_t;

/** Command structure for finalizing key provisioning
*/
typedef struct tz_prov_finalize_provision_req_s
{
    /** First 4 bytes are always command id */
    drmprov_cmd_type        cmd_id;

} __attribute__ ((packed)) tz_prov_finalize_provision_req_t;

typedef struct tz_prov_finalize_provision_rsp_s
{
    /** First 4 bytes are always command id */
    drmprov_cmd_type        cmd_id;

    /** Return vaule of provisioning */
    int32                   ret;
} __attribute__ ((packed)) tz_prov_finalize_provision_rsp_t;

/** Command structure for generic key provisioning of saving wrapped prov keys
*/
typedef struct tz_drm_save_ipc_wrapped_key_req_s
{
    /** First 4 bytes should always be command id */
    drmprov_cmd_type          pr_cmd;
    /** feature name length */
    uint32                    feature_name_len;
    /** file name length */
    uint32                    file_name_len;
    /** Length of the data */
    uint32                    msg_len;
    /** feature name */
    uint8                     feature_name[TZ_CM_MAX_NAME_LEN];
    /** file name */
    uint8                     file_name[TZ_CM_MAX_NAME_LEN];
    /** data for the keys */
    uint8                     msg_data[TZ_CM_PROV_PKG_SIZE];
    /** Indicates whether file to be created with O_TRUNC */
    uint8                     file_truncation_flag;
    /** Name of source app */
    uint8                     source_app_name[TZ_CM_MAX_NAME_LEN];
    /** Length of name of source app */
    uint32                    source_app_name_len;
} __attribute__ ((packed)) tz_drm_save_ipc_wrapped_key_req_t;

typedef struct tz_drm_save_ipc_wrapped_key_rsp_s
{
    /** First 4 bytes should always be command id */
    drmprov_cmd_type          pr_cmd;
    /** Path to the created file */
    uint8                     prt_path[TZ_CM_MAX_NAME_LEN];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_drm_save_ipc_wrapped_key_rsp_t;

/** Command structure for generic key provisioning of saving prov keys
*/
typedef struct tz_drm_encap_save_key_req_s
{
    /** First 4 bytes should always be command id */
    drmprov_cmd_type          pr_cmd;
    /** feature name length */
    uint32                    feature_name_len;
    /** file name length */
    uint32                    file_name_len;
    /** encap app name length */
    uint32                    encap_app_name_len;
    /** Length of the data */
    uint32                    msg_len;
    /** feature name */
    uint8                     feature_name[TZ_CM_MAX_NAME_LEN];
    /** file name */
    uint8                     file_name[TZ_CM_MAX_NAME_LEN];
    /** encap app name */
    uint8                     encap_app_name[TZ_CM_MAX_NAME_LEN];
    /** data for the keys */
    uint8                     msg_data[TZ_CM_PROV_PKG_SIZE];
    /** Indicates whether file to be created with O_TRUNC */
    uint8                     file_truncation_flag;
} __attribute__ ((packed)) tz_drm_encap_save_key_req_t;

typedef struct tz_drm_encap_save_key_rsp_s
{
    /** First 4 bytes should always be command id */
    drmprov_cmd_type          pr_cmd;
    /** Path to the created file */
    uint8                     prt_path[TZ_CM_MAX_NAME_LEN];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_drm_encap_save_key_rsp_t;

/** Command structure for saving generic drm prov keys
*/
typedef struct tz_drm_oem_unwrap_and_save_key_req_s
{
    /** First 4 bytes should always be command id */
    drmprov_cmd_type          pr_cmd;
    /** Length of the feature name */
    uint32                    feature_name_len;
    /** Length of the file name */
    uint32                    file_name_len;
    /** Length of the data */
    uint32                    msg_len;
    /** feature name */
    uint8                     feature_name[TZ_CM_MAX_NAME_LEN];
    /** file name */
    uint8                     file_name[TZ_CM_MAX_NAME_LEN];
    /** data for the keys */
    uint8                     msg_data[TZ_CM_PROV_PKG_SIZE];
    /** Indicates whether file to be created with O_TRUNC */
    uint8                     file_truncation_flag;
} __attribute__ ((packed)) tz_drm_oem_unwrap_and_save_key_req_t;

typedef struct tz_drm_oem_unwrap_and_save_key_rsp_s
{
    /** First 4 bytes should always be command id */
    drmprov_cmd_type          pr_cmd;
    /** Path to the created file */
    uint8                     prt_path[TZ_CM_MAX_NAME_LEN];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_drm_oem_unwrap_and_save_key_rsp_t;

#endif /* __TZ_SERVICE_H_ */
