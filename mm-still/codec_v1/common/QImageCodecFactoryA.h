/*****************************************************************************
* Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*****************************************************************************/

#ifndef __QIMAGECODECFACTORY_A_H__
#define __QIMAGECODECFACTORY_A_H__

#include "QImageCodecFactory.h"

/*===========================================================================
 * Class: QImageCodecFactoryA
 *
 * Description: This class represents the codec factory for A family chipsets
 *
 *
 * Notes: none
 *==========================================================================*/
class QImageCodecFactoryA : public QImageCodecFactory
{
public:

  /** QImageCodecFactoryA:
   *
   *  constructor
   **/
  QImageCodecFactoryA();

  /** ~QImageCodecFactoryA:
   *
   *  constructor
   **/
  virtual ~QImageCodecFactoryA();

  /** CreateEncoder:
   *  @aPref - codec preference
   *  @aParams - encode parameters
   *
   *  creates the encoder and returns the encoder object to the
   *  client
   **/
  QImageEncoderInterface *CreateEncoder(QCodecPrefType aPref,
    QIEncodeParams &aParams);

  /** CreateDecoder:
   *  @aPref - codec preference
   *  @aParams - encode parameters
   *
   *  creates the decoder and returns the decoder object to the
   *  client
   **/
  QImageDecoderInterface *CreateDecoder(QCodecPrefType aPref,
    QIDecodeParams &aParams);
};

#endif //__QIMAGECODECFACTORY_A_H__
