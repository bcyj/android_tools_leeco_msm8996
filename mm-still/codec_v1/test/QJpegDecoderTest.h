/*****************************************************************************
* Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.  *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QJPEGDECODER_TEST_H__
#define __QJPEGDECODER_TEST_H__

#include "QImageCodecFactoryB.h"
#include "QIDecoderParams.h"
#include "QImage.h"
#include "QImageDecoderObserver.h"
#include "QIHeapBuffer.h"
#include "QIThread.h"
#include "QIBuffer.h"
#include "QExifParser.h"
#include "QITime.h"
#include <stdio.h>
#include <stdlib.h>

/*===========================================================================
 * Class: QJpegDecoderTest
 *
 * Description: This class represents the test application for decoder
 *
 * Notes: none
 *==========================================================================*/
class QJpegDecoderTest : public QImageDecoderObserver, QImageReaderObserver {

public:

  /** QJpegDecoderTest
   *
   *  constructor
   **/
  QJpegDecoderTest();

  /** DecodeError
   *  @aErrorType: error type
   *
   *  callback when the decoding error is incurred
   **/
  int DecodeError(DecodeErrorType aErrorType);

  /** DecodeComplete
   *  @aOutputImage: image object
   *
   *  callback when the decoding is completed
   **/
  int DecodeComplete(QImage *aOutputImage);

  /** ReadComplete
   *  @aBuffer: Image buffer parsed
   *
   *  Callback issued when the Image buffer is parsed successfully
   *  by the component
   **/
  void ReadComplete(QIBuffer &aBuffer);

  /** ReadFragmentDone
   *  @aBuffer: Image buffer parsed
   *
   *  Callback issued when the Image buffer passed to the parser
   *  is read.
   *  Note that if this callback is issued, the client should call
   *  addBuffer to send more buffers
   **/
  void ReadFragmentDone(QIBuffer &aBuffer);

  /** ReadError
   *  @aErrorType: Error type
   *
   *  Callback issued when error is occured during parsing
   **/
  void ReadError(ErrorType aErrorType);

  /** PopulateOutput
   *
   *  populate the outputs
   **/
  int PopulateOutput();

  /** StartExifparser
   *
   *  start the exif parser
   **/
  int StartExifparser();

  /** ~QJpegDecoderTest
   *
   *  destructor
   **/
  ~QJpegDecoderTest();

  /** Init
   *
   *  initializes the decoder test app
   **/
  int Init();

  /** Read
   *
   *  read and fill the buffers
   **/
  int Read();

  /** Write
   *
   *  write the buffers to the file
   **/
  int Write();

  /** Start
   *
   *  start the decoder test app
   **/
  int Start();

  /** StartExifParser
   *
   *  start the exif parser
   **/
  int StartExifParser();

  /** mFactory
   *
   *  codec factory
   **/
#ifdef CODEC_B
  QImageCodecFactoryB mFactory;
#else
  QImageCodecFactoryA mFactory;
#endif

  /** mpDecoder
   *
   *  decoder object
   **/
  QImageDecoderInterface *mpDecoder;

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

  /** mDecodeParams
   *
   *  decode parameters
   **/
  QIDecodeParams mDecodeParams;

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

  /** mMutex
   *
   *  mutex object
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

  /** mParser
   *
   *  exif parser
   **/
  QExifParser *mParser;

  /** mFileLength
   *
   *  file length
   **/
  int mFileLength;

  /** mSize
   *
   *  image size
   **/
  QISize mSize;

  /** mFormat
   *
   *  image format
   **/
  QIFormat mFormat;

  /** mSS
   *
   *  image subsampling
   **/
  QISubsampling mSS;

  /** mImageSize
   *
   *  image size
   **/
  uint32_t mImageSize;

  /** mTime
   *
   *  time object
   **/
  QITime mTime;
};

#endif //__QJPEGDECODER_TEST_H__
