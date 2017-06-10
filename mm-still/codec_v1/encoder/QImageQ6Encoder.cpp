/*****************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QImageQ6Encoder.h"
#include "QIParams.h"

extern "C" {

#include "jpeg_buffer_private.h"
#include "jpege_engine_sw.h"
#include "jpege.h"
#include "jpegerr.h"
#include "remote.h"


#include <unistd.h>
#include <string.h>
#include <pthread.h>

//#define ENABLE_Q6_CONFIG_LOGS
//#define DUMP_JPEG_BITSTREAM


} // end extern "C"

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* Function prototypes of helper functions */
static int jpege_engine_sw_configure(jpege_q6_enc_cfg_target_t*,
  jpege_img_data_t*,QImage*);

#pragma weak  remote_register_buf

static void register_buf(void* buf, int size, int fd) {
  if(remote_register_buf) {
      remote_register_buf(buf, size, fd);
   }
}

/*===========================================================================
 * Function: IsAvailable
 *
 * Description: Static function to check if encoder params are valid
 *
 * Input parameters:
 *   aEncodeParams - encoder parameters for validation
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 *==========================================================================*/
bool QImageQ6Encoder::IsAvailable(QIEncodeParams& aEncodeParams)
{
  int lrc = QI_SUCCESS;
  bool lUpScale = false, lScaleEnabled = false;
  QISize lInputSize = aEncodeParams.InputSize();
  jpege_q6_enc_cfg_target_t *pQ6cfg =&engine.q6_enc_target_cfg;

  /* Check if cropping is enabled*/
  if(!aEncodeParams.Crop().isZero()){
    QIDBG_HIGH("%s:%d: Cropping not supported", __func__, __LINE__);
    return false;
  }

  /* Check if scaling is enabled*/
  lScaleEnabled = (aEncodeParams.OutputSize().Width() != lInputSize.Width())
    || (aEncodeParams.OutputSize().Height() != lInputSize.Height());

  if (lScaleEnabled) {
    QIDBG_HIGH("%s:%d] Wont support scaling", __func__, __LINE__);
    return false;
  }
  else
    pQ6cfg->scale_cfg.enable = 0;

  lrc = adsp_jpege_fastrpc_start();
  // If there is a failure in creating adsp jpege, exit
  if (lrc) {
    QIDBG_HIGH("jpege_engine_q6_init: adsp_jpege_fastrpc_start() failed: %d\n", lrc);
    return false;
  }

  lrc = adsp_jpege_init();
  // If there is a failure in initing adsp jpege, exit
  if (lrc) {
    QIDBG_ERROR("%s:%d] adsp_jpege_init() failed %d", __func__, __LINE__,lrc);
    return false;
  }
  return true;
}

/*===========================================================================
 * Function: New
 *
 * Description: 2 phase constructor for QImageQ6Encoder
 *
 * Input parameters:
 *   aParams - encoder params
 *
 * Return values:
 *   encoder interface pointer
 *
 * Notes: none
 *==========================================================================*/
QImageEncoderInterface* QImageQ6Encoder::New(QIEncodeParams &aParams)
{
  QImageQ6Encoder* lEncoder = new QImageQ6Encoder();
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
 * Function: QImageQ6Encoder
 *
 * Description: QImageQ6Encoder constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageQ6Encoder::QImageQ6Encoder()
{
  // Initialize the fields inside the engine structure below
  memset(&mEngine, 0, sizeof(jpege_engine_sw_t));
  memset(&engine, 0, sizeof(jpege_engine_q6_t));
  memset(&mSource, 0, sizeof(jpege_img_data_t));
  QI_MUTEX_INIT(&(mMutex));
  QI_COND_INIT(&(mCond));
  mState = ESTATE_IDLE;
  mEncodeParams = NULL;
  mInputImage = NULL;
  mOutputImage = NULL;
  mMode = ENORMAL_OUTPUT;
  mpEncoderThread = NULL;
  mpOutputThread = NULL;
  mObserverCnt = 0;
}

/*===========================================================================
 * Function: ~QImageQ6Encoder
 *
 * Description: QImageQ6Encoder destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageQ6Encoder::~QImageQ6Encoder()
{
  int lrc = QI_SUCCESS;
  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, mState);
  Stop();

  lrc = adsp_jpege_deinit();
  QIDBG_HIGH("%s:%d] adsp_jpege_deinit ret=%d ", __func__, __LINE__,lrc);

  if (mEngine.p_internal_buf) {
    jpeg_buffer_destroy((jpeg_buffer_t *)&(mEngine.p_internal_buf));
  }
  if (mEngine.jpegeBitStreamState.buffer) {
    JPEG_FREE(mEngine.jpegeBitStreamState.buffer);
    memset(&(mEngine.jpegeBitStreamState), 0, sizeof(bitstream_t));
  }

  if (mSource.p_fragments[0].color.yuv.luma_buf) {
    jpeg_buffer_destroy(&mSource.p_fragments[0].color.yuv.luma_buf);
  }
  if (mSource.p_fragments[0].color.yuv.chroma_buf) {
    jpeg_buffer_destroy(&mSource.p_fragments[0].color.yuv.chroma_buf);
  }

  if (mpOutputThread && (mMode == EPIECEWISE_OUTPUT)) {
    mpOutputThread->JoinThread();
  }

  if (mpEncoderThread) {
    mpEncoderThread->JoinThread();
    delete mpEncoderThread;
    mpEncoderThread = NULL;
  }

  if (mpOutputThread) {
    delete mpOutputThread;
    mpOutputThread = NULL;
  }

  QI_MUTEX_DESTROY(&mMutex);
  QI_COND_DESTROY(&mCond);
  QIDBG_MED("%s:%d] X", __func__, __LINE__);
}

/*===========================================================================
 * Function: SetOutputMode
 *
 * Description: Sets the output mode. Currently piecewise output is not
 *              supported
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
int QImageQ6Encoder::SetOutputMode(QIOutputMode aMode)
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
 * Function: Start
 *
 * Description: Started the software encoder
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
int QImageQ6Encoder::Start()
{
  int lrc = QI_SUCCESS;
  jpege_q6_enc_cfg_target_t *pQ6cfg = &engine.q6_enc_target_cfg;

  QIDBG_MED("%s:%d] E state %d", __func__, __LINE__, mState);
  QI_LOCK(&(mMutex));
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }

  if ((NULL == mInputImage) || (NULL == mOutputImage)
    || (NULL == mEncodeParams) || (mObserverCnt < 1)) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }

  if (((pQ6cfg->rotation == 90) ||  (pQ6cfg->rotation == 270)) &&
      (mInputImage->ActualSize().Height() != mInputImage->Size().Height())) {
    QIDBG_ERROR("%s:%d] padding is not supported !!", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  } else if (mInputImage->ActualSize().Width() != mInputImage->Size().Width()) {
    QIDBG_ERROR("%s:%d] padding is not supported !!", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  // Configure the engine based on the input configuration
  lrc = jpege_engine_sw_configure(pQ6cfg, &mSource,mOutputImage);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }
  mEncodeParams->setRestartInterval(pQ6cfg->restart_interval);

  mEngine.abort_flag = false;
  mEngine.error_flag = false;

  // Create output thread
  if (mMode == EPIECEWISE_OUTPUT) {
    mpOutputThread = new QIQ6OutputThread(*this);
    if (NULL == mpOutputThread) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      QI_UNLOCK(&(mMutex));
      return lrc;
    }

    lrc = mpOutputThread->StartThread(NULL);
    if (lrc < 0) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      QI_UNLOCK(&(mMutex));
      return lrc;
    }
  }

  mpEncoderThread = new QIQ6EncoderThread(*this);
  if (NULL == mpEncoderThread) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return lrc;
  }

  lrc = mpEncoderThread->StartThread(NULL);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return lrc;
  }

  mState = ESTATE_ACTIVE;

  QI_UNLOCK(&(mMutex));
  QIDBG_MED("%s:%d] X", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: PrintTables
 *
 * Description: print the huffman table values
 *
 * Input parameters:
 *   tbl - pointer to huff table
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
static void PrintTables(jpege_q6_huff_table_t *tbl)
{
  int j = 0, i = 0;
  QIDBG_LOW("huff_cfg.p_htables.bits = {");
  for (i=0; i<16; i+=8) {
    QIDBG_LOW("%d %d %d %d %d %d %d %d",
      tbl->bits[i],
      tbl->bits[i+1],
      tbl->bits[i+2],
      tbl->bits[i+3],
      tbl->bits[i+4],
      tbl->bits[i+5],
      tbl->bits[i+6],
      tbl->bits[i+7]);
  }
  QIDBG_LOW(" %d", tbl->bits[16]);
  QIDBG_LOW("}");
  QIDBG_LOW("huff_cfg.p_htables.values = {");
  for (i=0; i<256; i+=8) {
    QIDBG_LOW("%d %d %d %d %d %d %d %d",
      tbl->values[i],
      tbl->values[i+1],
      tbl->values[i+2],
      tbl->values[i+3],
      tbl->values[i+4],
      tbl->values[i+5],
      tbl->values[i+6],
      tbl->values[i+7]);
  }
  QIDBG_LOW("}");
}

/*===========================================================================
 * Function: copyHuffTable
 *
 * Description: convert the huffman table from QIHuffTable object to
 *              jpege_q6_huff_table_t
 *
 * Input parameters:
 *   ap_htable - pointer to jpege_q6_huff_table_t
 *   aType - type of huffman table
 *
 * Return values:
 *   QI_SUCCES
 *   QI_ERR_NOT_FOUND
 *
 * Notes: none
 *==========================================================================*/
int QImageQ6Encoder::copyHuffTable(jpege_q6_huff_table_t *ap_htable,
  QIHuffTable::QHuffTableType aType)
{
  QIHuffTable *lQHuffTbl = mEncodeParams->HuffTable(aType);
  const QIHuffTable::HuffTable *lHuffTbl = NULL;
  if (NULL == lQHuffTbl) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NOT_FOUND;
  }
  lHuffTbl = lQHuffTbl->Table();
  memcpy(ap_htable->bits, lHuffTbl->mBits, HUFF_BITS);
  memcpy(ap_htable->values, lHuffTbl->mValues, HUFF_VALUES);
  QIDBG_LOW("%s:%d] Hufftable %d", __func__, __LINE__, aType);
  //PrintTables(ap_htable);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: setEncodeParams
 *
 * Description: Sets the encoder parameters
 *
 * Input parameters:
 *   aParams - reference to encode parameters
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageQ6Encoder::setEncodeParams(QIEncodeParams &aParams)
{
  int lrc = QI_SUCCESS;
  QIQuantTable *lQTable = NULL;
  jpege_q6_enc_cfg_target_t *pQ6cfg =&engine.q6_enc_target_cfg;
  uint32_t    index;

  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, mState);
  QI_LOCK(&(mMutex));
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }

  mEncodeParams = &aParams;
  pQ6cfg->rotation = mEncodeParams->Rotation();

  if (QI_ERROR(copyHuffTable(&pQ6cfg->chroma_ac_huff_tbl,
    QIHuffTable::HTABLE_CHROMA_AC))) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  if (QI_ERROR(copyHuffTable(&pQ6cfg->chroma_dc_huff_tbl,
    QIHuffTable::HTABLE_CHROMA_DC))) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  if (QI_ERROR(copyHuffTable(&pQ6cfg->luma_ac_huff_tbl,
    QIHuffTable::HTABLE_LUMA_AC))) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  if (QI_ERROR(copyHuffTable(&pQ6cfg->luma_dc_huff_tbl,
    QIHuffTable::HTABLE_LUMA_DC))) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  lQTable = mEncodeParams->QuantTable(QIQuantTable::QTABLE_LUMA);
  if (NULL == lQTable) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }
  jpeg_quant_table_t qtbl = lQTable->Table();
  for(index = 0; index < 64; index++)
  {
    pQ6cfg->qtbl_0[index] = qtbl[index];
  }

  lQTable = mEncodeParams->QuantTable(QIQuantTable::QTABLE_CHROMA);
  if (NULL == lQTable) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  qtbl = lQTable->Table();

  for(index = 0; index < 64; index++)
  {
    pQ6cfg->qtbl_1[index] = qtbl[index];
  }

  QI_UNLOCK(&mMutex);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: ConfigureDimensions
 *
 * Description: This function is used configure the dimension
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *   None
 *
 * Notes: none
 *==========================================================================*/
void QImageQ6Encoder::ConfigureDimensions()
{
  uint32_t lPadWidth, lWidth;

  lPadWidth = mInputImage->Size().Width();
  lWidth = mInputImage->ActualSize().Width();

  QIDBG_HIGH("%s:%d] dim pad %dx%d orig %dx%d", __func__, __LINE__,
    mInputImage->Size().Width(),
    mInputImage->Size().Height(),
    mInputImage->ActualSize().Width(),
    mInputImage->ActualSize().Height());

  mSource.width = lPadWidth;
  mSource.height = mInputImage->ActualSize().Height();
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
int QImageQ6Encoder::addInputImage(QImage &aImage)
{
  int lrc = QI_SUCCESS;

  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, mState);
  QI_LOCK(&(mMutex));
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }

  mInputImage = &aImage;

  ConfigureDimensions();

  if (mInputImage->SubSampling() == QI_H2V2) {
    mSource.color_format = (mInputImage->Format() == QI_YCBCR_SP) ?
      YCBCRLP_H2V2 : YCRCBLP_H2V2;
  } else if (mInputImage->SubSampling() == QI_H2V1) {
    mSource.color_format = (mInputImage->Format() == QI_YCBCR_SP) ?
      YCBCRLP_H2V1 : YCRCBLP_H2V1;
  } else if (mInputImage->SubSampling() == QI_H1V2) {
    mSource.color_format = (mInputImage->Format() == QI_YCBCR_SP) ?
      YCBCRLP_H1V2 : YCRCBLP_H1V2;
  } else {
    mSource.color_format = (mInputImage->Format() == QI_YCBCR_SP) ?
      YCBCRLP_H1V1 : YCRCBLP_H1V1;
  }
  mSource.fragment_cnt = 1;
  QI_ERROR_RET_UNLOCK(jpeg_buffer_init(
    &mSource.p_fragments[0].color.yuv.luma_buf),
    &mMutex);
  QI_ERROR_RET_UNLOCK(jpeg_buffer_init(
    &mSource.p_fragments[0].color.yuv.chroma_buf),
    &mMutex);

  jpeg_buffer_reset(mSource.p_fragments[0].color.yuv.luma_buf);
  jpeg_buffer_reset(mSource.p_fragments[0].color.yuv.chroma_buf);

  QIPlane *lPlane = mInputImage->getPlane(QIPlane::PLANE_Y);
  if (NULL == lPlane) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  QIDBG_MED("%s:%d] Y addr %p len %d fd %d", __func__, __LINE__,
    lPlane->ActualAddr(), lPlane->Length(), lPlane->Fd());
  QI_ERROR_RET_UNLOCK(jpeg_buffer_use_external_buffer(
    mSource.p_fragments[0].color.yuv.luma_buf,
    lPlane->ActualAddr(),
    lPlane->Length(),
    lPlane->Fd()),
    &mMutex);
  jpeg_buffer_set_actual_size(mSource.p_fragments[0].color.yuv.luma_buf,
    lPlane->ActualSize().Length());

  lPlane = mInputImage->getPlane(QIPlane::PLANE_CB_CR);
  if (NULL == lPlane) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  QIDBG_MED("%s:%d] CbCr addr %p len %d fd %d", __func__, __LINE__,
    lPlane->ActualAddr(), lPlane->Length(), lPlane->Fd());
  QI_ERROR_RET_UNLOCK(jpeg_buffer_use_external_buffer(
    mSource.p_fragments[0].color.yuv.chroma_buf,
    lPlane->ActualAddr(),
    lPlane->Length(),
    lPlane->Fd()),
    &mMutex);

  jpeg_buffer_set_actual_size(mSource.p_fragments[0].color.yuv.chroma_buf,
    lPlane->ActualSize().Length());

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
int QImageQ6Encoder::addOutputImage(QImage &aImage)
{
  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, mState);
  QI_LOCK(&(mMutex));
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }
  mOutputImage = &aImage;
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
int QImageQ6Encoder::addObserver(QImageEncoderObserver &aObserver)
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
 * Function: Stop
 *
 * Description: Stops the encoder
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QImageQ6Encoder::Stop()
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
  lrc = adsp_jpege_deinit();
  QIDBG_HIGH("%s:%d] adsp_jpege_deinit ret=%d ", __func__, __LINE__,lrc);

  QI_UNLOCK(&(mMutex));
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: jpege_engine_sw_configure
 *
 * Description: Static function to configure the encoder
 *
 * Input parameters:
 *   p_engine - pointer to engine
 *   p_config - pointer to image config
 *   p_source - pointer to image source
 *
 * Return values:
 *   jpeg error values
 *
 * Notes: none
 *==========================================================================*/
static int jpege_engine_sw_configure(jpege_q6_enc_cfg_target_t *pQ6cfg,
  jpege_img_data_t *p_source,
  QImage *pOutputImage)
{
  int rc;

  uint32_t              luma_width, luma_height, chroma_width, chroma_height;
  uint32_t              rotated_hori_MCU_Count, index;
  jpeg_subsampling_t    subsampling;
  uint32_t              MCU_width, MCU_height, luma_blocks;

  pQ6cfg->base_restart_marker = 0;

  pQ6cfg->input_width  = p_source->width;
  pQ6cfg->input_height = p_source->height;
  pQ6cfg->input_stride = p_source->width;
  pQ6cfg->color_format = (jpege_q6_color_format_t)p_source->color_format;

  pQ6cfg->output_buffer_length = pOutputImage->Length();

  luma_width = p_source->width;
  luma_height = p_source->height;

  subsampling = (jpeg_subsampling_t)((uint8_t)p_source->color_format / 2);
  switch (subsampling)
  {
    case JPEG_H1V1:
    {
      chroma_width = luma_width;
      chroma_height = luma_height;
      MCU_width = 8;
      MCU_height = 8;
      luma_blocks = H1V1_NUM_LUMA_BLOCKS;
      break;
    }
    case JPEG_H1V2:
    {
      chroma_width = luma_width;
      chroma_height = (luma_height + 1) >> 1; // odd luma support
      MCU_width = 8;
      MCU_height = 16;
      luma_blocks = H1V2_NUM_LUMA_BLOCKS;
      break;
    }
    case JPEG_H2V1:
    {
      chroma_width = (luma_width + 1) >> 1; // odd luma support
      chroma_height = luma_height;
      MCU_width = 16;
      MCU_height = 8;
      luma_blocks = H2V1_NUM_LUMA_BLOCKS;
      break;
    }
    case JPEG_H2V2:
    {
      chroma_width = (luma_width + 1) >> 1; // odd luma support
      chroma_height = (luma_height + 1) >> 1; // odd luma support
      MCU_width = 16;
      MCU_height = 16;
      luma_blocks = H2V2_NUM_LUMA_BLOCKS;
      break;
    }
    default:
      QIDBG_ERROR("%s:%d] invalid jpeg subsampling: %d", __func__, __LINE__,subsampling);
      return JPEGERR_EBADPARM;
  }

  /* restart marker interval calculation for Q6 Encoder */
  {
    uint32_t restart_marker = 0;
    if ( (pQ6cfg->rotation == 90) ||  (pQ6cfg->rotation == 270))
      restart_marker = (luma_height + MCU_width -1)/MCU_width;
    else
      restart_marker = (luma_width + MCU_width -1)/MCU_width;

    pQ6cfg->restart_interval = restart_marker;

    QIDBG_HIGH("%s:%d] rotation = %d, restart_marker interval = %d", __func__, __LINE__,
    (unsigned int)pQ6cfg->rotation,restart_marker);
  }

  rotated_hori_MCU_Count  = (chroma_width+ 7) >> 3;  // divide by 8

  pQ6cfg->output_start_mcu_index = 0;

  pQ6cfg->output_MCUs = ((uint32_t)(luma_height + MCU_height - 1)/ MCU_height) *
              rotated_hori_MCU_Count;

  return JPEGERR_SUCCESS;
}

/*===========================================================================
 * Function: QIQ6OutputThread
 *
 * Description: Output thread constructor
 *
 * Input parameters:
 *   aEncoder - reference to encoder object
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIQ6OutputThread::QIQ6OutputThread(QImageQ6Encoder &aEncoder)
: mEncoder(aEncoder)
{
}

/*===========================================================================
 * Function: run
 *
 * Description: Main thread loop for output thread
 *
 * Input parameters:
 *   aData - reference to thread object
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIQ6OutputThread::run(QThreadObject *aData)
{
  /* TODO: for piecewise output*/
  QIDBG_ERROR("%s:%d] ", __func__, __LINE__);
}

/*===========================================================================
 * Function: QIQ6EncoderThread
 *
 * Description: QIQ6EncoderThread constructor
 *
 * Input parameters:
 *   aData - reference to thread object
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIQ6EncoderThread::QIQ6EncoderThread(QImageQ6Encoder &aEncoder)
: mEncoder(aEncoder)
{
}

/*===========================================================================
 * Function: run
 *
 * Description: Main thread loop for encoder thread
 *
 * Input parameters:
 *   aData - reference to thread object
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIQ6EncoderThread::run(QThreadObject *aData)
{
  mEncoder.Encode();
}

/*===========================================================================
 * Function: Encode
 *
 * Description: Encode main function
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QImageQ6Encoder::Encode()
{
  int lrc = QI_SUCCESS;
  jpege_q6_enc_cfg_target_t *pQ6cfg =&engine.q6_enc_target_cfg;
  uint32_t                 jpeg_output_size;

  uint8_t  *input_buffer_chroma, *input_buffer_luma;
  uint8_t  *p_output_buffer;

  QI_LOCK(&mMutex);

#ifdef ENABLE_Q6_CONFIG_LOGS

  int index;

  for(index =0; index < HUFF_BITS ;index++) {
    QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->chroma_dc_huff_tbl.bits[%d] = %d\n",
        index,pQ6cfg->chroma_dc_huff_tbl.bits[index]);
  }

  for(index =0; index < HUFF_VALUES ;index++) {
    QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->chroma_dc_huff_tbl.values[%d] = %d\n",
        index,pQ6cfg->chroma_dc_huff_tbl.values[index]);
  }

  for(index =0; index < HUFF_BITS ;index++) {
    QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->chroma_ac_huff_tbl.bits[%d] = %d\n",
        index,pQ6cfg->chroma_ac_huff_tbl.bits[index]);
  }

  for(index =0; index < HUFF_VALUES ;index++) {
    QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->chroma_ac_huff_tbl.values[%d] = %d\n",
        index,pQ6cfg->chroma_ac_huff_tbl.values[index]);
  }

  for(index =0; index < HUFF_BITS ;index++) {
    QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->luma_ac_huff_tbl.bits[%d] = %d\n",
        index,pQ6cfg->luma_ac_huff_tbl.bits[index]);
  }

  for(index =0; index < HUFF_VALUES ;index++) {
    QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->luma_ac_huff_tbl.values[%d] = %d\n",
        index,pQ6cfg->luma_ac_huff_tbl.values[index]);
  }

  for(index =0; index < HUFF_BITS ;index++) {
    QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->luma_dc_huff_tbl.bits[%d] = %d\n",
        index,pQ6cfg->luma_dc_huff_tbl.bits[index]);
  }

  for(index =0; index < HUFF_VALUES ;index++) {
    QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->luma_dc_huff_tbl.values[%d] = %d\n",
        index,pQ6cfg->luma_dc_huff_tbl.values[index]);
  }

  for(index = 0; index < QUANT_SIZE; index++) {
    QIDBG_ERROR("Q6 JPEG Config: Luma: pQ6cfg->qtbl_0[%d] = %d\n",
        index,pQ6cfg->qtbl_0[index]);
  }

  for(index = 0; index < QUANT_SIZE; index++) {
    QIDBG_ERROR("Q6 JPEG Config: Chroma: pQ6cfg->qtbl_1[%d] = %d\n",
        index,pQ6cfg->qtbl_1[index]);
  }

  QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->input_width = %d\n", (int)pQ6cfg->input_width);
  QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->input_height = %d\n", (int)pQ6cfg->input_height);
  QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->input_stride = %d\n", (int)pQ6cfg->input_stride);
  QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->output_MCUs = %d\n", (int)pQ6cfg->output_MCUs);
  QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->rotation = %d\n", (int)pQ6cfg->rotation);
  QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->output_buffer_length = %d\n",
                (int)pQ6cfg->output_buffer_length);
  QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->scale_cfg.enable = %d\n",
                (int)pQ6cfg->scale_cfg.enable);
  QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->restart_interval = %d\n",
                (int)pQ6cfg->restart_interval);
  QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->base_restart_marker = 0x%x\n",
                (int)pQ6cfg->base_restart_marker);
  QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->color_format = %d\n", (int)pQ6cfg->color_format);
  QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->scale_cfg.enable = %d\n",
                (int)pQ6cfg->scale_cfg.enable);
  QIDBG_ERROR("Q6 JPEG Config: pQ6cfg->output_start_mcu_index = %d\n",
                (int)pQ6cfg->output_start_mcu_index);

  QIDBG_ERROR("mInputImage->getPlane(QIPlane::PLANE_Y)->ActualAddr()=0x%x",
    (int)mInputImage->getPlane(QIPlane::PLANE_Y)->ActualAddr());
  QIDBG_ERROR("mInputImage->getPlane(QIPlane::PLANE_Y)->ActualSize().Length()=%d",
    (int)mInputImage->getPlane(QIPlane::PLANE_Y)->ActualSize().Length());
  QIDBG_ERROR("mInputImage->getPlane(QIPlane::PLANE_CB_CR)->ActualAddr()=0x%x",
    (int)mInputImage->getPlane(QIPlane::PLANE_CB_CR)->ActualAddr());
  QIDBG_ERROR("mInputImage->getPlane(QIPlane::PLANE_CB_CR)->ActualSize().Length()=%d",
    (int)mInputImage->getPlane(QIPlane::PLANE_CB_CR)->ActualSize().Length());

  QIDBG_ERROR("mOutputImage->BaseAddr()=0x%x",
    (int)mOutputImage->BaseAddr());
  QIDBG_ERROR("mOutputImage->WorkBufSize()=%d",
    (int)mOutputImage->WorkBufSize());
#endif //ENABLE_Q6_CONFIG_LOGS

  p_output_buffer = mOutputImage->BaseAddr();

  register_buf(mInputImage->BaseAddr(),
      (mInputImage->getPlane(QIPlane::PLANE_Y)->Size().Length() +
        mInputImage->getPlane(QIPlane::PLANE_CB_CR)->Size().Length()),
        mInputImage->Fd());

  register_buf(p_output_buffer, mOutputImage->WorkBufSize(),
      mOutputImage->Fd());

  QIDBG_HIGH("%s:%d] adsp_jpege_q6_process START ++++", __func__, __LINE__);

  lrc = adsp_jpege_q6_process(pQ6cfg,
            mInputImage->getPlane(QIPlane::PLANE_Y)->ActualAddr(),
            (int)mInputImage->getPlane(QIPlane::PLANE_Y)->ActualSize().Length(),
            mInputImage->getPlane(QIPlane::PLANE_CB_CR)->ActualAddr(),
            mInputImage->getPlane(QIPlane::PLANE_CB_CR)->ActualSize().Length(),
            NULL,
            0,
            NULL,
            0,
            (uint32*)&(jpeg_output_size),
            p_output_buffer,
            mOutputImage->WorkBufSize(),
            1);

  QIDBG_HIGH("%s:%d] adsp_jpege_q6_process DONE ++++ error=%d,Output size=%d",
      __func__, __LINE__, lrc,jpeg_output_size);

  register_buf(mInputImage->BaseAddr(),
      (mInputImage->getPlane(QIPlane::PLANE_Y)->Size().Length() +
        mInputImage->getPlane(QIPlane::PLANE_CB_CR)->Size().Length()), -1);

  register_buf(p_output_buffer, mOutputImage->WorkBufSize(), -1);

#ifdef DUMP_JPEG_BITSTREAM
  {
    FILE *fout;
    fout = fopen("/data/jpeg_q6.bs", "wb");
    if (fout) {
        fwrite(p_output_buffer, 1,jpeg_output_size, fout);
        fclose(fout);
    }
  }
#endif //DUMP_JPEG_BITSTREAM

  /*add EOI*/
  *(p_output_buffer+jpeg_output_size) = 0xFF;
  *(p_output_buffer+jpeg_output_size+1) = 0xD9;
  jpeg_output_size += 2;

  mState = ESTATE_IDLE;
  QI_SIGNAL(&mCond);
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] Error Encoding ", __func__, __LINE__);
    for (uint32_t k = 0; k < mObserverCnt; k++)
      mObserver[k]->EncodeError(QImageEncoderObserver::ERROR_GENERAL);
    QI_UNLOCK(&mMutex);
    return;
  }

  mOutputImage->SetFilledLen(jpeg_output_size);

  for (uint32_t k = 0; k < mObserverCnt; k++)
     mObserver[k]->EncodeComplete(mOutputImage);
  QI_UNLOCK(&mMutex);
  return;
}

void QImageQ6Encoder::ReleaseSession()
{
  QIDBG_HIGH("%s:%d] state %d", __func__, __LINE__, mState);
  Stop();

  if (mEngine.p_internal_buf) {
    jpeg_buffer_destroy((jpeg_buffer_t *)&(mEngine.p_internal_buf));
  }
  if (mEngine.jpegeBitStreamState.buffer) {
    JPEG_FREE(mEngine.jpegeBitStreamState.buffer);
    memset(&(mEngine.jpegeBitStreamState), 0, sizeof(bitstream_t));
  }

  if (mSource.p_fragments[0].color.yuv.luma_buf) {
    jpeg_buffer_destroy(&mSource.p_fragments[0].color.yuv.luma_buf);
  }
  if (mSource.p_fragments[0].color.yuv.chroma_buf) {
    jpeg_buffer_destroy(&mSource.p_fragments[0].color.yuv.chroma_buf);
  }

  if (mpOutputThread && (mMode == EPIECEWISE_OUTPUT)) {
    mpOutputThread->JoinThread();
  }

  if (mpEncoderThread) {
    mpEncoderThread->JoinThread();
    delete mpEncoderThread;
    mpEncoderThread = NULL;
  }

  if (mpOutputThread) {
    delete mpOutputThread;
    mpOutputThread = NULL;
  }
  mObserverCnt = 0;
}
