#ifndef DM_JNI_H_
#define DM_JNI_H_

#include "sci_types.h"
#include "vdm_mmi_pl.h"
#include "vdm_pl_types.h"
#include "smldm.h"
#include "libmem.h"

typedef enum {
    DM_PL_ALERT_NONE,
    DM_PL_ALERT_1100,
    DM_PL_ALERT_1101,
    DM_PL_ALERT_1102,
    DM_PL_ALERT_1103,
    DM_PL_ALERT_1104,
    DM_PL_ALERT_MAX,
} MMIDM_PL_ALERT_TYPE;

typedef enum {
    DM_SESSION_NONE,
    DM_SESSION_USER,
    DM_SESSION_CLIENT,
    DM_SESSION_SERVER,
    DM_SESSION_FUMO_RESUME,
    DM_SESSION_SCOMO_RESUME,
    DM_SESSION_FOTA,
    DM_SESSION_REMOVE,
    DM_SESSION_ADD,
    DM_SESSION_OTHER
} DM_SESSION_TYPE;

typedef enum {
    MMIDM_ALERT_NONE, MMIDM_ALERT_PL_DISPLAY,      //display to user
    MMIDM_ALERT_PL_CONFIRM,      //need user to confirm
    MMIDM_ALERT_NIA_DISPLAY,     //nia display to user
    MMIDM_ALERT_NIA_CONFIRM,     //nia user to confirm
    MMIDM_ALERT_FUMO_CONFIRM,     //fumo need user to confirm
    MMIDM_ALERT_FUMO_UPDATE,      //fumo need user to update
    MMIDM_ALERT_SCOMO_CONFIRM,     //fumo display to user
    MMIDM_ALERT_SINGLE_CHOICE,     //single choice
    MMIDM_ALERT_MULTI_CHOICE,      //multi choice
    MMIDM_ALERT_MAX
} MMIDM_ALERT_TYPE_E;

typedef struct _DM_MSG_Tag {
    DM_SESSION_TYPE type;
    char* msg_body;
    uint32 msg_size;
} DM_MSG_T;

typedef struct {
//    SIGNAL_VARS                         /*!<Signal Header.*/
    DM_MSG_T content;            // message content
} DM_SIGNAL_T;

#define MMIDM_APN_MAX_LEN           (50 + 1)             //define dm apn max length
#define MMIDM_GATEWAY_IP_MAX_LEN    255                                     //define dm gateway ip len
#define MMIDM_PORT_MAX_LEN          30                                      //define dm port max len
#define MMIDM_SRNUM_MAX_LEN         30                                      //define dm server self register number
#define MMIDM_SRPORT_MAX_LEN        30                                      //define dm server self register port
#define DM_RADIX_TYPE                       10
#define DM_MAX_ID_STR                       32
#define DM_STRING_LENGTH                    255
#define DM_STRING_LENGTH_OLD                127
#define DM_SCOMO_APPID_MAX_NUMBER           500
#define DM_MAX_DOWNLOAD_BUFFER_LEN      (20*1024)

#define DM_MAX_NONCE_LEN                    64

//define mmi dm state
typedef enum {
    DM_NONE,        //dm init state
    DM_START,       //dm starte state
    DM_RUN,         //dm run state
    DM_CANCEL,      //dm cancel state
} MMIDM_DM_STATE;

typedef struct {
    char* message_buffer;
    uint32 msg_size;
    DM_SESSION_TYPE type;
} MMIDM_SESSION_T;

//define mmi fota update state
typedef enum {
    DM_FOTA_UPDATA_NONE,
    DM_FOTA_UPDATA_START,
    DM_FOTA_UPDATA_DONE,
    DM_FOTA_UPDATA_FAIL,
} FOTA_UPDATA_STATE_E;

typedef enum {
    DM_SPREADTRUM, DM_LGOPHONE, DM_NEWPOST,
#ifdef FEA_APP_DM
    DM_HISENSE,
#endif /* FEA_APP_DM */
    DM_OTHER,
} MMIDM_TEST_SERVER_E;

typedef enum {
    DM_SELF_REG_NONE, DM_SELF_REG_SUCC, DM_SELF_REG_FAIL,
} MMIDM_SELF_REG_E;

typedef struct {
    char apn_info[MMIDM_APN_MAX_LEN];
    char sr_number[MMIDM_SRNUM_MAX_LEN];
    char sr_port[MMIDM_SRPORT_MAX_LEN];
    char server_url[MMIDM_GATEWAY_IP_MAX_LEN];
    char device_type_info[DM_MAX_ID_STR];
    char oem_info[DM_MAX_ID_STR];
    char srv_addr[MMIDM_GATEWAY_IP_MAX_LEN];
    char server_nonce[DM_MAX_NONCE_LEN+1];
    char client_nonce[DM_MAX_NONCE_LEN+1];
    char* firmwareversion;
    char* hardwareversion;
    BOOLEAN handset_lock;
    BOOLEAN is_download_finish;
    BOOLEAN is_java_app_install;
    MMIDM_DM_STATE dm_state;
    MMIDM_DM_STATE dl_state;
    DM_SESSION_TYPE session_type;
    FOTA_UPDATA_STATE_E fota_state;
    MMIDM_TEST_SERVER_E server_type;
    MMIDM_SELF_REG_E sr_ready;
} MMIDM_INFO_T;

typedef enum {
    DM_START_NONE, DM_START_SUCC, DM_START_FAIL, DM_START_DONE,
} DM_START_RESULT_E;

//typedef void (*DM_MMI_CANCEL_FUNC)(void);
//typedef void (*DM_MMI_CHOISELISTQUERY_FUNC)(IBITFLAGS inUserSelection);
//typedef void (*DM_MMI_CONFIRMATIONQUERY_FUNC)(IBOOL isContinue);
//typedef void (*DM_MMI_INFOMSG_FUNC)(void);
//typedef void (*DM_MMI_INPUTQUERY_FUNC)(UTF8CStr inUserInput);
//typedef void (*DM_MMI_TIMEOUTEVENT_FUNC)(void);
//
//
//typedef struct {
//  DM_MMI_CANCEL_FUNC notifyCanceled;
//  DM_MMI_CHOISELISTQUERY_FUNC notifyChoiceListResult;
//  DM_MMI_CONFIRMATIONQUERY_FUNC notifyConfirmationResult;
//  DM_MMI_INFOMSG_FUNC notifyInfoMsgClosed;
//  DM_MMI_INPUTQUERY_FUNC notifyInputResult;
//  DM_MMI_TIMEOUTEVENT_FUNC notifyTimeout;
//} VDM_MMI_Observer_t;

extern IBOOL VDM_MMI_PL_init(VDM_MMI_Observer_t* inObserver);
extern void VDM_MMI_PL_term(void);
extern void MMIDM_SetNextNonce(BOOLEAN is_client_nonce, char * nonce);
extern MMIDM_DM_STATE MMIDM_GetDmState(void);
extern void MMIDM_SetDmState(MMIDM_DM_STATE state);
extern DM_PARAM_P MMIDM_GetInitParam(void);
extern BOOLEAN MMIDM_IsDmRun(void);
extern char* MMIDM_GetDmProfile(void);
extern char* MMIDM_GetSrvAddURL(void);
extern char *MMIDM_GetMan(void);
extern char *MMIDM_GetModel(void);
extern char* DM_GetDevImeiInfo(void);
extern char* MMIDM_GetServerNonce();
extern char* MMIDM_GetClientNonce();

#define DM_MAX_NONCE_LEN 64

//static final int APN_ALREADY_ACTIVE     = 0;
//static final int APN_REQUEST_STARTED    = 1;
//static final int APN_TYPE_NOT_AVAILABLE = 2;
//static final int APN_REQUEST_FAILED     = 3;

#define     MMIDM_PDP_STATUS_CONNECT  0//connected
#define     MMIDM_PDP_STATUS_START       1 //begin connect
#define     MMIDM_PDP_STATUS_NONE        2  //no connect
#define     MMIDM_PDP_STATUS_ABORT      3 //cancel connect
#endif
