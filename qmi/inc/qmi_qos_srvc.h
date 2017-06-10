#ifndef QMI_QOS_SRVC_H
#define QMI_QOS_SRVC_H
/******************************************************************************
  @file    qmi_qos_srvc.h
  @brief   QMI message library QoS service definitions

  DESCRIPTION
  This file contains common, external header file definitions for QMI
  interface library.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_qos_srvc_init_client() must be called to create one or more clients
  qmi_qos_srvc_release_client() must be called to delete each client when
  finished.

  $Header: //source/qcom/qct/modem/datacommon/qmimsglib/dev/work/inc/qmi_qos_srvc.h#3 $
  $DateTime: 2009/07/15 10:38:12 $
  ---------------------------------------------------------------------------
  Copyright (c) 2007 - 2012,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#include "qmi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define QMI_QOS_MAX_FLOW_FILTER     16
#define QMI_QOS_MAX_INDS          QMI_QOS_MAX_FLOW_FILTER
#define QMI_QOS_MAX_FLOW_EVENTS   8
#define QMI_QOS_MAX_PROFILES      8 //TODO: ?
#define QMI_QOS_MAX_ERR_CODES_IN_RSP    10 // ? TODO : FIND OUT HOW MANY ?
#define QMI_QOS_IPV6_ADDR_SIZE_IN_BYTES 16

typedef unsigned char qmi_qos_ipv6_addr_type[QMI_QOS_IPV6_ADDR_SIZE_IN_BYTES];

/* typedefs related to flow descriptors */
typedef enum
{
  QMI_QOS_UMTS_TC_CONVERSATIONAL = 0,
  QMI_QOS_UMTS_TC_STREAMING      = 1,
  QMI_QOS_UMTS_TC_INTERACTIVE    = 2,
  QMI_QOS_UMTS_TC_BACKGROUND     = 3
} qmi_qos_umts_traffic_class_type;

typedef enum
{
  QMI_QOS_UMTS_BER_5X10_NEG2 = 0,
  QMI_QOS_UMTS_BER_1X10_NEG2 = 1,
  QMI_QOS_UMTS_BER_5X10_NEG3 = 2,
  QMI_QOS_UMTS_BER_4X10_NEG3 = 3,
  QMI_QOS_UMTS_BER_1X10_NEG3 = 4,
  QMI_QOS_UMTS_BER_1X10_NEG4 = 5,
  QMI_QOS_UMTS_BER_1X10_NEG5 = 6,
  QMI_QOS_UMTS_BER_1X10_NEG6 = 7,
  QMI_QOS_UMTS_BER_6X10_NEG8 = 8
} qmi_qos_umts_residual_ber_type;


typedef enum
{
  QMI_QOS_UMTS_PRIO_1 = 0,
  QMI_QOS_UMTS_PRIO_2 = 1,
  QMI_QOS_UMTS_PRIO_3 = 2
} qmi_qos_umts_handling_prio_type;


typedef enum
{
  QMI_QOS_UMTS_TECH_TYPE,
  QMI_QOS_CDMA_TECH_TYPE,
  QMI_QOS_DVBH_TECH_TYPE,
  QMI_QOS_MBMS_TECH_TYPE,
  QMI_QOS_BCMCS_TECH_TYPE,
  QMI_QOS_MEDIAFLO_TECH_TYPE,
  QMI_QOS_INVALID_TECH_TYPE = 0xFFFF
}qmi_qos_technology_type;

typedef int             qmi_qos_dss_errno;
#define QMI_QOS_EXTENDED_ERROR_INFO_PARAM       0x0001

typedef struct
{
  unsigned short              param_mask;
  qmi_qos_dss_errno           dss_errno;
}qmi_qos_dss_errno_type;


typedef struct
{
  unsigned long max_rate;
  unsigned long guaranteed_rate;
} qmi_qos_umts_data_rate_type;


typedef struct
{
  unsigned long peak_rate;
  unsigned long token_rate;
  unsigned long bucket_size;
} qmi_qos_umts_token_bucket_type;

typedef struct
{
  unsigned long multiplier;
  unsigned long exponent;
} qmi_qos_umts_packet_err_rate_type;

typedef enum
{
  QMI_QOS_FALSE   = 0x00,
  QMI_QOS_TRUE    = 0x01
}qmi_qos_bool_type;

#define QMI_QOS_UMTS_FLOW_PARAM_TRAFFIC_CLASS      0x0001
#define QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE          0x0002
#define QMI_QOS_UMTS_FLOW_PARAM_BUCKET_INFO        0x0004
#define QMI_QOS_UMTS_FLOW_PARAM_MAX_DELAY          0x0008
#define QMI_QOS_UMTS_FLOW_PARAM_MAX_JITTER         0x0010
#define QMI_QOS_UMTS_FLOW_PARAM_PKT_ERR_RATE       0x0020
#define QMI_QOS_UMTS_FLOW_PARAM_MIN_POL_PKT_SZ     0x0040
#define QMI_QOS_UMTS_FLOW_PARAM_MAX_ALLOW_PKT_SZ   0x0080
#define QMI_QOS_UMTS_FLOW_PARAM_RESIDUAL_BER       0x0100
#define QMI_QOS_UMTS_FLOW_PARAM_HANDLING_PRIO      0x0200
#define QMI_QOS_UMTS_FLOW_PARAM_3GPP2_FLOW_PRIO    0x0400
#define QMI_QOS_UMTS_FLOW_PARAM_IM_CN_FLAG         0x0800
#define QMI_QOS_UMTS_FLOW_PARAM_IP_FLOW_SIG_IND    0x1000
#define QMI_QOS_LTE_FLOW_PARAM_QCI_IND             0x2000

#define QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID         0x0001

/* This bit definition is overloaded.  But since this is a CDMA
** definition which is exclusive of the UMTS definitions, this
** is OK.  This is only used in the error reporting mask, not
** in QoS initiation, sice the CMDA profile ID is a required
** parameter for CDMA technology
*/
#define QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID_ERR  QMI_QOS_UMTS_FLOW_PARAM_TRAFFIC_CLASS


typedef struct
{
  /* Mask of which optional parameters below are valid */
  unsigned short param_mask;

  /* UMTS optional QoS parameters.  At least one of these
  ** must be included in a setup request
  */
  qmi_qos_umts_traffic_class_type     traffic_class;
  qmi_qos_umts_data_rate_type         data_rate;
  qmi_qos_umts_token_bucket_type      bucket_info;
  unsigned long                       max_delay;
  unsigned long                       max_jitter;
  qmi_qos_umts_packet_err_rate_type   pkt_err_rate;
  unsigned long                       min_policed_pkt_sz;
  unsigned long                       max_allowed_pkt_sz;
  qmi_qos_umts_residual_ber_type      residual_ber;
  qmi_qos_umts_handling_prio_type     handling_prio;
  unsigned char                       flow_priority_3gpp2;
  qmi_qos_bool_type                   im_cn_flag;/*Valid only for 3gpp*/
  qmi_qos_bool_type                   ip_flow_sig_ind;/*valid only for 3gpp*/
  unsigned char                       lte_qci;
} qmi_qos_umts_flow_desc_type;

typedef struct
{
  /* Mask of which optional parameters below are valid */
  unsigned short param_mask;

  unsigned long profile_id;
} qmi_qos_cdma_flow_desc_type;

typedef struct
{
  qmi_qos_cdma_flow_desc_type   cdma_flow_desc;
  qmi_qos_umts_flow_desc_type   umts_flow_desc;
} qmi_qos_flow_req_type;


/* typedefs related to filter descriptors */
typedef struct
{
  unsigned long ipv4_ip_addr;
  unsigned long ipv4_subnet_mask;
} qmi_qos_ipv4_addr_filter_type;

typedef struct
{
  qmi_qos_ipv6_addr_type   ipv6_ip_addr;
  unsigned char            ipv6_filter_prefix_len;
}qmi_qos_ipv6_addr_filter_type;

typedef struct
{
  unsigned char tos_value;
  unsigned char tos_mask;
} qmi_qos_tos_filter_type;


typedef struct
{
  unsigned short start_port;
  unsigned short range;
} qmi_qos_port_range_filter_type;

typedef enum
{
  QMI_QOS_TRANS_PROT_TCP     = 0x06,
  QMI_QOS_TRANS_PROT_UDP     = 0x11,
  QMI_QOS_TRANS_PROT_TCP_UDP = 0xFD
} qmi_qos_trans_protocol_filter_type;

typedef enum
{
  QMI_QOS_IP_VERSION_4 = 0x04,
  QMI_QOS_IP_VERSION_6 = 0x06
} qmi_qos_ip_version_type;


#define QMI_QOS_FILTER_PARAM_SRC_ADDR             0x00000001
#define QMI_QOS_FILTER_PARAM_DEST_ADDR            0x00000002
#define QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL       0x00000004
#define QMI_QOS_FILTER_PARAM_TOS                  0x00000008
#define QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS        0x00000010
#define QMI_QOS_FILTER_PARAM_TCP_DEST_PORTS       0x00000020
#define QMI_QOS_FILTER_PARAM_UDP_SRC_PORTS        0x00000040
#define QMI_QOS_FILTER_PARAM_UDP_DEST_PORTS       0x00000080
#define QMI_QOS_FILTER_PARAM_IPV6_SRC_ADDR        0x00000100
#define QMI_QOS_FILTER_PARAM_IPV6_DEST_ADDR       0x00000200
#define QMI_QOS_FILTER_PARAM_IPV6_TRAFFIC_CLASS   0x00000400
#define QMI_QOS_FILTER_PARAM_IPV6_FLOW_LABEL      0x00000800
#define QMI_QOS_FILTER_PARAM_ESP_SECURITY_POLICY  0x00001000
#define QMI_QOS_FILTER_PARAM_PRECEDENCE           0x00002000
#define QMI_QOS_FILTER_PARAM_FILTER_ID            0x00004000
#define QMI_QOS_FILTER_PARAM_TRANSPORT_SRC_PORTS  0x00008000
#define QMI_QOS_FILTER_PARAM_TRANSPORT_DEST_PORTS 0x00010000

/* This parameter is only used for error report mask, since
** thie IP version is a required field for all requests
*/
#define QMI_QOS_FILTER_PARAM_IP_VER_ERR      0x80000000



typedef struct
{
  /* Mask of which optional parameters below are valid */
  unsigned long param_mask;

  /* UMTS optional QoS parameters.  At least one of these
  ** must be included in a setup request
  */
  qmi_qos_ipv4_addr_filter_type       src_addr;
  qmi_qos_ipv4_addr_filter_type       dest_addr;
  qmi_qos_trans_protocol_filter_type  protocol;
  qmi_qos_tos_filter_type             tos;
  qmi_qos_port_range_filter_type      tcp_src_ports;
  qmi_qos_port_range_filter_type      tcp_dest_ports;
  qmi_qos_port_range_filter_type      udp_src_ports;
  qmi_qos_port_range_filter_type      udp_dest_ports;
  qmi_qos_ipv6_addr_filter_type       ipv6_src_addr;
  qmi_qos_ipv6_addr_filter_type       ipv6_dest_addr;
  struct
  {
    unsigned char     traffic_class_value;
    unsigned char     traffic_class_mask;
  }ipv6_traffic_class;
  unsigned long                       ipv6_flow_label;
  unsigned long                       esp_security_policy_index;
  unsigned char                       precedence;
  unsigned char                       filter_id;
  qmi_qos_port_range_filter_type      transport_src_ports;
  qmi_qos_port_range_filter_type      transport_dest_ports;

} qmi_qos_filter_desc_type;


/* This is the filter request data structure.  Note that the ip_version
** parameter is required and must be set for each request.  The filter_desc
** optional parameters must follow rules outlined in QC QoS document
*/
typedef struct
{
  qmi_qos_ip_version_type        ip_version;
  qmi_qos_filter_desc_type         filter_desc;
} qmi_qos_filter_req_type;

typedef enum
{
  QMI_QOS_UNASSIGNED_TYPE   = 0x00,
  QMI_QOS_REQUEST           = 0x01,
  QMI_QOS_CONFIGURE         = 0x02
}qmi_qos_req_opcode_type;

/* This is the main QoS specification data structure.  For each
** of the request types, there is a count of the number
** request elements provided (can be an array of elements,
** treated in priority order) and a pointer to the request
** elements.
*/
typedef struct
{
  unsigned long             qos_identifier;     /*To be used only for QOS Modify*/

  unsigned long             num_tx_flow_req;
  qmi_qos_flow_req_type     *tx_flow_req_array;

  unsigned long             num_rx_flow_req;
  qmi_qos_flow_req_type     *rx_flow_req_array;

  unsigned long             num_tx_filter_req;
  qmi_qos_filter_req_type   *tx_filter_req_array;

  unsigned long             num_rx_filter_req;
  qmi_qos_filter_req_type   *rx_filter_req_array;

} qmi_qos_spec_type;

/* QOS request error response data type.  If the err_present flag is
** set then one or more of the included error masks will have bit(s)
** set.  The bits will indicate which of the required or optional
** parameters had errors for each of the request types.  The array
** elements correspond to the possible array of requests passed in
** in the request.
*/
typedef struct
{
  int            errs_present;
  unsigned short tx_flow_req_err_mask[QMI_QOS_MAX_FLOW_FILTER];
  unsigned short rx_flow_req_err_mask[QMI_QOS_MAX_FLOW_FILTER];
  unsigned short tx_filter_req_err_mask[QMI_QOS_MAX_FLOW_FILTER];
  unsigned short rx_filter_req_err_mask[QMI_QOS_MAX_FLOW_FILTER];
} qmi_qos_err_rsp_type;

/************************************************************************
* Definitions associated with qmi_qos_get_nw_supported_qos_profiles()
************************************************************************/
typedef enum
{
  QMI_QOS_INVALID_IFACE_NAME = 0x0000,

  QMI_QOS_ANY_DEFAULT_GROUP  = 0x0001,
  QMI_QOS_WWAN_GROUP         = 0x0002,
  QMI_QOS_RM_GROUP           = 0x0004,
  QMI_QOS_BCAST_MCAST_GROUP  = 0x0008,
  QMI_QOS_IFACE_3GPP_GROUP   = 0x0010,
  QMI_QOS_IFACE_3GPP2_GROUP  = 0x0020,
  QMI_QOS_IFACE_EPC_GROUP    = 0x0040,
  QMI_QOS_ANY_IFACE_GROUP    = 0x7FFF,

  QMI_QOS_IFACE_MASK         = 0x8000,
  QMI_QOS_CDMA_SN_IFACE      = 0x8001,
  QMI_QOS_CDMA_AN_IFACE      = 0x8002,
  QMI_QOS_UMTS_IFACE         = 0x8004,
  QMI_QOS_SIO_IFACE          = 0x8008,
  QMI_QOS_CDMA_BCAST_IFACE   = 0x8010,
  QMI_QOS_WLAN_IFACE         = 0x8020,
  QMI_QOS_DUN_IFACE          = 0x8040,
  QMI_QOS_FLO_IFACE          = 0x8080,
  QMI_QOS_DVBH_IFACE         = 0x8100,
  QMI_QOS_STA_IFACE          = 0x8200,
  QMI_QOS_IPSEC_IFACE        = 0x8400,
  QMI_QOS_LO_MODE_B_IFACE    = 0x8401,
  QMI_QOS_LO_IFACE           = 0x8800,
  QMI_QOS_MBMS_IFACE         = 0x8801,
  QMI_QOS_IWLAN_3GPP_IFACE   = 0x8802,
  QMI_QOS_IWLAN_3GPP2_IFACE  = 0x8804,
  QMI_QOS_MIP6_IFACE         = 0x8808,
  QMI_QOS_SLIP_IFACE         = 0x8810,
  QMI_QOS_UICC_IFACE         = 0x8820,
  QMI_QOS_UW_FMC_IFACE       = 0x8840,
  QMI_QOS_EPC_IFACE          = 0x8880,
  QMI_QOS_NAT_IFACE          = 0x8881
} qmi_qos_iface_name_type;

typedef enum
{
  QMI_QOS_PROFILE_TECH_3GPP  = 0x01,
  QMI_QOS_PROFILE_TECH_3GPP2 = 0x02
} qmi_qos_profile_tech_type;

typedef struct
{
  qmi_qos_iface_name_type     iface_type;
  unsigned char               num_profiles;
  unsigned short              profile[QMI_QOS_MAX_PROFILES]; //TODO:?
}qmi_qos_profile_info_type;

typedef struct
{
  unsigned char             num_instances;
  qmi_qos_profile_info_type profile_info[QMI_QOS_MAX_PROFILES];
}qmi_qos_nw_supported_qos_profiles;


typedef struct
{
  qmi_qos_nw_supported_qos_profiles     qos_profiles;
  qmi_qos_dss_errno_type                  dss_err;
}qmi_qos_nw_supported_qos_profiles_rsp_type;

/*Event Report related structures*/

/* For turning event reporting on/off */
#define QMI_QOS_GLOBAL_FLOW_REPORTING_STATE_PARAM             0x0001
#define QMI_QOS_NW_SUPPORTED_PROFILE_CHANGE_STATE_PARAM       0x0002

typedef enum
{
  QMI_QOS_EVENT_REPORTING_OFF = 0,
  QMI_QOS_EVENT_REPORTING_ON = 1
} qmi_qos_global_flow_reporting_state_type;

typedef enum
{
  QMI_QOS_PROFILE_CHANGE_REPORTING_ENABLE   = 0x01,
  QMI_QOS_PROFILE_CHANGE_REPORTING_DISABLE  = 0x00
}qmi_qos_profile_change_status;

typedef struct
{
  qmi_qos_profile_change_status      profile_change_status;
  qmi_qos_iface_name_type            iface_type;/*IMPORTANT: USE ONLY "3GPP2"*/
}qmi_qos_net_supported_profile_change_type;

typedef struct
{
  unsigned short               param_mask;

  qmi_qos_global_flow_reporting_state_type      flow_state;
  qmi_qos_net_supported_profile_change_type     profile_change_state;

}qmi_qos_event_report_state_type;

/*Status report set response enum*/

typedef enum
{
  QMI_QOS_STATUS_ACTIVATED = 0x01,
  QMI_QOS_STATUS_SUSPENDED = 0x02,
  QMI_QOS_STATUS_GONE = 0x03
} qmi_qos_status_info;

typedef enum
{
  QMI_QOS_ACTIVATED_EV          = 0x01,
  QMI_QOS_SUSPENDED_EV          = 0x02,
  QMI_QOS_GONE_EV               = 0x03,
  QMI_QOS_MODIFY_ACCEPTED_EV    = 0x04,
  QMI_QOS_MODIFY_REJECTED_EV    = 0x05,
  QMI_QOS_INFO_CODE_UPDATED_EV  = 0x06,
} qmi_qos_status_event;

typedef enum
{
  QMI_QOS_INVALID_PARAMS                          = 0x01,
  QMI_QOS_INTERNAL_CALL_ENDED                     = 0x02,
  QMI_QOS_INTERNAL_ERROR                          = 0x03,
  QMI_QOS_INSUFFICIENT_LOCAL_RESOURCES            = 0x04,
  QMI_QOS_TIMED_OUT_OPERATION                     = 0x05,
  QMI_QOS_INTERNAL_UNKNOWN_CAUSE_CODE             = 0x06,
  QMI_QOS_INTERNAL_MODIFY_IN_PROGRESS             = 0x07,
  QMI_QOS_NOT_SUPPORTED                           = 0x08,
  QMI_QOS_NOT_AVAILABLE                           = 0x09,
  QMI_QOS_NOT_GUARANTEED                          = 0x0A,
  QMI_QOS_INSUFFICIENT_NETWORK_RESOURCES          = 0x0B,
  QMI_QOS_AWARE_SYSTEM                            = 0x0C,
  QMI_QOS_UNAWARE_SYSTEM                          = 0x0D,
  QMI_QOS_REJECTED_OPERATION                      = 0x0E,
  QMI_QOS_WILL_GRANT_WHEN_QOS_RESUMED             = 0x0F,
  QMI_QOS_NETWORK_CALL_ENDED                      = 0x10,
  QMI_QOS_NETWORK_SERVICE_NOT_AVAILABLE           = 0x11,
  QMI_QOS_NETWORK_L2_LINK_RELEASED                = 0x12,
  QMI_QOS_NETWORK_L2_LINK_REESTAB_REJ             = 0x13,
  QMI_QOS_NETWORK_L2_LINK_REESTAB_IND             = 0x14,
  QMI_QOS_NETWORK_UNKNOWN_CAUSE_CODE              = 0x15,
  QMI_QOS_NETWORK_BUSY                            = 0x16,
  QMI_QOS_NETWORK_DISJOINT_PROFILE_SET_SUGGESTED  = 0x17,
  QMI_QOS_NETWORK_NULL_PROFILE_SUGGESTED          = 0x18,
  QMI_QOS_NETWORK_UE_NOT_AUTHORIZED               = 0x19
} qmi_qos_reason_code;

typedef struct
{
  struct
  {
    unsigned long          qos_identifier;
    qmi_qos_status_info    qos_status;
    qmi_qos_status_event   qos_event;
  }qos_status_information;

  int qos_reason_is_valid;
  qmi_qos_reason_code   qos_reason;

}qmi_qos_status_report_type;

/************************************************************************
* Definitions associated with qmi_qos_get_granted_qos_info()
************************************************************************/

typedef enum
{
  QMI_QOS_FLOW_ACTIVATED = 0x01,
  QMI_QOS_FLOW_MODIFIED  = 0x02,
  QMI_QOS_FLOW_DELETED   = 0x03,
  QMI_QOS_FLOW_SUSPENDED = 0x04,
  QMI_QOS_FLOW_ENABLED   = 0x05,
  QMI_QOS_FLOW_DISABLED  = 0x06
}global_flow_state_change;

typedef struct
{
  unsigned short           ip_flow_index;
  qmi_qos_flow_req_type    qos_flow_granted;
}qmi_qos_granted_flow_data_type;

typedef struct
{
  unsigned short           filter_index;
  qmi_qos_filter_req_type    qos_filter;
}qmi_qos_granted_filter_data_type;


/*current granted qos info*/
typedef struct
{
  int                             tx_granted_flow_data_is_valid;
  qmi_qos_granted_flow_data_type  tx_granted_flow_data;

  int                             rx_granted_flow_data_is_valid;
  qmi_qos_granted_flow_data_type  rx_granted_flow_data;

  int                              tx_filter_count;
  qmi_qos_granted_filter_data_type tx_granted_filter_data[QMI_QOS_MAX_FLOW_FILTER];

  int                              rx_filter_count;
  qmi_qos_granted_filter_data_type rx_granted_filter_data[QMI_QOS_MAX_FLOW_FILTER];

  qmi_qos_dss_errno_type          dss_errno_type;
}qmi_qos_granted_info_rsp_type;


#define QMI_QOS_EVENT_REPORT_FLOW_INFO_FLOW_STATE_PARAM     0x0001
#define QMI_QOS_EVENT_REPORT_FLOW_INFO_FLOW_TYPE_PARAM      0x0002

typedef struct
{
  unsigned short          param_mask;

  struct
  {
    unsigned long     qos_identifier;
    unsigned char     new_flow;
    global_flow_state_change  report_flow_state_chng;
  } qos_flow_state;

  unsigned char           flow_type;

  /*If the Corresponding bit is set in the event mask*/
  int                     tx_granted_flow_data_is_valid;
  qmi_qos_granted_flow_data_type   tx_granted_flow_data;

  /*If the Corresponding bit is set in the event mask*/
  int                     rx_granted_flow_data_is_valid;
  qmi_qos_granted_flow_data_type   rx_granted_flow_data;

  unsigned int            tx_filter_count;/* other suggestions for this variable ? */
  qmi_qos_granted_filter_data_type  tx_granted_filter_data[QMI_QOS_MAX_FLOW_FILTER];

  unsigned int            rx_filter_count;
  qmi_qos_granted_filter_data_type  rx_granted_filter_data[QMI_QOS_MAX_FLOW_FILTER];

}qmi_qos_event_report_flow_info_type;


/*typedef event report indication structure*/
#define QMI_QOS_EVENT_REPORT_NW_SUPPORTED_QOS_PROFILES_PARAM     0x0001
#define QMI_QOS_EVENT_REPORT_GLOBAL_FLOW_INFO_PARAM              0x0002

typedef struct
{
  unsigned short                            param_mask; //PARAM MASK for nw_supported_qos_profiles

  int                                       num_flows; // Number of valid flows in the indication message.
  qmi_qos_event_report_flow_info_type       flow_info[QMI_QOS_MAX_FLOW_EVENTS];

  qmi_qos_profile_info_type                 nw_supported_qos_profiles;

}qmi_qos_event_report_type;


/* Distinguishes indication message types */
typedef enum
{
  QMI_QOS_SRVC_INVALID_IND_MSG,
  QMI_QOS_SRVC_EVENT_REPORT_IND_MSG,
  QMI_QOS_SRVC_STATUS_REPORT_IND_MSG,
  QMI_QOS_SRVC_NW_STATUS_REPORT_IND_MSG,
  QMI_QOS_SRVC_PRIMARY_QOS_EVENT_IND_MSG,
  QMI_QOS_SRVC_PROFILES_CHANGE_EVENT_IND_MSG
  /* To be filled in in future release */
} qmi_qos_indication_id_type;

typedef enum
{
  NW_DOESNT_SUPPORT_QOS = 0x00,
  NW_SUPPORTS_QOS       = 0x01
} qmi_qos_nw_support_status;

typedef struct
{
  qmi_qos_nw_support_status  qos_supported;
  qmi_qos_iface_name_type    iface_name;
} qmi_qos_nw_status_type;

typedef enum
{
  QMI_QOS_PRIMARY_FLOW_MODIFY_SUCCESS    = 0x0001,
  QMI_QOS_PRIMARY_FLOW_MODIFY_FAILURE   = 0x0002
}qmi_qos_primrary_qos_event_type;


/* Async notification reporting structure */
typedef union
{
  qmi_qos_nw_status_type            nw_report;
  qmi_qos_status_report_type        status_report;
  qmi_qos_event_report_type         event_report;
  qmi_qos_primrary_qos_event_type   primary_qos_modify_result;
  qmi_qos_event_report_state_type   event_report_state;
  /* To be filled in in future release */
} qmi_qos_indication_data_type;


typedef void (*qmi_qos_indication_hdlr_type)
(
int                           user_handle,
qmi_service_id_type           service_id,
void                          *user_data,
qmi_qos_indication_id_type    ind_id,
qmi_qos_indication_data_type  *ind_data
);

/************************************************************************
* Definitions associated with qmi_qos_bind_mux_data_port()
************************************************************************/

typedef enum
{
  QMI_QOS_PER_EP_TYPE_MIN      = 0x00,
  QMI_QOS_PER_EP_TYPE_RESERVED = 0x00,
  QMI_QOS_PER_EP_TYPE_HSIC     = 0x01,
  QMI_QOS_PER_EP_TYPE_HSUSB    = 0x02,
  QMI_QOS_PER_EP_TYPE_PCIE     = 0x03,
  QMI_QOS_PER_EP_TYPE_EMBEDDED = 0x04,
  QMI_QOS_PER_EP_TYPE_MAX
} qmi_qos_per_ep_type;

/** Structure for specifying EP ID information */
typedef struct
{
  qmi_qos_per_ep_type ep_type;          /** Peripheral end point type */
  unsigned long       iface_id;         /** Data end-point ID */
} qmi_qos_per_ep_id_type;

#define QMI_QOS_BIND_MUX_DATA_PORT_PARAMS_EP_ID     0x00000001
#define QMI_QOS_BIND_MUX_DATA_PORT_PARAMS_MUX_ID    0x00000002
#define QMI_QOS_BIND_MUX_DATA_PORT_PARAMS_REVERSED  0x00000004

/** Parameter structure for binding QOS client to a mux data
 *  port */
typedef struct
{
  unsigned long           params_mask;  /** Params mask */
  qmi_qos_per_ep_id_type  ep_id;        /** EP ID information */
  unsigned char           mux_id;       /** Mux ID to bind to */
  unsigned char           reversed;     /** Whether this is a reverse port */
} qmi_qos_bind_mux_data_port_params_type;

typedef enum
{
  QMI_QOS_DEFAULT_SUBS = 0x0000,
  QMI_QOS_PRIMARY_SUBS = 0x0001,
  QMI_QOS_SECONDARY_SUBS = 0x0002,
  QMI_QOS_TERITIARY_SUBS = 0x0003,
  QMI_QOS_DONT_CARE_SUBS = 0x00FF
} qmi_qos_bind_subscription_type;

/************************************************************************
* Definitions associated with qmi_qos_get_qos_nw_status()
************************************************************************/

/************************************************************************
* Function prototypes
************************************************************************/

/*===========================================================================
  FUNCTION  qmi_qos_srvc_init_client
===========================================================================*/
/*!
@brief
  This function is called to initialize the QoS service.  This function
  must be called prior to calling any other QoS service functions.
  For the time being, the indication handler callback and user data
  should be set to NULL until this is implemented.  Also note that this
  function may be called multiple times to allow for multiple, independent
  clients.

@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
EXTERN qmi_client_handle_type
qmi_qos_srvc_init_client
(
  const char                    *dev_id,
  qmi_qos_indication_hdlr_type  ind_hdlr,
  void                          *ind_hdlr_user_data,
  int                           *qmi_err_code
);



/*===========================================================================
  FUNCTION  qmi_qos_srvc_release_client
===========================================================================*/
/*!
@brief
  This function is called to release a client created by the
  qmi_qos_srvc_init_client() function.  This function should be called
  for any client created when terminating a client process, especially
  if the modem processor is not reset.  The modem side QMI server has
  a limited number of clients that it will allocate, and if they are not
  released, we will run out.

@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
EXTERN int
qmi_qos_srvc_release_client
(
  qmi_client_handle_type  client_handle,
  int                     *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qos_bind_mux_data_port
===========================================================================*/
/*!
@brief
  Binds a QMI-QOS client to a MUX data port. In the case of QMAP, we will
  have a single control channel, so QOS clients need to specify which
  data port their actions need to be associated with.

@param[in]  user_handle: QMI QOS client handle.
@param[in]  params: Bind MUX data port specification
@param[out] qmi_err_code: QMI error code in case of failure.

@see
  qmi_qos_bind_mux_data_port_params_type

@return
  0 if operation was successful.
  < 0 If operation failed.  qmi_err_code will contain the reason.

@dependencies
  qmi_qos_srvc_init_client() must be called before calling this.

*/
/*=========================================================================*/
EXTERN int
qmi_qos_bind_mux_data_port
(
  int                                     user_handle,
  qmi_qos_bind_mux_data_port_params_type *params,
  int                                    *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qos_bind_subscription
===========================================================================*/
/*!
@brief
 Binds a QoS client to a subscription.

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

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
qmi_qos_bind_subscription
(
  int                          user_handle,
  qmi_qos_bind_subscription_type  subs_id,
  int                         *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qos_get_bind_subscription
===========================================================================*/
/*!
@brief
  This message queries the current subscription the client is bound to.

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

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
qmi_qos_get_bind_subscription
(
  int                                          user_handle,
  qmi_qos_bind_subscription_type               *subs_id,
  int                                          *qmi_err_code
);
/*===========================================================================
  FUNCTION  qmi_qos_request_qos
===========================================================================*/
/*!
@brief
  This function sends a QoS request.  Note that for CDMA technology
  types, an array of up to 10 qos_specs may be specified at one time.
  For UMTS, only a single qos_spec can be specified at a time (num_qos_specs
  must be set to 1).  Upon successful return, the qos_identifiers pointer/array
  will contain identifiers for each of the qos_specs requested.  These
  identifiers will be used for any QoS operations/indications to follow on that
  particular session.

@return
  0 (QMI_NO_ERR) if operation was sucessful, < 0 if not.
  In the failure case when QMI_SERVICE_ERR is returned, the qmi_err_code
  will be valid and will contain the error code returned by QMI on the modem,
  and there may be more error information passed back in the qos_spec_errs data
  structure, depending on the nature of the error.

@note
    - Dependencies: qmi_qos_srvc_init_client() must be called before calling
                    this function.


    - Side Effects: Brings up a QoS session
*/
/*=========================================================================*/
EXTERN int
qmi_qos_request_qos
(
  qmi_client_handle_type        client_handle,
  int                           num_qos_specs,
  qmi_qos_spec_type             *qos_spec_array,
  qmi_qos_req_opcode_type       req_opcode_type,
  unsigned long                 *qos_id_array,    /* Valid if rc == QMI_NO_ERR */
  qmi_qos_err_rsp_type          *qos_spec_errs,   /* Valid if rc == QMI_SERVICE_ERR */
  int                           *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qos_modify_primary_qos
===========================================================================*/
/*!
@brief
  Modifies a primary QoS.

@return
  0 (QMI_NO_ERR) if operation was sucessful, < 0 if not.
  In the failure case when QMI_SERVICE_ERR is returned, the qmi_err_code
  will be valid and will contain the error code returned by QMI on the modem.

@note
    - Dependencies: qmi_qos_srvc_init_client() must be called before calling
                    this function.

*/
/*=========================================================================*/
EXTERN int
qmi_qos_modify_primary_qos
(
  qmi_client_handle_type        client_handle,
  qmi_qos_spec_type             *qos_spec,
  qmi_qos_err_rsp_type          *qos_spec_err,   /* Valid if rc == QMI_SERVICE_ERR */
  int                           *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_qos_modify_secondary_qos
===========================================================================*/
/*!
@brief
  Modifies a secondary QoS.

@return
  0 (QMI_NO_ERR) if operation was sucessful, < 0 if not.
  In the failure case when QMI_SERVICE_ERR is returned, the qmi_err_code
  will be valid and will contain the error code returned by QMI on the modem.

@note
    - Dependencies: qmi_qos_srvc_init_client() must be called before calling
                    this function.

*/
/*=========================================================================*/
EXTERN int
qmi_qos_modify_secondary_qos
(
  qmi_client_handle_type        client_handle,
  int                           num_qos_specs,
  qmi_qos_spec_type             *qos_spec_array,
  qmi_qos_err_rsp_type          *qos_spec_errs,   /* Valid if rc == QMI_SERVICE_ERR */
  int                           *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_qos_release_qos
===========================================================================*/
/*!
@brief
  Tears down a requested QoS.  More that one QoS may be specified by passing
  in an array of qos_identifiers and setting the num_qos_identifiers
  accordingly.

@return
  0 (QMI_NO_ERR) if operation was sucessful, < 0 if not.
  In the failure case when QMI_SERVICE_ERR is returned, the qmi_err_code
  will be valid and will contain the error code returned by QMI on the modem.

@note
    - Dependencies: qmi_qos_srvc_init_client() must be called before calling
                    this function.
    - Side Effects:  Tears down a QoS
*/
/*=========================================================================*/
EXTERN int
qmi_qos_release_qos
(
  qmi_client_handle_type   client_handle,
  int                      num_qos_identifiers,
  unsigned long            *qos_id_array,
  int                      *qmi_err_code
);



/*===========================================================================
  FUNCTION  qmi_qos_get_nw_supported_qos_profiles
===========================================================================*/
/*!
@brief
  This function is a request to resume one or more existing QoS flows.
  Each QoS flow is identified with its QoS identifier

@return

@note
  - Number of qos_identifiers for UMTS tech should always be 1
    (not more than that).
  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.
  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
EXTERN int
qmi_qos_get_nw_supported_qos_profiles
(
  qmi_client_handle_type                         client_handle,
  qmi_qos_technology_type                        tech_pref,
  qmi_qos_nw_supported_qos_profiles_rsp_type     *rsp_data,
  int                                            *qmi_err_code
);
/*===========================================================================
  FUNCTION  qmi_qos_resume_qos
===========================================================================*/
/*!
@brief
  This function is a request to resume one or more existing QoS flows.
  Each QoS flow is identified with its QoS identifier

@return

@note
  - Number of qos_identifiers for UMTS tech should always be 1
    (not more than that).
  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.
  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
EXTERN int
qmi_qos_resume_qos
(
  qmi_client_handle_type    client_handle,
  unsigned char             num_qos_ids,
  unsigned long             *qos_id_array,
  int                       *qmi_err_code
);
/*===========================================================================
  FUNCTION  qmi_qos_suspend_qos
===========================================================================*/
/*!
@brief
  This function is a request to suspend one or more existing QoS flows.
  Each QoS flow is identified with its QoS identifier

@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.
  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
EXTERN int
qmi_qos_suspend_qos
(
  qmi_client_handle_type    client_handle,
  unsigned char             num_qos_ids,
  unsigned long             *qos_id_array,
  int                       *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qos_reset_qos_srvc_variables
===========================================================================*/
/*!
@brief
  Resets the control points state which is kept by QOS.

@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.
    - The client handle of the requesting control point doesnot change.
    - The Control points state variables will be set to default values
     before the response.
  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
EXTERN int
qmi_qos_reset_qos_srvc_variables
(
  qmi_client_handle_type   client_handle,
  int                      *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_qos_get_primary_granted_qos_info
===========================================================================*/
/*!
@brief
  This function if successful returns the qos parameters for a
  specified primary QOS flow.


@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
EXTERN int
qmi_qos_get_primary_granted_qos_info
(
  qmi_client_handle_type                  client_handle,
  unsigned long                           qos_identifier,
  qmi_qos_granted_info_rsp_type           *granted_info,
  int                                     *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qos_get_secondary_granted_qos_info
===========================================================================*/
/*!
@brief
  This function if successful returns the qos parameters for a
  specified secondary QOS flow.


@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
EXTERN int
qmi_qos_get_secondary_granted_qos_info
(
  qmi_client_handle_type                  client_handle,
  unsigned long                           qos_identifier,
  qmi_qos_granted_info_rsp_type           *granted_info,
  int                                     *qmi_err_code
);



/*===========================================================================
  FUNCTION  qmi_qos_does_nw_support_qos
===========================================================================*/
/*!
@brief
  This function queries if the current network supports QOS!

@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
EXTERN int
qmi_qos_does_nw_support_qos
(
  qmi_client_handle_type            client_handle,
  qmi_qos_nw_status_type            *status_resp,
  int                               *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_qos_get_status
===========================================================================*/
/*!
@brief
  This function queries if the current network supports QOS


@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
EXTERN int
qmi_qos_get_status
(
  qmi_client_handle_type            client_handle,
  unsigned long                     qos_identifier,
  qmi_qos_status_info               *status_resp,
  int                               *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_qos_set_client_ip_pref
===========================================================================*/
/*!
@brief
  This message sends the prefered client IP family preference to the modem
  to support dual-IP over single RmNet.

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

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
qmi_qos_set_client_ip_pref
(
  int                          user_handle,
  qmi_ip_family_pref_type      ip_family,
  int                         *qmi_err_code
);


#ifdef __cplusplus
}
#endif

#endif /* QMI_QOS_SRVC_H */

