/*===========================================================================
  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*===========================================================================

                            INCLUDE FILES

===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "comdef.h"
#include "diag_lsm.h"
#include "rsu_lib.h"
#include "user_identity_module_v01.h"
#include "common_v01.h"
#include "qmi_client.h"
#include "qmi_client_instance_defs.h"
#include "qmi_cci_target_ext.h"

/*===========================================================================

                           DEFINES

===========================================================================*/

#define RSU_LOG_ERROR(...) \
  printf(__VA_ARGS__); \
  printf("\n")
#define RSU_LOG_INFO(...) \
  printf(__VA_ARGS__); \
  printf("\n")

#define TRUE                                    1
#define FALSE                                   0

#define RSU_QMI_MAX_TIMEOUT                  5000
#define RSU_QMI_CLIENT_SRV_REG_TIMEOUT       5000

#define RSU_IMSI_LEN                           15
#define RSU_EF_IMSI_LEN                         9
#define RSU_GET_MODEM_STATUS_BUFFER_MIN_LEN    16
#define RSU_UNLOCK_STATUS_OFFSET                3
#define RSU_TEMP_UNLOCK_TIME_OFFSET             4
#define RSU_NUM_MCC_MNC_SET_OFFSET              8
#define RSU_MCC_MNC_OFFSET                     10
#define RSU_GID1_OFFSET                        14

#define MODEM_SIMLOCK_VERSION_LEN               4
#define MODEM_STATUS_LEN                       16
#define MODEM_STATUS_LEN_NO_MCC_MNC_GID        10

#define MODEM_SIMLOCK_VERSION          0x00010005

static qmi_client_type            client_handle = NULL;

/*===========================================================================

                           FUNCTIONS

===========================================================================*/

static void* rsu_malloc(size_t size)
{
  void *temp_ptr;

  temp_ptr = NULL;

  if(0 < size)
  {
    temp_ptr = malloc(size);
    if(temp_ptr)
    {
      memset(temp_ptr, 0x00, size);
    }
    else
    {
      RSU_LOG_ERROR("Failed to allocate memory");
    }
  }
  else
  {
    RSU_LOG_ERROR("size of memory to be allocated is invalid");
  }

  return temp_ptr;
} /* rsu_malloc */


static void rsu_free(void** to_be_freed_memory_ptr)
{
  if(to_be_freed_memory_ptr)
  {
    if(NULL != *to_be_freed_memory_ptr)
    {
      free(*to_be_freed_memory_ptr);
      *to_be_freed_memory_ptr = NULL;
    }
  }
} /* rsu_free */


static int32_t rsu_remote_unlock_cmd
(
  uint32_t            request_type,
  uint8_t           * buffer_ptr,
  uint32_t            buffer_len,
  uint32_t            payload_len
)
{
  qmi_client_error_type            qmi_status   = QMI_NO_ERR;
  uim_remote_unlock_req_msg_v01  * rsu_req_ptr  = NULL;
  uim_remote_unlock_resp_msg_v01 * rsu_resp_ptr = NULL;
  int32_t                          rsu_status   = MODEM_STATUS_OK;

  if(buffer_ptr == NULL ||
     buffer_len == 0)
  {
    return MODEM_STATUS_BUFFER_TOO_SHORT;
  }

  rsu_req_ptr = rsu_malloc(sizeof(uim_remote_unlock_req_msg_v01));
  if(rsu_req_ptr == NULL)
  {
    return MODEM_STATUS_COMMAND_FAILED;
  }

  rsu_resp_ptr = rsu_malloc(sizeof(uim_remote_unlock_resp_msg_v01));
  if(rsu_resp_ptr == NULL)
  {
    rsu_free((void**)&rsu_req_ptr);
    return MODEM_STATUS_COMMAND_FAILED;
  }

  memset(rsu_req_ptr, 0x00, sizeof(uim_remote_unlock_req_msg_v01));
  memset(rsu_resp_ptr, 0x00, sizeof(uim_remote_unlock_resp_msg_v01));

  if (request_type == MODEM_REQUEST_UPDATE_SIMLOCK_SETTINGS)
  {
    if (payload_len == 0 ||
        payload_len > QMI_UIM_SIMLOCK_DATA_MAX_V01)
    {
      rsu_free((void**)&rsu_req_ptr);
      rsu_free((void**)&rsu_resp_ptr);
      return MODEM_STATUS_BUFFER_TOO_SHORT;
    }

    rsu_req_ptr->simlock_data_valid = TRUE;
    rsu_req_ptr->simlock_data_len = payload_len;
    memcpy(rsu_req_ptr->simlock_data,
           buffer_ptr,
           payload_len);
  }

  qmi_status = qmi_client_send_msg_sync(client_handle,
                                        QMI_UIM_REMOTE_UNLOCK_REQ_V01,
                                        rsu_req_ptr,
                                        sizeof(uim_remote_unlock_req_msg_v01),
                                        rsu_resp_ptr,
                                        sizeof(uim_remote_unlock_resp_msg_v01),
                                        RSU_QMI_MAX_TIMEOUT);
  do
  {
    if(qmi_status != QMI_NO_ERR ||
       rsu_resp_ptr->resp.result != QMI_RESULT_SUCCESS_V01)
    {
      RSU_LOG_INFO("remote unlock error qmi: 0x%x, error: 0x%x",
                   qmi_status, rsu_resp_ptr->resp.error);
      rsu_status = MODEM_STATUS_COMMAND_FAILED;
      break;
    }

    if(request_type == MODEM_REQUEST_GET_SHARED_KEY)
    {
      /* Memset buffer before copying response
         Hardcode of key mod envelope(1st byte in buffer) also is taken care in this memset */
      memset(buffer_ptr, 0x00, buffer_len);

      if(rsu_resp_ptr->encrypted_key_valid)
      {
        if (rsu_resp_ptr->encrypted_key_len == 0 ||
            rsu_resp_ptr->encrypted_key_len + sizeof(uint8_t) > buffer_len)
        {
          rsu_status = MODEM_STATUS_COMMAND_FAILED;
          break;
        }

        memcpy(&buffer_ptr[1],
               rsu_resp_ptr->encrypted_key,
               rsu_resp_ptr->encrypted_key_len);

        /* Populate the responds payload length as the return value */
        rsu_status = sizeof(uint8_t) + rsu_resp_ptr->encrypted_key_len;
      }
      else
      {
        rsu_status = MODEM_STATUS_COMMAND_FAILED;
        break;
      }
    }
  } while(0);

  rsu_free((void**)&rsu_req_ptr);
  rsu_free((void**)&rsu_resp_ptr);

  return rsu_status;
} /* rsu_remote_unlock_cmd */


static int32_t rsu_get_simlock_version
(
  uint8_t           * buffer_ptr,
  uint32_t            buffer_len
)
{
  uint32_t simlock_version     = 0;
  uint8_t  simlock_version_len = sizeof(simlock_version);

  if(buffer_ptr == NULL ||
     buffer_len < simlock_version_len)
  {
    return MODEM_STATUS_BUFFER_TOO_SHORT;
  }

  memset(buffer_ptr, 0x00, buffer_len);

  simlock_version = htonl(MODEM_SIMLOCK_VERSION);

  memcpy(buffer_ptr, (uint8_t*)&simlock_version, simlock_version_len);

  return MODEM_SIMLOCK_VERSION_LEN;
} /* rsu_get_simlock_version */


static uint32_t rsu_reset_simlock_settings
(
  void
)
{
  qmi_client_error_type                       qmi_status                 = QMI_NO_ERR;
  int32_t                                     rsu_status                 = MODEM_STATUS_OK;
  uim_depersonalization_secure_req_msg_v01  * rsu_deperso_req_ptr        = NULL;
  uim_depersonalization_secure_resp_msg_v01 * rsu_deperso_resp_ptr       = NULL;
  uim_personalization_secure_req_msg_v01    * rsu_perso_req_ptr          = NULL;
  uim_personalization_secure_resp_msg_v01   * rsu_perso_resp_ptr         = NULL;
  uint8_t                                     deperso_nw_req_data[]    = {0x56, 0x00, 0x05, 0x00,
                                                                          0x00, 0x00, 0x00, 0x00,
                                                                          0x01, 0x00, 0x00, 0x00,
                                                                          0x00, 0x00, 0x00, 0x00};
  uint8_t                                     deperso_sp_req_data[]    = {0x56, 0x00, 0x06, 0x00,
                                                                          0x00, 0x00, 0x00, 0x00,
                                                                          0x01, 0x02, 0x00, 0x00,
                                                                          0x00, 0x00, 0x00, 0x00};
  uint8_t                                     perso_nw_req_data[]     =
                                                 {0x57, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x01, 0x00, 0x13,
                                                  0x30, 0x30, 0x30, 0x02, 0x30, 0x31,
                                                  0x33, 0x31, 0x30, 0x03, 0x31, 0x36, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x32, 0x30, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x32, 0x31, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x32, 0x32, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x32, 0x33, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x32, 0x34, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x32, 0x35, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x32, 0x36, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x32, 0x37, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x33, 0x30, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x33, 0x31, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x34, 0x39, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x35, 0x33, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x35, 0x38, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x35, 0x39, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x36, 0x34, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x36, 0x36, 0x30,
                                                  0x33, 0x31, 0x30, 0x03, 0x38, 0x30, 0x30};

  rsu_deperso_req_ptr = rsu_malloc(sizeof(uim_depersonalization_secure_req_msg_v01));
  if(rsu_deperso_req_ptr == NULL)
  {
    rsu_status = MODEM_STATUS_COMMAND_FAILED;
    goto done;
  }

  rsu_deperso_resp_ptr = rsu_malloc(sizeof(uim_depersonalization_secure_resp_msg_v01));
  if(rsu_deperso_resp_ptr == NULL)
  {
    rsu_status = MODEM_STATUS_COMMAND_FAILED;
    goto done;
  }

  rsu_perso_req_ptr = rsu_malloc(sizeof(uim_personalization_secure_req_msg_v01));
  if(rsu_perso_req_ptr == NULL)
  {
    rsu_status = MODEM_STATUS_COMMAND_FAILED;
    goto done;
  }

  rsu_perso_resp_ptr = rsu_malloc(sizeof(uim_personalization_secure_resp_msg_v01));
  if(rsu_perso_resp_ptr == NULL)
  {
    rsu_status = MODEM_STATUS_COMMAND_FAILED;
    goto done;
  }

  memset(rsu_deperso_req_ptr, 0x00, sizeof(uim_depersonalization_secure_req_msg_v01));
  memset(rsu_deperso_resp_ptr, 0x00, sizeof(uim_depersonalization_secure_resp_msg_v01));
  memset(rsu_perso_req_ptr, 0x00, sizeof(uim_personalization_secure_req_msg_v01));
  memset(rsu_perso_resp_ptr, 0x00, sizeof(uim_personalization_secure_resp_msg_v01));

  memcpy(rsu_deperso_req_ptr->encrypted_depersonalization_data, &deperso_nw_req_data, sizeof(deperso_nw_req_data));
  rsu_deperso_req_ptr->encrypted_depersonalization_data_len = sizeof(deperso_nw_req_data);

  qmi_status = qmi_client_send_msg_sync(client_handle,
                                        QMI_UIM_DEPERSONALIZATION_SECURE_REQ_V01,
                                        rsu_deperso_req_ptr,
                                        sizeof(uim_depersonalization_secure_req_msg_v01),
                                        rsu_deperso_resp_ptr,
                                        sizeof(uim_depersonalization_secure_resp_msg_v01),
                                        RSU_QMI_MAX_TIMEOUT);
  if(qmi_status != QMI_NO_ERR ||
     (rsu_deperso_resp_ptr->resp.result != QMI_RESULT_SUCCESS_V01 &&
      rsu_deperso_resp_ptr->resp.error != QMI_ERR_NO_EFFECT_V01))
  {
    RSU_LOG_INFO("deperso nw error qmi: 0x%x, error: 0x%x",
                 qmi_status, rsu_deperso_resp_ptr->resp.error);
    rsu_status = MODEM_STATUS_COMMAND_FAILED;
    goto done;
  }

  memset(rsu_deperso_req_ptr, 0x00, sizeof(uim_depersonalization_secure_req_msg_v01));
  memset(rsu_deperso_resp_ptr, 0x00, sizeof(uim_depersonalization_secure_resp_msg_v01));

  memcpy(rsu_deperso_req_ptr->encrypted_depersonalization_data, &deperso_sp_req_data, sizeof(deperso_sp_req_data));
  rsu_deperso_req_ptr->encrypted_depersonalization_data_len = sizeof(deperso_sp_req_data);

  qmi_status = qmi_client_send_msg_sync(client_handle,
                                        QMI_UIM_DEPERSONALIZATION_SECURE_REQ_V01,
                                        rsu_deperso_req_ptr,
                                        sizeof(uim_depersonalization_secure_req_msg_v01),
                                        rsu_deperso_resp_ptr,
                                        sizeof(uim_depersonalization_secure_resp_msg_v01),
                                        RSU_QMI_MAX_TIMEOUT);
  if(qmi_status != QMI_NO_ERR ||
     (rsu_deperso_resp_ptr->resp.result != QMI_RESULT_SUCCESS_V01 &&
      rsu_deperso_resp_ptr->resp.error != QMI_ERR_NO_EFFECT_V01))
  {
    RSU_LOG_INFO("deperso sp error qmi: 0x%x, error: 0x%x",
                 qmi_status, rsu_deperso_resp_ptr->resp.error);
    rsu_status MODEM_STATUS_COMMAND_FAILED;
    goto done;
  }

  memset(rsu_deperso_req_ptr, 0x00, sizeof(uim_depersonalization_secure_req_msg_v01));
  memset(rsu_deperso_resp_ptr, 0x00, sizeof(uim_depersonalization_secure_resp_msg_v01));

  memcpy(rsu_perso_req_ptr->encrypted_personalization_data, &perso_nw_req_data, sizeof(perso_nw_req_data));
  rsu_perso_req_ptr->encrypted_personalization_data_len = sizeof(perso_nw_req_data);

  qmi_status = qmi_client_send_msg_sync(client_handle,
                                        QMI_UIM_PERSONALIZATION_SECURE_REQ_V01,
                                        rsu_perso_req_ptr,
                                        sizeof(uim_personalization_secure_req_msg_v01),
                                        rsu_perso_resp_ptr,
                                        sizeof(uim_personalization_secure_resp_msg_v01),
                                        RSU_QMI_MAX_TIMEOUT);
  if(qmi_status != QMI_NO_ERR ||
     rsu_perso_resp_ptr->resp.result != QMI_RESULT_SUCCESS_V01)
  {
    RSU_LOG_INFO("perso error qmi: 0x%x, error: 0x%x",
                 qmi_status, rsu_perso_resp_ptr->resp.error);
    rsu_status = MODEM_STATUS_COMMAND_FAILED;
    goto done;
  }

  /* Clear SimLock data before continue so the information does not stay in memory,
     vulnerable to attack. */
  memset(rsu_perso_req_ptr, 0x00, sizeof(uim_personalization_secure_req_msg_v01));
  memset(rsu_perso_resp_ptr, 0x00, sizeof(uim_personalization_secure_resp_msg_v01));

done:
  rsu_free((void**)&rsu_deperso_req_ptr);
  rsu_free((void**)&rsu_deperso_resp_ptr);
  rsu_free((void**)&rsu_perso_req_ptr);
  rsu_free((void**)&rsu_perso_resp_ptr);

  return rsu_status;
} /* rsu_reset_simlock_settings */


static int32_t rsu_get_modem_status
(
  uint8_t           * buffer_ptr,
  uint32_t            buffer_len
)
{
  qmi_client_error_type                qmi_status               = QMI_NO_ERR;
  uim_get_configuration_req_msg_v01    rsu_config_req;
  uim_get_configuration_resp_msg_v01 * rsu_config_resp_ptr      = NULL;
  uim_get_card_status_req_msg_v01      rsu_card_status_req;
  uim_get_card_status_resp_msg_v01   * rsu_card_status_resp_ptr = NULL;
  uim_read_transparent_req_msg_v01   * rsu_read_req_ptr         = NULL;
  uim_read_transparent_resp_msg_v01  * rsu_read_resp_ptr        = NULL;
  uint8_t                              sim_path[]               = {0x00, 0x3F, 0x20, 0x7F};
  uint8_t                              usim_path[]              = {0x00, 0x3F, 0xFF, 0x7F};
  uint8_t                              i                        = 0;
  uint16_t                             num_imsi                 = 0x0100;
  uint16_t                             card_index               = 0;
  uint16_t                             app_index                = 0;
  uim_app_type_enum_v01                app_type                 = UIM_APP_TYPE_UNKNOWN_V01;
  uim_file_id_type_v01                 file_id;
  uint8_t                              mnc_len                  = 0;

  if(buffer_ptr == NULL ||
     buffer_len < RSU_GET_MODEM_STATUS_BUFFER_MIN_LEN)
  {
    return MODEM_STATUS_BUFFER_TOO_SHORT;
  }

  memset(buffer_ptr, 0x00, buffer_len);
  memset(&file_id, 0x00, sizeof(file_id));

  rsu_config_resp_ptr = rsu_malloc(sizeof(uim_get_configuration_resp_msg_v01));
  if(rsu_config_resp_ptr == NULL)
  {
    return MODEM_STATUS_COMMAND_FAILED;
  }

  memset(&rsu_config_req, 0x00, sizeof(uim_get_configuration_req_msg_v01));
  memset(rsu_config_resp_ptr, 0x00, sizeof(uim_get_configuration_resp_msg_v01));

  rsu_config_req.configuration_mask_valid = TRUE;
  rsu_config_req.configuration_mask = UIM_GET_CONFIGURATION_PERSONALIZATION_STATUS_V01;

  qmi_status = qmi_client_send_msg_sync(client_handle,
                                        QMI_UIM_GET_CONFIGURATION_REQ_V01,
                                        &rsu_config_req,
                                        sizeof(uim_get_configuration_req_msg_v01),
                                        rsu_config_resp_ptr,
                                        sizeof(uim_get_configuration_resp_msg_v01),
                                        RSU_QMI_MAX_TIMEOUT);
  if(qmi_status != QMI_NO_ERR ||
     rsu_config_resp_ptr->resp.result != QMI_RESULT_SUCCESS_V01)
  {
    rsu_free((void**)&rsu_config_resp_ptr);
    return MODEM_STATUS_COMMAND_FAILED;
  }

  buffer_ptr[RSU_UNLOCK_STATUS_OFFSET] = MODEM_LOCK_STATE_PERMANENT_UNLOCK;

  if(rsu_config_resp_ptr->personalization_status_valid)
  {
    for(i = 0; i < rsu_config_resp_ptr->personalization_status_len &&
               i < QMI_UIM_PERSO_FEATURE_MAX_V01; i++)
    {
      if(rsu_config_resp_ptr->personalization_status[i].feature == UIM_PERSO_FEATURE_GW_SERVICE_PROVIDER_V01 ||
         rsu_config_resp_ptr->personalization_status[i].feature == UIM_PERSO_FEATURE_GW_NETWORK_V01)
      {
        buffer_ptr[RSU_UNLOCK_STATUS_OFFSET] = MODEM_LOCK_STATE_LOCKED;
        break;
      }
    }
  }

  if(rsu_config_resp_ptr->temporary_depersonalization_status_valid)
  {
    for(i = 0; i < rsu_config_resp_ptr->temporary_depersonalization_status[0].temporary_unlock_status_len &&
               i < QMI_UIM_PERSO_FEATURE_MAX_V01; i++)
    {
      if(rsu_config_resp_ptr->temporary_depersonalization_status[0].temporary_unlock_status[i].feature == UIM_PERSO_FEATURE_GW_NETWORK_V01 ||
         rsu_config_resp_ptr->temporary_depersonalization_status[0].temporary_unlock_status[i].feature == UIM_PERSO_FEATURE_GW_SERVICE_PROVIDER_V01)
      {
        uint32_t temp_unlock_time_left = 0;

        buffer_ptr[RSU_UNLOCK_STATUS_OFFSET] = MODEM_LOCK_STATE_TEMPORARY_UNLOCK;

        temp_unlock_time_left = htonl(rsu_config_resp_ptr->temporary_depersonalization_status[0].temporary_unlock_status[i].temporary_unlock_time_left);

        memcpy(&buffer_ptr[RSU_TEMP_UNLOCK_TIME_OFFSET],
               (uint8_t*)&temp_unlock_time_left,
               sizeof(temp_unlock_time_left));
        break;
      }
    }
  }

  rsu_free((void**)&rsu_config_resp_ptr);

  rsu_card_status_resp_ptr = rsu_malloc(sizeof(uim_get_card_status_resp_msg_v01));
  if(rsu_card_status_resp_ptr == NULL)
  {
    return MODEM_STATUS_COMMAND_FAILED;
  }

  memset(&rsu_card_status_req, 0x00, sizeof(uim_get_card_status_req_msg_v01));
  memset(rsu_card_status_resp_ptr, 0x00, sizeof(uim_get_card_status_resp_msg_v01));

  qmi_status = qmi_client_send_msg_sync(client_handle,
                                        QMI_UIM_GET_CARD_STATUS_REQ_V01,
                                        &rsu_card_status_req,
                                        sizeof(uim_get_card_status_req_msg_v01),
                                        rsu_card_status_resp_ptr,
                                        sizeof(uim_get_card_status_resp_msg_v01),
                                        RSU_QMI_MAX_TIMEOUT);
  if(qmi_status != QMI_NO_ERR ||
     rsu_card_status_resp_ptr->resp.result != QMI_RESULT_SUCCESS_V01 ||
     !rsu_card_status_resp_ptr->card_status_valid ||
     rsu_card_status_resp_ptr->card_status.index_gw_pri == 0xFFFF)
  {
    rsu_free((void**)&rsu_card_status_resp_ptr);
    return MODEM_STATUS_COMMAND_FAILED;
  }

  card_index = rsu_card_status_resp_ptr->card_status.index_gw_pri >> 8;
  app_index = rsu_card_status_resp_ptr->card_status.index_gw_pri & 0x00FF;

  if(card_index >= rsu_card_status_resp_ptr->card_status.card_info_len ||
     card_index >= QMI_UIM_CARDS_MAX_V01 ||
     app_index >= rsu_card_status_resp_ptr->card_status.card_info[card_index].app_info_len ||
     app_index >= QMI_UIM_APPS_MAX_V01)
  {
    rsu_free((void**)&rsu_card_status_resp_ptr);
    return MODEM_STATUS_COMMAND_FAILED;
  }

  app_type = rsu_card_status_resp_ptr->card_status.card_info[card_index].app_info[app_index].app_type;

  rsu_free((void**)&rsu_card_status_resp_ptr);

  if(app_type != UIM_APP_TYPE_SIM_V01 &&
     app_type != UIM_APP_TYPE_USIM_V01)
  {
    return MODEM_STATUS_COMMAND_FAILED;
  }

  rsu_read_req_ptr = rsu_malloc(sizeof(uim_read_transparent_req_msg_v01));
  if(rsu_read_req_ptr == NULL)
  {
    return MODEM_STATUS_LEN_NO_MCC_MNC_GID;
  }

  rsu_read_resp_ptr = rsu_malloc(sizeof(uim_read_transparent_resp_msg_v01));
  if(rsu_read_resp_ptr == NULL)
  {
    rsu_free((void**)&rsu_read_req_ptr);
    return MODEM_STATUS_LEN_NO_MCC_MNC_GID;
  }

  memset(rsu_read_req_ptr, 0x00, sizeof(uim_read_transparent_req_msg_v01));
  memset(rsu_read_resp_ptr, 0x00, sizeof(uim_read_transparent_resp_msg_v01));

  if(app_type == UIM_APP_TYPE_SIM_V01)
  {
    memcpy(file_id.path,
           sim_path,
           sizeof(sim_path));
    file_id.path_len = sizeof(sim_path);
  }
  else if(app_type == UIM_APP_TYPE_USIM_V01)
  {
    memcpy(file_id.path,
           usim_path,
           sizeof(usim_path));
    file_id.path_len = sizeof(usim_path);
  }

  /* Read EF-AD to determine the MNC length. */
  rsu_read_req_ptr->session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  file_id.file_id = 0x6FAD;
  rsu_read_req_ptr->file_id = file_id;
  rsu_read_req_ptr->read_transparent.length = 0;
  rsu_read_req_ptr->read_transparent.offset = 0;

  qmi_status = qmi_client_send_msg_sync(client_handle,
                                        QMI_UIM_READ_TRANSPARENT_REQ_V01,
                                        rsu_read_req_ptr,
                                        sizeof(uim_read_transparent_req_msg_v01),
                                        rsu_read_resp_ptr,
                                        sizeof(uim_read_transparent_resp_msg_v01),
                                        RSU_QMI_MAX_TIMEOUT);
  if(qmi_status != QMI_NO_ERR ||
     rsu_read_resp_ptr->resp.result != QMI_RESULT_SUCCESS_V01)
  {
    rsu_free((void**)&rsu_read_req_ptr);
    rsu_free((void**)&rsu_read_resp_ptr);
    return MODEM_STATUS_LEN_NO_MCC_MNC_GID;
  }

  if(!rsu_read_resp_ptr->read_result_valid ||
     rsu_read_resp_ptr->read_result.content_len < 4 ||
     rsu_read_resp_ptr->read_result.content[3] != 3)
  {
    mnc_len = 2;
  }
  else
  {
    mnc_len = 3;
  }

  /* Read EF-IMSI to determine the MCC and MNC. */
  rsu_read_req_ptr->session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  file_id.file_id = 0x6F07;
  rsu_read_req_ptr->file_id = file_id;
  rsu_read_req_ptr->read_transparent.length = 0;
  rsu_read_req_ptr->read_transparent.offset = 0;

  qmi_status = qmi_client_send_msg_sync(client_handle,
                                        QMI_UIM_READ_TRANSPARENT_REQ_V01,
                                        rsu_read_req_ptr,
                                        sizeof(uim_read_transparent_req_msg_v01),
                                        rsu_read_resp_ptr,
                                        sizeof(uim_read_transparent_resp_msg_v01),
                                        RSU_QMI_MAX_TIMEOUT);
  if(qmi_status != QMI_NO_ERR ||
     (rsu_read_resp_ptr->resp.result != QMI_RESULT_SUCCESS_V01 &&
      rsu_read_resp_ptr->resp.error != QMI_ERR_SIM_FILE_NOT_FOUND_V01) ||
     !rsu_read_resp_ptr->read_result_valid ||
     rsu_read_resp_ptr->read_result.content_len < RSU_EF_IMSI_LEN)
  {
    rsu_free((void**)&rsu_read_req_ptr);
    rsu_free((void**)&rsu_read_resp_ptr);
    return MODEM_STATUS_LEN_NO_MCC_MNC_GID;
  }

  if(rsu_read_resp_ptr->resp.result != QMI_RESULT_SUCCESS_V01 &&
     rsu_read_resp_ptr->resp.error == QMI_ERR_SIM_FILE_NOT_FOUND_V01)
  {
    rsu_free((void**)&rsu_read_req_ptr);
    rsu_free((void**)&rsu_read_resp_ptr);
    return MODEM_STATUS_LEN_NO_MCC_MNC_GID;
  }

  memcpy(&buffer_ptr[RSU_NUM_MCC_MNC_SET_OFFSET], &num_imsi, sizeof(num_imsi));

  /* Populate the MCC/MNC values in big endian format. */
  buffer_ptr[RSU_MCC_MNC_OFFSET]   = 0xF0 | ((rsu_read_resp_ptr->read_result.content[1] & 0xF0) >> 4);
  buffer_ptr[RSU_MCC_MNC_OFFSET+1] = ((rsu_read_resp_ptr->read_result.content[2] & 0x0F) << 4) |
                                     ((rsu_read_resp_ptr->read_result.content[2] & 0xF0) >> 4);

  if(mnc_len == 2)
  {
    buffer_ptr[RSU_MCC_MNC_OFFSET+2] = 0xFF;
    buffer_ptr[RSU_MCC_MNC_OFFSET+3] = ((rsu_read_resp_ptr->read_result.content[3] << 4) & 0xF0) |
                                       ((rsu_read_resp_ptr->read_result.content[3] >> 4) & 0x0F);
  }
  else if(mnc_len == 3)
  {
    buffer_ptr[RSU_MCC_MNC_OFFSET+2] = 0xF0 |
                                       (rsu_read_resp_ptr->read_result.content[3] & 0x0F);
    buffer_ptr[RSU_MCC_MNC_OFFSET+3] = (rsu_read_resp_ptr->read_result.content[3] & 0xF0) |
                                       (rsu_read_resp_ptr->read_result.content[4] & 0x0F);
  }

  memset(rsu_read_req_ptr, 0x00, sizeof(uim_read_transparent_req_msg_v01));
  memset(rsu_read_resp_ptr, 0x00, sizeof(uim_read_transparent_resp_msg_v01));

  /* Read EF-GID1 */
  rsu_read_req_ptr->session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  file_id.file_id = 0x6F3E;
  rsu_read_req_ptr->file_id = file_id;
  rsu_read_req_ptr->read_transparent.length = 0;
  rsu_read_req_ptr->read_transparent.offset = 0;

  /* Initalize GID buffer to invalid (0xFFFF) in case GID read fails. */
  memset(&buffer_ptr[RSU_GID1_OFFSET], 0xFF, sizeof(uint16_t));

  qmi_status = qmi_client_send_msg_sync(client_handle,
                                        QMI_UIM_READ_TRANSPARENT_REQ_V01,
                                        rsu_read_req_ptr,
                                        sizeof(uim_read_transparent_req_msg_v01),
                                        rsu_read_resp_ptr,
                                        sizeof(uim_read_transparent_resp_msg_v01),
                                        RSU_QMI_MAX_TIMEOUT);
  if(qmi_status == QMI_NO_ERR &&
     rsu_read_resp_ptr->resp.result == QMI_RESULT_SUCCESS_V01 &&
     rsu_read_resp_ptr->read_result_valid)
  {
    /* Input GID data in big endian format. */
    if(rsu_read_resp_ptr->read_result.content_len == 1)
    {
      buffer_ptr[RSU_GID1_OFFSET] = rsu_read_resp_ptr->read_result.content[0];
    }
    else if(rsu_read_resp_ptr->read_result.content_len > 1)
    {
      buffer_ptr[RSU_GID1_OFFSET]   = rsu_read_resp_ptr->read_result.content[0];
      buffer_ptr[RSU_GID1_OFFSET+1] = rsu_read_resp_ptr->read_result.content[1];
    }
  }

  rsu_free((void**)&rsu_read_req_ptr);
  rsu_free((void**)&rsu_read_resp_ptr);

  return MODEM_STATUS_LEN;
} /* rsu_get_modem_status */


extern int32_t ModemWrapper_Send_request (
  uint32_t            request_type,
  uint8_t           * buffer_ptr,
  uint32_t            buffer_len,
  uint32_t            payload_len
)
{
  int32_t  rsu_status = MODEM_STATUS_OK;
  boolean  disconnect = FALSE;

  if(client_handle == NULL)
  {
    /* In the case where the daemon has not connected to the modem, attempt to
       connect to the modem before running the command. Also assume the daemon
       does not disconnect from the modem so disconnect from the modem after
       the command. */
    if(Connect_To_Modem() != MODEM_STATUS_OK)
    {
      return MODEM_STATUS_CONNECTION_FAILED;
    }

    disconnect = TRUE;
  }

  switch (request_type)
  {
    case MODEM_REQUEST_GET_SHARED_KEY:
    case MODEM_REQUEST_UPDATE_SIMLOCK_SETTINGS:
      rsu_status = rsu_remote_unlock_cmd(request_type, buffer_ptr, buffer_len, payload_len);
      break;
    case MODEM_REQUEST_GET_SIMLOCK_VERSION:
      rsu_status = rsu_get_simlock_version(buffer_ptr, buffer_len);
      break;
    case MODEM_REQUEST_RESET_SIMLOCK_SETTINGS:
      rsu_status = rsu_reset_simlock_settings();
      break;
    case MODEM_REQUEST_GET_MODEM_STATUS:
      rsu_status = rsu_get_modem_status(buffer_ptr, buffer_len);
      break;
    default:
      rsu_status = MODEM_STATUS_UNSUPPORTED_COMMAND;
      break;
  }

  if(disconnect)
  {
    (void)Disconnect_From_Modem();
  }

  return rsu_status;
} /* ModemWrapper_Send_request */


extern int32_t Connect_To_Modem(
  void
)
{
  qmi_client_error_type       qmi_status = QMI_NO_ERR;
  qmi_client_os_params        os_params;
  boolean                     ret_val     = 0;
  qmi_idl_service_object_type service_object = uim_get_service_object_v01();

  if(client_handle != NULL)
  {
    return MODEM_STATUS_OK;
  }

  /* Initialize Diag for QCRIL logging */
  ret_val = Diag_LSM_Init(NULL);
  if (!ret_val)
  {
    RSU_LOG_ERROR("Fail to initialize Diag");
  }

  qmi_status = qmi_client_init_instance(service_object,
                                        QMI_CLIENT_INSTANCE_ANY,
                                        NULL,
                                        NULL,
                                        &os_params,
                                        RSU_QMI_CLIENT_SRV_REG_TIMEOUT,
                                        &client_handle);
  if (qmi_status != QMI_NO_ERR)
  {
    RSU_LOG_ERROR("RSU connect failure, qmi error %d", qmi_status);
    return MODEM_STATUS_CONNECTION_FAILED;
  }

  return MODEM_STATUS_OK;
} /* Connect_To_Modem */


extern int32_t Disconnect_From_Modem(
  void
)
{
  qmi_client_error_type      qmi_status = QMI_NO_ERR;

  if(client_handle == NULL)
  {
    return MODEM_STATUS_OK;
  }

  qmi_status = qmi_client_release(client_handle);
  if (qmi_status != QMI_NO_ERR)
  {
    RSU_LOG_ERROR("RSU disconnect failure, qmi error %d", qmi_status);
    return MODEM_STATUS_CONNECTION_FAILED;
  }

  client_handle = NULL;

  return MODEM_STATUS_OK;
} /* Disconnect_From_Modem */

