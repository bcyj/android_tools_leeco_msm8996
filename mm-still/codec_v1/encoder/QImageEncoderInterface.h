/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGE_ENCODER_INTERFACE_H__
#define __QIMAGE_ENCODER_INTERFACE_H__

#define MAX_IMG_OBSERVER_CNT 3

#include "QImageEncoderObserver.h"
#include "QImage.h"
#include "QEncodeParams.h"

/*===========================================================================
 * Class: QIEncodeParams
 *
 * Description: This class represents the interface for all core encoder
 *              classes.
 *
 * Notes: none
 *==========================================================================*/
class QImageEncoderInterface {

public:

  /** QIOutputMode:
   * ENORMAL_OUTPUT: normal output
   * EPIECEWISE_OUTPUT: piecewise output, output is delivered in
   *                    multiple buffers
   *
   *  output mode
   **/
  typedef enum {
    ENORMAL_OUTPUT,
    EPIECEWISE_OUTPUT,
  } QIOutputMode;

  /** QImageEncoderInterface:
   *
   *  constructor
   **/
  QImageEncoderInterface() {
    mObserverCnt = 0;
  }

  /** ~QImageEncoderInterface:
   *
   *  virtual destructor
   **/
  virtual ~QImageEncoderInterface() {}

  /** SetOutputMode:
   *  @aMode: output mode
   *
   *  sets the output mode
   **/
  virtual int SetOutputMode(QIOutputMode aMode) = 0;

  /** Start:
   *
   *  starts the encoder
   **/
  virtual int Start() = 0;

  /** Stop:
   *
   *  stops the encoder
   **/
  virtual int Stop() = 0;

  /** setEncodeParams:
   *  @aParams: encoder parameters
   *
   *  sets encoder parameters
   **/
  virtual int setEncodeParams(QIEncodeParams &aParams) = 0;

  /** addInputImage:
   *  @aImage: image object
   *
   *  add input image to encoder
   **/
  virtual int addInputImage(QImage &aImage) = 0;

  /** addOutputImage:
   *  @aImage: image object
   *
   *  add output image to encoder
   **/
  virtual int addOutputImage(QImage &aImage) = 0;

  /** addObserver:
   *  @aObserver: observer
   *
   *  add observer to the encoder
   **/
  virtual int addObserver(QImageEncoderObserver &aObserver) = 0;

  /** ReleaseSession:
   *
   *  release the current encoding session
   **/
  virtual void ReleaseSession() = 0;

protected:

  /** mObserver:
   *
   *  array of observers
   **/
  QImageEncoderObserver *mObserver[MAX_IMG_OBSERVER_CNT];

  /** mObserverCnt:
   *
   *  observer count
   **/
  uint32_t mObserverCnt;
};

#endif //__QIMAGE_ENCODER_INTERFACE_H__
