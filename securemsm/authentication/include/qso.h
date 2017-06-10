/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
*/

#pragma once

#include <stdint.h>

#ifdef __RVCT__
#include <qsee_uf_aes.h>
#else
#define SW_AES_IV_SIZE        16
#define SW_AES_BLOCK_BYTE_LEN 16
#endif

#define QSO_VERSION_MAJOR 0x0001
#define QSO_VERSION_MINOR 0x0000

/** Maximum size of the payload (plain length + encrypted length) of a secure object. */
#define QSO_PAYLOAD_MAX_SIZE      1000000

/** Secure object header.
* A secure object header introduces a secure object.
* Layout of a secure object:
* <pre>
* <code>
*
*     +--------+------------------+------------------------+--------+
*     | Header |   plain-data     |  encrypted-data        |  tag   |
*     +--------+------------------+------------------------+--------+
*
*     /--------/---- plainLen ----/----- encryptedLen -----/-- 16 --/
*
*     /--------- AAD -------------/                        /-- tag -/
*
*     /----------------- Input to AES GCM  ----------------/
*
*     /--------------------------- totalSoSize ---------------------/
*
* </code>
* </pre>
*/

typedef struct {
  /** Secure object version. */
  uint32_t version;
  /** Length of unencrypted user data (after the header). */
  uint32_t plainLen;
  /** Length of encrypted user data (after unencrypted data,
   * excl. padding bytes). */
  uint32_t encryptedLen;
  /** IV */
  uint8_t iv[SW_AES_IV_SIZE];
} QSoHeader_t;


/** Calculates the total size of a secure object.
* @param plainLen Length of plain text part within secure object.
* @param encryptedLen Length of encrypted part within secure object (excl.
* hash, padding).
* @param tagLen Length of GCM integrity tag.
* @return Total (gross) size of the secure object or 0 if given parameters are
* illegal or would lead to a secure object of invalid size.
*/
#define QSO_SIZE(plainLen, encryptedLen) ( \
  (sizeof(QSoHeader_t) + (plainLen) + (encryptedLen) + SW_AES_BLOCK_BYTE_LEN)  \
  )
//TODO: deal with wrong sizes

/** @} */
