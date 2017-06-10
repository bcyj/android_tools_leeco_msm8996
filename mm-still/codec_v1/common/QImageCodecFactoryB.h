/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGECODECFACTORY_B_H__
#define __QIMAGECODECFACTORY_B_H__

#include "QImageCodecFactory.h"

/*===========================================================================
 * Class: QImageCodecFactoryB
 *
 * Description: This class represents the codec factory for B family chipsets
 *
 *
 * Notes: none
 *==========================================================================*/
class QImageCodecFactoryB : public QImageCodecFactory
{
public:

  /** QImageCodecFactoryB:
   *
   *  constructor
   **/
  QImageCodecFactoryB();

  /** ~QImageCodecFactoryB:
   *
   *  constructor
   **/
  virtual ~QImageCodecFactoryB();

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

#endif //__QIMAGECODECFACTORY_B_H__
