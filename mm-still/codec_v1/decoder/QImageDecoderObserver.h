/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGE_DECODER_OBSERVER_H__
#define __QIMAGE_DECODER_OBSERVER_H__

#include "QImage.h"
#include "QIBuffer.h"

/*===========================================================================
 * Class: QImageDecoderObserver
 *
 * Description: This class represents the decoder observer interface
 *
 * Notes: none
 *==========================================================================*/
class QImageDecoderObserver {

public:

  /** EncodeErrorType:
   *  ERROR_GENERAL: generic error
   *  ERROR_HUFFMAN_DECODE: huffman decode error
   *
   *  decode error
   **/
  typedef enum EncodeErrorType {
    ERROR_GENERAL,
    ERROR_HUFFMAN_DECODE,
  } DecodeErrorType;

  /** ~QImageDecoderObserver:
   *
   *  virtual destructor
   **/
  virtual ~QImageDecoderObserver()
  {
  }

  /** DecodeComplete:
   *  @aOutputImage: image object
   *
   *  callback when the decoding is completed
   **/
  virtual int DecodeComplete(QImage *aOutputImage) = 0;

  /** DecodeError:
   *  @aErrorType: error type
   *
   *  callback when the decoding error is incurred
   **/
  virtual int DecodeError(DecodeErrorType aErrorType) = 0;
};

#endif //__QIMAGE_DECODER_OBSERVER_H__
