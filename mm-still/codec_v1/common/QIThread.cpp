/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QIThread.h"
#include "QICommon.h"
#include "errno.h"
#include "QImageSWEncoder.h"

/*===========================================================================
 * Function: QIThread
 *
 * Description: QIThread constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIThread::QIThread()
{
  mData = NULL;
  mThreadID = -1;
  mStarted = false;
  mReady = false;
  pthread_mutex_init(&mMutex, NULL);
  pthread_cond_init(&mCond, NULL);
}

/*===========================================================================
 * Function: QIThread
 *
 * Description: QIThread destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIThread::~QIThread()
{
  if (mReady) {
    QIDBG_MED("%s:%d] thread id %x %x", __func__, __LINE__,
      (uint32_t)mThreadID, (uint32_t)pthread_self());
    if (!IsSelf())
      pthread_join(mThreadID, NULL);
  }
  pthread_mutex_destroy(&mMutex);
  pthread_cond_destroy(&mCond);
}

/*===========================================================================
 * Function: JoinThread
 *
 * Description: join the thread
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIThread::JoinThread()
{
  if (mReady) {
    QIDBG_MED("%s:%d] thread id %x %x", __func__, __LINE__,
      (uint32_t)mThreadID, (uint32_t)pthread_self());
    if (!IsSelf())
      pthread_join(mThreadID, NULL);
    mReady = false;
  }
  mThreadID = -1;
}

/*===========================================================================
 * Function: run
 *
 * Description: The thread loop function. If the thread object is passed
 *              Execute() method of the thread object will be called.
 *              Otherwise user has to implement this function
 *
 * Input parameters:
 *   aData - pointer to the thread object
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIThread::run(QThreadObject *aData)
{
  if (mData)
    mData->Execute();
}

/*===========================================================================
 * Function: sRun
 *
 * Description: static function called by the pthread API. member function
 *              of the class will be called from this function.
 *
 * Input parameters:
 *   aData - pointer to the QIThread object
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void* QIThread::sRun(void *aData)
{
  QIThread *lThread = (QIThread *)aData;
  QI_LOCK(&lThread->mMutex);
  lThread->mReady = true;
  QI_SIGNAL(&lThread->mCond);
  QI_UNLOCK(&lThread->mMutex);
  QIDBG_LOW("%s:%d] %p", __func__, __LINE__, lThread->mData);
  lThread->run(lThread->mData);
  return NULL;
}

/*===========================================================================
 * Function: StartThread
 *
 * Description: Starts the execution of the thread
 *
 * Input parameters:
 *   aData - pointer to the QThreadObject object
 *   aSync - synchronizes the thread start. if set, this function will be
 *           returned only after the thread is started.
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QIThread::StartThread(QThreadObject *aData, bool aSync)
{
  int lStatus = QI_SUCCESS;

  pthread_mutex_lock(&mMutex);
  mReady = false;
  mData = aData;
  QIDBG_LOW("%s:%d] %p", __func__, __LINE__, mData);
  lStatus = pthread_create(&mThreadID, NULL, &QIThread::sRun, (void *)this);
  if (lStatus < 0) {
    QIDBG_ERROR("%s: pthread creation failed %d", __func__, errno);
    pthread_mutex_unlock(&mMutex);
    return QI_ERR_GENERAL;
  }

  if((false == mReady) && (true == aSync)) {
    QIDBG_MED("%s: before wait", __func__);
    pthread_cond_wait(&mCond, &mMutex);
  }
  QIDBG_MED("%s: after wait", __func__);
  pthread_mutex_unlock(&mMutex);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: WaitForCompletion
 *
 * Description: Static function to wait until the timer has expired.
 *
 * Input parameters:
 *   apCond - pointer to the pthread condition
 *   apMutex - pointer to the pthread mutex
 *   aMs - time in milli seconds
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QIThread::WaitForCompletion(pthread_cond_t *apCond,
  pthread_mutex_t *apMutex, uint32_t aMs)
{
  struct timespec lTs;
  int lrc = clock_gettime(CLOCK_REALTIME, &lTs);
  if (lrc < 0)
    return QI_ERR_GENERAL;

  if (aMs >= 1000) {
    lTs.tv_sec += (aMs / 1000);
    lTs.tv_nsec += ((aMs % 1000) * 1000000);
  } else {
    lTs.tv_nsec += (aMs * 1000000);
  }

  lrc = pthread_cond_timedwait(apCond, apMutex, &lTs);
  if (lrc == ETIMEDOUT) {
    lrc = QI_ERR_TIMEOUT;
  }
  return lrc;
}
