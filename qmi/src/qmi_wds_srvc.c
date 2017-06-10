/******************************************************************************
  @file    qmi_wds_srvc.c
  @brief   The QMI WDS service layer.

  DESCRIPTION
  QMI WDS service routines.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_wds_srvc_init_client() needs to be called before sending or receiving of any
  CTL service messages

  ---------------------------------------------------------------------------
  Copyright (c) 2007, 2014-2015 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <memory.h>
#include <stdlib.h>
#include "qmi_i.h"
#include "qmi_service.h"
#include "qmi_wds_srvc.h"
#include "qmi_wds_srvc_i.h"
#include "qmi_util.h"

#define QMI_WDS_UMTS_QOS_TLV_SIZE            33
#define QMI_WDS_GPRS_QOS_TLV_SIZE            QMI_WDS_GPRS_QOS_SIZE
#define QMI_WDS_LTE_QOS_TLV_SIZE             QMI_WDS_LTE_QOS_SIZE
#define QMI_WDS_MNC_TLV_SIZE                 QMI_WDS_MNC_SIZE


#define QMI_WDS_TFT_FILTER_TLV_SIZE          QMI_WDS_TFT_FILTER_SIZE

/* Space needed to hold IPv4 addr + IPv6 addr + Bearer IP Type (1 byte) */
#define QMI_WDS_HANDOFF_CONTEXT_TLV_SIZE     (QMI_WDS_IPV4_ADDR_SIZE_IN_BYTES + \
                                              QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES + \
                                              sizeof(unsigned char))

/* Space needed to hold IPv6 addr + prefix length (1 byte) */
#define QMI_WDS_REMOVE_DELEGATED_IPV6_PREFIX_TLV_SIZE  (QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES + \
                                                        sizeof(unsigned char))

/* Request/response message ID's */
#define QMI_RESULT_CODE_TYPE_ID                           0x0002
#define QMI_WDS_START_NW_IF_MSG_ID                        0x0020
#define QMI_WDS_STOP_NW_IF_MSG_ID                         0x0021
#define QMI_WDS_RESET_MSG_ID                              0x0000
#define QMI_WDS_ABORT_MSG_ID                              0x0002
#define QMI_WDS_GET_PKT_SRVC_STATUS_MSG_ID                0x0022
#define QMI_WDS_DELETE_PROFILE_MSG_ID                     0x0029
#define QMI_WDS_GET_DEFAULT_SETTINGS_MSG_ID               0x002C
#define QMI_WDS_SET_EVENT_REPORT_MSG_ID                   0x0001
#define QMI_WDS_GET_RUNTIME_SETTINGS_MSG_ID               0x002D
#define QMI_WDS_ROUTE_LOOK_UP_MSG_ID                      0xFFF2
#define QMI_WDS_INTERNAL_IFACE_EV_REGISTER_MSG_ID         0xFFFE
#define QMI_WDS_GET_CURRENT_RUNTIME_SETTINGS_MSG_ID       0xFFFD
#define QMI_WDS_REFRESH_DHCP_CONFIG_MSG_ID                0xFFFB
#define QMI_WDS_SET_CURRENT_RUNTIME_SETTINGS_MSG_ID       0xFFFC
#define QMI_WDS_REGISTER_MT_CALL_REQ_MSG_ID               0xFFFA
#define QMI_WDS_DE_REGISTER_MT_CALL_REQ_MSG_ID            0xFFF9
#define QMI_WDS_BCMCS_DB_UPDATE_REQ_MSG_ID                0xFFF8
#define QMI_WDS_BCMCS_HANDOFF_OPT_ENABLE_MSG_ID           0xFFF7
#define QMI_WDS_BCMCS_BOM_CACHING_SETUP_MSG_ID            0xFFF6
#define QMI_WDS_INITIATE_MCAST_JOIN_EX_REQ_MSG_ID         0xFFF5
#define QMI_WDS_INITIATE_MCAST_LEAVE_EX_REQ_MSG_ID        0xFFF4
#define QMI_WDS_INITIATE_MCAST_REGISTER_EX_REQ_MSG_ID     0xFFF3
#define QMI_WDS_GO_ACTIVE_REQ_MSG_ID                      0x0026
#define QMI_WDS_GO_DORMANT_REQ_MSG_ID                     0x0025
#define QMI_WDS_ACTIVATE_MBMS_MCAST_CONTEXT_REQ_MSG_ID    0x0039
#define QMI_WDS_DEACTIVATE_MBMS_MCAST_CONTEXT_REQ_MSG_ID  0x003A
#define QMI_WDS_GET_CURRENT_DATA_BEARER_TECH_REQ_MSG_ID   0x0044
#define QMI_WDS_GET_CURRENT_DATA_BEARER_TECH_EX_REQ_MSG_ID   0x0091
#define QMI_WDS_MCAST_JOIN_REQ_MSG_ID                     0x0031
#define QMI_WDS_MCAST_LEAVE_REQ_MSG_ID                    0x0032
#define QMI_WDS_GET_CURRENT_CHANNEL_RATE_REQ_MSG_ID       0x0023
#define QMI_WDS_GET_PKT_STATS_MSG_ID                      0x0024
#define QMI_WDS_GET_DORMANCY_STATUS_MSG_ID                0x0030
#define QMI_WDS_GET_DUN_CALL_INFO_MSG_ID                  0x0038
#define QMI_WDS_GET_DEFAULT_PROFILE_NUMBER_MSG_ID         0x0049
#define QMI_WDS_SET_DEAFULT_PROFILE_NUMBER_MSG_ID         0x004A
#define QMI_WDS_RESET_PROFILE_TO_DEFAULT_MSG_ID           0x004B
#define QMI_WDS_RESET_PROFILE_PARAM_TO_INVALID_MSG_ID     0x004C
#define QMI_WDS_SET_CLIENT_IP_FAMILY_PREF_MSG_ID          0x004D
#define QMI_WDS_FMC_SET_TUNNEL_PARAMS_MSG_ID              0x004E
#define QMI_WDS_FMC_CLEAR_TUNNEL_PARAMS_MSG_ID            0x004F
#define QMI_WDS_FMC_GET_TUNNEL_PARAMS_MSG_ID              0x0050
#define QMI_WDS_SET_MOBILE_IP_MODE_MSG_ID                 0x002E
#define QMI_WDS_GET_MOBILE_IP_MODE_MSG_ID                 0x002F
#define QMI_WDS_GET_ACTIVE_MOBILE_IP_PROFILE_MSG_ID       0x003C
#define QMI_WDS_SET_ACTIVE_MOBILE_IP_PROFILE_MSG_ID       0x003D
#define QMI_WDS_READ_MOBILE_IP_PROFILE_MSG_ID             0x003E
#define QMI_WDS_MODIFY_MOBILE_IP_PROFILE_MSG_ID           0x003F
#define QMI_WDS_INDICATION_REG_MSG_ID                     0x0003
#define QMI_WDS_EMBMS_TMGI_ACTIVATE_MSG_ID                0x0065
#define QMI_WDS_EMBMS_TMGI_DEACTIVATE_MSG_ID              0x0066
#define QMI_WDS_EMBMS_TMGI_ACT_DEACT_MSG_ID               0x0088
#define QMI_WDS_GET_PREF_DATA_SYS_MSG_ID                  0x0069
#define QMI_WDS_EMBMS_TMGI_LIST_QUERY_MSG_ID              0x0067
#define QMI_WDS_GET_CURRENT_DATA_SYS_STATUS_MSG_ID        0x006B
#define QMI_WDS_GET_LTE_ATTACH_PARAMS_MSG_ID              0x0085
#define QMI_WDS_RESET_PKT_STATS_MSG_ID                    0x0086
#define QMI_WDS_GET_LTE_MAX_ATTACH_PDN_NUM_MSG_ID         0x0092
#define QMI_WDS_SET_LTE_ATTACH_PDN_LIST_MSG_ID            0x0093
#define QMI_WDS_GET_LTE_ATTACH_PDN_LIST_MSG_ID            0x0094
#define QMI_WDS_SET_LTE_DATA_RETRY_MSG_ID                 0x0096
#define QMI_WDS_GET_LTE_DATA_RETRY_MSG_ID                 0x0097
#define QMI_WDS_SET_LTE_ATTACH_TYPE_MSG_ID                0x0098
#define QMI_WDS_GET_LTE_ATTACH_TYPE_MSG_ID                0x0099
#define QMI_WDS_REV_IP_TRANS_CONN_IND_REG_MSG_ID          0x008D
#define QMI_WDS_GET_IPSEC_STATIC_SA_CONFIG                0x008F
#define QMI_WDS_REV_IP_TRANS_CONFIG_COMPLETE_MSG_ID       0x0090
#define QMI_WDS_BIND_MUX_DATA_PORT_MSG_ID                 0x00A2
#define QMI_WDS_INITIATE_ESP_REKEY                        0x00A6
#define QMI_WDS_BIND_SUBSCRIPTION_MSG_ID                  0x00AF
#define QMI_WDS_GET_BIND_SUBSCRIPTION_MSG_ID              0x00B0
#define QMI_WDS_EMBMS_CONTENT_DESC_UPDATE_MSG_ID          0x00B5
#define QMI_WDS_SET_DATA_PATH_MSG_ID                      0x009C
#define QMI_WDS_GET_DATA_PATH_MSG_ID                      0x009D
#define QMI_WDS_REMOVE_DELEGATED_IPV6_PREFIX_REQ_MSG_ID   0x00AD

/* Start NW Interface TLV ID's */
#define QMI_WDS_NW_IF_PROFILE_IDX_REQ_TLV_ID               0x31
#define QMI_WDS_NW_IF_PROFILE_IDX_3GPP2_REQ_TLV_ID         0x32
#define QMI_WDS_NW_IF_APN_NAME_REQ_TLV_ID                  0x14
#define QMI_WDS_NW_IF_TECH_PREF_REQ_TLV_ID                 0x30
#define QMI_WDS_NW_IF_XTENDED_TECH_PREF_REQ_TLV_ID         0x34
#define QMI_WDS_NW_IF_IP_FAMILY_PREF_REQ_TLV_ID            0x19
#define QMI_WDS_NW_IF_PRIM_DNS_ADDR_PREF_REQ_TLV_ID        0x10
#define QMI_WDS_NW_IF_SECONDARY_DNS_ADDR_PREF_REQ_TLV_ID   0x11
#define QMI_WDS_NW_IF_PRIMARY_NBNS_ADDR_PREF_REQ_TLV_ID    0x12
#define QMI_WDS_NW_IF_SECONDARY_NBNS_ADDR_PREF_REQ_TLV_ID  0x13
#define QMI_WDS_NW_IF_IPV4_ADDR_PREF_REQ_TLV_ID            0x15
#define QMI_WDS_NW_IF_AUTHENTICATION_PREF_REQ_TLV_ID       0x16
#define QMI_WDS_NW_IF_USERNAME_REQ_TLV_ID                  0x17
#define QMI_WDS_NW_IF_PASSWORD_REQ_TLV_ID                  0x18
#define QMI_WDS_NW_IF_IFACE_HNDL_TLV_ID                    0xFE
#define QMI_WDS_NW_IF_DATA_CALL_TYPE_TLV_ID                0x35
#define QMI_WDS_NW_IF_HANDOFF_CONTEXT_TLV_ID               0x36

/*Get Current settings optional TLV IDS*/

#define QMI_WDS_REQ_RUNTIME_SETTINGS_TLV_ID        0x10
#define QMI_WDS_REQ_SESSION_TIMER_SEL_TLV_ID       0x11
#define QMI_WDS_REQ_SDB_FLAGS_TLV_ID               0x12

/* Current Run time Settings response TLV ID's */
#define QMI_WDS_EXTENDED_ERROR_INFO                 0xE0
#define QMI_WDS_OP_FAILURE_TLV_ID                   0xE1
#define QMI_WDS_DORM_TIMER_1X_TLV_ID                0x11
#define QMI_WDS_SESSION_TIMER_1X_TLV_ID             0x12
#define QMI_WDS_HDR_1X_HANDDOWN_TLV_ID              0x13
#define QMI_WDS_HYSTERISIS_ACTIVATION_TIMER_TLV_ID  0x14
#define QMI_WDS_SDB_SUPPORT_TLV_ID                  0x16
#define QMI_WDS_RF_CONDITIONS_TLV_ID                0x10
#define QMI_WDS_HDR_EIDLE_SLOT_MODE_OPT_TLV_ID      0x15

/*Set Internal Runtime Settings Request/Response TLV IDs*/
/*REQ type*/

#define QMI_WDS_SET_INTERNAL_RUNTIME_ENABLE_HOLDDOWN_REQ_TLV_ID         0x10
#define QMI_WDS_SET_INTERNAL_RUNTIME_HDR_HPT_REQ_TLV_ID                 0x16
#define QMI_WDS_SET_INTERNAL_RUNTIME_HDR_REV0_RATE_INERTIA_REQ_TLV_ID   0x17

/*MTerminated registration request related TLV IDS*/

#define QMI_WDS_IP_FAMILY_PREF_REQ_TLV_ID       0x10
#define QMI_WDS_3GPP_PROFILE_ID_REQ_TLV_ID      0x11
#define QMI_WDS_TECH_PREF_REQ_TLV_ID            0x12

/*MTerminated registration RESP related TLV IDS*/
#define QMI_WDS_MT_PACKET_HNDL                  0x01
#define QMI_WDS_MCAST_HNDL_LIST_RESP_TLV_ID     0x01


/*Mcast join(Non EX) Req Resp */
#define QMI_WDS_MCAST_JOIN_IPV4_INFO_REQ_TLV_ID           0x10
#define QMI_WDS_MCAST_JOIN_MBMS_JOIN_INFO_REQ_TLV_ID      0x11
/*RESP*/
#define QMI_WDS_MCAST_JOIN_MCAST_HNDL_RESP_TLV_ID         0x01
/*MCAST LEAVE REQ*/
#define QMI_WDS_MCAST_LEAVE_MCAST_HNDL_REQ_TLV_ID         0x01
/* General Request/response/indication TLV ID's */
#define QMI_WDS_NW_IF_PKT_HANDLE_RSP_TLV_ID           0x01
#define QMI_WDS_ABORT_REQ_TLV_ID                      0x01
#define QMI_WDS_STOP_NW_IF_REQ_TLV_ID                 0x01
#define QMI_WDS_PKT_SRVC_STATUS_TLV_ID                0x01
#define QMI_WDS_PKT_SRVC_CALL_END_TLV_ID              0x10
#define QMI_WDS_BCMCS_DB_UPDATE_REQ_TLV_ID            0x01
#define QMI_WDS_BCMCS_HANDOFF_ENABLE_REQ_TLV_ID       0x01
#define QMI_WDS_BCMCS_BOM_CACHING_SETUP_REQ_TLV_ID    0x01
#define QMI_WDS_INITIATE_MCAST_JOIN_EX_REQ_TLV_ID     0x01
#define QMI_WDS_MCAST_HNDL_LIST_REQ_TLV_ID            0x01
#define QMI_WDS_IPV4_CONTEXT_ACTIVATION_INFO_TLV_ID   0x10
#define QMI_WDS_MBMS_CONTEXT_HANDLE_RSP_TLV_ID        0x01
#define QMI_WDS_MBMS_MCAST_CONTEXT_STATUS_IND_TLV_ID  0x01
#define QMI_WDS_MBMS_MCAST_REASON_CODE_IND_TLV_ID     0x10
#define QMI_WDS_CURRENT_DATA_BEARER_TECH_RESP_TLV_ID  0x01
#define QMI_WDS_CURR_CHANNEL_RATE_RESP_TLV_ID         0x01
#define QMI_WDS_PKT_SRVC_CALL_END_VERBOSE_TLV_ID      0x11
#define QMI_WDS_PKT_SRVC_IP_FAMILY_TLV_ID             0x12
#define QMI_WDS_PKT_SRVC_TECH_NAME_TLV_ID             0x13
#define QMI_WDS_PKT_SRVC_XLAT_CAP_TLV_ID              0x15
#define QMI_WDS_CLIENT_IP_PREF_REQ_TLV_ID             0x01
#define QMI_WDS_REMOVE_DELEGATED_IPV6_PREFIX_REQ_TLV_ID  0x10

/* Route look-up response TLV ID's */
#define QMI_WDS_ROUTE_LOOKUP_TYPE_REQ_TLV_ID  0x01
#define QMI_WDS_ROUTE_LOOKUP_IFACE_TLV_ID     0x01
#define QMI_WDS_ROUTE_LOOKUP_QMI_INST_TLV_ID  0x10
#define QMI_WDS_ROUTE_LOOKUP_MUX_ID_TLV_ID    0x13

#define QMI_WDS_PROFILE_TECH_TYPE_TLV_ID                    0x01
#define QMI_WDS_DEFAULT_PROFILE_NUMBER_RSP_TLV_ID           0x01
#define QMI_WDS_PROFILE_IDENTIFIER_TLV_ID                   0x01
#define QMI_WDS_PROFILE_RESET_PROFILE_TLV_ID                0x01
#define QMI_WDS_RESET_PROFILE_PARAMT_TLV_ID                 0x01

/*Extended error codes TLV IDs*/
#define QMI_WDS_EXTENDED_ERROR_CODE_TLV_ID                  0xE0

/*Extended embms error codes TLV IDs*/
#define QMI_WDS_EMBMS_EXTEDNED_ERROR_CODE_TLV_ID            0x10

/* Get default settings tech type TLV ID */
#define QMI_WDS_GET_DEFAULT_SETTINGS_TECH_TLV_ID  0x01

/* Additional TLV's for QMI_WDS_GET_RUNTIME_SETTINGS */
#define QMI_WDS_CURR_CALL_PROFILE_ID_TLV_ID                 0x1F
#define QMI_WDS_CURR_CALL_IPV4_GATEWAY_ADDR_TLV_ID          0x20
#define QMI_WDS_CURR_CALL_IPV4_SUBNET_MASK_TLV_ID           0x21
#define QMI_WDS_CURR_CALL_PCSCF_ADDR_PCO_TLV_ID             0x22
#define QMI_WDS_CURR_CALL_PCSCF_SERVER_ADDR_LIST_TLV_ID     0x23
#define QMI_WDS_CURR_CALL_PCSCF_FQDN_LIST_TLV_ID            0x24
#define QMI_WDS_CURR_CALL_IPV6_ADDR_TLV_ID                  0x25
#define QMI_WDS_CURR_CALL_IPV6_GATEWAY_ADDR_TLV_ID          0x26
#define QMI_WDS_CURR_CALL_PRIMARY_DNS_IPV6_ADDR_TLV_ID      0x27
#define QMI_WDS_CURR_CALL_SECONDARY_DNS_IPV6_ADDR_TLV_ID    0x28
#define QMI_WDS_CURR_CALL_MTU_TLV_ID                        0x29
#define QMI_WDS_CURR_DOMAIN_NAME_LIST_TLV_ID                0x2A
#define QMI_WDS_CURR_IP_FAMILY_TLV_ID                       0x2B
#define QMI_WDS_CURR_IM_CN_FLAG_TLV_ID                      0x2C
#define QMI_WDS_CURR_TECHNOLOGY_TLV_ID                      0x2D
#define QMI_WDS_CURR_CALL_PCSCF_IPV6_SERVER_ADDR_LIST_TLV_ID   0x2E
#define QMI_WDS_CURR_CALL_IPV4_ADDR_TLV_ID                  QMI_WDS_UMTS_PROFILE_IPV4_ADDR_PREF_TLV_ID
#define QMI_WDS_CURR_CALL_PRIMARY_DNS_IPV4_ADDR_TLV_ID      QMI_WDS_UMTS_PROFILE_PRIM_DNS_TLV_ID
#define QMI_WDS_CURR_CALL_SECONDARY_DNS_IPV4_ADDR_TLV_ID    QMI_WDS_UMTS_PROFILE_SEC_DNS_TLV_ID
#define QMI_WDS_CURR_CALL_OPERATOR_RESERVED_PCO_TLV_ID      0x2F

/* Profile list TLV */
#define QMI_WDS_PROFILE_LIST_TLV_ID           0x01

/* Event Registration TLV ID */

#define QMI_WDS_IFACE_EVENT_REGISTRATION_TLV_ID      0x01

#define QMI_WDS_REQUEST_RUNTIME_SETTINGS_TLV_ID      0x10

/* Event report related request TLV ID's */
#define QMI_WDS_EVENT_RPT_CHAN_RATE_REQ_TLV_ID              0x10
#define QMI_WDS_EVENT_RPT_XFER_STATS_REQ_TLV_ID             0x11
#define QMI_WDS_EVENT_RPT_BEARER_TECH_REQ_TLV_ID            0x15
#define QMI_WDS_EVENT_RPT_DORM_STATUS_REQ_TLV_ID            0x13
#define QMI_WDS_EVENT_RPT_DATA_CAPABILITIES_REQ_TLV_ID      0x16
#define QMI_WDS_EVENT_RPT_DATA_CALL_STATUS_CHG_REQ_TLV_ID   0x17
#define QMI_WDS_EVENT_RPT_PREF_DATA_SYS_REQ_TLV_ID          0x18
#define QMI_WDS_EVENT_RPT_DATA_SYS_STATUS_REQ_TLV_ID        0x1A
#define QMI_WDS_EVENT_RPT_LIMITED_DATA_SYS_STATUS_REQ_TLV_ID  0x1C
#define QMI_WDS_EVENT_RPT_BEARER_TECH_EX_REQ_TLV_ID         0x1E

#define QMI_WDS_GET_PKT_STATS_REQ_TLV_ID          0x01

/* Event report related indication TLV ID's */
#define QMI_WDS_EVENT_RPT_XFER_STATS_TX_PKTS_GOOD_IND_TLV_ID      0x10
#define QMI_WDS_EVENT_RPT_XFER_STATS_RX_PKTS_GOOD_IND_TLV_ID      0x11
#define QMI_WDS_EVENT_RPT_XFER_STATS_TX_PKTS_ERR_IND_TLV_ID       0x12
#define QMI_WDS_EVENT_RPT_XFER_STATS_RX_PKTS_ERR_IND_TLV_ID       0x13
#define QMI_WDS_EVENT_RPT_XFER_STATS_TX_OVERFLOW_IND_TLV_ID       0x14
#define QMI_WDS_EVENT_RPT_XFER_STATS_RX_OVERFLOW_IND_TLV_ID       0x15
#define QMI_WDS_EVENT_RPT_XFER_STATS_TX_BYTES_GOOD_IND_TLV_ID     0x19
#define QMI_WDS_EVENT_RPT_XFER_STATS_RX_BYTES_GOOD_IND_TLV_ID     0x1A
#define QMI_WDS_EVENT_RPT_XFER_STATS_TX_PKTS_DROPPED_IND_TLV_ID   0x25
#define QMI_WDS_EVENT_RPT_XFER_STATS_RX_PKTS_DROPPED_IND_TLV_ID   0x26
#define QMI_WDS_EVENT_RPT_CHAN_RATE_IND_TLV_ID                    0x16
#define QMI_WDS_EVENT_RPT_DORM_STATUS_IND_TLV_ID                  0x18
#define QMI_WDS_EVENT_RPT_BEARER_TECH_IND_TLV_ID                  0x1D
#define QMI_WDS_EVENT_RPT_DATA_CAPABILITIES_TLV_ID                0x1E
#define QMI_WDS_EVENT_RPT_DATA_CALL_STATUS_CHG_TLV_ID             0x1F
#define QMI_WDS_EVENT_RPT_PREF_DATA_SYS_TLV_ID                    0x20
#define QMI_WDS_EVENT_RPT_IFACE_NAME_TLV_ID                       0x21
#define QMI_WDS_EVENT_RPT_DATA_CALL_TYPE_TLV_ID                   0x22
#define QMI_WDS_EVENT_RPT_DATA_SYS_STATUS_TLV_ID                  0x24
#define QMI_WDS_EVENT_RPT_DATA_CALL_ADDR_FAMILY_TLV_ID            0x28
#define QMI_WDS_EVENT_RPT_BEARER_TECH_IND_EX_TLV_ID               0x2A

#define QMI_WDS_PKT_STATS_TX_DROPPED_TLV_ID                      0x1D
#define QMI_WDS_PKT_STATS_RX_DROPPED_TLV_ID                      0x1E

/* WDS Internal iface event indication TLV ID's */

#define QMI_WDS_INTERNAL_IFACE_EVENT_IND_EVENT_NAME_TLV_ID                       0x01
#define QMI_WDS_INTERNAL_IFACE_EVENT_OUTAGE_INFO_TLV_ID                          0x10
#define QMI_WDS_INTERNAL_IFACE_EVENT_XTENDED_IP_CONFIG_TLV_ID                    0x11
#define QMI_WDS_INTERNAL_IFACE_EVENT_HDR_REV0_RATE_INTERTIA_FAILURE_TLV_ID       0x12
#define QMI_WDS_INTERNAL_IFACE_EVENT_HDR_SET_EIDLE_SLOTTED_MODE_FAILURE_TLV_ID   0x13
#define QMI_WDS_INTERNAL_IFACE_EVENT_HDR_SET_EIDLE_SLOTTED_MODE_SESSION_TLV_ID   0x14
#define QMI_WDS_INTERNAL_IFACE_EVENT_RF_CONDITIONS_TLV_ID                        0x15
#define QMI_WDS_INTERNAL_IFACE_EVENT_DOS_ACK_INFO_TLV_ID                         0x16

/* Wds Mcast indiacation TLV IDS*/

#define QMI_WDS_MCAST_STATUS_IND_TLV_ID            0x01
#define QMI_WDS_MCAST_STATUS_REASON_CODE_TLV_ID    0x10

/* Dormancy status */
#define QMI_WDS_DORMANCY_STATUS_TLV_ID             0x01

/* Get DUN call info TLV ID's */
#define QMI_WDS_DUN_CALL_INFO_REQ_INFO_TLV_ID      0x01

/* These DUN TLV ID's will be shared between requests, responses
** and inidications
*/
#define QMI_WDS_DUN_CALL_INFO_CONN_STATUS_IND_TLV_ID        0x10
#define QMI_WDS_DUN_CALL_INFO_TX_RX_STATS_IND_TLV_ID        0x11

/*TLV IDs for the indication message*/
#define QMI_WDS_DUN_CALL_INFO_TX_OK_BYTE_COUNT_IND_TLV_ID        0x12
#define QMI_WDS_DUN_CALL_INFO_RX_OK_BYTE_COUNT_IND_TLV_ID        0x13

#define QMI_WDS_DUN_CALL_INFO_CONN_STATUS_RSP_TLV_ID                        0x10
#define QMI_WDS_DUN_CALL_INFO_TX_OK_BYTE_COUNT_RSP_TLV_ID                   0x12
#define QMI_WDS_DUN_CALL_INFO_RX_OK_BYTE_COUNT_RSP_TLV_ID                   0x13
#define QMI_WDS_DUN_CALL_INFO_LAST_CALL_TX_OK_BYTE_COUNT_RSP_TLV_ID         0x17
#define QMI_WDS_DUN_CALL_INFO_LAST_CALL_RX_OK_BYTE_COUNT_RSP_TLV_ID         0x18

/* SIZE of TX_OK_BYTES/ RX_OK_BYTES */
#define QMI_WDS_DUN_CALL_INFO_TX_OK_BYTE_COUNT_SIZE   8
#define QMI_WDS_DUN_CALL_INFO_RX_OK_BYTE_COUNT_SIZE   8

/*MIP*/
#define QMI_WDS_MOBILE_IP_MODE_TLV_ID               0x01
#define QMI_WDS_MOBILE_IP_PROFILE_INDEX_TLV_ID      0x01
/*Read MIP profile*/
#define QMI_WDS_READ_MOBILE_IP_PROFILE_HA_KEY_STATE_TLV_ID      0x1A
#define QMI_WDS_READ_MOBILE_IP_PROFILE_AAA_KEY_STATE_TLV_ID     0x1B
#define QMI_WDS_READ_MOBILE_IP_PROFILE_NAI_TLV_ID               0x15

#define QMI_WDS_MODIFY_MOBILE_IP_PROFILE_STATE_TLV_ID                           0x10
#define QMI_WDS_MODIFY_MOBILE_IP_PROFILE_HOME_ADDR_TLV_ID                       0x11
#define QMI_WDS_MODIFY_MOBILE_IP_PROFILE_PRIMARY_HOME_AGENT_ADDR_TLV_ID         0x12
#define QMI_WDS_MODIFY_MOBILE_IP_PROFILE_SECONDARY_HOME_AGENT_ADDR_TLV_ID       0x13
#define QMI_WDS_MODIFY_MOBILE_IP_PROFILE_REVERSE_TUNNEL_PREF_TLV_ID             0x14
#define QMI_WDS_MODIFY_MOBILE_IP_PROFILE_NAI_TLV_ID                             0x15
#define QMI_WDS_MODIFY_MOBILE_IP_PROFILE_HA_SPI_TLV_ID                          0x16
#define QMI_WDS_MODIFY_MOBILE_IP_PROFILE_AAA_SPI_TLV_ID                         0x17
#define QMI_WDS_MODIFY_MOBILE_IP_PROFILE_MN_HA_KEY_TLV_ID                       0x18
#define QMI_WDS_MODIFY_MOBILE_IP_PROFILE_MN_AAA_KEY_TLV_ID                      0x19

/*Miscelleaneous Defines for processing RF conditions*/
#define QMI_WDS_FOR_RF_COND_CDMA_TYPE            0x01
#define QMI_WDS_FOR_RF_COND_UMTS_TYPE            0x02
#define QMI_WDS_FOR_RF_COND_CDMA_1X              0x01
#define QMI_WDS_FOR_RF_COND_CDMA_EVDO_REV0       0x02
#define QMI_WDS_FOR_RF_COND_CDMA_EVDO_REVA       0x04

/* EMBMS indication registration TLVs */
#define QMI_WDS_REPORT_EMBMS_TMGI_LIST_TLV_ID             0x10
#define QMI_WDS_REPORT_LTE_ATTACH_PDN_LIST_CHANGE_TLV_ID  0x13
#define QMI_WDS_REPORT_EMBMS_SAI_LIST_TLV_ID              0x17
#define QMI_WDS_REPORT_EXT_IP_CONFIG_CHANGE_TLV_ID        0x12
#define QMI_WDS_REPORT_DATA_PATH_CHANGE_TLV_ID            0x16
#define QMI_WDS_REPORT_EMBMS_CONTENT_DESC_CONTROL_TLV_ID  0x1C


/* Handoff indication registration TLV */
#define QMI_WDS_REPORT_HANDOFF_TLV_ID            0x15

/* Handoff info indication TLVs */
#define QMI_WDS_HANDOFF_INFO_IND_TLV             0x01
#define QMI_WDS_HANDOFF_INFO_IND_RAT_INFO_TLV    0x10


/* EMBMS TMGI Activate Status Indication TLVs */
#define QMI_WDS_EMBMS_TMGI_ACTIVATE_IND_STATUS_TLV_ID   0x01
#define QMI_WDS_EMBMS_TMGI_ACTIVATE_IND_LIST_TLV_ID     0x02
#define QMI_WDS_EMBMS_TMGI_ACTIVATE_IND_TRANX_ID_TLV_ID 0x10

/* EMBMS TMGI Deactivate Status Indication TLVs */
#define QMI_WDS_EMBMS_TMGI_DEACTIVATE_IND_STATUS_TLV_ID    0x01
#define QMI_WDS_EMBMS_TMGI_DEACTIVATE_IND_LIST_TLV_ID      0x02
#define QMI_WDS_EMBMS_TMGI_DEACTIVATE_IND_TRANX_ID_TLV_ID  0x10

/* EMBMS TMGI activate_deactivate Status Indication TLVs */
#define QMI_WDS_EMBMS_TMGI_ACT_DEACT_IND_ACTIVATE_STATUS_TLV_ID   0x01
#define QMI_WDS_EMBMS_TMGI_ACT_DEACT_IND_ACTIVATE_LIST_TLV_ID     0x02
#define QMI_WDS_EMBMS_TMGI_ACT_DEACT_IND_DEACTIVATE_STATUS_TLV_ID 0x03
#define QMI_WDS_EMBMS_TMGI_ACT_DEACT_IND_DEACTIVATE_LIST_TLV_ID   0x04
#define QMI_WDS_EMBMS_TMGI_ACT_DEACT_IND_TRANX_ID_TLV_ID          0x10

/* EMBMS TMGI List Indication TLVs */
#define QMI_WDS_EMBMS_LIST_IND_TMGI_LIST_TLV_ID         0x10
#define QMI_WDS_EMBMS_LIST_IND_OOS_WARNING_TLV_ID       0x11
#define QMI_WDS_EMBMS_LIST_IND_TRANX_ID_TLV_ID          0x12

/* EMBMS SAI LIST Indication TLVs */
#define QMI_WDS_EMBMS_LIST_IND_SAI_LIST_TLV_ID          0x10
#define QMI_WDS_EMBMS_LIST_IND_SAI_LIST_TRANX_ID_TLV_ID 0x11

/* EMBMS Content desc control Indication TLVs */
#define QMI_WDS_EMBMS_CONTENT_DESC_CONTROL_IND_TMGI_LIST_TLV_ID     0x01
#define QMI_WDS_EMBMS_CONTENT_DESC_CONTROL_IND_TRANX_ID_TLV_ID      0x10
#define QMI_WDS_EMBMS_CONTENT_DESC_CONTROL_IND_CONTENT_CTRL_TLV_ID  0x11
#define QMI_WDS_EMBMS_CONTENT_DESC_CONTROL_IND_STATUS_CTRL_TLV_ID   0x12

/* EMBMS TMGI Activate */
#define QMI_WDS_EMBMS_ACTIVATE_REQ_TMGI_TLV_ID          0x01
#define QMI_WDS_EMBMS_ACTIVATE_REQ_TRANX_ID_TLV_ID      0x10
#define QMI_WDS_EMBMS_ACTIVATE_REQ_PREEMPT_PRI_TLV_ID   0x11
#define QMI_WDS_EMBMS_ACTIVATE_REQ_EARFCNLIST_TLV_ID    0x12
#define QMI_WDS_EMBMS_ACTIVATE_REQ_SAILIST_TLV_ID       0x13

/* EMBMS TMGI Deactivate */
#define QMI_WDS_EMBMS_DEACTIVATE_REQ_TLV_ID             0x01
#define QMI_WDS_EMBMS_DEACTIVATE_REQ_TRANX_ID_TLV_ID    0x10

/* EMBMS TMGI Activate-Deactivate */
#define QMI_WDS_EMBMS_ACT_DEACT_REQ_ACTIVATE_TMGI_TLV_ID   0x01
#define QMI_WDS_EMBMS_ACT_DEACT_REQ_DEACTIVATE_TMGI_TLV_ID 0x02
#define QMI_WDS_EMBMS_ACT_DEACT_REQ_TRANX_ID_TLV_ID        0x10
#define QMI_WDS_EMBMS_ACT_DEACT_REQ_PREEMPT_PRI_TLV_ID     0x11
#define QMI_WDS_EMBMS_ACT_DEACT_REQ_EARFCNLIST_TLV_ID      0x12
#define QMI_WDS_EMBMS_ACT_DEACT_REQ_SAILIST_TLV_ID         0x13

/* EMBMS Content Desc update request */
#define QMI_WDS_EMBMS_CONT_DESC_UPDATE_REQ_TMGI_TLV_ID      0x01
#define QMI_WDS_EMBMS_CONT_DESC_UPDATE_REQ_TRANX_ID_TLV_ID  0x02
#define QMI_WDS_EMBMS_CONT_DESC_UPDATE_REQ_TLV_OFFSET       0x10

/* EMBMS TMGI List Query request */
#define QMI_WDS_EMBMS_LIST_QUERY_REQ_LIST_TYPE_TLV_ID       0x01
#define QMI_WDS_EMBMS_LIST_QUERY_REQ_TRANX_ID_TLV_ID        0x10

/* EMBMS TMGI List Query response */
#define QMI_WDS_EMBMS_LIST_QUERY_RESP_TMGI_LIST_TLV_ID      0x10

/* Get Pref Data System related */
#define QMI_WDS_GET_PREF_DATA_SYS_RESP_CUR_SYS_TLV_ID   0x10

/* Get Current Data System Status related */
#define QMI_WDS_GET_CURRENT_DATA_SYS_STATUS_TLV_ID      0x10

/* Get LTE Max Attach PDN related */
#define QMI_WDS_GET_LTE_MAX_NUM_ATTACH_PDNS_TLV_ID      0x10

/* Get/Set LTE Attach PDN List related */
#define QMI_WDS_SET_LTE_ATTACH_PDN_LIST_TLV_ID          0x01
#define QMI_WDS_GET_LTE_ATTACH_PDN_LIST_TLV_ID          0x10

#define QMI_WDS_LTE_ATTACH_LIST_IND_TLV_ID              0x10

/* Get LTE Attach Params related */
#define QMI_WDS_GET_LTE_ATTACH_PARAMS_APN_STRING_TLV_ID           0x10
#define QMI_WDS_GET_LTE_ATTACH_PARAMS_IP_TYPE_TLV_ID              0x11
#define QMI_WDS_GET_LTE_ATTACH_PARAMS_OTA_ATTACH_PERFORMED_TLV_ID 0x12

/* Get/Set LTE Data Retry related */
#define QMI_WDS_SET_LTE_DATA_RETRY_TLV_ID               0x01
#define QMI_WDS_GET_LTE_DATA_RETRY_TLV_ID               0x10

/* Get/Set LTE Data Retry related */
#define QMI_WDS_SET_LTE_ATTACH_TYPE_TLV_ID              0x01
#define QMI_WDS_GET_LTE_ATTACH_TYPE_TLV_ID              0x10

/* Timeout (in seconds) of the start nw if synchronous call */
#define QMI_WDS_SYNC_START_NW_IF_TIMEOUT         60

/* Reverse IP Transport related TLVs */
#define QMI_WDS_REV_IP_TRANS_CONN_IND_REG_TLV_ID         0x01

#define QMI_WDS_REV_IP_TRANS_CONFIG_COMPL_RESULT_TLV_ID  0x01
#define QMI_WDS_REV_IP_TRANS_CONFIG_COMPL_TXN_ID_TLV_ID  0x02

#define QMI_WDS_REV_IP_TRANS_CONN_STATUS_TLV_ID          0x01
#define QMI_WDS_REV_IP_TRANS_TXN_ID_TLV_ID               0x02
#define QMI_WDS_REV_IP_TRANS_TECH_NAME_TLV_ID            0x10
#define QMI_WDS_REV_IP_TRANS_SA_CONFIG_SHARED_TLV_ID     0x11
#define QMI_WDS_REV_IP_TRANS_IPV4_ADDR_TLV_ID            0x12
#define QMI_WDS_REV_IP_TRANS_IPV4_SUBNET_MASK_TLV_ID     0x13
#define QMI_WDS_REV_IP_TRANS_IPV6_ADDR_TLV_ID            0x14

#define QMI_WDS_REV_IP_TRANS_GET_SA_SPI_RX_TLV_ID                   0x10
#define QMI_WDS_REV_IP_TRANS_GET_SA_SPI_TX_TLV_ID                   0x11
#define QMI_WDS_REV_IP_TRANS_GET_SA_PROTO_TLV_ID                    0x12
#define QMI_WDS_REV_IP_TRANS_GET_SA_ENCAP_MODE_TLV_ID               0x13
#define QMI_WDS_REV_IP_TRANS_GET_SA_DEST_ADDR_TLV_ID                0x14
#define QMI_WDS_REV_IP_TRANS_GET_SA_LOCAL_ADDR_TLV_ID               0x15
#define QMI_WDS_REV_IP_TRANS_GET_SA_HASH_ALGO_TLV_ID                0x16
#define QMI_WDS_REV_IP_TRANS_GET_SA_HASH_KEY_RX_TLV_ID              0x17
#define QMI_WDS_REV_IP_TRANS_GET_SA_HASH_KEY_TX_TLV_ID              0x18
#define QMI_WDS_REV_IP_TRANS_GET_SA_CRYPTO_ALGO_TLV_ID              0x19
#define QMI_WDS_REV_IP_TRANS_GET_SA_CRYPTO_KEY_RX_TLV_ID            0x1A
#define QMI_WDS_REV_IP_TRANS_GET_SA_CRYPTO_KEY_TX_TLV_ID            0x1B
#define QMI_WDS_REV_IP_TRANS_GET_SA_IV_TLV_ID                       0x1C
#define QMI_WDS_REV_IP_TRANS_GET_SA_UDP_ENCAP_TLV_ID                0x1D
#define QMI_WDS_REV_IP_TRANS_GET_SA_NAT_LOCAL_IP_TLV_ID             0x1E
#define QMI_WDS_REV_IP_TRANS_GET_SA_NAT_REMOTE_IP_TLV_ID            0x1F
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_ADDR_TLV_ID       0x20
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_MASK_TLV_ID       0x21
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_DNS_TLV_ID        0x22
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_NBNS_TLV_ID       0x23
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_ADDR_EXPIRY_TLV_ID     0x24
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_DHCP_TLV_ID       0x25
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_APP_VER_TLV_ID         0x26
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_ADDR_TLV_ID       0x27
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_DNS_TLV_ID        0x28
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_NBNS_TLV_ID       0x29
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_DHCP_TLV_ID       0x2A
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_SUBNET_TLV_ID     0x2B
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_SUPP_ATTR_TLV_ID       0x2C
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_SUBNET_TLV_ID     0x2D
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_PCSCF_TLV_ID      0x2E
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_PCSCF_TLV_ID      0x2F
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_3GPP2_IPV4_HA_TLV_ID   0x30
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_3GPP2_IPV4_HOA_TLV_ID  0x31
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_3GPP2_IPV6_HA_TLV_ID   0x32
#define QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_3GPP2_IPV6_HOA_TLV_ID  0x33
#define QMI_WDS_REV_IP_TRANS_GET_SA_TRAFFIC_SEL_TLV_ID              0x34
#define QMI_WDS_REV_IP_TRANS_GET_SA_TRAFFIC_SEL_RESPONDER_TLV_ID    0x35
#define QMI_WDS_REV_IP_TRANS_GET_AES_MODE_TLV_ID                    0x37

/* Current bearer technology ext related TLVs */
#define QMI_WDS_CURRENT_DATA_BEARER_TECH_EX_CURRENT_CALL_TECH_TLV_ID 0x10

/* Bind MUX data port TLVs */
#define QMI_WDS_BIND_MUX_DATA_PORT_EP_ID_TLV_ID                     0x10
#define QMI_WDS_BIND_MUX_DATA_PORT_MUX_ID_TLV_ID                    0x11
#define QMI_WDS_BIND_MUX_DATA_PORT_REVERSED_TLV_ID                  0x12

#define QMI_WDS_BIND_SUB_ID_TLV_ID     0x01
#define QMI_WDS_GET_BIND_SUB_ID_TLV_ID     0x10

/* Extended IP config indication related TLVs */
#define QMI_WDS_EXT_IP_CONFIG_CHANGE_IND_TLV_ID                     0x10

/* Set data path related TLVs */
#define QMI_WDS_SET_DATA_PATH_TLV_ID  0x01

/* Get data path related TLVs */
#define QMI_WDS_PREF_DATA_PATH_TLV_ID    0x10
#define QMI_WDS_ACTUAL_DATA_PATH_TLV_ID  0x11

/* Data path change ind related TLVs */
#define QMI_WDS_PREF_DATA_PATH_IND_TLV_ID    0x01
#define QMI_WDS_ACTUAL_DATA_PATH_IND_TLV_ID  0x02

/* Global mutex used during allocation/de-allocation of client data structures */
static QMI_PLATFORM_MUTEX_DATA_TYPE qmi_wds_client_data_list_mutex;

typedef struct qmi_wds_client_data_type
{
  struct qmi_wds_client_data_type *next;
  int    client_user_handle;
  struct
  {
    unsigned long pkt_data_handle;
  } data_call;
} qmi_wds_client_data_type;

static qmi_wds_client_data_type *qmi_wds_client_data_list = NULL;

static int wds_service_initialized = FALSE;

/*===========================================================================
  FUNCTION  qmi_wds_embms_tmgi_release_mem
===========================================================================*/
/*!
@brief
  release dynamically allocated memory.

@return
  None
*/
/*=========================================================================*/
static void
qmi_wds_embms_tmgi_release_mem
(
  qmi_wds_embms_tmgi_type *tmgi_list_ptr
)
{
  if (NULL != tmgi_list_ptr )
  {
    free (tmgi_list_ptr);
    tmgi_list_ptr = NULL;
  }
}

/*===========================================================================
  FUNCTION  qmi_wds_get_client_data
===========================================================================*/
/*!
@brief
  Finds the client data structure based on the user handle.  Note that it
  is assumed that locking of appropriate mutex is done outside the scope
  of this function.

@return
  Pointer to client data structure or NULL if not found

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Stores pkt_data_handle in client data structure
*/
/*=========================================================================*/
static qmi_wds_client_data_type *
qmi_wds_get_client_data
(
  int user_handle
)
{
  qmi_wds_client_data_type *client_data,*prev;
  QMI_SLL_FIND (client_data,prev,qmi_wds_client_data_list,(client_data->client_user_handle == user_handle));
  (void)prev;  /* Keep lint happy */
  return client_data;
}


/*===========================================================================
  FUNCTION  qmi_wds_util_read_ext_err_code
===========================================================================*/
/*!
@brief
  reads the ext_err_code from the message response and sets it to qmi_err_code.

@return  QMI_INTERNAL_ERR on error and QMI_NO_ERR on success
@note
*/
/*=========================================================================*/
int
qmi_wds_util_read_ext_err_code
(
  unsigned char               **msg_buf,
  int                         *msg_buf_size,
  int                         *qmi_err_code
)
{
  unsigned long     type;
  unsigned long     length;
  unsigned char     *value_ptr;
  int rc = QMI_NO_ERR;

  if (!msg_buf || !*msg_buf || !qmi_err_code)
  {
    QMI_ERR_MSG_0 ("qmi_wds_util_read_ext_err_code: Bad Input received");
    return QMI_INTERNAL_ERR;
  }

  while (*msg_buf_size > 0)
  {
    if (qmi_util_read_std_tlv (msg_buf,
                               msg_buf_size,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      rc = QMI_INTERNAL_ERR;
      break;
    }

    if (type == QMI_WDS_EXTENDED_ERROR_CODE_TLV_ID)
    {
      unsigned short temp;
      READ_16_BIT_VAL (value_ptr,temp);
      *qmi_err_code = temp;
    }
    else if (type == QMI_WDS_EMBMS_EXTEDNED_ERROR_CODE_TLV_ID)
    {
      unsigned short temp;
      READ_16_BIT_VAL (value_ptr, temp);
      *qmi_err_code = temp;
    }
    else
    {
      QMI_ERR_MSG_1 ("qmi_wds_util_read_ext_err_code: unknown TLV returned = %x",(int)type);
      rc = QMI_INTERNAL_ERR;
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_pkt_srvc_ind
===========================================================================*/
/*!
@brief
  Processes a packet service indication TLV and translates it into
  C structure indication data

@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_pkt_srvc_ind
(
  unsigned char                 *rx_buf,
  int                           rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;


   /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }
  ind_data->pkt_srvc_status.param_mask = 0;
  /* Call end reason is optional, so initialize to UNDEFINED */
  ind_data->pkt_srvc_status.call_end_reason.legacy_reason \
    = QMI_WDS_CE_REASON_UNDEFINED;
  ind_data->pkt_srvc_status.call_end_reason.call_end_reason_verbose. \
    verbose_reason = QMI_WDS_VERBOSE_CE_INVALID;
  ind_data->pkt_srvc_status.call_end_reason.call_end_reason_verbose. \
    verbose_reason_type = QMI_WDS_CE_TYPE_INVALID;
  ind_data->pkt_srvc_status.ip_family = QMI_IP_FAMILY_PREF_UNSPECIFIED;

  /* Loop through all TLV's and process each one */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                                  &rx_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Now process TLV */
    switch (type)
    {
      case QMI_WDS_PKT_SRVC_STATUS_TLV_ID:
      {
        unsigned char temp;
        READ_8_BIT_VAL (value_ptr,temp);
        ind_data->pkt_srvc_status.link_status = (qmi_wds_link_status_type) temp;

        READ_8_BIT_VAL (value_ptr,temp);
        ind_data->pkt_srvc_status.reconfig_required = (int) temp;
      }
      break;

      case QMI_WDS_PKT_SRVC_CALL_END_TLV_ID:
      {
        unsigned int temp;
        ind_data->pkt_srvc_status.param_mask |= QMI_WDS_PKT_SRVC_IND_CALL_END_REASON;
        READ_16_BIT_VAL (value_ptr,temp);
        ind_data->pkt_srvc_status.call_end_reason.legacy_reason
                                                    = (qmi_wds_ce_reason_legacy_type) temp;
      }
      break;

      case QMI_WDS_PKT_SRVC_CALL_END_VERBOSE_TLV_ID:
        {
          ind_data->pkt_srvc_status.param_mask |= QMI_WDS_PKT_SRVC_IND_CALL_END_REASON;
          unsigned short tmp_short;
          READ_16_BIT_VAL (value_ptr,tmp_short);
          ind_data->pkt_srvc_status.call_end_reason.call_end_reason_verbose.verbose_reason_type
                                                          = (qmi_wds_verbose_ce_reason_type)tmp_short;
          READ_16_BIT_VAL (value_ptr,tmp_short);
          ind_data->pkt_srvc_status.call_end_reason.call_end_reason_verbose.verbose_reason
                                                          = (qmi_wds_verbose_ce_reason_codes)tmp_short;
        }
        break;

      case QMI_WDS_PKT_SRVC_IP_FAMILY_TLV_ID:
        {
          unsigned char ip_family;
          ind_data->pkt_srvc_status.param_mask |= QMI_WDS_PKT_SRVC_IND_IP_FAMILY;
          READ_8_BIT_VAL(value_ptr, ip_family);
          ind_data->pkt_srvc_status.ip_family = (qmi_ip_family_pref_type)ip_family;
        }
        break;
      case QMI_WDS_PKT_SRVC_TECH_NAME_TLV_ID:
        {
          unsigned short tech_name;
          ind_data->pkt_srvc_status.param_mask |= QMI_WDS_PKT_SRVC_IND_TECH_NAME;
          READ_16_BIT_VAL(value_ptr, tech_name);
          ind_data->pkt_srvc_status.tech_name = (qmi_wds_iface_name_type) tech_name;
        }
        break;
      case QMI_WDS_PKT_SRVC_XLAT_CAP_TLV_ID:
        {
          unsigned char xlat_capable = 0;
          ind_data->pkt_srvc_status.param_mask |= QMI_WDS_PKT_SRVC_IND_XLAT_CAP;
          READ_8_BIT_VAL(value_ptr, xlat_capable);
          ind_data->pkt_srvc_status.xlat_capable = xlat_capable;
          break;
        }
      default:
        break;
    }
  }
  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_bearer_tech
===========================================================================*/
/*!
@brief
  Processes the bearer technology related TLVs and translates it into a
  data structure

@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_bearer_tech
(
  unsigned char                  **value_ptr,
  qmi_wds_data_bearer_tech_type  *bearer_info
)
{
  unsigned char temp_8bit;
  unsigned long temp_32bit;
  unsigned char *tmp_val_ptr;

  if (!value_ptr || !(*value_ptr) || !bearer_info)
  {
    QMI_ERR_MSG_0 ("qmi_wds_srvc_process_bearer_tech: bad param(s)");
    return QMI_INTERNAL_ERR;
  }

  tmp_val_ptr = *value_ptr;

  /* Read the current_nw value */
  READ_8_BIT_VAL (tmp_val_ptr, temp_8bit);
  bearer_info->current_db_nw = (qmi_wds_data_bearer_type) temp_8bit;

  /* Read the rat_mask value */
  READ_32_BIT_VAL (tmp_val_ptr,temp_32bit);

  if (bearer_info->current_db_nw == QMI_WDS_CDMA_TYPE)
  {
    bearer_info->rat_mask.cdma_rat_mask = (qmi_wds_cdma_rat_mask)temp_32bit;
  }
  else
  {
    bearer_info->rat_mask.umts_rat_mask = (qmi_wds_umts_rat_mask)temp_32bit;
  }

  if (bearer_info->current_db_nw == QMI_WDS_UNKNOWN_TYPE ||
      bearer_info->current_db_nw == QMI_WDS_UMTS_TYPE    ||
      bearer_info->rat_mask.cdma_rat_mask == CDMA_EHRPD  ||
      bearer_info->rat_mask.cdma_rat_mask == CDMA_FMC)
  {
    bearer_info->db_so_mask.so_mask_value = 0;
    /*Make Sure the value stored is zero*/
  }

  /* Read the so_mask value */
  READ_32_BIT_VAL (tmp_val_ptr,temp_32bit);

  if (bearer_info->rat_mask.cdma_rat_mask == CDMA_1X)
  {
    bearer_info->db_so_mask.so_mask_1x = (qmi_wds_cdma_1x_db_so_mask)temp_32bit;
  }
  else if (bearer_info->rat_mask.cdma_rat_mask == CDMA_EVDO_REV0)
  {
    bearer_info->db_so_mask.so_mask_evdo_rev0 = (qmi_wds_cdma_evdo_rev0_db_so_mask)temp_32bit;
  }
  else if (bearer_info->rat_mask.cdma_rat_mask == CDMA_EVDO_REVA)
  {
    bearer_info->db_so_mask.so_mask_evdo_reva = (qmi_wds_cdma_evdo_reva_db_so_mask)temp_32bit;
  }
  else if (bearer_info->rat_mask.cdma_rat_mask == CDMA_EVDO_REVB)
  {
    bearer_info->db_so_mask.so_mask_evdo_revb = (qmi_wds_cdma_evdo_revb_db_so_mask)temp_32bit;
  }

  /* Update to the next TVL to read */
  *value_ptr = tmp_val_ptr;

  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_bearer_tech_ex
===========================================================================*/
/*!
@brief
  Processes the bearer technology related TLVs and translates it into a
  data structure

@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_bearer_tech_ex
(
  unsigned char                     **value_ptr,
  qmi_wds_data_bearer_tech_type_ex  *bearer_info
)
{
  unsigned long temp_32bit;
  uint64_t      temp_64bit;
  unsigned char *tmp_val_ptr;

  if (!value_ptr || !(*value_ptr) || !bearer_info)
  {
    QMI_ERR_MSG_0 ("qmi_wds_srvc_process_bearer_tech_ex: bad param(s)");
    return QMI_INTERNAL_ERR;
  }

  tmp_val_ptr = *value_ptr;

  /* Read the technology value */
  READ_32_BIT_VAL (tmp_val_ptr, temp_32bit);
  bearer_info->technology = (qmi_wds_data_bearer_type_ex) temp_32bit;

  /* Read the rat_mask value */
  READ_32_BIT_VAL (tmp_val_ptr,temp_32bit);
  bearer_info->rat_mask = (qmi_wds_bearer_tech_rat_mask_ex)temp_32bit;

  /* Read the so_mask value */
  READ_64_BIT_VAL (tmp_val_ptr,temp_64bit);
  bearer_info->so_mask = (qmi_wds_bearer_tech_so_mask_ex)temp_64bit;

  /* Update to the next TVL to read */
  *value_ptr = tmp_val_ptr;

  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_data_sys_status_tlvs
===========================================================================*/
/*!
@brief
  Processes the Data System Status related TLVs and translates it into a
  data structure indication

@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_data_sys_status_tlvs
(
  unsigned char                              **value_ptr,
  qmi_wds_data_network_type                  *pref_network,
  qmi_wds_data_sys_status_network_info_type  **network_info,
  unsigned int                               *network_info_len
)
{
  unsigned char temp_8bit;
  unsigned long temp_32bit;
  unsigned char i, qmi_nw_info_len = 0, qmi_pref_network;
  qmi_wds_data_sys_status_network_info_type  *nw_info = NULL;
  unsigned char *tmp_val_ptr;


  if (!value_ptr || !(*value_ptr) || !pref_network || !network_info || !network_info_len)
  {
    QMI_ERR_MSG_0 ("qmi_wds_srvc_process_data_sys_status_tlvs: bad param(s)");
    return QMI_INTERNAL_ERR;
  }

  /* Initialize output params to defaults */
  *pref_network = QMI_WDS_DATA_NETWORK_TYPE_INVALID;

  tmp_val_ptr = *value_ptr;

  /* Read the Preferred Network value */
  READ_8_BIT_VAL (tmp_val_ptr, qmi_pref_network);

  /* Read the network info length value */
  READ_8_BIT_VAL (tmp_val_ptr, qmi_nw_info_len);

  /* Determine if we need to allocate memory for the network_info array */
  if (NULL == *network_info)
  {
    if (qmi_nw_info_len > 0)
    {
      qmi_nw_info_len = (unsigned char) MIN(qmi_nw_info_len, QMI_WDS_MAX_DATA_SYS_STATUS_NETWORK_INFO_LEN);

      /* Allocate memory for the network info array */
      nw_info = malloc(sizeof(qmi_wds_data_sys_status_network_info_type) * qmi_nw_info_len);

      if (NULL == nw_info)
      {
        QMI_ERR_MSG_1 ("qmi_wds_srvc_process_data_sys_status_tlvs: Memory allocation failed "
                       "for data_sys_status->network_info, len=%d", qmi_nw_info_len);

        return QMI_INTERNAL_ERR;
      }

      /* Initialize the memory */
      memset(nw_info, 0, sizeof(qmi_wds_data_sys_status_network_info_type) * qmi_nw_info_len);
    }

    *network_info = nw_info;
    *network_info_len = qmi_nw_info_len;
  }
  else
  {
    nw_info = *network_info;
    qmi_nw_info_len = (unsigned char) MIN(*network_info_len, qmi_nw_info_len);
    *network_info_len = qmi_nw_info_len;
  }

  /* Read the set of network info values */
  for (i = 0; i < qmi_nw_info_len; ++i)
  {
    /* Read the network value */
    READ_8_BIT_VAL (tmp_val_ptr, temp_8bit);
    nw_info[i].network = (qmi_wds_data_network_type) temp_8bit;

    /* Read the rat_mask value */
    READ_32_BIT_VAL (tmp_val_ptr,temp_32bit);

    if (nw_info[i].network == QMI_WDS_DATA_NETWORK_TYPE_3GPP2)
    {
      nw_info[i].rat_mask.cdma_rat_mask = (qmi_wds_cdma_rat_mask)temp_32bit;
    }
    else
    {
      nw_info[i].rat_mask.umts_rat_mask = (qmi_wds_umts_rat_mask)temp_32bit;
    }

    if (nw_info[i].network == QMI_WDS_DATA_NETWORK_TYPE_INVALID ||
        nw_info[i].network == QMI_WDS_DATA_NETWORK_TYPE_3GPP    ||
        nw_info[i].rat_mask.cdma_rat_mask == CDMA_EHRPD         ||
        nw_info[i].rat_mask.cdma_rat_mask == CDMA_FMC)
    {
      nw_info[i].db_so_mask.so_mask_value = 0;
      /*Make Sure the value stored is zero*/
    }

    /* Read the so_mask value */
    READ_32_BIT_VAL (tmp_val_ptr,temp_32bit);

    if (nw_info[i].rat_mask.cdma_rat_mask == CDMA_1X)
    {
      nw_info[i].db_so_mask.so_mask_1x = (qmi_wds_cdma_1x_db_so_mask)temp_32bit;
    }
    else if (nw_info[i].rat_mask.cdma_rat_mask == CDMA_EVDO_REV0)
    {
      nw_info[i].db_so_mask.so_mask_evdo_rev0 = (qmi_wds_cdma_evdo_rev0_db_so_mask)temp_32bit;
    }
    else if (nw_info[i].rat_mask.cdma_rat_mask == CDMA_EVDO_REVA)
    {
      nw_info[i].db_so_mask.so_mask_evdo_reva = (qmi_wds_cdma_evdo_reva_db_so_mask)temp_32bit;
    }
    else if (nw_info[i].rat_mask.cdma_rat_mask == CDMA_EVDO_REVB)
    {
      nw_info[i].db_so_mask.so_mask_evdo_revb = (qmi_wds_cdma_evdo_revb_db_so_mask)temp_32bit;
    }
  }

  /* Update the pref_network */
  *pref_network = (qmi_wds_data_network_type) qmi_pref_network;

  /* Update to the next TLV to read */
  *value_ptr = tmp_val_ptr;

  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_event_report_ind
===========================================================================*/
/*!
@brief
  Processes a event report TLVs and translates it into
  C structure indication data

@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_event_report_ind
(
  unsigned char                 *rx_buf,
  int                           rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{

  qmi_wds_event_report_type     *event_report;
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  /* Set up pointer to event report data */
  event_report = &ind_data->event_report;

  /* Initialize event mask and xfer_stats mask */
  event_report->event_mask = 0;
  event_report->xfer_stats.mask = 0;
  event_report->data_capabilities[0] = 0;
  event_report->data_capabilities[1] = 0;
  event_report->iface_name = QMI_WDS_IFACE_NAME_NOT_REPORTED;

  /* Initialize data system status */
  event_report->data_sys_status.pref_network = QMI_WDS_DATA_NETWORK_TYPE_INVALID;
  event_report->data_sys_status.network_info = NULL;
  event_report->data_sys_status.network_info_len = 0;

  /* initialize data call status change ip address family */
  event_report->data_call_status_change.data_call_addr_family = QMI_WDS_IP_UNKNOWN;

  /* Loop through all TLV's and process each one */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                                  &rx_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Now process TLV */
    switch (type)
    {
      case QMI_WDS_EVENT_RPT_XFER_STATS_TX_PKTS_GOOD_IND_TLV_ID:
      {
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_XFER_STATS_IND;
        /* Set appropriate bit in xfer_stats mask field */
        event_report->xfer_stats.mask |= QMI_WDS_XFER_STATS_TX_PKTS_GOOD;

        /* Read in value */
        READ_32_BIT_VAL (value_ptr, event_report->xfer_stats.tx_good_pkt_cnt);
      }
      break;

      case QMI_WDS_EVENT_RPT_XFER_STATS_RX_PKTS_GOOD_IND_TLV_ID:
      {
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_XFER_STATS_IND;
        /* Set appropriate bit in xfer_stats mask field */
        event_report->xfer_stats.mask |= QMI_WDS_XFER_STATS_RX_PKTS_GOOD;

        /* Read in value */
        READ_32_BIT_VAL (value_ptr, event_report->xfer_stats.rx_good_pkt_cnt);
      }
      break;

      case QMI_WDS_EVENT_RPT_XFER_STATS_TX_PKTS_ERR_IND_TLV_ID:
      {
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_XFER_STATS_IND;
        /* Set appropriate bit in xfer_stats mask field */
        event_report->xfer_stats.mask |= QMI_WDS_XFER_STATS_TX_PKTS_ERR;

        /* Read in value */
        READ_32_BIT_VAL (value_ptr, event_report->xfer_stats.tx_err_pkt_cnt);
      }
      break;

      case QMI_WDS_EVENT_RPT_XFER_STATS_RX_PKTS_ERR_IND_TLV_ID:
      {
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_XFER_STATS_IND;
        /* Set appropriate bit in xfer_stats mask field */
        event_report->xfer_stats.mask |= QMI_WDS_XFER_STATS_RX_PKTS_ERR;

        /* Read in value */
        READ_32_BIT_VAL (value_ptr, event_report->xfer_stats.rx_err_pkt_cnt);
      }
      break;

      case QMI_WDS_EVENT_RPT_XFER_STATS_TX_OVERFLOW_IND_TLV_ID:
      {
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_XFER_STATS_IND;
        /* Set appropriate bit in xfer_stats mask field */
        event_report->xfer_stats.mask |= QMI_WDS_XFER_STATS_TX_OVERFLOW;

        /* Read in value */
        READ_32_BIT_VAL (value_ptr, event_report->xfer_stats.tx_overflow_cnt);
      }
      break;

      case QMI_WDS_EVENT_RPT_XFER_STATS_TX_BYTES_GOOD_IND_TLV_ID:
        {
          /* Set bit in event mask */
          event_report->event_mask |= QMI_WDS_EVENT_XFER_STATS_IND;
          /* Set appropriate bit in xfer_stats mask field */
          event_report->xfer_stats.mask |= QMI_WDS_XFER_STATS_TX_BYTES_OK;

          /* Read in value */
          READ_64_BIT_VAL (value_ptr, event_report->xfer_stats.tx_good_byte_cnt);
        }
        break;

      case QMI_WDS_EVENT_RPT_XFER_STATS_RX_BYTES_GOOD_IND_TLV_ID:
        {
          /* Set bit in event mask */
          event_report->event_mask |= QMI_WDS_EVENT_XFER_STATS_IND;
          /* Set appropriate bit in xfer_stats mask field */
          event_report->xfer_stats.mask |= QMI_WDS_XFER_STATS_RX_BYTES_OK;

          /* Read in value */
          READ_64_BIT_VAL (value_ptr, event_report->xfer_stats.rx_good_byte_cnt);
        }
        break;

      case QMI_WDS_EVENT_RPT_XFER_STATS_RX_OVERFLOW_IND_TLV_ID:
      {
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_XFER_STATS_IND;
        /* Set appropriate bit in xfer_stats mask field */
        event_report->xfer_stats.mask |= QMI_WDS_XFER_STATS_RX_OVERFLOW;

        /* Read in value */
        READ_32_BIT_VAL (value_ptr, event_report->xfer_stats.rx_overflow_cnt);
      }
      break;

      case QMI_WDS_EVENT_RPT_XFER_STATS_TX_PKTS_DROPPED_IND_TLV_ID:
      {
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_XFER_STATS_IND;
        /* Set appropriate bit in xfer_stats mask field */
        event_report->xfer_stats.mask |= QMI_WDS_XFER_STATS_TX_PACKETS_DROPPED;

        /* Read in value */
        READ_32_BIT_VAL (value_ptr, event_report->xfer_stats.tx_pkts_dropped);
      }
      break;

      case QMI_WDS_EVENT_RPT_XFER_STATS_RX_PKTS_DROPPED_IND_TLV_ID:
      {
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_XFER_STATS_IND;
        /* Set appropriate bit in xfer_stats mask field */
        event_report->xfer_stats.mask |= QMI_WDS_XFER_STATS_RX_PACKETS_DROPPED;

        /* Read in value */
        READ_32_BIT_VAL (value_ptr, event_report->xfer_stats.rx_pkts_dropped);
      }
      break;

      case QMI_WDS_EVENT_RPT_CHAN_RATE_IND_TLV_ID:
      {
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_CHAN_RATE_IND;
        /* Read TX and RX channel rate values */
        READ_32_BIT_VAL (value_ptr, event_report->chan_rate.tx_curr_channel_rate);
        READ_32_BIT_VAL (value_ptr, event_report->chan_rate.rx_curr_channel_rate);
      }
      break;

      case QMI_WDS_EVENT_RPT_BEARER_TECH_IND_TLV_ID:
      {
        if (QMI_NO_ERR == qmi_wds_srvc_process_bearer_tech(&value_ptr,
                                                           &event_report->data_bearer_tech_type))
        {
          /* Set bit in event mask */
          event_report->event_mask |= QMI_WDS_EVENT_BEARER_TECH_IND;
        }
      }
      break;

      case QMI_WDS_EVENT_RPT_BEARER_TECH_IND_EX_TLV_ID:
      {
        if (QMI_NO_ERR == qmi_wds_srvc_process_bearer_tech_ex(&value_ptr,
                                                              &event_report->data_bearer_tech_type_ex))
        {
          /* set bit in event mask */
          event_report->event_mask |= QMI_WDS_EVENT_BEARER_TECH_EX_IND;
        }
      }
      break;

      case QMI_WDS_EVENT_RPT_DORM_STATUS_IND_TLV_ID:
      {
        unsigned char temp;
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_DORM_STATUS_IND;
        /* Read dormancy status value */
        READ_8_BIT_VAL (value_ptr, temp);
        event_report->dorm_status = (qmi_wds_dorm_status_type) temp;
      }
      break;

      case QMI_WDS_EVENT_RPT_DATA_CAPABILITIES_TLV_ID:
      {
        unsigned long temp;
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_DATA_CAPABILITIES_IND;

        READ_32_BIT_VAL (value_ptr, temp);
        event_report->data_capabilities[0] = temp;
        READ_32_BIT_VAL (value_ptr, temp);
        event_report->data_capabilities[1] = temp;

      }
      break;

      case QMI_WDS_EVENT_RPT_DATA_CALL_STATUS_CHG_TLV_ID:
      {
        unsigned short temp;
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_DATA_CALL_STATUS_CHG_IND;

        READ_8_BIT_VAL (value_ptr, temp);
        event_report->data_call_status_change.data_call_status = (qmi_wds_data_call_status_type) temp;
      }
      break;

      case QMI_WDS_EVENT_RPT_PREF_DATA_SYS_TLV_ID:
      {
        unsigned short temp;
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_PREF_DATA_SYS_IND;

        READ_8_BIT_VAL (value_ptr, temp);
        event_report->pref_data_sys = (qmi_wds_pref_data_sys_type) temp;

      }
      break;

      case QMI_WDS_EVENT_RPT_IFACE_NAME_TLV_ID:
      {
        unsigned short temp;
        READ_16_BIT_VAL (value_ptr, temp);
        event_report->iface_name = (qmi_wds_iface_name_type) temp;

      }
      break;

      case QMI_WDS_EVENT_RPT_DATA_CALL_TYPE_TLV_ID:
      {
        unsigned short temp;
        /* Set bit in event mask */
        event_report->event_mask |= QMI_WDS_EVENT_DATA_CALL_TYPE_IND;

        READ_8_BIT_VAL (value_ptr, temp);
        event_report->data_call_type.data_call_type = (qmi_wds_data_call_type_type) temp;

        READ_8_BIT_VAL (value_ptr, temp);
        event_report->data_call_type.tethered_call_type = (qmi_wds_tethered_call_type_type) temp;

      }
      break;

      case QMI_WDS_EVENT_RPT_DATA_SYS_STATUS_TLV_ID:
      {
        int ret = qmi_wds_srvc_process_data_sys_status_tlvs(&value_ptr,
                                                            &event_report->data_sys_status.pref_network,
                                                            &event_report->data_sys_status.network_info,
                                                            &event_report->data_sys_status.network_info_len);

        if (QMI_NO_ERR != ret)
        {
          QMI_ERR_MSG_1 ("Call to qmi_wds_srvc_process_data_sys_status_tlvs() failed err=%d",
                         ret);
          return ret;
        }
        else
        {
          /* Set bit in event mask */
          event_report->event_mask |= QMI_WDS_EVENT_DATA_SYS_STATUS_IND;
        }
      }
      break;

      case QMI_WDS_EVENT_RPT_DATA_CALL_ADDR_FAMILY_TLV_ID:
      {
        unsigned long temp;
        READ_32_BIT_VAL (value_ptr, temp);

        event_report->data_call_status_change.data_call_addr_family = (qmi_wds_ip_family)temp;
      }
      break;

      default:
      {
        QMI_ERR_MSG_2 ("qmi_wds_srvc_process_event_report_ind: Unknown TLV ID=%x, len=%d",
                                                            (unsigned int)type,(int)length);
      }

    } /* switch */
  } /* while */

  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_internal_iface_event_ind
===========================================================================*/
/*!
@brief
  Processes a internal iface event indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
int
qmi_wds_srvc_process_internal_iface_event_ind
(
unsigned char                 *rx_buf,
int                            rx_buf_len,
qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  qmi_wds_internal_iface_event_ind_data_type *event_report;

  event_report = &ind_data->iface_event_report;
  event_report->param_mask = 0;
  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  /* Loop through all TLV's and process each one */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                                  &rx_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
    switch (type)
    {
      case QMI_WDS_INTERNAL_IFACE_EVENT_IND_EVENT_NAME_TLV_ID:
        {
          int   temp;
          READ_16_BIT_VAL (value_ptr,temp);
          event_report->iface_event_name = (qmi_wds_iface_event_name)temp;
        }
        break;

      case QMI_WDS_INTERNAL_IFACE_EVENT_OUTAGE_INFO_TLV_ID:
        {
          event_report->param_mask |= QMI_WDS_OUTAGE_INFO_PARAM_TYPE;
          READ_32_BIT_VAL (value_ptr,event_report->outage_information.time_to_outage);
          READ_32_BIT_VAL (value_ptr,event_report->outage_information.duration);
        }
        break;

      case QMI_WDS_INTERNAL_IFACE_EVENT_XTENDED_IP_CONFIG_TLV_ID:
        {
          unsigned char temp;
          event_report->param_mask |= QMI_WDS_XTENDED_IP_CONFIG_PARAM_TYPE;
          READ_8_BIT_VAL (value_ptr,temp);
          event_report->extended_ip_config_status = (qmi_wds_extended_ip_config_status)temp;
        }
        break;

      case QMI_WDS_INTERNAL_IFACE_EVENT_HDR_REV0_RATE_INTERTIA_FAILURE_TLV_ID:
        {
          unsigned char temp;
          event_report->param_mask |= QMI_WDS_HDR_REV0_INTERTIA_FAILURE_PARAM_TYPE;
          READ_8_BIT_VAL (value_ptr,temp);
          event_report->hdr_rev0_rate_inertia_failure_status = (qmi_wds_hdr_related_failure_codes)temp;
        }
        break;

      case QMI_WDS_INTERNAL_IFACE_EVENT_HDR_SET_EIDLE_SLOTTED_MODE_FAILURE_TLV_ID:
        {
          unsigned char temp;
          event_report->param_mask |= QMI_WDS_HDR_SET_EIDLE_SLOT_MODE_FAIL_PARAM_TYPE;
          READ_8_BIT_VAL (value_ptr,temp);
          event_report->hdr_set_eidle_slottedmode_failure_status = (qmi_wds_hdr_related_failure_codes)temp;
        }
        break;

      case QMI_WDS_INTERNAL_IFACE_EVENT_HDR_SET_EIDLE_SLOTTED_MODE_SESSION_TLV_ID:
        {
          event_report->param_mask |= QMI_WDS_HDR_SET_EIDLE_SLOT_MODE_SESSION_CHANGED_PARAM_TYPE;
          READ_8_BIT_VAL (value_ptr,event_report->hdr_set_eidle_slot_cycle_changed);
        }
        break;

      case QMI_WDS_INTERNAL_IFACE_EVENT_RF_CONDITIONS_TLV_ID:
        {
          unsigned char temp_8bit;
          unsigned long temp_32bit;
          event_report->param_mask |= QMI_WDS_RF_CONDITIONS_PARAM_TYPE;

          READ_8_BIT_VAL (value_ptr,temp_8bit);
          event_report->rf_conditions.current_db_nw = (qmi_wds_data_bearer_type)temp_8bit;

          if ((int)event_report->rf_conditions.current_db_nw == QMI_WDS_FOR_RF_COND_CDMA_TYPE)
          {
            READ_32_BIT_VAL (value_ptr,temp_32bit);
            event_report->rf_conditions.rat_mask.cdma_rat_mask = (qmi_wds_cdma_rat_mask)temp_32bit;
          }
          else
          {
            READ_32_BIT_VAL (value_ptr,temp_32bit);
            event_report->rf_conditions.rat_mask.umts_rat_mask = (qmi_wds_umts_rat_mask)temp_32bit;
          }
          if ((int)event_report->rf_conditions.current_db_nw == QMI_WDS_FOR_RF_COND_UMTS_TYPE
              || (int)event_report->rf_conditions.rat_mask.cdma_rat_mask == QMI_WDS_FOR_RF_COND_CDMA_EVDO_REV0)
          {
            event_report->rf_conditions.db_so_mask.so_mask_value = 0;
            /*Make Sure the value stored is zero*/
          }
          if ((int)event_report->rf_conditions.rat_mask.cdma_rat_mask == QMI_WDS_FOR_RF_COND_CDMA_1X)
          {
            READ_32_BIT_VAL (value_ptr,temp_32bit);
            event_report->rf_conditions.db_so_mask.so_mask_1x = (qmi_wds_cdma_1x_db_so_mask)temp_32bit;
          }
          else if ((int)event_report->rf_conditions.rat_mask.cdma_rat_mask == QMI_WDS_FOR_RF_COND_CDMA_EVDO_REVA)
          {
            READ_32_BIT_VAL (value_ptr,temp_32bit);
            event_report->rf_conditions.db_so_mask.so_mask_evdo_reva = (qmi_wds_cdma_evdo_db_so_mask)temp_32bit;
          }

          READ_8_BIT_VAL (value_ptr,temp_8bit);
          event_report->rf_conditions.rf_conditions = (qmi_wds_rf_conditions)temp_8bit;
        }
        break;

      case QMI_WDS_INTERNAL_IFACE_EVENT_DOS_ACK_INFO_TLV_ID:
        {
          event_report->param_mask |= QMI_WDS_DOS_ACK_INFO_PARAM_TYPE;
          READ_16_BIT_VAL (value_ptr,event_report->dos_ack_information.handle);
          READ_32_BIT_VAL (value_ptr,event_report->dos_ack_information.overflow);
          READ_32_BIT_VAL (value_ptr,event_report->dos_ack_information.dos_ack_status);
        }
        break;

      default:
        {
          QMI_ERR_MSG_2 ("qmi_wds_srvc_process_internal_iface_event_ind: Unknown TLV ID=%x, len=%d",
                         (unsigned int)type,(int)length);
          return QMI_INTERNAL_ERR;
        }
        //break;
    }
  }
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_mt_request_ind
===========================================================================*/
/*!
@brief
  Processes a internal iface event indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
int
qmi_wds_srvc_process_mt_request_ind
(
unsigned char                 *rx_buf,
int                            rx_buf_len,
qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                                  &rx_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
    switch (type)
    {
      case QMI_WDS_MT_PACKET_HNDL:
        {
          READ_32_BIT_VAL (value_ptr,ind_data->mt_handle);
        }
        break;
      default:
        {
          QMI_ERR_MSG_2 ("qmi_wds_srvc_process_mt_request_ind: Unknown TLV ID=%x, len=%d",
                         (unsigned int)type,(int)length);
          return QMI_INTERNAL_ERR;
        }
        //break;
    }
  }
  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_mcast_status_ind
===========================================================================*/
/*!
@brief
  Processes a internal iface event indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
int
qmi_wds_srvc_process_mcast_status_ind
(
unsigned char                 *rx_buf,
int                            rx_buf_len,
qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }
  ind_data->mcast_status.param_mask = 0;

  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                                  &rx_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
    switch (type)
    {
      case QMI_WDS_MCAST_STATUS_IND_TLV_ID:
        {
          unsigned char temp;
          READ_32_BIT_VAL (value_ptr,ind_data->mcast_status.multicast_status.mcast_handle);
          READ_8_BIT_VAL (value_ptr,temp);
          ind_data->mcast_status.multicast_status.mcast_status = (qmi_wds_mcast_status)temp;
        }
        break;
      case QMI_WDS_MCAST_STATUS_REASON_CODE_TLV_ID:
        {
          unsigned char temp;
          ind_data->mcast_status.param_mask |= QMI_WDS_MCAST_REASON_CODE_PARAM;
          READ_8_BIT_VAL (value_ptr,temp);
          ind_data->mcast_status.reason_code = (qmi_wds_mcast_info_reason_code)temp;
        }
        break;
      default:
        {
          QMI_ERR_MSG_2 ("qmi_wds_srvc_process_mcast_status_ind: Unknown TLV ID=%x, len=%d",
                         (unsigned int)type,(int)length);
          return QMI_INTERNAL_ERR;
        }
        //break;
    }
  }
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_mbms_mcast_status_ind
===========================================================================*/
/*!
@brief
  Processes a internal iface event indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
int
qmi_wds_srvc_process_mbms_mcast_status_ind
(
unsigned char                 *rx_buf,
int                            rx_buf_len,
qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }
  ind_data->mbms_mcast_context_status.param_mask = 0;

  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                                  &rx_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
    switch (type)
    {
      case QMI_WDS_MBMS_MCAST_CONTEXT_STATUS_IND_TLV_ID:
        {
          unsigned char temp;
          READ_8_BIT_VAL (value_ptr,temp);
          ind_data->mbms_mcast_context_status.mcast_status = (qmi_wds_mbms_mcast_status)temp;
        }
        break;
      case QMI_WDS_MBMS_MCAST_REASON_CODE_IND_TLV_ID:
        {
          unsigned char temp;
          ind_data->mbms_mcast_context_status.param_mask |= QMI_WDS_MBMS_MCAST_REASON_CODE_PARAM;
          READ_8_BIT_VAL (value_ptr,temp);
          ind_data->mbms_mcast_context_status.reason_code = (qmi_wds_mbms_mcast_reason_code)temp;
        }
        break;
      default:
        {
          QMI_ERR_MSG_2 ("qmi_wds_srvc_process_mbms_mcast_status_ind: Unknown TLV ID=%x, len=%d",
                         (unsigned int)type,(int)length);
          return QMI_INTERNAL_ERR;
        }
        //break;
    }
  }
  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_dun_call_info_ind
===========================================================================*/
/*!
@brief
  Processes a internal iface event indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_dun_call_info_ind
(
  unsigned char                 *rx_buf,
  int                            rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  /* Clear out info_mask */
  ind_data->dun_call_info.ind_info_mask = 0;

  /* Loop through looking for TLV's */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                               &rx_buf_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch (type)
    {
      case QMI_WDS_DUN_CALL_INFO_CONN_STATUS_IND_TLV_ID:
        {
          unsigned char temp;
          ind_data->dun_call_info.ind_info_mask |= QMI_WDS_DUN_CALL_INFO_CONN_STATUS_IND_PARAM_MASK;
          READ_8_BIT_VAL (value_ptr,temp);
          ind_data->dun_call_info.conn_state = (qmi_wds_dun_conn_state_type) temp;
        }
        break;
      case QMI_WDS_DUN_CALL_INFO_TX_OK_BYTE_COUNT_IND_TLV_ID:
        {
          unsigned char cp_length;
          if(length != QMI_WDS_DUN_CALL_INFO_TX_OK_BYTE_COUNT_SIZE)
          {
            QMI_ERR_MSG_1(" qmi_wds_srvc_process_dun_call_info_ind: invalude tx_ok_bytes_length = %x\n", length);
            return QMI_INTERNAL_ERR;
          }

          ind_data->dun_call_info.ind_info_mask |= QMI_WDS_DUN_CALL_INFO_TX_OK_BYTE_COUNT_IND_PARAM_MASK;

          cp_length = (QMI_WDS_DUN_CALL_INFO_TX_OK_BYTE_COUNT_SIZE < sizeof(ind_data->dun_call_info.tx_ok_byte_cnt))?
              QMI_WDS_DUN_CALL_INFO_TX_OK_BYTE_COUNT_SIZE : sizeof(ind_data->dun_call_info.tx_ok_byte_cnt);

          /* copy only what is allowed size */
          memcpy(&ind_data->dun_call_info.tx_ok_byte_cnt,value_ptr,cp_length);
        }
        break;
      case QMI_WDS_DUN_CALL_INFO_RX_OK_BYTE_COUNT_IND_TLV_ID:
        {
          unsigned char cp_length;
          if(length != QMI_WDS_DUN_CALL_INFO_RX_OK_BYTE_COUNT_SIZE)
          {
            QMI_ERR_MSG_1(" qmi_wds_srvc_process_dun_call_info_ind: invalid rx_ok_bytes_length = %x\n", length);
            return QMI_INTERNAL_ERR;
          }

          ind_data->dun_call_info.ind_info_mask |= QMI_WDS_DUN_CALL_INFO_RX_OK_BYTE_COUNT_IND_PARAM_MASK;

          cp_length = (QMI_WDS_DUN_CALL_INFO_RX_OK_BYTE_COUNT_SIZE < sizeof(ind_data->dun_call_info.rx_ok_byte_cnt))?
            QMI_WDS_DUN_CALL_INFO_RX_OK_BYTE_COUNT_SIZE : sizeof(ind_data->dun_call_info.rx_ok_byte_cnt);

          /* copy only what is allowed size */
          memcpy(&ind_data->dun_call_info.rx_ok_byte_cnt,value_ptr, cp_length);
        }
        break;
      default:
        QMI_ERR_MSG_1 ("qmi_wds_srvc_process_dun_call_info_ind: unknown type = %x\n",(unsigned) type);
        break;
    } /* switch */
  } /* while */

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_embms_tmgi_activate_status_ind
===========================================================================*/
/*!
@brief
  Processes a internal iface event indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_embms_tmgi_activate_status_ind
(
  unsigned char                 *rx_buf,
  int                            rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  qmi_wds_embms_tmgi_info_type activation_tmgi;

  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  ind_data->embms_activate_status.param_mask = 0;

  memset(&activation_tmgi, 0, sizeof(activation_tmgi));

  /* Loop through looking for TLV's */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                               &rx_buf_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch (type)
    {
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_IND_STATUS_TLV_ID:
        {
          unsigned long temp_32bit;
          READ_32_BIT_VAL (value_ptr, temp_32bit);
          ind_data->embms_activate_status.activate_status =
            (qmi_wds_embms_tmgi_activate_status_type)temp_32bit;
        }
        break;

      case QMI_WDS_EMBMS_TMGI_ACTIVATE_IND_LIST_TLV_ID:
        {
          unsigned char temp;

          activation_tmgi.tmgi_list_len =
            QMI_WDS_EMBMS_TMGI_STATUS_IND_LIST_SIZE;

          /* allocate memory */
          activation_tmgi.tmgi_list_ptr = (qmi_wds_embms_tmgi_type *)
                                           malloc( sizeof(qmi_wds_embms_tmgi_type));

          if( NULL != activation_tmgi.tmgi_list_ptr)
          {
            /* zero out memory */
            memset(activation_tmgi.tmgi_list_ptr,
                   0,
                   sizeof(qmi_wds_embms_tmgi_type));

            memcpy(activation_tmgi.tmgi_list_ptr->tmgi,
                   value_ptr,
                   QMI_WDS_EMBMS_TMGI_SIZE);

            value_ptr += QMI_WDS_EMBMS_TMGI_SIZE;

            READ_8_BIT_VAL (value_ptr, temp);
            activation_tmgi.tmgi_list_ptr->session_id_valid = temp;

            READ_8_BIT_VAL (value_ptr, temp);
            activation_tmgi.tmgi_list_ptr->session_id = temp;

            ind_data->embms_activate_status.activation_tmgi = activation_tmgi;
          }
          else
          {
            QMI_ERR_MSG_0("qmi_wds_srvc_process_embms_tmgi_activate_status_ind: "
                          "could not allocate memory\n");
            return QMI_INTERNAL_ERR;
          }
        }
        break;

      case QMI_WDS_EMBMS_TMGI_ACTIVATE_IND_TRANX_ID_TLV_ID:
        {
          ind_data->embms_activate_status.param_mask |=
            QMI_WDS_EMBMS_TMGI_ACTIVATE_STATUS_IND_TRANX_ID_PARAM;
          unsigned short temp_16bit;
          READ_16_BIT_VAL (value_ptr,temp_16bit);
          ind_data->embms_activate_status.dbg_trace_id = temp_16bit;
        }
        break;

      default:
        {
        QMI_ERR_MSG_1 ("qmi_wds_srvc_process_embms_tmgi_activate_status_ind: "
                       "unknown type = %x\n",(unsigned) type);
        return QMI_INTERNAL_ERR;
        }
    } /* switch */
  } /* while */

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_embms_tmgi_deactivate_status_ind
===========================================================================*/
/*!
@brief
  Processes a internal iface event indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_embms_tmgi_deactivate_status_ind
(
  unsigned char                 *rx_buf,
  int                            rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  qmi_wds_embms_tmgi_info_type deactivation_tmgi;

  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  ind_data->embms_deactivate_status.param_mask = 0;

  memset(&(deactivation_tmgi), 0, sizeof(deactivation_tmgi));

  /* Loop through looking for TLV's */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                               &rx_buf_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch (type)
    {
      case QMI_WDS_EMBMS_TMGI_DEACTIVATE_IND_STATUS_TLV_ID:
        {
          unsigned long temp_32bit;
          READ_32_BIT_VAL (value_ptr, temp_32bit);
          ind_data->embms_deactivate_status.deactivate_status =
            (qmi_wds_embms_tmgi_deactivate_status_type)temp_32bit;
        }
        break;

      case QMI_WDS_EMBMS_TMGI_DEACTIVATE_IND_LIST_TLV_ID:
        {
          unsigned char temp;

          deactivation_tmgi.tmgi_list_len = QMI_WDS_EMBMS_TMGI_STATUS_IND_LIST_SIZE;

          /* allocate memory */
          deactivation_tmgi.tmgi_list_ptr = (qmi_wds_embms_tmgi_type *)
                                             malloc( sizeof(qmi_wds_embms_tmgi_type));

          if( NULL != deactivation_tmgi.tmgi_list_ptr)
          {
            /* zero out memory */
            memset(deactivation_tmgi.tmgi_list_ptr,
                   0,
                   sizeof(qmi_wds_embms_tmgi_type));

            memcpy(deactivation_tmgi.tmgi_list_ptr->tmgi,
                   value_ptr,
                   QMI_WDS_EMBMS_TMGI_SIZE);

            value_ptr += QMI_WDS_EMBMS_TMGI_SIZE;

            READ_8_BIT_VAL (value_ptr, temp);
            deactivation_tmgi.tmgi_list_ptr->session_id_valid = temp;

            READ_8_BIT_VAL (value_ptr, temp);
            deactivation_tmgi.tmgi_list_ptr->session_id = temp;

            ind_data->embms_deactivate_status.deactivation_tmgi = deactivation_tmgi;
          }
          else
          {
            QMI_ERR_MSG_0 ("qmi_wds_srvc_process_embms_tmgi_deactivate_status_ind: "
                           "could not allocate memory\n");
            return QMI_INTERNAL_ERR;
          }
        }
        break;

      case QMI_WDS_EMBMS_TMGI_DEACTIVATE_IND_TRANX_ID_TLV_ID:
        {
          ind_data->embms_deactivate_status.param_mask |=
            QMI_WDS_EMBMS_TMGI_DEACTIVATE_STATUS_IND_TRANX_ID_PARAM;
          unsigned short temp_16bit;
          READ_16_BIT_VAL (value_ptr,temp_16bit);
          ind_data->embms_deactivate_status.dbg_trace_id = temp_16bit;
        }
        break;

      default:
        {
        QMI_ERR_MSG_1 ("qmi_wds_srvc_process_embms_tmgi_deactivate_status_ind: "
                       "unknown type = %x\n",(unsigned) type);
        return QMI_INTERNAL_ERR;
        }
    } /* switch */
  } /* while */

  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_embms_tmgi_act_deact_status_ind
===========================================================================*/
/*!
@brief
  Processes a internal iface event indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_embms_tmgi_act_deact_status_ind
(
  unsigned char                 *rx_buf,
  int                            rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  qmi_wds_embms_tmgi_info_type activation_tmgi;
  qmi_wds_embms_tmgi_info_type deactivation_tmgi;

  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  ind_data->embms_act_deact_status.param_mask = 0;
  memset(&(activation_tmgi), 0, sizeof(activation_tmgi));
  memset(&(deactivation_tmgi), 0, sizeof(deactivation_tmgi));

  /* Loop through looking for TLV's */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                               &rx_buf_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch (type)
    {
      case QMI_WDS_EMBMS_TMGI_ACT_DEACT_IND_ACTIVATE_STATUS_TLV_ID:
        {
          unsigned long temp_32bit;
          READ_32_BIT_VAL (value_ptr, temp_32bit);
          ind_data->embms_act_deact_status.activate_status =
            (qmi_wds_embms_tmgi_activate_status_type)temp_32bit;
        }
        break;

      case QMI_WDS_EMBMS_TMGI_ACT_DEACT_IND_ACTIVATE_LIST_TLV_ID:
        {
          unsigned char temp;

          activation_tmgi.tmgi_list_len = QMI_WDS_EMBMS_TMGI_STATUS_IND_LIST_SIZE;

          /* allocate memory */
          activation_tmgi.tmgi_list_ptr = (qmi_wds_embms_tmgi_type *)
                                           malloc( sizeof(qmi_wds_embms_tmgi_type));

          if( NULL != activation_tmgi.tmgi_list_ptr)
          {
            /* zero out memory */
            memset(activation_tmgi.tmgi_list_ptr,
                   0,
                   sizeof(qmi_wds_embms_tmgi_type));

            memcpy(activation_tmgi.tmgi_list_ptr->tmgi,
                   value_ptr,
                   QMI_WDS_EMBMS_TMGI_SIZE);

            value_ptr += QMI_WDS_EMBMS_TMGI_SIZE;

            READ_8_BIT_VAL (value_ptr, temp);
            activation_tmgi.tmgi_list_ptr->session_id_valid = temp;

            READ_8_BIT_VAL (value_ptr, temp);
            activation_tmgi.tmgi_list_ptr->session_id = temp;

            ind_data->embms_act_deact_status.activation_tmgi = activation_tmgi;
          }
          else
          {
            QMI_ERR_MSG_0("qmi_wds_srvc_process_embms_tmgi_act_deact_status_ind: "
                          "could not allocate memory\n");
            return QMI_INTERNAL_ERR;
          }
        }
        break;

      case QMI_WDS_EMBMS_TMGI_ACT_DEACT_IND_DEACTIVATE_STATUS_TLV_ID:
        {
          unsigned long temp_32bit;
          READ_32_BIT_VAL (value_ptr, temp_32bit);
          ind_data->embms_act_deact_status.deactivate_status =
            (qmi_wds_embms_tmgi_deactivate_status_type)temp_32bit;
        }
        break;

      case QMI_WDS_EMBMS_TMGI_ACT_DEACT_IND_DEACTIVATE_LIST_TLV_ID:
        {
          unsigned char temp;

          deactivation_tmgi.tmgi_list_len = QMI_WDS_EMBMS_TMGI_STATUS_IND_LIST_SIZE;

          /* allocate memory */
          deactivation_tmgi.tmgi_list_ptr = (qmi_wds_embms_tmgi_type *)
                                             malloc( sizeof(qmi_wds_embms_tmgi_type));

          if( NULL != deactivation_tmgi.tmgi_list_ptr)
          {
            /* zero out memory */
            memset(deactivation_tmgi.tmgi_list_ptr,
                   0,
                   sizeof(qmi_wds_embms_tmgi_type));

            memcpy(deactivation_tmgi.tmgi_list_ptr->tmgi,
                   value_ptr,
                   QMI_WDS_EMBMS_TMGI_SIZE);

            value_ptr += QMI_WDS_EMBMS_TMGI_SIZE;

            READ_8_BIT_VAL (value_ptr, temp);
            deactivation_tmgi.tmgi_list_ptr->session_id_valid = temp;

            READ_8_BIT_VAL (value_ptr, temp);
            deactivation_tmgi.tmgi_list_ptr->session_id = temp;

            ind_data->embms_act_deact_status.deactivation_tmgi = deactivation_tmgi;
          }
          else
          {
            QMI_ERR_MSG_0("qmi_wds_srvc_process_embms_tmgi_act_deact_status_ind: "
                          "could not allocate memory\n");
            return QMI_INTERNAL_ERR;
          }
        }
        break;

      case QMI_WDS_EMBMS_TMGI_ACT_DEACT_IND_TRANX_ID_TLV_ID:
        {
          ind_data->embms_act_deact_status.param_mask |=
            QMI_WDS_EMBMS_TMGI_ACT_DEACT_STATUS_IND_TRANX_ID_PARAM;
          unsigned short temp_16bit;
          READ_16_BIT_VAL (value_ptr,temp_16bit);
          ind_data->embms_act_deact_status.dbg_trace_id = temp_16bit;
        }
        break;

      default:
        {
          QMI_ERR_MSG_1 ("qmi_wds_srvc_process_embms_tmgi_act_deact_status_ind: "
                         "unknown type = %x\n",(unsigned) type);

          return QMI_INTERNAL_ERR;
        }
    } /* switch */
  } /* while */

  return QMI_NO_ERR;

}
/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_embms_tmgi_list_ind
===========================================================================*/
/*!
@brief
  Processes a internal iface event indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_embms_tmgi_list_ind
(
  unsigned char                 *rx_buf,
  int                            rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  int           i;

  qmi_wds_embms_tmgi_info_type tmgi_list;

  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  ind_data->embms_list.param_mask = 0;
  memset(&(tmgi_list), 0, sizeof(tmgi_list));

  /* Loop through looking for TLV's */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                               &rx_buf_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch(type)
    {
      case QMI_WDS_EMBMS_LIST_IND_TMGI_LIST_TLV_ID:
      {
        ind_data->embms_list.param_mask |=
          QMI_WDS_EMBMS_LIST_IND_TMGI_LIST_PARAM_MASK;

        unsigned char temp_len;
        unsigned char temp;
        READ_8_BIT_VAL (value_ptr, temp);
        ind_data->embms_list.list_type = (qmi_wds_embms_tmgi_list_type)temp;

        READ_8_BIT_VAL (value_ptr, temp_len);
        tmgi_list.tmgi_list_len = temp_len;

        if( 0 != tmgi_list.tmgi_list_len)
        {
          /* allocate memory */
          tmgi_list.tmgi_list_ptr = (qmi_wds_embms_tmgi_type *)
                                     malloc(temp_len * sizeof(qmi_wds_embms_tmgi_type));

          if (NULL != tmgi_list.tmgi_list_ptr)
          {
            /* zero out memeory */
            memset(tmgi_list.tmgi_list_ptr,
                   0,
                   temp_len * sizeof(qmi_wds_embms_tmgi_type));

            for(i = 0; i < tmgi_list.tmgi_list_len; i++)
            {
              memcpy(tmgi_list.tmgi_list_ptr[i].tmgi,
                     value_ptr,
                     QMI_WDS_EMBMS_TMGI_SIZE );
              value_ptr += QMI_WDS_EMBMS_TMGI_SIZE;

             READ_8_BIT_VAL (value_ptr, temp);
              tmgi_list.tmgi_list_ptr[i].session_id_valid = temp;

              READ_8_BIT_VAL (value_ptr, temp);
              tmgi_list.tmgi_list_ptr[i].session_id = temp;
            }
          }
          else
          {
             QMI_ERR_MSG_0("qmi_wds_srvc_process_embms_tmgi_list_ind: "
                          "could not allocate memory\n");
             return QMI_INTERNAL_ERR;
          }
        }/* 0 != tmgi_list.tmgi_list_len */

        ind_data->embms_list.tmgi_list = tmgi_list;
      }
      break;

      case QMI_WDS_EMBMS_LIST_IND_OOS_WARNING_TLV_ID:
      {
        ind_data->embms_list.param_mask |=
              QMI_WDS_EMBMS_LIST_IND_OOS_WARNING_PARAM_MASK;
        READ_32_BIT_VAL (value_ptr, ind_data->embms_list.oos_warning_reason);
      }
      break;

      case QMI_WDS_EMBMS_LIST_IND_TRANX_ID_TLV_ID:
        {
          ind_data->embms_list.param_mask |=
            QMI_WDS_EMBMS_LIST_IND_TRANX_ID_PARAM_MASK;
          READ_16_BIT_VAL (value_ptr, ind_data->embms_list.dbg_trace_id);

        }
        break;

      default:
      {
        QMI_ERR_MSG_1 ("qmi_wds_srvc_process_embms_tmgi_list_ind: "
                       "unknown type = %x\n",(unsigned) type);
        return QMI_INTERNAL_ERR;
      }
      break;
    } /*switch*/

  } /* while */

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_embms_sai_list_ind
===========================================================================*/
/*!
@brief
  Processes a internal iface event indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_embms_sai_list_ind
(
  unsigned char                 *rx_buf,
  int                            rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  ind_data->sai_list.param_mask = 0;

  /* Loop through looking for TLV's */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                               &rx_buf_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch(type)
    {
      case QMI_WDS_EMBMS_LIST_IND_SAI_LIST_TRANX_ID_TLV_ID:
      {
        ind_data->sai_list.param_mask |=
          QMI_WDS_EMBMS_LIST_IND_SAI_LIST_TRANX_ID_PARAM_MASK;
        READ_16_BIT_VAL (value_ptr, ind_data->sai_list.dbg_trace_id);
      }
      break;
      case QMI_WDS_EMBMS_LIST_IND_SAI_LIST_TLV_ID:
      {
        ind_data->sai_list.param_mask |=
          QMI_WDS_EMBMS_LIST_IND_SAI_LIST_PARAM_MASK;

        unsigned short  list_len;
        unsigned short  freq;
        unsigned char   is_serving_freq;
        unsigned char   sai_size = 0;
        unsigned short  camped_sai_list_size = 0;
        unsigned short  available_sai_list_size = 0;

        unsigned char   j = 0;
        unsigned char  *orig_value_ptr;
        unsigned short *orig_sai_size_ptr;
        unsigned int   *orig_camped_sai_list_ptr;
        unsigned int   *orig_available_sai_list_ptr;
        int temp_index;
        READ_8_BIT_VAL (value_ptr, list_len);

        /* backup the value_ptr */
        orig_value_ptr = value_ptr;

        /* validate number of freqencies */
        list_len = (unsigned short)((list_len > QMI_WDS_EMBMS_FREQ_MAX_SIZE)?
                                    QMI_WDS_EMBMS_FREQ_MAX_SIZE : list_len);

        /* calculate the size for arrays */
        for (j = 0; j < list_len; j ++)
        {
          READ_16_BIT_VAL(value_ptr, freq);
          READ_8_BIT_VAL(value_ptr, is_serving_freq);
          READ_8_BIT_VAL(value_ptr, sai_size);

          /* validate sai_size */
          if (sai_size > QMI_WDS_EMBMS_SAI_PER_FREQ_MAX_SIZE)
          {
             return QMI_INTERNAL_ERR;
          }

          available_sai_list_size = (unsigned short) (available_sai_list_size + sai_size);
          if (TRUE == is_serving_freq)
          {
            camped_sai_list_size = (unsigned short) (camped_sai_list_size + sai_size);
          }
          value_ptr += (sai_size * sizeof(unsigned int));
        }

        /* assign size value */
        ind_data->sai_list.available_sai_list_len = available_sai_list_size;
        ind_data->sai_list.camped_sai_list_len    = camped_sai_list_size;
        ind_data->sai_list.num_sai_per_group_len  = list_len;

        /* allocate memory for arrays*/
        orig_available_sai_list_ptr =
           (unsigned int*)malloc(available_sai_list_size * sizeof(unsigned int));
        orig_camped_sai_list_ptr =
           (unsigned int*)malloc( camped_sai_list_size * sizeof(unsigned int));
        orig_sai_size_ptr =
           (unsigned short*)malloc(list_len * sizeof(unsigned short));

        /* validate the memory is allocated */
        if (
            (0 != available_sai_list_size && NULL == orig_available_sai_list_ptr) ||
            (0 != camped_sai_list_size && NULL == orig_camped_sai_list_ptr) ||
            (0 != list_len && NULL == orig_sai_size_ptr))
        {
          QMI_ERR_MSG_0("qmi_wds_srvc_process_embms_sai_list_ind: "
                        "failed allocating memory ");

          /* release memory allocated successfully */
          if (NULL != orig_available_sai_list_ptr )
          {
            free(orig_available_sai_list_ptr);
          }
          if ( NULL != orig_camped_sai_list_ptr)
          {
            free(orig_camped_sai_list_ptr);
          }
          if ( NULL != orig_sai_size_ptr)
          {
            free(orig_sai_size_ptr);
          }

          return QMI_INTERNAL_ERR;
        }

        /* assign the allocated memory location */
        ind_data->sai_list.available_sai_list = orig_available_sai_list_ptr;
        ind_data->sai_list.camped_sai_list    = orig_camped_sai_list_ptr;
        ind_data->sai_list.num_sai_per_group  = orig_sai_size_ptr;

        /* move back the ptr to the original location */
        value_ptr = orig_value_ptr;

        for (j = 0; j < list_len; j ++)
        {
          READ_16_BIT_VAL(value_ptr, freq);
          READ_8_BIT_VAL(value_ptr, is_serving_freq);
          READ_8_BIT_VAL(value_ptr, sai_size);

          /* assign value for each array */
          *orig_sai_size_ptr = sai_size;
          orig_sai_size_ptr ++;

          if (TRUE == is_serving_freq)
          {
            memcpy(orig_camped_sai_list_ptr,
                   value_ptr,
                   sai_size * sizeof(unsigned int));
            orig_camped_sai_list_ptr += sai_size;
          }
          memcpy(orig_available_sai_list_ptr,
                 value_ptr,
                 sai_size * sizeof(unsigned int));
          orig_available_sai_list_ptr += sai_size;

          value_ptr += (sai_size * sizeof(unsigned int));
        }

        for (temp_index = 0; temp_index < (ind_data->sai_list.available_sai_list_len); temp_index ++)
        {
           QMI_ERR_MSG_1("available_sai_list:%d", ind_data->sai_list.available_sai_list[temp_index] );
        }
        for (temp_index = 0; temp_index < (ind_data->sai_list.camped_sai_list_len); temp_index ++)
        {
           QMI_ERR_MSG_1("camped_sai_list:%d", ind_data->sai_list.camped_sai_list[temp_index]);
        }
        for (temp_index = 0; temp_index < (ind_data->sai_list.num_sai_per_group_len); temp_index ++)
        {
           QMI_ERR_MSG_1("num_sai_per_group:%d", ind_data->sai_list.num_sai_per_group[temp_index]);
        }

      }
      break;

      default:
      {
        QMI_ERR_MSG_1 ("qmi_wds_srvc_process_embms_sai_list_ind: "
                       "unknown type = %x\n",(unsigned) type);
      }
      break;
    } /*switch*/

  } /* while */

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_embms_content_desc_control_ind
===========================================================================*/
/*!
@brief
  Processes a internal iface event indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_embms_content_desc_control_ind
(
  unsigned char                 *rx_buf,
  int                            rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  int           i;

  qmi_wds_embms_tmgi_info_type tmgi_list;

  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  ind_data->embms_content_desc_control.param_mask = 0;
  memset(&(tmgi_list), 0, sizeof(tmgi_list));

  /* Loop through looking for TLV's */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                               &rx_buf_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch(type)
    {
      case QMI_WDS_EMBMS_CONTENT_DESC_CONTROL_IND_TMGI_LIST_TLV_ID:
      {
        unsigned char temp;

        tmgi_list.tmgi_list_len = QMI_WDS_EMBMS_TMGI_STATUS_IND_LIST_SIZE;

        /* allocate memory */
        tmgi_list.tmgi_list_ptr = (qmi_wds_embms_tmgi_type *)
                                           malloc( sizeof(qmi_wds_embms_tmgi_type));

        if( NULL != tmgi_list.tmgi_list_ptr)
        {
          /* zero out memory */
          memset(tmgi_list.tmgi_list_ptr,
                 0,
                 sizeof(qmi_wds_embms_tmgi_type));

          memcpy(tmgi_list.tmgi_list_ptr->tmgi,
                 value_ptr,
                 QMI_WDS_EMBMS_TMGI_SIZE);

          value_ptr += QMI_WDS_EMBMS_TMGI_SIZE;

          READ_8_BIT_VAL (value_ptr, temp);
          tmgi_list.tmgi_list_ptr->session_id_valid = temp;

          READ_8_BIT_VAL (value_ptr, temp);
          tmgi_list.tmgi_list_ptr->session_id = temp;

          ind_data->embms_content_desc_control.content_desc_tmgi = tmgi_list;
        }
        else
        {
          QMI_ERR_MSG_0 ("qmi_wds_srvc_process_embms_content_desc_control_ind: "
                         "could not allocate memory\n");
          return QMI_INTERNAL_ERR;
        }
      }
      break;

      case QMI_WDS_EMBMS_CONTENT_DESC_CONTROL_IND_CONTENT_CTRL_TLV_ID:
      {
        ind_data->embms_list.param_mask |=
            QMI_WDS_EMBMS_CONTENT_DESC_CTRL_IND_CONTENT_CTRL_PARAM_MASK;
        READ_32_BIT_VAL (value_ptr, ind_data->embms_content_desc_control.content_control);
      }
      break;

      case QMI_WDS_EMBMS_CONTENT_DESC_CONTROL_IND_STATUS_CTRL_TLV_ID:
      {
        ind_data->embms_list.param_mask |=
            QMI_WDS_EMBMS_CONTENT_DESC_CTRL_IND_STATUS_CTRL_PARAM_MASK;
        READ_32_BIT_VAL (value_ptr, ind_data->embms_content_desc_control.status_control);
      }
      break;

      case QMI_WDS_EMBMS_CONTENT_DESC_CONTROL_IND_TRANX_ID_TLV_ID:
      {
        ind_data->embms_list.param_mask |=
            QMI_WDS_EMBMS_CONTENT_DESC_CTRL_IND_TRANX_ID_PARAM_MASK;
        READ_16_BIT_VAL (value_ptr, ind_data->embms_content_desc_control.dbg_trace_id);
      }
      break;

      default:
      {
        QMI_ERR_MSG_1 ("qmi_wds_srvc_process_embms_tmgi_list_ind: "
                       "unknown type = %x\n",(unsigned) type);
        return QMI_INTERNAL_ERR;
      }
      break;
    } /*switch*/

  } /* while */

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_lte_attach_pdn_list_ind
===========================================================================*/
/*!
@brief
  Processes a LTE attach PDN list indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_lte_attach_pdn_list_ind
(
  unsigned char                 *rx_buf,
  int                            rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  int           i;

  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  memset(&ind_data->lte_attach_pdn_list, 0, sizeof(ind_data->lte_attach_pdn_list));

  /* Loop through looking for TLV's */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                               &rx_buf_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch(type)
    {
      case QMI_WDS_LTE_ATTACH_LIST_IND_TLV_ID:
      {
        unsigned char tmp8;
        unsigned short tmp16;
        int max_pdn_list_len;

        READ_8_BIT_VAL (value_ptr, tmp8);
        max_pdn_list_len = MIN(tmp8, QMI_WDS_MAX_LTE_ATTACH_PDN_LIST_SIZE);

        for (i = 0; i < max_pdn_list_len; ++i)
        {
          READ_16_BIT_VAL (value_ptr, tmp16);
          ind_data->lte_attach_pdn_list.list[i] = tmp16;
        }
        ind_data->lte_attach_pdn_list.len = max_pdn_list_len;
      }
      break;

      default:
      {
        QMI_ERR_MSG_1 ("qmi_wds_srvc_process_lte_attach_pdn_list_ind: "
                       "unknown type = %x\n",(unsigned) type);
      }
      break;
    } /*switch*/

  } /* while */

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_rev_ip_trans_ind
===========================================================================*/
/*!
@brief
  Processes reverse adapter indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_rev_ip_trans_ind
(
  unsigned char                 *rx_buf,
  int                            rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  int           i;

  if (!rx_buf || !ind_data)
  {
    return QMI_INTERNAL_ERR;
  }

  ind_data->rev_ip_conn_ind.param_mask = 0;

  /* Loop through looking for TLV's */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                               &rx_buf_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch(type)
    {
      case QMI_WDS_REV_IP_TRANS_CONN_STATUS_TLV_ID:
      {
        unsigned long temp_32bit;

        READ_32_BIT_VAL(value_ptr, temp_32bit);
        ind_data->rev_ip_conn_ind.conn_status = (qmi_wds_rev_ip_trans_conn_status_type) temp_32bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_TXN_ID_TLV_ID:
      {
        unsigned long temp_32bit;

        READ_32_BIT_VAL(value_ptr, temp_32bit);
        ind_data->rev_ip_conn_ind.txn_id = (qmi_wds_txn_id_type) temp_32bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_TECH_NAME_TLV_ID:
      {
        unsigned short temp_16bit;

        READ_16_BIT_VAL(value_ptr, temp_16bit);
        ind_data->rev_ip_conn_ind.param_mask |= QMI_WDS_REV_IP_TRANS_IND_TECH_NAME_PARAM_MASK;
        ind_data->rev_ip_conn_ind.tech_name = (qmi_wds_iface_name_type) temp_16bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_SA_CONFIG_SHARED_TLV_ID:
      {
        ind_data->rev_ip_conn_ind.param_mask |= QMI_WDS_REV_IP_TRANS_IND_SA_CONFIG_SHARED_PARAM_MASK;
        READ_8_BIT_VAL(value_ptr, ind_data->rev_ip_conn_ind.is_sa_shared);
      }
      break;

      case QMI_WDS_REV_IP_TRANS_IPV4_ADDR_TLV_ID:
      {
        ind_data->rev_ip_conn_ind.param_mask |= QMI_WDS_REV_IP_TRANS_IND_IPV4_ADDR_PARAM_MASK;
        READ_32_BIT_VAL (value_ptr, ind_data->rev_ip_conn_ind.ipv4_addr);
      }
      break;

      case QMI_WDS_REV_IP_TRANS_IPV4_SUBNET_MASK_TLV_ID:
      {
        ind_data->rev_ip_conn_ind.param_mask |= QMI_WDS_REV_IP_TRANS_IND_IPV4_SUBNET_MASK_PARAM_MASK;
        READ_32_BIT_VAL (value_ptr, ind_data->rev_ip_conn_ind.ipv4_addr);
      }
      break;

      case QMI_WDS_REV_IP_TRANS_IPV6_ADDR_TLV_ID:
      {
        ind_data->rev_ip_conn_ind.param_mask |= QMI_WDS_REV_IP_TRANS_IND_IPV6_ADDR_PARAM_MASK;

        for (i = 0; i < QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES; ++i)
        {
          READ_8_BIT_VAL (value_ptr, ind_data->rev_ip_conn_ind.ipv6_addr_info.ipv6_addr[i]);
        }

        /* Read V6 prefix length */
        READ_8_BIT_VAL (value_ptr, ind_data->rev_ip_conn_ind.ipv6_addr_info.ipv6_prefix_len);
      }
      break;

      default:
      {
        QMI_ERR_MSG_1 ("qmi_wds_srvc_process_rev_ip_trans_ind: "
                       "unknown type = %x\n",(unsigned) type);
      }
      break;
    } /*switch*/

  } /* while */

  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_ext_ip_config_ind
===========================================================================*/
/*!
@brief
  Processes extended IP config indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_ext_ip_config_ind
(
  unsigned char                 *rx_buf,
  int                            rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  int           i;

  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  memset(&ind_data->ext_ip_ind, 0, sizeof(ind_data->ext_ip_ind));

  /* Loop through looking for TLV's */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                               &rx_buf_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch(type)
    {
      case QMI_WDS_EXT_IP_CONFIG_CHANGE_IND_TLV_ID:
      {
        unsigned long mask;

        READ_32_BIT_VAL (value_ptr, mask);
        ind_data->ext_ip_ind = (qmi_wds_ext_ip_config_ind_type) mask;
      }
      break;

      default:
      {
        QMI_ERR_MSG_1 ("qmi_wds_srvc_process_ext_ip_config_ind: "
                       "unknown type = %x\n",(unsigned) type);
      }
      break;
    } /*switch*/

  } /* while */

  return QMI_NO_ERR;
}

/*==========================================================================
  FUNCTION  qmi_wds_srvc_process_handoff_info_ind
===========================================================================*/
/*!
  @brief
  Processes a handoff indication TLV and translates it into
  C structure indication data

  @return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise
*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_handoff_info_ind
(
  unsigned char                 *rx_buf,
  int                            rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                                  &rx_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch (type)
    {
      case QMI_WDS_HANDOFF_INFO_IND_TLV:
      {
        unsigned char temp;
        READ_8_BIT_VAL (value_ptr,temp);
        ind_data->handoff_info.handoff_info_status = (qmi_wds_handoff_info_ind_type) temp;
      }
      break;
      case QMI_WDS_HANDOFF_INFO_IND_RAT_INFO_TLV:
      {
        unsigned long temp_32bit = 0;
        READ_32_BIT_VAL(value_ptr,temp_32bit);
        ind_data->handoff_info.source_rat = temp_32bit;

        temp_32bit = 0;
        READ_32_BIT_VAL(value_ptr,temp_32bit);
        ind_data->handoff_info.target_rat = temp_32bit;
        break;
      }
      default:
        break;
    }
  }
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_srvc_process_data_path_change_ind
===========================================================================*/
/*!
@brief
  Processes data path change indication TLVs and translates it into
  C structure indication data
@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_wds_srvc_process_data_path_change_ind
(
  unsigned char                 *rx_buf,
  int                            rx_buf_len,
  qmi_wds_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  int           i;

  /* Error check */
  if ((rx_buf == NULL) || (ind_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  ind_data->data_path_ind.pref_data_path = QMI_WDS_DATA_PATH_UNKNOWN;
  ind_data->data_path_ind.actual_data_path = QMI_WDS_DATA_PATH_UNKNOWN;

  /* Loop through looking for TLV's */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                               &rx_buf_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch(type)
    {
      case QMI_WDS_PREF_DATA_PATH_IND_TLV_ID:
      {
        unsigned long tmp32;

        READ_32_BIT_VAL (value_ptr, tmp32);
        ind_data->data_path_ind.pref_data_path = (qmi_wds_data_path_type) tmp32;
      }
      break;

      case QMI_WDS_ACTUAL_DATA_PATH_IND_TLV_ID:
      {
        unsigned long tmp32;

        READ_32_BIT_VAL (value_ptr, tmp32);
        ind_data->data_path_ind.actual_data_path = (qmi_wds_data_path_type) tmp32;
      }
      break;

      default:
      {
        QMI_ERR_MSG_1 ("qmi_wds_srvc_process_data_path_change_ind: "
                       "unknown type = %x\n",(unsigned) type);
      }
      break;
    } /*switch*/

  } /* while */

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_srvc_indication_cb
===========================================================================*/
/*!
@brief
  This is the callback function that will be called by the generic
  services layer to report asynchronous indications.  This function will
  process the indication TLV's and then call the user registered
  functions with the indication data.

@return
  None.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
static void
qmi_wds_srvc_indication_cb
(
  int                     user_handle,
  qmi_service_id_type     service_id,
  unsigned long           msg_id,
  void                                *user_ind_msg_hdlr,
  void                                *user_ind_msg_hdlr_user_data,
  unsigned char           *rx_msg_buf,
  int                     rx_msg_len
)
{
  qmi_wds_indication_id_type      ind_id;
  qmi_wds_indication_data_type    ind_data;
  qmi_wds_indication_hdlr_type    user_ind_hdlr;

  /* Make sure that the user indication handler isn't NULL */
  if (user_ind_msg_hdlr == NULL)
  {
    return;
  }

  /* initialize ind_data */
  memset(&ind_data, 0, sizeof(ind_data));

  /* Determine which indication it is and process appropriately */
  switch (msg_id)
  {
    case QMI_WDS_PKT_SRVC_STATUS_IND_MSG_ID:
    {
      ind_id = QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG;
      if (qmi_wds_srvc_process_pkt_srvc_ind (rx_msg_buf,
                                             rx_msg_len,
                                             &ind_data) < 0)
      {
        return;
      }
    }
    break;

    case QMI_WDS_EVENT_REPORT_IND_MSG_ID:
    {
      ind_id = QMI_WDS_SRVC_EVENT_REPORT_IND_MSG;
      if (qmi_wds_srvc_process_event_report_ind (rx_msg_buf,
                                                 rx_msg_len,
                                                 &ind_data) < 0)
      {
        return;
      }
    }
    break;

    case QMI_WDS_INTERNAL_IFACE_EV_REGISTER_IND_MSG_ID:
      {
        ind_id = QMI_WDS_SRVC_INTERNAL_IFACE_EVNT_REG_MSG;
        if (qmi_wds_srvc_process_internal_iface_event_ind (rx_msg_buf,
                                                           rx_msg_len,
                                                           &ind_data) < 0)
        {
          return;
        }
      }
      break;

    case QMI_WDS_INTERNAL_MT_REQ_IND_MSG_ID:
      {
        ind_id = QMI_WDS_SRVC_MT_REQUEST_IND_MSG;
        if (qmi_wds_srvc_process_mt_request_ind (rx_msg_buf,
                                                 rx_msg_len,
                                                 &ind_data) < 0)
        {
          return;
        }
      }
      break;

    case QMI_WDS_MCAST_STATUS_IND_MSG_ID:
      {
        ind_id = QMI_WDS_SRVC_MCAST_STATUS_IND_MSG;
        if (qmi_wds_srvc_process_mcast_status_ind (rx_msg_buf,
                                                   rx_msg_len,
                                                   &ind_data) < 0)
        {
          return;
        }
      }
      break;

    case QMI_WDS_MBMS_MCAST_CONTEXT_STATUS_IND_MSG_ID:
      {
        ind_id = QMI_WDS_SRVC_MBMS_MCAST_CONTEXT_STATUS_IND_MSG;
        if (qmi_wds_srvc_process_mbms_mcast_status_ind (rx_msg_buf,
                                                        rx_msg_len,
                                                        &ind_data) < 0)
        {
          return;
        }
      }
      break;

    case QMI_WDS_DUN_CALL_INFO_IND_MSG_ID:
      {
        ind_id = QMI_WDS_SRVC_DUN_CALL_INFO_IND_MSG;
        if (qmi_wds_srvc_process_dun_call_info_ind (rx_msg_buf,
                                                    rx_msg_len,
                                                    &ind_data) < 0)
        {
          return;
        }
      }
      break;

    case QMI_WDS_EMBMS_TMGI_ACTIVATE_IND_MSG_ID:
      {
        ind_id = QMI_WDS_SRVC_EMBMS_TMGI_ACTIVATE_IND_MSG;
        if (qmi_wds_srvc_process_embms_tmgi_activate_status_ind (rx_msg_buf,
                                                                 rx_msg_len,
                                                                 &ind_data) < 0)
        {
          /* release memory */
          qmi_wds_embms_tmgi_release_mem(ind_data.embms_activate_status.activation_tmgi.tmgi_list_ptr);
          return;
        }
      }
      break;

    case QMI_WDS_EMBMS_TMGI_DEACTIVATE_IND_MSG_ID:
      {
        ind_id = QMI_WDS_SRVC_EMBMS_TMGI_DEACTIVATE_IND_MSG;
        if (qmi_wds_srvc_process_embms_tmgi_deactivate_status_ind (rx_msg_buf,
                                                                   rx_msg_len,
                                                                   &ind_data) < 0)
        {
          /* release memory */
          qmi_wds_embms_tmgi_release_mem(ind_data.embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr);
          return;
        }
      }
      break;
    case QMI_WDS_EMBMS_TMGI_ACT_DEACT_IND_MSG_ID:
      {
        ind_id = QMI_WDS_SRVC_EMBMS_TMGI_ACT_DEACT_IND_MSG;
        if(qmi_wds_srvc_process_embms_tmgi_act_deact_status_ind (rx_msg_buf,
                                                                 rx_msg_len,
                                                                 &ind_data) < 0)
        {
          /* release memory */
          qmi_wds_embms_tmgi_release_mem(ind_data.embms_act_deact_status.activation_tmgi.tmgi_list_ptr);
          qmi_wds_embms_tmgi_release_mem(ind_data.embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr);
          return;
        }
      }
      break;

    case QMI_WDS_EMBMS_TMGI_LIST_IND_MSG_ID:
      {
        ind_id = QMI_WDS_SRVC_EMBMS_TMGI_LIST_IND_MSG;
        if (qmi_wds_srvc_process_embms_tmgi_list_ind (rx_msg_buf,
                                                      rx_msg_len,
                                                      &ind_data) < 0)
        {
          /* release memory */
          qmi_wds_embms_tmgi_release_mem(ind_data.embms_list.tmgi_list.tmgi_list_ptr);
          return;
        }
      }
      break;

    case QMI_WDS_EMBMS_CONTENT_DESC_CONTROL_IND_MSG_ID:
      {
        ind_id = QMI_WDS_SRVC_EMBMS_CONTENT_DESC_CONTROL_IND_MSG;
        if (qmi_wds_srvc_process_embms_content_desc_control_ind (rx_msg_buf,
                                                                 rx_msg_len,
                                                                 &ind_data) < 0)
        {
          /* release memory */
          qmi_wds_embms_tmgi_release_mem(ind_data.embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr);
          return;
        }
      }
      break;

    case QMI_WDS_LTE_ATTACH_PDN_LIST_IND_MSG_ID:
      {
        ind_id = QMI_WDS_SRVC_LTE_ATTACH_PDN_LIST_IND_MSG;
        if (qmi_wds_srvc_process_lte_attach_pdn_list_ind (rx_msg_buf,
                                                          rx_msg_len,
                                                          &ind_data) < 0)
        {
          return;
        }
      }
      break;

    case QMI_WDS_EMBMS_SAI_LIST_IND_MSG_ID:
      {
        ind_id = QMI_WDS_SRVC_EMBMS_SAI_LIST_IND_MSG;
        if (qmi_wds_srvc_process_embms_sai_list_ind (rx_msg_buf,
                                                     rx_msg_len,
                                                     &ind_data) < 0)
        {
          /* release memory */
          if (ind_data.sai_list.available_sai_list != NULL)
          {
            free(ind_data.sai_list.available_sai_list);
            ind_data.sai_list.available_sai_list = NULL;
          }
          if (ind_data.sai_list.camped_sai_list != NULL)
          {
            free(ind_data.sai_list.camped_sai_list);
            ind_data.sai_list.camped_sai_list = NULL;
          }
          if (ind_data.sai_list.num_sai_per_group != NULL)
          {
            free(ind_data.sai_list.num_sai_per_group );
            ind_data.sai_list.num_sai_per_group = NULL;
          }
          return;
        }
      }
      break;

     case QMI_WDS_REV_IP_TRANSPORT_IND_MSG_ID:
       {
         ind_id = QMI_WDS_SRVC_REV_IP_TRANSPORT_IND_MSG;
         if (qmi_wds_srvc_process_rev_ip_trans_ind (rx_msg_buf,
                                                    rx_msg_len,
                                                    &ind_data) < 0)
         {
           return;
         }
       }
       break;

     case QMI_WDS_HANDOFF_INFORMATION_IND_MSG_ID:
      {
        ind_id = QMI_WDS_SRVC_HANDOFF_INFORMATION_IND_MSG;
        if (qmi_wds_srvc_process_handoff_info_ind (rx_msg_buf,
                                                   rx_msg_len,
                                                   &ind_data) < 0)
        {
          return;
        }
      }
      break;

     case QMI_WDS_EXT_IP_CONFIG_IND_MSG_ID:
       {
         ind_id = QMI_WDS_SRVC_EXT_IP_CONFIG_IND_MSG;
         if (qmi_wds_srvc_process_ext_ip_config_ind (rx_msg_buf,
                                                     rx_msg_len,
                                                     &ind_data) < 0)
         {
           return;
         }
       }
       break;

     case QMI_WDS_DATA_PATH_CHANGE_IND_MSG_ID:
       {
         ind_id = QMI_WDS_SRVC_DATA_PATH_CHANGE_IND_MSG;
         if (qmi_wds_srvc_process_data_path_change_ind (rx_msg_buf,
                                                        rx_msg_len,
                                                        &ind_data) < 0)
         {
           return;
         }
       }
       break;

    default:
      return;
  }

  /* Get properly cast pointer to user indication handler */
  /*lint -e{611} */
  user_ind_hdlr = (qmi_wds_indication_hdlr_type) user_ind_msg_hdlr;

  /* Call user registered handler */
  user_ind_hdlr (user_handle,
                 service_id,
                 user_ind_msg_hdlr_user_data,
                 ind_id,
                 &ind_data);

  /* release memory */
  if(ind_id == QMI_WDS_SRVC_EMBMS_TMGI_ACTIVATE_IND_MSG)
  {
    qmi_wds_embms_tmgi_release_mem(ind_data.embms_activate_status.activation_tmgi.tmgi_list_ptr);
  }
  else if(ind_id == QMI_WDS_SRVC_EMBMS_TMGI_DEACTIVATE_IND_MSG )
  {
    qmi_wds_embms_tmgi_release_mem(ind_data.embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr);
  }
  else if(ind_id == QMI_WDS_SRVC_EMBMS_TMGI_ACT_DEACT_IND_MSG)
  {
    qmi_wds_embms_tmgi_release_mem(ind_data.embms_act_deact_status.activation_tmgi.tmgi_list_ptr);
    qmi_wds_embms_tmgi_release_mem(ind_data.embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr);
  }
  else if(ind_id == QMI_WDS_EMBMS_TMGI_LIST_IND_MSG_ID)
  {
    qmi_wds_embms_tmgi_release_mem(ind_data.embms_list.tmgi_list.tmgi_list_ptr);
  }
  else if(ind_id == QMI_WDS_EMBMS_CONTENT_DESC_CONTROL_IND_MSG_ID)
  {
    qmi_wds_embms_tmgi_release_mem(ind_data.embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr);
  }
  else if (ind_id == QMI_WDS_EMBMS_SAI_LIST_IND_MSG_ID)
  {
    if (ind_data.sai_list.available_sai_list != NULL)
    {
      free(ind_data.sai_list.available_sai_list);
      ind_data.sai_list.available_sai_list = NULL;
    }
    if (ind_data.sai_list.camped_sai_list != NULL)
    {
      free(ind_data.sai_list.camped_sai_list);
      ind_data.sai_list.camped_sai_list = NULL;
    }
    if (ind_data.sai_list.num_sai_per_group != NULL)
    {
      free(ind_data.sai_list.num_sai_per_group );
      ind_data.sai_list.num_sai_per_group = NULL;
    }
  }
  else if (ind_id == QMI_WDS_SRVC_EVENT_REPORT_IND_MSG)
  {
    /* Free data system status related memory */
    if (ind_data.event_report.event_mask & QMI_WDS_EVENT_DATA_SYS_STATUS_IND)
    {
      free(ind_data.event_report.data_sys_status.network_info);
      ind_data.event_report.data_sys_status.network_info = NULL;
    }
  }
}

/*===========================================================================
  FUNCTION  qmi_wds_handle_start_nw_rsp
===========================================================================*/
/*!
@brief
  This function handles reply from start network interface command.  It
  gets the pkt_data_handle which is returned in the reply and stores it
  for use when bringing the call down.

@return
  None.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Stores pkt_data_handle in client data structure
*/
/*=========================================================================*/
static int
qmi_wds_handle_start_nw_rsp
(
  int                          user_handle,
  unsigned char                *reply_msg_data,
  int                          reply_msg_size
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  int rc = QMI_NO_ERR;
  unsigned char pkt_data_hndl_found = FALSE;

  while (reply_msg_size > 0)
  {
    /* Read the reply TLV, should be 4 byte pkt_data_handle */
    if (qmi_util_read_std_tlv (&reply_msg_data,
                                &reply_msg_size,
                                &type,
                                &length,
                                &value_ptr) < 0)
    {
      rc = QMI_INTERNAL_ERR;
    }
    /* Verify that the TLV looks right */
    if ((type == QMI_WDS_NW_IF_PKT_HANDLE_RSP_TLV_ID) &&
        (length == 4))
    {
      qmi_wds_client_data_type *client_data;
      unsigned long tmp_pkt_data_handle;

      READ_32_BIT_VAL (value_ptr,tmp_pkt_data_handle);

      /* Now we want to store the packet data handle in the client
      ** data structure
      */

      /* Lock the client data list mutex */
      QMI_PLATFORM_MUTEX_LOCK (&qmi_wds_client_data_list_mutex);

      /* Get a pointer to the client data based on user handle */
      client_data = qmi_wds_get_client_data (user_handle);

      /* Make sure we got back valid pointer */
      if (client_data == NULL)
      {
        rc = QMI_INTERNAL_ERR;
      }
      else
      {
        client_data->data_call.pkt_data_handle = tmp_pkt_data_handle;
      }

      pkt_data_hndl_found = TRUE;
      QMI_PLATFORM_MUTEX_UNLOCK (&qmi_wds_client_data_list_mutex);
    }
  }

  if (!pkt_data_hndl_found)
  {
    rc = QMI_INTERNAL_ERR;
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_srvc_async_cb
===========================================================================*/
/*!
@brief
  This is the callback function that will be called by the generic
  services layer to report asynchronous responses.  This function will
  process the asynchronous TLV's and then call the user registered
  functions with the reply data

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
static void
qmi_wds_srvc_async_cb
(
  int                     user_handle,
  qmi_service_id_type     service_id,
  unsigned long           msg_id,
  int                     rsp_rc,
  int                     qmi_err_code,
  unsigned char           *reply_msg_data,
  int                     reply_msg_size,
  void                    *srvc_async_cb_data,
  void                    *user_async_cb_fn,
  void                    *user_async_cb_data
)
{
  qmi_wds_user_async_cb_type user_cb;
  qmi_wds_async_rsp_id_type   rsp_id;
  qmi_wds_async_rsp_data_type rsp_data;

  (void) srvc_async_cb_data;

  memset((void*)&rsp_data, 0x0, sizeof(rsp_data));

  /* Now set get response data based on which type of response it was */
  switch (msg_id)
  {
    case QMI_WDS_START_NW_IF_MSG_ID:
    {
      /* set response ID, and call function to handle start network
      ** return data
      */
      rsp_id = QMI_WDS_SRVC_START_NW_ASYNC_RSP_MSG;

      rsp_data.start_nw_rsp.call_end_reason.legacy_reason  \
        = QMI_WDS_CE_REASON_UNDEFINED;
      rsp_data.start_nw_rsp.call_end_reason.call_end_reason_verbose. \
        verbose_reason = QMI_WDS_VERBOSE_CE_INVALID;
      rsp_data.start_nw_rsp.call_end_reason.call_end_reason_verbose. \
        verbose_reason_type = QMI_WDS_CE_TYPE_INVALID;

      if (rsp_rc == QMI_NO_ERR)
      {
        rsp_rc = qmi_wds_handle_start_nw_rsp (user_handle,
                                              reply_msg_data,
                                              reply_msg_size);

        QMI_DEBUG_MSG_1("qmi_wds_handle_start_nw_rsp ret=%d\n", rsp_rc);
      }
        else if (rsp_rc == QMI_SERVICE_ERR)
        {
          unsigned long type;
          unsigned long length;
          unsigned char *value_ptr;

          while (reply_msg_size > 0)
          {
            if (qmi_util_read_std_tlv (&reply_msg_data,
                                          &reply_msg_size,
                                          &type,
                                          &length,
                                          &value_ptr) < 0)
            {
              return ;
            }
            switch (type)
            {
              case QMI_WDS_PKT_SRVC_CALL_END_TLV_ID:
                {
                  unsigned short tmp_short;
                  READ_16_BIT_VAL (value_ptr,tmp_short);
                  rsp_data.start_nw_rsp.call_end_reason.legacy_reason
                                            = (qmi_wds_ce_reason_legacy_type) tmp_short;
                }
                break;

              case QMI_WDS_PKT_SRVC_CALL_END_VERBOSE_TLV_ID:
                {
                  unsigned short tmp_short;
                  READ_16_BIT_VAL (value_ptr,tmp_short);
                  rsp_data.start_nw_rsp.call_end_reason.call_end_reason_verbose.verbose_reason_type
                                                        = (qmi_wds_verbose_ce_reason_type)tmp_short;
                  READ_16_BIT_VAL (value_ptr,tmp_short);
                  rsp_data.start_nw_rsp.call_end_reason.call_end_reason_verbose.verbose_reason
                                                         = (qmi_wds_verbose_ce_reason_codes)tmp_short;
                }
                break;

              default:
                {
                  QMI_ERR_MSG_1 ("qmi_wds_start_nw_if: unknown response TLV type = %x",(unsigned int)type);
                }
            break;
            }
          }//While
        }
    }
    break;

    case QMI_WDS_STOP_NW_IF_MSG_ID:
    {
      /* Set response ID code, no TLV's to process */
      rsp_id = QMI_WDS_SRVC_STOP_NW_ASYNC_RSP_MSG;
    }
    break;

    case QMI_WDS_RESET_MSG_ID:
    {
      /* Set response ID code, no TLV's to process */
      rsp_id = QMI_WDS_SRVC_RESET_ASYNC_RSP_MSG;
    }
    break;


    case QMI_WDS_ABORT_MSG_ID:
    {
      /* Set response ID code, no TLV's to process */
      rsp_id = QMI_WDS_SRVC_ABORT_ASYNC_RSP_MSG;
    }
    break;

    default:
    QMI_ERR_MSG_2 ("Discarding unknown async reply msg, msg_id=%x, user=%x",
                        (unsigned int) msg_id, (unsigned int) user_handle);
    return;
  }

  /* If the reply indicates that the command was aborted, don't
  ** call the user callback
  */
  if ((rsp_rc == QMI_SERVICE_ERR) &&
      (qmi_err_code == QMI_SERVICE_ERR_ABORTED))
  {
    QMI_DEBUG_MSG_2 ("Discarding aborted reply, msg_id=%x, user=%x",(unsigned int) msg_id,
                                                                  (unsigned int) user_handle);
  }
  else
  {
    /*lint -e{611} */
    user_cb = (qmi_wds_user_async_cb_type) user_async_cb_fn;

    /* Call the user callback */
    user_cb (user_handle,
             service_id,
             rsp_rc,
             qmi_err_code,
             user_async_cb_data,
             rsp_id,
             &rsp_data);
  }
}


/*===========================================================================
  FUNCTION  qmi_wds_srvc_init
===========================================================================*/
/*!
@brief
  This function is a callback that will be called once during client
  initialization

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int qmi_wds_srvc_init (void)
{
  int  rc = QMI_NO_ERR;
  if (!wds_service_initialized)
  {
    QMI_PLATFORM_MUTEX_INIT (&qmi_wds_client_data_list_mutex);
    rc = qmi_service_set_srvc_functions (QMI_WDS_SERVICE,
                                         qmi_wds_srvc_indication_cb);

    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_wds_srvc_init: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_wds_srvc_init: WDS successfully initialized");
      wds_service_initialized = TRUE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_wds_srvc_init: Init failed, WDS already initialized");
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_srvc_release
===========================================================================*/
/*!
@brief
  This function is a callback that will be called once during client
  release

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int qmi_wds_srvc_release (void)
{
  int  rc = QMI_NO_ERR;
  if (wds_service_initialized)
  {
    QMI_PLATFORM_MUTEX_DESTROY (&qmi_wds_client_data_list_mutex);
    rc = qmi_service_set_srvc_functions (QMI_WDS_SERVICE,NULL);
    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_wds_srvc_release: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_wds_srvc_release: WDS successfully released");
      wds_service_initialized = FALSE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_nas_wds_release: Release failed, WDS not initialized");
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_srvc_init_client
===========================================================================*/
/*!
@brief
  This function is called to initialize the WDS service.  This function
  must be called prior to calling any other WDS service functions.
  For the time being, the indication handler callback and user data
  should be set to NULL until this is implemented.  Also note that this
  function may be called multiple times to allow for multiple, independent
  clients.

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
qmi_client_handle_type
qmi_wds_srvc_init_client
(
  const char                    *dev_id,
  qmi_wds_indication_hdlr_type  user_ind_msg_hdlr,
  void                          *user_ind_msg_hdlr_user_data,
  int                           *qmi_err_code
)
{
  qmi_client_handle_type client_handle;
  qmi_connection_id_type conn_id;
  int mux_id = -1;
  int qmi_err;
  int rc;
  qmi_wds_bind_mux_data_port_params_type bind_params;
  int ep_type = -1;
  int epid = -1;

  if (QMI_CONN_ID_INVALID ==
      (conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID_EX(dev_id, &ep_type, &epid, &mux_id)))
  {
    return QMI_INTERNAL_ERR;
  }

  /* Call common service layer initialization function */
  /*lint -e{611} */
  client_handle =  qmi_service_init (conn_id,
                                   QMI_WDS_SERVICE,
                                   (void *)user_ind_msg_hdlr,
                                   user_ind_msg_hdlr_user_data,
                                   qmi_err_code);

  /* If we succefully create a new client, put it in the
  ** WDS layer client list
  */
  if (client_handle >= 0)
  {
    qmi_wds_client_data_type *client_data;

    /* Allocate a client data structure for the new client */
    client_data = malloc (sizeof (qmi_wds_client_data_type));

    /* Verify that we were able to allocate WDS client data structure */
    if (client_data == NULL)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Zero out the client data structure */
    memset (client_data,0,sizeof (qmi_wds_client_data_type));

    client_data->client_user_handle = client_handle;

    /* Lock mutex to add client data to list */
    QMI_PLATFORM_MUTEX_LOCK (&qmi_wds_client_data_list_mutex);
    QMI_SLL_ADD (client_data, qmi_wds_client_data_list);
    QMI_PLATFORM_MUTEX_UNLOCK (&qmi_wds_client_data_list_mutex);

    /* Bind to a epid/mux-id if binding is supported */
    if (0 < mux_id)
    {
      memset(&bind_params, 0, sizeof(bind_params));
      bind_params.params_mask |= QMI_WDS_BIND_MUX_DATA_PORT_PARAMS_MUX_ID;
      bind_params.mux_id = (unsigned char)mux_id;

     if(ep_type != -1 )
     {
       /* We have a valid end point and a valid ep id */
       bind_params.params_mask |= QMI_WDS_BIND_MUX_DATA_PORT_PARAMS_EP_ID;
       bind_params.ep_id.ep_type = ep_type;
       bind_params.ep_id.iface_id = (unsigned long) epid;
     }

#ifdef FEATURE_QMI_IWLAN
      /* Check if dev_id is reverse rmnet */
      if (!strncmp(dev_id, QMI_PLATFORM_REV_RMNET_DATA_PREFIX,
               strlen(QMI_PLATFORM_REV_RMNET_DATA_PREFIX)))
      {
        /* Specify reverse field */
        bind_params.params_mask |= QMI_WDS_BIND_MUX_DATA_PORT_PARAMS_REVERSED;
        bind_params.reversed = TRUE;
      }
#endif /* FEATURE_QMI_IWLAN */

      /* Ignore bind failure, WDS client is still usable */
      (void) qmi_wds_bind_mux_data_port(client_handle, &bind_params, &qmi_err);
    }
  }
  return client_handle;
}

/*===========================================================================
  FUNCTION  qmi_wds_srvc_release_client
===========================================================================*/
/*!
@brief
  This function is called to release a client created by the
  qmi_wds_srvc_init_client() function.  This function should be called
  for any client created when terminating a client process, especially
  if the modem processor is not reset.  The modem side QMI server has
  a limited number of clients that it will allocate, and if they are not
  released, we will run out.

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
int
qmi_wds_srvc_release_client
(
  int      user_handle,
  int      *qmi_err_code
)
{
  int rc;
  rc = qmi_service_release (user_handle, qmi_err_code);

  /* If we successfully deleted the client, remove WDS client info */
  if (rc == QMI_NO_ERR)
  {
    qmi_wds_client_data_type *client_data, *prev;

    /* Lock the list mutex */
    QMI_PLATFORM_MUTEX_LOCK (&qmi_wds_client_data_list_mutex);
    QMI_SLL_FIND_AND_REMOVE (client_data,prev,qmi_wds_client_data_list,(client_data->client_user_handle == user_handle));
    QMI_PLATFORM_MUTEX_UNLOCK (&qmi_wds_client_data_list_mutex);

    /* Free the memory */
    if (client_data != NULL)
    {
      free (client_data);
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_write_optional_start_nw_if_tlvs
===========================================================================*/
/*!
@brief
  Takes the input UMTS profile data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int
qmi_wds_write_optional_start_nw_if_tlvs
(
  unsigned char                   **tx_buf,
  int                             *tx_buf_len,
  qmi_wds_start_nw_if_params_type *params
)
{
  qmi_wds_data_call_origin_type    data_call_origin;

  /* Profile index param */
  if (params->params_mask & QMI_WDS_START_NW_PROFILE_IDX_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_PROFILE_IDX_REQ_TLV_ID,
                                   1,
                                   (void *)&params->profile_index) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (params->params_mask & QMI_WDS_START_NW_PROFILE_IDX_3GPP2_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_PROFILE_IDX_3GPP2_REQ_TLV_ID,
                                   1,
                                   (void *)&params->profile_index_3gpp2) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* APN name */
  if (params->params_mask & QMI_WDS_START_NW_APN_NAME_PARAM)
  {
    size_t name_len = strlen (params->apn_name);
    if (name_len >= QMI_WDS_MAX_APN_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }

    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_APN_NAME_REQ_TLV_ID,
                                   name_len,
                                   (void *)params->apn_name) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Technology preference */
  if (params->params_mask & QMI_WDS_START_NW_TECH_PREF_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_TECH_PREF_REQ_TLV_ID,
                                   1,
                                   (void *)&params->tech_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_START_NW_XTENDED_TECH_PREF_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_XTENDED_TECH_PREF_REQ_TLV_ID,
                                   2,
                                   (void *)&params->xtended_tech_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* IP family preference */
  if (params->params_mask & QMI_WDS_START_NW_IP_FAMILY_PREF_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_IP_FAMILY_PREF_REQ_TLV_ID,
                                   1,
                                   (void *)&params->ip_family_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_START_NW_PRIM_DNS_IPV4_ADDR_PREF_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_PRIM_DNS_ADDR_PREF_REQ_TLV_ID,
                                   4,
                                   (void *)&params->primary_dns_ip4_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_START_NW_SECONDRY_DNS_IPV4_ADDR_PREF_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_SECONDARY_DNS_ADDR_PREF_REQ_TLV_ID,
                                   4,
                                   (void *)&params->secondar_dns_ip4_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_START_NW_PRIMARY_NBNS_ADDR_PREF_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_PRIMARY_NBNS_ADDR_PREF_REQ_TLV_ID,
                                   4,
                                   (void *)&params->primary_nbns_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_START_NW_SECONDARY_NBNS_ADDR_PREF_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_SECONDARY_NBNS_ADDR_PREF_REQ_TLV_ID,
                                   4,
                                   (void *)&params->secondary_nbns_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_START_NW_IPV4_ADDR_PREF_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_IPV4_ADDR_PREF_REQ_TLV_ID,
                                   4,
                                   (void *)&params->ipv4_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_START_NW_USERNAME_PARAM)
  {
    size_t name_len = strlen (params->username);
    if (name_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_USERNAME_REQ_TLV_ID,
                                   name_len,
                                   (void *)params->username) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_START_NW_PASSWORD_PARAM)
  {
    size_t name_len = strlen (params->password);
    if (name_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_PASSWORD_REQ_TLV_ID,
                                   name_len,
                                   (void *)params->password) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_START_NW_AUTH_PREF_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_AUTHENTICATION_PREF_REQ_TLV_ID,
                                   1,
                                   (void *)&params->auth_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_START_NW_IFACE_HNDL_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_NW_IF_IFACE_HNDL_TLV_ID,
                                   4,
                                   (void *)&params->iface_handle) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }


  data_call_origin = (params->params_mask & QMI_WDS_START_NW_DATA_CALL_ORIGIN_PARAM) ?
                                  params->data_call_origin : QMI_WDS_DATA_CALL_ORIGIN_EMBEDDED;

  if (qmi_util_write_std_tlv (tx_buf,
                              tx_buf_len,
                              QMI_WDS_NW_IF_DATA_CALL_TYPE_TLV_ID,
                              1,
                              (void *)&data_call_origin) < 0)
  {
    return QMI_INTERNAL_ERR;
  }


  if (params->params_mask & QMI_WDS_START_NW_HANDOFF_CONTEXT_PARAM)
  {
    unsigned char tmp_buf[QMI_WDS_HANDOFF_CONTEXT_TLV_SIZE];
    unsigned char *tmp_buf_ptr = tmp_buf;
    ipv4_addr_type  null_ipv4_addr;
    ipv6_addr_type  null_ipv6_addr;
    unsigned char tmp8;

    memset(&null_ipv4_addr, 0, sizeof(null_ipv4_addr));
    memset(&null_ipv6_addr, 0, sizeof(null_ipv6_addr));

    if (QMI_WDS_HANDOFF_BEARER_IP_TYPE_V6 == params->handoff_context.bearer_ip_type)
    {
      memcpy(tmp_buf_ptr, &null_ipv4_addr, sizeof(ipv4_addr_type));
    }
    else
    {
      memcpy(tmp_buf_ptr, &params->handoff_context.ipv4_addr, sizeof(ipv4_addr_type));
    }
    tmp_buf_ptr += QMI_WDS_IPV4_ADDR_SIZE_IN_BYTES;

    if (QMI_WDS_HANDOFF_BEARER_IP_TYPE_V4 == params->handoff_context.bearer_ip_type)
    {
      memcpy(tmp_buf_ptr, &null_ipv6_addr, sizeof(ipv6_addr_type));
    }
    else
    {
      memcpy(tmp_buf_ptr, &params->handoff_context.ipv6_addr, sizeof(ipv6_addr_type));
    }
    tmp_buf_ptr += QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES;

    tmp8 = (unsigned char) params->handoff_context.bearer_ip_type;
    WRITE_8_BIT_VAL(tmp_buf_ptr, tmp8);

    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_NW_IF_HANDOFF_CONTEXT_TLV_ID,
                                QMI_WDS_HANDOFF_CONTEXT_TLV_SIZE,
                                (void *)tmp_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_write_optional_get_runtime_settings_tlvs
===========================================================================*/
/*!
@brief
  Takes the input UMTS profile data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int
qmi_wds_write_optional_get_runtime_settings_tlvs
(
unsigned char                                **tx_buf,
int                                          *tx_buf_len,
qmi_wds_internal_runtime_setings_params_type *params
)
{
  if (params->params_mask & QMI_WDS_REQ_INTERNAL_SETTINGS_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_REQ_RUNTIME_SETTINGS_TLV_ID,
                                   4,
                                   (void *)&params->req_internal_settings) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_REQ_SESSION_TIMER_SELECT_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_REQ_SESSION_TIMER_SEL_TLV_ID,
                                   1,
                                   (void *)&params->timer_info) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_REQ_SDB_FLAGS_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_REQ_SDB_FLAGS_TLV_ID,
                                   4,
                                   (void *)&params->sdb_flags) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_write_optional_set_runtime_settings_tlvs
===========================================================================*/
/*!
@brief
  Takes the input UMTS profile data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/

static int
qmi_wds_write_optional_set_runtime_settings_tlvs
(
unsigned char                                       **tx_buf,
int                                                 *tx_buf_len,
qmi_wds_set_internal_runtime_settings_params_type   *params
)
{
  if (params->params_mask & QMI_WDS_SET_INTERNAL_RUNTIME_ENABLE_HOLDDOWN_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_SET_INTERNAL_RUNTIME_ENABLE_HOLDDOWN_REQ_TLV_ID,
                                   1,
                                   (void *)&params->holddown_enable) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_SET_INTERNAL_RUNTIME_1X_DORM_TIMER_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_DORM_TIMER_1X_TLV_ID,
                                   4,
                                   (void *)&params->dorm_timer_for_1x) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_SET_INTERNAL_RUNTIME_1X_SESSION_TIMER_PARAM)
  {
    unsigned char tmp_buf [3];
    unsigned char *tmp_buf_ptr = tmp_buf;
    WRITE_8_BIT_VAL(tmp_buf_ptr,params->session_timer_1x.timer_select);
    WRITE_16_BIT_VAL(tmp_buf_ptr,params->session_timer_1x.timer_value);

    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_SESSION_TIMER_1X_TLV_ID,
                                3,
                                (void *)tmp_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_SET_INTERNAL_RUNTIME_HDR_1X_HANDDOWN_OPT_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_HDR_1X_HANDDOWN_TLV_ID,
                                   1,
                                   (void *)&params->hdr_handdown_option) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_SET_INTERNAL_RUNTIME_HYSTERISIS_ACTIVATION_TIMER_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_HYSTERISIS_ACTIVATION_TIMER_TLV_ID,
                                   4,
                                   (void *)&params->hysteresis_activation_timer) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_SET_INTERNAL_RUNTIME_HDR_SLOTTED_MODE_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_HDR_EIDLE_SLOT_MODE_OPT_TLV_ID,
                                   4,
                                   (void *)&params->slot_cycle_value) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_SET_INTERNAL_RUNTIME_ENABLE_HDR_HPT_MODE_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_SET_INTERNAL_RUNTIME_HDR_HPT_REQ_TLV_ID,
                                   1,
                                   (void *)&params->hdr_htp_mode_enable) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_SET_INTERNAL_RUNTIME_ENABLE_HDR_HPT_MODE_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_SET_INTERNAL_RUNTIME_HDR_REV0_RATE_INERTIA_REQ_TLV_ID,
                                   1,
                                   (void *)&params->hdr_rev0_rate_inertia_enable) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  return QMI_NO_ERR;
}

/*Some utility functions*/
static int
qmi_wds_copy_n_byte_value
(
unsigned char           **dest_buf,
unsigned char           *source,
int                     n_bytes
)
{
  int i = 0;
  unsigned char *tmp_ptr = *dest_buf;
  unsigned char *val     = source;
  for (i = 0; i < n_bytes; i++)
    *tmp_ptr++ = *val++;
  *dest_buf += n_bytes;
  return n_bytes;
}
/*===========================================================================
  FUNCTION  qmi_wds_write_optional_mcast_join_tlvs
===========================================================================*/
/*!
@brief
  Takes the input UMTS profile data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/

static int
qmi_wds_write_optional_mcast_join_tlvs
(
unsigned char                                       **tx_buf,
int                                                 *tx_buf_len,
qmi_wds_mcast_ipv4_join_info_param_type             *params
)
{
  if (params->params_mask & QMI_WDS_MCAST_JOIN_IPV4_ADDR_INFO_PARAM)
  {
    unsigned char tmp_buf [6];
    unsigned char *tmp_buf_ptr = tmp_buf;
    WRITE_32_BIT_VAL(tmp_buf_ptr,params->mcast_ip_addr_info.mcast_ip_addr);
    WRITE_16_BIT_VAL(tmp_buf_ptr,params->mcast_ip_addr_info.mcast_port);

    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_MCAST_JOIN_IPV4_INFO_REQ_TLV_ID,
                                   6,
                                   (void *)tmp_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (params->params_mask & QMI_WDS_MCAST_JOIN_MBMS_SPECIFIC_JOIN_INFO_PARAM)
  {
    unsigned char tmp_buf [30];
    unsigned char *tmp_buf_ptr = tmp_buf;

    (void)qmi_wds_copy_n_byte_value(&tmp_buf_ptr,(unsigned char *)params->mbms_specific_join_info.tmgi,8);
    (void)qmi_wds_copy_n_byte_value(&tmp_buf_ptr,(unsigned char *)params->mbms_specific_join_info.session_start_timer,8);
    (void)qmi_wds_copy_n_byte_value(&tmp_buf_ptr,(unsigned char *)params->mbms_specific_join_info.session_end_timer,8);
    WRITE_16_BIT_VAL(tmp_buf_ptr,params->mbms_specific_join_info.priority);
    WRITE_8_BIT_VAL(tmp_buf_ptr,params->mbms_specific_join_info.service_method);
    WRITE_8_BIT_VAL(tmp_buf_ptr,params->mbms_specific_join_info.service_type);
    WRITE_8_BIT_VAL(tmp_buf_ptr,params->mbms_specific_join_info.selected_service);
    WRITE_8_BIT_VAL(tmp_buf_ptr,params->mbms_specific_join_info.service_security);

    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_WDS_MCAST_JOIN_MBMS_JOIN_INFO_REQ_TLV_ID,
                                   30,
                                   (void *)tmp_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_bind_mux_data_port
===========================================================================*/
/*!
@brief
  Binds a QMI-WDS client to a MUX data port. In the case of QMAP, we will
  have a single control channel, so WDS clients need to specify which
  data port their actions need to be associated with.

@param[in]  user_handle: QMI WDS client handle.
@param[in]  params: Bind MUX data port specification
@param[out] qmi_err_code: QMI error code in case of failure.

@see
  qmi_wds_bind_mux_data_port_params_type

@return
  0 if operation was successful.
  < 0 If operation failed.  qmi_err_code will contain the reason.

@dependencies
  qmi_wds_srvc_init_client() must be called before calling this.

*/
/*=========================================================================*/
int qmi_wds_bind_mux_data_port
(
  int                                     user_handle,
  qmi_wds_bind_mux_data_port_params_type *params,
  int                                    *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (NULL == qmi_err_code || NULL == params)
  {
    QMI_ERR_MSG_0("qmi_wds_bind_mux_data_port(): Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;
  QMI_DEBUG_MSG_2("qmi_wds_bind_mux_data_port(): ENTRY: user handle %d, params_mask 0x%x",
      user_handle, params->params_mask);
  QMI_DEBUG_MSG_4("qmi_wds_bind_mux_data_port(): ENTRY: ep type %d, ep_id 0x%x, mux_id %d, reversed %d",
      params->ep_id.ep_type, params->ep_id.iface_id, params->mux_id, params->reversed);

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Write EP ID Type TLV (size 8 bytes) if appropriate */
  if (params->params_mask & QMI_WDS_BIND_MUX_DATA_PORT_PARAMS_EP_ID)
  {
    unsigned char tmp_buf[8];
    unsigned char *tmp_buf_ptr = tmp_buf;

    /* Set TLV based on parameter value */
    WRITE_32_BIT_VAL(tmp_buf_ptr, (unsigned long)(params->ep_id.ep_type));
    WRITE_32_BIT_VAL(tmp_buf_ptr, params->ep_id.iface_id);
    tmp_buf_ptr = tmp_buf;

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_BIND_MUX_DATA_PORT_EP_ID_TLV_ID,
                                8,
                                (void *)tmp_buf_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Write MUX ID Type TLV (size 1 byte) if appropriate */
  if (params->params_mask & QMI_WDS_BIND_MUX_DATA_PORT_PARAMS_MUX_ID)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_BIND_MUX_DATA_PORT_MUX_ID_TLV_ID,
                                1,
                                (void *)&params->mux_id) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Write Reversed TLV (size 1 byte) if appropriate */
  if (params->params_mask & QMI_WDS_BIND_MUX_DATA_PORT_PARAMS_REVERSED)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_BIND_MUX_DATA_PORT_REVERSED_TLV_ID,
                                1,
                                (void *)&params->reversed) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_BIND_MUX_DATA_PORT_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_bind_subscription
===========================================================================*/
/*!
@brief
  Binds a QMI-WDS client to Subscription. .

@param[in]  user_handle: QMI WDS client handle.
@param[in]  params: Subscription ID
@param[out] qmi_err_code: QMI error code in case of failure.

@see
  qmi_wds_bind_subscription_type

@return
  0 if operation was successful.
  < 0 If operation failed.  qmi_err_code will contain the reason.

@dependencies
  qmi_wds_srvc_init_client() must be called before calling this.

*/
/*=========================================================================*/
EXTERN int
qmi_wds_bind_subscription
(
  int                                     user_handle,
  qmi_wds_bind_subscription_type          subs_id,
  int                                    *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (NULL == qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_bind_mux_data_port(): Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;
  QMI_DEBUG_MSG_2("qmi_wds_bind_subscription(): ENTRY: user handle %d, subs_id 0x%x",
      user_handle, subs_id);

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Write SUBS ID Type TLV (size 4 byte) if appropriate */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_BIND_SUB_ID_TLV_ID,
                              4,
                              (void *)&subs_id) < 0)
  {
    return QMI_INTERNAL_ERR;
  }


  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_BIND_SUBSCRIPTION_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_bind_subscription
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
qmi_wds_get_bind_subscription
(
  int                                          user_handle,
  qmi_wds_bind_subscription_type               *subs_id,
  int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;
  int temp;

  if (!subs_id )
  {
    return QMI_INTERNAL_ERR;
  }

  if (!qmi_err_code )
  {
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_BIND_SUBSCRIPTION_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_GET_BIND_SUB_ID_TLV_ID:
          {
            READ_32_BIT_VAL (value_ptr, temp);
            *subs_id = (qmi_wds_bind_subscription_type) temp;
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_bind_subscription: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_start_nw_if
===========================================================================*/
/*!
@brief
  Brings up a data call.  If the user_cb function pointer is set to NULL,
  then this function will be invoked synchronously, otherwise it will
  be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_wds_abort() command.

@note

  - Dependencies
    - qmi_wds_srvc_init_client() must be called before calling this.

  - Side Effects
    - Brings up a data call
*/
/*=========================================================================*/
int
qmi_wds_start_nw_if
(
  int                               user_handle,
  qmi_wds_start_nw_if_params_type   *params,
  qmi_wds_user_async_cb_type        user_cb,
  void                              *user_data,
  qmi_wds_call_end_reason_type      *call_end_reason_resp,
  int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (call_end_reason_resp)
  {
    /* Call end reason is optional, so initialize to UNDEFINED */
  call_end_reason_resp->legacy_reason = QMI_WDS_CE_REASON_UNDEFINED;
  call_end_reason_resp->call_end_reason_verbose. \
    verbose_reason = QMI_WDS_VERBOSE_CE_INVALID;
  call_end_reason_resp->call_end_reason_verbose. \
    verbose_reason_type = QMI_WDS_CE_TYPE_INVALID;
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_wds_start_nw_if: Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Configure optional TLV's if the params are present */
  if (params && (params->params_mask != 0))
  {
    if (qmi_wds_write_optional_start_nw_if_tlvs (&tmp_msg_ptr,
                                                 &msg_size,
                                                 params) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  else
  {
    qmi_wds_data_call_origin_type  data_call_origin = QMI_WDS_DATA_CALL_ORIGIN_EMBEDDED;

    /*This TLV will be ignored by the non-2H09 modem packages,
      as unknown TLV in request*/
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_NW_IF_DATA_CALL_TYPE_TLV_ID,
                                1,
                                (void *) &data_call_origin) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Do async if user_cb is non-NULL, sync otherwise */
  /* Send the message.  When this function returns, we should have reply data in
  ** the msg buffer, and the size in the msg_size variable
  */

  if (user_cb)
  {
    /* In the case of async messages, the return code is a
    ** transaction ID that can be used by the client to abort
    */
    *qmi_err_code = QMI_SERVICE_ERR_NONE;
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (user_handle,
                                     QMI_WDS_SERVICE,
                                     QMI_WDS_START_NW_IF_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                     qmi_wds_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (user_handle,
                                    QMI_WDS_SERVICE,
                                    QMI_WDS_START_NW_IF_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_WDS_STD_MSG_SIZE,
                                    QMI_WDS_SYNC_START_NW_IF_TIMEOUT,
                                    qmi_err_code);

    if (rc == QMI_NO_ERR)
    {
      rc = qmi_wds_handle_start_nw_rsp (user_handle,
                                        msg,
                                        msg_size);
    }
    else
    {
      unsigned long type;
      unsigned long length;
      unsigned char *value_ptr;

      /* Set tmp_msg_ptr to return data */
      tmp_msg_ptr = msg;

      while (msg_size > 0)
      {
        if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                      &msg_size,
                                      &type,
                                      &length,
                                      &value_ptr) < 0)
        {
          return QMI_INTERNAL_ERR;
        }
        switch (type)
        {
          case QMI_WDS_PKT_SRVC_CALL_END_TLV_ID:
            {
              int temp;
              READ_16_BIT_VAL (value_ptr,temp);
              call_end_reason_resp->legacy_reason = (qmi_wds_ce_reason_legacy_type)temp;
            }
            break;

          case QMI_WDS_PKT_SRVC_CALL_END_VERBOSE_TLV_ID:
            {
              int temp;
              READ_16_BIT_VAL (value_ptr,temp);
              call_end_reason_resp->call_end_reason_verbose.verbose_reason_type
                = (qmi_wds_verbose_ce_reason_type)temp;
              READ_16_BIT_VAL (value_ptr,temp);
              call_end_reason_resp->call_end_reason_verbose.verbose_reason
                = (qmi_wds_verbose_ce_reason_codes)temp;
            }
            break;

          default:
            {
              QMI_ERR_MSG_1 ("qmi_wds_start_nw_if: unknown response TLV type = %x",(unsigned int)type);
            }
            break;
        }
      }//While
    }
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_stop_nw_if
===========================================================================*/
/*!
@brief
  Brings down a data call.  If the user_cb function pointer is set to NULL,
  then this function will be invoked synchronously, otherwise it will
  be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_wds_abort() command.

@note

  - Dependencies
    - qmi_wds_srvc_init_client() must be called before calling this.

  - Side Effects
    - Brings up a data call
*/
/*=========================================================================*/
int
qmi_wds_stop_nw_if
(
  int                         user_handle,
  qmi_wds_user_async_cb_type  user_cb,
  void                        *user_data,
  int                         *qmi_err_code
)
{
  unsigned char msg[QMI_WDS_STD_MSG_SIZE];
  int           msg_size;
  unsigned char *tmp_msg_ptr;
  qmi_wds_client_data_type *client_data;
  unsigned long tmp_pkt_data_handle;
  int rc;


  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Lock the client data list mutex, and get the stored pkt_data_handle  */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_wds_client_data_list_mutex);

  /* Get a pointer to the client data based on user handle */
  client_data = qmi_wds_get_client_data (user_handle);


  /* Make sure we got back valid pointer */
  if (client_data == NULL)
  {
    QMI_PLATFORM_MUTEX_UNLOCK (&qmi_wds_client_data_list_mutex);
    return QMI_INTERNAL_ERR;
  }
  else
  {
    tmp_pkt_data_handle = client_data->data_call.pkt_data_handle;
  }
  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_wds_client_data_list_mutex);



  /* Construct TLV for client ID request */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_WDS_STOP_NW_IF_REQ_TLV_ID,
                                 4,
                                 (void *)&tmp_pkt_data_handle) < 0)
  {
    return QMI_INTERNAL_ERR;
  }


  /* Do async if user_cb is non-NULL, sync otherwise */
  /* Send the message.  When this function returns, we should have reply data in
  ** the msg buffer, and the size in the msg_size variable
  */

  if (user_cb)
  {


    /* In the case of async messages, the return code is a
    ** transaction ID that can be used by the client to abort
    */
    *qmi_err_code = QMI_SERVICE_ERR_NONE;
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (user_handle,
                                     QMI_WDS_SERVICE,
                                     QMI_WDS_STOP_NW_IF_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                     qmi_wds_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);

    /* If we get a non-negative value back, this will be the transaction ID
    ** which can be used to abort the transaction.  We don't want to allow
    ** aborting of abort commands, so we will change this value to 0 which
    ** will never be a valid transaction ID
    */
    rc = (rc < 0) ? rc : 0;
  }
  else
  {
    rc = qmi_service_send_msg_sync (user_handle,
                                    QMI_WDS_SERVICE,
                                    QMI_WDS_STOP_NW_IF_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_WDS_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    qmi_err_code);


  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_reset
===========================================================================*/
/*!
@brief
  Resets WDS service.  If the user_cb function pointer is set to NULL,
  then this function will be invoked synchronously, otherwise it will
  be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_wds_abort() command.

@note

  - Dependencies
    - qmi_wds_srvc_init_client() must be called before calling this.

  - Side Effects
    - Resets WDS service
*/
/*=========================================================================*/
int
qmi_wds_reset
(
  int                         user_handle,
  qmi_wds_user_async_cb_type  user_cb,
  void                        *user_data,
  int                         *qmi_err_code
)
{
  unsigned char msg[QMI_WDS_STD_MSG_SIZE];
  int           msg_size;
  int rc;


  /* Do async if user_cb is non-NULL, sync otherwise */
  /* Send the message.  When this function returns, we should have reply data in
  ** the msg buffer, and the size in the msg_size variable
  */

  if (user_cb)
  {
    /* In the case of async messages, the return code is a
    ** transaction ID that can be used by the client to abort
    */
    *qmi_err_code = QMI_SERVICE_ERR_NONE;
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (user_handle,
                                     QMI_WDS_SERVICE,
                                     QMI_WDS_RESET_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     0,
                                     qmi_wds_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (user_handle,
                                    QMI_WDS_SERVICE,
                                    QMI_WDS_RESET_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    0,
                                    msg,
                                    &msg_size,
                                    QMI_WDS_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    qmi_err_code);
  }
  return rc;
}



/*===========================================================================
  FUNCTION  qmi_wds_abort
===========================================================================*/
/*!
@brief
  Aborts an asynchronous WDS operation. If the user_cb function pointer is
  set to NULL, then this function will be invoked synchronously, otherwise
  it will be invoked asynchronously.  The txn_handle parameter is the
  return code returned from any other asynchronous WDS call.  Note that
  synchronous API calls cannot be aborted.

@return
  0 if abort operation was sucessful, < 0 if not.  In the case where the
  abort is successful, an asychronous reply to the aborted command will NOT
  be returned, otherwise, it will.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error.

@note

  - Dependencies
    - qmi_wds_srvc_init_client() must be called before calling this.

  - Side Effects
    - Resets WDS service
*/
/*=========================================================================*/
int
qmi_wds_abort
(
  int                         user_handle,
  int                         txn_handle,
  qmi_wds_user_async_cb_type  user_cb,
  void                        *user_data,
  int                         *qmi_err_code
)
{
  unsigned char msg[QMI_WDS_STD_MSG_SIZE];
  int           msg_size;
  unsigned char *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Construct TLV for client ID request */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_WDS_ABORT_REQ_TLV_ID,
                                 2,
                                 (void *)&txn_handle) < 0)
  {
    return QMI_INTERNAL_ERR;
  }


  /* Do async if user_cb is non-NULL, sync otherwise */
  /* Send the message.  When this function returns, we should have reply data in
  ** the msg buffer, and the size in the msg_size variable
  */

  if (user_cb)
  {


    /* In the case of async messages, the return code is a
    ** transaction ID that can be used by the client to abort
    */
    *qmi_err_code = QMI_SERVICE_ERR_NONE;
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (user_handle,
                                     QMI_WDS_SERVICE,
                                     QMI_WDS_ABORT_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                     qmi_wds_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);

    /* If we get a non-negative value back, this will be the transaction ID
    ** which can be used to abort the transaction.  We don't want to allow
    ** aborting of abort commands, so we will change this value to 0 which
    ** will never be a valid transaction ID
    */
    rc = (rc < 0) ? rc : 0;
  }
  else
  {
    rc = qmi_service_send_msg_sync (user_handle,
                                    QMI_WDS_SERVICE,
                                    QMI_WDS_ABORT_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_WDS_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    qmi_err_code);
  }
  return rc;
} /* qmi_wds_abort */

/*===========================================================================
  FUNCTION  qmi_wds_indication_register
===========================================================================*/
/*!
@brief
  Register/deregister for different QMI_WDS indications. At lease one
  indication must be present. It is invoked synchronously.

@return
  0 if register/deregister was sucessful, < 0 if not.

@note

  - Dependencies
    - qmi_wds_srvc_init_client() must be called before calling this.

  - Side Effects
    - Resets WDS service
*/
/*=========================================================================*/
EXTERN int
qmi_wds_indication_register
(
  int                               user_handle,
  qmi_wds_indication_reg_req_type   ind_type,
  int                               *qmi_err_code
)
{
  unsigned char msg[QMI_WDS_STD_MSG_SIZE];
  int           msg_size;
  unsigned char *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Construct TLV for client ID request */

  /* write TMGI List TLV */
  if (ind_type.param_mask & QMI_WDS_EMBMS_TMGI_INDICATION_REG_LIST_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_REPORT_EMBMS_TMGI_LIST_TLV_ID,
                                1,
                                (void *)&ind_type.tmgi_list_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Register for LTE Attach PDN list change indications */
  if (ind_type.param_mask & QMI_WDS_LTE_ATTACH_PDN_LIST_INDICATION_REG_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_REPORT_LTE_ATTACH_PDN_LIST_CHANGE_TLV_ID,
                                1,
                                (void *)&ind_type.report_lte_attach_pdn_list_change) < 0)

    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Register for EMBMS control description control indications */
  if (ind_type.param_mask & QMI_WDS_EMBMS_CONT_DESC_CTRL_INDICATION_REG_LIST_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_REPORT_EMBMS_CONTENT_DESC_CONTROL_TLV_ID,
                                1,
                                (void *)&ind_type.cont_desc_ctrl_pref) < 0)

    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* write SAI List TLV */
  if (ind_type.param_mask & QMI_WDS_EMBMS_SAI_INDICATION_REG_LIST_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_REPORT_EMBMS_SAI_LIST_TLV_ID,
                                1,
                                (void*)&ind_type.sai_list_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Write Handoff TLV */
  if (ind_type.param_mask & QMI_WDS_HANDOFF_INDICATION_REG_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_REPORT_HANDOFF_TLV_ID,
                                1,
                                (void *)&ind_type.handoff_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Register for extended IP config change indications */
  if (ind_type.param_mask & QMI_WDS_EXT_IP_CONFIG_CHANGE_IND_REG_LIST_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_REPORT_EXT_IP_CONFIG_CHANGE_TLV_ID,
                                1,
                                (void *)&ind_type.ext_ip_pref) < 0)

    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Register for data path change indications */
  if (ind_type.param_mask & QMI_WDS_DATA_PATH_CHANGE_IND_REG_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_REPORT_DATA_PATH_CHANGE_TLV_ID,
                                1,
                                (void *)&ind_type.data_path_pref) < 0)

    {
      return QMI_INTERNAL_ERR;
    }
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_INDICATION_REG_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_embms_content_desc_update
===========================================================================*/
/*!
@brief  Updates embms content desc. It is invoked synchronously.

@return
  0 if query operation was sucessful, < 0 if not.
  If return code is QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error.

@note

  - Dependencies
    - qmi_wds_srvc_init_client() must be called before calling this.

  - Side Effects
*/
/*=========================================================================*/

EXTERN int
qmi_wds_embms_content_desc_update
(
  int                                           user_handle,
  qmi_wds_embms_content_desc_update_info_type  *params,
  int                                          *qmi_err_code
)
{
  unsigned char msg[QMI_WDS_STD_MSG_SIZE];
  int           msg_size;
  unsigned char *tmp_msg_ptr;
  int rc;
  unsigned int i;

  if(NULL == params ||
     NULL == qmi_err_code)
  {
    QMI_ERR_MSG_0 ("qmi_wds_embms_content_desc_update: Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Construct TLV for client ID request */

  /* write TMGI TLV */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_EMBMS_CONT_DESC_UPDATE_REQ_TMGI_TLV_ID,
                              TMGI_TYPE_SIZE,
                              (void *)params->tmgi_list) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* write transaction id TLVs */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_EMBMS_CONT_DESC_UPDATE_REQ_TRANX_ID_TLV_ID,
                              2,
                              (void*)&params->dbg_trace_id) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Process optional TLVs */
  /* Optional TLVs are passed in as an array of param-value pairs */
  /* Use the QMI_WDS_EMBMS_CONT_DESC_UPDATE_REQ_TLV_OFFSET to calculate
     actual TLV IDs */
  if(NULL != (void *)params->content_desc.content_desc_ptr)
  {
    for(i=0; i < params->content_desc.content_desc_len; i++)
    {
      if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                  &msg_size,
                                  params->content_desc.content_desc_ptr[i].param_code
                                    + QMI_WDS_EMBMS_CONT_DESC_UPDATE_REQ_TLV_OFFSET,
                                  4,
                                  (void *)&params->content_desc.content_desc_ptr[i].param_value) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_EMBMS_CONTENT_DESC_UPDATE_MSG_ID,
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
    if (qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                       &msg_size,
                                       qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_embms_content_desc_update: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }
  return rc;
}



/*===========================================================================
  FUNCTION  qmi_wds_embms_tmgi_activate
===========================================================================*/
/*!
@brief
  Activates a TMGI for EMBMS. It is invoked synchronously.

@return
  0 if activate operation was sucessful, < 0 if not.  In the case where the
  activate operation is successful, a Status indication will
  be returned, otherwise, it will not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error.

@note

  - Dependencies
    - qmi_wds_srvc_init_client() must be called before calling this.

  - Side Effects
    - Resets WDS service
*/
/*=========================================================================*/
EXTERN int
qmi_wds_embms_tmgi_activate
(
  int                                     user_handle,
  qmi_wds_embms_actdeact_tmgi_info_type  *params,
  int                                    *qmi_err_code
)
{
  unsigned char msg[QMI_WDS_STD_MSG_SIZE];
  int           msg_size;
  unsigned char *tmp_msg_ptr;
  int rc;

  if(NULL == params ||
     NULL == qmi_err_code)
  {
    QMI_ERR_MSG_0 ("qmi_wds_embms_tmgi_activate: Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Construct TLV for client ID request */

  /* write TMGI TLV */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_EMBMS_ACTIVATE_REQ_TMGI_TLV_ID,
                              TMGI_TYPE_SIZE,
                              (void *)params->tmgi_list) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* process optional TLVs */
  if(params->param_mask & QMI_WDS_EMBMS_TMGI_ACTIVATE_TRANX_ID_PARAM)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_EMBMS_ACTIVATE_REQ_TRANX_ID_TLV_ID,
                                2,
                                (void*)&params->dbg_trace_id) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if(params->param_mask & QMI_WDS_EMBMS_TMGI_ACTIVATE_PREEMPT_PRIORITY_PARAM)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_EMBMS_ACTIVATE_REQ_PREEMPT_PRI_TLV_ID,
                                4,
                                (void *)&params->preempt_priority) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if(params->param_mask & QMI_WDS_EMBMS_TMGI_ACTIVATE_EARFCNLIST_PARAM)
  {
    unsigned char *tmp_buf;
    unsigned char fcnlist_len;

    /* validate earfcnlist_len */
    fcnlist_len = (unsigned char)(params->earfcn_list_len > QMI_WDS_EMBMS_TMGI_EARFCNLIST_SIZE ?
                                  QMI_WDS_EMBMS_TMGI_EARFCNLIST_SIZE: params->earfcn_list_len);

    if (NULL == (tmp_buf = malloc(fcnlist_len * sizeof(unsigned short) + 1)))
    {
      /* extra 1 byte to hold the length of fcnlist, which is part of fcnlist TLV */
      return QMI_INTERNAL_ERR;
    }

    /* copy the len of fcnlist note: tmp_buf has moved to next byte after the write */
    WRITE_8_BIT_VAL(tmp_buf, fcnlist_len);

    /* copy fcnlist */
    memcpy(tmp_buf, params->earfcn_list, fcnlist_len * sizeof(unsigned short));

    /* get back to location when memory is allocated */
    tmp_buf--;

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_EMBMS_ACTIVATE_REQ_EARFCNLIST_TLV_ID,
                                fcnlist_len * sizeof(unsigned short) + 1,/* earfcnlist TLV size
                                                                          earfcnlist_len +
                                                                          earfcnlist */
                                (void *)tmp_buf) < 0)
    {
      free(tmp_buf);
      return QMI_INTERNAL_ERR;
    }

    free(tmp_buf);
  }

  if(params->param_mask & QMI_WDS_EMBMS_TMGI_ACTIVATE_SAILIST_PARAM)
  {
    unsigned char *tmp_buf;
    unsigned char sailist_len;

    /* validate sailist_len */
    sailist_len = (unsigned char)((params->sai_list_len > QMI_WDS_EMBMS_SAI_PER_FREQ_MAX_SIZE)?
                                  QMI_WDS_EMBMS_SAI_PER_FREQ_MAX_SIZE: params->sai_list_len);

    if (NULL == (tmp_buf = malloc(sailist_len * sizeof(unsigned int) + 1)))
    {
      /* extra 1 byte to hold the length of sailist, which is part of sailist TLV */
      return QMI_INTERNAL_ERR;
    }

    /* copy the len of sailist note: tmp_buf has moved to next byte after the write */
    WRITE_8_BIT_VAL(tmp_buf, sailist_len);

    /* copy sailist */
    memcpy(tmp_buf, params->sai_list, sailist_len * sizeof(unsigned int));

    /* get back to location when memory is allocated */
    tmp_buf--;

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_EMBMS_ACTIVATE_REQ_SAILIST_TLV_ID,
                                sailist_len * sizeof(unsigned int) + 1,/* sailist TLV size
                                                                          sailist_len +
                                                                          sailist */
                                (void *)tmp_buf) < 0)
    {
      free(tmp_buf);
      return QMI_INTERNAL_ERR;
    }

    free(tmp_buf);
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_EMBMS_TMGI_ACTIVATE_MSG_ID,
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
    if (qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                       &msg_size,
                                       qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_embms_tmgi_activate: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_embms_tmgi_deactivate
===========================================================================*/
/*!
@brief
  Deactivates a TMGI for EMBMS. It is invoked synchronously.

@return
  0 if deactivate operation was sucessful, < 0 if not.  In the case where the
  deactivate operation is successful, a Status indication will
  be returned, otherwise, it will not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error.


@note

  - Dependencies
    - qmi_wds_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int
qmi_wds_embms_tmgi_deactivate
(
  int                                     user_handle,
  qmi_wds_embms_actdeact_tmgi_info_type  *params,
  int                                    *qmi_err_code
)
{
  unsigned char msg[QMI_WDS_STD_MSG_SIZE];
  int           msg_size;
  unsigned char *tmp_msg_ptr;
  int rc;

  if(NULL == params ||
     NULL == qmi_err_code)
  {
    QMI_ERR_MSG_0 ("qmi_wds_embms_tmgi_deactivate: Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Construct TLV for client ID request */

  /* write TMGI TLV */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_EMBMS_DEACTIVATE_REQ_TLV_ID,
                              TMGI_TYPE_SIZE,
                              (void *)params->deactivate_tmgi_list) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* process optional TLVs */
  if(params->param_mask & QMI_WDS_EMBMS_TMGI_DEACTIVATE_TRANX_ID_PARAM)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_EMBMS_DEACTIVATE_REQ_TRANX_ID_TLV_ID,
                                2,
                                (void*)&params->dbg_trace_id) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_EMBMS_TMGI_DEACTIVATE_MSG_ID,
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
    if (qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                       &msg_size,
                                       qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_embms_tmgi_deactivate: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_embms_tmgi_activate_deactivate
===========================================================================*/
/*!
@brief
  Activate a TMGI and Deactivates different TMGIs for EMBMS. It is invoked
  synchronously.

@return
  0 if activate and deactivate operation was sucessful, < 0 if not.  In the
  case where the activate and deactivate operation is successful, a Status
  indication will be returned, otherwise, it will not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error.


@note

  - Dependencies
    - qmi_wds_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int
qmi_wds_embms_tmgi_activate_deactivate
(
  int                                     user_handle,
  qmi_wds_embms_actdeact_tmgi_info_type  *params,
  int                                    *qmi_err_code
)
{
  unsigned char msg[QMI_WDS_STD_MSG_SIZE];
  int           msg_size;
  unsigned char *tmp_msg_ptr;
  int rc;

  if(NULL == params ||
     NULL == qmi_err_code)
  {
    QMI_ERR_MSG_0 ("qmi_wds_embms_tmgi_activate: Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Construct TLV for client ID request */

  /* write TMGI TLV */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_EMBMS_ACT_DEACT_REQ_ACTIVATE_TMGI_TLV_ID,
                              TMGI_TYPE_SIZE,
                              (void *)params->tmgi_list) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_EMBMS_ACT_DEACT_REQ_DEACTIVATE_TMGI_TLV_ID,
                              TMGI_TYPE_SIZE,
                              (void *)params->deactivate_tmgi_list) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* process optional TLVs */
  if(params->param_mask & QMI_WDS_EMBMS_TMGI_ACT_DEACT_TRANX_ID_PARAM)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_EMBMS_ACT_DEACT_REQ_TRANX_ID_TLV_ID,
                                2,
                                (void*)&params->dbg_trace_id) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if(params->param_mask & QMI_WDS_EMBMS_TMGI_ACT_DEACT_PREEMPT_PRIORITY_PARAM)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_EMBMS_ACT_DEACT_REQ_PREEMPT_PRI_TLV_ID,
                                4,
                                (void *)&params->preempt_priority) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if(params->param_mask & QMI_WDS_EMBMS_TMGI_ACT_DEACT_EARFCNLIST_PARAM)
  {
    unsigned char *tmp_buf;
    unsigned char fcnlist_len;

    /* validate earfcnlist_len */
    fcnlist_len = (unsigned char)(params->earfcn_list_len > QMI_WDS_EMBMS_TMGI_EARFCNLIST_SIZE ?
                                  QMI_WDS_EMBMS_TMGI_EARFCNLIST_SIZE: params->earfcn_list_len);

    if (NULL == (tmp_buf = malloc(fcnlist_len * sizeof(unsigned short) + 1)))
    {
      /* extra 1 byte to hold the length of fcnlist */
      return QMI_INTERNAL_ERR;
    }

    /* copy the len of fcnlist note: tmp_buf has moved to next byte after the write */
    WRITE_8_BIT_VAL(tmp_buf, fcnlist_len);

    /* copy fcnlist */
    memcpy(tmp_buf, params->earfcn_list, fcnlist_len * sizeof(unsigned short));

    /* get back to location when memory is allocated */
    tmp_buf--;

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_EMBMS_ACT_DEACT_REQ_EARFCNLIST_TLV_ID,
                                fcnlist_len * sizeof(unsigned short) + 1,/* earfcnlist TLV size
                                                                          earfcnlist_len +
                                                                          earfcnlist */
                                (void *)tmp_buf ) < 0)
    {
      free(tmp_buf);
      return QMI_INTERNAL_ERR;
    }

    free(tmp_buf);
  }
  if(params->param_mask & QMI_WDS_EMBMS_TMGI_ACT_DEACT_SAILIST_PARAM)
  {
    unsigned char *tmp_buf;
    unsigned char sailist_len;

    /* validate sailist_len */
    sailist_len = (unsigned char)(params->sai_list_len > QMI_WDS_EMBMS_SAI_PER_FREQ_MAX_SIZE ?
                                 QMI_WDS_EMBMS_SAI_PER_FREQ_MAX_SIZE: params->sai_list_len);

    if (NULL == (tmp_buf = malloc(sailist_len * sizeof(unsigned int) + 1)))
    {
      /* extra 1 byte to hold the length of sailist, which is part of sailist TLV */
      return QMI_INTERNAL_ERR;
    }

    /* copy the len of sailist note: tmp_buf has moved to next byte after the write */
    WRITE_8_BIT_VAL(tmp_buf, sailist_len);

    /* copy sailist */
    memcpy(tmp_buf, params->sai_list, sailist_len * sizeof(unsigned int));

    /* get back to location when memory is allocated */
    tmp_buf--;

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_EMBMS_ACT_DEACT_REQ_SAILIST_TLV_ID,
                                sailist_len * sizeof(unsigned int) + 1,/* sailist TLV size
                                                                          sailist_len +
                                                                          sailist */
                                (void *)tmp_buf) < 0)
    {
      free(tmp_buf);
      return QMI_INTERNAL_ERR;
    }

    free(tmp_buf);
  }
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_EMBMS_TMGI_ACT_DEACT_MSG_ID,
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
    if (qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                       &msg_size,
                                       qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_embms_tmgi_activate: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_embms_tmgi_list_query
===========================================================================*/
/*!
@brief
  Queries active or available TMGI list. It is invoked synchronously.

@return
  0 if query operation was sucessful, < 0 if not.  In the case where the
  query operation is successful, a Status indication will
  be returned, otherwise, it will not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error.


@note

  - Dependencies
    - qmi_wds_srvc_init_client() must be called before calling this.

  - Side Effects
    - Resets WDS service
*/
/*=========================================================================*/
EXTERN int
qmi_wds_embms_tmgi_list_query
(
  int                               user_handle,
  qmi_wds_embms_tmgi_list_type      list_type,
  qmi_wds_embms_tmgi_list_ind_type  *list_query,
  int                               *qmi_err_code
)
{
  unsigned char msg[QMI_WDS_STD_MSG_SIZE];
  int           msg_size;
  unsigned char *tmp_msg_ptr;
  int rc;

  if(NULL == list_query ||
     NULL == qmi_err_code)
  {
    QMI_ERR_MSG_0 ("qmi_wds_embms_tmgi_list_query: Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Construct TLV for client ID request */

  /* write TMGI TLV */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_EMBMS_LIST_QUERY_REQ_LIST_TYPE_TLV_ID,
                              1,
                              &list_type) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  if(list_query->param_mask & QMI_WDS_EMBMS_LIST_QUERY_REQ_TRANX_ID_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_EMBMS_LIST_QUERY_REQ_TRANX_ID_TLV_ID,
                                2,
                                (void*)&list_query->dbg_trace_id) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_EMBMS_TMGI_LIST_QUERY_MSG_ID,
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
    /* Assume worst case */
    rc = QMI_INTERNAL_ERR;

    list_query->param_mask = 0;

    while (msg_size > 0)
    {
      unsigned long     type;
      unsigned long     length;
      unsigned char     *value_ptr;

      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        rc = QMI_INTERNAL_ERR;
        break;
      }

      if (type == QMI_WDS_EMBMS_LIST_QUERY_RESP_TMGI_LIST_TLV_ID)
      {
        unsigned char temp_len;
        unsigned char temp;
        int           i;

        list_query->param_mask |= QMI_WDS_EMBMS_LIST_QUERY_RESP_TMGI_LIST_PARAM_MASK;

        READ_8_BIT_VAL (value_ptr, temp);
        list_query->list_type = (qmi_wds_embms_tmgi_list_type)temp;

        READ_8_BIT_VAL (value_ptr, temp_len);
        list_query->tmgi_list.tmgi_list_len = temp_len;

        if(list_query->tmgi_list.tmgi_list_len == 0 )
        {
          list_query->tmgi_list.tmgi_list_ptr = NULL;
        }
        else
        {
          /* allocate memory */
          list_query->tmgi_list.tmgi_list_ptr = (qmi_wds_embms_tmgi_type *)
                                       malloc(temp_len * sizeof(qmi_wds_embms_tmgi_type));

          if (NULL != list_query->tmgi_list.tmgi_list_ptr)
          {
            /* zero out memeory */
            memset(list_query->tmgi_list.tmgi_list_ptr,
                   0,
                   temp_len * sizeof(qmi_wds_embms_tmgi_type));

            for(i = 0; i < temp_len; i++)
            {
              memcpy(list_query->tmgi_list.tmgi_list_ptr[i].tmgi,
                     value_ptr,
                     QMI_WDS_EMBMS_TMGI_SIZE );
              value_ptr += QMI_WDS_EMBMS_TMGI_SIZE;

              READ_8_BIT_VAL (value_ptr, temp);
              list_query->tmgi_list.tmgi_list_ptr[i].session_id_valid = temp;

              READ_8_BIT_VAL (value_ptr, temp);
              list_query->tmgi_list.tmgi_list_ptr[i].session_id = temp;
            }
          }
          else
          {
            QMI_DEBUG_MSG_0 ("qmi_wds_embms_tmgi_list_query, "
                             "could not allocate memory\n");
            break;
          }
        }
        rc = QMI_NO_ERR;
      }/* if (type == QMI_WDS_EMBMS_LIST_IND_TMGI_LIST_TLV_ID */
      else
      {
        QMI_DEBUG_MSG_1 ("qmi_wds_embms_tmgi_list_query, "
                         "unknown TLV returned = %x\n",(int)type);
        break;
      }
    }/* while */
  }/* if */

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_write_umts_qos_tlv
===========================================================================*/
/*!
@brief
  Takes the input UMTS profile UMTS QOS data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int
qmi_wds_write_umts_qos_tlv
(
  unsigned char                 **tx_buf,
  int                           *tx_buf_len,
  unsigned long                 tlv_id,
  qmi_wds_umts_qos_params_type   *params,
  unsigned char                  sig_ind_is_valid // to indicate validity of signal indication field
                                                  // 1 - sig indication valid , 0 - sig indication not valid
)
{
  unsigned char param_buf [QMI_WDS_UMTS_QOS_TLV_SIZE + 1];
  unsigned char *param_ptr = param_buf;
  unsigned long tlv_len = 0;
  /* Traffic class */
  WRITE_8_BIT_VAL (param_ptr, params->traffic_class);

  /* UL/DL max/guaranteed bit rates */
  WRITE_32_BIT_VAL (param_ptr, params->max_ul_bitrate);
  WRITE_32_BIT_VAL (param_ptr, params->max_dl_bitrate);
  WRITE_32_BIT_VAL (param_ptr, params->guaranteed_ul_bitrate);
  WRITE_32_BIT_VAL (param_ptr, params->guaranteed_dl_bitrate);

  /* Delivery order */
  WRITE_8_BIT_VAL (param_ptr, params->qos_delivery_order);

  /* Max SDU size */
  WRITE_32_BIT_VAL (param_ptr, params->max_sdu_size);

  /* SDU error ration */
  WRITE_8_BIT_VAL (param_ptr, params->sdu_error_ratio);

  /* Residual BER */
  WRITE_8_BIT_VAL (param_ptr, params->residual_ber_ratio);

  /* Delivery SDU */
  WRITE_8_BIT_VAL (param_ptr, params->deliver_error_sdu);

  /* Transfer delay */
  WRITE_32_BIT_VAL (param_ptr, params->transfer_delay);

  /* Traffic handling priority */
  WRITE_32_BIT_VAL (param_ptr, params->traffic_handling_prio);

  /*lint --e{774} */
  if (!sig_ind_is_valid)
  {
    tlv_len = QMI_WDS_UMTS_QOS_TLV_SIZE;
  }
  else if (sig_ind_is_valid)
  {
    WRITE_8_BIT_VAL (param_ptr, params->signal_indication);
    tlv_len = QMI_WDS_UMTS_QOS_TLV_SIZE + 1;
  }

  if (qmi_util_write_std_tlv (tx_buf,
                                 tx_buf_len,
                                 tlv_id,
                                 tlv_len,
                                 (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;
} /* qmi_wds_write_umts_qos_tlv */


/*===========================================================================
  FUNCTION  qmi_wds_read_umts_qos_tlv
===========================================================================*/
/*!
@brief
  Takes and input RX buffer containing UMTS profile UMTS QOS TLV data
  (minus the TLV header), and reads values into a QOS paramaters
  structure.

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
qmi_wds_read_umts_qos_tlv
(
  unsigned char                 *rx_buf,
  qmi_wds_umts_qos_params_type  *params,
  unsigned char                  sig_ind_is_valid // 1 indicates signal indication present.
)
{
  unsigned char tmp_char;
   /* Traffic class */
  READ_8_BIT_VAL (rx_buf, tmp_char);
  params->traffic_class = (qmi_wds_traffic_class_type) tmp_char;

  /* UL/DL max/guaranteed bit rates */
  READ_32_BIT_VAL (rx_buf, params->max_ul_bitrate);
  READ_32_BIT_VAL (rx_buf, params->max_dl_bitrate);
  READ_32_BIT_VAL (rx_buf, params->guaranteed_ul_bitrate);
  READ_32_BIT_VAL (rx_buf, params->guaranteed_dl_bitrate);

  /* Delivery order */
  READ_8_BIT_VAL (rx_buf, tmp_char);
  params->qos_delivery_order = (qmi_wds_qos_delivery_order_type) tmp_char;

  /* Max SDU size */
  READ_32_BIT_VAL (rx_buf, params->max_sdu_size);

  /* SDU error ration */
  READ_8_BIT_VAL (rx_buf, tmp_char);
  params->sdu_error_ratio = (qmi_wds_sdu_err_ratio_type) tmp_char;

  /* Residual BER */
  READ_8_BIT_VAL (rx_buf, tmp_char);
  params->residual_ber_ratio = (qmi_wds_residual_ber_ratio_type) tmp_char;

  /* Delivery SDU */
  READ_8_BIT_VAL (rx_buf, tmp_char);
  params->deliver_error_sdu = (qmi_wds_deliver_err_sdu_type) tmp_char;

  /* Transfer delay */
  READ_32_BIT_VAL (rx_buf, params->transfer_delay);

  /* Traffic handling priority */
  READ_32_BIT_VAL (rx_buf, params->traffic_handling_prio);
  if (sig_ind_is_valid)
  {
    READ_8_BIT_VAL (rx_buf, tmp_char);
    params->signal_indication = (qmi_wds_bool_type)tmp_char;
  }
  return QMI_NO_ERR;
} /* qmi_wds_read_umts_qos_tlv */

/*===========================================================================
  FUNCTION  qmi_wds_read_lte_qos_tlv
===========================================================================*/
/*!
@brief
  Takes and input RX buffer containing UMTS profile LTE QOS TLV data
  (minus the TLV header), and reads values into a QOS paramaters
  structure.

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
qmi_wds_read_lte_qos_tlv
(
  unsigned char                 *rx_buf,
  qmi_wds_lte_qos_params_type   *params
)
{
  unsigned char tmp_char;
   /* QCI */
  READ_8_BIT_VAL (rx_buf, tmp_char);
  params->qci = (qmi_wds_traffic_class_type) tmp_char;

  /* UL/DL max/guaranteed bit rates */
  READ_32_BIT_VAL (rx_buf, params->g_dl_bit_rate);
  READ_32_BIT_VAL (rx_buf, params->max_dl_bit_rate);
  READ_32_BIT_VAL (rx_buf, params->g_ul_bit_rate);
  READ_32_BIT_VAL (rx_buf, params->max_ul_bit_rate);

  return QMI_NO_ERR;
} /* qmi_wds_read_umts_qos_tlv */


/*===========================================================================
  FUNCTION  qmi_wds_read_mnc_tlv
===========================================================================*/
/*!
@brief
  Takes and input RX buffer containing UMTS or CDMA profile MNC TLV data
  (minus the TLV header), and reads values into a MNC paramaters
  structure.

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
qmi_wds_read_mnc_tlv
(
  unsigned char                 *rx_buf,
  qmi_wds_mnc_type              *params
)
{
  unsigned char tmp_char;
   /* MNC */
  READ_16_BIT_VAL (rx_buf, params->mnc);

  /* mnc_includes_pcs_digit */
   READ_8_BIT_VAL (rx_buf, tmp_char);
   params->mnc_includes_pcs_digit = (qmi_wds_traffic_class_type) tmp_char;

  return QMI_NO_ERR;
} /* qmi_wds_read_mnc_tlv */


/*===========================================================================
  FUNCTION  qmi_wds_write_gprs_qos_tlv
===========================================================================*/
/*!
@brief
  Takes the input UMTS profile GPRS QOS data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int
qmi_wds_write_gprs_qos_tlv
(
  unsigned char                 **tx_buf,
  int                           *tx_buf_len,
  unsigned long                 tlv_id,
  qmi_wds_gprs_qos_params_type  *params
)
{
  unsigned char param_buf [QMI_WDS_GPRS_QOS_TLV_SIZE];
  unsigned char *param_ptr = param_buf;


  /* precedence class*/
  WRITE_32_BIT_VAL (param_ptr, params->precedence_class);

  /* delay class*/
  WRITE_32_BIT_VAL (param_ptr, params->delay_class);

  /* reliability class*/
  WRITE_32_BIT_VAL (param_ptr, params->reliability_class);

  /* peak throughput class*/
  WRITE_32_BIT_VAL (param_ptr, params->peak_throughput_class);

  /* mean throughput class */
  WRITE_32_BIT_VAL (param_ptr, params->mean_throughput_class);

  if (qmi_util_write_std_tlv (tx_buf,
                                 tx_buf_len,
                                 tlv_id,
                                 QMI_WDS_GPRS_QOS_TLV_SIZE,
                                 (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;
} /* qmi_wds_write_gprs_qos_tlv */


/*===========================================================================
  FUNCTION  qmi_wds_write_lte_qos_tlv
===========================================================================*/
/*!
@brief
  Takes the input UMTS profile LTE QOS data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int
  qmi_wds_write_lte_qos_tlv

(
  unsigned char                 **tx_buf,
  int                           *tx_buf_len,
  unsigned long                 tlv_id,
  qmi_wds_lte_qos_params_type   *params
)
{
  unsigned char param_buf [QMI_WDS_LTE_QOS_TLV_SIZE];
  unsigned char *param_ptr = param_buf;


  /* qci */
  WRITE_8_BIT_VAL (param_ptr, params->qci);

  /* Guaranteed DL bit rate*/
  WRITE_32_BIT_VAL (param_ptr, params->g_dl_bit_rate);

  /* Maximum DL bit rate */
  WRITE_32_BIT_VAL (param_ptr, params->max_dl_bit_rate);

  /* Guaranteed UL bit rate */
  WRITE_32_BIT_VAL (param_ptr, params->g_ul_bit_rate);

  /* Maximum UL bit rate */
  WRITE_32_BIT_VAL (param_ptr, params->max_ul_bit_rate);

  if (qmi_util_write_std_tlv (tx_buf,
                              tx_buf_len,
                              tlv_id,
                              QMI_WDS_LTE_QOS_TLV_SIZE,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;
} /* qmi_wds_write_lte_qos_tlv */


/*===========================================================================
  FUNCTION  qmi_util_write_mnc_tlv
===========================================================================*/
/*!
@brief
  Takes the input MNC data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int
  qmi_util_write_mnc_tlv

(
  unsigned char                 **tx_buf,
  int                           *tx_buf_len,
  unsigned long                 tlv_id,
  qmi_wds_mnc_type              *params
)
{
  unsigned char param_buf [QMI_WDS_MNC_TLV_SIZE];
  unsigned char *param_ptr = param_buf;


  /* mnc */
  WRITE_16_BIT_VAL (param_ptr, params->mnc);

  /* mnc_includes_pcs_digit */
  WRITE_8_BIT_VAL (param_ptr, params->mnc_includes_pcs_digit);

  if (qmi_util_write_std_tlv (tx_buf,
                              tx_buf_len,
                              tlv_id,
                              QMI_WDS_MNC_TLV_SIZE,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;
} /* qmi_util_write_mnc_tlv */


/*===========================================================================
  FUNCTION  qmi_wds_read_gprs_qos_tlv
===========================================================================*/
/*!
@brief
  Takes and input RX buffer containing UMTS profile GPRS QOS TLV data
  (minus the TLV header), and reads values into a QOS paramaters
  structure.

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
qmi_wds_read_gprs_qos_tlv
(
  unsigned char                 *rx_buf,
  qmi_wds_gprs_qos_params_type  *params
)
{
  /* precedence class*/
  READ_32_BIT_VAL (rx_buf, params->precedence_class);

  /* delay class*/
  READ_32_BIT_VAL (rx_buf, params->delay_class);

  /* reliability class*/
  READ_32_BIT_VAL (rx_buf, params->reliability_class);

  /* peak throughput class*/
  READ_32_BIT_VAL (rx_buf, params->peak_throughput_class);

  /* mean throughput class */
  READ_32_BIT_VAL (rx_buf, params->mean_throughput_class);

  return QMI_NO_ERR;
} /* qmi_wds_read_gprs_qos_tlv */


/*===========================================================================
  FUNCTION  qmi_wds_write_profile_id_tlv
===========================================================================*/
/*!
@brief
  Takes the input UMTS profile ID data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
int
qmi_wds_write_profile_id_tlv
(
  unsigned char           **tx_buf,
  int                     *tx_buf_len,
  qmi_wds_profile_id_type *profile_id
)
{
  unsigned char buf[2];
  buf[0] = (unsigned char) profile_id->technology;
  buf[1] = (unsigned char) profile_id->profile_index;
  if (qmi_util_write_std_tlv (tx_buf,
                                 tx_buf_len,
                                 QMI_WDS_PROFILE_ID_TLV_ID,
                                 2,
                                 (void *)buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  return QMI_NO_ERR;
} /* qmi_wds_write_profile_id_tlv */


/*===========================================================================
  FUNCTION  qmi_wds_read_profile_id_tlv
===========================================================================*/
/*!
@brief
  Takes an input RX buffer containing UMTS profile ID TLV data
  and reads values into a profile ID paramaters structure.  TLV header
  data may or may not be read from buffer depending on 'read_tlv' boolean
  value


@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Reads data into params structure
*/
/*=========================================================================*/
int
qmi_wds_read_profile_id_tlv
(
  unsigned char           *rx_buf,
  unsigned long            rx_buf_len,
  qmi_wds_profile_id_type *profile_id,
  unsigned long           read_tlv
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  unsigned char tmp_char;
  int rx_len = (int)rx_buf_len;

  if (read_tlv)
  {
    while (rx_len > 0)
    {
      if (qmi_util_read_std_tlv (&rx_buf,
                                 &rx_len,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
      if (type == QMI_WDS_PROFILE_ID_TLV_ID)
      {
        READ_8_BIT_VAL (value_ptr, tmp_char);
        profile_id->technology = (qmi_wds_profile_tech_type) tmp_char;
        READ_8_BIT_VAL (value_ptr, profile_id->profile_index);
        break;
      }
    }
  }
  return QMI_NO_ERR;
}



/*===========================================================================
  FUNCTION  qmi_wds_write_tft_filter
===========================================================================*/
/*!
@brief
  Takes the input tft filter, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/

static int
qmi_wds_write_tft_filter
(
  unsigned char                 **tx_buf,
  int                           *tx_buf_len,
  unsigned long                 tlv_id,
  qmi_wds_tft_params_type       *params
)
{
  unsigned char param_buf [QMI_WDS_TFT_FILTER_TLV_SIZE + 1];
  unsigned char *param_ptr = param_buf;
  unsigned long tlv_len = 0;
  unsigned long padding = 0;
  /* Filter Id */
  WRITE_8_BIT_VAL (param_ptr, params->filter_id);


  WRITE_8_BIT_VAL (param_ptr, params->eval_id);
  WRITE_8_BIT_VAL (param_ptr, params->ip_family);
  if (params->ip_family == (int)QMI_WDS_IPV4_TYPE)
  {
    WRITE_32_BIT_VAL (param_ptr, params->source_ip);
    WRITE_32_BIT_VAL (param_ptr, padding);
    WRITE_32_BIT_VAL (param_ptr, padding);
    WRITE_32_BIT_VAL (param_ptr, padding);

  }
  if (params->ip_family == (int)QMI_WDS_IPV6_TYPE)
  {
    unsigned short i;
    for (i = 0; i < sizeof(ipv6_addr_type); i++)
    {
      *param_ptr++ = params->source_ip[i];
    }
  }

  WRITE_8_BIT_VAL (param_ptr, params->source_ip_mask);
  WRITE_8_BIT_VAL (param_ptr, params->next_header);

  WRITE_16_BIT_VAL (param_ptr, params->destination_port_range_start);

  WRITE_16_BIT_VAL (param_ptr, params->destination_port_range_end);

  WRITE_16_BIT_VAL (param_ptr, params->source_port_range_start);
  WRITE_16_BIT_VAL (param_ptr, params->source_port_range_end);

  WRITE_32_BIT_VAL (param_ptr, params->ipsec_param_index);

  WRITE_16_BIT_VAL (param_ptr, params->tos_mask);

  WRITE_32_BIT_VAL (param_ptr, params->flow_lable);

  tlv_len = QMI_WDS_TFT_FILTER_TLV_SIZE;

  if (qmi_util_write_std_tlv (tx_buf,
                              tx_buf_len,
                              tlv_id,
                              tlv_len,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;
} /* qmi_wds_write_tft_filter */

/*===========================================================================
  FUNCTION  qmi_wds_read_tft_filter
===========================================================================*/
/*!
@brief
  Takes and input RX buffer containing tft filter TLV data
  (minus the TLV header), and reads values into a tft params C data structure.

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
qmi_wds_read_tft_filter
(
  unsigned char                 *rx_buf,
  qmi_wds_tft_params_type       *params
)
{
  READ_8_BIT_VAL (rx_buf, params->eval_id);
  READ_8_BIT_VAL (rx_buf, params->ip_family);

  if (params->ip_family == (int)QMI_WDS_IPV4_TYPE)
  {
    READ_32_BIT_VAL (rx_buf, params->source_ip[0]);
  }
  if (params->ip_family == (int)QMI_WDS_IPV6_TYPE)
  {
    unsigned char i;
    for (i = 0; i < sizeof(ipv6_addr_type); i++)
    {
      params->source_ip[i] = *rx_buf++;
    }
  }

  READ_8_BIT_VAL (rx_buf, params->source_ip_mask);
  READ_8_BIT_VAL (rx_buf, params->next_header);
  READ_16_BIT_VAL (rx_buf, params->destination_port_range_start);
  READ_16_BIT_VAL (rx_buf, params->destination_port_range_end);
  READ_16_BIT_VAL (rx_buf, params->source_port_range_start);
  READ_16_BIT_VAL (rx_buf, params->source_port_range_end);
  READ_32_BIT_VAL (rx_buf, params->ipsec_param_index);
  READ_16_BIT_VAL (rx_buf, params->tos_mask);
  READ_32_BIT_VAL (rx_buf, params->flow_lable);

  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_write_mobile_ip_optional_profile_tlvs
===========================================================================*/
/*!
@brief
  Takes the input UMTS profile data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
=========================================================================*/
static int
qmi_wds_write_mobile_ip_optional_profile_tlvs
(
  unsigned char                                 **tx_buf,
  int                                           *tx_buf_len,
  qmi_wds_modify_mobile_ip_profile_req_type     *profile_params
)
{

  if (profile_params->param_mask &
      QMI_WDS_MODIFY_MIP_PROFILE_PROFILE_STATE_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_MODIFY_MOBILE_IP_PROFILE_STATE_TLV_ID,
                                1,
                                (void *)&profile_params->profile_state) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask &
        QMI_WDS_MODIFY_MIP_PROFILE_HOME_ADDR_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_MODIFY_MOBILE_IP_PROFILE_HOME_ADDR_TLV_ID,
                                sizeof(ipv4_addr_type),
                                (void *)&profile_params->home_address) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask &
      QMI_WDS_MODIFY_MIP_PROFILE_HOME_AGENT_PRIMARY_ADDR_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_MODIFY_MOBILE_IP_PROFILE_PRIMARY_HOME_AGENT_ADDR_TLV_ID,
                                sizeof(ipv4_addr_type),
                                (void *)&profile_params->home_agent_primary_address) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }


  if (profile_params->param_mask &
      QMI_WDS_MODIFY_MIP_PROFILE_HOME_AGENT_SECONDARY_ADDR_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_MODIFY_MOBILE_IP_PROFILE_SECONDARY_HOME_AGENT_ADDR_TLV_ID,
                                sizeof(ipv4_addr_type),
                                (void *)&profile_params->home_agent_secondary_address) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }


  if (profile_params->param_mask &
      QMI_WDS_MODIFY_MIP_PROFILE_REVERSE_TUNNEL_PREF_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_MODIFY_MOBILE_IP_PROFILE_REVERSE_TUNNEL_PREF_TLV_ID,
                                1,
                                (void *)&profile_params->reverse_tunnel_preference) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask &
      QMI_WDS_MODIFY_MIP_PROFILE_NAI_PARAM_MASK)
  {
    if (profile_params->nai_len > QMI_WDS_MOBILE_IP_PROFILE_MAX_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_MODIFY_MOBILE_IP_PROFILE_NAI_TLV_ID,
                                profile_params->nai_len,
                                (void *)&profile_params->nai) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask &
      QMI_WDS_MODIFY_MIP_PROFILE_HA_SPI_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_MODIFY_MOBILE_IP_PROFILE_HA_SPI_TLV_ID,
                                4,
                                (void *)&profile_params->ha_spi) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask &
      QMI_WDS_MODIFY_MIP_PROFILE_AAA_SPI_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_MODIFY_MOBILE_IP_PROFILE_AAA_SPI_TLV_ID,
                                4,
                                (void *)&profile_params->aaa_spi) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask &
      QMI_WDS_MODIFY_MIP_PROFILE_MN_HA_KEY_PARAM_MASK)
  {
    size_t name_len = strlen ((char *)profile_params->mn_ha_key);
    if (name_len >= QMI_WDS_MOBILE_IP_PROFILE_MAX_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_MODIFY_MOBILE_IP_PROFILE_MN_HA_KEY_TLV_ID,
                                name_len,
                                (void *)&profile_params->mn_ha_key) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask &
      QMI_WDS_MODIFY_MIP_PROFILE_MN_AAA_KEY_PARAM_MASK)
  {
    size_t name_len = strlen ((char *)profile_params->mn_aaa_key);
    if (name_len >= QMI_WDS_MOBILE_IP_PROFILE_MAX_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_MODIFY_MOBILE_IP_PROFILE_MN_AAA_KEY_TLV_ID,
                                name_len,
                                (void *)&profile_params->mn_aaa_key) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_write_umts_optional_profile_tlvs
===========================================================================*/
/*!
@brief
  Takes the input UMTS profile data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int
qmi_wds_write_umts_optional_profile_tlvs
(
  unsigned char               **tx_buf,
  int                         *tx_buf_len,
  qmi_wds_umts_profile_params_type    *profile_params
)
{
  /* Profile name */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_NAME_PARAM_MASK)
  {
    size_t name_len = strlen (profile_params->profile_name);
    if (name_len >= QMI_WDS_MAX_PROFILE_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_NAME_TLV_ID,
                                   name_len,
                                   (void *)profile_params->profile_name) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* PDP type */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_PDP_TYPE_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_PDP_TYPE_TLV_ID,
                                   1,
                                   (void *)&profile_params->pdp_type) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* APN name */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_APN_NAME_PARAM_MASK)
  {
    size_t name_len = strlen (profile_params->apn_name);
    if (name_len >= QMI_WDS_MAX_APN_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_APN_NAME_TLV_ID,
                                   name_len,
                                   (void *)profile_params->apn_name) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Primary DNS address preference */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_PRIM_DNS_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_PRIM_DNS_TLV_ID,
                                   4,
                                   (void *)&profile_params->primary_dns_pref_addr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Secondary DNS address preference */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_SEC_DNS_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_SEC_DNS_TLV_ID,
                                   4,
                                   (void *)&profile_params->secondary_dns_pref_addr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* UMTS requested QOS parameters */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_UMTS_REQ_QOS_PARAM_MASK)
  {
    if (qmi_wds_write_umts_qos_tlv (tx_buf,
                                    tx_buf_len,
                                    QMI_WDS_UMTS_PROFILE_UMTS_REQ_QOS_TLV_ID,
                                    &profile_params->umts_requested_qos,
                                    FALSE) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* UMTS minimum QOS parameters */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_UMTS_MIN_QOS_PARAM_MASK)
  {
    if (qmi_wds_write_umts_qos_tlv (tx_buf,
                                    tx_buf_len,
                                    QMI_WDS_UMTS_PROFILE_UMTS_MIN_QOS_TLV_ID,
                                    &profile_params->umts_minimum_qos,
                                    FALSE) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* GPRS requested QOS parameters */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_GPRS_REQ_QOS_PARAM_MASK)
  {
    if (qmi_wds_write_gprs_qos_tlv (tx_buf,
                                    tx_buf_len,
                                    QMI_WDS_UMTS_PROFILE_GPRS_REQ_QOS_TLV_ID,
                                    &profile_params->gprs_requested_qos) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* UMTS minimum QOS parameters */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_GPRS_MIN_QOS_PARAM_MASK)
  {
    if (qmi_wds_write_gprs_qos_tlv (tx_buf,
                                    tx_buf_len,
                                    QMI_WDS_UMTS_PROFILE_GPRS_MIN_QOS_TLV_ID,
                                    &profile_params->gprs_minimum_qos) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Username */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_USERNAME_PARAM_MASK)
  {
    size_t name_len = strlen (profile_params->username);
    if (name_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_USERNAME_TLV_ID,
                                   name_len,
                                   (void *)profile_params->username) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Password */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_PASSWORD_PARAM_MASK)
  {
    size_t name_len = strlen (profile_params->password);
    if (name_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_PASSWORD_TLV_ID,
                                   name_len,
                                   (void *)profile_params->password) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Authentication preference */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_AUTH_PREF_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_AUTH_PREF_TLV_ID,
                                   1,
                                   (void *)&profile_params->auth_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* IPV4 address preference */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_IPV4_ADDR_PREF_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_IPV4_ADDR_PREF_TLV_ID,
                                   4,
                                   (void *)&profile_params->ipv4_pref_addr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* PCSCF address using PCO */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_PCSCF_ADDR_VIA_PCO_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_PCSCF_ADDR_VIA_PCO_TLV_ID,
                                   1,
                                   (void *)&profile_params->pcscf_addr_via_pco) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Header Compression */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_HEADER_COMPRESSION_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_HEADER_COMPRESSION_TLV_ID,
                                1,
                                (void *)&profile_params->header_compression) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* Data Compression */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_DATA_COMPRESSION_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_DATA_COMPRESSION_TLV_ID ,
                                1,
                                (void *)&profile_params->data_compression) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Pdp accesss comtrol */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_PDP_ACCESS_CONTROL_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_PDP_ACCESS_CONTROL_TLV_ID ,
                                1,
                                (void *)&profile_params->pdp_access_control) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Pcscf via dhcp */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_PCSCF_VIA_DHCP_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_PCSCF_VIA_DHCP_TLV_ID ,
                                1,
                                (void *)&profile_params->pcscf_via_dhcp) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Im cn flag */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_IM_CN_FLAG_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_IM_CN_FLAG_TLV_ID ,
                                1,
                                (void *)&profile_params->im_cn_flag) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* TFT Filter ID 1 */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_TFT_FILTER_ID_1_PARAM_MASK)
  {
    if (qmi_wds_write_tft_filter (tx_buf,
                                  tx_buf_len,
                                  QMI_WDS_UMTS_PROFILE_TFT_FILTER_ID_1_TLV_ID,
                                  &profile_params->tft_filter_id_1) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* TFT Filter ID 2 */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_TFT_FILTER_ID_2_PARAM_MASK)
  {
    if (qmi_wds_write_tft_filter (tx_buf,
                                  tx_buf_len,
                                  QMI_WDS_UMTS_PROFILE_TFT_FILTER_ID_2_TLV_ID,
                                  &profile_params->tft_filter_id_2) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* PDP Context numner */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_PDP_CONTEXT_NUMBER_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_PDP_CONTEXT_NUMBER_TLV_ID ,
                                1,
                                (void *)&profile_params->pdp_context_number) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* PDP context secondary ID */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_PDP_CONTEXT_SECONDARY_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_PDP_CONTEXT_SECONDARY_TLV_ID ,
                                1,
                                (void *)&profile_params->pdp_context_secondary) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* PDP context primary id number */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_PDP_PRIMARY_ID_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_PDP_PRIMARY_ID_TLV_ID,
                                1,
                                (void *)&profile_params->pdp_context_primary_id_number) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* IPv6 address preferrence */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_IPV6_ADDR_PREF_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_IPV6_ADDR_PREF_TLV_ID ,
                                QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES,
                                (void *)profile_params->ipv6_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* UMTS minimum QOS with signal indication */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_UMTS_MIN_QOS_EXT_PARAM_MASK)
  {
    if (qmi_wds_write_umts_qos_tlv (tx_buf,
                                    tx_buf_len,
                                    QMI_WDS_UMTS_PROFILE_UMTS_MIN_QOS_EXT_TLV_ID,
                                    &profile_params->umts_minimum_qos_sig_ind,
                                    TRUE) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* UMTS requested QOS with signal indication */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_UMTS_REQ_QOS_EXT_PARAM_MASK)
  {
    if (qmi_wds_write_umts_qos_tlv (tx_buf,
                                    tx_buf_len,
                                    QMI_WDS_UMTS_PROFILE_UMTS_REQ_QOS_EXT_TLV_ID,
                                    &profile_params->umts_requested_qos_sig_ind,
                                    TRUE) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* primary DNS IPv6 address preference */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_PRIMARY_DNS_IPV6_ADDR_PREF_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_PRIMARY_DNS_IPV6_ADDR_PREF_TLV_ID ,
                                QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES,
                                (void *)profile_params->primary_dns_ipv6_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* secondary DNS IPv6 address preference */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_PARAM_MASK)
  {
   if (qmi_util_write_std_tlv (tx_buf,
                               tx_buf_len,
                               QMI_WDS_UMTS_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_TLV_ID ,
                               QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES,
                               (void *)profile_params->secondary_dns_ipv6_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* Address Allocation preference DHCP or NAS */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_ADDR_ALLOC_PREF_PARAM_MASK)
  {
   if (qmi_util_write_std_tlv (tx_buf,
                               tx_buf_len,
                               QMI_WDS_UMTS_PROFILE_ADDR_ALLOC_PREF_TLV_ID ,
                               1,
                               (void *)&profile_params->addr_alloc_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* 3GPP LTE QoS Parameters */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_LTE_QOS_PARAMS_PARAM_MASK)
  {
   if (qmi_wds_write_lte_qos_tlv (tx_buf,
                                  tx_buf_len,
                                  QMI_WDS_UMTS_PROFILE_LTE_QOS_PARAMS_TLV_ID,
                                  &profile_params->lte_qos_params) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* If set any call made on this profile fail locally as apn is disabled */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_APN_DISABLED_FLAG_PARAM_MASK)
  {
   if (qmi_util_write_std_tlv (tx_buf,
                               tx_buf_len,
                               QMI_WDS_UMTS_PROFILE_APN_DISABLED_FLAG_TLV_ID ,
                               1,
                               (void *)&profile_params->apn_disabled_flag) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Duration of inactivity (PDN) timer in seconds */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_PDN_INACTIVITY_TIMEOUT_PARAM_MASK)
  {
   if (qmi_util_write_std_tlv (tx_buf,
                               tx_buf_len,
                               QMI_WDS_UMTS_PROFILE_PDN_INACTIVITY_TIMEOUT_TLV_ID ,
                               4,
                               (void *)&profile_params->pdn_inactivity_timeout) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Not used by modem It can be set and querried any time though */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_APN_CLASS_PARAM_MASK)
  {
   if (qmi_util_write_std_tlv (tx_buf,
                               tx_buf_len,
                               QMI_WDS_UMTS_PROFILE_APN_CLASS_TLV_ID ,
                               1,
                               (void *)&profile_params->apn_class) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* APN bearer mask: Specifies whether a data call is allowed on specific RAT types */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_APN_BEARER_PARAM_MASK)
  {
   if (qmi_util_write_std_tlv (tx_buf,
                               tx_buf_len,
                               QMI_WDS_UMTS_PROFILE_APN_BEARER_TLV_ID ,
                               8,
                               (void *)&profile_params->apn_bearer) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Support emergency calls */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_SUPPORT_EMERGENCY_CALLS_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_SUPPORT_EMERGENCY_CALLS_TLV_ID,
                                1,
                                (void *)&profile_params->support_emergency_calls) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Operator Reserved PCO ID: If op_pco_id is configured, the UE sends
     the operator PCO with the container ID that is configured.
     Once configured, the profile cannot be unconfigured */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_OP_PCO_ID_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_OP_PCO_ID_TLV_ID,
                                2,
                                (void *)&profile_params->op_pco_id) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Mobile Country Code */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_PCO_MCC_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_PCO_MCC_TLV_ID,
                                2,
                                (void *)&profile_params->pco_mcc) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Mobile Network Code */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_MNC_PARAM_MASK)
  {
    if (qmi_util_write_mnc_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_MNC_TLV_ID,
                                &profile_params->mnc) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Persistence Flag */
  if (profile_params->param_mask & QMI_WDS_UMTS_PROFILE_IS_PERSISTENT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_PROFILE_IS_PERSISTENT_TLV_ID,
                                1,
                                (void *)&profile_params->is_persistent) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Max PDN connections per Time block*/
  if (profile_params->param_mask & QMI_WDS_UMTS_MAX_PDN_CONN_PER_BLOCK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_MAX_PDN_CONN_PER_BLOCK_TLV_ID,
                                2,
                                (void *)&profile_params->max_pdn_conn_per_block) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Max PDN connection Timer */
  if (profile_params->param_mask & QMI_WDS_UMTS_MAX_PDN_CONN_TIMER)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_MAX_PDN_CONN_TIMER_TLV_ID,
                                2,
                                (void *)&profile_params->max_pdn_conn_timer) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Min. time interval beween PDN connections request */
  if (profile_params->param_mask & QMI_WDS_UMTS_PDN_REQ_WAIT_INTERVAL)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_UMTS_PROFILE_PDN_REQ_WAIT_INTERVAL_TLV_ID,
                                2,
                                (void *)&profile_params->pdn_req_wait_interval) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  return QMI_NO_ERR;
} /* qmi_wds_write_umts_optional_profile_tlvs */


/*===========================================================================
  FUNCTION  qmi_wds_write_cdma_optional_profile_tlvs
===========================================================================*/
/*!
@brief
  Takes the input CDMA profile data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int
qmi_wds_write_cdma_optional_profile_tlvs
(
  unsigned char                      **tx_buf,
  int                                *tx_buf_len,
  qmi_wds_cdma_profile_params_type   *profile_params
)
{

  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_DNS_SERVER_PREF_PARAM_MASK)
  {
    unsigned char temp = (unsigned char)profile_params->dns_server_pref;
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_DNS_SERVER_PREF_TLV_ID,
                                1,
                                (void *)&temp) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_DO_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_DO_TLV_ID,
                                4,
                                (void *)&profile_params->ppp_session_close_timer_do) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_1X_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_1X_TLV_ID,
                                4,
                                (void *)&profile_params->ppp_session_close_timer_1x) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_ALLOW_LINGER_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_ALLOW_LINGER_TLV_ID,
                                1,
                                (void *)&profile_params->allow_linger) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_LCP_ACK_TIMEOUT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_LCP_ACK_TIMEOUT_TLV_ID,
                                2,
                                (void *)&profile_params->lcp_ack_timeout) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_IPCP_ACK_TIMEOUT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_IPCP_ACK_TIMEOUT_TLV_ID,
                                2,
                                (void *)&profile_params->ipcp_ack_timeout) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_AUTH_TIMEOUT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_AUTH_TIMEOUT_TLV_ID,
                                2,
                                (void *)&profile_params->auth_timeout) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_LCP_CONFIG_RETRY_RECOUNT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_LCP_CONFIG_RETRY_RECOUNT_TLV_ID,
                                1,
                                (void *)&profile_params->lcp_config_req_retry_count) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_IPCP_CONFIG_RETRY_RECOUNT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_IPCP_CONFIG_RETRY_RECOUNT_TLV_ID,
                                1,
                                (void *)&profile_params->ipcp_config_req_retry_count) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_AUTH_RETRY_RECOUNT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_AUTH_RETRY_RECOUNT_TLV_ID,
                                1,
                                (void *)&profile_params->auth_retry_count) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_AUTH_PROTOCOL_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_AUTH_PROTOCOL_TLV_ID,
                                1,
                                (void *)&profile_params->auth_protocol) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_USERNAME_PARAM_MASK)
  {
    size_t tmp_len = strlen (profile_params->username);
    if (tmp_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      QMI_ERR_MSG_1 ("qmi_wds_write_cdma_optional_profile_tlvs: username too large, size=%d\n",tmp_len);
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_USERNAME_TLV_ID,
                                tmp_len,
                                (void *)profile_params->username) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_PASSWORD_PARAM_MASK)
  {
    size_t tmp_len = strlen (profile_params->password);
    if (tmp_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      QMI_ERR_MSG_1 ("qmi_wds_write_cdma_optional_profile_tlvs: password too large, size=%d\n",tmp_len);
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_PASSWORD_TLV_ID,
                                tmp_len,
                                (void *)profile_params->password) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_DATA_RATE_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_DATA_RATE_TLV_ID,
                                1,
                                (void *)&profile_params->data_rate) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_DATA_MODE_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_DATA_MODE_TLV_ID,
                                1,
                                (void *)&profile_params->data_mode) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_APP_TYPE_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_APP_TYPE_TLV_ID,
                                4,
                                (void *)&profile_params->app_type) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_APP_PRIORITY_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_APP_PRIORITY_TLV_ID,
                                1,
                                (void *)&profile_params->app_priority) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_APN_STRING_PARAM_MASK)
  {
    size_t tmp_len = strlen (profile_params->apn_name);
    if (tmp_len >= QMI_WDS_MAX_APN_STR_SIZE)
    {
      QMI_ERR_MSG_1 ("qmi_wds_write_cdma_optional_profile_tlvs: apn_name too large, size=%d\n",tmp_len);
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_APN_STRING_TLV_ID,
                                tmp_len,
                                (void *)profile_params->apn_name) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_PDN_TYPE_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_PDN_TYPE_TLV_ID,
                                1,
                                (void *)&profile_params->pdn_type) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_IS_PCSCF_ADDR_NEEDED_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_IS_PCSCF_ADDR_NEEDED_TLV_ID,
                                1,
                                (void *)&profile_params->is_pcscf_addr_needed) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_PRIM_V4_DNS_ADDR_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_PRIM_V4_DNS_ADDR_TLV_ID,
                                4,
                                (void *)&profile_params->primary_dns_ipv4_pref_addr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_SEC_V4_DNS_ADDR_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_SEC_V4_DNS_ADDR_TLV_ID,
                                4,
                                (void *)&profile_params->secondary_dns_ipv4_pref_addr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_PRIM_V6_DNS_ADDR_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_PRIM_V6_DNS_ADDR_TLV_ID,
                                QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES,
                                (void *)profile_params->primary_dns_ipv6_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_SEC_V6_DNS_ADDR_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_SEC_V6_DNS_ADDR_TLV_ID,
                                QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES,
                                (void *)profile_params->secondary_dns_ipv6_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_RAT_TYPE_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_RAT_TYPE_TLV_ID,
                                1,
                                (void *)&profile_params->rat_type) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* Persistence Flag */
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_IS_PERSISTENT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_PROFILE_IS_PERSISTENT_TLV_ID,
                                1,
                                (void *)&profile_params->is_persistent) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* APN Enabled Flag */
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_APN_ENABLED_PARAM_MASK)
  {
    qmi_wds_bool_type temp = !(profile_params->apn_disabled_flag);

    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_APN_ENABLED_TLV_ID,
                                1,
                                (void *)&temp) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* PDP inactivity time out */
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_PDN_INACTIVITY_TIMEOUT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_PDN_INACTIVITY_TIMEOUT_TLV_ID,
                                4,
                                (void *)&profile_params->pdn_inactivity_timeout) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* APN Class */
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_APN_CLASS_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_APN_CLASS_TLV_ID,
                                1,
                                (void *)&profile_params->apn_class) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* PDN level Auth protocol */
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_TLV_ID,
                                1,
                                (void *)&profile_params->pdn_level_auth_protocol) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* PDN level User ID */
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_PDN_LEVEL_USER_ID_PARAM_MASK)
  {
    int tmp_len = strlen (profile_params->pdn_level_user_id);
    if (tmp_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      QMI_ERR_MSG_1 ("qmi_wds_write_cdma_optional_profile_tlvs: PDN level user id too large, size=%d\n",tmp_len);
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_PDN_LEVEL_USER_ID_TLV_ID,
                                tmp_len,
                                (void *)profile_params->pdn_level_user_id) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* PDN level auth password */
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PWD_PARAM_MASK)
  {
    int tmp_len = strlen (profile_params->pdn_level_auth_pwd);
    if (tmp_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      QMI_ERR_MSG_1 ("qmi_wds_write_cdma_optional_profile_tlvs: PDN level auth pwd too large, size=%d\n",tmp_len);
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PWD_TLV_ID,
                                tmp_len,
                                (void *)profile_params->pdn_level_auth_pwd) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* PDN Label */
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_PDN_LABEL_PARAM_MASK)
  {
    int tmp_len = strlen (profile_params->pdn_label);
    if (tmp_len >= QMI_WDS_MAX_APN_STR_SIZE)
    {
      QMI_ERR_MSG_1 ("qmi_wds_write_cdma_optional_profile_tlvs: PDN label too large, size=%d\n",tmp_len);
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_PDN_LABEL_TLV_ID,
                                tmp_len,
                                (void *)profile_params->pdn_label) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* Operator Reserved PCO ID */
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_OPERATOR_RESERVED_PCO_ID_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_OPERATOR_RESERVED_PCO_ID_TLV_ID,
                                2,
                                (void *)&profile_params->op_pco_id) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* 3GPP2 MCC */
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_MCC_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_MCC_TLV_ID,
                                2,
                                (void *)&profile_params->pco_mcc) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* 3GPP2 MNC */
  if (profile_params->param_mask & QMI_WDS_CDMA_PROFILE_MNC_PARAM_MASK)
  {
    if (qmi_util_write_mnc_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_CDMA_PROFILE_MNC_TLV_ID,
                                &profile_params->mnc) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  return QMI_NO_ERR;
}/*qmi_wds_write_cdma_optional_profile_tlvs*/

/*===========================================================================
  FUNCTION  qmi_wds_write_epc_optional_profile_tlvs
===========================================================================*/
/*!
@brief
  Takes the input EPC profile data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int
qmi_wds_write_epc_optional_profile_tlvs
(
  unsigned char                      **tx_buf,
  int                                *tx_buf_len,
  qmi_wds_epc_profile_params_type   *profile_params
)
{

  /* Profile name */
  if (profile_params->param_mask & QMI_WDS_EPC_PROFILE_NAME_PARAM_MASK)
  {
    int name_len = strlen (profile_params->profile_name);
    if (name_len >= QMI_WDS_MAX_PROFILE_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_NAME_TLV_ID,
                                   name_len,
                                   (void *)profile_params->profile_name) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* PDP type */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_PDP_TYPE_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_PDP_TYPE_TLV_ID,
                                   1,
                                   (void *)&profile_params->pdp_type) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* APN name */
  if (profile_params->param_mask & QMI_WDS_EPC_PROFILE_APN_NAME_PARAM_MASK)
  {
    int name_len = strlen (profile_params->apn_name);
    if (name_len >= QMI_WDS_MAX_APN_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_APN_NAME_TLV_ID,
                                   name_len,
                                   (void *)profile_params->apn_name) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Primary DNS address preference */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_PRIM_DNS_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_PRIM_DNS_TLV_ID,
                                   4,
                                   (void *)&profile_params->primary_dns_pref_addr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Secondary DNS address preference */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_SEC_DNS_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_SEC_DNS_TLV_ID,
                                   4,
                                   (void *)&profile_params->secondary_dns_pref_addr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* UMTS requested QOS parameters */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_UMTS_REQ_QOS_PARAM_MASK)
  {
    if (qmi_wds_write_umts_qos_tlv (tx_buf,
                                    tx_buf_len,
                                    QMI_WDS_EPC_UMTS_PROFILE_UMTS_REQ_QOS_TLV_ID,
                                    &profile_params->umts_requested_qos,
                                    FALSE) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* UMTS minimum QOS parameters */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_UMTS_MIN_QOS_PARAM_MASK)
  {
    if (qmi_wds_write_umts_qos_tlv (tx_buf,
                                    tx_buf_len,
                                    QMI_WDS_EPC_UMTS_PROFILE_UMTS_MIN_QOS_TLV_ID,
                                    &profile_params->umts_minimum_qos,
                                    FALSE) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* GPRS requested QOS parameters */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_GPRS_REQ_QOS_PARAM_MASK)
  {
    if (qmi_wds_write_gprs_qos_tlv (tx_buf,
                                    tx_buf_len,
                                    QMI_WDS_EPC_UMTS_PROFILE_GPRS_REQ_QOS_TLV_ID,
                                    &profile_params->gprs_requested_qos) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* UMTS minimum QOS parameters */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_GPRS_MIN_QOS_PARAM_MASK)
  {
    if (qmi_wds_write_gprs_qos_tlv (tx_buf,
                                    tx_buf_len,
                                    QMI_WDS_EPC_UMTS_PROFILE_GPRS_MIN_QOS_TLV_ID,
                                    &profile_params->gprs_minimum_qos) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Username */
  if (profile_params->param_mask & QMI_WDS_EPC_PROFILE_USERNAME_PARAM_MASK)
  {
    int name_len = strlen (profile_params->username);
    if (name_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_USERNAME_TLV_ID,
                                   name_len,
                                   (void *)profile_params->username) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Password */
  if (profile_params->param_mask & QMI_WDS_EPC_PROFILE_PASSWORD_PARAM_MASK)
  {
    int name_len = strlen (profile_params->password);
    if (name_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_PASSWORD_TLV_ID,
                                   name_len,
                                   (void *)profile_params->password) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Authentication preference */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_AUTH_PREF_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_AUTH_PREF_TLV_ID,
                                   1,
                                   (void *)&profile_params->auth_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* IPV4 address preference */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_IPV4_ADDR_PREF_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_IPV4_ADDR_PREF_TLV_ID,
                                   4,
                                   (void *)&profile_params->ipv4_pref_addr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* PCSCF address using PCO */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_PCSCF_ADDR_VIA_PCO_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_PCSCF_ADDR_VIA_PCO_TLV_ID,
                                   1,
                                   (void *)&profile_params->pcscf_addr_via_pco) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Header Compression */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_HEADER_COMPRESSION_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_HEADER_COMPRESSION_TLV_ID,
                                1,
                                (void *)&profile_params->header_compression) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* Data Compression */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_DATA_COMPRESSION_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_DATA_COMPRESSION_TLV_ID ,
                                1,
                                (void *)&profile_params->data_compression) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Pdp accesss comtrol */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_PDP_ACCESS_CONTROL_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_PDP_ACCESS_CONTROL_TLV_ID ,
                                1,
                                (void *)&profile_params->pdp_access_control) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Pcscf via dhcp */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_PCSCF_VIA_DHCP_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_PCSCF_VIA_DHCP_TLV_ID ,
                                1,
                                (void *)&profile_params->pcscf_via_dhcp) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Im cn flag */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_IM_CN_FLAG_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_IM_CN_FLAG_TLV_ID ,
                                1,
                                (void *)&profile_params->im_cn_flag) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* TFT Filter ID 1 */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_TFT_FILTER_ID_1_PARAM_MASK)
  {
    if (qmi_wds_write_tft_filter (tx_buf,
                                  tx_buf_len,
                                  QMI_WDS_EPC_UMTS_PROFILE_TFT_FILTER_ID_1_TLV_ID,
                                  &profile_params->tft_filter_id_1) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* TFT Filter ID 2 */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_TFT_FILTER_ID_2_PARAM_MASK)
  {
    if (qmi_wds_write_tft_filter (tx_buf,
                                  tx_buf_len,
                                  QMI_WDS_EPC_UMTS_PROFILE_TFT_FILTER_ID_2_TLV_ID,
                                  &profile_params->tft_filter_id_2) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* PDP Context numner */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_PDP_CONTEXT_NUMBER_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_PDP_CONTEXT_NUMBER_TLV_ID ,
                                1,
                                (void *)&profile_params->pdp_context_number) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* PDP context secondary ID */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_PDP_CONTEXT_SECONDARY_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_PDP_CONTEXT_SECONDARY_TLV_ID ,
                                1,
                                (void *)&profile_params->pdp_context_secondary) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* PDP context primary id number */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_PDP_PRIMARY_ID_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_PDP_PRIMARY_ID_TLV_ID,
                                1,
                                (void *)&profile_params->pdp_context_primary_id_number) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* IPv6 address preferrence */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_IPV6_ADDR_PREF_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_IPV6_ADDR_PREF_TLV_ID ,
                                QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES,
                                (void *)profile_params->ipv6_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* UMTS minimum QOS with signal indication */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_UMTS_MIN_QOS_EXT_PARAM_MASK)
  {
    if (qmi_wds_write_umts_qos_tlv (tx_buf,
                                    tx_buf_len,
                                    QMI_WDS_EPC_UMTS_PROFILE_UMTS_MIN_QOS_EXT_TLV_ID,
                                    &profile_params->umts_minimum_qos_sig_ind,
                                    TRUE) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* UMTS requested QOS with signal indication */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_UMTS_REQ_QOS_EXT_PARAM_MASK)
  {
    if (qmi_wds_write_umts_qos_tlv (tx_buf,
                                    tx_buf_len,
                                    QMI_WDS_EPC_UMTS_PROFILE_UMTS_REQ_QOS_EXT_TLV_ID,
                                    &profile_params->umts_requested_qos_sig_ind,
                                    TRUE) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* primary DNS IPv6 address preference */
  if (profile_params->param_mask & QMI_WDS_EPC_PROFILE_PRIMARY_DNS_IPV6_ADDR_PREF_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_PRIMARY_DNS_IPV6_ADDR_PREF_TLV_ID ,
                                QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES,
                                (void *)profile_params->primary_dns_ipv6_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* secondary DNS IPv6 address preference */
  if (profile_params->param_mask & QMI_WDS_EPC_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_PARAM_MASK)
  {
   if (qmi_util_write_std_tlv (tx_buf,
                               tx_buf_len,
                               QMI_WDS_EPC_UMTS_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_TLV_ID ,
                               QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES,
                               (void *)profile_params->secondary_dns_ipv6_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Support emergency calls */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PROFILE_SUPPORT_EMERGENCY_CALLS_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_SUPPORT_EMERGENCY_CALLS_TLV_ID,
                                1,
                                (void *)&profile_params->support_emergency_calls) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_DNS_SERVER_PREF_PARAM_MASK)
  {
    unsigned char temp = (unsigned char)profile_params->dns_server_pref;
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_DNS_SERVER_PREF_TLV_ID,
                                1,
                                (void *)&temp) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_DO_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_DO_TLV_ID,
                                4,
                                (void *)&profile_params->ppp_session_close_timer_do) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_1X_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_1X_TLV_ID,
                                4,
                                (void *)&profile_params->ppp_session_close_timer_1x) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_ALLOW_LINGER_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_ALLOW_LINGER_TLV_ID,
                                1,
                                (void *)&profile_params->allow_linger) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_LCP_ACK_TIMEOUT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_LCP_ACK_TIMEOUT_TLV_ID,
                                2,
                                (void *)&profile_params->lcp_ack_timeout) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_IPCP_ACK_TIMEOUT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_IPCP_ACK_TIMEOUT_TLV_ID,
                                2,
                                (void *)&profile_params->ipcp_ack_timeout) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_AUTH_TIMEOUT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_AUTH_TIMEOUT_TLV_ID,
                                2,
                                (void *)&profile_params->auth_timeout) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_LCP_CONFIG_RETRY_RECOUNT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_LCP_CONFIG_RETRY_RECOUNT_TLV_ID,
                                1,
                                (void *)&profile_params->lcp_config_req_retry_count) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_IPCP_CONFIG_RETRY_RECOUNT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_IPCP_CONFIG_RETRY_RECOUNT_TLV_ID,
                                1,
                                (void *)&profile_params->ipcp_config_req_retry_count) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_AUTH_RETRY_RECOUNT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_AUTH_RETRY_RECOUNT_TLV_ID,
                                1,
                                (void *)&profile_params->auth_retry_count) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_AUTH_PROTOCOL_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_AUTH_PROTOCOL_TLV_ID,
                                1,
                                (void *)&profile_params->auth_protocol) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_PROFILE_USERNAME_PARAM_MASK)
  {
    int tmp_len = strlen (profile_params->username);
    if (tmp_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      QMI_ERR_MSG_1 ("qmi_wds_write_cdma_optional_profile_tlvs: username too large, size=%d\n",tmp_len);
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_USERNAME_TLV_ID,
                                tmp_len,
                                (void *)profile_params->username) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_PROFILE_PASSWORD_PARAM_MASK)
  {
    int tmp_len = strlen (profile_params->password);
    if (tmp_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      QMI_ERR_MSG_1 ("qmi_wds_write_cdma_optional_profile_tlvs: password too large, size=%d\n",tmp_len);
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_PASSWORD_TLV_ID,
                                tmp_len,
                                (void *)profile_params->password) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_DATA_RATE_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_DATA_RATE_TLV_ID,
                                1,
                                (void *)&profile_params->data_rate) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_DATA_MODE_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_DATA_MODE_TLV_ID,
                                1,
                                (void *)&profile_params->data_mode) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_APP_TYPE_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_APP_TYPE_TLV_ID,
                                4,
                                (void *)&profile_params->app_type) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_APP_PRIORITY_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_APP_PRIORITY_TLV_ID,
                                1,
                                (void *)&profile_params->app_priority) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_PROFILE_APN_NAME_PARAM_MASK)
  {
    int tmp_len = strlen (profile_params->apn_name);
    if (tmp_len >= QMI_WDS_MAX_APN_STR_SIZE)
    {
      QMI_ERR_MSG_1 ("qmi_wds_write_cdma_optional_profile_tlvs: apn_name too large, size=%d\n",tmp_len);
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_APN_STRING_TLV_ID,
                                tmp_len,
                                (void *)profile_params->apn_name) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_PDN_TYPE_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_PDN_TYPE_TLV_ID,
                                1,
                                (void *)&profile_params->pdn_type) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_IS_PCSCF_ADDR_NEEDED_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_IS_PCSCF_ADDR_NEEDED_TLV_ID,
                                1,
                                (void *)&profile_params->is_pcscf_addr_needed) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_PRIM_V4_DNS_ADDR_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_PRIM_V4_DNS_ADDR_TLV_ID,
                                4,
                                (void *)&profile_params->primary_dns_ipv4_pref_addr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_SEC_V4_DNS_ADDR_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_SEC_V4_DNS_ADDR_TLV_ID,
                                4,
                                (void *)&profile_params->secondary_dns_ipv4_pref_addr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_PROFILE_PRIMARY_DNS_IPV6_ADDR_PREF_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_PRIM_V6_DNS_ADDR_TLV_ID,
                                QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES,
                                (void *)profile_params->primary_dns_ipv6_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_SEC_V6_DNS_ADDR_TLV_ID,
                                QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES,
                                (void *)profile_params->secondary_dns_ipv6_addr_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_CDMA_PROFILE_RAT_TYPE_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_CDMA_PROFILE_RAT_TYPE_TLV_ID,
                                1,
                                (void *)&profile_params->rat_type) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* Persistence Flag */
  if (profile_params->param_mask & QMI_WDS_EPC_PROFILE_IS_PERSISTENT_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_PROFILE_IS_PERSISTENT_TLV_ID,
                                1,
                                (void *)&profile_params->is_persistent) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (profile_params->param_mask & QMI_WDS_EPC_COMMON_APN_CLASS_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_COMMON_APN_CLASS_TLV_ID ,
                                1,
                                (void *)&profile_params->common_apn_class) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (profile_params->param_mask & QMI_WDS_EPC_COMMON_APN_DISABLED_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_COMMON_APN_DISABLED_FLAG_TLV_ID ,
                                1,
                                (void *)&profile_params->common_apn_disabled_flag) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* Username */
  if (profile_params->param_mask & QMI_WDS_EPC_COMMON_USER_ID_PARAM_MASK)
  {
    int name_len = strlen (profile_params->username);
    if (name_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_COMMON_USER_ID_TLV_ID,
                                name_len,
                                (void *)profile_params->username) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Password */
  if (profile_params->param_mask & QMI_WDS_EPC_COMMON_AUTH_PASSWORD_PARAM_MASK)
  {
    int name_len = strlen (profile_params->password);
    if (name_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_COMMON_AUTH_PASSWORD_TLV_ID,
                                name_len,
                                (void *)profile_params->password) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Authentication preference */
  if (profile_params->param_mask & QMI_WDS_EPC_COMMON_AUTH_PROTOCOL_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_COMMON_AUTH_PROTOCOL_TLV_ID,
                                1,
                                (void *)&profile_params->auth_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Max PDN connections per Time block*/
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_MAX_PDN_CONN_PER_BLOCK)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_MAX_PDN_CONN_PER_BLOCK_TLV_ID,
                                2,
                                (void *)&profile_params->max_pdn_conn_per_block) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Max PDN connection Timer */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_MAX_PDN_CONN_TIMER)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_MAX_PDN_CONN_TIMER_TLV_ID,
                                2,
                                (void *)&profile_params->max_pdn_conn_timer) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Min. time interval beween PDN connections request */
  if (profile_params->param_mask & QMI_WDS_EPC_UMTS_PDN_REQ_WAIT_INTERVAL)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                tx_buf_len,
                                QMI_WDS_EPC_UMTS_PROFILE_PDN_REQ_WAIT_INTERVAL_TLV_ID,
                                2,
                                (void *)&profile_params->pdn_req_wait_interval) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if((profile_params->param_mask & QMI_WDS_EPC_EXTENDED_PARAM_MASK) ==
       QMI_WDS_EPC_EXTENDED_PARAM_MASK)
  {
    /* PDN level AUTH Type TLV for EPC profile is the same as that of 3GPP2 profile */

    /* PDN level Auth protocol */
    if (profile_params->extended_param_mask &
                        QMI_WDS_EPC_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PARAM_MASK)
    {
      if (qmi_util_write_std_tlv (tx_buf,
                                  tx_buf_len,
                                  QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_TLV_ID,
                                  1,
                                  (void *)&profile_params->pdn_level_auth_protocol) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }

    /* PDN level USER ID TLV for EPC profile is the same as that of 3GPP2 profile */
    if (profile_params->extended_param_mask & QMI_WDS_EPC_PROFILE_PDN_LEVEL_USER_ID_PARAM_MASK)
    {
      int tmp_len = strlen (profile_params->pdn_level_user_id);
      if (tmp_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
      {
        return QMI_INTERNAL_ERR;
      }
      if (qmi_util_write_std_tlv (tx_buf,
                                  tx_buf_len,
                                  QMI_WDS_CDMA_PROFILE_PDN_LEVEL_USER_ID_TLV_ID,
                                  tmp_len,
                                  (void *)profile_params->pdn_level_user_id) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }

    /* PDN level AUTH PWRD TLV for EPC profile is the same as that of 3GPP2 profile */
    if (profile_params->extended_param_mask & QMI_WDS_EPC_PROFILE_PDN_LEVEL_AUTH_PWD_PARAM_MASK)
    {
      int tmp_len = strlen (profile_params->pdn_level_auth_pwd);

      if (tmp_len >= QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
      {
        return QMI_INTERNAL_ERR;
      }
      if (qmi_util_write_std_tlv (tx_buf,
                                  tx_buf_len,
                                  QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PWD_TLV_ID,
                                  tmp_len,
                                  (void *)profile_params->pdn_level_auth_pwd) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }
      /* Duration of inactivity (PDN) timer in seconds. 3GPP TLV is the same for EPC profile*/
    if (profile_params->extended_param_mask & QMI_WDS_EPC_PROFILE_PDN_LEVEL_INACTIVITY_TIMEOUT_MASK)
    {
     if (qmi_util_write_std_tlv (tx_buf,
                                 tx_buf_len,
                                 QMI_WDS_UMTS_PROFILE_PDN_INACTIVITY_TIMEOUT_TLV_ID ,
                                 4,
                                 (void *)&profile_params->pdn_inactivity_timeout) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }
      /* Duration of inactivity (PDN) timer in seconds for 3GPP2*/
    if (profile_params->extended_param_mask & QMI_WDS_EPC_PROFILE_PDN_LEVEL_INACTIVITY_TIMEOUT_3GPP2_MASK)
    {
     if (qmi_util_write_std_tlv (tx_buf,
                                 tx_buf_len,
                                 QMI_WDS_EPC_CDMA_PROFILE_PDN_INACTIVITY_TIMEOUT_TLV_ID ,
                                 4,
                                 (void *)&profile_params->pdn_inactivity_timeout_3gpp2) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }
  }

  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_read_umts_common_profile_tlvs
===========================================================================*/
/*!
@brief
  Takes an input RX buffer containing UMTS profile TLV data
  and reads values into a profile paramaters structure.


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
qmi_wds_read_umts_common_profile_tlvs
(
  unsigned long               type,
  unsigned long               length,
  unsigned char               *value_ptr,
  qmi_wds_umts_profile_params_type   *profile_params
)
{
  /* Declare/initialize return code */
  int rc = TRUE;
  unsigned char tmp_char;


  /* Now process TLV */
  switch (type)
  {
    case QMI_WDS_UMTS_PROFILE_NAME_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_NAME_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        unsigned long cpy_len = (length < QMI_WDS_MAX_PROFILE_STR_SIZE)
                                ? length : (QMI_WDS_MAX_PROFILE_STR_SIZE - 1);
        memcpy (profile_params->profile_name, (void *)value_ptr, cpy_len);
        profile_params->profile_name[cpy_len] = '\0';
      }
      else
      {
        profile_params->profile_name[0] = '\0';
      }
    }
    break;

    case QMI_WDS_UMTS_PROFILE_PDP_TYPE_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_PDP_TYPE_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pdp_type = (qmi_wds_pdp_type) tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_APN_NAME_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_APN_NAME_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        unsigned long cpy_len = (length < QMI_WDS_MAX_APN_STR_SIZE)
                                ? length : (QMI_WDS_MAX_APN_STR_SIZE - 1);
        memcpy (profile_params->apn_name, (void *)value_ptr, cpy_len);
        profile_params->apn_name[cpy_len] = '\0';
      }
      else
      {
        profile_params->apn_name[0] = '\0';
      }
    }
    break;

    case QMI_WDS_UMTS_PROFILE_PRIM_DNS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_PRIM_DNS_PARAM_MASK;
      READ_32_BIT_VAL (value_ptr, profile_params->primary_dns_pref_addr);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_SEC_DNS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_SEC_DNS_PARAM_MASK;
      READ_32_BIT_VAL (value_ptr, profile_params->secondary_dns_pref_addr);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_UMTS_REQ_QOS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_UMTS_REQ_QOS_PARAM_MASK;
        (void)qmi_wds_read_umts_qos_tlv (value_ptr,&profile_params->umts_requested_qos, FALSE);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_UMTS_MIN_QOS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_UMTS_MIN_QOS_PARAM_MASK;
        (void)qmi_wds_read_umts_qos_tlv (value_ptr,&profile_params->umts_minimum_qos, FALSE);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_GPRS_REQ_QOS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_GPRS_REQ_QOS_PARAM_MASK;
        (void)qmi_wds_read_gprs_qos_tlv (value_ptr,&profile_params->gprs_requested_qos);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_GPRS_MIN_QOS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_GPRS_MIN_QOS_PARAM_MASK;
        (void)qmi_wds_read_gprs_qos_tlv (value_ptr,&profile_params->gprs_minimum_qos);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_USERNAME_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_USERNAME_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        unsigned long cpy_len = (length < QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
                                ? length : (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE - 1);
        memcpy (profile_params->username, (void *)value_ptr, cpy_len);
        profile_params->username[cpy_len] = '\0';
      }
      else
      {
        profile_params->username[0] = '\0';
      }
    }
    break;
    case QMI_WDS_UMTS_PROFILE_PASSWORD_TLV_ID :
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_PASSWORD_PARAM_MASK ;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
                      ? length : (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE - 1);
        memcpy (profile_params->password, (void *)value_ptr, cpy_len);
        profile_params->password[cpy_len] = '\0';
      }
      else
      {
        profile_params->password[0] = '\0';
      }
    }
    break;

    case QMI_WDS_UMTS_PROFILE_AUTH_PREF_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_AUTH_PREF_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->auth_pref = (qmi_wds_auth_pref_type) tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_IPV4_ADDR_PREF_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_IPV4_ADDR_PREF_PARAM_MASK;
      READ_32_BIT_VAL (value_ptr, profile_params->ipv4_pref_addr);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_PCSCF_ADDR_VIA_PCO_TLV_ID:
      {
        /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_PCSCF_ADDR_VIA_PCO_PARAM_MASK;
        READ_8_BIT_VAL (value_ptr, tmp_char);
        profile_params->pcscf_addr_via_pco = (qmi_wds_pcscf_via_pco_type) tmp_char;
      }
      break;

    case QMI_WDS_UMTS_PROFILE_HEADER_COMPRESSION_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_HEADER_COMPRESSION_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->header_compression = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_DATA_COMPRESSION_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_DATA_COMPRESSION_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->data_compression = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_PDP_ACCESS_CONTROL_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_PDP_ACCESS_CONTROL_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pdp_access_control = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_PCSCF_VIA_DHCP_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_PCSCF_VIA_DHCP_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pcscf_via_dhcp = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_IM_CN_FLAG_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_IM_CN_FLAG_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->im_cn_flag = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_TFT_FILTER_ID_1_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_TFT_FILTER_ID_1_PARAM_MASK;
     (void)qmi_wds_read_tft_filter (value_ptr,&profile_params->tft_filter_id_1);
    }
    break;

     case QMI_WDS_UMTS_PROFILE_TFT_FILTER_ID_2_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_TFT_FILTER_ID_2_PARAM_MASK;
     (void)qmi_wds_read_tft_filter (value_ptr,&profile_params->tft_filter_id_2);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_PDP_CONTEXT_NUMBER_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_PDP_CONTEXT_NUMBER_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pdp_context_number = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_PDP_CONTEXT_SECONDARY_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_PDP_CONTEXT_SECONDARY_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pdp_context_secondary = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_PDP_PRIMARY_ID_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_PDP_PRIMARY_ID_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pdp_context_primary_id_number = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_IPV6_ADDR_PREF_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      if (length == QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES )
      {
        profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_IPV6_ADDR_PREF_PARAM_MASK;
        memcpy(profile_params->ipv6_addr_pref, value_ptr, QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES);
      }
      else
      {
        QMI_ERR_MSG_1 ("qmi_wds_read_umts_common_profile_tlvs: Invalid length received for type: %u",(unsigned int) type);
      }
    }
    break;
    case QMI_WDS_UMTS_PROFILE_UMTS_REQ_QOS_EXT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_UMTS_REQ_QOS_EXT_PARAM_MASK;
      (void)qmi_wds_read_umts_qos_tlv (value_ptr,&profile_params->umts_requested_qos_sig_ind,TRUE);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_UMTS_MIN_QOS_EXT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_UMTS_MIN_QOS_EXT_PARAM_MASK;
      (void)qmi_wds_read_umts_qos_tlv (value_ptr,&profile_params->umts_minimum_qos_sig_ind,TRUE);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_PRIMARY_DNS_IPV6_ADDR_PREF_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      if (length == QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES )
      {
        profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_PRIMARY_DNS_IPV6_ADDR_PREF_PARAM_MASK;
        memcpy(profile_params->primary_dns_ipv6_addr_pref, value_ptr, QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES);
      }
      else
      {
        QMI_ERR_MSG_1 ("qmi_wds_read_umts_common_profile_tlvs: Invalid length received for type %u",(unsigned int) type);
      }
    }
    break;

    case QMI_WDS_UMTS_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      if (length == QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES )
      {
        profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_PARAM_MASK;
        memcpy(profile_params->secondary_dns_ipv6_addr_pref, value_ptr, QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES);
      }
      else
      {
        QMI_ERR_MSG_1 ("qmi_wds_read_umts_common_profile_tlvs: Invalid length received for type %u",(unsigned int) type);
      }
    }
    break;

    case QMI_WDS_UMTS_PROFILE_ADDR_ALLOC_PREF_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_ADDR_ALLOC_PREF_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->addr_alloc_pref = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_LTE_QOS_PARAMS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_LTE_QOS_PARAMS_PARAM_MASK;
      (void)qmi_wds_read_lte_qos_tlv (value_ptr,&profile_params->lte_qos_params);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_APN_DISABLED_FLAG_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_APN_DISABLED_FLAG_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->apn_disabled_flag = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_PDN_INACTIVITY_TIMEOUT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_PDN_INACTIVITY_TIMEOUT_PARAM_MASK;
      READ_32_BIT_VAL (value_ptr, profile_params->pdn_inactivity_timeout);

    }
    break;

    case QMI_WDS_UMTS_PROFILE_APN_CLASS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_APN_CLASS_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->apn_class = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_APN_BEARER_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_APN_BEARER_PARAM_MASK;
      READ_64_BIT_VAL (value_ptr, profile_params->apn_bearer);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_SUPPORT_EMERGENCY_CALLS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_SUPPORT_EMERGENCY_CALLS_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->support_emergency_calls = (qmi_wds_bool_type)tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_OP_PCO_ID_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_OP_PCO_ID_PARAM_MASK;
      READ_16_BIT_VAL (value_ptr, profile_params->op_pco_id);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_PCO_MCC_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_PCO_MCC_PARAM_MASK;
      READ_16_BIT_VAL (value_ptr, profile_params->pco_mcc);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_MNC_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_MNC_PARAM_MASK;
      (void)qmi_wds_read_mnc_tlv (value_ptr,&profile_params->mnc);
    }
    break;

    case QMI_WDS_PROFILE_IS_PERSISTENT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_IS_PERSISTENT_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->is_persistent = (qmi_wds_bool_type)tmp_char;
    }
    break;

    case QMI_WDS_UMTS_PROFILE_MAX_PDN_CONN_PER_BLOCK_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_MAX_PDN_CONN_PER_BLOCK;
      READ_16_BIT_VAL (value_ptr, profile_params->max_pdn_conn_per_block);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_MAX_PDN_CONN_TIMER_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_MAX_PDN_CONN_TIMER;
      READ_16_BIT_VAL (value_ptr, profile_params->max_pdn_conn_timer);
    }
    break;

    case QMI_WDS_UMTS_PROFILE_PDN_REQ_WAIT_INTERVAL_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_UMTS_PDN_REQ_WAIT_INTERVAL;
      READ_16_BIT_VAL (value_ptr, profile_params->pdn_req_wait_interval);
    }
    break;

    default:
    {
      rc = FALSE;
    }
    break;
  } /* switch */

  return rc;
} /* qmi_wds_read_umts_common_profile_tlvs */


/*===========================================================================
  FUNCTION  qmi_wds_read_cdma_common_profile_tlvs
===========================================================================*/
/*!
@brief
  Takes an input RX buffer containing CDMA profile TLV data
  and reads values into a profile paramaters structure.


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
qmi_wds_read_cdma_common_profile_tlvs
(
  unsigned long                      type,
  unsigned long                      length,
  unsigned char                      *value_ptr,
  qmi_wds_cdma_profile_params_type   *profile_params
)
{
   /* Declare/initialize return code */
  int rc = TRUE;
  unsigned char tmp_char;


  /* Now process TLV */
  switch (type)
  {
    case QMI_WDS_CDMA_PROFILE_DNS_SERVER_PREF_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_DNS_SERVER_PREF_PARAM_MASK;

       READ_8_BIT_VAL (value_ptr, tmp_char);
       profile_params->dns_server_pref = (qmi_wds_bool_type) tmp_char;
    }
    break;
    case QMI_WDS_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_DO_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_DO_PARAM_MASK;

      READ_32_BIT_VAL (value_ptr, profile_params->ppp_session_close_timer_do);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_1X_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_1X_PARAM_MASK;

      READ_32_BIT_VAL (value_ptr, profile_params->ppp_session_close_timer_1x);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_ALLOW_LINGER_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_ALLOW_LINGER_PARAM_MASK;

      READ_8_BIT_VAL (value_ptr,  profile_params->allow_linger);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_LCP_ACK_TIMEOUT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_LCP_ACK_TIMEOUT_PARAM_MASK;

      READ_16_BIT_VAL (value_ptr,  profile_params->lcp_ack_timeout);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_IPCP_ACK_TIMEOUT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_IPCP_ACK_TIMEOUT_PARAM_MASK;

      READ_16_BIT_VAL (value_ptr,  profile_params->ipcp_ack_timeout);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_AUTH_TIMEOUT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_AUTH_TIMEOUT_PARAM_MASK;

      READ_16_BIT_VAL (value_ptr,  profile_params->auth_timeout);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_AUTH_PROTOCOL_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_AUTH_PROTOCOL_PARAM_MASK;

      READ_8_BIT_VAL (value_ptr,  profile_params->auth_protocol);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_LCP_CONFIG_RETRY_RECOUNT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_LCP_CONFIG_RETRY_RECOUNT_PARAM_MASK;

      READ_8_BIT_VAL (value_ptr,  profile_params->lcp_config_req_retry_count);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_IPCP_CONFIG_RETRY_RECOUNT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_IPCP_CONFIG_RETRY_RECOUNT_PARAM_MASK;

      READ_8_BIT_VAL (value_ptr,  profile_params->ipcp_config_req_retry_count);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_AUTH_RETRY_RECOUNT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_AUTH_RETRY_RECOUNT_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr,  profile_params->auth_retry_count);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_USERNAME_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_USERNAME_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        unsigned long cpy_len = (length < QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
                                ? length : (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE - 1);
        memcpy (profile_params->username, (void *)value_ptr, cpy_len);
        profile_params->username[cpy_len] = '\0';
      }
      else
      {
        profile_params->username[0] = '\0';
      }
    }
    break;
    case QMI_WDS_CDMA_PROFILE_PASSWORD_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_PASSWORD_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        unsigned long cpy_len = (length < QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
                                ? length : (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE - 1);
        memcpy (profile_params->password, (void *)value_ptr, cpy_len);
        profile_params->password[cpy_len] = '\0';
      }
      else
      {
        profile_params->password[0] = '\0';
      }
    }
    break;
    case QMI_WDS_CDMA_PROFILE_DATA_RATE_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_DATA_RATE_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr,  tmp_char);
      profile_params->data_rate = (qmi_wds_cdma_data_rate_type) tmp_char;
    }
    break;
    case QMI_WDS_CDMA_PROFILE_DATA_MODE_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_DATA_MODE_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr,  tmp_char);
      profile_params->data_mode = (qmi_wds_cdma_data_mode_type) tmp_char;
    }
    break;
    case QMI_WDS_CDMA_PROFILE_APP_TYPE_TLV_ID:
    {
      unsigned int tmp_word;
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_APP_TYPE_PARAM_MASK;
      READ_32_BIT_VAL (value_ptr,  tmp_word);
      profile_params->app_type = (qmi_wds_cdma_app_type) tmp_word;
    }
    break;
    case QMI_WDS_CDMA_PROFILE_APP_PRIORITY_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_APP_PRIORITY_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr,  profile_params->app_priority);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_APN_STRING_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_APN_STRING_PARAM_MASK;
      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        unsigned long cpy_len = (length < QMI_WDS_MAX_APN_STR_SIZE)
                                ? length : (QMI_WDS_MAX_APN_STR_SIZE - 1);
        memcpy (profile_params->apn_name, (void *)value_ptr, cpy_len);
        profile_params->apn_name[cpy_len] = '\0';
      }
      else
      {
        profile_params->apn_name[0] = '\0';
      }
    }
    break;
    case QMI_WDS_CDMA_PROFILE_PDN_TYPE_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_PDN_TYPE_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pdn_type = (qmi_wds_cdma_pdn_type)tmp_char;
    }
    break;
    case QMI_WDS_CDMA_PROFILE_IS_PCSCF_ADDR_NEEDED_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_IS_PCSCF_ADDR_NEEDED_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->is_pcscf_addr_needed = (qmi_wds_bool_type)tmp_char;
    }
    break;
    case QMI_WDS_CDMA_PROFILE_PRIM_V4_DNS_ADDR_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_PRIM_V4_DNS_ADDR_PARAM_MASK;
      READ_32_BIT_VAL (value_ptr, profile_params->primary_dns_ipv4_pref_addr);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_SEC_V4_DNS_ADDR_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_SEC_V4_DNS_ADDR_PARAM_MASK;
      READ_32_BIT_VAL (value_ptr, profile_params->secondary_dns_ipv4_pref_addr);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_PRIM_V6_DNS_ADDR_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      if (length == QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES)
      {
        profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_PRIM_V6_DNS_ADDR_PARAM_MASK;
        memcpy(profile_params->primary_dns_ipv6_addr_pref,value_ptr,QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES);
      }
      else
      {
        QMI_ERR_MSG_1 ("qmi_wds_read_cdma_common_profile_tlvs: Invalid length received for type",(unsigned int) type);
      }
    }
    break;
    case QMI_WDS_CDMA_PROFILE_SEC_V6_DNS_ADDR_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      if (length == QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES)
      {
        profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_PARAM_MASK;
        memcpy(profile_params->secondary_dns_ipv6_addr_pref,value_ptr,QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES);
      }
      else
      {
        QMI_ERR_MSG_1 ("qmi_wds_read_cdma_common_profile_tlvs: Invalid length received for type",(unsigned int) type);
      }
    }
    break;
    case QMI_WDS_CDMA_PROFILE_RAT_TYPE_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_RAT_TYPE_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->rat_type = (qmi_wds_cdma_rat_type)tmp_char;
    }
    break;
    case QMI_WDS_PROFILE_IS_PERSISTENT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_IS_PERSISTENT_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->is_persistent = (qmi_wds_bool_type)tmp_char;
    }
    break;
    case QMI_WDS_CDMA_PROFILE_APN_ENABLED_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_APN_ENABLED_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->apn_disabled_flag = !((qmi_wds_bool_type)tmp_char);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_PDN_INACTIVITY_TIMEOUT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_PDN_INACTIVITY_TIMEOUT_PARAM_MASK;

      READ_32_BIT_VAL (value_ptr, profile_params->pdn_inactivity_timeout);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_APN_CLASS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_APN_CLASS_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr,  profile_params->apn_class);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr,  profile_params->pdn_level_auth_protocol);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_PDN_LEVEL_USER_ID_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_PDN_LEVEL_USER_ID_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
                      ? length : (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE - 1);
        memcpy (profile_params->pdn_level_user_id, (void *)value_ptr, cpy_len);
        profile_params->pdn_level_user_id[cpy_len] = '\0';
      }
      else
      {
        profile_params->pdn_level_user_id[0] = '\0';
      }
    }
    break;
    case QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PWD_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PWD_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
                      ? length : (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE - 1);
        memcpy (profile_params->password, (void *)value_ptr, cpy_len);
        profile_params->password[cpy_len] = '\0';
      }
      else
      {
        profile_params->password[0] = '\0';
      }
    }
    break;
    case QMI_WDS_CDMA_PROFILE_PDN_LABEL_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_PDN_LABEL_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_APN_STR_SIZE)
                      ? length : (QMI_WDS_MAX_APN_STR_SIZE - 1);
        memcpy (profile_params->pdn_label, (void *)value_ptr, cpy_len);
        profile_params->pdn_label[cpy_len] = '\0';
      }
      else
      {
        profile_params->pdn_label[0] = '\0';
      }
    }
    break;
    case QMI_WDS_CDMA_PROFILE_OPERATOR_RESERVED_PCO_ID_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_OPERATOR_RESERVED_PCO_ID_PARAM_MASK;

      READ_16_BIT_VAL (value_ptr,  profile_params->op_pco_id);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_MCC_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_MCC_PARAM_MASK;

      READ_16_BIT_VAL (value_ptr,  profile_params->pco_mcc);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_MNC_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_CDMA_PROFILE_MNC_PARAM_MASK;
      (void)qmi_wds_read_mnc_tlv (value_ptr,&profile_params->mnc);
    }
    break;
    default:
    {
      rc = FALSE;
    }
  }
  return rc;
}/*qmi_wds_read_cdma_common_profile_tlvs*/

/*===========================================================================
  FUNCTION  qmi_wds_read_epc_common_profile_tlvs
===========================================================================*/
/*!
@brief
  Takes an input RX buffer containing EPC profile TLV data
  and reads values into a profile paramaters structure.


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
qmi_wds_read_epc_common_profile_tlvs
(
  unsigned long                      type,
  unsigned long                      length,
  unsigned char                      *value_ptr,
  qmi_wds_epc_profile_params_type   *profile_params
)
{
  /* Declare/initialize return code */
  int rc = TRUE;
  unsigned char tmp_char;

  /* Now process TLV */

  switch (type)
  {
    case QMI_WDS_EPC_UMTS_PROFILE_NAME_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_PROFILE_NAME_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_PROFILE_STR_SIZE)
                      ? length : (QMI_WDS_MAX_PROFILE_STR_SIZE - 1);
        memcpy (profile_params->profile_name, (void *)value_ptr, cpy_len);
        profile_params->profile_name[cpy_len] = '\0';
      }
      else
      {
        profile_params->profile_name[0] = '\0';
      }
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_PDP_TYPE_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_PDP_TYPE_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pdp_type = (qmi_wds_pdp_type) tmp_char;
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_APN_NAME_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_PROFILE_APN_NAME_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_APN_STR_SIZE)
                      ? length : (QMI_WDS_MAX_APN_STR_SIZE - 1);
        memcpy (profile_params->apn_name, (void *)value_ptr, cpy_len);
        profile_params->apn_name[cpy_len] = '\0';
      }
      else
      {
        profile_params->apn_name[0] = '\0';
      }
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_PRIM_DNS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_PRIM_DNS_PARAM_MASK;
      READ_32_BIT_VAL (value_ptr, profile_params->primary_dns_pref_addr);
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_SEC_DNS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_SEC_DNS_PARAM_MASK;
      READ_32_BIT_VAL (value_ptr, profile_params->secondary_dns_pref_addr);
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_UMTS_REQ_QOS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_UMTS_REQ_QOS_PARAM_MASK;
        (void)qmi_wds_read_umts_qos_tlv (value_ptr,&profile_params->umts_requested_qos, FALSE);
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_UMTS_MIN_QOS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_UMTS_MIN_QOS_PARAM_MASK;
        (void)qmi_wds_read_umts_qos_tlv (value_ptr,&profile_params->umts_minimum_qos, FALSE);
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_GPRS_REQ_QOS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_GPRS_REQ_QOS_PARAM_MASK;
        (void)qmi_wds_read_gprs_qos_tlv (value_ptr,&profile_params->gprs_requested_qos);
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_GPRS_MIN_QOS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_GPRS_MIN_QOS_PARAM_MASK;
        (void)qmi_wds_read_gprs_qos_tlv (value_ptr,&profile_params->gprs_minimum_qos);
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_USERNAME_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_PROFILE_USERNAME_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
                      ? length : (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE - 1);
        memcpy (profile_params->username, (void *)value_ptr, cpy_len);
        profile_params->username[cpy_len] = '\0';
      }
      else
      {
        profile_params->username[0] = '\0';
      }
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_AUTH_PREF_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_AUTH_PREF_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->auth_pref = (qmi_wds_auth_pref_type) tmp_char;
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_IPV4_ADDR_PREF_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_IPV4_ADDR_PREF_PARAM_MASK;
      READ_32_BIT_VAL (value_ptr, profile_params->ipv4_pref_addr);
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_PCSCF_ADDR_VIA_PCO_TLV_ID:
      {
        /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_PCSCF_ADDR_VIA_PCO_PARAM_MASK;
        READ_8_BIT_VAL (value_ptr, tmp_char);
        profile_params->pcscf_addr_via_pco = (qmi_wds_pcscf_via_pco_type) tmp_char;
      }
      break;

    case QMI_WDS_EPC_UMTS_PROFILE_HEADER_COMPRESSION_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_HEADER_COMPRESSION_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->header_compression = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_DATA_COMPRESSION_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_DATA_COMPRESSION_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->data_compression = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_PDP_ACCESS_CONTROL_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_PDP_ACCESS_CONTROL_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pdp_access_control = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_PCSCF_VIA_DHCP_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_PCSCF_VIA_DHCP_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pcscf_via_dhcp = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_IM_CN_FLAG_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_IM_CN_FLAG_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->im_cn_flag = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_TFT_FILTER_ID_1_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_TFT_FILTER_ID_1_PARAM_MASK;
     (void)qmi_wds_read_tft_filter (value_ptr,&profile_params->tft_filter_id_1);
    }
    break;

     case QMI_WDS_EPC_UMTS_PROFILE_TFT_FILTER_ID_2_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_TFT_FILTER_ID_2_PARAM_MASK;
     (void)qmi_wds_read_tft_filter (value_ptr,&profile_params->tft_filter_id_2);
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_PDP_CONTEXT_NUMBER_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_PDP_CONTEXT_NUMBER_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pdp_context_number = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_PDP_CONTEXT_SECONDARY_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_PDP_CONTEXT_SECONDARY_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pdp_context_secondary = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_PDP_PRIMARY_ID_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_PDP_PRIMARY_ID_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pdp_context_primary_id_number = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_IPV6_ADDR_PREF_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      if (length == QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES )
      {
        profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_IPV6_ADDR_PREF_PARAM_MASK;
        memcpy(profile_params->ipv6_addr_pref, value_ptr, QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES);
      }
      else
      {
        QMI_ERR_MSG_1 ("qmi_wds_read_umts_common_profile_tlvs: Invalid length received for type: %u",(unsigned int) type);
      }
    }
    break;
    case QMI_WDS_EPC_UMTS_PROFILE_UMTS_REQ_QOS_EXT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_UMTS_REQ_QOS_EXT_PARAM_MASK;
      (void)qmi_wds_read_umts_qos_tlv (value_ptr,&profile_params->umts_requested_qos_sig_ind,TRUE);
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_UMTS_MIN_QOS_EXT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_UMTS_MIN_QOS_EXT_PARAM_MASK;
      (void)qmi_wds_read_umts_qos_tlv (value_ptr,&profile_params->umts_minimum_qos_sig_ind,TRUE);
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_PRIMARY_DNS_IPV6_ADDR_PREF_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      if (length == QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES )
      {
        profile_params->param_mask |= QMI_WDS_EPC_PROFILE_PRIMARY_DNS_IPV6_ADDR_PREF_PARAM_MASK;
        memcpy(profile_params->primary_dns_ipv6_addr_pref, value_ptr, QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES);
      }
      else
      {
        QMI_ERR_MSG_1 ("qmi_wds_read_umts_common_profile_tlvs: Invalid length received for type %u",(unsigned int) type);
      }
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      if (length == QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES )
      {
        profile_params->param_mask |= QMI_WDS_EPC_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_PARAM_MASK;
        memcpy(profile_params->secondary_dns_ipv6_addr_pref, value_ptr, QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES);
      }
      else
      {
        QMI_ERR_MSG_1 ("qmi_wds_read_umts_common_profile_tlvs: Invalid length received for type %u",(unsigned int) type);
      }
    }
    break;

    case QMI_WDS_EPC_UMTS_PROFILE_SUPPORT_EMERGENCY_CALLS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_UMTS_PROFILE_SUPPORT_EMERGENCY_CALLS_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->support_emergency_calls = (qmi_wds_bool_type)tmp_char;
    }
    break;

    case QMI_WDS_EPC_CDMA_PROFILE_DNS_SERVER_PREF_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_DNS_SERVER_PREF_PARAM_MASK;

       READ_8_BIT_VAL (value_ptr, tmp_char);
       profile_params->dns_server_pref = (qmi_wds_bool_type) tmp_char;
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_DO_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_DO_PARAM_MASK;

      READ_32_BIT_VAL (value_ptr, profile_params->ppp_session_close_timer_do);
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_1X_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_PPP_SESSION_CLOSE_TIMER_1X_PARAM_MASK;

      READ_32_BIT_VAL (value_ptr, profile_params->ppp_session_close_timer_1x);
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_ALLOW_LINGER_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_ALLOW_LINGER_PARAM_MASK;

      READ_8_BIT_VAL (value_ptr,  profile_params->allow_linger);
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_LCP_ACK_TIMEOUT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_LCP_ACK_TIMEOUT_PARAM_MASK;

      READ_16_BIT_VAL (value_ptr,  profile_params->lcp_ack_timeout);
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_IPCP_ACK_TIMEOUT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_IPCP_ACK_TIMEOUT_PARAM_MASK;

      READ_16_BIT_VAL (value_ptr,  profile_params->ipcp_ack_timeout);
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_AUTH_TIMEOUT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_AUTH_TIMEOUT_PARAM_MASK;

      READ_16_BIT_VAL (value_ptr,  profile_params->auth_timeout);
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_LCP_CONFIG_RETRY_RECOUNT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_LCP_CONFIG_RETRY_RECOUNT_PARAM_MASK;

      READ_8_BIT_VAL (value_ptr,  profile_params->lcp_config_req_retry_count);
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_IPCP_CONFIG_RETRY_RECOUNT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_IPCP_CONFIG_RETRY_RECOUNT_PARAM_MASK;

      READ_8_BIT_VAL (value_ptr,  profile_params->ipcp_config_req_retry_count);
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_AUTH_RETRY_RECOUNT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_AUTH_RETRY_RECOUNT_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr,  profile_params->auth_retry_count);
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_USERNAME_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_PROFILE_USERNAME_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
                      ? length : (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE - 1);
        memcpy (profile_params->cdma_username, (void *)value_ptr, cpy_len);
        profile_params->cdma_username[cpy_len] = '\0';
      }
      else
      {
        profile_params->cdma_username[0] = '\0';
      }
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_PASSWORD_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_PROFILE_PASSWORD_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
                      ? length : (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE - 1);
        memcpy (profile_params->cdma_password, (void *)value_ptr, cpy_len);
        profile_params->cdma_password[cpy_len] = '\0';
      }
      else
      {
        profile_params->cdma_password[0] = '\0';
      }
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_AUTH_PROTOCOL_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_AUTH_PROTOCOL_PARAM_MASK;

      READ_8_BIT_VAL (value_ptr,  profile_params->auth_protocol);
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_DATA_RATE_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_DATA_RATE_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr,  tmp_char);
      profile_params->data_rate = (qmi_wds_cdma_data_rate_type) tmp_char;
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_DATA_MODE_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_DATA_MODE_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr,  tmp_char);
      profile_params->data_mode = (qmi_wds_cdma_data_mode_type) tmp_char;
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_APP_TYPE_TLV_ID:
    {
      int tmp_word;
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_APP_TYPE_PARAM_MASK;
      READ_32_BIT_VAL (value_ptr,  tmp_word);
      profile_params->app_type = (qmi_wds_cdma_app_type) tmp_word;
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_APP_PRIORITY_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_APP_PRIORITY_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr,  profile_params->app_priority);
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_APN_STRING_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_PROFILE_APN_NAME_PARAM_MASK;
      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_APN_STR_SIZE)
                      ? length : (QMI_WDS_MAX_APN_STR_SIZE - 1);
        memcpy (profile_params->cdma_apn_string, (void *)value_ptr, cpy_len);
        profile_params->cdma_apn_string[cpy_len] = '\0';
      }
      else
      {
        profile_params->cdma_apn_string[0] = '\0';
      }
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_PDN_TYPE_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_PDN_TYPE_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->pdn_type = (qmi_wds_cdma_pdn_type)tmp_char;
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_IS_PCSCF_ADDR_NEEDED_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_IS_PCSCF_ADDR_NEEDED_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->is_pcscf_addr_needed = (qmi_wds_bool_type)tmp_char;
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_PRIM_V4_DNS_ADDR_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_PRIM_V4_DNS_ADDR_PARAM_MASK;
      READ_32_BIT_VAL (value_ptr, profile_params->primary_dns_ipv4_pref_addr);
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_SEC_V4_DNS_ADDR_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_SEC_V4_DNS_ADDR_PARAM_MASK;
      READ_32_BIT_VAL (value_ptr, profile_params->secondary_dns_ipv4_pref_addr);
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_PRIM_V6_DNS_ADDR_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      if (length == QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES)
      {
        profile_params->param_mask |= QMI_WDS_EPC_PROFILE_PRIMARY_DNS_IPV6_ADDR_PREF_PARAM_MASK;
        memcpy(profile_params->primary_dns_ipv6_addr_pref,value_ptr,QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES);
      }
      else
      {
        QMI_ERR_MSG_1 ("qmi_wds_read_cdma_common_profile_tlvs: Invalid length received for type",(unsigned int) type);
      }
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_SEC_V6_DNS_ADDR_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      if (length == QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES)
      {
        profile_params->param_mask |= QMI_WDS_UMTS_PROFILE_SECONDARY_DNS_IPV6_ADDR_PREF_PARAM_MASK;
        memcpy(profile_params->secondary_dns_ipv6_addr_pref,value_ptr,QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES);
      }
      else
      {
        QMI_ERR_MSG_1 ("qmi_wds_read_cdma_common_profile_tlvs: Invalid length received for type",(unsigned int) type);
      }
    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_RAT_TYPE_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_CDMA_PROFILE_RAT_TYPE_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->rat_type = (qmi_wds_cdma_rat_type)tmp_char;
    }
    break;
    case QMI_WDS_PROFILE_IS_PERSISTENT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_PROFILE_IS_PERSISTENT_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->is_persistent = (qmi_wds_bool_type)tmp_char;
    }
    break;
    /* common parameter */
    case QMI_WDS_EPC_COMMON_USER_ID_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_COMMON_USER_ID_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
                      ? length : (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE - 1);
        memcpy (profile_params->common_username, (void *)value_ptr, cpy_len);
        profile_params->common_username[cpy_len] = '\0';
      }
      else
      {
        profile_params->common_username[0] = '\0';
      }
    }
    break;
    case QMI_WDS_EPC_COMMON_AUTH_PROTOCOL_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_COMMON_AUTH_PROTOCOL_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->common_auth_pref = (qmi_wds_auth_pref_type) tmp_char;
    }
    break;
    case QMI_WDS_EPC_COMMON_AUTH_PASSWORD_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_COMMON_AUTH_PASSWORD_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
                      ? length : (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE - 1);
        memcpy (profile_params->common_password, (void *)value_ptr, cpy_len);
        profile_params->common_password[cpy_len] = '\0';
      }
      else
      {
        profile_params->common_password[0] = '\0';
      }
    }
    break;
    case QMI_WDS_EPC_COMMON_APN_CLASS_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_COMMON_APN_CLASS_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->common_apn_class = (unsigned char) tmp_char;
    }
    break;
    case QMI_WDS_EPC_COMMON_APN_DISABLED_FLAG_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_COMMON_APN_DISABLED_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr, tmp_char);
      profile_params->common_apn_disabled_flag = (unsigned char) tmp_char;
    }
    break;

    case QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_TLV_ID:
    {
      /* PDN level Auth protocol TLV is same for EPC and 3GPP2 profile */
      profile_params->extended_param_mask |=
         QMI_WDS_EPC_PROFILE_PDN_LEVEL_AUTH_PROTOCOL_PARAM_MASK;
      profile_params->param_mask |= QMI_WDS_EPC_EXTENDED_PARAM_MASK;
      READ_8_BIT_VAL (value_ptr,  profile_params->pdn_level_auth_protocol);
    }
    break;
    case QMI_WDS_CDMA_PROFILE_PDN_LEVEL_USER_ID_TLV_ID:
    {
      /* PDN level USER ID TLV is same for EPC and 3GPP2 profile */
      profile_params->extended_param_mask |=
         QMI_WDS_EPC_PROFILE_PDN_LEVEL_USER_ID_PARAM_MASK;
      profile_params->param_mask |= QMI_WDS_EPC_EXTENDED_PARAM_MASK;

      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
                      ? length : (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE - 1);
        memcpy (profile_params->pdn_level_user_id, (void *)value_ptr, cpy_len);
        profile_params->pdn_level_user_id[cpy_len] = '\0';
      }
      else
      {
        profile_params->pdn_level_user_id[0] = '\0';
      }
    }
    break;
    case QMI_WDS_CDMA_PROFILE_PDN_LEVEL_AUTH_PWD_TLV_ID:
    {
      /* PDN level Passwd TLV is same for EPC and 3GPP2 profile */
      profile_params->extended_param_mask |= QMI_WDS_EPC_PROFILE_PDN_LEVEL_AUTH_PWD_PARAM_MASK;
      profile_params->param_mask |= QMI_WDS_EPC_EXTENDED_PARAM_MASK;
      /* String starts at value pointer and is length bytes long */
      if (length > 0)
      {
        int cpy_len = (length < QMI_WDS_MAX_USERNAME_PASS_STR_SIZE)
                      ? length : (QMI_WDS_MAX_USERNAME_PASS_STR_SIZE - 1);
        memcpy (profile_params->password, (void *)value_ptr, cpy_len);
        profile_params->password[cpy_len] = '\0';
      }
      else
      {
        profile_params->password[0] = '\0';
      }
    }
    break;
    case QMI_WDS_UMTS_PROFILE_PDN_INACTIVITY_TIMEOUT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_EXTENDED_PARAM_MASK;
      profile_params->extended_param_mask |= QMI_WDS_EPC_PROFILE_PDN_LEVEL_INACTIVITY_TIMEOUT_MASK;
      READ_32_BIT_VAL (value_ptr, profile_params->pdn_inactivity_timeout);

    }
    break;
    case QMI_WDS_EPC_CDMA_PROFILE_PDN_INACTIVITY_TIMEOUT_TLV_ID:
    {
      /* Set corresponding bit in optional param mask */
      profile_params->param_mask |= QMI_WDS_EPC_EXTENDED_PARAM_MASK;
      profile_params->extended_param_mask |= QMI_WDS_EPC_PROFILE_PDN_LEVEL_INACTIVITY_TIMEOUT_3GPP2_MASK;
      READ_32_BIT_VAL (value_ptr, profile_params->pdn_inactivity_timeout_3gpp2);

    }
    break;
    default:
    {
      rc = FALSE;
    }
  }

  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_read_query_profile_tlvs
===========================================================================*/
/*!
@brief
  Takes an input RX buffer containing UMTS profile TLV data
  and reads values into a profile paramaters structure.


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
qmi_wds_read_query_profile_tlvs
(
  unsigned char               *rx_buf,
  int                         rx_buf_len,
  qmi_wds_profile_id_type     *profile_id,
  qmi_wds_profile_params_type *profile_params
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  profile_id->valid_data_read = FALSE;

  memset(profile_params, 0, sizeof(*profile_params));

  /* Initialize UMTS optional param mask to 0 */
  profile_params->umts_profile_params.param_mask = 0;
  /* Initialize CDMA optional param mask to 0 */
  profile_params->cdma_profile_params.param_mask = 0;
  /* Initialize EPC optional param mask to 0 */
  profile_params->epc_profile_params.param_mask = 0;

  /* Loop through all TLV's aqmi_wds_curr_call_info_typend process each one */
  while (rx_buf_len > 0)
  {

    if ( qmi_util_read_std_tlv (&rx_buf,
                                  &rx_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Process message specific TLV's and then the common ones */
    switch (type)
    {
      case QMI_WDS_PROFILE_ID_TLV_ID:
      {
        (void)qmi_wds_read_profile_id_tlv (value_ptr,
                                           length,
                                           profile_id,
                                           FALSE);
      }
      break;

      default:
      {
        /* If the TLV is not one of message specific, try to get a common one */
        if (profile_id->technology == QMI_WDS_PROFILE_TECH_3GPP)
        {
          if (!(qmi_wds_read_umts_common_profile_tlvs (type,length,value_ptr,&profile_params->umts_profile_params)))
          {
            QMI_ERR_MSG_1 ("qmi_wds_read_umts_common_profile_tlvs: ignoring unknown TLV type=%x\n",(unsigned int) type);
          }
        }
        else if (profile_id->technology == QMI_WDS_PROFILE_TECH_3GPP2)
        {
          if (!(qmi_wds_read_cdma_common_profile_tlvs (type,length,value_ptr,&profile_params->cdma_profile_params)))
          {
            QMI_ERR_MSG_1 ("qmi_wds_read_cdma_common_profile_tlvs: ignoring unknown TLV type=%x\n",(unsigned int) type);
          }
        }
        else if (profile_id->technology == QMI_WDS_PROFILE_TECH_EPC)
        {
          if (!(qmi_wds_read_epc_common_profile_tlvs (type,length,value_ptr,&profile_params->epc_profile_params)))
          {
            QMI_ERR_MSG_1 ("qmi_wds_read_epc_common_profile_tlvs: ignoring unknown TLV type=%x\n",(unsigned int) type);
          }
        }
        else
        {
          QMI_ERR_MSG_0 ("qmi_wds_read_query_profile_tlvs: Invalid Tech type received");
          return QMI_INTERNAL_ERR;
        }
      }
      break;
    } /* switch */
  } /* while */

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_read_curr_call_tlvs
===========================================================================*/
/*!
@brief
  Takes an input RX buffer current call (GET RUNTIME SETTINGS) TLV info
  and translates to qmi_wds_curr_call_info_type C structure


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
qmi_wds_read_curr_call_tlvs
(
  unsigned char               *rx_buf,
  int                         rx_buf_len,
  qmi_wds_profile_id_type     *profile_id,
  qmi_wds_profile_params_type *profile_params,
  qmi_wds_curr_call_info_type  *curr_call_info
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;



  /* Initialize profile_id to not read */
  profile_id->valid_data_read = FALSE;

  memset(curr_call_info, 0, sizeof(*curr_call_info));

  /* Initialize call setting mask */
  curr_call_info->mask = 0;


  /* Loop through all TLV's aqmi_wds_curr_call_info_typend process each one */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                                  &rx_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Process query-specific TLV's */
    switch (type)
    {
      case QMI_WDS_CURR_CALL_PROFILE_ID_TLV_ID:
      {
        (void)qmi_wds_read_profile_id_tlv (value_ptr,
                                           length,
                                           profile_id,
                                           FALSE);
      }
      break;

      case QMI_WDS_CURR_CALL_IPV4_GATEWAY_ADDR_TLV_ID:
      {
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_IPV4_GATEWAY_ADDR;
        READ_32_BIT_VAL (value_ptr, curr_call_info->ipv4_gateway_addr);
      }
      break;

      case QMI_WDS_CURR_CALL_IPV4_SUBNET_MASK_TLV_ID:
      {
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_SUBNET_MASK;
        READ_32_BIT_VAL (value_ptr, curr_call_info->ipv4_subnet_mask);
      }
      break;

      case QMI_WDS_CURR_CALL_PCSCF_ADDR_PCO_TLV_ID:
      {
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_PCSCF_ADDR_USNG_PCO;
        READ_8_BIT_VAL (value_ptr, curr_call_info->p_cscf_addr_using_pco);
      }
      break;

      case QMI_WDS_CURR_CALL_PCSCF_SERVER_ADDR_LIST_TLV_ID:
      {
        int index = 0;
        int num_instances = 0;
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_PCSCF_IPV4_ADDR_LIST;
        READ_8_BIT_VAL (value_ptr, curr_call_info->p_cscf_ipv4_addrs.num_instances);
        num_instances = curr_call_info->p_cscf_ipv4_addrs.num_instances;

        if (num_instances > QMI_WDS_MAX_P_CSCF_IPV4_ADDRS)
          return QMI_INTERNAL_ERR;
        for (index = 0; index < num_instances; index++)
        {
          READ_32_BIT_VAL (value_ptr, curr_call_info->p_cscf_ipv4_addrs.p_cscf_ipv4_addr[index]);
        }
      }
      break;

      case QMI_WDS_CURR_CALL_PCSCF_IPV6_SERVER_ADDR_LIST_TLV_ID:
      {
        int index = 0;
        int num_instances = 0;
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_PCSCF_IPV6_ADDR_LIST;
        READ_8_BIT_VAL (value_ptr, curr_call_info->p_cscf_ipv6_addrs.num_instances);
        num_instances = curr_call_info->p_cscf_ipv6_addrs.num_instances;

        if (num_instances > QMI_WDS_MAX_P_CSCF_IPV6_ADDRS)
          return QMI_INTERNAL_ERR;
        for (index = 0; index < num_instances; index++)
        {
          memcpy(curr_call_info->p_cscf_ipv6_addrs.p_cscf_ipv6_addr[index],
                 value_ptr,
                 QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES);
          value_ptr += QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES;

        }
      }
      break;

      case QMI_WDS_CURR_CALL_PCSCF_FQDN_LIST_TLV_ID:
      {
        int index = 0;
        int string_len = 0;
        int num_instances = 0;
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_PCSCF_FQDN_LIST;
        READ_8_BIT_VAL (value_ptr, curr_call_info->fqdn_list.num_instances);
        num_instances = curr_call_info->fqdn_list.num_instances;

        if (num_instances > QMI_WDS_MAX_FQDN_STRINGS)
          return QMI_INTERNAL_ERR;

        for (index = 0; index < num_instances; index++)
        {
          READ_16_BIT_VAL (value_ptr, curr_call_info->fqdn_list.fqdn_strings[index].fqdn_length);
          string_len = curr_call_info->fqdn_list.fqdn_strings[index].fqdn_length;
          if (string_len < 0 || string_len > QMI_WDS_MAX_PROFILE_STR_SIZE)
            return QMI_INTERNAL_ERR;
          memcpy ((void*)curr_call_info->fqdn_list.fqdn_strings[index].fqdn_string,(void *)value_ptr, (size_t)string_len);
        }
        value_ptr += string_len;
      }
      break;

      case QMI_WDS_CURR_CALL_IPV6_ADDR_TLV_ID:
      {
        int index;

        if (length != (QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES + 1))
        {
          QMI_ERR_MSG_1 ("qmi_wds_read_curr_call_tlvs: Got bad IPV6 addr info length=%d, ignoring", length);
          continue;
        }

        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_IPV6_ADDR;

        for (index = 0; index < QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES; index++)
        {
          READ_8_BIT_VAL (value_ptr, curr_call_info->ipv6_addr_info.ipv6_addr[index]);
        }

        /* Read V6 prefix length */
        READ_8_BIT_VAL (value_ptr, curr_call_info->ipv6_addr_info.ipv6_prefix_len);
      }
      break;

      case QMI_WDS_CURR_CALL_IPV6_GATEWAY_ADDR_TLV_ID:
      {
        int index;
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_IPV6_GTWY_ADDR;
        for (index = 0; index < QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES; index++)
        {
          READ_8_BIT_VAL (value_ptr, curr_call_info->ipv6_gateway_addr_info.ipv6_addr[index]);
        }

        /* Read V6 prefix length */
        READ_8_BIT_VAL (value_ptr, curr_call_info->ipv6_gateway_addr_info.ipv6_prefix_len);
      }
      break;

      case QMI_WDS_CURR_CALL_PRIMARY_DNS_IPV6_ADDR_TLV_ID:
      {
        int index;
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_PRIMARY_DNS_IPV6_ADDR;
        for (index = 0; index < QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES; index++)
        {
          READ_8_BIT_VAL (value_ptr, curr_call_info->primary_dns_ipv6_addr[index]);
        }
      }
      break;

      case QMI_WDS_CURR_CALL_SECONDARY_DNS_IPV6_ADDR_TLV_ID:
      {
        int index;
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_SECONDARY_DNS_IPV6_ADDR;
        for (index = 0; index < QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES; index++)
        {
          READ_8_BIT_VAL (value_ptr, curr_call_info->secondary_dns_ipv6_addr[index]);
        }
      }
      break;

      case QMI_WDS_CURR_CALL_MTU_TLV_ID:
      {
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_MTU;
        READ_32_BIT_VAL (value_ptr, curr_call_info->mtu);
      }
      break;

      case QMI_WDS_CURR_DOMAIN_NAME_LIST_TLV_ID:
      {
        int index = 0;
        int string_len = 0;
        int num_instances = 0;
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_DOMAIN_NAME_LIST;

        READ_8_BIT_VAL (value_ptr, curr_call_info->domain_name_list.num_instances);
        num_instances = curr_call_info->domain_name_list.num_instances;

        if (num_instances < 0 || num_instances > QMI_WDS_MAX_DOMAIN_NAMES)
          return QMI_INTERNAL_ERR;
        for (index = 0; index < num_instances; index++)
        {
          READ_16_BIT_VAL (value_ptr, curr_call_info->domain_name_list.domain_names[index].domain_name_len);
          string_len = curr_call_info->domain_name_list.domain_names[index].domain_name_len;
          if (string_len <= 0 || string_len > QMI_WDS_MAX_PROFILE_STR_SIZE)
            return QMI_INTERNAL_ERR;
          memcpy ((void *)curr_call_info->domain_name_list.domain_names[index].domain_name, (void *)value_ptr, (size_t)string_len);
        }
      }
      break;

      case QMI_WDS_CURR_IP_FAMILY_TLV_ID:
      {
        unsigned char temp;
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_IP_FAMILY;
        READ_8_BIT_VAL (value_ptr,temp);
        curr_call_info->ip_family = (qmi_wds_ip_family)temp;
      }
      break;

      case QMI_WDS_CURR_IM_CN_FLAG_TLV_ID:
      {
        unsigned char temp;
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_IM_CN_FLAG;
        READ_8_BIT_VAL (value_ptr, temp);
        curr_call_info->im_cn_flag = (qmi_wds_cn_flag_true_false) temp;
      }
      break;

      case QMI_WDS_CURR_TECHNOLOGY_TLV_ID:
      {
        int temp;
        /* Set corresponding bit in mask */
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_TECHNOLOGY;
        READ_16_BIT_VAL (value_ptr, temp);
        curr_call_info->curr_technology = (qmi_wds_iface_name_type) temp;
      }
      break;

      case QMI_WDS_CURR_CALL_IPV4_ADDR_TLV_ID:
      {
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_IPV4_ADDR;
        READ_32_BIT_VAL (value_ptr, curr_call_info->ipv4_addr);
      }
      break;

      case QMI_WDS_CURR_CALL_PRIMARY_DNS_IPV4_ADDR_TLV_ID:
      {
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_PRIMARY_DNS_IPV4_ADDR;
        READ_32_BIT_VAL (value_ptr, curr_call_info->primary_dns_ipv4_addr);
      }
      break;

      case QMI_WDS_CURR_CALL_SECONDARY_DNS_IPV4_ADDR_TLV_ID:
      {
        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_SECONDARY_DNS_IPV4_ADDR;
        READ_32_BIT_VAL (value_ptr, curr_call_info->secondary_dns_ipv4_addr);
      }
      break;

      case QMI_WDS_CURR_CALL_OPERATOR_RESERVED_PCO_TLV_ID:
      {
        unsigned int i;

        curr_call_info->mask |= QMI_WDS_CURR_CALL_INFO_OPERATOR_RESERVED_PCO;

        READ_16_BIT_VAL(value_ptr, curr_call_info->operator_pco.mcc);
        READ_16_BIT_VAL(value_ptr, curr_call_info->operator_pco.mnc);

        READ_8_BIT_VAL(value_ptr, curr_call_info->operator_pco.mnc_includes_pcs_digit);

        READ_8_BIT_VAL(value_ptr, curr_call_info->operator_pco.app_specific_info_len);

        for (i = 0; i < curr_call_info->operator_pco.app_specific_info_len; ++i)
        {
          READ_8_BIT_VAL(value_ptr, curr_call_info->operator_pco.app_specific_info[i]);
        }

        READ_16_BIT_VAL(value_ptr, curr_call_info->operator_pco.container_id);
      }
      break;

      default:
      {
        /*Only UMTS profile read for Now*/
        /* Initialize optional param mask to 0 */
        profile_params->umts_profile_params.param_mask = 0;
        /* If the TLV is not one of message specific, try to get a common one */
        if (!(qmi_wds_read_umts_common_profile_tlvs (type,length,value_ptr,&profile_params->umts_profile_params)))
        {
          QMI_ERR_MSG_1 ("qmi_wds_read_curr_call_tlvs: ignoring unknown TLV type=%x\n",(unsigned int) type);
        }
      }
      break;
    } /* switch */
  }/* while */

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_wds_create_profile
===========================================================================*/
/*!
@brief
  Takes as input a user client handle, WDS profile parameters and a WDS
  profile ID structure and creates a QMI profile (on modem processor) based
  on these parameters.  The input profile ID MUST have the 'technology'
  field set to a valid value.

  Note that each of the 'profile_params' structure values are optional.
  To indicate that a particular structure field is valid in the input
  parameters, the corresponding bit should be set in the 'param_mask'
  field of the structure.  For example if the 'secondary_dns_pref_addr'
  field has been filled in and is valid, the QMI_WDS_PROFILE_PARAM_SEC_DNS
  bit should be set in the 'param_mask'

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not.  Upon successful
  return, the profile ID parameter will have the 'profile_index' field set to
  the profile index of the newly created profile.
  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.
@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - Creates a new profile on the modem processor.
*/
/*=========================================================================*/
int
qmi_wds_create_profile
(
  int                         user_handle,
  qmi_wds_profile_id_type     *profile_id,
  qmi_wds_profile_params_type *profile_params,
  int                         *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Write profile ID technology type to TLV buffer (mandatory) */
  if ((!profile_id) ||
      (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                  &msg_size,
                                  QMI_WDS_PROFILE_ID_TLV_ID,
                                  1,
                                  (void *)&profile_id->technology) < 0))
  {
    return QMI_INTERNAL_ERR;
  }

  /* Write profile params (optional) to TLV buffer */
  if (profile_params)
  {
    if (profile_id->technology == QMI_WDS_PROFILE_TECH_3GPP)
    {
      if (qmi_wds_write_umts_optional_profile_tlvs (&tmp_msg_ptr,
                                                    &msg_size,
                                                    &profile_params->umts_profile_params) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }
    else if (profile_id->technology == QMI_WDS_PROFILE_TECH_3GPP2)
    {
      if (qmi_wds_write_cdma_optional_profile_tlvs (&tmp_msg_ptr,
                                                    &msg_size,
                                                    &profile_params->cdma_profile_params) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }
    else if (profile_id->technology == QMI_WDS_PROFILE_TECH_EPC)
    {
      if (qmi_wds_write_epc_optional_profile_tlvs (&tmp_msg_ptr,
                                                    &msg_size,
                                                    &profile_params->epc_profile_params) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }
    else
    {
      QMI_ERR_MSG_1("qmi_wds_create_profile: tech type %x not supported for qmi_wds_create_profile",profile_id->technology);
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
    /* Do this to keep compiler happy */
    unsigned long ulong_len = (unsigned long) msg_size;
    rc = qmi_wds_read_profile_id_tlv (tmp_msg_ptr,ulong_len,profile_id,TRUE);
  }

  /* Retrieve the extended error code if it exists */
  else if (rc == QMI_SERVICE_ERR && *qmi_err_code == QMI_SERVICE_ERR_EXTENDED_INTERNAL)
  {
    rc = QMI_EXTENDED_ERR;
    if ( qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                        &msg_size,
                                        qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_create_profile: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }
  return rc;
}  /* qmi_wds_create_profile */


/*===========================================================================
  FUNCTION  qmi_wds_modify_profile
===========================================================================*/
/*!
@brief
  Takes as input a user client handle, WDS profile parameters and a WDS
  profile ID structure and modifies parameters of an existing profile
  on the modem processor. The input profile ID MUST have the 'technology'
  field and 'profile_index' set to an existing valid profile index value.

  Note that each of the 'profile_params' structure values are optional.
  To indicate that a particular structure field is valid in the input
  parameters, the corresponding bit should be set in the 'param_mask'
  field of the structure.  For example if the 'secondary_dns_pref_addr'
  field has been filled in and is valid, the QMI_WDS_PROFILE_PARAM_SEC_DNS
  bit should be set in the 'param_mask'

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not.
  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.
@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - Creates a new profile on the modem processor.
*/
/*=========================================================================*/
int
qmi_wds_modify_profile
(
  int                         user_handle,
  qmi_wds_profile_id_type     *profile_id,
  qmi_wds_profile_params_type *profile_params,
  int                         *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Write profile ID data (mandatory) to TLV buffer */
  if ((!profile_id) ||
      (qmi_wds_write_profile_id_tlv (&tmp_msg_ptr,
                                          &msg_size,
                                          profile_id) < 0))
  {
    return QMI_INTERNAL_ERR;
  }

  /* Write profile params (optional) to TLV buffer */
  if (profile_params)
  {
    if (profile_id->technology == QMI_WDS_PROFILE_TECH_3GPP)
    {
      if (qmi_wds_write_umts_optional_profile_tlvs (&tmp_msg_ptr,
                                                    &msg_size,
                                                    &profile_params->umts_profile_params) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }
    else if (profile_id->technology == QMI_WDS_PROFILE_TECH_3GPP2)
    {
      if (qmi_wds_write_cdma_optional_profile_tlvs (&tmp_msg_ptr,
                                                    &msg_size,
                                                    &profile_params->cdma_profile_params) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }
    else if (profile_id->technology == QMI_WDS_PROFILE_TECH_EPC)
    {
      if (qmi_wds_write_epc_optional_profile_tlvs (&tmp_msg_ptr,
                                                    &msg_size,
                                                    &profile_params->epc_profile_params) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }
    else
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
    if (qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                       &msg_size,
                                       qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_modify_profile: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_query_profile
===========================================================================*/
/*!
@brief
  Takes as input a user client handle, and a WDS
  profile ID structure and queries the parameters of an existing profile
  on the modem processor. The input profile ID MUST have the 'technology'
  field and 'profile_index' set to an existing valid profile index value.
  The current values of the parameters will be returned in the
  profile_params structure.

  Note that each of the 'profile_params' structure values are optional.
  To indicate that a particular structure field is valid in the output
  parameters, the corresponding bit will be set in the 'param_mask'
  field of the structure.  For example if the 'secondary_dns_pref_addr'
  field has been filled in and is valid, the QMI_WDS_PROFILE_PARAM_SEC_DNS
  bit will be set in the 'param_mask'

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not.

  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.
@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - Creates a new profile on the modem processor.
*/
/*=========================================================================*/
int
qmi_wds_query_profile
(
  int                         user_handle,
  qmi_wds_profile_id_type     *profile_id,
  qmi_wds_profile_params_type *profile_params,
  int                         *qmi_err_code
)
{
  unsigned char     msg[2*QMI_WDS_STD_MSG_SIZE];/* increased the expected message size due to reply msg size is >512 */
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* TMR: temporary */
  memset (msg,0,2*QMI_WDS_STD_MSG_SIZE);

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE((2*QMI_WDS_STD_MSG_SIZE));

  /* Write profile ID data (mandatory) to TLV buffer */
  if ((!profile_id) ||
      (qmi_wds_write_profile_id_tlv (&tmp_msg_ptr,
                                      &msg_size,
                                      profile_id) < 0))
  {
    return QMI_INTERNAL_ERR;
  }

  /* Synchronously send message to modem processor */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_PROFILE_DATA_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE((2*QMI_WDS_STD_MSG_SIZE)) - msg_size,
                                  msg,
                                  &msg_size,
                                  2*QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;
  /* Get profile settings information */
  if (rc == QMI_NO_ERR)
  {
    rc = qmi_wds_read_query_profile_tlvs (tmp_msg_ptr,msg_size,profile_id,profile_params);
  }

  /* Retrieve the extended error code if it exists */
  else if (rc == QMI_SERVICE_ERR && *qmi_err_code == QMI_SERVICE_ERR_EXTENDED_INTERNAL)
  {
    rc = QMI_EXTENDED_ERR;
    if (qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                       &msg_size,
                                       qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_query_profile: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_delete_profile
===========================================================================*/
/*!
@brief
  Takes as input a user client handle, and a WDS
  profile ID structure and deletes an existing profile
  on the modem processor. The input profile ID MUST have the 'technology'
  field and 'profile_index' set to an existing valid profile index value.


@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not.
  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.
@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - Creates a new profile on the modem processor.
*/
/*=========================================================================*/
int
qmi_wds_delete_profile
(
  int                         user_handle,
  qmi_wds_profile_id_type     *profile_id,
  int                         *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Write profile ID data (mandatory) to TLV buffer */
  if ((!profile_id) ||
      (qmi_wds_write_profile_id_tlv (&tmp_msg_ptr,
                                          &msg_size,
                                          profile_id) < 0))
  {
    return QMI_INTERNAL_ERR;
  }
  /* Synchronously send message to modem processor */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_DELETE_PROFILE_MSG_ID,
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
    if (qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                       &msg_size,
                                       qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_delete_profile: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }

  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_get_default_settings
===========================================================================*/
/*!
@brief
  Takes as input a user client handle, and a WDS
  profile ID structure and queries the parameters of the default profile
  modem processor. The input profile_id field MUST have the 'technology'
  field set to a valid profile technology type value before function is called.
  The current values of the parameters will be returned in the
  profile_params structure.

  Note that each of the 'profile_params' structure values are optional.
  To indicate that a particular structure field is valid in the output
  parameters, the corresponding bit will be set in the 'param_mask'
  field of the structure.  For example if the 'secondary_dns_pref_addr'
  field has been filled in and is valid, the QMI_WDS_PROFILE_PARAM_SEC_DNS
  bit will be set in the 'param_mask'

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    None.
*/
/*=========================================================================*/
int
qmi_wds_get_default_settings
(
  int                         user_handle,
  qmi_wds_profile_tech_type   profile_tech,
  qmi_wds_profile_params_type *profile_params,
  int                         *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  qmi_wds_profile_id_type dummy;
  int rc;
  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Write profile tech type to TLV buffer */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_WDS_GET_DEFAULT_SETTINGS_TECH_TLV_ID,
                               1,
                               (void *)&profile_tech) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Synchronously send message to modem processor */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_DEFAULT_SETTINGS_MSG_ID,
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
    rc = qmi_wds_read_query_profile_tlvs (tmp_msg_ptr,msg_size,&dummy,profile_params);
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_set_event_report
===========================================================================*/
/*!
@brief
  Takes as input a user client handle and an event reporting parameters
  structure and registers for the specified events.  The events will
  be reported as asynchronous indications


@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - Creates a new profile on the modem processor.
*/
/*=========================================================================*/
int
qmi_wds_set_event_report
(
  int                               user_handle,
  qmi_wds_event_report_params_type  *event_params,
  int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  unsigned char     tmp_tlv_buf [5];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;



  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Write report channel rate TLV if appropriate */
  if (event_params->param_mask & QMI_WDS_EVENT_CHAN_RATE_IND)
  {
    unsigned char *val_ptr = tmp_tlv_buf;
    /* Set TLV based on parameter value */
    WRITE_8_BIT_VAL (val_ptr, (int)((event_params->report_chan_rate) ? TRUE : FALSE));
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_EVENT_RPT_CHAN_RATE_REQ_TLV_ID,
                                   1,
                                   (void *)tmp_tlv_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Write report transfer stats TLV if appropriate */
  if (event_params->param_mask & QMI_WDS_EVENT_XFER_STATS_IND)
  {
    unsigned char *val_ptr = tmp_tlv_buf;
    WRITE_8_BIT_VAL (val_ptr, event_params->report_xfer_stats.period);
    WRITE_32_BIT_VAL (val_ptr, event_params->report_xfer_stats.mask);
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_EVENT_RPT_XFER_STATS_REQ_TLV_ID,
                                   5,
                                   (void *)tmp_tlv_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Write report bearer technology TLV if appropriate */
  if (event_params->param_mask & QMI_WDS_EVENT_BEARER_TECH_IND)
  {
    unsigned char *val_ptr = tmp_tlv_buf;
    WRITE_8_BIT_VAL (val_ptr, (int)((event_params->report_bearer_tech) ? TRUE : FALSE));
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_EVENT_RPT_BEARER_TECH_REQ_TLV_ID,
                                   1,
                                   (void *)tmp_tlv_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Write report bearer technology extended TLV if appropriate */
  if (event_params->param_mask & QMI_WDS_EVENT_BEARER_TECH_EX_IND)
  {
    unsigned char *val_ptr = tmp_tlv_buf;
    WRITE_8_BIT_VAL (val_ptr, (int)((event_params->report_bearer_tech_ex ) ? TRUE : FALSE));
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_EVENT_RPT_BEARER_TECH_EX_REQ_TLV_ID,
                                   1,
                                   (void *)tmp_tlv_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Write report bearer technology TLV if appropriate */
  if (event_params->param_mask & QMI_WDS_EVENT_DORM_STATUS_IND)
  {
    unsigned char *val_ptr = tmp_tlv_buf;
    WRITE_8_BIT_VAL (val_ptr, (int)((event_params->report_dorm_status) ? TRUE : FALSE));
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_EVENT_RPT_DORM_STATUS_REQ_TLV_ID,
                                   1,
                                   (void *)tmp_tlv_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Write report bearer technology TLV if appropriate */
  if (event_params->param_mask & QMI_WDS_EVENT_DATA_CAPABILITIES_IND)
  {
    unsigned char *val_ptr = tmp_tlv_buf;
    WRITE_8_BIT_VAL (val_ptr, (int)((event_params->report_data_capabilities) ? TRUE : FALSE));
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_EVENT_RPT_DATA_CAPABILITIES_REQ_TLV_ID,
                                   1,
                                   (void *)tmp_tlv_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

   /* Write report bearer technology TLV if appropriate */
  if (event_params->param_mask & QMI_WDS_EVENT_DATA_CALL_STATUS_CHG_IND)
  {
    unsigned char *val_ptr = tmp_tlv_buf;
    WRITE_8_BIT_VAL (val_ptr, (int)((event_params->report_data_call_status_chg) ? TRUE : FALSE));
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_EVENT_RPT_DATA_CALL_STATUS_CHG_REQ_TLV_ID,
                                   1,
                                   (void *)tmp_tlv_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

   /* Write report bearer technology TLV if appropriate */
  if (event_params->param_mask & QMI_WDS_EVENT_PREF_DATA_SYS_IND)
  {
    unsigned char *val_ptr = tmp_tlv_buf;
    WRITE_8_BIT_VAL (val_ptr, (int)((event_params->report_pref_data_sys) ? TRUE : FALSE));
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_EVENT_RPT_PREF_DATA_SYS_REQ_TLV_ID,
                                   1,
                                   (void *)tmp_tlv_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Write report data system status TLV if appropriate */
  if (event_params->param_mask & QMI_WDS_EVENT_DATA_SYS_STATUS_IND)
  {
    unsigned char *val_ptr = tmp_tlv_buf;
    WRITE_8_BIT_VAL (val_ptr, (int)((event_params->report_data_sys_status) ? TRUE : FALSE));
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_EVENT_RPT_DATA_SYS_STATUS_REQ_TLV_ID,
                                1,
                                (void *)tmp_tlv_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Write report data system status TLV if appropriate */
  if (event_params->param_mask & QMI_WDS_EVENT_LIMITED_DATA_SYS_STATUS_IND)
  {
    unsigned char *val_ptr = tmp_tlv_buf;
    WRITE_8_BIT_VAL (val_ptr, (int)((event_params->report_limited_data_sys_status) ? TRUE : FALSE));
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_EVENT_RPT_LIMITED_DATA_SYS_STATUS_REQ_TLV_ID,
                                1,
                                (void *)tmp_tlv_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Synchronously send message to modem processor */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_SET_EVENT_REPORT_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}  /* qmi_wds_set_event_report */

/*===========================================================================
  FUNCTION  qmi_wds_internal_iface_event_reg_req
===========================================================================*/
/*!
@brief
  Registers the control point for internal iface events which can be
  specified by an event mask.


@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
*/
/*=========================================================================*/
int
qmi_wds_internal_iface_event_reg_req
(
int                                       user_handle,
qmi_wds_iface_event_register_param_type   event_mask,
int                                       *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /*Create a iface event Registration TLV*/
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_WDS_IFACE_EVENT_REGISTRATION_TLV_ID,
                                 4,
                                 (void *)&event_mask) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  /* Synchronously send message to modem processor */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_INTERNAL_IFACE_EV_REGISTER_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;

}

/*===========================================================================
  FUNCTION  qmi_wds_get_curr_call_info
===========================================================================*/
/*!
@brief
  Takes as input a user client handle, and returns the current call
  info for that client in the profile_id, profile_params and call_settings

@return
  QMI_INTERNAL_ERR or QMI_SERVICE_ERR if an error occurred, QMI_NO_ERR if not.
  If QMI_SERVICE_ERR is returned, the qmi_err_code return parameter
  will be valid, otherwise it won't

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - Creates a new profile on the modem processor.
*/
/*=========================================================================*/
int
qmi_wds_get_curr_call_info
(
  int                                               user_handle,
  qmi_wds_req_runtime_settings_params_type          requested_settings,
  qmi_wds_profile_id_type                           *profile_id,
  qmi_wds_profile_params_type                       *profile_params,
  qmi_wds_curr_call_info_type                       *call_settings,
  int                                               *qmi_err_code
)
{
  unsigned char     msg[QMI_MAX_MSG_SIZE];
  int               msg_size;
  unsigned char     *reply_msg;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  /* QoS reqest messages can be very large, so we dynamically allocate these */
  reply_msg = (unsigned char *) malloc (QMI_MAX_MSG_SIZE);
  if (!reply_msg)
  {
    QMI_ERR_MSG_0 ("Unable do dynamically allocate memory for curr_call_info reply\n");
    return QMI_INTERNAL_ERR;
  }

  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /*Create a iface event Registration TLV*/
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_WDS_REQUEST_RUNTIME_SETTINGS_TLV_ID,
                                 4,
                                 (void *)&requested_settings) < 0)
  {
    rc = QMI_INTERNAL_ERR;
  }
  else
  {
    /* Send the message */
    rc = qmi_service_send_msg_sync (user_handle,
                                    QMI_WDS_SERVICE,
                                    QMI_WDS_GET_RUNTIME_SETTINGS_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                    reply_msg,
                                    &msg_size,
                                    QMI_MAX_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    qmi_err_code);

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = reply_msg;
  }

  /* Get profile settings information */
  if (rc == QMI_NO_ERR)
  {
    rc = qmi_wds_read_curr_call_tlvs (tmp_msg_ptr,msg_size,profile_id,profile_params,call_settings);
  }
  free (reply_msg);
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_profile_list
===========================================================================*/
/*!
@brief
  Takes as input a user client handle, and an array of profile query
  structures in which to place response, and the number of elements in that
  array.

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
qmi_wds_get_profile_list
(
  int                             user_handle,
  qmi_wds_profile_list_type  *profile_list,
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

  /* Send the message */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_PROFILE_LIST_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
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
  FUNCTION  qmi_wds_route_look_up
===========================================================================*/
/*!
@brief
  Takes as input the route lookup params type and returns the route
  lookup response.

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
qmi_wds_route_look_up
(
  int                               user_handle,
  qmi_wds_route_look_up_params_type *params,
  qmi_wds_route_look_up_rsp_type    *rsp_data,
  int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Make sure params is passed in and that only valid Route lookup
  ** optional params were passed in
  */
  if ((!params) || (!rsp_data))
  {
    QMI_ERR_MSG_2 ("qmi_wds_route_look_up: Error -- NULL params=%p or rsp_data=%p\n",
                                                           params,rsp_data);
    return QMI_INTERNAL_ERR;
  }

  /*Fill in the Mandatory TLV */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_WDS_ROUTE_LOOKUP_TYPE_REQ_TLV_ID,
                                 1,
                                 (void *)&params->route_lookup) < 0)
  {
    return QMI_INTERNAL_ERR;
  }


  /* Configure optional TLV's if the params are present */
  if (params->params_mask != 0)
  {
    if (qmi_wds_write_optional_start_nw_if_tlvs (&tmp_msg_ptr,
                                                 &msg_size,
                                                 params) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Send message */
  rc = qmi_service_send_msg_sync (user_handle,
                                    QMI_WDS_SERVICE,
                                    QMI_WDS_ROUTE_LOOK_UP_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_WDS_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    qmi_err_code);


  /* Get route look_up information */
  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;

    /* Initialize indication that we go qmi_inst to FALSE */
    memset(rsp_data, 0, sizeof(*rsp_data));
    rsp_data->qmi_inst_is_valid = FALSE;

    /* Loop through all TLV's and process each one */
    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_ROUTE_LOOKUP_IFACE_TLV_ID:
          {
            READ_32_BIT_VAL (value_ptr,rsp_data->iface_handle);
            READ_32_BIT_VAL (value_ptr,rsp_data->priority);
            READ_16_BIT_VAL (value_ptr,rsp_data->tech_name);
          }
          break;

    /* Modem would send either QMI instance or MUX id but not both */
        case QMI_WDS_ROUTE_LOOKUP_QMI_INST_TLV_ID:
        case QMI_WDS_ROUTE_LOOKUP_MUX_ID_TLV_ID:
          {
            rsp_data->qmi_inst_is_valid = TRUE;
            READ_8_BIT_VAL (value_ptr,rsp_data->qmi_inst);
            if(QMI_WDS_ROUTE_LOOKUP_MUX_ID_TLV_ID == type)
            {
              --rsp_data->qmi_inst;
              QMI_DEBUG_MSG_1 ("Mapped MUX ID to QMI inst [%d]",(int)rsp_data->qmi_inst);

            }
          }
          break;

        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_route_look_up: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_internal_runtime_settings
===========================================================================*/
/*!
@b//rief
  Takes as input the intenal settings request params type and returns the
  data sessions settings currently in use.

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.
  //
  - Side Effects
    - None.
*/
/*=========================================================================*/

int
qmi_wds_get_internal_runtime_settings
(
int                                            user_handle,
qmi_wds_internal_runtime_setings_params_type   *params,
qmi_wds_internal_runtime_settings_rsp_type     *rsp_data,
int                                            *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  if (params && (params->params_mask != 0))
  {
    if (qmi_wds_write_optional_get_runtime_settings_tlvs (&tmp_msg_ptr,
                                                          &msg_size,
                                                          params) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Send message */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_CURRENT_RUNTIME_SETTINGS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (QMI_NO_ERR == rc)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(rsp_data, 0, sizeof(*rsp_data));
    rsp_data->params_mask = 0;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_OP_FAILURE_TLV_ID:
          {
            int index;
            int num_instances = 0;
            /*Set Param Mask*/
            rsp_data->params_mask |= QMI_WDS_RUNTIME_SETTINGS_RSP_OP_FAILURE;

            READ_8_BIT_VAL (value_ptr,rsp_data->op_failure_info.num_instances);
            num_instances = rsp_data->op_failure_info.num_instances;
            for  (index = 0; index < num_instances; index++)
            {
              READ_8_BIT_VAL (value_ptr,rsp_data->op_failure_info.failure_info[index].bit_number);
              READ_16_BIT_VAL (value_ptr,rsp_data->op_failure_info.failure_info[index].dss_errno);
            }
          }
          break;

        case QMI_WDS_DORM_TIMER_1X_TLV_ID:
          {
            /*Set Param Mask*/
            rsp_data->params_mask |= QMI_WDS_RUNTIME_SETTINGS_DORM_TIMER_1X;
            READ_32_BIT_VAL (value_ptr,rsp_data->dorm_timer_for_1x);
          }
          break;
        case QMI_WDS_SESSION_TIMER_1X_TLV_ID:
          {
            /*Set Param Mask*/
            rsp_data->params_mask |= QMI_WDS_RUNTIME_SETTINGS_SESSION_TIMER_1X;
            READ_16_BIT_VAL (value_ptr,rsp_data->session_timer_for_1x);
          }
          break;
        case QMI_WDS_HDR_1X_HANDDOWN_TLV_ID:
          {
            unsigned char temp;
            /*Set Param Mask*/
            rsp_data->params_mask |= QMI_WDS_RUNTIME_SETTINGS_HANDDOWN_OPT_HDR_1X;
            READ_8_BIT_VAL (value_ptr,temp);
            rsp_data->hdr_handdown_option = (qmi_wds_hdr_handdown_option_1x) temp;
          }
          break;
        case QMI_WDS_HYSTERISIS_ACTIVATION_TIMER_TLV_ID:
          {
            /*Set Param Mask*/
            rsp_data->params_mask |= QMI_WDS_RUNTIME_SETTINGS_HYSTERISIS_ACT_TIMER;
            READ_32_BIT_VAL (value_ptr,rsp_data->hysteresis_activation_timer);
          }
          break;
        case QMI_WDS_SDB_SUPPORT_TLV_ID:
          {
            unsigned char temp;
            /*Set Param Mask*/
            rsp_data->params_mask |= QMI_WDS_RUNTIME_SETTINGS_SDB_SUPPORT;
            READ_8_BIT_VAL (value_ptr,temp);
            rsp_data->sdb_support = (qmi_wds_sdb_support)temp;
          }
          break;
        case QMI_WDS_RF_CONDITIONS_TLV_ID:
          {
            unsigned char temp_8bit;
            unsigned long temp_32bit;
            /*Set Param Mask*/
            rsp_data->params_mask |= QMI_WDS_RUNTIME_SETTINGS_RF_CONDITIONS;

            READ_8_BIT_VAL (value_ptr,temp_8bit);
            rsp_data->rf_conditions.current_db_nw = (qmi_wds_data_bearer_type)temp_8bit;
            if ((int)rsp_data->rf_conditions.current_db_nw == QMI_WDS_FOR_RF_COND_CDMA_TYPE)
            {
              READ_32_BIT_VAL (value_ptr,temp_32bit);
              rsp_data->rf_conditions.rat_mask.cdma_rat_mask = (qmi_wds_cdma_rat_mask)temp_32bit;
            }
            else
            {
              READ_32_BIT_VAL (value_ptr,temp_32bit);
              rsp_data->rf_conditions.rat_mask.umts_rat_mask  = (qmi_wds_umts_rat_mask)temp_32bit;
            }
            if ((int)rsp_data->rf_conditions.current_db_nw == QMI_WDS_FOR_RF_COND_UMTS_TYPE
                || (int)rsp_data->rf_conditions.rat_mask.cdma_rat_mask == QMI_WDS_FOR_RF_COND_CDMA_EVDO_REV0)
            {
              rsp_data->rf_conditions.db_so_mask.so_mask_value = 0;
              /*Make Sure the value stored is zero*/
            }
            if ((int)rsp_data->rf_conditions.rat_mask.cdma_rat_mask == QMI_WDS_FOR_RF_COND_CDMA_1X)
            {
              READ_32_BIT_VAL (value_ptr,temp_32bit);
              rsp_data->rf_conditions.db_so_mask.so_mask_1x = (qmi_wds_cdma_1x_db_so_mask)temp_32bit;
            }
            else if ((int)rsp_data->rf_conditions.rat_mask.cdma_rat_mask == QMI_WDS_FOR_RF_COND_CDMA_EVDO_REVA)
            {
              READ_32_BIT_VAL (value_ptr,temp_32bit);
              rsp_data->rf_conditions.db_so_mask.so_mask_evdo_reva = (qmi_wds_cdma_evdo_db_so_mask)temp_32bit;
            }

            READ_8_BIT_VAL (value_ptr,temp_8bit);
            rsp_data->rf_conditions.rf_conditions = (qmi_wds_rf_conditions)temp_8bit;
          }
          break;
        case QMI_WDS_HDR_EIDLE_SLOT_MODE_OPT_TLV_ID:
          {
            /*Set Param Mask*/
            rsp_data->params_mask |= QMI_WDS_RUNTIME_SETTINGS_HDR_EIDLE_SLOTTED_MODE_OPT;
            READ_32_BIT_VAL (value_ptr,rsp_data->hdr_eidle_slot_cycle_value);
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_internal_runtime_settings: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }/*switch*/
    }/*while*/
  }/*if*/
  return rc;
}



/*===========================================================================
  FUNCTION  qmi_wds_set_internal_runtime_settings
===========================================================================*/
/*!
@brief
  Takes as input the intenal settings request params type and allows the
  control point to set or modify some internal packet data session settings.

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
qmi_wds_set_internal_runtime_settings
(
int                                                 user_handle,
qmi_wds_set_internal_runtime_settings_params_type   *params,
qmi_wds_set_internal_runtime_settings_rsp_type      *rsp_data,
int                                                 *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  if (params && (params->params_mask != 0))
  {
    if (qmi_wds_write_optional_set_runtime_settings_tlvs (&tmp_msg_ptr,
                                                          &msg_size,
                                                          params) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Send message */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_SET_CURRENT_RUNTIME_SETTINGS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (QMI_NO_ERR == rc)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(rsp_data, 0, sizeof(*rsp_data));
    rsp_data->params_mask = 0;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_OP_FAILURE_TLV_ID:
          {
            int index;
            int num_instances = 0;
            /*Set Param Mask*/
            rsp_data->params_mask |= QMI_WDS_RUNTIME_SETTINGS_RSP_OP_FAILURE;

            READ_8_BIT_VAL (value_ptr,rsp_data->op_failures.num_failures);
            num_instances = rsp_data->op_failures.num_failures;
            for (index = 0; index < num_instances; index++)
            {
              READ_8_BIT_VAL (value_ptr,rsp_data->op_failures.op_failure[index].tlv_type);
              READ_16_BIT_VAL (value_ptr,rsp_data->op_failures.op_failure[index].dss_errno);
            }
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_set_internal_runtime_settings: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_refresh_dhcp_config_info
===========================================================================*/
/*!
@brief
  This message will trigger DHCP in the background and allow the control point
  to refresh the dhcp configuration.

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
qmi_wds_refresh_dhcp_config_info
(
int                    user_handle,
int                    *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size = 0;
  int rc;

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_REFRESH_DHCP_CONFIG_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);


  return rc;
}
/*===========================================================================
  FUNCTION  qmi_wds_reg_mobile_terminated_call_req
===========================================================================*/
/*!
@brief
  This message will allow a control point to register for a mobile terminated
  (MT) call request event. 3GPP profile ID is the input along with ip family
  preference and tech preference.

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
qmi_wds_reg_mobile_terminated_call_req
(
int                                         user_handle,
qmi_wds_reg_mob_term_call_req_params        *params,
qmi_wds_reg_mob_terminated_call_rsp_type    *rsp_data,
int                                         *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /*Stuff in the Mandatory TLVS into request msg*/
  if (params->params_mask & QMI_WDS_MT_REG_CB_IP_FAMILY_PARAM)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_IP_FAMILY_PREF_REQ_TLV_ID,
                                   1,
                                   (void *)&params->ip_family) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (params->params_mask & QMI_WDS_MT_REG_CB_PROFILE_INDEX_PARAM)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_3GPP_PROFILE_ID_REQ_TLV_ID,
                                   1,
                                   (void *)&params->profile_index) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_WDS_MT_REG_CB_TECH_PREF_PARAM)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_TECH_PREF_REQ_TLV_ID,
                                   2,
                                   (void *)&params->xtended_tech_pref) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_REGISTER_MT_CALL_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (QMI_NO_ERR == rc)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(rsp_data, 0, sizeof(*rsp_data));

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_MT_PACKET_HNDL:
          {
            unsigned long temp;
            READ_32_BIT_VAL (value_ptr,temp);
            rsp_data->pkt_hndl = temp;
          }
          break;

        case QMI_WDS_EXTENDED_ERROR_INFO:
          {
            READ_16_BIT_VAL(value_ptr, rsp_data->dss_errno);
          }
          break;

        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_reg_mobile_terminated_call: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_dereg_mobile_terminated_call_req
===========================================================================*/
/*!
@brief
  This message will take as input a mobile terminated related packet handle
  allow the control point to de register for mob term call request event.

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
qmi_wds_dereg_mobile_terminated_call_req
(
int                                           user_handle,
mob_terminated_packet_handle                  *mt_handle,
qmi_wds_de_reg_mob_terminated_call_rsp_type   *rsp_data,
int                                           *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /*Stuff in the Mandatory TLVS into request msg*/
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_WDS_MT_PACKET_HNDL,
                                 4,
                                 (void *)mt_handle) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_DE_REGISTER_MT_CALL_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);
  if (QMI_NO_ERR == rc)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(rsp_data, 0, sizeof(*rsp_data));
    rsp_data->param_mask = 0;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
      switch (type)
      {
        case QMI_WDS_EXTENDED_ERROR_INFO:
          {
            rsp_data->param_mask = QMI_WDS_EXTENDED_ERROR_INFO_PARAM;
            READ_16_BIT_VAL (value_ptr,rsp_data->dss_errno);
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_dereg_mobile_terminated_call_req: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_bcmcs_db_update_req
===========================================================================*/
/*!
@brief
  This message will take as input the bcmcs database info and allow the
  control point to update the bcmcs database table.

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
/*Move them to the top*/
#define QMI_WDS_IPV4    0x04
#define QMI_WDS_IPV6    0x06

int
qmi_wds_bcmcs_db_update_req
(
int                                    user_handle,
qmi_wds_bcmcs_db_updt_params_type      *bcmcs_db_data,
qmi_wds_bcmcs_db_updt_rsp_type         *rsp_data,
int                                    *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;
  unsigned long tlv_length = 0;
  unsigned char     tmp_buff[QMI_WDS_STD_MSG_SIZE];
  unsigned char     *tmp_buff_ptr = tmp_buff;
  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  if ((int)bcmcs_db_data->ip_family == (int)QMI_WDS_IPV4)
    tlv_length = 41;/*Manually counted the length from the spec for
                      both cases, In future any element adds up,
                      Please update the length.*/
  else if ((int)bcmcs_db_data->ip_family == (int)QMI_WDS_IPV6)
    tlv_length = 53;

  /*Stuff in data now*/
  WRITE_8_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->zone_type);
  (void)qmi_wds_copy_n_byte_value(&tmp_buff_ptr,bcmcs_db_data->subnet_hdr,16);
  WRITE_8_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->ip_family);

  if ((int)bcmcs_db_data->ip_family == (int)QMI_WDS_IPV4)
    WRITE_32_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->ip_addr.ipv4);
  else if ((int)bcmcs_db_data->ip_family == (int)QMI_WDS_IPV6)
    (void)qmi_wds_copy_n_byte_value(&tmp_buff_ptr,bcmcs_db_data->ip_addr.ipv6,16);

  WRITE_32_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->prog_id);
  WRITE_8_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->prog_id_len);
  WRITE_8_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->flow_discrim_len);
  WRITE_8_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->flow_discrim);
  WRITE_16_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->port);
  WRITE_8_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->frame_type);
  WRITE_8_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->protocol_type);
  WRITE_8_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->crc_length);
  WRITE_8_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->flow_format);
  WRITE_32_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->flow_id);
  WRITE_8_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->overwrite);
  WRITE_8_BIT_VAL(tmp_buff_ptr,bcmcs_db_data->flow_priority);

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_WDS_BCMCS_DB_UPDATE_REQ_TLV_ID,
                                 tlv_length,
                                 (void *)tmp_buff) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_BCMCS_DB_UPDATE_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);
  if (QMI_NO_ERR == rc)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(rsp_data, 0, sizeof(*rsp_data));
    rsp_data->param_mask = 0;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
      switch (type)
      {
        case QMI_WDS_EXTENDED_ERROR_INFO:
          {
            rsp_data->param_mask = QMI_WDS_EXTENDED_ERROR_INFO_PARAM;
            READ_16_BIT_VAL (value_ptr,rsp_data->dss_errno);
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_bcmcs_db_update_req: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_bcmcs_enable_handoff_reg_req
===========================================================================*/
/*!
@brief
  This message will take as input a bcmcs handoff optimization info and allows
  a control point to perform bcmcs handoff optimization for registration.

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
qmi_wds_bcmcs_enable_handoff_reg_req
(
int                                          user_handle,
qmi_wds_bcmcs_handoff_optimization_info      *handoff_optization_info,
qmi_wds_bcmcs_enable_handoff_reg_rsp_type    *rsp_data,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;
  unsigned long tlv_length = 0;
  unsigned char     tmp_buff[QMI_WDS_STD_MSG_SIZE];
  unsigned char     *tmp_buff_ptr = tmp_buff;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  if (handoff_optization_info->num_mcast_addr == 0)
  {
    tlv_length = 1;
    WRITE_8_BIT_VAL(tmp_buff_ptr,handoff_optization_info->num_mcast_addr);
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_BCMCS_HANDOFF_ENABLE_REQ_TLV_ID,
                                   tlv_length,
                                   (void *)tmp_buff) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  else if (handoff_optization_info->num_mcast_addr != 0)
  {
    int index;
    WRITE_8_BIT_VAL(tmp_buff_ptr,handoff_optization_info->num_mcast_addr);
    tlv_length += 1;

    for (index = 0; index < handoff_optization_info->num_mcast_addr; index++)
    {
      if ((int)handoff_optization_info->ip_info[index].ip_family == QMI_WDS_IPV4)
      {
        tlv_length += 7;
        WRITE_8_BIT_VAL(tmp_buff_ptr,handoff_optization_info->ip_info[index].ip_family);
        WRITE_32_BIT_VAL(tmp_buff_ptr,handoff_optization_info->ip_info[index].ip_addr.ipv4);
        WRITE_16_BIT_VAL(tmp_buff_ptr,handoff_optization_info->ip_info[index].mcast_port);
      }
      else if ((int)handoff_optization_info->ip_info[index].ip_family == QMI_WDS_IPV6)
      {
        tlv_length += 19;
        WRITE_8_BIT_VAL(tmp_buff_ptr,handoff_optization_info->ip_info[index].ip_family);
        (void)qmi_wds_copy_n_byte_value(&tmp_buff_ptr,handoff_optization_info->ip_info[index].ip_addr.ipv6,16);
        WRITE_16_BIT_VAL(tmp_buff_ptr,handoff_optization_info->ip_info[index].mcast_port);
      }
    }

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_BCMCS_HANDOFF_ENABLE_REQ_TLV_ID,
                                   tlv_length,
                                   (void *)tmp_buff) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_BCMCS_HANDOFF_OPT_ENABLE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);
  if (QMI_NO_ERR == rc)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(rsp_data, 0, sizeof(*rsp_data));
    rsp_data->param_mask = 0;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
      switch (type)
      {
        case QMI_WDS_EXTENDED_ERROR_INFO:
          {
            rsp_data->param_mask = QMI_WDS_EXTENDED_ERROR_INFO_PARAM;
            READ_16_BIT_VAL (value_ptr,rsp_data->dss_errno);
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_bcmcs_db_update_req: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_bcmcs_bom_caching_setup_req
===========================================================================*/
/*!
@brief
  This message will take as input a bmccs caching setup info and thereby
  allows a control point to enable or disable bcmcs bom caching

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
qmi_wds_bcmcs_bom_caching_setup_req
(
int                                          user_handle,
qmi_wds_bom_caching_setup_req_param_type     *bom_setup_info,
qmi_wds_bcmcs_bom_caching_setup_rsp_type     *rsp_data,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;
  unsigned long tlv_length = 0;
  unsigned char     tmp_buff[QMI_WDS_STD_MSG_SIZE];
  unsigned char     *tmp_buff_ptr = tmp_buff;
  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  tlv_length = 2;
  /*Stuff in data now*/
  WRITE_8_BIT_VAL(tmp_buff_ptr,bom_setup_info->bom_enbl_dsbl);
  WRITE_8_BIT_VAL(tmp_buff_ptr,bom_setup_info->bom_cache_timeout);

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_WDS_BCMCS_BOM_CACHING_SETUP_REQ_TLV_ID,
                                 tlv_length,
                                 (void *)tmp_buff) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_BCMCS_BOM_CACHING_SETUP_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);
  if (QMI_NO_ERR == rc)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(rsp_data, 0, sizeof(*rsp_data));
    rsp_data->param_mask = 0;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
      switch (type)
      {
        case QMI_WDS_EXTENDED_ERROR_INFO:
          {
            rsp_data->param_mask = QMI_WDS_EXTENDED_ERROR_INFO_PARAM;
            READ_16_BIT_VAL (value_ptr,rsp_data->dss_errno);
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_bcmcs_db_update_req: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }

  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_initiate_mcast_join_ex
===========================================================================*/
/*!
@brief
  This message will take as input a list of multicast ip addresses and
  initiates a join on those addresses to a session.

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
qmi_wds_initiate_mcast_join_ex
(
int                                          user_handle,
qmi_wds_mcast_join_req_params_type           *mcast_info,
qmi_wds_initiate_mcast_join_ex_rsp_type      *rsp_data,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;
  unsigned long tlv_length = 0;
  unsigned char     tmp_buff[QMI_WDS_STD_MSG_SIZE];
  unsigned char     *tmp_buff_ptr = tmp_buff;
  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  if (mcast_info->num_flows == 0)
  {
    tlv_length = 1;
    WRITE_8_BIT_VAL(tmp_buff_ptr,mcast_info->num_flows);

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_INITIATE_MCAST_JOIN_EX_REQ_TLV_ID,
                                   tlv_length,
                                   (void *)tmp_buff) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  else if (mcast_info->num_flows != 0)
  {
    int index;
    WRITE_8_BIT_VAL(tmp_buff_ptr,mcast_info->num_flows);
    tlv_length += 1;

    for  (index = 0; index < mcast_info->num_flows; index++)
    {
      if ((int)mcast_info->ip_info[index].mcast_ip_info.ip_family == QMI_WDS_IPV4)
      {
        tlv_length += 8;
        WRITE_8_BIT_VAL(tmp_buff_ptr,mcast_info->ip_info[index].mcast_ip_info.ip_family);
        WRITE_32_BIT_VAL(tmp_buff_ptr,mcast_info->ip_info[index].mcast_ip_info.ip_addr.ipv4);
        WRITE_16_BIT_VAL(tmp_buff_ptr,mcast_info->ip_info[index].mcast_ip_info.mcast_port);
        WRITE_8_BIT_VAL(tmp_buff_ptr,mcast_info->ip_info[index].config_flag);
      }
      else if ((int)mcast_info->ip_info[index].mcast_ip_info.ip_family == QMI_WDS_IPV6)
      {
        tlv_length += 20;
        WRITE_8_BIT_VAL(tmp_buff_ptr,mcast_info->ip_info[index].mcast_ip_info.ip_family);
        (void)qmi_wds_copy_n_byte_value(&tmp_buff_ptr,mcast_info->ip_info[index].mcast_ip_info.ip_addr.ipv6,16);
        WRITE_16_BIT_VAL(tmp_buff_ptr,mcast_info->ip_info[index].mcast_ip_info.mcast_port);
        WRITE_8_BIT_VAL(tmp_buff_ptr,mcast_info->ip_info[index].config_flag);
      }
    }

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_INITIATE_MCAST_JOIN_EX_REQ_TLV_ID,
                                   tlv_length,
                                   (void *)tmp_buff) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_INITIATE_MCAST_JOIN_EX_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);
  if (QMI_NO_ERR == rc)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(rsp_data, 0, sizeof(*rsp_data));
    rsp_data->dss_errno.param_mask = 0;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_MCAST_HNDL_LIST_RESP_TLV_ID:
          {
            int num_instances = 0;
            int index;
            READ_8_BIT_VAL (value_ptr,rsp_data->hndl_list.num_handles);
            num_instances = rsp_data->hndl_list.num_handles;
            for (index = 0; index < num_instances; index++)
              {
                READ_32_BIT_VAL (value_ptr,rsp_data->hndl_list.mcast_handle_list[index]);
              }
          }
          break;
        case QMI_WDS_EXTENDED_ERROR_INFO:
          {
            rsp_data->dss_errno.param_mask = QMI_WDS_EXTENDED_ERROR_INFO_PARAM;
            READ_16_BIT_VAL (value_ptr,rsp_data->dss_errno.dss_errno);
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_initiate_mcast_join_ex: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}
/*===========================================================================
  FUNCTION  qmi_wds_initiate_mcast_leave_ex
===========================================================================*/
/*!
@brief
  This message will take as input the list of mulitcast handles and initiates
  them to leave the flows/sessions .

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
qmi_wds_initiate_mcast_leave_ex
(
int                                          user_handle,
qmi_wds_mcast_hndl_list_type                 *mcast_hndl_list,
qmi_wds_initiate_mcast_leave_ex_rsp_type     *rsp_data,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;
  unsigned long tlv_length = 0;
  unsigned char     tmp_buff[QMI_WDS_STD_MSG_SIZE];
  unsigned char     *tmp_buff_ptr = tmp_buff;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  if (mcast_hndl_list->num_handles == 0)
  {
    tlv_length = 1;
    WRITE_8_BIT_VAL(tmp_buff_ptr,mcast_hndl_list->num_handles);

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_MCAST_HNDL_LIST_REQ_TLV_ID,
                                   tlv_length,
                                   (void *)tmp_buff) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  else if (mcast_hndl_list->num_handles != 0)
  {
    int index;
    tlv_length = sizeof(char) + (unsigned long)((mcast_hndl_list->num_handles) * 4);
    WRITE_8_BIT_VAL(tmp_buff_ptr,mcast_hndl_list->num_handles);

    for (index = 0; index < mcast_hndl_list->num_handles; index++)
    {
      WRITE_32_BIT_VAL(tmp_buff_ptr,mcast_hndl_list->mcast_handle_list[index]);
    }

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_MCAST_HNDL_LIST_REQ_TLV_ID,
                                   tlv_length,
                                   (void *)tmp_buff) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_INITIATE_MCAST_LEAVE_EX_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (QMI_NO_ERR == rc)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(rsp_data, 0, sizeof(*rsp_data));
    rsp_data->param_mask  = 0;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
      switch (type)
      {
        case QMI_WDS_EXTENDED_ERROR_INFO:
          {
            rsp_data->param_mask = QMI_WDS_EXTENDED_ERROR_INFO_PARAM;
            READ_16_BIT_VAL (value_ptr,rsp_data->dss_errno);
          }
          break;
          default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_bcmcs_db_update_req: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }

  return rc;
}
/*===========================================================================
  FUNCTION  qmi_wds_initiate_mcast_register_ex
===========================================================================*/
/*!
@brief
  This message will take as input the list of mulitcast handles and initiates
  them to leave the flows/sessions .

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
qmi_wds_initiate_mcast_register_ex
(
int                                          user_handle,
qmi_wds_mcast_hndl_list_type                 *mcast_handle_list,
qmi_wds_initiate_mcast_reg_ex_rsp_type       *rsp_data,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;
  unsigned long tlv_length = 0;
  unsigned char     tmp_buff[QMI_WDS_STD_MSG_SIZE];
  unsigned char     *tmp_buff_ptr = tmp_buff;
  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  if (mcast_handle_list->num_handles == 0)
  {
    tlv_length = 1;
    WRITE_8_BIT_VAL(tmp_buff_ptr,mcast_handle_list->num_handles);

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_MCAST_HNDL_LIST_REQ_TLV_ID,
                                   tlv_length,
                                   (void *)tmp_buff) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  else if (mcast_handle_list->num_handles != 0)
  {
    int index;
    tlv_length = sizeof(char) + (unsigned long)((mcast_handle_list->num_handles) * 4);
    WRITE_8_BIT_VAL(tmp_buff_ptr,mcast_handle_list->num_handles);

    for (index = 0; index < mcast_handle_list->num_handles; index++)
    {
      WRITE_32_BIT_VAL(tmp_buff_ptr,mcast_handle_list->mcast_handle_list[index]);
    }

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_MCAST_HNDL_LIST_REQ_TLV_ID,
                                   tlv_length,
                                   (void *)tmp_buff) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_INITIATE_MCAST_REGISTER_EX_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (QMI_NO_ERR == rc)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(rsp_data, 0, sizeof(*rsp_data));
    rsp_data->param_mask = 0;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
      switch (type)
      {
        case QMI_WDS_EXTENDED_ERROR_INFO:
          {
            rsp_data->param_mask = QMI_WDS_EXTENDED_ERROR_INFO_PARAM;
            READ_16_BIT_VAL (value_ptr,rsp_data->dss_errno);
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_bcmcs_db_update_req: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_initiate_mcast_join
===========================================================================*/
/*!
@brief
  This message will take as input the multicast ip addr, and joins the ip addr
  to the multicast session. The differece from the ex version is that only a
  single ip addr message is handled here to be joined.

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
qmi_wds_initiate_mcast_join
(
int                                          user_handle,
qmi_wds_mcast_ipv4_join_info_param_type      *mcast_join_info,
qmi_wds_mcast_hndl_rsp_type                  *mcast_hndl_resp,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  if (mcast_join_info->params_mask && (mcast_join_info->params_mask != 0))
  {
    if (qmi_wds_write_optional_mcast_join_tlvs (&tmp_msg_ptr,
                                                &msg_size,
                                                mcast_join_info) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_MCAST_JOIN_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(mcast_hndl_resp, 0, sizeof(*mcast_hndl_resp));

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_MCAST_JOIN_MCAST_HNDL_RESP_TLV_ID:
          {
            READ_32_BIT_VAL(value_ptr, *mcast_hndl_resp);
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_initiate_mcast_join: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_initiate_mcast_leave
===========================================================================*/
/*!
@brief
  This message will take as input the multicast ip addr, and joins the ip addr
  to the multicast session. The differece from the ex version is that only a
  single ip addr message is handled here to be joined.

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
qmi_wds_initiate_mcast_leave
(
int                                          user_handle,
qmi_wds_mcast_hndl_param_type                *mcast_handle,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /*Stuff in the Mandatory TLVS into request msg*/
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_WDS_MCAST_LEAVE_MCAST_HNDL_REQ_TLV_ID,
                                 4,
                                 (void *)mcast_handle) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_MCAST_LEAVE_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}
/*===========================================================================
  FUNCTION  qmi_wds_go_active_req
===========================================================================*/
/*!
@brief
  This message will force the device to re-establish the traffic channel on
  serving radio interface immediately. This message has no user specified
  params required.

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
qmi_wds_go_active_req
(
int                                          user_handle,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  int rc;

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GO_ACTIVE_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);



  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_initiate_mbms_context_activate
===========================================================================*/
/*!
@brief
  This message will take as input the ipv4 context activation info consisting
  of ipv4 addr to join and profile id and then activates the MBMS context.

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
qmi_wds_initiate_mbms_context_activate
(
int                                          user_handle,
qmi_wds_mbms_context_req_params_type         *params,
qmi_wds_mbms_context_handle                  *rsp_data,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;
  unsigned long tlv_length = 0;
  unsigned char     tmp_buff[QMI_WDS_STD_MSG_SIZE];
  unsigned char     *tmp_buff_ptr = tmp_buff;
  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  if (params && (params->param_mask != 0))
  {
    if (params->param_mask & QMI_WDS_IPV4_CONTEXT_ACTIVATION_INFO_PARAM)
    {
      tlv_length = 5;

      WRITE_32_BIT_VAL(tmp_buff_ptr,params->ipv4_context_activate_info.multicast_ip_addr);
      WRITE_8_BIT_VAL(tmp_buff_ptr,params->ipv4_context_activate_info.profile_id);
      if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                     &msg_size,
                                     QMI_WDS_IPV4_CONTEXT_ACTIVATION_INFO_TLV_ID,
                                     tlv_length,
                                     (void *)tmp_buff) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }
  }


  /* Send message */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_ACTIVATE_MBMS_MCAST_CONTEXT_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);


  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(rsp_data, 0, sizeof(*rsp_data));

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_MBMS_CONTEXT_HANDLE_RSP_TLV_ID:
          {
            unsigned long tmp_rsp_data;
            READ_32_BIT_VAL (value_ptr,tmp_rsp_data);
            *rsp_data = tmp_rsp_data;
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_initiate_mbms_context_activate: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_initiate_mbms_context_deactivate
===========================================================================*/
/*!
@brief
  This message will take as input the multicast context handle and deactivates
  the mbms context.


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
qmi_wds_initiate_mbms_context_deactivate
(
int                                          user_handle,
qmi_wds_mbms_context_handle                  *param,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_WDS_MBMS_CONTEXT_HANDLE_RSP_TLV_ID,
                                 4,
                                 (void *)param) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_DEACTIVATE_MBMS_MCAST_CONTEXT_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_current_bearer_tech
===========================================================================*/
/*!
@brief
  This message queries the current data bearer technology.

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
qmi_wds_get_current_bearer_tech
(
int                                          user_handle,
qmi_wds_data_bearer_tech_type                *rsp_data,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_CURRENT_DATA_BEARER_TECH_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_CURRENT_DATA_BEARER_TECH_RESP_TLV_ID:
          {
            if (QMI_NO_ERR != qmi_wds_srvc_process_bearer_tech(&value_ptr,
                                                               rsp_data))
            {
              QMI_ERR_MSG_0 ("qmi_wds_get_current_bearer_tech: failed to process response TLV");
            }
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_current_bearer_tech: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_current_bearer_tech_ex
===========================================================================*/
/*!
@brief
  This message queries the current extended data bearer technology.

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
qmi_wds_get_current_bearer_tech_ex
(
int                                          user_handle,
qmi_wds_data_bearer_tech_type_ex             *rsp_data,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_CURRENT_DATA_BEARER_TECH_EX_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_CURRENT_DATA_BEARER_TECH_EX_CURRENT_CALL_TECH_TLV_ID:
          {
            if (QMI_NO_ERR != qmi_wds_srvc_process_bearer_tech_ex(&value_ptr,
                                                               rsp_data))
            {
              QMI_ERR_MSG_0 ("qmi_wds_get_current_bearer_tech: failed to process response TLV");
            }
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_current_bearer_tech: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_current_channel_rate
===========================================================================*/
/*!
@brief
  This message will force the device to re-establish the traffic channel on
  serving radio interface immediately. This message has no user specified
  params required.

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
qmi_wds_get_current_channel_rate
(
int                                          user_handle,
qmi_wds_channel_rate_type                    *resp_data,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_CURRENT_CHANNEL_RATE_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);
  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(resp_data, 0, sizeof(*resp_data));

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_CURR_CHANNEL_RATE_RESP_TLV_ID:
          {
            READ_32_BIT_VAL (value_ptr,resp_data->current_channel_tx_rate);
            READ_32_BIT_VAL (value_ptr,resp_data->current_channel_rx_rate);
            READ_32_BIT_VAL (value_ptr,resp_data->max_channel_tx_rate);
            READ_32_BIT_VAL (value_ptr,resp_data->max_channel_rx_rate);
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_current_channel_rate: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}
/*===========================================================================
  FUNCTION  qmi_wds_go_dormant_req
===========================================================================*/
/*!
@brief
  This message will force the device to drop the traffic channel on
  serving radio interface immediately. This message has no user specified
  params required.

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
qmi_wds_go_dormant_req
(
int                                          user_handle,
int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  int rc;

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GO_DORMANT_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);



  return rc;

}

/*===========================================================================
  FUNCTION  qmi_wds_reset_pkt_statistics
===========================================================================*/
/*!
@brief
  This message is used to reset packet data statistics of the interface
  associated with the user_handle.

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
qmi_wds_reset_pkt_statistics
(
int                               user_handle,
int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  int rc;

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_RESET_PKT_STATS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_pkt_statistics
===========================================================================*/
/*!
@brief
  This message is used to retrieve packet data statistics of the interface
  associated with the user_handle.

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
qmi_wds_get_pkt_statistics
(
int                               user_handle,
unsigned long                     stats_mask,
qmi_wds_xfer_stats                *stats_resp,
int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* TODO:  In future when additional params are added,
  ** make sure that no invalid parameters are set
  */

  if (stats_mask)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_WDS_GET_PKT_STATS_REQ_TLV_ID,
                                   4,
                                   (void *)&stats_mask) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  else
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_PKT_STATS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Get route look_up information */
  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(stats_resp, 0, sizeof(*stats_resp));

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_EVENT_RPT_XFER_STATS_TX_PKTS_GOOD_IND_TLV_ID:
          {
            stats_resp->mask |= QMI_WDS_XFER_STATS_TX_PKTS_GOOD;
            READ_32_BIT_VAL (value_ptr,stats_resp->tx_good_pkt_cnt);
          }
          break;
        case QMI_WDS_EVENT_RPT_XFER_STATS_RX_PKTS_GOOD_IND_TLV_ID:
          {
            stats_resp->mask |= QMI_WDS_XFER_STATS_RX_PKTS_GOOD;
            READ_32_BIT_VAL (value_ptr,stats_resp->rx_good_pkt_cnt);
          }
          break;
        case QMI_WDS_EVENT_RPT_XFER_STATS_TX_PKTS_ERR_IND_TLV_ID:
          {
            stats_resp->mask |= QMI_WDS_XFER_STATS_TX_PKTS_ERR;
            READ_32_BIT_VAL (value_ptr,stats_resp->tx_err_pkt_cnt);
          }
          break;
        case QMI_WDS_EVENT_RPT_XFER_STATS_RX_PKTS_ERR_IND_TLV_ID:
          {
            stats_resp->mask |= QMI_WDS_XFER_STATS_RX_PKTS_ERR;
            READ_32_BIT_VAL (value_ptr,stats_resp->rx_err_pkt_cnt);
          }
          break;
        case QMI_WDS_EVENT_RPT_XFER_STATS_TX_OVERFLOW_IND_TLV_ID:
          {
            stats_resp->mask |= QMI_WDS_XFER_STATS_TX_OVERFLOW;
            READ_32_BIT_VAL (value_ptr,stats_resp->tx_overflow_cnt);
          }
          break;
        case QMI_WDS_EVENT_RPT_XFER_STATS_RX_OVERFLOW_IND_TLV_ID:
          {
            stats_resp->mask |= QMI_WDS_XFER_STATS_RX_OVERFLOW;
            READ_32_BIT_VAL (value_ptr,stats_resp->rx_overflow_cnt);
          }
          break;
        case QMI_WDS_EVENT_RPT_XFER_STATS_TX_BYTES_GOOD_IND_TLV_ID:
          {
            stats_resp->mask |= QMI_WDS_XFER_STATS_TX_BYTES_OK;
            READ_64_BIT_VAL (value_ptr,stats_resp->tx_good_byte_cnt);
          }
          break;

        case QMI_WDS_EVENT_RPT_XFER_STATS_RX_BYTES_GOOD_IND_TLV_ID:
          {
            stats_resp->mask |= QMI_WDS_XFER_STATS_RX_BYTES_OK;
            READ_64_BIT_VAL (value_ptr,stats_resp->rx_good_byte_cnt);
          }
          break;

        case QMI_WDS_PKT_STATS_TX_DROPPED_TLV_ID:
          {
            stats_resp->mask |= QMI_WDS_XFER_STATS_TX_PACKETS_DROPPED;
            READ_32_BIT_VAL (value_ptr,stats_resp->tx_pkts_dropped);
          }
          break;

        case QMI_WDS_PKT_STATS_RX_DROPPED_TLV_ID:
          {
            stats_resp->mask |= QMI_WDS_XFER_STATS_RX_PACKETS_DROPPED;
            READ_32_BIT_VAL (value_ptr,stats_resp->rx_pkts_dropped);
          }
          break;

        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_pkt_statistics: unknown response "
                           "TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_get_pkt_srvc_status
===========================================================================*/
/*!
@brief
  Takes as input a user client handle and returns the packet service status
  of the associated connection.

@return
  Value < 0 if error occurred, QMI_NO_ERR (0) if not.
  If QMI_NO_ERR is returned, the returned packet service status will be valid,
  otherwise it won't be.

  If QMI_SERVICE_ERR error code is returned, the qmi_err_code return parameter
  will be valid, otherwise it won't

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
qmi_wds_get_pkt_srvc_status
(
  int                       user_handle,
  qmi_wds_link_status_type  *pkt_srvc_status,
  int                       *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int               rc;

  /* Send the message */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_PKT_SRVC_STATUS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
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
    /* Assume worst case */
    rc = QMI_INTERNAL_ERR;
    while (msg_size > 0)
    {
      unsigned long     type;
      unsigned long     length;
      unsigned char     *value_ptr;

#ifdef FEATURE_QMI_TEST
      if (*tmp_msg_ptr == QMI_RESULT_CODE_TYPE_ID)
      {
        rc = qmi_util_get_std_result_code( &tmp_msg_ptr,&msg_size,qmi_err_code );
        continue;
      }
#endif /*FEATURE_QMI_TEST*/

      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                  &msg_size,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
      {
        rc = QMI_INTERNAL_ERR;
        break;
      }

      if (type == QMI_WDS_PKT_SRVC_STATUS_TLV_ID)
      {
        unsigned char temp;
        /* Read the pkt srvc status and set return code to no error */
        READ_8_BIT_VAL (value_ptr,temp);
        *pkt_srvc_status = (qmi_wds_link_status_type) temp;
        rc = QMI_NO_ERR;
      }
      else
      {
        QMI_DEBUG_MSG_1 ("qmi_wds_get_pkt_srvc_status, unknown TLV returned = %x",(int)type);
      }
    }
  }

  return rc;
}



/*===========================================================================
  FUNCTION  qmi_wds_get_dormancy_status
===========================================================================*/
/*!
@brief
  Gets current dormancy status.

@return
  QMI_NO_ERR (0) if no error occurred, one of the other QMI_*_ERR codes (< 0)
  if an error occurred.  If return is QMI_SERVICE_ERR, the the qmi_err_code
  is valid and contains one of the QMI_SERVICE_ERR_* error codes.

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
qmi_wds_get_dormancy_status
(
  int                       user_handle,
  qmi_wds_dorm_status_type  *dorm_status,
  int                       *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  int rc;

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_DORMANCY_STATUS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Process response TLV's if there is no error */
  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr, *tmp_msg_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;

    /* Extract all TLV's */
    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
      switch (type)
      {
        case QMI_WDS_DORMANCY_STATUS_TLV_ID:
          {
            unsigned char temp;
            READ_8_BIT_VAL (value_ptr,temp);
            *dorm_status = (qmi_wds_dorm_status_type) temp;
            if ((*dorm_status != QMI_WDS_DORM_STATUS_DORMANT) &&
                (*dorm_status != QMI_WDS_DORM_STATUS_ACTIVE))
            {
              rc = QMI_INTERNAL_ERR;
            }
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_dormancy_status: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_dun_call_info
===========================================================================*/
/*!
@brief
  Gets DUN call state and also sets items for reporting in DUN indications.

@return
  QMI_NO_ERR (0) if no error occurred, one of the other QMI_*_ERR codes (< 0)
  if an error occurred.  If return is QMI_SERVICE_ERR, the the qmi_err_code
  is valid and contains one of the QMI_SERVICE_ERR_* error codes.

  cmd_info is an input paramter which indicates which current call info to
  retrieve as well as which fields to report in the DUN indication message

  rsp_info is an output parameter which contains the current call info requested
  in the command.

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
qmi_wds_get_dun_call_info
(
  int                             user_handle,
  qmi_wds_dun_call_info_cmd_type  *cmd_info,
  qmi_wds_dun_call_info_rsp_type  *rsp_info,
  int                             *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char     *tmp_msg_ptr;
  int               rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);


  /*Fill in the Mandatory current info mask TLV */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_DUN_CALL_INFO_REQ_INFO_TLV_ID,
                              4,
                              (void *)&cmd_info->curr_info_mask) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Fill in the optional indication configuration TLV if present... this should be moved
  ** to a separate function if/when more parameters are added
  */
  if (cmd_info->ind_info_mask & QMI_WDS_DUN_CALL_INFO_CONN_STATUS_IND_PARAM_MASK)
  {
    unsigned char tmp = (cmd_info->conn_status_rpt) ? 0x01 : 0x00;
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_DUN_CALL_INFO_CONN_STATUS_IND_TLV_ID,
                                1,
                                (void *)&tmp) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  if (cmd_info->ind_info_mask & QMI_WDS_DUN_CALL_INFO_TX_RX_STATS_IND_PARAM_MASK)
  {
    unsigned char     tmp_msg_buf[5];
    unsigned char     *tmp_msg_buf_ptr;
    tmp_msg_buf_ptr = &tmp_msg_buf[0];
    WRITE_8_BIT_VAL(tmp_msg_buf_ptr,cmd_info->tx_rx_stats_rpt.stats_period);
    WRITE_32_BIT_VAL(tmp_msg_buf_ptr,cmd_info->tx_rx_stats_rpt.stats_mask);
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_WDS_DUN_CALL_INFO_TX_RX_STATS_IND_TLV_ID,
                                sizeof(tmp_msg_buf)/sizeof(unsigned char),
                                (void *)tmp_msg_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  rc = qmi_service_send_msg_sync (user_handle,
                                QMI_WDS_SERVICE,
                                QMI_WDS_GET_DUN_CALL_INFO_MSG_ID,
                                QMI_SRVC_PDU_PTR(msg),
                                (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                msg,
                                &msg_size,
                                QMI_WDS_STD_MSG_SIZE,
                                QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                qmi_err_code);

  /* Initialize rsp_info curr_status bit mask */
  rsp_info->curr_info_mask = 0;

  /* Process response TLV's if there is no error */
  if (rc == QMI_NO_ERR)
  {
    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;
    memset(rsp_info, 0, sizeof(*rsp_info));

    /* Extract and process all TLV's */
    while (msg_size > 0)
    {
      unsigned long type, length, tmp_val;
      unsigned char *value_ptr;

      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_DUN_CALL_INFO_CONN_STATUS_RSP_TLV_ID:
          {
            rsp_info->curr_info_mask |= QMI_WDS_DUN_CALL_INFO_CONN_STATUS_RSP_PARAM_MASK;
            READ_8_BIT_VAL (value_ptr,tmp_val);
            rsp_info->conn_status.conn_state = (qmi_wds_dun_conn_state_type) tmp_val;

            READ_32_BIT_VAL (value_ptr,tmp_val);
            rsp_info->conn_status.duration[0] = tmp_val;
            READ_32_BIT_VAL (value_ptr,tmp_val);
            rsp_info->conn_status.duration[1] = tmp_val;
          }
          break;

        case QMI_WDS_DUN_CALL_INFO_TX_OK_BYTE_COUNT_RSP_TLV_ID:
          {
            rsp_info->curr_info_mask |= QMI_WDS_DUN_CALL_INFO_TX_OK_BYTE_COUNT_RSP_PARAM_MASK;
            memcpy(&rsp_info->tx_ok_byte_cnt,value_ptr,length);
          }
          break;
        case QMI_WDS_DUN_CALL_INFO_RX_OK_BYTE_COUNT_RSP_TLV_ID:
          {
            rsp_info->curr_info_mask |= QMI_WDS_DUN_CALL_INFO_RX_OK_BYTE_COUNT_RSP_PARAM_MASK;
            memcpy(&rsp_info->rx_ok_byte_cnt,value_ptr,length);
          }
          break;
        case QMI_WDS_DUN_CALL_INFO_LAST_CALL_TX_OK_BYTE_COUNT_RSP_TLV_ID:
          {
            rsp_info->curr_info_mask |= QMI_WDS_DUN_CALL_INFO_LAST_CALL_TX_OK_BYTE_COUNT_RSP_PARAM_MASK;
            memcpy(&rsp_info->last_call_tx_ok_byte_count,value_ptr,length);
          }
          break;
        case QMI_WDS_DUN_CALL_INFO_LAST_CALL_RX_OK_BYTE_COUNT_RSP_TLV_ID:
          {
            rsp_info->curr_info_mask |= QMI_WDS_DUN_CALL_INFO_LAST_CALL_RX_OK_BYTE_COUNT_RSP_PARAM_MASK;
            memcpy(&rsp_info->last_call_rx_ok_byte_count,value_ptr,length);
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_dun_call_info: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      } /* switch */
    } /* while */
  } /* if */

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_default_profile_number
===========================================================================*/
/*!
@brief
  This messages retrieves the default profile number configured on the wireless
  device for the specified technology

@return
  QMI_NO_ERR (0) if no error occurred, one of the other QMI_*_ERR codes (< 0)
  if an error occurred.  If return is QMI_SERVICE_ERR, the the qmi_err_code
  is valid and contains one of the QMI_SERVICE_ERR_* error codes.

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
qmi_wds_get_default_profile_number
(
  int                                            user_handle,
  qmi_wds_profile_tech_type                      profile_tech,
  qmi_wds_profile_family                         profile_family,
  unsigned char                                  *default_profile_number,
  int                                            *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char     *tmp_msg_ptr;
  int rc;
  unsigned char      tmp_msg_buf[2];

  if (!default_profile_number || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_get_default_profile_number:Bad input received");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  tmp_msg_buf[0] = (unsigned char)profile_tech;
  tmp_msg_buf[1] = (unsigned char)profile_family;

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_PROFILE_TECH_TYPE_TLV_ID,
                              sizeof(tmp_msg_buf)/sizeof(unsigned char),
                              (void *)&tmp_msg_buf[0]) < 0)
   {
     return QMI_INTERNAL_ERR;
   }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_DEFAULT_PROFILE_NUMBER_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Retreive the profile number if success */
  if (rc == QMI_NO_ERR || ( rc == QMI_SERVICE_ERR && *qmi_err_code == QMI_SERVICE_ERR_EXTENDED_INTERNAL))
  {
    unsigned long     type;
    unsigned long     length;
    unsigned char     *value_ptr;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                  &msg_size,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
      {
        rc = QMI_INTERNAL_ERR;
        break;
      }

      if (type == QMI_WDS_DEFAULT_PROFILE_NUMBER_RSP_TLV_ID)
      {
        unsigned char temp;
        READ_8_BIT_VAL (value_ptr,temp);
        *default_profile_number = temp;
        rc = QMI_NO_ERR;
      }
      else if (type == QMI_WDS_EXTENDED_ERROR_CODE_TLV_ID)
      {
        unsigned short temp;
        READ_16_BIT_VAL (value_ptr,temp);
        *qmi_err_code = temp;
        rc = QMI_EXTENDED_ERR;
      }
      else
      {
        QMI_DEBUG_MSG_1 ("qmi_wds_get_default_profile_number, unknown TLV returned = %x",(int)type);
      }
    }
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_set_default_profile_number
===========================================================================*/
/*!
@brief
  This messages sets the default profile number on the wireless device for
  the specifed technology

@return
  QMI_NO_ERR (0) if no error occurred, one of the other QMI_*_ERR codes (< 0)
  if an error occurred.  If return is QMI_SERVICE_ERR, the the qmi_err_code
  is valid and contains one of the QMI_SERVICE_ERR_* error codes.

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
qmi_wds_set_default_profile_number
(
  int                                            user_handle,
  qmi_wds_profile_tech_type                      profile_tech,
  qmi_wds_profile_family                         profile_family,
  unsigned char                                  profile_index,
  int                                            *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char     *tmp_msg_ptr;
  int rc;

  unsigned char      tmp_msg_buf[3];

  if (!qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_set_default_profile_number: Bad Input received.");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /*prepare the buffer*/
  tmp_msg_buf[0] = (unsigned char)profile_tech;
  tmp_msg_buf[1] = (unsigned char)profile_family;
  tmp_msg_buf[2] = (unsigned char)profile_index;

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_PROFILE_IDENTIFIER_TLV_ID,
                              sizeof(tmp_msg_buf),
                              (void *)&tmp_msg_buf[0]) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_SET_DEAFULT_PROFILE_NUMBER_MSG_ID,
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
    if (qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                       &msg_size,
                                       qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_set_default_profile_number: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }

 return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_reset_profile_to_default
===========================================================================*/
/*!
@brief
  This messages resets all the parameters of the given profile and technology
  to default values.

@return
  QMI_NO_ERR (0) if no error occurred, one of the other QMI_*_ERR codes (< 0)
  if an error occurred.  If return is QMI_SERVICE_ERR, the the qmi_err_code
  is valid and contains one of the QMI_SERVICE_ERR_* error codes.

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
qmi_wds_reset_profile_to_default
(
  int                                            user_handle,
  qmi_wds_profile_tech_type                      profile_tech,
  unsigned char                                  profile_index,
  int                                            *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char     *tmp_msg_ptr;
  int rc;


  unsigned char      tmp_msg_buf[2];

  if (!qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_reset_profile_to_default: Bad Input received.");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /*prepare the buffer*/
  tmp_msg_buf[0]  = (unsigned char)profile_tech;
  tmp_msg_buf[1]  = (unsigned char)profile_index;

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_PROFILE_RESET_PROFILE_TLV_ID,
                               sizeof(tmp_msg_buf)/sizeof(unsigned char),
                              (void *)&tmp_msg_buf[0]) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_RESET_PROFILE_TO_DEFAULT_MSG_ID,
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
      QMI_ERR_MSG_0("qmi_wds_reset_profile_to_default: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_reset_profile_param_invalid
===========================================================================*/
/*!
@brief
  This messages resets the given profile param type to invalid for the given
  technology

@return
  QMI_NO_ERR (0) if no error occurred, one of the other QMI_*_ERR codes (< 0)
  if an error occurred.  If return is QMI_SERVICE_ERR, the the qmi_err_code
  is valid and contains one of the QMI_SERVICE_ERR_* error codes.
  TODO: Add information about Extended error codes.

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
qmi_wds_reset_profile_param_invalid
(
  int                                            user_handle,
  qmi_wds_profile_tech_type                      profile_tech,
  unsigned char                                  profile_index,
  qmi_wds_reset_profile_param_type               profile_param_id,
  int                                            *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char     *tmp_msg_ptr;
  int rc;
  unsigned char      tmp_msg_buf[6];
  unsigned char      *tmp_msg_buf_ptr =  tmp_msg_buf;

  if (!qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_reset_profile_param_invalid: Bad Input received.");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /*prepare the buffer*/
  WRITE_8_BIT_VAL(tmp_msg_buf_ptr,profile_tech);
  WRITE_8_BIT_VAL(tmp_msg_buf_ptr,profile_index);
  WRITE_32_BIT_VAL(tmp_msg_buf_ptr,profile_param_id);

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_RESET_PROFILE_PARAMT_TLV_ID,
                              sizeof(tmp_msg_buf)/sizeof(unsigned char),
                              (void *)tmp_msg_buf_ptr) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_RESET_PROFILE_PARAM_TO_INVALID_MSG_ID,
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
    if (qmi_wds_util_read_ext_err_code(&tmp_msg_ptr,
                                       &msg_size,
                                       qmi_err_code) == QMI_INTERNAL_ERR)
    {
      QMI_ERR_MSG_0("qmi_wds_reset_profile_param_invalid: Failed to read the extended error response");
      rc = QMI_INTERNAL_ERR;
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_set_client_ip_pref
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
qmi_wds_set_client_ip_pref
(
  int                          user_handle,
  qmi_ip_family_pref_type      ip_family,
  int                         *qmi_err_code
)
{
  unsigned char msg[QMI_WDS_STD_MSG_SIZE];
  int msg_size;
  unsigned char *pdu;

  if (!qmi_err_code)
  {
    QMI_ERR_MSG_1("qmi_wds_set_client_ip_pref bad parameters qmi_err_code=%p",
                  qmi_err_code);
    return QMI_INTERNAL_ERR;
  }

  pdu = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  if (qmi_util_write_std_tlv(&pdu,
                             &msg_size,
                             QMI_WDS_CLIENT_IP_PREF_REQ_TLV_ID,
                             1,
                             &ip_family) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return qmi_service_send_msg_sync(user_handle,
                                   QMI_WDS_SERVICE,
                                   QMI_WDS_SET_CLIENT_IP_FAMILY_PREF_MSG_ID,
                                   QMI_SRVC_PDU_PTR(msg),
                                   (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                   msg,
                                   &msg_size,
                                   QMI_WDS_STD_MSG_SIZE,
                                   QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                   qmi_err_code);
}

/*===========================================================================
  FUNCTION  qmi_wds_set_mip_mode
===========================================================================*/
/*!
@brief
  This command sets the current Mobile IP setting for the device.

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
qmi_wds_set_mip_mode
(
  int                             user_handle,
  qmi_wds_mobile_ip_mode_type     mip_mode,
  int                             *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char     *tmp_msg_ptr;
  int rc;


  if (!qmi_err_code || mip_mode < QMI_WDS_MOBILE_IP_OFF ||
                              mip_mode >= QMI_WDS_MOBILE_IP_MAX)
  {
    QMI_ERR_MSG_0("qmi_wds_set_mip_mode: bad Input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Write TLV data from above in main message buffer */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_MOBILE_IP_MODE_TLV_ID,
                              1,
                              (void *)&mip_mode) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_SET_MOBILE_IP_MODE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;

}


/*===========================================================================
  FUNCTION  qmi_wds_get_mip_mode
===========================================================================*/
/*!
@brief
  This command gets the current Mobile IP setting for the device.

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
qmi_wds_get_mip_mode
(
  int                             user_handle,
  qmi_wds_mobile_ip_mode_type     *mip_mode_resp,
  int                             *qmi_err_code
)
{
  unsigned char    msg[QMI_WDS_STD_MSG_SIZE];
  int              msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;


  if (!qmi_err_code || !mip_mode_resp)
  {
    QMI_ERR_MSG_0("qmi_wds_get_mip_mode: bad Input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_MOBILE_IP_MODE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  *mip_mode_resp = QMI_WDS_MOBILE_IP_MODE_INVALID;

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Process response TLV's if there is no error */
  if (rc == QMI_NO_ERR)
  {
    /* Extract and process all TLV's */
    while (msg_size > 0)
    {
      unsigned long type;
      unsigned long length;
      unsigned char *value_ptr;

      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_MOBILE_IP_MODE_TLV_ID:
          {
            unsigned char tmp;
            READ_8_BIT_VAL (value_ptr,tmp);
            *mip_mode_resp = (qmi_wds_mobile_ip_mode_type) tmp;
          }
          break;
          default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_mip_mode: unknown response TLV type "
                           "= %x",(unsigned int)type);
          }
          break;
      }/*Switch*/
    }/*While*/
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_get_active_mip_profile
===========================================================================*/
/*!
@brief
  This command queries the current mobile IP profile index for the device.

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
qmi_wds_get_active_mip_profile
(
  int                             user_handle,
  unsigned char                   *mip_profile_index,
  int                             *qmi_err_code
)
{
  unsigned char    msg[QMI_WDS_STD_MSG_SIZE];
  int              msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;


  if (!qmi_err_code || !mip_profile_index)
  {
    QMI_ERR_MSG_0("qmi_wds_get_active_mip_profile: bad Input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_ACTIVE_MOBILE_IP_PROFILE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Process response TLV's if there is no error */
  if (rc == QMI_NO_ERR)
  {
    /* Extract and process all TLV's */
    while (msg_size > 0)
    {
      unsigned long type;
      unsigned long length;
      unsigned char *value_ptr;

      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_MOBILE_IP_PROFILE_INDEX_TLV_ID:
          {
            unsigned char tmp;
            READ_8_BIT_VAL (value_ptr,tmp);
            *mip_profile_index =  tmp;
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_active_mip_profile: unknown response TLV type "
                           "= %x",(unsigned int)type);
          }
          break;
      }/*Switch*/
    }/*While*/
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_set_active_mip_profile
===========================================================================*/
/*!
@brief
  This command sets the current mobile IP profile index for the device.

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
qmi_wds_set_active_mip_profile
(
  int                             user_handle,
  srvc_programming_code_type      service_code,
  unsigned char                   profile_index,
  int                             *qmi_err_code
)
{

  unsigned char    msg[QMI_WDS_STD_MSG_SIZE];
  int              msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;
  unsigned char    tmp_msg_buf[QMI_WDS_MIP_SERVICE_PROGRAMMING_CODE_SIZE +
                                                        sizeof(unsigned char)];
  unsigned char    *tmp_msg_buf_ptr =  tmp_msg_buf;

  if (!qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_set_active_mip_profile: bad Input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /*prepare the non-optional TLVbuffer*/
  memcpy(tmp_msg_buf_ptr, service_code, QMI_WDS_MIP_SERVICE_PROGRAMMING_CODE_SIZE);
  tmp_msg_buf_ptr += QMI_WDS_MIP_SERVICE_PROGRAMMING_CODE_SIZE;
  WRITE_8_BIT_VAL(tmp_msg_buf_ptr, profile_index);

  /* Write TLV data from above in main message buffer */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_MOBILE_IP_PROFILE_INDEX_TLV_ID,
                              QMI_WDS_MIP_SERVICE_PROGRAMMING_CODE_SIZE +
                                                        sizeof(unsigned char),
                              (void *)tmp_msg_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_SET_ACTIVE_MOBILE_IP_PROFILE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);


  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_read_mip_profile
===========================================================================*/
/*!
@brief
  This command queries a mobile IP profile from the device.

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
qmi_wds_read_mip_profile
(
  int                                         user_handle,
  unsigned char                               profile_index,
  qmi_wds_read_mobile_ip_profile_resp_type    *profile_resp,
  int                                         *qmi_err_code
)
{

  unsigned char    msg[QMI_WDS_STD_MSG_SIZE];
  int              msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!qmi_err_code || !profile_resp)
  {
    QMI_ERR_MSG_0("qmi_wds_read_mip_profile: bad Input parameter\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_MOBILE_IP_PROFILE_INDEX_TLV_ID,
                              sizeof(unsigned char),
                              (void *)&profile_index) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_READ_MOBILE_IP_PROFILE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Process response TLV's if there is no error */
  if (rc == QMI_NO_ERR)
  {
    /* Extract and process all TLV's */
    while (msg_size > 0)
    {
      unsigned long type, length, tmp_val;
      unsigned char *value_ptr;

      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_READ_MOBILE_IP_PROFILE_HA_KEY_STATE_TLV_ID:
          {
            profile_resp->param_mask = QMI_WDS_READ_MIP_PROFILE_HA_KEY_PARAM_MASK;
            unsigned char tmp;
            READ_8_BIT_VAL (value_ptr,tmp);
            profile_resp->ha_key_state = tmp;
          }
          break;
        case QMI_WDS_READ_MOBILE_IP_PROFILE_AAA_KEY_STATE_TLV_ID:
          {
            profile_resp->param_mask = QMI_WDS_READ_MIP_PROFILE_AAA_KEY_PARAM_MASK;
            unsigned char tmp;
            READ_8_BIT_VAL (value_ptr,tmp);
            profile_resp->aaa_key_state = tmp;
          }
          break;

        case QMI_WDS_READ_MOBILE_IP_PROFILE_NAI_TLV_ID:
        {
         profile_resp->param_mask = QMI_WDS_READ_MIP_PROFILE_NAI_PARAM_MASK;

          /* String starts at value pointer and is length bytes long */
          if (length <= QMI_WDS_MOBILE_IP_PROFILE_MAX_STR_SIZE)
          {
            profile_resp->nai_len = (unsigned char) length;
            memcpy (profile_resp->nai,(void *)value_ptr, length);
          }
          else
          {
            QMI_ERR_MSG_1 ("qmi_wds_read_mip_profile: NAI length too long %d ",
                           (unsigned int)length);
          }
        }
        break;

        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_read_mip_profile: unknown response TLV type "
                           "= %x",(unsigned int)type);
          }
          break;
      }/*Switch*/
    }/*While*/
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_modify_mip_profile
===========================================================================*/
/*!
@brief
  This command modifies a mobile IP profile on the device.

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
qmi_wds_modify_mip_profile
(
  int                                         user_handle,
  srvc_programming_code_type                  service_code,
  unsigned char                               profile_index,
  qmi_wds_modify_mobile_ip_profile_req_type   *modify_profile_req,
  int                                         *qmi_err_code
)
{

  unsigned char    msg[QMI_WDS_STD_MSG_SIZE];
  int              msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;
  unsigned char    tmp_msg_buf[QMI_WDS_MIP_SERVICE_PROGRAMMING_CODE_SIZE +
                                                        sizeof(unsigned char)];
  unsigned char    *msg_ptr =  tmp_msg_buf;

  if (!qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_modify_mip_profile: bad Input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /*prepare the non-optional TLVbuffer*/
  memcpy(msg_ptr, service_code, QMI_WDS_MIP_SERVICE_PROGRAMMING_CODE_SIZE);
  msg_ptr += QMI_WDS_MIP_SERVICE_PROGRAMMING_CODE_SIZE;
  WRITE_8_BIT_VAL(msg_ptr, profile_index);

  /* Write TLV data from above in main message buffer */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_MOBILE_IP_PROFILE_INDEX_TLV_ID,
                              QMI_WDS_MIP_SERVICE_PROGRAMMING_CODE_SIZE +
                                                        sizeof(unsigned char),
                              (void *)tmp_msg_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  /* Write profile params (optional) to TLV buffer */
  if (modify_profile_req)
  {
    if (qmi_wds_write_mobile_ip_optional_profile_tlvs (&tmp_msg_ptr,
                                                       &msg_size,
                                                       modify_profile_req) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_MODIFY_MOBILE_IP_PROFILE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);


  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_pref_data_system
===========================================================================*/
/*!
@brief
  This message queries the current preferred data system.

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
qmi_wds_get_pref_data_system
(
  int                         user_handle,
  qmi_wds_pref_data_sys_type  *cur_pref_sys,
  int                         *qmi_err_code
)
{
  unsigned char    msg[QMI_WDS_STD_MSG_SIZE];
  int              msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!qmi_err_code || !cur_pref_sys)
  {
    QMI_ERR_MSG_0("qmi_wds_get_pref_data_system: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_PREF_DATA_SYS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Initialize cur_pref_sys to UNKNOWN */
  *cur_pref_sys = QMI_WDS_DATA_SYS_UNKNOWN;

  /* Process response TLV's if there is no error */
  if (rc == QMI_NO_ERR)
  {
    /* Extract and process all TLV's */
    while (msg_size > 0)
    {
      unsigned long type;
      unsigned long length;
      unsigned char *value_ptr;

      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_GET_PREF_DATA_SYS_RESP_CUR_SYS_TLV_ID:
          {
            unsigned long tmp;

            READ_32_BIT_VAL (value_ptr, tmp);
            *cur_pref_sys = (qmi_wds_pref_data_sys_type) tmp;
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_pref_data_system: unknown response TLV type "
                           "= %x",(unsigned int)type);
          }
          break;
      }/*Switch*/
    }/*While*/
  }

  return rc;
}


/*===========================================================================
  FUNCTION  qmi_wds_get_current_data_system_status
===========================================================================*/
/*!
@brief
  This message queries the current data system status.

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@param
  user_handle  [in]       - QMI user handle
  pref_network [out]      - Preferred network
  network_info [in]       - Pointer to an array for storing the network info
                            data
  network_info_len[inout] - Size of network_info array
  qmi_err_code [out]      - QMI error code when return value < 0

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - It is assumed that sufficient memory is allocated by the client for the
    network_info buffer. The number of entries actually stored is returned
    via network_info_len

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_wds_get_current_data_system_status
(
  int                                        user_handle,
  qmi_wds_data_network_type                  *pref_network,
  qmi_wds_data_sys_status_network_info_type  *network_info,
  unsigned int                               *network_info_len,
  int                                        *qmi_err_code
)
{
  unsigned char    msg[QMI_WDS_STD_MSG_SIZE];
  int              msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!pref_network || !network_info || !network_info_len || *network_info_len <= 0 || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_get_current_data_system_status: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_CURRENT_DATA_SYS_STATUS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Initialize output params to invalid */
  *pref_network = QMI_WDS_DATA_NETWORK_TYPE_INVALID;
  memset(network_info,
         0,
         sizeof(qmi_wds_data_sys_status_network_info_type)*(*network_info_len));

  /* Process response TLV's if there is no error */
  if (rc == QMI_NO_ERR)
  {
    /* Extract and process all TLV's */
    while (msg_size > 0)
    {
      unsigned long type;
      unsigned long length;
      unsigned char *value_ptr;

      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_GET_CURRENT_DATA_SYS_STATUS_TLV_ID:
          {
            rc = qmi_wds_srvc_process_data_sys_status_tlvs(&value_ptr,
                                                           pref_network,
                                                           &network_info,
                                                           network_info_len);

            if (QMI_NO_ERR != rc)
            {
              QMI_ERR_MSG_1 ("Call to qmi_wds_srvc_process_data_sys_status_tlvs() failed err=%d",
                             rc);
            }
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_pref_data_system: unknown response TLV type "
                           "= %x",(unsigned int)type);
          }
          break;
      }/*Switch*/
    }/*While*/
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_lte_max_attach_pdn_num
===========================================================================*/
/*!
@brief
  This message is used to query the maximum number of attached PDNs supported

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
qmi_wds_get_lte_max_attach_pdn_num
(
  int  user_handle,
  int  *max_attach_pdns,
  int  *qmi_err_code
)
{
  unsigned char    msg[QMI_WDS_STD_MSG_SIZE];
  int              msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!qmi_err_code || !max_attach_pdns)
  {
    QMI_ERR_MSG_0("qmi_wds_get_lte_max_attach_pdn_num: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_LTE_MAX_ATTACH_PDN_NUM_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Initialize max_attach_pdns */
  *max_attach_pdns = 0;

  /* Process response TLV's if there is no error */
  if (rc == QMI_NO_ERR)
  {
    /* Extract and process all TLV's */
    while (msg_size > 0)
    {
      unsigned long type;
      unsigned long length;
      unsigned char *value_ptr;

      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_GET_LTE_MAX_NUM_ATTACH_PDNS_TLV_ID:
          {
            unsigned char tmp8;

            READ_8_BIT_VAL (value_ptr, tmp8);
            *max_attach_pdns = tmp8;
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_lte_max_attach_pdn_num: unknown response TLV type "
                           "= %x",(unsigned int)type);
          }
          break;
      }/*Switch*/
    }/*While*/
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_set_lte_attach_pdn_list
===========================================================================*/
/*!
@brief
  This message is used to set the LTE attach PDN list

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
qmi_wds_set_lte_attach_pdn_list
(
  int                               user_handle,
  qmi_wds_lte_attach_pdn_list_type  *attach_pdn_list,
  int                               *qmi_err_code
)
{
  unsigned char  msg[QMI_WDS_STD_MSG_SIZE];
  int            msg_size;
  unsigned char  *tmp_msg_ptr;

  unsigned char  tmp_msg_buf[QMI_WDS_MAX_LTE_ATTACH_PDN_LIST_SIZE * sizeof(unsigned short) +
                             sizeof(unsigned char)];
  unsigned char  *tmp_msg_buf_ptr =  tmp_msg_buf;
  unsigned char  tmp8;
  unsigned short tmp16;
  int i, rc;

  if (!qmi_err_code ||
      !attach_pdn_list ||
      attach_pdn_list->len < 0 ||
      attach_pdn_list->len > QMI_WDS_MAX_LTE_ATTACH_PDN_LIST_SIZE)
  {
    QMI_ERR_MSG_0("qmi_wds_set_lte_attach_pdn_list: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Prepare the non-optional TLVbuffer */
  tmp8 = (unsigned char) attach_pdn_list->len;
  WRITE_8_BIT_VAL(tmp_msg_buf_ptr, tmp8);

  for (i = 0; i < attach_pdn_list->len; ++i)
  {
    tmp16 = (unsigned short) attach_pdn_list->list[i];
    WRITE_16_BIT_VAL(tmp_msg_buf_ptr, tmp16);
  }

  /* Write TLV data from above in main message buffer */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_SET_LTE_ATTACH_PDN_LIST_TLV_ID,
                              (unsigned long)(tmp_msg_buf_ptr-tmp_msg_buf),
                              (void *)tmp_msg_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_SET_LTE_ATTACH_PDN_LIST_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_lte_attach_pdn_list
===========================================================================*/
/*!
@brief
  This message is used to query the LTE attach PDN list

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
qmi_wds_get_lte_attach_pdn_list
(
  int                               user_handle,
  qmi_wds_lte_attach_pdn_list_type  *attach_pdn_list,
  int                               *qmi_err_code
)
{

  unsigned char    msg[QMI_WDS_STD_MSG_SIZE];
  int              msg_size;
  unsigned char    *tmp_msg_ptr;
  int i, rc;

  if (!qmi_err_code || !attach_pdn_list)
  {
    QMI_ERR_MSG_0("qmi_wds_get_lte_attach_pdn_list: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_LTE_ATTACH_PDN_LIST_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Initialize attach_pdn_list length */
  attach_pdn_list->len = 0;

  /* Process response TLV's if there is no error */
  if (rc == QMI_NO_ERR)
  {
    /* Extract and process all TLV's */
    while (msg_size > 0)
    {
      unsigned long type;
      unsigned long length;
      unsigned char *value_ptr;

      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_GET_LTE_ATTACH_PDN_LIST_TLV_ID:
          {
            unsigned char tmp8;
            unsigned short tmp16;

            READ_8_BIT_VAL (value_ptr, tmp8);

            if (tmp8 > QMI_WDS_MAX_LTE_ATTACH_PDN_LIST_SIZE)
            {
              QMI_ERR_MSG_2 ("qmi_wds_get_lte_attach_pdn_list: rcvd list length=%d exceeded max supported=%d",
                             (int) tmp8,
                             QMI_WDS_MAX_LTE_ATTACH_PDN_LIST_SIZE);
              return QMI_INTERNAL_ERR;
            }

            attach_pdn_list->len = tmp8;
            for (i = 0; i < attach_pdn_list->len; ++i)
            {
              READ_16_BIT_VAL (value_ptr, tmp16);
              attach_pdn_list->list[i] = tmp16;
            }
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_lte_attach_pdn_list: unknown response TLV type "
                           "= %x",(unsigned int)type);
          }
          break;
      }/*Switch*/
    }/*While*/
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_lte_attach_params
===========================================================================*/
/*!
@brief
  This message is used to query the LTE attach Params

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
qmi_wds_get_lte_attach_params
(
  int                        user_handle,
  qmi_wds_lte_attach_params  *attach_params,
  int                        *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  int rc;

  if (attach_params == NULL)
  {
    return QMI_INTERNAL_ERR;
  }
  memset(attach_params, 0x00, sizeof(qmi_wds_lte_attach_params));

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Send message */
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_LTE_ATTACH_PARAMS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Get route look_up information */
  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr, *tmp_msg_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_GET_LTE_ATTACH_PARAMS_APN_STRING_TLV_ID:
        {
          /* String starts at value pointer and is length bytes long */
          if (length > 0)
          {
            int cpy_len = (length < QMI_WDS_MAX_APN_NAME_SIZE)
                           ? length : (QMI_WDS_MAX_APN_NAME_SIZE -1);
            memcpy (attach_params->apn_string, (void *)value_ptr, cpy_len);
            attach_params->apn_string[cpy_len] = '\0';
            attach_params->apn_string_valid = TRUE;
          }
          else
          {
            attach_params->apn_string[0] = '\0';
            attach_params->apn_string_valid = FALSE;
          }
        }
        break;
        case QMI_WDS_GET_LTE_ATTACH_PARAMS_IP_TYPE_TLV_ID:
        {
          READ_8_BIT_VAL(value_ptr,attach_params->ip_type);
          attach_params->ip_type_valid = TRUE;
        }
        break;
        case QMI_WDS_GET_LTE_ATTACH_PARAMS_OTA_ATTACH_PERFORMED_TLV_ID:
        {
          READ_8_BIT_VAL (value_ptr,attach_params->ota_attach_performed);
          attach_params->ota_attach_performed_valid = TRUE;
        }
        break;
        default:
        {
          QMI_ERR_MSG_1 ("qmi_wds_get_lte_attach_params: unknown response "
                         "TLV type = %x",(unsigned int)type);
        }
        break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_set_lte_data_retry
===========================================================================*/
/*!
@brief
  This message is used to enable or disable retry on LTE data attach

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
qmi_wds_set_lte_data_retry
(
  int  user_handle,
  int  lte_data_retry,
  int  *qmi_err_code
)
{
  unsigned char    msg[QMI_WDS_STD_MSG_SIZE];
  int              msg_size;
  unsigned char    *tmp_msg_ptr;
  unsigned char    tmp8;
  int rc;

  if (!qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_set_lte_data_retry: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  tmp8 = (unsigned char) lte_data_retry;

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_SET_LTE_DATA_RETRY_TLV_ID,
                              sizeof(unsigned char),
                              (void *)&tmp8) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_SET_LTE_DATA_RETRY_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_lte_data_retry
===========================================================================*/
/*!
@brief
  This message is used to retrieve the current LTE data retry setting

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
qmi_wds_get_lte_data_retry
(
  int  user_handle,
  int  *lte_data_retry,
  int  *qmi_err_code
)
{
  unsigned char    msg[QMI_WDS_STD_MSG_SIZE];
  int              msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!qmi_err_code || !lte_data_retry)
  {
    QMI_ERR_MSG_0("qmi_wds_get_lte_data_retry: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_LTE_DATA_RETRY_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Initialize lte_data_retry */
  *lte_data_retry = FALSE;

  /* Process response TLV's if there is no error */
  if (rc == QMI_NO_ERR)
  {
    /* Extract and process all TLV's */
    while (msg_size > 0)
    {
      unsigned long type;
      unsigned long length;
      unsigned char *value_ptr;

      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_GET_LTE_DATA_RETRY_TLV_ID:
          {
            unsigned char tmp8;

            READ_8_BIT_VAL (value_ptr, tmp8);
            *lte_data_retry = tmp8;
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_lte_data_retry: unknown response TLV type "
                           "= %x",(unsigned int)type);
          }
          break;
      }/*Switch*/
    }/*While*/
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_set_lte_attach_type
===========================================================================*/
/*!
@brief
  This message is used to set whether the attach to be performed is
  initial or handoff

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
qmi_wds_set_lte_attach_type
(
  int                      user_handle,
  qmi_wds_lte_attach_type  lte_attach_type,
  int                      *qmi_err_code
)
{
  unsigned char    msg[QMI_WDS_STD_MSG_SIZE];
  int              msg_size;
  unsigned char    *tmp_msg_ptr;
  unsigned long    tmp32;
  int rc;

  if (!qmi_err_code ||
      lte_attach_type <= QMI_WDS_LTE_ATTACH_TYPE_INVALID ||
      lte_attach_type >= QMI_WDS_LTE_ATTACH_TYPE_MAX)
  {
    QMI_ERR_MSG_0("qmi_wds_set_lte_attach_type: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  tmp32 = (unsigned long) lte_attach_type;

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_SET_LTE_ATTACH_TYPE_TLV_ID,
                              4,
                              (void *)&tmp32) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_SET_LTE_ATTACH_TYPE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_lte_attach_type
===========================================================================*/
/*!
@brief
  This message is used to retrieve the current LTE attach type

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
qmi_wds_get_lte_attach_type
(
  int                      user_handle,
  qmi_wds_lte_attach_type  *lte_attach_type,
  int                      *qmi_err_code
)
{
  unsigned char    msg[QMI_WDS_STD_MSG_SIZE];
  int              msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!qmi_err_code || !lte_attach_type)
  {
    QMI_ERR_MSG_0("qmi_wds_get_lte_attach_type: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_LTE_ATTACH_TYPE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Initialize lte_attach_type */
  *lte_attach_type = QMI_WDS_LTE_ATTACH_TYPE_INVALID;

  /* Process response TLV's if there is no error */
  if (rc == QMI_NO_ERR)
  {
    /* Extract and process all TLV's */
    while (msg_size > 0)
    {
      unsigned long type;
      unsigned long length;
      unsigned char *value_ptr;

      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_GET_LTE_ATTACH_TYPE_TLV_ID:
          {
            unsigned long tmp32;

            READ_32_BIT_VAL (value_ptr, tmp32);
            *lte_attach_type = (qmi_wds_lte_attach_type) tmp32;
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_lte_attach_type: unknown response TLV type "
                           "= %x",(unsigned int)type);
          }
          break;
      }/*Switch*/
    }/*While*/
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_rev_ip_transport_conn_ind_registration
===========================================================================*/
/*!
@brief
  This message is used to register for indications on the reverse adapter
  connection.

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
qmi_wds_rev_ip_transport_conn_ind_registration
(
  int                                       user_handle,
  qmi_wds_rev_ip_transport_reg_status_type  reg_status,
  int                                       *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char     *tmp_msg_ptr;
  unsigned char     reg_dereg;
  int rc;


  if (!qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_rev_ip_transport_conn_ind_registration: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  reg_dereg = (unsigned char) reg_status;

  /* Write TLV data from above in main message buffer */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_REV_IP_TRANS_CONN_IND_REG_TLV_ID,
                              1,
                              (void *)&reg_dereg) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_REV_IP_TRANS_CONN_IND_REG_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}


/* Utility macro for reading the IP address in SA Config */
#define QMI_WDS_UTIL_READ_SA_CONFIG_IP_ADDR(value_ptr, v4v6addr) \
do                                                               \
{                                                                \
  int i;                                                         \
  unsigned long temp_32bit;                                      \
  unsigned char temp_8bit;                                       \
                                                                 \
  READ_32_BIT_VAL((value_ptr), temp_32bit);                      \
  (v4v6addr).family = (qmi_wds_ip_family) temp_32bit;            \
                                                                 \
  /* Read the address length */                                  \
  READ_8_BIT_VAL((value_ptr), temp_8bit);                        \
                                                                 \
  if (QMI_WDS_IPV4_TYPE == (v4v6addr).family)                    \
  {                                                              \
    if (QMI_WDS_IPV4_ADDR_SIZE_IN_BYTES != temp_8bit)            \
    {                                                            \
      return QMI_INTERNAL_ERR;                                   \
    }                                                            \
    /* Read the IPv4 address */                                  \
    READ_32_BIT_VAL((value_ptr), (v4v6addr).addr.ipv4);          \
  }                                                              \
  else if (QMI_WDS_IPV6_TYPE == (v4v6addr).family)               \
  {                                                              \
    if (QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES != temp_8bit)            \
    {                                                            \
      return QMI_INTERNAL_ERR;                                   \
    }                                                            \
    /* Read the IPv6 address */                                  \
    for (i = 0; i < QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES; i++)        \
    {                                                            \
      READ_8_BIT_VAL ((value_ptr), (v4v6addr).addr.ipv6[i]);     \
    }                                                            \
  }                                                              \
  else                                                           \
  {                                                              \
    return QMI_INTERNAL_ERR;                                     \
  }                                                              \
}                                                                \
while (0)

/* Utility macro for reading the cfg attr IPV4 address */
#define QMI_WDS_UTIL_READ_CFG_ATTR_IPV4_ADDR(val_ptr, cfg_ptr, mem) \
do                                                                  \
{                                                                   \
  int num_ele, i;                                                   \
  unsigned char tmp_8bit;                                           \
  unsigned long tmp_32bit;                                          \
                                                                    \
  READ_8_BIT_VAL(val_ptr, tmp_8bit);                                \
  num_ele = (tmp_8bit < QMI_WDS_IPSEC_CFG_ATTR_MAX_ATTRS) ?         \
            tmp_8bit :                                              \
            QMI_WDS_IPSEC_CFG_ATTR_MAX_ATTRS;                       \
                                                                    \
  for (i = 0; i < num_ele; ++i)                                     \
  {                                                                 \
    READ_32_BIT_VAL(val_ptr, tmp_32bit);                            \
    cfg_ptr->mem.addr[i] = (ipv4_addr_type) tmp_32bit;              \
  }                                                                 \
  cfg_ptr->mem.num_ele = num_ele;                                   \
}                                                                   \
while (0)

/* Utility macro for reading the cfg attr IPV6 address */
#define QMI_WDS_UTIL_READ_CFG_ATTR_IPV6_ADDR(val_ptr, cfg_ptr, mem)      \
do                                                                       \
{                                                                        \
  int num_ele, i, j;                                                     \
  unsigned char tmp_8bit;                                                \
  unsigned long tmp_32bit;                                               \
                                                                         \
  READ_8_BIT_VAL(val_ptr, tmp_8bit);                                     \
  num_ele = (tmp_8bit < QMI_WDS_IPSEC_CFG_ATTR_MAX_ATTRS) ?              \
            tmp_8bit :                                                   \
            QMI_WDS_IPSEC_CFG_ATTR_MAX_ATTRS;                            \
                                                                         \
  for (i = 0; i < num_ele; ++i)                                          \
  {                                                                      \
    for (j = 0; j < QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES; ++j)                \
    {                                                                    \
      READ_8_BIT_VAL (val_ptr, cfg_ptr->mem.addr[i].ipv6_addr[j]);       \
    }                                                                    \
                                                                         \
    /* Read V6 prefix length */                                          \
    READ_8_BIT_VAL (val_ptr, cfg_ptr->mem.addr[i].ipv6_prefix_len);      \
                                                                         \
  }                                                                      \
  cfg_ptr->mem.num_ele = num_ele;                                        \
}                                                                        \
while (0)

/*===========================================================================
  FUNCTION  qmi_wds_process_ipsec_cfg_attr_tlvs
===========================================================================*/
/*!
@brief
  Takes an input RX buffer containing IPSec SA config attributes TLV info and
  translates to qmi_wds_ipsec_cfg_attr_type structure

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Changes input buffer pointer
*/
/*=========================================================================*/
int qmi_wds_process_ipsec_cfg_attr_tlvs
(
  unsigned long                type,
  unsigned char                **value_ptr,
  qmi_wds_ipsec_cfg_attr_type  *cfg_attr
)
{
  int rc = QMI_NO_ERR;
  unsigned char *tmp_value_ptr = NULL;
  unsigned long temp_32bit;
  unsigned short temp_16bit;
  unsigned char temp_8bit;
  int i, num_ele;

  if (!value_ptr || !*value_ptr || !cfg_attr)
  {
    QMI_ERR_MSG_0("qmi_wds_process_ipsec_cfg_attr_tlvs: bad input parameters\n");
    goto bail;
  }

  tmp_value_ptr = *value_ptr;

  switch (type)
  {
     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_ADDR_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_IPV4_ADDR_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV4_ADDR(tmp_value_ptr, cfg_attr, ipv4_addr);
      break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_MASK_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_IPV4_NETMASK_PARAM_MASK;
       READ_32_BIT_VAL(tmp_value_ptr, cfg_attr->ipv4_netmask);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_DNS_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_IPV4_DNS_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV4_ADDR(tmp_value_ptr, cfg_attr, ipv4_dns);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_NBNS_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_IPV4_NBNS_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV4_ADDR(tmp_value_ptr, cfg_attr, ipv4_nbns);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_ADDR_EXPIRY_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_IPV4_ADDR_EXPIRY_PARAM_MASK;
       READ_32_BIT_VAL(tmp_value_ptr, cfg_attr->addr_expiry);
       break;


     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_DHCP_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_IPV4_DHCP_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV4_ADDR(tmp_value_ptr, cfg_attr, ipv4_dhcp);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_APP_VER_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_APP_VERSION_PARAM_MASK;
       READ_8_BIT_VAL(tmp_value_ptr, temp_8bit);
       cfg_attr->app_version.num_valid = (temp_8bit < QMI_WDS_IPSEC_MAX_CFG_DATA_SIZE) ?
                                         temp_8bit :
                                         QMI_WDS_IPSEC_MAX_CFG_DATA_SIZE;
       for (i = 0; i < cfg_attr->app_version.num_valid; ++i)
       {
         READ_8_BIT_VAL(tmp_value_ptr, cfg_attr->app_version.data[i]);
       }
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_ADDR_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_IPV6_ADDR_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV6_ADDR(tmp_value_ptr, cfg_attr, ipv6_addr);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_DNS_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_IPV6_DNS_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV6_ADDR(tmp_value_ptr, cfg_attr, ipv6_dns);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_NBNS_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_IPV6_NBNS_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV6_ADDR(tmp_value_ptr, cfg_attr, ipv6_nbns);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_DHCP_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_IPV6_DHCP_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV6_ADDR(tmp_value_ptr, cfg_attr, ipv6_dhcp);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_SUBNET_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_IPV4_SUBNET_PARAM_MASK;
       READ_8_BIT_VAL(tmp_value_ptr, temp_8bit);
       cfg_attr->ipv4_subnet.num_valid = (temp_8bit < QMI_WDS_IPSEC_MAX_CFG_DATA_SIZE) ?
                                         temp_8bit :
                                         QMI_WDS_IPSEC_MAX_CFG_DATA_SIZE;
       for (i = 0; i < cfg_attr->ipv4_subnet.num_valid; ++i)
       {
         READ_32_BIT_VAL(tmp_value_ptr, cfg_attr->ipv4_subnet.addr[i].ipv4_addr);
         READ_32_BIT_VAL(tmp_value_ptr, cfg_attr->ipv4_subnet.addr[i].subnet_mask);
       }
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_SUPP_ATTR_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_SUPPORTED_ATTR_PARAM_MASK;
       READ_8_BIT_VAL(tmp_value_ptr, temp_8bit);
       cfg_attr->supported_attrs.num_valid = (temp_8bit < QMI_WDS_IPSEC_MAX_CFG_DATA_SIZE) ?
                                             temp_8bit :
                                             QMI_WDS_IPSEC_MAX_CFG_DATA_SIZE;
       for (i = 0; i < cfg_attr->supported_attrs.num_valid; ++i)
       {
         READ_8_BIT_VAL(tmp_value_ptr, cfg_attr->supported_attrs.data[i]);
       }
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_SUBNET_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_IPV6_SUBNET_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV6_ADDR(tmp_value_ptr, cfg_attr, ipv6_subnet);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_PCSCF_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_PCSCF_IPV4_ADDR_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV4_ADDR(tmp_value_ptr, cfg_attr, ipv4_pcscf);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_PCSCF_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_PCSCF_IPV6_ADDR_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV6_ADDR(tmp_value_ptr, cfg_attr, ipv6_pcscf);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_3GPP2_IPV4_HA_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_3GPP2_MIP4_HA_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV4_ADDR(tmp_value_ptr, cfg_attr, ipv4_mip_ha);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_3GPP2_IPV4_HOA_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_3GPP2_MIP4_HOA_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV4_ADDR(tmp_value_ptr, cfg_attr, ipv4_mip_hoa);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_3GPP2_IPV6_HA_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_3GPP2_MIP6_HA_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV6_ADDR(tmp_value_ptr, cfg_attr, ipv6_mip_ha);
       break;

     case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_3GPP2_IPV6_HOA_TLV_ID:
       cfg_attr->param_mask |= QMI_WDS_IPSEC_CFG_ATTR_3GPP2_MIP6_HOA_PARAM_MASK;
       QMI_WDS_UTIL_READ_CFG_ATTR_IPV6_ADDR(tmp_value_ptr, cfg_attr, ipv6_mip_hoa);
       break;

     default:
       QMI_ERR_MSG_1("qmi_wds_process_ipsec_cfg_attr_tlvs: unknown msg_type=%d\n",
                     type);
       goto bail;
  }

  *value_ptr = tmp_value_ptr;

bail:
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_read_ipsec_sa_config_tlvs
===========================================================================*/
/*!
@brief
  Takes an input RX buffer containing IPSec SA config TLV info and translates
  to qmi_wds_ipsec_sa_config_type structure


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
qmi_wds_read_ipsec_sa_config_tlvs
(
  unsigned char                 *rx_buf,
  int                           rx_buf_len,
  qmi_wds_ipsec_sa_config_type  *sa_config
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  int           i;
  unsigned long temp_32bit;
  unsigned short temp_16bit;
  unsigned char temp_8bit;

  /* Temporary variables to read traffic
   * selector address TLVs */
  qmi_wds_ip_addr_type start_address;
  qmi_wds_ip_addr_type end_address;

  if (!rx_buf || !sa_config)
  {
    QMI_ERR_MSG_0("qmi_wds_read_ipsec_sa_config_tlvs: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  sa_config->param_mask = 0;
  sa_config->cfg_attr.param_mask = 0;

  /* Loop through looking for TLV's */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                               &rx_buf_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch(type)
    {
      case QMI_WDS_REV_IP_TRANS_GET_SA_SPI_RX_TLV_ID:
      {
        READ_32_BIT_VAL(value_ptr, temp_32bit);
        sa_config->param_mask |= QMI_WDS_IPSEC_SPI_RX_PARAM_MASK;
        sa_config->spi_rx = (qmi_wds_security_param_index_type) temp_32bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_SPI_TX_TLV_ID:
      {
        READ_32_BIT_VAL(value_ptr, temp_32bit);
        sa_config->param_mask |= QMI_WDS_IPSEC_SPI_TX_PARAM_MASK;
        sa_config->spi_tx = (qmi_wds_security_param_index_type) temp_32bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_PROTO_TLV_ID:
      {
        READ_32_BIT_VAL(value_ptr, temp_32bit);
        sa_config->param_mask |= QMI_WDS_IPSEC_SA_PROTO_PARAM_MASK;
        sa_config->proto = (qmi_wds_ipsec_sa_proto_type) temp_32bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_ENCAP_MODE_TLV_ID:
      {
        READ_32_BIT_VAL(value_ptr, temp_32bit);
        sa_config->param_mask |= QMI_WDS_IPSEC_ENCAP_MODE_PARAM_MASK;
        sa_config->encap_mode = (qmi_wds_ipsec_encapsulation_mode) temp_32bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_DEST_ADDR_TLV_ID:
      {
        sa_config->param_mask |= QMI_WDS_IPSEC_DEST_ADDR_PARAM_MASK;
        QMI_WDS_UTIL_READ_SA_CONFIG_IP_ADDR(value_ptr, sa_config->dest_addr);
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_LOCAL_ADDR_TLV_ID:
      {
        sa_config->param_mask |= QMI_WDS_IPSEC_LOCAL_ADDR_PARAM_MASK;
        QMI_WDS_UTIL_READ_SA_CONFIG_IP_ADDR(value_ptr, sa_config->local_addr);
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_HASH_ALGO_TLV_ID:
      {
        READ_32_BIT_VAL(value_ptr, temp_32bit);
        sa_config->param_mask |= QMI_WDS_IPSEC_HASH_ALGO_PARAM_MASK;
        sa_config->hash_algo = (qmi_wds_ipsec_hash_crypto_algo_type) temp_32bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_HASH_KEY_RX_TLV_ID:
      {
        READ_8_BIT_VAL(value_ptr, temp_8bit);
        sa_config->param_mask |= QMI_WDS_IPSEC_HASH_KEY_RX_PARAM_MASK;
        sa_config->hash_key_rx.size = temp_8bit;
        memcpy(sa_config->hash_key_rx.key, value_ptr, temp_8bit);
        value_ptr += temp_8bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_HASH_KEY_TX_TLV_ID:
      {
        READ_8_BIT_VAL(value_ptr, temp_8bit);
        sa_config->param_mask |= QMI_WDS_IPSEC_HASH_KEY_TX_PARAM_MASK;
        sa_config->hash_key_tx.size = temp_8bit;
        memcpy(sa_config->hash_key_tx.key, value_ptr, temp_8bit);
        value_ptr += temp_8bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_CRYPTO_ALGO_TLV_ID:
      {
        READ_32_BIT_VAL(value_ptr, temp_32bit);
        sa_config->param_mask |= QMI_WDS_IPSEC_CRYPTO_ALGO_PARAM_MASK;
        sa_config->crypto_algo = (qmi_wds_ipsec_hash_crypto_algo_type) temp_32bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_CRYPTO_KEY_RX_TLV_ID:
      {
        READ_8_BIT_VAL(value_ptr, temp_8bit);
        sa_config->param_mask |= QMI_WDS_IPSEC_CRYPTO_KEY_RX_PARAM_MASK;
        sa_config->crypto_key_rx.size = temp_8bit;
        memcpy(sa_config->crypto_key_rx.key, value_ptr, temp_8bit);
        value_ptr += temp_8bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_CRYPTO_KEY_TX_TLV_ID:
      {
        READ_8_BIT_VAL(value_ptr, temp_8bit);
        sa_config->param_mask |= QMI_WDS_IPSEC_CRYPTO_KEY_TX_PARAM_MASK;
        sa_config->crypto_key_tx.size = temp_8bit;
        memcpy(sa_config->crypto_key_tx.key, value_ptr, temp_8bit);
        value_ptr += temp_8bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_IV_TLV_ID:
      {
        READ_8_BIT_VAL(value_ptr, temp_8bit);
        sa_config->param_mask |= QMI_WDS_IPSEC_IV_PARAM_MASK;
        sa_config->iv.size = temp_8bit;
        memcpy(sa_config->iv.key, value_ptr, temp_8bit);
        value_ptr += temp_8bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_UDP_ENCAP_TLV_ID:
      {
        READ_8_BIT_VAL(value_ptr, temp_8bit);
        sa_config->param_mask |= QMI_WDS_IPSEC_UDP_ENCAP_PARAM_MASK;
        sa_config->is_udp_encap = temp_8bit;
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_NAT_LOCAL_IP_TLV_ID:
      {
        sa_config->param_mask |= QMI_WDS_IPSEC_NAT_LOCAL_ADDR_PARAM_MASK;
        QMI_WDS_UTIL_READ_SA_CONFIG_IP_ADDR(value_ptr, sa_config->nat_local_addr);
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_NAT_REMOTE_IP_TLV_ID:
      {
        sa_config->param_mask |= QMI_WDS_IPSEC_NAT_REMOTE_ADDR_PARAM_MASK;
        QMI_WDS_UTIL_READ_SA_CONFIG_IP_ADDR(value_ptr, sa_config->nat_remote_addr);
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_ADDR_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_MASK_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_DNS_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_NBNS_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_ADDR_EXPIRY_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_DHCP_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_APP_VER_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_ADDR_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_DNS_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_NBNS_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_DHCP_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_SUBNET_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_SUPP_ATTR_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_SUBNET_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV4_PCSCF_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_IPV6_PCSCF_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_3GPP2_IPV4_HA_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_3GPP2_IPV4_HOA_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_3GPP2_IPV6_HA_TLV_ID:
      case QMI_WDS_REV_IP_TRANS_GET_SA_CFG_ATTR_3GPP2_IPV6_HOA_TLV_ID:
      {
        if (QMI_NO_ERR != qmi_wds_process_ipsec_cfg_attr_tlvs(type,
                                                              &value_ptr,
                                                              &sa_config->cfg_attr))
        {
          return QMI_INTERNAL_ERR;
        }
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_TRAFFIC_SEL_TLV_ID:
      {
        sa_config->param_mask |= QMI_WDS_IPSEC_TS_LIST_PARAM_MASK;
        READ_8_BIT_VAL(value_ptr, temp_8bit);

        /* Determine the traffic selector list length */
        temp_8bit = (unsigned char)((temp_8bit < QMI_WDS_IPSEC_MAX_TRAFFIC_SEL_SIZE) ?
                                    temp_8bit :
                                    QMI_WDS_IPSEC_MAX_TRAFFIC_SEL_SIZE);

        sa_config->ts_list.len = temp_8bit;

        /* Read the values into the traffic selector array */
        for (i = 0; i < temp_8bit; ++i)
        {
          READ_8_BIT_VAL(value_ptr, sa_config->ts_list.ts[i].proto);
          READ_16_BIT_VAL(value_ptr, sa_config->ts_list.ts[i].start_port);
          READ_16_BIT_VAL(value_ptr, sa_config->ts_list.ts[i].end_port);

          /* Read the address from TLV into temp variable */
          QMI_WDS_UTIL_READ_SA_CONFIG_IP_ADDR(value_ptr, start_address);
          QMI_WDS_UTIL_READ_SA_CONFIG_IP_ADDR(value_ptr, end_address);

          /* Copy the address from TLV into the traffic selector array */
          memcpy(&sa_config->ts_list.ts[i].start_addr,
                 &start_address,
                 sizeof(qmi_wds_ip_addr_type));
          memcpy(&sa_config->ts_list.ts[i].end_addr,
                 &end_address,
                 sizeof(qmi_wds_ip_addr_type));
        }
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_SA_TRAFFIC_SEL_RESPONDER_TLV_ID:
      {
        sa_config->param_mask |= QMI_WDS_IPSEC_TS_RESP_LIST_PARAM_MASK;
        READ_8_BIT_VAL(value_ptr, temp_8bit);

        /* Determine the traffic selector list length */
        temp_8bit = (unsigned char)((temp_8bit < QMI_WDS_IPSEC_MAX_TRAFFIC_SEL_SIZE) ?
                                    temp_8bit :
                                    QMI_WDS_IPSEC_MAX_TRAFFIC_SEL_SIZE);

        sa_config->ts_list_resp.len = temp_8bit;

        /* Read the values into the traffic selector array */
        for (i = 0; i < temp_8bit; ++i)
        {
          READ_8_BIT_VAL(value_ptr, sa_config->ts_list_resp.ts[i].proto);
          READ_16_BIT_VAL(value_ptr, sa_config->ts_list_resp.ts[i].start_port);
          READ_16_BIT_VAL(value_ptr, sa_config->ts_list_resp.ts[i].end_port);

          /* Read the address from TLV into temp variable */
          QMI_WDS_UTIL_READ_SA_CONFIG_IP_ADDR(value_ptr, start_address);
          QMI_WDS_UTIL_READ_SA_CONFIG_IP_ADDR(value_ptr, end_address);

          /* Copy the address from TLV into the traffic selector array */
          memcpy(&sa_config->ts_list_resp.ts[i].start_addr,
                 &start_address,
                 sizeof(qmi_wds_ip_addr_type));
          memcpy(&sa_config->ts_list_resp.ts[i].end_addr,
                 &end_address,
                 sizeof(qmi_wds_ip_addr_type));
        }
      }
      break;

      case QMI_WDS_REV_IP_TRANS_GET_AES_MODE_TLV_ID:
      {
        sa_config->param_mask |= QMI_WDS_IPSEC_AES_MODE_PARAM_MASK;

        /* Read the AES Mode value */
        READ_32_BIT_VAL(value_ptr, temp_32bit);

        sa_config->aes_mode = (qmi_wds_ipsec_aes_algo_mode_type) temp_32bit;
      }
      break;

      default:
      {
        /* If the TLV is unknown, ignore it. Don't return error */
        QMI_ERR_MSG_1 ("qmi_wds_srvc_process_rev_ip_trans_ind: "
                       "unknown type = %x\n",(unsigned) type);
      }
      break;
    } /*switch*/

  } /* while */

  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_wds_get_ipsec_static_sa_config
===========================================================================*/
/*!
@brief
  This message is used to retrieve IPSec Security Associations for the ePDG
  call.

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
qmi_wds_get_ipsec_static_sa_config
(
  int                               user_handle,
  qmi_wds_ipsec_sa_config_type      *sa_config,
  int                               *qmi_err_code
)
{
  unsigned char     *msg = NULL;
  int               msg_size;
  unsigned char     *tmp_msg_ptr;
  int rc;

  if (!sa_config || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_get_ipsec_static_sa_config: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  msg = (unsigned char *) malloc (QMI_MAX_MSG_SIZE);

  if (!msg)
  {
    QMI_ERR_MSG_0 ("qmi_wds_get_ipsec_static_sa_config: malloc failed\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_MAX_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_IPSEC_STATIC_SA_CONFIG,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_MAX_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_MAX_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Get SA config information */
  if (rc == QMI_NO_ERR)
  {
    memset(sa_config, 0, sizeof(*sa_config));

    rc = qmi_wds_read_ipsec_sa_config_tlvs (tmp_msg_ptr,
                                            msg_size,
                                            sa_config);
  }

  free(msg);
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_rev_ip_trans_config_complete
===========================================================================*/
/*!
@brief
  This message is used to convey that the reverse adapter configuration is
  complete on the AP side

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
qmi_wds_rev_ip_transport_config_complete
(
  int                                          user_handle,
  qmi_wds_txn_id_type                          txn_id,
  qmi_wds_rev_ip_transport_config_result_type  result,
  int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char     *tmp_msg_ptr;
  unsigned char     config_result;
  int rc;


  if (!qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_rev_ip_transport_config_complete: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  config_result = (unsigned char) result;

  /* Write configuration result in main message buffer */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_REV_IP_TRANS_CONFIG_COMPL_RESULT_TLV_ID,
                              1,
                              (void *)&config_result) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Write txn id in main message buffer */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_REV_IP_TRANS_CONFIG_COMPL_TXN_ID_TLV_ID,
                              4,
                              (void *)&txn_id) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_REV_IP_TRANS_CONFIG_COMPLETE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_initiate_esp_rekey
===========================================================================*/
/*!
@brief
  This message is used to initiate an ESP rekey for the iWLAN call
  corresponding to the given WDS handle

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
qmi_wds_initiate_esp_rekey
(
  int  user_handle,
  int  *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  int rc;

  if (!qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_initiate_esp_rekey: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_INITIATE_ESP_REKEY,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_set_data_path_pref
===========================================================================*/
/*!
@brief
  This message is used to set the client data path

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
qmi_wds_set_data_path
(
  int                     user_handle,
  qmi_wds_data_path_type  data_path,
  int                     *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char     *tmp_msg_ptr;
  unsigned long     tmp32;
  int rc;


  if (!qmi_err_code || data_path <= QMI_WDS_DATA_PATH_UNKNOWN || data_path >= QMI_WDS_DATA_PATH_MAX)
  {
    QMI_ERR_MSG_0("qmi_wds_set_data_path_pref: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  tmp32 = (unsigned long) data_path;

  /* Write data path in main message buffer */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_WDS_SET_DATA_PATH_TLV_ID,
                              4,
                              (void *)&tmp32) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_SET_DATA_PATH_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_get_data_path
===========================================================================*/
/*!
@brief
  This message is used to query the current modem data path

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
qmi_wds_get_data_path
(
  int                         user_handle,
  qmi_wds_get_data_path_type  *data_path,
  int                         *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char     *tmp_msg_ptr;
  int rc;

  if (!data_path || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_wds_get_data_path: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Initialize out param to invalid value */
  data_path->param_mask = 0;

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_WDS_SERVICE,
                                  QMI_WDS_GET_DATA_PATH_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_WDS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Process response TLV's if there is no error */
  if (rc == QMI_NO_ERR)
  {
    /* Extract and process all TLV's */
    while (msg_size > 0)
    {
      unsigned long type;
      unsigned long length;
      unsigned char *value_ptr;

      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_WDS_PREF_DATA_PATH_TLV_ID:
          {
            unsigned long tmp32;

            READ_32_BIT_VAL (value_ptr, tmp32);
            data_path->pref_data_path = (qmi_wds_data_path_type) tmp32;
            data_path->param_mask |= QMI_WDS_PREF_DATA_PATH_PARAM_MASK;
          }
          break;
        case QMI_WDS_ACTUAL_DATA_PATH_TLV_ID:
          {
            unsigned long tmp32;

            READ_32_BIT_VAL (value_ptr, tmp32);
            data_path->actual_data_path = (qmi_wds_data_path_type) tmp32;
            data_path->param_mask |= QMI_WDS_ACTUAL_DATA_PATH_PARAM_MASK;
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_wds_get_data_path: unknown response TLV type "
                           "= %x",(unsigned int)type);
          }
          break;
      }/*Switch*/
    }/*While*/
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_wds_remove_delegated_ipv6_prefix
===========================================================================*/
/*!
@brief
  This message is used to remove IPv6 prefix from the interface

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
qmi_wds_remove_delegated_ipv6_prefix
(
  int                                 user_handle,
  qmi_wds_delegated_ipv6_prefix_type  *params,
  int                                 *qmi_err_code
)
{
  unsigned char     msg[QMI_WDS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char     *tmp_msg_ptr;

  if (!qmi_err_code || NULL == params)
  {
    QMI_ERR_MSG_0("qmi_wds_remove_delegated_ipv6_prefix: bad input parameters\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set the tmp_msg_ptr to beginning of message-specific TLV portion of message
   * buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE);

  /* Prepare message TLV */
  unsigned char   tmp_buf[QMI_WDS_REMOVE_DELEGATED_IPV6_PREFIX_TLV_SIZE];
  unsigned char   *tmp_buf_ptr = tmp_buf;
  ipv6_addr_type  ipv6_addr;
  unsigned char   tmp8;

  memcpy(tmp_buf_ptr, &params->ipv6_addr, sizeof(ipv6_addr));
  tmp_buf_ptr += QMI_WDS_IPV6_ADDR_SIZE_IN_BYTES;

  tmp8 = (unsigned char) params->prefix_len;
  WRITE_8_BIT_VAL(tmp_buf_ptr, tmp8);

  if (qmi_util_write_std_tlv(&tmp_msg_ptr,
                             &msg_size,
                             QMI_WDS_REMOVE_DELEGATED_IPV6_PREFIX_REQ_TLV_ID,
                             QMI_WDS_REMOVE_DELEGATED_IPV6_PREFIX_TLV_SIZE,
                             (void *) tmp_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return qmi_service_send_msg_sync(user_handle,
                                   QMI_WDS_SERVICE,
                                   QMI_WDS_REMOVE_DELEGATED_IPV6_PREFIX_REQ_MSG_ID,
                                   QMI_SRVC_PDU_PTR(msg),
                                   (int)QMI_SRVC_PDU_SIZE(QMI_WDS_STD_MSG_SIZE) - msg_size,
                                   msg,
                                   &msg_size,
                                   QMI_WDS_STD_MSG_SIZE,
                                   QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                   qmi_err_code);
}
