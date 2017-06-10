#ifndef __TZ_PLAYREADY_H_
#define __TZ_PLAYREADY_H_
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
07/09/14    am     Merge as is: kr: Use copy API for clear content playback through secure path
05/14/14    rk      Changed long to uint32
05/13/14    dm     Added support to toggle the IV constraint check at runtime
04/11/14    dm     Added command structure to save aes and keyfile data
04/03/14    tp     Formatted by running "atyle --style=allman" command.
01/16/14    dm     Reverting back the backward compatibility support added for TZ_OUT_BUF_MAX macro
11/14/13    wt     Added Media DRM API entry
11/01/13    cz     Added a new playready TZ_PR_CMD_DUPLICATE_CTX entry
05/20/13    dm     Added changes to support backward compatibility for the change in TZ_OUT_BUF_MAX
04/04/13    dm     Added changes to support internal decrypt testing for QSAPPS
03/15/13    kr     Update playready_reinitialize() with more parameters
03/07/13    dm     Added support to store AES and keyfile for internal testing purpose.
02/05/13    dm     Modified encrypt function to take type of stream as input argument
02/01/13    hw     Added decrypt_video_t struct for non-contiguous buffer
01/29/13    rk     Added new message to set robustness version dynamically
11/23/12    dm     Added extra paramter to encrypt function
09/18/12   chm     Added support for licence deletion with header & KID BASE64 encoded.
09/13/12    dm     Added toggling support to enable/disable parsing the non-silent URL
08/21/12    dm...  Added support for PK 2.0
08/08/12    rk     Moved provisioning code out of playready.
07/18/12    rp     Added toggling support for avoiding the need for special build for decrypt and domain test cases
05/16/12    dm     Added support for drm reinitialize and modified get property API.
05/07/12    dm     Moved DRM type defintions to drmtypes.h
04/16/12    kr     Additional PlayReady API support for WMDRM
04/11/12    rp     Added the support for turning on/off content protection check on TZ
03/26/12    dm     Added a new parameter in process license response()
03/21/12    kr     Add generic key provisioning APIs support
01/30/12    cz     Changed video and audio decryption functions for virtual address support
01/19/12    dm     Added audio decryption and tamepoering check.
12/14/11    dm     Added support for bind,commit,MTP,domaind and envelope.
10/14/11    cz     Cleaned up v1 functions and replace v1 funcation names with v2 names
10/12/11    cz     Added playready v2 functions and disabled v1 functions
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
#include "drmtypes.h"

/* Size of PR license or challenge */
#define TZ_PR_MAX_PSI_SIZE    9000
#define TZ_PR_MAX_CHG_SIZE    9000
#define TZ_PR_MAX_LIC_SIZE    100000
#define TZ_PR_PROV_PKG_SIZE   10240  //TZ_PR_PROV_PKG_SIZE must be smaller than TZ_PR_MAX_DATA_LEN
#define TZ_PR_MAX_NAME_LEN    256
#define TZ_PR_MAX_DATA_LEN    100000
#define TZ_PR_MAX_ENCRYPTION_UNITS 250


/* Size of PR license or challenge */

#define TZ_PR_MAX_PSIURL_SIZE         256     /* Maximum buffer length for Silent LA URL */
#define TZ_PR_MAX_PNONSIURL_SIZE      256     /* Maximum buffer length for NonSilent LA URL */
#define TZ_PR_MAX_LIC_STATE_UNITS     20
#define TZ_PR_MAX_KID_SIZE            16
#define TZ_PR_MAX_B64_KID_SIZE        24
#define TZ_PR_MAX_RIGHTS_SIZE         1
#define TZ_PR_MAX_CUSTOM_DATA_SIZE    4096
#define TZ_PR_MAX_METER_CERT_SIZE     9000

#define SUM(a,b,c,d)               sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d)
#define TZ_PR_REM_SIZE             TZ_PR_MAX_DATA_LEN - sizeof(tz_pr_cmd_type) - sizeof(int32)

#define PLAYREADY_CREATE_CMD(x)     (SVC_PLAYREADY_ID | x)

/* Size of Copy Buffers */
#define TZ_SC_AES_128_LEN             (16)
#define PLAYREADY_COPY_BUF_SIZE         (100*1024)
#define PLAYREADY_COPY_AUDIO_MAX_BUFFER (64*1024*2)
#define PLAYREADY_COPY_VIDEO_MAX_BUFFER ((1024*1024*2) - PLAYREADY_COPY_AUDIO_MAX_BUFFER)

//Copy Errors
typedef enum PlayreadyCopyResult {
  PLAYREADY_COPY_SUCCESS = 0,
  PLAYREADY_COPY_ERROR_COPY_FAILED,
  PLAYREADY_COPY_ERROR_ION_MALLOC_FAILED,
  PLAYREADY_COPY_ERROR_ION_FREE_FAILED,
  PLAYREADY_COPY_ERROR_INVALID_PARAMS,
  PLAYREADY_COPY_ERROR_FEATURE_NOT_SUPPORTED,
  PLAYREADY_COPY_FAILURE = 0x7FFFFFFF
} PlayreadyCopyResult;

//copy direction
typedef enum PlayreadyCopyDir {
  PLAYREADY_COPY_NONSECURE_TO_SECURE = 0,
  PLAYREADY_COPY_INVALID_DIR
}PlayreadyCopyDir;

/**
  Commands for TZ playready
 */
typedef enum
{
    TZ_PR_CMD_PR_INVALID      = PLAYREADY_CREATE_CMD(0x00000000),
    TZ_PR_CMD_INIT,                    /**< Initialize playready stack */
    TZ_PR_CMD_UNINIT,                  /**< Un Initialize playready stack */
    TZ_PR_CMD_GEN_CHLG,                /**< Get a license from DRM agent */
    TZ_PR_CMD_PROC_LICENSE,            /**< Store a license in database */
    TZ_PR_CMD_DEL_LICENSE,             /**< Delete a license from Store */
    TZ_PR_CMD_DECRYPT,                 /**< Decrypt DRM content data */
    TZ_PR_CMD_SET_HDR,                 /**< Set DRM Header */
    TZ_PR_CMD_GET_LIC_RIGHTS,          /**< Get the license rights */
    TZ_PR_CMD_PROV_START      = PLAYREADY_CREATE_CMD(0x00000101),
    TZ_PR_CMD_PROV_UNUSED1,   /**< moved to drmprov */
    TZ_PR_CMD_PROV_UNUSED2,   /**< moved to drmprov */
    TZ_PR_CMD_SAVE_KEY,       /**< Save keys using sfs */
    TZ_PR_CMD_VERIFY_KEY,     /**< Verify keys, just get the path */
    TZ_PR_CMD_DELETE_KEY,     /**< Delete keys, just get the path */
    TZ_PR_CMD_PROV_END,
    TZ_PR_CMD_BIND_LICENSE,            /**< Bind the license.*/
    TZ_PR_CMD_COMMIT_LICENSE,          /**< Commit the license to store.*/
    TZ_PR_CMD_SYNC_GEN_CHLG,           /**< Get the synchronization list from host */
    TZ_PR_CMD_METER_GEN_CHLG,          /**< Gets the meter challenge data */
    TZ_PR_CMD_PROC_METER_RESP,         /**< Sets the meter response on the device  */
    TZ_PR_CMD_CLEAN_DATA_STORE,        /**< Cleans the device data store */
    TZ_PR_CMD_GET_LICENSE_STATE,       /**< Gets  the license state of the media content */
    TZ_PR_CMD_GET_DEVICE_CERT,         /**< Gets the device certificate stored in SFS  */
    TZ_PR_CMD_PROCESS_COMMAND,         /**< Gets the status of the command sent for processing */
    TZ_PR_CMD_PROCESS_REQUEST,         /**< Gets the status of the data request being sent  */
    TZ_PR_CMD_ENV_OPEN,                /**< Opens an enveloped file */
    TZ_PR_CMD_ENV_CLOSE,               /**< Closes an enveloped file */
    TZ_PR_CMD_ENV_INIT_READ,           /**< Maps the decrypt context for an enveloped file */
    TZ_PR_CMD_ENV_READ,                /**< Reads the clear data from enveloped file */
    TZ_PR_CMD_ENV_SEEK,                /**< Seeks in an enveloped file */
    TZ_PR_CMD_ENV_DUP_FILE_CTX,        /**< Duplicates the enveloped context */
    TZ_PR_CMD_ENV_GET_SIZE,            /**< Gets the size of the enveloped file */
    TZ_PR_CMD_ENV_GET_ORIG_FILENAME,   /**< Gets the original filename of the enveloped file */
    TZ_PR_CMD_UPDATE_EMDSTORE,         /**<.Updates the embedded store with the license */
    TZ_PR_CMD_ENV_WRITE_PLAYREADY_OBJ, /**< Writes the playready object into the enveloped file */
    TZ_PR_CMD_UPDATE_EMDSTORE_COMMIT,  /**<.Commits the license to embedded store */
    TZ_PR_CMD_GET_PROP,                /**< Gets the DRM header property */
    TZ_PR_CMD_SET_DOMAINID,            /**< Set Domain id. */
    TZ_PR_CMD_JOINDOMAIN_GEN_CHLG,     /**< Gets the domain join challenge */
    TZ_PR_CMD_JOINDOMAIN_PROC_RESP,    /**< Sets the domain join response on the device  */
    TZ_PR_CMD_LEAVEDOMAIN_GEN_CHLG,    /**< Gets the domain leave challenge */
    TZ_PR_CMD_LEAVEDOMAIN_PROC_RESP,   /**< Sets the domain leave response on the device  */
    TZ_PR_CMD_DOMAINCERT_FIND,         /**< Finds the domain certificate */
    TZ_PR_CMD_DOMAINCERT_INITENUM,     /**< Initializes enumeration of domain certificate */
    TZ_PR_CMD_DOMAINCERT_ENUMNEXT,     /**< Gets the next element from the domain certificate list */
    TZ_PR_CMD_ENCRYPT,                 /**< Encrypt the audio/video block*/
    TZ_PR_CMD_DECRYPT_AUDIO,           /**< Decrypt Audio */
    TZ_DRM_CMD_UNUSED3,                /**< moved to drmprov */
    TZ_DRM_CMD_UNUSED4,                /**< moved to drmprov */
    TZ_PR_CMD_UNUSED6,                 /**< moved to tzcommon*/
    TZ_PR_CMD_INITDECRYPT,             /**< Initialize the state data for cocktail DRM content data Decryption */
    TZ_PR_CMD_REINIT,                  /**< Cleans up the header information stored in DRM context.*/
    TZ_PR_CMD_UNUSED5,                 /**< Enable/Disable the use of PR Key Hardcoding*/
    TZ_PR_CMD_GETADDN_RSPDATA,         /**< Retrieves the data element from the server response */
    TZ_PR_CMD_PARSE_NONSIURL_TOGGLE,   /**< Turns on/off parsing the non-silent url from DRM header */
    TZ_PR_CMD_REPROV,
    TZ_PR_CMD_STRMGMT_DELLICENSE,      /**< Delete Licenses based on BASE64 encoded KID */
    TZ_PR_CMD_RESET,                   /**< Reset DRM */
    TZ_PR_CMD_SET_ROBUSTNESS_VERSION,  /**< Set robustness version */
    TZ_PR_CMD_SAVE_AES_KEYFILE,        /**< Save aeskey and keyfile */
    TZ_PR_CMD_DUPLICATE_CTX,           /**< Duplicate PR context ID */
    TZ_PR_CMD_CONTENT_SET_PROP,        /**< PR Content set property */
    TZ_PR_CMD_IVCHECK_TOGGLE,          /**< Enable/Disable the IV contsraint check */
    TZ_PR_CMD_COPY,                    /**< Playready Copy nonsecure to secure buffers */
    TZ_PR_CMD_UNKNOWN         = PLAYREADY_CREATE_CMD(0x7FFFFFFF)
} tz_pr_cmd_type;



/** Command structure for saving prov keys
*/
typedef struct tz_pr_save_key_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /** Key file to provision */
    uint8                     key;
    /** Length of the data */
    uint32                    msg_len;
    /** data for the keys */
    uint8                     msg_data[TZ_PR_PROV_PKG_SIZE];
} __attribute__ ((packed)) tz_pr_save_key_req_t;

typedef struct tz_pr_save_key_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /** Path to the created file */
    uint8                     prt_path[TZ_PR_MAX_NAME_LEN];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_pr_save_key_rsp_t;


/** Command structure for verifying prov keys
*/
typedef struct tz_pr_verify_key_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
} __attribute__ ((packed)) tz_pr_verify_key_req_t;

typedef struct tz_pr_verify_key_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /** Path to the created file */
    uint8                     prt_path[TZ_PR_MAX_NAME_LEN];
    /** Path of drmms_sfs */
    uint8                     prt_path_drmms_sfs[TZ_PR_MAX_NAME_LEN];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_pr_verify_key_rsp_t;


/** Command structure for deleting prov keys
*/
typedef struct tz_pr_delete_key_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
} __attribute__ ((packed)) tz_pr_delete_key_req_t;

typedef struct tz_pr_delete_key_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /** Path of drmms */
    uint8                     prt_path_drmms[TZ_PR_MAX_NAME_LEN];
    /** Path of drmms_sfs */
    uint8                     prt_path_drmms_sfs[TZ_PR_MAX_NAME_LEN];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_pr_delete_key_rsp_t;


/** Command structure for initializing PR Stack
*/
typedef struct tz_pr_init_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
} __attribute__ ((packed)) tz_pr_init_req_t;

typedef struct tz_pr_init_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /** Application Context */
    uint32                    app_ctx;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_pr_init_rsp_t;


/** Command structure for un-initializing PR Stack
*/
typedef struct tz_pr_uninit_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /** Application Context */
    uint32                    app_ctx;
    /** Decrypt Context */
    uint32                    dec_ctx;
    /** Domain Context */
    uint32                    dom_ctx;
} __attribute__ ((packed)) tz_pr_uninit_req_t;

typedef struct tz_pr_uninit_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
} __attribute__ ((packed)) tz_pr_uninit_rsp_t;


/** Command structure to get license chlg from DRM agent
*/
typedef struct tz_pr_gen_chlg_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Size of license */
    uint32                  liclen;
    /** Array of rights pointers  */
    tzconststring_t         rights[TZ_PR_MAX_RIGHTS_SIZE];
    /** Size of rights array */
    uint32                  rightslen;
    /** Pointer to the domain id */
    tzdrmdomainid_t         domainid;
    /** Pointer to the custom data */
    uint8                   custdata[TZ_PR_MAX_CUSTOM_DATA_SIZE];
    /** Size of custom data */
    uint32                  custdatalen;
    /** Size of PSI obj */
    uint32                  silen;
    /** Size of PNONSI obj */
    uint32                  nonsilen;
} __attribute__ ((packed)) tz_pr_gen_chlg_req_t;

typedef struct tz_pr_gen_chlg_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** size of license */
    uint32                  liclen;
    /** Pointer to license challenge generated */
    uint8                   lic[TZ_PR_REM_SIZE - (SUM(tz_pr_gen_chlg_req_t,uint32,uint32,uint32)) - (TZ_PR_MAX_PSIURL_SIZE+TZ_PR_MAX_PNONSIURL_SIZE) - sizeof(uint32)];
    /** Size of PSI obj */
    uint32                  silen;
    /** Pointer to silent LA URL generated */
    uint8                   si[TZ_PR_MAX_PSIURL_SIZE];
    /** Size of PNONSI obj */
    uint32                  nonsilen;
    /** Pointer to nonsilent LA URL generated */
    uint8                   nonsi[TZ_PR_MAX_PNONSIURL_SIZE];
    /** Array to hold the rights array address */
    uint32                  addr[TZ_PR_MAX_RIGHTS_SIZE];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_gen_chlg_rsp_t;


/** Command structure to process response
*/
typedef struct tz_pr_proc_license_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Size of license response */
    uint32                  len;
    /** Pointer to license response */
    uint8                   lic_rsp_ptr[TZ_PR_REM_SIZE - sizeof(tz_pr_cmd_type) - (sizeof(uint32) *2) - (sizeof(tzdrmlicensersp_t)*2)];
    /** License response structure */
    tzdrmlicensersp_t       licrsp;
} __attribute__ ((packed)) tz_pr_proc_license_req_t;

typedef struct tz_pr_proc_license_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** License response structure */
    tzdrmlicensersp_t       licrsp;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_proc_license_rsp_t;


/** Command structure to delete license
*/
typedef struct tz_pr_del_license_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Handle of license to delete */
    char                    KID[TZ_PR_MAX_KID_SIZE];
} __attribute__ ((packed)) tz_pr_del_license_req_t;

typedef struct tz_pr_del_license_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_del_license_rsp_t;


/** Command structure to delete license with BASE64 encoded input.
*/
typedef struct tz_pr_strmgmt_dellicense_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Handle of license to delete */
    uint8                   KID[TZ_PR_MAX_B64_KID_SIZE];
    /** Length of KID */
    uint32				  KIDLen;
} __attribute__ ((packed)) tz_pr_strmgmt_dellicense_req_t;

typedef struct tz_pr_strmgmt_dellicense_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
    /** Number of licenses deleted */
    uint32				  licDeleted;
} __attribute__ ((packed)) tz_pr_strmgmt_dellicense_rsp_t;



/** Command structure to set drm header
*/
typedef struct tz_pr_set_header_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** The size of the header data */
    uint32                  len;
    /** pointer to the header data */
    uint8                   header[TZ_PR_REM_SIZE];
} __attribute__ ((packed)) tz_pr_set_header_req_t;

typedef struct tz_pr_set_header_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /**<-- 0 for success and non-zero for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_set_header_rsp_t;

/** Command structure to decrypt DRM content
*/
typedef struct tz_pr_decrypt_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;

    /** Decrypt Context */
    uint32                  dec_ctx;

    /**< Number of entries in encryptionMetaData */
    uint8                   numEncryptedUnits;

    /**< Physical address of encrypted data */
    uint32                  in_phy;

    /**< the size of input buffer */
    uint32                  input_length;

    /**< Physical address of decrypted data */
    uint32                  out_phy;

    /**< Contains a list of block to decrypt with associated IV etc. */
    tzEncryptionMetaData_t    encryptionMetaData[TZ_PR_MAX_ENCRYPTION_UNITS];

} __attribute__ ((packed)) tz_pr_decrypt_req_t;

/** Command structure to decrypt DRM content with 64bit address support
*/
typedef struct tz_pr_decrypt_64bit_req_s
{
  /** First 4 bytes should always be command id */
  tz_pr_cmd_type          pr_cmd;

  /** Decrypt Context */
  uint32                  dec_ctx;

  /**< Number of entries in encryptionMetaData */
  uint8                   numEncryptedUnits;

  /**< Physical address of encrypted data */
  uint64                  in_phy;

  /**< the size of input buffer */
  uint32                  input_length;

  /**< Physical address of decrypted data */
  uint64                  out_phy;

  /**< Contains a list of block to decrypt with associated IV etc. */
  tzEncryptionMetaData_t    encryptionMetaData[TZ_PR_MAX_ENCRYPTION_UNITS];

} __attribute__ ((packed)) tz_pr_decrypt_64bit_req_t;
typedef struct tz_pr_decrypt_video_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;

    /** Decrypt Context */
    uint32                  dec_ctx;

    /**< Number of entries in encryptionMetaData */
    uint8                   numEncryptedUnits;

    /**< Physical address of encrypted data */
    uint32                  in_phy;

    /**< the size of input buffer */
    uint32                  input_length;

    /**< Physical address of decrypted data */
    tz_buf_array_s_t        out_phy;

    /**< Contains a list of block to decrypt with associated IV etc. */
    tzEncryptionMetaData_t    encryptionMetaData[TZ_PR_MAX_ENCRYPTION_UNITS];

} __attribute__ ((packed)) tz_pr_decrypt_video_req_t;

typedef struct tz_pr_decrypt_video_64bit_req_s
{
  /** First 4 bytes should always be command id */
  tz_pr_cmd_type          pr_cmd;

  /** Decrypt Context */
  uint32                  dec_ctx;

  /**< Number of entries in encryptionMetaData */
  uint8                   numEncryptedUnits;

  /**< Physical address of encrypted data */
  uint64                  in_phy;

  /**< the size of input buffer */
  uint32                  input_length;

  /**< Physical address of decrypted data */
  tz_buf_64bit_array_s_t  out_phy;

  /**< Contains a list of block to decrypt with associated IV etc. */
  tzEncryptionMetaData_t  encryptionMetaData[TZ_PR_MAX_ENCRYPTION_UNITS];

} __attribute__ ((packed)) tz_pr_decrypt_video_64bit_req_t;
typedef struct tz_pr_decrypt_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_decrypt_rsp_t;

/** Command structure to InitDecrypt
*/
typedef struct tz_pr_init_decrypt_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;

    /** Decrypt Context */
    uint32                  dec_ctx;

    /**< Physical address of encrypted data */
    uint8                  in_phy[15];

    /**< Size of input buffer */
    uint32                  input_length;

} __attribute__ ((packed)) tz_pr_init_decrypt_req_t;

typedef struct tz_pr_init_decrypt_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_init_decrypt_rsp_t;

/** Command structure to get license state
*/
typedef struct tz_pr_get_license_rights_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
} __attribute__ ((packed)) tz_pr_get_license_rights_req_t;

typedef struct tz_pr_get_license_rights_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /**< Contains a list of block to decrypt with associated IV etc. */
    tzlicensestatedata_t      licStateData[TZ_PR_MAX_LIC_STATE_UNITS];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_pr_get_license_rights_rsp_t;

/** Command structure to bind the license
*/
typedef struct tz_pr_bind_license_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Array of rights pointers  */
    tzconststring_t         rights[TZ_PR_MAX_RIGHTS_SIZE];
    /** Size of rights array */
    uint32                  rightslen;
} __attribute__ ((packed)) tz_pr_bind_license_req_t;

typedef struct tz_pr_bind_license_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /** Decrypt Context */
    uint32                    dec_ctx;
    /** Array to hold the rights array address */
    uint32                    addr[TZ_PR_MAX_RIGHTS_SIZE];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_pr_bind_license_rsp_t;


typedef struct tz_pr_enc_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Plain data length */
    uint32                  len;
    /** Plain data */
    uint8                   data[9500];
    /** Decrypt context */
    uint32                  dec_ctx;
    /** Type of stream */
    tz_pr_stream_type       streamType;
} __attribute__ ((packed)) tz_pr_enc_req_t;

typedef struct tz_pr_enc_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /** Cipher data */
    uint8                     data[TZ_PR_REM_SIZE - sizeof(tz_pr_enc_req_t)];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_pr_enc_rsp_t;


/** Command structure to commit the license to store
*/
typedef struct tz_pr_commit_license_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
} __attribute__ ((packed)) tz_pr_commit_license_req_t;

typedef struct tz_pr_commit_license_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_pr_commit_license_rsp_t;

/** Command structure to remove the header information from DRM context structure
*/
typedef struct tz_pr_reinit_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Decrypt Context  */
    uint32                    dec_ctx;
    /** Domain Context */
    uint32                    dom_ctx;
} __attribute__ ((packed)) tz_pr_reinit_req_t;

typedef struct tz_pr_reinit_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            cmd_id;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_pr_reinit_rsp_t;


/** Command structure to get synchronization chlg from DRM agent
*/
typedef struct tz_pr_sync_gen_chlg_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Maximum play count remaining */
    uint32                  max_cnt;
    /** Maximum hours remaining  */
    uint32                  max_hrs;
    /** Index to start */
    uint32                  start_index;
    /** Number of items to process  */
    uint32                  items_to_process;
    /** Size of the synchronization challenge */
    uint32                  synclen;
    /** Application Context */
    uint32                  app_ctx;
} __attribute__ ((packed)) tz_pr_sync_gen_chlg_req_t;

typedef struct tz_pr_sync_gen_chlg_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Size of synchronization challenge */
    uint32                  len;
    /** Pointer to synchronization challenge generated */
    uint8                   sync_list[TZ_PR_REM_SIZE - (SUM(tz_pr_sync_gen_chlg_req_t,uint32,uint32,uint32))];
    /**Next index to start */
    uint32                  next_index;
    /** Number of items processed  */
    uint32                  items_processed;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_sync_gen_chlg_rsp_t;


/** Command structure to get meter chlg from DRM agent
*/
typedef struct tz_pr_meter_gen_chlg_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** size of meter certficate */
    uint32                  metercert_len;
    /** size of meter challenge */
    uint32                  meterlen;
    /** Pointer to PlayReady PSI object */
    uint8                   metercert[TZ_PR_MAX_METER_CERT_SIZE];
    /** Application Context */
    uint32                  app_ctx;
} __attribute__ ((packed)) tz_pr_meter_gen_chlg_req_t;

typedef struct tz_pr_meter_gen_chlg_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Size of meter challenge */
    uint32                  len;
    /** Pointer to meter challenge generated */
    uint8                   meter_chlg[TZ_PR_REM_SIZE - sizeof(tz_pr_meter_gen_chlg_req_t) - sizeof(uint32)];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_meter_gen_chlg_rsp_t;


/** Command structure to process meter response
*/
typedef struct tz_pr_proc_meter_data_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Size of license response */
    uint32                  len;
    /** Pointer to license response */
    uint8                   meter_rsp_ptr[TZ_PR_REM_SIZE - (SUM(tz_pr_cmd_type,uint32,uint32,uint32))];
    /** Application Context */
    uint32                  app_ctx;
} __attribute__ ((packed)) tz_pr_proc_meter_data_req_t;

typedef struct tz_pr_proc_meter_data_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Meter response flag */
    uint32                  meter_flag;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_proc_meter_data_rsp_t;


/** Command structure to clean data store
*/
typedef struct tz_pr_clean_data_store_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /** Application Context */
    uint32                    app_ctx;
    /** Cleanup Type */
    uint32					flags;
} __attribute__ ((packed)) tz_pr_clean_data_store_req_t;

typedef struct tz_pr_clean_data_store_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_pr_clean_data_store_rsp_t;

/** Command structure to clean data store
*/
typedef struct tz_pr_reset_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /** Reset options Flags */
    uint32					flags;
} __attribute__ ((packed)) tz_pr_reset_req_t;

typedef struct tz_pr_reset_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_pr_reset_rsp_t;

/** Command structure to get license state
*/
typedef struct tz_pr_get_license_state_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /** Query data */
    const unsigned short      querydata[TZ_PR_REM_SIZE - (SUM(tz_pr_cmd_type,uint32,uint32,uint32))];
    /** Size of query data */
    uint32                    querydata_len;
    /** Application Context */
    uint32                    app_ctx;
} __attribute__ ((packed)) tz_pr_get_license_state_req_t;

typedef struct tz_pr_get_license_state_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    /** License state category */
    uint32                    lic_category;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                     ret;
} __attribute__ ((packed)) tz_pr_get_license_state_rsp_t;


/** Command structure to get device cert from DRM agent
*/
typedef struct tz_pr_get_device_cert_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** size of device certificate */
    uint32                  devcert_len;
} __attribute__ ((packed)) tz_pr_get_device_cert_req_t;

typedef struct tz_pr_get_device_cert_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** size of device certificate */
    uint32                  len;
    /** Pointer to device certificate */
    uint8                   device_cert[TZ_PR_REM_SIZE - sizeof(tz_pr_get_device_cert_req_t) - sizeof(uint32)];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_get_device_cert_rsp_t;


/** Command structure to process WMDRM command
*/
typedef struct tz_pr_proc_command_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Operation code */
    uint32                  opcode;
    /** Size of request data */
    uint32                  len;
    /** Pointer to request data */
    uint8                   req_data_ptr[TZ_PR_REM_SIZE - (SUM(tz_pr_cmd_type,uint32,uint32,uint32)) - (sizeof(uint32) *8)];
    /** Request argument 1 */
    uint32                  req_arg1;
    /** Request argument 2 */
    uint32                  req_arg2;
    /** Request argument 3 */
    uint32                  req_arg3;
    /** Request argument 4 */
    uint32                  req_arg4;
    /** Application Context */
    uint32                  app_ctx;
} __attribute__ ((packed)) tz_pr_proc_command_req_t;

typedef struct tz_pr_proc_command_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Response argument 1 */
    uint32                  rsp_arg1;
    /** Response argument 2 */
    uint32                  rsp_arg2;
    /** Response argument 3 */
    uint32                  rsp_arg3;
    /** Response argument 4 */
    uint32                  rsp_arg4;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_proc_command_rsp_t;


/** Command structure to process WMDRM request
*/
typedef struct tz_pr_proc_request_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Operation code */
    uint32                  opcode;
    /** Request argument 1 */
    uint32                  req_arg1;
    /** Request argument 2 */
    uint32                  req_arg2;
    /** Request argument 3 */
    uint32                  req_arg3;
    /** Request argument 4 */
    uint32                  req_arg4;
    /** Application Context */
    uint32                  app_ctx;
} __attribute__ ((packed)) tz_pr_proc_request_req_t;

typedef struct tz_pr_proc_request_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Size of response data */
    uint32                  rsplen;
    /** Pointer to response data */
    uint8                   rsp_data_ptr[TZ_PR_REM_SIZE - sizeof(tz_pr_proc_request_req_t) - (sizeof(uint32)*5)];
    /** Response argument 1 */
    uint32                  rsp_arg1;
    /** Response argument 2 */
    uint32                  rsp_arg2;
    /** Response argument 3 */
    uint32                  rsp_arg3;
    /** Response argument 4 */
    uint32                  rsp_arg4;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_proc_request_rsp_t;


/** Command structure to open an enveloped file
*/
typedef struct tz_pr_env_open_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Filename of the envelope */
    unsigned short          filename[TZ_PR_MAX_NAME_LEN];
    /** Application Context */
    uint32                  app_ctx;
} __attribute__ ((packed)) tz_pr_env_open_req_t;

typedef struct tz_pr_env_open_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Envelope Context */
    uint32                  env_ctx;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_env_open_rsp_t;


/** Command structure to associate the decrypt context to the.file
*/
typedef struct tz_pr_env_init_read_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Envelope Context */
    uint32                  env_ctx;
    /** Decrypt Context */
    uint32                  dec_ctx;
} __attribute__ ((packed)) tz_pr_env_init_read_req_t;

typedef struct tz_pr_env_init_read_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_env_init_read_rsp_t;


/** Command structure to do a envelope file seek
*/
typedef struct tz_pr_env_seek_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Envelope Context */
    uint32                  env_ctx;
    /** Distance to move */
    uint32                  distomov;
    /** Move method */
    uint32                  movmethod;
} __attribute__ ((packed)) tz_pr_env_seek_req_t;

typedef struct tz_pr_env_seek_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** New location in the file */
    uint32                  new_ptr;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_env_seek_rsp_t;


/** Command structure to read the clear data
*/
typedef struct tz_pr_env_read_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Envelope Context */
    uint32                  env_ctx;
    /** Number of bytes to read */
    uint32                  len;
} __attribute__ ((packed)) tz_pr_env_read_req_t;

typedef struct tz_pr_env_read_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Pointer to the file data */
    uint8                   buffer[TZ_PR_REM_SIZE - sizeof(tz_pr_env_read_req_t) - sizeof(uint32)];
    /** Number of bytes read */
    uint32                  bytesread;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_env_read_rsp_t;


/** Command structure to close an enveloped file
*/
typedef struct tz_pr_env_close_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Envelope Context */
    uint32                  env_ctx;
} __attribute__ ((packed)) tz_pr_env_close_req_t;

typedef struct tz_pr_env_close_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_env_close_rsp_t;


/** Command structure to get the envelope file size
*/
typedef struct tz_pr_env_get_size_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Envelope Context */
    uint32                  env_ctx;
} __attribute__ ((packed)) tz_pr_env_get_size_req_t;

typedef struct tz_pr_env_get_size_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Size of the envelope file */
    uint32                  len;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_env_get_size_rsp_t;


/** Command structure to get the original filename of envelope
*/
typedef struct tz_pr_env_get_orig_fname_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Envelope Context */
    uint32                  env_ctx;
    /** Length of the  file name */
    uint32                  len;
} __attribute__ ((packed)) tz_pr_env_get_orig_fname_req_t;

typedef struct tz_pr_env_get_orig_fname_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Length of the  file name */
    uint32                  len;
    /** Pointer to the file name  */
    unsigned short          filename[TZ_PR_MAX_NAME_LEN];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_env_get_orig_fname_rsp_t;


/** Command structure to duplicate envelope file context
*/
typedef struct tz_pr_env_dup_filectx_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Envelope Context */
    uint32                  env_ctx;
    /** Pointer to the file name  */
    unsigned short          filename[TZ_PR_MAX_NAME_LEN];
} __attribute__ ((packed)) tz_pr_env_dup_filectx_req_t;

typedef struct tz_pr_env_dup_filectx_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Envelope Context */
    uint32                  env_ctx_new;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_env_dup_filectx_rsp_t;


/** Command structure to update the embedded store
*/
typedef struct tz_pr_update_embstore_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
} __attribute__ ((packed)) tz_pr_update_embstore_req_t;

typedef struct tz_pr_update_embstore_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_update_embstore_rsp_t;


/** Command structure to write the Playready object to the envelope file
*/
typedef struct tz_pr_env_write_plrdobj_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Envelope Context */
    uint32                  env_ctx;
    /** Pointer to the file name  */
    unsigned short          filename[TZ_PR_MAX_NAME_LEN];
} __attribute__ ((packed)) tz_pr_env_write_plrdobj_req_t;

typedef struct tz_pr_env_write_plrdobj_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Envelope Context */
    uint32                  env_ctx;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_env_write_plrdobj_rsp_t;


/** Command structure to commit the license to embedded store
*/
typedef struct tz_pr_update_embstore_commit_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
} __attribute__ ((packed)) tz_pr_update_embstore_commit_req_t;

typedef struct tz_pr_update_embstore_commit_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_update_embstore_commit_rsp_t;

/** Command structure to get content property from DRM Agent.
*/
typedef struct tz_pr_get_property_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Size of the DRM property data */
    uint32                  len;
    /** Get property type */
    tzdrmgetproptype        getproptype;
} __attribute__ ((packed)) tz_pr_get_property_req_t;

typedef struct tz_pr_get_property_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Size of the DRM property data */
    uint32                  len;
    /** Pointer to the DRM property data */
    uint8                   pdata[TZ_PR_REM_SIZE - sizeof(tz_pr_get_property_req_t) - sizeof(uint32)];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_get_property_rsp_t;

/** Command structure to set the domain id.
*/
typedef struct tz_pr_set_domainid_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Pointer to the service id */
    uint8                   servid[TZ_PR_MAX_NAME_LEN];
    /** Size of the service id */
    uint32                  servidlen;
    /** Pointer to the account  id */
    uint8                   accid[TZ_PR_MAX_NAME_LEN];
    /** Size of the account  id */
    uint32                  accidlen;
    /** Revision associated with the domain */
    uint32                  rev;
} __attribute__ ((packed)) tz_pr_set_domainid_req_t;

typedef struct tz_pr_set_domainid_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Domain ID structure*/
    tzdrmdomainid_t         domainid;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_set_domainid_rsp_t;


/** Command structure to get join domain chlg from DRM Agent.
*/
typedef struct tz_pr_joindomain_gen_chlg_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Custom data flag */
    uint32                  flag;
    /** Domain ID structure*/
    tzdrmdomainid_t         domainid;
    /** Pointer to the friendly name */
    uint8                   friendlyname[TZ_PR_MAX_NAME_LEN];
    /** Size of the friendly name */
    uint32                  friendlynamelen;
    /** Pointer to the custom data */
    uint8                   custdata[TZ_PR_MAX_CUSTOM_DATA_SIZE];
    /** Size of the custom data */
    uint32                  custdatalen;
    /** Size of the join domain challenge */
    uint32                  len;
} __attribute__ ((packed)) tz_pr_joindomain_gen_chlg_req_t;

typedef struct tz_pr_joindomain_gen_chlg_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Size of the join domain challenge */
    uint32                  len;
    /** Pointer to the file name  */
    uint8                   joindom_chlg[TZ_PR_REM_SIZE - sizeof(tz_pr_joindomain_gen_chlg_req_t) - sizeof(uint32)];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_joindomain_gen_chlg_rsp_t;


/** Command structure to process the join domain response.
*/
typedef struct tz_pr_joindomain_proc_resp_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Size of the domain join response */
    uint32                  joindom_rsp_len;
    /** Pointer to the domain join response */
    uint8                   joindom_rsp_ptr[TZ_PR_REM_SIZE - (SUM(tz_pr_cmd_type,uint32,uint32,int32)) - sizeof(tzdrmdomainid_t)];
} __attribute__ ((packed)) tz_pr_joindomain_proc_resp_req_t;

typedef struct tz_pr_joindomain_proc_resp_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** DRM Result */
    int32                   result;
    /** Domain ID structure */
    tzdrmdomainid_t         domainid;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_joindomain_proc_resp_rsp_t;


/** Command structure to get leave domain chlg from DRM Agent.
*/
typedef struct tz_pr_leavedomain_gen_chlg_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Custom data flag */
    uint32                  flag;
    /** Domain ID structure*/
    tzdrmdomainid_t         domainid;
    /** Pointer to the custom data */
    uint8                   custdata[TZ_PR_MAX_CUSTOM_DATA_SIZE];
    /** Size of the custom data */
    uint32                  custdatalen;
    /** Size of the leave domain challenge */
    uint32                  len;
} __attribute__ ((packed)) tz_pr_leavedomain_gen_chlg_req_t;

typedef struct tz_pr_leavedomain_gen_chlg_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Size of the leave domain challenge */
    uint32                  len;
    /** Pointer to the file name  */
    uint8                   leavedom_chlg[TZ_PR_REM_SIZE - sizeof(tz_pr_leavedomain_gen_chlg_req_t) - sizeof(uint32)];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_leavedomain_gen_chlg_rsp_t;


/** Command structure to process the leave domain response.
*/
typedef struct tz_pr_leavedomain_proc_resp_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Size of the domain leave response */
    uint32                  leavedom_rsp_len;
    /** Pointer to the domain leave response */
    uint8                   leavedom_rsp_ptr[TZ_PR_REM_SIZE - (SUM(tz_pr_cmd_type,uint32,uint32,int32))];
} __attribute__ ((packed)) tz_pr_leavedomain_proc_resp_req_t;

typedef struct tz_pr_leavedomain_proc_resp_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** DRM Result */
    int32                   result;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_leavedomain_proc_resp_rsp_t;


/** Command structure to retrieve a specific domain certificate.
*/
typedef struct tz_pr_domain_cert_find_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Size of the domain certificate */
    uint32                  len;
    /** Domain ID structure */
    tzdrmdomainid_t         domainid;
} __attribute__ ((packed)) tz_pr_domain_cert_find_req_t;

typedef struct tz_pr_domain_cert_find_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** DRM Result */
    uint32                  len;
    /** Pointer to the domain certificate */
    uint8                   domaincert[TZ_PR_REM_SIZE - sizeof(tz_pr_domain_cert_find_req_t) - sizeof(uint32)];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_domain_cert_find_rsp_t;


/** Command structure to initialize the domain enumeration.
*/
typedef struct tz_pr_domain_cert_initenum_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
} __attribute__ ((packed)) tz_pr_domain_cert_initenum_req_t;

typedef struct tz_pr_domain_cert_initenum_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Domain Context */
    uint32                  dom_ctx;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_domain_cert_initenum_rsp_t;


/** Command structure to retrieve the next item in the domain enumeration.
*/
typedef struct tz_pr_domain_cert_enumnext_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Domain Context */
    uint32                  dom_ctx;
} __attribute__ ((packed)) tz_pr_domain_cert_enumnext_req_t;

typedef struct tz_pr_domain_cert_enumnext_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** DRM Result */
    uint32                  len;
    /** Pointer to the domain certificate */
    tzdrmdomaincertinfo_t   domaincertinfo;
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_domain_cert_enumnext_rsp_t;


/** Command structure to process response
*/
typedef struct tz_pr_getaddn_rspdata_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx;
    /** Size of server response */
    uint32                  server_rsp_len;
    /** Pointer to server response */
    uint8                   server_rsp_ptr[TZ_PR_MAX_CHG_SIZE];
    /** Data element type */
    uint32                  datatype;
    /** Data string length */
    uint32                  len;
} __attribute__ ((packed)) tz_pr_getaddn_rspdata_req_t;

typedef struct tz_pr_getaddn_rspdata_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Data string length */
    uint32                  len;
    /** Data string */
    uint8                   datastr[TZ_PR_REM_SIZE - sizeof(tz_pr_getaddn_rspdata_req_t) - sizeof(uint32)];
    /**<-- E_SUCCESS for success and E_FAILURE for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_getaddn_rspdata_rsp_t;


/** Command structure to disable parsing the non-silent url in drm header
    to validate envelope use case using the demo apk
*/
typedef struct tz_pr_parse_nonsilenturl_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /*flag to turn off the non silent url parsing s*/
    uint8                   bParsenonsilenturl;
} __attribute__ ((packed)) tz_pr_parse_nonsilenturl_req_t;

typedef struct tz_pr_parse_nonsilenturl_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type            pr_cmd;
    int32                     ret;
} __attribute__ ((packed)) tz_pr_parse_nonsilenturl_rsp_t;

/** Command structure to set robustness version
*/
typedef struct tz_pr_set_robustness_ver_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /*Robustness version*/
    uint32                  robustness_version;
} __attribute__ ((packed)) tz_pr_set_robustness_ver_req_t;

typedef struct tz_pr_set_robustness_ver_rsp_s
{
    /** First 4 bytes are always command id */
    tz_pr_cmd_type          pr_cmd;
    /**<-- Return value for maintenance */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_set_robustness_ver_rsp_t;

/** Command structure for Duplicate Context
*/
typedef struct tz_pr_dup_ctx_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  ctx_id;
} __attribute__ ((packed)) tz_pr_dup_ctx_req_t;

typedef struct tz_pr_dup_ctx_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  ctx_id;
    /** zero for success and non-zero for failures */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_dup_ctx_rsp_t;

/** Command structure to set drm content property
*/
typedef struct tz_pr_content_set_property_req_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /** Application Context */
    uint32                  app_ctx_id;
    /** Set property type */
    uint32                  set_property_type;
    /** pointer to the header data */
    uint8                   data[TZ_PR_MAX_CUSTOM_DATA_SIZE];
    /** The size of the header data */
    uint32                  size;
} __attribute__ ((packed)) tz_pr_content_set_property_req_t;

typedef struct tz_pr_content_set_property_rsp_s
{
    /** First 4 bytes should always be command id */
    tz_pr_cmd_type          pr_cmd;
    /**<-- 0 for success and non-zero for failure */
    int32                   ret;
} __attribute__ ((packed)) tz_pr_content_set_property_rsp_t;

/** Command structure for saving aes and keyfile data
*/
typedef struct tz_pr_save_aeskeyfile_req_s
{
  /** First 4 bytes should always be command id */
  tz_pr_cmd_type            pr_cmd;
  /** Key file to provision */
  uint8                     key;
  /** Length of the data */
  uint32                    msg_len;
  /** data for the keys */
  uint8                     msg_data[TZ_PR_PROV_PKG_SIZE];
} __attribute__ ((packed)) tz_pr_save_aeskeyfile_req_t;

typedef struct tz_pr_save_aeskeyfile_rsp_s
{
  /** First 4 bytes should always be command id */
  tz_pr_cmd_type            pr_cmd;
  /** Path to the created file */
  uint8                     prt_path[TZ_PR_MAX_NAME_LEN];
  /**<-- E_SUCCESS for success and E_FAILURE for failure */
  long                      ret;
} __attribute__ ((packed)) tz_pr_save_aeskeyfile_rsp_t;

/** Command structure to set IV contsraint test flag
*/
typedef struct tz_pr_ivcheck_toggle_req_s
{
  /** First 4 bytes should always be command id */
  tz_pr_cmd_type          pr_cmd;
  /*flag to turn on/off the IV contsraint check */
  uint8                   bIVconstraint;
} __attribute__ ((packed)) tz_pr_ivcheck_toggle_req_t;

typedef struct tz_pr_ivcheck_toggle_rsp_s
{
  /** First 4 bytes should always be command id */
  tz_pr_cmd_type          pr_cmd;
  long                    ret;
} __attribute__ ((packed)) tz_pr_ivcheck_toggle_rsp_t;

/** Command structure for Copy
*/
typedef struct tz_pr_copy_req_s
{
  /** First 4 bytes should always be command id */
  uint32                          cmd_id;
  uint32                          copyDir;
  uint32                          nonSecBuffer;
  uint32                          nonSecBufferLength;
  tz_buf_array_s_t                secBufferHandle;
  uint32                          secBufferOffset;
} __attribute__ ((packed)) tz_pr_copy_req_t;

typedef struct tz_pr_copy_rsp_s
{
  /** First 4 bytes should always be command id */
  uint32                          cmd_id;
  uint32                          secBufferLength;
  long                            ret;
} __attribute__ ((packed)) tz_pr_copy_rsp_t;

#endif /* __TZ_PLAYREADY_H_ */
