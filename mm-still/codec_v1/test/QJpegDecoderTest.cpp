/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QJpegDecoderTest.h"
#include "QITime.h"

const char color_formats[17][13] =
{
  "YCRCBLP_H2V2",
  "YCBCRLP_H2V2",
  "YCRCBLP_H2V1",
  "YCBCRLP_H2V1",
  "YCRCBLP_H1V2",
  "YCBCRLP_H1V2",
  "YCRCBLP_H1V1",
  "YCBCRLP_H1V1",
  "MONOCHROME",
  "IYUV_H2V2",
  "YUV2_H2V2",
  "IYUV_H2V1",
  "YUV2_H2V1",
  "IYUV_H1V2",
  "YUV2_H1V2",
  "IYUV_H1V1",
  "YUV2_H1V1",
};

typedef enum
{
  QIYCRCBLP_H2V2,
  QIYCBCRLP_H2V2,
  QIYCRCBLP_H2V1,
  QIYCBCRLP_H2V1,
  QIYCRCBLP_H1V2,
  QIYCBCRLP_H1V2,
  QIYCRCBLP_H1V1,
  QIYCBCRLP_H1V1,
  QIMONOCHROME,
  QIIYUV_H2V2,
  QIYUV2_H2V2,
  QIIYUV_H2V1,
  QIYUV2_H2V1,
  QIIYUV_H1V2,
  QIYUV2_H1V2,
  QIIYUV_H1V1,
  QIYUV2_H1V1,
  QICOLOR_FORMAT_MAX,
} test_format_t;

/*===========================================================================
 * Function: QJpegDecoderTest
 *
 * Description: QJpegDecoderTest default constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QJpegDecoderTest::QJpegDecoderTest()
{
  mpDecoder = NULL;
  mOutputFilename = NULL;
  mInputFilename = NULL;
  mInput = NULL;
  mOutput = NULL;
  mInputdata = NULL;
  mOutputdata = NULL;
  QI_MUTEX_INIT(&mMutex);
  QI_COND_INIT(&mCond);
  mError = QI_SUCCESS;
}

/*===========================================================================
 * Function: DecodeComplete
 *
 * Description: Callback function called when the decoding is done
 *
 * Input parameters:
 *   aOutputImage - output image which is Decoded
 *
 * Return values:
 *   QI_SUCCESS
 *
 * Notes: none
 *==========================================================================*/
int QJpegDecoderTest::DecodeComplete(QImage *aOutputImage)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  QI_SIGNAL(&mCond);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: DecodeError
 *
 * Description: Callback function for error indication called when the
 *              decoding is done
 *
 * Input parameters:
 *   aErrorType - error type
 *
 * Return values:
 *   QI_SUCCESS
 *
 * Notes: none
 *==========================================================================*/
int QJpegDecoderTest::DecodeError(DecodeErrorType aErrorType)
{
  QIDBG_MED("%s:%d] Error %d", __func__, __LINE__, aErrorType);
  mError = QI_ERR_GENERAL;
  QI_SIGNAL(&mCond);
  return QI_SUCCESS;
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
void QJpegDecoderTest::ReadComplete(QIBuffer &aBuffer)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
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
void QJpegDecoderTest::ReadFragmentDone(QIBuffer &aBuffer)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
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
void QJpegDecoderTest::ReadError(ErrorType aErrorType)
{
  QIDBG_MED("%s:%d] Error %d", __func__, __LINE__, aErrorType);
}

/*===========================================================================
 * Function: ~QJpegDecoderTest
 *
 * Description: QJpegDecoderTest destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QJpegDecoderTest::~QJpegDecoderTest()
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  if (mpDecoder) {
    delete mpDecoder;
    mpDecoder = NULL;
  }
  if (mInput) {
    delete mInput;
    mInput = NULL;
  }
  if (mOutput) {
    delete mOutput;
    mOutput = NULL;
  }
  if (mInputdata) {
    delete mInputdata;
    mInputdata = NULL;
  }
  if (mOutputdata) {
    delete mOutputdata;
    mOutputdata = NULL;
  }
  if (mParser) {
    delete mParser;
    mParser = NULL;
  }
  QI_MUTEX_DESTROY(&mMutex);
  QI_COND_DESTROY(&mCond);
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
}

/*===========================================================================
 * Function: Init
 *
 * Description: initializate the Decoder test object
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QJpegDecoderTest::Init()
{
  QIDBG_MED("%s:%d] %p", __func__, __LINE__, this);

  /*exif parser*/
  mParser = QExifParser::New(*this);
  if (mParser == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: Read
 *
 * Description: read the data from the file and fill the buffers
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QJpegDecoderTest::Read()
{
  FILE *fp = fopen(mInputFilename, "rb+");
  int lrc = 0;
  QIDBG_MED("%s:%d] ", __func__, __LINE__);

  if (fp) {
    // Find out input file length
    fseek(fp, 0, SEEK_END);
    mFileLength = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    QIDBG_MED("%s:%d] file_length %d", __func__, __LINE__, mFileLength);

    /*create input buffer*/
    mInputdata = QIHeapBuffer::New(mFileLength);
    if (mInputdata == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      fclose(fp);
      return QI_ERR_NO_MEMORY;
    }
    mInput = new QImage(mInputdata->Addr(), mInputdata->Length(), QI_BITSTREAM);
    if (mInput == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      fclose(fp);
      return QI_ERR_NO_MEMORY;
    }
    lrc = fread(mInputdata->Addr(), 1, mInputdata->Length(), fp);
    QIDBG_HIGH("%s:%d] bytes_read %d", __func__, __LINE__, lrc);
    mInputdata->SetFilledLen(mFileLength);
    fclose(fp);
  } else {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: Write
 *
 * Description: write the filled data to the file
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QJpegDecoderTest::Write()
{
  FILE *fp = fopen(mOutputFilename, "wb+");
  int lrc = 0;
  QIDBG_MED("%s:%d] filled len %d", __func__, __LINE__, mImageSize);
  if (fp) {
    lrc = fwrite(mOutputdata->Addr(), 1, mImageSize, fp);
    QIDBG_HIGH("%s:%d] bytes_written %d", __func__, __LINE__, lrc);
    fclose(fp);
  } else {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: StartExifparser
 *
 * Description: start exif parser
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QJpegDecoderTest::StartExifparser()
{
  int lrc = QI_SUCCESS;
  QIDBG_MED("%s:%d] ", __func__, __LINE__);

  lrc = mParser->addBuffer(mInputdata);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = mParser->Start();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }
  return lrc;
}

/*===========================================================================
 * Function: GetInputInfo
 *
 * Description: get the input format and subsampling
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
static void GetInputInfo(test_format_t aTestformat, QIFormat *apFormat,
  QISubsampling *apSS)
{
  switch (aTestformat) {
  case QIYCRCBLP_H2V2:
    *apFormat = QI_YCRCB_SP;
    *apSS = QI_H2V2;
    break;
  case QIYCBCRLP_H2V2:
    *apFormat = QI_YCBCR_SP;
    *apSS = QI_H2V2;
    break;
  case QIYCRCBLP_H2V1:
    *apFormat = QI_YCRCB_SP;
    *apSS = QI_H2V1;
    break;
  case QIYCBCRLP_H2V1:
    *apFormat = QI_YCBCR_SP;
    *apSS = QI_H2V1;
    break;
  case QIYCRCBLP_H1V2:
    *apFormat = QI_YCRCB_SP;
    *apSS = QI_H1V2;
    break;
  case QIYCBCRLP_H1V2:
    *apFormat = QI_YCBCR_SP;
    *apSS = QI_H1V2;
    break;
  case QIYCRCBLP_H1V1:
    *apFormat = QI_YCRCB_SP;
    *apSS = QI_H1V1;
    break;
  case QIYCBCRLP_H1V1:
    *apFormat = QI_YCBCR_SP;
    *apSS = QI_H1V1;
    break;
  case QIMONOCHROME:
    *apFormat = QI_MONOCHROME;
    *apSS = QI_H2V2;
    break;
  case QIIYUV_H2V2:
    *apFormat = QI_IYUV;
    *apSS = QI_H2V2;
    break;
  case QIYUV2_H2V2:
    *apFormat = QI_YUV2;
    *apSS = QI_H2V2;
    break;
  case QIIYUV_H2V1:
    *apFormat = QI_IYUV;
    *apSS = QI_H2V1;
    break;
  case QIYUV2_H2V1:
    *apFormat = QI_YUV2;
    *apSS = QI_H2V1;
    break;
  case QIIYUV_H1V2:
    *apFormat = QI_IYUV;
    *apSS = QI_H2V2;
    break;
  case QIYUV2_H1V2:
    *apFormat = QI_YUV2;
    *apSS = QI_H1V2;
    break;
  case QIIYUV_H1V1:
    *apFormat = QI_IYUV;
    *apSS = QI_H1V1;
    break;
  case QIYUV2_H1V1:
    *apFormat = QI_YUV2;
    *apSS = QI_H1V1;
    break;
  default:
    *apFormat = QI_YCRCB_SP;
    *apSS = QI_H2V2;
    break;
  }
}

/*===========================================================================
 * Function: PopulateOutput
 *
 * Description: populate the output params
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QJpegDecoderTest::PopulateOutput()
{
  int lrc = QI_SUCCESS;
  jpeg_header_t *lJpegHeader = mParser->GetHeader();
  jpeg_frame_info_t *lMainFrameInfo = lJpegHeader->p_main_frame_info;

  /*update size*/
  mSize = QISize(lMainFrameInfo->width, lMainFrameInfo->height);
  QIDBG_HIGH("%s:%d] dimension %dx%d", __func__, __LINE__, mSize.Width(),
    mSize.Height());

  /*update format*/
  mImageSize = QImage::getImageSize(mSize, mSS, mFormat);
  QIDBG_HIGH("%s:%d] mImageSize %d %d", __func__, __LINE__, mImageSize,
    mSize.Length());

  mOutputdata = QIHeapBuffer::New(mImageSize);
  if (mOutputdata == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  mOutput = new QImage(mSize, mSS, mFormat, mSize);
  if (mOutput == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  lrc = mOutput->setDefaultPlanes(QImage::GetPlaneCount(mFormat),
    mOutputdata->Addr(), -1);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  /*decode params*/
  mDecodeParams.setInputSize(mSize);
  mDecodeParams.setOutputSize(mSize);
  mDecodeParams.setFrameInfo(lMainFrameInfo);

  return QI_SUCCESS;
}

/*===========================================================================
 * Function: Start
 *
 * Description: start jpeg Decoder
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QJpegDecoderTest::Start()
{
  int lrc = QI_SUCCESS;
  QIDBG_MED("%s:%d] ", __func__, __LINE__);

  /* create Decoder*/
  mpDecoder = mFactory.CreateDecoder(QImageCodecFactory::HW_CODEC_ONLY,
    mDecodeParams);
  if (mpDecoder == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  lrc = PopulateOutput();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = mpDecoder->setDecodeParams(mDecodeParams);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = mpDecoder->addInputImage(*mInput);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = mpDecoder->addOutputImage(*mOutput);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = mpDecoder->addObserver(*this);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  mTime.Start();
  QI_LOCK(&mMutex);
  mError = QI_SUCCESS;
  lrc = mpDecoder->Start();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_GENERAL;
  }

  lrc = QIThread::WaitForCompletion(&mCond, &mMutex, 30000);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_GENERAL;
  }

  QI_UNLOCK(&mMutex);
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: print_usage
 *
 * Description: print the usage of the test application
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void print_usage()
{
  fprintf(stderr, "Usage: program_name [options] [-I <input file>]"
    " [-O <output file] [-W <width>] [-H <height>] [-F <format>]\n");
  fprintf(stderr, "Mandatory options:\n");
  fprintf(stderr, "  -I FILE\t\tPath to the input file.\n");
  fprintf(stderr, "  -O FILE\t\tPath for the output file.\n");
  fprintf(stderr, "  -f FORMAT\t\tOutput format: \n");
  fprintf(stderr, "\t\t\t\tYCRCBLP_H2V2 (%d - Default), YCBCRLP_H2V2 (%d),"
    " YCRCBLP_H2V1 (%d), YCBCRLP_H2V1 (%d),"
    " YCRCBLP_H1V2 (%d), YCBCRLP_H1V2 (%d),"
    " YCRCBLP_H1V1 (%d), YCBCRLP_H1V1 (%d),"
    " MONOCHROME(%d)"
    " IYUV_H2V2 (%d), YUV2_H2V2 (%d),"
    " IYUV_H2V1 (%d), YUV2_H2V1 (%d),"
    " IYUV_H1V2 (%d), YUV2_H1V2 (%d),"
    " IYUV_H1V1 (%d), YUV2_H1V1 (%d)\n",
    QIYCRCBLP_H2V2, QIYCBCRLP_H2V2, QIYCRCBLP_H2V1, QIYCBCRLP_H2V1,
    QIYCRCBLP_H1V2, QIYCBCRLP_H1V2, QIYCRCBLP_H1V1, QIYCBCRLP_H1V1,
    QIMONOCHROME,
    QIIYUV_H2V2, QIYUV2_H2V2, QIIYUV_H2V1, QIYUV2_H2V1,
    QIIYUV_H1V2, QIYUV2_H1V2, QIIYUV_H1V1, QIYUV2_H1V1);
  fprintf(stderr, "\n");
}

/*===========================================================================
 * Function: main
 *
 * Description: main Decoder test app routine
 *
 * Input parameters:
 *   argc - argument count
 *   argv - argument strings
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int main(int argc, char* argv[])
{
  int rc, c, i, val;
  int lrc = QI_SUCCESS;
  QJpegDecoderTest *lDecoder = new QJpegDecoderTest();
  uint64_t lDecodeTime = 0LL;
  int test_format = -1;

  fprintf(stderr, "=======================================================\n");
  fprintf(stderr, " Qualcomm Decoder test\n");
  fprintf(stderr, "=======================================================\n");
  opterr = 1;

  if (lDecoder == NULL) {
    QIDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return 0;
  }

  while ((c = getopt(argc, argv, "I:O:F:PZ")) != -1) {
    switch (c) {
    case 'O':
      lDecoder->mOutputFilename = optarg;
      fprintf(stderr, "%-25s%s\n", "Output image path",
        lDecoder->mOutputFilename);
      break;
    case 'I':
      lDecoder->mInputFilename = optarg;
      fprintf(stderr, "%-25s%s\n", "Input image path",
        lDecoder->mInputFilename);
      break;
    case 'F': {
      test_format = atoi(optarg);
      QICLAMP(test_format, 0, (QICOLOR_FORMAT_MAX-1));
      GetInputInfo((test_format_t)test_format, &lDecoder->mFormat,
        &lDecoder->mSS);
      break;
    }
    default:
      break;
    }
  }

  if ((lDecoder->mOutputFilename != NULL) &&
    (lDecoder->mOutputFilename != NULL) &&
    (test_format >= 0)) {
    fprintf(stderr, "%-25s\n", "Decode started");
  } else {
    print_usage();
    return 1;
  }

  lrc = lDecoder->Init();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    goto error;
  }

  lrc = lDecoder->Read();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    goto error;
  }

  lrc = lDecoder->StartExifparser();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    goto error;
  }

  lrc = lDecoder->Start();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    goto error;
  }
  lDecodeTime = lDecoder->mTime.GetTimeInMilliSec();

  fprintf(stderr, "%-25s%d\n", "Input image width", lDecoder->mSize.Width());
  fprintf(stderr, "%-25s%d\n", "Input image height", lDecoder->mSize.Height());
  lrc = lDecoder->Write();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    goto error;
  }
  fprintf(stderr, "%-25s%lld\n", "Decode completed in milliseconds ",
    lDecodeTime);

  if (lDecoder) {
    delete lDecoder;
  }
  return 0;

error:
  fprintf(stderr, "%-25s\n", "Decode error");
  if (lDecoder) {
    delete lDecoder;
  }
  return 0;
}
