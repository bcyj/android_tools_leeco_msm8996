/*****************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/
#ifndef __QIMAGE_GEMINI_ENCODER_H__
#define __QIMAGE_GEMINI_ENCODER_H__

#include "QImageEncoderInterface.h"
#include "QIThread.h"
#include "QIONBuffer.h"
#include <dlfcn.h>

extern "C"{

#include "gemini_lib.h"

}


/*===========================================================================
 * Class: QImageGeminiEncoder
 *
 * Description: This class represents the HW encoder component for the Gemini
 * encoder on 8960/8064 targets
 * Notes: none
 *==========================================================================*/
class QImageGeminiEncoder : public QImageEncoderInterface, public QThreadObject
{
  public:

  /** QState:
   *  ESTATE_IDLE: Idle state
   *  ESTATE_ACTIVE: Active state
   *  ESTATE_STOP_REQUESTED: Stop pending state
   *
   *  State of encoder
   **/
  typedef enum {
    ESTATE_IDLE,
    ESTATE_ACTIVE,
    ESTATE_STOP_REQUESTED,
  } QState;

  /** ~QImageGeminiEncoder:
  *
  *  Destructor
  **/
  ~QImageGeminiEncoder();

  /** New:
   *  @aParams: encoder parameters
   *
   *  2 phase contsructor
   **/
  static QImageEncoderInterface* New(QIEncodeParams &aParams);

  /** IsAvailable:
  *  @aParams: encoder parameters
  *
  *  check if encoder is available
  **/
  bool IsAvailable(QIEncodeParams& aParams);

  /** Execute:
   *
   *  thread execute function
   **/
  virtual void Execute();

  /** Stop:
  *
  *  stop the encoder
  */
  virtual int Stop();

  /** Start:
  *
  *  starts the encoder
  **/
  virtual int Start();

  /** SetOutputMode
  *
  *  Set o/p mode to be ENORMAL_OUTPPUT or EPIECEWISE_OUTPUT
  **/
  virtual int SetOutputMode(QIOutputMode aMode);

  /** setEncodeParams:
   *  @aParams: encoder parameters
   *
   *  sets encode parameters
   **/
  virtual int setEncodeParams(QIEncodeParams &aParams);

  /** addOutputImage:
   *  @aImage: image object
   *
   *  add output image to encoder
   **/
  virtual int addOutputImage(QImage &aImage);

  /** addObserver:
  *  @aObserver: observer
  *
  *  add observer to the decoder
  **/
  virtual int addObserver(QImageEncoderObserver &aObserver);

  /** addInputImage:
  *  @aImage: image object
  *
  *  add input image to encoder
  **/
  virtual int addInputImage(QImage &aImage);

    /** OutputHandler
  *
  *  Output Handler for the hw encoder component
  **/
  int OutputHandler(gemini_buf *aoutBuffer);

  /** InputHandler
  *
  *  Input Handler for the hw encoder component
  **/
  int InputHandler(gemini_buf *aBuffer);

  /** EventHandler
  *
  *  Event Handler for the hw encoder component
  **/
  int EventHandler(int aEvent);

  private:

  /** QImageGeminiEncoder:
  *
  *  Default Constructor
  **/
  QImageGeminiEncoder();

  /** ConfigureBuffers:
  *
  *  configure encoder settings
  **/
  int Configure();

  /** ConfigureOutputBuffer:
  *
  *  configure output buffers
  **/
  int ConfigureOutputBuffer();

  /** ConfigureOutputBuffer
  *
  *  configure output buffers
  **/
  int ConfigureInputBuffer();

  /** ConfigureInputParams
  *
  *  Configure Input parameters
  **/
  int ConfigureInputParams();

  /** ConfigureTables
  *
  *  Configure Input parameters
  **/
  int ConfigureTables();

  /** ConfigureTables
  *
  *  Configure Output parameters
  **/
  int ConfigureOutputParams();

  /** populatePlaneAddr:
  *
  *  populate plane address
  **/
  int populatePlaneAddr();

  /** SetHWConfig:
  *
  *  Configure HW specific data
  **/
  int SetHWConfig();

  /** Encode:
  *
  *  Start Encode
  **/
  void Encode();

private:

  /** mMutex
  *
  *  mutex object
  **/
  pthread_mutex_t mMutex;

  /** mLibMutex
  *
  *  mutex for library calls
  **/
  pthread_mutex_t mLibMutex;

  /** mCond
  *
  *  conditional object
  **/
  pthread_cond_t mCond;

  /** mCond
  *
  *  condition for library calls
  **/
  pthread_cond_t mLibCond;

  /** mEncodeParams
  *
  *  Encoder parameters
  **/
  QIEncodeParams *mEncodeParams;

  /** mEncodeThread
  *
  *  Encode thread
  **/
  QIThread mEncodeThread;

  /** mInputImage
  *
  *  Input Image Object
  **/
  QImage *mInputImage;

  /** mOutputImage
  *
  *  Output Image Object
  **/
  QImage *mOutputImage;

  /** mIONInput
  *
  *  Input Ion buffer
  **/
  QIONBuffer *mIONInput;

  /** mIONOutput
  *
  *  Output Ion buffer
  **/
  QIONBuffer *mIONOutput;

  /** mInAddr
  *
  *  Array of input addresses
  **/
  uint8_t *mInAddr[QI_MAX_PLANES];

  /** mInSize
  *
  *  Array of input sizes
  **/
  QISize mInSize[QI_MAX_PLANES];

  /** mState:
   *
   *  Current state
   **/
  QState mState;

  /** mOutputDone:
  *
  *  Flag to check if Output is done
  **/
  bool mOutputDone;

  /** mIONInput:
  *
  *  Output mode
  **/
  QIOutputMode mMode;

  /** mError:
   *
   *  last error value
   **/
  int mError;

  /** mEncodeCfg
  *
  * Encode data(tables) for the HW encoder
  **/
  gemini_cmd_jpeg_encode_cfg mEncodeCfg;

  /** mInputCfg
  *
  * Data to configure the fetch engine of the HW encoder
  **/
  gemini_cmd_input_cfg mInputCfg;

  /** mEngine
  *
  * Gemini HW encoder Engine
  **/
  gmn_obj_t mEngine;

  /** mOutputCfg
  *
  * Data to configure the write engine of the HW encoder
  **/
  gemini_cmd_output_cfg mOutputCfg;

  /** mOperationCfg
  *
  * Configure the encoding mode
  **/
  gemini_cmd_operation_cfg mOperationCfg;


  /** mRotYOffset
  *
  * Rotation specific Y offset
  **/
  int mRotYOffset;

  /** mRotCrCbOffset
   *
   * Rotation specific CrCb offset
  **/
  int mRotCrCbOffset;

  /** mPadding
   *
   * Additional padding
  **/
  int mPadding;
};

#endif

