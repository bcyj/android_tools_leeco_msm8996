/*******************************************************************
--------------------------------------------------------------------
 Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
----------------------------------------------------------------------
QSEEComApi sample client app
*******************************************************************/

#ifndef __TOE_KEY_GEN_API
#define __TOE_KEY_GEN_API

#define PACKED  __attribute((__packed__))

/* This struct holds all the key offsets in IMEM.
   Offsets are positive for valid offset or -1 when key is NULL.
*/
typedef PACKED struct
{
    int32_t kenbRrcIntOffset;
    int32_t kenbRrcEncOffset;
    int32_t kenbUpEncOffset;
    int32_t kenbUpIntOffset;
} LteSecImemOffsets;

/* This struct holds the parameters needed to generated kenb*   */
typedef PACKED struct
{
    uint32_t pci;
    uint32_t earfcnDl;
    uint32_t useNH;
    uint32_t kenbStarIdx;
} LteKenbStarParams;

/* KDF Algorithm types  */
typedef enum
{ LTE_SEC_ALG_ID_NULL = 0,
    AES,
    SNOW3G,
    ZUC,
    NUM_KDF_ALGS
} LteKdfAlgTypes;

typedef PACKED struct
{
    uint8_t keyType;
    uint8_t algId;
} LteGenKeysParams;

#define K_RRC_ENC_ALG_P1 0x03
#define K_RRC_INT_ALG_P1 0x04
#define K_UP_ENC_ALG_P1  0x05
#define K_UP_INT_ALG_P1  0x06

/**************  API function declaration  *****************************/

/***********************************************
INPUT: ueIdx, kenb
OUTPUT: None
RETURN: SUCCESS/FAIL
SIDE EFFECT: stores kenb in IMEM for userIdx
*/
int lteStoreKenb(
    uint32_t ueIdx,
    uint8_t * kenb);

/***********************************************
INPUT: ueIdx, algId, keyType, useKenbStar
OUTPUT: None
RETURN: SUCCESS/FAIL
SIDE EFFECT: Generates keys and stores them in the offset locations in IMEM
*/
int lteGenerateKeys(
    uint32_t ueIdx,
    uint32_t keyType,
    LteKdfAlgTypes algId);

/***********************************************
   INPUT: ueIdx
   OUTPUT: keyOffsets
   RETURN: SUCCESS/FAIL
   SIDE EFFECT: populates key offsets from IMEM. An offset of -1 indicates NULL value for the key.
*/
int lteGetKeyOffsets(
    uint32_t ueIdx,
    LteSecImemOffsets * keyOffsets);

/***********************************************
   INPUT: ueIdx, ncc, params, cell_idx
   OUTPUT: None
   RETURN: SUCCESS/FAIL
   SIDE EFFECT: Generates and stores kenb* in IMEM. If useNH =1 it uses the stored NH to calculate kenb*

*/
int lteGenerateKenbStar(
    uint32_t ueIdx,
    const LteKenbStarParams * params);

/***********************************************
INPUT: ueIdx. kenbStarIdx
OUTPUT: kenbStar
RETURN: SUCCESS/FAIL
SIDE EFFECT: populates kenbStar from IMEM for ueIdx and cell kenbStarIdx.
*/
int lteGetKenbStar(
    uint32_t ueIdx,
    uint8_t * kenbStar);

/***********************************************
INPUT: ueIdx, nh
OUTPUT: None
RETURN: SUCCESS/FAIL
SIDE EFFECT: stores NH in IMEM for userIdx
*/
int lteStoreNh(
    uint32_t ueIdx,
    uint8_t * nh);

/***********************************************
INPUT: ueIdx
OUTPUT: None
RETURN: SUCCESS/FAIL
SIDE EFFECT: Deletes NH in IMEM for userIdx.
*/
int lteDeleteNh(
    uint32_t ueIdx);

/***********************************************
   INPUT: ueIdx
   OUTPUT: None
   RETURN:SUCCESS/FAIL
  SIDE EFFECT: Deletes all keys from IMEM for useIdx.
*/
int lteDeleteKeys(
    uint32_t ueIdx);

#endif  /*  */
