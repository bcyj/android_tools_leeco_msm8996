/*****************************************************************************
* Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*****************************************************************************/

#include "QImageCodecFactoryA.h"
#include "QImageSWEncoder.h"
#include "QImageGeminiEncoder.h"

/*===========================================================================
 * Function: QImageCodecFactoryA
 *
 * Description: QImageCodecFactoryA constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageCodecFactoryA::QImageCodecFactoryA()
{
}

/*===========================================================================
 * Function: ~QImageCodecFactoryA
 *
 * Description: QImageCodecFactoryA destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageCodecFactoryA::~QImageCodecFactoryA()
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
QImageEncoderInterface* QImageCodecFactoryA::CreateEncoder(
  QCodecPrefType aPref, QIEncodeParams &aParams)
{
  QImageEncoderInterface *lpEncoder = NULL;
  switch (aPref) {
  case SW_CODEC_PREF:
    lpEncoder = QImageSWEncoder::New(aParams);
    if (NULL == lpEncoder) {
      QIDBG_HIGH("%s:%d] Use Hardware encoder", __func__, __LINE__);
      lpEncoder = QImageGeminiEncoder::New(aParams);
    }
    break;
  case HW_CODEC_PREF:
    lpEncoder = QImageGeminiEncoder::New(aParams);
    if (NULL == lpEncoder) {
      QIDBG_HIGH("%s:%d] Use Software encoder", __func__, __LINE__);
      lpEncoder = QImageSWEncoder::New(aParams);
    }
    break;
  case SW_CODEC_ONLY:
    lpEncoder = QImageSWEncoder::New(aParams);
    break;
  case HW_CODEC_ONLY:
    lpEncoder = QImageGeminiEncoder::New(aParams);
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
QImageDecoderInterface* QImageCodecFactoryA::CreateDecoder(
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
    break;
  default:;
  }
  return lpDecoder;
}
