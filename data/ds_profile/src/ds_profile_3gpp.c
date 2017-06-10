/******************************************************************************
  @file    ds_profile_3gpp.c
  @brief   

  DESCRIPTION
  Tech specific implementation of 3GPP Profile Management  

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  N/A

  ---------------------------------------------------------------------------
  Copyright (C) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/modem/data/umts/dsprofile3gpp/main/latest/src/ds_profile_3gpp.c#4 $ $DateTime: 2009/10/26 12:52:40 $ $Author: mghotge $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/15/09   vk      Added support for LTE params
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/
#include "ds_profile_3gppi.h"
#include "ds_profile_os.h"

/*---------------------------------------------------------------------------
                       UTILITY MACROS
---------------------------------------------------------------------------*/

/*lint -save -e641*/

/* Macro to check info (for get_param function)
  (info->buf not NULL and validate info->len) */
#define GET_INFO_IS_VALID( info, max_len ) \
  ( (info->buf != NULL) && (info->len >= max_len) )

/* Macro to check info (for set_param function)
  (info->buf not NULL and validate info->len) */
#define SET_INFO_IS_VALID( info, max_len ) \
  ( (info->buf != NULL) && ( (info->len > 0) && (info->len <= max_len) ) )


/*---------------------------------------------------------------------------
              3GPP PARAMS ACCCESSOR/MUTATOR ROUTINES & UTILS
---------------------------------------------------------------------------*/

/*===========================================================================
FUNCTION dsi_profile_set_ group of functions

DESCRIPTION
  This set of accessor functions are used to set corresponding Profile 
  parameter

PARAMETERS
  profile : ptr to 3GPP Profile blob 
  mask    : mask to identify Profile parameter
  info    : ptr to mem containing data to be written to Profile blob
  
DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp_set_profile_name(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_profile_name: ENTRY", 0 ); 

  memcpy((void*)((dsi_profile_3gpp_type *)profile)->prf->profile_name, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_profile_name: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_pdp_context_pdp_type(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_pdp_type: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->context.pdp_type, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_pdp_type: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_pdp_context_h_comp(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_h_comp: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->context.h_comp, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_h_comp: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_pdp_context_d_comp(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_d_comp: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->context.d_comp, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_d_comp: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_pdp_context_apn(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_apn: ENTRY", 0 ); 

  memcpy((void*)((dsi_profile_3gpp_type *)profile)->prf->context.apn, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_apn: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_dns_addr_v4_primary(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  ds_profile_3gpp_ip_version_enum_type * ip_vsn_p;

  DS_PROFILE_LOGD( "_3gpp_set_dns_addr_v4_primary: ENTRY", 0 ); 

  ip_vsn_p = &(((dsi_profile_3gpp_type *)profile)->prf->dns_addr_primary_ip_vsn);

  if (*ip_vsn_p == DS_PROFILE_3GPP_IP_V6) {
    *ip_vsn_p = DS_PROFILE_3GPP_IP_V4V6;
  }

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->dns_addr.primary_dns_addr.ds_profile_3gpp_pdp_addr_ipv4, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_dns_addr_v4_primary: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_dns_addr_v6_primary(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  ds_profile_3gpp_ip_version_enum_type * ip_vsn_p;

  DS_PROFILE_LOGD( "_3gpp_set_dns_addr_v6_primary: ENTRY", 0 ); 

  ip_vsn_p = &(((dsi_profile_3gpp_type *)profile)->prf->dns_addr_primary_ip_vsn);

  if (*ip_vsn_p == DS_PROFILE_3GPP_IP_V4) {
    *ip_vsn_p = DS_PROFILE_3GPP_IP_V4V6;
  }

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->dns_addr.primary_dns_addr.ds_profile_3gpp_pdp_addr_ipv6, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_dns_addr_v6_primary: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_dns_addr_v4_secondary(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  ds_profile_3gpp_ip_version_enum_type * ip_vsn_p;

  DS_PROFILE_LOGD( "_3gpp_set_dns_addr_v4_secondary: ENTRY", 0 ); 

  ip_vsn_p = &(((dsi_profile_3gpp_type *)profile)->prf->dns_addr_primary_ip_vsn);

  if (*ip_vsn_p == DS_PROFILE_3GPP_IP_V6) {
    *ip_vsn_p = DS_PROFILE_3GPP_IP_V4V6;
  }

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->dns_addr.secondary_dns_addr.ds_profile_3gpp_pdp_addr_ipv4, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_dns_addr_v4_secondary: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_dns_addr_v6_secondary(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  ds_profile_3gpp_ip_version_enum_type * ip_vsn_p;

  DS_PROFILE_LOGD( "_3gpp_set_dns_addr_v6_secondary: ENTRY", 0 ); 

  ip_vsn_p = &(((dsi_profile_3gpp_type *)profile)->prf->dns_addr_secondary_ip_vsn);

  if (*ip_vsn_p == DS_PROFILE_3GPP_IP_V4) {
    *ip_vsn_p = DS_PROFILE_3GPP_IP_V4V6;
  }

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->dns_addr.secondary_dns_addr.ds_profile_3gpp_pdp_addr_ipv6, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_dns_addr_v6_secondary: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_umts_req_qos(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_3gpp_req_qos: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->qos_request_3gpp, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_3gpp_req_qos: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_umts_min_qos(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_3gpp_min_qos: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->qos_minimum_3gpp, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_3gpp_min_qos: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_gprs_req_qos(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_gprs_req_qos: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->qos_request_gprs, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_gprs_req_qos: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_gprs_min_qos(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_gprs_min_qos: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->qos_minimum_gprs, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_gprs_min_qos: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_auth_username(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_auth_username: ENTRY", 0 ); 

  memcpy((void*)((dsi_profile_3gpp_type *)profile)->prf->auth.username, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_auth_username: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_auth_password(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_auth_password: ENTRY", 0 ); 

  memcpy((void*)((dsi_profile_3gpp_type *)profile)->prf->auth.password, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_auth_password: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_auth_type(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_auth_type: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->auth.auth_type, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_auth_type: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_pdp_context_pdp_addr_v4(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  ds_profile_3gpp_ip_version_enum_type * ip_vsn_p;

  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_pdp_addr_v4: ENTRY", 0 ); 

  ip_vsn_p = &(((dsi_profile_3gpp_type *)profile)->prf->pdp_addr_ip_vsn);

  if (*ip_vsn_p == DS_PROFILE_3GPP_IP_V6) {
    *ip_vsn_p = DS_PROFILE_3GPP_IP_V4V6;
  }

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->context.pdp_addr.ds_profile_3gpp_pdp_addr_ipv4, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_pdp_addr_v4: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_pdp_context_pdp_addr_v6(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  ds_profile_3gpp_ip_version_enum_type * ip_vsn_p;

  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_pdp_addr_v6: ENTRY", 0 ); 

  ip_vsn_p = &(((dsi_profile_3gpp_type *)profile)->prf->pdp_addr_ip_vsn);

  if (*ip_vsn_p == DS_PROFILE_3GPP_IP_V4) {
    *ip_vsn_p = DS_PROFILE_3GPP_IP_V4V6;
  }

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->context.pdp_addr.ds_profile_3gpp_pdp_addr_ipv6, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_pdp_addr_v6: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_pcscf_req_flag(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_pcscf_req_flag: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->request_pcscf_address_flag, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_pcscf_req_flag: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_pdp_context_te_mt_access_ctrl_flag(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_te_mt_access_ctrl_flag: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->context.access_ctrl_flag, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_te_mt_access_ctrl_flag: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_pcscf_dhcp_req_flag(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_pcscf_dhcp_req_flag: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->request_pcscf_address_using_dhcp_flag, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_pcscf_dhcp_req_flag: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_im_cn_flag(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_im_cn_flag: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->im_cn_flag, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_im_cn_flag: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_param_tft_filter_id1(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_param_tft_filter_id1: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1], 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_param_tft_filter_id1: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_param_tft_filter_id2(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_param_tft_filter_id2: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2], 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_param_tft_filter_id2: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_pdp_context_number(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_number: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->context.pdp_context_number, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_number: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_pdp_context_secondary_flag(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_secondary_flag: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->context.secondary_flag, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_secondary_flag: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_pdp_context_primary_id(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{
  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_primary_id: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->context.primary_id, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_pdp_context_primary_id: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_ipv4_addr_alloc(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{ 
  DS_PROFILE_LOGD( "_3gpp_set_ipv4_addr_alloc: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->context.ipv4_addr_alloc, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_ipv4_addr_alloc: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_lte_req_qos(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{ 
  DS_PROFILE_LOGD( "_3gpp_set_lte_req_qos: ENTRY", 0 ); 

  memcpy((void*)&((dsi_profile_3gpp_type *)profile)->prf->qos_request_lte, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD( "_3gpp_set_lte_req_qos: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_set_param_invalid(
  void                       *profile,
  uint32                      mask,
  const ds_profile_info_type *info
)
{ 
  (void)profile; (void)mask; (void)info;
  DS_PROFILE_LOGE( "_3gpp_set: Invalid identifier", 0 );
  return DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT;
}

/*=========================================================================*/


/*---------------------------------------------------------------------------
                   ACCESSOR FUNCTIONS: GET
---------------------------------------------------------------------------*/

/*===========================================================================
FUNCTION dsi_profile_get_ group of functions

DESCRIPTION
  This set of accessor functions are used to get Profile 
  parameter from Profile blob

PARAMETERS
  profile : ptr to 3GPP Profile blob 
  info    : ptr to mem containing data to be written to Profile blob
  
DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  none
===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp_get_profile_name(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_profile_name: ENTRY", 0);

  info->len = DS_PROFILE_STR_LEN((char *)((dsi_profile_3gpp_type *)profile)->prf->profile_name)+1;
  memcpy( (void *)info->buf, 
          (void*)((dsi_profile_3gpp_type *)profile)->prf->profile_name, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_profile_name: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_pdp_context_pdp_type(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_pdp_context_pdp_type: ENTRY", 0);

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_context_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_pdp_context_pdp_type: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->context.pdp_type, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_pdp_context_pdp_type: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_pdp_context_h_comp(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_pdp_context_h_comp: ENTRY", 0);

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_context_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_pdp_context_h_comp: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->context.h_comp, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_pdp_context_h_comp: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_pdp_context_d_comp(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_pdp_context_d_comp: ENTRY", 0);

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_context_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_pdp_context_d_comp: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->context.d_comp, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_pdp_context_d_comp: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_pdp_context_apn(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_pdp_context_apn: ENTRY", 0);

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_context_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_pdp_context_apn: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  info->len = DS_PROFILE_STR_LEN((char *)((dsi_profile_3gpp_type *)profile)->prf->context.apn)+1;
  memcpy( (void *)info->buf, 
          (void*)((dsi_profile_3gpp_type *)profile)->prf->context.apn, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_pdp_context_apn: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_dns_addr_v4_primary(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_dns_addr_v4_primary: ENTRY", 0);

  if ( (((dsi_profile_3gpp_type *)profile)->prf->dns_addr_primary_ip_vsn != DS_PROFILE_3GPP_IP_V4) &&
       (((dsi_profile_3gpp_type *)profile)->prf->dns_addr_primary_ip_vsn != DS_PROFILE_3GPP_IP_V4V6) )
    return DS_PROFILE_REG_RESULT_ERR_INVAL;

  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->dns_addr.primary_dns_addr.ds_profile_3gpp_pdp_addr_ipv4, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_dns_addr_v4_primary: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_dns_addr_v6_primary(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_dns_addr_v6_primary: ENTRY", 0);

  if ( (((dsi_profile_3gpp_type *)profile)->prf->dns_addr_primary_ip_vsn != DS_PROFILE_3GPP_IP_V6) &&
       (((dsi_profile_3gpp_type *)profile)->prf->dns_addr_primary_ip_vsn != DS_PROFILE_3GPP_IP_V4V6) )
    return DS_PROFILE_REG_RESULT_ERR_INVAL;

  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->dns_addr.primary_dns_addr.ds_profile_3gpp_pdp_addr_ipv6, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_dns_addr_v6_primary: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_dns_addr_v4_secondary(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_dns_addr_v4_secondary: ENTRY", 0);

  if ( (((dsi_profile_3gpp_type *)profile)->prf->dns_addr_secondary_ip_vsn != DS_PROFILE_3GPP_IP_V4) &&
       (((dsi_profile_3gpp_type *)profile)->prf->dns_addr_secondary_ip_vsn != DS_PROFILE_3GPP_IP_V4V6) )
    return DS_PROFILE_REG_RESULT_ERR_INVAL;

  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->dns_addr.secondary_dns_addr.ds_profile_3gpp_pdp_addr_ipv4, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_dns_addr_v4_secondary: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_dns_addr_v6_secondary(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_dns_addr_v6_secondary: ENTRY", 0);

  if ( (((dsi_profile_3gpp_type *)profile)->prf->dns_addr_secondary_ip_vsn != DS_PROFILE_3GPP_IP_V6) &&
       (((dsi_profile_3gpp_type *)profile)->prf->dns_addr_secondary_ip_vsn != DS_PROFILE_3GPP_IP_V4V6) )
    return DS_PROFILE_REG_RESULT_ERR_INVAL;

  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->dns_addr.secondary_dns_addr.ds_profile_3gpp_pdp_addr_ipv6, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_dns_addr_v6_secondary: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_umts_req_qos(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_3gpp_req_qos: ENTRY", 0);

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_qos_req_3gpp_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_3gpp_req_qos: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->qos_request_3gpp, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_3gpp_req_qos: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_umts_min_qos(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_3gpp_min_qos: ENTRY", 0);

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_qos_min_3gpp_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_3gpp_min_qos: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->qos_minimum_3gpp, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_3gpp_min_qos: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_gprs_req_qos(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_gprs_req_qos: ENTRY", 0);

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_qos_req_gprs_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_gprs_req_qos: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->qos_request_gprs, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_gprs_req_qos: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_gprs_min_qos(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_gprs_min_qos: ENTRY", 0);

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_qos_min_gprs_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_gprs_min_qos: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->qos_minimum_gprs, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_gprs_min_qos: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_auth_username(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_auth_username: ENTRY", 0);

  info->len = DS_PROFILE_STR_LEN((char *)((dsi_profile_3gpp_type *)profile)->prf->auth.username)+1;
  memcpy( (void *)info->buf, 
          (void*)((dsi_profile_3gpp_type *)profile)->prf->auth.username, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_auth_username: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_auth_password(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_auth_password: ENTRY", 0);

  info->len = DS_PROFILE_STR_LEN((char *)((dsi_profile_3gpp_type *)profile)->prf->auth.password)+1;
  memcpy( (void *)info->buf, 
          (void*)((dsi_profile_3gpp_type *)profile)->prf->auth.password, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_auth_password: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_auth_type(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_auth_type: ENTRY", 0);

  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->auth.auth_type, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_auth_type: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_pdp_context_pdp_addr_v4(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_pdp_context_pdp_addr_v4: ENTRY", 0);

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_context_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_pdp_context_pdp_addr_v4: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  if ( (((dsi_profile_3gpp_type *)profile)->prf->pdp_addr_ip_vsn != DS_PROFILE_3GPP_IP_V4 ) &&
       (((dsi_profile_3gpp_type *)profile)->prf->pdp_addr_ip_vsn != DS_PROFILE_3GPP_IP_V4V6) )
    return DS_PROFILE_REG_RESULT_ERR_INVAL;

  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->context.pdp_addr.ds_profile_3gpp_pdp_addr_ipv4, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_pdp_context_pdp_addr_v4: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_pdp_context_pdp_addr_v6(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_pdp_context_pdp_addr_v6: ENTRY", 0);

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_context_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_pdp_context_pdp_addr_v6: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  if ( (((dsi_profile_3gpp_type *)profile)->prf->pdp_addr_ip_vsn != DS_PROFILE_3GPP_IP_V6 ) &&
       (((dsi_profile_3gpp_type *)profile)->prf->pdp_addr_ip_vsn != DS_PROFILE_3GPP_IP_V4V6) )
    return DS_PROFILE_REG_RESULT_ERR_INVAL;

  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->context.pdp_addr.ds_profile_3gpp_pdp_addr_ipv6, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_pdp_context_pdp_addr_v6: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_pcscf_req_flag(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_pcscf_req_flag: ENTRY", 0);

  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->request_pcscf_address_flag, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_pcscf_req_flag: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_pdp_context_te_mt_access_ctrl_flag(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_pdp_context_te_mt_access_ctrl_flag: ENTRY", 0);

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_context_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_pdp_context_te_mt_access_ctrl_flag: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->context.access_ctrl_flag, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_pdp_context_te_mt_access_ctrl_flag: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_pcscf_dhcp_req_flag(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_pcscf_dhcp_req_flag: ENTRY", 0);

  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->request_pcscf_address_using_dhcp_flag, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_pcscf_dhcp_req_flag: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_im_cn_flag(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_im_cn_flag: ENTRY", 0);

  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->im_cn_flag, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_im_cn_flag: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_param_tft_filter_id1(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_param_tft_filter_id1: ENTRY", 0);
 
  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_tft_valid_flg[DS_PROFILE_3GPP_TFT_FILTER_ID1] )
  {
    DS_PROFILE_LOGD("_3gpp_get_param_tft_filter_id1: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }

  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID1], 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_param_tft_filter_id1: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_param_tft_filter_id2(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_param_tft_filter_id2: ENTRY", 0);
 
  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_tft_valid_flg[DS_PROFILE_3GPP_TFT_FILTER_ID2] )
  {
    DS_PROFILE_LOGD("_3gpp_get_param_tft_filter_id2: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }

  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->tft[DS_PROFILE_3GPP_TFT_FILTER_ID2], 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_param_tft_filter_id1: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_pdp_context_number(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  DS_PROFILE_LOGD("_3gpp_get_pdp_context_number: ENTRY", 0);

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_context_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_pdp_context_number: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->context.pdp_context_number, 
          info->len );

  DS_PROFILE_LOGD("_3gpp_get_pdp_context_number: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_pdp_context_secondary_flag(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD( "_3gpp_get_pdp_context_secondary_flag: ENTRY", 0 ); 

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_context_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_pdp_context_secondary_flag: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->context.secondary_flag, 
          info->len );

  DS_PROFILE_LOGD( "_3gpp_get_pdp_context_secondary_flag: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_pdp_context_primary_id(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD( "_3gpp_get_pdp_context_primary_id: ENTRY", 0 ); 

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_context_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_pdp_context_primary_id: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->context.primary_id, 
          info->len );

  DS_PROFILE_LOGD( "_3gpp_get_pdp_context_primary_id: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_ipv4_addr_alloc(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD( "_3gpp_get_ipv4_addr_alloc: ENTRY", 0 ); 

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_context_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_ipv4_addr_alloc: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->context.ipv4_addr_alloc, 
          info->len );

  DS_PROFILE_LOGD( "_3gpp_get_ipv4_addr_alloc: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_lte_req_qos(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD( "_3gpp_get_lte_req_qos: ENTRY", 0 ); 

  if ( !((dsi_profile_3gpp_type *)profile)->prf->ds_profile_3gpp_qos_req_lte_valid_flg )
  {
    DS_PROFILE_LOGD("_3gpp_get_lte_req_qos: valid flg not set", 0);
    return DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET;
  }
  memcpy( (void *)info->buf, 
          (void*)&((dsi_profile_3gpp_type *)profile)->prf->qos_request_lte, 
          info->len );

  DS_PROFILE_LOGD( "_3gpp_get_lte_req_qos: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp_get_param_invalid(
  const void            *profile,
  ds_profile_info_type  *info
)
{ 
  (void)profile; (void)info;
  DS_PROFILE_LOGE( "_3gpp_get: Invalid identifier", 0 );
  return DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT;
}

/*=========================================================================*/


/*-----------------------------------------------------------------------------
          3GPP PARAMS Table internal to DSI_PROFILE_3GPP module
-----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------- 
  Table for parameter and the size of the parameter required in get/set
  functions 
-----------------------------------------------------------------------------*/
static dsi_profile_params_desc_type ds_profile_3gpp_profile_params_desc_tbl[] = {
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_INVALID, 
   0},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PROFILE_NAME, 
   (DS_PROFILE_3GPP_MAX_PROFILE_NAME_LEN+1)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_TYPE, 
   sizeof(ds_profile_3gpp_pdp_type_enum_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_H_COMP, 
   sizeof(ds_profile_3gpp_pdp_header_comp_e_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_D_COMP, 
   sizeof(ds_profile_3gpp_pdp_data_comp_e_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_APN, 
   (DS_PROFILE_3GPP_MAX_APN_STRING_LEN+1)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V4_PRIMARY, 
   sizeof(ds_profile_3gpp_pdp_addr_type_ipv4)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V4_SECONDARY, 
   sizeof(ds_profile_3gpp_pdp_addr_type_ipv4)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_REQ_QOS, 
   sizeof(ds_profile_3gpp_3gpp_qos_params_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_MIN_QOS, 
   sizeof(ds_profile_3gpp_3gpp_qos_params_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_GPRS_REQ_QOS, 
   sizeof(ds_profile_3gpp_gprs_qos_params_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_GPRS_MIN_QOS, 
   sizeof(ds_profile_3gpp_gprs_qos_params_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_USERNAME, 
   (DS_PROFILE_3GPP_MAX_QCPDP_STRING_LEN+1)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_PASSWORD, 
   (DS_PROFILE_3GPP_MAX_QCPDP_STRING_LEN+1)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_TYPE, 
   sizeof(ds_profile_3gpp_auth_pref_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_ADDR_V4, 
   sizeof(ds_profile_3gpp_pdp_addr_type_ipv4)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PCSCF_REQ_FLAG, 
   sizeof(ds_profile_3gpp_request_pcscf_address_flag_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_TE_MT_ACCESS_CTRL_FLAG, 
   sizeof(ds_profile_3gpp_pdp_access_control_e_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PCSCF_DHCP_REQ_FLAG, 
   sizeof(ds_profile_3gpp_request_pcscf_address_using_dhcp_flag_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_IM_CN_FLAG, 
   sizeof(ds_profile_3gpp_im_cn_flag_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_TFT_FILTER_ID1, 
   (sizeof(ds_profile_3gpp_tft_params_type))},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_TFT_FILTER_ID2, 
   (sizeof(ds_profile_3gpp_tft_params_type))},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_NUMBER, 
   sizeof(ds_profile_3gpp_pdp_context_number_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_SECONDARY_FLAG, 
   sizeof(ds_profile_3gpp_pdp_context_secondary_flag_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PRIMARY_ID, 
   sizeof(ds_profile_3gpp_pdp_context_primary_id_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_ADDR_V6, 
   sizeof(ds_profile_3gpp_pdp_addr_type_ipv6)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_REQ_QOS_EXTENDED, 
   (sizeof(ds_profile_3gpp_3gpp_qos_params_type) - sizeof(boolean))},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_MIN_QOS_EXTENDED, 
   (sizeof(ds_profile_3gpp_3gpp_qos_params_type) - sizeof(boolean))},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V6_PRIMARY, 
   sizeof(ds_profile_3gpp_pdp_addr_type_ipv6)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V6_SECONDARY, 
   sizeof(ds_profile_3gpp_pdp_addr_type_ipv6)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_IPV4_ADDR_ALLOC, 
   sizeof(ds_profile_3gpp_pdp_ipv4_addr_alloc_e_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_LTE_REQ_QOS, 
   sizeof(ds_profile_3gpp_lte_qos_params_type)}
};

/*----------------------------------------------------------------------------- 
  Table for parameter and its get/set functions
-----------------------------------------------------------------------------*/ 
static dsi_profile_acc_mut_fn_type ds_profile_3gpp_acc_mut_fn_tbl[] = {

   /* dummy function, ident not valid*/
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_INVALID,
  dsi_profile_3gpp_set_param_invalid,
  dsi_profile_3gpp_get_param_invalid},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PROFILE_NAME,
  dsi_profile_3gpp_set_profile_name,
  dsi_profile_3gpp_get_profile_name},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_TYPE,
  dsi_profile_3gpp_set_pdp_context_pdp_type,
  dsi_profile_3gpp_get_pdp_context_pdp_type},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_H_COMP,
  dsi_profile_3gpp_set_pdp_context_h_comp,
  dsi_profile_3gpp_get_pdp_context_h_comp},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_D_COMP,
  dsi_profile_3gpp_set_pdp_context_d_comp,
  dsi_profile_3gpp_get_pdp_context_d_comp},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_APN,
  dsi_profile_3gpp_set_pdp_context_apn,
  dsi_profile_3gpp_get_pdp_context_apn},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V4_PRIMARY,
  dsi_profile_3gpp_set_dns_addr_v4_primary,
  dsi_profile_3gpp_get_dns_addr_v4_primary},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V4_SECONDARY,
  dsi_profile_3gpp_set_dns_addr_v4_secondary,
  dsi_profile_3gpp_get_dns_addr_v4_secondary},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_REQ_QOS,
  dsi_profile_3gpp_set_umts_req_qos,
  dsi_profile_3gpp_get_umts_req_qos},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_MIN_QOS,
  dsi_profile_3gpp_set_umts_min_qos,
  dsi_profile_3gpp_get_umts_min_qos},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_GPRS_REQ_QOS,
  dsi_profile_3gpp_set_gprs_req_qos,
  dsi_profile_3gpp_get_gprs_req_qos},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_GPRS_MIN_QOS,
  dsi_profile_3gpp_set_gprs_min_qos,
  dsi_profile_3gpp_get_gprs_min_qos},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_USERNAME,
  dsi_profile_3gpp_set_auth_username,
  dsi_profile_3gpp_get_auth_username},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_PASSWORD,
  dsi_profile_3gpp_set_auth_password,
  dsi_profile_3gpp_get_auth_password},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_TYPE,
  dsi_profile_3gpp_set_auth_type,
  dsi_profile_3gpp_get_auth_type},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_ADDR_V4,
  dsi_profile_3gpp_set_pdp_context_pdp_addr_v4,
  dsi_profile_3gpp_get_pdp_context_pdp_addr_v4},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PCSCF_REQ_FLAG,
  dsi_profile_3gpp_set_pcscf_req_flag,
  dsi_profile_3gpp_get_pcscf_req_flag},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_TE_MT_ACCESS_CTRL_FLAG,
  dsi_profile_3gpp_set_pdp_context_te_mt_access_ctrl_flag,
  dsi_profile_3gpp_get_pdp_context_te_mt_access_ctrl_flag},         

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PCSCF_DHCP_REQ_FLAG,
  dsi_profile_3gpp_set_pcscf_dhcp_req_flag,
  dsi_profile_3gpp_get_pcscf_dhcp_req_flag},                      
                          
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_IM_CN_FLAG,
  dsi_profile_3gpp_set_im_cn_flag,
  dsi_profile_3gpp_get_im_cn_flag},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_TFT_FILTER_ID1,
  dsi_profile_3gpp_set_param_tft_filter_id1,
  dsi_profile_3gpp_get_param_tft_filter_id1}, 

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_TFT_FILTER_ID2,
  dsi_profile_3gpp_set_param_tft_filter_id2,
  dsi_profile_3gpp_get_param_tft_filter_id2}, 

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_NUMBER,
  dsi_profile_3gpp_set_pdp_context_number,
  dsi_profile_3gpp_get_pdp_context_number},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_SECONDARY_FLAG,
  dsi_profile_3gpp_set_pdp_context_secondary_flag,
  dsi_profile_3gpp_get_pdp_context_secondary_flag},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PRIMARY_ID,
  dsi_profile_3gpp_set_pdp_context_primary_id,
  dsi_profile_3gpp_get_pdp_context_primary_id},
              
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_ADDR_V6,
  dsi_profile_3gpp_set_pdp_context_pdp_addr_v6,
  dsi_profile_3gpp_get_pdp_context_pdp_addr_v6},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_REQ_QOS_EXTENDED,
  dsi_profile_3gpp_set_umts_req_qos,
  dsi_profile_3gpp_get_umts_req_qos},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_MIN_QOS_EXTENDED,
  dsi_profile_3gpp_set_umts_min_qos,
  dsi_profile_3gpp_get_umts_min_qos},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V6_PRIMARY,
  dsi_profile_3gpp_set_dns_addr_v6_primary,
  dsi_profile_3gpp_get_dns_addr_v6_primary},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V6_SECONDARY,
  dsi_profile_3gpp_set_dns_addr_v6_secondary,
  dsi_profile_3gpp_get_dns_addr_v6_secondary},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_IPV4_ADDR_ALLOC,
  dsi_profile_3gpp_set_ipv4_addr_alloc,
  dsi_profile_3gpp_get_ipv4_addr_alloc},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP_PROFILE_PARAM_LTE_REQ_QOS,
  dsi_profile_3gpp_set_lte_req_qos,
  dsi_profile_3gpp_get_lte_req_qos}
};

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_GET_INDEX_FROM_IDENT

DESCRIPTION
  This function gets index for the identifier to index into the function table.

PARAMETERS 
  ident : identifier for which index is to be returned

DEPENDENCIES 
  
RETURN VALUE 
  returns index
SIDE EFFECTS 
  
===========================================================================*/
/*lint -save -e656*/
uint8 dsi_profile_3gpp_get_index_from_ident( 
  ds_profile_identifier_type ident
)
{
  uint8 index = 0;
  uint8 i     = 0;
  for (i = 0; 
       i <= ((DS_PROFILE_3GPP_PROFILE_PARAM_MAX - DS_PROFILE_3GPP_PROFILE_PARAM_MIN)+1); 
       i++ )
  {
    if (ident == ds_profile_3gpp_profile_params_desc_tbl[i].uid)
    {
      index = i;
      break;
    }
  }
  return index;
}
/*lint -restore Restore lint error 656*/
/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_ALLOC_PROFILE
 
DESCRIPTION
  This function is used to allocate memory which will be used for the local copy
  of the profile

PARAMETERS 
  
DEPENDENCIES 
  
RETURN VALUE 
  pointer to which memory is allocated
  NULL on failure
 
SIDE EFFECTS 
  
===========================================================================*/
static void* dsi_profile_3gpp_alloc_profile()
{
  dsi_profile_3gpp_type *tmp_prf = NULL;
  DS_PROFILE_LOGD("_3gpp_alloc_profile: ENTRY", 0 );
  
  tmp_prf = (dsi_profile_3gpp_type *)DS_PROFILE_MEM_ALLOC(sizeof(dsi_profile_3gpp_type),
                                                     MODEM_MEM_CLIENT_DATA);

  if (tmp_prf == NULL) 
  {
    DS_PROFILE_LOGE("_3gpp_alloc_profile: FAILED DS_PROFILE_MEM_ALLOC", 0 );
    return NULL;
  }

  tmp_prf->prf = (ds_profile_3gpp_profile_info_type *)DS_PROFILE_MEM_ALLOC(
      sizeof(ds_profile_3gpp_profile_info_type),
      MODEM_MEM_CLIENT_DATA);

  if (tmp_prf->prf == NULL) 
  {
    DS_PROFILE_MEM_FREE( (void *)tmp_prf, 
			MODEM_MEM_CLIENT_DATA);
    tmp_prf = NULL;
    DS_PROFILE_LOGE("_3gpp_alloc_profile: FAILED DS_PROFILE_MEM_ALLOC", 0 );
    return NULL;
  }

  tmp_prf->self = tmp_prf;
  DS_PROFILE_LOGD("_3gpp_alloc_profile: EXIT with SUCCESS", 0);
  return tmp_prf;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_DEALLOC_PROFILE
 
DESCRIPTION
  This function is used to free memory which was allocated for the local copy
  of the profile

PARAMETERS 
  ptr : pointer to local copy of profile

DEPENDENCIES 
  
RETURN VALUE 
  DSI_SUCCESS
  DSI_FAILURE
 
SIDE EFFECTS 
  
===========================================================================*/
static int dsi_profile_3gpp_dealloc_profile(
  void *ptr
)
{
  DS_PROFILE_LOGD("_3gpp_dealloc_profile: ENTRY", 0);

  if ( ptr == NULL || ( ((dsi_profile_3gpp_type *)ptr)->prf == NULL ) ) 
  {
    DS_PROFILE_LOGE( "_3gpp_dealloc_profile: ptr NULL", 0);
    DS_PROFILE_LOGE( "_3gpp_dealloc_profile: EXIT with ERR", 0);
    return DSI_FAILURE;
  }

  DS_PROFILE_MEM_FREE( (void *) ((dsi_profile_3gpp_type *)ptr)->prf,
                  MODEM_MEM_CLIENT_DATA ); 
  DS_PROFILE_MEM_FREE( (void *)ptr,
                  MODEM_MEM_CLIENT_DATA );

  DS_PROFILE_LOGD("_3gpp_dealloc_profile: EXIT with SUCCESS", 0);
  return DSI_SUCCESS;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_SET_PARAM
 
DESCRIPTION
  This function is used to set 3GPP parameter value in the local copy
  of the profile

PARAMETERS 
  blob : pointer to local copy of profile
  ident : identifier whose value is to be set
  info : pointer to store value of identifier to be modified

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT
  DS_PROFILE_REG_RESULT_ERR_LEN_INVALID
  DS_PROFILE_REG_RESULT_SUCCESS
 
SIDE EFFECTS 
  
===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp_set_param( 
  void                        *blob,
  ds_profile_identifier_type   ident,
  const ds_profile_info_type  *info
)
{
  uint32  mask = 0;
  uint8 index = 0;
  DS_PROFILE_LOGD("_3gpp_set_param: ENTRY ", 0);

  /* Validate identifier */
  if ( !DS_PROFILE_3GPP_IDENT_IS_VALID( ident ) )
  {
    ident = DS_PROFILE_3GPP_PROFILE_PARAM_INVALID;
    return DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT;
  }
  index = dsi_profile_3gpp_get_index_from_ident( ident );
  /* Validate info->buf and info->len */
  if ( !SET_INFO_IS_VALID( info, ds_profile_3gpp_profile_params_desc_tbl[index].len ) ) 
  {
    return DS_PROFILE_REG_RESULT_ERR_LEN_INVALID;
  }
  /* get mask from identifier */
  CONVERT_IDENT_TO_MASK( mask, index );
  return ds_profile_3gpp_acc_mut_fn_tbl[index].set_fn(blob, mask, info);
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP_GET_PARAM
 
DESCRIPTION
  This function is used to get 3GPP parameter value from the local copy
  of the profile

PARAMETERS 
  blob : pointer to local copy of profile
  ident : identifier to get value
  info : pointer to store value of identifier fetched

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT
  DS_PROFILE_REG_RESULT_ERR_LEN_INVALID
  DS_PROFILE_REG_RESULT_SUCCESS
 
SIDE EFFECTS 
  
===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp_get_param(
  void                        *blob,
  ds_profile_identifier_type   ident,
  ds_profile_info_type        *info
)
{
  uint8 index = 0;
  DS_PROFILE_LOGD("_3gpp_get_param: ENTRY", 0);
  /* Validate identifier */
  if ( !DS_PROFILE_3GPP_IDENT_IS_VALID( ident ) )
  {
    ident = DS_PROFILE_3GPP_PROFILE_PARAM_INVALID; 
    return DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT;
  }
  index = dsi_profile_3gpp_get_index_from_ident( ident );
  /* Validate info->buf and info->len */
  if ( !GET_INFO_IS_VALID( info, ds_profile_3gpp_profile_params_desc_tbl[index].len ) ) 
  {
    return DS_PROFILE_REG_RESULT_ERR_LEN_INVALID;
  }
  info->len = ds_profile_3gpp_profile_params_desc_tbl[index].len;
  return ds_profile_3gpp_acc_mut_fn_tbl[index].get_fn(blob, info);
}

/*===========================================================================
FUNCTION DS_PROFILE_3GPP_INIT

DESCRIPTION
  This function is called on the library init. It initializes the function
  pointers to valid functions for 3gpp

PARAMETERS 
  fntbl : pointer to table of function pointers

DEPENDENCIES 
  
RETURN VALUE 
  returns the mask for 3gpp. (Used later as valid mask which is ORed value
  returned from all techs.
SIDE EFFECTS 
  
===========================================================================*/

uint8 ds_profile_3gpp_init ( tech_fntbl_type *fntbl )
{
  DS_PROFILE_LOGD("3gpp_init: ENTRY", 0);
  /* Init function pointers */
  fntbl->alloc     = dsi_profile_3gpp_alloc_profile;
  fntbl->dealloc   = dsi_profile_3gpp_dealloc_profile;
  fntbl->set_param = dsi_profile_3gpp_set_param;
  fntbl->get_param = dsi_profile_3gpp_get_param;

  DS_PROFILE_LOGD("3gpp_init: EXIT with SUCCESS", 0);
  return (0x01 << DS_PROFILE_TECH_3GPP);
}

/*lint -restore Restore lint error 641*/

