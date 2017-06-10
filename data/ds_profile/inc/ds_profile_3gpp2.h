#ifndef DS_PROFILE_3GPP2_H
#define DS_PROFILE_3GPP2_H
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
                       D A T A   S E R V I C E S

              DATA SESSION PROFILE DATA STRUCTURE AND CONTROL FUNCTIONS
                       
GENERAL DESCRIPTION
  The file contains data structures to support 3GPP2 Application profiles 


   Copyright (c) 2009 by Qualcomm Technologies, Inc.  All Rights Reserved.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/


/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/modem/api/data/main/latest/ds_profile_3gpp2.h#1 $ $DateTime: 2009/09/30 19:08:30 $ $Author: vsheth $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

/*===========================================================================
                     INCLUDE FILES FOR MODULE
===========================================================================*/
#include "comdef.h"

/*===========================================================================
                        MACROS, TYPEDEFS AND VARIABLES
===========================================================================*/

#define ds_profile_3gpp2_profile_id  int32

/* Profile numbers for 3gpp2 */
#define DS_PROFILE_3GPP2_PROFILE_ID_INVALID               -1
#define DS_PROFILE_3GPP2_PROFILE_ID_MIN                    0
#define DS_PROFILE_3GPP2_DEFAULT_PROFILE_NUM               0
        
#define DS_PROFILE_3GPP2_JCDMA_INET_PROFILE                1
#define DS_PROFILE_3GPP2_EMAIL_WITH_ATTACHMENT_PROFILE     2
#define DS_PROFILE_3GPP2_EMAIL_WITHOUT_ATTACHMENT_PROFILE  3
#define DS_PROFILE_3GPP2_PSMS_PROFILE                      4
#define DS_PROFILE_3GPP2_BREW_PCSV_PROFILE                 5
#define DS_PROFILE_3GPP2_BREW_FA_PROFILE                   6
#define DS_PROFILE_3GPP2_BREW_OTHER_PROFILE                7
#define DS_PROFILE_3GPP2_BREW_IPVT_PROFILE                 8
#define DS_PROFILE_3GPP2_BREW_PTM_PROFILE                  9
#define DS_PROFILE_3GPP2_OTA_PROFILE                      10
#define DS_PROFILE_3GPP2_UHM_PROFILE                      11
#define DS_PROFILE_3GPP2_KEITAI_UPDATE_PROFILE            12
#define DS_PROFILE_3GPP2_WAP_FA_PROFILE                   13
#define DS_PROFILE_3GPP2_PSMS_CHAT_PROFILE                14
#define DS_PROFILE_3GPP2_CORP_PROFILE                     15
#define DS_PROFILE_3GPP2_BML_PROFILE                      16
#define DS_PROFILE_3GPP2_DEVICE_MANAGEMENT_PROFILE        17
#define DS_PROFILE_3GPP2_WINGP_WAP_PROFILE                18
#define DS_PROFILE_3GPP2_WINGP_EMAIL_PROFILE              19
#define DS_PROFILE_3GPP2_WINGP_PCSV_PROFILE               20
#define DS_PROFILE_3GPP2_WINGP_PSMS_PROFILE               21
#define DS_PROFILE_3GPP2_INTERNATIONAL_ROAMING_PROFILE    22
        
#define DS_PROFILE_3GPP2_JCDMA_NO_VALID_PROFILE           22
#define DS_PROFILE_3GPP2_JCDMA_PROFILE_MIN                1
#define DS_PROFILE_3GPP2_JCDMA_PROFILE_MAX                100

#define DS_PROFILE_3GPP2_EHRPD_PROFILE_1                  101
#define DS_PROFILE_3GPP2_EHRPD_PROFILE_2                  102
#define DS_PROFILE_3GPP2_EHRPD_PROFILE_3                  103
#define DS_PROFILE_3GPP2_EHRPD_PROFILE_4                  104
#define DS_PROFILE_3GPP2_EHRPD_PROFILE_5                  105
#define DS_PROFILE_3GPP2_EHRPD_PROFILE_6                  106
#define DS_PROFILE_3GPP2_EHRPD_PROFILE_7                  107

#define DS_PROFILE_3GPP2_EHRPD_NO_VALID_PROFILE           7
#define DS_PROFILE_3GPP2_EHRPD_PROFILE_MIN                101
#define DS_PROFILE_3GPP2_EHRPD_PROFILE_MAX                200
        
#define DS_PROFILE_3GPP2_PROFILE_ID_MAX                   30


/* 0 since profile names are not supported for 3GPP2, 
   used in list operations */
#define DS_PROFILE_3GPP2_MAX_PROFILE_NAME_LEN 0

/*  same as defined in ps_ppp_defs.h   */
#define DS_PROFILE_3GPP2_PPP_MAX_USER_ID_LEN       127
#define DS_PROFILE_3GPP2_PPP_MAX_PASSWD_LEN        127
                                       
#define DS_PROFILE_3GPP2_APN_MAX_VAL_LEN           100

/* Auth protocol */
#define DS_PROFILE_3GPP2_AUTH_PROTOCOL_PAP         1
#define DS_PROFILE_3GPP2_AUTH_PROTOCOL_CHAP        2
#define DS_PROFILE_3GPP2_AUTH_PROTOCOL_PAP_CHAP    3

/* 
   Data Rate
   For RUIM - Default Value = 2; High Speed.
*/
/* Low Speed: Low speed Service Options (SO15) only */
#define DS_PROFILE_3GPP2_DATARATE_LOW              0
/* Medium Speed: SO33 + low R-SCH */
#define DS_PROFILE_3GPP2_DATARATE_MED              1
/* High Speed: SO33 + high R-SCH */
#define DS_PROFILE_3GPP2_DATARATE_HIGH             2

/* 
   Data Bearer 
   For RUIM - Default Value = 0; Hybrid 1x/1xEV-DO.
*/
/* Hybrid 1x/1xEV-DO */
#define DS_PROFILE_3GPP2_DATAMODE_CDMA_HDR         0
/* 1x only */
#define DS_PROFILE_3GPP2_DATAMODE_CDMA_ONLY        1
/* 1xEV-DO only */
#define DS_PROFILE_3GPP2_DATAMODE_HDR_ONLY         2

/* DNS address macros */
#define DS_PROFILE_3GPP2_PRIMARY_DNS_ADDR          0
#define DS_PROFILE_3GPP2_SECONDARY_DNS_ADDR        1
#define DS_PROFILE_3GPP2_MAX_NUM_DNS_ADDR          2  

/* PDN Type */
typedef enum
{
  DS_PROFILE_3GPP2_PDN_TYPE_V4     = 0,
  DS_PROFILE_3GPP2_PDN_TYPE_V6     = 1,
  DS_PROFILE_3GPP2_PDN_TYPE_V4_V6  = 2,
  DS_PROFILE_3GPP2_PDN_TYPE_UNSPEC = 3,
  DS_PROFILE_3GPP2_PDN_TYPE_MAX    = 0xFF
}ds_profile_3gpp2_pdn_type_enum_type;

/* RAT (Radio Access Technology) Type */
typedef enum
{
  DS_PROFILE_3GPP2_RAT_TYPE_HRPD       = 0,
  DS_PROFILE_3GPP2_RAT_TYPE_EHRPD      = 1,
  DS_PROFILE_3GPP2_RAT_TYPE_HRPD_EHRPD = 2,
  DS_PROFILE_3GPP2_RAT_TYPE_MAX        = 0xFF
}ds_profile_3gpp2_rat_type_enum_type;

/*similar to in_addr and in6_addr structures defined in ps_in.h */
typedef struct ds_profile_3gpp2_in_addr
{
  uint32 ds_profile_3gpp2_s_addr;                                         
}ds_profile_3gpp2_in_addr_type;


typedef struct ds_profile_3gpp2_in6_addr
{
  union
  {
    uint8   ds_profile_3gpp2_u6_addr8[16];
    uint16  ds_profile_3gpp2_u6_addr16[8];
    uint32  ds_profile_3gpp2_u6_addr32[4];
    uint64  ds_profile_3gpp2_u6_addr64[2];
  } ds_profile_3gpp2_in6_u;
#define ds_profile_3gpp2_s6_addr	   ds_profile_3gpp2_in6_u.ds_profile_3gpp2_u6_addr8
#define ds_profile_3gpp2_s6_addr16  ds_profile_3gpp2_in6_u.ds_profile_3gpp2_u6_addr16
#define ds_profile_3gpp2_s6_addr32  ds_profile_3gpp2_in6_u.ds_profile_3gpp2_u6_addr32
#define ds_profile_3gpp2_s6_addr64  ds_profile_3gpp2_in6_u.ds_profile_3gpp2_u6_addr64
}ds_profile_3gpp2_in6_addr_type;

typedef boolean ds_profile_3gpp2_negotiate_dns_server_type;
typedef uint32  ds_profile_3gpp2_ppp_session_close_timer_DO_type;
typedef uint32  ds_profile_3gpp2_ppp_session_close_timer_1X_type;
typedef boolean ds_profile_3gpp2_allow_linger_type;
typedef uint16  ds_profile_3gpp2_lcp_ack_timeout_type;
typedef uint16  ds_profile_3gpp2_ipcp_ack_timeout_type;
typedef uint16  ds_profile_3gpp2_auth_timeout_type;
typedef uint8   ds_profile_3gpp2_lcp_creq_retry_count_type;
typedef uint8   ds_profile_3gpp2_ipcp_creq_retry_count_type;
typedef uint8   ds_profile_3gpp2_auth_retry_count_type;
typedef uint8   ds_profile_3gpp2_auth_protocol_type;
typedef uint8   ds_profile_3gpp2_data_rate_type; 
typedef uint8   ds_profile_3gpp2_data_mode_type;
typedef uint32  ds_profile_3gpp2_app_type_type;    
typedef uint8   ds_profile_3gpp2_app_priority_type;
typedef boolean ds_profile_3gpp2_is_pcscf_addr_needed_type; 

/* This list is used to identify Profile parameters and is technology specific */
typedef enum
{
  DS_PROFILE_3GPP2_PROFILE_PARAM_INVALID                 = 0x0,
  DS_PROFILE_3GPP2_PROFILE_PARAM_MIN                     = 0x10,

  DS_PROFILE_3GPP2_PROFILE_PARAM_NEGOTIATE_DNS_SERVER    = 0x10,
  DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_DO  = 0x11,
  DS_PROFILE_3GPP2_PROFILE_PARAM_SESSION_CLOSE_TIMER_1X  = 0x12,
  DS_PROFILE_3GPP2_PROFILE_PARAM_ALLOW_LINGER            = 0x13,
  DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_ACK_TIMEOUT         = 0x14,
  DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_ACK_TIMEOUT        = 0x15,
  DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_TIMEOUT            = 0x16,
  DS_PROFILE_3GPP2_PROFILE_PARAM_LCP_CREQ_RETRY_COUNT    = 0x17,
  DS_PROFILE_3GPP2_PROFILE_PARAM_IPCP_CREQ_RETRY_COUNT   = 0x18,
  DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_RETRY_COUNT        = 0x19, 
  DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PROTOCOL           = 0x1A, 
  DS_PROFILE_3GPP2_PROFILE_PARAM_USER_ID                 = 0x1B,
  DS_PROFILE_3GPP2_PROFILE_PARAM_AUTH_PASSWORD           = 0x1C,   
  DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_RATE               = 0x1D,
  DS_PROFILE_3GPP2_PROFILE_PARAM_DATA_MODE               = 0x1F,
  DS_PROFILE_3GPP2_PROFILE_PARAM_APP_TYPE                = 0x1E,
  DS_PROFILE_3GPP2_PROFILE_PARAM_APP_PRIORITY            = 0x20,
  DS_PROFILE_3GPP2_PROFILE_PARAM_APN_STRING              = 0x21, 
  DS_PROFILE_3GPP2_PROFILE_PARAM_PDN_TYPE                = 0x22,
  DS_PROFILE_3GPP2_PROFILE_PARAM_IS_PCSCF_ADDR_NEEDED    = 0x23,
  DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_PRIMARY     = 0x24,
  DS_PROFILE_3GPP2_PROFILE_PARAM_V4_DNS_ADDR_SECONDARY   = 0x25,
  DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_PRIMARY     = 0x26,
  DS_PROFILE_3GPP2_PROFILE_PARAM_V6_DNS_ADDR_SECONDARY   = 0x27,
  DS_PROFILE_3GPP2_PROFILE_PARAM_RAT_TYPE                = 0x28,

  DS_PROFILE_3GPP2_PROFILE_PARAM_MAX  = DS_PROFILE_3GPP2_PROFILE_PARAM_RAT_TYPE
}ds_profile_3gpp2_param_enum_type;

#endif /* DS_PROFILE_3GPP2_H */
