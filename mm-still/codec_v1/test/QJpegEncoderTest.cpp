/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QJpegEncoderTest.h"
#include "QITime.h"

/*===========================================================================
 * Function: QJpegEncoderTest
 *
 * Description: QJpegEncoderTest default constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QJpegEncoderTest::QJpegEncoderTest()
{
  mpEncoder = NULL;
  mOutputFilename = NULL;
  mInputFilename = NULL;
  mInput = NULL;
  mOutput = NULL;
  mInputdata = NULL;
  mOutputdata = NULL;
  mSS = QI_H2V2;
  mImageSize = 0;
  mOutputSize = 0;
  QI_MUTEX_INIT(&mMutex);
  QI_COND_INIT(&mCond);
  mError = QI_SUCCESS;
  mQuality = 90;
  mFormat = QI_YCRCB_SP;
}

/*===========================================================================
 * Function: EncodeComplete
 *
 * Description: Callback function called when the encoding is done
 *
 * Input parameters:
 *   aOutputImage - output image which is encoded
 *
 * Return values:
 *   QI_SUCCESS
 *
 * Notes: none
 *==========================================================================*/
int QJpegEncoderTest::EncodeComplete(QImage *aOutputImage)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  QI_SIGNAL(&mCond);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: EncodeError
 *
 * Description: Callback function for error indication called when the
 *              encoding is done
 *
 * Input parameters:
 *   aErrorType - error type
 *
 * Return values:
 *   QI_SUCCESS
 *
 * Notes: none
 *==========================================================================*/
int QJpegEncoderTest::EncodeError(EncodeErrorType aErrorType)
{
  QIDBG_MED("%s:%d] Error %d", __func__, __LINE__, aErrorType);
  mError = QI_ERR_GENERAL;
  QI_SIGNAL(&mCond);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: OutputFragment
 *
 * Description: Callback function indicating the completion of piecewise
 *              ouput
 *
 * Input parameters:
 *   aBuffer - filled buffer sent by the component
 *
 * Return values:
 *   QI_SUCCESS
 *
 * Notes: none
 *==========================================================================*/
int QJpegEncoderTest::OutputFragment(QIBuffer &aBuffer)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: WriteComplete
 *
 * Description: Callback function called when exifcomposer is done writing
 *              the data
 *
 * Input parameters:
 *   aBuffer - filled buffer sent by the component
 *
 * Return values:
 *   QI_SUCCESS
 *
 * Notes: none
 *==========================================================================*/
void QJpegEncoderTest::WriteComplete(QIBuffer &aBuffer)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
}

/*===========================================================================
 * Function: WriteFragmentDone
 *
 * Description: Callback function called when buffer is filled
 *
 * Input parameters:
 *   aBuffer - filled buffer sent by the component
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QJpegEncoderTest::WriteFragmentDone(QIBuffer &aBuffer)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
}

/*===========================================================================
 * Function: WriteError
 *
 * Description: Callback function called when error in incurred during
 *              exif compose
 *
 * Input parameters:
 *   aErrorType - error type
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QJpegEncoderTest::WriteError(ErrorType aErrorType)
{
  QIDBG_MED("%s:%d] Error %d", __func__, __LINE__, aErrorType);
}

/*===========================================================================
 * Function: ~QJpegEncoderTest
 *
 * Description: QJpegEncoderTest destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QJpegEncoderTest::~QJpegEncoderTest()
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  if (mpEncoder) {
    delete mpEncoder;
    mpEncoder = NULL;
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
  if (mComposer) {
    delete mComposer;
    mComposer = NULL;
  }
  QI_MUTEX_DESTROY(&mMutex);
  QI_COND_DESTROY(&mCond);
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
}

/*===========================================================================
 * Function: Init
 *
 * Description: initializate the encoder test object
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
int QJpegEncoderTest::Init()
{
  QIDBG_MED("%s:%d] %p", __func__, __LINE__, this);

  mImageSize = QImage::getImageSize(mSize, mSS, mFormat);
  QIDBG_HIGH("%s:%d] mImageSize %d %d", __func__, __LINE__, mImageSize,
    mSize.Length());

  mInputdata = QIHeapBuffer::New(mImageSize);
  if (mInputdata == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  mOutputdata = QIHeapBuffer::New(mImageSize);
  if (mOutputdata == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  /*exif Composer*/
  mComposer = QExifComposer::New(*this);
  if (mComposer == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  /*encode params*/
  mEncodeParams.setQuality(mQuality);
  mEncodeParams.setDefaultTables(mQuality);
  mEncodeParams.setInputSize(mSize);
  mEncodeParams.setOutputSize(mSize);
  mEncodeParams.setRestartInterval(0);
  mEncodeParams.setRotation(0);

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
int QJpegEncoderTest::Read()
{
  FILE *fp = fopen(mInputFilename, "rb+");
  int lrc = 0;
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  if (fp) {
    lrc = fread(mInputdata->Addr(), 1, mImageSize, fp);
    QIDBG_HIGH("%s:%d] bytes_read %d", __func__, __LINE__, lrc);
    mInputdata->SetFilledLen(mImageSize);
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
int QJpegEncoderTest::Write()
{
  FILE *fp = fopen(mOutputFilename, "wb+");
  int lrc = 0;
  mOutputSize = mOutput->FilledLen() + mOutputdata->FilledLen();
  QIDBG_MED("%s:%d] filled len %d", __func__, __LINE__, mOutputSize);
  if (fp) {
    lrc = fwrite(mOutputdata->Addr(), 1, mOutputSize, fp);
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
 * Function: StartExifComposer
 *
 * Description: start exif composer
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
int QJpegEncoderTest::StartExifComposer()
{
  int lrc = QI_SUCCESS;
  QIDBG_MED("%s:%d] ", __func__, __LINE__);

  lrc = mComposer->addBuffer(mOutputdata);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  mExifParams.SetAppHeaderLen(0);
  mExifParams.SetEncodeParams(mEncodeParams);
  mExifParams.SetExif(NULL);
  mExifParams.SetSubSampling(mSS);

  lrc = mComposer->SetParams(mExifParams);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = mComposer->Start(NULL);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }
  return lrc;
}

/*===========================================================================
 * Function: Start
 *
 * Description: start jpeg encoder
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
int QJpegEncoderTest::Start()
{
  int lrc = QI_SUCCESS;
  QIDBG_MED("%s:%d] ", __func__, __LINE__);

  /* create encoder*/
  mpEncoder = mFactory.CreateEncoder(QImageCodecFactory::HW_CODEC_ONLY,
    mEncodeParams);
  if (mpEncoder == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  mInput = new QImage(mSize, mSS, mFormat);
  if (mInput == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  lrc = mInput->setDefaultPlanes(2, mInputdata->Addr(), -1);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  mOutput = new QImage(mOutputdata->Addr() + mOutputdata->FilledLen(),
    mOutputdata->Length() - mOutputdata->FilledLen(),
    QI_BITSTREAM);
  if (mOutput == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  lrc = mpEncoder->SetOutputMode(QImageEncoderInterface::ENORMAL_OUTPUT);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = mpEncoder->setEncodeParams(mEncodeParams);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = mpEncoder->addInputImage(*mInput);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = mpEncoder->addOutputImage(*mOutput);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = mpEncoder->addObserver(*this);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  QI_LOCK(&mMutex);
  mError = QI_SUCCESS;
  lrc = mpEncoder->Start();
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
  fprintf(stderr, "  -W WIDTH\t\tInput image width.\n");
  fprintf(stderr, "  -H HEIGHT\t\tInput image height.\n");
  fprintf(stderr, "  -Q QUALITY\t\tImage quality for compression.\n");
  fprintf(stderr, "\n");
}

/*===========================================================================
 * Function: main
 *
 * Description: main encoder test app routine
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
  QJpegEncoderTest *lEncoder = new QJpegEncoderTest();
  QITime lTime;
  uint64_t lEncodeTime = 0LL;

  fprintf(stderr, "=======================================================\n");
  fprintf(stderr, " Qualcomm Encoder test\n");
  fprintf(stderr, "=======================================================\n");
  opterr = 1;

  while ((c = getopt(argc, argv, "I:O:W:H:Q:PZ")) != -1) {
    switch (c) {
    case 'O':
      lEncoder->mOutputFilename = optarg;
      fprintf(stderr, "%-25s%s\n", "Output image path",
        lEncoder->mOutputFilename);
      break;
    case 'I':
      lEncoder->mInputFilename = optarg;
      fprintf(stderr, "%-25s%s\n", "Input image path",
        lEncoder->mInputFilename);
      break;
    case 'W':
      lEncoder->mSize.setWidth(atoi(optarg));
      fprintf(stderr, "%-25s%d\n", "Input image width",
        lEncoder->mSize.Width());
      break;
    case 'Q':
      lEncoder->mQuality = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input image width",
        lEncoder->mQuality);
      break;
    case 'H':
      lEncoder->mSize.setHeight(atoi(optarg));
      fprintf(stderr, "%-25s%d\n", "Input image height",
        lEncoder->mSize.Height());
      break;
    default:
      break;
    }
  }

  if ((lEncoder->mOutputFilename != NULL) &&
    (lEncoder->mOutputFilename != NULL) &&
    !(lEncoder->mSize.IsZero())) {
    fprintf(stderr, "%-25s\n", "Encode started");
  } else {
    print_usage();
    return 1;
  }

  lrc = lEncoder->Init();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = lEncoder->Read();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = lEncoder->StartExifComposer();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lTime.Start();

  lrc = lEncoder->Start();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }
  lEncodeTime = lTime.GetTimeInMilliSec();

  lrc = lEncoder->Write();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }
  fprintf(stderr, "%-25s%lld\n", "Encode completed in milliseconds ",
    lEncodeTime);
  fprintf(stderr, "%-25s%d\n", "Encoded size ", lEncoder->mOutputSize);
  delete lEncoder;
  return 0;
}
