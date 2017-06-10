/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Proprietary and Confidential.
 */

/*
 * Bluetooth Filter - BT module
 */

#ifdef BLUETOOTH_BLUEDROID

//#include "abtfilt_bluedroid.h"
#include "alljoyn_dbus.h"
#include <endian.h>
#include <linux/un.h>
#include <sys/poll.h>
#include <sys/uio.h>

#define BTEV_GET_BT_CONN_LINK_TYPE(p)   ((p)[9])
#define BTEV_GET_TRANS_INTERVAL(p)      ((p)[10])
#define BTEV_GET_RETRANS_INTERVAL(p)    ((p)[11])
#define BTEV_GET_RX_PKT_LEN(p)          ((A_UINT16)((p)[12]) | (((A_UINT16)((p)[13])) << 8))
#define BTEV_GET_TX_PKT_LEN(p)          ((A_UINT16)((p)[14]) | (((A_UINT16)((p)[15])) << 8))
#define BTEV_CMD_COMPLETE_GET_OPCODE(p) ((A_UINT16)((p)[1]) | (((A_UINT16)((p)[2])) << 8))
#define BTEV_CMD_COMPLETE_GET_STATUS(p) ((p)[3])

#define MAKE_BTSTATE_MASK(state) (1 << (state))
#define HCI_RESET_CMD 0x0c03

/* This is the global flag used to sync between hci filter
 * thread and the stack event thread (who could issue HCI
 * command) */
int disallow_hcithread_read = 0;
static inline void bdaddr_to_str(const bd_addr_t *src, char *dst)
{
	snprintf(dst, 17, "%X:%X:%X:%X:%X:%X", src->octet[5],
			src->octet[4], src->octet[3], src->octet[2], src->octet[1],
			src->octet[0]);
}

typedef void (*signal_handle_t)(void *arg, void *data);

typedef struct _BT_NOTIFICATION_CONFIG_PARAMS {
	char signal_num;
	signal_handle_t signal_handler;
} BT_NOTIFICATION_CONFIG_PARAMS;

/* Function Prototypes */
static void BtAdapterAdded(void *string,
		void * user_data);
static void BtAdapterRemoved(void *string,
		void * user_data);
static A_STATUS AcquireBtAdapter(ABF_BT_INFO *pAbfBtInfo);
static void ReleaseBTAdapter(ABF_BT_INFO *pAbfBtInfo);
static A_STATUS GetAdapterInfo(ABF_BT_INFO *pAbfBtInfo);
static void RemoteDeviceDisconnected(void *string,
		void * user_data);
static void RemoteDeviceConnected(void *string,
		void * user_data);
static void AudioDeviceAdded(void *string,
		void * user_data);
static void AudioDeviceRemoved(void *string,
		void * user_data);
static void DeviceDiscoveryStarted(void *arg, void * user_data);
static void DeviceDiscoveryFinished(void *arg, void * user_data);
static void AudioHeadsetConnected(void *arg, void * user_data);
static void AudioHeadsetDisconnected(void *arg, void * user_data);
static void AudioHeadsetStreamStarted(void *arg, void * user_data);
static void AudioHeadsetStreamStopped(void *arg, void * user_data);
static void AudioGatewayConnected(void *arg, void * user_data);
static void AudioGatewayDisconnected(void *arg, void * user_data);
static void AudioSinkConnected(void *arg, void * user_data);
static void AudioSinkDisconnected(void *arg, void * user_data);
static void AudioSinkStreamStarted(void *arg, void * user_data);
static void AudioSinkStreamStopped(void *arg, void * user_data);
static void AudioSourceConnected(void *arg, void * user_data);
static void AudioSourceDisconnected(void *arg, void * user_data);
static void InputDeviceConnected(void *arg, void * user_data);
static void InputDeviceDisconnected(void *arg, void * user_data);
static A_STATUS CheckAndAcquireDefaultAdapter(ABF_BT_INFO *pAbfBtInfo);
static void ReleaseDefaultAdapter(ABF_BT_INFO *pAbfBtInfo);
static void AcquireDefaultAudioDevice(ABF_BT_INFO *pAbfBtInfo);
static void ReleaseDefaultAudioDevice(ABF_BT_INFO *pAbfBtInfo);
static void GetBtAudioConnectionProperties(ABF_BT_INFO              *pAbfBtInfo,
		ATHBT_STATE_INDICATION   Indication);
static void GetBtAudioDeviceProperties(ABF_BT_INFO  *pAbfBtInfo);

static A_STATUS SetupHciEventFilter(ABF_BT_INFO *pAbfBtInfo);
static void CleanupHciEventFilter(ABF_BT_INFO *pAbfBtInfo);
static void CheckHciEventFilter(ABF_BT_INFO   *pAbfBtInfo);
void stop_monitor_stack_events(void *user_data);
A_STATUS start_monitor_stack_events(void *user_data);
static void thread_exit_handler(int sig);

static A_STATUS IssueHCICommand(ABF_BT_INFO *pAbfBtInfo,
		A_UINT16    OpCode,
		A_UCHAR     *pCmdData,
		int         CmdLength,
		int         EventRecvTimeoutMS,
		A_UCHAR     *pEventBuffer,
		int         MaxLength,
		A_UCHAR     **ppEventPtr,
		int         *pEventLength);


static A_STATUS CheckRemoteDeviceEDRCapable(ABF_BT_INFO *pAbfBtInfo, A_BOOL *pEDRCapable);

/* New function to check Remote LMP version */
static A_STATUS GetRemoteDeviceLMPVersion(ABF_BT_INFO *pAbfBtInfo);

static void *HCIFilterThread(void *arg);

A_BOOL g_AppShutdown = FALSE;

A_UINT16 g_ACLConnHandle;
A_UINT16 g_SCOConnHandle;
extern A_BOOL BT_terminated(void);

static BT_NOTIFICATION_CONFIG_PARAMS g_NotificationConfig[] =
{
	{BD_BT_ADAPTER_ADDED, BtAdapterAdded},
	{BD_BT_ADAPTER_REMOVED, BtAdapterRemoved},
	{BD_DEVICE_DISCOVERY_STARTED, DeviceDiscoveryStarted},
	{BD_DEVICE_DISCOVERY_FINISHED, DeviceDiscoveryFinished},
	{BD_REMOTE_DEVICE_CONNECTED, RemoteDeviceConnected},
	{BD_REMOTE_DEVICE_DISCONNECTED, RemoteDeviceDisconnected},
	{BD_AUDIO_DEVICE_ADDED, AudioDeviceAdded},
	{BD_AUDIO_DEVICE_REMOVED, AudioDeviceRemoved},
	{BD_AUDIO_HEADSET_CONNECTED, AudioHeadsetConnected},
	{BD_AUDIO_HEADSET_DISCONNECTED, AudioHeadsetDisconnected},
	{BD_AUDIO_HEADSET_STREAM_STARTED, AudioHeadsetStreamStarted},
	{BD_AUDIO_HEADSET_STREAM_STOPPED, AudioHeadsetStreamStopped},
	{BD_AUDIO_SINK_CONNECTED, AudioSinkConnected},
	{BD_AUDIO_SINK_DISCONNECTED, AudioSinkDisconnected},
	{BD_AUDIO_SINK_STREAM_STARTED, AudioSinkStreamStarted},
	{BD_AUDIO_SINK_STREAM_STOPPED, AudioSinkStreamStopped},
	{BD_INPUT_DEVICE_CONNECTED, InputDeviceConnected},
	{BD_INPUT_DEVICE_DISCONNECTED, InputDeviceDisconnected},
	{0, NULL},
};

#define ForgetRemoteAudioDevice(pA)     \
{                                       \
	A_MEMZERO((pA)->DefaultRemoteAudioDeviceAddress,sizeof((pA)->DefaultRemoteAudioDeviceAddress)); \
	(pA)->DefaultRemoteAudioDevicePropsValid = FALSE;                                               \
}

/* APIs exported to other modules */
A_STATUS Abf_BtStackNotificationInit(ATH_BT_FILTER_INSTANCE *pInstance,
									 A_UINT32 Flags)
{
	A_STATUS status = A_OK;
	ATHBT_FILTER_INFO *pInfo;
	ABF_BT_INFO *pAbfBtInfo;

	pInfo = (ATHBT_FILTER_INFO *)pInstance->pContext;
	if (pInfo->pBtInfo) {
		return A_OK;
	}

	if (g_AppShutdown) {
		A_ERR (" App was already shutdown cannot call Abf_BtStackNotificationInit again!!!");
		return A_ERROR;
	}

	pAbfBtInfo = (ABF_BT_INFO *)A_MALLOC(sizeof(ABF_BT_INFO));
	A_MEMZERO(pAbfBtInfo,sizeof(ABF_BT_INFO));

	pInfo->Flags = Flags;

	if (pInfo->Flags & ABF_ENABLE_AFH_CHANNEL_CLASSIFICATION) {
		A_INFO("AFH Classification Command will be issued on WLAN connect/disconnect \n");
	}

	if (pInfo->Flags & ABF_USE_HCI_FILTER_FOR_HEADSET_PROFILE) {
		A_INFO("Headset Profile notifications will use HCI filter instead of stack events \n");

		/* ignore certain state detections that we can handle through stack events */
		pInfo->FilterCore.StateFilterIgnore = MAKE_BTSTATE_MASK(ATH_BT_CONNECT) |
			MAKE_BTSTATE_MASK(ATH_BT_INQUIRY) |
			MAKE_BTSTATE_MASK(ATH_BT_A2DP);
	}

	pAbfBtInfo->AdapterAvailable = FALSE;
	pAbfBtInfo->pInfo = pInfo;
	pAbfBtInfo->HCIEventListenerSocket = -1;
	pInfo->pBtInfo = pAbfBtInfo;
	A_INFO("before init %s:%d\n", __FUNCTION__, __LINE__);

	/* oliu: remove alljoyn_init, need bluedroid_init
	alljoyn_init((void *)pAbfBtInfo);
	 */

	A_INFO("After init %s:%d\n", __FUNCTION__, __LINE__);

	/* check for default adapter at startup */
	status = CheckAndAcquireDefaultAdapter(pAbfBtInfo);

	/* oliu: remove REGISTER_SIGNAL
	REGISTER_SIGNAL(BT_ADAPTER_ADDED);
	REGISTER_SIGNAL(BT_ADAPTER_REMOVED);
	*/
	A_INFO("Init complete:%s:%d\n", __FUNCTION__, __LINE__);


	if (A_FAILED(status)) {
		Abf_BtStackNotificationDeInit(pInstance);
	}
	A_INFO("BT Stack Notification init complete\n");

	return status;
}

void Abf_BtStackNotificationDeInit(ATH_BT_FILTER_INSTANCE *pInstance)
{
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pInstance->pContext;
	ABF_BT_INFO *pAbfBtInfo = pInfo->pBtInfo;

	g_AppShutdown = TRUE;

	if (!pAbfBtInfo)
		return;

	/* Free the remaining resources */
	pAbfBtInfo->AdapterAvailable = FALSE;
	pInfo->pBtInfo = NULL;

	/* oliu: remove alljoyn_deinit
	alljoyn_deinit();
	*/
	ReleaseDefaultAdapter(pAbfBtInfo);
	A_FREE(pAbfBtInfo);
	A_INFO("BT Stack Notification de-init complete\n");
}

static A_STATUS CheckAndAcquireDefaultAdapter(ABF_BT_INFO *pAbfBtInfo)
{
	A_STATUS status = A_OK;

	do {
		if (pAbfBtInfo->AdapterAvailable) {
			/* already available */
			break;
		}
		/* acquire the adapter */
		status = AcquireBtAdapter(pAbfBtInfo);

	} while (FALSE);

	return status;
}

static void ReleaseDefaultAdapter(ABF_BT_INFO *pAbfBtInfo)
{

	if (pAbfBtInfo->AdapterAvailable) {
		/* Release the BT adapter */
		ReleaseBTAdapter(pAbfBtInfo);
		A_INFO("BT Adapter Removed\n");
	}
}

/* Event Notifications */
static void BtAdapterAdded(void *string, void * user_data)
{
	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);

	CheckAndAcquireDefaultAdapter((ABF_BT_INFO *)user_data);
}

static void BtAdapterRemoved(void *string, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;

	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);

        Abf_BToff();
	if (!pAbfBtInfo->AdapterAvailable)
		return;

	/* ReleaseDefaultAdapter involves the deletion
	 * of stack monitor thread that this function is
	 * invoked in. Move it to the function
	 * Abf_BtStackNotificationDeInit which is invoked
	 * in the main thread.
	 */
#if 0
	ReleaseDefaultAdapter(pAbfBtInfo);
#endif
	Abf_WlanIssueBtOnOff(pAbfBtInfo->pInfo, STATE_OFF);
	A_INFO("Terminate abtfilt\n");
}

static void DeviceDiscoveryStarted(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);
	AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_ON);
}

static void DeviceDiscoveryFinished(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);
	AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_OFF);
}

static void RemoteDeviceConnected(void *string, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);
	//A_STR2ADDR(string, pAbfBtInfo->RemoteDevice);
	AthBtIndicateState(pInstance, ATH_BT_CONNECT, STATE_ON);
}

static void RemoteDeviceDisconnected(void *string, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);
	//A_MEMZERO(pAbfBtInfo->RemoteDevice, sizeof(pAbfBtInfo->RemoteDevice));
	AthBtIndicateState(pInstance, ATH_BT_CONNECT, STATE_OFF);
}

static void ReleaseDefaultAudioDevice(ABF_BT_INFO *pAbfBtInfo)
{
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;

	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);
	if (pAbfBtInfo->AudioCbRegistered) {
		pAbfBtInfo->AudioCbRegistered = FALSE;
	}
}

static void AcquireDefaultAudioDevice(ABF_BT_INFO *pAbfBtInfo)
{
	A_BOOL     success = FALSE;

	A_INFO("[%s]StartRegister\n", __FUNCTION__);
	do {

		if (pAbfBtInfo->DefaultAudioDeviceAvailable) {
			/* already acquired */
			success = TRUE;
			break;
		}
		pAbfBtInfo->DefaultAudioDeviceAvailable = FALSE;

		pAbfBtInfo->AudioCbRegistered = TRUE;

		success = TRUE;

		A_INFO("[%s]EndRegister\n", __FUNCTION__);
	} while (FALSE);

	if (!success) {
		/* cleanup */
		ReleaseDefaultAudioDevice(pAbfBtInfo);
	}
}

static void AudioDeviceAdded(void *string, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;

	A_INFO("Audio Device Added: %s\n", string);
	/* release current one if any */
	ReleaseDefaultAudioDevice(pAbfBtInfo);
	/* re-acquire the new default, it could be the same one */
	AcquireDefaultAudioDevice(pAbfBtInfo);
}

static void AudioDeviceRemoved(void *string, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;

	A_INFO("Audio Device Removed: %s\n", string);
	/* release current one  */
	ReleaseDefaultAudioDevice(pAbfBtInfo);
	/* re-acquire the new default (if any) */
	AcquireDefaultAudioDevice(pAbfBtInfo);
}

static void AudioHeadsetConnected(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;
	A_INFO("Audio Headset Connected \n");

	if(!(pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING)) {
		/* todo: get capability and version */
		GetBtAudioDeviceProperties(pAbfBtInfo);
	}

	/* Turn off inquiry once connected */
	if (pAbfBtInfo->btInquiryState & 0x1) {
		pAbfBtInfo->btInquiryState &= ~(1 << 0);
		AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_OFF);
	}
}

static void AudioHeadsetDisconnected(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO  *)user_data;

	A_INFO("Audio Headset (%s) Disconnected\n",pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
	ForgetRemoteAudioDevice(pAbfBtInfo);
}

static void AudioHeadsetStreamStarted(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("Audio Headset (%s) Stream Started \n",pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
	if(!(pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING)) {
		if (!pAbfBtInfo->DefaultRemoteAudioDevicePropsValid) {
			/* todo: get capability and version */
			GetBtAudioDeviceProperties(pAbfBtInfo);
		}
	}
	/* make the indication */
	/* todo: get properties (role and capability) of this headset connection */
	GetBtAudioConnectionProperties(pAbfBtInfo, ATH_BT_SCO);

	AthBtIndicateState(pInstance,
			pAbfBtInfo->CurrentSCOLinkType == SCO_CONN ? ATH_BT_SCO : ATH_BT_ESCO,
			STATE_ON);
}

static void AudioHeadsetStreamStopped(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	/* This event can also be used to indicate the SCO state */
	A_INFO("Audio Headset (%s) Stream Stopped \n", pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
	AthBtIndicateState(pInstance,
			pAbfBtInfo->CurrentSCOLinkType == SCO_CONN ? ATH_BT_SCO : ATH_BT_ESCO,
			STATE_OFF);
}

static void AudioSinkConnected(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;
	A_INFO("Audio Sink Connected \n");

	if(!(pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING)) {
		/* todo: get version and capability */
		GetBtAudioDeviceProperties(pAbfBtInfo);
	}

	/* Turn off inquiry once connected */
	if (pAbfBtInfo->btInquiryState & 0x1) {
		pAbfBtInfo->btInquiryState &= ~(1 << 0);
		AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_OFF);
	}
}

static void AudioSinkDisconnected(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("Audio Sink (%s) Disconnected \n",pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
	AthBtIndicateState(pInstance, ATH_BT_A2DP, STATE_OFF);
	ForgetRemoteAudioDevice(pAbfBtInfo);
}

static void AudioSinkStreamStarted(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	if (!pAbfBtInfo->DefaultRemoteAudioDevicePropsValid) {
		/* todo: get capability and version */
		GetBtAudioDeviceProperties(pAbfBtInfo);
	}

	A_INFO("Audio Sink (%s) Stream Started \n",pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
	/* todo: get connection properties */
	GetBtAudioConnectionProperties(pAbfBtInfo, ATH_BT_A2DP);
	AthBtIndicateState(pInstance, ATH_BT_A2DP, STATE_ON);
}

static void AudioSinkStreamStopped(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("Audio Sink (%s) Stream Stopped \n",pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
	AthBtIndicateState(pInstance, ATH_BT_A2DP, STATE_OFF);
}

#ifdef HID_PROFILE_SUPPORT
A_UINT16 g_HIDConnHandle = 0;
#define HID_SUPPORT_NUM 6
struct HID_record{
	A_UINT16 handle;
	A_UINT8  sniff;
};
struct HID_record hid_conn[HID_SUPPORT_NUM] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};

void ConnHIDDevice(A_UINT16 handle){
	A_UINT8 i = 0;

	for (i=0; i<HID_SUPPORT_NUM; i++){
		if (hid_conn[i].handle == handle) {
			A_ERR("ERROR! duplicated HID conn_handle!!\n");
			return;
		}
	}
	for (i=0; i<HID_SUPPORT_NUM; i++){
		if (hid_conn[i].handle == 0) {
			hid_conn[i].handle = handle;
			A_INFO("add HID conn idx:%d  handle:%x", i, handle);
			break;
		}
	}

	if (i == HID_SUPPORT_NUM) {
		A_ERR("ERROR! no free HID conn_handle!!\n");
	}
}

void DisconnHIDDevice(A_UINT16 handle){
	A_UINT8 i = 0;

	for (i=0; i<HID_SUPPORT_NUM; i++){
		if (hid_conn[i].handle == handle) {
			hid_conn[i].handle = 0;
			A_INFO("del HID conn idx:%d  handle:%x", i, handle);
			return;
		}
	}
}

A_BOOL HIDDeviceAllDisconnect(void)
{
	A_UINT8 i = 0;

	for (i = 0; i < HID_SUPPORT_NUM; i++) {
		if (hid_conn[i].handle != 0) {
			return FALSE;
		}
	}
	return TRUE;
}

#endif

static void InputDeviceConnected(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_DEBUG("HID Connected\n");
	AthBtIndicateState(pInstance, ATH_BT_HID, STATE_ON);
	ConnHIDDevice(g_HIDConnHandle);
}

static void InputDeviceDisconnected(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_DEBUG("HID Disconnected\n");
}

static A_STATUS AcquireBtAdapter(ABF_BT_INFO *pAbfBtInfo)
{
	A_STATUS        status = A_ERROR;
	char            *hciName;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;

	do {

		/* assume ID 0 */
		pAbfBtInfo->AdapterId = 0;

		/* start stack event thread */
		if (!A_SUCCESS(start_monitor_stack_events((void *)pAbfBtInfo))) {
			break;
		}

		/* todo: setup HciEventFilter to get adapter info. */
		if(!(pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING)) {
			if (!A_SUCCESS(SetupHciEventFilter(pAbfBtInfo))) {
				break;
			}
			GetAdapterInfo(pAbfBtInfo);
			/* send AFH command here to make sure BTCServer socket is ready */
			IndicateCurrentWLANOperatingChannel(pInfo);
		}


		pAbfBtInfo->pInfo->LMPVersion = pAbfBtInfo->HCI_LMPVersion;

		pAbfBtInfo->AdapterAvailable = TRUE;

		pAbfBtInfo->AdapterCbRegistered = TRUE;

		A_INFO("BT Adapter Added\n");

		/* acquire default audio device */
		AcquireDefaultAudioDevice(pAbfBtInfo);

		status = A_OK;

	} while (FALSE);

	return status;
}

static void ReleaseBTAdapter(ABF_BT_INFO *pAbfBtInfo)
{

	if (pAbfBtInfo->AdapterCbRegistered) {
		pAbfBtInfo->AdapterCbRegistered = FALSE;
		/* Free the resources held for the event handlers */
	}

	ReleaseDefaultAudioDevice(pAbfBtInfo);

	stop_monitor_stack_events((void *)pAbfBtInfo);

	CleanupHciEventFilter(pAbfBtInfo);

	A_MEMZERO(pAbfBtInfo->HCI_DeviceAddress,
			sizeof(pAbfBtInfo->HCI_DeviceAddress));
	pAbfBtInfo->HCI_LMPVersion = 0;

	pAbfBtInfo->AdapterAvailable = FALSE;
}

static A_STATUS GetAdapterInfo(ABF_BT_INFO *pAbfBtInfo)
{

	A_STATUS status;

	int i;
	A_UCHAR eventBuffer[MAX_HCI_EVENT_SIZE];
	A_UCHAR *eventPtr;
	int eventLen;

	/* Get adapter/device address by issuing HCI command, Read_BD_ADDR */
	status = IssueHCICommand(pAbfBtInfo,
			HCICMD_OPCODE(OGF_INFORMATIONAL_PARAMS, OCF_READ_BD_ADDR),
			0,
			0,
			2000,
			eventBuffer,
			sizeof(eventBuffer),
			&eventPtr,
			&eventLen);

	if (A_FAILED(status)) {
		A_ERR("[%s] Failed to get BD_ADDR \n", __FUNCTION__);
		return status;
	}

	if (eventBuffer[6] == 0) { /* READ_BD_ADDR was a success */
		for (i = 0; i < BD_ADDR_SIZE; i++) {
			pAbfBtInfo->HCI_DeviceAddress[i] = eventBuffer[i+7];
		}
		A_DEBUG("BT-HCI Device Address: (%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X)\n",
				pAbfBtInfo->HCI_DeviceAddress[0], pAbfBtInfo->HCI_DeviceAddress[1],
				pAbfBtInfo->HCI_DeviceAddress[2], pAbfBtInfo->HCI_DeviceAddress[3],
				pAbfBtInfo->HCI_DeviceAddress[4], pAbfBtInfo->HCI_DeviceAddress[5]);
	}

	status = A_OK;

	return status;
}

static A_STATUS GetConnectedDeviceRole(ABF_BT_INFO   *pAbfBtInfo,
		A_CHAR        *Address,
		A_BOOL        IsSCO,
		A_UCHAR       *pRole)
{
        A_STATUS status = A_ERROR;
        A_UCHAR eventBuffer[MAX_HCI_EVENT_SIZE];
        A_UCHAR *eventPtr;
        int eventLen;
        A_UINT16 conn_handle = (IsSCO)? g_SCOConnHandle: g_ACLConnHandle;

        /* issue HCI_Role_Discovery command to get the current role */
        status = IssueHCICommand(pAbfBtInfo,
                HCICMD_OPCODE(OGF_LINK_POLICY, OCF_ROLE_DISCOVERY),
                (A_UCHAR *)&conn_handle,
                2,
                2000,
                eventBuffer,
                sizeof(eventBuffer),
                &eventPtr,
                &eventLen);

        if (A_FAILED(status)) {
                A_ERR("[%s] Failed to discover current role\n", __FUNCTION__);
                return status;
        }

	A_DUMP_BUFFER(eventPtr, eventLen, "Role Discovery");
        /* get current role */
        if (eventBuffer[6] == 0) { /* ROLE_DISCOVERY was a success */
                if (eventBuffer[9] == 0) {
                        *pRole = 0;
                }
                else {
                        *pRole = 1;
                }
                A_INFO("Found Connection (Link-Type : %d), found role:%d \n",
                        IsSCO, *pRole);
        }

        status = A_OK;

        return status;


}

static void GetBtAudioDeviceProperties(ABF_BT_INFO  *pAbfBtInfo)
{
	A_STATUS    status;

	pAbfBtInfo->DefaultRemoteAudioDevicePropsValid = FALSE;

	do {

		/* Need RemoteDeviceAddress */


		status = GetRemoteDeviceLMPVersion(pAbfBtInfo);

		/* assume 2.1 or later */
		pAbfBtInfo->DefaultAudioDeviceLmpVersion = 4;

		if (strstr(pAbfBtInfo->DefaultRemoteAudioDeviceVersion,"1.0") != NULL) {
			pAbfBtInfo->DefaultAudioDeviceLmpVersion = 0;
		} else if (strstr(pAbfBtInfo->DefaultRemoteAudioDeviceVersion,"1.1") != NULL) {
			pAbfBtInfo->DefaultAudioDeviceLmpVersion = 1;
		} else if (strstr(pAbfBtInfo->DefaultRemoteAudioDeviceVersion,"1.2") != NULL) {
			pAbfBtInfo->DefaultAudioDeviceLmpVersion = 2;
		} else if (strstr(pAbfBtInfo->DefaultRemoteAudioDeviceVersion,"2.0") != NULL) {
			/* NOTE: contrary to what the DBUS documentation says, the BT string will
			 * not indicate +EDR to indiate the remote device is EDR capable!
			 * */
			pAbfBtInfo->DefaultAudioDeviceLmpVersion = 3;
		}

		if (pAbfBtInfo->DefaultAudioDeviceLmpVersion >= 3) {
			A_BOOL EDRCapable = FALSE;
			/* double check that the device is EDR capable, a 2.0 device can be EDR or non EDR */
			status = CheckRemoteDeviceEDRCapable(pAbfBtInfo, &EDRCapable);
			if (A_SUCCESS(status)) {
				if (!EDRCapable) {
					A_INFO("Remote Audio Device (%s) is not EDR Capable, downgrading lmp version.. \n",
							pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
					/* for audio coex, treat this like a 1.2 device */
					pAbfBtInfo->DefaultAudioDeviceLmpVersion = 2;
				}
			}
		}
		pAbfBtInfo->DefaultRemoteAudioDevicePropsValid = TRUE;

	} while (FALSE);

}

#define LMP_FEATURE_ACL_EDR_2MBPS_BYTE_INDEX  3
#define LMP_FEATURE_ACL_EDR_2MBPS_BIT_MASK    0x2
#define LMP_FEATURE_ACL_EDR_3MBPS_BYTE_INDEX  3
#define LMP_FEATURE_ACL_EDR_3MBPS_BIT_MASK    0x4
#define LMP_FEATURES_LENGTH                   8

#define HCI_REMOTE_COMMAND_TIMEOUT 2000

static A_STATUS GetRemoteAclDeviceHandle(ABF_BT_INFO *pAbfBtInfo, A_UINT16 *pConn_handle)
{
	A_STATUS status = A_OK;
	*pConn_handle = g_ACLConnHandle;
	A_DEBUG("conn handle 0x%x\n", *pConn_handle);

	return status;

}

static A_STATUS CheckRemoteDeviceEDRCapable(ABF_BT_INFO *pAbfBtInfo, A_BOOL *pEDRCapable)
{
#if 1//WAR for "BT controller timeout" issue
	A_INFO("%s: forced to return EDR capable instead of issuing OCF_READ_REMOTE_FEATURES\n", __func__);
	*pEDRCapable = TRUE;
	return A_OK;
#else
	A_STATUS status;
	A_UINT16 conn_handle;
	A_UCHAR evtBuffer[MAX_HCI_EVENT_SIZE];
	A_UCHAR *eventPtr;
	int eventLen;
	A_UINT8 *lmp_features;
	do {
		status = GetRemoteAclDeviceHandle(pAbfBtInfo, &conn_handle);
		if (A_FAILED(status)) {
			break;
		}
		status = IssueHCICommand(pAbfBtInfo,
				HCICMD_OPCODE(OGF_LINK_CONTROL, OCF_READ_REMOTE_SUPPORTED_FEATURES),
				(A_UCHAR *)&conn_handle,
				2,
				HCI_REMOTE_COMMAND_TIMEOUT,
				evtBuffer,
				sizeof(evtBuffer),
				&eventPtr,
				&eventLen);

		if (A_FAILED(status)) {
			A_ERR("[%s] Failed to get remote features \n", __FUNCTION__);
			break;
		}

		/* Process LMP Features */
		lmp_features = &eventPtr[3];

		A_DUMP_BUFFER(lmp_features,sizeof(lmp_features),"Remote Device LMP Features:");

		if ((lmp_features[LMP_FEATURE_ACL_EDR_2MBPS_BYTE_INDEX] & LMP_FEATURE_ACL_EDR_2MBPS_BIT_MASK)  ||
				(lmp_features[LMP_FEATURE_ACL_EDR_3MBPS_BYTE_INDEX] & LMP_FEATURE_ACL_EDR_3MBPS_BIT_MASK)) {
			A_INFO("Device (%s) is EDR capable \n", pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
			*pEDRCapable = TRUE;
		} else {
			A_INFO("Device (%s) is NOT EDR capable \n", pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
			*pEDRCapable = FALSE;
		}
	} while (0);

	return status;
#endif
}

static A_STATUS GetRemoteDeviceLMPVersion(ABF_BT_INFO *pAbfBtInfo)
{
	A_STATUS status;
	A_UINT16 conn_handle;
	A_UCHAR evtBuffer[MAX_HCI_EVENT_SIZE];
	A_UCHAR *eventPtr;
	int eventLen;

	do {
		status = GetRemoteAclDeviceHandle(pAbfBtInfo, &conn_handle);
		if (A_FAILED(status)) {
			break;
		}
		status = IssueHCICommand(pAbfBtInfo,
				HCICMD_OPCODE(OGF_LINK_CONTROL, OCF_READ_REMOTE_VERSION_INFO),
				(A_UCHAR *)&conn_handle,
				2,
				HCI_REMOTE_COMMAND_TIMEOUT,
				evtBuffer,
				sizeof(evtBuffer),
				&eventPtr,
				&eventLen);

		if (A_FAILED(status)) {
			A_ERR("[%s] Failed to get remote Version \n", __FUNCTION__);
			break;
		}

		/* Process LMP Version */

		if (eventPtr[3] == 0) {
			strlcpy(&pAbfBtInfo->DefaultRemoteAudioDeviceVersion[0], "1.0", 32);
		} else if (eventPtr[3] == 1) {
			strlcpy(&pAbfBtInfo->DefaultRemoteAudioDeviceVersion[0], "1.1", 32);
		} else if (eventPtr[3] == 2) {
			strlcpy(&pAbfBtInfo->DefaultRemoteAudioDeviceVersion[0], "1.2", 32);
		} else if (eventPtr[3] == 3) {
			strlcpy(&pAbfBtInfo->DefaultRemoteAudioDeviceVersion[0], "2.0", 32);
		}

		A_INFO("[%s], Remote Device LMP Version: %d, in string format this is: %s\n", __FUNCTION__, eventPtr[3],
				pAbfBtInfo->DefaultRemoteAudioDeviceVersion);
	} while (0);

	return status;
}

static void GetBtAudioConnectionProperties(ABF_BT_INFO              *pAbfBtInfo,
		ATHBT_STATE_INDICATION   Indication)
{
	A_UCHAR     role = 0;
	A_CHAR      *pDescr = NULL;
	A_STATUS    status;

	do {
		if (!pAbfBtInfo->DefaultRemoteAudioDevicePropsValid) {
			break;
		}

		/* Incases where HciX is not supported, don't check for the role */
		if((pAbfBtInfo->pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING)) {
			return;
		}
		/* get role */
		status = GetConnectedDeviceRole(pAbfBtInfo,
				pAbfBtInfo->DefaultRemoteAudioDeviceAddress,
				Indication == ATH_BT_A2DP ?FALSE : TRUE,
				&role);
		if (A_FAILED(status)) {
			role = 0;
		}
		if (Indication == ATH_BT_A2DP) {
			pDescr = "A2DP";
			pAbfBtInfo->pInfo->A2DPConnection_LMPVersion = pAbfBtInfo->DefaultAudioDeviceLmpVersion;
			pAbfBtInfo->pInfo->A2DPConnection_Role = role;
			A_INFO("A2DP Version %d\n",pAbfBtInfo->pInfo->A2DPConnection_LMPVersion);
		} else if (Indication == ATH_BT_SCO) {
			if (pAbfBtInfo->CurrentSCOLinkType == SCO_CONN) {
				pDescr = "SCO";
			} else {
				pDescr = "eSCO";
			}
			pAbfBtInfo->pInfo->SCOConnection_LMPVersion = pAbfBtInfo->DefaultAudioDeviceLmpVersion;
			pAbfBtInfo->pInfo->SCOConnection_Role = role;

			if((pAbfBtInfo->pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING)) {

				pAbfBtInfo->pInfo->SCOConnectInfo.Valid = TRUE;
			}else{
				/* for SCO connections check if the event filter captured
				 * the SYNCH connection complete event */
				CheckHciEventFilter(pAbfBtInfo);
			}
		} else {
			pDescr = "UNKNOWN!!";
		}

		A_INFO("BT Audio connection properties:  (%s) (role: %s, lmp version: %d) \n",
				pDescr, role ? "SLAVE" : "MASTER", pAbfBtInfo->DefaultAudioDeviceLmpVersion);

	} while (FALSE);

}



static A_STATUS WaitForHCIEvent(int         Socket,
		int         TimeoutMs,
		A_UCHAR     *pBuffer,
		int         MaxLength,
		A_UCHAR     EventCode,
		A_UINT16    OpCode,
		A_UCHAR     **ppEventPtr,
		int         *pEventLength)
{
	int eventLen;
	hci_event_header *eventHdr;
	struct pollfd pfd;
	int result;
	A_UCHAR *eventPtr;
	A_STATUS status = A_OK;

	*ppEventPtr = NULL;
	A_MEMZERO(&pfd,sizeof(pfd));
	pfd.fd = Socket;
	pfd.events = POLLIN;

	if (EventCode == EVT_CODE_COMMAND_COMPLETE)
		A_INFO("Waiting for HCI CMD Complete Event, Opcode: 0x%4.4X (%d MS) \n",OpCode, TimeoutMs);
	else
		A_INFO("Waiting for HCI Event: %d (%d MS) \n",EventCode, TimeoutMs);

	while (1) {

		/* check socket for a captured event using a short timeout
		 * the caller usually calls this function when it knows there
		 * is an event that is likely to be captured */
		result = poll(&pfd, 1, TimeoutMs);

		if (result < 0) {
			if ((errno == EAGAIN) || (errno == EINTR)) {
				/* interrupted */
			} else {
				A_ERR("[%s] Socket Poll Failed! : %d \n", __FUNCTION__, errno);
				status = A_ERROR;
			}
			break;
		}

		if (result == 0) {
			A_ERR("[%s], poll returned with 0 \n",__FUNCTION__);
			status = A_ERROR;
			break;
		}

		if (!(pfd.revents & POLLIN)) {
			A_ERR("[%s], POLLIN check failed\n",__FUNCTION__);
			status = A_ERROR;
			break;
		}
		/* get the packet */
		eventLen = read(Socket, pBuffer, MaxLength);
		if (eventLen == 0) {
			/* no event */
			A_INFO("[%s], No Event\n",__FUNCTION__);
			status = A_ERROR;
			break;
		}
		if(eventLen > MaxLength) {
			A_ERR("[%s] Length longer than expected (%d) : %d \n", __FUNCTION__, MaxLength,
					eventLen);
			status = A_ERROR;
			break;
		}
		if (eventLen < (1 + (int)sizeof(hci_event_header))) {
			A_ERR("[%s] Unknown receive packet! len : %d \n", __FUNCTION__, eventLen);
			status = A_ERROR;
			break;
		}
		if (pBuffer[0] != HCI_EVENT_PACKET) {
			A_ERR("[%s] Unsupported packet type : %d \n", __FUNCTION__, pBuffer[0]);
			status = A_ERROR;
			break;
		}

		eventPtr = &pBuffer[1];
		eventLen--;
		eventHdr = (hci_event_header *)eventPtr;
		eventPtr += sizeof(hci_event_header);
		eventLen -= sizeof(hci_event_header);

		if (eventHdr->EventCode != EventCode) {
			/* not interested in this one */
			continue;
		}
		if(eventPtr == NULL) {
			A_ERR("[%s] Socket read points to NULL\n", __FUNCTION__);
			status = A_ERROR;
			break;
		}
		if (eventHdr->EventCode == EVT_CODE_COMMAND_COMPLETE) {
			if (eventLen < (int)sizeof(commad_complete_event)) {
				A_ERR("[%s] EVT_CODE_COMMAND_COMPLETE event is too small! len=%d \n", __FUNCTION__, eventLen);
				status = A_ERROR;
				break;
			} else {
				A_UINT16 evOpCode = letohs(BTEV_CMD_COMPLETE_GET_OPCODE(eventPtr));
				/* check for opCode match */
				if (OpCode != evOpCode) {
					/* keep searching */
					continue;
				}
			}
		}
		/* found it */
		*ppEventPtr = eventPtr;
		*pEventLength = eventLen;
		break;
	}
	return status;
}

static void CheckHciEventFilter(ABF_BT_INFO   *pAbfBtInfo)
{
	A_UCHAR     buffer[MAX_HCI_EVENT_SIZE];
	A_STATUS    status;
	A_UCHAR     *eventPtr;
	int         eventLen;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;

	do {

#if 0
		if (!USE_DBUS_FOR_HEADSET_PROFILE(pInfo)) {
			A_ERR("Calling CheckHciEventFilter is not valid in this mode! \n");
			break;
		}
#endif

		status = WaitForHCIEvent(pAbfBtInfo->HCIEventListenerSocket,
				100,
				buffer,
				sizeof(buffer),
				EVT_SCO_CONNECTION_COMPLETE,
				0,
				&eventPtr,
				&eventLen);

		if (A_FAILED(status))
			break;

		if (eventPtr == NULL)
			break;

		if (eventLen < EVENT_SCO_CONNECTION_COMP_SIZE) {
			A_ERR("SYNC_CONN_COMPLETE Event is too small! : %d \n", eventLen);
			break;
		}

		pAbfBtInfo->pInfo->SCOConnectInfo.LinkType = BTEV_GET_BT_CONN_LINK_TYPE(eventPtr);
		pAbfBtInfo->pInfo->SCOConnectInfo.TransmissionInterval = BTEV_GET_TRANS_INTERVAL(eventPtr);
		pAbfBtInfo->pInfo->SCOConnectInfo.RetransmissionInterval = BTEV_GET_RETRANS_INTERVAL(eventPtr);
		pAbfBtInfo->pInfo->SCOConnectInfo.RxPacketLength = BTEV_GET_RX_PKT_LEN(eventPtr);
		pAbfBtInfo->pInfo->SCOConnectInfo.TxPacketLength = BTEV_GET_TX_PKT_LEN(eventPtr);

		A_INFO("HCI SYNC_CONN_COMPLETE event captured, conn info (%d, %d, %d, %d, %d) \n",
				pAbfBtInfo->pInfo->SCOConnectInfo.LinkType,
				pAbfBtInfo->pInfo->SCOConnectInfo.TransmissionInterval,
				pAbfBtInfo->pInfo->SCOConnectInfo.RetransmissionInterval,
				pAbfBtInfo->pInfo->SCOConnectInfo.RxPacketLength,
				pAbfBtInfo->pInfo->SCOConnectInfo.TxPacketLength);

		/* now valid */
		pAbfBtInfo->pInfo->SCOConnectInfo.Valid = TRUE;

	} while (FALSE);

}

static void CleanupHciEventFilter(ABF_BT_INFO *pAbfBtInfo)
{
	A_STATUS status;

	if (pAbfBtInfo->HCIEventListenerSocket >= 0) {
		A_INFO("%s stop!\n", __FUNCTION__);
		pAbfBtInfo->HCIFilterThreadShutdown = TRUE;
		/* close socket, if there is a thread waiting on this socket, it will error and then exit */
		close(pAbfBtInfo->HCIEventListenerSocket);
		pAbfBtInfo->HCIEventListenerSocket = -1;
#if 0 /* Issue: WAR temporarily */
		if (pAbfBtInfo->HCIFilterThreadCreated) {
			A_INFO("[%s] Waiting for HCI filter thread to exit... \n",
					__FUNCTION__);
			/* wait for thread to exit
			 * note: JOIN cleans up thread resources as per POSIX spec. */
			status = A_TASK_JOIN(&pAbfBtInfo->hBtHCIFilterThread);
			if (A_FAILED(status)) {
				A_ERR("[%s] Failed to JOIN HCI filter thread \n",
						__FUNCTION__);
			}
			A_MEMZERO(&pAbfBtInfo->hBtHCIFilterThread,sizeof(pAbfBtInfo->hBtHCIFilterThread));
			pAbfBtInfo->HCIFilterThreadCreated = FALSE;
		}
#endif
	}

}


static A_STATUS SetupHciEventFilter(ABF_BT_INFO *pAbfBtInfo)
{
	struct sockaddr_un addr;
	socklen_t len;
	A_UINT16 retry_cnt = 0;
	A_STATUS status = A_ERROR;

	A_INFO("[%s] start\n", __FUNCTION__);

	do {
		memset(&addr, 0, sizeof(struct sockaddr_un));
		addr.sun_family = AF_LOCAL;
		strlcpy(&addr.sun_path[1], HCI_SOCKET_NAME, sizeof(addr.sun_path) - 1);
		len = offsetof(struct sockaddr_un, sun_path) + 1 + strlen(&addr.sun_path[1]);

		pAbfBtInfo->HCIEventListenerSocket = socket(PF_LOCAL, SOCK_STREAM, 0);

		if (pAbfBtInfo->HCIEventListenerSocket < 0) {
			A_ERR("[%s] Failed to get BTCServer socket: %d \n", __FUNCTION__, errno);
			break;
		}

		/* sympton: the BTCService could possibly start
		 * later than abtfilt. Add sleep here to delay
		 * socket connections to ensure BTCService is up.
		 */
		while (connect(pAbfBtInfo->HCIEventListenerSocket, (struct sockaddr *) &addr, len) < 0 &&
			retry_cnt < MAX_SOCKET_RETRY_CNT && !BT_terminated()) {
			retry_cnt++;
			A_INFO("%s: connect() retry %d times\n", __func__, retry_cnt);
			usleep(500000);
		}
		if (retry_cnt == MAX_SOCKET_RETRY_CNT || BT_terminated()) {
			A_ERR("%s: connect() failed: %s (%d)\n",
				__func__, strerror(errno), errno);
			break;
		}
		/* spawn a thread that will capture HCI events from the adapter */
		status = A_TASK_CREATE(&pAbfBtInfo->hBtHCIFilterThread, HCIFilterThread, pAbfBtInfo);
		if (A_FAILED(status)) {
			A_ERR("[%s] Failed to spawn a BT thread\n", __FUNCTION__);
			break;
		}
		pAbfBtInfo->HCIFilterThreadCreated = TRUE;

		status = A_OK;
	} while (FALSE);

	if (A_FAILED(status)) {
		CleanupHciEventFilter(pAbfBtInfo);
	}

	return status;
}


/* issue HCI command, currently this ONLY supports simple commands that
 * only expect a command complete, the event pointer returned points to the command
 * complete event structure for the caller to decode */
static A_STATUS IssueHCICommand(ABF_BT_INFO *pAbfBtInfo,
		A_UINT16    OpCode,
		A_UCHAR     *pCmdData,
		int         CmdLength,
		int         EventRecvTimeoutMS,
		A_UCHAR     *pEventBuffer,
		int         MaxLength,
		A_UCHAR     **ppEventPtr,
		int         *pEventLength)
{
	A_STATUS            status = A_ERROR;
	A_UCHAR             hciType = HCI_COMMAND_PACKET;
	hci_command_header  hciCommandHdr;
	struct  iovec       iv[3];
	int                 ivcount = 0;
	int                 sk,result;
	ATHBT_FILTER_INFO *pInfo = pAbfBtInfo->pInfo;
#if 0 /* use HCIEventListenerSocket temporarily */
	struct sockaddr_un addr;
	socklen_t len;
#endif

	A_MUTEX_LOCK(&pInfo->BTEventLock);
	disallow_hcithread_read = 1;

	do {
#if 0 /* use HCIEventListenerSocket temporarily */
		memset(&addr, 0, sizeof(struct sockaddr_un));
		addr.sun_family = AF_LOCAL;
		strlcpy(&addr.sun_path[1], HCI_SOCKET_NAME, sizeof(addr.sun_path) - 1);
		len = offsetof(struct sockaddr_un, sun_path) + 1 + strlen(&addr.sun_path[1]);
#endif
		sk = pAbfBtInfo->HCIEventListenerSocket;
		if (sk < 0) {
			A_ERR("Failed to get BTCServer socket: %s\n", __FUNCTION__);
			break;
		}
#if 0 /* use HCIEventListenerSocket temporarily */
		if (connect(sk, (struct sockaddr *) &addr, len) < 0) {
			A_ERR("%s: connect() failed: %s (%d)\n",
				__func__, strerror(errno), errno);
			close(sk);
			break;
		}
#endif

		hciCommandHdr.OpCode = htoles(OpCode);
		hciCommandHdr.ParamTotalLen = CmdLength;

		iv[0].iov_base = &hciType;
		iv[0].iov_len  = 1;
		ivcount++;
		iv[1].iov_base = &hciCommandHdr;
		iv[1].iov_len  = sizeof(hci_command_header);
		ivcount++;

		if (pCmdData != NULL) {
			iv[2].iov_base = pCmdData;
			iv[2].iov_len  = CmdLength;
			ivcount++;
		}
#if 1
		while ((result = writev(sk, iv, ivcount)) < 0) {
			if (errno == EAGAIN || errno == EINTR) {
				continue;
			}
			break;
		}

		if (result <= 0) {
			A_ERR("[%s] Failed to write to hci:%d (err:%d) \n", __FUNCTION__, pAbfBtInfo->AdapterId, errno);
			break;
		}
#endif

		/* To support new HCI Commands */
		switch (OpCode) {
			case HCICMD_OPCODE(OGF_LINK_CONTROL, OCF_READ_REMOTE_SUPPORTED_FEATURES):
				status = WaitForHCIEvent(sk,
						EventRecvTimeoutMS,
						pEventBuffer,
						MaxLength,
						EVT_CODE_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE,
						OpCode,
						ppEventPtr,
						pEventLength);
				break;
			case HCICMD_OPCODE(OGF_LINK_CONTROL, OCF_READ_REMOTE_VERSION_INFO):
				status = WaitForHCIEvent(sk,
						EventRecvTimeoutMS,
						pEventBuffer,
						MaxLength,
						EVT_CODE_READ_REMOTE_VERSION_INFO_COMPLETE,
						OpCode,
						ppEventPtr,
						pEventLength);
				break;
			default:
				status = WaitForHCIEvent(sk,
						EventRecvTimeoutMS,
						pEventBuffer,
						MaxLength,
						EVT_CODE_COMMAND_COMPLETE,
						OpCode,
						ppEventPtr,
						pEventLength);
				break;
		}

		if (A_FAILED(status)) {
			break;
		}

		status = A_OK;

	} while (FALSE);

	disallow_hcithread_read = 0;
	A_MUTEX_UNLOCK(&pInfo->BTEventLock);

#if 0 /* use HCIEventListenerSocket temporarily */
	if (sk >= 0) {
		close(sk);
	}
#endif

	return status;
}

#define AFH_CHANNEL_MAP_BYTES  10

typedef struct _WLAN_CHANNEL_MAP {
	A_UCHAR  Map[AFH_CHANNEL_MAP_BYTES];
} WLAN_CHANNEL_MAP;

#define MAX_WLAN_CHANNELS 14

typedef struct _WLAN_CHANNEL_RANGE {
	int    ChannelNumber;
	int    Center;       /* in Mhz */
} WLAN_CHANNEL_RANGE;

const WLAN_CHANNEL_RANGE g_ChannelTable[MAX_WLAN_CHANNELS] = {
	{ 1  , 2412},
	{ 2  , 2417},
	{ 3  , 2422},
	{ 4  , 2427},
	{ 5  , 2432},
	{ 6  , 2437},
	{ 7  , 2442},
	{ 8  , 2447},
	{ 9  , 2452},
	{ 10 , 2457},
	{ 11 , 2462},
	{ 12 , 2467},
	{ 13 , 2472},
	{ 14 , 2484},
};

static WLAN_CHANNEL_MAP g_ChannelMapTable[MAX_WLAN_CHANNELS + 1] = {
	{ {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F}}, /* 0 -- no WLAN */
	{ {0x00,0x00,0xC0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F}}, /* 1 */
	{ {0x0F,0x00,0x00,0xF8,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F}}, /* 2 */
	{ {0xFF,0x01,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F}}, /* 3 */
	{ {0xFF,0x3F,0x00,0x00,0xE0,0xFF,0xFF,0xFF,0xFF,0x7F}}, /* 4 */
	{ {0xFF,0xFF,0x07,0x00,0x00,0xFC,0xFF,0xFF,0xFF,0x7F}}, /* 5 */
	{ {0xFF,0xFF,0xFF,0x00,0x00,0x80,0xFF,0xFF,0xFF,0x7F}}, /* 6 */
	{ {0xFF,0xFF,0xFF,0x1F,0x00,0x00,0xF0,0xFF,0xFF,0x7F}}, /* 7 */
	{ {0xFF,0xFF,0xFF,0xFF,0x03,0x00,0x00,0xFE,0xFF,0x7F}}, /* 8 */
	{ {0xFF,0xFF,0xFF,0xFF,0x7F,0x00,0x00,0xC0,0xFF,0x7F}}, /* 9 */
	{ {0xFF,0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x00,0xF8,0x7F}}, /* 10 */
	{ {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x00,0x7F}}, /* 11 */
	{ {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3F,0x00,0x00,0x60}}, /* 12 */
	{ {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,0x00,0x00}}, /* 13 */
	{ {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x00}}, /* 14 */
};

#define AFH_COMMAND_COMPLETE_TIMEOUT_MS 2000

static int LookUpChannel(int FreqMhz)
{
	int i;

	if (FreqMhz == 0) {
		/* not connected */
		return 0;
	}

	for (i = 0; i < MAX_WLAN_CHANNELS; i++) {
		if (FreqMhz <= g_ChannelTable[i].Center) {
			break;
		}
	}
	return (i < MAX_WLAN_CHANNELS) ? g_ChannelTable[i].ChannelNumber : 0;
}
#ifdef MULTI_WLAN_CHAN_SUPPORT
static A_STATUS IssueAFHChannelClassification(ABF_BT_INFO *pAbfBtInfo,
					ABF_WLAN_INFO *pAbfWlanInfo)
{
	A_UCHAR     evtBuffer[MAX_HCI_EVENT_SIZE];
	A_STATUS    status;
	A_UCHAR     *eventPtr;
	int         eventLen;
	A_UCHAR     pChannelMap[AFH_CHANNEL_MAP_BYTES];
	ABF_WLAN_CONN_IF *conn = NULL;
	int channel, mapByte;

	/*init pChannelMap with all channels*/
	A_MEMCPY(pChannelMap, g_ChannelMapTable[0].Map, AFH_CHANNEL_MAP_BYTES);

	conn = pAbfWlanInfo->connIf;
	while (conn) {
		A_INFO("WLAN Operating Channel: %d \n", conn->channel);
		if (conn->channel >= 2412) {
			/* convert Mhz into a channel number */
			channel = LookUpChannel(conn->channel);
		} else {
			return A_ERROR;
		}

		/*Eliminate the channels*/
		for (mapByte = 0; mapByte < AFH_CHANNEL_MAP_BYTES; mapByte++)
			pChannelMap[mapByte] &=
				g_ChannelMapTable[channel].Map[mapByte];

		conn = conn->next;
	}
	A_DUMP_BUFFER(pChannelMap, AFH_CHANNEL_MAP_BYTES,
		"AFH Channel Classification Map");

	do {

		status = IssueHCICommand(pAbfBtInfo,
				HCICMD_OPCODE(3,0x3F),
				pChannelMap,
				AFH_CHANNEL_MAP_BYTES,
				AFH_COMMAND_COMPLETE_TIMEOUT_MS,
				evtBuffer,
				sizeof(evtBuffer),
				&eventPtr,
				&eventLen);


		if (A_FAILED(status)) {
			break;
		}

		status = A_ERROR;

		if (eventPtr == NULL) {
			A_ERR("[%s] Failed to capture AFH command complete event \n", __FUNCTION__);
			break;
		}

		if (eventLen < (int)sizeof(commad_complete_event) + 1) {
			A_ERR("[%s] not enough bytes in AFH command complete event %d \n", __FUNCTION__, eventLen);
			break;
		}

		/* check status parameter that follows the command complete event body */
		if (eventPtr[sizeof(commad_complete_event)] != 0) {
			A_ERR("[%s] AFH command complete event indicated failure : %d \n", __FUNCTION__,
					eventPtr[sizeof(commad_complete_event)]);
			break;
		}

		A_INFO(" AFH Command successfully issued \n");
		//A_DUMP_BUFFER(pChannelMap, AFH_CHANNEL_MAP_BYTES, "AFH Channel Classification Map");

		status = A_OK;

	} while (FALSE);

	return status;
}
#else
static A_STATUS IssueAFHChannelClassification(ABF_BT_INFO *pAbfBtInfo, int CurrentWLANChannel)
{
	A_UCHAR     evtBuffer[MAX_HCI_EVENT_SIZE];
	A_STATUS    status;
	A_UCHAR     *eventPtr;
	int         eventLen;
	A_UCHAR     *pChannelMap;

	A_INFO("WLAN Operating Channel: %d \n", CurrentWLANChannel);

	if (CurrentWLANChannel > MAX_WLAN_CHANNELS) {
		/* check if this is expressed in Mhz */
		if (CurrentWLANChannel >= 2412) {
			/* convert Mhz into a channel number */
			CurrentWLANChannel = LookUpChannel(CurrentWLANChannel);
		} else {
			return A_ERROR;
		}
	}

	pChannelMap = &(g_ChannelMapTable[CurrentWLANChannel].Map[0]);

	do {

		status = IssueHCICommand(pAbfBtInfo,
				HCICMD_OPCODE(3,0x3F),
				pChannelMap,
				AFH_CHANNEL_MAP_BYTES,
				AFH_COMMAND_COMPLETE_TIMEOUT_MS,
				evtBuffer,
				sizeof(evtBuffer),
				&eventPtr,
				&eventLen);


		if (A_FAILED(status)) {
			break;
		}

		status = A_ERROR;

		if (eventPtr == NULL) {
			A_ERR("[%s] Failed to capture AFH command complete event \n", __FUNCTION__);
			break;
		}

		if (eventLen < (int)sizeof(commad_complete_event) + 1) {
			A_ERR("[%s] not enough bytes in AFH command complete event %d \n", __FUNCTION__, eventLen);
			break;
		}

		/* check status parameter that follows the command complete event body */
		if (eventPtr[sizeof(commad_complete_event)] != 0) {
			A_ERR("[%s] AFH command complete event indicated failure : %d \n", __FUNCTION__,
					eventPtr[sizeof(commad_complete_event)]);
			break;
		}

		A_INFO(" AFH Command successfully issued \n");
		//A_DUMP_BUFFER(pChannelMap, AFH_CHANNEL_MAP_BYTES, "AFH Channel Classification Map");

		status = A_OK;

	} while (FALSE);

	return status;
}
#endif

#ifdef MULTI_WLAN_CHAN_SUPPORT
void IndicateCurrentWLANOperatingChannel(ATHBT_FILTER_INFO *pFilterInfo)
{
        ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)pFilterInfo->pBtInfo;
	ABF_WLAN_INFO *pAbfWlanInfo = (ABF_WLAN_INFO *)pFilterInfo->pWlanInfo;

        if (NULL == pAbfBtInfo) {
                return;
        }

        if (pFilterInfo->Flags & ABF_ENABLE_AFH_CHANNEL_CLASSIFICATION) {
               IssueAFHChannelClassification(pAbfBtInfo, pAbfWlanInfo);
        }
}

#else
void IndicateCurrentWLANOperatingChannel(ATHBT_FILTER_INFO *pFilterInfo, int CurrentWLANChannel)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)pFilterInfo->pBtInfo;
	ATHBT_FILTER_INFO *pInfo = pAbfBtInfo->pInfo;

	if (NULL == pAbfBtInfo) {
		return;
	}

	if (pFilterInfo->Flags & ABF_ENABLE_AFH_CHANNEL_CLASSIFICATION) {
		IssueAFHChannelClassification(pAbfBtInfo,CurrentWLANChannel);
	}

	if(pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING) {
		Abf_IssueAFHViaHciLib(pAbfBtInfo, CurrentWLANChannel);
	}
}
#endif

static void *HCIFilterThread(void *arg)
{
	ABF_BT_INFO            *pAbfBtInfo = (ABF_BT_INFO *)arg;
	ATHBT_FILTER_INFO      *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;
	A_UINT8                 buffer[300] = {0};
	A_UINT8                 *pBuffer;
	int                     eventLen;
	struct sigaction        actions;
	struct pollfd pfd;
	int poll_result;

	A_INFO("[%s] starting up \n", __FUNCTION__);

	memset(&actions, 0, sizeof(actions));
	sigemptyset(&actions.sa_mask);
	actions.sa_flags = 0;
	actions.sa_handler = thread_exit_handler;

	if (sigaction(SIGUSR1, &actions, NULL) < 0) {
		A_ERR("Error in sigaction in %s:  %s\n", __func__, strerror(errno));
		return NULL;
	}

	while (1) {

		pBuffer = buffer;

		if (pAbfBtInfo->HCIFilterThreadShutdown) {
			break;
		}

		A_MEMZERO(&pfd,sizeof(pfd));
		pfd.fd = pAbfBtInfo->HCIEventListenerSocket;
		pfd.events = POLLIN;

		poll_result = poll(&pfd, 1, 2000);
		if (poll_result < 0) {
			A_INFO("%s: poll failed: %s\n", __func__, strerror(errno));
			break;
		}
		if (poll_result == 0) {
			//A_INFO("%s: poll timeout\n", __func__);
			continue;
		}
		if (!(pfd.revents & POLLIN)) {
			A_INFO("%s: POLLIN check failed\n", __func__);
			continue;
		}

		if (disallow_hcithread_read) {
			A_INFO("%s: disallow hci thread to read, continue\n", __func__);
			continue;
		}

		/* get the packet */
		eventLen = read(pAbfBtInfo->HCIEventListenerSocket, pBuffer, sizeof(buffer));

		if (eventLen < 0) {
			if (!pAbfBtInfo->HCIFilterThreadShutdown) {
				A_ERR("[%s] socket error %d \n", __FUNCTION__, eventLen);
			}
			break;
		}

		if (eventLen == 0) {
			/* no event */
			continue;
		}

		if (eventLen < (1 + (int)sizeof(hci_event_header))) {
			A_ERR("[%s] Unknown receive packet! len : %d \n", __FUNCTION__, eventLen);
			continue;
		}

		A_INFO("%s: received hci event/cmd: %d", __func__, pBuffer[0]);

		/* first byte is a tag for the HCI packet type, we only care about events */
		if (pBuffer[0] == HCI_EVENT_PACKET) {
			/* pass this raw HCI event to the filter core */
			AthBtFilterHciEvent(pInstance,&pBuffer[1],eventLen - 1);
			A_UINT8 *eventCode = &pBuffer[1];
			A_ERR("event 0x%x\n", pBuffer[1]);
			/* revive deprecated "DiscoveryCompleted" signal in BlueZ 4.x */
			if (*eventCode == EVT_CODE_INQUIRY_COMPLETE) {
				A_DEBUG("Device Inquiry Completed\n");
				pAbfBtInfo->btInquiryState &= ~(1 << 0);
				if(!pAbfBtInfo->btInquiryState) {
					AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_OFF);
				}
			}
			if (*eventCode == EVT_CODE_PIN_CODE_REQUEST) {
				A_DEBUG("Pin Code Request\n");
				pAbfBtInfo->btInquiryState |= (1 << 0xF);
				AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_ON);
			}
			if (*eventCode == EVT_LINK_KEY_NOTIFICATION) {
				A_DEBUG("link key notify\n");
				pAbfBtInfo->btInquiryState &= ~(1 << 0xF);
				if(!pAbfBtInfo->btInquiryState) {
					AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_OFF);
				}
			}

			if(*eventCode == EVT_CODE_CONNECTION_COMPLETE) {
				A_DEBUG("Conn complete\n");

				/* save the connection handle from EVT_CODE_CONNECTION_COMPLETE */
				if (pBuffer[12] == 0x0) {
					A_DEBUG("sco link type\n");
					g_SCOConnHandle = letohs(pBuffer[4] | (pBuffer[5] << 8));
				}
				else {
					A_DEBUG("acl link type\n");
					g_ACLConnHandle = letohs(pBuffer[4] | (pBuffer[5] << 8));
#ifdef HID_PROFILE_SUPPORT
					g_HIDConnHandle = g_ACLConnHandle;
#endif
				}

				pAbfBtInfo->btInquiryState &= ~(1 << 2);
				if(!pAbfBtInfo->btInquiryState) {
					AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_OFF);
				}

				/* oliu: store the connection handle for each link type
				*   We don't need to clear the storage as every new connection should
				*   have its own connection complete event.
				*/
			}

			if (*eventCode == EVT_CODE_CONNECTION_REQUEST) {
				A_DEBUG("Conn request\n");
				/* record BD_ADDR of remote device */
				bdaddr_to_str((const bd_addr_t *)&pBuffer[3],
					&pAbfBtInfo->DefaultRemoteAudioDeviceAddress[0]);
			}

			if (*eventCode == EVT_CODE_DISCONNECTION_REQUEST) {
#ifdef HID_PROFILE_SUPPORT
				A_UINT16 handle;
				handle = letohs(pBuffer[4] | (pBuffer[5] << 8));
                                /* check LE & HID tables and remove the record by handle */
				DisconnHIDDevice(handle);

				if (HIDDeviceAllDisconnect()) {
					A_INFO("All HID devices disconncted!");
					AthBtIndicateState(pInstance,
					ATH_BT_HID, STATE_OFF);
				}
#endif
			}

			/* revive deprecated "DiscoveryStarted" signal by capturing INQUIRY commands */
		} else if (pBuffer[0] == HCI_COMMAND_PACKET) {
			A_UINT16 *packedOpCode = (A_UINT16 *)&pBuffer[1];
			if(HCICMD_OGF(*packedOpCode) == 0x1 &&
					HCICMD_OCF(*packedOpCode) == 0x5 )
			{
				A_DEBUG("Bt-Connect\n");
				pAbfBtInfo->btInquiryState |= (1 << 2);
				AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_ON);
			}

			if (*packedOpCode == HCI_RESET_CMD) {
				Abf_WlanIssueBtOnOff(pInfo, STATE_OFF);
				A_INFO("HCI command reset\n");
				Abf_BToff();
			}

			if (*packedOpCode ==
					HCICMD_OPCODE(OGF_LINK_CONTROL,
						OCF_INQUIRY_CANCEL)) {
				A_DEBUG("Device Inquiry Completed\n");
				pAbfBtInfo->btInquiryState &= ~(1 << 0);
				if (!pAbfBtInfo->btInquiryState)
					AthBtIndicateState(pInstance,
							ATH_BT_INQUIRY, STATE_OFF);
			}

			if (*packedOpCode == HCICMD_OPCODE(OGF_LINK_CONTROL, OCF_INQUIRY)
					|| *packedOpCode == HCICMD_OPCODE(OGF_LINK_CONTROL, OCF_PERIODIC_INQUIRY_MODE)) {
				A_DEBUG("Device Inquiry Started\n");
				pAbfBtInfo->btInquiryState |= (1 << 0);
				AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_ON);
			} else if (*packedOpCode == HCICMD_OPCODE(OGF_LINK_CONTROL, OCF_CREATE_CONNECTION)) {
				/* "Connected" signal is deprecated and won't record BD_ADDR of remote device connected */
				bdaddr_to_str((const bd_addr_t *)&pBuffer[4],
						&pAbfBtInfo->DefaultRemoteAudioDeviceAddress[0]);
			}
		} else {
			A_ERR("[%s] Unsupported packet type : %d \n", __FUNCTION__, buffer[0]);
			continue;
		}
	}

	A_INFO("[%s] exiting \n", __FUNCTION__);

	return NULL;
}

/*===========================================================================

FUNCTION     : thread_exit_handler

============================================================================*/
static void thread_exit_handler(int sig)
{
	if(sig == SIGUSR1)
		pthread_exit(0);

	A_ERR("%s: Error in thread exit handler! Sig value not SIGUSR1", __func__);

	return;
}

/*===========================================================================

FUNCTION     : Bluedroid_stack_event

DESCRIPTION  : This function connects to BTC socket exposed from Java and read
               the Stack/Profile related events

============================================================================*/
static void *Bluedroid_stack_event_thread(void *arg)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)arg;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;
	int event;
	int eventLen;
	int i;
	struct sigaction actions;

	A_INFO("[%s] starting up \n", __FUNCTION__);

	memset(&actions, 0, sizeof(actions));
	sigemptyset(&actions.sa_mask);
	actions.sa_flags = 0;
	actions.sa_handler = thread_exit_handler;

	if (sigaction(SIGUSR1, &actions, NULL) < 0) {
		A_ERR("Error in sigaction in %s:  %s\n", __func__, strerror(errno));
		return NULL;
	}

	while (1) {
#if 0
		if (pAbfBtInfo->HCIFilterThreadShutdown) {
			break;
		}
#endif
		/* todo: get the bluedroid stack event*/
		eventLen = read(pAbfBtInfo->StackEventSocket, &event, sizeof(event));

		if (eventLen < 0) {
#if 0
			if (!pAbfBtInfo->HCIFilterThreadShutdown) {
				A_ERR("[%s] socket error %d \n", __FUNCTION__, eventLen);
			}
#endif
			break;
		}

		if (eventLen == 0) {
			/* no event */
			continue;
		}

		/* TODO: check if this htonl is needed */
		/* event = htonl(event); */
		event = ntohl(event);
		A_INFO("%s: receved event: 0x%x", __func__, event);

		i = 0;
		/* todo: assume the event num is in pBuffer[0] */
		while (g_NotificationConfig[i].signal_num != 0) {
			if ((g_NotificationConfig[i].signal_num == event) && g_NotificationConfig[i].signal_handler) {
				(*g_NotificationConfig[i].signal_handler)(NULL, (ABF_BT_INFO *)pAbfBtInfo);
				break;
			}
			i++;
			if (g_NotificationConfig[i].signal_num == 0) {
				A_ERR("[%s] not supported event: %d\n", __FUNCTION__, event);
			}
		}
	}

	A_INFO("[%s] exiting \n", __FUNCTION__);

	return NULL;
}

/* =========================================================================
FUNCTION     : start_monitor_stack_events

DESCRIPTION  :

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
A_STATUS start_monitor_stack_events(void *user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	struct sockaddr_un addr;
	socklen_t len;
	A_STATUS status;
	A_UINT16 retry_cnt = 0;

	A_INFO("[%s] enter\n", __func__);

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_LOCAL;
	strlcpy(&addr.sun_path[1], STACK_SOCKET_NAME, sizeof(addr.sun_path) - 1);
	len = offsetof(struct sockaddr_un, sun_path) + 1 + strlen(&addr.sun_path[1]);

	pAbfBtInfo->StackEventSocket = socket(PF_LOCAL, SOCK_STREAM, 0);

	if(pAbfBtInfo->StackEventSocket < 0) {
		A_ERR("Failed to get BTCServer socket: %s", __func__);
		return A_ERROR;
	}

	while (connect(pAbfBtInfo->StackEventSocket, (struct sockaddr *) &addr, len) < 0 &&
		retry_cnt < MAX_SOCKET_RETRY_CNT && !BT_terminated()) {
		retry_cnt++;
		A_INFO("%s: connect() retry %d times\n", __func__, retry_cnt);
		usleep(500000);
	}
	if (retry_cnt == MAX_SOCKET_RETRY_CNT || BT_terminated()) {
		A_ERR("%s: connect() failed: %s (%d)\n",
			__func__, strerror(errno), errno);
		close(pAbfBtInfo->StackEventSocket);
		return A_ERROR;
	}

	/* spawn a thread that will capture Stack events from the adapter */
	status = A_TASK_CREATE(&pAbfBtInfo->hBtStackEventThread, Bluedroid_stack_event_thread, pAbfBtInfo);
	if (A_FAILED(status)) {
		A_ERR("[%s] Failed to spawn a BT thread to monitor stack events\n", __FUNCTION__);
		return A_ERROR;
	}

	return A_OK;
}

/* =========================================================================
FUNCTION     : stop_monitor_thread

DESCRIPTION  :

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/

void stop_monitor_stack_events(void *user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	A_STATUS status;

	if (pAbfBtInfo->StackEventSocket >= 0) {
		A_INFO("%s stop!\n", __FUNCTION__);
		/* close socket, if there is a thread waiting on this socket, it will error and then exit */
		close(pAbfBtInfo->StackEventSocket);
		pAbfBtInfo->StackEventSocket = -1;

		/* wait for thread to exit
		 * note: JOIN cleans up thread resources as per POSIX spec. */
		status = A_TASK_JOIN(&pAbfBtInfo->hBtStackEventThread);
		if (A_FAILED(status)) {
			A_ERR("[%s] Failed to JOIN HCI filter thread \n",
				__FUNCTION__);
		}
		A_MEMZERO(&pAbfBtInfo->hBtStackEventThread, sizeof(pAbfBtInfo->hBtStackEventThread));
	}
}

#else /* BLUETOOTH_BLUEDROID */

#include "alljoyn_dbus.h"

#undef HCI_INQUIRY
#include <bluetooth.h>
#include <hci.h>
#include <hci_lib.h>
#include <sys/poll.h>
#include <sys/uio.h>

/* Definitions */

#ifndef ANDROID
/* This should be undef'ed if compiled at FC9 using BlueZ 3.x */
#define BLUEZ4_3
#endif /* ANDROID */

#define ABTH_MAX_CONNECTIONS 16

#define INVALID_INTERFACE                 NULL

#define BTEV_GET_BT_CONN_LINK_TYPE(p)   ((p)[9])
#define BTEV_GET_TRANS_INTERVAL(p)      ((p)[10])
#define BTEV_GET_RETRANS_INTERVAL(p)    ((p)[11])
#define BTEV_GET_RX_PKT_LEN(p)          ((A_UINT16)((p)[12]) | (((A_UINT16)((p)[13])) << 8))
#define BTEV_GET_TX_PKT_LEN(p)          ((A_UINT16)((p)[14]) | (((A_UINT16)((p)[15])) << 8))
#define BTEV_CMD_COMPLETE_GET_OPCODE(p) ((A_UINT16)((p)[1]) | (((A_UINT16)((p)[2])) << 8))
#define BTEV_CMD_COMPLETE_GET_STATUS(p) ((p)[3])

#define DBUS_METHOD_CALL_TIMEOUT   (-1)   /* no timeout */
#define DBUS_MESSAGE_RECV_TIMEOUT  (-1)   /* no timeout */

#define USE_DBUS_FOR_HEADSET_PROFILE(pInfo) (!((pInfo)->Flags & ABF_USE_HCI_FILTER_FOR_HEADSET_PROFILE))
#define MAKE_BTSTATE_MASK(state) (1 << (state))
#define HCI_RESET_CMD 0x0c03

extern int alljoyn_init(void *pAbtInfo);
extern void alljoyn_deinit();
extern void alljoyn_register_signal(char *signal, char *interface);
extern void alljoyn_deregister_signal(char *signal, char *interface);

static inline void bdaddr_to_str(const bdaddr_t *src, char *dst)
{
	snprintf(dst, 17, "%X:%X:%X:%X:%X:%X", src->b[5],
			src->b[4], src->b[3], src->b[2], src->b[1],
			src->b[0]);
}

typedef void (*signal_handle_t)(void *arg, void *data);

typedef struct _BT_NOTIFICATION_CONFIG_PARAMS {
	char *signal_name;
	char *interface;
	signal_handle_t signal_handler;
} BT_NOTIFICATION_CONFIG_PARAMS;

#define REGISTER_SIGNAL(signal_index) alljoyn_register_signal(g_NotificationConfig[(signal_index)].signal_name, \
						g_NotificationConfig[(signal_index)].interface);
#define DEREGISTER_SIGNAL(signal_index) alljoyn_deregister_signal(g_NotificationConfig[(signal_index)].signal_name, \
						g_NotificationConfig[(signal_index)].interface);

/* Function Prototypes */
static void BtAdapterAdded(void *string,
		void * user_data);
static void BtAdapterRemoved(void *string,
		void * user_data);
static A_STATUS AcquireBtAdapter(ABF_BT_INFO *pAbfBtInfo);
static void ReleaseBTAdapter(ABF_BT_INFO *pAbfBtInfo);
static A_STATUS GetAdapterInfo(ABF_BT_INFO *pAbfBtInfo);
static void RemoteDeviceDisconnected(void *string,
		void * user_data);
static void RemoteDeviceConnected(void *string,
		void * user_data);
static void AudioDeviceAdded(void *string,
		void * user_data);
static void AudioDeviceRemoved(void *string,
		void * user_data);
static void DeviceDiscoveryStarted(void *arg, void * user_data);
static void DeviceDiscoveryFinished(void *arg, void * user_data);
static void AudioHeadsetConnected(void *arg, void * user_data);
static void AudioHeadsetDisconnected(void *arg, void * user_data);
static void AudioHeadsetStreamStarted(void *arg, void * user_data);
static void AudioHeadsetStreamStopped(void *arg, void * user_data);
static void AudioGatewayConnected(void *arg, void * user_data);
static void AudioGatewayDisconnected(void *arg, void * user_data);
static void AudioSinkConnected(void *arg, void * user_data);
static void AudioSinkDisconnected(void *arg, void * user_data);
static void AudioSinkStreamStarted(void *arg, void * user_data);
static void AudioSinkStreamStopped(void *arg, void * user_data);
static void AudioSourceConnected(void *arg, void * user_data);
static void AudioSourceDisconnected(void *arg, void * user_data);
#ifdef HID_PROFILE_SUPPORT
static void InputDevicePropertyChanged(void *arg, void * user_data);
#endif
static A_STATUS CheckAndAcquireDefaultAdapter(ABF_BT_INFO *pAbfBtInfo);
static void ReleaseDefaultAdapter(ABF_BT_INFO *pAbfBtInfo);
static void AcquireDefaultAudioDevice(ABF_BT_INFO *pAbfBtInfo);
static void ReleaseDefaultAudioDevice(ABF_BT_INFO *pAbfBtInfo);
static void GetBtAudioConnectionProperties(ABF_BT_INFO              *pAbfBtInfo,
		ATHBT_STATE_INDICATION   Indication);
static void GetBtAudioDeviceProperties(ABF_BT_INFO  *pAbfBtInfo);

static A_STATUS SetupHciEventFilter(ABF_BT_INFO *pAbfBtInfo);
static void CleanupHciEventFilter(ABF_BT_INFO *pAbfBtInfo);
static void CheckHciEventFilter(ABF_BT_INFO   *pAbfBtInfo);

static A_STATUS IssueHCICommand(ABF_BT_INFO *pAbfBtInfo,
		A_UINT16    OpCode,
		A_UCHAR     *pCmdData,
		int         CmdLength,
		int         EventRecvTimeoutMS,
		A_UCHAR     *pEventBuffer,
		int         MaxLength,
		A_UCHAR     **ppEventPtr,
		int         *pEventLength);

static A_STATUS CheckRemoteDeviceEDRCapable(ABF_BT_INFO *pAbfBtInfo, A_BOOL *pEDRCapable);

/* New function to check Remote LMP version */
static A_STATUS GetRemoteDeviceLMPVersion(ABF_BT_INFO *pAbfBtInfo);

static void *HCIFilterThread(void *arg);

A_BOOL g_AppShutdown = FALSE;

static BT_NOTIFICATION_CONFIG_PARAMS g_NotificationConfig[BT_EVENTS_NUM_MAX] =
{
	/* BT_ADAPTER_ADDED */
	{"AdapterAdded", MANAGER_INTERFACE, BtAdapterAdded},
	/* BT_ADAPTER_REMOVED */
	{"AdapterRemoved", MANAGER_INTERFACE, BtAdapterRemoved},
	/* DEVICE_DISCOVERY_STARTED */
	{"DiscoveryStarted", ADAPTER_INTERFACE, DeviceDiscoveryStarted},
	/* DEVICE_DISCOVERY_FINISHED */
	{"DiscoveryCompleted", ADAPTER_INTERFACE, DeviceDiscoveryFinished},
	/* REMOTE_DEVICE_CONNECTED */
	{"RemoteDeviceConnected", ADAPTER_INTERFACE, RemoteDeviceConnected},
	/* REMOTE_DEVICE_DISCONNECTED */
	{"RemoteDeviceDisconnected", ADAPTER_INTERFACE, RemoteDeviceDisconnected},
	/* AUDIO_DEVICE_ADDED */
	{"DeviceCreated", AUDIO_MANAGER_INTERFACE, AudioDeviceAdded},
	/* AUDIO_DEVICE_REMOVED */
	{"DeviceRemoved", AUDIO_MANAGER_INTERFACE, AudioDeviceRemoved},
	/* AUDIO_HEADSET_CONNECTED */
	{"Connected", AUDIO_HEADSET_INTERFACE, AudioHeadsetConnected},
	/* AUDIO_HEADSET_DISCONNECTED */
	{"Disconnected", AUDIO_HEADSET_INTERFACE, AudioHeadsetDisconnected},
	/* AUDIO_HEADSET_STREAM_STARTED */
	{"Playing", AUDIO_HEADSET_INTERFACE, AudioHeadsetStreamStarted},
	/* AUDIO_HEADSET_STREAM_STOPPED */
	{"Stopped", AUDIO_HEADSET_INTERFACE, AudioHeadsetStreamStopped},
	/* AUDIO_SINK_CONNECTED */
	{"Connected", AUDIO_SINK_INTERFACE, AudioSinkConnected},
	/* AUDIO_SINK_DISCONNECTED */
	{"Disconnected", AUDIO_SINK_INTERFACE, AudioSinkDisconnected},
	/* AUDIO_SINK_STREAM_STARTED */
	{"Playing", AUDIO_SINK_INTERFACE, AudioSinkStreamStarted},
	/* AUDIO_SINK_STREAM_STOPPED */
	{"Stopped", AUDIO_SINK_INTERFACE, AudioSinkStreamStopped},
#ifdef HID_PROFILE_SUPPORT
	/* INPUT_DEVICE_CONNECTED */
	{"PropertyChanged", INPUT_DEVICE_INTERFACE, InputDevicePropertyChanged},
#endif
};

#define ForgetRemoteAudioDevice(pA)     \
{                                       \
	A_MEMZERO((pA)->DefaultRemoteAudioDeviceAddress,sizeof((pA)->DefaultRemoteAudioDeviceAddress)); \
	(pA)->DefaultRemoteAudioDevicePropsValid = FALSE;                                               \
}

/* APIs exported to other modules */
A_STATUS Abf_BtStackNotificationInit(ATH_BT_FILTER_INSTANCE *pInstance,
									 A_UINT32 Flags)
{
	A_STATUS status = A_ERROR;
	ATHBT_FILTER_INFO *pInfo;
	ABF_BT_INFO *pAbfBtInfo;

	pInfo = (ATHBT_FILTER_INFO *)pInstance->pContext;
	if (pInfo->pBtInfo) {
		return A_OK;
	}

	if (g_AppShutdown) {
		A_ERR (" App was already shutdown cannot call Abf_BtStackNotificationInit again!!!");
		return A_ERROR;
	}

	pAbfBtInfo = (ABF_BT_INFO *)A_MALLOC(sizeof(ABF_BT_INFO));
	if (!pAbfBtInfo) {
		A_ERR (" Fail to allocate memory!");
		return A_NO_MEMORY;
	}
	A_MEMZERO(pAbfBtInfo,sizeof(ABF_BT_INFO));

	pInfo->Flags = Flags;

	if (pInfo->Flags & ABF_ENABLE_AFH_CHANNEL_CLASSIFICATION) {
		A_INFO("AFH Classification Command will be issued on WLAN connect/disconnect \n");
	}

	if (pInfo->Flags & ABF_USE_HCI_FILTER_FOR_HEADSET_PROFILE) {
		A_INFO("Headset Profile notifications will use HCI filter instead of DBUS \n");

#ifdef BLUEZ4_3
		/* We don't want to ignore INQUIRY message to implement "DiscoveryStarted" (deprecated in BlueZ 4.x) */
		pInfo->FilterCore.StateFilterIgnore = MAKE_BTSTATE_MASK(ATH_BT_A2DP); //| MAKE_BTSTATE_MASK(ATH_BT_CONNECT);
#else
		/* ignore certain state detections that we can handle through dbus */
		pInfo->FilterCore.StateFilterIgnore = MAKE_BTSTATE_MASK(ATH_BT_CONNECT) |
			MAKE_BTSTATE_MASK(ATH_BT_INQUIRY) |
			MAKE_BTSTATE_MASK(ATH_BT_A2DP);
#endif

	}

	pAbfBtInfo->AdapterAvailable = FALSE;
	pAbfBtInfo->pInfo = pInfo;
	pAbfBtInfo->HCIEventListenerSocket = -1;
	pInfo->pBtInfo = pAbfBtInfo;
	A_INFO("before init %s:%d\n", __FUNCTION__, __LINE__);

	alljoyn_init((void *)pAbfBtInfo);
	A_INFO("After init %s:%d\n", __FUNCTION__, __LINE__);

	/* check for default adapter at startup */
	status = CheckAndAcquireDefaultAdapter(pAbfBtInfo);
	if (A_FAILED(status)) {
		Abf_BtStackNotificationDeInit(pInstance);
		A_INFO("Fail to acquire default adapter\n");
	} else {
		REGISTER_SIGNAL(BT_ADAPTER_ADDED);
		REGISTER_SIGNAL(BT_ADAPTER_REMOVED);
		status = A_OK;
		A_INFO("Init complete:%s:%d\n", __FUNCTION__, __LINE__);
		A_INFO("BT Stack Notification init complete\n");
	}

	return status;
}

void Abf_BtStackNotificationDeInit(ATH_BT_FILTER_INSTANCE *pInstance)
{
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pInstance->pContext;
	ABF_BT_INFO *pAbfBtInfo = pInfo->pBtInfo;

	g_AppShutdown = TRUE;

	if (!pAbfBtInfo)
		return;

	/* Free the remaining resources */
	pAbfBtInfo->AdapterAvailable = FALSE;
	pInfo->pBtInfo = NULL;
	A_FREE(pAbfBtInfo);
	alljoyn_deinit();

	A_INFO("BT Stack Notification de-init complete\n");
}

static A_STATUS CheckAndAcquireDefaultAdapter(ABF_BT_INFO *pAbfBtInfo)
{
	A_STATUS status = A_OK;

	do {
		if (pAbfBtInfo->AdapterAvailable) {
			/* already available */
			break;
		}
		/* acquire the adapter */
		status = AcquireBtAdapter(pAbfBtInfo);

	} while (FALSE);

	return status;
}

static void ReleaseDefaultAdapter(ABF_BT_INFO *pAbfBtInfo)
{

	if (pAbfBtInfo->AdapterAvailable) {
		/* Release the BT adapter */
		ReleaseBTAdapter(pAbfBtInfo);
		A_INFO("BT Adapter Removed\n");
	}
}

/* Event Notifications */
static void BtAdapterAdded(void *string, void * user_data)
{
	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);

	sleep(5);
	CheckAndAcquireDefaultAdapter((ABF_BT_INFO *)user_data);
}

static void BtAdapterRemoved(void *string, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;

	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);

	if (!pAbfBtInfo->AdapterAvailable)
		return;

	ReleaseDefaultAdapter(pAbfBtInfo);
}

static void DeviceDiscoveryStarted(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);
	AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_ON);
}

static void DeviceDiscoveryFinished(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);
	AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_OFF);
}

static void RemoteDeviceConnected(void *string, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);
	A_STR2ADDR(string, pAbfBtInfo->RemoteDevice);
	AthBtIndicateState(pInstance, ATH_BT_CONNECT, STATE_ON);
}

static void RemoteDeviceDisconnected(void *string, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);
	A_MEMZERO(pAbfBtInfo->RemoteDevice, sizeof(pAbfBtInfo->RemoteDevice));
	AthBtIndicateState(pInstance, ATH_BT_CONNECT, STATE_OFF);
}

static void ReleaseDefaultAudioDevice(ABF_BT_INFO *pAbfBtInfo)
{
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;

	A_INFO("%s:%d\n", __FUNCTION__, __LINE__);
	if (pAbfBtInfo->AudioCbRegistered) {
		DEREGISTER_SIGNAL(AUDIO_HEADSET_CONNECTED);
		DEREGISTER_SIGNAL(AUDIO_HEADSET_DISCONNECTED);
		DEREGISTER_SIGNAL(AUDIO_HEADSET_STREAM_STARTED);
		DEREGISTER_SIGNAL(AUDIO_HEADSET_STREAM_STOPPED);
		DEREGISTER_SIGNAL(AUDIO_SINK_CONNECTED);
		DEREGISTER_SIGNAL(AUDIO_SINK_DISCONNECTED);
		DEREGISTER_SIGNAL(AUDIO_SINK_STREAM_STARTED);
		DEREGISTER_SIGNAL(AUDIO_SINK_STREAM_STOPPED);
		pAbfBtInfo->AudioCbRegistered = FALSE;
	}
}

static void AcquireDefaultAudioDevice(ABF_BT_INFO *pAbfBtInfo)
{
	A_BOOL     success = FALSE;
#ifndef BLUEZ4_3
	A_STATUS   status;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
#endif

	A_INFO("[%s]StartRegister\n", __FUNCTION__);
	do {

		if (pAbfBtInfo->DefaultAudioDeviceAvailable) {
			/* already acquired */
			success = TRUE;
			break;
		}
		pAbfBtInfo->DefaultAudioDeviceAvailable = FALSE;

		/* Register for audio specific events */
		REGISTER_SIGNAL(AUDIO_HEADSET_CONNECTED);
		REGISTER_SIGNAL(AUDIO_HEADSET_DISCONNECTED);
		REGISTER_SIGNAL(AUDIO_HEADSET_STREAM_STARTED);
		REGISTER_SIGNAL(AUDIO_HEADSET_STREAM_STOPPED);
		REGISTER_SIGNAL(AUDIO_SINK_CONNECTED);
		REGISTER_SIGNAL(AUDIO_SINK_DISCONNECTED);
		REGISTER_SIGNAL(AUDIO_SINK_STREAM_STARTED);
		REGISTER_SIGNAL(AUDIO_SINK_STREAM_STOPPED);

		pAbfBtInfo->AudioCbRegistered = TRUE;

		success = TRUE;

		A_INFO("[%s]EndRegister\n", __FUNCTION__);
	} while (FALSE);

	if (!success) {
		/* cleanup */
		ReleaseDefaultAudioDevice(pAbfBtInfo);
	}
}

static void AudioDeviceAdded(void *string, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;

	A_INFO("Audio Device Added: %s\n", string);
	/* release current one if any */
	ReleaseDefaultAudioDevice(pAbfBtInfo);
	/* re-acquire the new default, it could be the same one */
	AcquireDefaultAudioDevice(pAbfBtInfo);
}

static void AudioDeviceRemoved(void *string, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;

	A_INFO("Audio Device Removed: %s\n", string);
	/* release current one  */
	ReleaseDefaultAudioDevice(pAbfBtInfo);
	/* re-acquire the new default (if any) */
	AcquireDefaultAudioDevice(pAbfBtInfo);
}

static void AudioHeadsetConnected(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;
	A_INFO("Audio Headset Connected \n");

	if(!(pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING)) {
		GetBtAudioDeviceProperties(pAbfBtInfo);
	}

	/* Turn off inquiry once connected */
	if (pAbfBtInfo->btInquiryState & 0x1) {
		pAbfBtInfo->btInquiryState &= ~(1 << 0);
		AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_OFF);
	}
}

static void AudioHeadsetDisconnected(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO  *)user_data;

	A_INFO("Audio Headset (%s) Disconnected\n",pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
	ForgetRemoteAudioDevice(pAbfBtInfo);
}

static void AudioHeadsetStreamStarted(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("Audio Headset (%s) Stream Started \n",pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
	if(!(pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING)) {
		if (!pAbfBtInfo->DefaultRemoteAudioDevicePropsValid) {
			GetBtAudioDeviceProperties(pAbfBtInfo);
		}
	}
	/* make the indication */
	/* get properties of this headset connection */
	GetBtAudioConnectionProperties(pAbfBtInfo, ATH_BT_SCO);

	AthBtIndicateState(pInstance,
			pAbfBtInfo->CurrentSCOLinkType == SCO_LINK ? ATH_BT_SCO : ATH_BT_ESCO,
			STATE_ON);
}

static void AudioHeadsetStreamStopped(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	/* This event can also be used to indicate the SCO state */
	A_INFO("Audio Headset (%s) Stream Stopped \n", pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
	AthBtIndicateState(pInstance,
			pAbfBtInfo->CurrentSCOLinkType == SCO_LINK ? ATH_BT_SCO : ATH_BT_ESCO,
			STATE_OFF);
}

static void AudioSinkConnected(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;
	A_INFO("Audio Sink Connected \n");

	if(!(pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING)) {
		GetBtAudioDeviceProperties(pAbfBtInfo);
	}

	/* Turn off inquiry once connected */
	if (pAbfBtInfo->btInquiryState & 0x1) {
		pAbfBtInfo->btInquiryState &= ~(1 << 0);
		AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_OFF);
	}
}

static void AudioSinkDisconnected(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("Audio Sink (%s) Disconnected \n",pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
	AthBtIndicateState(pInstance, ATH_BT_A2DP, STATE_OFF);
	ForgetRemoteAudioDevice(pAbfBtInfo);
}

static void AudioSinkStreamStarted(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	if (!pAbfBtInfo->DefaultRemoteAudioDevicePropsValid) {
		GetBtAudioDeviceProperties(pAbfBtInfo);
	}

	A_INFO("Audio Sink (%s) Stream Started \n",pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
	/* get connection properties */
	GetBtAudioConnectionProperties(pAbfBtInfo, ATH_BT_A2DP);
	AthBtIndicateState(pInstance, ATH_BT_A2DP, STATE_ON);
}

static void AudioSinkStreamStopped(void *arg, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	A_INFO("Audio Sink (%s) Stream Stopped \n",pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
	AthBtIndicateState(pInstance, ATH_BT_A2DP, STATE_OFF);
}

#ifdef HID_PROFILE_SUPPORT
static void InputDevicePropertyChanged(void *val, void * user_data)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)user_data;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

	if (*(A_BOOL*)val) {
		A_DEBUG("HID Connected\n");
		AthBtIndicateState(pInstance, ATH_BT_HID, STATE_ON);
	}
	else {
		A_DEBUG("HID Disconnected\n");
		AthBtIndicateState(pInstance, ATH_BT_HID, STATE_OFF);
	}
}
#endif

static A_STATUS AcquireBtAdapter(ABF_BT_INFO *pAbfBtInfo)
{
	A_STATUS        status = A_ERROR;
	char            *hciName;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;

	do {

		/* assume ID 0 */
		pAbfBtInfo->AdapterId = 0;

		if(!(pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING)) {
			if (!A_SUCCESS(SetupHciEventFilter(pAbfBtInfo))) {
				break;
			}
			GetAdapterInfo(pAbfBtInfo);
		}


		pAbfBtInfo->pInfo->LMPVersion = pAbfBtInfo->HCI_LMPVersion;

		pAbfBtInfo->AdapterAvailable = TRUE;
		/* Register to get notified of different stack events */
		REGISTER_SIGNAL(DEVICE_DISCOVERY_STARTED);
		REGISTER_SIGNAL(DEVICE_DISCOVERY_FINISHED);
		REGISTER_SIGNAL(REMOTE_DEVICE_CONNECTED);
		REGISTER_SIGNAL(REMOTE_DEVICE_DISCONNECTED);
		REGISTER_SIGNAL(AUDIO_DEVICE_ADDED);
		REGISTER_SIGNAL(AUDIO_DEVICE_REMOVED);
#ifdef HID_PROFILE_SUPPORT
		REGISTER_SIGNAL(INPUT_DEVICE_PROPERTY_CHANGED);
#endif

		pAbfBtInfo->AdapterCbRegistered = TRUE;

		A_INFO("BT Adapter Added\n");

		/* acquire default audio device */
		AcquireDefaultAudioDevice(pAbfBtInfo);

		status = A_OK;

	} while (FALSE);

	return status;
}

static void ReleaseBTAdapter(ABF_BT_INFO *pAbfBtInfo)
{

	if (pAbfBtInfo->AdapterCbRegistered) {
		pAbfBtInfo->AdapterCbRegistered = FALSE;
		/* Free the resources held for the event handlers */
		DEREGISTER_SIGNAL(DEVICE_DISCOVERY_STARTED);
		DEREGISTER_SIGNAL(DEVICE_DISCOVERY_FINISHED);
		DEREGISTER_SIGNAL(REMOTE_DEVICE_CONNECTED);
		DEREGISTER_SIGNAL(REMOTE_DEVICE_DISCONNECTED);
		DEREGISTER_SIGNAL(AUDIO_DEVICE_ADDED);
		DEREGISTER_SIGNAL(AUDIO_DEVICE_REMOVED);
#ifdef HID_PROFILE_SUPPORT
		DEREGISTER_SIGNAL(INPUT_DEVICE_PROPERTY_CHANGED);
#endif
	}

	ReleaseDefaultAudioDevice(pAbfBtInfo);

	CleanupHciEventFilter(pAbfBtInfo);

	A_MEMZERO(pAbfBtInfo->HCI_DeviceAddress,
			sizeof(pAbfBtInfo->HCI_DeviceAddress));
	pAbfBtInfo->HCI_LMPVersion = 0;

	pAbfBtInfo->AdapterAvailable = FALSE;
}

static A_STATUS GetAdapterInfo(ABF_BT_INFO *pAbfBtInfo)
{

	A_STATUS status;

	int i;
	A_UCHAR eventBuffer[HCI_MAX_EVENT_SIZE];
	A_UCHAR *eventPtr;
	int eventLen;

	/* Get adapter/device address by issuing HCI command, Read_BD_ADDR */
	status = IssueHCICommand(pAbfBtInfo,
			cmd_opcode_pack(OGF_INFO_PARAM, OCF_READ_BD_ADDR),
			0,
			0,
			2000,
			eventBuffer,
			sizeof(eventBuffer),
			&eventPtr,
			&eventLen);

	if (A_FAILED(status)) {
		A_ERR("[%s] Failed to get BD_ADDR \n", __FUNCTION__);
		return status;
	}

	if (eventBuffer[6] == 0) { /* READ_BD_ADDR was a success */
		for (i = 0; i < BD_ADDR_SIZE; i++) {
			pAbfBtInfo->HCI_DeviceAddress[i] = eventBuffer[i+7];
		}
		A_DEBUG("BT-HCI Device Address: (%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X)\n",
				pAbfBtInfo->HCI_DeviceAddress[0], pAbfBtInfo->HCI_DeviceAddress[1],
				pAbfBtInfo->HCI_DeviceAddress[2], pAbfBtInfo->HCI_DeviceAddress[3],
				pAbfBtInfo->HCI_DeviceAddress[4], pAbfBtInfo->HCI_DeviceAddress[5]);
	}

	status = A_OK;

	return status;
}

static A_STATUS GetConnectedDeviceRole(ABF_BT_INFO   *pAbfBtInfo,
		A_CHAR        *Address,
		A_BOOL        IsSCO,
		A_UCHAR       *pRole)
{
	A_STATUS                    status = A_ERROR;
	struct hci_conn_list_req    *connList = NULL;
	struct hci_conn_info        *connInfo = NULL;
	int                         i, sk = -1;
	int                         len;

	do {

		sk = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);

		if (sk < 0) {
			A_ERR("[%s] Failed to get raw BT socket: %d \n", __FUNCTION__, errno);
			break;
		}

		len = (sizeof(*connInfo)) * ABTH_MAX_CONNECTIONS + sizeof(*connList);

		connList = (struct hci_conn_list_req *)A_MALLOC(len);
		if (connList == NULL) {
			break;
		}

		A_MEMZERO(connList,len);

		connList->dev_id = pAbfBtInfo->AdapterId;
		connList->conn_num = ABTH_MAX_CONNECTIONS;
		connInfo = connList->conn_info;

		if (ioctl(sk, HCIGETCONNLIST, (void *)connList)) {
			A_ERR("[%s] Failed to get connection list %d \n", __FUNCTION__, errno);
			break;
		}
		/* walk through connection list */
		for (i = 0; i < connList->conn_num; i++, connInfo++) {
			char addr[32];

			/* convert to a string to compare */
			bdaddr_to_str(&connInfo->bdaddr, addr);

			if (strcmp(addr,Address) != 0) {
				continue;
			}

			if (IsSCO) {
				/* look for first non-ACL connection */
				if (connInfo->type == ACL_LINK) {
					continue;
				}
				pAbfBtInfo->CurrentSCOLinkType = connInfo->type;
			} else {
				/* look for first ACL connection */
				if (connInfo->type != ACL_LINK) {
					continue;
				}
			}
			/* if we get here we have a connection we are interested in */
			if (connInfo->link_mode & HCI_LM_MASTER) {
				/* master */
				*pRole = 0;
			}  else {
				/* slave */
				*pRole = 1;
			}

			A_INFO("[%s] Found Connection (Link-Type : %d), found role:%d \n",
					Address, connInfo->type, *pRole);
			break;
		}

		if (i == connList->conn_num) {
			A_ERR("[%s] Could not find connection info for %s %d \n", __FUNCTION__, Address);
			break;
		}
		status = A_OK;

	} while (FALSE);

	if (sk >= 0) {
		close(sk);
	}

	if (connList != NULL) {
		A_FREE(connList);
	}

	return status;
}

static void GetBtAudioDeviceProperties(ABF_BT_INFO  *pAbfBtInfo)
{
	A_STATUS    status;

	pAbfBtInfo->DefaultRemoteAudioDevicePropsValid = FALSE;

	do {

		/* Need RemoteDeviceAddress */


		status = GetRemoteDeviceLMPVersion(pAbfBtInfo);

		/* assume 2.1 or later */
		pAbfBtInfo->DefaultAudioDeviceLmpVersion = 4;

		if (strstr(pAbfBtInfo->DefaultRemoteAudioDeviceVersion,"1.0") != NULL) {
			pAbfBtInfo->DefaultAudioDeviceLmpVersion = 0;
		} else if (strstr(pAbfBtInfo->DefaultRemoteAudioDeviceVersion,"1.1") != NULL) {
			pAbfBtInfo->DefaultAudioDeviceLmpVersion = 1;
		} else if (strstr(pAbfBtInfo->DefaultRemoteAudioDeviceVersion,"1.2") != NULL) {
			pAbfBtInfo->DefaultAudioDeviceLmpVersion = 2;
		} else if (strstr(pAbfBtInfo->DefaultRemoteAudioDeviceVersion,"2.0") != NULL) {
			/* NOTE: contrary to what the DBUS documentation says, the BT string will
			 * not indicate +EDR to indiate the remote device is EDR capable!
			 * */
			pAbfBtInfo->DefaultAudioDeviceLmpVersion = 3;
		}

		if (pAbfBtInfo->DefaultAudioDeviceLmpVersion >= 3) {
			A_BOOL EDRCapable = FALSE;
			/* double check that the device is EDR capable, a 2.0 device can be EDR or non EDR */
			status = CheckRemoteDeviceEDRCapable(pAbfBtInfo, &EDRCapable);
			if (A_SUCCESS(status)) {
				if (!EDRCapable) {
					A_INFO("Remote Audio Device (%s) is not EDR Capable, downgrading lmp version.. \n",
							pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
					/* for audio coex, treat this like a 1.2 device */
					pAbfBtInfo->DefaultAudioDeviceLmpVersion = 2;
				}
			}
		}
		pAbfBtInfo->DefaultRemoteAudioDevicePropsValid = TRUE;

	} while (FALSE);

}

#define LMP_FEATURE_ACL_EDR_2MBPS_BYTE_INDEX  3
#define LMP_FEATURE_ACL_EDR_2MBPS_BIT_MASK    0x2
#define LMP_FEATURE_ACL_EDR_3MBPS_BYTE_INDEX  3
#define LMP_FEATURE_ACL_EDR_3MBPS_BIT_MASK    0x4
#define LMP_FEATURES_LENGTH                   8

#define HCI_REMOTE_COMMAND_TIMEOUT 2000
static A_STATUS GetRemoteAclDeviceHandle(ABF_BT_INFO *pAbfBtInfo, A_UINT16 *pConn_handle)
{
	A_STATUS status = A_OK;
	int i, len, sk = -1;
	struct hci_conn_list_req *connList = NULL;
	struct hci_conn_info *connInfo = NULL;
	do {
		sk = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);

		if (sk < 0) {
			A_ERR("[%s] Failed to get raw BT socket: %d\n", __FUNCTION__, errno);
			status = A_NO_RESOURCE;
			break;
		}

		len = (sizeof(*connInfo)) * ABTH_MAX_CONNECTIONS + sizeof(*connList);
		connList = (struct hci_conn_list_req *)A_MALLOC(len);

		if (connList == NULL) {
			A_DEBUG("No connection found during calling function [%s]\n", __FUNCTION__);
			status = A_NO_MEMORY;
			break;
		}

		A_MEMZERO(connList, len);

		connList->dev_id = pAbfBtInfo->AdapterId;
		connList->conn_num = ABTH_MAX_CONNECTIONS;
		connInfo = connList->conn_info;

		if (ioctl(sk, HCIGETCONNLIST, (void *)connList)) {
			A_ERR("[%s] Failed to get connection list: %d\n", __FUNCTION__, errno);
			status = A_EPERM;
			break;
		}

		for (i = 0; i < connList->conn_num; i++, connInfo++) {
			if (connInfo->type == ACL_LINK) {
				*pConn_handle = connInfo->handle;
				break;
			}
		}

		if (i==connList->conn_num) {
			status = A_ENOENT;
			break;
		}
	} while (0);
	if (connList != NULL) {
		A_FREE(connList);
	}
	if (sk>=0) {
		close(sk);
	}
	return status;
}

static A_STATUS CheckRemoteDeviceEDRCapable(ABF_BT_INFO *pAbfBtInfo, A_BOOL *pEDRCapable)
{
	A_STATUS status;
	A_UINT16 conn_handle;
	A_UCHAR evtBuffer[HCI_MAX_EVENT_SIZE];
	A_UCHAR *eventPtr;
	int eventLen;
	A_UINT8 *lmp_features;
	do {
		status = GetRemoteAclDeviceHandle(pAbfBtInfo, &conn_handle);
		if (A_FAILED(status)) {
			break;
		}
		status = IssueHCICommand(pAbfBtInfo,
				cmd_opcode_pack(OGF_LINK_CTL, OCF_READ_REMOTE_FEATURES),
				(A_UCHAR *)&conn_handle,
				2,
				HCI_REMOTE_COMMAND_TIMEOUT,
				evtBuffer,
				sizeof(evtBuffer),
				&eventPtr,
				&eventLen);

		if (A_FAILED(status) || !eventPtr) {
			A_ERR("[%s] Failed to get remote features \n", __FUNCTION__);
			break;
		}

		/* Process LMP Features */
		lmp_features = &eventPtr[3];

		A_DUMP_BUFFER(lmp_features,sizeof(lmp_features),"Remote Device LMP Features:");

		if ((lmp_features[LMP_FEATURE_ACL_EDR_2MBPS_BYTE_INDEX] & LMP_FEATURE_ACL_EDR_2MBPS_BIT_MASK)  ||
				(lmp_features[LMP_FEATURE_ACL_EDR_3MBPS_BYTE_INDEX] & LMP_FEATURE_ACL_EDR_3MBPS_BIT_MASK)) {
			A_INFO("Device (%s) is EDR capable \n", pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
			*pEDRCapable = TRUE;
		} else {
			A_INFO("Device (%s) is NOT EDR capable \n", pAbfBtInfo->DefaultRemoteAudioDeviceAddress);
			*pEDRCapable = FALSE;
		}
	} while (0);

	return status;
}

/* This is new function to check remote device LMP version */
static A_STATUS GetRemoteDeviceLMPVersion(ABF_BT_INFO *pAbfBtInfo)
{
	A_STATUS status;
	A_UINT16 conn_handle;
	A_UCHAR evtBuffer[HCI_MAX_EVENT_SIZE];
	A_UCHAR *eventPtr;
	int eventLen;

	do {
		status = GetRemoteAclDeviceHandle(pAbfBtInfo, &conn_handle);
		if (A_FAILED(status)) {
			break;
		}
		status = IssueHCICommand(pAbfBtInfo,
				cmd_opcode_pack(OGF_LINK_CTL, OCF_READ_REMOTE_VERSION),
				(A_UCHAR *)&conn_handle,
				2,
				HCI_REMOTE_COMMAND_TIMEOUT,
				evtBuffer,
				sizeof(evtBuffer),
				&eventPtr,
				&eventLen);

		if (A_FAILED(status) || !eventPtr) {
			A_ERR("[%s] Failed to get remote Version \n", __FUNCTION__);
			break;
		}

		/* Process LMP Version */

		if (eventPtr[3] == 0) {
			strlcpy(&pAbfBtInfo->DefaultRemoteAudioDeviceVersion[0], "1.0", 32);
		} else if (eventPtr[3] == 1) {
			strlcpy(&pAbfBtInfo->DefaultRemoteAudioDeviceVersion[0], "1.1", 32);
		} else if (eventPtr[3] == 2) {
			strlcpy(&pAbfBtInfo->DefaultRemoteAudioDeviceVersion[0], "1.2", 32);
		} else if (eventPtr[3] == 3) {
			strlcpy(&pAbfBtInfo->DefaultRemoteAudioDeviceVersion[0], "2.0", 32);
		}

		A_INFO("[%s], Remote Device LMP Version: %d, in string format this is: %s\n", __FUNCTION__, eventPtr[3],
				pAbfBtInfo->DefaultRemoteAudioDeviceVersion);
	} while (0);

	return status;
}

static void GetBtAudioConnectionProperties(ABF_BT_INFO              *pAbfBtInfo,
		ATHBT_STATE_INDICATION   Indication)
{
	A_UCHAR     role = 0;
	A_CHAR      *pDescr = NULL;
	A_STATUS    status;

	do {
		if (!pAbfBtInfo->DefaultRemoteAudioDevicePropsValid) {
			break;
		}

		/* Incases where HciX is not supported, don't check for the role */
		if((pAbfBtInfo->pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING)) {
			return;
		}
		/* get role */
		status = GetConnectedDeviceRole(pAbfBtInfo,
				pAbfBtInfo->DefaultRemoteAudioDeviceAddress,
				Indication == ATH_BT_A2DP ?FALSE : TRUE,
				&role);
		if (A_FAILED(status)) {
			role = 0;
		}
		if (Indication == ATH_BT_A2DP) {
			pDescr = "A2DP";
			pAbfBtInfo->pInfo->A2DPConnection_LMPVersion = pAbfBtInfo->DefaultAudioDeviceLmpVersion;
			pAbfBtInfo->pInfo->A2DPConnection_Role = role;
			A_INFO("A2DP Version %d\n",pAbfBtInfo->pInfo->A2DPConnection_LMPVersion);
		} else if (Indication == ATH_BT_SCO) {
			if (pAbfBtInfo->CurrentSCOLinkType == SCO_LINK) {
				pDescr = "SCO";
			} else {
				pDescr = "eSCO";
			}
			pAbfBtInfo->pInfo->SCOConnection_LMPVersion = pAbfBtInfo->DefaultAudioDeviceLmpVersion;
			pAbfBtInfo->pInfo->SCOConnection_Role = role;

			if((pAbfBtInfo->pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING)) {

				pAbfBtInfo->pInfo->SCOConnectInfo.Valid = TRUE;
			}else{
				/* for SCO connections check if the event filter captured
				 * the SYNCH connection complete event */
				CheckHciEventFilter(pAbfBtInfo);
			}
		} else {
			pDescr = "UNKNOWN!!";
		}

		A_INFO("BT Audio connection properties:  (%s) (role: %s, lmp version: %d) \n",
				pDescr, role ? "SLAVE" : "MASTER", pAbfBtInfo->DefaultAudioDeviceLmpVersion);

	} while (FALSE);

}


static A_STATUS WaitForHCIEvent(int         Socket,
		int         TimeoutMs,
		A_UCHAR     *pBuffer,
		int         MaxLength,
		A_UCHAR     EventCode,
		A_UINT16    OpCode,
		A_UCHAR     **ppEventPtr,
		int         *pEventLength)
{
	int eventLen;
	hci_event_hdr *eventHdr;
	struct pollfd pfd;
	int result;
	A_UCHAR *eventPtr;
	A_STATUS status = A_OK;

	if (!pBuffer) {
		A_ERR("[%s] invalid input parameter\n", __FUNCTION__);
		return A_ERROR;
	}
	*ppEventPtr = NULL;
	A_MEMZERO(&pfd,sizeof(pfd));
	pfd.fd = Socket;
	pfd.events = POLLIN;

	if (EventCode == EVT_CMD_COMPLETE)
		A_INFO("Waiting for HCI CMD Complete Event, Opcode: 0x%4.4X (%d MS) \n",OpCode, TimeoutMs);
	else
		A_INFO("Waiting for HCI Event: %d (%d MS) \n",EventCode, TimeoutMs);

	while (1) {

		/* check socket for a captured event using a short timeout
		 * the caller usually calls this function when it knows there
		 * is an event that is likely to be captured */
		result = poll(&pfd, 1, TimeoutMs);

		if (result < 0) {
			if ((errno == EAGAIN) || (errno == EINTR)) {
				/* interrupted */
			} else {
				A_ERR("[%s] Socket Poll Failed! : %d \n", __FUNCTION__, errno);
				status = A_ERROR;
			}
			break;
		}

		if (result == 0) {
			A_ERR("[%s], poll returned with 0 \n",__FUNCTION__);
			status = A_ERROR;
			break;
		}

		if (!(pfd.revents & POLLIN)) {
			A_ERR("[%s], POLLIN check failed\n",__FUNCTION__);
			status = A_ERROR;
			break;
		}
		/* get the packet */
		eventLen = read(Socket, pBuffer, MaxLength);
		if (eventLen == 0) {
			/* no event */
			A_INFO("[%s], No Event\n",__FUNCTION__);
			status = A_ERROR;
			break;
		}
		if(eventLen > MaxLength) {
			A_ERR("[%s] Length longer than expected (%d) : %d \n", __FUNCTION__, MaxLength,
					eventLen);
			status = A_ERROR;
			break;
		}
		if (eventLen < (1 + HCI_EVENT_HDR_SIZE)) {
			A_ERR("[%s] Unknown receive packet! len : %d \n", __FUNCTION__, eventLen);
			status = A_ERROR;
			break;
		}
		if (pBuffer[0] != HCI_EVENT_PKT) {
			A_ERR("[%s] Unsupported packet type : %d \n", __FUNCTION__, pBuffer[0]);
			status = A_ERROR;
			break;
		}

		eventPtr = &pBuffer[1];
		eventLen--;
		eventHdr = (hci_event_hdr *)eventPtr;
		eventPtr += HCI_EVENT_HDR_SIZE;
		eventLen -= HCI_EVENT_HDR_SIZE;

		if (eventHdr->evt != EventCode) {
			/* not interested in this one */
			continue;
		}
		if(eventPtr == NULL) {
			A_ERR("[%s] Socket read points to NULL\n", __FUNCTION__);
			status = A_ERROR;
			break;
		}
		if (eventHdr->evt == EVT_CMD_COMPLETE) {
			if (eventLen < (int)sizeof(evt_cmd_complete)) {
				A_ERR("[%s] EVT_CMD_COMPLETE event is too small! len=%d \n", __FUNCTION__, eventLen);
				status = A_ERROR;
				break;
			} else {
				A_UINT16 evOpCode = btohs(BTEV_CMD_COMPLETE_GET_OPCODE(eventPtr));
				/* check for opCode match */
				if (OpCode != evOpCode) {
					/* keep searching */
					continue;
				}
			}
		}
		/* found it */
		*ppEventPtr = eventPtr;
		*pEventLength = eventLen;
		break;
	}
	return status;
}

static void CheckHciEventFilter(ABF_BT_INFO   *pAbfBtInfo)
{
	A_UCHAR     buffer[HCI_MAX_EVENT_SIZE];
	A_STATUS    status;
	A_UCHAR     *eventPtr;
	int         eventLen;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;

	do {

		if (!USE_DBUS_FOR_HEADSET_PROFILE(pInfo)) {
			A_ERR("Calling CheckHciEventFilter is not valid in this mode! \n");
			break;
		}

		status = WaitForHCIEvent(pAbfBtInfo->HCIEventListenerSocket,
				100,
				buffer,
				sizeof(buffer),
				EVT_SYNC_CONN_COMPLETE,
				0,
				&eventPtr,
				&eventLen);

		if (A_FAILED(status))
			break;

		if (eventPtr == NULL)
			break;

		if (eventLen < (int)sizeof(evt_sync_conn_complete)) {
			A_ERR("SYNC_CONN_COMPLETE Event is too small! : %d \n", eventLen);
			break;
		}

		pAbfBtInfo->pInfo->SCOConnectInfo.LinkType = BTEV_GET_BT_CONN_LINK_TYPE(eventPtr);
		pAbfBtInfo->pInfo->SCOConnectInfo.TransmissionInterval = BTEV_GET_TRANS_INTERVAL(eventPtr);
		pAbfBtInfo->pInfo->SCOConnectInfo.RetransmissionInterval = BTEV_GET_RETRANS_INTERVAL(eventPtr);
		pAbfBtInfo->pInfo->SCOConnectInfo.RxPacketLength = BTEV_GET_RX_PKT_LEN(eventPtr);
		pAbfBtInfo->pInfo->SCOConnectInfo.TxPacketLength = BTEV_GET_TX_PKT_LEN(eventPtr);

		A_INFO("HCI SYNC_CONN_COMPLETE event captured, conn info (%d, %d, %d, %d, %d) \n",
				pAbfBtInfo->pInfo->SCOConnectInfo.LinkType,
				pAbfBtInfo->pInfo->SCOConnectInfo.TransmissionInterval,
				pAbfBtInfo->pInfo->SCOConnectInfo.RetransmissionInterval,
				pAbfBtInfo->pInfo->SCOConnectInfo.RxPacketLength,
				pAbfBtInfo->pInfo->SCOConnectInfo.TxPacketLength);

		/* now valid */
		pAbfBtInfo->pInfo->SCOConnectInfo.Valid = TRUE;

	} while (FALSE);

}

static void CleanupHciEventFilter(ABF_BT_INFO *pAbfBtInfo)
{
	A_STATUS status;

	if (pAbfBtInfo->HCIEventListenerSocket >= 0) {
		pAbfBtInfo->HCIFilterThreadShutdown = TRUE;
		/* close socket, if there is a thread waiting on this socket, it will error and then exit */
		close(pAbfBtInfo->HCIEventListenerSocket);
		pAbfBtInfo->HCIEventListenerSocket = -1;

		if (pAbfBtInfo->HCIFilterThreadCreated) {
			A_INFO("[%s] Waiting for HCI filter thread to exit... \n",
					__FUNCTION__);
			/* wait for thread to exit
			 * note: JOIN cleans up thread resources as per POSIX spec. */
			status = A_TASK_JOIN(&pAbfBtInfo->hBtHCIFilterThread);
			if (A_FAILED(status)) {
				A_ERR("[%s] Failed to JOIN HCI filter thread \n",
						__FUNCTION__);
			}
			A_MEMZERO(&pAbfBtInfo->hBtHCIFilterThread,sizeof(pAbfBtInfo->hBtHCIFilterThread));
			pAbfBtInfo->HCIFilterThreadCreated = FALSE;
		}

		pAbfBtInfo->HCIFilterThreadShutdown = FALSE;
	}

}

static inline void set_bit(int bit, void *mask)
{
	*((uint32_t *) mask + (bit >> 5)) |= (1 << (bit & 31));
}

static inline void set_ptype(int type, struct hci_filter *filt)
{
	set_bit((type == HCI_VENDOR_PKT) ? 0 : (type & HCI_FLT_TYPE_BITS), &filt->type_mask);
}

static inline void set_event(int event, struct hci_filter *filt)
{
	set_bit((event & HCI_FLT_EVENT_BITS), &filt->event_mask);
}

static A_STATUS SetupHciEventFilter(ABF_BT_INFO *pAbfBtInfo)
{
	A_STATUS            status = A_ERROR;
	struct hci_filter   filterSetting;
	struct sockaddr_hci addr;
	ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	do {

		if (pAbfBtInfo->HCIEventListenerSocket >= 0) {
			/* close previous */
			CleanupHciEventFilter(pAbfBtInfo);
		}

		pAbfBtInfo->HCIEventListenerSocket = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);

		if (pAbfBtInfo->HCIEventListenerSocket < 0) {
			A_ERR("[%s] Failed to get raw BT socket: %d \n", __FUNCTION__, errno);
			break;
		}

		memset(&filterSetting, 0, sizeof(filterSetting));
		set_ptype(HCI_EVENT_PKT,  &filterSetting);
		/* To caputre INQUIRY command capture */
		set_ptype(HCI_COMMAND_PKT, &filterSetting);

		/* capture SYNC_CONN Complete */
		set_event(EVT_SYNC_CONN_COMPLETE, &filterSetting);

		/* Capture INQUIRY_COMPLETE event  */
		set_event(EVT_INQUIRY_COMPLETE, &filterSetting);
		set_event(EVT_CONN_REQUEST, &filterSetting);
		set_event(EVT_PIN_CODE_REQ, &filterSetting);
		set_event(EVT_LINK_KEY_REQ, &filterSetting);
		set_event(EVT_CONN_COMPLETE, &filterSetting);
		set_event(EVT_LINK_KEY_NOTIFY, &filterSetting);

		if (!USE_DBUS_FOR_HEADSET_PROFILE(pInfo)) {
			/* if we are not using DBUS for the headset profile, we need
			 * to capture other HCI event packets */
			set_event(EVT_DISCONN_COMPLETE, &filterSetting);
			set_event(EVT_CONN_COMPLETE, &filterSetting);
		}

		if (setsockopt(pAbfBtInfo->HCIEventListenerSocket,
					SOL_HCI,
					HCI_FILTER,
					&filterSetting,
					sizeof(filterSetting)) < 0) {
			A_ERR("[%s] Failed to set socket opt: %d \n", __FUNCTION__, errno);
			break;
		}

		A_MEMZERO(&addr,sizeof(addr));
		/* bind to the current adapter */
		addr.hci_family = AF_BLUETOOTH;
		addr.hci_dev = pAbfBtInfo->AdapterId;

		if (bind(pAbfBtInfo->HCIEventListenerSocket,
					(struct sockaddr *)&addr,
					sizeof(addr)) < 0) {
			A_ERR("[%s] Can't bind to hci:%d (err:%d) \n", __FUNCTION__, pAbfBtInfo->AdapterId, errno);
			break;
		}

		A_INFO("BT Event Filter Set, Mask: 0x%8.8X:%8.8X \n",
				filterSetting.event_mask[1], filterSetting.event_mask[0]);
		/* spawn a thread that will capture HCI events from the adapter */
		status = A_TASK_CREATE(&pAbfBtInfo->hBtHCIFilterThread, HCIFilterThread, pAbfBtInfo);
		if (A_FAILED(status)) {
			A_ERR("[%s] Failed to spawn a BT thread\n", __FUNCTION__);
			break;
		}
		pAbfBtInfo->HCIFilterThreadCreated = TRUE;

		status = A_OK;

	} while (FALSE);

	if (A_FAILED(status)) {
		CleanupHciEventFilter(pAbfBtInfo);
	}

	return status;
}


/* issue HCI command, currently this ONLY supports simple commands that
 * only expect a command complete, the event pointer returned points to the command
 * complete event structure for the caller to decode */
static A_STATUS IssueHCICommand(ABF_BT_INFO *pAbfBtInfo,
		A_UINT16    OpCode,
		A_UCHAR     *pCmdData,
		int         CmdLength,
		int         EventRecvTimeoutMS,
		A_UCHAR     *pEventBuffer,
		int         MaxLength,
		A_UCHAR     **ppEventPtr,
		int         *pEventLength)
{
	A_STATUS            status = A_ERROR;
	A_UCHAR             hciType = HCI_COMMAND_PKT;
	hci_command_hdr     hciCommandHdr;
	struct  iovec       iv[3];
	int                 ivcount = 0;
	int                 sk,result;
	struct hci_filter   filterSetting;
	struct sockaddr_hci addr;

	do {

		sk = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);

		if (sk < 0) {
			A_ERR("[%s] Failed to get raw BT socket: %d \n", __FUNCTION__, errno);
			break;
		}

		hciCommandHdr.opcode = htobs(OpCode);
		hciCommandHdr.plen= CmdLength;

		iv[0].iov_base = &hciType;
		iv[0].iov_len  = 1;
		ivcount++;
		iv[1].iov_base = &hciCommandHdr;
		iv[1].iov_len  = HCI_COMMAND_HDR_SIZE;
		ivcount++;

		if (pCmdData != NULL) {
			iv[2].iov_base = pCmdData;
			iv[2].iov_len  = CmdLength;
			ivcount++;
		}

		/* setup socket to capture the event */
		if (OpCode == cmd_opcode_pack(OGF_LINK_CTL, OCF_READ_REMOTE_FEATURES)) {
			memset(&filterSetting, 0, sizeof(filterSetting));
			set_ptype(HCI_EVENT_PKT, &filterSetting);
			set_event(EVT_READ_REMOTE_FEATURES_COMPLETE, &filterSetting);
		} else if (OpCode == cmd_opcode_pack(OGF_LINK_CTL, OCF_READ_REMOTE_VERSION)) {
			memset(&filterSetting, 0, sizeof(filterSetting));
			set_ptype(HCI_EVENT_PKT, &filterSetting);
			set_event(EVT_READ_REMOTE_VERSION_COMPLETE, &filterSetting);
		}
		else {
			memset(&filterSetting, 0, sizeof(filterSetting));
			set_ptype(HCI_EVENT_PKT,  &filterSetting);
			set_event(EVT_CMD_COMPLETE, &filterSetting);
		}

		if (setsockopt(sk, SOL_HCI, HCI_FILTER, &filterSetting, sizeof(filterSetting)) < 0) {
			A_ERR("[%s] Failed to set socket opt: %d \n", __FUNCTION__, errno);
			break;
		}

		A_MEMZERO(&addr,sizeof(addr));
		addr.hci_family = AF_BLUETOOTH;
		addr.hci_dev = pAbfBtInfo->AdapterId;

		if (bind(sk,(struct sockaddr *)&addr, sizeof(addr)) < 0) {
			A_ERR("[%s] Can't bind to hci:%d (err:%d) \n", __FUNCTION__, pAbfBtInfo->AdapterId, errno);
			break;
		}

		while ((result = writev(sk, iv, ivcount)) < 0) {
			if (errno == EAGAIN || errno == EINTR) {
				continue;
			}
			break;
		}

		if (result <= 0) {
			A_ERR("[%s] Failed to write to hci:%d (err:%d) \n", __FUNCTION__, pAbfBtInfo->AdapterId, errno);
			break;
		}

		/* To support new HCI Commands */
		switch (OpCode) {
			case cmd_opcode_pack(OGF_LINK_CTL, OCF_READ_REMOTE_FEATURES):
				status = WaitForHCIEvent(sk,
						EventRecvTimeoutMS,
						pEventBuffer,
						MaxLength,
						EVT_READ_REMOTE_FEATURES_COMPLETE,
						OpCode,
						ppEventPtr,
						pEventLength);
				break;
			case cmd_opcode_pack(OGF_LINK_CTL, OCF_READ_REMOTE_VERSION):
				status = WaitForHCIEvent(sk,
						EventRecvTimeoutMS,
						pEventBuffer,
						MaxLength,
						EVT_READ_REMOTE_VERSION_COMPLETE,
						OpCode,
						ppEventPtr,
						pEventLength);
				break;
			default:
				status = WaitForHCIEvent(sk,
						EventRecvTimeoutMS,
						pEventBuffer,
						MaxLength,
						EVT_CMD_COMPLETE,
						OpCode,
						ppEventPtr,
						pEventLength);
				break;
		}

		if (A_FAILED(status)) {
			break;
		}

		status = A_OK;

	} while (FALSE);

	if (sk >= 0) {
		close(sk);
	}

	return status;
}

#define AFH_CHANNEL_MAP_BYTES  10

typedef struct _WLAN_CHANNEL_MAP {
	A_UCHAR  Map[AFH_CHANNEL_MAP_BYTES];
} WLAN_CHANNEL_MAP;

#define MAX_WLAN_CHANNELS 14

typedef struct _WLAN_CHANNEL_RANGE {
	int    ChannelNumber;
	int    Center;       /* in Mhz */
} WLAN_CHANNEL_RANGE;

const WLAN_CHANNEL_RANGE g_ChannelTable[MAX_WLAN_CHANNELS] = {
	{ 1  , 2412},
	{ 2  , 2417},
	{ 3  , 2422},
	{ 4  , 2427},
	{ 5  , 2432},
	{ 6  , 2437},
	{ 7  , 2442},
	{ 8  , 2447},
	{ 9  , 2452},
	{ 10 , 2457},
	{ 11 , 2462},
	{ 12 , 2467},
	{ 13 , 2472},
	{ 14 , 2484},
};

static WLAN_CHANNEL_MAP g_ChannelMapTable[MAX_WLAN_CHANNELS + 1] = {
	{ {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F}}, /* 0 -- no WLAN */
	{ {0x00,0x00,0xC0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F}}, /* 1 */
	{ {0x0F,0x00,0x00,0xF8,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F}}, /* 2 */
	{ {0xFF,0x01,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F}}, /* 3 */
	{ {0xFF,0x3F,0x00,0x00,0xE0,0xFF,0xFF,0xFF,0xFF,0x7F}}, /* 4 */
	{ {0xFF,0xFF,0x07,0x00,0x00,0xFC,0xFF,0xFF,0xFF,0x7F}}, /* 5 */
	{ {0xFF,0xFF,0xFF,0x00,0x00,0x80,0xFF,0xFF,0xFF,0x7F}}, /* 6 */
	{ {0xFF,0xFF,0xFF,0x1F,0x00,0x00,0xF0,0xFF,0xFF,0x7F}}, /* 7 */
	{ {0xFF,0xFF,0xFF,0xFF,0x03,0x00,0x00,0xFE,0xFF,0x7F}}, /* 8 */
	{ {0xFF,0xFF,0xFF,0xFF,0x7F,0x00,0x00,0xC0,0xFF,0x7F}}, /* 9 */
	{ {0xFF,0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x00,0xF8,0x7F}}, /* 10 */
	{ {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x00,0x7F}}, /* 11 */
	{ {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3F,0x00,0x00,0x60}}, /* 12 */
	{ {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,0x00,0x00}}, /* 13 */
	{ {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x00}}, /* 14 */
};

#define AFH_COMMAND_COMPLETE_TIMEOUT_MS 2000

static int LookUpChannel(int FreqMhz)
{
	int i;

	if (FreqMhz == 0) {
		/* not connected */
		return 0;
	}

	for (i = 0; i < MAX_WLAN_CHANNELS; i++) {
		if (FreqMhz <= g_ChannelTable[i].Center) {
			break;
		}
	}
	return (i < MAX_WLAN_CHANNELS) ? g_ChannelTable[i].ChannelNumber : 0;
}
#ifdef MULTI_WLAN_CHAN_SUPPORT
static A_STATUS IssueAFHChannelClassification(ABF_BT_INFO *pAbfBtInfo,
					ABF_WLAN_INFO *pAbfWlanInfo)
{
	A_UCHAR     evtBuffer[HCI_MAX_EVENT_SIZE];
	A_STATUS    status;
	A_UCHAR     *eventPtr;
	int         eventLen;
	A_UCHAR     pChannelMap[AFH_CHANNEL_MAP_BYTES];
	ABF_WLAN_CONN_IF *conn = NULL;
	int channel, mapByte;

	/*init pChannelMap with all channels*/
	A_MEMCPY(pChannelMap, g_ChannelMapTable[0].Map, AFH_CHANNEL_MAP_BYTES);

	conn = pAbfWlanInfo->connIf;
	while (conn) {
		A_INFO("WLAN Operating Channel: %d \n", conn->channel);
		if (conn->channel >= 2412) {
			/* convert Mhz into a channel number */
			channel = LookUpChannel(conn->channel);
		} else {
			return A_ERROR;
		}

		/*Eliminate the channels*/
		for (mapByte = 0; mapByte < AFH_CHANNEL_MAP_BYTES; mapByte++)
			pChannelMap[mapByte] &=
				g_ChannelMapTable[channel].Map[mapByte];

		conn = conn->next;
	}
	A_DUMP_BUFFER(pChannelMap, AFH_CHANNEL_MAP_BYTES,
		"AFH Channel Classification Map");

	do {

		status = IssueHCICommand(pAbfBtInfo,
				cmd_opcode_pack(3,0x3F),
				pChannelMap,
				AFH_CHANNEL_MAP_BYTES,
				AFH_COMMAND_COMPLETE_TIMEOUT_MS,
				evtBuffer,
				sizeof(evtBuffer),
				&eventPtr,
				&eventLen);


		if (A_FAILED(status)) {
			break;
		}

		status = A_ERROR;

		if (eventPtr == NULL) {
			A_ERR("[%s] Failed to capture AFH command complete event \n", __FUNCTION__);
			break;
		}

		if (eventLen < (int)sizeof(evt_cmd_complete) + 1) {
			A_ERR("[%s] not enough bytes in AFH command complete event %d \n", __FUNCTION__, eventLen);
			break;
		}

		/* check status parameter that follows the command complete event body */
		if (eventPtr[sizeof(evt_cmd_complete)] != 0) {
			A_ERR("[%s] AFH command complete event indicated failure : %d \n", __FUNCTION__,
					eventPtr[sizeof(evt_cmd_complete)]);
			break;
		}

		A_INFO(" AFH Command successfully issued \n");
		//A_DUMP_BUFFER(pChannelMap, AFH_CHANNEL_MAP_BYTES, "AFH Channel Classification Map");

		status = A_OK;

	} while (FALSE);

	return status;
}
#else
static A_STATUS IssueAFHChannelClassification(ABF_BT_INFO *pAbfBtInfo, int CurrentWLANChannel)
{
	A_UCHAR     evtBuffer[HCI_MAX_EVENT_SIZE];
	A_STATUS    status;
	A_UCHAR     *eventPtr;
	int         eventLen;
	A_UCHAR     *pChannelMap;

	A_INFO("WLAN Operating Channel: %d \n", CurrentWLANChannel);

	if (CurrentWLANChannel > MAX_WLAN_CHANNELS) {
		/* check if this is expressed in Mhz */
		if (CurrentWLANChannel >= 2412) {
			/* convert Mhz into a channel number */
			CurrentWLANChannel = LookUpChannel(CurrentWLANChannel);
		} else {
			return A_ERROR;
		}
	}

	pChannelMap = &(g_ChannelMapTable[CurrentWLANChannel].Map[0]);

	do {

		status = IssueHCICommand(pAbfBtInfo,
				cmd_opcode_pack(3,0x3F),
				pChannelMap,
				AFH_CHANNEL_MAP_BYTES,
				AFH_COMMAND_COMPLETE_TIMEOUT_MS,
				evtBuffer,
				sizeof(evtBuffer),
				&eventPtr,
				&eventLen);


		if (A_FAILED(status)) {
			break;
		}

		status = A_ERROR;

		if (eventPtr == NULL) {
			A_ERR("[%s] Failed to capture AFH command complete event \n", __FUNCTION__);
			break;
		}

		if (eventLen < (int)sizeof(evt_cmd_complete) + 1) {
			A_ERR("[%s] not enough bytes in AFH command complete event %d \n", __FUNCTION__, eventLen);
			break;
		}

		/* check status parameter that follows the command complete event body */
		if (eventPtr[sizeof(evt_cmd_complete)] != 0) {
			A_ERR("[%s] AFH command complete event indicated failure : %d \n", __FUNCTION__,
					eventPtr[sizeof(evt_cmd_complete)]);
			break;
		}

		A_INFO(" AFH Command successfully issued \n");
		//A_DUMP_BUFFER(pChannelMap, AFH_CHANNEL_MAP_BYTES, "AFH Channel Classification Map");

		status = A_OK;

	} while (FALSE);

	return status;
}
#endif

#ifdef MULTI_WLAN_CHAN_SUPPORT
void IndicateCurrentWLANOperatingChannel(ATHBT_FILTER_INFO *pFilterInfo)
{
        ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)pFilterInfo->pBtInfo;
	ABF_WLAN_INFO *pAbfWlanInfo = (ABF_WLAN_INFO *)pFilterInfo->pWlanInfo;

        if (NULL == pAbfBtInfo) {
                return;
        }

        if (pFilterInfo->Flags & ABF_ENABLE_AFH_CHANNEL_CLASSIFICATION) {
               IssueAFHChannelClassification(pAbfBtInfo, pAbfWlanInfo);
        }
}

#else
void IndicateCurrentWLANOperatingChannel(ATHBT_FILTER_INFO *pFilterInfo, int CurrentWLANChannel)
{
	ABF_BT_INFO *pAbfBtInfo = (ABF_BT_INFO *)pFilterInfo->pBtInfo;
	ATHBT_FILTER_INFO *pInfo = pAbfBtInfo->pInfo;

	if (NULL == pAbfBtInfo) {
		return;
	}

	if (pFilterInfo->Flags & ABF_ENABLE_AFH_CHANNEL_CLASSIFICATION) {
		IssueAFHChannelClassification(pAbfBtInfo,CurrentWLANChannel);
	}

	if(pInfo->Flags & ABF_USE_ONLY_DBUS_FILTERING) {
		Abf_IssueAFHViaHciLib(pAbfBtInfo, CurrentWLANChannel);
	}
}
#endif
static void *HCIFilterThread(void *arg)
{
	ABF_BT_INFO            *pAbfBtInfo = (ABF_BT_INFO *)arg;
	ATHBT_FILTER_INFO      *pInfo = (ATHBT_FILTER_INFO *)pAbfBtInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;
	A_UINT8                 buffer[300];
	A_UINT8                 *pBuffer;
	int                     eventLen;

	A_INFO("[%s] starting up \n", __FUNCTION__);

	while (1) {

		pBuffer = buffer;

		if (pAbfBtInfo->HCIFilterThreadShutdown) {
			break;
		}
		/* get the packet */
		eventLen = read(pAbfBtInfo->HCIEventListenerSocket, pBuffer, sizeof(buffer));

		if (eventLen < 0) {
			if (!pAbfBtInfo->HCIFilterThreadShutdown) {
				A_ERR("[%s] socket error %d \n", __FUNCTION__, eventLen);
			}
			break;
		}

		if (eventLen == 0) {
			/* no event */
			continue;
		}

		if (eventLen < (1 + HCI_EVENT_HDR_SIZE)) {
			A_ERR("[%s] Unknown receive packet! len : %d \n", __FUNCTION__, eventLen);
			continue;
		}

		/* first byte is a tag for the HCI packet type, we only care about events */
		if (pBuffer[0] == HCI_EVENT_PKT) {
			/* pass this raw HCI event to the filter core */
			AthBtFilterHciEvent(pInstance,&pBuffer[1],eventLen - 1);
			A_UINT8 *eventCode = &pBuffer[1];
			/* revive deprecated "DiscoveryCompleted" signal in BlueZ 4.x */
			if (*eventCode == EVT_INQUIRY_COMPLETE) {
				A_DEBUG("Device Inquiry Completed\n");
				pAbfBtInfo->btInquiryState &= ~(1 << 0);
				if(!pAbfBtInfo->btInquiryState) {
					AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_OFF);
				}
			}
			if (*eventCode == EVT_PIN_CODE_REQ) {
				A_DEBUG("Pin Code Request\n");
				pAbfBtInfo->btInquiryState |= (1 << 0xF);
				AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_ON);
			}
			if (*eventCode == EVT_LINK_KEY_NOTIFY) {
				A_DEBUG("link key notify\n");
				pAbfBtInfo->btInquiryState &= ~(1 << 0xF);
				if(!pAbfBtInfo->btInquiryState) {
					AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_OFF);
				}
			}

			if(*eventCode == EVT_CONN_COMPLETE) {
				A_DEBUG("Conn complete\n");
				pAbfBtInfo->btInquiryState &= ~(1 << 2);
				if(!pAbfBtInfo->btInquiryState) {
					AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_OFF);
				}
			}

			if (*eventCode == EVT_CONN_REQUEST) {
				A_DEBUG("Conn request\n");
				/* record BD_ADDR of remote device */
				bdaddr_to_str((const bdaddr_t *)&pBuffer[3],
					&pAbfBtInfo->DefaultRemoteAudioDeviceAddress[0]);
			}

			/* revive deprecated "DiscoveryStarted" signal by capturing INQUIRY commands */
		} else if (pBuffer[0] == HCI_COMMAND_PKT) {
			A_UINT16 *packedOpCode = (A_UINT16 *)&pBuffer[1];
			if(cmd_opcode_ogf(*packedOpCode) == 0x1 &&
					cmd_opcode_ocf(*packedOpCode) == 0x5 )
			{
				A_DEBUG("Bt-Connect\n");
				pAbfBtInfo->btInquiryState |= (1 << 2);
				AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_ON);
			}

			if (*packedOpCode == HCI_RESET_CMD) {
				Abf_WlanIssueBtOnOff(pInfo, STATE_OFF);
				A_INFO("HCI command reset\n");
				Abf_BToff();
			}

			if (*packedOpCode ==
					cmd_opcode_pack(OGF_LINK_CTL,
						OCF_INQUIRY_CANCEL)) {
				A_DEBUG("Device Inquiry Completed\n");
				pAbfBtInfo->btInquiryState &= ~(1 << 0);
				if (!pAbfBtInfo->btInquiryState)
					AthBtIndicateState(pInstance,
							ATH_BT_INQUIRY, STATE_OFF);
			}

			if (*packedOpCode == cmd_opcode_pack(OGF_LINK_CTL, OCF_INQUIRY)
					|| *packedOpCode == cmd_opcode_pack(OGF_LINK_CTL, OCF_PERIODIC_INQUIRY)) {
				A_DEBUG("Device Inquiry Started\n");
				pAbfBtInfo->btInquiryState |= (1 << 0);
				AthBtIndicateState(pInstance, ATH_BT_INQUIRY, STATE_ON);
			} else if (*packedOpCode == cmd_opcode_pack(OGF_LINK_CTL, OCF_CREATE_CONN)) {
				/* "Connected" signal is deprecated and won't record BD_ADDR of remote device connected */
				bdaddr_to_str((const bdaddr_t *)&pBuffer[4],
						&pAbfBtInfo->DefaultRemoteAudioDeviceAddress[0]);
			}
		} else {
			A_ERR("[%s] Unsupported packet type : %d \n", __FUNCTION__, buffer[0]);
			continue;
		}
	}

	A_INFO("[%s] exiting \n", __FUNCTION__);

	return NULL;
}

void HandleBTEvent(BT_STACK_EVENT btevent, void *btfilt_info)
{
	if (g_NotificationConfig[btevent].signal_handler) {
		(*g_NotificationConfig[btevent].signal_handler)(NULL, (ABF_BT_INFO *)btfilt_info);
	}
}

#ifdef HID_PROFILE_SUPPORT
void HandleBTEvent1(BT_STACK_EVENT btevent, void *btfilt_info, A_BOOL val)
{
	if (g_NotificationConfig[btevent].signal_handler) {
		(*g_NotificationConfig[btevent].signal_handler)(&val, (ABF_BT_INFO *)btfilt_info);
	}
}
#endif

#endif /* BLUETOOTH_BLUEDROID */
