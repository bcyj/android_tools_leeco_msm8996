/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QIDecoderParams.h"

/*===========================================================================
 * Function: QIDecodeParams
 *
 * Description: QIDecodeParams constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIDecodeParams::QIDecodeParams()
{
  mInputSize = QISize(0, 0);
  mOutputSize = QISize(0, 0);
  mCrop = QICrop();
  mFrameInfo = NULL;
}

/*===========================================================================
 * Function: ~QIDecodeParams
 *
 * Description: QIDecodeParams destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIDecodeParams::~QIDecodeParams()
{
}
