/*******************************************************************
--------------------------------------------------------------------
 Copyright (c) 2014 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
----------------------------------------------------------------------
QSEEComApi sample client app
*******************************************************************/

#ifndef __TOE_IKE
#define __TOE_IKE

#define FACT_SZ 512
#define MOD_SZ 1024
#define PCOMP_SZ 1024
#define SIG_SZ 512

/* HLOS to send the RSA private key in this format*/
typedef struct{
    uint32_t nbits;   /* Number of bits in modulus */
    char n[MOD_SZ];   /*  Modulus */
    char p[FACT_SZ];  /*  p factor of N */
    char q[FACT_SZ];  /*  q factor of N */
    char e[FACT_SZ];  /*  e Public exponent */
    char d[PCOMP_SZ]; /*  Private exponent */
} tzbsp_rsa_key_t;

/* RSA padding type. PKCS #1 v2.1 */
typedef enum{
    /*PKCS1 v1.5 signature*/
    CE_RSA_PAD_PKCS1_V1_5_SIG = 1,
    /*PKCS1 v1.5 encryption*/
    CE_RSA_PAD_PKCS1_V1_5_ENC = 2,  /* not supported */
    /*OAEP Encryption*/
    CE_RSA_PAD_PKCS1_OAEP = 3,      /* not supported */
    /*PSS Signature*/
    CE_RSA_PAD_PKCS1_PSS = 4,
    /* No Padding */
    CE_RSA_NO_PAD = 5,
} CE_RSA_PADDING_TYPE;

/* Request Structure- Prov Key */
typedef struct{
    /* buffer containing the key */
    tzbsp_rsa_key_t keybuf;
} tzbsp_ike_prov_key_cmd_t;

/* Response Structure - Prov Key*/
typedef struct{
    int result;
} tzbsp_ike_rsp_prov_key_cmd_t;

/* Request Structure - Sign cmd */
typedef struct{
    /* Padding scheme to use for the signature:
     * PKCS1v1.5 or PKCS1 PSS*/
    CE_RSA_PADDING_TYPE rsa_pad;
    /* Salt length (only for PKCS1 PSS) */
    int saltlen;
    /* length of the hash */
    uint32_t buflen;
    /* PKC buffer to sign */
    uint8_t buf[SIG_SZ];
} tzbsp_ike_sign_cmd_t;

/* Response Structure - Sign cmd */
typedef struct{
    int result;
    uint32_t siglen;
    uint8_t buf[SIG_SZ];
} tzbsp_ike_rsp_sign_cmd_t;


#endif
