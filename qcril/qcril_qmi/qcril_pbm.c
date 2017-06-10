/******************************************************************************
  @file    qcril_pbm.c
  @brief   qcril qmi - PB

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2009-2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <string.h>
#include <cutils/properties.h>
#include "qcril_log.h"
#include "qcril_pbm.h"
#include "qcril_qmi_nas.h"
#include "qcril_other.h"

#include "phonebook_manager_service_v01.h"
#include "qcril_qmi_client.h"
#include "qmi_ril_platform_dep.h"

/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/
struct pbm_cache_info_type {
    // store emergency numbers reported by PBM IND
    char pbm_ind_num[PROPERTY_VALUE_MAX];
    int pbm_num_index;
    int pbm_ind_num_valid;

    // for protecting
    pthread_mutex_t     pbm_info_mutex;
    pthread_mutexattr_t pbm_info_mutex_attr;
};
static struct pbm_cache_info_type pbm_cache_info;

#define PBM_CACHE_LOCK()    do {\
        QCRIL_LOG_INFO("LOCK PBM_CACHE_LOCK");\
        pthread_mutex_lock(&pbm_cache_info.pbm_info_mutex);\
    } while (0)

#define PBM_CACHE_UNLOCK()  do {\
        pthread_mutex_unlock(&pbm_cache_info.pbm_info_mutex);\
        QCRIL_LOG_INFO("UNLOCK PBM_CACHE_LOCK");\
    } while (0)

/*===========================================================================

                                FUNCTIONS

===========================================================================*/
void get_ecc_property_name(char *ecc_prop_name);

void qcril_qmi_pbm_init
(
  void
)
{
  memset(&pbm_cache_info, 0, sizeof(pbm_cache_info));
  pthread_mutexattr_init(&pbm_cache_info.pbm_info_mutex_attr);
  pthread_mutex_init(&pbm_cache_info.pbm_info_mutex, &pbm_cache_info.pbm_info_mutex_attr);
  qcril_qmi_pbm_enable_emergency_number_indications(TRUE);
  qcril_qmi_nas_start_wait_for_pbm_ind_timer();
};

RIL_Errno qcril_qmi_pbm_enable_emergency_number_indications(int enable)
{

  RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;
  qmi_client_error_type qmi_transport_error;

  pbm_indication_register_req_msg_v01  indication_req;
  pbm_indication_register_resp_msg_v01 indication_resp;

  QCRIL_LOG_INFO("entered %d",enable);

  memset(&indication_req, 0, sizeof(indication_req));
  memset(&indication_resp, 0, sizeof(indication_resp));

  if( TRUE == enable )
  {/* Register for Emergency list indications */
    indication_req.reg_mask = 0x00000004;
  }
  else
  {/* Suppress Emergency list indications */
    indication_req.reg_mask = 0x00000000;
  }

  qmi_transport_error = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle ( QCRIL_QMI_CLIENT_PBM ),
                                                   QMI_PBM_INDICATION_REGISTER_REQ_V01,
                                                   &indication_req,
                                                   sizeof(indication_req),
                                                   &indication_resp,
                                                   sizeof(indication_resp),
                                                   QCRIL_QMI_SYNC_REQ_DEF_TIMEOUT );

  ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_transport_error, &indication_resp.resp );

  if ( RIL_E_SUCCESS == ril_req_res )
  {
    QCRIL_LOG_INFO("Indication register successful with mask %d", indication_resp.reg_mask);
  }
  else
  {
    QCRIL_LOG_INFO("Indication register failed, error %d", ril_req_res);
  }

  return ril_req_res;
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pbm_emergency_list_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PBM_EMERGENCY_LIST_IND.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pbm_emergency_list_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  unsigned int i, k;
  int j, len = 0, cnt = 0 ;
  char prop_val[ PROPERTY_VALUE_MAX ];
  char ecc_prop_name[ PROPERTY_KEY_MAX ];
  boolean is_prop_val_full = FALSE;
  pbm_emergency_list_ind_msg_v01 *emerg_ind;
  char temp_emer_num[QMI_PBM_EMER_NUM_MAX_LENGTH_V01 + 1];

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED( ind_data_len );

  if (  ind_data_ptr != NULL )
  {
    // multi-ril support, determine which ril.ecclist property to manipulate
    get_ecc_property_name(ecc_prop_name);
    property_get( ecc_prop_name, prop_val, "" );
    QCRIL_LOG_INFO( "current value of  %s = %s", ecc_prop_name, prop_val );

    emerg_ind = (pbm_emergency_list_ind_msg_v01*)ind_data_ptr;

    /* initialize the property value buffer */
    memset( prop_val, '\0', PROPERTY_VALUE_MAX );

    /* OTA emergency numbers */
    if(emerg_ind->network_emer_nums_valid)
    {
      for (k=0; k<emerg_ind->network_emer_nums_len && !is_prop_val_full; k++)
      {
        for(i=0; i < emerg_ind->network_emer_nums[k].emer_nums_len && !is_prop_val_full; i++)
        {
          len = emerg_ind->network_emer_nums[k].emer_nums[i].emer_num_len;
          if ( ( cnt + 1 + len ) <= ( PROPERTY_VALUE_MAX - 1 ) )
          {
            memset(temp_emer_num, 0, sizeof(temp_emer_num));
            memcpy(temp_emer_num, emerg_ind->network_emer_nums[k].emer_nums[i].emer_num, len);
            if( qcril_other_is_number_found( temp_emer_num, prop_val ) )
            {
              QCRIL_LOG_INFO("network emergency number %s already present in the accumulated list %s", temp_emer_num, prop_val);
              continue;
            }

            if ( prop_val[ 0 ] != '\0' )
            {
              /* If this is not the first emergency number, add a comma as a delimiter */
              prop_val[ cnt++ ] = ',';
            }

            for ( j = 0; j < len; j++ )
            {
              prop_val[ cnt++ ] = emerg_ind->network_emer_nums[k].emer_nums[i].emer_num[j];
            }
          }
          else
          {
            /* We cannot fit in any more emergency numbers */
            is_prop_val_full = TRUE;
            QCRIL_LOG_ERROR("emergancy number list exceeds max property legnth, dropping numbers");
          }
        }
      }
      QCRIL_LOG_INFO("Updating emergency number list from network, new list: \"%s\"", prop_val);
    }

    /* Emergency numbers on Card  */
    if ( !is_prop_val_full && emerg_ind->card_emer_nums_valid )
    {
      for (k=0; k<emerg_ind->card_emer_nums_len && !is_prop_val_full; k++)
      {
        for ( i = 0; i < emerg_ind->card_emer_nums[k].emer_nums_len && !is_prop_val_full; i++ )
        {
          len = emerg_ind->card_emer_nums[k].emer_nums[i].emer_num_len;
          if ( ( cnt + 1 + len ) <= ( PROPERTY_VALUE_MAX - 1 ) )
          {
            memset(temp_emer_num, 0, sizeof(temp_emer_num));
            memcpy(temp_emer_num, emerg_ind->card_emer_nums[k].emer_nums[i].emer_num, len);
            if( qcril_other_is_number_found( temp_emer_num, prop_val ) )
            {
              QCRIL_LOG_INFO("card emergency number %s already present in the accumulated list %s", temp_emer_num, prop_val);
              continue;
            }

            if ( prop_val[ 0 ] != '\0' )
            {
              prop_val[ cnt++ ] = ',';
            }

            for ( j = 0; j < len; j++ )
            {
              prop_val[ cnt++ ] = emerg_ind->card_emer_nums[k].emer_nums[i].emer_num[j];
            }
          }
          else
          {
            is_prop_val_full = TRUE;
            QCRIL_LOG_ERROR("emergancy number list exceeds max property legnth, dropping numbers");
          }
        }
      }
      QCRIL_LOG_INFO("Updating emergency number list from sim card, new list: \"%s\"", prop_val);
    }

    /* Hardcoded emergency numbers */
    if ( !is_prop_val_full )
    {
      for(i = 0; i < emerg_ind->emer_nums_len && !is_prop_val_full; i++)
      {
        len = emerg_ind->emer_nums[i].emer_num_len;
        if ( ( cnt + 1 + len ) <= ( PROPERTY_VALUE_MAX - 1 ) )
        {
          memset(temp_emer_num, 0, sizeof(temp_emer_num));
          memcpy(temp_emer_num, emerg_ind->emer_nums[i].emer_num, len);
          if( qcril_other_is_number_found( temp_emer_num, prop_val ) )
          {
            QCRIL_LOG_INFO("hardcoded emergency number %s already present in the accumulated list %s", temp_emer_num, prop_val);
            continue;
          }

          if ( prop_val[ 0 ] != '\0' )
          {
            prop_val[ cnt++ ] = ',';
          }
          for ( j = 0; j < len; j++ )
          {
            prop_val[ cnt++ ] = emerg_ind->emer_nums[i].emer_num[j];
          }
        }
        else
        {
          is_prop_val_full = TRUE;
          QCRIL_LOG_ERROR("emergancy number list exceeds max property legnth, dropping numbers");
        }
      }
      QCRIL_LOG_INFO("Updating emergency number from hardcoded value, new list: \"%s\"", prop_val);
    }

    /* Extended hardcoded emergency numbers */
    if ( !is_prop_val_full && emerg_ind->emer_nums_extended_valid )
    {
      for(i = 0; i < emerg_ind->emer_nums_extended_len && !is_prop_val_full; i++)
      {
        len = emerg_ind->emer_nums_extended[i].emer_num_len;
        if ( ( cnt + 1 + len ) <= ( PROPERTY_VALUE_MAX - 1 ) )
        {
          memset(temp_emer_num, 0, sizeof(temp_emer_num));
          memcpy(temp_emer_num, emerg_ind->emer_nums_extended[i].emer_num, len);
          if( qcril_other_is_number_found( temp_emer_num, prop_val ) )
          {
            QCRIL_LOG_INFO("hardcoded emergency number %s already present in the accumulated list %s", temp_emer_num, prop_val);
            continue;
          }

          if ( prop_val[ 0 ] != '\0' )
          {
            prop_val[ cnt++ ] = ',';
          }
          for ( j = 0; j < len; j++ )
          {
            prop_val[ cnt++ ] = emerg_ind->emer_nums_extended[i].emer_num[j];
          }
        }
        else
        {
          is_prop_val_full = TRUE;
          QCRIL_LOG_ERROR("emergancy number list exceeds max property legnth, dropping numbers");
        }
      }
      QCRIL_LOG_INFO("Updating emergency number from extended hardcoded value, new list: \"%s\"", prop_val);
    }

    /* NV emergency numbers */
    if ( !is_prop_val_full && emerg_ind->nv_emer_nums_valid )
    {
      for(i = 0; i < emerg_ind->nv_emer_nums_len && !is_prop_val_full; i++)
      {
        len = emerg_ind->nv_emer_nums[i].emer_num_len;
        if ( ( cnt + 1 + len ) <= ( PROPERTY_VALUE_MAX - 1 ) )
        {
          memset(temp_emer_num, 0, sizeof(temp_emer_num));
          memcpy(temp_emer_num, emerg_ind->nv_emer_nums[i].emer_num, len);
          if( qcril_other_is_number_found( temp_emer_num, prop_val ) )
          {
            QCRIL_LOG_INFO("NV emergency number %s already present in the accumulated list %s", temp_emer_num, prop_val);
            continue;
          }

          if ( prop_val[ 0 ] != '\0' )
          {
            prop_val[ cnt++ ] = ',';
          }
          for ( j = 0; j < len; j++ )
          {
            prop_val[ cnt++ ] = emerg_ind->nv_emer_nums[i].emer_num[j];
          }
        }
        else
        {
          is_prop_val_full = TRUE;
          QCRIL_LOG_ERROR("emergancy number list exceeds max property legnth, dropping numbers");
        }
      }
      QCRIL_LOG_INFO("Updating emergency number list from NV, new list: \"%s\"", prop_val);
    }

    /* Store the numbers into PBM cache, and do no set prop at this time for two reasons:
     *   - If custom_ecc is enabled, set the prop later together to avoid ecc prop has incompleted
     * numbers in a short time window.
     *   - Later when card_status changed or voice_rte_confidence changed, RIL will combine the numbers
     * matched in database and that in PBM cache into ecc.list property, other than appending news numbers
     * into existed ecc.list prop, which is not right.
     */
    PBM_CACHE_LOCK();
    pbm_cache_info.pbm_ind_num_valid = TRUE;
    pbm_cache_info.pbm_num_index = cnt;
    memcpy(pbm_cache_info.pbm_ind_num, prop_val, PROPERTY_VALUE_MAX);
    PBM_CACHE_UNLOCK();

    qmi_ril_nwreg_designated_number_ensure_fresh_check_ncl();
    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                   QCRIL_DEFAULT_MODEM_ID,
                   QCRIL_DATA_ON_STACK,
                   QCRIL_EVT_QMI_RIL_ASSESS_EMRGENCY_NUMBER_LIST_DESIGNATED_COUNTRY,
                   NULL,
                   QMI_RIL_ZERO,
                   (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );

  }

  QCRIL_LOG_FUNC_RETURN();

}/* qcril_qmi_pbm_emergency_list_ind_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_pbm_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Callback for QMI PBM indications

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pbm_unsol_ind_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                          *ind_buf,
  unsigned int                   ind_buf_len,
  void                          *ind_cb_data
)
{
  qmi_ind_callback_type qmi_callback;

  QCRIL_LOG_FUNC_ENTRY();

  memset(&qmi_callback,0,sizeof(qmi_callback));
  qmi_callback.data_buf = qcril_malloc(ind_buf_len);

  if( qmi_callback.data_buf )
  {
    qmi_callback.user_handle = user_handle;
    qmi_callback.msg_id = msg_id;
    memcpy(qmi_callback.data_buf,ind_buf,ind_buf_len);
    qmi_callback.data_buf_len = ind_buf_len;
    qmi_callback.cb_data = ind_cb_data;

    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                   QCRIL_DEFAULT_MODEM_ID,
                   QCRIL_DATA_ON_STACK,
                   QCRIL_EVT_QMI_PBM_HANDLE_INDICATIONS,
                   (void*) &qmi_callback,
                   sizeof(qmi_callback),
                   (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
  }
  else
  {
    QCRIL_LOG_FATAL("malloc failed");
  }

  QCRIL_LOG_FUNC_RETURN();
}/* qcril_qmi_pbm_unsol_ind_cb */


/*=========================================================================
  FUNCTION:  qcril_qmi_pbm_unsolicited_indication_cb_helper

===========================================================================*/
/*!
    @brief
    helper function for handling pbm indication

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pbm_unsolicited_indication_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    void * decoded_payload = NULL;
    qmi_client_error_type qmi_err;
    uint32_t decoded_payload_len;

    QCRIL_NOTUSED(ret_ptr);
    qmi_ind_callback_type *qmi_callback = (qmi_ind_callback_type*) params_ptr->data;

    do {

       if( !qmi_callback )
       {
          QCRIL_LOG_ERROR("qmi_callback is NULL");
          QCRIL_ASSERT(0); // this is a noop in release build
          break;
       }

       QCRIL_LOG_INFO("invoked msg 0x%x", (int) qmi_callback->msg_id);

       qmi_err = qmi_idl_get_message_c_struct_len( pbm_get_service_object_v01(),
                                                   QMI_IDL_INDICATION,
                                                   qmi_callback->msg_id,
                                                   &decoded_payload_len);
       if ( qmi_err != QMI_NO_ERR )
       {
          QCRIL_LOG_ERROR("Failed to process qmi message w/%d", qmi_err);
          QCRIL_ASSERT(0); // this is a noop in release build
          break;
       }

       if( !decoded_payload_len )
       {
          // ok, this is a null payload - don't need to process it.
       }
       else
       {
          // process the payload
          decoded_payload = qcril_malloc( decoded_payload_len );
          if ( !decoded_payload )
          {
             QCRIL_LOG_ERROR("Failed to alloc memory for decoded payload");
             QCRIL_ASSERT(0); // this is a noop in release build
             break;
          }

          memset( decoded_payload, 0, decoded_payload_len );

          qmi_err = qmi_client_message_decode(
                        qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_PBM ),
                        QMI_IDL_INDICATION,
                        qmi_callback->msg_id,
                        qmi_callback->data_buf,
                        qmi_callback->data_buf_len,
                        decoded_payload,
                        (int)decoded_payload_len );

          if ( qmi_err != QMI_NO_ERR )
          {
             QCRIL_LOG_ERROR("Failed to process qmi message w/%d", qmi_err);
             QCRIL_ASSERT(0); // this is a noop in release build
             qcril_free(decoded_payload);
             break;
          }

          switch ( qmi_callback->msg_id )
          {
            case QMI_PBM_EMERGENCY_LIST_IND_V01:
              qcril_qmi_pbm_emergency_list_ind_hdlr(decoded_payload, decoded_payload_len);
              break;

            default:
              QCRIL_LOG_INFO("Unsupported QMI PBM indication %x hex", qmi_callback->msg_id);
              break;
          }
          qcril_free( decoded_payload );
       }

    } while(0);

    if( qmi_callback && qmi_callback->data_buf )
    {
      qcril_free(qmi_callback->data_buf);
    }
    QCRIL_LOG_FUNC_RETURN();
}//qcril_qmi_pbm_unsolicited_indication_cb_helper

//===========================================================================
//qmi_ril_phone_number_is_emergency
//===========================================================================
int qmi_ril_phone_number_is_emergency(char * number_to_check)
{
  int res = FALSE;

  char ecc_prop_val[ PROPERTY_VALUE_MAX ];
  char ecc_prop_name[ PROPERTY_KEY_MAX ];
  int  prop_req_res;

  QCRIL_LOG_FUNC_ENTRY();

  get_ecc_property_name(ecc_prop_name);
  prop_req_res = property_get( ecc_prop_name, ecc_prop_val, "" );
  QCRIL_LOG_INFO(" .. prop req with %d", (int) prop_req_res);
  if ( prop_req_res  > 0 )
  {
    QCRIL_LOG_ESSENTIAL( "property %s = \"%s\"", ecc_prop_name, ecc_prop_val );
    res = qcril_other_is_number_found( number_to_check, ecc_prop_val );
  }
  else
  {
    QCRIL_LOG_INFO("propperty_get %s returned w/%d", ecc_prop_name, (int) prop_req_res);
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( res );
  return res;
} // qmi_ril_phone_number_is_emergency

//===========================================================================
//qmi_ril_phone_number_is_non_std_otasp
//===========================================================================
int qmi_ril_phone_number_is_non_std_otasp(const char * number_to_check)
{
  return strncmp(number_to_check, NON_STD_OTASP_NUMBER,strlen(NON_STD_OTASP_NUMBER)+1) ? 0 : 1;
} // qmi_ril_phone_number_is_non_std_otasp

//===========================================================================
//get_ecc_property_name
//Returns the ecc list property name in ecc_prop_name for this instance.
//For Default instance and in non multisim targets this is ril.ecclist.
//===========================================================================
void get_ecc_property_name(char *ecc_prop_name)
{
   qcril_instance_id_e_type instance_id = qmi_ril_get_process_instance_id();

   if ( ecc_prop_name == NULL)
   {
       QCRIL_LOG_ERROR("get_ecc_property_name: Invalid argument.");
   }
   else
   {
       if ( instance_id == QCRIL_DEFAULT_INSTANCE_ID )
       {
           snprintf( ecc_prop_name, PROPERTY_KEY_MAX, "%s",QCRIL_ECC_LIST);
       }
       else
       {
           snprintf( ecc_prop_name, PROPERTY_KEY_MAX, "%s%d", QCRIL_ECC_LIST, instance_id);
       }
       QCRIL_LOG_ERROR("ecc list name : %s.", ecc_prop_name);
   }
}

/*=========================================================================
  FUNCTION:  qmi_ril_set_ecc_property

===========================================================================*/
/*!
    @brief
    Combine param "ecc_bumbers" and "numbers in PBM cache" into ECC property

    @params
    - ecc_numbers: ecc numbers delimited with comma, ended with '\0'. It could
    be *NULL*.

    @return
    0 if success. Other value if failure
*/
/*=========================================================================*/

int qmi_ril_set_ecc_property(char* ecc_numbers)
{
    char ecc_prop_name[PROPERTY_KEY_MAX];
    char ecc_prop_val[PROPERTY_VALUE_MAX];
    size_t param_ecc_len, item_len, val_index;
    int ret, no_space = 0;
    const char delim[2] = {',',0};
    char *num_parsing_holder, *saveptr, *token;

    QCRIL_LOG_FUNC_ENTRY();

    /* get the right prop name for current RIL instance */
    get_ecc_property_name(ecc_prop_name);
    memset(ecc_prop_val, 0, PROPERTY_VALUE_MAX);

    /* copy numbers from PBM IND */
    PBM_CACHE_LOCK();
    if (pbm_cache_info.pbm_ind_num_valid)
    {
        if (strlcpy(ecc_prop_val, pbm_cache_info.pbm_ind_num, PROPERTY_VALUE_MAX) >= PROPERTY_VALUE_MAX)
        {
            /* should not happen since PBM IND handler already consider this */
            QCRIL_LOG_FATAL("PBM IND numbers exceeds the property length");
            no_space = 1;
        }
    }
    PBM_CACHE_UNLOCK();

    /* append the custom ecc numbers if have space */
    if (!no_space && ecc_numbers)
    {
        param_ecc_len = strlen(ecc_numbers);
        val_index = strlen(ecc_prop_val);
        num_parsing_holder = (char*)qcril_malloc(param_ecc_len + 1);
        if (num_parsing_holder)
        {
            /* not checking return values since there could not be truncation */
            strlcpy(num_parsing_holder, ecc_numbers, param_ecc_len + 1);
            token = strtok_r(num_parsing_holder, delim, &saveptr);
            while (token)
            {
                /* if it is not a duplicated number */
                if (!qcril_other_is_number_found(token, ecc_prop_val))
                {
                    /* check if we have enough space to append this number */
                    item_len = strlen(token) + ((!val_index)?0:1);
                    if ((item_len + 1) > (PROPERTY_VALUE_MAX - val_index))
                    {
                        no_space = 1;
                        break;
                    }
                    else
                    {
                        /* Already checked boundary */
                        if (val_index)
                            strlcat(ecc_prop_val, ",", PROPERTY_VALUE_MAX);
                        strlcat(ecc_prop_val, token, PROPERTY_VALUE_MAX);
                    }
                    val_index += item_len;

                }
                token = strtok_r(NULL, delim, &saveptr);
            }
        }
    }

    if (no_space)
    {
        QCRIL_LOG_ERROR("The ecc numbers exceeds the property length, abandon some of them");
    }
    QCRIL_LOG_INFO("new saved %s: %s", ecc_prop_name, ecc_prop_val);

    /* set the property */
    if ((ret = property_set(ecc_prop_name, ecc_prop_val)) != E_SUCCESS)
    {
        QCRIL_LOG_FATAL("Failed to save %s to system property", ecc_prop_name);
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);

    return ret;
}
