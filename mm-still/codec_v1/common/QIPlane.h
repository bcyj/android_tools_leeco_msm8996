/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIPLANE_H__
#define __QIPLANE_H__

#include "QIBase.h"
#include "QICommon.h"
#include "QISize.h"

/*===========================================================================
 * Class: QIHuffTable
 *
 * Description: This class represents the single plane for the image
 *
 *
 * Notes: none
 *==========================================================================*/
class QIPlane : public QIBase {

public:

  /** Type:
   *  PLANE_Y: Y plane
   *  PLANE_CB: Cb plane
   *  PLANE_CB_CR: interleaved CbCr plane
   *  PLANE_CR: Cr plane
   *  PLANE_Y_CB_CR: interleaved YCbCr plane
   *  PLANE_BS: bitstream plane
   *
   *  plane type
   **/
  enum Type {
    PLANE_Y,
    PLANE_CB,
    PLANE_CB_CR,
    PLANE_CR,
    PLANE_Y_CB_CR,
    PLANE_BS,
  };

  /** QIPlane:
   *  @aType: plane type
   *  @aAddr: plane base address
   *  @aStride: plane stride
   *  @aLength: plane length
   *  @aFd: fd of the plane
   *  @aSize: padded size of the plane
   *  @aActualSize: actual size of the plane
   *  @aOffset: offset of the plane
   *  @aPhyOffset: physical offset of the plane from base address
   *             of the buffer
   *  @aCrCb: order of interleaved plane is CrCb
   *
   *  constructor
   **/
  QIPlane(Type aType, uint8_t *aAddr, uint32_t aStride,
    uint32_t aLength, int aFd, QISize &aSize, QISize &aActualSize,
    int aOffset, int aPhyOffset = 0, bool aCrCb = true);

  /** QIPlane:
   *
   *  constructor
   **/
  QIPlane();

  /** ~QIPlane:
   *
   *  virtual destructor
   **/
  virtual ~QIPlane();

  /** setPlaneInfo:
   *  @aType: plane type
   *  @aAddr: plane base address
   *  @aStride: plane stride
   *  @aLength: plane length
   *  @aFd: fd of the plane
   *  @aSize: padded size of the plane
   *  @aActualSize: actual size of the plane
   *  @aOffset: offset of the plane
   *  @aPhyOffset: physical offset of the plane from base address
   *             of the buffer
   *  @aCrCb: order of interleaved plane is CrCb
   *
   *  sets the plane info
   **/
  void setPlaneInfo(Type aType, uint8_t *aAddr, uint32_t aStride,
    uint32_t aLength, int aFd, QISize &aSize, QISize &aActualSize,
    int aOffset, int aPhyOffset = 0, bool aCrCb = true);

  /** getType:
   *
   *  returns the type of the plane
   **/
  inline Type getType()
  {
    return mType;
  }

  /** BaseAddr:
   *
   *  returns the base address of the plane
   **/
  inline uint8_t* BaseAddr()
  {
    return mAddr;
  }

  /** ActualAddr:
   *
   *  returns the physical address of valid data of the plane in
   *  the memory
   **/
  inline uint8_t* ActualAddr()
  {
    return mAddr + mOffset + mPhyOffset;
  }

  /** Addr:
   *
   *  returns the physical address of the plane in the memory
   **/
  inline uint8_t* Addr()
  {
    return mAddr + mPhyOffset;
  }

  /** Stride:
   *
   *  returns the stride of the plane
   **/
  inline uint32_t Stride()
  {
    return mStride;
  }

  /** Length:
   *
   *  returns the length of the plane
   **/
  inline uint32_t Length()
  {
    return mLength;
  }

  /** Fd:
   *
   *  returns the fd of the plane
   **/
  inline int Fd()
  {
    return mFd;
  }

  /** Size:
   *
   *  returns the padded size of the plane
   **/
  inline QISize &Size()
  {
    return mSize;
  }

  /** ActualSize:
   *
   *  returns the actual size of the plane
   **/
  inline QISize &ActualSize()
  {
    return mActualSize;
  }

  /** Offset:
   *
   *  returns the offset of valid data in the plane
   **/
  inline int Offset()
  {
    return mOffset;
  }

  /** PhyOffset:
   *
   *  returns the physical offset of the plane
   **/
  inline int PhyOffset()
  {
    return mPhyOffset;
  }

private:

  /** mType:
   *
   *  type of the plane
   **/
  Type mType;

  /** mAddr:
   *
   *  base address of the plane
   **/
  uint8_t *mAddr;

  /** mStride:
   *
   *  stride of the plane
   **/
  uint32_t mStride;

  /** mLength:
   *
   *  length of the plane
   **/
  uint32_t mLength;

  /** mFd:
   *
   *  fd of the plane
   **/
  int mFd;

  /** mSize:
   *
   *  size of the plane
   **/
  QISize mSize;

  /** mActualSize:
   *
   *  actual size of the plane
   **/
  QISize mActualSize;

  /** mOffset:
   *
   *  offset of the plane
   **/
  int mOffset;

  /** mPhyOffset:
   *
   *  physical offset of the plane
   **/
  int mPhyOffset;

  /** mCrCb:
   *
   *  order of interleaved chroma plane is CrCb
   **/
  bool mCrCb;
};

#endif //__QIPLANE_H__
