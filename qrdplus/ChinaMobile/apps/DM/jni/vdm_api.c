#include "vdm_pl_types.h"
#include "vdm_error.h"
#include "vdm_types.h"
#include "dm_task.h"
//#include <utils/Log.h>
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"DMLIB ==>",__VA_ARGS__)

//#define LOGE
#define DM_TRACE LOGE

/*
 ** SMS Notify Index
 */
#define SMS_NOTIFY_MSG_VERSION_INDEX        0
#define SMS_NOTIFY_MSG_UI_MODE_INDEX        1
#define SMS_NOTIFY_MSG_HI_SESSION_ID_INDEX  5
#define SMS_NOTIFY_MSG_LO_SESSION_ID_INDEX  6
#define SMS_NOTIFY_MSG_SERVER_ID_LEN_INDEX  7
#define SMS_NOTIFY_MSG_SERVER_ID_INDEX      8

#define SMS_NOTIFY_MSG_BODY_MIN_LEN         24 //²»°üº¬Server IdendifierµÄtrigger header length
#define SMS_NOTIFY_MSG_DIGEST_LEN           16 //md5 Digest length
#define SMS_NOTIFY_MSG_SERVER_ID_MAX_LEN    32

#define SMS_NOTIFY_MSG_DM_VERSION           0xB // dm version: 0000001011 (1.1)
typedef struct _MMIDM_NOTIFYMSG_STRUCT {
    IU8 md5Digest[SMS_NOTIFY_MSG_DIGEST_LEN];
    E_VDM_NIA_UIMode_t uiMode;
    IU16 dmVersion;
    IU16 sessionId;
    IU8 serverId[SMS_NOTIFY_MSG_SERVER_ID_MAX_LEN];
    IU8 *vendorSpecificData;
    IU16 verdorSpecificDataLen;
} MMIDM_NOTIFYMSG_STRUCT_T;

extern task_relay_info_type* dm_task_relay_info;

MMIDM_NOTIFYMSG_STRUCT_T s_dm_parse_notifyMsg = { 0 };

/*****************************************************************************/
//  Description :  Notification message
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
IBOOL parseDmNotifyMsg(IU8* smsMsgBody,      //in
        IU32 smsMsgBodyLen,   //in
        MMIDM_NOTIFYMSG_STRUCT_T *notifyMsgPtr  //out
        ) {
    IBOOL result = FALSE;
    IU16 serverIdLen = 0;
    IU16 i = 0;

    DM_TRACE("parseDmNotifyMsg---Enter! smsMsgBodyLen = %d", smsMsgBodyLen);
    for (i = 0; i < smsMsgBodyLen; i++) {
        DM_TRACE("smsMsgBody[%d] = 0x%x", i, smsMsgBody[i]);
    }

    if ((NULL == notifyMsgPtr) || (NULL == smsMsgBody)
            || (smsMsgBodyLen < SMS_NOTIFY_MSG_BODY_MIN_LEN)) {
        DM_TRACE("parseDmNotifyMsg---invalidate parameter!");
        return result;
    }

    memset(notifyMsgPtr, 0, sizeof(notifyMsgPtr));
    memcpy(notifyMsgPtr->md5Digest, smsMsgBody, SMS_NOTIFY_MSG_DIGEST_LEN);
    smsMsgBody += SMS_NOTIFY_MSG_DIGEST_LEN;
    notifyMsgPtr->uiMode =
            (E_VDM_NIA_UIMode_t) (smsMsgBody[SMS_NOTIFY_MSG_UI_MODE_INDEX] >> 4)
                    & 0x03;
    DM_TRACE("parseDmNotifyMsg---notifyMsgPtr->uiMode = %d", notifyMsgPtr->uiMode);
    notifyMsgPtr->dmVersion = ((IU16) (smsMsgBody[SMS_NOTIFY_MSG_VERSION_INDEX])
            << 2) + (IU16) (smsMsgBody[SMS_NOTIFY_MSG_VERSION_INDEX + 1] >> 6);
    DM_TRACE("parseDmNotifyMsg---notifyMsgPtr->dmVersion = %d", notifyMsgPtr->dmVersion);
    notifyMsgPtr->sessionId =
            (IU16) (((IU16) (smsMsgBody[SMS_NOTIFY_MSG_HI_SESSION_ID_INDEX])
                    << 8)
                    + (IU16) (smsMsgBody[SMS_NOTIFY_MSG_LO_SESSION_ID_INDEX]));
    DM_TRACE("parseDmNotifyMsg---notifyMsgPtr->sessionId = %d", notifyMsgPtr->sessionId);

    serverIdLen = (IU16) smsMsgBody[SMS_NOTIFY_MSG_SERVER_ID_LEN_INDEX];
    DM_TRACE("parseDmNotifyMsg---serverIdLen = %d", serverIdLen);
    if (smsMsgBodyLen >= (SMS_NOTIFY_MSG_BODY_MIN_LEN + serverIdLen)) {
        memcpy(notifyMsgPtr->serverId,
                &(smsMsgBody[SMS_NOTIFY_MSG_SERVER_ID_INDEX]), serverIdLen);
        DM_TRACE("parseDmNotifyMsg---notifyMsgPtr->serverId = %s", notifyMsgPtr->serverId);

        notifyMsgPtr->verdorSpecificDataLen = smsMsgBodyLen
                - (SMS_NOTIFY_MSG_BODY_MIN_LEN + serverIdLen);
        DM_TRACE("parseDmNotifyMsg---notifyMsgPtr->verdorSpecificDataLen = %d", notifyMsgPtr->verdorSpecificDataLen);
        if (notifyMsgPtr->verdorSpecificDataLen > 0) {
            notifyMsgPtr->vendorSpecificData = malloc(
                    notifyMsgPtr->verdorSpecificDataLen + 1);
            if (NULL != notifyMsgPtr->vendorSpecificData) {
                memcpy(notifyMsgPtr->vendorSpecificData,
                        &(smsMsgBody[SMS_NOTIFY_MSG_BODY_MIN_LEN + serverIdLen]),
                        notifyMsgPtr->verdorSpecificDataLen);
            }
        }

        result = TRUE;
    } else {
        DM_TRACE("parseDmNotifyMsg---smsMsgBodyLen = %d is invalid!!!", smsMsgBodyLen);
    }

    return result;
}
/*****************************************************************************/
//  Description :judge if the phone's inner server id
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
IBOOL MMIDM_IsPreConfiguredServerId(IS8 *serverId) {
    IBOOL result = FALSE;
    IS8 *accServerId = dmTreeDmAcc_getServerList(); //inner server id

    if ((NULL != accServerId) && (NULL != serverId)
            && (0 == dm_smlLibStrcmp(accServerId, serverId))) {
        result = TRUE;
    }
    DM_TRACE("MMIDM_IsPreConfiguredServerId: return %d", result);
    //dm_debug_trace(MVA_DEBUG_DMMGR_MASK, "MMIDM_IsPreConfiguredServerId: return %d", result);
    return result;
}

/*!
 *******************************************************************************
 * Trigger a Notification Initiated DM session.
 *
 * Client should Call this function upon a reception of
 * General Notification Initiated Session Alert.
 *
 * Note: To be notified on DM session state changes, register as an observer
 * observer \ref VDM_registerSessionStateObserver. A session state observer
 * must implement \ref VDM_SessionStateNotifyCB callback.
 *
 * \param   inMessage           notification message content.
 * \param   inMessageLen        size of inMessage.
 * \param   inHandlerFunc       callback function that will be called once
 *                              inMessage is parsed and before a connection
 *                              is opened.
 * \param   inSessionContext    Context that will be sent as a param of the
 *                              notification callback.
 *
 * \return  VDM error code
 *******************************************************************************
 */
VDM_Error VDM_triggerNIADMSession(IU8* inMessage, IU32 inMessageLen,
        VDM_NIAHandlerCB inHandlerFunc, VDM_SessionContext_t* inSessionContext) {
    MMIDM_NOTIFYMSG_STRUCT_T notifyMsg = { 0 };
    VDM_Error result = VDM_ERR_OK;

    if (parseDmNotifyMsg(inMessage, inMessageLen, &notifyMsg)) {
        DM_TRACE("VDM_triggerNIADMSession---parse success ! \r\n");
        if ((SMS_NOTIFY_MSG_DM_VERSION == notifyMsg.dmVersion)
                && MMIDM_IsPreConfiguredServerId(notifyMsg.serverId)) {
            DM_TRACE("VDM_triggerNIADMSession---dm version ok ! \r\n");
            memcpy(&s_dm_parse_notifyMsg, &notifyMsg,
                    sizeof(MMIDM_NOTIFYMSG_STRUCT_T));

            if (NULL != inHandlerFunc) {
                inHandlerFunc(notifyMsg.uiMode, notifyMsg.dmVersion,
                        notifyMsg.vendorSpecificData,
                        notifyMsg.verdorSpecificDataLen, inSessionContext);
            }
        } else {
            DM_TRACE("VDM_triggerNIADMSession---notifyMsg.dmVersion = %d is invalid!!!\r\n", notifyMsg.dmVersion);
            result = VDM_ERR_INVALID_CALL;
        }
    } else {
        result = VDM_ERR_INVALID_CALL;
    }

    return result;
}

/*****************************************************************************/
//  Description : get session id
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
IU16 MMIDM_GetNotifySessionId() {
    return s_dm_parse_notifyMsg.sessionId;
}

/*****************************************************************************/
//  Description :get sever id
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
IU8 * MMIDM_GetNotifySeverId() {
    return s_dm_parse_notifyMsg.serverId;
}

VDM_Error VDM_create(void* inAsyncHandle) {
    return VDM_ERR_OK;
}

VDM_Error VDM_start(void) {
    return VDM_ERR_OK;
}

void VDM_destroy(void) {
}

VDM_Error VDM_run(void) {
    dm_task_handle();
    return VDM_ERR_OK;
}

VDM_Error VDM_setSessionEnableMode(E_VDM_SessionEnable_t inMode) {
    return VDM_ERR_OK;
}

E_VDM_SessionEnable_t VDM_getSessionEnableMode(void) {
    return 0;
}

void VDM_setCurrentAccount(UTF8CStr inAccount) {
}

UTF8CStr VDM_getCurrentAccount(void) {
    return 0;
}

VDM_SessionContext_t* VDM_createSessionContext(UTF8CStr inInitiatorId,
        void* inData) {
    return 0;
}

VDM_Error VDM_triggerDLSession(UTF8CStr inPath,
        VDM_DownloadPromptCB inDLPromptFunc,
        VDM_HandleDLContentCB inDLDataHandlerFunc,
        VDM_SessionContext_t* inSessionContext) {
    return VDM_ERR_OK;
}

VDM_Error VDM_triggerDMSession(UTF8CStr inPath, UTF8CStr inGenericAlertType,
        IU8* inMessage, IU32 inMessageLen,
        VDM_SessionContext_t* inSessionContext) {
    return VDM_ERR_OK;
}

VDM_Error VDM_triggerReportSession(UTF8CStr inPath, IU32 inReasonCode,
        UTF8CStr inAccount, UTF8CStr inGenericAlertType, UTF8CStr inCorrelator,
        VDM_SessionContext_t* inSessionContext) {
    return VDM_ERR_OK;
}

VDM_Error VDM_triggerMultipleGenericAlertsSession(UTF8CStr inAccount,
        VDM_CreateGenericAlertsCB inCreateGenAlertsFunc,
        VDM_SessionContext_t* inSessionContext) {
    return VDM_ERR_OK;
}

VDM_Error VDM_triggerBootstrapSession(UTF8CStr inAccount,
        E_VDM_Prov_Profile_t inProfile,
        E_VDM_Prov_Security_t inSecurityMechanism, UTF8CStr inMAC,
        IU8* inBuffer, IU32 inBufferLen, VDM_BootCallbacks_t * inCallbacks,
        void* inCallbacksContext, VDM_SessionContext_t* inSessionContext) {
    return VDM_ERR_OK;
}

VDM_Error VDM_registerSessionStateObserver(
        VDM_SessionStateNotifyCB inHandlerFunc) {
    dm_task_relay_info->session_state_notify_cb = inHandlerFunc;
    DM_TRACE("VDM_registerSessionStateObserver---inHandlerFunc = %d!!!\r\n", inHandlerFunc);
    return VDM_ERR_OK;
}

VDM_Error VDM_unregisterSessionStateObserver(
        VDM_SessionStateNotifyCB inHandlerFunc) {
    return VDM_ERR_OK;
}

void VDM_registerCPObserver(VDM_CPNotifyCB inObserverFunc) {
}

VDM_Error VDM_cancelSession(void) {
    return VDM_ERR_OK;
}

IBOOL VDM_isIdle(void) {
    return 0;
}

VDM_Error VDM_triggerDSSession(VDM_SessionContext_t* inSessionContext) {
    return VDM_ERR_OK;
}

VDM_Error VDM_notifyNIASessionProceed(void) {
    dm_task_init();
    dm_task_relay_info->proc_step = SYNC_PROC_NORMAL;
    //   VDM_PL_Async_signal(dm_task_relay_info->task_handle);
    DMTaskRun();
    return VDM_ERR_OK;
}

UTF8CStr VDM_getVersion(void) {
    return 0;
}

void VDM_DbgDump(void) {
}

IBOOL VDM_debugMemory(char* inMemFailureData) {
    return 0;
}

