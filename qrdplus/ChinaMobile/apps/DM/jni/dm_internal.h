#ifndef _DM_INTERNAL_H_
#define _DM_INTERNAL_H_

#include "sci_types.h"
#include "vdm_pl_types.h"
#include "vdm_error.h"
#include "dm_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
//  Description : start dm
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC DM_START_RESULT_E MMIDM_StartVDM(DM_SESSION_TYPE type, char* msg_body,
        uint32 msg_size);

/*****************************************************************************/
//  Description : vdm run
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC void MMIDM_Run(void);

/*****************************************************************************/
//  Description : dm destroy
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC void MMIDM_Destroy(void);

/*****************************************************************************/
//  Description : dm SessionCancel
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC void MMIDM_SessionCancel(void);

/*****************************************************************************/
//  Description : dm  NotifyNIASessionProceed
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_NotifyNIASessionProceed(void);

/*****************************************************************************/
//  Description : dm  MMIDM_registerSessionStateObserver
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_RegisterSessionStateObserver(
        VDM_SessionStateNotifyCB inHandlerFunc);

/*****************************************************************************/
//  Description : dm  MMIDM_ConfigSetSessionIDAsDec
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_ConfigSetSessionIDAsDec(IBOOL inSessionIDAsDec);

/*****************************************************************************/
//  Description : dm  MMIDM_Config_setMaxObjSize
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_Config_setMaxObjSize(IU32 inMaxObjSize);

/*****************************************************************************/
//  Description : dm  MMIDM_Config_setMaxMsgSize
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_Config_setMaxMsgSize(IU32 inMaxMsgSize);

/*****************************************************************************/
//  Description : dm  MMIDM_Config_setEncodeWBXMLMsg
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_Config_setEncodeWBXMLMsg(IBOOL inIsEncode);

/*****************************************************************************/
//  Description : dm  MMIDM_Config_setCheckDDVersion
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_Config_setCheckDDVersion(IBOOL inDDVersionCheck);

/*****************************************************************************/
//  Description : dm  MMIDM_TriggerNIADMSession
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_TriggerNIADMSession(IU8* inMessage, IU32 inMessageLen,
        VDM_NIAHandlerCB inHandlerFunc, VDM_SessionContext_t* inSessionContext);
/*****************************************************************************/
//  Description : dm  MMIDM_TriggerDMSession
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC VDM_Error MMIDM_TriggerDMSession(UTF8CStr inPath,
        UTF8CStr inGenericAlertType, IU8* inMessage, IU32 inMessageLen,
        VDM_SessionContext_t* inSessionContext);

#ifdef __cplusplus
}
#endif

#endif

