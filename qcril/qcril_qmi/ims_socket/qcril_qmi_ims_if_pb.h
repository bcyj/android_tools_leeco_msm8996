/*!
  @file
  qcril_qmi_ims_if_pb.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#ifndef QCRIL_QMI_IMS_IF_PB_H
#define QCRIL_QMI_IMS_IF_PB_H

#ifndef QCRIL_PROTOBUF_BUILD_ENABLED
typedef struct _Ims__MsgTag Ims__MsgTag;
typedef struct _Ims__CallFailCauseResponse Ims__CallFailCauseResponse;
typedef struct _Ims__StatusForAccessTech Ims__StatusForAccessTech;
typedef struct _Ims__Info Ims__Info;
typedef struct _Ims__SrvStatusList Ims__SrvStatusList;
typedef struct _Ims__CallDetails Ims__CallDetails;
typedef struct _Ims__CallModify Ims__CallModify;
typedef struct _Ims__CallList Ims__CallList;
typedef struct _Ims__CallList__Call Ims__CallList__Call;
typedef struct _Ims__Dial Ims__Dial;
typedef struct _Ims__Hangup Ims__Hangup;
typedef struct _Ims__DeflectCall Ims__DeflectCall;
typedef struct _Ims__Clir Ims__Clir;
typedef struct _Ims__Answer Ims__Answer;
typedef struct _Ims__SwitchWaitingOrHoldingAndActive Ims__SwitchWaitingOrHoldingAndActive;
typedef struct _Ims__Mute Ims__Mute;
typedef struct _Ims__Dtmf Ims__Dtmf;
typedef struct _Ims__Registration Ims__Registration;
typedef struct _Ims__RingBackTone Ims__RingBackTone;
typedef struct _Ims__IFConnected Ims__IFConnected;
typedef struct _Ims__LastFailCause Ims__LastFailCause;
typedef struct _Ims__Extra Ims__Extra;
typedef struct _Ims__Handover Ims__Handover;
typedef struct _Ims__TtyNotify Ims__TtyNotify;
typedef struct _Ims__RadioStateChanged Ims__RadioStateChanged;
typedef struct _Ims__ClipProvisionStatus Ims__ClipProvisionStatus;
typedef struct _Ims__ServiceClass Ims__ServiceClass;
typedef struct _Ims__CbNumList Ims__CbNumList;
typedef struct _Ims__CbNumListType Ims__CbNumListType;
typedef struct _Ims__CallWaitingInfo Ims__CallWaitingInfo;
typedef struct _Ims__CallForwardInfoList Ims__CallForwardInfoList;
typedef struct _Ims__CallForwardInfoList__CallForwardInfo Ims__CallForwardInfoList__CallForwardInfo;
typedef struct _Ims__CallFwdTimerInfo Ims__CallFwdTimerInfo;
typedef struct _Ims__ConfInfo Ims__ConfInfo;
typedef struct _Ims__SuppSvcNotification Ims__SuppSvcNotification;
typedef struct _Ims__SuppSvcStatus Ims__SuppSvcStatus;
typedef struct _Ims__SuppSvcRequest Ims__SuppSvcRequest;
typedef struct _Ims__SuppSvcResponse Ims__SuppSvcResponse;
typedef struct _Ims__Colr Ims__Colr;
typedef struct _Ims__VideoCallQuality Ims__VideoCallQuality;
typedef struct _Ims__MwiMessageSummary Ims__MwiMessageSummary;
typedef struct _Ims__MwiMessageDetails Ims__MwiMessageDetails;
typedef struct _Ims__Mwi Ims__Mwi;
typedef struct _Ims__Hold Ims__Hold;
typedef struct _Ims__Resume Ims__Resume;
typedef struct _Ims__RtpStatisticsData Ims__RtpStatisticsData;
typedef struct _Ims__WifiCallingInfo Ims__WifiCallingInfo;

typedef struct _ProtobufCBinaryData ProtobufCBinaryData;

struct _ProtobufCBinaryData
{
   size_t len;
   uint8_t *data;
};

typedef char *ProtobufCBuffer;
typedef char *ProtobufCAllocator;
typedef void* ProtobufCMessage;
typedef int protobuf_c_boolean;

/* --- enums --- */

typedef enum _Ims__Registration__RegState {
  IMS__REGISTRATION__REG_STATE__REGISTERED = 1,
  IMS__REGISTRATION__REG_STATE__NOT_REGISTERED = 2,
  IMS__REGISTRATION__REG_STATE__REGISTERING = 3
} Ims__Registration__RegState;
typedef enum _Ims__RingBackTone__ToneFlag {
  IMS__RING_BACK_TONE__TONE_FLAG__STOP = 0,
  IMS__RING_BACK_TONE__TONE_FLAG__START = 1
} Ims__RingBackTone__ToneFlag;
typedef enum _Ims__IFConnected__Version {
  IMS__IFCONNECTED__VERSION__VERSION_0 = 0
} Ims__IFConnected__Version;
typedef enum _Ims__MsgType {
  IMS__MSG_TYPE__UNKNOWN = 0,
  IMS__MSG_TYPE__REQUEST = 1,
  IMS__MSG_TYPE__RESPONSE = 2,
  IMS__MSG_TYPE__UNSOL_RESPONSE = 3
} Ims__MsgType;
typedef enum _Ims__MsgId {
  IMS__MSG_ID__UNKNOWN_REQ = 0,
  IMS__MSG_ID__REQUEST_IMS_REGISTRATION_STATE = 1,
  IMS__MSG_ID__REQUEST_DIAL = 2,
  IMS__MSG_ID__REQUEST_ANSWER = 3,
  IMS__MSG_ID__REQUEST_HANGUP = 4,
  IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE = 5,
  IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS = 6,
  IMS__MSG_ID__REQUEST_HANGUP_WAITING_OR_BACKGROUND = 7,
  IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND = 8,
  IMS__MSG_ID__REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE = 9,
  IMS__MSG_ID__REQUEST_CONFERENCE = 10,
  IMS__MSG_ID__REQUEST_EXIT_EMERGENCY_CALLBACK_MODE = 11,
  IMS__MSG_ID__REQUEST_EXPLICIT_CALL_TRANSFER = 12,
  IMS__MSG_ID__REQUEST_DTMF = 13,
  IMS__MSG_ID__REQUEST_DTMF_START = 14,
  IMS__MSG_ID__REQUEST_DTMF_STOP = 15,
  IMS__MSG_ID__REQUEST_UDUB = 16,
  IMS__MSG_ID__REQUEST_MODIFY_CALL_INITIATE = 17,
  IMS__MSG_ID__REQUEST_MODIFY_CALL_CONFIRM = 18,
  IMS__MSG_ID__REQUEST_QUERY_CLIP = 19,
  IMS__MSG_ID__REQUEST_GET_CLIR = 20,
  IMS__MSG_ID__REQUEST_SET_CLIR = 21,
  IMS__MSG_ID__REQUEST_QUERY_CALL_FORWARD_STATUS = 22,
  IMS__MSG_ID__REQUEST_SET_CALL_FORWARD_STATUS = 23,
  IMS__MSG_ID__REQUEST_QUERY_CALL_WAITING = 24,
  IMS__MSG_ID__REQUEST_SET_CALL_WAITING = 25,
  IMS__MSG_ID__REQUEST_IMS_REG_STATE_CHANGE = 26,
  IMS__MSG_ID__REQUEST_SET_SUPP_SVC_NOTIFICATION = 27,
  IMS__MSG_ID__REQUEST_ADD_PARTICIPANT = 28,
  IMS__MSG_ID__REQUEST_QUERY_SERVICE_STATUS = 29,
  IMS__MSG_ID__REQUEST_SET_SERVICE_STATUS = 30,
  IMS__MSG_ID__REQUEST_SUPP_SVC_STATUS = 31,
  IMS__MSG_ID__REQUEST_DEFLECT_CALL = 32,
  IMS__MSG_ID__REQUEST_GET_COLR = 33,
  IMS__MSG_ID__REQUEST_SET_COLR = 34,
  IMS__MSG_ID__REQUEST_QUERY_VT_CALL_QUALITY = 35,
  IMS__MSG_ID__REQUEST_SET_VT_CALL_QUALITY = 36,
  IMS__MSG_ID__REQUEST_HOLD = 37,
  IMS__MSG_ID__REQUEST_RESUME = 38,
  IMS__MSG_ID__REQUEST_SEND_UI_TTY_MODE = 39,
  IMS__MSG_ID__REQUEST_GET_RTP_STATISTICS = 40,
  IMS__MSG_ID__REQUEST_GET_RTP_ERROR_STATISTICS = 41,
  IMS__MSG_ID__REQUEST_GET_WIFI_CALLING_STATUS = 42,
  IMS__MSG_ID__REQUEST_SET_WIFI_CALLING_STATUS = 43,
  IMS__MSG_ID__UNSOL_RSP_BASE = 200,
  IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED = 201,
  IMS__MSG_ID__UNSOL_CALL_RING = 202,
  IMS__MSG_ID__UNSOL_RINGBACK_TONE = 203,
  IMS__MSG_ID__UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED = 204,
  IMS__MSG_ID__UNSOL_ENTER_EMERGENCY_CALLBACK_MODE = 205,
  IMS__MSG_ID__UNSOL_EXIT_EMERGENCY_CALLBACK_MODE = 206,
  IMS__MSG_ID__UNSOL_MODIFY_CALL = 207,
  IMS__MSG_ID__UNSOL_RESPONSE_HANDOVER = 208,
  IMS__MSG_ID__UNSOL_REFRESH_CONF_INFO = 209,
  IMS__MSG_ID__UNSOL_SRV_STATUS_UPDATE = 210,
  IMS__MSG_ID__UNSOL_SUPP_SVC_NOTIFICATION = 211,
  IMS__MSG_ID__UNSOL_TTY_NOTIFICATION = 212,
  IMS__MSG_ID__UNSOL_RADIO_STATE_CHANGED = 213,
  IMS__MSG_ID__UNSOL_MWI = 214
} Ims__MsgId;
typedef enum _Ims__Error {
  IMS__ERROR__E_SUCCESS = 0,
  IMS__ERROR__E_RADIO_NOT_AVAILABLE = 1,
  IMS__ERROR__E_GENERIC_FAILURE = 2,
  IMS__ERROR__E_REQUEST_NOT_SUPPORTED = 6,
  IMS__ERROR__E_CANCELLED = 7,
  IMS__ERROR__E_UNUSED = 16,
  IMS__ERROR__E_INVALID_PARAMETER = 27,
  IMS__ERROR__E_REJECTED_BY_REMOTE = 28,
  IMS__ERROR__E_IMS_DEREGISTERED = 29
} Ims__Error;
typedef enum _Ims__CallState {
  IMS__CALL_STATE__CALL_ACTIVE = 0,
  IMS__CALL_STATE__CALL_HOLDING = 1,
  IMS__CALL_STATE__CALL_DIALING = 2,
  IMS__CALL_STATE__CALL_ALERTING = 3,
  IMS__CALL_STATE__CALL_INCOMING = 4,
  IMS__CALL_STATE__CALL_WAITING = 5,
  IMS__CALL_STATE__CALL_END = 6
} Ims__CallState;
typedef enum _Ims__RadioState {
  IMS__RADIO_STATE__RADIO_STATE_OFF = 0,
  IMS__RADIO_STATE__RADIO_STATE_UNAVAILABLE = 1,
  IMS__RADIO_STATE__RADIO_STATE_ON = 10
} Ims__RadioState;
typedef enum _Ims__CallType {
  IMS__CALL_TYPE__CALL_TYPE_VOICE = 0,
  IMS__CALL_TYPE__CALL_TYPE_VT_TX = 1,
  IMS__CALL_TYPE__CALL_TYPE_VT_RX = 2,
  IMS__CALL_TYPE__CALL_TYPE_VT = 3,
  IMS__CALL_TYPE__CALL_TYPE_VT_NODIR = 4,
  IMS__CALL_TYPE__CALL_TYPE_CS_VS_TX = 5,
  IMS__CALL_TYPE__CALL_TYPE_CS_VS_RX = 6,
  IMS__CALL_TYPE__CALL_TYPE_PS_VS_TX = 7,
  IMS__CALL_TYPE__CALL_TYPE_PS_VS_RX = 8,
  IMS__CALL_TYPE__CALL_TYPE_UNKNOWN = 9,
  IMS__CALL_TYPE__CALL_TYPE_SMS = 10,
  IMS__CALL_TYPE__CALL_TYPE_UT = 11
} Ims__CallType;
typedef enum _Ims__CallSubstate {
  IMS__CALL_SUBSTATE__CALL_SUBSTATE_NONE = 0,
  IMS__CALL_SUBSTATE__CALL_SUBSTATE_AUDIO_CONNECTED_SUSPENDED = 1,
  IMS__CALL_SUBSTATE__CALL_SUBSTATE_VIDEO_CONNECTED_SUSPENDED = 2,
  IMS__CALL_SUBSTATE__CALL_SUBSTATE_AVP_RETRY = 4,
  IMS__CALL_SUBSTATE__CALL_SUBSTATE_MEDIA_PAUSED = 8
} Ims__CallSubstate;
typedef enum _Ims__CallFailCause {
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_UNOBTAINABLE_NUMBER = 1,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_NORMAL = 16,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_BUSY = 17,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_CONGESTION = 34,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_INCOMPATIBILITY_DESTINATION = 88,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_CALL_BARRED = 240,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_USER_BUSY = 501,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_USER_REJECT = 502,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_LOW_BATTERY = 503,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_BLACKLISTED_CALL_ID = 504,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_NETWORK_UNAVAILABLE = 1010,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_FEATURE_UNAVAILABLE = 1011,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_Error = 1012,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_MISC = 1013,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_ANSWERED_ELSEWHERE = 1014,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_REDIRECTED = 2001,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_BAD_REQUEST = 2002,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_FORBIDDEN = 2003,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_NOT_FOUND = 2004,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_NOT_SUPPORTED = 2005,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_REQUEST_TIMEOUT = 2006,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE = 2007,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_BAD_ADDRESS = 2008,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_BUSY = 2009,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_REQUEST_CANCELLED = 2010,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_NOT_ACCEPTABLE = 2011,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_NOT_REACHABLE = 2012,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_SERVER_INTERNAL_ERROR = 2013,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_SERVICE_UNAVAILABLE = 2014,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_SERVER_TIMEOUT = 2015,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_USER_REJECTED = 2016,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_GLOBAL_ERROR = 2017,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_MEDIA_INIT_FAILED = 3001,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_MEDIA_NO_DATA = 3002,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_MEDIA_NOT_ACCEPTABLE = 3003,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_MEDIA_UNSPECIFIED_ERROR = 3004,
  IMS__CALL_FAIL_CAUSE__CALL_FAIL_ERROR_UNSPECIFIED = 65535
} Ims__CallFailCause;
typedef enum _Ims__CallDomain {
  IMS__CALL_DOMAIN__CALL_DOMAIN_UNKNOWN = 0,
  IMS__CALL_DOMAIN__CALL_DOMAIN_CS = 1,
  IMS__CALL_DOMAIN__CALL_DOMAIN_PS = 2,
  IMS__CALL_DOMAIN__CALL_DOMAIN_AUTOMATIC = 3,
  IMS__CALL_DOMAIN__CALL_DOMAIN_NOT_SET = 4
} Ims__CallDomain;
typedef enum _Ims__SrvType {
  IMS__SRV_TYPE__SMS = 1,
  IMS__SRV_TYPE__VOIP = 2,
  IMS__SRV_TYPE__VT = 3
} Ims__SrvType;
typedef enum _Ims__StatusType {
  IMS__STATUS_TYPE__STATUS_DISABLED = 0,
  IMS__STATUS_TYPE__STATUS_PARTIALLY_ENABLED = 1,
  IMS__STATUS_TYPE__STATUS_ENABLED = 2,
  IMS__STATUS_TYPE__STATUS_NOT_SUPPORTED = 3
} Ims__StatusType;
typedef enum _Ims__RadioTechType {
  IMS__RADIO_TECH_TYPE__RADIO_TECH_ANY = -1,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_UNKNOWN = 0,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_GPRS = 1,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_EDGE = 2,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_UMTS = 3,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_IS95A = 4,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_IS95B = 5,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_1xRTT = 6,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_EVDO_0 = 7,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_EVDO_A = 8,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_HSDPA = 9,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_HSUPA = 10,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_HSPA = 11,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_EVDO_B = 12,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_EHRPD = 13,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_LTE = 14,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_HSPAP = 15,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_GSM = 16,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_TD_SCDMA = 17,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_WIFI = 18,
  IMS__RADIO_TECH_TYPE__RADIO_TECH_IWLAN = 19
} Ims__RadioTechType;
typedef enum _Ims__IpPresentation {
  IMS__IP_PRESENTATION__IP_PRESENTATION_NUM_ALLOWED = 0,
  IMS__IP_PRESENTATION__IP_PRESENTATION_NUM_RESTRICTED = 1,
  IMS__IP_PRESENTATION__IP_PRESENTATION_NUM_DEFAULT = 2
} Ims__IpPresentation;
typedef enum _Ims__HandoverMsgType {
  IMS__HANDOVER__MSG__TYPE__START = 0,
  IMS__HANDOVER__MSG__TYPE__COMPLETE_SUCCESS = 1,
  IMS__HANDOVER__MSG__TYPE__COMPLETE_FAIL = 2,
  IMS__HANDOVER__MSG__TYPE__CANCEL = 3,
  IMS__HANDOVER__MSG__TYPE__NOT_TRIGGERED = 4
} Ims__HandoverMsgType;
typedef enum _Ims__ExtraType {
  IMS__EXTRA__TYPE__LTE_TO_IWLAN_HO_FAIL = 1
} Ims__ExtraType;
typedef enum _Ims__TtyModeType {
  IMS__TTY__MODE__TYPE__TTY_MODE_OFF = 0,
  IMS__TTY__MODE__TYPE__TTY_MODE_FULL = 1,
  IMS__TTY__MODE__TYPE__TTY_MODE_HCO = 2,
  IMS__TTY__MODE__TYPE__TTY_MODE_VCO = 3
} Ims__TtyModeType;
typedef enum _Ims__ClipStatus {
  IMS__CLIP_STATUS__NOT_PROVISIONED = 0,
  IMS__CLIP_STATUS__PROVISIONED = 1,
  IMS__CLIP_STATUS__STATUS_UNKNOWN = 2
} Ims__ClipStatus;
typedef enum _Ims__ServiceClassStatus {
  IMS__SERVICE_CLASS_STATUS__DISABLED = 0,
  IMS__SERVICE_CLASS_STATUS__ENABLED = 1
} Ims__ServiceClassStatus;
typedef enum _Ims__ConfCallState {
  IMS__CONF_CALL_STATE__RINGING = 0,
  IMS__CONF_CALL_STATE__FOREGROUND = 1,
  IMS__CONF_CALL_STATE__BACKGROUND = 2
} Ims__ConfCallState;
typedef enum _Ims__NotificationType {
  IMS__NOTIFICATION_TYPE__MO = 0,
  IMS__NOTIFICATION_TYPE__MT = 1
} Ims__NotificationType;
typedef enum _Ims__SuppSvcOperationType {
  IMS__SUPP_SVC_OPERATION_TYPE__ACTIVATE = 1,
  IMS__SUPP_SVC_OPERATION_TYPE__DEACTIVATE = 2,
  IMS__SUPP_SVC_OPERATION_TYPE__QUERY = 3,
  IMS__SUPP_SVC_OPERATION_TYPE__REGISTER = 4,
  IMS__SUPP_SVC_OPERATION_TYPE__ERASURE = 5
} Ims__SuppSvcOperationType;
typedef enum _Ims__SuppSvcFacilityType {
  IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_CLIP = 1,
  IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_COLP = 2,
  IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAOC = 3,
  IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAOIC = 4,
  IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAOICxH = 5,
  IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAIC = 6,
  IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAICr = 7,
  IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BA_ALL = 8,
  IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BA_MO = 9,
  IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BA_MT = 10,
  IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BS_MT = 11,
  IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAICa = 12
} Ims__SuppSvcFacilityType;

typedef enum _Ims__Quality {
  IMS__QUALITY__LOW = 0,
  IMS__QUALITY__HIGH = 1
} Ims__Quality;
typedef enum _Ims__MwiMessageType {
  IMS__MWI_MESSAGE_TYPE__MWI_MSG_NONE = -1,
  IMS__MWI_MESSAGE_TYPE__MWI_MSG_VOICE = 0,
  IMS__MWI_MESSAGE_TYPE__MWI_MSG_VIDEO = 1,
  IMS__MWI_MESSAGE_TYPE__MWI_MSG_FAX = 2,
  IMS__MWI_MESSAGE_TYPE__MWI_MSG_PAGER = 3,
  IMS__MWI_MESSAGE_TYPE__MWI_MSG_MULTIMEDIA = 4,
  IMS__MWI_MESSAGE_TYPE__MWI_MSG_TEXT = 5
} Ims__MwiMessageType;
typedef enum _Ims__MwiPriority {
  IMS__MWI_PRIORITY__MWI_MSG_PRIORITY_UNKNOWN = -1,
  IMS__MWI_PRIORITY__MWI_MSG_PRIORITY_LOW = 0,
  IMS__MWI_PRIORITY__MWI_MSG_PRIORITY_NORMAL = 1,
  IMS__MWI_PRIORITY__MWI_MSG_PRIORITY_URGENT = 2
} Ims__MwiPriority;
typedef enum _Ims__WifiCallingStatus {
  IMS__WIFI_CALLING_STATUS__WIFI_NOT_SUPPORTED = 0,
  IMS__WIFI_CALLING_STATUS__WIFI_STATUS_ON = 1,
  IMS__WIFI_CALLING_STATUS__WIFI_STATUS_OFF = 2
} Ims__WifiCallingStatus;
typedef enum _Ims__WifiCallingPreference {
  IMS__WIFI_CALLING_PREFERENCE__WIFI_PREF_NONE = 0,
  IMS__WIFI_CALLING_PREFERENCE__WIFI_PREFERRED = 1,
  IMS__WIFI_CALLING_PREFERENCE__WIFI_ONLY = 2,
  IMS__WIFI_CALLING_PREFERENCE__CELLULAR_PREFERRED = 3,
  IMS__WIFI_CALLING_PREFERENCE__CELLULAR_ONLY = 4
} Ims__WifiCallingPreference;
/* --- messages --- */


struct  _Ims__MsgTag
{
  uint32_t token;
  Ims__MsgType type;
  Ims__MsgId id;
  Ims__Error error;
};
#define IMS__MSG_TAG__INIT {0,0,0,0}

struct  _Ims__CallFailCauseResponse
{
  ProtobufCMessage base;
  protobuf_c_boolean has_failcause;
  Ims__CallFailCause failcause;
  protobuf_c_boolean has_errorinfo;
  ProtobufCBinaryData errorinfo;
  char *networkerrorstring;
};
#define IMS__CALL_FAIL_CAUSE_RESPONSE__INIT { NULL, 0,0, 0,{0,NULL}, NULL }

struct  _Ims__StatusForAccessTech
{
  ProtobufCMessage base;
  protobuf_c_boolean has_networkmode;
  Ims__RadioTechType networkmode;
  protobuf_c_boolean has_status;
  Ims__StatusType status;
  protobuf_c_boolean has_restrictioncause;
  uint32_t restrictioncause;
  Ims__Registration *registered;
};
#define IMS__STATUS_FOR_ACCESS_TECH__INIT { NULL, 0,0, 0,0, 0,0, NULL }

struct  _Ims__Info
{
  ProtobufCMessage base;
  protobuf_c_boolean has_isvalid;
  protobuf_c_boolean isvalid;
  protobuf_c_boolean has_type;
  Ims__SrvType type;
  protobuf_c_boolean has_calltype;
  Ims__CallType calltype;
  protobuf_c_boolean has_status;
  Ims__StatusType status;
  protobuf_c_boolean has_userdata;
  ProtobufCBinaryData userdata;
  protobuf_c_boolean has_restrictcause;
  uint32_t restrictcause;
  size_t n_acctechstatus;
  Ims__StatusForAccessTech **acctechstatus;
};
#define IMS__INFO__INIT { NULL, 0,0, 0,0, 0,0, 0,0, 0,{0,NULL}, 0,0, 0,NULL }

struct  _Ims__SrvStatusList
{
  ProtobufCMessage base;
  size_t n_srvstatusinfo;
  Ims__Info **srvstatusinfo;
};
#define IMS__SRV_STATUS_LIST__INIT { NULL, 0,NULL }

struct  _Ims__CallDetails
{
  ProtobufCMessage base;
  protobuf_c_boolean has_calltype;
  Ims__CallType calltype;
  protobuf_c_boolean has_calldomain;
  Ims__CallDomain calldomain;
  protobuf_c_boolean has_extraslength;
  uint32_t extraslength;
  size_t n_extras;
  char **extras;
  Ims__SrvStatusList *localability;
  Ims__SrvStatusList *peerability;
  protobuf_c_boolean has_callsubstate;
  Ims__CallSubstate callsubstate;
  protobuf_c_boolean has_mediaid;
  int32_t mediaid;
};
#define IMS__CALL_DETAILS__INIT { NULL, 0,0, 0,0, 0,0, 0,NULL, NULL, NULL, 0,0 }

struct  _Ims__CallModify
{
  int has_callindex;
  uint32_t callindex;
  Ims__CallDetails *calldetails;
  protobuf_c_boolean has_error;
  Ims__Error error;
};
#define IMS__CALL_MODIFY__INIT { 0,0, NULL, 0,0 }

struct  _Ims__CallList__Call
{
  int has_state;
  Ims__CallState state;
  int has_index;
  uint32_t index;
  int has_toa;
  uint32_t toa;
  int has_ismpty;
  int ismpty;
  int has_ismt;
  int ismt;
  int has_als;
  uint32_t als;
  int has_isvoice;
  int isvoice;
  int has_isvoiceprivacy;
  int isvoiceprivacy;
  char *number;
  int has_numberpresentation;
  uint32_t numberpresentation;
  char *name;
  int has_namepresentation;
  uint32_t namepresentation;
  Ims__CallDetails *calldetails;
  Ims__CallFailCauseResponse *failcause;
};
#define IMS__CALL_LIST__CALL__INIT \
 { 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, NULL, 0,0, NULL, 0,0, NULL, NULL }


struct  _Ims__CallList
{
  size_t n_callattributes;
  Ims__CallList__Call **callattributes;
};
#define IMS__CALL_LIST__INIT {0,NULL }

struct  _Ims__Dial
{
  ProtobufCMessage base;
  char *address;
  int has_clir;
  uint32_t clir;
  Ims__CallDetails *calldetails;
  int has_presentation;
  Ims__IpPresentation presentation;
  protobuf_c_boolean has_isconferenceuri;
  protobuf_c_boolean isconferenceuri;
};
#define IMS__DIAL__INIT {NULL, NULL, 0,0, NULL, 0,0 ,0,0 }

struct  _Ims__Hangup
{
  ProtobufCMessage base;
  protobuf_c_boolean has_conn_index;
  uint32_t conn_index;
  protobuf_c_boolean has_multi_party;
  protobuf_c_boolean multi_party;
  char *conn_uri;
  protobuf_c_boolean has_conf_id;
  uint32_t conf_id;
  Ims__CallFailCauseResponse *failcauseresponse;
};
#define IMS__HANGUP__INIT \
 { 0,0, 0,0, NULL, 0,0, NULL }


struct  _Ims__DeflectCall
{
  ProtobufCMessage base;
  protobuf_c_boolean has_conn_index;
  uint32_t conn_index;
  char *number;
};
#define IMS__DEFLECT_CALL__INIT \
 { 0,0, NULL }


struct  _Ims__Clir
{
  int has_param_m;
  uint32_t param_m;
  int has_param_n;
  uint32_t param_n;
};
#define IMS__CLIR__INIT { 0,0, 0,0 }

struct  _Ims__Answer
{
  ProtobufCMessage base;
  protobuf_c_boolean has_call_type;
  Ims__CallType call_type;
  protobuf_c_boolean has_presentation;
  Ims__IpPresentation presentation;
};
#define IMS__ANSWER__INIT {NULL, 0,0, 0,0 }

struct  _Ims__SwitchWaitingOrHoldingAndActive
{
  ProtobufCMessage base;
  protobuf_c_boolean has_call_type;
  Ims__CallType call_type;
};
#define IMS__SWITCH_WAITING_OR_HOLDING_AND_ACTIVE__INIT { NULL, 0,0 }

struct  _Ims__Mute
{
  int has_mute_flag;
  int mute_flag;
};
#define IMS__MUTE__INIT {0, 0}

struct  _Ims__Dtmf
{
  char *dtmf;
};
#define IMS__DTMF__INIT {NULL}

struct  _Ims__Registration
{
  int has_state;
  Ims__Registration__RegState state;
  int has_errorcode;
  uint32_t errorcode;
  char *errormessage;
  protobuf_c_boolean has_radiotech;
  Ims__RadioTechType radiotech;
};
#define IMS__REGISTRATION__INIT {0, 0, 0,0, NULL, 0,0 }

struct  _Ims__RingBackTone
{
  int has_flag;
  Ims__RingBackTone__ToneFlag flag;
};
#define IMS__RING_BACK_TONE__INIT {0, 0}

struct  _Ims__IFConnected
{
  int has_if_version;
  Ims__IFConnected__Version if_version;
};
#define IMS__IFCONNECTED__INIT {0, 0}

struct  _Ims__LastFailCause
{
  int has_cause;
  Ims__CallFailCause cause;
};
#define IMS__LAST_FAIL_CAUSE__INIT {0,0}

struct  _Ims__Extra
{
  ProtobufCMessage base;
  protobuf_c_boolean has_type;
  Ims__ExtraType type;
  protobuf_c_boolean has_extrainfo;
  ProtobufCBinaryData extrainfo;
};
#define IMS__EXTRA__INIT { NULL, 0,0, 0,{0,NULL} }

struct  _Ims__Handover
{
  ProtobufCMessage base;
  protobuf_c_boolean has_type;
  Ims__HandoverMsgType type;
  protobuf_c_boolean has_srctech;
  Ims__RadioTechType srctech;
  protobuf_c_boolean has_targettech;
  Ims__RadioTechType targettech;
  Ims__Extra *hoextra;
  char *errorcode;
  char *errormessage;
};
#define IMS__HANDOVER__INIT {NULL, 0,0, 0,0, 0,0, NULL, NULL, NULL }

struct  _Ims__TtyNotify
{
  ProtobufCMessage base;
  protobuf_c_boolean has_mode;
  Ims__TtyModeType mode;
  protobuf_c_boolean has_userdata;
  ProtobufCBinaryData userdata;
};
#define IMS__TTY_NOTIFY__INIT { NULL, 0,0, 0,{0,NULL} }

struct  _Ims__RadioStateChanged
{
  int has_state;
  Ims__RadioState state;
};
#define IMS__RADIO_STATE_CHANGED__INIT { 0,0 }

struct  _Ims__ClipProvisionStatus
{
  ProtobufCMessage base;
  protobuf_c_boolean has_clip_status;
  Ims__ClipStatus clip_status;
};
#define IMS__CLIP_PROVISION_STATUS__INIT {NULL, 0,0}

struct  _Ims__ServiceClass
{
  int has_service_class;
  uint32_t service_class;
};
#define IMS__SERVICE_CLASS__INIT {0, 0}

struct  _Ims__CbNumList
{
  ProtobufCMessage base;
  protobuf_c_boolean has_status;
  Ims__ServiceClassStatus status;
  char *number;
};
#define IMS__CB_NUM_LIST__INIT { NULL, 0,0, NULL }


struct  _Ims__CbNumListType
{
  ProtobufCMessage base;
  Ims__ServiceClass *serviceclass;
  size_t n_cb_num_list;
  Ims__CbNumList **cb_num_list;
};
#define IMS__CB_NUM_LIST_TYPE__INIT { NULL, NULL, 0,NULL }

struct  _Ims__CallWaitingInfo
{
  int has_service_status;
  Ims__ServiceClassStatus service_status;
  Ims__ServiceClass *service_class;
};
#define IMS__CALL_WAITING_INFO__INIT {0, 0, NULL}

struct  _Ims__CallForwardInfoList__CallForwardInfo
{
  int has_status;
  uint32_t status;
  int has_reason;
  uint32_t reason;
  int has_service_class;
  uint32_t service_class;
  int has_toa;
  uint32_t toa;
  char *number;
  int has_time_seconds;
  uint32_t time_seconds;
  Ims__CallFwdTimerInfo *callfwdtimerstart;
  Ims__CallFwdTimerInfo *callfwdtimerend;
};
#define IMS__CALL_FORWARD_INFO_LIST__CALL_FORWARD_INFO__INIT {0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL}

struct  _Ims__CallForwardInfoList
{
  size_t n_info;
  Ims__CallForwardInfoList__CallForwardInfo **info;
};
#define IMS__CALL_FORWARD_INFO_LIST__INIT {0, NULL}

struct  _Ims__CallFwdTimerInfo
{
  protobuf_c_boolean has_year;
  uint32_t year;
  protobuf_c_boolean has_month;
  uint32_t month;
  protobuf_c_boolean has_day;
  uint32_t day;
  protobuf_c_boolean has_hour;
  uint32_t hour;
  protobuf_c_boolean has_minute;
  uint32_t minute;
  protobuf_c_boolean has_second;
  uint32_t second;
  protobuf_c_boolean has_timezone;
  uint32_t timezone;
};
#define IMS__CALL_FWD_TIMER_INFO__INIT {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0}

struct  _Ims__ConfInfo
{
  ProtobufCMessage base;
  protobuf_c_boolean has_conf_info_uri;
  ProtobufCBinaryData conf_info_uri;
  protobuf_c_boolean has_confcallstate;
  Ims__ConfCallState confcallstate;
};
#define IMS__CONF_INFO__INIT {NULL, 0,{0,NULL}, 0,0}

struct  _Ims__SuppSvcNotification
{
  ProtobufCMessage base;
  protobuf_c_boolean has_notificationtype;
  Ims__NotificationType notificationtype;
  protobuf_c_boolean has_code;
  uint32_t code;
  protobuf_c_boolean has_index;
  uint32_t index;
  protobuf_c_boolean has_type;
  uint32_t type;
  char *number;
  protobuf_c_boolean has_connid;
  uint32_t connid;
  char *history_info;
};
#define IMS__SUPP_SVC_NOTIFICATION__INIT { NULL, 0,0, 0,0, 0,0, 0,0, NULL, 0,0, NULL }

struct  _Ims__SuppSvcStatus
{
  ProtobufCMessage base;
  protobuf_c_boolean has_status;
  Ims__ServiceClassStatus status;
};
#define IMS__SUPP_SVC_STATUS__INIT { NULL, 0,0 }

struct  _Ims__SuppSvcRequest
{
  ProtobufCMessage base;
  protobuf_c_boolean has_operationtype;
  uint32_t operationtype;
  protobuf_c_boolean has_facilitytype;
  uint32_t facilitytype;
  Ims__CbNumListType *cbnumlisttype;
};
#define IMS__SUPP_SVC_REQUEST__INIT { NULL, 0,0, 0,0, NULL }

struct  _Ims__SuppSvcResponse
{
  ProtobufCMessage base;
  protobuf_c_boolean has_status;
  Ims__ServiceClassStatus status;
  protobuf_c_boolean has_facilitytype;
  uint32_t facilitytype;
  char *failurecause;
  size_t n_cbnumlisttype;
  Ims__CbNumListType **cbnumlisttype;
};
#define IMS__SUPP_SVC_RESPONSE__INIT { NULL, 0,0, 0,0, NULL, 0,NULL }

struct  _Ims__Colr
{
  ProtobufCMessage base;
  protobuf_c_boolean has_presentation;
  Ims__IpPresentation presentation;
};
#define IMS__COLR__INIT { NULL, 0,0 }

struct  _Ims__VideoCallQuality
{
  ProtobufCMessage base;
  protobuf_c_boolean has_quality;
  Ims__Quality quality;
};
#define IMS__VIDEO_CALL_QUALITY__INIT \
 { NULL, 0,0 }

struct  _Ims__MwiMessageSummary
{
  ProtobufCMessage base;
  protobuf_c_boolean has_messagetype;
  Ims__MwiMessageType messagetype;
  protobuf_c_boolean has_newmessage;
  uint32_t newmessage;
  protobuf_c_boolean has_oldmessage;
  uint32_t oldmessage;
  protobuf_c_boolean has_newurgent;
  uint32_t newurgent;
  protobuf_c_boolean has_oldurgent;
  uint32_t oldurgent;
};
#define IMS__MWI_MESSAGE_SUMMARY__INIT \
 { NULL, 0,0, 0,0, 0,0, 0,0, 0,0 }


struct  _Ims__MwiMessageDetails
{
  ProtobufCMessage base;
  char *toaddress;
  char *fromaddress;
  char *subject;
  char *date;
  protobuf_c_boolean has_priority;
  Ims__MwiPriority priority;
  char *messageid;
  protobuf_c_boolean has_messagetype;
  Ims__MwiMessageType messagetype;
};
#define IMS__MWI_MESSAGE_DETAILS__INIT \
 { NULL, NULL, NULL, NULL, NULL, 0,0, NULL, 0,0 }


struct  _Ims__Mwi
{
  ProtobufCMessage base;
  size_t n_mwimsgsummary;
  Ims__MwiMessageSummary **mwimsgsummary;
  char *ueaddress;
  size_t n_mwimsgdetail;
  Ims__MwiMessageDetails **mwimsgdetail;
};
#define IMS__MWI__INIT \
 { NULL, 0,NULL, NULL, 0,NULL }


struct  _Ims__Hold
{
  ProtobufCMessage base;
  protobuf_c_boolean has_callid;
  uint32_t callid;
};
#define IMS__HOLD__INIT \
 { 0,0 }


struct  _Ims__Resume
{
  ProtobufCMessage base;
  protobuf_c_boolean has_callid;
  uint32_t callid;
};
#define IMS__RESUME__INIT \
 { 0,0 }

struct  _Ims__RtpStatisticsData
{
  ProtobufCMessage base;
  protobuf_c_boolean has_count;
  uint32_t count;
};
#define IMS__RTP_STATISTICS_DATA__INIT \
 { 0,0 }

struct  _Ims__WifiCallingInfo
{
  ProtobufCMessage base;
  protobuf_c_boolean has_status;
  Ims__WifiCallingStatus status;
  protobuf_c_boolean has_preference;
  Ims__WifiCallingPreference preference;
};
#define IMS__WIFI_CALLING_INFO__INIT \
 { NULL, 0,0, 0,0 }

#endif

/* Ims__MsgTag methods */
void   qcril_qmi_ims__msg_tag__init
                     (Ims__MsgTag         *message);
size_t qcril_qmi_ims__msg_tag__get_packed_size
                     (const Ims__MsgTag   *message);
size_t qcril_qmi_ims__msg_tag__pack
                     (const Ims__MsgTag   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__msg_tag__pack_to_buffer
                     (const Ims__MsgTag   *message,
                      ProtobufCBuffer     *buffer);
Ims__MsgTag *
       qcril_qmi_ims__msg_tag__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__msg_tag__free_unpacked
                     (Ims__MsgTag *message,
                      ProtobufCAllocator *allocator);
/* Ims__CallFailCauseResponse methods */
void   qcril_qmi_ims__call_fail_cause_response__init
                     (Ims__CallFailCauseResponse         *message);
size_t qcril_qmi_ims__call_fail_cause_response__get_packed_size
                     (const Ims__CallFailCauseResponse   *message);
size_t qcril_qmi_ims__call_fail_cause_response__pack
                     (const Ims__CallFailCauseResponse   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__call_fail_cause_response__pack_to_buffer
                     (const Ims__CallFailCauseResponse   *message,
                      ProtobufCBuffer     *buffer);
Ims__CallFailCauseResponse *
       qcril_qmi_ims__call_fail_cause_response__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__call_fail_cause_response__free_unpacked
                     (Ims__CallFailCauseResponse *message,
                      ProtobufCAllocator *allocator);
/* Ims__StatusForAccessTech methods */
void   qcril_qmi_ims__status_for_access_tech__init
                     (Ims__StatusForAccessTech         *message);
size_t qcril_qmi_ims__status_for_access_tech__get_packed_size
                     (const Ims__StatusForAccessTech   *message);
size_t qcril_qmi_ims__status_for_access_tech__pack
                     (const Ims__StatusForAccessTech   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__status_for_access_tech__pack_to_buffer
                     (const Ims__StatusForAccessTech   *message,
                      ProtobufCBuffer     *buffer);
Ims__StatusForAccessTech *
       qcril_qmi_ims__status_for_access_tech__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__status_for_access_tech__free_unpacked
                     (Ims__StatusForAccessTech *message,
                      ProtobufCAllocator *allocator);
/* Ims__Info methods */
void   qcril_qmi_ims__info__init
                     (Ims__Info         *message);
size_t qcril_qmi_ims__info__get_packed_size
                     (const Ims__Info   *message);
size_t qcril_qmi_ims__info__pack
                     (const Ims__Info   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__info__pack_to_buffer
                     (const Ims__Info   *message,
                      ProtobufCBuffer     *buffer);
Ims__Info *
       qcril_qmi_ims__info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__info__free_unpacked
                     (Ims__Info *message,
                      ProtobufCAllocator *allocator);
/* Ims__SrvStatusList methods */
void   qcril_qmi_ims__srv_status_list__init
                     (Ims__SrvStatusList         *message);
size_t qcril_qmi_ims__srv_status_list__get_packed_size
                     (const Ims__SrvStatusList   *message);
size_t qcril_qmi_ims__srv_status_list__pack
                     (const Ims__SrvStatusList   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__srv_status_list__pack_to_buffer
                     (const Ims__SrvStatusList   *message,
                      ProtobufCBuffer     *buffer);
Ims__SrvStatusList *
       qcril_qmi_ims__srv_status_list__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__srv_status_list__free_unpacked
                     (Ims__SrvStatusList *message,
                      ProtobufCAllocator *allocator);
/* Ims__CallDetails methods */
void   qcril_qmi_ims__call_details__init
                     (Ims__CallDetails         *message);
size_t qcril_qmi_ims__call_details__get_packed_size
                     (const Ims__CallDetails   *message);
size_t qcril_qmi_ims__call_details__pack
                     (const Ims__CallDetails   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__call_details__pack_to_buffer
                     (const Ims__CallDetails   *message,
                      ProtobufCBuffer     *buffer);
Ims__CallDetails *
       qcril_qmi_ims__call_details__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__call_details__free_unpacked
                     (Ims__CallDetails *message,
                      ProtobufCAllocator *allocator);
/* Ims__CallModify methods */
void   qcril_qmi_ims__call_modify__init
                     (Ims__CallModify         *message);
size_t qcril_qmi_ims__call_modify__get_packed_size
                     (const Ims__CallModify   *message);
size_t qcril_qmi_ims__call_modify__pack
                     (const Ims__CallModify   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__call_modify__pack_to_buffer
                     (const Ims__CallModify   *message,
                      ProtobufCBuffer     *buffer);
Ims__CallModify *
       qcril_qmi_ims__call_modify__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__call_modify__free_unpacked
                     (Ims__CallModify *message,
                      ProtobufCAllocator *allocator);
/* Ims__CallList__Call methods */
void   qcril_qmi_ims__call_list__call__init
                     (Ims__CallList__Call         *message);
/* Ims__CallList methods */
void   qcril_qmi_ims__call_list__init
                     (Ims__CallList         *message);
size_t qcril_qmi_ims__call_list__get_packed_size
                     (const Ims__CallList   *message);
size_t qcril_qmi_ims__call_list__pack
                     (const Ims__CallList   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__call_list__pack_to_buffer
                     (const Ims__CallList   *message,
                      ProtobufCBuffer     *buffer);
Ims__CallList *
       qcril_qmi_ims__call_list__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__call_list__free_unpacked
                     (Ims__CallList *message,
                      ProtobufCAllocator *allocator);
/* Ims__Dial methods */
void   qcril_qmi_ims__dial__init
                     (Ims__Dial         *message);
size_t qcril_qmi_ims__dial__get_packed_size
                     (const Ims__Dial   *message);
size_t qcril_qmi_ims__dial__pack
                     (const Ims__Dial   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__dial__pack_to_buffer
                     (const Ims__Dial   *message,
                      ProtobufCBuffer     *buffer);
Ims__Dial *
       qcril_qmi_ims__dial__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__dial__free_unpacked
                     (Ims__Dial *message,
                      ProtobufCAllocator *allocator);
/* Ims__Hangup methods */
void   qcril_qmi_ims__hangup__init
                     (Ims__Hangup         *message);
size_t qcril_qmi_ims__hangup__get_packed_size
                     (const Ims__Hangup   *message);
size_t qcril_qmi_ims__hangup__pack
                     (const Ims__Hangup   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__hangup__pack_to_buffer
                     (const Ims__Hangup   *message,
                      ProtobufCBuffer     *buffer);
Ims__Hangup *
       qcril_qmi_ims__hangup__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__hangup__free_unpacked
                     (Ims__Hangup *message,
                      ProtobufCAllocator *allocator);
/* Ims__DeflectCall methods */
void   qcril_qmi_ims__deflect_call__init
                     (Ims__DeflectCall         *message);
size_t qcril_qmi_ims__deflect_call__get_packed_size
                     (const Ims__DeflectCall   *message);
size_t qcril_qmi_ims__deflect_call__pack
                     (const Ims__DeflectCall   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__deflect_call__pack_to_buffer
                     (const Ims__DeflectCall   *message,
                      ProtobufCBuffer     *buffer);
Ims__DeflectCall *
       qcril_qmi_ims__deflect_call__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__deflect_call__free_unpacked
                     (Ims__DeflectCall *message,
                      ProtobufCAllocator *allocator);
/* Ims__Clir methods */
void   qcril_qmi_ims__clir__init
                     (Ims__Clir         *message);
size_t qcril_qmi_ims__clir__get_packed_size
                     (const Ims__Clir   *message);
size_t qcril_qmi_ims__clir__pack
                     (const Ims__Clir   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__clir__pack_to_buffer
                     (const Ims__Clir   *message,
                      ProtobufCBuffer     *buffer);
Ims__Clir *
       qcril_qmi_ims__clir__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__clir__free_unpacked
                     (Ims__Clir *message,
                      ProtobufCAllocator *allocator);
/* Ims__Answer methods */
void   qcril_qmi_ims__answer__init
                     (Ims__Answer         *message);
size_t qcril_qmi_ims__answer__get_packed_size
                     (const Ims__Answer   *message);
size_t qcril_qmi_ims__answer__pack
                     (const Ims__Answer   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__answer__pack_to_buffer
                     (const Ims__Answer   *message,
                      ProtobufCBuffer     *buffer);
Ims__Answer *
       qcril_qmi_ims__answer__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__answer__free_unpacked
                     (Ims__Answer *message,
                      ProtobufCAllocator *allocator);
/* Ims__SwitchWaitingOrHoldingAndActive methods */
void   qcril_qmi_ims__switch_waiting_or_holding_and_active__init
                     (Ims__SwitchWaitingOrHoldingAndActive         *message);
size_t qcril_qmi_ims__switch_waiting_or_holding_and_active__get_packed_size
                     (const Ims__SwitchWaitingOrHoldingAndActive   *message);
size_t qcril_qmi_ims__switch_waiting_or_holding_and_active__pack
                     (const Ims__SwitchWaitingOrHoldingAndActive   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__switch_waiting_or_holding_and_active__pack_to_buffer
                     (const Ims__SwitchWaitingOrHoldingAndActive   *message,
                      ProtobufCBuffer     *buffer);
Ims__SwitchWaitingOrHoldingAndActive *
       qcril_qmi_ims__switch_waiting_or_holding_and_active__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__switch_waiting_or_holding_and_active__free_unpacked
                     (Ims__SwitchWaitingOrHoldingAndActive *message,
                      ProtobufCAllocator *allocator);
/* Ims__Mute methods */
void   qcril_qmi_ims__mute__init
                     (Ims__Mute         *message);
size_t qcril_qmi_ims__mute__get_packed_size
                     (const Ims__Mute   *message);
size_t qcril_qmi_ims__mute__pack
                     (const Ims__Mute   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__mute__pack_to_buffer
                     (const Ims__Mute   *message,
                      ProtobufCBuffer     *buffer);
Ims__Mute *
       qcril_qmi_ims__mute__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__mute__free_unpacked
                     (Ims__Mute *message,
                      ProtobufCAllocator *allocator);
/* Ims__Dtmf methods */
void   qcril_qmi_ims__dtmf__init
                     (Ims__Dtmf         *message);
size_t qcril_qmi_ims__dtmf__get_packed_size
                     (const Ims__Dtmf   *message);
size_t qcril_qmi_ims__dtmf__pack
                     (const Ims__Dtmf   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__dtmf__pack_to_buffer
                     (const Ims__Dtmf   *message,
                      ProtobufCBuffer     *buffer);
Ims__Dtmf *
       qcril_qmi_ims__dtmf__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__dtmf__free_unpacked
                     (Ims__Dtmf *message,
                      ProtobufCAllocator *allocator);
/* Ims__Registration methods */
void   qcril_qmi_ims__registration__init
                     (Ims__Registration         *message);
size_t qcril_qmi_ims__registration__get_packed_size
                     (const Ims__Registration   *message);
size_t qcril_qmi_ims__registration__pack
                     (const Ims__Registration   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__registration__pack_to_buffer
                     (const Ims__Registration   *message,
                      ProtobufCBuffer     *buffer);
Ims__Registration *
       qcril_qmi_ims__registration__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__registration__free_unpacked
                     (Ims__Registration *message,
                      ProtobufCAllocator *allocator);
/* Ims__RingBackTone methods */
void   qcril_qmi_ims__ring_back_tone__init
                     (Ims__RingBackTone         *message);
size_t qcril_qmi_ims__ring_back_tone__get_packed_size
                     (const Ims__RingBackTone   *message);
size_t qcril_qmi_ims__ring_back_tone__pack
                     (const Ims__RingBackTone   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__ring_back_tone__pack_to_buffer
                     (const Ims__RingBackTone   *message,
                      ProtobufCBuffer     *buffer);
Ims__RingBackTone *
       qcril_qmi_ims__ring_back_tone__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__ring_back_tone__free_unpacked
                     (Ims__RingBackTone *message,
                      ProtobufCAllocator *allocator);
/* Ims__IFConnected methods */
void   qcril_qmi_ims__ifconnected__init
                     (Ims__IFConnected         *message);
size_t qcril_qmi_ims__ifconnected__get_packed_size
                     (const Ims__IFConnected   *message);
size_t qcril_qmi_ims__ifconnected__pack
                     (const Ims__IFConnected   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__ifconnected__pack_to_buffer
                     (const Ims__IFConnected   *message,
                      ProtobufCBuffer     *buffer);
Ims__IFConnected *
       qcril_qmi_ims__ifconnected__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__ifconnected__free_unpacked
                     (Ims__IFConnected *message,
                      ProtobufCAllocator *allocator);
/* Ims__LastFailCause methods */
void   qcril_qmi_ims__last_fail_cause__init
                     (Ims__LastFailCause         *message);
size_t qcril_qmi_ims__last_fail_cause__get_packed_size
                     (const Ims__LastFailCause   *message);
size_t qcril_qmi_ims__last_fail_cause__pack
                     (const Ims__LastFailCause   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__last_fail_cause__pack_to_buffer
                     (const Ims__LastFailCause   *message,
                      ProtobufCBuffer     *buffer);
Ims__LastFailCause *
       qcril_qmi_ims__last_fail_cause__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__last_fail_cause__free_unpacked
                     (Ims__LastFailCause *message,
                      ProtobufCAllocator *allocator);
/* Ims__Extra methods */
void   qcril_qmi_ims__extra__init
                     (Ims__Extra         *message);
size_t qcril_qmi_ims__extra__get_packed_size
                     (const Ims__Extra   *message);
size_t qcril_qmi_ims__extra__pack
                     (const Ims__Extra   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__extra__pack_to_buffer
                     (const Ims__Extra   *message,
                      ProtobufCBuffer     *buffer);
Ims__Extra *
       qcril_qmi_ims__extra__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__extra__free_unpacked
                     (Ims__Extra *message,
                      ProtobufCAllocator *allocator);
/* Ims__Handover methods */
void   qcril_qmi_ims__handover__init
                     (Ims__Handover         *message);
size_t qcril_qmi_ims__handover__get_packed_size
                     (const Ims__Handover   *message);
size_t qcril_qmi_ims__handover__pack
                     (const Ims__Handover   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__handover__pack_to_buffer
                     (const Ims__Handover   *message,
                      ProtobufCBuffer     *buffer);
Ims__Handover *
       qcril_qmi_ims__handover__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__handover__free_unpacked
                     (Ims__Handover *message,
                      ProtobufCAllocator *allocator);

/* Ims__TtyNotify methods */
void   qcril_qmi_ims__tty_notify__init
                     (Ims__TtyNotify         *message);
size_t qcril_qmi_ims__tty_notify__get_packed_size
                     (const Ims__TtyNotify   *message);
size_t qcril_qmi_ims__tty_notify__pack
                     (const Ims__TtyNotify   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__tty_notify__pack_to_buffer
                     (const Ims__TtyNotify   *message,
                      ProtobufCBuffer     *buffer);
Ims__TtyNotify *
       qcril_qmi_ims__tty_notify__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__tty_notify__free_unpacked
                     (Ims__TtyNotify *message,
                      ProtobufCAllocator *allocator);

/* Ims__RadioStateChanged methods */
void   qcril_qmi_ims__radio_state_changed__init
                     (Ims__RadioStateChanged         *message);
size_t qcril_qmi_ims__radio_state_changed__get_packed_size
                     (const Ims__RadioStateChanged   *message);
size_t qcril_qmi_ims__radio_state_changed__pack
                     (const Ims__RadioStateChanged   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__radio_state_changed__pack_to_buffer
                     (const Ims__RadioStateChanged   *message,
                      ProtobufCBuffer     *buffer);
Ims__RadioStateChanged *
       qcril_qmi_ims__radio_state_changed__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__radio_state_changed__free_unpacked
                     (Ims__RadioStateChanged *message,
                      ProtobufCAllocator *allocator);

/* Ims__ServiceClass methods */
void   qcril_qmi_ims__service_class__init
                     (Ims__ServiceClass         *message);
size_t qcril_qmi_ims__service_class__get_packed_size
                     (const Ims__ServiceClass   *message);
size_t qcril_qmi_ims__service_class__pack
                     (const Ims__ServiceClass   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__service_class__pack_to_buffer
                     (const Ims__ServiceClass   *message,
                      ProtobufCBuffer     *buffer);
Ims__ServiceClass *
       qcril_qmi_ims__service_class__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__service_class__free_unpacked
                     (Ims__ServiceClass *message,
                      ProtobufCAllocator *allocator);
/* Ims__CbNumList methods */
void   qcril_qmi_ims__cb_num_list__init
                     (Ims__CbNumList         *message);
size_t qcril_qmi_ims__cb_num_list__get_packed_size
                     (const Ims__CbNumList   *message);
size_t qcril_qmi_ims__cb_num_list__pack
                     (const Ims__CbNumList   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__cb_num_list__pack_to_buffer
                     (const Ims__CbNumList   *message,
                      ProtobufCBuffer     *buffer);
Ims__CbNumList *
       qcril_qmi_ims__cb_num_list__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__cb_num_list__free_unpacked
                     (Ims__CbNumList *message,
                      ProtobufCAllocator *allocator);
/* Ims__CbNumListType methods */
void   qcril_qmi_ims__cb_num_list_type__init
                     (Ims__CbNumListType         *message);
size_t qcril_qmi_ims__cb_num_list_type__get_packed_size
                     (const Ims__CbNumListType   *message);
size_t qcril_qmi_ims__cb_num_list_type__pack
                     (const Ims__CbNumListType   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__cb_num_list_type__pack_to_buffer
                     (const Ims__CbNumListType   *message,
                      ProtobufCBuffer     *buffer);
Ims__CbNumListType *
       qcril_qmi_ims__cb_num_list_type__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__cb_num_list_type__free_unpacked
                     (Ims__CbNumListType *message,
                      ProtobufCAllocator *allocator);
/* Ims__CallWaitingInfo methods */
void   qcril_qmi_ims__call_waiting_info__init
                     (Ims__CallWaitingInfo         *message);
size_t qcril_qmi_ims__call_waiting_info__get_packed_size
                     (const Ims__CallWaitingInfo   *message);
size_t qcril_qmi_ims__call_waiting_info__pack
                     (const Ims__CallWaitingInfo   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__call_waiting_info__pack_to_buffer
                     (const Ims__CallWaitingInfo   *message,
                      ProtobufCBuffer     *buffer);
Ims__CallWaitingInfo *
       qcril_qmi_ims__call_waiting_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__call_waiting_info__free_unpacked
                     (Ims__CallWaitingInfo *message,
                      ProtobufCAllocator *allocator);
/* Ims__CallForwardInfoList__CallForwardInfo methods */
void   qcril_qmi_ims__call_forward_info_list__call_forward_info__init
                     (Ims__CallForwardInfoList__CallForwardInfo         *message);
/* Ims__CallForwardInfoList methods */
void   qcril_qmi_ims__call_forward_info_list__init
                     (Ims__CallForwardInfoList         *message);
size_t qcril_qmi_ims__call_forward_info_list__get_packed_size
                     (const Ims__CallForwardInfoList   *message);
size_t qcril_qmi_ims__call_forward_info_list__pack
                     (const Ims__CallForwardInfoList   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__call_forward_info_list__pack_to_buffer
                     (const Ims__CallForwardInfoList   *message,
                      ProtobufCBuffer     *buffer);
Ims__CallForwardInfoList *
       qcril_qmi_ims__call_forward_info_list__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__call_forward_info_list__free_unpacked
                     (Ims__CallForwardInfoList *message,
                      ProtobufCAllocator *allocator);
/* Ims__CallFwdTimerInfo methods */
void   qcril_qmi_ims__call_fwd_timer_info__init
                     (Ims__CallFwdTimerInfo         *message);
size_t qcril_qmi_ims__call_fwd_timer_info__get_packed_size
                     (const Ims__CallFwdTimerInfo   *message);
size_t qcril_qmi_ims__call_fwd_timer_info__pack
                     (const Ims__CallFwdTimerInfo   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__call_fwd_timer_info__pack_to_buffer
                     (const Ims__CallFwdTimerInfo   *message,
                      ProtobufCBuffer     *buffer);
Ims__CallFwdTimerInfo *
       qcril_qmi_ims__call_fwd_timer_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__call_fwd_timer_info__free_unpacked
                     (Ims__CallFwdTimerInfo *message,
                      ProtobufCAllocator *allocator);
/* Ims__ConfInfo methods */
void   qcril_qmi_ims__conf_info__init
                     (Ims__ConfInfo         *message);
size_t qcril_qmi_ims__conf_info__get_packed_size
                     (const Ims__ConfInfo   *message);
size_t qcril_qmi_ims__conf_info__pack
                     (const Ims__ConfInfo   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__conf_info__pack_to_buffer
                     (const Ims__ConfInfo   *message,
                      ProtobufCBuffer     *buffer);
Ims__ConfInfo *
       qcril_qmi_ims__conf_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__conf_info__free_unpacked
                     (Ims__ConfInfo *message,
                      ProtobufCAllocator *allocator);

/* Ims__SuppSvcNotification methods */
void   qcril_qmi_ims__supp_svc_notification__init
                     (Ims__SuppSvcNotification         *message);
size_t qcril_qmi_ims__supp_svc_notification__get_packed_size
                     (const Ims__SuppSvcNotification   *message);
size_t qcril_qmi_ims__supp_svc_notification__pack
                     (const Ims__SuppSvcNotification   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__supp_svc_notification__pack_to_buffer
                     (const Ims__SuppSvcNotification   *message,
                      ProtobufCBuffer     *buffer);
Ims__SuppSvcNotification *
       qcril_qmi_ims__supp_svc_notification__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__supp_svc_notification__free_unpacked
                     (Ims__SuppSvcNotification *message,
                      ProtobufCAllocator *allocator);

/* Ims__SuppSvcStatus methods */
void   qcril_qmi_ims__supp_svc_status__init
                     (Ims__SuppSvcStatus         *message);
size_t qcril_qmi_ims__supp_svc_status__get_packed_size
                     (const Ims__SuppSvcStatus   *message);
size_t qcril_qmi_ims__supp_svc_status__pack
                     (const Ims__SuppSvcStatus   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__supp_svc_status__pack_to_buffer
                     (const Ims__SuppSvcStatus   *message,
                      ProtobufCBuffer     *buffer);
Ims__SuppSvcStatus *
       qcril_qmi_ims__supp_svc_status__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__supp_svc_status__free_unpacked
                     (Ims__SuppSvcStatus *message,
                      ProtobufCAllocator *allocator);

/* Ims__SuppSvcRequest methods */
void   qcril_qmi_ims__supp_svc_request__init
                     (Ims__SuppSvcRequest         *message);
size_t qcril_qmi_ims__supp_svc_request__get_packed_size
                     (const Ims__SuppSvcRequest   *message);
size_t qcril_qmi_ims__supp_svc_request__pack
                     (const Ims__SuppSvcRequest   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__supp_svc_request__pack_to_buffer
                     (const Ims__SuppSvcRequest   *message,
                      ProtobufCBuffer     *buffer);
Ims__SuppSvcRequest *
       qcril_qmi_ims__supp_svc_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__supp_svc_request__free_unpacked
                     (Ims__SuppSvcRequest *message,
                      ProtobufCAllocator *allocator);

/* Ims__SuppSvcResponse methods */
void   qcril_qmi_ims__supp_svc_response__init
                     (Ims__SuppSvcResponse         *message);
size_t qcril_qmi_ims__supp_svc_response__get_packed_size
                     (const Ims__SuppSvcResponse   *message);
size_t qcril_qmi_ims__supp_svc_response__pack
                     (const Ims__SuppSvcResponse   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__supp_svc_response__pack_to_buffer
                     (const Ims__SuppSvcResponse   *message,
                      ProtobufCBuffer     *buffer);
Ims__SuppSvcResponse *
       qcril_qmi_ims__supp_svc_response__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__supp_svc_response__free_unpacked
                     (Ims__SuppSvcResponse *message,
                      ProtobufCAllocator *allocator);

/* Ims__ClipProvisionStatus methods */
void   qcril_qmi_ims__clip_provision_status__init
                     (Ims__ClipProvisionStatus         *message);
size_t qcril_qmi_ims__clip_provision_status__get_packed_size
                     (const Ims__ClipProvisionStatus   *message);
size_t qcril_qmi_ims__clip_provision_status__pack
                     (const Ims__ClipProvisionStatus   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__clip_provision_status__pack_to_buffer
                     (const Ims__ClipProvisionStatus   *message,
                      ProtobufCBuffer     *buffer);
Ims__ClipProvisionStatus *
       qcril_qmi_ims__clip_provision_status__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__clip_provision_status__free_unpacked
                     (Ims__ClipProvisionStatus *message,
                      ProtobufCAllocator *allocator);

/* Ims__Colr methods */
void   qcril_qmi_ims__colr__init
                     (Ims__Colr         *message);
size_t qcril_qmi_ims__colr__get_packed_size
                     (const Ims__Colr *message);
size_t qcril_qmi_ims__colr__pack
                     (const Ims__Colr *message,
                      uint8_t       *out);
size_t qcril_qmi_ims__colr__pack_to_buffer
                     (const Ims__Colr *message,
                      ProtobufCBuffer *buffer);
Ims__Colr *
       qcril_qmi_ims__colr__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__colr__free_unpacked
                     (Ims__Colr *message,
                      ProtobufCAllocator *allocator);

/* Ims__VideoCallQuality methods */
void   qcril_qmi_ims__video_call_quality__init
                     (Ims__VideoCallQuality         *message);
size_t qcril_qmi_ims__video_call_quality__get_packed_size
                     (const Ims__VideoCallQuality *message);
size_t qcril_qmi_ims__video_call_quality__pack
                     (const Ims__VideoCallQuality *message,
                      uint8_t       *out);
size_t qcril_qmi_ims__video_call_quality__pack_to_buffer
                     (const Ims__VideoCallQuality *message,
                      ProtobufCBuffer *buffer);
Ims__VideoCallQuality *
       qcril_qmi_ims__video_call_quality__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__video_call_quality__free_unpacked
                     (Ims__VideoCallQuality *message,
                      ProtobufCAllocator *allocator);

/* Ims__MwiMessageSummary methods */
void   qcril_qmi_ims__mwi_message_summary__init
                     (Ims__MwiMessageSummary         *message);
size_t qcril_qmi_ims__mwi_message_summary__get_packed_size
                     (const Ims__MwiMessageSummary   *message);
size_t qcril_qmi_ims__mwi_message_summary__pack
                     (const Ims__MwiMessageSummary   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__mwi_message_summary__pack_to_buffer
                     (const Ims__MwiMessageSummary   *message,
                      ProtobufCBuffer     *buffer);
Ims__MwiMessageSummary *
       qcril_qmi_ims__mwi_message_summary__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__mwi_message_summary__free_unpacked
                     (Ims__MwiMessageSummary *message,
                      ProtobufCAllocator *allocator);
/* Ims__MwiMessageDetails methods */
void   qcril_qmi_ims__mwi_message_details__init
                     (Ims__MwiMessageDetails         *message);
size_t qcril_qmi_ims__mwi_message_details__get_packed_size
                     (const Ims__MwiMessageDetails   *message);
size_t qcril_qmi_ims__mwi_message_details__pack
                     (const Ims__MwiMessageDetails   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__mwi_message_details__pack_to_buffer
                     (const Ims__MwiMessageDetails   *message,
                      ProtobufCBuffer     *buffer);
Ims__MwiMessageDetails *
       qcril_qmi_ims__mwi_message_details__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__mwi_message_details__free_unpacked
                     (Ims__MwiMessageDetails *message,
                      ProtobufCAllocator *allocator);
/* Ims__Mwi methods */
void   qcril_qmi_ims__mwi__init
                     (Ims__Mwi         *message);
size_t qcril_qmi_ims__mwi__get_packed_size
                     (const Ims__Mwi   *message);
size_t qcril_qmi_ims__mwi__pack
                     (const Ims__Mwi   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__mwi__pack_to_buffer
                     (const Ims__Mwi   *message,
                      ProtobufCBuffer     *buffer);
Ims__Mwi *
       qcril_qmi_ims__mwi__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__mwi__free_unpacked
                     (Ims__Mwi *message,
                      ProtobufCAllocator *allocator);
/* Ims__Hold methods */
void   qcril_qmi_ims__hold__init
                     (Ims__Hold         *message);
size_t qcril_qmi_ims__hold__get_packed_size
                     (const Ims__Hold   *message);
size_t qcril_qmi_ims__hold__pack
                     (const Ims__Hold   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__hold__pack_to_buffer
                     (const Ims__Hold   *message,
                      ProtobufCBuffer     *buffer);
Ims__Hold *
       qcril_qmi_ims__hold__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__hold__free_unpacked
                     (Ims__Hold *message,
                      ProtobufCAllocator *allocator);
/* Ims__Resume methods */
void   qcril_qmi_ims__resume__init
                     (Ims__Resume         *message);
size_t qcril_qmi_ims__resume__get_packed_size
                     (const Ims__Resume   *message);
size_t qcril_qmi_ims__resume__pack
                     (const Ims__Resume   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__resume__pack_to_buffer
                     (const Ims__Resume   *message,
                      ProtobufCBuffer     *buffer);
Ims__Resume *
       qcril_qmi_ims__resume__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__resume__free_unpacked
                     (Ims__Resume *message,
                      ProtobufCAllocator *allocator);
/* Ims__RtpStatisticsData methods */
void   qcril_qmi_ims__rtp_statistics_data__init
                     (Ims__RtpStatisticsData         *message);
size_t qcril_qmi_ims__rtp_statistics_data__get_packed_size
                     (const Ims__RtpStatisticsData   *message);
size_t qcril_qmi_ims__rtp_statistics_data__pack
                     (const Ims__RtpStatisticsData   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__rtp_statistics_data__pack_to_buffer
                     (const Ims__RtpStatisticsData   *message,
                      ProtobufCBuffer     *buffer);
Ims__RtpStatisticsData *
       qcril_qmi_ims__rtp_statistics_data__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__rtp_statistics_data__free_unpacked
                     (Ims__RtpStatisticsData *message,
                      ProtobufCAllocator *allocator);
/* Ims__WifiCallingInfo methods */
void   qcril_qmi_ims__wifi_calling_info__init
                     (Ims__WifiCallingInfo         *message);
size_t qcril_qmi_ims__wifi_calling_info__get_packed_size
                     (const Ims__WifiCallingInfo   *message);
size_t qcril_qmi_ims__wifi_calling_info__pack
                     (const Ims__WifiCallingInfo   *message,
                      uint8_t             *out);
size_t qcril_qmi_ims__wifi_calling_info__pack_to_buffer
                     (const Ims__WifiCallingInfo   *message,
                      ProtobufCBuffer     *buffer);
Ims__WifiCallingInfo *
       qcril_qmi_ims__wifi_calling_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   qcril_qmi_ims__wifi_calling_info__free_unpacked
                     (Ims__WifiCallingInfo *message,
                      ProtobufCAllocator *allocator);
#endif /* QCRIL_QMI_IMS_IF_PB_H */
