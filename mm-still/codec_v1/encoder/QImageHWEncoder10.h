/*****************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGE_HW10_ENCODER_H__
#define __QIMAGE_HW10_ENCODER_H__

#include "QImageEncoderInterface.h"
#include "QIThread.h"
#include "jpege_lib.h"
#include "jpege_lib_common.h"
#include "QIONBuffer.h"
#include <dlfcn.h>

/** QJpegeLib_t
 *  @jpege_lib_init: initializes the userspace driver
 *  @jpege_lib_release: deinitializes the userspace driver
 *  @jpege_lib_encode: start encoding
 *  @jpege_lib_input_buf_enq: input buffer configuration
 *  @jpege_lib_output_buf_enq: output buffer configuration
 *  @jpege_lib_wait_done: wait for threads to complete
 *  @jpege_lib_hw_config: configure the encoder registers
 *  @jpege_lib_stop: stop encoding
 *  @jpege_lib_get_event: get event (blocking call)
 *  @jpege_lib_get_input: get input buffer
 *  @jpege_lib_get_output: get output buffer
 *  @ptr : library handle
 *
 *  Jpeg 1.0 encoder userspace driver function pointers
 **/
typedef struct {

 int (*jpege_lib_init) (jpege_hw_obj_t * jpege_hw_obj, void *p_userdata,
    int (*event_handler) (jpege_hw_obj_t,struct jpege_hw_evt *,int event),
    int (*output_handler) (jpege_hw_obj_t, struct jpege_hw_buf *),
    int (*input_handler) (jpege_hw_obj_t, struct jpege_hw_buf *));

  int (*jpege_lib_release)(jpege_hw_obj_t jpege_hw_obj);

  int (*jpege_lib_hw_config) (jpege_hw_obj_t jpege_hw_obj,
    jpege_cmd_input_cfg * p_input_cfg,
    jpege_cmd_jpeg_encode_cfg * p_encode_cfg,
    jpege_cmd_scale_cfg * p_scale_cfg);

  int (*jpege_lib_input_buf_enq) (jpege_hw_obj_t jpege_hw_obj,
    struct jpege_hw_buf *);

  int (*jpege_lib_output_buf_enq) (jpege_hw_obj_t jpege_hw_obj,
    struct jpege_hw_buf *);

  int (*jpege_lib_encode) (jpege_hw_obj_t jpege_hw_obj);

  int (*jpege_lib_wait_done) (jpege_hw_obj_t jpege_hw_obj);

  int (*jpege_lib_stop) (jpege_hw_obj_t jpege_hw_obj);

  int (*jpege_lib_get_event)(jpege_hw_obj_t jpege_hw_obj,
    struct jpege_hw_evt *);

  int (*jpege_lib_get_input)(jpege_hw_obj_t jpege_hw_obj,
    struct jpege_hw_buf *);

  int (*jpege_lib_get_output)(jpege_hw_obj_t jpege_hw_obj,
    struct jpege_hw_buf *);

  void *ptr;

} QJpegeLib_t;

/*===========================================================================
 * Class: QImageHW10Encoder
 *
 * Description: This class represents the HW encoder component
 *
 * Notes: none
 *==========================================================================*/
class QImageHW10Encoder : public QImageEncoderInterface, public QThreadObject
{
  public:

  /** QState:
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
  } QState;

  /** ~QImageHW10Encoder:
  *
  *  Destructor
  **/
  ~QImageHW10Encoder();

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
  *  add input image to decoder
  **/
  virtual int addInputImage(QImage &aImage);

    /** OutputHandler
  *
  *  Output Handler for the hw encoder component
  **/
  int OutputHandler(jpege_hw_buf *aoutBuffer);

  /** InputHandler
  *
  *  Input Handler for the hw encoder component
  **/
  int InputHandler(jpege_hw_buf *aBuffer);

  /** EventHandler
  *
  *  Event Handler for the hw encoder component
  **/
  int EventHandler(int aEvent);

  void ReleaseSession();

  private:

  /** QImageHW10Encoder:
  *
  *  Default Constructor
  **/
  QImageHW10Encoder();

  /** ConfigureBuffers:
  *
  *  configure encoder settings
  **/
  int Configure();

  /** Load:
  *
  *  loads encoder library
  **/
  int Load();

  /** UnLoad:
  *
  *  unloads encoder library
  **/
  void UnLoad();

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

  /** ConfigureScaleParams
  *
  *  Configure scale parameters
  **/
  int ConfigureScaleParams();

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

  /** mJpegeLib:
  *
  *  Handle to the JPEGE Library
  **/
  QJpegeLib_t mJpegeLib;

  /** mJpegeLibHandle:
  *
  *  Userspace Driver handle
  **/
  void *mJpegeLibHandle;

  /** mOutputDone:
  *
  *  Flag to check if Output is done
  **/
  bool mOutputDone;

  /** mMode:
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
  jpege_cmd_jpeg_encode_cfg mEncodeCfg;

  /** mScaleCfg
  *
  * Scale data for the HW encoder
  **/
  jpege_cmd_scale_cfg mScaleCfg;

  /** mInputCfg
  *
  * Input data for the HW encoder
  **/
  jpege_cmd_input_cfg mInputCfg;

  /** mSyncMode
  *
  * Use synchronous mode for userspace driver interactions
  **/
  bool mSyncMode;

  /** mInPadSize
  *
  *  Array of input padded sizes
  **/
  QISize mInPadSize[QI_MAX_PLANES];

  int use_cpu;
};

#endif
