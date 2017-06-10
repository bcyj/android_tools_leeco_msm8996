/*****************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGE_Q6_ENCODER_H__
#define __QIMAGE_Q6_ENCODER_H__

#include "QImageEncoderInterface.h"
#include "QIThread.h"

extern "C" {

#include "jpeg_buffer_private.h"
#include "jpege_engine_sw.h"
#include "jpege.h"
#include "adsp_jpege.h"
} // extern "C"


typedef struct jpege_engine_q6 {
  /** abort_flag:
   *
   * abort flag
   **/
    uint8_t abort_flag;
  /** error_flag:
   *
   * error flag
   **/
    uint8_t error_flag;
  /** jpege_q6_enc_cfg_target_t:
   *
   * Q6 encoder struct
   **/
    jpege_q6_enc_cfg_target_t   q6_enc_target_cfg;

} jpege_engine_q6_t;

class QImageQ6Encoder;

/*===========================================================================
 * Class: QIQ6EncoderThread
 *
 * Description: This class represents the encoder main thread
 *
 * Notes: none
 *==========================================================================*/
class QIQ6EncoderThread : public QIThread {

public:

  /** QIQ6EncoderThread:
   *  @aEncoder: Encoder object reference
   *
   *  constructor
   **/
  QIQ6EncoderThread(QImageQ6Encoder &aEncoder);

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
  QImageQ6Encoder &mEncoder;
};

/*===========================================================================
 * Class: QIQ6OutputThread
 *
 * Description: This class represents the output handler thread
 *
 * Notes: none
 *==========================================================================*/
class QIQ6OutputThread : public QIThread {

public:

  /** QIQ6OutputThread:
   *  @aEncoder: Encoder object reference
   *
   *  constructor
   **/
  QIQ6OutputThread(QImageQ6Encoder &aEncoder);

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
  QImageQ6Encoder &mEncoder;
};

/*===========================================================================
 * Class: QImageQ6Encoder
 *
 * Description: This class represents the SW encoder component
 *
 * Notes: none
 *==========================================================================*/
class QImageQ6Encoder : public QImageEncoderInterface {

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

  /** ~QImageQ6Encoder:
   *
   *  destructor
   **/
  virtual ~QImageQ6Encoder();

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

  jpege_engine_q6_t engine;

private:

  /** QImageQ6Encoder:
   *
   *  private constructor
   **/
  QImageQ6Encoder();

  /** copyHuffTable:
   *  @ap_htable: huffman table used by SW encoder core
   *  @aType: huffman table type
   *
   *  This function is used to copy the huffman table from encoder
   *  parameter to the type used by SW engine core
   **/
  int copyHuffTable(jpege_q6_huff_table_t *ap_htable, QIHuffTable::QHuffTableType aType);

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
  QIQ6OutputThread *mpOutputThread;

  /** mEncodeParams:
   *
   *  jpeg encoder parameters
   **/
  QIEncodeParams *mEncodeParams;

  /** mSource:
   *
   *  jpeg encoder source
   **/
  jpege_img_data_t mSource;

  /** mpEncoderThread:
   *
   *  encoder main thread
   **/
  QIQ6EncoderThread *mpEncoderThread;

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
   *  QIQ6OutputThread is added as friend class to access the
   *  private variables of encoder class
   **/
  friend class QIQ6OutputThread;

  /**
   *  QIQ6EncoderThread is added as friend class to access the
   *  private variables of encoder class
   **/
  friend class QIQ6EncoderThread;
};

#endif //__QIMAGE_Q6_ENCODER_H__
