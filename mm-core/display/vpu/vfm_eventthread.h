/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef VFM_EVENT_THREAD_H
#define VFM_EVENT_THREAD_H

#include <pthread.h>
#include <sys/poll.h>

#define MAX_NUM_EVT_HANDLER (2)

#define VFM_EVT_THRD_NAME "vfmEventThread"

namespace vpu {

//Utility class for wait-signal pair
class WaitSignal{
public:
    WaitSignal():mSignalledFlag(0){
        pthread_mutex_init(&mMutex, NULL);
        pthread_cond_init(&mCond, NULL);
    }

    void wait(){
        pthread_mutex_lock(&mMutex);
        while(mSignalledFlag != 1){
            pthread_cond_wait(&mCond, &mMutex);
        }
        mSignalledFlag = 0;
        pthread_mutex_unlock(&mMutex);
        return;
    }

    void signal(){
        pthread_mutex_lock(&mMutex);
        mSignalledFlag = 1;
        pthread_cond_signal(&mCond);
        pthread_mutex_unlock(&mMutex);
        return;
    }
private:
    int32_t         mSignalledFlag;
    pthread_mutex_t mMutex;
    pthread_cond_t  mCond;
};

class EventThread {
public:
    struct event_procs_t {
        int32_t fd;
        status_t (*notifyEvent)( void* cookie);
        status_t (*notifyDequeue)(void* cookie, int32_t isInputPort);
        void* cookie;
    };

    EventThread(int32_t enableLogs);
    virtual ~EventThread();

    status_t registerFd(const event_procs_t& evtHdlr);
    status_t unregisterFd(const int32_t& fd);
    status_t listenToDequeueEvents(const int32_t& fd);
    status_t turnOffDequeueEvents(const int32_t& fd);
private:
    enum {
        REGISTER_FD = 0x1,
        UNREGISTER_FD,
        LISTEN_TO_DEQUEUE_EVENT,
        TURN_OFF_DEQUEUE_EVENT,
        EXIT_THREAD,
    };

    struct message_t{
        int64_t       what;
        event_procs_t eventHandler;
    };

    status_t postCommand(const message_t& msg);

    status_t createEventThread();

    status_t requestAndWaitForExit();

    static void* eventThreadWrapper(void*);

    void eventThreadLoop();

    status_t addFd(const event_procs_t& evtHdlr);

    status_t removeFd(const int32_t& fd);

    status_t processListenToDequeueEvents(const int32_t& fd);

    status_t processTurnOffDequeueEvents(const int32_t& fd);

    int32_t logAll() { return (mDebugLogs >= 2); }
    int32_t isDebug() { return (mDebugLogs >= 1); }

    int32_t         mCommandFd;
    pthread_t       mPThread;
    struct pollfd   mPfd[MAX_NUM_EVT_HANDLER + 1];
    int32_t         mNumActiveFds;
    event_procs_t   mEvtHandlers[MAX_NUM_EVT_HANDLER];
    int32_t         mNumActiveEvtHandlers;
    Mutex           mLock;
    message_t       mMsg;
    WaitSignal      waitSignal;
    int32_t         mDebugLogs;
};

};

#endif /* end of include guard: VFM_EVENT_THREAD_H */
