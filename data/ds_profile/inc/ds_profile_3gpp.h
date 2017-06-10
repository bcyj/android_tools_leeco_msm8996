/**************************************************************************
  @file    ds_profile_3gpp.h
  @brief   Definitions associated with profile related functions for UMTS

  DESCRIPTION
  

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  N/A

  -------------------------------------------------------------------------
  Copyright (C) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -------------------------------------------------------------------------
**************************************************************************/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/modem/api/data/main/latest/ds_profile_3gpp.h#2 $ $DateTime: 2009/10/21 22:45:47 $ $Author: vaibhavk $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/15/09   vk      Added support for LTE params
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

#ifndef DS_PROFILE_3GPP_H
#define DS_PROFILE_3GPP_H

#include "comdef.h"               

/*========================================================================
  Constant definitions
========================================================================*/

#define DS_PROFILE_3GPP_TFT_FILTER_ID1        0
#define DS_PROFILE_3GPP_TFT_FILTER_ID2        1
#define DS_PROFILE_3GPP_MAX_TFT_PARAM_SETS    2   /* Max no. of TFT param sets     */

#define DS_PROFILE_3GPP_MAX_APN_STRING_LEN    100 /* Max length of pdp context 
                                                     APN string                    */
#define DS_PROFILE_3GPP_MAX_QCPDP_STRING_LEN  127 /* Max len:username/password     */
		
#define DS_PROFILE_3GPP_MAX_PROFILE_NAME_LEN  15  /* Max len of profile name       */ 
#define DS_PROFILE_3GPP_MAX_OTAP_NAPID_LEN    48  /* Max len of OTAP NAPID         */

/*---------------------------------------------------------------------------
  Profile Family type. 
  Currently 3 profile families are supported -
  ATCOP Profiles for all externals calls
  Sockets Profiles for all embedded calls.
  RmNET profiles for all RmNET calls.
---------------------------------------------------------------------------*/
#define DS_PROFILE_3GPP_SOCKETS_PROFILE_FAMILY 1
#define DS_PROFILE_3GPP_ATCOP_PROFILE_FAMILY   2
#define DS_PROFILE_3GPP_RMNET_PROFILE_FAMILY   3

/* IP version. Max value 0xff is for internal use only */
typedef enum 
{
  DS_PROFILE_3GPP_IP_V4 = 4,
  DS_PROFILE_3GPP_IP_V6 = 6,
  DS_PROFILE_3GPP_IP_V4V6 = 10,
  DS_PROFILE_3GPP_IP_MAX = 0xff
}ds_profile_3gpp_ip_version_enum_type;

/*---------------------------------------------------------------------------
  The enum type for PDP Access Control 
  Max value 0xff is for INTERNAL USE ONLY
---------------------------------------------------------------------------*/
typedef enum 
{
  DS_PROFILE_3GPP_PDP_ACCESS_CONTROL_NONE       = 0x0,
  DS_PROFILE_3GPP_PDP_ACCESS_CONTROL_REJECT     = 0x1,
  DS_PROFILE_3GPP_PDP_ACCESS_CONTROL_PERMISSION = 0x2,
  DS_PROFILE_3GPP_PDP_ACCESS_CONTROL_MAX        = 0xff
}ds_profile_3gpp_pdp_access_control_e_type;

/*---------------------------------------------------------------------------
  The enum type for PDP Type 
  Max value 0xff is for INTERNAL USE ONLY
---------------------------------------------------------------------------*/
typedef enum
{
  DS_PROFILE_3GPP_PDP_IP = 0x0,                   /* PDP type IP                     */
  DS_PROFILE_3GPP_PDP_PPP,                        /* PDP type PPP                    */
  DS_PROFILE_3GPP_PDP_IPV6,                       /* PDP type IPV6                   */
  DS_PROFILE_3GPP_PDP_IPV4V6,                     /* PDP type IPV4V6                 */
  DS_PROFILE_3GPP_PDP_MAX = 0xff                  /* force max to 0xff so that enum 
                                                    is defined as a byte            */
}ds_profile_3gpp_pdp_type_enum_type;

/*-------------------------------------------------------------------------
  The enum type for Authentication Type for a PDP context
  Max value 0xff is for INTERNAL USE ONLY
--------------------------------------------------------------------------*/
typedef enum
{
  DS_PROFILE_3GPP_AUTH_PREF_PAP_CHAP_NOT_ALLOWED = 0x0,  /* No authentication       */
  DS_PROFILE_3GPP_AUTH_PREF_PAP_ONLY_ALLOWED     = 0x1,  /* PAP authentication      */
  DS_PROFILE_3GPP_AUTH_PREF_CHAP_ONLY_ALLOWED    = 0x2,  /* CHAP authentication     */
  DS_PROFILE_3GPP_AUTH_PREF_PAP_CHAP_ALLOWED     = 0x3,  /* PAP/CHAP authentication */
  DS_PROFILE_3GPP_AUTH_PREF_MAX                  = 0xff  /* force max to 0xff so that 
                                              enum is defined as a byte  */
} ds_profile_3gpp_auth_pref_type;

/*-------------------------------------------------------------------------
  The enum type for QOS Traffic Class Type
  Max value 0xff is for INTERNAL USE ONLY
--------------------------------------------------------------------------*/
typedef enum
{
  DS_PROFILE_3GPP_TC_SUBSCRIBED     = 0x0,        /* Subscribed                      */
  DS_PROFILE_3GPP_TC_CONVERSATIONAL = 0x1,        /* Conversational                  */
  DS_PROFILE_3GPP_TC_STREAMING      = 0x2,        /* Streaming                       */
  DS_PROFILE_3GPP_TC_INTERACTIVE    = 0x3,        /* Interactive                     */
  DS_PROFILE_3GPP_TC_BACKGROUND     = 0x4,        /* Background                      */
  DS_PROFILE_3GPP_TC_RESERVED       = 0xff        /* force max to 0xff so that
                                          enum is defined as a byte       */
}ds_profile_3gpp_traffic_class_type;

/*-------------------------------------------------------------------------
  The enum type for QOS Delivery Order Type
  Max value 0xff is for INTERNAL USE ONLY
---------------------------------------------------------------------------*/
typedef enum
{
  DS_PROFILE_3GPP_DO_SUBSCRIBED = 0x0,            /* Subscribed                      */
  DS_PROFILE_3GPP_DO_ON         = 0x1,            /* With delivery order             */
  DS_PROFILE_3GPP_DO_OFF        = 0x2,            /* Without delivery order          */
  DS_PROFILE_3GPP_DO_RESERVED   = 0xff            /* force max to 0xff so that
                                          enum is defined as a byte       */
}ds_profile_3gpp_qos_delivery_order_type;

/*-------------------------------------------------------------------------
  The enum type for QOS SDU Error Codes  (per 3GPP TS24.008 10.5.6.5)
--------------------------------------------------------------------------*/
typedef enum
{
  DS_PROFILE_3GPP_SDU_ERR_RATIO_SUBSCRIBE = 0x0,       /* Subscribed                 */
  DS_PROFILE_3GPP_SDU_ERR_RATIO_1ENEG2    = 0x1,       /* 1E-2  */
  DS_PROFILE_3GPP_SDU_ERR_RATIO_7ENEG3    = 0x2,       /* 7E-3  */
  DS_PROFILE_3GPP_SDU_ERR_RATIO_1ENEG3    = 0x3,       /* 1E-3  */
  DS_PROFILE_3GPP_SDU_ERR_RATIO_1ENEG4    = 0x4,       /* 1E-4  */
  DS_PROFILE_3GPP_SDU_ERR_RATIO_1ENEG5    = 0x5,       /* 1E-5  */
  DS_PROFILE_3GPP_SDU_ERR_RATIO_1ENEG6    = 0x6,       /* 1E-4  */
  DS_PROFILE_3GPP_SDU_ERR_RATIO_1ENEG1    = 0x7,       /* 1E-1  */
  DS_PROFILE_3GPP_SDU_ERR_RATIO_MAX       = 0x8,       /* Max Val                    */
  DS_PROFILE_3GPP_SDU_ERR_RESERVED        = 0xff       /* force max to 0xff so that
                                               enum is defined as a byte  */
}ds_profile_3gpp_sdu_err_ratio_type;

/*-------------------------------------------------------------------------
  The enum type for QOS RES BER Codes  (per 3GPP TS24.008 10.5.6.5)
  Max value 0xff is for INTERNAL USE ONLY
---------------------------------------------------------------------------*/
typedef enum
{
  DS_PROFILE_3GPP_RESIDUAL_BER_SUBSCRIBE  = 0x0,       /* Subscribed                  */
  DS_PROFILE_3GPP_RESIDUAL_BER_5ENEG2     = 0x1,       /* 5E-2 */
  DS_PROFILE_3GPP_RESIDUAL_BER_1ENEG2     = 0x2,       /* 1E-2 */
  DS_PROFILE_3GPP_RESIDUAL_BER_5ENEG3     = 0x3,       /* 5E-3 */
  DS_PROFILE_3GPP_RESIDUAL_BER_4ENEG3     = 0x4,       /* 4E-3 */
  DS_PROFILE_3GPP_RESIDUAL_BER_1ENEG3     = 0x5,       /* 1E-3 */
  DS_PROFILE_3GPP_RESIDUAL_BER_1ENEG4     = 0x6,       /* 1E-4 */
  DS_PROFILE_3GPP_RESIDUAL_BER_1ENEG5     = 0x7,       /* 1E-5 */
  DS_PROFILE_3GPP_RESIDUAL_BER_1ENEG6     = 0x8,       /* 1E-6 */
  DS_PROFILE_3GPP_RESIDUAL_BER_6ENEG8     = 0x9,       /* 6E-8 */
  DS_PROFILE_3GPP_RESIDUAL_BER_RESERVED   = 0xff       /* force max to 0xff so that
                                               enum is defined as a byte   */
}ds_profile_3gpp_residual_ber_ratio_type;

/*-------------------------------------------------------------------------
  The enum type for QOS Erroneous SDU Delivery Options 
  (per 3GPP TS2.008 10.5.6.5): Max value 0xff is for INTERNAL USE ONLY
--------------------------------------------------------------------------*/
typedef enum
{
  DS_PROFILE_3GPP_DELIVER_ERR_SDU_SUBSCRIBE  = 0x0,    /* Subscribed                  */
  DS_PROFILE_3GPP_DELIVER_ERR_SDU_NO_DETECT  = 0x1,    /* No detection                */
  DS_PROFILE_3GPP_DELIVER_ERR_SDU_DELIVER    = 0x2,    /* Erroneous SDU is delivered  */
  DS_PROFILE_3GPP_DELIVER_ERR_SDU_NO_DELIVER = 0x3,    /* Erroneous SDU not delivered */
  DS_PROFILE_3GPP_DELIVER_ERR_SDU_RESERVED   = 0xff    /* force max to 0xff so that
                                               enum is defined as a byte   */
}ds_profile_3gpp_deliver_err_sdu_type;

/*-------------------------------------------------------------------------
  Structure for IPV4/IPv6 addresses 
---------------------------------------------------------------------------*/
typedef PACKED struct PACKED_POST
{
  PACKED union PACKED_POST
  {
    uint8   u6_addr8[16]; 
    uint16  u6_addr16[8]; 
    uint32  u6_addr32[4]; 
    uint64  u6_addr64[2]; 
  }in6_u;
}ds_profile_3gpp_pdp_addr_type_ipv6;
 
/*-------------------------------------------------------------------------
  Generic type to hold IPv4 or IPv6 addresses 
---------------------------------------------------------------------------*/
typedef PACKED struct PACKED_POST
{
  PACKED struct PACKED_POST
  {
    uint32 ds_profile_3gpp_pdp_addr_ipv4;  
    ds_profile_3gpp_pdp_addr_type_ipv6 ds_profile_3gpp_pdp_addr_ipv6; 
  }ds_profile_3gpp_pdp_addr;
#define ds_profile_3gpp_pdp_addr_ipv4 ds_profile_3gpp_pdp_addr.ds_profile_3gpp_pdp_addr_ipv4
#define ds_profile_3gpp_pdp_addr_ipv6 ds_profile_3gpp_pdp_addr.ds_profile_3gpp_pdp_addr_ipv6
}ds_profile_3gpp_pdp_addr_type;

/*----------------------------------------------------------------------------
   PDP Header and Data compression enums
-----------------------------------------------------------------------------*/

/* PDP header compression types */
typedef PACKED enum 
{
  DS_PROFILE_3GPP_PDP_HEADER_COMP_OFF = 0,   /* PDP header compression is OFF.            */
  DS_PROFILE_3GPP_PDP_HEADER_COMP_ON  = 1,   /* Manufacturer preferred compression.       */
  DS_PROFILE_3GPP_PDP_HEADER_COMP_RFC1144,   /* PDP header compression based on rfc 1144. */
  DS_PROFILE_3GPP_PDP_HEADER_COMP_RFC2507,   /* PDP header compression based on rfc 2507. */
  DS_PROFILE_3GPP_PDP_HEADER_COMP_RFC3095,   /* PDP header compression based on rfc 3095. */   
  DS_PROFILE_3GPP_PDP_HEADER_COMP_MAX =0xFF  /* force max to 0xff so that
                                      enum is defined as a byte                */
} PACKED_POST ds_profile_3gpp_pdp_header_comp_e_type;


/* PDP Data compression types defined in TS 44.065 (sect 6.6.1.1.4)*/
typedef PACKED enum 
{
  DS_PROFILE_3GPP_PDP_DATA_COMP_OFF = 0,     /* PDP Data compression is OFF               */
  DS_PROFILE_3GPP_PDP_DATA_COMP_ON  = 1,     /* Manufacturer preferred compression.       */
  DS_PROFILE_3GPP_PDP_DATA_COMP_V42_BIS,     /* V.42BIS data compression                  */
  DS_PROFILE_3GPP_PDP_DATA_COMP_V44,         /* V.44 data compression                     */  
  DS_PROFILE_3GPP_PDP_DATA_COMP_MAX = 0xFF   /* force max to 0xff so that
                                      enum is defined as a byte                */
} PACKED_POST ds_profile_3gpp_pdp_data_comp_e_type;

/* IPv4 addr alloc mechanism */
typedef PACKED enum
{
  DS_PROFILE_3GPP_PDP_IPV4_ADDR_ALLOC_NAS = 0,    /* Addr alloc using NAS           */
  DS_PROFILE_3GPP_PDP_IPV4_ADDR_ALLOC_DHCPV4 = 1, /* Addr alloc using DHCPv4        */
  DS_PROFILE_3GPP_PDP_IPV4_ADDR_ALLOC_MAX
} PACKED_POST ds_profile_3gpp_pdp_ipv4_addr_alloc_e_type;

/* LTE QCI values */
typedef PACKED enum
{
  DS_PROFILE_3GPP_LTE_QCI_0 = 0,
  DS_PROFILE_3GPP_LTE_QCI_1 = 1, 
  DS_PROFILE_3GPP_LTE_QCI_2 = 2,
  DS_PROFILE_3GPP_LTE_QCI_3 = 3, 
  DS_PROFILE_3GPP_LTE_QCI_4 = 4,
  DS_PROFILE_3GPP_LTE_QCI_5 = 5, 
  DS_PROFILE_3GPP_LTE_QCI_6 = 6,
  DS_PROFILE_3GPP_LTE_QCI_7 = 7, 
  DS_PROFILE_3GPP_LTE_QCI_8 = 8,
  DS_PROFILE_3GPP_LTE_QCI_9 = 9, 
  DS_PROFILE_3GPP_LTE_QCI_INVALID
} PACKED_POST ds_profile_3gpp_lte_qci_e_type;

typedef byte ds_profile_3gpp_pdp_context_number_type;
typedef boolean ds_profile_3gpp_pdp_context_secondary_flag_type;
typedef byte ds_profile_3gpp_pdp_context_primary_id_type;
typedef uint32   ds_profile_3gpp_pdp_addr_type_ipv4;
typedef boolean  ds_profile_3gpp_request_pcscf_address_flag_type;
typedef boolean  ds_profile_3gpp_request_pcscf_address_using_dhcp_flag_type;
typedef boolean  ds_profile_3gpp_im_cn_flag_type;

/*---------------------------------------------------------------------------
  Structure to store PDP authentication parameters
---------------------------------------------------------------------------*/
typedef PACKED struct PACKED_POST
{
  ds_profile_3gpp_auth_pref_type  
            auth_type;                             /* Authentication type      */
  byte      password[DS_PROFILE_3GPP_MAX_QCPDP_STRING_LEN+1]; /* Passw/secret string      */
  byte      username[DS_PROFILE_3GPP_MAX_QCPDP_STRING_LEN+1]; /* Username string          */
}ds_profile_3gpp_pdp_auth_type;

/*---------------------------------------------------------------------------
  Structure to store UMTS Quality of Service  parameters.
---------------------------------------------------------------------------*/
typedef PACKED struct PACKED_POST
{
  ds_profile_3gpp_traffic_class_type 
              traffic_class;   /* Traffic Class         */
  uint32      max_ul_bitrate; /* Maximum UL bitrate    */
  uint32      max_dl_bitrate; /* Maximum DL bitrate    */
  uint32      gtd_ul_bitrate; /* Guaranteed UL bitrate */
  uint32      gtd_dl_bitrate; /* Guaranteed DL bitrate */
  ds_profile_3gpp_qos_delivery_order_type 
              dlvry_order;     /* SDU delivery order    */
  uint32      max_sdu_size;   /* Maximum SDU size      */
  ds_profile_3gpp_sdu_err_ratio_type      
              sdu_err;         /* SDU error ratio index */
  ds_profile_3gpp_residual_ber_ratio_type 
              res_biterr;      /* Residual bit err index*/
  ds_profile_3gpp_deliver_err_sdu_type    
              dlvr_err_sdu;    /* Delivery of err SDU   */
  uint32      trans_delay;    /* Transfer Delay        */
  uint32      thandle_prio;    /* Traffic handling prio */
  boolean     sig_ind;        /* Signalling Indication Flag    */
}ds_profile_3gpp_3gpp_qos_params_type;

/*-------------------------------------------------------------------------
  Structure to store GPRS Quality of Service  parameters.
---------------------------------------------------------------------------*/
typedef PACKED struct PACKED_POST
{
  uint32      precedence;  /* Precedence class             */
  uint32      delay;       /* Delay class                  */
  uint32      reliability; /* Reliability class            */
  uint32      peak;        /* Peak throughput class        */
  uint32      mean;        /* Mean throughput class        */
}ds_profile_3gpp_gprs_qos_params_type;

/*---------------------------------------------------------------------------
  Structure to store LTE Quality of Service  parameters.
---------------------------------------------------------------------------*/
typedef PACKED struct PACKED_POST
{
  ds_profile_3gpp_lte_qci_e_type    qci;             /* QCI Value                    */
  uint32                            g_dl_bit_rate;   /* Gauranteed DL bit rate       */
  uint32                            max_dl_bit_rate; /* Maximum DL bit rate          */
  uint32                            g_ul_bit_rate;   /* Gauranteed UL bit rate       */
  uint32                            max_ul_bit_rate; /* Maximum UL bit rate          */
} ds_profile_3gpp_lte_qos_params_type;

/*---------------------------------------------------------------------------
  Structure to store DNS parameters for the PDP profile
---------------------------------------------------------------------------*/
typedef PACKED struct PACKED_POST
{
  ds_profile_3gpp_pdp_addr_type        primary_dns_addr;   /* primary DNS address   */
  ds_profile_3gpp_pdp_addr_type        secondary_dns_addr; /* secondary DNS address */
}ds_profile_3gpp_dns_addresses_type;
  
/*---------------------------------------------------------------------------
  Structure to store Traffic Flow Template (TFT) parameters.
---------------------------------------------------------------------------*/
typedef PACKED struct PACKED_POST
{
  PACKED union PACKED_POST
  {
    uint32 ds_profile_3gpp_tft_addr_ipv4;  
    ds_profile_3gpp_pdp_addr_type_ipv6 ds_profile_3gpp_tft_addr_ipv6; 
  }ds_profile_3gpp_tft_addr;
#define ds_profile_3gpp_tft_addr_ipv4 ds_profile_3gpp_tft_addr.ds_profile_3gpp_tft_addr_ipv4
#define ds_profile_3gpp_tft_addr_ipv6 ds_profile_3gpp_tft_addr.ds_profile_3gpp_tft_addr_ipv6
}ds_profile_3gpp_tft_addr_type;

typedef PACKED struct PACKED_POST
{
  ds_profile_3gpp_tft_addr_type   address;  /* IPV4 or IPV6 Address        */
  uint8                           mask;     /* IPV4 or IPV6 Subnet mask    */
}ds_profile_3gpp_address_mask_type;

typedef PACKED struct PACKED_POST
{
  uint16      from; /* Range lower limit */
  uint16      to;   /* Range upper limit */
} ds_profile_3gpp_port_range_type;

typedef PACKED struct PACKED_POST
{
  byte                               filter_id;       /* Filter identifier           */
  byte                               eval_prec_id;    /* Evaluation precedence index */
  ds_profile_3gpp_ip_version_enum_type ip_version;    /* IP version for address      */
  ds_profile_3gpp_address_mask_type  src_addr;   /* Source address & mask       */
  byte                prot_num;      /* Protocol number => next_header in IPv6*/
  ds_profile_3gpp_port_range_type    dest_port_range; /* Destination port range      */
  ds_profile_3gpp_port_range_type    src_port_range;  /* Source port range           */
  uint32                             ipsec_spi;       /* Security parameter index    */
  uint16              tos_mask;      /* Type of srvc & mask => tclass in  IPv6*/
  uint32                             flow_label;      /* Flow label                 */
}ds_profile_3gpp_tft_params_type;

/*-------------------------------------------------------------------------
This list is used to identify Profile parameters and is technology specific 
--------------------------------------------------------------------------*/
typedef enum
{
  DS_PROFILE_3GPP_PROFILE_PARAM_INVALID                    = 0, 
  DS_PROFILE_3GPP_PROFILE_PARAM_MIN                        = 0x10,
  DS_PROFILE_3GPP_PROFILE_PARAM_PROFILE_NAME               = 0x10,
  DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_TYPE       = 0x11,
  DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_H_COMP         = 0x12,
  DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_D_COMP         = 0x13,
  DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_APN            = 0x14,
  DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V4_PRIMARY        = 0x15,
  DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V4_SECONDARY      = 0x16,
  DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_REQ_QOS               = 0x29,
  DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_MIN_QOS               = 0x2A,
  DS_PROFILE_3GPP_PROFILE_PARAM_GPRS_REQ_QOS               = 0x19,
  DS_PROFILE_3GPP_PROFILE_PARAM_GPRS_MIN_QOS               = 0x1A,
  DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_USERNAME              = 0x1B,
  DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_PASSWORD              = 0x1C,
  DS_PROFILE_3GPP_PROFILE_PARAM_AUTH_TYPE                  = 0x1D,
  DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_ADDR_V4    = 0x1E,
  DS_PROFILE_3GPP_PROFILE_PARAM_PCSCF_REQ_FLAG             = 0x1F,
  DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_TE_MT_ACCESS_CTRL_FLAG = 0x20,
  DS_PROFILE_3GPP_PROFILE_PARAM_PCSCF_DHCP_REQ_FLAG        = 0x21,
  DS_PROFILE_3GPP_PROFILE_PARAM_IM_CN_FLAG                 = 0x22,
  DS_PROFILE_3GPP_PROFILE_PARAM_TFT_FILTER_ID1             = 0x23,
  DS_PROFILE_3GPP_PROFILE_PARAM_TFT_FILTER_ID2             = 0x24,
  DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_NUMBER         = 0x25,
  DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_SECONDARY_FLAG = 0x26,
  DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PRIMARY_ID     = 0x27,
  DS_PROFILE_3GPP_PROFILE_PARAM_PDP_CONTEXT_PDP_ADDR_V6    = 0x28,
  /* Values 0x17 and 0x18 are reserved for internal use */
  DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_REQ_QOS_EXTENDED      = 0x17, 
  DS_PROFILE_3GPP_PROFILE_PARAM_UMTS_MIN_QOS_EXTENDED      = 0x18,
  DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V6_PRIMARY        = 0x2B,
  DS_PROFILE_3GPP_PROFILE_PARAM_DNS_ADDR_V6_SECONDARY      = 0x2C,
  DS_PROFILE_3GPP_PROFILE_PARAM_IPV4_ADDR_ALLOC            = 0x2D,
  DS_PROFILE_3GPP_PROFILE_PARAM_LTE_REQ_QOS                = 0x2E,
  DS_PROFILE_3GPP_PROFILE_PARAM_MAX = DS_PROFILE_3GPP_PROFILE_PARAM_LTE_REQ_QOS
}ds_profile_3gpp_param_enum_type;

#endif /* DS_PROFILE_3GPP_H */
