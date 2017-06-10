/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIBUFFER_H__
#define __QIBUFFER_H__

#include "QIBase.h"

/*===========================================================================
 * Class: QIBuffer
 *
 * Description: This class represents the base class of buffer class.
 *
 *
 * Notes: none
 *==========================================================================*/
class QIBuffer : public QIBase {

public:

  /** QIBuffer:
   *  @aAddr - address of the buffer
   *  @aLength - length of the buffer
   *
   *  buffer class constructor
   **/
  QIBuffer(uint8_t *aAddr, uint32_t aLength);

  /** ~QIBuffer:
   *
   *  buffer class destructor
   **/
  virtual ~QIBuffer();

  /** Addr:
   *
   *  returns the address of the buffer
   **/
  virtual uint8_t *Addr();

  /** Length:
   *
   *  returns the length of the buffer
   **/
  virtual uint32_t Length();

  /** Fd:
   *
   *  returns the fd of the buffer
   **/
  virtual int Fd();

  /** FilledLen:
   *
   *  returns the filled length of the buffer
   **/
  virtual uint32_t FilledLen();

  /** Offset:
   *
   *  returns the offset of the buffer
   **/
  virtual uint32_t Offset();

  /** SetFilledLen:
   *  @aLen - filled length of the buffer
   *
   *  sets the filled length of the buffer
   **/
  virtual void SetFilledLen(uint32_t aLen);

  /** SetOffset:
   *  @aOffset - offset of the buffer
   *
   *  sets the offset of the buffer
   **/
  virtual void SetOffset(uint32_t aOffset);

  /** SetAddr:
   *  @aAddr - offset of the buffer
   *
   *  sets the address of the buffer
   **/
  virtual void SetAddr(uint8_t* aAddr);

  /** SetFd:
   *  @aFd - fd of the buffer
   *
   *  sets the fd of the buffer
   **/
  virtual void SetFd(int aFd);

protected:

  /** mAddr:
   *
   *  address pointer
   **/
  uint8_t *mAddr;

  /** mLength:
   *
   *  buffer length
   **/
  uint32_t mLength;

  /** mFilledLen:
   *
   *  buffer filled length
   **/
  uint32_t mFilledLen;

  /** mFd:
   *
   *  buffer fd
   **/
  int mFd;

  /** mOffset:
   *
   *  buffer offset
   **/
  uint32_t mOffset;
};

#endif //__QIBUFFER_H__
