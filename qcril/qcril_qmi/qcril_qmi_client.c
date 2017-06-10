/*!
  @file
  qcril_qmi_client.c

  @brief
  Client initializations and common functions defined here.

*/

/*===========================================================================

  Copyright (c) 2009-2010, 2013 Qualcomm Technologies, Inc. All Rights Reserved

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


when       who     what, where, why
--------   ---     ----------------------------------------------------------
08/06/10   sk      Initial Changes

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <errno.h>
#include <cutils/memory.h>
//#include "qmi_client_instance_defs.h"
#include "qcril_qmi_client.h"
#include "common_v01.h"
#include "voice_service_v02.h"
#include "network_access_service_v01.h"
#include "wireless_messaging_service_v01.h"
#include "device_management_service_v01.h"
#include "wireless_data_service_v01.h"
#include "phonebook_manager_service_v01.h"
#include "specific_absorption_rate_v01.h"
#include "ip_multimedia_subsystem_application_v01.h"
#include "qcril_qmi_voice.h"
#include "qcril_qmi_nas.h"
#include "qcril_reqlist.h"
#include "qcril_pbm.h"
#include "qcril_qmi_sms.h"
#include "qcril_arb.h"
#include "qcril_qmi_ims.h"
#include "qcril_qmi_imsa.h"
#include "qcril_qmi_coex.h"
#include "qcril_qmi_imss.h"
#include "qcril_am.h"
#include "qmi_errors.h"
#include "qcril_qmi_pdc.h"
#include "qmi_ril_platform_dep.h"
#include "ip_multimedia_subsystem_presence_v01.h"
#include "radio_frequency_radiated_performance_enhancement_v01.h"
#include "ip_multimedia_subsystem_settings_v01.h"
#include "persistent_device_configuration_v01.h"

#ifdef QMI_RIL_UTF
#include <ril_utf_wake_lock_redef.h>
#else
#include <hardware_legacy/power.h>
#endif

#include "cri_core.h"
#include "hlos_csvt_core.h"

/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

#define MAX_TRIES_FOR_QMI_INIT 10
#define MAX_TRIES_FOR_QMI_IMSA_INIT 20

#define RESUMER_ACTION_DELAY 1

#define RESUMER_RETRY_DELAY  1

#define QCRIL_PRE_CLIENT_INIT_WAKE_LOCK_NAME "qcril_pre_client_init"
#define QCRIL_PRE_CLIENT_INIT_WAKE_LOCK_HELD_PROP "ril.qcril_pre_init_lock_held"
#define QMI_RIL_DO_NOT_INIT_CSVT "persist.radio.do_not_init_csvt"

// cache handling
#define CLIENT_CACHE_LOCK()                            { pthread_mutex_lock(&client_info.cache_lock_mutex); }
#define CLIENT_CACHE_UNLOCK()                          { pthread_mutex_unlock(&client_info.cache_lock_mutex); }

typedef struct
{
  int transition_index;
} qmi_ril_modem_reset_round_info_type;



/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/


qcril_qmi_client_private_info_type client_info;

static qmi_ril_modem_reset_round_info_type qmi_ril_modem_reset_round_info;

static const qcril_evt_e_type qmi_ril_modem_reset_suspend_evt_cycle[] =
{
  QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_SUSPEND_REQ,
  QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_SUSPEND_REQ,
  QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_SUSPEND_REQ,
  QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_SUSPEND_REQ,

  QCRIL_EVT_NONE // this must be final
};

static const qcril_evt_e_type qmi_ril_modem_reset_resume_evt_cycle[] =
{
  QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_RESUME_REQ,
  QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_RESUME_REQ,
  QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_RESUME_REQ,
  QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_RESUME_REQ,

  QCRIL_EVT_NONE // this must be final
};

static uint32 qmi_ril_ssr_voice_call_supress_timerid;

static uint8 qmi_ril_qmi_client_pre_initialization_lock_held;

/*===========================================================================

                   INTERNAL FUNCTION PROTOTYPES

===========================================================================*/
static void qcril_qmi_client_common_empty_unsol_ind_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                          *ind_buf,
  unsigned int                   ind_buf_len,
  void                          *ind_cb_data
);

static void qmi_ril_enter_suspend(void);
static void qmi_ril_enter_resume(void);
static void qmi_ril_next_suspending_action(void);
static void qmi_ril_next_resuming_action(void);
static RIL_Errno qcril_qmi_init_core_client_handles( void );
static void qcril_qmi_release_client_handles(void);
static RIL_Errno qmi_ril_resumer_initiate(void);
static void* qmi_ril_resumer_deferred_action_thread_proc(void * param);
static void qmi_ril_resume_retry_handler(void * param);
static void qmi_ril_pre_resume_main_threaded(void * param);
static void qmi_ril_suspend_init_work_handler();
static void qcril_qmi_cb_thread_name_init();
static void qcril_qmi_qmi_thread_name_init_cb
(
  qmi_client_type              user_handle,
  unsigned int                 msg_id,
  void                        *resp_c_struct,
  unsigned int                 resp_c_struct_len,
  void                        *resp_cb_data,
  qmi_client_error_type        transp_err
);

static void qmi_ril_ssr_perform_final_post_ssr_init(void * param);
static void qmi_ril_suspended_main_threaded(void * param);
static void qmi_ril_final_suspend_main_threaded(void * param);

static void qmi_ril_qmi_req_log( qmi_idl_service_object_type service_object,
                                 int message_id,
                                 void* encoded_qmi_bytestream,
                                 int encoded_qmi_bytestream_len );
static void qmi_ril_qmi_resp_log( qmi_idl_service_object_type service_object,
                                  int message_id,
                                  void* encoded_qmi_bytestream,
                                  int encoded_qmi_bytestream_len );
static void qmi_ril_qmi_ind_log( int service_id,
                                 int message_id,
                                 void* encoded_qmi_bytestream,
                                 int encoded_qmi_bytestream_len );

static void qmi_ril_qmi_client_pre_initialization_set();
static void qmi_ril_qmi_client_pre_initialization_get();

static void qcril_qmi_service_down_event
(
  qmi_client_type clnt,
  qmi_client_error_type error,
  void *error_cb_data
);

static void qcril_qmi_service_up_event
(
  qmi_client_type clnt,
  qmi_idl_service_object_type svc_obj,
  qmi_client_notify_event_type service_event,
  void *cb_data
);

int qcril_qmi_modem_power_voting_state();

/* data to check if service is up or not */
typedef struct {
    qmi_idl_service_object_type   svc_obj;
    void                         *cb_data;
} qcril_check_service_up_data;

void qcril_qmi_modem_power_process_bootup();

/*===========================================================================

                                FUNCTIONS

===========================================================================*/


//===========================================================================
//qcril_qmi_client_init
//===========================================================================
RIL_Errno qcril_qmi_client_init( void )
{
  qmi_client_error_type client_err = 0;

  RIL_Errno res = RIL_E_GENERIC_FAILURE;

  QCRIL_LOG_FUNC_ENTRY();

  /* Start modem or vote for start modem */
  qcril_qmi_modem_power_process_bootup();


  memset(&client_info, 0, sizeof(client_info));

  do
  {
      QCRIL_LOG_DEBUG( "Client connecting to QMI FW" );

      // QMI VOICE command callback
      client_info.client_cbs[QCRIL_QMI_CLIENT_VOICE] = qcril_qmi_voice_command_cb;

      // Get IDL service objects
      client_info.service_objects[QCRIL_QMI_CLIENT_VOICE] = voice_get_service_object_v02();
      client_info.service_objects[QCRIL_QMI_CLIENT_NAS] = nas_get_service_object_v01();
      client_info.service_objects[QCRIL_QMI_CLIENT_WMS] = wms_get_service_object_v01();
      client_info.service_objects[QCRIL_QMI_CLIENT_WDS] = wds_get_service_object_v01();
      client_info.service_objects[QCRIL_QMI_CLIENT_DMS] = dms_get_service_object_v01();
      /*client_info.service_objects[QCRIL_QMI_CLIENT_UIM] = uim_get_service_object_v01();*/
      client_info.service_objects[QCRIL_QMI_CLIENT_PBM] =  pbm_get_service_object_v01();
      client_info.service_objects[QCRIL_QMI_CLIENT_RF_SAR] =  sar_get_service_object_v01();
      client_info.service_objects[QCRIL_QMI_CLIENT_IMS_VT] =   ims_qmi_get_service_object_v01();
      client_info.service_objects[QCRIL_QMI_CLIENT_IMS_PRESENCE] = imsp_get_service_object_v01();
      client_info.service_objects[QCRIL_QMI_CLIENT_IMSA] = imsa_get_service_object_v01();
      client_info.service_objects[QCRIL_QMI_CLIENT_RFPE] = rfrpe_get_service_object_v01();
      client_info.service_objects[QCRIL_QMI_CLIENT_IMS_SETTING] = imss_get_service_object_v01();
      if ( qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID )
      {
        client_info.service_objects[QCRIL_QMI_CLIENT_PDC] = pdc_get_service_object_v01();
      }

      pthread_mutexattr_init(&client_info.cache_lock_mtx_atr);
      pthread_mutex_init(&client_info.cache_lock_mutex, &client_info.cache_lock_mtx_atr);

      res = qcril_qmi_init_core_client_handles();

      if (RIL_E_SUCCESS != res)
          break;

  } while (FALSE);

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);

  return res;

} // qcril_qmi_client_init

//===========================================================================
//qmi_ril_client_get_master_port
//===========================================================================
qmi_client_qmux_instance_type qmi_ril_client_get_master_port(void)
{
    int qmi_default_port;

    if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_SGLTE ) )
    {
      if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_SGLTE_CSFB ) )
      {
        if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_SGLTE2 ) )
        {
          qmi_default_port = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;  // local modem
        }
        else
        {
          qmi_default_port = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;  // local modem
        }
      }
      else
      {
        qmi_default_port = QMI_CLIENT_QMUX_PROXY_INSTANCE;
      }
    }
    else if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_FUSION_CSFB ) || qmi_ril_is_feature_supported( QMI_RIL_FEATURE_SVLTE2 ) )
    {
      qmi_default_port = QMI_CLIENT_QMUX_PROXY_INSTANCE;
    }
    else if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_APQ ) )
    {
        if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_PCI ) )
        {
            qmi_default_port = QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0;
        }
        else
        {
            qmi_default_port = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
        }
    }
    else if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_DSDA ) )
    {
      if (qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID)
      {
         qmi_default_port = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
      }
      else
      {
         qmi_default_port = QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0;
      }
    }
    else if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_DSDA2 ) )
    {
      if (qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID)
      {
         qmi_default_port = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
      }
      else
      {
         qmi_default_port = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
      }
    }
    else
    {
      qmi_default_port = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;  // default modem
    }

    QCRIL_LOG_INFO("using port %d", qmi_default_port);
    return qmi_default_port;
} // qmi_ril_client_get_master_port

//===========================================================================
// qcril_qmi_init_imsa_client_handles_proc
//===========================================================================
void * qcril_qmi_init_imsa_client_handles_proc(void * param)
{
   qmi_client_error_type client_err = QMI_NO_ERR;
   qmi_client_qmux_instance_type qmi_master_port = qmi_ril_client_get_master_port();
   int time_out = 4;
   int num_tries = 0;

   QCRIL_LOG_FUNC_ENTRY();
   QCRIL_NOTUSED(param);

   do
   {
      if (num_tries != 0)
      {
         sleep(1);
      }
      QCRIL_LOG_INFO("Trying for (0-referenced) try # %d port[%d]", num_tries, qmi_master_port);
      // Initialize the QMI IMSA client
      client_err = qmi_client_init_instance(client_info.service_objects[QCRIL_QMI_CLIENT_IMSA],
                                            qmi_master_port,
                                            qcril_qmi_imsa_unsol_ind_cb,
                                            NULL,
                                            &client_info.os_params[QCRIL_QMI_CLIENT_IMSA],
                                            time_out,
                                            &client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_IMSA]);
      num_tries++;
   } while ( (client_err != QMI_NO_ERR) && (num_tries < MAX_TRIES_FOR_QMI_IMSA_INIT) );

   if (client_err)
   {
      QCRIL_LOG_INFO("qmi_client_init_instance returned failure(%d) for IMSA ",
                     client_err);
      client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_IMSA] = NULL;
   }
   else
   {
      client_info.client_cbs[QCRIL_QMI_CLIENT_IMSA] = qcril_qmi_imsa_command_cb;
      qcril_qmi_imsa_init();
      client_err = qmi_client_register_error_cb(client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_IMSA],
                                                qcril_qmi_service_down_event,
                                                (void *)(intptr_t)QCRIL_QMI_CLIENT_IMSA);
   }

   QCRIL_LOG_FUNC_RETURN();
   return NULL;
} // qcril_qmi_init_imsa_client_handles_proc

//===========================================================================
// qcril_qmi_init_imsa_client_handles
//===========================================================================
void qcril_qmi_init_imsa_client_handles()
{
    pthread_attr_t attr;
    pthread_t      thread_pid;
    int res;

    QCRIL_LOG_FUNC_ENTRY();

    qcril_qmi_imsa_set_init_state(FALSE);

#ifdef QMI_RIL_UTF
    pthread_attr_init (&attr);
    res = utf_pthread_create_handler(&thread_pid, &attr, qcril_qmi_init_imsa_client_handles_proc, NULL);
    pthread_attr_destroy(&attr);
#else
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    res = pthread_create(&thread_pid, &attr, qcril_qmi_init_imsa_client_handles_proc, NULL);
    pthread_attr_destroy(&attr);
#endif
    qmi_ril_set_thread_name(thread_pid, QMI_RIL_CORE_INIT_IMSA_THREAD_NAME);
    QCRIL_LOG_INFO( "res, pid %d, %d", res, (int) thread_pid );

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_init_imsa_client_handles
//===========================================================================
//qcril_qmi_init_core_client_handles
//===========================================================================
RIL_Errno qcril_qmi_init_core_client_handles( void )
{
  qmi_client_error_type client_err = QMI_NO_ERR;
  int   num_tries = 0;
  qmi_client_qmux_instance_type qmi_master_port;
  int   idx;
  boolean imsp_init = FALSE;
  boolean imss_init = FALSE;
  boolean pdc_init = FALSE;

  RIL_Errno res = RIL_E_GENERIC_FAILURE;
  RIL_SubscriptionType sub_num;
  qcril_instance_id_e_type cur_proc_instance;
  RIL_Errno bind_res;

  char* log_failed_qmi_svc = NULL;
  int num_of_services = 0;
  int do_not_init_csvt = 0;
  int time_out = 4;

  int len = 0;
  char property_name[ PROPERTY_VALUE_MAX ];
  char property_value[ PROPERTY_VALUE_MAX ];
  uint8_t stack_id;

  QCRIL_LOG_FUNC_ENTRY();

  qmi_ril_set_sms_svc_status( QMI_RIL_SMS_SVC_NOT_INITIALZIED );

  do
  {
        if ( qmi_ril_is_multi_sim_feature_supported() )
        {
            cur_proc_instance = qmi_ril_get_process_instance_id();

            snprintf( property_name, sizeof(property_name), "%s%d", QMI_RIL_MULTI_SIM_STACK_ID, cur_proc_instance );
            qmi_ril_get_property_value_from_integer(property_name, &stack_id, QCRIL_MODEM_MAX_STACK_ID);
            QCRIL_LOG_INFO( "[MSIM].. stack id prop  %s - %d", property_name, stack_id );
            if ( ( stack_id < QCRIL_MODEM_PRIMARY_STACK_ID ) ||
                 ( stack_id >= QCRIL_MODEM_MAX_STACK_ID) )
            {
                QCRIL_LOG_INFO( "[MSIM].. bind to subscription based on instance %d", (int) cur_proc_instance );
                switch ( cur_proc_instance )
                {
                  case QCRIL_THIRD_INSTANCE_ID:
                    sub_num = RIL_SUBSCRIPTION_3;
                    break;
                  case QCRIL_SECOND_INSTANCE_ID:
                    sub_num = RIL_SUBSCRIPTION_2;
                    break;
                  case QCRIL_DEFAULT_INSTANCE_ID: // fallthrough
                  default:
                    sub_num = RIL_SUBSCRIPTION_1;
                    break;
                }
                stack_id = (uint8_t) sub_num;
            }
        }

        for ( idx = 0; idx < QCRIL_QMI_CLIENT_MAX; idx ++ )
        {
            client_info.qmi_svc_clients[ idx ] = NULL;
            client_info.client_state[ idx ]    = QCRIL_QMI_SERVICE_NOT_CONNECTED;
        }

        qmi_master_port = qmi_ril_client_get_master_port();
        num_tries = 0;
        do
        {
          if (num_tries != 0)
          {
            sleep(1);
          }

          QCRIL_LOG_INFO("Trying qmi_client_init_instance() try # %d port[%d]", num_tries, qmi_master_port);

          // Initialize the QMI VOICE client
          client_err = qmi_client_init_instance(client_info.service_objects[QCRIL_QMI_CLIENT_VOICE],
                                                qmi_master_port,
                                                qcril_qmi_voice_unsol_ind_cb,
                                                NULL,
                                                &client_info.os_params[QCRIL_QMI_CLIENT_VOICE],
                                                time_out,
                                                &client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_VOICE]);
          num_tries++;
        } while ( (client_err != QMI_NO_ERR) && (num_tries < MAX_TRIES_FOR_QMI_INIT) );
        QCRIL_LOG_INFO("qmi_client_init_instance returned (%d) for VOICE ",
                        client_err);

        if (client_err)
        {
          QCRIL_LOG_INFO("qmi_client_init_instance returned failure(%d) for VOICE ",
                         client_err);

          log_failed_qmi_svc = "QMI Voice";
          break;
        }
        else
        {
          client_info.client_state[QCRIL_QMI_CLIENT_VOICE] = QCRIL_QMI_SERVICE_CONNECTED;
          qmi_ril_qmi_client_pre_initialization_release();
        }

        // Initialize the QMI DMS client
        client_err = qmi_client_init_instance(client_info.service_objects[QCRIL_QMI_CLIENT_DMS],
                                              qmi_master_port,
                                              qcril_qmi_dms_unsolicited_indication_cb,
                                              NULL,
                                              &client_info.os_params[QCRIL_QMI_CLIENT_DMS],
                                              time_out,
                                              &client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_DMS]);
        QCRIL_LOG_INFO("qmi_client_init_instance returned (%d) for DMS ",
                        client_err);
        if (client_err)
        {
          QCRIL_LOG_INFO("qmi_client_init_instance returned failure(%d) for DMS ",
                          client_err);
          log_failed_qmi_svc = "QMI DMS";
          break;
        }
        else
        {
          client_info.client_state[QCRIL_QMI_CLIENT_DMS] = QCRIL_QMI_SERVICE_CONNECTED;
        }

        // Initialize the QMI NAS client
        client_err = qmi_client_init_instance(client_info.service_objects[QCRIL_QMI_CLIENT_NAS],
                                              qmi_master_port,
                                              qcril_qmi_nas_unsolicited_indication_cb,
                                              NULL,
                                              &client_info.os_params[QCRIL_QMI_CLIENT_NAS],
                                              time_out,
                                              &client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_NAS]);
        QCRIL_LOG_INFO("qmi_client_init_instance returned (%d) for NAS ",
                        client_err);
        if (client_err)
        {
          QCRIL_LOG_INFO("qmi_client_init_instance returned failure(%d) for NAS",
                         client_err);
          log_failed_qmi_svc = "QMI NAS";
          break;
        }
        else
        {
          client_info.client_state[QCRIL_QMI_CLIENT_NAS] = QCRIL_QMI_SERVICE_CONNECTED;
        }

        client_info.client_cbs[QCRIL_QMI_CLIENT_NAS] = qcril_qmi_nas_minority_command_cb;

        // Initialize the QMI PBM client
        client_err = qmi_client_init_instance(client_info.service_objects[QCRIL_QMI_CLIENT_PBM],
                                              qmi_master_port,
                                              qcril_qmi_pbm_unsol_ind_cb,
                                              NULL,
                                              &client_info.os_params[QCRIL_QMI_CLIENT_PBM],
                                              time_out,
                                              &client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_PBM]);
        QCRIL_LOG_INFO("qmi_client_init_instance returned (%d) for PBM ",
                        client_err);
        if (client_err)
        {
          QCRIL_LOG_INFO("qmi_client_init_instance returned failure(%d) for PBM ",
                          client_err);
          log_failed_qmi_svc = "QMI PBM";
          break;
        }
        else
        {
          client_info.client_state[QCRIL_QMI_CLIENT_PBM] = QCRIL_QMI_SERVICE_CONNECTED;
        }

        // Initialize the QMI RF SAR client
        client_err = qmi_client_init_instance(client_info.service_objects[QCRIL_QMI_CLIENT_RF_SAR],
                                              qmi_master_port,
                                              qcril_qmi_client_common_empty_unsol_ind_cb,
                                              NULL,
                                              &client_info.os_params[QCRIL_QMI_CLIENT_RF_SAR],
                                              time_out,
                                              &client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_RF_SAR]);
        QCRIL_LOG_INFO("qmi_client_init_instance returned (%d) for RF SAR ",
                        client_err);
        if (client_err)
        {
          // do not return failure if QMI RF SAR initialization fails
          QCRIL_LOG_INFO("qmi_client_init_instance returned failure(%d) for RF SAR ",
                          client_err);
          client_info.qmi_svc_clients[ QCRIL_QMI_CLIENT_RF_SAR ] = NULL;
        }
        else
        {
          client_info.client_state[QCRIL_QMI_CLIENT_RF_SAR] = QCRIL_QMI_SERVICE_CONNECTED;
        }

        // Initialize the QMI WMS client
        client_err = qmi_client_init_instance(client_info.service_objects[QCRIL_QMI_CLIENT_WMS],
                                              qmi_master_port,
                                              qcril_qmi_sms_unsol_ind_cb,
                                              NULL,
                                              &client_info.os_params[QCRIL_QMI_CLIENT_WMS],
                                              time_out,
                                              &client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_WMS]);
        QCRIL_LOG_INFO("qmi_client_init_instance returned (%d) for WMS ",
                        client_err);
        if (client_err)
        {
          QCRIL_LOG_INFO("qmi_client_init_instance returned failure(%d) for WMS",
                          client_err);
          log_failed_qmi_svc = "QMI WMS";
          break;
        }
        else
        {
          client_info.client_state[QCRIL_QMI_CLIENT_WMS] = QCRIL_QMI_SERVICE_CONNECTED;
        }

        // QMI WMS command callback
        client_info.client_cbs[QCRIL_QMI_CLIENT_WMS] = qcril_qmi_sms_command_cb;

        // Initialize the QMI IMS VT client
        client_err = qmi_client_init_instance(client_info.service_objects[QCRIL_QMI_CLIENT_IMS_VT],
                                              qmi_master_port,
                                              qcril_qmi_ims_vt_unsol_ind_cb,
                                              NULL,
                                              &client_info.os_params[QCRIL_QMI_CLIENT_IMS_VT],
                                              time_out,
                                              &client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_IMS_VT]);
        if ( client_err )
        {
          QCRIL_LOG_INFO("qcril_qmi_vt_init returned failure(%d)",client_err);
          client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_IMS_VT] = NULL;
        }

        // QMI IMS VT command callback
        client_info.client_cbs[QCRIL_QMI_CLIENT_IMS_VT] = qcril_qmi_ims_vt_command_cb;

        // Initialize the QMI IMS client
        client_err = qmi_client_init_instance(client_info.service_objects[QCRIL_QMI_CLIENT_IMS_PRESENCE],
                                              qmi_master_port,
                                              qcril_qmi_ims_presence_unsol_ind_cb,
                                              NULL,
                                              &client_info.os_params[QCRIL_QMI_CLIENT_IMS_PRESENCE],
                                              time_out,
                                              &client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_IMS_PRESENCE]);
        if ( client_err )
        {
          QCRIL_LOG_ERROR("qcril_qmi_presence_init returned failure(%d)",client_err);
          client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_IMS_PRESENCE] = NULL;
        }
        else
        {
          imsp_init = TRUE;
        }

        // QMI IMS PRESENCE command callback
        client_info.client_cbs[QCRIL_QMI_CLIENT_IMS_PRESENCE] = qcril_qmi_ims_presence_command_cb;

        // Initialize the QMI RFPE client
        client_err = qmi_client_init_instance(client_info.service_objects[QCRIL_QMI_CLIENT_RFPE],
                                              qmi_master_port,
                                              qcril_qmi_client_common_empty_unsol_ind_cb,
                                              NULL,
                                              &client_info.os_params[QCRIL_QMI_CLIENT_RFPE],
                                              time_out,
                                              &client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_RFPE]);
        QCRIL_LOG_INFO("qmi_client_init_instance returned (%d) for RFRPE ",
                        client_err);
        if (client_err)
        {
          // do not return failure if QMI RFRPE initialization fails
          QCRIL_LOG_INFO("qmi_client_init_instance returned failure(%s) for RFRPE ",
                          qmi_errstr(client_err));
          client_info.qmi_svc_clients[ QCRIL_QMI_CLIENT_RFPE ] = NULL;
        }
        else
        {
          client_info.client_state[QCRIL_QMI_CLIENT_RFPE] = QCRIL_QMI_SERVICE_CONNECTED;
        }


        qmi_ril_get_property_value_from_integer(QMI_RIL_DO_NOT_INIT_CSVT,
                                                &do_not_init_csvt,
                                                FALSE);


        cri_core_cri_client_init_info_type cri_core_cri_client_init_info;
        memset(&cri_core_cri_client_init_info, 0, sizeof(cri_core_cri_client_init_info));


        if(FALSE == do_not_init_csvt)
        {
            cri_core_cri_client_init_info.service_info[num_of_services].cri_service_id = QMI_CSVT_SERVICE;
            cri_core_cri_client_init_info.service_info[num_of_services].hlos_ind_cb =  hlos_csvt_unsol_ind_handler;
            num_of_services++;
        }

        cri_core_cri_client_init_info.number_of_cri_services_to_be_initialized = num_of_services;
        cri_core_cri_client_init_info.subscription_id = stack_id +
                                                        CRI_CORE_PRIMARY_CRI_SUBSCRIPTION_ID;
        cri_core_cri_client_init(&cri_core_cri_client_init_info);

        // Initialize the QMI IMS Setting client
        client_err = qmi_client_init_instance(client_info.service_objects[QCRIL_QMI_CLIENT_IMS_SETTING],
                                              qmi_master_port,
                                              qcril_qmi_client_common_empty_unsol_ind_cb,
                                              NULL,
                                              &client_info.os_params[QCRIL_QMI_CLIENT_IMS_SETTING],
                                              time_out,
                                              &client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_IMS_SETTING]);
        if (client_err)
        {
          // do not return failure if QMI IMS Setting initialization fails
          QCRIL_LOG_INFO("qmi_client_init_instance returned failure(%s) for IMS Setting ",
                          qmi_errstr(client_err));
          client_info.qmi_svc_clients[ QCRIL_QMI_CLIENT_IMS_SETTING ] = NULL;
        }
        else
        {
          imss_init = TRUE;
        }

        // QMI IMS SETTING command callback
        client_info.client_cbs[QCRIL_QMI_CLIENT_IMS_SETTING] = qcril_qmi_imss_command_cb;

        if ( qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID )
        {
          // Initialize the QMI PDC client
          client_err = qmi_client_init_instance(client_info.service_objects[QCRIL_QMI_CLIENT_PDC],
                                                qmi_master_port,
                                                qcril_qmi_pdc_unsol_ind_cb,
                                                NULL,
                                                &client_info.os_params[QCRIL_QMI_CLIENT_PDC],
                                                time_out,
                                                &client_info.qmi_svc_clients[QCRIL_QMI_CLIENT_PDC] );
          if (client_err)
          {
            // do not return failure if QMI PDC initialization fails
            QCRIL_LOG_ERROR("qmi_client_init returned failure(%s) for PDC",
                            qmi_errstr(client_err));
            client_info.qmi_svc_clients[ QCRIL_QMI_CLIENT_PDC ] = NULL;
          }
          else
          {
            client_info.client_state[QCRIL_QMI_CLIENT_PDC] = QCRIL_QMI_SERVICE_CONNECTED;
            pdc_init = TRUE;
          }
        }

        CLIENT_CACHE_LOCK();
        client_info.num_of_active_clients = 0;
        for ( idx = 0; idx < QCRIL_QMI_CLIENT_MAX; idx ++ )
        {
            if (client_info.client_state[idx] == QCRIL_QMI_SERVICE_CONNECTED)
            {
                client_err = qmi_client_register_error_cb(client_info.qmi_svc_clients[idx],
                                                          qcril_qmi_service_down_event,
                                                          (void*)(intptr_t)idx);
                client_info.num_of_active_clients++;
            }
        }

        client_info.max_active_clients       = client_info.num_of_active_clients;
        client_info.qmi_client_init_complete = TRUE;
        QCRIL_LOG_INFO("Max active clients %d", client_info.max_active_clients);
        CLIENT_CACHE_UNLOCK();

        QCRIL_LOG_INFO("Initing Voice client...");
        res = qcril_qmi_voice_init();

        QCRIL_LOG_INFO("Initing DMS client...");
        qcril_qmi_dms_init();

        QCRIL_LOG_INFO("Initing NAS client...");
        qcril_qmi_nas_init();

        QCRIL_LOG_INFO("Initing PBM client...");
        qcril_qmi_pbm_init();

        QCRIL_LOG_INFO("Initing SMS client...");
        client_err = qcril_qmi_sms_init();
        if (client_err)
        {
          QCRIL_LOG_INFO("qcril_qmi_sms_init returned failure(%d)",client_err);
          res = RIL_E_GENERIC_FAILURE;
          break;
        }

        // IMS_Presense
        if (imsp_init)
        {
          QCRIL_LOG_INFO("Initing IMS client...");
          qcril_qmi_ims_init();
        }

        if (imss_init)
        {
          QCRIL_LOG_INFO("Initing IMSS client...");
          qcril_qmi_imss_init();
        }

        // IMSA
        qcril_qmi_init_imsa_client_handles();

        // PDC
        if (pdc_init)
        {
          QCRIL_LOG_INFO("Initing PDC client...");
          client_err =  qcril_qmi_pdc_init();
          if (client_err)
          {
            QCRIL_LOG_INFO("qcril_qmi_pdc_init returned failure(%d)", client_err);
            res = RIL_E_GENERIC_FAILURE;
          }
          else
          {
#ifndef QMI_RIL_UTF
            qcril_qmi_pdc_retrieve_current_mbn_info();
#endif
          }
        }

        // COEX
        if ( QCRIL_IS_COEX_ENABLED() )
        {
          QCRIL_LOG_INFO("Initing COEX client...");
          qcril_qmi_coex_init();
        }

        // Bind to subsciption based on instance id as of now.
        // This will be adjusted as per app selected while processing
        // set_uicc_subsription.
        if ( qmi_ril_is_multi_sim_feature_supported() )
        {
            QCRIL_LOG_INFO( "[MSIM]..invoking bind for subscription %d", (int) stack_id );
            bind_res = qcril_qmi_client_dsds_bind_to_subscription( stack_id );
            QCRIL_LOG_INFO( ".. bind res %d", (int) bind_res );
            qcril_qmi_nas_multi_sim_init();
        }
        else
        {
            qcril_qmi_nas_get_device_capability(TRUE, TRUE);
        }

        qcril_qmi_nas_update_embms_status();
        qcril_qmi_nas_embms_send_embms_status(qcril_qmi_nas_get_embms_status());

        //As there is a chance that we may miss VSID info in SUBSCRIPTION_INFO_IND
        //we need to read SUBSCRIPTION_INFO at boot-up to be on the safer side.
        QCRIL_LOG_INFO( "..read subscription info");
        qcril_qmi_nas_get_subscription_info();

        qcril_qmi_cb_thread_name_init();

  } while (FALSE);


  if ( RIL_E_SUCCESS == res )
  {
    if (qcril_qmi_nas_modem_power_is_mdm_shdn_in_apm())
    {
      qcril_qmi_nas_modem_power_ril_resumed();
    }
    QCRIL_LOG_INFO( "Client init successful " );
  }
  else if ( log_failed_qmi_svc )
  {
    QCRIL_LOG_ESSENTIAL( "Client init failed, details: %s", log_failed_qmi_svc);
  }
  else
  {
    QCRIL_LOG_ESSENTIAL( "Post QMI client initialization failure");
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);

  return res;

} // qcril_qmi_init_core_client_handles


//===========================================================================
// qcril_qmi_client_destroy_mutex
//===========================================================================
void qcril_qmi_client_destroy_mutex
(
  void
)
{
    pthread_mutex_destroy(&client_info.cache_lock_mutex);
    pthread_mutexattr_destroy(&client_info.cache_lock_mtx_atr);
}// qcril_qmi_client_destroy_mutex

//===========================================================================
//qcril_qmi_client_release
//===========================================================================
void qcril_qmi_client_release
(
  void
)
{
  qcril_qmi_release_client_handles();
  qcril_qmi_client_destroy_mutex();

  qcril_qmi_nas_dms_commost_post_cleanup();
  qcril_qmi_voice_post_cleanup();
#ifndef QMI_RIL_UTF
  qcril_am_post_cleanup();
#endif
  qcril_qmi_sms_destroy();

}// qcril_qmi_client_release

//===========================================================================
//qcril_qmi_release_client_handles
//===========================================================================
void qcril_qmi_release_client_handles(void)
{
   int idx;
   QCRIL_LOG_FUNC_ENTRY();
   if (client_info.qmi_client_init_complete)
   {
      client_info.qmi_client_init_complete = FALSE;

      qcril_qmi_nas_cleanup();
      qcril_qmi_voice_cleanup();
   }

   for (idx = QCRIL_QMI_CLIENT_FIRST; idx < QCRIL_QMI_CLIENT_LAST; idx++)
   {
      if(client_info.qmi_svc_clients[idx])
      {
         qmi_client_release(client_info.qmi_svc_clients[idx]);
         client_info.qmi_svc_clients[idx] = NULL;
      }
   }

   if ( QCRIL_IS_COEX_ENABLED() )
   {
      qcril_qmi_coex_release();
   }

   cri_core_cri_client_release();
   QCRIL_LOG_FUNC_RETURN();

}// qcril_qmi_release_client_handles


/*=========================================================================
  FUNCTION:  qcril_qmi_client_map_qmi_err_to_ril_err

===========================================================================*/
/*!
    @brief
    Map QMI error to RIL error.

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_client_map_qmi_err_to_ril_err
(
  qmi_error_type_v01 qmi_err
)
{
  RIL_Errno ril_err;

  switch(qmi_err)
  {
    case QMI_ERR_NONE_V01:
      ril_err = RIL_E_SUCCESS;
      break;
    case QMI_ERR_FDN_RESTRICT_V01:
      ril_err = RIL_E_FDN_CHECK_FAILURE;
      break;
    default:
      ril_err = RIL_E_GENERIC_FAILURE;
      break;
  }
  return ril_err;
}/* qcril_qmi_client_map_qmi_err_to_ril_err */


/*=========================================================================
  FUNCTION:  qcril_qmi_client_send_msg_async

===========================================================================*/
/*!
    @brief
    Send QMI command asynchronously.

    @return
    None.
*/
/*=========================================================================*/
errno_enum_type qcril_qmi_client_send_msg_async
(
  qcril_qmi_client_e_type      svc_type,
  unsigned long                msg_id,
  void                         *req_c_struct,
  int                          req_c_struct_len,
  void                         *resp_c_struct,
  int                          resp_c_struct_len,
  void                         *resp_cb_data
)
{
  qmi_client_error_type qmi_error;
  errno_enum_type ret = E_FAILURE;
  qmi_txn_handle txn_handle;

 /* check if the client is initialized already */

  if( svc_type < QCRIL_QMI_CLIENT_LAST )
  {
    if (NULL != client_info.qmi_svc_clients[svc_type])
    {
      qmi_error =  qmi_client_send_msg_async_with_shm(client_info.qmi_svc_clients[svc_type],
                                      msg_id,
                                      req_c_struct,
                                      req_c_struct_len,
                                      resp_c_struct,
                                      resp_c_struct_len,
                                      client_info.client_cbs[svc_type],
                                      resp_cb_data,
                                      &txn_handle);
      if (qmi_error!= QMI_NO_ERR )
      {
        QCRIL_LOG_ERROR("Failed to send async qmi msg 0x%02x w/%s",
                         msg_id, qmi_errstr(qmi_error));
      }
      else
      {
        ret = E_SUCCESS;
      }
    }
    else
    {
       QCRIL_LOG_INFO("svc %d is not initialized", (int) svc_type);
    }
  }
  return ret;
} /* qcril_qmi_client_send_msg_async */

/*=========================================================================
  FUNCTION:  qcril_qmi_client_send_msg_async_ex

===========================================================================*/
/*!
    @brief
    Send QMI command asynchronously.

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_client_send_msg_async_ex
(
  qcril_qmi_client_e_type      svc_type,
  unsigned long                msg_id,
  void                         *req_c_struct,
  int                          req_c_struct_len,
  void                         *resp_c_struct,
  int                          resp_c_struct_len,
  void                         *resp_cb_data
)
{
  qmi_txn_handle txn_handle;

  RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;
  qmi_client_error_type qmi_transport_error;

 /* check if the client is initialized already */

  if( svc_type >= QCRIL_QMI_CLIENT_LAST )
  {
    QCRIL_LOG_ERROR("Invalid service %d, context msg 0x%x", svc_type, msg_id);
  }
  else
  {
    if (NULL != client_info.qmi_svc_clients[svc_type])
    {
      qmi_transport_error =  qmi_client_send_msg_async_with_shm(
                                      client_info.qmi_svc_clients[svc_type],
                                      msg_id,
                                      req_c_struct,
                                      req_c_struct_len,
                                      resp_c_struct,
                                      resp_c_struct_len,
                                      client_info.client_cbs[svc_type],
                                      resp_cb_data,
                                      &txn_handle);
      ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_transport_error, NULL);
      if ( RIL_E_SUCCESS != ril_err )
      {
        QCRIL_LOG_ERROR("error %d / %d, context msg hex %x, service %d ",ril_err, qmi_transport_error, msg_id, svc_type );
      }
    }
    else
    {
       QCRIL_LOG_INFO("svc %d is not initialized", (int) svc_type);
    }
  }

  return ril_err;
} /* qcril_qmi_client_send_msg_async_ex */


/*=========================================================================
  FUNCTION:  qcril_qmi_client_send_msg_sync

===========================================================================*/
/*!
    @brief
    Callback to handle CM Phone command callback.

    @return
    None.
*/
/*=========================================================================*/
errno_enum_type qcril_qmi_client_send_msg_sync
(
  qcril_qmi_client_e_type      svc_type,
  unsigned long                msg_id,
  void                         *req_c_struct,
  int                          req_c_struct_len,
  void                         *resp_c_struct,
  int                          resp_c_struct_len
)
{
  qmi_client_error_type qmi_error;
  errno_enum_type ret = E_FAILURE;

  if( svc_type < QCRIL_QMI_CLIENT_LAST )
  {
    if (NULL != client_info.qmi_svc_clients[svc_type])
    {
      qmi_error = qmi_client_send_msg_sync_with_shm(client_info.qmi_svc_clients[svc_type],
                              msg_id,
                              req_c_struct,
                              req_c_struct_len,
                              resp_c_struct,
                              resp_c_struct_len,
                              QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT
                               );

      if (qmi_error!= QMI_NO_ERR )
      {
        QCRIL_LOG_ERROR("Failed to send sync qmi msg 0x%02x w/%s",
                         msg_id, qmi_errstr(qmi_error));
      }
      else
      {
        ret = E_SUCCESS;
      }
    }
    else
    {
       QCRIL_LOG_INFO("svc %d is not initialized", (int) svc_type);
    }
  }
  return ret;
} /* qcril_qmi_client_send_msg_sync */

/*=========================================================================
  FUNCTION:  qcril_qmi_client_send_msg_sync_ex

===========================================================================*/
/*!
    @brief
    Callback to handle CM Phone command callback.

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_client_send_msg_sync_ex
(
  qcril_qmi_client_e_type      svc_type,
  unsigned long                msg_id,
  void                         *req_c_struct,
  int                          req_c_struct_len,
  void                         *resp_c_struct,
  int                          resp_c_struct_len,
  int                          timeout_msecs
)
{
  RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;
  qmi_client_error_type qmi_transport_error;

  qmi_response_type_v01* resp_err = NULL;

  if( svc_type >= QCRIL_QMI_CLIENT_LAST )
  {
    QCRIL_LOG_ERROR("Invalid service %d, context msg 0x%x", svc_type, msg_id);
    ril_err = RIL_E_GENERIC_FAILURE;
  }
  else
  {
    if (NULL != client_info.qmi_svc_clients[svc_type])
    {
      qmi_transport_error = qmi_client_send_msg_sync_with_shm(
                              qcril_qmi_client_get_user_handle( svc_type ),
                              msg_id,
                              req_c_struct,
                              req_c_struct_len,
                              resp_c_struct,
                              resp_c_struct_len,
                              timeout_msecs
                               );

      if ( QCRIL_QMI_CLIENT_WMS == svc_type )
      { //Need to properly locate the resp field of specific QMI WMS response messages when the response message does not start with the resp field
        switch(msg_id)
        {
          case QMI_WMS_RAW_WRITE_REQ_V01:
            resp_err = &((wms_raw_write_resp_msg_v01*)resp_c_struct)->resp;
            break;

          case QMI_WMS_RAW_SEND_REQ_V01:
            resp_err = &((wms_raw_send_resp_msg_v01*)resp_c_struct)->resp;
            break;

          case QMI_WMS_RAW_READ_REQ_V01:
            resp_err = &((wms_raw_read_resp_msg_v01*)resp_c_struct)->resp;
            break;

          case QMI_WMS_GET_MESSAGE_PROTOCOL_REQ_V01:
            resp_err = &((wms_get_message_protocol_resp_msg_v01*)resp_c_struct)->resp;
            break;

          case QMI_WMS_LIST_MESSAGES_REQ_V01:
            resp_err = &((wms_list_messages_resp_msg_v01*)resp_c_struct)->resp;
            break;

          case QMI_WMS_GET_ROUTES_REQ_V01:
            resp_err = &((wms_get_routes_resp_msg_v01*)resp_c_struct)->resp;
            break;

          case QMI_WMS_GET_SMSC_ADDRESS_REQ_V01:
            resp_err = &((wms_get_smsc_address_resp_msg_v01*)resp_c_struct)->resp;
            break;

          case QMI_WMS_GET_STORE_MAX_SIZE_REQ_V01:
            resp_err = &((wms_get_store_max_size_resp_msg_v01*)resp_c_struct)->resp;
            break;

          case QMI_WMS_GET_DOMAIN_PREF_REQ_V01:
            resp_err = &((wms_get_domain_pref_resp_msg_v01*)resp_c_struct)->resp;
            break;

          case QMI_WMS_GET_MESSAGE_WAITING_REQ_V01:
            resp_err = &((wms_get_message_waiting_resp_msg_v01*)resp_c_struct)->resp;
            break;

          default:
            resp_err = (qmi_response_type_v01*)resp_c_struct;
            break;
        }
      }
      else
      {
        resp_err = (qmi_response_type_v01*)resp_c_struct;
      }
      ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_transport_error, resp_err );

      if ( RIL_E_SUCCESS != ril_err )
      {
        QCRIL_LOG_ERROR("error %d / %d / %d / %d, context msg hex %x, service %d ",
                       ril_err,
                       qmi_transport_error,
                       resp_err->result,
                       resp_err->error,
                       msg_id,
                       svc_type );
      }
    }
    else
    {
      QCRIL_LOG_INFO("svc %d is not initialized", (int) svc_type);
    }
  }

  return ril_err;

} /* qcril_qmi_client_send_msg_sync_ex */


/*=========================================================================
  FUNCTION:  qcril_qmi_client_get_service_object

===========================================================================*/
/*!
    @brief
    Get the service object for the specific service type

    @return
    None.
*/
/*=========================================================================*/
qmi_idl_service_object_type qcril_qmi_client_get_service_object
(
  qcril_qmi_client_e_type        svc_type
)
{
  qmi_idl_service_object_type svc_obj = NULL;

  if( svc_type < QCRIL_QMI_CLIENT_LAST )
  {
    svc_obj = client_info.service_objects[svc_type];
  }
  return svc_obj;
}/* qcril_qmi_client_get_service_object */


/*=========================================================================
  FUNCTION:  qcril_qmi_client_get_user_handle

===========================================================================*/
/*!
    @brief
    Get the user handle for the specific service type

    @return
    None.
*/
/*=========================================================================*/
qmi_client_type qcril_qmi_client_get_user_handle
(
  qcril_qmi_client_e_type        svc_type
)
{
  qmi_client_type clnt_type = NULL;

  if( svc_type < QCRIL_QMI_CLIENT_LAST )
  {
    clnt_type = client_info.qmi_svc_clients[svc_type];
  }
  return clnt_type;
}/* qcril_qmi_client_get_user_handle */


qmi_client_type qcril_qmi_client_get_client_handle
(
  qcril_qmi_client_e_type client
)
{
  return client_info.qmi_svc_clients[client];
}
//===========================================================================
// qcril_qmi_util_convert_qmi_response_codes_to_ril_result
//===========================================================================
RIL_Errno qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_client_error_type qmi_transport_error, qmi_response_type_v01* qmi_service_response)
{
    return qcril_qmi_util_convert_qmi_response_codes_to_ril_result_ex(qmi_transport_error, qmi_service_response, QCRIL_QMI_ERR_CTX_NONE, NULL);
}
//===========================================================================
// qcril_qmi_util_convert_qmi_response_codes_to_ril_result_ex
//===========================================================================
RIL_Errno qcril_qmi_util_convert_qmi_response_codes_to_ril_result_ex(qmi_client_error_type qmi_transport_error,
                                                                     qmi_response_type_v01* qmi_service_response,
                                                                     qmi_ril_err_context_e_type context,
                                                                     void* any)
{
    RIL_Errno res;

    int res_settled;

    voice_dial_call_resp_msg_v02* dial_call_resp_msg;
    voice_orig_ussd_resp_msg_v02* ussd_resp_msg;
    qmi_ril_err_ctx_ss_resp_data_type* ss_resp_info;

    switch ( qmi_transport_error )
    {
        case QMI_NO_ERR:                // fallthrough
        case QMI_SERVICE_ERR:
            res_settled = FALSE;

            QCRIL_LOG_INFO("ctx check %d", (int) context );
            switch (context)
            {
                case QCRIL_QMI_ERR_CTX_DIAL_TXN:
                    dial_call_resp_msg    = (voice_dial_call_resp_msg_v02*)any;
                    QCRIL_LOG_INFO("ctx dial %p", dial_call_resp_msg );
                    if ( dial_call_resp_msg && dial_call_resp_msg->cc_result_type_valid )
                    {
                        QCRIL_LOG_INFO("ctx dial cc result type %d", (int) dial_call_resp_msg->cc_result_type );
                        switch ( dial_call_resp_msg->cc_result_type )
                        {
                            case VOICE_CC_RESULT_TYPE_VOICE_V02:
                                res = RIL_E_DIAL_MODIFIED_TO_DIAL;
                                res_settled = TRUE;
                                break;

                            case VOICE_CC_RESULT_TYPE_SUPS_V02:
                                res = RIL_E_DIAL_MODIFIED_TO_SS;
                                res_settled = TRUE;
                                break;

                            case VOICE_CC_RESULT_TYPE_USSD_V02:
                                res = RIL_E_DIAL_MODIFIED_TO_USSD;
                                res_settled = TRUE;
                                break;

                            default:
                                // skip
                                break;
                        }
                    }
                    break;

                case QCRIL_QMI_ERR_CTX_SEND_SS_TXN:
                    ss_resp_info          = (qmi_ril_err_ctx_ss_resp_data_type*)any;
                    if ( ss_resp_info && ss_resp_info->cc_result_type_valid )
                    {
                        switch ( *ss_resp_info->cc_result_type )
                        {
                            case VOICE_CC_RESULT_TYPE_VOICE_V02:
                                res = RIL_E_SS_MODIFIED_TO_DIAL;
                                res_settled = TRUE;
                                break;

                            case VOICE_CC_RESULT_TYPE_SUPS_V02:
                                res = RIL_E_SS_MODIFIED_TO_SS;
                                res_settled = TRUE;
                                break;

                            case VOICE_CC_RESULT_TYPE_USSD_V02:
                                res = RIL_E_SS_MODIFIED_TO_USSD;
                                res_settled = TRUE;
                                break;

                            default:
                                // skip
                                break;
                        }
                    }
                    break;

                case QCRIL_QMI_ERR_CTX_SEND_USSD_TXN:
                    ussd_resp_msg         = (voice_orig_ussd_resp_msg_v02*)any;
                    if ( ussd_resp_msg && ussd_resp_msg->cc_result_type_valid )
                    {
                        switch ( ussd_resp_msg->cc_result_type )
                        {
                            case VOICE_CC_RESULT_TYPE_VOICE_V02:
                                res = RIL_E_USSD_MODIFIED_TO_DIAL;
                                res_settled = TRUE;
                                break;

                            case VOICE_CC_RESULT_TYPE_SUPS_V02:
                                res = RIL_E_USSD_MODIFIED_TO_SS;
                                res_settled = TRUE;
                                break;

                            case VOICE_CC_RESULT_TYPE_USSD_V02:
                                res = RIL_E_USSD_MODIFIED_TO_USSD;
                                res_settled = TRUE;
                                break;

                            default:
                                // skip
                                break;
                        }
                    }
                    break;

                default:
                    // skip
                    break;
            }

            if ( !res_settled )
            {

                if ( NULL == qmi_service_response )
                {
                    res = RIL_E_SUCCESS;
                }
                else
                {
                    switch ( qmi_service_response->result )
                    {
                        case QMI_RESULT_SUCCESS_V01:
                            res = RIL_E_SUCCESS;
                            break;

                        case QMI_RESULT_FAILURE_V01:
                            if ( qmi_service_response )
                            {
                                switch ( qmi_service_response->error )
                                {
                                    case QMI_ERR_NONE_V01:
                                    case QMI_ERR_NO_EFFECT_V01:
                                        res = RIL_E_SUCCESS;
                                        break;

                                    case QMI_ERR_MALFORMED_MSG_V01:             // fallthough
                                    case QMI_ERR_NO_MEMORY_V01:                 // fallthough
                                    case QMI_ERR_INTERNAL_V01:                  // fallthough
                                    case QMI_ERR_CLIENT_IDS_EXHAUSTED_V01:
                                    case QMI_ERR_UNABORTABLE_TRANSACTION_V01:
                                    case QMI_ERR_INVALID_CLIENT_ID_V01:
                                    case QMI_ERR_INVALID_HANDLE_V01:
                                    case QMI_ERR_INVALID_PROFILE_V01:
                                    case QMI_ERR_NO_NETWORK_FOUND_V01:
                                    case QMI_ERR_OUT_OF_CALL_V01:
                                    case QMI_ERR_NOT_PROVISIONED_V01:
                                    case QMI_ERR_MISSING_ARG_V01:
                                    case QMI_ERR_ARG_TOO_LONG_V01:
                                    case QMI_ERR_INVALID_TX_ID_V01:
                                    case QMI_ERR_DEVICE_IN_USE_V01:
                                    case QMI_ERR_OP_DEVICE_UNSUPPORTED_V01:
                                    case QMI_ERR_NO_FREE_PROFILE_V01:
                                    case QMI_ERR_INVALID_PDP_TYPE_V01:
                                    case QMI_ERR_INVALID_TECH_PREF_V01:
                                        res = RIL_E_GENERIC_FAILURE;
                                        break;

                                    case QMI_ERR_OP_NETWORK_UNSUPPORTED_V01:
                                        res = RIL_E_MODE_NOT_SUPPORTED;
                                        break;

                                    case QMI_ERR_ABORTED_V01:
                                        res = RIL_E_CANCELLED;
                                        break;

                                    case QMI_ERR_FDN_RESTRICT_V01:
                                        res = RIL_E_FDN_CHECK_FAILURE;
                                        break;

                                    case QMI_ERR_INFO_UNAVAILABLE_V01:
                                        switch (context)
                                        {
                                            case QCRIL_QMI_ERR_TOLERATE_NOT_FOUND:
                                                res = RIL_E_SUCCESS;
                                                break;

                                            default:
                                                res = RIL_E_GENERIC_FAILURE;
                                                break;
                                        }
                                        break;

                                    case QMI_ERR_DEVICE_NOT_READY_V01:
                                    {
                                        switch (context)
                                        {
                                            case QCRIL_QMI_ERR_TOLERATE_NOT_READY:
                                                res = RIL_E_SUCCESS;
                                                break;

                                            default:
                                                res = RIL_E_GENERIC_FAILURE;
                                                break;
                                        }

                                        break;
                                    }

                                    default:
                                        res = RIL_E_GENERIC_FAILURE;
                                        break;
                                }
                            }
                            else
                            {
                                res = RIL_E_GENERIC_FAILURE;
                            }
                            break;

                        default:
                            res = RIL_E_GENERIC_FAILURE;
                            break;
                    }
                }
            }
            break;

        case QMI_INTERNAL_ERR:
            res = RIL_E_GENERIC_FAILURE;
            break;

        default:
            res = RIL_E_GENERIC_FAILURE;
            break;
    }

    return res;
} // qcril_qmi_util_convert_qmi_response_codes_to_ril_result_ex
//===========================================================================
// qcril_qmi_client_dsds_bind_to_subscription
//===========================================================================
RIL_Errno qcril_qmi_client_dsds_bind_to_subscription( RIL_SubscriptionType sub_num )
{
  RIL_Errno res;
  RIL_Errno res1;

  char property_name[ PROPERTY_VALUE_MAX ];
  char property_value[ PROPERTY_VALUE_MAX ];
  errno_enum_type result;

  nas_bind_subscription_req_msg_v01 nas_bind_request;
  nas_bind_subscription_resp_msg_v01 nas_bind_resp;
  wms_bind_subscription_req_msg_v01 wms_bind_request;
  wms_bind_subscription_resp_msg_v01 wms_bind_resp;
  pbm_bind_subscription_req_msg_v01 pbm_bind_request;
  pbm_bind_subscription_resp_msg_v01 pbm_bind_resp;
  voice_bind_subscription_req_msg_v02 voice_bind_request;
  voice_bind_subscription_resp_msg_v02 voice_bind_resp;
  dms_bind_subscription_req_msg_v01 dms_bind_request;
  dms_bind_subscription_resp_msg_v01 dms_bind_resp;

  memset( &nas_bind_request, 0, sizeof( nas_bind_request ) );
  memset( &wms_bind_request, 0, sizeof( wms_bind_request ) );
  memset( &pbm_bind_request, 0, sizeof( pbm_bind_request ) );
  memset( &voice_bind_request, 0, sizeof( voice_bind_request ) );
  memset( &dms_bind_request, 0, sizeof( dms_bind_request ) );

  QCRIL_LOG_INFO( "sub_num: %d", (int) sub_num );

  if (RIL_SUBSCRIPTION_3 == sub_num)
  {
    nas_bind_request.subs_type = NAS_TERTIARY_SUBSCRIPTION_V01;
    pbm_bind_request.subs_type = PBM_SUBS_TYPE_TERTIARY_V01;
    voice_bind_request.subs_type = VOICE_SUBS_TYPE_TERTIARY_V02;
    wms_bind_request.subs_type = WMS_TERTIARY_SUBSCRIPTION_V01;
    dms_bind_request.bind_subs = DMS_TERTIARY_SUBS_V01;
  }
  else if (RIL_SUBSCRIPTION_2 == sub_num)
  {
    nas_bind_request.subs_type = NAS_SECONDARY_SUBSCRIPTION_V01;
    pbm_bind_request.subs_type = PBM_SUBS_TYPE_SECONDARY_V01;
    voice_bind_request.subs_type = VOICE_SUBS_TYPE_SECONDARY_V02;
    wms_bind_request.subs_type = WMS_SECONDARY_SUBSCRIPTION_V01;
    dms_bind_request.bind_subs = DMS_SECONDARY_SUBS_V01;
  }
  else
  {
    nas_bind_request.subs_type = NAS_PRIMARY_SUBSCRIPTION_V01;
    pbm_bind_request.subs_type = PBM_SUBS_TYPE_PRIMARY_V01;
    voice_bind_request.subs_type = VOICE_SUBS_TYPE_PRIMARY_V02;
    wms_bind_request.subs_type = WMS_PRIMARY_SUBSCRIPTION_V01;
    dms_bind_request.bind_subs = DMS_PRIMARY_SUBS_V01;
  }

  // nas
  res =  qcril_qmi_client_send_msg_sync_ex( QCRIL_QMI_CLIENT_NAS,
                                             QMI_NAS_BIND_SUBSCRIPTION_REQ_MSG_V01,
                                             (void*)&nas_bind_request,
                                             sizeof( nas_bind_request ),
                                             (void*) &nas_bind_resp,
                                             sizeof( nas_bind_resp ),
                                             QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT
                                              );
  QCRIL_LOG_INFO( " ..nas %d", (int) res );

  // voice
  if ( RIL_E_SUCCESS == res )
  {
    res =  qcril_qmi_client_send_msg_sync_ex( QCRIL_QMI_CLIENT_VOICE,
                                               QMI_VOICE_BIND_SUBSCRIPTION_REQ_V02,
                                               &voice_bind_request,
                                               sizeof( voice_bind_request ),
                                               (void*) &voice_bind_resp,
                                               sizeof( voice_bind_resp ),
                                               QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT
                                                );
    QCRIL_LOG_INFO( " ..voice %d", (int) res );
  }

  // pbm
  if ( RIL_E_SUCCESS == res )
  {
    res =  qcril_qmi_client_send_msg_sync_ex( QCRIL_QMI_CLIENT_PBM,
                                               QMI_PBM_BIND_SUBSCRIPTION_REQ_V01,
                                               &pbm_bind_request,
                                               sizeof( pbm_bind_request ),
                                               (void*) &pbm_bind_resp,
                                               sizeof( pbm_bind_resp ),
                                               QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT
                                                );
    QCRIL_LOG_INFO( " ..pbm %d", (int) res );
  }

  // wms
  if ( RIL_E_SUCCESS == res )
  {
    res =  qcril_qmi_client_send_msg_sync_ex( QCRIL_QMI_CLIENT_WMS,
                                               QMI_WMS_BIND_SUBSCRIPTION_REQ_V01,
                                               &wms_bind_request,
                                               sizeof( wms_bind_request ),
                                               (void*) &wms_bind_resp,
                                               sizeof( wms_bind_resp ),
                                               QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT
                                                );
    QCRIL_LOG_INFO( " ..wms %d", (int) res );
  }

  // dms
  if ( RIL_E_SUCCESS == res )
  {
    res1 =  qcril_qmi_client_send_msg_sync_ex( QCRIL_QMI_CLIENT_DMS,
                                               QMI_DMS_BIND_SUBSCRIPTION_REQ_V01,
                                               (void*)&dms_bind_request,
                                               sizeof( dms_bind_request ),
                                               (void*) &dms_bind_resp,
                                               sizeof( dms_bind_resp ),
                                               QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT
                                                );
    QCRIL_LOG_INFO( " ..dms %d", (int) res1 );
  }

  if ( RIL_E_SUCCESS == res )
  {
      qcril_qmi_nas_update_modem_stack_id((uint8_t)sub_num);

      QCRIL_SNPRINTF( property_name, sizeof(property_name), "%s%d", QMI_RIL_MULTI_SIM_STACK_ID, qmi_ril_get_process_instance_id() );
      QCRIL_SNPRINTF( property_value, sizeof(property_value), "%d", (int) sub_num);
      result = property_set(property_name, property_value);
      QCRIL_LOG_INFO( ".. [MSIM] stack id prop %s - %s, %d", property_name, property_value, (int) result );
      if ( E_SUCCESS != result)
      {
          QCRIL_LOG_ERROR( "Failed to set stack prop!");
      }
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);

  return res;
}  //  qcril_qmi_client_dsds_bind_to_subscription

RIL_Errno qcril_qmi_client_dsds_cri_client_reinit( RIL_SubscriptionType sub_num )
{
  RIL_Errno res = RIL_E_SUCCESS;
  int num_of_services = 0;
  int do_not_init_csvt = 0;
  cri_core_cri_client_init_info_type cri_core_cri_client_init_info;

  memset(&cri_core_cri_client_init_info, 0, sizeof(cri_core_cri_client_init_info));

  qmi_ril_get_property_value_from_integer(QMI_RIL_DO_NOT_INIT_CSVT,
                                          &do_not_init_csvt,
                                          FALSE);
  if(FALSE == do_not_init_csvt)
  {
    cri_core_cri_client_init_info.service_info[num_of_services].cri_service_id = QMI_CSVT_SERVICE;
    num_of_services++;
  }

  cri_core_cri_client_init_info.number_of_cri_services_to_be_initialized = num_of_services;
  cri_core_cri_client_init_info.subscription_id = sub_num +
                                                  CRI_CORE_PRIMARY_CRI_SUBSCRIPTION_ID;
  cri_core_cri_client_reinit(&cri_core_cri_client_init_info);

  return res;
}

//===========================================================================
// qcril_qmi_client_dsds_cri_client_reset
//===========================================================================
RIL_Errno qcril_qmi_client_dsds_cri_client_reset()
{
  RIL_Errno res = RIL_E_SUCCESS;
  int num_of_services = 0;
  int do_not_init_csvt = 0;
  cri_core_cri_client_init_info_type cri_core_cri_client_init_info;

  memset(&cri_core_cri_client_init_info, 0, sizeof(cri_core_cri_client_init_info));

  qmi_ril_get_property_value_from_integer(QMI_RIL_DO_NOT_INIT_CSVT,
                                          &do_not_init_csvt,
                                          FALSE);
  if(FALSE == do_not_init_csvt)
  {
    cri_core_cri_client_init_info.service_info[num_of_services].cri_service_id = QMI_CSVT_SERVICE;
    num_of_services++;
  }

  cri_core_cri_client_init_info.number_of_cri_services_to_be_initialized = num_of_services;
  cri_core_cri_client_reset(&cri_core_cri_client_init_info);

  return res;
}  // qcril_qmi_client_dsds_cri_client_reset

//===========================================================================
// qcril_qmi_client_is_available
//===========================================================================
int qcril_qmi_client_is_available(void)
{
  return client_info.qmi_client_init_complete;
} // qcril_qmi_client_is_available

//===========================================================================
// qmi_ril_next_suspending_action
//===========================================================================
void qmi_ril_next_suspending_action(void)
{
    qcril_evt_e_type evt_to_deal_with;

    QCRIL_LOG_FUNC_ENTRY();

    evt_to_deal_with = qmi_ril_modem_reset_suspend_evt_cycle[ qmi_ril_modem_reset_round_info.transition_index ];
    QCRIL_LOG_INFO( " ..evt to deal with %d", (int) evt_to_deal_with );

    if ( QCRIL_EVT_NONE == evt_to_deal_with )
    { // suspend done
        qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID,
                                QCRIL_DEFAULT_MODEM_ID,
                                qmi_ril_suspended_main_threaded,
                                NULL,  // immediate
                                NULL );
    }
    else
    { // suspending - next step
        qmi_ril_modem_reset_round_info.transition_index++;
        qcril_event_queue( qmi_ril_get_process_instance_id(),
                           QCRIL_DEFAULT_MODEM_ID,
                           QCRIL_DATA_ON_STACK,
                           evt_to_deal_with,
                           NULL,
                           QMI_RIL_ZERO,
                           (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
    }
} // qmi_ril_next_suspending_action
//===========================================================================
// qmi_ril_suspended_main_threaded
//===========================================================================
void qmi_ril_suspended_main_threaded(void * param)
{
    qcril_modem_restart_con_type outcome;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( param );

    qmi_ril_fw_android_request_flow_control_drop_legacy_book_records( FALSE, FALSE );
    qmi_ril_fw_android_request_flow_control_abandon_all_requests_main_thrd( RIL_E_CANCELLED, FALSE );

    QCRIL_LOG_INFO( "QMI RIL suspended" );
    qmi_ril_set_operational_status( QMI_RIL_GEN_OPERATIONAL_STATUS_SUSPENDED );

    if (!qcril_qmi_nas_modem_power_is_mdm_shdn_in_apm() ||
          1 == qcril_qmi_modem_power_voting_state())
    {
        qcril_qmi_register_for_up_event();
    }
    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_suspended_main_threaded

//===========================================================================
// qmi_ril_next_suspending_action
//===========================================================================
void qmi_ril_suspending_con_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    qcril_modem_restart_con_type * confim_details;

    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_NOTUSED(ret_ptr);

    if(params_ptr != NULL && params_ptr->data != NULL)
    {
    QCRIL_LOG_INFO( ".. event %d", params_ptr->event_id );

    confim_details = (qcril_modem_restart_con_type *)params_ptr->data;

    QCRIL_LOG_INFO( ".. details %d / %d ", (int)confim_details->evt_originator, (int)confim_details->result );
    if ( RIL_E_SUCCESS == confim_details->result )
    {
        qmi_ril_next_suspending_action();
    }
    else
    {
        QCRIL_LOG_INFO( "SUSPENDING FAILED, RIL halted" );
        qmi_ril_set_operational_status( QMI_RIL_GEN_OPERATIONAL_STATUS_HALTED );
    }
    }
    else
    {
        QCRIL_LOG_FATAL("FATAL : CHECK FAILED");
    }

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_suspending_con_handler
//===========================================================================
// qmi_ril_core_pre_suspend_handler
//===========================================================================
void qmi_ril_core_pre_suspend_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    qcril_modem_restart_con_type outcome;

    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_NOTUSED( params_ptr );
    QCRIL_NOTUSED(ret_ptr);

    // cleanup NAS
    qcril_qmi_nas_cleanup();
    // clean up voice

    memset( &outcome, 0, sizeof(outcome) );
    outcome.evt_originator = QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_SUSPEND_REQ;
    outcome.result         = RIL_E_SUCCESS;
    qcril_modem_restart_confirm_suspend_resume_step( QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_SUSPEND_CON, &outcome );

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_core_pre_suspend_handler
//===========================================================================
// qmi_ril_core_final_suspend_handler
//===========================================================================
void qmi_ril_core_final_suspend_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( params_ptr );
    QCRIL_NOTUSED(ret_ptr);

    qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID,
                            QCRIL_DEFAULT_MODEM_ID,
                            qmi_ril_final_suspend_main_threaded,
                            NULL,  // immediate
                            NULL );


    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_core_final_suspend_handler
//===========================================================================
// qmi_ril_final_suspend_main_threaded
//===========================================================================
void qmi_ril_final_suspend_main_threaded(void * param)
{
    qcril_modem_restart_con_type outcome;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( param );

    qmi_ril_clear_timed_callback_list();

    // clean up core clients
    qcril_qmi_voice_cleanup();
#ifndef QMI_RIL_UTF
    qcril_am_state_reset();
#endif

    qcril_qmi_release_client_handles();

    memset( &outcome, 0, sizeof(outcome) );
    outcome.evt_originator = QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_SUSPEND_REQ;
    outcome.result         = RIL_E_SUCCESS;
    qcril_modem_restart_confirm_suspend_resume_step( QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_SUSPEND_CON, &outcome );

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_final_suspend_main_threaded

//===========================================================================
// qcril_modem_restart_confirm_suspend_resume_step
//===========================================================================
void qcril_modem_restart_confirm_suspend_resume_step( qcril_evt_e_type confirmation_evt, qcril_modem_restart_con_type * details )
{

    QCRIL_LOG_INFO( "confirmation_evt: %d", (int) confirmation_evt );
    QCRIL_ASSERT( details != NULL );

    qcril_event_queue( qmi_ril_get_process_instance_id(),
                       QCRIL_DEFAULT_MODEM_ID,
                       QCRIL_DATA_ON_STACK,
                       confirmation_evt,
                       details,
                       sizeof( *details ),
                       (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );

} // qcril_modem_restart_confirm_suspend_resume_step
//===========================================================================
// qmi_ril_enter_suspend
//===========================================================================
void qmi_ril_enter_suspend(void)
{
  qmi_ril_gen_operational_status_type cur_state;
  qmi_ril_gen_operational_status_type next_state;

  QCRIL_LOG_FUNC_ENTRY();
  cur_state = qmi_ril_get_operational_status();
  QCRIL_LOG_INFO( " ..where we are %d", (int) cur_state );
  next_state = QMI_RIL_GEN_OPERATIONAL_STATUS_UNKNOWN;

  qmi_ril_qmi_client_pre_initialization_acquire();

  qmi_ril_suspend_init_work_handler();

  switch (cur_state)
  {
      case QMI_RIL_GEN_OPERATIONAL_STATUS_UNRESTRICTED:
      case QMI_RIL_GEN_OPERATIONAL_STATUS_INIT_ONGOING:
      case QMI_RIL_GEN_OPERATIONAL_STATUS_RESUMING:
      case QMI_RIL_GEN_OPERATIONAL_STATUS_UNBIND:
          next_state = QMI_RIL_GEN_OPERATIONAL_STATUS_SUSPENDING;
          break;

      case QMI_RIL_GEN_OPERATIONAL_STATUS_INIT_PENDING:
          // qmi_todo: suspend ini retry logic
          next_state = QMI_RIL_GEN_OPERATIONAL_STATUS_SUSPENDED;
          break;

      default:
          // fasttrack service
          next_state = QMI_RIL_GEN_OPERATIONAL_STATUS_SUSPENDED;
          break;
  }

  QCRIL_LOG_INFO( " ..proposed state %d", (int) next_state );
  if ( QMI_RIL_GEN_OPERATIONAL_STATUS_UNKNOWN != next_state )
  {
      qmi_ril_set_operational_status( next_state );
      if (!qcril_qmi_nas_modem_power_is_mdm_shdn_in_apm() ||
          1 == qcril_qmi_modem_power_voting_state())
      {
          qcril_qmi_nas_initiate_radio_state_changed_ind();
          qcril_qmi_nas_embms_send_radio_state(RADIO_STATE_NOT_AVAILABLE_V01);
      }

      switch ( next_state )
      {
          case QMI_RIL_GEN_OPERATIONAL_STATUS_SUSPENDING:
              qmi_ril_modem_reset_round_info.transition_index = QMI_RIL_ZERO;
              qmi_ril_next_suspending_action();
              break;

          default:
              // nothing
              break;
      }
  }
} // qmi_ril_enter_suspend

//===========================================================================
// qmi_ril_wave_modem_status
//===========================================================================
void qmi_ril_wave_modem_status(void)
{
    qcril_unsol_resp_params_type unsol_resp_params;

    qcril_default_unsol_resp_params( QCRIL_DEFAULT_INSTANCE_ID, RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, &unsol_resp_params );
    qcril_send_unsol_response( &unsol_resp_params );

    qcril_qmi_voice_ims_send_unsol_radio_state_change_helper();
}
//===========================================================================
// qmi_ril_enter_resume
//===========================================================================
void qmi_ril_enter_resume(void)
{
  qmi_ril_gen_operational_status_type cur_state;
  qmi_ril_gen_operational_status_type next_state;

  QCRIL_LOG_FUNC_ENTRY();
  cur_state = qmi_ril_get_operational_status();
  QCRIL_LOG_INFO( " ..where we are %d", (int) cur_state );

  // Reset pending unsol list if any
  qmi_ril_reset_android_unsol_resp_dispatchable_table();
  qmi_ril_reset_unsol_resp_pending_list();

  switch (cur_state)
  {
      case QMI_RIL_GEN_OPERATIONAL_STATUS_SUSPENDED:
          next_state = QMI_RIL_GEN_OPERATIONAL_STATUS_RESUME_PENDING;
          break;

      default:
          // no change
          next_state = QMI_RIL_GEN_OPERATIONAL_STATUS_UNKNOWN;
          break;
  }

  QCRIL_LOG_INFO( " ..proposed state %d", (int) next_state );
  if ( QMI_RIL_GEN_OPERATIONAL_STATUS_UNKNOWN != next_state )
  {
      qmi_ril_set_operational_status( next_state );

      switch ( next_state )
      {
          case QMI_RIL_GEN_OPERATIONAL_STATUS_RESUME_PENDING:
              qmi_ril_modem_reset_round_info.transition_index = QMI_RIL_ZERO;
              qmi_ril_next_resuming_action();
              break;

          default:
              // nothing
              break;
      }
  }
  else
  {
      qmi_ril_resumer_initiate();
  }
  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_enter_resume

//===========================================================================
// qmi_ril_resumer_initiate
//===========================================================================
RIL_Errno qmi_ril_resumer_initiate(void)
{
    RIL_Errno res;
    pthread_attr_t attr;
    pthread_t      thread_pid;
    int conf;

    QCRIL_LOG_FUNC_ENTRY();

#ifdef QMI_RIL_UTF
    pthread_attr_init (&attr);
    conf = utf_pthread_create_handler(&thread_pid, &attr, qmi_ril_resumer_deferred_action_thread_proc, NULL);
    pthread_attr_destroy(&attr);
#else
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    conf = pthread_create(&thread_pid, &attr, qmi_ril_resumer_deferred_action_thread_proc, NULL);
    pthread_attr_destroy(&attr);
#endif
    qmi_ril_set_thread_name(thread_pid, QMI_RIL_RESUMER_DEFERRED_ACTION_THREAD_NAME);
    QCRIL_LOG_INFO( ".. conf, pid %d, %d", (int)conf, (int) thread_pid );

    res =  (conf < 0 ) ? RIL_E_GENERIC_FAILURE : RIL_E_SUCCESS;

    QCRIL_LOG_FUNC_RETURN_WITH_RET(res);

    return res;
} // qmi_ril_resumer_initiate
//===========================================================================
// qmi_ril_resumer_deferred_action_thread_proc
//===========================================================================
void * qmi_ril_resumer_deferred_action_thread_proc(void * param)
{
    qmi_ril_gen_operational_status_type cur_state;

    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_NOTUSED(param);

    sleep( RESUMER_ACTION_DELAY );
    QCRIL_LOG_INFO( "awaken" );

    cur_state = qmi_ril_get_operational_status();
    QCRIL_LOG_INFO( " ..where we are %d", (int) cur_state );
    switch (cur_state)
    {
        case QMI_RIL_GEN_OPERATIONAL_STATUS_SUSPENDED:
            qmi_ril_set_operational_status( QMI_RIL_GEN_OPERATIONAL_STATUS_RESUME_PENDING );
            qmi_ril_modem_reset_round_info.transition_index = QMI_RIL_ZERO;
            qmi_ril_next_resuming_action();
            break;

        case QMI_RIL_GEN_OPERATIONAL_STATUS_RESUMING:       // fallthrough
        case QMI_RIL_GEN_OPERATIONAL_STATUS_UNRESTRICTED:
            // no action, no need for resume kick
            break;

        default:
            // we got to wait and see
            qmi_ril_resumer_initiate();
            break;
    }

    QCRIL_LOG_FUNC_RETURN();

    qmi_ril_clear_thread_name(pthread_self());
    return NULL;
} // qmi_ril_resumer_deferred_action_thread_proc

//===========================================================================
// qmi_ril_resume_retry_handler
//===========================================================================
void qmi_ril_voice_call_supress_handler(void * param)
{
    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( param );

    qmi_ril_ssr_voice_call_supress_timerid = QMI_RIL_ZERO;

    qmi_ril_set_supress_voice_calls( FALSE );

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_resume_retry_handler

//===========================================================================
// qmi_ril_next_suspending_action
//===========================================================================
void qmi_ril_next_resuming_action(void)
{
    qcril_evt_e_type evt_to_deal_with;

    QCRIL_LOG_FUNC_ENTRY();

    evt_to_deal_with = qmi_ril_modem_reset_resume_evt_cycle[ qmi_ril_modem_reset_round_info.transition_index ];
    QCRIL_LOG_INFO( " ..evt to deal with %d", (int) evt_to_deal_with );

    if ( QCRIL_EVT_NONE == evt_to_deal_with )
    { // resume done, retranslate action to main thread
      qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID,
                                  QCRIL_DEFAULT_MODEM_ID,
                                  qmi_ril_ssr_perform_final_post_ssr_init,
                                  NULL,  // immediate
                                  NULL );
    }
    else
    { // resume - next step
        qmi_ril_modem_reset_round_info.transition_index++;
        qcril_event_queue( qmi_ril_get_process_instance_id(),
                           QCRIL_DEFAULT_MODEM_ID,
                           QCRIL_DATA_ON_STACK,
                           evt_to_deal_with,
                           NULL,
                           QMI_RIL_ZERO,
                           (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
    }

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_next_resuming_action

//===========================================================================
// qmi_ril_ssr_perform_final_post_ssr_init
//===========================================================================
void qmi_ril_ssr_perform_final_post_ssr_init(void * param)
{
    const struct timeval supress_delay = { 30 , 0 }; // 30 seconds

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( param );

    QCRIL_LOG_INFO( "QMI RIL resumed" );

    qmi_ril_wave_modem_status(); // this should trigger reporting of modem state to Android
    qmi_ril_core_init_enter_warp( );

    if ( !qmi_ril_is_feature_supported( QMI_RIL_FEATURE_8960 ) &&
         !qmi_ril_is_feature_supported( QMI_RIL_FEATURE_8974 ) &&
         !qmi_ril_is_feature_supported( QMI_RIL_FEATURE_8610 ) &&
         !qmi_ril_is_feature_supported( QMI_RIL_FEATURE_8916) &&
         !qmi_ril_is_feature_supported( QMI_RIL_FEATURE_8909) &&
         !qmi_ril_is_feature_supported( QMI_RIL_FEATURE_8084 ) &&
         !qmi_ril_is_feature_supported( QMI_RIL_FEATURE_8226 ) &&
         !qmi_ril_is_feature_supported( QMI_RIL_FEATURE_8994 )
        )
    { // suppress voice call for prespecified time window
        qmi_ril_set_supress_voice_calls( TRUE );
        if ( qmi_ril_ssr_voice_call_supress_timerid != QMI_RIL_ZERO )
        {
            qcril_cancel_timed_callback((void*)(uintptr_t) qmi_ril_ssr_voice_call_supress_timerid);
            qmi_ril_ssr_voice_call_supress_timerid = QMI_RIL_ZERO;
        }
        qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID,
                                    QCRIL_DEFAULT_MODEM_ID,
                                    qmi_ril_voice_call_supress_handler,
                                    &supress_delay,
                                    &qmi_ril_ssr_voice_call_supress_timerid );
    }

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_ssr_perform_final_post_ssr_init


//===========================================================================
// qmi_ril_resuming_con_handler
//===========================================================================
void qmi_ril_resuming_con_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    qcril_modem_restart_con_type * confim_details;
    qmi_ril_gen_operational_status_type cur_state;

    const struct timeval retry_delay = { RESUMER_ACTION_DELAY , 0 };

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED(ret_ptr);

    if( params_ptr != NULL && params_ptr->data != NULL)
    {
        QCRIL_LOG_INFO( ".. event %d", params_ptr->event_id );

        confim_details = (qcril_modem_restart_con_type *)params_ptr->data;

        QCRIL_LOG_INFO( ".. details %d / %d ", (int)confim_details->evt_originator, (int)confim_details->result );
        cur_state = qmi_ril_get_operational_status();
        QCRIL_LOG_INFO( " ..where we are %d", (int) cur_state );

        if ( RIL_E_SUCCESS == confim_details->result )
        {
            qmi_ril_next_resuming_action();
        }
        else
        {
            QCRIL_LOG_ERROR( "RESUMING FAILED" );
            switch ( cur_state )
            {
                case QMI_RIL_GEN_OPERATIONAL_STATUS_RESUME_RETRY: // rollback, rollback and restart
                    QCRIL_LOG_INFO( "resume retry initiate" );
                    qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID,
                                                QCRIL_DEFAULT_MODEM_ID,
                                                qmi_ril_resume_retry_handler,
                                                &retry_delay,
                                                NULL );
                    break;

                default:
                    QCRIL_LOG_INFO( "RIL halted." );
                    qmi_ril_set_operational_status( QMI_RIL_GEN_OPERATIONAL_STATUS_HALTED );
                    break;
            }
        }
    }
    else
    {
        QCRIL_LOG_FATAL("CHECK FAILED");
    }

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_resuming_con_handler
//===========================================================================
// qmi_ril_resume_retry_handler
//===========================================================================
void qmi_ril_resume_retry_handler(void * param)
{
    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( param );

    qmi_ril_set_operational_status( QMI_RIL_GEN_OPERATIONAL_STATUS_RESUME_PENDING );
    qmi_ril_modem_reset_round_info.transition_index = QMI_RIL_ZERO;
    qmi_ril_next_resuming_action();

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_resume_retry_handler
//===========================================================================
// qmi_ril_pre_resume_main_threaded
//===========================================================================
void qmi_ril_pre_resume_main_threaded(void * param)
{
    qcril_modem_restart_con_type outcome;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( param );

    qmi_ril_set_operational_status( QMI_RIL_GEN_OPERATIONAL_STATUS_RESUMING );

    memset( &outcome, 0, sizeof(outcome) );
    outcome.result = qcril_qmi_init_core_client_handles();
    QCRIL_LOG_INFO( ".. outcome of qcril_qmi_init_core_client_handles() - %d", (int) outcome.result );
    if (RIL_E_SUCCESS != outcome.result)
    { // rollback
        qmi_ril_set_operational_status( QMI_RIL_GEN_OPERATIONAL_STATUS_RESUME_RETRY );
    }
    else
    {
        qcril_qmi_nas_dms_commmon_post_init();
    }

    outcome.evt_originator = QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_RESUME_REQ;
    qcril_modem_restart_confirm_suspend_resume_step( QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_RESUME_CON, &outcome );

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_pre_resume_main_threaded
//===========================================================================
// qmi_ril_core_pre_resume_handler
//===========================================================================
void qmi_ril_core_pre_resume_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( params_ptr );
    QCRIL_NOTUSED(ret_ptr);

    qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID,
                            QCRIL_DEFAULT_MODEM_ID,
                            qmi_ril_pre_resume_main_threaded,
                            NULL,  // immediate
                            NULL );

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_core_pre_resume_handler

//===========================================================================
// qmi_ril_core_final_resume_handler
//===========================================================================
void qmi_ril_core_final_resume_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    qcril_modem_restart_con_type outcome;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( params_ptr );
    QCRIL_NOTUSED(ret_ptr);

    qcril_qmi_nas_embms_send_radio_state(RADIO_STATE_AVAILABLE_V01);
    // operation mode - go

    memset( &outcome, 0, sizeof(outcome) );
    outcome.evt_originator = QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_RESUME_REQ;
    outcome.result         = RIL_E_SUCCESS;
    qcril_modem_restart_confirm_suspend_resume_step( QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_RESUME_CON, &outcome );

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_core_final_resume_handler

/*===========================================================================

  FUNCTION: qcril_qmi_check_if_service_is_up

===========================================================================*/
/*!
    @brief
    checks if a service is up

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_check_if_service_is_up
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
    int rc;
    qmi_service_info info;

    qcril_check_service_up_data *qcril_service_up_data = NULL;

    QCRIL_NOTUSED(ret_ptr);

    if (params_ptr)
    {
        qcril_service_up_data = params_ptr->data;
    }

    if (qcril_service_up_data &&
        (client_info.client_state[(intptr_t)qcril_service_up_data->cb_data] ==
                                            QCRIL_QMI_SERVICE_NOT_AVAILABLE))
    {
        QCRIL_LOG_INFO("number of active clients %d", client_info.num_of_active_clients);
        if (QMI_NO_ERR == (rc = qmi_client_get_service_instance(qcril_service_up_data->svc_obj,
                                                          qmi_ril_client_get_master_port(),
                                                          &info)))
        {
            client_info.client_state[(intptr_t)qcril_service_up_data->cb_data] =
                                                             QCRIL_QMI_SERVICE_CONNECTED;
            qmi_client_release(client_info.notifier[(intptr_t)qcril_service_up_data->cb_data]);
            client_info.notifier[(intptr_t)qcril_service_up_data->cb_data] = NULL;

            CLIENT_CACHE_LOCK();
            client_info.num_of_active_clients++;
            CLIENT_CACHE_UNLOCK();
        }
        else
        {
            QCRIL_LOG_ERROR("qmi_client_get_service_instance return  %x %d",
                                           qcril_service_up_data->svc_obj, rc);
        }

        CLIENT_CACHE_LOCK();
        if (client_info.num_of_active_clients == client_info.max_active_clients)
        {
            CLIENT_CACHE_UNLOCK();
            QCRIL_LOG_INFO("resuming qcril");
            qmi_ril_enter_resume();
        }
        else
        {
            CLIENT_CACHE_UNLOCK();
        }
    }

    return;
}


/*===========================================================================

  FUNCTION: qcril_qmi_service_up_event

===========================================================================*/
/*!
    @brief
    processes service up event

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_service_up_event
(
  qmi_client_type               clnt,
  qmi_idl_service_object_type   svc_obj,
  qmi_client_notify_event_type  service_event,
  void                         *cb_data
)
{
    qmi_service_info info;
    qcril_check_service_up_data qcril_service_up_data = {0};

    QCRIL_NOTUSED(clnt);
    QCRIL_LOG_INFO("qcril_qmi_service_up_event %"PRIdPTR" %d",
                             (intptr_t) cb_data, service_event);

    if (service_event == QMI_CLIENT_SERVICE_COUNT_INC)
    {
        if (client_info.client_state[(intptr_t)cb_data] == QCRIL_QMI_SERVICE_NOT_AVAILABLE)
        {
            qcril_service_up_data.svc_obj = svc_obj;
            qcril_service_up_data.cb_data = cb_data;

            qcril_event_queue( qmi_ril_get_process_instance_id(),
                               QCRIL_DEFAULT_MODEM_ID,
                               QCRIL_DATA_ON_STACK,
                               QCRIL_EVT_QMI_RIL_MODEM_RESTART_CHECK_IF_SERVICE_UP,
                               &qcril_service_up_data,
                               sizeof(qcril_service_up_data),
                               (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
        }
    }

    QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION: qcril_qmi_service_down_event

===========================================================================*/
/*!
    @brief
    processes service down event

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_service_down_event
(
  qmi_client_type       clnt,
  qmi_client_error_type error,
  void                 *error_cb_data
)
{
    int srv_index = (intptr_t) error_cb_data;
    QCRIL_LOG_INFO("qcril_qmi_service_down_event %d", (int) srv_index);

    QCRIL_NOTUSED(clnt);
    QCRIL_NOTUSED(error);
    QCRIL_LOG_INFO("qcril_qmi_service_down_event %"PRIdPTR, (intptr_t) error_cb_data);

    if ((client_info.client_state[srv_index] == QCRIL_QMI_SERVICE_CONNECTED) ||
        !client_info.num_of_active_clients)
    {
      qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                                    QCRIL_DEFAULT_MODEM_ID,
                                    QCRIL_DATA_ON_STACK,
                                    QCRIL_EVT_QMI_RIL_SERVICE_DOWN,
                                    &srv_index,
                                    sizeof(srv_index),
                                    (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
    }
}


// qcril_qmi_handle_event_service_down
void qcril_qmi_handle_event_service_down(
const qcril_request_params_type *const params_ptr,
qcril_request_return_type       *const ret_ptr
)
{
    int srv_index;

    QCRIL_NOTUSED( ret_ptr );

    srv_index = *((int*)params_ptr->data);

    QCRIL_LOG_INFO("qcril_qmi_handle_event_service_down %d", (int) srv_index);

    CLIENT_CACHE_LOCK();

    if (client_info.client_state[srv_index] == QCRIL_QMI_SERVICE_CONNECTED)
    {
        client_info.client_state[srv_index] = QCRIL_QMI_SERVICE_NOT_AVAILABLE;
        client_info.num_of_active_clients--;
    }

    QCRIL_LOG_INFO("number of active clients %d", client_info.num_of_active_clients);
    if (!client_info.num_of_active_clients)
    {
        CLIENT_CACHE_UNLOCK();
        qmi_ril_enter_suspend();
    }
    else
    {
        CLIENT_CACHE_UNLOCK()
    }
} // qcril_qmi_handle_event_service_down

/*===========================================================================

  FUNCTION: qcril_qmi_register_for_up_event

===========================================================================*/
/*!
    @brief
    Register for qmi notifier event

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_register_for_up_event
(
    void
)
{
    int                  idx;
    int                  rc = 0;

    QCRIL_LOG_FUNC_ENTRY();
    for (idx = QCRIL_QMI_CLIENT_FIRST; idx < QCRIL_QMI_CLIENT_LAST; idx++)
    {
        if (client_info.client_state[idx] == QCRIL_QMI_SERVICE_NOT_AVAILABLE)
        {
            if (!client_info.notifier[idx])
            {
                rc = qmi_client_notifier_init(client_info.service_objects[idx],
                                              &client_info.os_params[idx],
                                              &client_info.notifier[idx]);
                QCRIL_LOG_INFO("qmi_client_notifier_init return %d", (int) rc);
            }

            if (!rc)
            {
                rc = qmi_client_register_notify_cb(client_info.notifier[idx],
                                              qcril_qmi_service_up_event,
                                              (void*)(intptr_t)idx);
                QCRIL_LOG_ERROR("qmi_client_register_notify_cb %d", (int) rc);
            }
        }
    }

    QCRIL_LOG_FUNC_RETURN();
    return;
}

void qcril_qmi_release_services()
{
    int iter;
    CLIENT_CACHE_LOCK();
    for (iter = QCRIL_QMI_CLIENT_FIRST; iter < QCRIL_QMI_CLIENT_LAST; iter++)
    {
        if (client_info.client_state[iter] == QCRIL_QMI_SERVICE_CONNECTED)
        {
            client_info.client_state[iter] = QCRIL_QMI_SERVICE_NOT_AVAILABLE;
            client_info.num_of_active_clients--;
        }
    }
    CLIENT_CACHE_UNLOCK();
    QCRIL_LOG_INFO("released all services");
    qmi_ril_enter_suspend();
}

//===========================================================================
// qmi_ril_stub_data_suspend_handler
//===========================================================================
void qmi_ril_stub_data_suspend_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    qcril_modem_restart_con_type outcome;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( params_ptr );
    QCRIL_NOTUSED( ret_ptr );

    qcril_data_qmi_wds_release();
    memset( &outcome, 0, sizeof(outcome) );
    outcome.result = RIL_E_SUCCESS;

    outcome.evt_originator = QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_SUSPEND_REQ;
    qcril_modem_restart_confirm_suspend_resume_step( QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_SUSPEND_CON, &outcome );

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_stub_data_suspend_handler

//===========================================================================
// qmi_ril_stub_data_resume_handler
//===========================================================================
void qmi_ril_stub_data_resume_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    qcril_modem_restart_con_type outcome;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( params_ptr );
    QCRIL_NOTUSED( ret_ptr );

    qcril_data_qmi_wds_init(TRUE);

    memset( &outcome, 0, sizeof(outcome) );
    outcome.result = RIL_E_SUCCESS;

    outcome.evt_originator = QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_RESUME_REQ;
    qcril_modem_restart_confirm_suspend_resume_step( QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_RESUME_CON, &outcome );

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_stub_data_resume_handler

//===========================================================================
// qmi_ril_route_uim_suspend_handler
//===========================================================================
void qmi_ril_route_uim_suspend_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    qcril_modem_restart_con_type outcome;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( params_ptr );
    QCRIL_NOTUSED( ret_ptr );

    qcril_process_event( qmi_ril_get_process_instance_id(),
                           QCRIL_DEFAULT_MODEM_ID,
                           QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_START,   // route to UIM
                           params_ptr->data,
                           params_ptr->datalen,
                           (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );


    memset( &outcome, 0, sizeof(outcome) );
    outcome.result = RIL_E_SUCCESS;

    outcome.evt_originator = QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_SUSPEND_REQ;
    qcril_modem_restart_confirm_suspend_resume_step( QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_SUSPEND_CON, &outcome );

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_route_uim_suspend_handler

//===========================================================================
// qmi_ril_route_uim_resume_handler
//===========================================================================
void qmi_ril_route_uim_resume_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    qcril_modem_restart_con_type outcome;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( params_ptr );
    QCRIL_NOTUSED( ret_ptr );

    qcril_process_event( qmi_ril_get_process_instance_id(),
                           QCRIL_DEFAULT_MODEM_ID,
                           QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_COMPLETE,   // route to UIM
                           params_ptr->data,
                           params_ptr->datalen,
                           (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );


    memset( &outcome, 0, sizeof(outcome) );
    outcome.result = RIL_E_SUCCESS;

    outcome.evt_originator = QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_RESUME_REQ;
    qcril_modem_restart_confirm_suspend_resume_step( QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_RESUME_CON, &outcome );

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_route_uim_resume_handler

//===========================================================================
// qcril_qmi_client_common_empty_unsol_ind_cb
//===========================================================================
void qcril_qmi_client_common_empty_unsol_ind_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                          *ind_buf,
  unsigned int                   ind_buf_len,
  void                          *ind_cb_data
)
{
    QCRIL_NOTUSED(user_handle);
    QCRIL_NOTUSED(ind_buf);
    QCRIL_NOTUSED(ind_buf_len);
    QCRIL_NOTUSED(ind_cb_data);
    QCRIL_LOG_INFO( "msg_id %d", (int) msg_id );
} // qcril_qmi_client_common_empty_unsol_ind_cb

//===========================================================================
// qmi_ril_suspend_init_work_handler
//===========================================================================
void qmi_ril_suspend_init_work_handler()
{
    QCRIL_LOG_FUNC_ENTRY();

    qmi_ril_reset_android_unsol_resp_dispatchable_table();
    qmi_ril_reset_unsol_resp_pending_list();

    qcril_qmi_nas_cancel_srv_domain_camped_timer_helper();
    qcril_qmi_nas_cancel_wait_for_pbm_ind_timer();
    if (!qcril_qmi_nas_modem_power_is_mdm_shdn_in_apm() ||
        1 == qcril_qmi_modem_power_voting_state())
    {
        qcril_qmi_nas_cancel_radio_power_process(QMI_RIL_DMS_RADIO_PWR_CANCEL_SSR);
    }
    qcril_qmi_nas_dms_update_card_status(qmi_ril_get_process_instance_id(), qmi_ril_get_sim_slot(), FALSE, QCRIL_CARD_STATUS_UNKNOWN);
    qcril_qmi_arb_reset_pref_data_snapshot();
    qcril_qmi_drop_sig_info_cache();
    if ( qmi_ril_is_feature_supported(QMI_RIL_FEATURE_APQ ) ||
         QCRIL_IS_DSDA_COEX_ENABLED() )
    {
      qcril_qmi_coex_terminate_riva_thread();
    }
    qcril_qmi_voice_speech_codec_info_cleanup();
    qcril_qmi_nas_cancel_sys_sel_pref_tmr();
    QCRIL_LOG_FUNC_RETURN();
} //qmi_ril_suspend_init_work_handler

//===========================================================================
// qcril_qmi_cb_thread_name_init
//===========================================================================
void qcril_qmi_cb_thread_name_init()
{
    RIL_Errno res = RIL_E_GENERIC_FAILURE;
    dms_get_device_mfr_resp_msg_v01 * qmi_response;
    qmi_client_error_type qmi_client_error;
    qmi_txn_handle txn_handle;

    qmi_response = qcril_malloc( sizeof(*qmi_response) );
    if ( qmi_response )
    {
        qmi_client_error =  qmi_client_send_msg_async( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_DMS ),
                                                           QMI_DMS_GET_DEVICE_MFR_REQ_V01,
                                                           NULL,
                                                           QMI_RIL_ZERO,
                                                           (void*) qmi_response,
                                                           sizeof( *qmi_response ),
                                                           qcril_qmi_qmi_thread_name_init_cb,
                                                           NULL,
                                                           &txn_handle
                                                            );

        res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, NULL );
        QCRIL_LOG_INFO("posted req %d, %d", (int) qmi_client_error, (int) res );

        if( RIL_E_SUCCESS != res )
        {
            qcril_free( qmi_response );
        }
    }
} // qcril_qmi_cb_thread_name_init


//===========================================================================
// qcril_qmi_qmi_thread_name_init_cb
//===========================================================================
void qcril_qmi_qmi_thread_name_init_cb
(
  qmi_client_type              user_handle,
  unsigned int                 msg_id,
  void                        *resp_c_struct,
  unsigned int                 resp_c_struct_len,
  void                        *resp_cb_data,
  qmi_client_error_type        transp_err
)
{
    dms_get_device_mfr_resp_msg_v01 * qmi_response   = (dms_get_device_mfr_resp_msg_v01 *) resp_c_struct;
    RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;

    qmi_ril_set_thread_name(pthread_self(), QMI_RIL_QMI_CALLBACK_THREAD_NAME);
    QCRIL_LOG_INFO("transp_err %d", (int) transp_err);

    QCRIL_NOTUSED(user_handle);
    QCRIL_NOTUSED(msg_id);
    QCRIL_NOTUSED(resp_c_struct_len);
    QCRIL_NOTUSED(resp_cb_data);

    if ( qmi_response )
    {
        ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( transp_err, &qmi_response->resp );
        if( RIL_E_SUCCESS == ril_req_res )
        {
            QCRIL_LOG_INFO( "Device Manufacturer : %s",qmi_response->device_manufacturer );
        }
        qcril_free( qmi_response );
    }

    QCRIL_LOG_FUNC_RETURN();
}  // qcril_qmi_qmi_thread_name_init_cb

//===========================================================================
// qmi_ril_qmi_client_get_qmi_service_name
//===========================================================================
const char* qmi_ril_qmi_client_get_qmi_service_name( qcril_qmi_client_e_type qmi_service )
{
  const char * service_name_res = NULL;
  const char * const service_names[QCRIL_QMI_CLIENT_MAX] =
  {
    [QCRIL_QMI_CLIENT_VOICE]        = "QMI Voice",
    [QCRIL_QMI_CLIENT_NAS]          = "QMI NAS",
    [QCRIL_QMI_CLIENT_WMS]          = "QMI WMS",
    [QCRIL_QMI_CLIENT_WDS]          = "QMI WDS",
    [QCRIL_QMI_CLIENT_DMS]          = "QMI DMS",
    [QCRIL_QMI_CLIENT_UIM]          = "QMI UIM",
    [QCRIL_QMI_CLIENT_PBM]          = "QMI PBM",
    [QCRIL_QMI_CLIENT_RF_SAR]       = "QMI RF SAR",
    [QCRIL_QMI_CLIENT_IMS_VT]       = "QMI IMS VT",
    [QCRIL_QMI_CLIENT_IMS_PRESENCE] = "QMI IMS Presence",
    [QCRIL_QMI_CLIENT_RFPE]         = "QMI RFPE",
    [QCRIL_QMI_CLIENT_PDC]          = "QMI PDC",
  };

  if ( qmi_service < QCRIL_QMI_CLIENT_MAX )
  {
    service_name_res = service_names[ qmi_service ];
  }

  if ( !service_name_res )
  {
    service_name_res = "unknown";
  }

  return service_name_res;
} // qmi_ril_qmi_client_get_qmi_service_name
//===========================================================================
// qmi_ril_qmi_client_log_request
//===========================================================================
void qmi_ril_qmi_client_log_request(
                                    qcril_qmi_client_e_type qmi_service,
                                    int                     message_id,
                                    void*                   message_req_c_struct_payload,
                                    int                     message_req_c_struct_payload_len,
                                    char*                   log_extra
                                    )
{
  const char*                       svc_name;
  uint32_t                          qmi_message_len;
  qmi_idl_service_object_type       svc_object;
  int32_t                           qmi_idl_err;
  char                              log_essence[ QCRIL_MAX_LOG_MSG_SIZE ];
  char                              log_addon[ QCRIL_MAX_LOG_MSG_SIZE ];
  void*                             encoded_qmi_bytestream;
  uint32_t                          encoded_qmi_bytestream_len;

  QCRIL_NOTUSED(log_extra);

  if ( qmi_service < QCRIL_QMI_CLIENT_MAX )
  {
    svc_name   = qmi_ril_qmi_client_get_qmi_service_name ( qmi_service );

    snprintf( log_essence, QCRIL_MAX_LOG_MSG_SIZE,
                "QMI_IO: send to %s msg id %d", svc_name, message_id );

    svc_object = client_info.service_objects [ qmi_service ];

    qmi_idl_err = qmi_idl_get_max_message_len( svc_object, QMI_IDL_REQUEST, message_id, &qmi_message_len );

    if ( QMI_IDL_LIB_NO_ERR == qmi_idl_err )
    {
      encoded_qmi_bytestream = qcril_malloc( qmi_message_len );
      if ( NULL != encoded_qmi_bytestream )
      {
        encoded_qmi_bytestream_len = QMI_RIL_ZERO;
        qmi_idl_err = qmi_idl_message_encode( svc_object,
                                              QMI_IDL_REQUEST,
                                              message_id,
                                              message_req_c_struct_payload,
                                              message_req_c_struct_payload_len,
                                              encoded_qmi_bytestream,
                                              qmi_message_len,
                                              &encoded_qmi_bytestream_len
                                             );
        if ( QMI_IDL_LIB_NO_ERR == qmi_idl_err )
        {
          snprintf( log_addon, QCRIL_MAX_LOG_MSG_SIZE,
                        " encoded len %d", encoded_qmi_bytestream_len );
          strlcat( log_essence, log_addon, sizeof( log_essence ) );

          qcril_qmi_print_hex( encoded_qmi_bytestream, encoded_qmi_bytestream_len );
        }
        qcril_free( encoded_qmi_bytestream );
      }
    }

    QCRIL_LOG_ESSENTIAL( log_essence );
  }

} // qmi_ril_qmi_client_log_request

//===========================================================================
// qmi_ril_qmi_client_get_qmi_service_name_from_obj
//===========================================================================
const char* qmi_ril_qmi_client_get_qmi_service_name_from_obj( qmi_idl_service_object_type service_object )
{
  qcril_qmi_client_e_type qmi_service;
  int                     found;
  const char * service_name_res;

  found = FALSE;
  for ( qmi_service = QCRIL_QMI_CLIENT_FIRST; qmi_service < QCRIL_QMI_CLIENT_LAST; qmi_service++ )
  {
    if ( client_info.service_objects [ qmi_service ] == service_object )
    {
      found = TRUE;
      break;
    }
  }

  if ( found )
  {
    service_name_res = qmi_ril_qmi_client_get_qmi_service_name (qmi_service);
  }
  else
  {
    service_name_res = "unknown";
  }

  return service_name_res;
} // qmi_ril_qmi_client_get_qmi_service_name_from_obj

//===========================================================================
// qmi_ril_qmi_req_log
//===========================================================================
void qmi_ril_qmi_req_log( qmi_idl_service_object_type service_object,
                          int message_id,
                          void* encoded_qmi_bytestream,
                          int encoded_qmi_bytestream_len )
{
  QCRIL_LOG_DEBUG( "req %s, msg id: %d, e len:%d",
                       qmi_ril_qmi_client_get_qmi_service_name_from_obj(service_object) ,
                       message_id,
                       encoded_qmi_bytestream_len
                     );

  if ( NULL != encoded_qmi_bytestream && encoded_qmi_bytestream_len > 0 )
  {
    qcril_qmi_print_hex( encoded_qmi_bytestream, encoded_qmi_bytestream_len );
  }
} // qmi_ril_qmi_req_log

//===========================================================================
// qmi_ril_qmi_resp_log
//===========================================================================
void qmi_ril_qmi_resp_log( qmi_idl_service_object_type service_object,
                           int message_id,
                           void* encoded_qmi_bytestream,
                           int encoded_qmi_bytestream_len )
{
  QCRIL_LOG_DEBUG( "rsp - %s, msg id: %d, e len:%d",
                       qmi_ril_qmi_client_get_qmi_service_name_from_obj(service_object) ,
                       message_id,
                       encoded_qmi_bytestream_len
                     );

  if ( NULL != encoded_qmi_bytestream && encoded_qmi_bytestream_len > 0 )
  {
    qcril_qmi_print_hex( encoded_qmi_bytestream, encoded_qmi_bytestream_len );
  }
} // qmi_ril_qmi_resp_log

//===========================================================================
// qmi_ril_qmi_ind_log
//===========================================================================
void qmi_ril_qmi_ind_log( int service_id,
                          int message_id,
                          void* encoded_qmi_bytestream,
                          int encoded_qmi_bytestream_len )
{
  char * svc_name;

  switch ( service_id )
  {
    case QMI_WDS_SERVICE:
      svc_name = "QMI WDS";
      break;

    case QMI_DMS_SERVICE:
      svc_name = "QMI DMS";
      break;

    case QMI_NAS_SERVICE:
      svc_name = "QMI NAS";
      break;

    case QMI_QOS_SERVICE:
      svc_name = "QMI QOS";
      break;

    case QMI_WMS_SERVICE:
      svc_name = "QMI WMS";
      break;

    case QMI_VOICE_SERVICE:
      svc_name = "QMI Voice";
      break;

    case QMI_CAT_SERVICE:
      svc_name = "QMI CAT";
      break;

    case QMI_UIM_SERVICE:
      svc_name = "QMI UIM";
      break;

    case QMI_PBM_SERVICE:
      svc_name = "QMI PBM";
      break;

    case QMI_SAR_SERVICE:
      svc_name = "QMI SAR";
      break;

    case QMI_IMS_VIDEO_SERVICE:
      svc_name = "QMI IMS Video";
      break;

    case QMI_IMS_VT_SERVICE:
      svc_name = "QMI IMS VT";
      break;

    case QMI_IMS_PRESENCE_SERVICE:
      svc_name = "QMI IMS Presence";
      break;

    case QMI_RFPE_SERVICE:
      svc_name = "QMI RFPE";
      break;

    default:
      svc_name = "unknown";
      break;
  }

  QCRIL_LOG_DEBUG( "ind %s, msg id: %d, e len:%d",
                       svc_name,
                       message_id,
                       encoded_qmi_bytestream_len
                     );

  if ( NULL != encoded_qmi_bytestream && encoded_qmi_bytestream_len > 0 )
  {
    qcril_qmi_print_hex( encoded_qmi_bytestream, encoded_qmi_bytestream_len );
  }
} // qmi_ril_qmi_ind

//===========================================================================
// qmi_ril_qmi_client_pre_initialization_init
//===========================================================================
void qmi_ril_qmi_client_pre_initialization_init()
{
    QCRIL_LOG_DEBUG("entry");

    qmi_ril_enter_critical_section();
    qmi_ril_qmi_client_pre_initialization_get();
    qmi_ril_leave_critical_section();

    QCRIL_LOG_DEBUG("exit");
} //qmi_ril_qmi_client_pre_initialization_init

//===========================================================================
// qmi_ril_qmi_client_pre_initialization_acquire
//===========================================================================
void qmi_ril_qmi_client_pre_initialization_acquire()
{
    QCRIL_LOG_ESSENTIAL("entry");

    qmi_ril_enter_critical_section();
    if(FALSE == qmi_ril_qmi_client_pre_initialization_lock_held)
    {
        acquire_wake_lock( PARTIAL_WAKE_LOCK, QCRIL_PRE_CLIENT_INIT_WAKE_LOCK_NAME );
        qmi_ril_qmi_client_pre_initialization_lock_held = TRUE;
        qmi_ril_qmi_client_pre_initialization_set();
    }
    qmi_ril_leave_critical_section();

    QCRIL_LOG_ESSENTIAL("exit");
} //qmi_ril_qmi_client_pre_initialization_acquire

//===========================================================================
// qmi_ril_qmi_client_pre_initialization_release
//===========================================================================
void qmi_ril_qmi_client_pre_initialization_release()
{
    QCRIL_LOG_ESSENTIAL("entry");

    qmi_ril_enter_critical_section();
    if(TRUE == qmi_ril_qmi_client_pre_initialization_lock_held)
    {
        release_wake_lock( QCRIL_PRE_CLIENT_INIT_WAKE_LOCK_NAME );
        qmi_ril_qmi_client_pre_initialization_lock_held = FALSE;
        qmi_ril_qmi_client_pre_initialization_set();
    }
    qmi_ril_leave_critical_section();

    QCRIL_LOG_ESSENTIAL("exit");
} //qmi_ril_qmi_client_pre_initialization_release

//===========================================================================
// qmi_ril_qmi_client_pre_initialization_set
//===========================================================================
void qmi_ril_qmi_client_pre_initialization_set()
{
    char args[ PROPERTY_VALUE_MAX ];

    QCRIL_LOG_INFO( "qmi_ril_qmi_client_pre_initialization_lock_held %d", qmi_ril_qmi_client_pre_initialization_lock_held );

    QCRIL_SNPRINTF( args, sizeof( args ), "%d", qmi_ril_qmi_client_pre_initialization_lock_held );

    if ( property_set( QCRIL_PRE_CLIENT_INIT_WAKE_LOCK_HELD_PROP, args ) != E_SUCCESS )
    {
      QCRIL_LOG_ERROR( "Fail to save %s to system property", QCRIL_PRE_CLIENT_INIT_WAKE_LOCK_HELD_PROP );
    }
    else
    {
      QCRIL_LOG_INFO("Set %s property to %s", QCRIL_PRE_CLIENT_INIT_WAKE_LOCK_HELD_PROP, args);
    }
} //qmi_ril_qmi_client_pre_initialization_set

//===========================================================================
// qmi_ril_qmi_client_pre_initialization_get
//===========================================================================
void qmi_ril_qmi_client_pre_initialization_get()
{
    int temp_len;
    unsigned long ret_val;
    char args[ PROPERTY_VALUE_MAX ];
    char *end_ptr;

    qmi_ril_qmi_client_pre_initialization_lock_held = FALSE;
    property_get( QCRIL_PRE_CLIENT_INIT_WAKE_LOCK_HELD_PROP, args, "" );
    temp_len = strlen( args );
    if ( temp_len > 0 )
    {
      ret_val = strtoul( args, &end_ptr, 0 );
      if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
      {
        QCRIL_LOG_ERROR( "Fail to convert QCRIL_PRE_CLIENT_INIT_WAKE_LOCK_HELD_PROP %s", args );
      }
      else if ( ret_val > 1 )
      {
        QCRIL_LOG_ERROR( "Invalid saved QCRIL_PRE_CLIENT_INIT_WAKE_LOCK_HELD_PROP %ld, use default", ret_val );
      }
      else
      {
        qmi_ril_qmi_client_pre_initialization_lock_held = ( uint8 ) ret_val;
      }
    }
    QCRIL_LOG_DEBUG( "QCRIL_PRE_CLIENT_INIT_WAKE_LOCK_HELD_PROP=%d", qmi_ril_qmi_client_pre_initialization_lock_held);
} //qmi_ril_qmi_client_pre_initialization_get


