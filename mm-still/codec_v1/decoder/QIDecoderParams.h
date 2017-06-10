/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIDECODEPARAMS_H__
#define __QIDECODEPARAMS_H__

#include "QIParams.h"
#include "QICrop.h"
#include "QISize.h"

extern "C" {
#include "jpeg_header.h"
}

/*===========================================================================
 * Class: QIDecodeParams
 *
 * Description: This class represents the decode parameters
 *
 * Notes: none
 *==========================================================================*/
class QIDecodeParams :  public QIBase {

public:

  /** QIDecodeParams:
   *
   *  constructor
   **/
  QIDecodeParams();

  /** ~QIDecodeParams:
   *
   *  virtual destructor
   **/
  ~QIDecodeParams();

  /** FrameInfo:
   *
   *  returns frame info for decoder
   **/
  inline jpeg_frame_info_t *FrameInfo()
  {
    return mFrameInfo;
  }

  /** Crop:
   *
   *  returns crop window for decoding
   **/
  inline QICrop &Crop()
  {
    return mCrop;
  }

  /** InputSize:
   *
   *  returns input image size
   **/
  inline QISize &InputSize()
  {
    return mInputSize;
  }

  /** OutputSize:
   *
   *  returns output image size
   **/
  inline QISize &OutputSize()
  {
    return mOutputSize;
  }

  /** setCrop:
   *  @aCrop: crop object
   *
   *  sets the image crop
   **/
  inline void setCrop(QICrop &aCrop)
  {
    mCrop = aCrop;
  }

  /** setOutputSize:
   *  @aSize: image size
   *
   *  sets the output image size
   **/
  inline void setOutputSize(QISize &aSize)
  {
    mOutputSize = aSize;
  }

  /** setInputSize:
   *  @aSize: image size
   *
   *  sets the input image size
   **/
  inline void setInputSize(QISize &aSize)
  {
    mInputSize = aSize;
  }

  /** setFrameInfo:
   *  @aFrameInfo: image frame info
   *
   *  sets the image frame info
   **/
  inline void setFrameInfo(jpeg_frame_info_t *aFrameInfo)
  {
    mFrameInfo = aFrameInfo;
  }

private:

  /** mFrameInfo:
   *
   *  image frame info
   **/
  jpeg_frame_info_t *mFrameInfo;

  /** mCrop:
   *
   *  decode crop info
   **/
  QICrop mCrop;

  /** mOutputSize:
   *
   *  decode output size
   **/
  QISize mOutputSize;

  /** mInputSize:
   *
   *  decode input size
   **/
  QISize mInputSize;
};

#endif //__QIDECODEPARAMS_H__
