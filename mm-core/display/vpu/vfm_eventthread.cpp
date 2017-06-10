/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <sys/prctl.h>
#include <sys/eventfd.h>

#include "vfm_utils.h"

namespace vpu {

/*****************************************************************************
    Public methods
*****************************************************************************/
EventThread::EventThread( int32_t enableDebugLogs)
    : mCommandFd(-1),
      mNumActiveFds(0),
      mNumActiveEvtHandlers(0),
      mDebugLogs(enableDebugLogs)
{
    ALOGD_IF(logAll(), "%s: E", __func__);
    /* Initialize pfd array */
    memset(mPfd, 0, (MAX_NUM_EVT_HANDLER + 1) * sizeof(struct pollfd));

    /* start the thread */
    createEventThread();
}

EventThread::~EventThread()
{
    ALOGD_IF(logAll(), "%s: E", __func__);
    /* exit the thread */
    requestAndWaitForExit();
}

status_t EventThread::registerFd(const event_procs_t& evtHdlr)
{
    status_t err = NO_ERROR;
    message_t msg;

    ALOGD_IF(isDebug(), "%s: E, fd: %d, ntfyEvt: %p, ntfyDeq: %p, cookie: %p",
                __func__, evtHdlr.fd, evtHdlr.notifyEvent,
                evtHdlr.notifyDequeue, evtHdlr.cookie);
    /* Input validation */

    msg.what = REGISTER_FD;
    msg.eventHandler = evtHdlr;
    postCommand(msg);

    return err;
}

status_t EventThread::unregisterFd(const int32_t& fd)
{
    status_t err = NO_ERROR;
    message_t msg;

    ALOGD_IF(isDebug(), "%s: E, fd: %d", __func__, fd);
    msg.what = UNREGISTER_FD;
    msg.eventHandler.fd = fd;
    postCommand(msg);

    return err;
}

status_t EventThread::listenToDequeueEvents(const int32_t& fd)
{
    status_t err = NO_ERROR;
    message_t msg;

    ALOGD_IF(isDebug(), "%s: E, fd: %d", __func__, fd);
    msg.what = LISTEN_TO_DEQUEUE_EVENT;
    msg.eventHandler.fd = fd;
    postCommand(msg);

    return err;
}

status_t EventThread::turnOffDequeueEvents(const int32_t& fd)
{
    status_t err = NO_ERROR;
    message_t msg;

    ALOGD_IF(isDebug(), "%s: E, fd: %d", __func__, fd);
    msg.what = TURN_OFF_DEQUEUE_EVENT;
    msg.eventHandler.fd = fd;
    postCommand(msg);

    return err;
}
/*****************************************************************************
    Private methods
*****************************************************************************/
status_t EventThread::postCommand(const message_t& msg)
{
    status_t err = NO_ERROR;

    mLock.lock();
    mMsg = msg;
    mLock.unlock();

    ALOGD_IF(isDebug(), "%s: E, cmdFd: %d what: %lld",
                __func__, mCommandFd, msg.what);
    if(-1 != mCommandFd){
        int32_t ret;

        ret = write(mCommandFd, &msg, sizeof(msg.what));
        if(ret < 0 ){
            ALOGE("%s: msg write failed. errno: %d", __func__, errno);
        }
        ALOGD_IF(logAll(), "%s: written %d bytes", __func__, ret);
    }
    //wait until the command is processed
    waitSignal.wait();
    return err;
}

status_t EventThread::createEventThread()
{
    status_t err = NO_ERROR;
    int32_t ret;
    pthread_attr_t attr;

    ALOGD_IF(isDebug(), "%s: E", __func__);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    ret = pthread_create(&mPThread, &attr, eventThreadWrapper, this);
    if(ret) {
        ALOGE("%s: failed to create %s: %s", __func__,
                VFM_EVT_THRD_NAME, strerror(ret));
    }

    ALOGD_IF(isDebug(), "%s: Event thread created", __func__);
    return err;
}

status_t EventThread::requestAndWaitForExit()
{
    status_t err = NO_ERROR;
    message_t msg;

    ALOGD_IF(isDebug(), "%s: E", __func__);
    msg.what = EXIT_THREAD;
    postCommand(msg);

    pthread_join(mPThread, NULL);
    return err;
}

void * EventThread::eventThreadWrapper(void* me)
{
    static_cast<EventThread *>(me)->eventThreadLoop();
    return NULL;
}

void EventThread::eventThreadLoop()
{
    int32_t timeout = -1; //indefinite wait
    message_t msg;
    int32_t err = 0;

    char threadName[64] = VFM_EVT_THRD_NAME;
    prctl(PR_SET_NAME, (unsigned long) &threadName, 0, 0, 0);

    ALOGD_IF(isDebug(), "%s: E", __func__);
    /* FIXME
    setpriority(PRIO_PROCESS, 0, HAL_PRIORITY_URGENT_DISPLAY +
            android::PRIORITY_MORE_FAVORABLE); */

    /* Allocate FDs for polling */
    /* 1st Fd is a local fd so that this thread can be woken up for commands */
    mCommandFd = eventfd(0,0);
    mPfd[0].fd = mCommandFd;
    mPfd[0].events = (POLLIN | POLLERR | POLLNVAL);
    mNumActiveFds++;

    // FIXME: Read once from the fds to clear the first notify
    //pread(mPfd[0].fd, &msg , sizeof(message_t), 0);

    while(((err = poll(mPfd, mNumActiveFds, timeout)) >=0)) {
        ALOGD_IF(logAll(), "%s: [0].revents: %d, [1].revents: %d, [2].revents:"
        " %d", __func__, mPfd[0].revents, mPfd[1].revents, mPfd[2].revents);
        // Handle Poll errors on command fd
        struct pollfd* pfdCmd = &mPfd[0];
        if ((pfdCmd->revents & POLLERR) || (pfdCmd->revents & POLLNVAL)) {
            pfdCmd->revents = 0;
            ALOGE("%s: Error in polling cmd fd. Continue to poll", __func__);
            continue;
        }

        // Handle commands
        if (pfdCmd->revents & POLLIN) {
            message_t msg;
            int64_t what;
            int32_t bytesRead;

            //Lock across mMsg usage
            mLock.lock();
            bytesRead = read(pfdCmd->fd, &what, sizeof(what));
            ALOGD_IF(logAll(), "%s: bytesRead: %d", __func__, bytesRead);
            pfdCmd->revents = 0;

            ALOGD_IF(logAll(), "%s: msg:what: %lld, fd: %d notifyEvt: %p "
                "notifyDequeue: %p, cookie: %p", __func__, msg.what,
            msg.eventHandler.fd, msg.eventHandler.notifyEvent,
            msg.eventHandler.notifyDequeue, msg.eventHandler.cookie);
#if !DISABLE_IOCTL //If IOCTL is not enabled, fd is dummy, hence do not poll
            if(REGISTER_FD == what){
                addFd(mMsg.eventHandler);
            }
            else if(UNREGISTER_FD == what){
                removeFd(mMsg.eventHandler.fd);
            }
            else if(LISTEN_TO_DEQUEUE_EVENT == what){
                processListenToDequeueEvents(mMsg.eventHandler.fd);
            }
            else if(TURN_OFF_DEQUEUE_EVENT == what){
                processTurnOffDequeueEvents(mMsg.eventHandler.fd);
            }
#endif
            //Lock across mMsg usage
            mLock.unlock();
            // Signal that command is processed
            waitSignal.signal();
            if(EXIT_THREAD == what){
                break;
            }
        }

        // Handle errors and events on other fds
        for(int32_t i = 0; i < mNumActiveEvtHandlers; i++){
            struct pollfd* pfd      = &mPfd[i + 1];
            event_procs_t* evtHdler = &mEvtHandlers[i];

            /* errors */
            if(pfd->revents & POLLERR){
                ALOGE("%s: Poll error on [%d].pfd %d",
                    __func__, i+1, pfd->fd);
                /* UnsubsribeEvents/removeFd/pauseListening from here? */
            }
            /* dequeue event occured on input port */
            if(pfd->revents & (POLLOUT | POLLWRNORM)){
                if(evtHdler->notifyDequeue){
                    evtHdler->notifyDequeue(evtHdler->cookie, true);
                }
            }
#if DBG_VPUOUT_HOST
            if(pfd->revents & (POLLIN | POLLRDNORM)){
                if(evtHdler->notifyDequeue){
                    evtHdler->notifyDequeue(evtHdler->cookie, false);
                }
            }
#endif
            /* other events occured */
            if(pfd->revents & POLLPRI){
                if(evtHdler->notifyEvent){
                    evtHdler->notifyEvent(evtHdler->cookie);
                }
            } //POLLPRI

        } //for(mNumActiveEvtHandlers)
    } //while(poll)

    // Handle poll errors
    if(err < 0)
        ALOGE("%s: Event polling failed: %d", __func__, err);

    switch(errno){
        case EINVAL:
            ALOGE("%s: nfds greater than supported", __func__);
            break;

        case EINTR:
            ALOGE("%s: signal caught during poll", __func__);
            break;

        case EAGAIN:
            ALOGE("%s: allocation of driver internal structs failed", __func__);
            break;

        default:
            ALOGE("%s: Unknown poll error %d", __func__, errno);
            break;
    }

    if(mCommandFd != -1){
        close(mCommandFd);
        mCommandFd = -1;
        mNumActiveFds--;
    }
    ALOGD_IF(isDebug(), "%s: Exit from thread", __func__);
    return;
}

/* This function achieves "vector.push_back" functionality on
   mEvtHandlers and mPfd */
status_t EventThread::addFd(const event_procs_t& evtHdlr)
{
    status_t err = NO_ERROR;
    struct pollfd* pfd = NULL;

    ALOGD_IF(isDebug(), "%s: E. fd: %d", __func__, evtHdlr.fd);
    if(mNumActiveEvtHandlers == MAX_NUM_EVT_HANDLER){
        ALOGE("%s: Max number of registrations reached", __func__);
        return NO_MEMORY;
    }
    /* Store in local context */
    mEvtHandlers[mNumActiveEvtHandlers++] = evtHdlr;

    /* Add the fd to poll fd list */
    /* 0 idx is occupied by local command fd */
    pfd = &mPfd[mNumActiveFds++];

    //listen to only errors and events except the dequeue event
    pfd->fd = evtHdlr.fd;
    pfd->events = POLLPRI; /* for other events */
    pfd->events |= POLLERR; /* for errors */
    ALOGD_IF(isDebug(), "%s: fd: %d events: %d", __func__, pfd->fd,
                            pfd->events);
    return err;
}

/* Searches for the fd from the mEvtHandlers and mPfd arrays and removes it */
status_t EventThread::removeFd(const int32_t& fd)
{
    status_t err = NO_ERROR;
    int32_t found = 0, i = 0;

    ALOGD_IF(isDebug(), "%s: E, fd: %d", __func__, fd);
    /* Input validation */
    if(fd < 0){
        return BAD_VALUE;
    }

    /* Search for the index to be removed */
    for(i = 0; i < mNumActiveEvtHandlers; i++){
        if(mEvtHandlers[i].fd == fd){
            found = 1;
            break;
        }
    }

    if (found){
        /* Nothing to be done if tail is the one to be removed
            If tail is not the one to be removed, purge the hole */
        //TODO: Remove this purging code and instead set the pfdRemove = -1
        int32_t tailIdx = mNumActiveEvtHandlers - 1;
        if(tailIdx > i ){
            for(; i < tailIdx; i++){
                //First element of pollfd is local command fd
                struct pollfd* pfdRemove = &mPfd[i + 1];
                event_procs_t* mEvtHdlrRemove = &mEvtHandlers[i];

                /* Move the next element to current */
                *pfdRemove      = *(pfdRemove + 1);
                *mEvtHdlrRemove = *(mEvtHdlrRemove + 1);
            }
        }
        mNumActiveEvtHandlers--;
        mNumActiveFds--;
    }
    else {
        ALOGE("%s: Trying to unregister without registering: %d", __func__, fd);
        return BAD_VALUE;
    }
    return err;
}

/* Searches for the fd from the mPfd array and updates the poll events */
status_t EventThread::processListenToDequeueEvents(const int32_t& fd)
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s: E, fd: %d", __func__, fd);
    /* Input validation */
    if(fd < 0){
        return BAD_VALUE;
    }

    /* search for the fd in the mPfd list and add dequeue events */
    /* 0 idx is occupied by local command fd */
    for(int32_t i = 1; i < mNumActiveFds; i++){
        struct pollfd* pfd = &mPfd[i];
        if(pfd->fd == fd){
            pfd->events |= POLLOUT | POLLWRNORM; /* for dequeue event */
#if DBG_VPUOUT_HOST
            pfd->events |= POLLIN | POLLRDNORM;
#endif
            ALOGD_IF(isDebug(), "%s: dequeue event added fd: %d", __func__, fd);
            ALOGD_IF(isDebug(), "%s: fd: %d events: %d", __func__, pfd->fd,
                                    pfd->events);
        }
    }
    return err;
}

/* Searches for the fd from the mPfd array and updates the poll events */
status_t EventThread::processTurnOffDequeueEvents(const int32_t& fd)
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s: E, fd: %d", __func__, fd);
    /* Input validation */
    if(fd < 0){
        return BAD_VALUE;
    }

    /* search for the fd in the mPfd list and add dequeue events */
    /* 0 idx is occupied by local command fd */
    for(int32_t i = 1; i < mNumActiveFds; i++){
        struct pollfd* pfd = &mPfd[i];
        if(pfd->fd == fd){
            pfd->events &= ~(POLLOUT | POLLWRNORM); /* for dequeue event */
#if DBG_VPUOUT_HOST
            pfd->events &= ~(POLLIN | POLLRDNORM);
#endif
            ALOGD_IF(isDebug(), "%s: dequeue event disabled fd: %d",
                __func__, fd);
            ALOGD_IF(isDebug(), "%s: fd: %d events: %d", __func__, pfd->fd,
                                    pfd->events);
        }
    }
    return err;
}

};
