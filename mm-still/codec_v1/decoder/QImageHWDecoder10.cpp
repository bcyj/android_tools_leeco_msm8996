/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QImageHWDecoder10.h"
#include <dlfcn.h>

/*============================================================================
   MACROS and CONSTANTS
============================================================================*/
#define IY 0
#define IC 1
#define IC1 1
#define IC2 2
/* plane count for semiplanar and planar*/
#define MONO_CNT 1
#define SP_CNT 2
#define P_CNT 3

#define DECODE_TIMEOUT 10000

typedef struct {
  float ratio;
  jpegd_scale_type_t scale_type;
} jpegd_scale_val_t;

static const jpegd_scale_val_t gScaleFactor[] =
{
  {1.0/8.0, SCALE_1_8},
  {2.0/8.0, SCALE_2_8},
  {3.0/8.0, SCALE_3_8},
  {4.0/8.0, SCALE_4_8},
  {5.0/8.0, SCALE_5_8},
  {6.0/8.0, SCALE_6_8},
  {7.0/8.0, SCALE_7_8},
  {    1.0, SCALE_NONE}
};

/*===========================================================================
 * Function: EventHandler
 *
 * Description: Event handler for the decoder
 *
 * Input parameters:
 *   aEvt - event from the userspace driver
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Decoder::EventHandler(jpegd_event_t *aEvt)
{
  QIDBG_MED("%s:%d] type %d", __func__, __LINE__, aEvt->type);

  QI_LOCK(&mLibMutex);
  if (JPEGD_EVT_FRAMEDONE == aEvt->type) {
    mOutputDone = true;
  } else if (JPEGD_EVT_ERROR == aEvt->type) {
    mError = QI_ERR_GENERAL;
  }
  QI_SIGNAL(&mLibCond);
  QI_UNLOCK(&mLibMutex);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: jpegd_hw10_event_handler
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
int jpegd_hw10_event_handler(void *p_user, jpegd_event_t *p_evt)
{
  QImageHW10Decoder *lEngine = (QImageHW10Decoder *)p_user;
  if (lEngine && p_evt) {
    return lEngine->EventHandler(p_evt);
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: IsAvailable
 *
 * Description: Function to check if decode params are valid
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 *==========================================================================*/
bool QImageHW10Decoder::IsAvailable(QIDecodeParams& aParams)
{
  int lrc = QI_SUCCESS;

  lrc = Load();
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] cannot load library", __func__, __LINE__);
    return false;
  }

  return true;
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
QImageDecoderInterface* QImageHW10Decoder::New(QIDecodeParams &aParams)
{
  QImageHW10Decoder* lDecoder = new QImageHW10Decoder();
  if (NULL == lDecoder) {
    QIDBG_ERROR("%s:%d] no memory", __func__, __LINE__);
    return NULL;
  }
  if (!lDecoder->IsAvailable(aParams)) {
    QIDBG_ERROR("%s:%d] validation failed", __func__, __LINE__);
    delete lDecoder;
    return NULL;
  }
  return lDecoder;
}

/*===========================================================================
 * Function: QImageHW10Decoder
 *
 * Description: QImageHW10Decoder constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageHW10Decoder::QImageHW10Decoder()
{
  QI_MUTEX_INIT(&mMutex);
  QI_COND_INIT(&mCond);
  mDecodeParams = NULL;
  mInputImage = NULL;
  mOutputImage = NULL;
  mState = ESTATE_IDLE;
  mLibHandle = NULL;
  mIONInput = NULL;
  mIONOutput = NULL;
  mOutputDone = false;
  mError = QI_SUCCESS;
  QI_MUTEX_INIT(&mLibMutex);
  QI_COND_INIT(&mLibCond);
  memset(&mBaseCfg, 0x0, sizeof(jpegd_base_config_t));
  memset(&mDQTTables, 0x0, sizeof(jpegd_cmd_quant_cfg_t));
  memset(&mLib, 0x0, sizeof(QJpegDLib_t));

  for (int i = 0; i < QI_MAX_PLANES; i++) {
    mOutAddr[i] = NULL;
  }
}

/*===========================================================================
 * Function: ~QImageHW10Decoder
 *
 * Description: QImageHW10Decoder destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageHW10Decoder::~QImageHW10Decoder()
{
  /*join the decoder thread*/
  mDecodeThread.JoinThread();

  /* Free quant tables */
  for (int i = 0; i < 4; i++) {
    JPEG_FREE(mDQTTables.p_qtables[i]);
  }

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

  /*unload the library*/
  UnLoad();
}

/*===========================================================================
 * Function: addInputImage
 *
 * Description: Adds the input buffer to the decoder
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
int QImageHW10Decoder::addInputImage(QImage &aImage)
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
 * Description: Adds the observer for the decoder
 *
 * Input parameters:
 *   aObserver - decoder observer
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_INVALID_OPERATION
 *   QI_ERR_OUT_OF_BOUNDS
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Decoder::addObserver(QImageDecoderObserver &aObserver)
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
 * Description: Adds the output buffer to the decoder
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
int QImageHW10Decoder::addOutputImage(QImage &aImage)
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
 * Function: setDecodeParams
 *
 * Description: Sets the decoder parameters
 *
 * Input parameters:
 *   aParams - reference to decoder parameters
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Decoder::setDecodeParams(QIDecodeParams &aParams)
{
  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, mState);
  QI_LOCK(&mMutex);
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_INVALID_OPERATION;
  }
  mDecodeParams = &aParams;
  QI_UNLOCK(&mMutex);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: Start
 *
 * Description: Started the hardware decoder
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
int QImageHW10Decoder::Start()
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
    || (NULL == mDecodeParams) || (mObserverCnt < 1)) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_INVALID_OPERATION;
  }

  lrc = mLib.jpegd_lib_init(&mLibHandle,
    (void *)this,
    jpegd_hw10_event_handler);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_GENERAL;
  }

  lrc = Configure();
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] config failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_GENERAL;
  }

  lrc = mDecodeThread.StartThread(this);
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
 * Description: Stops the hardware decoder
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
int QImageHW10Decoder::Stop()
{
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: Execute
 *
 * Description: Executes the hardware decoder
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
void QImageHW10Decoder::Execute()
{
  if (mDecodeThread.IsSelf()) {
    /* decoder thread */
    Decode();
  }
}

/*===========================================================================
 * Function: setScaleFactor
 *
 * Description: Configure the scale type of the component
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
int QImageHW10Decoder::setScaleFactor()
{
  float lInScaleRatio = mDecodeParams->InputSize().AspectRatio();
  float lOutScaleRatio = mDecodeParams->OutputSize().AspectRatio();
  float lScaleRatio = (float)mDecodeParams->OutputSize().Width()/
    mDecodeParams->InputSize().Width();

  QIDBG_HIGH("%s:%d] Input %dx%d ratio %f", __func__, __LINE__,
    mDecodeParams->InputSize().Width(),
    mDecodeParams->InputSize().Height(),
    mDecodeParams->InputSize().AspectRatio());
  QIDBG_HIGH("%s:%d] Output %dx%d ratio %f", __func__, __LINE__,
    mDecodeParams->OutputSize().Width(),
    mDecodeParams->OutputSize().Height(),
    mDecodeParams->OutputSize().AspectRatio());

  QIDBG_HIGH("%s:%d] scale ratio %f", __func__, __LINE__, lScaleRatio);

  if (!QIF_EQUAL(lInScaleRatio, lOutScaleRatio)) {
    QIDBG_ERROR("%s:%d] Aspect ratio doesnt match",
      __func__, __LINE__);
    return QI_ERR_INVALID_OPERATION;
  }

  int count = sizeof(gScaleFactor)/sizeof(gScaleFactor[0]);
  for (int i = 0; i < count; i++) {
    if (QIF_EQUAL(lScaleRatio, gScaleFactor[i].ratio)) {
      mBaseCfg.scale_factor = gScaleFactor[i].scale_type;
      QIDBG_HIGH("%s:%d] scale_type %d", __func__, __LINE__,
        mBaseCfg.scale_factor);
      return QI_SUCCESS;
    }
  }
  return QI_ERR_NOT_FOUND;
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
int QImageHW10Decoder::populatePlaneAddr()
{
  QIPlane *lPlane = NULL;

  /*set the padded dimension*/
  if (MONO_CNT <= mOutputImage->PlaneCount()) {
    lPlane = mOutputImage->getPlane(QIPlane::PLANE_Y);
    if (NULL == lPlane) {
      QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    mOutAddr[IY] = lPlane->ActualAddr();
    mOutSize[IY] = lPlane->ActualSize();
  }

  if (SP_CNT == mOutputImage->PlaneCount()) {
    lPlane = mOutputImage->getPlane(QIPlane::PLANE_CB_CR);
    if (NULL == lPlane) {
      QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    mOutAddr[IC] = lPlane->ActualAddr();
    mOutSize[IC] = lPlane->ActualSize();
  } else if (P_CNT == mOutputImage->PlaneCount()) {
    QIPlane::Type lType[2];

    if (QImage::IsCrCb(mOutputImage->Format())) {
      lType[0] = QIPlane::PLANE_CR;
      lType[1] = QIPlane::PLANE_CB;
    } else {
      lType[0] = QIPlane::PLANE_CB;
      lType[1] = QIPlane::PLANE_CR;
    }

    for (int j = 0; j < 2; j++) {
      lPlane = mOutputImage->getPlane(lType[j]);
      if (NULL == lPlane) {
        QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
        return QI_ERR_INVALID_INPUT;
      }
      mOutAddr[j+1] = lPlane->ActualAddr();
      mOutSize[j+1] = lPlane->ActualSize();
    }
  }
  QIDBG_MED("%s:%d] plane address %p %p %p", __func__, __LINE__,
    mOutAddr[0], mOutAddr[1], mOutAddr[2]);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: setPaddedDimensions
 *
 * Description: Sets the padded dimensions for the buffers
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
int QImageHW10Decoder::setPaddedDimensions()
{
  QIPlane *lPlane;
  QISize lPaddedSize = QImage::PaddedSize(*mOutputImage);
  float lHSS = QImage::getChromaWidthFactor(mOutputImage->SubSampling());
  float lVSS = QImage::getChromaHeightFactor(mOutputImage->SubSampling());

  QIDBG_HIGH("%s:%d] padded size %dx%d subsampling %fx%f plane cnt %d",
    __func__, __LINE__, lPaddedSize.Width(), lPaddedSize.Height(),
    lHSS, lVSS, mOutputImage->PlaneCount());

  /*set the padded dimension*/
  if (MONO_CNT == mOutputImage->PlaneCount()) {
    mPaddedSize[IY] = lPaddedSize;
  } else if (SP_CNT == mOutputImage->PlaneCount()) {
    mPaddedSize[IY] = lPaddedSize;
    mPaddedSize[IC] = QISize(lPaddedSize.Width() * lHSS * 2,
      lPaddedSize.Height() * lVSS);
  } else if (P_CNT == mOutputImage->PlaneCount()) {
    mPaddedSize[IY] = lPaddedSize;
    mPaddedSize[IC1] = mPaddedSize[IC2] =
      QISize(lPaddedSize.Width() * lHSS, lPaddedSize.Height() * lVSS);
  }
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
 *   QI_ERR_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Decoder::ConfigureInputBuffer()
{
  int lrc = QI_SUCCESS;
  uint32_t lOffset = mDecodeParams->FrameInfo()->pp_scan_infos[0]->offset;
  uint32_t lLength = mInputImage->Length();
  QIDBG_MED("%s:%d] lOffset %d lLength %d",
    __func__, __LINE__, lOffset, lLength);

  jpegd_buf lBuf;
  memset(&lBuf, 0x0, sizeof(jpegd_buf));

  QIDBG_ERROR("%s:%d] Input buffer fd is %d", __func__, __LINE__,
       mInputImage->Fd());

  if (mInputImage->Fd() < 0) {
    mIONInput = QIONBuffer::New(lLength - lOffset, false);
    if (NULL == mIONInput) {
      QIDBG_ERROR("%s:%d] cannot alloc input buffers", __func__, __LINE__);
      return QI_ERR_NO_MEMORY;
    }
    memcpy(mIONInput->Addr(), mInputImage->BaseAddr() + lOffset,
        mIONInput->Length());
    mIONInput->SetFilledLen(lLength);

    lBuf.type = 0;
    lBuf.vaddr = mIONInput->Addr();
    lBuf.fd = mIONInput->Fd();
    lBuf.y_len = mIONInput->Length();
  } else {
    lBuf.type = 0;
    lBuf.vaddr = mInputImage->BaseAddr();
    lBuf.fd = mInputImage->Fd();
    lBuf.y_len = mInputImage->Length() - lOffset;
    lBuf.offset = lOffset;
  }


  lrc = mLib.jpegd_lib_input_buf_cfg(mLibHandle, &lBuf);
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d]", __func__, __LINE__);
    return QI_ERR_GENERAL;
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
 *   QI_ERR_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QImageHW10Decoder::ConfigureOutputBuffer()
{
  int lrc = QI_SUCCESS;
  uint32_t lOutputLength = 0;
  jpegd_buf lBuf;
  memset(&lBuf, 0x0, sizeof(jpegd_buf));
  lBuf.type = 0;

  QIDBG_ERROR("%s:%d] Output buffer fd is %d", __func__, __LINE__,
      mOutputImage->Fd());

  for (uint32_t i = 0; i < mOutputImage->PlaneCount(); i++) {
    lOutputLength += mPaddedSize[i].Length();
  }
  QIDBG_MED("%s:%d] lOutputLength %d",
      __func__, __LINE__, lOutputLength);

  if (mOutputImage->Fd() < 0) {
    mIONOutput = QIONBuffer::New(lOutputLength, false);
    if (NULL == mIONOutput) {
      QIDBG_ERROR("%s:%d] cannot alloc input buffers", __func__, __LINE__);
      return QI_ERR_NO_MEMORY;
    }

    lBuf.vaddr = mIONOutput->Addr();
    lBuf.fd = mIONOutput->Fd();
    lBuf.y_len = mPaddedSize[IY].Length();
    lBuf.cbcr_len = mPaddedSize[IC1].Length();
    lBuf.pln2_len = mPaddedSize[IC2].Length();
  } else {
    lBuf.vaddr = mOutputImage->BaseAddr();
    lBuf.fd = mOutputImage->Fd();
    lBuf.y_len = mPaddedSize[IY].Length();
    lBuf.cbcr_len = mPaddedSize[IC1].Length();
    lBuf.pln2_len = mPaddedSize[IC2].Length();

    mOutputImage->SetFilledLen(lOutputLength);
  }

  if (SP_CNT <= mOutputImage->PlaneCount())
    lBuf.cbcr_off = lBuf.y_len;
  if (P_CNT == mOutputImage->PlaneCount())
    lBuf.pln2_off = lBuf.cbcr_len;

  lrc = mLib.jpegd_lib_output_buf_cfg(mLibHandle, &lBuf);
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d]", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: ConfigureBuffers
 *
 * Description: Configure the buffers
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
int QImageHW10Decoder::ConfigureBuffers()
{
  int lrc = QI_SUCCESS;

  lrc = setPaddedDimensions();
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return lrc;
  }

  lrc = populatePlaneAddr();
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return lrc;
  }

  lrc = ConfigureInputBuffer();
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return lrc;
  }

  lrc = ConfigureOutputBuffer();
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return lrc;
  }
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
int QImageHW10Decoder::Configure()
{
  int lrc = QI_SUCCESS;
  uint8_t lHSample[JPEGD_MAXCOMPONENTS];
  uint8_t lVSample[JPEGD_MAXCOMPONENTS];
  int lNumComp = 0;

  /* set the scale factor */
  lrc = setScaleFactor();
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return lrc;
  }

  /* get the format*/
  lNumComp = mDecodeParams->FrameInfo()->num_comps;
  QIDBG_MED("%s:%d] Num comp %d", __func__, __LINE__, lNumComp);
  for (int j = 0; j < lNumComp; j++) {
    lHSample[j] = mDecodeParams->FrameInfo()->p_comp_infos[j].sampling_h;
    lVSample[j] = mDecodeParams->FrameInfo()->p_comp_infos[j].sampling_v;
    QIDBG_MED("%s:%d] subsampling[%d] %dx%d",
      __func__, __LINE__, j, lHSample[j], lVSample[j]);
  }

  if (lNumComp == 1) {
    mBaseCfg.format = JPEG_GRAYSCALE;
  } else if (lNumComp == 3) {
    if ((lHSample[0] == 1) && (lVSample[0] == 1))
      mBaseCfg.format = JPEG_H1V1;
    else if ((lHSample[0] == 1) && (lVSample[0] == 2))
      mBaseCfg.format = JPEG_H1V2;
    else if ((lHSample[0] == 2) && (lVSample[0] == 1))
      mBaseCfg.format = JPEG_H2V1;
    else if ((lHSample[0] == 2) && (lVSample[0] == 2))
      mBaseCfg.format = JPEG_H2V2;
    else {
      QIDBG_ERROR("%s:%d] invalid ss", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
  } else {
    QIDBG_ERROR("%s:%d] invalid comp count", __func__, __LINE__);
    return QI_ERR_INVALID_INPUT;
  }

  /* configure quant tables */
  mDQTTables.qtable_present_flag =
    mDecodeParams->FrameInfo()->qtable_present_flag;

  QIDBG_HIGH("%s:%d, qtable_present_flag= 0x%x",
    __func__, __LINE__, mDQTTables.qtable_present_flag);

  /* Allocate qtable*/
  for (int i = 0; i < 4; i++) {
    if (!(mDQTTables.qtable_present_flag & (1 << i))) {
      continue;
    }
    mDQTTables.p_qtables[i] = (jpeg_quant_table_t)JPEG_MALLOC(QUANT_SIZE *
      sizeof(uint16_t));
    if (!mDQTTables.p_qtables[i])
      return QI_ERR_NO_MEMORY;

    memcpy(mDQTTables.p_qtables[i], mDecodeParams->FrameInfo()->p_qtables[i],
      QUANT_SIZE * sizeof(uint16_t));
  }

  /*Huffman tables*/
  jpegd_cmd_huff_cfg_t lHuffCfg;
  int lHuffCnt = 0;
  lHuffCfg.htable_present_flag =
    mDecodeParams->FrameInfo()->htable_present_flag;
  QIDBG_HIGH("%s:%d, htable_present_flag = 0x%x",
    __func__, __LINE__, lHuffCfg.htable_present_flag);

  for (int i = 0; i < 8; i++) {
    if (!(lHuffCfg.htable_present_flag & (1 << i))) {
      lHuffCnt++;
      continue;
    }
    for (int k = 1; k < 17; k++) {
      lHuffCfg.p_htables[lHuffCnt].bits[k-1] =
        mDecodeParams->FrameInfo()->p_htables[i].bits[k];
    }
    for (int k = 0; k < 256; k++) {
      lHuffCfg.p_htables[lHuffCnt].values[k] =
        mDecodeParams->FrameInfo()->p_htables[i].values[k];
    }
    lHuffCnt++;
  }

  mBaseCfg.width = mDecodeParams->InputSize().Width();
  mBaseCfg.height =  mDecodeParams->InputSize().Height();
  mBaseCfg.actual_width = mDecodeParams->InputSize().Width();
  mBaseCfg.actual_height = mDecodeParams->InputSize().Height();
  mBaseCfg.rotation = 0;
  mBaseCfg.crcb_order = QImage::IsCrCb(mOutputImage->Format());
  mBaseCfg.num_planes = mOutputImage->PlaneCount();
  mBaseCfg.restart_interval =
    mDecodeParams->FrameInfo()->restart_interval;

  lrc = mLib.jpegd_lib_configure_baseline(mLibHandle,
    &mDQTTables,
    &lHuffCfg,
    &mBaseCfg);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed %d", __func__, __LINE__, lrc);
    return QI_ERR_GENERAL;
  }

  lrc = ConfigureBuffers();
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return lrc;
  }

  return QI_SUCCESS;
}

/*===========================================================================
 * Function: FillOutputImage
 *
 * Description: Fill output image object provided by client
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
int QImageHW10Decoder::FillOutputImage()
{
  bool lLineCopy = (mOutputImage->Size().Width() != mPaddedSize[IY].Width());
  uint8_t *lAddr[QI_MAX_PLANES] = { NULL, NULL, NULL };
  QIPlane *lPlane = NULL;
  uint32_t lLength = 0;

  lAddr[IY] = mIONOutput->Addr();
  lAddr[IC1] = lAddr[IY] + mPaddedSize[IY].Length();
  lAddr[IC2] = lAddr[IC1] + mPaddedSize[IC1].Length();
  QIDBG_MED("%s:%d] Orig dim %dx%d Padded %dx%d linecopy %d",
    __func__, __LINE__,
    mOutputImage->Size().Width(),
    mOutputImage->Size().Height(),
    mPaddedSize[IY].Width(),
    mPaddedSize[IY].Height(),
    lLineCopy);


  for (uint32_t i = 0; i < mOutputImage->PlaneCount(); i++) {
    if (!lLineCopy) {
        memcpy(mOutAddr[i], lAddr[i], mOutSize[i].Length());
    } else {
      uint8_t *lDest = mOutAddr[i];
      uint8_t *lSrc = lAddr[i];
      QIDBG_MED("%s:%d] src: %p dst: %p w: %d h: %d", __func__, __LINE__, lSrc,lDest, mOutSize[i].Width(),
          mOutSize[i].Height());
      for (uint32_t j = 0; j < mOutSize[i].Height(); j++) {
        memcpy(lDest, lSrc, mOutSize[i].Width());
        lSrc += mPaddedSize[i].Width();
        lDest += mOutSize[i].Width();
      }
    }
    lLength += mOutSize[i].Length();
  }
  mOutputImage->SetFilledLen(lLength);

  return QI_SUCCESS;
}

/*===========================================================================
 * Function: Decode
 *
 * Description: Decoder main function
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
void QImageHW10Decoder::Decode()
{
  int lrc = QI_SUCCESS;
  jpegd_buf lBuf;

  QIDBG_MED("%s:%d] E", __func__, __LINE__);
  lrc = mLib.jpegd_lib_decode(mLibHandle);
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] Error Decoding ", __func__, __LINE__);
    goto decode_error;
  }

  QIDBG_MED("%s:%d] waiting for frame done event ", __func__, __LINE__);

  QI_LOCK(&mLibMutex);
  if (!mOutputDone && QI_SUCCEEDED(mError)) {
    lrc = QIThread::WaitForCompletion(&mLibCond, &mLibMutex, DECODE_TIMEOUT);
  }
  QI_UNLOCK(&mLibMutex);

  QIDBG_MED("%s:%d] wait done %d", __func__, __LINE__, lrc);

  if (QI_ERROR(mError) || (QI_ERR_TIMEOUT == lrc)) {

    QIDBG_ERROR("%s:%d] error event", __func__, __LINE__);
    mLib.jpegd_lib_input_buf_get(mLibHandle, &lBuf);
    mLib.jpegd_lib_output_buf_get(mLibHandle, &lBuf);
    goto decode_error;
  }

  /* get the buffers */
  mLib.jpegd_lib_input_buf_get(mLibHandle, &lBuf);
  mLib.jpegd_lib_output_buf_get(mLibHandle, &lBuf);

  if (mOutputImage->Fd() < 0) {
    lrc = FillOutputImage();
    if (QI_ERROR(lrc)) {
      QIDBG_ERROR("%s:%d] Error filling output", __func__, __LINE__);
      goto decode_error;
    }
  }

  lrc = mLib.jpegd_lib_wait_done(mLibHandle);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] jpegd_lib_wait_done failed", __func__, __LINE__);
    goto decode_error;
  }

  lrc = mLib.jpegd_lib_release(mLibHandle);
  if (lrc)
    QIDBG_ERROR("%s:%d] release failed %d", __func__, __LINE__, lrc);
  mLibHandle = NULL;

  for (uint32_t k = 0; k < mObserverCnt; k++)
    mObserver[k]->DecodeComplete(mOutputImage);

  return;

decode_error:

  lrc = mLib.jpegd_lib_release(mLibHandle);
  if (lrc)
    QIDBG_ERROR("%s:%d] release failed %d", __func__, __LINE__, lrc);
  mLibHandle = NULL;

  for (uint32_t k = 0; k < mObserverCnt; k++)
    mObserver[k]->DecodeError(QImageDecoderObserver::ERROR_GENERAL);
}

/*===========================================================================
 * Function: Load
 *
 * Description: Loads the decoder userspace driver
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
int QImageHW10Decoder::Load()
{
  if (mLib.ptr) {
    QIDBG_ERROR("%s:%d] library already loaded", __func__, __LINE__);
    return QI_ERR_NOT_FOUND;
  }

  mLib.ptr = dlopen("libjpegdhw.so", RTLD_NOW);
  if (!mLib.ptr) {
    QIDBG_ERROR("%s:%d] Error opening JpegD library", __func__, __LINE__);
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mLib.jpegd_lib_configure_baseline) =
    dlsym(mLib.ptr, "jpegd_lib_configure_baseline");
  if (!mLib.jpegd_lib_configure_baseline) {
    QIDBG_ERROR("%s:%d] Error linking jpegd_lib_configure_baseline",
      __func__, __LINE__);
    dlclose(mLib.ptr);
    mLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mLib.jpegd_lib_init) =
    dlsym(mLib.ptr, "jpegd_lib_init");
  if (!mLib.jpegd_lib_init) {
    QIDBG_ERROR("%s:%d] Error linking jpegd_lib_init",
      __func__, __LINE__);
    dlclose(mLib.ptr);
    mLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mLib.jpegd_lib_release) =
    dlsym(mLib.ptr, "jpegd_lib_release");
  if (!mLib.jpegd_lib_release) {
    QIDBG_ERROR("%s:%d] Error linking jpegd_lib_release",
      __func__, __LINE__);
    dlclose(mLib.ptr);
    mLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mLib.jpegd_lib_decode) =
    dlsym(mLib.ptr, "jpegd_lib_decode");
  if (!mLib.jpegd_lib_decode) {
    QIDBG_ERROR("%s:%d] Error linking jpegd_lib_decode",
      __func__, __LINE__);
    dlclose(mLib.ptr);
    mLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mLib.jpegd_lib_input_buf_cfg) =
    dlsym(mLib.ptr, "jpegd_lib_input_buf_cfg");
  if (!mLib.jpegd_lib_input_buf_cfg) {
    QIDBG_ERROR("%s:%d] Error linking jpegd_lib_input_buf_cfg",
      __func__, __LINE__);
    dlclose(mLib.ptr);
    mLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mLib.jpegd_lib_output_buf_get) =
    dlsym(mLib.ptr, "jpegd_lib_output_buf_get");
  if (!mLib.jpegd_lib_output_buf_get) {
    QIDBG_ERROR("%s:%d] Error linking jpegd_lib_output_buf_get",
      __func__, __LINE__);
    dlclose(mLib.ptr);
    mLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mLib.jpegd_lib_input_buf_get) =
    dlsym(mLib.ptr, "jpegd_lib_input_buf_get");
  if (!mLib.jpegd_lib_input_buf_get) {
    QIDBG_ERROR("%s:%d] Error linking jpegd_lib_input_buf_get",
      __func__, __LINE__);
    dlclose(mLib.ptr);
    mLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mLib.jpegd_lib_output_buf_cfg) =
    dlsym(mLib.ptr, "jpegd_lib_output_buf_cfg");
  if (!mLib.jpegd_lib_output_buf_cfg) {
    QIDBG_ERROR("%s:%d] Error linking jpegd_lib_output_buf_cfg",
      __func__, __LINE__);
    dlclose(mLib.ptr);
    mLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **)&(mLib.jpegd_lib_wait_done) =
    dlsym(mLib.ptr, "jpegd_lib_wait_done");
  if (!mLib.jpegd_lib_wait_done) {
    QIDBG_ERROR("%s:%d] Error linking jpegd_lib_wait_done",
      __func__, __LINE__);
    dlclose(mLib.ptr);
    mLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }
  QIDBG_HIGH("%s:%d] JpegD library loaded successfully", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: UnLoad
 *
 * Description: UnLoads the decoder userspace driver
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QImageHW10Decoder::UnLoad()
{
  int rc = 0;
  QIDBG_HIGH("%s:%d] ptr %p", __func__, __LINE__, mLib.ptr);
  if (mLib.ptr) {
    rc = dlclose(mLib.ptr);
    if (rc < 0)
      QIDBG_HIGH("%s:%d] error %s", __func__, __LINE__, dlerror());
    mLib.ptr = NULL;
  }
}
