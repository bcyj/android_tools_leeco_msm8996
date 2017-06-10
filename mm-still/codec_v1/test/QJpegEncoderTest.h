/*****************************************************************************
* Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.  *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QJPEGENCODER_TEST_H__
#define __QJPEGENCODER_TEST_H__

#include "QImageCodecFactoryA.h"
#include "QImageCodecFactoryB.h"
#include "QEncodeParams.h"
#include "QImage.h"
#include "QImageEncoderObserver.h"
#include "QIHeapBuffer.h"
#include "QIThread.h"
#include "QIBuffer.h"
#include "QExifComposerParams.h"
#include "QExifComposer.h"
#include <stdio.h>
#include <stdlib.h>

/*===========================================================================
 * Class: QJpegEncoderTest
 *
 * Description: This class represents the test application for encoder
 *
 * Notes: none
 *==========================================================================*/
class QJpegEncoderTest : public QImageEncoderObserver, QImageWriterObserver {

public:

  /** QJpegEncoderTest
   *
   *  constructor
   **/
  QJpegEncoderTest();

  /** OutputFragment
   *  @aBuffer: output buffer
   *
   *  callback when the encoding is completed for one buffer. This
   *  function is not used
   **/
  int OutputFragment(QIBuffer &aBuffer);

  /** EncodeError
   *  @aErrorType: error type
   *
   *  callback when the encoding error is incurred
   **/
  int EncodeError(EncodeErrorType aErrorType);

  /** EncodeComplete
   *  @aOutputImage: image object
   *
   *  callback when the encoding is completed
   **/
  int EncodeComplete(QImage *aOutputImage);

  /** WriteComplete
   *  @aBuffer: buffer which is filled by the composer
   *
   *  Callback issued when the image header is written
   *  successfully into the buffer
   **/
  void WriteComplete(QIBuffer &aBuffer);

  /** WriteFragmentDone
   *  @aBuffer: buffer which is filled by the composer
   *
   *  Callback issued when the buffer passed to the composer is
   *  written and is insufficient.
   *  Note that if this callback is issued, the client should call
   *  addBuffer to send more buffers
   **/
  void WriteFragmentDone(QIBuffer &aBuffer);

  /** WriteError
   *  @aErrorType: Error type
   *
   *  Callback issued when error is occured during composition
   **/
  void WriteError(ErrorType aErrorType);

  /** ~QJpegEncoderTest
   *
   *  destructor
   **/
  ~QJpegEncoderTest();

  /** Init
   *
   *  initializes the test app
   **/
  int Init();

  /** Read
   *
   *  read data from the file and fill the buffers
   **/
  int Read();

  /** Write
   *
   *  write data to the file
   **/
  int Write();

  /** Start
   *
   *  start encoder
   **/
  int Start();

  /** StartExifComposer
   *
   *  start exif composer
   **/
  int StartExifComposer();

  /** mFactory
   *
   *  codec factory
   **/
#ifdef CODEC_B
  QImageCodecFactoryB mFactory;
#else
  QImageCodecFactoryA mFactory;
#endif

  /** mpEncoder
   *
   *  encoder object
   **/
  QImageEncoderInterface *mpEncoder;

  /** mOutputFilename
   *
   *  output filename
   **/
  char *mOutputFilename;

  /** mInputFilename
   *
   *  input filename
   **/
  char *mInputFilename;

  /** mEncodeParams
   *
   *  encode paramters
   **/
  QIEncodeParams mEncodeParams;

  /** mInput
   *
   *  input image object
   **/
  QImage *mInput;

  /** mOutput
   *
   *  output image object
   **/
  QImage *mOutput;

  /** mInputdata
   *
   *  input buffer
   **/
  QIBuffer *mInputdata;

  /** mOutputdata
   *
   *  output buffer
   **/
  QIBuffer *mOutputdata;

  /** mSize
   *
   *  image size
   **/
  QISize mSize;

  /** mSS
   *
   *  image subsampling
   **/
  QISubsampling mSS;

  /** mImageSize
   *
   *  total image size
   **/
  uint32_t mImageSize;

  /** mOutputSize
   *
   *  output bitstream size
   **/
  uint32_t mOutputSize;

  /** mMutex
   *
   *  mutex variable object
   **/
  pthread_mutex_t mMutex;

  /** mCond
   *
   *  conditional variable object
   **/
  pthread_cond_t mCond;

  /** mError
   *
   *  last error value
   **/
  int mError;

  /** mQuality
   *
   *  image quality
   **/
  uint32_t mQuality;

  /** mFormat
   *
   *  image format
   **/
  QIFormat mFormat;

  /** mComposer
   *
   *  exif composer object
   **/
  QExifComposer *mComposer;

  /** mExifParams
   *
   *  exif composer parameters
   **/
  QExifComposerParams mExifParams;
};

#endif //__QJPEGENCODER_TEST_H__
