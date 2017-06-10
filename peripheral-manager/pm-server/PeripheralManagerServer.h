/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __PM_SERVER_H__
#define __PM_SERVER_H__

#include <list>
#include <queue>

#include <binder/MemoryHeapBase.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

#include "IPeripheralManager.h"
#include "Client.h"
#include "Peripheral.h"
#include "pm-service.h"

using namespace std;

namespace android {

class PeripheralManagerServer: public BnPeripheralManager {

  public:
    // Interface to APP peripheral maager clients
    status_t registar(const String8 &devName, const String8 &clientName,
                      const sp<IPeriperalManagerCb> &notifier, int64_t *clientId,
                      int64_t *devState);

    status_t unregister(int64_t clientId);
    status_t connect(int64_t clientId);

    status_t disconnect(int64_t clientId);
    status_t acknowledge(int64_t clientId, int32_t event);

    status_t shutdown();

    void dumpPeripheralInfo();
    void clientDied(Client *client);

    status_t initPeripherals(int offTimeout, int ackTimeout, bool debugMode);
    status_t deInitPeripherals();
    // Usually called in a SSREQ
    status_t restartPeripheral(int subSystemId, qmi_client_handle qmiClient);

    // Load modems info
    struct dev_info *loadModemInfo();

  private:
    // Internal functions
    Peripheral *findByName(const String8 &name);
    Peripheral *findBySubSystem(int subSystemId);

    list<Peripheral *> mPeripherals;
};

}; // namespace android
#endif
