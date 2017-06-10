#include "dm_internal.h"

/*****************************************************************************/
//  Description : vdm run
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC void MMIDM_Run(void) {
    DM_TRACE("MMIDM==>  MMIDM_Run ");
    VDM_run();
}

/*****************************************************************************/
//  Description : dm destroy
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC void MMIDM_Destroy(void) {
    DM_TRACE("MMIDM==>  MMIDM_Destroy ");

}

/*****************************************************************************/
//  Description : dm SessionCancel
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC void MMIDM_SessionCancel(void) {
    DM_TRACE("MMIDM==>  MMIDM_SessionCancel ");

}

/*****************************************************************************/
//  Description : dm  NotifyNIASessionProceed
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_NotifyNIASessionProceed(void) {
    return VDM_notifyNIASessionProceed();
}

/*****************************************************************************/
//  Description : dm  MMIDM_registerSessionStateObserver
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_RegisterSessionStateObserver(
        VDM_SessionStateNotifyCB inHandlerFunc) {
    return VDM_registerSessionStateObserver(inHandlerFunc);
}
/*****************************************************************************/
//  Description : dm  MMIDM_RegisterResume
//  Global resource dependence :
//  Note:
/*****************************************************************************/
//PUBLIC void MMIDM_RegisterResume(VDM_DL_resumeGetFunc getFunc, VDM_DL_resumeSetFunc setFunc)
//{
//
//}
/*****************************************************************************/
//  Description : dm  MMIDM_registerSessionStateObserver
//  Global resource dependence :
//  Note:
/*****************************************************************************/
//PUBLIC VDM_Error MMIDM_ConfigSetNotificationVerificationMode(E_VDM_NotificationVerificationMode_t inMode)
//{
//    return VDM_ERR_OK;
//}
/*****************************************************************************/
//  Description : dm  MMIDM_ConfigSetSessionIDAsDec
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_ConfigSetSessionIDAsDec(IBOOL inSessionIDAsDec) {
    return VDM_ERR_OK;
}
/*****************************************************************************/
//  Description : dm  MMIDM_UTL_Logger_setDefaultLevel
//  Global resource dependence :
//  Note:
/*****************************************************************************/
//PUBLIC VDM_Error MMIDM_UTL_Logger_setDefaultLevel( E_VDM_LOGLEVEL_TYPE inLevel)
//{
//    return VDM_ERR_OK;
//}
/*****************************************************************************/
//  Description : dm  MMIDM_Config_setMaxObjSize
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_Config_setMaxObjSize(IU32 inMaxObjSize) {
    return VDM_ERR_OK;
}
/*****************************************************************************/
//  Description : dm  MMIDM_Config_setMaxMsgSize
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_Config_setMaxMsgSize(IU32 inMaxMsgSize) {
    return VDM_ERR_OK;
}
/*****************************************************************************/
//  Description : dm  MMIDM_Config_setEncodeWBXMLMsg
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_Config_setEncodeWBXMLMsg(IBOOL inIsEncode) {
    return VDM_ERR_OK;
}
/*****************************************************************************/
//  Description : dm  MMIDM_Config_setCheckDDVersion
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_Config_setCheckDDVersion(IBOOL inDDVersionCheck) {
    return VDM_ERR_OK;
}

/*****************************************************************************/
//  Description : dm  MMIDM_TriggerNIADMSession
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_TriggerNIADMSession(IU8* inMessage, IU32 inMessageLen,
        VDM_NIAHandlerCB inHandlerFunc, VDM_SessionContext_t* inSessionContext) {
    return VDM_triggerNIADMSession(inMessage, inMessageLen, inHandlerFunc,
            inSessionContext);
}
/*****************************************************************************/
//  Description : dm  MMIDM_TriggerDMSession
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_TriggerDMSession(UTF8CStr inPath,
        UTF8CStr inGenericAlertType, IU8* inMessage, IU32 inMessageLen,
        VDM_SessionContext_t* inSessionContext) {
    return VDM_ERR_OK;
}

