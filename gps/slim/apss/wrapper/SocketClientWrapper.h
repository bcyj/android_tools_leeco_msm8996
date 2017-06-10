/*============================================================================
@file SocketClientWrapper.h

  Socket Client Wrapper header file

  Socket client wrapper converts request from sensor cleint into socket
  messages and forwards it to the daemon. Also delivers sensor data from provider
  using socket communication

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef SOCKET_CLIENT_WRAPPER_H
#define SOCKET_CLIENT_WRAPPER_H

#include <errno.h>
#include <grp.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include "SocketClientTypes.h"

namespace socket_client_wrapper {

class SocketClientWrapper {
    static SocketClientWrapper* mMe;
protected:

public:
    static pthread_t SCWthread;
    static int mSocketFd;
    static slimNotifyCallbackFunctionT mCallbackFunction;
    SocketClientWrapper();
    inline virtual ~SocketClientWrapper() {};

    static SocketClientWrapper* get();

    static void* SocketClientWrapperThread(void* arg);
    virtual int ClientRegister(slim_OpenTxnStructType &z_Txn);
    virtual int ClientDeRegister(slim_GenericTxnStructType &z_Txn);

    virtual int requestSensorData(slim_EnableSensorDataTxnStructType &z_Txn);
    //virtual int requestPedometerDataRequest(slim_EnablePedometerTxnStructType &z_Txn);

    virtual int OpenConnection();

    static int ProcessReceivedMessage(
        SocketClientMsgHeader &slim_msg_hdr,
        uint32_t q_msg_payload_size,
        void* slim_msg_ptr
        );

};

};//namespace

#endif //SOCKET_CLIENT_WRAPPER_H
