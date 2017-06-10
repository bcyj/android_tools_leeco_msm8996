#include "sci_types.h"
#include "dm_config.h"
#include "dm_internal.h"
#include "dm_app.h"
/**--------------------------------------------------------------------------*
 **                         MACRO DEFINITION                                 *
 **--------------------------------------------------------------------------*/
#define     DM_STACK_SIZE                 (1024*30)           //dm task stack size
#define     DM_STACK_QUEUE_NUM            (120)               //dm task stack queue number
#define     DM_TASK_PRIORITY              (30)               //dm task priority
#define     DM_PDP_WAITING_TIME           1000             //dm waiting pdp time
/**--------------------------------------------------------------------------*
 **                         STATIC DEFINITION                                *
 **--------------------------------------------------------------------------*/LOCAL uint32 s_dm_task_id =
        0;                     //dm task id
LOCAL uint32 s_pdp_net_id = 0;

/*****************************************************************************/
//  Description : active pdp connect
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC BOOLEAN MMIDM_ActivePdpConnect(void) {
    BOOLEAN result = FALSE;

    if (MMIDM_PDP_STATUS_CONNECT != JgetPppConnectStatus()) {
        DM_TRACE("JgetPppConnectStatus  Need to Activet PdpContex    ");
        if (MMIDM_PDP_STATUS_CONNECT == JstartPppConnect()) {
            result = TRUE;
        } else {
            result = FALSE;
        }
    } else {
        DM_TRACE("JgetPppConnectStatus  PdpContex  IS  Activet ");
        result = TRUE;
    }
    return result;
}

/*****************************************************************************/
//  Description : handle pdp active cnf
//  Global resource dependence :
//  Note:
/*****************************************************************************/
//LOCAL void HandlePdpActiveCnf(MMIPDP_CNF_INFO_T *msg_ptr)
//{
//    DM_TRACE("MMIDM==> pdp active result =%d",msg_ptr->result);
//
//    s_pdp_net_id = msg_ptr->nsapi;
//    //get pdp active cnf
//    if (MMIPDP_RESULT_SUCC == msg_ptr->result)
//    {
//        //pdp active success
//        SetPdpActiveStatus(MMIDM_PDP_STATUS_CONNECT);
//    }
//    else
//    {
//        SetPdpActiveStatus(MMIDM_PDP_STATUS_ABORT);
//    }
//
//}
static BOOLEAN is_on_free = FALSE;
;

void DMTaskStart(MMIDM_SESSION_T* signal_ptr) {
    DM_START_RESULT_E result = DM_START_NONE;

    DM_TRACE("MMIDM==> task DM_TASK_START_MESSAGE ");
    result = MMIDM_StartVDM(signal_ptr->type, signal_ptr->message_buffer,
            signal_ptr->msg_size);
    DM_TRACE("MMIDM==> task DM_TASK_START_MESSAGE result=%d",result);
    switch (result) {
    case DM_START_FAIL:
        MMIDM_SetDmState(DM_CANCEL);
        //      MMIDM_CloseWaitingWin();
        //      MMIDM_SendSigToDmTask(DM_TASK_DM_EXIT,MMIDM_GetDmTaskID(),PNULL);
        DMTaskCancel();
        break;
    case DM_START_DONE:
        //      MMIDM_CloseWaitingWin();
        //      MMIDM_SendSigToDmTask(DM_TASK_EXIT_MESSAGE,MMIDM_GetDmTaskID(),PNULL);
        DMTaskExit();
        break;
    default:
        break;
    }
    DM_TRACE("MMIDM==> task DM_TASK_START_MESSAGE free start");
    if (NULL != signal_ptr->message_buffer) {
        free(signal_ptr->message_buffer);
    }
    DM_TRACE("MMIDM==> task DM_TASK_START_MESSAGE free end");

}

void DMTaskRun() {
    DM_TRACE("MMIDM==> task DM_TASK_RUN_MESSAGE ");
    if (DM_NONE != MMIDM_GetDmState() && !is_on_free) {
        MMIDM_Run();
    }
}

//void DMTaskExit() {
//  MMIDM_ExitDM();
//}

void DMTaskCancel() {
    MMIDM_SetDmState(DM_CANCEL);
}

void DMTaskExit() {
    if (DM_CANCEL == MMIDM_GetDmState()) {
        MMIDM_ExitDM();
        MMIDM_SetDmState(DM_CANCEL);
    }
}

void DMTaskTerminate() {
    if (DM_NONE != MMIDM_GetDmState() && !is_on_free) {
        is_on_free = TRUE;
//      MMIDM_FreeAllMOInstance();
//      MMIDM_Destroy();
//      MMIDM_DeactivePdpContext();
//      MMIDM_DeleteAllResumeFile();
//      MMIDM_SetDmState(DM_NONE);
//      is_on_free = FALSE;
//      if (MMIDM_IsNeedReset())
//      {
//          MMIAPIPHONE_PowerReset();
//      }
//      MMIDM_SendSignalToMMITask(APP_DM_MESSAGE_IND,MSG_DM_SESSION_EXIT,PNULL);
    }
}

/*****************************************************************************/
//  Description : handle dm task
//  Global resource dependence :
//  Note:
/*****************************************************************************/
//LOCAL void DM_TaskHandle(uint32 argc, void * argv)
//{
//    DM_SIGNAL_T                 *signal_ptr = PNULL;
//    BLOCK_ID                    task_id = s_dm_task_id;
//    SOCKET_CONNECT_EVENT_IND_SIG_T*       connect_ind_sig_ptr = PNULL;
//    uint32                      sock_id = 0;
//    uint32                      sock_err = 0;
//    DM_START_RESULT_E           result = DM_START_NONE;
//    static   BOOLEAN            is_on_free= FALSE;;
//    DM_TRACE("DM:task start!");
//
//    while(TRUE)/*lint !e716 */
//    {
//        signal_ptr = (DM_SIGNAL_T*)SCI_GetSignal(SCI_IdentifyThread();
//        if (PNULL != signal_ptr)
//        {
//            DM_TRACE("MMIDM==> task cycle, signal code = 0x%x",signal_ptr->SignalCode );
//
//            switch(signal_ptr ->SignalCode)
//            {
//            case DM_TASK_START_MESSAGE:
//                 DM_TRACE("MMIDM==> task DM_TASK_START_MESSAGE ");
//                 result = MMIDM_StartVDM(signal_ptr->content.type,signal_ptr->content.msg_body,signal_ptr->content.msg_size);
//                 DM_TRACE("MMIDM==> task DM_TASK_START_MESSAGE result=%d",result);
//                 switch(result)
//                 {
//                    case DM_START_FAIL:
//                        MMIDM_SetDmState(DM_CANCEL);
//                        MMIDM_CloseWaitingWin();
//                        MMIDM_SendSigToDmTask(DM_TASK_DM_EXIT,MMIDM_GetDmTaskID(),PNULL);
//                      break;
//                    case DM_START_DONE:
//                        MMIDM_CloseWaitingWin();
//                        MMIDM_SendSigToDmTask(DM_TASK_EXIT_MESSAGE,MMIDM_GetDmTaskID(),PNULL);
//                        break;
//                    default:
//                        break;
//                 }
//                 if (PNULL!=signal_ptr->content.msg_body)
//                 {
//                    SCI_FREE(signal_ptr->content.msg_body);
//                 }
//                 break;
//            case DM_TASK_RUN_MESSAGE:
//                 DM_TRACE("MMIDM==> task DM_TASK_RUN_MESSAGE ");
//                 if (DM_NONE != MMIDM_GetDmState() && !is_on_free)
//                 {
//                     MMIDM_Run();
//                 }
//                 break;
//            case DM_TASK_DM_CANCEL:
//                MMIDM_SetDmState(DM_CANCEL);
//                break;
//            case DM_TASK_DM_CLOSE:
//                if(DM_CANCEL == MMIDM_GetDmState())
//                {
//                    MMIDM_ExitDM();
//                }
//                break;
//            case DM_TASK_DL_CANCEL:
//                 MMIDM_SetDlState(DM_CANCEL);
//                 break;
//            case DM_TASK_DL_CLOSE:
//                if(DM_CANCEL == MMIDM_GetDlState())
//                {
//                    MMIDM_ExitDM();
//                    MMIDM_SetDmState(DM_CANCEL);
//                }
//                 break;
//            case DM_TASK_DM_EXIT:
//                MMIDM_ExitDM();
//                break;
//            case DM_TASK_DM_BROKEN:
//                MMIDM_BrokenDMSession();
//                break;
//            case DM_TASK_EXIT_MESSAGE:
//                 SCI_Sleep(100);
//                 DM_TRACE("MMIDM==> task DM_TASK_EXIT_MESSAGE ");
//                 if (DM_NONE != MMIDM_GetDmState()&&!is_on_free)
//                 {
//                    is_on_free = TRUE;
//                    MMIDM_FreeAllMOInstance();
//                    MMIDM_Destroy();
//                    MMIDM_DeactivePdpContext();
//                    MMIDM_DeleteAllResumeFile();
//                    MMIDM_SetDmState(DM_NONE);
//                    is_on_free = FALSE;
//                    if (MMIDM_IsNeedReset())
//                    {
//                        MMIAPIPHONE_PowerReset();
//                    }
//                    MMIDM_SendSignalToMMITask(APP_DM_MESSAGE_IND,MSG_DM_SESSION_EXIT,PNULL);
//                 }
//                 break;
//
//            case SOCKET_READ_EVENT_IND:
//                 sock_id = ((SOCKET_READ_EVENT_IND_SIG_T*)signal_ptr)->socket_id;
//                 DM_TRACE("MMIDM==> task SOCKET_READ_EVENT_IND ");
//                 MMIDM_HandleSocketMessage(SOCKET_STATE_READ,sock_id);
//                break;
//            case SOCKET_WRITE_EVENT_IND:
//                 sock_id = ((SOCKET_WRITE_EVENT_IND_SIG_T*)signal_ptr)->socket_id;
//                 DM_TRACE("MMIDM==> task SOCKET_WRITE_EVENT_IND ");
//                 MMIDM_HandleSocketMessage(SOCKET_STATE_WRITE,sock_id);
//                 break;
//            case SOCKET_CONNECT_EVENT_IND:
//                 sock_id = ((SOCKET_CONNECT_EVENT_IND_SIG_T*)signal_ptr)->socket_id;
//                 DM_TRACE("MMIDM==> task SOCKET_CONNECT_EVENT_IND ");
//                 if (0==((SOCKET_CONNECT_EVENT_IND_SIG_T*)signal_ptr)->error_code)
//                 {
//                     MMIDM_HandleSocketMessage(SOCKET_STATE_CONNECT,sock_id);
//                 }
//                 else
//                 {
//                     MMIDM_HandleSocketMessage(SOCKET_STATE_CLOSE,sock_id);
//                 }
//                 break;
//            case SOCKET_CONNECTION_CLOSE_EVENT_IND:
//                 sock_id = ((SOCKET_CONNECTION_CLOSE_EVENT_IND_SIG_T*)signal_ptr)->socket_id;
//                 MMIDM_HandleSocketMessage(SOCKET_STATE_CLOSE,sock_id);
//                 break;
//            default:
//                 break;
//            }
//
//            SCI_FREE(signal_ptr);
//        }
//
//        if (!s_dm_task_id)
//        {
//            SCI_ThreadExit();
//        }
//    }
//
//}
/*****************************************************************************/
//  Description : create dm task
//  Global resource dependence :
//  Note:
/*****************************************************************************/
//PUBLIC uint32 MMIDM_CreateTask(void)
//{
//    if (0 != s_dm_task_id)
//    {
//        DM_TRACE("MMIDM==> DM_CreateTask, task hasn't been closed!");
//        return 0;
//    }
//
//    s_dm_task_id = SCI_CreateThread(
//                        "T_P_DM",
//                        "Q_P_DM",
//                        DM_TaskHandle,
//                        0,
//                        0,
//                        DM_STACK_SIZE,
//                        DM_STACK_QUEUE_NUM,
//                        DM_TASK_PRIORITY,
//                        SCI_PREEMPT,
//                        SCI_AUTO_START);
//
//    DM_TRACE("MMIDM==> DM_CreateTask, task id = %d",s_dm_task_id);
//    return s_dm_task_id;
//}
/*****************************************************************************/
//  Description : get Dm task id
//  Global resource dependence :
//  Note:
/*****************************************************************************/
PUBLIC uint32 MMIDM_GetDmTaskID(void) {
    return s_dm_task_id;
}

