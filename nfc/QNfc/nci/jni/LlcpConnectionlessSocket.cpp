/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "LlcpConnectionlessSocket.h"

#include "OverrideLog.h"

// <DTA>
#include "dta_mode.h"
// </DTA>


// #### STATIC PART ####

/**
 * The maximum number of local SAPs available.
 */
static const int NFA_P2P_NUM_SAP = 64;

/**
 * NFA P2P event names.
 */
static const char* p2pEvents[] = {
    "NFA_P2P_REG_SERVER_EVT",     //  0x00    /* Server is registered                         */
    "NFA_P2P_REG_CLIENT_EVT",     //  0x01    /* Client is registered                         */
    "NFA_P2P_ACTIVATED_EVT",      //  0x02    /* LLCP Link has been activated                 */
    "NFA_P2P_DEACTIVATED_EVT",    //  0x03    /* LLCP Link has been deactivated               */
    "NFA_P2P_CONN_REQ_EVT",       //  0x04    /* Data link connection request from peer       */
    "NFA_P2P_CONNECTED_EVT",      //  0x05    /* Data link connection has been established    */
    "NFA_P2P_DISC_EVT",           //  0x06    /* Data link connection has been disconnected   */
    "NFA_P2P_DATA_EVT",           //  0x07    /* Data received from peer                      */
    "NFA_P2P_CONGEST_EVT",        //  0x08    /* Status indication of outgoing data           */
    "NFA_P2P_LINK_INFO_EVT",      //  0x09    /* link MIU and Well-Known Service list         */
    "NFA_P2P_SDP_EVT"             //  0x0A    /* Remote SAP of SDP result                     */
};

/**
 * Prints a debug entry that consists of status, function name and free text part.
 */
static void debugStatus(tNFA_STATUS status, const char* function, const char* text) {

  if (status == NFA_STATUS_OK) {
    ALOGD("%s [CL]: %s, result = NFA_STATUS_OK", function, text);
  }
  else {
    ALOGE("%s [CL]: %s, result = NFA_STATUS_FAILED", function, text);
  }

}

/**
 * The main mutex for making sure that all socket instances are thread-safe.
 */
Mutex LlcpConnectionlessSocket::mainMutex;

/**
 * The mutex that is used for synchronizing registration of the socket.
 */
SyncEvent LlcpConnectionlessSocket::regEvent;

/**
 * The registration handle of the socket being opened.
 */
tNFA_HANDLE LlcpConnectionlessSocket::regHandle = NFA_HANDLE_INVALID;

/**
 * A mutex for synchronizing the SDP request sequence.
 */
SyncEvent LlcpConnectionlessSocket::sdpEvent;

/**
 * Gets the SAP number stored in the handle.
 *
 * @param handle The NFA handle.
 *
 * @return the SAP number.
 */
static UINT8 getSap(tNFA_HANDLE handle) {

  return (UINT8) (handle & NFA_HANDLE_MASK);
}


// #### STATIC SOCKET MEMBERS ####

/**
 * An array containing all connectionless LLCP sockets.
 */
LlcpConnectionlessSocket LlcpConnectionlessSocket::socketMap[NFA_P2P_NUM_SAP];

/**
 * Closes all open sockets. Called when the NFC stack is shut down.
 */
void LlcpConnectionlessSocket::closeAllSockets() {

  for (unsigned int i = 0; i < NFA_P2P_NUM_SAP; ++i) {
    socketMap[i].close();
  }

  // Release all static mutexes just to be sure:
  mainMutex.unlock();
  regEvent.notifyOne();
  sdpEvent.notifyOne();
}

/**
 * Gets the LlcpConnectionlessSocket instance corresponding to the specified local SAP.
 *
 * @param localSap The local SAP number or handle of the socket.
 *
 * @return The pointer at the LlcpConnectionlessSocket instance or 0 if the socket is not open.
 */
LlcpConnectionlessSocket* LlcpConnectionlessSocket::getSocket(int handle) {

  const UINT8 local_sap = getSap((tNFA_HANDLE)handle);

  ALOGD("%s [CL]: sap=0x%x", __FUNCTION__, local_sap);

  if (local_sap >= NFA_P2P_NUM_SAP) {
    return 0;
  }

  return &socketMap[local_sap];
}

/**
 * Handles the tNFA_P2P_EVT events.
 *
 * @param event The P2P event.
 * @param p_data The P2P event data.
 */
void LlcpConnectionlessSocket::notifyNfaP2pEvent(tNFA_P2P_EVT event, tNFA_P2P_EVT_DATA *eventData) {

  ALOGD("%s [CL]: enter, event 0x%x", __FUNCTION__, event);

  if (event > NFA_P2P_SDP_EVT) {
    ALOGE ("%s [CL]: unknown event received: 0x%x !!", __FUNCTION__, event);
    return;
  }

  ALOGD ("%s [CL]: %s event received", __FUNCTION__, p2pEvents[event]);

  if (event == NFA_P2P_REG_CLIENT_EVT) {
    SyncEventGuard guard(regEvent);

    regHandle = eventData->reg_client.client_handle;

    ALOGD("%s [CL]: CLIENT HANDLE=0x%04x; client sap=0x%02x", __FUNCTION__, regHandle, getSap(regHandle));

    regEvent.notifyOne();
  }
  else if (event == NFA_P2P_REG_SERVER_EVT) {
    SyncEventGuard guard(regEvent);

    regHandle = eventData->reg_server.server_handle;

    ALOGD ("%s [CL]: SERVER HANDLE=0x%04x; service sap=0x%02x, service name: %s", __FUNCTION__,
        regHandle, eventData->reg_server.server_sap, eventData->reg_server.service_name);

    regEvent.notifyOne();
  }
  else if (event == NFA_P2P_ACTIVATED_EVT) {
    AutoMutex lock(mainMutex);

    const tNFA_P2P_ACTIVATED& act = eventData->activated;

    LlcpConnectionlessSocket* socket = getSocket(act.handle);
    if (socket != 0) {
      socket->localMiu = act.local_link_miu;
      socket->remoteMiu = act.remote_link_miu;
    }

    ALOGD("%s [CL]: local handle=0x%04x; local SAP=0x%02x; local MIU=%d; remote MIU=%d", __FUNCTION__,
          act.handle, getSap(act.handle), act.local_link_miu, act.remote_link_miu);
  }
  else if (event == NFA_P2P_SDP_EVT) {
    SyncEventGuard guard(sdpEvent);

    const tNFA_P2P_SDP& sdp = eventData->sdp;
    LlcpConnectionlessSocket* socket = getSocket(sdp.handle);
    if (socket != 0) {
      socket->remoteSapForWrite = sdp.remote_sap;
    }

    ALOGD("%s [CL]: LOCAL HANDLE=0x%x, REMOTE_SAP=0x%x", __FUNCTION__, sdp.handle, sdp.remote_sap);
    sdpEvent.notifyOne();
  }
  else if (event == NFA_P2P_DATA_EVT) {
    AutoMutex lock(mainMutex);

    const tNFA_P2P_DATA& data = eventData->data;

    if ((data.link_type & NFA_P2P_LLINK_TYPE) != 0) {
      ALOGD("%s [CL]: logical data link PDU", __FUNCTION__);

      LlcpConnectionlessSocket* socket = getSocket(data.handle);
      if (socket != 0 && socket->readEvent != 0) {
        SyncEventGuard guard(*(socket->readEvent));
        socket->remoteSapForRead = data.remote_sap;
        socket->readEvent->notifyOne();
      }
    }
    else {
      ALOGD("%s [CL]: *not* logical data link PDU - ignore", __FUNCTION__);
    }

    ALOGD("%s [CL]: LOCAL HANDLE=0x%x, REMOTE_SAP=0x%x", __FUNCTION__, data.handle, data.remote_sap);
  }
  else if (event == NFA_P2P_LINK_INFO_EVT) {
    AutoMutex lock(mainMutex);

    const tNFA_P2P_LINK_INFO& info = eventData->link_info;
    LlcpConnectionlessSocket* socket = getSocket(info.handle);
    if (socket != 0) {
      socket->localMiu = info.local_link_miu;
      socket->remoteMiu = info.remote_link_miu;
    }

    ALOGD("%s [CL]: local handle=0x%04x; local sap=0x%02x; local MIU=%d; remote MIU=%d", __FUNCTION__,
          info.handle, getSap(info.handle), info.local_link_miu, info.remote_link_miu);
  }
  else if (event == NFA_P2P_DEACTIVATED_EVT) {
    // Call sdpEvent.notifyOne() to avoid stack freeze incase packet sending is pending.
    // mainMutex is locked in the send method and the stack will freeze here if notifyOne() is not called.
    sdpEvent.notifyOne();
    AutoMutex lock(mainMutex);

    const tNFA_P2P_DEACTIVATED& deac = eventData->deactivated;
    LlcpConnectionlessSocket* socket = getSocket(deac.handle);
    if (socket != 0) {
      // Reset remote peer related variables:
      socket->remoteSapForRead = 0;
      socket->currRemoteServiceName.clear();
      socket->remoteMiu = LLCP_DEFAULT_MIU;
    }

    ALOGD("%s [CL]: local handle=0x%04x; local sap=0x%02x", __FUNCTION__,
          deac.handle, getSap(deac.handle));
  }

}

/**
 * Opens a new connectionless LLCP service socket that is registered to the speficied local SAP and will provide the specified
 * local service name for remote peers. However, if the local service name parameter is null or empty string, the socket is
 * registered as a client socket to have the next free local SAP and the parameter SAP is ignored.
 *
 * @param localServiceName The name of the service accessible via the socket. Can be null or empty string.
 * @paral localSap The local SAP of the socket in the range 11h-1Fh. If the service name is not provided, the local SAP is ignored and
 *  the next free SAP will be assigned for the socket.
 *
 * @return The LlcpConnectionless socket or 0 if the socket could not be opened.
 */
LlcpConnectionlessSocket*
LlcpConnectionlessSocket::open(const char* localServiceName, int localSap) {

  AutoMutex lock(mainMutex);
  SyncEventGuard guard(regEvent);

  ALOGD("%s [CL]: localSap=0x%x", __FUNCTION__, localSap);

  tNFA_STATUS registrationStatus = NFA_STATUS_OK;
  LlcpConnectionlessSocket* socket = 0;

  // If localServiceName is not provided, register as client:
  if (localServiceName == 0 || strlen(localServiceName) == 0) {
    registrationStatus = NFA_P2pRegisterClClient(NFA_P2P_LLINK_TYPE, (uint8_t) localSap, &notifyNfaP2pEvent);
  }
  // Otherwise as server:
  else {
    registrationStatus = NFA_P2pRegisterServer(localSap, NFA_P2P_LLINK_TYPE, (char*)localServiceName, &notifyNfaP2pEvent);
  }

  if (registrationStatus == NFA_STATUS_OK) {
    regEvent.wait();

    ALOGD("%s [CL]: continue on regEvent", __FUNCTION__);

    socket = getSocket(regHandle);
    if (socket != 0) {
      socket->socketHandle = regHandle;
      socket->readEvent = new SyncEvent();
    }
    else {
      registrationStatus = NFA_STATUS_FAILED;
    }
  }

  debugStatus(registrationStatus, __FUNCTION__, "socket registration");
  return socket;
}

/**
 * Opens a new client socket that will automatically get the next free local SAP assigned to it.
 *
 * @return The client socket instance or 0 if opening the socket failed.
 */
LlcpConnectionlessSocket*
LlcpConnectionlessSocket::open() {

  return open(0,0);
}


// #### DYNAMIC SOCKET MEMBERS ####

/**
 * Initializes an unregistered socket instance.
 */
LlcpConnectionlessSocket::LlcpConnectionlessSocket() :
    socketHandle(NFA_HANDLE_INVALID),
    localMiu(LLCP_DEFAULT_MIU), remoteMiu(LLCP_DEFAULT_MIU),
    remoteSapForWrite(0), currRemoteServiceName(),
    readEvent(0), remoteSapForRead(0)
{
  // ALOGD("%s [CL]: constructor called.", __FUNCTION__);
}

/**
 * Gets the local SAP assigned to the socket.
 *
 * @return The local SAP assigned to the socket or -1 if the socket is not open.
 */
int LlcpConnectionlessSocket::getLocalSap() {

  AutoMutex lock(mainMutex);

  if (!isOpen()) {
    return -1;
  }

  return getSap(socketHandle);
}

/**
 * Checks whether the socket is open.
 *
 * @return true if the socket is open, false otherwise.
 */
bool LlcpConnectionlessSocket::isOpen() {

  return socketHandle != NFA_HANDLE_INVALID;
}

/**
 * Sends the specified data packet to the remove SAP port.
 *
 * @param remoteSAP The remote SAP.
 * @param data The data to send.
 *
 * @return true if the sending was successful, false otherwise.
 */
bool LlcpConnectionlessSocket::sendPDU(int remoteSap, const std::vector<uint8_t>& data) {

  ALOGD("%s [CL]: local SAP = 0x%x, remote SAP = 0x%x; data len = %d", __FUNCTION__, getSap(socketHandle), remoteSap, data.size());
  //debugMessage("LlcpConnectionlessSocket.sendPDU: [CL] MESSAGE", (unsigned char*)&data[0], data.size());

  if (!isOpen()) {
    ALOGE("%s [CL]: the socket is closed - ignore send!", __FUNCTION__);
    return false;
  }

  tNFA_STATUS status = NFA_P2pSendUI(socketHandle, remoteSap, data.size(), (unsigned char*)&data[0]);

  if (status != NFA_STATUS_OK) {
    ALOGE("%s [CL]: NFA_P2pSendUI failed, status = %d", __FUNCTION__, status);
    return false;
  }

  ALOGD("%s [CL]: NFA_P2pSendUI successful, status = NFA_STATUS_OK", __FUNCTION__);
  return true;
}

/**
 * Sends the specified data packet to the remove SAP port.
 *
 * @param remoteSAP The remote SAP.
 * @param data The data to send.
 *
 * @return true if the sending was successful, false otherwise.
 */
bool LlcpConnectionlessSocket::send(int remoteSap, const std::vector<uint8_t>& data) {

  AutoMutex lock(mainMutex);

  return sendPDU(remoteSapForWrite, data);
}

/**
 * Sends the specified data packet to the remote service.
 *
 * @param remoteServiceName The name of the remote service.
 * @param data The data to send.
 *
 * @return true if the sending was successful, false otherwise.
 */
bool LlcpConnectionlessSocket::send(const char* remoteServiceName, const std::vector<uint8_t>& data) {

  AutoMutex lock(mainMutex);

  ALOGD("%s [CL]", __FUNCTION__);

  if (remoteServiceName == 0 || strlen(remoteServiceName) == 0) {
    ALOGE("%s [CL]: the remote service name is null or empty - cannot send!", __FUNCTION__);
    return false;
  }

  if (!isOpen()) {
    ALOGE("%s [CL]: the socket is closed - cannot send!", __FUNCTION__);
    return false;
  }

  tNFA_STATUS status = NFA_STATUS_OK;

  if (currRemoteServiceName.compare(remoteServiceName) != 0 || remoteSapForWrite == 0) {
    SyncEventGuard guard(sdpEvent);

    remoteSapForWrite = 0;
    status = NFA_P2pGetRemoteSap(socketHandle, (char*) remoteServiceName);
    debugStatus(status, __FUNCTION__, "get remote SAP");

    if (status == NFA_STATUS_OK) {
      sdpEvent.wait();
      if (remoteSapForWrite != 0) {
        currRemoteServiceName.assign(remoteServiceName);
      }
    }
  }

  if (status == NFA_STATUS_OK && remoteSapForWrite != 0) {
    return sendPDU(remoteSapForWrite, data);
  }

  return false;
}

/**
 * Receives a PDU from a remote peer. This method will block indefinitely until it receives the PDU or
 * the socket gets closed.
 *
 * @param data The data vector where the received PDU is stored.
 *             The vector is automatically resized to match the size of the PDU.
 *
 * @return The remote SAP of the remote peer or -1 if the socket was closed before receiving the PDU.
 */
int LlcpConnectionlessSocket::receive(std::vector<unsigned char>& data) {

  ALOGD("%s [CL]", __FUNCTION__);

  while (isOpen()) {
    {
      AutoMutex lock(mainMutex);

      data.resize(localMiu);
      UINT32 numReadBytes = 0;
      BOOLEAN isThereMore = false;

      ALOGD("%s [CL]: trying to read data, data.size()=%d", __FUNCTION__, data.size());

      tNFA_STATUS status = NFA_P2pReadUI(socketHandle,
                                         localMiu,
                                         &remoteSapForRead,
                                         &numReadBytes,
                                         (UINT8*)&data[0],
                                         &isThereMore);

      ALOGD("%s [CL]: data reading attempt over, numReadBytes=%d", __FUNCTION__, numReadBytes);

      if (status == NFA_STATUS_OK && numReadBytes > 0) {
        data.resize(numReadBytes);
        ALOGD("%s [CL]: read successfully %d bytes. Local SAP=0x%x, remote SAP=0x%x", __FUNCTION__, numReadBytes, getSap(socketHandle), remoteSapForRead);
        return remoteSapForRead;
      }
    }

    ALOGD("%s [CL]: waiting for data... Local SAP=0x%x", __FUNCTION__, getSap(socketHandle));

    if (readEvent != 0) {
      SyncEventGuard guard(*readEvent);
      readEvent->wait();
    }

  }

  return -1;
}

/**
 * Closes the socket.
 */
void LlcpConnectionlessSocket::close() {

  AutoMutex lock(mainMutex);

  ALOGD("%s [CL]: local sap=0x%x", __FUNCTION__, getSap(socketHandle));

  if (isOpen()) {
    NFA_P2pDeregister(socketHandle);
    socketHandle = NFA_HANDLE_INVALID;
  }

  // Release the read mutex:
  if (readEvent != 0) {
    readEvent->notifyOne();
    delete readEvent;
    readEvent = 0;
  }

}
