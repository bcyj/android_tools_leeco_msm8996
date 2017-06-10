/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGE_READER_OBSERVER_H__
#define __QIMAGE_READER_OBSERVER_H__

#include "QIBuffer.h"

/*===========================================================================
 * Class: QImageReaderObserver
 *
 * Description: This class represents the image reader observer interface
 *
 * Notes: none
 *==========================================================================*/
class QImageReaderObserver {

public:

  /** ErrorType
   *  ERR_BUFFER_FULL: Buffer to be parsed is full
   *  ERR_GENERAL: Unknown error
   *
   *  image reader observer error values
   **/
  typedef enum {
    ERR_BUFFER_FULL,
    ERR_GENERAL,
  } ErrorType;

  /** ~QImageReaderObserver
   *
   *  virtual destructor
   **/
  virtual ~QImageReaderObserver()
  {
  }

  /** ReadComplete
   *  @aBuffer: Image buffer parsed
   *
   *  Callback issued when the Image buffer is parsed successfully
   *  by the component
   **/
  virtual void ReadComplete(QIBuffer &aBuffer) = 0;

  /** ReadFragmentDone
   *  @aBuffer: Image buffer parsed
   *
   *  Callback issued when the Image buffer passed to the parser
   *  is read.
   *  Note that if this callback is issued, the client should call
   *  addBuffer to send more buffers
   **/
  virtual void ReadFragmentDone(QIBuffer &aBuffer) = 0;

  /** ReadError
   *  @aErrorType: Error type
   *
   *  Callback issued when error is occured during parsing
   **/
  virtual void ReadError(ErrorType aErrorType) = 0;
};

#endif //__QIMAGE_READER_OBSERVER_H__
