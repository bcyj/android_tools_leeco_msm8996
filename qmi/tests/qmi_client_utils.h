/******************************************************************************
  @file    qmi_client_utils.h
  @brief   The QMI Client Utils for telephony adaptation or test code.

  DESCRIPTION
  QMI message send / receive high level utilities  - declarations

  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include <sys/time.h>           //timeval
#include <pthread.h>            //pthread_mutex_t
#include <stdio.h>
#include "qmi.h"
#include "qmi_client.h"

#ifndef QMI_CLIENT_UTILS_H
#define QMI_CLIENT_UTILS_H

#ifndef TRUE
#define TRUE    (1)
#endif // TRUE

#ifndef FALSE
#define FALSE   (0)
#endif // FALSE

#define QMI_SYNC_REQ_DEF_TIMEOUT (500)

extern FILE* distributor_data_handle;

typedef struct qmi_util_request_params 
{
  int service_id;
  int message_id;
  int message_specific_payload_len;
  void* message_specific_payload;
} qmi_util_request_params;

typedef unsigned int qmi_util_request_id;

typedef struct qmi_util_service_message_info
{
  qmi_util_request_id request_id;
  int service_id;
  int message_id;
  int message_specific_payload_len;
  void* message_specific_payload;
  qmi_client_error_type error_code;
  struct timeval request_sent_time;
  struct timeval response_received_time;
} qmi_util_service_message_info;

enum
{
  QMI_UTIL_SVC_FIRST,
  QMI_UTIL_SVC_VOICE = QMI_UTIL_SVC_FIRST,
  QMI_UTIL_SVC_NAS,
  QMI_UTIL_SVC_WMS,
  QMI_UTIL_SVC_WDS,
  QMI_UTIL_SVC_DMS,
  QMI_UTIL_SVC_UIM,
  QMI_UTIL_SVC_PB,
  QMI_UTIL_SVC_RF_SAR,
  QMI_UTIL_SVC_LAST,
  QMI_UTIL_SVC_NOF_SVC = QMI_UTIL_SVC_LAST
} qmi_util_common_services_type;

typedef int (* qmi_util_service_message_handler) (qmi_util_service_message_info* message_info);

extern qmi_util_request_id qmi_util_post_request(qmi_util_request_params* request_params);
extern qmi_util_request_id qmi_util_post_request_direct(int service_id, int message_id, void* message_specific_payload, int message_specific_payload_len);
extern int qmi_util_common_init(qmi_util_service_message_handler resp_handler, qmi_util_service_message_handler ind_handler);
extern int qmi_util_common_shutdown();

#define QMI_UTIL_INVALID_REQUEST_ID      (0)
#define QMI_UTIL_RESERVED_1_REQUEST_ID   (1)
#define QMI_UTIL_RESERVED_2_REQUEST_ID   (2)
#define QMI_UTIL_RESERVED_3_REQUEST_ID   (3)
#define QMI_UTIL_NOTIFICATION_REQUEST_ID (4)

#define QMI_UTIL_FIRST_AVAILABLE_REQUEST_ID (QMI_UTIL_NOTIFICATION_REQUEST_ID + 1)

typedef struct qmi_util_sync_object_info
{
    pthread_mutex_t mutex;
    pthread_mutexattr_t mtx_atr;
    pthread_cond_t  cond_var;
} qmi_util_sync_object_info;

extern void qmi_util_sync_object_init(qmi_util_sync_object_info* sync_object_info);
extern void qmi_util_sync_object_destroy(qmi_util_sync_object_info* sync_object_info);
extern void qmi_util_sync_object_wait(qmi_util_sync_object_info* sync_object_info);
extern void qmi_util_sync_object_signal(qmi_util_sync_object_info* sync_object_info);

typedef struct qmi_util_event_info
{
    struct qmi_util_event_info * next_event;
    int category;
} qmi_util_event_info;

typedef struct qmi_util_event_pipe_info
{
    qmi_util_event_info * event_list;
    pthread_mutex_t event_list_guard;
    pthread_mutexattr_t mtx_atr;
    qmi_util_sync_object_info sync_object;
} qmi_util_event_pipe_info;

extern void qmi_util_event_pipe_init_obj(qmi_util_event_pipe_info* event_pipe_info);
extern void qmi_util_event_pipe_destroy_obj(qmi_util_event_pipe_info* event_pipe_info);
extern void qmi_util_event_pipe_post_event(qmi_util_event_pipe_info* event_pipe_info, qmi_util_event_info * new_event);
extern void qmi_util_event_pipe_get_event(qmi_util_event_pipe_info* event_pipe_info, qmi_util_event_info ** new_event);
extern void qmi_util_event_pipe_wait_for_event(qmi_util_event_pipe_info* event_pipe_info, qmi_util_event_info ** new_event);
extern int qmi_util_get_service_version(int service_id, qmi_service_version_info* version_info);

extern char* qmi_util_str_clone(char * str);

extern void qmi_util_str_destroy(char * str);

extern void qmi_util_log(const char * fmt, ...);
extern void qmi_util_logln0 ( char * str);
extern void qmi_util_logln1 ( char * str, int param1);
extern void qmi_util_logln1s ( char * str, char * param1);
extern void qmi_util_logln2 ( char * str, int param1, int param2);
extern void qmi_util_logln2s ( char * str, char * param1, char * param2);
extern void qmi_util_log_enable ( int enable);
#ifdef FEATURE_QMI_ANDROID
extern int qmi_util_set_modem ( char *modem);
#endif
void qmi_util_set_modem_port (const char *modem_port);
extern long long qmi_util_calculate_time_difference(struct timeval * start_time,struct timeval * end_time);

extern qmi_util_event_info* qmi_util_create_generic_event( int category_param );
extern void qmi_util_destroy_generic_event( qmi_util_event_info* event_info );

#endif // QMI_CLIENT_UTILS_H
