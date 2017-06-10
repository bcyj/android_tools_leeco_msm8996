/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QImageSWEncoder.h"
#include "QIParams.h"

extern "C" {

#include "jpeg_buffer_private.h"
#include "jpege_engine_sw.h"
#include "jpege.h"
#include "jpegerr.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* Static constants */
static const int16_t zigzagTable[JBLOCK_SIZE] =
{
  0,   1,  8, 16,  9,  2,  3, 10,
  17, 24, 32, 25, 18, 11,  4,  5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13,  6,  7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
};
static const int16_t zigzagTableTranspose[JBLOCK_SIZE] =
{
  0,  8,  1,  2,  9,  16, 24, 17,
  10, 3,  4,  11, 18, 25, 32, 40,
  33, 26, 19, 12, 5,  6,  13, 20,
  27, 34, 41, 48, 56, 49, 42, 35,
  28, 21, 14, 7,  15, 22, 29, 36,
  43, 50, 57, 58, 51, 44, 37, 30,
  23, 31, 38, 45, 52, 59, 60, 53,
  46, 39, 47, 54, 61, 62, 55, 63
};
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* Extern functions from other files within the same engine */
extern void jpege_engine_sw_huff_init_tables (jpege_engine_sw_t *p_engine);
extern void jpege_engine_sw_huff_encode(jpege_engine_sw_t *p_engine,
  int16_t *zigzagOutput,
  yCbCrType_t yORcbcr);
extern void jpege_engine_sw_fdct_block (const uint8_t *pixelDomainBlock,
  DCTELEM       *frequencyDomainBlock);
extern void jpege_engine_sw_fetch_dct_block_luma(const uint8_t *,
  int16_t *, uint32_t);
extern void jpege_engine_sw_fetch_dct_block_chroma(const uint8_t  *,
  int16_t *, uint32_t,
  input_format_t);
extern void jpege_engine_sw_quant_zigzag (const int16_t  *quantInput,
  int16_t  *zigzagOutput,
  const int16_t  *zigzagOffsetTable,
  const int16_t  *quantTable);
extern void jpege_engine_sw_pack_bs (uint32_t, uint32_t, bitstream_t *);

/* configue function of in-line scale */
extern int  jpege_engine_sw_scale_configure(jpege_engine_sw_t  *p_engine,
  jpege_img_cfg_t    *p_config,
  jpege_img_data_t   *p_source);

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
#define INTERNAL_BUFFER_SIZE   8192 /* in bytes */
#define MAX_MCU_SIZE           384  /* in bytes */

#define MAX_UPSCALE_SHIFT  5   // support up to 32 upscale

#define MAX_DOWNSCALE_SHIFT  5 // support up to 1/32 downscale

} // end extern "C"

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* Function prototypes of helper functions */
static int jpege_engine_sw_configure(jpege_engine_sw_t*, jpege_img_cfg_t*,
  jpege_img_data_t*);
static void jpege_engine_sw_copy_bytes(jpege_engine_sw_t *p_engine,
  uint8_t force_flush);
static void jpege_engine_sw_fetch_dct_block_partial_luma(jpege_engine_sw_t *,
  const uint8_t *,
  uint32_t, uint32_t, int16_t *);
static void jpege_engine_sw_fetch_dct_block_partial_chroma(
  jpege_engine_sw_t *,
  const uint8_t *,
  uint32_t, uint32_t, int16_t *);
static void jpege_engine_sw_fetch_dct_mcu(jpege_engine_sw_t *,
  int16_t *, int16_t *);
static int jpege_engine_sw_configure_internal_buffer(
  jpege_engine_sw_t *p_engine);
static int jpege_engine_sw_configure_parameter(
  jpege_engine_sw_t  *p_engine);
static int jpege_engine_sw_configure_rotation(jpege_engine_sw_t *,
  jpege_img_cfg_t *);
static void jpege_engine_sw_configure_huff_tables(
  jpege_engine_sw_t  *p_engine,
  jpege_img_cfg_t *p_config);

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
bool QImageSWEncoder::IsAvailable(QIEncodeParams& aEncodeParams)
{
  bool lUpScale = false, lScaleEnabled = false;
  QISize lInputSize = aEncodeParams.InputSize();

  if (aEncodeParams.Rotation() % 90) {
    QIDBG_ERROR("%s:%d] rotation failed ", __func__, __LINE__);
    return false;
  }

  if (aEncodeParams.OutputSize().IsZero()
    || aEncodeParams.InputSize().IsZero()) {
    QIDBG_ERROR("%s:%d] dimension failed ", __func__, __LINE__);
    return false;
  }

  if (!aEncodeParams.Crop().isZero()) {
    lInputSize = aEncodeParams.Crop().Size();
    if (!aEncodeParams.Crop().isValid()) {
      QIDBG_ERROR("%s:%d] crop failed (%d %d %d %d)", __func__, __LINE__,
        aEncodeParams.Crop().Left(),
        aEncodeParams.Crop().Top(),
        aEncodeParams.Crop().Width(),
        aEncodeParams.Crop().Height());
      return false;
    }

    if (aEncodeParams.Crop().isValid(aEncodeParams.InputSize())) {
      QIDBG_ERROR("%s:%d] crop dim failed ", __func__, __LINE__);
      return false;
    }
  }

  if ((aEncodeParams.OutputSize().Width() < lInputSize.Width())
    && (aEncodeParams.OutputSize().Height() > lInputSize.Height())) {
    QIDBG_ERROR("%s:%d] scale dir failed ", __func__, __LINE__);
    return false;
  }

  if ((aEncodeParams.OutputSize().Width() > lInputSize.Width())
    && (aEncodeParams.OutputSize().Height() < lInputSize.Height())) {
    QIDBG_ERROR("%s:%d] scale dir failed ", __func__, __LINE__);
    return false;
  }

  lScaleEnabled = (aEncodeParams.OutputSize().Width() != lInputSize.Width())
    || (aEncodeParams.OutputSize().Height() != lInputSize.Height())
    || !aEncodeParams.Crop().isZero();

  QIDBG_HIGH("%s:%d] scale enabled %d", __func__, __LINE__, lScaleEnabled);
  if (lScaleEnabled) {
    lUpScale = aEncodeParams.OutputSize().Width() > lInputSize.Width();
    if (lUpScale) {
      if (aEncodeParams.OutputSize().Width() >
        (lInputSize.Width() << MAX_UPSCALE_SHIFT)) {
        QIDBG_ERROR("%s:%d] scale ratio failed ", __func__, __LINE__);
        return false;
      }
      if (aEncodeParams.OutputSize().Height() >
        (lInputSize.Height() << MAX_UPSCALE_SHIFT)) {
        QIDBG_ERROR("%s:%d] scale ratio failed ", __func__, __LINE__);
        return false;
      }
    } else { // downscale
      if (aEncodeParams.OutputSize().Width() <
        (lInputSize.Width() >> MAX_DOWNSCALE_SHIFT)) {
        QIDBG_ERROR("%s:%d] scale ratio failed ", __func__, __LINE__);
        return false;
      }
      if (aEncodeParams.OutputSize().Height() <
        (lInputSize.Height() >> MAX_DOWNSCALE_SHIFT)) {
        QIDBG_ERROR("%s:%d] scale ratio failed ", __func__, __LINE__);
        return false;
      }
    }
  }
  /* populate the scale structure*/
  if (lScaleEnabled) {
    mConfig.scale_cfg.enable = 1;
    mConfig.scale_cfg.input_width = aEncodeParams.Crop().Width();
    mConfig.scale_cfg.input_height = aEncodeParams.Crop().Height();
    mConfig.scale_cfg.h_offset = aEncodeParams.Crop().Left();
    mConfig.scale_cfg.v_offset = aEncodeParams.Crop().Top();
    mConfig.scale_cfg.output_width = aEncodeParams.OutputSize().Width();
    mConfig.scale_cfg.output_height = aEncodeParams.OutputSize().Height();
    QIDBG_HIGH("%s:%d] input %dx%d crop (%d, %d) output %dx%d",
      __func__, __LINE__,
      mConfig.scale_cfg.input_width,
      mConfig.scale_cfg.input_height,
      mConfig.scale_cfg.h_offset,
      mConfig.scale_cfg.v_offset,
      mConfig.scale_cfg.output_width,
      mConfig.scale_cfg.output_height);
  } else {
    mConfig.scale_cfg.enable = 0;
  }
  return true;
}

/*===========================================================================
 * Function: New
 *
 * Description: 2 phase constructor for QImageSWEncoder
 *
 * Input parameters:
 *   aParams - encoder params
 *
 * Return values:
 *   encoder interface pointer
 *
 * Notes: none
 *==========================================================================*/
QImageEncoderInterface* QImageSWEncoder::New(QIEncodeParams &aParams)
{
  QImageSWEncoder* lEncoder = new QImageSWEncoder();
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
 * Function: QImageSWEncoder
 *
 * Description: QImageSWEncoder constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageSWEncoder::QImageSWEncoder()
{
  // Initialize the fields inside the engine structure below
  memset(&mEngine, 0, sizeof(jpege_engine_sw_t));
  memset(&mConfig, 0, sizeof(jpege_img_cfg_t));
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
 * Function: ~QImageSWEncoder
 *
 * Description: QImageSWEncoder destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageSWEncoder::~QImageSWEncoder()
{
  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, mState);
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
  // Destroy the scale internal buffers
  jpege_engine_sw_scale_destroy(&(mEngine.jpege_scale));

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
int QImageSWEncoder::SetOutputMode(QIOutputMode aMode)
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
int QImageSWEncoder::Start()
{
  int lrc = QI_SUCCESS;

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
  // Configure the engine based on the input configuration
  lrc = jpege_engine_sw_configure(&mEngine, &mConfig, &mSource);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  mEngine.abort_flag = false;
  mEngine.error_flag = false;

  // Create output thread
  if (mMode == EPIECEWISE_OUTPUT) {
    mpOutputThread = new QISWOutputThread(*this);
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

  mpEncoderThread = new QISWEncoderThread(*this);
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
static void PrintTables(jpeg_huff_table_t *tbl)
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
 *              jpeg_huff_table_t
 *
 * Input parameters:
 *   ap_htable - pointer to jpeg_huff_table_t
 *   aType - type of huffman table
 *
 * Return values:
 *   QI_SUCCES
 *   QI_ERR_NOT_FOUND
 *
 * Notes: none
 *==========================================================================*/
int QImageSWEncoder::copyHuffTable(jpeg_huff_table_t *ap_htable,
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
int QImageSWEncoder::setEncodeParams(QIEncodeParams &aParams)
{
  int lrc = QI_SUCCESS;
  QIQuantTable *lQTable = NULL;

  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, mState);
  QI_LOCK(&(mMutex));
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }

  mEncodeParams = &aParams;
  mConfig.quality = mEncodeParams->Quality();
  mConfig.restart_interval = mEncodeParams->RestartInterval();
  mConfig.rotation_degree_clk = mEncodeParams->Rotation();

  if (QI_ERROR(copyHuffTable(&mConfig.chroma_ac_huff_tbl,
    QIHuffTable::HTABLE_CHROMA_AC))) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  if (QI_ERROR(copyHuffTable(&mConfig.chroma_dc_huff_tbl,
    QIHuffTable::HTABLE_CHROMA_DC))) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  if (QI_ERROR(copyHuffTable(&mConfig.luma_ac_huff_tbl,
    QIHuffTable::HTABLE_LUMA_AC))) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  if (QI_ERROR(copyHuffTable(&mConfig.luma_dc_huff_tbl,
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
  mConfig.luma_quant_tbl = lQTable->Table();

  lQTable = mEncodeParams->QuantTable(QIQuantTable::QTABLE_CHROMA);
  if (NULL == lQTable) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }
  mConfig.chroma_quant_tbl = lQTable->Table();

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
void QImageSWEncoder::ConfigureDimensions()
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

  if ((lPadWidth != lWidth) && !mConfig.scale_cfg.enable) {
    mConfig.scale_cfg.enable = 1;
    mConfig.scale_cfg.input_width = lWidth;
    mConfig.scale_cfg.input_height = mEncodeParams->InputSize().Height();
    mConfig.scale_cfg.h_offset = 0;
    mConfig.scale_cfg.v_offset = 0;
    mConfig.scale_cfg.output_width = lWidth;
    mConfig.scale_cfg.output_height = mEncodeParams->InputSize().Height();
  }
  QIDBG_HIGH("%s:%d] input %dx%d crop (%d, %d) output %dx%d",
    __func__, __LINE__,
    mConfig.scale_cfg.input_width,
    mConfig.scale_cfg.input_height,
    mConfig.scale_cfg.h_offset,
    mConfig.scale_cfg.v_offset,
    mConfig.scale_cfg.output_width,
    mConfig.scale_cfg.output_height);
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
int QImageSWEncoder::addInputImage(QImage &aImage)
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
int QImageSWEncoder::addOutputImage(QImage &aImage)
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
int QImageSWEncoder::addObserver(QImageEncoderObserver &aObserver)
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
int QImageSWEncoder::Stop()
{
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
static int jpege_engine_sw_configure(jpege_engine_sw_t  *p_engine,
  jpege_img_cfg_t *p_config,
  jpege_img_data_t *p_source)
{
  int rc;

  /*------------------------------------------------------------
    Initialize the fields in the p_engine structure
  --------------------------------------------------------------*/
  p_engine->restartInterval = p_config->restart_interval;
  p_engine->rotation = p_config->rotation_degree_clk;
  if (p_config->scale_cfg.enable) {
    p_engine->lumaWidth  = p_config->scale_cfg.output_width;
    p_engine->lumaHeight = p_config->scale_cfg.output_height;
  }
  else {
    p_engine->lumaWidth = p_source->width;
    p_engine->lumaHeight = p_source->height;
  }
  p_engine->InputFormat = ((uint8_t)p_source->color_format % 2) ?
    YCbCr : YCrCb;
  p_engine->subsampling =
    (jpeg_subsampling_t)((uint8_t)p_source->color_format / 2);

  /* Initialize input Y and CbCr/CrCb pointers */
  p_engine->inputYPtr =
    ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->ptr +
    ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->start_offset;
  p_engine->inputCbCrPtr =
    ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->ptr +
    ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->
      start_offset;
  QIDBG_MED("jpege_engine_sw_configure luma_start_offset %d "
     "chroma_start_offset %d",
    ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->start_offset,
    ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->
    start_offset);

  /* Initialize dest buffers and internal bitstream buffer */
  rc = jpege_engine_sw_configure_internal_buffer(p_engine);
  if (JPEG_FAILED(rc))
  {
    return rc;
  }
  /* Configure parameters - chroma sizes and MCU sizes determined by
   * subsample format, and other initializations.
   */
  rc = jpege_engine_sw_configure_parameter(p_engine);
  if (JPEG_FAILED(rc))
  {
      return rc;
  }
  /* Configure rotation related parameters - rotated luma & chroma sizes,
   * rotated MCU sizes, rotated luma & chroma fetch origins and steps,
   * zigzag table and quant tables for DCT domain rotation.
   */
  rc = jpege_engine_sw_configure_rotation(p_engine, p_config);
  if (JPEG_FAILED(rc)) {
      return rc;
  }
  /* Configure huffman tables */
  jpege_engine_sw_configure_huff_tables(p_engine, p_config);

  /* Configure default fetch mcu function */
  p_engine->jpege_engine_sw_fetch_dct_mcu_func =
    &jpege_engine_sw_fetch_dct_mcu;

  /* Check if scale is enabled */
  if (p_config->scale_cfg.enable) {
    /* Configue for in-line crop and scale */
    rc = jpege_engine_sw_scale_configure(p_engine, p_config, p_source);
    return rc;
  }
  return JPEGERR_SUCCESS;
}

/*===========================================================================
 * Function: jpege_engine_sw_stuff_bs
 *
 * Description: Static function to stuff bitstream
 *
 * Input parameters:
 *   p_bitstream - pointer to bitstream buffer
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
static void jpege_engine_sw_stuff_bs(bitstream_t *p_bitstream)
{
  if (p_bitstream->bitsFree != 32) {
    /*-----------------------------------------------------------------------
    At least one bit in the 32-bit assembly buffer. Append 7 1s starting
    from the bitsFree. There is at least one 1 already there.
    -----------------------------------------------------------------------*/
    jpege_engine_sw_pack_bs(0x7F, 7, p_bitstream);

    //Clear bitAssemblybuffer and reset empty bit location.
    p_bitstream->bitAssemblyBufferUL32 = 0;
    p_bitstream->bitsFree = 32;
  }
}

/*===========================================================================
 * Function: jpege_engine_sw_insert_restart_marker
 *
 * Description: Static function to insert restart marker
 *
 * Input parameters:
 *   p_engine - pointer to engine
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
static void jpege_engine_sw_insert_restart_marker(jpege_engine_sw_t *p_engine)
{
  bitstream_t *p_bitstream = &(p_engine->jpegeBitStreamState);
  uint16_t currentMarker;

  /*--------------------------------------------------------------------------
  1. 1-bit stuffing at the end of remaining bits in the 64-bit word buffer.
  2. Flush out the remaining byte in the 64-bit word buffer to the bit-stream.
     Reset the bitCount
  3. Insert the appropriate the restart marker to the bit-stream
    ------------------------------------------------------------------------*/

  jpege_engine_sw_stuff_bs(p_bitstream);

  // Insert Restart Marker and update Marker Mod Count.
  currentMarker = BASE_RESTART_MARKER + p_engine->restartMarkerModCount;
  p_engine->restartMarkerModCount =
      (p_engine->restartMarkerModCount + 1) % MAX_RESTART_MARKER_COUNT;

  // Copy the restart marker to the bit-stream buffer. Increment byte count.
  *(p_bitstream->nextBytePtr++) = 0xFF;
  *(p_bitstream->nextBytePtr++) = (uint8_t)(currentMarker & 0xFF);

  // Reset Y, Cb and Cr DC values.
  p_engine->prevLumaDC = 0;
  p_engine->prevCbDC = 0;
  p_engine->prevCrDC = 0;
}

/*===========================================================================
 * Function: jpege_engine_sw_fetch_dct_mcu
 *
 * Description: Static function to fetch and dor DCT
 *
 * Input parameters:
 *   p_engine - pointer to the engine
 *   currMCULumaDctOutput - pointer to luma output
 *   currMCUChromaDctOutput - pointer to chroma output
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
static void jpege_engine_sw_fetch_dct_mcu(jpege_engine_sw_t *p_engine,
  int16_t *currMCULumaDctOutput,
  int16_t *currMCUChromaDctOutput)
{
  uint8_t *currMCUYStartAddr, *currMCUCbCrStartAddr;
  uint8_t *currBlockStartAddr;
  int16_t *currDctOutputPtr;
  uint32_t horiPixelIndex, vertPixelIndex;
  uint32_t vertMCUOffset, horiMCUOffset, vertBound, horiBound;
  int32_t  vertCross, horiCross;
  uint32_t i, j;

  QIDBG_LOW("jpege_engine_sw_fetch_mcu: horiMCUIndex=%d, vertMCUIndex=%d\n",
    p_engine->rotatedhoriMCUIndex, p_engine->rotatedvertMCUIndex);

  /* fetch luma data */
  currDctOutputPtr = currMCULumaDctOutput;

  /* Calculate the modified origin for the luma data */
  horiPixelIndex = p_engine->rotatedhoriMCUIndex * p_engine->rotatedMCUWidth;
  vertPixelIndex = p_engine->rotatedvertMCUIndex * p_engine->rotatedMCUHeight;

  /* Use the MCU x and y coordinates from above to calculate the address of
   * the first pixel in the current MCU.
   */
  currMCUYStartAddr = p_engine->inputYPtr +
    (int32_t)p_engine->currOrigin_luma +
    //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
    (int32_t)horiPixelIndex * p_engine->horiIncrement_luma +
    //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
    (int32_t)vertPixelIndex * p_engine->vertIncrement_luma;
  QIDBG_LOW("%s: inputYPtr=0x%x, currMCUYStartAddr=0x%x\n",
    __func__, (uint32_t)p_engine->inputYPtr, (uint32_t)currMCUYStartAddr);

  /* Two layers of loops for the luma blocks within a MCU */
  vertMCUOffset = 0;

  for (i=0; i < p_engine->rotatedMCUHeight/BLOCK_HEIGHT; i++) {
    /* Calculate how many pixels are needed to pad vertically */
    vertCross = (int32_t)(p_engine->rotatedlumaHeight -
      (vertPixelIndex + vertMCUOffset + 1));
    if (vertCross >= BLOCK_HEIGHT) {
      vertBound = BLOCK_HEIGHT;
      currBlockStartAddr = currMCUYStartAddr +
        ((int32_t)vertMCUOffset * p_engine->vertIncrement_luma);
      //lint -e{639} Strong type mismatch for type 'uint32_t' in
      // binary operation
      //int32_t*unit32 is not a problem.
    } else if (vertCross >= 0) {
      // lint -e{632} Assignment to strong type 'uint32_t'
      // in context: assignment
      // assignment to uint32_t is not a problem, since vertCross >= 0 here.
      vertBound = vertCross + 1;
      // lint -e{639} Strong type mismatch for type 'uint32_t'
      // in binary operation
      // int32_t*unit32 is not a problem.
      currBlockStartAddr = currMCUYStartAddr + (int32_t)vertMCUOffset *
        p_engine->vertIncrement_luma;
    }
    else {
      vertBound = 1;
      currBlockStartAddr = currMCUYStartAddr +
        ((int32_t)(vertMCUOffset - p_engine->rotatedMCUHeight) +
      // lint -e{639} Strong type mismatch for type
      // 'uint32_t' in binary operation
        (BLOCK_HEIGHT + vertCross)) * p_engine->vertIncrement_luma;
    }
    horiMCUOffset = 0;
    for (j=0; j < p_engine->rotatedMCUWidth/BLOCK_WIDTH; j++) {
      /* Calculate how many pixels are needed to pad horizontally */
      //lint -e{632} Assignment to strong type 'int32_t'
      // in context: assignment
      horiCross = p_engine->rotatedlumaWidth - (horiPixelIndex + horiMCUOffset
        + 1);
      if (horiCross >= BLOCK_WIDTH){
        horiBound = BLOCK_WIDTH;
        //lint -e{639} Strong type mismatch for type 'uint32_t'
        // in binary operation
        currBlockStartAddr += (int32_t)horiMCUOffset *
          p_engine->horiIncrement_luma;
      } else if (horiCross >= 0) {
        //lint -e{632} Assignment to strong type 'uint32_t' in context:
        //assignment to uint32_t is not a problem, since horiCross >= 0 here.
        horiBound = horiCross + 1;
        //lint -e{639} Strong type mismatch for type 'uint32_t'
        // in binary operation
        currBlockStartAddr += (int32_t)horiMCUOffset *
          p_engine->horiIncrement_luma;
      } else {
        horiBound = 1;
        currBlockStartAddr += ((int32_t)(horiMCUOffset -
          p_engine->rotatedMCUWidth) +
          (BLOCK_WIDTH + horiCross)) * p_engine->horiIncrement_luma;
        //lint -e{639} Strong type mismatch for type 'uint32_t' in
        // binary operation
      }
      QIDBG_LOW("jpege_engine_sw_fetch_mcu: currYBlockStartAddr=0x%x\n",
                   (uint32_t)currBlockStartAddr);

      if ((vertBound == BLOCK_HEIGHT) && (horiBound == BLOCK_WIDTH)) {
        jpege_engine_sw_fetch_dct_block_luma(
          currBlockStartAddr, currDctOutputPtr, p_engine->lumaWidth);
      } else {
        jpege_engine_sw_fetch_dct_block_partial_luma(
          p_engine, currBlockStartAddr, horiBound, vertBound,
          currDctOutputPtr);
      }

      currDctOutputPtr += JBLOCK_SIZE;
      horiMCUOffset += BLOCK_WIDTH;
    }
    vertMCUOffset += BLOCK_HEIGHT;
  }

  /* fetch chroma data */
  currDctOutputPtr = currMCUChromaDctOutput;

  /* Calculate the modified origin for the chroma data */
  horiPixelIndex = p_engine->rotatedhoriMCUIndex * BLOCK_WIDTH;
  vertPixelIndex = p_engine->rotatedvertMCUIndex * BLOCK_HEIGHT;
  currMCUCbCrStartAddr =
    p_engine->inputCbCrPtr + (int32_t)p_engine->currOrigin_chroma +
    //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
    (int32_t)horiPixelIndex * p_engine->horiIncrement_chroma +
    //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
    (int32_t)vertPixelIndex * p_engine->vertIncrement_chroma;
  QIDBG_LOW("%s: inputCbCrPtr=0x%x, currMCUCbCrStartAddr=0x%x\n",
    __func__, (uint32_t)p_engine->inputCbCrPtr,
    (uint32_t)currMCUCbCrStartAddr);

  /* Calculate how many pixels are needed to pad vertically */
  //lint -e{632} Assignment to strong type 'int32_t' in context: assignment
  vertCross = p_engine->rotatedchromaHeight - (vertPixelIndex + 1);
  if (vertCross >= BLOCK_HEIGHT) {
    vertBound = BLOCK_HEIGHT; // No pad
  } else {
    //lint -e{632} Assignment to strong type 'uint32_t' in context: assignment
    vertBound = vertCross + 1; // Partially Pad
  }
  /* Calculate how many pixels are needed to pad horizontally */
  //lint -e{632} Assignment to strong type 'int32_t' in context: assignment
  horiCross = p_engine->rotatedchromaWidth - (horiPixelIndex + 1);
  if (horiCross >= BLOCK_WIDTH) {
    horiBound = BLOCK_WIDTH; // No pad
  } else {
    //lint -e{632} Assignment to strong type 'uint32_t' in context: assignment
    horiBound = horiCross + 1; // Partially pad
  }

  if ((vertBound == BLOCK_HEIGHT) && (horiBound == BLOCK_WIDTH)) {
    jpege_engine_sw_fetch_dct_block_chroma(
      currMCUCbCrStartAddr, currDctOutputPtr, p_engine->chromaWidth,
      p_engine->InputFormat);
  } else {
    jpege_engine_sw_fetch_dct_block_partial_chroma(
      p_engine, currMCUCbCrStartAddr, horiBound, vertBound, currDctOutputPtr);
  }
}

/*===========================================================================
 * Function: jpege_engine_sw_fetch_dct_block_partial_luma
 *
 * Description: Static function to fetch and do partial luma
 *
 * Input parameters:
 *   p_engine - pointer to the engine
 *   p_luma_block_origin - pointer to luma output
 *   currMCUChromaDctOutput -
 *   hori_bound -
 *   vert_bound -
 *   p_dct_output -
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
static void jpege_engine_sw_fetch_dct_block_partial_luma(
  jpege_engine_sw_t *p_engine,
  const uint8_t *p_luma_block_origin,
  uint32_t hori_bound,
  uint32_t vert_bound,
  int16_t *p_dct_output)
{
  const uint8_t *p_luma_block;
  const uint8_t *p_padding_line;
  uint32_t luma_width = p_engine->lumaWidth;
  uint8_t fetched_block[JBLOCK_SIZE];
  uint8_t *p_fetched_block = fetched_block;
  uint32_t temp_bound;
  uint32_t i, j;

  switch (p_engine->rotation) {
  case 90:
    p_luma_block = p_luma_block_origin + ((BLOCK_HEIGHT - hori_bound) *
      luma_width);
    // swap hori_bound and vert_bound
    temp_bound = hori_bound;
    hori_bound = vert_bound;
    vert_bound = temp_bound;
    break;

  case 180:
    p_luma_block = p_luma_block_origin + ((BLOCK_HEIGHT - vert_bound) *
      luma_width) +
      (BLOCK_WIDTH  - hori_bound);
    break;

  case 270:
    p_luma_block = p_luma_block_origin + (BLOCK_WIDTH  - vert_bound);
    // swap horiBound and vertBound
    temp_bound = hori_bound;
    hori_bound = vert_bound;
    vert_bound = temp_bound;
    break;

  default:
    p_luma_block = p_luma_block_origin;
    break;
  }
  // Vertically unpadded part
  for (i = 0; i < vert_bound; i++) {
    // Horizontally unpadded part
    STD_MEMMOVE(p_fetched_block, p_luma_block, (hori_bound * sizeof(uint8_t)));
    p_fetched_block += hori_bound;

    // Horizontally padded part
    for (j = hori_bound; j < BLOCK_WIDTH; j++) {
        *p_fetched_block = *(p_fetched_block - 1);
        p_fetched_block++;
    }
    p_luma_block += luma_width;
  }

  // Vertically padded part
  p_padding_line = p_fetched_block - BLOCK_WIDTH;
  for (i = vert_bound; i < BLOCK_HEIGHT; i++) {
    STD_MEMMOVE(p_fetched_block, p_padding_line, (BLOCK_WIDTH *
      sizeof(uint8_t)));
    p_fetched_block += BLOCK_WIDTH;
    p_luma_block    += luma_width;
  }

  // Do DCT
  jpege_engine_sw_fdct_block(fetched_block, p_dct_output);
}

/*===========================================================================
 * Function: jpege_engine_sw_fetch_dct_block_partial_luma
 *
 * Description: Static function to fetch and do partial luma
 *
 * Input parameters:
 *   p_engine - pointer to the engine
 *   p_luma_block_origin - pointer to luma output
 *   currMCUChromaDctOutput -
 *   hori_bound -
 *   vert_bound -
 *   p_dct_output -
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
static void jpege_engine_sw_fetch_dct_block_partial_chroma(
  jpege_engine_sw_t *p_engine,
  const uint8_t *p_chroma_block_origin,
  uint32_t hori_bound,
  uint32_t vert_bound,
  int16_t *p_dct_output)
{
  const uint8_t *p_chroma_block;
  const uint8_t *p_padding_line;
  uint32_t chroma_width = p_engine->chromaWidth;
  // chroma is interleaved
  uint8_t fetched_block[JBLOCK_SIZE * NUM_CHROMA_BLOCKS];
  uint8_t deinterleaved_block[JBLOCK_SIZE * NUM_CHROMA_BLOCKS];
  uint8_t *p_fetched_block = fetched_block;
  uint8_t *p_deinterleaved_block1 = deinterleaved_block;
  uint8_t *p_deinterleaved_block2 = p_deinterleaved_block1 + JBLOCK_SIZE;
  uint8_t *p_cb_block, *p_cr_block;
  uint32_t temp_bound;
  uint32_t i, j;

  switch (p_engine->rotation) {
  case 90:
    p_chroma_block = p_chroma_block_origin +
      ((BLOCK_HEIGHT - hori_bound) * (chroma_width * 2));
    // swap hori_bound and vert_bound
    temp_bound = hori_bound;
    hori_bound = vert_bound;
    vert_bound = temp_bound;
    break;

  case 180:
    p_chroma_block = p_chroma_block_origin +
      ((BLOCK_HEIGHT - vert_bound) * (chroma_width * 2)) +
      ((BLOCK_WIDTH  - hori_bound) * 2);
    break;

  case 270:
    p_chroma_block = p_chroma_block_origin + ((BLOCK_WIDTH  - vert_bound) * 2);
    // swap hori_bound and vert_bound
    temp_bound = hori_bound;
    hori_bound = vert_bound;
    vert_bound = temp_bound;
    break;

  default:
    p_chroma_block = p_chroma_block_origin;
    break;
  }

  // Vertically unpadded part
  for (i = 0; i < vert_bound; i++) {
    // Horizontally unpadded part
    STD_MEMMOVE(p_fetched_block, p_chroma_block, (hori_bound *
      sizeof(uint8_t) * 2));
    p_fetched_block += (hori_bound * 2);

    // Horizontally padded part
    for (j = hori_bound; j < BLOCK_WIDTH; j++) {
      *(p_fetched_block)     = *(p_fetched_block - 2);
      *(p_fetched_block + 1) = *(p_fetched_block - 1);
      p_fetched_block += 2;
    }
    p_chroma_block += (chroma_width * 2);
  }

  // Vertically padded part
  p_padding_line = p_fetched_block - (BLOCK_WIDTH * 2);
  for (i = vert_bound; i < BLOCK_HEIGHT; i++) {
    STD_MEMMOVE(p_fetched_block, p_padding_line, (BLOCK_WIDTH *
      sizeof(uint8_t) * 2));
    p_fetched_block += (BLOCK_WIDTH * 2);
    p_chroma_block  += (chroma_width * 2);
  }

  // Deinterleave chroma block
  for (i = 0; i < BLOCK_HEIGHT; i++) {
    for (j = 0; j < BLOCK_WIDTH; j++) {
      *p_deinterleaved_block1++ = fetched_block[i*BLOCK_WIDTH*2+j*2];
      *p_deinterleaved_block2++ = fetched_block[i*BLOCK_WIDTH*2+j*2+1];
    }
  }

  // Dual Format Selection
  if (p_engine->InputFormat == YCrCb) {
    p_cb_block = deinterleaved_block + JBLOCK_SIZE;
    p_cr_block = deinterleaved_block;
  } else {
    p_cb_block = deinterleaved_block;
    p_cr_block = deinterleaved_block + JBLOCK_SIZE;
  }

  // Do DCT - always in the order of CbCr
  jpege_engine_sw_fdct_block(p_cb_block, p_dct_output);
  jpege_engine_sw_fdct_block(p_cr_block, (p_dct_output + JBLOCK_SIZE));
}

/*===========================================================================
 * Function: FillDestBuffer
 *
 * Description: Fills the output buffer
 *
 * Input parameters:
 *   aForceFlush - flag to indicate whether to force flushing of buffers
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QImageSWEncoder::FillDestBuffer(bool aForceFlush)
{
  bitstream_t  *p_bitstream = &(mEngine.jpegeBitStreamState);
  uint32_t bytes_to_copy, bytes_remaining, bytes_written = 0;
  int rc;

  uint8_t *lDestAddr = NULL;;

  bytes_remaining = (p_bitstream->nextBytePtr - p_bitstream->buffer);

  while (bytes_remaining) {
    lDestAddr = mOutputImage->BaseAddr() + mOutputImage->FilledLen();

    if (mOutputImage->FilledLen() + bytes_remaining >
      mOutputImage->Length()) {
      QIDBG_ERROR("%s:%d] Output buffer insufficient %d",
        __func__, __LINE__, mOutputImage->Length());
      mEngine.error_flag = true;
      return QI_ERR_OUT_OF_BOUNDS;
    }
    // Calculate how much can be written to the destination buffer
    bytes_to_copy = STD_MIN(bytes_remaining,
      (mOutputImage->Length() - mOutputImage->FilledLen()));

    /* Copy the data from bitstream buffer to destination buffer
      starting from the offset */
    memcpy(lDestAddr, p_bitstream->buffer + bytes_written, bytes_to_copy);

    // Update destination buffer offset and other state variables
    mOutputImage->SetFilledLen(mOutputImage->FilledLen() + bytes_to_copy);
    bytes_remaining -= bytes_to_copy;
    bytes_written += bytes_to_copy;
  }

  // Reset internal bitstream buffer
  p_bitstream->nextBytePtr = p_bitstream->buffer;
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: jpege_engine_sw_configure_internal_buffer
 *
 * Description: Static function to configure internal buffer
 *
 * Input parameters:
 *   p_engine - pointer to the engine
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
static int jpege_engine_sw_configure_internal_buffer(
  jpege_engine_sw_t *p_engine)
{
  bitstream_t *p_bitstream = &(p_engine->jpegeBitStreamState);

  /* Free previously allocated internal bitstream buffer first */
  if (p_bitstream->buffer) {
    JPEG_FREE(p_bitstream->buffer);
    STD_MEMSET(p_bitstream, 0, sizeof(bitstream_t));
  }
  /* Allocate the internal bitstream buffer with size INTERNAL_BUFFER_SIZE */
  p_bitstream->buffer = (uint8_t *)JPEG_MALLOC(INTERNAL_BUFFER_SIZE *
    sizeof (uint8_t));
  if (!p_bitstream->buffer) return JPEGERR_EMALLOC;
  p_bitstream->buffersize = INTERNAL_BUFFER_SIZE;
  p_bitstream->nextBytePtr = p_bitstream->buffer;
  /* clear bit-stream assembly buffer */
  p_bitstream->bitAssemblyBufferUL32 = 0;
  /* init emptyBitLocation to 32 */
  p_bitstream->bitsFree = 32;
  return JPEGERR_SUCCESS;
}

/*===========================================================================
 * Function: jpege_engine_sw_configure_parameter
 *
 * Description: Static function to configure parameters
 *
 * Input parameters:
 *   p_engine - pointer to the engine
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
static int jpege_engine_sw_configure_parameter(jpege_engine_sw_t  *p_engine)
{
  /*-----------------------------------------------------------------------
    Calculate Chroma Width and Height
  -----------------------------------------------------------------------*/
  switch (p_engine->subsampling) {
  case JPEG_H1V1: {
    p_engine->chromaWidth = p_engine->lumaWidth;
    p_engine->chromaHeight = p_engine->lumaHeight;
    p_engine->MCUWidth = 8;
    p_engine->MCUHeight = 8;
    p_engine->numLumaBlocks = H1V1_NUM_LUMA_BLOCKS;
    break;
  }
  case JPEG_H1V2: {
    p_engine->chromaWidth = p_engine->lumaWidth;
    p_engine->chromaHeight = (p_engine->lumaHeight + 1) >> 1;//odd luma support
    p_engine->MCUWidth = 8;
    p_engine->MCUHeight = 16;
    p_engine->numLumaBlocks = H1V2_NUM_LUMA_BLOCKS;
    break;
  }
  case JPEG_H2V1: {
    p_engine->chromaWidth = (p_engine->lumaWidth + 1) >> 1; // odd luma support
    p_engine->chromaHeight = p_engine->lumaHeight;
    p_engine->MCUWidth = 16;
    p_engine->MCUHeight = 8;
    p_engine->numLumaBlocks = H2V1_NUM_LUMA_BLOCKS;
    break;
  }
  case JPEG_H2V2: {
    // odd luma support
    p_engine->chromaWidth = (p_engine->lumaWidth + 1) >> 1;
    // odd luma support
    p_engine->chromaHeight = (p_engine->lumaHeight + 1) >> 1;
    p_engine->MCUWidth = 16;
    p_engine->MCUHeight = 16;
    p_engine->numLumaBlocks = H2V2_NUM_LUMA_BLOCKS;
    break;
  }
  default:
    QIDBG_ERROR("jpege_engine_sw_configure: invalid jpeg subsampling: %d\n",
      p_engine->subsampling);
    return JPEGERR_EBADPARM;
  }

  /*-----------------------------------------------------------------------
   Calculate the horizontal and vertical MCU counts
  -----------------------------------------------------------------------*/
  // divide by 8 calculate the amounts of MCUs
  p_engine->horiMCUCount = (p_engine->chromaWidth + 7) >> 3;
  // with padding support
  p_engine->vertMCUCount = (p_engine->chromaHeight + 7) >> 3;

  /*-----------------------------------------------------------------------
    Initialize horizontal and vertical MCU indices to 0
  -----------------------------------------------------------------------*/
  p_engine->horiMCUIndex = 0;
  p_engine->vertMCUIndex = 0;

  /*-----------------------------------------------------------------------
   Reset DC prediction values
  -----------------------------------------------------------------------*/
  p_engine->prevLumaDC = 0;
  p_engine->prevCbDC = 0;
  p_engine->prevCrDC = 0;

  /*-----------------------------------------------------------------------
    Initialize Restart MCU Count and Restart Marker Mod Count
  -----------------------------------------------------------------------*/
  p_engine->restartMCUModCount = 0;
  p_engine->restartMarkerModCount = 0;

  p_engine->rotatedhoriMCUIndex = 0;
  p_engine->rotatedvertMCUIndex = 0;

  return JPEGERR_SUCCESS;
}

/*===========================================================================
 * Function: jpege_engine_sw_configure_rotation
 *
 * Description: Static function to configure rotation
 *
 * Input parameters:
 *   p_engine - pointer to the engine
 *   p_config - pointer to image config
 *
 * Return values:
 *   jpeg errot values
 *
 * Notes: none
 *==========================================================================*/
static int jpege_engine_sw_configure_rotation(jpege_engine_sw_t *p_engine,
  jpege_img_cfg_t *p_config)
{
  uint32_t lumaWidth = p_engine->lumaWidth;
  uint32_t lumaHeight = p_engine->lumaHeight;
  uint32_t chromaWidth = p_engine->chromaWidth;
  uint32_t chromaHeight = p_engine->chromaHeight;
  uint16_t *lumaQTablePtr = (uint16_t *)p_config->luma_quant_tbl;
  uint16_t *chromaQTablePtr = (uint16_t *)p_config->chroma_quant_tbl;
  const int16_t *zigzagTablePtr;
  uint16_t qFactorInvQ15;
  uint32_t i, j;

  switch (p_engine->rotation) {
  case 0: {
    /* setup real geometry parameters */
    p_engine->rotatedMCUWidth = p_engine->MCUWidth;
    p_engine->rotatedMCUHeight = p_engine->MCUHeight;
    p_engine->rotatedhoriMCUCount = (chromaWidth + 7) >> 3;  // divide by 8
    p_engine->rotatedvertMCUCount = (chromaHeight + 7) >> 3; // divide by 8
    p_engine->rotatedlumaWidth = lumaWidth;
    p_engine->rotatedlumaHeight = lumaHeight;
    p_engine->rotatedchromaWidth = chromaWidth;
    p_engine->rotatedchromaHeight = chromaHeight;
    /* setup Origin */
    p_engine->currOrigin_luma = 0;
    p_engine->currOrigin_chroma = 0;
    /* setup relative step */
    p_engine->horiIncrement_luma = 1;
    p_engine->vertIncrement_luma = lumaWidth;
    p_engine->horiIncrement_chroma = 2;
    p_engine->vertIncrement_chroma = 2 * chromaWidth;
    /* Calculate zigzag table and quant table for rotation */
    /* for 0 degree rotation, use original zigzag table */
    zigzagTablePtr = zigzagTable;
    /* for 0 degree rotation, use original quant tables */
    for (i = 0; i < 8; i++) {
      for (j = 0; j < 8; j++){
        qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j] *
          ((1<<(FIX_POINT_FACTOR-3))-1) + 0.5);
        p_engine->lumaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;
        qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j] *
          ((1<<(FIX_POINT_FACTOR-3))-1) + 0.5);
        p_engine->chromaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;
      }
    }
    break;
  }
  case 90: {
    /* setup real geometry parameters */
    p_engine->rotatedMCUWidth = p_engine->MCUHeight;
    p_engine->rotatedMCUHeight = p_engine->MCUWidth;
    p_engine->rotatedhoriMCUCount = (chromaHeight + 7) >> 3; // divide by 8
    p_engine->rotatedvertMCUCount = (chromaWidth + 7) >> 3;  // divide by 8
    p_engine->rotatedlumaWidth = lumaHeight;
    p_engine->rotatedlumaHeight = lumaWidth;
    p_engine->rotatedchromaWidth = chromaHeight;
    p_engine->rotatedchromaHeight = chromaWidth;
    /* setup Origin */
    p_engine->currOrigin_luma = (lumaHeight - BLOCK_HEIGHT) * lumaWidth;
    p_engine->currOrigin_chroma = (chromaHeight - BLOCK_HEIGHT) *
      chromaWidth * 2;
    /* setup relative step */
    p_engine->horiIncrement_luma = (int32_t)(0 - lumaWidth);
    p_engine->vertIncrement_luma = 1;
    p_engine->horiIncrement_chroma = (-2 *(int32_t)chromaWidth);
    p_engine->vertIncrement_chroma = 2;
    /* calculate zigzag table and quant table for rotation */
    /* for 90 degree rotation, transpose zigzag table */
    zigzagTablePtr = zigzagTableTranspose;
    /* for 90 degree rotation, transpose quant tables */
    for (i = 0; i < 8; i++) {
      for (j = 0; j < 8; j++) {
          qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j] *
            ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
          p_engine->lumaQuantTableQ16[j*8+i] = (int16_t)qFactorInvQ15;
          qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j] *
            ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
          p_engine->chromaQuantTableQ16[j*8+i] = (int16_t)qFactorInvQ15;
      }
    }
    /* then flip the sign of odd quant row */
    for (i = 1; i < 8; i+=2) {
      for (j = 0; j < 8; j++) {
          p_engine->lumaQuantTableQ16[i*8+j] =
            -p_engine->lumaQuantTableQ16[i*8+j];
          p_engine->chromaQuantTableQ16[i*8+j] =
            -p_engine->chromaQuantTableQ16[i*8+j];
      }
    }
    break;
  }
  case 180: {
    /* setup real geometry parameters */
    p_engine->rotatedMCUWidth = p_engine->MCUWidth;
    p_engine->rotatedMCUHeight = p_engine->MCUHeight;
    p_engine->rotatedhoriMCUCount = (chromaWidth + 7) >> 3;  // divide by 8
    p_engine->rotatedvertMCUCount = (chromaHeight + 7) >> 3; // divide by 8
    p_engine->rotatedlumaWidth = lumaWidth;
    p_engine->rotatedlumaHeight = lumaHeight;
    p_engine->rotatedchromaWidth = chromaWidth;
    p_engine->rotatedchromaHeight = chromaHeight;
    /* setup Origin */
    p_engine->currOrigin_luma = (lumaHeight - BLOCK_HEIGHT) * lumaWidth +
     (lumaWidth - BLOCK_WIDTH);
    p_engine->currOrigin_chroma =
      (chromaHeight - BLOCK_HEIGHT) * chromaWidth * 2 +
      (chromaWidth - BLOCK_WIDTH) * 2;
    /* setup relative step */
    p_engine->horiIncrement_luma   = -1;
    p_engine->vertIncrement_luma   = (int32_t)(0 - lumaWidth);
    p_engine->horiIncrement_chroma = -2;
    p_engine->vertIncrement_chroma = (-2 * (int32_t)chromaWidth);
    /* calculate zigzag table and quant table for rotation */
    /* for 180 degree rotation, use original zigzag table */
    zigzagTablePtr = zigzagTable;
    /* for 180 degree rotation, flip odd entries for quant tables */
    for (i = 0; i < 7; i+=2) {
      for (j = 0; j < 7; j+=2) {
        qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j] *
          ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
        p_engine->lumaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;
        qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j+1] *
          ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
        p_engine->lumaQuantTableQ16[i*8+j+1] = -(int16_t)qFactorInvQ15;

        qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j] *
          ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
        p_engine->chromaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;
        qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j+1] *
          ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
        p_engine->chromaQuantTableQ16[i*8+j+1] = -(int16_t)qFactorInvQ15;
      }
      for (j = 0; j < 7; j+=2) {
        qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[(i+1)*8+j] *
          ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
        p_engine->lumaQuantTableQ16[(i+1)*8+j] = -(int16_t)qFactorInvQ15;
        qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[(i+1)*8+j+1] *
          ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
        p_engine->lumaQuantTableQ16[(i+1)*8+j+1] = (int16_t)qFactorInvQ15;

        qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[(i+1)*8+j] *
          ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
        p_engine->chromaQuantTableQ16[(i+1)*8+j] = -(int16_t)qFactorInvQ15;
        qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[(i+1)*8+j+1] *
          ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
        p_engine->chromaQuantTableQ16[(i+1)*8+j+1] = (int16_t)qFactorInvQ15;
      }
    }
    break;
  }
  case 270: {
    /* setup real geometry parameters */
    p_engine->rotatedMCUWidth = p_engine->MCUHeight;
    p_engine->rotatedMCUHeight = p_engine->MCUWidth;
    p_engine->rotatedhoriMCUCount = (chromaHeight + 7) >> 3; // divide by 8
    p_engine->rotatedvertMCUCount = (chromaWidth + 7) >> 3;  // divide by 8
    p_engine->rotatedlumaWidth = lumaHeight;
    p_engine->rotatedlumaHeight = lumaWidth;
    p_engine->rotatedchromaWidth = chromaHeight;
    p_engine->rotatedchromaHeight = chromaWidth;
    /* setup Origin */
    p_engine->currOrigin_luma = (lumaWidth - BLOCK_WIDTH);
    p_engine->currOrigin_chroma = (chromaWidth - BLOCK_WIDTH) * 2;
    /* setup relative step */
    p_engine->horiIncrement_luma = lumaWidth;
    p_engine->vertIncrement_luma = -1;
    p_engine->horiIncrement_chroma = 2 * chromaWidth;
    p_engine->vertIncrement_chroma = -2;
    /* calculate zigzag table and quant table for rotation */
    /* for 270 degree rotation, transpose zigzag table */
    zigzagTablePtr = zigzagTableTranspose;
    /* for 270 degree rotation, transpose quant tables */
    for (i = 0; i < 8; i++) {
      for (j = 0; j < 8; j++) {
        qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j] *
          ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
        p_engine->lumaQuantTableQ16[j*8+i] = (int16_t)qFactorInvQ15;
        qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j] *
          ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
        p_engine->chromaQuantTableQ16[j*8+i] = (int16_t)qFactorInvQ15;
      }
    }
    /* then flip sign of odd quant col */
    for (i = 0; i < 8; i++) {
      for (j = 1; j < 8; j+=2) {
        p_engine->lumaQuantTableQ16[i*8+j] =
          -p_engine->lumaQuantTableQ16[i*8+j];
        p_engine->chromaQuantTableQ16[i*8+j] =
          -p_engine->chromaQuantTableQ16[i*8+j];
      }
    }
    break;
  }
  default:
    QIDBG_ERROR("jpege_engine_sw_configure: unrecognized rotation: %d\n",
      p_engine->rotation);
    return JPEGERR_EBADPARM;
  }

  /* Calculate the zigzagOffsetTable, the offset is the relative BYTES !!!
   * between DCTOutput[i] and DCTOutput[i+1] (which is int16_t), this is due to
   * the limitation of ARM LDRSH instruction which does not allow <opsh>, like:
   *
   * LDRSH Rd, [Rn], Rm, LSL #1
   *
   * and this zigzagOffsetTable is scanned decrementally, that is from
   * 63 down to 0.
   */
  for (i = JBLOCK_SIZE_MINUS_1; i != 0; i--) {
    p_engine->zigzagOffsetTable[i] =
      (zigzagTablePtr[i - 1] - zigzagTablePtr[i]) * sizeof(int16_t);
  }
  p_engine->zigzagOffsetTable[0] = 0;

  return JPEGERR_SUCCESS;
}

/*===========================================================================
 * Function: jpege_engine_sw_configure_huff_tables
 *
 * Description: Static function to configure huffman tables
 *
 * Input parameters:
 *   p_engine - pointer to the engine
 *   p_config - pointer to image config
 *
 * Return values:
 *   jpeg error values
 *
 * Notes: none
 *==========================================================================*/
static void jpege_engine_sw_configure_huff_tables(jpege_engine_sw_t *p_engine,
  jpege_img_cfg_t    *p_config)
{
  jpeg_huff_table_t *tempHuffTablePtr;
  uint8_t *tempSrcPtr, *tempDestPtr;
  uint16_t j;
  /*-----------------------------------------------------------------------
    Copy the Huffman Tables
  -----------------------------------------------------------------------*/
  /* Luma DC */
  tempHuffTablePtr = &(p_config->luma_dc_huff_tbl);
  tempSrcPtr = tempHuffTablePtr->bits;
  tempDestPtr = p_engine->jpegeLumaBitsValTables.dcBits;
  for (j=1; j<=16; j++)
    tempDestPtr[j-1] = tempSrcPtr[j];

  tempDestPtr = p_engine->jpegeLumaBitsValTables.dcHuffVal;
  tempSrcPtr = tempHuffTablePtr->values;
  for (j=0; j<12; j++)
    tempDestPtr[j] = tempSrcPtr[j];

  /* Luma AC */
  tempHuffTablePtr = &(p_config->luma_ac_huff_tbl);
  tempSrcPtr = tempHuffTablePtr->bits;
  tempDestPtr = p_engine->jpegeLumaBitsValTables.acBits;
  for (j=1; j<=16; j++)
    tempDestPtr[j-1] = tempSrcPtr[j];

  tempDestPtr = p_engine->jpegeLumaBitsValTables.acHuffVal;
  tempSrcPtr = tempHuffTablePtr->values;
  for (j=0; j<162; j++)
    tempDestPtr[j] = tempSrcPtr[j];

  /* Chroma DC */
  tempHuffTablePtr = &(p_config->chroma_dc_huff_tbl);
  tempSrcPtr = tempHuffTablePtr->bits;
  tempDestPtr = p_engine->jpegeChromaBitsValTables.dcBits;
  for (j=1; j<=16; j++)
    tempDestPtr[j-1] = tempSrcPtr[j];

  tempSrcPtr = tempHuffTablePtr->values;
  tempDestPtr = p_engine->jpegeChromaBitsValTables.dcHuffVal;
  for (j=0; j<12; j++)
      tempDestPtr[j] = tempSrcPtr[j];

  /* Chroma AC */
  tempHuffTablePtr = &(p_config->chroma_ac_huff_tbl);
  tempSrcPtr = tempHuffTablePtr->bits;
  tempDestPtr = p_engine->jpegeChromaBitsValTables.acBits;
  for (j=1; j<=16; j++)
    tempDestPtr[j-1] = tempSrcPtr[j];

  tempSrcPtr = tempHuffTablePtr->values;
  tempDestPtr = p_engine->jpegeChromaBitsValTables.acHuffVal;
  for (j=0; j<162; j++)
    tempDestPtr[j] = tempSrcPtr[j];
}

/*===========================================================================
 * Function: QISWOutputThread
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
QISWOutputThread::QISWOutputThread(QImageSWEncoder &aEncoder)
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
void QISWOutputThread::run(QThreadObject *aData)
{
  /* TODO: for piecewise output*/
  QIDBG_ERROR("%s:%d] ", __func__, __LINE__);
}

/*===========================================================================
 * Function: QISWEncoderThread
 *
 * Description: QISWEncoderThread constructor
 *
 * Input parameters:
 *   aData - reference to thread object
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QISWEncoderThread::QISWEncoderThread(QImageSWEncoder &aEncoder)
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
void QISWEncoderThread::run(QThreadObject *aData)
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
void QImageSWEncoder::Encode()
{
  uint32_t currMCUIndex, totalMCUCount;
  int16_t *currDctOutputPtr;
  int16_t zigzagOutput[JBLOCK_SIZE];
  uint32_t numLumaBlocks = mEngine.numLumaBlocks;
  int16_t *currMCULumaDctOutputPtr = mEngine.currMCULumaDctOutput;
  int16_t *currMCUChromaDctOutputPtr = mEngine.currMCUChromaDctOutput;
  bitstream_t *p_bitstream = &(mEngine.jpegeBitStreamState);
  uint8_t *p_bound = p_bitstream->buffer +
    (p_bitstream->buffersize - MAX_MCU_SIZE);
  uint32_t i;

  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  /* Initialize Huffman lookup tables */
  jpege_engine_sw_huff_init_tables(&mEngine);

  /* Set totalMCUCount = horiMCUCount * vertMCUCount*/
  totalMCUCount = mEngine.horiMCUCount * mEngine.vertMCUCount;
  QIDBG_LOW("%s:%d] totalMCUCount %d", __func__, __LINE__, totalMCUCount);

  /*------------------------------------------------------------
    MCU processing loop. Process 1 MCU per iteration.
  ------------------------------------------------------------*/
  for (currMCUIndex = 0; currMCUIndex < totalMCUCount; currMCUIndex++) {

    QIDBG_LOW("%s:%d] currMCUIndex %d", __func__, __LINE__, currMCUIndex);
    /* Check for cancel or error */
    QI_LOCK(&mMutex);
    if (mState == ESTATE_STOP_REQUESTED) {
      QIDBG_MED("%s:%d] Stop called", __func__, __LINE__);
      goto stop;
    } else if (mEngine.error_flag) {
      mEngine.error_flag = false;
      QI_UNLOCK(&(mMutex));
      // Throw encode error event
      for (uint32_t k = 0; k < mObserverCnt; k++)
        mObserver[k]->EncodeError(QImageEncoderObserver::ERROR_GENERAL);
      goto stop;
    }
    QI_UNLOCK(&mMutex);

    /* Restart Interval Processing, if necessary */
    if ((mEngine.restartMCUModCount == mEngine.restartInterval) &&
      (mEngine.restartInterval != 0)) {
      mEngine.restartMCUModCount = 0;
      jpege_engine_sw_insert_restart_marker(&mEngine);
    }
    mEngine.restartMCUModCount++;

    /* Fetch MCU from the input image. Do DCT */
    (*(mEngine.jpege_engine_sw_fetch_dct_mcu_func))(&mEngine,
      currMCULumaDctOutputPtr,
      currMCUChromaDctOutputPtr);

    /*--------------------------------------------------------
      Loop to process Luma Blocks. Do Quant, Zigzag and
      Entropy Encode
    --------------------------------------------------------*/
    currDctOutputPtr = currMCULumaDctOutputPtr;
    for (i = 0; i < numLumaBlocks; i++)
    {
      /* Do Quant and Zigzag */
      jpege_engine_sw_quant_zigzag (
        currDctOutputPtr,
        zigzagOutput,
        mEngine.zigzagOffsetTable,
        mEngine.lumaQuantTableQ16);

      /* Do Baseline Huffman Encode */
      jpege_engine_sw_huff_encode (&mEngine, zigzagOutput, LUMA);
      currDctOutputPtr += JBLOCK_SIZE;
    }

    /*--------------------------------------------------------
      Cb: Do Quant, Zigzag and Entropy Encode
    --------------------------------------------------------*/
    /* Do Quant and Zigzag */
    jpege_engine_sw_quant_zigzag (
      currMCUChromaDctOutputPtr,
      zigzagOutput,
      mEngine.zigzagOffsetTable,
      mEngine.chromaQuantTableQ16);

    /* Do Baseline Huffman Encode */
    jpege_engine_sw_huff_encode (&mEngine, zigzagOutput, CB);

    /*--------------------------------------------------------
     Cr: Do Quant, Zigzag and Entropy Encode
    --------------------------------------------------------*/
    /* Do Quant and Zigzag */
    jpege_engine_sw_quant_zigzag (
      (currMCUChromaDctOutputPtr + JBLOCK_SIZE),
      zigzagOutput,
      mEngine.zigzagOffsetTable,
      mEngine.chromaQuantTableQ16);

    /* Do Baseline Huffman Encode */
    jpege_engine_sw_huff_encode (&mEngine, zigzagOutput, CR);

    /*--------------------------------------------------------
      If this is the last MCU block of the image,
     last residue bits should be stuffed by 1s to form a byte.
    --------------------------------------------------------*/
    if (currMCUIndex == (totalMCUCount - 1)) {
        jpege_engine_sw_stuff_bs(p_bitstream);
    }

    /* Update horizontal and vertical indices for the next MCU */
    mEngine.rotatedhoriMCUIndex ++;
    if (mEngine.rotatedhoriMCUIndex == mEngine.rotatedhoriMCUCount) {
      mEngine.rotatedhoriMCUIndex = 0;
      mEngine.rotatedvertMCUIndex++;
    }
    /* Check if the upper bound of internal bitstream buffer is reached */
    if (p_bitstream->nextBytePtr > p_bound) {
      /* Copy the bitstream to output buffer */
      FillDestBuffer(false); // Do not force flushing
    }
  }// End MCU processing loop

  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  /* Check for cancel or error */
  QI_LOCK(&mMutex);
  if (mState == ESTATE_STOP_REQUESTED) {
    QIDBG_MED("%s:%d] Stop called", __func__, __LINE__);
    goto stop;
  } else if (mEngine.error_flag) {
    mEngine.error_flag = false;
    QI_UNLOCK(&(mMutex));
    // Throw encode error event
    for (uint32_t k = 0; k < mObserverCnt; k++)
      mObserver[k]->EncodeError(QImageEncoderObserver::ERROR_GENERAL);
    goto stop;
  }
  QI_UNLOCK(&mMutex);

  // finish reset of encoding if not error or abort yet
  // Emit EOI
  *(p_bitstream->nextBytePtr++) = 0xFF;
  *(p_bitstream->nextBytePtr++) = 0xD9;

  FillDestBuffer(true);

  /* Check for cancel or error */
  QI_LOCK(&mMutex);
  if (mState == ESTATE_STOP_REQUESTED) {
    QIDBG_MED("%s:%d] Stop called", __func__, __LINE__);
    goto stop;
  } else if (mEngine.error_flag) {
    mEngine.error_flag = false;
    mState = ESTATE_IDLE;
    QI_UNLOCK(&(mMutex));
    // Throw encode error event
    for (uint32_t k = 0; k < mObserverCnt; k++)
      mObserver[k]->EncodeError(QImageEncoderObserver::ERROR_GENERAL);

    return;
  }
  mState = ESTATE_IDLE;
  QI_UNLOCK(&mMutex);

  QIDBG_MED("%s:%d] size %d", __func__, __LINE__, mOutputImage->FilledLen());
  for (uint32_t k = 0; k < mObserverCnt; k++)
    mObserver[k]->EncodeComplete(mOutputImage);

  return;

stop:
  mState = ESTATE_IDLE;
  QI_SIGNAL(&mCond);
  QI_UNLOCK(&mMutex);
  return;
}

/*===========================================================================
 * Function: ReleaseSession
 *
 * Description: Release the current encoding session
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QImageSWEncoder::ReleaseSession()
{
  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, mState);
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
  // Destroy the scale internal buffers
  jpege_engine_sw_scale_destroy(&(mEngine.jpege_scale));

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
