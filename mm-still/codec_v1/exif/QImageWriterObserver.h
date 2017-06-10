/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGE_WRITER_OBSERVER_H__
#define __QIMAGE_WRITER_OBSERVER_H__

#include "QIBuffer.h"

/*===========================================================================
 * Class: QImageWriterObserver
 *
 * Description: This class represents the image writer observer interface
 *
 * Notes: none
 *==========================================================================*/
class QImageWriterObserver {

public:

  /** ErrorType
   *  ERR_THUMBNAIL_DROPPED: Thumbnail cannot be fit into the EXIF
   *                        header due to image size limitation
   *  ERR_BUFFER_FULL: Buffer is full
   *  ERR_GENERAL: Unknown error is occurred
   *
   *  image writer observer error values
   **/
  typedef enum {
    ERR_THUMBNAIL_DROPPED,
    ERR_BUFFER_FULL,
    ERR_GENERAL,
  } ErrorType;

  /** ~QImageWriterObserver
   *
   *  virtual destructor
   **/
  virtual ~QImageWriterObserver()
  {
  }

  /** WriteComplete
   *  @aBuffer: buffer which is filled by the composer
   *
   *  Callback issued when the image header is written
   *  successfully into the buffer
   **/
  virtual void WriteComplete(QIBuffer &aBuffer) = 0;

  /** WriteFragmentDone
   *  @aBuffer: buffer which is filled by the composer
   *
   *  Callback issued when the buffer passed to the composer is
   *  written and is insufficient.
   *  Note that if this callback is issued, the client should call
   *  addBuffer to send more buffers
   **/
  virtual void WriteFragmentDone(QIBuffer &aBuffer) = 0;

  /** WriteError
   *  @aErrorType: Error type
   *
   *  Callback issued when error is occured during composition
   **/
  virtual void WriteError(ErrorType aErrorType) = 0;
};

#endif //__QIMAGE_WRITER_OBSERVER_H__
