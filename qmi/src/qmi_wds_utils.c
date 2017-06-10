/******************************************************************************
  @file    qmi_wds_utils.c
  @brief   The QMI WDS utility services layer.

  DESCRIPTION
  QMI WDS util service routines

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_wds_srvc_init_client() needs to be called before sending or receiving of any
  CTL service messages

  ---------------------------------------------------------------------------
  Copyright (c) 2007, 2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#include <memory.h>
#include <stdlib.h>
#include "qmi_i.h"
#include "qmi_service.h"
#include "qmi_wds_utils.h"
#include "qmi_wds_srvc_i.h"
#include "qmi_util.h"
#include "ds_sl_list.h"

/*===========================================================================
                          Utility Functions
===========================================================================*/
/*===========================================================================
  FUNCTION qmi_wds_utils_write_optional_profile_tlvs
===========================================================================*/
/*!
@brief
  Takes the input profile data in the form of a linked list, and writes
  it in TLV form to the tx buffer.  Buffer pointers and length indicators
  are adjusted to reflect new TLV.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note
  - This function frees the list elements, list node and the list head.

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
int
qmi_wds_utils_write_optional_profile_tlvs
(
  unsigned char                   **tx_buf,
  int                             *tx_buf_len,
  qmi_wds_profile_node_list_type  *profile_list
)
{

  qmi_wds_profile_node_type   *profile_node = NULL;
  qmi_wds_list_link_type      *list_node_link = NULL;

  if (!tx_buf || !profile_list ||!(*tx_buf)|| !tx_buf_len )
  {
    QMI_ERR_MSG_0("qmi_wds_utils_write_optional_profile_tlvs: Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }
  if (list_size(profile_list) == 0)
  {
    QMI_ERR_MSG_0("qmi_wds_utils_write_optional_profile_tlvs: Linked list empty, Nothing to format.\n");
    return QMI_NO_ERR;
  }
  if (list_size(profile_list) > 0)
  {
    list_node_link = list_peek_front(profile_list);
  }

  /*While there are list elements, process each element and prepare TLV*/
  while(list_node_link != NULL)
  {
    profile_node = (qmi_wds_profile_node_type *)list_node_link;

      if (qmi_util_write_std_tlv  (tx_buf,
                                     tx_buf_len,
                                     profile_node->profile_element.type,
                                     profile_node->profile_element.len,
                                     (void *)profile_node->profile_element.data) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
   list_node_link =  (qmi_wds_list_link_type *) list_peek_next(profile_list, list_node_link);

  }/*While*/

  return QMI_NO_ERR;
}
/*===========================================================================
  FUNCTION  qmi_wds_utils_read_common_profile_tlvs
===========================================================================*/
/*!
@brief
  Takes an input RX buffer containing UMTS profile TLV data
  and prepares a linked list of profile parameters.


@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Changes input buffer pointer and size variable
    - Reads data into params structure
*/
/*=========================================================================*/

static int
qmi_wds_utils_read_common_profile_tlvs
(
  unsigned long                       type,
  unsigned long                       length,
  unsigned char                       *value_ptr,
  qmi_wds_profile_node_list_type      *profile_list
)
{
  qmi_wds_profile_node_type *profile_node = NULL;

  /*create a Node Element*/

  if (!value_ptr || !profile_list)
  {
    QMI_ERR_MSG_0("qmi_wds_utils_read_common_profile_tlvs: Bad Input received \n");
    return QMI_INTERNAL_ERR;
  }

  profile_node =
    (qmi_wds_profile_node_type *)malloc (sizeof(qmi_wds_profile_node_type));

  if (!profile_node)
  {
    QMI_ERR_MSG_0("qmi_wds_utils_read_common_profile_tlvs:unable to malloc memory for profile node \n");
    return QMI_INTERNAL_ERR;
  }

  memset (profile_node,0,sizeof(qmi_wds_profile_node_type));

  profile_node->profile_element.data = (unsigned char *)malloc(length);
  if (!profile_node->profile_element.data)
  {
    QMI_ERR_MSG_0("qmi_wds_utils_read_common_profile_tlvs:unable to malloc memory for profile node's data \n");
    free(profile_node);
    return QMI_INTERNAL_ERR;
  }

  profile_node->profile_element.type  = (unsigned char)type;
  profile_node->profile_element.len   = (unsigned short)length;
  memcpy(profile_node->profile_element.data,value_ptr,length);

  /*If we reach here, a profile node has been successfully created,
    Insert the node into the list Now*/
  list_push_back(profile_list,(qmi_wds_list_link_type *)profile_node);

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_cleanup_list
===========================================================================*/
/*!
@brief
  Takes as input a linked list head node and frees up the entire list.
  i.e. all the list nodes elements, the nodes itself and the list head node.


@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
void
qmi_wds_utils_cleanup_list
(
  qmi_wds_profile_node_list_type      *profile_list
)
{
  qmi_wds_profile_node_type   *profile_node;
  if (!profile_list)
  {
    return;
  }

  while (list_size(profile_list) > 0)
  {
    profile_node = (qmi_wds_profile_node_type *)list_pop_front(profile_list);
    if (profile_node && profile_node->profile_element.data)
    {
      free(profile_node->profile_element.data);
    }
    if (profile_node)
    {
      free(profile_node);
    }
  }

  /*Free the list head node here*/
  profile_list->back_ptr = NULL;
  profile_list->front_ptr = NULL;
  profile_list->size = 0;
 return;
}

/*===========================================================================
  FUNCTION  qmi_wds_utils_read_query_profile_tlvs
===========================================================================*/
/*!
@brief
  Takes an input RX buffer containing UMTS profile TLV data
  and prepares a linked list of profile params.


@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note
  - This function generates the list nodes and the elements within the nodes.
  - Dependencies
    - None.

  - Side Effects
    - Changes input buffer pointer and size variable
    - Reads data into params structure
*/
/*=========================================================================*/

static int
qmi_wds_utils_read_query_profile_tlvs
(
  unsigned char                       *rx_buf,
  int                                 rx_buf_len,
  qmi_wds_profile_id_type             *profile_id,
  qmi_wds_profile_node_list_type      *profile_list
)
{
  unsigned long type   =  0xFFFFFFFF;
  unsigned long length =  0xFFFFFFFF;
  unsigned char *value_ptr;

  if ( !rx_buf || !profile_id || !profile_list)
  {
    QMI_ERR_MSG_0 ("qmi_wds_utils_read_query_profile_tlvs: Bad Input Received\n");
    return QMI_INTERNAL_ERR;
  }

  /* Loop through all TLV's and process each one */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (   &rx_buf,
                                  &rx_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
    /* If the TLV is not one of the common profile TLV's, try to process */
    if (qmi_wds_utils_read_common_profile_tlvs (type,length,value_ptr,profile_list)== QMI_INTERNAL_ERR)
    {
      goto err_label;
    } /* if */
  } /* while */

  return QMI_NO_ERR;

err_label:
  qmi_wds_utils_cleanup_list(profile_list);
  return QMI_INTERNAL_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_utils_encode_3gpp_params
===========================================================================*/
/*!
@brief
  Takes 3GPP profile parameter mask and value, and encodes node with
  TLV values

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note
  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
static int
qmi_wds_utils_encode_3gpp_params
(
  uint64_t                    param_mask,
  void                      * val_ptr,
  qmi_wds_profile_node_type * node_ptr
)
{
  if (!val_ptr || !node_ptr )
  {
    QMI_ERR_MSG_0("qmi_wds_utils_encode_3gpp_params: Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  memset( node_ptr, 0x0, sizeof(qmi_wds_profile_node_type) );

  switch( param_mask )
  {
    case QMI_WDS_UMTS_PROFILE_NAME_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_NAME_TLV_ID;
      node_ptr->profile_element.len  = (QMI_WDS_MAX_PROFILE_STR_SIZE-1);
      break;

    case QMI_WDS_UMTS_PROFILE_PDP_TYPE_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_PDP_TYPE_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_APN_NAME_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_APN_NAME_TLV_ID;
      node_ptr->profile_element.len  = (QMI_WDS_MAX_APN_STR_SIZE-1);
      break;

    case QMI_WDS_UMTS_PROFILE_PRIM_DNS_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_PRIM_DNS_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_SEC_DNS_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_SEC_DNS_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_UMTS_REQ_QOS_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_UMTS_REQ_QOS_TLV_ID;
      node_ptr->profile_element.len  = sizeof(qmi_wds_umts_qos_params_type);
      break;

    case QMI_WDS_UMTS_PROFILE_UMTS_MIN_QOS_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_UMTS_MIN_QOS_TLV_ID;
      node_ptr->profile_element.len  = sizeof(qmi_wds_umts_qos_params_type);
      break;

    case QMI_WDS_UMTS_PROFILE_GPRS_REQ_QOS_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_GPRS_REQ_QOS_TLV_ID;
      node_ptr->profile_element.len  = sizeof(qmi_wds_gprs_qos_params_type);
      break;

    case QMI_WDS_UMTS_PROFILE_GPRS_MIN_QOS_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_GPRS_MIN_QOS_TLV_ID;
      node_ptr->profile_element.len  = sizeof(qmi_wds_gprs_qos_params_type);
      break;

    case QMI_WDS_UMTS_PROFILE_USERNAME_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_USERNAME_TLV_ID;
      node_ptr->profile_element.len  = (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1);
      break;

    case QMI_WDS_UMTS_PROFILE_AUTH_PREF_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_AUTH_PREF_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_IPV4_ADDR_PREF_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_IPV4_ADDR_PREF_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_PCSCF_ADDR_VIA_PCO_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_PCSCF_ADDR_VIA_PCO_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_HEADER_COMPRESSION_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_HEADER_COMPRESSION_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_DATA_COMPRESSION_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_DATA_COMPRESSION_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_PDP_ACCESS_CONTROL_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_PDP_ACCESS_CONTROL_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_PCSCF_VIA_DHCP_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_PCSCF_VIA_DHCP_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_IM_CN_FLAG_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_IM_CN_FLAG_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_TFT_FILTER_ID_1_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_TFT_FILTER_ID_1_TLV_ID;
      node_ptr->profile_element.len  = sizeof(qmi_wds_tft_params_type);
      break;

    case QMI_WDS_UMTS_PROFILE_TFT_FILTER_ID_2_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_TFT_FILTER_ID_2_TLV_ID;
      node_ptr->profile_element.len  = sizeof(qmi_wds_tft_params_type);
      break;

    case QMI_WDS_UMTS_PROFILE_PDP_CONTEXT_NUMBER_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_PDP_CONTEXT_NUMBER_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_PDP_CONTEXT_SECONDARY_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_PDP_CONTEXT_SECONDARY_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_PDP_PRIMARY_ID_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_PDP_PRIMARY_ID_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_UMTS_REQ_QOS_EXT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_UMTS_REQ_QOS_EXT_TLV_ID;
      node_ptr->profile_element.len  = sizeof(qmi_wds_umts_qos_params_type);
      break;

    case QMI_WDS_UMTS_PROFILE_UMTS_MIN_QOS_EXT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_UMTS_MIN_QOS_EXT_TLV_ID;
      node_ptr->profile_element.len  = sizeof(qmi_wds_umts_qos_params_type);
      break;

    case QMI_WDS_UMTS_PROFILE_PRIMARY_DNS_IPV6_ADDR_PREF_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_PRIMARY_DNS_IPV6_ADDR_PREF_TLV_ID;
      node_ptr->profile_element.len  = QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES;
      break;

    case QMI_WDS_UMTS_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_TLV_ID;
      node_ptr->profile_element.len  = QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES;
      break;

    case QMI_WDS_UMTS_PROFILE_ADDR_ALLOC_PREF_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_ADDR_ALLOC_PREF_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_LTE_QOS_PARAMS_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_LTE_QOS_PARAMS_TLV_ID;
      node_ptr->profile_element.len  = sizeof(qmi_wds_lte_qos_params_type);
      break;

    case QMI_WDS_UMTS_PROFILE_APN_DISABLED_FLAG_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_APN_DISABLED_FLAG_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_PDN_INACTIVITY_TIMEOUT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_PDN_INACTIVITY_TIMEOUT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_APN_CLASS_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_APN_CLASS_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_APN_BEARER_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_APN_BEARER_TLV_ID;
      node_ptr->profile_element.len  = SIZE_64_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_SUPPORT_EMERGENCY_CALLS_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_SUPPORT_EMERGENCY_CALLS_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_OP_PCO_ID_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_OP_PCO_ID_TLV_ID;
      node_ptr->profile_element.len  = SIZE_16_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_PCO_MCC_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_PCO_MCC_TLV_ID;
      node_ptr->profile_element.len  = SIZE_16_BIT_VAL;
      break;

    case QMI_WDS_UMTS_PROFILE_MNC_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_MNC_TLV_ID;
      node_ptr->profile_element.len  = sizeof(qmi_wds_mnc_type);
      break;

    case QMI_WDS_UMTS_PROFILE_IPV6_ADDR_PREF_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_UMTS_PROFILE_IPV6_ADDR_PREF_TLV_ID;
      node_ptr->profile_element.len  = QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES;
      break;

    case QMI_WDS_UMTS_PROFILE_PASSWORD_PARAM_MASK:
    default:
      QMI_ERR_MSG_1("qmi_wds_utils_encode_3gpp_params: unsupported profile parameter[0x%x]\n", param_mask);
      return QMI_INTERNAL_ERR;
  }

  if( node_ptr->profile_element.len )
  {
    /* Assign address value */
    node_ptr->profile_element.data = val_ptr;
  }

  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_utils_encode_3gpp2_params
===========================================================================*/
/*!
@brief
  Takes 3GPP2 profile parameter mask and value, and encodes node with
  TLV values

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note
  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
static int
qmi_wds_utils_encode_3gpp2_params
(
  uint64_t                    param_mask,
  void                      * val_ptr,
  qmi_wds_profile_node_type * node_ptr
)
{
  if (!val_ptr || !node_ptr )
  {
    QMI_ERR_MSG_0("qmi_wds_utils_encode_cdma_params: Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  memset( node_ptr, 0x0, sizeof(qmi_wds_profile_node_type) );

  switch( param_mask )
  {
    case QMI_WDS_CDMA_PROFILE_DNS_SERVER_PREF_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_DNS_SERVER_PREF_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_DO_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_DO_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_1X_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_1X_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_ALLOW_LINGER_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_ALLOW_LINGER_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_LCP_ACK_TIMEOUT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_LCP_ACK_TIMEOUT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_16_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_AUTH_TIMEOUT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_AUTH_TIMEOUT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_16_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_LCP_CONFIG_RETRY_RECOUNT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_LCP_CONFIG_RETRY_RECOUNT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_AUTH_RETRY_RECOUNT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_AUTH_RETRY_RECOUNT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_AUTH_PROTOCOL_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_AUTH_PROTOCOL_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_USERNAME_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_USERNAME_TLV_ID;
      node_ptr->profile_element.len  = (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1);
      break;

    case QMI_WDS_CDMA_PROFILE_DATA_RATE_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_DATA_RATE_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_DATA_MODE_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_DATA_MODE_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_APP_TYPE_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_APP_TYPE_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_APP_PRIORITY_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_APP_TYPE_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_APN_STRING_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_APN_STRING_TLV_ID;
      node_ptr->profile_element.len  = (QMI_WDS_MAX_APN_STR_SIZE-1);
      break;

    case QMI_WDS_CDMA_PROFILE_PDN_TYPE_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_PDN_TYPE_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_IS_PCSCF_ADDR_NEEDED_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_IS_PCSCF_ADDR_NEEDED_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_PRIM_V4_DNS_ADDR_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_PRIM_V4_DNS_ADDR_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_SEC_V4_DNS_ADDR_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_SEC_V4_DNS_ADDR_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_PRIM_V6_DNS_ADDR_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_PRIM_V6_DNS_ADDR_TLV_ID;
      node_ptr->profile_element.len  = QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES;
      break;

    case QMI_WDS_CDMA_PROFILE_SEC_V6_DNS_ADDR_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_SEC_V6_DNS_ADDR_TLV_ID;
      node_ptr->profile_element.len  = QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES;
      break;

    case QMI_WDS_CDMA_PROFILE_IPCP_ACK_TIMEOUT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_IPCP_ACK_TIMEOUT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_16_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_IPCP_CONFIG_RETRY_RECOUNT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_IPCP_CONFIG_RETRY_RECOUNT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_RAT_TYPE_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_RAT_TYPE_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_APN_ENABLED_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_APN_ENABLED_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_PDN_INACTIVITY_TIMEOUT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_PDN_INACTIVITY_TIMEOUT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_APN_CLASS_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_APN_CLASS_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_PDN_LEVEL_USER_ID_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_PDN_LEVEL_USER_ID_TLV_ID;
      node_ptr->profile_element.len  = (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1);
      break;

    case QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PWD_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PWD_TLV_ID;
      node_ptr->profile_element.len  = (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1);
      break;

    case QMI_WDS_CDMA_PROFILE_PDN_LABEL_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_PDN_LABEL_TLV_ID;
      node_ptr->profile_element.len  = (QMI_WDS_MAX_APN_STR_SIZE-1);
      break;

    case QMI_WDS_CDMA_PROFILE_OPERATOR_RESERVED_PCO_ID_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_OPERATOR_RESERVED_PCO_ID_TLV_ID;
      node_ptr->profile_element.len  = SIZE_16_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_MCC_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_MCC_TLV_ID;
      node_ptr->profile_element.len  = SIZE_16_BIT_VAL;
      break;

    case QMI_WDS_CDMA_PROFILE_MNC_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_CDMA_PROFILE_MNC_TLV_ID;
      node_ptr->profile_element.len  = sizeof(qmi_wds_mnc_type);
      break;

    case QMI_WDS_CDMA_PROFILE_PASSWORD_PARAM_MASK:
    default:
      QMI_ERR_MSG_1("qmi_wds_utils_encode_3gpp2_params: unsupported profile parameter[0x%x]\n", param_mask);
      return QMI_INTERNAL_ERR;
  }

  if( node_ptr->profile_element.len )
  {
    /* Assign address value */
    node_ptr->profile_element.data = val_ptr;
  }

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_utils_encode_epc_params
===========================================================================*/
/*!
@brief
  Takes EPC profile parameter mask and value, and encodes node with
  TLV values

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note
  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
static int
qmi_wds_utils_encode_epc_params
(
  uint64_t                    param_mask,
  void                      * val_ptr,
  qmi_wds_profile_node_type * node_ptr
)
{
  if (!val_ptr || !node_ptr )
  {
    QMI_ERR_MSG_0("qmi_wds_utils_encode_cdma_params: Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  memset( node_ptr, 0x0, sizeof(qmi_wds_profile_node_type) );

  switch( param_mask )
  {

    case QMI_WDS_EPC_PROFILE_NAME_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_UMTS_PROFILE_NAME_TLV_ID;
      node_ptr->profile_element.len  = (QMI_WDS_MAX_PROFILE_STR_SIZE-1);
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_DNS_SERVER_PREF_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_DNS_SERVER_PREF_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_DO_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_DO_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_1X_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_1X_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_ALLOW_LINGER_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_ALLOW_LINGER_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_LCP_ACK_TIMEOUT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_LCP_ACK_TIMEOUT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_16_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_AUTH_TIMEOUT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_AUTH_TIMEOUT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_16_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_LCP_CONFIG_RETRY_RECOUNT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_LCP_CONFIG_RETRY_RECOUNT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_AUTH_RETRY_RECOUNT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_AUTH_RETRY_RECOUNT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_AUTH_PROTOCOL_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_AUTH_PROTOCOL_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_PROFILE_USERNAME_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_USERNAME_TLV_ID;
      node_ptr->profile_element.len  = (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1);
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_DATA_RATE_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_DATA_RATE_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_DATA_MODE_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_DATA_MODE_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_APP_TYPE_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_APP_TYPE_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_APP_PRIORITY_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_APP_PRIORITY_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_PROFILE_APN_NAME_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_APN_STRING_TLV_ID;
      node_ptr->profile_element.len  = (QMI_WDS_MAX_APN_STR_SIZE-1);
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_PDN_TYPE_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_PDN_TYPE_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_IS_PCSCF_ADDR_NEEDED_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_IS_PCSCF_ADDR_NEEDED_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_PRIM_V4_DNS_ADDR_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_PRIM_V4_DNS_ADDR_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_SEC_V4_DNS_ADDR_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_SEC_V4_DNS_ADDR_TLV_ID;
      node_ptr->profile_element.len  = SIZE_32_BIT_VAL;
      break;

    case QMI_WDS_EPC_PROFILE_PRIMARY_DNS_IPV6_ADDR_PREF_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_PRIM_V6_DNS_ADDR_TLV_ID;
      node_ptr->profile_element.len  = QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES;
      break;

    case QMI_WDS_EPC_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_SEC_V6_DNS_ADDR_TLV_ID;
      node_ptr->profile_element.len  = QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_IPCP_ACK_TIMEOUT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_IPCP_ACK_TIMEOUT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_16_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_IPCP_CONFIG_RETRY_RECOUNT_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_IPCP_CONFIG_RETRY_RECOUNT_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_CDMA_PROFILE_RAT_TYPE_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_CDMA_PROFILE_RAT_TYPE_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_COMMON_AUTH_PROTOCOL_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_COMMON_AUTH_PROTOCOL_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_COMMON_USER_ID_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_COMMON_USER_ID_TLV_ID;
      node_ptr->profile_element.len  = (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1);
      break;

    case QMI_WDS_EPC_COMMON_APN_CLASS_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_COMMON_APN_CLASS_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

   case QMI_WDS_EPC_COMMON_APN_DISABLED_PARAM_MASK:
      node_ptr->profile_element.type = QMI_WDS_EPC_COMMON_APN_DISABLED_FLAG_TLV_ID;
      node_ptr->profile_element.len  = SIZE_8_BIT_VAL;
      break;

    case QMI_WDS_EPC_PROFILE_PASSWORD_PARAM_MASK:
    case QMI_WDS_EPC_COMMON_AUTH_PASSWORD_PARAM_MASK:/* TBD? */
    default:
      QMI_ERR_MSG_1("qmi_wds_utils_encode_epc_params: unsupported profile parameter[0x%x]\n", param_mask);
      return QMI_INTERNAL_ERR;

  }

  if( node_ptr->profile_element.len )
  {
    /* Assign address value */
    node_ptr->profile_element.data = val_ptr;
  }

  return QMI_NO_ERR;
}

/*===========================================================================
  List version WDS Profile API's
===========================================================================*/

/*===========================================================================
  FUNCTION  qmi_wds_utils_create_profile
===========================================================================*/
/*!
@brief
  The List version of the qmi_wds_create_profile API.
  Takes as input a user client handle, profile params in the form of a linked list
  and a WDS profile ID structure and creates a QMI profile (on modem processor)
  based on these parameters.  The input profile ID MUST have the 'technology'
  field set to a valid value.

@return
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is
  QMI_SERVICE_ERR,then the qmi_err_code will be valid and will indicate which
  QMI error occurred.  Upon successful  return, the profile ID parameter will
  have the 'profile_index' field set to the profile index of the newly created
  profile.

  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.
@note

  - This function executes synchronously.
  - Dependencies
    - None.

  - Side Effects
    - Creates a new profile on the modem processor.
*/
/*=========================================================================*/
int
qmi_wds_utils_create_profile
(
  int                               user_handle,
  qmi_wds_profile_id_type           *profile_id,
  qmi_wds_profile_node_list_type    *profile_list,
  int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc = FALSE;

  if ( !profile_id  || !profile_list || !qmi_err_code)
  {
    QMI_ERR_MSG_0 ("qmi_wds_utils_create_profile: Bad Input received.\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Write profile ID technology type to TLV buffer (mandatory) */
  if ( profile_id->technology != QMI_WDS_PROFILE_TECH_3GPP ||
      (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                  &msg_size,
                                  QMI_WDS_PROFILE_ID_TLV_ID,
                                  sizeof(byte),
                                  (void *)&profile_id->technology) < 0))
  {
    return QMI_INTERNAL_ERR;
  }

  /*Profile Params are Optional*/
  /*lint --e{506,774} */
  if (profile_list && list_is_valid(profile_list))
  {
    if (qmi_wds_utils_write_optional_profile_tlvs (&tmp_msg_ptr,
                                                &msg_size,
                                                profile_list) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Synchronously send message to modem processor */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_CREATE_PROFILE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Get profile ID information */
  if (rc == QMI_NO_ERR)
  {
    rc = qmi_wds_read_profile_id_tlv (tmp_msg_ptr,(unsigned long)msg_size,profile_id,TRUE);
  }
  /* Retrieve the extended error code if it exists */
  else if (rc == QMI_SERVICE_ERR && *qmi_err_code == QMI_SERVICE_ERR_EXTENDED_INTERNAL)
  {
    rc = QMI_EXTENDED_ERR;
    if ( qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                        &msg_size,
                                        qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_utils_create_profile: Failed to read the error response");
      rc = QMI_INTERNAL_ERR;
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_utils_modify_profile
===========================================================================*/
/*!
@brief
  The List version of the qmi_wds_modify_profile API
  Takes as input a user client handle, WDS profile parameters as a linked list
  and a WDS profile ID structure and modifies parameters of an existing profile
  on the modem processor. The input profile ID MUST have the 'technology'
  field and 'profile_index' set to an existing valid profile index value.


@return
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is
  QMI_SERVICE_ERR,then the qmi_err_code will be valid and will indicate which
  QMI error occurred.

  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.

@note

  - This function executes synchronously.

  - Dependencies
    - None.

  - Side Effects
    - Modifies an existing profile on the modem processor.
*/
/*=========================================================================*/
int
qmi_wds_utils_modify_profile
(
  int                               user_handle,
  qmi_wds_profile_id_type           *profile_id,
  qmi_wds_profile_node_list_type    *profile_list,
  int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc = FALSE;

  if (!profile_id  || !profile_list || !qmi_err_code)
  {
    QMI_ERR_MSG_0 ("qmi_wds_utils_modify_profile: Profile id not received\n");
    return QMI_INTERNAL_ERR;
  }
  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Write profile ID technology type to TLV buffer (mandatory) */
  if (qmi_wds_write_profile_id_tlv (&tmp_msg_ptr,
                                  &msg_size,
                                    (void *)profile_id)  < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /*lint --e{506,774} */
  if (profile_list && list_is_valid(profile_list))
  {
    if (qmi_wds_utils_write_optional_profile_tlvs (&tmp_msg_ptr,
                                                &msg_size,
                                                profile_list) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Synchronously send message to modem processor */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_MODIFY_PROFILE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Retrieve the extended error code if it exists */
  if (rc == QMI_SERVICE_ERR && *qmi_err_code == QMI_SERVICE_ERR_EXTENDED_INTERNAL)
  {
    rc = QMI_EXTENDED_ERR;
    if ( qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                        &msg_size,
                                        qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_utils_modify_profile: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_utils_query_profile
===========================================================================*/
/*!
@brief
  The list version of the qmi_wds_query_profile
  Takes as input a user client handle, and a WDS  profile ID structure and
  queries the parameters of an existing profileon the modem processor. The
  input profile ID MUST have the 'technology' field and 'profile_index' set
  to an existing valid profile index value. The current values of the parameters
  will be returned in the profile_params structure.

  On a successful return the profile_list will point to a linked list of
  profile params for the given profile Id.

@return
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is
  QMI_SERVICE_ERR,then the qmi_err_code will be valid and will indicate which
  QMI error occurred.

  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.
@note

  - This function executes synchronously.
  - The library generates a linked list of profile params on successful query.
  - The client should free up the linked list i.e. the data of each node,
    the node itself and list's head node.

  - Dependencies
    - None.

  - Side Effects
    - Queries an existing profile on the modem processor.
*/
/*=========================================================================*/
int
qmi_wds_utils_query_profile
(
  int                               user_handle,
  qmi_wds_profile_id_type           *profile_id,
  qmi_wds_profile_node_list_type    *profile_list,/*Should point to a VALID list head node*/
                                                  /*Sucessful return(QMI_NO_ERR) will have this point
                                                   to a linked list of profile params*/
  int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc = FALSE;

  if (!profile_id || !qmi_err_code ||
      !profile_list || profile_list->front_ptr != NULL ||
      profile_list->back_ptr != NULL || profile_list->size != 0)
  {
     QMI_ERR_MSG_0 ("qmi_wds_utils_query_profile: Bad input received!\n");
     return QMI_INTERNAL_ERR;
  }
  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Write profile ID data (mandatory) to TLV buffer */
  if ( qmi_wds_write_profile_id_tlv (&tmp_msg_ptr,
                                     &msg_size,
                                     profile_id) < 0)
  {
    return QMI_INTERNAL_ERR;
  }


  /* Synchronously send message to modem processor */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_PROFILE_DATA_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);


   /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Get profile settings information */
  if (rc == QMI_NO_ERR)
  {
    rc = qmi_wds_utils_read_query_profile_tlvs (tmp_msg_ptr,msg_size,profile_id,profile_list);
  }

  if (rc != QMI_NO_ERR)
  {
    profile_list = NULL;
  }

  /* Retrieve the extended error code if it exists */
  if (rc == QMI_SERVICE_ERR && *qmi_err_code == QMI_SERVICE_ERR_EXTENDED_INTERNAL)
  {
    rc = QMI_EXTENDED_ERR;
    if ( qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                        &msg_size,
                                        qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_utils_query_profile: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_utils_get_profile_list
===========================================================================*/
/*!
@brief
  Takes as input a user client handle, and an array of profile query
  structures in which to place response, and the number of elements in that
  array. Also optionally the technology and key-value search pairs(formatted as
  tlvs linked list) can be given as input, these will be used to further filter
  the search on the modem.

  Note that the num_profile_list_elements is both an input and output
  parameter.  It should be used to indicated the number of elements in
  the profile_list array when calling the function, and will contain the
  number of elements returned when the function returns.

@return
  Returned is the array filled in with return data, and the number of
  elements of the array that have been returned.
  QMI_INTERNAL_ERR or QMI_SERVICE_ERR if an error occurred, QMI_NO_ERR if not.
  If QMI_SERVICE_ERR is returned, the qmi_err_code return parameter
  will be valid, otherwise it won't
  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_wds_utils_get_profile_list
(
  int                             user_handle,
  qmi_wds_profile_tech_type       *profile_tech,
  qmi_wds_profile_node_list_type  *profile_search_list,
  qmi_wds_profile_list_type       *profile_list,
  int                             *num_profile_list_elements,
  int                             *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int               rc, i, temp;

  unsigned long     type;
  unsigned long     length;
  unsigned char     *value_ptr;

  if( !profile_list || !num_profile_list_elements || !qmi_err_code )
  {
    QMI_ERR_MSG_0("qmi_wds_utils_get_profile_list: Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /*write the options tlvs if any*/
  if (profile_tech)
  {
    unsigned char tmp = (unsigned char)*profile_tech;

    QMI_ERR_MSG_1 ("qmi_wds_get_profile_list: profile technology sent %d",(unsigned int)tmp);

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_GET_PROFILE_LIST_TECH_TYPE_TLV_ID,
                                1,
                                (void *)&tmp) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

  }
  if (profile_search_list)
  {
    if (qmi_wds_utils_write_optional_profile_tlvs (&tmp_msg_ptr,
                                                   &msg_size,
                                                   profile_search_list) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Send the message */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_PROFILE_LIST_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);


  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Get profile settings information */
  if (rc == QMI_NO_ERR)
  {
    if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                  &msg_size,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      rc = QMI_INTERNAL_ERR;
    }
    else if (type != QMI_WDS_PROFILE_LIST_TLV_ID)
    {
      rc = QMI_INTERNAL_ERR;
    }
    else if (msg_size != 0)
    {
      rc = QMI_INTERNAL_ERR;
    }
    else
    {

      /* Read the number of profiles returned */
      READ_8_BIT_VAL (value_ptr,temp);

      /* Set the number of profiles to be returned as the lesser of the number
      ** returned and the number in the client's array
      */
      *num_profile_list_elements = (temp < *num_profile_list_elements)
                                    ? temp : *num_profile_list_elements;

      /* Read the profile list elements */
      for (i=0; i < *num_profile_list_elements; i++)
      {
        /* Read and save the profile type */
        READ_8_BIT_VAL (value_ptr,temp);
        profile_list->profile_type = (qmi_wds_profile_tech_type) temp;

        /* Read the profile index */
        READ_8_BIT_VAL (value_ptr,temp);
        profile_list->profile_index = (unsigned long)temp;

        /* Read the length of the profile ID string and copy it into client data */
        READ_8_BIT_VAL (value_ptr,temp);
        memcpy (profile_list->profile_name, (void *)value_ptr, (size_t)temp);
        profile_list->profile_name[temp] = '\0';
        value_ptr += temp;

        /* increment client profile data pointer */
        profile_list++;
      }
    }
  }
  /* Retrieve the extended error code if it exists */
  else if (rc == QMI_SERVICE_ERR && *qmi_err_code == QMI_SERVICE_ERR_EXTENDED_INTERNAL)
  {
    rc = QMI_EXTENDED_ERR;
    if ( qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                        &msg_size,
                                        qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_get_profile_list: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_utils_get_profile_list2
===========================================================================*/
/*!
@brief
  Takes as input a user client handle, and an array of profile query
  structures in which to place response, and the number of elements in
  that array. Also optionally the technology and single key-value search
  pair (formatted as parameter mask and value) can be given as input,
  these will be used to further filter the search on the modem.

  Note that the num_profile_list_elements is both an input and output
  parameter.  It should be used to indicated the number of elements in
  the profile_list array when calling the function, and will contain the
  number of elements returned when the function returns.

@return
  Returned is the array filled in with return data, and the number of
  elements of the array that have been returned.
  QMI_INTERNAL_ERR or QMI_SERVICE_ERR if an error occurred, QMI_NO_ERR if not.
  If QMI_SERVICE_ERR is returned, the qmi_err_code return parameter
  will be valid, otherwise it won't
  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_wds_utils_get_profile_list2
(
  int                              user_handle,
  qmi_wds_profile_tech_type        profile_tech,
  uint64_t                         param_mask,
  void                            *param_value,
  qmi_wds_profile_list_type       *profile_list,
  int                             *num_profile_list_elements,
  int                             *qmi_err_code
)
{
  qmi_wds_profile_node_list_type prof_param_list;
  qmi_wds_profile_node_type node;
  qmi_wds_profile_node_type *node_ptr = &node;
  int rc = QMI_NO_ERR;

  if( !profile_list || !num_profile_list_elements || !qmi_err_code )
  {
    QMI_ERR_MSG_0("qmi_wds_utils_get_profile_list2: Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  list_init( &prof_param_list );

  switch( profile_tech )
  {
    case QMI_WDS_PROFILE_TECH_3GPP:
      rc = qmi_wds_utils_encode_3gpp_params( param_mask, param_value, node_ptr );
      break;
    case QMI_WDS_PROFILE_TECH_3GPP2:
      rc = qmi_wds_utils_encode_3gpp2_params( param_mask, param_value, node_ptr );
      break;
    case QMI_WDS_PROFILE_TECH_EPC:
      rc = qmi_wds_utils_encode_epc_params( param_mask, param_value, node_ptr );
      break;
    default:
      QMI_DEBUG_MSG_0("qmi_wds_utils_get_profile_list2: no profile tech specified\n");
      break;
  }

  if( QMI_NO_ERR != rc )
  {
    return QMI_INTERNAL_ERR;
  }

  /* Build list of key-value pairs.  Note only one pair supported at this time. */
  list_push_back( &prof_param_list, (qmi_wds_list_link_type*)node_ptr );

  rc = qmi_wds_utils_get_profile_list( user_handle,
                                       &profile_tech,
                                       &prof_param_list,
                                       profile_list,
                                       num_profile_list_elements,
                                       qmi_err_code);
  if (rc != QMI_NO_ERR)
  {
    QMI_ERR_MSG_0("qmi_wds_utils_get_profile_list2: failed on qmi_wds_utils_get_profile_list\n");
    return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;
}
