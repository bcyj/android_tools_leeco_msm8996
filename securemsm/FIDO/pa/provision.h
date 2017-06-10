/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
*/

#pragma once

#include <stdint.h>

#ifdef __RVCT__
#pragma anon_unions
#endif

#pragma pack(push, provision, 1)

#define QC_SEC_APP_NAME_LEN 32 /**< As in lkthread.h */
/** Approximation of an RSA 2048b private key with all fields set, in BER, should be 1223 max */
#define RSA_KEY_DER_MAX_LEN 1300
/** DER-encoded X.509 certificate */
#define X509_VALUE_MAX_LEN 4096

/** RSA 2048bit private key, internal representation */
#define MODULUS_MAX_LEN           (512)
#define PRIVATE_EXPONENT_MAX_LEN  (512)
#define PUBLIC_EXPONENT_LEN       (4)
typedef struct {
  uint8_t modulus[MODULUS_MAX_LEN]; /**< CKA_MODULUS */
  uint32_t modulus_len;
  uint8_t public_exponent[PUBLIC_EXPONENT_LEN]; /**< CKA_PUBLIC_EXPONENT */
  uint32_t public_exponent_len;
  uint8_t exponent[PRIVATE_EXPONENT_MAX_LEN]; /**< CKA_PRIVATE_EXPONENT */
  uint32_t exponent_len;
} qc_rsa_private_key_t;

#define QC_ATTESTATION_KEY_LEN  sizeof(qc_rsa_private_key_t)

/** Key as received during provisioning */
typedef struct {
  char authenticator[QC_SEC_APP_NAME_LEN];
  uint32_t keyDerLen;
  uint8_t keyDer[RSA_KEY_DER_MAX_LEN];
  uint32_t certificateLen;
  uint8_t certificate[X509_VALUE_MAX_LEN];
} qc_provisioned_material_t;


#pragma pack(pop, provision)
