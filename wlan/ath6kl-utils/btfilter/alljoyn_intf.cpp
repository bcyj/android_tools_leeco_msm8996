/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Proprietary and Confidential.
 */

#include <qcc/platform.h>
#include <qcc/String.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/ProxyBusObject.h>
#include <alljoyn/DBusStd.h>
#include <alljoyn/AllJoynStd.h>
#include <Status.h>
#include <assert.h>

#include <utils/Log.h>
#ifdef ANDROID
#include <cutils/log.h>
#endif
#include "alljoyn_dbus.h"

using namespace std;
using namespace qcc;
using namespace ajn;
class BtConnObj;
extern "C" {
	void HandleBTEvent(BT_STACK_EVENT bt_event, void *btfilt_info);
#ifdef HID_PROFILE_SUPPORT
	void HandleBTEvent1(BT_STACK_EVENT btevent, void *btfilt_info, A_BOOL val);
#endif
	void BT_LOG(const char *format, ...);
}

static BusAttachment *g_msgBus = NULL;
BtConnObj *btfilt_obj = NULL;

class BtConnObj:  public BusObject
{
	public:

	BtConnObj(BusAttachment& bus, const char* path, void *btfilt_info);

	void BTAdapterAdded(const InterfaceDescription::Member* member,
		const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(BT_ADAPTER_ADDED, mBtFiltInfo);
	}

	void BTAdapterRemoved(const InterfaceDescription::Member* member,
		const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(BT_ADAPTER_REMOVED, mBtFiltInfo);
	}

	void DiscoveryStarted(const InterfaceDescription::Member* member,
		const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(DEVICE_DISCOVERY_STARTED, mBtFiltInfo);
	}

	void DiscoveryCompleted(const InterfaceDescription::Member* member,
		const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(DEVICE_DISCOVERY_FINISHED, mBtFiltInfo);
	}

	void RemoteDeviceConnected(const InterfaceDescription::Member* member,
		const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(REMOTE_DEVICE_CONNECTED, mBtFiltInfo);
	}

	void RemoteDeviceDisconnected(const InterfaceDescription::Member* member,
		const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(REMOTE_DEVICE_DISCONNECTED, mBtFiltInfo);
	}

	void AudioDeviceCreated(const InterfaceDescription::Member* member,
		const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(AUDIO_DEVICE_ADDED, mBtFiltInfo);
	}

	void AudioDeviceRemoved(const InterfaceDescription::Member* member,
		const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(AUDIO_DEVICE_REMOVED, mBtFiltInfo);
	}

	void AudioHeadsetConnect(const InterfaceDescription::Member* member,
		const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(AUDIO_HEADSET_CONNECTED, mBtFiltInfo);
	}

	void AudioHeadsetDisConnect(const InterfaceDescription::Member* member,
		const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(AUDIO_HEADSET_DISCONNECTED, mBtFiltInfo);
	}

	void AudioHeadsetPlaying(const InterfaceDescription::Member* member, const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(AUDIO_HEADSET_STREAM_STARTED, mBtFiltInfo);
	}

	void AudioHeadsetStopped(const InterfaceDescription::Member* member, const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(AUDIO_HEADSET_STREAM_STOPPED, mBtFiltInfo);
	}

	void AudioSinkConnect(const InterfaceDescription::Member* member,
		const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(AUDIO_SINK_CONNECTED, mBtFiltInfo);
	}

	void AudioSinkDisConnect(const InterfaceDescription::Member* member,
		const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(AUDIO_SINK_DISCONNECTED, mBtFiltInfo);
	}

	void AudioSinkPlaying(const InterfaceDescription::Member* member, const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(AUDIO_SINK_STREAM_STARTED, mBtFiltInfo);
	}

	void AudioSinkStopped(const InterfaceDescription::Member* member, const char* srcPath, Message& msg)
	{
		BT_LOG("%s\n", __FUNCTION__);
		HandleBTEvent(AUDIO_SINK_STREAM_STOPPED, mBtFiltInfo);
	}

#ifdef HID_PROFILE_SUPPORT
	void InputDevicePropertyChanged(const InterfaceDescription::Member* member, const char* srcPath, Message& msg)
	{
		const char* property;
		const MsgArg* value;

		BT_LOG("%s\n", __FUNCTION__);

		msg->GetArgs("sv", &property, &value);
		if (strcmp(property, "Connected") == 0) {
			bool connected;

			value->Get("b", &connected);
			HandleBTEvent1(INPUT_DEVICE_PROPERTY_CHANGED, mBtFiltInfo, connected);
		}
	}
#endif

	private:
		const InterfaceDescription::Member* mBTEvents[BT_EVENTS_NUM_MAX];
		void *mBtFiltInfo;
};

BtConnObj::BtConnObj(BusAttachment& bus, const char* path, void *btfilt_info)
		 : BusObject(bus, path)
{
	InterfaceDescription* ManagerIntf = NULL;
	QStatus status = bus.CreateInterface(MANAGER_INTERFACE, ManagerIntf);
	if (status == ER_OK) {
		ManagerIntf->AddSignal("AdapterAdded", NULL,  NULL, 0);
		ManagerIntf->AddSignal("AdapterRemoved", NULL,  NULL, 0);
		ManagerIntf->Activate();

		status = AddInterface(*ManagerIntf);
		if (status == ER_OK) {
			/* Register the signal handler 'SapStateChanged' with the bus*/
			mBTEvents[BT_ADAPTER_ADDED] = ManagerIntf->GetMember("AdapterAdded");
			assert(mBTEvents[BT_ADAPTER_ADDED]);
			mBTEvents[BT_ADAPTER_REMOVED] = ManagerIntf->GetMember("AdapterRemoved");
			assert(mBTEvents[BT_ADAPTER_REMOVED]);
		}

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>(&BtConnObj::BTAdapterAdded),
				mBTEvents[BT_ADAPTER_ADDED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>(&BtConnObj::BTAdapterRemoved),
				mBTEvents[BT_ADAPTER_REMOVED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n",
					__FUNCTION__, __LINE__);

	} else {
		BT_LOG("%s:Failed to create interface %s\n", __func__, AUDIO_SINK_INTERFACE);
	}

	InterfaceDescription* AdapterIntf = NULL;
	status = bus.CreateInterface(ADAPTER_INTERFACE, AdapterIntf);
	if (status == ER_OK) {
		AdapterIntf->AddSignal("DiscoveryStarted", NULL,  NULL, 0);
		AdapterIntf->AddSignal("DiscoveryCompleted", NULL,  NULL, 0);
		AdapterIntf->AddSignal("RemoteDeviceConnected", NULL,  NULL, 0);
		AdapterIntf->AddSignal("RemoteDeviceDisconnected", NULL,  NULL, 0);
		AdapterIntf->Activate();

		status = AddInterface(*AdapterIntf);
		if (status == ER_OK) {
			/* Register the signal handler 'SapStateChanged' with the bus*/
			mBTEvents[DEVICE_DISCOVERY_STARTED] =
					AdapterIntf->GetMember("DiscoveryStarted");
			assert(mBTEvents[DEVICE_DISCOVERY_STARTED]);
			mBTEvents[DEVICE_DISCOVERY_FINISHED] =
					AdapterIntf->GetMember("DiscoveryCompleted");
			assert(mBTEvents[DEVICE_DISCOVERY_STARTED]);
			mBTEvents[REMOTE_DEVICE_CONNECTED] =
					AdapterIntf->GetMember("RemoteDeviceConnected");
			assert(mBTEvents[REMOTE_DEVICE_CONNECTED]);
			mBTEvents[REMOTE_DEVICE_DISCONNECTED] =
					AdapterIntf->GetMember("RemoteDeviceDisconnected");
			assert(mBTEvents[REMOTE_DEVICE_DISCONNECTED]);
		}

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>(&BtConnObj::DiscoveryStarted),
				mBTEvents[DEVICE_DISCOVERY_STARTED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>
						(&BtConnObj::DiscoveryCompleted),
				mBTEvents[DEVICE_DISCOVERY_FINISHED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>
						(&BtConnObj::RemoteDeviceConnected),
				mBTEvents[REMOTE_DEVICE_CONNECTED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>
						(&BtConnObj::RemoteDeviceDisconnected),
				mBTEvents[REMOTE_DEVICE_DISCONNECTED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);

	} else {
		BT_LOG("%s:Failed to create interface %s\n", __func__, ADAPTER_INTERFACE);
	}

	InterfaceDescription* AudioManagerIntf = NULL;
	status = bus.CreateInterface(AUDIO_MANAGER_INTERFACE, AudioManagerIntf);
	if (status == ER_OK) {
		AudioManagerIntf->AddSignal("DeviceCreated", NULL,  NULL, 0);
		AudioManagerIntf->AddSignal("DeviceRemoved", NULL,  NULL, 0);
		AudioManagerIntf->Activate();

		status = AddInterface(*AudioManagerIntf);
		if (status == ER_OK) {
			/* Register the signal handler 'SapStateChanged' with the bus*/
			mBTEvents[AUDIO_DEVICE_ADDED] =
					AudioManagerIntf->GetMember("DeviceCreated");
			assert(mBTEvents[AUDIO_DEVICE_ADDED]);
			mBTEvents[AUDIO_DEVICE_REMOVED] =
					AudioManagerIntf->GetMember("DeviceRemoved");
			assert(mBTEvents[AUDIO_DEVICE_REMOVED]);
		}

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>
					(&BtConnObj::AudioDeviceCreated),
				mBTEvents[AUDIO_DEVICE_ADDED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>
					(&BtConnObj::AudioDeviceRemoved),
				mBTEvents[AUDIO_DEVICE_REMOVED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n",
					__FUNCTION__, __LINE__);

	} else {
		BT_LOG("%s:Failed to create interface %s\n",
			__func__, AUDIO_MANAGER_INTERFACE);
	}

	InterfaceDescription* AudioHeadsetIntf = NULL;
	status = bus.CreateInterface(AUDIO_HEADSET_INTERFACE, AudioHeadsetIntf);
	if (status == ER_OK) {
		AudioHeadsetIntf->AddSignal("Connected", NULL,  NULL, 0);
		AudioHeadsetIntf->AddSignal("Disconnected", NULL,  NULL, 0);
		AudioHeadsetIntf->AddSignal("Playing", NULL,  NULL, 0);
		AudioHeadsetIntf->AddSignal("Stopped", NULL,  NULL, 0);
		AudioHeadsetIntf->Activate();

		status = AddInterface(*AudioHeadsetIntf);
		if (status == ER_OK) {
			/* Register the signal handler 'SapStateChanged' with the bus*/
			mBTEvents[AUDIO_HEADSET_CONNECTED] =
				AudioHeadsetIntf->GetMember("Connected");
			assert(mBTEvents[AUDIO_HEADSET_CONNECTED]);
			mBTEvents[AUDIO_HEADSET_DISCONNECTED] =
				AudioHeadsetIntf->GetMember("Disconnected");
			assert(mBTEvents[AUDIO_HEADSET_DISCONNECTED]);
			mBTEvents[AUDIO_HEADSET_STREAM_STARTED] =
				AudioHeadsetIntf->GetMember("Playing");
			assert(mBTEvents[AUDIO_HEADSET_STREAM_STARTED]);
			mBTEvents[AUDIO_HEADSET_STREAM_STOPPED] =
				AudioHeadsetIntf->GetMember("Stopped");
			assert(mBTEvents[AUDIO_HEADSET_STREAM_STOPPED]);
		}

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>
					(&BtConnObj::AudioHeadsetConnect),
				mBTEvents[AUDIO_HEADSET_CONNECTED],
				NULL);
		if (status != ER_OK) {
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);
		}

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>
					(&BtConnObj::AudioHeadsetDisConnect),
				mBTEvents[AUDIO_HEADSET_DISCONNECTED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>
					(&BtConnObj::AudioHeadsetPlaying),
				mBTEvents[AUDIO_HEADSET_STREAM_STARTED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>
					(&BtConnObj::AudioHeadsetStopped),
				mBTEvents[AUDIO_HEADSET_STREAM_STOPPED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);
	} else {
		BT_LOG("%s:Failed to create interface %s\n", __func__, AUDIO_HEADSET_INTERFACE);
	}

	InterfaceDescription* AudioSinkIntf = NULL;
	status = bus.CreateInterface(AUDIO_SINK_INTERFACE, AudioSinkIntf);
	if (status == ER_OK) {
		AudioSinkIntf->AddSignal("Connected", NULL,  NULL, 0);
		AudioSinkIntf->AddSignal("Disconnected", NULL,  NULL, 0);
		AudioSinkIntf->AddSignal("Playing", NULL,  NULL, 0);
		AudioSinkIntf->AddSignal("Stopped", NULL,  NULL, 0);
		AudioSinkIntf->Activate();

		status = AddInterface(*AudioSinkIntf);
		if (status == ER_OK) {
			/* Register the signal handler 'SapStateChanged' with the bus*/
			mBTEvents[AUDIO_SINK_CONNECTED] = AudioSinkIntf->GetMember("Connected");
			assert(mBTEvents[AUDIO_SINK_CONNECTED]);
			mBTEvents[AUDIO_SINK_DISCONNECTED] = AudioSinkIntf->GetMember("Disconnected");
			assert(mBTEvents[AUDIO_SINK_DISCONNECTED]);
			mBTEvents[AUDIO_SINK_STREAM_STARTED] = AudioSinkIntf->GetMember("Playing");
			assert(mBTEvents[AUDIO_SINK_STREAM_STARTED]);
			mBTEvents[AUDIO_SINK_STREAM_STOPPED] = AudioSinkIntf->GetMember("Stopped");
			assert(mBTEvents[AUDIO_SINK_STREAM_STOPPED]);
		}

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>(&BtConnObj::AudioSinkConnect),
				mBTEvents[AUDIO_SINK_CONNECTED],
				NULL);
		if (status != ER_OK) {
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);
		}

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>
					(&BtConnObj::AudioSinkDisConnect),
				mBTEvents[AUDIO_SINK_DISCONNECTED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>(&BtConnObj::AudioSinkPlaying),
				mBTEvents[AUDIO_SINK_STREAM_STARTED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>(&BtConnObj::AudioSinkStopped),
				mBTEvents[AUDIO_SINK_STREAM_STOPPED],
				NULL);
		if (status != ER_OK)
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);
	} else {
		BT_LOG("%s:Failed to create interface %s\n", __func__, AUDIO_SINK_INTERFACE);
	}

#ifdef HID_PROFILE_SUPPORT
	InterfaceDescription* InputDeviceIntf = NULL;
	status = bus.CreateInterface(INPUT_DEVICE_INTERFACE, InputDeviceIntf);
	if (status == ER_OK) {
		InputDeviceIntf->AddSignal("PropertyChanged", "sv", NULL, 0);
		InputDeviceIntf->Activate();

		status = AddInterface(*InputDeviceIntf);
		if (status == ER_OK) {
			mBTEvents[INPUT_DEVICE_PROPERTY_CHANGED] = InputDeviceIntf->GetMember("PropertyChanged");
			assert(mBTEvents[INPUT_DEVICE_PROPERTY_CHANGED]);
		}

		status =  bus.RegisterSignalHandler(this,
				static_cast<MessageReceiver::SignalHandler>
					(&BtConnObj::InputDevicePropertyChanged),
				mBTEvents[INPUT_DEVICE_PROPERTY_CHANGED],
				NULL);
		if (status != ER_OK) {
			BT_LOG("Failed to register signal handler %s:%d\n", __FUNCTION__, __LINE__);
		}
	} else {
		BT_LOG("%s:Failed to create interface %s\n", __func__, INPUT_DEVICE_INTERFACE);
	}
#endif
	mBtFiltInfo = btfilt_info;
}
extern "C" void alljoyn_register_signal(char *signal, char *interface)
{
	char signal_buf[300];
	QStatus status = ER_OK;

	snprintf(signal_buf, sizeof(signal_buf),
		"type='signal',member='%s',interface='%s'",
		signal, interface);
	status = g_msgBus->AddMatch(signal_buf);

	if (ER_OK != status)
		BT_LOG("Add Match failed: %s:%d\n", __FUNCTION__, __LINE__);

	return;
}

extern "C" void alljoyn_deregister_signal(char *signal, char *interface)
{
	char signal_buf[300];
	QStatus status = ER_OK;

	snprintf(signal_buf, sizeof(signal_buf),
		"type='signal',member='%s',interface='%s'",
		signal, interface);
	status = g_msgBus->RemoveMatch(signal_buf);
	if (ER_OK != status)
		BT_LOG("Remove Match failed: %s:%d\n", __FUNCTION__, __LINE__);
	return;
}

extern "C" int alljoyn_init(void *btfilt_info)
{
	QStatus status = ER_OK;

	/* Create message bus */
	g_msgBus = new BusAttachment("btfilter", true);
	if (g_msgBus == NULL) {
		return 1;
	}

	btfilt_obj = new BtConnObj(*g_msgBus, AUDIO_MANAGER_PATH, btfilt_info);

	/* Start the msg bus */
	status = g_msgBus->Start();
	if (ER_OK != status)
		return 1;

	g_msgBus->RegisterBusObject(*btfilt_obj);

	status = g_msgBus->Connect("unix:path=/dev/socket/dbus");
	if (ER_OK != status)
		return 1;

	BT_LOG("BusAttachement connected to %s\n", "unix:path=/dev/socket/dbus");

	/* Request name */
	uint32_t flags = DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE;
	status = g_msgBus->RequestName(AUDIO_SINK_INTERFACE, flags);
	BT_LOG("Bus name %s registered ::%s\n",
			AUDIO_SINK_INTERFACE, QCC_StatusText(status));

	return 0;
}

extern "C" void alljoyn_deinit()
{
	if (btfilt_obj != NULL) {
		delete btfilt_obj;
		btfilt_obj = NULL;
	}

	BT_LOG("destroyed the connection Status Object\n");
	/* Stop the bus (not strictly necessary since
	 * we are going to delete it anyways) */
	if (g_msgBus) {
		QStatus s = g_msgBus->Stop();
		if (ER_OK != s) {
			BT_LOG("BusAttachment::Stop failed\n");
		}
	}

	BT_LOG("Stopped the message bus\n");
	/* Deallocate bus */
	if (g_msgBus) {
		delete g_msgBus;
		g_msgBus = NULL;
	}

	BT_LOG("<- %s\n", __func__);
}
