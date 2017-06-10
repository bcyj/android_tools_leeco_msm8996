/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIQUEUE_H__
#define __QIQUEUE_H__

#include "QINode.h"

/*===========================================================================
 * Class: QIQueue
 *
 * Description: This class represents the queue utility
 *
 *
 * Notes: none
 *==========================================================================*/
class QIQueue : public QIBase {

public:

  /** QIQueue
   *
   *  constructor
   **/
  QIQueue();

  /** ~QIQueue
   *
   *  virtual destructor
   **/
  virtual ~QIQueue();

  /** Enqueue
   *  @aObj: base object to be enqueued
   *
   *  enqueues an object to the queue
   **/
  int Enqueue(QIBase *aObj);

  /** Dequeue
   *
   *  dequeues an object from the queue
   **/
  QIBase* Dequeue();

  /** Count
   *
   *  returns the queue count
   **/
  inline int Count()
  {
    return mCount;
  }

  /** DeleteAll
   *
   *  deletes all elements in the queue
   **/
  void DeleteAll();

  /** Flush
   *
   *  flushes all elements in the queue without calling the
   *  destructor of the base objects
   **/
  void Flush();

private:

  /** mFront
   *
   *  front of the queue
   **/
  QINode *mFront;

  /** mRear
   *
   *  rear of the queue
   **/
  QINode *mRear;

  /** mCount
   *
   *  number of elements in the queue
   **/
  uint32_t mCount;
};

#endif //__QIQUEUE_H__
