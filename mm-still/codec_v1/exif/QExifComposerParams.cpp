/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QExifComposerParams.h"


/*===========================================================================
 * Function: QExifComposerParams
 *
 * Description: QExifComposerParams constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QExifComposerParams::QExifComposerParams()
{
  mApp2Len = 0;
  mExifObjInfo = NULL;
  mMainEncodeParams = NULL;
  mThumbEncodeParams = NULL;
  mMainSS = QI_H2V2;
  mThumbSS = QI_H2V2;
  mMobicatStr = 0;
  mEnableMobicat = false;
  mStatsPayload = 0;
  mStatsPayloadSize = 0;
  mEnable3A = false;
}

/*===========================================================================
 * Function: SetEncodeParams
 *
 * Description: set the exif composer parameters
 *
 * Input parameters:
 *   aParams - reference to encode params object
 *   aThumb - thumbnail indicator
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QExifComposerParams::SetEncodeParams(QIEncodeParams &aParams, bool aThumb)
{
  if (aThumb)
    mThumbEncodeParams = &aParams;
  else
    mMainEncodeParams = &aParams;
}

/*===========================================================================
 * Function: SetSubSampling
 *
 * Description: set the exif composer subsampling
 *
 * Input parameters:
 *   aSS - subsampling type
 *   aThumb - thumbnail indicator
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QExifComposerParams::SetSubSampling(QISubsampling aSS, bool aThumb)
{
  if(aThumb)
    mThumbSS = aSS;
  else
    mMainSS = aSS;
}
