/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QICROP_H__
#define __QICROP_H__

#include "QISize.h"

/*===========================================================================
 * Class: QICrop
 *
 * Description: This class represents the crop region for the image
 *
 *
 * Notes: none
 *==========================================================================*/
class QICrop {

public:

  /** QICrop:
   *
   *  constructor
   **/
  QICrop()
  {
    mX1 = mX2 = mY1 = mY2 = 0;
  }

  /** QICrop:
   *  @aX1 - left cordinate
   *  @aY1 - top cordinate
   *  @aX2 - right cordinate
   *  @aY2 - bottom cordinate
   *
   *  overloaded constructor
   **/
  QICrop(uint32_t aX1, uint32_t aY1, uint32_t aX2, uint32_t aY2)
  {
    mX1 = aX1;
    mX2 = aX2;
    mY1 = aY1;
    mY2 = aY2;
  }

  /** ~QICrop:
   *
   *  destructor
   **/
  ~QICrop()
  {
  }

  /** Width:
   *
   *  return the width of the crop window
   **/
  inline uint32_t Width()
  {
    return mX2 - mX1;
  }

  /** Height:
   *
   *  return the height of the crop window
   **/
  inline uint32_t Height()
  {
    return mY2 - mY1;
  }

  /** Left:
   *
   *  return the left cordinate
   **/
  inline uint32_t Left()
  {
    return mX1;
  }

  /** Top:
   *
   *  return the top cordinate
   **/
  inline uint32_t Top()
  {
    return mY1;
  }

  /** Right:
   *
   *  return the right cordinate
   **/
  inline uint32_t Right()
  {
    return mX2;
  }

  /** Bottom:
   *
   *  return the bottom co-ordinate
   **/
  inline uint32_t Bottom()
  {
    return mY2;
  }

  /** Length:
   *
   *  returns the length of the crop window in bytes
   **/
  inline uint32_t Length()
  {
    return Width() * Height();
  }

  /** Size:
   *
   *  returns the size of the crop window in bytes
   **/
  inline QISize Size()
  {
    return QISize(Width(), Height());
  }

  /** setCrop:
   *  @aX1 - left cordinate
   *  @aY1 - top cordinate
   *  @aX2 - right cordinate
   *  @aY2 - bottom cordinate
   *
   *  set the crop info
   **/
  inline void setCrop(uint32_t aX1, uint32_t aY1, uint32_t aX2,
    uint32_t aY2)
  {
    mX1 = aX1;
    mX2 = aX2;
    mY1 = aY1;
    mY2 = aY2;
  }

  /** isZero:
   *
   *  check if the crop is zero
   **/
  inline bool isZero()
  {
    return ((Width() == 0) || (Height() == 0));
  }

  /** isValid:
   *
   *  check if the crop is valid
   **/
  inline bool isValid()
  {
    return ((mX1 < mX2) && (mY1 < mY2));
  }

  /** isValid:
   *
   *  check if the crop is valid given the window
   **/
  inline bool isValid(QISize &aMaxSize)
  {
    return ((Width() > aMaxSize.Width()) || (Height() > aMaxSize.Height()));
  }

private:

  /** mX1:
   *
   *  left cordinate
   **/
  uint32_t mX1;

  /** mY1:
   *
   *  top cordinate
   **/
  uint32_t mY1;

  /** mX2:
   *
   *  right cordinate
   **/
  uint32_t mX2;

  /** mY2:
   *
   *  bottom cordinate
   **/
  uint32_t mY2;
};
#endif //__QICROP_H__
