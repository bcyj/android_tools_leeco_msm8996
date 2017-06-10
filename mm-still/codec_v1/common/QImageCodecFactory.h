/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGECODECFACTORY_H__
#define __QIMAGECODECFACTORY_H__

#include "QImageEncoderInterface.h"
#include "QImageDecoderInterface.h"

/*===========================================================================
 * Class: QImageCodecFactory
 *
 * Description: This class represents the base class for the codec factory
 *
 *
 * Notes: none
 *==========================================================================*/
class QImageCodecFactory {

public:

  /** QCodecPrefType:
   *  SW_CODEC_PREF:  software preferred
   *  HW_CODEC_PREF: hardware preferred
   *  SW_CODEC_ONLY: software codec only
   *  HW_CODEC_ONLY: hardware codec only
   *
   *  codec preference enumeration
   *
   **/
  typedef enum _QCodecPrefType {
    SW_CODEC_PREF,
    HW_CODEC_PREF,
    SW_CODEC_ONLY,
    HW_CODEC_ONLY,
  } QCodecPrefType;

  /** ~QImageCodecFactory:
   *
   *  virtual destructor for the image codec factory base class
   *
   **/
  virtual ~QImageCodecFactory()
  {
  }

  /** CreateEncoder:
   *  @aPref - codec preference
   *  @aParams - encode parameters
   *
   *  creates the encoder and returns the encoder object to the
   *  client
   **/
  virtual QImageEncoderInterface *CreateEncoder(QCodecPrefType aPref,
    QIEncodeParams &aParams) = 0;

  /** CreateDecoder:
   *  @aPref - codec preference
   *  @aParams - encode parameters
   *
   *  creates the decoder and returns the decoder object to the
   *  client
   **/
  virtual QImageDecoderInterface *CreateDecoder(QCodecPrefType aPref,
    QIDecodeParams &aParams) = 0;
};

#endif //__QIMAGECODECFACTORY_H__
