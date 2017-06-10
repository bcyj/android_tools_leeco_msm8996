/*****************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QImageHWEncoder10.h"

/*============================================================================
   MACROS and CONSTANTS
============================================================================*/
#define IY 0
#define IC 1
#define IC1 1
#define IC2 2

#define ENCODE_TIMEOUT 10000

/* plane count for semiplanar and planar*/
#define MONO_CNT 1
#define SP_CNT 2
#define P_CNT 3

#define FLOOR_PAD_TO_16(a)  ((a>>4)<<4)
#define MAX_JPEGE_OUT_SIZE  FLOOR_PAD_TO_16(0x4000000)


#include <sched.h>

/*===========================================================================
 * Function: QImageHW10Encoder
 *
 * Description: QImageHW10Encoder constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageHW10Encoder::QImageHW10Encoder()
{
  QI_MUTEX_INIT(&mMutex);
  QI_COND_INIT(&mCond);
  mEncodeParams = NULL;
  mInputImage = NULL;
  mOutputImage = NULL;
  mState = ESTATE_IDLE;
  mJpegeLibHandle = NULL;
  mIONInput = NULL;
  mIONOutput = NULL;
  mOutputDone = false;
  mError = QI_SUCCESS;
  QI_MUTEX_INIT(&mLibMutex);
  QI_COND_INIT(&mLibCond);
  memset(&mJpegeLib, 0x0, sizeof(QJpegeLib_t));
  memset(&mEncodeCfg,  0x0, sizeof(jpege_cmd_jpeg_encode_cfg));
  memset(&mScaleCfg, 0, sizeof(jpege_cmd_scale_cfg));
  memset(&mInputCfg, 0, sizeof(jpege_cmd_input_cfg));
  mEncodeCfg.quantTblY = NULL;
  mEncodeCfg.quantTblChroma = NULL;
  for (int i = 0; i < QI_MAX_PLANES; i++) {
    mInAddr[i] = NULL;
  }
  /*enable synchronous mode*/
  mSyncMode = true;
  mObserverCnt = 0;
}


/*===========================================================================
 * Function: ReleaseSession
 *
 * Description: release the current encoding session
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QImageHW10Encoder::ReleaseSession()
{
  Stop();

  /*join the encoder thread*/
  mEncodeThread.JoinThread();

  if (mIONInput) {
    delete mIONInput;
    mIONInput = NULL;
  }

  if (mIONOutput) {
    delete mIONOutput;
    mIONOutput = NULL;
  }

  if (mEncodeCfg.quantTblY) {
    free(mEncodeCfg.quantTblY);
    mEncodeCfg.quantTblY = NULL;
  }
  if (mEncodeCfg.quantTblChroma) {
    free(mEncodeCfg.quantTblChroma);
    mEncodeCfg.quantTblChroma = NULL;
  }

  mObserverCnt = 0;
}

/*===========================================================================
 * Function: ~QImageHW10Encoder
 *
 * Description: QImageHW10Encoder destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageHW10Encoder::~QImageHW10Encoder()
{
  Stop();

  int lrc = 0;

  if (mJpegeLib.ptr) {
    mJpegeLib.jpege_lib_release(mJpegeLibHandle);
    if (lrc < 0)
      QIDBG_ERROR("%s:%d: release failed %d", __func__, __LINE__, lrc);
    mJpegeLibHandle = NULL;
  }

  /*join the encoder thread*/
  mEncodeThread.JoinThread();

  if (mIONInput) {
    delete mIONInput;
    mIONInput = NULL;
  }

  if (mIONOutput) {
    delete mIONOutput;
    mIONOutput = NULL;
  }

  QI_MUTEX_DESTROY(&mMutex);
  QI_COND_DESTROY(&mCond);

  if (mEncodeCfg.quantTblY) {
    free(mEncodeCfg.quantTblY);
    mEncodeCfg.quantTblY = NULL;
  }
  if (mEncodeCfg.quantTblChroma) {
    free(mEncodeCfg.quantTblChroma);
    mEncodeCfg.quantTblChroma = NULL;
  }
  /*unload the library*/
  UnLoad();
}

/*===========================================================================
 * Function: addInputImage
 *
 * Description: Adds the input buffer to the encoder
 *
 * Input parameters:
 *   aImage - reference to input image object
 *
 * Return values:
 *   QI_SUCCES
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::addInputImage(QImage &aImage)
{
  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, mState);
  QI_LOCK(&mMutex);
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_INVALID_OPERATION;
  }
  mInputImage = &aImage;
  QI_UNLOCK(&mMutex);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: addObserver
 *
 * Description: Adds the observer for the encoder
 *
 * Input parameters:
 *   aObserver - encoder observer
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_INVALID_OPERATION
 *   QI_ERR_OUT_OF_BOUNDS
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::addObserver(QImageEncoderObserver &aObserver)
{
  QIDBG_MED("%s:%d] state %d %p", __func__, __LINE__, mState, &aObserver);
  QI_LOCK(&(mMutex));
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }

  if (mObserverCnt >= MAX_IMG_OBSERVER_CNT) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_OUT_OF_BOUNDS;
  }
  mObserver[mObserverCnt++] = &aObserver;
  QI_UNLOCK(&mMutex);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: addOutputImage
 *
 * Description: Adds the output buffer to the encoder
 *
 * Input parameters:
 *   aImage - reference to output image object
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::addOutputImage(QImage &aImage)
{
  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, mState);
  QI_LOCK(&mMutex);
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_INVALID_OPERATION;
  }
  mOutputImage = &aImage;
  QI_UNLOCK(&mMutex);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: setEncodeParams
 *
 * Description: Sets the encoder parameters
 *
 * Input parameters:
 *   aParams - reference to encoder parameters
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::setEncodeParams(QIEncodeParams &aParams)
{
  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, mState);
  QI_LOCK(&mMutex);
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_INVALID_OPERATION;
  }
  mEncodeParams = &aParams;
  QI_UNLOCK(&mMutex);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: SetOutputMode
 *
 * Description: Sets the output mode. Currently piecewise output is not
 * supported
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_NOT_SUPPORTED
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::SetOutputMode(QIOutputMode aMode)
{
  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, mState);
  QI_LOCK(&(mMutex));
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }

  if (aMode != ENORMAL_OUTPUT) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_NOT_SUPPORTED;
  }
  mMode = aMode;
  QI_UNLOCK(&(mMutex));
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: OutputHandler
 *
 * Description: Event handler for the encoder
 *
 * Input parameters:
 *   aJpegeLibHandle - Jpege HW Obj
 *   aoutBuffer - Output Buffer
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::OutputHandler(jpege_hw_buf *aoutBuffer)
{
  if (NULL == aoutBuffer) {
    mOutputDone = false;
    mError = QI_ERR_GENERAL;
    return QI_SUCCESS;
  }

  QIDBG_MED("%s:%d] type %d", __func__, __LINE__, aoutBuffer->type);

  QI_LOCK(&mLibMutex);

  QIDBG_HIGH("%s %d:Frame done length %d", __func__, __LINE__,
    aoutBuffer->framedone_len);

  if (aoutBuffer->framedone_len > mOutputImage->Length()) {
    QIDBG_ERROR("%s %d:JPEG output length %d larger than buffer length %d",
       __func__, __LINE__, aoutBuffer->framedone_len, mOutputImage->Length());
    mError = QI_ERR_OUT_OF_BOUNDS;
    QI_UNLOCK(&mLibMutex);
    return QI_ERR_OUT_OF_BOUNDS;
  }
  if (mOutputImage->Fd() < 0) {
    //copy to output buffer if it not done in OMX layer
    memcpy(mOutputImage->BaseAddr(), aoutBuffer->vaddr,
      aoutBuffer->framedone_len);
  }

  mOutputImage->SetFilledLen(aoutBuffer->framedone_len);

  QI_UNLOCK(&mLibMutex);

  return QI_SUCCESS;
}

/*===========================================================================
 * Function: jpege_hw10_output_handler
 *
 * Description: Static function for output handler
 *
 * Input parameters:
 *   p_user - HW object pointer
 *   aBuffer - jpege_hw_buf Buffer
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int jpege_hw10_output_handler(void *p_user,
  jpege_hw_buf *aBuffer)
{
  QImageHW10Encoder *lEngine = (QImageHW10Encoder *)p_user;
  if (lEngine) {
    return lEngine->OutputHandler(aBuffer);
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: InputHandler
 *
 * Description: Event handler for the encoder
 *
 * Input parameters:
 *   aJpegeLibHandle - Jpege HW Obj
 *   aBuffer - jpege_hw_buf Buffer
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::InputHandler(jpege_hw_buf *aBuffer)
{
  QIDBG_MED("%s:%d] type %d", __func__, __LINE__, aBuffer->type);

  QI_LOCK(&mLibMutex);
  if (JPEG_EVT_SESSION_DONE == aBuffer->type) {
    mOutputDone = true;
  } else if (JPEG_EVT_ERROR == aBuffer->type) {
    mError = QI_ERR_GENERAL;
  }
  QI_SIGNAL(&mLibCond);
  QI_UNLOCK(&mLibMutex);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: jpege_hw10_input_handler
 *
 * Description: Static function for input handler
 *
 * Input parameters:
 *   p_user - HW object pointer
 *   aBuffer - jpege_hw_buf Buffer
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int jpege_hw10_input_handler(void *p_user, jpege_hw_buf *aBuffer)
{
  QImageHW10Encoder *lEngine = (QImageHW10Encoder *)p_user;
  if (lEngine) {
    return lEngine->InputHandler(aBuffer);
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: EventHandler
 *
 * Description: Event handler for the encoder
 *
 * Input parameters:
 *   aEvent - Event
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::EventHandler(int aEvent)
{
  QIDBG_MED("%s:%d] type %d", __func__, __LINE__, aEvent);

  QI_LOCK(&mLibMutex);
  if (JPEG_EVT_SESSION_DONE == aEvent) {
    QIDBG_HIGH("%s %d: JPEG_EVT_SESSION_DONE", __func__, __LINE__);
    mOutputDone = true;
  } else if (JPEG_EVT_ERROR == aEvent) {
    QIDBG_HIGH("%s %d: JPEG_EVT_ERROR", __func__, __LINE__);
    mError = QI_ERR_GENERAL;
  }
  QI_SIGNAL(&mLibCond);
  QI_UNLOCK(&mLibMutex);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: jpege_hw10_event_handler
 *
 * Description: Static function for event handler
 *
 * Input parameters:
 *   p_user - HW object pointer
 *   p_evt - pointer to the HW events
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int jpege_hw10_event_handler(void *p_user, jpege_hw_evt *aJpegCtrlCmd,
  int aEvent)
{
  QImageHW10Encoder *lEngine = (QImageHW10Encoder *)p_user;
  if (lEngine) {
    return lEngine->EventHandler(aEvent);
  }
  return QI_SUCCESS;
}
/*===========================================================================
 * Function: ConfigureOutputBuffer
 *
 * Description: Configure the output buffer
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::ConfigureOutputBuffer()
{
  int lrc = QI_SUCCESS;
  uint32_t lBufferlength = 0, lOffset = 0;

  jpege_hw_buf lJpegOutBuf;
  memset(&lJpegOutBuf, 0x0, sizeof(jpege_hw_buf));

  lBufferlength = mOutputImage->Length();

  QIDBG_HIGH("%s:%d] Received output image fd %d "
    "addr %p WorkBufSize %d ", __func__, __LINE__, mOutputImage->Fd(),
    mOutputImage->BaseAddr(), mOutputImage->WorkBufSize());

  if (mOutputImage->Fd() < 0) {
    /* output buffer not allocated, allocate it now */
    mIONOutput = QIONBuffer::New(lBufferlength);
    if (NULL == mIONOutput) {
      QIDBG_ERROR("%s:%d] cannot alloc input buffers", __func__, __LINE__);
      return QI_ERR_NO_MEMORY;
    }
    QIONBuffer::DoCacheOps(mIONOutput, QIONBuffer::CACHE_CLEAN_INVALIDATE);

    mIONOutput->SetFilledLen(lBufferlength);
    lJpegOutBuf.fd = mIONOutput->Fd();
    lJpegOutBuf.vaddr = mIONOutput->Addr();
    lJpegOutBuf.y_len = lBufferlength;

  } else {
    /* ION buffer has already been allocated in OMX layer */
    lJpegOutBuf.fd = mOutputImage->Fd();
    lJpegOutBuf.vaddr = mOutputImage->BaseAddr();
    lJpegOutBuf.y_len = mOutputImage->Length();
  }

  lrc = mJpegeLib.jpege_lib_output_buf_enq(mJpegeLibHandle,  &lJpegOutBuf);
  if (lrc) {
    delete mIONOutput;
    mIONOutput = NULL;
  }

  return lrc;
}

/*===========================================================================
 * Function: populatePlaneAddr
 *
 * Description: Populate the plane addresses of the buffers
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::populatePlaneAddr()
{
  QIPlane *lPlane = NULL;

  /*set the padded dimension*/
  if (MONO_CNT <= mInputImage->PlaneCount()) {
    lPlane = mInputImage->getPlane(QIPlane::PLANE_Y);
    if (NULL == lPlane) {
      QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    mInAddr[IY] = lPlane->ActualAddr();
    mInSize[IY] = lPlane->ActualSize();
    mInPadSize[IY] = lPlane->Size();
  }

  if (SP_CNT == mInputImage->PlaneCount()) {
    lPlane = mInputImage->getPlane(QIPlane::PLANE_CB_CR);
    if (NULL == lPlane) {
      QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    mInAddr[IC] = lPlane->ActualAddr();
    mInSize[IC] = lPlane->ActualSize();
    mInPadSize[IC] = lPlane->Size();
  } else if (P_CNT == mInputImage->PlaneCount()) {
    QIPlane::Type lType[2];

    if (QImage::IsCrCb(mInputImage->Format())) {
      lType[0] = QIPlane::PLANE_CR;
      lType[1] = QIPlane::PLANE_CB;
    } else {
      lType[0] = QIPlane::PLANE_CB;
      lType[1] = QIPlane::PLANE_CR;
    }

    for (int j = 0; j < 2; j++) {
      lPlane = mInputImage->getPlane(lType[j]);
      if (NULL == lPlane) {
        QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
        return QI_ERR_INVALID_INPUT;
      }
      mInAddr[j+1] = lPlane->ActualAddr();
      mInSize[j+1] = lPlane->ActualSize();
      mInPadSize[j+1] = lPlane->Size();
    }
  }
  QIDBG_MED("%s:%d] plane address %p %p %p", __func__, __LINE__,
    mInAddr[0], mInAddr[1], mInAddr[2]);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: ConfigureInputBuffer
 *
 * Description: Configure the input buffer
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::ConfigureInputBuffer()
{
  uint32_t lBufferlength = 0, lOffset = 0;
  int lrc = QI_SUCCESS;
  QIPlane *iplane;

  jpege_hw_buf lJpegBuf;
  memset(&lJpegBuf, 0x0, sizeof(jpege_hw_buf));

  //Configure the JPEG HW bufer structure
    QIDBG_MED("%s %d: Num of planes = %d", __func__, __LINE__,
    mInputImage->PlaneCount());

  lJpegBuf.type = 0x0;

  if (mInputImage->PlaneCount() == 1) {
    /* monochrome */
    if (!mInputImage->getPlane(QIPlane::PLANE_Y)) {
      QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    lBufferlength = mInSize[IY].Length();
    lJpegBuf.y_len = mInSize[IY].Length();
    iplane = mInputImage->getPlane(QIPlane::PLANE_Y);
    if(iplane==NULL) {
     QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
     return QI_ERR_INVALID_INPUT;
    }
    lJpegBuf.y_off = iplane->Offset();
  } else if (mInputImage->PlaneCount() == 2) {
    /* pseudo-planar input */
    if (!mInputImage->getPlane(QIPlane::PLANE_CB_CR) ||
      !mInputImage->getPlane(QIPlane::PLANE_Y)) {
      QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    iplane = mInputImage->getPlane(QIPlane::PLANE_CB_CR);
    if(iplane==NULL) {
      QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    lBufferlength = iplane->PhyOffset() + mInSize[IC].Length();
    iplane = mInputImage->getPlane(QIPlane::PLANE_CB_CR);
    if(iplane==NULL) {
      QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    lJpegBuf.y_len = iplane->PhyOffset();
    iplane = mInputImage->getPlane(QIPlane::PLANE_Y);
    if(iplane==NULL) {
      QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    lJpegBuf.y_off = iplane->Offset();
    lJpegBuf.cbcr_len = mInSize[IC].Length();
    iplane = mInputImage->getPlane(QIPlane::PLANE_CB_CR);
    if(iplane==NULL) {
      QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    lJpegBuf.cbcr_off = iplane->Offset();
  } else if (mInputImage->PlaneCount() == 3) {
    /* planar input */
    if (!mInputImage->getPlane(QIPlane::PLANE_CB) ||
      !mInputImage->getPlane(QIPlane::PLANE_CR) ||
      !mInputImage->getPlane(QIPlane::PLANE_Y)) {
      QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    lBufferlength = mInSize[IY].Length() + mInSize[IC1].Length() +
      mInSize[IC2].Length();
    lJpegBuf.y_len = mInSize[IY].Length();
    iplane = mInputImage->getPlane(QIPlane::PLANE_Y);
    if(iplane==NULL) {
      QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    lJpegBuf.y_off = iplane->Offset();
    lJpegBuf.cbcr_len = mInSize[IC1].Length();
    iplane = mInputImage->getPlane(QIPlane::PLANE_CB);
    if(iplane==NULL) {
      QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    lJpegBuf.cbcr_off = iplane->Offset();
    lJpegBuf.cr_len = mInSize[IC2].Length();
    iplane = mInputImage->getPlane(QIPlane::PLANE_CR);
    if(iplane==NULL) {
      QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    lJpegBuf.cr_offset = iplane->Offset();
  } else {
    QIDBG_ERROR("%s %d: Plance count %d not supported", __func__, __LINE__,
      mInputImage->PlaneCount());
    return QI_ERR_INVALID_INPUT;
  }
  QIDBG_MED("%s %d: Y-len = %d, y-offset =%d, cbcr_len = %d, cbcr_off = %d,"
    "cr_len = %d, cr_off = %d", __func__, __LINE__, lJpegBuf.y_len,
    lJpegBuf.y_off, lJpegBuf.cbcr_len, lJpegBuf.cbcr_off, lJpegBuf.cr_len,
    lJpegBuf.cr_offset);

  /*If the input ion buffer is not alread allocated, allocate it now*/
  if((iplane=mInputImage->getPlane(QIPlane::PLANE_Y))) {
    if (iplane->Fd() > 0) {
      iplane = mInputImage->getPlane(QIPlane::PLANE_Y);
      if(iplane==NULL) {
        QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
        return QI_ERR_INVALID_INPUT;
      }
      lJpegBuf.vaddr = iplane->Addr();
      iplane = mInputImage->getPlane(QIPlane::PLANE_Y);
      if(iplane==NULL) {
        QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
        return QI_ERR_INVALID_INPUT;
      }
      lJpegBuf.fd = iplane->Fd();
    } else {
      QIDBG_MED("%s:%d] lLength %d ", __func__, __LINE__, lBufferlength);

      mIONInput = QIONBuffer::New(lBufferlength);
      if (NULL == mIONInput) {
        QIDBG_ERROR("%s:%d] cannot alloc input buffers", __func__, __LINE__);
        return QI_ERR_NO_MEMORY;
      }

      memcpy(mIONInput->Addr(), mInAddr[IY], mInSize[IY].Length());
      if (mInputImage->PlaneCount() == 2) {
        memcpy((mIONInput->Addr() + mInSize[IY].Length()), mInAddr[IC],
        mInSize[IC].Length());
      } else if (mInputImage->PlaneCount() == 3) {
        memcpy(mIONInput->Addr() + mInSize[IY].Length(), mInAddr[IC1],
          mInSize[IC1].Length());
        memcpy(mIONInput->Addr() + mInSize[IY].Length() + mInSize[IC1].Length(),
          mInAddr[IC2], mInSize[IC2].Length());
      }

      QIONBuffer::DoCacheOps(mIONInput, QIONBuffer::CACHE_CLEAN_INVALIDATE);
      mIONInput->SetFilledLen(lBufferlength);

      QIDBG_ERROR("%s %d: I/p plane vaddress = %p", __func__, __LINE__,
        mIONInput->Addr());
      lJpegBuf.vaddr = mIONInput->Addr();
      lJpegBuf.fd = mIONInput->Fd();
     }
  } else {
        QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
        return QI_ERR_INVALID_INPUT;
    }

  lrc = mJpegeLib.jpege_lib_input_buf_enq(mJpegeLibHandle, &lJpegBuf);
  return lrc;
}

/*===========================================================================
 * Function: ConfigureInputParams
 *
 * Description: Configure the unput params for the HW encoder
 *
 * Input parameters:
 *  None
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::ConfigureInputParams()
{
  int lrc = QI_SUCCESS;
  QISubsampling lSubsampling = mInputImage->SubSampling();
  QIFormat lFormat = mInputImage->Format();
  /* Set the Color Format */
  switch (lSubsampling) {
  case QI_H2V2: {
    mInputCfg.inputFormat = JPEGE_INPUT_H2V2;
    break;
  }
  case QI_H2V1: {
    mInputCfg.inputFormat = JPEGE_INPUT_H2V1;
    break;
  }
  case QI_H1V1: {
    mInputCfg.inputFormat = JPEGE_INPUT_H1V1;
    break;
  }
  case QI_H1V2: {
    mInputCfg.inputFormat = JPEGE_INPUT_H1V2;
    break;
  }
  }

  /* Set the CBCR ordering */
  switch (lFormat) {
  case QI_YCBCR_SP: {
    mInputCfg.input_cbcr_order = 0;
    mInputCfg.num_of_input_plns = 2;
    break;
  }
  case QI_YCRCB_SP: {
    mInputCfg.input_cbcr_order = 1;
    mInputCfg.num_of_input_plns = 2;
    break;
  }
  case QI_IYUV: {
    mInputCfg.input_cbcr_order = 0;
    mInputCfg.num_of_input_plns = 3;
  }
  case QI_YUV2: {
    mInputCfg.input_cbcr_order = 1;
    mInputCfg.num_of_input_plns = 3;
    break;
  }
  case QI_MONOCHROME: {
    mInputCfg.input_cbcr_order = 0;
    mInputCfg.inputFormat = JPEGE_INPUT_MONOCHROME;
    mInputCfg.num_of_input_plns = 1;
    break;
  }
  default: {
    QIDBG_ERROR("%s %d: Format not supported = %d", __func__, __LINE__,
      lFormat);
    return QI_ERR_INVALID_INPUT;
  }

  } /*end of switch*/

  mInputCfg.image_height = mEncodeParams->InputSize().Height();
  mInputCfg.image_width = mEncodeParams->InputSize().Width();
  mInputCfg.stride = mInPadSize[IY].Width();
  mInputCfg.scanline = mInPadSize[IY].Height();

  QIDBG_MED("%s %d: Width = %d, Height = %d, cbcr order = %d, format = %d,"
    "num_of_input_plns = %d", __func__, __LINE__, mInputCfg.image_width,
    mInputCfg.image_height,  mInputCfg.input_cbcr_order,
    mInputCfg.inputFormat, mInputCfg.num_of_input_plns);

  return lrc;

}
/*===========================================================================
 * Function: ConfigureScaleParams
 *
 * Description: Configure the HW decoder component
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::ConfigureScaleParams()
{
  int lrc = QI_SUCCESS;
  QISize lOutSize = mEncodeParams->OutputSize();

  if (!mEncodeParams->Crop().isZero()) {
    mScaleCfg.crop_enable = true;
    mScaleCfg.scale_input_width = mEncodeParams->Crop().Width();
    mScaleCfg.scale_input_height = mEncodeParams->Crop().Height();
    mScaleCfg.h_offset =  mEncodeParams->Crop().Left();
    mScaleCfg.v_offset = mEncodeParams->Crop().Top();
  } else {
    mScaleCfg.scale_input_width = mEncodeParams->InputSize().Width();
    mScaleCfg.scale_input_height = mEncodeParams->InputSize().Height();
  }
  if (!lOutSize.IsZero()) {
    /* Check if scaling is enabled */
    if ((lOutSize.Width() != mScaleCfg.scale_input_width) ||
       (lOutSize.Height() != mScaleCfg.scale_input_height)) {
      mScaleCfg.scale_enable = true;
    }
    mScaleCfg.output_width = lOutSize.Width();
    mScaleCfg.output_height = lOutSize.Height();
  }
  return lrc;
}

/*===========================================================================
 * Function: ConfigureTables
 *
 * Description: Configure Quantization and Huffman tables
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::ConfigureTables()
{
  int lrc = QI_SUCCESS;
  QIQuantTable *lQTable = NULL;
  uint16_t lQtableArray[64];

  mEncodeCfg.restartInterval = 0;
  mEncodeCfg.bCustomHuffmanTbl = false;
  mEncodeCfg.huffmanTblYDcPtr = NULL;
  mEncodeCfg.huffmanTblYAcPtr = NULL;
  mEncodeCfg.huffmanTblCbcrDcPtr = NULL;
  mEncodeCfg.huffmanTblCbcrAcPtr = NULL;

  /*Set Quantization Table*/
  lQTable = mEncodeParams->QuantTable(QIQuantTable::QTABLE_LUMA);
  if (lQTable == NULL) {
    QIDBG_ERROR("%s:%d: failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }
  mEncodeCfg.quantTblY = (jpege_quantTable *)malloc(sizeof(jpege_quantTable));
  if (mEncodeCfg.quantTblY == NULL) {
    QIDBG_ERROR("%s:%d: failed to allocate memory", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }
  for (int i =0; i< QUANT_SIZE; i++) {
   mEncodeCfg.quantTblY->qtbl[i] = *(lQTable->Table()+i);
  }
  lQTable = mEncodeParams->QuantTable(QIQuantTable::QTABLE_CHROMA);
  if (lQTable == NULL) {
    QIDBG_ERROR("%s:%d: failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }
  mEncodeCfg.quantTblChroma = (jpege_quantTable *)
    malloc(sizeof(jpege_quantTable));
  if (mEncodeCfg.quantTblChroma == NULL) {
    QIDBG_ERROR("%s:%d: failed to allocate memory", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }
  for (int i =0; i< QUANT_SIZE; i++) {
    mEncodeCfg.quantTblChroma->qtbl[i] = *(lQTable->Table()+i);
  }
  return lrc;
}
/*===========================================================================
 * Function: SetHWConfig
 *
 * Description: Configure the HW encoder component
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::SetHWConfig()
{
  int lrc = QI_SUCCESS;

  lrc = ConfigureScaleParams();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring Scale params", __func__, __LINE__);
    return lrc;
  }
  QIDBG_MED("%s %d: Scale enabled = %d, crop enabled = %d, crop width = %d,"
    "crop height = %d, output width = %d output height = %d", __func__,
    __LINE__, mScaleCfg.scale_enable, mScaleCfg.crop_enable,
    mScaleCfg.scale_input_width, mScaleCfg.scale_input_height,
    mScaleCfg.output_width, mScaleCfg.output_height);

  mInputCfg.hw_buf_size = mOutputImage->WorkBufSize();
  QIDBG_MED("%s %d: WorkBufSize = %d", __func__,
    __LINE__, mInputCfg.hw_buf_size);

  lrc = ConfigureInputParams();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring Scale params", __func__, __LINE__);
    return lrc;
  }
  QIDBG_MED("%s:%d] Width = %d, Height = %d, cbcr order = %d, format = %d,"
    "num_of_input_plns = %d Stride = %d Scanline = %d",
    __func__, __LINE__, mInputCfg.image_width,
    mInputCfg.image_height, mInputCfg.input_cbcr_order,
    mInputCfg.inputFormat, mInputCfg.num_of_input_plns,
    mInputCfg.stride, mInputCfg.scanline);

  lrc = ConfigureTables();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring Quantization tables",
      __func__, __LINE__);
    return lrc;
  }
  mEncodeCfg.speed_mode = JPEG_SPEED_NORMAL;
  if (mEncodeParams->HiSpeed()) {
    mEncodeCfg.speed_mode = JPEG_SPEED_HIGH;
  }
  QIDBG_MED("%s %d: mScaleCfg = %p", __func__, __LINE__, &mScaleCfg);
  lrc = mJpegeLib.jpege_lib_hw_config(mJpegeLibHandle, &mInputCfg, &mEncodeCfg,
    &mScaleCfg);

   return lrc;
}
/*===========================================================================
 * Function: Configure
 *
 * Description: Configure the HW decoder component
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::Configure()
{
  int lrc = QI_SUCCESS;

  lrc = populatePlaneAddr();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring plane address", __func__, __LINE__);
    return lrc;
  }
  lrc = SetHWConfig();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring HW params", __func__, __LINE__);
    return lrc;
  }
  lrc = ConfigureOutputBuffer();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring output buffer", __func__, __LINE__);
    return lrc;
  }
  lrc = ConfigureInputBuffer();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring input buffer", __func__, __LINE__);
    return lrc;
  }

  return lrc;
}
/*===========================================================================
 * Function: Start
 *
 * Description: Started the hardware encoder
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::Start()
{
  int lrc = QI_SUCCESS;

  QIDBG_MED("%s:%d] E state %d", __func__, __LINE__, mState);
  QI_LOCK(&mMutex);
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_INVALID_OPERATION;
  }

  if ((NULL == mInputImage) || (NULL == mOutputImage)
    || (NULL == mEncodeParams) || (mObserverCnt < 1)) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_INVALID_OPERATION;
  }

  lrc = Configure();
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] config failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_GENERAL;
  }

  lrc = mEncodeThread.StartThread(this);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return lrc;
  }

  mState = ESTATE_ACTIVE;
  QI_UNLOCK(&mMutex);
  return lrc;
}

/*===========================================================================
 * Function: Stop
 *
 * Description: Stops the hardware encoder
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::Stop()
{
  int lrc = QI_SUCCESS;

  QIDBG_MED("%s:%d] mState %d", __func__, __LINE__, mState);

  QI_LOCK(&(mMutex));

  if (mState != ESTATE_ACTIVE) {
    QI_UNLOCK(&(mMutex));
    return QI_SUCCESS;
  }

  mState = ESTATE_STOP_REQUESTED;

  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  QI_WAIT(&(mCond), &(mMutex));
  QIDBG_MED("%s:%d] ", __func__, __LINE__);

  QI_UNLOCK(&(mMutex));

  return QI_SUCCESS;
}

/*===========================================================================
 * Function: Execute
 *
 * Description: Executes the hardware encoder
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
void QImageHW10Encoder::Execute()
{
  if (mEncodeThread.IsSelf()) {
    /* encoder thread */
    Encode();
  }
}

/*===========================================================================
 * Function: Encode
 *
 * Description: Start Encode
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/

void QImageHW10Encoder::Encode()
{
  int lrc = QI_SUCCESS;
  struct jpege_hw_evt lEvent;
  struct jpege_hw_buf lInputBuf;
  struct jpege_hw_buf lOutputBuf;
  QImageEncoderObserver::EncodeErrorType lEncError;

  lrc = mJpegeLib.jpege_lib_encode(mJpegeLibHandle);
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] Error Encoding ", __func__, __LINE__);
    goto encode_error;
  }

  if (false == mSyncMode) {
    QIDBG_MED("%s:%d] waiting for session done event ", __func__, __LINE__);

    QI_LOCK(&mLibMutex);
    if (!mOutputDone && QI_SUCCEEDED(mError)) {
      lrc = QIThread::WaitForCompletion(&mLibCond, &mLibMutex, ENCODE_TIMEOUT);
    }
    QI_UNLOCK(&mLibMutex);

    QIDBG_MED("%s:%d] wait done %d", __func__, __LINE__, lrc);

    if (QI_ERROR(mError) || (QI_ERR_TIMEOUT == lrc)) {
      QIDBG_ERROR("%s:%d] error event", __func__, __LINE__);
      goto encode_error;
    }

    lrc = mJpegeLib.jpege_lib_wait_done(mJpegeLibHandle);
    if (lrc < 0) {
      QIDBG_ERROR("%s:%d: jpege_lib_wait_done failed", __func__, __LINE__);
      goto encode_error;
    }
  } else {
    lrc = mJpegeLib.jpege_lib_get_event(mJpegeLibHandle, &lEvent);
    if (QI_ERROR(lrc)) {
      QIDBG_ERROR("%s:%d] Error cannot get event", __func__, __LINE__);
      goto encode_error;
    }
    EventHandler(lEvent.type);

    lrc = mJpegeLib.jpege_lib_get_input(mJpegeLibHandle, &lInputBuf);
    if (QI_ERROR(lrc)) {
      QIDBG_ERROR("%s:%d] Error cannot get input", __func__, __LINE__);
      goto encode_error;
    }
    InputHandler(&lInputBuf);

    lrc = mJpegeLib.jpege_lib_get_output(mJpegeLibHandle, &lOutputBuf);
    if (QI_ERROR(lrc)) {
      QIDBG_ERROR("%s:%d] Error cannot get input", __func__, __LINE__);
      goto encode_error;
    }
    OutputHandler(&lOutputBuf);

    if (QI_ERROR(mError)) {
      goto encode_error;
    }
  }

  QI_LOCK(&mMutex);
  if (mState == ESTATE_STOP_REQUESTED) {
    goto stop;
  }
  mState = ESTATE_IDLE;
  QI_UNLOCK(&mMutex);

  for (uint32_t k = 0; k < mObserverCnt; k++)
    mObserver[k]->EncodeComplete(mOutputImage);

  return;

stop:
  mState = ESTATE_IDLE;
  QI_SIGNAL(&mCond);
  QI_UNLOCK(&mMutex);
  return;

encode_error:

  QI_LOCK(&mMutex);
  if (mState != ESTATE_STOP_REQUESTED) {
    lrc = mJpegeLib.jpege_lib_stop(mJpegeLibHandle);
    if (lrc < 0) {
      QIDBG_ERROR("%s:%d: Failed to stop the HW encoder", __func__, __LINE__);
    }
  } else {
    QI_SIGNAL(&mCond);
  }
  mState = ESTATE_IDLE;
  QI_UNLOCK(&mMutex);

  lEncError = QImageEncoderObserver::ERROR_GENERAL;
  if (mError == QI_ERR_OUT_OF_BOUNDS) {
    lEncError = QImageEncoderObserver::ERROR_OVERFLOW;
  }
  for (uint32_t k = 0; k < mObserverCnt; k++)
    mObserver[k]->EncodeError(lEncError);

  return;
}

/*===========================================================================
 * Function: Load
 *
 * Description: Loads the userspace encoder driver
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Encoder::Load()
{
  if (mJpegeLib.ptr) {
    QIDBG_ERROR("%s:%d: library already loaded", __func__, __LINE__);
    return QI_ERR_NOT_FOUND;
  }
  mJpegeLib.ptr = dlopen("libjpegehw.so", RTLD_NOW);
  if (!mJpegeLib.ptr) {
    QIDBG_ERROR("%s:%d] Error opening JpegE library", __func__, __LINE__);
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mJpegeLib.jpege_lib_init) =
    dlsym(mJpegeLib.ptr, "jpege_lib_init");
  if (!mJpegeLib.jpege_lib_init) {
    QIDBG_ERROR("%s:%d] Error linking jpege_lib_init",
      __func__, __LINE__);
    dlclose(mJpegeLib.ptr);
    mJpegeLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mJpegeLib.jpege_lib_encode) =
    dlsym(mJpegeLib.ptr, "jpege_lib_encode");
  if (!mJpegeLib.jpege_lib_encode) {
    QIDBG_ERROR("%s:%d] Error linking jpege_lib_encode",
      __func__, __LINE__);
    dlclose(mJpegeLib.ptr);
    mJpegeLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mJpegeLib.jpege_lib_hw_config) =
    dlsym(mJpegeLib.ptr, "jpege_lib_hw_config");
  if (!mJpegeLib.jpege_lib_hw_config) {
    QIDBG_ERROR("%s:%d] Error linking jpege_lib_hw_config",
      __func__, __LINE__);
    dlclose(mJpegeLib.ptr);
    mJpegeLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mJpegeLib.jpege_lib_input_buf_enq) =
    dlsym(mJpegeLib.ptr, "jpege_lib_input_buf_enq");
  if (!mJpegeLib.jpege_lib_input_buf_enq) {
    QIDBG_ERROR("%s:%d] Error linking jpege_lib_input_buf_enq",
      __func__, __LINE__);
    dlclose(mJpegeLib.ptr);
    mJpegeLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mJpegeLib.jpege_lib_output_buf_enq) =
    dlsym(mJpegeLib.ptr, "jpege_lib_output_buf_enq");
  if (!mJpegeLib.jpege_lib_output_buf_enq) {
    QIDBG_ERROR("%s:%d] Error linking jpege_lib_output_buf_enq",
      __func__, __LINE__);
    dlclose(mJpegeLib.ptr);
    mJpegeLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mJpegeLib.jpege_lib_release) =
    dlsym(mJpegeLib.ptr, "jpege_lib_release");
  if (!mJpegeLib.jpege_lib_release) {
    QIDBG_ERROR("%s:%d] Error linking jpege_lib_release",
      __func__, __LINE__);
    dlclose(mJpegeLib.ptr);
    mJpegeLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mJpegeLib.jpege_lib_wait_done) =
    dlsym(mJpegeLib.ptr, "jpege_lib_wait_done");
  if (!mJpegeLib.jpege_lib_wait_done) {
    QIDBG_ERROR("%s:%d] Error linking jpege_lib_wait_done",
      __func__, __LINE__);
    dlclose(mJpegeLib.ptr);
    mJpegeLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mJpegeLib.jpege_lib_stop) =
    dlsym(mJpegeLib.ptr, "jpege_lib_stop");
  if (!mJpegeLib.jpege_lib_stop) {
    QIDBG_ERROR("%s:%d] Error linking jpege_lib_stop",
      __func__, __LINE__);
    dlclose(mJpegeLib.ptr);
    mJpegeLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mJpegeLib.jpege_lib_get_event) =
    dlsym(mJpegeLib.ptr, "jpege_lib_get_event");
  if (!mJpegeLib.jpege_lib_get_event) {
    QIDBG_ERROR("%s:%d] Error linking jpege_lib_get_event",
      __func__, __LINE__);
    dlclose(mJpegeLib.ptr);
    mJpegeLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mJpegeLib.jpege_lib_get_input) =
    dlsym(mJpegeLib.ptr, "jpege_lib_get_input");
  if (!mJpegeLib.jpege_lib_get_input) {
    QIDBG_ERROR("%s:%d] Error linking jpege_lib_get_input",
      __func__, __LINE__);
    dlclose(mJpegeLib.ptr);
    mJpegeLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mJpegeLib.jpege_lib_get_output) =
    dlsym(mJpegeLib.ptr, "jpege_lib_get_output");
  if (!mJpegeLib.jpege_lib_get_output) {
    QIDBG_ERROR("%s:%d] Error linking jpege_lib_get_output",
      __func__, __LINE__);
    dlclose(mJpegeLib.ptr);
    mJpegeLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }
  QIDBG_HIGH("%s:%d] JpegE library loaded successfully", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: UnLoad
 *
 * Description: UnLoads the encoder userspace driver
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QImageHW10Encoder::UnLoad()
{
  int rc = 0;
  QIDBG_HIGH("%s:%d: ptr %p", __func__, __LINE__, mJpegeLib.ptr);
  if (mJpegeLib.ptr) {
    rc = dlclose(mJpegeLib.ptr);
    if (rc < 0)
      QIDBG_HIGH("%s:%d] error %s", __func__, __LINE__, dlerror());
    mJpegeLib.ptr = NULL;
  }
}

/*===========================================================================
 * Function: New
 *
 * Description: 2 phase constructor for QImageHW10Decoder
 *
 * Input parameters:
 *   aParams - decoder params
 *
 * Return values:
 *   decoder interface pointer
 *
 * Notes: none
 *==========================================================================*/
QImageEncoderInterface* QImageHW10Encoder::New(QIEncodeParams &aParams)
{
  QImageHW10Encoder* lEncoder = new QImageHW10Encoder();
  if (NULL == lEncoder) {
    QIDBG_ERROR("%s:%d] no memory", __func__, __LINE__);
    return NULL;
  }
  if (!lEncoder->IsAvailable(aParams)) {
    QIDBG_ERROR("%s:%d] validation failed", __func__, __LINE__);
    delete lEncoder;
    return NULL;
  }
  return lEncoder;
}

/*===========================================================================
 * Function: IsAvailable
 *
 * Description: Function to check if encode params are valid
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 *==========================================================================*/
bool QImageHW10Encoder::IsAvailable(QIEncodeParams& aParams)
{
  int lrc = QI_SUCCESS;
  int lJpegeHwFd = -1;

  QI_LOCK(&mMutex);

  /* check if rotation is enabled */
  if (aParams.Rotation() != 0) {
    QIDBG_ERROR("%s:%d: wont support rotation", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return false;
  }

  lrc = Load();
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d: cannot load library", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return false;
  }

  if (false == mSyncMode) {
    lJpegeHwFd = mJpegeLib.jpege_lib_init(&mJpegeLibHandle, this,
      jpege_hw10_event_handler, jpege_hw10_output_handler,
      jpege_hw10_input_handler);
  } else {
    lJpegeHwFd = mJpegeLib.jpege_lib_init(&mJpegeLibHandle, this,
      NULL, NULL, NULL);
  }
  if (lJpegeHwFd < 0 || !mJpegeLibHandle) {
    QIDBG_ERROR("%s %d: Failed to Initialize the Jpege HW lib",
      __func__, __LINE__);
    UnLoad();
    QI_UNLOCK(&mMutex);
    return false;
  }
  QI_UNLOCK(&mMutex);
  return true;
}
