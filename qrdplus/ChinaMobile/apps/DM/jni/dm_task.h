#ifndef HEADER_FILE_DM_TASK
#define HEADER_FILE_DM_TASK

#include "vdm_pl_types.h"
#include "vdm_error.h"
#include "vdm_comm_pl_types.h"
#include "vdm_types.h"
#include "vdm_mmi_pl.h"
#include "smlcore.h"

typedef enum {
    STEP_CLIENT_INIT = 0,
    STEP_SERVER_INIT,
    STEP_CLIENT_REINIT,
    STEP_CLIENT_RESPONSE,
    STEP_SERVER_CONTINUE,
    STEP_CLIENT_ASK_MORE,
    STEP_SYNC_OVER,
    STEP_SYNC_ERROR
} task_protocol_step_type;

/*supported sync type*/
typedef enum {
    SYNC_TYPE_DUMMY = 0, SYNC_SERVER_INITIATED, SYNC_CLIENT_INITIATED,
} sync_type;

typedef enum {
    SYNC_CONTROL_FINISH = 0,
    SYNC_CONTROL_CONTINUE,
    SYNC_CONTROL_WAIT_USER_OPE,
    SYNC_CONTROL_ERROR
} task_protocol_control_type;

typedef enum {
    SYNC_PROC_NORMAL = 0, SYNC_PROC_SENT, SYNC_PROC_RESENT, SYNC_PROC_ERROR
} task_process_step;

typedef struct {
    VDM_Handle_t task_handle;
    task_process_step proc_step;
    unsigned int sessionID;       //session ID
    unsigned int messageID;     //messageID
    unsigned int cmdID;           //command ID
    unsigned int maxmsg_size;
    unsigned int maxobj_size;
    char* cred;             //certification ,a group char,use for authentication
    unsigned char authored; //a flag judge wether pass the authentication,true:pass
    auth_type authtype;        //auth type b64 or md5 or none.
    char* server_nextnonce; //auth next nonce from server which is used by client for the next md5 auth.
    char* client_nextnonce; //client's nextnonce,use for checking the completeness of server's package
    char * client_nextnonce_bak;
    short workspaceid;                               //syncml core workspace id
    SmlEncoding_t encodetype;                  //style of encode ,xml/wbxml
    sync_type synctype;                         //synctype,1:twoway,2:slowsync
    int8 use_proxy;            //wether use substitutive server
    char* proxy_ip;                //substitutive server's IP
    char* proxy_port;            //substitutive server's port
    char* proxy_username;  //substitutive server's username
    char* proxy_password;   //passwords
    char* syncml_dm_server_name;  //SYNCML protocol param,DM server name
    char* syncml_dm_server_port;  //SYNCML protocol param,DMserver's port
    char* syncml_dm_username;     //SYNCML protocol ,DM username
    char* syncml_dm_password;     //SYNCML protocol ,DM user's password
    char * syncml_connect_addr;
    char* syncml_dm_client_imei;       //SYNCML protocol param ,DM client's imei
    VDM_SessionStateNotifyCB session_state_notify_cb;
    VDM_CommObserver_t comm_observer;
    VDM_Comm_HMAC_t comm_hmac;
    void * comm_context_p;
    E_VDM_SessionType_t comm_session_type;
    //VDM_SessionStateNotifyCB session_state_notify_cb;
    char * cach_buffer_ptr;
} task_relay_info_type;

/*==========================================================
 * function     : dm_task_start
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/12/2011
 ==========================================================*/
short dm_task_start(void);

/*==========================================================
 * function     : dm_task_terminate
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/12/2011
 ==========================================================*/
void dm_task_terminate(void);

/*==========================================================
 * function     : dm_task_main
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/12/2011
 ==========================================================*/
void dm_task_main(void);

/*==========================================================
 * function     : dm_task_set_ctrl_type
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/12/2011
 ==========================================================*/
void dm_task_set_ctrl_type(task_protocol_control_type sync_ctrl_type);

/*==========================================================
 * function     : dm_task_init
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/8/2011
 ==========================================================*/
short dm_task_init();

/*==========================================================
 * function     : dm_task_handle
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/18/2011
 ==========================================================*/
void dm_task_handle(void);

#endif
