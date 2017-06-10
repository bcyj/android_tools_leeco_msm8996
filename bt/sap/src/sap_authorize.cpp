/******************************************************************************NS

  @file    sap_authorize.cpp
  @brief

  DESCRIPTION

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/
#include <qcc/platform.h>
#include <qcc/String.h>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/ProxyBusObject.h>
#include <alljoyn/DBusStd.h>
#include <alljoyn/AllJoynStd.h>
#include <Status.h>

#include <assert.h>

#include <cutils/properties.h>
#include <utils/Log.h>

#include "sap_authorize.h"
#include "sap_constants.h"

#define SAP_UUID        "0000112D-0000-1000-8000-00805F9B34FB"
#define PATH_LENGTH_MAX 255

#undef  LOGE
#undef  LOGV
#define LOGE ALOGE
#define LOGV ALOGV

/*constants*/
static const char* BLUEZ_DEFAULT_BUS       = "org.bluez";
static const char* MGR_INTERFACE_NAME      = "org.bluez.Manager";
static const char* AGENT_INTERFACE_NAME    = "org.bluez.Agent";
static const char* ADAPTER_INTERFACE_NAME  = "org.bluez.Adapter";

static const char agent_path[]             = "/android/bluetooth/agent";

static const char* DEF_ADAP_SERVICE_PATH   = "DefaultAdapter";
static const char* AUTH_SERVICE_PATH       = "Authorize";
static const char* FIND_DEV_SERVICE_PATH   = "FindDevice";
static const char* SAP_STATUS_CHANGE       = "SapStateChanged";
static const char* SAP_DISCONNECT          = "DisConnect";
static const char* QCOM_SAP_BUS            = "org.qcom.sap";

char remote_device_path[PATH_LENGTH_MAX];

using namespace std;
using namespace qcc;
using namespace ajn;
class SapConnStatusObj;

/** Static top level message bus object */
static BusAttachment* g_msgBus = NULL;
static ProxyBusObject g_adapterObj, g_agentObj, g_managerObj;
InterfaceDescription *intf = NULL;
SapConnStatusObj *gSapConnStatusObj = NULL;

char* get_adapter_path(char *path)
{
        Message reply(*g_msgBus);
        QStatus status = ER_OK;

        status = g_managerObj.MethodCall(MGR_INTERFACE_NAME, DEF_ADAP_SERVICE_PATH, NULL, 0, reply, 5000);
        if (ER_OK == status) {
                strlcpy(path, reply->GetArg(0)->v_string.str, (PATH_LENGTH_MAX-1));
                LOGE("%s.%s ( path=%s) returned \"%s\"\n", MGR_INTERFACE_NAME, DEF_ADAP_SERVICE_PATH,
                           DEF_ADAP_SERVICE_PATH, reply->GetArg(0)->v_string.str);
        } else {
            LOGE("MethodCall on %s.%s failed", MGR_INTERFACE_NAME, DEF_ADAP_SERVICE_PATH);
        }

        return path;
}

class SapConnStatusObj:  public BusObject
{
  public:
        SapConnStatusObj(BusAttachment& bus, const char* path, int sap_session_handle) :
                BusObject(bus, path)
        {
                InterfaceDescription* intf = NULL;
                QStatus status = bus.CreateInterface(QCOM_SAP_BUS, intf);
                stateChangedMember = NULL;
                if (status == ER_OK) {
                        intf->AddMethod(SAP_DISCONNECT, NULL, NULL, NULL, 0);
                        intf->AddSignal(SAP_STATUS_CHANGE, "os",  "sapState", 0);
                        intf->Activate();

                        status = AddInterface(*intf);
                        if (status == ER_OK) {
                                /* Register the signal handler 'SapStateChanged' with the bus*/
                                stateChangedMember = intf->GetMember(SAP_STATUS_CHANGE);
                                assert(stateChangedMember);
                        } else {
                                LOGE("%s: Failed to Add interface: %s", __func__, AGENT_INTERFACE_NAME);
                        }

                        /** Register the method handlers with the object */
                        const MethodEntry methodEntries[] = {
                                { intf->GetMember(SAP_DISCONNECT),
                                static_cast<MessageReceiver::MethodHandler>(&SapConnStatusObj::DisConnect) }
                        };
                        status = AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
                        if (ER_OK != status) {
                                LOGE("%s: Failed to register method handlers for BasicSampleObject", __func__);
                        }
                } else {
                        LOGE("%s:Failed to create interface %s\n", __func__, QCOM_SAP_BUS);
                }
                mSapSessionHandle = sap_session_handle;
        }

        void DisConnect(const InterfaceDescription::Member* member, Message& msg) {

                QStatus err = ER_OK;
                int ret = 0;
                LOGV("-> %s ", __func__);
                ret = disconnect_sap(mSapSessionHandle);
                if (ret < 0) {
                        LOGE("Error from disconnect_sap\n");
                        err = ER_FAIL;
                }

                QStatus status = MethodReply(msg, err);
                if (ER_OK != status) {
                        LOGE("%s: Error sending reply\n", __func__);
                }
                return;
        }

        QStatus EmitNameChangedSignal(qcc::String sapState)
        {
                assert(stateChangedMember);
                MsgArg inputs[2];
                inputs[0].Set("o", remote_device_path);
                inputs[1].Set("s", sapState.c_str());

                uint8_t flags = ALLJOYN_FLAG_GLOBAL_BROADCAST;
                QStatus status = Signal(NULL, 0, *stateChangedMember, inputs, 2, 0, flags);
                LOGV("Emitted signal %s: status %d\n", sapState.c_str(), (int)status);
                return status;
        }
        private:
                const InterfaceDescription::Member* stateChangedMember;
                int mSapSessionHandle;
};

int updateStateChanged(const char* state)
{
        QStatus status = ER_FAIL;
        if (gSapConnStatusObj != NULL) {
                status = gSapConnStatusObj->EmitNameChangedSignal(state);
        }

        return (int)status;
}

int init_alljoyn(int sap_session_handle)
{
        QStatus status = ER_OK;
        /* Create message bus */
        g_msgBus = new BusAttachment("bt_sap", true);

        if (g_msgBus == NULL) {
                LOGE("%s: failed to instantiate BusAttachment\n", __func__);
                return -1;
        }

        InterfaceDescription* testIntf = NULL;
        status = g_msgBus->CreateInterface(MGR_INTERFACE_NAME, testIntf);
        if (status == ER_OK) {
                LOGV("Manager Interface Created.\n");
                testIntf->AddMethod(DEF_ADAP_SERVICE_PATH, NULL,  "o", NULL, 0);
                testIntf->Activate();
        } else {
                LOGE("%s: Failed to create manager interface\n", __func__);
                return -1;
        }

        status = g_msgBus->CreateInterface(AGENT_INTERFACE_NAME, intf);
        if (status == ER_OK) {
                LOGV("Agent Interface Created.\n");
                intf->AddMethod(AUTH_SERVICE_PATH, "os",  NULL, NULL, 0);
                intf->Activate();
        } else {
                LOGE("%s: Failed to create agent interface\n", __func__);
                return -1;
        }

        status = g_msgBus->CreateInterface(ADAPTER_INTERFACE_NAME, testIntf);
        if (status == ER_OK) {
                LOGV("Adapter Interface Created.\n");
                testIntf->AddMethod(FIND_DEV_SERVICE_PATH, "s", "o", NULL, 0);
                testIntf->Activate();
        } else {
                LOGE("%s: Failed to create Adapter interface\n", __func__);
                return -1;
        }

        if (gSapConnStatusObj == NULL) {
                gSapConnStatusObj = new SapConnStatusObj(*g_msgBus, "/SapService", sap_session_handle);
        }

        /* Start the msg bus */
        if (ER_OK == status) {
                status = g_msgBus->Start();
                if (ER_OK != status) {
                        LOGE("BusAttachment::Start failed\n");
                        return -1;
                } else {
                        LOGV("BusAttachment started.\n");
                        g_msgBus->RegisterBusObject(*gSapConnStatusObj);
                        LOGV("BusObject registered\n");
                }
        }

        if (ER_OK == status) {
                status = g_msgBus->Connect("unix:path=/dev/socket/dbus");
                if (ER_OK != status) {
                        LOGE("%s: BusAttachment::Connect(\"%s\") failed\n", __func__, "unix:path=/dev/socket/dbus");
                        return -1;
                } else {

                        LOGV("BusAttchement connected to %s\n", "unix:path=/dev/socket/dbus");
                         /* Request name */
                        uint32_t flags = DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE;
                        status = g_msgBus->RequestName(QCOM_SAP_BUS, flags);
                        if (ER_OK != status) {
                                LOGE("%s: RequestName(%s) failed (status=%s)\n", __func__, QCOM_SAP_BUS, QCC_StatusText(status));
                                return -1;
                        }
                        LOGV("Bus name %s registered ::%s\n", QCOM_SAP_BUS, QCC_StatusText(status));
                }
        }

        if (ER_OK == status) {
                g_managerObj = ProxyBusObject(*g_msgBus, BLUEZ_DEFAULT_BUS, "/", 0);
                const InterfaceDescription* alljoynTestIntf = g_msgBus->GetInterface(MGR_INTERFACE_NAME);
                assert(alljoynTestIntf);
                g_managerObj.AddInterface(*alljoynTestIntf);
        }

        char adapter_path[PATH_LENGTH_MAX];
        get_adapter_path(adapter_path);
        if (ER_OK == status) {
                g_adapterObj = ProxyBusObject(*g_msgBus, BLUEZ_DEFAULT_BUS, adapter_path, 0);
                const InterfaceDescription* alljoynTestIntf = g_msgBus->GetInterface(ADAPTER_INTERFACE_NAME);
                assert(alljoynTestIntf);
                g_adapterObj.AddInterface(*alljoynTestIntf);
                LOGV("%s: adapter object interface set\n", __func__);
        }

        char value[PROPERTY_VALUE_MAX];
        property_get("bluetooth.eventloop.dbus", value, "");

        if (!strcmp(value, "")) {
                LOGE("Sender dbus object is NULL");
                return 0;
        }

        LOGV("Sender obj is %s", value);

        if (ER_OK == status) {
                /*Replace BUS_NAME with property value*/
                g_agentObj = ProxyBusObject(*g_msgBus, value, agent_path, 0);
                const InterfaceDescription* alljoynTestIntf = g_msgBus->GetInterface(AGENT_INTERFACE_NAME);
                assert(alljoynTestIntf);
                g_agentObj.AddInterface(*alljoynTestIntf);
                LOGV("%s: agent object interface set\n", __func__);
        }

        return (int) status;
}

const char* find_device(const char *address, char *dev_path)
{
        MsgArg inputs[1];
        QStatus status = ER_OK;

        LOGV("-> %s\n", __func__);
        Message reply(*g_msgBus);
        inputs[0].Set("s", address);
        status = g_adapterObj.MethodCall(ADAPTER_INTERFACE_NAME, FIND_DEV_SERVICE_PATH, inputs, 1, reply, 5000);
        if (ER_OK == status) {
                LOGE("%s.%s ( path=%s) returned \"%s\"\n", ADAPTER_INTERFACE_NAME, FIND_DEV_SERVICE_PATH,
                           FIND_DEV_SERVICE_PATH, reply->GetArg(0)->v_string.str);
                strlcpy(dev_path, reply->GetArg(0)->v_string.str, (PATH_LENGTH_MAX-1));
        } else {
            LOGE("MethodCall on %s.%s failed: Status: %d\n", ADAPTER_INTERFACE_NAME, FIND_DEV_SERVICE_PATH, (int)status);
        }
        return dev_path;
}

void cleanup_alljoyn()
{
    if (gSapConnStatusObj != NULL) {
                delete gSapConnStatusObj;
                gSapConnStatusObj = NULL;
    }

    LOGV("destroyed the connection Status Object\n");
    /* Stop the bus (not strictly necessary since we are going to delete it anyways) */
    if (g_msgBus) {
        QStatus s = g_msgBus->Stop();
        if (ER_OK != s) {
            LOGE("BusAttachment::Stop failed\n");
        }
    }

   /* Dummy assignments for the proxy objects */
    g_adapterObj = ProxyBusObject();
    g_agentObj = ProxyBusObject();
    g_managerObj = ProxyBusObject();
    LOGV("Stopped the message bus\n");
    /* Deallocate bus */
    if (g_msgBus) {
        BusAttachment* deleteMe = g_msgBus;
        g_msgBus = NULL;
        delete deleteMe;
    }

    LOGV("<- %s\n", __func__);
}

int call_authorize(char *addr)
{
        Message reply(*g_msgBus);
        MsgArg inputs[2];
        int res = 0;
        QStatus status = ER_OK;

        char adapter_path[PATH_LENGTH_MAX];
        get_adapter_path(adapter_path);

        find_device(addr, remote_device_path);
        LOGV("remote_device_path: %s\n", remote_device_path);

        inputs[0].Set("o", remote_device_path);
        inputs[1].Set("s", SAP_UUID);
        LOGE("Calling Auth\n");
	/*Authorization is disabled due to some issues with aj for now,
	* Will be enabled back once it is fixed
        status = g_agentObj.MethodCall(AGENT_INTERFACE_NAME, AUTH_SERVICE_PATH, inputs, 2, reply, 50000);
	*/
	/*always authorize for now*/
	status = ER_OK;
        if (ER_OK == status) {
                LOGV("SAP authorized\n");
                res = 0;

        } else {
                LOGE("MethodCall on %s.%s failed: status %d\n", AGENT_INTERFACE_NAME, AUTH_SERVICE_PATH, (int)status);
                res = -1;
        }
        return res;
}
