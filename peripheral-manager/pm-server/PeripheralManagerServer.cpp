/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_TAG "PerMgrSrv"

#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/reboot.h>

#include <utils/Errors.h>
#include <cutils/properties.h>
#include <cutils/log.h>

#include <private/android_filesystem_config.h>

#include "qmi_idl_lib.h"
#include "qmi_csi.h"
#include "qmi_csi_common.h"
#include "qmi_cci_target_ext.h"

extern "C" {
#include "mdm_detect.h"
}

#include "subsystem_request_v01.h"
#include "PeripheralManagerServer.h"

namespace android {

#define QMI_START_RETRY_INTERVAL        10   // ms
#define APP_CLIENTS_ACK_DELAY           200  // ms
#define APP_CLIENTS_OFF_DELAY           500  // ms

struct QmiClientInfo {
    const char *name;
    int aid_cnt;
    int aid[4];
};

struct QmiClientInfo qmiClientsInfo[] = {
    [SSREQ_QMI_CLIENT_INSTANCE_APSS_V01] = {
        "APPS", 3, {AID_ROOT, AID_RADIO, AID_SYSTEM}
    },
    [SSREQ_QMI_CLIENT_INSTANCE_MPSS_V01] = {
        "MPSS", 4, {AID_ROOT, AID_RADIO, AID_SYSTEM, AID_MEDIA}
    },
    [SSREQ_QMI_CLIENT_INSTANCE_WCNSS_V01] = {
        "WCNSS", 3, {AID_ROOT, AID_RADIO, AID_SYSTEM}
    },
    [SSREQ_QMI_CLIENT_INSTANCE_ADSP_V01] = {
        "ADSP", 3, {AID_ROOT, AID_RADIO, AID_SYSTEM}
    },
    [SSREQ_QMI_CLIENT_INSTANCE_VPU_V01] = {
        "VPU", 3, {AID_ROOT, AID_RADIO, AID_SYSTEM}
    },
    [SSREQ_QMI_CLIENT_INSTANCE_MDM_V01] = {
        "MDM", 3, {AID_ROOT, AID_RADIO, AID_SYSTEM}
    },
    [SSREQ_QMI_CLIENT_INSTANCE_QSC_V01] = {
        "QSC", 3, {AID_ROOT, AID_RADIO, AID_SYSTEM}
    }
};

struct QmiThreadInfo {
    PeripheralManagerServer *pmService;
    int quitFd;
};

static int getUID() {
    return IPCThreadState::self()->getCallingUid();
}

static int getPID() {
    return IPCThreadState::self()->getCallingPid();
}

status_t PeripheralManagerServer::registar(const String8 &devName,
                                           const String8 &clientName,
                                           const sp<IPeriperalManagerCb> &notifier,
                                           int64_t *clientId, int64_t *devState) {
    Peripheral *peripheral;
    Client *client;

    *clientId = 0;
    *devState = 0;

    peripheral = findByName(devName);
    if (!peripheral) {
        ALOGE("No such peripheral %s", (const char *)devName);
        return BAD_VALUE;
    }

    if (!peripheral->checkPermission(getUID())) {
        ALOGE("Permission denied, client UID %d, peripehral %s",
              getUID(), (const char *)devName);
        return PERMISSION_DENIED;
    }

    client = new Client(notifier, this, clientName);
    if (!client) {
        ALOGE("Failed to create new client struct for %s for peripheral %s",
              (const char *)clientName, (const char *)devName);
        return NO_MEMORY;
    }

    *clientId = (int64_t)client;
    *devState = (int64_t)peripheral->getState();

    peripheral->addClient(client);

    ALOGD("%s registered", (const char *)clientName);

    return OK;
}

status_t PeripheralManagerServer::unregister(int64_t clientId) {
    list<Peripheral *>::iterator iter;
    Peripheral *peripheral;
    bool found = false;
    Client *client;

    client = (Client *)clientId;

    // Be paranoid and validate user input
    for (iter = mPeripherals.begin(); iter != mPeripherals.end(); iter++) {
        peripheral = *iter;
        found = peripheral->findClient(client);
        if (found)
            break;
    }

    if (!found) {
        ALOGE("Unregister: No such client %p", (void *)clientId);
        return NAME_NOT_FOUND;
    }

    // Ensure that client did not hold Device powered
    peripheral->decreaseVoters(client, false);
    peripheral->removeClient(client);

    ALOGD("%s unregistered", client->getName());

    delete client;
    return OK;
}

status_t PeripheralManagerServer::connect(int64_t clientId) {
    list<Peripheral *>::iterator iter;
    Peripheral *peripheral;
    bool found = false;
    Client *client;

    client = (Client *)clientId;

    // Be paranoid and validate user input
    for (iter = mPeripherals.begin(); iter != mPeripherals.end(); iter++) {
        peripheral = *iter;
        found = peripheral->findClient(client);
        if (found)
            break;
    }

    if (!found) {
        ALOGE("Connect: No such client %p", (void *)clientId);
        return NAME_NOT_FOUND;
    }

    ALOGD("%s voting for %s", client->getName(),
           peripheral->getName().string());

    return peripheral->increaseVoters(client);
}

status_t PeripheralManagerServer::disconnect(int64_t clientId) {
    list<Peripheral *>::iterator iter;
    Peripheral *peripheral;
    bool found = false;
    Client *client;

    client = (Client *)clientId;

    // Be paranoid and validate user input
    for (iter = mPeripherals.begin(); iter != mPeripherals.end(); iter++) {
        peripheral = *iter;
        found = peripheral->findClient(client);
        if (found)
            break;
    }

    if (!found) {
        ALOGE("Disconnect: No such client %p", (void *)clientId);
        return NAME_NOT_FOUND;
    }

    ALOGD("%s removing vote for %s", client->getName(),
          peripheral->getName().string());

    return peripheral->decreaseVoters(client, false);
}

status_t PeripheralManagerServer::acknowledge(int64_t clientId, int32_t event) {
    list<Peripheral *>::iterator iter;
    Peripheral *peripheral;
    bool found = false;
    Client *client;

    client = (Client *)clientId;

    for (iter = mPeripherals.begin(); iter != mPeripherals.end(); iter++) {
        peripheral = *iter;
        found = peripheral->findClient(client);
        if (found)
            break;
    }

    if (!found) {
        ALOGE("Acknowledge: No such client %p", (void *)clientId);
        return NAME_NOT_FOUND;
    }

    return peripheral->notifyClientAck(client, (enum pm_event)event);
}

status_t PeripheralManagerServer::shutdown(void) {
    int ret, uid, pid;

    uid = getUID();
    pid = getPID();

    if (uid != AID_ROOT && uid != AID_SYSTEM) {
        ALOGE("Shutdown denied for PID %d", pid);
        return PERMISSION_DENIED;
    }

    ALOGI("Shutdown from PID %d UID %d", pid, uid);

    // Send exit to main thread
    ret = kill(getpid(), SIGINT);

    return (!ret) ? OK : BAD_VALUE;
}

void PeripheralManagerServer::clientDied(Client *client) {
    list<Peripheral *>::iterator iter;
    Peripheral *peripheral;
    bool found = false;

    for (iter = mPeripherals.begin(); iter != mPeripherals.end(); iter++) {
        peripheral = *iter;
        found = peripheral->findClient(client);
        if (found)
            break;
    }

    if (!found) {
        ALOGE("Client died: No such client %p", client);
        return;
    }

    ALOGE("%s registered for %s has died", client->getName(),
          peripheral->getName().string());

    // Ensure that client did not hold Device powered up
    peripheral->decreaseVoters(client, true);
    peripheral->removeClient(client);
    delete client;
}

status_t PeripheralManagerServer::initPeripherals(int offTimeout, int ackTimeout,
                                               bool debugMode) {
    struct dev_info modems;
    Peripheral *peripheral;
    int identifier;

    if (get_system_info(&modems) != RET_SUCCESS) {
        ALOGE("Could not retrieve peripheral devices info");
        return NAME_NOT_FOUND;
    }

    if (modems.num_modems == 0) {
        ALOGE("Zero peripherals found");
        return NAME_NOT_FOUND;
    }

    for (int i = 0; i < modems.num_modems; i++) {
        // Hardcode this until we can have more robust mapping
        if (MDM_TYPE_EXTERNAL == modems.mdm_list[i].type) {
            identifier = SSREQ_QMI_CLIENT_INSTANCE_MDM_V01;
        } else {
            identifier = SSREQ_QMI_CLIENT_INSTANCE_MPSS_V01;
        }

        peripheral = new Peripheral(modems.mdm_list[i].mdm_name,
                                    modems.mdm_list[i].powerup_node,
                                    identifier, offTimeout, ackTimeout,
                                    debugMode);
        if (!peripheral) {
            ALOGE("Can not allocate peripheral for %s",
                  modems.mdm_list[i].mdm_name);
            continue;
        }

        if (OK != peripheral->init()) {
            delete peripheral;
        } else {
            // Add specific access permissions for each peripheral
            peripheral->addPermissions(qmiClientsInfo[identifier].aid_cnt,
                                       qmiClientsInfo[identifier].aid);
            mPeripherals.push_back(peripheral);
        }
    }

    return OK;
}

status_t PeripheralManagerServer::deInitPeripherals() {
    list<Peripheral *>::iterator iter;
    Peripheral *peripheral;

    for (iter = mPeripherals.begin(); iter != mPeripherals.end(); iter++) {
        peripheral = *iter;
        peripheral->shutdownStart();
    }

    for (iter = mPeripherals.begin(); iter != mPeripherals.end(); iter++) {
        peripheral = *iter;
        peripheral->shutdownWait();
        peripheral->deInit();
    }

    return OK;
}

void PeripheralManagerServer::dumpPeripheralInfo() {
    list<Peripheral *>::iterator iter;

    for (iter = mPeripherals.begin(); iter != mPeripherals.end(); iter++) {
        Peripheral *peripheral = *iter;
        peripheral->dumpStatistics();
    }
}

status_t PeripheralManagerServer::restartPeripheral(int identifier,
                                                 qmi_client_handle qmiClient) {
    Peripheral *peripheral;
    bool started;

    peripheral = findBySubSystem(identifier);
    if (peripheral == NULL) {
        ALOGE("No such peripheral SSID %d, QMI client %p", identifier, qmiClient);
        return BAD_VALUE;
    }

    // Check if peripheral has a QMI client subscribed for indications.
    // If it has such, we are processing another request from the same
    // identifier, so return an error.
    started = peripheral->setRestartRequested((void *)qmiClient);
    return started ? OK : BAD_VALUE;
}

// Private functions
Peripheral *PeripheralManagerServer::findByName(const String8 &devName) {
    list<Peripheral *>::iterator iter;

    for (iter = mPeripherals.begin(); iter != mPeripherals.end(); iter++) {
        Peripheral *peripheral = *iter;

        if (peripheral->getName() == devName) {
            return peripheral;
        }
    }

    return NULL;
}

Peripheral *PeripheralManagerServer::findBySubSystem(int identifier) {
    list<Peripheral *>::iterator iter;

    for (iter = mPeripherals.begin(); iter != mPeripherals.end(); iter++) {
        Peripheral *peripheral = *iter;

        if (peripheral->getSubSytemId() == identifier) {
            return peripheral;
        }
    }

    return NULL;
}

}; // namespace android

using namespace android;

const char *qmiServerClientName(ssreq_qmi_ss_identifier_enum_type_v01 id) {

    if (id < SSREQ_QMI_CLIENT_INSTANCE_APSS_V01 ||
        id > SSREQ_QMI_CLIENT_INSTANCE_QSC_V01) {
        return "unknown";
    }

    return qmiClientsInfo[id].name;
}

// QXDM test command: send_data 75 37 03 00 00
qmi_csi_cb_error qmiServerShutdown(qmi_client_handle qmiClient,
                                   qmi_req_handle reqHandle,
                                   unsigned int msgId, void *reqStruct,
                                   PeripheralManagerServer *pmService) {
    qmi_ssreq_system_shutdown_resp_msg_v01 response;
    qmi_ssreq_system_shutdown_req_msg_v01 *request;
    ssreq_qmi_ss_identifier_enum_type_v01 identifier;
    qmi_ssreq_system_shutdown_ind_msg_v01 ind;
    qmi_csi_cb_error cbErr = QMI_CSI_CB_NO_ERR;
    qmi_csi_error csiErr;

    // Not used for now
    pmService = pmService;

    request = (qmi_ssreq_system_shutdown_req_msg_v01 *) reqStruct;
    identifier = request->ss_client_id;

    ALOGD("QMI service system shutdown request from %s",
          qmiServerClientName(identifier));

    memset(&ind, 0, sizeof(ind));
    memset(&response, 0, sizeof(response));

    response.resp.result = QMI_RESULT_SUCCESS_V01;
    response.resp.error = QMI_ERR_NONE_V01;

    csiErr = qmi_csi_send_resp(reqHandle, msgId, &response, sizeof(response));
    if (csiErr != QMI_CSI_NO_ERR) {
        ALOGE("QMI service system shutdown response error %d", csiErr);
        cbErr = QMI_CSI_CB_INTERNAL_ERR;
    }

    // Fake: action done.
    msgId = QMI_SSREQ_SYSTEM_SHUTDOWN_IND_V01;
    ind.status = SSREQ_QMI_REQUEST_SERVICED_V01;

    csiErr = qmi_csi_send_ind(qmiClient, msgId, &ind, sizeof(ind));

    if (csiErr) {
        ALOGE("QMI service system shutdown indicatation error %d", csiErr);
    }

    if (reboot(LINUX_REBOOT_CMD_POWER_OFF)) {
        ALOGE("QMI service system shutdown fail %s", strerror(errno));
    }

    return cbErr;
}

// QXDM test command: send_data 75 37 03 00 01
qmi_csi_cb_error qmiServerRestart(qmi_client_handle qmiClient,
                                 qmi_req_handle reqHandle,
                                 unsigned int msgId, void *reqStruct,
                                 PeripheralManagerServer *pmService) {
    ssreq_qmi_ss_identifier_enum_type_v01 identifier;
    qmi_ssreq_system_restart_resp_msg_v01 response;
    qmi_ssreq_system_restart_req_msg_v01 *request;
    qmi_ssreq_system_restart_ind_msg_v01 ind;
    qmi_csi_cb_error cbErr = QMI_CSI_CB_NO_ERR;
    qmi_csi_error csiErr;

    // Not used for now
    pmService = pmService;

    request = (qmi_ssreq_system_restart_req_msg_v01 *) reqStruct;
    identifier = request->ss_client_id;

    ALOGD("QMI service system restart request from %s",
          qmiServerClientName(identifier));

    memset(&ind, 0, sizeof(ind));
    memset(&response, 0, sizeof(response));

    response.resp.result = QMI_RESULT_SUCCESS_V01;
    response.resp.error = QMI_ERR_NONE_V01;

    csiErr = qmi_csi_send_resp(reqHandle, msgId, &response, sizeof(response));
    if (csiErr != QMI_CSI_NO_ERR) {
        ALOGE("QMI service system restart response error %d", csiErr);
        cbErr = QMI_CSI_CB_INTERNAL_ERR;
    }

    // Fake: action done.
    msgId = QMI_SSREQ_SYSTEM_RESTART_IND_V01;
    ind.status = SSREQ_QMI_REQUEST_SERVICED_V01;

    csiErr = qmi_csi_send_ind(qmiClient, msgId, &ind, sizeof(ind));

    if (csiErr) {
        ALOGE("QMI service system restart indicatation error %d", csiErr);
    }

    if (reboot(LINUX_REBOOT_CMD_RESTART)) {
        ALOGE("QMI service system restart fail %s", strerror(errno));
    }

    return cbErr;
}

// QXDM test command: send_data 75 37 03 00 02
qmi_csi_cb_error qmiServerPeripheralRestart(qmi_client_handle qmiClient,
                                         qmi_req_handle reqHandle,
                                         unsigned int msgId, void *reqStruct,
                                         PeripheralManagerServer *pmService) {
    qmi_ssreq_peripheral_restart_resp_msg_v01 response;
    qmi_ssreq_peripheral_restart_req_msg_v01 *request;
    ssreq_qmi_ss_identifier_enum_type_v01 identifier;
    qmi_csi_cb_error cbErr = QMI_CSI_CB_NO_ERR;
    qmi_csi_error csiErr;
    status_t status;

    request = (qmi_ssreq_peripheral_restart_req_msg_v01 *) reqStruct;
    identifier = request->ss_client_id;

    ALOGD("QMI service peripheral restart request from %s",
          qmiServerClientName(identifier));

    status = pmService->restartPeripheral(identifier, qmiClient);
    if (OK != status) {
        cbErr = QMI_CSI_CB_INTERNAL_ERR;
    }

    memset(&response, 0, sizeof(response));

    if (cbErr != QMI_CSI_CB_NO_ERR) {
        response.resp.result = QMI_RESULT_FAILURE_V01;
        response.resp.error = QMI_ERR_GENERAL_V01;
    } else {
        response.resp.result = QMI_RESULT_SUCCESS_V01;
        response.resp.error = QMI_ERR_NONE_V01;
    }

    csiErr = qmi_csi_send_resp(reqHandle, msgId, &response, sizeof(response));
    if (csiErr != QMI_CSI_NO_ERR) {
        ALOGE("QMI service peripheral restart response error %d", csiErr);
        cbErr = QMI_CSI_CB_INTERNAL_ERR;
    }

    return cbErr;
}

qmi_csi_cb_error qmiServerConnect(qmi_client_handle qmiClient, void *pmService,
                                  void **qmiConnection) {

    if (!qmiConnection || !pmService) {
        return QMI_CSI_CB_INTERNAL_ERR;
    }

    // In other callbacks we do not receive clientHandle, but we need it...
    // Furthermore we receive a qmiClient, which is not needed
    // anywhere. So just use qmiClient as clientHandle.
    *qmiConnection = qmiClient;
    ALOGI("QMI client %p connected", qmiClient);
    return QMI_CSI_CB_NO_ERR;
}

void qmiServerDisconnect(void *qmiClient, void *pmService) {
    pmService = pmService;
    qmiClient = qmiClient;
    ALOGI("QMI client %p disconnected", qmiClient);
}

qmi_csi_cb_error qmiServerProcessRequest(void *__qmiClient,
                                        qmi_req_handle reqHandle,
                                        unsigned int msgId, void *reqStruct,
                                        unsigned int reqStructLen,
                                        void *__pmService) {
    qmi_csi_cb_error cbErr = QMI_CSI_CB_NO_ERR;
    PeripheralManagerServer *pmService;
    qmi_client_handle qmiClient;

    reqStructLen = reqStructLen;
    pmService = (PeripheralManagerServer *)__pmService;
    qmiClient = (qmi_client_handle) __qmiClient;

    switch (msgId) {
    case QMI_SSREQ_SYSTEM_SHUTDOWN_REQ_V01:
        cbErr = qmiServerShutdown(qmiClient, reqHandle, msgId, reqStruct,
                                  pmService);
        break;
    case QMI_SSREQ_SYSTEM_RESTART_REQ_V01:
        cbErr = qmiServerRestart(qmiClient, reqHandle, msgId, reqStruct,
                                 pmService);
        break;

    case QMI_SSREQ_PERIPHERAL_RESTART_REQ_V01:
        cbErr = qmiServerPeripheralRestart(qmiClient, reqHandle, msgId,
                                           reqStruct, pmService);
        break;

    default:
        ALOGE("QMI service message %x not supported, client %p", msgId, qmiClient);
        cbErr = QMI_CSI_CB_INTERNAL_ERR;
    }

    return cbErr;
}


void *qmiServerThread(void *data) {
    struct QmiThreadInfo *info;
    PeripheralManagerServer *pmService;
    qmi_idl_service_object_type serviceObject;
    qmi_csi_os_params osParams, osReady;
    qmi_csi_service_handle qmiService;
    qmi_csi_options options;
    qmi_csi_error error;
    struct timeval time;
    int ret, quitFd, maxFd;

    info = (struct QmiThreadInfo *)data;

    pmService = info->pmService;
    quitFd = info->quitFd;

    serviceObject = ssreq_get_service_object_v01();

    FD_ZERO(&osReady.fds);

    memset(&osParams, 0, sizeof(osParams));

    time.tv_sec = 0;
    time.tv_usec = QMI_START_RETRY_INTERVAL * 1000;

    QMI_CSI_OPTIONS_INIT(options);
    QMI_CSI_OPTIONS_SET_INSTANCE_ID(options, SSREQ_QMI_SERVICE_INSTANCE_APSS_V01);

    do {
        // Init QMI server
        error = qmi_csi_register_with_options(serviceObject, qmiServerConnect,
                                              qmiServerDisconnect,
                                              qmiServerProcessRequest,
                                              pmService, &osParams, &options,
                                              &qmiService);
        if (error == QMI_CSI_NO_ERR) {
            break;
        }

        // Wait for signal form main thread or timeout
        FD_SET(quitFd, &osReady.fds);

        if (0 < select(quitFd, &osReady.fds, NULL, NULL, &time)) {
            ALOGE("QMI service canceled");
            return NULL;
        }

    } while (1);

    // Get larger number file desctiptor between main thread
    // quit FD and value returned from CSI register call
    maxFd = osParams.max_fd;
    if (quitFd > maxFd)
        maxFd = quitFd;

    ALOGI("QMI service start");

    do {

        osReady = osParams;

        // Wait also for signal form main thread
        FD_SET(quitFd, &osReady.fds);

        // Wait for event from QMI clients
        ret = select(maxFd + 1, &osReady.fds, NULL, NULL, NULL);
        switch (ret) {
            case -1:
                ALOGW("QMI service select error: %s", strerror(errno));
                continue;
            case 0:
                // Timeout, should not happen, but...
                ALOGI("QMI service select timeout");
                continue;
            default:
                break;
        }

        if (FD_ISSET(quitFd, &osReady.fds)) {
            break;
        }

        // Pass ready events to handler.
        error = qmi_csi_handle_event(qmiService, &osReady);
        if (error != QMI_CSI_NO_ERR) {
            ALOGE("QMI service process event error %d", error);
        }

    } while (1);

    // Clean and exit.
    qmi_csi_unregister(qmiService);
    ALOGI("QMI service exited");
    return NULL;
}


int main(int argc, char **argv) {
    struct QmiThreadInfo info;
    PeripheralManagerServer pmService;
    sp<IServiceManager> sm;
    pthread_t qmiInitTaskId;
    sigset_t waitset;
    status_t status;
    int ret, op, idx, sig, qmiPipe[2];
    long offTimeout, ackTimeout, debugMode;
    uid_t uid;

    static struct option options[] = {
        {"help",    no_argument,         NULL, 'h'},
        {"off",     required_argument,   NULL, 'o'},
        {"ack",     required_argument,   NULL, 'a'},
        {"debug",   no_argument,         NULL, 'd'},
        {0, 0, 0, 0}
    };

    uid = getuid();

    if (uid != AID_ROOT && uid != AID_SYSTEM && uid != AID_RADIO) {
        ALOGE("Insufficient privileges, UID %d", uid);
        return PERMISSION_DENIED;
    }

    offTimeout = APP_CLIENTS_OFF_DELAY;
    ackTimeout = APP_CLIENTS_ACK_DELAY;
    debugMode  = false;

    while (1) {

        op = getopt_long(argc, argv, "?ho:a:d", options, &idx);

        /* Detect the end of the options. */
        if (op == -1)
            break;

        switch (op) {
        case 'o':
            offTimeout = atol(optarg);
            break;
        case 'a':
            ackTimeout = atol(optarg);
            break;
        case 'd':
            debugMode = true;
            break;
        case 'h':
        case '?':
        default:
            ALOGE("Usage: %s [--off|-o <ms>] [--ack|-a <ms>]\n"
                  "\t--off - timeout before device power off, default %ld,ms\n"
                  "\t--ack - timeout before transition to new state, default %ld,ms\n",
                  argv[0], offTimeout, ackTimeout);
            return -1;
        }
    }

    if (offTimeout < 0 || ackTimeout < 0) {
        ALOGE("Invalid --off %ld,ms or --ack %ld,ms", offTimeout, ackTimeout);
        return -1;
    }

    ret = pipe(qmiPipe);
    if (ret < 0) {
        ALOGE("Can't create QMI pipe: %s", strerror(errno));
        return ret;
    }

    status = pmService.initPeripherals(offTimeout, ackTimeout, debugMode);
    if (status != OK) {
        close(qmiPipe[0]);
        close(qmiPipe[1]);
        return -1;
    }

    sm = defaultServiceManager();
    // Register our service
    status = sm->addService(String16("vendor.qcom.PeripheralManager"), &pmService);
    if (status != OK) {
        ALOGE("Adding Peripheral Manager service fail");
        goto exit;
    }

    info.pmService = &pmService;
    info.quitFd = qmiPipe[0];

    // Register QMI server in other thread. It might block for a while
    ret = pthread_create(&qmiInitTaskId, NULL, qmiServerThread, &info);
    if (ret < 0) {
        ALOGE("Can't create QMI thread: %s", strerror(errno));
        goto exit;
    }

    sigfillset(&waitset);
    sigprocmask(SIG_BLOCK, &waitset, NULL);

    ALOGI("Peripheral Mananager service start");
    ALOGI("power-off timeout %ld,ms; event-ack timeout %ld,ms; debug-mode %s",
           offTimeout, ackTimeout, debugMode ? "true" : "false");
    // Just for the record
    pmService.dumpPeripheralInfo();

    // Allow binder to use our thread when calls us
    ProcessState::self()->startThreadPool();

    do {
        if (sigwait(&waitset, &sig)) {
            ALOGD("Signal read error: %s", strerror(errno));
            continue;
        }

        // Android SIGUSR1 is 10 *%$#@!
        if (sig == SIGUSR1) {
            pmService.dumpPeripheralInfo();
            continue;
        }

        // Exit signal?
        if (sig == SIGINT) {
            if (0 < write(qmiPipe[1], &sig, sizeof(sig))) {
                break;
            }

            ALOGW("Send QUIT to QMI service fail: %s", strerror(errno));
        }
    } while (1);

    pthread_join(qmiInitTaskId, NULL);
exit:
    close(qmiPipe[0]);
    close(qmiPipe[1]);
    pmService.deInitPeripherals();
    ALOGI("Peripheral Mananager service exited");
    return 0;
}
