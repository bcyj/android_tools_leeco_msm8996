#ifdef WIN32
#include "std_header.h"
#endif
#include "libmem.h"
#include "syncmlcomm.h"
#include "callback_handler.h"
#include "dm_task.h"
#include "smlerr.h"

typedef unsigned char byte;

BOOLEAN is_broken_outset = FALSE;/*lint !e526*/ //sync or no
IBOOL is_pause_out = FALSE;

//main step needed in transfer
task_protocol_step_type* dm_protocol_step = NULL;
task_protocol_step_type* dm_protocol_step_priv = NULL;

//main param needed by task,include anchor and so on
task_relay_info_type* dm_task_relay_info = NULL;

//main param needed by task,response server 's status order
status_cmd_queue_type* dm_status_cmd_que = NULL;

//response server's result order
results_cmd_queue_type* dm_results_cmd_que = NULL;

//judge if interrupt,wether need the client's feedback
task_protocol_control_type dm_ctrl_type = SYNC_CONTROL_FINISH;

extern void VDM_Notify_PL_Task_Finished(void);

/*==========================================================
 * function     : dm_init_mmi
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/11/2011
 ==========================================================*/
void dm_init_mmi(void);

/*==========================================================
 * function     : dm_task_terminate
 * decr     : end the progress of sync ,release the resource asked before
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/8/2011
 ==========================================================*/
void dm_task_terminate() {
    // pim_sync_result sync_result={0};
    syncml_task_message("MMIDM  ^_^ enter syncml dm_task_terminate!\n");

    //release the overall variable of communication
    dm_syncml_Comm_Destory();

    //release the variable of the process
    if (NULL != dm_protocol_step) {
        dm_smlLibFree((void *) dm_protocol_step);
        dm_protocol_step = NULL;
    }
    if (NULL != dm_protocol_step_priv) {
        dm_smlLibFree((void *) dm_protocol_step_priv);
        dm_protocol_step_priv = NULL;
    }

    if (NULL != dm_task_relay_info) {

        if (NULL != dm_task_relay_info->proxy_ip) {
            syncml_task_message("MMIDM  ^_^ syncml dm_Data_Sync_terminate!free proxy_ip\n");
            dm_smlLibFree(((void *) dm_task_relay_info->proxy_ip));
            dm_task_relay_info->proxy_ip = NULL;
        }
        if (NULL != dm_task_relay_info->proxy_port) {
            dm_smlLibFree(((void *) dm_task_relay_info->proxy_port));
            dm_task_relay_info->proxy_port = NULL;
        }
        if (NULL != dm_task_relay_info->proxy_username) {
            dm_smlLibFree(((void *) dm_task_relay_info->proxy_username));
            dm_task_relay_info->proxy_username = NULL;
        }
        if (NULL != dm_task_relay_info->proxy_password) {
            dm_smlLibFree(((void *) dm_task_relay_info->proxy_password));
            dm_task_relay_info->proxy_password = NULL;
        }

        if (NULL != dm_task_relay_info->syncml_dm_server_name) {
            dm_smlLibFree(((void *) dm_task_relay_info->syncml_dm_server_name));
            dm_task_relay_info->syncml_dm_server_name = NULL;
        }

        if (NULL != dm_task_relay_info->syncml_dm_server_port) {
            dm_smlLibFree(((void *) dm_task_relay_info->syncml_dm_server_port));
            dm_task_relay_info->syncml_dm_server_port = NULL;
        }
        if (NULL != dm_task_relay_info->syncml_dm_username) {
            dm_smlLibFree(((void *) dm_task_relay_info->syncml_dm_username));
            dm_task_relay_info->syncml_dm_username = NULL;
        }

        if (NULL != dm_task_relay_info->syncml_dm_password) {
            dm_smlLibFree(((void *) dm_task_relay_info->syncml_dm_password));
            dm_task_relay_info->syncml_dm_password = NULL;
        }

        if (NULL != dm_task_relay_info->syncml_connect_addr) {
            dm_smlLibFree(((void *) dm_task_relay_info->syncml_connect_addr));
            dm_task_relay_info->syncml_connect_addr = NULL;
        }

        if (NULL != dm_task_relay_info->syncml_dm_client_imei) {
            dm_smlLibFree(((void *) dm_task_relay_info->syncml_dm_client_imei));
            dm_task_relay_info->syncml_dm_client_imei = NULL;
        }

        if (NULL != dm_task_relay_info->client_nextnonce) {
            syncml_task_message("MMIDM  ^_^ syncml dm_Data_Sync_terminate!free client_nextnonce\n");
            dm_smlLibFree(((void *) dm_task_relay_info->client_nextnonce));
            dm_task_relay_info->client_nextnonce = NULL;
        }

        if (NULL != dm_task_relay_info->client_nextnonce_bak) {
            syncml_task_message("MMIDM  ^_^ syncml dm_Data_Sync_terminate!free client_nextnonce_bak\n");
            dm_smlLibFree((void *) dm_task_relay_info->client_nextnonce_bak);
            dm_task_relay_info->client_nextnonce_bak = NULL;
        }

        if (NULL != dm_task_relay_info->server_nextnonce) {
            syncml_task_message("MMIDM  ^_^ syncml dm_Data_Sync_terminate!free server_nextnonce\n");
            dm_smlLibFree(((void *) dm_task_relay_info->server_nextnonce));
            dm_task_relay_info->server_nextnonce = NULL;
        }

        syncml_task_message("MMIDM  ^_^ syncml dm_Data_Sync_terminate!free dm_task_relay_info\n");
        dm_smlLibFree(((void *) dm_task_relay_info));
        dm_task_relay_info = NULL;
    }

    if (NULL != dm_status_cmd_que) {
        //release status queue
        if (dm_status_cmd_que->totalnumber != 0) {
            syncml_task_message("MMIDM  ^_^ syncml dm_Data_Sync_terminate!free dm_status_cmd_que\n");
            dm_myFreestatusofCQ();
        }
        dm_smlLibFree((void*) dm_status_cmd_que);
        dm_status_cmd_que = NULL;
    }

    if (NULL != dm_results_cmd_que) {
        if (dm_results_cmd_que->totalnumber != 0) {
            syncml_task_message("MMIDM  ^_^ syncml dm_Data_Sync_terminate!free dm_results_cmd_que\n");
            dm_myFreeResultsofCQ();
        }
        dm_smlLibFree((void*) dm_results_cmd_que);
        dm_results_cmd_que = NULL;
    }

    syncml_task_message("MMIDM  ^_^ syncml dm_Data_Sync_terminate!free all instance\n");
    if (NULL != dm_pGlobalAnchor) {
        //release all instance
        dm_syncml_TerminateAllExitInstance();
        //release pGlobalAnchor
        dm_syncml_Terminate();
    }
    //      dm_task_relay_info->session_state_notify_cb=NULL;
    dm_ctrl_type = SYNC_CONTROL_FINISH;
//     dm_task_relay_info->proc_step=SYNC_PROC_NORMAL;
    is_broken_outset = FALSE;
    is_pause_out = FALSE;
    syncml_task_message("MMIDM  ^_^leave syncml dm_Data_Sync_terminate!\n");
}

/*==========================================================
 * function     : dm_got_free_id
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/8/2011
 ==========================================================*/
void dm_got_free_id(void) {
    dm_task_relay_info->workspaceid++;
    if (dm_task_relay_info->workspaceid == 5) {
        dm_task_relay_info->workspaceid = 1;
    }
    //release first workspace,for use now
    dm_syncml_TerminateAllExitInstance();
    syncml_task_message("MMIDM  ^_^ Begin dm_got_free_id workspaceid= %d", dm_task_relay_info->workspaceid);
}

/*==========================================================
 * function     : dm_modify_statuscmd_data
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/21/2011
 ==========================================================*/
void dm_modify_statuscmd_data(void) {
    status_element_type* queueptr;
    syncml_task_message("MMIDM  ^_^ dm_modify_statuscmd_data 1");
    if (NULL == dm_status_cmd_que->queue) {
        return;
    }
    //make sure the alert's state
    queueptr = dm_status_cmd_que->queue;
    syncml_task_message("MMIDM  ^_^ dm_modify_statuscmd_data 2 ");
    do {
        if (queueptr->status) {
            if (0 == xppStrcmp(queueptr->status->cmd->content,"SyncHdr")) {

                if (dm_task_relay_info->authored) {
                    syncml_task_message("MMIDM  ^_^ dm_modify_statuscmd_data 3 ,200 ");
                    queueptr->status->data = dm_smlString2Pcdata(__FILE__,
                            __LINE__, "200");
                } else {
                    syncml_task_message("MMIDM  ^_^ dm_modify_statuscmd_data 4  ,407");
                    queueptr->status->data = dm_smlString2Pcdata(__FILE__,
                            __LINE__, "407");
                }
                break;
            }
        }
        queueptr = queueptr->next;
    } while (queueptr);

}

/*==========================================================
 * function     : dm_step_device_init
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/8/2011
 ==========================================================*/
void dm_step_client_init() {

    char worksapce_name[] = "Hisense DM device init";

    syncml_task_message("MMIDM  ^_^ Begin dm_step_device_init");
    dm_got_free_id();

    dm_syncml_initance(&(dm_task_relay_info->workspaceid), worksapce_name);

    /*build sync header and syncbody first parm*/
    dm_syncml_startmessage(dm_task_relay_info->workspaceid);

    /*Start status cmd for two way sync server syncheader*/
#if 0//del  2011.3.19
    if (0 < dm_status_cmd_que->totalnumber)
    {
        dm_myFreestatusofCQ();
    }

    if (0 < dm_results_cmd_que->totalnumber)
    {
        dm_myFreeResultsofCQ();
    }
#endif
    //use for intiallizition again
    dm_modify_statuscmd_data();
    //Start status cmd for two way sync server syncheader
    dm_syncml_StatusCmd(dm_task_relay_info->workspaceid);

    if (SYNC_SERVER_INITIATED == dm_task_relay_info->synctype) {
        /*Start alert cmd for initialize slow two way sync*/
        dm_syncml_AlertCmd(dm_task_relay_info->workspaceid, "1200");
        syncml_task_message("MMIDM  ^_^ Begin step_client_init_sync_process2  ,1200");
    } else if (SYNC_CLIENT_INITIATED == dm_task_relay_info->synctype) {
        /*Start alert cmd for initialize slow two way sync*/
        dm_syncml_AlertCmd(dm_task_relay_info->workspaceid, "1201");
        syncml_task_message("MMIDM  ^_^ Begin step_client_init_sync_process2,1201");
    } else {
        // to  deal with the error of sync style
    }
    syncml_task_message("MMIDM  ^_^ Begin step_client_init_sync_process3");

    /*Start replace  cmd for require the server's capability*/
    dm_syncml_ReplaceCmd(dm_task_relay_info->workspaceid);

    /* --- End message and the syncbody last parm--- */
    dm_syncml_EndMessage(dm_task_relay_info->workspaceid);

    dm_task_relay_info->messageID++;   //count messageid

    /* --- Print the document which was generated --- */
#ifdef FEATURE_DM_DEBUG
    syncml_task_message("MMIDM  ^_^ Begin Write xml logs.\n");
    dm_myPrintxmlorwbxml("client_init", dm_task_relay_info->workspaceid);
    syncml_task_message("MMIDM  ^_^ xml logs write complete.\n");
#endif

    syncml_task_message("MMIDM  ^_^ client init data ok,begin send!\n");

    //prepare for creating connect
    if (VDM_ERR_OK == dm_syncml_Comm_Open()) {
        *dm_protocol_step = STEP_SERVER_INIT;
        *dm_protocol_step_priv = STEP_CLIENT_ASK_MORE;
        dm_ctrl_type = SYNC_CONTROL_FINISH;
    }
}

/*==========================================================
 * function     : dm_step_server_init
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/8/2011
 ==========================================================*/
void dm_step_server_init() {

    /* --- get the document which was in a file and was considered as the server init document --- */

    char worksapce_name[] = "Hisense DS Server init";

    syncml_task_message("MMIDM  ^_^ Begin dm_step_server_init_sync_process !\n");
    dm_got_free_id();
    syncml_task_message("MMIDM  ^_^  dm_step_server_init_sync_process 1!workspaceid=%d",dm_task_relay_info->workspaceid);
    dm_syncml_initance(&(dm_task_relay_info->workspaceid), worksapce_name);

    syncml_task_message("MMIDM  ^_^ prepare ok,begin receive!\n");
    syncml_task_message("MMIDM  ^_^  dm_step_server_init_sync_process 2 !workspaceid=%d",dm_task_relay_info->workspaceid);
    if (SML_ERR_OK == dm_syncml_Comm_RecData(dm_task_relay_info->workspaceid)) {
        syncml_task_message("MMIDM  ^_^ server init receive ok!\n");

#ifdef FEATURE_DM_DEBUG
        //these are for syncml core test on windows.  make it disabled.2005-11-7.
        syncml_task_message("MMIDM  ^_^ Begin Write xml logs.\n");
        dm_myPrintxmlorwbxml("server_init", dm_task_relay_info->workspaceid);
        syncml_task_message("MMIDM  ^_^ xml logs write complete.\n");
#endif
        syncml_task_message("MMIDM  ^_^ analysis recevie data.\n");
        dm_syncml_ReceiveData(dm_task_relay_info->workspaceid);
        syncml_task_message("MMIDM  ^_^ analysis server init complete.\n");

        if (*dm_protocol_step == STEP_SYNC_ERROR) {
            syncml_task_message("MMIDM  ^_^ server init protocol type error!\n");
            dm_task_main();
        } else {
            if (*dm_protocol_step == STEP_CLIENT_REINIT) {
                syncml_task_message("MMIDM  ^_^ dm_protocol_step == STEP_CLIENT_REINIT\n");
                // *dm_protocol_step = STEP_CLIENT_INIT;
                dm_task_main();
            } else {
                syncml_task_message("MMIDM  ^_^ server init protocol type,ctrl type is %d ",dm_ctrl_type);

                if (*dm_protocol_step == *dm_protocol_step_priv) {
                    if (dm_ctrl_type == SYNC_CONTROL_CONTINUE) {
                        *dm_protocol_step = STEP_CLIENT_RESPONSE;
                        dm_task_main();
                    } else if (dm_ctrl_type == SYNC_CONTROL_FINISH) {
                        *dm_protocol_step = STEP_SYNC_OVER;
                        dm_task_main();
                    } else if (dm_ctrl_type == SYNC_CONTROL_WAIT_USER_OPE) {
                        *dm_protocol_step = STEP_CLIENT_RESPONSE;
                    } else {
                        syncml_task_message("MMIDM  ^_^ server init protocol type,ctrl type is error!\n");
                    }
                } else {
                    syncml_task_message("MMIDM  ^_^ dm_step_server_init,the package is not finish!\n");
                    *dm_protocol_step_priv = *dm_protocol_step;
                    *dm_protocol_step = STEP_CLIENT_ASK_MORE;
                }
            }
        }
    } else {
        //get package failed ,stop
        *dm_protocol_step = STEP_SYNC_ERROR;
        dm_task_main();
        syncml_task_message("MMIDM  ^_^ server init receive data error!\n");
    }
    return;
}

/*==========================================================
 * function     : dm_step_client_response
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/8/2011
 ==========================================================*/
void dm_step_client_response() {

    char worksapce_name[] = "Hisense DM Client modify";
    syncml_task_message("MMIDM  ^_^ Begin dm_step_client_modi_process");
    dm_got_free_id();

    dm_syncml_initance(&(dm_task_relay_info->workspaceid), worksapce_name);

    /*build sync header and syncbody first parm*/
    dm_syncml_startmessage(dm_task_relay_info->workspaceid);

    /*Start status cmd for two way sync server syncheader*/
    dm_syncml_StatusCmd(dm_task_relay_info->workspaceid);

    syncml_task_message("MMIDM  ^_^ workspaceid=%d,authored=%d",dm_task_relay_info->workspaceid,dm_task_relay_info->authored);

    /*Start result cmd for two way sync server syncheader*/
    dm_syncml_ResultsCmd(dm_task_relay_info->workspaceid);

    /* End the sync block */
    // dm_syncml_EndSync(dm_task_relay_info->workspaceid);
    /* --- End message and the syncbody last parm--- */
    dm_syncml_EndMessage(dm_task_relay_info->workspaceid);

    dm_task_relay_info->messageID++;   //count messageid

    /* --- Print the document which was generated --- */
#ifdef FEATURE_DM_DEBUG
    syncml_task_message("MMIDM  ^_^ Begin Write xml logs.\n");
    dm_myPrintxmlorwbxml("client_modi", dm_task_relay_info->workspaceid);
    syncml_task_message("MMIDM  ^_^ Write xml logs complete.\n");
#endif

    syncml_task_message("MMIDM  ^_^ client modi data ok,begin send!");

    if (VDM_ERR_OK == dm_syncml_Comm_Open()) {
        *dm_protocol_step = STEP_SERVER_CONTINUE;
        *dm_protocol_step_priv = STEP_CLIENT_ASK_MORE;
        dm_ctrl_type = SYNC_CONTROL_FINISH;
    }
}

/*==========================================================
 * function     : dm_step_server_continue
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/8/2011
 ==========================================================*/
void dm_step_server_continue() {
    /* --- get the document which was in a file and was considered as the server init document --- */

    char worksapce_name[] = "Hisense DS Server status and modi";

    syncml_task_message("MMIDM  ^_^ Begin dm_step_server_continue");
    dm_got_free_id();

    dm_syncml_initance(&(dm_task_relay_info->workspaceid), worksapce_name);

    syncml_task_message("MMIDM  ^_^ prepare ok,begin receive!\n");
    if (SML_ERR_OK == dm_syncml_Comm_RecData(dm_task_relay_info->workspaceid)) {
        syncml_task_message("MMIDM  ^_^ server init receive ok!\n");

#ifdef FEATURE_DM_DEBUG
        //these are for syncml core test on windows.  make it disabled.2005-11-7.
        syncml_task_message("MMIDM  ^_^ Begin Write xml logs.\n");
        dm_myPrintxmlorwbxml("server_modi", dm_task_relay_info->workspaceid);
        syncml_task_message("MMIDM  ^_^ xml logs write complete.\n");
#endif

        syncml_task_message("MMIDM  ^_^ analysis recevie data.\n");
        dm_syncml_ReceiveData(dm_task_relay_info->workspaceid);
        syncml_task_message("MMIDM  ^_^ analysis server conf complete.\n");

        if (*dm_protocol_step == STEP_SYNC_ERROR) {
            syncml_task_message("MMIDM  ^_^ dm_step_server_continue, protocol type error!\n");
            dm_task_main();
        } else {
            syncml_task_message("MMIDM  ^_^ dm_step_server_continue ,ctrl type is %d ",dm_ctrl_type);
            if (*dm_protocol_step == *dm_protocol_step_priv) {
                if (dm_ctrl_type == SYNC_CONTROL_CONTINUE) {
                    *dm_protocol_step = STEP_CLIENT_RESPONSE;
                    dm_task_main();
                } else if (dm_ctrl_type == SYNC_CONTROL_FINISH) {
                    *dm_protocol_step = STEP_SYNC_OVER;
                    dm_task_main();
                } else if (dm_ctrl_type == SYNC_CONTROL_WAIT_USER_OPE) {
                    *dm_protocol_step = STEP_CLIENT_RESPONSE;
                } else {
                    syncml_task_message("MMIDM  ^_^ dm_step_server_continue,ctrl type is error!\n");
                }
            } else {
                syncml_task_message("MMIDM  ^_^ dm_step_server_continue,the package is not finish!\n");
                *dm_protocol_step_priv = *dm_protocol_step;
                *dm_protocol_step = STEP_CLIENT_ASK_MORE;
            }
        }
    } else {
        *dm_protocol_step = STEP_SYNC_ERROR;
        dm_task_main();
        syncml_task_message("MMIDM  ^_^ dm_step_server_continue receive data error!\n");
    }
    syncml_task_message("MMIDM @@dm_step_server_continue  end .");

    return;

}

/*==========================================================
 * function     : dm_step_client_ask_more
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/8/2011
 ==========================================================*/
void dm_step_client_ask_more() {
//    char docname[]="clientmodi";
    char worksapce_name[] = "Hisense DS multi-package";

    syncml_task_message("MMIDM  ^_^ Begin dm_step_client_ask_more ");
    dm_got_free_id();

    dm_syncml_initance(&(dm_task_relay_info->workspaceid), worksapce_name);

    /*build sync header and syncbody first parm*/
    dm_syncml_startmessage(dm_task_relay_info->workspaceid);

    /*Start status cmd for server syncheader*/
    dm_syncml_StatusCmd(dm_task_relay_info->workspaceid);

    /*Start result cmd for two way sync server syncheader*/
    dm_syncml_ResultsCmd(dm_task_relay_info->workspaceid);

    /*Alert cmd for more!!with alert 222*/
    dm_syncml_AlertCmd(dm_task_relay_info->workspaceid, "222");

    /* --- End message and the syncbody last parm--- */
    dm_syncml_EndMessageWithoutFinal(dm_task_relay_info->workspaceid);

    dm_task_relay_info->messageID++;   //count messageid

    /* --- Print the document which was generated --- */
#ifdef FEATURE_DM_DEBUG
    syncml_task_message("MMIDM  ^_^ Begin Write xml logs.\n");
    dm_myPrintxmlorwbxml("server_more", dm_task_relay_info->workspaceid);
    syncml_task_message("MMIDM  ^_^ Write xml logs complete.\n");
#endif

    syncml_task_message("MMIDM  ^_^ server more data ok,begin send!");

    if (VDM_ERR_OK == dm_syncml_Comm_Open()) {
        *dm_protocol_step = *dm_protocol_step_priv;
        *dm_protocol_step_priv = STEP_CLIENT_ASK_MORE;
        dm_ctrl_type = SYNC_CONTROL_FINISH;
    }
}

/*==========================================================
 * function     : DM_Comm_notifyCommOpenCB
 * decr     :callback func,connection has finished,can send package
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/9/2011
 ==========================================================*/
VDM_Error DM_Comm_notifyCommOpenCB(VDM_Handle_t inContext, IS32 inConnId) {
    syncml_task_message("MMIDM  ^_^ DM_Comm_notifyCommOpenCB,begin!");
    if (FALSE == is_broken_outset) {
        if (SYNC_PROC_RESENT == dm_task_relay_info->proc_step) {
            syncml_task_message(("MMIDM  ^_^ DM_Comm_notifyCommOpenCB,begin Resent!"));
            dm_syncml_Comm_SendData(dm_task_relay_info->workspaceid, TRUE);
            dm_task_relay_info->proc_step = SYNC_PROC_NORMAL;

        } else {
            dm_task_relay_info->proc_step = SYNC_PROC_SENT;
            //VDM_PL_Async_signal(dm_task_relay_info->task_handle);
            dm_syncml_Comm_SendData(dm_task_relay_info->workspaceid, FALSE);
        }
    }
    syncml_task_message("MMIDM  ^_^ DM_Comm_notifyCommOpenCB,end!");
    return SML_ERR_OK;
}

/*==========================================================
 * function     : DM_Comm_notifyTransportCB
 * decr     :callback func,package has been recived ,can be copy
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/9/2011
 ==========================================================*/
VDM_Error DM_Comm_notifyTransportCB(VDM_Handle_t inContext, IS32 inConnId) {

    //prepare for getting data
    syncml_task_message("MMIDM  ^_^ DM_Comm_notifyTransportCB,begin!");
    //first,release the memory allocated last time
    if (dm_task_relay_info->cach_buffer_ptr) {
        dm_smlLibFree(dm_task_relay_info->cach_buffer_ptr);
    }
    if (FALSE == is_broken_outset) {
        dm_task_relay_info->proc_step = SYNC_PROC_NORMAL;
        //  VDM_PL_Async_signal(dm_task_relay_info->task_handle);
    }
    dm_task_main();
    syncml_task_message("MMIDM  ^_^ DM_Comm_notifyTransportCB,end!");
    return SML_ERR_OK;

}

/*==========================================================
 * function     : DM_Comm_notifyCommBrokenCB
 * decr     :callback func,need interrupt deal
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/9/2011
 ==========================================================*/
VDM_Error DM_Comm_notifyCommBrokenCB(VDM_Handle_t inContext, IS32 inConnId,
        IBOOL inIsFatalError) {
    is_broken_outset = inIsFatalError;
    // return SML_ERR_OK;
    syncml_task_message("!!!!!!!dm_task broken    dm_protocol_step=%d",*dm_protocol_step);
    *dm_protocol_step = STEP_SYNC_ERROR;
    dm_task_main();
    return SML_ERR_OK;
}
VDM_Error DM_Comm_notifyCommPauseCB(VDM_Handle_t inContext, IS32 inConnId,
        IBOOL inIsFatalError) {
    is_pause_out = inIsFatalError;
    syncml_task_message("MMIDM  ^_^ DM_Comm_notifyCommPauseCB is_pause_out=%d",is_pause_out);
    if (FALSE == is_pause_out) {
        //dm_task_handle();
        syncml_task_message("MMIDM  ^_^ DM_Comm_notifyCommPauseCB  Resume procss!");
        dm_task_relay_info->proc_step = SYNC_PROC_RESENT;
        dm_task_handle();
    }
    return SML_ERR_OK;
}

/*!
 *******************************************************************************
 * Callback function for notifying vDM Engine that an information message
 * from the DM engine has been closed by the user.
 *******************************************************************************
 */
void dm_mmi_infoMsgcb(void) {

}

/*!
 *******************************************************************************
 * Callback function for notifying vDM Engine that the user has confirmed
 * or denied an action prompted by the DM server.
 *
 * \param    inContinue          TRUE if user has confirmed the action,
 *                               FALSE if user has denied it.
 *******************************************************************************
 */
void dm_mmi_confirmationQuerycb(IBOOL isContinue) {
    status_element_type* queueptr;
    syncml_task_message("MMIDM  ^_^ dm_mmi_confirmationQuerycb,iscontinue=%d",isContinue);

    if (NULL == dm_status_cmd_que->queue) {
        return;
    }
    queueptr = dm_status_cmd_que->queue;
    syncml_task_message("MMIDM  ^_^ dm_mmi_confirmationQuerycb  1,queue=%xd",&(dm_status_cmd_que->queue));
    do {
        if (queueptr->status) {
            syncml_task_message("MMIDM  ^_^ dm_mmi_confirmationQuerycb 2,queue=%xd",&(dm_status_cmd_que->queue));
            syncml_task_message("MMIDM  ^_^ dm_mmi_confirmationQuerycb 2,content=%s",queueptr->status->cmd->content);

            if (0 == xppStrcmp(queueptr->status->cmd->content,"Alert")) {
                if (isContinue) {
                    syncml_task_message("MMIDM  ^_^ dm_mmi_confirmationQuerycb 3,queue=%xd",&(dm_status_cmd_que->queue));
                    queueptr->status->data = dm_smlString2Pcdata(__FILE__,
                            __LINE__, "200");
                } else {
                    syncml_task_message("MMIDM  ^_^ dm_mmi_confirmationQuerycb 4,queue=%xd",&(dm_status_cmd_que->queue));
                    queueptr->status->data = dm_smlString2Pcdata(__FILE__,
                            __LINE__, "304");
                }
                break;
            }
        }
        queueptr = queueptr->next;
    } while (queueptr);
    dm_task_relay_info->proc_step = SYNC_PROC_NORMAL;
    dm_task_main();
}

/*******************************************************************************
 * Callback function for notifying vDM Engine the user has entered input
 * requested by the DM server.
 *
 * \param    inUserInput         Text entered by the user.
 *******************************************************************************
 */
void dm_mmi_inputQuerycb(UTF8CStr inUserInput) {
    syncml_task_message("MMIDM  ^_^ dm_mmi_inputQuerycb,UserInput=%s",inUserInput);
}

/*!
 *******************************************************************************
 * Callback function for notifying vDM Engine that the user has made a selection
 * on a choice list given by the DM server.
 *
 * \param    inUserSelection     Each bit represents an item in the list. If set,
 *                               then user has selected the item.
 *                               In single-selection list, only one bit must be set.
 *******************************************************************************
 */
void dm_mmi_choiceListQuerycb(IBITFLAGS inUserSelection) {
    syncml_task_message("MMIDM  ^_^ dm_mmi_choiceListQuerycb,UserSelection=%d",inUserSelection);
}

/*!
 *******************************************************************************
 * Callback function for notifying vDM Engine that the user has canceled the
 * operation.
 *
 * May be called from any MMI screen instead of the screen's result
 * callback.
 *******************************************************************************
 */
void dm_mmi_cancelEventcb(void) {
    syncml_task_message("MMIDM  ^_^ dm_mmi_cancelEventcb");
}

/*!
 *******************************************************************************
 * Callback function for notifying vDM Engine that a timeout event has occurred
 * (maxDisplayTime seconds have passed) without any user response.
 *
 * May be called from any MMI screen instead of the screen's result
 * callback.
 *******************************************************************************
 */
void dm_mmi_timeoutEventcb(void) {
    syncml_task_message("MMIDM  ^_^ dm_mmi_timeoutEventcb");
}
/*==========================================================
 * function     : dm_init_mmi
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/11/2011
 ==========================================================*/
void dm_init_mmi(void) {
    VDM_MMI_Observer_t mmi_observer;
    syncml_task_message("MMIDM  ^_^ dm_init_mmi");
    dm_smlLibMemset(&mmi_observer, 0, sizeof(VDM_MMI_Observer_t));
    mmi_observer.notifyCanceled = dm_mmi_cancelEventcb;
    mmi_observer.notifyChoiceListResult = dm_mmi_choiceListQuerycb;
    mmi_observer.notifyConfirmationResult = dm_mmi_confirmationQuerycb;
    mmi_observer.notifyInfoMsgClosed = dm_mmi_infoMsgcb;
    mmi_observer.notifyInputResult = dm_mmi_inputQuerycb;
    mmi_observer.notifyTimeout = dm_mmi_timeoutEventcb;
    VDM_MMI_PL_init(&mmi_observer);
}

/*==========================================================
 * function     : dm_task_set_ctrl_type
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/12/2011
 ==========================================================*/
void dm_task_set_ctrl_type(task_protocol_control_type sync_ctrl_type) {
    syncml_task_message("MMIDM  ^_^ dm_task_set_ctrl_type,control_type=%d",sync_ctrl_type);

    dm_ctrl_type = sync_ctrl_type;
}

/*==========================================================
 * function     : dm_task_main
 * decr     : task module's main function,control the whole process of session
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/8/2011
 ==========================================================*/
void dm_task_main() {
    syncml_task_message("MMIDM  ^_^ dm_task_main step=%d \n",*dm_protocol_step);

    /*watch the step,then do as the rule*/
    switch (*dm_protocol_step) {

    case STEP_CLIENT_INIT: {
        syncml_task_message("MMIDM  ^_^ Begin device init!\n");
        dm_step_client_init();
    }
        break;
    case STEP_CLIENT_REINIT: {
        syncml_task_message(("MMIDM  ^_^ Begin device reinit!\n"));
        dm_step_client_init();
        //dm_ctrl_type=SYNC_CONTROL_CONTINUE;
        //*dm_protocol_step_priv=*dm_protocol_step;
    }
        break;

    case STEP_SERVER_INIT: {
        syncml_task_message("MMIDM  ^_^ Begin Server init!\n");
        dm_step_server_init();
    }
        break;

    case STEP_CLIENT_RESPONSE: {
        syncml_task_message("MMIDM  ^_^ Begin device response!\n");
        dm_step_client_response();
    }
        break;

    case STEP_SERVER_CONTINUE: {
        syncml_task_message("MMIDM  ^_^ Begin Server continue!\n");
        dm_step_server_continue();
    }
        break;

    case STEP_SYNC_OVER: {

        syncml_task_message("MMIDM  ^_^ All sync operation is over,then release the resources!\n");

        //¶Ônextnonce½øÐÐ´æ´¢
        MMIDM_SetNextNonce(TRUE, dm_task_relay_info->client_nextnonce_bak);
        MMIDM_SetNextNonce(FALSE, dm_task_relay_info->server_nextnonce);

        syncml_task_message("MMIDM  ^_^ dm_task_relay_info->session_state_notify_cb = %d!\n", dm_task_relay_info->session_state_notify_cb);
        if (dm_task_relay_info->session_state_notify_cb) {
            dm_task_relay_info->session_state_notify_cb(VDM_SessionType_DM,
                    VDM_SessionState_Complete, VDM_ERR_OK, NULL,
                    &(dm_task_relay_info->comm_context_p));
        }
        dm_task_terminate();
        VDM_Notify_PL_Task_Finished();
    }
        break;

    case STEP_SYNC_ERROR: {

        syncml_task_message("MMIDM  ^_^ All sync operation is over,then release the resources!\n");

        // storage nextnonce
        MMIDM_SetNextNonce(TRUE, dm_task_relay_info->client_nextnonce_bak);
        MMIDM_SetNextNonce(FALSE, dm_task_relay_info->server_nextnonce);
        if (dm_task_relay_info->session_state_notify_cb) {
            dm_task_relay_info->session_state_notify_cb(VDM_SessionType_DM,
                    VDM_SessionState_Aborted, VDM_ERR_INVALID_PROTO_OR_VERSION,
                    NULL, &(dm_task_relay_info->comm_context_p));
        }
        dm_task_terminate();
        VDM_Notify_PL_Task_Finished();
    }
        break;

        //if there are transmitting data,send alert 222 apply for continue sending
    case STEP_CLIENT_ASK_MORE: {
        syncml_task_message("MMIDM  ^_^ ask for more data,send alert 222 for more!\n");
        dm_step_client_ask_more();
    }
        break;

        //there are no other unexception
    default: {
        syncml_task_message("MMIDM  *_* STEP_SYNC_ERROR!\n");
    }
        break;

    }

    return;
}

/*==========================================================
 * function     : dm_task_init
 * decr     :according achieved param ,intlizition the param needed by session
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/8/2011
 ==========================================================*/
short dm_task_init() {
    char* client_nextnonce = NULL;
    char* client_nextnonce_bak = NULL;
    char * server_nextnonce = NULL;
    char* proxyaddr = NULL;
    char* proxyport = NULL;
    char* proxyuname = NULL;
    char* proxypword = NULL;

    char* serveraddr = NULL;
    char* serverport = NULL;
    char* user = NULL;
    char* password = NULL;

    char * connect_addr = NULL;

    char* clientimei = NULL;

    DM_PARAM_P pMe = MMIDM_GetInitParam();

    syncml_task_message("MMIDM  ^_^dm_Data_Sync_init");

    dm_protocol_step = (task_protocol_step_type*) dm_smlLibMalloc(__FILE__,
            __LINE__, (long) sizeof(task_protocol_step_type));
    if (NULL == dm_protocol_step) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(dm_protocol_step, 0,
            (long) sizeof(task_protocol_step_type));
    *dm_protocol_step = STEP_CLIENT_INIT;

    dm_protocol_step_priv = (task_protocol_step_type*) dm_smlLibMalloc(__FILE__,
            __LINE__, (long) sizeof(task_protocol_step_type));
    if (NULL == dm_protocol_step_priv) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(dm_protocol_step_priv, 0,
            (long) sizeof(task_protocol_step_type));
    *dm_protocol_step_priv = *dm_protocol_step;

    dm_task_relay_info = (task_relay_info_type*) dm_smlLibMalloc(__FILE__,
            __LINE__, (long) sizeof(task_relay_info_type));
    if (NULL == dm_task_relay_info) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(dm_task_relay_info, 0, (long) sizeof(task_relay_info_type));

    dm_status_cmd_que = (status_cmd_queue_type*) dm_smlLibMalloc(__FILE__,
            __LINE__, (long) sizeof(status_cmd_queue_type));
    if (NULL == dm_status_cmd_que) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(dm_status_cmd_que, 0, (long) sizeof(status_cmd_queue_type));
    dm_status_cmd_que->queue = NULL;
    dm_status_cmd_que->totalnumber = 0;

    dm_results_cmd_que = (status_cmd_queue_type*) dm_smlLibMalloc(__FILE__,
            __LINE__, (long) sizeof(status_cmd_queue_type));
    if (NULL == dm_results_cmd_que) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(dm_results_cmd_que, 0,
            (long) sizeof(results_cmd_queue_type));
    dm_results_cmd_que->queue = NULL;
    dm_results_cmd_que->totalnumber = 0;

    dm_task_relay_info->authtype == pMe->m_authtype;
    dm_task_relay_info->encodetype = pMe->m_sml_encoding_type;
    syncml_task_message("MMIDM  ^_^dm_Data_Sync_init,authtype=%d,encodetype=%d", dm_task_relay_info->authtype,dm_task_relay_info->encodetype);

    dm_task_relay_info->sessionID = MMIDM_GetNotifySessionId();

    dm_task_relay_info->messageID = 1;
    dm_task_relay_info->cmdID = 1;
    dm_task_relay_info->maxmsg_size = pMe->m_max_msg_size;
    dm_task_relay_info->maxobj_size = pMe->m_max_obj_size;
    dm_task_relay_info->comm_context_p = PNULL;
    dm_task_relay_info->comm_session_type = VDM_SessionType_DM;
    dm_task_relay_info->cred = NULL;
    dm_task_relay_info->authored = 0;
    dm_task_relay_info->workspaceid = (short) 0;

    //allocate proxy.
    dm_task_relay_info->use_proxy = pMe->m_proxy_setting.use_proxy;
    proxyaddr = (char*) dm_smlLibMalloc(__FILE__, __LINE__,
            MMIDM_MAX_IP_LEN + 1);
    if (NULL == proxyaddr) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(proxyaddr, 0, MMIDM_MAX_IP_LEN + 1);
    dm_task_relay_info->proxy_ip = proxyaddr;
    dm_smlLibStrcpy(proxyaddr, (char*) (pMe->m_proxy_setting.addr));

    proxyport = (char*) dm_smlLibMalloc(__FILE__, __LINE__,
            MMIDM_MAX_PORT_LEN + 1);
    if (NULL == proxyport) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(proxyport, 0, MMIDM_MAX_PORT_LEN + 1);
    dm_task_relay_info->proxy_port = proxyport;
    dm_smlLibStrcpy(proxyport, (char*) (pMe->m_proxy_setting.port));

    proxyuname = (char*) dm_smlLibMalloc(__FILE__, __LINE__,
            MMIDM_MAX_USER_LEN + 1);
    if (NULL == proxyuname) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(proxyuname, 0, MMIDM_MAX_USER_LEN + 1);
    dm_task_relay_info->proxy_username = proxyuname;
    dm_smlLibStrcpy(proxyuname, (char*) (pMe->m_proxy_setting.username));

    proxypword = (char*) dm_smlLibMalloc(__FILE__, __LINE__,
            MMIDM_MAX_PASSWORD_LEN + 1);
    if (NULL == proxypword) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(proxypword, 0, MMIDM_MAX_PASSWORD_LEN + 1);
    dm_task_relay_info->proxy_password = proxypword;
    dm_smlLibStrcpy(proxypword, (char*) (pMe->m_proxy_setting.password));

    //allocate server
    serveraddr = (char*) dm_smlLibMalloc(__FILE__, __LINE__,
            MMIDM_MAX_URL_LEN + 1);
    if (NULL == serveraddr) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(serveraddr, 0, MMIDM_MAX_URL_LEN + 1);
    dm_task_relay_info->syncml_dm_server_name = serveraddr;
    dm_smlLibStrcpy(serveraddr, (char*) (pMe->m_server_setting.addr));

    serverport = (char*) dm_smlLibMalloc(__FILE__, __LINE__,
            MMIDM_MAX_PORT_LEN + 1);
    if (NULL == serverport) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(serverport, 0, MMIDM_MAX_PORT_LEN + 1);
    dm_task_relay_info->syncml_dm_server_port = serverport;
    dm_smlLibStrcpy(serverport, (char*) (pMe->m_server_setting.port));

    user = (char*) dm_smlLibMalloc(__FILE__, __LINE__, MMIDM_MAX_USER_LEN + 1);
    if (NULL == user) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(user, 0, MMIDM_MAX_USER_LEN + 1);
    dm_task_relay_info->syncml_dm_username = user;
    dm_smlLibStrcpy(user, (char*) (pMe->m_server_setting.username));
    syncml_task_message("MMIDM  ^_^ dm_task_init ,username=%s ",user);

    dm_task_relay_info->synctype = SYNC_SERVER_INITIATED;

    password = (char*) dm_smlLibMalloc(__FILE__, __LINE__,
            MMIDM_MAX_PASSWORD_LEN + 1);
    if (NULL == password) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(password, 0, MMIDM_MAX_PASSWORD_LEN + 1);
    dm_task_relay_info->syncml_dm_password = password;
    dm_smlLibStrcpy(password, (char*) (pMe->m_server_setting.password));
    syncml_task_message("MMIDM  ^_^ dm_task_init ,password=%s ",password);

    dm_task_relay_info->authtype = pMe->m_authtype;
    connect_addr = (char*) dm_smlLibMalloc(__FILE__, __LINE__,
            MMIDM_FULLPATH_LEN + 1);
    if (NULL == connect_addr) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(connect_addr, 0, MMIDM_FULLPATH_LEN + 1);
    dm_task_relay_info->syncml_connect_addr = connect_addr;
    //if(1==dm_task_relay_info->use_proxy)
    if (0) {
        xppStrcpy(dm_task_relay_info->syncml_connect_addr, "http://");
        xppStrlcat(dm_task_relay_info->syncml_connect_addr,
                dm_task_relay_info->proxy_ip, sizeof(dm_task_relay_info->syncml_connect_addr));
        xppStrlcat(dm_task_relay_info->syncml_connect_addr, ":",
                sizeof(dm_task_relay_info->syncml_connect_addr));
        xppStrlcat(dm_task_relay_info->syncml_connect_addr,
                dm_task_relay_info->proxy_port, sizeof(dm_task_relay_info->syncml_connect_addr));
        xppStrlcat(dm_task_relay_info->syncml_connect_addr, "/",
                sizeof(dm_task_relay_info->syncml_connect_addr));
    } else {
        //xppStrcpy( dm_task_relay_info->syncml_connect_addr, "http://");
        xppStrlcat(dm_task_relay_info->syncml_connect_addr,
                dm_task_relay_info->syncml_dm_server_name,
                sizeof(dm_task_relay_info->syncml_connect_addr));
        //xppStrcat(dm_task_relay_info->syncml_connect_addr, ":");
        //xppStrcat(dm_task_relay_info->syncml_connect_addr,dm_task_relay_info->syncml_dm_server_port);
        //xppStrcat(dm_task_relay_info->syncml_connect_addr,"/");
        syncml_task_message("MMIDM  ^_^ dm_task_init ,connect_addr=%s ",dm_task_relay_info->syncml_connect_addr);

    }

    //allocate client
    clientimei = (char*) dm_smlLibMalloc(__FILE__, __LINE__,
            MMIDM_IMEI_STR_LEN + 1);
    if (NULL == clientimei) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(clientimei, 0, MMIDM_IMEI_STR_LEN + 1);
    dm_task_relay_info->syncml_dm_client_imei = clientimei;
    dm_smlLibStrcpy(clientimei, pMe->m_imei_info);

    client_nextnonce = (char*) dm_smlLibMalloc(__FILE__, __LINE__,
            MMIDM_NEXT_NONCE);
    if (NULL == client_nextnonce) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(client_nextnonce, 0, MMIDM_NEXT_NONCE);
    dm_task_relay_info->client_nextnonce = client_nextnonce;
    dm_smlLibStrcpy(client_nextnonce, pMe->m_client_nextnonce);

    client_nextnonce_bak = (char*) dm_smlLibMalloc(__FILE__, __LINE__,
            MMIDM_NEXT_NONCE);
    if (NULL == client_nextnonce_bak) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(client_nextnonce_bak, 0, MMIDM_NEXT_NONCE);
    dm_task_relay_info->client_nextnonce_bak = client_nextnonce_bak;
    dm_smlLibStrcpy(client_nextnonce_bak, pMe->m_client_nextnonce);

    server_nextnonce = (char*) dm_smlLibMalloc(__FILE__, __LINE__,
            MMIDM_NEXT_NONCE);
    if (NULL == server_nextnonce) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(server_nextnonce, 0, MMIDM_NEXT_NONCE);
    dm_task_relay_info->server_nextnonce = server_nextnonce;
    dm_smlLibStrcpy(server_nextnonce, pMe->m_server_nextnonce);

    //initalize the syncml praser
    if (SML_ERR_OK != dm_syncml_init()) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    dm_task_relay_info->task_handle = pMe->task_handle;

    dm_task_relay_info->session_state_notify_cb = pMe->session_state_notify_cb;

    syncml_task_message("MMIDM  ^_^ dm_task_init ,session_state_notify_cb=%d ",dm_task_relay_info->session_state_notify_cb);

    dm_task_relay_info->comm_observer.context = PNULL;
    dm_task_relay_info->comm_observer.notifyCommOpen = DM_Comm_notifyCommOpenCB;
    dm_task_relay_info->comm_observer.notifyTransport =
            DM_Comm_notifyTransportCB;
    dm_task_relay_info->comm_observer.notifyCommBroken =
            DM_Comm_notifyCommBrokenCB;
    dm_task_relay_info->comm_observer.notifyCommPause =
            DM_Comm_notifyCommPauseCB;

    dm_init_mmi();

    syncml_task_message("MMIDM  ^_^ dm_syncml_init ok!\n");

    is_broken_outset = FALSE;
    is_pause_out = FALSE;
    dm_ctrl_type = SYNC_CONTROL_FINISH;
    dm_task_relay_info->proc_step = SYNC_PROC_NORMAL;

    if (SML_ERR_OK != dm_syncml_Comm_Init()) {
        return SML_ERR_A_COMM_ERROR;
    }

    syncml_task_message("MMIDM  ^_^ syncml communication init ok!");

    return SML_ERR_OK;

}

/*==========================================================
 * function     : dm_task_handle
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/9/2011
 ==========================================================*/
void dm_task_handle(void) {
    syncml_task_message("MMIDM  ^_^  dm_task_handle ,proc_step=%d",dm_task_relay_info->proc_step);
    if (is_pause_out) {
        syncml_task_message("MMIDM  ^_^  dm_task_handle ,Paused outset! ");

        return;
    }
    if (SYNC_PROC_NORMAL == dm_task_relay_info->proc_step) {
        dm_task_main();
    } else if (SYNC_PROC_SENT == dm_task_relay_info->proc_step) {
        dm_syncml_Comm_SendData(dm_task_relay_info->workspaceid, FALSE);
        dm_task_relay_info->proc_step = SYNC_PROC_NORMAL;
    } else if (SYNC_PROC_RESENT == dm_task_relay_info->proc_step) {
        dm_syncml_Comm_Open();
    } else {
        syncml_task_message("MMIDM  ^_^  dm_task_handle,proc_step is error! ");
    }
}

