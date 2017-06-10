/*****************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include "QIBuffer.h"
#include "QExifParser.h"
#include "QCrypt.h"
}

#include "QJpegMakernoteExtract.h"

#define USAGE "\n\nUsage: %s <key> <JPEG file>\n\n"


/*===========================================================================
 * Function: QJpegMakernoteExtractor
 *
 * Description: Constructor
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *   None
 *
 * Notes: none
 *==========================================================================*/
QJpegMakernoteExtractor::QJpegMakernoteExtractor() :
    mKeyFile(NULL), mJpgFile(NULL), mInBuffer(NULL),
    mKey(NULL), mParser(NULL), mCrypto(NULL),
    mDecData(NULL)
{

}


int QJpegMakernoteExtractor::Init()
{
  mParser = QExifParser::New(*this);
  if (!mParser) {
    MKN_ERR("Failed to create exif parser");
    return 1;
  }
  mCrypto = QCrypt::New();
  if (!mCrypto) {
    MKN_ERR("Failed to create decryption class");
  }

  return 0;
}



/*===========================================================================
 * Function: ReadComplete
 *
 * Description: Callback function called when exifparser is done parsing
 *              the data
 *
 * Input parameters:
 *   aBuffer - buffer read by the component
 *
 * Return values:
 *   QI_SUCCESS
 *
 * Notes: none
 *==========================================================================*/
void QJpegMakernoteExtractor::ReadComplete(QIBuffer &aBuffer)
{

}

/*===========================================================================
 * Function: ReadFragmentDone
 *
 * Description: Callback function called when buffer is read
 *
 * Input parameters:
 *   aBuffer - filled buffer sent by the component
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QJpegMakernoteExtractor::ReadFragmentDone(QIBuffer &aBuffer)
{

}

/*===========================================================================
 * Function: ReadError
 *
 * Description: Callback function called when error in incurred during
 *              exif parsing
 *
 * Input parameters:
 *   aErrorType - error type
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QJpegMakernoteExtractor::ReadError(ErrorType aErrorType)
{

}

/*===========================================================================
 * Function: setKeyFile
 *
 * Description: set name of key file
 *
 * Input parameters:
 *   aKeyFile - Name of key file
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QJpegMakernoteExtractor::setKeyFile(char *aKeyFile)
{
  mKeyFile = aKeyFile;
}

/*===========================================================================
 * Function: setJpegFile
 *
 * Description: set name of image file
 *
 * Input parameters:
 *   aJpegFile - Name of image file
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QJpegMakernoteExtractor::setJpegFile(char *aJpegFile)
{
  mJpgFile = aJpegFile;
}

/*===========================================================================
 * Function: loadKey
 *
 * Description: load key file
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QJpegMakernoteExtractor::loadKey()
{
  int lFileSize = 0;
  int lrc = 0;
  FILE * lKeyFp = fopen(mKeyFile, "rb");
  if (!lKeyFp) {
    MKN_ERR("Error opening file");
    return 1;
  }
  fseek(lKeyFp, 0, SEEK_END);
  lFileSize = ftell(lKeyFp);
  fseek(lKeyFp, 0, SEEK_SET);

  mKey = new char[lFileSize + 1];
  if (!mKey) {
    MKN_ERR("Error allocating key buffer");
    fclose(lKeyFp);
    return 1;
  }
  lrc = fread(mKey, 1, lFileSize, lKeyFp);
  MKN_ERR("Read %d bytes", lrc);

  mKey[lFileSize] = '\0';

  fclose(lKeyFp);

  return 0;
}

/*===========================================================================
 * Function: loadImage
 *
 * Description: load Image file
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QJpegMakernoteExtractor::loadImage()
{
  int lFileSize = 0;
  int lrc = 0;
  FILE * lJpgFp = fopen(mJpgFile, "rb");
  if (!lJpgFp) {
    MKN_ERR("Error opening file");
    return 1;
  }
  fseek(lJpgFp, 0, SEEK_END);
  lFileSize = ftell(lJpgFp);
  fseek(lJpgFp, 0, SEEK_SET);

  mInBuffer = QIHeapBuffer::New(lFileSize);
  if (NULL == mInBuffer) {
    MKN_ERR("Error allocating input buffer");
    fclose(lJpgFp);
    return 1;
  }
  lrc = fread(mInBuffer->Addr(), 1, mInBuffer->Length(), lJpgFp);
  MKN_ERR("Read %d bytes", lrc);

  mInBuffer->SetFilledLen(lFileSize);
  fclose(lJpgFp);

  return 0;
}

/*===========================================================================
 * Function: parseExif
 *
 * Description: parse exif data
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QJpegMakernoteExtractor::parseExif()
{
  int lrc = QI_SUCCESS;

  lrc = mParser->addBuffer(mInBuffer);
  if (QI_SUCCESS != lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = mParser->Start();
  if (QI_SUCCESS != lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  mJpgHeader = mParser->GetHeader();
  if (!mJpgHeader) {
    MKN_ERR("Error getting jpeg header");
    return 1;
  }

  return 0;
}

/*===========================================================================
 * Function: decryptMakernote
 *
 * Description: decrypt makernote data
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QJpegMakernoteExtractor::decryptMakernote()
{
  int lrc = 0;

  lrc = mCrypto->setDecKey(mKey, strlen(mKey) + 1);
  if (lrc) {
    MKN_ERR("Failed to set decryption key");
    return 1;
  }
  int lMkCnt = mJpgHeader->p_exif_info->exif_ifd.p_exif_maker_note->entry.count;
  unsigned char *lMkPtr = mJpgHeader->p_exif_info->
      exif_ifd.p_exif_maker_note->entry.data._undefined;

  MKN_ERR("Makernote count %d ptr %p", lMkCnt, lMkPtr);

  mCrypto->setDecMsgLen(lMkCnt);

  mDecData = new char[lMkCnt];
  if (!mDecData) {
    MKN_ERR("Failed to allocate plaintext data");
    return 1;
  }

  lrc = mCrypto->decrypt((uint8_t*)lMkPtr,(uint8_t*) mDecData);
  if (!lrc){
    MKN_ERR("Failed to decrypt data");
    return 1;
  }
  MKN_ERR("Decrypted %d bytes", lrc);

  return lrc;
}

/*===========================================================================
 * Function: Extract
 *
 * Description: Decrypt and extract the makernote data
 *
 * Input parameters:
 *   aOutBuf - pointer to the output buffer
 *
 * Return values:
 *   int - size of extracted makernote data
 *
 * Notes: none
 *==========================================================================*/
int QJpegMakernoteExtractor::Extract(char **aOutBuf)
{
  int lrc = 0;
  lrc = loadKey();
  if (lrc) {
    MKN_ERR("Failed to load key");
    return lrc;
  }
  lrc = loadImage();
  if (lrc) {
    MKN_ERR("Failed to load image");
    return lrc;
  }
  lrc = parseExif();
  if (lrc) {
    MKN_ERR("Failed to parse exif");
    return lrc;
  }
  int lLen = decryptMakernote();
  if (!lLen) {
    MKN_ERR("Failed to decrypt makernote data");
    return lLen;
  }
  *aOutBuf = mDecData;
  return lLen;
}

/*===========================================================================
 * Function: ~QJpegMakernoteExtractor
 *
 * Description: Destructor
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *   None
 *
 * Notes: none
 *==========================================================================*/
QJpegMakernoteExtractor::~QJpegMakernoteExtractor()
{
  if (mInBuffer) {
    delete mInBuffer;
    mInBuffer = NULL;
  }
  if (mKey) {
    delete mKey;
    mKey = NULL;
  }
  if (mParser) {
    delete mParser;
    mParser = NULL;
  }
  if (mCrypto) {
    delete mCrypto;
    mCrypto = NULL;
  }
  if (mDecData) {
    delete mDecData;
    mDecData = NULL;
  }
}

/*===========================================================================
 * Function: main
 *
 * Description: Application main function
 *
 * Input parameters:
 *   argc, argv
 *
 * Return values:
 *   int
 *
 * Notes: none
 *==========================================================================*/
int main(int argc, char *argv[])
{
  int lrc = 0;
  if (argc < 3) {
    fprintf(stderr, USAGE, argv[0]);
    return 1;
  }
  QJpegMakernoteExtractor *lExtractor = new QJpegMakernoteExtractor();
  if (!lExtractor) {
    MKN_ERR("Failed to create mkn extractor");
    return 1;
  }
  lrc = lExtractor->Init();

  if (lrc) {
    MKN_ERR("Failed to init extractor");
    delete lExtractor;
    return 1;
  }

  lExtractor->setKeyFile(argv[1]);
  lExtractor->setJpegFile(argv[2]);

  char *loutBuf;
  int lLen = lExtractor->Extract(&loutBuf);
  if (!lLen) {
    MKN_ERR("Failed to extract data");
    delete lExtractor;
    return 1;
  }

  printf("lLen = %d\n", lLen);
  while(lLen) {
    int len = printf("%s", loutBuf) + 1;
    lLen -= len;
    loutBuf += len;

    printf(" = ");

    len = printf("%s", loutBuf) + 1;
    lLen -= len;
    loutBuf += len;

    printf("\n");
  }

  delete lExtractor;


  return 0;
}
