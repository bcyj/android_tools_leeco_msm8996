/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QITHREAD_H__
#define __QITHREAD_H__

#include "QIBase.h"

/*===========================================================================
 * Class: QThreadObject
 *
 * Description: This class represents the thread object. If this object is
 *              passed to QIthread object, then Execute() function will be
 *              called in the new thread context
 *
 *
 * Notes: none
 *==========================================================================*/
class QThreadObject {

public:

  /** Execute
   *
   *  virtual function which will be called during thread
   *  execution
   **/
  virtual void Execute() = 0;

  /** ~QThreadObject
   *
   *  virtual destructor
   **/
  virtual ~QThreadObject()
  {
  }
};

/*===========================================================================
 * Class: QIThread
 *
 * Description: This class represents the thread utility
 *
 *
 * Notes: none
 *==========================================================================*/
class QIThread : public QIBase {

public:

  /** QIThread:
   *
   *  contructor
   **/
  QIThread();

  /** ~QIThread:
   *
   *  virtual destructor
   **/
  virtual ~QIThread();

  /** run:
   *  @aData: thread object
   *
   *  function called in new thread context
   **/
  virtual void run(QThreadObject *aData);

  /** StartThread:
   *  @aData: thread object
   *  @aSync: flag to indicate whether to wait for thread start
   *
   *  function to start the thread
   **/
  int StartThread(QThreadObject *aData, bool aSync = true);

  /** JoinThread:
   *
   *  function to join the thread
   **/
  void JoinThread();

  /** IsSelf:
   *
   *  checks if the function call is made in the QIthread's
   *  context
   **/
  inline bool IsSelf()
  {
    return pthread_equal(pthread_self(), mThreadID);
  }

  /** WaitForCompletion:
   *  @apCond: condition object
   *  @apMutex: mutex object
   *  @ms: time in milliseconds
   *
   *  Static function to wait until the timer has expired.
   **/
  static int WaitForCompletion(pthread_cond_t *apCond,
    pthread_mutex_t *apMutex, uint32_t ms);

  /** setThreadName:
   *  @aThreadName: desired name for the thread
   *
   *  function to name the thread
   **/
  inline void setThreadName(const char *aThreadName)
  {
    pthread_setname_np(mThreadID, aThreadName);
  };

private:

  /** sRun:
   *  @aData: thread class object
   *
   *  Static function to executed in new thread context
   **/
  static void* sRun(void *aData);

protected:

  /** mThreadID:
   *
   *  thread id
   **/
  pthread_t mThreadID;

  /** mData:
   *
   *  thread object
   **/
  QThreadObject *mData;

  /** mStarted:
   *
   *  thread start indication flag
   **/
  bool mStarted;

  /** mMutex:
   *
   *  mutex object
   **/
  pthread_mutex_t mMutex;

  /** mCond:
   *
   *  condition object
   **/
  pthread_cond_t mCond;

  /** mReady:
   *
   *  thread ready indication flag
   **/
  bool mReady;
};

#endif //__QITHREAD_H__
