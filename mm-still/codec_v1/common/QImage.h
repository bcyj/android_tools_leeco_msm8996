/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGE_H__
#define __QIMAGE_H__

#include "QIBase.h"
#include "QICommon.h"
#include "QIPlane.h"

/*============================================================================
    MACROS
 *===========================================================================*/
#define QI_MAX_PLANES 3

/*===========================================================================
 * Class: QImage
 *
 * Description: This class represents the image data which is used by the
 *              codec framework
 *
 *
 * Notes: none
 *==========================================================================*/
class QImage : public QIBase {

public:

  /** QImage:
   *  @aSize - image size
   *  @aSubSampling - image subsampling
   *  @aFormat - image format
   *
   *  overloaded constructor for uncompressed images
   **/
  QImage(QISize &aSize, QISubsampling aSubSampling, QIFormat aFormat);

  /** QImage:
   *  @aSize - padded image size
   *  @aSubSampling - image subsampling
   *  @aFormat - image format
   *  @aActualSize - actual image size
   *
   *  overloaded constructor for uncompressed images with padding
   **/
  QImage(QISize &aSize, QISubsampling aSubSampling, QIFormat aFormat,
    QISize &aActualSize);

  /** QImage:
   *  @aAddr - address of image
   *  @aLength - length of the image
   *  @aFormat - image format
   *
   *  overloaded constructor for compressed images
   **/
  QImage(uint8_t *aAddr, uint32_t aLength, QIFormat aFormat);

  /** ~QImage:
   *
   *  virtual destructor
   **/
  virtual ~QImage();

  /** QImage:
   *  @aPlaneCnt - plane count
   *  @aAddr - address of the image
   *  @aFd - fd of the plane
   *  @aOffset - valid data offset of the plane
   *  @aPhyOffset - physical offset of the plane w.r.t the start
   *              of the physical buffer
   *
   *  set the default planes for the image
   *  if this function is called, client is not supposed to call
   *  addPlane
   **/
  int setDefaultPlanes(uint32_t aPlaneCnt, uint8_t *aAddr, int aFd,
    uint32_t *aOffset = NULL, uint32_t *aPhyOffset = NULL);

  /** addPlane:
   *  @aPlane - plane object
   *
   *  add plane to the image class
   *  if this function is called, client is not supposed to call
   *  setDefaultPlanes
   **/
  int addPlane(QIPlane *aPlane);

  /** PlaneCount:
   *
   *  returns the plane count
   **/
  inline uint32_t PlaneCount()
  {
    return mPlaneCnt;
  }

  /** getPlane:
   *  @aType - plane type
   *
   *  get the plane object of the image
   **/
  QIPlane* getPlane(QIPlane::Type aType);

  /** getImageSize:
   *  @aSize - image dimension
   *  @aSubSampling - image subsampling
   *  @aFormat - image format
   *
   *  static utility to get total image size
   **/
  static uint32_t getImageSize(QISize &aSize, QISubsampling aSubSampling,
    QIFormat aFormat);

  /** getChromaHeightFactor:
   *  @aSubSampling - image subsampling
   *
   *  static utility to get chroma height factor
   **/
  static float getChromaHeightFactor(QISubsampling aSubSampling);

  /** getChromaWidthFactor:
   *  @aSubSampling - image subsampling
   *
   *  static utility to get chroma width factor
   **/
  static float getChromaWidthFactor(QISubsampling aSubSampling);

  /** IsCrCb:
   *  @aFormat - image format
   *
   *  static utility to get utility ot get chroma order
   **/
  static bool IsCrCb(QIFormat aFormat);

  /** PaddedSize:
   *  @aImage - image object
   *
   *  static utility to get padded size for given image object
   **/
  static QISize PaddedSize(QImage &aImage);

  /** GetPlaneCount:
   *  @aFormat - image format
   *
   *  static utility to get the plane count given a format
   **/
  static uint32_t GetPlaneCount(QIFormat aFormat);

  /** BaseAddr:
   *
   *  returns base address of the image
   *  this could be used only for contiguous data. check for
   *  invalid values. if the values are invalid, use plane to
   *  fetch address
   *
   **/
  inline uint8_t* BaseAddr()
  {
    return mBaseAddr;
  }

  /** Fd:
   *
   *  returns fd of the image
   *  this could be used only for contiguous data. check for
   *  invalid values
   *
   **/
  inline int Fd()
  {
    return mFd;
  }

  /** WorkBufSize:
   *
   *  returns size of the workbuffer
   *
   **/
  inline int WorkBufSize()
  {
    return mWorkBufSize;
  }

  /** setBaseAddr:
   *  @aBaseAddr - base address of image
   *
   *  sets the base address
   *
   **/
  inline void setBaseAddr(uint8_t *aBaseAddr)
  {
    mBaseAddr = aBaseAddr;
  }

  /** setFd:
   *  @aFd - fd of the image incase of ion memory
   *
   *  sets the fd in case of physical memory
   *
   **/
  inline void setFd(int aFd)
  {
    mFd = aFd;
  }

  /** setWorkBufSize:
   *  @aWorkBufSize - size of workBuf
   *
   *  sets the size of work buffer
   *
   **/
  inline void setWorkBufSize(int aWorkBufSize)
  {
    mWorkBufSize = aWorkBufSize;
  }

  /** operator=:
   *  @aOther - image object
   *
   *  assignment operator overloaded for image class
   *
   **/
  QImage& operator=(const QImage& aOther);

  /** Format:
   *
   *  returns the format of the image data
   *
   **/
  inline QIFormat Format()
  {
    return mFormat;
  }

  /** SubSampling:
   *
   *  returns the subsampling type of the image data
   *
   **/
  inline QISubsampling SubSampling()
  {
    return mSubSampling;
  }

  /** Size:
   *
   *  returns the padded dimension of the image data
   *
   **/
  inline QISize &Size()
  {
    return mSize;
  }

  /** ActualSize:
   *
   *  returns the actual dimension of the image data
   *
   **/
  inline QISize &ActualSize()
  {
    return mActualSize;
  }

  /**
   *  The following APIs are applicable only for bitstreams
   *
   **/

  /** FilledLen:
   *
   *  returns the filled length of the image data
   *
   **/
  inline uint32_t FilledLen()
  {
    return mFilledLen;
  }

  /** FilledLen:
   *
   *  returns the length of the image data
   *
   **/
  inline uint32_t Length()
  {
    return mLength;
  }

  /** SetFilledLen:
   *
   *  sets the filled length of the image data
   *
   **/
  inline void SetFilledLen(uint32_t aFilledLen)
  {
    mFilledLen = aFilledLen;
  }

  /** IsFilled:
   *
   *  check if the image data is fully filled
   *
   **/
  inline bool IsFilled()
  {
    return mFilledLen == mLength;
  }

private:

  /** mPlane:
   *
   *  array of plane objects
   *
   **/
  QIPlane *mPlane[QI_MAX_PLANES];

  /** mPlaneCnt:
   *
   *  plane count
   *
   **/
  uint32_t mPlaneCnt;

  /** mSize:
   *
   *  padded size of the image
   *
   **/
  QISize mSize;

  /** mActualSize:
   *
   *  actual size of the image
   *
   **/
  QISize mActualSize;

  /** mSubSampling:
   *
   *  image subsampling
   *
   **/
  QISubsampling mSubSampling;

  /** mFormat:
   *
   *  format of the image
   *
   **/
  QIFormat mFormat;

  /** mLength:
   *
   *  length of the image
   *
   **/
  uint32_t mLength;

  /** mBaseAddr:
   *
   *  base address
   *
   **/
  uint8_t *mBaseAddr;

  /** mFd:
   *
   *  fd of image
   *
   **/
  int mFd;

  /** mFilledLen:
   *
   *  filled length
   *
   **/
  uint32_t mFilledLen;

  /** mWorkBufSize:
   *
   *  work buffer size
   *
   **/
  uint32_t mWorkBufSize;

};

#endif //__QIMAGE_H__
