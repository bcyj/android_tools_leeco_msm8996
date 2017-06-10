/*****************************************************************************
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
 * Qualcomm Technologies Proprietary and Confidential.                        *
 *****************************************************************************/

#include "QImageGeminiEncoder.h"

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

#define MCU_SIZE 4

#define FLOOR_PAD_TO_16(a)  ((a>>4)<<4)
#define MAX_JPEGE_OUT_SIZE  FLOOR_PAD_TO_16(0x4000000)
#define MAX_GEMINI_WIDTH 4736
#define MAX_GEMINI_HEIGHT 3552

#define ENCODE_TIMEOUT 10000

/*===========================================================================
 * Function: QImageGeminiEncoder
 *
 * Description: QImageGeminiEncoder constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageGeminiEncoder::QImageGeminiEncoder()
{
  QI_MUTEX_INIT(&mMutex);
  QI_COND_INIT(&mCond);
  mEncodeParams = NULL;
  mInputImage = NULL;
  mOutputImage = NULL;
  mState = ESTATE_IDLE;
  mEngine = NULL;
  mIONInput = NULL;
  mIONOutput = NULL;
  mOutputDone = false;
  mError = QI_SUCCESS;
  QI_MUTEX_INIT(&mLibMutex);
  QI_COND_INIT(&mLibCond);
  memset(&mEncodeCfg,  0x0, sizeof(gemini_cmd_jpeg_encode_cfg));
  memset(&mOutputCfg, 0x0, sizeof(gemini_cmd_output_cfg));
  memset(&mInputCfg, 0x0, sizeof(gemini_cmd_input_cfg));
  memset(&mOperationCfg, 0x0, sizeof(gemini_cmd_operation_cfg));
  mEncodeCfg.quantTblY = NULL;
  mEncodeCfg.quantTblChroma = NULL;
  for (int i = 0; i < QI_MAX_PLANES; i++) {
    mInAddr[i] = NULL;
  }
  mRotYOffset = 0;
  mRotCrCbOffset = 0;
}


/*===========================================================================
 * Function: ~QImageGeminiEncoder
 *
 * Description: QImageGeminiEncoder destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageGeminiEncoder::~QImageGeminiEncoder()
{
  Stop();

  int lrc = 0;

  /*join the encoder thread*/
  mEncodeThread.JoinThread();

  if (mEngine) {
    gemini_lib_release(mEngine);
    if (lrc < 0)
      QIDBG_ERROR("%s:%d: release failed %d", __func__, __LINE__, lrc);
    mEngine = NULL;
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
  QI_MUTEX_DESTROY(&mLibMutex);
  QI_COND_DESTROY(&mLibCond);

  if (mEncodeCfg.quantTblY) {
    free(mEncodeCfg.quantTblY);
  }
  if (mEncodeCfg.quantTblChroma) {
    free(mEncodeCfg.quantTblChroma);
  }
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
int QImageGeminiEncoder::addInputImage(QImage &aImage)
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
int QImageGeminiEncoder::addObserver(QImageEncoderObserver &aObserver)
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
int QImageGeminiEncoder::addOutputImage(QImage &aImage)
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
int QImageGeminiEncoder::setEncodeParams(QIEncodeParams &aParams)
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
int QImageGeminiEncoder::SetOutputMode(QIOutputMode aMode)
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
int QImageGeminiEncoder::EventHandler(int aEvent)
{
  QIDBG_MED("%s:%d] type %d", __func__, __LINE__, aEvent);


  QI_LOCK(&mLibMutex);
  if (GEMINI_EVT_FRAMEDONE  == aEvent) {
    QIDBG_HIGH("%s %d: GEMINI_EVT_FRAMEDONE", __func__, __LINE__);
  } else {
    QIDBG_HIGH("%s %d: GEMINI_EVT_ERROR", __func__, __LINE__);
    mError = QI_ERR_GENERAL;
    QI_SIGNAL(&mLibCond);
  }

  QI_UNLOCK(&mLibMutex);


  return QI_SUCCESS;
}


/*===========================================================================
 * Function: gemini_event_handler
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
int gemini_event_handler(void *p_user, gemini_evt *aJpegCtrlCmd,
    int aEvent)
{
  QImageGeminiEncoder *lEngine = (QImageGeminiEncoder *)p_user;
  if (lEngine) {
    return lEngine->EventHandler(aEvent);
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
int QImageGeminiEncoder::InputHandler(gemini_buf *aBuffer)
{
  QIDBG_MED("%s:%d] type %d", __func__, __LINE__, aBuffer->type);

  QI_LOCK(&mLibMutex);
  if ((GEMINI_EVT_RESET == aBuffer->type) ||
      (GEMINI_EVT_FRAMEDONE == aBuffer->type)) {
  } else {
    mError = QI_ERR_GENERAL;
    QI_SIGNAL(&mLibCond);
  }
  QI_UNLOCK(&mLibMutex);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: gemini_input_handler
 *
 * Description: Static function for input handler
 *
 * Input parameters:
 *   p_user - HW object pointer
 *   aBuffer - gemini_buf Buffer
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int gemini_input_handler(void *p_user, gemini_buf *aBuffer)
{
  QImageGeminiEncoder *lEngine = (QImageGeminiEncoder *)p_user;
  if (lEngine) {
    return lEngine->InputHandler(aBuffer);
  }
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
int QImageGeminiEncoder::OutputHandler(gemini_buf *aoutBuffer)
{
  QIDBG_MED("%s:%d] type %d", __func__, __LINE__, aoutBuffer->type);

  QI_LOCK(&mLibMutex);
  if (NULL == aoutBuffer) {
    mOutputDone = false;
    mError = QI_ERR_GENERAL;
    QI_SIGNAL(&mLibCond);
    QI_UNLOCK(&mLibMutex);
    return QI_SUCCESS;
  }

  QIDBG_HIGH("%s %d:Frame done length %d", __func__, __LINE__,
      aoutBuffer->framedone_len);
  memcpy(mOutputImage->BaseAddr(), aoutBuffer->vaddr,
      aoutBuffer->framedone_len);
  mOutputImage->SetFilledLen(aoutBuffer->framedone_len);
  mOutputDone = true;
  QI_SIGNAL(&mLibCond);
  QI_UNLOCK(&mLibMutex);

  return QI_SUCCESS;
}

/*===========================================================================
 * Function: gemini_output_handler
 *
 * Description: Static function for output handler
 *
 * Input parameters:
 *   p_user - HW object pointer
 *   aBuffer - gemini_buf Buffer
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int gemini_output_handler(void *p_user,
    gemini_buf *aBuffer)
{
  QImageGeminiEncoder *lEngine = (QImageGeminiEncoder *)p_user;
  if (lEngine) {
    return lEngine->OutputHandler(aBuffer);
  }
  return QI_SUCCESS;
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
QImageEncoderInterface* QImageGeminiEncoder::New(QIEncodeParams &aParams)
{
  QImageGeminiEncoder* lEncoder = new QImageGeminiEncoder();
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
bool QImageGeminiEncoder::IsAvailable(QIEncodeParams& aParams)
{
  int lrc = QI_SUCCESS;
  int lGmnFd = -1;
  QISize lOutSize = aParams.OutputSize();
  int rot = aParams.Rotation();

  QI_LOCK(&mMutex);

  /* check if rotation is enabled */
  if ((aParams.Rotation() != 0) && (aParams.Rotation() != 90) &&
      (aParams.Rotation() != 180) && (aParams.Rotation() != 270)) {
    QIDBG_ERROR("%s:%d: Invalid rotation = %d", __func__, __LINE__,rot);
    QI_UNLOCK(&mMutex);
    return false;
  }

  /* Check if cropping is enabled*/
  if(!aParams.Crop().isZero()){
    QIDBG_ERROR("%s:%d: Cropping not supported", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return false;
  }

  /* Check if scaling is enabled*/
  if (!lOutSize.IsZero()) {
    if ((lOutSize.Width() > aParams.InputSize().Width()) ||
        (lOutSize.Height() > aParams.InputSize().Height()) ||
        (lOutSize.Width() < aParams.InputSize().Width()) ||
        (lOutSize.Height() < aParams.InputSize().Height())) {
      QIDBG_ERROR("%s:%d: Scaling not supported", __func__, __LINE__);
      QI_UNLOCK(&mMutex);
      return false;
    }
  }

  /* Check if input image size exceeds the MAX input size */
  if ((aParams.InputSize().Height() > MAX_GEMINI_HEIGHT) ||
      (aParams.InputSize().Width() > MAX_GEMINI_WIDTH)) {
    QIDBG_ERROR("%s:%d: Input size (%d, %d) exceeds max supported"
        "input size of (%d, %d)", __func__, __LINE__,
        aParams.InputSize().Width(), aParams.InputSize().Height(),
        MAX_GEMINI_WIDTH, MAX_GEMINI_HEIGHT);
    QI_UNLOCK(&mMutex);
    return false;
  }


  QIDBG_ERROR("%s %d: BEFORE GMN_LIB INIT",
      __func__, __LINE__);
  /* Initialize the Gemini userspace driver*/
  lGmnFd = gemini_lib_init(&mEngine, this,
      gemini_event_handler, gemini_output_handler,
      gemini_input_handler);
  QIDBG_ERROR("%s %d: AFTER GMN_LIB INIT",__func__, __LINE__);
  if (lGmnFd < 0 || !mEngine) {
    QIDBG_ERROR("%s %d: Failed to Initialize the Gemini HW lib",
        __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return false;
  }
  QI_UNLOCK(&mMutex);
  return true;
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
int QImageGeminiEncoder::ConfigureInputParams()
{
  int lrc = QI_SUCCESS;
  QISubsampling lSubsampling = mInputImage->SubSampling();
  QIFormat lFormat = mInputImage->Format();
  uint32_t lPaddedHeight = CEILING16(mEncodeParams->InputSize().Height());
  uint32_t lPaddedWidth = CEILING16(mEncodeParams->InputSize().Width());

  uint32_t lLumaSize= mInputImage->getPlane(QIPlane::PLANE_CB_CR)->PhyOffset();
  uint32_t lChromaSize = mInputImage->getPlane(QIPlane::PLANE_CB_CR)->
      ActualSize().Length();
  uint32_t lOffset = lLumaSize - lChromaSize;
  uint32_t lYLen = lPaddedHeight * lPaddedWidth;
  QIDBG_ERROR("%s luma_size = %d chroma_size = %d offset = %d y_len =%d\n",
      __func__,lLumaSize,lChromaSize,lOffset,lYLen);

  mPadding = lYLen - lOffset;

  QIDBG_ERROR("%s: Additional paddding = %d\n",__func__,mPadding);

  /* Set the Color Format */
  switch (lSubsampling) {
  case QI_H2V2: {
    mInputCfg.inputFormat = GEMINI_INPUT_H2V2;
    break;
  }
  case QI_H2V1: {
    mInputCfg.inputFormat = GEMINI_INPUT_H2V1;
    break;
  }
  case QI_H1V2: {
    mInputCfg.inputFormat = GEMINI_INPUT_H1V2;
    break;
  }
  default: {
    QIDBG_ERROR("%s %d: Subsampling not supported = %d", __func__, __LINE__,
        lSubsampling);
    return QI_ERR_INVALID_INPUT;
  }
  }

  /* Set the CBCR ordering */
  switch (lFormat) {
  case QI_YCBCR_SP: {
    mInputCfg.input_cbcr_order = 0;
    break;
  }
  case QI_YCRCB_SP: {
    mInputCfg.input_cbcr_order = 1;
    break;
  }
  case QI_IYUV:
  case QI_YUV2:
  case QI_MONOCHROME:
  default: {
    QIDBG_ERROR("%s %d: Format not supported = %d", __func__, __LINE__,
        lFormat);
    return QI_ERR_INVALID_INPUT;
  }

  } /*end of switch*/

  /*Set the MCUs*/
  mInputCfg.frame_height_mcus = lPaddedHeight >> MCU_SIZE;
  mInputCfg.frame_width_mcus = lPaddedWidth >> MCU_SIZE;

  /*Set the Fetch Engine burst length*/
  mInputCfg.byte_ordering = 0;

  /*Set the fetch Enfine Burst length*/
  mInputCfg.fe_burst_length = GEMINI_BURST_LENGTH4;

  QIDBG_MED("%s %d: Width = %d, padded Height = %d, cbcr order = %d, format = %d,"
      "frame_height_mcus = %d, frame_width_mcus = %d, fe_burst_length = %d,"
      "byte_ordering = %d", __func__, __LINE__, lPaddedWidth, lPaddedHeight,
      mInputCfg.input_cbcr_order, mInputCfg.inputFormat,
      mInputCfg.frame_height_mcus, mInputCfg.frame_width_mcus,
      mInputCfg.fe_burst_length, mInputCfg.byte_ordering);

  return lrc;

}

/*===========================================================================
 * Function: ConfigureOutputParams
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
int QImageGeminiEncoder::ConfigureOutputParams()
{
  int lrc = QI_SUCCESS;
  mOutputCfg.we_burst_length = GEMINI_BURST_LENGTH4;
  mOutputCfg.byte_ordering = 0;

  QIDBG_HIGH("%s:%d: Requested rotation %d", __func__, __LINE__,
      mEncodeParams->Rotation());

  switch (mEncodeParams->Rotation()) {
  case 0: {
    mOperationCfg.useMode = GEMINI_MODE_OFFLINE_ENCODE;
    mOperationCfg.rotationDegree = GEMINI_ROTATE_NONE;
    break;
  }
  case 90: {
    mOperationCfg.useMode = MSM_GEMINI_MODE_OFFLINE_ROTATION;
    mOperationCfg.rotationDegree = GEMINI_ROTATE_270;
    break;
  }
  case 180: {
    mOperationCfg.useMode = MSM_GEMINI_MODE_OFFLINE_ROTATION;
    mOperationCfg.rotationDegree = GEMINI_ROTATE_180;
    break;
  }
  case 270: {
    mOperationCfg.useMode = MSM_GEMINI_MODE_OFFLINE_ROTATION;
    mOperationCfg.rotationDegree = GEMINI_ROTATE_90;
    break;
  }
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
int QImageGeminiEncoder::ConfigureTables()
{
  int lrc = QI_SUCCESS;
  QIQuantTable *lQTable = NULL;
  QIHuffTable *lHuffTable = NULL;
  uint16_t lQtableArray[64];

  mEncodeCfg.restartInterval = 0;
  mEncodeCfg.bFSCEnable = 0;

  /*Set Quantization Table */
  lQTable = mEncodeParams->QuantTable(QIQuantTable::QTABLE_LUMA);
  if (lQTable == NULL) {
    QIDBG_ERROR("%s:%d: failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }
  mEncodeCfg.quantTblY = (gemini_quantTable *)malloc(sizeof(gemini_quantTable));
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
  mEncodeCfg.quantTblChroma = (gemini_quantTable *)
            malloc(sizeof(gemini_quantTable));
  if (mEncodeCfg.quantTblChroma == NULL) {
    QIDBG_ERROR("%s:%d: failed to allocate memory", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }
  for (int i =0; i< QUANT_SIZE; i++) {
    mEncodeCfg.quantTblChroma->qtbl[i] = *(lQTable->Table()+i);
  }

  /*Set Huffman Table*/
  mEncodeCfg.bCustomHuffmanTbl = 0;
  mEncodeCfg.huffmanTblCbcrAcPtr = NULL;
  mEncodeCfg.huffmanTblCbcrDcPtr = NULL;
  mEncodeCfg.huffmanTblYAcPtr = NULL;
  mEncodeCfg.huffmanTblYDcPtr = NULL;

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
int QImageGeminiEncoder::SetHWConfig()
{
  int lrc = QI_SUCCESS;

  lrc = ConfigureInputParams();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring Input params", __func__, __LINE__);
    return lrc;
  }

  lrc = ConfigureTables();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring Quantization & Huff tables",
        __func__, __LINE__);
    return lrc;
  }

  lrc = ConfigureOutputParams();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring Output params",
        __func__, __LINE__);
    return lrc;
  }

  lrc = gemini_lib_hw_config(mEngine, &mInputCfg, &mOutputCfg, &mEncodeCfg,
      &mOperationCfg);
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
int QImageGeminiEncoder::populatePlaneAddr()
{
  QIPlane *lPlane = NULL;

  if (SP_CNT == mInputImage->PlaneCount()) {
    lPlane = mInputImage->getPlane(QIPlane::PLANE_CB_CR);
    if (NULL == lPlane) {
      QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    mInAddr[IC] = lPlane->ActualAddr();
    mInSize[IC] = lPlane->ActualSize();
  } else {
    QIDBG_ERROR("%s:%d] Invalid input. Only pseudoplanar input is supported",
        __func__, __LINE__);
    return QI_ERR_INVALID_INPUT;
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
int QImageGeminiEncoder::ConfigureInputBuffer()
{
  uint32_t lBufferlength = 0, lOffset = 0;
  int lrc = QI_SUCCESS;

  gemini_buf lGmnBuf;
  memset(&lGmnBuf, 0x0, sizeof(gemini_buf));

  uint32_t lPaddLines = CEILING16(mEncodeParams->InputSize().Height())-
      mEncodeParams->InputSize().Height();

  lGmnBuf.type = 0x0;
  if (mInputImage->PlaneCount() == 2) {
    lBufferlength = mInSize[IY].Length() + mInSize[IC].Length();
    /* pseudo-planar input */
    lGmnBuf.y_len = mInSize[IY].Length();
    if (!mInputImage->getPlane(QIPlane::PLANE_CB_CR) ||
        !mInputImage->getPlane(QIPlane::PLANE_Y)) {
      QIDBG_MED("%s:%d] Error invalid planes", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    lBufferlength = mInputImage->getPlane(QIPlane::PLANE_CB_CR)->PhyOffset() +
        mInSize[IC].Length();
    lGmnBuf.y_len = mInputImage->getPlane(QIPlane::PLANE_CB_CR)->PhyOffset();
    lGmnBuf.y_off = mInputImage->getPlane(QIPlane::PLANE_Y)->Offset();
    lGmnBuf.cbcr_off = 0;
    lGmnBuf.cbcr_len = mInSize[IC].Length();
    if ((mOperationCfg.rotationDegree == GEMINI_ROTATE_270) ||
        (mOperationCfg.rotationDegree == GEMINI_ROTATE_180)){
      uint32_t lPaddOff = CEILING16(mEncodeParams->InputSize().Width()) *
          lPaddLines;
      if (lGmnBuf.y_off >= lPaddOff) {
        lGmnBuf.y_off -= lPaddOff;
        lGmnBuf.cbcr_off += lPaddOff/2;
      }
    }
  } else {
    QIDBG_MED("%s:%d] Error invalid number of planes = %d", __func__, __LINE__,
        mInputImage->PlaneCount());
    return QI_ERR_INVALID_INPUT;
  }

  /*If the input buffer is not alread an ION buffer, assign the vaddr*/
  if (mInputImage->getPlane(QIPlane::PLANE_Y)->Fd() > 0) {
    lGmnBuf.vaddr = mInputImage->getPlane(QIPlane::PLANE_Y)->Addr();
    lGmnBuf.fd = mInputImage->getPlane(QIPlane::PLANE_Y)->Fd();
  } else {
    QIDBG_MED("%s:%d] lLength %d ", __func__, __LINE__, lBufferlength);

    mIONInput = QIONBuffer::New(lBufferlength, false);
    if (NULL == mIONInput) {
      QIDBG_ERROR("%s:%d] cannot alloc input buffers", __func__, __LINE__);
      return QI_ERR_NO_MEMORY;
    }
    memcpy(mIONInput->Addr(), mInAddr[IY], mInSize[IY].Length());
    memcpy((mIONInput->Addr() + mInSize[IY].Length()), mInAddr[IC],
        mInSize[IC].Length());
    mIONInput->SetFilledLen(lBufferlength);

    QIDBG_ERROR("%s %d: I/p plane vaddress = %p", __func__, __LINE__,
        mIONInput->Addr());
    lGmnBuf.vaddr = mIONInput->Addr();
    lGmnBuf.fd = mIONInput->Fd();
  }
  lGmnBuf.num_of_mcu_rows = CEILING16(mEncodeParams->InputSize().Height()) >>
      MCU_SIZE;
  lGmnBuf.offset = 0;

  lrc =   gemini_lib_input_buf_enq(mEngine, &lGmnBuf);
  return lrc;
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
int QImageGeminiEncoder::ConfigureOutputBuffer()
{
  int lrc = QI_SUCCESS;
  uint32_t lBufferlength = 0, lOffset = 0;

  gemini_buf lGmnOutBuf;
  memset(&lGmnOutBuf, 0x0, sizeof(gemini_buf));

  lBufferlength = mOutputImage->Length();

  mIONOutput = QIONBuffer::New(lBufferlength, false);
  if (NULL == mIONOutput) {
    QIDBG_ERROR("%s:%d] cannot alloc input buffers", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }
  mIONOutput->SetFilledLen(lBufferlength);
  lGmnOutBuf.fd = mIONOutput->Fd();
  lGmnOutBuf.vaddr = mIONOutput->Addr();
  lGmnOutBuf.y_len = lBufferlength;

  lrc =   gemini_lib_output_buf_enq(mEngine, &lGmnOutBuf);
  if (lrc) {
    delete mIONOutput;
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
int QImageGeminiEncoder::Configure()
{
  int lrc = QI_SUCCESS;

  lrc = SetHWConfig();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring HW params", __func__, __LINE__);
    return lrc;
  }
  lrc = populatePlaneAddr();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring plane address", __func__, __LINE__);
    return lrc;
  }
  lrc = ConfigureInputBuffer();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring input buffer", __func__, __LINE__);
    return lrc;
  }
  lrc = ConfigureOutputBuffer();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring output buffer", __func__, __LINE__);
    return lrc;
  }
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
int QImageGeminiEncoder::Stop()
{
  int lrc = QI_SUCCESS;

  QIDBG_MED("%s:%d] mState %d", __func__, __LINE__, mState);

  QI_LOCK(&(mMutex));

  if (mState != ESTATE_ACTIVE) {
    QI_UNLOCK(&(mMutex));
    return QI_SUCCESS;
  }

  mState = ESTATE_STOP_REQUESTED;

  QI_LOCK(&(mLibMutex));
  lrc = gemini_lib_stop (mEngine, 0);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d: Failed to stop the Gemini encoder", __func__, __LINE__);
  }
  QI_SIGNAL(&(mLibCond));
  QI_UNLOCK(&(mLibMutex));

  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  QI_WAIT(&(mCond), &(mMutex));
  QIDBG_MED("%s:%d] ", __func__, __LINE__);

  QI_UNLOCK(&(mMutex));

  return lrc;
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
void QImageGeminiEncoder::Execute()
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
void QImageGeminiEncoder::Encode()
{
  int lrc = QI_SUCCESS;

  QIDBG_ERROR("%s:%d] STARTING GEMINI ENCODE ", __func__, __LINE__);

  lrc = gemini_lib_encode (mEngine);
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] Error Encoding ", __func__, __LINE__);
    goto encode_error;
  }

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
    lrc = gemini_lib_stop(mEngine, 0);
    if (lrc < 0) {
        QIDBG_ERROR("%s:%d: Failed to stop the Gemini encoder", __func__, __LINE__);
    }
  } else {
    QI_SIGNAL(&mCond);
  }
  mState = ESTATE_IDLE;
  QI_UNLOCK(&mMutex);

  for (uint32_t k = 0; k < mObserverCnt; k++)
    mObserver[k]->EncodeError(QImageEncoderObserver::ERROR_GENERAL);
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
int QImageGeminiEncoder::Start()
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
