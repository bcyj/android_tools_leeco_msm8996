/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Proprietary and Confidential.
 */

#ifndef __ALLJOYN_DBUS_H
#define __ALLJOYN_DBUS_H

#ifdef BLUETOOTH_BLUEDROID

#include "abtfilt_int.h"

/* BT spec v4.0 Vol.2 Part.B Sec.1.2*/
typedef struct {
	A_UINT8 octet[6];
} __attribute__((packed)) bd_addr_t;

/* BT spec v4.0 Vol.2 Part.C Sec.2.3:
*  All parameters have a little-endian format,
*  i.e. the least significant byte is transferred first.
*/
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htoles(d)  (d)
#define letohs(d)  (d)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define htoles(d)  bswap_16(d)
#define letohs(d)  bswap_16(d)
#endif

/* BT spec v4.0 Vol.2 Part.E Sec.5.4.1 */
typedef struct {
	A_UINT16	OpCode;
	A_UINT8		ParamTotalLen;
} __attribute__ ((packed)) hci_command_header;

/* BT spec v4.0 Vol.2 Part.E Sec.5.4.1: The OGF occupies the upper
*  6 bits of the Opcode, while the OCF occupies the remaining 10 bits.
*/
#define HCICMD_OPCODE(ogf, ocf)	(A_UINT16)((ogf << 10) | (ocf & 0x03ff))
#define HCICMD_OGF(opcode)		(opcode >> 10)
#define HCICMD_OCF(opcode)		(opcode & 0x03ff)

/* BT spec v4.0 Vol.2 Part.E Sec.7.7.35*/
#define SCO_CONN	0x00
#define ESCO_CONN	0x02

/* BT spec v4.0 Vol.2 Part.E Sec.5.4.4 */
typedef struct {
	A_UINT8		EventCode;
	A_UINT8		ParamTotalLen;
} __attribute__ ((packed)) hci_event_header;

/* BT spec v4.0 Vol.2 Part.E Sec.7.1:
*  For the Link Control commands, the OGF is defined as 0x01.
*/
#define OGF_LINK_CONTROL		0x01

/* BT spec v4.0 Vol.2 Part.E Sec.7.1.1 */
#define OCF_INQUIRY			0x0001
/* BT spec v4.0 Vol.2 Part.E Sec.7.1.2 */
#define OCF_INQUIRY_CANCEL		0x0002
/* BT spec v4.0 Vol.2 Part.E Sec.7.1.3 */
#define OCF_PERIODIC_INQUIRY_MODE		0x0003
/* BT spec v4.0 Vol.2 Part.E Sec.7.1.5 */
#define OCF_CREATE_CONNECTION			0x0005
/* BT spec v4.0 Vol.2 Part.E Sec.7.1.21 */
#define OCF_READ_REMOTE_SUPPORTED_FEATURES	0x001B
/* BT spec v4.0 Vol.2 Part.E Sec.7.1.22 */
#define OCF_READ_REMOTE_VERSION_INFO		0x001D

/* BT spec v4.0 Vol.2 Part.E Sec.7.2:
*  For the Link Policy Commands, the OGF is defined as 0x02.
*/
#define OGF_LINK_POLICY		0x02
/* BT spec v4.0 Vol.2 Part.E Sec.7.2.7 */
#define OCF_ROLE_DISCOVERY		0x0009

/* BT spec v4.0 Vol.2 Part.E Sec.7.4:
*  For Informational Parameters Commands, the OGF is defined as 0x04.
*/
#define OGF_INFORMATIONAL_PARAMS		0x04
/* BT spec v4.0 Vol.2 Part.E Sec.7.4.6 */
#define OCF_READ_BD_ADDR		0x0009

/* BT spec v4.0 Vol.2 Part.E Sec.7.7.1 */
#define EVT_CODE_INQUIRY_COMPLETE		0x01
/* BT spec v4.0 Vol.2 Part.E Sec.7.7.3 */
#define EVT_CODE_CONNECTION_COMPLETE		0x03
/* BT spec v4.0 Vol.2 Part.E Sec.7.7.4 */
#define EVT_CODE_CONNECTION_REQUEST		0x04
/* BT spec v4.0 Vol.2 Part.E Sec.7.7.5 */
#define EVT_CODE_DISCONNECTION_REQUEST          0x05
/* BT spec v4.0 Vol.2 Part.E Sec.7.7.11 */
#define EVT_CODE_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE	0x0B
/* BT spec v4.0 Vol.2 Part.E Sec.7.7.12 */
#define EVT_CODE_READ_REMOTE_VERSION_INFO_COMPLETE	0x0C
/* BT spec v4.0 Vol.2 Part.E Sec.7.7.14 */
#define EVT_CODE_COMMAND_COMPLETE		0x0E
typedef struct {
	A_UINT8		Num_HCI_Command_Packets;
	A_UINT16	Command_Opcode;
} __attribute__ ((packed)) commad_complete_event;

/* BT spec v4.0 Vol.2 Part.E Sec.7.7.22 */
#define EVT_CODE_PIN_CODE_REQUEST		0x16
/* BT spec v4.0 Vol.2 Part.E Sec.7.7.24 */
#define EVT_LINK_KEY_NOTIFICATION		0x18
/* BT spec v4.0 Vol.2 Part.E Sec.7.7.35 */
#define EVT_SCO_CONNECTION_COMPLETE		0x2C
/* BT spec v4.0 Vol.2 Part.E Sec.7.7.35 */
#define EVENT_SCO_CONNECTION_COMP_SIZE  17

/* BT spec v4.0 Vol.4 Part.A Sec.2 */
enum HCI_PACKET_TYPE {
	HCI_COMMAND_PACKET = 0x01,
	HCI_ACL_DATA_PACKET = 0x02,
	HCI_SCO_DATA_PACKET = 0x03,
	HCI_EVENT_PACKET = 0x04
};

/* according to BT spec v4.0: 256 (one octet) +
*  4 (packet type/length/event code/param len)
*/
#define MAX_HCI_EVENT_SIZE	260

/*
        for Bluedroid
  */
#define STACK_SOCKET_NAME "qcom.btc.server"
#define HCI_SOCKET_NAME "btc_hci"

enum BTCEvent {
	BD_NONE = 0x00,
	BD_BT_ADAPTER_ADDED = 0x20,
	BD_BT_ADAPTER_REMOVED = 0x21,
	BD_DEVICE_DISCOVERY_STARTED = 0x22,
	BD_DEVICE_DISCOVERY_FINISHED = 0x23,
	BD_REMOTE_DEVICE_CONNECTED = 0x24,
	BD_REMOTE_DEVICE_DISCONNECTED = 0x25,
	BD_AUDIO_DEVICE_ADDED = 0x26,
	BD_AUDIO_DEVICE_REMOVED = 0x27,
	BD_AUDIO_HEADSET_CONNECTED = 0x40,
	BD_AUDIO_HEADSET_DISCONNECTED = 0x41,
	BD_AUDIO_HEADSET_STREAM_STARTED = 0x42,
	BD_AUDIO_HEADSET_STREAM_STOPPED = 0x43,
	BD_AUDIO_SINK_CONNECTED = 0x60,
	BD_AUDIO_SINK_DISCONNECTED = 0x61,
	BD_AUDIO_SINK_STREAM_STARTED = 0x62,
	BD_AUDIO_SINK_STREAM_STOPPED = 0x63,
	BD_INPUT_DEVICE_CONNECTED = 0x80,
	BD_INPUT_DEVICE_DISCONNECTED = 0x81,
};

/* modify this definition after adding/deleting any BT event*/
#define BT_EVENTS_NUM_MAX 18

/* abtfilter related data structure */
#define STRING_SIZE_MAX		128
#define BD_ADDR_SIZE		6
#define MAX_SOCKET_RETRY_CNT	400

void Abf_BToff();
typedef void (* BT_EVENT_HANDLER)(void *,void *);

typedef struct _ABF_BT_INFO {
	ATHBT_FILTER_INFO              *pInfo;
	A_BOOL                          AdapterAvailable;
	A_UINT8                         HCIVersion;
	A_UINT16                        HCIRevision;
	A_UINT8                         HCI_LMPVersion;
	A_UINT16                        HCI_LMPSubVersion;
	A_UINT8                         RemoteDevice[BD_ADDR_SIZE];
	A_UINT8                         HCI_DeviceAddress[BD_ADDR_SIZE];
	A_CHAR                          HCI_AdapterName[STRING_SIZE_MAX];
	A_CHAR                          HCI_DeviceName[STRING_SIZE_MAX];
	A_CHAR                          HCI_ManufacturerName[STRING_SIZE_MAX];
	A_CHAR                          HCI_ProtocolVersion[STRING_SIZE_MAX];
	A_BOOL                          AdapterCbRegistered;
	A_CHAR                          DefaultAudioDeviceName[STRING_SIZE_MAX];
	A_CHAR                          DefaultRemoteAudioDeviceAddress[32];
	A_CHAR                          DefaultRemoteAudioDeviceVersion[32];
	A_UINT8                         DefaultAudioDeviceLmpVersion;
	A_BOOL                          DefaultAudioDeviceAvailable;
	A_BOOL                          AudioCbRegistered;
	A_UCHAR                         CurrentSCOLinkType;
	int                             AdapterId;
	int				StackEventSocket; /* socket for BT stack/profile events*/
	A_TASK_HANDLE                   hBtStackEventThread;
	A_BOOL                          StackEventThreadCreated;
	A_BOOL                          StackEventThreadShutdown;
	int                             HCIEventListenerSocket;
	A_TASK_HANDLE                   hBtHCIFilterThread;
	A_BOOL                          HCIFilterThreadCreated;
	A_BOOL                          HCIFilterThreadShutdown;
	BT_EVENT_HANDLER                SignalHandlers[BT_EVENTS_NUM_MAX];
	A_BOOL                          DefaultRemoteAudioDevicePropsValid;
	A_BOOL                          ThreadCreated;
	A_UINT32                        btInquiryState;
} ABF_BT_INFO;

#else /* BLUETOOTH_BLUEDROID */

#include "abtfilt_int.h"
/*-----------------------------------------------------------------------*/
/* BT Section */
#define STRING_SIZE_MAX             128
#define BD_ADDR_SIZE                6

#define BLUEZ_NAME                        "org.bluez"
#define ADAPTER_INTERFACE                 "org.bluez.Adapter"
#define MANAGER_INTERFACE                 "org.bluez.Manager"

#ifdef BLUEZ4_3
#define BLUEZ_PATH                        "/"
#define AUDIO_MANAGER_PATH                "/org/bluez/"
#define AUDIO_MANAGER_INTERFACE           "org.bluez"
#define AUDIO_SINK_INTERFACE              "org.bluez.AudioSink"
#define AUDIO_SOURCE_INTERFACE            "org.bluez.AudioSource"
#define AUDIO_HEADSET_INTERFACE           "org.bluez.Headset"
#define AUDIO_GATEWAY_INTERFACE           "org.bluez.Gateway"
#define AUDIO_DEVICE_INTERFACE            "org.bluez.Device"
#define INPUT_DEVICE_INTERFACE            "org.bluez.Input"
#else
#define BLUEZ_PATH                        "/org/bluez"
#define AUDIO_MANAGER_PATH                "/org/bluez/audio"
#define AUDIO_SINK_INTERFACE              "org.bluez.audio.Sink"
#define AUDIO_SOURCE_INTERFACE            "org.bluez.audio.Source"
#define AUDIO_HEADSET_INTERFACE           "org.bluez.audio.Headset"
#define AUDIO_GATEWAY_INTERFACE           "org.bluez.audio.Gateway"
#define AUDIO_MANAGER_INTERFACE           "org.bluez.audio.Manager"
#define AUDIO_DEVICE_INTERFACE            "org.bluez.audio.Device"
#endif

void Abf_BToff();
typedef void (* BT_EVENT_HANDLER)(void *,void *);

typedef enum {
    BT_ADAPTER_ADDED = 0,
    BT_ADAPTER_REMOVED,
    DEVICE_DISCOVERY_STARTED,
    DEVICE_DISCOVERY_FINISHED,
    REMOTE_DEVICE_CONNECTED,
    REMOTE_DEVICE_DISCONNECTED,
    AUDIO_DEVICE_ADDED,
    AUDIO_DEVICE_REMOVED,
    AUDIO_HEADSET_CONNECTED,
    AUDIO_HEADSET_DISCONNECTED,
    AUDIO_HEADSET_STREAM_STARTED,
    AUDIO_HEADSET_STREAM_STOPPED,
    AUDIO_SINK_CONNECTED,
    AUDIO_SINK_DISCONNECTED,
    AUDIO_SINK_STREAM_STARTED,
    AUDIO_SINK_STREAM_STOPPED,
#ifdef HID_PROFILE_SUPPORT
    INPUT_DEVICE_PROPERTY_CHANGED,
#endif
    BT_EVENTS_NUM_MAX,
} BT_STACK_EVENT;

typedef struct _ABF_BT_INFO {
    ATHBT_FILTER_INFO              *pInfo;
    A_BOOL                          AdapterAvailable;
    A_UINT8                         HCIVersion;
    A_UINT16                        HCIRevision;
    A_UINT8                         HCI_LMPVersion;
    A_UINT16                        HCI_LMPSubVersion;
    A_UINT8                         RemoteDevice[BD_ADDR_SIZE];
    A_UINT8                         HCI_DeviceAddress[BD_ADDR_SIZE];
    A_CHAR                          HCI_AdapterName[STRING_SIZE_MAX];
    A_CHAR                          HCI_DeviceName[STRING_SIZE_MAX];
    A_CHAR                          HCI_ManufacturerName[STRING_SIZE_MAX];
    A_CHAR                          HCI_ProtocolVersion[STRING_SIZE_MAX];
    A_BOOL                          AdapterCbRegistered;
    A_CHAR                          DefaultAudioDeviceName[STRING_SIZE_MAX];
    A_CHAR                          DefaultRemoteAudioDeviceAddress[32];
    A_CHAR                          DefaultRemoteAudioDeviceVersion[32];
    A_UINT8                         DefaultAudioDeviceLmpVersion;
    A_BOOL                          DefaultAudioDeviceAvailable;
    A_BOOL                          AudioCbRegistered;
    A_UCHAR                         CurrentSCOLinkType;
    int                             AdapterId;
    int                             HCIEventListenerSocket;
    A_TASK_HANDLE                   hBtHCIFilterThread;
    A_BOOL                          HCIFilterThreadCreated;
    A_BOOL                          HCIFilterThreadShutdown;
    BT_EVENT_HANDLER                SignalHandlers[BT_EVENTS_NUM_MAX];
    A_BOOL                          DefaultRemoteAudioDevicePropsValid;
    A_BOOL                          ThreadCreated;
    A_UINT32                        btInquiryState;
} ABF_BT_INFO;

#endif /* BLUETOOTH_BLUEDROID */
#endif /*ABTFILT_BTSTACK_DBUS_H_*/
