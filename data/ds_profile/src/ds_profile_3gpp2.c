/******************************************************************************
  @file    ds_profile_3gpp2.c
  @brief   

  DESCRIPTION
  Tech specific implementation of 3GPP2 1x Profile Management  

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

  $Header: //source/qcom/qct/modem/data/1x/dsprofile3gpp2/main/latest/src/ds_profile_3gpp2.c#3 $ $DateTime: 2009/10/26 12:52:40 $ $Author: mghotge $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

#include "ds_profile_3gpp2i.h"
#include "ds_profile_os.h"

/*lint -save -e641*/
/*lint -save -e655*/

/*---------------------------------------------------------------------------
                       UTILITY MACROS
---------------------------------------------------------------------------*/

/* Macro to check info (for get_param function)
  (info->buf not NULL and validate info->len) */
#define GET_INFO_IS_VALID( info, max_len ) \
  ( (info->buf != NULL) && (info->len >= max_len) )

/* Macro to check info (for set_param function)
  (info->buf not NULL and validate info->len) */
#define SET_INFO_IS_VALID( info, max_len ) \
  ( (info->buf != NULL) && ( (info->len > 0) && (info->len <= max_len) ) )


/*---------------------------------------------------------------------------
              3GPP2 PARAMS ACCCESSOR/MUTATOR ROUTINES & UTILS
---------------------------------------------------------------------------*/

/*===========================================================================
FUNCTION dsi_profile_set_ group of functions

DESCRIPTION
  This set of accessor functions are used to set corresponding Profile 
  parameter

PARAMETERS
  profile : ptr to 3GPP2 Profile blob 
  mask    : mask to identify Profile parameter
  info    : ptr to mem containing data to be written to Profile blob
  
DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  none
===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_negotiate_dns_server(
  void                  *profile,
  uint32                 mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD( "_3gpp2_set_negotiate_dns_server: ENTRY", 0 ); 
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->negotiate_dns_server, 
        info->buf, info->len ); 
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_negotiate_dns_server: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_ppp_session_close_timer_DO(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_ppp_session_close_timer_DO: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->ppp_session_close_timer_DO, 
       info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_ppp_session_close_timer_DO: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_ppp_session_close_timer_1X(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_ppp_session_close_timer_1X: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->ppp_session_close_timer_1X, 
       info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_ppp_session_close_timer_1X: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_allow_linger(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_allow_linger: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->allow_linger, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_allow_linger: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_lcp_ack_timeout(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_lcp_ack_timeout: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->lcp_ack_timeout, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_lcp_ack_timeout: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_ipcp_ack_timeout(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_ipcp_ack_timeout: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->ipcp_ack_timeout, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_ipcp_ack_timeout: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_auth_timeout(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_auth_timeout: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->auth_timeout, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_auth_timeout: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_lcp_creq_retry_count(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_lcp_creq_retry_count: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->lcp_creq_retry_count, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_lcp_creq_retry_count: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_ipcp_creq_retry_count(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_ipcp_creq_retry_count: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->ipcp_creq_retry_count, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_ipcp_creq_retry_count: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_auth_retry_count(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_auth_retry_count: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->auth_retry_count, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_auth_retry_count: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_auth_protocol(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_auth_protocol: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->auth_protocol, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_auth_protocol: EXIT with SUCCESS", 0); 
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_user_id(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_user_id: ENTRY", 0);
  memcpy((void*)((dsi_profile_3gpp2_type *)profile)->prf->user_id, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->prf->user_id_len = (uint8)info->len;
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_user_id: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_auth_password(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_auth_password: ENTRY", 0);
  memcpy((void*)((dsi_profile_3gpp2_type *)profile)->prf->auth_password, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->prf->auth_password_len = (uint8)info->len;
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_auth_password: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_data_rate(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_data_rate: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->data_rate, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_data_rate: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_data_mode(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_data_mode: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->data_mode, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_data_mode: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_app_type(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  (void)profile; (void)mask;(void)info;
  DS_PROFILE_LOGD("_3gpp2_set_app_type: ENTRY, set app_type not allowed", 0);
  
  /*memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->app_type, 
          info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  PLM_LOGD("_3gpp2_set_app_type: EXIT with SUCCESS", 0);*/
  return DS_PROFILE_REG_RESULT_ERR_INVAL_OP;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_app_priority(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  (void)profile; (void)mask;(void)info;
  DS_PROFILE_LOGD("_3gpp2_set_app_priority: ENTRY, set app_priority not allowed", 0);
  
  /*memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->app_priority, 
          info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  PLM_LOGD("_3gpp2_set_app_priority: EXIT with SUCCESS", 0);*/
  return DS_PROFILE_REG_RESULT_ERR_INVAL_OP;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_apn_string(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_apn_string: ENTRY", 0);
  memcpy((void*)((dsi_profile_3gpp2_type *)profile)->prf->apn_string, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->prf->apn_string_len = (uint8)info->len;
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_apn_string: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_pdn_type(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_pdn_type: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->pdn_type, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_pdn_type: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_is_pcscf_addr_needed(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_is_pcscf_addr_needed: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->is_pcscf_addr_needed, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_is_pcscf_addr_needed: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_v4_dns_addr_primary(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_v4_dns_addr_primary: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->v4_dns_addr[DS_PROFILE_3GPP2_PRIMARY_DNS_ADDR], 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_v4_dns_addr_primary: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_v4_dns_addr_secondary(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_v4_dns_addr_secondary: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->v4_dns_addr[DS_PROFILE_3GPP2_SECONDARY_DNS_ADDR], 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_v4_dns_addr_secondary: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_v6_dns_addr_primary(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_v6_dns_addr_primary: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->v6_dns_addr[DS_PROFILE_3GPP2_PRIMARY_DNS_ADDR], 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_v6_dns_addr_primary: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_v6_dns_addr_secondary(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_v6_dns_addr_secondary: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->v6_dns_addr[DS_PROFILE_3GPP2_SECONDARY_DNS_ADDR], 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_v6_dns_addr_secondary: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_rat_type(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_set_rat_type: ENTRY", 0);
  memcpy((void*)&((dsi_profile_3gpp2_type *)profile)->prf->rat_type, 
        info->buf, info->len );
  ((dsi_profile_3gpp2_type *)profile)->mask |= mask;

  DS_PROFILE_LOGD("_3gpp2_set_rat_type: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_set_param_invalid(
  void                  *profile,
  uint32               mask,
  const ds_profile_info_type  *info
)
{ 
  (void)profile; (void)mask;(void)info;
  DS_PROFILE_LOGE("_3gpp2_set: Invalid identifier", 0);
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
  parameter identified by mask from Profile blob

PARAMETERS
  -> profile : ptr to 3GPP2 Profile blob 
  -> info    : ptr to mem containing data to be written to Profile blob
  
DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
  none
===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_negotiate_dns_server(
  const void             *profile,
  ds_profile_info_type   *info
)
{ 
  DS_PROFILE_LOGD("_3gpp2_get_negotiate_dns_server: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->negotiate_dns_server, 
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_negotiate_dns_server: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_ppp_session_close_timer_DO(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_ppp_session_close_timer_DO: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->ppp_session_close_timer_DO,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_ppp_session_close_timer_DO: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_ppp_session_close_timer_1X(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_ppp_session_close_timer_1X: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->ppp_session_close_timer_1X,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_ppp_session_close_timer_1X: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_allow_linger(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_allow_linger: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->allow_linger,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_allow_linger: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_lcp_ack_timeout(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_lcp_ack_timeout: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->lcp_ack_timeout,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_lcp_ack_timeout: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_ipcp_ack_timeout(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_ipcp_ack_timeout: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->ipcp_ack_timeout,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_ipcp_ack_timeout: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_auth_timeout(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_auth_timeout: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->auth_timeout,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_auth_timeout: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_lcp_creq_retry_count(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_lcp_creq_retry_count: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->lcp_creq_retry_count,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_lcp_creq_retry_count: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_ipcp_creq_retry_count(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_ipcp_creq_retry_count: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->ipcp_creq_retry_count,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_ipcp_creq_retry_count: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_auth_retry_count(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_auth_retry_count: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->auth_retry_count,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_auth_retry_count: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_auth_protocol(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_auth_protocol: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->auth_protocol,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_auth_protocol: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_user_id(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_user_id: ENTRY", 0);
  memcpy( info->buf, 
        (void*)((dsi_profile_3gpp2_type *)profile)->prf->user_id,
        ((dsi_profile_3gpp2_type *)profile)->prf->user_id_len );
  info->len = ((dsi_profile_3gpp2_type *)profile)->prf->user_id_len;

  DS_PROFILE_LOGD("_3gpp2_get_user_id: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_auth_password(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_auth_password: ENTRY", 0);
  memcpy( info->buf, 
        (void*)((dsi_profile_3gpp2_type *)profile)->prf->auth_password,
        ((dsi_profile_3gpp2_type *)profile)->prf->auth_password_len );
  info->len = ((dsi_profile_3gpp2_type *)profile)->prf->auth_password_len;

  DS_PROFILE_LOGD("_3gpp2_get_auth_password: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_data_rate(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_data_rate: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->data_rate,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_data_rate: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_data_mode(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_data_mode: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->data_mode,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_data_mode: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_app_type(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  (void)profile; (void)info;
  DS_PROFILE_LOGD("_3gpp2_get_app_type: ENTRY, get app_type not allowed", 0);
  /*memcpy( info->buf, 
          (void*)&((dsi_profile_3gpp2_type *)profile)->prf->app_type,
          info->len );

  PLM_LOGD("_3gpp2_get_app_type: EXIT with SUCCESS", 0); */
  return DS_PROFILE_REG_RESULT_ERR_INVAL_OP;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_app_priority(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  (void)profile; (void)info;
  DS_PROFILE_LOGD("_3gpp2_get_app_priority: ENTRY, get app_priority not allowed", 0);
  /*memcpy( info->buf, 
          (void*)&((dsi_profile_3gpp2_type *)profile)->prf->app_priority,
          info->len );

  PLM_LOGD("_3gpp2_get_app_priority: EXIT with SUCCESS", 0);*/
  return DS_PROFILE_REG_RESULT_ERR_INVAL_OP;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_apn_string(
  const void             *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_apn_string: ENTRY", 0);
  memcpy( info->buf, 
        (void*)((dsi_profile_3gpp2_type *)profile)->prf->apn_string,
        ((dsi_profile_3gpp2_type *)profile)->prf->apn_string_len );
  info->len = ((dsi_profile_3gpp2_type *)profile)->prf->apn_string_len;

  DS_PROFILE_LOGD("_3gpp2_get_apn_string: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_pdn_type(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_pdn_type: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->pdn_type,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_pdn_type: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_is_pcscf_addr_needed(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_is_pcscf_addr_needed: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->is_pcscf_addr_needed,
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_is_pcscf_addr_needed: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_v4_dns_addr_primary(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_v4_dns_addr_primary: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->v4_dns_addr[DS_PROFILE_3GPP2_PRIMARY_DNS_ADDR],
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_v4_dns_addr_primary: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_v4_dns_addr_secondary(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_v4_dns_addr_secondary: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->v4_dns_addr[DS_PROFILE_3GPP2_SECONDARY_DNS_ADDR],
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_v4_dns_addr_secondary: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_v6_dns_addr_primary(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_v6_dns_addr_primary: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->v6_dns_addr[DS_PROFILE_3GPP2_PRIMARY_DNS_ADDR],
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_v6_dns_addr_primary: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_v6_dns_addr_secondary(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_v6_dns_addr_secondary: ENTRY", 0);
  memcpy( info->buf, 
        (void*)&((dsi_profile_3gpp2_type *)profile)->prf->v6_dns_addr[DS_PROFILE_3GPP2_SECONDARY_DNS_ADDR],
        info->len );

  DS_PROFILE_LOGD("_3gpp2_get_v6_dns_addr_secondary: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*=========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_rat_type(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  DS_PROFILE_LOGD("_3gpp2_get_rat_type: ENTRY", 0);
  memcpy( info->buf, 
          (void*)&((dsi_profile_3gpp2_type *)profile)->prf->rat_type,
          info->len );

  DS_PROFILE_LOGD("_3gpp2_get_rat_type: EXIT with SUCCESS", 0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

/*===========================================================================*/
static ds_profile_status_etype dsi_profile_3gpp2_get_param_invalid(
  const void            *profile,
  ds_profile_info_type  *info
)
{
  (void)profile; (void)info;
  DS_PROFILE_LOGE("_3gpp2_get: Invalid identifier", 0);
  return DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT;
}

/*=========================================================================*/


/*-----------------------------------------------------------------------------
          3GPP2 PARAMS Table internal to DSI_PROFILE_3GPP2 module
-----------------------------------------------------------------------------*/ 
/*----------------------------------------------------------------------------- 
  Table for parameter and its validity mask
-----------------------------------------------------------------------------*/ 
static dsi_profile_3gpp2_params_valid_mask ds_profile_3gpp2_params_valid_mask[] = {
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_INVALID, 
   DS_PROFILE_3GPP2_PROFILE_INVALID},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_NEGOTIATE_DNS_SERVER, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_DO, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_1X, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_ALLOW_LINGER, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},
  
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_ACK_TIMEOUT, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_ACK_TIMEOUT, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_TIMEOUT, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_CREQ_RETRY_COUNT, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_CREQ_RETRY_COUNT, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_RETRY_COUNT, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PROTOCOL, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_USER_ID, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PASSWORD, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_RATE, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_MODE, 
   DS_PROFILE_3GPP2_PROFILE_COMMON},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_APP_TYPE, 
   DS_PROFILE_3GPP2_PROFILE_OMH},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_APP_PRIORITY, 
   DS_PROFILE_3GPP2_PROFILE_OMH},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_APN_STRING, 
   DS_PROFILE_3GPP2_PROFILE_EHRPD},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_PDN_TYPE, 
   DS_PROFILE_3GPP2_PROFILE_EHRPD},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_IS_PCSCF_ADDR_NEEDED, 
   DS_PROFILE_3GPP2_PROFILE_EHRPD},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_PRIMARY, 
   DS_PROFILE_3GPP2_PROFILE_EHRPD},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_SECONDARY, 
   DS_PROFILE_3GPP2_PROFILE_EHRPD},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_PRIMARY, 
   DS_PROFILE_3GPP2_PROFILE_EHRPD},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_SECONDARY, 
   DS_PROFILE_3GPP2_PROFILE_EHRPD},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_RAT_TYPE, 
   DS_PROFILE_3GPP2_PROFILE_EHRPD}
};

/*----------------------------------------------------------------------------- 
  Table for parameter and the size of the parameter required in get/set
  functions 
-----------------------------------------------------------------------------*/ 
static dsi_profile_params_desc_type ds_profile_3gpp2_profile_params_desc_tbl[] = {
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_INVALID, 
   0},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_NEGOTIATE_DNS_SERVER, 
   sizeof(ds_profile_3gpp2_negotiate_dns_server_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_DO, 
   sizeof(ds_profile_3gpp2_ppp_session_close_timer_DO_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_1X, 
   sizeof(ds_profile_3gpp2_ppp_session_close_timer_1X_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_ALLOW_LINGER, 
   sizeof(ds_profile_3gpp2_allow_linger_type)},
  
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_ACK_TIMEOUT, 
   sizeof(ds_profile_3gpp2_lcp_ack_timeout_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_ACK_TIMEOUT, 
   sizeof(ds_profile_3gpp2_ipcp_ack_timeout_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_TIMEOUT, 
   sizeof(ds_profile_3gpp2_auth_timeout_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_CREQ_RETRY_COUNT, 
   sizeof(ds_profile_3gpp2_lcp_creq_retry_count_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_CREQ_RETRY_COUNT, 
   sizeof(ds_profile_3gpp2_ipcp_creq_retry_count_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_RETRY_COUNT, 
   sizeof(ds_profile_3gpp2_auth_retry_count_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PROTOCOL, 
   sizeof(ds_profile_3gpp2_auth_protocol_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_USER_ID, 
   (DS_PROFILE_3GPP2_PPP_MAX_USER_ID_LEN)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PASSWORD, 
   (DS_PROFILE_3GPP2_PPP_MAX_PASSWD_LEN)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_RATE, 
   sizeof(ds_profile_3gpp2_data_rate_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_MODE, 
   sizeof(ds_profile_3gpp2_data_mode_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_APP_TYPE, 
   sizeof(ds_profile_3gpp2_app_type_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_APP_PRIORITY, 
   sizeof(ds_profile_3gpp2_app_priority_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_APN_STRING, 
   (DS_PROFILE_3GPP2_APN_MAX_VAL_LEN)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_PDN_TYPE, 
   sizeof(ds_profile_3gpp2_pdn_type_enum_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_IS_PCSCF_ADDR_NEEDED, 
   sizeof(ds_profile_3gpp2_is_pcscf_addr_needed_type)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_PRIMARY, 
   sizeof(struct ds_profile_3gpp2_in_addr)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_SECONDARY, 
   sizeof(struct ds_profile_3gpp2_in_addr)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_PRIMARY, 
   sizeof(struct ds_profile_3gpp2_in6_addr)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_SECONDARY, 
   sizeof(struct ds_profile_3gpp2_in6_addr)},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_RAT_TYPE, 
   sizeof(ds_profile_3gpp2_rat_type_enum_type)}
};

/*----------------------------------------------------------------------------- 
  Table for parameter and its get/set functions
-----------------------------------------------------------------------------*/ 
static dsi_profile_acc_mut_fn_type ds_profile_3gpp2_acc_mut_fn_tbl[] = {

   /* dummy function, ident not valid*/
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_INVALID,
  dsi_profile_3gpp2_set_param_invalid,
  dsi_profile_3gpp2_get_param_invalid},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_NEGOTIATE_DNS_SERVER,
  dsi_profile_3gpp2_set_negotiate_dns_server,
  dsi_profile_3gpp2_get_negotiate_dns_server},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_DO,
  dsi_profile_3gpp2_set_ppp_session_close_timer_DO,
  dsi_profile_3gpp2_get_ppp_session_close_timer_DO},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_1X,
  dsi_profile_3gpp2_set_ppp_session_close_timer_1X,
  dsi_profile_3gpp2_get_ppp_session_close_timer_1X},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_ALLOW_LINGER,
  dsi_profile_3gpp2_set_allow_linger,
  dsi_profile_3gpp2_get_allow_linger},   
    
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_ACK_TIMEOUT,
  dsi_profile_3gpp2_set_lcp_ack_timeout,
  dsi_profile_3gpp2_get_lcp_ack_timeout}, 

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_ACK_TIMEOUT,
  dsi_profile_3gpp2_set_ipcp_ack_timeout,
  dsi_profile_3gpp2_get_ipcp_ack_timeout},
              
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_TIMEOUT,
  dsi_profile_3gpp2_set_auth_timeout,
  dsi_profile_3gpp2_get_auth_timeout},         

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_CREQ_RETRY_COUNT,
  dsi_profile_3gpp2_set_lcp_creq_retry_count,
  dsi_profile_3gpp2_get_lcp_creq_retry_count},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_CREQ_RETRY_COUNT,
  dsi_profile_3gpp2_set_ipcp_creq_retry_count,
  dsi_profile_3gpp2_get_ipcp_creq_retry_count},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_RETRY_COUNT,
  dsi_profile_3gpp2_set_auth_retry_count,
  dsi_profile_3gpp2_get_auth_retry_count},
      
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PROTOCOL,
  dsi_profile_3gpp2_set_auth_protocol,
  dsi_profile_3gpp2_get_auth_protocol},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_USER_ID,
  dsi_profile_3gpp2_set_user_id,
  dsi_profile_3gpp2_get_user_id},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PASSWORD,
  dsi_profile_3gpp2_set_auth_password,
  dsi_profile_3gpp2_get_auth_password},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_RATE,
  dsi_profile_3gpp2_set_data_rate,
  dsi_profile_3gpp2_get_data_rate},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_MODE,
  dsi_profile_3gpp2_set_data_mode,
  dsi_profile_3gpp2_get_data_mode},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_APP_TYPE,
  dsi_profile_3gpp2_set_app_type,
  dsi_profile_3gpp2_get_app_type},
                 
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_APP_PRIORITY,
  dsi_profile_3gpp2_set_app_priority,
  dsi_profile_3gpp2_get_app_priority},            
                   
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_APN_STRING,
  dsi_profile_3gpp2_set_apn_string,
  dsi_profile_3gpp2_get_apn_string},                
                    
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_PDN_TYPE,
  dsi_profile_3gpp2_set_pdn_type,
  dsi_profile_3gpp2_get_pdn_type},                      
                          
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_IS_PCSCF_ADDR_NEEDED,
  dsi_profile_3gpp2_set_is_pcscf_addr_needed,
  dsi_profile_3gpp2_get_is_pcscf_addr_needed},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_PRIMARY,
  dsi_profile_3gpp2_set_v4_dns_addr_primary,
  dsi_profile_3gpp2_get_v4_dns_addr_primary},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_SECONDARY,
  dsi_profile_3gpp2_set_v4_dns_addr_secondary,
  dsi_profile_3gpp2_get_v4_dns_addr_secondary},
                 
  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_PRIMARY,
  dsi_profile_3gpp2_set_v6_dns_addr_primary,
  dsi_profile_3gpp2_get_v6_dns_addr_primary},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_SECONDARY,
  dsi_profile_3gpp2_set_v6_dns_addr_secondary,
  dsi_profile_3gpp2_get_v6_dns_addr_secondary},

  {(ds_profile_identifier_type)
      DS_PROFILE_3GPP2_PROFILE_PARAM_RAT_TYPE,
  dsi_profile_3gpp2_set_rat_type,
  dsi_profile_3gpp2_get_rat_type}

};

/*===========================================================================
FUNCTION GET_VALID_MASK_FROM_IDENT

DESCRIPTION
  This function returns the valid mask of the identifier.

PARAMETERS 
  param_id : identifier for which mask is to be returned 

DEPENDENCIES 
  
RETURN VALUE 
  returns the valid mask for the identifier
SIDE EFFECTS 
  
===========================================================================*/
/*lint -save -e656*/

ds_profile_3gpp2_valid_profile_enum_type get_valid_mask_from_ident( 
  ds_profile_identifier_type param_id 
)
{
  uint8 index = 0;
  for (index = 0; 
       index <= ((DS_PROFILE_3GPP2_PROFILE_PARAM_MAX - DS_PROFILE_3GPP2_PROFILE_PARAM_MIN)+1); 
       index++ )
  {
    if ( param_id == ds_profile_3gpp2_params_valid_mask[index].ident )
    {
      return ds_profile_3gpp2_params_valid_mask[index].valid_mask;
    }
  }
  return DS_PROFILE_3GPP2_PROFILE_INVALID;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_GET_INDEX_FROM_IDENT

DESCRIPTION
  This function gets index for the identifier to index into the function table.

PARAMETERS 
  ident : identifier for which index is to be returned

DEPENDENCIES 
  
RETURN VALUE 
  returns index
SIDE EFFECTS 
  
===========================================================================*/

uint8 dsi_profile_3gpp2_get_index_from_ident( 
  ds_profile_identifier_type ident
)
{
  uint8 index = 0;
  uint8 i     = 0;
  for (i = 0; 
       i <= ((DS_PROFILE_3GPP2_PROFILE_PARAM_MAX - DS_PROFILE_3GPP2_PROFILE_PARAM_MIN)+1); 
       i++ )
  {
    if (ident == ds_profile_3gpp2_profile_params_desc_tbl[i].uid)
    {
      index = i;
      break;
    }
  }
  return index;
}

/*lint -restore Restore lint error 656*/
/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_ALLOC_PROFILE
 
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

static void* dsi_profile_3gpp2_alloc_profile()
{
  dsi_profile_3gpp2_type *tmp_prf = NULL;
  DS_PROFILE_LOGD("_3gpp2_alloc_profile: ENTRY", 0 );
  
  tmp_prf = (dsi_profile_3gpp2_type *)DS_PROFILE_MEM_ALLOC(sizeof(dsi_profile_3gpp2_type),
                                                     MODEM_MEM_CLIENT_DATA);

  if (tmp_prf == NULL) 
  {
    DS_PROFILE_LOGE("_3gpp2_alloc_profile: FAILED DS_PROFILE_MEM_ALLOC", 0 );
    return NULL;
  }

  tmp_prf->prf = (ds_profile_3gpp2_profile_info_type *)DS_PROFILE_MEM_ALLOC(
      sizeof(ds_profile_3gpp2_profile_info_type),
      MODEM_MEM_CLIENT_DATA);

  if (tmp_prf->prf == NULL) 
  {
    DS_PROFILE_MEM_FREE( (void *)tmp_prf,
			MODEM_MEM_CLIENT_DATA);
    tmp_prf=NULL;
    DS_PROFILE_LOGE("_3gpp2_alloc_profile: FAILED DS_PROFILE_MEM_ALLOC", 0 );
    return NULL;
  }
  tmp_prf->mask = 0;
  tmp_prf->self = tmp_prf;
  DS_PROFILE_LOGD("_3gpp2_alloc_profile: EXIT with SUCCESS", 0);
  return tmp_prf;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_DEALLOC_PROFILE
 
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

static int dsi_profile_3gpp2_dealloc_profile(
  void *ptr
)
{
  DS_PROFILE_LOGD("_3gpp2_dealloc_profile: ENTRY", 0);

  if ( ptr == NULL || ( ((dsi_profile_3gpp2_type *)ptr)->prf == NULL ) ) 
  {
    DS_PROFILE_LOGE( "_3gpp2_dealloc_profile: ptr NULL", 0);
    DS_PROFILE_LOGE( "_3gpp2_dealloc_profile: EXIT with ERR", 0);
    return DSI_FAILURE;
  }

#ifdef FEATURE_DATA_MODEM_HEAP
  modem_mem_free( (void *)((dsi_profile_3gpp2_type *)ptr)->prf,
        MODEM_MEM_CLIENT_DATA ); 
  modem_mem_free( (void *)ptr,
                  MODEM_MEM_CLIENT_DATA );
#else
  free( (void *)((dsi_profile_3gpp2_type *)ptr)->prf);
  free( (void *)ptr);
#endif /* FEATURE_DATA_MODEM_HEAP */
  DS_PROFILE_LOGD("_3gpp2_dealloc_profile: EXIT with SUCCESS", 0);
  return DSI_SUCCESS;
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_SET_PARAM
 
DESCRIPTION
  This function is used to set 3GPP2 parameter value in the local copy
  of the profile

PARAMETERS 
  blob : pointer to local copy of profile
  ident : identifier whose value is to be set
  info : pointer to store value of identifier to be modified

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT
  DS_PROFILE_REG_3GPP2_ERR_INVALID_IDENT_FOR_PROFILE
  DS_PROFILE_REG_RESULT_ERR_LEN_INVALID
  DS_PROFILE_REG_RESULT_SUCCESS
 
SIDE EFFECTS 
  
===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp2_set_param( 
  void                        *blob,
  ds_profile_identifier_type   ident,
  const ds_profile_info_type  *info
)
{
  uint8  index = 0;
  uint32 mask  = 0;
  ds_profile_3gpp2_valid_profile_enum_type valid_mask = DS_PROFILE_3GPP2_PROFILE_INVALID;

  /* Validate identifier */
  if ( !DS_PROFILE_3GPP2_IDENT_IS_VALID( ident ) )
  {
    ident = DS_PROFILE_3GPP2_PROFILE_PARAM_INVALID;
    return DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT;
  }
  /* check if identifier is valid for this profile */
  valid_mask = get_valid_mask_from_ident( ident );
  if ( !(((dsi_profile_3gpp2_type *)blob)->prf->profile_type & valid_mask) )
  {
    return DS_PROFILE_REG_3GPP2_ERR_INVALID_IDENT_FOR_PROFILE;
  }

  index = dsi_profile_3gpp2_get_index_from_ident( ident );

  /* Validate info->buf and info->len */
  if ( !SET_INFO_IS_VALID( info, ds_profile_3gpp2_profile_params_desc_tbl[index].len ) ) 
  {
    return DS_PROFILE_REG_RESULT_ERR_LEN_INVALID;
  }
  /* get mask from identifier */
  CONVERT_IDENT_TO_MASK( mask, index );
  return ds_profile_3gpp2_acc_mut_fn_tbl[index].set_fn(blob, mask, info);
}

/*===========================================================================
FUNCTION DSI_PROFILE_3GPP2_GET_PARAM
 
DESCRIPTION
  This function is used to get 3GPP2 parameter value from the local copy
  of the profile

PARAMETERS 
  blob : pointer to local copy of profile
  ident : identifier to get value
  info : pointer to store value of identifier fetched

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT
  DS_PROFILE_REG_3GPP2_ERR_INVALID_IDENT_FOR_PROFILE
  DS_PROFILE_REG_RESULT_ERR_LEN_INVALID
  DS_PROFILE_REG_RESULT_SUCCESS
 
SIDE EFFECTS 
  
===========================================================================*/

static ds_profile_status_etype dsi_profile_3gpp2_get_param(
  void                        *blob,
  ds_profile_identifier_type   ident,
  ds_profile_info_type        *info
)
{
  uint8 index = 0;
  ds_profile_3gpp2_valid_profile_enum_type valid_mask = DS_PROFILE_3GPP2_PROFILE_INVALID;
  /* Validate identifier */
  if ( !DS_PROFILE_3GPP2_IDENT_IS_VALID( ident ) )
  {
    ident = DS_PROFILE_3GPP2_PROFILE_PARAM_INVALID; 
    return DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT;
  }
  /* check if identifier is valid for this profile */
  valid_mask = get_valid_mask_from_ident( ident );
  if ( !(((dsi_profile_3gpp2_type *)blob)->prf->profile_type & valid_mask) )
  {
    return DS_PROFILE_REG_3GPP2_ERR_INVALID_IDENT_FOR_PROFILE;
  }

  index = dsi_profile_3gpp2_get_index_from_ident( ident );
  /* Validate info->buf and info->len */
  if ( !GET_INFO_IS_VALID( info, ds_profile_3gpp2_profile_params_desc_tbl[index].len ) ) 
  {
    return DS_PROFILE_REG_RESULT_ERR_LEN_INVALID;
  }
  info->len = ds_profile_3gpp2_profile_params_desc_tbl[index].len;
  return ds_profile_3gpp2_acc_mut_fn_tbl[index].get_fn(blob, info);
}

/*===========================================================================
FUNCTION DS_PROFILE_3GPP2_INIT

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

uint8 ds_profile_3gpp2_init ( tech_fntbl_type *fntbl )
{
  DS_PROFILE_LOGD("3gpp2_init: ENTRY", 0);
  /* Init function pointers */
  fntbl->alloc     = dsi_profile_3gpp2_alloc_profile;
  fntbl->dealloc   = dsi_profile_3gpp2_dealloc_profile;
  fntbl->set_param = dsi_profile_3gpp2_set_param;
  fntbl->get_param = dsi_profile_3gpp2_get_param;

  DS_PROFILE_LOGD("3gpp2_init: EXIT with SUCCESS", 0);
  return (0x01 << DS_PROFILE_TECH_3GPP2);
}

/*lint -restore Restore lint error 655*/
/*lint -restore Restore lint error 641*/
