#ifndef FWLOCK_H
#define FWLOCK_H

/** @file fwlock.h
 * @brief
 * This file contains the definitions of the constants, data structures
 * and interfaces for the Firmware Lock module
 */
/*===========================================================================

Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

 ===========================================================================*/

/*===========================================================================

 EDIT HISTORY FOR FILE

 when         who     what, where, why
 ----------   ---     -------------------------------------------------------
 04/06/2014   ablay   Initial Version for a Demo.
 06/10/2014   ablay   Simplified API, added PKCS7 message support.

 ===========================================================================*/

/*----------------------------------------------------------------------------
 * Macro Declarations
 * -------------------------------------------------------------------------*/

#define FWLOCK_TOKEN_LEN 16
#define FWLOCK_BLOCK_SIZE 1024*1024*16
#define FWLOCK_MAX_PIN_LEN 8
#define FWLOCK_MAX_BLOCKS 1024
#define FWLOCK_MAX_PARTITIONS 3
#define FWLOCK_MAX_PARTITION_NAME_LEN 100
#define FWLOCK_MAX_PKCS7_MSG_LEN 2048
#define FWLOCK_MAX_CRL_LEN 2048

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

#pragma pack(push, fwlock, 1)

/** Firmware Lock error types. */
typedef enum {
    FWLOCK_SUCCESS = 0,                 /**< Success error code. */
    FWLOCK_FAILURE,                     /**< Failure error code. */
    FWLOCK_ERR_SIZE = 0x7FFFFFFF
} fwlock_status_t;

/** Firmware PKCS7 message types. */
typedef enum {
    FWLOCK_MSG_ACTIVATION = 0,          /**< Lock activation message type. */
    FWLOCK_MSG_DEACTIVATION,            /**< Lock deactivation message type. */
    FWLOCK_MSG_SIZE = 0x7FFFFFFF
} fwlock_msg_type_t;

/** Firmware Lock state. */
typedef enum {
    FWLOCK_STATE_NOT_SUPPORTED = 0,     /**< FWLock is not supported on the device. */
    FWLOCK_STATE_ACTIVE,                /**< FWLock is active on the device; the firmware is locked. */
    FWLOCK_STATE_DEACTIVE,              /**< FWLock is not activated on the device; the firmware is not locked. */
    FWLOCK_STATE_SIZE = 0x7FFFFFFF
} fwlock_state_t;

/** Firmware Lock Hash mode. */
typedef enum {
    FWLOCK_MODE_SINGLE = 0,             /**< A single hash on the entire partition. */
    FWLOCK_MODE_BLOCK,                  /**< A hash per FWLOCK_BLOCK_SIZE sized block. */
    FWLOCK_MODE_SIZE = 0x7FFFFFFF
} fwlock_mode_t;

/** Firmware Lock User Authenticator mode. */
typedef enum {
    FWLOCK_AUTH_UI_PIN = 0,         /**< Locally authenticate the user using the PIN entered via the UI. */
    FWLOCK_AUTH_TUI_PIN,            /**< Locally authenticate the user using the PIN entered via a Trusted UI. */
    FWLOCK_AUTH_SIZE = 0x7FFFFFFF
} fwlock_local_auth_mode_t;

/** Firmware Lock user authenticator for local deactivation. */
typedef struct fwlock_local_auth {
    fwlock_local_auth_mode_t local_auth_mode;       /**< User authenticator mode for local deactivation. */
    union {
      char local_auth_ui_pin[FWLOCK_MAX_PIN_LEN+1]; /**< PIN provided by the user for a local auhenticator of type FWLOCK_AUTH_UI_PIN. */
    } local_auth_data;
} fwlock_user_auth_t;

/** Firmware Lock partition configuration. */
typedef struct fwlock_partition_cfg {
    char name[FWLOCK_MAX_PARTITION_NAME_LEN];       /**< Partition name. */
    fwlock_mode_t hash_mode;                        /**< Single hash or hash per block. */
    uint8 verify_ratio;                             /**< Statistically verify only a given partition ratio (1-100). */
    uint8 force_verify_block[FWLOCK_MAX_BLOCKS];    /**< Verify only given block numbers. */
} fwlock_partition_cfg_t;

/** Firmware Lock configuration. */
typedef struct fwlock_lock_cfg {
    uint8 num_partitions;                   /**< Number of partitions to protect. */
    fwlock_partition_cfg_t *partitions;     /**< Lock configuration per partition. */
    uint8 enable_local_deactivation;        /**< Allow local lock deactivation using a user authenticator. */
    fwlock_user_auth_t user_auth;           /**< User authenticator, such as a PIN. */
} fwlock_lock_cfg_t;

/** Firmware Lock sized buffer type. */
typedef struct fwlock_buffer {
    size_t len;             /**< Buffer length in bytes. */
    unsigned char* data;    /**< Buffer data. */
} fwlock_buffer_t;

/** Firmware Lock activation token. */
typedef struct fwlock_token {
    unsigned char data[FWLOCK_TOKEN_LEN];   /**< UUID representing a lock activation. */
} fwlock_token_t;

/** Firmware Lock activation and deactivation message data. */
typedef struct fwlock_msg_data {
    fwlock_msg_type_t msg_type;         /**< Type of Firmware Lock message. */
    fwlock_token_t token;               /**< Activation token. */
} fwlock_msg_data_t;

#pragma pack(pop, fwlock)

/*----------------------------------------------------------------------------
 * FWLock API functions
 * -------------------------------------------------------------------------*/

/*****************************************************************************************************************/

/**
  Provides a nondefault configuration for activation/deactivation.

  @param[in]  lock_config   Lock configuration for the activation/deactication.

  @return
  FWLOCK_SUCCESS -- Success. \n
  FWLOCK_FAILURE -- Failure.

  @dependencies
  None.
*/
fwlock_status_t fwlock_lib_set_config(
    fwlock_lock_cfg_t lock_config);

/*****************************************************************************************************************/

/**
  Activates the Firmware Lock protection, given a signed PKCS7 request from a cloud service.

  @param[in]    activation_msg
                PKCS7 message sent by an ISV cloud service. The PKCS7 message
                should be a SignedData message, as specified in RFC2315 -
                "PKCS #7: Cryptographic Message Syntax":

    @code
     SignedData ::= SEQUENCE {
       version Version,
       digestAlgorithms
       DigestAlgorithmIdentifiers,
       contentInfo ContentInfo,
       certificates ExtendedCertificatesAndCertificates
       signerInfos SignerInfos
     }

     ContentInfo ::= SEQUENCE {
       contentType ContentType = <data>,
       content fwlock_msg_data_t;
     }
    @endcode

 A SignedData structure defined by RFC2315 holds various PKCS7 message components.
 The content of the message itself is embedded in the ContentInfo component.
 The above code snippet demonstrates how the PKCS7 message should be constructed,
 and where the actual message content should be placed.

  @param[out] blob          Blob related to activation to be stored in the cloud service.

  @return
  FWLOCK_SUCCESS -- Success. \n
  FWLOCK_FAILURE -- Failure.

  @dependencies
  A SignedData structure should be constructed as demonstrated above.
	Inside the SignedData structure, the fwlock_msg_data_t parameter should be set to:
  fwlock_msg_data_t.msg_type = FWLOCK_MSG_ACTIVATION;
  fwlock_msg_data_t.token = token;

*/
fwlock_status_t fwlock_lib_activate(
    fwlock_buffer_t activation_msg,
    fwlock_buffer_t *blob);

/*****************************************************************************************************************/

/**
  Deactivates the Firmware Lock protection, given a signed PKCS7 request
  from a cloud service.

  @param[in]    deactivation_msg A PKCS7 formatted request
                for deactivation. See #fwlock_lib_activate.

  The fwlock_activation_data_t parameter should be set to: \n

  @return
  FWLOCK_SUCCESS -- Success. \n
  FWLOCK_FAILURE -- Failure.

  @dependencies
  A SignedData structure should be constructed as demonstrated in fwlock_lib_activate.
	Inside the SignedData structure, the fwlock_msg_data_t parameter should be set to:
  fwlock_msg_data_t.msg_type = FWLOCK_MSG_DEACTIVATION;
  fwlock_msg_data_t.token = token;
*/
fwlock_status_t fwlock_lib_deactivate_remote(
    fwlock_buffer_t deactivation_msg);

/*****************************************************************************************************************/

/**
  Manually deactivates the Firmware Lock protection, without a cloud service connection.

  The function deactivates the Firmware Lock, depending on a verification against a user authenticator (e.g., a PIN)
  entered during Lock Activation.

  @param[in]  user_auth  User authenticator, such as a PIN.

  @return
  FWLOCK_SUCCESS -- Success. \n
  FWLOCK_FAILURE -- Failure.

  @dependencies
  None
*/
fwlock_status_t fwlock_lib_deactivate_local(fwlock_user_auth_t* user_auth);

/*****************************************************************************************************************/

/**
  Generates a unique token to be included in a signed cloud service message.

  @param[out]  token     16-byte token.

  @return
  FWLOCK_SUCCESS -- Success. \n
  FWLOCK_FAILURE -- Failure.

  @dependencies
  None.
*/
fwlock_status_t fwlock_lib_gen_token(fwlock_token_t *token);

/*****************************************************************************************************************/

/**
  Updates the Firmware Lock with a given RFC1422 CRL.

  @param[in]  crl  Certificate Revocation List (CRL), according to RFC1422.

  @return
  FWLOCK_SUCCESS -- Success. \n
  FWLOCK_FAILURE -- Failure.

  @dependencies
  None
*/
fwlock_status_t fwlock_lib_update_crl(fwlock_buffer_t isv_crl);

/*****************************************************************************************************************/

/**
  Returns whether the firmware is locked on the device.

  @param[out]  state  Current firmware lock state.

  @return
  FWLOCK_SUCCESS -- Success. \n
  FWLOCK_FAILURE -- Failure.

  @dependencies
  None
*/
fwlock_status_t fwlock_lib_get_state(fwlock_state_t* state);

/*-------------------------------------------------------------------------*/

#endif /* FWLOCK_H */
