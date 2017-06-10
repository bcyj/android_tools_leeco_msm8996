/******************************************************************************
  @file    ds_profile_3gpp_qmi.c
  @brief   

  DESCRIPTION
  Tech specific implementation of 3GPP Profile Management  

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  N/A

  ---------------------------------------------------------------------------
  Copyright (C) 2009-2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
****************************************************************************/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/modem/data/1x/707/main/latest/src/ds707_data_session_profile.c#32 $ $DateTime: 2009/09/11 10:21:08 $ $Author: lipings $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   SM      Created the module. First version of the file.
===========================================================================*/

#include "comdef.h"

#ifdef FEATURE_DS_PROFILE_ACCESS_QMI

#include "qmi_wds_srvc.h"
#include "qmi_wds_utils.h"
#include "ds_profilei.h"
#include "ds_profile_3gppi.h"
#include "ds_profile_os.h"
#include "ds_util.h"

/*Global Definitions*/
static int qmi_3gpp_client_handle;

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


/*utility functions/Macros*/

/*The following two macros are required for copying data 
from packed to unpacked structures and vice versa*/

#define WRITE_N_BYTE_VAL(dst,buf,n) \
do { unsigned char *b_ptr = (unsigned char *)dst; \
      unsigned char *v_ptr = (buf); \
      int unlikely_cntr; \
      for (unlikely_cntr=0; unlikely_cntr<(int)n; unlikely_cntr++) {*b_ptr++ = *v_ptr++;} \
      dst = b_ptr; \
    } while (0)

#define READ_N_BYTE_VAL(src,dst,n) \
do { unsigned char *d_ptr = (unsigned char *)(dst); \
      unsigned char *s_ptr = (unsigned char *)(src); \
      int unlikely_cntr; \
      for (unlikely_cntr=0; unlikely_cntr<(int)n; unlikely_cntr++) {*d_ptr++ = *s_ptr++;} \
      src = s_ptr; \
    } while (0)


#define DSI_PROFILE_CREATE_LIST_PROFILE_LIST_SIZE  15
#define DS_UMTS_MAX_PDP_PROFILE_NUM  16


typedef struct 
{
  ds_profile_identifier_type  ident;

  ds_profile_status_etype (*write_fn)(
    const void                          *ptr,
    void                                *node
  );
}dsi_profile_3gpp_write_fn_type;


/*===========================================================================
FUNCTION qmi_cleanup_3gpp

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
qmi_cleanup_3gpp
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

  if (qmi_3gpp_client_handle >= 0)
  {
    qmi_wds_srvc_release_client( qmi_3gpp_client_handle, &qmi_err );
    DS_PROFILE_LOGD("Releasing the WDS qmi_client_handle 0x%08x \n",
                      qmi_3gpp_client_handle);
  }
  /* Release QMI library connection */
  qmi_release(qmi_handle);
}
/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_QMI_WRITE_ set of functions

DESCRIPTION
  These functions create a list node with the TLV information. This node will be
  inserted into the double linked list.This list will be sent to the modem using 
  QMI msg library for either create-new/modify-existing profile.

PARAMETERS 
  num : profile number
  ptr : double pointer to a linked list node.
  
DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS
  DS_PROFILE_REG_RESULT_FAIL
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM
  
SIDE EFFECTS 
  
===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_invalid(
  const void           *ptr,
  void                 *node
)
{
  (void)ptr; (void)node;
  return DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT; 
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_profile_name(
  const void           *ptr,
  void                 *node
)
{
  void * tmp_data_ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;

  DS_PROFILE_LOGD("_3gpp_qmi_write_profile_name: Entry", 0);
  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_profile_name: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_PROFILE_NAME;
  profile_node->profile_element.len   = strlen((char *)umts_prf->profile_name);
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);

  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                  (unsigned char *)umts_prf->profile_name,
                  profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_pdp_context_pdp_type(
  const void           *ptr,
  void                 *node
)
{
  void * tmp_data_ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  DS_PROFILE_LOGD("_3gpp_qmi_write_pdp_context_pdp_type: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_pdp_context_pdp_type: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_TYPE;
  profile_node->profile_element.len   = sizeof(unsigned char);
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);

  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                  (unsigned char *)&umts_prf->context.pdp_type,
                  profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_pdp_context_h_comp(
  const void           *ptr,
  void                 *node
)
{
  void * tmp_data_ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  DS_PROFILE_LOGD("_3gpp_qmi_write_pdp_context_h_comp: Entry", 0);

  if ( ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_pdp_context_h_comp: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_H_COMP;
  profile_node->profile_element.len   = sizeof(unsigned char);

  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);

  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                  (unsigned char *)&umts_prf->context.h_comp,
                  profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_pdp_context_d_comp(
  const void           *ptr,
  void                 *node
)
{
  void * tmp_data_ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  DS_PROFILE_LOGD("_3gpp_qmi_write_pdp_context_d_comp: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_pdp_context_d_comp: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_D_COMP;
  profile_node->profile_element.len   = sizeof(unsigned char);

  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->context.d_comp,
                   profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_pdp_context_apn(
  const void           *ptr,
  void                 *node
)
{
  void * tmp_data_ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  DS_PROFILE_LOGD("_3gpp_qmi_write_pdp_context_apn: Entry", 0);

  if (ptr == NULL || profile_node == NULL)  
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_pdp_context_apn: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_APN;
  profile_node->profile_element.len   = strlen((char *)umts_prf->context.apn);

  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                  (unsigned char *)umts_prf->context.apn,
                  profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_pdp_context_ipv4_addr_alloc(
  const void           *ptr,
  void                 *node
)
{
  void * tmp_data_ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  DS_PROFILE_LOGD("_3gpp_modem_qmi_pdp_context_ipv4_addr_alloc: Entry", 0);

  if (ptr == NULL || profile_node == NULL)  
  {
    DS_PROFILE_LOGE("_3gpp_modem_qmi_pdp_context_ipv4_addr_alloc: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_IPV4_ADDR_ALLOC;
  profile_node->profile_element.len   = sizeof(unsigned char);

  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                  (unsigned char *)&umts_prf->context.ipv4_addr_alloc,
                  profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}
/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_dns_addr_primary(
  const void           *ptr,
  void                 *node
)
{
  void * tmp_data_ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  DS_PROFILE_LOGD("_3gpp_qmi_write_dns_addr_primary: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_dns_addr_primary: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  if ( (ds_profile_3gpp_ip_version_enum_type)umts_prf->dns_addr_primary_ip_vsn == DS_PROFILE_3GPP_IP_V4  ||
        (ds_profile_3gpp_ip_version_enum_type)umts_prf->dns_addr_primary_ip_vsn == DS_PROFILE_3GPP_IP_V4V6)
  {
    profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V4_PRIMARY;
    profile_node->profile_element.len   = sizeof(ipv4_addr_type);

    ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
    tmp_data_ptr = profile_node->profile_element.data;

    WRITE_N_BYTE_VAL(tmp_data_ptr,
                    (unsigned char *)&umts_prf->dns_addr.primary_dns_addr.ds_profile_3gpp_pdp_addr_ipv4,
                    profile_node->profile_element.len);

  }
  else if ((ds_profile_3gpp_ip_version_enum_type)umts_prf->dns_addr_primary_ip_vsn == DS_PROFILE_3GPP_IP_V6 ||
            (ds_profile_3gpp_ip_version_enum_type)umts_prf->dns_addr_primary_ip_vsn == DS_PROFILE_3GPP_IP_V4V6)
  {
    profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V6_PRIMARY;
    profile_node->profile_element.len   = sizeof(ipv6_addr_type);

    ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
    tmp_data_ptr = profile_node->profile_element.data;

    WRITE_N_BYTE_VAL(tmp_data_ptr,
                    (unsigned char *)&umts_prf->dns_addr.primary_dns_addr.ds_profile_3gpp_pdp_addr_ipv6,
                    profile_node->profile_element.len);

  }
  else
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_dns_addr_primary: DS_PROFILE_3GPP_IP_NOTHING", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_dns_address_secondary(
  const void           *ptr,
  void                 *node
)
{
  void * tmp_data_ptr;
  qmi_wds_profile_node_type *profile_node =  (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  DS_PROFILE_LOGD("_3gpp_qmi_write_dns_address_secondary: Entry", 0);

  if ( ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_dns_address_secondary: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  if ((ds_profile_3gpp_ip_version_enum_type)umts_prf->dns_addr_secondary_ip_vsn ==  DS_PROFILE_3GPP_IP_V4 ||
        (ds_profile_3gpp_ip_version_enum_type)umts_prf->dns_addr_secondary_ip_vsn ==  DS_PROFILE_3GPP_IP_V4V6)
  {
      DS_PROFILE_LOGD("_3gpp_qmi_write_dns_address_secondary: DS_PROFILE_3GPP_IP_V4", 0);
      profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V4_SECONDARY;
      profile_node->profile_element.len   = sizeof(ipv4_addr_type);

      ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
      tmp_data_ptr = profile_node->profile_element.data;

      WRITE_N_BYTE_VAL(tmp_data_ptr,
                      (unsigned char *)&umts_prf->dns_addr.secondary_dns_addr.ds_profile_3gpp_pdp_addr_ipv4,
                      profile_node->profile_element.len);

  }
  else if ((ds_profile_3gpp_ip_version_enum_type)umts_prf->dns_addr_secondary_ip_vsn ==  DS_PROFILE_3GPP_IP_V6 ||
            (ds_profile_3gpp_ip_version_enum_type)umts_prf->dns_addr_secondary_ip_vsn ==  DS_PROFILE_3GPP_IP_V4V6)
  {
    DS_PROFILE_LOGD("_3gpp_qmi_write_dns_address_secondary: DS_PROFILE_3GPP_IP_V6", 0);
    profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V6_SECONDARY;
    profile_node->profile_element.len   = sizeof(ipv6_addr_type);

    ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
    tmp_data_ptr = profile_node->profile_element.data;

    WRITE_N_BYTE_VAL(tmp_data_ptr,
                    (unsigned char *)&umts_prf->dns_addr.secondary_dns_addr.ds_profile_3gpp_pdp_addr_ipv6,
                    profile_node->profile_element.len);
  }
  else
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_dns_address_secondary: DS_PROFILE_3GPP_IP_NOTHING", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_umts_req_qos(
  const void           *ptr,
  void                 *node
)
{
  void *tmp_data_ptr = NULL;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  DS_PROFILE_LOGD("_3gpp_qmi_write_umts_req_qos: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_umts_req_qos: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_REQ_QOS;
  profile_node->profile_element.len   = QMI_WDS_UMTS_QOS_SIZE + 1;
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.traffic_class,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.max_ul_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.max_dl_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.gtd_ul_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.gtd_dl_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.dlvry_order,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.max_sdu_size,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.sdu_err,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.res_biterr,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.dlvr_err_sdu,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.trans_delay,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.thandle_prio,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.sig_ind,
                   sizeof(unsigned char));

  return DS_PROFILE_REG_RESULT_SUCCESS;

}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_umts_req_qos_ext(
  const void           *ptr,
  void                 *node
)
{
  void *tmp_data_ptr = NULL;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  DS_PROFILE_LOGD("_3gpp_qmi_write_umts_req_qos_ext: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_umts_req_qos_ext: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_REQ_QOS_EXTENDED;
  profile_node->profile_element.len   = QMI_WDS_UMTS_QOS_SIZE;
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.traffic_class,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.max_ul_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.max_dl_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.gtd_ul_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.gtd_dl_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.dlvry_order,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.max_sdu_size,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.sdu_err,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.res_biterr,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.dlvr_err_sdu,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.trans_delay,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_3gpp.thandle_prio,
                    sizeof(unsigned long));

  return DS_PROFILE_REG_RESULT_SUCCESS;

}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_umts_min_qos(
  const void           *ptr,
  void                 *node
)
{
  void *tmp_data_ptr = NULL;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  DS_PROFILE_LOGD("_3gpp_qmi_write_umts_min_qos: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_umts_min_qos: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_MIN_QOS;
  profile_node->profile_element.len   = QMI_WDS_UMTS_QOS_SIZE + 1;
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.traffic_class,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.max_ul_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.max_dl_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.gtd_ul_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.gtd_dl_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.dlvry_order,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.max_sdu_size,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.sdu_err,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.res_biterr,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.dlvr_err_sdu,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.trans_delay,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.thandle_prio,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.sig_ind,
                   sizeof(unsigned char));

  return DS_PROFILE_REG_RESULT_SUCCESS;

}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_umts_min_qos_ext(
  const void           *ptr,
  void                 *node
)
{
  void *tmp_data_ptr = NULL;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  DS_PROFILE_LOGD("_3gpp_qmi_write_umts_min_qos_ext: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_umts_min_qos_ext: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_MIN_QOS_EXTENDED;
  profile_node->profile_element.len   = QMI_WDS_UMTS_QOS_SIZE;
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.traffic_class,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.max_ul_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.max_dl_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.gtd_ul_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.gtd_dl_bitrate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.dlvry_order,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.max_sdu_size,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.sdu_err,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.res_biterr,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.dlvr_err_sdu,
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.trans_delay,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_3gpp.thandle_prio,
                   sizeof(unsigned long));

  return DS_PROFILE_REG_RESULT_SUCCESS;

}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_gprs_req_qos(
  const void                           *ptr,
  void                                 *node
)
{
  void * tmp_data_ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  DS_PROFILE_LOGD("_3gpp_qmi_write_gprs_req_qos: Entry", 0);

  if ( ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_gprs_req_qos: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_GPRS_REQ_QOS;
  profile_node->profile_element.len   = QMI_WDS_GPRS_QOS_SIZE;

  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);

  tmp_data_ptr = profile_node->profile_element.data;


  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_gprs.precedence,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_gprs.delay,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_gprs.reliability,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_gprs.peak,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_gprs.mean,
                   sizeof(unsigned long));

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_gprs_min_qos(
  const void                          *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;

  DS_PROFILE_LOGD("_3gpp_qmi_write_gprs_min_qos: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_gprs_min_qos: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_GPRS_MIN_QOS;
  profile_node->profile_element.len   = QMI_WDS_GPRS_QOS_SIZE;

  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);

  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_gprs.precedence,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_gprs.delay,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_gprs.reliability,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_gprs.peak,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_minimum_gprs.mean,
                   sizeof(unsigned long));

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_lte_req_qos(
  const void                          *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;

  DS_PROFILE_LOGD("_3gpp_qmi_write_lte_req_qos: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_lte_req_qos: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_LTE_REQ_QOS;
  profile_node->profile_element.len   = QMI_WDS_LTE_QOS_SIZE;

  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);

  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_lte.qci,
                   sizeof(umts_prf->qos_request_lte.qci));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_lte.g_dl_bit_rate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_lte.max_dl_bit_rate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_lte.g_ul_bit_rate,
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->qos_request_lte.max_ul_bit_rate,
                   sizeof(unsigned long));

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_auth_username(
  const void                          *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;

  byte len = (byte)strlen((char *)umts_prf->auth.username);

  DS_PROFILE_LOGD("_3gpp_qmi_write_auth_username: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_auth_username: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_USERNAME;
  profile_node->profile_element.len   = len;
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);

  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)umts_prf->auth.username,
                   len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_auth_password(
  const void                          *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  byte len = (byte)strlen((char *)umts_prf->auth.password);

  DS_PROFILE_LOGD("_3gpp_qmi_write_auth_password: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_auth_password: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_PASSWORD;
  profile_node->profile_element.len   = len;
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);

  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)umts_prf->auth.password,
                   len);

  return DS_PROFILE_REG_RESULT_SUCCESS;

}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_auth_type(
  const void                          *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  DS_PROFILE_LOGD("_3gpp_qmi_write_auth_type: Entry", 0);

  if ( ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_auth_type: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_TYPE;
  profile_node->profile_element.len   = sizeof(unsigned char);

  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->auth.auth_type,
                   profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_pdp_context_pdp_addr(
  const void                          *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  DS_PROFILE_LOGD("_3gpp_qmi_write_pdp_context_pdp_addr: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_pdp_context_pdp_addr: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  if ((ds_profile_3gpp_ip_version_enum_type)umts_prf->pdp_addr_ip_vsn == DS_PROFILE_3GPP_IP_V4 ||
        (ds_profile_3gpp_ip_version_enum_type)umts_prf->pdp_addr_ip_vsn == DS_PROFILE_3GPP_IP_V4V6)
  {
    profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_ADDR_V4;
    profile_node->profile_element.len   = sizeof(ipv4_addr_type);

    ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
    tmp_data_ptr = profile_node->profile_element.data;

    WRITE_N_BYTE_VAL(tmp_data_ptr,
                    (unsigned char *)&umts_prf->context.pdp_addr.ds_profile_3gpp_pdp_addr_ipv4,
                    profile_node->profile_element.len);
  }
  else if ((ds_profile_3gpp_ip_version_enum_type)umts_prf->pdp_addr_ip_vsn == DS_PROFILE_3GPP_IP_V6 ||
             (ds_profile_3gpp_ip_version_enum_type)umts_prf->pdp_addr_ip_vsn == DS_PROFILE_3GPP_IP_V4V6)
  {
    profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_ADDR_V6;
    profile_node->profile_element.len   = sizeof(ipv6_addr_type);

    ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
    tmp_data_ptr = profile_node->profile_element.data;

    WRITE_N_BYTE_VAL(tmp_data_ptr,
                    (unsigned char *)&umts_prf->context.pdp_addr.ds_profile_3gpp_pdp_addr_ipv6,
                    profile_node->profile_element.len);
  }
  else
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_pdp_context_pdp_addr: DS_PROFILE_3GPP_IP_NOTHING", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }


  return DS_PROFILE_REG_RESULT_SUCCESS;

}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_pcscf_req_flag(
  const void                          *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  DS_PROFILE_LOGD("_3gpp_qmi_write_pcscf_req_flag: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_pcscf_req_flag: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_PCSCF_REQ_FLAG;
  profile_node->profile_element.len   = sizeof(unsigned char);

  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);

  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->request_pcscf_address_flag,
                   profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_pdp_context_access_ctrl_flag(
  const void                          *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;

  DS_PROFILE_LOGD("_3gpp_qmi_write_pdp_context_access_ctrl_flag: Entry", 0);
  if (ptr  == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_pdp_context_access_ctrl_flag: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_TE_MT_ACCESS_CTRL_FLAG;
  profile_node->profile_element.len   = sizeof(unsigned char);

  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);

  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->context.access_ctrl_flag,
                   profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_dhcp_req_flag(
  const void                          *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  DS_PROFILE_LOGD("_3gpp_qmi_write_dhcp_req_flag: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_dhcp_req_flag: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_PCSCF_DHCP_REQ_FLAG;
  profile_node->profile_element.len   = sizeof(unsigned char);
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->request_pcscf_address_using_dhcp_flag,
                   profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_im_cn_flag(
   const void                           *ptr,
   void                                *node
)
{
  void * tmp_data_ptr;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  DS_PROFILE_LOGD("_3gpp_qmi_write_im_cn_flag: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_im_cn_flag: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_IM_CN_FLAG;
  profile_node->profile_element.len   = sizeof(unsigned char);
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);

  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->im_cn_flag,
                   profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_tft_filter_id1(
  const void                          *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  qmi_wds_profile_node_type *profile_node =  (qmi_wds_profile_node_type *) node;
  DS_PROFILE_LOGD("_3gpp_qmi_write_tft_filter_id1: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_tft_filter_id1: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_TFT_FILTER_ID1;
  profile_node->profile_element.len   = QMI_WDS_TFT_FILTER_SIZE;
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data ;


  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                  (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].ipsec_spi, 
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].flow_label, 
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].dest_port_range.from, 
                   sizeof(unsigned short));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].dest_port_range.to,
                    sizeof(unsigned short));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].src_port_range.from, 
                   sizeof(unsigned short));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].src_port_range.to, 
                   sizeof(unsigned short));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].tos_mask, 
                   sizeof(unsigned short));

  if ( (ds_profile_3gpp_ip_version_enum_type)umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].ip_version ==  DS_PROFILE_3GPP_IP_V4 )
  {
    unsigned char tmp[ sizeof(ipv6_addr_type ) - sizeof(ipv4_addr_type) ];
    memset(&tmp[0],0,sizeof(ipv6_addr_type ) - sizeof(ipv4_addr_type));
    WRITE_N_BYTE_VAL(tmp_data_ptr, 
                    (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].src_addr.address.ds_profile_3gpp_tft_addr_ipv4, 
                    sizeof(ipv4_addr_type));
    WRITE_N_BYTE_VAL(tmp_data_ptr,
                     tmp,
                     (int)(sizeof(ipv6_addr_type ) - sizeof(ipv4_addr_type)));

  }
  else if ( (ds_profile_3gpp_ip_version_enum_type)umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].ip_version ==  DS_PROFILE_3GPP_IP_V6 )
  {
    WRITE_N_BYTE_VAL(tmp_data_ptr, 
                    (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].src_addr.address.ds_profile_3gpp_tft_addr_ipv6, 
                     sizeof(ipv6_addr_type ));
  }
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].ip_version, 
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].src_addr.mask, 
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].prot_num, 
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].filter_id, 
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1].eval_prec_id, 
                   sizeof(unsigned char));


  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_tft_filter_id2(
  const void           *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  DS_PROFILE_LOGD("_3gpp_qmi_write_tft_filter_id2: Entry", 0);

  if (ptr == NULL ||  profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_tft_filter_id2: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_TFT_FILTER_ID2;
  profile_node->profile_element.len   = QMI_WDS_TFT_FILTER_SIZE;

  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data;


  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].ipsec_spi, 
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].flow_label, 
                   sizeof(unsigned long));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].dest_port_range.from, 
                   sizeof(unsigned short));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].dest_port_range.to, 
                   sizeof(unsigned short));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].src_port_range.from, 
                   sizeof(unsigned short));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].src_port_range.to, 
                   sizeof(unsigned short));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].tos_mask, 
                   sizeof(unsigned short));

  if ( (ds_profile_3gpp_ip_version_enum_type)umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].ip_version ==  DS_PROFILE_3GPP_IP_V4 )
  {
    unsigned char tmp[ sizeof(ipv6_addr_type ) - sizeof(ipv4_addr_type) ];
    memset(&tmp[0],0,sizeof(ipv6_addr_type ) - sizeof(ipv4_addr_type));
    WRITE_N_BYTE_VAL(tmp_data_ptr, 
                    (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].src_addr.address.ds_profile_3gpp_tft_addr_ipv4, 
                    sizeof(ipv4_addr_type));

    WRITE_N_BYTE_VAL(tmp_data_ptr, 
                     tmp,
                     (sizeof(ipv6_addr_type ) - sizeof(ipv4_addr_type)));
  }

  if ( (ds_profile_3gpp_ip_version_enum_type)umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].ip_version ==  DS_PROFILE_3GPP_IP_V6 )
  {
    WRITE_N_BYTE_VAL(tmp_data_ptr, 
                    (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].src_addr.address.ds_profile_3gpp_tft_addr_ipv6, 
                     sizeof(ipv6_addr_type));
  }
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].ip_version, 
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].src_addr.mask, 
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].prot_num, 
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].filter_id, 
                   sizeof(unsigned char));
  WRITE_N_BYTE_VAL(tmp_data_ptr, 
                   (unsigned char *)&umts_prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2].eval_prec_id, 
                   sizeof(unsigned char));

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_pdp_context_number(
  const void                          *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  DS_PROFILE_LOGD("_3gpp_qmi_write_pdp_context_number: Entry", 0);

  if ( ptr == NULL ||  profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_pdp_context_number: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_NUMBER;
  profile_node->profile_element.len   = sizeof(unsigned char);
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->context.pdp_context_number,
                   profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;

}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_pdp_context_secondary_flag(
  const void                          *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  DS_PROFILE_LOGD("_3gpp_qmi_write_pdp_context_secondary_flag: Entry", 0);

  if ( ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_pdp_context_secondary_flag: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_SECONDARY_FLAG;
  profile_node->profile_element.len   = sizeof(unsigned char);
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                   (unsigned char *)&umts_prf->context.secondary_flag,
                   profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}
/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_qmi_write_pdp_context_primary_id(
  const void                          *ptr,
  void                                *node
)
{
  void * tmp_data_ptr;
  qmi_wds_profile_node_type *profile_node = (qmi_wds_profile_node_type *) node;
  ds_profile_3gpp_profile_info_type *umts_prf =  (ds_profile_3gpp_profile_info_type *)ptr;
  DS_PROFILE_LOGD("_3gpp_qmi_write_pdp_context_primary_id: Entry", 0);

  if (ptr == NULL || profile_node == NULL)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write_pdp_context_primary_id: Bad Input received", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  profile_node->profile_element.type  = (unsigned char)DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PRIMARY_ID;
  profile_node->profile_element.len   = sizeof(unsigned char);
  ALLOC_PROFILE_LIST_NODE_DATA(profile_node->profile_element.len);
  tmp_data_ptr = profile_node->profile_element.data;

  WRITE_N_BYTE_VAL(tmp_data_ptr,
                  (unsigned char *)&umts_prf->context.primary_id,
                  profile_node->profile_element.len);

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================
  Table for identifier and its correponding write function  
===========================================================================*/
static dsi_profile_3gpp_write_fn_type ds_profile_3gpp_write_fn_tbl[] = {
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_INVALID,
  dsi_profile_3gpp_qmi_write_invalid},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PROFILE_NAME,
  dsi_profile_3gpp_qmi_write_profile_name},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_TYPE,
  dsi_profile_3gpp_qmi_write_pdp_context_pdp_type},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_H_COMP,
  dsi_profile_3gpp_qmi_write_pdp_context_h_comp},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_D_COMP,
  dsi_profile_3gpp_qmi_write_pdp_context_d_comp},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_APN,
  dsi_profile_3gpp_qmi_write_pdp_context_apn},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V4_PRIMARY,
  dsi_profile_3gpp_qmi_write_dns_addr_primary},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V4_SECONDARY,
  dsi_profile_3gpp_qmi_write_dns_address_secondary},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_REQ_QOS,
  dsi_profile_3gpp_qmi_write_umts_req_qos},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_MIN_QOS,
  dsi_profile_3gpp_qmi_write_umts_min_qos},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_GPRS_REQ_QOS,
  dsi_profile_3gpp_qmi_write_gprs_req_qos},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_GPRS_MIN_QOS,
  dsi_profile_3gpp_qmi_write_gprs_min_qos},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_USERNAME,
  dsi_profile_3gpp_qmi_write_auth_username},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_PASSWORD,
  dsi_profile_3gpp_qmi_write_auth_password}, 

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_TYPE,
  dsi_profile_3gpp_qmi_write_auth_type},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_ADDR_V4,
  dsi_profile_3gpp_qmi_write_pdp_context_pdp_addr}, 

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PCSCF_REQ_FLAG,
  dsi_profile_3gpp_qmi_write_pcscf_req_flag},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_TE_MT_ACCESS_CTRL_FLAG,
  dsi_profile_3gpp_qmi_write_pdp_context_access_ctrl_flag},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PCSCF_DHCP_REQ_FLAG,
  dsi_profile_3gpp_qmi_write_dhcp_req_flag},                      
                          
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_IM_CN_FLAG,
  dsi_profile_3gpp_qmi_write_im_cn_flag},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_TFT_FILTER_ID1,
  dsi_profile_3gpp_qmi_write_tft_filter_id1}, 

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_TFT_FILTER_ID2,
  dsi_profile_3gpp_qmi_write_tft_filter_id2},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_NUMBER,
  dsi_profile_3gpp_qmi_write_pdp_context_number},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_SECONDARY_FLAG,
  dsi_profile_3gpp_qmi_write_pdp_context_secondary_flag},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PRIMARY_ID,
  dsi_profile_3gpp_qmi_write_pdp_context_primary_id},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_ADDR_V6,
  dsi_profile_3gpp_qmi_write_pdp_context_pdp_addr},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_REQ_QOS_EXTENDED,
  dsi_profile_3gpp_qmi_write_umts_req_qos_ext},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_MIN_QOS_EXTENDED,
  dsi_profile_3gpp_qmi_write_umts_min_qos_ext},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V6_PRIMARY,
  dsi_profile_3gpp_qmi_write_dns_addr_primary},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V6_SECONDARY,
  dsi_profile_3gpp_qmi_write_dns_address_secondary},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_IPV4_ADDR_ALLOC,
  dsi_profile_3gpp_qmi_write_pdp_context_ipv4_addr_alloc},

  {(ds_profile_identifier_type)
     DS_PROFILE_3GPP_PROFILE_PARAM_LTE_REQ_QOS,
 dsi_profile_3gpp_qmi_write_lte_req_qos}

};

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_QMI_WRITE

DESCRIPTION
  This function is used to write the profile blob back to modem using the 
  QMI msg library.

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

static ds_profile_status_etype dsi_profile_3gpp_qmi_write(
  ds_profile_num_type   num,
  void                 *ptr
)
{
  dsi_profile_3gpp_type *dsi_3gpp_ptr = NULL;
  uint32 ident = 0;
  uint32 mask = 0;
  uint8 index = 0;
  
  int                         qmi_err_code;
  qmi_wds_profile_id_type     profile_id;
  int                         rc = (int)DS_PROFILE_REG_RESULT_FAIL;
  ds_profile_status_etype  return_status = DS_PROFILE_REG_RESULT_FAIL;
  /*create a linked list head node */
  qmi_wds_profile_node_list_type      profile_list;
  qmi_wds_profile_node_type           *profile_node = NULL;
  list_init(&profile_list);

  DS_PROFILE_LOGD("_3gpp_qmi_write: ENTRY", 0);

  profile_id.technology = QMI_WDS_PROFILE_TECH_3GPP;
  profile_id.profile_index = num;

  dsi_3gpp_ptr = (dsi_profile_3gpp_type *)ptr;

  if (dsi_3gpp_ptr->mask == 0)
  {
    DS_PROFILE_LOGD("_3gpp_qmi_write: Mask is Zero, EXIT with SUCCESS", 0);
    return DS_PROFILE_REG_RESULT_SUCCESS;  
  }

  for ( ident = (uint8)DS_PROFILE_3GPP_PROFILE_PARAM_MIN;
       ident <= (uint8)DS_PROFILE_3GPP_PROFILE_PARAM_MAX;
       ident++ )
  {
    index = dsi_profile_3gpp_get_index_from_ident( ident );
    CONVERT_IDENT_TO_MASK( mask, index );

    if (dsi_3gpp_ptr->mask & mask)
    {
      DS_PROFILE_LOGD("_3gpp_qmi_write: Write function for indentifier %x exists ", (unsigned int)ident);

      profile_node = (qmi_wds_profile_node_type *)malloc (sizeof(qmi_wds_profile_node_type));

      if (!profile_node)
      {
        DS_PROFILE_LOGE("_3gpp_qmi_write: unable to malloc memory for profile node \n",0);
        goto err_label;
      }

      memset (profile_node,0,sizeof(qmi_wds_profile_node_type));

      if ( (return_status = ds_profile_3gpp_write_fn_tbl[index].write_fn((void *)dsi_3gpp_ptr->prf, 
                                                                         profile_node)) != DS_PROFILE_REG_RESULT_SUCCESS)
      {
        //DS_PROFILE_LOGE("_3gpp_qmi_write: write FAIL lib in inconsistent state", 0); 
        free(profile_node);
        profile_node = NULL;
        continue;
        //goto err_label;
      }
      /*If we reach here, a profile node has been successfully created,
        Insert the node into the list Now*/
      list_push_back(&profile_list,(qmi_wds_list_link_type *)profile_node);
      profile_node = NULL;
    }
  }

  /*Write the list to the modem here*/
  if ((rc = qmi_wds_utils_modify_profile(qmi_3gpp_client_handle, 
                                         &profile_id,
                                         &profile_list,
                                         &qmi_err_code)) != QMI_NO_ERR)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_write: Cannot modify profile, rc = %d ", rc);
    DS_PROFILE_LOGE("_3gpp_qmi_write: Cannot modify profile, qmi_err_code = %d", qmi_err_code );
    if (rc == QMI_EXTENDED_ERR)
    {
      qmi_wds_utils_cleanup_list(&profile_list);
      return (ds_profile_status_etype)qmi_err_code;
    }
    goto err_label;
  }    

  /*clean up list here*/
  qmi_wds_utils_cleanup_list(&profile_list);
  DS_PROFILE_LOGD("_3gpp_qmi_write: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;

err_label:
  qmi_wds_utils_cleanup_list(&profile_list);  
  DS_PROFILE_LOGE("_3gpp_qmi_write: EXIT with ERR", 0);  
  return DS_PROFILE_REG_RESULT_FAIL;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_QMI_READ

DESCRIPTION
  This function is used to read a profile from QMI to the local copy

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
static ds_profile_status_etype dsi_profile_3gpp_qmi_read(
  ds_profile_num_type   num,
  void                 *ptr
)
{
  qmi_wds_profile_id_type     profile_id;
  int qmi_err_code, rc = (int)DS_PROFILE_REG_RESULT_FAIL;
  /*create a linked list head node */
  qmi_wds_profile_node_list_type  profile_list;
  qmi_wds_list_link_type          *list_node_link = NULL;
  qmi_wds_profile_node_type       *profile_node = NULL;
  dsi_profile_3gpp_type *dsi_3gpp_ptr = NULL;
  unsigned char  *tmp_data_ptr;
  int i;

  DS_PROFILE_LOGD("_3gpp_qmi_read: ENTRY", 0 );

  list_init(&profile_list);

  profile_id.technology = QMI_WDS_PROFILE_TECH_3GPP;
  profile_id.profile_index = num;

  if ((rc = qmi_wds_utils_query_profile(qmi_3gpp_client_handle, 
                                        &profile_id, 
                                        &profile_list,
                                        &qmi_err_code)) != QMI_NO_ERR)
  {

    DS_PROFILE_LOGE("_3gpp_qmi_read: Query modem for profile information failed rc = %d, ", rc);
    DS_PROFILE_LOGE("_3gpp_qmi_read: Query modem for profile information failed qmi_err_code = %d", qmi_err_code );

    if (rc == QMI_EXTENDED_ERR)
    {
      DS_PROFILE_LOGE("_3gpp_qmi_read: return code = %d", rc);
      DS_PROFILE_LOGE("_3gpp_qmi_read:  extended error code = %d", qmi_err_code);
      return (ds_profile_status_etype)qmi_err_code;
    }
    else
    {
      return DS_PROFILE_REG_RESULT_FAIL;
    }
  }

  /*If we reach here we have a valid list of profile elements*/

  if (list_size(&profile_list) == 0)
  {
     DS_PROFILE_LOGE("_3gpp_qmi_read: List is empty", 0 );
     return DS_PROFILE_REG_RESULT_FAIL;
  }
  else if (list_size(&profile_list) > 0)
  {
    DS_PROFILE_LOGD("_3gpp_qmi_read: List is not-empty list size is %d", list_size(&profile_list) );
    list_node_link = list_peek_front(&profile_list);
  }

  dsi_3gpp_ptr = (dsi_profile_3gpp_type *)ptr;
  dsi_3gpp_ptr->mask = 0;

  memset((void *)dsi_3gpp_ptr->prf, 0, sizeof(ds_profile_3gpp_profile_info_type));

  /*go through the list nodes and populate the 3gpp profile structure.*/
  while(list_node_link != NULL)
  {
    profile_node = (qmi_wds_profile_node_type *)list_node_link;
    tmp_data_ptr = (unsigned char *)profile_node->profile_element.data;
    switch(profile_node->profile_element.type)
    {
      case DS_PROFILE_3GPP_PROFILE_PARAM_PROFILE_NAME:
        {
           if (profile_node->profile_element.len <= DS_PROFILE_3GPP_MAX_PROFILE_NAME_LEN )
           {
              READ_N_BYTE_VAL(tmp_data_ptr,
                             (unsigned char *)dsi_3gpp_ptr->prf->profile_name,
                             profile_node->profile_element.len);
              dsi_3gpp_ptr->prf->profile_name[profile_node->profile_element.len] = '\0';
           }
           else
           {
             DS_PROFILE_LOGE("_3gpp_qmi_read: Size not enough to copy profile name", 0 );
           }
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_TYPE:
        {
             dsi_3gpp_ptr->prf->ds_profile_3gpp_context_valid_flg = TRUE;
             READ_N_BYTE_VAL(tmp_data_ptr,
                             (unsigned char *)&dsi_3gpp_ptr->prf->context.pdp_type,
                             profile_node->profile_element.len);
        }
        break; 

      case DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_APN:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_context_valid_flg =  TRUE;
          if (profile_node->profile_element.len <= DS_PROFILE_3GPP_MAX_APN_STRING_LEN)
          {
            READ_N_BYTE_VAL(tmp_data_ptr,
                            (unsigned char *)dsi_3gpp_ptr->prf->context.apn,
                            profile_node->profile_element.len);
             dsi_3gpp_ptr->prf->context.apn[profile_node->profile_element.len] = '\0';
          }
           else
           {
             DS_PROFILE_LOGE("_3gpp_qmi_read: Size not enough to copy profile APN", 0 );
           }
        }
        break; 

      case DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V4_PRIMARY:
        {
          if (dsi_3gpp_ptr->prf->dns_addr_primary_ip_vsn == DS_PROFILE_3GPP_IP_V6)
          {
            dsi_3gpp_ptr->prf->dns_addr_primary_ip_vsn = DS_PROFILE_3GPP_IP_V4V6;
          }
          else
          {
            dsi_3gpp_ptr->prf->dns_addr_primary_ip_vsn = DS_PROFILE_3GPP_IP_V4;
          }
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->dns_addr.primary_dns_addr.ds_profile_3gpp_pdp_addr_ipv4,
                          profile_node->profile_element.len);
        }
        break; 

      case DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V4_SECONDARY:
        {
          if (dsi_3gpp_ptr->prf->dns_addr_secondary_ip_vsn == DS_PROFILE_3GPP_IP_V6)
          {
            dsi_3gpp_ptr->prf->dns_addr_secondary_ip_vsn = DS_PROFILE_3GPP_IP_V4V6;
          }
          else
          {
            dsi_3gpp_ptr->prf->dns_addr_secondary_ip_vsn = DS_PROFILE_3GPP_IP_V4;
          }
          READ_N_BYTE_VAL(tmp_data_ptr,
                         (unsigned char *)&dsi_3gpp_ptr->prf->dns_addr.secondary_dns_addr.ds_profile_3gpp_pdp_addr_ipv4,
                          profile_node->profile_element.len);
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_REQ_QOS:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_qos_req_3gpp_valid_flg = TRUE;
          tmp_data_ptr = (unsigned char *)profile_node->profile_element.data;
          READ_N_BYTE_VAL( tmp_data_ptr,
                           (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.traffic_class,
                            sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                         (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.max_ul_bitrate,
                         sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                         (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.max_dl_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.gtd_ul_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.gtd_dl_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.dlvry_order,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.max_sdu_size,
                           sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.sdu_err,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)& dsi_3gpp_ptr->prf->qos_request_3gpp.res_biterr,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.dlvr_err_sdu,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.trans_delay,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.thandle_prio,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)& dsi_3gpp_ptr->prf->qos_request_3gpp.sig_ind,
                          sizeof(unsigned char));
        }
        break;

     case DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_MIN_QOS:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_qos_min_3gpp_valid_flg = TRUE;
          tmp_data_ptr = (unsigned char *)profile_node->profile_element.data;
          READ_N_BYTE_VAL( tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.traffic_class ,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.max_ul_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.max_dl_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.gtd_ul_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.gtd_dl_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.dlvry_order,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.max_sdu_size,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.sdu_err,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)& dsi_3gpp_ptr->prf->qos_minimum_3gpp.res_biterr,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.dlvr_err_sdu,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.trans_delay,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.thandle_prio,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)& dsi_3gpp_ptr->prf->qos_minimum_3gpp.sig_ind,
                          sizeof(unsigned char));
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_GPRS_REQ_QOS:
        {
           dsi_3gpp_ptr->prf->ds_profile_3gpp_qos_req_gprs_valid_flg = TRUE;
          tmp_data_ptr = (unsigned char *)profile_node->profile_element.data;
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_gprs.precedence,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_gprs.delay,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_gprs.reliability,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_gprs.peak,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_gprs.mean,
                          sizeof(unsigned long));
        }
        break; 

      case DS_PROFILE_3GPP_PROFILE_PARAM_GPRS_MIN_QOS:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_qos_min_gprs_valid_flg = TRUE;
          tmp_data_ptr = (unsigned char *)profile_node->profile_element.data;
          READ_N_BYTE_VAL( tmp_data_ptr,
                         (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_gprs.precedence ,
                         sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                         (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_gprs.delay ,
                         sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                         (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_gprs.reliability,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_gprs.peak,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_gprs.mean,
                          sizeof(unsigned long));
        }
        break;

        case DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_USERNAME:
        {
          if (profile_node->profile_element.len <= DS_PROFILE_3GPP_MAX_QCPDP_STRING_LEN)
           {
             READ_N_BYTE_VAL(tmp_data_ptr,
                             (unsigned char *)dsi_3gpp_ptr->prf->auth.username,
                             profile_node->profile_element.len);
             dsi_3gpp_ptr->prf->auth.username[profile_node->profile_element.len] = '\0';
           }
           else
           {
             DS_PROFILE_LOGE("_3gpp_qmi_read: Size not enough to copy profile username", 0 );
           }
        }
        break;

        case DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_PASSWORD:
        {
          if (profile_node->profile_element.len <= DS_PROFILE_3GPP_MAX_QCPDP_STRING_LEN)
           {
             READ_N_BYTE_VAL(tmp_data_ptr,
                             (unsigned char *)dsi_3gpp_ptr->prf->auth.password,
                             profile_node->profile_element.len);
             dsi_3gpp_ptr->prf->auth.password[DS_PROFILE_3GPP_MAX_QCPDP_STRING_LEN] = '\0';
           }
           else
           {
             DS_PROFILE_LOGE("_3gpp_qmi_read: Size not enough to copy profile password", 0 );
           }
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_TYPE:
        {
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->auth.auth_type,
                           profile_node->profile_element.len);
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_ADDR_V4:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_context_valid_flg =  TRUE;
          if (dsi_3gpp_ptr->prf->pdp_addr_ip_vsn == DS_PROFILE_3GPP_IP_V6)
          {
            dsi_3gpp_ptr->prf->pdp_addr_ip_vsn = DS_PROFILE_3GPP_IP_V4V6;
          }
          else
          {
            dsi_3gpp_ptr->prf->pdp_addr_ip_vsn = DS_PROFILE_3GPP_IP_V4;
          }
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->context.pdp_addr.ds_profile_3gpp_pdp_addr_ipv4,
                          profile_node->profile_element.len);
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_PCSCF_REQ_FLAG:
        {
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->request_pcscf_address_flag,
                          profile_node->profile_element.len);
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_H_COMP:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_context_valid_flg =  TRUE;
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->context.h_comp,
                          profile_node->profile_element.len);
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_D_COMP:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_context_valid_flg =  TRUE;
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->context.d_comp,
                           profile_node->profile_element.len);
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_TE_MT_ACCESS_CTRL_FLAG:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_context_valid_flg =  TRUE;
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->context.access_ctrl_flag,
                          profile_node->profile_element.len);
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_PCSCF_DHCP_REQ_FLAG:
        {
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->request_pcscf_address_using_dhcp_flag,
                          profile_node->profile_element.len);
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_IM_CN_FLAG:
        {
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->im_cn_flag,
                          profile_node->profile_element.len);
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_TFT_FILTER_ID1:
      case DS_PROFILE_3GPP_PROFILE_PARAM_TFT_FILTER_ID2:
        {
          tmp_data_ptr = (unsigned char *)profile_node->profile_element.data;
          i = 0;

          READ_N_BYTE_VAL(tmp_data_ptr,
                          &i,
                          sizeof(unsigned char));
          i -= 1;
          dsi_3gpp_ptr->prf->ds_profile_3gpp_tft_valid_flg[i] = TRUE;
          dsi_3gpp_ptr->prf->tft[i].filter_id = i;

          READ_N_BYTE_VAL(tmp_data_ptr,
                          &dsi_3gpp_ptr->prf->tft[i].eval_prec_id,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          &dsi_3gpp_ptr->prf->tft[i].ip_version,
                          sizeof(unsigned char));
          if ( dsi_3gpp_ptr->prf->tft[i].ip_version == DS_PROFILE_3GPP_IP_V4 )
          {
            READ_N_BYTE_VAL(tmp_data_ptr,
                          &dsi_3gpp_ptr->prf->tft[i].src_addr.address.ds_profile_3gpp_tft_addr_ipv4,
                          sizeof(ipv4_addr_type));

            tmp_data_ptr += sizeof(ipv6_addr_type) - sizeof(ipv4_addr_type);

          }
          if ( dsi_3gpp_ptr->prf->tft[i].ip_version == DS_PROFILE_3GPP_IP_V6 )
          {
            READ_N_BYTE_VAL(tmp_data_ptr,
                          &dsi_3gpp_ptr->prf->tft[i].src_addr.address.ds_profile_3gpp_tft_addr_ipv6,
                          sizeof(ipv6_addr_type));

          }

          READ_N_BYTE_VAL(tmp_data_ptr,
                          &dsi_3gpp_ptr->prf->tft[i].prot_num,
                          sizeof(unsigned char));

          READ_N_BYTE_VAL(tmp_data_ptr,
                          &dsi_3gpp_ptr->prf->tft[i].dest_port_range.from,
                          sizeof(unsigned short));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          &dsi_3gpp_ptr->prf->tft[i].dest_port_range.to,
                          sizeof(unsigned short));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          &dsi_3gpp_ptr->prf->tft[i].src_port_range.from,
                          sizeof(unsigned short));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          &dsi_3gpp_ptr->prf->tft[i].src_port_range.to,
                          sizeof(unsigned short));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          &dsi_3gpp_ptr->prf->tft[i].ipsec_spi,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          &dsi_3gpp_ptr->prf->tft[i].tos_mask,
                          sizeof(unsigned short));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          &dsi_3gpp_ptr->prf->tft[i].flow_label,
                          sizeof(unsigned long));
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_NUMBER:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_context_valid_flg =  TRUE;
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->context.pdp_context_number,
                          profile_node->profile_element.len);
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_SECONDARY_FLAG:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_context_valid_flg =  TRUE;
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->context.secondary_flag,
                          profile_node->profile_element.len);
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PRIMARY_ID:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_context_valid_flg =  TRUE;
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->context.primary_id,
                          profile_node->profile_element.len);
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_ADDR_V6:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_context_valid_flg =  TRUE;
          if (dsi_3gpp_ptr->prf->pdp_addr_ip_vsn == DS_PROFILE_3GPP_IP_V4)
          {
            dsi_3gpp_ptr->prf->pdp_addr_ip_vsn = DS_PROFILE_3GPP_IP_V4V6;
          }
          else
          {
            dsi_3gpp_ptr->prf->pdp_addr_ip_vsn = DS_PROFILE_3GPP_IP_V6;
          }
          if (profile_node->profile_element.len == sizeof(ipv6_addr_type))
          {
            READ_N_BYTE_VAL(tmp_data_ptr,
                            (unsigned char *)&dsi_3gpp_ptr->prf->context.pdp_addr.ds_profile_3gpp_pdp_addr_ipv6,
                            profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp_qmi_read: Invalid ipv6 address received. Length mismatch", 0 );
          }
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V6_PRIMARY:
        {
          if (dsi_3gpp_ptr->prf->dns_addr_primary_ip_vsn == DS_PROFILE_3GPP_IP_V4)
          {
            dsi_3gpp_ptr->prf->dns_addr_primary_ip_vsn = DS_PROFILE_3GPP_IP_V4V6;
          }
          else
          {
            dsi_3gpp_ptr->prf->dns_addr_primary_ip_vsn = DS_PROFILE_3GPP_IP_V6;
          }
          if (profile_node->profile_element.len == sizeof(ipv6_addr_type))
          {
            READ_N_BYTE_VAL(tmp_data_ptr,
                            (unsigned char *)&dsi_3gpp_ptr->prf->dns_addr.primary_dns_addr.ds_profile_3gpp_pdp_addr_ipv6,
                            profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp_qmi_read: Invalid ipv6 address received. Length mismatch", 0 );
          }
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V6_SECONDARY:
        {
          if (dsi_3gpp_ptr->prf->dns_addr_secondary_ip_vsn == DS_PROFILE_3GPP_IP_V4)
          {
            dsi_3gpp_ptr->prf->dns_addr_secondary_ip_vsn = DS_PROFILE_3GPP_IP_V4V6;
          }
          else
          {
            dsi_3gpp_ptr->prf->dns_addr_secondary_ip_vsn = DS_PROFILE_3GPP_IP_V6;
          }

          if (profile_node->profile_element.len == sizeof(ipv6_addr_type))
          {
            READ_N_BYTE_VAL(tmp_data_ptr,
                            (unsigned char *)&dsi_3gpp_ptr->prf->dns_addr.secondary_dns_addr.ds_profile_3gpp_pdp_addr_ipv6,
                            profile_node->profile_element.len);
          }
          else
          {
            DS_PROFILE_LOGE("_3gpp_qmi_read: Invalid ipv6 address received. Length mismatch", 0 );
          }
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_IPV4_ADDR_ALLOC:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_context_valid_flg =  TRUE;

            READ_N_BYTE_VAL(tmp_data_ptr,
                            (unsigned char *)&dsi_3gpp_ptr->prf->context.ipv4_addr_alloc,
                            profile_node->profile_element.len);
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_LTE_REQ_QOS:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_qos_req_lte_valid_flg =  TRUE;
          tmp_data_ptr = (unsigned char *)profile_node->profile_element.data;

          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_lte.qci,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_lte.g_dl_bit_rate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_lte.max_dl_bit_rate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_lte.g_ul_bit_rate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_lte.max_ul_bit_rate,
                          sizeof(unsigned long));
        }
        break;

      case DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_REQ_QOS_EXTENDED:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_qos_req_3gpp_valid_flg = TRUE;
          tmp_data_ptr = (unsigned char *)profile_node->profile_element.data;
          READ_N_BYTE_VAL( tmp_data_ptr,
                           (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.traffic_class,
                            sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                         (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.max_ul_bitrate,
                         sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                         (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.max_dl_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.gtd_ul_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.gtd_dl_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.dlvry_order,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.max_sdu_size,
                           sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.sdu_err,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)& dsi_3gpp_ptr->prf->qos_request_3gpp.res_biterr,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.dlvr_err_sdu,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.trans_delay,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.thandle_prio,
                          sizeof(unsigned long));
        }
        break;

     case DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_MIN_QOS_EXTENDED:
        {
          dsi_3gpp_ptr->prf->ds_profile_3gpp_qos_min_3gpp_valid_flg = TRUE;
          tmp_data_ptr = (unsigned char *)profile_node->profile_element.data;
          READ_N_BYTE_VAL( tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_request_3gpp.traffic_class ,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.max_ul_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.max_dl_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.gtd_ul_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.gtd_dl_bitrate,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.dlvry_order,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.max_sdu_size,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.sdu_err,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)& dsi_3gpp_ptr->prf->qos_minimum_3gpp.res_biterr,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.dlvr_err_sdu,
                          sizeof(unsigned char));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.trans_delay,
                          sizeof(unsigned long));
          READ_N_BYTE_VAL(tmp_data_ptr,
                          (unsigned char *)&dsi_3gpp_ptr->prf->qos_minimum_3gpp.thandle_prio,
                          sizeof(unsigned long));
        }
        break;

      default:
         DS_PROFILE_LOGE("_3gpp_qmi_read: Invalid profile element received %x", profile_node->profile_element.type );
    }/*Switch*/

    list_node_link =  (qmi_wds_list_link_type *) list_peek_next(&profile_list, list_node_link);
    
  }/*While*/

  /*Successfully copied the profile from modem to local packed struct*/
  /*clean up the list nodes and the list here!*/

  qmi_wds_utils_cleanup_list(&profile_list);
  DS_PROFILE_LOGD("_3gpp_qmi_read: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS; 
}
/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_QMI_CREATE_LIST

DESCRIPTION
  This function is used to in the get_list function to create a list

PARAMETERS 
  hndl : list handle
  mn : minimum profile number
  mx : maximum profile number
  
DEPENDENCIES 
  
RETURN VALUE 
  DSI_SUCCESS
  DSI_FAILURE
  
SIDE EFFECTS 
  
===========================================================================*/
static int dsi_profile_3gpp_qmi_create_list( 
  ds_util_list_hndl_type          hndl,
  uint16                          mn,
  uint16                          mx
)
{
  int rc, qmi_err_code;
  qmi_wds_profile_list_type profile_list [DSI_PROFILE_CREATE_LIST_PROFILE_LIST_SIZE];
  int profile_list_size = DSI_PROFILE_CREATE_LIST_PROFILE_LIST_SIZE;
  ds_profile_3gpp_list_info_type  node;
  uint32 info_size = DS_FALSE;

  (void)mn; (void)mx;
  memset( (void *)&node, 0, sizeof( ds_profile_3gpp_list_info_type ) );

  if ((rc = qmi_wds_utils_get_profile_list (qmi_3gpp_client_handle,
                                            NULL,
                                            NULL,
                                            profile_list,
                                            &profile_list_size,
                                            &qmi_err_code)) != QMI_NO_ERR)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_create_list: Unable to query for profile list", 0);
    if (rc == QMI_EXTENDED_ERR)
    {
      DS_PROFILE_LOGE("_3gpp_qmi_create_list: return code = %d", rc);
      DS_PROFILE_LOGE("_3gpp_qmi_create_list: extended error code = %d",qmi_err_code );
      return qmi_err_code;
    }
    return DSI_FAILURE;
  }
  else
  {
    int i;
    int len;
    for (i=0; i < profile_list_size; i++)
    {
      node.prf_num = (ds_profile_num_type) profile_list[i].profile_index;

      len = MIN( (sizeof(node.prf_name)-1), strlen(profile_list[i].profile_name) );
      memcpy( (void *)node.prf_name,
              (void *)profile_list[i].profile_name,
              len);
      node.prf_name[len] = '\0';

      info_size = sizeof( ds_profile_3gpp_list_info_type );
      if ( ds_util_list_add( hndl, &node, info_size) != DS_SUCCESS)
      {
        DS_PROFILE_LOGE("_3gpp_qmi_create_list: unable to add node to list", 0);
        return DSI_FAILURE;
      }
    }
  }
  return DSI_SUCCESS;
}
/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_GET_LIST

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
static ds_profile_status_etype dsi_profile_3gpp_get_list(
  ds_util_list_hndl_type hndl,
  ds_profile_list_type  *lst
)
{
  short mn = 0, mx = 0;

  /* Input parameter validation */
  switch (lst->dfn)
  {
    case DS_PROFILE_LIST_ALL_PROFILES:
      dsi_profile_get_profile_num_range( DS_PROFILE_TECH_3GPP, (uint16 *)&mn,(uint16 *)&mx );
      if ( dsi_profile_3gpp_qmi_create_list( hndl, mn, mx ) != DSI_SUCCESS )
        return DS_PROFILE_REG_RESULT_FAIL;
    break;

    default:
      DS_PROFILE_LOGE("_3gpp_get_list: list dfn [%d] not impl", lst->dfn );
      return DS_PROFILE_REG_RESULT_ERR_INVAL_OP;
  }

  return DS_PROFILE_REG_RESULT_SUCCESS;
} 
/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_GET_LIST_NODE

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
ds_profile_status_etype dsi_profile_3gpp_get_list_node(
  ds_util_itr_hndl_type       hndl,
  ds_profile_list_info_type  *list_info
)
{
  ds_profile_3gpp_list_info_type node;
  uint32 info_size = sizeof(ds_profile_3gpp_list_info_type);

  if ( ds_util_itr_get_data(hndl, (void *)&node, &info_size) != DS_SUCCESS)
  {
    DS_PROFILE_LOGE("_3gpp_get_list_node: unable to get node from list", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  if ( (list_info->name != NULL) && 
       (list_info->name->len >= strlen( node.prf_name )) && 
       (list_info->name->buf != NULL) )
  {
    list_info->num = node.prf_num;
    list_info->name->len = strlen( node.prf_name );
    memcpy( (void *)list_info->name->buf, 
            (void *)node.prf_name, 
            list_info->name->len);
  }
  else
  {
    return DS_PROFILE_REG_RESULT_ERR_INVAL;
  }
   
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_CREATE

DESCRIPTION
  This function is used to mark a free profile as valid for use. Creates a
  new profile.

PARAMETERS 
  num : profile number

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp_create(
  ds_profile_num_type  *num
)
{
  qmi_wds_profile_id_type profile_id;
  int qmi_err_code;
  int rc = (int)DS_PROFILE_REG_RESULT_FAIL;

  profile_id.technology = QMI_WDS_PROFILE_TECH_3GPP;

  if ((rc = qmi_wds_create_profile(qmi_3gpp_client_handle, 
                                   &profile_id,
                                   NULL,
                                   &qmi_err_code)) != QMI_NO_ERR)
  {
    DS_PROFILE_LOGE("dsi_profile_3gpp_create: Cannot create profile ", 0);
    DS_PROFILE_LOGE("dsi_profile_3gpp_create: EXIT with ERR ", 0);
    if (rc == QMI_EXTENDED_ERR)
    {
      DS_PROFILE_LOGE("dsi_profile_3gpp_create: QMI returned with "
                                      "extended error code %d ",qmi_err_code);
      return (ds_profile_status_etype)qmi_err_code;
    }
    else
    {
      return DS_PROFILE_REG_RESULT_FAIL;
    }
  }
  *num = (ds_profile_num_type)profile_id.profile_index;
  return DS_PROFILE_REG_RESULT_SUCCESS;
}


/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_SET_DEFAULT_PROFILE

DESCRIPTION
  This function is used to set default profile number for a particular
  family

PARAMETERS 
  family : type of profile (socket, rmnet, atcop)
  num : profile number

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/
ds_profile_status_etype dsi_profile_3gpp_set_default_profile(  
  uint32                 family, 
  ds_profile_num_type    num
)
{
  unsigned char     profile_idx = (unsigned char)num ;
  int               qmi_err_code;
  int               rc = (int)DS_PROFILE_REG_RESULT_FAIL;

  qmi_wds_profile_tech_type         tech = QMI_WDS_PROFILE_TECH_3GPP;
  qmi_wds_profile_family            profile_family = (qmi_wds_profile_family)family;

  switch (profile_family)
  {
    /* RMNET */
    case QMI_WDS_PROFILE_SOCKETS_FAMILY:
      {
        if ((rc = qmi_wds_set_default_profile_number(qmi_3gpp_client_handle, 
                                                     tech, 
                                                     profile_family,
                                                     profile_idx, 
                                                     &qmi_err_code)) != QMI_NO_ERR)
        {
          DS_PROFILE_LOGE("_3gpp_set_default_profile: EXIT with ERR ", 0);

          if (rc == QMI_EXTENDED_ERR)
          {
            return (ds_profile_status_etype)qmi_err_code;
          }
          else
          {
            return DS_PROFILE_REG_RESULT_FAIL;
          }
        }
      }
      break;

    default:
      {
        DS_PROFILE_LOGE("_3gpp_set_default_profile: Profile family not supported. ", 0);
        return DS_PROFILE_REG_3GPP_INVAL_PROFILE_FAMILY;
      }
  }
  return DS_PROFILE_REG_RESULT_SUCCESS; 
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_GET_DEFAULT_PROFILE

DESCRIPTION
  This function is used to get default profile number for a particular
  family

PARAMETERS 
  family : type of profile (socket, rmnet, atcop)
  num : profile number

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/
ds_profile_status_etype dsi_profile_3gpp_get_default_profile(
  uint32                 family, 
  ds_profile_num_type   *num
)
{
  unsigned char default_profile_idx;
  int qmi_err_code, rc = (int)DS_PROFILE_REG_RESULT_FAIL;

  qmi_wds_profile_tech_type       tech = QMI_WDS_PROFILE_TECH_3GPP;
  qmi_wds_profile_family          profile_family = (qmi_wds_profile_family)family;

  switch (profile_family)
  {
    /* RMNET */
    case QMI_WDS_PROFILE_SOCKETS_FAMILY:
      {
        if ((rc = qmi_wds_get_default_profile_number(qmi_3gpp_client_handle, 
                                                     tech,
                                                     profile_family,
                                                     &default_profile_idx, 
                                                     &qmi_err_code)) != QMI_NO_ERR)
        {
          DS_PROFILE_LOGE("_3gpp_get_default_profile: EXIT with ERR ", 0);
          
          if (rc == QMI_EXTENDED_ERR)
          {
            return (ds_profile_status_etype)qmi_err_code;
          }  
          else
          {
            return DS_PROFILE_REG_RESULT_FAIL;
          }
        }
      }
      *num = default_profile_idx;
      break;

    default:
      {
        *num = 0;
        DS_PROFILE_LOGE("_3gpp_get_default_profile: Profile family not supported. ", 0);
        return DS_PROFILE_REG_3GPP_INVAL_PROFILE_FAMILY;
      }
  }
  return DS_PROFILE_REG_RESULT_SUCCESS; 
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_DEL

DESCRIPTION
  This function is used to mark profile as free, resets the profile to
  undefined.

PARAMETERS 
  num : profile number

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp_del(
  ds_profile_num_type  num 
)
{
  qmi_wds_profile_id_type profile_idx;
  int qmi_err_code;
  int rc = (int)DS_PROFILE_REG_RESULT_FAIL;
  ds_profile_num_type rmnet_def_num = (ds_profile_num_type)DS_INVALID;

  /* check if num is the default profile number, if yes delete not allowed */
  if (dsi_profile_3gpp_get_default_profile ( DS_PROFILE_3GPP_RMNET_PROFILE_FAMILY,
                                             &rmnet_def_num) != DS_PROFILE_REG_RESULT_SUCCESS)
  {
    DS_PROFILE_LOGE("_3gpp_del: EXIT with ERR, cannot get the default profile", 0);
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  if (num == rmnet_def_num)
  {
     DS_PROFILE_LOGE("_3gpp_del: EXIT with ERR, cannot delete default profile", 0);
     return DS_PROFILE_REG_RESULT_FAIL;
  }
  profile_idx.technology = QMI_WDS_PROFILE_TECH_3GPP;
  profile_idx.profile_index = num;

  /*Now delete the profile*/
  if ((rc = qmi_wds_delete_profile(qmi_3gpp_client_handle, 
                                   &profile_idx,
                                   &qmi_err_code)) != QMI_NO_ERR)
  {
     DS_PROFILE_LOGE("_3gpp_del: EXIT with ERR, cannot delete profile", 0);

     if (rc == QMI_EXTENDED_ERR)
     {
       return (ds_profile_status_etype)qmi_err_code;
     }
     else
     {
       return DS_PROFILE_REG_RESULT_FAIL;
     }
  }
  return DS_PROFILE_REG_RESULT_SUCCESS;
}
/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_QMI_RESET_PARAM

DESCRIPTION
  This function is used to reset a particular identifier value to default.
  Valid for particular identifiers, no-op for others.

PARAMETERS 
  num : profile number
  ident : identifier for the paramter

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/
ds_profile_status_etype dsi_profile_3gpp_qmi_reset_param(
  ds_profile_num_type         num,
  ds_profile_identifier_type  ident
)
{
  int qmi_err_code;
  int rc = (int)DS_PROFILE_REG_RESULT_FAIL;
  unsigned char profile_idx = (unsigned char)num;
  qmi_wds_profile_tech_type tech = (qmi_wds_profile_tech_type)QMI_WDS_PROFILE_TECH_3GPP;

  qmi_wds_reset_profile_param_type  profile_param_id = (qmi_wds_reset_profile_param_type)ident;

  if ((rc = qmi_wds_reset_profile_param_invalid(qmi_3gpp_client_handle, 
                                                tech,
                                                profile_idx,
                                                profile_param_id,
                                                &qmi_err_code)) != QMI_NO_ERR)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_reset_param: EXIT with ERR ", 0);

    if (rc == QMI_EXTENDED_ERR)
    {
      return (ds_profile_status_etype)qmi_err_code;
    }
    else
    {
      return DS_PROFILE_REG_RESULT_FAIL;
    }
  }
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_QMI_RESET_PROFILE_TO_DEFAULT

DESCRIPTION
  This function is used to reset profile to default values.


PARAMETERS 
  num : profile number

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/

ds_profile_status_etype dsi_profile_3gpp_qmi_reset_profile_to_default(  
  ds_profile_num_type    num
)
{ 
  int qmi_err_code;
  int rc = (int)DS_PROFILE_REG_RESULT_FAIL;
  qmi_wds_profile_tech_type tech = QMI_WDS_PROFILE_TECH_3GPP;
  unsigned char profile_idx = (unsigned char)num;

  if ((rc = qmi_wds_reset_profile_to_default(qmi_3gpp_client_handle, 
                                             tech,
                                             profile_idx, 
                                             &qmi_err_code)) != QMI_NO_ERR)
  {
    DS_PROFILE_LOGE("_reset_profile_to_default: EXIT with ERR ", 0);

    if (rc == QMI_EXTENDED_ERR)
    {
      return (ds_profile_status_etype)qmi_err_code;
    }
    else
    {
      return DS_PROFILE_REG_RESULT_FAIL;
    }
  }
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_VALIDATE_PROFILE_NUM

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
ds_profile_status_etype dsi_profile_3gpp_validate_profile_num(
  ds_profile_num_type num
)
{
  uint16 min_num = 0;
  uint16 max_num = 0;

  /* check profile number */
  dsi_profile_get_profile_num_range(DS_PROFILE_TECH_3GPP, 
                                    &min_num, 
                                    &max_num);

  if ( ( num > max_num ) || ( num < min_num ) )
  {
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM;
  }

  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_GET_NUM_RANGE

DESCRIPTION
  This function is used to get the range of 3GPP profile numbers

PARAMETERS 
  min_num, max_num : pointers to store range (min & max profile numbers)

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  
===========================================================================*/
void dsi_profile_3gpp_get_num_range(
  uint16 *min_num,
  uint16 *max_num
)
{
  *min_num = DS_PROFILE_3GPP_PROFILE_NUM_MIN;
  *max_num = DS_UMTS_MAX_PDP_PROFILE_NUM;
  return;
}
/*===========================================================================
FUNCTION DS_PROFILE_3GPP_QMI_INIT

DESCRIPTION
  This function is called on the library init. It initializes the function
  pointers to valid functions for 3gpp
  Also initializes the QMI client.
PARAMETERS 
  fntbl : pointer to table of function pointers

DEPENDENCIES 
  
RETURN VALUE 
  returns the mask for 3gpp. (Used later as valid mask which is ORed value
  returned from all techs.
SIDE EFFECTS 
  
===========================================================================*/
uint8 ds_profile_3gpp_qmi_init( tech_fntbl_type *fntbl )
{
  int qmi_err;

  DS_PROFILE_LOGD("_3gpp_qmi_init: ENTRY", 0);
  /* Init function pointers */
  fntbl->create         = dsi_profile_3gpp_create;
  fntbl->del            = dsi_profile_3gpp_del;
  fntbl->profile_read   = dsi_profile_3gpp_qmi_read;
  fntbl->profile_write  = dsi_profile_3gpp_qmi_write;
  fntbl->validate_profile_num = dsi_profile_3gpp_validate_profile_num;
  fntbl->get_list = dsi_profile_3gpp_get_list;
  fntbl->get_list_node = dsi_profile_3gpp_get_list_node;
  fntbl->reset_param    = dsi_profile_3gpp_qmi_reset_param;
  fntbl->reset_profile_to_default = dsi_profile_3gpp_qmi_reset_profile_to_default;
  fntbl->set_default_profile = dsi_profile_3gpp_set_default_profile;
  fntbl->get_default_profile = dsi_profile_3gpp_get_default_profile;
  fntbl->get_num_range = dsi_profile_3gpp_get_num_range;

  qmi_handle = qmi_init(NULL, NULL);
  if (qmi_handle < 0)
  {
    DS_PROFILE_LOGE("_3gpp_qmi_init: QMI message library init failed ", 0);
    return DS_FALSE;

  }
  /* Initialize qmi connection */
  if (qmi_connection_init(QMI_PORT_RMNET_1, &qmi_err) < 0) 
  {
    DS_PROFILE_LOGE("_3gpp_qmi_init: QMI connection init failed ", 0);
    return DS_FALSE;
  }
  /* Initialize WDS client */
  if ((qmi_3gpp_client_handle = qmi_wds_srvc_init_client(QMI_PORT_RMNET_1, 
                                                         NULL, 
                                                         NULL, 
                                                         &qmi_err)) < 0) 
  {
    DS_PROFILE_LOGE("_3gpp_qmi_init: wds client init failed ", 0);
    return DS_FALSE;
  }
  /* Register process exit cleanup handler */
  atexit(qmi_cleanup_3gpp);
  DS_PROFILE_LOGD("_3gpp_qmi_init: EXIT with SUCCESS", 0);
  
  return (0x01 << (uint8)DS_PROFILE_TECH_3GPP);
}

#endif /* FEATURE_DS_PROFILE_ACCESS_QMI */
