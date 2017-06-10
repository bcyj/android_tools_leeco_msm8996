/******************************************************************************
  @file    qcril_qmi_pdc.c
  @brief   qcril qmi - PDC

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI PDC.

  ---------------------------------------------------------------------------

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


//===========================================================================
//
//                           INCLUDE FILES
//
//===========================================================================

#include <errno.h>
#include <cutils/memory.h>
#include <cutils/properties.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include "ril.h"
#include "comdef.h"
#include "qcrili.h"
#include "qmi_errors.h"
#include "qcril_reqlist.h"
#include "qcril_qmi_client.h"
#include "qcril_qmi_pdc.h"
#include "qmi_ril_platform_dep.h"
#include "persistent_device_configuration_v01.h"


//===========================================================================
//
//                    INTERNAL DEFINITIONS AND TYPES
//
//===========================================================================

#define QCRIL_MODEM_MBN_FILE_PATH           "persist.radio.mbn_path"
#define QCRIL_MODEM_MBN_DEFAULT_PATH        "/data/modem_config/"
#define QCRIL_PDC_FRAME_SIZE                900
#define QCRIL_PDC_MAX_SUBS_SUPPORT          3
#define QCRIL_PDC_ALL_CONFIGS_LEN           ((PDC_CONFIG_ID_SIZE_MAX_V01+1) * PDC_CONFIG_LIST_SIZE_MAX_V01+1)

#define PDC_CONFIG_LOCK()    pthread_mutex_lock(&g_pdc_info.pdc_config_mutex)
#define PDC_CONFIG_UNLOCK()  pthread_mutex_unlock(&g_pdc_info.pdc_config_mutex)

// error code
#define QCRIL_PDC_NO_ERROR      0
#define QCRIL_PDC_GENERIC_FAIL  -1
#define QCRIL_PDC_NO_MEMORY     -2
#define QCRIL_PDC_NO_CONFIGS    -3
#define QCRIL_PDC_SELECT_FAIL   -4
#define QCRIL_PDC_COMMERCIAL_MBN_PREFIX "commercial"

// validation result related
#define QCRIL_QMI_PDC_INDEX_END         (0xffffffffL)
#define QCRIL_QMI_PDC_IS_LAST_INDEX(idx)   ((idx) == QCRIL_QMI_PDC_INDEX_END)

typedef struct
{
  // user set info
  uint8_t config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
  uint32_t config_id_len;
  uint32_t conf_size;
  uint32_t load_size;
  int conf_fd;
} pdc_config_info_type;

typedef struct {
  // for loading configurations
  pdc_config_info_type pdc_config_info;
  char mbn_file_dir[ PROPERTY_VALUE_MAX ];
  pdc_config_info_resp_type_v01 cur_config_lists[ PDC_CONFIG_LIST_SIZE_MAX_V01 ];
  // for deleting all configurations
  uint32_t cur_config_len;
  uint8_t cur_delete_idx;
  // for select all SUBs
  uint8_t sub_count;
  uint8_t cur_select_sub;
  uint8_t sub_select_mask;
  // for deactivate all SUBs
  uint8_t cur_deact_sub;
  // is processing
  uint8_t is_under_processing;
  // is user request list
  uint8_t is_user_request_list;
  // is under list config request
  uint8_t is_under_list_request;

  // dump file description (MBN diff dump file: XML format)
  int dump_fd;
  char dump_file[QCRIL_DUMP_FILE_PATH_LEN];

  /* is retrieving mbn info */
  uint8_t is_retrieving_mbn_info;
  uint8_t  config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
  uint32_t config_id_len;
  // for pretecting
  pthread_mutex_t        pdc_config_mutex;
  pthread_mutexattr_t    pdc_config_mutex_attr;
} pdc_global_info_type;

typedef enum
{
  QCRIL_PDC_MBN_INFO_NONE,
  QCRIL_PDC_MBN_INFO_RETRIEVE_ACTIVE_ID,
  QCRIL_PDC_MBN_INFO_RETRIEVE_CONFIG_INFO
} qcril_pdc_mbn_info_retrieval_state;

static pdc_global_info_type g_pdc_info;

static boolean qcril_qmi_pdc_is_test_mbn
(
    void         *config_desc,
    unsigned int  config_desc_len
);

static void qcril_qmi_pdc_retrieve_mbn_info_for_config_id
(
    void
);

extern void qcril_request_clean_up_suppress_list();

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_load_configuraiton

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_RIL_PDC_LOAD_CONFIGURATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_load_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  pdc_load_config_req_msg_v01 qmi_request;
  pdc_load_config_resp_msg_v01 qmi_response;
  pdc_load_config_info_type_v01 *p_load_info;
  qmi_client_error_type qmi_error;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  char payload;
  int rd_len;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ret_ptr);
  QCRIL_NOTUSED(params_ptr);

  memset( &qmi_request, 0, sizeof(qmi_request) );
  p_load_info = &qmi_request.load_config_info;
  p_load_info->config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
  p_load_info->total_config_size = g_pdc_info.pdc_config_info.conf_size;

  strlcpy(p_load_info->config_id, g_pdc_info.pdc_config_info.config_id, PDC_CONFIG_ID_SIZE_MAX_V01);
  p_load_info->config_id_len = strlen(p_load_info->config_id);

  // read loop in case that it is interrupt by signals
  // for example TASK_FREEZE when sleep */
  if ( g_pdc_info.pdc_config_info.conf_fd == -1)
  {
    rd_len = -1;
    QCRIL_LOG_ERROR("The MBN file descriptor is -1");
  }
  else
  {
    do
    {
      rd_len = read( g_pdc_info.pdc_config_info.conf_fd,
          p_load_info->config_frame, QCRIL_PDC_FRAME_SIZE );
    } while ( ( rd_len == -1 ) && ( errno == EINTR ) );
  }

  if ( rd_len == -1 ) // there is some error when read file
  {
    result = RIL_E_GENERIC_FAILURE;
    QCRIL_LOG_ERROR("failed to read MBN file");
  }
  else if (rd_len == 0) // reach file end
  {
    // should not reach here, since the indication handler
    // will not queue this event if all of this config has
    // been loaded. Treat it as a error (one case: the file
    // length is empty)
    result = RIL_E_GENERIC_FAILURE;
    QCRIL_LOG_ERROR("reach file end, should not happen");
  }
  else
  {
    p_load_info->config_frame_len = rd_len;
    g_pdc_info.pdc_config_info.load_size += rd_len;
    qmi_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PDC ),
                                        QMI_PDC_LOAD_CONFIG_REQ_V01,
                                        &qmi_request,
                                        sizeof( qmi_request ),
                                        &qmi_response,
                                        sizeof( qmi_response ),
                                        QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT
                                        );
   result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
   if ( g_pdc_info.pdc_config_info.load_size >= g_pdc_info.pdc_config_info.conf_size )
   {
     QCRIL_LOG_INFO("load_size is %d, conf_size is %d",
         g_pdc_info.pdc_config_info.load_size, g_pdc_info.pdc_config_info.conf_size);
     close( g_pdc_info.pdc_config_info.conf_fd );
     g_pdc_info.pdc_config_info.conf_fd = -1;
   }
  }

  // on failure, send an unsol message to indicate error
  // on success, do nothing here, the load indication handler
  // will handle the remaining things
  if ( result == RIL_E_GENERIC_FAILURE )
  {
    payload = QCRIL_PDC_GENERIC_FAIL;
    g_pdc_info.is_under_processing = 0;
    qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_PDC_CONFIG,
                                    (void *)(&payload), sizeof(payload));
  }

  QCRIL_LOG_FUNC_RETURN();
}


/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_load_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_LOAD_CONFIG_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_load_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_load_config_ind_msg_v01 *load_ind;
  char payload;
  boolean load_completed = FALSE;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ind_data_len);

  if ( ind_data_ptr != NULL )
  {
    load_ind = (pdc_load_config_ind_msg_v01*)ind_data_ptr;
    if ( load_ind->error == QMI_ERR_NONE_V01 )
    {
      // if modem pass the remaining info, we use it to determine
      // whether the load has completed
      if ( load_ind->remaining_config_size_valid )
      {
        QCRIL_LOG_INFO("The remaininng_config_size is %d", load_ind->remaining_config_size);
        QCRIL_LOG_INFO("The received_config_size is %d", load_ind->received_config_size);
        if ( load_ind->remaining_config_size == 0 )
        {
          load_completed = TRUE;
        }
        else
        {
          // continue loading config
          qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                        QCRIL_DEFAULT_MODEM_ID,
                        QCRIL_DATA_ON_STACK,
                        QCRIL_EVT_QMI_RIL_PDC_LOAD_CONFIGURATION,
                        NULL,
                        QMI_RIL_ZERO,
                        (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
        }
      }
      else
      {
        if ( g_pdc_info.pdc_config_info.load_size >= g_pdc_info.pdc_config_info.conf_size )
        {
          load_completed = TRUE;
        }
        else
        {
          // continue loading config
          qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                        QCRIL_DEFAULT_MODEM_ID,
                        QCRIL_DATA_ON_STACK,
                        QCRIL_EVT_QMI_RIL_PDC_LOAD_CONFIGURATION,
                        NULL,
                        QMI_RIL_ZERO,
                        (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
        }
      }
    }
    else // there is some error
    {
      // case 1: the config is already loaded
      if ( load_ind->error == QMI_ERR_INVALID_ID_V01 )
      {
        QCRIL_LOG_INFO("Invalid config id, maybe already exists");
        load_completed = TRUE;
      }
      // case 2: no enough space
      else if ( load_ind->error == QMI_ERR_NO_MEMORY_V01 )
      {
        QCRIL_LOG_INFO("no memory in modem EFS");
        g_pdc_info.is_under_processing = 0;
        payload = QCRIL_PDC_NO_MEMORY;
        qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_PDC_CONFIG,
                                      (void *)(&payload), sizeof(payload) );
      }
      else
      {
        QCRIL_LOG_INFO("Failed to load configuration");
        g_pdc_info.is_under_processing = 0;
        payload = QCRIL_PDC_GENERIC_FAIL;
        qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_PDC_CONFIG,
                                      (void *)(&payload), sizeof(payload) );
      }
    } // there is some error

    if ( load_completed )
    {
      // get first sub that needs to select
      g_pdc_info.cur_select_sub = 0;
      while ( g_pdc_info.cur_select_sub < g_pdc_info.sub_count )
      {
        if ( (1 << g_pdc_info.cur_select_sub) & g_pdc_info.sub_select_mask )
          break;
        g_pdc_info.cur_select_sub++;
      }
      if ( g_pdc_info.cur_select_sub >= g_pdc_info.sub_count )
      {
        // no sub needs to select
        // all done, sent unsol msg to ATEL
        g_pdc_info.is_under_processing = 0;
        payload = QCRIL_PDC_NO_ERROR;
        qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_PDC_CONFIG,
                                        (void *)(&payload), sizeof(payload) );
      }
      else
      {
        // kick the select config start
        qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                      QCRIL_DEFAULT_MODEM_ID,
                      QCRIL_DATA_ON_STACK,
                      QCRIL_EVT_QMI_RIL_PDC_SELECT_CONFIGURATION,
                      NULL,
                      QMI_RIL_ZERO,
                      (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
      }
    }
  } //ind_data_ptr != NULL
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_select_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_SET_SELECTED_CONFIG_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_select_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_set_selected_config_ind_msg_v01 *set_ind;
  char payload;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ind_data_len);

  payload = QCRIL_PDC_GENERIC_FAIL;
  if ( ind_data_ptr != NULL )
  {
    set_ind = (pdc_set_selected_config_ind_msg_v01*)ind_data_ptr;
    if ( set_ind->error == QMI_ERR_NONE_V01 )
    {
      QCRIL_LOG_INFO("select successful for SUB:%d", g_pdc_info.cur_select_sub);
      payload = QCRIL_PDC_NO_ERROR;
    }
    else // there is some error
    {
      QCRIL_LOG_ERROR("select indication error for SUB:%d", g_pdc_info.cur_select_sub);
      payload = QCRIL_PDC_SELECT_FAIL;
    }
  }
  else
  {
    QCRIL_LOG_ERROR("select indication param NULL");
    payload = QCRIL_PDC_GENERIC_FAIL;
  }

  // get next sub that needs to be selected
  while (g_pdc_info.cur_select_sub < g_pdc_info.sub_count )
  {
    g_pdc_info.cur_select_sub++;
    if ( (1 << g_pdc_info.cur_select_sub) & g_pdc_info.sub_select_mask )
      break;
  }

  if ( g_pdc_info.cur_select_sub >= g_pdc_info.sub_count )
  {
    // all done, sent unsol msg to ATEL
    g_pdc_info.is_under_processing = 0;
    qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_PDC_CONFIG,
                                        (void *)(&payload), sizeof(payload) );
  }
  else
  {
    // continue to select config
    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                  QCRIL_DEFAULT_MODEM_ID,
                  QCRIL_DATA_ON_STACK,
                  QCRIL_EVT_QMI_RIL_PDC_SELECT_CONFIGURATION,
                  NULL,
                  QMI_RIL_ZERO,
                  (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
  }
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_activate_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_ACTIVATE_CONFIG_IND_V01

    @return
    None.

    @note
    This function should not be entered in, since modem will perform SSR
    before activation indication comes.
*/
/*=========================================================================*/
void qcril_qmi_pdc_activate_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_activate_config_ind_msg_v01 *act_ind;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  errno_enum_type reqlist_found;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ind_data_len);

  if ( ind_data_ptr != NULL )
  {
    act_ind = (pdc_activate_config_ind_msg_v01*)ind_data_ptr;
    if ( act_ind->error == QMI_ERR_NONE_V01 )
    {
      QCRIL_LOG_INFO("activate successful");
      result = RIL_E_SUCCESS;
    }
    else
    {
      QCRIL_LOG_ERROR("activate error, qmi error num: %d", act_ind->error);
    }
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_delete_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_DELETE_CONFIG_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_delete_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_delete_config_ind_msg_v01 *del_ind;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;
  RIL_Errno result = RIL_E_SUCCESS;
  char payload;

  QCRIL_LOG_FUNC_ENTRY();
  if ( ind_data_ptr != NULL )
  {
    del_ind = (pdc_delete_config_ind_msg_v01*)ind_data_ptr;
    if ( del_ind->error == QMI_ERR_NONE_V01 )
    {
      QCRIL_LOG_INFO("delete one configuration successfully");
    }
    else
    {
      QCRIL_LOG_ERROR("failed to delete one configuration, error id = %d", del_ind->error);
      result = RIL_E_GENERIC_FAILURE;
    }
  }
  else // ind_data_ptr = NULL
  {
    QCRIL_LOG_ERROR("NULL parameter");
    result = RIL_E_GENERIC_FAILURE;
  }

  // ignore the result and delete the next configuration
  qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                     QCRIL_DEFAULT_MODEM_ID,
                     QCRIL_DATA_ON_STACK,
                     QCRIL_EVT_QMI_RIL_PDC_DELETE_CONFIGURATION,
                     NULL,
                     QMI_RIL_ZERO,
                     (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
}


/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_get_config_info_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_GET_CONFIG_INFO_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_get_config_info_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_get_config_info_ind_msg_v01 *get_ind;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;
  RIL_Errno result;
  errno_enum_type reqlist_found;

  boolean has_config_desc = FALSE;
  int8_t config_desc[PDC_CONFIG_DESC_SIZE_MAX_V01];
  uint32_t config_desc_len;
  boolean has_base_version = FALSE;
  boolean has_config_version = FALSE;
  uint32_t base_version;
  uint32_t config_version;

  QCRIL_LOG_FUNC_ENTRY();

  if ( ind_data_ptr != NULL )
  {
    get_ind = (pdc_get_config_info_ind_msg_v01*)ind_data_ptr;
    // Ignore the qmi error code. Sometimes config_desc is reported although
    // there is qmi error. (Will be fixed by modem MCFG team later)
    if ( ( get_ind->config_desc_valid == 1) && ( get_ind->config_desc_len > 0 )
            && ( get_ind->config_desc_len < PDC_CONFIG_DESC_SIZE_MAX_V01 ) )
    {
      has_config_desc = TRUE;
      config_desc_len = get_ind->config_desc_len;
      memcpy(config_desc, get_ind->config_desc, config_desc_len);

      PDC_CONFIG_LOCK();
      if (QCRIL_PDC_MBN_INFO_RETRIEVE_CONFIG_INFO == g_pdc_info.is_retrieving_mbn_info)
      {
          g_pdc_info.is_retrieving_mbn_info = QCRIL_PDC_MBN_INFO_NONE;

          PDC_CONFIG_UNLOCK();
          if (qcril_qmi_pdc_is_test_mbn(config_desc, config_desc_len))
	      {
	          qcril_request_suppress_request(RIL_REQUEST_SET_INITIAL_ATTACH_APN);
	          qcril_request_suppress_request(RIL_REQUEST_SETUP_DATA_CALL);
	      }
          else
          {
              qcril_request_clean_up_suppress_list();
          }
      }
      else
      {
          PDC_CONFIG_UNLOCK();
      }
    }

    if (get_ind->base_version_valid == 1)
    {
      has_base_version = TRUE;
      base_version = get_ind->base_version;
    }

    if (get_ind->config_version_valid == 1)
    {
      has_config_version = TRUE;
      config_version = get_ind->config_version;
    }
  }
  else // ind_data_ptr = NULL
  {
    QCRIL_LOG_ERROR("NULL parameter");
  }

  // send response for QCRIL_EVT_HOOK_GET_META_INFO if has
  reqlist_found = qcril_reqlist_query_by_event( QCRIL_DEFAULT_INSTANCE_ID,
                                                QCRIL_DEFAULT_MODEM_ID,
                                                QCRIL_EVT_HOOK_GET_META_INFO,
                                                &req_info );
  if ( reqlist_found == E_SUCCESS )
  {
    result = (has_config_desc) ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE;
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                       req_info.t,
                                       req_info.request,
                                       result,
                                       &resp );
    if ( has_config_desc )
    {
      resp.resp_pkt = (void *) config_desc;
      resp.resp_len = config_desc_len;
    }
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_INFO("unable to find reqlist by event QCRIL_EVT_HOOK_GET_META_INFO");
  }

  // send response for QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIGID if has
  reqlist_found = qcril_reqlist_query_by_event( QCRIL_DEFAULT_INSTANCE_ID,
                                                QCRIL_DEFAULT_MODEM_ID,
                                                QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIGID,
                                                &req_info );
  if ( reqlist_found == E_SUCCESS )
  {
    result = (has_base_version) ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE;
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                       req_info.t,
                                       req_info.request,
                                       result,
                                       &resp );
    if ( has_base_version )
    {
      resp.resp_pkt = (void *) (&base_version);
      resp.resp_len = sizeof(base_version);
    }
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_INFO("unable to find reqlist by event QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIGID");
  }

  // send response for QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_CONFIGID if has
  reqlist_found = qcril_reqlist_query_by_event( QCRIL_DEFAULT_INSTANCE_ID,
                                                QCRIL_DEFAULT_MODEM_ID,
                                                QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_CONFIGID,
                                                &req_info );
  if ( reqlist_found == E_SUCCESS )
  {
    result = (has_base_version) ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE;
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                       req_info.t,
                                       req_info.request,
                                       result,
                                       &resp );
    if ( has_config_version )
    {
      resp.resp_pkt = (void *) (&config_version);
      resp.resp_len = sizeof(config_version);
    }
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_INFO("unable to find reqlist by event QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_CONFIGID");
  }

  g_pdc_info.is_under_processing = 0;

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_list_configs_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_LIST_CONFIGS_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_list_configs_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_list_configs_ind_msg_v01 *list_ind;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;
  RIL_Errno result = RIL_E_SUCCESS;
  errno_enum_type reqlist_found;
  char* result_array = NULL;
  char payload = QCRIL_PDC_NO_ERROR;
  int i, index;

  QCRIL_LOG_FUNC_ENTRY();

  if ( ind_data_ptr != NULL )
  {
    list_ind = (pdc_list_configs_ind_msg_v01*)ind_data_ptr;
    if ( list_ind->error == QMI_ERR_NONE_V01 )
    {
      if ( (list_ind->config_list_valid) && (list_ind->config_list_len > 0)
            && (list_ind->config_list_len <= PDC_CONFIG_LIST_SIZE_MAX_V01) )
      {
        QCRIL_LOG_INFO("total configuration count %d", list_ind->config_list_len);
      }
      else
      {
        QCRIL_LOG_ERROR("no valid config lists available");
        payload = QCRIL_PDC_NO_CONFIGS;
        result = RIL_E_GENERIC_FAILURE;
      }
    }
    else // QMI ERROR
    {
      QCRIL_LOG_ERROR("QMI error, error code %d", list_ind->error);
      payload = QCRIL_PDC_GENERIC_FAIL;
      result = RIL_E_GENERIC_FAILURE;
    }
  }
  else // ind_data_ptr = NULL
  {
    QCRIL_LOG_ERROR("NULL parameter");
    payload = QCRIL_PDC_GENERIC_FAIL;
    result = RIL_E_GENERIC_FAILURE;
  }

  if ( g_pdc_info.is_user_request_list )
  {
    reqlist_found = qcril_reqlist_query_by_event( QCRIL_DEFAULT_INSTANCE_ID,
                                                  QCRIL_DEFAULT_MODEM_ID,
                                                  QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS,
                                                  &req_info );
    if ( reqlist_found == E_SUCCESS )
    {
      if ( result == RIL_E_SUCCESS )
      {
        result_array = qcril_malloc( QCRIL_PDC_ALL_CONFIGS_LEN );
        if ( !result_array )
        {
          result = RIL_E_GENERIC_FAILURE;
        }
        else
        {
          result_array[0] = list_ind->config_list_len;
          index = 1;
          for (i = 0; i < list_ind->config_list_len; i++)
          {
            result_array[index++] = list_ind->config_list[i].config_id_len;
            memcpy(result_array+index, list_ind->config_list[i].config_id,
                            list_ind->config_list[i].config_id_len);
            index += list_ind->config_list[i].config_id_len;
          } // for

        }
      } //result
      qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                         req_info.t,
                                         req_info.request,
                                         result,
                                         &resp);
      if ( result == RIL_E_SUCCESS && result_array )
      {
        resp.resp_pkt = (void *)result_array;
        resp.resp_len = index;
      }
      qcril_send_request_response( &resp );
      if ( result_array )
      {
        free( result_array );
      }
      g_pdc_info.is_user_request_list = 0;
      g_pdc_info.is_under_processing = 0;
    } // reqlist_found
    else
    {
      QCRIL_LOG_ERROR("not found reqlist for QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS");
      result = RIL_E_GENERIC_FAILURE;
      g_pdc_info.is_under_processing = 0;
    }
  }
  else
  {
    if ( result == RIL_E_GENERIC_FAILURE )
    {
        g_pdc_info.is_under_list_request = 0;
        g_pdc_info.is_under_processing = 0;
        qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_PDC_CLEAR_CONFIGS,
                                            (void *)(&payload), sizeof(payload) );
    }
    else
    {
        g_pdc_info.cur_config_len = list_ind->config_list_len;
        g_pdc_info.cur_delete_idx = 0;
        memcpy( g_pdc_info.cur_config_lists, list_ind->config_list,
                g_pdc_info.cur_config_len * sizeof(pdc_config_info_resp_type_v01));
        // kick the delete task
        qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                        QCRIL_DEFAULT_MODEM_ID,
                        QCRIL_DATA_ON_STACK,
                        QCRIL_EVT_QMI_RIL_PDC_DELETE_CONFIGURATION,
                        NULL,
                        QMI_RIL_ZERO,
                        (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
    }
  }
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_validate_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_VALIDATE_CONFIG_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_validate_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_validate_config_ind_msg_v01 *valid_ind;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( !g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("ignore this indication, since error happens");
      break;
    }

    if ( ind_data_ptr != NULL )
    {
      valid_ind = (pdc_validate_config_ind_msg_v01*)ind_data_ptr;
      if ( valid_ind->error == QMI_ERR_NONE_V01 )
      {
        QCRIL_LOG_INFO("Validate success");
        result = RIL_E_SUCCESS;
      }
      else // QMI ERROR
      {
        QCRIL_LOG_ERROR("QMI error code %d", valid_ind->error);
        break;
      }
    }
    else // ind_data_ptr = NULL
    {
      QCRIL_LOG_ERROR("NULL parameter");
      break;
    }

    if ( !valid_ind->frame_index_valid || !valid_ind->result_frame_valid
              || valid_ind->result_frame_len > PDC_CONFIG_FRAME_SIZE_MAX_V01)
    {
      result = RIL_E_GENERIC_FAILURE;
      QCRIL_LOG_ERROR("frame_index valid: %d, result_frame valid: %d, frame_len: %d",
            valid_ind->frame_index_valid, valid_ind->result_frame_valid,
            valid_ind->result_frame_len);
      break;
    }

    if ( write(g_pdc_info.dump_fd, valid_ind->result_frame, valid_ind->result_frame_len)
                    != valid_ind->result_frame_len )
    {
      result = RIL_E_GENERIC_FAILURE;
      QCRIL_LOG_ERROR("failed to write the diff result to dump file: %s", strerror(errno));
      break;
    }

    if (QCRIL_QMI_PDC_IS_LAST_INDEX(valid_ind->frame_index))
    {
      QCRIL_LOG_INFO("validation result is fully dumped to file");
      g_pdc_info.is_under_processing = 0;
      close(g_pdc_info.dump_fd);
      qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_PDC_VALIDATE_DUMPED,
                                        (void*)g_pdc_info.dump_file, strlen(g_pdc_info.dump_file));
      qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                         QCRIL_DEFAULT_MODEM_ID,
                         QCRIL_DATA_ON_STACK,
                         QCRIL_EVT_QMI_RIL_PDC_PARSE_DIFF_RESULT,
                         (void*)g_pdc_info.dump_file,
                         strlen(g_pdc_info.dump_file),
                         (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
    }
  } while (0);

  if ( (result == RIL_E_GENERIC_FAILURE) && (g_pdc_info.is_under_processing) )
  {
    g_pdc_info.is_under_processing = 0;
    // send a message to ATEL, indicating the failure
    qcril_qmi_mbn_diff_send_unsol_msg(result, -1, NULL, NULL, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_deactivate_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_DEACTIVATE_CONFIG_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_deactivate_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_deactivate_config_ind_msg_v01 *deact_ind;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;
  RIL_Errno result;
  errno_enum_type reqlist_found;

  QCRIL_LOG_FUNC_ENTRY();

  if ( ind_data_ptr != NULL )
  {
    deact_ind = (pdc_deactivate_config_ind_msg_v01*)ind_data_ptr;
    if ( deact_ind->error == QMI_ERR_NONE_V01 )
    {
      QCRIL_LOG_INFO("Deactivate success for SUB%d", g_pdc_info.cur_deact_sub);
      result = RIL_E_SUCCESS;
    }
    else // QMI ERROR
    {
      QCRIL_LOG_ERROR("QMI error for SUB%d, error code %d", g_pdc_info.cur_deact_sub, deact_ind->error);
      result = RIL_E_GENERIC_FAILURE;
    }
  }
  else // ind_data_ptr = NULL
  {
    QCRIL_LOG_ERROR("NULL parameter");
    result = RIL_E_GENERIC_FAILURE;
  }

  // if the there is the error, we still do the delete work anyway
  // no need to check the result
  g_pdc_info.cur_deact_sub++;
  if ( g_pdc_info.cur_deact_sub >= g_pdc_info.sub_count )
  {
    if ( g_pdc_info.is_under_list_request )
    {
      // kick the LIST_CONFIGS start
      qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                     QCRIL_DEFAULT_MODEM_ID,
                     QCRIL_DATA_ON_STACK,
                     QCRIL_EVT_QMI_RIL_PDC_LIST_CONFIGURATION,
                     NULL,
                     QMI_RIL_ZERO,
                     (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
    }
    else
    {
      // send response for deactivate
      reqlist_found = qcril_reqlist_query_by_event( QCRIL_DEFAULT_INSTANCE_ID,
                                                    QCRIL_DEFAULT_MODEM_ID,
                                                    QCRIL_EVT_HOOK_DEACTIVATE_CONFIGS,
                                                    &req_info );
      if ( reqlist_found == E_SUCCESS )
      {
        result = RIL_E_SUCCESS;
        qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                           req_info.t,
                                           req_info.request,
                                           result,
                                           &resp );
        qcril_send_request_response( &resp );
      }
      else
      {
        QCRIL_LOG_ERROR("unable to find reqlist by event QCRIL_EVT_HOOK_DEACTIVATE_CONFIGS");
      }
      g_pdc_info.is_under_list_request = 0;
      g_pdc_info.is_under_processing = 0;
    }
  }
  else
  {
    // continue to deactivate the next SUB
    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                    QCRIL_DEFAULT_MODEM_ID,
                    QCRIL_DATA_ON_STACK,
                    QCRIL_EVT_QMI_RIL_PDC_DEACTIVATE_CONFIGURATION,
                    NULL,
                    QMI_RIL_ZERO,
                    (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_get_selected_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_GET_SELECTED_CONFIG_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_get_selected_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_get_selected_config_ind_msg_v01 *sel_ind;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;
  uint8_t config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
  uint32_t config_id_len;
  RIL_Errno result = RIL_E_SUCCESS;
  errno_enum_type reqlist_found;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ind_data_len);

  if ( ind_data_ptr != NULL )
  {
    sel_ind = (pdc_get_selected_config_ind_msg_v01*)ind_data_ptr;
    if ( sel_ind->error == QMI_ERR_NONE_V01 )
    {
      if ( sel_ind->active_config_id_valid ) // selected and activated
      {
        config_id_len = sel_ind->active_config_id_len;
        if ( config_id_len >= PDC_CONFIG_ID_SIZE_MAX_V01 )
        {
          result = RIL_E_GENERIC_FAILURE;
        }
        else
        {
          memcpy(config_id, sel_ind->active_config_id, config_id_len);

          PDC_CONFIG_LOCK();
          memcpy(g_pdc_info.config_id, sel_ind->active_config_id, config_id_len);
          g_pdc_info.config_id_len = sel_ind->active_config_id_len;
          PDC_CONFIG_UNLOCK();
          qcril_setup_timed_callback(QCRIL_DEFAULT_INSTANCE_ID,
                                     QCRIL_DEFAULT_MODEM_ID,
                                     qcril_qmi_pdc_retrieve_mbn_info_for_config_id,
                                     NULL, NULL );

        }
        QCRIL_LOG_INFO("current active config id is %s", config_id);

      }
      else if ( sel_ind->pending_config_id_valid ) // selected but not activated
      {
        config_id_len = sel_ind->pending_config_id_len;
        if ( config_id_len >= PDC_CONFIG_ID_SIZE_MAX_V01 )
        {
          result = RIL_E_GENERIC_FAILURE;
        }
        else
        {
          memcpy(config_id, sel_ind->pending_config_id, config_id_len);
        }
         QCRIL_LOG_INFO("current select config id is %s", config_id);
      }
    }
    else // QMI ERROR
    {
      QCRIL_LOG_ERROR("no selected config id");
      result = RIL_E_GENERIC_FAILURE;
    }
  }
  else // ind_data_ptr = NULL
  {
    QCRIL_LOG_ERROR("NULL parameter");
    result = RIL_E_GENERIC_FAILURE;
  }

  // send response
  reqlist_found = qcril_reqlist_query_by_event( QCRIL_DEFAULT_INSTANCE_ID,
                                                QCRIL_DEFAULT_MODEM_ID,
                                                QCRIL_EVT_HOOK_QUERY_MODEM_TEST_MODE,
                                                &req_info );
  if ( reqlist_found == E_SUCCESS )
  {
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                       req_info.t,
                                       req_info.request,
                                       result,
                                       &resp );
    resp.resp_pkt = (void *) config_id;
    resp.resp_len = config_id_len;
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_ERROR("unable to find reqlist by event QCRIL_EVT_HOOK_QUERY_MODEM_TEST_MODE");
  }

  g_pdc_info.is_under_processing = 0;

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_select_configuration

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_RIL_PDC_SELECT_CONFIGURATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_select_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  pdc_set_selected_config_req_msg_v01 qmi_request;
  pdc_set_selected_config_resp_msg_v01 qmi_response;
  pdc_config_info_req_type_v01 *p_config_info;
  qmi_client_error_type qmi_error;
  RIL_Errno ril_req_res;
  char payload;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(params_ptr);
  QCRIL_NOTUSED(ret_ptr);

  memset( &qmi_request, 0, sizeof(qmi_request) );
  p_config_info = &qmi_request.new_config_info;
  p_config_info->config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
  p_config_info->config_id_len = g_pdc_info.pdc_config_info.config_id_len;
  memcpy(p_config_info->config_id, g_pdc_info.pdc_config_info.config_id, p_config_info->config_id_len);
  qmi_request.subscription_id_valid = 1;
  qmi_request.subscription_id = g_pdc_info.cur_select_sub;

  qmi_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PDC ),
                                        QMI_PDC_SET_SELECTED_CONFIG_REQ_V01,
                                        &qmi_request,
                                        sizeof( qmi_request ),
                                        &qmi_response,
                                        sizeof( qmi_response ),
                                        QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT
                                        );

  ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );

  // on failure, send an unsol message to indicate error
  // on success, do nothing here, the select indication handler
  // will handle the remaining things
  if ( ril_req_res != RIL_E_SUCCESS )
  {
    g_pdc_info.is_under_processing = 0;
    payload = QCRIL_PDC_GENERIC_FAIL;
    qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_PDC_CONFIG,
                                    (void *)(&payload), sizeof(payload) );
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_activate_configuration

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_RIL_PDC_ACTIVATE_CONFIGURATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_activate_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  pdc_activate_config_req_msg_v01 qmi_request;
  pdc_activate_config_resp_msg_v01 qmi_response;
  qmi_client_error_type qmi_error;
  RIL_Errno ril_req_res;
  char payload;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(params_ptr);
  QCRIL_NOTUSED(ret_ptr);

  memset( &qmi_request, 0, sizeof(qmi_request) );
  qmi_request.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;

  qmi_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PDC ),
                                        QMI_PDC_ACTIVATE_CONFIG_REQ_V01,
                                        &qmi_request,
                                        sizeof( qmi_request ),
                                        &qmi_response,
                                        sizeof( qmi_response ),
                                        QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT
                                        );

  ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );

  // on failure, send an unsol message to indicate error
  // on success, do nothing here, the activate indication handler
  // will handle the remaining things
  if ( ril_req_res != RIL_E_SUCCESS )
  {
    g_pdc_info.is_under_processing = 0;
    payload = QCRIL_PDC_GENERIC_FAIL;
    qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_PDC_CONFIG,
                                    (void *)(&payload), sizeof(payload) );
  }

  QCRIL_LOG_FUNC_RETURN();
}


/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_delete_configuration

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_RIL_PDC_DELETE_CONFIGURATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_delete_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  pdc_delete_config_req_msg_v01 qmi_request;
  pdc_delete_config_resp_msg_v01 qmi_response;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type req_info;
  qmi_client_error_type qmi_error;
  RIL_Errno result;
  char payload;

  QCRIL_LOG_FUNC_ENTRY();

  if ( g_pdc_info.cur_delete_idx == g_pdc_info.cur_config_len)
  {
    QCRIL_LOG_INFO("delete all loaded configuration");
    result = RIL_E_SUCCESS;
  }
  else
  {
    memset( &qmi_request, 0, sizeof(qmi_request) );
    qmi_request.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
    qmi_request.config_id_valid = 1;
    qmi_request.config_id_len = g_pdc_info.cur_config_lists[g_pdc_info.cur_delete_idx].config_id_len;
    memcpy(qmi_request.config_id, g_pdc_info.cur_config_lists[g_pdc_info.cur_delete_idx].config_id,
                      qmi_request.config_id_len);
    QCRIL_LOG_INFO("request to delete config id: %s, index: %d", qmi_request.config_id, g_pdc_info.cur_delete_idx);
    qmi_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PDC ),
                                          QMI_PDC_DELETE_CONFIG_REQ_V01,
                                          &qmi_request,
                                          sizeof( qmi_request ),
                                          &qmi_response,
                                          sizeof( qmi_response ),
                                          QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT
                                          );
    result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
  }

  // send response, if all configurations are deleted or failed to delete
  if ( ( g_pdc_info.cur_delete_idx == g_pdc_info.cur_config_len ) || ( result != RIL_E_SUCCESS ) )
  {
    g_pdc_info.is_under_processing = 0;
    g_pdc_info.is_under_list_request = 0;
    g_pdc_info.cur_delete_idx = 0;
    payload = ( result == RIL_E_SUCCESS ) ? QCRIL_PDC_NO_ERROR : QCRIL_PDC_GENERIC_FAIL;
    QCRIL_LOG_INFO("delete completed, with error %d", result);
    qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_PDC_CLEAR_CONFIGS,
                                (void *)(&payload), sizeof(payload) );
  }
  else
  {
    g_pdc_info.cur_delete_idx++;
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_list_configuration

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_RIL_PDC_LIST_CONFIGURATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_list_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  pdc_list_configs_req_msg_v01 qmi_request;
  pdc_list_configs_resp_msg_v01 qmi_response;
  qcril_request_resp_params_type resp;
  qmi_client_error_type qmi_error;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  char payload;

  QCRIL_LOG_FUNC_ENTRY();

  memset( &qmi_request, 0, sizeof(qmi_request) );
  qmi_request.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
  qmi_request.config_type_valid = 1;
  qmi_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PDC ),
                                        QMI_PDC_LIST_CONFIGS_REQ_V01,
                                        &qmi_request,
                                        sizeof( qmi_request ),
                                        &qmi_response,
                                        sizeof( qmi_response ),
                                        QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT
                                        );
  result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );

  // on failure, send an unsol message to indicate error
  // on success, do nothing here, the list indication handler
  // will handle the remain things
  if ( result == RIL_E_GENERIC_FAILURE )
  {
    g_pdc_info.is_under_list_request = 0;
    g_pdc_info.is_under_processing = 0;
    payload = QCRIL_PDC_GENERIC_FAIL;
    qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_PDC_CLEAR_CONFIGS,
                                (void *)(&payload), sizeof(payload));
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_deactivate_configuration

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_RIL_PDC_DEACTIVATE_CONFIGURATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_deactivate_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  pdc_deactivate_config_req_msg_v01 qmi_request;
  pdc_deactivate_config_resp_msg_v01 qmi_response;
  qmi_client_error_type qmi_error;
  RIL_Errno result;

  QCRIL_LOG_FUNC_ENTRY();

  memset( &qmi_request, 0, sizeof(qmi_request) );
  qmi_request.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
  qmi_request.subscription_id_valid = 1;
  qmi_request.subscription_id = g_pdc_info.cur_deact_sub;
  qmi_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PDC ),
                                        QMI_PDC_DEACTIVATE_CONFIG_REQ_V01,
                                        &qmi_request,
                                        sizeof( qmi_request ),
                                        &qmi_response,
                                        sizeof( qmi_response ),
                                        QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT
                                        );
  result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );

  // 1. on failure, RIL will continue the delete process
  //    next step is getting all loaded configurations
  // 2. on success, we will handle the next step in the
  //    deactivate qmi indication handler
  if ( result != RIL_E_SUCCESS )
  {
    // kick the LIST_CONFIGS start
    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                   QCRIL_DEFAULT_MODEM_ID,
                   QCRIL_DATA_ON_STACK,
                   QCRIL_EVT_QMI_RIL_PDC_LIST_CONFIGURATION,
                   NULL,
                   QMI_RIL_ZERO,
                   (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
  }

  QCRIL_LOG_FUNC_RETURN();
}


//===========================================================================
// QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS
//===========================================================================
void qcril_qmi_pdc_get_available_configs
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  qcril_reqlist_public_type qcril_req_info;
  pdc_list_configs_req_msg_v01 qmi_request;
  pdc_list_configs_resp_msg_v01 qmi_response;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  qcril_request_resp_params_type resp;
  qmi_client_error_type qmi_error;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                 QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS,
                                 NULL,
                                 &qcril_req_info);
    if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &qcril_req_info ) == E_SUCCESS )
    {
      g_pdc_info.is_user_request_list = 1;
      memset( &qmi_request, 0, sizeof(qmi_request) );
      qmi_request.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
      qmi_request.config_type_valid = 1;
      qmi_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PDC ),
                                            QMI_PDC_LIST_CONFIGS_REQ_V01,
                                            &qmi_request,
                                            sizeof( qmi_request ),
                                            &qmi_response,
                                            sizeof( qmi_response ),
                                            QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT );
     result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
    }
    else
    {
      QCRIL_LOG_ERROR("failed to allocate memory for reqlist");
      result = RIL_E_GENERIC_FAILURE;
    }
  } while (0);

  if ( result == RIL_E_GENERIC_FAILURE )
  {
    g_pdc_info.is_under_processing = 0;
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                       params_ptr->t,
                                       params_ptr->event_id,
                                       result,
                                       &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN();
}



//===========================================================================
//QCRIL_EVT_HOOK_SET_MODEM_TEST_MODE
// param: config_id
// TODO: need to block the call if the previous set is not complete
//===========================================================================
void qcril_qmi_pdc_set_modem_test_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  qcril_request_resp_params_type resp;
  char mbn_file_path[QCRIL_MBN_FILE_PATH_LEN];
  RIL_Errno result = RIL_E_SUCCESS;
  struct stat f_stat;
  char property_name[ 40 ];
  size_t str_size;
  char* str_pos;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ret_ptr);

  do
  {
    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    memset(&g_pdc_info.pdc_config_info, 0, sizeof(pdc_config_info_type));
    g_pdc_info.pdc_config_info.conf_fd = -1;
    if ( (NULL != params_ptr->data) && (1 < params_ptr->datalen) )
    {
        QCRIL_LOG_INFO("data len is %d", params_ptr->datalen);
        g_pdc_info.sub_select_mask = ((uint8_t*)params_ptr->data)[0];
        QCRIL_LOG_INFO("sub_select_mask is %d", g_pdc_info.sub_select_mask);
        // fetch file name
        str_size = strlcpy( mbn_file_path, (const char*)(params_ptr->data+1),
                                                QCRIL_MBN_FILE_PATH_LEN);
        QCRIL_LOG_INFO("mbn file path is %s", mbn_file_path);
        if ( str_size > QCRIL_MBN_FILE_PATH_LEN )
        {
            QCRIL_LOG_ERROR("invalid parameter: file name length too long");
            result = RIL_E_GENERIC_FAILURE;
            break;
        }
        if ( str_size + 1 > params_ptr->datalen )
        {
            QCRIL_LOG_ERROR("invalid parameter: no config_id specified");
            result = RIL_E_GENERIC_FAILURE;
            break;
        }
        // fetch config_id
        str_pos = params_ptr->data + 1 + str_size + 1;
        str_size = strlcpy(g_pdc_info.pdc_config_info.config_id,
                    (const char*)str_pos, PDC_CONFIG_ID_SIZE_MAX_V01);
        QCRIL_LOG_INFO("config id is %s", g_pdc_info.pdc_config_info.config_id);
        if ( str_size > PDC_CONFIG_ID_SIZE_MAX_V01 )
        {
            QCRIL_LOG_ERROR("valid paramter: config_id length too long");
            result = RIL_E_GENERIC_FAILURE;
            break;
        }
        g_pdc_info.pdc_config_info.config_id_len = str_size;
    }
    else
    {
        QCRIL_LOG_ERROR("invalid parameter");
        result = RIL_E_GENERIC_FAILURE;
        break;
    }

    g_pdc_info.sub_count = QCRIL_PDC_MAX_SUBS_SUPPORT;
    g_pdc_info.cur_select_sub = 0;
    // fill the file descripter and the config size
    g_pdc_info.pdc_config_info.conf_fd = open( mbn_file_path, O_RDONLY );
    if ( g_pdc_info.pdc_config_info.conf_fd == -1 )
    {
      QCRIL_LOG_ERROR("Failed to open file %s: %s", mbn_file_path, strerror(errno));
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    if ( fstat( g_pdc_info.pdc_config_info.conf_fd, &f_stat ) == -1 )
    {
      QCRIL_LOG_ERROR("Failed to fstat file: %s", strerror(errno));
      close(g_pdc_info.pdc_config_info.conf_fd);
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.pdc_config_info.conf_size = f_stat.st_size;

    // kick load config start
    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                    QCRIL_DEFAULT_MODEM_ID,
                    QCRIL_DATA_ON_STACK,
                    QCRIL_EVT_QMI_RIL_PDC_LOAD_CONFIGURATION,
                    NULL,
                    QMI_RIL_ZERO,
                    (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );

  } while (0);

  if ( result == RIL_E_GENERIC_FAILURE )
  {
    g_pdc_info.is_under_processing = 0;
  }
  qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                     params_ptr->t,
                                     params_ptr->event_id,
                                     result,
                                     &resp );
  qcril_send_request_response( &resp );
  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
//QCRIL_EVT_HOOK_QUERY_MODEM_TEST_MODE
//===========================================================================
void qcril_qmi_pdc_query_modem_test_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  pdc_get_selected_config_req_msg_v01 qmi_request;
  pdc_get_selected_config_resp_msg_v01 qmi_response;
  qcril_request_resp_params_type resp;
  qmi_client_error_type qmi_error;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  qcril_reqlist_public_type qcril_req_info;
  uint32_t sub_info;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ret_ptr);

  do
  {
    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      break;
    }

    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      break;
    }
    g_pdc_info.is_under_processing = 1;

    if ( params_ptr->data == NULL || params_ptr->datalen < 4 )
    {
      QCRIL_LOG_ERROR("invalid parater");
      break;
    }

    sub_info = ((uint32_t*)params_ptr->data)[0];
    if ( sub_info > QCRIL_PDC_MAX_SUBS_SUPPORT )
    {
      QCRIL_LOG_ERROR("sub index too large");
      break;
    }

    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                 QCRIL_EVT_HOOK_QUERY_MODEM_TEST_MODE,
                                 NULL,
                                 &qcril_req_info);
    if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &qcril_req_info ) == E_SUCCESS )
    {
        memset( &qmi_request, 0, sizeof(qmi_request) );
        qmi_request.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
        qmi_request.subscription_id_valid = 1;
        qmi_request.subscription_id = sub_info;
        qmi_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PDC ),
                                            QMI_PDC_GET_SELECTED_CONFIG_REQ_V01,
                                            &qmi_request,
                                            sizeof( qmi_request ),
                                            &qmi_response,
                                            sizeof( qmi_response ),
                                            QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT );
        result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
    }
    else
    {
        QCRIL_LOG_ERROR("No memory to allocate reqlist");
    }
  } while (0);

  if ( result == RIL_E_GENERIC_FAILURE )
  {
    g_pdc_info.is_under_processing = 0;
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                    params_ptr->t,
                                    params_ptr->event_id,
                                    result,
                                    &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// QCRIL_EVT_HOOK_ACTIVATE_CONFIGS
//===========================================================================
void qcril_qmi_pdc_activate_configs
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  qcril_request_resp_params_type resp;
  qmi_client_error_type qmi_error;
  pdc_activate_config_req_msg_v01 qmi_request;
  pdc_activate_config_resp_msg_v01 qmi_response;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    // base check
    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    // fill the qmi_request
    memset(&qmi_request, 0, sizeof(qmi_request));
    qmi_request.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;

    // send QMI request
    qmi_error = qmi_client_send_msg_sync( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PDC ),
                                          QMI_PDC_ACTIVATE_CONFIG_REQ_V01,
                                          &qmi_request,
                                          sizeof( qmi_request ),
                                          &qmi_response,
                                          sizeof( qmi_response ),
                                          QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT );
    result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
  } while (0);

  g_pdc_info.is_under_processing = 0;
  qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                     params_ptr->t,
                                     params_ptr->event_id,
                                     result,
                                     &resp );
  qcril_send_request_response( &resp );

  QCRIL_LOG_FUNC_RETURN_WITH_RET(result);
}

//===========================================================================
// QCRIL_EVT_HOOK_DEACTIVATE_CONFIGS
//===========================================================================
void qcril_qmi_pdc_deactivate_configs
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  RIL_Errno result;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type qcril_req_info;

  QCRIL_LOG_FUNC_ENTRY();

  if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
  {
    QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
    result = RIL_E_GENERIC_FAILURE;
  }
  else if ( g_pdc_info.is_under_processing )
  {
    QCRIL_LOG_ERROR("QMI PDC is busy");
    result = RIL_E_GENERIC_FAILURE;
  }
  else
  {
    g_pdc_info.is_under_processing = 1;
    g_pdc_info.is_under_list_request = 0;
    // FIXME: hard code sub_count as 3 when deactivate
    // 1. when deactivate all subs, by default we deactivate for SUB0, SUB1, SUB2,
    // in case we miss any. Because if one SUB still has active configuration, we
    // will fail to delete all configurations.
    // 2. And If any SUB is not supported during deactivate or there is no active
    // config on it, MCFG will report error in QMI deactivate indication. And RIL
    // will ignore this error.
    g_pdc_info.sub_count = QCRIL_PDC_MAX_SUBS_SUPPORT;
    g_pdc_info.cur_deact_sub = 0;
    QCRIL_LOG_INFO("The current maximum subscriptions is %d", g_pdc_info.sub_count);

    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                 QCRIL_EVT_HOOK_DEACTIVATE_CONFIGS,
                                 NULL,
                                 &qcril_req_info);
    if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &qcril_req_info ) == E_SUCCESS )
    {
      qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                      QCRIL_DEFAULT_MODEM_ID,
                      QCRIL_DATA_ON_STACK,
                      QCRIL_EVT_QMI_RIL_PDC_DEACTIVATE_CONFIGURATION,
                      NULL,
                      QMI_RIL_ZERO,
                      (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
      result = RIL_E_SUCCESS;
    }
    else
    {
      QCRIL_LOG_ERROR("No memory to allocate reqlist");
      result = RIL_E_GENERIC_FAILURE;
    }
  }

  if ( result == RIL_E_GENERIC_FAILURE )
  {
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                       params_ptr->t,
                                       params_ptr->event_id,
                                       result,
                                       &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// QCRIL_EVT_HOOK_CLEANUP_LOADED_CONFIGS
//===========================================================================
void qcril_qmi_pdc_cleanup_loaded_configs
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  RIL_Errno result;
  qcril_request_resp_params_type resp;

  QCRIL_LOG_FUNC_ENTRY();

  if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
  {
    QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
    result = RIL_E_GENERIC_FAILURE;
  }
  else if ( g_pdc_info.is_under_processing )
  {
    QCRIL_LOG_ERROR("QMI PDC is busy");
    result = RIL_E_GENERIC_FAILURE;
  }
  else
  {
    g_pdc_info.is_under_processing = 1;
    g_pdc_info.is_under_list_request = 1;
    // FIXME: hard code sub_count as 3 when deactivate
    // 1. when deactivate all subs, by default we deactivate for SUB0, SUB1, SUB2,
    // in case we miss any. Because if one SUB still has active configuration, we
    // will fail to delete all configurations.
    // 2. And If any SUB is not supported during deactivate or there is no active
    // config on it, MCFG will report error in QMI deactivate indication. And RIL
    // will ignore this error.
    g_pdc_info.sub_count = QCRIL_PDC_MAX_SUBS_SUPPORT;
    g_pdc_info.cur_deact_sub = 0;
    QCRIL_LOG_INFO("The current maximum subscriptions is %d", g_pdc_info.sub_count);

    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                    QCRIL_DEFAULT_MODEM_ID,
                    QCRIL_DATA_ON_STACK,
                    QCRIL_EVT_QMI_RIL_PDC_DEACTIVATE_CONFIGURATION,
                    NULL,
                    QMI_RIL_ZERO,
                    (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
    result = RIL_E_SUCCESS;
  }

  qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                     params_ptr->t,
                                     params_ptr->event_id,
                                     result,
                                     &resp );
  qcril_send_request_response( &resp );

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// QCRIL_EVT_HOOK_SEL_CONFIG
//===========================================================================
void qcril_qmi_pdc_select_configs
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  RIL_Errno result = RIL_E_SUCCESS;
  qcril_request_resp_params_type resp;

  QCRIL_LOG_FUNC_ENTRY();
  do
  {
    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscription");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    if ( (NULL != params_ptr->data) && (1 < params_ptr->datalen)
                  && ((PDC_CONFIG_ID_SIZE_MAX_V01 + 1) > params_ptr->datalen) )
    {
      g_pdc_info.sub_select_mask = ((uint8_t*)params_ptr->data)[0];
      QCRIL_LOG_INFO("sub_select_mask is %d", g_pdc_info.sub_select_mask);
      memcpy(g_pdc_info.pdc_config_info.config_id, (uint8_t*)params_ptr->data + 1,
                                            params_ptr->datalen - 1);
      g_pdc_info.pdc_config_info.config_id_len = params_ptr->datalen - 1;
    }
    else
    {
      QCRIL_LOG_ERROR("parameter error");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    // submask: each bit represents each SUB and the LSB is for SUB0
    //          value 1 means enable while value 0 means disable
    g_pdc_info.cur_select_sub = 0;
    g_pdc_info.sub_count = QCRIL_PDC_MAX_SUBS_SUPPORT;
    // if no SUB is selected, return directly
    if (!( g_pdc_info.sub_select_mask & ( (1 << g_pdc_info.sub_count) - 1) ))
    {
      QCRIL_LOG_ERROR("no selected subscription");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    // get first sub that needs to be selected
    while ( g_pdc_info.cur_select_sub < g_pdc_info.sub_count )
    {
      // found the first enabled SUB
      if ( (1 << g_pdc_info.cur_select_sub) & g_pdc_info.sub_select_mask )
        break;
      g_pdc_info.cur_select_sub++;
    }

    // kick the select config start
    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                  QCRIL_DEFAULT_MODEM_ID,
                  QCRIL_DATA_ON_STACK,
                  QCRIL_EVT_QMI_RIL_PDC_SELECT_CONFIGURATION,
                  NULL,
                  QMI_RIL_ZERO,
                 (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
  } while (0);

  if ( result == RIL_E_GENERIC_FAILURE )
  {
    g_pdc_info.is_under_processing = 0;
  }
  qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                     params_ptr->t,
                                     params_ptr->event_id,
                                     result,
                                     &resp );
  qcril_send_request_response( &resp );
  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// QCRIL_EVT_HOOK_GET_META_INFO
//===========================================================================
void qcril_qmi_pdc_get_meta_info_of_config
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  pdc_get_config_info_req_msg_v01 qmi_request;
  pdc_get_config_info_resp_msg_v01 qmi_response;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  qcril_reqlist_public_type qcril_req_info;
  qmi_client_error_type qmi_error;
  qcril_request_resp_params_type resp;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    memset( &qmi_request, 0, sizeof(qmi_request) );
    qmi_request.new_config_info.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
    if ( (NULL != params_ptr->data) && (0 < params_ptr->datalen)
                  && (PDC_CONFIG_ID_SIZE_MAX_V01 > params_ptr->datalen) )
    {
      qmi_request.new_config_info.config_id_len = params_ptr->datalen;
      memcpy( qmi_request.new_config_info.config_id,
                  params_ptr->data, params_ptr->datalen );
    }
    else
    {
      QCRIL_LOG_ERROR("invalid parameter for config_id");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                 QCRIL_EVT_HOOK_GET_META_INFO,
                                 NULL,
                                 &qcril_req_info);
    if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &qcril_req_info ) == E_SUCCESS )
    {
      qmi_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PDC ),
                                            QMI_PDC_GET_CONFIG_INFO_REQ_V01,
                                            &qmi_request,
                                            sizeof( qmi_request ),
                                            &qmi_response,
                                            sizeof( qmi_response ),
                                            QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT );
      result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
    }
    else
    {
      QCRIL_LOG_ERROR("No memory to allocate reqlist");
    }
  } while (0);

  if ( result == RIL_E_GENERIC_FAILURE )
  {
    g_pdc_info.is_under_processing = 0;
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                    params_ptr->t,
                                    params_ptr->event_id,
                                    result,
                                    &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN();

}

//===========================================================================
// QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIGID
//===========================================================================
void qcril_qmi_pdc_get_qc_version_of_configid
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  pdc_get_config_info_req_msg_v01 qmi_request;
  pdc_get_config_info_resp_msg_v01 qmi_response;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  qcril_reqlist_public_type qcril_req_info;
  qmi_client_error_type qmi_error;
  qcril_request_resp_params_type resp;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    memset( &qmi_request, 0, sizeof(qmi_request) );
    qmi_request.new_config_info.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
    if ( (NULL != params_ptr->data) && (0 < params_ptr->datalen)
                  && (PDC_CONFIG_ID_SIZE_MAX_V01 > params_ptr->datalen) )
    {
      qmi_request.new_config_info.config_id_len = params_ptr->datalen;
      memcpy( qmi_request.new_config_info.config_id,
                  params_ptr->data, params_ptr->datalen );
    }
    else
    {
      QCRIL_LOG_ERROR("invalid parameter for config_id");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                 QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIGID,
                                 NULL,
                                 &qcril_req_info);
    if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &qcril_req_info ) == E_SUCCESS )
    {
      qmi_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PDC ),
                                            QMI_PDC_GET_CONFIG_INFO_REQ_V01,
                                            &qmi_request,
                                            sizeof( qmi_request ),
                                            &qmi_response,
                                            sizeof( qmi_response ),
                                            QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT );
      result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
    }
    else
    {
      QCRIL_LOG_ERROR("No memory to allocate reqlist");
    }
  } while (0);

  if ( result == RIL_E_GENERIC_FAILURE )
  {
    g_pdc_info.is_under_processing = 0;
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                    params_ptr->t,
                                    params_ptr->event_id,
                                    result,
                                    &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_CONFIGID
//===========================================================================
void qcril_qmi_pdc_get_oem_version_of_configid
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  pdc_get_config_info_req_msg_v01 qmi_request;
  pdc_get_config_info_resp_msg_v01 qmi_response;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  qcril_reqlist_public_type qcril_req_info;
  qmi_client_error_type qmi_error;
  qcril_request_resp_params_type resp;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    memset( &qmi_request, 0, sizeof(qmi_request) );
    qmi_request.new_config_info.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
    if ( (NULL != params_ptr->data) && (0 < params_ptr->datalen)
                  && (PDC_CONFIG_ID_SIZE_MAX_V01 > params_ptr->datalen) )
    {
      qmi_request.new_config_info.config_id_len = params_ptr->datalen;
      memcpy( qmi_request.new_config_info.config_id,
                  params_ptr->data, params_ptr->datalen );
    }
    else
    {
      QCRIL_LOG_ERROR("invalid parameter for config_id");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                 QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_CONFIGID,
                                 NULL,
                                 &qcril_req_info);
    if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &qcril_req_info ) == E_SUCCESS )
    {
      qmi_error = qmi_client_send_msg_sync( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PDC ),
                                            QMI_PDC_GET_CONFIG_INFO_REQ_V01,
                                            &qmi_request,
                                            sizeof( qmi_request ),
                                            &qmi_response,
                                            sizeof( qmi_response ),
                                            QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT );
      result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
    }
    else
    {
      QCRIL_LOG_ERROR("No memory to allocate reqlist");
    }
  } while (0);

  if ( result == RIL_E_GENERIC_FAILURE )
  {
    g_pdc_info.is_under_processing = 0;
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                    params_ptr->t,
                                    params_ptr->event_id,
                                    result,
                                    &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// QCRIL_EVT_HOOK_VALIDATE_CONFIG
//===========================================================================
void qcril_qmi_pdc_validate_config
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  pdc_validate_config_req_msg_v01 qmi_request;
  pdc_validate_config_resp_msg_v01 qmi_response;
  qmi_client_error_type qmi_error;
  qcril_request_resp_params_type resp;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  size_t str_size;
  int len;

  QCRIL_LOG_FUNC_ENTRY();
  do
  {
    g_pdc_info.dump_fd == -1;

    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    // fill qmi request structure & check parameter
    memset( &qmi_request, 0, sizeof(qmi_request) );
    qmi_request.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
    if ( (NULL != params_ptr->data) && (1 < params_ptr->datalen) )
    {
      QCRIL_LOG_INFO("data len is %d", params_ptr->datalen);
      // fetch sub_id
      qmi_request.subscription_id = ((uint8_t*)params_ptr->data)[0];
      qmi_request.subscription_id_valid = 1;
      QCRIL_LOG_INFO("sub_id is %d", qmi_request.subscription_id);
      // fetch config_id
      str_size = strlcpy( qmi_request.config_id,
            (const char*)(params_ptr->data+1), PDC_CONFIG_ID_SIZE_MAX_V01 );
      if ( str_size > PDC_CONFIG_ID_SIZE_MAX_V01 )
      {
        QCRIL_LOG_ERROR("invalid parameter: config_id too long");
        break;
      }
      qmi_request.config_id_valid = 1;
      qmi_request.config_id_len = strlen(qmi_request.config_id);
      QCRIL_LOG_INFO("config id is %s, config_id_len is %d",
                      qmi_request.config_id, qmi_request.config_id_len);
    }
    else
    {
      QCRIL_LOG_ERROR("invalid parameter");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    // create DIFF dump file
    len = snprintf(g_pdc_info.dump_file, QCRIL_DUMP_FILE_PATH_LEN,
            QCRIL_DUMP_FILE_PREFIX"%d.xml", qmi_request.subscription_id);
    if ( len == QCRIL_DUMP_FILE_PATH_LEN )
    {
      QCRIL_LOG_ERROR("unexpected: dump length exceed");
      break;
    }
    // permission should be "radio:radio -rw-r--r--"
    g_pdc_info.dump_fd = open(g_pdc_info.dump_file, O_WRONLY | O_CREAT | O_TRUNC,
                                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if ( g_pdc_info.dump_fd == -1 )
    {
      QCRIL_LOG_ERROR("failed to create dump file: %s, %s",
                                    g_pdc_info.dump_file, strerror(errno));
      break;
    }

    // send validate request to MCFG
    qmi_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PDC ),
                                          QMI_PDC_VALIDATE_CONFIG_REQ_V01,
                                          &qmi_request,
                                          sizeof( qmi_request ),
                                          &qmi_response,
                                          sizeof( qmi_response ),
                                          QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT );

    result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp);
  } while (0);

  if ( result == RIL_E_GENERIC_FAILURE )
  {
    g_pdc_info.is_under_processing = 0;
    if ( g_pdc_info.dump_fd != -1 )
      close(g_pdc_info.dump_fd);
  }
  qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                     params_ptr->t,
                                     params_ptr->event_id,
                                     result,
                                     &resp );
  qcril_send_request_response( &resp );
  QCRIL_LOG_FUNC_RETURN();

}
/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Callback for QMI PDC indications

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_unsol_ind_cb
(
  qmi_client_type       user_handle,
  unsigned long         msg_id,
  unsigned char         *ind_buf,
  int                   ind_buf_len,
  void                  *ind_cb_data
)
{
  uint32_t decoded_payload_len = 0;
  qmi_client_error_type qmi_err;
  void* decoded_payload = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED( user_handle );
  QCRIL_NOTUSED( ind_cb_data );

  qmi_err = qmi_idl_get_message_c_struct_len(qcril_qmi_client_get_service_object(QCRIL_QMI_CLIENT_PDC),
                                                QMI_IDL_INDICATION,
                                                msg_id,
                                                &decoded_payload_len);
  if ( qmi_err == QMI_NO_ERR )
  {
    decoded_payload = qcril_malloc( decoded_payload_len );
    if ( NULL != decoded_payload )
    {
      qmi_err = qmi_client_message_decode(qcril_qmi_client_get_user_handle(QCRIL_QMI_CLIENT_PDC),
                                          QMI_IDL_INDICATION,
                                          msg_id,
                                          ind_buf,
                                          ind_buf_len,
                                          decoded_payload,
                                          (int)decoded_payload_len);

      if (QMI_NO_ERR == qmi_err)
      {
        switch(msg_id)
        {
          case QMI_PDC_LOAD_CONFIG_IND_V01:
            // TODO: add load configure indication process
            qcril_qmi_pdc_load_config_ind_hdlr(decoded_payload, decoded_payload_len);
            break;

          case QMI_PDC_SET_SELECTED_CONFIG_IND_V01:
            qcril_qmi_pdc_select_config_ind_hdlr(decoded_payload, decoded_payload_len);
            break;

          case QMI_PDC_ACTIVATE_CONFIG_IND_V01:
            qcril_qmi_pdc_activate_config_ind_hdlr(decoded_payload, decoded_payload_len);
            break;

          case QMI_PDC_GET_SELECTED_CONFIG_IND_V01:
            qcril_qmi_pdc_get_selected_config_ind_hdlr(decoded_payload, decoded_payload_len);
            break;

          case QMI_PDC_DEACTIVATE_CONFIG_IND_V01:
            qcril_qmi_pdc_deactivate_config_ind_hdlr(decoded_payload, decoded_payload_len);
            break;

          case QMI_PDC_LIST_CONFIGS_IND_V01:
            qcril_qmi_pdc_list_configs_ind_hdlr(decoded_payload, decoded_payload_len);
            break;

          case QMI_PDC_DELETE_CONFIG_IND_V01:
            qcril_qmi_pdc_delete_config_ind_hdlr(decoded_payload, decoded_payload_len);
            break;

          case QMI_PDC_GET_CONFIG_INFO_IND_V01:
            qcril_qmi_pdc_get_config_info_ind_hdlr(decoded_payload, decoded_payload_len);
            break;

          case QMI_PDC_VALIDATE_CONFIG_IND_V01:
            qcril_qmi_pdc_validate_config_ind_hdlr(decoded_payload, decoded_payload_len);
            break;

          default:
            QCRIL_LOG_INFO("Unsupported QMI PDC indication %x hex", msg_id);
            break;
        }
      }
      else
      {
        QCRIL_LOG_INFO("Indication decode failed for msg %d of svc %d with error %d", msg_id, QCRIL_QMI_CLIENT_PDC, qmi_err );
      }

      qcril_free(decoded_payload);
    }
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int)qmi_err );
}

/*===========================================================================

  FUNCTION:  qcril_qmi_pdc_init

===========================================================================*/
/*!
    @brief
    Initialize the PDC subsystem of the RIL.

    @return
    None.
*/
 /*=========================================================================*/

qmi_client_error_type qcril_qmi_pdc_init
(
  void
)
{
  qmi_client_error_type qmi_err = QMI_NO_ERR;
  char property_name[ 40 ];

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_MODEM_MBN_FILE_PATH );
  property_get( property_name, g_pdc_info.mbn_file_dir, QCRIL_MODEM_MBN_DEFAULT_PATH );
  /* TODO: check if the data path is valid */

  pthread_mutexattr_init( &g_pdc_info.pdc_config_mutex_attr );
  pthread_mutex_init( &g_pdc_info.pdc_config_mutex, &g_pdc_info.pdc_config_mutex_attr );

  QCRIL_LOG_FUNC_RETURN_WITH_RET(qmi_err);

  return (qmi_err);
}

/*===========================================================================

  FUNCTION  qcril_qmi_pdc_retrieve_current_mbn_info

===========================================================================*/
/*!
    @brief
    retrieve active mbn info

    @return
    0 on success
*/
/*=========================================================================*/
void qcril_qmi_pdc_retrieve_current_mbn_info
(
    void
)
{
    pdc_get_selected_config_req_msg_v01  qmi_request;
    pdc_get_selected_config_resp_msg_v01 qmi_response;
    qmi_client_error_type                qmi_error;
    RIL_Errno 				 result = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();
    memset(&qmi_request, 0, sizeof(qmi_request));

    qmi_request.config_type           = PDC_CONFIG_TYPE_MODEM_SW_V01;
    qmi_request.subscription_id_valid = 1;
    qmi_request.subscription_id       = 0;
    qmi_error = qmi_client_send_msg_sync_with_shm(qcril_qmi_client_get_user_handle(QCRIL_QMI_CLIENT_PDC),
                                         QMI_PDC_GET_SELECTED_CONFIG_REQ_V01,
                                         &qmi_request,
                                         sizeof( qmi_request ),
                                         &qmi_response,
                                         sizeof( qmi_response ),
                                         QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT);

    result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
    if (result == RIL_E_SUCCESS)
    {
        PDC_CONFIG_LOCK();

        /* update state that retrieveing mbn info */
        g_pdc_info.is_retrieving_mbn_info = QCRIL_PDC_MBN_INFO_RETRIEVE_ACTIVE_ID;
        PDC_CONFIG_UNLOCK();
    }
    else
    {
        PDC_CONFIG_LOCK();

        /* update state that retrieveing active mbn info */
        g_pdc_info.is_retrieving_mbn_info = QCRIL_PDC_MBN_INFO_NONE;
        qcril_request_clean_up_suppress_list();
        PDC_CONFIG_UNLOCK();
    }
    QCRIL_LOG_FUNC_RETURN_WITH_RET(result);
}

/*===========================================================================

  FUNCTION  qcril_qmi_pdc_retrieve_mbn_info_for_config_id

===========================================================================*/
/*!
    @brief
    retrieve mbn info for config id

    @return
    0 on success
*/
/*=========================================================================*/
void qcril_qmi_pdc_retrieve_mbn_info_for_config_id
(
    void
)
{
    pdc_get_config_info_req_msg_v01      qmi_request;
    pdc_get_config_info_resp_msg_v01     qmi_response;
    qmi_client_error_type                qmi_error;
    RIL_Errno 				             result = RIL_E_GENERIC_FAILURE;
    uint8_t                              config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
    uint32_t                             config_id_len;

    QCRIL_LOG_FUNC_ENTRY();
    memset(&qmi_request, 0, sizeof(qmi_request));
    qmi_request.new_config_info.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;

    PDC_CONFIG_LOCK();
    g_pdc_info.is_under_processing = 1;
    memcpy(config_id, g_pdc_info.config_id, g_pdc_info.config_id_len);
    config_id_len = g_pdc_info.config_id_len;
    PDC_CONFIG_UNLOCK();

    if ((0 < config_id_len) &&
         (config_id_len <= PDC_CONFIG_ID_SIZE_MAX_V01))
    {
      qmi_request.new_config_info.config_id_len = config_id_len;
      memcpy(qmi_request.new_config_info.config_id,
                  config_id, config_id_len);
      qmi_error = qmi_client_send_msg_sync_with_shm(qcril_qmi_client_get_user_handle(QCRIL_QMI_CLIENT_PDC),
                                         QMI_PDC_GET_CONFIG_INFO_REQ_V01,
                                         &qmi_request,
                                         sizeof( qmi_request ),
                                         &qmi_response,
                                         sizeof( qmi_response ),
                                         QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT);


      result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
    }

    if (result == RIL_E_SUCCESS)
    {
        PDC_CONFIG_LOCK();

        /* update state that retrieveing active mbn info */
        g_pdc_info.is_retrieving_mbn_info = QCRIL_PDC_MBN_INFO_RETRIEVE_CONFIG_INFO;
        PDC_CONFIG_UNLOCK();
    }
    else
    {
        PDC_CONFIG_LOCK();

        g_pdc_info.is_under_processing = 0;
        /* update state that retrieveing active mbn info */
        g_pdc_info.is_retrieving_mbn_info = QCRIL_PDC_MBN_INFO_NONE;
        qcril_request_clean_up_suppress_list();
        PDC_CONFIG_UNLOCK();
    }
    QCRIL_LOG_FUNC_RETURN_WITH_RET(result);
}

/*===========================================================================

  FUNCTION  qcril_qmi_pdc_is_test_mbn

===========================================================================*/
/*!
    @brief
    check if the mbn is test mbn

    @return
    0 on success
*/
/*=========================================================================*/
boolean qcril_qmi_pdc_is_test_mbn(void *data, unsigned int len)
{
    boolean ret = FALSE;
    char text[PDC_CONFIG_DESC_SIZE_MAX_V01 + 1];
    int min_mbn_len;
    do
    {
        if ((data == NULL) || (len == 0) || (len > PDC_CONFIG_DESC_SIZE_MAX_V01))
        {
            break;
        }

        memset (text, 0, PDC_CONFIG_DESC_SIZE_MAX_V01 + 1);
        memcpy (text, data, len);

        if (!strcasestr(text, QCRIL_PDC_COMMERCIAL_MBN_PREFIX))
        {
            /* This is test mbn */
            ret = TRUE;
        }

    } while (FALSE);
    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
}
