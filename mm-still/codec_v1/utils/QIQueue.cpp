/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QIQueue.h"
#include "QICommon.h"

/*===========================================================================
 * Function: QIQueue
 *
 * Description: QIQueue constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIQueue::QIQueue()
{
  mFront = NULL;
  mRear = NULL;
  mCount = 0;
}

/*===========================================================================
 * Function: ~QIQueue
 *
 * Description: QIQueue destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIQueue::~QIQueue()
{
}

/*===========================================================================
 * Function: Enqueue
 *
 * Description: enqueues the data into the queue
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QIQueue::Enqueue(QIBase *aObj)
{
  QINode *lTemp = new QINode(aObj);

  if (NULL == lTemp) {
    QIDBG_ERROR("%s:%d] Enqueue failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }
  if ((NULL == mFront) || (NULL == mRear)) {
    mFront = mRear = lTemp;
  } else if (mFront == mRear) {
    mRear = lTemp;
    mFront->setNext(mRear);
  } else {
    mRear->setNext(lTemp);
    mRear = lTemp;
  }
  mCount++;
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: Dequeue
 *
 * Description: dequeues the data from the queue
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   base object pointer
 *   NULL in error case
 *
 * Notes: none
 *==========================================================================*/
QIBase* QIQueue::Dequeue()
{
  QIBase* lData = NULL;
  QINode* lNode = NULL;
  if ((NULL == mFront) || (NULL == mRear)) {
    QIDBG_ERROR("%s:%d] Dequeue failed", __func__, __LINE__);
    return NULL;
  }
  lData = mFront->Data();
  lNode = mFront->Next();
  if (mFront == mRear) {
    mRear = NULL;
  }
  delete mFront;
  mFront = lNode;
  mCount--;
  return lData;
}

/*===========================================================================
 * Function: DeleteAll
 *
 * Description: deletes all the data from the queue
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIQueue::DeleteAll()
{
  QIBase* lTemp = NULL;
  while (mCount > 0) {
    lTemp = Dequeue();
    if (NULL == lTemp) {
      QIDBG_ERROR("%s:%d] queue corrupted", __func__, __LINE__);
    }
    delete lTemp;
  }
}

/*===========================================================================
 * Function: Flush
 *
 * Description: flush all the data from the queue
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIQueue::Flush()
{
  QIBase* lTemp = NULL;
  while (mCount > 0) {
    lTemp = Dequeue();
    if (NULL == lTemp) {
      QIDBG_ERROR("%s:%d] queue corrupted", __func__, __LINE__);
    }
  }
}

