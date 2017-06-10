/*!
  @file
  qcril_data_netctrl.c

  @brief
  Handles RIL requests for DATA services.

*/

/*===========================================================================

  Copyright (c) 2008-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_data.c#17 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
02/12/11   sg      Added timeout functionality for setup data call request
05/05/10   js      replaced dss with dsi_netctrl
03/01/10   fc      Re-architecture to support split modem
06/26/09   fc      Fixed the issue of bogus RIL Request reported in call flow
                   log packet.
05/29/09   fc      Renamed functions.
05/21/09   sm      Passes auth pref to dss from ril
05/14/09   pg      Changed NULL APN handling in data call setup.
                   Mainlined FEATURE_MULTIMODE_ANDROID.
04/05/09   fc      Cleanup log macros.
03/10/09   pg      Fixed multi-mode data call.
12/29/08   fc      Fixed wrong size issue being reported for the response
                   payload of RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE.
12/23/08   asn     code reorg and IP addr fix
12/23/08   asn     Added handling on MO call-end, IP addr issue, APN trunc issue
12/15/08   asn     Fixed call teardown and improved stability
12/08/08   pg      Added multi-mode data call hook up.
11/18/08   fc      Changes to avoid APN string being truncated.
11/14/08   sm      Added temp CDMA data support.
08/29/08   asn     Added data support
08/08/08   asn     Initial version

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <stdio.h>
#ifdef QCRIL_DATA_OFFTARGET
   #include <netinet/in.h>
   #include <errno.h>
#endif
#include <net/if.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#ifdef QCRIL_DATA_OFFTARGET
   #include <signal.h>
#else
   #include <asm-generic/siginfo.h>
#endif
#include <linux/socket.h>

#include "ril.h"
#include "IxErrno.h"
#include "qcrili.h"
#include "qcril_reqlist.h"
#include "dsi_netctrl.h"
#include "qmi_wds_srvc.h"
#include "qmi_wds_utils.h"
#include "qmi_platform.h"
#include "qdp.h"
#include "qcril_arb.h" /* QCRIL_ARB_* defines*/
#include "qcril_data.h"
#include "qcril_data_defs.h"
#include "qcril_data_utils.h"
#include "qcril_data_client.h"
#include "qmi_ril_platform_dep.h"

#define HACK_MODEM_QUERY_SEARCH_UNAVAIL
#include <string.h>

#include <cutils/properties.h>
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
   // Don't have system properties on Ubunto x86, use property server
   #include <sys/system_properties.h>
#endif
#include "ds_string.h"
#include "data_system_determination_v01.h"
#include "qmi_client_instance_defs.h"
#include "ds_util.h"

/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/
#define QCRIL_DATA_INVALID_INSTANCE_ID (QCRIL_MAX_INSTANCE_ID)
#define QCRIL_DATA_INVALID_MODEM_ID    (0xFFFFFFFF)

/*---------------------------------------------------------------------------
   Macros for Android setup data call timer override
---------------------------------------------------------------------------*/
/* System property for setup data call timeout, supported range 0-99999
   if it is set to 0, a default timeout value QCRIL_DATA_PROPERTY_TIMEOUT_DEFAULT
   will be used.
*/
#define QCRIL_DATA_PROPERTY_TIMEOUT             "persist.qcril.datacall_timeout"
#define QCRIL_DATA_PROPERTY_TIMEOUT_SIZE        (6)

/*---------------------------------------------------------------------------
  System properties for power optimization:
---------------------------------------------------------------------------*/
/* This property drops the output TCP RST packets property when screen is OFF */
#define QCRIL_DATA_PROPERTY_TCP_RST_DROP         "persist.data.tcp_rst_drop"

/* Default timeout is slightly more than five times T3380 (30 seconds*5).
   Reference [3GPP TS24.008 section 6.1.3.1.5]
*/
#define QCRIL_DATA_PROPERTY_TIMEOUT_DEFAULT  (155)
#define QCRIL_DATA_PROPERTY_TIMEOUT_INVALID  (0)

#define QCRIL_DATA_CALL_OBJ_MATCH_FOUND      (SUCCESS)
#define QCRIL_DATA_CALL_OBJ_MATCH_NOT_FOUND  (FAILURE)

#define QCRIL_DATA_QMI_DSD_SYNC_MSG_TIMEOUT  (10000)
#define QCRIL_QMI_DSD_TIMEOUT                (30000)

typedef enum
{
  QCRIL_DATA_IFACE_IOCTL_GO_DORMANT,
  QCRIL_DATA_IFACE_IOCTL_DORMANCY_INDICATIONS_OFF,
  QCRIL_DATA_IFACE_IOCTL_DORMANCY_INDICATIONS_ON
} qcril_data_iface_ioctl_type;

#ifndef PLATFORM_LTK
#ifdef ASSERT
#undef ASSERT
#endif /* ASSERT */
#define ASSERT( xx_exp ) ((void)0)
#endif /* PLATFORM_LTK */

#define QMI_WDS_CDMA_PROFILE_APP_TYPE_TLV_ID 0x1E
#define QMI_WDS_CDMA_PROFILE_APP_TYPE_TLV_SIZE 4

/* Booleans */
#define TRUE  1
#define FALSE 0

/* Map RIL commands to local names */
#define DS_RIL_REQ_ACT_DATA_CALL        RIL_REQUEST_SETUP_DATA_CALL
#define DS_RIL_REQ_DEACT_DATA_CALL      RIL_REQUEST_DEACTIVATE_DATA_CALL
#define DS_RIL_REQ_GET_CALL_LIST        RIL_REQUEST_DATA_CALL_LIST
#define DS_RIL_REQ_GET_LAST_FAIL_CAUSE  RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE
#define DS_RIL_IND_CALL_LIST_CHANGED    RIL_UNSOL_DATA_CALL_LIST_CHANGED

#define QCRIL_DATA_ADDR_DELIMITER      " "
#define QCRIL_DATA_ADDR_DELIMITER_LEN  1

#define QCRIL_DATA_SUGGESTEDRETRYTIME  (-1)      /* no value suggested, see ril.h */
#define QCRIL_DATA_NORETRY_TIME        (INT_MAX) /* no value suggested, see ril.h */

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))

typedef struct
{
  unsigned long       ipv4;
  union
  {
    unsigned long long u64;
    unsigned long      u32[2];
    char                u8[16];
  } ipv6;
} qcril_data_addr_t;


#define DS_CALL_ADDR_FORMAT_IPV4(data,str,len)                          \
        (void)snprintf( str, len,                                       \
                        "%d.%d.%d.%d%c",                                \
                        ((char*)data)[ 0],((char*)data)[ 1],            \
                        ((char*)data)[ 2],((char*)data)[ 3], 0 );

#define DS_CALL_ADDR_FORMAT_IPV6(data,str,len)                          \
        (void)snprintf( str, len,                                       \
                        "%.02x%.02x:%.02x%.02x:%.02x%.02x:%.02x%.02x:"  \
                        "%.02x%.02x:%.02x%.02x:%.02x%.02x:%.02x%.02x%c",\
                        ((char*)data)[ 0],((char*)data)[ 1],            \
                        ((char*)data)[ 2],((char*)data)[ 3],            \
                        ((char*)data)[ 4],((char*)data)[ 5],            \
                        ((char*)data)[ 6],((char*)data)[ 7],            \
                        ((char*)data)[ 8],((char*)data)[ 9],            \
                        ((char*)data)[10],((char*)data)[11],            \
                        ((char*)data)[12],((char*)data)[13],            \
                        ((char*)data)[14],((char*)data)[15], 0 );

#define DS_CALL_ADDR_FORMAT_IPV4V6(addr,str,len)                        \
        if( AF_INET == SASTORAGE_FAMILY(*addr) )                        \
          DS_CALL_ADDR_FORMAT_IPV4( SASTORAGE_DATA(*addr), str, len )   \
        else if( AF_INET6 == SASTORAGE_FAMILY(*addr) )                  \
          DS_CALL_ADDR_FORMAT_IPV6( SASTORAGE_DATA(*addr), str, len )

/*Format IP address with Subnet Mask or prefix length in CIDR notation.
 * if_addr/prefix length: 10.123.240.10/24 */
#define DS_CALL_ADDR_FORMAT_IPV4_WITH_SUBNET_MASK(data,mask,str,len)    \
        (void)snprintf( str, len,                                       \
                        "%d.%d.%d.%d/%d%c",                             \
                        ((char*)data)[ 0],((char*)data)[ 1],            \
                        ((char*)data)[ 2],((char*)data)[ 3],            \
                        mask, 0 );

#define DS_CALL_ADDR_FORMAT_IPV6_WITH_PREFIX_LENGTH(data,mask,str,len) \
        (void)snprintf( str, len,                                            \
                        "%.02x%.02x:%.02x%.02x:%.02x%.02x:%.02x%.02x:"       \
                        "%.02x%.02x:%.02x%.02x:%.02x%.02x:%.02x%.02x/%d%c",  \
                        ((char*)data)[ 0],((char*)data)[ 1],                 \
                        ((char*)data)[ 2],((char*)data)[ 3],                 \
                        ((char*)data)[ 4],((char*)data)[ 5],                 \
                        ((char*)data)[ 6],((char*)data)[ 7],                 \
                        ((char*)data)[ 8],((char*)data)[ 9],                 \
                        ((char*)data)[10],((char*)data)[11],                 \
                        ((char*)data)[12],((char*)data)[13],                 \
                        ((char*)data)[14],((char*)data)[15], mask, 0 );

#define DS_CALL_ADDR_FORMAT_IPV4V6_WITH_PREFIX_LEN(addr,mask,str,len)         \
        if( AF_INET == SASTORAGE_FAMILY(*addr) )                              \
          DS_CALL_ADDR_FORMAT_IPV4_WITH_SUBNET_MASK( SASTORAGE_DATA(*addr),   \
                                                     mask, str, len )         \
        else if( AF_INET6 == SASTORAGE_FAMILY(*addr) )                        \
          DS_CALL_ADDR_FORMAT_IPV6_WITH_PREFIX_LENGTH( SASTORAGE_DATA(*addr), \
                                                       mask, str, len )

#if 0 /* For use when dual-IP supported */
#define DS_CALL_ADDR_FORMAT(data,str,len)                                         \
        if( (data)->ipv4 ) DS_CALL_ADDR_FORMAT_IPV4( &(data)->ipv4, str, len );   \
        if( (data)->ipv4 && (data)->ipv6.u64 ) strncat( str, " ", sizeof(str) );  \
        if( (data)->ipv6.u64 ) DS_CALL_ADDR_FORMAT_IPV6( &(data)->ipv6.u64,       \
                                                         (str+strlen(str)),       \
                                                         (len-strlen(str)) );
#endif /* 0 */

#define QCRIL_DATA_IS_RIL_RADIO_TECH_CDMA_1X_EVDO(t)  ((t == QCRIL_DATA_RIL_RADIO_TECH_IS95A)  || \
                                                       (t == QCRIL_DATA_RIL_RADIO_TECH_IS95B)  || \
                                                       (t == QCRIL_DATA_RIL_RADIO_TECH_1xRTT)  || \
                                                       (t == QCRIL_DATA_RIL_RADIO_TECH_EVDO_0) || \
                                                       (t == QCRIL_DATA_RIL_RADIO_TECH_EVDO_A) || \
                                                       (t == QCRIL_DATA_RIL_RADIO_TECH_EVDO_B))

#define QCRIL_DATA_IS_RIL_RADIO_TECH_EHRPD(t) (t == QCRIL_DATA_RIL_RADIO_TECH_EHRPD)

#define QCRIL_DATA_IS_RIL_RADIO_TECH_CDMA(t)  (QCRIL_DATA_IS_RIL_RADIO_TECH_CDMA_1X_EVDO(t) ||        \
                                                       (t == QCRIL_DATA_RIL_RADIO_TECH_EHRPD))

#define QCRIL_DATA_IS_RIL_RADIO_TECH_3GPP_EHRPD(t)    ((t == QCRIL_DATA_RIL_RADIO_TECH_GPRS)    || \
                                                       (t == QCRIL_DATA_RIL_RADIO_TECH_HSDPA)   || \
                                                       (t == QCRIL_DATA_RIL_RADIO_TECH_HSUPA)   || \
                                                       (t == QCRIL_DATA_RIL_RADIO_TECH_HSPA)    || \
                                                       (t == QCRIL_DATA_RIL_RADIO_TECH_HSPAP)   || \
                                                       (t == QCRIL_DATA_RIL_RADIO_TECH_EDGE)    || \
                                                       (t == QCRIL_DATA_RIL_RADIO_TECH_TDSCDMA) || \
                                                       (t == QCRIL_DATA_RIL_RADIO_TECH_EHRPD))

#endif /* ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6)) */


/*! @brief Typedef variables internal to module qcril_data.c
*/
typedef struct
{
  unsigned char rflag;  /* determines RIL Token validity */
  RIL_Token     rt;     /* valid RIL token if flag is set */
  int           request;
  void         *data;
  void         *self;
} qcril_data_net_cb_info_t;

/*! @brief Typedef variables internal to module qcril_data.c
*/
typedef struct
{
  int wds_hndl;
  qmi_service_id_type sid;
  void* user_data;
  qmi_wds_indication_id_type ind_id;
  qmi_wds_indication_data_type ind_data;
  void* self;
} qcril_data_wds_event_data_t;


typedef struct qcril_data_call_curr_info_s
{
  qmi_wds_profile_id_type profile_id;
  qmi_wds_profile_params_type profile_params;
  qmi_wds_curr_call_info_type call_info;
  qmi_wds_data_bearer_tech_type bearer_info;
  qmi_wds_channel_rate_type channel_info;
} qcril_data_call_curr_info_type;


#define IPCONV( ip_addr , t ) ( ( ip_addr >> (24 - 8 * t)) & 0xFF)

#define QCRIL_MAX_DEV_NAME_SIZE             DS_CALL_INFO_DEV_NAME_MAX_LEN
#define QCRIL_DATA_MAX_DEVS                 16
#define QCRIL_DATA_MAX_DEVS_SIZE            2  /* Digits required */

/* Scan from end of device name string to determine numeric instance */
#define GET_DEV_INSTANCE_FROM_NAME(index, dest)                          \
    do                                                                   \
    {                                                                    \
      dest = QCRIL_DATA_HNDL_INVALID;                                    \
      if (TRUE == info_tbl[index].info_flg)                              \
      {                                                                  \
        char *ptr = NULL;                                                \
        ptr = strpbrk(info_tbl[index].call_info.dev_name, "0123456789"); \
        if (ptr)                                                         \
        {                                                                \
          unsigned int len = strlen(ptr);                                \
          if (len && QCRIL_DATA_MAX_DEVS_SIZE >= len)                    \
          { /* TODO:consider changing this to strtol()*/                 \
            dest = atoi(ptr);                                            \
          }                                                              \
        }                                                                \
      }                                                                  \
    } while (0)

/* temporary define here until available in msg lib */
#define QMI_WDS_CE_REASON_SYS_NOT_PREF 14

#define QCRIL_QMI_PORT_NAME_SIZE  (16)
typedef struct qcril_data_modem_qmi_port_map_s
{
  int modem_id;
  char qmi_port[QCRIL_QMI_PORT_NAME_SIZE+1];
} qcril_data_modem_port_map;

void qcril_data_util_update_mtu
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr
);


/*===========================================================================

                         GLOBAL VARIABLES

===========================================================================*/

/* these are events that qcril_data posts to
* itself for being processed from within
* the qcril_event thread. We must start
* the enumeration from 1000 in order to
* avoid the collision with dsi events */
typedef enum qcril_data_internal_events_e
{
  QCRIL_DATA_EV_MIN = 1000,
  QCRIL_DATA_EV_DATA_CALL_STATUS_CHANGED =
  QCRIL_DATA_EV_MIN+1,
  QCRIL_DATA_EV_MAX
} qcril_data_internal_events;

qcril_data_call_info_tbl_type info_tbl[ MAX_CONCURRENT_UMTS_DATA_CALLS ];

/* Table containing the state for handling dsi callbacks */
qcril_data_dsi_cb_tbl_type dsi_cb_tbl[ MAX_CONCURRENT_UMTS_DATA_CALLS ];

/*
  This is used to store last call end reason for responding
  to RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE. Meant for Single PDP only.
  Functionality fails for multiple PDP using QCRIL.
*/
static int last_call_end_reason = PDP_FAIL_ERROR_UNSPECIFIED;
static int global_qmi_wds_hndl = QCRIL_DATA_HNDL_INVALID;
static int global_instance_id = QCRIL_DEFAULT_INSTANCE_ID;
static int global_subs_id = QCRIL_DEFAULT_MODEM_STACK_ID;
static int global_modem_id = QCRIL_DEFAULT_MODEM_ID;
static boolean ignore_ril_tech = TRUE;
static qmi_client_type  global_qmi_dsd_hndl = NULL;
static unsigned int qcril_data_mtu;
static boolean epc_profile_supported = TRUE;

static qmi_client_os_params dsd_os_params;

/* Info about current data system */
typedef struct
{
  qcril_data_legacy_dsd_mode_t  dsd_mode;
  qmi_wds_pref_data_sys_type    pref_data_tech;
  qmi_wds_data_sys_status_type  data_sys_status;
} qcril_data_legacy_dsd_info_t;

typedef enum
{
  QCRIL_DATA_DSD_SERVICE_TYPE_UNKNOWN,
  QCRIL_DATA_DSD_SERVICE_TYPE_LEGACY,
  QCRIL_DATA_DSD_SERVICE_TYPE_QMI_DSD,
} qcril_data_dsd_service_t;

typedef struct
{
  qcril_data_dsd_service_t  service;
  qcril_data_legacy_dsd_info_t  legacy;
  dsd_system_status_ind_msg_v01 qmi_dsd;
} qcril_data_dsd_info_t;

static qcril_data_dsd_info_t  global_dsd_info;


#define QCRIL_NETWORK_INFO_LEN 10

#define QCRIL_DATA_PROP_SIZE     PROPERTY_VALUE_MAX
#define QCRIL_DATA_PROP_DONT_USE_DSD "persist.radio.dont_use_dsd"

static qcril_data_modem_port_map qcril_data_modem_port_map_tbl[] =
{
  { QCRIL_DEFAULT_MODEM_ID, QMI_PORT_RMNET_0 },
  { QCRIL_SECOND_MODEM_ID, QMI_PORT_RMNET_SDIO_0 }
};

static char * default_qmi_port = QMI_PORT_RMNET_0;

static qmi_client_qmux_instance_type default_qmi_instance = QMI_CLIENT_QMUX_BASE_INSTANCE;

typedef enum RIL_tethered_mode_states_e
{
  QCRIL_TETHERED_MODE_OFF = 0,
  QCRIL_TETHERED_MODE_ON = 1
} RIL_tethered_mode_state;

static RIL_tethered_mode_state global_tethered_state = QCRIL_TETHERED_MODE_OFF;

/* this mutex protects info tbl */
pthread_mutex_t info_tbl_mutex;

qmi_wds_dorm_status_type global_dorm_status = 0xffffffff;

static qcril_data_dormancy_ind_switch_type global_dorm_ind = DORMANCY_INDICATIONS_OFF;
static qcril_data_limited_sys_ind_switch_type global_data_sys_ind_switch = LIMITED_SYS_INDICATIONS_OFF;

/* add 1 for ending NULL character */
typedef char qcril_data_go_dormant_params_type[QCRIL_MAX_DEV_NAME_SIZE+1];

extern int dsi_set_ril_instance(int instance);

extern int dsi_set_modem_subs_id(int subs_id);

extern int ds_atoi(const char * str);
extern void ds_get_epid (char *net_dev, ds_ep_type_t *ep_type, int *epid);

#ifdef RIL_REQUEST_SET_INITIAL_ATTACH_APN

#define QCRIL_PROPERTY_DEFAULT_PROFILE       "persist.qcril.attach.profile"
#define QCRIL_LTE_DEFAULT_PROFILE_VALUE      "0"
#define QCRIL_PROPERTY_DEFAULT_PROFILE_SIZE  (5)
#define QCRIL_LTE_ATTACH_SUCCESS             (0)
#define QCRIL_LTE_ATTACH_FAILURE             (1)

static unsigned int global_lte_attach_profile = QDP_INVALID_PROFILE;

#endif /* RIL_REQUEST_SET_INITIAL_ATTACH_APN */

/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/
static void qcril_data_response_data_call_failure
(
 qcril_instance_id_e_type instance_id,
 RIL_Token t,
 int request,
 RIL_DataCallFailCause response
);

static void qcril_data_response_setup_data_call_failure
(
  qcril_data_call_info_tbl_type * info_tble_ptr,
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  RIL_DataCallFailCause response
);

static void qcril_data_unsol_call_list_changed
(
  qcril_instance_id_e_type instance_id
);

static void qcril_data_unsol_tethered_mode_state_changed
(
  qcril_instance_id_e_type instance_id,
  void                    *response,
  size_t                   response_len
);

static void qcril_data_util_buffer_dump
(
  const dsi_call_param_value_t  *const info
);

static void qcril_data_util_fill_call_params
(
  qcril_data_call_info_tbl_type *
);

static void qcril_data_util_create_setup_rsp
(
  qcril_data_call_info_tbl_type *,
  qcril_data_call_response_type *
);

static int qcril_data_iface_go_dormant
(
  int qmi_wds_hndl,
  dsi_ip_family_t ip_family;
);

static int qcril_data_iface_set_mtu
(
  dsi_hndl_t dsi_hndl,
  int mtu
);

static int qcril_data_iface_ioctl
(
  qcril_data_call_info_tbl_type * info_tbl_ptr,
  int ioctl,
  boolean * dorm_status_changed,
  int * call_state
);

static int qcril_data_set_nai
(
  int info_tbl_index,
  const char * ril_user,
  const char * ril_pass,
  const char * ril_auth_pref
);

static int qcril_data_set_ril_profile_id
(
  int info_tbl_index,
  const char * ril_profile,
  const char * ril_tech
);

static int qcril_data_apn_based_profile_look_up
(
  const char * str,
  int * umts_profile_id,
  int * cdma_profile_id
);

static inline int qcril_data_get_modem_port
(
  int modem_id,
  char * qmi_port
);

static void qcril_data_post_dsi_netctrl_event
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type    modem_id,
  RIL_Token                pend_tok,
  qcril_data_call_info_tbl_type *info_tbl_ptr,
  dsi_net_evt_t                  net_evt,
  dsi_evt_payload_t             *payload_ptr
);

static IxErrnoType qcril_data_stop_data_call
(
  qcril_data_call_info_tbl_type *info_tbl_ptr
);

static int qcril_data_all_calls_dormant
(
  void
);

static int qcril_data_reg_sys_ind
(
  qcril_data_limited_sys_ind_switch_type       sys_ind_switch
);

static void qcril_data_partial_retry_hdlr
(
  union sigval sval
);

static void qcril_data_abort_incompatible_pending_calls
(
  qmi_wds_data_sys_status_network_info_type  *nw_info
);





/*===========================================================================

                                FUNCTIONS

===========================================================================*/
/*-------------------------------------------------------------------------

                               CALLBACK FUNCTIONS

-------------------------------------------------------------------------*/
/*===========================================================================

  FUNCTION:  qcril_data_post_qmi_events

===========================================================================*/
/*!
    @brief
    Called in order to post events (received directly from qmi) to
    qcril event thread.
    Note: do not assume the context in which handler will be called
    Note: do not assume that info_tbl_ptr would be non NULL as some
    generic QMI events (such as bearer tech ind) are not related to
    a particular info_tbl entry

    @pre if info_tbl_ptr is not null, caller must have locked info_tbl_mutex
    prior to calling this function.

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_post_qmi_events(
  qcril_data_call_info_tbl_type * info_tbl_ptr,
  dsi_net_evt_t net_evt
)
{
  int ret = DSI_ERROR;
  qcril_reqlist_public_type reqlist_info;
  qcril_data_event_data_t * evt;
  RIL_Token tok = 0;
  qcril_instance_id_e_type instance_id = global_instance_id;
  qcril_modem_id_e_type modem_id = global_modem_id;

  QCRIL_LOG_DEBUG("%s","qcril_data_post_qmi_events: entry");

  do
  {

    /* validate info_tbl_ptr if it's not NULL */
    if (NULL != info_tbl_ptr)
    {
      if ((!VALIDATE_LOCAL_DATA_OBJ(info_tbl_ptr)))
      {
        QCRIL_LOG_ERROR( "invalid info_tbl_ptr [%p] ",
                         (unsigned int *)info_tbl_ptr);
        break;
      }

      /* we have a valid info_tbl_ptr, use it to
       * populate/override following local params */
      tok = info_tbl_ptr->pend_tok;
      instance_id = info_tbl_ptr->instance_id;
      modem_id = info_tbl_ptr->modem_id;
    }

    /* Allocate from heap here and clean-up on call end */
    evt = ( qcril_data_event_data_t *)malloc( sizeof( qcril_data_event_data_t ) );
    if ( evt == NULL )
    {
      QCRIL_LOG_ERROR( "%s","unable to alloc mem for event obj" );
      break;
    }
    QCRIL_DS_LOG_DBG_MEM( "event obj alloc", evt );
    memset( evt, 0, sizeof( qcril_data_event_data_t ) );

    /* Populate data event obj */
    evt->evt      = net_evt;
    evt->data     = info_tbl_ptr;
    evt->data_len = sizeof( qcril_data_call_info_tbl_type );
    evt->self     = evt;

    /*
      Call QCRIL API to process this event
      The data event hdlr will be called by RIL thread
      In case of unsol event RIL Token will be 0
    */
    QCRIL_LOG_VERBOSE( "queue QCRIL DATA event for RIL Token [%d] "     \
                       "instance_id [%d], and modem_id [%d]",
                       tok, instance_id, modem_id);

    if(E_SUCCESS != qcril_event_queue( instance_id,
                                       modem_id,
                                       QCRIL_DATA_NOT_ON_STACK,
                                       QCRIL_EVT_DATA_EVENT_CALLBACK,
                                       ( void * )evt,
                                       sizeof( qcril_data_event_data_t ),
                                       tok ))
    {
      QCRIL_LOG_ERROR("%s", "qcril_event_queue failed\n");

      /* free allocated memory for immediate failure from qcril_event_queue */
      free(evt);
      break;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    QCRIL_LOG_DEBUG("%s","qcril_data_post_qmi_events: exit with success");
  }
  else
  {
    QCRIL_LOG_ERROR("%s","qcril_data_post_qmi_events: exit with error");
  }

}/* qcril_data_post_qmi_events() */

/*===========================================================================

  FUNCTION:  qcril_data_process_pref_tech_change_ind

===========================================================================*/
/*!
    @brief
    processes pref_tech_change_ind

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
int qcril_data_process_pref_tech_change_ind
(
  qmi_wds_pref_data_sys_type pref_data_sys
)
{
  int ret = FAILURE;
  int reti = SUCCESS;
  qcril_arb_pref_data_tech_e_type qcril_arb_tech;

  do
  {
    QCRIL_LOG_DEBUG("%s", "pref data sys ind is received");
    switch(pref_data_sys)
    {
    case QMI_WDS_DATA_SYS_UNKNOWN:
      QCRIL_LOG_DEBUG("%s","QCRIL_ARB_PREF_DATA_TECH_UNKNOWN is reported");
      qcril_arb_tech = QCRIL_ARB_PREF_DATA_TECH_UNKNOWN;
      break;
    case QMI_WDS_DATA_SYS_CDMA_1X:
      QCRIL_LOG_DEBUG("%s","QCRIL_ARB_PREF_DATA_TECH_CDMA is reported");
      qcril_arb_tech = QCRIL_ARB_PREF_DATA_TECH_CDMA;
      break;
    case QMI_WDS_DATA_SYS_EVDO:
      QCRIL_LOG_DEBUG("%s","QCRIL_ARB_PREF_DATA_TECH_EVDO is reported");
      qcril_arb_tech = QCRIL_ARB_PREF_DATA_TECH_EVDO;
      break;
    case QMI_WDS_DATA_SYS_GPRS:
      QCRIL_LOG_DEBUG("%s","QCRIL_ARB_PREF_DATA_TECH_GSM is reported");
      qcril_arb_tech = QCRIL_ARB_PREF_DATA_TECH_GSM;
      break;
    case QMI_WDS_DATA_SYS_WCDMA:
      QCRIL_LOG_DEBUG("%s","QCRIL_ARB_PREF_DATA_TECH_UMTS is reported");
      qcril_arb_tech = QCRIL_ARB_PREF_DATA_TECH_UMTS;
      break;
#if 0
      /* currently, qmi wds doesn't report EHRPD */
    case QMI_WDS_DATA_SYS_EHRPD:
      qcril_arb_tech = QCRIL_ARB_PREF_DATA_TECH_EHRPD;
      break;
#endif
    case QMI_WDS_DATA_SYS_LTE:
      QCRIL_LOG_DEBUG("%s","QCRIL_ARB_PREF_DATA_TECH_LTE is reported");
      qcril_arb_tech = QCRIL_ARB_PREF_DATA_TECH_LTE;
      break;
    case QMI_WDS_DATA_SYS_TDSCDMA:
      QCRIL_LOG_DEBUG("%s","QCRIL_ARB_PREF_DATA_TECH_TDSCDMA is reported");
      qcril_arb_tech = QCRIL_ARB_PREF_DATA_TECH_TDSCDMA;
      break;
    default:
      QCRIL_LOG_ERROR("wds reported unrecognized preferred " \
                      "technology [%d]",
                      pref_data_sys);
      reti = FAILURE;
      break;
    }
    if (FAILURE == reti)
    {
      break;
    }
    qcril_arb_set_pref_data_tech(global_instance_id,
                                 qcril_arb_tech);
    ret = SUCCESS;
  } while (0);

  if (SUCCESS == ret)
  {
    /* If there's a change in the preferred system */
    if (global_dsd_info.legacy.pref_data_tech != pref_data_sys)
    {
      /* Check if we need to start partial retry for any of the calls */
      qcril_data_util_reinitiate_partial_retry(TRUE, FALSE);
    }

    global_dsd_info.legacy.pref_data_tech = pref_data_sys;

    QCRIL_LOG_DEBUG("%s","qcril_data_process_pref_tech_change_ind succeeded");
  }
  else
  {
    QCRIL_LOG_ERROR("%s","qcril_data_process_pref_tech_change_ind failed");
  }

  return ret;
}
/*===========================================================================

  FUNCTION:  qcril_data_set_is_data_enabled

===========================================================================*/
/*!
    @brief
    sends data enabled to modem if qcril passes the parameter either true
    or false.

    @return
    RIL_E_SUCCESS         :- If QMI DSD returns rc = 0
    RIL_E_GENERIC_FAILURE :- If QMI DSD returns rc = negative
*/
/*=========================================================================*/
RIL_Errno qcril_data_set_is_data_enabled(boolean is_data_enabled )
{
  qmi_client_error_type rc;
  dsd_notify_data_settings_req_msg_v01  data_settings_req_msg;
  dsd_notify_data_settings_resp_msg_v01 data_settings_resp_msg;

  if(global_qmi_dsd_hndl == NULL)
  {
    QCRIL_LOG_ERROR("%s","DSD Client unavailable");
    return RIL_E_GENERIC_FAILURE;
  }

  memset(&data_settings_req_msg, 0, sizeof(data_settings_req_msg));
  memset(&data_settings_resp_msg, 0, sizeof(data_settings_resp_msg));

  data_settings_req_msg.data_service_switch_valid = TRUE;
  data_settings_req_msg.data_service_switch = is_data_enabled;

  QCRIL_LOG_DEBUG("Setting data_enabled =%d subs_id = %d",
                    data_settings_req_msg.data_service_switch,
                    global_subs_id);

  rc = qmi_client_send_msg_sync(global_qmi_dsd_hndl,
                                  QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01,
                                  &data_settings_req_msg,
                                  sizeof(data_settings_req_msg),
                                  &data_settings_resp_msg,
                                  sizeof(data_settings_resp_msg),
                                  QCRIL_DATA_QMI_DSD_SYNC_MSG_TIMEOUT);
  if (QMI_NO_ERR != rc) {
      QCRIL_LOG_ERROR("failed to send QMI message, err=%d",
                      rc);
      return RIL_E_GENERIC_FAILURE;
  }
  else if (QMI_NO_ERR != data_settings_resp_msg.resp.result) {
      QCRIL_LOG_ERROR("failed to set data settings, response_err=%d",
                        data_settings_resp_msg.resp.error);
      return RIL_E_GENERIC_FAILURE;
  }
  return RIL_E_SUCCESS;
}

/*===========================================================================

  FUNCTION:   qcril_data_set_is_data_roaming_enabled

===========================================================================*/
/*!
    @brief
    sends roaming status to modem if user sets it manually via UI

    @return
    RIL_E_SUCCESS         :- If QMI DSD returns rc = 0
    RIL_E_GENERIC_FAILURE :- If QMI DSD returns rc = negative
*/
/*=========================================================================*/

RIL_Errno qcril_data_set_is_data_roaming_enabled(boolean is_data_roaming_enabled)
{
  qmi_client_error_type rc;
  dsd_notify_data_settings_req_msg_v01  data_settings_req_msg;
  dsd_notify_data_settings_resp_msg_v01 data_settings_resp_msg;

  if(global_qmi_dsd_hndl == NULL)
  {
    QCRIL_LOG_ERROR("%s","DSD Client unavailable");
    return RIL_E_GENERIC_FAILURE;
  }

  memset(&data_settings_req_msg, 0, sizeof(data_settings_req_msg));
  memset(&data_settings_resp_msg, 0, sizeof(data_settings_resp_msg));

  data_settings_req_msg.data_service_roaming_switch_valid = TRUE;
  data_settings_req_msg.data_service_roaming_switch = is_data_roaming_enabled;

  QCRIL_LOG_DEBUG("Setting data_roaming_enabled =%d,subs_id = %d",
                    data_settings_req_msg.data_service_roaming_switch,
                    global_subs_id );

  rc = qmi_client_send_msg_sync(global_qmi_dsd_hndl,
                                  QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01,
                                  &data_settings_req_msg,
                                  sizeof(data_settings_req_msg),
                                  &data_settings_resp_msg,
                                  sizeof(data_settings_resp_msg),
                                  QCRIL_DATA_QMI_DSD_SYNC_MSG_TIMEOUT);
    if (QMI_NO_ERR != rc) {
      QCRIL_LOG_ERROR("failed to send QMI message, err=%d",
                      rc);
      return RIL_E_GENERIC_FAILURE;
    }
    else if (QMI_NO_ERR != data_settings_resp_msg.resp.result) {
      QCRIL_LOG_ERROR("failed to set data settings,resp_err=%d",
                        data_settings_resp_msg.resp.error);
      return RIL_E_GENERIC_FAILURE;
    }
 return RIL_E_SUCCESS;
}

/*===========================================================================

  FUNCTION:   qcril_data_set_apn_info

===========================================================================*/
/*!
    @brief
    sets the apn info table

    @return
    RIL_E_SUCCESS         :- If QMI DSD returns rc = 0
    RIL_E_GENERIC_FAILURE :- If QMI DSD returns rc = negative
*/
/*=========================================================================*/

RIL_Errno qcril_data_set_apn_info(char *type, char *name,int32 is_apn_valid)
{
  qmi_client_error_type        rc;
  dsd_set_apn_info_req_msg_v01 data_setting_set_apn_param;
  dsd_set_apn_info_resp_msg_v01 data_setting_set_apn_param_resp;
  int size = 0;

  if ((strlen(name) == 0) || (NULL == type) || (strlen(type) == 0))
  {
    QCRIL_LOG_ERROR("%s","invalid parameters received");
    return RIL_E_GENERIC_FAILURE;
  }
  if (strlen(name) > QMI_DSD_MAX_APN_LEN_V01)
  {
    QCRIL_LOG_ERROR("APN string [%s] is too long [%d]." \
                    " max allowed string size is [%d]",
                    name, strlen(name),
                    QMI_DSD_MAX_APN_LEN_V01);
    return RIL_E_GENERIC_FAILURE;
  }

  memcpy( &data_setting_set_apn_param.apn_info.apn_name[0], name, QMI_DSD_MAX_APN_LEN_V01);
  data_setting_set_apn_param.apn_invalid_valid = TRUE;
  if(is_apn_valid)
  {
    data_setting_set_apn_param.apn_invalid = FALSE;
  }
  else
  {
    data_setting_set_apn_param.apn_invalid = TRUE;
  }
  QCRIL_LOG_DEBUG("Is APN valid flag set to [%d]",is_apn_valid);
  QCRIL_LOG_DEBUG("Using APN name [%s] and APN type as [%s] subs_id [%d]",name,type,global_subs_id);
  size = strlen(type);
  do
  {
    if(strncmp(type,"default",size)== 0)
    {
      data_setting_set_apn_param.apn_info.apn_type = DSD_APN_TYPE_DEFAULT_V01;
      QCRIL_LOG_DEBUG("APN type is [%s]",type);
      break;
    }
    else if (strncmp(type,"internet",size)== 0)
    {
      data_setting_set_apn_param.apn_info.apn_type = DSD_APN_TYPE_DEFAULT_V01;
      QCRIL_LOG_DEBUG("APN type is [%s]",type);
      break;
    }
    else if (strncmp(type,"ims",size)== 0)
    {
      data_setting_set_apn_param.apn_info.apn_type = DSD_APN_TYPE_IMS_V01;
      QCRIL_LOG_DEBUG("APN type is [%s]",type);
      break;
    }
    else if (strncmp(type,"fota",size)== 0)
    {
      data_setting_set_apn_param.apn_info.apn_type = DSD_APN_TYPE_ENUM_MAX_ENUM_VAL_V01;
      QCRIL_LOG_DEBUG("APN type is [%s]",type);
      return RIL_E_GENERIC_FAILURE;
    }
    else if (strncmp(type,"mms",size)== 0)
    {
      data_setting_set_apn_param.apn_info.apn_type = DSD_APN_TYPE_ENUM_MAX_ENUM_VAL_V01;
      QCRIL_LOG_DEBUG("APN type is [%s]",type);
      return RIL_E_GENERIC_FAILURE;
    }
    else if (strncmp(type,"supl",size)== 0)
    {
      data_setting_set_apn_param.apn_info.apn_type = DSD_APN_TYPE_ENUM_MAX_ENUM_VAL_V01;
      QCRIL_LOG_DEBUG("APN type is [%s]",type);
      return RIL_E_GENERIC_FAILURE;
    }
    else if (strncmp(type,"emergency",size)== 0)
    {
      data_setting_set_apn_param.apn_info.apn_type = DSD_APN_TYPE_ENUM_MAX_ENUM_VAL_V01;
      QCRIL_LOG_DEBUG("APN type is [%s]",type);
      return RIL_E_GENERIC_FAILURE;
    }
    else if (strncmp(type,"ia",size)== 0)
    {
      data_setting_set_apn_param.apn_info.apn_type = DSD_APN_TYPE_ENUM_MAX_ENUM_VAL_V01;
      QCRIL_LOG_DEBUG("APN type is [%s]",type);
      return RIL_E_GENERIC_FAILURE;
    }
    else if (strncmp(type,"cbs",size)== 0)
    {
      data_setting_set_apn_param.apn_info.apn_type = DSD_APN_TYPE_ENUM_MAX_ENUM_VAL_V01;
      QCRIL_LOG_DEBUG("APN type is [%s]",type);
      return RIL_E_GENERIC_FAILURE;
    }
    else if (strncmp(type,"dun",size)== 0)
    {
      data_setting_set_apn_param.apn_info.apn_type = DSD_APN_TYPE_ENUM_MAX_ENUM_VAL_V01;
      QCRIL_LOG_DEBUG("APN type is [%s]",type);
      return RIL_E_GENERIC_FAILURE;
    }
    else if (strncmp(type,"hipri",size)== 0)
    {
      data_setting_set_apn_param.apn_info.apn_type = DSD_APN_TYPE_ENUM_MAX_ENUM_VAL_V01;
      QCRIL_LOG_DEBUG("APN type is [%s]",type);
      return RIL_E_GENERIC_FAILURE;
    }
    else
    {
      data_setting_set_apn_param.apn_info.apn_type = DSD_APN_TYPE_ENUM_MAX_ENUM_VAL_V01;
      QCRIL_LOG_DEBUG("APN type is [%s]",type);
      return RIL_E_GENERIC_FAILURE;
    }
  }while(0);
    QCRIL_LOG_DEBUG("Setting apn info = [%s] and apn type = [%d]",
                   data_setting_set_apn_param.apn_info.apn_name,
                   data_setting_set_apn_param.apn_info.apn_type);

    rc = qmi_client_send_msg_sync(global_qmi_dsd_hndl,
                                  QMI_DSD_SET_APN_INFO_REQ_V01,
                                  &data_setting_set_apn_param,
                                  sizeof(data_setting_set_apn_param),
                                  &data_setting_set_apn_param_resp,
                                  sizeof(data_setting_set_apn_param_resp),
                                  QCRIL_DATA_QMI_DSD_SYNC_MSG_TIMEOUT);
    if (QMI_NO_ERR != rc) {
      QCRIL_LOG_ERROR("failed to send QMI message, err=%d",
                      rc);
      return RIL_E_GENERIC_FAILURE;
    }
    else if (QMI_NO_ERR != data_setting_set_apn_param_resp.resp.result) {
      QCRIL_LOG_ERROR("failed to set data settings, resp_err=%d",
                        data_setting_set_apn_param_resp.resp.error);
      return RIL_E_GENERIC_FAILURE;
    }
 return RIL_E_SUCCESS;
}


/*===========================================================================

  FUNCTION:  qcril_data_process_data_sys_status_ind

===========================================================================*/
/*!
    @brief
    processes data system status indication
    All the mapping work is done in RIL, only pass it over to upper layer

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
int qcril_data_process_data_sys_status_ind
(
  qmi_wds_data_sys_status_type  *sys_status_type
)
{
  unsigned int i, nw_info_len = 0;
  qmi_wds_data_sys_status_network_info_type  *nw_info = NULL;

  if (!sys_status_type || 0 == sys_status_type->network_info_len)
  {
    QCRIL_LOG_ERROR("Invalid parameter");
    return FAILURE;
  }

  QCRIL_LOG_DEBUG("%s", "qcril_data_process_data_sys_status_ind ENTRY");
  QCRIL_LOG_DEBUG("recevd curr_data_sys_status pref_network=0x%x, len=%d",
                  sys_status_type->pref_network,
                  sys_status_type->network_info_len);

  for(i = 0; i < sys_status_type->network_info_len; i++)
  {
    QCRIL_LOG_DEBUG("recvd curr_data_sys_status nw=0x%x, rat_mask=0x%x, so_mask=0x%x",
                    sys_status_type->network_info[i].network,
                    sys_status_type->network_info[i].rat_mask,
                    sys_status_type->network_info[i].db_so_mask );
  }

  /* In case of 1x to ehrpd or vice versa rat change abort the pending calls with older tech */
  if (TRUE == qcril_data_util_is_new_rat_1x_ehrpd(sys_status_type, &nw_info))
  {
    QCRIL_LOG_DEBUG( "data sys status changed to 1x/eHRPD, aborting incompatible calls" );
    qcril_data_abort_incompatible_pending_calls(nw_info);
  }


  /* If there's a change in the data system status, check if partial retry needs to
     be restarted for any active calls */
  if (TRUE == qcril_data_util_data_sys_status_changed(&global_dsd_info.legacy.data_sys_status,
                                                      sys_status_type))
  {
    boolean check_ce_reason;

    QCRIL_LOG_DEBUG( "data sys status has changed, initiating partial retry" );

    /* In case of a change in the preferred network, skip the CE failure code check */
    check_ce_reason = (global_dsd_info.legacy.data_sys_status.pref_network != sys_status_type->pref_network) ?
                      FALSE:
                      TRUE;
    qcril_data_util_reinitiate_partial_retry(TRUE, check_ce_reason);
  }
  else
  {
    QCRIL_LOG_DEBUG( "data sys status has not changed, skipping partial retry" );
  }

  nw_info_len = MIN(global_dsd_info.legacy.data_sys_status.network_info_len,
                    sys_status_type->network_info_len);

  memcpy(&global_dsd_info.legacy.data_sys_status.network_info[0],
         sys_status_type->network_info,
         sizeof(qmi_wds_data_sys_status_network_info_type)* nw_info_len);

  global_dsd_info.legacy.data_sys_status.network_info_len = nw_info_len;
  global_dsd_info.legacy.data_sys_status.pref_network = sys_status_type->pref_network;


  QCRIL_LOG_DEBUG( "Updated legacy len - %d,MIN len -%d",
                   global_dsd_info.legacy.data_sys_status.network_info_len, nw_info_len);

  qcril_arb_set_data_sys_status(global_instance_id,
                                &global_dsd_info.legacy.data_sys_status);

  QCRIL_LOG_DEBUG("%s", "qcril_data_process_data_sys_status_ind EXIT");

  return SUCCESS;
}

/*===========================================================================

  FUNCTION:  qcril_data_process_tethered_state_change_ind

===========================================================================*/
/*!
    @brief
    processes tethered_state_change_ind

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
int qcril_data_process_tethered_state_change_ind
(
  qmi_wds_data_call_status_type data_call_status,
  qmi_wds_data_call_type_type   data_call_type
)
{
  int ret = FAILURE;
  int reti = SUCCESS;

  QCRIL_LOG_DEBUG("%s", "tethered data call status is received");

  do
  {
    if( data_call_type != QMI_WDS_DATA_CALL_TYPE_TETHERED )
    {
      QCRIL_LOG_DEBUG( "call type[%d] not tethered data, ignoring",
                       data_call_type );
      ret = SUCCESS;
      break;
    }
    else
    {
      switch( data_call_status )
      {
        case QMI_WDS_DATA_CALL_ACTIVATED:
          QCRIL_LOG_DEBUG("%s", "tethered data call is activated "
                          "on the modem side");
          global_tethered_state = QCRIL_TETHERED_MODE_ON;
          break;
        case QMI_WDS_DATA_CALL_TERMINATED:
          global_tethered_state = QCRIL_TETHERED_MODE_OFF;
          QCRIL_LOG_DEBUG("%s", "tethered data call is terminated "
                          "on the modem side");

          /* Since a tethered call got terminated, check if we need to
             start our partial retries */
          qcril_data_util_reinitiate_partial_retry(FALSE, TRUE);
          break;
        default:
          QCRIL_LOG_ERROR("wds reported unrecognized data "
                          "call status on modem side [%d]",
                          data_call_status);
          reti = FAILURE;
          break;
      }
      if (FAILURE == reti)
      {
        break;
      }
      /* info_tbl_mutex need not be locked in this case because
         info_tbl_ptr is NULL */
      qcril_data_post_qmi_events(NULL,
                                 QCRIL_DATA_EV_DATA_CALL_STATUS_CHANGED);
      ret = SUCCESS;
    }
  } while (0);

  if (SUCCESS == ret)
  {
    QCRIL_LOG_DEBUG("%s","qcril_data_process_tethered_state_change_ind succeeded");
  }
  else
  {
    QCRIL_LOG_ERROR("%s","qcril_data_process_tethered_state_change_ind failed");
  }

  return ret;
}

/*===========================================================================

  FUNCTION:  qcril_data_process_ext_ip_config_ind

===========================================================================*/
/*!
    @brief
    Processes extended IP config change indication

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_process_ext_ip_config_ind
(
  qcril_instance_id_e_type        instance_id,
  int                             qmi_wds_hndl,
  qmi_wds_ext_ip_config_ind_type  ext_ip_ind
)
{
  qmi_wds_profile_id_type      profile_id;
  qmi_wds_profile_params_type  profile_params;
  qmi_wds_curr_call_info_type  call_info;
  qmi_wds_req_runtime_settings_params_type  req_mask = QMI_WDS_GET_CURR_CALL_INFO_OPERATOR_RESERVED_PCO_PARAM_MASK;
  int rc, qmi_err;

  QCRIL_LOG_FUNC_ENTRY();

  if (QCRIL_DATA_HNDL_INVALID == qmi_wds_hndl)
  {
    QCRIL_LOG_ERROR( "invalid input parameter" );
    goto bail;
  }

  if (ext_ip_ind & QMI_WDS_GET_CURR_CALL_INFO_OPERATOR_RESERVED_PCO_PARAM_MASK)
  {
    memset(&call_info, 0, sizeof(call_info));

    /* Query Modem for requested parameters */
    if (QMI_NO_ERR != (rc = qmi_wds_get_curr_call_info( qmi_wds_hndl,
                                                        req_mask,
                                                        &profile_id,
                                                        &profile_params,
                                                        &call_info,
                                                        &qmi_err )))
    {
      QCRIL_LOG_ERROR("failed on qmi_wds_get_curr_call_info: rc=%d qmi_err=%d\n",
                      rc,
                      qmi_err);
      goto bail;
    }

    /* Send the operator reserved PCO information to the Telephony layer */
    if (QMI_WDS_CURR_CALL_INFO_OPERATOR_RESERVED_PCO & call_info.mask)
    {
      QCRIL_LOG_DEBUG( "Operator reserved PCO info update" );
      QCRIL_LOG_DEBUG( "MCC: 0x%x", (int)call_info.operator_pco.mcc );
      QCRIL_LOG_DEBUG( "MNC: 0x%x", (int)call_info.operator_pco.mnc );
      QCRIL_LOG_DEBUG( "MNC includes PCS digit: %s", (call_info.operator_pco.mnc_includes_pcs_digit)?
                                                     "TRUE": "FALSE" );
      QCRIL_LOG_DEBUG( "App info len: %d", (int)call_info.operator_pco.app_specific_info_len );
      QCRIL_LOG_DEBUG( "Container ID: 0x%x", (int)call_info.operator_pco.container_id );

      qcril_hook_unsol_response(instance_id,
                                QCRIL_EVT_HOOK_UNSOL_OPERATOR_RESERVED_PCO,
                                (char*)&call_info.operator_pco,
                                sizeof(call_info.operator_pco));
    }
  }

bail:
  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_data_process_wds_ind

===========================================================================*/
/*!
    @brief
    Processes qmi wds indications
    Currently, this function is responsible for processing
    physlink events - these are specific to an info_tbl entry (or a call)
    bearer tech ind - these are generic events

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_process_wds_ind
(
  int wds_hndl,
  qmi_service_id_type sid,
  void* user_data,
  qmi_wds_indication_id_type ind_id,
  qmi_wds_indication_data_type* ind_data
)
{
  int ret = FAILURE;
  int reti = SUCCESS;
  qcril_data_call_info_tbl_type * info_tbl_ptr = NULL;
  int all_calls_dormant;

  QCRIL_LOG_DEBUG("%s","qcril_data_process_wds_ind: entry");

  do
  {
    /* sanity checking */
    if (wds_hndl < 0)
    {
      QCRIL_LOG_ERROR("invalid wds hndl received [%d]", wds_hndl);
      break;
    }

    if (NULL == ind_data)
    {
      QCRIL_LOG_ERROR("%s","NULL ind_data recieved");
      break;
    }

    /* if a non NULL user_data is received, validate it */
    if (NULL != user_data)
    {
      info_tbl_ptr = (qcril_data_call_info_tbl_type *)user_data;
      if(!VALIDATE_LOCAL_DATA_OBJ(info_tbl_ptr))
      {
        /* The call may have been cleaned up through dsi netctrl
           NO_NET event.  Do not report invalid user_data error
           if the self pointer is null.*/
        if (info_tbl_ptr->self != NULL)
        {
          QCRIL_LOG_ERROR("%s","invalid info_tbl_ptr (user_data)" \
                          "received by qcril_data_process_wds_ind");
        }
        break;
      }

      if (info_tbl_ptr->qmi_wds_hndl != wds_hndl)
      {
        QCRIL_LOG_ERROR("wds hndl mismatch [0x%x] passed by qmi versus" \
                        "[0x%x] stored in info_tbl entry",
                        wds_hndl,
                        info_tbl_ptr->qmi_wds_hndl);
        break;
      }
    }
    /* if NULL user_data is received, now try to
     * match wds_hndl with our global wds hndl */
    else if (wds_hndl != global_qmi_wds_hndl)
    {
      QCRIL_LOG_ERROR("wds indication arrived with a handle [0x%x] "    \
                      "that we do not recognize", wds_hndl);
      break;
    }

    /* handle individual wds indications */
    reti = SUCCESS;
    QCRIL_LOG_DEBUG("qcril_data_process_wds_ind: Handling ind [%x] with QMI service ID type [%x]",
                    ind_id, (unsigned int) sid);


    switch (ind_id)
    {
    case QMI_WDS_SRVC_EVENT_REPORT_IND_MSG:
      if (ind_data->event_report.event_mask &
          QMI_WDS_EVENT_DORM_STATUS_IND)
      {
        switch(ind_data->event_report.dorm_status)
        {
        case QMI_WDS_DORM_STATUS_DORMANT:
          QCRIL_LOG_DEBUG("%s","PHYSLINK went dormant");
          /* we mandate that physlink event be associated
           * with a valid call (i.e. info_tbl_ptr), otherwise
           * it's ignored  */
          if (NULL == info_tbl_ptr)
          {
            QCRIL_LOG_DEBUG("%s", "generic physlink indication is ignored" \
                            " by qcril_data_netctrl");
            break;
          }

          info_tbl_ptr->dorm_status = QMI_WDS_DORM_STATUS_DORMANT;
          all_calls_dormant = qcril_data_all_calls_dormant();
          if (all_calls_dormant &&
              global_data_sys_ind_switch == LIMITED_SYS_INDICATIONS_ON)
          {
            qcril_data_reg_sys_ind(LIMITED_SYS_INDICATIONS_ON);
          }

          qcril_data_post_qmi_events(info_tbl_ptr,
                                     DSI_EVT_PHYSLINK_DOWN_STATE);
          break;
        case QMI_WDS_DORM_STATUS_ACTIVE:
          QCRIL_LOG_DEBUG("%s","PHYSLINK went active");
          /* we mandate that physlink event be associated
           * with a valid call (i.e. info_tbl_ptr), otherwise
           * it's ignored  */
          if (NULL == info_tbl_ptr)
          {
            QCRIL_LOG_DEBUG("%s", "generic physlink indication is ignored" \
                            " by qcril_data_netctrl");
            break;
          }

          info_tbl_ptr->dorm_status = QMI_WDS_DORM_STATUS_ACTIVE;
          if (global_data_sys_ind_switch == LIMITED_SYS_INDICATIONS_ON)
          {
            qcril_data_reg_sys_ind(LIMITED_SYS_INDICATIONS_OFF);
          }

          qcril_data_post_qmi_events(info_tbl_ptr,
                                     DSI_EVT_PHYSLINK_UP_STATE);
          break;
        default:
          QCRIL_LOG_ERROR("invalid dorm status [%d] received",
                          ind_data->event_report.dorm_status);
          reti = FAILURE;
          break;
        }
      }
      if ( QCRIL_DATA_LEGACY_DSD_MODE_PREF_DATA_TECH == global_dsd_info.legacy.dsd_mode &&
          (ind_data->event_report.event_mask & QMI_WDS_EVENT_PREF_DATA_SYS_IND))
      {
        if (SUCCESS != qcril_data_process_pref_tech_change_ind(
              ind_data->event_report.pref_data_sys))
        {
          QCRIL_LOG_ERROR("%s","qcril_data_process_pref_tech_change_ind "\
                          "failed");
          reti = FAILURE;
          break;
        }
      }

      if ( QCRIL_DATA_LEGACY_DSD_MODE_DATA_SYS_STATUS == global_dsd_info.legacy.dsd_mode &&
          (ind_data->event_report.event_mask & QMI_WDS_EVENT_DATA_SYS_STATUS_IND))
      {
        if (SUCCESS != qcril_data_process_data_sys_status_ind(
              &ind_data->event_report.data_sys_status))
        {
          QCRIL_LOG_ERROR("%s", "qcril_data_process_data_sys_status_ind failed");
          reti = FAILURE;
          break;
        }
      }

      if( (ind_data->event_report.event_mask & QMI_WDS_EVENT_DATA_CALL_STATUS_CHG_IND) &&
          (ind_data->event_report.event_mask & QMI_WDS_EVENT_DATA_CALL_TYPE_IND) )
      {
        if (SUCCESS != qcril_data_process_tethered_state_change_ind
                         (ind_data->event_report.data_call_status_change.data_call_status,
                          ind_data->event_report.data_call_type.data_call_type))
        {
          QCRIL_LOG_ERROR("%s","qcril_data_process_tethered_state_change_ind "\
                          "failed");
          reti = FAILURE;
          break;
        }
      }
      break;

    case QMI_WDS_SRVC_EXT_IP_CONFIG_IND_MSG:
      /* Process extended IP config indication */
      qcril_data_process_ext_ip_config_ind((info_tbl_ptr) ? info_tbl_ptr->instance_id :
                                                            (unsigned int) global_instance_id,
                                           wds_hndl,
                                           ind_data->ext_ip_ind);
      break;

    default:
      QCRIL_LOG_DEBUG("Ignoring wds ind cb event %d", ind_id);
      break;
    }
    if (FAILURE == reti)
    {
      break;
    }

    ret = SUCCESS;
  } while (0);

  if (SUCCESS == ret)
  {
    QCRIL_LOG_DEBUG("%s","qcril_data_process_wds_ind: exit with success");
  }
  else
  {
    QCRIL_LOG_ERROR("%s","qcril_data_process_wds_ind: exit with error");
  }
}/*qcril_data_process_wds_ind*/

/*===========================================================================

  FUNCTION:  qcril_data_qmi_wds_ind_cb

===========================================================================*/
/*!
    @brief
    Receives qmi wds indications

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_qmi_wds_ind_cb
(
  int wds_hndl,
  qmi_service_id_type sid,
  void * user_data,
  qmi_wds_indication_id_type ind_id,
  qmi_wds_indication_data_type * ind_data
)
{
  qcril_data_wds_event_data_t* event_data = NULL;
  int lock_result = 0;

  QCRIL_LOG_DEBUG( "%s", "qcril_data_qmi_wds_ind_cb: ENTRY" );

  QCRIL_LOG_DEBUG( "%s %d ind", "qcril_data_qmi_wds_ind_cb",ind_id );

  if ((event_data = malloc(sizeof(qcril_data_wds_event_data_t))) != NULL)
  {
    /* Lock could not be acquired without blocking.  Make a deep copy of wds
       indication data and post the event to QCRIL for processing. */
    event_data->wds_hndl = wds_hndl;
    event_data->sid = sid;
    event_data->user_data = user_data;
    event_data->ind_id = ind_id;

    memcpy(&event_data->ind_data, ind_data, sizeof(event_data->ind_data));

    /* Make a copy of the network_info data if valid */
    if ((ind_data->event_report.event_mask & QMI_WDS_EVENT_DATA_SYS_STATUS_IND) &&
        NULL != ind_data->event_report.data_sys_status.network_info)
    {
      qmi_wds_data_sys_status_network_info_type  *nw_info = NULL;
      unsigned int nw_info_len = ind_data->event_report.data_sys_status.network_info_len;

      nw_info = malloc(sizeof(qmi_wds_data_sys_status_network_info_type) * nw_info_len);

      if (!nw_info)
      {
        QCRIL_LOG_ERROR( "%s", "qcril_data_qmi_wds_ind_cb: failed to allocate nw_info" );
        free(event_data);
        goto bail;
      }

      memcpy(nw_info,
            ind_data->event_report.data_sys_status.network_info,
            sizeof(qmi_wds_data_sys_status_network_info_type) * nw_info_len);

      event_data->ind_data.event_report.data_sys_status.network_info = nw_info;
    }

    if(E_SUCCESS != qcril_event_queue(global_instance_id,
                                      global_modem_id,
                                      QCRIL_DATA_NOT_ON_STACK,
                                      QCRIL_EVT_DATA_WDS_EVENT_CALLBACK,
                                      event_data,
                                      sizeof(qcril_data_wds_event_data_t),
                                     (RIL_Token) QCRIL_TOKEN_ID_INTERNAL))
    {
      QCRIL_LOG_ERROR("%s", "qcril_event_queue failed\n");

      /* free allocated memory for immediate failure from qcril_event_queue */
      if(NULL != event_data->ind_data.event_report.data_sys_status.network_info)
      {
        free(event_data->ind_data.event_report.data_sys_status.network_info);
        event_data->ind_data.event_report.data_sys_status.network_info = NULL;
      }
      free(event_data);
    }
    /* If E_SUCCESS, event_data and its inner pointer will be freed after qm_wds indication
    is processed, at qcril_data_wds_event_hdlr */
  }

bail:
  QCRIL_LOG_DEBUG( "%s", "qcril_data_qmi_wds_ind_cb: EXIT" );
}

/*===========================================================================

  FUNCTION:  qcril_data_get_qmi_dsd_subscription_id

===========================================================================*/
/*!
    @brief
    Returns the subscription ID corresponding to the current RIL instance

    @return
    None.
*/
/*=========================================================================*/
static dsd_bind_subscription_enum_v01
qcril_data_get_qmi_dsd_subscription_id(void)
{
  dsd_bind_subscription_enum_v01  sub_num;

  switch (global_subs_id)
  {
#if 0
    case QCRIL_THIRD_INSTANCE_ID:
      sub_num = DSD_TERTIARY_SUBS_V01;
      break;
#endif

    case QMI_WDS_SECONDARY_SUBS:
      sub_num = DSD_SECONDARY_SUBS_V01;
      break;

    default:
      sub_num = DSD_PRIMARY_SUBS_V01;
      break;
  }

  return sub_num;
}

/*===========================================================================

  FUNCTION:  qcril_data_process_qmi_dsd_ind

===========================================================================*/
/*!
    @brief
    Processes the QMI DSD indications

    @return
    None.
*/
/*=========================================================================*/
static void qcril_data_process_qmi_dsd_ind
(
  dsd_system_status_ind_msg_v01  *ind_data
)
{
  dsd_system_status_info_type_v01  *info_old = NULL;
  dsd_system_status_info_type_v01  *info_new = NULL;
  unsigned int i;

  QCRIL_LOG_DEBUG( "%s", "qcril_data_process_qmi_dsd_ind: ENTRY" );

  if (!ind_data)
  {
    QCRIL_LOG_ERROR( "invalid input" );
    return;
  }

  if (QCRIL_DATA_DSD_SERVICE_TYPE_QMI_DSD != global_dsd_info.service)
  {
    QCRIL_LOG_ERROR( "unexpected QMI-DSD indication" );
    return;
  }

  /* If the available systems in invalid, report NULL bearer to RIL */
  if (FALSE == ind_data->avail_sys_valid)
  {
    QCRIL_LOG_DEBUG( "invalid avail sys, reporting NULL bearer" );

    ind_data->avail_sys_valid = TRUE;
    ind_data->avail_sys_len = 1;
    ind_data->avail_sys[0].technology = DSD_SYS_NETWORK_3GPP_V01;
    ind_data->avail_sys[0].rat_value = DSD_SYS_RAT_EX_NULL_BEARER_V01;
    ind_data->avail_sys[0].so_mask = 0ull;

    memcpy(&global_dsd_info.qmi_dsd, ind_data, sizeof(dsd_system_status_ind_msg_v01));
  }
  else
  {
    QCRIL_LOG_DEBUG("rcvd avail_sys_len=0x%x", ind_data->avail_sys_len);

    for (i = 0 ; i < ind_data->avail_sys_len; ++i)
    {
      QCRIL_LOG_DEBUG("sys[%d] network=0x%x, rat=0x%08x, so_mask=0x%016llx",
                      i,
                      ind_data->avail_sys[i].technology,
                      ind_data->avail_sys[i].rat_value,
                      ind_data->avail_sys[i].so_mask);
    }

    if(TRUE == ind_data->apn_avail_sys_info_valid)
    {
      QCRIL_LOG_DEBUG("rcvd apn_avil_sys_info_len=0x%x", ind_data->apn_avail_sys_info_len);

      for (i = 0 ; i < ind_data->apn_avail_sys_info_len; ++i)
      {
        QCRIL_LOG_DEBUG("pdn[%d] name=%s",
                        i,
                        ind_data->apn_avail_sys_info[i].apn_name);
      }
    }
    info_old = &global_dsd_info.qmi_dsd.avail_sys[0];
    info_new = &ind_data->avail_sys[0];

    /* If there's a change in the data system status, check if partial retry needs to
       be restarted for any active calls */
    if (DSD_SYS_RAT_EX_NULL_BEARER_V01 != info_new->rat_value &&
        (info_old->technology != info_new->technology ||
         info_old->rat_value  != info_new->rat_value  ||
         info_old->so_mask    != info_new->so_mask))
    {
      boolean check_ce_reason;

      QCRIL_LOG_DEBUG( "data sys status has changed, initiating partial retry" );

      /* In case of a change in the network, skip the CE failure code check */
      check_ce_reason = ((info_old->technology != info_new->technology) ? FALSE : TRUE);

      qcril_data_util_reinitiate_partial_retry(TRUE, check_ce_reason);
    }
    else
    {
      QCRIL_LOG_DEBUG( "data sys status has not changed, skipping partial retry" );
    }

    if (DSD_SYS_RAT_EX_NULL_BEARER_V01 != info_new->rat_value)
    {
      memcpy(&global_dsd_info.qmi_dsd, ind_data, sizeof(dsd_system_status_ind_msg_v01));
    }
    else
    {
      QCRIL_LOG_DEBUG( "NULL bearer reported, skipping cache update" );
    }
  }

  /* Update RIL with the DSD info */
  qcril_arb_set_dsd_sys_status(ind_data);

  QCRIL_LOG_DEBUG("%s", "qcril_data_process_qmi_dsd_ind EXIT");
}

/*===========================================================================

  FUNCTION:  qcril_data_qmi_dsd_ind_cb

===========================================================================*/
/*!
    @brief
    Receives QMI-DSD indications

    @return
    None.
*/
/*=========================================================================*/
static void qcril_data_qmi_dsd_ind_cb
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  unsigned char                  *ind_buf,
  int                            ind_buf_len,
  void                           *ind_cb_data
)
{
  dsd_system_status_ind_msg_v01  *dsd_ind_msg = NULL;
  int                           rc = QMI_INTERNAL_ERR;


  if (!user_handle || !ind_buf)
  {
    QCRIL_LOG_ERROR("qcril_data_qmi_dsd_ind_cb: bad param(s)");
    goto bail;
  }

  QCRIL_LOG_DEBUG("qcril_data_qmi_dsd_ind_cb: recvd ind=%lu", msg_id);

  dsd_ind_msg = calloc(1, sizeof(dsd_system_status_ind_msg_v01));

  if (!dsd_ind_msg)
  {
    QCRIL_LOG_ERROR("qcril_data_qmi_dsd_ind_cb: failed to alloc memory");
    goto bail;
  }

  switch (msg_id)
  {
    case QMI_DSD_SYSTEM_STATUS_IND_V01:
      /* Decode the QMI indication message to its corresponding C structure */
      rc = qmi_client_message_decode(user_handle,
                                     QMI_IDL_INDICATION,
                                     msg_id,
                                     ind_buf,
                                     ind_buf_len,
                                     dsd_ind_msg,
                                     sizeof(*dsd_ind_msg));

      if (QMI_NO_ERR != rc)
      {
        QCRIL_LOG_ERROR("qcril_data_qmi_dsd_ind_cb: indication decode failed err=%d", rc);
        goto bail;
      }

      qcril_event_queue(global_instance_id,
                        global_modem_id,
                        QCRIL_DATA_NOT_ON_STACK,
                        QCRIL_EVT_DATA_DSD_EVENT_CALLBACK,
                        dsd_ind_msg,
                        sizeof(*dsd_ind_msg),
                        (RIL_Token) QCRIL_TOKEN_ID_INTERNAL);
      break;

    default:
      break;
  }

bail:
  if (QMI_NO_ERR !=rc && NULL != dsd_ind_msg)
  {
    free(dsd_ind_msg);
  }
  return;
}


/*===========================================================================

  FUNCTION:  qcril_data_net_cb

===========================================================================*/
/*!
    @brief
    Called on call control events from Data Services (DSI).

    @pre Before calling, dsi_cb_tbl_mutex must not be locked by the
         calling thread

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_net_cb(
  dsi_hndl_t     dsi_hndl,
  void          *user_data,
  dsi_net_evt_t      net_evt,
  dsi_evt_payload_t *payload
)
{
  qcril_data_dsi_cb_tbl_type *dsi_cb_tbl_ptr = NULL;

  QCRIL_LOG_DEBUG("%s", "qcril_data_net_cb: ENTRY");

  QCRIL_DATA_MUTEX_LOCK(&dsi_cb_tbl_mutex);

  /* Input Validation, user data was pointer to dsi_cb_tbl */
  dsi_cb_tbl_ptr = ( qcril_data_dsi_cb_tbl_type *) user_data;
  if (NULL == dsi_cb_tbl_ptr)
  {
    QCRIL_LOG_ERROR("%s","qcril_data_net_cb:invalid arg, user_data is NULL");
    goto err_bad_input;
  }

  if ( ( NULL == dsi_cb_tbl_ptr->info_tbl_ptr ) ||
       ( dsi_hndl != dsi_cb_tbl_ptr->dsi_hndl ) ||
       ( QCRIL_DATA_INVALID_INSTANCE_ID == dsi_cb_tbl_ptr->instance_id ) ||
       ( QCRIL_DATA_INVALID_MODEM_ID == dsi_cb_tbl_ptr->modem_id) )
  {
    QCRIL_LOG_ERROR( "invalid arg, user_data [%p], info_tbl_ptr [%p], "
                     "dsi_hndl [%p], dsi_cb_tbl_ptr->dsi_hndl [%p], "
                     "instance_id [%#x], modem_id [%#x], payload [%p]",
                     (unsigned int *)user_data,
                     (unsigned int *)dsi_cb_tbl_ptr->info_tbl_ptr,
                     (unsigned int *)dsi_hndl,
                     (unsigned int *)dsi_cb_tbl_ptr->dsi_hndl,
                     (unsigned int)dsi_cb_tbl_ptr->instance_id,
                     (unsigned int)dsi_cb_tbl_ptr->modem_id,
                     payload );
    goto err_bad_input;
  }

  qcril_data_post_dsi_netctrl_event(dsi_cb_tbl_ptr->instance_id,
                                    dsi_cb_tbl_ptr->modem_id,
                                    dsi_cb_tbl_ptr->pend_tok,
                                    dsi_cb_tbl_ptr->info_tbl_ptr,
                                    net_evt,
                                    payload );

  QCRIL_DATA_MUTEX_UNLOCK(&dsi_cb_tbl_mutex);

  QCRIL_LOG_DEBUG("%s","qcril_data_net_cb: EXIT with suc");
  return;

err_bad_input:
  QCRIL_DATA_MUTEX_UNLOCK(&dsi_cb_tbl_mutex);
  QCRIL_LOG_ERROR("%s","qcril_data_net_cb: EXIT with err");
  return;
}/* qcril_data_net_cb() */


#ifdef FEATURE_DATA_EMBMS
/*===========================================================================
  FUNCTION:  qcril_data_handle_embms_events

===========================================================================*/
/*!
    @brief
    Handles embms events. This is an internal function.

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
static void
qcril_data_handle_embms_events
(
  qcril_instance_id_e_type       instance_id,
  qcril_data_call_info_tbl_type *info_tbl_ptr,
  qcril_data_event_data_t       *evt_info_ptr
)
{

  embms_activate_tmgi_resp_msg_v01             activate_response;
  embms_deactivate_tmgi_resp_msg_v01           deactivate_response;
  qcril_request_resp_params_type               resp;
  embms_unsol_active_tmgi_ind_msg_v01          active_indication;
  qcril_embms_enable_response_payload_type     enable_response;
  qcril_embms_disable_indication_payload_type  disable_indication;
  qcril_modem_id_e_type                        modem_id;

  RIL_Errno reti = RIL_E_GENERIC_FAILURE;
  qcril_data_embms_tmgi_requests_type  *tmgi_txn;

  QCRIL_DS_ASSERT( evt_info_ptr != NULL, "validate pend RIL Token" );
  QCRIL_DS_ASSERT( info_tbl_ptr != NULL, "validate pend RIL Token" );

  QCRIL_LOG_DEBUG("%s", "qcril_data_handle_embms_events: ENTRY");

  QCRIL_LOG_DEBUG("%s", "Print Transaction List and its entries");

  if (DSI_NET_TMGI_ACTIVATED == evt_info_ptr->evt ||
      DSI_NET_TMGI_DEACTIVATED == evt_info_ptr->evt ||
      DSI_NET_TMGI_ACTIVATED_DEACTIVATED == evt_info_ptr->evt)
  {
     for (tmgi_txn  = (qcril_data_embms_tmgi_requests_type *) list_peek_front (&info_tbl_ptr->embms_txns_list);
           tmgi_txn != NULL;
           tmgi_txn  = (qcril_data_embms_tmgi_requests_type *)
                       list_peek_next (&info_tbl_ptr->embms_txns_list, &(tmgi_txn->link)))
     {
       QCRIL_LOG_DEBUG(" request:%s, and token:%d, TMGI:%x%x%x%x%x%x DEACT_TMGI:%x%x%x%x%x%x into the transactions list",
                       qcril_log_lookup_event_name(tmgi_txn->pend_req),
                       qcril_log_get_token_id(tmgi_txn->pend_tok ),
                       tmgi_txn->tmgi[0],
                       tmgi_txn->tmgi[1],
                       tmgi_txn->tmgi[2],
                       tmgi_txn->tmgi[3],
                       tmgi_txn->tmgi[4],
                       tmgi_txn->tmgi[5],
                       tmgi_txn->deact_tmgi[0],
                       tmgi_txn->deact_tmgi[1],
                       tmgi_txn->deact_tmgi[2],
                       tmgi_txn->deact_tmgi[3],
                       tmgi_txn->deact_tmgi[4],
                       tmgi_txn->deact_tmgi[5]);

     }

     if(NULL != evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list &&
        0 != evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list_len)
     {
       QCRIL_LOG_DEBUG("Processing TMGI received from modem ACTIVATE:%x%x%x%x%x%x",
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[0].tmgi[0],
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[0].tmgi[1],
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[0].tmgi[2],
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[0].tmgi[3],
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[0].tmgi[4],
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[0].tmgi[5]);
     }
     if(NULL != evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list &&
        0 != evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list_len)
     {
       QCRIL_LOG_DEBUG("Processing TMGI received from modem DEACTIVATE:%x%x%x%x%x%x",
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list[0].tmgi[0],
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list[0].tmgi[1],
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list[0].tmgi[2],
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list[0].tmgi[3],
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list[0].tmgi[4],
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list[0].tmgi[5]
                     );
     }
  }
  /* Matching Algorithm Assumption: Indication responses
  ** for requests are gaurenteed to be received in order sent.
  */
  switch(evt_info_ptr->evt)
  {
    case DSI_NET_SAI_LIST_CHANGED:
    {
      embms_unsol_sai_ind_msg_v01  sai_indication;
      unsigned int list_index;

      QCRIL_LOG_INFO(">>>DSI_NET_SAI_LIST_CHANGED: START>>> cid [%d],"
                     "index [%d]",
                     info_tbl_ptr->cid,
                     info_tbl_ptr->index);

      memset(&sai_indication, 0, sizeof(sai_indication));

      sai_indication.available_sai_list_len = evt_info_ptr->payload.embms_sai_info.available_sai_list_len;
      /* validate available_sai_list_len */
      if (sai_indication.available_sai_list_len > SAI_MAX_V01)
      {
        QCRIL_LOG_ERROR(" Oversize available_sai_list_len, Drop SAI_LIST indication:%d",
                        sai_indication.available_sai_list_len);
        goto err_ret;
      }
      for (list_index = 0; list_index < sai_indication.available_sai_list_len; list_index ++)
      {
         sai_indication.available_sai_list[list_index] =
              evt_info_ptr->payload.embms_sai_info.available_sai_list[list_index];
      }

      sai_indication.camped_sai_list_len = evt_info_ptr->payload.embms_sai_info.camped_sai_list_len;
      /* validate camped_sai_list_len */
      if (sai_indication.camped_sai_list_len > SAI_PER_FREQ_MAX_V01)
      {
        QCRIL_LOG_ERROR(" Oversize camped_sai_list_len, Drop SAI_LIST indication:%d",
                        sai_indication.camped_sai_list_len);
        goto err_ret;
      }
      for (list_index = 0; list_index < sai_indication.camped_sai_list_len; list_index ++)
      {
         sai_indication.camped_sai_list[list_index] =
              evt_info_ptr->payload.embms_sai_info.camped_sai_list[list_index];
      }

      sai_indication.num_of_sai_per_group_len = evt_info_ptr->payload.embms_sai_info.num_sai_per_group_len;
      /* validate num_of_sai_per_group_len */
      if (sai_indication.num_of_sai_per_group_len > FREQ_MAX_V01)
      {
        QCRIL_LOG_ERROR(" Oversize num_of_sai_per_group_len, Drop SAI_LIST indication:%d",
                        sai_indication.num_of_sai_per_group_len);
        goto err_ret;
      }
      for (list_index = 0; list_index < sai_indication.num_of_sai_per_group_len; list_index ++)
      {
         sai_indication.num_of_sai_per_group[list_index] =
              evt_info_ptr->payload.embms_sai_info.num_sai_per_group[list_index];
      }

      sai_indication.dbg_trace_id = evt_info_ptr->payload.embms_sai_info.dbg_trace_id;

      QCRIL_LOG_INFO( "EMBMS SAI list indication, available_sai_list_len: [%d],"
                      "camped_sai_list_len: [%d],"
                      "num_of_sai_per_group_len: [%d],"
                      "dbg_trace_id: [%d]",
                      sai_indication.available_sai_list_len,
                      sai_indication.camped_sai_list_len,
                      sai_indication.num_of_sai_per_group_len,
                      sai_indication.dbg_trace_id);

      unsigned int i;
      for (i = 0; i < sai_indication.available_sai_list_len; i++)
      {
         QCRIL_LOG_INFO("available_sai_list:%d", sai_indication.available_sai_list[i]);
      }
      for (i = 0; i < sai_indication.camped_sai_list_len; i++)
      {
         QCRIL_LOG_INFO("camped_sai_list:%d", sai_indication.camped_sai_list[i]);
      }
      for (i = 0; i < sai_indication.num_of_sai_per_group_len; i++)
      {
         QCRIL_LOG_INFO("num_of_sai_per_group:%d", sai_indication.num_of_sai_per_group[i]);
      }
      qcril_hook_unsol_response(instance_id,
                                QCRIL_EVT_HOOK_EMBMS_UNSOL_SAI_LIST,
                                (char*)&sai_indication,
                                sizeof(sai_indication));
    }
    break;

    case DSI_NET_TMGI_LIST_CHANGED:
    {
      QCRIL_LOG_INFO( ">>>DSI_NET_TMGI_LIST_CHANGED: START>>> cid [%d],"
                      "index [%d], list indication type [%d]",
                        info_tbl_ptr->cid,
                        info_tbl_ptr->index,
                        evt_info_ptr->payload.embms_tmgi_info.list_type );

      switch(evt_info_ptr->payload.embms_tmgi_info.list_type)
      {
        case QMI_WDS_EMBMS_TMGI_LIST_ACTIVE:
          {

            int index = 0;

            memset(&active_indication, 0, sizeof(active_indication));
            active_indication.tmgi_info_len =
              evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list_len;

            /* validate tmgi_info_len */
            if(active_indication.tmgi_info_len > NUMBER_MAX_V01)
            {
              QCRIL_LOG_ERROR( "Oversize tmgi_info_len, Dropping TMGI list indication:%d",
                                 active_indication.tmgi_info_len );
              goto err_ret;
            }

            for (index = 0; index < (int)active_indication.tmgi_info_len; index++)
            {
              active_indication.tmgi_info[index].tmgi_len = TMGI_LENGTH_MAX_V01;
              memcpy(&(active_indication.tmgi_info[index].tmgi),
                     &(evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[index].tmgi),
                     TMGI_LENGTH_MAX_V01);
            }

            active_indication.dbg_trace_id =
              evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.dbg_trace_id;

            QCRIL_LOG_INFO( "EMBMS ACTIVE list ind debug_id [%d], num TMGI's [%d]",
                            active_indication.dbg_trace_id, active_indication.tmgi_info_len);

            qcril_hook_unsol_response(instance_id,
                                      QCRIL_EVT_HOOK_EMBMS_UNSOL_ACTIVE_TMGI,
                                      (char*)&active_indication,
                                      sizeof(active_indication));
          }
          break;

        case QMI_WDS_EMBMS_TMGI_LIST_OOS_WARNING:
          {
            int index = 0;
            embms_unsol_oos_warning_ind_msg_v01   oos_warning_ind;

            memset(&oos_warning_ind, 0, sizeof(oos_warning_ind));

            oos_warning_ind.tmgi_info_len =
              evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list_len;

            /* validate tmgi_info_len */
            if(oos_warning_ind.tmgi_info_len > NUMBER_MAX_V01)
            {
              QCRIL_LOG_ERROR( "Oversize tmgi_info_len, Dropping TMGI list indication:%d",
                                 oos_warning_ind.tmgi_info_len );
              goto err_ret;
            }

            for (index = 0; index < (int)oos_warning_ind.tmgi_info_len; index++)
            {
              oos_warning_ind.tmgi_info[index].tmgi_len = TMGI_LENGTH_MAX_V01;
              memcpy(&(oos_warning_ind.tmgi_info[index].tmgi),
                     &(evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[index].tmgi),
                     TMGI_LENGTH_MAX_V01);
            }

            oos_warning_ind.reason = evt_info_ptr->payload.embms_tmgi_info.oos_warning;
            oos_warning_ind.dbg_trace_id =
              evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.dbg_trace_id;

            QCRIL_LOG_INFO( "EMBMS OOS warning ind [%d], debug_id [%d] num TMGI's [%d]",
                            oos_warning_ind.reason,
                            oos_warning_ind.dbg_trace_id,
                            oos_warning_ind.tmgi_info_len);

            qcril_hook_unsol_response(instance_id,
                                      QCRIL_EVT_HOOK_EMBMS_UNSOL_OSS_WARNING,
                                      (char*)&oos_warning_ind,
                                      sizeof(oos_warning_ind));
          }
          break;

        case QMI_WDS_EMBMS_TMGI_LIST_AVAILABLE:
          {
            embms_unsol_available_tmgi_ind_msg_v01 available_tmgi_list;
            int index = 0;

            memset(&available_tmgi_list, 0, sizeof(available_tmgi_list));
            available_tmgi_list.tmgi_info_len =
              evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list_len;

            /* validate tmgi_info_len */
            if(available_tmgi_list.tmgi_info_len > NUMBER_MAX_V01)
            {
              QCRIL_LOG_ERROR( "Oversize tmgi_info_len, Dropping TMGI list indication:%d",
                                 available_tmgi_list.tmgi_info_len );
              goto err_ret;
            }

            for (index = 0; index < (int)available_tmgi_list.tmgi_info_len; index++)
            {
              available_tmgi_list.tmgi_info[index].tmgi_len = TMGI_LENGTH_MAX_V01;
              memcpy(&(available_tmgi_list.tmgi_info[index].tmgi),
                     &(evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[index].tmgi),
                     TMGI_LENGTH_MAX_V01);
            }

            available_tmgi_list.dbg_trace_id =
              evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.dbg_trace_id;

            QCRIL_LOG_INFO( "EMBMS AVAILABLE list ind debug_id [%d], num TMGI's [%d]",
                            available_tmgi_list.dbg_trace_id, available_tmgi_list.tmgi_info_len);

            qcril_hook_unsol_response(instance_id,
                                      QCRIL_EVT_HOOK_EMBMS_UNSOL_AVAILABLE_TMGI,
                                      (char*)&available_tmgi_list,
                                      sizeof(available_tmgi_list));
          }
          break;

        default:
          break;
      }
    }
    break;

    case DSI_NET_TMGI_DEACTIVATED:
    {
      /* When this event arrives, we extract the first node from
      ** the pending request list, which has matching TMGI. Then
      ** based on what the pending request was, we process the event
      ** as a success or a failure response.
      */
      qcril_data_embms_tmgi_requests_type  *txn;

      QCRIL_LOG_INFO( ">>>DSI_NET_TMGI_DEACTIVATED: START>>> cid [%d], index [%d]",
                      info_tbl_ptr->cid, info_tbl_ptr->index );

      for (txn  = (qcril_data_embms_tmgi_requests_type *) list_peek_front (&info_tbl_ptr->embms_txns_list);
           txn != NULL;
           txn  = (qcril_data_embms_tmgi_requests_type *)
                    list_peek_next (&info_tbl_ptr->embms_txns_list, &(txn->link)))
      {
        if(NULL != evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list &&
           NULL == evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list)
        {
          QCRIL_LOG_DEBUG("%s", "Possible Deactivate Request");

          if ((memcmp(txn->deact_tmgi,
                      evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list[0].tmgi,
                      QCRIL_DATA_EMBMS_TMGI_LEN)) == 0)
          {
            QCRIL_LOG_DEBUG( "%s", "DEACTIVATED EVENT, TMGI match occured");
            if (txn->pend_req == QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI)
            {
              QCRIL_LOG_DEBUG( "%s", "DEACTIVATED EVENT, pending request match occured");

              /* prepare response deactivate_resp */
              memset ( &deactivate_response, 0, sizeof( deactivate_response ) );
              deactivate_response.call_id_valid   = TRUE;
              deactivate_response.call_id         = info_tbl_ptr->cid;
              deactivate_response.tmgi_info_valid = TRUE;
              deactivate_response.tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
              memcpy(&(deactivate_response.tmgi_info.tmgi[0]),
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list[0].tmgi,
                     TMGI_LENGTH_MAX_V01
                    );

              deactivate_response.dbg_trace_id =
                evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.dbg_trace_id;

              qcril_default_request_resp_params( instance_id,
                                                 txn->pend_tok,
                                                 QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI,
                                                 RIL_E_SUCCESS,/* revisit on error code */
                                                 &resp );

              list_pop_item( &info_tbl_ptr->embms_txns_list, &txn->link );
              free( txn );
              QCRIL_LOG_DEBUG( "%s", "deleted tmgi txn");

              resp.resp_pkt = &deactivate_response;
              resp.resp_len = sizeof( deactivate_response );
              qcril_send_request_response( &resp );
              goto ret;
            }/* if(txn->pend_req == QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI) */
          }/* if(memcmp(deactivate_tmgi_list)*/
        }/* deactivate_tmgi_list != NULL && tmgi_list == NULL */
        else if(NULL == evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list &&
                NULL != evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list)
        {
          QCRIL_LOG_DEBUG("%s", "Possible Activate Request");

          if((memcmp(txn->tmgi,
                     evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[0].tmgi,
                     QCRIL_DATA_EMBMS_TMGI_LEN)) == 0)
          {
            QCRIL_LOG_DEBUG("%s", "DEACTIVATED_EVENT, TMGI match occured");
            if(txn->pend_req == QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI)
            {
              QCRIL_LOG_DEBUG("%s", "DEACTIVATED EVENT, pending activate request match occured");
              QCRIL_LOG_DEBUG( "%s", "Request to Activate TMGI failed");
              memset (&activate_response, 0, sizeof( activate_response));
              activate_response.call_id_valid = TRUE;
              activate_response.call_id       = info_tbl_ptr->cid;
              activate_response.tmgi_info_valid = TRUE;
              activate_response.tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
              memcpy(&(activate_response.tmgi_info.tmgi[0]),
                       evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[0].tmgi,
                       TMGI_LENGTH_MAX_V01);
              activate_response.dbg_trace_id =
                evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.dbg_trace_id;

              if(SUCCESS != qcril_data_utils_embms_get_ril_status_code(
                (int)evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.activate_status,
                (int *)&activate_response.resp_code))
              {
                QCRIL_LOG_ERROR("%s", "Unable to get activate error status code ");
                activate_response.resp_code = QCRIL_DATA_EMBMS_ERROR_UNKNOWN;
              }
              qcril_default_request_resp_params( instance_id,
                                                 txn->pend_tok,
                                                 QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI,
                                                 RIL_E_GENERIC_FAILURE,/* revisit on error code */
                                                 &resp );
              list_pop_item( &info_tbl_ptr->embms_txns_list, &txn->link );
              free( txn );
              QCRIL_LOG_DEBUG( "%s", "deleted tmgi txn");

              resp.resp_pkt = &activate_response;
              resp.resp_len = sizeof( activate_response );

              qcril_send_request_response( &resp );
              goto ret;
            }/* if(txn->pend_req == QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI */
          }/* if(memcmp tmgi_list) */
        }/* deactivate_tmgi_list == NULL && tmgi_list != NULL */
      }/* for */

      QCRIL_LOG_ERROR("%s", "Processed DSI_NET_TMGI_DEACTIVATED, "
                      "TMGI not found, Dropping Indication");
      goto err_ret;
    }
    break;

    case DSI_NET_TMGI_ACTIVATED_DEACTIVATED:
    {
      qcril_data_embms_tmgi_requests_type *txn;
      embms_activate_deactivate_tmgi_resp_msg_v01 act_deact_resp;
      RIL_Errno err_no = RIL_E_SUCCESS;

      QCRIL_LOG_INFO( ">>>DSI_NET_TMGI_ACTIVATED_DEACTIVATED: START>>> cid [%d], index [%d]",
                      info_tbl_ptr->cid, info_tbl_ptr->index);

      txn = (qcril_data_embms_tmgi_requests_type *) list_peek_front (&info_tbl_ptr->embms_txns_list);

      while(txn != NULL)
      {
        if((memcmp(txn->tmgi,
                   evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[0].tmgi,
                   QCRIL_DATA_EMBMS_TMGI_LEN) == 0) &&
           (memcmp(txn->deact_tmgi,
                   evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list[0].tmgi,
                   QCRIL_DATA_EMBMS_TMGI_LEN) == 0)
           )
        {
          QCRIL_LOG_DEBUG("%s", "ACTIVATED_DEACTIVATED EVENT, TMGI match occured");
          if(txn->pend_req == QCRIL_EVT_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI)
          {
            QCRIL_LOG_DEBUG("%s", "ACTIVATED_DEACTIVATED EVENT, Pending request matched");

            /* prepare activate_deactivate response */
            memset(&act_deact_resp, 0, sizeof(act_deact_resp));
            act_deact_resp.act_tmgi_info_valid = TRUE;
            act_deact_resp.call_id = info_tbl_ptr->cid;

            act_deact_resp.act_tmgi_info_valid = TRUE;
            act_deact_resp.act_tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
            memcpy(&act_deact_resp.act_tmgi_info.tmgi[0],
                   evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[0].tmgi,
                   TMGI_LENGTH_MAX_V01);
            act_deact_resp.deact_tmgi_info_valid = TRUE;
            act_deact_resp.deact_tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
            memcpy(&act_deact_resp.deact_tmgi_info.tmgi[0],
                   evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list[0].tmgi,
                   TMGI_LENGTH_MAX_V01);

            act_deact_resp.dbg_trace_id =
              evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.dbg_trace_id;

            if((QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS ==
                evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.activate_status) &&
               (QMI_WDS_EMBMS_TMGI_DEACTIVATE_SUCCESS ==
                evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_status)
               )
            {
              err_no = RIL_E_SUCCESS;
            }
            else
            {
              if(SUCCESS != qcril_data_utils_embms_get_ril_status_code(
                evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.activate_status,
                (int *)&act_deact_resp.act_resp_code))
              {
                QCRIL_LOG_ERROR("%s", "Unable to get activate error status code ");
                act_deact_resp.act_resp_code = QCRIL_DATA_EMBMS_ERROR_UNKNOWN;
              }
              err_no = RIL_E_GENERIC_FAILURE;
            }

            qcril_default_request_resp_params(instance_id,
                                              txn->pend_tok,
                                              QCRIL_EVT_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI,
                                              err_no,
                                              &resp);
            list_pop_item( &info_tbl_ptr->embms_txns_list, &txn->link );
            free( txn );
            QCRIL_LOG_DEBUG( "%s", "deleted tmgi txn");

            resp.resp_pkt = &act_deact_resp;
            resp.resp_len = sizeof( act_deact_resp );
            qcril_send_request_response( &resp );

            goto ret;

          }/* pend_req == QCRIL_EVT_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI*/
        }/* memcpy */

        txn  = (qcril_data_embms_tmgi_requests_type *)
                    list_peek_next (&info_tbl_ptr->embms_txns_list, &(txn->link));
      }/* while */
      QCRIL_LOG_ERROR("%s", "Processed DSI_NET_TMGI_ACTIVATED_DEACTIVATED, "
                      "TMGI not found, Dropping Indication");
      goto err_ret;
    }
    break;

    case DSI_NET_TMGI_ACTIVATED:
    {
      qcril_data_embms_tmgi_requests_type  *txn;

      QCRIL_LOG_INFO( ">>>DSI_NET_TMGI_ACTIVATED: START>>> cid [%d], index [%d]",
                      info_tbl_ptr->cid, info_tbl_ptr->index );

      txn  = (qcril_data_embms_tmgi_requests_type *) list_peek_front (&info_tbl_ptr->embms_txns_list);

      while(txn != NULL)
      {
        if ((memcmp(txn->tmgi,
                   evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[0].tmgi,
                   QCRIL_DATA_EMBMS_TMGI_LEN)) == 0)
        {
          QCRIL_LOG_DEBUG( "%s", "ACTIVATED EVENT, TMGI match occured");
          if (txn->pend_req == QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI)
          {
            QCRIL_LOG_DEBUG( "%s", "ACTIVATED EVENT, Pending request matched");
            /* prepare activate response */
            memset ( &activate_response, 0, sizeof( activate_response ) );
            activate_response.call_id_valid   = TRUE;
            activate_response.call_id         = info_tbl_ptr->cid;
            activate_response.tmgi_info_valid = TRUE;
            activate_response.tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
            memcpy(&activate_response.tmgi_info.tmgi[0],
                   evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list[0].tmgi,
                   TMGI_LENGTH_MAX_V01);
            activate_response.dbg_trace_id =
              evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.dbg_trace_id;

            qcril_default_request_resp_params( instance_id,
                                               txn->pend_tok,
                                               QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI,
                                               RIL_E_SUCCESS,/* revisit on error code */
                                               &resp );

            list_pop_item( &info_tbl_ptr->embms_txns_list, &txn->link );
            free( txn );
            QCRIL_LOG_DEBUG( "%s", "deleted tmgi txn");

            resp.resp_pkt = &activate_response;
            resp.resp_len = sizeof( activate_response );
            qcril_send_request_response( &resp );

            goto ret;
          }
          else if (txn->pend_req == QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI)
          {

            QCRIL_LOG_ERROR( "%s", "Activated received when expecting a "
                                   "deactivated indication, Dropping Indication");
            break;
          }
        }
        txn  = (qcril_data_embms_tmgi_requests_type *)
                    list_peek_next (&info_tbl_ptr->embms_txns_list, &(txn->link));
      }
      QCRIL_LOG_ERROR("%s", "Processed DSI_NET_TMGI_ACTIVATED, "
                      "TMGI not found, Dropping Indication");
      goto err_ret;
    }
    break;

    case DSI_NET_CONTENT_DESC_CONTROL:
    {
      embms_unsol_content_desc_update_per_obj_ind_msg_v01 content_desc_ctrl_ind;
      unsigned int list_index;

      QCRIL_LOG_INFO(">>>DSI_NET_CONTENT_DESC_CONTROL: START>>> cid [%d],"
                     "index [%d]",
                     info_tbl_ptr->cid,
                     info_tbl_ptr->index);

      memset(&content_desc_ctrl_ind, 0, sizeof(content_desc_ctrl_ind));

      /* validate tmgi_info_len */
      if ( evt_info_ptr->payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_len > NUMBER_MAX_V01)
      {
        QCRIL_LOG_ERROR(" Oversize tmgi_len, Drop CONTENT_DESC_CONTROL indication:%d",
                        evt_info_ptr->payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_len);
        goto err_ret;
      }

      memcpy(&(content_desc_ctrl_ind.tmgi_info.tmgi),
             &(evt_info_ptr->payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr->tmgi),
             TMGI_LENGTH_MAX_V01);

      content_desc_ctrl_ind.tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
      content_desc_ctrl_ind.dbg_trace_id = evt_info_ptr->payload.embms_content_desc_info.dbg_trace_id;

      if (evt_info_ptr->payload.embms_content_desc_info.param_mask &
            QMI_WDS_EMBMS_CONTENT_DESC_CTRL_IND_CONTENT_CTRL_PARAM_MASK)
      {
        content_desc_ctrl_ind.per_object_content_ctrl_valid = TRUE;
        content_desc_ctrl_ind.per_object_content_ctrl =
            evt_info_ptr->payload.embms_content_desc_info.content_control;
      }
      if (evt_info_ptr->payload.embms_content_desc_info.param_mask &
            QMI_WDS_EMBMS_CONTENT_DESC_CTRL_IND_STATUS_CTRL_PARAM_MASK)
      {
        content_desc_ctrl_ind.per_object_status_ctrl_valid = TRUE;
        content_desc_ctrl_ind.per_object_status_ctrl =
            evt_info_ptr->payload.embms_content_desc_info.status_control;
      }

      QCRIL_LOG_INFO( "EMBMS content desc control indication, "
                      "status_control: [%d], valid: [%d]"
                      "content_control: [%d], valid: [%d]"
                      "dbg_trace_id: [%d]"
                      "tmgi_list: %d:[%X,%X,%X,%X,%X,%X]",
                      content_desc_ctrl_ind.per_object_status_ctrl,
                        content_desc_ctrl_ind.per_object_status_ctrl_valid,
                      content_desc_ctrl_ind.per_object_content_ctrl,
                        content_desc_ctrl_ind.per_object_content_ctrl_valid,
                      content_desc_ctrl_ind.dbg_trace_id,
                      content_desc_ctrl_ind.tmgi_info.tmgi_len,
                      content_desc_ctrl_ind.tmgi_info.tmgi[0],
                      content_desc_ctrl_ind.tmgi_info.tmgi[1],
                      content_desc_ctrl_ind.tmgi_info.tmgi[2],
                      content_desc_ctrl_ind.tmgi_info.tmgi[3],
                      content_desc_ctrl_ind.tmgi_info.tmgi[4],
                      content_desc_ctrl_ind.tmgi_info.tmgi[5]);

      qcril_hook_unsol_response(instance_id,
                                QCRIL_EVT_HOOK_EMBMS_UNSOL_CONTENT_DESC_CONTROL,
                                (char*)&content_desc_ctrl_ind,
                                sizeof(content_desc_ctrl_ind));
    }
    break;

    default:
      QCRIL_LOG_ERROR("Unknown embms event received [%d]", evt_info_ptr->evt);
      break;

  } /*switch(evt_info_ptr->evt)*/

ret:
  switch(evt_info_ptr->evt)
  {
    case DSI_NET_TMGI_ACTIVATED:
      {
        QCRIL_LOG_INFO( "%s", "<<<DSI_NET_TMGI_ACTIVATED: DONE<<<" );
      }
      break;
    case DSI_NET_TMGI_DEACTIVATED:
      {
        QCRIL_LOG_INFO( "%s", "<<<DSI_NET_TMGI_DEACTIVATED: DONE<<<" );
      }
      break;
    case DSI_NET_TMGI_ACTIVATED_DEACTIVATED:
      {
        QCRIL_LOG_INFO("%s", "<<<DSI_NET_TMGI_ACTIVATED_DEACTIVATED: DONE<<<");
      }
      break;
    case DSI_NET_SAI_LIST_CHANGED:
      {
        QCRIL_LOG_DEBUG("%s", "<<<RIL UNSOL RSP SENT<<<");
        QCRIL_LOG_INFO("%s", "<<<DSI_NET_SAI_LIST_CHANGED: DONE<<<");
      }
      break;
    case DSI_NET_TMGI_LIST_CHANGED:
      {
         QCRIL_LOG_DEBUG( "%s", "<<<RIL UNSOL RSP SENT<<<" );
         QCRIL_LOG_INFO( "%s", "<<<DSI_NET_TMGI_LIST_CHANGED: DONE<<<" );
      }
      break;
    case DSI_NET_CONTENT_DESC_CONTROL:
      {
         QCRIL_LOG_DEBUG( "%s", "<<<RIL UNSOL RSP SENT<<<" );
         QCRIL_LOG_INFO( "%s", "<<<DSI_NET_CONTENT_DESC_CONTROL: DONE<<<" );
      }
      break;
    default:
      break;
  }

  QCRIL_LOG_DEBUG("%s", "Print List and its entries");

  if (DSI_NET_TMGI_ACTIVATED == evt_info_ptr->evt ||
      DSI_NET_TMGI_DEACTIVATED == evt_info_ptr->evt ||
      DSI_NET_TMGI_ACTIVATED_DEACTIVATED == evt_info_ptr->evt)
  {
    for (tmgi_txn  = (qcril_data_embms_tmgi_requests_type *) list_peek_front (&info_tbl_ptr->embms_txns_list);
         tmgi_txn != NULL;
         tmgi_txn  = (qcril_data_embms_tmgi_requests_type *)
                      list_peek_next (&info_tbl_ptr->embms_txns_list, &(tmgi_txn->link)))
    {
      QCRIL_LOG_DEBUG(" request:%s, and token:%d, TMGI:%x%x%x%x%x%x DEACT_TMGI:%x%x%x%x%x%x into the transactions list",
                      qcril_log_lookup_event_name(tmgi_txn->pend_req),
                      qcril_log_get_token_id(tmgi_txn->pend_tok ),
                      tmgi_txn->tmgi[0],
                      tmgi_txn->tmgi[1],
                      tmgi_txn->tmgi[2],
                      tmgi_txn->tmgi[3],
                      tmgi_txn->tmgi[4],
                      tmgi_txn->tmgi[5],
                      tmgi_txn->deact_tmgi[0],
                      tmgi_txn->deact_tmgi[1],
                      tmgi_txn->deact_tmgi[2],
                      tmgi_txn->deact_tmgi[3],
                      tmgi_txn->deact_tmgi[4],
                      tmgi_txn->deact_tmgi[5]);
    }
  }

err_ret:
  /* release memory */
  if(NULL != evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list)
  {
     QCRIL_LOG_DEBUG("free memory for tmgi_list");
    free(evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list);
    evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list = NULL;
  }

  if(NULL != evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list)
  {
    QCRIL_LOG_DEBUG("free memory for deactivate_tmgi_list");
    free(evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list);
    evt_info_ptr->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list = NULL;
  }

  if (NULL != evt_info_ptr->payload.embms_sai_info.available_sai_list)
  {
    QCRIL_LOG_DEBUG("free memory for available_sai_list");
    free(evt_info_ptr->payload.embms_sai_info.available_sai_list);
    evt_info_ptr->payload.embms_sai_info.available_sai_list = NULL;
  }

  if (NULL != evt_info_ptr->payload.embms_sai_info.camped_sai_list)
  {
    QCRIL_LOG_DEBUG("free memory for camped_sai_list");
    free(evt_info_ptr->payload.embms_sai_info.camped_sai_list);
    evt_info_ptr->payload.embms_sai_info.camped_sai_list = NULL;
  }

  if ( NULL != evt_info_ptr->payload.embms_sai_info.num_sai_per_group)
  {
    QCRIL_LOG_DEBUG("free memory for num_sai_per_group");
    free(evt_info_ptr->payload.embms_sai_info.num_sai_per_group);
    evt_info_ptr->payload.embms_sai_info.num_sai_per_group = NULL;
  }

  if (NULL != evt_info_ptr->payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr)
  {
    QCRIL_LOG_DEBUG("free memory for content_desc_ctr_tmgi");
    free(evt_info_ptr->payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr);
    evt_info_ptr->payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr = NULL;
  }

  return;
}
#endif /*FEATURE_DATA_EMBMS*/


/*===========================================================================
  FUNCTION:  qcril_data_get_numeric_ril_technology
===========================================================================*/
/*!
  @brief
  Maps the given RIL technology string to a numeric value

  @param
  ril_tech - RIL technology string

  @return
  Numeric value corresponding to the ril_tech string
*/
/*=========================================================================*/
static qcril_data_ril_radio_tech_t
qcril_data_get_numeric_ril_technology
(
  const char *ril_tech
)
{
  qcril_data_ril_radio_tech_t  numeric_ril_tech = QCRIL_DATA_RIL_RADIO_TECH_UNKNOWN;

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
  long int ret;
  char *end = NULL;

  if (!ril_tech)
  {
    QCRIL_LOG_ERROR( "%s", "bad parameter" );
  }
  else
  {
    ret = strtol(ril_tech, &end, 10);

    /* If we had a successful conversion, update the return value */
    if (end && *end == '\0')
    {
      numeric_ril_tech = (qcril_data_ril_radio_tech_t) ret;
    }
  }
#else
  QCRIL_LOG_DEBUG("%s", "This function is not supported");
#endif

  QCRIL_LOG_DEBUG("ril_tech=%s, numeric_ril_tech=%d",
                  (ril_tech) ? ril_tech : "",
                  numeric_ril_tech);

  return numeric_ril_tech;
}


/*===========================================================================
  FUNCTION:  qcril_data_validate_call_technology

===========================================================================*/
/*!
  @brief
  This function validates that the Android Telephony requested technology and
  the technology on which the call was brought up by the modem match.

  @pre Before calling, info_tbl_mutex must not be locked by the calling thread

  @return
  SUCCESS - if the validation was successful
  FAILURE - otherwise
*/
/*=========================================================================*/
static int
qcril_data_validate_call_technology
(
  qcril_data_call_info_tbl_type *info_tbl_ptr
)
{
  int ret = SUCCESS;
  const char *str = NULL;

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
  dsi_data_bearer_tech_t  bearer_tech;
  qcril_data_ril_radio_tech_t  numeric_ril_tech;

  numeric_ril_tech = qcril_data_get_numeric_ril_technology(info_tbl_ptr->call_params->ril_tech);

  str = qcril_data_util_get_ril_tech_string(numeric_ril_tech);
  QCRIL_LOG_DEBUG("RIL requested technology=%s, numeric=%d",
                   str, numeric_ril_tech);

    memset(&bearer_tech, 0, sizeof(bearer_tech));

    bearer_tech = dsi_get_current_data_bearer_tech(info_tbl_ptr->dsi_hndl);

    QCRIL_LOG_DEBUG("Modem reported data bearer technology=%s, numeric=%d",
                    qcril_data_util_get_dsi_bearer_tech_string(bearer_tech),
                    bearer_tech);

  /* If RIL provided technology is 1x or EVDO, determine the modem technology
     for the brought up call */
  if (QCRIL_DATA_IS_RIL_RADIO_TECH_CDMA_1X_EVDO(numeric_ril_tech))
  {
    /* If the call came up on anything other than 1x/EVDO on the modem side
       return failure (FMC is an exception) */
    if (DSI_DATA_BEARER_TECH_FMC == bearer_tech)
    {
      QCRIL_LOG_DEBUG("%s", "FMC bearer tech detected, returning SUCCESS");
    }
    else if (!QCRIL_DATA_IS_DSI_DATA_BEARER_TECH_CDMA_1X_EVDO(bearer_tech))
    {
      QCRIL_LOG_DEBUG("%s", "Tech mismatch detected between RIL and modem reported techs");
      ret = FAILURE;
    }
    else
    {
      QCRIL_LOG_DEBUG("%s", "Tech matched between RIL and modem reported techs");
    }
  }
  else if(QCRIL_DATA_IS_RIL_RADIO_TECH_3GPP_EHRPD(numeric_ril_tech))
  {
    if(!QCRIL_DATA_IS_DSI_DATA_BEARER_TECH_3GPP_EHRPD(bearer_tech))
    {
      QCRIL_LOG_DEBUG("%s", "Tech mismatch detected between RIL and modem reported techs");
      ret = FAILURE;
    }
    else
    {
      QCRIL_LOG_DEBUG("%s", "Tech matched between RIL and modem reported techs");
    }
  }
  else
  {
    QCRIL_LOG_DEBUG("%s", "RIL requested tech is not CDMA or UMTS, returning SUCCESS");
  }
#else
  QCRIL_LOG_DEBUG("%s", "This function is not supported, returning SUCCESS");
#endif

  return ret;
}

/*===========================================================================

  FUNCTION:  qcril_data_dont_use_dsd()

===========================================================================*/
/*!
    @brief
    Check ADB property to override using DSD indications for
    data registration.

    @return
    TRUE: If ADB property persist.radio.dont_use_dsd is set to TRUE
    FALSE: If the property is set to FALSE/not set/does not exist.
*/
/*=========================================================================*/
boolean qcril_data_dont_use_dsd()
{
  char args[QCRIL_DATA_PROP_SIZE];

  memset(args, 0, sizeof(args));
  property_get(QCRIL_DATA_PROP_DONT_USE_DSD, args, "");
  if (0 == strcmp(args, "true"))
  {
    QCRIL_LOG_DEBUG("%s", "persist.radio.dont_use_dsd = true");
    return TRUE;
  }
  else
  {
    QCRIL_LOG_DEBUG("%s", "persist.radio.dont_use_dsd = false");
    return FALSE;
  }
}

/*===========================================================================
  FUNCTION:  qcril_data_reg_sys_ind

===========================================================================*/
/*!
  @brief
  Helper routine to register for system indications. We can either register
  for full set of indications or limited set of indications using this.

  @return QCRIL_DS_SUCCESS - if registration was successful.
  @return QCRIL_DS_ERROR - if registration failed for some reason.
*/
/*=========================================================================*/
static int qcril_data_reg_sys_ind
(
  qcril_data_limited_sys_ind_switch_type       sys_ind_switch
)
{
  qmi_wds_event_report_params_type event_report_params;
  int rc;
  int qmi_err_code;

  dsd_system_status_change_req_msg_v01  sys_reg_req_msg;
  dsd_system_status_change_resp_msg_v01 sys_reg_resp_msg;
  dsd_get_system_status_resp_msg_v01  sys_resp_msg;

  dsd_system_status_ind_msg_v01 *ind_data = NULL;

  QCRIL_LOG_INFO( "qcril_data_reg_sys_ind: ENTRY, switch %d", sys_ind_switch);

  if (TRUE == qcril_data_dont_use_dsd())
  {
    QCRIL_LOG_INFO("%s", "DSD not used due to persist.radio.dont_use_dsd");
    return QCRIL_DS_ERROR;
  }

  memset( &event_report_params, 0x0, sizeof(event_report_params) );
  if (LIMITED_SYS_INDICATIONS_ON == sys_ind_switch)
  {
    /* Enable limited indications */
    event_report_params.param_mask |= QMI_WDS_EVENT_DATA_SYS_STATUS_IND;
    event_report_params.report_data_sys_status = FALSE;

    event_report_params.param_mask |= QMI_WDS_EVENT_LIMITED_DATA_SYS_STATUS_IND;
    event_report_params.report_limited_data_sys_status = TRUE;
  }
  else
  {
    /* Enable full indications */
    event_report_params.param_mask |= QMI_WDS_EVENT_DATA_SYS_STATUS_IND;
    event_report_params.report_data_sys_status = TRUE;

    event_report_params.param_mask |= QMI_WDS_EVENT_LIMITED_DATA_SYS_STATUS_IND;
    event_report_params.report_limited_data_sys_status = FALSE;
  }

  rc = qmi_wds_set_event_report(global_qmi_wds_hndl,
                                &event_report_params,
                                &qmi_err_code);
  if (rc != QMI_NO_ERR)
  {
    QCRIL_LOG_ERROR("qmi_wds_set_event_report failed with err [%d][%d]",
                    rc, qmi_err_code);
    goto err_label;
  }

  memset(&sys_reg_req_msg, 0, sizeof(sys_reg_req_msg));
  memset(&sys_reg_resp_msg, 0, sizeof(sys_reg_resp_msg));


  sys_reg_req_msg.limit_so_mask_change_ind_valid = TRUE;

  /* Register to not report SO mask indications when
     Limited sys indications are ON   */
  if (LIMITED_SYS_INDICATIONS_ON == sys_ind_switch)
  {
     sys_reg_req_msg.limit_so_mask_change_ind = TRUE;
  }

  rc = qmi_client_send_msg_sync_with_shm(global_qmi_dsd_hndl,
                                QMI_DSD_SYSTEM_STATUS_CHANGE_REQ_V01,
                                &sys_reg_req_msg,
                                sizeof(sys_reg_req_msg),
                                &sys_reg_resp_msg,
                                sizeof(sys_reg_resp_msg),
                                QCRIL_DATA_QMI_DSD_SYNC_MSG_TIMEOUT);

  if (QMI_NO_ERR != rc)
  {
    QCRIL_LOG_ERROR("failed to send qmi_dsd_reg_system_status_ind, err=%d",rc);
    goto err_label;
  }
  else if (QMI_NO_ERR != sys_reg_resp_msg.resp.result)
  {
    QCRIL_LOG_ERROR("failed to send qmi_dsd_reg_system_status_ind, err=%d",
                      sys_reg_resp_msg.resp.error);
    goto err_label;
  }

  /* Query the current sys_status to propagate the change
     when Limited sys indications are off */

  if (LIMITED_SYS_INDICATIONS_OFF == sys_ind_switch)
  {
    memset(&sys_resp_msg, 0, sizeof(sys_resp_msg));

    rc = qmi_client_send_msg_sync_with_shm(global_qmi_dsd_hndl,
                                  QMI_DSD_GET_SYSTEM_STATUS_REQ_V01,
                                  NULL,
                                  0,
                                  &sys_resp_msg,
                                  sizeof(sys_resp_msg),
                                  QCRIL_DATA_QMI_DSD_SYNC_MSG_TIMEOUT);

    if (QMI_NO_ERR != rc)
    {
      QCRIL_LOG_ERROR("failed to send qmi_dsd_get_system_status, err=%d", rc);
    }
    else if (QMI_NO_ERR != sys_resp_msg.resp.result)
    {
      QCRIL_LOG_ERROR("failed to send qmi_dsd_get_system_status, err=%d",
                      sys_resp_msg.resp.error);
    }

    /* Process the system status response */
    ind_data = (dsd_system_status_ind_msg_v01 *)((char *)&sys_resp_msg +
                               offsetof(dsd_get_system_status_resp_msg_v01,
                                                         avail_sys_valid));

    qcril_data_process_qmi_dsd_ind(ind_data);

  }


  QCRIL_LOG_INFO( "%s","qcril_data_reg_sys_ind: EXIT with SUCCESS");
  return QCRIL_DS_SUCCESS;

err_label:
  QCRIL_LOG_ERROR( "%s","qcril_data_reg_sys_ind: EXIT with ERROR");
  return QCRIL_DS_ERROR;
}


/*===========================================================================
  FUNCTION:  qcril_data_reg_sys_ind

===========================================================================*/
/*!
  @brief
  Helper routine to check if all the established calls are dormant.
  Checks from the qcril_data local cache and does not end up
  sending query to modem.

  @return 1 - if all established calls are dormant.
  @return 0 - if any of the established calls is not dormant.
*/
/*=========================================================================*/
static int qcril_data_all_calls_dormant
(
  void
)
{
  int i;
  int dev_instance;
  int qmi_wds_hndl;
  int calls_dormant = 1;

  /* Get the current dormancy status for all calls. */
  for (i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++)
  {
    GET_DEV_INSTANCE_FROM_NAME(i, dev_instance);
    if (dev_instance < 0 ||
        dev_instance >= QCRIL_DATA_MAX_DEVS ||
        !VALIDATE_LOCAL_DATA_OBJ(&info_tbl[i]) ||
        info_tbl[i].cid == CALL_ID_INVALID)
    {
      continue;
    }

    qmi_wds_hndl = info_tbl[i].qmi_wds_hndl;
    if(QCRIL_DATA_HNDL_INVALID == qmi_wds_hndl)
    {
      continue;
    }

    QCRIL_LOG_INFO ("Dormancy status for inst: %d, wds_hndl %d, status: %d",
                    i, qmi_wds_hndl, info_tbl[i].dorm_status);

    if (info_tbl[i].dorm_status == QMI_WDS_DORM_STATUS_ACTIVE)
    {
      /* If any call is active, return FALSE */
      calls_dormant = 0;
    }

  } /* for */

  return calls_dormant;

} /* qcril_data_all_calls_dormant() */


/*===========================================================================
  FUNCTION:  qcril_data_event_hdlr

===========================================================================*/
/*!
    @brief
    Registered with QCRIL to be called by QCRIL on event
    QCRIL_EVT_DATA_EVENT_CALLBACK

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_event_hdlr(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr  /*output*/
)
{
  qcril_instance_id_e_type       instance_id;
  qcril_reqlist_public_type      reqlist_info;
  qcril_data_call_info_tbl_type *info_tbl_ptr;
  qcril_data_call_response_type  response;
  void                          *response_ptr = (void*)&response;
  qmi_wds_event_report_params_type  event_rep;
  int                            result = FAILURE;
  qcril_data_event_data_t       *evt_info_ptr;
  char *dummy_rsp[3] = { "0", "rmnet0", "0.0.0.0" };
  unsigned int                   pend_req  = DS_RIL_REQ_INVALID;
  int                            request_id = DS_RIL_REQ_INVALID;
  errno_enum_type                qcril_ret = E_FAILURE;
  dsi_ce_reason_t                tmp_end_reason;
  char                           buf[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1];
  int                            qmi_err_code;
  boolean                        dorm_status_changed = FALSE;
  int                            call_state;
  size_t                         response_len = 0;
  dsi_ip_family_t                ipf;
  int                            call_status;
  qmi_wds_bind_mux_data_port_params_type bind_params;
  char                           dev_name[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1];

#ifdef FEATURE_DATA_EMBMS
  qcril_request_resp_params_type               resp;
  qcril_embms_enable_response_payload_type     enable_response;
  qcril_embms_disable_indication_payload_type  disable_indication;
  qcril_modem_id_e_type                        modem_id;
  int                                          if_index;

  RIL_Errno reti = RIL_E_GENERIC_FAILURE;
#endif

  QCRIL_LOG_DEBUG( "%s", "qcril_data_event_hdlr: ENTRY" );

  /* Input Validation */
  QCRIL_DS_ASSERT( params_ptr != NULL, "validate input params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate input ret_ptr" );
  if ( ( params_ptr == NULL ) || ( ret_ptr == NULL ) )
  {
    goto err_label_exit;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    goto err_label_exit;
  }
#ifdef FEATURE_DATA_EMBMS
  modem_id     = params_ptr->modem_id;
#endif

  memset( &reqlist_info, 0, sizeof( qcril_reqlist_public_type ) );
  memset( &response,     0, sizeof( qcril_data_call_response_type ) );

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  evt_info_ptr = ( qcril_data_event_data_t * )params_ptr->data;
  QCRIL_DS_LOG_DBG_MEM( "event obj", evt_info_ptr );
  if ( !VALIDATE_LOCAL_DATA_OBJ( evt_info_ptr ) )
  {
    QCRIL_LOG_ERROR( "%s", "bad event obj, cannot free mem, ret with err" );
    goto err_label_no_free;
  }

  /* data call status is event not associated with any info_tbl entry */
  if ((int) QCRIL_DATA_EV_DATA_CALL_STATUS_CHANGED ==
      (int) evt_info_ptr->evt)
  {
    QCRIL_LOG_INFO("%s","QCRIL_DATA_EV_DATA_CALL_STATUS_CHANGED arrived "
                   "in the qcril_event thread");
    qcril_data_unsol_tethered_mode_state_changed( global_instance_id,
                                                  &global_tethered_state,
                                                  sizeof(RIL_tethered_mode_state));
    goto ret;
  }

  /* Pointer to info Tbl is derived from event posted to QCRIL */
  info_tbl_ptr =  ( qcril_data_call_info_tbl_type *)evt_info_ptr->data;
  QCRIL_DS_LOG_DBG_MEM( "info_tbl", info_tbl_ptr );
  if ( !VALIDATE_LOCAL_DATA_OBJ( info_tbl_ptr ) )
  {
    QCRIL_LOG_ERROR( "%s", "invalid info_tbl ref" );
    QCRIL_DS_ASSERT( evt_info_ptr != NULL, "validate event obj" );
    QCRIL_LOG_INFO( "%s", "try event obj dealloc" );
    if ( evt_info_ptr != NULL ) free( evt_info_ptr );
    QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
    return;
  }

#ifdef FEATURE_DATA_EMBMS
  if (evt_info_ptr->evt == DSI_NET_TMGI_ACTIVATED ||
        evt_info_ptr->evt == DSI_NET_TMGI_DEACTIVATED ||
      evt_info_ptr->evt == DSI_NET_TMGI_ACTIVATED_DEACTIVATED ||
        evt_info_ptr->evt == DSI_NET_TMGI_LIST_CHANGED ||
      evt_info_ptr->evt == DSI_NET_CONTENT_DESC_CONTROL ||
        evt_info_ptr->evt == DSI_NET_SAI_LIST_CHANGED)
  {
    qcril_data_handle_embms_events(instance_id, info_tbl_ptr, evt_info_ptr);
    goto ret;
  }
#endif /*FEATURE_DATA_EMBMS*/

  /* Check whether REQ is pending */
#ifdef FEATURE_DATA_EMBMS
  if( qcril_data_util_is_req_pending( info_tbl_ptr, &pend_req ) &&
      (QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_REQ != pend_req) &&
      (QCRIL_EVT_HOOK_EMBMS_UNSOL_ACTIVE_TMGI != pend_req))
#else
  if( qcril_data_util_is_req_pending( info_tbl_ptr, &pend_req ))
#endif
  {
    QCRIL_LOG_INFO( "RIL REQ pend [%s], cid [%d], index [%d]",
                    qcril_log_lookup_event_name( pend_req ),
                    info_tbl_ptr->cid,
                    info_tbl_ptr->index );
    QCRIL_LOG_INFO( "DEBUG:  params->t[0x%x] info_tbl_ptr->pend_tok[0x%x]",
                     params_ptr->t, info_tbl_ptr->pend_tok );
    QCRIL_DS_ASSERT( params_ptr->t != NULL, "validate pend RIL Token" );
    QCRIL_DS_ASSERT( params_ptr->t == info_tbl_ptr->pend_tok,
                     "validate pend RIL Token consistency" );

    if ( ( qcril_ret = qcril_reqlist_query( instance_id, params_ptr->t,
                                            &reqlist_info ) ) != E_SUCCESS )
    {
      QCRIL_LOG_ERROR( "unable to find reqlist entry, RIL Token [%d]",
                       qcril_log_get_token_id( params_ptr->t ) );
      goto err_label;
    }

    QCRIL_LOG_DEBUG( "Req list entry found for RIL Token [%d]",
                     qcril_log_get_token_id( params_ptr->t ) );

    /* Validate xtracted local info tbl pointer for a pending RIL Token from reqlist Node */
    if ( qcril_ret == E_SUCCESS )
    {
      QCRIL_DS_ASSERT( ( info_tbl_ptr ==
                     ( qcril_data_call_info_tbl_type * )reqlist_info.sub.ds.info ),
                 "validate info_tbl ref" );
    }
    QCRIL_DS_LOG_DBG_MEM( "info_tbl", info_tbl_ptr );

  }
  else /* RIL REQ not pending */
  {
    QCRIL_LOG_INFO( "%s", "RIL REQ NOT pend" );
    QCRIL_DS_ASSERT( params_ptr->t == NULL, "validate null RIL Token" );

    if (NULL != params_ptr->t)
    {
      QCRIL_LOG_INFO( "params_ptr->t=0x%x", params_ptr->t);
    }
  }

  /* Dispatch the specific call event */
  switch( evt_info_ptr->evt )
  {
  case DSI_EVT_NET_IS_CONN:

    QCRIL_LOG_INFO( ">>>DSI_EVT_NET_IS_CONN: START>>> cid [%d], index [%d]",
                    info_tbl_ptr->cid, info_tbl_ptr->index );
    /*
      Check pending RIL SETUP REQ
      NET_IS_CONN event is not expected if a SETUP REQ is not pending
    */
    if ( pend_req == RIL_REQUEST_SETUP_DATA_CALL )
    {
      memset(buf, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);
      /* Now that call is up now, find out what port the call
       * came up on. Also init and store the related wds client
       * handle locally in the info_tbl entry */
      QCRIL_LOG_DEBUG("Get device name and fill the value at [%p]",
                      buf);

      /* We got the data call setup response, so stop the response timer */
      qcril_data_util_stop_timer(&info_tbl_ptr->timer_id);

      /* Validate the technology on which the call was brought up. This is
         necessary to support 1x/EVDO -> eHRPD -> LTE IRAT scenario.
         For example, when a 1x/EVDO call is requested by Android Telephony and
         the technology on the modem side changes to eHRPD in the interim,
         the call would still be brought up (on eHRPD) because a default profile
         was used for the 1x/EVDO call. This causes problems during IRAT to LTE
         because the profile parameters weren't completely specified initially.
         So, the call gets torn down. To avoid this situation, tear down the call
         when a 1x/EVDO call is specified by Android Telephony and the modem
         brings up the call on a technology other than 1x/EVDO. */
      if (SUCCESS != qcril_data_validate_call_technology(info_tbl_ptr))
      {
        /* Stop the data call */
        QCRIL_LOG_DEBUG ("%s", "unexpected technology... stopping data call");
        if( E_SUCCESS != qcril_data_stop_data_call(info_tbl_ptr) )
        {
          qcril_data_response_generic_failure( info_tbl_ptr->instance_id,
                                               info_tbl_ptr->pend_tok,
                                               RIL_REQUEST_SETUP_DATA_CALL );

          /* Cleanup call state */
          qcril_data_cleanup_call_state( info_tbl_ptr );
        }

        /* Response to client will be generated in event handler */
        goto ret;
      }

      if (DSI_SUCCESS != dsi_get_qmi_port_name(info_tbl_ptr->dsi_hndl,
                                               buf,
                                               DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1))
      {
        QCRIL_LOG_ERROR("couldn't get device name for info_tbl [%d]",
                        info_tbl_ptr->index);
        QCRIL_DS_ASSERT( 0, "programming error: dsi_get_qmi_port_name" );
        qcril_data_cleanup_call_state( info_tbl_ptr );
        goto err_label;
      }
      else
      {
        buf[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN] = '\0';
        QCRIL_LOG_DEBUG("device [%s] is associated with info tbl indx [%d]",
                        buf, info_tbl_ptr->index);
      }

      /* Release stale QMI WDS handle, if any */
      if (QCRIL_DATA_HNDL_INVALID != info_tbl_ptr->qmi_wds_hndl)
      {
        int rc;

        QCRIL_LOG_DEBUG("releasing stale QMI WDS handle [0x%x] associated with info tbl indx [%d]",
                        info_tbl_ptr->qmi_wds_hndl,
                        info_tbl_ptr->index);

        if (QMI_NO_ERR != (rc = qmi_wds_srvc_release_client(info_tbl_ptr->qmi_wds_hndl,
                                                            &qmi_err_code)))
        {
          QCRIL_LOG_ERROR("failed to release QMI WDS handle [0x%x], rc [%d], qmi_err [%d]",
                          info_tbl_ptr->qmi_wds_hndl,
                          rc,
                          qmi_err_code);
        }

        info_tbl_ptr->qmi_wds_hndl = QCRIL_DATA_HNDL_INVALID;
      }

      info_tbl_ptr->qmi_wds_hndl = qmi_wds_srvc_init_client(buf,
                                                            qcril_data_qmi_wds_ind_cb,
                                                            info_tbl_ptr,
                                                            &qmi_err_code);
      if (info_tbl_ptr->qmi_wds_hndl < 0)
      {
        QCRIL_LOG_ERROR("couldn't init wds srvc client for info_tbl [%d]",
                        info_tbl_ptr->index);
        info_tbl_ptr->qmi_wds_hndl = QCRIL_DATA_HNDL_INVALID;
        qcril_data_cleanup_call_state( info_tbl_ptr );
        goto err_label;
      }
      else
      {
        int rval;
        int qmi_err;

        QCRIL_LOG_DEBUG("obtained wds srvc clnt id [0x%x] for info tbl indx [%d]",
                        info_tbl_ptr->qmi_wds_hndl, info_tbl_ptr->index);

        memset(dev_name, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);
        memset(&bind_params, 0, sizeof(qmi_wds_bind_mux_data_port_params_type));

        if (DSI_SUCCESS != dsi_get_device_name(info_tbl_ptr->dsi_hndl,
                                               dev_name,
                                               DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1))
        {
          QCRIL_LOG_ERROR("couldn't get device name for info_tbl [%d]",
                          info_tbl_ptr->index);
        }
        bind_params.params_mask |= QMI_WDS_BIND_MUX_DATA_PORT_PARAMS_EP_ID;
        bind_params.params_mask |= QMI_WDS_BIND_MUX_DATA_PORT_PARAMS_MUX_ID;
        QCRIL_LOG_DEBUG("DSI_EVT_NET_IS_CONN: devname [%s]", dev_name);
        QMI_PLATFORM_DEV_NAME_TO_CONN_ID_EX(dev_name,
                                            &bind_params.ep_id.ep_type,
                                            &bind_params.ep_id.iface_id,
                                            &bind_params.mux_id);

        if (QMI_NO_ERR != qmi_wds_bind_mux_data_port(info_tbl_ptr->qmi_wds_hndl,
                                                     &bind_params,
                                                     &qmi_err_code))
        {
          QCRIL_LOG_ERROR("qdi_wds_srvc_init_clients: qmi_wds_bind_mux_data_port failed "
                         "for qmi_cid_v4 %d with error %d,\n",
                          info_tbl_ptr->qmi_wds_hndl, qmi_err_code);
        }

        if(DSI_IP_VERSION_6 == info_tbl_ptr->dsi_ip_version)
        {
          /* bind wds handle to IPV6 family */
          rval = qmi_wds_set_client_ip_pref(info_tbl_ptr->qmi_wds_hndl,
                                            QMI_WDS_IP_FAMILY_PREF_IPV6,
                                            &qmi_err);

          if (QMI_NO_ERR == rval)
          {
            QCRIL_LOG_INFO("%s", "Successfully bound IPv6 QMI WDS client to IPv6 family");
          }
          else if (QMI_SERVICE_ERR == rval && QMI_SERVICE_ERR_NO_EFFECT == qmi_err)
          {
            QCRIL_LOG_INFO("%s", "IPv6 QMI WDS client already bound to IPv6 family");
          }
          else
          {
            QCRIL_LOG_ERROR(" binding v6 handle to IPV6 family failed rval=%d, qmi_err=%d",
                            rval,
                            qmi_err);
          }
        }/*(DSI_IP_VERSION_6 == ip_family)*/
      }

      if (QMI_NO_ERR != qmi_wds_bind_subscription ( info_tbl_ptr->qmi_wds_hndl,
                                               global_subs_id,
                                              &qmi_err_code))
      {
        QCRIL_LOG_ERROR("qmi_wds_bind_subscription: subs_id binding failed");
      }

      if (qcril_data_mtu > 0)
      {
        QCRIL_LOG_DEBUG("Changing MTU value of new call to [%d] as specified by RIL",
                        qcril_data_mtu);
        qcril_data_iface_set_mtu(info_tbl->dsi_hndl, qcril_data_mtu);
      }
      /* Get and fill all call attributes */
      qcril_data_util_fill_call_params( info_tbl_ptr );

      /* Update state of local tbl */
      info_tbl_ptr->pend_tok = NULL;
      info_tbl_ptr->pend_req = DS_RIL_REQ_INVALID;  /* is there a RIL_REQUEST_INVALID */

      QCRIL_DATA_MUTEX_LOCK(&dsi_cb_tbl_mutex);
      dsi_cb_tbl[info_tbl_ptr->index].pend_tok = NULL;
      QCRIL_DATA_MUTEX_UNLOCK(&dsi_cb_tbl_mutex);

      /* Update call_tbl to point to latest call info */
      qcril_data_util_update_call_state( info_tbl_ptr, CALL_ACTIVE_PHYSLINK_UP, PDP_FAIL_NONE );

      /* Create RIL REQ SETUP response */
      qcril_data_util_create_setup_rsp( info_tbl_ptr, &response );

      /* Post RIL Response */
      QCRIL_LOG_DEBUG( ">>>RIL RSP READY>>> cid [%d], ifname [%s], ip_addr [%s]",
                       response.setup_rsp.cid, response.setup_rsp.ifname, response.setup_rsp.addresses );

      /* Notify internal QCRIL clients of call connect */
      (void)qcril_data_client_notify(QCRIL_DATA_EVT_CALL_CONNECTED, NULL);

      qcril_data_response_success(instance_id,
                                  params_ptr->t,
                                  RIL_REQUEST_SETUP_DATA_CALL,
                                  (void *) ( &(response.setup_rsp)),
                                  sizeof( response.setup_rsp));
      QCRIL_LOG_DEBUG("%s", "<<<RIL RSP SENT<<<");

      QCRIL_LOG_INFO("%s", "<<<DSI_EVT_NET_IS_CONN: DONE<<<");

      /* Register for extended IP config indication */
      qcril_data_util_register_ext_ip_config_ind(info_tbl_ptr);

      /* we only request dormancy indication report on/off when
       there is previous request to do so. Previous request is
       assciated with screen state from Telephony.
       We don't want to request it to be ON if screen is OFF;
       We need to request it to be OFF per previous request */
      if(DORMANCY_INDICATIONS_ON == global_dorm_ind)
      {
        /* register for physlink up/down events */
        /*Register for Physlink UP DOWN Indication Events */
        QCRIL_LOG_DEBUG("%s", "Registering for Physlink Events");
        if( FAILURE == qcril_data_iface_ioctl(info_tbl_ptr,
                       QCRIL_DATA_IFACE_IOCTL_DORMANCY_INDICATIONS_ON,
                       &dorm_status_changed,
                       &call_state))
        {
          QCRIL_LOG_ERROR("%s", "Error registering for Physlink Events");
        }
      }
      else if(DORMANCY_INDICATIONS_OFF == global_dorm_ind)
      {
        QCRIL_LOG_DEBUG("%s", "De-Registering for Physlink Events");
        if(FAILURE == qcril_data_iface_ioctl(info_tbl_ptr,
                      QCRIL_DATA_IFACE_IOCTL_DORMANCY_INDICATIONS_OFF,
                      &dorm_status_changed,
                      &call_state))
        {
          QCRIL_LOG_ERROR("%s", "Error De-registering for Physlink Events");
        }
      }
      if(dorm_status_changed)
      {
        QCRIL_LOG_INFO("<<<[%d] processing started<<<", call_state);
        qcril_data_util_update_call_state(info_tbl_ptr, call_state, PDP_FAIL_NONE);
      }

      /* If we received fewer IP addresses for a Dual-IP call, initiate partial retry */
      if (DSI_IP_VERSION_4_6 == info_tbl_ptr->dsi_ip_version &&
          info_tbl_ptr->last_addr_count < DSI_NUM_IP_FAMILIES)
      {
        qcril_data_util_schedule_partial_retry_attempt(info_tbl_ptr, TRUE, TRUE);
      }
    }
#ifdef FEATURE_DATA_EMBMS
    else if ( pend_req == QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_REQ )
    {
      do
      {
        /* Update state of local tbl */
        info_tbl_ptr->pend_tok   = NULL;
        info_tbl_ptr->pend_req   = DS_RIL_REQ_INVALID;

        memset ( &enable_response, 0, sizeof( enable_response ) );
        enable_response.call_id = info_tbl_ptr->cid;

        memset(buf, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);
        QCRIL_LOG_DEBUG("Get device name and fill the value at [%p]", buf);

        if (DSI_SUCCESS != dsi_get_device_name(info_tbl_ptr->dsi_hndl,
                                               buf,
                                               DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1))
        {
          QCRIL_LOG_ERROR("couldn't get device name for info_tbl [%d]",
                          info_tbl_ptr->index);
          QCRIL_DS_ASSERT( 0, "programming error: dsi_get_device_name" );
          qcril_data_cleanup_call_state( info_tbl_ptr );
          enable_response.cause = RIL_E_GENERIC_FAILURE;
          enable_response.resp_code = QCRIL_DATA_EMBMS_ERROR_UNKNOWN;

          break;
        }
        else
        {
          buf[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN] = '\0';
          QCRIL_LOG_DEBUG("device [%s] is associated with info tbl indx [%d]",
                          buf, info_tbl_ptr->index);
          if(SUCCESS != qcril_data_get_ifindex(buf, &if_index))
          {
            QCRIL_LOG_ERROR("couldn't get if_index for info_tbl [%d]",
                            info_tbl_ptr->index);
            QCRIL_DS_ASSERT(0, "programming error: qcril_data_get_ifindex");
            qcril_data_cleanup_call_state( info_tbl_ptr);
            enable_response.cause = RIL_E_GENERIC_FAILURE;
            enable_response.resp_code = QCRIL_DATA_EMBMS_ERROR_UNKNOWN;

            break;
          }
          else
          {
            enable_response.cause = RIL_E_SUCCESS;
            enable_response.resp_code = QCRIL_DATA_EMBMS_ERROR_NONE;
            strlcpy(&enable_response.interafce_name[0],
                    &buf[0],
                    QCRIL_EMBMS_INTERFACE_NAME_LENGTH_MAX);
            enable_response.if_index = if_index;

            /* Update dev_name in info_tbl_ptr */
            memset( info_tbl_ptr->call_info.dev_name, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1 );
            memcpy( info_tbl_ptr->call_info.dev_name, buf, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN );
          }
        }
        reti = RIL_E_SUCCESS;
      }while(0);

      qcril_event_queue( instance_id,
                         modem_id,
                         QCRIL_DATA_ON_STACK,
                         QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_CON,
                         (void *) &enable_response,
                         sizeof( enable_response ),
                         (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );

      if(RIL_E_SUCCESS != reti)
      {
        goto err_label;
      }
      QCRIL_LOG_DEBUG( "%s", "<<<RIL RSP SENT<<<" );

      QCRIL_LOG_INFO( "%s", "<<<DSI_EVT_NET_IS_CONN: DONE<<<" );
    }/* else if ( pend_req == QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_REQ ) */
#endif
    else
    {
      QCRIL_LOG_DEBUG( "RIL token [%d], pend req [%d]",
                       qcril_log_get_token_id( info_tbl_ptr->pend_tok ), info_tbl_ptr->pend_req );
      goto err_label_exit;
    }
    break; /* DSI_EVT_NET_IS_CONN */

  case DSI_EVT_NET_PARTIAL_CONN:
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))

    QCRIL_LOG_INFO( ">>>DSI_EVT_NET_PARTIAL_CONN: START>>> cid [%d], index [%d]",
                    info_tbl_ptr->cid, info_tbl_ptr->index );

    /* We got the partial retry response, so stop the response timer */
    qcril_data_util_stop_timer(&info_tbl_ptr->timer_id);

    /* A DSI_EVT_NET_PARTIAL_NET event is received for a Dual-IP partial retry
       failure. */
    info_tbl_ptr->is_partial_retry = FALSE;

    QCRIL_LOG_DEBUG( "%s", "Dual-IP Partial Retry: Failure" );

    qcril_data_util_schedule_partial_retry_attempt(info_tbl_ptr, TRUE, TRUE);

    QCRIL_LOG_INFO( "%s", "<<<DSI_EVT_NET_PARTIAL_CONN: processing complete<<<" );
    goto ret;
    break; /* DSI_EVT_NET_PARTIAL_CONN */

#endif /* (RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6) */

  case DSI_EVT_NET_NO_NET:
    QCRIL_LOG_INFO( ">>>DSI_EVT_NET_NO_NET: START>>> cid [%d], index [%d]",
                    info_tbl_ptr->cid, info_tbl_ptr->index );

    /* Switch on PEND RIL REQ */
    switch( pend_req )
    {
#ifdef FEATURE_DATA_EMBMS
      case QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_REQ:
        {
          /* Update state of local tbl */
          info_tbl_ptr->pend_tok   = NULL;
          info_tbl_ptr->pend_req   = DS_RIL_REQ_INVALID;

          memset ( &enable_response, 0, sizeof( enable_response ) );
          enable_response.call_id  = info_tbl_ptr->cid;
          enable_response.cause    = RIL_E_GENERIC_FAILURE;
          enable_response.resp_code = QCRIL_DATA_EMBMS_ERROR_UNKNOWN;

          qcril_event_queue( instance_id,
                             modem_id,
                             QCRIL_DATA_ON_STACK,
                             QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_CON,
                             (void *) &enable_response,
                             sizeof( enable_response ),
                             (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
          goto ret;
        }
        break;
#endif

      case RIL_REQUEST_SETUP_DATA_CALL:
      case RIL_REQUEST_DEACTIVATE_DATA_CALL:
        /* Do nothing */
        break;

      default:
        QCRIL_LOG_DEBUG( "RIL REQ NOT pending, Token ID [%d]",
                         qcril_log_get_token_id( params_ptr->t ) );
        QCRIL_DS_ASSERT( info_tbl_ptr->pend_req == DS_RIL_REQ_INVALID,
                         "validate pend_req" );
        QCRIL_DS_ASSERT( info_tbl_ptr->pend_tok == NULL, "validate pend_tok" );
    }

    ipf = (DSI_IP_VERSION_6 == info_tbl_ptr->dsi_ip_version) ?
          DSI_IP_FAMILY_V6 : DSI_IP_FAMILY_V4;

    /* Get call fail reason or use default reason */
    if ( dsi_get_call_end_reason(info_tbl_ptr->dsi_hndl,
                                 &tmp_end_reason,
                                 ipf ) == DSI_SUCCESS )
    {
      QCRIL_LOG_DEBUG("dsi_get_call_end_reason returned [%d]",
                      tmp_end_reason.reason_code);
      if (SUCCESS != qcril_data_get_ril_ce_code(&tmp_end_reason,
                                                &last_call_end_reason))
      {
        QCRIL_LOG_DEBUG("%s","**programming err**");
        last_call_end_reason = PDP_FAIL_ERROR_UNSPECIFIED;
      }
      QCRIL_LOG_INFO( "set call end reason [%d]", last_call_end_reason );
    }
    else
    {
      last_call_end_reason = PDP_FAIL_ERROR_UNSPECIFIED;
      QCRIL_LOG_ERROR( "get call end reason FAILED, got [%u], use def",
                       last_call_end_reason );
    }

    info_tbl_ptr->call_info.inactiveReason = last_call_end_reason;

    /* Notify internal QCRIL clients of call connect */
    (void)qcril_data_client_notify( QCRIL_DATA_EVT_CALL_RELEASED, NULL );

    /* Post RIL RSP  */
    if ( pend_req == RIL_REQUEST_SETUP_DATA_CALL )
    {
      /* Failure response, indicated in RIL_REQUEST_SETUP_DATA_CALL.status */
      qcril_data_response_setup_data_call_failure( info_tbl_ptr,
                                                   instance_id,
                                                   params_ptr->t,
                                                   last_call_end_reason );
      qcril_data_cleanup_call_state( info_tbl_ptr );
    }
    else if ( pend_req == RIL_REQUEST_DEACTIVATE_DATA_CALL )
    {
      /* Successful response */
      qcril_data_response_success( instance_id,
                                   params_ptr->t,
                                   pend_req,
                                   NULL,
                                   0 );
      /* NO_NET event from LL, send ind to RIL & cleanup call state */
      qcril_data_util_update_call_state( info_tbl_ptr, CALL_INACTIVE, PDP_FAIL_NONE );
      qcril_data_unsol_call_list_changed( instance_id );
      qcril_data_cleanup_call_state( info_tbl_ptr );
    }
    else
    {
#ifdef FEATURE_DATA_EMBMS
      dsi_call_tech_type call_tech;

      if ((dsi_get_call_tech(info_tbl_ptr->dsi_hndl,
                             &call_tech) == DSI_SUCCESS) &&
            (call_tech == DSI_EXT_TECH_EMBMS))
      {
        /*NO_NET indication received for EMBMS call, relay event to NAS*/
        memset(&disable_indication, 0, sizeof(disable_indication));

        /*Populate indication response and send event to NAS*/
        disable_indication.call_id = info_tbl_ptr->cid;
        disable_indication.cause = last_call_end_reason;
        disable_indication.resp_code = QCRIL_DATA_EMBMS_ERROR_NONE;

        qcril_event_queue( instance_id,
                           modem_id,
                           QCRIL_DATA_ON_STACK,
                           QCRIL_EVT_QMI_RIL_EMBMS_DISABLE_DATA_IND,
                           (void *) &disable_indication,
                           sizeof( disable_indication ),
                           (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
      }
      else
      {
        /* NO_NET event from LL, send ind to RIL & cleanup call state */
        qcril_data_util_update_call_state( info_tbl_ptr, CALL_INACTIVE, last_call_end_reason );
        qcril_data_unsol_call_list_changed( instance_id );
      }
#else
      /* NO_NET event from LL, send ind to RIL & cleanup call state */
      qcril_data_util_update_call_state( info_tbl_ptr, CALL_INACTIVE, last_call_end_reason );
      qcril_data_unsol_call_list_changed( instance_id );
#endif
      qcril_data_cleanup_call_state( info_tbl_ptr );
    }

    QCRIL_LOG_INFO( "%s", "<<<DSI_EVT_NET_NO_NET: processing complete<<<" );
    goto ret;
    break; /*DSI_EVT_NET_NO_NET*/

  case DSI_EVT_PHYSLINK_DOWN_STATE:
    QCRIL_LOG_INFO( "%s", "<<<DSI_EVT_PHYSLINK_DOWN_STATE: processing Started<<<" );

    /* Update call_tbl to point to latest call info */
    qcril_data_util_update_call_state( info_tbl_ptr, CALL_ACTIVE_PHYSLINK_DOWN, PDP_FAIL_NONE );

    /* Notify internal QCRIL clients of call state change */
    (void)qcril_data_client_notify(QCRIL_DATA_EVT_CALL_PHYSLINK_DOWN, NULL);

    goto unsol_rsp;

  case DSI_EVT_PHYSLINK_UP_STATE:
    QCRIL_LOG_INFO( "%s",  "<<<DSI_EVT_PHYSLINK_UP_STATE: processing Started<<<" );

    /* Update call_tbl to point to latest call info */
    qcril_data_util_update_call_state( info_tbl_ptr, CALL_ACTIVE_PHYSLINK_UP, PDP_FAIL_NONE );

    /* Notify internal QCRIL clients of call state change */
    (void)qcril_data_client_notify(QCRIL_DATA_EVT_CALL_PHYSLINK_UP, NULL);

    goto unsol_rsp;

  case DSI_EVT_NET_NEWADDR:
  case DSI_EVT_NET_RECONFIGURED:
    if (DSI_EVT_NET_NEWADDR == evt_info_ptr->evt)
    {
      QCRIL_LOG_INFO( ">>>DSI_EVT_NET_NEWADDR: START>>> cid [%d], index [%d]",
                      info_tbl_ptr->cid, info_tbl_ptr->index );
    }
    else if (DSI_EVT_NET_RECONFIGURED == evt_info_ptr->evt)
    {
      QCRIL_LOG_INFO( ">>>DSI_EVT_NET_RECONFIGURED: START>>> cid [%d], index [%d]",
                      info_tbl_ptr->cid, info_tbl_ptr->index );
    }

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
    /* When we receive a DSI_EVT_NET_NEWADDR event during a Dual-IP partial
       retry, send a setup data call success to the upper layer */
    if (TRUE == info_tbl_ptr->is_partial_retry)
    {
      info_tbl_ptr->is_partial_retry = FALSE;

      if (0 == info_tbl_ptr->last_addr_count)
      {
        QCRIL_LOG_DEBUG( "Dual-IP Partial Retry: last_addr_count=%d, "
                         "propagating event", info_tbl_ptr->last_addr_count );

        info_tbl_ptr->partial_retry_count = 0;

        /* Schedule a partial retry attempt if possible */
        qcril_data_util_schedule_partial_retry_attempt(info_tbl_ptr, TRUE, TRUE);
      }
      else if (dsi_get_ip_addr_count(info_tbl_ptr->dsi_hndl) < DSI_NUM_IP_FAMILIES)
      {
        QCRIL_LOG_ERROR( "%s", "Dual-IP Partial Retry: Both address families not up, "
                         "ignoring spurious event" );
        goto ret;
      }

      QCRIL_LOG_DEBUG( "%s", "Dual-IP Partial Retry: Success" );

      /* We got the data call setup response, so stop the response timer */
      qcril_data_util_stop_timer(&info_tbl_ptr->timer_id);

      info_tbl_ptr->partial_retry_count = 0;

      /* Stop the partial retry timer, just in case it is still running */
      qcril_data_util_disarm_timer(&info_tbl_ptr->retry_timer_id);
    }
    /* We got a new address assigned for a Dual-IP call, stop the retry timer */
    else if (DSI_IP_VERSION_4_6 == info_tbl_ptr->dsi_ip_version &&
             info_tbl_ptr->last_addr_count < DSI_NUM_IP_FAMILIES)
    {
      /* Stop the setup timer, just in case it is still running */
      qcril_data_util_stop_timer(&info_tbl_ptr->timer_id);

      /* Stop the partial retry timer, just in case it is still running */
      qcril_data_util_disarm_timer(&info_tbl_ptr->retry_timer_id);

      info_tbl_ptr->partial_retry_count = 0;
    }
#endif /* (RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6) */
    /* Intentional fallthrough */

  case DSI_EVT_NET_DELADDR:
    if (DSI_EVT_NET_DELADDR == evt_info_ptr->evt)
    {
      QCRIL_LOG_INFO( ">>>DSI_EVT_NET_DELADDR: START>>> cid [%d], index [%d]",
                      info_tbl_ptr->cid, info_tbl_ptr->index );
    }

    /* Get and fill all call attributes */
    qcril_data_util_fill_call_params( info_tbl_ptr );

    /* Update call_tbl to point to latest call info */
    qcril_data_util_update_call_state( info_tbl_ptr, CALL_ACTIVE_PHYSLINK_UP, PDP_FAIL_NONE );

    if (DSI_EVT_NET_RECONFIGURED == evt_info_ptr->evt)
    {
      QCRIL_LOG_INFO( "%s", "<<<DSI_EVT_NET_RECONFIGURED: DONE<<<" );
    }
    else if (DSI_EVT_NET_NEWADDR == evt_info_ptr->evt)
    {
      QCRIL_LOG_INFO( "%s", "<<<DSI_EVT_NET_NEWADDR: DONE<<<" );
    }
    else if (DSI_EVT_NET_DELADDR == evt_info_ptr->evt)
    {
      /* Schedule a partial retry attempt if possible */
      qcril_data_util_schedule_partial_retry_attempt(info_tbl_ptr, TRUE, TRUE);

      QCRIL_LOG_INFO( "%s", "<<<DSI_EVT_NET_DELADDR: DONE<<<" );
    }

    goto unsol_rsp;

  case DSI_EVT_NET_NEWMTU:
    QCRIL_LOG_INFO( ">>>DSI_EVT_NET_NETMTU: START>>> cid [%d], index [%d]",
                    info_tbl_ptr->cid, info_tbl_ptr->index );

#if (RIL_VERSION >= 11)

    qcril_data_util_update_mtu(info_tbl_ptr);

#endif

    QCRIL_LOG_INFO( "%s", "<<<DSI_EVT_NET_NEWMTU: DONE<<<" );
    goto unsol_rsp;

  case DSI_EVT_QOS_STATUS_IND:

    QCRIL_LOG_INFO( ">>>DSI_EVT_QOS_STATUS_IND: START>>> cid [%d], index [%d]",
                    info_tbl_ptr->cid, info_tbl_ptr->index );

#if (RIL_QCOM_VERSION >= 2)
    /* Process QOS status indication to generate response */
    if( SUCCESS != qcril_data_qos_status_ind_handler( evt_info_ptr,
                                                      (void**)&response_ptr,
                                                      &response_len ) )
    {
      QCRIL_LOG_ERROR( "%s", "error on qcril_data_qos_status_ind_handler" );
      goto err_label;
    }

    QCRIL_LOG_DEBUG( "%s", ">>>RIL UNSOL RSP READY>>>" );
    qcril_data_unsol_qos_state_changed( instance_id,
                                        response_ptr,
                                        response_len );
    QCRIL_LOG_DEBUG( "%s", "<<<RIL UNSOL RSP SENT<<<" );
#endif /* RIL_QCOM_VERSION >= 2 */

    /* TODO: extend this when list of flows supported */
    QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.dummy.string1 );
    QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.dummy.string2 );
    QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.dummy.string3 );
    QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.dummy.string4 );

    QCRIL_LOG_INFO( "%s", "<<<DSI_EVT_QOS_STATUS_IND: DONE<<<" );
    goto ret;

  default:
    QCRIL_LOG_ERROR( "invalid dsi_net_ev [%d]", (dsi_net_evt_t)params_ptr->data );
    QCRIL_DS_ASSERT( 0, "validate dsi_net_ev" );
    goto err_label_exit;
  }/* switch() */

unsol_rsp:
  qcril_data_unsol_call_list_changed( instance_id);
ret:
  QCRIL_DS_ASSERT( evt_info_ptr != NULL, "validate event obj" );
  QCRIL_LOG_INFO( "%s", "try event obj dealloc" );
  if ( evt_info_ptr != NULL ) free( evt_info_ptr );
  QCRIL_LOG_DEBUG( "%s", "qcril_data_event_hdlr: EXIT with suc" );
  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
  return;

err_label:
  QCRIL_DS_ASSERT( evt_info_ptr != NULL, "validate event obj" );
  QCRIL_LOG_INFO( "%s", "try event obj dealloc" );
  if ( evt_info_ptr != NULL ) free( evt_info_ptr );

err_label_no_free:
  if ( qcril_reqlist_query( instance_id, params_ptr->t, &reqlist_info ) == E_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s", "respond to QCRIL as generic failure" );
    qcril_data_response_generic_failure( instance_id, reqlist_info.t, reqlist_info.request );
  }

err_label_exit:
  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
  QCRIL_LOG_ERROR("%s", "qcril_data_event_hdlr: EXIT with err");
  return;

}/* qcril_data_event_hdlr() */



/*===========================================================================
  FUNCTION:  qcril_data_wds_event_hdlr

===========================================================================*/
/*!
    @brief
    Registered with QCRIL to be called by QCRIL on event
    QCRIL_EVT_DATA_WDS_EVENT_CALLBACK

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_wds_event_hdlr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr  /*output*/
)
{
  qcril_data_wds_event_data_t* event_data;
  QCRIL_LOG_DEBUG( "%s", "qcril_data_wds_event_hdlr: ENTRY" );

  do
  {
    if (!params_ptr || !ret_ptr)
    {
      QCRIL_LOG_ERROR("%s", "invalid parameters");
      break;
    }

    if (params_ptr->instance_id >= QCRIL_MAX_INSTANCE_ID)
    {
      QCRIL_LOG_ERROR("%s", "invalid instance id");
      break;
    }

    event_data = params_ptr->data;

    if (!event_data)
    {
      QCRIL_LOG_ERROR("%s", "invalid event data pointer");
      break;
    }

    QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

    qcril_data_process_wds_ind(event_data->wds_hndl,
                               event_data->sid,
                               event_data->user_data,
                               event_data->ind_id,
                               &event_data->ind_data);

    /* Free network_info if it was allocated earlier */
    if ((event_data->ind_data.event_report.event_mask & QMI_WDS_EVENT_DATA_SYS_STATUS_IND) &&
        NULL != event_data->ind_data.event_report.data_sys_status.network_info)
    {
      free(event_data->ind_data.event_report.data_sys_status.network_info);
      event_data->ind_data.event_report.data_sys_status.network_info = NULL;
    }
    free(event_data);

    QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
  } while (0);

  QCRIL_LOG_DEBUG( "%s", "qcril_data_wds_event_hdlr: EXIT" );
}/*qcril_data_wds_event_hdlr*/


/*===========================================================================
  FUNCTION:  qcril_data_dsd_event_hdlr

===========================================================================*/
/*!
    @brief
    Registered with QCRIL to be called by QCRIL on event
    QCRIL_EVT_DATA_DSD_EVENT_CALLBACK

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_dsd_event_hdlr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr  /*output*/
)
{
  dsd_system_status_ind_msg_v01  *ind_data = NULL;
  QCRIL_LOG_DEBUG( "%s", "qcril_data_dsd_event_hdlr: ENTRY" );

  do
  {
    if (!params_ptr || !ret_ptr)
    {
      QCRIL_LOG_ERROR("%s", "invalid parameters");
      break;
    }

    if (params_ptr->instance_id >= QCRIL_MAX_INSTANCE_ID)
    {
      QCRIL_LOG_ERROR("%s", "invalid instance id");
      break;
    }

    ind_data = params_ptr->data;

    if (!ind_data)
    {
      QCRIL_LOG_ERROR("%s", "invalid event data pointer");
      break;
    }

    qcril_data_process_qmi_dsd_ind(ind_data);

    free(ind_data);
  } while (0);

  QCRIL_LOG_DEBUG( "%s", "qcril_data_dsd_event_hdlr: EXIT" );
}


/*-------------------------------------------------------------------------

                           UTILITY ROUTINES

-------------------------------------------------------------------------*/
/*===========================================================================

  FUNCTION:  qcril_data_go_dormant

===========================================================================*/
/*!
    @brief
    puts the physlink corresponding to the qmi_wds_hndl dormant

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_iface_go_dormant
(
  int qmi_wds_hndl,
  dsi_ip_family_t ip_family
)
{
  int ret = FAILURE;
  int rval, qmi_err;

  do
  {

    if (QCRIL_DATA_HNDL_INVALID == qmi_wds_hndl)
    {
      QCRIL_LOG_ERROR("%s","invalid qmi_wds_hndl received");
      break;
    }

    rval = qmi_wds_go_dormant_req(qmi_wds_hndl,&qmi_err);

    if (rval < 0)
    {
      QCRIL_LOG_ERROR("qmi_wds_go_dormant_req failed with err %d "\
                      "qmi_err %d", rval, qmi_err);
      break;
    }

    ret = SUCCESS;
  } while (0);

  return ret;
}

/*===========================================================================

  FUNCTION:  qcril_data_iface_set_mtu

===========================================================================*/
static int qcril_data_iface_set_mtu
(
  dsi_hndl_t dsi_hndl,
  int mtu
)
{
  int ret = FAILURE, ioctl_err;
  struct ifreq ifr;
  int socket_fd;
  char iface_name[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1];

  /* Create a socket to set use for setting the MTU value*/
  socket_fd = socket(PF_INET,SOCK_DGRAM,0);
  if (socket_fd < 0)
  {
    QCRIL_LOG_ERROR("Creating socket failed. Could not set MTU to [%d]", mtu);
    goto socket_err_label;
  }

  /* Get the name of the interface */
  if (DSI_ERROR == dsi_get_device_name(dsi_hndl,
                                       iface_name,
                                       DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1))
  {
    QCRIL_LOG_ERROR("couldn't get device name. Could not set MTU to [%d]", mtu);
    goto err_label;
  }
  iface_name[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN] = '\0';
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_ifrn.ifrn_name, iface_name, sizeof(ifr.ifr_ifrn.ifrn_name) );

  /* Set the correct MTU value*/
  ifr.ifr_ifru.ifru_mtu = mtu;
  ioctl_err = ioctl(socket_fd, SIOCSIFMTU, &ifr);
  if ( ioctl_err < 0 )
  {
    QCRIL_LOG_ERROR("Error setting MTU to [%d] on interface [%s] error %d", mtu, iface_name, ioctl_err);
    goto err_label;
  }

  QCRIL_LOG_DEBUG("MTU set to [%d] on interface [%s]", mtu, iface_name);
  ret = SUCCESS;

err_label:
  close(socket_fd);
socket_err_label:
  return ret;
}


/*===========================================================================

  FUNCTION:  qcril_data_iface_ioctl

===========================================================================*/
/*!
    @brief
    executes the given ioctl on the given interface

    if ioctl is QCRIL_DATA_IOCTL_DORMANCY_INDICATIONS_ON,
    and if current dormancy status has changed from the last
    global status we were aware of, dorm_status_changed would be
    set to TRUE, and call_state would be set to appropriate
    PHYSLINK status.
    CALL_ACTIVE_PHYSLINK_DOWN
    CALL_ACTIVE_PHYSLINK_UP

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_iface_ioctl
(
  qcril_data_call_info_tbl_type * info_tbl_ptr,
  int ioctl,
  boolean * dorm_status_changed,
  int * call_state
)
{
  boolean dorm_ind = FALSE;
  int ret = FAILURE;
  int reti = SUCCESS;
  qmi_wds_event_report_params_type event_report_params;
  int rc, qmi_err;
  int qmi_wds_hndl;
  dsi_net_evt_t dsi_event;
  int state;

  do
  {

    if(!VALIDATE_LOCAL_DATA_OBJ(info_tbl_ptr))
    {
      QCRIL_LOG_ERROR("%s","invlaid info_tbl_ptr received");
      break;
    }

    qmi_wds_hndl = info_tbl_ptr->qmi_wds_hndl;

    if(QCRIL_DATA_HNDL_INVALID == qmi_wds_hndl)
    {
      QCRIL_LOG_ERROR("Invalid qmi_wds_hndl found in this info_tbl_ptr %x",
                      info_tbl_ptr);
      break;
    }

    if(NULL == dorm_status_changed || NULL == call_state)
    {
      QCRIL_LOG_ERROR("%s","bad parameters received");
      break;
    }

    *dorm_status_changed = FALSE;

    switch(ioctl)
    {
    case QCRIL_DATA_IFACE_IOCTL_DORMANCY_INDICATIONS_OFF:
    case QCRIL_DATA_IFACE_IOCTL_DORMANCY_INDICATIONS_ON:
      if (QCRIL_DATA_IFACE_IOCTL_DORMANCY_INDICATIONS_OFF == ioctl)
      {
        dorm_ind = FALSE;
      }
      else
      {
        dorm_ind = TRUE;
      }

      event_report_params.param_mask = FALSE;
      event_report_params.param_mask |= QMI_WDS_EVENT_DORM_STATUS_IND;
      event_report_params.report_dorm_status = dorm_ind;

      QCRIL_LOG_DEBUG("setting dormancy event report on wds client [%d]",
                      qmi_wds_hndl);
      if ((rc = qmi_wds_set_event_report(qmi_wds_hndl, &event_report_params,&qmi_err) < 0))
      {
        QCRIL_LOG_ERROR("dsc_qmi_wds_util_set_event_report: " \
                        "Set event report failed with with error %d, qmi_err %d",
                        rc,qmi_err);
        reti = FAILURE;
        break;
      }

      if (dorm_ind == TRUE)
      {
        QCRIL_LOG_DEBUG("getting dormancy status for wds client [%d]",
                        qmi_wds_hndl);
        rc = qmi_wds_get_dormancy_status(qmi_wds_hndl,
                                         &info_tbl_ptr->dorm_status,
                                         &qmi_err);

        if (rc < 0)
        {
          QCRIL_LOG_ERROR("qmi_wds_get_dormancy_status failed " \
                          "error: %d, qmi_err %d", rc, qmi_err);
          reti = FAILURE;
          break;
        }

        if (info_tbl_ptr->dorm_status != global_dorm_status)
        {
          QCRIL_LOG_VERBOSE("dormancy status has changed since "     \
                          "last registration");
          if (info_tbl_ptr->dorm_status == QMI_WDS_DORM_STATUS_ACTIVE)
          {
            /* dsi_event = DSI_EVT_PHYSLINK_UP_STATE; */
            state = CALL_ACTIVE_PHYSLINK_UP;
            /*
            QCRIL_LOG_INFO("%s", "<<<DSI_EVT_PHYSLINK_UP " \
            "processing started<<<");*/
          }
          else
          {
            /* dsi_event = DSI_EVT_PHYSLINK_DOWN_STATE; */
            state = CALL_ACTIVE_PHYSLINK_DOWN;
            /* QCRIL_LOG_INFO("%s", "<<<DSI_EVT_PHYSLINK_DOWN "  \
               "processing started<<<"); */
          }

          *dorm_status_changed = TRUE;
          *call_state = state;
        }

        global_dorm_status = info_tbl_ptr->dorm_status;
      }

      break;
    case QCRIL_DATA_IFACE_IOCTL_GO_DORMANT:
      reti = qcril_data_iface_go_dormant(qmi_wds_hndl, info_tbl_ptr->dsi_ip_version);
      break;
    default:
      QCRIL_LOG_ERROR("unsupported ioctl %d", ioctl);
      reti = FAILURE;
      break;
    }
    if(reti == FAILURE)
    {
      break;
    }

    ret = SUCCESS;
  } while (0);

  return ret;
}

/*===========================================================================

  FUNCTION:  qcril_data_set_nai

===========================================================================*/
/*!
    @brief
    Sets the RIL provided NAI (username, password, auth_pref)
    on the dsi_hndl

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_set_nai
(
  int info_tbl_index,
  const char * ril_user,
  const char * ril_pass,
  const char * ril_auth_pref
)
{
  int ret = FAILURE;
  int reti = SUCCESS;
  dsi_call_param_value_t username_info, password_info,authpref_info;

  do
  {

    memset( &username_info, 0, sizeof( dsi_call_param_value_t ) );
    memset( &password_info, 0, sizeof( dsi_call_param_value_t ) );
    memset( &authpref_info, 0, sizeof( dsi_call_param_value_t ) );

    if(ril_auth_pref != NULL && strlen(ril_auth_pref) !=0)
    {
      /* set auth_pref */
      authpref_info.buf_val = NULL;
      authpref_info.num_val = atoi(ril_auth_pref);
      reti = SUCCESS;
      switch(authpref_info.num_val)
      {
      case 0:
        authpref_info.num_val = DSI_AUTH_PREF_PAP_CHAP_NOT_ALLOWED;
        break;
      case 1:
        authpref_info.num_val = DSI_AUTH_PREF_PAP_ONLY_ALLOWED;
        break;
      case 2:
        authpref_info.num_val = DSI_AUTH_PREF_CHAP_ONLY_ALLOWED;
        break;
      case 3:
        authpref_info.num_val = DSI_AUTH_PREF_PAP_CHAP_BOTH_ALLOWED;
        break;
      default:
        QCRIL_LOG_ERROR("invalid auth pref received [%d]", authpref_info.num_val);
        reti = FAILURE;
        break;
      }
      if (SUCCESS != reti)
      {
        break;
      }
      if((dsi_set_data_call_param(info_tbl[info_tbl_index].dsi_hndl,
                                  DSI_CALL_INFO_AUTH_PREF,
                                  &authpref_info) ) != DSI_SUCCESS)
      {
        QCRIL_LOG_ERROR("unable to set AUTH PREF [%d], info_tbl index [%d]",
                        authpref_info.num_val,info_tbl_index);
        break;
      }
    }

    /* set username */
    if ((ril_user != NULL) && strlen(ril_user) != 0)
    {
      username_info.buf_val = (void *)ril_user;
      username_info.num_val = strlen( ril_user );
      QCRIL_LOG_VERBOSE( "RIL provided UserName, len [%d]", username_info.num_val);
      if ( ( dsi_set_data_call_param( info_tbl[info_tbl_index].dsi_hndl ,
                                      DSI_CALL_INFO_USERNAME,
                                     &username_info ) ) != DSI_SUCCESS )
      {
        QCRIL_LOG_ERROR("unable to set username [%s], info_tbl index [%d]",
                        ril_user, info_tbl_index);
        break;
      }
    }

    /* set password */
    if ((ril_pass != NULL) && strlen(ril_pass) != 0)
    {
      password_info.buf_val = (void *)ril_pass;
      password_info.num_val = strlen( ril_pass );
      QCRIL_LOG_VERBOSE( "RIL provided Password, len [%d]", password_info.num_val);

      if ( ( dsi_set_data_call_param( info_tbl[info_tbl_index].dsi_hndl ,
                                      DSI_CALL_INFO_PASSWORD,
                                      &password_info ) ) != DSI_SUCCESS )
      {
        QCRIL_LOG_ERROR("unable to set password [%s], info_tbl index [%d]",
                        ril_pass,info_tbl_index);
        break;
      }
    }
    ret = SUCCESS;
  }while(0);

  if (SUCCESS == ret)
  {
    QCRIL_LOG_DEBUG("%s", "qcril_data_set_nai successful");

  }
  else
  {
    QCRIL_LOG_DEBUG("%s", "qcril_data_set_nai failed");
  }

  return ret;
}

/*===========================================================================

  FUNCTION:  qcril_data_set_ril_profile_id

===========================================================================*/
/*!
    @brief
    Sets the RIL provided profile id (technology specific) into the
    dsi_hndl

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_set_ril_profile_id
(
  int info_tbl_index,
  const char * ril_profile,
  const char * ril_tech
)
{
  int ret = FAILURE;
  int reti = SUCCESS;
  dsi_call_param_value_t profile_id;
  dsi_call_param_value_t call_type;
  int profile_param_id = 0;

  do
  {
    /* sanity checking */
    if (NULL == ril_profile)
    {
      QCRIL_LOG_ERROR("%s","NULL ril_profile received");
      QCRIL_LOG_ERROR("%s","Ignoring profile id for SETUP_DATA_CALL");
      return SUCCESS;
    }

    if ((RIL_DATA_PROFILE_DEFAULT == atoi(ril_profile)))
    {
      QCRIL_LOG_ERROR("default profile id [%d] provided. " \
                      "no need to set in dsi_hndl", atoi(ril_profile));
      ret = SUCCESS;
      break;
    }

    if(ril_tech == NULL)
    {
      QCRIL_LOG_ERROR("RIL provided profile id without specifying" \
                      " the technology (CDMA/UMTS). This profile id" \
                      " [%s] will be ignored", ril_profile);
      break;
    }

    /* prepare dsi parameter with profile id */
    memset( &profile_id, 0, sizeof( dsi_call_param_value_t ) );
    profile_id.buf_val = NULL;
    profile_id.num_val = atoi(ril_profile);
    QCRIL_LOG_VERBOSE("RIL provided PROFILE ID Number [%d]",
                      profile_id.num_val);

    /* adjust the profile id according to ril.h */
    if (profile_id.num_val < RIL_DATA_PROFILE_OEM_BASE)
    {
      reti = SUCCESS;
      switch(profile_id.num_val)
      {
      case RIL_DATA_PROFILE_TETHERED:
        QCRIL_LOG_VERBOSE("RIL provided [%d] profile id. Tethered call "
                          "will be used", profile_id.num_val);
        call_type.buf_val = NULL;
        call_type.num_val = DSI_CALL_TYPE_TETHERED;
        if (dsi_set_data_call_param(info_tbl[info_tbl_index].dsi_hndl,
                                    DSI_CALL_INFO_CALL_TYPE,
                                    &call_type) != DSI_SUCCESS)
        {
          QCRIL_LOG_ERROR("Couldn't set call_type [%d]",
                          call_type.num_val);
          reti = FAILURE;
          break;
        }
        break;
      default:
        QCRIL_LOG_ERROR("RIL provided [%d] profile id. This is currently "
                        "not supported", profile_id.num_val);
      }
      if (reti != SUCCESS)
      {
        break;
      }
    }
    else
    {
      /* adjust for the OEM base */
      profile_id.num_val -= RIL_DATA_PROFILE_OEM_BASE;
      QCRIL_LOG_VERBOSE("profile_id.num_val %d will be used (android provided %d)",
                        profile_id.num_val,
                        profile_id.num_val + RIL_DATA_PROFILE_OEM_BASE);
      /* figure out whether this is umts or cdma profile id */
      if (!strcmp(ril_tech, QCRIL_CDMA_STRING))
      {
        profile_param_id = DSI_CALL_INFO_CDMA_PROFILE_IDX;
      }
      else if (!strcmp(ril_tech, QCRIL_UMTS_STRING))
      {
        profile_param_id = DSI_CALL_INFO_UMTS_PROFILE_IDX;
      }
      else
      {
        QCRIL_LOG_ERROR("RIL provided incorrect/malformed technology [%s]",
                        ril_tech);
        break;
      }

      /* now set the profile id onto dsi_hndl */
      if ( ( dsi_set_data_call_param(info_tbl[info_tbl_index].dsi_hndl,
                                     profile_param_id,
                                     &profile_id ) ) != DSI_SUCCESS )
      {
        QCRIL_LOG_ERROR("unable to set profile id [%d], info_tbl index [%d]",
                        profile_id.num_val, info_tbl_index);
        break;
      }
    }


    ret = SUCCESS;
  } while (0);

  if (SUCCESS == ret)
  {
    QCRIL_LOG_DEBUG("%s", "qcril_data_set_ril_profile_id successful");

  }
  else
  {
    QCRIL_LOG_DEBUG("%s", "qcril_data_set_ril_profile_id failed");
  }

  return ret;
}

/*===========================================================================

  FUNCTION:  qcril_data_apn_based_profile_look_up_using_qdp

===========================================================================*/
/*!
    @brief
    Looks up the 3GPP and 3GPP2 profile IDs based on the APN string
    and IP_FAMILY. QDP considers RIL provided optional technology and
    profile_id to determine which profiles(if any) need to be looked
    up or created.

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_apn_based_profile_look_up_using_qdp
(
  const char * ril_apn,
  const char * ril_ipfamily,
  const char * ril_tech,
  const char * ril_profile,
  const char * ril_user,
  const char * ril_pass,
  const char * ril_auth_pref,
  int * umts_profile_id,
  qdp_profile_pdn_type *umts_profile_pdn_type,
  int * cdma_profile_id,
  qdp_profile_pdn_type *cdma_profile_pdn_type,
  RIL_DataCallFailCause * cause,
  boolean * abort_call
)
{
  int ret = FAILURE;
  const char * qdp_params[QDP_RIL_PARAM_MAX];
  qdp_tech_t current_tech = QDP_NOTECH;
  qdp_error_info_t error_info;
  int i=0, rc;

  QCRIL_LOG_DEBUG("%s","qcril_data_apn_based_profile_look_up_using_qdp: ENTRY");

  do
  {
    if( NULL == umts_profile_id       ||
        NULL == umts_profile_pdn_type ||
        NULL == cdma_profile_id       ||
        NULL == cdma_profile_pdn_type ||
        NULL == cause                 ||
        NULL == abort_call )
    {
      QCRIL_LOG_ERROR("%s","NULL placeholders provided");
      break;
    }

    /* only APN is mandatory RIL param. Rest are optional RIL params */
    if (NULL == ril_apn)
    {
      QCRIL_LOG_ERROR("%s","ril_apn (provided NULL) is mandatory param for "
                      "this function.");
      break;
    }

    memset(qdp_params, 0, sizeof(qdp_params));

    /* build up QDP parameters */
    /* prepare APN */
    if (strlen(ril_apn) > (QMI_WDS_MAX_APN_STR_SIZE-1))
    {
      QCRIL_LOG_ERROR("RIL provided invalid APN [%s] "
                      "APN length [%d] exceeds max allowed [%d]",
                      ril_apn, strlen(ril_apn), QMI_WDS_MAX_APN_STR_SIZE-1);
      break;
    }
    qdp_params[QDP_RIL_APN] = ril_apn;

    QCRIL_LOG_DEBUG("qdp_param APN = [%s]", qdp_params[QDP_RIL_APN]);
    /* prepare technology */
    if (ril_tech != NULL && strlen(ril_tech)>0)
    {
      if (0 == strncmp(QDP_RIL_3GPP, ril_tech, QDP_RIL_TECH_LEN))
      {
        current_tech = QDP_3GPP;
      }
      else if(0 == strncmp(QDP_RIL_3GPP2, ril_tech, QDP_RIL_TECH_LEN))
      {
        current_tech = QDP_3GPP2;
      }
      else
      {
        QCRIL_LOG_ERROR("RIL provided unsupported technology [%s]", ril_tech);
        break;
      }

      /* do not use RIL technology for profile look up */
#ifndef FEATURE_QCRIL_FUSION
      /* for targets that are not Fusion I devices, we rely on
       * ro.baseband property to determine if the ril tech should be
       * ignored or not (done at power up) */
      if (FALSE == ignore_ril_tech)
      {
        if (strlen(ril_tech) > QDP_RIL_TECH_LEN)
        {
          QCRIL_LOG_ERROR("RIL provided invalid technology [%s] "
                          "tech length [%d] exceeds max allowed [%d]",
                          ril_tech, strlen(ril_tech), QDP_RIL_TECH_LEN);
          break;
        }
        qdp_params[QDP_RIL_TECH] = ril_tech;
        QCRIL_LOG_DEBUG("qdp_param TECH = [%s]", qdp_params[QDP_RIL_TECH]);
      }
      else
      {
        QCRIL_LOG_DEBUG("%s","ignore_ril_tech is set to TRUE");
        QCRIL_LOG_DEBUG("%s","qdp param TECH not set on purpose in order to"                             " enforce 3gpp and 3gpp2 profile look up");
      }
#else
      QCRIL_LOG_DEBUG("%s","qdp param TECH not set on purpose in order to"
                           " enforce 3gpp and 3gpp2 profile look up");
#endif
    }

    /* prepare profile id */
    if (ril_profile != NULL && strlen(ril_profile)>0)
    {
      if (strlen(ril_profile) > QCRIL_PROFILE_ID_STR_MAX)
      {
        QCRIL_LOG_ERROR("RIL provided invalid profile id [%s] "
                        "profile id length [%d] exceeds max allowed [%d]",
                        ril_profile, strlen(ril_profile), QCRIL_PROFILE_ID_STR_MAX);
        break;
      }
      qdp_params[QDP_RIL_PROFILE_ID] = ril_profile;

      QCRIL_LOG_DEBUG("qdp param PROFILE_ID = [%s]", qdp_params[QDP_RIL_PROFILE_ID]);
    }
    /* prepare IP family version */
    if (ril_ipfamily != NULL && strlen(ril_ipfamily) > 0)
    {
      if (strlen(ril_ipfamily) > QCRIL_IP_FAMILY_STR_MAX)
      {
        QCRIL_LOG_ERROR("RIL provided invalid ip family [%s] "
                        "ip family length [%d] exceeds max allowed [%d]",
                        ril_ipfamily, strlen(ril_ipfamily), QCRIL_IP_FAMILY_STR_MAX);
        break;
      }
      qdp_params[QDP_RIL_IP_FAMILY] = ril_ipfamily;

      QCRIL_LOG_DEBUG("qdp param IP_FAMILY = [%s]", qdp_params[QDP_RIL_IP_FAMILY]);
    }

    /* Prepare username */
    if (ril_user != NULL && strlen(ril_user) > 0)
    {
      if (strlen(ril_user) > QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1)
      {
        QCRIL_LOG_ERROR("RIL provided username exceeds max allowed [%d]",
                        QMI_WDS_MAX_USERNAME_PASS_STR_SIZE);
      }
      else
      {
        qdp_params[QDP_RIL_NAI] = ril_user;
        QCRIL_LOG_DEBUG("qdp param USERNAME = [%s]", qdp_params[QDP_RIL_NAI]);
      }
    }

    /* prepare password */
    if (ril_pass != NULL && strlen(ril_pass) > 0)
    {
      if (strlen(ril_pass) > QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1)
      {
        QCRIL_LOG_ERROR("RIL provided password exceeds max allowed [%d]",
                        QMI_WDS_MAX_USERNAME_PASS_STR_SIZE);
      }
      else
      {
        qdp_params[QDP_RIL_PASSWORD] = ril_pass;
        QCRIL_LOG_DEBUG("qdp param PASSWORD = [%s]", qdp_params[QDP_RIL_PASSWORD]);
      }
    }

    /* prepare authtype */
    if (ril_auth_pref != NULL && strlen(ril_auth_pref) > 0)
    {
      if (strlen(ril_auth_pref) > QCRIL_DATA_AUTH_PREF_MAX_LEN-1)
      {
        QCRIL_LOG_ERROR("RIL provided auth exceeds max allowed [%d]",
                        QCRIL_DATA_AUTH_PREF_MAX_LEN);
      }
      else
      {
        qdp_params[QDP_RIL_AUTH] = ril_auth_pref;
        QCRIL_LOG_DEBUG("qdp param AUTH type = [%s]", qdp_params[QDP_RIL_AUTH]);
      }
    }
    *umts_profile_id = *cdma_profile_id = 0;
    memset( &error_info, 0x0, sizeof(error_info) );
    *abort_call = FALSE;

    rc = qdp_profile_look_up( qdp_params,
                              (unsigned int *)umts_profile_id,
                              umts_profile_pdn_type,
                              (unsigned int *)cdma_profile_id,
                              cdma_profile_pdn_type,
                              &error_info );

    if( QDP_SUCCESS == rc )
    {
      QCRIL_LOG_DEBUG("successfully looked up 3gpp profile id [%d], type [%d] and "
                      "3gpp2 profile id [%d], type [%d] using ril params, "
                      "lookup_error[%d], abort call [%d]",
                      *umts_profile_id, *umts_profile_pdn_type,
                      *cdma_profile_id, *cdma_profile_pdn_type,
                      error_info.error, error_info.tech);
    }
    else
    {
      QCRIL_LOG_DEBUG("not able to look up both profile ids. "
                      "3gpp profile id [%d], type [%d], and 3gpp2 profile id [%d], type [%d] "
                      "returned by QDP, lookup error[%d] tech[%d]",
                      *umts_profile_id, *cdma_profile_id,
                      *umts_profile_pdn_type, *cdma_profile_pdn_type,
                      error_info.error, error_info.tech);

      /* Check if the technology reporting failed lookup matches the
       * tech reported by RIL API as current service.  On match, abort
       * the call setup request. */
      if( (QDP_ERROR_NONE != error_info.error) &&
          (current_tech == error_info.tech) )
      {
        QCRIL_LOG_DEBUG("lookup[%d] and RIL[%d] tech match, abort call.",
                        error_info.tech, current_tech);
        *abort_call = TRUE;

        /* Map lookup_error to RIL API error */
        switch( error_info.error )
        {
          case QDP_ERROR_ONLY_IPV4_ALLOWED:
            *cause = PDP_FAIL_ONLY_IPV4_ALLOWED;
            break;
          case QDP_ERROR_ONLY_IPV6_ALLOWED:
            *cause = PDP_FAIL_ONLY_IPV6_ALLOWED;
            break;
          case QDP_ERROR_NONE:
            *cause = PDP_FAIL_NONE;
            break;
          default:
            QCRIL_LOG_ERROR("unsupported lookup_error[%d]", error_info.error);
        }
      }
      break;
    }
    ret = SUCCESS;
  } while (0);

  QCRIL_LOG_DEBUG("%s","qcril_data_apn_based_profile_look_up_using_qdp: EXIT");

  return ret;
}

/*===========================================================================

  FUNCTION:  qcril_data_apn_based_profile_look_up

===========================================================================*/
/*!
    @brief
    Looks up the 3GPP and 3GPP2 profile IDs based on the APN string

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
#define QMI_WDS_CDMA_PROFILE_APN_STRING_TLV_ID 0x21
#define QMI_WDS_UMTS_PROFILE_APN_NAME_TLV_ID 0x14
#define QCRIL_DATA_NUM_UMTS_PROFILES_EXPECTED 8
static int qcril_data_apn_based_profile_look_up
(
  const char * str,
  int * umts_profile_id,
  int * cdma_profile_id
)
{
  int ret = FAILURE;
  int qmi_wds_hndl = QCRIL_DATA_HNDL_INVALID;
  int qmi_err_code, rc;
  qmi_wds_profile_node_list_type cdma_list;
  qmi_wds_profile_node_list_type umts_list;
  qmi_wds_profile_node_type umts_node;
  qmi_wds_profile_node_type cdma_node;
  qmi_wds_profile_tech_type profile_tech;
  int num_elements_expected = 1;
  int num_elements_expected_3gpp = QCRIL_DATA_NUM_UMTS_PROFILES_EXPECTED;
  qmi_wds_profile_list_type result_list;
  qmi_wds_profile_list_type result_list_3gpp[QCRIL_DATA_NUM_UMTS_PROFILES_EXPECTED];

  do
  {
    /* sanity check */
    if (NULL == str ||
        NULL == umts_profile_id ||
        NULL == cdma_profile_id)
    {
      QCRIL_LOG_ERROR("%s","invalid parameters received");
      break;
    }

    /* make sure APN fits the limit */
    if (strlen(str) > QMI_WDS_MAX_APN_STR_SIZE-1)
    {
      QCRIL_LOG_ERROR("APN string [%s] is too long [%d]." \
                      " max allowed string size is [%d]",
                      str, strlen(str),
                      QMI_WDS_MAX_APN_STR_SIZE);
      break;
    }

    QCRIL_LOG_DEBUG("Using QMI port [%s] for profile look up",
                    default_qmi_port);

    /* get wds hndl */
    /* default qmi port is set to MDM or MSM based on whether
     * this is Fusion or non-Fusion respectively */
    qmi_wds_hndl = qmi_wds_srvc_init_client(default_qmi_port,
                                            NULL,
                                            NULL,
                                            &qmi_err_code);

    if (qmi_wds_hndl < 0)
    {
      QCRIL_LOG_ERROR("invalid qmi_wds_hndl [%d] returned. " \
                      "qmi_err_code is set to [%d]",
                      qmi_wds_hndl, qmi_err_code);
      qmi_wds_hndl = QCRIL_DATA_HNDL_INVALID;
      break;
    }

    if (QMI_NO_ERR != qmi_wds_bind_subscription ( qmi_wds_hndl,
                                              global_subs_id,
                                              &qmi_err_code))
    {
      QCRIL_LOG_ERROR("qmi_wds_bind_subscription: subs_id binding failed");
    }
    /* this is a void function */
    /* init the key-value pair list */
    list_init(&umts_list);

    umts_node.profile_element.type = QMI_WDS_UMTS_PROFILE_APN_NAME_TLV_ID;
    umts_node.profile_element.len = strlen(str);
    umts_node.profile_element.data = (void *)str;

    list_push_back(&umts_list, (qmi_wds_list_link_type *)&umts_node);

    memset(&result_list,0,sizeof(qmi_wds_profile_list_type));
    /* query umts profile id. */
    profile_tech = QMI_WDS_PROFILE_TECH_3GPP;
    /* VK: hack to work around the modem limitation
     * on querying with search string. If this hack
     * is enabled, we bring in entire list of profiles
     * and search through it manually */
#ifndef HACK_MODEM_QUERY_SEARCH_UNAVAIL
    rc = qmi_wds_utils_get_profile_list(
      qmi_wds_hndl,
      &profile_tech,
      &umts_list,
      &result_list,
      &num_elements_expected,
      &qmi_err_code);
#else
    rc = qmi_wds_utils_get_profile_list(
      qmi_wds_hndl,
      &profile_tech,
      NULL,
      result_list_3gpp,
      &num_elements_expected_3gpp,
      &qmi_err_code);
#endif
    QCRIL_LOG_DEBUG("qmi_wds_utils_get_profile_list returned [%d] " \
                    "return code for 3GPP technology", rc);
    if (QMI_NO_ERR != rc)
    {
      QCRIL_LOG_ERROR("get_profile_list failed with error [%d] " \
                      "qmi_err_code [%d]", rc, qmi_err_code);
      break;
    }
    else
    {
#ifndef HACK_MODEM_QUERY_SEARCH_UNAVAIL
      QCRIL_LOG_DEBUG("get_profile_list for profile_tech [%d] " \
                      "returned [%d] profile ids",
                      profile_tech, num_elements_expected);
      if (num_elements_expected == 1)
      {
      QCRIL_LOG_DEBUG("get_profile_list for profile_tech [%d] " \
                      "returned [%s] profile with id [%d]",
                      result_list.profile_type,
                      result_list.profile_name,
                      result_list.profile_index);
        *umts_profile_id = result_list.profile_index;
      }
      else
      {
        QCRIL_LOG_ERROR("qmi_wds_utils_get_profile_list returned [%d] results",
                        num_elements_expected);
        break;
      }
#else
      QCRIL_LOG_DEBUG("get_profile_list for profile_tech [%d] " \
                      "returned [%d] profile ids",
                      profile_tech, num_elements_expected_3gpp);
      {

        {
      int i;
      qmi_wds_profile_id_type prof_id;
      qmi_wds_profile_params_type prof_params;

      prof_params.umts_profile_params.param_mask = QMI_WDS_UMTS_PROFILE_APN_NAME_PARAM_MASK;

      for (i = 0; i < num_elements_expected_3gpp; ++i) {
        prof_id.technology = QMI_WDS_PROFILE_TECH_3GPP;
        prof_id.profile_index = result_list_3gpp[i].profile_index;
        rc = qmi_wds_query_profile(
        qmi_wds_hndl,
        &prof_id,
        &prof_params,
        &qmi_err_code
           );
            if (QMI_NO_ERR != rc)
            {
               QCRIL_LOG_ERROR("query_profile failed with error [%d] " \
                      "qmi_err_code [%d]", rc, qmi_err_code);
               break;
            }

        if (strncasecmp(str, prof_params.umts_profile_params.apn_name, strlen(str)) == 0) {
        /* match found */
        QCRIL_LOG_DEBUG("Match found for profile id [%d] ", prof_id.profile_index);
        *umts_profile_id = prof_id.profile_index;
            break;
        }
      }

      if ((QMI_NO_ERR != rc) || (i >= num_elements_expected_3gpp))
      {
        QCRIL_LOG_DEBUG("could not locate a 3gpp profile that matches"
                        "APN [%s]", str);
      }
        }
      }
#endif
    }

    /* this is a void function */
    /* init the key-value pair list */
    list_init(&cdma_list);

    cdma_node.profile_element.type = QMI_WDS_CDMA_PROFILE_APN_STRING_TLV_ID;
    cdma_node.profile_element.len = strlen(str);
    cdma_node.profile_element.data = (void *)str;

    list_push_back(&cdma_list, (qmi_wds_list_link_type *)&cdma_node);


    memset(&result_list,0,sizeof(qmi_wds_profile_list_type));
    /* query cdma profile id */
    profile_tech = QMI_WDS_PROFILE_TECH_3GPP2;
    rc = qmi_wds_utils_get_profile_list(
      qmi_wds_hndl,
      &profile_tech,
      &cdma_list,
      &result_list,
      &num_elements_expected,
      &qmi_err_code);
    QCRIL_LOG_DEBUG("qmi_wds_utils_get_profile_list returned [%d] " \
                    "return code for 3GPP2 technology", rc);
    if (QMI_NO_ERR != rc)
    {
      QCRIL_LOG_ERROR("get_profile_list failed with error [%d] " \
                      "qmi_err_code [%d]", rc, qmi_err_code);
      break;
    }
    else
    {
      QCRIL_LOG_DEBUG("get_profile_list for profile_tech [%d] " \
                      "returned [%d] profile ids",
                      profile_tech, num_elements_expected);


      if (num_elements_expected == 1)
      {
      QCRIL_LOG_DEBUG("get_profile_list for profile_tech [%d] " \
                      "returned [%s] profile with id [%d]",
                      result_list.profile_type,
                      result_list.profile_name,
                      result_list.profile_index);
        *cdma_profile_id = result_list.profile_index;
      }
      else
      {
        QCRIL_LOG_ERROR("qmi_wds_utils_get_profile_list returned [%d] results",
                        num_elements_expected);
        break;
      }
    }

    ret = SUCCESS;
  } while (0);

  if (qmi_wds_hndl > 0)
  {
    rc = qmi_wds_srvc_release_client(qmi_wds_hndl, &qmi_err_code);
    if (rc != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("couldn't release qmi wds hndl [%d] " \
                      "return code [%d], error code [%d]",
                      qmi_wds_hndl, rc, qmi_err_code);
    }
  }

  if (SUCCESS == ret)
  {
    QCRIL_LOG_DEBUG("%s", "qcril_data_apn_based_profile_look_up successful");

  }
  else
  {
    QCRIL_LOG_DEBUG("%s", "qcril_data_apn_based_profile_look_up failed");
  }

  return ret;

}

/*=========================================================================
  FUNCTION:  qcril_data_response_generic_failure

===========================================================================*/
/*!
    @brief
    Remove the entry from the ReqList. Send E_GENERIC_FAILURE response.

    @return
    None
*/
/*=========================================================================*/
void qcril_data_response_generic_failure
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int request
)
{
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_ERROR( "%s", "qcril_data_response_generic_failure: ENTRY" );

  /* Remove the entry from ReqList */
  (void) qcril_reqlist_free( instance_id, t );

  /* Send GenericFailure as the response to the RIL command */
  qcril_default_request_resp_params( instance_id, t, request, RIL_E_GENERIC_FAILURE, &resp );
  qcril_send_request_response( &resp );

  QCRIL_LOG_ERROR( "%s", "qcril_data_response_generic_failure: EXIT" );
} /* qcril_data_response_generic_failure */


#if !((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
/*=========================================================================
  FUNCTION:  qcril_data_response_data_call_failure

===========================================================================*/
/*!
    @brief
    Remove the entry from the ReqList. Send a RIL_E_SETUP_DATA_CALL_FAILURE
    cause code response.

    @return
    None
*/
/*=========================================================================*/
static void qcril_data_response_data_call_failure
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int request,
  RIL_DataCallFailCause response
)
{
  qcril_request_resp_params_type resp;
  char * resp_ptr = NULL;

  QCRIL_LOG_DEBUG("%s", "qcril_data_response_data_call_failure: ENTRY");

  /* Successfully remove the entry from ReqList */
  (void) qcril_reqlist_free(instance_id, t);

  /* Send FAILURE as the response to the RIL command */
  qcril_default_request_resp_params(instance_id,
                                    t,
                                    request,
                                    RIL_E_SETUP_DATA_CALL_FAILURE,
                                    &resp);

  /* Send error code response as string value */
  asprintf( &resp_ptr, "%d", response );
  resp.resp_pkt = &resp_ptr;
  resp.resp_len = sizeof(resp_ptr);

  qcril_send_request_response(&resp);
  free( resp_ptr );

  QCRIL_LOG_DEBUG("%s", "qcril_data_response_data_call_failure: EXIT");
} /* qcril_data_response_data_call_failure */
#endif /* !((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6)) */


/*=========================================================================
  FUNCTION:  qcril_data_response_setup_data_call_failure

===========================================================================*/
/*!
    @brief
    Generate setup data call error response based on RIL API version.

    @return
    None
*/
/*=========================================================================*/
static void qcril_data_response_setup_data_call_failure
(
  qcril_data_call_info_tbl_type * info_tbl_ptr,
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  RIL_DataCallFailCause response
)
{
  (void)info_tbl_ptr;
  (void)response;

  /* Invoke the proper error response based on RIL version */
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
  qcril_data_call_response_type  resp;

  qcril_data_util_update_call_state( info_tbl_ptr, CALL_INACTIVE, response );
  qcril_data_util_create_setup_rsp( info_tbl_ptr, &resp );
  qcril_data_response_success( instance_id,
                               t,
                               RIL_REQUEST_SETUP_DATA_CALL,
                               (void *) ( &(resp.setup_rsp) ),
                               sizeof( resp.setup_rsp ) );

#elif (RIL_VERSION >= 4)
  qcril_data_response_data_call_failure( instance_id,
                                         t,
                                         RIL_REQUEST_SETUP_DATA_CALL,
                                         response );
#else
  qcril_data_response_generic_failure( instance_id,
                                       t,
                                       RIL_REQUEST_SETUP_DATA_CALL );
#endif

} /* qcril_data_response_setup_data_call_failure */


/*=========================================================================
  FUNCTION:  qcril_data_response_setup_data_call_failure_generic_err

===========================================================================*/
/*!
    @brief
    Generate setup data call generic error response based on RIL API version.

    @return
    None
*/
/*=========================================================================*/
static void qcril_data_response_setup_data_call_failure_generic_err
(
  qcril_instance_id_e_type  instance_id,
  RIL_Token                 t
)
{
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))

#if (RIL_VERSION >= 11)

  /* Response to setup data call request */
  RIL_Data_Call_Response_v11  setup_rsp;

#elif (RIL_VERSION >= 10)

  /* Response to setup data call request */
  RIL_Data_Call_Response_v9  setup_rsp;

#else

  /* Response to setup data call request */
  RIL_Data_Call_Response_v6  setup_rsp;

#endif /* (RIL_VERSION >= 11)  */

  qcril_request_resp_params_type resp;

  memset( &setup_rsp, 0x0, sizeof(setup_rsp) );
  setup_rsp.status = PDP_FAIL_ERROR_UNSPECIFIED;
  setup_rsp.suggestedRetryTime = QCRIL_DATA_SUGGESTEDRETRYTIME;
  setup_rsp.cid = -1;

  qcril_default_request_resp_params( instance_id,
                                     t,
                                     RIL_REQUEST_SETUP_DATA_CALL,
                                     RIL_E_SUCCESS,
                                     &resp );
  resp.resp_pkt = (void *)&(setup_rsp);
  resp.resp_len = sizeof(setup_rsp);
  qcril_send_request_response( &resp );

#elif (RIL_VERSION >= 4)
   qcril_data_response_data_call_failure( instance_id,
                                          t,
                                          RIL_REQUEST_SETUP_DATA_CALL,
                                          PDP_FAIL_ERROR_UNSPECIFIED );
#else
   qcril_data_response_generic_failure( instance_id,
                                        t,
                                        RIL_REQUEST_SETUP_DATA_CALL );
#endif
} /* qcril_data_response_setup_data_call_failure_generic_err */


/*=========================================================================
  FUNCTION:  qcril_data_response_success

===========================================================================*/
/*!
    @brief
    Remove the entry from the ReqList. Send RIL_E_SUCCESS response.

    @return
    None
*/
/*=========================================================================*/
void qcril_data_response_success
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int request,
  void *response,
  size_t response_len
)
{
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s", "qcril_data_response_success: ENTRY" );

  /* Successfully remove the entry from ReqList */
  (void) qcril_reqlist_free( instance_id, t );

  /* Send SUCCESS as the response to the RIL command */
  qcril_default_request_resp_params( instance_id, t, request, RIL_E_SUCCESS, &resp );
  resp.resp_pkt = response;
  resp.resp_len = response_len;
  qcril_send_request_response( &resp );

  QCRIL_LOG_DEBUG( "%s", "qcril_data_response_success: EXIT" );
} /* qcril_data_response_success */

/*=========================================================================
  FUNCTION:  qcril_data_unsol_call_list_changed

===========================================================================*/
/*!
    @brief
    Remove the entry from the ReqList.

    @return
    None
*/
/*=========================================================================*/
static void qcril_data_unsol_call_list_changed
(
  qcril_instance_id_e_type instance_id
)
{
  qcril_unsol_resp_params_type unsol_resp;
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))

#if (RIL_VERSION >= 11)

  RIL_Data_Call_Response_v11 *active_call_table = NULL;

#elif (RIL_VERSION >= 10)

  RIL_Data_Call_Response_v9 *active_call_table = NULL;

#else

  RIL_Data_Call_Response_v6 *active_call_table = NULL;

#endif /* (RIL_VERSION >= 11)  */

#else
  RIL_Data_Call_Response    *active_call_table = NULL;
#endif /* (RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6) */

  size_t response_len;
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s", "qcril_data_unsol_call_list_changed: ENTRY" );

  QCRIL_LOG_DEBUG( "%s", "sending RIL_UNSOL_DATA_CALL_LIST_CHANGED" );
  qcril_data_get_active_call_list((void**)&active_call_table, &response_len);

  qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_DATA_CALL_LIST_CHANGED, &unsol_resp );
  unsol_resp.resp_pkt = active_call_table;
  unsol_resp.resp_len = response_len;
  qcril_send_unsol_response( &unsol_resp );

  /* free memeory */
  if(NULL != active_call_table)
  {
    QCRIL_LOG_DEBUG("%s", "qcril_data_unsol_call_list_changed: free memory");
    qcril_free(active_call_table);
    active_call_table = NULL;
  }
  QCRIL_LOG_DEBUG( "%s", "qcril_data_unsol_call_list_changed: EXIT with succ" );
  return;

}

/*=========================================================================
  FUNCTION:  qcril_data_unsol_tethered_mode_state_changed

===========================================================================*/
/*!
    @brief
    send RIL_UNSOL_RESPONSE_TETHERED_MODE_STATE_CHANGED to QCRIL

    @return
    None
*/
/*=========================================================================*/
static void qcril_data_unsol_tethered_mode_state_changed
(
  qcril_instance_id_e_type instance_id,
  void                    *response,
  size_t                   response_len
)
{
  qcril_unsol_resp_params_type unsol_resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s", "qcril_data_unsol_tethered_mode_state_changed: ENTRY" );

  QCRIL_LOG_DEBUG( "%s", "sending RIL_UNSOL_RESPONSE_TETHERED_MODE_STATE_CHANGED" );

  qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_TETHERED_MODE_STATE_CHANGED, &unsol_resp );
  unsol_resp.resp_pkt = ( void * ) response;
  unsol_resp.resp_len = response_len;
  qcril_send_unsol_response( &unsol_resp );

  QCRIL_LOG_DEBUG( "%s", "qcril_data_unsol_tethered_mode_state_changed: EXIT with succ" );
  return;

}

/*=========================================================================
  FUNCTION:  qcril_data_util_buffer_dump

===========================================================================*/
/*!
    @brief

    @return
    NONE
*/
/*=========================================================================*/
static void qcril_data_util_buffer_dump(
  const dsi_call_param_value_t    *const info
)
{
  int i = 0;
  const dsi_call_param_value_t * tmp = (dsi_call_param_value_t *) info;

  if ( tmp == NULL )
  {
    QCRIL_LOG_ERROR( "%s", "cannot dump null ptr" );
    goto err_label;
  }

  QCRIL_LOG_DEBUG( "%s", ">>>start buffer dump>>>" );

  for( i = 0; i < tmp->num_val; i++ )
  {
    if ( isalpha( ( (char *)tmp->buf_val )[ i ]) )
    {
      QCRIL_LOG_DEBUG( "buffer[%d] = %c", i, ((char *)tmp->buf_val)[ i ] );
    }
    else
    {
      QCRIL_LOG_DEBUG( "buffer[%d] = %p", i, ((unsigned int *)tmp->buf_val)[ i ] );
    }
  }

  QCRIL_LOG_DEBUG( "%s", "<<<end buffer dump<<<" );

err_label:
  return;
}/* qcril_data_buffer_dump() */


/*=========================================================================
  FUNCTION:  qcril_data_util_is_pending

===========================================================================*/
/*!
    @brief

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    NONE
*/
/*=========================================================================*/
int qcril_data_util_is_req_pending
(
  const qcril_data_call_info_tbl_type *info_tbl_ptr,
  unsigned int *pend_req
)
{
  int ret = FALSE;

  QCRIL_DS_ASSERT_H( info_tbl_ptr != NULL, "validate input ptr");
  QCRIL_DS_ASSERT_H( pend_req     != NULL, "validate ret ptr" );

  if (info_tbl_ptr == NULL || pend_req == NULL)
  {
    return ret;
  }

  if ( ( info_tbl_ptr->pend_tok == NULL ) &&
       ( info_tbl_ptr->pend_req == DS_RIL_REQ_INVALID ) )
  {
    *pend_req = DS_RIL_REQ_INVALID;
    goto err_label;
  }
  else if ( ( info_tbl_ptr->pend_tok != NULL ) &&
            ( info_tbl_ptr->pend_req != DS_RIL_REQ_INVALID ) )
  {
    *pend_req = info_tbl_ptr->pend_req;
    ret = TRUE;
  }
  else
  {
    QCRIL_DS_ASSERT_H( 0, "bad state, pend_tok and pend_req out of sync" );
    goto err_label;
  }

err_label:
  return ret;
}/* qcril_data_util_is_req_pending() */


/*=========================================================================
  FUNCTION:  qcril_data_util_fill_bearer_tech

===========================================================================*/
/*!
    @brief
    converts qmi_wds_data_bearer_tech_type into a valid
    RIL_RadioTechnology and stores it in the info_tbl_ptr

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    NONE
*/
/*=========================================================================*/
static void qcril_data_util_fill_bearer_tech
(
  qcril_data_call_info_tbl_type * info_tbl_ptr,
  qmi_wds_data_bearer_tech_type * db_tech_type
)
{
  if (NULL == info_tbl_ptr ||
      NULL == db_tech_type)
  {
    QCRIL_LOG_ERROR("%s","invalid parameters received");
    return;
  }

  switch(db_tech_type->current_db_nw)
  {
  case QMI_WDS_CDMA_TYPE:

    if (db_tech_type->rat_mask.cdma_rat_mask & CDMA_1X)
    {
      QCRIL_LOG_DEBUG("call [%d] bearer is [%s]",
                      info_tbl_ptr->index, "CDMA_1X");
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_1xRTT;
      break;
    }

    if (db_tech_type->rat_mask.cdma_rat_mask & CDMA_EVDO_REV0)
    {
      QCRIL_LOG_DEBUG("call [%d] bearer is [%s]",
                      info_tbl_ptr->index, "CDMA_EVDO_REV0");
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_EVDO_0;
      break;
    }

    if (db_tech_type->rat_mask.cdma_rat_mask & CDMA_EVDO_REVA)
    {
      QCRIL_LOG_DEBUG("call [%d] bearer is [%s]",
                      info_tbl_ptr->index, "CDMA_EVDO_REVA");
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_EVDO_A;
      break;
    }

    /* 0x08 and 0x20 are EHRPD enumerations in QMI */
    if (db_tech_type->db_so_mask.so_mask_evdo_reva & CDMA_EVDO_REVA_EMPA_EHRPD ||
        db_tech_type->db_so_mask.so_mask_evdo_revb & CDMA_EVDO_REVB_EMPA_EHRPD ||
        db_tech_type->db_so_mask.so_mask_evdo_revb & CDMA_EVDO_REVB_MMPA_EHRPD )
    {
      QCRIL_LOG_DEBUG("call [%d] bearer is [%s]",
                      info_tbl_ptr->index, "EHRPD");
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_EHRPD;
      break;
    }

    /* we reach here, that means we have not found
     any of the radio technologies we care about */
    QCRIL_LOG_ERROR("QMI returned unknown CDMA rat mask [%d]",
                    db_tech_type->rat_mask);
    break;

  case QMI_WDS_UMTS_TYPE:

    if (db_tech_type->rat_mask.umts_rat_mask & UMTS_WCDMA)
    {
      QCRIL_LOG_DEBUG("call [%d] bearer is [%s]",
                      info_tbl_ptr->index, "WCDMA");
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_UMTS;
      break;
    }

    if (db_tech_type->rat_mask.umts_rat_mask & UMTS_GPRS)
    {
      QCRIL_LOG_DEBUG("call [%d] bearer is [%s]",
                      info_tbl_ptr->index, "GPRS");
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_GPRS;
      break;
    }

    if (db_tech_type->rat_mask.umts_rat_mask & UMTS_HSDPA)
    {
      QCRIL_LOG_DEBUG("call [%d] bearer is [%s]",
                      info_tbl_ptr->index, "HSDPA");
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_HSDPA;
      break;
    }

    if (db_tech_type->rat_mask.umts_rat_mask & UMTS_HSUPA)
    {
      QCRIL_LOG_DEBUG("call [%d] bearer is [%s]",
                      info_tbl_ptr->index, "HSUPA");
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_HSUPA;
      break;
    }

    if (db_tech_type->rat_mask.umts_rat_mask & UMTS_EDGE)
    {
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_EDGE;
      break;
    }

    if (db_tech_type->rat_mask.umts_rat_mask & UMTS_LTE)
    {
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_LTE;
      break;
    }

#if (RIL_QCOM_VERSION >= 3)
    if (db_tech_type->rat_mask.umts_rat_mask & UMTS_TDSCDMA)
    {
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_TD_SCDMA;
      break;
    }
#endif /* (RIL_QCOM_VERSION >= 3) */

/*
    switch(db_tech_type.rat_mask)
    {
    case UMTS_WCDMA:
      break;
    case UMTS_GPRS:
      break;
    case UMTS_HSDPA:
      break;
    case UMTS_HSUPA:
      break;
    case UMTS_EDGE:
      break;
    default:
      break;
    }
*/
    /* we reach here, that means we have not found
     any of the radio technologies we care about */
    QCRIL_LOG_ERROR("QMI returned unknown UMTS rat mask [%d]",
                    db_tech_type->rat_mask);
    break;
  default:
    QCRIL_LOG_ERROR("%s","QMI returned unknown bearer technology");
    break;
  }
}

/*=========================================================================
  FUNCTION:  qcril_data_util_fill_bearer_tech_ex

===========================================================================*/
/*!
    @brief
    converts qmi_wds_data_bearer_tech_type into a valid
    RIL_RadioTechnology and stores it in the info_tbl_ptr

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    NONE
*/
/*=========================================================================*/
static void qcril_data_util_fill_bearer_tech_ex
(
  qcril_data_call_info_tbl_type * info_tbl_ptr,
  qmi_wds_data_bearer_tech_type_ex * db_tech_type_ex
)
{
  if (NULL == info_tbl_ptr ||
      NULL == db_tech_type_ex)
  {
    QCRIL_LOG_ERROR("%s","invalid parameters received");
    return;
  }

  if (QMI_WDS_TECHNOLOGY_3GPP2 == db_tech_type_ex->technology)
  {
    if (QMI_WDS_BEARER_TECH_RAT_EX_3GPP2_1X == db_tech_type_ex->rat_mask)
    {
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_1xRTT;
    }
    else if (QMI_WDS_BEARER_TECH_RAT_EX_3GPP2_HRPD == db_tech_type_ex->rat_mask)
    {
      if (QMI_WDS_3GPP2_SO_MASK_HDR_REV0_DPA & db_tech_type_ex->so_mask)
      {
        info_tbl_ptr->call_info.radioTech = RADIO_TECH_EVDO_0;
      }
      else if ((QMI_WDS_3GPP2_SO_MASK_HDR_REVA_DPA & db_tech_type_ex->so_mask) ||
               (QMI_WDS_3GPP2_SO_MASK_HDR_REVA_MPA & db_tech_type_ex->so_mask) ||
               (QMI_WDS_3GPP2_SO_MASK_HDR_REVA_EMPA & db_tech_type_ex->so_mask))
      {
        info_tbl_ptr->call_info.radioTech = RADIO_TECH_EVDO_A;
      }
      else if ((QMI_WDS_3GPP2_SO_MASK_HDR_REVB_DPA & db_tech_type_ex->so_mask) ||
               (QMI_WDS_3GPP2_SO_MASK_HDR_REVB_MPA & db_tech_type_ex->so_mask) ||
               (QMI_WDS_3GPP2_SO_MASK_HDR_REVB_EMPA & db_tech_type_ex->so_mask)||
               (QMI_WDS_3GPP2_SO_MASK_HDR_REVB_MMPA & db_tech_type_ex->so_mask))

      {
        info_tbl_ptr->call_info.radioTech = RADIO_TECH_EVDO_B;
      }
    }
    else if (QMI_WDS_BEARER_TECH_RAT_EX_3GPP2_EHRPD == db_tech_type_ex->rat_mask)
    {
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_EHRPD;
    }
    else if (QMI_WDS_BEARER_TECH_RAT_EX_3GPP2_WLAN == db_tech_type_ex->rat_mask)
    {
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_IWLAN;
    }
    else
    {
      /* we reach here, that means we have not found
         any of the radio technologies we care about */
      QCRIL_LOG_ERROR("QMI returned unknown 3GPP2 rat mask [%d]",
                    db_tech_type_ex->rat_mask);
    }
  }
  else if (QMI_WDS_TECHNOLOGY_3GPP == db_tech_type_ex->technology)
  {
    if (QMI_WDS_BEARER_TECH_RAT_EX_3GPP_WCDMA == db_tech_type_ex->rat_mask)
    {
      if((QMI_WDS_3GPP_SO_MASK_HSDPAPLUS & db_tech_type_ex->so_mask) ||
         (QMI_WDS_3GPP_SO_MASK_DC_HSDPAPLUS & db_tech_type_ex->so_mask) ||
         (QMI_WDS_3GPP_SO_MASK_64_QAM & db_tech_type_ex->so_mask))
      {
        info_tbl_ptr->call_info.radioTech = RADIO_TECH_HSPAP;
      }
      else if ((QMI_WDS_3GPP_SO_MASK_HSPA & db_tech_type_ex->so_mask) ||
               ((QMI_WDS_3GPP_SO_MASK_HSUPA & db_tech_type_ex->so_mask) &&
                (QMI_WDS_3GPP_SO_MASK_HSDPA & db_tech_type_ex->so_mask)))
      {
        info_tbl_ptr->call_info.radioTech = RADIO_TECH_HSPA;
      }
      else if (QMI_WDS_3GPP_SO_MASK_HSUPA & db_tech_type_ex->so_mask)
      {
        info_tbl_ptr->call_info.radioTech = RADIO_TECH_HSUPA;
      }
      else if (QMI_WDS_3GPP_SO_MASK_HSDPA & db_tech_type_ex->so_mask)
      {
        info_tbl_ptr->call_info.radioTech = RADIO_TECH_HSDPA;
      }
      else
      {
        info_tbl_ptr->call_info.radioTech = RADIO_TECH_UMTS;
      }
    }
    else if (QMI_WDS_BEARER_TECH_RAT_EX_3GPP_GERAN == db_tech_type_ex->rat_mask)
    {
      if (QMI_WDS_3GPP_SO_MASK_EDGE & db_tech_type_ex->so_mask)
      {
        info_tbl_ptr->call_info.radioTech = RADIO_TECH_EDGE;
      }
      else if (QMI_WDS_3GPP_SO_MASK_GPRS & db_tech_type_ex->so_mask)
      {
        info_tbl_ptr->call_info.radioTech = RADIO_TECH_GPRS;
      }
      else if (QMI_WDS_3GPP_SO_MASK_GSM & db_tech_type_ex->so_mask)
      {
        info_tbl_ptr->call_info.radioTech = RADIO_TECH_GSM;
      }
    }
    else if (QMI_WDS_BEARER_TECH_RAT_EX_3GPP_LTE == db_tech_type_ex->rat_mask)
    {
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_LTE;
    }
    else if (QMI_WDS_BEARER_TECH_RAT_EX_3GPP_TDSCDMA == db_tech_type_ex->rat_mask)
    {
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_TD_SCDMA;
    }
    else if (QMI_WDS_BEARER_TECH_RAT_EX_3GPP_WLAN == db_tech_type_ex->rat_mask &&
             QMI_WDS_3GPP_SO_MASK_S2B == db_tech_type_ex->so_mask)
    {
      info_tbl_ptr->call_info.radioTech = RADIO_TECH_IWLAN;
    }
    else
    {
      /* we reach here, that means we have not found
       any of the radio technologies we care about */
      QCRIL_LOG_ERROR("QMI returned unknown 3GPP rat mask [%d]",
                      db_tech_type_ex->rat_mask);
    }
  }
  else
  {
    QCRIL_LOG_ERROR("QMI returned unknown bearer technology=[%d]",
                    db_tech_type_ex->technology);
  }
}

/*=========================================================================
  FUNCTION:  qcril_data_util_fill_call_type

===========================================================================*/
/*!
    @brief
    Stores the call type in the info_tbl_ptr

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    NONE
*/
/*=========================================================================*/
static void qcril_data_util_fill_call_type
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr
)
{
  if (NULL == info_tbl_ptr)
  {
    QCRIL_LOG_ERROR("%s","invalid parameters received");
    return;
  }

  switch (info_tbl_ptr->dsi_ip_version)
  {
  case DSI_IP_VERSION_4:
      strlcpy(info_tbl_ptr->call_info.type,
              QCRIL_DATA_IP_FAMILY_V4,
              sizeof(info_tbl_ptr->call_info.type));
      break;

    case DSI_IP_VERSION_6:
      strlcpy(info_tbl_ptr->call_info.type,
              QCRIL_DATA_IP_FAMILY_V6,
              sizeof(info_tbl_ptr->call_info.type));
      break;

    case DSI_IP_VERSION_4_6:
      strlcpy(info_tbl_ptr->call_info.type,
              QCRIL_DATA_IP_FAMILY_V4V6,
              sizeof(info_tbl_ptr->call_info.type));
      break;

    default:
      QCRIL_LOG_ERROR("Invalid IP version=%d", info_tbl_ptr->dsi_ip_version);
      break;
  }
}

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))

/*=========================================================================
  FUNCTION:  qcril_data_util_format_sockaddr

===========================================================================*/
/*!
    @brief
    This function formats the given sockaddr to a string and prepends a space
    delimiter (if necessary)

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @param
    addr               [in]    - sockaddr to convert to string
    prefix_len         [in]    - prefix length
    prefix_len_valid   [in]    - if given prefix_len is valid or not
    addr_str           [out]   - storage for the formatted address string
    avail_len          [inout] - size of available storage in addr_str before and
                                 after storing the formatted address string
    preprend_delimiter [in]    - if a delimiter should be prepended to the address
                                 string

    @return
    None
*/
/*=========================================================================*/
static void qcril_data_util_format_sockaddr
(
  struct sockaddr_storage  *addr,
  unsigned int             prefix_len,
  boolean                  prefix_len_valid,
  char                     *addr_str,
  int                      addr_str_size,
  boolean                  prepend_delimiter
)
{
  char fmtstr[DS_CALL_INFO_ADDR_IPV6_MAX_LEN + 1] = "";

  if (!addr || !addr_str || addr_str_size <= 0)
  {
    QCRIL_LOG_ERROR("%s", "qcril_data_util_format_sockaddr: bad param(s)");
    return;
  }

  QCRIL_LOG_DEBUG("prefix_len=%u, prefix_len_valid=%d, prepend_delimiter=%d",
                  prefix_len, prefix_len_valid, prepend_delimiter);

  if (prefix_len_valid)
  {
    DS_CALL_ADDR_FORMAT_IPV4V6_WITH_PREFIX_LEN( addr,
                                                prefix_len,
                                                fmtstr,
                                                sizeof(fmtstr) );
  }
  else
  {
    DS_CALL_ADDR_FORMAT_IPV4V6( addr,
                                fmtstr,
                                sizeof(fmtstr) );
  }

  QCRIL_LOG_DEBUG("formatted address string=%s", fmtstr);

  strlcat(addr_str,
          prepend_delimiter ? QCRIL_DATA_ADDR_DELIMITER : "",
          addr_str_size);

  strlcat(addr_str,
          fmtstr,
          addr_str_size);
}


/*=========================================================================
  FUNCTION:  qcril_data_util_dsi_addr_info_to_str

===========================================================================*/
/*!
    @brief
    Converts the IP, gateway, DNS addresses in the given array of addr_info
    structures into a space delimited string and stores the corresponding
    value in the given call_info table entry.

    @param
    addr_info [in]     - array of dsi_addr_info_t structures to be converted
    num_addr_info [in] - number of elements of the addr_info array
    call_info [out]    - storage for the converted IP, Gateway and DNS address
                         strings

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    SUCCESS - If conversion to string was successful
    FAILURE - Otherwise
*/
/*=========================================================================*/
static int qcril_data_util_dsi_addr_info_to_str
(
  dsi_addr_info_t *addr_info,
  int num_addr_info,
  qcril_data_call_info_t *call_info
)
{
  int i;
  int ret = FAILURE;
  int address_avail_len;
  int gateway_avail_len;
  int dns_avail_len;

  if (!addr_info || num_addr_info <= 0 || !call_info)
  {
    QCRIL_LOG_ERROR("qcril_data_util_dsi_addr_to_str: bad param(s) "
                    "addr_info=0x%x, num_addr_info=%d, call_info=0x%x",
                    addr_info, num_addr_info, call_info);
    return FAILURE;
  }

  address_avail_len = sizeof(call_info->address.fmtstr);
  gateway_avail_len = sizeof(call_info->gateway.fmtstr);
  dns_avail_len     = sizeof(call_info->dns.fmtstr);

  memset(call_info->address.fmtstr, 0x0, address_avail_len);
  memset(call_info->gateway.fmtstr, 0x0, gateway_avail_len);
  memset(call_info->dns.fmtstr, 0x0, dns_avail_len);

  for (i = 0; i < num_addr_info; ++i)
  {
    /* Copy iface address if valid */
    if ( addr_info[i].iface_addr_s.valid_addr )
    {
      qcril_data_util_format_sockaddr(&addr_info[i].iface_addr_s.addr,
                                      addr_info[i].iface_mask,
                                      TRUE,
                                      call_info->address.fmtstr,
                                      address_avail_len,
                                      (i > 0) ? TRUE : FALSE);
    }
    else
    {
      QCRIL_LOG_ERROR("qcril_data_util_dsi_addr_to_str: iface address invalid [%d]",
                      i);
      goto bail;
    }

    /* Copy gateway address if valid */
    if ( addr_info[i].gtwy_addr_s.valid_addr )
    {
      qcril_data_util_format_sockaddr(&addr_info[i].gtwy_addr_s.addr,
                                      addr_info[i].gtwy_mask,
                                      FALSE,
                                      call_info->gateway.fmtstr,
                                      gateway_avail_len,
                                      (i > 0) ? TRUE : FALSE);
    }
    else
    {
      QCRIL_LOG_ERROR("qcril_data_util_dsi_addr_to_str: gateway address invalid [%d]",
                      i);
      goto bail;
    }

    /* Copy DNS primary address if valid */
    if ( addr_info[i].dnsp_addr_s.valid_addr )
    {
      qcril_data_util_format_sockaddr(&addr_info[i].dnsp_addr_s.addr,
                                      0,
                                      FALSE,
                                      call_info->dns.fmtstr,
                                      dns_avail_len,
                                      (i > 0) ? TRUE : FALSE);

      /* Copy DNS secondary address if valid */
      if ( addr_info[i].dnss_addr_s.valid_addr )

      {
        /* Always prepend a delimiter between primary and secondary DNS addresses */
        qcril_data_util_format_sockaddr(&addr_info[i].dnss_addr_s.addr,
                                        0,
                                        FALSE,
                                        call_info->dns.fmtstr,
                                        dns_avail_len,
                                        TRUE);
      }
    }
  }

  ret = SUCCESS;

bail:
  return ret;
}
#endif /* ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6)) */

/*===========================================================================*/
/*!
    @brief
    Update the MTU in the call_info table

    @return
    None
*/
/*=========================================================================*/
void qcril_data_util_update_mtu
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr
)
{
#if (RIL_VERSION >= 11)
  unsigned int mtu;

  if (NULL == info_tbl_ptr)
  {
    QCRIL_DS_ASSERT(0, "programming error: NULL info_tbl_ptr passed");
    return;
  }

  /* Update iface MTU */
  if (qcril_data_mtu > 0)
  {
    info_tbl_ptr->call_info.mtu = qcril_data_mtu;
  }
  else if (DSI_SUCCESS != dsi_get_link_mtu(info_tbl_ptr->dsi_hndl, &mtu))
  {
    QCRIL_LOG_ERROR( "%s", "failed to get link MTU" );
  }
  else
  {
    info_tbl_ptr->call_info.mtu = mtu;
  }

  QCRIL_LOG_INFO( "MTU set to %d", info_tbl_ptr->call_info.mtu );
#else
  (void) info_tbl_ptr;
#endif

  return;
}

/*=========================================================================
  FUNCTION:  qcril_data_util_fill_call_params

===========================================================================*/
/*!
    @brief
    Uses QMI WDS service to get current call params and fill into
    the call table

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    NONE
*/
/*=========================================================================*/
static void qcril_data_util_fill_call_params
(
  qcril_data_call_info_tbl_type *info_tbl_ptr
)
{
  qmi_wds_req_runtime_settings_params_type req_param_mask = 0;
  qcril_data_call_curr_info_type call_curr_info;
  qmi_wds_data_bearer_tech_type_ex bearer_tech_ex_rsp;
  qmi_wds_data_bearer_tech_type bearer_tech_rsp;
  int qmi_err_code;
  int rc;
  int ret = FAILURE;
  int reti = SUCCESS;
  char buf[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1];
  struct sockaddr_storage sockaddr;
  dsi_addr_info_t *addr_info = NULL;
  unsigned int count;

  QCRIL_LOG_DEBUG("%s","qcril_data_util_fill_call_params: ENTRY");

  memset(&bearer_tech_ex_rsp, 0, sizeof(bearer_tech_ex_rsp));
  memset(&bearer_tech_rsp, 0, sizeof(bearer_tech_rsp));

  ret = FAILURE;
  do
  {
    if (NULL == info_tbl_ptr)
    {
      QCRIL_DS_ASSERT(0, "programming error: NULL info_tbl_ptr passed");
      break;
    }

    /***** Fill CID *******/
    snprintf( info_tbl_ptr->call_info.cid, 4, "%d", info_tbl_ptr->cid);

    /****** Fill the call type ******/
    qcril_data_util_fill_call_type(info_tbl_ptr);

    /* Fill device name */
    QCRIL_LOG_DEBUG("Get device name and fill the value at [%p]",
                    buf);
    rc = dsi_get_device_name(info_tbl_ptr->dsi_hndl,
                             buf,
                             DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);
    if (DSI_SUCCESS != rc)
    {
      QCRIL_LOG_DEBUG("couldn't get device name for info_tbl [%d]",
                      info_tbl_ptr->index);
      QCRIL_DS_ASSERT( 0, "programming error: dsi_get_device_name" );
      break;
    }
    /* Copy according to presentation layer, limit size to IFNAMSIZ  */
    memcpy( info_tbl_ptr->call_info.dev_name, buf, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN );
    info_tbl_ptr->call_info.dev_name[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN] = '\0';
    QCRIL_LOG_DEBUG( "dev_name [%s]", info_tbl_ptr->call_info.dev_name);
    /* this info_tbl_ptr now contains a valid device name */
    info_tbl_ptr->info_flg = TRUE;

    /******* Fill ip address ******/
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
    count = dsi_get_ip_addr_count(info_tbl_ptr->dsi_hndl);
#else
    count = 1;
#endif

    info_tbl_ptr->last_addr_count = count;

    if (0 == count)
    {
      QCRIL_LOG_DEBUG("couldn't get valid count of addr info for info_tbl [%d]",
                      info_tbl_ptr->index);
      break;
    }

    addr_info = calloc(count, sizeof(dsi_addr_info_t));

    if (NULL == addr_info)
    {
      QCRIL_LOG_ERROR("couldn't allocate addr info for info_tbl [%d] count [%u]",
                      info_tbl_ptr->index, count);
      break;
    }

    rc = dsi_get_ip_addr(info_tbl_ptr->dsi_hndl, addr_info, count);
    if (DSI_SUCCESS != rc)
    {
      QCRIL_LOG_ERROR("couldn't get addr info for info_tbl [%d] err [%d]",
                      info_tbl_ptr->index, rc);
      break;
    }

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
    /* Convert addr_info[] to string */
    rc = qcril_data_util_dsi_addr_info_to_str(addr_info, count, &info_tbl_ptr->call_info);
    if (SUCCESS != rc)
    {
      QCRIL_LOG_ERROR("couldn't convert addr info to str info_tbl [%d] err [%d]",
                      info_tbl_ptr->index, rc);
      break;
    }

    QCRIL_LOG_DEBUG("IP Addresses     : %s", info_tbl_ptr->call_info.address.fmtstr);
    QCRIL_LOG_DEBUG("Gateway Addresses: %s", info_tbl_ptr->call_info.gateway.fmtstr);
    QCRIL_LOG_DEBUG("DNS Addresses    : %s", info_tbl_ptr->call_info.dns.fmtstr);
    QCRIL_LOG_DEBUG("Call Type        : %s", info_tbl_ptr->call_info.type);

#if (RIL_VERSION >= 10)

    memset(info_tbl_ptr->call_info.pcscf.fmtstr, 0x0,
           sizeof(info_tbl_ptr->call_info.pcscf.fmtstr));

#endif

#if (RIL_VERSION >= 11)
    qcril_data_util_update_mtu(info_tbl_ptr);

#endif

#else
    /* prepare the network ordered ip addr for presentation */
    reti = SUCCESS;
    switch( SASTORAGE_FAMILY(addr_info->iface_addr_s.addr) )
    {
    case AF_INET:
      QCRIL_LOG_DEBUG("IPv4 address found. data buf is [%d] long",
                      sizeof( SASTORAGE_DATA(addr_info->iface_addr_s.addr) ));
      snprintf(info_tbl_ptr->call_info.address,
               sizeof(info_tbl_ptr->call_info.address),
               "%d.%d.%d.%d/%d%c",
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[0],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[1],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[2],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[3],
               addr_info.iface_mask, '\0');
      QCRIL_LOG_DEBUG("IPv4 address [%s] populated in info_tbl[%d]",
                      info_tbl_ptr->call_info.address,
                      info_tbl_ptr->index);
      break;
    case AF_INET6:
      QCRIL_LOG_DEBUG("IPv6 address found. data buf is [%d] long",
                      sizeof(SASTORAGE_DATA(addr_info->iface_addr_s.addr)));
      snprintf(info_tbl_ptr->call_info.address,
               sizeof(info_tbl_ptr->call_info.address),
               "%.2x%.2x:%.2x%.2x:%.2x%.2x:%.2x%.2x:"
               "%.2x%.2x:%.2x%.2x:%.2x%.2x:%.2x%.2x/%d%c",
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[0],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[1],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[2],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[3],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[4],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[5],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[6],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[7],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[8],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[9],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[10],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[11],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[12],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[13],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[14],
               SASTORAGE_DATA(addr_info.iface_addr_s.addr)[15],
               addr_info.iface_mask, '\0');
      QCRIL_LOG_DEBUG("IPv6 address [%s] populated in info_tbl[%d]",
                      info_tbl_ptr->call_info.address,
                      info_tbl_ptr->index);
      break;
    default:
      QCRIL_LOG_ERROR("invalid/unsupported ss_family [%d]",
                      SASTORAGE_FAMILY(addr_info->iface_addr_s.addr));
      sprintf( info_tbl_ptr->call_info.address, "%s", "unknown\0" );
      reti = FAILURE;
      break;
    }
    if (SUCCESS != reti)
    {
      break;
    }
#endif /* ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6)) */

    /****** fill in the current bearer tech ******/

    rc = qmi_wds_get_current_bearer_tech_ex(info_tbl_ptr->qmi_wds_hndl,
                                            &bearer_tech_ex_rsp,
                                            &qmi_err_code);
    if (rc != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("qmi_wds_get_current_bearer_tech_ex failed with"
                      "return value [%d], qmi_err_code [%d]",
                      rc, qmi_err_code);

      QCRIL_LOG_DEBUG("%s", "Fallback on qmi_wds_get_current_bearer_tech");

      rc = qmi_wds_get_current_bearer_tech(info_tbl_ptr->qmi_wds_hndl,
                                           &bearer_tech_rsp,
                                           &qmi_err_code);
      ret = SUCCESS;
      if (rc != QMI_NO_ERR)
      {
        QCRIL_LOG_ERROR("qmi_wds_get_current_bearer_tech failed with"
                        "return value [%d], qmi_err_code [%d]",
                        rc, qmi_err_code);
        break;
      }

      qcril_data_util_fill_bearer_tech(info_tbl_ptr,
                                       &bearer_tech_rsp);
    }
    else
    {
      qcril_data_util_fill_bearer_tech_ex(info_tbl_ptr,
                                          &bearer_tech_ex_rsp);
      ret = SUCCESS;
    }

    ret = SUCCESS;
  } while(0);

  free(addr_info);

  if (SUCCESS == ret)
  {
    QCRIL_LOG_DEBUG("%s","qcril_data_util_fill_call_params: EXIT success");
  }
  else
  {
    QCRIL_LOG_DEBUG("%s","qcril_data_util_fill_call_params: EXIT error");
  }
}/* qcril_data_util_fill_call_params() */

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
/*=========================================================================
  FUNCTION:  qcril_data_util_create_setup_rsp

===========================================================================*/
/*!
    @brief
    Create RIL REQ SETUP response.

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    NONE
*/
/*=========================================================================*/
static void qcril_data_util_create_setup_rsp
(
  qcril_data_call_info_tbl_type *info_tbl_ptr,
  qcril_data_call_response_type *rsp_ptr
)
{
  /* validate input parameter */
  if(!info_tbl_ptr || !rsp_ptr)
  {
    QCRIL_LOG_ERROR("qcril_data_util_create_setup_rsp: bad param(s) "
                    "info_tbl_ptr=0x%x, rsp_ptr=0x%x",
                    info_tbl_ptr, rsp_ptr);
    return;
  }

  /* Transfer the call state */
  rsp_ptr->setup_rsp.status = info_tbl_ptr->status;
  rsp_ptr->setup_rsp.suggestedRetryTime = info_tbl_ptr->suggestedRetryTime;
  rsp_ptr->setup_rsp.cid = info_tbl_ptr->cid;
  rsp_ptr->setup_rsp.active = info_tbl_ptr->call_info.active;
  rsp_ptr->setup_rsp.type = info_tbl_ptr->call_info.type;
  rsp_ptr->setup_rsp.ifname = info_tbl_ptr->call_info.dev_name;
  rsp_ptr->setup_rsp.addresses = info_tbl_ptr->call_info.address.fmtstr;
  rsp_ptr->setup_rsp.dnses = info_tbl_ptr->call_info.dns.fmtstr;
  rsp_ptr->setup_rsp.gateways = info_tbl_ptr->call_info.gateway.fmtstr;

#if (RIL_VERSION >= 10)

  rsp_ptr->setup_rsp.pcscf = info_tbl_ptr->call_info.pcscf.fmtstr;

#endif

#if (RIL_VERSION >= 11)

  rsp_ptr->setup_rsp.mtu = info_tbl_ptr->call_info.mtu;

#endif

  rsp_ptr->size               = sizeof(rsp_ptr->setup_rsp);

}/* qcril_data_util_create_setup_rsp() */

#else
/*=========================================================================
  FUNCTION:  qcril_data_util_create_setup_rsp

===========================================================================*/
/*!
    @brief
    Create RIL REQ SETUP response.

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    NONE
*/
/*=========================================================================*/
static void qcril_data_util_create_setup_rsp
(
  qcril_data_call_info_tbl_type *info_tbl_ptr,
  qcril_data_call_response_type *rsp_ptr
)
{
  /* validate input parameter */
  if(!info_tbl_ptr || !rsp_ptr)
  {
    QCRIL_LOG_ERROR("qcril_data_util_create_setup_rsp: bad param(s) "
                    "info_tbl_ptr=0x%x, rsp_ptr=0x%x",
                    info_tbl_ptr, rsp_ptr);
    return;
  }

  rsp_ptr->setup_rsp.cid      = info_tbl_ptr->call_info.cid;
  rsp_ptr->setup_rsp.dev_name = info_tbl_ptr->call_info.dev_name;
  rsp_ptr->setup_rsp.ip_addr  = info_tbl_ptr->call_info.address;
  rsp_ptr->size               = sizeof rsp_ptr->setup_rsp ;
}/* qcril_data_util_create_setup_rsp() */
#endif /* ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6)) */


/*=========================================================================
  FUNCTION:  qcril_data_get_modem_port

===========================================================================*/
/*!
    @brief
    gets modem specific default qmi port

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static inline int qcril_data_get_modem_port
(
  int modem_id,
  char * qmi_port
)
{
  unsigned int i=0;


  for(i=0; i<sizeof(qcril_data_modem_port_map_tbl)/sizeof(qcril_data_modem_port_map);
      i++)
  {
    if (qcril_data_modem_port_map_tbl[i].modem_id ==
        modem_id)
      qmi_port = qcril_data_modem_port_map_tbl[i].qmi_port;
    return SUCCESS;
  }
  return FAILURE;
}

/*===========================================================================

  FUNCTION:  qcril_data_post_dsi_netctrl_event

===========================================================================*/
/*!
    @brief
    This routine calls qcril_event_queue to let QCRIL call the appropriate
    event/cmd handler for event/cmd.
    Note: do not assume the context in which handler will be called

    @return
    None.
*/
/*=========================================================================*/
static void qcril_data_post_dsi_netctrl_event
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type    modem_id,
  RIL_Token                pend_tok,
  qcril_data_call_info_tbl_type *info_tbl_ptr,
  dsi_net_evt_t            net_evt,
  dsi_evt_payload_t       *payload_ptr
)
{
  qcril_reqlist_public_type      reqlist_info;
  qcril_data_event_data_t       *evt;

  QCRIL_DS_LOG_DBG_MEM( "info_tbl", info_tbl_ptr );
  QCRIL_LOG_DEBUG( "dsi net evt [%d], info_tbl index [%d], pend RIL Token [%d]",
                   net_evt, info_tbl_ptr->index,
                   qcril_log_get_token_id( info_tbl_ptr->pend_tok ) );

  /* Allocate from heap here and clean-up on call end */
  evt = ( qcril_data_event_data_t *)malloc( sizeof( qcril_data_event_data_t ) );
  if ( evt == NULL )
  {
    QCRIL_LOG_ERROR( "%s","unable to alloc mem for event obj" );
    return;
  }
  QCRIL_DS_LOG_DBG_MEM( "event obj alloc", evt );
  memset( evt, 0, sizeof( qcril_data_event_data_t ) );

  /* Populate data event obj */
  evt->evt      = net_evt;
  evt->data     = info_tbl_ptr;
  evt->data_len = sizeof( qcril_data_call_info_tbl_type );
  evt->self     = evt;

  if( NULL != payload_ptr )
  {
    evt->payload = *payload_ptr;
#ifdef FEATURE_DATA_EMBMS
    if((net_evt == DSI_NET_TMGI_DEACTIVATED)||
       (net_evt == DSI_NET_TMGI_ACTIVATED)||
       (net_evt == DSI_NET_TMGI_LIST_CHANGED)||
       (net_evt == DSI_NET_TMGI_ACTIVATED_DEACTIVATED))
    {
      if(NULL != payload_ptr->embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list)
      {
        qcril_data_utils_embms_copy_tmgi_list(
          &(evt->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list),
          payload_ptr->embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list,
          payload_ptr->embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list_len);
      }

      if(NULL != payload_ptr->embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list)
      {
        qcril_data_utils_embms_copy_tmgi_list(
          &(evt->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list),
          payload_ptr->embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list,
          payload_ptr->embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list_len);
      }
    }

    if(net_evt == DSI_NET_SAI_LIST_CHANGED)
    {
      if (NULL != payload_ptr->embms_sai_info.available_sai_list)
      {
        if (NULL == (evt->payload.embms_sai_info.available_sai_list =
            malloc(payload_ptr->embms_sai_info.available_sai_list_len * sizeof(unsigned int))))
        {
          QCRIL_LOG_ERROR("%s", "failed to allocate memory for available_sai_list\n");
          goto err_ret;
        }
        memcpy(evt->payload.embms_sai_info.available_sai_list,
               payload_ptr->embms_sai_info.available_sai_list,
               payload_ptr->embms_sai_info.available_sai_list_len * sizeof(unsigned int));
      }

      if (NULL != payload_ptr->embms_sai_info.camped_sai_list)
      {
        if (NULL == (evt->payload.embms_sai_info.camped_sai_list =
                     malloc(payload_ptr->embms_sai_info.camped_sai_list_len * sizeof(unsigned int))))
        {
          QCRIL_LOG_ERROR("%s", "failed to allocate memory for camped_sai_list\n");
          goto err_ret;
        }
        memcpy(evt->payload.embms_sai_info.camped_sai_list,
               payload_ptr->embms_sai_info.camped_sai_list,
               payload_ptr->embms_sai_info.camped_sai_list_len * sizeof(unsigned int));
      }

      if (NULL != payload_ptr->embms_sai_info.num_sai_per_group)
      {
        if (NULL == (evt->payload.embms_sai_info.num_sai_per_group =
                     malloc(payload_ptr->embms_sai_info.num_sai_per_group_len * sizeof(unsigned short))))
        {
          QCRIL_LOG_ERROR("%s", "failed to allocate memory for num_sai_per_group\n");
          goto err_ret;
        }
        memcpy(evt->payload.embms_sai_info.num_sai_per_group,
               payload_ptr->embms_sai_info.num_sai_per_group,
               payload_ptr->embms_sai_info.num_sai_per_group_len * sizeof(unsigned short));
      }
    }

    if(net_evt == DSI_NET_CONTENT_DESC_CONTROL)
    {
      if (NULL != payload_ptr->embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr)
      {
        qcril_data_utils_embms_copy_tmgi_list(
            &(evt->payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr),
            payload_ptr->embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr,
            payload_ptr->embms_content_desc_info.content_desc_tmgi.tmgi_list_len);
      }
    }
#endif
  }

  /*
    Call QCRIL API to process this event
    The data event hdlr will be called by RIL thread
    In case of unsol event RIL Token will be 0
  */
  QCRIL_LOG_VERBOSE( "queue QCRIL DATA event for RIL Token [%d] "       \
                     "instance_id [%d], and modem_id [%d]",
                     qcril_log_get_token_id( info_tbl_ptr->pend_tok ),
                     info_tbl_ptr->instance_id, info_tbl_ptr->modem_id);

  if(E_SUCCESS == qcril_event_queue( instance_id,
                                     modem_id,
                                     QCRIL_DATA_NOT_ON_STACK,
                                     QCRIL_EVT_DATA_EVENT_CALLBACK,
                                     ( void * )evt,
                                     sizeof( qcril_data_event_data_t ),
                                     pend_tok ))
  {
    return;
  }
  else
  {
    QCRIL_LOG_ERROR("%s", "qcril_event_queue failed\n");
  }

err_ret:
  /* free allocated memory for immediate failure from qcril_event_queue */
#ifdef FEATURE_DATA_EMBMS
  if( NULL != evt->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list)
  {
    QCRIL_LOG_DEBUG("%s", "free memory for tmgi_list\n");
    free(evt->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list);
    evt->payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list = NULL;
  }

  if( NULL != evt->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list)
  {
    QCRIL_LOG_DEBUG("%s", "free memory for deactivate_tmgi_list\n");
    free(evt->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list);
    evt->payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list = NULL;
  }

  if ( NULL != evt->payload.embms_sai_info.available_sai_list)
  {
    QCRIL_LOG_DEBUG("%s", "free memory for available_sai_list\n");
    free(evt->payload.embms_sai_info.available_sai_list);
    evt->payload.embms_sai_info.available_sai_list = NULL;
  }

  if ( NULL != evt->payload.embms_sai_info.camped_sai_list)
  {
    QCRIL_LOG_DEBUG("%s", "free memory for camped_sai_list\n");
    free(evt->payload.embms_sai_info.camped_sai_list);
    evt->payload.embms_sai_info.camped_sai_list = NULL;
  }

  if ( NULL != evt->payload.embms_sai_info.num_sai_per_group)
  {
    QCRIL_LOG_DEBUG("%s", "free memory for num_sai_per_group\n");
    free(evt->payload.embms_sai_info.num_sai_per_group);
    evt->payload.embms_sai_info.num_sai_per_group = NULL;
  }
#endif

  free(evt);
}

/*===========================================================================

                        FUNCTIONS REGISTERED WITH QCRIL

===========================================================================*/
/*===========================================================================

  FUNCTION:  qcril_data_command_hdlr

===========================================================================*/
/*!
    @brief
    Command handler for QCRIL Data

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_command_hdlr(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  QCRIL_LOG_DEBUG( "%s", "qcril_data_command_hdlr: ENTRY" );

  /*-----------------------------------------------------------------------*/
  QCRIL_DS_ASSERT( ( params_ptr != NULL ), "validate input params_ptr" );
  QCRIL_DS_ASSERT( ( ret_ptr    != NULL ), "validate input ret_ptr" );
  /*-----------------------------------------------------------------------*/

  if ( ( params_ptr == NULL ) || ( ret_ptr == NULL ) )
  {
    goto bail;
  }

  qcril_data_cmd_data_t *cmd_data;
  int qmi_err_code, rc;
  dsd_bind_subscription_req_msg_v01  bind_req_msg;
  dsd_bind_subscription_resp_msg_v01  bind_resp_msg;

  cmd_data = ( qcril_data_cmd_data_t * )params_ptr->data;

  QCRIL_DS_LOG_DBG_MEM( "event obj", cmd_data );
  if ( !VALIDATE_LOCAL_DATA_OBJ( cmd_data ) )
  {
    QCRIL_LOG_ERROR( "%s", "bad event obj, cannot free mem, ret with err" );
    goto bail;
  }

  if(cmd_data->cmd_id == QCRIL_DATA_STACK_SWITCH_CMD)
  {
    switch(cmd_data->new_stack_id)
    {
      case QCRIL_MODEM_PRIMARY_STACK_ID:
       global_subs_id = QMI_WDS_PRIMARY_SUBS;
       break;

      case QCRIL_MODEM_SECONDARY_STACK_ID:
       global_subs_id = QMI_WDS_SECONDARY_SUBS;
       break;

      case QCRIL_MODEM_TERTIARY_STACK_ID:
       global_subs_id = QMI_WDS_TERTIARY_SUBS;
       break;

      default:
       global_subs_id = QMI_WDS_DEFAULT_SUBS;
    }

    QCRIL_LOG_DEBUG( "qcril_data_command_hdlr: stack_id: %d, subs_id: %d",
                        cmd_data->new_stack_id,global_subs_id);

    dsi_set_modem_subs_id(global_subs_id);

    qdp_set_subscription(global_subs_id);

    if (QMI_NO_ERR != qmi_wds_bind_subscription ( global_qmi_wds_hndl,
                                              global_subs_id,
                                               &qmi_err_code))
    {
      QCRIL_LOG_ERROR("qmi_wds_bind_subscription: subs_id binding failed %d error",
              qmi_err_code);
    }

    if(global_qmi_dsd_hndl != NULL)
    {
      /* Bind the subscription */
      memset(&bind_req_msg, 0, sizeof(bind_req_msg));
      memset(&bind_resp_msg, 0, sizeof(bind_resp_msg));

      bind_req_msg.bind_subs = qcril_data_get_qmi_dsd_subscription_id();

      QCRIL_LOG_DEBUG("binding subscription to subs=%d",
                      bind_req_msg.bind_subs);

      rc = qmi_client_send_msg_sync(global_qmi_dsd_hndl,
                                    QMI_DSD_BIND_SUBSCRIPTION_REQ_V01,
                                    &bind_req_msg,
                                    sizeof(bind_req_msg),
                                    &bind_resp_msg,
                                    sizeof(bind_resp_msg),
                                    QCRIL_DATA_QMI_DSD_SYNC_MSG_TIMEOUT);
      if (QMI_NO_ERR != rc) {
        QCRIL_LOG_ERROR("failed to bind subscription, err=%d",
                        rc);
      }
      else if (QMI_NO_ERR != bind_resp_msg.resp.result) {
        QCRIL_LOG_ERROR("failed to bind subscription, err=%d",
                          bind_resp_msg.resp.error);
      }

    }

  }
  free(cmd_data);

bail:
   QCRIL_LOG_DEBUG( "%s", "qcril_data_command_hdlr: EXIT" );

 }/* qcril_data_command_callback() */



/*===========================================================================

  FUNCTION:  qcril_data_process_qcrilhook_go_dormant

===========================================================================*/
/*!
    @brief

    Handles RIL_REQUEST_OEM_HOOK_RAW - QCRIL_EVT_HOOK_DATA_GO_DORMANT.
    The input to this handler can be a name of an interface, in which
    case this routine will issue dormancy command on the specified interface.
    If no input is provided, Dormancy is issued on all Active interfaces.
    The request is considered to be successful on receipt of PHSYLINK_DOWN
    indication from DSS.

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return

    None on Success, Generic failure response on Failure

    Notes: This handler assumes that dev names are enumerated as rmnet[0,1,2].
*/
/*=========================================================================*/
void qcril_data_process_qcrilhook_go_dormant
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id = global_instance_id;
  qcril_data_go_dormant_params_type dev_name;
  int dev_instance = QCRIL_DATA_HNDL_INVALID;
  int i, ret_val;
  int rmnet_physlink_down[QCRIL_DATA_MAX_DEVS];
  qcril_data_iface_ioctl_type ioctl = QCRIL_DATA_IFACE_IOCTL_GO_DORMANT;

  boolean dorm_status_changed = FALSE;
  int call_state;

  QCRIL_LOG_DEBUG( "%s","Entered: qcril_data_process_qcrilhook_go_dormant");
  QCRIL_LOG_DEBUG( " request = %d", params_ptr->event_id);
  QCRIL_LOG_DEBUG( " Data Length = %d", params_ptr->datalen);
  QCRIL_LOG_DEBUG( " token = %d", params_ptr->t);

  QCRIL_DS_ASSERT( params_ptr != NULL, "validate params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate ret_ptr" );

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  if ( ( params_ptr == NULL ) || ( ret_ptr == NULL ) )
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p]",
                     (unsigned int *)params_ptr, (unsigned int *)ret_ptr );
    goto err_label;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    goto err_label;
  }

  for(i=0; i<QCRIL_DATA_MAX_DEVS; i++)
  {
    rmnet_physlink_down[i] = FALSE;
  }

  if (params_ptr->datalen == 0)
  {
    /*Issue Go Dormant on all active interfaces*/
    QCRIL_LOG_DEBUG( "%s","RIL provided NULL dev name will issue Dormancy on all active interfaces");

    for( i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
    {
      /* TODO: find out why this check needs to be removed. CR 238473 */
      /* (call_tbl[ info_tbl[i].index ].active  == CALL_ACTIVE_PHYSLINK_UP) &&*/
      GET_DEV_INSTANCE_FROM_NAME(i, dev_instance);
      if (dev_instance < 0 || dev_instance >= QCRIL_DATA_MAX_DEVS)
      {
        QCRIL_LOG_ERROR("invalid dev_instance [%d] derived for index [%d]",
                        dev_instance, i);
        continue;
      }
      if(VALIDATE_LOCAL_DATA_OBJ(&info_tbl[i]) &&
         rmnet_physlink_down[dev_instance] == FALSE)
      {
        QCRIL_LOG_DEBUG( "selected index = %d",i);
        if ((ret_val = qcril_data_iface_ioctl(&info_tbl[i],
                                              ioctl,
                                              &dorm_status_changed,
                                              &call_state))
            == FAILURE)
        {
          QCRIL_LOG_ERROR( "%s","Request to issue Dormancy failed.");
          goto err_label;
        }
        rmnet_physlink_down[dev_instance] = TRUE;
      }
      else
      {
        QCRIL_LOG_ERROR("Can not issue Dormancy for index [%d]"     \
                        "rmnet_phylink_down [%d]",
                        i, rmnet_physlink_down[dev_instance]);
      }
    }/* for() */
  }
  else if (params_ptr->datalen > 0 && params_ptr->datalen <= QCRIL_MAX_DEV_NAME_SIZE)
  {
    /* prepare dev_name buffer */
    memset(&dev_name, 0, sizeof(dev_name));
    /* copy bytes received from RIL */
    memcpy(&dev_name, params_ptr->data,QCRIL_MAX_DEV_NAME_SIZE);
    /* put a null char at the end as required by strncmp function later */
    dev_name[params_ptr->datalen] = '\0';

    for( i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
    {
      GET_DEV_INSTANCE_FROM_NAME(i, dev_instance);
      if (dev_instance < 0 || dev_instance >= QCRIL_DATA_MAX_DEVS)
      {
        QCRIL_LOG_ERROR("invalid dev_instance [%d] derived for index [%d]",
                        dev_instance, i);
        continue;
      }

      /*search info table for the specified interface*/
      if (VALIDATE_LOCAL_DATA_OBJ((&info_tbl[i])) &&
          (!strncmp(info_tbl[i].call_info.dev_name,dev_name,QCRIL_MAX_DEV_NAME_SIZE)) &&
          rmnet_physlink_down[dev_instance] == FALSE)
      {
        QCRIL_LOG_DEBUG( "selected index = %d",i);
        if ((ret_val = qcril_data_iface_ioctl(&info_tbl[i],
                                              ioctl,
                                              &dorm_status_changed,
                                              &call_state))
            == FAILURE)
        {
          QCRIL_LOG_ERROR( "%s","Request to issue Dormancy failed.");
          goto err_label;
        }
        rmnet_physlink_down[dev_instance] = TRUE;
      }
      else
      {
        QCRIL_LOG_ERROR("Can not issue Dormancy for index [%d] with dev_name: %s"     \
                        "rmnet_phylink_down [%d]",
                        i, dev_name,rmnet_physlink_down[dev_instance]);
      }
    } /*for*/
  } /*else*/
  else
  {
    QCRIL_LOG_ERROR( "%s","qcril_data_process_qcrilhook_go_dormant: Bad input received");
    goto err_label;
  }

  QCRIL_LOG_INFO( "%s","qcril_data_process_qcrilhook_go_dormant: EXIT with SUCCESS");

  qcril_data_response_success( instance_id,
                               params_ptr->t,
                               params_ptr->event_id,
                               NULL,
                               0 );
  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
  return;

err_label:
  //post generic failure resp
  QCRIL_LOG_INFO( "%s","qcril_data_process_qcrilhook_go_dormant: EXIT with ERROR");
  if (NULL != params_ptr)
  {
    qcril_data_response_generic_failure( instance_id,
                                         params_ptr->t,
                                         params_ptr->event_id );
  }
  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
  return;
}

/*===========================================================================

  FUNCTION:  qcril_data_process_screen_state_change

===========================================================================*/
/*!
  @brief
  Update qcril_data about the current screen status. QCRIL data can perform
  further optimization processing as part of this indication.

  @args[in] Screen state:
            0 - screen OFF
            1 - screen ON

  @return QCRIL_DS_SUCCESS on success
  @return QCRIL_DS_ERROR on failure.
*/
/*=========================================================================*/
int
qcril_data_process_screen_state_change
(
  boolean screen_state
)
{
  static char  args[PROPERTY_VALUE_MAX];
  int          ret;
  static int   prev_screen_state = -1;

  memset(args, 0, sizeof(args));
  property_get(QCRIL_DATA_PROPERTY_TCP_RST_DROP, args, "");

  if (((0 == strcmp(args, "true")) || (0 == strcmp(args, "TRUE"))) &&
      (prev_screen_state != (int)screen_state))
  {
    QCRIL_LOG_INFO("qcril_data: Screen state changed to: %s",
      screen_state ? "ON" : "OFF");
    dsi_process_screen_state_change(screen_state);
    prev_screen_state = (int)screen_state;
  }

  return QCRIL_DS_SUCCESS;

} /* qcril_data_process_screen_state_change() */


/*===========================================================================

  FUNCTION:  qcril_data_toggle_dormancy_indications

===========================================================================*/
/*!
    @brief

    Handles request to turn ON/OFF dormancy indications. Typically called to
    turn off indications when in power save mode  and turn back on when out
    of power save mode.

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return QCRIL_DS_SUCCESS on success and QCRIL_DS_ERROR on failure.
*/
/*=========================================================================*/
int qcril_data_toggle_dormancy_indications
(
  qcril_data_dormancy_ind_switch_type dorm_ind_switch
)
{
  int i, rmnet_physlink_toggled[QCRIL_DATA_MAX_DEVS];
  int dev_instance;
  int ret_val = QCRIL_DS_ERROR;
  boolean dorm_status_changed = FALSE;
  int call_state;
  dsi_net_evt_t dsi_event = DSI_EVT_MAX;

  qcril_data_iface_ioctl_type ioctl;

  memset(rmnet_physlink_toggled, FALSE, sizeof(rmnet_physlink_toggled));

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  if (dorm_ind_switch == DORMANCY_INDICATIONS_OFF)
  {
    ioctl = QCRIL_DATA_IFACE_IOCTL_DORMANCY_INDICATIONS_OFF;
  }
  else if (dorm_ind_switch == DORMANCY_INDICATIONS_ON)
  {
    ioctl = QCRIL_DATA_IFACE_IOCTL_DORMANCY_INDICATIONS_ON;
  }
  else
  {
    QCRIL_LOG_ERROR( "%s","Bad input received.");
    goto err_label;
  }

  /* update global_dorm_ind
     so if data call is up later, we know what is the prvioius request
     from telephony, the request normally is associted with screen state */

  global_dorm_ind = dorm_ind_switch;

  QCRIL_LOG_DEBUG("Switch ON/OFF dormancy indications on all active " \
                  "interfaces ioctl:%d Update global_dorm_ind: %d",
                  ioctl, global_dorm_ind);

  for( i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
  {
    GET_DEV_INSTANCE_FROM_NAME(i, dev_instance);
    if (QCRIL_DATA_HNDL_INVALID  == dev_instance)
    {
      continue;
    }
    if (dev_instance < 0 || dev_instance >= QCRIL_DATA_MAX_DEVS)
    {
      QCRIL_LOG_ERROR("invalid dev_instance [%d] derived for index [%d]",
                      dev_instance, i);
      continue;
    }

    if(VALIDATE_LOCAL_DATA_OBJ(&info_tbl[i]) &&
       (info_tbl[i].cid != CALL_ID_INVALID) &&
       rmnet_physlink_toggled[dev_instance] == FALSE)
    {
      QCRIL_LOG_DEBUG( "selected index = %d",i);
      if ((ret_val = qcril_data_iface_ioctl(&info_tbl[i],
                                            ioctl,
                                            &dorm_status_changed,
                                            &call_state))
          == FAILURE)
      {
        QCRIL_LOG_ERROR( "%s","Request to toggle Dormancy indication failed.");
        ret_val = QCRIL_DS_ERROR;
        goto err_label;
      }
      else if (dorm_status_changed)
      {
        /* derive dsi event from the call_state */
        switch(call_state)
        {
        case CALL_ACTIVE_PHYSLINK_UP:
          dsi_event = DSI_EVT_PHYSLINK_UP_STATE;
          break;
        case CALL_ACTIVE_PHYSLINK_DOWN:
          dsi_event = DSI_EVT_PHYSLINK_DOWN_STATE;
          break;
        default:
          QCRIL_LOG_ERROR("invalid call_state [%d]", call_state);
          dsi_event = DSI_EVT_MAX;
          break;
        }
        if (dsi_event != DSI_EVT_MAX)
        {
          /* post an async event to the event thread */
          qcril_data_post_dsi_netctrl_event(info_tbl[i].instance_id,
                                            info_tbl[i].modem_id,
                                            info_tbl[i].pend_tok,
                                            &info_tbl[i],
                                            dsi_event,
                                            NULL);
        }
      }

      rmnet_physlink_toggled[dev_instance] = TRUE;
    }
  }/* for() */

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
  QCRIL_LOG_INFO( "%s","qcril_data_toggle_dormancy_indications: EXIT with SUCCESS");
  return ret_val = QCRIL_DS_SUCCESS;

err_label:
  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
  QCRIL_LOG_ERROR( "%s","qcril_data_toggle_dormancy_indications: EXIT with ERROR");
  return ret_val;
}

/*===========================================================================

  FUNCTION:  qcril_data_toggle_limited_sys_indications

===========================================================================*/
/*!
    @brief

    Handles request to turn ON/OFF limited data system status change
    indications. Typically called to TURN-ON limited indications when
    in screen-off state. In screen-on state, this is TURNED-OFF so
    full data system status indications can be received.

    @return QCRIL_DS_SUCCESS on success
    @return QCRIL_DS_ERROR on failure.
*/
/*=========================================================================*/
int
qcril_data_toggle_limited_sys_indications
(
  qcril_data_limited_sys_ind_switch_type       sys_ind_switch
)
{
  int all_calls_dormant;
  int res = QCRIL_DS_SUCCESS;

  all_calls_dormant = qcril_data_all_calls_dormant();

  QCRIL_LOG_INFO ("ENTRY: Limited data system indications switch: %d, "
                  "All calls dormancy status: %d",
                   sys_ind_switch, all_calls_dormant);

  global_data_sys_ind_switch = sys_ind_switch;

  if (LIMITED_SYS_INDICATIONS_OFF == sys_ind_switch)
  {
    /* Register for full indications immedately */
    res = qcril_data_reg_sys_ind(LIMITED_SYS_INDICATIONS_OFF);
  }
  else if (all_calls_dormant)
  {
    /* Register for limited indications immediately */
    res = qcril_data_reg_sys_ind(LIMITED_SYS_INDICATIONS_ON);
  }
  else
  {
    /* Defer registration for limited indications */
    QCRIL_LOG_DEBUG ("Calls active, defering limited "
                     "data sys indication registration");

  }

  QCRIL_LOG_INFO ("%s", "EXIT");
  return res;
}


/*===========================================================================

  FUNCTION:  qcril_data_setup_data_call_timeout_hdlr

===========================================================================*/
/*!
    @brief
    Timeout handler for RIL_REQUEST_SETUP_DATA_CALL

    @return
    None.
*/
/*=========================================================================*/
static void qcril_data_setup_data_call_timeout_hdlr
(
  union sigval sval
)
{
  qcril_data_call_info_tbl_type *info_tbl_ptr = sval.sival_ptr;
  qcril_data_call_response_type  response;

  QCRIL_LOG_VERBOSE( "%s", "qcril_data_setup_data_call_timeout_hdlr: ENTRY" );

  /* Lock the mutex before cleanup of info_tbl */
  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  if (!VALIDATE_LOCAL_DATA_OBJ(info_tbl_ptr))
  {
    QCRIL_LOG_ERROR( "invalid info_tbl_ptr [%p] ",
                     (unsigned int *)info_tbl_ptr);
    goto err_label;
  }

  /* If the timer hasn't been canceled right after timing out */
  if (QCRIL_DATA_INVALID_TIMERID != info_tbl_ptr->timer_id)
  {
    QCRIL_LOG_DEBUG ("timeout handler for timer [%#x], cid [%d], tok [%ld]",
          info_tbl_ptr->timer_id, info_tbl_ptr->cid,
          qcril_log_get_token_id( info_tbl_ptr->pend_tok ));

    /* If this is a Dual-IP partial retry */
    if (TRUE == info_tbl_ptr->is_partial_retry &&
        DSI_IP_VERSION_4_6 == info_tbl_ptr->dsi_ip_version)
    {
      QCRIL_LOG_DEBUG ("%s", "Partial retry had been attempted...");

      /* Clear the flag and send a response to the upper layer */
      info_tbl_ptr->is_partial_retry = FALSE;

      /* Schedule a new partial retry attempt */
      qcril_data_util_schedule_partial_retry_attempt(info_tbl_ptr, TRUE, TRUE);
    }
    else
    {
      /* Stop the data call */
      QCRIL_LOG_DEBUG ("%s", "stopping data call...");
      if( E_SUCCESS != qcril_data_stop_data_call(info_tbl_ptr) )
      {
        qcril_data_response_generic_failure( info_tbl_ptr->instance_id,
                                             info_tbl_ptr->pend_tok,
                                             RIL_REQUEST_SETUP_DATA_CALL );

        /* Cleanup call state */
        qcril_data_cleanup_call_state( info_tbl_ptr );
      }

      /* Response to client will be generated in event handler */
    }
  }

err_label:
  /* Unock the mutex of info_tbl */
  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

  QCRIL_LOG_VERBOSE( "%s", "qcril_data_setup_data_call_timeout_hdlr: EXIT" );
}


/*===========================================================================

  FUNCTION:  qcril_data_start_response_timer

===========================================================================*/
/*!
    @brief
    Create and start a response timer for setup data call request

    @pre Before calling, info_tbl_mutex must be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
static void qcril_data_start_response_timer
(
  qcril_data_call_info_tbl_type *info_tbl_ptr
)
{
  struct sigevent sigev;
  struct itimerspec itimers;

  static char  args[PROPERTY_VALUE_MAX];
  char  def[QCRIL_DATA_PROPERTY_TIMEOUT_SIZE+1];
  int   ret;

  QCRIL_LOG_VERBOSE( "%s", "qcril_data_start_response_timer: ENTRY" );

  if (!VALIDATE_LOCAL_DATA_OBJ(info_tbl_ptr))
  {
    QCRIL_LOG_ERROR( "invalid info_tbl_ptr:[%p] ",
                     (unsigned int *)info_tbl_ptr);
    return;
  }

  sigev.sigev_notify            = SIGEV_THREAD;
  sigev.sigev_notify_attributes = NULL;
  sigev.sigev_value.sival_ptr   = info_tbl_ptr;
  sigev.sigev_notify_function   = qcril_data_setup_data_call_timeout_hdlr;

  /* The timer_id should be invalid, if not log an error and delete it */
  if (QCRIL_DATA_INVALID_TIMERID != info_tbl_ptr->timer_id)
  {
    QCRIL_LOG_ERROR( "deleting stale timer_id:[%#"PRIxPTR"] ",
                     (uintptr_t)info_tbl_ptr->timer_id);

    qcril_data_util_stop_timer(&info_tbl_ptr->timer_id);
  }

  /* Create the timer */
  if (-1 == timer_create(CLOCK_REALTIME, &sigev, &info_tbl_ptr->timer_id))
  {
    QCRIL_LOG_ERROR( "failed to create timer for info_tbl_ptr:[%p] ",
                     (unsigned int *)info_tbl_ptr);
    return;
  }

  QCRIL_LOG_VERBOSE( "timer creation success: [%#x]", info_tbl_ptr->timer_id );

  memset( args, 0x0, sizeof(args) );
  memset( def, 0x0, sizeof(def) );

  /* assign default value before query property value */
  itimers.it_value.tv_sec = QCRIL_DATA_PROPERTY_TIMEOUT_DEFAULT;
  itimers.it_value.tv_nsec    = 0;
  itimers.it_interval.tv_sec  = 0;
  itimers.it_interval.tv_nsec = 0;

  /* Query Android proprerty for timeout override */
  std_strlprintf( def, sizeof(def)-1, "%d", QCRIL_DATA_PROPERTY_TIMEOUT_DEFAULT );

  ret = property_get( QCRIL_DATA_PROPERTY_TIMEOUT, args, def );

  if( (QCRIL_DATA_PROPERTY_TIMEOUT_SIZE) < ret )
  {
    /* unexpected property size, use default value */
    QCRIL_LOG_ERROR( "System property %s has unexpected size(%d)"
                     "using default value %d\n",
                     QCRIL_DATA_PROPERTY_TIMEOUT,
                     ret,
                     itimers.it_value.tv_sec );
  }
  else
  {
    ret = ds_atoi( args );
    if( QCRIL_DATA_PROPERTY_TIMEOUT_INVALID < ret )
    {
      /* Update timeout value using property */
      itimers.it_value.tv_sec = ret;
      QCRIL_LOG_VERBOSE("setup data call Timer overide specified, using value %d\n",
                         ret );
    }
    else
    {
      /* invalid property value, use default value */
      QCRIL_LOG_VERBOSE("Invalid data call Timer, using default value %d\n",
                         itimers.it_value.tv_sec);
    }
  }

  /* Start the timer */
  if (-1 == timer_settime(info_tbl_ptr->timer_id, 0, &itimers, NULL))
  {
    QCRIL_LOG_ERROR( "failed to start timer for timer_id [%#"PRIxPTR"], deleting... ",
                     (uintptr_t)info_tbl_ptr->timer_id);

    qcril_data_util_stop_timer(&info_tbl_ptr->timer_id);
  }
}


/*===========================================================================

  FUNCTION:  qcril_data_create_retry_timer

===========================================================================*/
/*!
    @brief
    Create a partial retry timer for use with Dual-IP calls

    @pre Before calling, info_tbl_mutex must be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
static void qcril_data_create_retry_timer
(
  qcril_data_call_info_tbl_type *info_tbl_ptr
)
{
  struct sigevent sigev;
  struct itimerspec itimers;

  QCRIL_LOG_FUNC_ENTRY();

  if (!VALIDATE_LOCAL_DATA_OBJ(info_tbl_ptr))
  {
    QCRIL_LOG_ERROR( "invalid info_tbl_ptr:[%p] ",
                     (unsigned int *)info_tbl_ptr);
    goto bail;
  }

  /* The timer_id should be invalid, if not log an error and delete it */
  if (QCRIL_DATA_INVALID_TIMERID != info_tbl_ptr->retry_timer_id)
  {
    QCRIL_LOG_ERROR( "deleting stale retry_timer_id:[%#"PRIxPTR"] ",
                     (uintptr_t)info_tbl_ptr->retry_timer_id);

    qcril_data_util_stop_timer(&info_tbl_ptr->retry_timer_id);
  }

  sigev.sigev_notify            = SIGEV_THREAD;
  sigev.sigev_notify_attributes = NULL;
  sigev.sigev_value.sival_ptr   = info_tbl_ptr;
  sigev.sigev_notify_function   = qcril_data_partial_retry_hdlr;

  /* Create the timer */
  if (-1 == timer_create(CLOCK_REALTIME, &sigev, &info_tbl_ptr->retry_timer_id))
  {
    QCRIL_LOG_ERROR( "failed to create timer for info_tbl_ptr:[%p] ",
                     (unsigned int *)info_tbl_ptr);
    goto bail;
  }

  QCRIL_LOG_VERBOSE( "timer creation success: [%#x]", info_tbl_ptr->retry_timer_id );

bail:
  QCRIL_LOG_FUNC_RETURN();
}


/*===========================================================================

  FUNCTION:  qcril_data_partial_retry_hdlr

===========================================================================*/
/*!
    @brief
    Handler for Dual-IP partial retries

    @return
    None.
*/
/*=========================================================================*/
static void qcril_data_partial_retry_hdlr
(
  union sigval sval
)
{
  qcril_data_call_info_tbl_type *info_tbl_ptr = sval.sival_ptr;
  qcril_data_call_response_type  response;
  dsi_call_param_value_t    partial_retry;
  int ret_failure = FALSE;

  QCRIL_LOG_FUNC_ENTRY();

  /* Lock the mutex before cleanup of info_tbl */
  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  if (!VALIDATE_LOCAL_DATA_OBJ(info_tbl_ptr))
  {
    QCRIL_LOG_ERROR( "invalid info_tbl_ptr [%p] ",
                     (unsigned int *)info_tbl_ptr);
    goto err_label;
  }
  else if (FALSE == qcril_data_util_is_partial_retry_allowed(info_tbl_ptr))
  {
    info_tbl_ptr->partial_retry_count = 0;
    goto err_label;
  }

  /* If the timer hasn't been canceled right after timing out */
  if (QCRIL_DATA_INVALID_TIMERID != info_tbl_ptr->retry_timer_id)
  {
    QCRIL_LOG_DEBUG ("partial retry handler for timer [%#x], cid [%d]",
                     info_tbl_ptr->retry_timer_id,
                     info_tbl_ptr->cid);

    /* Let DSI library know that this is a partial retry */
    partial_retry.buf_val = NULL;
    partial_retry.num_val = TRUE;

    if ( ( dsi_set_data_call_param( info_tbl_ptr->dsi_hndl ,
                                    DSI_CALL_INFO_PARTIAL_RETRY,
                                    &partial_retry) ) != DSI_SUCCESS )
    {
      QCRIL_LOG_ERROR("unable to set partial retry call param, cid [%d]",
                      info_tbl_ptr->cid );
      info_tbl_ptr->is_partial_retry = FALSE;
      ret_failure = TRUE;
      goto err_label;
    }

    /* Start data call */
    if ( ( dsi_start_data_call( info_tbl_ptr->dsi_hndl ) ) != DSI_SUCCESS )
    {
      QCRIL_LOG_ERROR("unable to start data call, cid [%d]",
                      info_tbl_ptr->cid );
      info_tbl_ptr->is_partial_retry = FALSE;
      ret_failure = TRUE;
      goto err_label;
    }
  }

  info_tbl_ptr->is_partial_retry = TRUE;
  qcril_data_start_response_timer(info_tbl_ptr);

err_label:
  if (ret_failure == TRUE)
  {
    /* Notify internal QCRIL clients of call connect */
    (void)qcril_data_client_notify( QCRIL_DATA_EVT_CALL_RELEASED, NULL );

    /* NO_NET event from LL, send ind to RIL & cleanup call state */
    qcril_data_util_update_call_state( info_tbl_ptr, CALL_INACTIVE, PDP_FAIL_ERROR_UNSPECIFIED );
    qcril_data_unsol_call_list_changed( info_tbl_ptr->instance_id );

    /* Clean up call state */
    qcril_data_cleanup_call_state( info_tbl_ptr );
  }

  /* Unock the mutex of info_tbl */
  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_data_store_call_params

===========================================================================*/
/*!
    @brief
    This function stores the given call parameters in the corresponding
    info_tbl entry

    @pre Before calling, info_tbl_mutex must be locked by the calling thread

    @param
      info_tbl_ptr  [in] - info_tbl entry to copy the call paramters to
      ril_tech      [in] - Technology
      ril_profile   [in] - Profile Number
      ril_apn       [in] - APN
      ril_user      [in] - Username
      ril_pass      [in] - Password
      ril_auth_pref [in] - Authentication Preference
      ril_ipfamily  [in] - IP Family

    @return
      None
*/
/*=========================================================================*/
static void
qcril_data_store_call_params
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr,
  const char                     *ril_tech,
  const char                     *ril_profile,
  const char                     *ril_apn,
  const char                     *ril_user,
  const char                     *ril_pass,
  const char                     *ril_auth_pref,
  const char                     *ril_ipfamily
)
{
  if (!info_tbl_ptr)
  {
    QCRIL_LOG_ERROR("%s", "bad parameter");
    return;
  }

  QCRIL_LOG_DEBUG("%s", "storing RIL call params");

  /* Free the memory for call_params if it hasn't been freed */
  if (NULL != info_tbl_ptr->call_params)
  {
    free(info_tbl_ptr->call_params);
    info_tbl_ptr->call_params = NULL;
  }

  /* Allocate storage for call parameters */
  info_tbl_ptr->call_params = malloc(sizeof(qcril_data_call_params_type));

  if (!info_tbl_ptr->call_params)
  {
    QCRIL_LOG_ERROR("%s", "failed to allocate call_params");
    return;
  }

  /* Initialize the memory */
  memset(info_tbl_ptr->call_params, 0, sizeof(qcril_data_call_params_type));

  /* Copy the technology */
  if (ril_tech)
  {
    QCRIL_LOG_DEBUG("copying ril_tech=%s", ril_tech);

    strlcpy(info_tbl_ptr->call_params->ril_tech,
            ril_tech,
            sizeof(info_tbl_ptr->call_params->ril_tech));
  }

  /* Copy the profile */
  if (ril_profile)
  {
    QCRIL_LOG_DEBUG("copying ril_profile=%s", ril_profile);

    strlcpy(info_tbl_ptr->call_params->ril_profile,
            ril_profile,
            sizeof(info_tbl_ptr->call_params->ril_profile));
  }

  /* Copy the APN */
  if (ril_apn)
  {
    QCRIL_LOG_DEBUG("copying ril_apn=%s", ril_apn);

    strlcpy(info_tbl_ptr->call_params->ril_apn,
            ril_apn,
            sizeof(info_tbl_ptr->call_params->ril_apn));
  }

  /* Copy the Username */
  if (ril_user)
  {
    QCRIL_LOG_DEBUG("copying ril_user=%s", ril_user);

    strlcpy(info_tbl_ptr->call_params->ril_user,
            ril_user,
            sizeof(info_tbl_ptr->call_params->ril_user));
  }

  /* Copy the Password */
  if (ril_pass)
  {
    QCRIL_LOG_DEBUG("copying ril_pass=%s", ril_pass);

    strlcpy(info_tbl_ptr->call_params->ril_pass,
            ril_pass,
            sizeof(info_tbl_ptr->call_params->ril_pass));
  }

  /* Copy the Authentication Preference */
  if (ril_auth_pref)
  {
    QCRIL_LOG_DEBUG("copying ril_auth_pref=%s", ril_auth_pref);

    strlcpy(info_tbl_ptr->call_params->ril_auth_pref,
            ril_auth_pref,
            sizeof(info_tbl_ptr->call_params->ril_auth_pref));
  }

  /* Copy the IP Family */
  if (ril_ipfamily)
  {
    QCRIL_LOG_DEBUG("copying ril_ipfamily=%s", ril_ipfamily);

    strlcpy(info_tbl_ptr->call_params->ril_ipfamily,
            ril_ipfamily,
            sizeof(info_tbl_ptr->call_params->ril_ipfamily));
  }
}

#ifdef RIL_REQUEST_SET_INITIAL_ATTACH_APN
/*===========================================================================

  FUNCTION: qcril_data_is_attach_required

===========================================================================*/
/*!
  @brief
  Checks if the attach profile parameters sent by telephony with the
  attach profile parameters on the modem. LTE attach would be required
  only if there is any change in the attach APN parameters

  @return
  TRUE if any of the parameters doesn't match
  FALSE if all the parameters match

*/
/*===========================================================================*/
LOCAL boolean qcril_data_is_attach_required
(
  unsigned int  profile_id,
  const char    * ril_apn,
  const char    * ril_ipfamily,
  const char    * ril_user,
  const char    * ril_pass,
  const int     ril_user_len,
  const int     ril_pass_len,
  const int     auth_type
)
{

#define PAP_CHAP_NOT_ALLOWED   0
#define PAP_ONLY_ALLOWED       1
#define CHAP_ONLY_ALLOWED      2
#define PAP_CHAP_BOTH_ALLOWED  3
#define UNKNONW_AUTH          -1
#define QDP_UNIQUE_PROFILE_NAME "qdp_profile"

  int                               i, rc = QMI_INTERNAL_ERR, error_code;
  qmi_wds_lte_attach_pdn_list_type  prev_pdn_list;
  qmi_wds_profile_id_type           prof_id;
  qmi_wds_profile_params_type       prof_params;
  qmi_wds_auth_pref_type            modem_auth_pref;
  boolean                           qdp_created_profile = FALSE;

  /* Check for EPC profiles only if epc_profiles are supported */

  if(epc_profile_supported)
  {
    prof_id.technology = QMI_WDS_PROFILE_TECH_EPC;
    prof_id.profile_index = profile_id;

    rc = qmi_wds_query_profile(global_qmi_wds_hndl,
                             &prof_id,
                             &prof_params,
                             &error_code);

  }

  if ((QMI_NO_ERR == rc) && (QMI_SERVICE_ERR_NONE == error_code))
  {
    QCRIL_LOG_DEBUG("Profile id [%d] is an EPC profile",prof_id.profile_index);
  }
  else
  {
    /* This is not a EPC profile continue assuming 3GPP */

    prof_id.technology = QMI_WDS_PROFILE_TECH_3GPP;
    prof_id.profile_index = profile_id;

    rc = qmi_wds_query_profile(global_qmi_wds_hndl,
                               &prof_id,
                               &prof_params,
                               &error_code);


    if (QMI_NO_ERR != rc)
    {
      QCRIL_LOG_ERROR("Profile querying failed for profile [%lu] with error code [%d][%d]",
                        prof_id.profile_index, rc, error_code);

       /*If profile query fails, we might need to do LTE attach */
     return TRUE;
    }
    else
    {

      QCRIL_LOG_DEBUG("Profile id [%d] is a 3GPP profile",prof_id.profile_index);
    }

  }


  if(prof_id.technology == QMI_WDS_PROFILE_TECH_3GPP)
  {
  /* Check all the parameters of the profile with the ones sent
   * from telephony. If all the parameters match then we do not
   * need to do detach and attach. If one of the parameters
   * do not match, then LTE detach and attach is required */

    /* Check APN*/
    if ((!(prof_params.umts_profile_params.param_mask &
                 QMI_WDS_UMTS_PROFILE_APN_NAME_PARAM_MASK)) ||
              NULL == prof_params.umts_profile_params.apn_name)
    {
      /* Modem APN parameter not available */
      QCRIL_LOG_DEBUG("%s", "APN information not available, we need to do LTE attach");
      return TRUE;
    }
    else
    {
      if (strncasecmp( ril_apn,
                       prof_params.umts_profile_params.apn_name,
                       (strlen(ril_apn)+1) ) != 0)
      {
        QCRIL_LOG_ERROR("Modem APN [%s] Attach APN [%s]",
                        prof_params.umts_profile_params.apn_name,
                        ril_apn);
        return TRUE;
      }
    }

    /* Check protocol */
    if(!(prof_params.umts_profile_params.param_mask &
               QMI_WDS_UMTS_PROFILE_PDP_TYPE_PARAM_MASK))
    {
      /* Modem protocol parameter not available */
      QCRIL_LOG_ERROR("%s", "Protocol info not available, we need to do LTE attach");
      return TRUE;
    }

    if(strncasecmp(prof_params.umts_profile_params.profile_name,
               QDP_UNIQUE_PROFILE_NAME,
                   QMI_WDS_MAX_PROFILE_STR_SIZE))
    {
      qdp_created_profile = FALSE;
    }
    else
    {
      qdp_created_profile = TRUE;
      QCRIL_LOG_DEBUG("%s", "Found QDP created profile");
    }

    /* Check for strict IP type for QDP created profiles  */
    switch(prof_params.umts_profile_params.pdp_type)
    {
    case QMI_WDS_PDP_TYPE_IP:
      if (( strncasecmp( ril_ipfamily,
                        QDP_RIL_IP_4,
                        (strlen(QDP_RIL_IP_4)+1) ) != 0 ) ||
            (( strlen(QDP_RIL_IP_4) != strlen(ril_ipfamily)) && (qdp_created_profile)))
      {
        QCRIL_LOG_ERROR("%s", "Modem IP is [%s] and ril_ipfamily is [%s]",
                        QDP_RIL_IP_4, ril_ipfamily);
        return TRUE;
      }

      break;

    case QMI_WDS_PDP_TYPE_IPV6:
      if (( strncasecmp( ril_ipfamily,
                        QDP_RIL_IP_6,
                        (strlen(QDP_RIL_IP_6)+1) ) != 0 ) ||
            (( strlen(QDP_RIL_IP_6) != strlen(ril_ipfamily))&& (qdp_created_profile)))
      {
        QCRIL_LOG_ERROR("%s", "Modem IP is [%s] and ril_ipfamily is [%s]",
                        QDP_RIL_IP_6, ril_ipfamily);
        return TRUE;
      }

      break;

    case QMI_WDS_PDP_TYPE_IPV4V6:
      {
        if (( strncasecmp( ril_ipfamily,
                        QDP_RIL_IP_4_6,
                        (strlen(QDP_RIL_IP_4_6)+1) ) != 0 ) && (qdp_created_profile))
        {
          QCRIL_LOG_ERROR("%s", "Modem IP is [%s] and ril_ipfamily is [%s]",
                          QDP_RIL_IP_4_6, ril_ipfamily);
          return TRUE;
        }
      }
      break;

    default:
      break;
    }
     /* user name checking */
    if ((!(prof_params.umts_profile_params.param_mask &
                 QMI_WDS_UMTS_PROFILE_USERNAME_PARAM_MASK)) ||
              NULL == prof_params.umts_profile_params.username)
    {
      /* Modem not provided user but ril provided */
      if (ril_user_len > 0)
      {
         QCRIL_LOG_DEBUG("%s", "Modem user name:NULL but ril has user so do LTE attach");
         return TRUE;
      }

    }
    else
    {
      /* modem provided username and ril also provided so compare*/
      if (ril_user_len > 0)
      {
        if (strncasecmp( ril_user,
                         prof_params.umts_profile_params.username,
                         (strlen(ril_user)+1) ) != 0)
        {
          QCRIL_LOG_ERROR("Modem user [%s] Attach user [%s]",
                          prof_params.umts_profile_params.username,
                          ril_user);
          return TRUE;
        }
      }
    }

    /* Modem and RIL didn't provide username do nothing */

    /* password checking */
    if ((!(prof_params.umts_profile_params.param_mask &
                 QMI_WDS_UMTS_PROFILE_PASSWORD_PARAM_MASK)) ||
              NULL == prof_params.umts_profile_params.password)
    {
      /* Modem not provided password but ril provided */
      if (ril_pass_len > 0)
      {
         QCRIL_LOG_ERROR("%s", "Modem pwd name:NULL but ril has password so do LTE attach");
         return TRUE;
      }

    }
    else
    {
      /* modem provided pass and ril also provided so compare */
      if (ril_pass_len > 0)
      {
        if (strncasecmp( ril_pass,
                         prof_params.umts_profile_params.password,
                         (strlen(ril_pass)+1) ) != 0)
        {
          QCRIL_LOG_ERROR("Modem pwd [%s] Attach pwd [%s]",
                          prof_params.umts_profile_params.password,
                          ril_pass);
          return TRUE;
        }
      }
    }

    /* Auth-protocol */
    if(!(prof_params.umts_profile_params.param_mask &
               QMI_WDS_UMTS_PROFILE_AUTH_PREF_PARAM_MASK))
    {
      /* Modem protocol parameter not available */
      QCRIL_LOG_ERROR("%s", "Auth-Protocol info not available, we need to do LTE attach");
      return TRUE;
    }

    switch(auth_type)
    {
      case PAP_CHAP_NOT_ALLOWED:
      case UNKNONW_AUTH:
        if (prof_params.umts_profile_params.auth_pref != QMI_WDS_AUTH_PREF_PAP_CHAP_NOT_ALLOWED )
        {
          QCRIL_LOG_ERROR("Modem auth type is [%d] and ril_auth-type is [%d]",
                          QMI_WDS_AUTH_PREF_PAP_CHAP_NOT_ALLOWED, auth_type);
          return TRUE;
        }
        break;

      case PAP_ONLY_ALLOWED:
        if (prof_params.umts_profile_params.auth_pref != QMI_WDS_AUTH_PREF_PAP_ONLY_ALLOWED )
        {
          QCRIL_LOG_ERROR("Modem auth type is [%d] and ril_auth-type is [%d]",
                          QMI_WDS_AUTH_PREF_PAP_ONLY_ALLOWED, auth_type);
          return TRUE;
        }
        break;

      case CHAP_ONLY_ALLOWED:
        if (prof_params.umts_profile_params.auth_pref != QMI_WDS_AUTH_PREF_CHAP_ONLY_ALLOWED )
        {
          QCRIL_LOG_ERROR("Modem auth type is [%d] and ril_auth-type is [%d]",
                          QMI_WDS_AUTH_PREF_CHAP_ONLY_ALLOWED, auth_type);
          return TRUE;
        }
        break;

      case PAP_CHAP_BOTH_ALLOWED:
        if (prof_params.umts_profile_params.auth_pref != QMI_WDS_AUTH_PREF_PAP_CHAP_BOTH_ALLOWED )
        {
          QCRIL_LOG_ERROR("Modem auth type is [%d] and ril_auth-type is [%d]",
                          QMI_WDS_AUTH_PREF_PAP_CHAP_BOTH_ALLOWED, auth_type);
          return TRUE;
        }
        break;

      default:
        break;
    }
  }
  else
  {
    /* Check APN*/
    if (!(prof_params.epc_profile_params.param_mask &
                 QMI_WDS_EPC_PROFILE_APN_NAME_PARAM_MASK))
    {
      /* Modem APN parameter not available */
      QCRIL_LOG_DEBUG("%s", "APN information not available, we need to do LTE attach");
      return TRUE;
    }
    else
    {
      if((NULL != prof_params.epc_profile_params.apn_name) &&
         (NULL != prof_params.epc_profile_params.cdma_apn_string))
      {
        if (strncasecmp(
            prof_params.epc_profile_params.apn_name,
            prof_params.epc_profile_params.cdma_apn_string,
            std_strlen(prof_params.epc_profile_params.apn_name)) != 0)
        {
          QCRIL_LOG_DEBUG("Invalid config UMTS_APN[%s] != CDMA_APN[%s]",
                            prof_params.epc_profile_params.apn_name,
                            prof_params.epc_profile_params.cdma_apn_string);
          return TRUE;
        }
      }
      /* If CMDA string is filled, copy the same to UMTS APN for comparision */
      else if(NULL != prof_params.epc_profile_params.cdma_apn_string)
      {
        memcpy(prof_params.epc_profile_params.apn_name,
                prof_params.epc_profile_params.cdma_apn_string,
                std_strlen(prof_params.epc_profile_params.cdma_apn_string));
      }

      if( NULL != prof_params.epc_profile_params.apn_name )
      {
        if (strncasecmp( ril_apn,
                         prof_params.epc_profile_params.apn_name,
                         (strlen(ril_apn)+1) ) != 0)
        {
          QCRIL_LOG_DEBUG("Modem APN [%s] Attach APN [%s]",
                          prof_params.epc_profile_params.apn_name,
                          ril_apn);
          return TRUE;
        }
      }
    }

    /* Check protocol */
    if(!(prof_params.epc_profile_params.param_mask &
               QMI_WDS_EPC_UMTS_PROFILE_PDP_TYPE_PARAM_MASK))
    {
      /* Modem protocol parameter not available */
      QCRIL_LOG_DEBUG("%s", "Protocol info not available, we need to do LTE attach");
      return TRUE;
    }

    switch(prof_params.epc_profile_params.pdp_type)
    {
    case QMI_WDS_PDP_TYPE_IP:
      if ( strncasecmp( ril_ipfamily,
                        QDP_RIL_IP_4,
                        (strlen(QDP_RIL_IP_4)+1) ) != 0 )
      {
        QCRIL_LOG_DEBUG("%s", "Modem IP is [%s] and ril_ipfamily is [%s]",
                        QDP_RIL_IP_4, ril_ipfamily);
        return TRUE;
      }

      break;

    case QMI_WDS_PDP_TYPE_IPV6:
      if ( strncasecmp( ril_ipfamily,
                        QDP_RIL_IP_6,
                        (strlen(QDP_RIL_IP_6)+1) ) != 0 )
      {
        QCRIL_LOG_DEBUG("%s", "Modem IP is [%s] and ril_ipfamily is [%s]",
                        QDP_RIL_IP_6, ril_ipfamily);
        return TRUE;
      }

      break;

    case QMI_WDS_PDP_TYPE_IPV4V6:
      {
        QCRIL_LOG_DEBUG("%s", "Modem has more pref IP  [%s] than  ril_ipfamily  [%s]"
                        "skip re-attach ",QDP_RIL_IP_4_6, ril_ipfamily);
      }
      break;

    default:
      break;
    }
     /* user name checking */

    if((prof_params.epc_profile_params.param_mask &
                   QMI_WDS_EPC_COMMON_USER_ID_PARAM_MASK) &&
            (NULL != prof_params.epc_profile_params.common_username))
    {
      if (ril_user_len > 0)
      {
        if (strncasecmp( ril_user,
                         prof_params.epc_profile_params.common_username,
                         (strlen(ril_user)+1) ) != 0)
        {
          QCRIL_LOG_DEBUG("Modem user [%s] Attach user [%s]",
                          prof_params.epc_profile_params.common_username,
                           ril_user);
          return TRUE;
        }
      }
    }
    else
    {
      if (!(prof_params.epc_profile_params.param_mask &
                   QMI_WDS_EPC_PROFILE_USERNAME_PARAM_MASK))
      {
        /* Modem not provided user but ril provided */
        if (ril_user_len > 0)
        {
           QCRIL_LOG_DEBUG("%s", "Modem user name:NULL but ril has user so do LTE attach");
           return TRUE;
        }

      }
      else
      {
        if((NULL != prof_params.epc_profile_params.username) &&
           (NULL != prof_params.epc_profile_params.cdma_username))
        {
          if (strncasecmp(
              prof_params.epc_profile_params.username,
              prof_params.epc_profile_params.cdma_username,
              std_strlen(prof_params.epc_profile_params.username)) != 0)
          {
            QCRIL_LOG_DEBUG("Invalid config UMTS_UNAME[%s] != CDMA_UNAME[%s]",
                              prof_params.epc_profile_params.username,
                              prof_params.epc_profile_params.cdma_username);
            return TRUE;
          }
        }
        else if(NULL != prof_params.epc_profile_params.cdma_username)
        {
          memcpy(prof_params.epc_profile_params.username,
                  prof_params.epc_profile_params.cdma_username,
                  std_strlen(prof_params.epc_profile_params.cdma_username));
        }

        /* modem provided username and ril also provided so compare*/
        if(NULL != prof_params.epc_profile_params.username)
        {
          if (ril_user_len > 0)
          {
            if (strncasecmp( ril_user,
                             prof_params.epc_profile_params.username,
                             (strlen(ril_user)+1) ) != 0)
            {
              QCRIL_LOG_DEBUG("Modem user [%s] Attach user [%s]",
                              prof_params.epc_profile_params.username,
                              ril_user);
              return TRUE;
            }
          }
        }
      }
    }

    /* Modem and RIL didn't provide username do nothing */

    /* password checking */
    if((prof_params.epc_profile_params.param_mask &
                   QMI_WDS_EPC_COMMON_AUTH_PASSWORD_PARAM_MASK) &&
            (NULL != prof_params.epc_profile_params.common_password))
    {
      if (ril_pass_len > 0)
      {
        if (strncasecmp( ril_pass,
                         prof_params.epc_profile_params.common_password,
                         (strlen(ril_pass)+1) ) != 0)
        {
          QCRIL_LOG_DEBUG("Modem pass [%s] Attach user [%s]",
                          prof_params.epc_profile_params.common_password,
                           ril_pass);
          return TRUE;
        }
      }
    }
    else
    {
      if (!(prof_params.epc_profile_params.param_mask &
                   QMI_WDS_EPC_PROFILE_USERNAME_PARAM_MASK))
      {
        /* Modem not provided user but ril provided */
        if (ril_pass_len > 0)
        {
           QCRIL_LOG_DEBUG("%s", "Modem user name:NULL but ril has user so do LTE attach");
           return TRUE;
        }

      }
      else
      {
        if((NULL != prof_params.epc_profile_params.password) &&
           (NULL != prof_params.epc_profile_params.cdma_password))
        {
          if (strncasecmp(
              prof_params.epc_profile_params.password,
              prof_params.epc_profile_params.cdma_password,
              std_strlen(prof_params.epc_profile_params.password)) != 0)
          {
            QCRIL_LOG_DEBUG("Invalid config UMTS_PWORD[%s] != CDMA_PWORD[%s]",
                              prof_params.epc_profile_params.password,
                              prof_params.epc_profile_params.cdma_password);
            return TRUE;
          }
        }
        else if(NULL != prof_params.epc_profile_params.cdma_password)
        {
          memcpy(prof_params.epc_profile_params.password,
                  prof_params.epc_profile_params.cdma_password,
                  std_strlen(prof_params.epc_profile_params.cdma_password));
        }

        /* modem provided username and ril also provided so compare*/
        if(NULL != prof_params.epc_profile_params.password)
        {
          if (ril_pass_len > 0)
          {
            if (strncasecmp( ril_pass,
                             prof_params.epc_profile_params.password,
                             (strlen(ril_pass)+1) ) != 0)
            {
              QCRIL_LOG_DEBUG("Modem pword [%s] Attach pword [%s]",
                              prof_params.epc_profile_params.password,
                              ril_pass);
              return TRUE;
            }
          }
        }

        if(!(prof_params.epc_profile_params.param_mask &
                   QMI_WDS_EPC_COMMON_AUTH_PROTOCOL_PARAM_MASK) ||
           !(prof_params.epc_profile_params.param_mask &
                   QMI_WDS_EPC_UMTS_PROFILE_AUTH_PREF_PARAM_MASK))
        {
          /* Modem protocol parameter not available */
          QCRIL_LOG_DEBUG("%s", "Auth-Protocol info not available, we need to do LTE attach");
          return TRUE;
        }
	else if(prof_params.epc_profile_params.param_mask &
                   QMI_WDS_EPC_COMMON_AUTH_PROTOCOL_PARAM_MASK)
        {
          modem_auth_pref = prof_params.epc_profile_params.common_auth_pref;
        }
	else if(prof_params.epc_profile_params.param_mask &
                   QMI_WDS_EPC_UMTS_PROFILE_AUTH_PREF_PARAM_MASK)
        {
          modem_auth_pref = prof_params.epc_profile_params.auth_pref;
	}


        switch(auth_type)
        {
          case PAP_CHAP_NOT_ALLOWED:
          case UNKNONW_AUTH:
            if (modem_auth_pref != QMI_WDS_AUTH_PREF_PAP_CHAP_NOT_ALLOWED )
            {
              QCRIL_LOG_ERROR("Modem auth type is [%d] and ril_auth-type is [%d]",
                              modem_auth_pref, auth_type);
              return TRUE;
            }
            break;

          case PAP_ONLY_ALLOWED:
            if (modem_auth_pref != QMI_WDS_AUTH_PREF_PAP_ONLY_ALLOWED )
            {
              QCRIL_LOG_ERROR("Modem auth type is [%d] and ril_auth-type is [%d]",
                              modem_auth_pref, auth_type);
              return TRUE;
            }
            break;

          case CHAP_ONLY_ALLOWED:
            if (modem_auth_pref != QMI_WDS_AUTH_PREF_CHAP_ONLY_ALLOWED )
            {
              QCRIL_LOG_ERROR("Modem auth type is [%d] and ril_auth-type is [%d]",
                              modem_auth_pref, auth_type);
              return TRUE;
            }
            break;

          case PAP_CHAP_BOTH_ALLOWED:
            if (modem_auth_pref != QMI_WDS_AUTH_PREF_PAP_CHAP_BOTH_ALLOWED )
            {
              QCRIL_LOG_ERROR("Modem auth type is [%d] and ril_auth-type is [%d]",
                              modem_auth_pref, auth_type);
              return TRUE;
            }
            break;

          default:
            break;
        }
      }
    }
  }
  /* Auth-protocol */

  /* We compared all the parameters and found that
   * the profile information sent by telephony is
   * already available on the modem, so we need not
   * do LTE attach */
  return FALSE;
}

/*===========================================================================

  FUNCTION: qcril_data_request_set_lte_attach_profile

===========================================================================*/
/*!
  @brief
  Handles RIL_REQUEST_SET_INITIAL_ATTACH_APN.

  @return
  None.

*/
/*===========================================================================*/
RIL_Errno qcril_data_request_set_lte_attach_profile
(
  RIL_InitialAttachApn* attachInfo
)
{
  int                               i, rc;
  const char                        *ril_apn;
  const char                        *ril_user;
  const char                        *ril_pass;
  int                               auth_type;
  const char                        *ril_auth_type = NULL;
  const char                        *ril_ipfamily = NULL;
  const char                        *qdp_params[QDP_RIL_PARAM_MAX];
  unsigned int                      lte_profile_id = QDP_INVALID_PROFILE;
  unsigned int                      input_profile_id = QDP_INVALID_PROFILE;
  qdp_profile_pdn_type              qdp_lte_profile_pdn_type = QDP_PROFILE_PDN_TYPE_INVALID;
  qdp_error_info_t                  error_info;

  int                               max_attach_pdns;
  int                               error_code;
  qmi_wds_lte_attach_pdn_list_type  prev_pdn_list;

  /* Profile information stored locally. In case
   * lte attach fails this information will be
   * used to restore the profile on the modem */
  qmi_wds_profile_params_type       prof_params;
  qmi_wds_profile_id_type           prof_id;
  int                               reti = QCRIL_LTE_ATTACH_SUCCESS;
  int                               qmi_err_code, temp_len = 0;
  char                              args[PROP_VALUE_MAX];
  int                               ril_user_len =0;
  int                               ril_pass_len =0;
  QCRIL_LOG_VERBOSE( "%s", "qcril_data_request_set_lte_attach_profile: ENTRY" );

  if ( attachInfo == NULL )
  {
    QCRIL_LOG_ERROR( "%s", "BAD input, attachInfo");
    return QCRIL_LTE_ATTACH_FAILURE;
  }

  /* Get the data */
  ril_apn       = attachInfo->apn;
  ril_user      = attachInfo->username;
  ril_pass      = attachInfo->password;
  auth_type     = attachInfo->authtype;
  ril_ipfamily  = attachInfo->protocol;

  do
  {
    if (NULL == ril_apn)
    {
      /* TODO: Case when provided APN is NULL */
      QCRIL_LOG_ERROR("%s","ril_apn (provided NULL) is mandatory param for "
                      "looking up profile.");
      reti = QCRIL_LTE_ATTACH_FAILURE;
      break;
    }

    memset(qdp_params, 0, sizeof(qdp_params));

    /* build up QDP parameters */
    /* prepare APN */
    if (strlen(ril_apn) > (QMI_WDS_MAX_APN_STR_SIZE-1))
    {
      QCRIL_LOG_ERROR("RIL provided invalid APN [%s] "
                      "APN length [%d] exceeds max allowed [%d]",
                      ril_apn, strlen(ril_apn), QMI_WDS_MAX_APN_STR_SIZE-1);
      reti = QCRIL_LTE_ATTACH_FAILURE;
      break;
    }
    qdp_params[QDP_RIL_APN] = ril_apn;

    QCRIL_LOG_DEBUG("qdp_param APN = [%s]", qdp_params[QDP_RIL_APN]);

    /* prepare tech */
    qdp_params[QDP_RIL_TECH] = QDP_RIL_3GPP;

    /* prepare ip family */
    if (ril_ipfamily != NULL && strlen(ril_ipfamily) > 0)
    {
      if (strlen(ril_ipfamily) > QCRIL_IP_FAMILY_STR_MAX)
      {
        QCRIL_LOG_ERROR("RIL provided invalid ip family [%s] "
                        "ip family length [%d] exceeds max allowed [%d]",
                        ril_ipfamily, strlen(ril_ipfamily), QCRIL_IP_FAMILY_STR_MAX);
        reti = QCRIL_LTE_ATTACH_FAILURE;
        break;
      }
      qdp_params[QDP_RIL_IP_FAMILY] = ril_ipfamily;

      QCRIL_LOG_DEBUG("qdp param IP_FAMILY = [%s]", qdp_params[QDP_RIL_IP_FAMILY]);
    }

    /* prepare auth */
    switch (auth_type)
    {
    case QMI_WDS_AUTH_PREF_PAP_CHAP_NOT_ALLOWED:
        ril_auth_type = QDP_RIL_PAP_CHAP_NOT_ALLOWED;
        break;
    case QMI_WDS_AUTH_PREF_PAP_ONLY_ALLOWED:
        ril_auth_type = QDP_RIL_PAP_ONLY_ALLOWED;
        break;
    case QMI_WDS_AUTH_PREF_CHAP_ONLY_ALLOWED:
        ril_auth_type = QDP_RIL_CHAP_ONLY_ALLOWED;
        break;
    case QMI_WDS_AUTH_PREF_PAP_CHAP_BOTH_ALLOWED:
        ril_auth_type = QDP_RIL_PAP_CHAP_BOTH_ALLOWED;
        break;
    default:
        QCRIL_LOG_DEBUG("Unknown auth_type [%d]",
                        auth_type);
        ril_auth_type = QDP_RIL_PAP_CHAP_NOT_ALLOWED;
        break;
    }

    if (QCRIL_LTE_ATTACH_SUCCESS != reti)
    {
      break;
    }

    qdp_params[QDP_RIL_AUTH] = ril_auth_type;
    QCRIL_LOG_DEBUG("qdp param RIL_AUTH = [%s]", qdp_params[QDP_RIL_AUTH]);

    /* Prepare username */
    if (ril_user != NULL && strlen(ril_user) > 0)
    {
      if (strlen(ril_user) > QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1)
      {
        QCRIL_LOG_ERROR("RIL provided username exceeds max allowed [%d]",
                        QMI_WDS_MAX_USERNAME_PASS_STR_SIZE);
      }
      else
      {
        qdp_params[QDP_RIL_NAI] = ril_user;
        ril_user_len = strlen(ril_user)+1;
        QCRIL_LOG_DEBUG("qdp param USERNAME = [%s]", qdp_params[QDP_RIL_NAI]);
      }
    }

    /* prepare password */
    if (ril_pass != NULL && strlen(ril_pass) > 0)
    {
      if (strlen(ril_pass) > QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1)
      {
        QCRIL_LOG_ERROR("RIL provided password exceeds max allowed [%d]",
                        QMI_WDS_MAX_USERNAME_PASS_STR_SIZE);
      }
      else
      {
        qdp_params[QDP_RIL_PASSWORD] = ril_pass;
        ril_pass_len = strlen(ril_pass)+1;
        QCRIL_LOG_DEBUG("qdp param PASSWORD = [%s]", qdp_params[QDP_RIL_PASSWORD]);
      }
    }

    memset( &error_info, 0x0, sizeof(error_info) );
    memset( &prof_params, 0,  sizeof(prof_params) );

    if (QMI_NO_ERR == qmi_wds_get_lte_max_attach_pdn_num( global_qmi_wds_hndl,
                                                          &max_attach_pdns,
                                                          &error_code))
    {
      /* Query existing attach profiles on modem */
      memset(&prev_pdn_list, 0, sizeof(prev_pdn_list));
      rc = qmi_wds_get_lte_attach_pdn_list(global_qmi_wds_hndl,
                                           &prev_pdn_list,
                                           &error_code);

      if (QMI_NO_ERR != rc)
      {
        QCRIL_LOG_ERROR("qmi_wds_get_lte_attach_pdn_list failed with error code [%d][%d]",
                        rc, error_code);
        reti = QCRIL_LTE_ATTACH_FAILURE;
      }
      else
      {
        /* Check all profiles in the attach pdn list and compare with incoming APN
         * If the profile is present, send RIL_E_SKIP_LTE_REATTACH. If not, proceed
         * with profile lookup */
        for (i=0; i<prev_pdn_list.len; i++)
        {
          if ( FALSE == qcril_data_is_attach_required(prev_pdn_list.list[i],
                                                    ril_apn,
                                                    ril_ipfamily,
                                                    ril_user,
                                                    ril_pass,
                                                    ril_user_len,
                                                    ril_pass_len,
                                                    auth_type  ) )
        {
            /* The profile already exists on modem and there are no changes to it,
             * skip reattach */
          QCRIL_LOG_DEBUG("%s", "Reattach not required!");
          return RIL_E_SKIP_LTE_REATTACH;
        }
      }
      }
      if (prev_pdn_list.len != 0)
      {
         input_profile_id = prev_pdn_list.list[0];
      }


      /* Lookup profile using QDP */
      rc = qdp_lte_attach_profile_lookup( qdp_params,
                                          &lte_profile_id,
                                          &qdp_lte_profile_pdn_type,
                                          &input_profile_id,
                                          &prof_params,
                                          &error_info);

      if( QDP_SUCCESS != rc )
      {
        QCRIL_LOG_ERROR("Not able to look 3gpp profile id [%d], "
                        "returned by QDP, lookup error[%d] tech[%d]",
                        lte_profile_id, error_info.error, error_info.tech);
        reti = QCRIL_LTE_ATTACH_FAILURE;
        break;
      }

      if (lte_profile_id != QDP_INVALID_PROFILE)
      {

        if (0 == prev_pdn_list.len )
        {
           prev_pdn_list.len = 1;
        }

        prev_pdn_list.list[0] = lte_profile_id;
        rc = qmi_wds_set_lte_attach_pdn_list( global_qmi_wds_hndl,
                                              &prev_pdn_list,
                                              &error_code);

        if (QMI_NO_ERR != rc)
        {
          QCRIL_LOG_ERROR("LTE attach PDN for profile [%d] failed with error [%d][%d]",
                          lte_profile_id, rc, error_code);

          /* Reset profile on the modem */
          prof_id.technology    = QMI_WDS_PROFILE_TECH_3GPP;
          prof_id.profile_index = lte_profile_id;
          rc = qmi_wds_modify_profile( global_qmi_wds_hndl,
                                       &prof_id,
                                       &prof_params,
                                       &error_code);

          if (QMI_NO_ERR != rc)
          {
            QCRIL_LOG_ERROR("Unable to restore profile on modem for profile id [%d] Error [%d][%d]",
                            lte_profile_id, rc, error_code);
          }

          /* Release the profile */
          rc = qdp_profile_release(lte_profile_id);
          if (QDP_SUCCESS != rc)
          {
            QCRIL_LOG_ERROR("%s", "Profile could either be modem profile or there "
                            "was an error during profile release");
          }

          /* Attach failed */
          reti = QCRIL_LTE_ATTACH_FAILURE;
          break;
        }

        /* If this is an attach request with a new APN, we would need to release
         * the old profile */
        if (QDP_INVALID_PROFILE != global_lte_attach_profile
            && global_lte_attach_profile != lte_profile_id)
        {
          rc = qdp_profile_release_ex(global_lte_attach_profile);
          if (QDP_SUCCESS != rc)
          {
            QCRIL_LOG_ERROR("%s", "Profile could either be modem profile or there "
                            "was an error during profile release");
          }
        }

        /* Save the profile in the global LTE profile variable */
        global_lte_attach_profile = lte_profile_id;
      }
    }
    else
    {
      /* Fallback mechanism */
      memset( args, 0x0, sizeof(args) );
      rc = property_get( QCRIL_PROPERTY_DEFAULT_PROFILE,
                         args,
                         QCRIL_LTE_DEFAULT_PROFILE_VALUE);

      if ( QCRIL_PROPERTY_DEFAULT_PROFILE_SIZE < rc )
      {
        QCRIL_LOG_ERROR("Modem does not support this feature!");
        return RIL_E_REQUEST_NOT_SUPPORTED;
      }

      rc = ds_atoi(args);

      if ( rc <= 0 )
      {
        QCRIL_LOG_ERROR("Modem does not support this feature!");
        return RIL_E_REQUEST_NOT_SUPPORTED;
      }

      /* Set the default profile to the property value */
      lte_profile_id = rc;

      /* Check if LTE attach is required */
      if ( FALSE == qcril_data_is_attach_required( lte_profile_id,
                                                   ril_apn,
                                                   ril_ipfamily,
                                                   ril_user,
                                                   ril_pass,
                                                   ril_user_len,
                                                   ril_pass_len,
                                                   auth_type   ) )
      {
        /* Attach is not required since no paramaeters changed */
        QCRIL_LOG_DEBUG("%s", "Reattach not required!");
        return RIL_E_SKIP_LTE_REATTACH;
      }

      rc = qdp_lte_attach_profile_lookup( qdp_params,
                                          &lte_profile_id,
                                          &qdp_lte_profile_pdn_type,
                                          &input_profile_id,
                                          &prof_params,
                                          &error_info);

      if( QDP_SUCCESS != rc )
      {
        QCRIL_LOG_ERROR("Not able to look 3gpp profile id [%d], "
                        "returned by QDP, lookup error[%d] tech[%d]",
                        lte_profile_id, error_info.error, error_info.tech);
        reti = QCRIL_LTE_ATTACH_FAILURE;
        break;
      }

      /* Save the profile in the global LTE profile variable. In case of
       * the fallback case for LTE attach, even if am attach is triggered
       * with a new APN, we would still be updating the default profile id
       * on the modem. So there is no need to free the profile on the modem */
      global_lte_attach_profile = lte_profile_id;
    }
  } while (0);

  if (QCRIL_LTE_ATTACH_SUCCESS != reti)
  {
    QCRIL_LOG_ERROR( "%s", "EXIT with FAILURE" );
    return RIL_E_GENERIC_FAILURE;
  }
  else
  {
    QCRIL_LOG_DEBUG( "%s", "EXIT with SUCCESS" );
    return RIL_E_SUCCESS;
  }
} /* qcril_data_request_set_lte_attach_profile */
#endif /* RIL_REQUEST_SET_INITIAL_ATTACH_APN */

/*===========================================================================

  FUNCTION:  qcril_data_request_setup_data_call

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SETUP_DATA_CALL

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_request_setup_data_call
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type  instance_id;
  qcril_modem_id_e_type     modem_id;
  dsi_hndl_t               *data_hndl;
  int                       i, apn_len;
  dsi_call_param_value_t    apn_info;
  dsi_call_param_value_t    ipfamily_info;
  dsi_call_param_value_t    techpref_info;
  const char               *ril_apn;
  const char               *ril_user;
  const char               *ril_pass;
  const char               *ril_auth_pref = NULL;
  const char               *ril_tech = NULL;
  const char               *ril_common_tech = NULL;
  const char               *ril_profile = NULL;
  const char               *ril_ipfamily = NULL;
  char                      tmp_apn[ DS_CALL_INFO_APN_MAX_LEN + 1 ];
  qcril_reqlist_u_type      u_info;
  qcril_reqlist_public_type reqlist_entry;
  qcril_data_net_cb_info_t *trn;
  int                       umts_profile_id = 0;
  int                       cdma_profile_id = 0;
  int                       call_end_reason = PDP_FAIL_ERROR_UNSPECIFIED;
  dsi_ce_reason_t           dsi_ce_reason;
  qcril_data_call_response_type  response;
  boolean abort_call = FALSE;
  qcril_data_ril_radio_tech_t  numeric_ril_tech = QCRIL_DATA_RIL_RADIO_TECH_UNKNOWN;

  QCRIL_LOG_VERBOSE( "%s", "qcril_data_request_setup_data_call: ENTRY" );

  QCRIL_DS_ASSERT( params_ptr != NULL, "validate params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate ret_ptr" );
  QCRIL_DS_ASSERT ( ( params_ptr->datalen % 4 ) == 0, "validate datalen" );

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  if ( ( params_ptr == NULL ) || ( ret_ptr == NULL ) || ( ( params_ptr->datalen % 4 ) != 0) )
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p], datalen [%d]",
                     (unsigned int *)params_ptr, (unsigned int *)ret_ptr,
                     params_ptr ? params_ptr->datalen : 0 );
    goto err_bad_input;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    goto err_bad_input;
  }

  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  if (modem_id >= QCRIL_MAX_MODEM_ID)
  {
    goto err_bad_input;
  }

  {
    qcril_reqlist_public_type req;

    /* Block SETUP_DATA_CALL request if there is a
     * DEACTIVATE_DATA_CALL request in progress. */
    if( E_SUCCESS == qcril_reqlist_query_by_request( instance_id,
                                                     RIL_REQUEST_DEACTIVATE_DATA_CALL,
                                                     &req ) )
    {
      /* Found pending deactivate request, generate error response */
      QCRIL_LOG_ERROR("%s", "Blocking SETUP_DATA_CALL due to pending DEACTIVATE");
      qcril_data_response_setup_data_call_failure_generic_err(instance_id,
                                                             params_ptr->t);
      goto err_bad_input;
    }
  }


  memset( &u_info,   0, sizeof( qcril_reqlist_u_type ) );
  memset( tmp_apn,   0, DS_CALL_INFO_APN_MAX_LEN + 1 );
  memset( &apn_info, 0, sizeof( dsi_call_param_value_t ) );

  ril_apn  = ((const char **)params_ptr->data)[2];
  ril_user = ((const char **)params_ptr->data)[3];
  ril_pass = ((const char **)params_ptr->data)[4];
  ril_tech = ((const char **)params_ptr->data)[0];
  ril_profile = ((const char **)params_ptr->data)[1];
  ril_auth_pref = ((const char **)params_ptr->data)[5];
  ril_ipfamily = ((const char **)params_ptr->data)[6];

  if (ril_tech != NULL)
  {
    QCRIL_LOG_DEBUG ("RIL provided tech pref [%s]", ril_tech);

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
    numeric_ril_tech = qcril_data_get_numeric_ril_technology(ril_tech);
    ril_common_tech = QCRIL_DATA_IS_RIL_RADIO_TECH_CDMA(numeric_ril_tech) ?
                      QCRIL_CDMA_STRING :
                      QCRIL_UMTS_STRING;
#else
    ril_common_tech = ril_tech;
#endif
    QCRIL_LOG_DEBUG ("RIL - Common tech [%s]", ril_common_tech);
  }
  else
  {
    QCRIL_LOG_DEBUG ("%s", "RIL did not provide tech pref");
  }

  if ( (ril_user != NULL) && (ril_pass != NULL) )
  {
    QCRIL_LOG_DEBUG( "RIL provided USERNAME len [%d], PASSWORD len [%d]",
                      strlen( ril_user), strlen( ril_pass ) );
  }

  if ( ril_apn != NULL )
  {
    /* APN len calculations */
    apn_len = MINIMUM( strlen( ril_apn ), DS_CALL_INFO_APN_MAX_LEN );
    memcpy( tmp_apn, ril_apn, apn_len );
    tmp_apn[ apn_len ] = '\0';
    QCRIL_LOG_DEBUG( "RIL APN [%s]", ril_apn );
  }
  else
  {
    tmp_apn[0] = '\0';
    QCRIL_LOG_DEBUG( "%s", "RIL APN is NULL, Use NULL string." );
  }

  if (!ril_auth_pref)
  {
    QCRIL_LOG_DEBUG("%s","No Authentication Preference provided.");
  }
  else
  {
    QCRIL_LOG_DEBUG("RIL provided authentication preference %s",ril_auth_pref);
  }

  for (i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++)
  {
    if (!VALIDATE_LOCAL_DATA_OBJ( (&info_tbl[i])))
    {
      break;
    }
  }
  if ( i == MAX_CONCURRENT_UMTS_DATA_CALLS )
  {
    QCRIL_LOG_ERROR( "%s", "no free info_tbl entry" );
    goto err_label;
  }

    /* The corresponding dsi_cb_tbl entry should have already been
       cleaned up. But, perform cleanup just in case */
    qcril_data_clear_dsi_cb_tbl_entry(&dsi_cb_tbl[i]);

    /* Allocate CID, copy APN and other call params */
    info_tbl[i].info_flg    = FALSE; /* mark call_info and dev_name as invalid */
    info_tbl[i].index       = i;
    info_tbl[i].cid         = i;
    info_tbl[i].suggestedRetryTime = QCRIL_DATA_SUGGESTEDRETRYTIME;
    memset(info_tbl[i].call_info.apn,
           0,
           sizeof(info_tbl[i].call_info.apn));

    strlcpy( info_tbl[i].call_info.apn,
             tmp_apn,
             sizeof(info_tbl[i].call_info.apn));
    info_tbl[i].instance_id = instance_id;
    info_tbl[i].modem_id    = modem_id;
    info_tbl[i].pend_tok    = params_ptr->t;
    info_tbl[i].pend_req    = RIL_REQUEST_SETUP_DATA_CALL;
    QCRIL_LOG_INFO( "DEBUG:  %s step %d - info_tbl_ptr->pend_tok[0x%x] info_tbl_ptr->pend_req[0x%X]",
                    __func__, 0, info_tbl[i].pend_tok, info_tbl[i].pend_req );
    info_tbl[i].qmi_wds_hndl = QCRIL_DATA_HNDL_INVALID;
    info_tbl[i].dsi_hndl    =  QCRIL_DSI_HNDL_INVALID;
    info_tbl[i].qdp_umts_profile_id = QCRIL_INVALID_PROFILE_ID;
    info_tbl[i].qdp_cdma_profile_id = QCRIL_INVALID_PROFILE_ID;
    info_tbl[i].qdp_3gpp_profile_pdn_type  = QDP_PROFILE_PDN_TYPE_INVALID;
    info_tbl[i].qdp_3gpp2_profile_pdn_type = QDP_PROFILE_PDN_TYPE_INVALID;
    info_tbl[i].self        = &info_tbl[i];
    info_tbl[i].is_partial_retry = FALSE;
    info_tbl[i].last_addr_count  = 0;
    info_tbl[i].partial_retry_count = 0;

    /* Save the RIL provided call paramters */
    qcril_data_store_call_params(&info_tbl[i],
                                 ril_tech,
                                 ril_profile,
                                 ril_apn,
                                 ril_user,
                                 ril_pass,
                                 ril_auth_pref,
                                 ril_ipfamily);

    /* store default call fail error */
    info_tbl[i].call_info.inactiveReason = PDP_FAIL_ERROR_UNSPECIFIED;

    /* The timer shouldn't be running. If for some reason it is
       running then stop it */
    qcril_data_util_stop_timer(&info_tbl[i].timer_id);

    /* The timer shouldn't be running. If for some reason it is
       running then stop it */
    qcril_data_util_stop_timer(&info_tbl[i].retry_timer_id);

    if ( ( info_tbl[i].dsi_hndl =
           dsi_get_data_srvc_hndl( qcril_data_net_cb,
                                   ( void *)&dsi_cb_tbl[i] ) ) == QCRIL_DSI_HNDL_INVALID )
    {
      QCRIL_LOG_ERROR( "%s", "unable to get dsi hndl" );
      goto err_label;
    }

    /* Update the dsi_cb_tbl[] entry with the required state information */
    qcril_data_update_dsi_cb_tbl_entry(&dsi_cb_tbl[i],
                                       instance_id,
                                       modem_id,
                                       params_ptr->t,
                                       info_tbl[i].dsi_hndl,
                                       &info_tbl[i]);

    QCRIL_LOG_DEBUG("info_tbl[%d] has reserved dsi_hndl[0x%x]",
                    i, info_tbl[i].dsi_hndl);

    QCRIL_DS_LOG_DBG_MEM( "dsi_hndl", info_tbl[i].dsi_hndl );

    /* check if APN is provided  */
    if (ril_apn != NULL)
    {
      RIL_DataCallFailCause cause = PDP_FAIL_NONE;
      umts_profile_id = cdma_profile_id = 0;

  #ifdef FEATURE_QCRIL_USE_QDP
      if( FAILURE ==
          qcril_data_apn_based_profile_look_up_using_qdp( ril_apn,
                                                          ril_ipfamily,
                                                          ril_common_tech,
                                                          ril_profile,
                                                          ril_user,
                                                          ril_pass,
                                                          ril_auth_pref,
                                                          &umts_profile_id,
                                                          &info_tbl[i].qdp_3gpp_profile_pdn_type,
                                                          &cdma_profile_id,
                                                          &info_tbl[i].qdp_3gpp2_profile_pdn_type,
                                                          &cause,
                                                          &abort_call ))
      {
        QCRIL_LOG_ERROR("%s", "qcril_data_apn_based_profile_look_up failed");
      }
  #else
      qcril_data_apn_based_profile_look_up( ril_apn,
                                            &umts_profile_id,
                                            &cdma_profile_id);
  #endif

      /* Check if profile lookup failed with abort call status */
      if( abort_call )
      {
        QCRIL_LOG_ERROR("%s", "profile lookup failed with abort call status");
        info_tbl[i].call_info.inactiveReason = cause;
        goto err_label;
      }

      if(umts_profile_id > 0)
      {
        info_tbl[i].qdp_umts_profile_id = umts_profile_id;
        /* we have both 3GPP and 3GPP2 profiles */
        /* set umts profile id */
        apn_info.buf_val = NULL;
        apn_info.num_val = umts_profile_id;
        QCRIL_LOG_VERBOSE("Setting umts profile id [%d] on info_tbl [%d]",
                          umts_profile_id, i);
        if ( ( dsi_set_data_call_param( info_tbl[i].dsi_hndl ,
                                        DSI_CALL_INFO_UMTS_PROFILE_IDX,
                                        &apn_info ) ) != DSI_SUCCESS )
        {
          QCRIL_LOG_ERROR("unable to set umts profile id on info_tbl  index [%d]",
                          info_tbl[i].index );
          goto err_label;
        }
      }

      if (cdma_profile_id >0)
      {
        info_tbl[i].qdp_cdma_profile_id = cdma_profile_id;
        /* set cdma profile id */
        apn_info.buf_val = NULL;
        apn_info.num_val = cdma_profile_id;
        QCRIL_LOG_VERBOSE("Setting cdma profile id [%d] on info_tbl [%d]",
                          cdma_profile_id, i);
        if ( ( dsi_set_data_call_param( info_tbl[i].dsi_hndl ,
                                        DSI_CALL_INFO_CDMA_PROFILE_IDX,
                                        &apn_info ) ) != DSI_SUCCESS )
        {
          QCRIL_LOG_ERROR("unable to set cdma profile id on info_tbl  index [%d]",
                          info_tbl[i].index );
          goto err_label;
        }
      }

      if (cdma_profile_id ==0 || umts_profile_id == 0)
      {
        /* set APN on the dsi_hndl */
        apn_info.buf_val = (void *)ril_apn;
        apn_info.num_val = strlen( ril_apn );
        QCRIL_LOG_VERBOSE( "RIL provided APN len [%d], APN string [%s]",
                           apn_info.num_val, (char *) apn_info.buf_val );
        if ( ( dsi_set_data_call_param( info_tbl[i].dsi_hndl ,
                                        DSI_CALL_INFO_APN_NAME,
                                        &apn_info ) ) != DSI_SUCCESS )
        {
          QCRIL_LOG_ERROR( "unable to set APN, index [%d]", info_tbl[i].index );
          goto err_label;
        }
      }
    }
    else
    {
      QCRIL_LOG_VERBOSE( "%s", "RIL did not provide APN, not setting any APN in start_nw_params" );
    }

    /* use the RIL profile id only if
     * APN was not provided */
    if (ril_apn == NULL)
    {
      /* set profile id in the dsi store */
      if (ril_profile == NULL)
      {
          QCRIL_LOG_ERROR("%s", "NULL profile (params->data[1]) received");
          QCRIL_LOG_ERROR("$s", "RIL interface advises to use value 0");
          goto err_label;
      }

      if (SUCCESS != qcril_data_set_ril_profile_id(
            i,
            ril_profile,
            ril_common_tech))
      {
        QCRIL_LOG_ERROR("could not set ril profile id in info_tbl index [%d]",
                        i);
        goto err_label;
      }
    }

    /* Update dsi tech pref to cdma so that route lookup returns cdma iface */

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
    if (QCRIL_DATA_IS_RIL_RADIO_TECH_CDMA_1X_EVDO(numeric_ril_tech))
#else
    if ((ril_common_tech != NULL) && (!strcmp(ril_common_tech, QCRIL_CDMA_STRING)))
#endif
    {
      techpref_info.buf_val = NULL;
      techpref_info.num_val = DSI_RADIO_TECH_CDMA;

      if ( ( DSI_SUCCESS !=
             dsi_set_data_call_param( info_tbl[i].dsi_hndl,
                                      DSI_CALL_INFO_TECH_PREF,
                                      &techpref_info ) ) )
      {
        QCRIL_LOG_ERROR("unable to set tech pref, index [%d]", info_tbl[i].index );
        goto err_label;
      }
    }

    /* set nai */
    if (SUCCESS != qcril_data_set_nai(
          i,
          ril_user,
          ril_pass,
          ril_auth_pref))
    {
      QCRIL_LOG_ERROR("could not set ril nai in info_tbl index [%d]",
                      i);
      goto err_label;
    }

    /* set ip family (v4/v6) */
    if (ril_ipfamily != NULL && strlen(ril_ipfamily)>0)
    {
      ipfamily_info.buf_val = NULL;

      if (strcmp(ril_ipfamily, QCRIL_DATA_IP_FAMILY_V4) == 0)
      {
        ipfamily_info.num_val = DSI_IP_VERSION_4;
      }
      else if (strcmp(ril_ipfamily, QCRIL_DATA_IP_FAMILY_V6) == 0)
      {
        ipfamily_info.num_val = DSI_IP_VERSION_6;
      }
      else if (strcmp(ril_ipfamily, QCRIL_DATA_IP_FAMILY_V4V6) == 0)
      {
  #if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
        ipfamily_info.num_val = DSI_IP_VERSION_4_6;
  #else
        QCRIL_LOG_DEBUG("%s", "respond to QCRIL as data call setup FAILURE");
        qcril_data_response_setup_data_call_failure( &info_tbl[i],
                                                     instance_id,
                                                     params_ptr->t,
                                                     PDP_FAIL_ONLY_SINGLE_BEARER_ALLOWED );
        goto cleanup;
  #endif /* ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6)) */
      }
      else
      {
        QCRIL_LOG_ERROR("ril passed invalid ip family string [%s]",
                        ril_ipfamily);
        goto err_label;
      }

      if ( ( dsi_set_data_call_param( info_tbl[i].dsi_hndl ,
                                      DSI_CALL_INFO_IP_VERSION,
                                      &ipfamily_info) ) != DSI_SUCCESS )
      {
        QCRIL_LOG_ERROR("unable to set ip family, index [%d]", info_tbl[i].index );
        goto err_label;
      }

      info_tbl[i].dsi_ip_version = ipfamily_info.num_val;
    }
  #if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
    /* Default to a Dual-IP call */
    else
    {
      ipfamily_info.num_val = DSI_IP_VERSION_4_6;
      ipfamily_info.buf_val = NULL;

      if ( ( dsi_set_data_call_param( info_tbl[i].dsi_hndl ,
                                      DSI_CALL_INFO_IP_VERSION,
                                      &ipfamily_info) ) != DSI_SUCCESS )
      {
        QCRIL_LOG_ERROR("unable to set ip family, index [%d]", info_tbl[i].index );
        goto err_label;
      }

      info_tbl[i].dsi_ip_version = DSI_IP_VERSION_4_6;
    }
  #endif /* ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6)) */

  /* Insert local info tbl ref in reqlist */
  u_info.ds.info = (void *) &info_tbl[i];
  qcril_reqlist_default_entry( params_ptr->t,
                               params_ptr->event_id,
                               modem_id,
                               QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                               QCRIL_EVT_DATA_EVENT_CALLBACK,
                               &u_info,
                               &reqlist_entry );
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to insert entry to ReqList */
    QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
    return;
  }

  QCRIL_LOG_VERBOSE( "%s", "reqlist node insert complete, setup data call" );

  /* Start data call */
  if ( ( dsi_start_data_call( info_tbl[i].dsi_hndl ) ) != DSI_SUCCESS )
  {
    dsi_ip_family_t ipf = (DSI_IP_VERSION_6 == info_tbl[i].dsi_ip_version) ?
                          DSI_IP_FAMILY_V6 : DSI_IP_FAMILY_V4;

    QCRIL_LOG_ERROR( "unable to setup data call, index [%d]", info_tbl[i].index );
    QCRIL_DS_LOG_DBG_MEM( "dsi_hndl", info_tbl[i].dsi_hndl );

    if ( dsi_get_call_end_reason(info_tbl[i].dsi_hndl,
                                 &dsi_ce_reason,
                                 ipf ) == DSI_SUCCESS )
    {
      if (SUCCESS != qcril_data_get_ril_ce_code(&dsi_ce_reason,
                                                &call_end_reason))
      {
        QCRIL_LOG_DEBUG("%s","**programming err**");
        info_tbl[i].call_info.inactiveReason = PDP_FAIL_ERROR_UNSPECIFIED;
      }
      else
      {
        info_tbl[i].call_info.inactiveReason = call_end_reason;
      }
    }
    else
    {
      QCRIL_LOG_ERROR("%s","dsi_get_call_end_reason failed");
      info_tbl[i].call_info.inactiveReason = PDP_FAIL_ERROR_UNSPECIFIED;
    }

    qcril_reqlist_free( instance_id, params_ptr->t );
    goto err_label;
  }

  /* Create and start a data call setup response timer to make sure that
     we don't wait forever for the data call setup response */
  QCRIL_LOG_DEBUG("starting response timer for info_tbl [%#x]",&info_tbl[i]);
  qcril_data_start_response_timer(&info_tbl[i]);

  /* For a Dual-IP call create the partial retry timer */
  if (DSI_IP_VERSION_4_6 == ipfamily_info.num_val)
  {
    QCRIL_LOG_DEBUG("creating retry timer for info_tbl [%#x]",&info_tbl[i]);
    qcril_data_create_retry_timer(&info_tbl[i]);
  }

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
  QCRIL_LOG_DEBUG("%s", "qcril_data_request_setup_data_call: EXIT with suc");

  return;

err_label:
  QCRIL_LOG_DEBUG("%s", "respond to QCRIL with failure");
  qcril_data_response_setup_data_call_failure( &info_tbl[i],
                                               instance_id,
                                               params_ptr->t,
                                               info_tbl[i].call_info.inactiveReason );
cleanup:
  /* clean up if we had reserved an info tbl entry */
  if (i < MAX_CONCURRENT_UMTS_DATA_CALLS    &&
      VALIDATE_LOCAL_DATA_OBJ(&info_tbl[i]) &&
      info_tbl[i].pend_tok == params_ptr->t)
  {
    qcril_data_cleanup_call_state(&info_tbl[i]);
  }
err_bad_input:
  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
  QCRIL_LOG_ERROR("%s", "qcril_data_request_setup_data_call: EXIT with FAILURE");
  return;

}/* qcril_data_request_setup_data_call() */

/*===========================================================================

  FUNCTION:  qcril_data_stop_data_call

===========================================================================*/
/*!
    @brief
    Performs the steps required to stop a data call.

    @pre Before calling, info_tbl_mutex must be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
static IxErrnoType qcril_data_stop_data_call
(
  qcril_data_call_info_tbl_type *info_tbl_ptr
)
{
  IxErrnoType ret = E_FAILURE;

  QCRIL_LOG_DEBUG( "%s", "call termination starting..." );

  if (!VALIDATE_LOCAL_DATA_OBJ(info_tbl_ptr) ||
      QCRIL_DSI_HNDL_INVALID == info_tbl_ptr->dsi_hndl)
  {
    QCRIL_LOG_ERROR( "%s", "invalid info_tbl_ptr" );
    return E_INVALID_ARG;
  }

  /* Stop the data call */
  if ( ( dsi_stop_data_call( info_tbl_ptr->dsi_hndl ) ) != DSI_SUCCESS )
  {
    QCRIL_LOG_ERROR( "unable to teardown data call, index [%d]", info_tbl_ptr->index );
  }
  else
  {
    /* Removed decrement qdp ref count to avoid in_use errors from modem
       It is done after NO_NET event is received */
    ret = E_SUCCESS;
  }

  return ret;
}

/*===========================================================================

  FUNCTION:  qcril_data_request_deactivate_data_call

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_DEACTIVATE_DATA_CALL.

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_request_deactivate_data_call
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type  instance_id;
  qcril_modem_id_e_type     modem_id;
  int                       i;
  qcril_reqlist_u_type      u_info;
  qcril_reqlist_public_type reqlist_entry;
  int                       cid;
  unsigned int              intcid, pend_req = DS_RIL_REQ_INVALID;
  const char               *cid_ptr;
  const char              **in_data;

  QCRIL_LOG_DEBUG( "%s", "qcril_data_request_deact_data_call: ENTRY" );

  QCRIL_DS_ASSERT( params_ptr != NULL, "validate params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate ret_ptr" );
  if ( ( params_ptr == NULL ) || ( ret_ptr == NULL ) )
  {
    QCRIL_LOG_ERROR("%s", "BAD input");
    goto err_bad_input;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    goto err_bad_input;
  }

  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  if (modem_id >= QCRIL_MAX_MODEM_ID)
  {
    goto err_bad_input;
  }

  cid_ptr  = ((const char **)params_ptr->data)[0];

  memset( &u_info, 0, sizeof( qcril_reqlist_u_type ) );

  cid    = strtol( cid_ptr, NULL, 10 );

  QCRIL_LOG_DEBUG( "RIL says CID [%d], len [%d]",
                   cid, params_ptr->datalen );

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  for( i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
  {
    QCRIL_LOG_DEBUG("info_tbl CID [%d], index [%d]", info_tbl[i].cid, i);

    if( ( VALIDATE_LOCAL_DATA_OBJ( &info_tbl[i] ) ) && ( info_tbl[i].cid == cid ) )
    {
      QCRIL_LOG_DEBUG("found matching CID [%d], index [%d]", cid, i);
      break;
    }
  }/* for() */

  if ( i == MAX_CONCURRENT_UMTS_DATA_CALLS )
  {
    QCRIL_LOG_ERROR( "no valid CID [%d] match found", cid );
    goto err_label;
  }

  /* Check whether REQ is pending */
  if ( qcril_data_util_is_req_pending( &info_tbl[i], &pend_req ) )
  {
    QCRIL_LOG_INFO( "UNEXPECTED RIL REQ pend [%s], cid [%d], index [%d]",
                    qcril_log_lookup_event_name( pend_req ), info_tbl[i].cid, info_tbl[i].index );
    /* if deactivate request is already pending, ignore another one */
    if (pend_req == RIL_REQUEST_DEACTIVATE_DATA_CALL)
    {
        QCRIL_LOG_INFO("RIL_REQUEST_DEACTIVATE_DATA_CALL already pending, cid [%d], index [%d]",
                          info_tbl[i].cid, info_tbl[i].index);
    }
    goto err_label;
  }

  /* Stop the setup data call response timer if it is running */
  qcril_data_util_stop_timer(&info_tbl[i].timer_id);

  /* Stop the retry timer if it is running */
  qcril_data_util_stop_timer(&info_tbl[i].retry_timer_id);

  /*----------------------------------------------------------------------
    Call deactivation
  ----------------------------------------------------------------------*/
  info_tbl[i].instance_id = instance_id;
  info_tbl[i].modem_id    = modem_id;
  info_tbl[i].pend_tok    = params_ptr->t;
  info_tbl[i].pend_req    = RIL_REQUEST_DEACTIVATE_DATA_CALL;
  QCRIL_LOG_INFO( "DEBUG:  %s step %d - info_tbl_ptr->pend_tok[0x%x] info_tbl_ptr->pend_req[0x%X]",
                  __func__, 0, info_tbl[i].pend_tok, info_tbl[i].pend_req );

  /* Update the dsi_cb_tbl[] entry with the new state information */
  qcril_data_update_dsi_cb_tbl_entry(&dsi_cb_tbl[i],
                                     instance_id,
                                     modem_id,
                                     params_ptr->t,
                                     dsi_cb_tbl[i].dsi_hndl,
                                     dsi_cb_tbl[i].info_tbl_ptr);

  /* Insert local info tbl ref in reqlist */

  u_info.ds.info = ( void *) &info_tbl[i];

  qcril_reqlist_default_entry( params_ptr->t,
                               RIL_REQUEST_DEACTIVATE_DATA_CALL,
                               modem_id,
                               QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                               QCRIL_EVT_DATA_EVENT_CALLBACK,
                               &u_info,
                               &reqlist_entry );
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add entry */
    QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
    return;
  }

  /* Stop data call */
  if (E_SUCCESS != qcril_data_stop_data_call(&info_tbl[i]))
  {
    /* Cleanup call state */
    qcril_data_cleanup_call_state( &info_tbl[i] );
    goto err_label;
  }

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
  QCRIL_LOG_DEBUG("%s", "qcril_data_request_deact_data_call: EXIT with suc");
  return;

err_label:
  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
  qcril_data_response_generic_failure( instance_id, params_ptr->t, RIL_REQUEST_DEACTIVATE_DATA_CALL );
  QCRIL_LOG_DEBUG("%s", "respond to QCRIL as generic failure");

err_bad_input:
  QCRIL_LOG_ERROR("%s", "qcril_data_request_deact_data_call: EXIT with err");
  return;

} /* qcril_data_request_deactivate_data_call() */


/*===========================================================================

  FUNCTION:  qcril_data_request_last_data_call_fail_cause

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE.
    This function calls
    @return
    None.
*/
/*=========================================================================*/
void qcril_data_request_last_data_call_fail_cause
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type      instance_id;
  qcril_data_call_response_type response;

  QCRIL_LOG_VERBOSE( "%s", "qcril_data_request_last_data_call_fail_cause: ENTRY" );

  QCRIL_DS_ASSERT( params_ptr != NULL, "validate params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate ret_ptr" );
  if ( ( params_ptr == NULL ) || ( ret_ptr == NULL ) )
  {
    QCRIL_LOG_ERROR("%s", "BAD input");
    goto err_bad_input;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    goto err_bad_input;
  }

  memset( &response, 0, sizeof ( qcril_data_call_response_type ) );

  response.cause_code = last_call_end_reason;
  response.size = sizeof( response.cause_code );

  QCRIL_LOG_VERBOSE( "send cause code [%u], size [%d] ",
                     response.cause_code, response.size );

  qcril_data_response_success( instance_id,
                               params_ptr->t,
                               RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE,
                               (void *) &(response.cause_code),
                               response.size );

  QCRIL_LOG_DEBUG("%s", "qcril_data_request_last_data_call_fail_cause: EXIT with suc");
  return;

err_label:
  QCRIL_LOG_DEBUG("%s", "respond to QCRIL as generic failure");
  qcril_data_response_generic_failure( instance_id, params_ptr->t, RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE );
err_bad_input:
  QCRIL_LOG_ERROR("%s", "qcril_data_request_last_data_fail_cause: EXIT with err");
  return;

} /* qcril_data_request_last_data_call_fail_cause() */


/*===========================================================================

  FUNCTION:  qcril_data_request_data_call_list

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_DATA_CALL_LIST.

    @return
    PDP Context List
    None.
*/
/*=========================================================================*/
void qcril_data_request_data_call_list
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type      instance_id;
  qcril_data_call_response_type response;

  QCRIL_DS_ASSERT( params_ptr != NULL, "validate params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate ret_ptr" );
  if ( ( params_ptr == NULL ) || ( ret_ptr == NULL ) )
  {
    QCRIL_LOG_ERROR("%s", "BAD input");
    goto err_bad_input;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    goto err_bad_input;
  }

  memset( &response, 0, sizeof( qcril_data_call_response_type ) );

  QCRIL_LOG_DEBUG( "%s", "qcril_data_request_data_call_list: ENTRY" );

  qcril_data_get_active_call_list((void**)&(response.list), &response.size);

  QCRIL_DS_LOG_DBG_MEM( "response", response.list );

  QCRIL_LOG_DEBUG( "len [%d]", response.size );

  qcril_data_response_success( instance_id,
                               params_ptr->t,
                               RIL_REQUEST_DATA_CALL_LIST,
                               (void *) response.list,
                               response.size );

  /* free memory */
  if(NULL != response.list)
  {
    free(response.list);
  }

  QCRIL_LOG_DEBUG("%s", "qcril_data_request_data_call_list: EXIT with suc");
  return;

err_label:
  qcril_data_response_generic_failure( instance_id, params_ptr->t, RIL_REQUEST_DATA_CALL_LIST );
  QCRIL_LOG_DEBUG("%s", "respond to QCRIL as generic failure");
err_bad_input:
  QCRIL_LOG_ERROR("%s", "qcril_data_request_data_call_list: EXIT with err");
  return;

} /* qcril_data_request_data_call_list() */


/*===========================================================================

  FUNCTION:  qcril_data_request_omh_profile_info

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_DATA_CALL_PROFILE.
    On Success a list of structs containg the  OMH app type and app priorities
    is sent to RIL as response.

    @return
*/
/*=========================================================================*/

void qcril_data_request_omh_profile_info
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type      instance_id;
  qcril_data_call_response_type response;
  int i = 0;
  int rc, qmi_err_code;
  int ret = FAILURE;
  char * qmi_conn_port_omh = NULL;
  int qmi_wds_omh_hndl = QCRIL_DATA_HNDL_INVALID;
  /*get_profile_list() and query_profile() related*/
  qmi_wds_profile_tech_type profile_tech;
  int num_elements_expected = QCRIL_DATA_NUM_OMH_PROFILES_EXPECTED;
  qmi_wds_profile_list_type result_list[QCRIL_DATA_NUM_OMH_PROFILES_EXPECTED];

  unsigned long app_type;
  /*-----------------------------------------------------------------------*/
  if ( ( params_ptr == NULL ) || ( ret_ptr == NULL ) )
  {
    QCRIL_LOG_ERROR("%s", "BAD input");
    return;
  }
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  /*-----------------------------------------------------------------------*/

  memset( &response, 0, sizeof( qcril_data_call_response_type ) );

  QCRIL_LOG_DEBUG( "%s", "qcril_data_request_omh_profile_info: ENTRY" );

  /* Both RIL and QMI app types match the RUIM spec,
   * so no need of any mapping here, before calling the QMI API.
   */
  app_type = *((int *)params_ptr->data);

  QCRIL_LOG_DEBUG( "qcril_data_request_omh_profile_info: "
                         "RIL provided app type of %d",(int)app_type );

  /*get qmi wds cilent handle*/
  qmi_wds_omh_hndl = qmi_wds_srvc_init_client(default_qmi_port,
                                              NULL,
                                              NULL,
                                              &qmi_err_code);

  if (qmi_wds_omh_hndl < 0)
  {
    QCRIL_LOG_ERROR("invalid qmi_wds_omh_hndl [%d] returned. "
                    "qmi_err_code is set to [%d]",
                    qmi_wds_omh_hndl, qmi_err_code);
    qmi_wds_omh_hndl = QCRIL_DATA_HNDL_INVALID;
    goto bail;
  }

  if (QMI_NO_ERR != qmi_wds_bind_subscription ( qmi_wds_omh_hndl,
                                              global_subs_id,
                                              &qmi_err_code))
  {
    QCRIL_LOG_ERROR("qmi_wds_bind_subscription: subs_id binding failed");
  }

  QCRIL_LOG_DEBUG("qmi_wds_omh_hndl [%d] initialized. "
                  "qmi_err_code is set to [%d]", qmi_wds_omh_hndl,
                  qmi_err_code);

  memset(&result_list, 0, sizeof(result_list) );
  profile_tech = QMI_WDS_PROFILE_TECH_3GPP2;

  /* query cdma profile id. */
  rc = qmi_wds_utils_get_profile_list2( qmi_wds_omh_hndl,
                                        profile_tech,
                                        QMI_WDS_CDMA_PROFILE_APP_TYPE_PARAM_MASK,
                                        (void *)&app_type,
                                        result_list,
                                        &num_elements_expected,
                                        &qmi_err_code );

  QCRIL_LOG_DEBUG("qmi_wds_utils_get_profile_list returned [%d] " \
                    "return code for 3GPP2 technology", rc);

  if (rc != QMI_NO_ERR)
  {
    QCRIL_LOG_ERROR("qmi_wds_utils_get_profile_list failed with error [%d] " \
                    "qmi_err_code [%d]", rc, qmi_err_code);
    goto release_qmi_resources;
  }
  else
  {
    QCRIL_LOG_DEBUG("qmi_wds_utils_get_profile_list for profile_tech [%d] " \
                    "returned [%d] profile ids",
                    profile_tech, num_elements_expected);

    if (num_elements_expected == 0)
    {
      qcril_data_response_success( instance_id,
                                   params_ptr->t,
                                   RIL_REQUEST_GET_DATA_CALL_PROFILE,
                                   NULL,
                                   0);
      ret = SUCCESS;
      goto release_qmi_resources;
    }
    else if (num_elements_expected > 0)
    {
      int i;
      qmi_wds_profile_id_type prof_id;
      qmi_wds_profile_params_type prof_params;

      for (i = 0; i < num_elements_expected; i++)
      {
        prof_id.technology = QMI_WDS_PROFILE_TECH_3GPP2;
        prof_id.profile_index = result_list[i].profile_index;
        rc = qmi_wds_query_profile(qmi_wds_omh_hndl,
                                   &prof_id,
                                   &prof_params,
                                   &qmi_err_code);
        if (rc != QMI_NO_ERR)
        {
          QCRIL_LOG_ERROR("query_profile for porifile %d failed with error [%d] "
                          "qmi_err_code [%d]", prof_id.profile_index, rc, qmi_err_code);
          goto release_qmi_resources;
        }
        else
        {
          if (prof_params.cdma_profile_params.param_mask
                                    & QMI_WDS_CDMA_PROFILE_APP_PRIORITY_PARAM_MASK)
          {
            response.omh_profile_info.profile_info[response.
              omh_profile_info.num_profiles].profileId =
              result_list[i].profile_index;
            response.omh_profile_info.profile_info[response.
              omh_profile_info.num_profiles].priority =
              prof_params.cdma_profile_params.app_priority;
            response.omh_profile_info.num_profiles++;
          }
          else
          {
            QCRIL_LOG_ERROR("App priority is not set in the profile %d",
                                                    result_list[i].profile_index);
            continue;

          }
        }
      }/*for*/

      /*Response is already populated. Now send it.*/
      response.size = sizeof(response.omh_profile_info);

      qcril_data_response_success( instance_id,
                                   params_ptr->t,
                                   RIL_REQUEST_GET_DATA_CALL_PROFILE,
                                   (void *) ( &(response.omh_profile_info) ),
                                   response.size );

    }/*else-if*/
  }/*else*/

  ret = SUCCESS;

release_qmi_resources:
  if (qmi_wds_omh_hndl > 0)
  {
    rc = qmi_wds_srvc_release_client(qmi_wds_omh_hndl, &qmi_err_code);
    if (rc != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("couldn't release qmi wds hndl [%d] " \
                      "return code [%d], error code [%d]",
                      qmi_wds_omh_hndl, rc, qmi_err_code);
    }
  }

bail:
  if (ret == SUCCESS)
  {
    QCRIL_LOG_DEBUG("%s", "qcril_data_request_omh_profile_info: EXIT with success");
  }
  else if (ret == FAILURE)
  {
    qcril_data_response_generic_failure(instance_id,
                                        params_ptr->t,
                                        RIL_REQUEST_GET_DATA_CALL_PROFILE);

    QCRIL_LOG_DEBUG("%s", "respond to RIL with generic failure \n");
    QCRIL_LOG_ERROR("%s", "qcril_data_request_omh_profile_info: EXIT with err \n");
  }
  return;
}

/*===========================================================================

    FUNCTION:  qcril_data_request_set_data_profile

    ===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SET_DATA_PROFILE

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/

void qcril_data_request_set_data_profile
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{

#define IMS_CLASS 1
#define ADMN_CLASS 2
#define INET_CLASS 3
#define APP_CLASS 4

  const char               *ril_apn;
  const char               *ril_user;
  const char               *ril_pass;
  int                       auth_type;
  const char               *ril_tech = NULL;
  const char               *ril_common_tech = NULL;
  const char               *ril_profile = NULL;
  const char               *ril_ipfamily = NULL;
  const char               *ril_auth_type = NULL;
  int                       ril_maxConnstime;
  int                       ril_waitTime;
  int                       ril_maxConns;
  int                       ril_enabled;
  int                       i=0,max_profiles =0, rc;
  int                      umts_profile_id;
  int                      cdma_profile_id;
  int                      ril_user_len =0;
  int                      ril_pass_len =0;

  qcril_request_resp_params_type    resp;

  /* stores QDP profile PDN types */
  qdp_profile_pdn_type     umts_profile_pdn_type;
  qdp_profile_pdn_type     cdma_profile_pdn_type;
  qdp_error_info_t         error_info;
  int                      qmi_err_code;
  uint8 apn_class;

  qcril_instance_id_e_type  instance_id;
  qcril_modem_id_e_type     modem_id;

  const char * qdp_params[QDP_RIL_PARAM_MAX];

  qmi_wds_profile_params_type *p_params = NULL;
  qmi_wds_profile_params_type *new_p_params = NULL;
  qdp_tech_t                  tech_type;
  qmi_wds_pdp_type            pdp_type;

  QCRIL_LOG_DEBUG( "%s", "qcril_data_request_set_data_profile: ENTRY" );

  QCRIL_DS_ASSERT( params_ptr != NULL, "validate params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr != NULL, "validate ret_ptr" );


  if ( ( params_ptr == NULL ) || ( ret_ptr == NULL ) )
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p], datalen [%d]",
            (unsigned int *)params_ptr, (unsigned int *)ret_ptr,
            params_ptr ? params_ptr->datalen : 0 );
    goto err_bad_input;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    goto err_bad_input;
  }

  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  if (modem_id >= QCRIL_MAX_MODEM_ID)
  {
    goto err_bad_input;
  }

  p_params = malloc(sizeof(qmi_wds_profile_params_type));

  if(p_params == NULL)
  {
    QCRIL_LOG_ERROR( "Not Enuf Memory");
    goto err_bad_input;
  }

/*
typedef struct {
    int profileId;
    char* apn;
    char* protocol;
    int authType;
    char* user;
    char* password;
    int type;
    int maxConnsTime;
    int maxConns;
    int waitTime;
    int enabled;
} RIL_DataProfileInfo;
*/

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  RIL_DataProfileInfo **data_profile_arr = (RIL_DataProfileInfo **)params_ptr->data;


  QCRIL_LOG_DEBUG("params_ptr->datalen [%d] ",params_ptr->datalen);

  do
  {

    RIL_DataProfileInfo *data_profile_info = data_profile_arr[i];

    if (NULL == data_profile_info)
    {
      QCRIL_LOG_ERROR("%s","Invalid Input");
      goto err_bad_input;
    }

    /* Convert Profile ID to APN class as we expect the same from Telephony */
    switch(data_profile_info->profileId)
    {
      case 0:
       apn_class = INET_CLASS;
       break;
      case 2:
       apn_class = IMS_CLASS;
       break;
      case 3:
       apn_class = ADMN_CLASS;
       break;
      case 4:
       apn_class = APP_CLASS;
       break;
      default:
        QCRIL_LOG_ERROR("%s","Invalid Input");
        goto err_bad_input;
    }

    ril_apn       = data_profile_info->apn;
    ril_ipfamily  = data_profile_info->protocol;
    auth_type     = data_profile_info->authType;
    ril_user      = data_profile_info->user;
    ril_pass      = data_profile_info->password;

    ril_maxConns  = data_profile_info->maxConns;
    ril_maxConnstime = data_profile_info->maxConnsTime;
    ril_waitTime = data_profile_info->waitTime;
    ril_enabled = data_profile_info->enabled;

    memset(qdp_params, 0, sizeof(qdp_params));

    if (NULL == ril_apn || 0 == strlen(ril_apn))
    {
      QCRIL_LOG_ERROR("%s","ril_apn provided is NULL");
    }
    else if (strlen(ril_apn) > (QMI_WDS_MAX_APN_STR_SIZE-1))
    {
      QCRIL_LOG_ERROR("RIL provided invalid APN [%s] "
                      "APN length [%d] exceeds max allowed [%d]",
                      ril_apn, strlen(ril_apn), QMI_WDS_MAX_APN_STR_SIZE-1);
      goto err_bad_input;
    }
    qdp_params[QDP_RIL_APN] = ril_apn;

    QCRIL_LOG_DEBUG("qdp_param APN = [%s]", qdp_params[QDP_RIL_APN]);

    /* prepare tech */
    qdp_params[QDP_RIL_TECH] = QDP_RIL_3GPP;

    /* prepare ip family */
    if (ril_ipfamily != NULL && strlen(ril_ipfamily) > 0)
    {
      if (strlen(ril_ipfamily) > QCRIL_IP_FAMILY_STR_MAX)
      {
        QCRIL_LOG_ERROR("RIL provided invalid ip family [%s] "
                        "ip family length [%d] exceeds max allowed [%d]",
                        ril_ipfamily, strlen(ril_ipfamily), QCRIL_IP_FAMILY_STR_MAX);
        goto err_bad_input;;
      }
      qdp_params[QDP_RIL_IP_FAMILY] = ril_ipfamily;

      if (strncasecmp(
            ril_ipfamily,
            "IP",
            std_strlen("IP")+1) == 0)
      {
        pdp_type = QMI_WDS_PDP_TYPE_IP;
      }
      else if (strncasecmp(
                 ril_ipfamily,
                 "IPV6",
                std_strlen("IPV6")+1) == 0)
      {
        pdp_type = QMI_WDS_PDP_TYPE_IPV6;
      }
      else if (strncasecmp(
                 ril_ipfamily,
                 "IPV4V6",
                std_strlen("IPV4V6")+1) == 0)
      {
        pdp_type = QMI_WDS_PDP_TYPE_IPV4V6;
      }

      QCRIL_LOG_DEBUG("qdp param IP_FAMILY = [%s]", qdp_params[QDP_RIL_IP_FAMILY]);
    }

    /* prepare auth */
    switch (auth_type)
    {
    case QMI_WDS_AUTH_PREF_PAP_CHAP_NOT_ALLOWED:
        ril_auth_type = QDP_RIL_PAP_CHAP_NOT_ALLOWED;
        break;
    case QMI_WDS_AUTH_PREF_PAP_ONLY_ALLOWED:
        ril_auth_type = QDP_RIL_PAP_ONLY_ALLOWED;
        break;
    case QMI_WDS_AUTH_PREF_CHAP_ONLY_ALLOWED:
        ril_auth_type = QDP_RIL_CHAP_ONLY_ALLOWED;
        break;
    case QMI_WDS_AUTH_PREF_PAP_CHAP_BOTH_ALLOWED:
        ril_auth_type = QDP_RIL_PAP_CHAP_BOTH_ALLOWED;
        break;
    default:
        QCRIL_LOG_DEBUG("Unknown auth_type [%d]",
                        auth_type);
        break;
    }

    qdp_params[QDP_RIL_AUTH] = ril_auth_type;
    QCRIL_LOG_DEBUG("qdp param RIL_AUTH = [%s]", qdp_params[QDP_RIL_AUTH]);

    /* Prepare username */
    if (ril_user != NULL && strlen(ril_user) > 0)
    {
      if (strlen(ril_user) > QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1)
      {
        QCRIL_LOG_ERROR("RIL provided username exceeds max allowed [%d]",
                        QMI_WDS_MAX_USERNAME_PASS_STR_SIZE);
      }
      else
      {
        qdp_params[QDP_RIL_NAI] = ril_user;
        ril_user_len = strlen(ril_user)+1;
        QCRIL_LOG_DEBUG("qdp param USERNAME = [%s]", qdp_params[QDP_RIL_NAI]);
      }
    }

    /* prepare password */
    if (ril_pass != NULL && strlen(ril_pass) > 0)
    {
      if (strlen(ril_pass) > QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1)
      {
        QCRIL_LOG_ERROR("RIL provided password exceeds max allowed [%d]",
                        QMI_WDS_MAX_USERNAME_PASS_STR_SIZE);
      }
      else
      {
        qdp_params[QDP_RIL_PASSWORD] = ril_pass;
        ril_pass_len = strlen(ril_pass)+1;
        QCRIL_LOG_DEBUG("qdp param PASSWORD = [%s]", qdp_params[QDP_RIL_PASSWORD]);
      }
    }

    umts_profile_id = cdma_profile_id = 0;

    memset(p_params ,0x0, sizeof(qmi_wds_profile_params_type));

    /* Look up for a profile that matches APN class */
    rc = qdp_profile_look_up_by_param( qdp_params,
                                       QDP_RIL_CLASS,
                                       apn_class,
                                       (unsigned int *)&umts_profile_id,
                                       &umts_profile_pdn_type,
                                       (unsigned int *)&cdma_profile_id,
                                       &cdma_profile_pdn_type,
                                       p_params,
                                       &tech_type,
                                       &error_info );

     /* Currently support for 3GPP tech */
     if(umts_profile_id != QDP_INVALID_PROFILE)
     {
        new_p_params =  malloc(sizeof(qmi_wds_profile_params_type));

        if(new_p_params == NULL)
        {
          QCRIL_LOG_ERROR( "Not Enuf Memory");
          goto err_bad_input;
        }

        memset(new_p_params ,0x0, sizeof(qmi_wds_profile_params_type));

        if(tech_type == QDP_3GPP)
        {
          new_p_params->umts_profile_params.param_mask =
                                                  QMI_WDS_UMTS_PROFILE_APN_NAME_PARAM_MASK|
                                                  QMI_WDS_UMTS_PROFILE_USERNAME_PARAM_MASK|
                                                  QMI_WDS_UMTS_PROFILE_PASSWORD_PARAM_MASK|
                                                  QMI_WDS_UMTS_PROFILE_AUTH_PREF_PARAM_MASK|
                                                  QMI_WDS_UMTS_PROFILE_PDP_TYPE_PARAM_MASK|
                                                  QMI_WDS_UMTS_PROFILE_APN_CLASS_PARAM_MASK|
                                                  QMI_WDS_UMTS_PDN_REQ_WAIT_INTERVAL|
                                                  QMI_WDS_UMTS_MAX_PDN_CONN_TIMER|
                                                  QMI_WDS_UMTS_MAX_PDN_CONN_PER_BLOCK|
                                                  QMI_WDS_UMTS_PROFILE_APN_DISABLED_FLAG_PARAM_MASK;


          memcpy(new_p_params->umts_profile_params.apn_name,ril_apn,strlen(ril_apn));
          memcpy(new_p_params->umts_profile_params.username,ril_user,strlen(ril_user));
          memcpy(new_p_params->umts_profile_params.password,ril_pass,strlen(ril_pass));

          new_p_params->umts_profile_params.pdp_type = pdp_type;
          new_p_params->umts_profile_params.auth_pref = auth_type;

          new_p_params->umts_profile_params.apn_class = apn_class;
          new_p_params->umts_profile_params.max_pdn_conn_timer = ril_maxConnstime;
          new_p_params->umts_profile_params.max_pdn_conn_per_block = ril_maxConns;
          new_p_params->umts_profile_params.pdn_req_wait_interval = ril_waitTime;
          new_p_params->umts_profile_params.apn_disabled_flag = (ril_enabled == 0)? 1:0;

          qdp_3gpp_profile_update_ex( new_p_params,
                                       umts_profile_id,
                                       &qmi_err_code);
        }
        else if (tech_type == QDP_EPC)
        {
          new_p_params->epc_profile_params.param_mask =
                                                       QMI_WDS_EPC_PROFILE_APN_NAME_PARAM_MASK|
                                                       QMI_WDS_EPC_PROFILE_USERNAME_PARAM_MASK|
                                                       QMI_WDS_EPC_PROFILE_PASSWORD_PARAM_MASK|
                                                       QMI_WDS_EPC_UMTS_PROFILE_PDP_TYPE_PARAM_MASK|
                                                       QMI_WDS_EPC_UMTS_PROFILE_AUTH_PREF_PARAM_MASK|
                                                       QMI_WDS_EPC_COMMON_APN_CLASS_PARAM_MASK|
                                                       QMI_WDS_EPC_UMTS_PDN_REQ_WAIT_INTERVAL|
                                                       QMI_WDS_EPC_UMTS_MAX_PDN_CONN_TIMER|
                                                       QMI_WDS_EPC_UMTS_MAX_PDN_CONN_PER_BLOCK|
                                                       QMI_WDS_EPC_COMMON_APN_DISABLED_PARAM_MASK;


          memcpy(new_p_params->epc_profile_params.apn_name,ril_apn,strlen(ril_apn));
          memcpy(new_p_params->epc_profile_params.username,ril_user,strlen(ril_user));
          memcpy(new_p_params->epc_profile_params.password,ril_pass,strlen(ril_pass));

          new_p_params->epc_profile_params.pdp_type = pdp_type;
          new_p_params->epc_profile_params.auth_pref = auth_type;

          new_p_params->epc_profile_params.common_apn_class = apn_class;
          new_p_params->epc_profile_params.max_pdn_conn_timer = ril_maxConnstime;
          new_p_params->epc_profile_params.max_pdn_conn_per_block = ril_maxConns;
          new_p_params->epc_profile_params.pdn_req_wait_interval = ril_waitTime;
          new_p_params->epc_profile_params.common_apn_disabled_flag = (ril_enabled == 0)? 1:0;

          qdp_epc_profile_update_ex( new_p_params,
                                       umts_profile_id,
                                       &qmi_err_code);

        }
        free(new_p_params);

     }

    i++;

  }while(i < (params_ptr->datalen/sizeof(RIL_DataProfileInfo *)));

  if(p_params != NULL)
  {
    free(p_params);
  }

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

  QCRIL_LOG_DEBUG("%s", "qcril_data_request_setup_data_call: EXIT with SUCCESS");

  qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                           params_ptr->t,
                                           params_ptr->event_id,
                                           RIL_E_SUCCESS,
                                           &resp );

  return qcril_send_request_response( &resp );


err_bad_input:

  if(p_params != NULL)
  {
    free(p_params);
  }

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

  QCRIL_LOG_ERROR("%s", "qcril_data_request_setup_data_call: EXIT with FAILURE");

    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                           params_ptr->t,
                                           params_ptr->event_id,
                                           RIL_E_GENERIC_FAILURE,
                                           &resp );

    return qcril_send_request_response( &resp );

}


/*===========================================================================

  FUNCTION:  qcril_data_qmi_wds_init

===========================================================================*/
/*!
    @brief
    Initialize the QMI WDS and DSD services on a well known port for receiving
    a set of events

    @return
    none
*/
/*=========================================================================*/
void qcril_data_qmi_wds_init(boolean from_ssr)
{
  int rc, qmi_err_code;
  qmi_wds_event_report_params_type event_report_params;
  qmi_wds_pref_data_sys_type       cur_pref_sys;
  qcril_arb_pref_data_tech_e_type  qcril_arb_tech;
  int ret = DSI_ERROR;
  qmi_idl_service_object_type dsd_svc_obj;
  qcril_modem_stack_id_e_type stack_id;
  char args[PROPERTY_VALUE_MAX];

  QCRIL_LOG_DEBUG("%s","qcril_data_qmi_wds_init: entry");

  /* Fetch subs_id again during SSR initialization*/
  if(from_ssr)
  {
    global_instance_id = qmi_ril_get_process_instance_id();

    dsi_set_ril_instance(global_instance_id);

    stack_id = qmi_ril_get_stack_id(global_instance_id);

    switch(stack_id)
    {
      case QCRIL_MODEM_PRIMARY_STACK_ID:
        global_subs_id = QMI_WDS_PRIMARY_SUBS;
        break;

      case QCRIL_MODEM_SECONDARY_STACK_ID:
        global_subs_id = QMI_WDS_SECONDARY_SUBS;
        break;

      case QCRIL_MODEM_TERTIARY_STACK_ID:
        global_subs_id = QMI_WDS_TERTIARY_SUBS;
        break;

      default:
        global_subs_id = QMI_WDS_DEFAULT_SUBS;
    }

    QCRIL_LOG_DEBUG( "qcril_data_qmi_wds_init: stack_id: %d, subs_id: %d",
                                              stack_id,global_subs_id);

    dsi_set_modem_subs_id(global_subs_id);
  }

  do
  {
    if (DSI_SUCCESS != dsi_init(DSI_MODE_GENERAL))
    {
      QCRIL_LOG_ERROR("dsi_init failed");
    }

    global_qmi_wds_hndl = QCRIL_DATA_HNDL_INVALID;
    memset( &event_report_params, 0x0, sizeof(event_report_params) );

    /* initialize data_sys_status, it is used by each indication of
       preferred system change and freed in qcril_data_qmi_wds_release */
    memset( &global_dsd_info.legacy.data_sys_status,
            0x00,
            sizeof(global_dsd_info.legacy.data_sys_status));

    global_dsd_info.legacy.data_sys_status.network_info_len = QCRIL_NETWORK_INFO_LEN;
    global_dsd_info.legacy.dsd_mode = QCRIL_DATA_LEGACY_DSD_MODE_UNKNOWN;

    global_dsd_info.legacy.data_sys_status.network_info = malloc(
      sizeof(qmi_wds_data_sys_status_network_info_type)*
      global_dsd_info.legacy.data_sys_status.network_info_len);

    if(NULL == global_dsd_info.legacy.data_sys_status.network_info)
    {
      QCRIL_LOG_ERROR("%s", "Failed to allocate memory for network_info");
      global_dsd_info.legacy.data_sys_status.network_info_len = 0;
      break;
    }

    /*Initialize QCCI dsd client to send recieve mesages */
    dsd_svc_obj = dsd_get_service_object_v01();
    if (dsd_svc_obj == NULL)
    {
      QCRIL_LOG_ERROR("dsd_get_service_object failed ");
      break;
    }

    global_qmi_wds_hndl = qmi_wds_srvc_init_client(default_qmi_port,
                                                   qcril_data_qmi_wds_ind_cb,
                                                   NULL,
                                                   &qmi_err_code);

    if (global_qmi_wds_hndl < 0)
    {
      QCRIL_LOG_ERROR("invalid qmi_wds_hndl [%d] returned. "
                      "qmi_err_code is set to [%d]",
                      global_qmi_wds_hndl, qmi_err_code);
      global_qmi_wds_hndl = QCRIL_DATA_HNDL_INVALID;
      break;
    }

    if (QMI_NO_ERR != qmi_wds_bind_subscription ( global_qmi_wds_hndl,
                                              global_subs_id,
                                              &qmi_err_code))
    {
      QCRIL_LOG_ERROR("qmi_wds_bind_subscription: subs_id binding failed");
    }

    QCRIL_LOG_DEBUG("global qmi_wds_hndl [%d] initialized. "
                    "qmi_err_code is set to [%d]", global_qmi_wds_hndl,
                    qmi_err_code);


    /* Call flow for data registration indications:
     *
     * If persist.radio.dont_use.dsd is TRUE
     *   Use NAS indications
     * Else if qmi_dsd_service_supported is TRUE
     *   Use QMI-DSD indication
     * Else if query_current_data_system is TRUE
     *   Use Current data system indication
     * Else if query_preferred_data_system is TRUE
     *   Use Preferred data system indications
     * Else
     *   Use NAS indications
     */
    if (TRUE == qcril_data_dont_use_dsd())
    {
      QCRIL_LOG_DEBUG("%s", "ADB override for persist.radio.dont_use_dsd, using NAS indications");
      qcril_arb_set_pref_data_tech(global_instance_id, QCRIL_ARB_PREF_DATA_TECH_INVALID);
      global_dsd_info.service = QCRIL_DATA_DSD_SERVICE_TYPE_LEGACY;
      global_dsd_info.legacy.dsd_mode = QCRIL_DATA_LEGACY_DSD_MODE_NAS;
    }
    else if ( QMI_NO_ERR == qmi_client_init_instance(dsd_svc_obj,
                                       default_qmi_instance,
                                       (qmi_client_ind_cb) qcril_data_qmi_dsd_ind_cb,
                                       NULL,
                                       &dsd_os_params,
                                       QCRIL_QMI_DSD_TIMEOUT,
                                       &global_qmi_dsd_hndl))
    {
      dsd_bind_subscription_req_msg_v01  bind_req_msg;
      dsd_bind_subscription_resp_msg_v01  bind_resp_msg;

      dsd_get_system_status_resp_msg_v01  sys_resp_msg;
      dsd_system_status_ind_msg_v01 *ind_data = NULL;

      dsd_system_status_change_req_msg_v01  sys_reg_req_msg;
      dsd_system_status_change_resp_msg_v01 sys_reg_resp_msg;

      QCRIL_LOG_DEBUG("%s", "Modem supports QMI-DSD service, using QMI-DSD indications");

      global_dsd_info.service = QCRIL_DATA_DSD_SERVICE_TYPE_QMI_DSD;

      /* Bind the subscription */
      memset(&bind_req_msg, 0, sizeof(bind_req_msg));
      memset(&bind_resp_msg, 0, sizeof(bind_resp_msg));

      bind_req_msg.bind_subs = qcril_data_get_qmi_dsd_subscription_id();

      QCRIL_LOG_DEBUG("binding subscription to subs=%d",
                      bind_req_msg.bind_subs);

      rc = qmi_client_send_msg_sync_with_shm(global_qmi_dsd_hndl,
                                    QMI_DSD_BIND_SUBSCRIPTION_REQ_V01,
                                    &bind_req_msg,
                                    sizeof(bind_req_msg),
                                    &bind_resp_msg,
                                    sizeof(bind_resp_msg),
                                    QCRIL_DATA_QMI_DSD_SYNC_MSG_TIMEOUT);

     if(qmi_ril_is_multi_sim_feature_supported())
     {
       if ((QMI_NO_ERR != rc) ||
            (QMI_NO_ERR != bind_resp_msg.resp.result))
        {
          QCRIL_LOG_ERROR("failed to bind subscription, rc err=%d, resp_error=%d",
                            rc,bind_resp_msg.resp.error);

          qmi_client_release( global_qmi_dsd_hndl );
          global_qmi_dsd_hndl = NULL;

          QCRIL_LOG_DEBUG("%s", "Subs binding failed for Multisim, using NAS indications");

          qcril_arb_set_pref_data_tech(global_instance_id, QCRIL_ARB_PREF_DATA_TECH_INVALID);
          global_dsd_info.service = QCRIL_DATA_DSD_SERVICE_TYPE_LEGACY;
          global_dsd_info.legacy.dsd_mode = QCRIL_DATA_LEGACY_DSD_MODE_NAS;

          goto continue_init;

        }
      }

      /* Query the current system status from QMI-DSD service */
      memset(&sys_resp_msg, 0, sizeof(sys_resp_msg));

      rc = qmi_client_send_msg_sync_with_shm(global_qmi_dsd_hndl,
                                    QMI_DSD_GET_SYSTEM_STATUS_REQ_V01,
                                    NULL,
                                    0,
                                    &sys_resp_msg,
                                    sizeof(sys_resp_msg),
                                    QCRIL_DATA_QMI_DSD_SYNC_MSG_TIMEOUT);

      if (QMI_NO_ERR != rc) {
        QCRIL_LOG_ERROR("failed to send qmi_dsd_get_system_status, err=%d",
                        rc);
        break;
      }
      else if (QMI_NO_ERR != sys_resp_msg.resp.result) {
        QCRIL_LOG_ERROR("failed to send qmi_dsd_get_system_status, err=%d",
                        sys_resp_msg.resp.error);
        break;
      }

      /* Process the system status response */
      ind_data = (dsd_system_status_ind_msg_v01 *)((char *)&sys_resp_msg +
                                                   offsetof(dsd_get_system_status_resp_msg_v01,
                                                            avail_sys_valid));
      qcril_data_process_qmi_dsd_ind(ind_data);

      /* Register for the QMI-DSD indications */
      memset(&sys_reg_req_msg, 0, sizeof(sys_reg_req_msg));
      memset(&sys_reg_resp_msg, 0, sizeof(sys_reg_resp_msg));

      rc = qmi_client_send_msg_sync_with_shm(global_qmi_dsd_hndl,
                                    QMI_DSD_SYSTEM_STATUS_CHANGE_REQ_V01,
                                    &sys_reg_req_msg,
                                    sizeof(sys_reg_req_msg),
                                    &sys_reg_resp_msg,
                                    sizeof(sys_reg_resp_msg),
                                    QCRIL_DATA_QMI_DSD_SYNC_MSG_TIMEOUT);

      if (QMI_NO_ERR != rc) {
        QCRIL_LOG_ERROR("failed to send qmi_dsd_reg_system_status_ind, err=%d",
                        rc);
        break;
      }
      else if (QMI_NO_ERR != sys_reg_resp_msg.resp.result) {
        QCRIL_LOG_ERROR("failed to send qmi_dsd_reg_system_status_ind, err=%d",
                        sys_reg_resp_msg.resp.error);
        break;
      }
    }
    else if (QMI_NO_ERR ==
        qmi_wds_get_current_data_system_status(global_qmi_wds_hndl,
                                               &(global_dsd_info.legacy.data_sys_status.pref_network),
                                               global_dsd_info.legacy.data_sys_status.network_info,
                                               &(global_dsd_info.legacy.data_sys_status.network_info_len),
                                               &qmi_err_code))
    {
      QCRIL_LOG_DEBUG("%s", "Modem supports current data system, using current data system indications");
      qcril_data_process_data_sys_status_ind(&global_dsd_info.legacy.data_sys_status);
      global_dsd_info.service = QCRIL_DATA_DSD_SERVICE_TYPE_LEGACY;
      global_dsd_info.legacy.dsd_mode = QCRIL_DATA_LEGACY_DSD_MODE_DATA_SYS_STATUS;

      /* Register for current data system status indication */
      event_report_params.param_mask |= QMI_WDS_EVENT_DATA_SYS_STATUS_IND;
      event_report_params.report_data_sys_status = TRUE;
    }
    else if (QMI_NO_ERR ==
        qmi_wds_get_pref_data_system(global_qmi_wds_hndl,
                                     &cur_pref_sys,
                                     &qmi_err_code))
    {
      QCRIL_LOG_DEBUG("%s", "Modem supports preferred data system, using preferred data system indications");
      qcril_data_process_pref_tech_change_ind(cur_pref_sys);
      global_dsd_info.service = QCRIL_DATA_DSD_SERVICE_TYPE_LEGACY;
      global_dsd_info.legacy.dsd_mode = QCRIL_DATA_LEGACY_DSD_MODE_PREF_DATA_TECH;
      global_dsd_info.legacy.pref_data_tech = cur_pref_sys;

      /* Register for preferred data system indications */
      event_report_params.param_mask |= QMI_WDS_EVENT_PREF_DATA_SYS_IND;
      event_report_params.report_pref_data_sys = TRUE;
    }
    /* Fallback to the legacy behavior */
    else
    {
      QCRIL_LOG_DEBUG("%s", "Modem does not support current/preferred data system, fallback to NAS indications");
      qcril_arb_set_pref_data_tech(global_instance_id,
                                   QCRIL_ARB_PREF_DATA_TECH_INVALID);
      global_dsd_info.service = QCRIL_DATA_DSD_SERVICE_TYPE_LEGACY;
      global_dsd_info.legacy.dsd_mode = QCRIL_DATA_LEGACY_DSD_MODE_NAS;
    }

    /* Register for data call status indications */
    event_report_params.param_mask |= QMI_WDS_EVENT_DATA_CALL_STATUS_CHG_IND;
    event_report_params.report_data_call_status_chg = TRUE;

    rc = qmi_wds_set_event_report(global_qmi_wds_hndl,
                                   &event_report_params,
                                   &qmi_err_code);

    if (rc != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("qmi_wds_set_event_report failed with err [%d][%d]",
                       rc, qmi_err_code);
      break;
    }


continue_init:

#ifdef FEATURE_QCRIL_USE_QDP
    /* init qdp */
    qdp_init(default_qmi_port);

    memset(args, 0, sizeof(args));

    property_get("persist.data.dont_use_epc", args, "true");

    if (0 == strcmp(args, "true"))
    {
      QCRIL_LOG_DEBUG("%s", "persist.data.dont_use_epc = true");

      epc_profile_supported = FALSE;
    }

    qdp_set_subscription(global_subs_id);

#endif

    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    QCRIL_LOG_DEBUG("%s","qcril_data_qmi_wds_init: exit with success");
  }
  else
  {
    QCRIL_LOG_ERROR("%s","qcril_data_qmi_wds_init: exit with error");
  }
}

/*===========================================================================

  FUNCTION:  qcril_data_qmi_wds_release

===========================================================================*/
/*!
    @brief
    releases QMI WDS resources.
    only used by QMI RIL and only during SSR

    @return
    none
*/
/*=========================================================================*/
void qcril_data_qmi_wds_release(void)
{
  int qmi_err_code = QMI_NO_ERR;

  QCRIL_LOG_DEBUG("%s", "qcril_data_qmi_wds_release: ENTRY");

  /* Release the DSI library */
  if (DSI_SUCCESS != dsi_release(DSI_MODE_GENERAL))
  {
    QCRIL_LOG_ERROR("dsi_release failed");
  }

  qcril_arb_set_pref_data_tech( global_instance_id, QCRIL_ARB_PREF_DATA_TECH_INVALID );
  if ( QCRIL_DATA_HNDL_INVALID != global_qmi_wds_hndl )
  {
    qmi_wds_srvc_release_client( global_qmi_wds_hndl, &qmi_err_code );
    global_qmi_wds_hndl = QCRIL_DATA_HNDL_INVALID;
    QCRIL_LOG_DEBUG( ".. WDS handle released, error code %d ", (int)qmi_err_code );
  }

  if ( NULL != global_qmi_dsd_hndl )
  {
    qmi_err_code = qmi_client_release( global_qmi_dsd_hndl );
    global_qmi_dsd_hndl = NULL;
    QCRIL_LOG_DEBUG( ".. DSD handle released, error code %d ", (int)qmi_err_code );
  }

#ifdef RIL_REQUEST_SET_INITIAL_ATTACH_APN
  /* Try and release attach profile in case of SSR */
  if (QDP_INVALID_PROFILE != global_lte_attach_profile)
  {
    if ( QDP_SUCCESS != qdp_profile_release(global_lte_attach_profile) )
    {
      QCRIL_LOG_ERROR("%s", "LTE attach profile release failed because either the "
                      "profile is a modem profile or there was an internal "
                      "error in qdp");
    }

    global_lte_attach_profile = QDP_INVALID_PROFILE;
  }
#endif

  QCRIL_LOG_DEBUG("%s", "releasing QDP resources");
  qdp_deinit();

  if(NULL != global_dsd_info.legacy.data_sys_status.network_info)
  {
    qcril_free(global_dsd_info.legacy.data_sys_status.network_info);
    global_dsd_info.legacy.data_sys_status.network_info = NULL;
  }

  QCRIL_LOG_DEBUG("%s", "qcril_data_qmi_wds_release: EXIT");
}

/*===========================================================================

  FUNCTION:  qcril_data_find_default_port

===========================================================================*/
/*!
    @brief
    Finds and sets default QMI port to be used.

    @return
    none
*/
/*=========================================================================*/
void qcril_data_set_default_port()
{
  int ret = 0;
  ds_target_t target;
  const char *target_str;

  target = ds_get_target();
  target_str = ds_get_target_str(target);

  QCRIL_LOG_DEBUG("qcril_data_set_default_port(): target: [%d]: [%s]", target, target_str);

  do
  {
    if (DS_TARGET_MSM == target ||
        DS_TARGET_MSM8994 == target)
    {
      /* use smd/bam port */
      default_qmi_port = QMI_PORT_RMNET_0;
      default_qmi_instance = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;  // local modem
      QCRIL_LOG_DEBUG("default_qmi_port set to %s, default_qmi_instance \
                       set to %d", default_qmi_port, default_qmi_instance);
      global_modem_id = QCRIL_DEFAULT_MODEM_ID;
      ignore_ril_tech = TRUE;
    }
    else if (DS_TARGET_SVLTE1 == target ||
             DS_TARGET_SVLTE2 == target ||
             DS_TARGET_CSFB == target)
    {
      if (DS_TARGET_SVLTE2 == target)
      {
        /* ignore ril technology if svlte2 is the target */
        /* in nutshell, we are required to put this hack for certain
         * targets where 3gpp to 3gpp2 handover is possible, and hence
         * preferred ril technology shall be ignored for the profile
         * look up purposes (i.e. both 3gpp and 3gpp2 profiles shall
         * be looked up together regardless of the preferred technology */
        ignore_ril_tech = TRUE;
      }
      /* use sdio port */
      default_qmi_port = QMI_PORT_RMNET_SDIO_0;
      default_qmi_instance = QMI_CLIENT_QMUX_RMNET_SDIO_INSTANCE_0;
      QCRIL_LOG_DEBUG("default_qmi_port set to %s, default_qmi_instance \
                       set to %d", default_qmi_port, default_qmi_instance);
      global_modem_id = QCRIL_SECOND_MODEM_ID;
    }
    else if (DS_TARGET_MDM == target)
    {
      ignore_ril_tech = TRUE; /* see above */

      /* use usb port */
      default_qmi_port = QMI_PORT_RMNET_USB_0;
      default_qmi_instance = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
      strlcpy( qcril_data_modem_port_map_tbl[QCRIL_SECOND_MODEM_ID].qmi_port,
               QMI_PORT_RMNET_USB_0,
               sizeof(qcril_data_modem_port_map_tbl[QCRIL_SECOND_MODEM_ID].qmi_port));
      QCRIL_LOG_DEBUG("default_qmi_port set to %s, default_qmi_instance \
                       set to %d", default_qmi_port, default_qmi_instance);
      global_modem_id = QCRIL_DEFAULT_MODEM_ID;
    }
    else if (DS_TARGET_FUSION4_5_PCIE == target)
    {
      ignore_ril_tech = TRUE; /* see above */

      /* use MHI port */
      default_qmi_port = QMI_PORT_RMNET_MHI_0;
      default_qmi_instance = QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0;
      strlcpy( qcril_data_modem_port_map_tbl[QCRIL_SECOND_MODEM_ID].qmi_port,
               QMI_PORT_RMNET_MHI_0,
               sizeof(qcril_data_modem_port_map_tbl[QCRIL_SECOND_MODEM_ID].qmi_port));
      QCRIL_LOG_DEBUG("default qmi_port set to %s, default_qmi_instance \
                      set to %d", default_qmi_port, default_qmi_instance);
      global_modem_id = QCRIL_DEFAULT_MODEM_ID;
    }
    else if (DS_TARGET_DSDA == target || DS_TARGET_SGLTE2 == target)
    {
      ignore_ril_tech = TRUE; /* see above */

      if (qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID)
      {
         /* use usb port */
         default_qmi_port = QMI_PORT_RMNET_USB_0;
        default_qmi_instance = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
         strlcpy( qcril_data_modem_port_map_tbl[QCRIL_SECOND_MODEM_ID].qmi_port,
               QMI_PORT_RMNET_USB_0,
               sizeof(qcril_data_modem_port_map_tbl[QCRIL_SECOND_MODEM_ID].qmi_port));
      }
      else
      {
         /* use smux port */
         default_qmi_port = QMI_PORT_RMNET_SMUX_0;
         default_qmi_instance = QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0;
         strlcpy( qcril_data_modem_port_map_tbl[QCRIL_SECOND_MODEM_ID].qmi_port,
               QMI_PORT_RMNET_SMUX_0,
               sizeof(qcril_data_modem_port_map_tbl[QCRIL_SECOND_MODEM_ID].qmi_port));
      }
      QCRIL_LOG_DEBUG("default_qmi_port set to %s, default_qmi_instance \
                       set to %d", default_qmi_port, default_qmi_instance);
      global_modem_id = QCRIL_DEFAULT_MODEM_ID;
    }
    else if (DS_TARGET_DSDA2 == target)
    {
      ignore_ril_tech = TRUE; /* see above */

      if (qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID)
      {
         /* use HSIC port */
         default_qmi_port = QMI_PORT_RMNET_USB_0;
         default_qmi_instance = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
         strlcpy( qcril_data_modem_port_map_tbl[QCRIL_SECOND_MODEM_ID].qmi_port,
               QMI_PORT_RMNET_USB_0,
               sizeof(qcril_data_modem_port_map_tbl[QCRIL_SECOND_MODEM_ID].qmi_port));
      }
      else
      {
         /* use USB port corresponding to second modem */
         default_qmi_port = QMI_PORT_RMNET2_USB_0;
        default_qmi_instance = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
         strlcpy( qcril_data_modem_port_map_tbl[QCRIL_SECOND_MODEM_ID].qmi_port,
               QMI_PORT_RMNET2_USB_0,
               sizeof(qcril_data_modem_port_map_tbl[QCRIL_SECOND_MODEM_ID].qmi_port));
      }
      QCRIL_LOG_DEBUG("default_qmi_port set to %s, default_qmi_instance \
                       set to %d", default_qmi_port, default_qmi_instance);
      global_modem_id = QCRIL_DEFAULT_MODEM_ID;
    }
    else
    {
      /* do not set default_qmi_port for any thing else right now
       * as we don't know */
      QCRIL_LOG_DEBUG("default_qmi_port left as-is to %s, default_qmi_instance\
                       to %d", default_qmi_port, default_qmi_instance);
    }

  } while (0);

  QCRIL_LOG_DEBUG("%s", "qcril_data_set_default_port: EXIT");
}


/*===========================================================================

  FUNCTION:  qcril_data_init

===========================================================================*/
/*!
    @brief
    Initialize the DATA subsystem of the RIL.

    (1) Call init routine of Data Services Internal Module

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_init()
{
/*-----------------------------------------------------------------------*/
  int i = 0;
  qcril_modem_stack_id_e_type stack_id;
  pthread_mutexattr_t info_tbl_mutex_attr;

  QCRIL_LOG_DEBUG( "%s", "qcril_data_init: ENTRY" );
/*
  QCRIL_LOG_DEBUG( "DS hdlr reg: qcril_data_request_data_call_list at [%x]\n",
                   qcril_data_request_data_call_list );
  QCRIL_LOG_DEBUG( "DS hdlr reg: qcril_data_request_data_call at [%x]\n",
                   qcril_data_request_setup_data_call );
  QCRIL_LOG_DEBUG( "DS hdlr reg: qcril_data_request_deactivate_data_call at [%x]\n",
                   qcril_data_request_deactivate_data_call );
  QCRIL_LOG_DEBUG( "DS hdlr reg: qcril_data_request_last_data_call_fail_cause at [%x]\n",
                   qcril_data_request_last_data_call_fail_cause );
  QCRIL_LOG_DEBUG( "DS hdlr reg: qcril_data_request_data_call_list at [%x]\n",
                   qcril_data_request_data_call_list );
  QCRIL_LOG_DEBUG( "DS hdlr reg: qcril_data_event_hdlr at [%x]\n",
                   qcril_data_event_hdlr );
  QCRIL_LOG_DEBUG( "DS hdlr reg: qcril_data_command_hdlr at [%x]\n",
                   qcril_data_command_hdlr );

  QCRIL_LOG_DEBUG( "info_tbl at [%x], call_tbl at [%x]",
                   info_tbl, call_tbl );
*/

#ifdef FEATURE_WAIT_FOR_MODEM
  sleep(5);
#endif

  global_instance_id = qmi_ril_get_process_instance_id();

  /* init dsi */
  dsi_set_ril_instance(global_instance_id);

  stack_id = qmi_ril_get_stack_id(global_instance_id);

  switch(stack_id)
  {
    case QCRIL_MODEM_PRIMARY_STACK_ID:
     global_subs_id = QMI_WDS_PRIMARY_SUBS;
     break;

    case QCRIL_MODEM_SECONDARY_STACK_ID:
     global_subs_id = QMI_WDS_SECONDARY_SUBS;
     break;

    case QCRIL_MODEM_TERTIARY_STACK_ID:
     global_subs_id = QMI_WDS_TERTIARY_SUBS;
     break;

    default:
     global_subs_id = QMI_WDS_DEFAULT_SUBS;
  }

  QCRIL_LOG_DEBUG( "qcril_data_init: stack_id: %d, subs_id: %d",
                      stack_id,global_subs_id);

  dsi_set_modem_subs_id(global_subs_id);

#ifdef FEATURE_QCRIL_FUSION
  /* use MDM for Fusion build */
  default_qmi_port = QMI_PORT_RMNET_SDIO_0;
#else
  /* for targets that are not 7630_fusion, use the ro.baseband
   * property to determine the modem_id, and qmi_port */
  qcril_data_set_default_port();
#endif

  qcril_data_qmi_wds_init(FALSE);

  qcril_data_util_update_partial_retry_enabled_flag();

  qcril_data_util_update_max_partial_retry_timeout();

  memset( info_tbl, 0, sizeof info_tbl );
  memset( dsi_cb_tbl, 0, sizeof dsi_cb_tbl );

  for( i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
  {
    info_tbl[ i ].index = i;
    info_tbl[ i ].timer_id = QCRIL_DATA_INVALID_TIMERID;
    info_tbl[ i ].cid = CALL_ID_INVALID;
    info_tbl[ i ].dorm_status = QMI_WDS_DORM_STATUS_DORMANT;

    dsi_cb_tbl[i].instance_id = QCRIL_DATA_INVALID_INSTANCE_ID;
    dsi_cb_tbl[i].modem_id = QCRIL_DATA_INVALID_MODEM_ID;
    dsi_cb_tbl[i].dsi_hndl = QCRIL_DSI_HNDL_INVALID;

    QCRIL_LOG_VERBOSE("info_tbl[%d].index = %d", i, i);
  }/* for() */


  pthread_mutex_init(&dsi_cb_tbl_mutex, NULL);

  /* Get the default set of attributes */
  pthread_mutexattr_init(&info_tbl_mutex_attr);

  /* Set this mutex to be recursive i.e. the same thread can lock the
     mutex multiple times without being blocked.
     This is required because we can get the dsi callbacks for some
     dsi calls (e.g. dsi_stop_data_call) in the context of the calling
     qcril thread (which might hold the info_tbl_mutex before calling)
     and we could potentially reacquire the info_tbl_mutex
     in qcril_data_net_cb() and deadlock */
  pthread_mutexattr_settype(&info_tbl_mutex_attr, PTHREAD_MUTEX_RECURSIVE);

  pthread_mutex_init(&info_tbl_mutex, &info_tbl_mutex_attr);

  /* release resource */
  pthread_mutexattr_destroy(&info_tbl_mutex_attr);

#if (RIL_QCOM_VERSION >= 2)
  /* Initialize QOS support */
  qcril_data_qos_init();
#endif /* RIL_QCOM_VERSION >= 2 */

  /* Initialize Client support */
  qcril_data_client_init();

  QCRIL_LOG_DEBUG( "%s", "qcril_data_init: EXIT" );
} /* qcril_data_init() */

/*===========================================================================

  FUNCTION:  qcril_data_abort_incompatible_pending_calls

===========================================================================*/
/*!
    @brief
    Abort pending calls on older tech on a RAT change

    @return
    NONE
*/
/*=========================================================================*/
static void qcril_data_abort_incompatible_pending_calls
(
  qmi_wds_data_sys_status_network_info_type  *nw_info
)
{
  int i;
  const char *str = NULL;

  if (!nw_info)
  {
    QCRIL_LOG_ERROR("Invalid parameter");
    return;
  }

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  for (i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; ++i)
  {
    qcril_data_ril_radio_tech_t    numeric_ril_tech;
    qcril_data_call_info_tbl_type  *info_tbl_ptr = &info_tbl[i];

    if (!VALIDATE_LOCAL_DATA_OBJ(info_tbl_ptr)                ||
        RIL_REQUEST_SETUP_DATA_CALL != info_tbl_ptr->pend_req ||
        !info_tbl_ptr->call_params)
    {
      continue;
    }

    numeric_ril_tech = qcril_data_get_numeric_ril_technology(info_tbl_ptr->call_params->ril_tech);

    str = qcril_data_util_get_ril_tech_string(numeric_ril_tech);

    /* If RIL provided technology is 1x or EVDO and the new Modem reported
       technology is not 1x or EVDO, abort the call */
    if (QCRIL_DATA_IS_RIL_RADIO_TECH_CDMA_1X_EVDO(numeric_ril_tech) &&
        !QCRIL_DATA_IS_DATA_SYS_STATUS_RAT_MASK_3GPP2_1X_DO(nw_info->network,
                                                            nw_info->rat_mask.cdma_rat_mask,
                                                            nw_info->db_so_mask))
    {
      QCRIL_LOG_DEBUG("RIL requested technology=%s, Modem reported technology=!1x/DO",
                      str);
      qcril_data_stop_data_call(info_tbl_ptr);
    }
    /* If RIL provided technology is eHRPD and the new Modem reported
       technology is not eHRPD, abort the call */
    else if (QCRIL_DATA_IS_RIL_RADIO_TECH_EHRPD(numeric_ril_tech) &&
             !QCRIL_DATA_IS_DATA_SYS_STATUS_RAT_MASK_3GPP2_EHRPD(nw_info->network,
                                                                 nw_info->rat_mask.cdma_rat_mask,
                                                                 nw_info->db_so_mask))
    {
      QCRIL_LOG_DEBUG("RIL requested technology=%s, Modem reported technology=!eHRPD - aborting call",
                      str);
      qcril_data_stop_data_call(info_tbl_ptr);
    }
    else
    {
      QCRIL_LOG_DEBUG("RIL requested technology=%s, skipping abort");
    }
  }

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
} /*qcril_data_abort_incompatible_pending_calls() */

/*===========================================================================

  FUNCTION:  qcril_data_update_mtu

===========================================================================*/
/*!
    @brief
    Changes MTU value on all active calls
    @return
    NONE
*/

void qcril_data_update_mtu
(
  unsigned int mtu
)
{
  int i;
  int dev_instance;
  dsi_hndl_t dsi_hndl;

  qcril_data_mtu = mtu;

  if (qcril_data_mtu > 0)
  {
    /* Looking for all active calls*/
    for (i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++)
    {
      GET_DEV_INSTANCE_FROM_NAME(i, dev_instance);
      if (dev_instance < 0 ||
          dev_instance >= QCRIL_DATA_MAX_DEVS ||
          !VALIDATE_LOCAL_DATA_OBJ(&info_tbl[i]) ||
          info_tbl[i].cid == CALL_ID_INVALID)
      {
        continue;
      }

      /* Check if dsi_hdnl is valid and set MTU */
      if (info_tbl[i].dsi_hndl != NULL)
      {
        QCRIL_LOG_DEBUG("Changing default MTU value to [%d] for Call [%d]",
                        mtu, info_tbl[i].dsi_hndl);
        qcril_data_iface_set_mtu(info_tbl[i].dsi_hndl, mtu);

        /* Post a MTU update event */
        qcril_data_post_dsi_netctrl_event(info_tbl[i].instance_id,
                                          info_tbl[i].modem_id,
                                          info_tbl[i].pend_tok,
                                          &info_tbl[i],
                                          DSI_EVT_NET_NEWMTU,
                                          NULL);
      }
    } /* for */
  }
}
void qcril_data_process_stack_switch
(
  qcril_modem_stack_id_e_type old_stack_id,
  qcril_modem_stack_id_e_type new_stack_id,
  qcril_instance_id_e_type instance_id
)
{
 qcril_data_cmd_data_t *cmd_data;

 QCRIL_LOG_VERBOSE( "%s", "qcril_data_process_stack_switch: ENTRY" );

 if(old_stack_id != new_stack_id)
 {
   QCRIL_LOG_DEBUG("Stack ID changed from %d to %d",
     old_stack_id,new_stack_id);
 }
 else
 {
   goto bail;
 }

 cmd_data = ( qcril_data_cmd_data_t *)malloc( sizeof( qcril_data_cmd_data_t ) );

 if(NULL == cmd_data)
 {
   QCRIL_LOG_ERROR("%s", "Not enough memory\n");
   goto bail;
 }

 cmd_data->old_stack_id = old_stack_id;
 cmd_data->new_stack_id = new_stack_id;
 cmd_data->self = cmd_data;
 cmd_data->cmd_id = QCRIL_DATA_STACK_SWITCH_CMD;

 if(E_SUCCESS != qcril_event_queue( instance_id,
                                      global_modem_id,
                                      QCRIL_DATA_NOT_ON_STACK,
                                      QCRIL_EVT_DATA_COMMAND_CALLBACK,
                                      ( void * )cmd_data,
                                      sizeof( qcril_data_cmd_data_t ),
                                      (RIL_Token) QCRIL_TOKEN_ID_INTERNAL ) )
 {
   QCRIL_LOG_ERROR("%s", "qcril_event_queue failed\n");
   goto bail;
 }

bail:

 QCRIL_LOG_VERBOSE( "%s", "qcril_data_process_stack_switch: EXIT" );
}
