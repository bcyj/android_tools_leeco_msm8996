/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
*/

#pragma once

/** Definitions from the FIDO UAF Authenticator Commands document
 */
/** Table 4.3: UAF Authenticator Status Codes \n
*/
typedef enum {
  UAF_STATUS_OK                   = 0x0000, /**< Success */
  UAF_STATUS_ERR_UNKNOWN          = 0x0001, /**< A generic error */
  UAF_STATUS_ERR_ACCESS_DENIED    = 0x0002, /**< Access to command is denied */
  UAF_STATUS_USER_NOT_ENROLLED    = 0x0003, /**< User is not enrolled */
  UAF_STATUS_MAX                  = 0x0FFF,
} UAF_STATUS_RV;

/** Table 4.1: Non-normative TAGs used in this document only (0x1000 – 0x10FF). \n
*/
typedef enum {
  TAG_KEYHANDLE         = 0x1001, /**< Represents KeyHandle. */
  TAG_KHANDLE_LIST      = 0x1002, /**< Represents a list of KeyHandles. */
  TAG_UNAME_LIST        = 0x1003, /**< Represents a list of Usernames. */
  TAG_USERVERIFY_TOKEN  = 0x1004, /**< Represents a User Verification Token. */
  TAG_FULL_APPID        = 0x1005, /**< Represents a full AppID. */
  TAG_KEYID             = 0x1006, /**< Represents a generated KeyID. */
  UAF_NN_TAG_MAX               = 0x10FF,
} UAF_NN_AUTH_TAG;

/** Table 4.2: UAF Authenticator Command TAGs (0x2000 - 0x21FF). \n
*/
typedef enum {
  TAG_UAFV1_GETINFO_CMD = 0x2001, /**< Tag for GetInfo command. */
  TAG_UAFV1_GETINFO_CMD_RESP = 0x2101, /**< Tag for GetInfo command response. */
  TAG_UAFV1_REG_CMD = 0x2002, /**< Tag for Register command. */
  TAG_UAFV1_REG_CMD_RESP = 0x2102, /**< Tag for Register command response. */
  TAG_UAFV1_SIGN_CMD = 0x2003, /**< Tag for Sign command. */
  TAG_UAFV1_SIGN_CMD_RESP = 0x2103, /**< Tag for Sign command response. */
  TAG_UAFV1_DEREG_CMD = 0x2004, /**< Tag for Deregister command. */
  TAG_UAFV1_DEREG_CMD_RESP = 0x2104, /**< Tag for Deregister command response. */
  TAG_UAFV1_MAX = 0x21FF,
} UAF_AUTH_CMD_TAG;

/** Maximum lengths as specified by the standard. \n
*/
#define UAFV1_FINAL_CHALLENGE_MAX_LEN 32
#define UAFV1_KH_ACCESS_TOKEN_MAX_LEN 32
#define UAFV1_USERNAME_MAX_LEN        128
#define UAFV1_APPID_MAX_LEN           256

/** Aritrary maximum lengths in our implementation. \n
*/
#define QC_REG_CMD_RESP_MAX_LEN        16384
#define QC_SIGN_CMD_RESP_MAX_LEN       16384

/** Definitions from the FIDO UAF Registry of Predefined Values
 */
/** 3.1 Authentication Factors
 */
typedef enum {
  USER_VERIFY_PRESENCE    = 0x01,
  USER_VERIFY_FINGERPRINT = 0x02,
  USER_VERIFY_PASSCODE    = 0x04,
  USER_VERIFY_VOICEPRINT  = 0x08,
  USER_VERIFY_FACEPRINT   = 0x10,
  USER_VERIFY_LOCATION    = 0x20,
  USER_VERIFY_EYEPRINT    = 0x40,
  USER_VERIFY_PATTERN     = 0x80,
  USER_VERIFY_HANDPRINT   = 0x100,
  USER_VERIFY_NONE        = 0x200,
  USER_VERIFY_MAX = USER_VERIFY_NONE,
  USER_VERIFY_INVALID = 0x7FFFFFFF
} UAF_AUTH_FACTORS_FLAG;

/** 3.2 Key Protection Types
 */
typedef enum {
  KEY_PROTECTION_SOFTWARE       = 0x01,
  KEY_PROTECTION_HARDWARE       = 0x02,
  KEY_PROTECTION_TEE            = 0x04,
  KEY_PROTECTION_SECURE_ELEMENT = 0x08,
  KEY_PROTECTION_REMOTE_HANDLE  = 0x10,
  KEY_PROTECTION_MAX = KEY_PROTECTION_REMOTE_HANDLE,
  KEY_PROTECTION_INVALID = 0x7FFFFFFF
} UAF_AUTH_KEY_PROTECTION_TYPE_FLAG;

/** 3.3 Authenticator Attachment Hints
 */
typedef enum {
  ATTACHMENT_HINT_INTERNAL  = 0x01,
  ATTACHMENT_HINT_EXTERNAL  = 0x02,
  ATTACHMENT_HINT_WIRED     = 0x04,
  ATTACHMENT_HINT_WIRELESS  = 0x08,
  ATTACHMENT_HINT_NFC       = 0x10,
  ATTACHMENT_HINT_BLUETOOTH = 0x20,
  ATTACHMENT_HINT_NETWORK   = 0x40,
  ATTACHMENT_HINT_READY     = 0x80,
  ATTACHMENT_HINT_MAX = ATTACHMENT_HINT_READY,
  ATTACHMENT_HINT_INVALID = 0x7FFFFFFF
} UAF_AUTH_ATTACHMENT_HINT_FLAG;

/** 3.4 Secure Display Types
 */
typedef enum {
  SECURE_DISPLAY_ANY                  = 0x01,
  SECURE_DISPLAY_PRIVILEGED_SOFTWARE  = 0x02,
  SECURE_DISPLAY_TEE                  = 0x04,
  SECURE_DISPLAY_HARDWARE             = 0x08,
  SECURE_DISPLAY_REMOTE               = 0x10,
  SECURE_DISPLAY_MAX = SECURE_DISPLAY_REMOTE,
  SECURE_DISPLAY_INVALID = 0x7FFFFFFF
} UAF_AUTH_SECURE_DISPLAY_FLAG;

/** 4.1 Tags used in the protocol
 */
typedef enum {
  TAG_UAFV1_REG_RESPONSE  = 0x01,
  TAG_UAFV1_SIGN_RESPONSE = 0x02,
  TAG_UAFV1_KRD           = 0x03,
  TAG_UAFV1_SIGNEDDATA    = 0x04,
  TAG_ATTESTATION_CERT    = 0x05,
  TAG_SIGNATURE           = 0x06,
  UAF_AUTH_TAG_MAX        = 0x0FFF,
} UAF_AUTH_TAG;

/** 4.3 Authentication Algorithms
 */
typedef enum {
  UAF_ALG_SIGN_ECDSA_SHA256_RAW       = 0x01,
  UAF_ALG_SIGN_ECDSA_SHA256_DER       = 0x02,
  UAF_ALG_SIGN_RSASSA_PSS_SHA256_RAW  = 0x03,
  UAF_ALG_SIGN_RSASSA_PSS_SHA256_DER  = 0x04,
} UAF_ALG;

/** 4.4 Public Key Representation Formats
 */
typedef enum {
  UAF_ALG_KEY_ECC_NISTP256R1_X962_RAW = 0x100,
  UAF_ALG_KEY_ECC_NISTP256R1_X962_DER = 0x101,
  UAF_ALG_KEY_RSA_2048_PSS_RAW        = 0x102,
  UAF_ALG_KEY_RSA_2048_PSS_DER        = 0x103,
} UAF_ALG_KEY_FORMAT;

/** 5 Assertion Schemes
 */
#define UAF_TLV_SCHEME "UAFV1TLV"
