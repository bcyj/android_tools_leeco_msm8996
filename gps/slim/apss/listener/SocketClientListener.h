/*============================================================================
@file SocketClientListener.h

  Socket Client Listener header file

  Socket client listener is interface to AP sensor clients
  (PIP) Sensor data requests and sensor data
  injection is through unix socket.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef SOCKET_CLIENT_LISTENER_H
#define SOCKET_CLIENT_LISTENER_H

#include <ClientListener.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <pthread.h>
#include <stddef.h>
#include "SocketClientTypes.h"

namespace socket_client_listener {

class SocketClientListener : public ClientListener {
    static SocketClientListener* mMe;
protected:
    int mClientFd;
    int mServerFd;
public:
    static pthread_t mSCLthread;
    static slimServiceProviderEnumT mProvider;
    SocketClientListener(void* pMsgQ);
    inline virtual ~SocketClientListener() {}
    int SocketClientListenerThread();
    static void* ListenerThreadEntry(void* arg);

    static SocketClientListener* get(void* pMsgQ);

    //void handleSSR();

    static void forwardCallbackData(uint64_t t_CallbackData,
                                    const slimMessageHeaderStructT *pz_MessageHeader,
                                    void *p_Message);

    void callbackHandler(uint64 t_CallbackData,
                         const slimMessageHeaderStructT *pz_MessageHeader,
                         void *p_Message);

    bool handleSensorService(uint64 t_CallbackData,
                             const slimMessageHeaderStructT *pz_MessageHeader,
                             void *p_Message);

    bool handleTimeService(uint64 t_CallbackData,
                           const slimMessageHeaderStructT *pz_MessageHeader,
                           void *p_Message);

    bool handlePedometerService(uint64 t_CallbackData,
                                const slimMessageHeaderStructT *pz_MessageHeader,
                                void *p_Message);


    bool injectTimeData(SocketClientTimeInd timeInd);

    bool injectSensorData(SocketClientSensorDataInd sensorDataInd);

};

};//namespace

#endif //SOCKET_CLIENT_LISTENER_H
