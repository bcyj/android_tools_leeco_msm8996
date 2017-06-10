/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QIBuffer.h"

/*===========================================================================
 * Function: QIBuffer
 *
 * Description: QIBuffer constuctor
 *
 * Input parameters:
 *   aAddr - Address of the buffer
 *   aLength - Length of the buffer
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIBuffer::QIBuffer(uint8_t *aAddr, uint32_t aLength)
{
  mAddr = aAddr;
  mLength = aLength;
  mFd = -1;
  mOffset = 0;
  mFilledLen = 0;
}

/*===========================================================================
 * Function: ~QIBuffer
 *
 * Description: QIBuffer destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIBuffer::~QIBuffer()
{
}

/*===========================================================================
 * Function: Addr
 *
 * Description: Get the Address of the buffer
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   address of the buffer
 *
 * Notes: none
 *==========================================================================*/
uint8_t *QIBuffer::Addr()
{
  return mAddr;
}

/*===========================================================================
 * Function: Length
 *
 * Description: Get the length of the buffer
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   length of the buffer
 *
 * Notes: none
 *==========================================================================*/
uint32_t QIBuffer::Length()
{
  return mLength;
}

/*===========================================================================
 * Function: Fd
 *
 * Description: Get the fd of the buffer
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   fd of the buffer
 *
 * Notes: none
 *==========================================================================*/
int QIBuffer::Fd()
{
  return mFd;
}

/*===========================================================================
 * Function: FilledLen
 *
 * Description: Get the filled length of the buffer
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   filled len of the buffer
 *
 * Notes: none
 *==========================================================================*/
uint32_t QIBuffer::FilledLen()
{
  return mFilledLen;
}

/*===========================================================================
 * Function: SetFilledLen
 *
 * Description: Set the filled length of the buffer
 *
 * Input parameters:
 *   aLen - length to be set
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIBuffer::SetFilledLen(uint32_t aLen)
{
  mFilledLen = aLen;
}

/*===========================================================================
 * Function: Offset
 *
 * Description: Get the offset of the buffer from which data can be read
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   offset to the buffer
 *
 * Notes: none
 *==========================================================================*/
uint32_t QIBuffer::Offset()
{
  return mOffset;
}

/*===========================================================================
 * Function: SetOffset
 *
 * Description: Set the offset of the buffer from which data can be read
 *
 * Input parameters:
 *   aOffset - offset of the buffer
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIBuffer::SetOffset(uint32_t aOffset)
{
  mOffset = aOffset;
}

/*===========================================================================
 * Function: SetAddr
 *
 * Description: Set the starting address of the buffer
 *
 * Input parameters:
 *   aAddr - Addr of the buffer
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIBuffer::SetAddr(uint8_t *aAddr)
{
  mAddr = aAddr;
}

/*===========================================================================
 * Function: SetFd
 *
 * Description: Set the Fd for the buffer
 *
 * Input parameters:
 *   aFd - Fd of the buffer
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIBuffer::SetFd(int aFd)
{
  mFd = aFd;
}
