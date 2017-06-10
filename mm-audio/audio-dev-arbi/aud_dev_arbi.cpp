/*===========================================================================
                           aud_dev_arbi.cpp

DESCRIPTION: This file implements audio device arbitration.

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "aud_dev_arbi"

/*-----------------------------------------------------------------------------
    Includes
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <pthread.h>
#include "aud_dev_arbi.h"
#include <utils/Log.h>
#include "common_log.h"

/*-----------------------------------------------------------------------------
    Consts and macros
-----------------------------------------------------------------------------*/
#define CMD_TIMEOUT_SEC (2)
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

const char *AUD_DEV_ARBI_SOCKET_FULL_PATH = "/data/misc/audio/aud_devi_arbi";

/*-----------------------------------------------------------------------------
    Functions
-----------------------------------------------------------------------------*/
static bool isAudioDeviceSupported(audio_devices_t audioDevice)
{
    static const audio_devices_t supportedAudioDevices[] = {
        AUDIO_DEVICE_OUT_EARPIECE
    };

    for (unsigned int ind = 0; ind < ARRAY_SIZE(supportedAudioDevices); ++ind) {
        if (audioDevice == supportedAudioDevices[ind]) {
            return true;
        }
    }

    return false;
}

/*============================================================================
    FUNCTION:  Socket::init
============================================================================*/
int Socket::init(const char *sockPath, struct sockaddr_un* sockAddr)
{
    int socketFd = socket(AF_UNIX,
                        SOCK_STREAM,
                        0);
    if (socketFd < 0) {
        LOGE("%s:socket() failed, %d, %s",
              __FUNCTION__,
              errno,
              strerror(errno));
        return -1;
    }

    sockAddr->sun_family = AF_UNIX;
    strlcpy(sockAddr->sun_path,
            sockPath,
            sizeof(sockAddr->sun_path));

    return socketFd;
}

/*============================================================================
    FUNCTION:  Socket::sendBuf
============================================================================*/
int Socket::sendBuf(int fd, void* buf, int len)
{
    ssize_t bytesWritten = 0;

    while ((bytesWritten >= 0) && (len > 0)) {
        bytesWritten = write(fd,
                              buf,
                              len);

        if (bytesWritten > 0) {
            buf = (char*)buf + bytesWritten;
            len -= bytesWritten;
        }
    }

    if (bytesWritten < 0) {
        LOGE("%s: write() failed, %d, %s",
             __FUNCTION__,
             errno,
             strerror(errno));

        return -1;
    }

    return 0;
}

/*============================================================================
    FUNCTION:  Socket::receiveBuf
============================================================================*/
int Socket::receiveBuf(int fd, int abortFd, void* buf, int len, int timeout)
{
    ssize_t bytesRcvd = 0;

    while ((bytesRcvd >= 0) && (len > 0)) {
        fd_set rfds;
        struct timeval tv;
        struct timeval *timeoutPtr = NULL;

        // TODO update to remaining time
        if (timeout) {
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout % 1000) * 1000;
            timeoutPtr = &tv;
        }

        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        if (abortFd) {
            FD_SET(abortFd, &rfds);
        }

        int nfds = (fd > abortFd ? fd : abortFd) + 1;

        int rc = select(nfds, &rfds, NULL, NULL, timeoutPtr);

        if (rc < 0) {
            LOGE("%s: select() failed, %d",
                 __FUNCTION__,
                 rc);

            return -1;
        } else if(0 == rc) {
            LOGW("%s: timeout waiting for buffer",
                 __FUNCTION__);

            return -1;
        }

        if (abortFd && FD_ISSET(abortFd, &rfds)) {
            LOGD("%s: abort fd signaled ",
                 __FUNCTION__);

            return -EINTR;
        }

        bytesRcvd = read(fd,
                          buf,
                          len);

        if (0 == bytesRcvd) {
            LOGD("%s: read() returned EOF",
                 __FUNCTION__);

            return -1;
        }

        if (bytesRcvd < 0) {
            LOGE("%s: read() failed, %d, %s",
                 __FUNCTION__,
                 errno,
                 strerror(errno));

            return -1;
        } else {
            buf = (char*)buf + bytesRcvd;
            len -= bytesRcvd;
        }
    }

    return 0;
}

/*============================================================================
    FUNCTION:  IIpcEndpointHandler::~IIpcEndpointHandler
============================================================================*/
IIpcEndpointHandler::~IIpcEndpointHandler()
{
}

/*============================================================================
    FUNCTION:  IpcEndpoint::start
============================================================================*/
int IpcEndpoint::start(IIpcEndpointHandler* ipcHandler)
{
    mIpcEpHandler = NULL;
    mThreadId = -1;
    mAbortReadFd = -1;
    mOutgoingCmdPending = false;
    mStopEp = false;

    mIpcEpHandler = ipcHandler;

    if (sem_init(&mCmdDone, 0, 0)) {
        LOGE("%s: could not initialize sempahore, %d, %s",
             __FUNCTION__,
             errno,
             strerror(errno));

        return -1;
    }

    mAbortReadFd = eventfd(0, 0);

    // Run the server thread
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    return pthread_create((pthread_t*)&mThreadId,
                        &attr,
                        threadFuncCb,
                        this);
}

/*============================================================================
    FUNCTION:  IpcEndpoint::stop
============================================================================*/
void IpcEndpoint::stop()
{
    if (0 == mThreadId) {
      return;
    }
    // request thread to stop
    mStopEp = true;

    uint64_t counterInc = 1;
    if (write(mAbortReadFd, &counterInc, sizeof(counterInc)) <= 0) {
    LOGE("%s: error writing to command pending fd, %d, %s",
         __FUNCTION__,
         errno,
         strerror(errno));

        return;
    }

    if(pthread_join(mThreadId, NULL) != 0) {
        LOGE("%s: error terminating thread",
             __FUNCTION__);
    }

    mStopEp = false;
    mThreadId = 0;
    close(mAbortReadFd);
}

/*============================================================================
    FUNCTION:  IpcEndpoint::threadFuncCb
============================================================================*/
void *IpcEndpoint::threadFuncCb(void* param)
{
    IpcEndpoint *ep = (IpcEndpoint *) param;
    ep->mIpcEpHandler->runSocketThread();
    return NULL;
}

/*============================================================================
    FUNCTION:  IpcEndpoint::handleIncomingMessage
============================================================================*/
int IpcEndpoint::handleIncomingMessage(int sockFd)
{
    aud_dev_arbi_msg msg;

    LOGD("%s: waiting for msg", __FUNCTION__);

    int rc = Socket::receiveBuf(sockFd, mAbortReadFd,
                               &msg, sizeof(msg), 0);

    if (rc < 0) {
        if(-EINTR == rc) {
          LOGD("%s: wait aborted by abort fd", __FUNCTION__);

          return rc;
        } else {
          LOGE("%s: error receiving data from client",
               __FUNCTION__);

          return -1;
        }
    }

    LOGD("%s: received msgId=%d, clientId=%d, deviceId=%d",
       __FUNCTION__,
       msg.msgId,
       msg.clientId,
       msg.deviceId);

    if (msg.msgId >= AUD_DEV_ARBI_NUM_IPC_MSG) {
        LOGW("%s: received msgId out of range (%d)",
             __FUNCTION__,
             msg.msgId);

        return -1;
    }

    if (msg.clientId >= AUD_DEV_ARBI_NUM_CLIENTS) {
        LOGW("%s: received clientId out of range (%d)",
             __FUNCTION__,
             msg.clientId);

        return -1;
    }

    if ((AUD_DEV_ARBI_IPC_MSG_ACK == msg.msgId) ||
          (AUD_DEV_ARBI_IPC_MSG_NACK == msg.msgId)) {
        // got ACK or NACK
        if(mOutgoingCmdPending) {
            mOutgoingCmdPending = false;

            mCmdResult = msg.msgId;
            if (sem_post(&mCmdDone) < 0) {
                LOGE("%s: error signaling command done",
                     __FUNCTION__);

                return -1;
            }
        } else {
            LOGE("%s: error, unexpected event %d (outgoing command not pending)",
                 __FUNCTION__,
                 msg.msgId);

            return -1;
        }
    } else {
        // message which is not ACK or NACK (command)
        aud_dev_arbi_ipc_msg_t response =
        mIpcEpHandler->handleIncomingCommand(msg);

        aud_dev_arbi_msg responseMsg;
        responseMsg.msgId = response;
        responseMsg.clientId = msg.clientId;
        responseMsg.deviceId = msg.deviceId;

        rc = Socket::sendBuf(sockFd, &responseMsg, sizeof(responseMsg));
        if (rc < 0) {
            LOGE("%s: error sending response", __FUNCTION__);

            return -1;
        }
    }

    return 0;
}


/*============================================================================
    FUNCTION:  IpcEndpoint::handleSession
============================================================================*/
bool IpcEndpoint::handleSession(int sockFd)
{
    LOGD("%s: entered, sockFd=%d", __FUNCTION__,
       sockFd);

    do {
        int rc = handleIncomingMessage(sockFd);
        if (rc < 0) {
          if (-EINTR == rc) {
            if (mStopEp) {
              // stop command was issued
              break;
            }
          } else {
              LOGE("%s: error receiving command",
                   __FUNCTION__);
              break;
          }
        }
    } while (true);

    return mStopEp;
}

/*============================================================================
    FUNCTION:  IpcEndpoint::notify
============================================================================*/
int IpcEndpoint::notify(int sockFd,
                        aud_dev_arbi_msg msg,
                        aud_dev_arbi_ipc_msg_t* response)
{
    if (NULL == response) {
        LOGE("%s: error, response input parameter is null",
             __FUNCTION__);

        return -1;
    }

    mOutgoingCmdPending = true;

    LOGD("%s: sending msg, msgId=%d, clientId=%d, deviceId=%d", __FUNCTION__,
         msg.msgId,
         msg.clientId,
         msg.deviceId);

    if (Socket::sendBuf(sockFd, &msg, sizeof(msg))) {
        LOGE("%s: error sending message",
                 __FUNCTION__);

        return -1;
    }

    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
        LOGE("%s: error getting time, %d, %s",
             __FUNCTION__,
             errno,
             strerror(errno));

        return -1;
    }

    ts.tv_sec += CMD_TIMEOUT_SEC;

    // wait for command done notification
    int rc = sem_timedwait(&mCmdDone, &ts);
    if (rc < 0) {
        if(-ETIMEDOUT == rc) {
          LOGE("%s: timeout waiting for cmd done",
                 __FUNCTION__);
        } else {
          LOGE("%s: error waiting for cmd done, %d, %s",
               __FUNCTION__,
               errno,
               strerror(errno));
        }

        return -1;
    }

    *response = mCmdResult;
    return 0;
}

/*============================================================================
    FUNCTION:  ServerIpcEndpoint::acquireDevice
============================================================================*/
bool ServerIpcEndpoint::acquireDevice(audio_devices_t device)
{
    bool result = true;
    int rc = pthread_mutex_lock(&mMutex);

    if (rc != 0) {
        LOGE("%s: error locking mutex, %d",
             __FUNCTION__, rc);

        return false;
    }

    if (AUD_DEV_ARBI_DEVICE_OWNER_CLIENT != mDeviceOwner) {
        goto end;
    }

    if (device != mDevice) {
        LOGD("%s: not sending command, device argument (%d) does not match registered device (%d)",
              __FUNCTION__,
              device,
              mDevice);

        goto end;
    }

    aud_dev_arbi_ipc_msg_t response;
    aud_dev_arbi_msg msg;
    msg.msgId = AUD_DEV_ARBI_IPC_MSG_DEVICE_REQUESTED;
    msg.clientId = mClient;
    msg.deviceId = device;

    rc = mIpcEndpoint.notify(mClientFd,
                             msg,
                             &response);

    if ((rc < 0) || (AUD_DEV_ARBI_IPC_MSG_ACK != response)) {
        result = false;
    }

end:
    mDeviceOwner = AUD_DEV_ARBI_DEVICE_OWNER_SERVER;

    LOGD("%s: device owner is %d",
       __FUNCTION__, mDeviceOwner);

    pthread_mutex_unlock(&mMutex);
    return result;
}

/*============================================================================
    FUNCTION:  ServerIpcEndpoint::releaseDevice
============================================================================*/
bool ServerIpcEndpoint::releaseDevice(audio_devices_t device)
{
    bool result = true;

    int rc = pthread_mutex_lock(&mMutex);
    if (rc != 0) {
        LOGE("%s: error locking mutex, %d",
                   __FUNCTION__, rc);

        return false;
    }

    if (AUD_DEV_ARBI_DEVICE_OWNER_SERVER != mDeviceOwner) {
        LOGW("%s: server is not the owner",
                 __FUNCTION__);

        goto end;
    }

    if (device != mDevice) {
        LOGD("%s: not sending command, device argument (%d) does not match registered device (%d)",
             __FUNCTION__,
             device,
             mDevice);

        goto end;
    }

    aud_dev_arbi_ipc_msg_t response;
    aud_dev_arbi_msg msg;
    msg.msgId = AUD_DEV_ARBI_IPC_MSG_DEVICE_RELEASED;
    msg.clientId = mClient;
    msg.deviceId = device;

    rc = mIpcEndpoint.notify(mClientFd,
                             msg,
                             &response);

    if ((rc < 0) || (AUD_DEV_ARBI_IPC_MSG_ACK != response)) {
        result = false;
    }

end:
    mDeviceOwner = AUD_DEV_ARBI_DEVICE_OWNER_NONE;

    LOGD("%s: device owner is %d",
       __FUNCTION__, mDeviceOwner);

    pthread_mutex_unlock(&mMutex);
    return result;
}

/*============================================================================
    FUNCTION:  IpcEndpoint::~IpcEndpoint
============================================================================*/
IpcEndpoint::~IpcEndpoint()
{
}

/*============================================================================
    FUNCTION:  ServerIpcEndpoint::runSocketThread
============================================================================*/
int ServerIpcEndpoint::runSocketThread()
{
    LOGD("%s: entered", __FUNCTION__);

    int len = sizeof(mAddress.sun_family) + strlen(mAddress.sun_path);

    if (bind(mSocketFd,
           (struct sockaddr *)&mAddress,
           len) < 0) {
        LOGE("%s: bind() failed (fd=%d), %s",
             __FUNCTION__,
             mSocketFd,
             strerror(errno));
        return -1;
    }

    // allows group access to socket
    chmod(mAddress.sun_path, 0660);

    if (listen(mSocketFd, 5) < 0) {
        LOGE("%s: listen() failed (fd=%d), %s",
             __FUNCTION__,
             mSocketFd,
             strerror(errno));
        return -1;
    }

    while(true) {
        struct sockaddr_un faddress;
        int fromlen;

        fromlen = sizeof(faddress);

        if ((mClientFd = accept(mSocketFd,
                                  (struct sockaddr*)&faddress,
                                  &fromlen)) < 0) {
            LOGE("%s: accept() failed, %s",
                 __FUNCTION__,
                 strerror(errno));

            continue;
        }

        bool shouldStop = mIpcEndpoint.handleSession(mClientFd);
        if (shouldStop) {
            LOGD("%s: stopping thread",
                 __FUNCTION__);

            break;
        }

        // reset the device
        mDevice = AUDIO_DEVICE_NONE;
    }

    LOGD("%s: exiting thread", __FUNCTION__);

    return 0;
}

/*============================================================================
    FUNCTION:  ServerIpcEndpoint::handleIncomingCommand
============================================================================*/
aud_dev_arbi_ipc_msg_t ServerIpcEndpoint::handleIncomingCommand(aud_dev_arbi_msg msg)
{
    aud_dev_arbi_ipc_msg_t response = AUD_DEV_ARBI_IPC_MSG_NACK;
    int rc = pthread_mutex_lock(&mMutex);

    if (rc != 0) {
        LOGE("%s: error locking mutex, %d",
              __FUNCTION__, rc);

        return response;
    }

    aud_dev_arbi_device_owner_t nextDeviceOwner = mDeviceOwner;

    if ((AUD_DEV_ARBI_IPC_MSG_DEVICE_REQUESTED == msg.msgId)
      || (AUD_DEV_ARBI_IPC_MSG_DEVICE_RELEASED == msg.msgId)) {
      if (msg.deviceId != mDevice) {
          LOGW("%s: deviceId in msg (%d) does not match registered device (%d), msg ignored",
          __FUNCTION__,
          msg.deviceId,
          mDevice);

          goto end;
      }
    }

    switch(msg.msgId) {
    case AUD_DEV_ARBI_IPC_MSG_DEVICE_REQUESTED:
      if (AUD_DEV_ARBI_DEVICE_OWNER_SERVER != mDeviceOwner) {
          response = AUD_DEV_ARBI_IPC_MSG_ACK;
          nextDeviceOwner = AUD_DEV_ARBI_DEVICE_OWNER_CLIENT;
      }
      break;

    case AUD_DEV_ARBI_IPC_MSG_DEVICE_RELEASED:
      if (AUD_DEV_ARBI_DEVICE_OWNER_CLIENT == mDeviceOwner) {
          response = AUD_DEV_ARBI_IPC_MSG_ACK;
          nextDeviceOwner = AUD_DEV_ARBI_DEVICE_OWNER_NONE;
      } else {
          LOGW("%s: unexpected RELEASE event, owner is %d",
               __FUNCTION__,
               mDeviceOwner);
      }
      break;

    case AUD_DEV_ARBI_IPC_MSG_REGISTER_FOR_DEVICE:
        if (isAudioDeviceSupported(msg.deviceId)) {
            mDevice = (audio_devices_t)msg.deviceId;
            mClient = (aud_dev_arbi_client_t)msg.clientId;
            response = AUD_DEV_ARBI_IPC_MSG_ACK;
        } else {
            LOGW("%s: audio device %d is not supported for arbitration",
                 __FUNCTION__,
                 msg.deviceId);
        }
      break;

    default:
      break;
    }

end:

    mDeviceOwner = nextDeviceOwner;

    LOGD("%s: returning, response %d, next owner %d",
         __FUNCTION__,
         response,
         mDeviceOwner);

    pthread_mutex_unlock(&mMutex);

    return response;
}

/*============================================================================
    FUNCTION:  ServerIpcEndpoint::start
============================================================================*/
int ServerIpcEndpoint::start(const char *sockPath)
{
    if (mIsActive) {
    return 0;
    }

    mDevice = AUDIO_DEVICE_NONE;
    mDeviceOwner = AUD_DEV_ARBI_DEVICE_OWNER_NONE;
    mSocketFd = -1;
    mClientFd = -1;
    mIsActive = false;

    int rc = pthread_mutex_init(&mMutex, NULL);
    if (rc != 0) {
        LOGE("%s: error initializing mutex, %d",
             __FUNCTION__,
             rc);
    }

    mSocketFd = Socket::init(sockPath, &mAddress);
    if (mSocketFd < 0) {
        LOGE("%s: error initializing socket",
             __FUNCTION__);
        return -1;
    }

    unlink(mAddress.sun_path);

    if (mIpcEndpoint.start(this) < 0) {
        LOGE("%s: error starting endpoint",
             __FUNCTION__);
        return -1;
    }

    mIsActive = true;

    return 0;
}

/*============================================================================
    FUNCTION:  ServerIpcEndpoint::stop
============================================================================*/
void ServerIpcEndpoint::stop()
{
    if (!mIsActive) {
        LOGE("%s: endpoint inactive, returning",
                   __FUNCTION__);
        return;
    }

    mIpcEndpoint.stop();

    if (0 <= mSocketFd) {
        close(mSocketFd);
    }

    if (0 <= mClientFd) {
        close(mClientFd);
    }

    pthread_mutex_destroy(&mMutex);

    mIsActive = false;
}

/*============================================================================
    FUNCTION:  ServerIpcEndpoint::~ServerIpcEndpoint
============================================================================*/
ServerIpcEndpoint::~ServerIpcEndpoint()
{
}


/*============================================================================
    FUNCTION:  ClientIpcEndpoint::runSocketThread
============================================================================*/
int ClientIpcEndpoint::runSocketThread()
{
    LOGD("%s: entered",
       __FUNCTION__);

    while(true) {
        bool shouldStop = mIpcEndpoint.handleSession(mSocketFd);
        if (shouldStop) {
            LOGD("%s: stopping thread",
                 __FUNCTION__);
            break;
        }
    }

    LOGD("%s: exiting thread", __FUNCTION__);

    return 0;
}

/*============================================================================
    FUNCTION:  ClientIpcEndpoint::handleIncomingCommand
============================================================================*/
aud_dev_arbi_ipc_msg_t ClientIpcEndpoint::handleIncomingCommand(aud_dev_arbi_msg msg)
{
    aud_dev_arbi_ipc_msg_t response = AUD_DEV_ARBI_IPC_MSG_ACK;
    int rc = pthread_mutex_lock(&mMutex);
    if (rc != 0) {
        LOGE("%s: error locking mutex, %d",
                   __FUNCTION__, rc);

        return response;
    }

    if (mEvtCb) {
        aud_dev_arbi_event_t evt;
        switch (msg.msgId) {
            case AUD_DEV_ARBI_IPC_MSG_DEVICE_REQUESTED:
                evt = AUD_DEV_ARBI_EVENT_DEVICE_REQUESTED;
                break;
            case AUD_DEV_ARBI_IPC_MSG_DEVICE_RELEASED:
                evt = AUD_DEV_ARBI_EVENT_DEVICE_RELEASED;
                break;
            default:
                LOGE("%s: unexpected msg (%d)", __FUNCTION__,
                     msg.msgId);
                evt = AUD_DEV_ARBI_NUM_EVENTS;
                break;
        }

        if (AUD_DEV_ARBI_NUM_EVENTS != evt) {
              mEvtCb(msg.deviceId, evt);
        }
    }

    LOGD("%s: returning, response %d",
         __FUNCTION__,
         response);

    pthread_mutex_unlock(&mMutex);

    return response;
}

/*============================================================================
    FUNCTION:  ClientIpcEndpoint::start
============================================================================*/
int ClientIpcEndpoint::start(const char *sockPath, aud_dev_arbi_event_cb_t evtCb)
{
    LOGD("%s: Enter", __FUNCTION__);

    if (mIsActive) {
        return 0;
    }

    mSocketFd = -1;
    mClient = AUD_DEV_ARBI_NUM_CLIENTS;
    mIsActive = false;
    mEvtCb = NULL;

    int rc = pthread_mutex_init(&mMutex, NULL);
    if (rc != 0) {
        LOGE("%s: error initializing mutex, %d",
             __FUNCTION__,
             rc);
    }

    if (NULL == evtCb) {
        LOGE("%s: event callback argument is null",
             __FUNCTION__);

        return -1;
    }

    mEvtCb = evtCb;

    mSocketFd = Socket::init(sockPath, &mAddress);
    if (mSocketFd < 0) {
        LOGE("%s: error initializing socket",
             __FUNCTION__);
        return -1;
    }

    rc = connect(mSocketFd, (struct sockaddr*)&mAddress, sizeof(mAddress));
    if (rc != 0) {
        LOGE("%s: error connecting to socket, %d, %s",
             __FUNCTION__,
             rc,
             strerror(errno));

        return rc;
    }

    LOGD("%s: connected to socket %d",
        __FUNCTION__,
        mSocketFd);

    if (mIpcEndpoint.start(this) < 0) {
        LOGE("%s: error starting endpoint",
             __FUNCTION__);

        return -1;
    }

    mIsActive = true;
    return 0;
}

/*============================================================================
    FUNCTION:  ClientIpcEndpoint::stop
============================================================================*/
void ClientIpcEndpoint::stop()
{
    if (!mIsActive) {
        LOGE("%s: endpoint inactive, returning",
             __FUNCTION__);
        return;
    }

    mIpcEndpoint.stop();

    if (0 <= mSocketFd) {
        close(mSocketFd);
    }

    pthread_mutex_destroy(&mMutex);

    mIsActive = false;
}

/*============================================================================
    FUNCTION:  ClientIpcEndpoint::registerForDevice
============================================================================*/
bool ClientIpcEndpoint::registerForDevice(aud_dev_arbi_client_t client,
                                            audio_devices_t device)
{
    bool result = true;

    if (client >= AUD_DEV_ARBI_NUM_CLIENTS) {
      LOGE("%s: invalid client id (%d)",
           __FUNCTION__, (uint32_t)client);

      return false;
    }

    int rc = pthread_mutex_lock(&mMutex);

    if (rc != 0) {
        LOGE("%s: error locking mutex, %d",
             __FUNCTION__, rc);

        return false;
    }

    mClient = client;

    aud_dev_arbi_ipc_msg_t response;
    aud_dev_arbi_msg msg;
    msg.msgId = AUD_DEV_ARBI_IPC_MSG_REGISTER_FOR_DEVICE;
    msg.clientId = mClient;
    msg.deviceId = device;

    rc = mIpcEndpoint.notify(mSocketFd,
                             msg,
                             &response);

    if ((rc < 0) || (AUD_DEV_ARBI_IPC_MSG_ACK != response)) {
        result = false;
    }

    pthread_mutex_unlock(&mMutex);

    return result;
}

/*============================================================================
    FUNCTION:  ClientIpcEndpoint::acquireDevice
============================================================================*/
bool ClientIpcEndpoint::acquireDevice(audio_devices_t device)
{
    bool result = true;

    int rc = pthread_mutex_lock(&mMutex);

    if (rc != 0) {
        LOGE("%s: error locking mutex, %d",
             __FUNCTION__, rc);

        return false;
    }

    aud_dev_arbi_ipc_msg_t response;
    aud_dev_arbi_msg msg;
    msg.msgId = AUD_DEV_ARBI_IPC_MSG_DEVICE_REQUESTED;
    msg.clientId = mClient;
    msg.deviceId = device;

    rc = mIpcEndpoint.notify(mSocketFd,
                               msg,
                               &response);

    if ((rc < 0) || (AUD_DEV_ARBI_IPC_MSG_ACK != response)) {
        result = false;
    }

    pthread_mutex_unlock(&mMutex);

    return result;
}

/*============================================================================
    FUNCTION:  ClientIpcEndpoint::releaseDevice
============================================================================*/
bool ClientIpcEndpoint::releaseDevice(audio_devices_t device)
{
    bool result = true;

    int rc = pthread_mutex_lock(&mMutex);

    if (rc != 0) {
        LOGE("%s: error locking mutex, %d",
             __FUNCTION__, rc);

        return false;
    }

    aud_dev_arbi_ipc_msg_t response;
    aud_dev_arbi_msg msg;
    msg.msgId = AUD_DEV_ARBI_IPC_MSG_DEVICE_RELEASED;
    msg.clientId = mClient;
    msg.deviceId = device;

    rc = mIpcEndpoint.notify(mSocketFd,
                             msg,
                             &response);

    if ((rc < 0) || (AUD_DEV_ARBI_IPC_MSG_ACK != response)) {
        result = false;
    }

    pthread_mutex_unlock(&mMutex);

    return result;
}

/*============================================================================
    FUNCTION:  ClientIpcEndpoint::~ClientIpcEndpoint
============================================================================*/
ClientIpcEndpoint::~ClientIpcEndpoint()
{
}
