/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QIHeapBuffer.h"

/*===========================================================================
 * Function: QIHeapBuffer
 *
 * Description: QIHeapBuffer constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIHeapBuffer::QIHeapBuffer()
: QIBuffer(NULL, 0)
{
}

/*===========================================================================
 * Function: ~QIHeapBuffer
 *
 * Description: QIHeapBuffer destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIHeapBuffer::~QIHeapBuffer()
{
  if (mAddr) {
    free(mAddr);
    mAddr = NULL;
  }
}

/*===========================================================================
 * Function: New
 *
 * Description: 2 phase constructor for heap buffer
 *
 * Input parameters:
 *   aLength - length of the buffer
 *
 * Return values:
 *   pointer to QIHeapBuffer object
 *
 * Notes: none
 *==========================================================================*/
QIHeapBuffer *QIHeapBuffer::New(uint32_t aLength)
{
  QIHeapBuffer *lBuffer = new QIHeapBuffer();
  if (NULL == lBuffer) {
    return NULL;
  }
  lBuffer->mAddr = (uint8_t *)malloc(aLength);
  if (NULL == lBuffer->mAddr) {
    delete lBuffer;
    return NULL;
  }
  lBuffer->mLength = aLength;
  return lBuffer;
}
