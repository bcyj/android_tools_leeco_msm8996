/*!
  @file
  atfwd_daemon.c

  @brief
  ATFWD daemon which registers with QMI ATCOP service and forwards AT commands

*/

/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.


when       who      what, where, why
--------   ---      ---------------------------------------------------------
04/11/11  jaimel  ATFWD-daemon to register and forward AT commands to Apps
04/19/11  c_spotha Added default port changes

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#define LOG_NDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0
#define LOG_TAG "Atfwd_Daemon"
#include <utils/Log.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include "sendcmd.h"
#include <qmi_atcop_srvc.h>
#include <qmi.h>
#include <string.h>
#include <cutils/properties.h>
#include "common_log.h"
#include "AtCmdFwd.h"
#include "mdm_detect.h"

#define MAX_DIGITS 10
#define DEFAULT_QMI_PORT QMI_PORT_RMNET_SDIO_0
#define DEFAULT_SMD_PORT QMI_PORT_RMNET_0

#define ATFWD_DATA_PROP_BASEBAND            "ro.baseband"
#define ATFWD_DATA_PROP_BOARD_PLATFORM      "ro.board.platform"

#define ATFWD_DATA_TARGET_MSM         "msm"
#define ATFWD_DATA_TARGET_MSM8994     "msm8994"
#define ATFWD_DATA_TARGET_MSM8916     "msm8916"
#define ATFWD_DATA_TARGET_MSM8936     "msm8936"
#define ATFWD_DATA_TARGET_MSM8939     "msm8939"
#define ATFWD_DATA_TARGET_MSM8909     "msm8909"
#define ATFWD_DATA_TARGET_SVLTE1      "svlte1"
#define ATFWD_DATA_TARGET_SVLTE2A     "svlte2a"
#define ATFWD_DATA_TARGET_CSFB        "csfb"
#define ATFWD_DATA_TARGET_MDM         "mdm"
#define ATFWD_DATA_TARGET_MDM2        "mdm2" /* For Fusion 4.5 devices */
#define ATFWD_DATA_TARGET_SGLTE       "sglte"
#define ATFWD_DATA_TARGET_APQ         "apq" /* unused right now */

 /* For new MSM targets that use rmnet0 port - MSM8994, MSM8916, MSM8936,
    MSM8939, MSM8909 */
#define ATFWD_DATA_TARGET_MSM_NEW     "msm_new"

#define ATFWD_DATA_PROP_SIZE (PROPERTY_VALUE_MAX)
#define ATFWD_ATCOP_PORTS    2

typedef enum {
    INIT_QMI = 0,
    INIT_QMI_SRVC,
    INIT_ATFWD_SRVC,
    INIT_MAX
} atfwd_init_type_t;

/*qmi message library handle*/
int qmiHandle = QMI_INVALID_CLIENT_HANDLE;

/*===========================================================================

                           Global Variables

===========================================================================*/

int userData; //Extra user data sent by QMI
int qmiErrorCode; //Stores the QMI error codes
int userHandle; //Connection ID
int userHandleSMD; //Conn ID for SVLTE II , USB --> SMD --> 8k modem
qmi_atcop_abort_type abortType; //AT command abort type
const char *qmiPort = NULL;
const char *secondaryPort = NULL;
qmi_atcop_at_cmd_fwd_req_type atCmdFwdReqType[] = {
    { //AT command fwd type
        1, // Number of commands
        {
            { QMI_ATCOP_AT_CMD_NOT_ABORTABLE, "+CKPD"},
        }
    },
    { //AT command fwd type
        1, // Number of commands
        {
            { QMI_ATCOP_AT_CMD_NOT_ABORTABLE, "+CTSA"},
        }
    },
    { //AT command fwd type
        1, // Number of commands
        {
            { QMI_ATCOP_AT_CMD_NOT_ABORTABLE, "+CFUN"},
        }
    },
    { //AT command fwd type
        1, // Number of commands
        {
            { QMI_ATCOP_AT_CMD_NOT_ABORTABLE, "+CMAR"},
        }
    },
    { //AT command fwd type
        1, // Number of commands
        {
            { QMI_ATCOP_AT_CMD_NOT_ABORTABLE, "+CDIS"},
        }
    },
    { //AT command fwd type
        1, // Number of commands
        {
            { QMI_ATCOP_AT_CMD_NOT_ABORTABLE, "+CRSL"},
        }
    },
    { //AT command fwd type
        1, // Number of commands
        {
            { QMI_ATCOP_AT_CMD_NOT_ABORTABLE, "+CSS"},
        }
    },
    { //AT command fwd type
        1, // Number of commands
        {
            { QMI_ATCOP_AT_CMD_NOT_ABORTABLE, "$QCPWRDN"},
        }
    },
};

qmi_atcop_at_cmd_hndl_type commandHandle; //ATFWD request handle
qmi_atcop_at_cmd_fwd_ind_type request; //Input request string

//qmi_atcop_fwd_resp_status_type responseStatus; //ATFWD response status
qmi_atcop_fwd_resp_result_type responseResult; //ATFWD response result
qmi_atcop_fwd_resp_response_type responseType; //ATFWD response type
qmi_atcop_fwd_resp_at_resp_type atCmdResponse; //Actual ATFWD response

AtCmd fwdcmd;
AtCmdResponse fwdresponse;

pthread_cond_t ctrCond; //Condition variable that will be used to indicate if a request has arrived
pthread_mutex_t ctrMutex; //Mutex that will be locked when a request is processed
int newRequest = 0; //New request indication
int regForPrimaryPort = 0; //Re registration indication for primary qmi port
int regForSecondaryPort = 0; //Re registration indication for secondary qmi port
char target[ATFWD_DATA_PROP_SIZE] = {0}; // Stores target info

typedef struct atfwd_sig_handler_s
{
  int sig;
  sighandler_t handler;
} atfwd_sig_handler_t;

/* All termination SIGNALS except SIGKILL
 * SIGKILL cannot be handled or ignored
*/
atfwd_sig_handler_t atfwd_sig_handler_tbl[] =
{
  {SIGTERM, NULL},
  {SIGINT, NULL},
  {SIGQUIT, NULL},
  {SIGHUP, NULL}
};

/*===========================================================================
  FUNCTION  parseInput
===========================================================================*/
/*!
@brief
  Parses the input request string and populates the AtCmd struct with the
  data to forward

@return
  None

*/
/*=========================================================================*/
void parseInput()
{
    int i;
    fwdcmd.opcode = request.op_code;
    fwdcmd.name = strdup((char *)request.at_name);
    fwdcmd.ntokens = request.num_tokens;
    fwdcmd.tokens = calloc(request.num_tokens, sizeof(char *));
    if(NULL != fwdcmd.tokens) {
        for (i = 0; i < request.num_tokens; i++) {
            fwdcmd.tokens[i] = strdup((char *)request.tokens[i]);
        }
    }
}

/*===========================================================================
  FUNCTION  exitDaemon
===========================================================================*/
/*!
@brief
  Utility method which handles when user presses CTRL+C

@return
  None

@note
  None
*/
/*=========================================================================*/
void exitDaemon(int sig)
{
    LOGI("Going to kill ATFWD daemon\n");
    (void)sig;
    unsigned int i=0;
    /* Note that the handler should ignore all the reg. signals
     * because they do not want to be interfered
     * while an ongoing signal is being processed
     */
    for(i=0; i<sizeof(atfwd_sig_handler_tbl)/sizeof(atfwd_sig_handler_t); i++) {
        signal(atfwd_sig_handler_tbl[i].sig, SIG_IGN);
    }
    int clientRelease = qmi_atcop_srvc_release_client (userHandle, &qmiErrorCode);
    if (clientRelease < 0) {
        LOGI("QMI client release error: %d\n", qmiErrorCode);
    }
    if (qmiHandle >= 0) {
        qmi_release(qmiHandle);
    }
    pthread_cond_destroy(&ctrCond);
    pthread_mutex_destroy(&ctrMutex);

    for(i=0; i<sizeof(atfwd_sig_handler_tbl)/sizeof(atfwd_sig_handler_t); i++) {
        LOGI("\natfwd_sig_handler_tbl[i].sig : %d", atfwd_sig_handler_tbl[i].sig);
        if (atfwd_sig_handler_tbl[i].sig == sig &&
            atfwd_sig_handler_tbl[i].handler != NULL) {
            /* call  default installed handler */
            LOGI("\ncall default handler [%p] for sig [%d]",
                  atfwd_sig_handler_tbl[i].handler,
                  atfwd_sig_handler_tbl[i].sig);
            (atfwd_sig_handler_tbl[i].handler)(sig);
            break;
        }
    }
    exit(0);
}

/*===========================================================================
  FUNCTION:  signalInit
===========================================================================*/
/*!
    @brief
    Signal specific initialization

    @return
    void
*/
/*=========================================================================*/
void signalInit(void)
{
    unsigned int i=0;
    sighandler_t temp;

    for(i=0; i<sizeof(atfwd_sig_handler_tbl)/sizeof(atfwd_sig_handler_t); i++) {
        temp = atfwd_sig_handler_tbl[i].handler;
        atfwd_sig_handler_tbl[i].handler = signal(atfwd_sig_handler_tbl[i].sig,
                                                  exitDaemon);
        /* swap previous handler back if signal() was unsuccessful */
        if (SIG_ERR == atfwd_sig_handler_tbl[i].handler) {
            atfwd_sig_handler_tbl[i].handler = temp;
        }
    }
}

/*===========================================================================
  FUNCTION  sendSuccessResponse
===========================================================================*/
/*!
@brief
  Sends OK response to QMI.

@return
  None

@note
  None
*/
/*=========================================================================*/
void sendSuccessResponse()
{
    //responseStatus = QMI_ATCOP_SUCCESS;
    responseResult = QMI_ATCOP_RESULT_OK;
    responseType = QMI_ATCOP_RESP_COMPLETE;
    atCmdResponse.at_hndl = commandHandle;
    //atCmdResponse.status = responseStatus;
    atCmdResponse.result = responseResult;
    atCmdResponse.response = responseType;
    atCmdResponse.at_resp = NULL;
    if (qmi_atcop_fwd_at_cmd_resp(userHandle, &atCmdResponse, &qmiErrorCode) < 0) {
        LOGI("QMI response error: %d\n", qmiErrorCode);
    }
}

/*===========================================================================
  FUNCTION  sendResponse
===========================================================================*/
/*!
@brief
  Sends response to QMI.

@return
  None

@note
  None
*/
/*=========================================================================*/
void sendResponse(AtCmdResponse *response)
{
    if (!response) {
        LOGI("Have null response");
        return;
    }
    //responseStatus = QMI_ATCOP_SUCCESS;
    responseResult = response->result;
    responseType = QMI_ATCOP_RESP_COMPLETE;
    atCmdResponse.at_hndl = commandHandle;
    //atCmdResponse.status = responseStatus;
    atCmdResponse.result = responseResult;
    atCmdResponse.response = responseType;
    if ((response->result != QMI_ATCOP_RESULT_OK) && (request.cmee_val == 0)) {
        atCmdResponse.at_resp = NULL;
    } else {
        char *msg = NULL;
        unsigned long s3 = request.s3_val;
        unsigned long s4 = request.s4_val;
        if (response->response && response->response[0]) {
            // Need space for the end of line and carriage return chars (S3/S4)
            size_t l = 4*4 + strlen(response->response) + 1;
            msg = malloc(l);
            if(NULL != msg)
                snprintf(msg, l, "%c%c%s%c%c", (char)s3, \
                         (char)s4, response->response, (char)s3, (char)s4);
        }
        atCmdResponse.at_resp = (unsigned char *)msg;
    }
    if (qmi_atcop_fwd_at_cmd_resp(userHandle, &atCmdResponse, &qmiErrorCode) < 0) {
        LOGI("QMI response error: %d\n", qmiErrorCode);
    }
    if(NULL != atCmdResponse.at_resp) {
        free(atCmdResponse.at_resp);
        atCmdResponse.at_resp = NULL;
    }
}

/*===========================================================================
  FUNCTION  sendInvalidCommandResponse
===========================================================================*/
/*!
@brief
  Sends ERROR response to QMI.

@return
  None

@note
  None
*/
/*=========================================================================*/
void sendInvalidCommandResponse()
{
    responseResult = QMI_ATCOP_RESULT_ERROR;
    responseType   = QMI_ATCOP_RESP_COMPLETE;

    atCmdResponse.at_hndl  = commandHandle;
    atCmdResponse.result   = responseResult;
    atCmdResponse.response = responseType;

    if (request.cmee_val == 0) {
        atCmdResponse.at_resp = NULL;
    } else {
        char *response;
        char s3Val[MAX_DIGITS];
        char s4Val[MAX_DIGITS];

        snprintf(s3Val, MAX_DIGITS, "%c", (char) request.s3_val);
        snprintf(s4Val, MAX_DIGITS, "%c", (char) request.s4_val);

        size_t respLen  = ((strlen(s3Val) * 2) + (strlen(s4Val) * 2) + 13 + 1) * sizeof(char);
        response = (char *)malloc(respLen);
        if (!response) {
            LOGI("No memory for generating invalid command response\n");
            atCmdResponse.at_resp = NULL;
        } else {
            snprintf(response, respLen, "%s%s+CME ERROR :2%s%s", s3Val, s4Val, s3Val, s3Val);
            atCmdResponse.at_resp = (unsigned char *)response;
        }
    }

    if (qmi_atcop_fwd_at_cmd_resp(userHandle, &atCmdResponse, &qmiErrorCode) < 0) {
        LOGI("QMI response error: %d\n", qmiErrorCode);
    }

    if (atCmdResponse.at_resp) {
        free(atCmdResponse.at_resp);
        atCmdResponse.at_resp = NULL;
    }
}

/*===========================================================================
  FUNCTION  sendCommand
===========================================================================*/
/*!
@brief
  Routine that will be invoked by QMI upon a request for AT command. It
  checks for the validity of the request and finally spawns a new thread to
  process the key press events.

@return
  None

@note
  None

*/
/*=========================================================================*/
static void atCommandCb(int userHandle, qmi_service_id_type serviceID,
                             void *userData, qmi_atcop_indication_id_type indicationID,
                             qmi_atcop_indication_data_type  *indicationData)
{
    LOGI("atCommandCb\n");

    /* Check if it's an abort request */
    if (indicationID == QMI_ATCOP_SRVC_ABORT_MSG_IND_TYPE) {
        LOGI("Received abort message from QMI\n");
    } else if (indicationID == QMI_ATCOP_SRVC_AT_FWD_MSG_IND_TYPE) {
        LOGI("Received AT command forward request\n");
        pthread_mutex_lock(&ctrMutex);
        commandHandle = indicationData->at_hndl;
        request = indicationData->at_cmd_fwd_type;
        parseInput();
        newRequest = 1;
        pthread_cond_signal(&ctrCond);
        pthread_mutex_unlock(&ctrMutex);
    }
}

char* getTargetFromSysProperty()
{
    int ret = 0;
    struct dev_info devInfo = {0,};
    char msm_tgt[ATFWD_DATA_PROP_SIZE] = {0};

    /* get baseband property. in error case, def will be used */
    ret = property_get(ATFWD_DATA_PROP_BASEBAND, target, "");

    /* For Fusion targets, apply ESOC logic to derive the target */
    if (!strcmp(ATFWD_DATA_TARGET_MDM, target)
            || !strcmp(ATFWD_DATA_TARGET_MDM2, target)) {
        ret = get_system_info(&devInfo);
        if (ret != 0) {
            LOGI("Could not retrieve ESOC info. get_system_info returns (%d)\n", ret);
        } else {
            LOGI("link name %s ", devInfo.mdm_list[0].mdm_link);
            if (!strcmp("HSIC", devInfo.mdm_list[0].mdm_link)) {
                strlcpy(target, ATFWD_DATA_TARGET_MDM, ATFWD_DATA_PROP_SIZE);
            } else {
                strlcpy(target, ATFWD_DATA_TARGET_MDM2, ATFWD_DATA_PROP_SIZE);
            }
        }
    } else if (!strcmp(ATFWD_DATA_TARGET_MSM, target)) {
        /* get the board platform to check if it is msm8994 */
        ret = property_get(ATFWD_DATA_PROP_BOARD_PLATFORM, msm_tgt, "");
        if (!strcmp(ATFWD_DATA_TARGET_MSM8994, msm_tgt) ||
                !strcmp(ATFWD_DATA_TARGET_MSM8916, msm_tgt) ||
                !strcmp(ATFWD_DATA_TARGET_MSM8936, msm_tgt) ||
                !strcmp(ATFWD_DATA_TARGET_MSM8939, msm_tgt) ||
                !strcmp(ATFWD_DATA_TARGET_MSM8909, msm_tgt)) {
            strlcpy(target, ATFWD_DATA_TARGET_MSM_NEW, ATFWD_DATA_PROP_SIZE);
        }
    }

    return target;
}

char* getDefaultPort()
{
    char *defaultQmiPort = NULL;

    if (!strncmp(ATFWD_DATA_TARGET_MSM, target,strlen(target)) ||
        !strncmp(ATFWD_DATA_TARGET_SGLTE, target, strlen(target))) {
        /* use smd port */
        defaultQmiPort = QMI_PORT_RMNET_1;
    } else if (!strncmp(ATFWD_DATA_TARGET_MSM_NEW, target, strlen(target))) {
        /* 8994, 8916, 8936, 8939 & 8909 would use first conn id */
        defaultQmiPort = QMI_PORT_RMNET_0;
    } else if (!strncmp(ATFWD_DATA_TARGET_SVLTE1, target, strlen(target)) ||
               !strncmp(ATFWD_DATA_TARGET_SVLTE2A, target, strlen(target)) ||
               !strncmp(ATFWD_DATA_TARGET_CSFB, target, strlen(target))) {
        /* use sdio port */
        defaultQmiPort = QMI_PORT_RMNET_SDIO_0;
    } else if (!strncmp(ATFWD_DATA_TARGET_MDM, target, strlen(target))) {
        /* use usb port */
        defaultQmiPort = QMI_PORT_RMNET_USB_0;
    } else if (!strncmp(ATFWD_DATA_TARGET_MDM2, target, strlen(target))) {
        /* use MHI port for fusion 4.5 devices */
        defaultQmiPort = QMI_PORT_RMNET_MHI_0;
    } else {
        /* do not set defaultQmiPort for any thing else right now
         * as we don't know */
        LOGI("defaultQmiPort left as-is to %s", defaultQmiPort);
    }

    return defaultQmiPort;
}

void initAtCopServiceByPort(const char *port, int *handle) {
    int retryCnt;
    for (retryCnt = 1; retryCnt <= ATFWD_MAX_RETRY_ATTEMPTS; ++retryCnt) {
        *handle = qmi_atcop_srvc_init_client(port, atCommandCb, NULL , &qmiErrorCode);
        LOGI("qmi_atcop_srvc_init_client - QMI Err code %d , handle %d", qmiErrorCode, *handle);
        if (*handle < 0 || qmiErrorCode != 0) {
            LOGI("Could not register with the QMI Interface on port %s. " \
                    "The QMI error code is %d", port, qmiErrorCode);
            // retry after yielding..
            sleep(retryCnt * ATFWD_RETRY_DELAY);
            continue;
        }
        break;
    }
    return;
}

void stopSelf() {
    LOGI("Stop the daemon....");
    property_set("ctl.stop", "atfwd");
    return;
}

void initAtcopServiceAndRegisterCommands(const char *port, int *handle) {
    int i, nErrorCnt, nCommands;
    i = nErrorCnt = 0;

    initAtCopServiceByPort(port, handle);

    if (*handle > 0) {
        nCommands = sizeof(atCmdFwdReqType) / sizeof(atCmdFwdReqType[0]);
        LOGI("Trying to register %d commands:\n", nCommands);
        for (i = 0; i < nCommands ; i++) {
            LOGI("cmd%d: %s\n", i, atCmdFwdReqType[i].qmi_atcop_at_cmd_fwd_req_type[0].at_cmd_name);

            qmiErrorCode = 0;
            int registrationStatus = qmi_atcop_reg_at_command_fwd_req(*handle, \
                                              &atCmdFwdReqType[i], &qmiErrorCode);
            LOGI("qmi_atcop_reg_at_command_fwd_req: %d", qmiErrorCode);
            if (registrationStatus < 0 || qmiErrorCode != 0) {
                LOGI("Could not register AT command : %s with the QMI Interface - Err code:%d\n",
                     atCmdFwdReqType[i].qmi_atcop_at_cmd_fwd_req_type[0].at_cmd_name, qmiErrorCode);
                nErrorCnt++;
                qmiErrorCode = 0;
            }
        }

        if(nErrorCnt == nCommands) {
            LOGI("AT commands registration failure... Release client handle: %d\n", *handle);
            qmi_atcop_srvc_release_client(*handle, &qmiErrorCode);
            *handle = -1;
            return;
        }
    } else {
        LOGI("ATcop Service Init failed\n");
        return;
    }

    LOGI("Registered AT Commands event handler\n");
    return;
}

static void atfwdSysEventHandler (qmi_sys_event_type event_id,
        const qmi_sys_event_info_type *event_info, void *user_data) {

    int ret;
    const char* devId;

    if (event_info && event_info->qmi_modem_service_ind.dev_id) {
        devId = event_info->qmi_modem_service_ind.dev_id;
        LOGI("Received sys_event: %d from QMI devId: %s\n", (int) event_id, devId);
        if ((qmiPort && !strcmp(qmiPort, devId))
                || (secondaryPort && !strcmp(secondaryPort, devId))) {
            switch (event_id) {
                case QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND:
                    LOGI("Modem Out Of Service --> Release ATCOP service client handles, if any\n");
                    if (qmiPort && !strcmp(qmiPort, devId) && (userHandle > 0)) {
                        qmiErrorCode = 0;
                        ret = qmi_atcop_srvc_release_client(userHandle, &qmiErrorCode);
                        LOGI("release_client returns %d, qmiErroCode = %d for qmiPort: %s & "
                                "userHandle: %d\n", ret, qmiErrorCode, qmiPort, userHandle);
                        userHandle = -1;
                    }
                    if (secondaryPort && !strcmp(secondaryPort, devId) && (userHandleSMD > 0)) {
                        qmiErrorCode = 0;
                        ret = qmi_atcop_srvc_release_client(userHandleSMD, &qmiErrorCode);
                        LOGI("release_client returns %d, qmiErroCode = %d for secondaryPort: %s & "
                            "userHandleSMD: %d\n", ret, qmiErrorCode, secondaryPort, userHandleSMD);
                        userHandleSMD = -1;
                    }
                    break;

                case QMI_SYS_EVENT_MODEM_IN_SERVICE_IND:
                    LOGI("Modem In Service --> Notify Main loop to init ATCOP service client\n");
                    if (qmiPort && !strcmp(qmiPort, devId)) {
                        pthread_mutex_lock(&ctrMutex);
                        regForPrimaryPort = 1;
                        pthread_cond_signal(&ctrCond);
                        pthread_mutex_unlock(&ctrMutex);
                    }
                    if (secondaryPort && !strcmp(secondaryPort, devId)) {
                        pthread_mutex_lock(&ctrMutex);
                        regForSecondaryPort = 1;
                        pthread_cond_signal(&ctrCond);
                        pthread_mutex_unlock(&ctrMutex);
                    }

                    break;

                default:
                    LOGI("Unhandled SYS event (%d) from QMI....\n", (int) event_id);
                    break;
            }
        }
    } else {
        LOGI("Ignoring sys_event: %d from QMI with NO event_info/dev_id\n", (int) event_id);
    }
}

void tryInit (atfwd_init_type_t type, int *result) {
    int retryCnt = 1;

    for (; retryCnt <= ATFWD_MAX_RETRY_ATTEMPTS; retryCnt++) {
        qmiErrorCode = 0;
        switch (type) {
            case INIT_QMI:
                qmiHandle = qmi_init(atfwdSysEventHandler, NULL);
                *result = qmiHandle;
                break;
            case INIT_QMI_SRVC:
                *result = qmi_connection_init(qmiPort, &qmiErrorCode);
                break;
            case INIT_ATFWD_SRVC:
                *result = initializeAtFwdService();
                break;
            default:
                LOGI("Invalid type %d", type);
                return;
        }
        LOGI("result : %d \t ,Init step :%d \t ,qmiErrorCode: %d", *result, type, qmiErrorCode);
        if (*result >= 0 && qmiErrorCode == 0) {
            break;
        }
        sleep(retryCnt * ATFWD_RETRY_DELAY);
    }

    return;
}

int isNewCommandAvailable() {
    if (newRequest || regForPrimaryPort || regForSecondaryPort) {
        return 1;
    }
    return 0;
}
/*=========================================================================
  FUNCTION:  main

===========================================================================*/
/*!
@brief
  Initialize the QMI connection and register the ATFWD event listener.
  argv[1] if provided, gives the name of the qmi port to open.
  Default is "rmnet_sdio0".

*/
/*=========================================================================*/
int main (int argc, char **argv)
{
    AtCmdResponse *response;
    int i, connectionResult, initType;

    userHandle = userHandleSMD = -1;
    i = connectionResult = 0;

    (void) getTargetFromSysProperty();

    if (!strncmp(ATFWD_DATA_TARGET_APQ, target,
                 strlen(target))) {
        LOGI("APQ baseband : Explicitly stopping ATFWD service....\n");
        stopSelf();
        return -1;
    }

    if (argc >= 2) {
        qmiPort = argv[1];
    } else {
        qmiPort = getDefaultPort();
        if( NULL == qmiPort ) {
            qmiPort = DEFAULT_QMI_PORT;
        }
    }

    if (argc >= 3) {
        secondaryPort = argv[2];
    } else if (!strncmp(ATFWD_DATA_TARGET_SVLTE2A, target, strlen(target))) {
       /* For SVLTE type II targets, Modem currently exposes two ATCOP ports.
        * One bridged from USB to SDIO, directly talking to 9k modem
        * Another bridged from USB to SMD, directly talking to 8k
        * Therefore given this modem architecture, ATFWD-daemon needs to
        * listen to both the modems( 8k & 9K).
        * Register with 8k modem
        */
        secondaryPort = DEFAULT_SMD_PORT;
    } else if (!strncmp(ATFWD_DATA_TARGET_SGLTE, target, strlen(target))) {
        // For SGLTE targets, Register with the SMUX port.
        secondaryPort = QMI_PORT_RMNET_SMUX_0;
    }

    signalInit();

    pthread_mutexattr_t attr;
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&ctrMutex, &attr);
    pthread_cond_init(&ctrCond, NULL);

    for (initType = INIT_QMI; initType != INIT_MAX; initType++) {
        connectionResult = 0;
        tryInit (initType, &connectionResult);
        if (connectionResult < 0) {
            if (qmiHandle >= 0) {
                qmi_release(qmiHandle);
            }
            stopSelf();
            return -1;
        }
    }

    if (qmiPort) {
        LOGI("ATFWD --> QMI Port : %s\n" , qmiPort);
        initAtcopServiceAndRegisterCommands(qmiPort, &userHandle);
    }

    if (secondaryPort) {
        LOGI("ATFWD --> secondaryPort : %s\n" , secondaryPort);
        initAtcopServiceAndRegisterCommands(secondaryPort, &userHandleSMD);
    }

    if(userHandle < 0 && userHandleSMD < 0) {
        LOGI("Could not register userhandle(s) with both 8k and 9k modems -- bail out");
        if (qmiHandle >= 0) {
            qmi_release(qmiHandle);
        }
        stopSelf();
        return -1;
    }

    while (1) {
        pthread_mutex_lock(&ctrMutex);
        while (!isNewCommandAvailable()) {
            pthread_cond_wait(&ctrCond, &ctrMutex);
        }

        if (regForPrimaryPort == 1) {
            if (qmiPort) {
                LOGI("Rcvd pthread notification for primary QMI port registration");
                initAtcopServiceAndRegisterCommands(qmiPort, &userHandle);
            } else {
                LOGI("Notification for primary QMI port registration when NOT valid, ignore...");
            }
            regForPrimaryPort = 0;
        }

        if (regForSecondaryPort == 1) {
            if (secondaryPort) {
                LOGI("Rcvd pthread notification for secondary QMI port registration");
                initAtcopServiceAndRegisterCommands(secondaryPort, &userHandleSMD);
            } else {
                LOGI("Notification for secondary QMI port registration when NOT valid, ignore...");
            }
            regForSecondaryPort = 0;
        }

        if(userHandle < 0 && userHandleSMD < 0) {
            LOGI("userhandle(s) for both 8k and 9k modems NOT valid -- bail out");
            if (qmiHandle >= 0) {
                qmi_release(qmiHandle);
            }
            stopSelf();
            return -1;
        }

        if (newRequest == 1) {
            LOGI("pthread notified for new request\n");
            response = sendit(&fwdcmd);
            if (response == NULL) {
                sendInvalidCommandResponse();
            } else {
                sendResponse(response);
            }

            if (fwdcmd.name) free(fwdcmd.name);
            if (fwdcmd.tokens) {
                for (i = 0; i < fwdcmd.ntokens; i++) {
                    free(fwdcmd.tokens[i]);
                }
                free(fwdcmd.tokens);
            }
            freeAtCmdResponse(response);
            newRequest = 0;
        }

        pthread_mutex_unlock(&ctrMutex);
    }

    return 0;
}
