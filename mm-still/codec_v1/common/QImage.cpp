/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QImage.h"

/*===========================================================================
 * Function: QImage
 *
 * Description: QImage constructor
 *
 * Input parameters:
 *   aSize - size of the image
 *   aSubSampling - subsampling of the image
 *   aFormat - format of the image
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImage::QImage(QISize &aSize, QISubsampling aSubSampling, QIFormat aFormat) :
  mSize(aSize),
  mActualSize(aSize)
{
  mPlaneCnt = 0;
  mSubSampling = aSubSampling;
  mFormat = aFormat;
  mBaseAddr = NULL;
  mFd = -1;
  mLength = 0;
  mFilledLen = 0;
  for (int i = 0; i < QI_MAX_PLANES; i++) {
    mPlane[i] = NULL;
  }
}

/*===========================================================================
 * Function: QImage
 *
 * Description: QImage constructor
 *
 * Input parameters:
 *   aSize - padded size of the image
 *   aSubSampling - subsampling of the image
 *   aFormat - format of the image
 *   aActualSize - actual size of the image
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImage::QImage(QISize &aSize, QISubsampling aSubSampling, QIFormat aFormat,
  QISize &aActualSize) :
  mSize(aSize),
  mActualSize(aActualSize)
{
  mPlaneCnt = 0;
  mSubSampling = aSubSampling;
  mFormat = aFormat;
  mBaseAddr = NULL;
  mFd = -1;
  mLength = 0;
  mFilledLen = 0;
  for (int i = 0; i < QI_MAX_PLANES; i++) {
    mPlane[i] = NULL;
  }
}

/*===========================================================================
 * Function: QImage
 *
 * Description: QImage constructor
 *
 * Input parameters:
 *   aAddr - Address of the image data
 *   aLength - Length of the image data
 *   aFormat - Format of the image
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImage::QImage(uint8_t *aAddr, uint32_t aLength, QIFormat aFormat) :
  mSize(QISize(0, 0))
{
  mPlaneCnt = 0;
  mSubSampling = QI_H2V2;
  mFormat = aFormat;
  mLength = aLength;
  mFilledLen = 0;
  for (int i = 0; i < QI_MAX_PLANES; i++) {
    mPlane[i] = NULL;
  }
  mBaseAddr = aAddr;
  mFd = -1;
}

/*===========================================================================
 * Function: ~QImage
 *
 * Description: QImage destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImage::~QImage()
{
  for (uint32_t i = 0; i < mPlaneCnt; i++) {
    if (NULL != mPlane[i])
      delete mPlane[i];
  }
}

/*===========================================================================
 * Function: addPlane
 *
 * Description: Add the plane to the image
 *
 * Input parameters:
 *   aPlane - Pointer to QIPlane
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_OUT_OF_BOUNDS
 *
 * Notes: none
 *==========================================================================*/
int QImage::addPlane(QIPlane *aPlane)
{
  if (mPlaneCnt >= QI_MAX_PLANES) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_OUT_OF_BOUNDS;
  }
  mPlane[mPlaneCnt++] = aPlane;
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: getPlane
 *
 * Description: Get the plane object of the specified type
 *
 * Input parameters:
 *   aType - type of the plane
 *
 * Return values:
 *   NULL
 *   Plane address
 *
 * Notes: none
 *==========================================================================*/
QIPlane* QImage::getPlane(QIPlane::Type aType)
{
  for (uint32_t i = 0; i < mPlaneCnt; i++) {
    if (mPlane[i]->getType() == aType) {
      return mPlane[i];
    }
  }
  QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
  return NULL;
}

/*===========================================================================
 * Function: getChromaWidthFactor
 *
 * Description: Static function to get the chroma width factor in double
 *
 * Input parameters:
 *   aSubSampling - subsampling type
 *
 * Return values:
 *   subsampling factor in double
 *
 * Notes: none
 *==========================================================================*/
float QImage::getChromaWidthFactor(QISubsampling aSubSampling)
{
  switch (aSubSampling) {
  case QI_H2V1:
    return 0.5;
  case QI_H1V2:
    return 1.0;
  case QI_H1V1:
    return 1.0;
  default:
  case QI_H2V2:
    return 0.5;
  }
}

/*===========================================================================
 * Function: IsCrCb
 *
 * Description: Static function to get the chroma order
 *
 * Input parameters:
 *   aFormat - format of the image
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 *==========================================================================*/
bool QImage::IsCrCb(QIFormat aFormat)
{
  switch (aFormat) {
  case QI_YCBCR_SP:
  case QI_YUV2:
    return false;
  default:
  case QI_YCRCB_SP:
  case QI_IYUV:
    return true;
  }
}

/*===========================================================================
 * Function: PaddedSize
 *
 * Description: Static function to get the padded size of the image
 *
 * Input parameters:
 *   aImage - image object
 *
 * Return values:
 *   padded size of type QISize
 *
 * Notes: none
 *==========================================================================*/
QISize QImage::PaddedSize(QImage &aImage)
{
  if (QI_BITSTREAM == aImage.Format()) {
    return aImage.mActualSize;
  } else if (QI_MONOCHROME != aImage.Format()) {
      switch (aImage.SubSampling()) {
      case QI_H2V1:
        return QISize(CEILING16(aImage.mActualSize.Width()),
          CEILING8(aImage.mActualSize.Height()));
      case QI_H1V2:
        return QISize(CEILING8(aImage.mActualSize.Width()),
          CEILING16(aImage.mActualSize.Height()));
      case QI_H1V1:
        return QISize(CEILING8(aImage.mActualSize.Width()),
          CEILING8(aImage.mActualSize.Height()));
      default:
      case QI_H2V2:
        return QISize(CEILING16(aImage.mActualSize.Width()),
          CEILING16(aImage.mActualSize.Height()));
      }
  } else { /*monochrome*/
    return QISize(CEILING8(aImage.mActualSize.Width()),
      CEILING8(aImage.mActualSize.Height()));
  }
}

/*===========================================================================
 * Function: GetPlaneCount
 *
 * Description: Static function to get the plane count given the formats
 *
 * Input parameters:
 *   aFormat - image format
 *
 * Return values:
 *   number for planes for the format
 *
 * Notes: none
 *==========================================================================*/
uint32_t QImage::GetPlaneCount(QIFormat aFormat)
{
  switch (aFormat) {
  case QI_IYUV:
  case QI_YUV2:
    return 3;
  case QI_MONOCHROME:
  case QI_BITSTREAM:
    return 1;
  default:
  case QI_YCRCB_SP:
  case QI_YCBCR_SP:
    return 2;
  }
}

/*===========================================================================
 * Function: getChromaHeightFactor
 *
 * Description: Static function to get the chroma height factor in double
 *
 * Input parameters:
 *   aSubSampling - subsampling type
 *
 * Return values:
 *   subsampling factor in double
 *
 * Notes: none
 *==========================================================================*/
float QImage::getChromaHeightFactor(QISubsampling aSubSampling)
{
  switch (aSubSampling) {
  case QI_H2V1:
    return 1.0;
  case QI_H1V2:
    return 0.5;
  case QI_H1V1:
    return 1.0;
  default:
  case QI_H2V2:
    return 0.5;
  }
}

/*===========================================================================
 * Function: getImageSize
 *
 * Description: Static function to get the image size in bytes given
 *              resolution, subsampling and format
 *
 * Input parameters:
 *   aSize - dimension of the image
 *   aSubSampling - subsampling type
 *   aFormat - format
 *
 * Return values:
 *   image size in bytes
 *
 * Notes: none
 *==========================================================================*/
uint32_t QImage::getImageSize(QISize &aSize, QISubsampling aSubSampling,
  QIFormat aFormat)
{
  float lChromaFactor = getChromaWidthFactor(aSubSampling) *
    getChromaHeightFactor(aSubSampling);
  switch (aFormat) {
  case QI_MONOCHROME:
    return aSize.Length();
  case QI_YCRCB_SP:
  case QI_YCBCR_SP:
  case QI_IYUV:
  case QI_YUV2:
    return aSize.Length() + aSize.Length() * lChromaFactor * 2;
  default:
    return 0;
  }
}

/*===========================================================================
 * Function: setDefaultPlanes
 *
 * Description: Create the planes with the initial parameters passed for the
 *              image
 *
 * Input parameters:
 *   aPlaneCnt - plane count
 *   aAddr - address of the image
 *   aFd - fd of the image
 *
 * Return values:
 *   QI_ERR_INVALID_INPUT
 *   QI_SUCCESS
 *
 * Notes: none
 *==========================================================================*/
int QImage::setDefaultPlanes(uint32_t aPlaneCnt, uint8_t *aAddr, int aFd,
  uint32_t *aOffset, uint32_t *aPhyOffset)
{
  QIPlane *lPlane = NULL;
  uint8_t *lPlaneAddr[QI_MAX_PLANES] = {NULL, NULL, NULL};
  QISize lSize[QI_MAX_PLANES] = {QISize(0, 0), QISize(0, 0), QISize(0, 0)};
  QISize lActualSize[QI_MAX_PLANES] = {QISize(0, 0), QISize(0, 0),
    QISize(0, 0)};
  bool lCrCb[QI_MAX_PLANES] = {true, true, true};
  QIPlane::Type lType[QI_MAX_PLANES];
  uint32_t lOffset[QI_MAX_PLANES] = {0, 0, 0};
  uint32_t lPhyOffset[QI_MAX_PLANES] = {0, 0, 0};
  int lStatus = QI_SUCCESS;
  uint32_t lPlaneCnt = 0;
  bool lCalcPhyOffset = false;

  if (NULL == aOffset) {
    aOffset = lOffset;
    QIDBG_MED("%s:%d] Use default offsets", __func__, __LINE__);
  }

  if (NULL == aPhyOffset) {
    aPhyOffset = lPhyOffset;
    lCalcPhyOffset = true;
    QIDBG_MED("%s:%d] Use default physical offsets", __func__, __LINE__);
  }

  switch (mFormat) {
  case QI_YCRCB_SP:
  case QI_YCBCR_SP: {
    lPlaneCnt = 2;

    lPlaneAddr[0] = aAddr;
    lSize[0] = mSize;
    lActualSize[0] = mActualSize;
    lType[0] = QIPlane::PLANE_Y;

    lPlaneAddr[1] = aAddr;
    lSize[1] = QISize(mSize.Width() * getChromaWidthFactor(mSubSampling) * 2,
      mSize.Height() * getChromaHeightFactor(mSubSampling));
    lActualSize[1] =
      QISize(mActualSize.Width() * getChromaWidthFactor(mSubSampling) * 2,
      mActualSize.Height() * getChromaHeightFactor(mSubSampling));
    lCrCb[1] = (mFormat == QI_YCRCB_SP);
    lType[1] = QIPlane::PLANE_CB_CR;
    if (lCalcPhyOffset)
      aPhyOffset[1] = lSize[0].Length();

    break;
  }
  case QI_YUV2:
  case QI_IYUV: {
    lPlaneCnt = 3;

    lPlaneAddr[0] = aAddr;
    lSize[0] = mSize;
    lActualSize[0] = mActualSize;
    lType[0] = QIPlane::PLANE_Y;

    lPlaneAddr[1] = aAddr;
    lSize[1] = QISize(mSize.Width() * getChromaWidthFactor(mSubSampling),
      mSize.Height() * getChromaHeightFactor(mSubSampling));
    lActualSize[1] = QISize(mActualSize.Width() * getChromaWidthFactor(mSubSampling),
      mActualSize.Height() * getChromaHeightFactor(mSubSampling));
    if (lCalcPhyOffset)
      aPhyOffset[1] = lSize[0].Length();

    lPlaneAddr[2] = aAddr;
    lSize[2] = lSize[1];
    lActualSize[2] = lActualSize[1];
    if (lCalcPhyOffset)
      aPhyOffset[2] = lPhyOffset[1] + lSize[1].Length();

    if (QI_YUV2 == mFormat) {
      lType[1] = QIPlane::PLANE_CB;
      lType[2] = QIPlane::PLANE_CR;
    } else {
      lType[1] = QIPlane::PLANE_CR;
      lType[2] = QIPlane::PLANE_CB;
    }
    break;
  }
  case QI_MONOCHROME: {
    lPlaneCnt = 1;
    lPlaneAddr[0] = aAddr;
    lSize[0] = mSize;
    lActualSize[0] = mActualSize;
    lType[0] = QIPlane::PLANE_Y;
    break;
  }
  case QI_BITSTREAM: {
    mPlaneCnt = 1;
    lPlaneAddr[0] = aAddr;
    lSize[0] = QISize(mLength, 1);
    lType[0] = QIPlane::PLANE_BS;
    break;
  }
  default: {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_INVALID_INPUT;
  }
  }

  for (uint32_t i = 0; i < lPlaneCnt; i++) {
    lPlane = new QIPlane(lType[i], lPlaneAddr[i], lSize[i].Width(),
      lSize[i].Length(), aFd, lSize[i], lActualSize[i], aOffset[i],
      aPhyOffset[i], lCrCb[i]);
    if (NULL == lPlane) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    lStatus = addPlane(lPlane);
    if (QI_SUCCESS != lStatus) {
      QIDBG_ERROR("%s:%d] failed %d", __func__, __LINE__, lStatus);
      return lStatus;
    }
  }
  mBaseAddr = aAddr;
  mFd = aFd;
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: operator=
 *
 * Description: Overload the assignment operator for image
 *
 * Input parameters:
 *   aOther - QImage reference to be copied
 *
 * Return values:
 *   reference to the input image
 *
 * Notes: none
 *==========================================================================*/
QImage& QImage::operator=(const QImage& aOther)
{
  uint32_t lStatus = QI_SUCCESS;
  QIPlane *lPlane = NULL;
  mPlaneCnt = aOther.mPlaneCnt;
  mSize = aOther.mSize;
  mActualSize = aOther.mActualSize;
  mSubSampling = aOther.mSubSampling;
  mFormat = aOther.mFormat;
  mLength = aOther.mLength;
  mBaseAddr = aOther.mBaseAddr;
  mFd = aOther.mFd;

  for (uint32_t i = 0; i < mPlaneCnt; i++) {
    mPlane[i] = new QIPlane();
    if (NULL == lPlane) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return *this;
    }
    mPlane[i] = aOther.mPlane[i];
  }
  return *this;
}
