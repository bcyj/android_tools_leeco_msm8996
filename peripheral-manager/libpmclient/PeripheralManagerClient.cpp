/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_TAG "PerMgrLib"

#define CLIENT_REGISTER_RETRY_INTERVAL 1    // seconds
#define CLIENT_REGISTER_RETRY_COUNT    5    // xCLIENT_REGISTER_RETRY_INTERVAL

extern "C" {
#include "mdm_detect.h"
}

#include "PeripheralManagerClient.h"

namespace android {


list<struct PeripheralManagerClient *> mClientsInProcess;
Mutex mClientsInProcessLock;

static int pm_is_supported()
{
#ifdef PER_MGR_SUPPORTED
    return 1;
#else
    return 0;
#endif
}

// Search in above list for client. If client was found function will get
// structure access lock. If paramter 'remove' is true, found element will
// be removed from global list of valid clients.
bool pm_client_find_and_lock(PeripheralManagerClient *searched, bool remove) {
    list<PeripheralManagerClient *>::iterator iter;
    PeripheralManagerClient *client;

    Mutex::Autolock lock(mClientsInProcessLock);

    for (iter = mClientsInProcess.begin(); iter != mClientsInProcess.end(); iter++) {
        client = *iter;
        if (client == searched) {
            client->mLock.lock();
            if (remove) {
                mClientsInProcess.erase(iter);
            }
            return true;
        }
    }

    return false;
}

// Unlock client previously locked from the above function
void pm_client_unlock(PeripheralManagerClient *client) {
    client->mLock.unlock();
}

// Helper function used for initial client registration and reregistraion
// of the existing clients. User of the functions have to pass valid
// 'client' reference and 'devName', 'connected' and 'notifier' have to
// be set to valid values.
int pm_register_connect(struct PeripheralManagerClient *client, enum pm_event *state) {
    sp<IServiceManager> sm = defaultServiceManager();
    int64_t devSate, clientId;
    sp<IBinder> service;
    status_t status;

    service = sm->getService(String16("vendor.qcom.PeripheralManager"));
    if (service == NULL) {
        ALOGE("%s get service fail", (const char *)client->devName);
        return -1;
    }

    client->ops = IPeripheralManager::asInterface(service);
    if (client->ops == NULL) {
        ALOGE("%s get interface fail", (const char *)client->devName);
        return -1;
    }

    status = client->ops->registar(client->devName, client->clientName,
                                   client->notifier, &clientId, &devSate);
    if (status != OK) {
        ALOGE("%s registar fail", (const char *)client->devName);
        goto cleanup;
    }

    client->recipient = new ServerDiedNotifier(client);
    if (client->recipient == NULL) {
        ALOGE("%s death notifier fail", (const char *)client->devName);
        goto unregister;
    }

    if (client->ops->asBinder() != NULL) {
            if (client->ops->asBinder()->linkToDeath(client->recipient)) {
                    ALOGE("%s link to binder fail",
                                    (const char *)client->devName);
                    goto unregister;
            }
    }


    // This state track whether client was already successfully
    // connected to server.
    if (client->connected) {
        // Reconnect after server have been restarted
        status = client->ops->connect(clientId);
        if (status != OK) {
            // We will try after a second
            ALOGE("%s connect fail", (const char *)client->devName);
            goto unlink;
        }
    }

    if (state) {
        *state = (enum pm_event) devSate;
    }

    client->id = clientId;

    // Put this client in list of know/valid users.
    mClientsInProcessLock.lock();
    mClientsInProcess.push_back(client);
    mClientsInProcessLock.unlock();

    return 0;

unlink:
    client->ops->asBinder()->unlinkToDeath(client->recipient);
unregister:
    client->ops->unregister(clientId);
cleanup:
    client->id = 0;
    client->ops = NULL;
    client->recipient = NULL;
    return -1;
}


ServerDiedNotifier::ServerDiedNotifier(struct PeripheralManagerClient *client) {
    mClient = client;
}

void ServerDiedNotifier::binderDied(const wp<IBinder> &who) {
    // Just silent unused parameter warning
    const wp<IBinder> unused = who;
    enum pm_event state;
    int cnt, status;

    ALOGE("Peripheral manager server died");

    // Ensure this is still valid client reference and remove
    // it temporally from the list. It will be added again to this
    // list of successful re-registration
    if (!pm_client_find_and_lock(mClient, true)) {
        return;
    }
    if (mClient->ops->asBinder() != NULL)
        mClient->ops->asBinder()->unlinkToDeath(mClient->recipient);
    else
        ALOGE("Failed to get binder object");
    mClient->id = 0;
    mClient->ops = NULL;
    mClient->recipient = NULL;

    for (cnt = 0; cnt < CLIENT_REGISTER_RETRY_COUNT; cnt++) {

        sleep(CLIENT_REGISTER_RETRY_INTERVAL);

        // Register our client again and connect it, if it was
        // connected before server crash
        status = pm_register_connect(mClient, &state);
        if (status == 0) {
            ALOGI("Peripheral manager server alive");
            break;
        }
    }

    pm_client_unlock(mClient);
    if (status != 0) {
        delete mClient;
    }
}

ServerQuitNotifier::ServerQuitNotifier(Mutex *lock) {
    mDone = lock;
}

void ServerQuitNotifier::binderDied(const wp<IBinder> &who) {
    // Just silent unused parameter warning
    const wp<IBinder> unused = who;

    ALOGD("Peripheral manager server exited");

    mDone->unlock();
}

EventNotifier::EventNotifier(pm_client_notifier notifier, void *clientData) {
    mClientCb = notifier;
    mClientData = clientData;
}

void EventNotifier::notifyCallback(int32_t event) {
    if (mClientCb) {
        // Execute client supplied notifier
        mClientCb(mClientData, (enum pm_event) event);
    }
}

}; // namespace android

using namespace android;

extern "C" {
int pm_show_peripherals(const char *names[PERIPH_MAX_SUPPORTED]) {
    struct dev_info modems;
    int cnt;

    if (names == NULL) {
        return -1;
    }

    // Ask libmdmdetect for available pripherals
    if (get_system_info(&modems) != RET_SUCCESS) {
        ALOGE("Could not retrieve peripheral devices info");
        return -1;
    }

    for (cnt = 0; cnt < PERIPH_MAX_SUPPORTED && cnt < modems.num_modems; cnt++) {
        names[cnt] = modems.mdm_list[cnt].mdm_name;
    }

    return cnt;
}

int pm_client_register(pm_client_notifier cb, void *clientData, const char *devName,
                       const char *clientName, enum pm_event *state, void **handle) {
    struct PeripheralManagerClient *client;
    int ret;

    if(!pm_is_supported()) {
        ALOGE("Peripheral manager is not supported on this device");
        return PM_RET_UNSUPPORTED;
    }
    if (devName == NULL || clientName == NULL || handle == NULL) {
        ALOGE("Peripheral/Client name not specified");
        return PM_RET_FAILED;
    }
    client = new PeripheralManagerClient();
    if (!client) {
        ALOGE("%s create client fail", clientName);
        goto error;
    }
    client->devName = devName;
    client->clientName = clientName;
    // Client has not issued connect call, yet
    client->connected = false;

    client->notifier = new EventNotifier(cb, clientData);
    if (client->notifier == NULL) {
        ALOGE("%s cleate event notifier fail", clientName);
        delete client;
        goto error;
    }
    // In this case we will just try to registar our client to
    // server, no connect call. See comment above.
    ret = pm_register_connect(client, state);
    if (ret) {
        client->notifier = NULL;
        delete client;
        goto error;
    } else {
        // Allow binder to use our thread when calls us
        ProcessState::self()->startThreadPool();
    }
    *handle = client;
    ALOGD("%s successfully registered for %s", clientName, devName);
    return PM_RET_SUCCESS;
error:
    ALOGE("%s failed to register for %s", clientName, devName);
    return PM_RET_FAILED;
}

int pm_client_unregister(void *clientId) {
    struct PeripheralManagerClient *client;

    client = (struct PeripheralManagerClient *)clientId;

    // Check for valid reference and remove it from the list
    // with valid clients to prevent any erroneous API calls
    if (!pm_client_find_and_lock(client, true)) {
        return PM_RET_FAILED;
    }

    ALOGD("%s unregistering from peripheral manager",
          client->clientName.string());

    client->ops->unregister(client->id);

    if (client->ops->asBinder() != NULL)
        client->ops->asBinder()->unlinkToDeath(client->recipient);
    else
        ALOGE("Failed to get binder object for client : %s",
                        client->clientName.string());
    client->id = 0;
    // Release sp<>
    client->ops = NULL;
    client->notifier = NULL;
    client->recipient = NULL;

    pm_client_unlock(client);
    delete client;
    return PM_RET_SUCCESS;
}

int pm_client_connect(void *clientId) {
    struct PeripheralManagerClient *client;
    status_t status;

    client = (struct PeripheralManagerClient *)clientId;

    // Check for valid reference and lock the structure if found.
    // This will prevent execution in parallel of the unregister API.
    if (!pm_client_find_and_lock(client, false)) {
        return PM_RET_FAILED;
    }

    ALOGD("%s voting for %s", client->clientName.string(),
          client->devName.string());

    status = client->ops->connect(client->id);
    if (status == OK) {
        // Marking client as connected. This state will be used if Server
        // crashes and restarts to reconnect client automatically
        client->connected = true;
    }

    // Release client lock
    pm_client_unlock(client);

    return (status == OK) ? PM_RET_SUCCESS : PM_RET_FAILED;
}

int pm_client_disconnect(void *clientId) {
    struct PeripheralManagerClient *client;
    status_t status;

    client = (struct PeripheralManagerClient *)clientId;

    // Check for valid reference and lock the structure if found.
    // This will prevent execution in parallel of the unregister API.
    if (!pm_client_find_and_lock(client, false)) {
        return PM_RET_FAILED;
    }

    ALOGD("%s unvoting for %s", client->clientName.string(),
          client->devName.string());

    status = client->ops->disconnect(client->id);
    if (status == OK) {
        // Marking client as disconnected.
        client->connected = false;
    }

    pm_client_unlock(client);

    return (status == OK) ? PM_RET_SUCCESS : PM_RET_FAILED;
}

int pm_client_event_acknowledge(void *clientId, enum pm_event event) {
    struct PeripheralManagerClient *client;
    status_t status;

    client = (struct PeripheralManagerClient *)clientId;

    // Check for valid reference and lock the structure if found.
    // This will prevent execution in parallel of the unregister API.
    if (!pm_client_find_and_lock(client, false)) {
        return PM_RET_FAILED;
    }

    status = client->ops->acknowledge(client->id, (int64_t)event);

    pm_client_unlock(client);

    return (OK == status) ? PM_RET_SUCCESS : PM_RET_FAILED;
}

/*
   Shutdown Peripheral Manager Service. This call wll block and wait
   untill serivice exit. Periperal manager will automaticaly power-off
   all peripehrals before exit. All registered clients will get
   EVENT_PERIPH_GOING_OFFLINE/EVENT_PERIPH_IS_OFFLINE(if peripehral was
   powered on) before service exit. This request will be processed by serviced
   only if user have requred UID.
   Return 0 on success or -1 on error
 */
int pm_service_shutdown(void) {
    sp<IServiceManager> sm = defaultServiceManager();
    sp<ServerQuitNotifier> quitNotifier;
    sp<IPeripheralManager> pmInterface;
    sp<IBinder> service;
    Mutex quitComplete;
    status_t status;

    service = sm->getService(String16("vendor.qcom.PeripheralManager"));
    if (service == NULL) {
        ALOGE("Get service fail");
        return PM_RET_FAILED;
    }

    pmInterface = IPeripheralManager::asInterface(service);
    if (pmInterface == NULL) {
        ALOGE("Get interface fail");
        return PM_RET_FAILED;
    }

    quitNotifier = new ServerQuitNotifier(&quitComplete);
    if (pmInterface->asBinder() != NULL) {
        if (pmInterface->asBinder()->linkToDeath(quitNotifier)) {
            ALOGE("Quit notification fail");
            return PM_RET_FAILED;
        }
    } else {
        ALOGE("Failed to get binder interface object");
    }
    // Allow binder to use our thread when calls us
    ProcessState::self()->startThreadPool();

    quitComplete.lock();
    status = pmInterface->shutdown();
    if (status == OK) {
        quitComplete.lock();
    }

    return (status == OK) ? PM_RET_SUCCESS : PM_RET_FAILED;
}

}       // extern "C"
