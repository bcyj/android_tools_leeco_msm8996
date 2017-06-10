/******************************************************************************
  @file    dsi_profile_3gpp2.h
  @brief   Definitions associated with profile related functions for 3GPP2. This
           is for internal use only

  DESCRIPTION
  

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

  $Header: //source/qcom/qct/modem/data/1x/dsprofile3gpp2/main/latest/inc/ds_profile_3gpp2i.h#3 $ $DateTime: 2009/10/23 14:44:55 $ $Author: mghotge $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/
#ifndef DS_PROFILE_3GPP2I_H
#define DS_PROFILE_3GPP2I_H

#include "ds_profile_3gpp2.h"
#include "ds_profile_tech_common.h"
#include "customer.h"
#include "qmi_wds_srvc.h"
#include "qmi_wds_utils.h"

/* Macro to check if identifier is valid */
#define DS_PROFILE_3GPP2_IDENT_IS_VALID( ident ) \
  ( ( ident >= DS_PROFILE_3GPP2_PROFILE_PARAM_MIN ) && \
    ( ident <= DS_PROFILE_3GPP2_PROFILE_PARAM_MAX ) )

typedef uint64 dsi_profile_3gpp2_mask_type;

/* internal implementation for list node */
typedef struct
{
  ds_profile_num_type   num;
  char  name[DS_PROFILE_3GPP2_MAX_PROFILE_NAME_LEN+1];
} ds_profile_3gpp2_list_info_type;

/* bitmask to specify validity of parameters in a profile */
typedef enum
{
  DS_PROFILE_3GPP2_PROFILE_INVALID     = 0,
  DS_PROFILE_3GPP2_PROFILE_COMMON      = 1 << 0, // ONLY COMMON PARAMS ARE VALID
  DS_PROFILE_3GPP2_PROFILE_KDDI        = 1 << 1, // ONLY KDDI PARAMS ARE VALID
  DS_PROFILE_3GPP2_PROFILE_EHRPD       = 1 << 2, // ONLY EHRPD PARAMS ARE VALID
  DS_PROFILE_3GPP2_PROFILE_OMH         = 1 << 3, // ONLY OMH PARAMS ARE VALID
  DS_PROFILE_3GPP2_PROFILE_MAX         = 1 << 30
}ds_profile_3gpp2_valid_profile_enum_type;

/* Internal data structure for storing data session profile parameter */
typedef struct
{
  ds_profile_3gpp2_profile_id           data_session_profile_id; 
                                    
  /* Data profile specific (common across all the builds) */
  /* ON or OFF default ON */
  ds_profile_3gpp2_negotiate_dns_server_type       negotiate_dns_server; 
  /* session close timer values need to be specified in sec */
  ds_profile_3gpp2_ppp_session_close_timer_DO_type ppp_session_close_timer_DO;
  ds_profile_3gpp2_ppp_session_close_timer_1X_type ppp_session_close_timer_1X;
  /* allowed or disallowed */
  ds_profile_3gpp2_allow_linger_type               allow_linger;            
                                      
  /* Expose only to KDDI though it is common to all carriers */
  /* Timeout values need to be specified in msec*/
  /* lcp C-Req Retry Timer */
  ds_profile_3gpp2_lcp_ack_timeout_type       lcp_ack_timeout; 
  /* ipcp C-Req Retry Timer */
  ds_profile_3gpp2_ipcp_ack_timeout_type      ipcp_ack_timeout; 
  /* PAP or CHAP Retry Timer */
  ds_profile_3gpp2_auth_timeout_type          auth_timeout; 
                                      
  /* number of retries */
  ds_profile_3gpp2_lcp_creq_retry_count_type  lcp_creq_retry_count; 
  ds_profile_3gpp2_ipcp_creq_retry_count_type ipcp_creq_retry_count; 
  ds_profile_3gpp2_auth_retry_count_type      auth_retry_count; 
                                      
  /* Which auth protocol is negotiated PAP or CHAP */
  ds_profile_3gpp2_auth_protocol_type         auth_protocol;
  char                                 user_id[DS_PROFILE_3GPP2_PPP_MAX_USER_ID_LEN];
  uint8                                user_id_len;
  char                                 auth_password[DS_PROFILE_3GPP2_PPP_MAX_PASSWD_LEN];
  uint8                                auth_password_len;
                                      
  /* high, medium or low */
  ds_profile_3gpp2_data_rate_type             data_rate; 
  /* hybrid or 1x */
  ds_profile_3gpp2_data_mode_type             data_mode; 
                                      
  /* OMH related params */            
  ds_profile_3gpp2_app_type_type              app_type;
  ds_profile_3gpp2_app_priority_type          app_priority;
                                      
  /* eHRPD related params */          
  char                                 apn_string[DS_PROFILE_3GPP2_APN_MAX_VAL_LEN];
  uint8                                apn_string_len;
  ds_profile_3gpp2_pdn_type_enum_type   pdn_type;
  ds_profile_3gpp2_is_pcscf_addr_needed_type  is_pcscf_addr_needed; 
  ds_profile_3gpp2_in_addr_type               v4_dns_addr[DS_PROFILE_3GPP2_MAX_NUM_DNS_ADDR];
  ds_profile_3gpp2_in6_addr_type              v6_dns_addr[DS_PROFILE_3GPP2_MAX_NUM_DNS_ADDR];
  ds_profile_3gpp2_rat_type_enum_type   rat_type;

  /* validity mask */
  ds_profile_3gpp2_valid_profile_enum_type profile_type; 
}ds_profile_3gpp2_profile_info_type;

/* Used internally to exchange 3GPP2 parameters */
typedef struct 
{
  dsi_profile_3gpp2_mask_type         mask;
  ds_profile_3gpp2_profile_info_type *prf;
  void                              *self;
}dsi_profile_3gpp2_type;

/* structure to store identifier and validity mask mapping */
typedef struct
{
  ds_profile_identifier_type ident;  
  ds_profile_3gpp2_valid_profile_enum_type valid_mask;  
}dsi_profile_3gpp2_params_valid_mask;

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

ds_profile_3gpp2_valid_profile_enum_type get_valid_mask_from_ident( 
  ds_profile_identifier_type param_id 
);

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
);

/*===========================================================================
FUNCTION DS_PROFILE_3GPP2_INIT

DESCRIPTION
  This function is called on the library init. It initializes the function
  table pointers to valid functions for 3gpp2

PARAMETERS 
  fntbl : pointer to function table

DEPENDENCIES 
  
RETURN VALUE 
  returns the mask for 3gpp2. (Used later as valid mask which is ORed value
  returned from all techs.
SIDE EFFECTS 
  
===========================================================================*/
uint8 ds_profile_3gpp2_init ( 
  tech_fntbl_type *fntbl 
);

extern void dsi_profile_get_profile_num_range (
  ds_profile_tech_etype tech,
  uint16               *min_num,
  uint16               *max_num
);

#endif /* DS_PROFILE_3GPP2I_H */
