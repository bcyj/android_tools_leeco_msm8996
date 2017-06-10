/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QIPlane.h"

/*===========================================================================
 * Function: QIPlane
 *
 * Description: QIPlane constructor
 *
 * Input parameters:
 *   aType - plane type
 *   aAddr - address of the plane
 *   aStride - stride of plane
 *   aLength - length of the plane
 *   aFd - file descriptor of the plane
 *   aSize - padded dimension of the plane
 *   aActualSize - actual dimension of the plane
 *   aOffset - offset of the valid data in the plane
 *   aPhyOffset - physical offset of the plane address from the start
 *                of physical memory
 *   aCrCb - order of chroma plane
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIPlane::QIPlane(Type aType, uint8_t *aAddr, uint32_t aStride,
  uint32_t aLength, int aFd, QISize &aSize, QISize &aActualSize,
  int aOffset, int aPhyOffset, bool aCrCb) :
  mSize(aSize),
  mActualSize(aActualSize)
{
  mType = aType;
  mAddr = aAddr;
  mStride = aStride;
  mLength = aLength;
  mFd = aFd;
  mOffset = aOffset;
  mPhyOffset = aPhyOffset;
  mCrCb = aCrCb;
}

/*===========================================================================
 * Function: QIPlane
 *
 * Description: QIPlane constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIPlane::QIPlane() :
  mSize(QISize(0, 0)),
  mActualSize(QISize(0, 0))
{
  mType = PLANE_Y;
  mAddr = NULL;
  mStride = 0;
  mLength = 0;
  mFd = -1;
  mOffset = 0;
  mPhyOffset = 0;
  mCrCb = true;
}

/*===========================================================================
 * Function: QIPlane
 *
 * Description: QIPlane destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIPlane::~QIPlane()
{
}

/*===========================================================================
 * Function: setPlaneInfo
 *
 * Description: set the plane information.
 *
 * Input parameters:
 *   aType - plane type
 *   aAddr - address of the plane
 *   aStride - stride of plane
 *   aLength - length of the plane
 *   aFd - file descriptor of the plane
 *   aSize - padded dimension of the plane
 *   aActualSize - actual dimension of the plane
 *   aOffset - offset of the valid data in the plane
 *   aPhyOffset - physical offset of the plane address from the start
 *                of physical memory
 *   aCrCb - order of chroma plane
 *
 * Return values:
 *   none
 *
 * Notes: This function needs to be called only when default constructor
 *        is called.
 *==========================================================================*/
void QIPlane::setPlaneInfo(Type aType, uint8_t *aAddr, uint32_t aStride,
  uint32_t aLength, int aFd, QISize &aSize, QISize &aActualSize,
  int aOffset, int aPhyOffset, bool aCrCb)
{
  mType = aType;
  mAddr = aAddr;
  mStride = aStride;
  mLength = aLength;
  mFd = aFd;
  mOffset = aOffset;
  mSize = aSize;
  mActualSize = aActualSize;
  mCrCb = aCrCb;
  mPhyOffset = aPhyOffset;
}
