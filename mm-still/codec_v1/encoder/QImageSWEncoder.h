/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGE_SW_ENCODER_H__
#define __QIMAGE_SW_ENCODER_H__

#include "QImageEncoderInterface.h"
#include "QIThread.h"

extern "C" {

#include "jpeg_buffer_private.h"
#include "jpege_engine_sw.h"
#include "jpege.h"

} // extern "C"

class QImageSWEncoder;

/*===========================================================================
 * Class: QISWEncoderThread
 *
 * Description: This class represents the encoder main thread
 *
 * Notes: none
 *==========================================================================*/
class QISWEncoderThread : public QIThread {

public:

  /** QISWEncoderThread:
   *  @aEncoder: Encoder object reference
   *
   *  constructor
   **/
  QISWEncoderThread(QImageSWEncoder &aEncoder);

  /** run:
   *  @aData: Thread object
   *
   *  Encoder thread main function
   **/
  virtual void run(QThreadObject *aData);

private:

  /** mEncoder:
   *
   *  Encoder core object reference
   **/
  QImageSWEncoder &mEncoder;
};

/*===========================================================================
 * Class: QISWOutputThread
 *
 * Description: This class represents the output handler thread
 *
 * Notes: none
 *==========================================================================*/
class QISWOutputThread : public QIThread {

public:

  /** QISWOutputThread:
   *  @aEncoder: Encoder object reference
   *
   *  constructor
   **/
  QISWOutputThread(QImageSWEncoder &aEncoder);

  /** run:
   *  @aData: Thread object
   *
   *  Output thread main function
   **/
  virtual void run(QThreadObject *aData);

private:

  /** mEncoder:
   *
   *  Encoder core object reference
   **/
  QImageSWEncoder &mEncoder;
};

/*===========================================================================
 * Class: QImageSWEncoder
 *
 * Description: This class represents the SW encoder component
 *
 * Notes: none
 *==========================================================================*/
class QImageSWEncoder : public QImageEncoderInterface {

public:

  /** QSWState:
   *  ESTATE_IDLE: Idle state
   *  ESTATE_ACTIVE: Active state
   *  ESTATE_STOP_REQUESTED: Stop pending state
   *  ESTATE_STOPPED: Stopped state
   *
   *  State of encoder
   **/
  typedef enum {
    ESTATE_IDLE,
    ESTATE_ACTIVE,
    ESTATE_STOP_REQUESTED,
    ESTATE_STOPPED,
  } QSWState;

  /** New:
   *  @aParams: encoder parameters
   *
   *  2 phase constructor
   **/
  static QImageEncoderInterface* New(QIEncodeParams &aParams);

  /** IsAvailable:
   *  @aEncodeParams: encoder parameters
   *
   *  This function is used to check if SW encoder can be used or
   *  not
   **/
  bool IsAvailable(QIEncodeParams& aEncodeParams);

  /** ~QImageSWEncoder:
   *
   *  destructor
   **/
  virtual ~QImageSWEncoder();

  /** Start:
   *
   *  starts the encoder
   **/
  int Start();

  /** Stop:
   *
   *  stops the encoder
   **/
  int Stop();

  /** setEncodeParams:
   *  @aParams: encoder parameters
   *
   *  sets encoder parameters
   **/
  int setEncodeParams(QIEncodeParams &aParams);

  /** addInputImage:
   *  @aImage: image object
   *
   *  add input image to encoder
   **/
  int addInputImage(QImage &aImage);

  /** addOutputImage:
   *  @aImage: image object
   *
   *  add output image to encoder
   **/
  int addOutputImage(QImage &aImage);

  /** addObserver:
   *  @aObserver: observer
   *
   *  add observer to the encoder
   **/
  int addObserver(QImageEncoderObserver &aObserver);

  /** SetOutputMode:
   *  @aMode: output mode
   *
   *  This function is used to set output mode
   **/
  int SetOutputMode(QIOutputMode aMode);

  void ReleaseSession();

private:

  /** QImageSWEncoder:
   *
   *  private constructor
   **/
  QImageSWEncoder();

  /** copyHuffTable:
   *  @ap_htable: huffman table used by SW encoder core
   *  @aType: huffman table type
   *
   *  This function is used to copy the huffman table from encoder
   *  parameter to the type used by SW engine core
   **/
  int copyHuffTable(jpeg_huff_table_t *ap_htable, QIHuffTable::QHuffTableType aType);

  /** FillDestBuffer:
   *  @aForceFlush: force flushing
   *
   *  This function is used to fill the destination buffers
   **/
  int FillDestBuffer(bool aForceFlush);

  /** Encode:
   *
   *  SW encoder main function
   **/
  void Encode();

  /** ConfigureDimensions:
   *
   *  This function is used configure the dimension
   **/
  void ConfigureDimensions();

private:

  /** mEngine:
   *
   *  SW encoder engine
   **/
  jpege_engine_sw_t mEngine;

  /** mpOutputThread:
   *
   *  output thread
   **/
  QISWOutputThread *mpOutputThread;

  /** mEncodeParams:
   *
   *  jpeg encoder parameters
   **/
  QIEncodeParams *mEncodeParams;

  /** mConfig:
   *
   *  jpeg encoder configuration
   **/
  jpege_img_cfg_t mConfig;

  /** mSource:
   *
   *  jpeg encoder source
   **/
  jpege_img_data_t mSource;

  /** mpEncoderThread:
   *
   *  encoder main thread
   **/
  QISWEncoderThread *mpEncoderThread;

  /** mInputImage:
   *
   *  encoder input image
   **/
  QImage *mInputImage;

  /** mOutputImage:
   *
   *  encoder output image
   **/
  QImage *mOutputImage;

  /** mState:
   *
   *  encoder state
   **/
  QSWState mState;

  /** mMode:
   *
   *  encoder output mode
   **/
  QIOutputMode mMode;

  /** mMutex:
   *
   *  mutex object
   **/
  pthread_mutex_t mMutex;

  /** mCond:
   *
   *  conditional variable
   **/
  pthread_cond_t mCond;

  /**
   *  QISWOutputThread is added as friend class to access the
   *  private variables of encoder class
   **/
  friend class QISWOutputThread;

  /**
   *  QISWEncoderThread is added as friend class to access the
   *  private variables of encoder class
   **/
  friend class QISWEncoderThread;
};

#endif //__QIMAGE_SW_ENCODER_H__
