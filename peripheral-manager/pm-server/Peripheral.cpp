/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_TAG "PerMgrSrv"

#include <fcntl.h>

#include "subsystem_request_v01.h"
#include "Peripheral.h"

namespace android {

void notificationAckTimeout(sigval_t v) {
    Peripheral *peripheral = (Peripheral *) v.sival_ptr;

    // Go and change device state.
    peripheral->notifyClientAck(0, EVENT_INVALID);
}

Peripheral::Peripheral(char *name, char *deviceFile, int subSystemId,
                     int offTimeout, int ackTimeout, bool debugMode) {
    mName = name;
    mDeviceFile = deviceFile;
    mSubSytemId = subSystemId;
    mOffTimeout = offTimeout;
    mAckTimeout = ackTimeout;
    mSimulationMode = debugMode;
    mRestartRequestor = NULL;
    mNumVoters = 0;
    mLastClientCrashed = false;
    mShutdown = false;
    mFileDesc = -1;
    mAckTimer = 0;
    mState = EVENT_PERIPH_IS_OFFLINE;
    mStateCanaged = systemTime(CLOCK_REALTIME);
    mPowerOnCount = 0;
    mPowerOffCount = 0;
    mMaxListenersCount = 0;
    mMaxVotersCount = 0;
}

status_t Peripheral::init() {
    struct sigevent sigEvent;

    if (access(mDeviceFile, F_OK)) {
        ALOGE("%s can not access device file %s: %s", (const char *)mName,
              (const char *)mDeviceFile, strerror(errno));
        return PERMISSION_DENIED;
    }

    memset(&sigEvent, 0, sizeof(sigEvent));

    sigEvent.sigev_notify = SIGEV_THREAD;
    sigEvent.sigev_notify_function = notificationAckTimeout;
    sigEvent.sigev_value.sival_ptr = this;

    if (timer_create(CLOCK_REALTIME, &sigEvent, &mAckTimer) < 0) {
        ALOGE("%s notification timer creation failed %s", (const char * )mName,
              strerror(errno));
        return BAD_VALUE;
    }

    return OK;
}

void Peripheral::deInit() {

    timerStop();

    if (timer_delete(mAckTimer) < 0) {
        ALOGE("%s notification timer delete fail %s", (const char *)mName,
              strerror(errno));
    }
}

void Peripheral::addClient(Client *client) {

    Mutex::Autolock lock(mLock);

    mClients.push_back(client);

    ALOGD("%s state: %s, add client %s", (const char *)mName, getStateName(),
          client->getName());

    if (mClients.size() > (unsigned) mMaxListenersCount) {
        // For statistic only
        mMaxListenersCount = mClients.size();
    }
}

void Peripheral::removeClient(Client *client) {
    list<Client *>::iterator iter;

    Mutex::Autolock lock(mLock);

    for (iter = mClients.begin(); iter != mClients.end(); iter++) {
        Client *nextClient = *iter;
        if (nextClient == client) {
            mClients.erase(iter);
            ALOGD("%s state: %s, remove client %s", (const char *)mName,
                  getStateName(), client->getName());
            break;
        }
    }
}

bool Peripheral::findClient(Client *searched) {
    list<Client *>::iterator iter;
    Client *client;

    for (iter = mClients.begin(); iter != mClients.end(); iter++) {
        client = *iter;
        if (client == searched) {
            return true;
        }
    }

    return false;
}

void Peripheral::notifyClients(enum pm_event event) {
    list<Client *>::iterator iter;

    timerStop();

    // Change state
    mState = event;
    mStateCanaged = systemTime(CLOCK_REALTIME);

    ALOGD("%s new state: %s", (const char *)mName, getStateName());

    for (iter = mClients.begin(); iter != mClients.end(); iter++) {
        Client *client = *iter;
        client->eventSend(event);
    }

    timerStart(event == EVENT_PERIPH_GOING_OFFLINE ? mOffTimeout : mAckTimeout);
}

status_t Peripheral::notifyClientAck(Client *client, enum pm_event evAck) {
    list<Client *>::iterator iter;
    bool waitComplete = true;
    int ret, error;

    Mutex::Autolock lock(mLock);

    if (client) {
        // Check that client still exist
        if (findClient(client)) {
            client->eventReceived(evAck);
        }
    } else {
        // Timeout
        if (mLastClientCrashed) {
            mLastClientCrashed = false;
            // The last voter that crashed did not re-register
            // So we start the power down sequence for the peripheral.
            ALOGI("%s voter did not reregister after crash. Powering down",
                  (const char *)mName);
            notifyClients(EVENT_PERIPH_GOING_OFFLINE);
            return OK;
        }

        // Log clients that didn't send an ack yet and cancel
        // any delayed events from them
        for (iter = mClients.begin(); iter != mClients.end(); iter++) {
            client = *iter;
            client->eventReceived(EVENT_INVALID);
        }
    }

    // Still waiting state change ACK's?
    for (iter = mClients.begin(); iter != mClients.end(); iter++) {
        client = *iter;
        if (client->isAckExpected()) {
            waitComplete = false;
        }
    }

    if (mShutdown) {
        // Nothing can stop this
        waitComplete = true;
    }

    if (!waitComplete) {
        return OK;
    }

    // We are here because of timeout or all clients have acknowledged
    // state transition. In any case ensure that timer is stopped.
    timerStop();

    ret = error = 0;
    switch (mState) {
    case EVENT_PERIPH_GOING_ONLINE:
        if (mFileDesc < 0) {
            if (!mSimulationMode) {
                mFileDesc = open(mDeviceFile, O_RDONLY);
                error = errno;
            } else {
                mFileDesc = 1;
            }

            if (mFileDesc > 0) {
                // Just for the statistics
                mPowerOnCount++;
            }
        }

        if (mFileDesc > 0) {
            notifyClients(EVENT_PERIPH_IS_ONLINE);
        } else {
            ALOGE("power on %s failed %s: %s", (const char *)mName,
                  (const char *)mDeviceFile, strerror(error));
            notifyClients(EVENT_PERIPH_IS_OFFLINE);
        }
        break;

    case EVENT_PERIPH_IS_ONLINE:
        break;

    case EVENT_PERIPH_GOING_OFFLINE:
        if (isRestartRequested()) {
            ALOGI("%s going off-line because restart request", (const char *)mName);
            sendRestartIndication();
        }

        if (mFileDesc > 0) {
            if (!mSimulationMode) {
                ret = close(mFileDesc);
                error = errno;
            }
        }

        if (!ret) {
            mFileDesc = -1;
            mPowerOffCount++;
            notifyClients(EVENT_PERIPH_IS_OFFLINE);
        } else {
            ALOGE("%s power off failed %s: %s", (const char *)mName,
                  (const char *)mDeviceFile, strerror(error));
            notifyClients(EVENT_PERIPH_IS_ONLINE);
        }
        break;

    case EVENT_PERIPH_IS_OFFLINE:
        if (mShutdown) {
            // Done
            mShutdownComplete.unlock();
        }

        if (isRestartRequested()) {
            setRestartRequested(NULL);
            ALOGI("%s going on-line because restart request", (const char *)mName);
            notifyClients(EVENT_PERIPH_GOING_ONLINE);
        }
        break;

    default:
        break;
    }

    return OK;
}

status_t Peripheral::increaseVoters(Client *client) {

    Mutex::Autolock lock(mLock);

    // Check that client still exist
    // Be paranoid and validate user input
    if (!findClient(client)) {
        ALOGE("%s no such client %s", (const char *)mName, client->getName());
        return NAME_NOT_FOUND;
    }

    if (client->getIsVoter()) {
        // Already Voter
        ALOGD("%s client %s is voter already", (const char *)mName, client->getName());
        return BAD_VALUE;
    }

    if (mShutdown) {
        // Refuse any APP client requests
        ALOGI("%s client %s vote ignored, shutdown in progress",
              (const char *)mName, client->getName());
        return BAD_VALUE;
    }

    client->setIsVoter(true);

    if (mNumVoters++ == 0) {
        // If the last client had crashed then we do not need to
        // send out any notifications from here. Let the expiry of the
        // time started when the crash was detected figure out what to do
        if (!mLastClientCrashed) {
            notifyClients(EVENT_PERIPH_GOING_ONLINE);
        } else {
            mLastClientCrashed = false;
            timerStop();
            ALOGD("%s client %s reconnected within timeout",
                  (const char *)mName, client->getName());
        }
    }

    ALOGD("%s num voters is %d", (const char*)mName, mNumVoters);

    if (mNumVoters > mMaxVotersCount) {
        mMaxVotersCount = mNumVoters;
    }

    return OK;
}

status_t Peripheral::decreaseVoters(Client *client, bool crash) {

    Mutex::Autolock lock(mLock);

    // Check that client still exist
    // Be paranoid and validate user input
    if (!findClient(client)) {
        ALOGE("%s no such client %p", (const char *)mName, client);
        return NAME_NOT_FOUND;
    }

    if (!client->getIsVoter()) {
        ALOGD("%s client %s is not a voter", (const char *)mName, client->getName());
        return BAD_VALUE;
    }

    if (mShutdown) {
        // Refuse any APP client requests
        ALOGI("%s client %s unvote ignored, shutdown in progress",
              (const char *)mName, client->getName());
        return BAD_VALUE;
    }

    if (!mNumVoters) {
        ALOGE("%s unbalanced voters decrease %s", (const char *)mName,
              client->getName());
        return BAD_VALUE;
    }

    ALOGD("%s num voters is %d", (const char*)mName, mNumVoters);
    client->setIsVoter(false);

    // Shutdown peripheral only if last voter has disconnected
    if (--mNumVoters == 0) {
        if (crash) {
            ALOGE("%s last client %s crashed", (const char *)mName,
                  client->getName());
            mLastClientCrashed = true;
            // Just start waiting for reconnect. This avoid needless
            // peripheral power cycle.
            timerStart(mOffTimeout);
        } else {
            notifyClients(EVENT_PERIPH_GOING_OFFLINE);
        }
    }

    return OK;
}

void Peripheral::shutdownStart() {

    Mutex::Autolock lock(mLock);

    ALOGD("%s shut down", (const char *)mName);

    mShutdown = true;
    mShutdownComplete.lock();

    // Forget about the crash
    mLastClientCrashed = false;
    mNumVoters = 0;
    mRestartRequestor = NULL;

    if (mState != EVENT_PERIPH_IS_OFFLINE) {
        notifyClients(EVENT_PERIPH_GOING_OFFLINE);
    } else {
        // Done
        mShutdownComplete.unlock();
    }
}

void Peripheral::shutdownWait() {
    mShutdownComplete.lock();
    ALOGD("%s turned off", (const char *)mName);
}

const String8 &Peripheral::getName() {

    Mutex::Autolock lock(mLock);
    return mName;
}

int Peripheral::getSubSytemId() {

    Mutex::Autolock lock(mLock);
    return mSubSytemId;
}

int Peripheral::getState() {

    Mutex::Autolock lock(mLock);
    return mState;
}

void Peripheral::dumpStatistics() {
    list<Client *>::iterator iter;
    int voters = 0;
    int listeners = 0;

    for (iter = mClients.begin(); iter != mClients.end(); iter++) {
        Client *client = *iter;
        if (client->getIsVoter()) {
            voters++;
        } else
            listeners++;
    }
    ALOGI("Statistics for : %s", (const char *)mName);
    ALOGI("%s: Current State : %s", (const char *)mName, getStateName());
    ALOGI("%s: Voter/Listener count : %d/%d", (const char *)mName,
                    voters,
                    listeners);
    ALOGI("%s: Power on count : %d", (const char *)mName, mPowerOnCount);
    ALOGI("%s: Power off count: %d", (const char *)mName, mPowerOffCount);
    ALOGI("%s: Client list:", (const char *)mName);
    for (iter = mClients.begin(); iter != mClients.end(); iter++) {
        Client *client = *iter;
        if (client->getIsVoter()) {
            ALOGI("\t%s: %s(voter)", (const char *)mName, client->getName());
        } else {
            ALOGI("\t%s: %s(listener)", (const char *)mName, client->getName());
        }
    }
}

bool Peripheral::setRestartRequested(void *handle) {
    bool set = false;

    if (handle) {
        // called from Peripheral Manager thread
        mLock.lock();
    }

    if (mShutdown) {
        if (handle) {
            mLock.unlock();
        }
        // Refuse any QMI client requests
        return set;
    }

    if (handle) {
        if (mRestartRequestor == NULL) {
            mRestartRequestor = handle;
            set = true;
        } else {
            // Sorry other request is ongoing
        }
    } else {
        mRestartRequestor = handle;
        set = true;
    }

    if (set && handle) {
        notifyClients(EVENT_PERIPH_GOING_OFFLINE);
    }

    if (handle) {
        mLock.unlock();
    }

    return set;
}

bool Peripheral::isRestartRequested() {
    return mRestartRequestor != NULL;
}

void Peripheral::sendRestartIndication() {
    qmi_ssreq_peripheral_restart_ind_msg_v01 ind;
    qmi_csi_error ret = QMI_CSI_NO_ERR;
    qmi_client_handle client;
    unsigned int msgId;

    client = (qmi_client_handle)mRestartRequestor;
    msgId = QMI_SSREQ_PERIPHERAL_RESTART_IND_V01;
    ind.status = SSREQ_QMI_REQUEST_SERVICED_V01;

    ret = qmi_csi_send_ind(client, msgId, &ind, sizeof(ind));
    if (ret) {
        ALOGE("%s failed to send a restart indication to QMI client %p",
              (const char *)mName, client);
    }
}

void Peripheral::addPermissions(int cnt, int perm[]) {

    Mutex::Autolock lock(mLock);

    for (int idx = 0; idx < cnt; idx++) {
        mPermissions.push_back(perm[idx]);
    }
}

bool Peripheral::checkPermission(int perm) {
    list<int> ::iterator iter;

    Mutex::Autolock lock(mLock);

    for (iter = mPermissions.begin(); iter != mPermissions.end(); iter++) {
        int neededPerm = *iter;

        if (perm == neededPerm) {
            return true;
        }
    }

    return false;
}

const char *Peripheral::getStateName() {

    const char *state;

    switch (mState) {
    case EVENT_PERIPH_GOING_OFFLINE:
        state = "going off-line";
        break;
    case EVENT_PERIPH_IS_OFFLINE:
        state = "is off-line";
        break;
    case EVENT_PERIPH_GOING_ONLINE:
        state = "going on-line";
        break;
    case EVENT_PERIPH_IS_ONLINE:
        state = "is on-line";
        break;
    default:
        state = "state unknown";
        break;
    }

    return state;
}

int Peripheral::timerStart(int ms) {
    struct itimerspec oldTime;
    struct itimerspec newTime;
    time_t sec;
    int ret;

    sec = 0;
    if (ms >= 1000) {
        sec = ms / 1000;
        ms -= sec * 1000;
    }

    newTime.it_value.tv_sec = sec;
    newTime.it_value.tv_nsec = ms * 1000000;
    newTime.it_interval.tv_sec = 0;
    newTime.it_interval.tv_nsec = 0;

    ret = timer_settime(mAckTimer, CLOCK_REALTIME, &newTime, &oldTime);
    if (ret < 0) {
        ALOGE("%s timer start failed: %s", (const char *)mName,
              strerror(errno));
        return BAD_VALUE;
    }

    return OK;
}

void Peripheral::timerStop() {
    struct itimerspec oldTime;
    struct itimerspec newTime;
    int ret;

    newTime.it_value.tv_sec = 0;
    newTime.it_value.tv_nsec = 0;
    newTime.it_interval.tv_sec = 0;
    newTime.it_interval.tv_nsec = 0;

    ret = timer_settime(mAckTimer, CLOCK_REALTIME, &newTime, &oldTime);
    if (ret < 0) {
        ALOGE("%s timer stop failed: %s", (const char *)mName,
              strerror(errno));
    }
}

}; // namespace android
