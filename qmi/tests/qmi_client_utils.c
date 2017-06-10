/******************************************************************************
  @file    qmi_client_utils.c
  @brief   The QMI Client Utils for telephony adaptation or test code.

  DESCRIPTION
  QMI message send / receive high level utilities  

  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdarg.h>             //va_list
#ifdef FEATURE_QMI_ANDROID
#include <cutils/properties.h>
#else
#include <syslog.h>
#endif

#include "qmi_client_utils.h"

#include "common_v01.h" 
#include "voice_service_v02.h"
#include "network_access_service_v01.h"
#include "wireless_messaging_service_v01.h"
#include "device_management_service_v01.h"   
#include "wireless_data_service_v01.h" 
#include "phonebook_manager_service_v01.h" 
#include "user_identity_module_v01.h"
#include "specific_absorption_rate_v01.h" 

#define ASSERT(a)
#define QMI_UTIL_SVC_CONTROL_EVT_QUIT (1)  
#define QMI_UTIL_SVC_CONTROL_EVT_INDICATION_REPOST (2)
#define QMI_UTIL_SVC_CONTROL_EVT_EXECUTE_QMI_REQ (3)

#define MAX_QMIPORT_NAME 16
static int qmi_client_util_is_logging_enabled = 0;
static char qmi_client_util_port[MAX_QMIPORT_NAME];
extern pthread_mutex_t log_mutex;
extern FILE* qmi_simple_ril_suite_output_handle();

/*qmi message library handle*/
static int qmi_handle = QMI_INVALID_CLIENT_HANDLE;

int is_sig_pipe = 0;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct qmi_util_outstanding_request_info
{
    qmi_util_request_id         request_id;
    int                         service_id;
    int                         message_id;

    int                         message_specific_payload_len;
    void*                       message_specific_payload;

    int                         control_info;

    struct qmi_util_outstanding_request_info* next;
} qmi_util_outstanding_request_info;


typedef struct qmi_util_requester_private_info
{
    int is_initialized;
    qmi_util_service_message_handler service_message_handler;
    qmi_util_service_message_handler service_indication_handler;

    qmi_idl_service_object_type service_objects[QMI_UTIL_SVC_NOF_SVC];
    qmi_client_type             qmi_svc_clients[QMI_UTIL_SVC_NOF_SVC];
    qmi_service_version_info    qmi_svc_versions[QMI_UTIL_SVC_NOF_SVC];    

    qmi_util_request_id         last_generated_request_id;

    pthread_mutex_t             request_list_guard;
    pthread_mutexattr_t         mtx_atr;
    qmi_util_outstanding_request_info*
                                request_list;
    qmi_util_sync_object_info   downlink_event;
    pthread_t                   requester_thread;
} qmi_util_requester_private_info;

static qmi_util_requester_private_info requester_info;

static void qmi_util_event_pipe_do_get_event(qmi_util_event_pipe_info* event_pipe_info, qmi_util_event_info ** new_event);

//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
static void qmi_util_inication_repost(int service_id, int message_id, void * message_payload, int message_payload_len);
static void* qmi_util_requester_thread_func(void *param);
static void qmi_util_unsolicited_voice_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  );
static void qmi_util_unsolicited_nas_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  );
static void qmi_util_unsolicited_dms_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  );
static void qmi_util_unsolicited_wms_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  );
static void qmi_util_unsolicited_wds_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  );
static void qmi_util_unsolicited_uim_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  );
static void qmi_util_unsolicited_pb_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  );

static void qmi_util_unsolicited_rf_sar_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  );
static void qmi_util_record_time(struct timeval * recorded_time);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void qmi_util_sync_object_init(qmi_util_sync_object_info* sync_object_info)
    {
    ASSERT(sync_object_info != NULL);
    if (sync_object_info)
        {
        memset(sync_object_info, 0, sizeof(*sync_object_info));
        
        pthread_mutexattr_init(&sync_object_info->mtx_atr);        
        pthread_mutexattr_setpshared(&sync_object_info->mtx_atr, PTHREAD_PROCESS_SHARED);

        pthread_mutex_init(&sync_object_info->mutex, &sync_object_info->mtx_atr);

        pthread_cond_init(&sync_object_info->cond_var, NULL);
        }    
    }

void qmi_util_sync_object_destroy(qmi_util_sync_object_info* sync_object_info)
    {
    ASSERT(sync_object_info != NULL);
    if (sync_object_info)
        {
        pthread_cond_destroy(&sync_object_info->cond_var);
        pthread_mutexattr_destroy(&sync_object_info->mtx_atr);
        pthread_mutex_destroy(&sync_object_info->mutex);
        }
    }

void qmi_util_sync_object_wait(qmi_util_sync_object_info* sync_object_info)
    {
    ASSERT(sync_object_info != NULL);
    if (sync_object_info)
        {
        pthread_mutex_lock(&sync_object_info->mutex);
        pthread_cond_wait(&sync_object_info->cond_var, &sync_object_info->mutex);
        pthread_mutex_unlock(&sync_object_info->mutex);
        }
    }

void qmi_util_sync_object_signal(qmi_util_sync_object_info* sync_object_info)
    {
    ASSERT(sync_object_info != NULL);
    if (sync_object_info)
        {
        pthread_mutex_lock(&sync_object_info->mutex);
        pthread_cond_signal(&sync_object_info->cond_var);
        pthread_mutex_unlock(&sync_object_info->mutex);
        }
    }

#ifdef FEATURE_QMI_ANDROID
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int qmi_util_set_modem (char *modem)
    {
    if(strlen(modem) >= sizeof(qmi_client_util_port))
        {
        return 1;
        }
    
    if(0 == strcmp(modem, "local")) 
        {
        strncpy(qmi_client_util_port, QMI_PORT_RMNET_0, sizeof(qmi_client_util_port));
        }
    else if(0 == strcmp(modem, "remote")) 
        {
        strncpy(qmi_client_util_port, QMI_PORT_RMNET_SDIO_0, sizeof(qmi_client_util_port));
        }
    else if(0 == strcmp(modem, "default")) 
        {
        char baseband[PROPERTY_VALUE_MAX];
        property_get("ro.baseband", baseband, "msm");
        if ((strcmp(baseband, "csfb") == 0) || (strcmp(baseband,"svlte2a") == 0))
            {
            // Fusion 9K available via sdio0
            strncpy(qmi_client_util_port, QMI_PORT_RMNET_SDIO_0, sizeof(qmi_client_util_port));
            }
        else
            {
            // default modem available via smd0
            strncpy(qmi_client_util_port, QMI_PORT_RMNET_0, sizeof(qmi_client_util_port));
            }
        }
    else if(0 == strncmp(modem, "rmnet", strlen("rmnet"))) 
        {
        strncpy(qmi_client_util_port, modem, sizeof(qmi_client_util_port));
        }
    else 
        {
        return 1;
        }
    return 0;
    }
#endif

void qmi_util_set_modem_port (const char *modem_port)
{
    strncpy(qmi_client_util_port, modem_port, sizeof(qmi_client_util_port));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
int qmi_util_common_init(qmi_util_service_message_handler resp_handler, qmi_util_service_message_handler ind_handler)
    {
    int rs = 1;

    qmi_util_logln0("qmi_util_common_init entry");

    qmi_client_error_type client_err = 0;

    if (!requester_info.is_initialized)
        {
        requester_info.service_message_handler = resp_handler;
        requester_info.service_indication_handler = ind_handler;

        pthread_mutexattr_init(&requester_info.mtx_atr);
        pthread_mutexattr_setpshared(&requester_info.mtx_atr, PTHREAD_PROCESS_SHARED);

        pthread_mutex_init(&requester_info.request_list_guard, &requester_info.mtx_atr);

        qmi_util_sync_object_init(&requester_info.downlink_event);

        // ***
        qmi_handle = qmi_init(NULL, NULL);
        if (qmi_handle < 0)
        {
          qmi_util_logln0("qmi message library init failed. \n");
          return rs;
        }

        // *** service objects
        requester_info.service_objects[QMI_UTIL_SVC_VOICE] = voice_get_service_object_v02();
        requester_info.service_objects[QMI_UTIL_SVC_NAS] = nas_get_service_object_v01();
        requester_info.service_objects[QMI_UTIL_SVC_WMS] = wms_get_service_object_v01();
        requester_info.service_objects[QMI_UTIL_SVC_WDS] = wds_get_service_object_v01();
        requester_info.service_objects[QMI_UTIL_SVC_DMS] = dms_get_service_object_v01();
        requester_info.service_objects[QMI_UTIL_SVC_UIM] = uim_get_service_object_v01();
        requester_info.service_objects[QMI_UTIL_SVC_PB] =  pbm_get_service_object_v01();
        requester_info.service_objects[QMI_UTIL_SVC_RF_SAR] =  sar_get_service_object_v01();
        
        // todo: conclude

        // *** handlers
        do
            { 
            client_err = qmi_client_init(qmi_client_util_port,
                            requester_info.service_objects[QMI_UTIL_SVC_VOICE],
                            qmi_util_unsolicited_voice_ind_handler,
                            requester_info.service_objects[QMI_UTIL_SVC_VOICE],
                            &requester_info.qmi_svc_clients[QMI_UTIL_SVC_VOICE] );
            qmi_util_logln1("qmi_util_common_init VOICE svc client", client_err);
            if (client_err)
                break;
            client_err = qmi_client_get_service_version(qmi_client_util_port,requester_info.service_objects[QMI_UTIL_SVC_VOICE], &requester_info.qmi_svc_versions[QMI_UTIL_SVC_VOICE] );
            if (client_err)
                break;
            qmi_util_logln2("qmi_util_common_init VOICE versions", requester_info.qmi_svc_versions[QMI_UTIL_SVC_VOICE].major_ver, requester_info.qmi_svc_versions[QMI_UTIL_SVC_VOICE].minor_ver);
            
            client_err = qmi_client_init(qmi_client_util_port,
                            requester_info.service_objects[QMI_UTIL_SVC_NAS],
                            qmi_util_unsolicited_nas_ind_handler,
                            requester_info.service_objects[QMI_UTIL_SVC_NAS],
                            &requester_info.qmi_svc_clients[QMI_UTIL_SVC_NAS] );
            qmi_util_logln1("qmi_util_common_init NAS svc client", client_err);
            if (client_err)
                break;
            client_err = qmi_client_get_service_version(qmi_client_util_port,requester_info.service_objects[QMI_UTIL_SVC_NAS], &requester_info.qmi_svc_versions[QMI_UTIL_SVC_NAS] );
            if (client_err)
                break;
            qmi_util_logln2("qmi_util_common_init NAS versions", requester_info.qmi_svc_versions[QMI_UTIL_SVC_NAS].major_ver, requester_info.qmi_svc_versions[QMI_UTIL_SVC_NAS].minor_ver);

            client_err = qmi_client_init(qmi_client_util_port,
                            requester_info.service_objects[QMI_UTIL_SVC_DMS],
                            qmi_util_unsolicited_dms_ind_handler,
                            requester_info.service_objects[QMI_UTIL_SVC_DMS],
                            &requester_info.qmi_svc_clients[QMI_UTIL_SVC_DMS] );
            qmi_util_logln1("qmi_util_common_init DMS svc client", client_err);
            if (client_err)
                break;
            client_err = qmi_client_get_service_version(qmi_client_util_port,requester_info.service_objects[QMI_UTIL_SVC_DMS], &requester_info.qmi_svc_versions[QMI_UTIL_SVC_DMS] );
            if (client_err)
                break;
            qmi_util_logln2("qmi_util_common_init DMS versions", requester_info.qmi_svc_versions[QMI_UTIL_SVC_DMS].major_ver, requester_info.qmi_svc_versions[QMI_UTIL_SVC_DMS].minor_ver);

            client_err = qmi_client_init(qmi_client_util_port,
                            requester_info.service_objects[QMI_UTIL_SVC_WDS],
                            qmi_util_unsolicited_wds_ind_handler,
                            requester_info.service_objects[QMI_UTIL_SVC_WDS],
                            &requester_info.qmi_svc_clients[QMI_UTIL_SVC_WDS] );
            qmi_util_logln1("qmi_util_common_init WDS svc client", client_err);
            if (client_err)
                break;
            client_err = qmi_client_get_service_version(qmi_client_util_port,requester_info.service_objects[QMI_UTIL_SVC_WDS], &requester_info.qmi_svc_versions[QMI_UTIL_SVC_WDS] );
            if (client_err)
                break;
            qmi_util_logln2("qmi_util_common_init WDS versions", requester_info.qmi_svc_versions[QMI_UTIL_SVC_WDS].major_ver, requester_info.qmi_svc_versions[QMI_UTIL_SVC_WDS].minor_ver);

            client_err = qmi_client_init(qmi_client_util_port,
                            requester_info.service_objects[QMI_UTIL_SVC_UIM],
                            qmi_util_unsolicited_uim_ind_handler,
                            requester_info.service_objects[QMI_UTIL_SVC_UIM],
                            &requester_info.qmi_svc_clients[QMI_UTIL_SVC_UIM] );
            qmi_util_logln1("qmi_util_common_init UIM svc client", client_err);
            if (client_err)
                break;
            client_err = qmi_client_get_service_version(qmi_client_util_port,requester_info.service_objects[QMI_UTIL_SVC_UIM], &requester_info.qmi_svc_versions[QMI_UTIL_SVC_UIM] );
            if (client_err)
                break;
            qmi_util_logln2("qmi_util_common_init UIM versions", requester_info.qmi_svc_versions[QMI_UTIL_SVC_UIM].major_ver, requester_info.qmi_svc_versions[QMI_UTIL_SVC_UIM].minor_ver);


            client_err = qmi_client_init(qmi_client_util_port,
                            requester_info.service_objects[QMI_UTIL_SVC_PB],
                            qmi_util_unsolicited_pb_ind_handler,
                            requester_info.service_objects[QMI_UTIL_SVC_PB],
                            &requester_info.qmi_svc_clients[QMI_UTIL_SVC_PB] );
            qmi_util_logln1("qmi_util_common_init PB svc client", client_err);
            if (client_err)
                break;
            client_err = qmi_client_get_service_version(qmi_client_util_port,requester_info.service_objects[QMI_UTIL_SVC_PB], &requester_info.qmi_svc_versions[QMI_UTIL_SVC_PB] );
            if (client_err)
                break;
            qmi_util_logln2("qmi_util_common_init NAS versions", requester_info.qmi_svc_versions[QMI_UTIL_SVC_PB].major_ver, requester_info.qmi_svc_versions[QMI_UTIL_SVC_PB].minor_ver);

            client_err = qmi_client_init(qmi_client_util_port,
                            requester_info.service_objects[QMI_UTIL_SVC_WMS],
                            qmi_util_unsolicited_wms_ind_handler,
                            requester_info.service_objects[QMI_UTIL_SVC_WMS],
                            &requester_info.qmi_svc_clients[QMI_UTIL_SVC_WMS] );
            qmi_util_logln1("qmi_util_common_init WMS svc client", client_err);
            if (client_err)
                break;
            client_err = qmi_client_get_service_version(qmi_client_util_port,requester_info.service_objects[QMI_UTIL_SVC_WMS], &requester_info.qmi_svc_versions[QMI_UTIL_SVC_WMS] );
            if (client_err)
                break;
            qmi_util_logln2("qmi_util_common_init WMS versions", requester_info.qmi_svc_versions[QMI_UTIL_SVC_WMS].major_ver, requester_info.qmi_svc_versions[QMI_UTIL_SVC_WMS].minor_ver);

            // rf sar
            client_err = qmi_client_init(qmi_client_util_port,
                            requester_info.service_objects[QMI_UTIL_SVC_RF_SAR],
                            qmi_util_unsolicited_rf_sar_ind_handler,
                            requester_info.service_objects[QMI_UTIL_SVC_RF_SAR],
                            &requester_info.qmi_svc_clients[QMI_UTIL_SVC_RF_SAR] );
            qmi_util_logln1("qmi_util_common_init RF SAR svc client", client_err);
            //qmi_util_logln1(".. RF SAR svc client handle ", (int) requester_info.qmi_svc_clients[QMI_UTIL_SVC_RF_SAR]);
            // ignore error for rf sar if (client_err)
            client_err = qmi_client_get_service_version(qmi_client_util_port,requester_info.service_objects[QMI_UTIL_SVC_RF_SAR], &requester_info.qmi_svc_versions[QMI_UTIL_SVC_RF_SAR] );
            // ignore error for rf sar if (client_err)
            qmi_util_logln2("qmi_util_common_init RF SAR versions", requester_info.qmi_svc_versions[QMI_UTIL_SVC_RF_SAR].major_ver, requester_info.qmi_svc_versions[QMI_UTIL_SVC_RF_SAR].minor_ver);

            client_err = 0;
            rs = 0;
            } while (FALSE);

        if (!rs && !client_err)
            {
            pthread_create(&requester_info.requester_thread, NULL, qmi_util_requester_thread_func, NULL);
            requester_info.is_initialized = 1;
            }
        }
    
    qmi_util_logln1("qmi_util_common_init exit", rs);

    return rs;
    }

int qmi_util_common_shutdown()
    {
    int rs = 0;

    qmi_client_error_type client_err = 0;

    if (requester_info.is_initialized)
        {
        // todo: release svc handles
        if (qmi_handle >= 0)
        {
          qmi_release(qmi_handle); 
        }

        // ***
        pthread_mutexattr_destroy(&requester_info.mtx_atr);
        pthread_mutex_destroy(&requester_info.request_list_guard);
        qmi_util_sync_object_destroy(&requester_info.downlink_event);
        // ***
        requester_info.is_initialized = 0;
        }
    
    return rs;
    }

int qmi_util_get_service_version(int service_id, qmi_service_version_info* version_info)
    {
    int rs = 1;
    if (version_info && requester_info.is_initialized)
        {
        *version_info = requester_info.qmi_svc_versions[service_id];
        rs = 0;
        }
    return rs;
    }

qmi_util_request_id qmi_util_post_request_direct(int service_id, int message_id, void* message_specific_payload, int message_specific_payload_len)
    {
    qmi_util_request_id rs;

    qmi_util_request_params req_params;
    memset(&req_params, 0, sizeof(req_params));
    req_params.service_id = service_id;
    req_params.message_id = message_id;
    req_params.message_specific_payload = message_specific_payload;
    req_params.message_specific_payload_len = message_specific_payload_len;
    
    rs = qmi_util_post_request(&req_params);

    return rs;
    }


qmi_util_request_id qmi_util_post_request(qmi_util_request_params* request_params)
    {
    qmi_util_request_id rs = QMI_UTIL_INVALID_REQUEST_ID;

    qmi_util_logln0("qmi_util_post_request entry");

    if (requester_info.is_initialized && request_params != NULL)
        {
        qmi_util_outstanding_request_info* request_info = malloc(sizeof(*request_info));
        qmi_util_outstanding_request_info* iter = NULL;

        if (request_info)
            {
            memset(request_info, 0, sizeof(*request_info));

            do
                { 
                // clone payload
                if (request_params->message_specific_payload != NULL && request_params->message_specific_payload_len > 0)
                    {
                    request_info->message_specific_payload = malloc(request_params->message_specific_payload_len);
                    if (request_info->message_specific_payload != NULL)
                        {
                        memcpy(request_info->message_specific_payload, request_params->message_specific_payload, request_params->message_specific_payload_len);
                        request_info->message_specific_payload_len = request_params->message_specific_payload_len;
                        qmi_util_logln1("qmi_util_post_request payload cloned len", request_info->message_specific_payload_len);
                        }
                    else
                        break;
                    }
                // allocate request ID and queue request
                rs = (
                      QMI_UTIL_INVALID_REQUEST_ID != requester_info.last_generated_request_id && 
                      QMI_UTIL_RESERVED_1_REQUEST_ID != requester_info.last_generated_request_id &&
                      QMI_UTIL_RESERVED_2_REQUEST_ID != requester_info.last_generated_request_id &&
                      QMI_UTIL_RESERVED_3_REQUEST_ID != requester_info.last_generated_request_id &&
                      QMI_UTIL_NOTIFICATION_REQUEST_ID != requester_info.last_generated_request_id
                      ) 
                      ? requester_info.last_generated_request_id : QMI_UTIL_FIRST_AVAILABLE_REQUEST_ID;

                pthread_mutex_lock(&requester_info.request_list_guard);
                request_info->request_id = QMI_UTIL_INVALID_REQUEST_ID;

                do
                {
                    request_info->request_id = rs;                
                    iter = requester_info.request_list;
                    while (NULL != iter)
                        {
                        if (iter->request_id == rs)
                            {
                            rs++;
                            rs = (
                                  QMI_UTIL_INVALID_REQUEST_ID != rs && 
                                  QMI_UTIL_RESERVED_1_REQUEST_ID != rs && 
                                  QMI_UTIL_RESERVED_2_REQUEST_ID != rs && 
                                  QMI_UTIL_RESERVED_3_REQUEST_ID != rs && 
                                  QMI_UTIL_NOTIFICATION_REQUEST_ID != rs
                                  ) 
                                ? rs : QMI_UTIL_FIRST_AVAILABLE_REQUEST_ID;

                            request_info->request_id = QMI_UTIL_INVALID_REQUEST_ID; // reset
                            break;
                            }
                        iter = iter->next;
                        }
                } while (QMI_UTIL_INVALID_REQUEST_ID == request_info->request_id);

                request_info->next = requester_info.request_list;
                requester_info.request_list = request_info;
                pthread_mutex_unlock(&requester_info.request_list_guard);

                request_info->message_id = request_params->message_id;
                request_info->service_id = request_params->service_id;
                request_info->control_info = QMI_UTIL_SVC_CONTROL_EVT_EXECUTE_QMI_REQ;

                requester_info.last_generated_request_id = rs;
                } while (0);

            if (QMI_UTIL_INVALID_REQUEST_ID == rs)
                { // rollback
                free(request_info);
                }
            else
                { // submit
                qmi_util_sync_object_signal(&requester_info.downlink_event);
                qmi_util_logln0("qmi_util_post_request probably posted");
                }
            }
        }

    qmi_util_logln1("qmi_util_post_request exit", rs);

    return rs;
    }


static void* qmi_util_requester_thread_func(void *param)		
    {
    int time_to_go = 0;
    int preserve_request;
    qmi_client_type qmi_svc_client;
    qmi_util_outstanding_request_info* request_info = NULL;
    qmi_util_outstanding_request_info* iter;
    qmi_util_outstanding_request_info* prev;
    qmi_client_error_type send_error;

    uint32_t response_payload_len;
    void* response_payload;

    qmi_util_service_message_info response_info;
    qmi_util_service_message_info indication_info;

    qmi_util_logln0("qmi_util_requester_thread_func entry");

    do
        {        
        do
            {       
            // let's see what we got
            pthread_mutex_lock(&requester_info.request_list_guard);
            request_info = requester_info.request_list;
            pthread_mutex_unlock(&requester_info.request_list_guard);
            
            preserve_request = 0;
            if (request_info != NULL)
                {
                qmi_util_logln1("qmi_util_requester_thread_func got svc id", request_info->control_info);
                switch (request_info->control_info)
                    {
                    case QMI_UTIL_SVC_CONTROL_EVT_QUIT:
                        qmi_util_logln0("qmi_util_requester_thread_func got quit signal");
                        time_to_go = 1; // exit 
                        break;
    
                    case QMI_UTIL_SVC_CONTROL_EVT_INDICATION_REPOST:
                        qmi_util_logln0("qmi_util_requester_thread_func got indication repost msg");
                        memset(&indication_info, 0, sizeof(indication_info));
                        indication_info.service_id = request_info->service_id;
                        indication_info.message_id = request_info->message_id;
                        indication_info.request_id = request_info->request_id;
                        indication_info.message_specific_payload = request_info->message_specific_payload;
                        indication_info.message_specific_payload_len = request_info->message_specific_payload_len;
                        (*requester_info.service_indication_handler)(&indication_info);
                        break;

                    case QMI_UTIL_SVC_CONTROL_EVT_EXECUTE_QMI_REQ:
                        qmi_util_logln0("qmi_util_requester_thread_func got send msg request");
                        // send message
                        qmi_svc_client = requester_info.qmi_svc_clients[request_info->service_id];
                        //qmi_util_logln1(".. client",(int)qmi_svc_client );
                        response_payload_len = 0;
                        qmi_idl_get_message_c_struct_len(requester_info.service_objects[request_info->service_id], 
                                                    QMI_IDL_RESPONSE, 
                                                    request_info->message_id, 
                                                    &response_payload_len); 
                        qmi_util_logln1(".. payload len", response_payload_len);
                        response_payload = malloc(response_payload_len);
                        if (response_payload)
                            {
                            memset(&response_info, 0, sizeof(response_info));
                            qmi_util_logln0("qmi_util_requester_thread_func .. sending message");
                            qmi_util_logln1(".. msg ID", request_info->message_id);
                            qmi_util_logln1(".. svc ID", request_info->service_id);
                            qmi_util_record_time(&response_info.request_sent_time);

                            send_error = qmi_client_send_msg_sync(qmi_svc_client, 
                                                    request_info->message_id, 
                                                    request_info->message_specific_payload,
                                                    request_info->message_specific_payload_len,
                                                    response_payload,
                                                    response_payload_len,
                                                    QMI_SYNC_REQ_DEF_TIMEOUT
                                                     );

                            qmi_util_record_time(&response_info.response_received_time);
                            qmi_util_logln1("qmi_util_requester_thread_func .. message sent", send_error);
                            
                            // communicate result to client
                            response_info.service_id = request_info->service_id;
                            response_info.message_id = request_info->message_id;
                            response_info.message_specific_payload = response_payload;
                            response_info.message_specific_payload_len = response_payload_len;
                            response_info.error_code = send_error;
                            response_info.request_id = request_info->request_id;

                            qmi_util_logln0("qmi_util_requester_thread_func .. invoking callback");
                            (*requester_info.service_message_handler)(&response_info);
                            qmi_util_logln0("qmi_util_requester_thread_func .. callback returned");
    
                            free(response_payload);
                            }
                        break;

                    default:
                        qmi_util_logln0(".. unexpected control info data");
                        break;
                    } // switch
                
                // next request if any
                pthread_mutex_lock(&requester_info.request_list_guard);
                iter = requester_info.request_list;
                prev = NULL;
                while (iter != request_info && iter != NULL)
                    {
                    prev = iter;
                    iter = iter->next;
                    }
                if (iter != NULL)
                    {
                    if (prev != NULL)                
                        {
                        prev->next = iter->next;
                        }
                    if (requester_info.request_list == iter)
                        {
                        requester_info.request_list = iter->next;
                        } 
                    }
                pthread_mutex_unlock(&requester_info.request_list_guard);  
                if (!preserve_request)              
                    {
                    if (request_info->message_specific_payload != NULL && request_info->message_specific_payload_len > 0)
                        free(request_info->message_specific_payload);
                    free(request_info);
                    }
                } // if
            } while (request_info != NULL);

        qmi_util_logln0("qmi_util_requester_thread_func now awaiting message");
        qmi_util_sync_object_wait(&requester_info.downlink_event); 
        qmi_util_logln0("qmi_util_requester_thread_func got something");
        }
    while (!time_to_go);

    qmi_util_logln0("qmi_util_requester_thread_func exiting");

    return NULL;
    }

void qmi_util_inication_repost(int service_id, int message_id, void * message_payload, int message_payload_len)
    {
    qmi_util_outstanding_request_info*  indication_request_info = malloc(sizeof(*indication_request_info));
    uint32_t decoded_payload_len = 0;
    qmi_client_error_type qmi_err;

    if (indication_request_info)
        {
        memset(indication_request_info, 0, sizeof(*indication_request_info));
        indication_request_info->request_id = QMI_UTIL_NOTIFICATION_REQUEST_ID; // dedicated unique ID for all indications
        indication_request_info->service_id = service_id; 
        indication_request_info->message_id = message_id;
        indication_request_info->control_info = QMI_UTIL_SVC_CONTROL_EVT_INDICATION_REPOST;
        if (message_payload_len > 0)
            {            
            qmi_idl_get_message_c_struct_len(requester_info.service_objects[service_id], 
                                        QMI_IDL_INDICATION, 
                                        message_id, 
                                        &decoded_payload_len); 
            qmi_util_logln1("qmi_util_inication_repost encoded payload size", message_payload_len );
            qmi_util_logln2("qmi_util_inication_repost expectations ", decoded_payload_len, message_id);
            indication_request_info->message_specific_payload = malloc(decoded_payload_len);
            if (NULL != indication_request_info->message_specific_payload)
              {
                qmi_err = qmi_client_message_decode(requester_info.qmi_svc_clients[service_id],
								  QMI_IDL_INDICATION,
								  message_id,
								  message_payload,
								  message_payload_len,
								  indication_request_info->message_specific_payload,
								  decoded_payload_len);
                qmi_util_logln1("qmi_util_inication_repost decode status", (int) qmi_err);
                if (qmi_err)
                {
                    free(indication_request_info->message_specific_payload);
                    indication_request_info->message_specific_payload = NULL;
                }
                else
                {
                    indication_request_info->message_specific_payload_len = decoded_payload_len;
                }
              }
            }
        pthread_mutex_lock(&requester_info.request_list_guard);
        indication_request_info->next = requester_info.request_list;
        requester_info.request_list = indication_request_info;
        pthread_mutex_unlock(&requester_info.request_list_guard);
        qmi_util_sync_object_signal(&requester_info.downlink_event);
        }
    }

//--------------------------------------------------------------------------------------------------------------------------------------------------------
// --- event pipe ---
//--------------------------------------------------------------------------------------------------------------------------------------------------------
qmi_util_event_info* qmi_util_create_generic_event( int category_param )
{
    qmi_util_event_info* res = NULL;
    res = malloc( sizeof( *res ) );
    if ( res )
    {
        memset( res, 0, sizeof( *res ) );
        res->category = category_param;
    }
    return res;
}

void qmi_util_destroy_generic_event( qmi_util_event_info* event_info )
{
    if ( event_info )
    {
        free( event_info );
    }
}

void qmi_util_event_pipe_init_obj(qmi_util_event_pipe_info* event_pipe_info)
    {
    if (event_pipe_info)
        {
        memset(event_pipe_info, 0, sizeof(*event_pipe_info));

        pthread_mutexattr_init(&event_pipe_info->mtx_atr);
        pthread_mutexattr_setpshared(&event_pipe_info->mtx_atr, PTHREAD_PROCESS_PRIVATE);

        pthread_mutex_init(&event_pipe_info->event_list_guard, &event_pipe_info->mtx_atr);

        qmi_util_sync_object_init(&event_pipe_info->sync_object);
        }
    }

void qmi_util_event_pipe_destroy_obj(qmi_util_event_pipe_info* event_pipe_info)
    {
    if (event_pipe_info)
        {
        qmi_util_sync_object_destroy(&event_pipe_info->sync_object);
        pthread_mutexattr_destroy(&event_pipe_info->mtx_atr);
        pthread_mutex_destroy(&event_pipe_info->event_list_guard);
        }
    }

void qmi_util_event_pipe_post_event(qmi_util_event_pipe_info* event_pipe_info, qmi_util_event_info * new_event)
    {
    int alert;
    qmi_util_event_info * iter;
    if (event_pipe_info && new_event)
        {    
        pthread_mutex_lock(&event_pipe_info->event_list_guard);

        alert = FALSE;
        iter = event_pipe_info->event_list;
        while ( iter != NULL && !alert)
        {
            if ( new_event == iter )
            {
                alert = TRUE;
            }
            else
            {
                iter = iter->next_event;
            }
        }
        if ( !alert )
        {
            new_event->next_event = event_pipe_info->event_list;
            event_pipe_info->event_list = new_event;
        }
        else
        {
            qmi_util_logln1("ALERT: qmi_util_event_pipe_post_event asked to loop", new_event->category);
        }

        pthread_mutex_unlock(&event_pipe_info->event_list_guard);
        qmi_util_sync_object_signal(&event_pipe_info->sync_object);
        }
    }

void qmi_util_event_pipe_get_event(qmi_util_event_pipe_info* event_pipe_info, qmi_util_event_info ** new_event)
    {
    if (event_pipe_info && new_event)
        {    
        pthread_mutex_lock(&event_pipe_info->event_list_guard);
        qmi_util_event_pipe_do_get_event(event_pipe_info, new_event);
        pthread_mutex_unlock(&event_pipe_info->event_list_guard);
        }
    }

void qmi_util_event_pipe_wait_for_event(qmi_util_event_pipe_info* event_pipe_info, qmi_util_event_info ** new_event)
{
    int already_fetched;
    if (event_pipe_info && new_event)
    {    
        pthread_mutex_lock( &event_pipe_info->event_list_guard );

        already_fetched = (NULL != event_pipe_info->event_list) ? TRUE : FALSE;
        if ( already_fetched )
        {
            qmi_util_event_pipe_do_get_event(event_pipe_info, new_event);
        }
        pthread_mutex_unlock(&event_pipe_info->event_list_guard);
        if (!already_fetched)
            {
            qmi_util_sync_object_wait(&event_pipe_info->sync_object);
            qmi_util_event_pipe_get_event(event_pipe_info, new_event);
            }
    }
}

void qmi_util_event_pipe_do_get_event(qmi_util_event_pipe_info* event_pipe_info, qmi_util_event_info ** new_event)
{
    qmi_util_event_info * cur;    
    qmi_util_event_info * prev;    

    cur = event_pipe_info->event_list;

    if ( NULL != cur )
    {
        prev = NULL;
        while ( NULL != cur->next_event )
        {
            prev = cur;
            cur = cur->next_event;
        }
        *new_event = cur;
        if ( NULL != prev )
        {
            prev->next_event = NULL;
        }
        else
        { // no prev
            event_pipe_info->event_list = NULL;
        }
    
    }
    else
    {
        *new_event = NULL;
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
// --- strings ---
//--------------------------------------------------------------------------------------------------------------------------------------------------------
char* qmi_util_str_clone(char * str)
    {
    char* res = NULL;
    int len;
    if (str)
        {
        len = strlen(str) + 1;
        res = malloc(len);
        if (res)
            {
            memcpy(res, str, len);
            }
        }
    return res;
    }

void qmi_util_str_destroy(char * str)
{
    if (str)
    {
        free(str);
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
// --- logging ---
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void qmi_util_log_enable ( int enable)
    {
    qmi_client_util_is_logging_enabled = enable;
    }

void qmi_util_logln0 ( char * str)
    {
    if (qmi_client_util_is_logging_enabled)
        {
        qmi_util_log("%s",str);
        }
    }

void qmi_util_logln1 ( char * str, int param1)
    {
    if (qmi_client_util_is_logging_enabled)
        {
        qmi_util_log("%s 1 - %d",str, param1);
        }
    }

void qmi_util_logln2 ( char * str, int param1, int param2)
    {
    if (qmi_client_util_is_logging_enabled)
        {
        qmi_util_log("%s 1 - %d, 2 - %d",str, param1, param2);
        }
    }

void qmi_util_logln1s ( char * str, char * param1)
    {
    char * input;
    if (qmi_client_util_is_logging_enabled)
        {
        input = (NULL != param1) ? param1 : "NULL";
        qmi_util_log("%s 1 - %s", str, input);
        }
    }

void qmi_util_logln2s ( char * str, char * param1, char * param2)
    {
    char * input;
    char * input2;
    if (qmi_client_util_is_logging_enabled)
        {
        input = (NULL != param1) ? param1 : "NULL";
        input2 = (NULL != param2) ? param2 : "NULL";
        qmi_util_log("%s 1 - %s 2 - %s", str, input, input2);
        }
    }

void qmi_util_log(const char * fmt, ...){
    time_t rawtime;
    struct tm * timeinfo;
    char szTime[20] = {0}; //Initializes all 20 elements to null
    FILE * file = qmi_simple_ril_suite_output_handle();

    if (is_sig_pipe)
    {
        return;
    }

    if(NULL == file){
        file = stdout;
    }

    // get current time string
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if(timeinfo != NULL)
    {
        //08/23/01 14:55:02
        strftime(szTime, 20, "%x %X", timeinfo);
    }

    pthread_mutex_lock( &log_mutex );
    va_list args;
    va_start(args, fmt);
    fprintf(file, "%s  ", szTime);
    //fprintf(file, "TAG: ");  //TODO add TAG?
    vfprintf(file, fmt, args);
    fprintf(file, "\n");
    fflush(file);

    // write out to stdout
    if(stdout != file) {
        fprintf(stdout, "%s  ", szTime);
        vfprintf(stdout, fmt, args);
        fprintf(stdout, "\n");
    }

#ifndef FEATURE_QMI_ANDROID
    vsyslog(LOG_DEBUG, fmt, args);
#endif
    va_end(args);
    pthread_mutex_unlock( &log_mutex );
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
// --- voice ---
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void qmi_util_unsolicited_voice_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  )
    {
    qmi_util_logln2("qmi_util_unsolicited_voice_ind_handler", msg_id, ind_buf_len);
    qmi_util_inication_repost(QMI_UTIL_SVC_VOICE, msg_id, ind_buf, ind_buf_len);
    }
//--------------------------------------------------------------------------------------------------------------------------------------------------------
// --- NAS ---
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void qmi_util_unsolicited_nas_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  )
    {
    qmi_util_logln1("qmi_util_unsolicited_nas_ind_handler, msg", msg_id);
    qmi_util_inication_repost(QMI_UTIL_SVC_NAS, msg_id, ind_buf, ind_buf_len);
    }
//--------------------------------------------------------------------------------------------------------------------------------------------------------
// --- DMS ---
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void qmi_util_unsolicited_dms_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  )
    {
    qmi_util_logln1("qmi_util_unsolicited_dms_ind_handler, msg", msg_id);
    qmi_util_inication_repost(QMI_UTIL_SVC_DMS, msg_id, ind_buf, ind_buf_len);
    }

//--------------------------------------------------------------------------------------------------------------------------------------------------------
// --- WMS ---
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void qmi_util_unsolicited_wms_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  )
    {
    qmi_util_logln1("qmi_util_unsolicited_wms_ind_handler, msg", msg_id);
    qmi_util_inication_repost(QMI_UTIL_SVC_WMS, msg_id, ind_buf, ind_buf_len);
    }
//--------------------------------------------------------------------------------------------------------------------------------------------------------
// --- RF SAR ---
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void qmi_util_unsolicited_rf_sar_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  )
    {
    qmi_util_logln1("qmi_util_unsolicited_rf_sar_ind_handler, msg", msg_id);
    qmi_util_inication_repost(QMI_UTIL_SVC_RF_SAR, msg_id, ind_buf, ind_buf_len);
    }
// --- WDS ---
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void qmi_util_unsolicited_wds_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  )
    {
    qmi_util_logln1("qmi_util_unsolicited_wds_ind_handler, msg", msg_id);
    qmi_util_inication_repost(QMI_UTIL_SVC_WDS, msg_id, ind_buf, ind_buf_len);
    }
//--------------------------------------------------------------------------------------------------------------------------------------------------------
// --- UIM ---
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void qmi_util_unsolicited_uim_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  )
    {
    qmi_util_logln1("qmi_util_unsolicited_uim_ind_handler, msg", msg_id);
    qmi_util_inication_repost(QMI_UTIL_SVC_UIM, msg_id, ind_buf, ind_buf_len);
    }
//--------------------------------------------------------------------------------------------------------------------------------------------------------
// --- PB ---
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void qmi_util_unsolicited_pb_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  )
    {
    qmi_util_logln1("qmi_util_unsolicited_pb_ind_handler, msg", msg_id);
    qmi_util_inication_repost(QMI_UTIL_SVC_PB, msg_id, ind_buf, ind_buf_len);
    }


void qmi_util_record_time(struct timeval * recorded_time)
{
    if(recorded_time)
    {
        gettimeofday(recorded_time,NULL);
    }
}

long long qmi_util_calculate_time_difference(struct timeval * start_time,struct timeval * end_time)
{
    struct timeval difference_time;

    memset(&difference_time,0,sizeof(difference_time));
    if(start_time && end_time)
    {
      difference_time.tv_sec  = end_time->tv_sec - start_time->tv_sec ;
      difference_time.tv_usec = end_time->tv_usec - start_time->tv_usec;

      if(difference_time.tv_usec<0)
      {
        difference_time.tv_usec += 1000000;
        difference_time.tv_sec -= 1;
      }
    }
    return 1000000LL*difference_time.tv_sec+ difference_time.tv_usec;
}

