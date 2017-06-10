/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGE_ENCODER_OBSERVER_H__
#define __QIMAGE_ENCODER_OBSERVER_H__

#include "QImage.h"
#include "QIBuffer.h"

/*===========================================================================
 * Class: QImageEncoderObserver
 *
 * Description: This class represents the encoder observer interface
 *
 * Notes: none
 *==========================================================================*/
class QImageEncoderObserver {

public:

  /** EncodeErrorType:
   *  ERROR_GENERAL: generic error
   *  ERROR_THUMBNAIL_DROPPED: thumbnail is dropped
   *
   *  encode error
   **/
  typedef enum EncodeErrorType {
    ERROR_GENERAL,
    ERROR_THUMBNAIL_DROPPED,
    ERROR_OVERFLOW
  } EncodeErrorType;

  /** ~QImageEncoderObserver:
   *
   *  virtual destructor
   **/
  virtual ~QImageEncoderObserver() {}

  /** EncodeComplete:
   *  @aOutputImage: image object
   *
   *  callback when the encoding is completed
   **/
  virtual int EncodeComplete(QImage *aOutputImage) = 0;

  /** EncodeError:
   *  @aErrorType: error type
   *
   *  callback when the encoding error is incurred
   **/
  virtual int EncodeError(EncodeErrorType aErrorType) = 0;

  /** OutputFragment:
   *  @aBuffer: output buffer
   *
   *  callback when the encoding is completed for one buffer.
   *
   *  This function is called only when piecewise mode is set
   **/
  virtual int OutputFragment(QIBuffer &aBuffer) = 0;
};

#endif //__QIMAGE_ENCODER_OBSERVER_H__
