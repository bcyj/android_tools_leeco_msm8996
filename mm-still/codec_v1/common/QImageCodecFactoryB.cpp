/*****************************************************************************
* Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QImageCodecFactoryB.h"
#include "QImageSWEncoder.h"
#include "QImageHWDecoder10.h"
#include "QImageHWEncoder10.h"
#include "QImageQ6Encoder.h"
#include "QImageHybridEncoder.h"

/*===========================================================================
 * Function: QImageCodecFactoryB
 *
 * Description: QImageCodecFactoryB constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageCodecFactoryB::QImageCodecFactoryB()
{
}

/*===========================================================================
 * Function: ~QImageCodecFactoryB
 *
 * Description: QImageCodecFactoryB destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageCodecFactoryB::~QImageCodecFactoryB()
{
}

/*===========================================================================
 * Function: CreateEncoder
 *
 * Description: creates an encoder object and pass it to the client
 *
 * Input parameters:
 *   aPref - preference type
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageEncoderInterface* QImageCodecFactoryB::CreateEncoder(
  QCodecPrefType aPref, QIEncodeParams &aParams)
{
  QImageEncoderInterface *lpEncoder = NULL;
  switch (aPref) {
  case SW_CODEC_PREF:
    lpEncoder = QImageHybridEncoder::New(aParams);
    if (NULL == lpEncoder) {
      QIDBG_HIGH("%s:%d] Use hardware encoder", __func__, __LINE__);
      lpEncoder = QImageHW10Encoder::New(aParams);
    }
    break;
  case HW_CODEC_PREF:
#ifdef JPEG_USE_QDSP6_ENCODER
    lpEncoder = QImageHybridEncoder::New(aParams);
#else
    lpEncoder = QImageHW10Encoder::New(aParams);
#endif
    if (NULL == lpEncoder) {
      QIDBG_HIGH("%s:%d] Use Software encoder", __func__, __LINE__);
      lpEncoder = QImageHybridEncoder::New(aParams);
    }
    break;
  case SW_CODEC_ONLY:
    lpEncoder = QImageHybridEncoder::New(aParams);
    break;
  case HW_CODEC_ONLY:
    lpEncoder = QImageHW10Encoder::New(aParams);
    break;
  default:;
  }
  return lpEncoder;
}

/*===========================================================================
 * Function: CreateDecoder
 *
 * Description: creates an decoder object and pass it to the client
 *
 * Input parameters:
 *   aPref - preference type
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageDecoderInterface* QImageCodecFactoryB::CreateDecoder(
  QCodecPrefType aPref, QIDecodeParams &aParams)
{
  QImageDecoderInterface *lpDecoder = NULL;
  switch (aPref) {
  case SW_CODEC_PREF:
    break;
  case HW_CODEC_PREF:
    break;
  case SW_CODEC_ONLY:
    break;
  case HW_CODEC_ONLY:
    lpDecoder = QImageHW10Decoder::New(aParams);
    break;
  default:;
  }
  return lpDecoder;
}
