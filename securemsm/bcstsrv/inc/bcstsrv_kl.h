#ifndef _BCSTSRV_KL_H_
#define _BCSTSRV_KL_H_

/** @file bcstsrv_kl.h
 * @brief
 * This file contains the definitions of the constants, data structures
 * and interfaces to the Broadcast Service Key Ladder Module
 */

/*===========================================================================
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

 EDIT HISTORY FOR FILE

 when       who     what, where, why
 --------   ---     ----------------------------------------------------------
 01/30/2014 rzysman Extended API to support generateRoot, step, sendCW
 11/28/2013 ablay   Updates for mainline.
 10/20/13   mg      Initial Version.

===========================================================================*/

#define KLMD_MAX_KEY_LADDER_ENTRIES 32
#define KLMD_NUM_TSPPS              1
#define KLMD_NUM_MRKS               4
#define KLMD_NUM_KDFS               8
#define KLMD_MAX_HEIGHT             4

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

typedef enum {
  KLMD_ALG_DES,
  KLMD_ALG_AES,
  KLMD_ALG_CSAV1,
  KLMD_ALG_CSAV2,
  KLMD_ALG_CSAV3,
  KLMD_ALG_MULTI2,
  KLMD_ALG_MAX,
  KLMD_ALG_SIZE = 0x7FFFFFFF
} eKLMD_CeAlgorithm;

/*-------------------------------------------------------------------------*/

typedef enum {
  KLMD_KEY_PARITY_EVEN,
  KLMD_KEY_PARITY_ODD,
  KLMD_KEY_PARITY_MAX,
  KLMD_KEY_SIZE = 0x7FFFFFFF
} eKLMD_KeyParity_t;

/*-------------------------------------------------------------------------*/

typedef enum {
  KLMD_CIPHER_ENCRYPT,
  KLMD_CIPHER_DECRYPT,
  KLMD_CIPHER_BOTH,
  KLMD_CIPHER_MAX,
  KLMD_CIPHER_SIZE = 0x7FFFFFFF
} eKLMD_CipherAction_t;

/*-------------------------------------------------------------------------*/

typedef enum {
  KLMD_CIPHER_CHAIN_NONE,
  KLMD_CIPHER_CHAIN_ECB,
  KLMD_CIPHER_CHAIN_CBC,
  KLMD_CIPHER_CHAIN_MAX,
  KLMD_CHAIN_SIZE = 0x7FFFFFFF
} eKLMD_CipherChainMode_t;

/*-------------------------------------------------------------------------*/

typedef enum {
  KLMD_CIPHER_TERM_NONE,
  KLMD_CIPHER_TERM_CLEAR,
  KLMD_CIPHER_TERM_ISDA,
  KLMD_CIPHER_TERM_MAX,
  KLMD_TERM_SIZE = 0x7FFFFFFF
} eKLMD_CipherTermMode_t;

/*-------------------------------------------------------------------------*/

typedef enum {
  KLMD_KDF_ALG_DES = 0,
  KLMD_KDF_ALG_AES,
  KLMD_KDF_ALG_MAX,
  KLMD_KDF_ALG_SIZE = 0x7FFFFFFF
} eKLMD_KdfAlgorithm_t;

/*-------------------------------------------------------------------------*/

/** Error types */
typedef enum {
  BCSTSRV_KL_SUCCESS = 0,
  BCSTSRV_KL_ERR_FAILURE = -100,
  BCSTSRV_KL_ERR_INVALID_VERSION,
  BCSTSRV_KL_ERR_INVALID_PARAMETER,
  BCSTSRV_KL_ERR_INIT_FAILED,
  BCSTSRV_KL_ERR_TERM_FAILED,
  BCSTSRV_KL_ERR_ALLOC_KL_FAILED,
  BCSTSRV_KL_ERR_INVALID_KL_INDEX,
  BCSTSRV_KL_ERR_FREE_KL_FAILED,
  BCSTSRV_KL_ERR_DEACTIVATE_KL_FAILED,
  BCSTSRV_KL_ERR_SEND_DIRECT_FAILED,
  BCSTSRV_ERR_GENERATE_ROOT_FAILED,
  BCSTSRV_ERR_KL_STEP_FAILED,
  BCSTSRV_ERR_SEND_CW_FAILED,
  BCSTSRV_KL_ERR_SIZE = 0x7FFFFFFF
} BCSTSRV_KL_Status_t;

BCSTSRV_KL_Status_t BCSTSRV_KL_Initialize();

BCSTSRV_KL_Status_t BCSTSRV_KL_Terminate();

BCSTSRV_KL_Status_t BCSTSRV_KL_AllocateKeyLadder(uint8_t *u8KeyLadderIdx);

BCSTSRV_KL_Status_t BCSTSRV_KL_FreeKeyLadder(uint8_t u8LadderIdx);

BCSTSRV_KL_Status_t BCSTSRV_KL_DeactivateKeyLadder(uint8_t u8LadderIdx);

BCSTSRV_KL_Status_t BCSTSRV_KL_SendDirect(uint8_t u8KeyLadderIdx, uint8_t u8TsppId,
    eKLMD_CeAlgorithm eAlgorithm, uint8_t *au8Key, uint8_t *au8IV,
    eKLMD_KeyParity_t eOddEven, eKLMD_CipherAction_t eAction,
    eKLMD_CipherChainMode_t eChainMode, eKLMD_CipherTermMode_t eTermMode);

BCSTSRV_KL_Status_t BCSTSRV_KL_GenerateRoot(uint8_t u8MasterRootKeyIdx, uint8_t u8KdfIdx, uint8_t u8LadderHeight,
    uint8_t u8KeyLadderIdx, eKLMD_KdfAlgorithm_t eAlgorithm);

BCSTSRV_KL_Status_t BCSTSRV_KL_Step(uint8_t u8KeyLadderIdx, uint8_t *au8DataIn, uint8_t u8Height);

BCSTSRV_KL_Status_t BCSTSRV_KL_SendCW(uint8_t u8KeyLadderIdx, uint8_t u8TsppId,
    eKLMD_KeyParity_t eOddEven, uint8_t *au8IV,
    eKLMD_CipherChainMode_t eChainMode, eKLMD_CipherTermMode_t eTermMode);

#endif //_BCSTSRV_KL_H_
