#ifndef _SMLDM_H_
#define _SMLDM_H_

/**--------------------------------------------------------------------------*
 **                         Include Files                                    *
 **--------------------------------------------------------------------------*/
#include "sci_types.h"
#include "vdm_pl_types.h"
#include "vdm_error.h"
#include "vdm_types.h"
/**--------------------------------------------------------------------------*
 **                         Compiler Flag                                    *
 **--------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/**--------------------------------------------------------------------------*
 **                         MACRO DEFINITION                                 *
 **--------------------------------------------------------------------------*/

/****************************************************************
 FEATURE define
 *****************************************************************/
#define FEATURE_DM_DEBUG

//#define DM_MEMORY_LEAK_DEBUG

//#define DM_LOG_FILE
//#define LOGE
#define MMIDM_MAX_STR_LEN 255
#define MMIDM_MAX_APN_LEN   50
#define MMIDM_MAX_USER_LEN  50
#define MMIDM_MAX_PASSWORD_LEN   20
#define MMIDM_MAX_IP_LEN    20
#define MMIDM_MAX_PORT_LEN  5
#define MMIDM_MAX_SERVER_ADDR_LEN   100
#define MMIDM_MAX_DATABASE_LEN  30
#define MMIDM_MAX_URL_LEN   200
#define MMIDM_ANCHOR_LEN 40
//#define MMIDM_NEXT_NONCE 50
#define MMIDM_NEXT_NONCE 64
#define MMIDM_IMEI_STR_LEN 20
#define MMIDM_FULLPATH_LEN MMIDM_MAX_URL_LEN+MMIDM_MAX_PORT_LEN+10

#define MMIDM_UA_CUSTOM_MAX_INPUT_LEN 512

#if 1//def FEATURE_DM_DEBUG
//#define syncml_message(x)  DBGPRINTF x

//            #define syncml_message(x)

#define FEATURE_EFS_LOG

#define FEATURE_CALLBACK_LOG

#define FEATURE_HTTP_LOG

#define FEATURE_COMM_LOG

#define FEATURE_TASK_LOG

#define FEATURE_SYNCML_CORE_LOG

// #define  FEATURE_SYNCML_CODEC_LOG

#if 0
#define  FEATURE_VCARD_PARSE_LOG

#ifdef FEATURE_EFS_LOG
#define syncml_efs_message(x) syncml_message(x)
#else
#define syncml_efs_message(x)
#endif

#ifdef FEATURE_CALLBACK_LOG
#define syncml_cb_message(x) syncml_message(x)
#else
#define syncml_cb_message(x)
#endif

#ifdef FEATURE_HTTP_LOG
#define syncml_http_message(x) syncml_message(x)
#else
#define syncml_http_message(x)
#endif

#ifdef FEATURE_COMM_LOG
#define syncml_comm_message(x) syncml_message(x)
#else
#define syncml_comm_message(x)
#endif

#ifdef FEATURE_TASK_LOG
#define syncml_task_message(x) syncml_message(x)
#else
#define syncml_task_message(x)
#endif

#ifdef FEATURE_SYNCML_CORE_LOG
#define syncml_core_message(x) syncml_message(x)
#else
#define syncml_core_message(x)
#endif

#ifdef FEATURE_SYNCML_CODEC_LOG
#define syncml_codec_message(x) syncml_message(x)
#else
#define syncml_codec_message(x)
#endif

#ifdef FEATURE_VCARD_PARSE_LOG
#define syncml_vcard_parse_message(x) syncml_message(x)
#else
#define syncml_vcard_parse_message(x)
#endif
#endif
#if 1
#define syncml_efs_message LOGE
#define syncml_cb_message LOGE
#define syncml_http_message LOGE
#define syncml_comm_message LOGE
#define syncml_task_message LOGE
#define syncml_core_message LOGE
#if 0
#define syncml_codec_message LOGE
#else
#define syncml_codec_message //
#endif
#define syncml_vcard_parse_message LOGE
#define syncml_message LOGE
#ifdef syncml_message
#define syncml_message LOGE
#endif

#endif
#else
#define syncml_message(x)
#define syncml_efs_message(x)
#define syncml_cb_message(x)
#define syncml_http_message(x)
#define syncml_comm_message(x)
#define syncml_task_message(x)
#define syncml_core_message(x)
#define syncml_codec_message(x)
#define syncml_vcard_parse_message(x)
#endif

/**--------------------------------------------------------------------------*
 **                         TYPE AND CONSTANT                                *
 **--------------------------------------------------------------------------*/
typedef enum {
    DM_GPRS_DEACTIVE_OK,    //DEACTIVE over
    DM_GPRS_ACTIVING,      //active processing
    DM_GPRS_ACTIVE_OK,      //ACTIVE over
    DM_GPRS_DEACTIVEING,        //DEACTIVE is going
    DM_GPRS_PDP_REJECT,
    DM_GPRS_MAX
} DM_GPRS_STATE_E;        //GPRS state

typedef struct {
    char apn[MMIDM_MAX_APN_LEN + 1];                //APN
    char username[MMIDM_MAX_USER_LEN + 1];  //user
    char password[MMIDM_MAX_PASSWORD_LEN + 1];   //password
} net_setting_info_type;

typedef struct {
    int8 use_proxy;
    char addr[MMIDM_MAX_IP_LEN + 1];
    char port[MMIDM_MAX_PORT_LEN + 1];
    char username[MMIDM_MAX_USER_LEN + 1];
    char password[MMIDM_MAX_PASSWORD_LEN + 1];
} proxy_setting_info_type;

typedef struct {
    char addr[MMIDM_MAX_URL_LEN + 1];
    char port[MMIDM_MAX_PORT_LEN + 1];
    char username[MMIDM_MAX_USER_LEN + 1];
    char password[MMIDM_MAX_PASSWORD_LEN + 1];
} server_setting_info_type;

typedef enum {
    AUTH_DUMMY = 0, AUTH_B64, AUTH_MD5
} auth_type;

/** Type of used encoding **/
typedef enum {
    SML_UNDEF = 0, SML_WBXML, SML_XML
} SmlEncoding_t;

typedef struct {
    char man[MMIDM_UA_CUSTOM_MAX_INPUT_LEN + 1];
    //char oem[MMIDM_UA_CUSTOM_MAX_INPUT_LEN+1];
    char mod[MMIDM_UA_CUSTOM_MAX_INPUT_LEN + 1];
    //char ua[MMIDM_UA_CUSTOM_MAX_INPUT_LEN+1];
} MMIDM_UA_INFO_T;

typedef struct {
    BOOLEAN is_init;

    VDM_Handle_t task_handle;
    VDM_SessionStateNotifyCB session_state_notify_cb;
#if 0//del   2011.3.14
    int m_pISocket;
    DM_GPRS_STATE_E gprs_state;
    uint32 m_netid;
#endif

    // uint32 m_session_id;
    uint32 m_max_msg_size;
    uint32 m_max_obj_size;

    auth_type m_authtype;
    SmlEncoding_t m_sml_encoding_type;
    //net_setting_info_type m_net_setting;
    proxy_setting_info_type m_proxy_setting;
    server_setting_info_type m_server_setting;

    MMIDM_UA_INFO_T m_ua_info;
    char m_imei_info[20];
    char* m_client_nextnonce[MMIDM_NEXT_NONCE];
    char* m_server_nextnonce[MMIDM_NEXT_NONCE];

} DM_PARAM_T, *DM_PARAM_P;

/**--------------------------------------------------------------------------*
 **                         FUNCTION DEFINITION                              *
 **--------------------------------------------------------------------------*/

/*==========================================================
 * function     : MMIDM_GetInitParam
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/14/2011
 ==========================================================*/
PUBLIC DM_PARAM_P MMIDM_GetInitParam(void);

/*==========================================================
 * function     : MMIDM_SetNextNonce
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/14/2011
 ==========================================================*/
PUBLIC void MMIDM_SetNextNonce(BOOLEAN is_client_nonce, char * nonce);

/**--------------------------------------------------------------------------*
 **                         Compiler Flag                                    *
 **--------------------------------------------------------------------------*/
#ifdef   __cplusplus
}
#endif

#endif

