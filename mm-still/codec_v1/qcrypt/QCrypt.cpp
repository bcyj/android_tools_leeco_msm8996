/*****************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QCrypt.h"


/*==============================================================================
* Function : New
* Parameters: void
* Return Value : QCrypt * - pointer to new instance, NULL if failure
* Description: QCrypt static factory method
==============================================================================*/
QCrypt * QCrypt::New()
{
  int ret;
  QCrypt *instance = new QCrypt();

  ret = instance->Init();
  if (QI_SUCCESS != ret) {
    delete instance;
    instance = NULL;
  }
  return instance;
}

/*==============================================================================
* Function : Init
* Parameters: void
* Return Value : int
* Description: initialisation method
==============================================================================*/
int QCrypt::Init()
{
  mRsaEncCtx = (EVP_CIPHER_CTX *) malloc(sizeof(EVP_CIPHER_CTX));

  if (NULL == mRsaEncCtx) {
    return QI_ERR_NO_MEMORY;
  }

  EVP_CIPHER_CTX_init(mRsaEncCtx);

  return QI_SUCCESS;
}

/*==============================================================================
* Function : QCrypt
* Parameters: void
* Return Value : void
* Description: constructor
==============================================================================*/
QCrypt::QCrypt()
{
  mKeyPair = NULL;
  mRsaEncCtx = NULL;
  mEncMsgLen = 0;
}

/*==============================================================================
* Function : setEncKey
* Parameters: aEncKey, aLen
* Return Value : int
* Description: set encryption key
==============================================================================*/
int QCrypt::setEncKey(char *aEncKey, uint32_t aLen)
{
  BIO *lBIO = BIO_new(BIO_s_mem());
  if (NULL == lBIO) {
    return QI_ERR_NO_MEMORY;
  }

  int lRet = BIO_write(lBIO, aEncKey, aLen);
  if ((lRet < 0) && ((uint32_t)lRet != aLen)) {
    BIO_free_all(lBIO);
    QIDBG_ERROR("%s:%d] BIO_write() failed", __func__,
             __LINE__);
    return QI_ERR_GENERAL;
  }

  QIDBG_MED("%s:%d] Reading Pubkey...\n",  __func__,__LINE__);
  if (NULL == PEM_read_bio_PUBKEY(lBIO, &mKeyPair, NULL, NULL)) {
    BIO_free_all(lBIO);
    QIDBG_ERROR("%s:%d] Failed to read the private key", __func__,
            __LINE__);
    return QI_ERR_GENERAL;
  }

  BIO_free_all(lBIO);

  mEkLen = EVP_PKEY_size(mKeyPair);

  return QI_SUCCESS;
}

/*==============================================================================
* Function : setDecKey
* Parameters: aDecKey, aLen
* Return Value : int
* Description: set decryption key
==============================================================================*/
int QCrypt::setDecKey(char *aDecKey, uint32_t aLen)
{
  BIO *lBIO = BIO_new(BIO_s_mem());
  if (NULL == lBIO) {
    return QI_ERR_NO_MEMORY;
  }

  int lRet = BIO_write(lBIO, aDecKey, aLen);
  if ((lRet < 0) && ((uint32_t)lRet != aLen)) {
    QIDBG_ERROR("%s:%d] BIO_write() failed", __func__,
             __LINE__);

    BIO_free_all(lBIO);
    return QI_ERR_GENERAL;
  }

  QIDBG_MED("%s:%d] Reading Privkey...\n", __func__,__LINE__);
  if (NULL == PEM_read_bio_PrivateKey(lBIO, &mKeyPair, NULL, NULL)) {
    QIDBG_ERROR("%s:%d] Failed to read the private key", __func__,
            __LINE__);

    BIO_free_all(lBIO);
    return QI_ERR_GENERAL;
  }

  BIO_free_all(lBIO);

  return QI_SUCCESS;
}

/*==============================================================================
* Function : setEncMsgLen
* Parameters: aEncLen
* Return Value : uint32_t
* Description: Set length of message to be encrypted. Returns max length
* of encrypted message.
==============================================================================*/
uint32_t QCrypt::setEncMsgLen(uint32_t aEncLen)
{
  mEncMsgLen = aEncLen;

  if (NULL == mKeyPair) {
    mEncMsgLen = 0;
    return 0;
  }

  uint32_t lDecMaxLen = headerSize() + mEncMsgLen + AES_BLOCK_SIZE;

  return lDecMaxLen;
}

/*==============================================================================
* Function : encrypt
* Parameters: aSrc, aDst
* Return Value : uint32_t
* Description: Encrypt aSrc into aDst. Returns length of encrypted message.
==============================================================================*/
uint32_t QCrypt::encrypt(uint8_t *aSrc, uint8_t *aDst)
{
  if ((NULL == aSrc) || (NULL == aDst)) {
    QIDBG_ERROR("%s:%d] NULL ptr", __func__, __LINE__);
    return 0;
  }

  if (!encParamsOK()) {
    QIDBG_ERROR("%s:%d] Params validation failed", __func__, __LINE__);
    return 0;
  }

  uint8_t *lMsgStart = aDst + headerSize();

  enc_msg_hdr_t lHdr;
  lHdr.msg_offset = headerSize();
  lHdr.ek_offset = sizeof(lHdr);
  lHdr.iv_len = EVP_MAX_IV_LENGTH;
  lHdr.iv_offset = lHdr.ek_offset + mEkLen;

  uint8_t *lEk = aDst + lHdr.ek_offset;
  int ret = EVP_SealInit(mRsaEncCtx, EVP_aes_256_cbc(),
                &lEk, (int *)&lHdr.ek_len, aDst + lHdr.iv_offset,
                &mKeyPair, 1);
  if (!ret) {
    QIDBG_ERROR("%s:%d] Error in SealInit()", __func__, __LINE__);
    return 0;
  }

  uint32_t lDstLen = 0, lDstLenBlk = 0;
  ret = EVP_SealUpdate(mRsaEncCtx, lMsgStart, (int *)&lDstLenBlk,
                aSrc, mEncMsgLen);
  if (!ret) {
    QIDBG_ERROR("%s:%d] Error in SealUpdate()", __func__, __LINE__);
    return 0;
  }

  lMsgStart += lDstLenBlk;
  lDstLen += lDstLenBlk;

  ret = EVP_SealFinal(mRsaEncCtx, lMsgStart, (int*)&lDstLenBlk);


  if (!ret) {
    QIDBG_ERROR("%s:%d] Error in SealFinal()", __func__, __LINE__);
    return 0;
  }

  lDstLen += lDstLenBlk;
  lHdr.msg_len = lDstLen;

  memcpy(aDst, &lHdr, sizeof(lHdr));

  return headerSize() + lDstLen;
}

/*==============================================================================
* Function : setDecMsgLen
* Parameters: dec_len
* Return Value : void
* Description: Set length of message to be decrypted
==============================================================================*/
void QCrypt::setDecMsgLen(uint32_t aDecLen)
{
  mDecMsgLen = aDecLen;
}

/*==============================================================================
* Function : hdrValid
* Parameters: hdr
* Return Value : int
* Description: Perform validation on the encryption header values
==============================================================================*/
bool QCrypt::hdrValid(enc_msg_hdr_t *aHdr)
{
  if (((aHdr->ek_offset + aHdr->ek_len) > mDecMsgLen) ||
      ((aHdr->iv_offset + aHdr->iv_len) > mDecMsgLen) ||
      ((aHdr->msg_offset + aHdr->msg_len) > mDecMsgLen)) {

        return 0;
  }
  return 1;
}

/*==============================================================================
* Function : decrypt
* Parameters: aSrc, aDst
* Return Value : uint32_t
* Description: Decrypt aSrc into aDst
==============================================================================*/
uint32_t QCrypt::decrypt(uint8_t *aSrc, uint8_t *aDst)
{
  if ((NULL == aSrc) || (NULL == aDst)) {
    QIDBG_ERROR("%s:%d] NULL ptr", __func__, __LINE__);
    return 0;
  }

  if (!decParamsOK()) {
    QIDBG_ERROR("%s:%d] Params validation failed", __func__, __LINE__);
    return 0;
  }

  enc_msg_hdr_t lHdr;
  memcpy(&lHdr, aSrc, sizeof(lHdr));

  if (!hdrValid(&lHdr)) {
    QIDBG_ERROR("%s:%d] Error validating header", __func__, __LINE__);
    return 0;
  }

  EVP_CIPHER_CTX_init(mRsaEncCtx);

  uint8_t *lEk = aSrc + lHdr.ek_offset;
  uint32_t lEkLen = lHdr.ek_len;
  uint8_t *lIv = aSrc + lHdr.iv_offset;
  uint8_t *lMsgStart = aSrc + lHdr.msg_offset;
  uint32_t lMsgLen = lHdr.msg_len;

  int ret = EVP_OpenInit(mRsaEncCtx, EVP_aes_256_cbc(), lEk,
      lEkLen, lIv, mKeyPair);

  if (!ret) {
    QIDBG_ERROR("%s:%d] Error in OpenInit()", __func__, __LINE__);
    return 0;
  }

  uint32_t lDecLen = 0, lLenBlk = 0;
  ret = EVP_OpenUpdate(mRsaEncCtx, aDst, (int*)&lLenBlk, lMsgStart, lMsgLen);

  if (!ret) {
    QIDBG_ERROR("%s:%d] Error in OpenUpdate()", __func__, __LINE__);
    return 0;
  }

  lDecLen += lLenBlk;
  aDst += lLenBlk;

  ret = EVP_OpenFinal(mRsaEncCtx, aDst, (int*)&lLenBlk);

  if (!ret) {
    QIDBG_ERROR("%s:%d] Error in OpenFinal()", __func__, __LINE__);
    return 0;
  }

  lDecLen += lLenBlk;

  return lDecLen;

}


/*==============================================================================
* Function : headerSize
* Parameters: void
* Return Value : uint32_t
* Description: Return the size of the encryption header before start of msg
==============================================================================*/
uint32_t QCrypt::headerSize()
{
  uint32_t size = sizeof(enc_msg_hdr_t) + mEkLen +
      EVP_MAX_IV_LENGTH;

  return size;
}

/*==============================================================================
* Function : encParamsOK
* Parameters: void
* Return Value : int
* Description: Check if encryption parameters are set correctly
==============================================================================*/
bool QCrypt::encParamsOK()
{
  if (mEncMsgLen && mKeyPair) {
    return 1;
  }
  return 0;
}

/*==============================================================================
* Function : decParamsOK
* Parameters: void
* Return Value : int
* Description: Check if decryption parameters are set correctly
==============================================================================*/
bool QCrypt::decParamsOK()
{
  if (mDecMsgLen && mKeyPair) {
    return 1;
  }
  return 0;
}
/*==============================================================================
* Function : ~QCrypt
* Parameters: void
* Return Value : void
* Description: destructor
==============================================================================*/
QCrypt::~QCrypt()
{
  if (NULL != mKeyPair) {
    EVP_PKEY_free(mKeyPair);
    mKeyPair = NULL;
  }

  if (NULL != mRsaEncCtx) {
    EVP_CIPHER_CTX_cleanup(mRsaEncCtx);
    free(mRsaEncCtx);
    mRsaEncCtx = NULL;
  }
}
