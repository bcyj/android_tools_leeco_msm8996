/******************************************************************************
  @file    ds_profile_3gppi.h
  @brief   Definitions associated with profile related functions for 3GPP. This
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

  $Header: //source/qcom/qct/modem/data/umts/dsprofile3gpp/main/latest/inc/ds_profile_3gppi.h#4 $ $DateTime: 2009/10/23 14:44:55 $ $Author: mghotge $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/15/09   vk      Added support for LTE params
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/
#ifndef DS_PROFILE_3GPPI_H
#define DS_PROFILE_3GPPI_H

#include "ds_profile_3gpp.h"
#include "ds_profile_tech_common.h"
#include "customer.h"
#include "qmi_wds_srvc.h"
#include "qmi_wds_utils.h"

#define DS_PROFILE_3GPP_PROFILE_NUM_MIN 1

/* Macro to check if identifier is valid */
#define DS_PROFILE_3GPP_IDENT_IS_VALID( ident ) \
  ( ( ident >= DS_PROFILE_3GPP_PROFILE_PARAM_MIN ) && \
    ( ident <= DS_PROFILE_3GPP_PROFILE_PARAM_MAX ) )

/* Internal list node */
typedef struct
{
  ds_profile_num_type   prf_num;
  char                  prf_name[DS_PROFILE_3GPP_MAX_PROFILE_NAME_LEN+1];
} ds_profile_3gpp_list_info_type;

typedef uint64 dsi_profile_3gpp_mask_type;

typedef PACKED struct PACKED_POST
{
  ds_profile_3gpp_pdp_context_number_type pdp_context_number; 
                                                     /* same as profile number */
  ds_profile_3gpp_pdp_type_enum_type      pdp_type;  /* PDP type (IP/PPP)      */
  ds_profile_3gpp_pdp_header_comp_e_type  h_comp;    /* PDP Header Comp support*/
  ds_profile_3gpp_pdp_data_comp_e_type    d_comp;    /* Data Comp       support*/
  ds_profile_3gpp_pdp_addr_type           pdp_addr;  /* PDP address            */ 
  byte                                    apn[DS_PROFILE_3GPP_MAX_APN_STRING_LEN+1];          
                                                    /* APN string              */
  ds_profile_3gpp_pdp_context_secondary_flag_type  secondary_flag;                         
                                                    /* this is a sec profile   */
  ds_profile_3gpp_pdp_context_primary_id_type      primary_id;
                                                    /* link to primary profile */ 
  ds_profile_3gpp_pdp_access_control_e_type	access_ctrl_flag;
                                                    /* TE MT PDP access cntrl  */
  ds_profile_3gpp_pdp_ipv4_addr_alloc_e_type ipv4_addr_alloc; 
                                                    /* IPv4 addr alloc mech.   */
}ds_profile_3gpp_pdp_context_type;


/*---------------------------------------------------------------------------
  PDP Profile complete structure definition
---------------------------------------------------------------------------*/
typedef PACKED struct PACKED_POST
{
  byte                                 version;          /* Version identifier */
  boolean                              read_only_flag;   /* Is context read-only*/
  char                                 profile_name[DS_PROFILE_3GPP_MAX_PROFILE_NAME_LEN+1];
                                                         /* Profile name in    */
  boolean ds_profile_3gpp_context_valid_flg;
  ds_profile_3gpp_ip_version_enum_type pdp_addr_ip_vsn;
  ds_profile_3gpp_pdp_context_type     context;          /* Context definition */
  
  ds_profile_3gpp_pdp_auth_type        auth;             /* Authentication info*/

  boolean ds_profile_3gpp_qos_req_3gpp_valid_flg;
  boolean ds_profile_3gpp_qos_min_3gpp_valid_flg;
  boolean ds_profile_3gpp_qos_req_gprs_valid_flg;   
  boolean ds_profile_3gpp_qos_min_gprs_valid_flg;
  boolean ds_profile_3gpp_qos_req_lte_valid_flg;
  ds_profile_3gpp_3gpp_qos_params_type qos_request_3gpp; /* 3GPP QOS params:req*/
  ds_profile_3gpp_3gpp_qos_params_type qos_minimum_3gpp; /* 3GPP QOS params:min*/
  ds_profile_3gpp_gprs_qos_params_type qos_request_gprs; /* GPRS QOS Params:req*/
  ds_profile_3gpp_gprs_qos_params_type qos_minimum_gprs; /* GPRS QOS params:min*/
  ds_profile_3gpp_lte_qos_params_type  qos_request_lte;  /* LTE QOS params:req */

  ds_profile_3gpp_ip_version_enum_type dns_addr_primary_ip_vsn;
  ds_profile_3gpp_ip_version_enum_type dns_addr_secondary_ip_vsn;
  ds_profile_3gpp_dns_addresses_type   dns_addr;         /* DNS addr.-user 
                                                            specfied           */
  boolean ds_profile_3gpp_tft_valid_flg[DS_PROFILE_3GPP_MAX_TFT_PARAM_SETS]; 
  ds_profile_3gpp_tft_params_type      tft[DS_PROFILE_3GPP_MAX_TFT_PARAM_SETS];
                                                         /* Traffic Flow 
                                                            Template           */
  boolean                        otap_enabled_flag; /* Is context OTA      */
                                                    /* provisionable       */
  byte                           otap_napid[DS_PROFILE_3GPP_MAX_OTAP_NAPID_LEN+1];
                                        /* Network Access Point Identifier */
                                        /* in UTF8 format with variable    */
                                        /* length encoding                 */
  boolean                              request_pcscf_address_flag; 
                                               /* Flag to indicate if pcscf */
                                               /* address should be         */
                                               /* requested in PCO for      */
                                               /* this profile.             */
  boolean                              request_pcscf_address_using_dhcp_flag;
                                               /* Flag to indicate if pcscf */
                                               /* address should be         */
                                               /* requested using DHCP for  */
                                               /* this profile.             */
  boolean                              im_cn_flag; 
                                               /* Flag to indicate if im_cn */
                                               /* flag should be requested  */
                                               /* for this profile.         */
}ds_profile_3gpp_profile_info_type;

/* Used internally to exchange 3GPP parameters */
typedef struct 
{
  dsi_profile_3gpp_mask_type         mask;
  ds_profile_3gpp_profile_info_type *prf;
  void                              *self;
}dsi_profile_3gpp_type;

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
uint8 dsi_profile_3gpp_get_index_from_ident( 
  ds_profile_identifier_type ident
);

/*===========================================================================
FUNCTION DS_PROFILE_3GPP_INIT

DESCRIPTION
  This function is called on the library init. It initializes the function
  table pointers to valid functions for 3gpp

PARAMETERS 
  fntbl : pointer to function table

DEPENDENCIES 
  
RETURN VALUE 
  returns the mask for 3gpp. (Used later as valid mask which is ORed value
  returned from all techs.
SIDE EFFECTS 
  
===========================================================================*/
uint8 ds_profile_3gpp_init( 
  tech_fntbl_type *fntbl 
);

extern void dsi_profile_get_profile_num_range (
  ds_profile_tech_etype tech,
  uint16               *min_num,
  uint16               *max_num
);

#endif /* DS_PROFILE_3GPPI_H */
