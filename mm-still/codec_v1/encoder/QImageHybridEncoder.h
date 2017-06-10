/*****************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential. *
*****************************************************************************/

#ifndef __QIMAGE_HYBRID_ENCODER_H__
#define __QIMAGE_HYBRID_ENCODER_H__

#include "QImageEncoderInterface.h"
#include "QIThread.h"

extern "C" {

#include "jpeg_buffer_private.h"
#include "jpege_engine_sw.h"
#include "jpege.h"
#include "hybrid_encoder_lib.h"

} // extern "C"

class QImageHybridEncoder;


/*===========================================================================
 * Class: QIHybridEncoderThread
 *
 * Description: This class represents the encoder main thread
 *
 * Notes: none
 *==========================================================================*/
class QIHybridEncoderThread : public QIThread {

public:

  /** QIHybridEncoderThread:
   *  @aEncoder: Encoder object reference
   *
   *  constructor
   **/
  QIHybridEncoderThread(QImageHybridEncoder &aEncoder);

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
  QImageHybridEncoder &mEncoder;
};

/*===========================================================================
 * Class: QIHybridOutputThread
 *
 * Description: This class represents the output handler thread
 *
 * Notes: none
 *==========================================================================*/
class QIHybridOutputThread : public QIThread {

public:

  /** QIHybridOutputThread:
   *  @aEncoder: Encoder object reference
   *
   *  constructor
   **/
  QIHybridOutputThread(QImageHybridEncoder &aEncoder);

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
  QImageHybridEncoder &mEncoder;
};

/*===========================================================================
 * Class: QImageHybridEncoder
 *
 * Description: This class represents the Hybrid encoder component
 *
 * Notes: none
 *==========================================================================*/
class QImageHybridEncoder : public QImageEncoderInterface {

public:

  /** mHybridObject:
   *
   *  jpeg Hybrid encoder object
   **/
  jpege_obj_t mHybridObject;

  /** mHybridConfig:
   *
   *  jpeg Hybrid encoder configuration
   **/
  test_args_t mHybridConfig;

  /** mThreadCtrlBlk:
  *
  *  jpeg Hybrid thread control block configuration
  **/

  thread_ctrl_blk_t mThreadCtrlBlk;

  /** mSource:
   *
   *  jpeg encoder source
   **/
  jpege_img_data_t mSource;

  /** mConfig:
   *
   *  jpeg encoder internal config
   **/

  jpege_cfg_t mConfig;

  /** mImgSource:
   *
   *  jpeg encoder internal source
   **/

  jpege_src_t mImgSource;

  /** mSource:
   *
   *  jpeg encoder internal image info
   **/
  jpege_img_data_t mImgInfo;

  /** mDest:
   *
   *  jpeg encoder internal destination
   **/

  jpege_dst_t mDest;

  /** mBuffers:
   *
   *  jpeg encoder destination buffers
   **/
  jpeg_buffer_t mBuffers[2];

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

  /** ~QImageHybridEncoder:
   *
   *  destructor
   **/
  virtual ~QImageHybridEncoder();

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

  /** QImageHybridEncoder:
   *
   *  private constructor
   **/
  QImageHybridEncoder();

  /** copyHuffTable:
   *  @ap_htable: huffman table used by Hybrid encoder core
   *  @aType: huffman table type
   *
   *  This function is used to copy the huffman table from encoder
   *  parameter to the type used by Hybrid engine core
   **/
  int copyHuffTable(jpeg_huff_table_t *ap_htable,
    QIHuffTable::QHuffTableType aType);

  /** FillDestBuffer:
   *  @aForceFlush: force flushing
   *
   *  This function is used to fill the destination buffers
   **/
  int FillDestBuffer(bool aForceFlush);

  /** Encode:
   *
   *  Hybrid encoder main function
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
   *  Hybrid encoder engine
   **/
  jpege_engine_sw_t mEngine;

  /** mpOutputThread:
   *
   *  output thread
   **/
  QIHybridOutputThread *mpOutputThread;

  /** mEncodeParams:
   *
   *  jpeg encoder parameters
   **/
  QIEncodeParams *mEncodeParams;



  /** mpEncoderThread:
   *
   *  encoder main thread
   **/
  QIHybridEncoderThread *mpEncoderThread;

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

#ifdef USE_PERFORMANCE_LOCK
  /** mPerfLibHandle:
   *
   *  performance lib handle
   **/
  void* mPerfLibHandle;

  /** mPerfLockHandle:
   *
   *  performance lock handle
   **/
  void* mPerfLockHandle;
#endif //USE_PERFORMANCE_LOCK

  /**
   *  QIHybridOutputThread is added as friend class to access the
   *  private variables of encoder class
   **/
  friend class QIHybridOutputThread;

  /**
   *  QIHybridEncoderThread is added as friend class to access the
   *  private variables of encoder class
   **/
  friend class QIHybridEncoderThread;


  /** mUseFastCv:
   *
   *  Use FastCV lib for scaling
   **/
  bool mUseFastCv;

  /** mScaleFactorH:
   *
   *  Horizontal scale factor
   **/
  float mScaleFactorH;

  /** mScaleFactorV:
   *
   *  Vertical scale factor
   **/
  float mScaleFactorW;

  /** mRszOutSize:
   *
   *  Output size of fastCV scaled image
   **/
  QISize mRszOutSize;

  /** mRszOutImage:
   *
   *  Output fastCV scaled image
   **/
  QImage *mRszOutImage;

  /** mRszOutBuffer:
   *
   *  Output fastCV scaled image buffer
   **/
  QIBuffer *mRszOutBuffer;

  /** mScaleEnabled:
   *
   *  Flag indicating scaling is enabled
   **/
  bool mScaleEnabled;

  /** Resize:
   *
   *  Resize image using fastCV
   **/
  int Resize();

  /** mUseMultiFastCV:
   *
   *  Enables multi threaded FastCV scaling
   **/
   bool mUseMultiFastCV;

   /** mfastCVbuffer:
    *
    *  To hold the FastCV scaled temporary
       output.
    **/
    uint8_t *mfastCVbuffer;

  /** reconfigureInputParamaters()
   *
   *  Reconfigures the session for the
   *  encoding of the scaled image.
   **/
  void reconfigureInputParamaters();
};

#endif //__QIMAGE_HYBRID_ENCODER_H__
