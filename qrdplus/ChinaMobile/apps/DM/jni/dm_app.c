#include <utils/Log.h>

#include "libmem.h"
#include "smldm.h"
#include "dm_app.h"
#include "vdm_comm_pl_types.h"
#include "dm_config.h"
#include "dm_pl_tree.h"
#include "dm_pl_debug.h"
#define DM_MAX_ID_STR 32
#define MMIFMM_FILE_FILE_NAME_MAX_LEN 256
#define RDM_MAX_SEND_PACKAGE (1024*15)
#define RDM_SOCKET_DM_RECEV_BUFFER_LEN (15*1024)

#define RDM_DM_USERNAME_MAX_LEN             MMIFMM_FILE_FILE_NAME_MAX_LEN
#define RDM_DM_MAC_MAX_LEN                  MMIFMM_FILE_FILE_NAME_MAX_LEN
#define RDM_DM_ALGORITHM_LEN                MMIFMM_FILE_FILE_NAME_MAX_LEN
#define RDM_DM_SERVER_URL_LEN               MMIFMM_FILE_FILE_NAME_MAX_LEN

#define DM_DEFAULT_MAN                    "HISENSE"
#define DM_DEFAULT_MOD                    "T81"
#define DM_MAN_INFO_EXT1                    "LG"
#define DM_MOD_INFO_EXT1                    "GW880"
#define DM_VER_INFO_EXT1                    "OPhone 1.5"
#define DM_MAN_INFO_EXT2                    "Newpostcom"
#define DM_MOD_INFO_EXT2                    "N330"
#define DM_VER_INFO_EXT2                    "N330_s3b_V1.2.1"
#define DM_IMEI_NUM_EXT                     "IMEI:001010623456291"

//add  for node handler

const int DM_NULL_DIALOG = 0;
const int DM_NIA_INFO_DIALOG = 1;
const int DM_NIA_CONFIRM_DIALOG = 2;
const int DM_ALERT_INFO_DIALOG = 3;
const int DM_ALERT_CONFIRM_DIALOG = 4;
const int DM_ALERT_SINGLE_CHOICE_DIALOG = 5;
const int DM_ALERT_MULTI_CHOICE_DIALOG = 6;
const int DM_CONFIRM_DOWNLOAD_DIALOG = 7;
const int DM_CONFIRM_UPDATE_DIALOG = 8;
const int DM_SIMULATE_UPDATE_DIALOG = 9;
const int DM_PROGRESS_DIALOG = 10;
#define NULL_NONCE   "ffff"
typedef struct {
    char man[DM_MAX_ID_STR];
    char mode[DM_MAX_ID_STR];
    char version[DM_MAX_ID_STR];
    char imei[DM_MAX_ID_STR];
    BOOLEAN IdasdecType;
} MMIDM_SERVER_TYPE_T;

static MMIDM_SERVER_TYPE_T s_server_type_tab[] = { { DM_DEFAULT_MAN,
        DM_DEFAULT_MOD, DM_DEFAULT_VER, DM_DEFAULT_IMEI, TRUE }, {
        DM_MAN_INFO_EXT1, DM_MOD_INFO_EXT1, DM_VER_INFO_EXT2, DM_IMEI_NUM_EXT,
        FALSE }, { DM_MAN_INFO_EXT2, DM_MOD_INFO_EXT2, DM_VER_INFO_EXT2,
        DM_IMEI_NUM_EXT, TRUE }, { "Hisense", "HS-N51", "TI3.2.03.02.00",
        "IMEI:860206000003973", TRUE }, 0 };

typedef struct {
    char username[RDM_DM_USERNAME_MAX_LEN];
    char mac[RDM_DM_MAC_MAX_LEN];
    char algorithm[RDM_DM_ALGORITHM_LEN];
    char server_url[RDM_DM_SERVER_URL_LEN];
} RDM_CONNECT_INFO_T;

static VDM_CommObserver_t* s_dm_observer = PNULL;
static VDM_MMI_Observer_t s_observer;
static unsigned char* s_dm_OutPackage_ptr = PNULL;
static MMIDM_PL_ALERT_TYPE s_pl_alert_type = DM_PL_ALERT_NONE;
static E_VDM_NIA_UIMode_t s_UIMode = E_VDM_NIA_UIMode_NotSpecified;
static MMIDM_INFO_T s_dm_info;

static char* s_loc_algorithm = NULL;
static char* s_loc_username = NULL;
static char* s_loc_mac = NULL;
char* s_recvBuffer_ptr = PNULL;
uint32 s_recvActualLen = 0;
static int16 s_dm_isWBXML = 1;
static uint32 s_recvpos = 0;
static BOOLEAN s_is_dm_run = FALSE; //mmi task state
static RDM_CONNECT_INFO_T* s_connect_info_ptr = PNULL;

extern const int DEVID_IO_HANDLER;
//extern const int FROMFILE_IO_HANDLER;
extern const int MODEL_IO_HANDLER;
extern const int MAN_IO_HANDLER;
extern const int DM_APN_IO_HANDLER;
extern const int SERVER_ADDR_IO_HANDLER;

extern BOOLEAN MMIDM_ActivePdpConnect(void);
extern void DMTaskStart(MMIDM_SESSION_T* signal_ptr);

/*!*******************************************************************************
 * Perform MMI initialization. This will be called before any other MMI
 * function is called. This MUST NOT cause anything to be displayed.
 *
 * \return  TRUE if initialization was successful, FALSE if not.
 ********************************************************************************/
IBOOL VDM_MMI_PL_init(VDM_MMI_Observer_t* inObserver) {
    if (PNULL == inObserver) {
        DM_TRACE("MMIDM==> VDM_MMI_PL_init fail!");
        return FALSE;
    }
    SCI_MEMCPY(&s_observer, inObserver, sizeof(VDM_MMI_Observer_t));
    return TRUE;
}

/*!*******************************************************************************
 * Perform MMI termination. No MMI functions will be called after this has been
 * called, except for perhaps \ref VDM_MMI_PL_init() to reinitialize the MMI
 * system
 ********************************************************************************/
void VDM_MMI_PL_term(void) {
    DM_TRACE("MMIDM==> VDM_MMI_PL_term inInfoType ");
    SCI_MEMSET(&s_observer, 0, sizeof(VDM_MMI_Observer_t));
}

DM_PARAM_T mmidm_init_param = { 0 };
DM_PARAM_P MMIDM_GetInitParam(void) {
    return &mmidm_init_param;
}

/*****************************************************************************/
//  Description : set server nonce
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
static BOOLEAN MMIDM_SetServerNonce(char* nonce) {
    if ((PNULL != nonce) && (strlen(nonce) <= DM_MAX_NONCE_LEN)) {
        SCI_MEMCPY(s_dm_info.server_nonce, nonce, (strlen(nonce)+1) * sizeof(char));
//        MMINV_WRITE(MMINV_DM_SERVER_NONCE, nonce);
        JMMIDM_SetServerNonce(s_dm_info.server_nonce);
        return TRUE;
    } else {
        return FALSE;
    }
}

/*****************************************************************************/
//  Description : set client nonce
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
static BOOLEAN MMIDM_SetClientNonce(char* nonce) {
    if ((PNULL != nonce) && (strlen(nonce) <= DM_MAX_NONCE_LEN)) {
        SCI_MEMCPY(s_dm_info.client_nonce, nonce, (strlen(nonce)+1) * sizeof(char));
//        MMINV_WRITE(MMINV_DM_CLIENT_NONCE, nonce);
        JMMIDM_SetClientNonce(s_dm_info.client_nonce);
        return TRUE;
    } else {
        return FALSE;
    }
}

void MMIDM_SetNextNonce(BOOLEAN is_client_nonce, char * nonce) {
    DM_TRACE("MMIDM==> MMIDM_SetNextNonce: is_client_nonce = %d, nonce = %s", is_client_nonce, nonce);
    if (is_client_nonce) {
        MMIDM_SetClientNonce(nonce);
    } else {
        MMIDM_SetServerNonce(nonce);
    }
}

E_VDM_MMI_Result_t VDM_MMI_PL_confirmationQuery(
        VDM_MMI_ScreenContext_t* inScreenContext,
        E_VDM_MMI_ConfirmCmd_t inDefaultCommand) {
    E_VDM_MMI_Result_t result = E_VDM_MMI_Result_OK;

    DM_TRACE("MMIDM==> VDM_MMI_PL_infoMessage inDefaultCommand = %d",inDefaultCommand);
    s_pl_alert_type = DM_PL_ALERT_1101;
//    DM_OpenDMAlerthandleWin(MMIDM_ALERT_PL_CONFIRM, inScreenContext->displayText, (uint16)strlen((char*)inScreenContext->displayText),inScreenContext->maxDisplayTime);

    return result;
}

//This for the part of vdm_pl_async
/*!
 *******************************************************************************
 * Allow asynchronous object to explicitly yield the processor.
 *
 * This is useful for non-preemptive environments, such as BREW.
 ******************************************************************************
 */
IBOOL VDM_PL_Async_signal(VDM_Handle_t inAsyncHandle) {
    uint32 task_id = (uint32) inAsyncHandle;
//    MMIDM_SESSION_T session ={0};
//    session.type = DM_SESSION_NONE;
//    session.message_buffer = PNULL;
//    session.msg_size = 0;
//    DM_TRACE("MMIDM==> VDM_PL_Async_signal ");
//
//    MMIDM_SendSigToDmTask(DM_TASK_RUN_MESSAGE,task_id,&session);
    return TRUE;
}

PUBLIC void VDM_Notify_PL_Task_Finished(void) {
//    MMIDM_SESSION_T session ={0};
//    session.type = DM_SESSION_NONE;
//    session.message_buffer = PNULL;
//    session.msg_size = 0;
//    DM_TRACE("MMIDM==> VDM_Notify_PL_Task_Finished ");
//
//    MMIDM_SendSigToDmTask(DM_TASK_EXIT_MESSAGE,MMIDM_GetDmTaskID(),&session);
    MMIDM_ExitDM();
    MMIDM_SetDmState(DM_NONE);
}

/********************************************************************************
 * Terminate the connection.
 *
 * Terminate the communications support code and perform any cleanup required.
 *
 * \param   inContext   The context previously set by VDM_Comm_PL_HTTP_init().
 *
 * \return  \ref VDM_ERR_defs "An error code" (VDM_ERR_OK if no error)
 ********************************************************************************/
VDM_Error VDM_Comm_PL_HTTP_term(void* inContext) {
    DM_TRACE("MMIDM==>  VDM_Comm_PL_HTTP_term called\n");

    if (PNULL != s_loc_algorithm) {
        SCI_FREE(s_loc_algorithm);
        s_loc_algorithm = PNULL;
    }
    if (PNULL != s_loc_username) {
        SCI_FREE(s_loc_username);
        s_loc_username = PNULL;
    }
    if (PNULL != s_loc_mac) {
        SCI_FREE(s_loc_mac);
        s_loc_mac = PNULL;
    }

    if (PNULL != s_dm_OutPackage_ptr) {
        SCI_FREE(s_dm_OutPackage_ptr);
        s_dm_OutPackage_ptr = PNULL;
    }
    if (PNULL != s_dm_observer) {
        SCI_FREE(s_dm_observer);
        s_dm_observer = PNULL;
    }

    if (PNULL != s_connect_info_ptr) {
        SCI_FREE(s_connect_info_ptr);
        s_connect_info_ptr = PNULL;
    }
    if (PNULL != s_recvBuffer_ptr) {
        SCI_FREE(s_recvBuffer_ptr);
        s_recvBuffer_ptr = PNULL;
    }
    DM_TRACE("MMIDM==> VDM_Comm_PL_HTTP_term returns\n");
    return VDM_ERR_OK;
}

/********************************************************************************
 * End the request/response exchange for the current message.
 *
 * \param   inContext   The context previously set by VDM_Comm_PL_HTTP_init().
 *
 * \param   inConnId    Reference to the connection, as returned by
 *                      VDM_Comm_PL_HTTP_open().
 *******************************************************************************/
void VDM_Comm_PL_HTTP_close(void* inContext, IS32 inConnId) {
//    sci_sock_socketclose(s_dm_Socket_Id);
//    s_dm_Socket_Id = 0;
    DM_TRACE("MMIDM==>  VDM_Comm_PL_HTTP_close (connId = %d) returns\n", inConnId);
}

/*********************************************************************************
 //  Description : parse syncml
 //  Global resource dependence :
 //  Note:
 *********************************************************************************/
short RDM_parse_syncml(char* infoStr, char** username, char** mac,
        char** algorithm) {
    //char * first_occ_str = strstr((char*)infoStr, "x-syncml-hmac:");
    char * first_occ_str = strcasestr((char*) infoStr, "x-syncml-hmac:"); // modify for ignore case
    DM_TRACE("MMIDM==> [RDM_parse_syncml]  begin\r\n");

    if (PNULL != first_occ_str) {
        char* end_syncml_hdr_ptr = (char*) strstr((char*) first_occ_str,
                "\r\n");
        char* username_occ_ptr = PNULL;
        char* username_end_ptr = PNULL;
        char* mac_occ_ptr = PNULL;
        char* mac_end_ptr = PNULL;
        char* algorithm_occ_ptr = PNULL;
        char* algorithm_end_ptr = PNULL;
        int32 username_len = 0;
        int32 mac_len = 0;
        int32 algorithm_len = 0;
        int32 syncml_hdr_len = end_syncml_hdr_ptr - first_occ_str;
        char* syncml_hdr_ptr = SCI_ALLOCA((syncml_hdr_len + 1) * sizeof(char));/*lint !e737 */
        if (PNULL == syncml_hdr_ptr) {
            DM_TRACE("MMIDM==> [RDM_parse_syncml]  syncml_hdr_len=%d",syncml_hdr_len);

            return RDM_ERR_MEMORY;
        }
        /*
        * Initializing syncml_hdr_ptr with the null character to avoid any possibility of
        * string without null character termination issue.
        */
        SCI_MEMSET(syncml_hdr_ptr, '\0', syncml_hdr_len + 1);
        SCI_MEMCPY(syncml_hdr_ptr, first_occ_str,
                syncml_hdr_len * sizeof(char));/*lint !e737 */
        end_syncml_hdr_ptr = syncml_hdr_ptr + syncml_hdr_len;
        username_occ_ptr = strstr(syncml_hdr_ptr, (char*) "username=");
        if (PNULL != username_occ_ptr) {
            username_end_ptr = strstr(username_occ_ptr, (char*) ",");
        }
        mac_occ_ptr = strstr(syncml_hdr_ptr, (char*) "mac=");
        if (PNULL != mac_occ_ptr) {
            mac_end_ptr = strstr(mac_occ_ptr, (char*) ",");
        }
        algorithm_occ_ptr = strstr(syncml_hdr_ptr, (char*) "algorithm=");
        if (PNULL != algorithm_occ_ptr) {
            algorithm_end_ptr = strstr(algorithm_occ_ptr, (char*) ",");
        }
//        if(!username_occ_ptr || !mac_occ_ptr || !algorithm_occ_ptr)/*lint !e774 */
//        {
//            DM_TRACE("MMIDM==> [RDM_parse_syncml]2 RDM_ERR_COMMS_NON_FATAL");
//            return RDM_ERR_COMMS_NON_FATAL;
//         }

        username_occ_ptr += 9;
        mac_occ_ptr += 4;
        algorithm_occ_ptr += 10;
        if (PNULL != username_end_ptr) {
            username_len = username_end_ptr - username_occ_ptr;
        } else {
            username_len = end_syncml_hdr_ptr - username_occ_ptr;
        }
        if (PNULL != algorithm_end_ptr) {
            algorithm_len = algorithm_end_ptr - algorithm_occ_ptr;
        } else {
            algorithm_len = end_syncml_hdr_ptr - algorithm_occ_ptr;
        }
        if (PNULL != mac_end_ptr) {
            mac_len = mac_end_ptr - mac_occ_ptr;
        } else {
            mac_len = end_syncml_hdr_ptr - mac_occ_ptr;
        }
        if (PNULL != s_loc_algorithm) {
            SCI_FREE(s_loc_algorithm);
            s_loc_algorithm = PNULL;
        }
        if (PNULL != s_loc_username) {
            SCI_FREE(s_loc_username);
            s_loc_username = PNULL;
        }
        if (PNULL != s_loc_mac) {
            SCI_FREE(s_loc_mac);
            s_loc_mac = PNULL;
        }

        s_loc_username = SCI_ALLOC_APP((username_len + 1) * sizeof(char));/*lint !e737 */
        s_loc_mac = SCI_ALLOC_APP((mac_len + 1) * sizeof(char));/*lint !e737 */
        s_loc_algorithm = SCI_ALLOC_APP((algorithm_len + 1) * sizeof(char));/*lint !e737 */
        if (PNULL == s_loc_algorithm || PNULL == s_loc_algorithm
                || PNULL == s_loc_username) {
            DM_TRACE("MMIDM==> [RDM_parse_syncml]3 RDM_ERR_MEMORY");
            return RDM_ERR_MEMORY;
        }
        username_occ_ptr[username_len] = '\0';
        mac_occ_ptr[mac_len] = '\0';
        algorithm_occ_ptr[algorithm_len] = '\0';
        SCI_MEMCPY(s_loc_username, username_occ_ptr, username_len + 1);
        if (s_loc_mac != NULL) {
            SCI_MEMCPY(s_loc_mac, mac_occ_ptr, mac_len + 1);
        }
        SCI_MEMCPY(s_loc_algorithm, algorithm_occ_ptr, algorithm_len + 1);
        *username = s_loc_username;
        DM_TRACE("MMIDM==> [RDM_parse_syncml] s_loc_username =%s",s_loc_username);
        *mac = s_loc_mac;
        DM_TRACE("MMIDM==> [RDM_parse_syncml] s_loc_mac =%s",s_loc_mac);
        *algorithm = s_loc_algorithm;
        DM_TRACE("MMIDM==> [RDM_parse_syncml] s_loc_algorithm =%s",s_loc_algorithm);
        //  if(PNULL != syncml_hdr_ptr)
        {
            //      SCI_FREE(syncml_hdr_ptr);
            //      syncml_hdr_ptr = PNULL;
        }
        //VDM_PL_free(syncml_hdr);
    } else {
        //  *username = NULL;
        //  *mac = NULL;
        //  *algorithm = NULL;
        DM_TRACE("MMIDM==> [RDM_parse_syncml]  no mac\r\n");
    }
    return RDM_ERR_OK;
}
/*********************************************************************************
 //  Description : memory compare
 //  Global resource dependence :
 //  Note:
 *********************************************************************************/LOCAL BOOLEAN RDM_COMMS_memcmpN(
        uint8* str1, uint8* str2, int N) {
    uint16 i = 0;

    if (PNULL == str1 || PNULL == str2) {
        return FALSE;
    }
    for (i = 0; i < N; i++) {
        if (str1[i] != str2[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

/*********************************************************************************
 //  Description : find next char
 //  Global resource dependence :
 //  Note:
 *********************************************************************************/LOCAL uint8* RDM_COMMS_findNextCharN(
        uint8* str, char ch, int N) {

    uint8* strN = str + N;
    uint8* strtmp = PNULL;

    for (strtmp = str; strtmp < strN; strtmp++) {
        if (*strtmp == ch) {
            return strtmp;
        }
    }
    return PNULL;
}
/*********************************************************************************
 //  Description : find substring
 //  Global resource dependence :
 //  Note:
 *********************************************************************************/LOCAL uint8* RDM_COMMS_findSubStringN(
        uint8* str, uint8* subStr, int N) {

    BOOLEAN is_same = FALSE;
    uint8* curr_ptr = PNULL;
    uint8* next_ptr = PNULL;
    uint8* tmp_ptr = PNULL;
    const char fc = subStr[0];
    uint32 len = strlen((char*) subStr);
    uint8* strN = str + N - 4;
    dm_debug_trace(DM_DEBUG_SOCKET_MASK,
            "MMIDM==> [findSubStringN] was called len= 0x%x,N= 0x%x,strN=0x%x\r\n",
            len, N, strN);

    for (curr_ptr = str; curr_ptr < strN; curr_ptr++) {
        next_ptr = RDM_COMMS_findNextCharN(curr_ptr, fc,
                N - (uint32) curr_ptr - 4 + (int) str);/*lint !e737 */

        if (PNULL == next_ptr) {
            return NULL ;
        }
        // print out the following 4 char
        tmp_ptr = next_ptr;

        is_same = RDM_COMMS_memcmpN(next_ptr, subStr, len);

        if (!is_same) {
            dm_debug_trace(DM_DEBUG_SOCKET_MASK,
                    "MMIDM==> [findSubStringN]found the whole string,next =0x%x\r\n",
                    next_ptr);
            return next_ptr;
        } else {
            curr_ptr = next_ptr;
        }
    }
    return NULL ;
}

/********************************************************************************
 * Receive a reply. The data is expected to be written to the buffer supplied
 * in the argument 'ioMsgBuf', which can hold a maximum of 'inMsgBufSize' bytes of data.
 * The length of the data read in is returned in 'outMsgLen'. This function is
 * expected to read the full content of the reply before returning. If the
 * buffer is not large enough, VDM_ERR_BUFFER_OVERFLOW should be returned and
 * the required buffer size returned in outMsgLen. If the mime-type of the reply does
 * not match the mime-type for the document sent (ie if XML is received when
 * WBXML was sent) then an error (VDM_ERR_COMMS_MIME_MISMATCH) should be
 * returned.
 *
 * \param   inContext    The context previously set by VDM_Comm_PL_HTTP_init().
 *
 * \param   inConnId     Reference to the connection, as returned by
 *                       VDM_Comm_PL_HTTP_open().
 *
 * \param   ioMsgBuf     Pointer to a buffer to receive the incoming message.
 *
 * \param   inMsgBufSize The maximum number of bytes which can be held in the
 *                       supplied buffer.
 *
 * \param   outMsgLen    The number of bytes read.
 *
 * \param   outHMACinfo  Structure that will be filled with information from the
 *                       HMAC credentials or NULL if no HMAC credentials were
 *                       supplied. This pointer need only remain valid until
 *                       VDM_Comm_PL_DM_endMessage() is called.
 *
 * \return  \ref VDM_ERR_defs "An error code" (VDM_ERR_OK if no error)
 ********************************************************************************/
VDM_Error VDM_Comm_PL_HTTP_receive(
        void* inContext, //dataPtr
        IS32 inConnId, IU8 * ioMsgBuf, IS32 inMsgBufSize, IS32* outMsgLen,
        VDM_Comm_HMAC_t* outHMACinfo)   //username, mac, algorithm
{
    RDM_Error ret = RDM_ERR_OK;

    //int32   totalRead = 0;
    int32 LenHTTPbody = 0;
    uint8* t1_ptr = PNULL;
    //uint8* findMac = NULL;
    // The actual recv was done inside RDM_COMMS_sendMessage and stored locally.
    // Here we just copy the stored data.

    DM_TRACE("MMIDM==> [VDM_Comm_PL_HTTP_receive]  bufSize = %d, s_recvActualLen = %d\r\n",inMsgBufSize, s_recvActualLen);

    if ((uint32) inMsgBufSize < s_recvActualLen) {
        DM_TRACE("MMIDM==> [VDM_Comm_PL_HTTP_receive]  Error:RDM_ERR_BUFFER_OVERFLOW!\r\n");
        ret = RDM_ERR_BUFFER_OVERFLOW;
    }
    /*
     t1_ptr = (uint8*)RDM_COMMS_findSubStringN((uint8*)s_recvBuffer_ptr,(uint8*)"\r\n\r\n",(int)s_recvActualLen);

     if (PNULL == t1_ptr)
     {
     DM_TRACE("MMIDM==> [VDM_Comm_PL_HTTP_receive]  return error when cut the http header, t1_ptr=0x%x\r\n",t1_ptr);
     ret =  RDM_ERR_BAD_INPUT;
     return ret;
     }
     else
     {
     LenHTTPbody = (uint32)s_recvBuffer_ptr+ s_recvActualLen-(IU32)t1_ptr-4;
     DM_TRACE("MMIDM==> [VDM_Comm_PL_HTTP_receive]  LenHTTPbody=0x%x\r\n",LenHTTPbody);

     SCI_MEMCPY(ioMsgBuf,t1_ptr+4,LenHTTPbody);
     *outMsgLen = LenHTTPbody;
     }
     */SCI_MEMCPY(ioMsgBuf, s_recvBuffer_ptr, s_recvActualLen);
    *outMsgLen = s_recvActualLen;

    /*
     ret = RDM_parse_syncml((char*)s_recvBuffer_ptr, (char**)&(outHMACinfo->username), (char**)&(outHMACinfo->mac), (char**)&(outHMACinfo->algorithm));
     */DM_TRACE("MMIDM==> RDM_COMMS_recvMessage return ret =%d\r\n", ret);
    return ret;

}

/********************************************************************************
 * Send the data for the next msg to the specified address. This function is
 * not expected to return until all the data has been sent.
 *
 * \param   inContext   The context previously set by VDM_Comm_PL_HTTP_init().
 *
 * \param   inConnId    Reference to the connection, as returned by
 *                      VDM_Comm_PL_HTTP_open().
 *
 * \param   inMsg       Pointer to the data to be sent.
 *
 * \param   inMsgLen    The length of the data to send.
 *
 * \return  \ref VDM_ERR_defs "An error code" (VDM_ERR_OK if no error)
 ********************************************************************************/
VDM_Error VDM_Comm_PL_HTTP_send(void* inContext, IS32 inConnId, IU8 * inMsg,
        IS32 inMsgLength) {
    RDM_Error result = RDM_ERR_OK;
    int16 total = 0;
    unsigned int sentLen = 0;
    VDM_CommObserver_t* observer = (VDM_CommObserver_t*) inContext;
    DM_TRACE("MMIDM==> VDM_Comm_PL_HTTP_send\r\n");

//    total = (int16)RDM_COMMS_Construct_http(s_dm_OutPackage_ptr, RDM_MAX_SEND_PACKAGE, inMsg, inMsgLength);
//    if(total > 0)
//    {
    short re = 0;
    re = pimTaskComm_SendData(); /*lint !e64*/
    if (re < 0)/*lint !e737 */
    {
        result = observer->notifyCommBroken(observer->context,
                VDM_SessionType_DM, FALSE);
    }
//    }
//    else
//    {
//        result = observer->notifyCommBroken(observer->context,VDM_SessionType_DM,FALSE);
//    }

    DM_TRACE("MMIDM==> VDM_Comm_PL_HTTP_send, total = %d, sentLen = %d",total, sentLen);
    return result;
}

/********************************************************************************
 * Mark the start of a new message to the server.
 *
 * If HMAC authentication is to be sent then both username and mac must be
 * supplied but algorithm need only be supplied if not MD5.
 *
 * \param   inContext   The context previously set by VDM_Comm_PL_HTTP_init().
 *
 * \param   outConnId   Opaque application data, used to identify the
 *                      connection. every subsequent event/API regarding
 *                      this message should pass this value
 *
 * \param   inAddr      The address of the SyncML Server.
 *
 * \param   inMsgLen    The length of the message to be sent.
 *
 * \param   inMimeType  Whether the data is in DM-WBXML, DM-XML, DS-WBXML, DS-XML format.
 *
 * \param   inHMACinfo  Information used for HMAC authentication or NULL.
 *
 * \return  \ref VDM_ERR_defs "An error code" (VDM_ERR_OK if no error)
 ********************************************************************************/
VDM_Error VDM_Comm_PL_HTTP_open(void* inContext, IS32* outConnId,
        UTF8Str inAddr, IS32 inMsgLen, IU32 inMimeType,
        VDM_Comm_HMAC_t* inHMACinfo) {
    const char *ipA = PNULL;
    DM_TRACE("MMIDM==> [VDM_Comm_PL_HTTP_open] begin, *outConnId = %d, addr = %s \r\n", *outConnId, inAddr);
    DM_TRACE("MMIDM==> [VDM_Comm_PL_HTTP_open]   inMimeType = %d, msgLen = %d,addr = %d \r\n", inMimeType, inMsgLen);

    SCI_MEMSET(s_connect_info_ptr->username, 0x00, RDM_DM_USERNAME_MAX_LEN);
    SCI_MEMSET(s_connect_info_ptr->mac, 0x00, RDM_DM_MAC_MAX_LEN);
    SCI_MEMSET(s_connect_info_ptr->algorithm, 0x00, RDM_DM_ALGORITHM_LEN);
    SCI_MEMSET(s_connect_info_ptr->server_url, 0x00, RDM_DM_SERVER_URL_LEN);

    s_dm_isWBXML = (int16) (inMimeType ? 1 : 0);
    if (inAddr) {
        strlcpy(s_connect_info_ptr->server_url, (const char*) inAddr,
                sizeof(s_connect_info_ptr->server_url));
    }
    if (inHMACinfo) {
        DM_TRACE("MMIDM==> [VDM_Comm_PL_HTTP_open]   username %x = %s, mac %x = %s, algorithm %x = %s \r\n", inHMACinfo->username,inHMACinfo->username, inHMACinfo->mac,inHMACinfo->mac,inHMACinfo->algorithm, inHMACinfo->algorithm);
        if (inHMACinfo->username) {
            strlcpy(s_connect_info_ptr->username, (char*) inHMACinfo->username,
                    sizeof(s_connect_info_ptr->username));
        }
        if (inHMACinfo->mac) {
            strlcpy(s_connect_info_ptr->mac, (char*) inHMACinfo->mac,
                    sizeof(s_connect_info_ptr->mac));
        }
        if (inHMACinfo->algorithm) {
            strlcpy(s_connect_info_ptr->algorithm,
                    (char*) inHMACinfo->algorithm, sizeof(s_connect_info_ptr->algorithm));
        }
    }
    ipA = (const char *) inAddr;
    DM_TRACE("MMIDM==> [VDM_Comm_PL_HTTP_open] s_connect_info_ptr->server_url = %s\r\n",s_connect_info_ptr->server_url);

    *outConnId = VDM_SessionType_DM;

    if (!MMIDM_ActivePdpConnect()) {
        DM_TRACE("MMIDM==> [VDM_Comm_PL_HTTP_open] ActivePdp fail!");
        //MMIDM_ErrorAlert(0,*outConnId);
        s_dm_observer->notifyCommBroken(s_dm_observer->context,
                VDM_SessionType_DM, FALSE);
        return VDM_ERR_UNSPECIFIC;
    } else {
//       if(0 !=RDM_Socket_open(ipA, RDM_COMMS_GetDMSocketID_cbFunc)) /*lint !e64*/
        DM_TRACE("MMIDM==> [VDM_Comm_PL_HTTP_open] open OK!");
        s_dm_observer->notifyCommOpen(s_dm_observer->context,
                VDM_SessionType_DM);
        //MMIDM_ErrorAlert(sci_sock_errno(s_dm_Socket_Id),VDM_SessionType_DM);
        //   s_dm_observer->notifyCommBroken(s_dm_observer->context,*outConnId,FALSE);
    }
    return VDM_ERR_OK;
}

/********************************************************************************
 * Initialize the http communication support code.
 *
 * Provide the AddrType to be used for the SyncMLDM access.
 *
 * \param   outContext VDM_Comm_PL_HTTP's opaque context.
 *
 * \param   inAddrType  The type of the address specified. From the DMStdObj
 *                      specification the possible values are:"URI","IPv4","IPv6".
 *
 * \param   inMaxMsgSize    Maximum size allowed for a message.
 *
 * \param   inObserver      A structure containing callback functions that
 *                          must be invoked at the end of each async operation
 *
 *
 * \return  \ref VDM_ERR_defs "An error code" (VDM_ERR_OK if no error).
 ********************************************************************************/
VDM_Error VDM_Comm_PL_HTTP_init(void** outContext, UTF8CStr inAddrType,
        IU32 inMaxMsgSize, VDM_CommObserver_t* inObserver) {
    VDM_Error result = VDM_ERR_OK;

    DM_TRACE("MMIDM==>  VDM_Comm_PL_HTTP_init called (inAddrType=%s)\n", inAddrType);
    UNUSED(inAddrType);
    UNUSED(inMaxMsgSize);

    if (PNULL == s_dm_observer) {
        s_dm_observer = SCI_ALLOC_APP(sizeof(VDM_CommObserver_t));
    }
    if (PNULL != s_dm_observer) {
        SCI_MEMSET(s_dm_observer, 0x00, sizeof(VDM_CommObserver_t));
        SCI_MEMCPY(s_dm_observer, inObserver, sizeof(VDM_CommObserver_t));
    } else {
        result = VDM_ERR_MEMORY;
    }

    if (PNULL == s_dm_OutPackage_ptr) {
        s_dm_OutPackage_ptr = SCI_ALLOC_APP(RDM_MAX_SEND_PACKAGE);
    }
    if (PNULL != s_dm_OutPackage_ptr) {
        SCI_MEMSET(s_dm_OutPackage_ptr, 0, RDM_MAX_SEND_PACKAGE);
    } else {
        result = VDM_ERR_MEMORY;
    }

    if (PNULL == s_connect_info_ptr) {
        s_connect_info_ptr = SCI_ALLOC_APP(sizeof(RDM_CONNECT_INFO_T));
    }
    if (PNULL != s_connect_info_ptr) {
        SCI_MEMSET(s_connect_info_ptr, 0, sizeof(RDM_CONNECT_INFO_T));
    } else {
        result = VDM_ERR_MEMORY;
    }

    if (PNULL == s_recvBuffer_ptr) {
        s_recvBuffer_ptr = SCI_ALLOC_APP(RDM_SOCKET_DM_RECEV_BUFFER_LEN+1);
    }
    if (PNULL != s_recvBuffer_ptr) {
        SCI_MEMSET(s_recvBuffer_ptr, 0, (RDM_SOCKET_DM_RECEV_BUFFER_LEN + 1));
    } else {
        result = VDM_ERR_MEMORY;
    }
    s_recvActualLen = 0;
    s_recvpos = 0;
    *outContext = NULL;
    DM_TRACE("MMIDM==> VDM_Comm_PL_HTTP_init called s_dm_observer=%x\n", s_dm_observer);
    DM_TRACE("MMIDM==> VDM_Comm_PL_HTTP_init returns 0x%x\n", result);

    return result;
}

/*****************************************************************************/
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
void MMIDM_InitParam(void) {
    DM_PARAM_P init_param = &mmidm_init_param;

    SCI_MEMSET(init_param, 0, sizeof(DM_PARAM_T));

    init_param->task_handle = -1 /*MMIDM_CreateThread()*/;
    init_param->m_max_msg_size = DM_CLIENT_PL_MAX_MSG_SIZE;
    init_param->m_max_obj_size = DM_CLIENT_PL_MAX_OBJ_SIZE;
    init_param->m_authtype = (auth_type) dmTreeParam_GetAuthType();
    init_param->m_sml_encoding_type = SML_XML;

    if (strcmp(MMIDM_GetDmProfile(), DM_APN_CMWAP_INFO) == 0) {
        init_param->m_proxy_setting.use_proxy = 1;
        strlcpy(init_param->m_proxy_setting.addr, DM_PROXY_IP_NUMBER,
                sizeof(init_param->m_proxy_setting.addr));
        strlcpy(init_param->m_proxy_setting.port, DM_PROXY_PORT_NUMBER,
                sizeof(init_param->m_proxy_setting.port));
    }
    //else
    {
        strlcpy(init_param->m_server_setting.addr, MMIDM_GetSrvAddURL(),
                sizeof(init_param->m_server_setting.addr));
        DM_TRACE("MMIDM==> MMIDM_InitParam: ser_add = %s", init_param->m_server_setting.addr);
    }

    strlcpy(init_param->m_ua_info.man, MMIDM_GetMan(), sizeof(init_param->m_ua_info.man));
    strlcpy(init_param->m_ua_info.mod, MMIDM_GetModel(), sizeof(init_param->m_ua_info.mod));
    strlcpy(init_param->m_imei_info, DM_GetDevImeiInfo(), sizeof(init_param->m_imei_info));

    strlcpy(init_param->m_server_nextnonce, MMIDM_GetServerNonce(),
            sizeof(init_param->m_server_nextnonce));
    strlcpy(init_param->m_client_nextnonce, MMIDM_GetClientNonce(),
            sizeof(init_param->m_client_nextnonce));

    strlcpy(init_param->m_server_setting.username,
            dmTreeParam_getDmAccInfo()->accUserName, sizeof(init_param->m_server_setting.username));
    strlcpy(init_param->m_server_setting.password,
            dmTreeParam_getDmAccInfo()->accUserPw, sizeof(init_param->m_server_setting.password));

    DM_TRACE("MMIDM==> MMIDM_InitParam: dmTreeParam_getDmAccInfo()->accUserName = %s", dmTreeParam_getDmAccInfo()->accUserName);
    DM_TRACE("MMIDM==> MMIDM_InitParam: dmTreeParam_getDmAccInfo()->accUserPw = %s", dmTreeParam_getDmAccInfo()->accUserPw);

//     init_param->session_state_notify_cb=DM_SessionStateChange;
}

/*****************************************************************************/
//  Description : The function is NIA_Plugin_handleAlert
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
static VDM_Error NIA_Plugin_handleAlert(E_VDM_NIA_UIMode_t inUIMode,
        IU16 inDMVersion, UTF8Str inVendorSpecificData,
        IU16 inVendorSpecificDataLength, VDM_SessionContext_t* inSessionContext) {
    VDM_Error result = VDM_ERR_OK;

    DM_TRACE("MMIDM==>  NIA_Plugin_handleAlert inUIMode =%d",inUIMode);

    s_UIMode = inUIMode;
    switch (inUIMode) {
    case E_VDM_NIA_UIMode_NotSpecified:
    case E_VDM_NIA_UIMode_Background:
        MMIDM_NotifyNIASessionProceed();
        break;
    case E_VDM_NIA_UIMode_Informative:
        dmTaskComm_displayDialog(DM_NIA_INFO_DIALOG, inVendorSpecificData, 30);
        //UI alert
//          DM_OpenDMAlerthandleWin(MMIDM_ALERT_NIA_DISPLAY,inVendorSpecificData,inVendorSpecificDataLength,0);
        break;
    case E_VDM_NIA_UIMode_UI:
        dmTaskComm_displayDialog(DM_NIA_CONFIRM_DIALOG, inVendorSpecificData,
                30);
//          DM_OpenDMAlerthandleWin(MMIDM_ALERT_NIA_CONFIRM,inVendorSpecificData,inVendorSpecificDataLength,0);
        break;
    default:
        break;
    }
    return result;
}

/*****************************************************************************/
//  Description : init dm session
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
static VDM_Error DM_initiateSession(DM_SESSION_TYPE type, char* msg_body,
        uint32 msg_size) {
    VDM_Error result = VDM_ERR_OK;
//    E_VDM_FUMO_RESULT_t fumo_result = E_VDM_FUMO_RESULT_client_error;

    DM_TRACE("MMIDM==>  DM_initiateSession type =%d",type);
    switch (type) {
    case DM_SESSION_USER:
        DM_TRACE("DM_initiateSession DM_SESSION_USER");
//           result = MMIDM_TriggerDMSession(NULL, NULL, NULL, 0, NULL);
        break;
    case DM_SESSION_SERVER:
        DM_TRACE("DM_initiateSession DM_SESSION_SERVER");
        result = MMIDM_TriggerNIADMSession(msg_body, msg_size,
                NIA_Plugin_handleAlert, NULL );/*lint !e64*/
        break;
    case DM_SESSION_CLIENT:
//           result = MMIDM_FUMO_triggerSession(s_fumo_instance,NULL,0,E_VDM_FUMO_Client_device);
        break;
    case DM_SESSION_FUMO_RESUME:
//           result = MMIDM_FUMO_SessionController_triggerDL(s_fumo_instance,E_VDM_FUMO_Session_dlAndUpdate);
        break;
    case DM_SESSION_SCOMO_RESUME:
//           result = MMIDM_SCOMO_DP_SessionHandler_DL_trigger(s_scomoInstance->scomoDP,E_VDM_SCOMO_DP_Session_dlAndInstall);
        break;
    case DM_SESSION_FOTA:
//            if (1 == msg_size)//E_FOTA_SUCCESS
//            {
//                fumo_result = E_VDM_FUMO_RESULT_successful;
//            }
//            else //E_FOTA_UNCOMPATIBLE
//            {
//                fumo_result = E_VDM_FUMO_RESULT_client_error;
//            }
//            result = MMIDM_FUMO_setUpdateResult(s_fumo_instance,fumo_result);
        break;
    case DM_SESSION_REMOVE:
//            result = DM_RemoveDCFromInventory(msg_size);
        break;
    case DM_SESSION_ADD:
//            result = DM_SetDPInstallResult(SCOMO_Successful,msg_size,&s_dp_header);
//            MMIDM_CleanInstallFile();
        break;
    default:
        break;
    }

    return result;
}

/*****************************************************************************/
//  Description : start dm
//  Global resource dependence :
//  Note:
/*****************************************************************************/
DM_START_RESULT_E MMIDM_StartVDM(DM_SESSION_TYPE type, char* msg_body,
        uint32 msg_size) {
    VDM_Error result = FALSE;
    DM_TREE_HANDLE tree_handle = PNULL;

    DM_TRACE("MMIDM==> MMIDM_StartVDM === start ===");

    //set dm start state
    MMIDM_SetDmState(DM_START);

#if 1
    //init tree
    tree_handle = dmTree_read();
    if (PNULL == tree_handle) {
        DM_TRACE("MMIDM==> MMIDM_StartVDM Read tree fail!");
        return DM_START_FAIL;
    }
    DM_TRACE("MMIDM==> MMIDM_StartVDM dmTree_read end!");
    //init param
    MMIDM_InitParam();
    DM_TRACE("MMIDM==> MMIDM_StartVDM MMIDM_InitParam end!");
#endif

    //MMIDM_RegisterSessionStateObserver(DM_SessionStateChange);
    DM_TRACE("MMIDM==> MMIDM_StartVDM === MMIDM_RegisterSessionStateObserver ===");

    //initiate session
    result = DM_initiateSession(type, msg_body, msg_size);
    if (result != VDM_ERR_OK) {
        DM_TRACE("MMIDM==> MMIDM_StartVDM Failed to trigger session: error 0x%x", result);
        return DM_START_FAIL;
    }

//    MMIDM_SetDmState(DM_RUN);
    DM_TRACE("MMIDM==> MMIDM_StartVDM === end ===");

    return DM_START_SUCC;
}

/*****************************************************************************/
//  Description : start dm session
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
PUBLIC BOOLEAN MMIDM_StartDm(DM_SESSION_TYPE type, char* msg_body,
        uint32 msg_size) {
    MMIDM_SESSION_T dm_session_info = { 0 };
    BOOLEAN result = FALSE;
    DM_TRACE("MMIDM==> MMIDM_StartDm type=%d",type);

//    CreateDmDirectory();

//    CreateDmTreeXmlFile(FALSE);

//    MMIDM_CreateTask();

    dm_session_info.message_buffer = (char*) malloc(msg_size + 1);
    if (dm_session_info.message_buffer != NULL ) {
        memset(dm_session_info.message_buffer, 0, msg_size + 1);
        memcpy(dm_session_info.message_buffer, msg_body, msg_size);
    }
    //dm_session_info.message_buffer = msg_body;
    dm_session_info.msg_size = msg_size;
    dm_session_info.type = type;
    s_dm_info.session_type = type;

    if (!MMIDM_IsDmRun()) {
        DM_TRACE("MMIDM==> MMIDM_StartDm dm is not running, start dm");
        s_is_dm_run = TRUE;
        DMTaskStart(&dm_session_info);
//        MMIDM_CreateDmRunCheckTimer();
//        result = MMIDM_SendSigToDmTask(DM_TASK_START_MESSAGE,MMIDM_GetDmTaskID(),&dm_session_info);
    } else {
        DM_TRACE("MMIDM==> MMIDM_StartDm dm is running, exit dm");
//        MMIDM_CloseWaitingWin();
//        MMIDM_SendSigToDmTask(DM_TASK_DM_EXIT,MMIDM_GetDmTaskID(),PNULL);
//        MMIPUB_OpenAlertWinByTextId(PNULL,TXT_DM_SERVICE_RUN,TXT_NULL,IMAGE_PUBWIN_WARNING,PNULL,PNULL,MMIPUB_SOFTKEY_ONE,PNULL);
        MMIDM_ExitDM();
    }
    if (!result) {
        s_dm_info.session_type = DM_SESSION_NONE;
    }
    return result;
}
/*****************************************************************************/
//  Description : exit dm session
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
BOOLEAN MMIDM_ExitDM(void) {
    //MMIDM_SessionCancel();
    dmTree_save(); //save tree
    MMIDM_SetDmState(DM_NONE);
    // if (MMIDM_PDP_STATUS_NONE !=JgetPppConnectStatus())
    {
        JstopPppConnect();
    }
    return TRUE;
}

/*****************************************************************************/
//  Discription: This function is to get dm state
//  Global resource dependence: None
//  Note :
//  Return:
/*****************************************************************************/
MMIDM_DM_STATE MMIDM_GetDmState(void) {
    DM_TRACE("MMIDM==> get dm state = %d",s_dm_info.dm_state);
    return (s_dm_info.dm_state);
}

/*****************************************************************************/
//  Discription: This function is to set dm state
//  Global resource dependence: None
//  Note :
//  Return:
/*****************************************************************************/
void MMIDM_SetDmState(MMIDM_DM_STATE state) {
    DM_TRACE("MMIDM==> set dm state = %d",state);
    s_dm_info.dm_state = state;
}

/*****************************************************************************/
//  Description : get dl state
//  Global resource dependence :
//  Note:
/*****************************************************************************/
MMIDM_DM_STATE MMIDM_GetDlState(void) {
    DM_TRACE("MMIDM==> MMIDM_GetDlState state = %d ",s_dm_info.dl_state);
    return s_dm_info.dl_state;
}

/*****************************************************************************/
//  Description : set dl state
//  Global resource dependence :
//  Note:
/*****************************************************************************/
void MMIDM_SetDlState(MMIDM_DM_STATE state) {
    DM_TRACE("MMIDM==> MMIDM_SetDlState state = %d ",state);
    s_dm_info.dl_state = state;
}

/*****************************************************************************/
//  Discription: This function is dm run
//  Global resource dependence: None
//  Note :
//  Return:
/*****************************************************************************/
BOOLEAN MMIDM_IsDmRun(void) {
    DM_TRACE("MMIDM==> MMIDM_IsDmRun = %d",s_dm_info.dm_state);
    if ((s_dm_info.dm_state == DM_START) || (s_dm_info.dm_state == DM_RUN))
        return TRUE;
    else
        return FALSE;
    //return (s_is_dm_run);
}

char* MMIDM_GetDmProfile(void) {

    char* string = PNULL;
    string = MMIDM_GetCBFunc(DM_APN_IO_HANDLER, 0);
    DM_TRACE("MMIDM==> MMIDM_GetDmProfile %s", string);
    return string;
    //DM_TRACE("MMIDM==>MMIDM_GetDmProfile");
    //return s_dm_info.apn_info;

}

/*****************************************************************************/
//  Description : get srvadd URL
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
char* MMIDM_GetSrvAddURL(void) {

    char* string = PNULL;
    string = MMIDM_GetCBFunc(SERVER_ADDR_IO_HANDLER, 0);
    DM_TRACE("MMIDM==> MMIDM_GetSrvAddURL %s", string);
    return string;
    //DM_TRACE("MMIDM==> MMIDM_GetSrvAddURL srv_addr = %s",s_dm_info.srv_addr);
    //return s_dm_info.srv_addr;
}

/*****************************************************************************/
// Description : This function retrieves the device Manufacturer from
//          Non-volatile memory.
// return String that holds the name of the Manufacturer.
// Note:
/*****************************************************************************/
char *MMIDM_GetMan(void) {
    char* string = PNULL;
    string = MMIDM_GetCBFunc(MAN_IO_HANDLER, 0);
    DM_TRACE("MMIDM==> MMIDM_GetMan %s", string);
    return string;
    // DM_TRACE("MMIDM==> MMIDM_GetMan %s", s_server_type_tab[s_dm_info.server_type].man);
    // return s_server_type_tab[s_dm_info.server_type].man;
}

/*****************************************************************************/
// Description : This function retrieves the device Model from Non-volatile memory.
// return String that holds the name of the device Model.
// Note:
/*****************************************************************************/
char *MMIDM_GetModel(void) {
    char* string = PNULL;
    string = MMIDM_GetCBFunc(MODEL_IO_HANDLER, 0);
    DM_TRACE("MMIDM==> MMIDM_GetModel %s", string);
    return string;
    //DM_TRACE("MMIDM==> MMIDM_GetModel %s", s_server_type_tab[s_dm_info.server_type].mode);
    //return s_server_type_tab[s_dm_info.server_type].mode;
}

/*****************************************************************************/
//  Description : get device imei str
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
char* DM_GetDevImeiInfo(void) {

    char* string = PNULL;
    string = MMIDM_GetCBFunc(DEVID_IO_HANDLER, 0);
    DM_TRACE("MMIDM==>DM_GetDevImeiInfo%s", string);
    return string;
    // return(s_server_type_tab[s_dm_info.server_type].imei);

}

/*****************************************************************************/
//  Description : get server nonce
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
char* MMIDM_GetServerNonce() {
//    MN_RETURN_RESULT_E return_value = MN_RETURN_FAILURE;

    //init server nonce
//    MMINV_READ(MMINV_DM_SERVER_NONCE, s_dm_info.server_nonce, return_value);
//    if(MN_RETURN_SUCCESS != return_value)
    char *nonce = JMMIDM_GetServerNonce();

    if (nonce == NULL || strcmp(nonce, NULL_NONCE) == 0) {
        DMTREE_ACC_INFO* acc_info = dmTreeParam_getDmAccInfo();
        DM_TRACE("MMIDM==>DM_source accServerNonce = %s", acc_info->accServerNonce);
        SCI_MEMCPY(s_dm_info.server_nonce, acc_info->accServerNonce,
                sizeof(acc_info->accServerNonce));
        DM_TRACE("s_dm_info.server_nonce=  %s", s_dm_info.server_nonce);
        // MMINV_WRITE(MMINV_DM_SERVER_NONCE, s_dm_info.server_nonce);
        JMMIDM_SetServerNonce(s_dm_info.server_nonce);
    } else {
        strlcpy(s_dm_info.server_nonce, nonce, sizeof(s_dm_info.server_nonce));
    }
    return s_dm_info.server_nonce;
}

/*****************************************************************************/
//  Description : get client nonce
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
char* MMIDM_GetClientNonce() {
//    MN_RETURN_RESULT_E return_value = MN_RETURN_FAILURE;

    //init client nonce
//    MMINV_READ(MMINV_DM_CLIENT_NONCE, s_dm_info.server_nonce, return_value);
//    if(MN_RETURN_SUCCESS != return_value)
    char *nonce = JMMIDM_GetClientNonce();

    if (nonce == NULL || strcmp(nonce, NULL_NONCE) == 0) {
        DMTREE_ACC_INFO* acc_info = dmTreeParam_getDmAccInfo();
        DM_TRACE("MMIDM==>DM_source accClientNonce = %s", acc_info->accClientNonce);
        SCI_MEMCPY(s_dm_info.client_nonce, acc_info->accClientNonce,
                sizeof(acc_info->accClientNonce));

        DM_TRACE("MMIDM==>DM_s_dm_info.client_nonce = %s", s_dm_info.client_nonce);
//        MMINV_WRITE(MMINV_DM_CLIENT_NONCE, s_dm_info.client_nonce);
        JMMIDM_SetClientNonce(s_dm_info.client_nonce);
    } else {
        strlcpy(s_dm_info.client_nonce, nonce, sizeof(s_dm_info.client_nonce));
    }
    return s_dm_info.client_nonce;
}
