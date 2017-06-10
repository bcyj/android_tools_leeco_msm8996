/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGE_HW10_DECODER_H__
#define __QIMAGE_HW10_DECODER_H__

#include "QImageDecoderInterface.h"
#include "QIThread.h"
#include "jpegd_lib.h"
#include "QIONBuffer.h"


/** QJpegDLib_t
 *  @jpegd_lib_init: initializes the userspace driver
 *  @jpegd_lib_release: deinitializes the userspace driver
 *  @jpegd_lib_decode: start decoding
 *  @jpegd_lib_input_buf_cfg: input buffer configuration
 *  @jpegd_lib_output_buf_get: get output buffer
 *  @jpegd_lib_input_buf_get:  get input buffer
 *  @jpegd_lib_output_buf_cfg: output buffer configuration
 *  @jpegd_lib_wait_done: wait for threads to complete
 *  @jpegd_lib_configure_baseline: configure baseline decoder
 *  @ptr: library handle
 *
 *  Jpeg 1.0 decoder userspace driver function pointers
 **/
typedef struct {
  //
  int (*jpegd_lib_init)(void **handle, void *p_user,
    jpegd_evt_handler evt_handler);
  //
  int (*jpegd_lib_release)(void *handle);
  //
  int (*jpegd_lib_decode)(void *handle);
  //
  int (*jpegd_lib_input_buf_cfg)(void *handle, jpegd_buf *);
  //
  int (*jpegd_lib_output_buf_get)(void *handle, jpegd_buf *);
  //
  int (*jpegd_lib_input_buf_get)(void *handle, jpegd_buf *);
  //
  int (*jpegd_lib_output_buf_cfg)(void *handle, jpegd_buf *);
  //
  int (*jpegd_lib_wait_done)(void* handle);
  //
  int (*jpegd_lib_configure_baseline)(
    void *handle,
    jpegd_cmd_quant_cfg_t *,
    jpegd_cmd_huff_cfg_t *,
    jpegd_base_config_t *);
  //
  void *ptr;
} QJpegDLib_t;

/*===========================================================================
 * Class: QImageHW10Decoder
 *
 * Description: This class represents the HW decoder component
 *
 * Notes: none
 *==========================================================================*/
class QImageHW10Decoder : public QImageDecoderInterface, public QThreadObject {

public:

  /** QState:
   *  ESTATE_IDLE: Idle state
   *  ESTATE_ACTIVE: Active state
   *  ESTATE_STOP_REQUESTED: Stop pending state
   *  ESTATE_STOPPED: Stopped state
   *
   *  State of decoder
   **/
  typedef enum {
    ESTATE_IDLE,
    ESTATE_ACTIVE,
    ESTATE_STOP_REQUESTED,
    ESTATE_STOPPED,
  } QState;

  /** ~QImageHW10Decoder:
   *
   *  virtual destructor
   **/
  ~QImageHW10Decoder();

  /** New:
   *  @aParams: decoder parameters
   *
   *  2 phase contructor
   **/
  static QImageDecoderInterface* New(QIDecodeParams &aParams);

  /** IsAvailable:
   *  @aParams: decoder parameters
   *
   *  check if decoder is available
   **/
  bool IsAvailable(QIDecodeParams& aParams);

  /** Stop:
   *
   *  stop the decoder
   **/
  int Stop();

  /** Start:
   *
   *  starts the decoder
   **/
  int Start();

  /** setDecodeParams:
   *  @aParams: decoder parameters
   *
   *  sets decode parameters
   **/
  int setDecodeParams(QIDecodeParams &aParams);

  /** addOutputImage:
   *  @aImage: image object
   *
   *  add output image to decoder
   **/
  int addOutputImage(QImage &aImage);

  /** addObserver:
   *  @aObserver: observer
   *
   *  add observer to the decoder
   **/
  int addObserver(QImageDecoderObserver &aObserver);

  /** addInputImage:
   *  @aImage: image object
   *
   *  add input image to decoder
   **/
  int addInputImage(QImage &aImage);

  /** Execute:
   *
   *  thread execute function
   **/
  void Execute();

  /** EventHandler:
   *  @aEvt: jpeg event
   *
   *  event handler
   **/
  int EventHandler(jpegd_event_t *aEvt);

private:

  /** QImageHW10Decoder:
   *
   *  constructor
   **/
  QImageHW10Decoder();

  /** setScaleFactor:
   *
   *  set scale factor
   **/
  int setScaleFactor();

  /** Configure:
   *
   *  configure the decoder
   **/
  int Configure();

  /** Load:
   *
   *  loads decoder library
   **/
  int Load();

  /** UnLoad:
   *
   *  unloads decoder library
   **/
  void UnLoad();

  /** ConfigureBuffers:
   *
   *  configure decode buffers
   **/
  int ConfigureBuffers();

  /** ConfigureOutputBuffer:
   *
   *  configure output buffers
   **/
  int ConfigureOutputBuffer();

  /** ConfigureInputBuffer:
   *
   *  configure input buffers
   **/
  int ConfigureInputBuffer();

  /** setPaddedDimensions:
   *
   *  set padded dimension
   **/
  int setPaddedDimensions();

  /** populatePlaneAddr:
   *
   *  populate plane address
   **/
  int populatePlaneAddr();

  /** FillOutputImage:
   *
   *  fill output image
   **/
  int FillOutputImage();

  /** Decode:
   *
   *  main decode function
   **/
  void Decode();

private:

  /** mMutex:
   *
   *  mutex object
   **/
  pthread_mutex_t mMutex;

  /** mCond:
   *
   *  conditional object
   **/
  pthread_cond_t mCond;

  /** mDecodeParams:
   *
   *  decode paramters
   **/
  QIDecodeParams *mDecodeParams;

  /** mDecodeThread:
   *
   *  decode thread
   **/
  QIThread mDecodeThread;

  /** mInputImage:
   *
   *  input image
   **/
  QImage *mInputImage;

  /** mOutputImage:
   *
   *  output image
   **/
  QImage *mOutputImage;

  /** mState:
   *
   *  state of the component
   **/
  QState mState;

  /** mLibHandle:
   *
   *  userspace driver handle
   **/
  void *mLibHandle;

  /** mOutputDone:
   *
   *  flag to indicate whether output is done
   **/
  bool mOutputDone;

  /** mError:
   *
   *  last error value
   **/
  int mError;

  /** mLibMutex:
   *
   *  mutex for library calls
   **/
  pthread_mutex_t mLibMutex;

  /** mLibCond:
   *
   *  condition for library calls
   **/
  pthread_cond_t mLibCond;

  /** mDQTTables:
   *
   *  quant tables
   **/
  jpegd_cmd_quant_cfg_t mDQTTables;

  /** mPaddedSize:
   *
   *  padded sizes of each plane
   **/
  QISize mPaddedSize[QI_MAX_PLANES];

  /** mLib:
   *
   *  userspace driver library
   **/
  QJpegDLib_t mLib;

  /** mBaseCfg:
   *
   *  jpeg 1.0 decoder base configuration
   **/
  jpegd_base_config_t mBaseCfg;

  /** mIONInput:
   *
   *  input buffer
   **/
  QIONBuffer *mIONInput;

  /** mIONOutput:
   *
   *  output buffer
   **/
  QIONBuffer *mIONOutput;

  /** mOutAddr:
   *
   *  array of output address
   **/
  uint8_t *mOutAddr[QI_MAX_PLANES];

  /** mOutSize:
   *
   *  array of output sizes
   **/
  QISize mOutSize[QI_MAX_PLANES];
};

#endif //__QIMAGE_HW10_DECODER_H__
