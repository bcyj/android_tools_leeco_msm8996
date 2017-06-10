/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
*/

#pragma once

#include "qso.h"

#ifdef __RVCT__
#pragma anon_unions
#endif

#define QC_USERNAME_MAX_LEN 128 /**< In line with FIDO UAFV1 */
#define QC_SEC_APP_NAME_LEN 32 /**< As in lkthread.h */
#define QC_APP_ID_MAX_LEN   256 /**< In line with FIDO UAFV1 */
#define QC_AES_IV_SIZE      16
#define QC_TRANSACTION_MAX_LEN  1024 /**< In line with FIDO UAFV1 */
#define QC_AUTH_NONCE       32

/**
 * This file is shared between HLOS and QSEE, so QSEE definitions cannot
 * be used directly.
 */

/** Size of a QSEE encapsulated message
 *
 * Note: the following MUST be kept in sync with QSEE Message definitions
 *
 */
#define QSEE_MESSAGE_HEADER_LEN 84
#define QSEE_MESSAGE_MAC_LEN    32

#define ENC_SIZE(x) ((x) + QSEE_MESSAGE_HEADER_LEN + QSEE_MESSAGE_MAC_LEN)

/** ECC keys */
#define QC_ECC_256_PRV_KEY_LEN 36 /**< 9 uint32 */

#pragma pack(push, auth, 1)

typedef enum {
  QC_AUTH_CODE_OK  = 0, /**< Authentication was ok */
  QC_AUTH_CODE_ERR = 1, /**< Generic error */
  QC_AUTH_CODE_MAX = QC_AUTH_CODE_ERR,
  QC_AUTH_CODE_NA = 0x7FFFFFFF
} qc_auth_code_t;

/** Authentication token
*/
typedef struct {
  uint8_t version; /**< AuthenticatorVersion */
  qc_auth_code_t result;
  char username[QC_USERNAME_MAX_LEN+1];
  uint64_t userId;
  uint64_t uptime;
  uint64_t nonce;
  char appId[QC_APP_ID_MAX_LEN+1];
  char secAppName[QC_SEC_APP_NAME_LEN]; /**< \0 terminated */
} qc_authentication_token_t;

typedef union {
  uint8_t value[QSO_SIZE(0,sizeof(qc_authentication_token_t))];
  qc_authentication_token_t token;
  struct {
    QSoHeader_t header;
    qc_authentication_token_t token;
    uint8_t tag[SW_AES_BLOCK_BYTE_LEN];
  } wrapped;
} qc_so_authentication_token_t;

#define QC_USER_VERIFICATION_TOKEN_LEN ENC_SIZE(sizeof(qc_authentication_token_t))

typedef struct {
  char secAppName[QC_SEC_APP_NAME_LEN]; /**< \0 terminated */
  uint8_t message[QC_USER_VERIFICATION_TOKEN_LEN];
} qc_user_verification_token_t;

/** Transaction content */
typedef struct {
  char content[QC_TRANSACTION_MAX_LEN]; /**< \0 terminated */
  uint64_t uptime;
  char appId[QC_APP_ID_MAX_LEN+1];
  qc_authentication_token_t token;
} qc_transaction_t;

typedef struct {
  char secAppName[QC_SEC_APP_NAME_LEN]; /**< \0 terminated */
  char message[ENC_SIZE(sizeof(qc_transaction_t))];
} qc_enc_transaction_t;


#pragma pack(pop, auth)
