/******************************************************************************
  @file    ds_profile_3gpp2_qmi.c
  @brief   

  DESCRIPTION
  Tech specific implementation of 3GPP2 1x Profile Management  

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  N/A

  ---------------------------------------------------------------------------
  Copyright (C) 2009-2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/modem/data/1x/707/main/latest/src/ds707_data_session_profile.c#32 $ $DateTime: 2009/09/11 10:21:08 $ $Author: lipings $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

#include "comdef.h"

#ifdef FEATURE_DS_PROFILE_ACCESS_QMI

#include "ds_profile_3gpp2i.h"
#include "ds_profile_os.h"
#include "ds_util.h"
#include "qmi_wds_srvc.h"
#include "qmi_wds_utils.h"

static int qmi_3gpp2_client_handle;

/*qmi message library handle*/
static int qmi_handle = QMI_INVALID_CLIENT_HANDLE;

/*Alloc dll's profile elements data*/
#define ALLOC_PROFILE_LIST_NODE_DATA(len) \
do {  profile_node->profile_element.data = (unsigned char *)malloc(len); \
  if (!profile_node->profile_element.data) \
  { \
    DS_PROFILE_LOGE("unable to malloc memory for profile node's data \n",0); \
    free(profile_node); \
    profile_node = NULL; \
    return DS_PROFILE_REG_RESULT_FAIL; \
  } \
} while(0)

#define DSI_PROFILE_CREATE_LIST_PROFILE_LIST_SIZE  15

/*lint -save -e641*/
/*lint -save -e655*/
/*===========================================================================
FUNCTION qmi_cleanup_3gpp2

DESCRIPTION
  cleanup routine registered using atexit to clean up qmi resources

PARAMETERS 
  fntbl : pointer to table of function pointers

DEPENDENCIES 
  
RETURN VALUE 
  void
SIDE EFFECTS 
  
===========================================================================*/
void 
qmi_cleanup_3gpp2
(
  void
)
{
  int qmi_err;

  if (qmi_handle < 0)
  {
    DS_PROFILE_LOGE("QMI message library was never initialized. "
                                             "invalid qmi handle. ");
    return;
  }

  if (qmi_3gpp2_client_handle >= 0)
  {
    qmi_wds_srvc_release_client( qmi_3gpp2_client_handle, &qmi_err );
    DS_PROFILE_LOGD("Releasing the WDS qmi_client_handle 0x%08x \n",
                      qmi_3gpp2_client_handle);
  }
  /* Release QMI library connection */
  qmi_release(qmi_handle);
}
/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_QMI_READ

DESCRIPTION
  This function is used to read a profile from modem to the local copy

PARAMETERS 
  num : profile number
  ptr : pointer to profile blob
  
DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS
  DS_PROFILE_REG_RESULT_FAIL
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM
  
SIDE EFFECTS 
  
===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp2_qmi_read(
  ds_profile_num_type   num,
  void                 *ptr
)
{
  qmi_wds_profile_id_type     profile_id;
  int qmi_err_code;
  int rc = DS_PROFILE_REG_RESULT_FAIL;
  /*create a linked list head node */
  qmi_wds_profile_node_list_type      profile_list;
  qmi_wds_list_link_type      *list_node_link = NULL;
  dsi_profile_3gpp2_type *dsi_3gpp2_ptr = NULL;
  qmi_wds_profile_node_type           *profile_node = NULL;


  list_init(&profile_list);

  profile_id.technology = QMI_WDS_PROFILE_TECH_3GPP2;
  profile_id.profile_index = num;
  if ((rc = qmi_wds_utils_query_profile(qmi_3gpp2_client_handle, 
                                        &profile_id, 
                                        &profile_list,
                                        &qmi_err_code)) != QMI_NO_ERR)
  {

    DS_PROFILE_LOGE("_3gpp2_qmi_read: Query modem for profile information failed,  return code = %d", rc );

    if (rc == QMI_EXTENDED_ERR)
    {
      DS_PROFILE_LOGE("_3gpp2_qmi_read: return code = %d ", rc );
      DS_PROFILE_LOGE("_3gpp2_qmi_read: extended error code = %d", qmi_err_code );
      return (ds_profile_status_etype)qmi_err_code;
    }

    return DS_PROFILE_REG_RESULT_FAIL;
  }

  /*If we reach here we have a valid list of profile elements*/

  if (list_size(&profile_list) == 0)
  {
     DS_PROFILE_LOGE("_3gpp2_qmi_read: List is empty", 0 );
     return  DS_PROFILE_REG_RESULT_SUCCESS;
  }
  else if (list_size(&profile_list) > 0)
  {
    list_node_link = list_peek_front(&profile_list);
  }

  dsi_3gpp2_ptr = (dsi_profile_3gpp2_type *)ptr;
  dsi_3gpp2_ptr->mask = 0;
  memset((void *)dsi_3gpp2_ptr->prf, 0, sizeof(ds_profile_3gpp2_profile_info_type));
  
  /*While there are list elements, process each element and prepare TLV*/
  while(list_node_link != NULL)
  {
    profile_node = (qmi_wds_profile_node_type *)list_node_link;

    switch(profile_node->profile_element.type)
    {
      case DS_PROFILE_3GPP2_PROFILE_PARAM_NEGOTIATE_DNS_SERVER:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->negotiate_dns_server) )
          {
            memcpy(&dsi_3gpp2_ptr->prf->negotiate_dns_server,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_DO:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->ppp_session_close_timer_DO) )
          {
            memcpy(&dsi_3gpp2_ptr->prf->ppp_session_close_timer_DO,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
      case   DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_1X:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->ppp_session_close_timer_1X) )
          {
            memcpy(&dsi_3gpp2_ptr->prf->ppp_session_close_timer_1X,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_ALLOW_LINGER:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->allow_linger) )
          {
            memcpy(&dsi_3gpp2_ptr->prf->allow_linger,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_ACK_TIMEOUT:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->lcp_ack_timeout) )
          {
            memcpy(&dsi_3gpp2_ptr->prf->lcp_ack_timeout,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_ACK_TIMEOUT:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->ipcp_ack_timeout) )
          {                                     
            memcpy(&dsi_3gpp2_ptr->prf->ipcp_ack_timeout,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_TIMEOUT:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->auth_timeout) )
          {
            memcpy(&dsi_3gpp2_ptr->prf->auth_timeout,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_CREQ_RETRY_COUNT:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->lcp_creq_retry_count) )
          {
            memcpy(&dsi_3gpp2_ptr->prf->lcp_creq_retry_count,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_CREQ_RETRY_COUNT:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->ipcp_creq_retry_count))
          {
            memcpy(&dsi_3gpp2_ptr->prf->ipcp_creq_retry_count,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_RETRY_COUNT:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->auth_retry_count) )
          {
            memcpy(&dsi_3gpp2_ptr->prf->auth_retry_count,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PROTOCOL:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->auth_protocol) )
          {
            memcpy(&dsi_3gpp2_ptr->prf->auth_protocol,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_USER_ID:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= DS_PROFILE_3GPP2_PPP_MAX_USER_ID_LEN )
          {
            memcpy(dsi_3gpp2_ptr->prf->user_id,
                   profile_node->profile_element.data,
                   (uint8)profile_node->profile_element.len);
            dsi_3gpp2_ptr->prf->user_id_len = (unsigned char)profile_node->profile_element.len;
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PASSWORD:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= DS_PROFILE_3GPP2_PPP_MAX_PASSWD_LEN )
          {
            memcpy(dsi_3gpp2_ptr->prf->auth_password,
                   profile_node->profile_element.data,
                   (uint8)profile_node->profile_element.len);
            dsi_3gpp2_ptr->prf->auth_password_len = (unsigned char)profile_node->profile_element.len;
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_RATE:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->data_rate) )
          {
            memcpy(&dsi_3gpp2_ptr->prf->data_rate,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
      case   DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_MODE:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->data_mode))
          {
            memcpy(&dsi_3gpp2_ptr->prf->data_mode,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_APP_TYPE:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->app_type))
          {
            memcpy(&dsi_3gpp2_ptr->prf->app_type,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_APP_PRIORITY:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->app_priority))
          {
            memcpy(&dsi_3gpp2_ptr->prf->app_priority,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_APN_STRING:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= DS_PROFILE_3GPP2_APN_MAX_VAL_LEN )
          {
            memcpy(dsi_3gpp2_ptr->prf->apn_string,
                   profile_node->profile_element.data,
                   (uint8)profile_node->profile_element.len);
            dsi_3gpp2_ptr->prf->apn_string_len = (unsigned char)profile_node->profile_element.len;
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
      case   DS_PROFILE_3GPP2_PROFILE_PARAM_PDN_TYPE:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->pdn_type) )
          {
            memcpy(&dsi_3gpp2_ptr->prf->pdn_type,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
      case   DS_PROFILE_3GPP2_PROFILE_PARAM_IS_PCSCF_ADDR_NEEDED:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(dsi_3gpp2_ptr->prf->is_pcscf_addr_needed) )
          {
            memcpy(&dsi_3gpp2_ptr->prf->is_pcscf_addr_needed,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_PRIMARY:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(uint32))
          {
            memcpy(&dsi_3gpp2_ptr->prf->v4_dns_addr[0].ds_profile_3gpp2_s_addr,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_SECONDARY:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(uint32))
          {
            memcpy(&dsi_3gpp2_ptr->prf->v4_dns_addr[1].ds_profile_3gpp2_s_addr,
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_PRIMARY:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(ds_profile_3gpp2_in6_addr_type))
          {
            memcpy(&dsi_3gpp2_ptr->prf->v6_dns_addr[0],
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
        case   DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_SECONDARY:
        {
          dsi_3gpp2_ptr->prf->profile_type |=  get_valid_mask_from_ident(profile_node->profile_element.type);
          if( profile_node->profile_element.len <= sizeof(ds_profile_3gpp2_in6_addr_type))
          {
            memcpy(&dsi_3gpp2_ptr->prf->v6_dns_addr[1],
                   profile_node->profile_element.data,
                   profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp2_qmi_read: Length of type %x is invalid, "
                            "cannot be copied to struct", profile_node->profile_element.type );
          }
        }
        break;
      default:
        {
          DS_PROFILE_LOGE("_3gpp2_qmi_read: Invalid profile param received %x",profile_node->profile_element.type );
        }
        break;
    }/*switch*/
    list_node_link =  (qmi_wds_list_link_type *) list_peek_next(&profile_list, list_node_link);
  }/*While*/

   /*cleanup list here and return success*/
   qmi_wds_utils_cleanup_list(&profile_list);
   return DS_PROFILE_REG_RESULT_SUCCESS;
}


/*===========================================================================
FUNCTION DS_PROFILE_3GPP2_CREATE_LIST_NODE

DESCRIPTION
  This function is used to write the profile blob back to modem.

PARAMETERS 
  num : profile identifier
  ptr : pointer to profile node list element
  
DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS
  DS_PROFILE_REG_RESULT_FAIL
  
SIDE EFFECTS 
  
===========================================================================*/
static ds_profile_status_etype ds_profile_3gpp2_create_list_node(
  ds_profile_3gpp2_profile_info_type           *profile_ptr,
  uint32                                       ident,
  qmi_wds_profile_node_type                    **node
)
{
  qmi_wds_profile_node_type     *profile_node;
  short param_len;

  profile_node = (qmi_wds_profile_node_type *)malloc (sizeof(qmi_wds_profile_node_type));

  if (!profile_node)
  {
    DS_PROFILE_LOGE("_3gpp2_create_list:unable to malloc memory for profile node \n",0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }
  memset (profile_node,0,sizeof(qmi_wds_profile_node_type));

  switch(ident)
  {
    case DS_PROFILE_3GPP2_PROFILE_PARAM_NEGOTIATE_DNS_SERVER:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_NEGOTIATE_DNS_SERVER;
        param_len = sizeof(unsigned char);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->negotiate_dns_server, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_DO:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_DO;
        param_len = sizeof(unsigned long);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->ppp_session_close_timer_DO, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_1X:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_1X;
        param_len = sizeof(unsigned long);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->ppp_session_close_timer_1X, param_len);
      }
      break;  

    case DS_PROFILE_3GPP2_PROFILE_PARAM_ALLOW_LINGER:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_ALLOW_LINGER;
        param_len = sizeof(unsigned char);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->allow_linger, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_ACK_TIMEOUT:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_ACK_TIMEOUT;
        param_len = sizeof(unsigned short);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->lcp_ack_timeout, param_len);
      }
      break;  

    case DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_ACK_TIMEOUT:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_ACK_TIMEOUT;
        param_len = sizeof(unsigned short);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->ipcp_ack_timeout, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_TIMEOUT:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_TIMEOUT;
        param_len = sizeof(unsigned short);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->auth_timeout, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_CREQ_RETRY_COUNT:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_CREQ_RETRY_COUNT;
        param_len = sizeof(unsigned char);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->lcp_creq_retry_count, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_RETRY_COUNT:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_RETRY_COUNT;
        param_len = sizeof(unsigned char);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->auth_retry_count, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_CREQ_RETRY_COUNT:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_CREQ_RETRY_COUNT;
        param_len = sizeof(unsigned char);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->ipcp_creq_retry_count, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PROTOCOL:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PROTOCOL;
        param_len = sizeof(unsigned char);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->auth_protocol, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_USER_ID:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_USER_ID;
        param_len = strlen(profile_ptr->user_id);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,profile_ptr->user_id, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PASSWORD:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PASSWORD;
        param_len = strlen(profile_ptr->auth_password);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,profile_ptr->auth_password, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_RATE:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_RATE;
        param_len = sizeof(unsigned char);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->data_rate, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_MODE:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_MODE;
        param_len = sizeof(unsigned char);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->data_mode, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_APP_TYPE:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_APP_TYPE;
        param_len = sizeof(unsigned long);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->app_type, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_APP_PRIORITY:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_APP_PRIORITY;
        param_len = sizeof(unsigned char);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->app_priority, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_APN_STRING:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_APN_STRING;
        param_len = strlen(profile_ptr->apn_string);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,profile_ptr->apn_string, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_PDN_TYPE:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_PDN_TYPE;
        param_len = sizeof(unsigned char);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->pdn_type, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_IS_PCSCF_ADDR_NEEDED:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_IS_PCSCF_ADDR_NEEDED;
        param_len = sizeof(unsigned char);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->is_pcscf_addr_needed, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_PRIMARY:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_PRIMARY;
        param_len = sizeof(ipv4_addr_type);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->v4_dns_addr[0].ds_profile_3gpp2_s_addr, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_SECONDARY:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_SECONDARY;
        param_len = sizeof(ipv4_addr_type);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->v4_dns_addr[1].ds_profile_3gpp2_s_addr, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_PRIMARY:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_PRIMARY;
        param_len = sizeof(ipv6_addr_type);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->v6_dns_addr[0].ds_profile_3gpp2_in6_u, param_len);
      }
      break;

    case DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_SECONDARY:
      {
        profile_node->profile_element.type = DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_SECONDARY;
        param_len = sizeof(ipv6_addr_type);
        profile_node->profile_element.len = param_len;
        ALLOC_PROFILE_LIST_NODE_DATA(param_len);
        memcpy(profile_node->profile_element.data,&profile_ptr->v6_dns_addr[1].ds_profile_3gpp2_in6_u, param_len);
      }
      break;

    default:
      {
        DS_PROFILE_LOGE("_3gpp2_create_list: Identifier type not recognized \n",0);
        if( profile_node->profile_element.data )
        { 
          free(profile_node->profile_element.data);
        }
        free(profile_node);
        profile_node = NULL;
        return DS_PROFILE_REG_RESULT_FAIL;
      }
  }/*switch*/

  *node = profile_node;
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_QMI_WRITE

DESCRIPTION
  This function is used to write the profile blob back to modem.

PARAMETERS 
  num : profile number
  ptr : pointer to profile blob
  
DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS
  DS_PROFILE_REG_RESULT_FAIL
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM
  
SIDE EFFECTS 
  
===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp2_qmi_write(
  ds_profile_num_type   num,
  void                 *ptr
)
{
  dsi_profile_3gpp2_type *dsi_3gpp2_ptr = NULL;
  uint32 ident = 0;
  uint32 mask = 0;
  uint8 index = 0;

  /*create a linked list head node */
  qmi_wds_profile_node_list_type      profile_list;
  int                                 rc = DS_PROFILE_REG_RESULT_FAIL;
  int                                 qmi_err_code;
  qmi_wds_profile_node_type           *profile_node = NULL;
  qmi_wds_profile_id_type             profile_id;

  list_init(&profile_list);

  DS_PROFILE_LOGD("_3gpp2_qmi_write: ENTRY", 0);

  dsi_3gpp2_ptr = (dsi_profile_3gpp2_type *)ptr;

  if ( dsi_3gpp2_ptr->mask == 0 )
  {
    DS_PROFILE_LOGD("_3gpp2_qmi_write: EXIT with SUCCESS", 0);
    return DS_PROFILE_REG_RESULT_SUCCESS;  
  }

  for ( ident = DS_PROFILE_3GPP2_PROFILE_PARAM_MIN;
       ident <= DS_PROFILE_3GPP2_PROFILE_PARAM_MAX;
       ident++ )
  {
    index = dsi_profile_3gpp2_get_index_from_ident( ident );
    CONVERT_IDENT_TO_MASK( mask, index );

    if (dsi_3gpp2_ptr->mask & mask)
    {
      if ( ( rc = ds_profile_3gpp2_create_list_node(dsi_3gpp2_ptr->prf,
                                                    ident, 
                                                    &profile_node))!= DS_PROFILE_REG_RESULT_SUCCESS )
      {
        DS_PROFILE_LOGE("_3gpp2_qmi_write: write FAIL lib in inconsistent state", 0);   
        goto err_label;
      }
      /*If we reach here, a profile node has been successfully created,
        Insert the node into the list Now*/
      list_push_back(&profile_list,(qmi_wds_list_link_type *)profile_node);
      profile_node = NULL;
    } 
  }

  /*write the list here using QMI*/
  profile_id.technology = QMI_WDS_PROFILE_TECH_3GPP2;
  profile_id.profile_index = num;

  if ((rc = qmi_wds_utils_modify_profile(qmi_3gpp2_client_handle, 
                                                    &profile_id,
                                                    &profile_list,
                                                    &qmi_err_code)) != QMI_NO_ERR)
  {
    DS_PROFILE_LOGE("_3gpp2_qmi_write: Cannot modify profile ", 0);
    if (rc == QMI_EXTENDED_ERR)
    {
      DS_PROFILE_LOGE("_3gpp2_qmi_write: return code = %d ", rc);
      DS_PROFILE_LOGE("_3gpp2_qmi_write:  extended error code = %d", qmi_err_code );
      return (ds_profile_status_etype)qmi_err_code;
    }
    else
    {
      goto err_label;
    }
  }    

  /*clean up list here*/
  qmi_wds_utils_cleanup_list(&profile_list);
  DS_PROFILE_LOGD("_3gpp2_qmi_write: EXIT with SUCCESS", 0); 
  return DS_PROFILE_REG_RESULT_SUCCESS;

err_label:
  DS_PROFILE_LOGE("_3gpp2_qmi_write: Cleaning up the partial linked list", 0);
  qmi_wds_utils_cleanup_list(&profile_list);
  DS_PROFILE_LOGE("_3gpp2_qmi_write: EXIT with ERR", 0); 
  return DS_PROFILE_REG_RESULT_FAIL;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_GET_LIST

DESCRIPTION
  This function is used to form a list of profile numbers depending on input
  value. (All profile numbers or search according to <key, value> pair)

PARAMETERS 
  hndl : list handle
  lst : pointer to return list
  
DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS
  DS_PROFILE_REG_RESULT_FAIL
  DS_PROFILE_REG_RESULT_LIST_END : empty list is being returned
  
SIDE EFFECTS 
  
===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp2_get_list(
  ds_util_list_hndl_type hndl,
  ds_profile_list_type  *lst
)
{
  uint32 info_size = 0;
  boolean list_is_empty = TRUE;
  ds_profile_3gpp2_list_info_type node;
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  
  int rc, qmi_err_code;
  qmi_wds_profile_list_type profile_list [DSI_PROFILE_CREATE_LIST_PROFILE_LIST_SIZE];
  int profile_list_size = DSI_PROFILE_CREATE_LIST_PROFILE_LIST_SIZE;


  switch (lst->dfn)
  {
    /* List all profiles */
    case DS_PROFILE_LIST_ALL_PROFILES:
      return_status = DS_PROFILE_REG_RESULT_SUCCESS;

      if ((rc = qmi_wds_utils_get_profile_list (qmi_3gpp2_client_handle,
                                          NULL, 
                                          NULL, 
                                          profile_list,
                                          &profile_list_size,
                                          &qmi_err_code)) != QMI_NO_ERR)
      {
        DS_PROFILE_LOGE("_3gpp2_get_list: Unable to query for profile list", 0);
        if (rc == QMI_EXTENDED_ERR)
        {
          return (ds_profile_status_etype)qmi_err_code;
        }
        else
        {
          return (ds_profile_status_etype)DSI_FAILURE;
        }
      }
      else
      {                                           
        int i;
        for (i=0; i < profile_list_size; i++)
        {
            node.num = (ds_profile_num_type) profile_list[i].profile_index;
            node.name[0] = '\0';
            info_size = sizeof(ds_profile_3gpp2_list_info_type);
            if ( ds_util_list_add( hndl, &node, info_size) != DS_SUCCESS)
            {
              DS_PROFILE_LOGE("_3gpp2_get_list: unable to add node to list", 0);
              return_status = DS_PROFILE_REG_RESULT_FAIL;
              break;
            }
            list_is_empty = FALSE;
        }
      }
        
      break;

    /* List profiles based on search condition */
    case DS_PROFILE_LIST_SEARCH_PROFILES:
    {
      qmi_wds_profile_tech_type       profile_tech;
      qmi_wds_profile_node_list_type  profile_search_list;
      qmi_wds_profile_node_type       profile_node;

      memset(&profile_node,0,sizeof(qmi_wds_profile_node_type));
      return_status = DS_PROFILE_REG_RESULT_SUCCESS;

      if ( !DS_PROFILE_3GPP2_IDENT_IS_VALID( lst->ident ) )
      {
        return DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT;
      }
      
      profile_tech = QMI_WDS_PROFILE_TECH_3GPP2;
      list_init(&profile_search_list);
      // valid_mask = get_valid_mask_from_ident( lst->ident );
      profile_node.profile_element.type = (unsigned char)lst->ident;
      profile_node.profile_element.len  = lst->info.len;
      profile_node.profile_element.data = (void *) malloc(lst->info.len);
      if(profile_node.profile_element.data == NULL)
      {
        //ERROR malloc returned null
        DS_PROFILE_LOGE("_3gpp2_get_list: Unable to malloc memory for "
                        "profile_node.profile_element.data",0);
        return DS_PROFILE_REG_RESULT_FAIL; 
      }
      memcpy(profile_node.profile_element.data, lst->info.buf, lst->info.len);
      list_push_back(&profile_search_list,(qmi_wds_list_link_type *)&profile_node);

      if ((rc = qmi_wds_utils_get_profile_list (qmi_3gpp2_client_handle,
                                                &profile_tech, 
                                                &profile_search_list, 
                                                profile_list,
                                                &profile_list_size,
                                                &qmi_err_code)) != QMI_NO_ERR)
      {
        DS_PROFILE_LOGE("_3gpp2_get_list: Unable to query for profile list", 0);
        if (rc == QMI_EXTENDED_ERR)
        {
          return (ds_profile_status_etype)qmi_err_code;
        }
        else
        {
          return (ds_profile_status_etype)DSI_FAILURE;
        }
      }
      else
      {
        int i;
        for (i=0; i < profile_list_size; i++)
        {
            node.num = (ds_profile_num_type) profile_list[i].profile_index;
            node.name[0] = '\0';
            info_size = sizeof(ds_profile_3gpp2_list_info_type);
            if ( ds_util_list_add( hndl, &node, info_size) != DS_SUCCESS)
            {
              DS_PROFILE_LOGE("_3gpp2_get_list: unable to add node to list", 0);
              return_status = DS_PROFILE_REG_RESULT_FAIL;
              break;
            }
            list_is_empty = FALSE;
        }
      }
    }
    break;

    default:
      DS_PROFILE_LOGE("_3gpp2_get_list: EXIT with ERR", 0);
      return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_OP;
      break;
  }

  if ( ( return_status == DS_PROFILE_REG_RESULT_SUCCESS ) && 
       ( list_is_empty == TRUE ) )
    return_status = DS_PROFILE_REG_RESULT_LIST_END;

  return return_status;
} 

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_GET_LIST_NODE

DESCRIPTION
  This function is used to get info from a particular node in the list

PARAMETERS 
  hndl : iterator handle
  list_info : pointer to store information to be returned
  
DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS
  DS_PROFILE_REG_RESULT_FAIL
  
SIDE EFFECTS 
  
===========================================================================*/

ds_profile_status_etype dsi_profile_3gpp2_get_list_node(
  ds_util_itr_hndl_type  hndl,
  ds_profile_list_info_type  *list_info
)
{
  ds_profile_3gpp2_list_info_type node;
  uint32 info_size = sizeof(ds_profile_3gpp2_list_info_type);

  if ( ds_util_itr_get_data(hndl, (void *)&node, &info_size) != DS_SUCCESS)
  {
    DS_PROFILE_LOGE("_3gpp2_get_list_node: unable to get node from list", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  list_info->num = node.num; 
  if ( (list_info->name->len >= strlen( node.name )) && 
       (list_info->name->buf !=NULL) )
  {
    list_info->num = node.num;
    list_info->name->len =strlen( node.name );
    memcpy(list_info->name->buf, node.name, list_info->name->len);
  }
  else
  {
    list_info->name->buf = NULL;
  }
   
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_VALIDATE_PROFILE_NUM

DESCRIPTION
  This function is used to validate a profile number.

PARAMETERS 
  num : profile number
  
DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM
  DS_PROFILE_REG_RESULT_SUCCESS
  
SIDE EFFECTS 
  
===========================================================================*/

ds_profile_status_etype dsi_profile_3gpp2_validate_profile_num(
  ds_profile_num_type num
)
{
  (void)num;
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_QMI_RESET_PARAM

DESCRIPTION
  This function is used to reset a particular identifier value to default

PARAMETERS 
  num : profile number
  ident : identifier for the paramter

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/

ds_profile_status_etype dsi_profile_3gpp2_qmi_reset_param (  
  ds_profile_num_type         num,
  ds_profile_identifier_type  ident
)
{
  (void)ident; (void)num;
  /* reset param to default not supported */
  return DS_PROFILE_REG_RESULT_ERR_INVAL_OP;  
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_QMI_RESET_PROFILE_TO_DEFAULT

DESCRIPTION
  This function is used to reset profile to default values (No-op for 3GPP2)

PARAMETERS 
  num : profile number

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/

ds_profile_status_etype dsi_profile_3gpp2_qmi_reset_profile_to_default(  
  ds_profile_num_type         num
)
{
  (void)num;
  /* reset profile to default not supported */
  return DS_PROFILE_REG_RESULT_ERR_INVAL_OP;  
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_SET_DEFAULT_PROFILE

DESCRIPTION
  This function is used to set default profile number for a particular
  family (No-op for 3GPP2)

PARAMETERS 
  family : type of profile 
  num : profile number

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/

ds_profile_status_etype dsi_profile_3gpp2_set_default_profile(  
  uint32               family, 
  ds_profile_num_type  num
)
{
  (void)family; (void)num;
  /* set default profile not supported */
  return DS_PROFILE_REG_RESULT_ERR_INVAL_OP;  
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_GET_DEFAULT_PROFILE

DESCRIPTION
  This function is used to get default profile number for a particular
  family (No-op for 3GPP2)

PARAMETERS 
  family : type of profile 
  num : pointer to store profile number

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/

ds_profile_status_etype dsi_profile_3gpp2_get_default_profile(  
  uint32                 family, 
  ds_profile_num_type   *num
)
{
  (void)family; (void)num;
  /* get default profile not supported */
  return DS_PROFILE_REG_RESULT_ERR_INVAL_OP;  
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_CREATE

DESCRIPTION
  This function is used to create profile on modem (No-op for 3GPP2)

PARAMETERS 
  num : profile number

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_create(
  ds_profile_num_type  *num
)
{
  *num = 0;
  /* create profile not supported */
  return DS_PROFILE_REG_RESULT_ERR_INVAL_OP;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_DEL

DESCRIPTION
  This function is used to delete profile on modem (No-op for 3GPP2)

PARAMETERS 
  num : profile number

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp2_del(
  ds_profile_num_type  num 
)
{
  (void)num;
  /* delete profile not supported */
  return DS_PROFILE_REG_RESULT_ERR_INVAL_OP;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_GET_NUM_RANGE

DESCRIPTION
  This function is used to get the range of 3GPP2 profile numbers

PARAMETERS 
  min_num, max_num : pointers to store range (min & max profile numbers)

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/

void dsi_profile_3gpp2_get_num_range(
  uint16 *min_num,
  uint16 *max_num
)
{
  (void)min_num; (void)max_num;
  return;
}

/*===========================================================================
FUNCTION DS_PROFILE_3GPP2_QMI_INIT

DESCRIPTION
  This function is called on the library init. It initializes the function
  pointers to valid functions for 3gpp2

PARAMETERS 
  fntbl : pointer to table of function pointers

DEPENDENCIES 
  
RETURN VALUE 
  returns the mask for 3gpp2. (Used later as valid mask which is ORed value
  returned from all techs.
SIDE EFFECTS 
  
===========================================================================*/

uint8 ds_profile_3gpp2_qmi_init( tech_fntbl_type *fntbl )
{
  int qmi_err;
  DS_PROFILE_LOGD("3gpp2_qmi_init: ENTRY", 0);
  /* Init function pointers */
  fntbl->create         = dsi_profile_3gpp2_create;
  fntbl->del            = dsi_profile_3gpp2_del;
  fntbl->profile_read   = dsi_profile_3gpp2_qmi_read;
  fntbl->profile_write  = dsi_profile_3gpp2_qmi_write;
  fntbl->reset_param    = dsi_profile_3gpp2_qmi_reset_param;
  fntbl->reset_profile_to_default = dsi_profile_3gpp2_qmi_reset_profile_to_default;
  fntbl->set_default_profile      = dsi_profile_3gpp2_set_default_profile;
  fntbl->get_default_profile      = dsi_profile_3gpp2_get_default_profile;
  fntbl->get_list = dsi_profile_3gpp2_get_list;
  fntbl->get_list_node = dsi_profile_3gpp2_get_list_node;
  fntbl->validate_profile_num = dsi_profile_3gpp2_validate_profile_num;
  fntbl->get_num_range = dsi_profile_3gpp2_get_num_range;

  qmi_handle = qmi_init(NULL, NULL);
  if (qmi_handle < 0)
  {
    DS_PROFILE_LOGE("_3gpp2_qmi_init: QMI message library init failed ", 0);
    return DS_FALSE;
  }

  /* Initialize qmi connection */
  if (qmi_connection_init(QMI_PORT_RMNET_1, &qmi_err) < 0) 
  {
    DS_PROFILE_LOGE("_3gpp2_qmi_init: QMI connection init failed ", 0);
    return DS_FALSE;
  }

  /* Initialize WDS client */
  if ((qmi_3gpp2_client_handle = qmi_wds_srvc_init_client(QMI_PORT_RMNET_1, 
                                                          NULL, 
                                                          NULL, 
                                                          &qmi_err)) < 0) 
  {
    DS_PROFILE_LOGE("_3gpp2_qmi_init: wds client init failed ", 0);
    return DS_FALSE;
  }
   atexit(qmi_cleanup_3gpp2);
  DS_PROFILE_LOGD("3gpp2_qmi_init: EXIT with SUCCESS", 0);

  return (0x01 << DS_PROFILE_TECH_3GPP2);
}

/*lint -restore Restore lint error 655*/
/*lint -restore Restore lint error 641*/

#endif /* FEATURE_DS_PROFILE_ACCESS_QMI */

