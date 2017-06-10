/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QISIZE_H__
#define __QISIZE_H__

/*===========================================================================
 * Class: QISize
 *
 * Description: This class represents the dimension
 *
 *
 * Notes: none
 *==========================================================================*/
class QISize {

public:

  /** QISize:
   *
   *  constructor
   **/
  QISize()
  {
    mWidth = mHeight = 0;
  }

  /** QISize:
   *  @aWidth: width of the image
   *  @aHeight: height of the image
   *
   *  constructor with size initialization
   **/
  QISize(uint32_t aWidth, uint32_t aHeight)
  {
    mWidth = aWidth;
    mHeight = aHeight;
  }

  /** QISize:
   *
   *  virtual destructor
   **/
  ~QISize()
  {
  }

  /** Width:
   *
   *  returns width of the image
   **/
  inline uint32_t Width()
  {
    return mWidth;
  }

  /** AspectRatio:
   *
   *  returns aspect ratio of the image
   **/
  inline float AspectRatio()
  {
    return IsZero() ? 1.0 : (float)mWidth/mHeight;
  }

  /** Height:
   *
   *  returns height of the image
   **/
  inline uint32_t Height()
  {
    return mHeight;
  }

  /** Length:
   *
   *  returns length of the image
   **/
  inline uint32_t Length()
  {
    return mHeight * mWidth;
  }

  /** setWidth:
   *  @aWidth: frame width
   *
   *  sets width of the image
   **/
  inline void setWidth(uint32_t aWidth)
  {
    mWidth = aWidth;
  }

  /** setHeight:
   *  @aHeight: frame height
   *
   *  sets height of the image
   **/
  inline void setHeight(uint32_t aHeight)
  {
    mHeight = aHeight;
  }

  /** operator==:
   *  @aOther: QISize object to be compared
   *
   *  check if the QISize object are similar
   **/
  inline bool operator==(const QISize& aOther)
  {
    return ((mWidth == aOther.mWidth) &&
      (mHeight == aOther.mHeight));
  }

  /** operator!=:
   *  @aOther: QISize object to be compared
   *
   *  check if the QISize object are different
   **/
  inline bool operator!=(const QISize& aOther)
  {
    return ((mWidth != aOther.mWidth) ||
      (mHeight != aOther.mHeight));
  }

  /** IsZero:
   *
   *  check if either dimension is zero
   **/
  inline bool IsZero()
  {
    return ((mHeight == 0) || (mWidth == 0));
  }

private:

  /** mWidth:
   *
   *  object width
   **/
  uint32_t mWidth;

  /** mHeight:
   *
   *  object height
   **/
  uint32_t mHeight;
};

#endif //__QISIZE_H__
