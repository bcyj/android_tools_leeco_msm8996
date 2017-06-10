/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGE_DECODER_INTERFACE_H__
#define __QIMAGE_DECODER_INTERFACE_H__

#define MAX_IMG_OBSERVER_CNT 3

#include "QImageDecoderObserver.h"
#include "QImage.h"
#include "QIDecoderParams.h"

/*===========================================================================
 * Class: QImageDecoderInterface
 *
 * Description: This class represents the interface for all core encoder
 *              classes.
 *
 * Notes: none
 *==========================================================================*/
class QImageDecoderInterface {

public:

  /** QImageDecoderInterface:
   *
   *  contructor
   **/
  QImageDecoderInterface()
  {
    mObserverCnt = 0;
  }

  /** ~QImageDecoderInterface:
   *
   *  virtual destructor
   **/
  virtual ~QImageDecoderInterface()
  {
  }

  /** Start:
   *
   *  starts the decoder
   **/
  virtual int Start() = 0;

  /** Stop:
   *
   *  stops the decoder
   **/
  virtual int Stop() = 0;

  /** setDecodeParams:
   *  @aParams: decoder parameters
   *
   *  sets decode parameters
   **/
  virtual int setDecodeParams(QIDecodeParams &aParams) = 0;

  /** addInputImage:
   *  @aImage: image object
   *
   *  add input image to decoder
   **/
  virtual int addInputImage(QImage &aImage) = 0;

  /** addOutputImage:
   *  @aImage: image object
   *
   *  add output image to decoder
   **/
  virtual int addOutputImage(QImage &aImage) = 0;

  /** addObserver:
   *  @aObserver: observer
   *
   *  add observer to the decoder
   **/
  virtual int addObserver(QImageDecoderObserver &aObserver) = 0;

protected:

  /** mObserver:
   *
   *  array of observers
   **/
  QImageDecoderObserver *mObserver[MAX_IMG_OBSERVER_CNT];

  /** mObserverCnt:
   *
   *  observer count
   **/
  uint32_t mObserverCnt;
};

#endif //__QIMAGE_DECODER_INTERFACE_H__
