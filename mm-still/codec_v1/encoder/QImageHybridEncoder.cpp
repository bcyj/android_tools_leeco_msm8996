/*****************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential. *
*****************************************************************************/

#include "QImageHybridEncoder.h"
#include "QIParams.h"

extern "C" {
#include "jpege.h"
#include "jpegerr.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "jpege.h"
#include "jpegd.h"
#include "jpeg_buffer.h"
#include "os_thread.h"
#include "os_timer.h"
#include "jpeg_buffer_private.h"
#include "fastcv/fastcv.h"
#include "QIHeapBuffer.h"
#ifdef JPEG_USE_QDSP6_ENCODER
#include "apr_pmem.h"
#include "remote.h"
#pragma weak  remote_register_buf
#endif

static void register_buf(void* buf, int size, int fd) {
#ifdef JPEG_USE_QDSP6_ENCODER
  if(remote_register_buf) {
    QIDBG_ERROR("%s:%d] remote_register_buf buf_addr 0x%x size %d fd %d",
      __func__, __LINE__, (unsigned int)buf, size, fd);
    remote_register_buf(buf, size, fd);
  }
#endif
}

#if (defined WIN32 || defined WINCE)
#include "wingetopt.h"
#else
#include <getopt.h>
#endif

#ifdef USE_PERFORMANCE_LOCK
#include "performance.h"
#include <sys/time.h>
#include "QIPerf.h"

extern int perf_lock_acq(int, int, int[], int);
extern int perf_lock_rel(int);
#define JPEG_ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* Static constants */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
#define MAX_UPSCALE_SHIFT  5   // support up to 32 upscale
#define MAX_DOWNSCALE_SHIFT  5 // support up to 1/32 downscale

} // end extern "C"

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* Function prototypes of helper functions */
extern "C" {
extern int hybrid_jpege_start(void *mHybridObject, void *arg,
  int* size, jpege_cfg_t *config, uint8_t *fastCV_buffer);
} // extern "C"

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
bool QImageHybridEncoder::IsAvailable(QIEncodeParams& aEncodeParams)
{
  bool lUpScale = false, lScaleEnabled = false;
  QISize lInputSize = aEncodeParams.InputSize();

  QIDBG_LOW("%s:%d] Entered...", __func__, __LINE__);

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
    || (aEncodeParams.OutputSize().Height() != lInputSize.Height());

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

  mScaleEnabled = lScaleEnabled;

  /* populate the scale structure*/
  if (lScaleEnabled&& !mUseFastCv) {
    mHybridConfig.main_scale_cfg.enable = 1;
    mHybridConfig.main_scale_cfg.input_width =
      aEncodeParams.Crop().Width();
    mHybridConfig.main_scale_cfg.input_height =
      aEncodeParams.Crop().Height();
    mHybridConfig.main_scale_cfg.output_width =
      aEncodeParams.OutputSize().Width();
    mHybridConfig.main_scale_cfg.output_height =
      aEncodeParams.OutputSize().Height();
    mHybridConfig.main_scale_cfg.h_offset =
      aEncodeParams.Crop().Left();
    mHybridConfig.main_scale_cfg.v_offset =
      aEncodeParams.Crop().Top();

    QIDBG_ERROR("%s:%d] scale input %dx%d crop (%d, %d) output %dx%d rotation %d",
      __func__, __LINE__,
      mHybridConfig.main_scale_cfg.input_width,
      mHybridConfig.main_scale_cfg.input_height ,
      mHybridConfig.main_scale_cfg.h_offset,
      mHybridConfig.main_scale_cfg.v_offset,
      mHybridConfig.main_scale_cfg.output_width,
      mHybridConfig.main_scale_cfg.output_height,
      aEncodeParams.Rotation());
  } else {
    mHybridConfig.main_scale_cfg.enable = 0;

    if (lScaleEnabled && mUseFastCv) {
      uint16_t lInWidth = lInputSize.Width();
      uint16_t lInHeight = lInputSize.Height();
      if (!aEncodeParams.Crop().isZero()) {
        uint16_t lInWidth = aEncodeParams.Crop().Width();
        uint16_t lInHeight = aEncodeParams.Crop().Height();
      }
      mScaleFactorH = (float) aEncodeParams.OutputSize().Height() / lInHeight;
      mScaleFactorW = (float) aEncodeParams.OutputSize().Width() / lInWidth;
      mRszOutSize.setWidth(((uint16_t)(lInputSize.Width() * mScaleFactorW) +
          7) & ~7);
      mRszOutSize.setHeight(((uint16_t)(lInputSize.Height() * mScaleFactorH) +
          7) & ~7);

      if (!aEncodeParams.Crop().isZero()) {
        mHybridConfig.main_scale_cfg.enable = 1;
        mHybridConfig.main_scale_cfg.input_width =
          aEncodeParams.Crop().Width();
        mHybridConfig.main_scale_cfg.input_height =
          aEncodeParams.Crop().Height();
        mHybridConfig.main_scale_cfg.output_width =
          aEncodeParams.OutputSize().Width();
        mHybridConfig.main_scale_cfg.output_height =
          aEncodeParams.OutputSize().Height();
        mHybridConfig.main_scale_cfg.h_offset =
          aEncodeParams.Crop().Left();
        mHybridConfig.main_scale_cfg.v_offset =
          aEncodeParams.Crop().Top();
      }
    }
  }
  QIDBG_LOW("%s:%d] Exited...", __func__, __LINE__);
  return true;
}

/*===========================================================================
 * Function: New
 *
 * Description: 2 phase constructor for QImageHybridEncoder
 *
 * Input parameters:
 *   aParams - encoder params
 *
 * Return values:
 *   encoder interface pointer
 *
 * Notes: none
 *==========================================================================*/
QImageEncoderInterface* QImageHybridEncoder::New(QIEncodeParams &aParams)
{
  QIDBG_LOW("%s:%d] Entered...", __func__, __LINE__);
  QImageHybridEncoder* lEncoder = new QImageHybridEncoder();
  if (NULL == lEncoder) {
    QIDBG_ERROR("%s:%d] no memory", __func__, __LINE__);
    return NULL;
  }
  if (!lEncoder->IsAvailable(aParams)) {
    QIDBG_ERROR("%s:%d] validation failed", __func__, __LINE__);
    delete lEncoder;
    return NULL;
  }
  QIDBG_LOW("%s:%d] Exited... 0x%x", __func__, __LINE__, (unsigned int)lEncoder);
  return lEncoder;
}

/*===========================================================================
 * Function: QImageHybridEncoder
 *
 * Description: QImageHybridEncoder constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageHybridEncoder::QImageHybridEncoder()
{
  // Initialize the fields inside the engine structure below
  QIDBG_LOW("%s:%d] Entered...", __func__, __LINE__);
  memset(&mEngine, 0, sizeof(jpege_engine_sw_t));
  memset(&mHybridObject, 0, sizeof(jpege_obj_t));
  memset(&mThreadCtrlBlk, 0, sizeof(thread_ctrl_blk_t));
  memset(&mHybridConfig, 0, sizeof(test_args_t));
  memset(&mSource, 0, sizeof(jpege_img_data_t));
  memset(&mConfig, 0, sizeof(jpege_cfg_t));
  memset(&mImgSource, 0, sizeof(jpege_src_t));
  memset(&mImgInfo, 0, sizeof(jpege_img_data_t));
  memset(&mDest, 0, sizeof(jpege_dst_t));
  memset(&mBuffers[0], 0, sizeof(jpeg_buffer_t));
  memset(&mBuffers[1], 0, sizeof(jpeg_buffer_t));
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
  // Set only one flag from mUseFastCv, mUseMultiFastCV as true
  mUseFastCv = false;
  mUseMultiFastCV = true;
  mRszOutImage = NULL;
  mRszOutBuffer = NULL;
  mfastCVbuffer = NULL;
#ifdef USE_PERFORMANCE_LOCK
  mPerfLibHandle = img_perf_handle_create();
  mPerfLockHandle = NULL;
#endif
  QIDBG_LOW("%s:%d] Exited...", __func__, __LINE__);
}

/*===========================================================================
 * Function: ~QImageHybridEncoder
 *
 * Description: QImageHybridEncoder destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageHybridEncoder::~QImageHybridEncoder()
{
  jpege_obj_t *p_encoder = (jpege_obj_t *)&mHybridObject;

  QIDBG_LOW("%s:%d] Entered...", __func__, __LINE__);
  Stop();
  QI_LOCK(&(mMutex));
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
  (void)os_mutex_destroy(&mThreadCtrlBlk.output_handler_args.mutex);
  (void)os_cond_destroy(&mThreadCtrlBlk.output_handler_args.cond);
  jpeg_buffer_destroy(&mBuffers[0]);
  jpeg_buffer_destroy(&mBuffers[1]);

  // Clean up encoder
  jpege_destroy(p_encoder);

  if (mRszOutImage) {
    delete mRszOutImage;
    mRszOutImage = NULL;
  }

  if (mRszOutBuffer) {
    delete mRszOutBuffer;
    mRszOutBuffer = NULL;
  }
  if(mUseMultiFastCV) {
    JPEG_FREE(mfastCVbuffer);
    mfastCVbuffer = NULL;
  }
  QI_UNLOCK(&(mMutex));
#ifdef USE_PERFORMANCE_LOCK
  //Release the performance lock
  if (mPerfLockHandle != NULL) {
    img_perf_lock_end(mPerfLibHandle, mPerfLockHandle);
  }
  img_perf_handle_destroy(mPerfLibHandle);
#endif

  QI_MUTEX_DESTROY(&mMutex);
  QI_COND_DESTROY(&mCond);

  QIDBG_LOW("%s:%d] Exited...", __func__, __LINE__);
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
int QImageHybridEncoder::SetOutputMode(QIOutputMode aMode)
{
  QIDBG_LOW("%s:%d] Entered...", __func__, __LINE__);
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
  QIDBG_LOW("%s:%d] Exited...", __func__, __LINE__);
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
int QImageHybridEncoder::Start()
{
  int lrc = QI_SUCCESS;

  QIDBG_LOW("%s:%d] Entered...", __func__, __LINE__);
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
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  if (mInputImage->SubSampling() != QI_H2V2) {
    QIDBG_ERROR("%s:%d] Non-H2V2 formats are not supported !!", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  mEngine.abort_flag = false;
  mEngine.error_flag = false;

  // Create output thread
  if (mMode == EPIECEWISE_OUTPUT) {
    mpOutputThread = new QIHybridOutputThread(*this);
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

  mpEncoderThread = new QIHybridEncoderThread(*this);
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
  QIDBG_LOW("%s:%d] Exited...", __func__, __LINE__);
  return QI_SUCCESS;
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
int QImageHybridEncoder::copyHuffTable(jpeg_huff_table_t *ap_htable,
  QIHuffTable::QHuffTableType aType)
{
  QIDBG_LOW("%s:%d] Entered...", __func__, __LINE__);
  QIHuffTable *lQHuffTbl = mEncodeParams->HuffTable(aType);
  const QIHuffTable::HuffTable *lHuffTbl = NULL;
  if (NULL == lQHuffTbl) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NOT_FOUND;
  }
  lHuffTbl = lQHuffTbl->Table();
  memcpy(ap_htable->bits, lHuffTbl->mBits, HUFF_BITS);
  memcpy(ap_htable->values, lHuffTbl->mValues, HUFF_VALUES);

  QIDBG_LOW("%s:%d] Exited...", __func__, __LINE__);
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
int QImageHybridEncoder::setEncodeParams(QIEncodeParams &aParams)
{
  int lrc = QI_SUCCESS;
  QIQuantTable *lQTable = NULL;

  QIDBG_LOW("%s:%d] Entered...", __func__, __LINE__);

  QI_LOCK(&(mMutex));
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }
  mEncodeParams = &aParams;
  mHybridConfig.rotation = mEncodeParams->Rotation();
  mHybridConfig.main.quality = mEncodeParams->Quality();
  mHybridConfig.restart_interval = mEncodeParams->RestartInterval();
  QIDBG_ERROR("%s:%d] Rotation %d Quality %d", __func__, __LINE__,
    mEncodeParams->Rotation(), mEncodeParams->Quality());
  QI_UNLOCK(&mMutex);
  QIDBG_LOW("%s:%d] Exited...", __func__, __LINE__);
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
void QImageHybridEncoder::ConfigureDimensions()
{
  int lrc = QI_SUCCESS;
  bool lCropEnabled = false, lScaleEnabled = false;
  QISize lInputSize = mEncodeParams->InputSize();

  QIDBG_HIGH("%s:%d] dim pad %dx%d orig %dx%d", __func__, __LINE__,
  mInputImage->Size().Width(),
  mInputImage->Size().Height(),
  mInputImage->ActualSize().Width(),
  mInputImage->ActualSize().Height());

  mHybridConfig.back_to_back_count = 1; // Run only one snapshot
  mHybridConfig.output_file = NULL;     // No need to dump to file
  mHybridConfig.main.file_name = NULL;
  mHybridConfig.target_filesize = 0;    // Disable file size control
  mHybridConfig.output_buf_size = 0;
  mHybridConfig.abort_time = 0;         // Disable abort time feature
  mHybridConfig.reference_file = NULL;  // No reference file for comparison
  mHybridConfig.use_pmem = true;        // Never used
  mHybridConfig.output_nowrite = 0;     // Should be disabled

  mHybridConfig.main.memory_format = NV12;     // NV12 by default

  if (mInputImage->SubSampling() == QI_H2V2) {
    mHybridConfig.main.format = (mInputImage->Format() == QI_YCBCR_SP) ?
      YCBCRLP_H2V2 : YCRCBLP_H2V2;
  } else if (mInputImage->SubSampling() == QI_H2V1) {
    mHybridConfig.main.format = (mInputImage->Format() == QI_YCBCR_SP) ?
      YCBCRLP_H2V1 : YCRCBLP_H2V1;
  } else if (mInputImage->SubSampling() == QI_H1V2) {
    mHybridConfig.main.format = (mInputImage->Format() == QI_YCBCR_SP) ?
      YCBCRLP_H1V2 : YCRCBLP_H1V2;
  } else {
    mHybridConfig.main.format = (mInputImage->Format() == QI_YCBCR_SP) ?
      YCBCRLP_H1V1 : YCRCBLP_H1V1;
  }

  mHybridConfig.preference = 3;                // Run in software only mode

  if (mScaleEnabled && mUseFastCv) {
    mHybridConfig.main.width  = mRszOutSize.Width();
    mHybridConfig.main.height = mRszOutSize.Height();
  } else {
    mHybridConfig.main.width  = mInputImage->ActualSize().Width();
    mHybridConfig.main.height = mInputImage->ActualSize().Height();
    if (mInputImage->ActualSize().Width() != mInputImage->Size().Width()) {
      mHybridConfig.main_stride_cfg.enable = true;
      mHybridConfig.main_stride_cfg.input_luma_stride = mInputImage->Size().Width();
      mHybridConfig.main_stride_cfg.input_chroma_stride = mInputImage->Size().Width();
      mHybridConfig.main_stride_cfg.output_luma_stride = 0; // Not applicable
      mHybridConfig.main_stride_cfg.output_chroma_stride = 0; // Not applicable
    }
  }
  mHybridConfig.encode_thumbnail = false;         // Disable thumbnail
  mHybridConfig.thumbnail.width = 0;
  mHybridConfig.thumbnail.file_name = NULL;
  mHybridConfig.thumbnail.quality = 0;            // Quality = 0 by default
  mHybridConfig.thumbnail.height = 0;
  mHybridConfig.thumbnail.format = YCRCBLP_H2V2;  // Thumbnail format
  mHybridConfig.tn_scale_cfg.enable = false;
  mHybridConfig.tn_scale_cfg.input_width = 0;
  mHybridConfig.tn_scale_cfg.input_height = 0;
  mHybridConfig.tn_scale_cfg.output_width = 0;
  mHybridConfig.tn_scale_cfg.output_height = 0;
  mHybridConfig.tn_scale_cfg.h_offset = 0;
  mHybridConfig.tn_scale_cfg.v_offset = 0;

  lScaleEnabled = (mEncodeParams->OutputSize().Width() != lInputSize.Width())
    || (mEncodeParams->OutputSize().Height() != lInputSize.Height() ||
    mEncodeParams->Crop().Left() || mEncodeParams->Crop().Top());

  lCropEnabled = mEncodeParams->Crop().Left() || mEncodeParams->Crop().Top();

  if ((mEncodeParams->Crop().Left() != 0 ||
        mEncodeParams->Crop().Top() != 0 ||
    ((mEncodeParams->Crop().Left() + mEncodeParams->Crop().Width()) <
          mInputImage->ActualSize().Width()) ||
    ((mEncodeParams->Crop().Top() + mEncodeParams->Crop().Height()) <
          mInputImage->ActualSize().Height())) &&
    (mEncodeParams->Crop().Width() != mInputImage->Size().Width() ||
     mEncodeParams->Crop().Height() != mInputImage->Size().Height()))
  {
    lCropEnabled = true;
  }

  if (lCropEnabled) {
    QIDBG_ERROR("%s:%d] Crop enabled so disabling multi threaded FastCV", __func__, __LINE__);
    mUseMultiFastCV = false;
  }

  /* populate the scale structure*/
  if (lScaleEnabled && !mUseFastCv) {
    mHybridConfig.main_scale_cfg.enable = 1;
    mHybridConfig.main_scale_cfg.input_width =
      mEncodeParams->Crop().Width();
    mHybridConfig.main_scale_cfg.input_height =
      mEncodeParams->Crop().Height();
    mHybridConfig.main_scale_cfg.output_width =
      mEncodeParams->OutputSize().Width();
    mHybridConfig.main_scale_cfg.output_height =
      mEncodeParams->OutputSize().Height();
    mHybridConfig.main_scale_cfg.h_offset =
      mEncodeParams->Crop().Left();
    mHybridConfig.main_scale_cfg.v_offset =
      mEncodeParams->Crop().Top();
  } else {
    mHybridConfig.main_scale_cfg.enable = 0;
    if (lScaleEnabled && mUseFastCv) {
      uint16_t lInWidth = lInputSize.Width();
      uint16_t lInHeight = lInputSize.Height();
      if (!mEncodeParams->Crop().isZero()) {
        uint16_t lInWidth = mEncodeParams->Crop().Width();
        uint16_t lInHeight = mEncodeParams->Crop().Height();
      }
      mScaleFactorH = (float) mEncodeParams->OutputSize().Height() / lInHeight;
      mScaleFactorW = (float) mEncodeParams->OutputSize().Width() / lInWidth;
      mRszOutSize.setWidth(((uint16_t)(lInputSize.Width() * mScaleFactorW) +
          7) & ~7);
      mRszOutSize.setHeight(((uint16_t)(lInputSize.Height() * mScaleFactorH) +
          7) & ~7);

      if (!mEncodeParams->Crop().isZero()) {
        mHybridConfig.main_scale_cfg.enable = 1;
        mHybridConfig.main_scale_cfg.input_width =
          mEncodeParams->Crop().Width();
        mHybridConfig.main_scale_cfg.input_height =
          mEncodeParams->Crop().Height();
        mHybridConfig.main_scale_cfg.output_width =
          mEncodeParams->OutputSize().Width();
        mHybridConfig.main_scale_cfg.output_height =
          mEncodeParams->OutputSize().Height();
        mHybridConfig.main_scale_cfg.h_offset =
          mEncodeParams->Crop().Left();
        mHybridConfig.main_scale_cfg.v_offset =
          mEncodeParams->Crop().Top();
      }
    }
  }

  QIDBG_LOW("%s:%d] Exited...", __func__, __LINE__);
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
int QImageHybridEncoder::addInputImage(QImage &aImage)
{
  int lrc = QI_SUCCESS;
  jpege_obj_t       *p_encoder;
  thread_ctrl_blk_t *p_thread_arg;
  test_args_t *p_args;
  jpege_obj_t encoder;
  QImage *IEncInImage;
  uint32_t  buffer_size;
#ifdef USE_PERFORMANCE_LOCK
  //Performance lock to turbo
  int32_t perf_lock_params[5] = { CPUS_ONLINE_MIN_4, CPU0_MIN_FREQ_TURBO_MAX,
    CPU1_MIN_FREQ_TURBO_MAX ,CPU2_MIN_FREQ_TURBO_MAX,CPU3_MIN_FREQ_TURBO_MAX};
#endif


  QIDBG_LOW("%s:%d] Entered ... state %d", __func__, __LINE__, mState);

  QI_LOCK(&(mMutex));
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }

  mInputImage = &aImage;

  ConfigureDimensions();
  IEncInImage = mInputImage;
  if (mScaleEnabled && mUseFastCv) {
    uint32_t lRszOutSz = QImage::getImageSize(mRszOutSize,
        mInputImage->SubSampling(), mInputImage->Format());
    QIDBG_MED("%s:%d] lRszOutSz = %d", __func__, __LINE__, lRszOutSz);
    uint32_t lRszOutSzPad = (lRszOutSz + 16);
    mRszOutBuffer = QIHeapBuffer::New(lRszOutSzPad);
    if (mRszOutBuffer == NULL) {
      QI_UNLOCK(&(mMutex));
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return QI_ERR_GENERAL;
    }
    uint32_t lOutAddr = ((uint32_t)mRszOutBuffer->Addr() + 15) & ~15;

    mRszOutImage = new QImage(mRszOutSize, mInputImage->SubSampling(),
        mInputImage->Format(),
        mRszOutSize);

    if (mRszOutImage == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      QI_UNLOCK(&(mMutex));
      return QI_ERR_GENERAL;
    }
    lrc = mRszOutImage->setDefaultPlanes(
        mInputImage->GetPlaneCount(mInputImage->Format()),
        (uint8_t *)lOutAddr,
        mRszOutBuffer->Fd());
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      QI_UNLOCK(&(mMutex));
      return QI_ERR_GENERAL;
    }

    IEncInImage = mRszOutImage;
  }
  buffer_size = mHybridConfig.main_scale_cfg.output_width * mHybridConfig.main_scale_cfg.output_height * 1.5;
  if(mUseMultiFastCV) {
   mfastCVbuffer = (uint8_t *)JPEG_MALLOC(buffer_size * sizeof (uint8_t));
   if (mfastCVbuffer == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      QI_UNLOCK(&(mMutex));
      return QI_ERR_GENERAL;
    }
  }
#ifdef USE_PERFORMANCE_LOCK
  if (mPerfLockHandle != NULL) {
    mPerfLockHandle = img_perf_lock_start(mPerfLockHandle, perf_lock_params,
      JPEG_ARRAY_SIZE(perf_lock_params), 1000);
  }
#endif

  p_thread_arg = &mThreadCtrlBlk;
  p_encoder    = &mHybridObject;
  p_thread_arg->tid    = 0;
  p_thread_arg->p_args = &mHybridConfig;
  os_mutex_init(&p_thread_arg->output_handler_args.mutex);
  os_cond_init(&p_thread_arg->output_handler_args.cond);

  // Initialize encoder
  lrc = jpege_init(p_encoder, &encoder_event_handler, (void *)p_thread_arg);

  if (JPEG_FAILED(lrc)) {
    QIDBG_ERROR("%s:%d] failed \n", __func__, __LINE__ );
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }
  p_thread_arg->encoder = (jpege_obj_t)(*(p_encoder));

  if (mInputImage->SubSampling() == QI_H2V2) {
    mSource.color_format = (IEncInImage->Format() == QI_YCBCR_SP) ?
      YCBCRLP_H2V2 : YCRCBLP_H2V2;
  } else if (IEncInImage->SubSampling() == QI_H2V1) {
    mSource.color_format = (IEncInImage->Format() == QI_YCBCR_SP) ?
      YCBCRLP_H2V1 : YCRCBLP_H2V1;
  } else if (IEncInImage->SubSampling() == QI_H1V2) {
    mSource.color_format = (IEncInImage->Format() == QI_YCBCR_SP) ?
      YCBCRLP_H1V2 : YCRCBLP_H1V2;
  } else {
    mSource.color_format = (IEncInImage->Format() == QI_YCBCR_SP) ?
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

  QIPlane *lPlane = IEncInImage->getPlane(QIPlane::PLANE_Y);
  if (NULL == lPlane) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  QIDBG_ERROR("%s:%d] Y addr %p len %d fd %d", __func__, __LINE__,
    lPlane->ActualAddr(), lPlane->Length(), lPlane->Fd());
  QI_ERROR_RET_UNLOCK(jpeg_buffer_use_external_buffer(
    mSource.p_fragments[0].color.yuv.luma_buf,
    lPlane->ActualAddr(),
    lPlane->Length(),
    lPlane->Fd()),
    &mMutex);
  jpeg_buffer_set_actual_size(mSource.p_fragments[0].color.yuv.luma_buf,
    lPlane->ActualSize().Length());

  lPlane = IEncInImage->getPlane(QIPlane::PLANE_CB_CR);
  if (NULL == lPlane) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_GENERAL;
  }

  QIDBG_ERROR("%s:%d] CbCr addr %p len %d fd %d", __func__, __LINE__,
    lPlane->ActualAddr(), lPlane->Length(), lPlane->Fd());
  QI_ERROR_RET_UNLOCK(jpeg_buffer_use_external_buffer(
    mSource.p_fragments[0].color.yuv.chroma_buf,
    lPlane->ActualAddr(),
    lPlane->Length(),
    lPlane->Fd()),
    &mMutex);

  jpeg_buffer_set_actual_size(mSource.p_fragments[0].color.yuv.chroma_buf,
    lPlane->ActualSize().Length());

  encoder = (jpege_obj_t)*(p_encoder);
  p_args = (test_args_t *)p_thread_arg->p_args;
  // Set default configuration
  // Set mImgSource information (main)
  mImgInfo.color_format           = p_args->main.format;
  mImgInfo.width                  = p_args->main.width;
  mImgInfo.height                 = p_args->main.height;
  mImgInfo.fragment_cnt           = 1;
  mImgInfo.p_fragments[0].width   = p_args->main.width;
  mImgInfo.p_fragments[0].height  = p_args->main.height;
  mImgInfo.p_fragments[0].num_of_planes = p_args->main.num_of_planes;

  if (mImgInfo.color_format <= MONOCHROME) {
    mImgInfo.p_fragments[0].color.yuv.luma_buf =
      mSource.p_fragments[0].color.yuv.luma_buf;
    mImgInfo.p_fragments[0].color.yuv.chroma_buf =
      mSource.p_fragments[0].color.yuv.chroma_buf;
  }
  else if ((mImgInfo.color_format >= JPEG_BITSTREAM_H2V2) &&
    (mImgInfo.color_format < JPEG_COLOR_FORMAT_MAX)) {
    QIDBG_ERROR("%s:%d] Unsupported Format \n", __func__, __LINE__ );
    QI_UNLOCK(&mMutex);
    return QI_ERR_INVALID_OPERATION;
  }
  mImgSource.p_main = &mImgInfo;
  mImgSource.p_thumbnail = NULL;

  lrc = jpege_set_source(encoder, &mImgSource);

  if(QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] Failed ", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }

  QI_UNLOCK(&mMutex);
  QIDBG_LOW("%s:%d] Exited ...", __func__, __LINE__);
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
int QImageHybridEncoder::addOutputImage(QImage &aImage)
{
  int lrc = QI_SUCCESS;
  jpege_obj_t *p_encoder = (jpege_obj_t *)&mHybridObject;
  thread_ctrl_blk_t *p_thread_arg = (thread_ctrl_blk_t *)&mThreadCtrlBlk;
  jpege_obj_t encoder = (jpege_obj_t)*(p_encoder);
  int use_pmem = 0 ;
  int output_filename = 1;


  QIDBG_LOW("%s:%d] Entered ...", __func__, __LINE__);
  QI_LOCK(&(mMutex));
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }
  mOutputImage = &aImage;

  // Set destination information
  mDest.p_output_handler = encoder_output_handler;
  mDest.p_arg = (void*)&p_thread_arg->output_handler_args;
  mDest.buffer_cnt = 2;
  // Set default output buf size equal to 8K
  // if it is not provided by user
  if (!p_thread_arg->p_args->output_buf_size) {
    p_thread_arg->p_args->output_buf_size = 8192;
  }
  if (JPEG_FAILED(jpeg_buffer_init(&mBuffers[0])) ||
    JPEG_FAILED(jpeg_buffer_init(&mBuffers[1])) ||
    JPEG_FAILED(jpeg_buffer_allocate(mBuffers[0],
    p_thread_arg->p_args->output_buf_size, use_pmem)) ||
    JPEG_FAILED(jpeg_buffer_allocate(mBuffers[1],
    p_thread_arg->p_args->output_buf_size, use_pmem))) {
    QIDBG_ERROR("%s:%d] failed to allocate destination mBuffers... \n",
      __func__, __LINE__ );
    jpeg_buffer_destroy(&mBuffers[0]);
    jpeg_buffer_destroy(&mBuffers[1]);
    mDest.buffer_cnt = 0;
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }

  if(output_filename) {
    p_thread_arg->output_handler_args.fout = mOutputImage->BaseAddr();
    if (!p_thread_arg->output_handler_args.fout) {
      QIDBG_ERROR("%s:%d] failed to open output file: %d \n",
        __func__, __LINE__, output_filename);
      QI_UNLOCK(&(mMutex));
      return QI_ERR_INVALID_OPERATION;
    }
  }
  p_thread_arg->output_handler_args.size = 0;
  p_thread_arg->output_handler_args.nowrite =
    p_thread_arg->p_args->output_nowrite;

  mDest.p_buffer = &mBuffers[0];
  lrc = jpege_set_destination(encoder, &mDest);

  if(QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] Failed ", __func__, __LINE__);
    QI_UNLOCK(&(mMutex));
    return QI_ERR_INVALID_OPERATION;
  }
  QI_UNLOCK(&mMutex);
  QIDBG_LOW("%s:%d] Exited ...", __func__, __LINE__);
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
int QImageHybridEncoder::addObserver(QImageEncoderObserver &aObserver)
{
  QIDBG_LOW("%s:%d] Entered ...", __func__, __LINE__);
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
  QIDBG_LOW("%s:%d] Exited ...", __func__, __LINE__);
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
int QImageHybridEncoder::Stop()   // How to handle abort in the code?
{
  QIDBG_LOW("%s:%d] Entered ...", __func__, __LINE__);
  QI_LOCK(&(mMutex));
  if (mState != ESTATE_ACTIVE) {
    QI_UNLOCK(&(mMutex));
    QIDBG_ERROR("%s:%d] Stop ", __func__, __LINE__);
    return QI_SUCCESS;
  }

  mState = ESTATE_STOP_REQUESTED;
  QI_WAIT(&(mCond), &(mMutex));
  QI_UNLOCK(&(mMutex));
  QIDBG_LOW("%s:%d] Exited ...", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: QIHybridOutputThread
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
QIHybridOutputThread::QIHybridOutputThread(QImageHybridEncoder &aEncoder)
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
void QIHybridOutputThread::run(QThreadObject *aData)
{
}

/*===========================================================================
 * Function: QIHybridEncoderThread
 *
 * Description: QIHybridEncoderThread constructor
 *
 * Input parameters:
 *   aData - reference to thread object
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIHybridEncoderThread::QIHybridEncoderThread(QImageHybridEncoder &aEncoder)
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
void QIHybridEncoderThread::run(QThreadObject *aData)
{
  mEncoder.Encode();
}

/*===========================================================================
 * Function: Resize
 *
 * Resize input image before encoding
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QImageHybridEncoder::Resize()
{
  QIPlane *lPlane = mInputImage->getPlane(QIPlane::PLANE_Y);
  if (NULL == lPlane) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  QIPlane *lPlaneOut = mRszOutImage->getPlane(QIPlane::PLANE_Y);
  if (NULL == lPlaneOut) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  QISize InSize = mEncodeParams->InputSize();

  QIDBG_MED("%s:%d] Plane Y In addr 0x%x Out addr 0x%x ", __func__, __LINE__,
      (uint32_t)lPlane->Addr(),
      (uint32_t)lPlaneOut->Addr());

  QIDBG_MED("%s:%d] Plane Y Rsz IN w %d h %d str %d out w %d h %d str %d ",
      __func__, __LINE__,InSize.Width(),
      InSize.Height(), mInputImage->Size().Width(), mRszOutSize.Width(),
      mRszOutSize.Height(), mRszOutSize.Width());

  if (mScaleFactorH < 1.0) {
    fcvScaleDownMNu8(lPlane->Addr(),
        InSize.Width(),
        InSize.Height(),
        mInputImage->Size().Width(),
        lPlaneOut->Addr(),
        mRszOutSize.Width(),
        mRszOutSize.Height(),
        mRszOutSize.Width()
    );
  } else {
    fcvScaleUpPolyu8(lPlane->Addr(),
        InSize.Width(),
        InSize.Height(),
        mInputImage->Size().Width(),
        lPlaneOut->Addr(),
        mRszOutSize.Width(),
        mRszOutSize.Height(),
        mRszOutSize.Width()
    );
  }

  lPlane = mInputImage->getPlane(QIPlane::PLANE_CB_CR);
  if (NULL == lPlane) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lPlaneOut = mRszOutImage->getPlane(QIPlane::PLANE_CB_CR);
  if (NULL == lPlaneOut) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }


  QIDBG_MED("%s:%d] Plane CBCR In addr 0x%x Out addr 0x%x ", __func__, __LINE__,
      (uint32_t)lPlane->Addr(),
      (uint32_t) lPlaneOut->Addr());

  QIDBG_MED("%s:%d] Plane CBCR Rsz IN w %d h %d str %d out w %d h %d str %d ",
      __func__, __LINE__,lPlane->ActualSize().Width() >> 1,
      lPlane->ActualSize().Height(), lPlane->Size().Width(),
      lPlaneOut->ActualSize().Width() >> 1, lPlaneOut->ActualSize().Height(),
      lPlaneOut->ActualSize().Width());

  if (mScaleFactorH < 1.0) {
    fcvScaleDownMNInterleaveu8(lPlane->Addr(),
        lPlane->ActualSize().Width() >> 1,
        lPlane->ActualSize().Height(),
        lPlane->Size().Width(),
        lPlaneOut->Addr(),
        lPlaneOut->ActualSize().Width() >> 1,
        lPlaneOut->ActualSize().Height(),
        lPlaneOut->ActualSize().Width());
  } else {
    fcvScaleUpPolyInterleaveu8(lPlane->Addr(),
        lPlane->ActualSize().Width() >> 1,
        lPlane->ActualSize().Height(),
        lPlane->Size().Width(),
        lPlaneOut->Addr(),
        lPlaneOut->ActualSize().Width() >> 1,
        lPlaneOut->ActualSize().Height(),
        lPlaneOut->ActualSize().Width());
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: reconfigureInputParamaters()
 *
 * Description: reconfigureInputParamaters() main function
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: Reconfigures the session for encoding the scaled image.
          Disables the scaling flag so that only encoding takes place,
          makes the height and width of input image equal to the scaled,
          height and width, sets the luma and chroma input pointers to
          point to scaled image, and disables the fastCV_flag for
          lower layers.
 *==========================================================================*/
void QImageHybridEncoder::reconfigureInputParamaters()
{
  jpege_obj_t *p_encoder = &mHybridObject;
  jpege_obj_t encoder = (jpege_obj_t)(*(p_encoder));
  thread_ctrl_blk_t *p_thread_arg = &mThreadCtrlBlk;
  test_args_t *p_args    = p_thread_arg->p_args;
  uint32_t   thumbHeight = p_args->main_scale_cfg.output_height;
  uint32_t   thumbWidth  = p_args->main_scale_cfg.output_width;

  mHybridConfig.main_scale_cfg.enable = 0;
  mHybridConfig.main_stride_cfg.enable = false;
  mHybridConfig.main_stride_cfg.input_luma_stride = 0;
  mHybridConfig.main_stride_cfg.input_chroma_stride = 0;
  mHybridConfig.main_stride_cfg.output_luma_stride = 0;
  mHybridConfig.main_stride_cfg.output_chroma_stride = 0;

  mImgInfo.width = thumbWidth;
  mImgInfo.height = thumbHeight;
  mImgInfo.p_fragments[0].width   = thumbWidth;
  mImgInfo.p_fragments[0].height  = thumbHeight;
  mImgSource.p_main = &mImgInfo;
  jpege_set_source(encoder, &mImgSource);
  mSource.p_fragments[0].color.yuv.luma_buf->ptr =
                                    mConfig.fastCV_buffer;
  mSource.p_fragments[0].color.yuv.luma_buf->size =
                                        thumbHeight*thumbWidth;
  mSource.p_fragments[0].color.yuv.chroma_buf->ptr =
            mConfig.fastCV_buffer + thumbHeight*thumbWidth;
  // Assuming H2V2 sub-sampling
  mSource.p_fragments[0].color.yuv.chroma_buf->size =
                                    thumbHeight*thumbWidth*0.5;
  p_args->fastCV_flag = 0;
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
void QImageHybridEncoder::Encode()
{
  int lrc = QI_SUCCESS;
  int size;
  int restart_interval, luma_width, luma_height;
  thread_ctrl_blk_t *p_thread_arg = &mThreadCtrlBlk;
  jpege_obj_t *p_encoder = &mHybridObject;
  test_args_t *p_args    = p_thread_arg->p_args;
  jpege_obj_t encoder = (jpege_obj_t)(*(p_encoder));
  uint32_t   buffer_size;
  uint32_t   thumbHeight = p_args->main_scale_cfg.output_height;
  uint32_t   thumbWidth  = p_args->main_scale_cfg.output_width;

  QIDBG_LOW("%s:%d] Entered ", __func__, __LINE__);

  QI_LOCK(&mMutex);

  if(mUseFastCv)
    mHybridConfig.main_scale_cfg.enable = 0;

  if (mScaleEnabled && mUseFastCv) {
   if (QI_SUCCESS != Resize()) {
    for (uint32_t k = 0; k < mObserverCnt; k++)
      mObserver[k]->EncodeError(QImageEncoderObserver::ERROR_GENERAL);
      QI_UNLOCK(&mMutex);
      return;
    }
  }

  if (mHybridConfig.main_scale_cfg.enable) {
    luma_width = mHybridConfig.main_scale_cfg.output_width;
    luma_height = mHybridConfig.main_scale_cfg.output_height;
  }
  else {
    luma_width = mHybridConfig.main.width;
    luma_height = mHybridConfig.main.height;
    QIDBG_MED("%s:%d] width %d height %d ", __func__, __LINE__, mHybridConfig.main.width, mHybridConfig.main.height);
  }

  if ( (mHybridConfig.rotation == 0) ||  (mHybridConfig.rotation == 180)) {
      restart_interval = (luma_width + 16 - 1) / 16;
  } else {
      restart_interval = (luma_height + 16 - 1) / 16;
  }

  mEncodeParams->setRestartInterval(restart_interval);

  register_buf(mInputImage->BaseAddr(),
    (mInputImage->getPlane(QIPlane::PLANE_Y)->Size().Length() +
    mInputImage->getPlane(QIPlane::PLANE_CB_CR)->Size().Length()),
    mInputImage->Fd());

  if(mUseMultiFastCV && mHybridConfig.main_scale_cfg.enable) {
    // First call to hybrid_jpege_start performs the scaling of the image
    // by defaulting the rotation to 0 degrees.
    // The second call is made only when the scaling is complete for all the
    // threads as for encoding with rotation, complete scaled image needs to be
    // present.
    p_args->fastCV_flag = 1;
    // Assuming H2V2 sub-sampling
    if(mfastCVbuffer) {
      lrc = hybrid_jpege_start(&mHybridObject, &mThreadCtrlBlk, &size, &mConfig, mfastCVbuffer);
      if (QI_ERROR(lrc)) {
        QIDBG_ERROR("Failed to start hybrid_jpege_start\n");
        for (uint32_t k = 0; k < mObserverCnt; k++) {
          mObserver[k]->EncodeError(QImageEncoderObserver::ERROR_GENERAL);
        }
        QI_UNLOCK(&mMutex);
        return;
      }
      reconfigureInputParamaters();
    } else {
      QIDBG_ERROR("Failed to allocate buffers: falling back to no FastCV\n");
      p_args->fastCV_flag = 0;
    }
  }
  lrc = hybrid_jpege_start(&mHybridObject, &mThreadCtrlBlk, &size, &mConfig, NULL);

  register_buf(mInputImage->BaseAddr(),
    (mInputImage->getPlane(QIPlane::PLANE_Y)->Size().Length() +
    mInputImage->getPlane(QIPlane::PLANE_CB_CR)->Size().Length()), -1);

  mState = ESTATE_IDLE;
  QI_SIGNAL(&mCond);
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("Failed to start hybrid_jpege_start\n");
    for (uint32_t k = 0; k < mObserverCnt; k++) {
      mObserver[k]->EncodeError(QImageEncoderObserver::ERROR_GENERAL);
    }
    QI_UNLOCK(&mMutex);
    return;
  }
  mOutputImage->SetFilledLen(size);

  QIDBG_ERROR("%s:%d] Encode done : output_size %d", __func__, __LINE__, size);
  for (uint32_t k = 0; k < mObserverCnt; k++) {
    mObserver[k]->EncodeComplete(mOutputImage);
  }
  QI_UNLOCK(&mMutex);

  QIDBG_LOW("%s:%d] Exited ...", __func__, __LINE__);
  return;
}


/*===========================================================================
 * Function: hybrid_jpege_start
 *
 * Description: Encode start function
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   JPEGERR_SUCCESS
 *   JPEGERR_EFAILED
 *
 * Notes: none
 *==========================================================================*/
int hybrid_jpege_start(void *pHybridEncoder, void *arg, int *size,
  jpege_cfg_t *pConfig, uint8_t *fastCV_buffer)
{
  int  rc = JPEGERR_SUCCESS;
  thread_ctrl_blk_t *p_thread_arg = (thread_ctrl_blk_t *)arg;
  jpege_obj_t *p_encoder = (jpege_obj_t *)pHybridEncoder;
  test_args_t *p_args    = p_thread_arg->p_args;
  jpege_obj_t encoder = (jpege_obj_t)(*(p_encoder));

  QIDBG_LOW("%s:%d] Entered... \n", __func__, __LINE__ );
  // Set default configuration
  rc = jpege_get_default_config(pConfig);
  if (JPEG_FAILED(rc)) {
    QIDBG_ERROR("%s:%d] jpege_set_default_config failed \n",
      __func__, __LINE__ );
    return JPEGERR_EFAILED;
  }

  // Set custom configuration
  pConfig->main_cfg.rotation_degree_clk = p_args->rotation;
  if (p_args->main.quality) {
    pConfig->main_cfg.quality = p_args->main.quality;
  }
  pConfig->thumbnail_cfg.rotation_degree_clk = p_args->rotation;

  if (p_args->thumbnail.quality) {
    pConfig->thumbnail_cfg.quality = p_args->thumbnail.quality;
  }
  pConfig->thumbnail_present = p_args->encode_thumbnail;

  if (p_args->preference < JPEG_ENCODER_PREF_MAX) {
    pConfig->preference = (jpege_preference_t)p_args->preference;
  }

  // Set scale cfg
  pConfig->main_cfg.scale_cfg = p_args->main_scale_cfg;
  pConfig->thumbnail_cfg.scale_cfg = p_args->tn_scale_cfg;

  // Set stride cfg
  pConfig->main_cfg.stride_cfg = p_args->main_stride_cfg;
  pConfig->thumbnail_cfg.stride_cfg = p_args->tn_stride_cfg;
  // Set target file size
  pConfig->target_filesize =  p_args->target_filesize;

  // Specify restart interval
  pConfig->main_cfg.restart_interval = p_args->restart_interval;

  if(p_args->fastCV_flag) {
    pConfig->fastCV_buffer = fastCV_buffer;
    pConfig->fastCV_flag = 1;
  } else {
    pConfig->fastCV_flag = 0;
  }

  p_thread_arg->encoding = true;
  rc = jpege_start(encoder, pConfig, NULL);
  if (JPEG_FAILED(rc)) {
    QIDBG_ERROR("%s:%d] failed \n", __func__, __LINE__ );
    return JPEGERR_EFAILED;
  }

  if (p_args->abort_time) {
    os_mutex_lock(&p_thread_arg->output_handler_args.mutex);
    while (p_thread_arg->encoding) {
      rc = os_cond_timedwait(&p_thread_arg->output_handler_args.cond,
        &p_thread_arg->output_handler_args.mutex,
        p_args->abort_time);

      if (rc == JPEGERR_ETIMEDOUT) {
        // Do abort
        QIDBG_ERROR("%s:%d] : abort now...\n", __func__, __LINE__);
        os_mutex_unlock(&p_thread_arg->output_handler_args.mutex);
        rc = jpege_abort(encoder);
        if (rc) {
          QIDBG_ERROR("%s:%d] : failed \n", __func__, __LINE__);
          return JPEGERR_EFAILED;
        }
        break;
      }
    }
    if (p_thread_arg->encoding) {
      os_mutex_unlock(&p_thread_arg->output_handler_args.mutex);
    }
  } else {
    // Wait until encoding is done or stopped due to error
    os_mutex_lock(&p_thread_arg->output_handler_args.mutex);
    while (p_thread_arg->encoding) {
      os_cond_wait(&p_thread_arg->output_handler_args.cond,
        &p_thread_arg->output_handler_args.mutex);
    }
    os_mutex_unlock(&p_thread_arg->output_handler_args.mutex);
  }

  rc = jpege_get_actual_config(encoder, pConfig);
  if (JPEG_FAILED(rc)) {
    QIDBG_ERROR("%s:%d] failed \n", __func__, __LINE__ );
    return JPEGERR_EFAILED;
  }
  *(size) = p_thread_arg->output_handler_args.size;
  QIDBG_LOW("%s:%d] Exited... \n", __func__, __LINE__ );
  return JPEGERR_SUCCESS;
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
void QImageHybridEncoder::ReleaseSession()
{
  jpege_obj_t *p_encoder = (jpege_obj_t *)&mHybridObject;
  QIDBG_LOW("%s:%d] Entered ...", __func__, __LINE__);
  Stop();
  QI_LOCK(&(mMutex));
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
  (void)os_mutex_destroy(&mThreadCtrlBlk.output_handler_args.mutex);
  (void)os_cond_destroy(&mThreadCtrlBlk.output_handler_args.cond);
  jpeg_buffer_destroy(&mBuffers[0]);
  jpeg_buffer_destroy(&mBuffers[1]);
  // Clean up encoder
  jpege_destroy(p_encoder);

  if (mRszOutImage) {
    delete mRszOutImage;
    mRszOutImage = NULL;
  }

  if (mRszOutBuffer) {
    delete mRszOutBuffer;
    mRszOutBuffer = NULL;
  }
  if(mUseMultiFastCV) {
    JPEG_FREE(mfastCVbuffer);
    mfastCVbuffer = NULL;
  }

  mObserverCnt = 0;
  QI_UNLOCK(&(mMutex));
  QIDBG_LOW("%s:%d] Exited ...", __func__, __LINE__);
}



void encoder_event_handler(void *p_user_data,
  jpeg_event_t event, void *p_arg)
{
  thread_ctrl_blk_t* p_thread_arg = (thread_ctrl_blk_t *)p_user_data;
  // If it is not a warning event, encoder has stopped; Signal
  // main thread to clean up
  if (event == JPEG_EVENT_DONE || event == JPEG_EVENT_ERROR) {
    os_mutex_lock(&p_thread_arg->output_handler_args.mutex);
    p_thread_arg->encoding = false;
    os_cond_signal(&p_thread_arg->output_handler_args.cond);
    os_mutex_unlock(&p_thread_arg->output_handler_args.mutex);
  }
}

int encoder_output_handler(void *p_user_data, void *p_arg,
  jpeg_buffer_t buffer, uint8_t last_buf_flag)
{
  output_handler_args_t *p_output_args = (output_handler_args_t*)p_arg;
  uint32_t buf_size;
  uint8_t *buf_ptr;
  uint32_t rc = JPEGERR_SUCCESS;
  thread_ctrl_blk_t* p_thread_arg = (thread_ctrl_blk_t *)p_user_data;
  os_timer_t os_timer;
  int diff = 0;

  os_mutex_lock(&p_output_args->mutex);
  if (!p_output_args->fout) {
    QIDBG_ERROR("%s:%d] invalid p_arg \n", __func__, __LINE__ );
    os_mutex_unlock(&p_output_args->mutex);
    return JPEGERR_EFAILED;
  }

  if (JPEG_FAILED(jpeg_buffer_get_actual_size(buffer, &buf_size))) {
    QIDBG_ERROR("%s:%d] failed \n", __func__, __LINE__ );
    os_mutex_unlock(&p_output_args->mutex);
    return JPEGERR_EFAILED;
  }
  if (JPEG_FAILED(jpeg_buffer_get_addr(buffer, &buf_ptr))) {
    QIDBG_ERROR("%s:%d] jpeg_buffer_get_addr failed \n",
      __func__, __LINE__ );
    os_mutex_unlock(&p_output_args->mutex);
    return JPEGERR_EFAILED;
  }
  if (!p_output_args->nowrite) {
    //os_timer_start(&os_timer);
    memcpy(p_output_args->fout, buf_ptr, buf_size);
    //os_timer_get_elapsed(&os_timer, &diff, 0);
  }
  p_output_args->size += buf_size;
  p_output_args->fout = (void *)((uint32_t)p_output_args->fout + buf_size);
  os_mutex_unlock(&p_output_args->mutex);

  if (last_buf_flag) {
    QIDBG_HIGH("%s:%d] encoder_output_handler: \
      received last output buffer\n",
      __func__, __LINE__ );
  }

  // Set the output buffer offset to zero
  if (JPEG_FAILED(jpeg_buffer_set_actual_size(buffer, 0))) {
    QIDBG_ERROR("%s:%d] jpeg_buffer_set_actual_size failed\n",
      __func__, __LINE__ );
  }

  // Enqueue back the output buffer to queue
  rc = jpege_enqueue_output_buffer((p_thread_arg->encoder),
    &buffer, 1);

  if (JPEG_FAILED(rc)) {
    QIDBG_ERROR("encoder_output_handler: \
      jpege_enqueue_output_buffer failed\n");
    return JPEGERR_EFAILED;
  }
  return JPEGERR_SUCCESS;
}
