/*===========================================================================

  @file    qmi_proxy.c
  @brief   QMI Proxy Layer

  DESCRIPTION
  QMI Proxy Layer handles the arbitration requirements for QMI Service for
  different modem configurations including Multimode, SVLTE Type 1,
  CSFB, SVLTE Type 2 and SGLTE.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_proxy_init() needs to be called before processing of any QMI Proxy
  requests

  ---------------------------------------------------------------------------
  Copyright (c) 2010-2012, 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#define LOG_NDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0

#ifdef LOG_TAG
  #undef LOG_TAG
  #define LOG_TAG "RILC"
#endif /* LOG_TAG */

#include <utils/Log.h>

#include "comdef.h"
#include <msg.h>
#include <msgcfg.h>

#undef LOG_TAG
#include <diag_lsm.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>
#include <limits.h>
#include "qmi.h"
#include "qmi_i.h"
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_proxy.h"
#include "qmi_qmux_if.h"
#include "qmi_service.h"
#include "common_v01.h"
#include "device_management_service_v01.h"
#include "network_access_service_v01.h"
#include "voice_service_v02.h"
#include "phonebook_manager_service_v01.h"
#include "wireless_messaging_service_v01.h"
#include "specific_absorption_rate_v01.h"
#include "qmi_ims_vt_v01.h"
#include "ip_multimedia_subsystem_presence_v01.h"
#include "qmi_proxy_sglte_sm.h"
#include "qmi_proxy_queue.h"
/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

/* Maximum number of QMI service transaction on each modem for each QMI Proxy Service Request */
#define QMI_PROXY_MAX_SRVC_TXNS                            2

/* Timeout for Synchronous QMI Service Request */
#ifdef FEATURE_RILTEST
   #define QMI_PROXY_SYNC_REQ_TIMEOUT                         1500
#else
   #define QMI_PROXY_SYNC_REQ_TIMEOUT                         500
#endif

/* Local modem's QMI Service connection port info */
#define QMI_PROXY_LOCAL_CONN_NAME                          (qmi_proxy_get_local_modem_connection_name())
#define QMI_PROXY_LOCAL_CONN_ID                            (qmi_proxy_get_local_modem_connection_id())

/* Remote modem's QMI Service connection port info */
#define QMI_PROXY_REMOTE_CONN_NAME                         (qmi_proxy_get_remote_modem_connection_name())
#define QMI_PROXY_REMOTE_CONN_ID                           (qmi_proxy_get_remote_modem_connection_id())

/* Internal QMI Proxy Client ID */
#define QMI_PROXY_INTERNAL_CLIENT_ID                       0

/* QMI CTL Result code TLV */
#define QMI_PROXY_CTL_RESULT_CODE_TYPE_ID                  (0x02)
#define QMI_PROXY_CTL_GET_CLIENT_ID_TYPE_ID_REQ_RSP         0x01

/* QMI CTL Message Header size values */
#define QMI_PROXY_CTL_TXN_HDR_SIZE                         ( 2 )
#define QMI_PROXY_CTL_MSG_HDR_SIZE                         ( QMI_PROXY_CTL_TXN_HDR_SIZE + QMI_SRVC_STD_MSG_HDR_SIZE )
#define QMI_PROXY_CTL_MAX_MSG_SIZE                         1294

/* QMI CTL Message Flag */
#define QMI_PROXY_CTL_REQUEST_CONTROL_FLAG                 0x00
#define QMI_PROXY_CTL_RESPONSE_CONTROL_FLAG                0x01
#define QMI_PROXY_CTL_INDICATION_CONTROL_FLAG              0x02

/* QMI CTL Service */
#define QMI_PROXY_CTL_SERVICE                              0

/* QMI CTL message ID's */
#define QMI_PROXY_CTL_GET_VER_INFO_MSG_ID                  0x0021
#define QMI_PROXY_CTL_GET_CLIENT_ID_MSG_ID                 0x0022
#define QMI_PROXY_CTL_RELEASE_CLIENT_ID_MSG_ID             0x0023
#define QMI_PROXY_CTL_SET_DATA_FORMAT_MSG_ID               0x0026
#define QMI_PROXY_CTL_SYNC_MSG_ID                          0x0027
#define QMI_PROXY_CTL_REG_PWR_SAVE_MODE_MSG_ID             0x0028
#define QMI_PROXY_CTL_CONFIG_PWR_SAVE_SETTINGS_MSG_ID      0x0029
#define QMI_PROXY_CTL_SET_PWR_SAVE_MODE_MSG_ID             0x002A
#define QMI_PROXY_CTL_GET_PWR_SAVE_MODE_MSG_ID             0x002B

/* QMI CTL response TLV */
#define QMI_PROXY_CTL_GET_CLIENT_ID_TYPE_ID_REQ_RSP        0x01
#define QMI_PROXY_CTL_RELEASE_CLIENT_ID_TYPE_ID_REQ_RSP    0x01
#define QMI_PROXY_CTL_GET_SERVICE_VERSION_LIST_TYPE_ID     0x01
#define QMI_PROXY_CTL_GET_PWR_SAVE_MODE_TYPE_ID_RSP        0x01
#define QMI_PROXY_CTL_SET_DATA_FORMAT_LINK_PROT_REQ_RSP    0x10

/* QMI Proxy Service Control Flag */
#define QMI_PROXY_SERVICE_CONTROL_FLAG                     0x80
#define QMI_PROXY_CONTROL_FLAG                             0x01

/* QMI SRVC Control Flag values */
#define QMI_PROXY_STD_SRVC_REQUEST_CONTROL_FLAG            0x00
#define QMI_PROXY_STD_SRVC_RESPONSE_CONTROL_FLAG           0x02
#define QMI_PROXY_STD_SRVC_INDICATION_CONTROL_FLAG         0x04

/* Control the display of QMI Message */
#define QMI_PROXY_MAX_BUFFER_BYTES_PER_LINE                16
#define QMI_PROXY_MAX_OUTPUT_BUF_SIZE                      ( ( QMI_PROXY_MAX_BUFFER_BYTES_PER_LINE * 3 ) + 2 )

#define QMI_PROXY_MAX_SRV_VER_LEN  ( QMI_MAX_SERVICE_VERSIONS * 5 ) + 1

/* SGLTE User Mode Pref Property */
#define QMI_PROXY_SGLTE_USER_MODE_PREF_PROPERTY         "persist.radio.sglte.mode_pref"
#define QMI_PROXY_SGLTE_PLMN_LIST_PROPERTY              "persist.radio.sglte.plmn_list"

/* SGLTE Test mode configuration */
#define QMI_PROXY_SGLTE_TEST_MODE_PROPERTY              "persist.radio.sglte.test_mode"

/* QMI Proxy PLMN list, separator between entries */
#define QMI_PROXY_PLMN_LIST_ITEM_BOUNDARY               ':'

/* QMI PRoxy PLMN list entry, separator between MCC, MNC */
#define QMI_PROXY_PLMN_LIST_MCC_BOUNDARY                 ','

/* Maximum number of SGLTE PLMNs */
#define QMI_PROXY_MAX_SGLTE_PLMN_ENTRIES                  13

/* Length of MCC/MNC when represented as ASCII string */
#define QMI_PROXY_MCC_LENGTH                              4  /* Three digits plus null terminator */
#define QMI_PROXY_MNC_LENGTH                              4  /* Three digits plus null terminator */

/* Default SGLTE PLMNs */
#define DEFAULT_SGLTE_MCC_1 "460"
#define DEFAULT_SGLTE_MNC_1 "00F"
#define DEFAULT_SGLTE_MCC_2 "460"
#define DEFAULT_SGLTE_MNC_2 "02F"
#define DEFAULT_SGLTE_MCC_3 "460"
#define DEFAULT_SGLTE_MNC_3 "07F"
#define DEFAULT_SGLTE_PLMNS 3

#define NAS_VAL_ROAMING_HOME_EX                       (64)
/* QMI Proxy PLMN type */
typedef struct
{
  char mcc[QMI_PROXY_MCC_LENGTH]; /* MCC digits in ASCII characters */

  char mnc[QMI_PROXY_MNC_LENGTH]; /* MNC digits in ASCII characters.
                                  ** Digit 3 in MNC is optional and when
                                  ** not present, the unused digit will
                                  ** be set to 0xFF. For example, 15
                                  ** a two-digit MNC) is reported using
                                  ** the byte stream 0x31 0x35 0xFF.
                                  */
} qmi_proxy_plmn_id_s_type;

#define DEFAULT_SGLTE_PS_TO_LT_TIMER 22  /* Default value for SGLTE PS->LT
                                         ** timer in sec */
#define DEFAULT_SGLTE_PS_TO_G_TIMER  150 /* Default value for SGLTE PS->G
                                         ** timer in sec */
#define DEFAULT_SGLTE_RAT_TIMER      430 /* Default value for SGLTE rat
                                         ** full service acquisition
                                         ** timer in sec */
#define DEFAULT_NON_SGLTE_RAT_TIMER  180 /* Default value for non-SGLTE rat
                                         ** full service acquisition
                                         ** timer in sec */

/* QMUX hdr type */
typedef struct
{
  uint8  i_f;
  uint16 len;
  uint8  ctl_flag;
  uint8  service_id;
  uint8  client_id;
} qmi_proxy_qmux_hdr_type;

/* QMI CTL service message type */
typedef enum
{
  QMI_PROXY_CTL_SERVICE_REQUEST_MSG,
  QMI_PROXY_CTL_SERVICE_RESPONSE_MSG,
  QMI_PROXY_CTL_SERVICE_INDICATION_MSG,
  QMI_PROXY_CTL_SERVICE_ERROR_MSG
} qmi_proxy_ctl_service_msg_type;

/* QMI Proxy Configuration */
typedef enum qmi_proxy_cfg_type
{
  QMI_PROXY_CFG_MULTIMODE,  /* Multimode */
  QMI_PROXY_CFG_SVLTE_1,    /* SVLTE Type 1 */
  QMI_PROXY_CFG_CSFB,       /* CSFB */
  QMI_PROXY_CFG_SVLTE_2,    /* SVLTE Type 2 */
  QMI_PROXY_CFG_APQ,        /* APQ */
  QMI_PROXY_CFG_SGLTE,      /* SGLTE */
  QMI_PROXY_CFG_SGLTE_2,    /* SGLTE Type 2*/
  QMI_PROXY_MAX_CFGS
} qmi_proxy_cfg_type;

/* QMI Proxy connection type */
typedef enum qmi_proxy_conn_type
{
  QMI_PROXY_LOCAL_CONN_TYPE,  /* Local modem */
  QMI_PROXY_REMOTE_CONN_TYPE, /* Remote modem */
  QMI_PROXY_MAX_CONN_TYPE,
  QMI_PROXY_BOTH_CONN_TYPE
} qmi_proxy_conn_type;

/* QMI Proxy Transaction State */
typedef enum qmi_proxy_txn_state_type
{
  QMI_PROXY_TXN_IDLE,              /* Idle */
  QMI_PROXY_TXN_IN_PROGRESS,       /* In progress */
  QMI_PROXY_TXN_COMPLETED_FAILURE, /* Completed with failure result */
  QMI_PROXY_TXN_COMPLETED_SUCCESS  /* Completed with success result */
} qmi_proxy_txn_state_type;

/* QMI Proxy Asynchronous Transaction State */
typedef enum qmi_proxy_async_state_type
{
  QMI_PROXY_ASYNC_IDLE,                /* Idle */
  QMI_PROXY_ASYNC_LOCAL_TXN_ON_HOLD,   /* Local transaction put on hold */
  QMI_PROXY_ASYNC_REMOTE_TXN_ON_HOLD,  /* Remote transaction put on hold */
  QMI_PROXY_ASYNC_LOCAL_TXN_WAIT_IND,  /* Local transaction cannot be dispatched until QMI Indication is received */
  QMI_PROXY_ASYNC_REMOTE_TXN_WAIT_IND  /* Remote transaction cannot be dispatched until QMI indication is received */
} qmi_proxy_async_state_type;

/* QMI Proxy Transaction Entry State */
typedef enum qmi_proxy_txn_entry_state_type
{
  QMI_PROXY_TXN_ENTRY_SETUP,      /* Setup transaction entry */
  QMI_PROXY_TXN_ENTRY_DISPATCHED, /* Dispatched for execution */
  QMI_PROXY_TXN_ENTRY_DONE        /* Completed */
} qmi_proxy_txn_entry_state_type;

/* Mode Preference */
typedef enum qmi_proxy_mode_pref_type
{
  QMI_PROXY_MODE_PREF_UNKNOWN                 = -1, /* Only used as a placeholder */
  QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED     = 0,  /* Option allowed for non-fusion and fusion */
  QMI_PROXY_MODE_PREF_GSM_ONLY                = 1,  /* Option allowed for non-fusion and fusion */
  QMI_PROXY_MODE_PREF_WCDMA_ONLY              = 2,  /* Option allowed for non-fusion and fusion */
  QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO          = 3,  /* Option allowed for non-fusion and fusion */
  QMI_PROXY_MODE_PREF_CDMA_EVDO               = 4,  /* Option allowed for non-fusion and fusion */
  QMI_PROXY_MODE_PREF_CDMA_ONLY               = 5,  /* Option allowed for non-fusion and fusion */
  QMI_PROXY_MODE_PREF_EVDO_ONLY               = 6,  /* Option allowed for non-fusion and fusion */
  QMI_PROXY_MODE_PREF_GSM_WCDMA_CDMA_EVDO     = 7,  /* Option allowed for non-fusion */
  QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO           = 8,  /* Option allowed for fusion */
  QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA           = 9,  /* Option allowed for fusion */
  QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO_GSM_WCDMA = 10, /* Option allowed for fusion */
  QMI_PROXY_MODE_PREF_LTE_ONLY                = 11, /* Option allowed for fusion */
  QMI_PROXY_MODE_PREF_TDSCDMA_ONLY            = 12, /* Option allowed for fusion */
  QMI_PROXY_MAX_MODE_PREFS
} qmi_proxy_mode_pref_type;

/* proxy clients*/
typedef enum qmi_proxy_client_type
{
  QMI_PROXY_QMUX_CLIENT,       /* QMUX client */
  QMI_PROXY_TETHER_CLIENT,     /* TETHER client */
  QMI_PROXY_MAX_CLIENT
}qmi_proxy_client_type;

#define QMI_PROXY_QMUX_CONN_SOCKET_PATH    "/dev/socket/qmux_radio/proxy_qmux_connect_socket"
#define QMI_PROXY_QMUX_CLIENT_SOCKET_PATH  "/dev/socket/qmux_radio/proxy_qmux_client_socket"

#define QMI_PROXY_TETHER_CONN_SOCKET_PATH   "/dev/socket/qmux_radio/proxy_tether_connect_socket"
#define QMI_PROXY_TETHERCLIENT_SOCKET_PATH  "/dev/socket/qmux_radio/proxy_tether_client_socket"

/* Maximum length of log message */
#define QMI_PROXY_MAX_LOG_MSG_SIZE                         512

/* QMI Proxy Registration Info */
typedef struct qmi_proxy_reg_info_type
{
  int local_srvc_required;           /* Indicates if QMI Proxy Client requires to reqister on local modem */
  int remote_srvc_required;          /* Indicates if QMI Proxy Client requires to reqister on remote modem */
  int internal_local_srvc_required;  /* Indicates if QMI Proxy itself requires to register on local modem */
  int internal_remote_srvc_required; /* Indicates if QMI Proxy itself requires to register on remote modem */
} qmi_proxy_reg_info_type;

/* QMI Proxy Registered Service Info */
typedef struct qmi_proxy_srvc_info_type
{
  int                      registered;    /* Indicates if registered for a QMI Service */
  qmi_client_type          client_handle; /* Client Handle for the registered QMI Service */
} qmi_proxy_srvc_info_type;

typedef struct qmi_proxy_rx_message_s {
  qmi_client_type        client_handle;
  unsigned long          msg_id;
  qmi_service_id_type    proxy_srvc_id;
  qmi_client_id_type     proxy_client_id;
  qmi_connection_id_type proxy_conn_id;
  qmi_proxy_conn_type    proxy_conn_type;
  void                  *decoded_payload;
  uint32_t               decoded_payload_len;
}qmi_proxy_rx_message;

/* QMI Proxy Client Info */
typedef struct qmi_proxy_client_info_type
{
  struct qmi_proxy_client_info_type *next;                                /* Link to next QMI Proxy Client */
  int                               ref_count;                            /* Record references to the QMI Proxy Client Info */
  unsigned int                      ready_to_delete;                      /* Need to be deleted after ref_count becomes 0 */
  qmi_client_id_type                proxy_client_id;                      /* QMI Proxy Client ID */
  qmi_connection_id_type            proxy_conn_id;                        /* QMI Proxy Connection ID */
  qmi_service_id_type               proxy_srvc_id;                        /* QMI Proxy Service ID */
  qmi_proxy_srvc_info_type          srvc_info[ QMI_PROXY_MAX_CONN_TYPE ]; /* QMI Proxy Registered Service info */
  QMI_PLATFORM_MUTEX_DATA_TYPE      mutex;                                /* Mutex to control client info update */
  int fd;
} qmi_proxy_client_info_type;

/* QMI Proxy Service Arbitration Handler type */
typedef int (*qmi_proxy_srvc_arb_hdlr_type)
(
  qmi_service_id_type        srvc_id,             /* QMI Service ID */
  qmi_client_id_type         client_id,           /* QMI Client ID */
  qmi_connection_id_type     conn_id,             /* QMI Connection ID */
  unsigned long              rx_txn_id,           /* Transaction ID of QMI Service Request originated from QMI Proxy Client */
  unsigned long              srvc_msg_id,         /* Service Message ID */
  void                       *decoded_payload,    /* QMI Servic Request Message in C structure format */
  uint32_t                   decoded_payload_len, /* Length of QMI Service Request Message in C structure format */
  qmi_proxy_client_info_type *client              /* QMI Proxy Client Info */
);

/* QMI Proxy Service Err Handler type */
typedef void (*qmi_proxy_rx_ind_hdlr_type)
(
  qmi_proxy_rx_message  *rx_msg               /* decoded indication message */
);

/* QMI Proxy Service Err Handler type */
typedef void (*qmi_proxy_srvc_err_hdlr_type)
(
  qmi_client_id_type     proxy_client_id,     /* QMI Client ID */
  qmi_connection_id_type proxy_conn_id,       /* QMI Connection ID */
  qmi_proxy_conn_type    proxy_conn_type,     /* Proxy Connection Type */
  unsigned long          msg_id,              /* Message ID */
  unsigned long          txn_id,              /* Transaction ID */
  qmi_error_type_v01 error                    /* QMI Error Type */
);

/* QMI Proxy Transaction ID type */
typedef uint16 qmi_proxy_txn_id_type;

/* QMI Proxy Transaction key (i.e. service ID + transaction ID) type */
typedef uint32 qmi_proxy_txn_key_type;

/* QMI Proxy Transaction Entry */
typedef struct qmi_proxy_txn_entry_type
{
  qmi_proxy_txn_entry_state_type state;           /* State of QMI Proxy Transaction Entry */
  qmi_service_id_type            srvc_id;         /* QMI Service ID */
  unsigned long                  msg_id;          /* QMI Service Request Message ID */
  void                           *msg;            /* QMI Service Request Message in raw format */
  uint32_t                       msg_len;         /* Length of QMI Service Request Message in raw format */
} qmi_proxy_txn_entry_type;

/* QMI Proxy Transaction Entry List */
typedef struct qmi_proxy_txn_entry_list_type
{
  uint8                    num_txn;                          /* Number of Transaction Entries*/
  qmi_proxy_txn_state_type state;                            /* Overall State of all Transaction Entries in the List */
  qmi_proxy_txn_entry_type entry[ QMI_PROXY_MAX_SRVC_TXNS ]; /* Transaction Entry Info */
} qmi_proxy_txn_entry_list_type;

/* QMI Proxy Transaction Info */
typedef struct qmi_proxy_txn_info_type
{
  struct qmi_proxy_txn_info_type *next;                           /* Link to next QMI Proxy Transaction */
  int                            ref_count;                       /* Record references to the info of the QMI Proxy Transaction */
  unsigned int                   ready_to_delete;                 /* Need to be deleted after ref_count becomes 0 */
  qmi_service_id_type            proxy_srvc_id;                   /* QMI Proxy Service ID */
  qmi_client_id_type             proxy_client_id;                 /* QMI Proxy Client ID */
  qmi_connection_id_type         proxy_conn_id;                   /* QMI Proxy Connection ID */
  qmi_proxy_txn_key_type         proxy_txn_key;                   /* QMI Proxy Transaction Key */
  unsigned long                  proxy_msg_id;                    /* QMI Proxy Service Request Message ID */
  unsigned long                  rx_txn_id;                       /* Transaction ID associated with QMI Request Message originated
                                                                     from QMI Proxy Client */
  qmi_proxy_txn_state_type       state;                           /* Oversall State of QMI Proxy Transaction */
  QMI_PLATFORM_MUTEX_DATA_TYPE   mutex;                           /* Mutex to control update to Transaction State */
  qmi_proxy_txn_entry_list_type  txns[ QMI_PROXY_MAX_CONN_TYPE ]; /* Transaction List for both modems */
  qmi_proxy_async_state_type     async_state;                 /* Asynchronous State of QMI Proxy Transaction  */
  boolean                        user_data_valid;                 /* Indicates if user data is valid */
  boolean                        is_recvd_ind;                    /* Indicates if the txn wait indication is received or not */
  uint32                         user_data;                       /* User data */
} qmi_proxy_txn_info_type;

/* Sketch pad for aggregating response from both modems */
typedef struct qmi_proxy_svlte_cache_type
{
  dms_get_operating_mode_resp_msg_v01              dms_get_oprt_mode_resp;
  dms_get_band_capability_resp_msg_v01             dms_get_band_cap_resp;
  dms_get_device_cap_resp_msg_v01                  dms_get_dev_cap_resp;
  nas_get_sys_info_resp_msg_v01                    nas_get_sys_info_resp;
  nas_get_sig_info_resp_msg_v01                    nas_get_sig_info_resp;
  nas_get_err_rate_resp_msg_v01                    nas_get_err_rate_resp;
  nas_perform_network_scan_resp_msg_v01            nas_per_net_scan_resp;
  nas_get_rf_band_info_resp_msg_v01                nas_get_rf_band_info_resp;
  nas_get_system_selection_preference_resp_msg_v01 nas_get_sys_sel_pref_resp;
  nas_get_operator_name_data_resp_msg_v01          nas_get_opr_nam_resp;
  wms_get_indication_register_resp_msg_v01         wms_get_indication_register_resp;
  wms_get_service_ready_status_resp_msg_v01        wms_get_service_ready_status_resp;
} qmi_proxy_svlte_cache_type;

typedef struct qmi_proxy_voip_info_type
{
  boolean is_registered_on_ims;
  boolean is_voip_enabled;
} qmi_proxy_voip_info_type;

typedef struct qmi_proxy_ssr_info_type
{
  boolean is_ssr;
  boolean ignore_oprt_mode;
} qmi_proxy_ssr_info_type;

/* QMI Proxy Internal Info */
typedef struct qmi_proxy_internal_info_type
{
  boolean initialized;                                               /* Indicates if the internal info has been initialized */
  QMI_PLATFORM_MUTEX_DATA_TYPE init_mutex;                           /* Mutex to control initialization */
  qmi_proxy_srvc_info_type srvc_info[ QMI_PROXY_MAX_CONN_TYPE ][ QMI_MAX_SERVICES ];
                                                                     /* QMI Proxy Internal Service Handles used for managing fusion
                                                                        requirements */
  dms_operating_mode_enum_v01 svlte_oprt_mode;                       /* SVLTE Operating Mode */
  qmi_proxy_mode_pref_type svlte_mode_pref;                          /* SVLTE Mode Preference */
  qmi_proxy_mode_pref_type sglte_user_mode_pref;                     /* SGLTE User Mode Preference */
  uint32 svlte_radio_if_mask;                                        /* SVLTE Radio IF mask */
  boolean in_emergency_mode;                                         /* Indicates if in emergency mode or not */
  int     number_of_emergency_calls;                                 /* Number of emergency calls active */
  int number_of_calls;                                               /* Number of calls active */
  call_mode_enum_v02 emergency_call_mode;                            /* Emergency call mode */
  qmi_proxy_svlte_cache_type svlte_cache[ QMI_PROXY_MAX_CONN_TYPE ]; /* SVLTE Cache */
  QMI_PLATFORM_MUTEX_DATA_TYPE cache_mutex;                          /* Mutex to control access to SVLTE cache */
  qmi_proxy_voip_info_type  voip_info;                               /* voip related information */
  QMI_PLATFORM_MUTEX_DATA_TYPE voip_info_mutex;                      /* Mutex to control access to voip info */
  qmi_proxy_ssr_info_type  ssr_info;                                 /* SSR information */
  QMI_PLATFORM_MUTEX_DATA_TYPE ssr_info_mutex;                       /* Mutex to control SSR info */
  qmi_proxy_sglte_state_machine_type *sglte_sm;                      /* State machine to control SGLTE states */
  boolean is_sglte_power_up;                                         /* Initial powerup for SGLTE */
  qmi_proxy_conn_type cs_active_modem;                               /* SGLTE - Indicates which modem is CS active currently */
  qmi_proxy_conn_type ps_active_modem;                               /* SGLTE - Indicates which modem is PS active currently */
  boolean is_ps_registered;                                          /* SGLTE - Indicates whether ps is registered or not */
  boolean is_ps_roaming;                                             /* SGLTE - Indicates whether ps is roaming or not */
  boolean is_cs_registered;                                          /* SGLTE - Indicates whether cs is registered or not */
  boolean is_cs_roaming;                                             /* SGLTE - Indicates whether cs is roaming or not */
  qmi_proxy_plmn_id_s_type last_sglte_plmn[ QMI_PROXY_MAX_CONN_TYPE]; /* SGLTE: Last reported PLMN */
  int is_sglte_plmn[ QMI_PROXY_MAX_CONN_TYPE ];                      /* SGLTE: Whether last reported PLMN for modem is SGLTE */
  int is_sglte_plmn_valid[ QMI_PROXY_MAX_CONN_TYPE ];                /* SGLTE: Whether the current PLMN for modem is valid */
  int is_in_sglte_coverage;
  int desired_mode;                                                  /* SGLTE: Requested operating mode */
  qmi_proxy_sglte_sm_event_id pending_event;                         /* SGLTE: Event which is pending to be queued */
  char *pending_event_name;                                          /* SGLTE: Name of the event which is pending to be queued */
  int is_dtm_supported[ QMI_PROXY_MAX_CONN_TYPE ];                   /* SGLTE: Whether Dual transfer mode is
                                                                      *        supported by the current gsm service */
  boolean is_local_svc;                                              /* SGLTE: Indicates if local is in service */
  boolean is_remote_svc;                                             /* SGLTE: Indicates if remote is in service */
  wms_service_ready_status_enum_v01 service_ready[ QMI_PROXY_MAX_CONN_TYPE ]; /* local modem wms ready status */
} qmi_proxy_internal_info_type;

/* Thread data */
typedef struct qmi_proxy_thread_data_type
{
  qmi_qmux_if_msg_hdr_type msg_hdr;
  int                      msg_len;
  unsigned char            *msg;
  int                      fd;
} qmi_proxy_thread_data_type;

typedef struct qmi_proxy_force_sglte_sys_sel_pref_and_oprt_mode_params
{
  int cs_active_modem;
  int ps_active_modem;
  qmi_proxy_mode_pref_type user_mode_pref;
  int is_sglte_plmn;
  int is_user_request;
  int is_mode_pref_required;
} qmi_proxy_force_sglte_sys_sel_pref_and_oprt_mode_params;

typedef void (*timer_expiry_handler_type)();

typedef struct qmi_proxy_sglte_hystersis_timer_info_s
{
  int hyst_ps_to_g_timer_value;
  int hyst_ps_to_lt_timer_value;
  int sglte_rat_timer_value;
  int non_sglte_rat_timer_value;
  pthread_t timer_thread_id;
  QMI_PLATFORM_SIGNAL_DATA_TYPE(signal);
} qmi_proxy_sglte_hystersis_timer_info_type;

typedef struct qmi_proxy_sglte_hystersis_helper_info_s
{
  struct timeval hystersis_timer;
  timer_expiry_handler_type timer_expiry_handler;
} qmi_proxy_sglte_hystersis_timer_helper_info_type;

/*===========================================================================

                         FUNCTION DECLARATIONS

===========================================================================*/

static int qmi_proxy_force_online
(
  qmi_proxy_conn_type conn_type
);

static int qmi_proxy_force_sys_sel_pref_sync
(
  qmi_proxy_conn_type conn_type,
  nas_set_system_selection_preference_req_msg_v01 *request,
  nas_set_system_selection_preference_resp_msg_v01 *response,
  int timeout_msecs
);

void qmi_proxy_format_log_msg
(
  char *buf_ptr,
  int  buf_size,
  char *fmt,
  ...
);

static void qmi_proxy_log_msg_to_adb
(
  int  lvl,
  char *msg_ptr
);

static void qmi_proxy_txn_relref
(
  qmi_proxy_txn_info_type **txn,
  boolean                 txn_done,
  boolean                 lock_list_mutex
);

static void qmi_proxy_complete_txn
(
  qmi_client_type       client_handle,
  unsigned long         msg_id,
  void                  *resp_c_struct,
  int                   resp_c_struct_len,
  void                  *resp_cb_data,
  qmi_client_error_type transp_err
);

static int qmi_proxy_dms_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
);

static int qmi_proxy_dms_srvc_response_update_cache
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *modified_resp,
  void                **modified_resp_buf,
  int                 *modified_resp_buf_len
);

static int qmi_proxy_dms_srvc_response_update_cache_sglte
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *modified_resp,
  void                **modified_resp_buf,
  int                 *modified_resp_buf_len
);

static void qmi_proxy_srvc_unsol_ind_cb
(
  qmi_client_type client_handle,
  unsigned long   msg_id,
  unsigned char   *ind_buf,
  int             ind_buf_len,
  void            *ind_cb_data
);

static void qmi_proxy_dms_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
);

int qmi_proxy_nas_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
);

static int qmi_proxy_nas_srvc_response_update_cache_sglte
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *modified_resp,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
);

static int qmi_proxy_nas_srvc_response_update_cache
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *modified_resp,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
);

static int qmi_proxy_wms_srvc_response_update_cache_sglte
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *modified_resp,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
);
static void qmi_proxy_srvc_send_qmi_response
(
  qmi_service_id_type    proxy_srvc_id,
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  void                   *resp_c_struct,
  int                    resp_c_struct_len,
  unsigned long          rx_txn_id
);

void qmi_proxy_nas_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
);

static int qmi_proxy_voice_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
);

static void qmi_proxy_voice_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
);

static int qmi_proxy_pbm_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
);

static void qmi_proxy_pbm_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
);

static int qmi_proxy_wms_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
);

static void qmi_proxy_wms_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
);

static int qmi_proxy_sar_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
);

static void qmi_proxy_sar_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
);

static int qmi_proxy_ims_vt_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
);

static int qmi_proxy_ims_presence_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
);

static void qmi_proxy_ims_vt_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
);

static void qmi_proxy_ims_presence_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
);

static void qmi_proxy_send_response_to_clients
(
  unsigned char *msg,
  int  msg_len,
  qmi_service_id_type srvc_id,
  qmi_client_id_type client_id,
  unsigned char  control_flag,
  int            fd
);

static void qmi_proxy_dms_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
);

static void qmi_proxy_nas_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
);

static void qmi_proxy_wms_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
);

static void qmi_proxy_voice_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
);

static void qmi_proxy_pbm_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
);

static void qmi_proxy_sar_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
);

static void qmi_proxy_ims_vt_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
);

static int qmi_proxy_release_internal_qmi_handle
(
  void
);

static void qmi_proxy_add_txn_entry
(
  qmi_proxy_txn_entry_list_type *txn_entry_list,
  qmi_service_id_type            srvc_id,
  unsigned long                  msg_id,
  void                           *msg,
  uint32_t                       msg_len
);

static boolean qmi_proxy_mcc_same_country
(
  const qmi_proxy_plmn_id_s_type  * const plmn_1,
  const qmi_proxy_plmn_id_s_type  * const plmn_2
);

static boolean qmi_proxy_is_sglte_plmn
(
  const qmi_proxy_plmn_id_s_type * const plmn
);

static void qmi_proxy_use_default_sglte_plmn_list
(
  void
);

static void qmi_proxy_save_sglte_plmn_list
(
  const qmi_proxy_plmn_id_s_type * const plmn
);

static void qmi_proxy_read_sglte_plmn_list
(
  qmi_proxy_plmn_id_s_type * plmn
);

static void qmi_proxy_sglte_update_srv_domain_timers
(
  qmi_proxy_conn_type proxy_conn_type,
  nas_sys_info_ind_msg_v01 * nas_sys_info_ind
);

void qmi_proxy_sglte_update_last_known_plmn
(
  nas_sys_info_ind_msg_v01 * nas_sys_info_ind,
  qmi_proxy_conn_type modem_index
);

static const char *qmi_proxy_get_remote_modem_connection_name
(
  void
);

static int qmi_proxy_get_remote_modem_connection_id
(
  void
);

static const char *qmi_proxy_get_local_modem_connection_name
(
  void
);

static int qmi_proxy_get_local_modem_connection_id
(
  void
);

static qmi_proxy_mode_pref_type qmi_proxy_nas_mask_to_user_mode_pref
(
  int mask,
  nas_gw_acq_order_pref_enum_type_v01 acq_order_pref
);

static int qmi_proxy_user_mode_pref_to_ril_nas_mask
(
  qmi_proxy_mode_pref_type mode_pref
);

static void qmi_proxy_force_sglte_sys_sel_pref_and_oprt_mode
(
  int cs_active,
  int ps_active,
  qmi_proxy_mode_pref_type user_mode_pref,
  int is_sglte_plmn,
  int is_user_request,
  int is_mode_pref_required,
  int async
);

static void qmi_proxy_update_cs_active_ps_active_sglte(
  void
);

static void qmi_proxy_get_cs_active_ps_active_locked(
  qmi_proxy_conn_type *cs_active_modem,
  qmi_proxy_conn_type *ps_active_modem
);

static void qmi_proxy_get_cs_active_ps_active(
  qmi_proxy_conn_type *cs_active_modem,
  qmi_proxy_conn_type *ps_active_modem
);

static void qmi_proxy_sglte_invalidate_last_plmns
(
  void
);

static void qmi_proxy_sglte_invalidate_last_plmns_locked
(
  void
);

static void qmi_proxy_sglte_invalidate_last_plmn
(
  qmi_proxy_conn_type modem_index
);

static void qmi_proxy_sglte_invalidate_last_plmn_locked
(
  qmi_proxy_conn_type modem_index
);

static void qmi_proxy_create_timer
(
  int timer_in_seconds,
  timer_expiry_handler_type timer_expiry_handler
);

static void qmi_proxy_cancel_timer
(
  void
);

static void * qmi_proxy_timer_thread_proc
(
  void * param
);

static void qcril_proxy_signal_handler_sigusr1
(
  int arg
);

void qmi_proxy_sglte_ps_to_g_timer_exp_handler
(
  void
);

void qmi_proxy_sglte_ps_to_lt_timer_exp_handler
(
  void
);
uint16_t qmi_proxy_get_sglte_expanded_sys_sel_pref_mask_for_oos
(
  uint16_t original_mask
);

static int qmi_proxy_set_sys_sel_pref_per_sglte_mode_pref
(
  qmi_proxy_mode_pref_type                        mode_pref,
  int                                         is_sglte_plmn,
  boolean                                         is_user_request,
  nas_set_system_selection_preference_req_msg_v01 *local_nas_set_sys_sel_pref_req,
  nas_set_system_selection_preference_req_msg_v01 *remote_nas_set_sys_sel_pref_req
);

static void qmi_proxy_sglte_update_in_sglte_coverage
(
  void
);

static void qmi_proxy_sglte_update_in_sglte_coverage_locked
(
  void
);

static nas_network_type_mask_type_v01 qmi_proxy_get_current_network_mode_for_scan
(
  qmi_proxy_conn_type modem_index
);

static void qmi_proxy_rx_queue_msg
(
  qmi_client_type        client_handle,
  unsigned long          msg_id,
  qmi_service_id_type    proxy_srvc_id,
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  void                  *decoded_payload,
  uint32_t               decoded_payload_len
);

static int qmi_proxy_is_full_service
(
  nas_sys_info_ind_msg_v01 *nas_sys_info_ind
);

static int qmi_proxy_rx_queue_msg_locked
(
  qmi_client_type        client_handle,
  unsigned long          msg_id,
  qmi_service_id_type    proxy_srvc_id,
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  void                  *decoded_payload,
  uint32_t               decoded_payload_len
);

static int qmi_proxy_correct_sys_sel_pref_srv_reg_restriction
(
  nas_set_system_selection_preference_req_msg_v01 *local_nas_set_sys_sel_pref_req,
  nas_set_system_selection_preference_req_msg_v01 *remote_nas_set_sys_sel_pref_req
);

/*===========================================================================

                            MACROS

===========================================================================*/

/* Log message to Diag */
#define QMI_PROXY_LOG_MSG( lvl, fmt, ... )                                         \
  {                                                                                \
    char buf[ QMI_PROXY_MAX_LOG_MSG_SIZE ];                                        \
                                                                                   \
    qmi_proxy_format_log_msg( buf, QMI_PROXY_MAX_LOG_MSG_SIZE, fmt, __VA_ARGS__ ); \
    qmi_proxy_log_msg_to_adb( lvl, buf );                                          \
    MSG_SPRINTF_1( MSG_SSID_LINUX_DATA, lvl, "%s", buf );                          \
  }

/* Log debug message */
#define QMI_PROXY_DEBUG_MSG(format, ...)   QMI_PROXY_LOG_MSG(MSG_LEGACY_HIGH, "[%d] %s: " format, (int)pthread_self(), __FUNCTION__, __VA_ARGS__)

/* Log error message */
#define QMI_PROXY_ERR_MSG(format, ...)     QMI_PROXY_LOG_MSG(MSG_LEGACY_ERROR, "[%d] %s: " format, (int)pthread_self(), __FUNCTION__, __VA_ARGS__)

/* Macro to handle snprintf */
#define QMI_PROXY_SNPRINTF  snprintf

/* Macro to check whether it is APQ target */
#define QMI_PROXY_APQ_TARGET()        ( qmi_proxy_cfg == QMI_PROXY_CFG_APQ )

/* Macro to check whether it is Multimode target */
#define QMI_PROXY_MULTIMODE_TARGET()  ( qmi_proxy_cfg == QMI_PROXY_CFG_MULTIMODE )

/* Macro to check whether it is SVLTE Type 1 target */
#define QMI_PROXY_SVLTE_1_TARGET()    ( qmi_proxy_cfg == QMI_PROXY_CFG_SVLTE_1 )

/* Macro to check whether it is SVLTE Type 2 target */
#define QMI_PROXY_SVLTE_2_TARGET()    ( qmi_proxy_cfg == QMI_PROXY_CFG_SVLTE_2 )

/* Macro to check whether it is SGLTE target */
#define QMI_PROXY_SGLTE_TARGET()      (( qmi_proxy_cfg == QMI_PROXY_CFG_SGLTE ) || \
                                         ( qmi_proxy_cfg == QMI_PROXY_CFG_SGLTE_2 ))

/* Macro to check whether it is SGLTE type 1 target */
#define QMI_PROXY_SGLTE_1_TARGET()      ( qmi_proxy_cfg == QMI_PROXY_CFG_SGLTE )

/* Macro to check whether it is SGLTE type 2 target */
#define QMI_PROXY_SGLTE_2_TARGET()      ( qmi_proxy_cfg == QMI_PROXY_CFG_SGLTE_2 )

/* Macro to check whether it is CSFB target */
#define QMI_PROXY_CSFB_TARGET()       ( qmi_proxy_cfg == QMI_PROXY_CFG_CSFB )

/* Macro to check whether VOIP is enabled */
#define QMI_PROXY_VOIP_ENABLED() ( qmi_proxy_internal_info.voip_info.is_voip_enabled == TRUE)

/* Macro to check whether proxy is registered on IMS */
#define QMI_PROXY_IMS_REGISTERED() ( qmi_proxy_internal_info.voip_info.is_registered_on_ims == TRUE)

/* Macro to check whether VOIP is active */
#define QMI_PROXY_VOIP_ACTIVE()            ( QMI_PROXY_VOIP_ENABLED() && QMI_PROXY_IMS_REGISTERED() )

/* Macro to check whether remote modem (QSC) is CS active */
#define QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() ( qmi_proxy_internal_info.cs_active_modem == QMI_PROXY_REMOTE_CONN_TYPE )
/* Macro to check whether remote modem (QSC) is PS active */
#define QMI_PROXY_SGLTE_REMOTE_MODEM_PS_ACTIVE() ( qmi_proxy_internal_info.ps_active_modem == QMI_PROXY_REMOTE_CONN_TYPE )

/* Macro to check whether local modem (QSC) is CS active */
#define QMI_PROXY_SGLTE_LOCAL_MODEM_CS_ACTIVE() ( qmi_proxy_internal_info.cs_active_modem == QMI_PROXY_LOCAL_CONN_TYPE )
/* Macro to check whether local modem (QSC) is PS active */
#define QMI_PROXY_SGLTE_LOCAL_MODEM_PS_ACTIVE() ( qmi_proxy_internal_info.ps_active_modem == QMI_PROXY_LOCAL_CONN_TYPE )

/* Macro to check whether local modem (MSM) is online */
#define QMI_PROXY_SGLTE_LOCAL_MODEM_ONLINE() \
  ( QMI_PROXY_SGLTE_LOCAL_MODEM_CS_ACTIVE() || \
    ( QMI_PROXY_SGLTE_LOCAL_MODEM_PS_ACTIVE() && !qmi_proxy_internal_info.number_of_emergency_calls ) )

/* Macro to check whether it is Fusion target */
#define QMI_PROXY_FUSION_TARGET() \
  ( ( qmi_proxy_cfg == QMI_PROXY_CFG_SVLTE_1 ) || \
    ( qmi_proxy_cfg == QMI_PROXY_CFG_SVLTE_2 ) || \
    ( qmi_proxy_cfg == QMI_PROXY_CFG_SGLTE   ) || \
    ( qmi_proxy_cfg == QMI_PROXY_CFG_SGLTE_2 ) || \
    ( qmi_proxy_cfg == QMI_PROXY_CFG_CSFB ) )

/* Macro to check whether it is SVLTE target */
#define QMI_PROXY_SVLTE_TARGET() \
  ( ( qmi_proxy_cfg == QMI_PROXY_CFG_SVLTE_1 ) || ( qmi_proxy_cfg == QMI_PROXY_CFG_SVLTE_2 ) )

/* Macro to check whether a QMI Service is supported by QMI Proxy */
#define QMI_PROXY_SRVC_IS_SUPPORTED( srvc_id ) \
  ( ( qmi_proxy_reg_info_tbl[ qmi_proxy_cfg ] [ srvc_id ].local_srvc_required ) || \
    ( qmi_proxy_reg_info_tbl[ qmi_proxy_cfg ] [ srvc_id ].remote_srvc_required ) )

/* Macro to retrieve the result of QMI Service Request */
#define QMI_PROXY_OP_RESULT( rc, response )                                              \
  ( ( rc != QMI_NO_ERR ) ? rc :                                                          \
                           ( ( ( response == NULL ) ||                                   \
                               ( response->resp.result == QMI_RESULT_SUCCESS_V01 ) ||    \
                               ( ( response->resp.result == QMI_RESULT_FAILURE_V01 ) &&  \
                                 ( response->resp.error == QMI_ERR_NO_EFFECT_V01 ) )     \
                             ) ? QMI_NO_ERR : response->resp.result ) )

/* Macro to check the failure/sucess of QMI Service Request */
#define QMI_PROXY_RESPONSE_SUCCESS( rc, response ) \
  ( ( rc = QMI_PROXY_OP_RESULT( rc, response ) ) == QMI_NO_ERR )

#define QMI_PROXY_THREAD_DATA_CONN_ID_MASK  0xFFFF
#define QMI_PROXY_THREAD_DATA_CONN_ID_SHIFT 16
#define QMI_PROXY_THREAD_DATA_SRVC_ID_MASK  0xFFFF

/* Compose thread data */
#define QMI_PROXY_COMPOSE_THREAD_DATA( conn_id, srvc_id )                                         \
  ( ( ( conn_id & QMI_PROXY_THREAD_DATA_CONN_ID_MASK ) << QMI_PROXY_THREAD_DATA_CONN_ID_SHIFT ) | \
    ( srvc_id & QMI_PROXY_THREAD_DATA_SRVC_ID_MASK ) )

/* Extract Conn ID from thread data */
#define QMI_PROXY_THREAD_DATA_TO_CONN_ID( user_data ) \
  ( ( user_data >> QMI_PROXY_THREAD_DATA_CONN_ID_SHIFT ) & QMI_PROXY_THREAD_DATA_CONN_ID_MASK )

/* Extract Service ID from thread data */
#define QMI_PROXY_THREAD_DATA_TO_SRVC_ID( user_data ) \
  ( user_data & QMI_PROXY_THREAD_DATA_SRVC_ID_MASK )

#define QMI_PROXY_UNSOL_IND_CB_CONN_TYPE_MASK    0xF
#define QMI_PROXY_UNSOL_IND_CB_CONN_TYPE_SHIFT   28
#define QMI_PROXY_UNSOL_IND_CB_SRVC_ID_MASK      0xFFF
#define QMI_PROXY_UNSOL_IND_CB_SRVC_ID_SHIFT     16
#define QMI_PROXY_UNSOL_IND_CB_CLIENT_ID_MASK    0xFF
#define QMI_PROXY_UNSOL_IND_CB_CLIENT_ID_SHIFT   8
#define QMI_PROXY_UNSOL_IND_CB_CONN_ID_MASK      0xFF

/* Compose user data for unsolicited indication callback */
#define QMI_PROXY_COMPOSE_UNSOL_IND_CB_USER_DATA( conn_type, srvc_id, client_id, conn_id )                \
  ( ( ( conn_type & QMI_PROXY_UNSOL_IND_CB_CONN_TYPE_MASK ) << QMI_PROXY_UNSOL_IND_CB_CONN_TYPE_SHIFT ) | \
    ( ( srvc_id & QMI_PROXY_UNSOL_IND_CB_SRVC_ID_MASK ) << QMI_PROXY_UNSOL_IND_CB_SRVC_ID_SHIFT ) |         \
    ( ( client_id & QMI_PROXY_UNSOL_IND_CB_CLIENT_ID_MASK ) << QMI_PROXY_UNSOL_IND_CB_CLIENT_ID_SHIFT ) | \
    ( conn_id & QMI_PROXY_UNSOL_IND_CB_CONN_ID_MASK ) )

/* Extract Conn Type from unsolicited indication callback data */
#define QMI_PROXY_UNSOL_IND_CB_USER_DATA_TO_CONN_TYPE( user_data ) \
  ( ( user_data >> QMI_PROXY_UNSOL_IND_CB_CONN_TYPE_SHIFT ) & QMI_PROXY_UNSOL_IND_CB_CONN_TYPE_MASK )

/* Extract Service ID from unsolicited indication callback data */
#define QMI_PROXY_UNSOL_IND_CB_USER_DATA_TO_SRVC_ID( user_data ) \
  ( ( user_data >> QMI_PROXY_UNSOL_IND_CB_SRVC_ID_SHIFT ) & QMI_PROXY_UNSOL_IND_CB_SRVC_ID_MASK )

/* Extract Client ID from unsolicited indication callback data */
#define QMI_PROXY_UNSOL_IND_CB_USER_DATA_TO_CLIENT_ID( user_data ) \
  ( ( user_data >> QMI_PROXY_UNSOL_IND_CB_CLIENT_ID_SHIFT ) & QMI_PROXY_UNSOL_IND_CB_CLIENT_ID_MASK )

/* Extract Conn ID from unsolicited indication callback data */
#define QMI_PROXY_UNSOL_IND_CB_USER_DATA_TO_CONN_ID( user_data )   \
  ( user_data & QMI_PROXY_UNSOL_IND_CB_CONN_ID_MASK )

#define QMI_PROXY_TXN_DATA_CONN_TYPE_MASK  0xF
#define QMI_PROXY_TXN_DATA_CONN_TYPE_SHIFT 28
#define QMI_PROXY_TXN_DATA_TXN_INDEX_MASK  0xF
#define QMI_PROXY_TXN_DATA_TXN_INDEX_SHIFT 24
#define QMI_PROXY_TXN_DATA_TXN_KEY_MASK    0xFFFFFF

/* Compose user data for a transaction */
#define QMI_PROXY_COMPOSE_TXN_USER_DATA( conn_type, txn_index, txn_key )                          \
  ( ( ( conn_type & QMI_PROXY_TXN_DATA_CONN_TYPE_MASK ) << QMI_PROXY_TXN_DATA_CONN_TYPE_SHIFT ) | \
    ( ( txn_index & QMI_PROXY_TXN_DATA_TXN_INDEX_MASK ) << QMI_PROXY_TXN_DATA_TXN_INDEX_SHIFT ) | \
    ( txn_key & QMI_PROXY_TXN_DATA_TXN_KEY_MASK ) )

/* Extract Conn Type from transaction user data */
#define QMI_PROXY_TXN_USER_DATA_TO_CONN_TYPE( user_data )     \
  ( ( user_data >> QMI_PROXY_TXN_DATA_CONN_TYPE_SHIFT ) & QMI_PROXY_TXN_DATA_CONN_TYPE_MASK )

/* Extract TXN Index from transaction user data */
#define QMI_PROXY_TXN_USER_DATA_TO_TXN_INDEX( user_data )     \
  ( ( user_data >> QMI_PROXY_TXN_DATA_TXN_INDEX_SHIFT ) & QMI_PROXY_TXN_DATA_TXN_INDEX_MASK )

/* Extract Txn Key from transaction user data */
#define QMI_PROXY_TXN_USER_DATA_TO_TXN_KEY( user_data )       \
  ( user_data & QMI_PROXY_TXN_DATA_TXN_KEY_MASK )

#define QMI_PROXY_TXN_KEY_SRVC_ID_MASK  0xFF
#define QMI_PROXY_TXN_KEY_SRVC_ID_SHIFT 16
#define QMI_PROXY_TXN_KEY_TXN_ID_MASK   0xFFFF

/* Compose Txn Key for a transaction */
#define QMI_PROXY_COMPOSE_TXN_KEY( srvc_id, txn_id ) \
  ( ( ( srvc_id & QMI_PROXY_TXN_KEY_SRVC_ID_MASK ) << QMI_PROXY_TXN_KEY_SRVC_ID_SHIFT ) | ( txn_id & QMI_PROXY_TXN_KEY_TXN_ID_MASK ) )

/* Extract Service ID from transaction key */
#define QMI_PROXY_TXN_KEY_TO_SRVC_ID( user_data ) ( ( user_data >> QMI_PROXY_TXN_KEY_SRVC_ID_SHIFT ) & QMI_PROXY_TXN_KEY_SRVC_ID_MASK )

/* Extract Txn ID from transaction key */
#define QMI_PROXY_TXN_KEY_TO_TXN_ID( user_data )  ( user_data & QMI_PROXY_TXN_KEY_TXN_ID_MASK )

#define QMI_PROXY_NAS_ASYNCB_CONN_TYPE_MASK    0xFFFF
#define QMI_PROXY_NAS_ASYNCB_CONN_TYPE_SHIFT   16
#define QMI_PROXY_NAS_ASYNCB_MODE_PREF_MASK 0xFFFF

/* Compose NAS mode preference asychronous callback user data */
#define QMI_PROXY_COMPOSE_NAS_MODE_PREF_ASYNCB_USER_DATA( conn_type, mode_pref )                      \
  ( ( ( conn_type & QMI_PROXY_NAS_ASYNCB_CONN_TYPE_MASK ) << QMI_PROXY_NAS_ASYNCB_CONN_TYPE_SHIFT ) | \
    ( mode_pref & QMI_PROXY_NAS_ASYNCB_MODE_PREF_MASK ) )

/* Extract Conn Type from NAS mode preference Async CB data */
#define QMI_PROXY_NAS_ASYNCB_USER_DATA_TO_CONN_TYPE( user_data ) \
  ( ( user_data >> QMI_PROXY_NAS_ASYNCB_CONN_TYPE_SHIFT ) & QMI_PROXY_NAS_ASYNCB_CONN_TYPE_MASK )

/* Extract Mode Preference from NAS mode preference Async CB data */
#define QMI_PROXY_NAS_ASYNCB_USER_DATA_TO_MODE_PREF( user_data ) \
    ( user_data & QMI_PROXY_NAS_ASYNCB_MODE_PREF_MASK )

#define QMI_PROXY_DMS_ASYNCB_CONN_TYPE_MASK  0xFFFF
#define QMI_PROXY_DMS_ASYNCB_CONN_TYPE_SHIFT 16
#define QMI_PROXY_DMS_ASYNCB_OPRT_MODE_MASK  0xFFFF

/* Compose DMS operating mode asynchronous callback user data */
#define QMI_PROXY_COMPOSE_DMS_OPRT_MODE_ASYNCB_USER_DATA( conn_type, oprt_mode )                      \
  ( ( ( conn_type & QMI_PROXY_DMS_ASYNCB_CONN_TYPE_MASK ) << QMI_PROXY_DMS_ASYNCB_CONN_TYPE_SHIFT ) | \
    ( oprt_mode & QMI_PROXY_DMS_ASYNCB_OPRT_MODE_MASK ) )

/* Extract Conn Type from DMS operating mode Async CB data */
#define QMI_PROXY_DMS_OPRT_MODE_ASYNCB_USER_DATA_TO_CONN_TYPE( user_data ) \
  ( ( user_data >> QMI_PROXY_DMS_ASYNCB_CONN_TYPE_SHIFT ) & QMI_PROXY_DMS_ASYNCB_CONN_TYPE_MASK )

/* Extract Oprt Mode from DMS operating mode Async CB data */
#define QMI_PROXY_DMS_OPRT_MODE_ASYNCB_USER_DATA_TO_OPRT_MODE( user_data ) \
    ( user_data & QMI_PROXY_DMS_ASYNCB_OPRT_MODE_MASK )

/* Figure out the connection type of peer modem */
#define QMI_PROXY_PEER_CONN_TYPE( conn_type )     ( ( conn_type + 1 ) % 2 )

/* Check whether remote modem should be active per selected modem preference */
#define QMI_PROXY_REMOTE_MODEM_IS_ACTIVE()                                                       \
  ( ( qmi_proxy_internal_info.svlte_mode_pref == QMI_PROXY_MODE_PREF_CDMA_EVDO ) ||              \
    ( qmi_proxy_internal_info.svlte_mode_pref == QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO ) ||          \
    ( qmi_proxy_internal_info.svlte_mode_pref == QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA ) ||          \
    ( qmi_proxy_internal_info.svlte_mode_pref == QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO_GSM_WCDMA ) )

/* Mode preference marks */
#define QMI_PROXY_MODE_PREF_MASK_CDMA  ( 1 << QMI_NAS_RAT_MODE_PREF_CDMA2000_1X_BIT_V01 )
#define QMI_PROXY_MODE_PREF_MASK_EVDO  ( 1 << QMI_NAS_RAT_MODE_PREF_CDMA2000_HRPD_BIT_V01 )
#define QMI_PROXY_MODE_PREF_MASK_GSM   ( 1 << QMI_NAS_RAT_MODE_PREF_GSM_BIT_V01 )
#define QMI_PROXY_MODE_PREF_MASK_UMTS  ( 1 << QMI_NAS_RAT_MODE_PREF_UMTS_BIT_V01 )
#define QMI_PROXY_MODE_PREF_MASK_LTE   ( 1 << QMI_NAS_RAT_MODE_PREF_LTE_BIT_V01 )
#define QMI_PROXY_MODE_PREF_MASK_TDSCDMA   ( 1 << QMI_NAS_RAT_MODE_PREF_TDSCDMA_BIT_V01 )

#define QMI_PROXY_MODE_PREF_CDMA_GSM_UMTS_MASK  \
  ( QMI_PROXY_MODE_PREF_MASK_CDMA | QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS )

#define QMI_PROXY_MODE_PREF_GSM_UMTS_MASK  \
  ( QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS )

#define QMI_PROXY_MODE_PREF_GSM_UMTS_LTE_MASK  \
  ( QMI_PROXY_MODE_PREF_GSM_UMTS_MASK | QMI_PROXY_MODE_PREF_MASK_LTE )

#define QMI_PROXY_MODE_PREF_EVDO_LTE_MASK       \
  ( QMI_PROXY_MODE_PREF_MASK_EVDO | QMI_PROXY_MODE_PREF_MASK_LTE )

#define QMI_PROXY_CONN_TYPE_IS_VALID( conn_type ) ( conn_type <= QMI_PROXY_MAX_CONN_TYPE )

#define QMI_PROXY_TXN_INDEX_IS_VALID( txn_id ) ( txn_id < QMI_PROXY_MAX_SRVC_TXNS )

#define QMI_PROXY_SRVC_ID_IS_VALID( srvc_id ) ( srvc_id < QMI_MAX_SERVICES )
/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/* QMI Proxy Current Configuration */
static qmi_proxy_cfg_type qmi_proxy_cfg = QMI_PROXY_CFG_MULTIMODE;

/* Indicates whether QMI Proxy debug messages logged on ADB or not */
static boolean qmi_proxy_adb_log_on = FALSE;

/* QMI Proxy Configuration Name */
static char *qmi_proxy_cfg_name[ QMI_PROXY_MAX_CFGS + 1 ] = { "MSM", "SVLTE1", "CSFB", "SVLTE2A", "APQ", "SGLTE", "SGLTE2", "Unknown" };

/* QMI Proxy Modem Name */
static char *qmi_proxy_modem_name[ QMI_PROXY_MAX_CONN_TYPE + 2 ] = { "Local modem", "Remote modem", "Unknown", "Both modems" };

/* QMI Proxy Service Name */
static char *qmi_proxy_srvc_name[ QMI_MAX_SERVICES + 1 ] =
  { "CTL", "WDS", "DMS", "NAS", "QOS", "WMS", "Unknown", "EAP", "ATCOP", "Voice", "CAT", "UIM", "PBM",
    "Unknown", "Unknown", "Unknown", "Unknown", "SAR", "IMS VIDEO", "Unknown", "CSD", "Unknown",
    "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "IMS VT", "IMS PRESENCE", "VS SAR" };

/* QMI Proxy Service Message Type Name */
static char *qmi_proxy_msg_type_name[ QMI_IDL_NUM_MSG_TYPES ] = { "Request", "Response", "Indication" };

/* QMI Proxy SVLTE Mode Preference Name */
static char *qmi_proxy_svlte_mode_pref_name[ QMI_PROXY_MAX_MODE_PREFS ] =
 { "GSM/WCDMA Preferred", "GSM Only", "WCDMA Only", "GSM/WCDMA Auto", "CDMA/EVDO", "CDMA Only", "EVDO Only",
   "GSM/WCDMA/CDMA/EVDO", "LTE/CDMA/EVDO", "LTE/GSM/WCDMA", "LTE/CDMA/EVDO/GSM/WCDMA", "LTE Only" };

/* QMI Proxy Registration Info per Configuration */
static qmi_proxy_reg_info_type qmi_proxy_reg_info_tbl[ QMI_PROXY_MAX_CFGS ] [ QMI_MAX_SERVICES ] =
{
  /* Multimode configuration */
  {
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* WDS service */
    { TRUE,  FALSE, FALSE, FALSE }, /* DMS service */
    { TRUE,  FALSE, FALSE, FALSE }, /* NAS service */
    { FALSE, FALSE, FALSE, FALSE }, /* QOS service */
    { TRUE,  FALSE, FALSE, FALSE }, /* WMS service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* EAP service */
    { FALSE, FALSE, FALSE, FALSE }, /* ATCOP service */
    { TRUE,  FALSE, FALSE, FALSE }, /* Voice service */
    { FALSE, FALSE, FALSE, FALSE }, /* CAT service */
    { FALSE, FALSE, FALSE, FALSE }, /* UIM service */
    { TRUE,  FALSE, FALSE, FALSE }, /* PBM service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { TRUE,  FALSE, FALSE, FALSE }, /* SAR service */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS Video service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* CSD service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS VTservice */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS Presence service */
    { FALSE, FALSE, FALSE, FALSE }  /* VS SAR Service */
  },
  /* SVLTE Type 1 configuration */
  {
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* WDS service */
    { TRUE,  TRUE,  TRUE,  TRUE  }, /* DMS service */
    { TRUE,  TRUE,  TRUE,  TRUE  }, /* NAS service */
    { FALSE, FALSE, FALSE, FALSE }, /* QOS service */
    { TRUE,  TRUE,  FALSE, FALSE }, /* WMS service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* EAP service */
    { FALSE, FALSE, FALSE, FALSE }, /* ATCOP service */
    { TRUE,  FALSE, FALSE, FALSE }, /* Voice service */
    { FALSE, FALSE, FALSE, FALSE }, /* CAT service */
    { FALSE, FALSE, FALSE, FALSE }, /* UIM service */
    { FALSE, TRUE,  FALSE, FALSE }, /* PBM service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { TRUE,  TRUE,  FALSE, FALSE }, /* SAR service */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS Video service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* CSD service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS VTservice */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS Presence service */
    { FALSE, FALSE, FALSE, FALSE }  /* VS SAR Service */
  },
  /* CSFB configuration */
  {
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* WDS service */
    { FALSE, TRUE,  FALSE, FALSE }, /* DMS service */
    { FALSE, TRUE,  FALSE, FALSE }, /* NAS service */
    { FALSE, FALSE, FALSE, FALSE }, /* QOS service */
    { FALSE, TRUE,  FALSE, FALSE }, /* WMS service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* EAP service */
    { FALSE, FALSE, FALSE, FALSE }, /* ATCOP service */
    { FALSE, TRUE,  FALSE, FALSE }, /* Voice service */
    { FALSE, FALSE, FALSE, FALSE }, /* CAT service */
    { FALSE, FALSE, FALSE, FALSE }, /* UIM service */
    { FALSE, TRUE,  FALSE, FALSE }, /* PBM service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, TRUE,  FALSE, FALSE }, /* SAR service */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS Video service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* CSD service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, TRUE,  FALSE, FALSE }, /* IMS VTservice */
    { FALSE, TRUE,  FALSE, FALSE }, /* IMS Presence service */
    { FALSE, FALSE, FALSE, FALSE }  /* VS SAR Service */
  },
  /* SVLTE Type 2 configuration */
  {
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* WDS service */
    { TRUE,  TRUE,  TRUE,  TRUE  }, /* DMS service */
    { TRUE,  TRUE,  TRUE,  TRUE  }, /* NAS service */
    { FALSE, FALSE, FALSE, FALSE }, /* QOS service */
    { TRUE,  TRUE,  FALSE, FALSE }, /* WMS service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* EAP service */
    { FALSE, FALSE, FALSE, FALSE }, /* ATCOP service */
    { TRUE,  FALSE, FALSE, FALSE }, /* Voice service */
    { FALSE, FALSE, FALSE, FALSE }, /* CAT service */
    { FALSE, FALSE, FALSE, FALSE }, /* UIM service */
    { FALSE, TRUE,  FALSE, FALSE }, /* PBM service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { TRUE,  TRUE,  FALSE, FALSE }, /* SAR service */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS Video service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* CSD service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, TRUE,  FALSE, FALSE }, /* IMS VTservice */
    { FALSE, TRUE,  FALSE, FALSE }, /* IMS Presence service */
    { FALSE, FALSE, FALSE, FALSE }  /* VS SAR Service */
  },
  /* APQ configuration */
  {
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* WDS service */
    { FALSE, FALSE, FALSE, FALSE }, /* DMS service */
    { FALSE, FALSE, FALSE, FALSE }, /* NAS service */
    { FALSE, FALSE, FALSE, FALSE }, /* QOS service */
    { FALSE, FALSE, FALSE, FALSE }, /* WMS service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* EAP service */
    { FALSE, FALSE, FALSE, FALSE }, /* ATCOP service */
    { FALSE, FALSE, FALSE, FALSE }, /* Voice service */
    { FALSE, FALSE, FALSE, FALSE }, /* CAT service */
    { FALSE, FALSE, FALSE, FALSE }, /* UIM service */
    { FALSE, FALSE, FALSE, FALSE }, /* PBM service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* SAR service */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS Video service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* CSD service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS VTs ervice */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS Presence service */
    { FALSE, FALSE, FALSE, FALSE }  /* VS SAR Service */
  },
  /* SGLTE configuration */
  {
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* WDS service */
    { TRUE,  TRUE,  TRUE,  TRUE  }, /* DMS service */
    { TRUE,  TRUE,  TRUE,  TRUE  }, /* NAS service */
    { FALSE, FALSE, FALSE, FALSE }, /* QOS service */
    { TRUE,  TRUE,  FALSE, FALSE }, /* WMS service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* EAP service */
    { FALSE, FALSE, FALSE, FALSE }, /* ATCOP service */
    { TRUE,  TRUE,  FALSE, FALSE }, /* Voice service */
    { FALSE, FALSE, FALSE, FALSE }, /* CAT service */
    { FALSE, FALSE, FALSE, FALSE }, /* UIM service */
    { TRUE,  TRUE,  FALSE, FALSE }, /* PBM service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { TRUE,  TRUE,  FALSE, FALSE }, /* SAR service */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS Video service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* CSD service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS VTs ervice */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS Presence service */
    { FALSE, FALSE, FALSE, FALSE }  /* VS SAR Service */
  },
  /* SGLTE2 configuration */
  {
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* WDS service */
    { TRUE,  TRUE,  TRUE,  TRUE  }, /* DMS service */
    { TRUE,  TRUE,  TRUE,  TRUE  }, /* NAS service */
    { FALSE, FALSE, FALSE, FALSE }, /* QOS service */
    { TRUE,  TRUE,  FALSE, FALSE }, /* WMS service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* EAP service */
    { FALSE, FALSE, FALSE, FALSE }, /* ATCOP service */
    { TRUE,  TRUE,  FALSE, FALSE }, /* Voice service */
    { FALSE, FALSE, FALSE, FALSE }, /* CAT service */
    { FALSE, FALSE, FALSE, FALSE }, /* UIM service */
    { TRUE,  TRUE,  FALSE, FALSE }, /* PBM service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { TRUE,  TRUE,  FALSE, FALSE }, /* SAR service */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS Video service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* CSD service */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* Service not implemented */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS VTs ervice */
    { FALSE, FALSE, FALSE, FALSE }, /* IMS Presence service */
    { FALSE, FALSE, FALSE, FALSE }  /* VS SAR Service */
  }
};

/* List of QMI service arbitration handlers */
static qmi_proxy_srvc_arb_hdlr_type qmi_proxy_srvc_arb_hdlr_tbl[ QMI_MAX_SERVICES ] =
{
  NULL,                          /* Service not implemented */
  NULL,                          /* WDS service */
  qmi_proxy_dms_srvc_arb_hdlr,   /* DMS service */
  qmi_proxy_nas_srvc_arb_hdlr,   /* NAS service */
  NULL,                          /* QOS service */
  qmi_proxy_wms_srvc_arb_hdlr,   /* WMS service */
  NULL,                          /* Service not implemented */
  NULL,                          /* EAP service */
  NULL,                          /* ATCOP service */
  qmi_proxy_voice_srvc_arb_hdlr, /* Voice service */
  NULL,                          /* CAT service */
  NULL,                          /* UIM service */
  qmi_proxy_pbm_srvc_arb_hdlr,   /* PBM service */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  qmi_proxy_sar_srvc_arb_hdlr,   /* SAR service */
  NULL,                          /* IMS Video service  */
  NULL,                          /* Service not implemented */
  NULL,                          /* CSD service */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  qmi_proxy_ims_vt_srvc_arb_hdlr,   /* IMS VT service */
  qmi_proxy_ims_presence_srvc_arb_hdlr,   /* IMS Presence service */
  NULL                           /* Service not implemented */
};

/* List of QMI Proxy service unsolicited indication callback handlers */
static qmi_proxy_rx_ind_hdlr_type qmi_proxy_srvc_unsol_ind_cb_tbl[ QMI_MAX_SERVICES ] =
{
  NULL,                               /* Service not implemented */
  NULL,                               /* WDS service */
  qmi_proxy_dms_srvc_unsol_ind_cb,    /* DMS service */
  qmi_proxy_nas_srvc_unsol_ind_cb,    /* NAS service */
  NULL,                               /* QOS service */
  qmi_proxy_wms_srvc_unsol_ind_cb,    /* WMS service */
  NULL,                               /* Service not implemented */
  NULL,                               /* EAP service */
  NULL,                               /* ATCOP service */
  qmi_proxy_voice_srvc_unsol_ind_cb,  /* Voice service */
  NULL,                               /* CAT service */
  NULL,                               /* UIM service */
  qmi_proxy_pbm_srvc_unsol_ind_cb,    /* PBM service */
  NULL,                               /* Service not implemented */
  NULL,                               /* Service not implemented */
  NULL,                               /* Service not implemented */
  NULL,                               /* Service not implemented */
  qmi_proxy_sar_srvc_unsol_ind_cb,    /* SAR service */
  NULL,                               /* IMS Video service */
  NULL,                               /* Service not implemented */
  NULL,                               /* CSD service */
  NULL,                               /* Service not implemented */
  NULL,                               /* Service not implemented */
  NULL,                               /* Service not implemented */
  NULL,                               /* Service not implemented */
  NULL,                               /* Service not implemented */
  NULL,                               /* Service not implemented */
  NULL,                               /* Service not implemented */
  qmi_proxy_ims_vt_srvc_unsol_ind_cb,    /* IMS VT service */
  qmi_proxy_ims_presence_srvc_unsol_ind_cb,    /* IMS Presence service */
  NULL                                /* Service not implemented */
};

static qmi_proxy_srvc_err_hdlr_type qmi_proxy_srvc_err_hdlr_tbl[ QMI_MAX_SERVICES ] =
{
  NULL,                          /* Service not implemented */
  NULL,                          /* WDS service */
  qmi_proxy_dms_err_resp_hdlr,   /* DMS service */
  qmi_proxy_nas_err_resp_hdlr,   /* NAS service */
  NULL,                          /* QOS service */
  qmi_proxy_wms_err_resp_hdlr,   /* WMS service */
  NULL,                          /* Service not implemented */
  NULL,                          /* EAP service */
  NULL,                          /* ATCOP service */
  qmi_proxy_voice_err_resp_hdlr, /* Voice service */
  NULL,                          /* CAT service */
  NULL,                          /* UIM service */
  qmi_proxy_pbm_err_resp_hdlr,   /* PBM service */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  qmi_proxy_sar_err_resp_hdlr,   /* SAR service */
  NULL,                          /* IMS Video service */
  NULL,                          /* Service not implemented */
  NULL,                          /* CSD service */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  NULL,                          /* Service not implemented */
  qmi_proxy_ims_vt_err_resp_hdlr,   /* IMS VT service */
  NULL,                          /* IMS Presence service */
  NULL                           /* Service not implemented */
};

/* Indicates whether QMI Proxy has registered for QMI System Events */
boolean qmi_proxy_sys_event_registered = FALSE;

/* QMI Init handle */
int qmi_proxy_init_handle = 0;

/* List of QMI service objects */
static qmi_idl_service_object_type qmi_proxy_srvc_obj_tbl[ QMI_MAX_SERVICES ];

/* Mutex used during the Initialization/Release of QMI Proxy */
QMI_PLATFORM_MUTEX_CREATE_AND_INIT_STATIC_MUTEX( qmi_proxy_mutex );

/* Indicates whether QMI Proxy initialization is done */
static boolean qmi_proxy_initialization_done = FALSE;

/* QMI Proxy Internal Info */
static qmi_proxy_internal_info_type qmi_proxy_internal_info;

/* Next available QMI Proxy client ID */
static uint8 qmi_proxy_client_id;

/* Mutex used during the allocation of QMI Proxy client ID */
static QMI_PLATFORM_MUTEX_DATA_TYPE qmi_proxy_client_id_mutex;

/* List of QMI Proxy Client structures */
static qmi_proxy_client_info_type *qmi_proxy_client_info_tbl[ QMI_MAX_SERVICES ];

/* List of mutexes used during allocation/de-allocation/lookup of client info */
static QMI_PLATFORM_MUTEX_DATA_TYPE qmi_proxy_client_list_mutex_tbl[ QMI_MAX_SERVICES ];

/* Next available QMI Proxy Transaction ID */
static qmi_proxy_txn_id_type qmi_proxy_txn_id;

/* Mutex used during the allocation of QMI Proxy transaction ID */
static QMI_PLATFORM_MUTEX_DATA_TYPE qmi_proxy_txn_id_mutex;

/* List of QMI Proxy Transaction structures */
static qmi_proxy_txn_info_type *qmi_proxy_txn_info_tbl[ QMI_MAX_SERVICES ];

/* List of mutexes used during allocation/de-allocation/lookup of transactions */
static QMI_PLATFORM_MUTEX_DATA_TYPE qmi_proxy_txn_list_mutex_tbl[ QMI_MAX_SERVICES ];

/* File descriptor sets */
static fd_set qmi_proxy_qmux_master_fd_set, qmi_proxy_qmux_read_fd_set;
static fd_set qmi_proxy_tether_master_fd_set, qmi_proxy_tether_read_fd_set;

/* Listener File descriptor */
static int qmi_proxy_qmux_listener_fd, qmi_proxy_tether_listener_fd;

/* maximum file descriptor number */
static int qmi_proxy_qmux_max_fd, qmi_proxy_tether_max_fd;;

/* Property for logging the memory allocation/free messages */
static boolean qmi_proxy_mem_leak_debug_on = FALSE;

/* Counter to track outstanding memory for detecting memory leak */
static int qmi_proxy_mem_counter = 0;

/* Mutex for memory counter */
static QMI_PLATFORM_MUTEX_DATA_TYPE qmi_proxy_mem_counter_mutex;

/* List of SGLTE PLMNs */
static qmi_proxy_plmn_id_s_type sglte_plmn_list[QMI_PROXY_MAX_SGLTE_PLMN_ENTRIES];

/* QMI Proxy SGLTE test mode enabled or not */
static boolean qmi_proxy_sglte_is_test_mode = FALSE;
/* SGLTE Hystersis timers */
static qmi_proxy_sglte_hystersis_timer_info_type        qmi_proxy_sglte_hystersis_timer_info;
static qmi_proxy_sglte_hystersis_timer_helper_info_type qmi_proxy_sglte_hystersis_helper_info;
QMI_PLATFORM_MUTEX_CREATE_AND_INIT_STATIC_MUTEX(qmi_proxy_timer_mutex);

static QMI_PLATFORM_SIGNAL_DATA_TYPE qmi_proxy_rx_msg_queue_signal;
static qmi_proxy_queue *qmi_proxy_rx_msg_queue;

#include "qmi_proxy_queue.c"
#include "qmi_proxy_sm.c"
#include "qmi_proxy_sglte_sm.c"

/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*=========================================================================
  FUNCTION:  qmi_proxy_format_log_msg

===========================================================================*/
/*!
    @brief
    Format debug message for logging.

    @return
    None
*/
/*=========================================================================*/
void qmi_proxy_format_log_msg
(
  char *buf_ptr,
  int  buf_size,
  char *fmt,
  ...
)
{
  va_list ap;

  /*-----------------------------------------------------------------------*/

  if ( ( buf_ptr != NULL ) && ( buf_size > 0 ) )
  {
    va_start( ap, fmt );

    vsnprintf( buf_ptr, buf_size, fmt, ap );

    va_end( ap );

  }

} /* qmi_proxy_format_log_msg */


/*=========================================================================
  FUNCTION:  qmi_proxy_log_msg_to_adb

===========================================================================*/
/*!
    @brief
    Log debug message to ADB.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_log_msg_to_adb
(
  int  lvl,
  char *msg_ptr
)
{
  if ( qmi_proxy_adb_log_on )
  {
    switch ( lvl )
    {
      case MSG_LEGACY_ERROR:
      case MSG_LEGACY_FATAL:
        LOGE( "qmi_proxy: %s", msg_ptr );
        break;

      case MSG_LEGACY_HIGH:
        LOGD( "qmi_proxy: %s", msg_ptr );
        break;

      default:
        break;
    }
  }

} /* qmi_proxy_log_msg_to_adb */


/*===========================================================================

  FUNCTION:  qmi_proxy_malloc

===========================================================================*/
/*!
    @brief
    Allocate memory from heap and nullify the memory

    @return
    Pointer to allocated memory region.
*/
/*=========================================================================*/
void *qmi_proxy_malloc
(
  size_t size
)
{
  void *mem_ptr = NULL;
  int temp_mem_counter;
  /*-----------------------------------------------------------------------*/

  if ( size > 0 )
  {
    mem_ptr = malloc( size );
    if ( mem_ptr != NULL )
    {
      memset( mem_ptr, 0, size );
    }
    else
    {
      QMI_PROXY_DEBUG_MSG( "%s: Fail to allocate buffer from heap\n", __FUNCTION__ );
    }
  }
  else
  {
    QMI_PROXY_DEBUG_MSG( "%s: Invalid buffer size 0\n", __FUNCTION__ );
  }

  if ( mem_ptr != NULL )
  {
    QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_mem_counter_mutex );
    qmi_proxy_mem_counter++;
    temp_mem_counter = qmi_proxy_mem_counter;
    QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_mem_counter_mutex );

    if ( qmi_proxy_mem_leak_debug_on )
    {
      QMI_PROXY_DEBUG_MSG( "%s: allocated memory, count = %d, address = %u \n", __FUNCTION__, temp_mem_counter, mem_ptr);
    }
  }
  return mem_ptr;

} /* qmi_proxy_malloc */


/*===========================================================================

  FUNCTION:  qmi_proxy_free

===========================================================================*/
/*!
    @brief
    Free the allocated memory from heap and set the pointer to NULL.

    @return
    None.
*/
/*=========================================================================*/
void qmi_proxy_free
(
  void **allocated_memory
)
{
  int temp_mem_counter;

  if ( *allocated_memory != NULL )
  {
    QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_mem_counter_mutex );
    qmi_proxy_mem_counter--;
    temp_mem_counter = qmi_proxy_mem_counter;
    QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_mem_counter_mutex );

    if ( qmi_proxy_mem_leak_debug_on )
    {
      QMI_PROXY_DEBUG_MSG( "%s: freeing the allocated memory, count = %d, address = %u\n", __FUNCTION__, temp_mem_counter,  *allocated_memory);
    }

    free(*allocated_memory);
    *allocated_memory = NULL;
  }
  else
  {
    QMI_PROXY_DEBUG_MSG( "%s: NULL paramter passed\n", __FUNCTION__);
  }
} /* qmi_proxy_free */

/*=========================================================================
  FUNCTION:  qmi_proxy_print_hex

===========================================================================*/
/*!
    @brief
    Print specified buffer context in hexadecimal format.

    @return
    None
*/
/*=========================================================================*/
void qmi_proxy_print_hex
(
  unsigned char *msg,
  int            msg_len
)
{
  char output_buf[ QMI_PROXY_MAX_OUTPUT_BUF_SIZE ];
  unsigned char val;
  char *p;
  int i;

  /*-----------------------------------------------------------------------*/

  while ( msg_len > 0 )
  {
    p = output_buf;

    /* Collect QMI_PROXY_MAX_BUFFER_BYTES_PER_LINE bytes of buffer for display */
    for ( i = 0; ( i < QMI_PROXY_MAX_BUFFER_BYTES_PER_LINE ) && ( msg_len > 0 ); i++ )
    {
      /* First digit */
      val = ( *msg >> 4 ) & 0x0F;
      if ( val <= 9 )
      {
        *p++ = val + '0';
      }
      else
      {
        *p++ = ( val - 10 ) + 'A';
      }

      /* Second digit... ugly copied code */
      val = *msg & 0x0F;
      if ( val <= 9 )
      {
        *p++ = val + '0';
      }
      else
      {
        *p++ = ( val - 10 ) + 'A';
      }

      /* Add a space, and increment msg pointer */
      *p++ = ' ';
      msg++;
      msg_len--;
    }

    /* Add \n and NULL terminator and print out */
    *p++ = '\n';
    *p = '\0';
    QMI_PROXY_DEBUG_MSG( "%s", output_buf );
  }

} /* qmi_proxy_print_hex */


/*===========================================================================
  FUNCTION  qmi_proxy_read_ctl_srvc_msg_hdr
===========================================================================*/
/*!
    @brief
    Removes a CTL service message header from a message payload.

    @return
    0 if function is successful, negative value if not.
*/
/*=========================================================================*/
static int qmi_proxy_read_ctl_srvc_msg_hdr
(
  unsigned char                   **msg,
  int                             *msg_len,
  unsigned long                   *rx_msg_id,
  unsigned long                   *rx_length,
  unsigned long                   *rx_txn_id,
  qmi_proxy_ctl_service_msg_type  *rx_msg_type
)
{
  unsigned char *tmp_msg;
  unsigned char tmp_msg_type;

  /*-----------------------------------------------------------------------*/

  /* Move up the msg_buf pointer by the size of a QMI_SRVC_STD_TXN_HDR_SIZE */
  /* Make sure buffer size is correct */
  if ( *msg_len < ( QMI_PROXY_CTL_TXN_HDR_SIZE + QMI_SRVC_STD_MSG_HDR_SIZE ) )
  {
    QMI_PROXY_ERR_MSG( "Fail to read CTL service header, invalid msg len %d\n", *msg_len );
    return QMI_INTERNAL_ERR;
  }

    /* Move up the msg_buf pointer by the size of a QMI_SRVC_STD_TXN_HDR_SIZE */
  tmp_msg = *msg;
  *msg += ( QMI_PROXY_CTL_TXN_HDR_SIZE + QMI_SRVC_STD_MSG_HDR_SIZE );
  *msg_len -= ( QMI_PROXY_CTL_TXN_HDR_SIZE + QMI_SRVC_STD_MSG_HDR_SIZE );

  /* Read the message type, and translate into the
  ** generic message type enum
  */
  READ_8_BIT_VAL( tmp_msg, tmp_msg_type );

  if ( tmp_msg_type == QMI_PROXY_CTL_REQUEST_CONTROL_FLAG )
  {
    *rx_msg_type = QMI_PROXY_CTL_SERVICE_REQUEST_MSG;
  }
  else if ( tmp_msg_type == QMI_PROXY_CTL_RESPONSE_CONTROL_FLAG )
  {
    *rx_msg_type = QMI_PROXY_CTL_SERVICE_RESPONSE_MSG;
  }
  else if ( tmp_msg_type == QMI_PROXY_CTL_INDICATION_CONTROL_FLAG )
  {
    *rx_msg_type = QMI_PROXY_CTL_SERVICE_INDICATION_MSG;
  }
  else
  {
    QMI_PROXY_ERR_MSG( "Fail to read CTL service header, unknown msg type %d\n", tmp_msg_type );
    return QMI_INTERNAL_ERR;
  }

  /* Read 8 bit transaction ID */
  READ_8_BIT_VAL( tmp_msg, *rx_txn_id );

  /* Get the message ID field (16 bits) */
  READ_16_BIT_VAL( tmp_msg, *rx_msg_id );

  /* Get the length field (16 bits) */
  READ_16_BIT_VAL( tmp_msg, *rx_length );

  return QMI_NO_ERR;

} /* qmi_proxy_read_ctl_srvc_msg_hdr */


/*===========================================================================
  FUNCTION  qmi_proxy_read_std_txn_hdr
===========================================================================*/
/*!
    @brief
    Removes a "standard" QMI service header from a message payload.

    @return
    0 if function is successful, negative value if not.
*/
/*=========================================================================*/
static int qmi_proxy_read_std_txn_hdr
(
  unsigned char        **msg_buf,
  int                  *msg_buf_size,
  unsigned long        *rx_txn_id,
  qmi_service_msg_type *rx_msg_type
)
{
  unsigned char *tmp_msg_buf;
  unsigned char  tmp_msg_type;

  /*-----------------------------------------------------------------------*/

  /* Sanity check */
  if ( ( msg_buf == NULL ) || ( *msg_buf == NULL ) || ( msg_buf_size == NULL ) || ( rx_txn_id == NULL ) ||
       ( rx_msg_type == NULL ) )
  {
    QMI_PROXY_DEBUG_MSG( "%s\n", "Fail to read std txn hdr, invalid parameter" );

    return QMI_INTERNAL_ERR;
  }

  /* Make sure buffer size is correct */
  if ( *msg_buf_size < QMI_SRVC_STD_TXN_HDR_SIZE )
  {
    QMI_PROXY_DEBUG_MSG( "%s\n", "Message buffer size < QMI_SRVC_STD_TXN_HDR_SIZE" );

    return QMI_INTERNAL_ERR;
  }

  /* Move up the msg_buf pointer by the size of a QMI_SRVC_STD_TXN_HDR_SIZE */
  tmp_msg_buf = *msg_buf;

  /* Read the message type and translate into the generic message
  ** type enum
  */
  READ_8_BIT_VAL( tmp_msg_buf, tmp_msg_type );

  if ( tmp_msg_type == QMI_PROXY_STD_SRVC_REQUEST_CONTROL_FLAG )
  {
    *rx_msg_type = QMI_SERVICE_REQUEST_MSG;
  }
  else if ( tmp_msg_type == QMI_PROXY_STD_SRVC_RESPONSE_CONTROL_FLAG )
  {
    *rx_msg_type = QMI_SERVICE_RESPONSE_MSG;
  }
  else if ( tmp_msg_type == QMI_PROXY_STD_SRVC_INDICATION_CONTROL_FLAG )
  {
    *rx_msg_type = QMI_SERVICE_INDICATION_MSG;
  }
  else
  {
    return QMI_INTERNAL_ERR;
  }

  /* Read 16-bit transaction ID */
  READ_16_BIT_VAL( tmp_msg_buf, *rx_txn_id );

  /* Set output pointer and size values */
  *msg_buf += QMI_SRVC_STD_TXN_HDR_SIZE;
  *msg_buf_size -= QMI_SRVC_STD_TXN_HDR_SIZE;

  return QMI_NO_ERR;

} /* qmi_proxy_read_std_txn_hdr */


/*===========================================================================
  FUNCTION  qmi_proxy_read_std_srvc_msg_hdr
===========================================================================*/
/*!
    @brief
    Decodes a QMI message header (ID & length fields).

    @return
    0 if succees, negative value if error occurs
*/
/*=========================================================================*/
static int qmi_proxy_read_std_srvc_msg_hdr
(
  unsigned char **msg_buf,
  int           *msg_buf_size,
  unsigned long *msg_id,
  unsigned long *length
)
{
  unsigned char *tmp_msg_buf;

  /*-----------------------------------------------------------------------*/

  /* Sanity check */
  if ( ( msg_buf == NULL ) || ( *msg_buf == NULL ) || ( msg_buf_size == NULL ) || ( msg_id == NULL ) || ( length == NULL ) )
  {
    QMI_PROXY_DEBUG_MSG( "%s\n", "Fail to read std srvc msg hdr, invalid parameter" );

    return QMI_INTERNAL_ERR;
  }

  /* Make sure buffer size is correct */
  if ( *msg_buf_size < QMI_SRVC_STD_MSG_HDR_SIZE )
  {
    QMI_PROXY_DEBUG_MSG( "%s\n", "Message buffer size < QMI_SRVC_STD_MSG_HDR_SIZE" );

    return QMI_INTERNAL_ERR;
  }

  /* Move up the msg_buf pointer by the size of a QMI_SRVC_STD_TXN_HDR_SIZE */
  tmp_msg_buf = *msg_buf;
  *msg_buf += QMI_SRVC_STD_MSG_HDR_SIZE;
  *msg_buf_size -= QMI_SRVC_STD_MSG_HDR_SIZE;

  /* Get the message ID field (16 bits) */
  READ_16_BIT_VAL( tmp_msg_buf, *msg_id );

  /* Get the length field (16 bits) */
  READ_16_BIT_VAL( tmp_msg_buf, *length );

  return QMI_NO_ERR;

} /* qmi_proxy_read_std_srvc_msg_hdr */


/*===========================================================================
  FUNCTION  qmi_proxy_write_ctl_srvc_txn_hdr
===========================================================================*/
/*!
    @brief
    Put a CTL service message header from a message payload.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_write_ctl_srvc_msg_hdr
(
  unsigned char **msg,
  int           *msg_len,
  unsigned long msg_id,
  unsigned char txn_id
)
{
  unsigned char *tmp_msg;
  int           tmp_msg_len;

  /*-----------------------------------------------------------------------*/

  tmp_msg_len = *msg_len;

  /* Back up the msg_buf pointer by the size of a QMI_SRVC_STD_TXN_HDR_SIZE */
  *msg -= ( QMI_PROXY_CTL_TXN_HDR_SIZE + QMI_SRVC_STD_MSG_HDR_SIZE );
  *msg_len += ( QMI_PROXY_CTL_TXN_HDR_SIZE + QMI_SRVC_STD_MSG_HDR_SIZE );

  tmp_msg = *msg;

  /* CTL service header consists of a 1 byte control flags
  ** field of message type (request/response/indication),
  ** followed by a 1 byte transaction
  ** ID
  */
  WRITE_8_BIT_VAL( tmp_msg, QMI_PROXY_CTL_RESPONSE_CONTROL_FLAG );
  WRITE_8_BIT_VAL( tmp_msg, txn_id );

  /* Write the message ID field (16 bits) */
  WRITE_16_BIT_VAL( tmp_msg, msg_id );

  /* Write the length field */
  WRITE_16_BIT_VAL( tmp_msg, tmp_msg_len );

} /* qmi_proxy_write_ctl_srvc_msg_hdr */


/*===========================================================================
  FUNCTION  qmi_proxy_write_std_txn_hdr
===========================================================================*/
/*!
    @brief
    Put TXN ID header on message.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_write_std_txn_hdr
(
  unsigned char **msg_buf,
  int           *msg_buf_size,
  unsigned long txn_id,
  unsigned char msg_type
)
{
  unsigned char *tmp_msg_buf;

  /*-----------------------------------------------------------------------*/

  /* Sanity check */
  if ( ( msg_buf == NULL ) || ( *msg_buf == NULL ) || ( msg_buf_size == NULL ) )
  {
    QMI_PROXY_DEBUG_MSG( "%s\n", "Fail to write std txn hdr, invalid parameter" );

    return;
  }

  /* Back up the msg_buf pointer by the size of a QMI_SRVC_STD_TXN_HDR_SIZE */
  *msg_buf -= QMI_SRVC_STD_TXN_HDR_SIZE;
  *msg_buf_size += QMI_SRVC_STD_TXN_HDR_SIZE;

  tmp_msg_buf = *msg_buf;
  /* Standard service header consists of a 1 byte control flags,
  ** field of message type (request/response/indication),
  ** followed by a 2-byte transaction
  ** ID
  */
  WRITE_8_BIT_VAL( tmp_msg_buf, msg_type );
  WRITE_16_BIT_VAL( tmp_msg_buf, txn_id );

} /* qmi_proxy_write_std_txn_hdr */


/*===========================================================================
  FUNCTION  qmi_proxy_write_std_srvc_msg_hdr
===========================================================================*/
/*!
    @brief
    Put on message ID and length header.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_write_std_srvc_msg_hdr
(
  unsigned char **msg_buf,
  int           *msg_buf_size,
  unsigned long msg_id,
  int           length
)
{
  unsigned char *tmp_msg_buf;

  /*-----------------------------------------------------------------------*/

  /* Sanity check */
  if ( ( msg_buf == NULL ) || ( *msg_buf == NULL ) || ( msg_buf_size == NULL ) )
  {
    QMI_PROXY_DEBUG_MSG( "%s\n", "Fail to write std srvc msg hdr, invalid parameter" );

    return;
  }

  /* Back pointer up by 4 bytes */
  *msg_buf -= QMI_SRVC_STD_MSG_HDR_SIZE;
  *msg_buf_size += QMI_SRVC_STD_MSG_HDR_SIZE;

  tmp_msg_buf = *msg_buf;

  /* Write the message ID field (16 bits) */
  WRITE_16_BIT_VAL( tmp_msg_buf, msg_id );
  /* Write the length field */
  WRITE_16_BIT_VAL( tmp_msg_buf, length );

} /* qmi_proxy_write_std_srvc_msg_hdr */


/*=========================================================================
  FUNCTION:  qmi_proxy_read_modem_cfg

===========================================================================*/
/*!
    @brief
    Read QMI Proxy Modem Configuration.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_read_modem_cfg
(
  void
)
{
  char *prop_name = "ro.baseband";
  char cfg_val[ PROPERTY_VALUE_MAX ];
  uint8 i;

  /*-----------------------------------------------------------------------*/

  /* Read QMI Proxy Configuration setting from property */
  property_get( prop_name, cfg_val, "" );
  for ( i = 0; i < QMI_PROXY_MAX_CFGS; i++ )
  {
    if ( strcasecmp( cfg_val, qmi_proxy_cfg_name[ i ] ) == 0 )
    {
      qmi_proxy_cfg = i;
      break;
    }
  }

  if ( i > QMI_PROXY_MAX_CFGS )
  {
    QMI_PROXY_ERR_MSG( "Invalid qmi_proxy_cfg %s, use default\n", cfg_val );
  }

  QMI_PROXY_DEBUG_MSG( "qmi_proxy_cfg = %s(%d)\n", qmi_proxy_cfg_name[ qmi_proxy_cfg ], qmi_proxy_cfg );

} /* qmi_proxy_read_modem_cfg */


/*=========================================================================
  FUNCTION:  qmi_proxy_read_adb_log_cfg

===========================================================================*/
/*!
    @brief
    Read ADB Logging Configuration.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_read_adb_log_cfg
(
  void
)
{
  char *prop_name = "persist.radio.adb_log_on";
  char args[ PROPERTY_VALUE_MAX ];
  int len;
  char *end_ptr;
  unsigned long ret_val;
  uint8 i;

  /*-----------------------------------------------------------------------*/

  qmi_proxy_adb_log_on = FALSE;

  property_get( prop_name, args, "" );
  len = strlen( args );
  if ( len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
    {
      QMI_PROXY_ERR_MSG( "Fail to convert qmi_proxy_adb_on setting %s\n", args );
    }
    else if ( ret_val > 1 )
    {
      QMI_PROXY_ERR_MSG( "Invalid saved qmi_proxy_adb_on setting %ld, use default\n", ret_val );
    }
    else
    {
      qmi_proxy_adb_log_on = ( boolean ) ret_val;
    }
  }

  /* Save ADB Log Enabled setting to system property */
  QMI_PROXY_SNPRINTF( args, sizeof( args ), "%d", qmi_proxy_adb_log_on );
  if ( property_set( prop_name, args ) != 0 )
  {
    QMI_PROXY_ERR_MSG( "Fail to save %s to system property\n", prop_name );
  }

  QMI_PROXY_DEBUG_MSG( "qmi_proxy_adb_on = %d\n", qmi_proxy_adb_log_on );

} /* qmi_proxy_read_adb_log_cfg */

static unsigned long qmi_proxy_read_timer_value
(
  const char *prop_name,
  unsigned long defval
)
{
  char args[ PROPERTY_VALUE_MAX ];
  int len;
  char *end_ptr;
  unsigned long ret_val;
  uint8 i;

  property_get( prop_name, args, "" );
  len = strlen( args );
  if ( len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) &&
         ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) )
       )
    {
      QMI_PROXY_ERR_MSG( "Fail to convert %s setting %s," \
                         "use default\n", prop_name, args );
      ret_val = defval;
    }
    else if ( ret_val < 1 )
    {
      QMI_PROXY_ERR_MSG( "Invalid saved %s setting %ld," \
                         "use default\n", prop_name, ret_val );
      ret_val = defval;
    }
  }
  else
  {
     ret_val = defval;
  }

  return ret_val;
}
/*=========================================================================
FUNCTION: qmi_proxy_read_sglte_hystersis_timers

===========================================================================*/
/*!
@brief
Read SGLTE hystersis timer values from properties.

@return
None
*/
/*=========================================================================*/
static void qmi_proxy_read_sglte_hystersis_timers
(
void
)
{
  char *prop_name_1 = "persist.radio.hyst_ps_to_g";
  char *prop_name_2 = "persist.radio.hyst_ps_to_lt";
  char *prop_name_3 = "persist.radio.sglte_timer";
  char *prop_name_4 = "persist.radio.non_sglte_timer";
  int len;

  /*-----------------------------------------------------------------------*/

  memset(&qmi_proxy_sglte_hystersis_timer_info, 0, sizeof(qmi_proxy_sglte_hystersis_timer_info));
  QMI_PLATFORM_INIT_SIGNAL_DATA(&qmi_proxy_sglte_hystersis_timer_info.signal);

  qmi_proxy_sglte_hystersis_timer_info.hyst_ps_to_g_timer_value =
          qmi_proxy_read_timer_value(prop_name_1, DEFAULT_SGLTE_PS_TO_G_TIMER);
  QMI_PROXY_DEBUG_MSG( "hyst_ps_to_g = %d\n",
    qmi_proxy_sglte_hystersis_timer_info.hyst_ps_to_g_timer_value );

  qmi_proxy_sglte_hystersis_timer_info.hyst_ps_to_lt_timer_value =
          qmi_proxy_read_timer_value(prop_name_2, DEFAULT_SGLTE_PS_TO_LT_TIMER);
  QMI_PROXY_DEBUG_MSG( "hyst_ps_to_lt = %d\n",
          qmi_proxy_sglte_hystersis_timer_info.hyst_ps_to_lt_timer_value );

  qmi_proxy_sglte_hystersis_timer_info.sglte_rat_timer_value =
          qmi_proxy_read_timer_value(prop_name_3, DEFAULT_SGLTE_RAT_TIMER);
  QMI_PROXY_DEBUG_MSG( "sglte_rat_timer = %d\n",
          qmi_proxy_sglte_hystersis_timer_info.sglte_rat_timer_value );

  qmi_proxy_sglte_hystersis_timer_info.non_sglte_rat_timer_value =
          qmi_proxy_read_timer_value(prop_name_4, DEFAULT_NON_SGLTE_RAT_TIMER);
  QMI_PROXY_DEBUG_MSG( "non_sglte_rat_timer = %d\n",
          qmi_proxy_sglte_hystersis_timer_info.non_sglte_rat_timer_value );

} /* qmi_proxy_read_sglte_hystersis_timers */


/*=========================================================================
  FUNCTION:  qmi_proxy_read_voip_cfg

===========================================================================*/
/*!
    @brief
    Read Voip Configuration.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_read_voip_cfg
(
  void
)
{
  char *prop_name = "persist.radio.voip_enabled";
  char args[ PROPERTY_VALUE_MAX ];
  int len;
  char *end_ptr;
  unsigned long ret_val;
  uint8 i;

  /*-----------------------------------------------------------------------*/

  property_get( prop_name, args, "" );
  len = strlen( args );
  if ( len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
    {
      QMI_PROXY_ERR_MSG( "Fail to convert voip setting %s\n", args );
    }
    else if ( ret_val > 1 )
    {
      QMI_PROXY_ERR_MSG( "Invalid saved voip setting %ld, use default\n", ret_val );
    }
    else
    {
       qmi_proxy_internal_info.voip_info.is_voip_enabled= (boolean) ret_val;
    }
  }

  /* Save Voip setting to system property */
  QMI_PROXY_SNPRINTF( args, sizeof( args ), "%d", qmi_proxy_internal_info.voip_info.is_voip_enabled );
  if ( property_set( prop_name, args ) != 0 )
  {
    QMI_PROXY_ERR_MSG( "Fail to save %s to system property\n", prop_name );
  }

  QMI_PROXY_DEBUG_MSG( "is_voip_enabled = %d\n", qmi_proxy_internal_info.voip_info.is_voip_enabled );

} /* qmi_proxy_read_voip_cfg */


/*=========================================================================
  FUNCTION:  qmi_proxy_save_svlte_cfg

===========================================================================*/
/*!
    @brief
    Save SVLTE Configuration.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_save_svlte_cfg
(
  void
)
{
  char *prop_name = "persist.radio.svlte_cfg";
  char cfg_val[ PROPERTY_VALUE_MAX ];

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_SNPRINTF( cfg_val, sizeof( cfg_val ), "%d", qmi_proxy_internal_info.svlte_mode_pref );
  if ( property_set( prop_name, cfg_val ) == 0 )
  {
    QMI_PROXY_DEBUG_MSG( "QPI: saved SVLTE cfg = %s(%d)\n",
                         qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.svlte_mode_pref ],
                         qmi_proxy_internal_info.svlte_mode_pref );
  }
  else
  {
    QMI_PROXY_ERR_MSG( "QPI: failed to save SVLTE mode pref %s(%d) to property %s\n",
                       qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.svlte_mode_pref ],
                       qmi_proxy_internal_info.svlte_mode_pref,
                       prop_name );
  }

} /* qmi_proxy_save_svlte_cfg */


/*=========================================================================
  FUNCTION:  qmi_proxy_read_svlte_cfg

===========================================================================*/
/*!
    @brief
    Read SVLTE Configuration.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_read_svlte_cfg
(
  void
)
{
  char *prop_name = "persist.radio.svlte_cfg";
  char cfg_val[ PROPERTY_VALUE_MAX ];
  unsigned long ret_val;
  char *end_ptr;
  uint8 i;

  /*-----------------------------------------------------------------------*/

  /* Default SVLTE mode preference */
  if ( QMI_PROXY_SVLTE_1_TARGET() )
  {
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO;
  }
  else
  {
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO_GSM_WCDMA;
  }

  /* Read QMI Proxy's last saved SVLTE Mode Preference */
  property_get( prop_name, cfg_val, "" );
  if ( strlen( cfg_val ) > 0 )
  {
    ret_val = strtoul( cfg_val, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
    {
      QMI_PROXY_ERR_MSG( "QPI: fail to convert saved SVLTE mode preference %s, use default\n", cfg_val );
    }
    else if ( ret_val > QMI_PROXY_MODE_PREF_LTE_ONLY )
    {
      QMI_PROXY_ERR_MSG( "QPI: invalid saved SVLTE mode preference %ld, use default\n", ret_val );
    }
    else
    {
      qmi_proxy_internal_info.svlte_mode_pref = ret_val;
    }
  }

  /* Save SVLTE Configuration */
  qmi_proxy_save_svlte_cfg();

  QMI_PROXY_DEBUG_MSG( "QPI: SVLTE cfg= %s(%d)\n",
                       qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.svlte_mode_pref ],
                       qmi_proxy_internal_info.svlte_mode_pref );

} /* qmi_proxy_read_svlte_cfg */


/*=========================================================================
  FUNCTION:  qmi_proxy_save_sglte_user_mode_pref

===========================================================================*/
/*!
    @brief
    Save SGLTE User Mode Pref.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_save_sglte_user_mode_pref
(
  void
)
{
  char *prop_name = QMI_PROXY_SGLTE_USER_MODE_PREF_PROPERTY;
  char cfg_val[ PROPERTY_VALUE_MAX ];

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_SNPRINTF( cfg_val, sizeof( cfg_val ), "%d", qmi_proxy_internal_info.sglte_user_mode_pref );
  if ( property_set( prop_name, cfg_val ) == 0 )
  {
    QMI_PROXY_DEBUG_MSG( "Saved SGLTE cfg = %s(%d)\n",
                         qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.sglte_user_mode_pref ],
                         qmi_proxy_internal_info.sglte_user_mode_pref );
  }
  else
  {
    QMI_PROXY_ERR_MSG( "Failed to save SGLTE mode pref %s(%d) to property %s\n",
                       qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.sglte_user_mode_pref ],
                       qmi_proxy_internal_info.sglte_user_mode_pref,
                       prop_name );
  }

} /* qmi_proxy_save_sglte_cfg */


/*=========================================================================
  FUNCTION:  qmi_proxy_read_sglte_user_mode_pref

===========================================================================*/
/*!
    @brief
    Read SGLTE User Mode Pref.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_read_sglte_user_mode_pref
(
  void
)
{
  char *prop_name = QMI_PROXY_SGLTE_USER_MODE_PREF_PROPERTY;
  char cfg_val[ PROPERTY_VALUE_MAX ];
  unsigned long ret_val;
  char *end_ptr;
  uint8 i;

  /*-----------------------------------------------------------------------*/

  /* Default SGLTE mode preference */
  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
  qmi_proxy_internal_info.sglte_user_mode_pref = QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA;
  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Read QMI Proxy's last saved SGLTE Mode Preference */
  property_get( prop_name, cfg_val, "" );
  if ( strlen( cfg_val ) > 0 )
  {
    ret_val = strtoul( cfg_val, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
    {
      QMI_PROXY_ERR_MSG( "QPI: fail to convert saved SGLTE mode preference %s, use default\n", cfg_val );
    }
    else if ( ret_val >= QMI_PROXY_MAX_MODE_PREFS )
    {
      QMI_PROXY_ERR_MSG( "QPI: invalid saved SGLTE mode preference %ld, use default\n", ret_val );
    }
    else
    {
      QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
      qmi_proxy_internal_info.sglte_user_mode_pref = ret_val;
      QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
    }
  }

  /* Save SGLTE Configuration */
  qmi_proxy_save_sglte_user_mode_pref();

  QMI_PROXY_DEBUG_MSG( "QPI: SGLTE user mode pref = %s(%d)\n",
                       qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.sglte_user_mode_pref ],
                       qmi_proxy_internal_info.sglte_user_mode_pref );

} /* qmi_proxy_read_sglte_user_mode_pref */

/*=========================================================================
  FUNCTION:  qmi_proxy_save_voice_modem_index

===========================================================================*/
/*!
    @brief
    Save voice modem index, indicates on which modem voice calls are placed.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_save_voice_modem_index
(
  qmi_proxy_conn_type voice_modem_index
)
{
  char *prop_name = "persist.radio.voice.modem.index";
  char cfg_val[ PROPERTY_VALUE_MAX ];

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_SNPRINTF( cfg_val, sizeof( cfg_val ), "%d", voice_modem_index );
  if ( property_set( prop_name, cfg_val ) == 0 )
  {
    QMI_PROXY_DEBUG_MSG( "Saved voice modem index = %d\n", voice_modem_index );
  }
  else
  {
    QMI_PROXY_ERR_MSG( "Failed to save voice modem index %d to property %s\n",
                       voice_modem_index,
                       prop_name );
  }

} /* qmi_proxy_save_voice_modem_index */


/*=========================================================================
  FUNCTION:  qmi_proxy_read_sglte_test_mode

===========================================================================*/
/*!
    @brief
    Read the SGLTE test mode configuration from property.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_read_sglte_test_mode
(
  void
)
{
  char *prop_name = QMI_PROXY_SGLTE_TEST_MODE_PROPERTY;
  char test_mode[ PROPERTY_VALUE_MAX ];

  /* Check if SGLTE test mode is enabled or not. */
  property_get( prop_name, test_mode, "" );
  if ((strlen(test_mode) > 0) && (atoi(test_mode) == 1))
  {
    qmi_proxy_sglte_is_test_mode = TRUE;
  }

} /* qmi_proxy_read_sglte_test_mode */


/*=========================================================================
  FUNCTION:  qmi_proxy_read_sglte_plmn_list

===========================================================================*/
/*!
    @brief
    Read the SGLTE PLMNs list from property.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_read_sglte_plmn_list
(
  qmi_proxy_plmn_id_s_type * plmn
)
{
  char *prop_name = QMI_PROXY_SGLTE_PLMN_LIST_PROPERTY;
  char plmn_list_buffer [ PROPERTY_VALUE_MAX ];
  char *plmn_list = plmn_list_buffer;
  int i;

  /*-----------------------------------------------------------------------*/

  char *end_ptr;
  char *mnc_ptr;
  uint8 mcc_length;
  uint8 mnc_length;

  /*-----------------------------------------------------------------------*/

  (void) plmn;

  /* Read configured PLMN list. */
  property_get( prop_name, plmn_list, "" );
  /* Reset sglte_plmn_list */
  memset(sglte_plmn_list, 0, sizeof(sglte_plmn_list));


  i = 0;

  if ( '+' == plmn_list[0]) {
    /* A '+' at the start of the property value means to append to the default list,
     * rather than replace
     */
    qmi_proxy_use_default_sglte_plmn_list();
    plmn_list ++;
    i = DEFAULT_SGLTE_PLMNS;
  }

  for (; i < QMI_PROXY_MAX_SGLTE_PLMN_ENTRIES && strlen( plmn_list ) > 0; i++)
  {

    /* Parse the PLMN list & store it */
    mnc_ptr = strchr(plmn_list, QMI_PROXY_PLMN_LIST_MCC_BOUNDARY);
    end_ptr = strchr(plmn_list, QMI_PROXY_PLMN_LIST_ITEM_BOUNDARY);

    if (!end_ptr)
    {
      end_ptr = plmn_list + strlen(plmn_list);
    }

    if ( !(mnc_ptr))
    {
        QMI_PROXY_ERR_MSG( "QPI: Improper format SGLTE PLMN list %s, use default\n", plmn_list );
        break;
    }

    mnc_ptr++;
    mcc_length = mnc_ptr - plmn_list - 1;      /* Adjust length for , after MCC */
    mnc_length = end_ptr - mnc_ptr;

    if ( mcc_length != QMI_PROXY_MCC_LENGTH - 1 /* NULL terminator */ ||
         mnc_length <  QMI_PROXY_MNC_LENGTH - 2 /* 2-digit MNC */ ||
         mnc_length >  QMI_PROXY_MNC_LENGTH - 1)
    {
      QMI_PROXY_ERR_MSG( "QPI: Invalid SGLTE PLMN list %s, mcc_len %d, mnc_len %d\n",
                         plmn_list, mcc_length, mnc_length );
      break;
    }
    else
    {
      QMI_PROXY_DEBUG_MSG( "SGLTE PLMN list = %s, mcc_len %d, mnc_len %d\n",
                           plmn_list, mcc_length, mnc_length );

      memcpy(sglte_plmn_list[i].mcc,
             plmn_list,
             mcc_length) ;

      memcpy(sglte_plmn_list[i].mnc,
             mnc_ptr,
             mnc_length);

      if ( mnc_length == 2 )
      {
        /* For 2 digit MNC, set third digit to be 0xFF */
        sglte_plmn_list[i].mnc[2] = 0xFF;
      }
    }

    if ( QMI_PROXY_PLMN_LIST_ITEM_BOUNDARY == *end_ptr )
    {
      end_ptr++;
    }

    plmn_list = end_ptr;
  }

  if (i == 0)
  {
    QMI_PROXY_ERR_MSG( "QPI: fail to read SGLTE PLMN list %s, use default\n", plmn_list );
    qmi_proxy_use_default_sglte_plmn_list();
  }

} /* qmi_proxy_read_sglte_plmn_list */

/*=========================================================================
  FUNCTION:  qmi_proxy_save_sglte_plmn_list

===========================================================================*/
/*!
    @brief
    Save SGLTE PLMNs to property.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_save_sglte_plmn_list
(
  const qmi_proxy_plmn_id_s_type * const plmn
)
{
  char *prop_name = QMI_PROXY_SGLTE_PLMN_LIST_PROPERTY;
  char cfg_val[ PROPERTY_VALUE_MAX ];

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_SNPRINTF( cfg_val, sizeof( cfg_val ), "%s,%s",
                      plmn->mcc, plmn->mnc  );
  if ( property_set( prop_name, cfg_val ) == 0 )
  {
    QMI_PROXY_DEBUG_MSG( "Saved SGLTE PLMN list = %s\n", cfg_val );
  }
  else
  {
    QMI_PROXY_ERR_MSG( "Failed to save SGLTE PLMN list %s to property %s\n",
                       cfg_val,
                       prop_name );
  }

} /* qmi_proxy_save_sglte_plmn_list */


/*=========================================================================
  FUNCTION:  qmi_proxy_read_memory_leak_debug_prop

===========================================================================*/
/*!
    @brief
    read QMI Proxy memory leak debug property.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_read_memory_leak_debug_prop
(
  void
)
{
  char *prop_name = "persist.radio.mem_leak_debug";
  char args[ PROPERTY_VALUE_MAX ];
  int len;
  char *end_ptr;
  unsigned long ret_val;
  uint8 i;

  /*-----------------------------------------------------------------------*/

  property_get( prop_name, args, "" );
  len = strlen( args );
  if ( len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
    {
      QMI_PROXY_ERR_MSG( "Fail to convert memory leak setting %s\n", args );
    }
    else if ( ret_val > 1 )
    {
      QMI_PROXY_ERR_MSG( "Invalid saved memory leak setting %ld, use default\n", ret_val );
    }
    else
    {
       qmi_proxy_mem_leak_debug_on = (boolean) ret_val;
    }
  }

  /* Save mem leak setting to system property */
  QMI_PROXY_SNPRINTF( args, sizeof( args ), "%d", qmi_proxy_mem_leak_debug_on );
  if ( property_set( prop_name, args ) != 0 )
  {
    QMI_PROXY_ERR_MSG( "Fail to save %s to system property\n", prop_name );
  }

  QMI_PROXY_DEBUG_MSG( "mem leak debug enabled = %d\n", qmi_proxy_mem_leak_debug_on);

} /* qmi_proxy_read_memory_leak_debug_prop */

/*=========================================================================
  FUNCTION:  qmi_proxy_get_registration_status

===========================================================================*/
/*!
    @brief
    Get the ps and cs registration status as well as
    cs and ps roaming status

    @return
    none
*/
/*=========================================================================*/
void qmi_proxy_get_registration_status(
  boolean *is_cs_registered,
  boolean *is_cs_roaming,
  boolean *is_ps_registered,
  boolean *is_ps_roaming
)
{
  if (!is_cs_registered || !is_cs_roaming || !is_ps_registered || !is_ps_roaming)
  {
    QMI_PROXY_DEBUG_MSG("%s: invalid arguments", __FUNCTION__);
    return;
  }

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
  *is_cs_registered = qmi_proxy_internal_info.is_cs_registered;
  *is_cs_roaming    = qmi_proxy_internal_info.is_cs_roaming;
  *is_ps_registered = qmi_proxy_internal_info.is_ps_registered;
  *is_ps_roaming    = qmi_proxy_internal_info.is_ps_roaming;
  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
} /* qmi_proxy_get_registration_status */

/*===========================================================================
  FUNCTION  qmi_proxy_sglte_set_registration_roaming_status
===========================================================================*/
/*!
@brief
  Helper function to evaluate the registration and roaming status

@return
  none
*/
/*=========================================================================*/
void qmi_proxy_sglte_set_registration_roaming_status
(
  uint8_t srv_status_info_valid,
  nas_service_status_enum_type_v01 srv_status,
  nas_common_sys_info_type_v01 *common_sys_info,
  boolean *is_cs_registered,
  boolean *is_ps_registered,
  boolean *is_cs_roaming,
  boolean *is_ps_roaming
)
{

  if (!is_cs_registered || !is_ps_registered || !is_cs_roaming ||
        !is_ps_roaming)
  {
    return;
  }

  if (srv_status_info_valid &&
       (srv_status == NAS_SYS_SRV_STATUS_SRV_V01))
  {
    *is_ps_registered = TRUE;
    *is_cs_registered = TRUE;

    if  (common_sys_info && !(common_sys_info->roam_status_valid &&
      (NAS_SYS_ROAM_STATUS_OFF_V01 == common_sys_info->roam_status
      || (nas_roam_status_enum_type_v01)NAS_VAL_ROAMING_HOME_EX ==
            common_sys_info->roam_status )))
    {
      *is_ps_roaming = TRUE;
      *is_cs_roaming = TRUE;
    }
  }

  return;
}

/*===========================================================================
  FUNCTION qmi_proxy_sglte_update_registration_status
===========================================================================*/
/*!
@brief
  Helper function to evaluate if local modem has full service when
  in SGLTE configuration when in a SGLTE PLMN.

@return
   TRUE if local modem reported full service
   FALSE otherwise
*/
/*=========================================================================*/
void  qmi_proxy_sglte_update_registration_status
(
    qmi_proxy_conn_type cs_active_modem,
    qmi_proxy_conn_type ps_active_modem,
    qmi_proxy_conn_type proxy_conn_type,
    nas_sys_info_ind_msg_v01 * nas_sys_info_ind
)
{

  uint8_t lte_srv_status_info_valid;
  nas_service_status_enum_type_v01 lte_srv_status;
  uint8_t tdscdma_srv_status_info_valid;
  nas_service_status_enum_type_v01 tdscdma_srv_status;
  uint8_t gsm_srv_status_info_valid;
  nas_service_status_enum_type_v01 gsm_srv_status;
  boolean is_ps_registered = FALSE;
  boolean is_ps_roaming    = FALSE;
  boolean is_cs_registered = FALSE;
  boolean is_cs_roaming    = FALSE;

  if ((cs_active_modem != proxy_conn_type) && (ps_active_modem != proxy_conn_type))
  {
    return;
  }

  if (QMI_PROXY_LOCAL_CONN_TYPE == proxy_conn_type)
  {
    lte_srv_status_info_valid = nas_sys_info_ind->lte_srv_status_info_valid;
    lte_srv_status = nas_sys_info_ind->lte_srv_status_info.srv_status;

    qmi_proxy_sglte_set_registration_roaming_status( lte_srv_status_info_valid, lte_srv_status,
                                                &(nas_sys_info_ind->lte_sys_info.common_sys_info),
                                                &is_cs_registered,
                                                &is_ps_registered,
                                                &is_cs_roaming,
                                                &is_ps_roaming);

    tdscdma_srv_status_info_valid = nas_sys_info_ind->tdscdma_srv_status_info_valid;
    tdscdma_srv_status = nas_sys_info_ind->tdscdma_srv_status_info.srv_status;

    qmi_proxy_sglte_set_registration_roaming_status( tdscdma_srv_status_info_valid, tdscdma_srv_status,
                                                &(nas_sys_info_ind->tdscdma_sys_info.common_sys_info),
                                                &is_cs_registered,
                                                &is_ps_registered,
                                                &is_cs_roaming,
                                                &is_ps_roaming);
  }
  else
  {
    gsm_srv_status_info_valid = nas_sys_info_ind->gsm_srv_status_info_valid;
    gsm_srv_status = nas_sys_info_ind->gsm_srv_status_info.srv_status;

    qmi_proxy_sglte_set_registration_roaming_status( gsm_srv_status_info_valid, gsm_srv_status,
                                                &(nas_sys_info_ind->gsm_sys_info.common_sys_info),
                                                &is_cs_registered,
                                                &is_ps_registered,
                                                &is_cs_roaming,
                                                &is_ps_roaming);
  }

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
  if (proxy_conn_type == ps_active_modem)
  {
    qmi_proxy_internal_info.is_ps_registered = is_ps_registered;
    qmi_proxy_internal_info.is_ps_roaming = is_ps_roaming;
  }

  if (proxy_conn_type == cs_active_modem)
  {
    qmi_proxy_internal_info.is_cs_registered = is_cs_registered;
    qmi_proxy_internal_info.is_cs_roaming = is_cs_roaming;
  }
  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  QMI_PROXY_DEBUG_MSG( "%s ....... proxy connection type:%d, is cs registered:%d, "
                        "is ps registered:%d, is cs roaming:%d, is ps roaming:%d \n",
          __FUNCTION__, proxy_conn_type, qmi_proxy_internal_info.is_cs_registered,
          qmi_proxy_internal_info.is_ps_registered,
          qmi_proxy_internal_info.is_cs_roaming,
          qmi_proxy_internal_info.is_ps_roaming);

} /* qmi_proxy_sglte_update_registration_status */

/*=========================================================================
  FUNCTION:  qmi_proxy_is_cs_active_changed

===========================================================================*/
/*!
    @brief
    Check if cs active modem going to be changed

    @return
    True  - if cs active modem is going to be changed
    False - otherwise
*/
/*=========================================================================*/
static boolean qmi_proxy_is_cs_active_changed
(
  qmi_proxy_mode_pref_type  mode_pref
)
{
  QMI_PROXY_DEBUG_MSG( "%s ....... user_mode_pref: %d in sglte coverage: %d.\n",
          __FUNCTION__, mode_pref,
          qmi_proxy_internal_info.is_in_sglte_coverage);

  if(mode_pref == QMI_PROXY_MODE_PREF_GSM_ONLY &&
           qmi_proxy_sglte_is_test_mode)
  {
    if (qmi_proxy_internal_info.cs_active_modem != QMI_PROXY_REMOTE_CONN_TYPE)
    {
      return TRUE;
    }
  }
  else if( (mode_pref == QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA &&
            (qmi_proxy_sglte_is_test_mode ||
               qmi_proxy_internal_info.is_in_sglte_coverage)) )
  {
    if (qmi_proxy_internal_info.cs_active_modem != QMI_PROXY_REMOTE_CONN_TYPE)
    {
      return TRUE;
    }
  }
  else
  {
    if (qmi_proxy_internal_info.cs_active_modem != QMI_PROXY_LOCAL_CONN_TYPE)
    {
      return TRUE;
    }
  }

  return FALSE;
} /* qmi_proxy_is_cs_active_changed */

/*=========================================================================
  FUNCTION:  qmi_proxy_update_cs_active_ps_active_sglte

===========================================================================*/
/*!
    @brief
    Set cs_active_modem and ps_active_modem  properties according to sglte
    requirements

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_update_cs_active_ps_active_sglte(
  void
)
{
  QMI_PROXY_DEBUG_MSG( "%s ....... sglte_user_mode_pref: %d. in sglte coverage: %d\n",
          __FUNCTION__, qmi_proxy_internal_info.sglte_user_mode_pref,
          qmi_proxy_internal_info.is_in_sglte_coverage);

  if(qmi_proxy_internal_info.sglte_user_mode_pref == QMI_PROXY_MODE_PREF_GSM_ONLY &&
           qmi_proxy_sglte_is_test_mode)
  {
    qmi_proxy_save_voice_modem_index(QMI_PROXY_REMOTE_CONN_TYPE);
    qmi_proxy_internal_info.cs_active_modem = QMI_PROXY_REMOTE_CONN_TYPE;
    qmi_proxy_internal_info.ps_active_modem = QMI_PROXY_REMOTE_CONN_TYPE;
  }
  else if( (qmi_proxy_internal_info.sglte_user_mode_pref == QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA &&
            (qmi_proxy_sglte_is_test_mode ||
               qmi_proxy_internal_info.is_in_sglte_coverage) ))
  {
    /* Initialize QMI voice modem index */
    qmi_proxy_save_voice_modem_index( QMI_PROXY_REMOTE_CONN_TYPE );
    qmi_proxy_internal_info.cs_active_modem = QMI_PROXY_REMOTE_CONN_TYPE;
    qmi_proxy_internal_info.ps_active_modem = QMI_PROXY_LOCAL_CONN_TYPE;
  }
  else
  {
    qmi_proxy_save_voice_modem_index( QMI_PROXY_LOCAL_CONN_TYPE );
    qmi_proxy_internal_info.cs_active_modem = QMI_PROXY_LOCAL_CONN_TYPE;
    qmi_proxy_internal_info.ps_active_modem = QMI_PROXY_LOCAL_CONN_TYPE;
  }
} /* qmi_proxy_update_cs_active_ps_active_sglte */

/*=========================================================================
  FUNCTION:  qmi_proxy_init

===========================================================================*/
/*!
    @brief
    Initializes the QMI Proxy to be ready for servicing clients.

    @return
    None
*/
/*=========================================================================*/
void qmi_proxy_init
(
  void
)
{
  unsigned long ret_val;
  uint8 i;
  char prop_str[ PROPERTY_VALUE_MAX ];

  /*-----------------------------------------------------------------------*/

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_mutex );

  if ( !qmi_proxy_initialization_done )
  {
    QMI_PROXY_DEBUG_MSG( "qmi_proxy_init: starting, QMI Proxy config %s(%d)\n",
                          qmi_proxy_cfg_name[ qmi_proxy_cfg ],
                          qmi_proxy_cfg );

    qmi_proxy_initialization_done = TRUE;

    /* Initialize Diag for QCRIL logging */
    ret_val = Diag_LSM_Init(NULL);
    if ( !ret_val )
    {
      LOGE( "Fail to initialize Diag for QMI Proxy logging\n" );
    }

    /* Read QMI Proxy Target Configuration */
    qmi_proxy_read_modem_cfg();

    /* Read QMI Proxy ADB Logging Configuration */
    qmi_proxy_read_adb_log_cfg();

    /* Read QMI Proxy SGLTE test mode configuration */
    qmi_proxy_read_sglte_test_mode();

    /* Read QMI Proxy memory lead debug messages property */
    qmi_proxy_read_memory_leak_debug_prop();
    QMI_PLATFORM_MUTEX_INIT(&qmi_proxy_mem_counter_mutex );

    /* Initialize QMI Proxy Internal Info */
    memset( &qmi_proxy_internal_info, 0 , sizeof( qmi_proxy_internal_info ) );
    QMI_PLATFORM_MUTEX_INIT( &qmi_proxy_internal_info.init_mutex );
    QMI_PLATFORM_MUTEX_INIT( &qmi_proxy_internal_info.cache_mutex );

    QMI_PLATFORM_MUTEX_INIT( &qmi_proxy_internal_info.voip_info_mutex );
    /* Read QMI Proxy Voip Configuration */
    qmi_proxy_read_voip_cfg();

    QMI_PLATFORM_MUTEX_INIT( &qmi_proxy_internal_info.ssr_info_mutex );

    /* Initialize QMI Proxy Client ID */
    qmi_proxy_client_id = 1;
    QMI_PLATFORM_MUTEX_INIT( &qmi_proxy_client_id_mutex );

    /* Initialize QMI Proxy Client info structures */
    for ( i = 0; i < QMI_MAX_SERVICES; i++ )
    {
      qmi_proxy_client_info_tbl[ i ] = NULL;
      QMI_PLATFORM_MUTEX_INIT( &qmi_proxy_client_list_mutex_tbl[ i ] );
    }

    /* Initialize QMI Proxy Transaction ID */
    qmi_proxy_txn_id = 1;
    QMI_PLATFORM_MUTEX_INIT( &qmi_proxy_txn_id_mutex );

    /* Initialize QMI Proxy Transaction info structures */
    for ( i = 0; i < QMI_MAX_SERVICES; i++ )
    {
      qmi_proxy_txn_info_tbl[ i ] = NULL;
      QMI_PLATFORM_MUTEX_INIT( &qmi_proxy_txn_list_mutex_tbl[ i ] );
    }

    /* Initialize QMI Service Object table */
    memset( &qmi_proxy_srvc_obj_tbl, 0 , sizeof( qmi_proxy_srvc_obj_tbl ) );
    qmi_proxy_srvc_obj_tbl[ QMI_DMS_SERVICE ] = dms_get_service_object_v01();
    qmi_proxy_srvc_obj_tbl[ QMI_NAS_SERVICE ] = nas_get_service_object_v01();
    qmi_proxy_srvc_obj_tbl[ QMI_WMS_SERVICE ] = wms_get_service_object_v01();
    qmi_proxy_srvc_obj_tbl[ QMI_VOICE_SERVICE ] = voice_get_service_object_v02();
    qmi_proxy_srvc_obj_tbl[ QMI_PBM_SERVICE ] = pbm_get_service_object_v01();
    qmi_proxy_srvc_obj_tbl[ QMI_SAR_SERVICE ] = sar_get_service_object_v01();
    qmi_proxy_srvc_obj_tbl[ QMI_IMS_VT_SERVICE ] = ims_qmi_get_service_object_v01();
    qmi_proxy_srvc_obj_tbl[ QMI_IMS_PRESENCE_SERVICE ] = imsp_get_service_object_v01();

    qmi_proxy_internal_info.sglte_sm = qmi_proxy_sglte_sm_new();
    qmi_proxy_sglte_sm_start(qmi_proxy_internal_info.sglte_sm);
    /* Read SGLTE PLMN list */
    qmi_proxy_read_sglte_plmn_list(sglte_plmn_list);

    /* Read SGLTE user_mode_pref */
    qmi_proxy_read_sglte_user_mode_pref();

    if (QMI_PROXY_SGLTE_TARGET())
    {
      /* Read SGLTE hystersis timer configuration */
      qmi_proxy_read_sglte_hystersis_timers();

      /* Initialize voice modem index property and internal cs and ps active variable */
      qmi_proxy_sglte_invalidate_last_plmn(QMI_PROXY_LOCAL_CONN_TYPE);
      qmi_proxy_sglte_invalidate_last_plmn(QMI_PROXY_REMOTE_CONN_TYPE);
      qmi_proxy_sglte_update_in_sglte_coverage();
      qmi_proxy_update_cs_active_ps_active_sglte();
      qmi_proxy_internal_info.is_dtm_supported[QMI_PROXY_LOCAL_CONN_TYPE] = -1;
      qmi_proxy_internal_info.is_dtm_supported[QMI_PROXY_REMOTE_CONN_TYPE] = -1;
      qmi_proxy_internal_info.desired_mode = DMS_OP_MODE_LOW_POWER_V01;
    }

    qmi_proxy_internal_info.service_ready[QMI_PROXY_LOCAL_CONN_TYPE]  = WMS_SERVICE_READY_STATUS_ENUM_MIN_ENUM_VAL_V01;
    qmi_proxy_internal_info.service_ready[QMI_PROXY_REMOTE_CONN_TYPE]  = WMS_SERVICE_READY_STATUS_ENUM_MIN_ENUM_VAL_V01;


    QMI_PROXY_DEBUG_MSG( "qmi_proxy_init: qmi_proxy_internal_info.cs_active_modem=%d\n",
                         qmi_proxy_internal_info.cs_active_modem );
    QMI_PROXY_DEBUG_MSG( "qmi_proxy_init: qmi_proxy_internal_info.ps_active_modem=%d\n",
                         qmi_proxy_internal_info.ps_active_modem );

    QMI_PROXY_DEBUG_MSG( "qmi_proxy_init: init done, QMI Proxy config %s(%d)\n",
                         qmi_proxy_cfg_name[ qmi_proxy_cfg ],
                         qmi_proxy_cfg );
  }
  else
  {
    QMI_PROXY_DEBUG_MSG( "qmi_proxy_init: already init, QMI Proxy config %s(%d)\n",
                         qmi_proxy_cfg_name[ qmi_proxy_cfg ],
                         qmi_proxy_cfg );
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_mutex );

} /* qmi_proxy_init */


/*=========================================================================
  FUNCTION:  qmi_proxy_print_client_list

===========================================================================*/
/*!
    @brief
    Print the content of client list for a specified QMI Service.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_print_client_list
(
  qmi_service_id_type proxy_srvc_id
)
{
  qmi_proxy_client_info_type *client = NULL;
  uint8 i, count = 0;

  /* -------------------------------------------------------------------  */

  client = qmi_proxy_client_info_tbl[ proxy_srvc_id ];

  if ( client != NULL )
  {
    QMI_PROXY_DEBUG_MSG( "Client List for QMI %s(%d)\n",
                         qmi_proxy_srvc_name[ proxy_srvc_id ],
                         proxy_srvc_id );
  }

  while ( client != NULL )
  {
    QMI_PROXY_DEBUG_MSG( "   [%d] QMI %s(%d), Client ID %d, Conn ID %d, FD %d\n",
                         count++,
                         qmi_proxy_srvc_name[ client->proxy_srvc_id ],
                         client->proxy_srvc_id,
                         client->proxy_client_id,
                         client->proxy_conn_id,
                         client->fd );

    for ( i = 0; i < QMI_PROXY_MAX_CONN_TYPE; i++ )
    {
      if ( client->srvc_info[ i ].registered )
      {
        QMI_PROXY_DEBUG_MSG( "             ... %s(%d)\n",
                             qmi_proxy_modem_name[ i ],
                             i );
      }
    }

    client = client->next;
  }

} /* qmi_proxy_print_client_list */


/*=========================================================================
  FUNCTION:  qmi_proxy_alloc_client_data

===========================================================================*/
/*!
    @brief
    Allocate memory for a QMI Proxy Client data

    @return
    Pointer to QMI Proxy Client data
*/
/*=========================================================================*/
static qmi_proxy_client_info_type *qmi_proxy_alloc_client_data
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_service_id_type    proxy_srvc_id,
  int fd
)
{
  qmi_proxy_client_info_type *client = NULL;

  /* -------------------------------------------------------------------  */

  /* Dynamically allocate a client data structure */
  client = qmi_proxy_malloc( sizeof( qmi_proxy_client_info_type ) );

  if ( client == NULL )
  {
    return NULL;
  }

  /* Initialize structure to all 0's */
  memset( ( void * ) client, 0, sizeof( qmi_proxy_client_info_type ) );

  QMI_PLATFORM_MUTEX_INIT( &client->mutex );

  /* Fill in ID fields */
  client->proxy_client_id = proxy_client_id;
  client->proxy_conn_id = proxy_conn_id;
  client->proxy_srvc_id = proxy_srvc_id;
  client->fd = fd;

  /* Lock global client list access mutex */
  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_client_list_mutex_tbl[ proxy_srvc_id ] );

  /* Add new client data record to the list */
  client->next = qmi_proxy_client_info_tbl[ proxy_srvc_id ];
  qmi_proxy_client_info_tbl[ proxy_srvc_id ] = client;

  qmi_proxy_print_client_list( proxy_srvc_id );

  /* Unlock global client list access mutex */
  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_client_list_mutex_tbl[ proxy_srvc_id ] );

  QMI_PROXY_DEBUG_MSG( "Allocated client data ....... QMI %s(%d), Client ID %d\n",
                       qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id, proxy_client_id );

  /* return allocated client data pointer */
  return client;

} /* qmi_proxy_alloc_client_data */


/*===========================================================================
  FUNCTION  qmi_proxy_release_client_data
===========================================================================*/
/*!
    @brief
    Release all QMI service connections and free memory used by specified
    QMI Proxy client.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_release_client_data
(
  qmi_client_id_type  proxy_client_id,
  qmi_service_id_type proxy_srvc_id,
  boolean             lock_list_mutex,
  boolean             modem_restarted
)
{
  qmi_proxy_client_info_type *client = NULL, *prev = NULL;
  qmi_proxy_srvc_info_type *srvc_ptr = NULL;
  uint8 i;
  int rc = QMI_NO_ERR;

  /* -------------------------------------------------------------------  */
  if ( proxy_srvc_id >= QMI_MAX_SERVICES )
  {
    QMI_PROXY_ERR_MSG(" Invalid Service Id(%d) received, skip deleting the client data \n", proxy_srvc_id );
    return QMI_INTERNAL_ERR;
  }

  if ( lock_list_mutex )
  {
    /* Lock global client list access mutex */
    QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_client_list_mutex_tbl[ proxy_srvc_id ] );
  }

  /* Look up the QMI Proxy Client record */
  client = qmi_proxy_client_info_tbl[ proxy_srvc_id ];
  while ( client != NULL )
  {
    if ( client->proxy_client_id == proxy_client_id )
    {
      break;
    }

    prev = client;
    client = client->next;
  }

  /* Client not found */
  if ( client == NULL )
  {
     if ( lock_list_mutex )
     {
       /* Unlock global client access mutex */
       QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_client_list_mutex_tbl[ proxy_srvc_id ] );
     }

    QMI_PROXY_ERR_MSG( "Fail to delete client, Client Rec (ID %d) not found in the QMI %s(%d) Client List\n",
                       proxy_client_id,
                       qmi_proxy_srvc_name[ proxy_srvc_id ],
                       proxy_srvc_id );

    rc = QMI_INTERNAL_ERR;
  }
  /* If we find matching client, proceed.... */
  else
  {
    /* Lock client mutex */
    QMI_PLATFORM_MUTEX_LOCK ( &client->mutex );

    /* Set the ready for delete status flag */
    client->ready_to_delete = TRUE;

    /* If reference count is 0, delete client object */
    if ( client->ref_count > 0 )
    {
      if ( lock_list_mutex )
      {
        /* Unlock global client access mutex */
        QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_client_list_mutex_tbl[ proxy_srvc_id ] );
      }
    }
    else
    {
      /* Remove client data record from the client list */
      if ( client != NULL )
      {
        if ( prev == NULL )
        {
          qmi_proxy_client_info_tbl[ proxy_srvc_id ] = client->next;
        }
        else
        {
          prev->next = client->next;
        }

        client->next = NULL;
      }

      qmi_proxy_print_client_list( proxy_srvc_id );

      if ( lock_list_mutex )
      {
        /* Unlock global client access mutex */
        QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_client_list_mutex_tbl[ proxy_srvc_id ] );
      }

      /* Release the modem's QMI service connection */
      if ( !modem_restarted )
      {
        for ( i = 0; i < QMI_PROXY_MAX_CONN_TYPE; i++ )
        {
          srvc_ptr = &client->srvc_info[ i ];
          if ( srvc_ptr->registered )
          {
            rc = qmi_client_release( srvc_ptr->client_handle );
            srvc_ptr->registered = FALSE;
            if ( rc != QMI_NO_ERR )
            {
              QMI_PROXY_ERR_MSG( "Fail to release %s(%d) service connection for QMI %s(%d)\n",
                                 qmi_proxy_modem_name[ i ], i, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id );
            }
          }
        }
      }

      /* Unlock the client mutex */
      QMI_PLATFORM_MUTEX_UNLOCK( &client->mutex );

      /* Destroy the client mutex */
      QMI_PLATFORM_MUTEX_DESTROY( &client->mutex );

      /* Free the memory used by the client */
      qmi_proxy_free( (void **) &client );

      client = NULL;

      QMI_PROXY_DEBUG_MSG( "Released client data ....... QMI %s(%d), Client ID %d\n",
                           qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id, proxy_client_id );
    }
  }

  return rc;

} /* qmi_proxy_release_client_data */

/*=========================================================================
  FUNCTION:  qmi_proxy_release_clients_with_fd

===========================================================================*/
/*!
    @brief
    Release QMI Proxy client connections which matches file descriptor

    @return
    None
*/
/*=========================================================================*/
static void *qmi_proxy_release_clients_with_fd
(
  void *data
)
{
 qmi_proxy_client_info_type *client = NULL, *tmp_client = NULL;
 int i =0;
 qmi_proxy_thread_data_type *thread_data = NULL;

  /* Extract the input data */
  thread_data = ( qmi_proxy_thread_data_type * ) data;

  if ( thread_data == NULL )
  {
    QMI_PROXY_ERR_MSG( "Invalid input parameter in %s\n", __FUNCTION__ );
    return NULL;
  }

  QMI_PROXY_DEBUG_MSG("%s : release clients of all services associated with fd = %d", __FUNCTION__, thread_data->fd );

  /* Cleanup QMI Proxy Client Data */
  for ( i = 0; i < QMI_MAX_SERVICES; i++ )
  {
    QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_client_list_mutex_tbl[ i ] );

    client = qmi_proxy_client_info_tbl[ i ];

    /* list of clients before releasing the clients associated with fd */
    qmi_proxy_print_client_list( i );

    while ( client != NULL )
    {
       tmp_client = client;
       client = client->next;
       if ( tmp_client->fd == thread_data->fd )
       {
         (void) qmi_proxy_release_client_data( tmp_client->proxy_client_id, tmp_client->proxy_srvc_id, FALSE, FALSE );
       }
    }

    QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_client_list_mutex_tbl[ i ] );
  }

  QMI_PROXY_DEBUG_MSG("%s : completed releasing clients of all services associated with fd = %d", __FUNCTION__, thread_data->fd );

  /* free thread data */
  qmi_proxy_free( (void **)&thread_data );

  return NULL;
} /* qmi_proxy_release_clients_with_fd */


/*=========================================================================
  FUNCTION:  qmi_proxy_reset_internal_info

===========================================================================*/
/*!
    @brief
    Reset internal_info variables. Needed to avoid problems after subsystem restart (SSR)
    @return
    None
*/
/*=========================================================================*/

void qmi_proxy_reset_internal_info
(
  void
)
{
    qmi_proxy_internal_info.in_emergency_mode = FALSE;
    qmi_proxy_internal_info.is_cs_registered = FALSE;
    qmi_proxy_internal_info.is_cs_roaming = FALSE;
    qmi_proxy_internal_info.is_dtm_supported[QMI_PROXY_LOCAL_CONN_TYPE] = -1;
    qmi_proxy_internal_info.is_dtm_supported[QMI_PROXY_REMOTE_CONN_TYPE] = -1;
    qmi_proxy_internal_info.is_in_sglte_coverage = FALSE;
    qmi_proxy_internal_info.is_local_svc = FALSE;
    qmi_proxy_internal_info.is_ps_registered = FALSE;
    qmi_proxy_internal_info.is_remote_svc = FALSE;
    qmi_proxy_internal_info.number_of_calls = 0;
    qmi_proxy_internal_info.number_of_emergency_calls = 0;
    qmi_proxy_internal_info.pending_event = SGLTE_SM_EVENT_INVALID;
    qmi_proxy_internal_info.pending_event_name = NULL;
    qmi_proxy_internal_info.cs_active_modem = QMI_PROXY_LOCAL_CONN_TYPE;
    qmi_proxy_internal_info.ps_active_modem = QMI_PROXY_LOCAL_CONN_TYPE;
}/* qmi_proxy_reset_internal_info */


/*=========================================================================
  FUNCTION:  qmi_proxy_release

===========================================================================*/
/*!
    @brief
    Release all QMI service connections and free memory used by QMI Proxy
    clients.

    @return
    None
*/
/*=========================================================================*/
void qmi_proxy_release
(
  void
)
{
  qmi_proxy_client_info_type *client = NULL, *tmp_client = NULL;
  qmi_proxy_txn_info_type *txn = NULL, *tmp_txn = NULL;
  uint8 i;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "%s (enter) .......\n", __FUNCTION__ );

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_mutex );

  if ( qmi_proxy_initialization_done )
  {
    qmi_proxy_initialization_done = FALSE;

    /* Cleanup QMI Proxy Transaction Data */
    for ( i = 0; i < QMI_MAX_SERVICES; i++ )
    {
      QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_txn_list_mutex_tbl[ i ] );

      txn = qmi_proxy_txn_info_tbl[ i ];

      while ( txn != NULL )
      {
        tmp_txn = txn;
        txn = txn->next;
        qmi_proxy_txn_relref( &tmp_txn, TRUE, FALSE );
      }

      qmi_proxy_txn_info_tbl[ i ] = NULL;

      QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_txn_list_mutex_tbl[ i ] );
    }

    /* Cleanup QMI Proxy Client Data */
    for ( i = 0; i < QMI_MAX_SERVICES; i++ )
    {
      QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_client_list_mutex_tbl[ i ] );

      client = qmi_proxy_client_info_tbl[ i ];

      while ( client != NULL )
      {
        tmp_client = client;
        client = client->next;
        (void) qmi_proxy_release_client_data( tmp_client->proxy_client_id, tmp_client->proxy_srvc_id, FALSE, TRUE );
      }

      qmi_proxy_client_info_tbl[ i ] = NULL;

      QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_client_list_mutex_tbl[ i ] );
    }
  }

  //resetting internal info variables
  qmi_proxy_reset_internal_info();

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_mutex );

  QMI_PROXY_DEBUG_MSG( "...... (exit) %s\n", __FUNCTION__ );

} /* qmi_proxy_release */


/*=========================================================================
  FUNCTION:  qmi_proxy_sys_event_rx_hdlr

===========================================================================*/
/*!
    @brief
    Process QMI system event.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static void qmi_proxy_sys_event_rx_hdlr
(
  qmi_sys_event_type            event_id,
  const qmi_sys_event_info_type *event_info,
  void                          *user_data
)
{

  /* -------------------------------------------------------------------  */

  (void) event_info;
  (void) user_data;

  QMI_PROXY_DEBUG_MSG( "%s......Event ID %d\n", __FUNCTION__, event_id );

  /* Modem restart happen */
  if ( event_id == QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND )
  {
    /* remember the ssr info */
    QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.ssr_info_mutex);
    qmi_proxy_internal_info.ssr_info.is_ssr = TRUE;
    qmi_proxy_internal_info.ssr_info.ignore_oprt_mode = TRUE;
    QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.ssr_info_mutex );

    qmi_proxy_release();
    qmi_proxy_release_internal_qmi_handle();
    qmi_proxy_internal_info.initialized = FALSE;
    qmi_proxy_internal_info.svlte_oprt_mode = DMS_OP_MODE_SHUTTING_DOWN_V01;

    // Reset the SGLTE state machine to initial state.
    if (QMI_PROXY_SGLTE_TARGET())
    {
      qmi_proxy_sglte_sm_queue_event(
              qmi_proxy_internal_info.sglte_sm,
              SGLTE_SM_EVENT_LPM_IND,
              "SGLTE_SM_EVENT_LPM_IND",
              NULL);
    }

    QMI_PROXY_DEBUG_MSG( "%s\n", "Modem restarted !!!" );
  }
  else if( event_id == QMI_SYS_EVENT_MODEM_IN_SERVICE_IND )
  {
    /* remember the ssr info */
    QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.ssr_info_mutex);
    qmi_proxy_internal_info.ssr_info.is_ssr = FALSE;
    QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.ssr_info_mutex );
  }

} /* qmi_proxy_sys_event_rx_hdlr */


/*=========================================================================
  FUNCTION:  qmi_proxy_send_dms_err_resp

===========================================================================*/
/*!
    @brief
    sends error response for dms requests

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_dms_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
)
{
  int resp_len = 0, resp_msg_id = -1;
  void *dms_err_resp = NULL;

  switch ( msg_id )
  {
    case QMI_DMS_SET_OPERATING_MODE_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_set_operating_mode_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_set_operating_mode_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_set_operating_mode_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_set_operating_mode_resp_msg_v01);
        resp_msg_id = QMI_DMS_SET_OPERATING_MODE_RESP_V01;
      }
      break;

    case QMI_DMS_RESET_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_reset_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_reset_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_reset_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_reset_resp_msg_v01);
        resp_msg_id = QMI_DMS_RESET_RESP_V01;
      }
      break;

    case QMI_DMS_SET_EVENT_REPORT_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_set_event_report_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_set_event_report_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_set_event_report_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_set_event_report_resp_msg_v01);
        resp_msg_id = QMI_DMS_SET_EVENT_REPORT_RESP_V01;
      }
      break;

    case QMI_DMS_GET_DEVICE_CAP_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_get_device_cap_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_get_device_cap_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_get_device_cap_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_get_device_cap_resp_msg_v01);
        resp_msg_id = QMI_DMS_GET_DEVICE_CAP_RESP_V01;
      }
      break;

    case QMI_DMS_GET_OPERATING_MODE_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_get_operating_mode_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_get_operating_mode_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_get_operating_mode_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_get_operating_mode_resp_msg_v01);
        resp_msg_id = QMI_DMS_GET_OPERATING_MODE_RESP_V01;
      }
      break;

    case QMI_DMS_SET_USER_LOCK_STATE_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_set_user_lock_state_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_set_user_lock_state_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_set_user_lock_state_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_set_user_lock_state_resp_msg_v01);
        resp_msg_id = QMI_DMS_SET_USER_LOCK_STATE_RESP_V01;
      }
      break;

    case QMI_DMS_SET_USER_LOCK_CODE_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_set_user_lock_code_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_set_user_lock_code_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_set_user_lock_code_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_set_user_lock_code_resp_msg_v01);
        resp_msg_id = QMI_DMS_SET_USER_LOCK_CODE_RESP_V01;
      }
      break;

    case QMI_DMS_GET_BAND_CAPABILITY_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_get_band_capability_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_get_band_capability_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_get_band_capability_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_get_band_capability_resp_msg_v01);
        resp_msg_id = QMI_DMS_GET_BAND_CAPABILITY_RESP_V01;
      }
      break;

    case QMI_DMS_SET_TIME_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_set_time_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_set_time_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_set_time_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_set_time_resp_msg_v01);
        resp_msg_id = QMI_DMS_SET_TIME_RESP_V01;
      }
      break;

    case QMI_DMS_UIM_SET_PIN_PROTECTION_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_uim_set_pin_protection_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_uim_set_pin_protection_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_uim_set_pin_protection_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_uim_set_pin_protection_resp_msg_v01);
        resp_msg_id = QMI_DMS_UIM_SET_PIN_PROTECTION_RESP_V01;
      }
      break;

    case QMI_DMS_UIM_VERIFY_PIN_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_uim_verify_pin_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_uim_verify_pin_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_uim_verify_pin_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_uim_verify_pin_resp_msg_v01);
        resp_msg_id = QMI_DMS_UIM_VERIFY_PIN_RESP_V01;
      }
      break;

    case QMI_DMS_UIM_UNBLOCK_PIN_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_uim_unblock_pin_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_uim_verify_pin_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_uim_verify_pin_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_uim_unblock_pin_resp_msg_v01);
        resp_msg_id = QMI_DMS_UIM_UNBLOCK_PIN_RESP_V01;
      }
      break;

    case QMI_DMS_UIM_CHANGE_PIN_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_uim_change_pin_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_uim_verify_pin_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_uim_verify_pin_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_uim_change_pin_resp_msg_v01);
        resp_msg_id = QMI_DMS_UIM_CHANGE_PIN_RESP_V01;
      }
      break;

    case QMI_DMS_UIM_GET_PIN_STATUS_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_uim_get_pin_status_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_uim_verify_pin_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_uim_verify_pin_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_uim_get_pin_status_resp_msg_v01);
        resp_msg_id = QMI_DMS_UIM_GET_PIN_STATUS_RESP_V01;
      }
      break;

    case QMI_DMS_UIM_GET_ICCID_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_uim_get_iccid_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_uim_get_iccid_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_uim_get_iccid_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_uim_get_iccid_resp_msg_v01);
        resp_msg_id = QMI_DMS_UIM_GET_ICCID_RESP_V01;
      }
      break;

    case QMI_DMS_UIM_GET_CK_STATUS_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_uim_get_ck_status_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_uim_get_ck_status_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_uim_get_ck_status_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_uim_get_ck_status_resp_msg_v01);
        resp_msg_id = QMI_DMS_UIM_GET_CK_STATUS_RESP_V01;
      }
      break;

    case QMI_DMS_UIM_SET_CK_PROTECTION_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_uim_set_ck_protection_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_uim_set_ck_protection_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_uim_set_ck_protection_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_uim_set_ck_protection_resp_msg_v01);
        resp_msg_id = QMI_DMS_UIM_SET_CK_PROTECTION_RESP_V01;
      }
      break;

    case QMI_DMS_UIM_UNBLOCK_CK_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_uim_unblock_pin_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_uim_unblock_pin_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_uim_unblock_pin_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_uim_unblock_pin_resp_msg_v01);
        resp_msg_id = QMI_DMS_UIM_UNBLOCK_CK_RESP_V01;
      }
      break;

    case QMI_DMS_UIM_GET_IMSI_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_uim_get_imsi_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_uim_get_imsi_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_uim_get_imsi_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_uim_get_imsi_resp_msg_v01);
        resp_msg_id = QMI_DMS_UIM_GET_IMSI_RESP_V01;
      }
      break;

    case QMI_DMS_UIM_GET_STATE_REQ_V01:
      dms_err_resp = qmi_proxy_malloc( sizeof(dms_uim_get_state_resp_msg_v01) );
      if ( dms_err_resp != NULL )
      {
        ( (dms_uim_get_state_resp_msg_v01 *)dms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (dms_uim_get_state_resp_msg_v01 *)dms_err_resp )->resp.error = error;
        resp_len = sizeof(dms_uim_get_state_resp_msg_v01);
        resp_msg_id = QMI_DMS_UIM_GET_STATE_RESP_V01;
      }
      break;

    default:
      QMI_PROXY_DEBUG_MSG("received un-handled message-id while sending error response = %lu", msg_id);
      break;
  }

  if ( dms_err_resp != NULL  )
  {
    /* Send the QMI Proxy Service Response Message to QMUX */
    qmi_proxy_srvc_send_qmi_response( QMI_DMS_SERVICE,
                                      proxy_client_id,
                                      proxy_conn_id,
                                      proxy_conn_type,
                                      resp_msg_id,
                                      dms_err_resp,
                                      resp_len,
                                      txn_id );

    qmi_proxy_free( (void **)&dms_err_resp );
  }
}


/*=========================================================================
  FUNCTION:  qmi_proxy_send_nas_err_resp

===========================================================================*/
/*!
    @brief
    sends error response for nas requests

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_nas_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
)
{
  void *nas_err_resp = NULL;
  int resp_len = 0, resp_msg_id = -1;

  switch ( msg_id )
  {
    case QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_set_system_selection_preference_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_set_system_selection_preference_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_set_system_selection_preference_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_set_system_selection_preference_resp_msg_v01);
        resp_msg_id = QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_get_system_selection_preference_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_set_system_selection_preference_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_set_system_selection_preference_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_get_system_selection_preference_resp_msg_v01);
        resp_msg_id = QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_RESET_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_reset_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_reset_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_reset_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_reset_resp_msg_v01);
        resp_msg_id = QMI_NAS_RESET_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_ABORT_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_abort_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_abort_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_abort_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_abort_resp_msg_v01);
        resp_msg_id = QMI_NAS_ABORT_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_set_event_report_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_set_event_report_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_set_event_report_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_set_event_report_resp_msg_v01);
        resp_msg_id = QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_indication_register_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_indication_register_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_indication_register_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_indication_register_resp_msg_v01);
        resp_msg_id = QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_SET_ACCOLC_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_set_accolc_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_set_accolc_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_set_accolc_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_set_accolc_resp_msg_v01);
        resp_msg_id = QMI_NAS_SET_ACCOLC_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_SET_DEVICE_CONFIG_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_set_device_config_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_set_device_config_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_set_device_config_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_set_device_config_resp_msg_v01);
        resp_msg_id = QMI_NAS_SET_DEVICE_CONFIG_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_GET_RF_BAND_INFO_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_get_rf_band_info_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_get_rf_band_info_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_get_rf_band_info_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_get_rf_band_info_resp_msg_v01);
        resp_msg_id = QMI_NAS_GET_RF_BAND_INFO_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_set_3gpp2_subscription_info_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_set_3gpp2_subscription_info_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_set_3gpp2_subscription_info_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_set_3gpp2_subscription_info_resp_msg_v01);
        resp_msg_id = QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_SET_RTRE_CONFIG_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_set_rtre_config_resp_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_set_rtre_config_resp_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_set_rtre_config_resp_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_set_rtre_config_resp_v01);
        resp_msg_id = QMI_NAS_SET_RTRE_CONFIG_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_CONFIG_SIG_INFO_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_config_sig_info_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_config_sig_info_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_config_sig_info_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_config_sig_info_resp_msg_v01);
        resp_msg_id = QMI_NAS_CONFIG_SIG_INFO_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_GET_SIG_INFO_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_get_sig_info_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_get_sig_info_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_get_sig_info_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_get_sig_info_resp_msg_v01);
        resp_msg_id = QMI_NAS_GET_SIG_INFO_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_GET_ERR_RATE_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_get_err_rate_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_get_err_rate_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_get_err_rate_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_get_err_rate_resp_msg_v01);
        resp_msg_id = QMI_NAS_GET_ERR_RATE_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_INITIATE_ATTACH_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_initiate_attach_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_initiate_attach_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_initiate_attach_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_initiate_attach_resp_msg_v01);
        resp_msg_id = QMI_NAS_INITIATE_ATTACH_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_GET_PREFERRED_NETWORKS_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_get_preferred_networks_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_get_preferred_networks_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_get_preferred_networks_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_get_preferred_networks_resp_msg_v01);
        resp_msg_id = QMI_NAS_GET_PREFERRED_NETWORKS_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_GET_FORBIDDEN_NETWORKS_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_get_forbidden_networks_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_get_forbidden_networks_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_get_forbidden_networks_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_get_forbidden_networks_resp_msg_v01);
        resp_msg_id = QMI_NAS_GET_FORBIDDEN_NETWORKS_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_SET_FORBIDDEN_NETWORKS_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_set_forbidden_networks_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_set_forbidden_networks_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_set_forbidden_networks_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_set_forbidden_networks_resp_msg_v01);
        resp_msg_id = QMI_NAS_SET_FORBIDDEN_NETWORKS_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_GET_CSP_PLMN_MODE_BIT_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_get_csp_plmn_mode_bit_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_get_csp_plmn_mode_bit_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_get_csp_plmn_mode_bit_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_get_csp_plmn_mode_bit_resp_msg_v01);
        resp_msg_id = QMI_NAS_GET_CSP_PLMN_MODE_BIT_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_GET_OPERATOR_NAME_DATA_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_get_operator_name_data_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_get_operator_name_data_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_get_operator_name_data_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_get_operator_name_data_resp_msg_v01);
        resp_msg_id = QMI_NAS_GET_OPERATOR_NAME_DATA_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_perform_network_scan_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_perform_network_scan_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_perform_network_scan_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_perform_network_scan_resp_msg_v01);
        resp_msg_id = QMI_NAS_PERFORM_NETWORK_SCAN_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_get_plmn_name_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_get_plmn_name_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_get_plmn_name_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_get_plmn_name_resp_msg_v01);
        resp_msg_id = QMI_NAS_GET_PLMN_NAME_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_INITIATE_NETWORK_REGISTER_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_initiate_network_register_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_initiate_network_register_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_initiate_network_register_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_initiate_network_register_resp_msg_v01);
        resp_msg_id = QMI_NAS_INITIATE_NETWORK_REGISTER_RESP_MSG_V01;
      }
      break;

    case QMI_NAS_GET_SYS_INFO_REQ_MSG_V01:
      nas_err_resp = qmi_proxy_malloc( sizeof(nas_get_sys_info_resp_msg_v01) );
      if ( nas_err_resp != NULL )
      {
        ( (nas_get_sys_info_resp_msg_v01 *)nas_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (nas_get_sys_info_resp_msg_v01 *)nas_err_resp )->resp.error = error;
        resp_len = sizeof(nas_get_sys_info_resp_msg_v01);
        resp_msg_id = QMI_NAS_GET_SYS_INFO_RESP_MSG_V01;
      }
      break;

    default:
      QMI_PROXY_DEBUG_MSG("received un-handled message-id while sending error response = %lu", msg_id);
      break;
  }

  if ( nas_err_resp != NULL )
  {
    /* Send the QMI Proxy Service Response Message to QMUX */
    qmi_proxy_srvc_send_qmi_response( QMI_NAS_SERVICE,
                                      proxy_client_id,
                                      proxy_conn_id,
                                      proxy_conn_type,
                                      resp_msg_id,
                                      nas_err_resp,
                                      resp_len,
                                      txn_id );
    qmi_proxy_free( (void **)&nas_err_resp );
  }

} /* qmi_proxy_send_nas_err_resp */


/*=========================================================================
  FUNCTION:  qmi_proxy_send_wms_err_resp

===========================================================================*/
/*!
    @brief
    sends error response for wms requests

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_wms_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
)
{
  void *wms_err_resp = NULL;
  int resp_len = 0, resp_msg_id = -1;

  switch ( msg_id )
  {
    /* Reset the control point WMS state */
    case QMI_WMS_RESET_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_reset_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_reset_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_reset_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_reset_resp_msg_v01);
        resp_msg_id = QMI_WMS_RESET_RESP_V01;
      }
      break;

    /* Set WMS event reporting for the control point */
    case QMI_WMS_SET_EVENT_REPORT_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_set_event_report_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_set_event_report_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_set_event_report_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_set_event_report_resp_msg_v01);
        resp_msg_id = QMI_WMS_SET_EVENT_REPORT_RESP_V01;
      }
      break;

    /* Set the action performed upon WMS message receipt for the specified message routes. It also
       sets the action performed upon WMS receipt of status reports */
    case QMI_WMS_SET_ROUTES_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_set_routes_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_set_routes_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_set_routes_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_set_routes_resp_msg_v01);
        resp_msg_id = QMI_WMS_SET_ROUTES_RESP_V01;
      }
      break;

    /* Set the SMSC address used when storing or saving SMS messages */
    case QMI_WMS_SET_SMSC_ADDRESS_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_set_smsc_address_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_set_smsc_address_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_set_smsc_address_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_set_smsc_address_resp_msg_v01);
        resp_msg_id = QMI_WMS_SET_SMSC_ADDRESS_RESP_V01;
      }
      break;

    /* Configures the retry period */
    case QMI_WMS_SET_RETRY_PERIOD_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_set_retry_period_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_set_retry_period_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_set_retry_period_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_set_retry_period_resp_msg_v01);
        resp_msg_id = QMI_WMS_SET_RETRY_PERIOD_RESP_V01;
      }
      break;

    /* Configures the retry interval */
    case QMI_WMS_SET_RETRY_INTERVAL_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_set_retry_interval_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_set_retry_interval_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_set_retry_interval_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_set_retry_interval_resp_msg_v01);
        resp_msg_id = QMI_WMS_SET_RETRY_INTERVAL_RESP_V01;
      }
      break;

    /* Set whether the client has storage available for new SMS message */
    case QMI_WMS_SET_MEMORY_STATUS_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_set_memory_status_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_set_memory_status_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_set_memory_status_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_set_memory_status_resp_msg_v01);
        resp_msg_id = QMI_WMS_SET_MEMORY_STATUS_RESP_V01;
      }
      break;

    /* Set the GW domain preference */
    case QMI_WMS_SET_DOMAIN_PREF_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_set_domain_pref_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_set_domain_pref_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_set_domain_pref_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_set_domain_pref_resp_msg_v01);
        resp_msg_id = QMI_WMS_SET_DOMAIN_PREF_RESP_V01;
      }
      break;

    /* Set client as primary client */
    case QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_set_primary_client_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_set_primary_client_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_set_primary_client_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_set_primary_client_resp_msg_v01);
        resp_msg_id = QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01;
      }
      break;

    /* Set SMS parameters */
    case QMI_WMS_SET_SMS_PARAMETERS_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_set_sms_parameters_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_set_sms_parameters_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_set_sms_parameters_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_set_sms_parameters_resp_msg_v01);
        resp_msg_id = QMI_WMS_SET_SMS_PARAMETERS_RESP_V01;
      }
      break;

    /* send request for SMS */
    case QMI_WMS_RAW_SEND_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_raw_send_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_raw_send_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_raw_send_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_raw_send_resp_msg_v01);
        resp_msg_id = QMI_WMS_RAW_SEND_RESP_V01;
      }
      break;

    /* Read a message from the device memory storage */
    case QMI_WMS_RAW_READ_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_raw_read_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_raw_read_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_raw_read_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_raw_read_resp_msg_v01);
        resp_msg_id = QMI_WMS_RAW_RESP_MSG_V01;
      }
      break;

    /* Write a new message given in its raw format */
    case QMI_WMS_RAW_WRITE_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_raw_write_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_raw_write_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_raw_write_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_raw_write_resp_msg_v01);
        resp_msg_id = QMI_WMS_RAW_WRITE_RESP_V01;
      }
      break;

    /* Modify the metatag of a message in the device storage */
    case QMI_WMS_MODIFY_TAG_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_modify_tag_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_modify_tag_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_modify_tag_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_modify_tag_resp_msg_v01);
        resp_msg_id = QMI_WMS_MODIFY_TAG_RESP_V01;
      }
      break;

    /* Delete messages in a specified memory location */
    case QMI_WMS_DELETE_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_delete_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_delete_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_delete_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_delete_resp_msg_v01);
        resp_msg_id = QMI_WMS_DELETE_RESP_V01;
      }
      break;

    /* Query a list of WMS message indices and meta information within the specified memory storage,
       matching a specified message tag */
    case QMI_WMS_LIST_MESSAGES_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_list_messages_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_list_messages_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_list_messages_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_list_messages_resp_msg_v01);
        resp_msg_id = QMI_WMS_LIST_MESSAGES_RESP_V01;
      }
      break;

    /* Query the maximum number of messages that can be stored per memory storage, as well as the number of slots
       currently available */
    case QMI_WMS_GET_STORE_MAX_SIZE_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_get_store_max_size_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_get_store_max_size_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_get_store_max_size_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_get_store_max_size_resp_msg_v01);
        resp_msg_id = QMI_WMS_GET_STORE_MAX_SIZE_RESP_V01;
      }
      break;

    /* Sends an ACK to the network for transfer-only routes  */
    case QMI_WMS_SEND_ACK_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_send_ack_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_send_ack_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_send_ack_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_send_ack_resp_msg_v01);
        resp_msg_id = QMI_WMS_SEND_ACK_RESP_V01;
      }
      break;

    /* Enable o disable the reception of broadcast SMS message */
    case QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_set_broadcast_activation_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_set_broadcast_activation_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_set_broadcast_activation_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_set_broadcast_activation_resp_msg_v01);
        resp_msg_id = QMI_WMS_SET_BROADCAST_ACTIVATION_RESP_V01;
      }
      break;

    /* Set the broadcast SMS configuration */
    case QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_set_broadcast_config_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_set_broadcast_config_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_set_broadcast_config_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_set_broadcast_config_resp_msg_v01);
        resp_msg_id = QMI_WMS_SET_BROADCAST_CONFIG_RESP_V01;
      }
      break;

    /* Query the current broadcast SMS configuration */
    case QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_get_broadcast_config_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_get_broadcast_config_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_get_broadcast_config_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_get_broadcast_config_resp_msg_v01);
        resp_msg_id = QMI_WMS_GET_BROADCAST_CONFIG_RESP_V01;
      }
      break;

    /* Query SMS parameters */
    case QMI_WMS_GET_SMS_PARAMETERS_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_get_sms_parameters_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_get_sms_parameters_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_get_sms_parameters_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_get_sms_parameters_resp_msg_v01);
        resp_msg_id = QMI_WMS_GET_SMS_PARAMETERS_RESP_V01;
      }
      break;

    /* Send message from memory store */
    case QMI_WMS_SEND_FROM_MEM_STORE_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_send_from_mem_store_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_send_from_mem_store_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_send_from_mem_store_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_send_from_mem_store_resp_msg_v01);
        resp_msg_id = QMI_WMS_SEND_FROM_MEM_STORE_RESP_V01;
      }
      break;

    /* Register transport layer info events */
    case QMI_WMS_INDICATION_REGISTER_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_indication_register_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_indication_register_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_indication_register_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_indication_register_resp_msg_v01);
        resp_msg_id = QMI_WMS_INDICATION_REGISTER_RESP_V01;
      }
      break;

    /* Query registered transport layer info events */
    case QMI_WMS_GET_INDICATION_REGISTER_REQ_V01:
      wms_err_resp = qmi_proxy_malloc( sizeof(wms_get_indication_register_resp_msg_v01) );
      if ( wms_err_resp != NULL )
      {
        ( (wms_get_indication_register_resp_msg_v01 *)wms_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (wms_get_indication_register_resp_msg_v01 *)wms_err_resp )->resp.error = error;
        resp_len = sizeof(wms_get_indication_register_resp_msg_v01);
        resp_msg_id = QMI_WMS_GET_INDICATION_REGISTER_RESP_V01;
      }
      break;

    default:
      QMI_PROXY_DEBUG_MSG("received un-handled message-id while sending error response = %lu", msg_id);
      break;
  }

  if ( wms_err_resp != NULL )
  {

    /* Send the QMI Proxy Service Response Message to QMUX */
    qmi_proxy_srvc_send_qmi_response( QMI_WMS_SERVICE,
                                      proxy_client_id,
                                      proxy_conn_id,
                                      proxy_conn_type,
                                      resp_msg_id,
                                      wms_err_resp,
                                      resp_len,
                                      txn_id );
    qmi_proxy_free( (void **)&wms_err_resp );
  }

} /* qmi_proxy_wms_err_resp_hdlr */


/*=========================================================================
  FUNCTION:  qmi_proxy_send_pbm_err_resp

===========================================================================*/
/*!
    @brief
    sends error response for pbm requests

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_pbm_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
)
{
  void *pbm_err_resp = NULL;
  int resp_len = 0, resp_msg_id = 0;

  switch ( msg_id )
  {
    case QMI_PBM_INDICATION_REGISTER_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_indication_register_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_indication_register_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_indication_register_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_indication_register_resp_msg_v01);
        resp_msg_id = QMI_PBM_INDICATION_REGISTER_RESP_V01;
      }
      break;

    case QMI_PBM_GET_PB_CAPABILITIES_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_get_pb_capabilities_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_get_pb_capabilities_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_get_pb_capabilities_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_get_pb_capabilities_resp_msg_v01);
        resp_msg_id = QMI_GET_PB_CAPABILITIES_RESP_V01;
      }
      break;

    case QMI_PBM_GET_ALL_PB_CAPABILITIES_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_get_all_pb_capabilities_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_get_all_pb_capabilities_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_get_all_pb_capabilities_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_get_all_pb_capabilities_resp_msg_v01);
        resp_msg_id = QMI_PBM_GET_ALL_PB_CAPABILITIES_RESP_V01;
      }
      break;

    case QMI_PBM_READ_RECORDS_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_read_records_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_read_records_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_read_records_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_read_records_resp_msg_v01);
        resp_msg_id = QMI_PBM_READ_RECORDS_RESP_V01;
      }
      break;

    case QMI_PBM_WRITE_RECORD_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_write_record_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_write_record_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_write_record_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_write_record_resp_msg_v01);
        resp_msg_id = QMI_PBM_WRITE_RECORD_RESP_V01;
      }
      break;

    case QMI_PBM_DELETE_RECORD_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_delete_record_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_delete_record_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_delete_record_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_delete_record_resp_msg_v01);
        resp_msg_id = QMI_PBM_DELETE_RECORD_RESP_V01;
      }
      break;

    case QMI_PBM_DELETE_ALL_PB_RECORDS_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_delete_all_pb_records_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_delete_all_pb_records_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_delete_all_pb_records_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_delete_all_pb_records_resp_msg_v01);
        resp_msg_id = QMI_PBM_DELETE_ALL_PB_RECORDS_RESP_V01;
      }
      break;

    case QMI_PBM_SEARCH_RECORDS_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_search_records_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_search_records_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_search_records_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_search_records_resp_msg_v01);
        resp_msg_id = QMI_PBM_SEARCH_RECORDS_RESP_V01;
      }
      break;

    case QMI_PBM_GET_EMERGENCY_LIST_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_get_emergency_list_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_get_emergency_list_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_get_emergency_list_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_get_emergency_list_resp_msg_v01);
        resp_msg_id = QMI_PBM_GET_EMERGENCY_LIST_RESP_V01;
      }
      break;

    case QMI_PBM_GET_ALL_GROUPS_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_get_all_groups_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_get_all_groups_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_get_all_groups_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_get_all_groups_resp_msg_v01);
        resp_msg_id = QMI_PBM_GET_ALL_GROUPS_RESP_V01;
      }
      break;

    case QMI_PBM_SET_GROUP_INFO_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_set_group_info_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_set_group_info_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_set_group_info_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_set_group_info_resp_msg_v01);
        resp_msg_id = QMI_PBM_SET_GROUP_INFO_RESP_V01;
      }
      break;

    case QMI_PBM_GET_PB_STATE_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_get_pb_state_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_get_pb_state_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_get_pb_state_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_get_pb_state_resp_msg_v01);
        resp_msg_id = QMI_PBM_GET_PB_STATE_RESP_V01;
      }
      break;

    case QMI_PBM_READ_ALL_HIDDEN_RECORDS_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_read_all_hidden_records_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_read_all_hidden_records_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_read_all_hidden_records_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_read_all_hidden_records_resp_msg_v01);
        resp_msg_id = QMI_PBM_READ_ALL_HIDDEN_RECORDS_RESP_V01;
      }
      break;

    case QMI_PBM_BIND_SUBSCRIPTION_REQ_V01:
      pbm_err_resp = qmi_proxy_malloc( sizeof(pbm_bind_subscription_resp_msg_v01) );
      if ( pbm_err_resp != NULL )
      {
        ( (pbm_bind_subscription_resp_msg_v01 *)pbm_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (pbm_bind_subscription_resp_msg_v01 *)pbm_err_resp )->resp.error = error;
        resp_len = sizeof(pbm_bind_subscription_resp_msg_v01);
        resp_msg_id = QMI_PBM_BIND_SUBSCRIPTION_RESP_V01;
      }
      break;

    default:
      QMI_PROXY_DEBUG_MSG("received un-handled message-id while sending error response = %lu", msg_id);
      break;
  }

  if ( pbm_err_resp != NULL  )
  {
    /* Send the QMI Proxy Service Response Message to QMUX */
    qmi_proxy_srvc_send_qmi_response( QMI_PBM_SERVICE,
                                      proxy_client_id,
                                      proxy_conn_id,
                                      proxy_conn_type,
                                      resp_msg_id,
                                      pbm_err_resp,
                                      resp_len,
                                      txn_id );

    qmi_proxy_free( (void **)&pbm_err_resp );
  }

}


/*=========================================================================
  FUNCTION:  qmi_proxy_send_voice_err_resp

===========================================================================*/
/*!
    @brief
    sends error response for voice requests

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_voice_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
)
{
  void *voice_err_resp = NULL;
  int resp_len = 0, resp_msg_id = 0;

  switch ( msg_id )
  {
    case  QMI_VOICE_DIAL_CALL_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_dial_call_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_dial_call_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_dial_call_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_dial_call_resp_msg_v02);
        resp_msg_id = QMI_VOICE_DIAL_CALL_RESP_V02;
      }
      break;

    case  QMI_VOICE_END_CALL_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_end_call_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_end_call_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_end_call_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_end_call_resp_msg_v02);
        resp_msg_id = QMI_VOICE_END_CALL_RESP_V02;
      }
      break;

    case  QMI_VOICE_ANSWER_CALL_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_answer_call_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_answer_call_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_answer_call_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_answer_call_resp_msg_v02);
        resp_msg_id = QMI_VOICE_ANSWER_CALL_RESP_V02;
      }
      break;

    case  QMI_VOICE_GET_CALL_INFO_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_get_call_info_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_get_call_info_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_get_call_info_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_get_call_info_resp_msg_v02);
        resp_msg_id = QMI_VOICE_GET_CALL_INFO_RESP_V02;
      }
      break;

    case  QMI_VOICE_SEND_FLASH_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_send_flash_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_send_flash_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_send_flash_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_send_flash_resp_msg_v02);
        resp_msg_id = QMI_VOICE_SEND_FLASH_RESP_V02;
      }
      break;

    case  QMI_VOICE_BURST_DTMF_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_burst_dtmf_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_burst_dtmf_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_burst_dtmf_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_burst_dtmf_resp_msg_v02);
        resp_msg_id = QMI_VOICE_BURST_DTMF_RESP_V02;
      }
      break;

    case  QMI_VOICE_START_CONT_DTMF_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_start_cont_dtmf_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_start_cont_dtmf_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_start_cont_dtmf_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_start_cont_dtmf_resp_msg_v02);
        resp_msg_id = QMI_VOICE_START_CONT_DTMF_RESP_V02;
      }
      break;

    case  QMI_VOICE_STOP_CONT_DTMF_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_stop_cont_dtmf_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_stop_cont_dtmf_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_stop_cont_dtmf_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_stop_cont_dtmf_resp_msg_v02);
        resp_msg_id = QMI_VOICE_STOP_CONT_DTMF_RESP_V02;
      }
      break;

    case  QMI_VOICE_SET_PREFERRED_PRIVACY_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_set_preferred_privacy_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_set_preferred_privacy_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_set_preferred_privacy_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_set_preferred_privacy_resp_msg_v02);
        resp_msg_id = QMI_VOICE_SET_PREFERRED_PRIVACY_RESP_V02;
      }
      break;

    case  QMI_VOICE_GET_ALL_CALL_INFO_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_get_all_call_info_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_get_all_call_info_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_get_all_call_info_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_get_all_call_info_resp_msg_v02);
        resp_msg_id = QMI_VOICE_GET_ALL_CALL_INFO_RESP_V02;
      }
      break;

    case  QMI_VOICE_MANAGE_CALLS_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_manage_calls_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_manage_calls_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_manage_calls_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_manage_calls_resp_msg_v02);
        resp_msg_id = QMI_VOICE_MANAGE_CALLS_RESP_V02;
      }
      break;

    case  QMI_VOICE_SET_SUPS_SERVICE_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_set_sups_service_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_set_sups_service_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_set_sups_service_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_set_sups_service_resp_msg_v02);
        resp_msg_id = QMI_VOICE_SET_SUPS_SERVICE_RSEP_V02;
      }
      break;

    case  QMI_VOICE_GET_CALL_WAITING_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_get_call_waiting_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_get_call_waiting_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_get_call_waiting_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_get_call_waiting_resp_msg_v02);
        resp_msg_id = QMI_VOICE_GET_CALL_WAITING_RESP_V02;
      }
      break;

    case  QMI_VOICE_GET_CALL_BARRING_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_get_call_barring_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_get_call_barring_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_get_call_barring_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_get_call_barring_resp_msg_v02);
        resp_msg_id = QMI_VOICE_GET_CALL_BARRING_RESP_V02;
      }
      break;

    case  QMI_VOICE_GET_CLIP_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_get_clip_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_get_clip_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_get_clip_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_get_clip_resp_msg_v02);
        resp_msg_id = QMI_VOICE_GET_CLIP_RESP_V02;
      }
      break;

    case  QMI_VOICE_GET_CLIR_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_get_clir_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_get_clir_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_get_clir_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_get_clir_resp_msg_v02);
        resp_msg_id = QMI_VOICE_GET_CLIR_RESP_V02;
      }
      break;

    case  QMI_VOICE_GET_CALL_FORWARDING_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_get_call_forwarding_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_get_call_forwarding_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_get_call_forwarding_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_get_call_forwarding_resp_msg_v02);
        resp_msg_id = QMI_VOICE_GET_CALL_FORWARDING_RESP_V02;
      }
      break;

    case  QMI_VOICE_SET_CALL_BARRING_PASSWORD_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_set_call_barring_password_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_set_call_barring_password_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_set_call_barring_password_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_set_call_barring_password_resp_msg_v02);
        resp_msg_id = QMI_VOICE_SET_CALL_BARRING_PASSWORD_RESP_V02;
      }
      break;

    case  QMI_VOICE_ORIG_USSD_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_orig_ussd_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_orig_ussd_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_orig_ussd_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_orig_ussd_resp_msg_v02);
        resp_msg_id = QMI_VOICE_ORIG_USSD_RESP_V02;
      }
      break;

    case  QMI_VOICE_ANSWER_USSD_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_answer_ussd_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_answer_ussd_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_answer_ussd_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_answer_ussd_resp_msg_v02);
        resp_msg_id = QMI_VOICE_ANSWER_USSD_RESP_V02;
      }
      break;

    case  QMI_VOICE_CANCEL_USSD_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_cancel_ussd_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_cancel_ussd_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_cancel_ussd_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_cancel_ussd_resp_msg_v02);
        resp_msg_id = QMI_VOICE_CANCEL_USSD_RESP_V02;
      }
      break;

    case  QMI_VOICE_SET_CONFIG_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_set_config_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_set_config_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_set_config_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_set_config_resp_msg_v02);
        resp_msg_id = QMI_VOICE_SET_CONFIG_RESP_V02;
      }
      break;

    case  QMI_VOICE_GET_CONFIG_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_get_config_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_get_config_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_get_config_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_get_config_resp_msg_v02);
        resp_msg_id = QMI_VOICE_GET_CONFIG_RESP_V02;
      }
      break;

    case  QMI_VOICE_ORIG_USSD_NO_WAIT_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_orig_ussd_no_wait_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_orig_ussd_no_wait_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_orig_ussd_no_wait_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_orig_ussd_no_wait_resp_msg_v02);
        resp_msg_id = QMI_VOICE_ORIG_USSD_NO_WAIT_RESP_V02;
      }
      break;

    case  QMI_VOICE_BIND_SUBSCRIPTION_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_bind_subscription_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_bind_subscription_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_bind_subscription_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_bind_subscription_resp_msg_v02);
        resp_msg_id = QMI_VOICE_BIND_SUBSCRIPTION_RESP_V02;
      }
      break;

    case  QMI_VOICE_ALS_SET_LINE_SWITCHING_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_als_set_line_switching_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_als_set_line_switching_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_als_set_line_switching_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_als_set_line_switching_resp_msg_v02);
        resp_msg_id = QMI_VOICE_ALS_SET_LINE_SWITCHING_RESP_V02;
      }
      break;

    case  QMI_VOICE_ALS_SELECT_LINE_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_als_select_line_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_als_select_line_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_als_select_line_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_als_select_line_resp_msg_v02);
        resp_msg_id = QMI_VOICE_ALS_SELECT_LINE_RESP_V02;
      }
      break;

    case  QMI_VOICE_AOC_RESET_ACM_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_aoc_reset_acm_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_aoc_reset_acm_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_aoc_reset_acm_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_aoc_reset_acm_resp_msg_v02);
        resp_msg_id = QMI_VOICE_AOC_RESET_ACM_RESP_V02;
      }
      break;

    case  QMI_VOICE_AOC_SET_ACMMAX_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_aoc_set_acmmax_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_aoc_set_acmmax_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_aoc_set_acmmax_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_aoc_set_acmmax_resp_msg_v02);
        resp_msg_id = QMI_VOICE_AOC_SET_ACMMAX_RESP_V02;
      }
      break;

    case  QMI_VOICE_AOC_GET_CALL_METER_INFO_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_aoc_get_call_meter_info_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_aoc_get_call_meter_info_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_aoc_get_call_meter_info_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_aoc_get_call_meter_info_resp_msg_v02);
        resp_msg_id = QMI_VOICE_AOC_GET_CALL_METER_INFO_RESP_V02;
      }
      break;

    case  QMI_VOICE_GET_COLP_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_get_colp_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_get_colp_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_get_colp_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_get_colp_resp_msg_v02);
        resp_msg_id = QMI_VOICE_GET_COLP_RESP_V02;
      }
      break;

    case  QMI_VOICE_GET_COLR_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_get_colr_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_get_colr_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_get_colr_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_get_colr_resp_msg_v02);
        resp_msg_id = QMI_VOICE_GET_COLR_RESP_V02;
      }
      break;

    case  QMI_VOICE_GET_CNAP_REQ_V02:
      voice_err_resp = qmi_proxy_malloc( sizeof(voice_get_cnap_resp_msg_v02) );
      if ( voice_err_resp != NULL )
      {
        ( (voice_get_cnap_resp_msg_v02 *)voice_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (voice_get_cnap_resp_msg_v02 *)voice_err_resp )->resp.error = error;
        resp_len = sizeof(voice_get_cnap_resp_msg_v02);
        resp_msg_id = QMI_VOICE_GET_CNAP_RESP_V02;
      }
      break;

    default:
      QMI_PROXY_DEBUG_MSG("received un-handled message-id while sending error response = %lu", msg_id);
      break;
  }

  if ( voice_err_resp != NULL )
  {
    /* Send the QMI Proxy Service Response Message to QMUX */
    qmi_proxy_srvc_send_qmi_response( QMI_VOICE_SERVICE,
                                      proxy_client_id,
                                      proxy_conn_id,
                                      proxy_conn_type,
                                      resp_msg_id,
                                      voice_err_resp,
                                      resp_len,
                                      txn_id );
    qmi_proxy_free( (void **)&voice_err_resp );
  }
}


/*=========================================================================
  FUNCTION:  qmi_proxy_send_sar_err_resp

===========================================================================*/
/*!
    @brief
    sends error response for sar requests

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_sar_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
)
{
  void *sar_err_resp = NULL;
  int resp_len = 0, resp_msg_id = 0;

  switch ( msg_id )
  {
    case  QMI_SAR_RF_SET_STATE_REQ_MSG_V01:
      sar_err_resp = qmi_proxy_malloc( sizeof(sar_rf_set_state_resp_msg_v01) );
      if ( sar_err_resp != NULL )
      {
        ( (sar_rf_set_state_resp_msg_v01 *)sar_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (sar_rf_set_state_resp_msg_v01 *)sar_err_resp )->resp.error = error;
        resp_len = sizeof(sar_rf_set_state_resp_msg_v01);
        resp_msg_id = QMI_SAR_RF_SET_STATE_RESP_MSG_V01;
      }
      break;

    case  QMI_SAR_RF_GET_STATE_REQ_MSG_V01:
      sar_err_resp = qmi_proxy_malloc( sizeof(sar_rf_get_state_resp_msg_v01) );
      if ( sar_err_resp != NULL )
      {
        ( (sar_rf_get_state_resp_msg_v01 *)sar_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (sar_rf_get_state_resp_msg_v01 *)sar_err_resp )->resp.error = error;
        resp_len = sizeof(sar_rf_get_state_resp_msg_v01);
        resp_msg_id = QMI_SAR_RF_GET_STATE_RESP_MSG_V01;
      }
      break;

    default:
      QMI_PROXY_DEBUG_MSG("received un-handled message-id while sending error response = %lu", msg_id);
      break;
  }

  if ( sar_err_resp != NULL )
  {
    /* Send the QMI Proxy Service Response Message to QMUX */
    qmi_proxy_srvc_send_qmi_response( QMI_SAR_SERVICE,
                                      proxy_client_id,
                                      proxy_conn_id,
                                      proxy_conn_type,
                                      resp_msg_id,
                                      sar_err_resp,
                                      resp_len,
                                      txn_id );
    qmi_proxy_free( (void **)&sar_err_resp );
  }
}


/*=========================================================================
  FUNCTION:  qmi_proxy_ims_vt_err_resp_hdlr

===========================================================================*/
/*!
    @brief
    sends error response for ims requests

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_ims_vt_err_resp_hdlr
(
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  unsigned long          txn_id,
  qmi_error_type_v01 error
)
{
  void *ims_err_resp = NULL;
  int resp_len = 0, resp_msg_id = 0;

  switch ( msg_id )
  {

    case  IMS_VT_DIAL_CALL_REQ_V01:
      ims_err_resp = qmi_proxy_malloc( sizeof(ims_vt_dial_call_resp_v01) );
      if ( ims_err_resp != NULL )
      {
        ( (ims_vt_dial_call_resp_v01 *)ims_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (ims_vt_dial_call_resp_v01 *)ims_err_resp )->resp.error = error;
        resp_len = sizeof(ims_vt_dial_call_resp_v01);
        resp_msg_id = IMS_VT_DIAL_CALL_RESP_V01;
      }
      break;

    case  IMS_VT_END_CALL_REQ_V01:
      ims_err_resp = qmi_proxy_malloc( sizeof(ims_vt_end_call_resp_v01) );
      if ( ims_err_resp != NULL )
      {
        ( (ims_vt_end_call_resp_v01 *)ims_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (ims_vt_end_call_resp_v01 *)ims_err_resp )->resp.error = error;
        resp_len = sizeof(ims_vt_end_call_resp_v01);
        resp_msg_id = IMS_VT_END_CALL_RESP_V01;
      }
      break;

    case  IMS_VT_ANSWER_CALL_REQ_V01:
      ims_err_resp = qmi_proxy_malloc( sizeof(ims_vt_answer_call_req_v01) );
      if ( ims_err_resp != NULL )
      {
        ( (ims_vt_answer_call_resp_v01 *)ims_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (ims_vt_answer_call_resp_v01 *)ims_err_resp )->resp.error = error;
        resp_len = sizeof(ims_vt_answer_call_resp_v01);
        resp_msg_id = IMS_VT_ANSWER_CALL_RESP_V01;
      }
      break;

    case  IMS_VT_GET_CALL_INFO_REQ_V01:
      ims_err_resp = qmi_proxy_malloc( sizeof(ims_vt_get_call_info_resp_v01) );
      if ( ims_err_resp != NULL )
      {
        ( (ims_vt_get_call_info_resp_v01 *)ims_err_resp )->resp.result = QMI_RESULT_FAILURE_V01;
        ( (ims_vt_get_call_info_resp_v01 *)ims_err_resp )->resp.error = error;
        resp_len = sizeof(ims_vt_get_call_info_resp_v01);
        resp_msg_id = IMS_VT_GET_CALL_INFO_RESP_V01;
      }
      break;

    default:
      QMI_PROXY_DEBUG_MSG("received un-handled message-id while sending error response = %lu", msg_id);
      break;
  }

  if ( ims_err_resp != NULL )
  {
    /* Send the QMI Proxy Service Response Message to QMUX */
    qmi_proxy_srvc_send_qmi_response( QMI_IMS_VT_SERVICE,
                                      proxy_client_id,
                                      proxy_conn_id,
                                      proxy_conn_type,
                                      resp_msg_id,
                                      ims_err_resp,
                                      resp_len,
                                      txn_id );

    qmi_proxy_free( (void **)&ims_err_resp );
  }
}


/*=========================================================================
  FUNCTION:  qmi_proxy_setup_internal_qmi_handle

===========================================================================*/
/*!
    @brief
    Register QMI services on the behalf of QMI Proxy to handle Fusion
    requirements.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_setup_internal_qmi_handle
(
  void
)
{
  qmi_proxy_reg_info_type *reg_ptr = NULL;
  qmi_proxy_conn_type conn_type;
  const char *conn_name = NULL;
  uint32 unsol_ind_cb_user_data;
  qmi_proxy_srvc_info_type *srvc_ptr = NULL;
  int rc = QMI_NO_ERR;
  uint8 i;

  /* -------------------------------------------------------------------  */

  /* No need to handle Fusion requirements */
  if ( !QMI_PROXY_SVLTE_TARGET() && !QMI_PROXY_SGLTE_TARGET() )
  {
    return rc;
  }

  QMI_PROXY_DEBUG_MSG( "%s\n", "Setup QMI handles for QMI Proxy Internal" );

  /* Setup QMI Proxy client handle to manage Fusion requirements */
  for ( i = QMI_FIRST_SERVICE; i < QMI_MAX_SERVICES; i++ )
  {
    /* Lookup QMI Proxy registration info */
    reg_ptr = &qmi_proxy_reg_info_tbl[ qmi_proxy_cfg ][ i ];

    /* Register QMI service on local modem for QMI Proxy Internal */
    if ( reg_ptr->internal_local_srvc_required )
    {
      conn_type = QMI_PROXY_LOCAL_CONN_TYPE;
      conn_name = QMI_PROXY_LOCAL_CONN_NAME;
      unsol_ind_cb_user_data = QMI_PROXY_COMPOSE_UNSOL_IND_CB_USER_DATA( conn_type,
                                                                         i,
                                                                         QMI_PROXY_INTERNAL_CLIENT_ID,
                                                                         QMI_PLATFORM_DEV_NAME_TO_CONN_ID( conn_name ) );
      srvc_ptr = &qmi_proxy_internal_info.srvc_info[ conn_type ][ i ];
      rc = qmi_client_init( conn_name,
                            qmi_proxy_srvc_obj_tbl[ i ],
                            qmi_proxy_srvc_unsol_ind_cb,
                            (void *)(intptr_t) unsol_ind_cb_user_data,
                            &srvc_ptr->client_handle );
      if ( rc )
      {
        QMI_PROXY_ERR_MSG( "QPI: failed to register QMI %s(%d} on %s(%d)\n",
                           qmi_proxy_srvc_name[ i ], i, qmi_proxy_modem_name[ conn_type ], conn_type );
        qmi_proxy_free ( (void **)&srvc_ptr->client_handle );
      }
      else
      {
        QMI_PROXY_DEBUG_MSG( "QPI: registered QMI %s(%d} on %s(%d)\n",
                             qmi_proxy_srvc_name[ i ], i, qmi_proxy_modem_name[ conn_type ], conn_type );
        srvc_ptr->registered = TRUE;
      }
    }

    /* Register QMI service on remote modem for QMI Proxy Internal */
    if ( rc == QMI_NO_ERR )
    {
      if ( reg_ptr->internal_remote_srvc_required )
      {
        conn_type = QMI_PROXY_REMOTE_CONN_TYPE;
        conn_name = QMI_PROXY_REMOTE_CONN_NAME;
        unsol_ind_cb_user_data = QMI_PROXY_COMPOSE_UNSOL_IND_CB_USER_DATA( conn_type,
                                                                           i,
                                                                           QMI_PROXY_INTERNAL_CLIENT_ID,
                                                                           QMI_PLATFORM_DEV_NAME_TO_CONN_ID( conn_name ) );
        srvc_ptr = &qmi_proxy_internal_info.srvc_info[ conn_type ][ i ];
        rc = qmi_client_init( conn_name,
                              qmi_proxy_srvc_obj_tbl[ i ],
                              qmi_proxy_srvc_unsol_ind_cb,
                              (void *)(intptr_t) unsol_ind_cb_user_data,
                              &srvc_ptr->client_handle );
        if ( rc )
        {
          QMI_PROXY_ERR_MSG( "QPI: failed to register QMI %s(%d) on %s(%d)\n",
                             qmi_proxy_srvc_name[ i ], i, qmi_proxy_modem_name[ conn_type ], conn_type );
          qmi_proxy_free ( (void **)&srvc_ptr->client_handle );
        }
        else
        {
          QMI_PROXY_DEBUG_MSG( "QPI: registered QMI %s(%d) on %s(%d)\n",
                               qmi_proxy_srvc_name[ i ], i, qmi_proxy_modem_name[ conn_type ], conn_type );
          srvc_ptr->registered = TRUE;
        }
      }
    }
  }

  return rc;

} /* qmi_proxy_setup_internal_qmi_handle */


/*=========================================================================
  FUNCTION:  qmi_proxy_release_internal_qmi_handle

===========================================================================*/
/*!
    @brief
    De-register QMI services registered on the behalf of QMI Proxy.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
int qmi_proxy_release_internal_qmi_handle
(
  void
)
{
  qmi_proxy_conn_type conn_type;
  qmi_proxy_srvc_info_type *srvc_ptr = NULL;
  int rc = QMI_NO_ERR;
  uint8 i;

  /* -------------------------------------------------------------------  */

  QMI_PROXY_DEBUG_MSG( "%s\n", "Release QMI handles for QMI Proxy Internal" );

  for ( i = QMI_FIRST_SERVICE; i < QMI_MAX_SERVICES; i++ )
  {
    /* Release local modem's QMI service */
    conn_type = QMI_PROXY_LOCAL_CONN_TYPE;
    srvc_ptr = &qmi_proxy_internal_info.srvc_info[ conn_type ][ i ];
    if ( srvc_ptr->registered )
    {
      rc = qmi_client_release( srvc_ptr->client_handle );
      srvc_ptr->registered = FALSE;
      if ( rc != QMI_NO_ERR )
      {
        QMI_PROXY_ERR_MSG( "QPI: failed to de-register QMI %s(%d) on %s(%d)\n",
                           qmi_proxy_srvc_name[ i ], i, qmi_proxy_modem_name[ conn_type ], conn_type );
      }
      else
      {
        QMI_PROXY_DEBUG_MSG( "QPI: de-registered QMI %s(%d) on %s(%d)\n",
                             qmi_proxy_srvc_name[ i ], i, qmi_proxy_modem_name[ conn_type ], conn_type );
      }
    }

    /* Release remote modem's QMI service */
    conn_type = QMI_PROXY_REMOTE_CONN_TYPE;
    srvc_ptr = &qmi_proxy_internal_info.srvc_info[ conn_type ][ i ];
    if ( srvc_ptr->registered )
    {
      rc = qmi_client_release( srvc_ptr->client_handle );
      srvc_ptr->registered = FALSE;
      if ( rc != QMI_NO_ERR )
      {
        QMI_PROXY_ERR_MSG( "QPI: failed to de-register QMI %s(%d) on %s(%d)\n",
                           qmi_proxy_srvc_name[ i ], i, qmi_proxy_modem_name[ conn_type ], conn_type );
      }
      else
      {
        QMI_PROXY_DEBUG_MSG( "QPI: de-registered QMI %s(%d) on %s(%d)\n",
                             qmi_proxy_srvc_name[ i ], i, qmi_proxy_modem_name[ conn_type ], conn_type );
      }
    }
  }

  return rc;

} /* qmi_proxy_release_internal_qmi_handle */


/*=========================================================================
  FUNCTION:  qmi_proxy_nas_ind_reg_async_cb

===========================================================================*/
/*!
    @brief
    Handle the asychronous callback for indication register.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_nas_ind_reg_async_cb
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  void                           *resp_cb_data,
  qmi_client_error_type          transp_err
)
{
  qmi_proxy_conn_type conn_type;
  nas_indication_register_resp_msg_v01 *nas_ind_reg_resp = NULL;
  int rc = transp_err;

  /* ------------------------------------------------------------------- */

  (void) user_handle;
  (void) msg_id;
  (void) resp_c_struct_len;

  conn_type = ( qmi_proxy_conn_type ) resp_cb_data;

  nas_ind_reg_resp = ( nas_indication_register_resp_msg_v01 * ) resp_c_struct;
  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, nas_ind_reg_resp ) )
  {
    QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) success to set NAS indication register\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type );
  }
  else
  {
    QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) failed to set NAS indication regsiter, rc=%d\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type, rc );
  }

  /* Free buffer */
  qmi_proxy_free( (void **)&resp_c_struct );

} /* qmi_proxy_nas_ind_reg_async_cb */


/*=========================================================================
  FUNCTION:  qmi_proxy_sync_sys_sel_pref

===========================================================================*/
/*!
    @brief
    Query modem's mode preference and enable unsolicated reporting on modem's
    mode preference change.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_sync_sys_sel_pref
(
  qmi_proxy_conn_type conn_type
)
{
  qmi_client_type client_handle = NULL;
  nas_indication_register_req_msg_v01 *nas_ind_reg_req = NULL;
  nas_indication_register_resp_msg_v01 *nas_ind_reg_resp = NULL;
  nas_get_system_selection_preference_resp_msg_v01 *nas_get_sys_sel_pref_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  uint32 user_data = 0;
  qmi_txn_handle txn_handle = 0;
  qmi_client_error_type rc = QMI_NO_ERR;

  /* ------------------------------------------------------------------- */

  if ( !qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_NAS_SERVICE ].registered )
  {
    QMI_PROXY_ERR_MSG( "QPI: %s(%d) not registered for QMI NAS\n",
                       qmi_proxy_modem_name[ conn_type ], conn_type );

    return QMI_INTERNAL_ERR;
  }
  else
  {
    /* Retrieve the client handle */
    client_handle = qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_NAS_SERVICE ].client_handle;
  }

  /* Allocate buffers */
  nas_ind_reg_req = qmi_proxy_malloc( sizeof( nas_indication_register_req_msg_v01 ) );
  if ( nas_ind_reg_req == NULL )
  {
    rc = QMI_INTERNAL_ERR;
  }
  else
  {
    nas_ind_reg_resp = qmi_proxy_malloc( sizeof( nas_indication_register_resp_msg_v01 ) );
    if ( nas_ind_reg_resp == NULL )
    {
      qmi_proxy_free( (void **)&nas_ind_reg_req );
      rc = QMI_INTERNAL_ERR;
    }
    else
    {
      nas_get_sys_sel_pref_resp = qmi_proxy_malloc( sizeof( nas_get_system_selection_preference_resp_msg_v01 ) );
      if ( nas_get_sys_sel_pref_resp == NULL )
      {
        qmi_proxy_free( (void **)&nas_ind_reg_req );
        qmi_proxy_free( (void **)&nas_ind_reg_resp );
        rc = QMI_INTERNAL_ERR;
      }
    }
  }

  /* Enable unsolicited indication for system selection preference change */
  if ( rc == QMI_NO_ERR )
  {
    nas_ind_reg_req->reg_sys_sel_pref_valid = TRUE;
    nas_ind_reg_req->reg_sys_sel_pref = TRUE;
    nas_ind_reg_req->req_serving_system_valid = TRUE;
    nas_ind_reg_req->req_serving_system = FALSE;
    if ( QMI_PROXY_SGLTE_TARGET() )
    {
      nas_ind_reg_req->sys_info_valid = TRUE;
      nas_ind_reg_req->sys_info       = TRUE;
    }

    user_data = conn_type;
    rc = qmi_client_send_msg_async( client_handle,
                                    QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                    (void *) nas_ind_reg_req,
                                    sizeof( nas_indication_register_req_msg_v01 ),
                                    (void *) nas_ind_reg_resp,
                                    sizeof( nas_indication_register_resp_msg_v01 ),
                                    qmi_proxy_nas_ind_reg_async_cb,
                                    (void *)(intptr_t) user_data,
                                    &txn_handle );
    if ( rc != QMI_NO_ERR )
    {
      QMI_PROXY_ERR_MSG( "QPI: %s(%d) failed to enable QMI NAS to report mode pref change, rc=%d\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type, rc );

      qmi_proxy_free( (void **)&nas_ind_reg_resp );
    }
  }

  /* Sync modem's mode preference */
  if ( rc == QMI_NO_ERR )
  {
    rc = qmi_client_send_msg_sync( client_handle,
                                   QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                   NULL,
                                   0,
                                   (void *) nas_get_sys_sel_pref_resp,
                                   sizeof( nas_get_system_selection_preference_resp_msg_v01 ),
                                   QMI_PROXY_SYNC_REQ_TIMEOUT );
    if ( !QMI_PROXY_RESPONSE_SUCCESS( rc, nas_get_sys_sel_pref_resp ) )
    {
      QMI_PROXY_ERR_MSG( "QPI: %s(%d) failed to query QMI NAS for mode pref, rc=%d\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type, rc );
    }
    else
    {
       svlte_cache_ptr->nas_get_sys_sel_pref_resp = *nas_get_sys_sel_pref_resp;
       QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) mode pref = 0x%x\n",
                            qmi_proxy_modem_name[ conn_type ], conn_type,
                            svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref );
    }
  }

  /* Free buffers */
  qmi_proxy_free( (void **)&nas_ind_reg_req );
  qmi_proxy_free( (void **)&nas_get_sys_sel_pref_resp );

  return rc;

} /* qmi_proxy_sync_sys_sel_pref */


/*=========================================================================
  FUNCTION:  qmi_proxy_set_sys_sel_pref_per_svlte_mode_pref

===========================================================================*/
/*!
    @brief
    Figure out system selection preference per input mode preference.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_set_sys_sel_pref_per_svlte_mode_pref
(
  qmi_proxy_mode_pref_type                        mode_pref,
  nas_set_system_selection_preference_req_msg_v01 *local_nas_set_sys_sel_pref_req,
  nas_set_system_selection_preference_req_msg_v01 *remote_nas_set_sys_sel_pref_req
)
{
  /* ------------------------------------------------------------------- */

  /* Sanity check */
  if ( ( local_nas_set_sys_sel_pref_req == NULL ) ||
       ( remote_nas_set_sys_sel_pref_req == NULL ) )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Null NAS Set Sys Sel Pref Req pointer" );

    return QMI_INTERNAL_ERR;
  }

  /* Figure system selection preference per mode preference */
  switch( mode_pref )
  {
    case QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO:
      local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS;
      remote_nas_set_sys_sel_pref_req->mode_pref_valid = FALSE;
      break;

    case QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED:
      /* Broken QMI interface - it is not able to support the differentation of QMI_PROXY_PREF_GSM_WCDMA_PREFERRED */
      local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS;
      remote_nas_set_sys_sel_pref_req->mode_pref_valid = FALSE;
      break;

    case QMI_PROXY_MODE_PREF_GSM_ONLY:
      local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_GSM;
      remote_nas_set_sys_sel_pref_req->mode_pref_valid = FALSE;
      break;

    case QMI_PROXY_MODE_PREF_WCDMA_ONLY:
      local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_UMTS;
      remote_nas_set_sys_sel_pref_req->mode_pref_valid = FALSE;
      break;

    case QMI_PROXY_MODE_PREF_CDMA_EVDO:
      local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_CDMA;
      remote_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      remote_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_EVDO;
      break;

    case QMI_PROXY_MODE_PREF_CDMA_ONLY:
      local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_CDMA;
      remote_nas_set_sys_sel_pref_req->mode_pref_valid = FALSE;
      break;

    case QMI_PROXY_MODE_PREF_EVDO_ONLY:
      local_nas_set_sys_sel_pref_req->mode_pref_valid = FALSE;
      remote_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      remote_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_EVDO;
      break;

    case QMI_PROXY_MODE_PREF_GSM_WCDMA_CDMA_EVDO:
      local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_GSM |
                                                  QMI_PROXY_MODE_PREF_MASK_UMTS |
                                                  QMI_PROXY_MODE_PREF_MASK_CDMA;
      remote_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      remote_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_EVDO;
      break;

    case QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO:
      local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_CDMA;
      remote_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      remote_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_EVDO;
      break;

    case QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA:
      local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS;
      remote_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      remote_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_LTE;
      break;

    case QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
      local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS |
                                                  QMI_PROXY_MODE_PREF_MASK_CDMA;
      remote_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      remote_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_EVDO;
      break;

    case QMI_PROXY_MODE_PREF_LTE_ONLY:
      local_nas_set_sys_sel_pref_req->mode_pref_valid = FALSE;
      remote_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      remote_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_LTE;
      break;

    default:
      QMI_PROXY_ERR_MSG( "Invalid mode pref %d\n", mode_pref );
      return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;

} /* qmi_proxy_set_sys_sel_pref_per_svlte_mode_pref */


/*=========================================================================
  FUNCTION:  qmi_proxy_use_default_sglte_sys_sel_parameters

===========================================================================*/
/*!
    @brief
    Sets Service Domain and Service Registration Restriction options that
    are used for most SGLTE user mode pref.

    Service Domain is PS/CS on Local Modem and Invalid on Remote Modem
    Service Restrictions is Unrestricted on Local Modem and Invalid on Remote

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_use_default_sglte_sys_sel_parameters
(
  nas_set_system_selection_preference_req_msg_v01 *local_nas_set_sys_sel_pref_req,
  nas_set_system_selection_preference_req_msg_v01 *remote_nas_set_sys_sel_pref_req
)
{
    /* Service Domain */
    local_nas_set_sys_sel_pref_req->srv_domain_pref_valid = TRUE;
    local_nas_set_sys_sel_pref_req->srv_domain_pref = QMI_SRV_DOMAIN_PREF_CS_PS_V01;
    remote_nas_set_sys_sel_pref_req->srv_domain_pref_valid = FALSE;

    /* Service Restrictions */
    local_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = TRUE;
    local_nas_set_sys_sel_pref_req->srv_reg_restriction = NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01;
    remote_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = FALSE;

    /* UE Usage Preference */
    local_nas_set_sys_sel_pref_req->usage_setting_valid = TRUE;
    local_nas_set_sys_sel_pref_req->usage_setting = NAS_USAGE_VOICE_CENTRIC_V01;
    remote_nas_set_sys_sel_pref_req->usage_setting_valid = FALSE;
} /* qmi_proxy_use_default_sglte_sys_sel_parameters */

/*=========================================================================
  FUNCTION:  qmi_proxy_get_sglte_local_expanded_sys_sel_pref_mask_for_oos

===========================================================================*/
/*!
    @brief
    While in OOS it is necessary to have both WCDMA and TDSCDMA when any
    of those is present in the mode_pref. This function adds the required
    bit to the mask
    user mode preference.

    @return
    The modified mode_pref containing both TDSCDMA and WCDMA if either is
    present in the original mask
*/
/*=========================================================================*/
uint16_t qmi_proxy_get_sglte_local_expanded_sys_sel_pref_mask_for_oos
(
  uint16_t local_mask,
  uint16_t remote_mask
)
{
  uint16_t ret = local_mask;

  if (remote_mask & QMI_PROXY_MODE_PREF_MASK_GSM)
  {
    ret |= QMI_PROXY_MODE_PREF_MASK_GSM;
  }

  if ( (QMI_PROXY_MODE_PREF_MASK_TDSCDMA | QMI_PROXY_MODE_PREF_MASK_UMTS  ) &
       local_mask )
  {
    ret |= QMI_PROXY_MODE_PREF_MASK_TDSCDMA |
           QMI_PROXY_MODE_PREF_MASK_UMTS;
  }
  return ret;
} /* qmi_proxy_get_sglte_local_expanded_sys_sel_pref_mask_for_oos */


/*=========================================================================
  FUNCTION:  qmi_proxy_set_sys_sel_pref_per_sglte_mode_pref

===========================================================================*/
/*!
    @brief
    Set the system selection preference parameters according to the SGLTE
    user mode preference.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_set_sys_sel_pref_per_sglte_mode_pref
(
  qmi_proxy_mode_pref_type                        mode_pref,
  int                                             is_sglte_plmn,
  boolean                                         is_user_request,
  nas_set_system_selection_preference_req_msg_v01 *local_nas_set_sys_sel_pref_req,
  nas_set_system_selection_preference_req_msg_v01 *remote_nas_set_sys_sel_pref_req
)
{
  /* ------------------------------------------------------------------- */

  QMI_PROXY_DEBUG_MSG("%s: mode_pref: %d. is_sglte_plmn: %d. is_user_request: %d",
           __FUNCTION__,
           mode_pref,
           is_sglte_plmn,
           is_user_request);

  /* Sanity check */
  if ( ( local_nas_set_sys_sel_pref_req == NULL ) ||
       ( remote_nas_set_sys_sel_pref_req == NULL ) )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Null NAS Set Sys Sel Pref Req pointer" );

    return QMI_INTERNAL_ERR;
  }

  remote_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_GSM;
  remote_nas_set_sys_sel_pref_req->mode_pref_valid = FALSE;

  if ((qmi_proxy_sglte_is_test_mode && (QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA == mode_pref)) ||
        (!(qmi_proxy_sglte_is_test_mode &&
                (QMI_PROXY_MODE_PREF_GSM_ONLY == mode_pref))
           && is_sglte_plmn))
  {
      /* Determine the system selection preference parameters according to
      ** the SGLTE user mode pref */
      switch( mode_pref )
      {
        case QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA:
          /* Mode Pref */
          local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
          local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_LTE |
                                                      QMI_PROXY_MODE_PREF_MASK_TDSCDMA;

          /* Service Domain Pref */
          local_nas_set_sys_sel_pref_req->srv_domain_pref_valid = TRUE;
          local_nas_set_sys_sel_pref_req->srv_domain_pref = QMI_SRV_DOMAIN_PREF_PS_ONLY_V01;
          remote_nas_set_sys_sel_pref_req->srv_domain_pref_valid = TRUE;
          remote_nas_set_sys_sel_pref_req->srv_domain_pref = QMI_SRV_DOMAIN_PREF_CS_ONLY_V01;

          /* Service Restriction */
          local_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = TRUE;
          local_nas_set_sys_sel_pref_req->srv_reg_restriction = 
                                       NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01;

          remote_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
          remote_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = TRUE;
          remote_nas_set_sys_sel_pref_req->srv_reg_restriction = 
                                       NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01;
          /* UE Usage Preference */
          local_nas_set_sys_sel_pref_req->usage_setting_valid = TRUE;
          local_nas_set_sys_sel_pref_req->usage_setting = NAS_USAGE_DATA_CENTRIC_V01;
          break;

        case QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO:
        case QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED:
            /* Broken QMI interface - it is not able to support the differentation of */
            /* QMI_PROXY_PREF_GSM_WCDMA_PREFERRED */
            local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
            local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_GSM |
                                                        QMI_PROXY_MODE_PREF_MASK_TDSCDMA;
          local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;

          /* Service Domain Pref, Service Restrictions and UE Usage Preference*/
          qmi_proxy_use_default_sglte_sys_sel_parameters(local_nas_set_sys_sel_pref_req,
                                                         remote_nas_set_sys_sel_pref_req);

          break;

        case QMI_PROXY_MODE_PREF_LTE_ONLY:
          local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
          local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_LTE;

          /* Service Domain Pref */
          local_nas_set_sys_sel_pref_req->srv_domain_pref_valid = TRUE;
          local_nas_set_sys_sel_pref_req->srv_domain_pref = QMI_SRV_DOMAIN_PREF_CS_PS_V01;

          /* Service Restriction */
          local_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = TRUE;
          local_nas_set_sys_sel_pref_req->srv_reg_restriction = NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01;

          /* UE Usage Preference */
          local_nas_set_sys_sel_pref_req->usage_setting_valid = TRUE;
          local_nas_set_sys_sel_pref_req->usage_setting = NAS_USAGE_DATA_CENTRIC_V01;
          break;

        case QMI_PROXY_MODE_PREF_GSM_ONLY:
          local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
          local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_GSM;

          /* Service Domain Pref, Service Restrictions and UE Usage Preference*/
          qmi_proxy_use_default_sglte_sys_sel_parameters(local_nas_set_sys_sel_pref_req,
                                                         remote_nas_set_sys_sel_pref_req);
          break;

        case QMI_PROXY_MODE_PREF_WCDMA_ONLY:
        case QMI_PROXY_MODE_PREF_TDSCDMA_ONLY:
          local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
          local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_TDSCDMA;

          /* Service Domain Pref, Service Restrictions and UE Usage Preference*/
          qmi_proxy_use_default_sglte_sys_sel_parameters(local_nas_set_sys_sel_pref_req,
                                                         remote_nas_set_sys_sel_pref_req);
          break;

        case QMI_PROXY_MODE_PREF_CDMA_EVDO:
        case QMI_PROXY_MODE_PREF_CDMA_ONLY:
        case QMI_PROXY_MODE_PREF_EVDO_ONLY:
        case QMI_PROXY_MODE_PREF_GSM_WCDMA_CDMA_EVDO:
        case QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO:
        case QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
        default:
          QMI_PROXY_ERR_MSG( "Invalid mode pref %d\n", mode_pref );
          return QMI_INTERNAL_ERR;
      }
  }
  else
  {
      /* Determine the system selection preference parameters according to
      ** the non-SGLTE user mode pref */
      switch( mode_pref )
      {
        case QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA:
          /* Mode Pref */
          local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
          local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_LTE |
                                                      QMI_PROXY_MODE_PREF_MASK_UMTS |
                                                      QMI_PROXY_MODE_PREF_MASK_GSM;
          /* Service Domain Pref, Service Restrictions and UE Usage Preference*/
          qmi_proxy_use_default_sglte_sys_sel_parameters(local_nas_set_sys_sel_pref_req,
                                                         remote_nas_set_sys_sel_pref_req);
          break;

        case QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO:
        case QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED:
          /* Broken QMI interface - it is not able to support the differentation of */
          /* QMI_PROXY_PREF_GSM_WCDMA_PREFERRED */

          /* Mode Pref */
          local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
          local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_UMTS |
                                                      QMI_PROXY_MODE_PREF_MASK_GSM;

          /* Service Domain Pref, Service Restrictions and UE Usage Preference*/
          qmi_proxy_use_default_sglte_sys_sel_parameters(local_nas_set_sys_sel_pref_req,
                                                         remote_nas_set_sys_sel_pref_req);
          break;

        case QMI_PROXY_MODE_PREF_LTE_ONLY:
          local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
          local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_LTE;

          /* Service Domain Pref */
          local_nas_set_sys_sel_pref_req->srv_domain_pref_valid = TRUE;
          local_nas_set_sys_sel_pref_req->srv_domain_pref = QMI_SRV_DOMAIN_PREF_CS_PS_V01;

          /* Service Restriction */
          local_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = TRUE;
          local_nas_set_sys_sel_pref_req->srv_reg_restriction = NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01;

          /* UE Usage Preference */
          local_nas_set_sys_sel_pref_req->usage_setting_valid = TRUE;
          local_nas_set_sys_sel_pref_req->usage_setting = NAS_USAGE_DATA_CENTRIC_V01;
          break;

        case QMI_PROXY_MODE_PREF_GSM_ONLY:
          if (qmi_proxy_sglte_is_test_mode)
          {
            remote_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
            remote_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_GSM;
            local_nas_set_sys_sel_pref_req->mode_pref_valid = FALSE;

            /* Service Domain */
            remote_nas_set_sys_sel_pref_req->srv_domain_pref_valid = TRUE;
            remote_nas_set_sys_sel_pref_req->srv_domain_pref = QMI_SRV_DOMAIN_PREF_CS_PS_V01;
            local_nas_set_sys_sel_pref_req->srv_domain_pref_valid = FALSE;

            /* Service Restrictions */
            remote_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = TRUE;
            remote_nas_set_sys_sel_pref_req->srv_reg_restriction =
                                                    NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01;
            local_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = FALSE;
          }
          else
          {
            local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
            local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_GSM;

            /* Service Domain Pref, Service Restrictions and UE Usage Preference*/
            qmi_proxy_use_default_sglte_sys_sel_parameters(local_nas_set_sys_sel_pref_req,
                                                         remote_nas_set_sys_sel_pref_req);
          }
          break;

        case QMI_PROXY_MODE_PREF_WCDMA_ONLY:
          local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
          local_nas_set_sys_sel_pref_req->mode_pref = QMI_PROXY_MODE_PREF_MASK_UMTS;

          /* Service Domain Pref, Service Restrictions and UE Usage Preference*/
          qmi_proxy_use_default_sglte_sys_sel_parameters(local_nas_set_sys_sel_pref_req,
                                                         remote_nas_set_sys_sel_pref_req);
          break;

        case QMI_PROXY_MODE_PREF_CDMA_EVDO:
        case QMI_PROXY_MODE_PREF_CDMA_ONLY:
        case QMI_PROXY_MODE_PREF_EVDO_ONLY:
        case QMI_PROXY_MODE_PREF_GSM_WCDMA_CDMA_EVDO:
        case QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO:
        case QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
        default:
            QMI_PROXY_ERR_MSG( "Invalid mode pref %d\n", mode_pref );
            return QMI_INTERNAL_ERR;
        }
  }

  if (!remote_nas_set_sys_sel_pref_req->mode_pref_valid)
  {
    remote_nas_set_sys_sel_pref_req->mode_pref = 0;
  }

  if (!is_user_request)
  {
    /* Correct the Service Restrictions */
    qmi_proxy_correct_sys_sel_pref_srv_reg_restriction(local_nas_set_sys_sel_pref_req,
                                                       remote_nas_set_sys_sel_pref_req);

    /* Adjust for Complete Out of Service scenario */
    if (is_sglte_plmn < 0 && !qmi_proxy_sglte_is_test_mode )
    {
      /* Adjust local mode_pref */
      local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      local_nas_set_sys_sel_pref_req->mode_pref =
              qmi_proxy_get_sglte_local_expanded_sys_sel_pref_mask_for_oos(
                      local_nas_set_sys_sel_pref_req->mode_pref,
                      remote_nas_set_sys_sel_pref_req->mode_pref);
      if (mode_pref == QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA)
      {
        local_nas_set_sys_sel_pref_req->srv_domain_pref = QMI_SRV_DOMAIN_PREF_PS_ONLY_V01;
        remote_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = TRUE;
        remote_nas_set_sys_sel_pref_req->srv_reg_restriction =
                                                        NAS_SRV_REG_RESTRICTION_LIMITED_V01;
        remote_nas_set_sys_sel_pref_req->change_duration_valid = TRUE;
        remote_nas_set_sys_sel_pref_req->change_duration = NAS_POWER_CYCLE_V01;

        remote_nas_set_sys_sel_pref_req->srv_domain_pref_valid = TRUE;
        remote_nas_set_sys_sel_pref_req->srv_domain_pref = QMI_SRV_DOMAIN_PREF_CS_ONLY_V01;
      }
      else
      {
        local_nas_set_sys_sel_pref_req->srv_domain_pref = QMI_SRV_DOMAIN_PREF_CS_PS_V01;
      }

      local_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = TRUE;
      local_nas_set_sys_sel_pref_req->srv_reg_restriction =
                                                      NAS_SRV_REG_RESTRICTION_LIMITED_V01;
      local_nas_set_sys_sel_pref_req->change_duration_valid = TRUE;
      local_nas_set_sys_sel_pref_req->change_duration = NAS_POWER_CYCLE_V01;

    }
  }
  else
  {
    if (is_sglte_plmn < 0) {
      /* Adjust local mode_pref */
      local_nas_set_sys_sel_pref_req->mode_pref_valid = TRUE;
      local_nas_set_sys_sel_pref_req->mode_pref =
              qmi_proxy_get_sglte_local_expanded_sys_sel_pref_mask_for_oos(
                      local_nas_set_sys_sel_pref_req->mode_pref,
                      remote_nas_set_sys_sel_pref_req->mode_pref);
    }
    /* Don't send srv domain or restriction, to prevent triggering user selection mode in CM
     * by virtue of sending the parameters twice
     */
    local_nas_set_sys_sel_pref_req->srv_domain_pref_valid = FALSE;
    local_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = FALSE;
    local_nas_set_sys_sel_pref_req->change_duration_valid = FALSE;
    remote_nas_set_sys_sel_pref_req->srv_domain_pref_valid = FALSE;
    remote_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = FALSE;
    remote_nas_set_sys_sel_pref_req->change_duration_valid = FALSE;
  }

  QMI_PROXY_DEBUG_MSG( "SGLTE: local mode_pref_valid=%d, mode_pref=0x%x\n",
                       local_nas_set_sys_sel_pref_req->mode_pref_valid,
                       local_nas_set_sys_sel_pref_req->mode_pref);
  QMI_PROXY_DEBUG_MSG( "SGLTE: remote mode_pref_valid=%d, mode_pref=0x%x\n",
                       remote_nas_set_sys_sel_pref_req->mode_pref_valid,
                       remote_nas_set_sys_sel_pref_req->mode_pref);
  QMI_PROXY_DEBUG_MSG( "SGLTE: local srv_reg_restriction_valid=%d, srv_reg_restriction=%d\n",
                       local_nas_set_sys_sel_pref_req->srv_reg_restriction_valid,
                       local_nas_set_sys_sel_pref_req->srv_reg_restriction);
  QMI_PROXY_DEBUG_MSG( "SGLTE: remote srv_reg_restriction_valid=%d, srv_reg_restriction=%d\n",
                       remote_nas_set_sys_sel_pref_req->srv_reg_restriction_valid,
                       remote_nas_set_sys_sel_pref_req->srv_reg_restriction);
  QMI_PROXY_DEBUG_MSG( "SGLTE: local srv_domain_pref_valid=%d, srv_domain_pref=%d\n",
                       local_nas_set_sys_sel_pref_req->srv_domain_pref_valid,
                       local_nas_set_sys_sel_pref_req->srv_domain_pref);
  QMI_PROXY_DEBUG_MSG( "SGLTE: remote srv_domain_pref_valid=%d, srv_domain_pref=%d\n",
                       remote_nas_set_sys_sel_pref_req->srv_domain_pref_valid,
                       remote_nas_set_sys_sel_pref_req->srv_domain_pref);

  return QMI_NO_ERR;

} /* qmi_proxy_set_sys_sel_pref_per_sglte_mode_pref */


/*=========================================================================
  FUNCTION:  qmi_proxy_nas_set_sys_sel_pref_async_cb

===========================================================================*/
/*!
    @brief
    Handle the asychronous callback for setting system selection preference.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_nas_set_sys_sel_pref_async_cb
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  void                           *resp_cb_data,
  qmi_client_error_type          transp_err
)
{
  qmi_proxy_conn_type conn_type;
  nas_set_system_selection_preference_resp_msg_v01 *nas_set_sys_sel_pref_resp = NULL;
  uint16_t mode_pref;
  uint32 user_data = 0;
  int rc = transp_err;

  /* ------------------------------------------------------------------- */

  (void) user_handle;
  (void) msg_id;
  (void) resp_c_struct_len;

  user_data = (intptr_t) resp_cb_data;

  conn_type = QMI_PROXY_NAS_ASYNCB_USER_DATA_TO_CONN_TYPE( user_data );

  mode_pref = QMI_PROXY_NAS_ASYNCB_USER_DATA_TO_MODE_PREF( user_data );

  nas_set_sys_sel_pref_resp = ( nas_set_system_selection_preference_resp_msg_v01 * ) resp_c_struct;

  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, nas_set_sys_sel_pref_resp ) )
  {
    QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) success to set mode pref 0x%x per SVLTE mode pref\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type, mode_pref );
  }
  else
  {
    QMI_PROXY_ERR_MSG( "QPI: %s(%d) failed to set mode pref 0x%x to match SVLTE mode pref, rc=%d\n",
                       qmi_proxy_modem_name[ conn_type ], conn_type, mode_pref, rc );
  }

  /* Free buffer */
  qmi_proxy_free( (void **)&resp_c_struct );

} /* qmi_proxy_nas_set_sys_sel_pref_async_cb */


/*=========================================================================
  FUNCTION:  qmi_proxy_sync_svlte_mode_pref

===========================================================================*/
/*!
    @brief
    Determine the SVLTE mode preference at powerup.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_sync_svlte_mode_pref
(
  void
)
{
  qmi_client_type client_handle = NULL;
  nas_set_system_selection_preference_req_msg_v01 *local_nas_set_sys_sel_pref_req = NULL, *remote_nas_set_sys_sel_pref_req = NULL;
  nas_set_system_selection_preference_resp_msg_v01 *local_nas_set_sys_sel_pref_resp = NULL, *remote_nas_set_sys_sel_pref_resp = NULL;
  boolean svlte_mode_pref_matched = FALSE;
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  uint32 user_data = 0;
  qmi_txn_handle txn_handle = 0;
  int rc = QMI_NO_ERR, tmp_rc = QMI_NO_ERR;

  /* ------------------------------------------------------------------- */

  /* Sanity check */
  if ( !QMI_PROXY_SVLTE_TARGET() )
  {
    return rc;
  }

  /* Allocate buffers */
  local_nas_set_sys_sel_pref_req = qmi_proxy_malloc( sizeof( nas_set_system_selection_preference_req_msg_v01 ) );
  if ( local_nas_set_sys_sel_pref_req == NULL )
  {
    return QMI_INTERNAL_ERR;
  }

  local_nas_set_sys_sel_pref_resp = qmi_proxy_malloc( sizeof( nas_set_system_selection_preference_resp_msg_v01 ) );
  if ( local_nas_set_sys_sel_pref_resp == NULL )
  {
    qmi_proxy_free( (void **)&local_nas_set_sys_sel_pref_req );
    return QMI_INTERNAL_ERR;
  }

  remote_nas_set_sys_sel_pref_req = qmi_proxy_malloc( sizeof( nas_set_system_selection_preference_req_msg_v01 ) );
  if ( remote_nas_set_sys_sel_pref_req == NULL )
  {
    qmi_proxy_free( (void **)&local_nas_set_sys_sel_pref_req );
    qmi_proxy_free( (void **)&local_nas_set_sys_sel_pref_resp);
    return QMI_INTERNAL_ERR;
  }

  remote_nas_set_sys_sel_pref_resp = qmi_proxy_malloc( sizeof( nas_set_system_selection_preference_resp_msg_v01 ) );
  if ( remote_nas_set_sys_sel_pref_resp == NULL )
  {
    qmi_proxy_free( (void **)&local_nas_set_sys_sel_pref_req );
    qmi_proxy_free( (void **)&local_nas_set_sys_sel_pref_resp);
    qmi_proxy_free( (void **)&remote_nas_set_sys_sel_pref_req );
    return QMI_INTERNAL_ERR;
  }

  /* Read last saved SVLTE Mode Preference */
  qmi_proxy_read_svlte_cfg();

  /* Figure out system selection preference per SVLTE mode preference */
  qmi_proxy_set_sys_sel_pref_per_svlte_mode_pref( qmi_proxy_internal_info.svlte_mode_pref,
                                                  local_nas_set_sys_sel_pref_req,
                                                  remote_nas_set_sys_sel_pref_req );

  /* Verify the SVLTE mode preference vs modems' mode preference setting */
  if ( ( !local_nas_set_sys_sel_pref_req->mode_pref_valid ||
         ( local_nas_set_sys_sel_pref_req->mode_pref_valid &&
           ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == local_nas_set_sys_sel_pref_req->mode_pref ) ) ) &&
         ( !remote_nas_set_sys_sel_pref_req->mode_pref_valid ||
           ( remote_nas_set_sys_sel_pref_req->mode_pref_valid &&
             ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == remote_nas_set_sys_sel_pref_req->mode_pref ) ) ) )
  {
    QMI_PROXY_DEBUG_MSG( "QPI: SVLTE mode pref = %s(%d)\n",
                         qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.svlte_mode_pref ],
                         qmi_proxy_internal_info.svlte_mode_pref );
    svlte_mode_pref_matched = TRUE;

    /* release the response buffer in case mode preference match */
    qmi_proxy_free( (void **)&local_nas_set_sys_sel_pref_resp );
    qmi_proxy_free( (void **)&remote_nas_set_sys_sel_pref_resp );
  }

  /* Either saved SVLTE mode preference not found or saved SVLTE mode preference not matching modems' mode preference setting */
  if ( !svlte_mode_pref_matched )
  {
    QMI_PROXY_ERR_MSG( "QPI: %s(%d) mode pref = 0x%x and %s(%d) mode_pref = 0x%x not match saved SVLTE mode pref = %s(%d)\n",
                       qmi_proxy_modem_name[ QMI_PROXY_LOCAL_CONN_TYPE ], QMI_PROXY_LOCAL_CONN_TYPE,
                       local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref,
                       qmi_proxy_modem_name[ QMI_PROXY_REMOTE_CONN_TYPE ], QMI_PROXY_REMOTE_CONN_TYPE,
                       remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref,
                       qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.svlte_mode_pref ],
                       qmi_proxy_internal_info.svlte_mode_pref );

    /* Deduce SVLTE mode preference from modems' mode preference setting */
    if ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_CDMA )
    {
      if ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_EVDO )
      {
        qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_CDMA_EVDO;
      }
      else if ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
                ( QMI_PROXY_MODE_PREF_MASK_EVDO | QMI_PROXY_MODE_PREF_MASK_LTE ) )
      {
        qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO;
      }
      else
      {
        qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_CDMA_ONLY;
      }
    }
    else if ( ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
                ( QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS | QMI_PROXY_MODE_PREF_MASK_CDMA ) )  ||
              ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
                ( QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS | QMI_PROXY_MODE_PREF_MASK_CDMA |QMI_PROXY_MODE_PREF_MASK_EVDO) ) )
    {
      /* above check for EVDO is for supporting the automatic mode during power on,
           as nas reports all technologies in mode preference in case of automatic mode */
      if ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
           ( QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_EVDO ) )
      {
        qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO_GSM_WCDMA;
      }
      else if ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_EVDO )
      {
        qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_GSM_WCDMA_CDMA_EVDO;
      }
    }
    else if ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
              ( QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS ) )
    {
      if ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_LTE )
      {
        qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA;
      }
      else
      {
        /* Broken QMI interface - it is not able to support the differentation of QMI_PROXY_PREF_GSM_WCDMA_PREFERRED */
        qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO;
      }
    }
    else if ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_GSM )
    {
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_GSM_ONLY;
    }
    else if ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_UMTS )
    {
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_WCDMA_ONLY;
    }
    else if ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_EVDO )
    {
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_EVDO_ONLY;
    }
    else if ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_LTE )
    {
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_ONLY;
    }

    QMI_PROXY_DEBUG_MSG( "SVLTE mode pref from modem setting = %s(%d)\n",
                         qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.svlte_mode_pref ],
                         qmi_proxy_internal_info.svlte_mode_pref );


    /* Save SVLTE Mode Preference setting */
    qmi_proxy_save_svlte_cfg();

    /* Figure out system selection preference per SVLTE mode preference */
    qmi_proxy_set_sys_sel_pref_per_svlte_mode_pref( qmi_proxy_internal_info.svlte_mode_pref,
                                                    local_nas_set_sys_sel_pref_req,
                                                    remote_nas_set_sys_sel_pref_req );


    /* Set the local modem's mode preference setting per SVLTE mode preference */
    if ( local_nas_set_sys_sel_pref_req->mode_pref_valid &&
         ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref != local_nas_set_sys_sel_pref_req->mode_pref ) )
    {
      if ( !qmi_proxy_internal_info.srvc_info[ QMI_PROXY_LOCAL_CONN_TYPE ][ QMI_NAS_SERVICE ].registered )
      {
        QMI_PROXY_ERR_MSG( "QPI: %s(%d) not registered for QMI NAS\n",
                           qmi_proxy_modem_name[ QMI_PROXY_LOCAL_CONN_TYPE ], QMI_PROXY_LOCAL_CONN_TYPE );
        rc = QMI_INTERNAL_ERR;

        /* Free buffer */
        qmi_proxy_free ( (void **)&local_nas_set_sys_sel_pref_resp );
      }
      else
      {
        client_handle = qmi_proxy_internal_info.srvc_info[ QMI_PROXY_LOCAL_CONN_TYPE ][ QMI_NAS_SERVICE ].client_handle;
        user_data = QMI_PROXY_COMPOSE_NAS_MODE_PREF_ASYNCB_USER_DATA( QMI_PROXY_LOCAL_CONN_TYPE,
                                                                      local_nas_set_sys_sel_pref_req->mode_pref );
        tmp_rc = qmi_client_send_msg_async( client_handle,
                                            QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                            (void *) local_nas_set_sys_sel_pref_req,
                                            sizeof( nas_set_system_selection_preference_req_msg_v01 ),
                                            (void *) local_nas_set_sys_sel_pref_resp,
                                            sizeof( nas_set_system_selection_preference_resp_msg_v01 ),
                                            qmi_proxy_nas_set_sys_sel_pref_async_cb,
                                            (void *)(intptr_t) user_data,
                                            &txn_handle );

        if ( tmp_rc != QMI_NO_ERR )
        {
          QMI_PROXY_ERR_MSG( "QPI: %s(%d) failed to set mode pref 0x%x to match SVLTE preference %s(%d), rc=%d\n",
                             qmi_proxy_modem_name[ QMI_PROXY_LOCAL_CONN_TYPE ], QMI_PROXY_LOCAL_CONN_TYPE,
                             local_nas_set_sys_sel_pref_req->mode_pref,
                             qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.svlte_mode_pref ],
                             qmi_proxy_internal_info.svlte_mode_pref,
                             tmp_rc );

          qmi_proxy_free( (void **)&local_nas_set_sys_sel_pref_resp );
        }
      }
    }
    else
    {
      qmi_proxy_free ( (void **)&local_nas_set_sys_sel_pref_resp );
    }

    /* Set remote modem's mode preference setting per SVLTE mode preference */
    if ( ( rc == QMI_NO_ERR ) && remote_nas_set_sys_sel_pref_req->mode_pref_valid &&
         ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref != remote_nas_set_sys_sel_pref_req->mode_pref ) )
    {
      if ( !qmi_proxy_internal_info.srvc_info[ QMI_PROXY_REMOTE_CONN_TYPE ][ QMI_NAS_SERVICE ].registered )
      {
        QMI_PROXY_ERR_MSG( "QPI: %s(%d) not registered for QMI NAS\n",
                           qmi_proxy_modem_name[ QMI_PROXY_REMOTE_CONN_TYPE ], QMI_PROXY_REMOTE_CONN_TYPE );
        rc = QMI_INTERNAL_ERR;

        /* Free buffer */
        qmi_proxy_free ( (void **)&remote_nas_set_sys_sel_pref_resp );
      }
      else
      {
        client_handle = qmi_proxy_internal_info.srvc_info[ QMI_PROXY_REMOTE_CONN_TYPE ][ QMI_NAS_SERVICE ].client_handle;
        user_data = QMI_PROXY_COMPOSE_NAS_MODE_PREF_ASYNCB_USER_DATA( QMI_PROXY_REMOTE_CONN_TYPE,
                                                                      remote_nas_set_sys_sel_pref_req->mode_pref );
        tmp_rc = qmi_client_send_msg_async( client_handle,
                                            QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                            (void*) remote_nas_set_sys_sel_pref_req,
                                            sizeof( nas_set_system_selection_preference_req_msg_v01 ),
                                            (void*) remote_nas_set_sys_sel_pref_resp,
                                            sizeof( nas_set_system_selection_preference_resp_msg_v01 ),
                                            qmi_proxy_nas_set_sys_sel_pref_async_cb,
                                            (void *)(intptr_t) user_data,
                                            &txn_handle );

        if ( tmp_rc != QMI_NO_ERR )
        {
          QMI_PROXY_ERR_MSG( "QPI: %s(%d) failed to set mode pref = 0x%x to match SVLTE preference %s(%d), rc=%d\n",
                             qmi_proxy_modem_name[ QMI_PROXY_REMOTE_CONN_TYPE ], QMI_PROXY_REMOTE_CONN_TYPE,
                             remote_nas_set_sys_sel_pref_req->mode_pref,
                             qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.svlte_mode_pref ],
                             qmi_proxy_internal_info.svlte_mode_pref,
                             tmp_rc );

          qmi_proxy_free( (void **)&remote_nas_set_sys_sel_pref_resp );
        }
      }
    }
    else
    {
      qmi_proxy_free( (void **)&remote_nas_set_sys_sel_pref_resp );
    }
  }

  /* Free buffers */
  qmi_proxy_free( (void **)&local_nas_set_sys_sel_pref_req );
  qmi_proxy_free( (void **)&remote_nas_set_sys_sel_pref_req );

  return rc;

} /* qmi_proxy_sync_svlte_mode_pref */


/*=========================================================================
  FUNCTION:  qmi_proxy_set_cache_svlte_radio_if_mask

===========================================================================*/
/*!
    @brief
    Set cached SVLTE radio IF mask

    @return
  None
*/
/*=========================================================================*/
static void qmi_proxy_set_cache_svlte_radio_if_mask
(
  void
)
{
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = NULL;
  uint32 i, j;

  /*-----------------------------------------------------------------------*/

  /* Figure the modem's radio if mask */
  qmi_proxy_internal_info.svlte_radio_if_mask = 0;

  for ( i = 0; i < QMI_PROXY_MAX_CONN_TYPE; i++ )
  {
    svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ i ];

    if ( svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities.radio_if_list_len > 0 )
    {
      for ( j = 0; j < svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities.radio_if_list_len &&
                   j < QMI_DMS_RADIO_IF_LIST_MAX_V01; j++ )
      {
        switch( svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities.radio_if_list[ j ] )
        {
          case DMS_RADIO_IF_1X_V01:
            qmi_proxy_internal_info.svlte_radio_if_mask |= QMI_PROXY_MODE_PREF_MASK_CDMA;
            break;

          case DMS_RADIO_IF_1X_EVDO_V01:
            qmi_proxy_internal_info.svlte_radio_if_mask |= QMI_PROXY_MODE_PREF_MASK_EVDO;
            break;

          case DMS_RADIO_IF_GSM_V01:
            qmi_proxy_internal_info.svlte_radio_if_mask |= QMI_PROXY_MODE_PREF_MASK_GSM;
            break;

          case DMS_RADIO_IF_UMTS_V01:
            qmi_proxy_internal_info.svlte_radio_if_mask |= QMI_PROXY_MODE_PREF_MASK_UMTS;
            break;

          case DMS_RADIO_IF_LTE_V01:
            qmi_proxy_internal_info.svlte_radio_if_mask |= QMI_PROXY_MODE_PREF_MASK_LTE;
            break;

          case DMS_RADIO_IF_TDS_V01:
            qmi_proxy_internal_info.svlte_radio_if_mask |= QMI_PROXY_MODE_PREF_MASK_TDSCDMA;
            break;

          default:
            break;
        }
      }
    }
  }

} /* qmi_proxy_set_cache_svlte_radio_if_mask */


/*=========================================================================
  FUNCTION:  qmi_proxy_lookup_oprt_mode_name

===========================================================================*/
/*!
    @brief
    Lookup operating mode name

    @return
    Operating mode name
*/
/*=========================================================================*/
static char *qmi_proxy_lookup_oprt_mode_name
(
  dms_operating_mode_enum_v01 oprt_mode
)
{
  /*-----------------------------------------------------------------------*/

  switch ( oprt_mode )
  {
    case DMS_OP_MODE_ONLINE_V01:
      return "Online";

    case DMS_OP_MODE_LOW_POWER_V01:
      return "LPM";

    case DMS_OP_MODE_FACTORY_TEST_MODE_V01:
      return "FTM";

    case DMS_OP_MODE_OFFLINE_V01:
      return "Offline";

    case DMS_OP_MODE_RESETTING_V01:
      return "Resetting";

    case DMS_OP_MODE_SHUTTING_DOWN_V01:
      return "Power off";

    default:
      return "Unknown";
  }

} /* qmi_proxy_lookup_oprt_mode_name */


/*=========================================================================
  FUNCTION:  qmi_proxy_get_cache_svlte_operating_mode

===========================================================================*/
/*!
    @brief
    Get cached SVLTE operating mode

    @return
    SVLTE operating mode
*/
/*=========================================================================*/
static uint8_t qmi_proxy_get_cache_svlte_operating_mode
(
  void
)
{
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];

  /*-----------------------------------------------------------------------*/

  /* Any modem in FTM mode means FTM */
  if ( ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) ||
       ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) )
  {
    qmi_proxy_internal_info.svlte_oprt_mode = DMS_OP_MODE_FACTORY_TEST_MODE_V01;
  }
  /* Commenting out the following piece of code, as SHUTTING_DOWN is considered as valid case during power on,
     removing check that shutting down of either modem should be considered as overall svlte2 shutting down,
     giving priority to operating mode preference then considering "Any modem in SHUTTING_DOWN mode means SHUTTING_DOWN" */
  /*else if ( ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_SHUTTING_DOWN_V01 ) ||
            ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_SHUTTING_DOWN_V01 ) )
  {
    qmi_proxy_internal_info.svlte_oprt_mode = DMS_OP_MODE_SHUTTING_DOWN_V01;
  }*/
  /* Any modem in RESETTING mode means RESETTING */
  else if ( ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_RESETTING_V01 ) ||
            ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_RESETTING_V01 ) )
  {
    qmi_proxy_internal_info.svlte_oprt_mode = DMS_OP_MODE_RESETTING_V01;
  }
  /* Any modem in OFFLINE mode means OFFLINE */
  else if ( ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_OFFLINE_V01 ) ||
            ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_OFFLINE_V01 ) )
  {
    qmi_proxy_internal_info.svlte_oprt_mode = DMS_OP_MODE_OFFLINE_V01;
  }
  else
  {
    switch ( qmi_proxy_internal_info.svlte_mode_pref )
    {
      /* Local modem should be the good one to check on operating mode */
      case QMI_PROXY_MODE_PREF_CDMA_ONLY:
      case QMI_PROXY_MODE_PREF_GSM_ONLY:
      case QMI_PROXY_MODE_PREF_WCDMA_ONLY:
      case QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO:
      case QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED:
        qmi_proxy_internal_info.svlte_oprt_mode = local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode;
        break;

      /* Remote modem should be the good one to check on operating mode */
      case QMI_PROXY_MODE_PREF_EVDO_ONLY:
      case QMI_PROXY_MODE_PREF_LTE_ONLY:
        qmi_proxy_internal_info.svlte_oprt_mode = remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode;
        break;

      default:
        /* Not in emergency callback mode, thermal emergency state, GW Limited/Full Service or in the middle of operating mode
           change, and both modems are in sync on the operating mode */
        if ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode ==
             remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode )
        {
          qmi_proxy_internal_info.svlte_oprt_mode = local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode;
        }
    }
  }

  QMI_PROXY_DEBUG_MSG( "SVLTE mode pref = %s(%d), SVLTE oprt mode = %s(%d)\n",
                       qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.svlte_mode_pref ],
                       qmi_proxy_internal_info.svlte_mode_pref,
                       qmi_proxy_lookup_oprt_mode_name( qmi_proxy_internal_info.svlte_oprt_mode ),
                       qmi_proxy_internal_info.svlte_oprt_mode );

  return qmi_proxy_internal_info.svlte_oprt_mode;

} /* qmi_proxy_get_cache_svlte_operating_mode */

/*=========================================================================
  FUNCTION:  qmi_proxy_get_cache_service_ready_state

===========================================================================*/
/*!
    @brief
    Get cached SGLTE service ready state

    @return
    SGLTE operating mode
*/
/*=========================================================================*/
static unsigned int qmi_proxy_get_cache_service_ready_state
(
  void
)
{
  wms_service_ready_status_enum_v01 local_service_ready = qmi_proxy_internal_info.service_ready[QMI_PROXY_LOCAL_CONN_TYPE];
  wms_service_ready_status_enum_v01 remote_service_ready = qmi_proxy_internal_info.service_ready[QMI_PROXY_REMOTE_CONN_TYPE];
  wms_service_ready_status_enum_v01 service_ready;

  service_ready = local_service_ready;

  switch (local_service_ready)
  {
    case WMS_SERVICE_READY_STATUS_3GPP_V01:
    {
      switch (remote_service_ready)
      {
        case WMS_SERVICE_READY_STATUS_3GPP2_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01:
        {
          service_ready = WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01;
          break;
        }
        default:
          break;
      }
      break;
    }
    case WMS_SERVICE_READY_STATUS_3GPP2_V01:
    {
      switch (remote_service_ready)
      {
        case WMS_SERVICE_READY_STATUS_3GPP_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01:
        {
          service_ready = WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01;
          break;
        }
        default:
          break;
      }
      break;
    }
    case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01:
    {
      break;
    }
    default:
    {
      service_ready = remote_service_ready;
      break;
    }
  }

  QMI_PROXY_DEBUG_MSG( "SGLTE service ready status = %d, local service ready status %d, remote service reasy state %d\n",
                       qmi_proxy_internal_info.service_ready[QMI_PROXY_LOCAL_CONN_TYPE],
                       qmi_proxy_internal_info.service_ready[QMI_PROXY_REMOTE_CONN_TYPE],
                       service_ready);

  return service_ready;

} /* qmi_proxy_get_cache_service_ready_state */

/*=========================================================================
  FUNCTION:  qmi_proxy_get_cache_sglte_operating_mode

===========================================================================*/
/*!
    @brief
    Get cached SGLTE operating mode

    @return
    SGLTE operating mode
*/
/*=========================================================================*/
static uint8_t qmi_proxy_get_cache_sglte_operating_mode
(
  void
)
{
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  int mode_online;
  int mode_shutting_down;

  /*-----------------------------------------------------------------------*/

  /* Any modem in FTM mode means FTM */
  if ( ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) ||
       ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) )
  {
    qmi_proxy_internal_info.svlte_oprt_mode = DMS_OP_MODE_FACTORY_TEST_MODE_V01;
  }
  /* Commenting out the following piece of code, as SHUTTING_DOWN is considered as valid case during power on,
     removing check that shutting down of either modem should be considered as overall svlte2 shutting down,
     giving priority to operating mode preference then considering "Any modem in SHUTTING_DOWN mode means SHUTTING_DOWN" */
  /*else if ( ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_SHUTTING_DOWN_V01 ) ||
            ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_SHUTTING_DOWN_V01 ) )
  {
    qmi_proxy_internal_info.svlte_oprt_mode = DMS_OP_MODE_SHUTTING_DOWN_V01;
  }*/
  /* Any modem in RESETTING mode means RESETTING */
  else if ( ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_RESETTING_V01 ) ||
            ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_RESETTING_V01 ) )
  {
    qmi_proxy_internal_info.svlte_oprt_mode = DMS_OP_MODE_RESETTING_V01;
  }
  /* Any modem in OFFLINE mode means OFFLINE */
  else if ( ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_OFFLINE_V01 ) ||
            ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_OFFLINE_V01 ) )
  {
    qmi_proxy_internal_info.svlte_oprt_mode = DMS_OP_MODE_OFFLINE_V01;
  }
  else
  {
    QMI_PROXY_DEBUG_MSG("%s: remote cs active? %d. sglte coverage? %d\n",
                        __FUNCTION__,
                        QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE(),
                        qmi_proxy_internal_info.is_in_sglte_coverage);
    mode_online = FALSE;
    mode_shutting_down = FALSE;

    if ( QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() ||
         QMI_PROXY_SGLTE_REMOTE_MODEM_PS_ACTIVE())
    {
      mode_online = remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode ==
              DMS_OP_MODE_ONLINE_V01;
      mode_shutting_down = remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode ==
              DMS_OP_MODE_SHUTTING_DOWN_V01;
    }

    if ( QMI_PROXY_SGLTE_LOCAL_MODEM_CS_ACTIVE() ||
         QMI_PROXY_SGLTE_LOCAL_MODEM_PS_ACTIVE())
    {
      mode_online = ( mode_online ||
              local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_ONLINE_V01 );
      mode_shutting_down = (mode_shutting_down ||
              local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode ==
              DMS_OP_MODE_SHUTTING_DOWN_V01);
    }

    if (mode_shutting_down)
    {
      qmi_proxy_internal_info.svlte_oprt_mode = DMS_OP_MODE_SHUTTING_DOWN_V01;
    }
    else if (mode_online)
    {
      qmi_proxy_internal_info.svlte_oprt_mode = DMS_OP_MODE_ONLINE_V01;
    }
    else
    {
      qmi_proxy_internal_info.svlte_oprt_mode = DMS_OP_MODE_LOW_POWER_V01;
    }


  }

  QMI_PROXY_DEBUG_MSG( "SGLTE mode pref = %s(%d), SGLTE oprt mode = %s(%d)\n",
                       qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.sglte_user_mode_pref ],
                       qmi_proxy_internal_info.sglte_user_mode_pref,
                       qmi_proxy_lookup_oprt_mode_name( qmi_proxy_internal_info.svlte_oprt_mode ),
                       qmi_proxy_internal_info.svlte_oprt_mode );

  return qmi_proxy_internal_info.svlte_oprt_mode;

} /* qmi_proxy_get_cache_sglte_operating_mode */


/*=========================================================================
  FUNCTION:  qmi_proxy_dms_set_evt_rpt_async_cb

===========================================================================*/
/*!
    @brief
    Handle the asychronous callback for setting event report.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_dms_set_evt_rpt_async_cb
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  void                           *resp_cb_data,
  qmi_client_error_type          transp_err
)
{
  qmi_proxy_conn_type conn_type;
  dms_set_event_report_resp_msg_v01 *dms_set_evt_rpt_resp = NULL;
  int rc = transp_err;

  /* ------------------------------------------------------------------- */

  (void) user_handle;
  (void) msg_id;
  (void) resp_c_struct_len;

  conn_type = ( qmi_proxy_conn_type ) resp_cb_data;

  dms_set_evt_rpt_resp = ( dms_set_event_report_resp_msg_v01 * ) resp_c_struct;
  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, dms_set_evt_rpt_resp ) )
  {
    QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) success to set DMS event report\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type );
  }
  else
  {
    QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) failed to set DMS event report, rc=%d\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type, rc );
  }

  /* Free buffer */
  qmi_proxy_free( (void **)&resp_c_struct );

} /* qmi_proxy_dms_set_evt_rpt_async_cb */


/*=========================================================================
  FUNCTION:  qmi_proxy_sync_cap_and_oprt_mode

===========================================================================*/
/*!
    @brief
    Query modem's capability, operating mode and enable the unsoliciated
    reporting on modem's operating mode change.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_sync_cap_and_oprt_mode
(
  qmi_proxy_conn_type conn_type
)
{
  qmi_client_type client_handle = NULL;
  dms_set_event_report_req_msg_v01 *dms_set_evt_rpt_req = NULL;
  dms_set_event_report_resp_msg_v01 *dms_set_evt_rpt_resp = NULL;
  dms_get_device_cap_resp_msg_v01 *dms_get_dev_cap_resp = NULL;
  dms_get_operating_mode_resp_msg_v01 *dms_get_oprt_mode_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  uint32 user_data = 0;
  qmi_txn_handle txn_handle = 0;
  qmi_client_error_type rc = QMI_NO_ERR;

  /* ------------------------------------------------------------------- */

  /* Sanity check to make sure QMI Proxy Internal registered QMI DMS Service on the modem */
  if ( !qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_DMS_SERVICE ].registered )
  {
    QMI_PROXY_ERR_MSG( "QPI: %s(%d) not registered for QMI DMS\n",
                       qmi_proxy_modem_name[ conn_type ], conn_type );

    return QMI_INTERNAL_ERR;
  }
  else
  {
    /* Retrieve the client handle */
    client_handle = qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_DMS_SERVICE ].client_handle;
  }

  /* Allocate buffers */
  dms_set_evt_rpt_req = qmi_proxy_malloc( sizeof( dms_set_event_report_req_msg_v01 ) );
  if ( dms_set_evt_rpt_req == NULL )
  {
    rc = QMI_INTERNAL_ERR;
  }
  else
  {
    dms_set_evt_rpt_resp = qmi_proxy_malloc( sizeof( dms_set_event_report_resp_msg_v01 ) );
    if ( dms_set_evt_rpt_resp == NULL )
    {
      qmi_proxy_free( (void **)&dms_set_evt_rpt_req );
      rc = QMI_INTERNAL_ERR;
    }
    else
    {
      dms_get_dev_cap_resp = qmi_proxy_malloc( sizeof( dms_get_device_cap_resp_msg_v01 ) );
      if ( dms_get_dev_cap_resp == NULL )
      {
        qmi_proxy_free( (void **)&dms_set_evt_rpt_req );
        qmi_proxy_free( (void **)&dms_set_evt_rpt_resp );
        rc = QMI_INTERNAL_ERR;
      }
      else
      {
        dms_get_oprt_mode_resp = qmi_proxy_malloc( sizeof( dms_get_operating_mode_resp_msg_v01 ) );
        if ( dms_get_oprt_mode_resp == NULL )
        {
          qmi_proxy_free( (void **)&dms_set_evt_rpt_req );
          qmi_proxy_free( (void **)&dms_set_evt_rpt_resp );
          qmi_proxy_free( (void **)&dms_get_oprt_mode_resp );
          rc = QMI_INTERNAL_ERR;
        }
      }
    }
  }

  /* Enable unsolicited indication on operating mode change for SVLTE */
  if ( rc == QMI_NO_ERR )
  {
    /* Only interested in operating mode change reporting */
    dms_set_evt_rpt_req->report_oprt_mode_state_valid = TRUE;
    dms_set_evt_rpt_req->report_oprt_mode_state = TRUE;

    user_data = conn_type;
    rc = qmi_client_send_msg_async( client_handle,
                                    QMI_DMS_SET_EVENT_REPORT_REQ_V01,
                                    (void *) dms_set_evt_rpt_req,
                                    sizeof( dms_set_event_report_req_msg_v01 ),
                                    (void *) dms_set_evt_rpt_resp,
                                    sizeof( dms_set_event_report_resp_msg_v01 ),
                                    qmi_proxy_dms_set_evt_rpt_async_cb,
                                    (void *)(intptr_t) user_data,
                                    &txn_handle );
    if ( rc != QMI_NO_ERR )
    {
      QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) failed to enable QMI DMS to report oprt mode change, rc=%d\n",
                           qmi_proxy_modem_name[ conn_type ], conn_type, rc );

      qmi_proxy_free( (void **)&dms_set_evt_rpt_resp );
    }
  }

  /* Sync modem's capability */
  if ( rc == QMI_NO_ERR )
  {
    rc = qmi_client_send_msg_sync( client_handle,
                                   QMI_DMS_GET_DEVICE_CAP_REQ_V01,
                                   NULL,
                                   0,
                                   (void *) dms_get_dev_cap_resp,
                                   sizeof( dms_get_device_cap_resp_msg_v01 ),
                                   QMI_PROXY_SYNC_REQ_TIMEOUT );
    if ( !QMI_PROXY_RESPONSE_SUCCESS( rc, dms_get_dev_cap_resp ) )
    {
      QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) failed to query QMI DMS for device capability, rc=%d\n",
                           qmi_proxy_modem_name[ conn_type ], conn_type, rc );
    }
    else
    {
      svlte_cache_ptr->dms_get_dev_cap_resp = *dms_get_dev_cap_resp;

      /* Set SVLTE Radio IF mask */
      qmi_proxy_set_cache_svlte_radio_if_mask();
      QMI_PROXY_DEBUG_MSG( "QPI: SVLTE Radio IF = 0x%x\n", qmi_proxy_internal_info.svlte_radio_if_mask );
    }
  }

  /* Sync modem's operating mode */
  if ( rc == QMI_NO_ERR )
  {
    rc = qmi_client_send_msg_sync( client_handle,
                                   QMI_DMS_GET_OPERATING_MODE_REQ_V01,
                                   NULL,
                                   0,
                                   (void *) dms_get_oprt_mode_resp,
                                   sizeof( dms_get_operating_mode_resp_msg_v01 ),
                                   QMI_PROXY_SYNC_REQ_TIMEOUT );
    if ( !QMI_PROXY_RESPONSE_SUCCESS( rc, dms_get_oprt_mode_resp ) )
    {
      QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) failed to query QMI DMS for oprt mode, rc=%d\n",
                           qmi_proxy_modem_name[ conn_type ], conn_type, rc );
    }
    else
    {
      svlte_cache_ptr->dms_get_oprt_mode_resp = *dms_get_oprt_mode_resp;
      QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) oprt mode = %s(%d)\n",
                           qmi_proxy_modem_name[ conn_type ], conn_type,
                           qmi_proxy_lookup_oprt_mode_name( svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode ),
                           svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode );

      if ( QMI_PROXY_SGLTE_TARGET() )
      {
        /* Set SGLTE operating mode */
        qmi_proxy_internal_info.svlte_oprt_mode = qmi_proxy_get_cache_sglte_operating_mode();
      }
      else
      {
      /* Set SVLTE operating mode */
      qmi_proxy_internal_info.svlte_oprt_mode = qmi_proxy_get_cache_svlte_operating_mode();
    }
  }
  }
  else
  {
    /* Default SVLTE operating mode */
    qmi_proxy_internal_info.svlte_oprt_mode = DMS_OP_MODE_SHUTTING_DOWN_V01;
  }

  /* Free buffers */
  qmi_proxy_free( (void **)&dms_set_evt_rpt_req );
  qmi_proxy_free( (void **)&dms_get_dev_cap_resp );
  qmi_proxy_free( (void **)&dms_get_oprt_mode_resp );

  return rc;

} /* qmi_proxy_sync_cap_and_oprt_mode */


/*=========================================================================
  FUNCTION:  qmi_proxy_dms_set_oprt_mode_async_cb

===========================================================================*/
/*!
    @brief
    Handle the asychronous callback for setting operating mode.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_dms_set_oprt_mode_async_cb
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  void                           *resp_cb_data,
  qmi_client_error_type          transp_err
)
{
  qmi_proxy_conn_type conn_type;
  dms_operating_mode_enum_v01 oprt_mode;
  dms_set_operating_mode_resp_msg_v01 *dms_set_oprt_mode_resp = NULL;
  uint32 user_data = 0;
  int rc = transp_err;

  /* ------------------------------------------------------------------- */

  (void) user_handle;
  (void) msg_id;
  (void) resp_c_struct_len;

  user_data = (intptr_t) resp_cb_data;

  conn_type = QMI_PROXY_DMS_OPRT_MODE_ASYNCB_USER_DATA_TO_CONN_TYPE( user_data );

  oprt_mode = QMI_PROXY_DMS_OPRT_MODE_ASYNCB_USER_DATA_TO_OPRT_MODE( user_data );

  dms_set_oprt_mode_resp = ( dms_set_operating_mode_resp_msg_v01 * ) resp_c_struct;
  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, dms_set_oprt_mode_resp ) )
  {
    QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) success to set oprt mode = %s(%d)\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type,
                         qmi_proxy_lookup_oprt_mode_name( oprt_mode ), oprt_mode );
  }
  else
  {
    QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) failed to set oprt mode = %s(%d), rc = %d, ext = %d\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type,
                         qmi_proxy_lookup_oprt_mode_name( oprt_mode ), oprt_mode,
                         rc,
                         (int)dms_set_oprt_mode_resp->resp.error );
  }

  /* Free buffer */
  qmi_proxy_free( (void **)&resp_c_struct );

} /* qmi_proxy_dms_set_oprt_mode_async_cb */


/*=========================================================================
  FUNCTION:  qmi_proxy_force_lpm_sync

===========================================================================*/
/*!
    @brief
    Put modem to LPM if it is not currently in FTM and LPM.

    @return
    none
*/
/*=========================================================================*/
int qmi_proxy_force_lpm_sync
(
  qmi_proxy_conn_type conn_type,
  int timeout_msecs
)
{
  qmi_client_type client_handle = NULL;
  dms_set_operating_mode_req_msg_v01 *dms_set_oprt_mode_req = NULL;
  dms_set_operating_mode_resp_msg_v01 *dms_set_oprt_mode_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];;
  uint32 user_data = 0;
  qmi_txn_handle txn_handle = 0;
  int rc = QMI_NO_ERR;

  /* ------------------------------------------------------------------- */

  if ( !qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_DMS_SERVICE ].registered )
  {
    QMI_PROXY_ERR_MSG( "QPI: %s(%d) not registered for QMI DMS\n",
                       qmi_proxy_modem_name[ conn_type ], conn_type );

    return QMI_INTERNAL_ERR;
  }
  else
  {
    /* Retrieve client handle */
    client_handle = qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_DMS_SERVICE ].client_handle;
  }

  /* Allocate buffers */
  dms_set_oprt_mode_req = qmi_proxy_malloc( sizeof( dms_set_operating_mode_req_msg_v01 ) );
  if ( dms_set_oprt_mode_req == NULL )
  {
    rc = QMI_INTERNAL_ERR;
  }
  else
  {
    dms_set_oprt_mode_resp = qmi_proxy_malloc( sizeof( dms_set_operating_mode_resp_msg_v01 ) );
    if ( dms_set_oprt_mode_resp == NULL )
    {
      rc = QMI_INTERNAL_ERR;
    }
  }

  /* Change to LPM if not in FTM */
  if ( ( rc == QMI_NO_ERR ) &&
       ( ( svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode != DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) &&
         ( svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode != DMS_OP_MODE_LOW_POWER_V01 ) ) )
  {
    dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_LOW_POWER_V01;
    user_data = QMI_PROXY_COMPOSE_DMS_OPRT_MODE_ASYNCB_USER_DATA( conn_type, dms_set_oprt_mode_req->operating_mode );
    rc = qmi_client_send_msg_sync( client_handle,
                                    QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                                    (void*) dms_set_oprt_mode_req,
                                    sizeof( dms_set_operating_mode_req_msg_v01 ),
                                    (void*) dms_set_oprt_mode_resp,
                                    sizeof( dms_set_operating_mode_resp_msg_v01 ),
                                    timeout_msecs );
    if ( rc == QMI_NO_ERR )
    {
      QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) to LPM\n", qmi_proxy_modem_name[ conn_type ], conn_type );
      svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode = DMS_OP_MODE_LOW_POWER_V01;
    }
    else
    {
      QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) failed to LPM, rc=%d\n",
                           qmi_proxy_modem_name[ conn_type ], conn_type, rc );

      qmi_proxy_free( (void **)&dms_set_oprt_mode_resp );
    }
  }
  else
  {
    qmi_proxy_free( (void **)&dms_set_oprt_mode_resp );
  }

  /* Free buffer */
  qmi_proxy_free( (void **)&dms_set_oprt_mode_req );

  return rc;

} /* qmi_proxy_force_lpm_sync */


/*=========================================================================
  FUNCTION:  qmi_proxy_force_lpm

===========================================================================*/
/*!
    @brief
    Put modem to LPM if it is not currently in FTM and LPM.

    @return
    none
*/
/*=========================================================================*/
static int qmi_proxy_force_lpm
(
  qmi_proxy_conn_type conn_type
)
{
  qmi_client_type client_handle = NULL;
  dms_set_operating_mode_req_msg_v01 *dms_set_oprt_mode_req = NULL;
  dms_set_operating_mode_resp_msg_v01 *dms_set_oprt_mode_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];;
  uint32 user_data = 0;
  qmi_txn_handle txn_handle = 0;
  int rc = QMI_NO_ERR;

  /* ------------------------------------------------------------------- */

  if ( !qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_DMS_SERVICE ].registered )
  {
    QMI_PROXY_ERR_MSG( "QPI: %s(%d) not registered for QMI DMS\n",
                       qmi_proxy_modem_name[ conn_type ], conn_type );

    return QMI_INTERNAL_ERR;
  }
  else
  {
    /* Retrieve client handle */
    client_handle = qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_DMS_SERVICE ].client_handle;
  }

  /* Allocate buffers */
  dms_set_oprt_mode_req = qmi_proxy_malloc( sizeof( dms_set_operating_mode_req_msg_v01 ) );
  if ( dms_set_oprt_mode_req == NULL )
  {
    rc = QMI_INTERNAL_ERR;
  }
  else
  {
    dms_set_oprt_mode_resp = qmi_proxy_malloc( sizeof( dms_set_operating_mode_resp_msg_v01 ) );
    if ( dms_set_oprt_mode_resp == NULL )
    {
      rc = QMI_INTERNAL_ERR;
    }
  }

  /* Change to LPM if not in FTM */
  if ( ( rc == QMI_NO_ERR ) &&
       ( ( svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode != DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) &&
         ( svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode != DMS_OP_MODE_LOW_POWER_V01 ) ) )
  {
    dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_LOW_POWER_V01;
    user_data = QMI_PROXY_COMPOSE_DMS_OPRT_MODE_ASYNCB_USER_DATA( conn_type, dms_set_oprt_mode_req->operating_mode );
    rc = qmi_client_send_msg_async( client_handle,
                                    QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                                    (void*) dms_set_oprt_mode_req,
                                    sizeof( dms_set_operating_mode_req_msg_v01 ),
                                    (void*) dms_set_oprt_mode_resp,
                                    sizeof( dms_set_operating_mode_resp_msg_v01 ),
                                    qmi_proxy_dms_set_oprt_mode_async_cb,
                                    (void *)(intptr_t) user_data,
                                    &txn_handle );
    if ( rc == QMI_NO_ERR )
    {
      QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) to LPM\n", qmi_proxy_modem_name[ conn_type ], conn_type );
    }
    else
    {
      QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) failed to LPM, rc=%d\n",
                           qmi_proxy_modem_name[ conn_type ], conn_type, rc );

      qmi_proxy_free( (void **)&dms_set_oprt_mode_resp );
    }
  }
  else
  {
    qmi_proxy_free( (void **)&dms_set_oprt_mode_resp );
  }

  /* Free buffer */
  qmi_proxy_free( (void **)&dms_set_oprt_mode_req );

  return rc;

} /* qmi_proxy_force_lpm */


/*=========================================================================
  FUNCTION:  qmi_proxy_force_online_sync

===========================================================================*/
/*!
    @brief
    Synchronously put modem to Online if it is not currently in FTM and Online.

    @return
    none
*/
/*=========================================================================*/
static int qmi_proxy_force_online_sync
(
  qmi_proxy_conn_type conn_type,
  int timeout_msecs
)
{
  qmi_client_type client_handle = NULL;
  dms_set_operating_mode_req_msg_v01 *dms_set_oprt_mode_req = NULL;
  dms_set_operating_mode_resp_msg_v01 *dms_set_oprt_mode_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];;
  uint32 user_data = 0;
  qmi_txn_handle txn_handle;
  int rc = QMI_NO_ERR;

  /* ------------------------------------------------------------------- */

  QMI_PROXY_DEBUG_MSG("%s conn_type: %d. timeout: %d\n", __FUNCTION__, conn_type, timeout_msecs);
  if ( !qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_DMS_SERVICE ].registered )
  {
    QMI_PROXY_ERR_MSG( "QPI: %s(%d) not registered for QMI DMS\n",
                       qmi_proxy_modem_name[ conn_type ], conn_type );

    return QMI_INTERNAL_ERR;
  }
  else
  {
    /* Retrieve client handle */
    client_handle = qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_DMS_SERVICE ].client_handle;
  }

  /* Allocate buffers */
  dms_set_oprt_mode_req = qmi_proxy_malloc( sizeof( dms_set_operating_mode_req_msg_v01 ) );
  if ( dms_set_oprt_mode_req == NULL )
  {
    rc = QMI_INTERNAL_ERR;
  }
  else
  {
    dms_set_oprt_mode_resp = qmi_proxy_malloc( sizeof( dms_set_operating_mode_resp_msg_v01 ) );
    if ( dms_set_oprt_mode_resp == NULL )
    {
      rc = QMI_INTERNAL_ERR;
    }
  }

  /* Change to Online if not in FTM */
  if ( ( rc == QMI_NO_ERR ) && ( svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode != DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) &&
       ( svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode != DMS_OP_MODE_ONLINE_V01 ) )
  {
    dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_ONLINE_V01;
    user_data = QMI_PROXY_COMPOSE_DMS_OPRT_MODE_ASYNCB_USER_DATA( conn_type, dms_set_oprt_mode_req->operating_mode );
    rc = qmi_client_send_msg_sync( client_handle,
                                    QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                                    (void*) dms_set_oprt_mode_req,
                                    sizeof( dms_set_operating_mode_req_msg_v01 ),
                                    (void*) dms_set_oprt_mode_resp,
                                    sizeof( dms_set_operating_mode_resp_msg_v01 ),
                                    timeout_msecs );

    if ( rc == QMI_NO_ERR )
    {
      QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) to Online\n", qmi_proxy_modem_name[ conn_type ], conn_type );
      svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode = DMS_OP_MODE_ONLINE_V01;
    }
    else
    {
      QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) failed to Online, rc=%d\n",
                           qmi_proxy_modem_name[ conn_type ], conn_type, rc );

      qmi_proxy_free( (void **)&dms_set_oprt_mode_resp );
    }
  }
  else
  {
    /* Free buffer */
    qmi_proxy_free( (void **)&dms_set_oprt_mode_resp );
  }

  /* Free buffers */
  qmi_proxy_free( (void **)&dms_set_oprt_mode_req );

  return rc;

} /* qmi_proxy_force_online_sync */


/*=========================================================================
  FUNCTION:  qmi_proxy_force_online

===========================================================================*/
/*!
    @brief
    Put modem to Online if it is not currently in FTM and Online.

    @return
    none
*/
/*=========================================================================*/
static int qmi_proxy_force_online
(
  qmi_proxy_conn_type conn_type
)
{
  qmi_client_type client_handle = NULL;
  dms_set_operating_mode_req_msg_v01 *dms_set_oprt_mode_req = NULL;
  dms_set_operating_mode_resp_msg_v01 *dms_set_oprt_mode_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];;
  uint32 user_data = 0;
  qmi_txn_handle txn_handle;
  int rc = QMI_NO_ERR;

  /* ------------------------------------------------------------------- */

  if ( !qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_DMS_SERVICE ].registered )
  {
    QMI_PROXY_ERR_MSG( "QPI: %s(%d) not registered for QMI DMS\n",
                       qmi_proxy_modem_name[ conn_type ], conn_type );

    return QMI_INTERNAL_ERR;
  }
  else
  {
    /* Retrieve client handle */
    client_handle = qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_DMS_SERVICE ].client_handle;
  }

  /* Allocate buffers */
  dms_set_oprt_mode_req = qmi_proxy_malloc( sizeof( dms_set_operating_mode_req_msg_v01 ) );
  if ( dms_set_oprt_mode_req == NULL )
  {
    rc = QMI_INTERNAL_ERR;
  }
  else
  {
    dms_set_oprt_mode_resp = qmi_proxy_malloc( sizeof( dms_set_operating_mode_resp_msg_v01 ) );
    if ( dms_set_oprt_mode_resp == NULL )
    {
      rc = QMI_INTERNAL_ERR;
    }
  }

  /* Change to Online if not in FTM */
  if ( ( rc == QMI_NO_ERR ) && ( svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode != DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) &&
       ( svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode != DMS_OP_MODE_ONLINE_V01 ) )
  {
    dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_ONLINE_V01;
    user_data = QMI_PROXY_COMPOSE_DMS_OPRT_MODE_ASYNCB_USER_DATA( conn_type, dms_set_oprt_mode_req->operating_mode );
    rc = qmi_client_send_msg_async( client_handle,
                                    QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                                    (void*) dms_set_oprt_mode_req,
                                    sizeof( dms_set_operating_mode_req_msg_v01 ),
                                    (void*) dms_set_oprt_mode_resp,
                                    sizeof( dms_set_operating_mode_resp_msg_v01 ),
                                    qmi_proxy_dms_set_oprt_mode_async_cb,
                                    (void *)(intptr_t) user_data,
                                    &txn_handle );

    if ( rc == QMI_NO_ERR )
    {
      QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) to Online\n", qmi_proxy_modem_name[ conn_type ], conn_type );
    }
    else
    {
      QMI_PROXY_DEBUG_MSG( "QPI: %s(%d) failed to Online, rc=%d\n",
                           qmi_proxy_modem_name[ conn_type ], conn_type, rc );

      qmi_proxy_free( (void **)&dms_set_oprt_mode_resp );
    }
  }
  else
  {
    /* Free buffer */
    qmi_proxy_free( (void **)&dms_set_oprt_mode_resp );
  }

  /* Free buffers */
  qmi_proxy_free( (void **)&dms_set_oprt_mode_req );

  return rc;

} /* qmi_proxy_force_online */


/*=========================================================================
  FUNCTION:  qmi_proxy_force_sys_sel_pref_sync

===========================================================================*/
/*!
    @brief
    Send system selection pref request to the modem

    @return
    QMI_NO_ERR if successful
*/
/*=========================================================================*/
static int qmi_proxy_force_sys_sel_pref_sync
(
  qmi_proxy_conn_type conn_type,
  nas_set_system_selection_preference_req_msg_v01 *request,
  nas_set_system_selection_preference_resp_msg_v01 *response,
  int timeout_msecs
)
{
  qmi_client_type client_handle = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  uint32 user_data = 0;
  qmi_txn_handle txn_handle;
  int rc = QMI_NO_ERR;

  /* ------------------------------------------------------------------- */

  if ( request &&  response )
  {
    if ( qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_NAS_SERVICE ].registered )
    {
      /* Retrieve client handle */
      client_handle = qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_NAS_SERVICE ].client_handle;
      user_data = QMI_PROXY_COMPOSE_NAS_MODE_PREF_ASYNCB_USER_DATA( conn_type, request->mode_pref );

      rc = qmi_client_send_msg_sync( client_handle,
                                      QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                      (void *) request,
                                      sizeof( nas_set_system_selection_preference_req_msg_v01 ),
                                      (void *) response,
                                      sizeof( nas_set_system_selection_preference_resp_msg_v01 ),
                                      timeout_msecs );
    }
    else
    {
        QMI_PROXY_ERR_MSG( "QPI: %s(%d) not registered for QMI NAS\n",
                           qmi_proxy_modem_name[ conn_type ], conn_type );
        rc = QMI_INTERNAL_ERR;
    }

  }
  else
  {
    QMI_PROXY_ERR_MSG( "%s: Invalid parameters passed: %8.8x %8.8x", __FUNCTION__, request, response);
    rc = QMI_INTERNAL_ERR;
  }

  return rc;

} /* qmi_proxy_force_sys_sel_pref_sync */


/*=========================================================================
  FUNCTION:  qmi_proxy_force_sys_sel_pref

===========================================================================*/
/*!
    @brief
    Send system selection pref request to the modem

    @return
    QMI_NO_ERR if successful
*/
/*=========================================================================*/
static int qmi_proxy_force_sys_sel_pref
(
  qmi_proxy_conn_type conn_type,
  nas_set_system_selection_preference_req_msg_v01 *request,
  nas_set_system_selection_preference_resp_msg_v01 *response
)
{
  qmi_client_type client_handle = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  uint32 user_data = 0;
  qmi_txn_handle txn_handle;
  int rc = QMI_NO_ERR;

  /* ------------------------------------------------------------------- */

  if ( request &&  response )
  {
    if ( qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_NAS_SERVICE ].registered )
    {
      /* Retrieve client handle */
      client_handle = qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_NAS_SERVICE ].client_handle;
      user_data = QMI_PROXY_COMPOSE_NAS_MODE_PREF_ASYNCB_USER_DATA( conn_type, request->mode_pref );

      rc = qmi_client_send_msg_async( client_handle,
                                      QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                      (void *) request,
                                      sizeof( nas_set_system_selection_preference_req_msg_v01 ),
                                      (void *) response,
                                      sizeof( nas_set_system_selection_preference_resp_msg_v01 ),
                                      qmi_proxy_nas_set_sys_sel_pref_async_cb,
                                      (void *)(intptr_t) user_data,
                                      &txn_handle );
    }
    else
    {
        QMI_PROXY_ERR_MSG( "QPI: %s(%d) not registered for QMI NAS\n",
                           qmi_proxy_modem_name[ conn_type ], conn_type );
        rc = QMI_INTERNAL_ERR;
    }

  }
  else
  {
    QMI_PROXY_ERR_MSG( "%s: Invalid parameters passed: %8.8x %8.8x", __FUNCTION__, request, response);
    rc = QMI_INTERNAL_ERR;
  }

  return rc;

} /* qmi_proxy_force_sys_sel_pref */

/*=========================================================================
 FUNCTION:  qmi_proxy_get_sys_sel_pref_sync

===========================================================================*/
/*!
    @brief
    Send get system selection pref request to the modem

    @return
    QMI_NO_ERR if successful
*/
/*=========================================================================*/
static int qmi_proxy_get_sys_sel_pref_sync
(
  qmi_proxy_conn_type conn_type,
  nas_get_system_selection_preference_resp_msg_v01 *response,
  int timeout_msecs
)
{
  qmi_client_type client_handle = NULL;
  int rc = QMI_NO_ERR;

  /* ------------------------------------------------------------------- */

  if ( response )
  {
    if ( qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_NAS_SERVICE ].registered )
    {
      /* Retrieve client handle */
      client_handle = qmi_proxy_internal_info.srvc_info[ conn_type ][ QMI_NAS_SERVICE ].client_handle;

      rc = qmi_client_send_msg_sync( client_handle,
                                      QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                      NULL,
                                      0,  // empty request payload
                                      (void *) response,
                                      sizeof( nas_get_system_selection_preference_resp_msg_v01 ),
                                      timeout_msecs );
    }
    else
    {
        QMI_PROXY_ERR_MSG( "QPI: %s(%d) not registered for QMI NAS\n",
                           qmi_proxy_modem_name[ conn_type ], conn_type );
        rc = QMI_INTERNAL_ERR;
    }

  }
  else
  {
    QMI_PROXY_ERR_MSG( "Invalid parameters passed: %8.8x", response);
    rc = QMI_INTERNAL_ERR;
  }

  return rc;

} /* qmi_proxy_get_sys_sel_pref_sync */

/*=========================================================================
 FUNCTION:  qmi_proxy_get_sys_sel_pref_srv_reg_restriction

===========================================================================*/
/*!
    @brief
    Query the system selection pref from the specified modem and return the
    Service Registration Restriction

    @return
    QMI_NO_ERR if successful
*/
/*=========================================================================*/
static int qmi_proxy_get_sys_sel_pref_srv_reg_restriction
(
  qmi_proxy_conn_type conn_type,
  nas_srv_reg_restriction_enum_v01 *srv_reg_restriction
)
{
  int rc = QMI_INTERNAL_ERR;
  int timeout_msecs = QMI_PROXY_SYNC_REQ_TIMEOUT;
  nas_get_system_selection_preference_resp_msg_v01 *response = NULL;

  response = qmi_proxy_malloc( sizeof(nas_get_system_selection_preference_resp_msg_v01) );
  if ( response && srv_reg_restriction )
  {
    if ( qmi_proxy_get_sys_sel_pref_sync(conn_type, response, timeout_msecs) == QMI_NO_ERR &&
            response->srv_reg_restriction_valid )
    {
      *srv_reg_restriction = response->srv_reg_restriction;
      rc = QMI_NO_ERR;
    }
  }
  else
  {
    QMI_PROXY_ERR_MSG( "Invalid parameters passed: %8.8x, %8.8x",
                       response, srv_reg_restriction);
  }

  return rc;
} /* qmi_proxy_get_sys_sel_pref_srv_reg_restriction */

/*=========================================================================
 FUNCTION:  qmi_proxy_correct_sys_sel_pref_srv_reg_restriction

===========================================================================*/
/*!
    @brief
    Set the Service Registration Restriction option to FLASE for the Local
    modem and Remote modem requests if it is already set as UNRESTRICTED

    @return
    QMI_NO_ERR if successful
*/
/*=========================================================================*/
static int qmi_proxy_correct_sys_sel_pref_srv_reg_restriction
(
  nas_set_system_selection_preference_req_msg_v01 *local_nas_set_sys_sel_pref_req,
  nas_set_system_selection_preference_req_msg_v01 *remote_nas_set_sys_sel_pref_req
)
{
    int rc = QMI_INTERNAL_ERR;
    nas_srv_reg_restriction_enum_v01 srv_reg_restriction;

    if ( local_nas_set_sys_sel_pref_req && remote_nas_set_sys_sel_pref_req )
    {
        rc = qmi_proxy_get_sys_sel_pref_srv_reg_restriction(QMI_PROXY_LOCAL_CONN_TYPE, &srv_reg_restriction);
        if ( rc == QMI_NO_ERR &&
                srv_reg_restriction == NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01 &&
                local_nas_set_sys_sel_pref_req->srv_reg_restriction_valid &&
                local_nas_set_sys_sel_pref_req->srv_reg_restriction == NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01 )
        {
          local_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = FALSE;
          QMI_PROXY_DEBUG_MSG( "%s", "srv reg restriction is already unrestricted on Local Modem" );
        }
        rc = qmi_proxy_get_sys_sel_pref_srv_reg_restriction(QMI_PROXY_REMOTE_CONN_TYPE, &srv_reg_restriction);
        if ( rc == QMI_NO_ERR &&
                srv_reg_restriction == NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01 &&
                remote_nas_set_sys_sel_pref_req->srv_reg_restriction_valid &&
                remote_nas_set_sys_sel_pref_req->srv_reg_restriction == NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01 )
        {
          remote_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = FALSE;
          QMI_PROXY_DEBUG_MSG( "%s", "srv reg restriction is already unrestricted on Remote Modem" );
        }
    }
    else
    {
      QMI_PROXY_ERR_MSG( "Invalid parameters passed: %8.8x, %8.8x",
                         local_nas_set_sys_sel_pref_req, remote_nas_set_sys_sel_pref_req);
    }

    return rc;
} /* qmi_proxy_correct_sys_sel_pref_srv_reg_restriction */

/*=========================================================================
  FUNCTION:  qmi_proxy_internal_init

===========================================================================*/
/*!
    @brief
    Setup QMI Proxy Internal for handling Fusion requirements.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_internal_init
(
  void
)
{
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  qmi_client_error_type rc = QMI_NO_ERR;

  /* -------------------------------------------------------------------  */

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.init_mutex );

  if ( !qmi_proxy_internal_info.initialized )
  {
    if ( !qmi_proxy_sys_event_registered )
    {
      /* Register for QMI system event */
      qmi_proxy_init_handle = qmi_init( qmi_proxy_sys_event_rx_hdlr, NULL );
      qmi_proxy_sys_event_registered = TRUE;
    }

    if ( QMI_PROXY_SVLTE_TARGET() )
    {
      /* Register QMI services on the behalf of QMI Proxy to handle Fusion requirements */
      rc = qmi_proxy_setup_internal_qmi_handle();

      /* Query and enable system selection preference indication on local modem */
      rc = qmi_proxy_sync_sys_sel_pref( QMI_PROXY_LOCAL_CONN_TYPE );

      if ( rc == QMI_NO_ERR )
      {
        /* Query and enable system selection preference indication on remote modem */
        rc = qmi_proxy_sync_sys_sel_pref( QMI_PROXY_REMOTE_CONN_TYPE );
        if ( rc == QMI_NO_ERR )
        {
          /* Sync mode preference from modems */
          rc = qmi_proxy_sync_svlte_mode_pref();
          if ( rc == QMI_NO_ERR )
          {
            /* Query and enable operating mode indication on local modem */
            rc = qmi_proxy_sync_cap_and_oprt_mode( QMI_PROXY_LOCAL_CONN_TYPE );
            if ( rc == QMI_NO_ERR )
            {
              /* Query and enable operating mode indication on remote modem */
              rc = qmi_proxy_sync_cap_and_oprt_mode( QMI_PROXY_REMOTE_CONN_TYPE );
              if ( rc == QMI_NO_ERR )
              {
                /* Not in FTM */
                if ( ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode != DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) &&
                     ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode != DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) )
                {
                  /* Put local modem to LPM */
                  qmi_proxy_force_lpm( QMI_PROXY_LOCAL_CONN_TYPE );

                  /* Put remote modem to LPM */
                  qmi_proxy_force_lpm( QMI_PROXY_REMOTE_CONN_TYPE );
                }
              }
            }
          }
        }
      }
    }
    else if ( QMI_PROXY_SGLTE_TARGET() )
    {
      /* Register QMI services on the behalf of QMI Proxy to handle Fusion requirements */
      rc = qmi_proxy_setup_internal_qmi_handle();

      if ( rc == QMI_NO_ERR )
      {
        /* Query and enable system selection preference indication on local modem */
        rc = qmi_proxy_sync_sys_sel_pref( QMI_PROXY_LOCAL_CONN_TYPE );

        if ( rc == QMI_NO_ERR )
        {
          /* Query and enable system selection preference indication on remote modem */
          rc = qmi_proxy_sync_sys_sel_pref( QMI_PROXY_REMOTE_CONN_TYPE );
          if ( rc == QMI_NO_ERR )
          {
            /* Query and enable operating mode indication on local modem */
            rc = qmi_proxy_sync_cap_and_oprt_mode( QMI_PROXY_LOCAL_CONN_TYPE );
            if ( rc == QMI_NO_ERR )
            {
              /* Query and enable operating mode indication on remote modem */
              rc = qmi_proxy_sync_cap_and_oprt_mode( QMI_PROXY_REMOTE_CONN_TYPE );
              if ( rc == QMI_NO_ERR )
              {
                /* Not in FTM */
                if ( ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode != DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) &&
                     ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode != DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) )
                {
                  /* Put local modem to LPM */
                  qmi_proxy_force_lpm( QMI_PROXY_LOCAL_CONN_TYPE );

                  /* Put remote modem to LPM */
                  qmi_proxy_force_lpm( QMI_PROXY_REMOTE_CONN_TYPE );
                }
              }
            }
          }
        }
      }
    }

    if ( rc == QMI_NO_ERR )
    {
      QMI_PROXY_DEBUG_MSG( "%s\n", "QPI: init success\n" );
      qmi_proxy_internal_info.initialized = TRUE;
    }
    else
    {
      rc = QMI_INTERNAL_ERR;
      QMI_PROXY_ERR_MSG( "%s\n", "QPI: init fail\n" );
      qmi_proxy_release_internal_qmi_handle();
    }
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.init_mutex );

  return rc;

} /* qmi_proxy_internal_init */


/*===========================================================================
  FUNCTION  qmi_proxy_client_init
===========================================================================*/
/*!
    @brief
    Setup a QMI Proxy Client Connection

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_client_init
(
  qmi_client_id_type     *proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_service_id_type    proxy_srvc_id,
  int fd
)
{
  qmi_proxy_reg_info_type reg_info;
  qmi_proxy_client_info_type *client_ptr = NULL;
  qmi_proxy_conn_type conn_type;
  const char *conn_name = NULL;
  uint32 unsol_ind_cb_user_data = 0;
  qmi_proxy_srvc_info_type *srvc_ptr = NULL;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "%s ....... QMI %s(%d) \n",
                       __FUNCTION__, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id );

  /* Sanity check on input parameters */
  if ( proxy_client_id == NULL )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Invalid QMI Proxy client id pointer" );

    return QMI_INTERNAL_ERR;
  }

  /* Check whether QMI service is supported by QMI Proxy */
  if ( !QMI_PROXY_SRVC_IS_SUPPORTED( proxy_srvc_id ) )
  {
    QMI_PROXY_ERR_MSG( "QMI %s(%d) not supported by QMI Proxy\n",
                       qmi_proxy_srvc_name[ proxy_srvc_id ],
                       proxy_srvc_id );

    return QMI_SERVICE_ERR;
  }

  /* Initialize QMI Proxy internal to handle fusion requirements if necessary */
  rc = qmi_proxy_internal_init();
  if ( rc != QMI_NO_ERR )
  {
    return rc;
  }

  /* Lock QMI Proxy Client ID mutex */
  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_client_id_mutex );

  /* Assign QMI Proxy Client ID */
  *proxy_client_id = qmi_proxy_client_id;
  qmi_proxy_client_id = ( qmi_proxy_client_id + 1 ) % 255;
  if ( qmi_proxy_client_id == 0 )
  {
    qmi_proxy_client_id = 1;
  }

  /* Unlock QMI Proxy Client ID mutex */
  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_client_id_mutex );

  /* Allow a QMI Proxy client structure */
  client_ptr = qmi_proxy_alloc_client_data( *proxy_client_id, proxy_conn_id, proxy_srvc_id, fd );
  if ( client_ptr == NULL )
  {
    QMI_PROXY_ERR_MSG( "Fail to allocate client data for QMI Proxy Client ID %d, QMI %s(%d)\n",
                       *proxy_client_id,
                       qmi_proxy_srvc_name[ proxy_srvc_id ],
                       proxy_srvc_id );

    return QMI_CLIENT_ALLOC_FAILURE;
  }

  /* Register QMI service(s) for the QMI Proxy client per requirements of QMI Proxy registration info */

  /* Lookup QMI Proxy registration info */
  reg_info = qmi_proxy_reg_info_tbl[ qmi_proxy_cfg ][ proxy_srvc_id ];

  if ( QMI_PROXY_SVLTE_2_TARGET() && QMI_PROXY_VOIP_ENABLED() && (proxy_srvc_id == QMI_VOICE_SERVICE) )
  {
    reg_info.remote_srvc_required = TRUE;
    QMI_PROXY_DEBUG_MSG( "%s\n","registering for voice on remote modem as VOIP is enabled");
  }

  /* Register on local modem's QMI service */
  if ( reg_info.local_srvc_required )
  {
    conn_type = QMI_PROXY_LOCAL_CONN_TYPE;
    conn_name = QMI_PROXY_LOCAL_CONN_NAME;
    unsol_ind_cb_user_data = QMI_PROXY_COMPOSE_UNSOL_IND_CB_USER_DATA( conn_type,
                                                                       client_ptr->proxy_srvc_id,
                                                                       client_ptr->proxy_client_id,
                                                                       client_ptr->proxy_conn_id );
    srvc_ptr = &client_ptr->srvc_info[ conn_type ];
    rc = qmi_client_init( conn_name,
                          qmi_proxy_srvc_obj_tbl[ client_ptr->proxy_srvc_id ],
                          qmi_proxy_srvc_unsol_ind_cb,
                          (void *)(intptr_t) unsol_ind_cb_user_data,
                          &srvc_ptr->client_handle );
    if ( rc )
    {
      QMI_PROXY_ERR_MSG( "Fail to init QMI %s(%d) on local modem for QMI Proxy client ID %d, rc %d\n",
                         qmi_proxy_srvc_name[ client_ptr->proxy_srvc_id ],
                         client_ptr->proxy_srvc_id,
                         client_ptr->proxy_client_id,
                         rc );
    }
    else
    {
      srvc_ptr->registered = TRUE;
      QMI_PROXY_DEBUG_MSG( "QMI_Proxy Client ID %d, QMI %s(%d), registered on local modem\n",
                           client_ptr->proxy_client_id,
                           qmi_proxy_srvc_name[ client_ptr->proxy_srvc_id ],
                           client_ptr->proxy_srvc_id );
    }
  }

  /* Register on remote modem's QMI service */
  if ( ( rc == QMI_NO_ERR ) && reg_info.remote_srvc_required )
  {
    conn_type = QMI_PROXY_REMOTE_CONN_TYPE;
    conn_name = QMI_PROXY_REMOTE_CONN_NAME;
    unsol_ind_cb_user_data = QMI_PROXY_COMPOSE_UNSOL_IND_CB_USER_DATA( conn_type,
                                                                       client_ptr->proxy_srvc_id,
                                                                       client_ptr->proxy_client_id,
                                                                       client_ptr->proxy_conn_id );
    srvc_ptr = &client_ptr->srvc_info[ conn_type ];
    rc = qmi_client_init( conn_name,
                          qmi_proxy_srvc_obj_tbl[ client_ptr->proxy_srvc_id ],
                          qmi_proxy_srvc_unsol_ind_cb,
                          (void *) (intptr_t)unsol_ind_cb_user_data,
                          &srvc_ptr->client_handle );
    if ( rc )
    {
      QMI_PROXY_ERR_MSG( "Fail to init QMI %s(%d) on remote modem for QMI Proxy Client ID %d\n",
                         qmi_proxy_srvc_name[ client_ptr->proxy_srvc_id ],
                         client_ptr->proxy_srvc_id,
                         client_ptr->proxy_client_id );
    }
    else
    {
      srvc_ptr->registered = TRUE;
      QMI_PROXY_DEBUG_MSG( "QMI_Proxy Client ID %d, QMI %s(%d), registered on remote modem\n",
                           client_ptr->proxy_client_id,
                           qmi_proxy_srvc_name[ client_ptr->proxy_srvc_id ],
                           client_ptr->proxy_srvc_id );
    }
  }

  /* Registration failure, release the QMI Proxy client */
  if ( rc != QMI_NO_ERR )
  {
    if ( client_ptr != NULL )
    {
      (void) qmi_proxy_release_client_data( client_ptr->proxy_client_id, client_ptr->proxy_srvc_id, TRUE, FALSE );
    }
  }

  return rc;

} /* qmi_proxy_client_init */


/*===========================================================================
  FUNCTION  qmi_proxy_client_release
===========================================================================*/
/*!
    @brief
    Release a QMI Proxy Client Connection

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_client_release
(
  qmi_client_id_type  proxy_client_id,
  qmi_service_id_type proxy_srvc_id
)
{
  int rc;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "%s .......QMI %s(%d)\n", __FUNCTION__, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id );

  /* Check whether QMI service is supported by QMI Proxy */
  if ( !QMI_PROXY_SRVC_IS_SUPPORTED( proxy_srvc_id ) )
  {
    QMI_PROXY_ERR_MSG( "QMI %s(%d) not supported by QMI Proxy\n",
                       qmi_proxy_srvc_name[ proxy_srvc_id ],
                       proxy_srvc_id );

    return QMI_SERVICE_ERR;
  }

  /* Release the QMI Proxy Client registration */
  rc = qmi_proxy_release_client_data( proxy_client_id, proxy_srvc_id, TRUE, FALSE );

  return rc;

} /* qmi_proxy_client_release */


/*===========================================================================
  FUNCTION  qmi_proxy_ctl_send_qmi_response
===========================================================================*/
/*!
    @brief
    Send QMI Proxy CTL response message to QMUX.

    @return
    None
*/
/*=========================================================================*/
void qmi_proxy_ctl_send_qmi_response
(
  qmi_qmux_if_msg_hdr_type *msg_hdr,
  unsigned long            rx_msg_id,
  unsigned long            rx_txn_id,
  unsigned char            *cmd_data,
  int                      cmd_data_len,
  int                      rc,
  int                      fd
)
{
  unsigned char *tmp_msg;
  int tmp_msg_len, tmp_msg_buf_size = 0, ret_rc = QMI_NO_ERR;
  unsigned char *tx_buf;
  int buf_size = 0;
  uint32_t result_code;

  /* -------------------------------------------------------------------  */

  buf_size = QMI_QMUX_HDR_SIZE + QMI_PROXY_CTL_MSG_HDR_SIZE + QMI_TLV_HDR_SIZE + sizeof( uint32_t ) +
             cmd_data_len;

  tx_buf = qmi_proxy_malloc( buf_size );
  if ( tx_buf == NULL )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Fail to allocate buffer for sending QMI Proxy CTL Response Msg" );
    rc = QMI_INTERNAL_ERR;
    return;
  }

  memset( tx_buf, 0, buf_size );

  /* Compose QMI CTL response */
  tmp_msg = tx_buf + QMI_QMUX_HDR_SIZE + QMI_PROXY_CTL_MSG_HDR_SIZE;
  tmp_msg_len = buf_size - QMI_QMUX_HDR_SIZE - QMI_PROXY_CTL_MSG_HDR_SIZE;
  qmi_proxy_write_ctl_srvc_msg_hdr( &tmp_msg,
                                    &tmp_msg_len,
                                    rx_msg_id,
                                    rx_txn_id );

  if ( rc != QMI_NO_ERR )
  {
    result_code = 1;
  }
  else
  {
    result_code = 0;
  }

  tmp_msg_buf_size = buf_size - QMI_QMUX_HDR_SIZE - QMI_PROXY_CTL_MSG_HDR_SIZE;
  tmp_msg = tx_buf + QMI_QMUX_HDR_SIZE + QMI_PROXY_CTL_MSG_HDR_SIZE;
  ret_rc = qmi_util_write_std_tlv( &tmp_msg,
                                   &tmp_msg_buf_size,
                                   QMI_PROXY_CTL_RESULT_CODE_TYPE_ID,
                                   sizeof( uint32_t ),
                                   (void *) &result_code );
  if ( ret_rc  == QMI_INTERNAL_ERR )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Failed to write std result code" );
  }

  tmp_msg = tx_buf + QMI_QMUX_HDR_SIZE + QMI_PROXY_CTL_MSG_HDR_SIZE + QMI_TLV_HDR_SIZE + sizeof( uint32_t );
  memcpy( tmp_msg, cmd_data, cmd_data_len );

  QMI_PROXY_DEBUG_MSG( "Sending QMI CTL response to QMI Proxy Client ...... Msg ID 0x%x, rc %d\n", msg_hdr->msg_id, rc );

  tmp_msg = tx_buf + QMI_QMUX_HDR_SIZE;
  qmi_proxy_send_response_to_clients( tx_buf + QMI_QMUX_HDR_SIZE,
                           buf_size - QMI_QMUX_HDR_SIZE,
                           msg_hdr->qmi_service_id,
                           msg_hdr->qmi_client_id,
                           QMI_PROXY_CONTROL_FLAG,
                           fd );

  /* free tx buffer */
  qmi_proxy_free( (void **)&tx_buf );

} /* qmi_proxy_ctl_send_qmi_response */


/*=========================================================================
  FUNCTION:  qmi_proxy_ctl_handle_request

===========================================================================*/
/*!
    @brief
    Handle the QMI CTL message.

    @return
    None

    @note
    qmi_proxy_init() must be called already.
*/
/*=========================================================================*/
static void *qmi_proxy_ctl_handle_request
(
  void *data
)
{
  qmi_proxy_thread_data_type *thread_data = NULL;
  qmi_qmux_if_msg_hdr_type *msg_hdr;
  unsigned char *msg;
  int msg_len;
  qmi_qmux_if_cmd_rsp_type cmd_data;
  qmi_connection_id_type conn_id;
  unsigned long rx_msg_id;
  unsigned long rx_length;
  unsigned long rx_txn_id;
  qmi_proxy_ctl_service_msg_type rx_msg_type;
  unsigned long type;
  unsigned long i, length;
  unsigned char *value_ptr = NULL;
  int qmi_err_code = QMI_NO_ERR;
  int rc = QMI_NO_ERR;
  unsigned char *ctl_req;
  int ctl_req_len;
  int ctl_resp_len = QMI_PROXY_CTL_MAX_MSG_SIZE;
  unsigned char *ctl_resp = NULL, *ctl_resp_ptr = NULL;
  unsigned char tlv_data[2];
  unsigned char *tlv_srv_ver_data = NULL, *tlv_srv_ver_data_ptr = NULL;
  int tlv_srv_ver_data_len = 0;
  int index;

  /*-----------------------------------------------------------------------*/

  ctl_resp = qmi_proxy_malloc( QMI_PROXY_CTL_MAX_MSG_SIZE );
  if ( ctl_resp == NULL )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Fail to allocate buffer for QMI Proxy CTL Response Msg" );
    rc = QMI_INTERNAL_ERR;
  }

  tlv_srv_ver_data = qmi_proxy_malloc( QMI_PROXY_MAX_SRV_VER_LEN );
  if ( tlv_srv_ver_data == NULL )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Fail to allocate buffer for QMI Proxy CTL Response Msg" );
    rc = QMI_INTERNAL_ERR;
  }

  /* Extract input data */
  thread_data = ( qmi_proxy_thread_data_type * ) data;
  if ( thread_data == NULL )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Invalid parameter in QMI Proxy CTL Request Msg" );
    rc = QMI_INTERNAL_ERR;
  }
  else
  {
    msg_hdr = &thread_data->msg_hdr;
    msg_len = thread_data->msg_len;
    msg = thread_data->msg;

    /* Sanity check on parameters */
    if ( ( msg_hdr == NULL ) || ( msg == NULL ) || ( msg_len == 0 ) )
    {
      QMI_PROXY_ERR_MSG( "%s\n", "Invalid parameter in QMI Proxy CTL Request Msg" );
      rc = QMI_INTERNAL_ERR;
    }
  }

  if ( rc == QMI_NO_ERR )
  {
    ctl_req = msg;
    ctl_req_len = msg_len;
    ctl_resp_ptr = ctl_resp;

    /* Read CTL service message header */
    rc = qmi_proxy_read_ctl_srvc_msg_hdr( &ctl_req, &ctl_req_len, &rx_msg_id, &rx_length, &rx_txn_id, &rx_msg_type );

    if ( rc == QMI_NO_ERR )
    {
      QMI_PROXY_DEBUG_MSG( "%s ....... Msg ID 0x%x Conn ID %d\n", __FUNCTION__, rx_msg_id, msg_hdr->qmi_conn_id );

      /* QMI CTL message specific processing */
      switch( rx_msg_id )
      {
        /* Setup a QMI Proxy Client */
        case QMI_PROXY_CTL_GET_CLIENT_ID_MSG_ID:
          if ( ( rc = qmi_util_read_std_tlv( &ctl_req, &ctl_req_len, &type, &length, &value_ptr ) ) !=  QMI_INTERNAL_ERR )
          {
            rc = QMI_NO_ERR;

            READ_8_BIT_VAL( value_ptr, cmd_data.qmi_qmux_if_alloc_client_req.service_id );

            tlv_data[ 0 ] = (unsigned char) cmd_data.qmi_qmux_if_alloc_client_req.service_id;

            rc = qmi_proxy_client_init( &cmd_data.qmi_qmux_if_alloc_client_rsp.new_client_id,
                                        msg_hdr->qmi_conn_id,
                                        cmd_data.qmi_qmux_if_alloc_client_req.service_id,
                                        thread_data->fd);

            /* Pack CTL response */
            tlv_data[ 1 ] = (unsigned char) cmd_data.qmi_qmux_if_alloc_client_rsp.new_client_id;


            if ( qmi_util_write_std_tlv( &ctl_resp_ptr,
                                         &ctl_resp_len,
                                         QMI_PROXY_CTL_GET_CLIENT_ID_TYPE_ID_REQ_RSP,
                                         2,
                                         (void *) tlv_data) < 0 )
            {
              QMI_PROXY_ERR_MSG( "%s\n", "Fail to pack CTL response" );
            }
          }
          else
          {
            QMI_PROXY_ERR_MSG( "%s\n", "Fail to read std tlv" );
          }
          break;

        /* Release QMI Proxy Client */
        case QMI_PROXY_CTL_RELEASE_CLIENT_ID_MSG_ID:
          if ( ( rc = qmi_util_read_std_tlv( &ctl_req, &ctl_req_len, &type, &length, &value_ptr ) ) !=  QMI_INTERNAL_ERR )
          {
            rc = QMI_NO_ERR;

            READ_8_BIT_VAL( value_ptr, cmd_data.qmi_qmux_if_release_client_req.delete_service_id );
            READ_8_BIT_VAL( value_ptr, cmd_data.qmi_qmux_if_release_client_req.delete_client_id );

            tlv_data[ 0 ] = (unsigned char) cmd_data.qmi_qmux_if_release_client_req.delete_service_id;
            tlv_data[ 1 ] = (unsigned char) cmd_data.qmi_qmux_if_release_client_req.delete_client_id;

            rc = qmi_proxy_client_release( cmd_data.qmi_qmux_if_release_client_req.delete_client_id,
                                           cmd_data.qmi_qmux_if_release_client_req.delete_service_id );

            /* Pack CTL response */
            if ( qmi_util_write_std_tlv( &ctl_resp_ptr,
                                         &ctl_resp_len,
                                         QMI_PROXY_CTL_RELEASE_CLIENT_ID_TYPE_ID_REQ_RSP,
                                         2,
                                         (void *) tlv_data) < 0 )
            {
              QMI_PROXY_ERR_MSG( "%s\n", "Fail to pack CTL response" );
            }
          }
          else
          {
            QMI_PROXY_ERR_MSG( "%s\n", "Fail to read std tlv" );
          }
          break;

        case QMI_PROXY_CTL_GET_VER_INFO_MSG_ID:
          if ( QMI_PROXY_CSFB_TARGET() )
          {
            conn_id = QMI_PROXY_REMOTE_CONN_ID;
          }
          else
          {
            conn_id = QMI_PROXY_LOCAL_CONN_ID;
          }

          rc = qmi_qmux_if_send_if_msg_to_qmux( qmi_service_get_qmux_if_handle(),
                                                QMI_QMUX_IF_GET_VERSION_INFO,
                                                conn_id,
                                                &cmd_data,
                                                &qmi_err_code,
                                                QMI_SYNC_MSG_DEFAULT_TIMEOUT );

          /* Pack the CTL response */
          tlv_srv_ver_data_ptr = tlv_srv_ver_data;
          WRITE_8_BIT_VAL( tlv_srv_ver_data_ptr, cmd_data.qmi_qmux_if_get_version_info_rsp.qmi_service_version_len );
          tlv_srv_ver_data_len += 1;
          for ( index = 0; index < cmd_data.qmi_qmux_if_get_version_info_rsp.qmi_service_version_len; index++ )
          {
            WRITE_8_BIT_VAL( tlv_srv_ver_data_ptr, cmd_data.qmi_qmux_if_get_version_info_rsp.qmi_service_version[ index ].qmi_svc_type );
            WRITE_16_BIT_VAL( tlv_srv_ver_data_ptr, cmd_data.qmi_qmux_if_get_version_info_rsp.qmi_service_version[ index ].major_ver );
            WRITE_16_BIT_VAL( tlv_srv_ver_data_ptr, cmd_data.qmi_qmux_if_get_version_info_rsp.qmi_service_version[ index ].minor_ver );
            tlv_srv_ver_data_len += 5;
          }

          if ( qmi_util_write_std_tlv( &ctl_resp_ptr,
                                       &ctl_resp_len,
                                       QMI_PROXY_CTL_GET_SERVICE_VERSION_LIST_TYPE_ID,
                                       tlv_srv_ver_data_len,
                                       (void *) tlv_srv_ver_data ) < 0 )
          {
            QMI_PROXY_ERR_MSG( "%s\n", "Fail to pack CTL response" );
          }
          break;

        case QMI_PROXY_CTL_GET_PWR_SAVE_MODE_MSG_ID:
          if ( QMI_PROXY_CSFB_TARGET() )
          {
            conn_id = QMI_PROXY_REMOTE_CONN_ID;
          }
          else
          {
            conn_id = QMI_PROXY_LOCAL_CONN_ID;
          }

          rc = qmi_qmux_if_send_if_msg_to_qmux( qmi_service_get_qmux_if_handle(),
                                                QMI_QMUX_IF_GET_PWR_STATE_MSG_ID,
                                                conn_id,
                                                &cmd_data,
                                                &qmi_err_code,
                                                QMI_SYNC_MSG_DEFAULT_TIMEOUT );

          /* Pack CTL response */
          if ( qmi_util_write_std_tlv( &ctl_resp_ptr,
                                       &ctl_resp_len,
                                       QMI_PROXY_CTL_GET_PWR_SAVE_MODE_TYPE_ID_RSP,
                                       4,
                                       (void *) &cmd_data.qmi_qmux_if_get_pwr_save_mode_rsp.curr_pwr_state ) < 0 )
          {
            QMI_PROXY_ERR_MSG( "%s\n", "Fail to pack CTL response" );
          }
          break;

        case QMI_PROXY_CTL_SET_DATA_FORMAT_MSG_ID:
          if ( ( rc = qmi_util_read_std_tlv( &ctl_req, &ctl_req_len, &type, &length, &value_ptr ) ) !=  QMI_INTERNAL_ERR )
          {
            rc = QMI_NO_ERR;

            READ_8_BIT_VAL( value_ptr, cmd_data.qmi_qmux_if_set_data_format_req.qos_hdr_state );

            if ( ctl_req_len > QMI_TLV_HDR_SIZE )
            {
              if ( ( rc = qmi_util_read_std_tlv( &ctl_req, &ctl_req_len, &type, &length, &value_ptr ) ) !=  QMI_INTERNAL_ERR )
              {
                rc = QMI_NO_ERR;

                READ_16_BIT_VAL( value_ptr, cmd_data.qmi_qmux_if_set_data_format_req.link_protocol );

                if ( QMI_PROXY_FUSION_TARGET() )
                {
                  conn_id = QMI_PROXY_REMOTE_CONN_ID;
                  rc = qmi_qmux_if_send_if_msg_to_qmux( qmi_service_get_qmux_if_handle(),
                                                        QMI_QMUX_IF_SET_DATA_FORMAT_MSG_ID,
                                                        conn_id,
                                                        &cmd_data,
                                                        &qmi_err_code,
                                                        QMI_SYNC_MSG_DEFAULT_TIMEOUT );
                }

                if ( ( QMI_PROXY_SVLTE_TARGET() || QMI_PROXY_MULTIMODE_TARGET() ) && ( rc == QMI_NO_ERR ) )
                {
                  conn_id = QMI_PROXY_LOCAL_CONN_ID;
                  rc = qmi_qmux_if_send_if_msg_to_qmux( qmi_service_get_qmux_if_handle(),
                                                        QMI_QMUX_IF_SET_DATA_FORMAT_MSG_ID,
                                                        conn_id,
                                                        &cmd_data,
                                                        &qmi_err_code,
                                                        QMI_SYNC_MSG_DEFAULT_TIMEOUT );
                }
              }
            }
          }

          /* Pack CTL response */
          if ( qmi_util_write_std_tlv( &ctl_resp_ptr,
                                       &ctl_resp_len,
                                       QMI_PROXY_CTL_SET_DATA_FORMAT_LINK_PROT_REQ_RSP,
                                       2,
                                       (void *) &cmd_data.qmi_qmux_if_set_data_format_rsp.link_protocol ) < 0 )
          {
            QMI_PROXY_ERR_MSG( "%s\n", "Fail to pack CTL response" );
          }

          if ( rc != QMI_NO_ERR )
          {
            QMI_PROXY_ERR_MSG( "%s\n", "Fail to read std tlv" );
          }
          break;

        case QMI_PROXY_CTL_REG_PWR_SAVE_MODE_MSG_ID:
          if ( ( rc = qmi_util_read_std_tlv( &ctl_req, &ctl_req_len, &type, &length, &value_ptr ) ) !=  QMI_INTERNAL_ERR )
          {
            rc = QMI_NO_ERR;

            READ_32_BIT_VAL( value_ptr, cmd_data.qmi_qmux_if_reg_pwr_save_mode_req.report_state );

            if ( QMI_PROXY_FUSION_TARGET() )
            {
              conn_id = QMI_PROXY_REMOTE_CONN_ID;
              rc = qmi_qmux_if_send_if_msg_to_qmux( qmi_service_get_qmux_if_handle(),
                                                    QMI_QMUX_IF_REG_PWR_SAVE_MODE_MSG_ID,
                                                    conn_id,
                                                    &cmd_data,
                                                    &qmi_err_code,
                                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT );
            }

            if ( ( QMI_PROXY_SVLTE_TARGET() || QMI_PROXY_MULTIMODE_TARGET() ) && ( rc == QMI_NO_ERR ) )
            {
              conn_id = QMI_PROXY_LOCAL_CONN_ID;
              rc = qmi_qmux_if_send_if_msg_to_qmux( qmi_service_get_qmux_if_handle(),
                                                    QMI_QMUX_IF_REG_PWR_SAVE_MODE_MSG_ID,
                                                    conn_id,
                                                    &cmd_data,
                                                    &qmi_err_code,
                                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT );
            }
          }
          else
          {
            QMI_PROXY_ERR_MSG( "%s\n", "Fail to read std tlv" );
          }
          break;

        case QMI_PROXY_CTL_SET_PWR_SAVE_MODE_MSG_ID:
          if ( ( rc = qmi_util_read_std_tlv( &ctl_req, &ctl_req_len, &type, &length, &value_ptr ) ) !=  QMI_INTERNAL_ERR )
          {
            rc = QMI_NO_ERR;

            READ_32_BIT_VAL( value_ptr, cmd_data.qmi_qmux_if_set_pwr_save_mode_req.new_pwr_state );

            if ( QMI_PROXY_FUSION_TARGET() )
            {
              conn_id = QMI_PROXY_REMOTE_CONN_ID;
              rc = qmi_qmux_if_send_if_msg_to_qmux( qmi_service_get_qmux_if_handle(),
                                                    QMI_QMUX_IF_SET_PWR_STATE_MSG_ID,
                                                    conn_id,
                                                    &cmd_data,
                                                    &qmi_err_code,
                                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT );
            }

            if ( ( QMI_PROXY_SVLTE_TARGET() || QMI_PROXY_MULTIMODE_TARGET() ) && ( rc == QMI_NO_ERR ) )
            {
              conn_id = QMI_PROXY_LOCAL_CONN_ID;
              rc = qmi_qmux_if_send_if_msg_to_qmux( qmi_service_get_qmux_if_handle(),
                                                    QMI_QMUX_IF_SET_PWR_STATE_MSG_ID,
                                                    conn_id,
                                                    &cmd_data,
                                                    &qmi_err_code,
                                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT );
            }
          }
          else
          {
            QMI_PROXY_ERR_MSG( "%s\n", "Fail to read std tlv" );
          }
          break;

        case QMI_PROXY_CTL_CONFIG_PWR_SAVE_SETTINGS_MSG_ID:
          if ( ( rc = qmi_util_read_std_tlv( &ctl_req, &ctl_req_len, &type, &length, &value_ptr ) ) !=  QMI_INTERNAL_ERR )
          {
            rc = QMI_NO_ERR;

            READ_32_BIT_VAL( value_ptr, cmd_data.qmi_qmux_if_config_pwr_save_settings_req.pwr_state_hndl );
            READ_8_BIT_VAL( value_ptr, cmd_data.qmi_qmux_if_config_pwr_save_settings_req.service_id );

            if ( ctl_req_len > QMI_TLV_HDR_SIZE )
            {
              if ( ( rc = qmi_util_read_std_tlv( &ctl_req, &ctl_req_len, &type, &length, &value_ptr ) ) !=  QMI_INTERNAL_ERR )
              {
                rc = QMI_NO_ERR;

                for ( i = 0 ; i < length; i++ )
                {
                  READ_16_BIT_VAL( value_ptr, cmd_data.qmi_qmux_if_config_pwr_save_settings_req.indication_ids[ i ] );
                  length -= 2;
                }

                if ( QMI_PROXY_FUSION_TARGET() )
                {
                  conn_id = QMI_PROXY_REMOTE_CONN_ID;
                  rc = qmi_qmux_if_send_if_msg_to_qmux( qmi_service_get_qmux_if_handle(),
                                                        QMI_QMUX_IF_CONFIG_PWR_SAVE_SETTINGS_MSG_ID,
                                                        conn_id,
                                                        &cmd_data,
                                                        &qmi_err_code,
                                                        QMI_SYNC_MSG_DEFAULT_TIMEOUT );
                }

                if ( ( QMI_PROXY_SVLTE_TARGET() || QMI_PROXY_MULTIMODE_TARGET() ) && ( rc == QMI_NO_ERR ) )
                {
                  conn_id = QMI_PROXY_LOCAL_CONN_ID;
                  rc = qmi_qmux_if_send_if_msg_to_qmux( qmi_service_get_qmux_if_handle(),
                                                        QMI_QMUX_IF_CONFIG_PWR_SAVE_SETTINGS_MSG_ID,
                                                        conn_id,
                                                        &cmd_data,
                                                        &qmi_err_code,
                                                        QMI_SYNC_MSG_DEFAULT_TIMEOUT );
                }
              }
            }
          }

          if ( rc != QMI_NO_ERR )
          {
            QMI_PROXY_ERR_MSG( "%s\n", "Fail to read std tlv" );
          }
          break;

        default:
          QMI_PROXY_ERR_MSG( "Unhandled QMI CTL request 0x%x\n", rx_msg_id );
          rc = QMI_INTERNAL_ERR;
          break;
      }

      QMI_PROXY_DEBUG_MSG( "CTL buff length = %d, response length = %d\n", ctl_resp_len, (QMI_PROXY_CTL_MAX_MSG_SIZE - ctl_resp_len));
      /* Send QMI Proxy CTL Response Message to QMUX */
      qmi_proxy_ctl_send_qmi_response( msg_hdr, rx_msg_id, rx_txn_id, ctl_resp, (QMI_PROXY_CTL_MAX_MSG_SIZE - ctl_resp_len), rc, thread_data->fd );
    }
  }

  /* Free buffer */
  qmi_proxy_free( (void **)&ctl_resp );

  qmi_proxy_free( (void **)&tlv_srv_ver_data );

  /* Free thread data */
  if ( thread_data != NULL )
  {
    if ( thread_data->msg != NULL )
    {
      qmi_proxy_free( (void **)&thread_data->msg );
    }

    qmi_proxy_free( (void **)&thread_data );
  }

  return NULL;

} /* qmi_proxy_ctl_handle_request */


/*===========================================================================
  FUNCTION  qmi_proxy_client_find_and_addref
===========================================================================*/
/*!
    @brief
    Lookup th    QMI Proxy Client record with specified key. Increment the
    reference count of the record if requested.

    @return
    Pointer to QMI Proxy Client record if success. Otherwise, NULL.
*/
/*=========================================================================*/
static qmi_proxy_client_info_type *qmi_proxy_client_find_and_addref
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id
)
{
  qmi_proxy_client_info_type *client = NULL;

  /* -------------------------------------------------------------------  */

  //QMI_PROXY_DEBUG_MSG( "%s ....... Client ID %d, QMI %s(%d)\n",
  //                     __FUNCTION__, proxy_client_id, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id );

  /* Lock global client list access mutex */
  QMI_PLATFORM_MUTEX_LOCK ( &qmi_proxy_client_list_mutex_tbl[ proxy_srvc_id ] );

  /* Look up the QMI Proxy Client record */
  client = qmi_proxy_client_info_tbl[ proxy_srvc_id ];
  while ( client != NULL )
  {
    if ( client->proxy_client_id == proxy_client_id )
    {
      break;
    }

    client = client->next;
  }

  /* Client not found */
  if ( client == NULL )
  {
    QMI_PROXY_ERR_MSG( "Fail to add reference, Client Rec not found (Client ID %d, QMI %s(%d)\n",
                       proxy_client_id, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id );
  }
  /* If we found the client and we want to increment reference count, do so */
  else
  {
    QMI_PLATFORM_MUTEX_LOCK( &client->mutex );

    if ( client->ready_to_delete )
    {
      QMI_PROXY_ERR_MSG( "Fail to add reference, Client Rec to be deleted (Client ID %d, QMI %s(%d)\n",
                         proxy_client_id, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id );

      QMI_PLATFORM_MUTEX_UNLOCK( &client->mutex );
      client = NULL;
    }
    else
    {
      client->ref_count++;
      QMI_PLATFORM_MUTEX_UNLOCK( &client->mutex );
    }

  }

  /* Unlock global client list access mutex */
  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_client_list_mutex_tbl[ proxy_srvc_id ] );

  return client;

} /* qmi_proxy_client_find_and_addref */


/*===========================================================================
  FUNCTION  qmi_proxy_get_client_info
===========================================================================*/
/*!
    @brief
    Finds the QMI Proxy Client information record corresponding to the input service and client id.

    @return
    Pointer to QMI Proxy Client record if success. Otherwise, NULL.
*/
/*=========================================================================*/
static qmi_proxy_client_info_type *qmi_proxy_get_client_info
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id
)
{
  qmi_proxy_client_info_type *client = NULL;

  /* -------------------------------------------------------------------  */

  /* Lock global client list access mutex */
  QMI_PLATFORM_MUTEX_LOCK ( &qmi_proxy_client_list_mutex_tbl[ proxy_srvc_id ] );

  /* Look up the QMI Proxy Client record */
  client = qmi_proxy_client_info_tbl[ proxy_srvc_id ];
  while ( client != NULL )
  {
    if ( client->proxy_client_id == proxy_client_id )
    {
      break;
    }

    client = client->next;
  }

  /* Client not found */
  if ( client == NULL )
  {
    QMI_PROXY_ERR_MSG( "Fail to add reference, Client Rec not found (Client ID %d, QMI %s(%d)\n",
                       proxy_client_id, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id );
  }

  /* Unlock global client list access mutex */
  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_client_list_mutex_tbl[ proxy_srvc_id ] );

  return client;

} /* qmi_proxy_get_client_info */


/*===========================================================================
  FUNCTION  qmi_proxy_client_relref
===========================================================================*/
/*!
    @brief
    Decrement the reference count to specified QMI Proxy Client record.
    In case of the reference count down to zero, delete the QMI Proxy client
    record if it is slated for deletion.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_client_relref
(
  qmi_proxy_client_info_type **client
)
{
  qmi_service_id_type proxy_srvc_id;
  qmi_client_id_type proxy_client_id;
  qmi_proxy_client_info_type *tmp_client;
  boolean delete_client = FALSE;

  /* -------------------------------------------------------------------  */

  /* Sanity check */
  if ( ( client == NULL ) || ( *client == NULL ) )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Fail to release reference count, null client pointer" );

    return;
  }

  tmp_client = *client;
  proxy_srvc_id = tmp_client->proxy_srvc_id;
  proxy_client_id = tmp_client->proxy_client_id;

  //QMI_PROXY_DEBUG_MSG( "%s ....... Client ID %d, QMI %s(%d)\n",
  //                     __FUNCTION__,
  //                    tmp_client->proxy_client_id,
  //                    qmi_proxy_srvc_name[ tmp_client->proxy_srvc_id ],
  //                    tmp_client->proxy_srvc_id );

  /* Set Client pointer to NULL */
  *client = NULL;

  /* Lock global client list mutex */
  QMI_PLATFORM_MUTEX_LOCK ( &qmi_proxy_client_list_mutex_tbl[ proxy_srvc_id ] );

  QMI_PLATFORM_MUTEX_LOCK( &tmp_client->mutex );

  /* Decrement the reference count */
  if ( tmp_client->ref_count )
  {
    tmp_client->ref_count--;
  }

  /* Check the ready for delete flag... if set, and reference count is 0,
  ** delete it
  */
  delete_client = ( ( tmp_client->ready_to_delete ) &&
                    ( tmp_client->ref_count == 0 ) ) ? TRUE : FALSE;

  QMI_PLATFORM_MUTEX_UNLOCK( &tmp_client->mutex );

  /* Delete the client if ready */
  if ( delete_client )
  {
    (void) qmi_proxy_release_client_data( tmp_client->proxy_client_id, tmp_client->proxy_srvc_id, FALSE, FALSE );
  }

  /* Unlock global client list mutex */
  QMI_PLATFORM_MUTEX_UNLOCK ( &qmi_proxy_client_list_mutex_tbl[ proxy_srvc_id ] );

} /* qmi_proxy_client_relref */


/*===========================================================================
  FUNCTION  qmi_proxy_lookup_client_handle
===========================================================================*/
/*!
    @brief
    Lookup the client handle for specified service from QMI Proxy client
    record

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_lookup_client_handle
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_proxy_client_info_type *client,
  qmi_service_id_type        srvc_id,
  qmi_proxy_conn_type        conn_type,
  qmi_client_type            *client_handle
)
{
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  //QMI_PROXY_DEBUG_MSG( "%s ....... QMI %s(%d), %s(%d)\n",
  //                    __FUNCTION__, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id, qmi_proxy_modem_name[ conn_type ], conn_type );

  /* Lookup the client handle for specified service from the QMI Proxy client record */
  if ( ( proxy_srvc_id == QMI_MAX_SERVICES ) || ( srvc_id == QMI_MAX_SERVICES ) )
  {
    QMI_PROXY_ERR_MSG( "QMI Proxy Client handle not found for QMI %s(%d)\n",
                       qmi_proxy_srvc_name[ srvc_id ],
                       srvc_id );
    rc = QMI_SERVICE_ERR;
  }
  else if ( ( client != NULL ) && ( proxy_srvc_id == srvc_id ) )
  {
    if ( client->srvc_info[ conn_type ].registered )
    {
      *client_handle = client->srvc_info[ conn_type ].client_handle;
      rc = QMI_NO_ERR;
    }
    else
    {
      QMI_PROXY_ERR_MSG( "QMI Proxy Client %d handle not found for QMI %s(%d)\n",
                         client->proxy_client_id,
                         qmi_proxy_srvc_name[ srvc_id ],
                         srvc_id );
      rc = QMI_SERVICE_ERR;
    }
  }
  /* Lookup the client handle for specified service from QMI Proxy Internal handle */
  else if ( (QMI_PROXY_SGLTE_TARGET() ||  QMI_PROXY_SVLTE_TARGET()) &&
            qmi_proxy_internal_info.srvc_info[ conn_type ][ srvc_id ].registered &&
            ( qmi_proxy_internal_info.srvc_info[ conn_type ][ srvc_id ].client_handle != NULL ) )
  {
    *client_handle = qmi_proxy_internal_info.srvc_info[ conn_type ][ srvc_id ].client_handle;
    rc = QMI_NO_ERR;
  }
  else
  {
    QMI_PROXY_ERR_MSG( "QPI: handle not found for QMI %s(%d) on %s(%d)\n",
                       qmi_proxy_srvc_name[ srvc_id ],
                       srvc_id,
                       qmi_proxy_modem_name[ conn_type ],
                       conn_type );
    rc = QMI_SERVICE_ERR;
  }

  return rc;

} /* qmi_proxy_lookup_client_handle */


/*=========================================================================
  FUNCTION:  qmi_proxy_decode_qmi_srvc_message

===========================================================================*/
/*!
    @brief
    Decode QMI Service Message.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_decode_qmi_srvc_message
(
  qmi_service_id_type          proxy_srvc_id,
  qmi_client_id_type           proxy_client_id,
  qmi_proxy_conn_type          conn_type,
  qmi_service_id_type          srvc_id,
  qmi_idl_type_of_message_type idl_msg_type,
  unsigned long                msg_id,
  unsigned char                *rx_msg,
  int                          rx_msg_len,
  void                         **decoded_payload,
  uint32_t                     *decoded_payload_len
)
{
  qmi_proxy_client_info_type *client = NULL;
  qmi_client_type client_handle = NULL;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "%s ....... Client ID %d, QMI %s(%d), Msg ID 0x%x, RX Msg Len %d\n",
                       __FUNCTION__, proxy_client_id, qmi_proxy_srvc_name[ srvc_id ], srvc_id, msg_id, rx_msg_len );

  if ( srvc_id >= QMI_MAX_SERVICES )
  {
    QMI_PROXY_ERR_MSG(" Invalid Service Id(%d) received, skip decoding the message \n", srvc_id );
    return QMI_INTERNAL_ERR;
  }

  if ( rx_msg_len == 0 )
  {
    QMI_PROXY_DEBUG_MSG( "Skip decoding of QMI %s(%d), %s(%d), Msg ID 0x%x with zero payload for client %d\n",
                         qmi_proxy_srvc_name[ srvc_id ],
                         srvc_id,
                         qmi_proxy_msg_type_name[ idl_msg_type ],
                         idl_msg_type,
                         msg_id,
                         proxy_client_id );

    return rc;
  }

  /* Allocate buffer */
  rc = qmi_idl_get_message_c_struct_len( qmi_proxy_srvc_obj_tbl[ srvc_id ],
                                         idl_msg_type,
                                         msg_id,
                                         decoded_payload_len );
  if ( rc != QMI_NO_ERR )
  {
    QMI_PROXY_ERR_MSG( "Fail to get msg len to decode QMI %s(%d), %s(%d), Msg ID 0x%x for Client %d, rc %d\n",
                       qmi_proxy_srvc_name[ srvc_id ],
                       srvc_id,
                       qmi_proxy_msg_type_name[ idl_msg_type ],
                       idl_msg_type,
                       msg_id,
                       proxy_client_id,
                       rc );

    return QMI_INTERNAL_ERR;
  }

  if ( *decoded_payload_len > 0 )
  {
    *decoded_payload = qmi_proxy_malloc( *decoded_payload_len );
    if ( *decoded_payload == NULL )
    {
      QMI_PROXY_ERR_MSG( "Fail to allocate buffer to decode QMI %s(%d), %s(%d), Msg ID 0x%x, for Client %d\n",
                         qmi_proxy_srvc_name[ srvc_id ],
                         srvc_id,
                         qmi_proxy_msg_type_name[ idl_msg_type ],
                         idl_msg_type,
                         msg_id,
                         proxy_client_id );

      return QMI_INTERNAL_ERR;
    }
  }
  else
  {
    QMI_PROXY_ERR_MSG( "Zero size c struct for response!, QMI %s(%d), %s(%d), Msg ID 0x%x, RX Msg Len %d\n",
                       qmi_proxy_srvc_name[ srvc_id ],
                       srvc_id,
                       qmi_proxy_msg_type_name[ idl_msg_type ],
                       idl_msg_type,
                       msg_id,
                       rx_msg_len );

    return QMI_INTERNAL_ERR;
  }

  /* Lookup QMI Proxy client record */
  if ( proxy_client_id != QMI_PROXY_INTERNAL_CLIENT_ID )
  {
    client = qmi_proxy_client_find_and_addref( proxy_srvc_id, proxy_client_id );
    if ( client == NULL )
    {
      QMI_PROXY_ERR_MSG( "Client %d not registered, fail to decode QMI %s(%d), %s(%d), Msg ID 0x%x\n",
                         proxy_client_id,
                         qmi_proxy_srvc_name[ srvc_id ],
                         srvc_id,
                         qmi_proxy_msg_type_name[ idl_msg_type ],
                         idl_msg_type,
                         msg_id );

      /* Free buffer */
      qmi_proxy_free( (void **)decoded_payload );
      return QMI_SERVICE_ERR;
    }
  }

  /* Lookup the service user handle info */
  rc = qmi_proxy_lookup_client_handle( proxy_srvc_id,
                                       client,
                                       proxy_srvc_id,
                                       conn_type,
                                       &client_handle );
  if ( rc != QMI_NO_ERR )
  {
    QMI_PROXY_ERR_MSG( "Fail to lookup client %d handle to decode QMI %s(%d), %s(%d), Msg ID 0x%x\n",
                       proxy_client_id,
                       qmi_proxy_srvc_name[ srvc_id ],
                       srvc_id,
                       qmi_proxy_msg_type_name[ idl_msg_type ],
                       idl_msg_type,
                       msg_id );
  }
  else
  {
    /* Decode the QMI Service Message */
    rc = qmi_client_message_decode( client_handle,
                                    idl_msg_type,
                                    msg_id,
                                    rx_msg,
                                    rx_msg_len,
                                    *decoded_payload,
                                    *decoded_payload_len );
    if ( rc != QMI_NO_ERR )
    {
      QMI_PROXY_ERR_MSG( "Fail to decode QMI %s(%d), %s(%d), Msg ID 0x%x, for Client %d, rc %d\n",
                         qmi_proxy_srvc_name[ srvc_id ],
                         srvc_id,
                         qmi_proxy_msg_type_name[ idl_msg_type ],
                         idl_msg_type,
                         msg_id,
                         proxy_client_id,
                         rc );
    }
  }

  if ( ( proxy_client_id != QMI_PROXY_INTERNAL_CLIENT_ID ) && ( client != NULL ) )
  {
    /* Release reference to QMI Proxy Client Info */
    qmi_proxy_client_relref( &client );
  }

  if ( ( rc != QMI_NO_ERR ) && ( *decoded_payload != NULL ) )
  {
    qmi_proxy_free( (void **)decoded_payload );
    *decoded_payload = NULL;
  }

  return rc;

} /* qmi_proxy_decode_qmi_srvc_message */


/*=========================================================================
  FUNCTION:  qmi_proxy_encode_qmi_srvc_message

===========================================================================*/
/*!
    @brief
    Encode QMI Service Message.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_encode_qmi_srvc_message
(
  qmi_service_id_type          proxy_srvc_id,
  qmi_client_id_type           proxy_client_id,
  qmi_proxy_conn_type          conn_type,
  qmi_service_id_type          srvc_id,
  qmi_idl_type_of_message_type req_resp_ind,
  unsigned long                msg_id,
  void                         *req_c_struct,
  int                          req_c_struct_len,
  unsigned char                **encoded_payload,
  int                          *encoded_payload_len,
  unsigned char                **encoded_msg,
  int                          *encoded_msg_len
)
{
  qmi_proxy_client_info_type *client = NULL;
  qmi_client_type client_handle = NULL;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "%s ....... QMI %s(%d), %s(%d), %s(%d), Msg ID 0x%x\n",
                       __FUNCTION__,
                       qmi_proxy_srvc_name[ srvc_id ],
                       srvc_id,
                       qmi_proxy_modem_name[ conn_type ],
                       conn_type,
                       qmi_proxy_msg_type_name[ req_resp_ind ],
                       req_resp_ind,
                       msg_id );

  /* Extract the max message size from the service object */
  rc = qmi_idl_get_max_message_len( qmi_proxy_srvc_obj_tbl[ srvc_id ],
                                    req_resp_ind,
                                    msg_id,
                                    (uint32_t *) encoded_payload_len );
  if ( rc != QMI_NO_ERR )
  {
    QMI_PROXY_DEBUG_MSG( "Fail to get max msg size to encode QMI %s(%d), %s(%d), Msg ID 0x%x for client %d\n",
                         qmi_proxy_srvc_name[ srvc_id ],
                         srvc_id,
                         qmi_proxy_msg_type_name[ req_resp_ind ],
                         req_resp_ind,
                         msg_id,
                         proxy_client_id );

    return rc;
  }

  /* Allocate buffer */
  *encoded_payload_len += QMI_MAX_HDR_SIZE;
  *encoded_payload = qmi_proxy_malloc( *encoded_payload_len );
  if ( *encoded_payload == NULL )
  {
    QMI_PROXY_DEBUG_MSG( "Fail to allocate buffer to encode QMI %s(%d), %s(%d), Msg ID 0x%x for client %d\n",
                         qmi_proxy_srvc_name[ srvc_id ],
                         srvc_id,
                         qmi_proxy_msg_type_name[ req_resp_ind ],
                         req_resp_ind,
                         msg_id,
                         proxy_client_id );

    return QMI_CLIENT_ALLOC_FAILURE;
  }

  /* Set msg_ptr to beginning of message-specific TLV portion of message buffer */
  *encoded_msg = QMI_CLIENT_TLV_PTR( *encoded_payload );
  *encoded_msg_len = *encoded_payload_len - QMI_MAX_HDR_SIZE;

  /* Encode the buffer  */
  if ( req_c_struct_len > 0 )
  {
    if ( proxy_client_id != QMI_PROXY_INTERNAL_CLIENT_ID )
    {
      /* Locate the QMI Proxy client info */
      client = qmi_proxy_client_find_and_addref( proxy_srvc_id, proxy_client_id );
      if ( client == NULL )
      {
        QMI_PROXY_DEBUG_MSG( "Fail to encode QMI %s(%d), %s(%d), Msg ID 0x%x for client %d not registered\n",
                             qmi_proxy_srvc_name[ srvc_id ],
                             srvc_id,
                             qmi_proxy_msg_type_name[ req_resp_ind ],
                             req_resp_ind,
                             msg_id,
                             proxy_client_id );

        rc = QMI_SERVICE_ERR;
      }
    }

    if ( rc == QMI_NO_ERR )
    {
      /* Lookup the service user handle info */
      rc = qmi_proxy_lookup_client_handle( proxy_srvc_id,
                                           client,
                                           srvc_id,
                                           conn_type,
                                           &client_handle );
      if ( rc != QMI_NO_ERR )
      {
        QMI_PROXY_DEBUG_MSG( "Fail to encode QMI %s(%d), %s(%d), Msg ID 0x%x for client %d handle not found\n",
                             qmi_proxy_srvc_name[ srvc_id ],
                             srvc_id,
                             qmi_proxy_msg_type_name[ req_resp_ind ],
                             req_resp_ind,
                             msg_id,
                             proxy_client_id );
      }
    }

    if ( rc == QMI_NO_ERR )
    {
      /* Encode QMI Service Message */
      rc = qmi_client_message_encode( client_handle,
                                      req_resp_ind,
                                      msg_id,
                                      req_c_struct,
                                      req_c_struct_len,
                                      *encoded_msg,
                                      *encoded_msg_len,
                                      encoded_msg_len );
      if ( rc != QMI_NO_ERR )
      {
        QMI_PROXY_DEBUG_MSG( "Fail to encode QMI %s(%d), %s(%d), Msg ID 0x%x for client %d, rc %d\n",
                             qmi_proxy_srvc_name[ srvc_id ],
                             srvc_id,
                             qmi_proxy_msg_type_name[ req_resp_ind ],
                             req_resp_ind,
                             msg_id,
                             proxy_client_id,
                             rc );
      }
    }


    /* Release reference to QMI Proxy Client Info */
    if ( ( proxy_client_id != QMI_PROXY_INTERNAL_CLIENT_ID ) && ( client != NULL ) )
    {
      qmi_proxy_client_relref( &client );
    }
  }

  if ( ( rc != QMI_NO_ERR ) && ( *encoded_payload != NULL ) )
  {
    qmi_proxy_free( (void **)encoded_payload );
    *encoded_payload = NULL;
  }
  /* Comment out for debug use only
  else
  {
    QMI_PROXY_DEBUG_MSG( "%d bytes encoded\n", *encoded_msg_len );
    qmi_proxy_print_hex( *encoded_msg, *encoded_msg_len );
  }
  */

  return rc;

} /* qmi_proxy_encode_qmi_srvc_message */


/*=========================================================================
  FUNCTION:  qmi_proxy_print_txn_list

===========================================================================*/
/*!
    @brief
    Pront the content of the transaction list for a specified QMI Service.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_print_txn_list
(
  qmi_service_id_type proxy_srvc_id
)
{
  qmi_proxy_txn_info_type *txn = NULL;
  uint8 i, j, count = 0;

  /* -------------------------------------------------------------------  */

  txn = qmi_proxy_txn_info_tbl[ proxy_srvc_id ];

  if ( txn != NULL )
  {
    QMI_PROXY_DEBUG_MSG( "Txn List for QMI %s(%d):\n",
                         qmi_proxy_srvc_name[ proxy_srvc_id ],
                         proxy_srvc_id );
  }

  while ( txn != NULL )
  {
    QMI_PROXY_DEBUG_MSG( "   Txn Rec[%d] (Txn Key 0x%x, Txn ID %d, QMI %s(%d), Client ID %d, RX Txn ID %d, User Data valid %d, User Data 0x%x)\n",
                         count++,
                         txn->proxy_txn_key,
                         QMI_PROXY_TXN_KEY_TO_TXN_ID( txn->proxy_txn_key ),
                         qmi_proxy_srvc_name[ txn->proxy_srvc_id ],
                         txn->proxy_srvc_id,
                         txn->proxy_client_id,
                         txn->rx_txn_id,
                         txn->user_data_valid,
                         txn->user_data );

    for ( i = 0; i < QMI_PROXY_MAX_CONN_TYPE; i++ )
    {
      for ( j = 0; j < txn->txns[ i ].num_txn && j < QMI_PROXY_MAX_SRVC_TXNS; j++ )
      {
        QMI_PROXY_DEBUG_MSG( "             ... %s(%d) : Txn Index %d, QMI %s(%d), Msg ID 0x%x, Msg Len %d\n",
                             qmi_proxy_modem_name[ i ],
                             i,
                             j,
                             qmi_proxy_srvc_name[ txn->txns[ i ].entry[ j ].srvc_id ],
                             txn->txns[ i ].entry[ j ].srvc_id,
                             txn->txns[ i ].entry[ j ].msg_id,
                             txn->txns[ i ].entry[ j ].msg_len );
      }
    }

    txn = txn->next;
  }

} /* qmi_proxy_print_txn_list */


/*=========================================================================
  FUNCTION:  qmi_proxy_alloc_txn_data

===========================================================================*/
/*!
    @brief
    Allocate memory for a QMI Proxy transaction data

    @return
    Pointer to QMI Proxy transaction data
*/
/*=========================================================================*/
static qmi_proxy_txn_info_type *qmi_proxy_alloc_txn_data
(
  qmi_service_id_type            proxy_srvc_id,
  qmi_client_id_type             proxy_client_id,
  qmi_connection_id_type         proxy_conn_id,
  unsigned long                  proxy_msg_id,
  qmi_proxy_txn_entry_list_type  *local_txn_list,
  qmi_proxy_txn_entry_list_type  *remote_txn_list,
  unsigned long                  rx_txn_id,
  qmi_proxy_async_state_type     async_state,
  boolean                        user_data_valid,
  uint32                         user_data
)
{
  qmi_proxy_txn_info_type *txn = NULL;
  qmi_proxy_conn_type conn_type;
  uint8 i, j;
  int rc = QMI_NO_ERR;

  /* -------------------------------------------------------------------  */

  /* Dynamically allocate a transaction data structure */
  txn = qmi_proxy_malloc( sizeof( qmi_proxy_txn_info_type ) );

  if ( txn == NULL )
  {
    QMI_PROXY_DEBUG_MSG( "Fail to allocate memory to setup QMI Proxy Txn (Client ID %d, QMI %s(%d))\n",
                         proxy_client_id, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id );
    return txn;
  }

  /* Assign TXN ID fields */
  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_txn_id_mutex );

  txn->proxy_txn_key = QMI_PROXY_COMPOSE_TXN_KEY( proxy_srvc_id, qmi_proxy_txn_id );

  /* Advance to next available transaction ID */
  qmi_proxy_txn_id = ( qmi_proxy_txn_id + 1 ) & 0xFFFF;
  if ( qmi_proxy_txn_id == 0 )
  {
    qmi_proxy_txn_id = 1;
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_txn_id_mutex );

  txn->proxy_srvc_id = proxy_srvc_id;
  txn->proxy_client_id = proxy_client_id;
  txn->proxy_conn_id = proxy_conn_id;
  txn->proxy_msg_id = proxy_msg_id;
  txn->rx_txn_id = rx_txn_id;
  txn->state = QMI_PROXY_TXN_IN_PROGRESS;
  txn->txns[ QMI_PROXY_LOCAL_CONN_TYPE ].state = QMI_PROXY_TXN_IDLE;
  txn->txns[ QMI_PROXY_LOCAL_CONN_TYPE ].num_txn = 0;
  txn->txns[ QMI_PROXY_REMOTE_CONN_TYPE ].state = QMI_PROXY_TXN_IDLE;
  txn->txns[ QMI_PROXY_REMOTE_CONN_TYPE ].num_txn = 0;
  txn->async_state = async_state;
  txn->user_data_valid = user_data_valid;
  txn->user_data = user_data;

  /* Setup Local QMI Service transaction(s) */
  if ( local_txn_list->num_txn > 0 )
  {
    /* Fill in transaction entry details */
    conn_type = QMI_PROXY_LOCAL_CONN_TYPE;
    for ( i = 0; i < local_txn_list->num_txn && i < QMI_PROXY_MAX_SRVC_TXNS; i++ )
    {
      txn->txns[ conn_type ].entry[ i ].msg = NULL;

      /* Allocate the message structure for QMI Request message */
      if ( local_txn_list->entry[ i ].msg_len > 0 )
      {
        txn->txns[ conn_type ].entry[ i ].msg = qmi_proxy_malloc( local_txn_list->entry[ i ].msg_len );
        if ( txn->txns[ conn_type ].entry[ i ].msg == NULL )
        {
          QMI_PROXY_DEBUG_MSG( "%s\n", "Fail to allocate local txn message buffer" );
          rc = QMI_INTERNAL_ERR;
          break;
        }

        memcpy( txn->txns[ conn_type ].entry[ i ].msg, local_txn_list->entry[ i ].msg, local_txn_list->entry[ i ].msg_len );
      }

      txn->txns[ conn_type ].entry[ i ].msg_id = local_txn_list->entry[ i ].msg_id;
      txn->txns[ conn_type ].entry[ i ].msg_len = local_txn_list->entry[ i ].msg_len;
      txn->txns[ conn_type ].entry[ i ].srvc_id = local_txn_list->entry[ i ].srvc_id;
      txn->txns[ conn_type ].entry[ i ].state = QMI_PROXY_TXN_ENTRY_SETUP;
      txn->txns[ conn_type ].state = QMI_PROXY_TXN_IN_PROGRESS;
      txn->txns[ conn_type ].num_txn += 1;
    }
  }

  /* Setup Remote QMI Service transaction(s) */
  if ( ( rc == QMI_NO_ERR ) && ( remote_txn_list->num_txn > 0 ) )
  {
    /* Fill in transaction entry details */
    conn_type = QMI_PROXY_REMOTE_CONN_TYPE;
    for ( i = 0; i < remote_txn_list->num_txn && i < QMI_PROXY_MAX_SRVC_TXNS; i++ )
    {
      txn->txns[ conn_type ].entry[ i ].msg = NULL;

      /* Allocate the message structure for QMI Request message */
      if ( remote_txn_list->entry[ i ].msg_len > 0 )
      {
        txn->txns[ conn_type ].entry[ i ].msg = qmi_proxy_malloc( remote_txn_list->entry[ i ].msg_len );
        if ( txn->txns[ conn_type ].entry[ i ].msg == NULL )
        {
          QMI_PROXY_DEBUG_MSG( "%s\n", "Fail to allocate remote rxn message buffer" );
          rc = QMI_INTERNAL_ERR;
          break;
        }

        memcpy( txn->txns[ conn_type ].entry[ i ].msg, remote_txn_list->entry[ i ].msg, remote_txn_list->entry[ i ].msg_len );
      }

      txn->txns[ conn_type ].entry[ i ].msg_id = remote_txn_list->entry[ i ].msg_id;
      txn->txns[ conn_type ].entry[ i ].msg_len = remote_txn_list->entry[ i ].msg_len;
      txn->txns[ conn_type ].entry[ i ].srvc_id = remote_txn_list->entry[ i ].srvc_id;
      txn->txns[ conn_type ].entry[ i ].state = QMI_PROXY_TXN_ENTRY_SETUP;
      txn->txns[ conn_type ].state = QMI_PROXY_TXN_IN_PROGRESS;
      txn->txns[ conn_type ].num_txn += 1;
    }
  }

  if ( rc == QMI_NO_ERR )
  {
    /* Initialize mutex */
    QMI_PLATFORM_MUTEX_INIT( &txn->mutex );

    /* Increment reference count */
    txn->ref_count++;

    /* Lock global transaction access mutex */
    QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_txn_list_mutex_tbl[ proxy_srvc_id ] );

    /* Add new transaction data record to the list */
    txn->next = qmi_proxy_txn_info_tbl[ proxy_srvc_id ];
    qmi_proxy_txn_info_tbl[ proxy_srvc_id ] = txn;

    qmi_proxy_print_txn_list( proxy_srvc_id );

    /* Unlock global client access mutex */
    QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_txn_list_mutex_tbl[ proxy_srvc_id ] );

    QMI_PROXY_DEBUG_MSG( "Txn Setup (Txn Key 0x%x, Txn ID %d, QMI %s(%d), Client ID %d, RX Txn ID %d, Async State %d)\n",
                         txn->proxy_txn_key,
                         QMI_PROXY_TXN_KEY_TO_TXN_ID( txn->proxy_txn_key ),
                         qmi_proxy_srvc_name[ txn->proxy_srvc_id ],
                         txn->proxy_srvc_id,
                         txn->proxy_client_id,
                         txn->rx_txn_id,
                         txn->async_state );
  }
  else
  {
    /* Release transaction message memory */
    for ( i = 0; i < QMI_PROXY_MAX_CONN_TYPE; i++ )
    {
      for ( j = 0; j < txn->txns[ i ].num_txn; j++ )
      {
        if ( txn->txns[ i ].entry[ j ].msg != NULL )
        {
          qmi_proxy_free( (void **)&txn->txns[ i ].entry[ j ].msg );
          txn->txns[ i ].entry[ j ].msg = NULL;
        }
      }
    }

    qmi_proxy_free( (void **)&txn );

    txn = NULL;
  }

  return txn;

} /* qmi_proxy_alloc_txn_data */


/*===========================================================================
  FUNCTION  qmi_proxy_txn_find_and_addref
===========================================================================*/
/*!
    @brief
    Lookup the QMI Proxy transaction record with specified key or
    specified user data on QMI Indication. Increment the
    reference count of the transaction record if requested.

    @return
    Pointer to QMI Proxy Transaction record if success. Otherwise, NULL.
*/
/*=========================================================================*/
static qmi_proxy_txn_info_type *qmi_proxy_txn_find_and_addref
(
  qmi_proxy_conn_type    proxy_conn_type,
  qmi_service_id_type    proxy_srvc_id,
  boolean                proxy_txn_key_included,
  qmi_proxy_txn_key_type proxy_txn_key,
  boolean                user_data_included,
  uint32                 user_data,
  boolean                is_recvd_ind
)
{
  qmi_proxy_txn_info_type *txn = NULL, *prev = NULL;
  int rc = QMI_NO_ERR;

  /* -------------------------------------------------------------------  */

  (void) user_data_included;

  if ( proxy_txn_key_included )
  {
//    QMI_PROXY_DEBUG_MSG( "%s ....... Txn Key 0x%x, QMI %s(%d)\n",
//                         __FUNCTION__, proxy_txn_key, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id );
  }
  else
  {
    QMI_PROXY_DEBUG_MSG( "%s ....... QMI %s(%d), user data %d\n",
                         __FUNCTION__, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id, user_data );
  }

  QMI_PLATFORM_MUTEX_LOCK ( &qmi_proxy_txn_list_mutex_tbl[ proxy_srvc_id ] );

  /* Look up the QMI Proxy Transaction record */
  txn = qmi_proxy_txn_info_tbl[ proxy_srvc_id ];
  while ( txn != NULL )
  {
    if ( proxy_txn_key_included )
    {
      if ( txn->proxy_txn_key == proxy_txn_key )
      {
        break;
      }
    }
    else
    {
      if ( ( ( proxy_conn_type == QMI_PROXY_LOCAL_CONN_TYPE ) &&
             ( txn->async_state == QMI_PROXY_ASYNC_REMOTE_TXN_WAIT_IND ) && txn->user_data_valid && ( txn->user_data == user_data ) ) ||
           ( ( proxy_conn_type == QMI_PROXY_REMOTE_CONN_TYPE ) &&
             ( txn->async_state == QMI_PROXY_ASYNC_LOCAL_TXN_WAIT_IND ) && txn->user_data_valid && ( txn->user_data == user_data ) ) )
      {
        txn->async_state = QMI_PROXY_ASYNC_IDLE;
        break;
      }
      else if (!txn->is_recvd_ind)
      {
        txn->is_recvd_ind = is_recvd_ind;
      }
    }

    prev = txn;
    txn = txn->next;
  }

  /* Txn not found */
  if ( txn == NULL )
  {
    if ( proxy_txn_key_included )
    {
      QMI_PROXY_ERR_MSG( "Fail to add reference, Txn Rec not found (Txn Key 0x%x, QMI %s(%d)\n",
                         proxy_txn_key, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id );
    }
  }
  /* If we found the Txn and we want to increment reference count, do so */
  else
  {
    QMI_PLATFORM_MUTEX_LOCK( &txn->mutex );

    if ( !txn->ready_to_delete )
    {
      txn->ref_count++;
    }
    else
    {
      rc = QMI_INTERNAL_ERR;

      QMI_PROXY_ERR_MSG( "Fail to add reference, Txn Rec to be deleted (Txn Key 0x%x, QMI %s(%d)\n",
                         proxy_txn_key, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id );
    }

    QMI_PLATFORM_MUTEX_UNLOCK( &txn->mutex );
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_txn_list_mutex_tbl[ proxy_srvc_id ] );

  if ( rc != QMI_NO_ERR )
  {
    txn = NULL;
  }

  QMI_PROXY_DEBUG_MSG( "%s ....... returning %p\n",
                         __FUNCTION__, txn );


  return txn;

} /* qmi_proxy_txn_find_and_addref */


/*===========================================================================
  FUNCTION  qmi_proxy_txn_relref
===========================================================================*/
/*!
    @brief
    Decrement the reference count to specified QMI Proxy transaction record.
    In case of the reference count down to zero, delete the QMI Proxy
    transaction record if it is slated for deletion.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_txn_relref
(
  qmi_proxy_txn_info_type **txn,
  boolean                 txn_done,
  boolean                 lock_list_mutex
)
{
  qmi_proxy_txn_key_type proxy_txn_key;
  qmi_service_id_type proxy_srvc_id;
  qmi_proxy_txn_info_type *tmp_txn = NULL, *cur = NULL, *prev = NULL;
  boolean delete_txn = FALSE;
  uint8 i, j;

  /* -------------------------------------------------------------------  */

  /* Sanity check */
  if ( ( txn == NULL ) || ( *txn == NULL ) )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Fail to release reference count, null txn pointer" );
    return;
  }

  tmp_txn = *txn;
  proxy_srvc_id = tmp_txn->proxy_srvc_id;
  proxy_txn_key = tmp_txn->proxy_txn_key;

  if ( tmp_txn->proxy_srvc_id == QMI_MAX_SERVICES )
  {
    QMI_PROXY_ERR_MSG( "Fail to release reference count, invalid service id %d\n", tmp_txn->proxy_srvc_id );
    return;
  }

  //QMI_PROXY_DEBUG_MSG( "%s ....... Txn Key 0x%x, QMI %s(%d)\n",
  //                     __FUNCTION__, proxy_txn_key, qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id );

  /* Set Txn pointer to NULL */
  *txn = NULL;

  /* Lock global transaction access mutex */
  if ( lock_list_mutex )
  {
    QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_txn_list_mutex_tbl[ proxy_srvc_id ] );
  }

  QMI_PLATFORM_MUTEX_LOCK( &tmp_txn->mutex );

  /* Transaction is done, set the delete_flag */
  if ( txn_done )
  {
    tmp_txn->ready_to_delete = TRUE;
  }

  /* Decrement the reference count */
  if ( tmp_txn->ref_count )
  {
    tmp_txn->ref_count--;
  }

  /* Check the ready for delete flag... if set, and reference count is 0, delete it */
  delete_txn = ( ( tmp_txn->ready_to_delete ) &&
                 ( tmp_txn->ref_count == 0 ) ) ? TRUE : FALSE;

  /* Delete the client if ready */
  if ( !delete_txn )
  {
    QMI_PLATFORM_MUTEX_UNLOCK( &tmp_txn->mutex );
  }
  else
  {
    /* Locate the Transaction record */
    cur = qmi_proxy_txn_info_tbl[ proxy_srvc_id ];
    while ( cur != NULL )
    {
      if ( cur->proxy_txn_key == proxy_txn_key )
      {
        break;
      }

      prev = cur;
      cur = cur->next;
    }

    if ( cur == NULL )
    {
      QMI_PROXY_ERR_MSG( "Fail to delete Txn, Txn Rec (Txn Key 0x%x) not found in the QMI %s(%d) Txn List\n",
                         tmp_txn->proxy_txn_key,
                         qmi_proxy_srvc_name[ tmp_txn->proxy_srvc_id ],
                         tmp_txn->proxy_srvc_id );

      QMI_PLATFORM_MUTEX_UNLOCK( &tmp_txn->mutex );
    }
    /* Remove the transaction record from the list */
    else
    {
      if ( prev == NULL )
      {
        qmi_proxy_txn_info_tbl[ tmp_txn->proxy_srvc_id ] = cur->next;
      }
      else
      {
        prev->next = cur->next;
      }

      tmp_txn->next = NULL;

      qmi_proxy_print_txn_list( proxy_srvc_id );

      /* Release transaction message memory */
      for ( i = 0; i < QMI_PROXY_MAX_CONN_TYPE; i++ )
      {
        for ( j = 0; j < tmp_txn->txns[ i ].num_txn; j++ )
        {
          if ( tmp_txn->txns[ i ].entry[ j ].msg != NULL )
          {
            qmi_proxy_free( (void **)&tmp_txn->txns[ i ].entry[ j ].msg );
            tmp_txn->txns[ i ].entry[ j ].msg = NULL;
          }
        }
      }

      /* Unlock the transaction mutex */
      QMI_PLATFORM_MUTEX_UNLOCK( &tmp_txn->mutex );

      /* Destroy the transaction mutex */
      QMI_PLATFORM_MUTEX_DESTROY( &tmp_txn->mutex );

      /* Free the memory used by the transaction */
      qmi_proxy_free( (void **)&tmp_txn );

      tmp_txn = NULL;
    }
  }

  /* Unlock global transaction access mutex */
  if ( lock_list_mutex )
  {
    QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_txn_list_mutex_tbl[ proxy_srvc_id ] );
  }

} /* qmi_proxy_txn_relref */


/*===========================================================================
  FUNCTION  qmi_proxy_dispatch_txn
===========================================================================*/
/*!
    @brief
    Dispatch a QMI Proxy transaction by calling QMI client API to send
    asychronous QMI request message.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_dispatch_txn
(
  qmi_proxy_txn_info_type *txn,
  qmi_proxy_conn_type     conn_type
)
{
  qmi_proxy_txn_entry_list_type *txn_list = NULL;
  uint32 txn_user_data = 0;
  qmi_proxy_client_info_type *client = NULL;
  qmi_client_type client_handle = NULL;
  void *resp_c_struct = NULL;
  int resp_c_struct_len = 0;
  qmi_txn_handle txn_handle = 0;
  qmi_proxy_txn_state_type state;
  uint8 i;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  /* Parameter check */
  if ( txn == NULL )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Fail to dispatch Txn Entry, null txn pointer" );

    return QMI_INTERNAL_ERR;
  }

  txn_list = &txn->txns[ conn_type ];
  if ( txn_list == NULL )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Fail to dispatch Txn entry, null txn list" );

    return QMI_INTERNAL_ERR;
  }

  /* Locate the QMI Proxy client info */
  client = qmi_proxy_client_find_and_addref( txn->proxy_srvc_id, txn->proxy_client_id );
  if ( client == NULL )
  {
    QMI_PROXY_ERR_MSG( "Fail to dispatch Txn Entry (Client ID %d, Txn ID %d, QMI %s(%d), RX Txn ID %d), client not registered\n",
                       txn->proxy_client_id,
                       QMI_PROXY_TXN_KEY_TO_TXN_ID( txn->proxy_txn_key ),
                       qmi_proxy_srvc_name [ txn->proxy_srvc_id ],
                       txn->proxy_srvc_id,
                       txn->rx_txn_id );

    return QMI_SERVICE_ERR;
  }

  /* Make sure the QMI Proxy transaction is not being aborted */
  if ( txn_list->state == QMI_PROXY_TXN_IN_PROGRESS )
  {
    if ( ( ( conn_type == QMI_PROXY_LOCAL_CONN_TYPE ) && ( txn->async_state == QMI_PROXY_ASYNC_LOCAL_TXN_ON_HOLD ) ) ||
         ( ( conn_type == QMI_PROXY_REMOTE_CONN_TYPE ) && ( txn->async_state == QMI_PROXY_ASYNC_REMOTE_TXN_ON_HOLD ) ) )
    {
      QMI_PROXY_DEBUG_MSG( "Hold %s Txn Entry dispatch till peer modem completed the txn (Client ID %d, Txn ID %d, QMI %s(%d), RX Txn ID %d, Async State %d)\n",
                         qmi_proxy_modem_name[ conn_type ],
                         txn->proxy_client_id,
                         QMI_PROXY_TXN_KEY_TO_TXN_ID( txn->proxy_txn_key ),
                         qmi_proxy_srvc_name [ txn->proxy_srvc_id ],
                         txn->proxy_srvc_id,
                         txn->rx_txn_id,
                         txn->async_state );
    }
    else
    {
      /* Send asychronous QMI request message to dispatch transaction to a modem */
      for ( i = 0; i < txn_list->num_txn && i < QMI_PROXY_MAX_SRVC_TXNS; i++ )
      {
        if ( txn_list->entry[ i ].state == QMI_PROXY_TXN_ENTRY_SETUP )
        {
          /* Lookup client handle */
          rc = qmi_proxy_lookup_client_handle( txn->proxy_srvc_id,
                                               client,
                                               txn_list->entry[ i ].srvc_id,
                                               conn_type,
                                               &client_handle );
          if ( rc != QMI_NO_ERR )
          {
            QMI_PROXY_ERR_MSG( "Fail to dispatch Txn Entry (Client ID %d, Txn ID %d, QMI %s(%d), RX Txn ID %d, Async State %d), client handle not found\n",
                               txn->proxy_client_id,
                               QMI_PROXY_TXN_KEY_TO_TXN_ID( txn->proxy_txn_key ),
                               qmi_proxy_srvc_name [ txn->proxy_srvc_id ],
                               txn->proxy_srvc_id,
                               txn->rx_txn_id,
                               txn->async_state );
          }
          else
          {
            /* Allocate response buffer */

            /* Extract the max message size from the service object */
            rc = qmi_idl_get_message_c_struct_len( qmi_proxy_srvc_obj_tbl[ txn_list->entry[ i ].srvc_id ],
                                                   QMI_IDL_RESPONSE,
                                                   txn_list->entry[ i ].msg_id,
                                                   (uint32_t *) &resp_c_struct_len );
            if ( rc == QMI_NO_ERR )
            {
              resp_c_struct = qmi_proxy_malloc( resp_c_struct_len );
              if ( resp_c_struct == NULL )
              {
                rc = QMI_INTERNAL_ERR;
              }
            }

            if ( rc != QMI_NO_ERR )
            {
              QMI_PROXY_ERR_MSG( "Fail to dispatch Txn Entry (Client ID %d, Txn ID %d, QMI %s(%d), RX Txn ID %d, Async State %d), msg buffer not allocated\n",
                                 txn->proxy_client_id,
                                 QMI_PROXY_TXN_KEY_TO_TXN_ID( txn->proxy_txn_key ),
                                 qmi_proxy_srvc_name [ txn->proxy_srvc_id ],
                                 txn->proxy_srvc_id,
                                 txn->rx_txn_id,
                                 txn->async_state );
            }
            else
            {
              QMI_PLATFORM_MUTEX_LOCK( &txn->mutex );
              state = txn->state;
              QMI_PLATFORM_MUTEX_UNLOCK( &txn->mutex );

              if ( state == QMI_PROXY_TXN_IN_PROGRESS )
              {
                txn_list->entry[ i ].state = QMI_PROXY_TXN_ENTRY_DISPATCHED;
                txn_user_data = QMI_PROXY_COMPOSE_TXN_USER_DATA( conn_type, i, txn->proxy_txn_key );

                rc = qmi_client_send_msg_async( client_handle,
                                                txn_list->entry[ i ].msg_id,
                                                txn_list->entry[ i ].msg,
                                                txn_list->entry[ i ].msg_len,
                                                resp_c_struct,
                                                resp_c_struct_len,
                                                qmi_proxy_complete_txn,
                                                (void *)(intptr_t) txn_user_data,
                                                &txn_handle );
                if ( rc != QMI_NO_ERR )
                {
                  QMI_PROXY_DEBUG_MSG( "Fail to dispatch Txn Entry (%s(%d), Txn key 0x%x, Txn ID %d, Txn Index %d, QMI %s(%d), Msg ID 0x%x, Client ID %d, RX Txn ID %d, Async State %d, rc %d)\n",
                                       qmi_proxy_modem_name[ conn_type ],
                                       conn_type,
                                       txn->proxy_txn_key,
                                       QMI_PROXY_TXN_KEY_TO_TXN_ID( txn->proxy_txn_key ),
                                       i,
                                       qmi_proxy_srvc_name[ txn_list->entry[ i ].srvc_id ],
                                       txn_list->entry[ i ].srvc_id,
                                       txn_list->entry[ i ].msg_id,
                                       client->proxy_client_id,
                                       txn->rx_txn_id,
                                       txn->async_state,
                                       rc );

                    /* Free buffer */
                    qmi_proxy_free( (void **)&resp_c_struct );
                }
                else
                {
                  QMI_PROXY_DEBUG_MSG( "Txn Entry Dispatched (%s(%d), Txn key 0x%x, Txn ID %d, Txn Index %d, QMI %s(%d), Msg ID 0x%x, Client ID %d, RX Txn ID %d, Async State %d)\n",
                                       qmi_proxy_modem_name[ conn_type ],
                                       conn_type,
                                       txn->proxy_txn_key,
                                       QMI_PROXY_TXN_KEY_TO_TXN_ID( txn->proxy_txn_key ),
                                       i,
                                       qmi_proxy_srvc_name[ txn_list->entry[ i ].srvc_id ],
                                       txn_list->entry[ i ].srvc_id,
                                       txn_list->entry[ i ].msg_id,
                                       client->proxy_client_id,
                                       txn->rx_txn_id,
                                       txn->async_state );
                }
              }
              else
              {
                QMI_PROXY_DEBUG_MSG("Invalid transaction state %u, free the response structure\n", state);

                /* Free buffer */
                qmi_proxy_free( (void **)&resp_c_struct );
              }
            }
          }

          break;
        }
      } /* end for */
    }
  }

  /* Release reference to QMI Proxy Client Info */
  qmi_proxy_client_relref( &client );

  return rc;

} /* qmi_proxy_dispatch_txn */


/*===========================================================================
  FUNCTION  qmi_proxy_dispatch_txn_on_hold
===========================================================================*/
/*!
    @brief
    Dispatch QMI Proxy transaction that is put on hold because of waiting on
    QMI indication.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_dispatch_txn_on_hold
(
  qmi_proxy_conn_type proxy_conn_type,
  qmi_service_id_type proxy_srvc_id,
  uint32 user_data
)
{
  qmi_proxy_txn_info_type *txn = NULL;
  int rc = QMI_INTERNAL_ERR;

  /*-----------------------------------------------------------------------*/

  /* Get the transaction on hold */
  txn = qmi_proxy_txn_find_and_addref( proxy_conn_type, proxy_srvc_id, FALSE, 0, TRUE, user_data, TRUE );
  if ( txn != NULL )
  {
    rc = qmi_proxy_dispatch_txn( txn, QMI_PROXY_PEER_CONN_TYPE( proxy_conn_type ) );

    /* Release the reference to transaction */
    qmi_proxy_txn_relref( &txn, FALSE, TRUE );
  }

  return rc;
} /* qmi_proxy_dispatch_txn_on_hold */


/*=========================================================================
  FUNCTION:  qmi_proxy_setup_and_dispatch_txn

===========================================================================*/
/*!
    @brief
    Setup the transaction record for a QMI Proxy Request and dispatch
    the next available transaction to modem(s).

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_setup_and_dispatch_txn
(
  qmi_service_id_type            proxy_srvc_id,
  qmi_client_id_type             proxy_client_id,
  qmi_connection_id_type         proxy_conn_id,
  unsigned long                  proxy_msg_id,
  qmi_proxy_txn_entry_list_type  *local_txn_list,
  qmi_proxy_txn_entry_list_type  *remote_txn_list,
  unsigned long                  rx_txn_id,
  qmi_proxy_async_state_type     async_state,
  boolean                        user_data_valid,
  uint32                         user_data
)
{
  qmi_proxy_txn_info_type *txn = NULL;
  qmi_proxy_conn_type conn_type;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  /* Parameter check */
  if ( ( local_txn_list == NULL ) || ( remote_txn_list == NULL ) )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Fail to setup and dispatch txn, null Txn list pointer\n" );

    return QMI_INTERNAL_ERR;
  }

  /* Setup the QMI Proxy Transaction */
  txn = qmi_proxy_alloc_txn_data( proxy_srvc_id,
                                  proxy_client_id,
                                  proxy_conn_id,
                                  proxy_msg_id,
                                  local_txn_list,
                                  remote_txn_list,
                                  rx_txn_id,
                                  async_state,
                                  user_data_valid,
                                  user_data );

  if ( txn != NULL  )
  {
    /* Dispatch the first available local QMI service transaction */
    if ( local_txn_list->num_txn > 0 )
    {
      conn_type = QMI_PROXY_LOCAL_CONN_TYPE;
      rc = qmi_proxy_dispatch_txn( txn, conn_type );
    }

    if ( rc == QMI_NO_ERR  )
    {
      /* Dispatch the first available remote QMI service transaction */
      if ( remote_txn_list->num_txn > 0 )
      {
        conn_type = QMI_PROXY_REMOTE_CONN_TYPE;
        rc = qmi_proxy_dispatch_txn( txn, conn_type );
      }
    }

    if ( rc != QMI_NO_ERR )
    {
      QMI_PROXY_DEBUG_MSG( "Abort Txn (Client ID %d, Txn ID %d, Service ID %s(%d), RX Txn ID %d)\n",
                           txn->proxy_client_id,
                           QMI_PROXY_TXN_KEY_TO_TXN_ID( txn->proxy_txn_key ),
                           qmi_proxy_srvc_name[ txn->proxy_srvc_id ],
                           txn->proxy_srvc_id,
                           txn->rx_txn_id );


      /* Failure, need to send response */
      if ( rc != QMI_NO_ERR )
      {
        /* Send the QMI Proxy Service Response Message (Empty) to QMUX */
        qmi_proxy_srvc_send_qmi_response( proxy_srvc_id,
                                          proxy_client_id,
                                          proxy_conn_id,
                                          conn_type,
                                          proxy_msg_id,
                                          NULL,
                                          0,
                                          rx_txn_id );
      }
    }

    /* Release reference to transaction */
    qmi_proxy_txn_relref( &txn, rc, TRUE );
  }

  return rc;

} /* qmi_proxy_setup_and_dispatch_txn */


/*=========================================================================
  FUNCTION:  qmi_proxy_srvc_response_update_Cache

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per QMI service response message

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_srvc_response_update_cache
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *modified_resp,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  if ( QMI_PROXY_SVLTE_TARGET() )
  {
    switch ( proxy_srvc_id )
    {
      case QMI_DMS_SERVICE:
        rc = qmi_proxy_dms_srvc_response_update_cache( proxy_srvc_id,
                                                       proxy_client_id,
                                                       proxy_msg_id,
                                                       conn_type,
                                                       srvc_id,
                                                       msg_id,
                                                       resp_c_struct,
                                                       resp_c_struct_len,
                                                       last_txn,
                                                       user_data_valid,
                                                       user_data,
                                                       modified_resp,
                                                       modified_resp_c_struct,
                                                       modified_resp_c_struct_len );
        break;

      case QMI_NAS_SERVICE:
        rc = qmi_proxy_nas_srvc_response_update_cache( proxy_srvc_id,
                                                       proxy_client_id,
                                                       proxy_msg_id,
                                                       conn_type,
                                                       srvc_id,
                                                       msg_id,
                                                       resp_c_struct,
                                                       resp_c_struct_len,
                                                       last_txn,
                                                       user_data_valid,
                                                       user_data,
                                                       modified_resp,
                                                       modified_resp_c_struct,
                                                       modified_resp_c_struct_len );
        break;

      default:
        break;
    }
  }
  else if ( QMI_PROXY_SGLTE_TARGET() )
  {
    switch ( proxy_srvc_id )
    {
      case QMI_DMS_SERVICE:
        rc = qmi_proxy_dms_srvc_response_update_cache_sglte( proxy_srvc_id,
                                                       proxy_client_id,
                                                       proxy_msg_id,
                                                       conn_type,
                                                       srvc_id,
                                                       msg_id,
                                                       resp_c_struct,
                                                       resp_c_struct_len,
                                                       last_txn,
                                                       user_data_valid,
                                                       user_data,
                                                       modified_resp,
                                                       modified_resp_c_struct,
                                                       modified_resp_c_struct_len );
        break;

      case QMI_NAS_SERVICE:
        rc = qmi_proxy_nas_srvc_response_update_cache_sglte( proxy_srvc_id,
                                                       proxy_client_id,
                                                       proxy_msg_id,
                                                       conn_type,
                                                       srvc_id,
                                                       msg_id,
                                                       resp_c_struct,
                                                       resp_c_struct_len,
                                                       last_txn,
                                                       user_data_valid,
                                                       user_data,
                                                       modified_resp,
                                                       modified_resp_c_struct,
                                                       modified_resp_c_struct_len );
        break;

      case QMI_WMS_SERVICE:
        rc = qmi_proxy_wms_srvc_response_update_cache_sglte( proxy_srvc_id,
                                                       proxy_client_id,
                                                       proxy_msg_id,
                                                       conn_type,
                                                       srvc_id,
                                                       msg_id,
                                                       resp_c_struct,
                                                       resp_c_struct_len,
                                                       last_txn,
                                                       user_data_valid,
                                                       user_data,
                                                       modified_resp,
                                                       modified_resp_c_struct,
                                                       modified_resp_c_struct_len );
        break;

      default:
        break;
    }
  }

  return rc;

} /* qmi_proxy_srv_response_update_cache */


/*===========================================================================
  FUNCTION qmi_proxy_srvc_send_qmi_response
===========================================================================*/
/*!
    @brief
    Encode the QMI Service Response Message and Forward to QMI Proxy Client.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_srvc_send_qmi_response
(
  qmi_service_id_type    proxy_srvc_id,
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  void                   *resp_c_struct,
  int                    resp_c_struct_len,
  unsigned long          rx_txn_id
)
{
  unsigned char *encoded_payload = NULL, *encoded_msg = NULL;
  int encoded_payload_len = 0, encoded_msg_len = 0;
  int rc = QMI_NO_ERR;
  qmi_proxy_client_info_type *client_info;
  /*-----------------------------------------------------------------------*/

  /* Encode response */
  rc = qmi_proxy_encode_qmi_srvc_message( proxy_srvc_id,
                                          proxy_client_id,
                                          proxy_conn_type,
                                          proxy_srvc_id,
                                          QMI_IDL_RESPONSE,
                                          msg_id,
                                          resp_c_struct,
                                          resp_c_struct_len,
                                          &encoded_payload,
                                          &encoded_payload_len,
                                          &encoded_msg,
                                          &encoded_msg_len );

  if ( rc == QMI_NO_ERR )
  {
    /* Put on Message ID and Message Length header */
    qmi_proxy_write_std_srvc_msg_hdr( &encoded_msg,
                                      &encoded_msg_len,
                                      msg_id,
                                      encoded_msg_len );

    /* Put Transaction ID and Message Type header */
    qmi_proxy_write_std_txn_hdr( &encoded_msg,
                                 &encoded_msg_len,
                                 rx_txn_id,
                                 (unsigned char)QMI_PROXY_STD_SRVC_RESPONSE_CONTROL_FLAG );

    QMI_PROXY_DEBUG_MSG( "Sending QMI %s(%d), Response, Msg ID 0x%x to Client %d, Conn ID %d, RX Txn ID %d\n",
                         qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id, msg_id, proxy_client_id, proxy_conn_id, rx_txn_id  );

    client_info = qmi_proxy_get_client_info( proxy_srvc_id, proxy_client_id );

    if ( client_info != NULL )
    {
      QMI_PROXY_DEBUG_MSG( "Sending QMI SRVC response to QMI Proxy Client,  [fd=%d]\n", client_info->fd );
      qmi_proxy_send_response_to_clients( encoded_msg,
                               encoded_msg_len,
                               proxy_srvc_id,
                               proxy_client_id,
                               QMI_PROXY_SERVICE_CONTROL_FLAG,
                               client_info->fd);
    }
    else
    {
      QMI_PROXY_DEBUG_MSG( "can't find record for cliend_id=%d, srvc_id=%d, ignoring the srvc req for msg_id = %d\n", proxy_client_id, proxy_srvc_id, msg_id);
    }
   }

  /* Release buffer */
  qmi_proxy_free( (void **)&encoded_payload );

} /* qmi_proxy_srvc_send_qmi_response */


/*===========================================================================
  FUNCTION qmi_proxy_srvc_send_qmi_indication
===========================================================================*/
/*!
    @brief
    Encode the QMI Service Indication Message and Forward to QMI Proxy Client.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_srvc_send_qmi_indication
(
  qmi_service_id_type    proxy_srvc_id,
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  unsigned long          msg_id,
  void                   *ind_c_struct,
  int                    ind_c_struct_len
)
{
  unsigned char *encoded_payload = NULL, *encoded_msg = NULL;
  int encoded_payload_len = 0, encoded_msg_len = 0;
  int rc = QMI_NO_ERR;
  qmi_proxy_client_info_type *client_info = NULL;
  /*-----------------------------------------------------------------------*/

  /* Encode response */
  rc = qmi_proxy_encode_qmi_srvc_message( proxy_srvc_id,
                                          proxy_client_id,
                                          proxy_conn_type,
                                          proxy_srvc_id,
                                          QMI_IDL_INDICATION,
                                          msg_id,
                                          ind_c_struct,
                                          ind_c_struct_len,
                                          &encoded_payload,
                                          &encoded_payload_len,
                                          &encoded_msg,
                                          &encoded_msg_len );

  if ( rc == QMI_NO_ERR )
  {
    /* Put on Message ID and Message Length header */
    qmi_proxy_write_std_srvc_msg_hdr( &encoded_msg,
                                      &encoded_msg_len,
                                      msg_id,
                                      encoded_msg_len );

    /* Put Transaction ID and Message Type header */
    qmi_proxy_write_std_txn_hdr( &encoded_msg,
                                 &encoded_msg_len,
                                 0,
                                 (unsigned char)QMI_PROXY_STD_SRVC_INDICATION_CONTROL_FLAG );

    QMI_PROXY_DEBUG_MSG( "Sending QMI %s(%d), Indication, Msg ID 0x%x to Client %d, Conn ID %d\n",
                         qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id, msg_id, proxy_client_id, proxy_conn_id );

    /* Send QMI Proxy Service Indication message to QMUX */

    client_info = qmi_proxy_get_client_info( proxy_srvc_id, proxy_client_id );

    if ( client_info != NULL )
    {
      QMI_PROXY_DEBUG_MSG( "Sending QMI indication to QMI Proxy Client on [fd=%d]\n", client_info->fd );
      qmi_proxy_send_response_to_clients( encoded_msg,
                               encoded_msg_len,
                               proxy_srvc_id,
                               proxy_client_id,
                               QMI_PROXY_SERVICE_CONTROL_FLAG,
                               client_info->fd );
    }
    else
    {
      QMI_PROXY_DEBUG_MSG( "can't find record for cliend_id=%d, srvc_id=%d, ignoring the srvc req for msg_id = %d\n", proxy_client_id, proxy_srvc_id, msg_id);
    }
  }

  /* Release buffer */
  qmi_proxy_free( (void **)&encoded_payload );

} /* qmi_proxy_srvc_send_qmi_indication */


/*===========================================================================
  FUNCTION  qmi_proxy_complete_txn
===========================================================================*/
/*!
    @brief
    Handle the post-processing of a completed QMI service transaction.

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_complete_txn
(
  qmi_client_type       client_handle,
  unsigned long         msg_id,
  void                  *resp_c_struct,
  int                   resp_c_struct_len,
  void                  *resp_cb_data,
  qmi_client_error_type transp_err
)
{
  qmi_client_id_type proxy_client_id;
  qmi_service_id_type proxy_srvc_id;
  qmi_connection_id_type proxy_conn_id;
  unsigned long proxy_msg_id;
  uint32 proxy_txn_user_data;
  qmi_proxy_conn_type proxy_conn_type;
  uint8 proxy_txn_index;
  qmi_proxy_txn_key_type proxy_txn_key;
  unsigned long proxy_txn_id, proxy_rx_txn_id;
  qmi_proxy_txn_info_type *txn = NULL;
  boolean txns_done = FALSE, last_txn = FALSE, dispatch_next_txn = FALSE;
  qmi_proxy_txn_state_type txn_state = QMI_PROXY_TXN_IN_PROGRESS;
  boolean resp_modified = FALSE;
  void *modified_resp_c_struct = NULL;
  int modified_resp_c_struct_len = 0;
  void *proxy_resp_c_struct = NULL;
  int proxy_resp_c_struct_len = 0;
  int rc = QMI_INTERNAL_ERR;
  boolean dispatch_txn_on_hold = FALSE;

  /*-----------------------------------------------------------------------*/

  (void) client_handle;

  /* Sanity check */
  if ( resp_cb_data == NULL )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Fail to complete txn, NULL resp cb data" );

    return;
  }

  proxy_txn_user_data = (intptr_t) resp_cb_data;

  /* Extract Conn Type */
  proxy_conn_type = QMI_PROXY_TXN_USER_DATA_TO_CONN_TYPE( proxy_txn_user_data );

  /* Extract Txn Index */
  proxy_txn_index = QMI_PROXY_TXN_USER_DATA_TO_TXN_INDEX( proxy_txn_user_data );

  /* Extract Proxy Txn Key */
  proxy_txn_key = QMI_PROXY_TXN_USER_DATA_TO_TXN_KEY( proxy_txn_user_data );

  /* Extract Proxy Service ID */
  proxy_srvc_id = QMI_PROXY_TXN_KEY_TO_SRVC_ID( proxy_txn_key );

  /* Extract Txn ID */
  proxy_txn_id = QMI_PROXY_TXN_KEY_TO_TXN_ID( proxy_txn_key );

  /* validate conn_type, txn_index, srvc_id */
  if ( !QMI_PROXY_CONN_TYPE_IS_VALID( proxy_conn_type ) || !QMI_PROXY_TXN_INDEX_IS_VALID( proxy_txn_index ) ||
       !QMI_PROXY_SRVC_ID_IS_VALID ( proxy_srvc_id ) )
  {
    QMI_PROXY_ERR_MSG( "Invalid value for either conn_type = %d, txn_ind = %u or srvc_id = %d is received",
                       proxy_conn_type, proxy_txn_index, proxy_srvc_id );
    return;
  }

  QMI_PROXY_DEBUG_MSG( "%s ....... (%s(%d), Txn Key 0x%x, Txn ID %d, Txn Index %d, QMI %s(%d), Response, Msg ID 0x%x)\n",
                       __FUNCTION__,
                       qmi_proxy_modem_name[ proxy_conn_type ],
                       proxy_conn_type,
                       proxy_txn_key,
                       proxy_txn_id,
                       proxy_txn_index,
                       qmi_proxy_srvc_name[ proxy_srvc_id ],
                       proxy_srvc_id,
                       msg_id );

  /* Get the transaction.  If we can't find it, it is possible that it has been deleted due to
     transaction being aborted, so just return */
  txn = qmi_proxy_txn_find_and_addref( proxy_conn_type, proxy_srvc_id, TRUE, proxy_txn_key, FALSE, 0, FALSE);
  if ( txn == NULL )
  {
    QMI_PROXY_DEBUG_MSG( "Ignore, Txn not found, Txn Key 0x%x, Txn ID %d, QMI %s(%d)\n",
                         proxy_txn_key,
                         proxy_txn_id,
                         qmi_proxy_srvc_name[ proxy_srvc_id ],
                         proxy_srvc_id );

    /* Release buffers */
    qmi_proxy_free( (void **)&resp_c_struct );

    return;
  }

  if ( proxy_txn_index >= txn->txns[ proxy_conn_type ].num_txn )
  {
    QMI_PROXY_DEBUG_MSG( "Ignore, Txn Entry not found, Txn Key 0x%x Txn ID %d, Txn Index %d, QMI %s(%d)\n",
                         proxy_txn_key,
                         QMI_PROXY_TXN_KEY_TO_TXN_ID( proxy_txn_key ),
                         proxy_txn_index,
                         qmi_proxy_srvc_name[ proxy_srvc_id ],
                         proxy_srvc_id );

    /* Release the reference to transaction */
    qmi_proxy_txn_relref( &txn, FALSE, TRUE );

    /* Release buffers */
    qmi_proxy_free( (void **)&resp_c_struct );

    return;
  }

  proxy_client_id = txn->proxy_client_id;
  proxy_conn_id = txn->proxy_conn_id;
  proxy_msg_id = txn->proxy_msg_id;
  proxy_rx_txn_id = txn->rx_txn_id;

  /* Update transaction entry state */
  txn->txns[ proxy_conn_type ].entry[ proxy_txn_index ].state = QMI_PROXY_TXN_ENTRY_DONE;

  /* Transaction completed with error */
  if ( transp_err != QMI_NO_ERR )
  {
    QMI_PROXY_DEBUG_MSG( "Txn Entry Completed Failure (%s(%d), Txn key 0x%x, Txn ID %d, Txn Index %d, QMI %s(%d), Msg ID 0x%x, Client ID %d, RX Txn ID %d, Transp Err %d)\n",
                         qmi_proxy_modem_name[ proxy_conn_type ],
                         proxy_conn_type,
                         proxy_txn_key,
                         proxy_txn_id,
                         proxy_txn_index,
                         qmi_proxy_srvc_name[ txn->txns[ proxy_conn_type ]. entry[ proxy_txn_index ].srvc_id ],
                         txn->txns[ proxy_conn_type ].entry[ proxy_txn_index ].srvc_id,
                         txn->txns[ proxy_conn_type ].entry[ proxy_txn_index ].msg_id,
                         proxy_client_id,
                         proxy_rx_txn_id,
                         transp_err );

    QMI_PLATFORM_MUTEX_LOCK( &txn->mutex );
    /* Abort all transactions related to the same modem */
    txn->txns[ proxy_conn_type ].state = QMI_PROXY_TXN_COMPLETED_FAILURE;

    /* Abort the whole QMI Proxy transaction */
    if ( txn->state == QMI_PROXY_TXN_IN_PROGRESS )
    {
      txn->state = QMI_PROXY_TXN_COMPLETED_FAILURE;
      txns_done = TRUE;
    }

    txn_state = txn->state;

    QMI_PLATFORM_MUTEX_UNLOCK( &txn->mutex );

  }
  else
  {
    QMI_PLATFORM_MUTEX_LOCK( &txn->mutex );

    /* Last transaction for the modem */
    if ( proxy_txn_index == ( txn->txns[ proxy_conn_type ].num_txn - 1 ) )
    {
      /* All transactions of the modem completed with success */
      txn->txns[ proxy_conn_type ].state = QMI_PROXY_TXN_COMPLETED_SUCCESS;

      /* Check whether there is a need to unhold transaction */
      if ( ( ( proxy_conn_type == QMI_PROXY_LOCAL_CONN_TYPE ) && ( txn->async_state == QMI_PROXY_ASYNC_REMOTE_TXN_ON_HOLD ) ) ||
           ( ( proxy_conn_type == QMI_PROXY_REMOTE_CONN_TYPE ) && ( txn->async_state == QMI_PROXY_ASYNC_LOCAL_TXN_ON_HOLD ) ) )
      {
        if ( txn->async_state == QMI_PROXY_ASYNC_REMOTE_TXN_ON_HOLD )
        {
          txn->async_state = QMI_PROXY_ASYNC_REMOTE_TXN_WAIT_IND;
        }
        else
        {
          txn->async_state = QMI_PROXY_ASYNC_LOCAL_TXN_WAIT_IND;
        }

        if (txn->is_recvd_ind)
        {
          dispatch_txn_on_hold = TRUE;
        }

        QMI_PROXY_DEBUG_MSG( "Unhold Txn Entry Dispatch (%s(%d), Txn key 0x%x, Txn ID %d, QMI %s(%d), Client Id %d, RX Txn ID %d, Async state %d)\n",
                             qmi_proxy_modem_name[ QMI_PROXY_PEER_CONN_TYPE( proxy_conn_type ) ],
                             proxy_conn_type,
                             proxy_txn_key,
                             proxy_txn_id,
                             qmi_proxy_srvc_name [ txn->proxy_srvc_id ],
                             txn->proxy_srvc_id,
                             proxy_client_id,
                             proxy_rx_txn_id,
                             txn->async_state );
      }

      /* All transactions for the peer modem also completed with success */
      if ( ( txn->state == QMI_PROXY_TXN_IN_PROGRESS ) &&
           ( ( txn->txns[ QMI_PROXY_PEER_CONN_TYPE( proxy_conn_type ) ].state == QMI_PROXY_TXN_IDLE ) ||
             ( txn->txns[ QMI_PROXY_PEER_CONN_TYPE( proxy_conn_type ) ].state == QMI_PROXY_TXN_COMPLETED_SUCCESS ) )
         )
      {
        txn->state = QMI_PROXY_TXN_COMPLETED_SUCCESS;
        txns_done = TRUE;
        last_txn = TRUE;
      }
    }
    /* See if next transaction can be dispatched */
    else if ( txn->state == QMI_PROXY_TXN_IN_PROGRESS )
    {
      dispatch_next_txn = TRUE;
    }

    txn_state = txn->state;

    QMI_PLATFORM_MUTEX_UNLOCK( &txn->mutex );

    if (dispatch_txn_on_hold)
    {
      /* See if there is any on-hold transaction need to be dispatched */
      qmi_proxy_dispatch_txn( txn, QMI_PROXY_PEER_CONN_TYPE( proxy_conn_type ) );
    }

    /* Verify response message result */
    rc = qmi_proxy_srvc_response_update_cache( proxy_srvc_id,
                                               proxy_client_id,
                                               proxy_msg_id,
                                               proxy_conn_type,
                                               txn->txns[ proxy_conn_type ].entry[ proxy_txn_index ].srvc_id,
                                               msg_id,
                                               resp_c_struct,
                                               resp_c_struct_len,
                                               last_txn,
                                               txn->user_data_valid,
                                               txn->user_data,
                                               &resp_modified,
                                               &modified_resp_c_struct,
                                               &modified_resp_c_struct_len );
    if ( rc != QMI_NO_ERR )
    {
      QMI_PLATFORM_MUTEX_LOCK( &txn->mutex );

      /* Abort all transactions related to the same modem */
      txn->txns[ proxy_conn_type ].state = QMI_PROXY_TXN_COMPLETED_FAILURE;

      /* Abort the whole QMI Proxy transaction */
      if ( txn->state == QMI_PROXY_TXN_IN_PROGRESS )
      {
        txn->state = QMI_PROXY_TXN_COMPLETED_FAILURE;
        txns_done = TRUE;
      }

      dispatch_next_txn = FALSE;
      txn_state = txn->state;

      QMI_PLATFORM_MUTEX_UNLOCK( &txn->mutex );

      QMI_PROXY_DEBUG_MSG( "Txn Entry Completed Failure (%s(%d), Txn key 0x%x, Txn ID %d, Txn Index %d, QMI %s(%d), Msg ID 0x%x, Client Id %d, RX Txn ID %d, rc %d)\n",
                           qmi_proxy_modem_name[ proxy_conn_type ],
                           proxy_conn_type,
                           proxy_txn_key,
                           proxy_txn_id,
                           proxy_txn_index,
                           qmi_proxy_srvc_name[ txn->txns[ proxy_conn_type ].entry[ proxy_txn_index ].srvc_id ],
                           txn->txns[ proxy_conn_type ].entry[ proxy_txn_index ].srvc_id,
                           txn->txns[ proxy_conn_type ].entry[ proxy_txn_index ].msg_id,
                           proxy_client_id,
                           proxy_rx_txn_id,
                           rc );
    }
    else
    {
      QMI_PROXY_DEBUG_MSG( "Txn Entry Completed Success (%s(%d), Txn Key 0x%x, Txn ID %d, Txn Index %d, QMI %s(%d), Msg ID 0x%x, Client ID %d, RX Txn ID %d)\n",
                           qmi_proxy_modem_name[ proxy_conn_type ],
                           proxy_conn_type,
                           proxy_txn_key,
                           proxy_txn_id,
                           proxy_txn_index,
                           qmi_proxy_srvc_name[ txn->txns[ proxy_conn_type ].entry[ proxy_txn_index ].srvc_id ],
                           txn->txns[ proxy_conn_type ].entry[ proxy_txn_index ].srvc_id,
                           txn->txns[ proxy_conn_type ].entry[ proxy_txn_index ].msg_id,
                           proxy_client_id,
                           proxy_rx_txn_id );

      /* Dispatch next transaction entry */
      if ( dispatch_next_txn )
      {
        rc = qmi_proxy_dispatch_txn( txn, proxy_conn_type );
        if ( rc != QMI_NO_ERR )
        {
          QMI_PLATFORM_MUTEX_LOCK( &txn->mutex );

          /* Abort all transactions related to the same modem */
          txn->txns[ proxy_conn_type ].state = QMI_PROXY_TXN_COMPLETED_FAILURE;

          /* Abort the whole QMI Proxy transaction */
          if ( txn->state == QMI_PROXY_TXN_IN_PROGRESS )
          {
            txn->state = QMI_PROXY_TXN_COMPLETED_FAILURE;
            txns_done = TRUE;
          }

          txn_state = txn->state;

          QMI_PLATFORM_MUTEX_UNLOCK( &txn->mutex );
        }
      }
    }
  }

  /* Release the reference to transaction */
  qmi_proxy_txn_relref( &txn, txns_done, TRUE );

  /* All transactions done, send response back to QMI Proxy Client */
  if ( txns_done )
  {
    if ( txn_state == QMI_PROXY_TXN_COMPLETED_FAILURE )
    {
      QMI_PROXY_DEBUG_MSG( "Abort Txn (Txn Key 0x%x, Txn ID %d, QMI %s(%d), Client ID %d, RX Txn ID %d\n",
                           proxy_txn_key,
                           proxy_txn_id,
                           qmi_proxy_srvc_name[ proxy_srvc_id ],
                           proxy_srvc_id,
                           proxy_client_id,
                           proxy_rx_txn_id );
    }
    else if ( txn_state == QMI_PROXY_TXN_COMPLETED_SUCCESS )
    {
      QMI_PROXY_DEBUG_MSG( "All Txn Entries Completed Success (Txn Key 0x%x, Txn ID %d, QMI %s(%d), Client ID %d, RX Txn ID %d)\n",
                           proxy_txn_key,
                           proxy_txn_id,
                           qmi_proxy_srvc_name[ proxy_srvc_id ],
                           proxy_srvc_id,
                           proxy_client_id,
                           proxy_rx_txn_id );
    }
    else
    {
      QMI_PROXY_ERR_MSG( "Txn completed with odd state %d\n", txn_state );
    }

    /* Pack the response message for the QMI Proxy request */
    if ( resp_modified )
    {
      proxy_resp_c_struct = modified_resp_c_struct;
      proxy_resp_c_struct_len = modified_resp_c_struct_len;

    }
    else
    {
      proxy_resp_c_struct = resp_c_struct;
      proxy_resp_c_struct_len = resp_c_struct_len;
    }

    /* Send the QMI Proxy Service Response Message to QMUX */
    qmi_proxy_srvc_send_qmi_response( proxy_srvc_id,
                                      proxy_client_id,
                                      proxy_conn_id,
                                      proxy_conn_type,
                                      proxy_msg_id,
                                      proxy_resp_c_struct,
                                      proxy_resp_c_struct_len,
                                      proxy_rx_txn_id );

  }

  /* Release buffers */
  qmi_proxy_free( (void **)&resp_c_struct );

  qmi_proxy_free( (void **)&modified_resp_c_struct );

} /* qmi_proxy_complete_txn */


/*=========================================================================
  FUNCTION:  qmi_proxy_init_txn_entry_list

===========================================================================*/
/*!
    @brief
    Initialize the transaction entry list

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_init_txn_entry_list
(
  qmi_proxy_txn_entry_list_type *local_txn_entry_list,
  qmi_proxy_txn_entry_list_type *remote_txn_entry_list
)
{
  /*-----------------------------------------------------------------------*/

  if ( local_txn_entry_list != NULL )
  {
    memset( (void *) local_txn_entry_list, 0, sizeof( qmi_proxy_txn_entry_list_type ) );
  }

  if ( remote_txn_entry_list != NULL )
  {
    memset( (void *) remote_txn_entry_list, 0, sizeof( qmi_proxy_txn_entry_list_type ) );
  }

} /* qmi_proxy_init_txn_entry_list */


/*=========================================================================
  FUNCTION:  qmi_proxy_add_txn_entry

===========================================================================*/
/*!
    @brief
    Add entry to specified transaction entry list

    @return
    None
*/
/*=========================================================================*/
static void qmi_proxy_add_txn_entry
(
  qmi_proxy_txn_entry_list_type *txn_entry_list,
  qmi_service_id_type            srvc_id,
  unsigned long                  msg_id,
  void                           *msg,
  uint32_t                       msg_len
)
{
  uint8 i;

  /*-----------------------------------------------------------------------*/

  if ( txn_entry_list != NULL )
  {
    if ( txn_entry_list->num_txn < QMI_PROXY_MAX_SRVC_TXNS )
    {
      i = txn_entry_list->num_txn;
      txn_entry_list->entry[ i ].srvc_id = srvc_id;
      txn_entry_list->entry[ i ].msg_id = msg_id;
      txn_entry_list->entry[ i ].msg = msg;
      txn_entry_list->entry[ i ].msg_len = msg_len;
      txn_entry_list->num_txn += 1;
    }
    else
    {
      QMI_PROXY_ERR_MSG( "Number of transactions exceed maximum allowed %d\n", QMI_PROXY_MAX_SRVC_TXNS );
    }
  }

} /* qmi_proxy_add_txn_entry */

/*=========================================================================
  FUNCTION:  qmi_proxy_sglte_txn_list_from_modem

===========================================================================*/
/*!
    @brief
    Return local or remote transaction list depending on modem selected

    @return
    transaction list that corresponds to modem
*/
/*=========================================================================*/
static qmi_proxy_txn_entry_list_type *qmi_proxy_txn_list_from_modem
(
  qmi_proxy_conn_type modem_id,
  qmi_proxy_txn_entry_list_type *local_txn_entry_list,
  qmi_proxy_txn_entry_list_type *remote_txn_entry_list
)
{
  qmi_proxy_txn_entry_list_type *ret = NULL;

  if (modem_id == QMI_PROXY_LOCAL_CONN_TYPE) {
    ret = local_txn_entry_list;
  }
  else if (modem_id == QMI_PROXY_REMOTE_CONN_TYPE)
  {
    ret = remote_txn_entry_list;
  }

  return ret;
} /* qmi_proxy_sglte_txn_list_from_modem */
/*=========================================================================
  FUNCTION:  qmi_proxy_dms_srvc_arb_hdlr

===========================================================================*/
/*!
    @brief
    Perform the arbitration for QMI DMS service

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_dms_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
)
{
  dms_set_operating_mode_req_msg_v01 *dms_set_oprt_mode_req = NULL;
  dms_set_operating_mode_req_msg_v01 *remote_dms_set_oprt_mode_req = NULL;
  qmi_proxy_txn_entry_list_type local_txn_entry_list, remote_txn_entry_list;
  qmi_proxy_mode_pref_type svlte_mode_pref;
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  qmi_proxy_async_state_type async_state = QMI_PROXY_ASYNC_IDLE;
  boolean user_data_valid = FALSE;
  uint32 user_data = 0;
  int is_in_ftm;
  int is_local_online;
  int is_remote_online;
  int rc = QMI_NO_ERR;
  qmi_proxy_mode_pref_type sglte_mode_pref;
  boolean test_mode = qmi_proxy_sglte_is_test_mode;
  boolean changed_local = FALSE;
  boolean changed_remote = FALSE;
  qmi_proxy_conn_type cs_active;
  qmi_proxy_conn_type ps_active;

  /*-----------------------------------------------------------------------*/

  (void) client;

  QMI_PROXY_DEBUG_MSG( "%s ....... Msg ID 0x%x\n", __FUNCTION__, srvc_msg_id );

  /* Initialize transaction entry lists */
  qmi_proxy_init_txn_entry_list( &local_txn_entry_list, &remote_txn_entry_list );

  /* Perform service arbitration */
  switch( srvc_msg_id )
  {
    /* Set the operating mode of the device */
    case QMI_DMS_SET_OPERATING_MODE_REQ_V01:
      dms_set_oprt_mode_req = ( dms_set_operating_mode_req_msg_v01 *) decoded_payload;

      if ( QMI_PROXY_CSFB_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else if ( QMI_PROXY_SVLTE_TARGET() )
      {
        if ( ( dms_set_oprt_mode_req->operating_mode == DMS_OP_MODE_SHUTTING_DOWN_V01 ) ||
             ( dms_set_oprt_mode_req->operating_mode == DMS_OP_MODE_RESETTING_V01 ) )
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

          /* Request also applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
          svlte_mode_pref = qmi_proxy_internal_info.svlte_mode_pref;
          QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

          /* reset  the ssr operating mode */
          if( dms_set_oprt_mode_req->operating_mode == DMS_OP_MODE_ONLINE_V01 )
          {
            QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.ssr_info_mutex);
            qmi_proxy_internal_info.ssr_info.ignore_oprt_mode = FALSE;
            QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.ssr_info_mutex );
          }

          switch( svlte_mode_pref )
          {
            case QMI_PROXY_MODE_PREF_CDMA_ONLY:
            case QMI_PROXY_MODE_PREF_GSM_ONLY:
            case QMI_PROXY_MODE_PREF_WCDMA_ONLY:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED:
              /* Request applies to local modem processing */
              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
              break;

            case QMI_PROXY_MODE_PREF_EVDO_ONLY:
            case QMI_PROXY_MODE_PREF_LTE_ONLY:
              /* Request applies to remote modem processing */
              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
              break;

            default:
              if ( dms_set_oprt_mode_req->operating_mode == DMS_OP_MODE_ONLINE_V01 )
              {
                /* Hold remote transaction till local transaction is completed to make sure MRU/TOT taking effect */
                async_state = QMI_PROXY_ASYNC_REMOTE_TXN_ON_HOLD;
                user_data_valid = TRUE;
                user_data = DMS_OP_MODE_ONLINE_V01;
              }

              /* Request applies to local modem processing */
              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

              /* Request also applies to remote modem processing */
              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
              break;
          }
        }
      }
      else if( QMI_PROXY_SGLTE_TARGET() )
      {
        dms_set_oprt_mode_req = (dms_set_operating_mode_req_msg_v01 *) decoded_payload;
        remote_dms_set_oprt_mode_req = (dms_set_operating_mode_req_msg_v01 *) calloc(sizeof(dms_set_operating_mode_req_msg_v01), 1);
        sglte_mode_pref = qmi_proxy_internal_info.sglte_user_mode_pref;
        test_mode = qmi_proxy_sglte_is_test_mode;

        if (remote_dms_set_oprt_mode_req)
        {
          *remote_dms_set_oprt_mode_req = *dms_set_oprt_mode_req;

          is_in_ftm = ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode ==
                        DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) ||
                      ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode ==
                        DMS_OP_MODE_FACTORY_TEST_MODE_V01 );

          is_local_online = ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode ==
                              DMS_OP_MODE_ONLINE_V01 );

          is_remote_online = ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode ==
                              DMS_OP_MODE_ONLINE_V01 );

          qmi_proxy_get_cs_active_ps_active(&cs_active, &ps_active);

          qmi_proxy_internal_info.desired_mode = dms_set_oprt_mode_req->operating_mode;
          if ( !is_in_ftm )
          {
            if (dms_set_oprt_mode_req->operating_mode == DMS_OP_MODE_ONLINE_V01)
            {
              if ( cs_active == QMI_PROXY_LOCAL_CONN_TYPE ||
                   ps_active == QMI_PROXY_LOCAL_CONN_TYPE )
              {
                if ( !is_local_online )
                {
                  qmi_proxy_add_txn_entry( &local_txn_entry_list,
                                           QMI_DMS_SERVICE,
                                           srvc_msg_id,
                                           decoded_payload,
                                           decoded_payload_len );
                  changed_local = TRUE;
                }
                else
                {
                  QMI_PROXY_DEBUG_MSG("%s: Local Modem already online", __FUNCTION__);
                }
              }
              else
              {
                if ( is_local_online )
                {
                  QMI_PROXY_ERR_MSG("%s: Local modem shouldn't be online. Setting lpm.", __FUNCTION__);
                  dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_LOW_POWER_V01;
                  qmi_proxy_add_txn_entry( &local_txn_entry_list,
                                           QMI_DMS_SERVICE,
                                           srvc_msg_id,
                                           decoded_payload,
                                           decoded_payload_len );
                  changed_local = TRUE;
                }
              }

              if ( cs_active == QMI_PROXY_REMOTE_CONN_TYPE ||
                   ps_active == QMI_PROXY_REMOTE_CONN_TYPE )
              {
                if ( !is_remote_online )
                {
                  qmi_proxy_add_txn_entry( &remote_txn_entry_list,
                                           QMI_DMS_SERVICE,
                                           srvc_msg_id,
                                           remote_dms_set_oprt_mode_req,
                                           sizeof(*remote_dms_set_oprt_mode_req) );
                  changed_remote = TRUE;
                }
                else
                {
                  QMI_PROXY_DEBUG_MSG("%s: Remote Modem already online", __FUNCTION__);
                }
              }
              else
              {
                if ( is_remote_online )
                {
                  QMI_PROXY_ERR_MSG("%s: Remote modem shouldn't be online. Setting lpm.", __FUNCTION__);
                  remote_dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_LOW_POWER_V01;
                  qmi_proxy_add_txn_entry( &remote_txn_entry_list,
                                           QMI_DMS_SERVICE,
                                           srvc_msg_id,
                                           remote_dms_set_oprt_mode_req,
                                           sizeof(*remote_dms_set_oprt_mode_req) );
                  changed_remote = TRUE;
                }
              }

              QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.ssr_info_mutex);
              qmi_proxy_internal_info.ssr_info.ignore_oprt_mode = FALSE;
              QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.ssr_info_mutex );

              if ( changed_remote && changed_local )
              {
                /* Hold remote transaction till local transaction is completed to make sure MRU/TOT taking effect */
                async_state = QMI_PROXY_ASYNC_REMOTE_TXN_ON_HOLD;
                user_data_valid = TRUE;
                user_data = DMS_OP_MODE_ONLINE_V01;
              }
              //qmi_proxy_set_sglte_sys_sel_pref( &local_txn_entry_list, &remote_txn_entry_list );
            }
            else
            {
              qmi_proxy_add_txn_entry( &local_txn_entry_list,
                                       QMI_DMS_SERVICE,
                                       srvc_msg_id,
                                       decoded_payload,
                                       decoded_payload_len );

              qmi_proxy_add_txn_entry( &remote_txn_entry_list,
                                       QMI_DMS_SERVICE,
                                       srvc_msg_id,
                                       decoded_payload,
                                       decoded_payload_len );
              user_data_valid = TRUE;
              user_data       = DMS_OP_MODE_LOW_POWER_V01;
              qmi_proxy_sglte_sm_queue_event(
                        qmi_proxy_internal_info.sglte_sm,
                        SGLTE_SM_EVENT_LPM_REQUEST,
                        "SGLTE_SM_EVENT_LPM_REQUEST",
                        NULL);
            }
          }
        }
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    /* Reset ithe control point DMS state */
    case QMI_DMS_RESET_REQ_V01:
    /* Sets DMS event reporting for the control point */
    case QMI_DMS_SET_EVENT_REPORT_REQ_V01:
    /* Query device capabilities */
    case QMI_DMS_GET_DEVICE_CAP_REQ_V01:
    /* Query the current operating mode of the device */
    case QMI_DMS_GET_OPERATING_MODE_REQ_V01:
    /* Set user lock state */
    case QMI_DMS_SET_USER_LOCK_STATE_REQ_V01:
    /* Set user lock code */
    case QMI_DMS_SET_USER_LOCK_CODE_REQ_V01:
    /* Query the band capability of the device */
    case QMI_DMS_GET_BAND_CAPABILITY_REQ_V01:
     /* Set time on the device */
    case QMI_DMS_SET_TIME_REQ_V01:
      if ( QMI_PROXY_FUSION_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }

      if ( QMI_PROXY_SVLTE_TARGET() || QMI_PROXY_MULTIMODE_TARGET() || QMI_PROXY_SGLTE_TARGET() )
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    /* Enable or disable protection of UIM contens by a given PIN */
    case QMI_DMS_UIM_SET_PIN_PROTECTION_REQ_V01:
    /* Verify the PIN before accessing the UIM contents */
    case QMI_DMS_UIM_VERIFY_PIN_REQ_V01:
    /* Unblock a blocked PIN */
    case QMI_DMS_UIM_UNBLOCK_PIN_REQ_V01:
    /* Change the PIN value */
    case QMI_DMS_UIM_CHANGE_PIN_REQ_V01:
    /* Query the status of the PIN */
    case QMI_DMS_UIM_GET_PIN_STATUS_REQ_V01:
    /* Read the ICCID of the UIM for the device */
    case QMI_DMS_UIM_GET_ICCID_REQ_V01:
    /* Query the status of a UIM facility control key */
    case QMI_DMS_UIM_GET_CK_STATUS_REQ_V01:
    /* Set the protection of a UIM facility control key */
    case QMI_DMS_UIM_SET_CK_PROTECTION_REQ_V01:
    /* Unblocks a UIM facility control key */
    case QMI_DMS_UIM_UNBLOCK_CK_REQ_V01:
    /* Read the IMSI for the device */
    case QMI_DMS_UIM_GET_IMSI_REQ_V01:
    /* Query the state of the UIM */
    case QMI_DMS_UIM_GET_STATE_REQ_V01:
      if ( QMI_PROXY_FUSION_TARGET() && !QMI_PROXY_SGLTE_TARGET())
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    default:
      if ( QMI_PROXY_CSFB_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_DMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;
  }

  /* Setup and dispatch transaction */
  rc = qmi_proxy_setup_and_dispatch_txn( proxy_srvc_id,
                                         proxy_client_id,
                                         proxy_conn_id,
                                         srvc_msg_id,
                                         &local_txn_entry_list,
                                         &remote_txn_entry_list,
                                         rx_txn_id,
                                         async_state,
                                         user_data_valid,
                                         user_data );

  if (remote_dms_set_oprt_mode_req)
  {
      free(remote_dms_set_oprt_mode_req);
  }

  return rc;

} /* qmi_proxy_dms_srvc_arb_hdlr */


/*=========================================================================
  FUNCTION:  qmi_proxy_dms_srvc_response_update_cache_sglte

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per DMS service response message

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_dms_srvc_response_update_cache_sglte
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *modified_resp,
  void                **modified_resp_buf,
  int                 *modified_resp_buf_len
)
{
  dms_get_operating_mode_resp_msg_v01 *dms_get_oprt_mode_resp = NULL;
  dms_get_band_capability_resp_msg_v01 *dms_get_band_cap_resp = NULL;
  dms_get_device_cap_resp_msg_v01 *dms_get_dev_cap_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  uint8 i, j;
  int rc = QMI_NO_ERR;

  /* Supress compiler warnings for unused variables */
  (void) proxy_srvc_id;
  (void) proxy_client_id;
  (void) proxy_msg_id;
  (void) resp_c_struct_len;
  (void) modified_resp;
  (void) modified_resp_buf;
  (void) modified_resp_buf_len;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "%s ....... QMI %s(%d), %s(%d), Response, Msg ID 0x%x\n",
                       __FUNCTION__,
                       qmi_proxy_srvc_name[ srvc_id ],
                       srvc_id,
                       qmi_proxy_modem_name[ conn_type ],
                       conn_type,
                       msg_id );

  QMI_PROXY_DEBUG_MSG("%s ... local oprt mode = %d", __FUNCTION__, local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode);
  QMI_PROXY_DEBUG_MSG("%s ... remote oprt mode = %d", __FUNCTION__, remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode);
  switch(  msg_id )
  {
    case QMI_DMS_GET_DEVICE_CAP_RESP_V01:
      QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

      dms_get_dev_cap_resp = ( dms_get_device_cap_resp_msg_v01 *) resp_c_struct;

      /* Update cache if no error */
      if ( !QMI_PROXY_RESPONSE_SUCCESS( rc, dms_get_dev_cap_resp ) )
      {
        if ( dms_get_dev_cap_resp != NULL )
        {
          /* Update cache if no error */
          svlte_cache_ptr->dms_get_dev_cap_resp = *dms_get_dev_cap_resp;
        }

        if ( last_txn )
        {
          /* Compose the consolidated response */
          if ( QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() )
          {
            /* Obtain the data and SIM capabilities from remote modem */
            dms_get_dev_cap_resp->device_capabilities = remote_svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities;
            /* Obtain radio interface capabilities from both modems */
            for ( i = 0,
                  j = remote_svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities.radio_if_list_len;
                  i < local_svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities.radio_if_list_len;
                  i++, j++ )
            {
              dms_get_dev_cap_resp->device_capabilities.radio_if_list[ j ] =
                local_svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities.radio_if_list[ i ];
              dms_get_dev_cap_resp->device_capabilities.radio_if_list_len += 1;
            }
          }
          else
          {
            /* Local modem should be checked for device capabilities */
            dms_get_dev_cap_resp->device_capabilities = local_svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities;
          }
        }
      }

      QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
      break;

    case QMI_DMS_GET_OPERATING_MODE_RESP_V01:
      QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

      dms_get_oprt_mode_resp = ( dms_get_operating_mode_resp_msg_v01 *) resp_c_struct;

      if ( QMI_PROXY_RESPONSE_SUCCESS( rc, dms_get_oprt_mode_resp ) )
      {
        if ( dms_get_oprt_mode_resp != NULL )
        {
          /* Update cache if no error */
          svlte_cache_ptr->dms_get_oprt_mode_resp = *dms_get_oprt_mode_resp;

          /* Compose the consolidated response */
          if ( last_txn )
          {
            dms_get_oprt_mode_resp->operating_mode = qmi_proxy_get_cache_sglte_operating_mode();
          }
        }
      }

      QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
      break;

    case QMI_DMS_SET_OPERATING_MODE_RESP_V01:

      dms_get_oprt_mode_resp = (dms_get_operating_mode_resp_msg_v01*) resp_c_struct;

      if ( !QMI_PROXY_RESPONSE_SUCCESS( rc, dms_get_oprt_mode_resp ) )
      {
        if (user_data_valid && (DMS_OP_MODE_LOW_POWER_V01 == user_data))
        {
          qmi_proxy_sglte_sm_queue_event(
                    qmi_proxy_internal_info.sglte_sm,
                    SGLTE_SM_EVENT_LPM_FAIL,
                    "SGLTE_SM_EVENT_LPM_FAIL",
                    NULL);
        }
      }

      break;

    case QMI_DMS_GET_BAND_CAPABILITY_RESP_V01:
      QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

      dms_get_band_cap_resp = ( dms_get_band_capability_resp_msg_v01 *) resp_c_struct;

      /* Update cache if no error */
      if ( QMI_PROXY_RESPONSE_SUCCESS( rc, dms_get_band_cap_resp ) )
      {
        if ( dms_get_band_cap_resp != NULL )
        {
          /* Update cache if no error */
          svlte_cache_ptr->dms_get_band_cap_resp = *dms_get_band_cap_resp;

          if ( last_txn )
          {
            /* Compose the consolidated response. Reported the combined band capability from modems */
            dms_get_band_cap_resp->band_capability = local_svlte_cache_ptr->dms_get_band_cap_resp.band_capability |
                                                     remote_svlte_cache_ptr->dms_get_band_cap_resp.band_capability;
          }
        }
      }

      QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
      break;

    default:
      break;
  }

  return rc;

} /* qmi_proxy_dms_srvc_response_update_cache */


/*=========================================================================
  FUNCTION:  qmi_proxy_dms_srvc_response_update_cache

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per DMS service response message

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_dms_srvc_response_update_cache
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *modified_resp,
  void                **modified_resp_buf,
  int                 *modified_resp_buf_len
)
{
  dms_get_operating_mode_resp_msg_v01 *dms_get_oprt_mode_resp = NULL;
  dms_get_band_capability_resp_msg_v01 *dms_get_band_cap_resp = NULL;
  dms_get_device_cap_resp_msg_v01 *dms_get_dev_cap_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  uint8 i, j;
  int rc = QMI_NO_ERR;

  /* Supress compiler warnings for unused variables */
  (void) proxy_srvc_id;
  (void) proxy_client_id;
  (void) proxy_msg_id;
  (void) resp_c_struct_len;
  (void) user_data_valid;
  (void) user_data;
  (void) modified_resp;
  (void) modified_resp_buf;
  (void) modified_resp_buf_len;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "%s ....... QMI %s(%d), %s(%d), Response, Msg ID 0x%x\n",
                       __FUNCTION__,
                       qmi_proxy_srvc_name[ srvc_id ],
                       srvc_id,
                       qmi_proxy_modem_name[ conn_type ],
                       conn_type,
                       msg_id );

  QMI_PROXY_DEBUG_MSG("%s ... local oprt mode = %d", __FUNCTION__, local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode);
  QMI_PROXY_DEBUG_MSG("%s ... remote oprt mode = %d", __FUNCTION__, remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode);
  switch(  msg_id )
  {
    case QMI_DMS_GET_DEVICE_CAP_RESP_V01:
      QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

      dms_get_dev_cap_resp = ( dms_get_device_cap_resp_msg_v01 *) resp_c_struct;

      /* Update cache if no error */
      if ( !QMI_PROXY_RESPONSE_SUCCESS( rc, dms_get_dev_cap_resp ) )
      {
        if ( dms_get_dev_cap_resp != NULL )
        {
          /* Update cache if no error */
          svlte_cache_ptr->dms_get_dev_cap_resp = *dms_get_dev_cap_resp;
        }

        if ( last_txn )
        {
          /* Compose the consolidated response */
          switch ( qmi_proxy_internal_info.svlte_mode_pref )
          {
            /* Local modem should be checked for device capabilities */
            case QMI_PROXY_MODE_PREF_CDMA_ONLY:
            case QMI_PROXY_MODE_PREF_GSM_ONLY:
            case QMI_PROXY_MODE_PREF_WCDMA_ONLY:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED:
              dms_get_dev_cap_resp->device_capabilities = local_svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities;
              break;

            /* Remote modem should be checked for device capabilities */
            case QMI_PROXY_MODE_PREF_EVDO_ONLY:
            case QMI_PROXY_MODE_PREF_LTE_ONLY:
              dms_get_dev_cap_resp->device_capabilities = remote_svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities;
              break;

            /* Both modems should be checked for device capabilities */
            default:
              /* Obtain the data and SIM capabilities from remote modem */
              dms_get_dev_cap_resp->device_capabilities = remote_svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities;
              /* Obtain radio interface capabilities from both modems */
              for ( i = 0,
                    j = remote_svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities.radio_if_list_len;
                    i < local_svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities.radio_if_list_len;
                    i++, j++ )
              {
                dms_get_dev_cap_resp->device_capabilities.radio_if_list[ j ] =
                  local_svlte_cache_ptr->dms_get_dev_cap_resp.device_capabilities.radio_if_list[ i ];
                dms_get_dev_cap_resp->device_capabilities.radio_if_list_len += 1;
              }
              break;
          }
        }
      }

      QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
      break;

    case QMI_DMS_GET_OPERATING_MODE_RESP_V01:
      QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

      dms_get_oprt_mode_resp = ( dms_get_operating_mode_resp_msg_v01 *) resp_c_struct;

      if ( QMI_PROXY_RESPONSE_SUCCESS( rc, dms_get_oprt_mode_resp ) )
      {
        if ( dms_get_oprt_mode_resp != NULL )
        {
          /* Update cache if no error */
          svlte_cache_ptr->dms_get_oprt_mode_resp = *dms_get_oprt_mode_resp;

          /* Compose the consolidated response */
          if ( last_txn )
          {
            dms_get_oprt_mode_resp->operating_mode = qmi_proxy_get_cache_svlte_operating_mode();
          }
        }
      }

      QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
      break;

    case QMI_DMS_GET_BAND_CAPABILITY_RESP_V01:
      QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

      dms_get_band_cap_resp = ( dms_get_band_capability_resp_msg_v01 *) resp_c_struct;

      /* Update cache if no error */
      if ( QMI_PROXY_RESPONSE_SUCCESS( rc, dms_get_band_cap_resp ) )
      {
        if ( dms_get_band_cap_resp != NULL )
        {
          /* Update cache if no error */
          svlte_cache_ptr->dms_get_band_cap_resp = *dms_get_band_cap_resp;

          if ( last_txn )
          {
            /* Compose the consolidated response. Reported the combined band capability from modems */
            dms_get_band_cap_resp->band_capability = local_svlte_cache_ptr->dms_get_band_cap_resp.band_capability |
                                                     remote_svlte_cache_ptr->dms_get_band_cap_resp.band_capability;
          }
        }
      }

      QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
      break;

    default:
      break;
  }

  return rc;

} /* qmi_proxy_dms_srvc_response_update_cache */


/*=========================================================================
  FUNCTION:  qmi_proxy_dms_srvc_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Handle QMI DMS Service unsolicited callback indications.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_dms_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
)
{
  void *decoded_payload = NULL;
  uint32_t decoded_payload_len = 0;
  qmi_service_id_type proxy_srvc_id;
  qmi_client_id_type proxy_client_id;
  qmi_connection_id_type proxy_conn_id;
  qmi_proxy_conn_type proxy_conn_type;
  unsigned long   msg_id;
  dms_event_report_ind_msg_v01 *dms_evt_rpt_ind = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = NULL;
  int rc = QMI_NO_ERR;
  int dispatch_rc = QMI_NO_ERR;
  dms_operating_mode_enum_v01 original_oprt;
  dms_operating_mode_enum_v01 desired_oprt;

  /*-----------------------------------------------------------------------*/

  msg_id = rx_msg->msg_id;
  proxy_srvc_id = rx_msg->proxy_srvc_id;
  proxy_client_id = rx_msg->proxy_client_id;
  proxy_conn_type = rx_msg->proxy_conn_type;
  proxy_conn_id = rx_msg->proxy_conn_id;
  decoded_payload = rx_msg->decoded_payload;
  decoded_payload_len = rx_msg->decoded_payload_len;

  /* Set the cache pointer */
  svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ proxy_conn_type ];
  uint8_t (*get_cache_operating_mode)(void);
  qmi_proxy_svlte_cache_type *peer_svlte_cache_ptr =
          &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_PEER_CONN_TYPE(proxy_conn_id) ];
  dms_operating_mode_enum_v01 peer_oprt_mode, oprt_mode;

  if (QMI_PROXY_SVLTE_TARGET() )
  {
    get_cache_operating_mode = qmi_proxy_get_cache_svlte_operating_mode;
  }
  else if( QMI_PROXY_SGLTE_TARGET() )
  {
    get_cache_operating_mode = qmi_proxy_get_cache_sglte_operating_mode;
  }

  if ( QMI_PROXY_SVLTE_TARGET() || QMI_PROXY_SGLTE_TARGET() )
  {
    /* Process the unsolicted indication */
    switch(  msg_id )
    {
      case QMI_DMS_EVENT_REPORT_IND_V01:
        dms_evt_rpt_ind = ( dms_event_report_ind_msg_v01 *) decoded_payload;

        if ( dms_evt_rpt_ind->operating_mode_valid )
        {
          original_oprt = dms_evt_rpt_ind->operating_mode;
          desired_oprt = original_oprt;
          if (QMI_PROXY_SGLTE_TARGET())
          {
            desired_oprt = qmi_proxy_internal_info.desired_mode;
          }
          if ( DMS_OP_MODE_LOW_POWER_V01 == original_oprt )
          {
            qmi_proxy_sglte_invalidate_last_plmn(proxy_conn_type);
          }

          QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

          svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode = dms_evt_rpt_ind->operating_mode;

          /* Encode the consolidated indication message from modems */
          if ( proxy_client_id != QMI_PROXY_INTERNAL_CLIENT_ID )
          {
            dms_evt_rpt_ind->operating_mode = get_cache_operating_mode();
          }

          peer_oprt_mode = peer_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode;
          QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

          if ( QMI_PROXY_SGLTE_TARGET() )
          {

            //qmi_proxy_internal_info.desired_mode = get_cache_operating_mode();

            if (proxy_client_id != QMI_PROXY_INTERNAL_CLIENT_ID)
            {
                /* See there is any on-hold transaction need to be dispatched */
                dispatch_rc = qmi_proxy_dispatch_txn_on_hold( proxy_conn_type, proxy_srvc_id, desired_oprt );
                QMI_PROXY_DEBUG_MSG("%s: dispatch_rc: %d\n", __FUNCTION__, dispatch_rc);

              if (dispatch_rc == QMI_NO_ERR)
              {
                proxy_client_id = QMI_PROXY_INTERNAL_CLIENT_ID;
              }
              else
              {
                if (get_cache_operating_mode() == DMS_OP_MODE_ONLINE_V01)
                {
                  if ( DMS_OP_MODE_ONLINE_V01 == original_oprt )
                  {
                    qmi_proxy_sglte_sm_queue_event(
                            qmi_proxy_internal_info.sglte_sm,
                            SGLTE_SM_EVENT_ONLINE_IND,
                            "SGLTE_SM_EVENT_ONLINE_IND",
                            NULL);
                  }
                }
                else
                {
                  qmi_proxy_sglte_sm_queue_event(
                          qmi_proxy_internal_info.sglte_sm,
                          SGLTE_SM_EVENT_LPM_IND,
                          "SGLTE_SM_EVENT_LPM_IND",
                          NULL);
                }
              }
            }
            if (qmi_proxy_internal_info.number_of_emergency_calls &&
                    (QMI_PROXY_LOCAL_CONN_TYPE == proxy_conn_type) &&
                    QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() &&
                     QMI_PROXY_SGLTE_LOCAL_MODEM_PS_ACTIVE())
            {
              proxy_client_id = QMI_PROXY_INTERNAL_CLIENT_ID;
              break;
            }
          }
          else
          {
              /* See there is any on-hold transaction need to be dispatched */
              dispatch_rc = qmi_proxy_dispatch_txn_on_hold( proxy_conn_type, proxy_srvc_id, desired_oprt );
              QMI_PROXY_DEBUG_MSG("%s: dispatch_rc: %d\n", __FUNCTION__, dispatch_rc);
          }
        }
        break;

      default:
        break;
    }
  }

  /* Forward the QMI Service Indication to QMI Proxy Client */
  if ( ( proxy_client_id != QMI_PROXY_INTERNAL_CLIENT_ID ) && ( rc == QMI_NO_ERR ) )
  {
    qmi_proxy_srvc_send_qmi_indication( proxy_srvc_id,
                                        proxy_client_id,
                                        proxy_conn_id,
                                        proxy_conn_type,
                                        msg_id,
                                        decoded_payload,
                                        decoded_payload_len );
  }

  /* Release buffer */
  qmi_proxy_free( (void **)&decoded_payload );

} /* qmi_proxy_dms_srvc_unsol_ind_cb */


/*=========================================================================
  FUNCTION:  qmi_proxy_get_cs_active_ps_active_locked

===========================================================================*/
/*!
    @brief
    Find which modem should be active on cs and which on ps according
    to the cached information. Caller must own cache_mutex

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_get_cs_active_ps_active_locked(
  qmi_proxy_conn_type *cs_active_modem,
  qmi_proxy_conn_type *ps_active_modem
)
{
  if (!cs_active_modem || !ps_active_modem)
  {
    QMI_PROXY_DEBUG_MSG("%s: invalid arguments", __FUNCTION__);
    return;
  }

  *cs_active_modem = qmi_proxy_internal_info.cs_active_modem;
  *ps_active_modem = qmi_proxy_internal_info.ps_active_modem;
} /* qmi_proxy_get_cs_active_ps_active_locked */

/*=========================================================================
  FUNCTION:  qmi_proxy_get_cs_active_ps_active

===========================================================================*/
/*!
    @brief
    Find which modem should be active on cs and which on ps according
    to the cached information

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_get_cs_active_ps_active(
  qmi_proxy_conn_type *cs_active_modem,
  qmi_proxy_conn_type *ps_active_modem
)
{
  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
  qmi_proxy_get_cs_active_ps_active_locked(cs_active_modem, ps_active_modem);
  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
} /* qmi_proxy_get_cs_active_ps_active */

/*=========================================================================
  FUNCTION:  qmi_proxy_nas_srvc_arb_hdlr

===========================================================================*/
/*!
    @brief
    Perform the arbitration for QMI NAS service

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
int qmi_proxy_nas_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
)
{
  qmi_proxy_txn_entry_list_type local_txn_entry_list, remote_txn_entry_list;
  dms_set_operating_mode_req_msg_v01 *dms_set_oprt_mode_req = NULL, *remote_dms_set_oprt_mode_req = NULL;
  nas_set_system_selection_preference_req_msg_v01 *nas_set_sys_sel_pref_req = NULL, *remote_nas_set_sys_sel_pref_req = NULL;
  nas_set_preferred_networks_req_msg_v01 *local_nas_set_pref_net_req = NULL, *remote_nas_set_pref_net_req = NULL;
  nas_initiate_network_register_req_msg_v01 *nas_init_net_reg_req = NULL;
  nas_get_plmn_name_req_msg_v01 *nas_get_plmn_name_req = NULL;
  nas_perform_network_scan_req_msg_v01 *local_nas_perform_network_scan_req = NULL;
  nas_perform_network_scan_req_msg_v01 *remote_nas_perform_network_scan_req = NULL;
  nas_network_type_mask_type_v01 nas_network_type = 0;
  uint8_t nas_perform_network_scan_req_len = 0;
  qmi_proxy_mode_pref_type svlte_mode_pref;
  qmi_proxy_mode_pref_type sglte_user_mode_pref;
  qmi_proxy_async_state_type async_state = QMI_PROXY_ASYNC_IDLE;
  boolean user_data_valid = FALSE;
  uint32 user_data = 0, radio_if_mask = 0;
  qmi_proxy_conn_type cs_active_modem, ps_active_modem;
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  int is_in_ftm;
  int is_local_online;
  int is_remote_online;
  int num_of__voice_call_active;
  boolean is_cs_active_changed;
  int rc = QMI_NO_ERR;

  /* Supress compiler warnings for unused variables */
  (void) client;
  (void) proxy_client_id;
  (void) user_data_valid;
  (void) user_data;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "%s ....... Msg ID 0x%x\n", __FUNCTION__, srvc_msg_id );

  is_in_ftm = ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode ==
                DMS_OP_MODE_FACTORY_TEST_MODE_V01 ) ||
              ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode ==
                DMS_OP_MODE_FACTORY_TEST_MODE_V01 );
  is_local_online = ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode ==
                      DMS_OP_MODE_ONLINE_V01 );
  is_remote_online = ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode ==
                      DMS_OP_MODE_ONLINE_V01 );
  /* Initialize transaction entry lists */
  qmi_proxy_init_txn_entry_list( &local_txn_entry_list, &remote_txn_entry_list );

  /* Allocate buffer */
  remote_nas_set_sys_sel_pref_req = qmi_proxy_malloc( sizeof( nas_set_system_selection_preference_req_msg_v01 ) );
  if ( remote_nas_set_sys_sel_pref_req == NULL )
  {
    rc = QMI_INTERNAL_ERR;
  }
  else
  {
    local_nas_set_pref_net_req = qmi_proxy_malloc( sizeof( nas_set_preferred_networks_req_msg_v01 ) );
    if ( local_nas_set_pref_net_req == NULL )
    {
      rc = QMI_INTERNAL_ERR;
      qmi_proxy_free( (void **)&remote_nas_set_sys_sel_pref_req );
    }
    else
    {
      remote_nas_set_pref_net_req = qmi_proxy_malloc( sizeof( nas_set_preferred_networks_req_msg_v01 ) );
      if ( remote_nas_set_pref_net_req == NULL )
      {
        rc = QMI_INTERNAL_ERR;
        qmi_proxy_free( (void **)&remote_nas_set_sys_sel_pref_req );
        qmi_proxy_free( (void **)&local_nas_set_pref_net_req );
      }
      else
      {
        dms_set_oprt_mode_req = qmi_proxy_malloc( sizeof( dms_set_operating_mode_req_msg_v01 ) );
        if ( dms_set_oprt_mode_req == NULL )
        {
          rc = QMI_INTERNAL_ERR;
          qmi_proxy_free( (void **)&remote_nas_set_sys_sel_pref_req );
          qmi_proxy_free( (void **)&local_nas_set_pref_net_req );
          qmi_proxy_free( (void **)&remote_nas_set_pref_net_req );
        }
        else
        {
          remote_dms_set_oprt_mode_req = qmi_proxy_malloc( sizeof( dms_set_operating_mode_req_msg_v01 ) );
          if ( remote_dms_set_oprt_mode_req == NULL )
          {
            rc = QMI_INTERNAL_ERR;
            qmi_proxy_free( (void **)&remote_nas_set_sys_sel_pref_req );
            qmi_proxy_free( (void **)&local_nas_set_pref_net_req );
            qmi_proxy_free( (void **)&remote_nas_set_pref_net_req );
            qmi_proxy_free( (void **)&dms_set_oprt_mode_req );
          }
        }
      }
    }
  }

  if ( rc != QMI_NO_ERR )
  {
    return rc;
  }

  /* Perform the service arbitration */
  switch ( srvc_msg_id )
  {
    /* Set system selection preference of the device */
    case QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01:
      if ( QMI_PROXY_CSFB_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else if ( QMI_PROXY_SVLTE_TARGET() )
      {
        nas_set_sys_sel_pref_req = ( nas_set_system_selection_preference_req_msg_v01 *) decoded_payload;

        /* This is a request about emergency mode change */
        if ( nas_set_sys_sel_pref_req->emergency_mode_valid )
        {
          QMI_PROXY_DEBUG_MSG( "Request to change emergency mode to 0x%x\n", nas_set_sys_sel_pref_req->emergency_mode );

          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        /* This is a request for mode preference change */
        else if ( nas_set_sys_sel_pref_req->mode_pref_valid )
        {
          /* Check wheher the radio IF is supported by UE. Need to do the check before honor any mode preference request because of
             over-kill in operations of restoring old mode preference in case of radio IF not supported by fusion UE */
          QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
          radio_if_mask = qmi_proxy_internal_info.svlte_radio_if_mask;
          QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

          if ( !( radio_if_mask & nas_set_sys_sel_pref_req->mode_pref ) )
          {
            QMI_PROXY_ERR_MSG( "New mode pref 0x%x not supported by UE 0x%x\n", nas_set_sys_sel_pref_req->mode_pref, radio_if_mask );
            rc = QMI_SERVICE_ERR_OP_DEVICE_UNSUPPORTED;
          }
          else
          {
            /* Save mode preference setting to be used in transaction complete processing */
            user_data_valid = TRUE;
            user_data = nas_set_sys_sel_pref_req->mode_pref;

            /* Mode preference may apply to both modems. Save a copy for remote modem reference */
            memcpy( remote_nas_set_sys_sel_pref_req,
                    nas_set_sys_sel_pref_req,
                    sizeof( nas_set_system_selection_preference_req_msg_v01 ) );

            /* Mode preference change applies to local modem only */
            if ( ( nas_set_sys_sel_pref_req->mode_pref & QMI_PROXY_MODE_PREF_CDMA_GSM_UMTS_MASK ) &&
                 !( nas_set_sys_sel_pref_req->mode_pref & QMI_PROXY_MODE_PREF_EVDO_LTE_MASK ) )
            {
              QMI_PROXY_DEBUG_MSG( "Request to change local modem's mode pref to 0x%x\n", nas_set_sys_sel_pref_req->mode_pref );

              /* Local modem to Online */
              dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_ONLINE_V01;
              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_DMS_SERVICE, QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                                       (void *) dms_set_oprt_mode_req, sizeof( dms_set_operating_mode_req_msg_v01 ) );

              /* Local modem to mode preference change */
              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

              /* Remote modem to LPM */
              remote_dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_LOW_POWER_V01;
              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_DMS_SERVICE, QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                                       (void *) remote_dms_set_oprt_mode_req, sizeof( dms_set_operating_mode_req_msg_v01 ) );
            }
            /* Mode preference change applies to remote modem only */
            else if ( !( nas_set_sys_sel_pref_req->mode_pref & QMI_PROXY_MODE_PREF_CDMA_GSM_UMTS_MASK ) &&
                       ( nas_set_sys_sel_pref_req->mode_pref & QMI_PROXY_MODE_PREF_EVDO_LTE_MASK ) )
            {
              QMI_PROXY_DEBUG_MSG( "Request to change remote modem's mode pref to 0x%x\n", nas_set_sys_sel_pref_req->mode_pref );

              /* Local modem to LPM */
              dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_LOW_POWER_V01;
              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_DMS_SERVICE, QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                                       (void *) dms_set_oprt_mode_req, sizeof( dms_set_operating_mode_req_msg_v01 ) );

              /* Remote modem to Online */
              remote_dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_ONLINE_V01;
              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_DMS_SERVICE, QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                                       (void *) remote_dms_set_oprt_mode_req, sizeof( dms_set_operating_mode_req_msg_v01 ) );

              /* Remote modem to mode preference change */
              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
            }
            /* Mode preference change applies to both local and remote modems */
            else
            {
              QMI_PROXY_DEBUG_MSG( "Request to change local/remote modems' mode pref to 0x%x\n", nas_set_sys_sel_pref_req->mode_pref );

              /* Local modem to Online */
              dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_ONLINE_V01;
              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_DMS_SERVICE, QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                                       (void *) dms_set_oprt_mode_req, sizeof( dms_set_operating_mode_req_msg_v01 ) );

              /* Local modem to mode preference change */
              nas_set_sys_sel_pref_req->mode_pref = nas_set_sys_sel_pref_req->mode_pref & QMI_PROXY_MODE_PREF_CDMA_GSM_UMTS_MASK;
              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

              /* Remote modem to Online */
              remote_dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_ONLINE_V01;
              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_DMS_SERVICE, QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                                       (void *) remote_dms_set_oprt_mode_req, sizeof( dms_set_operating_mode_req_msg_v01 ) );

              /* Remote modem to mode preference change */
              remote_nas_set_sys_sel_pref_req->mode_pref =
                remote_nas_set_sys_sel_pref_req->mode_pref & QMI_PROXY_MODE_PREF_EVDO_LTE_MASK;
              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id,
                                       (void *) remote_nas_set_sys_sel_pref_req, decoded_payload_len );
            }
          }
        }
        /* Automatic or Manual Network Selection */
        else if ( nas_set_sys_sel_pref_req->net_sel_pref_valid )
        {
          QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
          svlte_mode_pref = qmi_proxy_internal_info.svlte_mode_pref;
          QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

          switch( svlte_mode_pref )
          {
            case QMI_PROXY_MODE_PREF_CDMA_ONLY:
            case QMI_PROXY_MODE_PREF_CDMA_EVDO:
            case QMI_PROXY_MODE_PREF_GSM_ONLY:
            case QMI_PROXY_MODE_PREF_WCDMA_ONLY:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_CDMA_EVDO:
              /* Request applies to local modem processing */
              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
              break;

            case QMI_PROXY_MODE_PREF_EVDO_ONLY:
            case QMI_PROXY_MODE_PREF_LTE_ONLY:
            case QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO:
              /* Request applies to remote modem processing */
              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
              break;

            case QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA:
              /* for manual registration, it is applicalbe only for MSM */
              if( nas_set_sys_sel_pref_req->net_sel_pref.net_sel_pref == 0x01 )
              {
                /* Request applies to local modem processing */
                qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
                break;
              }
              /* for automatic registration, fall through */

            default:
              /* Request applies to local modem processing */
              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

              /* Request applies to remote modem processing */
              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
              break;
          }
        }
        /* All other system selection preference change */
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else if ( QMI_PROXY_SGLTE_TARGET() )
      {
        nas_set_sys_sel_pref_req = ( nas_set_system_selection_preference_req_msg_v01 *) decoded_payload;


        qmi_proxy_get_cs_active_ps_active(&cs_active_modem, &ps_active_modem);

        QMI_PROXY_DEBUG_MSG("%s: cs_active_modem: %s(%d): ps_active_modem: %s(%d)\n",
                            __FUNCTION__,
                            qmi_proxy_modem_name[cs_active_modem],
                            cs_active_modem,
                            qmi_proxy_modem_name[ps_active_modem],
                            ps_active_modem);

        /* This is a request about emergency mode change */
        if ( nas_set_sys_sel_pref_req->emergency_mode_valid )
        {
          QMI_PROXY_DEBUG_MSG( "Request to change emergency mode to 0x%x\n", nas_set_sys_sel_pref_req->emergency_mode );

          if ( QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() )
          {
            qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
          }
          else
          {
            qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
          }
        }
        /* This is a request for mode preference change */
        else if ( nas_set_sys_sel_pref_req->mode_pref_valid )
        {
          /* Check wheher the radio IF is supported by UE. Need to do the check before honor any mode preference request because of
             over-kill in operations of restoring old mode preference in case of radio IF not supported by fusion UE */
          QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
          radio_if_mask = qmi_proxy_internal_info.svlte_radio_if_mask;
          QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

          if ( !( radio_if_mask & nas_set_sys_sel_pref_req->mode_pref ) )
          {
            QMI_PROXY_ERR_MSG( "New mode pref 0x%x not supported by UE 0x%x\n", nas_set_sys_sel_pref_req->mode_pref, radio_if_mask );
            rc = QMI_SERVICE_ERR_OP_DEVICE_UNSUPPORTED;
          }
          else if ( DMS_OP_MODE_LOW_POWER_V01 == qmi_proxy_internal_info.desired_mode )
          {
            /* Do nothing as we are in air plane mode */
          }
          else
          {
            /* Save mode preference setting to be used in transaction complete processing */
            user_data_valid = TRUE;
            user_data = nas_set_sys_sel_pref_req->mode_pref;

            /* Mode preference may apply to both modems. Save a copy for remote modem reference */
            memcpy( remote_nas_set_sys_sel_pref_req,
                    nas_set_sys_sel_pref_req,
                    sizeof( nas_set_system_selection_preference_req_msg_v01 ) );

            sglte_user_mode_pref = qmi_proxy_nas_mask_to_user_mode_pref(
                    nas_set_sys_sel_pref_req->mode_pref,
                    nas_set_sys_sel_pref_req->gw_acq_order_pref);

            /* Return error if cs active modem is going to be changed and
               voice call is active on the same */
            QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
            is_cs_active_changed      = qmi_proxy_is_cs_active_changed(sglte_user_mode_pref);
            num_of__voice_call_active = qmi_proxy_internal_info.number_of_calls;
            QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

            if (is_cs_active_changed && num_of__voice_call_active)
            {
                QMI_PROXY_DEBUG_MSG("%s: Reject mode preference change as it "
                                    "causes CS to move from voice call active modem to the other. \n",
                                    __FUNCTION__);
                rc = QMI_SERVICE_ERR_INTERNAL;
            }
            else
            {
              if ( QMI_NO_ERR == qmi_proxy_set_sys_sel_pref_per_sglte_mode_pref(
                      sglte_user_mode_pref,
                      qmi_proxy_internal_info.is_in_sglte_coverage,
                      TRUE,
                      nas_set_sys_sel_pref_req,
                      remote_nas_set_sys_sel_pref_req) )
              {

                QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
                if (qmi_proxy_internal_info.sglte_user_mode_pref != sglte_user_mode_pref)
                {
                  if (sglte_user_mode_pref == QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA)
                  {
                    qmi_proxy_sglte_sm_queue_event(qmi_proxy_internal_info.sglte_sm,
                            SGLTE_SM_EVENT_SGLTE_MODE,
                            "SGLTE_SM_EVENT_SGLTE_MODE",
                            NULL);
                  }
                  else
                  {
                    qmi_proxy_sglte_sm_queue_event(qmi_proxy_internal_info.sglte_sm,
                            SGLTE_SM_EVENT_NON_SGLTE_MODE,
                            "SGLTE_SM_EVENT_NON_SGLTE_MODE",
                            NULL);
                  }
                }
                qmi_proxy_internal_info.sglte_user_mode_pref = sglte_user_mode_pref;
                qmi_proxy_update_cs_active_ps_active_sglte();
                QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
                qmi_proxy_get_cs_active_ps_active(&cs_active_modem, &ps_active_modem);

                QMI_PROXY_DEBUG_MSG("%s: After setting mode: cs_active_modem: %s(%d): ps_active_modem: %s(%d)\n",
                                    __FUNCTION__,
                                    qmi_proxy_modem_name[cs_active_modem],
                                    cs_active_modem,
                                    qmi_proxy_modem_name[ps_active_modem],
                                    ps_active_modem);


                /* Mode preference change applies to local modem  */
                if ( cs_active_modem == QMI_PROXY_LOCAL_CONN_TYPE || ps_active_modem == QMI_PROXY_LOCAL_CONN_TYPE )
                {
                  /* Local modem to Online */
                  if ( !is_in_ftm &&
                       !is_local_online )
                  {
                    QMI_PROXY_DEBUG_MSG("%s: Setting local modem to online\n", __FUNCTION__);
                    dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_ONLINE_V01;

                    qmi_proxy_add_txn_entry(
                            &local_txn_entry_list,
                            QMI_DMS_SERVICE,
                            QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                            (void *) dms_set_oprt_mode_req,
                            sizeof( dms_set_operating_mode_req_msg_v01 ) );
                  }

                  qmi_proxy_add_txn_entry(
                          &local_txn_entry_list,
                          QMI_NAS_SERVICE,
                          QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                          (void *) nas_set_sys_sel_pref_req,
                          sizeof( nas_set_system_selection_preference_req_msg_v01 ) );

                }
                else
                {
                  /* Local modem to LPM */
                  //qmi_proxy_sglte_invalidate_last_plmn(QMI_PROXY_LOCAL_CONN_TYPE);
                  dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_LOW_POWER_V01;
                  if ( !is_in_ftm )
                  {
                    QMI_PROXY_DEBUG_MSG( "%s: Setting local modem to LPM\n", __FUNCTION__ );
                    qmi_proxy_add_txn_entry(
                            &local_txn_entry_list,
                            QMI_DMS_SERVICE,
                            QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                            (void *) dms_set_oprt_mode_req,
                            sizeof( dms_set_operating_mode_req_msg_v01 ) );
                  }
                }

                /* Mode preference change applies to remote modem */
                if ( cs_active_modem == QMI_PROXY_REMOTE_CONN_TYPE ||
                     ps_active_modem == QMI_PROXY_REMOTE_CONN_TYPE )
                {
                  /* Remote modem to Online */
                  if ( !is_in_ftm &&
                       !is_remote_online )
                  {
                    QMI_PROXY_DEBUG_MSG( "%s: Setting remote modem to online\n", __FUNCTION__);
                    remote_dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_ONLINE_V01;

                    qmi_proxy_add_txn_entry(
                            &remote_txn_entry_list,
                            QMI_DMS_SERVICE,
                            QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                            (void *) remote_dms_set_oprt_mode_req,
                            sizeof( dms_set_operating_mode_req_msg_v01 ) );
                  }

                  qmi_proxy_add_txn_entry(
                          &remote_txn_entry_list,
                          QMI_NAS_SERVICE,
                          QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                          (void *) remote_nas_set_sys_sel_pref_req,
                          sizeof( nas_set_system_selection_preference_req_msg_v01 ) );

                }
                else
                {
                  /* Remote modem to LPM */
                  remote_dms_set_oprt_mode_req->operating_mode = DMS_OP_MODE_LOW_POWER_V01;
                  //qmi_proxy_sglte_invalidate_last_plmn(QMI_PROXY_REMOTE_CONN_TYPE);
                  if ( !is_in_ftm )
                  {
                    QMI_PROXY_DEBUG_MSG( "%s: Setting remote modem to LPM\n", __FUNCTION__ );

                    qmi_proxy_add_txn_entry(
                            &remote_txn_entry_list,
                            QMI_DMS_SERVICE,
                            QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                            (void *) remote_dms_set_oprt_mode_req,
                            sizeof( dms_set_operating_mode_req_msg_v01 ) );
                  }
                }

                QMI_PROXY_DEBUG_MSG( "Request to change remote modem's mode pref to 0x%x sglte_user_mode_pref: %d)\n",
                        remote_nas_set_sys_sel_pref_req->mode_pref,
                        sglte_user_mode_pref );
              }
            }
          }
        }
        /* Automatic or Manual Network Selection */
        else if ( nas_set_sys_sel_pref_req->net_sel_pref_valid )
        {
          QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
          svlte_mode_pref = qmi_proxy_internal_info.svlte_mode_pref;
          cs_active_modem = qmi_proxy_internal_info.cs_active_modem;
          ps_active_modem = qmi_proxy_internal_info.ps_active_modem;
          QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

          if ( cs_active_modem == QMI_PROXY_LOCAL_CONN_TYPE ||
               ps_active_modem == QMI_PROXY_LOCAL_CONN_TYPE )
          {
            /* Request applies to local modem processing */
            qmi_proxy_add_txn_entry(
                    &local_txn_entry_list,
                    QMI_NAS_SERVICE,
                    srvc_msg_id,
                    decoded_payload,
                    decoded_payload_len );
          }

          if ( cs_active_modem == QMI_PROXY_REMOTE_CONN_TYPE ||
               ps_active_modem == QMI_PROXY_REMOTE_CONN_TYPE )
          {
            /* Request applies to remote modem processing */
            qmi_proxy_add_txn_entry(
                    &remote_txn_entry_list,
                    QMI_NAS_SERVICE,
                    srvc_msg_id,
                    decoded_payload,
                    decoded_payload_len );
          }
        }
        /* All other system selection preference change */
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry(
                  &local_txn_entry_list,
                  QMI_NAS_SERVICE,
                  srvc_msg_id,
                  decoded_payload,
                  decoded_payload_len );

          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry(
                  &remote_txn_entry_list,
                  QMI_NAS_SERVICE,
                  srvc_msg_id,
                  decoded_payload,
                  decoded_payload_len );
        }
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry(
                &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    /* Query system selection preference of the device */
    case QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01:
      if ( QMI_PROXY_CSFB_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else if ( QMI_PROXY_SVLTE_TARGET() || QMI_PROXY_SGLTE_TARGET() )
      {

        QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.ssr_info_mutex);

        /* during SSR ignore the operating mode as modem which is in SSR will be offline, this will  result incorrect mode pref setting,
             once dms sends online mode request , reset it */
        if( qmi_proxy_internal_info.ssr_info.ignore_oprt_mode )
        {
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* Need to check both operating mode and system selection preference of remote modem */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_DMS_SERVICE, QMI_DMS_GET_OPERATING_MODE_REQ_V01, NULL, 0 );
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

          /* Need to check both operating mode and system selection preference of local modem */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_DMS_SERVICE, QMI_DMS_GET_OPERATING_MODE_REQ_V01, NULL, 0 );
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }

        QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.ssr_info_mutex );
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    /* Reset the control point NAS state */
    case QMI_NAS_RESET_REQ_MSG_V01:
    /* Abort a previously issued NAS command */
    case QMI_NAS_ABORT_REQ_MSG_V01:
    /* Set NAS event reporting for the control point */
    case QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01:
    /* Register for QMI NAS indications */
    case QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01:
    /* Write access overload class */
    case QMI_NAS_SET_ACCOLC_REQ_MSG_V01:
    /* Set network device configuration */
    case QMI_NAS_SET_DEVICE_CONFIG_REQ_MSG_V01:
    /* Query radio band/channel information regarding the system currently providing service */
    case QMI_NAS_GET_RF_BAND_INFO_REQ_MSG_V01:
    /* Set 3GPP2 subscription info */
    case QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01:
    /* Set RTRE config */
    case QMI_NAS_SET_RTRE_CONFIG_REQ_MSG_V01:
    /* Set signal strength threshold */
    case QMI_NAS_CONFIG_SIG_INFO_REQ_MSG_V01:
    /* Query signal strength */
    case QMI_NAS_GET_SIG_INFO_REQ_MSG_V01:
    /* Query error rate information */
    case QMI_NAS_GET_ERR_RATE_REQ_MSG_V01:
      if ( QMI_PROXY_FUSION_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }

      if ( QMI_PROXY_SVLTE_TARGET() || QMI_PROXY_MULTIMODE_TARGET() || QMI_PROXY_SGLTE_TARGET() )
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    /* Attaches to a network domain */
    case QMI_NAS_INITIATE_ATTACH_REQ_MSG_V01:
      if ( QMI_PROXY_CSFB_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else if ( QMI_PROXY_SVLTE_TARGET() )
      {
          QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
          svlte_mode_pref = qmi_proxy_internal_info.svlte_mode_pref;
          QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

          switch( svlte_mode_pref )
          {
            case QMI_PROXY_MODE_PREF_CDMA_ONLY:
            case QMI_PROXY_MODE_PREF_GSM_ONLY:
            case QMI_PROXY_MODE_PREF_WCDMA_ONLY:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED:
              /* Request applies to local modem processing */
              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
              break;

            case QMI_PROXY_MODE_PREF_EVDO_ONLY:
            case QMI_PROXY_MODE_PREF_LTE_ONLY:
            case QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_CDMA_EVDO:
            case QMI_PROXY_MODE_PREF_CDMA_EVDO:
              /* Request applies to remote modem processing */
              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
              break;

            default:
              /* Request applies to remote modem processing */
              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
              break;
          }
      }
      else if ( QMI_PROXY_SGLTE_TARGET() )
       {
           QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
           svlte_mode_pref = qmi_proxy_internal_info.svlte_mode_pref;
           cs_active_modem = qmi_proxy_internal_info.cs_active_modem;
           ps_active_modem = qmi_proxy_internal_info.ps_active_modem;
           QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

           if ( cs_active_modem == QMI_PROXY_LOCAL_CONN_TYPE || ps_active_modem == QMI_PROXY_LOCAL_CONN_TYPE )
           {
             /* Request applies to local modem processing */
             qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
           }

           if ( cs_active_modem == QMI_PROXY_REMOTE_CONN_TYPE || ps_active_modem == QMI_PROXY_REMOTE_CONN_TYPE )
           {
               /* Request applies to remote modem processing */
               qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
           }
       }
      else
      {
         /* Request applies to local modem processing */
         qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    /* Read the list of preferred networks from the device */
    case QMI_NAS_GET_PREFERRED_NETWORKS_REQ_MSG_V01:
    /* Read the list fo forbidden networks from the device */
    case QMI_NAS_GET_FORBIDDEN_NETWORKS_REQ_MSG_V01:
    /* Write a list of forbidden networks to the device */
    case QMI_NAS_SET_FORBIDDEN_NETWORKS_REQ_MSG_V01:
    /* Retrieves the PLMN mode bit data from the Customer Service Profile (CSP) */
    case QMI_NAS_GET_CSP_PLMN_MODE_BIT_REQ_MSG_V01:
      if ( QMI_PROXY_CSFB_TARGET())
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    /* Query the last AN-AAA authentication request for the current 1xEV-DO session */
    case QMI_NAS_GET_AN_AAA_STATUS_REQ_MSG_V01:
      if ( QMI_PROXY_FUSION_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    /* Retrieve PLMN name */
    case QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01:
      if ( QMI_PROXY_SGLTE_TARGET() )
      {
        nas_get_plmn_name_req = decoded_payload;

        /* Check if request applies to remote modem processing */
        if ((nas_get_plmn_name_req->plmn.mcc ==
                              atoi(qmi_proxy_internal_info.last_sglte_plmn[QMI_PROXY_REMOTE_CONN_TYPE].mcc)) &&
            (nas_get_plmn_name_req->plmn.mnc ==
                              atoi(qmi_proxy_internal_info.last_sglte_plmn[QMI_PROXY_REMOTE_CONN_TYPE].mnc)) &&
            ((nas_get_plmn_name_req->plmn.mcc !=
                              atoi(qmi_proxy_internal_info.last_sglte_plmn[QMI_PROXY_LOCAL_CONN_TYPE].mcc)) ||
            (nas_get_plmn_name_req->plmn.mnc !=
                              atoi(qmi_proxy_internal_info.last_sglte_plmn[QMI_PROXY_LOCAL_CONN_TYPE].mnc))))
        {
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id,
                                  nas_get_plmn_name_req, decoded_payload_len );
          user_data = QMI_PROXY_REMOTE_CONN_TYPE;
        }

        /* Check if request applies to local modem processing */
        else
        {
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id,
                                                  nas_get_plmn_name_req, decoded_payload_len );
          user_data = QMI_PROXY_LOCAL_CONN_TYPE;
        }

        user_data_valid = TRUE;
        break;
      }

    /* Fall through in non sglte scenarios */

    /* Initiate a scan for visible networks */
    case QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01:
      if ( QMI_PROXY_CSFB_TARGET() || QMI_PROXY_SVLTE_1_TARGET() )
      {
        /* Remember which modem the request was sent that used later for txn complete processing */
        user_data_valid = TRUE;
        user_data = QMI_PROXY_REMOTE_CONN_TYPE;

        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else if ( QMI_PROXY_SVLTE_2_TARGET() )
      {
        QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
        svlte_mode_pref = qmi_proxy_internal_info.svlte_mode_pref;
        QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

        switch( svlte_mode_pref )
        {
          case QMI_PROXY_MODE_PREF_CDMA_ONLY:
          case QMI_PROXY_MODE_PREF_CDMA_EVDO:
          case QMI_PROXY_MODE_PREF_GSM_ONLY:
          case QMI_PROXY_MODE_PREF_WCDMA_ONLY:
          case QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO:
          case QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED:
          case QMI_PROXY_MODE_PREF_GSM_WCDMA_CDMA_EVDO:
            /* Remember which modem the request was sent that used later for txn complete processing */
            user_data_valid = TRUE;
            user_data = QMI_PROXY_LOCAL_CONN_TYPE;

            /* Request applies to local modem processing */
            qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
            break;

          case QMI_PROXY_MODE_PREF_EVDO_ONLY:
          case QMI_PROXY_MODE_PREF_LTE_ONLY:
          case QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO:
            /* Remember which modem the request was sent that used later for txn complete processing */
            user_data_valid = TRUE;
            user_data = QMI_PROXY_REMOTE_CONN_TYPE;

            /* Request applies to remote modem processing */
            qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
            break;

          case QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA:
            if ( srvc_msg_id == QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01 )
            {
              user_data_valid = TRUE;
              user_data = QMI_PROXY_LOCAL_CONN_TYPE;

              /* Request applies to local modem processing */
              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
              break;
            }
            /* for other type of srvc_msg_id fall through */

          default:
            /* Remember which modem the request was sent that used later for txn complete processing */
            user_data_valid = TRUE;
            user_data = QMI_PROXY_BOTH_CONN_TYPE;

            /* Request applies to local modem processing */
            qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

            /* Request applies to remote modem processing */
            qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
            break;
        }
      }
      else if ( QMI_PROXY_SGLTE_TARGET() )
      {
          /* Network scan may apply to both modems. Save a copy for remote modem reference */
          remote_nas_perform_network_scan_req =  qmi_proxy_malloc( sizeof( nas_perform_network_scan_req_msg_v01 ) );
          if ( remote_nas_perform_network_scan_req && decoded_payload)
          {
              memcpy( remote_nas_perform_network_scan_req, decoded_payload, decoded_payload_len );
          }

          local_nas_perform_network_scan_req = decoded_payload;
          nas_perform_network_scan_req_len = decoded_payload_len;
          if (!local_nas_perform_network_scan_req)
          {
            local_nas_perform_network_scan_req =  qmi_proxy_malloc( sizeof( nas_perform_network_scan_req_msg_v01 ) );
            nas_perform_network_scan_req_len = sizeof( nas_perform_network_scan_req_msg_v01 );
          }

          if (local_nas_perform_network_scan_req && remote_nas_perform_network_scan_req)
          {
            QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
            cs_active_modem = qmi_proxy_internal_info.cs_active_modem;
            ps_active_modem = qmi_proxy_internal_info.ps_active_modem;
            QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

            user_data = QMI_PROXY_MAX_CONN_TYPE;

            if ( cs_active_modem == QMI_PROXY_LOCAL_CONN_TYPE || ps_active_modem == QMI_PROXY_LOCAL_CONN_TYPE )
            {
              /* Query system selection preference local modem */
              rc = qmi_proxy_sync_sys_sel_pref( QMI_PROXY_LOCAL_CONN_TYPE );
              if ( rc ==  QMI_NO_ERR )
              {
                /* Request applies to local modem processing */
                nas_network_type = qmi_proxy_get_current_network_mode_for_scan(QMI_PROXY_LOCAL_CONN_TYPE);
              }

              if ( nas_network_type )
              {
                local_nas_perform_network_scan_req->network_type_valid = TRUE;
                local_nas_perform_network_scan_req->network_type = nas_network_type;
                nas_network_type = 0;
              }
              else if( !decoded_payload )
              {
                nas_perform_network_scan_req_len = decoded_payload_len;
              }

              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id,
                                       local_nas_perform_network_scan_req, nas_perform_network_scan_req_len);
              user_data = QMI_PROXY_LOCAL_CONN_TYPE;
              user_data_valid = TRUE;
            }

            if ( cs_active_modem == QMI_PROXY_REMOTE_CONN_TYPE || ps_active_modem == QMI_PROXY_REMOTE_CONN_TYPE )
            {
              /* Query system selection preference remote modem */
              rc = qmi_proxy_sync_sys_sel_pref( QMI_PROXY_REMOTE_CONN_TYPE );
              if ( rc ==  QMI_NO_ERR )
              {
                /* Request applies to remote modem processing */
                nas_network_type = qmi_proxy_get_current_network_mode_for_scan(QMI_PROXY_REMOTE_CONN_TYPE);
              }

              if ( nas_network_type )
              {
                remote_nas_perform_network_scan_req->network_type_valid = TRUE;
                remote_nas_perform_network_scan_req->network_type = nas_network_type;
                nas_perform_network_scan_req_len = sizeof( nas_perform_network_scan_req_msg_v01 );
              }
              else if( !decoded_payload )
              {
                nas_perform_network_scan_req_len = decoded_payload_len;
              }

              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id,
                                        remote_nas_perform_network_scan_req, nas_perform_network_scan_req_len);
              if ( user_data == QMI_PROXY_MAX_CONN_TYPE )
              {
                user_data = QMI_PROXY_REMOTE_CONN_TYPE;
              }
              else
              {
                user_data = QMI_PROXY_BOTH_CONN_TYPE;
              }
              user_data_valid = TRUE;
            }
          }
          else
          {
            rc = QMI_INTERNAL_ERR;
          }
      }
      else
      {
        /* Remember which modem the request was sent that used later for txn complete processing */
        user_data_valid = TRUE;
        user_data = QMI_PROXY_LOCAL_CONN_TYPE;

        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

        if ( QMI_PROXY_SGLTE_TARGET() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
          user_data = QMI_PROXY_BOTH_CONN_TYPE;
        }
      }
      break;

    /* Register to a network */
    case QMI_NAS_INITIATE_NETWORK_REGISTER_REQ_MSG_V01:
      if ( QMI_PROXY_CSFB_TARGET() || QMI_PROXY_SVLTE_1_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else if ( QMI_PROXY_SVLTE_2_TARGET() )
      {
        nas_init_net_reg_req = ( nas_initiate_network_register_req_msg_v01 * ) decoded_payload;
        if ( nas_init_net_reg_req->manual_network_register_info_valid )
        {
          /* LTE network */
          if ( nas_init_net_reg_req->manual_network_register_info.radio_access_technology & NAS_RADIO_IF_LTE_V01 )
          {
            /* Request applies to remote modem processing */
            qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
          }
          /* non-LTE network */
          else
          {
            /* Request applies to local modem processing */
            qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
          }
        }
        else
        {
          QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
          svlte_mode_pref = qmi_proxy_internal_info.svlte_mode_pref;
          QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

          switch( svlte_mode_pref )
          {
            case QMI_PROXY_MODE_PREF_CDMA_ONLY:
            case QMI_PROXY_MODE_PREF_CDMA_EVDO:
            case QMI_PROXY_MODE_PREF_GSM_ONLY:
            case QMI_PROXY_MODE_PREF_WCDMA_ONLY:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_CDMA_EVDO:
              /* Request applies to local modem processing */
              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
              break;

            case QMI_PROXY_MODE_PREF_EVDO_ONLY:
            case QMI_PROXY_MODE_PREF_LTE_ONLY:
            case QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO:
              /* Request applies to remote modem processing */
              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
              break;

            default:
              /* Request applies to local modem processing */
              qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

              /* Request applies to remote modem processing */
              qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
              break;
          }
        }
      }
      else if ( QMI_PROXY_SGLTE_TARGET() )
      {
          QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
          cs_active_modem = qmi_proxy_internal_info.cs_active_modem;
          ps_active_modem = qmi_proxy_internal_info.ps_active_modem;
          QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

          if ( cs_active_modem == QMI_PROXY_LOCAL_CONN_TYPE || ps_active_modem == QMI_PROXY_LOCAL_CONN_TYPE )
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
          }

          if ( cs_active_modem == QMI_PROXY_REMOTE_CONN_TYPE || ps_active_modem == QMI_PROXY_REMOTE_CONN_TYPE )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    /* Query system info */
    case QMI_NAS_GET_SYS_INFO_REQ_MSG_V01:
    /* wake up modem from deep sleep */
    case QMI_NAS_FORCE_NETWORK_SEARCH_REQ_MSG_V01:
      if ( QMI_PROXY_CSFB_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else if ( QMI_PROXY_SVLTE_TARGET() )
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else if ( QMI_PROXY_SGLTE_TARGET() )
      {
          QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
          cs_active_modem = qmi_proxy_internal_info.cs_active_modem;
          ps_active_modem = qmi_proxy_internal_info.ps_active_modem;
          QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

          user_data = QMI_PROXY_MAX_CONN_TYPE;

          if ( cs_active_modem == QMI_PROXY_LOCAL_CONN_TYPE || ps_active_modem == QMI_PROXY_LOCAL_CONN_TYPE )
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
          }

          if ( cs_active_modem == QMI_PROXY_REMOTE_CONN_TYPE || ps_active_modem == QMI_PROXY_REMOTE_CONN_TYPE )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    default:
      if ( QMI_PROXY_CSFB_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        if ( QMI_PROXY_SGLTE_TARGET() )
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_NAS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      break;
  }

  if ( rc == QMI_NO_ERR )
  {
    /* Setup and dispatch transaction */
    rc = qmi_proxy_setup_and_dispatch_txn( proxy_srvc_id,
                                           proxy_client_id,
                                           proxy_conn_id,
                                           srvc_msg_id,
                                           &local_txn_entry_list,
                                           &remote_txn_entry_list,
                                           rx_txn_id,
                                           async_state,
                                           user_data_valid,
                                           user_data );
  }

  /* Free buffers */
  if ( !decoded_payload )
  {
    if (local_nas_perform_network_scan_req)
    {
      qmi_proxy_free( (void **)&local_nas_perform_network_scan_req);
    }
  }

  if (remote_nas_perform_network_scan_req)
  {
    qmi_proxy_free( (void **)&remote_nas_perform_network_scan_req);
  }

  qmi_proxy_free( (void **)&remote_nas_set_sys_sel_pref_req );
  qmi_proxy_free( (void **)&local_nas_set_pref_net_req );
  qmi_proxy_free( (void **)&remote_nas_set_pref_net_req );
  qmi_proxy_free( (void **)&dms_set_oprt_mode_req );
  qmi_proxy_free( (void **)&remote_dms_set_oprt_mode_req );

  return rc;

} /* qmi_proxy_nas_srvc_arb_hdlr */


/*=========================================================================
  FUNCTION:  qmi_proxy_update_cache_svlte_mode_pref

===========================================================================*/
/*!
    @brief
    Update cached SVLTE mode preference based on user data.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_update_cache_svlte_mode_pref_from_user_data
(
  uint32 user_data
)
{
  boolean net_pref_valid = FALSE;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "Set SVLTE mode pref from user data 0x%x\n", user_data );

  if ( user_data == ( QMI_PROXY_MODE_PREF_MASK_CDMA | QMI_PROXY_MODE_PREF_MASK_EVDO ) )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_CDMA_EVDO;
  }
  else if ( user_data == ( QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_CDMA | QMI_PROXY_MODE_PREF_MASK_EVDO ) )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO;
  }
  else if ( user_data == ( QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_CDMA | QMI_PROXY_MODE_PREF_MASK_EVDO |
                           QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS ) )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO_GSM_WCDMA;
  }
  else if ( user_data == ( QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS | QMI_PROXY_MODE_PREF_MASK_CDMA |
                           QMI_PROXY_MODE_PREF_MASK_EVDO ) )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_GSM_WCDMA_CDMA_EVDO;
  }
  else if ( user_data == ( QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS ) )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA;
  }
  else if ( user_data == QMI_PROXY_MODE_PREF_MASK_CDMA )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_CDMA_ONLY;
  }
  else if ( user_data == QMI_PROXY_MODE_PREF_MASK_GSM )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_GSM_ONLY;
  }
  else if ( user_data == QMI_PROXY_MODE_PREF_MASK_UMTS )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_WCDMA_ONLY;
  }
  else if ( user_data == ( QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS ) )
  {
    /* Broken QMI interface - it is not able to support the differentation of QMI_PROXY_PREF_GSM_WCDMA_PREFERRED */
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO;
  }
  else if ( user_data == QMI_PROXY_MODE_PREF_MASK_EVDO )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_EVDO_ONLY;
  }
  else if ( user_data == QMI_PROXY_MODE_PREF_MASK_LTE )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_ONLY;
  }

  if ( net_pref_valid )
  {
    QMI_PROXY_DEBUG_MSG( "Set SVLTE mode pref = %s(%d) from user data\n",
                         qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.svlte_mode_pref ],
                         qmi_proxy_internal_info.svlte_mode_pref );

    /* Save SVLTE Network Preference Setting */
    qmi_proxy_save_svlte_cfg();
  }
  else
  {
    QMI_PROXY_ERR_MSG( "Invalid user data %d to update SVLTE mode pref\n", user_data );
  }

} /* qmi_proxy_update_cache_svlte_mode_pref_from_user_data */


/*=========================================================================
  FUNCTION:  qmi_proxy_update_cache_sglte_mode_pref

===========================================================================*/
/*!
    @brief
    Update cached SVLTE mode preference based on user data.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_update_cache_sglte_mode_pref_from_user_data
(
  uint32 user_data
)
{
  boolean net_pref_valid = FALSE;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "Set SGLTE mode pref from user data 0x%x\n", user_data );

  if ( user_data == ( QMI_PROXY_MODE_PREF_GSM_UMTS_LTE_MASK ) )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.sglte_user_mode_pref = QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA;
  }
  else if ( user_data == ( QMI_PROXY_MODE_PREF_GSM_UMTS_MASK ) )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.sglte_user_mode_pref = QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED;
  }
  else if ( user_data == ( QMI_PROXY_MODE_PREF_MASK_GSM ) )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.sglte_user_mode_pref = QMI_PROXY_MODE_PREF_GSM_ONLY;
  }
  else if ( user_data == ( QMI_PROXY_MODE_PREF_MASK_UMTS ) )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.sglte_user_mode_pref = QMI_PROXY_MODE_PREF_WCDMA_ONLY;
  }
  else if ( user_data == ( QMI_PROXY_MODE_PREF_MASK_LTE ) )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.sglte_user_mode_pref =  QMI_PROXY_MODE_PREF_LTE_ONLY;
  }
  else if ( user_data == ( QMI_PROXY_MODE_PREF_MASK_TDSCDMA ) )
  {
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.sglte_user_mode_pref =  QMI_PROXY_MODE_PREF_TDSCDMA_ONLY;
  }

  if ( net_pref_valid )
  {
    QMI_PROXY_DEBUG_MSG( "Set SGLTE mode pref = %s(%d) from user data\n",
                         qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.sglte_user_mode_pref ],
                         qmi_proxy_internal_info.sglte_user_mode_pref );

    /* Save SGLTE Network Preference Setting */
    qmi_proxy_save_sglte_user_mode_pref();
  }
  else
  {
    QMI_PROXY_ERR_MSG( "Invalid user data %d to update SGLTE mode pref\n", user_data );
  }

} /* qmi_proxy_update_cache_sglte_mode_pref_from_user_data */


/*=========================================================================
  FUNCTION:  qmi_proxy_get_current_network_mode_for_scan

===========================================================================*/
/*!
    @brief
    Get configured network types to be sent to perform network scan

    @return
    Configured Network types
*/
/*=========================================================================*/
static nas_network_type_mask_type_v01 qmi_proxy_get_current_network_mode_for_scan
(
  qmi_proxy_conn_type modem_index
)
{
  qmi_proxy_svlte_cache_type *svlte_cache_ptr;
  nas_network_type_mask_type_v01 nas_network_type = 0;

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[modem_index];
  if (svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref_valid)
  {
    if ( svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref & QMI_NAS_RAT_MODE_PREF_LTE_V01 )
    {
      nas_network_type |= NAS_NETWORK_TYPE_LTE_ONLY_V01;
    }

    if ( svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref & QMI_NAS_RAT_MODE_PREF_GSM_V01 )
    {
      nas_network_type |= NAS_NETWORK_TYPE_GSM_ONLY_V01;
    }

    if ( svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref & QMI_NAS_RAT_MODE_PREF_UMTS_V01 )
    {
      nas_network_type |= NAS_NETWORK_TYPE_WCDMA_ONLY_V01;
    }

    if ( svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref & QMI_NAS_RAT_MODE_PREF_TDSCDMA_V01 )
    {
      nas_network_type |= NAS_NETWORK_TYPE_TDSCDMA_ONLY_V01;
    }
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
  return nas_network_type;

} /* qmi_proxy_get_current_network_mode_for_scan */

/*=========================================================================
  FUNCTION:  qmi_proxy_update_cache_svlte_mode_pref_from_resp

===========================================================================*/
/*!
    @brief
    Update cached SVLTE mode preference from received QMI Service Response
    Message.

    @return
    SVLTE Mode Preference
*/
/*=========================================================================*/
static qmi_proxy_mode_pref_type qmi_proxy_update_cache_svlte_mode_pref_from_resp
(
  void
)
{
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  boolean net_pref_valid = FALSE;

  /*-----------------------------------------------------------------------*/

  /* Both modems are online */
  if ( ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_ONLINE_V01 ) &&
       ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_ONLINE_V01 ) )
  {
    if ( ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_CDMA ) &&
         ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_EVDO ) )
    {
      net_pref_valid = TRUE;
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_CDMA_EVDO;
    }
    else if ( ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_CDMA ) &&
              ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
                ( QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_EVDO ) ) )
    {
      net_pref_valid = TRUE;
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO;
    }
    else if ( ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
                ( QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS | QMI_PROXY_MODE_PREF_MASK_CDMA ) ) &&
              ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_EVDO ) )
    {
      net_pref_valid = TRUE;
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_GSM_WCDMA_CDMA_EVDO;
    }
    else if ( ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
                ( QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS | QMI_PROXY_MODE_PREF_MASK_CDMA ) ) &&
              ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
                ( QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_EVDO ) ) )
    {
      net_pref_valid = TRUE;
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO_GSM_WCDMA;
    }
    else if ( ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
                ( QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS ) ) &&
              ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_LTE ) )
    {
      net_pref_valid = TRUE;
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA;
    }
  }
  /* Only local modem stays online because of in emergency call or thermal state */
  else if ( ( qmi_proxy_internal_info.svlte_mode_pref == QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO ) &&
            ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_CDMA ) &&
            ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
              ( QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_EVDO ) ) )
  {
    QMI_PROXY_DEBUG_MSG( "%s\n", "SVLTE1, MDM not online" );
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO;
  }
  /* Only local modem stays online because of in emergency call or thermal state or in GW service */
  else if ( ( qmi_proxy_internal_info.svlte_mode_pref == QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO_GSM_WCDMA ) &&
            ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
              ( QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS | QMI_PROXY_MODE_PREF_MASK_CDMA ) ) &&
            ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
              ( QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_EVDO ) ) )
  {
    QMI_PROXY_DEBUG_MSG( "%s\n", "SVLTE2, MDM not online" );
    net_pref_valid = TRUE;
    qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO_GSM_WCDMA;
  }
  /* Only local modem is online */
  else if ( local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_ONLINE_V01 )
  {
    if ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_CDMA )
    {
      net_pref_valid = TRUE;
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_CDMA_ONLY;
    }
    else if ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_GSM )
    {
      net_pref_valid = TRUE;
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_GSM_ONLY;
    }
    else if ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_UMTS )
    {
      net_pref_valid = TRUE;
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_WCDMA_ONLY;
    }
    else if ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref ==
              ( QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS ) )
    {
      /* Broken QMI interface - it is not able to support the differentation of QMI_PROXY_PREF_GSM_WCDMA_PREFERRED */
      net_pref_valid = TRUE;
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO;
    }
  }
  /* Only remote modem is online */
  else if ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_ONLINE_V01 )
  {
    if ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_EVDO )
    {
      net_pref_valid = TRUE;
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_EVDO_ONLY;
    }
    else if ( remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref == QMI_PROXY_MODE_PREF_MASK_LTE )
    {
      net_pref_valid = TRUE;
      qmi_proxy_internal_info.svlte_mode_pref = QMI_PROXY_MODE_PREF_LTE_ONLY;
    }
  }

  if ( net_pref_valid )
  {
    QMI_PROXY_DEBUG_MSG( "Updated SVLTE mode pref = %s(%d) from resp\n",
                         qmi_proxy_svlte_mode_pref_name[ qmi_proxy_internal_info.svlte_mode_pref ],
                         qmi_proxy_internal_info.svlte_mode_pref );

    /* Save QMI Proxy SVLTE Network Preference setting */
    qmi_proxy_save_svlte_cfg();
  }
  else
  {
    QMI_PROXY_ERR_MSG( "Invalid local_oprt_mode = %d, local mode_pref = %d, remote_oprt_mode = %d, remote_mode_pref = %d from ind/resp to update SVLTE mode pref\n",
                       local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode,
                       local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref,
                       remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode,
                       remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref );
  }

  return qmi_proxy_internal_info.svlte_mode_pref;

} /* qmi_proxy_update_cache_svlte_mode_pref_from_resp */


/*=========================================================================
  FUNCTION:  qmi_proxy_nas_mask_to_user_mode_pref

===========================================================================*/
/*!
    @brief
    Convert a nas technology mask to qmi_proxy_mode_pref_type

    @return
    User mode pref corresponding to mask
*/
/*=========================================================================*/
static qmi_proxy_mode_pref_type qmi_proxy_nas_mask_to_user_mode_pref
(
  int mask,
  nas_gw_acq_order_pref_enum_type_v01 acq_order_pref
)
{
  qmi_proxy_mode_pref_type ret = QMI_PROXY_MODE_PREF_UNKNOWN;

  switch(mask)
  {
    case QMI_PROXY_MODE_PREF_MASK_LTE:
      ret = QMI_PROXY_MODE_PREF_LTE_ONLY;
      break;

    case QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_CDMA |
         QMI_PROXY_MODE_PREF_MASK_EVDO | QMI_PROXY_MODE_PREF_MASK_GSM |
         QMI_PROXY_MODE_PREF_MASK_UMTS:
      ret = QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO_GSM_WCDMA;
      break;

    case QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_GSM |
         QMI_PROXY_MODE_PREF_MASK_UMTS:
      ret = QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA;
      break;

    case QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_CDMA |
         QMI_PROXY_MODE_PREF_MASK_EVDO:
      ret = QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO;
      break;

    case QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS |
         QMI_PROXY_MODE_PREF_MASK_CDMA |
         QMI_PROXY_MODE_PREF_MASK_EVDO:
      ret = QMI_PROXY_MODE_PREF_GSM_WCDMA_CDMA_EVDO;
      break;

    case QMI_PROXY_MODE_PREF_MASK_EVDO:
      ret = QMI_PROXY_MODE_PREF_EVDO_ONLY;
      break;

    case QMI_PROXY_MODE_PREF_MASK_CDMA:
      ret = QMI_PROXY_MODE_PREF_CDMA_ONLY;
      break;

    case QMI_PROXY_MODE_PREF_MASK_CDMA | QMI_PROXY_MODE_PREF_MASK_EVDO:
      ret = QMI_PROXY_MODE_PREF_CDMA_EVDO;
      break;

    case QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS:
      if (acq_order_pref == NAS_GW_ACQ_ORDER_PREF_WCDMA_GSM_V01)
      {
          ret = QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED;
      }
      else
      {
          ret = QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO;
      }
      break;

    case QMI_PROXY_MODE_PREF_MASK_UMTS:
      ret = QMI_PROXY_MODE_PREF_WCDMA_ONLY;
      break;

    case QMI_PROXY_MODE_PREF_MASK_GSM:
      ret = QMI_PROXY_MODE_PREF_GSM_ONLY;
      break;

    case QMI_PROXY_MODE_PREF_MASK_TDSCDMA:
      ret = QMI_PROXY_MODE_PREF_TDSCDMA_ONLY;
      break;

    default:
      QMI_PROXY_ERR_MSG( "Unsupported QMI NAS mode_pref mask  = 0x%x,\n", mask);

  }

  return ret;
} /* qmi_proxy_nas_mask_to_user_mode_pref */


/*=========================================================================
  FUNCTION:  qmi_proxy_user_mode_pref_to_ril_nas_mask

===========================================================================*/
/*!
    @brief
    Convert a qmi_proxy_mode_pref_type to nas technology mask

    @return
    Mask corresponding to user_mode_pref
*/
/*=========================================================================*/
static int qmi_proxy_user_mode_pref_to_ril_nas_mask
(
  qmi_proxy_mode_pref_type mode_pref
)
{
  int ret = 0;

  switch(mode_pref)
  {
    case QMI_PROXY_MODE_PREF_LTE_ONLY:
      ret = QMI_PROXY_MODE_PREF_MASK_LTE;
      break;

    case QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO_GSM_WCDMA:
      ret = QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_CDMA | QMI_PROXY_MODE_PREF_MASK_EVDO |
         QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS;
      break;

    case QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA:
      ret = QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS;
      break;

    case QMI_PROXY_MODE_PREF_LTE_CDMA_EVDO:
      ret = QMI_PROXY_MODE_PREF_MASK_LTE | QMI_PROXY_MODE_PREF_MASK_CDMA | QMI_PROXY_MODE_PREF_MASK_EVDO;
      break;

    case QMI_PROXY_MODE_PREF_GSM_WCDMA_CDMA_EVDO:
      ret = QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS | QMI_PROXY_MODE_PREF_MASK_CDMA |
         QMI_PROXY_MODE_PREF_MASK_EVDO;
      break;

    case QMI_PROXY_MODE_PREF_EVDO_ONLY:
      ret = QMI_PROXY_MODE_PREF_MASK_EVDO;
      break;

    case QMI_PROXY_MODE_PREF_CDMA_ONLY:
      ret = QMI_PROXY_MODE_PREF_MASK_CDMA;
      break;

    case QMI_PROXY_MODE_PREF_CDMA_EVDO:
      ret = QMI_PROXY_MODE_PREF_MASK_CDMA | QMI_PROXY_MODE_PREF_MASK_EVDO;
      break;

    case QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED:
    case QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO:
      ret = QMI_PROXY_MODE_PREF_MASK_GSM | QMI_PROXY_MODE_PREF_MASK_UMTS;
      break;

    case QMI_PROXY_MODE_PREF_WCDMA_ONLY:
      ret = QMI_PROXY_MODE_PREF_MASK_UMTS;
      break;

    case QMI_PROXY_MODE_PREF_GSM_ONLY:
      ret = QMI_PROXY_MODE_PREF_MASK_GSM;
      break;

    case QMI_PROXY_MODE_PREF_TDSCDMA_ONLY:
      ret = QMI_PROXY_MODE_PREF_MASK_TDSCDMA;
      break;

    default:
      QMI_PROXY_ERR_MSG( "Unsupported QMI NAS mode_pref = %d,\n", mode_pref);

  }

  return ret;
} /* qmi_proxy_nas_mask_to_user_mode_pref */


/*=========================================================================
  FUNCTION:  qmi_proxy_update_cache_sglte_mode_pref_from_resp

===========================================================================*/
/*!
    @brief
    Update cached SGLTE mode preference from received QMI Service Response
    Message.

    @return
    SGLTE Mode Preference
*/
/*=========================================================================*/
static qmi_proxy_mode_pref_type qmi_proxy_update_cache_sglte_mode_pref_from_resp
(
  void
)
{
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  boolean net_pref_valid = FALSE;
  qmi_proxy_mode_pref_type mode_pref = QMI_PROXY_MODE_PREF_UNKNOWN;
  int mask = 0;
  nas_gw_acq_order_pref_enum_type_v01 gw_acq_pref = NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01;

  /*-----------------------------------------------------------------------*/

  /* Both modems are online */
  QMI_PROXY_DEBUG_MSG( "%s: Local modem CS_ACTIVE. mode_pref_valid: %d mode_pref: %x\n"
                       "Remote modem mode_pref_valid: %d mode_pref: %x\n",
                       __FUNCTION__,
                       local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref_valid,
                       local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref,
                       remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref_valid,
                       remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref );

  /* Obtain a combined mask containing mode pref for both modems */
  if ( local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref_valid )
  {
    mask = local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref;
    /* TODO: figure out how to handle acquisition pref */
  }

  if ( QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() &&
       remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref_valid )
  {
    mask |= remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref;
    /* TODO: figure out how to handle acquisition pref */
  }

  /* Obtain the mode pref corresponding to the combined mask */
  /* TODO: Add mode prefs for the possible expected combined masks */
  mode_pref = qmi_proxy_nas_mask_to_user_mode_pref( mask, gw_acq_pref );

  if ( mode_pref != QMI_PROXY_MODE_PREF_UNKNOWN )
  {

    /* Save QMI Proxy SGLTE Network Preference setting */
    qmi_proxy_save_sglte_user_mode_pref();
  }
  else
  {
    /* Being here means that modems are configured in an unsupported combination */
    QMI_PROXY_ERR_MSG( "Invalid local_oprt_mode = %d, local mode_pref = %d, remote_oprt_mode = %d, remote_mode_pref = %d from ind/resp to update SGLTE mode pref\n",
                       local_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode,
                       local_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref,
                       remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode,
                       remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref );
  }

  return qmi_proxy_internal_info.sglte_user_mode_pref;

} /* qmi_proxy_update_cache_sglte_mode_pref_from_resp */


/*=========================================================================
  FUNCTION:  qmi_proxy_wms_resp_proc_wms_get_indication_register

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal svlte cache per received
    QMI_WMS_GET_INDICATION_REGISTER_RESP

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_wms_resp_proc_wms_get_indication_register
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  wms_get_indication_register_resp_msg_v01 *wms_get_indication_register_resp      = NULL;
  wms_get_indication_register_resp_msg_v01 *wms_get_indication_register_resp_orig = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  /* Supress compiler warnings for unused variables */
  (void) proxy_client_id;
  (void) resp_c_struct_len;
  (void) user_data_valid;
  (void) user_data;

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  wms_get_indication_register_resp_orig = ( wms_get_indication_register_resp_msg_v01 *) resp_c_struct;

  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, wms_get_indication_register_resp ) )
  {
    memset( &svlte_cache_ptr->wms_get_indication_register_resp, 0, sizeof( wms_get_indication_register_resp_msg_v01 ) );

    if ( wms_get_indication_register_resp_orig != NULL )
    {
      svlte_cache_ptr->wms_get_indication_register_resp = *wms_get_indication_register_resp_orig;
    }
    else
    {
      /* Ideally response should not be NULL as it has mandatory field, keeping klockwork happy */
      QMI_PROXY_ERR_MSG( "Response is NULL for srvc_id = %d, msg_id = 0x%x\n", srvc_id, msg_id );
      return QMI_INTERNAL_ERR;
    }

    if ( last_txn )
    {
      /* Allocate buffer */
      rc = qmi_idl_get_message_c_struct_len( qmi_proxy_srvc_obj_tbl[ proxy_srvc_id ],
                                             QMI_IDL_RESPONSE,
                                             proxy_msg_id,
                                             (uint32_t *) modified_resp_c_struct_len );
      *modified_resp_c_struct = qmi_proxy_malloc( *modified_resp_c_struct_len );
      if ( *modified_resp_c_struct == NULL )
      {
        rc =  QMI_INTERNAL_ERR;
      }
      else
      {
        wms_get_indication_register_resp = ( wms_get_indication_register_resp_msg_v01 *) *modified_resp_c_struct;
        wms_get_indication_register_resp->resp = wms_get_indication_register_resp_orig->resp;
        *resp_modified = TRUE;

        if (local_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_layer_info_events_valid
             && remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_layer_info_events_valid)
        {
           wms_get_indication_register_resp->reg_transport_layer_info_events_valid = TRUE;
           wms_get_indication_register_resp->reg_transport_layer_info_events =
                 local_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_layer_info_events |
                     remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_layer_info_events;
        }
        else if (local_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_layer_info_events_valid)
        {
           wms_get_indication_register_resp->reg_transport_layer_info_events_valid = TRUE;
           wms_get_indication_register_resp->reg_transport_layer_info_events =
                 local_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_layer_info_events;
        }
        else if (remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_layer_info_events_valid)
        {
           wms_get_indication_register_resp->reg_transport_layer_info_events_valid = TRUE;
           wms_get_indication_register_resp->reg_transport_layer_info_events =
                 remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_layer_info_events;
        }
        else
        {
           wms_get_indication_register_resp->reg_transport_layer_info_events_valid = FALSE;
        }

        if (local_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_nw_reg_info_events_valid
             && remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_nw_reg_info_events_valid )
        {
           wms_get_indication_register_resp->reg_transport_nw_reg_info_events_valid = TRUE;
           wms_get_indication_register_resp->reg_transport_nw_reg_info_events       =
                 local_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_nw_reg_info_events |
                     remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_nw_reg_info_events;
        }
        else if ( local_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_nw_reg_info_events_valid )
        {
           wms_get_indication_register_resp->reg_transport_nw_reg_info_events_valid = TRUE;
           wms_get_indication_register_resp->reg_transport_nw_reg_info_events       =
                 local_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_nw_reg_info_events;
        }
        else if ( remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_nw_reg_info_events_valid )
        {
           wms_get_indication_register_resp->reg_transport_nw_reg_info_events_valid = TRUE;
           wms_get_indication_register_resp->reg_transport_nw_reg_info_events       =
                 remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_transport_nw_reg_info_events;
        }
        else
        {
           wms_get_indication_register_resp->reg_transport_nw_reg_info_events_valid = FALSE;
        }

        if (local_svlte_cache_ptr->wms_get_indication_register_resp.reg_call_status_info_events_valid
             && remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_call_status_info_events_valid)
        {
           wms_get_indication_register_resp->reg_call_status_info_events_valid = TRUE;
           wms_get_indication_register_resp->reg_call_status_info_events       =
                 local_svlte_cache_ptr->wms_get_indication_register_resp.reg_call_status_info_events |
                     remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_call_status_info_events;
        }
        else if (local_svlte_cache_ptr->wms_get_indication_register_resp.reg_call_status_info_events_valid)
        {
           wms_get_indication_register_resp->reg_call_status_info_events_valid = TRUE;
           wms_get_indication_register_resp->reg_call_status_info_events       =
                 local_svlte_cache_ptr->wms_get_indication_register_resp.reg_call_status_info_events;
        }
        else if (remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_call_status_info_events_valid)
        {
           wms_get_indication_register_resp->reg_call_status_info_events_valid = TRUE;
           wms_get_indication_register_resp->reg_call_status_info_events       =
                 remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_call_status_info_events;
        }
        else
        {
           wms_get_indication_register_resp->reg_call_status_info_events_valid = FALSE;
        }

        if (local_svlte_cache_ptr->wms_get_indication_register_resp.reg_service_ready_events_valid
             && remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_service_ready_events_valid)
        {
           wms_get_indication_register_resp->reg_service_ready_events_valid = TRUE;
           wms_get_indication_register_resp->reg_service_ready_events       =
                 local_svlte_cache_ptr->wms_get_indication_register_resp.reg_service_ready_events |
                     remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_service_ready_events;
        }
        else if (local_svlte_cache_ptr->wms_get_indication_register_resp.reg_service_ready_events_valid)
        {
           wms_get_indication_register_resp->reg_service_ready_events_valid = TRUE;
           wms_get_indication_register_resp->reg_service_ready_events =
                               local_svlte_cache_ptr->wms_get_indication_register_resp.reg_service_ready_events;
        }
        else if  (remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_service_ready_events_valid)
        {
           wms_get_indication_register_resp->reg_service_ready_events_valid = TRUE;
           wms_get_indication_register_resp->reg_service_ready_events =
                               remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_service_ready_events;
        }
        else
        {
           wms_get_indication_register_resp->reg_service_ready_events_valid = FALSE;
        }

        if (local_svlte_cache_ptr->wms_get_indication_register_resp.reg_broadcast_config_events_valid
             && remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_broadcast_config_events_valid)
        {
           wms_get_indication_register_resp->reg_broadcast_config_events_valid = TRUE;
           wms_get_indication_register_resp->reg_broadcast_config_events =
                          remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_broadcast_config_events |
                              local_svlte_cache_ptr->wms_get_indication_register_resp.reg_broadcast_config_events;
        }
        else if (local_svlte_cache_ptr->wms_get_indication_register_resp.reg_broadcast_config_events_valid)
        {
           wms_get_indication_register_resp->reg_broadcast_config_events_valid = TRUE;
           wms_get_indication_register_resp->reg_broadcast_config_events =
                                      local_svlte_cache_ptr->wms_get_indication_register_resp.reg_broadcast_config_events;
        }
        else if (remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_broadcast_config_events_valid)
        {
           wms_get_indication_register_resp->reg_broadcast_config_events_valid = TRUE;
           wms_get_indication_register_resp->reg_broadcast_config_events =
                                      remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_broadcast_config_events;
        }
        else
        {
           wms_get_indication_register_resp->reg_broadcast_config_events_valid = FALSE;
        }
      }
    }
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_wms_resp_proc_wms_get_indication_register */

/*=========================================================================
  FUNCTION:  qmi_proxy_wms_resp_proc_wms_get_service_ready_status

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal svlte cache per received
    QMI_WMS_GET_SERVICE_READY_STATUS_RESP

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_wms_resp_proc_wms_get_service_ready_status
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  wms_get_service_ready_status_resp_msg_v01 *wms_get_service_ready_status_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Supress compiler warnings for unused variables */
  (void) proxy_srvc_id;
  (void) proxy_client_id;
  (void) proxy_msg_id;
  (void) resp_c_struct_len;
  (void) resp_modified;
  (void) modified_resp_c_struct;
  (void) modified_resp_c_struct_len;

  wms_get_service_ready_status_resp = ( wms_get_service_ready_status_resp_msg_v01 *) resp_c_struct;

  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, wms_get_service_ready_status_resp ) )
  {
    memset( &svlte_cache_ptr->wms_get_service_ready_status_resp, 0, sizeof( wms_get_service_ready_status_resp_msg_v01 ) );

    if ( wms_get_service_ready_status_resp != NULL )
    {
      svlte_cache_ptr->wms_get_service_ready_status_resp = *wms_get_service_ready_status_resp;

      if (wms_get_service_ready_status_resp->ready_status_valid)
      {
        qmi_proxy_internal_info.service_ready[ conn_type ] = wms_get_service_ready_status_resp->ready_status;
      }
    }
    else
    {
      /* Ideally response should not be NULL as it has mandatory field, keeping klockwork happy */
      QMI_PROXY_ERR_MSG( "Response is NULL for srvc_id = %d, msg_id = 0x%x\n", srvc_id, msg_id );
      return QMI_INTERNAL_ERR;
    }

    if ( last_txn && user_data_valid && (user_data == QMI_PROXY_BOTH_CONN_TYPE))
    {
      if ( local_svlte_cache_ptr->wms_get_service_ready_status_resp.ready_status_valid
           && remote_svlte_cache_ptr->wms_get_service_ready_status_resp.ready_status_valid )
      {
         wms_get_service_ready_status_resp->ready_status_valid = TRUE;
         wms_get_service_ready_status_resp->ready_status = local_svlte_cache_ptr->wms_get_service_ready_status_resp.ready_status;
         switch (local_svlte_cache_ptr->wms_get_service_ready_status_resp.ready_status)
         {
           case WMS_SERVICE_READY_STATUS_3GPP_V01:
           {
             switch (remote_svlte_cache_ptr->wms_get_service_ready_status_resp.ready_status)
             {
               case WMS_SERVICE_READY_STATUS_3GPP2_V01:
               case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01:
               {
                 wms_get_service_ready_status_resp->ready_status = WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01;
                 break;
               }
               default:
                 break;
             }
             break;
           }
           case WMS_SERVICE_READY_STATUS_3GPP2_V01:
           {
             switch (remote_svlte_cache_ptr->wms_get_service_ready_status_resp.ready_status)
             {
               case WMS_SERVICE_READY_STATUS_3GPP_V01:
               case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01:
               {
                 wms_get_service_ready_status_resp->ready_status = WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01;
                 break;
               }
               default:
                 break;
             }
             break;
           }
           case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01:
           {
             break;
           }
           default:
           {
             wms_get_service_ready_status_resp->ready_status = remote_svlte_cache_ptr->wms_get_service_ready_status_resp.ready_status;
             break;
           }
         }
      }
      else if ( local_svlte_cache_ptr->wms_get_service_ready_status_resp.ready_status_valid )
      {
         memcpy(wms_get_service_ready_status_resp, &(local_svlte_cache_ptr->wms_get_service_ready_status_resp), sizeof(*wms_get_service_ready_status_resp));
      }
      else if ( remote_svlte_cache_ptr->wms_get_service_ready_status_resp.ready_status_valid )
      {
         memcpy(wms_get_service_ready_status_resp, &(remote_svlte_cache_ptr->wms_get_service_ready_status_resp), sizeof(*wms_get_service_ready_status_resp));
      }
      else
      {
         wms_get_service_ready_status_resp->ready_status_valid = FALSE;
      }
    }
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_wms_resp_proc_wms_get_service_ready_status */

/*=========================================================================
  FUNCTION:  qmi_proxy_nas_resp_proc_dms_set_oprt_mode

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per received
    QMI_DMS_SET_OPERATING_MODE_RESP initiated from NAS service request.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_resp_proc_dms_set_oprt_mode
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  dms_set_operating_mode_resp_msg_v01 *dms_set_oprt_mode_resp = NULL;
  nas_set_system_selection_preference_resp_msg_v01 *nas_set_sys_sel_pref_resp = NULL;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Supress compiler warnings for unused variables */
  (void) proxy_client_id;
  (void) srvc_id;
  (void) msg_id;
  (void) resp_c_struct_len;

  dms_set_oprt_mode_resp = ( dms_set_operating_mode_resp_msg_v01 *) resp_c_struct;

  if ( !QMI_PROXY_RESPONSE_SUCCESS( rc, dms_set_oprt_mode_resp ) )
  {
    /* Encode the failure response message */
    QMI_PROXY_DEBUG_MSG( "Fail to change %s(%d) oprt mode, qmi_err %d, qmi_result %d\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type,
                         dms_set_oprt_mode_resp->resp.error, dms_set_oprt_mode_resp->resp.result );

    /* Allocate buffer */
    rc = qmi_idl_get_message_c_struct_len( qmi_proxy_srvc_obj_tbl[ proxy_srvc_id ],
                                           QMI_IDL_RESPONSE,
                                           proxy_msg_id,
                                           (uint32_t *) modified_resp_c_struct_len );
    *modified_resp_c_struct = qmi_proxy_malloc( *modified_resp_c_struct_len );
    if ( *modified_resp_c_struct == NULL )
    {
      rc = QMI_INTERNAL_ERR;
    }
    else
    {
      nas_set_sys_sel_pref_resp = ( nas_set_system_selection_preference_resp_msg_v01 *) *modified_resp_c_struct;
      nas_set_sys_sel_pref_resp->resp = dms_set_oprt_mode_resp->resp;
      *resp_modified = TRUE;
    }
  }
  else
  {
    /* Need to update SGLTE mode preference */
    if ( last_txn && user_data_valid )
    {
      if ( QMI_PROXY_SGLTE_TARGET() )
      {
        qmi_proxy_update_cache_sglte_mode_pref_from_user_data( user_data );
      }
      else
      {
      qmi_proxy_update_cache_svlte_mode_pref_from_user_data( user_data );
    }
  }
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_nas_resp_proc_dms_set_oprt_mode */

/*=========================================================================
  FUNCTION:  qmi_proxy_nas_resp_proc_dms_get_oprt_mode

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per received
    QMI_DMS_GET_OPERATING_MODE_RESP initiated from NAS service request.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_resp_proc_dms_get_oprt_mode
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  dms_get_operating_mode_resp_msg_v01 *dms_get_oprt_mode_resp = NULL;
  nas_get_system_selection_preference_resp_msg_v01 *nas_get_sys_sel_pref_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Supress compiler warnings for unused variables */
  (void) proxy_client_id;
  (void) srvc_id;
  (void) msg_id;
  (void) resp_c_struct_len;
  (void) last_txn;
  (void) user_data_valid;
  (void) user_data;

  dms_get_oprt_mode_resp = ( dms_get_operating_mode_resp_msg_v01 *) resp_c_struct;

  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, dms_get_oprt_mode_resp ) )
  {
    memset( &svlte_cache_ptr->dms_get_oprt_mode_resp, 0, sizeof( dms_get_operating_mode_resp_msg_v01 ) );

    if ( dms_get_oprt_mode_resp != NULL )
    {
      /* Update cache if no error */
      svlte_cache_ptr->dms_get_oprt_mode_resp = *dms_get_oprt_mode_resp;
    }
  }
  else
  {
    /* Encode the failure response message */
    QMI_PROXY_DEBUG_MSG( "Fail to query %s(%d) oprt mode, qmi_err %d, qmi_result %d\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type,
                         dms_get_oprt_mode_resp->resp.error, dms_get_oprt_mode_resp->resp.result );

    /* Allocate buffer */
    rc = qmi_idl_get_message_c_struct_len( qmi_proxy_srvc_obj_tbl[ proxy_srvc_id ],
                                           QMI_IDL_RESPONSE,
                                           proxy_msg_id,
                                           (uint32_t *) modified_resp_c_struct_len );
    *modified_resp_c_struct = qmi_proxy_malloc( *modified_resp_c_struct_len );
    if ( *modified_resp_c_struct == NULL )
    {
      rc =  QMI_INTERNAL_ERR;
    }
    else
    {
      nas_get_sys_sel_pref_resp = ( nas_get_system_selection_preference_resp_msg_v01 *) *modified_resp_c_struct;
      nas_get_sys_sel_pref_resp->resp = dms_get_oprt_mode_resp->resp;
      *resp_modified = TRUE;
    }
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_nas_resp_proc_dms_get_oprt_mode */


/*=========================================================================
  FUNCTION:  qmi_proxy_nas_resp_proc_nas_get_sys_info

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per received
    QMI_NAS_GET_SYS_INFO_RESP_MSG_V01.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_resp_proc_nas_get_sys_info
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  nas_get_sys_info_resp_msg_v01 *nas_get_sys_info_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Supress compiler warnings for unused variables */
  (void) proxy_srvc_id;
  (void) proxy_client_id;
  (void) proxy_msg_id;
  (void) resp_c_struct_len;
  (void) user_data_valid;
  (void) user_data;
  (void) resp_modified;
  (void) modified_resp_c_struct;
  (void) modified_resp_c_struct_len;

  nas_get_sys_info_resp = ( nas_get_sys_info_resp_msg_v01 *) resp_c_struct;

  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, nas_get_sys_info_resp ) )
  {
    memset( &svlte_cache_ptr->nas_get_sys_info_resp, 0, sizeof( nas_get_sys_info_resp_msg_v01 ) );

    if ( nas_get_sys_info_resp != NULL )
    {
      svlte_cache_ptr->nas_get_sys_info_resp = *nas_get_sys_info_resp;

      /* reset some of the information selectively depending on the type of the connection type local/remote,
          as they are not valid for the corresponding conn_type */
      if ( conn_type == QMI_PROXY_LOCAL_CONN_TYPE )
      {
        svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.hdr_sys_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.lte_sys_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.hdr_sys_info2_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.lte_sys_info2_valid = FALSE;
      }
      else if ( conn_type == QMI_PROXY_REMOTE_CONN_TYPE )
      {
        svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.cdma_sys_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.cdma_sys_info2_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info2_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info2_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info3_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info3_valid = FALSE;
      }

    }

    if ( last_txn )
    {
      /* System info from local modem */
      if ( local_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Local modem CDMA srv status %d\n",
                             local_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info.srv_status );
      }

      if ( local_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Local modem HDR srv status %d\n",
                             local_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info.srv_status );
      }

      if ( local_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Local modem GSM srv status %d\n",
                             local_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info.srv_status );
      }

      if ( local_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Local modem WCDMA srv status %d\n",
                             local_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info.srv_status );
      }

      if ( local_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Local modem LTE srv status %d\n",
                             local_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info.srv_status );
      }

      /* System info from remote modem */
      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Remote modem CDMA srv status %d\n",
                             remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info.srv_status );
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Remote modem HDR srv status %d\n",
                             remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info.srv_status );
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Remote modem GSM srv status %d\n",
                             remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info.srv_status );
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Remote modem WCDMA srv status %d\n",
                             remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info.srv_status );
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Remote modem LTE srv status %d\n",
                             remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info.srv_status );
      }

      /* Aggregate response */
      if ( nas_get_sys_info_resp == NULL )
      {
        /* Ideally response should not be NULL as it has mandatory field, keeping klockwork happy */
        QMI_PROXY_ERR_MSG( "Response is NULL for srvc_id = %d, msg_id = 0x%x\n", srvc_id, msg_id );
        return QMI_INTERNAL_ERR;
      }

      *nas_get_sys_info_resp = local_svlte_cache_ptr->nas_get_sys_info_resp;

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info_valid )
      {
        if ( !nas_get_sys_info_resp->cdma_srv_status_info_valid )
        {
          nas_get_sys_info_resp->cdma_srv_status_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info_valid;
          nas_get_sys_info_resp->cdma_srv_status_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info_valid )
      {
        if ( !nas_get_sys_info_resp->hdr_srv_status_info_valid )
        {
          nas_get_sys_info_resp->hdr_srv_status_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info_valid;
          nas_get_sys_info_resp->hdr_srv_status_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info_valid )
      {
        if ( !nas_get_sys_info_resp->gsm_srv_status_info_valid )
        {
          nas_get_sys_info_resp->gsm_srv_status_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info_valid;
          nas_get_sys_info_resp->gsm_srv_status_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info_valid )
      {
        if ( !nas_get_sys_info_resp->wcdma_srv_status_info_valid )
        {
          nas_get_sys_info_resp->wcdma_srv_status_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info_valid;
          nas_get_sys_info_resp->wcdma_srv_status_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info_valid )
      {
        if ( !nas_get_sys_info_resp->lte_srv_status_info_valid )
        {
          nas_get_sys_info_resp->lte_srv_status_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info_valid;
          nas_get_sys_info_resp->lte_srv_status_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_sys_info_valid )
      {
        if ( !nas_get_sys_info_resp->cdma_sys_info_valid )
        {
          nas_get_sys_info_resp->cdma_sys_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_sys_info_valid;
          nas_get_sys_info_resp->cdma_sys_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_sys_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_sys_info_valid )
      {
        if ( !nas_get_sys_info_resp->hdr_sys_info_valid )
        {
          nas_get_sys_info_resp->hdr_sys_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_sys_info_valid;
          nas_get_sys_info_resp->hdr_sys_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_sys_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info_valid )
      {
        if ( !nas_get_sys_info_resp->gsm_sys_info_valid )
        {
          nas_get_sys_info_resp->gsm_sys_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info_valid;
          nas_get_sys_info_resp->gsm_sys_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info_valid )
      {
        if ( !nas_get_sys_info_resp->wcdma_sys_info_valid )
        {
          nas_get_sys_info_resp->wcdma_sys_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info_valid;
          nas_get_sys_info_resp->wcdma_sys_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_sys_info_valid )
      {
        if ( !nas_get_sys_info_resp->lte_sys_info_valid )
        {
          nas_get_sys_info_resp->lte_sys_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_sys_info_valid;
          nas_get_sys_info_resp->lte_sys_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_sys_info;
        }
      }

      /* Consolidate response */
      if ( nas_get_sys_info_resp->cdma_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Consolidated CDMA srv status %d\n",
                             nas_get_sys_info_resp->cdma_srv_status_info.srv_status );
      }

      if ( nas_get_sys_info_resp->hdr_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Consolidated HDR srv status %d\n",
                             nas_get_sys_info_resp->hdr_srv_status_info.srv_status );
      }

      if ( nas_get_sys_info_resp->gsm_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Consolidated GSM srv status %d\n",
                             nas_get_sys_info_resp->gsm_srv_status_info.srv_status );
      }

      if ( nas_get_sys_info_resp->wcdma_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Consolidated WCDMA srv status %d\n",
                             nas_get_sys_info_resp->wcdma_srv_status_info.srv_status );
      }

      if ( nas_get_sys_info_resp->lte_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Consolidated LTE srv status %d\n",
                             nas_get_sys_info_resp->lte_srv_status_info.srv_status );
      }
    }
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_nas_resp_proc_nas_get_sys_info */

/*=========================================================================
  FUNCTION:  qmi_proxy_nas_resp_proc_nas_get_sys_info_sglte

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per received
    QMI_NAS_GET_SYS_INFO_RESP_MSG_V01.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_resp_proc_nas_get_sys_info_sglte
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  nas_get_sys_info_resp_msg_v01 *nas_get_sys_info_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  int cs_active_modem, ps_active_modem;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  (void) user_data;

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Supress compiler warnings for unused variables */
  (void) proxy_srvc_id;
  (void) proxy_client_id;
  (void) proxy_msg_id;
  (void) resp_c_struct_len;
  (void) user_data_valid;
  (void) resp_modified;
  (void) modified_resp_c_struct;
  (void) modified_resp_c_struct_len;

  nas_get_sys_info_resp = ( nas_get_sys_info_resp_msg_v01 *) resp_c_struct;

  cs_active_modem = qmi_proxy_internal_info.cs_active_modem;
  ps_active_modem = qmi_proxy_internal_info.ps_active_modem;

  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, nas_get_sys_info_resp ) )
  {
    memset( &svlte_cache_ptr->nas_get_sys_info_resp, 0, sizeof( nas_get_sys_info_resp_msg_v01 ) );

    if ( nas_get_sys_info_resp != NULL )
    {
      svlte_cache_ptr->nas_get_sys_info_resp = *nas_get_sys_info_resp;

      /* reset some of the information selectively depending on the type of the connection type local/remote,
          as they are not valid for the corresponding conn_type */
      if ( conn_type == QMI_PROXY_LOCAL_CONN_TYPE && (cs_active_modem == QMI_PROXY_REMOTE_CONN_TYPE || ps_active_modem == QMI_PROXY_REMOTE_CONN_TYPE))
      {
        svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info2_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info3_valid = FALSE;
        if ( cs_active_modem != QMI_PROXY_LOCAL_CONN_TYPE && ps_active_modem != QMI_PROXY_LOCAL_CONN_TYPE )
        {
            svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_srv_status_info_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.cdma_sys_info_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_sys_info_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.hdr_sys_info_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.lte_sys_info_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.cdma_sys_info2_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info2_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info3_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.hdr_sys_info2_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.lte_sys_info2_valid = FALSE;
    }
      }
      else if ( conn_type == QMI_PROXY_REMOTE_CONN_TYPE && (cs_active_modem == QMI_PROXY_LOCAL_CONN_TYPE || ps_active_modem == QMI_PROXY_LOCAL_CONN_TYPE))
    {
        svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_srv_status_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.cdma_sys_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_sys_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.hdr_sys_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.lte_sys_info_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.cdma_sys_info2_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info2_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info3_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.hdr_sys_info2_valid = FALSE;
        svlte_cache_ptr->nas_get_sys_info_resp.lte_sys_info2_valid = FALSE;
        if ( cs_active_modem != QMI_PROXY_REMOTE_CONN_TYPE && ps_active_modem != QMI_PROXY_REMOTE_CONN_TYPE)
        {
            svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info2_valid = FALSE;
            svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info3_valid = FALSE;
        }
      }

    }

    if ( last_txn )
    {
      /* System info from local modem */
      if ( local_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Local modem CDMA srv status %d\n",
                             local_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info.srv_status );
      }

      if ( local_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Local modem HDR srv status %d\n",
                             local_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info.srv_status );
      }

      if ( local_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info_valid )
          {
        QMI_PROXY_DEBUG_MSG( "Local modem GSM srv status %d\n",
                             local_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info.srv_status );
      }

      if ( local_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info_valid )
            {
        QMI_PROXY_DEBUG_MSG( "Local modem WCDMA srv status %d\n",
                             local_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info.srv_status );
      }

      if ( local_svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_srv_status_info_valid )
            {
        QMI_PROXY_DEBUG_MSG( "Local modem TDSCDMA srv status %d\n",
                             local_svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_srv_status_info.srv_status );
      }

      if ( local_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info_valid )
              {
        QMI_PROXY_DEBUG_MSG( "Local modem LTE srv status %d\n",
                             local_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info.srv_status );
              }

      /* System info from remote modem */
      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Remote modem CDMA srv status %d\n",
                             remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info.srv_status );
            }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info_valid )
            {
        QMI_PROXY_DEBUG_MSG( "Remote modem HDR srv status %d\n",
                             remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info.srv_status );
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info_valid )
              {
        QMI_PROXY_DEBUG_MSG( "Remote modem GSM srv status %d\n",
                             remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info.srv_status );
              }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Remote modem WCDMA srv status %d\n",
                             remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info.srv_status );
            }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Remote modem TDSCDMA srv status %d\n",
                             remote_svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_srv_status_info.srv_status );
            }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Remote modem LTE srv status %d\n",
                             remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info.srv_status );
          }

      /* Aggregate response */
      if ( nas_get_sys_info_resp == NULL )
          {
        /* Ideally response should not be NULL as it has mandatory field, keeping klockwork happy */
        QMI_PROXY_ERR_MSG( "Response is NULL for srvc_id = %d, msg_id = 0x%x\n", srvc_id, msg_id );
        return QMI_INTERNAL_ERR;
      }

      *nas_get_sys_info_resp = local_svlte_cache_ptr->nas_get_sys_info_resp;

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info_valid )
            {
        if ( !nas_get_sys_info_resp->cdma_srv_status_info_valid )
              {
          nas_get_sys_info_resp->cdma_srv_status_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info_valid;
          nas_get_sys_info_resp->cdma_srv_status_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_srv_status_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info_valid )
      {
        if ( !nas_get_sys_info_resp->hdr_srv_status_info_valid )
        {
          nas_get_sys_info_resp->hdr_srv_status_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info_valid;
          nas_get_sys_info_resp->hdr_srv_status_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_srv_status_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info_valid )
      {
        if ( !nas_get_sys_info_resp->gsm_srv_status_info_valid )
        {
          nas_get_sys_info_resp->gsm_srv_status_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info_valid;
          nas_get_sys_info_resp->gsm_srv_status_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_srv_status_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info_valid )
      {
        if ( !nas_get_sys_info_resp->wcdma_srv_status_info_valid )
        {
          nas_get_sys_info_resp->wcdma_srv_status_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info_valid;
          nas_get_sys_info_resp->wcdma_srv_status_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_srv_status_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_srv_status_info_valid )
      {
        if ( !nas_get_sys_info_resp->tdscdma_srv_status_info_valid )
        {
          nas_get_sys_info_resp->tdscdma_srv_status_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_srv_status_info_valid;
          nas_get_sys_info_resp->tdscdma_srv_status_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_srv_status_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info_valid )
      {
        if ( !nas_get_sys_info_resp->lte_srv_status_info_valid )
        {
          nas_get_sys_info_resp->lte_srv_status_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info_valid;
          nas_get_sys_info_resp->lte_srv_status_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_srv_status_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_sys_info_valid )
      {
        if ( !nas_get_sys_info_resp->cdma_sys_info_valid )
        {
          nas_get_sys_info_resp->cdma_sys_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_sys_info_valid;
          nas_get_sys_info_resp->cdma_sys_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.cdma_sys_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_sys_info_valid )
      {
        if ( !nas_get_sys_info_resp->hdr_sys_info_valid )
        {
          nas_get_sys_info_resp->hdr_sys_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_sys_info_valid;
          nas_get_sys_info_resp->hdr_sys_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.hdr_sys_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info_valid )
      {
        if ( !nas_get_sys_info_resp->gsm_sys_info_valid )
        {
          nas_get_sys_info_resp->gsm_sys_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info_valid;
          nas_get_sys_info_resp->gsm_sys_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.gsm_sys_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info_valid )
      {
        if ( !nas_get_sys_info_resp->wcdma_sys_info_valid )
        {
          nas_get_sys_info_resp->wcdma_sys_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info_valid;
          nas_get_sys_info_resp->wcdma_sys_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.wcdma_sys_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_sys_info_valid )
      {
        if ( !nas_get_sys_info_resp->tdscdma_sys_info_valid )
        {
          nas_get_sys_info_resp->tdscdma_sys_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_sys_info_valid;
          nas_get_sys_info_resp->tdscdma_sys_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.tdscdma_sys_info;
        }
      }

      if ( remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_sys_info_valid )
      {
        if ( !nas_get_sys_info_resp->lte_sys_info_valid )
        {
          nas_get_sys_info_resp->lte_sys_info_valid = remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_sys_info_valid;
          nas_get_sys_info_resp->lte_sys_info = remote_svlte_cache_ptr->nas_get_sys_info_resp.lte_sys_info;
        }
      }

      /* Consolidate response */
      if ( nas_get_sys_info_resp->cdma_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Consolidated CDMA srv status %d\n",
                             nas_get_sys_info_resp->cdma_srv_status_info.srv_status );
      }

      if ( nas_get_sys_info_resp->hdr_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Consolidated HDR srv status %d\n",
                             nas_get_sys_info_resp->hdr_srv_status_info.srv_status );
      }

      if ( nas_get_sys_info_resp->gsm_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Consolidated GSM srv status %d\n",
                             nas_get_sys_info_resp->gsm_srv_status_info.srv_status );
      }

      if ( nas_get_sys_info_resp->wcdma_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Consolidated WCDMA srv status %d\n",
                             nas_get_sys_info_resp->wcdma_srv_status_info.srv_status );
      }

      if ( nas_get_sys_info_resp->tdscdma_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Consolidated TDSCDMA srv status %d\n",
                             nas_get_sys_info_resp->tdscdma_srv_status_info.srv_status );
      }

      if ( nas_get_sys_info_resp->lte_srv_status_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "Consolidated LTE srv status %d\n",
                             nas_get_sys_info_resp->lte_srv_status_info.srv_status );
      }
    }
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_nas_resp_proc_nas_get_sys_info_sglte */


/*=========================================================================
  FUNCTION:  qmi_proxy_nas_per_net_scan_check_if_3gpp_info_dup

===========================================================================*/
/*!
    @brief
    Check 3gpp info entry is duplicate to any of the existing entries
    @return
    TRUE if entry is duplicate. Otherwise, FALSE
*/
/*=========================================================================*/
static boolean qmi_proxy_nas_per_net_scan_check_if_3gpp_info_dup
(
 nas_perform_network_scan_resp_msg_v01        *local_nas_per_net_scan_resp,
 nas_perform_network_scan_resp_msg_v01        *remote_nas_per_net_scan_resp,
 int                                           remote_nas_3gpp_network_info_index
 )
{
    unsigned int i;

    if (local_nas_per_net_scan_resp->nas_3gpp_network_info_valid &&
            remote_nas_per_net_scan_resp->nas_3gpp_network_info_valid &&
            local_nas_per_net_scan_resp->nas_network_radio_access_technology_valid &&
            remote_nas_per_net_scan_resp->nas_network_radio_access_technology_valid &&
            local_nas_per_net_scan_resp->mnc_includes_pcs_digit_valid &&
            remote_nas_per_net_scan_resp->mnc_includes_pcs_digit_valid)
    {
        for (i = 0; i < local_nas_per_net_scan_resp->nas_network_radio_access_technology_len; i++)
        {
            if ((local_nas_per_net_scan_resp->nas_3gpp_network_info[i].mobile_country_code ==
                        remote_nas_per_net_scan_resp->nas_3gpp_network_info[remote_nas_3gpp_network_info_index].mobile_country_code) &&
                    (local_nas_per_net_scan_resp->nas_3gpp_network_info[i].mobile_network_code ==
                     remote_nas_per_net_scan_resp->nas_3gpp_network_info[remote_nas_3gpp_network_info_index].mobile_network_code) &&
                    (local_nas_per_net_scan_resp->nas_3gpp_network_info[i].network_status ==
                     remote_nas_per_net_scan_resp->nas_3gpp_network_info[remote_nas_3gpp_network_info_index].network_status) &&
                    (local_nas_per_net_scan_resp->nas_network_radio_access_technology[i].rat ==
                     remote_nas_per_net_scan_resp->nas_network_radio_access_technology[remote_nas_3gpp_network_info_index].rat) &&
                    (local_nas_per_net_scan_resp->mnc_includes_pcs_digit[i].mnc_includes_pcs_digit ==
                     remote_nas_per_net_scan_resp->mnc_includes_pcs_digit[remote_nas_3gpp_network_info_index].mnc_includes_pcs_digit))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/*=========================================================================
  FUNCTION:  qmi_proxy_nas_resp_proc_nas_per_net_scan

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per received
    QMI_NAS_PERFORM_NETWORK_SCAN_RESP_MSG.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_resp_proc_nas_per_net_scan
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  nas_perform_network_scan_resp_msg_v01 *nas_per_net_scan_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  uint32 i, j;
  int rc = QMI_NO_ERR;
  nas_perform_network_scan_resp_msg_v01 *temp_resp = NULL;

  /*-----------------------------------------------------------------------*/

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Supress compiler warnings for unused variables */
  (void) proxy_srvc_id;
  (void) proxy_client_id;
  (void) proxy_msg_id;
  (void) resp_c_struct_len;
  (void) user_data_valid;
  (void) resp_modified;
  (void) modified_resp_c_struct;
  (void) modified_resp_c_struct_len;

  nas_per_net_scan_resp = ( nas_perform_network_scan_resp_msg_v01 *) resp_c_struct;

  /* Irrespective of whether response was success or not, store the response in svlte cache */
  memset( &svlte_cache_ptr->nas_per_net_scan_resp, 0, sizeof( nas_perform_network_scan_resp_msg_v01 ) );

  if ( nas_per_net_scan_resp != NULL )
  {
    svlte_cache_ptr->nas_per_net_scan_resp = *nas_per_net_scan_resp;
  }
  else
  {
    /* Ideally response should not be NULL as it has mandatory field, keeping klockwork happy */
    QMI_PROXY_ERR_MSG( "Response is NULL for srvc_id = %d, msg_id = 0x%x\n", srvc_id, msg_id );
    rc = QMI_INTERNAL_ERR;
  }


  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, nas_per_net_scan_resp ) )
  {
    if ( last_txn )
    {
      switch( user_data )
      {
        case QMI_PROXY_LOCAL_CONN_TYPE:
          *nas_per_net_scan_resp = local_svlte_cache_ptr->nas_per_net_scan_resp;
          break;

        case QMI_PROXY_REMOTE_CONN_TYPE:
          *nas_per_net_scan_resp = remote_svlte_cache_ptr->nas_per_net_scan_resp;
          break;

        default:
          *nas_per_net_scan_resp = local_svlte_cache_ptr->nas_per_net_scan_resp;

          /* Check if network scan succeded in both modems*/

          temp_resp = &local_svlte_cache_ptr->nas_per_net_scan_resp;
          if( !QMI_PROXY_RESPONSE_SUCCESS( rc, temp_resp ) )
          {
            break;
          }

          temp_resp = &remote_svlte_cache_ptr->nas_per_net_scan_resp;
          if( !QMI_PROXY_RESPONSE_SUCCESS( rc, temp_resp ) )
          {
            *nas_per_net_scan_resp = remote_svlte_cache_ptr->nas_per_net_scan_resp;
            break;
          }

          /* If NAS_SCAN failed, copy the failed response and return */

          if( local_svlte_cache_ptr->nas_per_net_scan_resp.scan_result_valid &&
              local_svlte_cache_ptr->nas_per_net_scan_resp.scan_result != NAS_SCAN_SUCCESS_V01 )
          {
            //nas_per_net_scan_resp already set
            break;
          }

          if( remote_svlte_cache_ptr->nas_per_net_scan_resp.scan_result_valid &&
              remote_svlte_cache_ptr->nas_per_net_scan_resp.scan_result != NAS_SCAN_SUCCESS_V01 )
          {
            *nas_per_net_scan_resp = remote_svlte_cache_ptr->nas_per_net_scan_resp;
            break;
          }


          // Do the following iff scan result is set success for both modems
          if ( remote_svlte_cache_ptr->nas_per_net_scan_resp.nas_3gpp_network_info_valid )
          {
            if ( local_svlte_cache_ptr->nas_per_net_scan_resp.nas_3gpp_network_info_valid )
            {
              for ( i = local_svlte_cache_ptr->nas_per_net_scan_resp.nas_3gpp_network_info_len, j = 0;
                    j < remote_svlte_cache_ptr->nas_per_net_scan_resp.nas_3gpp_network_info_len &&
                    j < NAS_3GPP_NETWORK_INFO_LIST_MAX_V01 &&
                    i < NAS_3GPP_NETWORK_INFO_LIST_MAX_V01; j++ )
              {
                if (QMI_PROXY_SGLTE_TARGET() &&
                      qmi_proxy_nas_per_net_scan_check_if_3gpp_info_dup(
                          &local_svlte_cache_ptr->nas_per_net_scan_resp,
                          &remote_svlte_cache_ptr->nas_per_net_scan_resp,
                          j))
                {
                  continue;
                }
                nas_per_net_scan_resp->nas_3gpp_network_info[ i ] =
                  remote_svlte_cache_ptr->nas_per_net_scan_resp.nas_3gpp_network_info[ j ];
                nas_per_net_scan_resp->nas_3gpp_network_info_len++;

                nas_per_net_scan_resp->nas_network_radio_access_technology[ i ] =
                  remote_svlte_cache_ptr->nas_per_net_scan_resp.nas_network_radio_access_technology[ j ];
                nas_per_net_scan_resp->nas_network_radio_access_technology_len++;

                nas_per_net_scan_resp->mnc_includes_pcs_digit[ i ] =
                  remote_svlte_cache_ptr->nas_per_net_scan_resp.mnc_includes_pcs_digit[ j ];
                nas_per_net_scan_resp->mnc_includes_pcs_digit_len++;
                i++;
              }
            }
            else
            {
              nas_per_net_scan_resp->nas_3gpp_network_info_valid = TRUE;
              nas_per_net_scan_resp->nas_3gpp_network_info_len =
                remote_svlte_cache_ptr->nas_per_net_scan_resp.nas_3gpp_network_info_len;

              nas_per_net_scan_resp->nas_network_radio_access_technology_valid = TRUE;
              nas_per_net_scan_resp->nas_network_radio_access_technology_len =
                remote_svlte_cache_ptr->nas_per_net_scan_resp.nas_network_radio_access_technology_len;

              nas_per_net_scan_resp->mnc_includes_pcs_digit_valid = TRUE;
              nas_per_net_scan_resp->mnc_includes_pcs_digit_len =
                remote_svlte_cache_ptr->nas_per_net_scan_resp.mnc_includes_pcs_digit_len;

              for ( i = 0; i < remote_svlte_cache_ptr->nas_per_net_scan_resp.nas_3gpp_network_info_len &&
                           i < NAS_3GPP_NETWORK_INFO_LIST_MAX_V01; i++ )
              {
                nas_per_net_scan_resp->nas_3gpp_network_info[ i ] =
                  remote_svlte_cache_ptr->nas_per_net_scan_resp.nas_3gpp_network_info[ i ];
                nas_per_net_scan_resp->nas_network_radio_access_technology[ i ] =
                  remote_svlte_cache_ptr->nas_per_net_scan_resp.nas_network_radio_access_technology[ i ];
                nas_per_net_scan_resp->mnc_includes_pcs_digit[ i ] =
                  remote_svlte_cache_ptr->nas_per_net_scan_resp.mnc_includes_pcs_digit[ i ];
              }
            }
          }
          break;
      }
    }
  }

  if ( rc != QMI_NO_ERR && !last_txn )
  {
      /* Do not send a failure response till you get last txn */
      rc = QMI_NO_ERR;
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_nas_resp_proc_nas_per_net_scan */

/*=========================================================================
  FUNCTION:  qmi_proxy_nas_resp_proc_nas_set_sys_sel_pref

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per received
    QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_resp_proc_nas_set_sys_sel_pref
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  nas_set_system_selection_preference_resp_msg_v01 *nas_set_sys_sel_pref_resp = NULL;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Supress compiler warnings for unused variables */
  (void) proxy_srvc_id;
  (void) proxy_client_id;
  (void) proxy_msg_id;
  (void) srvc_id;
  (void) msg_id;
  (void) resp_c_struct;
  (void) resp_c_struct_len;
  (void) resp_modified;
  (void) modified_resp_c_struct;
  (void) modified_resp_c_struct_len;

  nas_set_sys_sel_pref_resp = ( nas_set_system_selection_preference_resp_msg_v01 *) resp_c_struct;

  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, nas_set_sys_sel_pref_resp ) )
  {
    /* Need to update SVLTE mode preference */
    if ( last_txn && user_data_valid )
    {
      if ( QMI_PROXY_SGLTE_TARGET() )
      {
        qmi_proxy_update_cache_sglte_mode_pref_from_user_data( user_data );
      }
      else
      {
        qmi_proxy_update_cache_svlte_mode_pref_from_user_data( user_data );
      }
    }
  }
  else
  {
    QMI_PROXY_DEBUG_MSG( "Fail to set %s(%d) sys sel pref, qmi_err %d, qmi_result %d\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type,
                         nas_set_sys_sel_pref_resp->resp.error, nas_set_sys_sel_pref_resp->resp.result );
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_nas_resp_proc_nas_set_sys_sel_pref */


/*=========================================================================
  FUNCTION:  qmi_proxy_nas_resp_proc_nas_get_sys_sel_pref

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per received
    QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_resp_proc_nas_get_sys_sel_pref
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  nas_get_system_selection_preference_resp_msg_v01 *nas_get_sys_sel_pref_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  qmi_proxy_mode_pref_type svlte_mode_pref;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Supress compiler warnings for unused variables */
  (void) proxy_srvc_id;
  (void) proxy_client_id;
  (void) proxy_msg_id;
  (void) resp_c_struct_len;
  (void) resp_modified;
  (void) user_data_valid;
  (void) user_data;
  (void) modified_resp_c_struct;
  (void) modified_resp_c_struct_len;

  nas_get_sys_sel_pref_resp = ( nas_get_system_selection_preference_resp_msg_v01 * ) resp_c_struct;

  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, nas_get_sys_sel_pref_resp ) )
  {
    memset( &svlte_cache_ptr->nas_get_sys_sel_pref_resp, 0, sizeof( nas_get_system_selection_preference_resp_msg_v01 ) );

    if ( nas_get_sys_sel_pref_resp != NULL )
    {
      QMI_PROXY_DEBUG_MSG( "Query %s(%d) mode_pref_valid = %d, mode_pref = 0x%x\n",
                           qmi_proxy_modem_name[ conn_type ], conn_type,
                           nas_get_sys_sel_pref_resp->mode_pref_valid,
                           nas_get_sys_sel_pref_resp->mode_pref );

      svlte_cache_ptr->nas_get_sys_sel_pref_resp = *nas_get_sys_sel_pref_resp;
    }
    else
    {
      /* Ideally response should not be NULL as it has mandatory field, keeping klockwork happy */
      QMI_PROXY_ERR_MSG( "Response is NULL for srvc_id = %d, msg_id = 0x%x\n", srvc_id, msg_id );
      return QMI_INTERNAL_ERR;
    }

    /* Need to update SVLTE mode pref */
    if ( last_txn )
    {
      /* Update SVLTE mode preference from cache */
      svlte_mode_pref = qmi_proxy_update_cache_svlte_mode_pref_from_resp();

      /* Compose consolidated response */
      switch( svlte_mode_pref )
      {
        case QMI_PROXY_MODE_PREF_CDMA_ONLY:
        case QMI_PROXY_MODE_PREF_GSM_ONLY:
        case QMI_PROXY_MODE_PREF_WCDMA_ONLY:
        case QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO:
        case QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED:
          *nas_get_sys_sel_pref_resp = local_svlte_cache_ptr->nas_get_sys_sel_pref_resp;
          break;

        case QMI_PROXY_MODE_PREF_EVDO_ONLY:
        case QMI_PROXY_MODE_PREF_LTE_ONLY:
          *nas_get_sys_sel_pref_resp = remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp;
          break;

        default:
          *nas_get_sys_sel_pref_resp = local_svlte_cache_ptr->nas_get_sys_sel_pref_resp;
          /* Mode preference is the combo of both modems. Every other settings should be the same for both modems */
          nas_get_sys_sel_pref_resp->mode_pref_valid |=
            remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref_valid;
          nas_get_sys_sel_pref_resp->mode_pref |=
            remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref;
          break;
      }

      QMI_PROXY_DEBUG_MSG( "Resp mode_pref_valid = %d, mode_pref = 0x%x\n",
                           nas_get_sys_sel_pref_resp->mode_pref_valid,
                           nas_get_sys_sel_pref_resp->mode_pref );
    }
  }
  else
  {
    QMI_PROXY_DEBUG_MSG( "Fail to query %s(%d) sys sel pref, qmi_err %d, qmi_result %d\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type,
                         nas_get_sys_sel_pref_resp->resp.error, nas_get_sys_sel_pref_resp->resp.result );
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_nas_resp_proc_nas_get_sys_sel_pref */


/*=========================================================================
  FUNCTION:  qmi_proxy_nas_resp_proc_nas_get_sys_sel_pref_sglte

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per received
    QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_resp_proc_nas_get_sys_sel_pref_sglte
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  nas_get_system_selection_preference_resp_msg_v01 *nas_get_sys_sel_pref_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  qmi_proxy_mode_pref_type sglte_mode_pref;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  (void) user_data;

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Supress compiler warnings for unused variables */
  (void) proxy_srvc_id;
  (void) proxy_client_id;
  (void) proxy_msg_id;
  (void) resp_c_struct_len;
  (void) user_data_valid;
  (void) resp_modified;
  (void) modified_resp_c_struct;
  (void) modified_resp_c_struct_len;

  nas_get_sys_sel_pref_resp = ( nas_get_system_selection_preference_resp_msg_v01 * ) resp_c_struct;

  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, nas_get_sys_sel_pref_resp ) )
  {
    memset( &svlte_cache_ptr->nas_get_sys_sel_pref_resp, 0, sizeof( nas_get_system_selection_preference_resp_msg_v01 ) );

    if ( nas_get_sys_sel_pref_resp != NULL )
    {
      QMI_PROXY_DEBUG_MSG( "Query %s(%d) mode_pref_valid = %d, mode_pref = 0x%x\n",
                           qmi_proxy_modem_name[ conn_type ], conn_type,
                           nas_get_sys_sel_pref_resp->mode_pref_valid,
                           nas_get_sys_sel_pref_resp->mode_pref );

      svlte_cache_ptr->nas_get_sys_sel_pref_resp = *nas_get_sys_sel_pref_resp;
    }
    else
    {
      /* Ideally response should not be NULL as it has mandatory field, keeping klockwork happy */
      QMI_PROXY_ERR_MSG( "Response is NULL for srvc_id = %d, msg_id = 0x%x\n", srvc_id, msg_id );
      return QMI_INTERNAL_ERR;
    }

    /* Need to update SGLTE mode pref */
    if ( last_txn )
    {
      /* Update SGLTE mode preference from cache */
      sglte_mode_pref = qmi_proxy_update_cache_sglte_mode_pref_from_resp();

      /* Compose consolidated response */
      *nas_get_sys_sel_pref_resp = local_svlte_cache_ptr->nas_get_sys_sel_pref_resp;
      /* Mode preference is the mode_pref that we have cached. Send mode pref only if either modem is sending */
      nas_get_sys_sel_pref_resp->mode_pref_valid |=
        remote_svlte_cache_ptr->nas_get_sys_sel_pref_resp.mode_pref_valid;
      nas_get_sys_sel_pref_resp->mode_pref =
        qmi_proxy_user_mode_pref_to_ril_nas_mask(sglte_mode_pref);


      QMI_PROXY_DEBUG_MSG( "Resp mode_pref_valid = %d, mode_pref = 0x%x\n",
                           nas_get_sys_sel_pref_resp->mode_pref_valid,
                           nas_get_sys_sel_pref_resp->mode_pref );
    }
  }
  else
  {
    QMI_PROXY_DEBUG_MSG( "Fail to query %s(%d) sys sel pref, qmi_err %d, qmi_result %d\n",
                         qmi_proxy_modem_name[ conn_type ], conn_type,
                         nas_get_sys_sel_pref_resp->resp.error, nas_get_sys_sel_pref_resp->resp.result );
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_nas_resp_proc_nas_get_sys_sel_pref_sglte */


/*=========================================================================
  FUNCTION:  qmi_proxy_nas_resp_proc_nas_get_rf_band_info

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per received
    QMI_NAS_GET_RF_BAND_INFO_RESP_MSG.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_resp_proc_nas_get_rf_band_info
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  nas_get_rf_band_info_resp_msg_v01 *nas_get_rf_band_info_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  uint32 i,j;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Supress compiler warnings for unused variables */
  (void) proxy_srvc_id;
  (void) proxy_client_id;
  (void) proxy_msg_id;
  (void) resp_c_struct_len;
  (void) user_data_valid;
  (void) user_data;
  (void) resp_modified;
  (void) modified_resp_c_struct;
  (void) modified_resp_c_struct_len;

  nas_get_rf_band_info_resp = ( nas_get_rf_band_info_resp_msg_v01 * ) resp_c_struct;

  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, nas_get_rf_band_info_resp ) )
  {
    memset( &svlte_cache_ptr->nas_get_rf_band_info_resp, 0, sizeof( nas_get_rf_band_info_resp_msg_v01 ) );

    if ( nas_get_rf_band_info_resp != NULL )
    {
      svlte_cache_ptr->nas_get_rf_band_info_resp = *nas_get_rf_band_info_resp;
    }
    else
    {
      /* Ideally response should not be NULL as it has mandatory field, keeping klockwork happy */
      QMI_PROXY_ERR_MSG( "Response is NULL for srvc_id = %d, msg_id = 0x%x\n", srvc_id, msg_id );
      return QMI_INTERNAL_ERR;
    }

    if ( last_txn )
    {
      /* Aggregate response */
      *nas_get_rf_band_info_resp = local_svlte_cache_ptr->nas_get_rf_band_info_resp;

      for ( i = 0, j = nas_get_rf_band_info_resp->rf_band_info_list_len;
            i < remote_svlte_cache_ptr->nas_get_rf_band_info_resp.rf_band_info_list_len &&
            j < NAS_RF_BAND_INFO_LIST_MAX_V01;
            i++, j++ )
      {
        nas_get_rf_band_info_resp->rf_band_info_list_len += 1;
        nas_get_rf_band_info_resp->rf_band_info_list[ j ] =
          remote_svlte_cache_ptr->nas_get_rf_band_info_resp.rf_band_info_list[ i ];
      }
    }
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_nas_resp_proc_nas_get_rf_band_info */


/*=========================================================================
  FUNCTION:  qmi_proxy_nas_resp_proc_nas_get_opr_nam

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per received
    QMI_NAS_GET_OPERATOR_NAME_DATA_RESP_MSG.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_resp_proc_nas_get_opr_nam
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  nas_get_operator_name_data_resp_msg_v01 *nas_get_opr_nam_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Supress compiler warnings for unused variables */
  (void) proxy_srvc_id;
  (void) proxy_client_id;
  (void) proxy_msg_id;
  (void) resp_c_struct_len;
  (void) user_data_valid;
  (void) resp_modified;
  (void) modified_resp_c_struct;
  (void) modified_resp_c_struct_len;

  nas_get_opr_nam_resp = ( nas_get_operator_name_data_resp_msg_v01 * ) resp_c_struct;

  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, nas_get_opr_nam_resp ) )
  {
    memset( &svlte_cache_ptr->nas_get_opr_nam_resp, 0, sizeof( nas_get_operator_name_data_resp_msg_v01 ) );

    if ( nas_get_opr_nam_resp != NULL )
    {
      svlte_cache_ptr->nas_get_opr_nam_resp = *nas_get_opr_nam_resp;
    }
    else
    {
      /* Ideally response should not be NULL as it has mandatory field, keeping klockwork happy */
      QMI_PROXY_ERR_MSG( "Response is NULL for srvc_id = %d, msg_id = 0x%x\n", srvc_id, msg_id );
      return QMI_INTERNAL_ERR;
    }

    if ( last_txn )
    {
      switch( user_data )
      {
        case QMI_PROXY_LOCAL_CONN_TYPE:
          *nas_get_opr_nam_resp = local_svlte_cache_ptr->nas_get_opr_nam_resp;
          break;

        case QMI_PROXY_REMOTE_CONN_TYPE:
          *nas_get_opr_nam_resp = remote_svlte_cache_ptr->nas_get_opr_nam_resp;
          break;

        default:
          *nas_get_opr_nam_resp = local_svlte_cache_ptr->nas_get_opr_nam_resp;

          /* Only modem camped network has the NITZ info */
          if ( !nas_get_opr_nam_resp->nitz_information_valid &&
               remote_svlte_cache_ptr->nas_get_opr_nam_resp.nitz_information_valid )
          {
            nas_get_opr_nam_resp->nitz_information_valid = TRUE;
            nas_get_opr_nam_resp->nitz_information = remote_svlte_cache_ptr->nas_get_opr_nam_resp.nitz_information;
          }
      }
    }
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_nas_resp_proc_nas_get_opr_nam */


/*=========================================================================
  FUNCTION:  qmi_proxy_nas_resp_proc_nas_get_sig_info

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per received
    QMI_NAS_GET_SIG_INFO_RESP_MSG.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_resp_proc_nas_get_sig_info
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  nas_get_sig_info_resp_msg_v01 *nas_get_sig_info_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Supress compiler warnings for unused variables */
  (void) proxy_srvc_id;
  (void) proxy_client_id;
  (void) proxy_msg_id;
  (void) resp_c_struct_len;
  (void) user_data_valid;
  (void) user_data;
  (void) resp_modified;
  (void) modified_resp_c_struct;
  (void) modified_resp_c_struct_len;

  nas_get_sig_info_resp = ( nas_get_sig_info_resp_msg_v01 * ) resp_c_struct;

  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, nas_get_sig_info_resp ) )
  {
    memset( &svlte_cache_ptr->nas_get_sig_info_resp, 0, sizeof( nas_get_sig_info_resp_msg_v01 ) );

    if ( nas_get_sig_info_resp != NULL )
    {
      svlte_cache_ptr->nas_get_sig_info_resp = *nas_get_sig_info_resp;
    }
    else
    {
      /* Ideally response should not be NULL as it has mandatory field, keeping klockwork happy */
      QMI_PROXY_ERR_MSG( "Response is NULL for srvc_id = %d, msg_id = 0x%x\n", srvc_id, msg_id );
      return QMI_INTERNAL_ERR;
    }

    if ( last_txn )
     {
      /* Sig info from local modem */
      if ( local_svlte_cache_ptr->nas_get_sig_info_resp.cdma_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Local modem CDMA sig reported" );
      }

      if ( local_svlte_cache_ptr->nas_get_sig_info_resp.hdr_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Local modem HDR sig reported" );
      }

      if ( local_svlte_cache_ptr->nas_get_sig_info_resp.gsm_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Local modem GSM sig reported" );
      }

      if ( local_svlte_cache_ptr->nas_get_sig_info_resp.wcdma_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Local modem WCDMA sig reported" );
      }

      if ( local_svlte_cache_ptr->nas_get_sig_info_resp.rscp_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Local modem TDSCDMA sig reported" );
      }

      if ( local_svlte_cache_ptr->nas_get_sig_info_resp.lte_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Local modem LTE sig reported" );
      }

      /* Sig info from remote modem */
      if ( remote_svlte_cache_ptr->nas_get_sig_info_resp.cdma_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Remote modem CDMA sig reported" );
      }

      if ( remote_svlte_cache_ptr->nas_get_sig_info_resp.hdr_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Remote modem HDR sig reported" );
      }

      if ( remote_svlte_cache_ptr->nas_get_sig_info_resp.gsm_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Remote modem GSM sig reported" );
      }

      if ( remote_svlte_cache_ptr->nas_get_sig_info_resp.wcdma_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Remote modem WCDMA sig reported" );
      }

      if ( remote_svlte_cache_ptr->nas_get_sig_info_resp.rscp_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Remote modem TDSCDMA sig reported" );
      }

      if ( remote_svlte_cache_ptr->nas_get_sig_info_resp.lte_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Remote modem LTE sig reported" );
      }

      /* Aggregate response */
      *nas_get_sig_info_resp = local_svlte_cache_ptr->nas_get_sig_info_resp;

      if ( !nas_get_sig_info_resp->cdma_sig_info_valid &&
           remote_svlte_cache_ptr->nas_get_sig_info_resp.cdma_sig_info_valid )
      {
        nas_get_sig_info_resp->cdma_sig_info_valid = TRUE;
        nas_get_sig_info_resp->cdma_sig_info = remote_svlte_cache_ptr->nas_get_sig_info_resp.cdma_sig_info;
      }

      if ( !nas_get_sig_info_resp->hdr_sig_info_valid &&
           remote_svlte_cache_ptr->nas_get_sig_info_resp.hdr_sig_info_valid )
      {
        nas_get_sig_info_resp->hdr_sig_info_valid = TRUE;
        nas_get_sig_info_resp->hdr_sig_info = remote_svlte_cache_ptr->nas_get_sig_info_resp.hdr_sig_info;
      }

      if ( !nas_get_sig_info_resp->gsm_sig_info_valid &&
           remote_svlte_cache_ptr->nas_get_sig_info_resp.gsm_sig_info_valid )
      {
        nas_get_sig_info_resp->gsm_sig_info_valid = TRUE;
        nas_get_sig_info_resp->gsm_sig_info = remote_svlte_cache_ptr->nas_get_sig_info_resp.gsm_sig_info;
      }

      if ( !nas_get_sig_info_resp->wcdma_sig_info_valid &&
           remote_svlte_cache_ptr->nas_get_sig_info_resp.wcdma_sig_info_valid )
      {
        nas_get_sig_info_resp->wcdma_sig_info_valid = TRUE;
        nas_get_sig_info_resp->wcdma_sig_info = remote_svlte_cache_ptr->nas_get_sig_info_resp.wcdma_sig_info;
      }

      if ( !nas_get_sig_info_resp->rscp_valid &&
           remote_svlte_cache_ptr->nas_get_sig_info_resp.rscp_valid )
      {
        nas_get_sig_info_resp->rscp_valid = TRUE;
        nas_get_sig_info_resp->rscp = remote_svlte_cache_ptr->nas_get_sig_info_resp.rscp;
      }

      if ( !nas_get_sig_info_resp->lte_sig_info_valid &&
           remote_svlte_cache_ptr->nas_get_sig_info_resp.lte_sig_info_valid )
      {
        nas_get_sig_info_resp->lte_sig_info_valid = TRUE;
        nas_get_sig_info_resp->lte_sig_info = remote_svlte_cache_ptr->nas_get_sig_info_resp.lte_sig_info;
      }

      /* Consolidated Sig info */
      if ( nas_get_sig_info_resp->cdma_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Consolidated CDMA sig reported" );
      }

      if ( nas_get_sig_info_resp->hdr_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Consolidated HDR sig reported" );
      }

      if ( nas_get_sig_info_resp->gsm_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Consolidated GSM sig reported" );
      }

      if ( nas_get_sig_info_resp->wcdma_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Consolidated WCDMA sig reported" );
      }

      if ( nas_get_sig_info_resp->rscp_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Consolidated TDSCDMA sig reported" );
      }

      if ( nas_get_sig_info_resp->lte_sig_info_valid )
      {
        QMI_PROXY_DEBUG_MSG( "%s\n", "Consolidated LTE sig reported" );
      }
    }
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_nas_resp_proc_nas_get_sig_info */


/*=========================================================================
  FUNCTION:  qmi_proxy_nas_resp_proc_nas_get_err_rate

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per received
    QMI_NAS_GET_ERR_RATE_RESP_MSG.

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_resp_proc_nas_get_err_rate
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  nas_get_err_rate_resp_msg_v01 *nas_get_err_rate_resp = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ conn_type ];
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

  /* Supress compiler warnings for unused variables */
  (void) proxy_srvc_id;
  (void) proxy_client_id;
  (void) proxy_msg_id;
  (void) resp_c_struct_len;
  (void) user_data_valid;
  (void) user_data;
  (void) resp_modified;
  (void) modified_resp_c_struct;
  (void) modified_resp_c_struct_len;

  nas_get_err_rate_resp = ( nas_get_err_rate_resp_msg_v01 * ) resp_c_struct;

  if ( QMI_PROXY_RESPONSE_SUCCESS( rc, nas_get_err_rate_resp ) )
  {
    memset( &svlte_cache_ptr->nas_get_err_rate_resp, 0, sizeof( nas_get_err_rate_resp_msg_v01 ) );

    if ( nas_get_err_rate_resp != NULL )
    {
      svlte_cache_ptr->nas_get_err_rate_resp = *nas_get_err_rate_resp;
    }
    else
    {
      /* Ideally response should not be NULL as it has mandatory field, keeping klockwork happy */
      QMI_PROXY_ERR_MSG( "Response is NULL for srvc_id = %d, msg_id = 0x%x\n", srvc_id, msg_id );
      return QMI_INTERNAL_ERR;
    }

    if ( last_txn )
    {
      /* Aggregate response */
      *nas_get_err_rate_resp = local_svlte_cache_ptr->nas_get_err_rate_resp;

      if ( !nas_get_err_rate_resp->cdma_frame_err_rate_valid &&
           remote_svlte_cache_ptr->nas_get_err_rate_resp.cdma_frame_err_rate_valid )
      {
        nas_get_err_rate_resp->cdma_frame_err_rate_valid = TRUE;
        nas_get_err_rate_resp->cdma_frame_err_rate = remote_svlte_cache_ptr->nas_get_err_rate_resp.cdma_frame_err_rate;
      }

      if ( !nas_get_err_rate_resp->hdr_packet_err_rate_valid &&
           remote_svlte_cache_ptr->nas_get_err_rate_resp.hdr_packet_err_rate_valid )
      {
        nas_get_err_rate_resp->hdr_packet_err_rate_valid = TRUE;
        nas_get_err_rate_resp->hdr_packet_err_rate = remote_svlte_cache_ptr->nas_get_err_rate_resp.hdr_packet_err_rate;
      }

      if ( !nas_get_err_rate_resp->gsm_bit_err_rate_valid &&
           remote_svlte_cache_ptr->nas_get_err_rate_resp.gsm_bit_err_rate_valid )
      {
        nas_get_err_rate_resp->gsm_bit_err_rate_valid = TRUE;
        nas_get_err_rate_resp->gsm_bit_err_rate = remote_svlte_cache_ptr->nas_get_err_rate_resp.gsm_bit_err_rate;
      }

      if ( !nas_get_err_rate_resp->wcdma_block_err_rate_valid &&
           remote_svlte_cache_ptr->nas_get_err_rate_resp.wcdma_block_err_rate_valid )
      {
        nas_get_err_rate_resp->wcdma_block_err_rate_valid = TRUE;
        nas_get_err_rate_resp->wcdma_block_err_rate = remote_svlte_cache_ptr->nas_get_err_rate_resp.wcdma_block_err_rate;
      }

      if ( !nas_get_err_rate_resp->tdscdma_block_err_rate_valid &&
           remote_svlte_cache_ptr->nas_get_err_rate_resp.tdscdma_block_err_rate_valid )
      {
        nas_get_err_rate_resp->tdscdma_block_err_rate_valid = TRUE;
        nas_get_err_rate_resp->tdscdma_block_err_rate = remote_svlte_cache_ptr->nas_get_err_rate_resp.tdscdma_block_err_rate;
      }
    }
  }

  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

  return rc;

} /* qmi_proxy_nas_resp_proc_nas_get_err_rate */


/*=========================================================================
  FUNCTION:  qmi_proxy_nas_srvc_response_update_Cache

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SVLTE cache per NAS service response message

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_srvc_response_update_cache
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "%s ....... QMI %s(%d), %s(%d), Response, Msg ID 0x%x\n",
                       __FUNCTION__,
                       qmi_proxy_srvc_name[ srvc_id ],
                       srvc_id,
                       qmi_proxy_modem_name[ conn_type ],
                       conn_type,
                       msg_id );

  if ( srvc_id == QMI_DMS_SERVICE )
  {
    switch ( msg_id )
    {
      /* This for NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ */
      case QMI_DMS_SET_OPERATING_MODE_RESP_V01:
        rc = qmi_proxy_nas_resp_proc_dms_set_oprt_mode( proxy_srvc_id,
                                                        proxy_client_id,
                                                        proxy_msg_id,
                                                        conn_type,
                                                        srvc_id,
                                                        msg_id,
                                                        resp_c_struct,
                                                        resp_c_struct_len,
                                                        last_txn,
                                                        user_data_valid,
                                                        user_data,
                                                        resp_modified,
                                                        modified_resp_c_struct,
                                                        modified_resp_c_struct_len );
        break;

      /* This for NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ */
      case QMI_DMS_GET_OPERATING_MODE_RESP_V01:
        rc = qmi_proxy_nas_resp_proc_dms_get_oprt_mode( proxy_srvc_id,
                                                        proxy_client_id,
                                                        proxy_msg_id,
                                                        conn_type,
                                                        srvc_id,
                                                        msg_id,
                                                        resp_c_struct,
                                                        resp_c_struct_len,
                                                        last_txn,
                                                        user_data_valid,
                                                        user_data,
                                                        resp_modified,
                                                        modified_resp_c_struct,
                                                        modified_resp_c_struct_len );
        break;

      default:
        break;
    }
  }
  else
  {
    switch(  msg_id )
    {
      case QMI_NAS_GET_SYS_INFO_RESP_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_get_sys_info( proxy_srvc_id,
                                                       proxy_client_id,
                                                       proxy_msg_id,
                                                       conn_type,
                                                       srvc_id,
                                                       msg_id,
                                                       resp_c_struct,
                                                       resp_c_struct_len,
                                                       last_txn,
                                                       user_data_valid,
                                                       user_data,
                                                       resp_modified,
                                                       modified_resp_c_struct,
                                                       modified_resp_c_struct_len );
        break;

      case QMI_NAS_PERFORM_NETWORK_SCAN_RESP_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_per_net_scan( proxy_srvc_id,
                                                       proxy_client_id,
                                                       proxy_msg_id,
                                                       conn_type,
                                                       srvc_id,
                                                       msg_id,
                                                       resp_c_struct,
                                                       resp_c_struct_len,
                                                       last_txn,
                                                       user_data_valid,
                                                       user_data,
                                                       resp_modified,
                                                       modified_resp_c_struct,
                                                       modified_resp_c_struct_len );
        break;

      case QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_set_sys_sel_pref( proxy_srvc_id,
                                                           proxy_client_id,
                                                           proxy_msg_id,
                                                           conn_type,
                                                           srvc_id,
                                                           msg_id,
                                                           resp_c_struct,
                                                           resp_c_struct_len,
                                                           last_txn,
                                                           user_data_valid,
                                                           user_data,
                                                           resp_modified,
                                                           modified_resp_c_struct,
                                                           modified_resp_c_struct_len );
        break;

      case QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_get_sys_sel_pref( proxy_srvc_id,
                                                           proxy_client_id,
                                                           proxy_msg_id,
                                                           conn_type,
                                                           srvc_id,
                                                           msg_id,
                                                           resp_c_struct,
                                                           resp_c_struct_len,
                                                           last_txn,
                                                           user_data_valid,
                                                           user_data,
                                                           resp_modified,
                                                           modified_resp_c_struct,
                                                           modified_resp_c_struct_len );
        break;

      case QMI_NAS_GET_RF_BAND_INFO_RESP_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_get_rf_band_info( proxy_srvc_id,
                                                           proxy_client_id,
                                                           proxy_msg_id,
                                                           conn_type,
                                                           srvc_id,
                                                           msg_id,
                                                           resp_c_struct,
                                                           resp_c_struct_len,
                                                           last_txn,
                                                           user_data_valid,
                                                           user_data,
                                                           resp_modified,
                                                           modified_resp_c_struct,
                                                           modified_resp_c_struct_len );
        break;

      case QMI_NAS_GET_OPERATOR_NAME_DATA_RESP_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_get_opr_nam( proxy_srvc_id,
                                                      proxy_client_id,
                                                      proxy_msg_id,
                                                      conn_type,
                                                      srvc_id,
                                                      msg_id,
                                                      resp_c_struct,
                                                      resp_c_struct_len,
                                                      last_txn,
                                                      user_data_valid,
                                                      user_data,
                                                      resp_modified,
                                                      modified_resp_c_struct,
                                                      modified_resp_c_struct_len );
        break;

      case QMI_NAS_GET_SIG_INFO_REQ_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_get_sig_info( proxy_srvc_id,
                                                       proxy_client_id,
                                                       proxy_msg_id,
                                                       conn_type,
                                                       srvc_id,
                                                       msg_id,
                                                       resp_c_struct,
                                                       resp_c_struct_len,
                                                       last_txn,
                                                       user_data_valid,
                                                       user_data,
                                                       resp_modified,
                                                       modified_resp_c_struct,
                                                       modified_resp_c_struct_len );
        break;

      case QMI_NAS_GET_ERR_RATE_REQ_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_get_err_rate( proxy_srvc_id,
                                                       proxy_client_id,
                                                       proxy_msg_id,
                                                       conn_type,
                                                       srvc_id,
                                                       msg_id,
                                                       resp_c_struct,
                                                       resp_c_struct_len,
                                                       last_txn,
                                                       user_data_valid,
                                                       user_data,
                                                       resp_modified,
                                                       modified_resp_c_struct,
                                                       modified_resp_c_struct_len );
        break;

      default:
        break;
    }
  }

  return rc;

} /* qmi_proxy_nas_srvc_response_update_cache */


/*=========================================================================
  FUNCTION:  qmi_proxy_nas_srvc_response_update_cache_sglte

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SGLTE cache per NAS service response message

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_nas_srvc_response_update_cache_sglte
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "%s ....... QMI %s(%d), %s(%d), Response, Msg ID 0x%x\n",
                       __FUNCTION__,
                       qmi_proxy_srvc_name[ srvc_id ],
                       srvc_id,
                       qmi_proxy_modem_name[ conn_type ],
                       conn_type,
                       msg_id );

  if ( srvc_id == QMI_DMS_SERVICE )
  {
    switch ( msg_id )
    {
      /* This for NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ */
      case QMI_DMS_SET_OPERATING_MODE_RESP_V01:
        rc = qmi_proxy_nas_resp_proc_dms_set_oprt_mode( proxy_srvc_id,
                                                        proxy_client_id,
                                                        proxy_msg_id,
                                                        conn_type,
                                                        srvc_id,
                                                        msg_id,
                                                        resp_c_struct,
                                                        resp_c_struct_len,
                                                        last_txn,
                                                        user_data_valid,
                                                        user_data,
                                                        resp_modified,
                                                        modified_resp_c_struct,
                                                        modified_resp_c_struct_len );
        break;

      /* This for NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ */
      case QMI_DMS_GET_OPERATING_MODE_RESP_V01:
        rc = qmi_proxy_nas_resp_proc_dms_get_oprt_mode( proxy_srvc_id,
                                                        proxy_client_id,
                                                        proxy_msg_id,
                                                        conn_type,
                                                        srvc_id,
                                                        msg_id,
                                                        resp_c_struct,
                                                        resp_c_struct_len,
                                                        last_txn,
                                                        user_data_valid,
                                                        user_data,
                                                        resp_modified,
                                                        modified_resp_c_struct,
                                                        modified_resp_c_struct_len );
        break;

      default:
        break;
    }
  }
  else
  {
    switch(  msg_id )
    {
      case QMI_NAS_GET_SYS_INFO_RESP_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_get_sys_info_sglte( proxy_srvc_id,
                                                       proxy_client_id,
                                                       proxy_msg_id,
                                                       conn_type,
                                                       srvc_id,
                                                       msg_id,
                                                       resp_c_struct,
                                                       resp_c_struct_len,
                                                       last_txn,
                                                       user_data_valid,
                                                       user_data,
                                                       resp_modified,
                                                       modified_resp_c_struct,
                                                       modified_resp_c_struct_len );
        break;

      case QMI_NAS_PERFORM_NETWORK_SCAN_RESP_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_per_net_scan( proxy_srvc_id,
                                                       proxy_client_id,
                                                       proxy_msg_id,
                                                       conn_type,
                                                       srvc_id,
                                                       msg_id,
                                                       resp_c_struct,
                                                       resp_c_struct_len,
                                                       last_txn,
                                                       user_data_valid,
                                                       user_data,
                                                       resp_modified,
                                                       modified_resp_c_struct,
                                                       modified_resp_c_struct_len );
        break;

      case QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_set_sys_sel_pref( proxy_srvc_id,
                                                           proxy_client_id,
                                                           proxy_msg_id,
                                                           conn_type,
                                                           srvc_id,
                                                           msg_id,
                                                           resp_c_struct,
                                                           resp_c_struct_len,
                                                           last_txn,
                                                           user_data_valid,
                                                           user_data,
                                                           resp_modified,
                                                           modified_resp_c_struct,
                                                           modified_resp_c_struct_len );
        break;

      case QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_get_sys_sel_pref_sglte( proxy_srvc_id,
                                                           proxy_client_id,
                                                           proxy_msg_id,
                                                           conn_type,
                                                           srvc_id,
                                                           msg_id,
                                                           resp_c_struct,
                                                           resp_c_struct_len,
                                                           last_txn,
                                                           user_data_valid,
                                                           user_data,
                                                           resp_modified,
                                                           modified_resp_c_struct,
                                                           modified_resp_c_struct_len );
        break;

      case QMI_NAS_GET_RF_BAND_INFO_RESP_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_get_rf_band_info( proxy_srvc_id,
                                                           proxy_client_id,
                                                           proxy_msg_id,
                                                           conn_type,
                                                           srvc_id,
                                                           msg_id,
                                                           resp_c_struct,
                                                           resp_c_struct_len,
                                                           last_txn,
                                                           user_data_valid,
                                                           user_data,
                                                           resp_modified,
                                                           modified_resp_c_struct,
                                                           modified_resp_c_struct_len );
        break;

      case QMI_NAS_GET_OPERATOR_NAME_DATA_RESP_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_get_opr_nam( proxy_srvc_id,
                                                      proxy_client_id,
                                                      proxy_msg_id,
                                                      conn_type,
                                                      srvc_id,
                                                      msg_id,
                                                      resp_c_struct,
                                                      resp_c_struct_len,
                                                      last_txn,
                                                      user_data_valid,
                                                      user_data,
                                                      resp_modified,
                                                      modified_resp_c_struct,
                                                      modified_resp_c_struct_len );
        break;

      case QMI_NAS_GET_SIG_INFO_REQ_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_get_sig_info( proxy_srvc_id,
                                                       proxy_client_id,
                                                       proxy_msg_id,
                                                       conn_type,
                                                       srvc_id,
                                                       msg_id,
                                                       resp_c_struct,
                                                       resp_c_struct_len,
                                                       last_txn,
                                                       user_data_valid,
                                                       user_data,
                                                       resp_modified,
                                                       modified_resp_c_struct,
                                                       modified_resp_c_struct_len );
        break;

      case QMI_NAS_GET_ERR_RATE_REQ_MSG_V01:
        rc = qmi_proxy_nas_resp_proc_nas_get_err_rate( proxy_srvc_id,
                                                       proxy_client_id,
                                                       proxy_msg_id,
                                                       conn_type,
                                                       srvc_id,
                                                       msg_id,
                                                       resp_c_struct,
                                                       resp_c_struct_len,
                                                       last_txn,
                                                       user_data_valid,
                                                       user_data,
                                                       resp_modified,
                                                       modified_resp_c_struct,
                                                       modified_resp_c_struct_len );
        break;

      default:
        break;
    }
  }

  return rc;

} /* qmi_proxy_nas_srvc_response_update_cache_sglte */

/*=========================================================================
  FUNCTION:  qmi_proxy_wms_srvc_response_update_cache_sglte

===========================================================================*/
/*!
    @brief
    Update QMI Proxy internal SGLTE cache per WMS service response message

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_wms_srvc_response_update_cache_sglte
(
  qmi_service_id_type proxy_srvc_id,
  qmi_client_id_type  proxy_client_id,
  unsigned long       proxy_msg_id,
  qmi_proxy_conn_type conn_type,
  qmi_service_id_type srvc_id,
  unsigned long       msg_id,
  void                *resp_c_struct,
  int                 resp_c_struct_len,
  boolean             last_txn,
  boolean             user_data_valid,
  uint32              user_data,
  boolean             *resp_modified,
  void                **modified_resp_c_struct,
  int                 *modified_resp_c_struct_len
)
{
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "%s ....... QMI %s(%d), %s(%d), Response, Msg ID 0x%x\n",
                       __FUNCTION__,
                       qmi_proxy_srvc_name[ srvc_id ],
                       srvc_id,
                       qmi_proxy_modem_name[ conn_type ],
                       conn_type,
                       msg_id );

  if ( srvc_id == QMI_WMS_SERVICE )
  {
    switch ( msg_id )
    {
      /* This for QMI_WMS_GET_INDICATION_REGISTER_REQ_V01 */
      case QMI_WMS_GET_INDICATION_REGISTER_RESP_V01:
        rc = qmi_proxy_wms_resp_proc_wms_get_indication_register( proxy_srvc_id,
                                                        proxy_client_id,
                                                        proxy_msg_id,
                                                        conn_type,
                                                        srvc_id,
                                                        msg_id,
                                                        resp_c_struct,
                                                        resp_c_struct_len,
                                                        last_txn,
                                                        user_data_valid,
                                                        user_data,
                                                        resp_modified,
                                                        modified_resp_c_struct,
                                                        modified_resp_c_struct_len );
        break;

      /* This for QMI_WMS_GET_SERVICE_READY_STATUS_REQ_V01 */
      case QMI_WMS_GET_SERVICE_READY_STATUS_RESP_V01:
        rc = qmi_proxy_wms_resp_proc_wms_get_service_ready_status( proxy_srvc_id,
                                                        proxy_client_id,
                                                        proxy_msg_id,
                                                        conn_type,
                                                        srvc_id,
                                                        msg_id,
                                                        resp_c_struct,
                                                        resp_c_struct_len,
                                                        last_txn,
                                                        user_data_valid,
                                                        user_data,
                                                        resp_modified,
                                                        modified_resp_c_struct,
                                                        modified_resp_c_struct_len );
        break;

      default:
        break;
    }
  }

  return rc;
} /* qmi_proxy_wms_srvc_response_update_cache_sglte */


/*=========================================================================
  FUNCTION:  qmi_proxy_do_force_sglte_sys_sel_pref_and_oprt_mode

===========================================================================*/
/*!
    @brief
    Set modems to appropriate operating mode and mode pref based on
    current sglte parameters.

    @return
    nothing
*/
/*=========================================================================*/
static void *qmi_proxy_do_force_sglte_sys_sel_pref_and_oprt_mode
(
  void *arg
)
{
  qmi_proxy_conn_type cs_active;
  qmi_proxy_conn_type ps_active;
  qmi_proxy_mode_pref_type user_mode_pref;
  int is_sglte_plmn;
  int is_user_request;
  int local_online;
  int remote_online;
  int rc;
  int is_mode_pref_required;
  nas_set_system_selection_preference_req_msg_v01 *local_nas_set_sys_sel_pref_req = NULL;
  nas_set_system_selection_preference_req_msg_v01 *remote_nas_set_sys_sel_pref_req = NULL;
  nas_set_system_selection_preference_resp_msg_v01 *local_nas_set_sys_sel_pref_resp = NULL;
  nas_set_system_selection_preference_resp_msg_v01 *remote_nas_set_sys_sel_pref_resp = NULL;
  int free_local_response = TRUE;
  int free_remote_response = TRUE;
#define QMI_PROXY_SYNC_OPRT_MODE_TIMEOUT 5000
#define QMI_PROXY_SYNC_SYS_SEL_PREF_TIMEOUT 50000
  int oprt_mode_timeout = QMI_PROXY_SYNC_OPRT_MODE_TIMEOUT;
  int sys_sel_pref_timeout = QMI_PROXY_SYNC_SYS_SEL_PREF_TIMEOUT;

  qmi_proxy_force_sglte_sys_sel_pref_and_oprt_mode_params *params =
          (qmi_proxy_force_sglte_sys_sel_pref_and_oprt_mode_params *) arg;

  if ( DMS_OP_MODE_LOW_POWER_V01 == qmi_proxy_internal_info.desired_mode )
  {
    /* Do nothing as we are in airplane mode */
  }
  else if (arg)
  {
    cs_active = params->cs_active_modem;
    ps_active = params->ps_active_modem;
    user_mode_pref = params->user_mode_pref;
    is_sglte_plmn = params->is_sglte_plmn;
    is_user_request = params->is_user_request;
    is_mode_pref_required = params->is_mode_pref_required;

    local_online = cs_active == QMI_PROXY_LOCAL_CONN_TYPE ||
                   ps_active == QMI_PROXY_LOCAL_CONN_TYPE;

    remote_online = cs_active == QMI_PROXY_REMOTE_CONN_TYPE ||
                    ps_active == QMI_PROXY_REMOTE_CONN_TYPE;

    if ( qmi_proxy_internal_info.number_of_emergency_calls &&
            cs_active == QMI_PROXY_REMOTE_CONN_TYPE &&
            local_online )
    {
      QMI_PROXY_DEBUG_MSG("%s", "Emergency calls are active, do not put Local Modem to Online");
      local_online = FALSE;
    }

    QMI_PROXY_DEBUG_MSG("%s: cs_active = %d,ps_active = %d,"
                        "user_mode_pref = %d, is_sglte_plmn = %d, "
                        "local_online = %d, remote_online = %d "
                        "is_user_request = %d",
                        __FUNCTION__,
                        cs_active, ps_active, user_mode_pref,
                        is_sglte_plmn, local_online, remote_online,
                        is_user_request);

    /* Allocate buffers for requests and responses */
    local_nas_set_sys_sel_pref_req = qmi_proxy_malloc(sizeof(*local_nas_set_sys_sel_pref_req));
    remote_nas_set_sys_sel_pref_req = qmi_proxy_malloc(sizeof(*remote_nas_set_sys_sel_pref_req));
    local_nas_set_sys_sel_pref_resp = qmi_proxy_malloc(sizeof(*local_nas_set_sys_sel_pref_resp));
    remote_nas_set_sys_sel_pref_resp = qmi_proxy_malloc(sizeof(*remote_nas_set_sys_sel_pref_resp));

    if ( local_nas_set_sys_sel_pref_req && remote_nas_set_sys_sel_pref_req &&
            local_nas_set_sys_sel_pref_resp && remote_nas_set_sys_sel_pref_resp )
    {
      /* Allocation succeeded */

      /* Prepare request */
      rc = qmi_proxy_set_sys_sel_pref_per_sglte_mode_pref(user_mode_pref,
              is_sglte_plmn,
              is_user_request,
              local_nas_set_sys_sel_pref_req,
              remote_nas_set_sys_sel_pref_req);

      if (rc == QMI_NO_ERR)
      {
        // Mode pref would be already set properly to the modems. We need to set
        // the service domain, reg restriction, ue usage setting only
        if ( !is_mode_pref_required )
        {
          local_nas_set_sys_sel_pref_req->mode_pref_valid = FALSE;
          remote_nas_set_sys_sel_pref_req->mode_pref_valid = FALSE;
        }

        if ( local_online && remote_online )
        {
          do
          {
            QMI_PROXY_DEBUG_MSG("%s: Setting local && remote modems online", __FUNCTION__);

            rc = qmi_proxy_force_online_sync( QMI_PROXY_LOCAL_CONN_TYPE,
                                              oprt_mode_timeout );
            if ( rc != QMI_NO_ERR )
            {
              QMI_PROXY_ERR_MSG("%s: Unable to put local modem online\n", __FUNCTION__);
              break;
            }

            rc = qmi_proxy_force_sys_sel_pref_sync( QMI_PROXY_LOCAL_CONN_TYPE,
                    local_nas_set_sys_sel_pref_req,
                    local_nas_set_sys_sel_pref_resp,
                    sys_sel_pref_timeout );
            if (rc != QMI_NO_ERR)
            {
              QMI_PROXY_ERR_MSG("%s: Unable to send sys_sel_pref request to local modem", __FUNCTION__);
              break;
            }

            rc = qmi_proxy_force_online_sync( QMI_PROXY_REMOTE_CONN_TYPE, oprt_mode_timeout );
            if ( rc != QMI_NO_ERR )
            {
              QMI_PROXY_ERR_MSG("%s: Unable to put remote modem online\n", __FUNCTION__);
              break;
            }

            rc = qmi_proxy_force_sys_sel_pref_sync( QMI_PROXY_REMOTE_CONN_TYPE,
                    remote_nas_set_sys_sel_pref_req,
                    remote_nas_set_sys_sel_pref_resp,
                    sys_sel_pref_timeout );
            if (rc != QMI_NO_ERR)
            {
              QMI_PROXY_ERR_MSG("%s: Unable to send sys_sel_pref_request to remote modem", __FUNCTION__);
              break;
            }
          } while(0);

        }
        else if ( local_online && !remote_online)
        {
          do
          {
            QMI_PROXY_DEBUG_MSG("%s: Setting local modem online && remote lpm", __FUNCTION__);

            //qmi_proxy_sglte_invalidate_last_plmn(QMI_PROXY_REMOTE_CONN_TYPE);
            rc = qmi_proxy_force_lpm_sync( QMI_PROXY_REMOTE_CONN_TYPE,
                                           oprt_mode_timeout );
            if ( rc != QMI_NO_ERR )
            {
              QMI_PROXY_ERR_MSG("%s: Unable to put remote modem in lpm\n", __FUNCTION__);
              break;
            }

            rc = qmi_proxy_force_online_sync( QMI_PROXY_LOCAL_CONN_TYPE,
                                              oprt_mode_timeout );
            if ( rc != QMI_NO_ERR )
            {
              QMI_PROXY_ERR_MSG("%s: Unable to put local modem online\n", __FUNCTION__);
              break;
            }

            rc = qmi_proxy_force_sys_sel_pref_sync( QMI_PROXY_LOCAL_CONN_TYPE,
                    local_nas_set_sys_sel_pref_req,
                    local_nas_set_sys_sel_pref_resp,
                    sys_sel_pref_timeout );
            if ( rc != QMI_NO_ERR )
            {
              QMI_PROXY_ERR_MSG("%s: Unable to send sys_sel_pref_request to local modem\n", __FUNCTION__);
              break;
            }
          } while(0);

        }
        else if ( !local_online && remote_online )
        {
          do
          {
            QMI_PROXY_DEBUG_MSG("%s: Setting local modem lpm && remote online", __FUNCTION__);

            //qmi_proxy_sglte_invalidate_last_plmn(QMI_PROXY_LOCAL_CONN_TYPE);
            rc = qmi_proxy_force_lpm_sync( QMI_PROXY_LOCAL_CONN_TYPE,
                                           oprt_mode_timeout );
            if ( rc != QMI_NO_ERR )
            {
              QMI_PROXY_ERR_MSG("%s: Unable to put local modem in lpm\n", __FUNCTION__);
              break;
            }

            rc = qmi_proxy_force_online_sync( QMI_PROXY_REMOTE_CONN_TYPE,
                                              oprt_mode_timeout );
            if ( rc != QMI_NO_ERR )
            {
              QMI_PROXY_ERR_MSG("%s: Unable to put remote modem online\n", __FUNCTION__);
              break;
            }

            rc = qmi_proxy_force_sys_sel_pref_sync( QMI_PROXY_REMOTE_CONN_TYPE,
                    remote_nas_set_sys_sel_pref_req,
                    remote_nas_set_sys_sel_pref_resp,
                    sys_sel_pref_timeout );
            if ( rc != QMI_NO_ERR )
            {
              QMI_PROXY_ERR_MSG("%s: Unable to send sys_sel_pref_request to remote modem\n", __FUNCTION__);
              break;
            }
          } while (0);
        }
        else
        {
          QMI_PROXY_ERR_MSG("%s: ERROR. Requesting to put bot modems to LPM. This is a bug!!!\n", __FUNCTION__);
        }
      }
      else
      {
        QMI_PROXY_ERR_MSG("%s: Invalid mode pref %d\n", __FUNCTION__, user_mode_pref)
      }
    }
    else
    {
      QMI_PROXY_DEBUG_MSG("%s: allocation failed\n", __FUNCTION__);
    }
  }
  else
  {
    QMI_PROXY_ERR_MSG("%s: Invalid parameters", __FUNCTION__);
  }


  if (arg)
  {
    qmi_proxy_free((void **)&arg);
  }
  if (local_nas_set_sys_sel_pref_req)
  {
    qmi_proxy_free((void **)&local_nas_set_sys_sel_pref_req);
  }
  if (local_nas_set_sys_sel_pref_resp && free_local_response)
  {
    qmi_proxy_free((void **)&local_nas_set_sys_sel_pref_resp);
  }
  if (remote_nas_set_sys_sel_pref_req)
  {
    qmi_proxy_free((void **)&remote_nas_set_sys_sel_pref_req);
  }
  if (remote_nas_set_sys_sel_pref_resp && free_remote_response)
  {
    qmi_proxy_free((void **)&remote_nas_set_sys_sel_pref_resp);
  }
  return NULL;
} /* qmi_proxy_do_force_sglte_mode */

/*=========================================================================
  FUNCTION:  qmi_proxy_force_sglte_sys_sel_pref_and_oprt_mode

===========================================================================*/
/*!
    @brief
    Spawn a thread to send appropriate system selection preference and
    operating mode based on current sglte parameters.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_force_sglte_sys_sel_pref_and_oprt_mode
(
  int cs_active,
  int ps_active,
  qmi_proxy_mode_pref_type user_mode_pref,
  int is_sglte_plmn,
  int is_user_request,
  int is_mode_pref_required,
  int async
)
{
  qmi_proxy_force_sglte_sys_sel_pref_and_oprt_mode_params *params =
          qmi_proxy_malloc(sizeof(qmi_proxy_force_sglte_sys_sel_pref_and_oprt_mode_params));
  pthread_attr_t pth_attr;
  pthread_t thread;

  QMI_PROXY_DEBUG_MSG("async: %d\n", async);
  if (params)
  {
    params->cs_active_modem = cs_active;
    params->ps_active_modem = ps_active;
    params->user_mode_pref = user_mode_pref;
    params->is_sglte_plmn = is_sglte_plmn;
    params->is_user_request = is_user_request;
    params->is_mode_pref_required = is_mode_pref_required;
    if (async)
    {
      pthread_attr_init(&pth_attr);
      pthread_attr_setdetachstate(&pth_attr, PTHREAD_CREATE_DETACHED);
      if (pthread_create(&thread, &pth_attr, qmi_proxy_do_force_sglte_sys_sel_pref_and_oprt_mode, (void *)params))
      {
        QMI_PROXY_ERR_MSG("%s: Unable to create thread", __FUNCTION__);
        qmi_proxy_free((void **)&params);
      }
      pthread_attr_destroy(&pth_attr);
    } else
    {
      qmi_proxy_do_force_sglte_sys_sel_pref_and_oprt_mode(params);
    }
  }
  else
  {
    QMI_PROXY_ERR_MSG("%s: Unable to allocate memory", __FUNCTION__);
  }
} /* qmi_proxy_force_sglte_sys_sel_pref_and_oprt_mode */

/*=========================================================================
  FUNCTION:  qmi_proxy_nas_srvc_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Handle QMI NAS Service unsolicited callback indications.

    @return
    none
*/
/*=========================================================================*/
void qmi_proxy_nas_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
)
{
  qmi_service_id_type proxy_srvc_id;
  qmi_client_id_type proxy_client_id;
  qmi_connection_id_type proxy_conn_id;
  qmi_proxy_conn_type proxy_conn_type;
  void *decoded_payload = NULL;
  uint32_t decoded_payload_len = 0;
  unsigned long   msg_id;
  nas_serving_system_ind_msg_v01 *nas_srv_sys_ind = NULL;
  nas_system_selection_preference_ind_msg_v01 *nas_sys_sel_pref_ind = NULL;
  nas_sys_info_ind_msg_v01 *nas_sys_info_ind = NULL;
  qmi_proxy_svlte_cache_type *svlte_cache_ptr = NULL;
  qmi_proxy_svlte_cache_type *peer_svlte_cache_ptr = NULL;
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = NULL;
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = NULL;
  qmi_proxy_mode_pref_type sglte_mode_pref;
  uint32 i;
  qmi_proxy_conn_type cs_active_modem, ps_active_modem;
  int was_sglte_plmn;
  int eons_active_modem;
  int is_sglte_plmn;
  int is_complete_oos;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  msg_id = rx_msg->msg_id;
  proxy_srvc_id = rx_msg->proxy_srvc_id;
  proxy_client_id = rx_msg->proxy_client_id;
  proxy_conn_type = rx_msg->proxy_conn_type;
  proxy_conn_id = rx_msg->proxy_conn_id;
  decoded_payload = rx_msg->decoded_payload;
  decoded_payload_len = rx_msg->decoded_payload_len;

  qmi_proxy_get_cs_active_ps_active(&cs_active_modem, &ps_active_modem);
  /* Setup cache pointers */
  svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ proxy_conn_type ];
  peer_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_PEER_CONN_TYPE( proxy_conn_type ) ];
  local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];

  /* Decode the QMI unsolicted indication */
  if ( QMI_PROXY_SVLTE_TARGET() )
  {
    /* Process the unsolicted indication */
    switch ( msg_id )
    {
      case QMI_NAS_SERVING_SYSTEM_IND_MSG_V01:
        nas_srv_sys_ind = ( nas_serving_system_ind_msg_v01 * ) decoded_payload;

        QMI_PROXY_DEBUG_MSG( "%s(%d), reg_state %d, roam_ind_valid %d, roam_ind %d, data_cap_valid %d\n",
                             qmi_proxy_modem_name[ proxy_conn_type ],
                             proxy_conn_type,
                             nas_srv_sys_ind->serving_system.registration_state,
                             nas_srv_sys_ind->roaming_indicator_valid,
                             nas_srv_sys_ind->roaming_indicator,
                             nas_srv_sys_ind->data_capabilities_valid );

        if ( nas_srv_sys_ind->data_capabilities_valid )
        {
          for ( i = 0; i < nas_srv_sys_ind->data_capabilities_len && i < NAS_DATA_CAPABILITIES_LIST_MAX_V01; i++ )
          {
            QMI_PROXY_DEBUG_MSG( "%s(%d), data_cap[%d] 0x%x\n",
                                 qmi_proxy_modem_name[ proxy_conn_type ],
                                 proxy_conn_type,
                                 i, nas_srv_sys_ind->data_capabilities[ i ] );
          }
        }

        break;

      case QMI_NAS_SYSTEM_SELECTION_PREFERENCE_IND_MSG_V01:
        nas_sys_sel_pref_ind = ( nas_system_selection_preference_ind_msg_v01 * ) decoded_payload;

        QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

        if ( ( proxy_conn_type == QMI_PROXY_LOCAL_CONN_TYPE ) && nas_sys_sel_pref_ind->emergency_mode_valid )
        {
          /* May need to put MDM back to Online since it is now exit from emergency mode callback */
          if ( ( nas_sys_sel_pref_ind->emergency_mode == 0 ) && qmi_proxy_internal_info.number_of_emergency_calls)
          {
            QMI_PROXY_DEBUG_MSG( "%s\n", "Exit emergency callback mode" );
            qmi_proxy_internal_info.number_of_emergency_calls = 0;

            if ( QMI_PROXY_REMOTE_MODEM_IS_ACTIVE() )
            {
              /* Exit emergency callback. Put remote modem back to ONLINE */
              qmi_proxy_force_online( QMI_PROXY_REMOTE_CONN_TYPE );
            }
          }
        }

        if ( nas_sys_sel_pref_ind->mode_pref_valid )
        {
          switch( qmi_proxy_internal_info.svlte_mode_pref )
          {
            case QMI_PROXY_MODE_PREF_CDMA_ONLY:
            case QMI_PROXY_MODE_PREF_GSM_ONLY:
            case QMI_PROXY_MODE_PREF_WCDMA_ONLY:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_AUTO:
            case QMI_PROXY_MODE_PREF_GSM_WCDMA_PREFERRED:
              /* Emergency mode and mode preference should not be reported from Remote Modem till the mode change is completed */
              if ( proxy_conn_type == QMI_PROXY_REMOTE_CONN_TYPE )
              {
                nas_sys_sel_pref_ind->emergency_mode_valid = FALSE;
                nas_sys_sel_pref_ind->mode_pref_valid = FALSE;
              }
              break;

            case QMI_PROXY_MODE_PREF_EVDO_ONLY:
            case QMI_PROXY_MODE_PREF_LTE_ONLY:
              /* Emergency mode and mode preference should not be reported from Local Modem till the mode change is completed */
              if ( proxy_conn_type == QMI_PROXY_LOCAL_CONN_TYPE )
              {
                nas_sys_sel_pref_ind->emergency_mode_valid = FALSE;
                nas_sys_sel_pref_ind->mode_pref_valid = FALSE;
              }
              break;
              break;

            default:
              /* Emergency mode should be from Local modem */
              if ( proxy_conn_type == QMI_PROXY_REMOTE_CONN_TYPE )
              {
                nas_sys_sel_pref_ind->emergency_mode_valid = FALSE;
              }

              /* Mode preference is in the middle of the change. Don't report intermediate changes from either Local and Remote modems */
              nas_sys_sel_pref_ind->mode_pref_valid = FALSE;
              break;
          }
        }

        QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
        break;

      case QMI_NAS_SYS_INFO_IND_MSG_V01:
        /* reset some of the information selectively depending on the type of the connection type local/remote,
            as they are not valid for the corresponding conn_type */
        nas_sys_info_ind = (nas_sys_info_ind_msg_v01 *)decoded_payload;

        if ( proxy_conn_type == QMI_PROXY_LOCAL_CONN_TYPE )
        {
          nas_sys_info_ind->hdr_srv_status_info_valid = FALSE;
          nas_sys_info_ind->lte_srv_status_info_valid = FALSE;
          nas_sys_info_ind->hdr_sys_info_valid = FALSE;
          nas_sys_info_ind->lte_sys_info_valid = FALSE;
          nas_sys_info_ind->hdr_sys_info2_valid = FALSE;
          nas_sys_info_ind->lte_sys_info2_valid = FALSE;
        }
        else if ( proxy_conn_type == QMI_PROXY_REMOTE_CONN_TYPE )
        {
          nas_sys_info_ind->cdma_srv_status_info_valid = FALSE;
          nas_sys_info_ind->gsm_srv_status_info_valid = FALSE;
          nas_sys_info_ind->wcdma_srv_status_info_valid = FALSE;
          nas_sys_info_ind->cdma_sys_info_valid = FALSE;
          nas_sys_info_ind->gsm_sys_info_valid = FALSE;
          nas_sys_info_ind->wcdma_sys_info_valid = FALSE;
          nas_sys_info_ind->cdma_sys_info2_valid = FALSE;
          nas_sys_info_ind->gsm_sys_info2_valid = FALSE;
          nas_sys_info_ind->wcdma_sys_info2_valid = FALSE;
          nas_sys_info_ind->gsm_sys_info3_valid = FALSE;
          nas_sys_info_ind->wcdma_sys_info3_valid = FALSE;
        }

        QMI_PROXY_DEBUG_MSG( "cdma_sys_info_valid = %d, gsm_sys_info_valid = %d, wcdma_sys_info_valid = %d\n",
                             nas_sys_info_ind->cdma_sys_info_valid, nas_sys_info_ind->gsm_sys_info_valid, nas_sys_info_ind->wcdma_sys_info_valid );

        QMI_PROXY_DEBUG_MSG( "hdr_sys_info_valid = %d, lte_sys_info_valid = %d\n",
                             nas_sys_info_ind->hdr_sys_info_valid, nas_sys_info_ind->lte_sys_info_valid );

        QMI_PROXY_DEBUG_MSG( "cdma_sys_info2_valid = %d, gsm_sys_info2_valid = %d, wcdma_sys_info2_valid = %d\n",
                             nas_sys_info_ind->cdma_sys_info2_valid, nas_sys_info_ind->gsm_sys_info2_valid, nas_sys_info_ind->wcdma_sys_info2_valid );

        QMI_PROXY_DEBUG_MSG( "hdr_sys_info2_valid = %d, lte_sys_info2_valid = %d\n",
                             nas_sys_info_ind->hdr_sys_info2_valid, nas_sys_info_ind->lte_sys_info2_valid );

        QMI_PROXY_DEBUG_MSG( "gsm_sys_info3_valid = %d, wcdma_sys_info3_valid = %d\n",
                             nas_sys_info_ind->gsm_sys_info3_valid, nas_sys_info_ind->wcdma_sys_info3_valid);
        break;

      default:
        break;
    }
  }
  else if ( QMI_PROXY_SGLTE_TARGET() )
  {
    qmi_proxy_sglte_state_machine_type *sm = qmi_proxy_internal_info.sglte_sm;
    /* Process the unsolicted indication */
    switch ( msg_id )
    {
      case QMI_NAS_SERVING_SYSTEM_IND_MSG_V01:
        nas_srv_sys_ind = ( nas_serving_system_ind_msg_v01 * ) decoded_payload;

        QMI_PROXY_DEBUG_MSG( "%s(%d), reg_state %d, roam_ind_valid %d, roam_ind %d, data_cap_valid %d\n",
                             qmi_proxy_modem_name[ proxy_conn_type ],
                             proxy_conn_type,
                             nas_srv_sys_ind->serving_system.registration_state,
                             nas_srv_sys_ind->roaming_indicator_valid,
                             nas_srv_sys_ind->roaming_indicator,
                             nas_srv_sys_ind->data_capabilities_valid );

        if ( nas_srv_sys_ind->data_capabilities_valid )
        {
          for ( i = 0; i < nas_srv_sys_ind->data_capabilities_len && i < NAS_DATA_CAPABILITIES_LIST_MAX_V01; i++ )
          {
            QMI_PROXY_DEBUG_MSG( "%s(%d), data_cap[%d] 0x%x\n",
                                 qmi_proxy_modem_name[ proxy_conn_type ],
                                 proxy_conn_type,
                                 i, nas_srv_sys_ind->data_capabilities[ i ] );
          }
        }

        break;

      case QMI_NAS_SYSTEM_SELECTION_PREFERENCE_IND_MSG_V01:
        nas_sys_sel_pref_ind = ( nas_system_selection_preference_ind_msg_v01 * ) decoded_payload;

        QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
        sglte_mode_pref = qmi_proxy_internal_info.sglte_user_mode_pref;
        cs_active_modem = qmi_proxy_internal_info.cs_active_modem;
        ps_active_modem = qmi_proxy_internal_info.ps_active_modem;
        is_sglte_plmn = qmi_proxy_internal_info.is_in_sglte_coverage;
        is_complete_oos = TRUE;

        if ( nas_sys_sel_pref_ind->mode_pref_valid )
        {
          /* Mode preference is the mode_pref that we have cached. */
          nas_sys_sel_pref_ind->mode_pref =
            qmi_proxy_user_mode_pref_to_ril_nas_mask(sglte_mode_pref);

          if ( !QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() )
          {
            if ( proxy_conn_type == QMI_PROXY_REMOTE_CONN_TYPE )
            {
              nas_sys_sel_pref_ind->emergency_mode_valid = FALSE;
              nas_sys_sel_pref_ind->mode_pref_valid = FALSE;
            }
          }
        }

        if ( nas_sys_sel_pref_ind->mode_pref_valid || nas_sys_sel_pref_ind->srv_reg_restriction_valid  || nas_sys_sel_pref_ind->srv_domain_pref_valid ) 
        {
          qmi_proxy_dispatch_txn_on_hold ( proxy_conn_type , proxy_srvc_id, DMS_OP_MODE_ONLINE_V01 ); // DMS_OP_MODE_ONLINE_V01 as only need sys_sel_pref for online
        }

        if (proxy_conn_type == QMI_PROXY_LOCAL_CONN_TYPE &&
            nas_sys_sel_pref_ind->srv_reg_restriction_valid)
        {
          if (nas_sys_sel_pref_ind->srv_reg_restriction == NAS_SRV_REG_RESTRICTION_CAMPED_ONLY_V01)
          {
            QMI_PROXY_DEBUG_MSG( "%s: Local camped only\n", __FUNCTION__);
            qmi_proxy_sglte_sm_queue_event(
                    qmi_proxy_internal_info.sglte_sm,
                    SGLTE_SM_EVENT_CAMP_ONLY_ACTIVE_ON_LOCAL,
                    "SGLTE_SM_EVENT_CAMP_ONLY_ACTIVE_ON_LOCAL",
                    NULL);
          }
          else if (nas_sys_sel_pref_ind->srv_reg_restriction == NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01)
          {
            QMI_PROXY_DEBUG_MSG( "%s: Local unrestricted\n", __FUNCTION__);
            qmi_proxy_sglte_sm_queue_event(
                    qmi_proxy_internal_info.sglte_sm,
                    SGLTE_SM_EVENT_UNRESTRICTED_ACTIVE_ON_LOCAL,
                    "SGLTE_SM_EVENT_UNRESTRICTED_ACTIVE_ON_LOCAL",
                    NULL);
          }
        }

        QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
        break;

      case QMI_NAS_SYS_INFO_IND_MSG_V01:
        /* reset some of the information selectively depending on the type of the connection type local/remote,
            as they are not valid for the corresponding conn_type */
        nas_sys_info_ind = (nas_sys_info_ind_msg_v01 *)decoded_payload;

        QMI_PROXY_DEBUG_MSG("cs_active_modem: %d. ps_active_modem: %d\n", cs_active_modem, ps_active_modem);

        was_sglte_plmn = qmi_proxy_internal_info.is_in_sglte_coverage;
        qmi_proxy_sglte_update_last_known_plmn(nas_sys_info_ind, proxy_conn_type);
        is_sglte_plmn = qmi_proxy_internal_info.is_in_sglte_coverage;

        if ( proxy_conn_type == QMI_PROXY_LOCAL_CONN_TYPE && (cs_active_modem == QMI_PROXY_REMOTE_CONN_TYPE || ps_active_modem == QMI_PROXY_REMOTE_CONN_TYPE))
        {
          nas_sys_info_ind->gsm_srv_status_info_valid = FALSE;
          nas_sys_info_ind->gsm_sys_info_valid = FALSE;
          nas_sys_info_ind->gsm_sys_info2_valid = FALSE;
          nas_sys_info_ind->gsm_sys_info3_valid = FALSE;
          if ( cs_active_modem != QMI_PROXY_LOCAL_CONN_TYPE && ps_active_modem != QMI_PROXY_LOCAL_CONN_TYPE )
          {
            nas_sys_info_ind->cdma_srv_status_info_valid = FALSE;
            nas_sys_info_ind->wcdma_srv_status_info_valid = FALSE;
            nas_sys_info_ind->tdscdma_srv_status_info_valid = FALSE;
            nas_sys_info_ind->hdr_srv_status_info_valid = FALSE;
            nas_sys_info_ind->lte_srv_status_info_valid = FALSE;
            nas_sys_info_ind->cdma_sys_info_valid = FALSE;
            nas_sys_info_ind->wcdma_sys_info_valid = FALSE;
            nas_sys_info_ind->tdscdma_sys_info_valid = FALSE;
            nas_sys_info_ind->hdr_sys_info_valid = FALSE;
            nas_sys_info_ind->lte_sys_info_valid = FALSE;
            nas_sys_info_ind->cdma_sys_info2_valid = FALSE;
            nas_sys_info_ind->wcdma_sys_info2_valid = FALSE;
            nas_sys_info_ind->wcdma_sys_info3_valid = FALSE;
            nas_sys_info_ind->hdr_sys_info2_valid = FALSE;
            nas_sys_info_ind->lte_sys_info2_valid = FALSE;
          }
        }
        else if ( proxy_conn_type == QMI_PROXY_REMOTE_CONN_TYPE && (cs_active_modem == QMI_PROXY_LOCAL_CONN_TYPE || ps_active_modem == QMI_PROXY_LOCAL_CONN_TYPE))
        {
          nas_sys_info_ind->cdma_srv_status_info_valid = FALSE;
          nas_sys_info_ind->hdr_srv_status_info_valid = FALSE;
          nas_sys_info_ind->lte_srv_status_info_valid = FALSE;
          nas_sys_info_ind->wcdma_srv_status_info_valid = FALSE;
          nas_sys_info_ind->cdma_sys_info_valid = FALSE;
          nas_sys_info_ind->wcdma_sys_info_valid = FALSE;
          nas_sys_info_ind->hdr_sys_info_valid = FALSE;
          nas_sys_info_ind->lte_sys_info_valid = FALSE;
          nas_sys_info_ind->cdma_sys_info2_valid = FALSE;
          nas_sys_info_ind->wcdma_sys_info2_valid = FALSE;
          nas_sys_info_ind->wcdma_sys_info3_valid = FALSE;
          nas_sys_info_ind->hdr_sys_info2_valid = FALSE;
          nas_sys_info_ind->lte_sys_info2_valid = FALSE;
          if ( cs_active_modem != QMI_PROXY_REMOTE_CONN_TYPE && ps_active_modem != QMI_PROXY_REMOTE_CONN_TYPE)
          {
              nas_sys_info_ind->gsm_srv_status_info_valid = FALSE;
              nas_sys_info_ind->gsm_sys_info_valid = FALSE;
              nas_sys_info_ind->gsm_sys_info2_valid = FALSE;
              nas_sys_info_ind->gsm_sys_info3_valid = FALSE;
          }
        }

        qmi_proxy_sglte_update_registration_status(cs_active_modem, ps_active_modem, proxy_conn_type, nas_sys_info_ind);

        QMI_PROXY_DEBUG_MSG( "cdma_sys_info_valid = %d, gsm_sys_info_valid = %d, wcdma_sys_info_valid = %d\n",
                             nas_sys_info_ind->cdma_sys_info_valid, nas_sys_info_ind->gsm_sys_info_valid, nas_sys_info_ind->wcdma_sys_info_valid );

        QMI_PROXY_DEBUG_MSG( "tdscdma_sys_info_valid = %d\n", nas_sys_info_ind->tdscdma_sys_info_valid );

        QMI_PROXY_DEBUG_MSG( "hdr_sys_info_valid = %d, lte_sys_info_valid = %d\n",
                             nas_sys_info_ind->hdr_sys_info_valid, nas_sys_info_ind->lte_sys_info_valid );

        QMI_PROXY_DEBUG_MSG( "cdma_sys_info2_valid = %d, gsm_sys_info2_valid = %d, wcdma_sys_info2_valid = %d\n",
                             nas_sys_info_ind->cdma_sys_info2_valid, nas_sys_info_ind->gsm_sys_info2_valid, nas_sys_info_ind->wcdma_sys_info2_valid );

        QMI_PROXY_DEBUG_MSG( "hdr_sys_info2_valid = %d, lte_sys_info2_valid = %d\n",
                             nas_sys_info_ind->hdr_sys_info2_valid, nas_sys_info_ind->lte_sys_info2_valid );

        QMI_PROXY_DEBUG_MSG( "gsm_sys_info3_valid = %d, wcdma_sys_info3_valid = %d\n",
                             nas_sys_info_ind->gsm_sys_info3_valid, nas_sys_info_ind->wcdma_sys_info3_valid);

        QMI_PROXY_DEBUG_MSG( "%s: Previous Coverage: %s. New Coverage: %s\n",
                             __FUNCTION__,
                             was_sglte_plmn == 0 ? "Non-SGLTE" : was_sglte_plmn == -1 ? "Unknown" : "SGLTE",
                             is_sglte_plmn == 0 ? "Non-SGLTE" : is_sglte_plmn == -1 ? "Unknown" : "SGLTE" )

        qmi_proxy_sglte_update_srv_domain_timers(proxy_conn_type, nas_sys_info_ind);

        break;

      default:
        break;
    }
  }

  /* Forward the QMI Service Indication to QMI Proxy Client */
  if ( ( proxy_client_id != QMI_PROXY_INTERNAL_CLIENT_ID ) && ( rc == QMI_NO_ERR ) )
  {
    qmi_proxy_srvc_send_qmi_indication( proxy_srvc_id,
                                        proxy_client_id,
                                        proxy_conn_id,
                                        proxy_conn_type,
                                        msg_id,
                                        decoded_payload,
                                        decoded_payload_len );
  }

  /* Release buffer */
  qmi_proxy_free( (void **)&decoded_payload );

} /* qmi_proxy_nas_srvc_unsol_ind_cb */


/*=========================================================================
  FUNCTION:  qmi_proxy_voice_srvc_arb_hdlr

===========================================================================*/
/*!
    @brief
    Perform the arbitration for QMI VOICE service

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_voice_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
)
{
  qmi_proxy_txn_entry_list_type local_txn_entry_list, remote_txn_entry_list;
  qmi_proxy_async_state_type async_state = QMI_PROXY_ASYNC_IDLE;
  boolean user_data_valid = FALSE;
  uint32 user_data = 0;
  int rc = QMI_NO_ERR;

  /* Supress compiler warnings for unused variables */
  (void) client;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "%s ....... Msg ID 0x%x\n", __FUNCTION__, srvc_msg_id );

  /* Initialize transaction entry lists */
  qmi_proxy_init_txn_entry_list( &local_txn_entry_list, &remote_txn_entry_list );

   QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.voip_info_mutex);

  /* Perform service arbitration */
  if ( QMI_PROXY_CSFB_TARGET() || QMI_PROXY_VOIP_ACTIVE() || (QMI_PROXY_SGLTE_TARGET() && QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE()) )
  {
    QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.voip_info_mutex );

    QMI_PROXY_DEBUG_MSG("routing the voice messages to remote modem, voip status = %u\n", QMI_PROXY_VOIP_ACTIVE());
    /* Request applies to remote modem processing  */
    qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_VOICE_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
  }
  else
  {
    QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.voip_info_mutex );

    /* Request applies to local modem processing */
    qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_VOICE_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
  }

  /* Setup and dispatch transaction */
  rc = qmi_proxy_setup_and_dispatch_txn( proxy_srvc_id,
                                         proxy_client_id,
                                         proxy_conn_id,
                                         srvc_msg_id,
                                         &local_txn_entry_list,
                                         &remote_txn_entry_list,
                                         rx_txn_id,
                                         async_state,
                                         user_data_valid,
                                         user_data );

  return rc;

} /* qmi_proxy_voice_srvc_arb_hdlr */


/*=========================================================================
  FUNCTION:  qmi_proxy_voice_srvc_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Handle QMI VOICE Service unsolicited callback indications.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_voice_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
)
{
  qmi_service_id_type proxy_srvc_id;
  qmi_client_id_type proxy_client_id;
  qmi_connection_id_type proxy_conn_id;
  qmi_proxy_conn_type proxy_conn_type;
  void *decoded_payload = NULL;
  uint32_t decoded_payload_len = 0;
  unsigned long   msg_id;
  voice_all_call_status_ind_msg_v02 *voice_all_call_status_ind = NULL;
  int rc = QMI_NO_ERR;
  uint32 i;
  int number_of_calls = 0;
  nas_set_system_selection_preference_req_msg_v01 *local_nas_set_sys_sel_pref_req = NULL;
  nas_set_system_selection_preference_req_msg_v01 *remote_nas_set_sys_sel_pref_req = NULL;
  nas_set_system_selection_preference_resp_msg_v01 *local_nas_set_sys_sel_pref_resp = NULL;
  boolean exit_emergency_mode = FALSE;
  boolean enter_emergency_mode = FALSE;

  /*-----------------------------------------------------------------------*/

  msg_id = rx_msg->msg_id;
  proxy_srvc_id = rx_msg->proxy_srvc_id;
  proxy_client_id = rx_msg->proxy_client_id;
  proxy_conn_type = rx_msg->proxy_conn_type;
  proxy_conn_id = rx_msg->proxy_conn_id;
  decoded_payload = rx_msg->decoded_payload;
  decoded_payload_len = rx_msg->decoded_payload_len;

  if (( QMI_PROXY_SVLTE_TARGET() ) || QMI_PROXY_SGLTE_TARGET())
  {
    if ( msg_id == QMI_VOICE_ALL_CALL_STATUS_IND_V02 )
    {
      voice_all_call_status_ind = ( voice_all_call_status_ind_msg_v02 * ) decoded_payload;

      QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );

      QMI_PROXY_DEBUG_MSG( "Voice all call status ind, len %d\n", voice_all_call_status_ind->call_info_len );
      for ( i = 0; i < voice_all_call_status_ind->call_info_len && i < QMI_VOICE_CALL_INFO_MAX_V02; i++ )
      {
        QMI_PROXY_DEBUG_MSG( "Voice call, mode %d, dir %d, type %d call id %d, state %d\n",
                             voice_all_call_status_ind->call_info[ i ].mode,
                             voice_all_call_status_ind->call_info[ i ].direction,
                             voice_all_call_status_ind->call_info[ i ].call_type,
                             voice_all_call_status_ind->call_info[ i ].call_id,
                             voice_all_call_status_ind->call_info[ i ].call_state );

        if ( voice_all_call_status_ind->call_info[ i ].call_state != CALL_STATE_END_V02)
        {
          number_of_calls ++;
        }

        if ( voice_all_call_status_ind->call_info[ i ].call_type == CALL_TYPE_EMERGENCY_V02 )
        {
          if ( voice_all_call_status_ind->call_info[ i ].call_state == CALL_STATE_ORIGINATING_V02 )
          {
            if ( !qmi_proxy_internal_info.number_of_emergency_calls )
            {
              QMI_PROXY_DEBUG_MSG( "Emergency call originating, mode %d, call id %d\n",
                                   voice_all_call_status_ind->call_info[ i ].mode,
                                   voice_all_call_status_ind->call_info[ i ].call_id );

              if ( QMI_PROXY_SVLTE_TARGET() && ( QMI_PROXY_REMOTE_MODEM_IS_ACTIVE() ))
              {
                /* Put remote modem to LPM */
                rc = qmi_proxy_force_lpm( QMI_PROXY_REMOTE_CONN_TYPE );
              }
              else if (QMI_PROXY_SGLTE_TARGET())
              {
                if (QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() &&
                     QMI_PROXY_SGLTE_LOCAL_MODEM_PS_ACTIVE())
                {
                  /* Put local modem to LPM */
                  enter_emergency_mode = TRUE;
                }
              }
            }
            else
            {
              QMI_PROXY_DEBUG_MSG( "Emergency call re-originating, mode %d, call id %d\n",
                                   voice_all_call_status_ind->call_info[ i ].mode,
                                   voice_all_call_status_ind->call_info[ i ].call_id );
            }

            qmi_proxy_internal_info.number_of_emergency_calls++;
            qmi_proxy_internal_info.emergency_call_mode = voice_all_call_status_ind->call_info[ i ].mode;
          }
          else if ( voice_all_call_status_ind->call_info[ i ].call_state == CALL_STATE_END_V02 )
          {
            QMI_PROXY_DEBUG_MSG( "Emergency call end, mode %d, call id %d\n",
                                 voice_all_call_status_ind->call_info[ i ].mode,
                                 voice_all_call_status_ind->call_info[ i ].call_id );

            if ( ( qmi_proxy_internal_info.number_of_emergency_calls ) &&
                 ( qmi_proxy_internal_info.emergency_call_mode != CALL_MODE_CDMA_V02 ) )
            {
              qmi_proxy_internal_info.number_of_emergency_calls--;
              if ( !qmi_proxy_internal_info.number_of_emergency_calls )
              {
                if ( QMI_PROXY_SVLTE_TARGET() && ( QMI_PROXY_REMOTE_MODEM_IS_ACTIVE() ))
                {
                  /* Exit emergency callback. Put remote modem back to ONLINE */
                  QMI_PROXY_DEBUG_MSG( "Exit emergency mode, mode %d, call id %d\n",
                                       voice_all_call_status_ind->call_info[ i ].mode,
                                       voice_all_call_status_ind->call_info[ i ].call_id );
                  qmi_proxy_force_online( QMI_PROXY_REMOTE_CONN_TYPE );
                }
                else if (QMI_PROXY_SGLTE_TARGET())
                {
                  if (QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() &&
                         QMI_PROXY_SGLTE_LOCAL_MODEM_PS_ACTIVE())
                  {
                    /* Exit emergency callback. Put local modem back to ONLINE */
                    QMI_PROXY_DEBUG_MSG( "Exit emergency mode, mode %d, call id %d\n",
                                       voice_all_call_status_ind->call_info[ i ].mode,
                                       voice_all_call_status_ind->call_info[ i ].call_id );

                    exit_emergency_mode = TRUE;
                  }
                }
              }
            }
          }

          break;
        }
      }

      qmi_proxy_internal_info.number_of_calls = number_of_calls;

      QMI_PROXY_DEBUG_MSG("Connection: %s (%d). Calls: %d. is_dtm_supported: %d",
              qmi_proxy_modem_name[proxy_conn_type],
              proxy_conn_type,
              number_of_calls,
              qmi_proxy_internal_info.is_dtm_supported[proxy_conn_type]);

      if (qmi_proxy_internal_info.number_of_calls)
      {
        if (qmi_proxy_internal_info.is_dtm_supported[proxy_conn_type] <= 0)
        {
          qmi_proxy_sglte_sm_queue_event(qmi_proxy_internal_info.sglte_sm,
                  SGLTE_SM_EVENT_VOICE_CALL_ACTIVE_NO_DTM,
                  "SGLTE_SM_EVENT_VOICE_CALL_ACTIVE_NO_DTM", NULL);
        }
      }
      else
      {
        qmi_proxy_sglte_sm_queue_event(qmi_proxy_internal_info.sglte_sm,
                SGLTE_SM_EVENT_NO_VOICE_CALL,
                "SGLTE_SM_EVENT_NO_VOICE_CALL", NULL);

        if (qmi_proxy_internal_info.pending_event != SGLTE_SM_EVENT_INVALID)
        {
          qmi_proxy_sglte_sm_queue_event(qmi_proxy_internal_info.sglte_sm,
                                        qmi_proxy_internal_info.pending_event,
                                        qmi_proxy_internal_info.pending_event_name, NULL);
          qmi_proxy_internal_info.pending_event = SGLTE_SM_EVENT_INVALID;
        }
      }
      QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
    }
  }

  if( enter_emergency_mode )
  {
    rc = qmi_proxy_force_lpm_sync( QMI_PROXY_LOCAL_CONN_TYPE, QMI_PROXY_SYNC_OPRT_MODE_TIMEOUT);

    if (rc != QMI_NO_ERR)
    {
      QMI_PROXY_ERR_MSG("%s\n", "Error putting local modem in LPM");
    }
  }

  if ( exit_emergency_mode )
  {
    /* Exit emergency callback. Put local modem back to ONLINE */
    QMI_PROXY_DEBUG_MSG( "desired_mode = %d", qmi_proxy_internal_info.desired_mode );
    if ( DMS_OP_MODE_ONLINE_V01 == qmi_proxy_internal_info.desired_mode )
    {
      rc = qmi_proxy_force_online_sync( QMI_PROXY_LOCAL_CONN_TYPE,
                                        QMI_PROXY_SYNC_OPRT_MODE_TIMEOUT );

      local_nas_set_sys_sel_pref_req = qmi_proxy_malloc(sizeof(*local_nas_set_sys_sel_pref_req));
      local_nas_set_sys_sel_pref_resp = qmi_proxy_malloc(sizeof(*local_nas_set_sys_sel_pref_resp));
      remote_nas_set_sys_sel_pref_req = qmi_proxy_malloc(sizeof(*remote_nas_set_sys_sel_pref_req));

      if (rc == QMI_NO_ERR &&
              local_nas_set_sys_sel_pref_req &&
              local_nas_set_sys_sel_pref_resp &&
              remote_nas_set_sys_sel_pref_req)
      {
        /* Prepare request */
        rc = qmi_proxy_set_sys_sel_pref_per_sglte_mode_pref(
                qmi_proxy_internal_info.sglte_user_mode_pref,
                qmi_proxy_internal_info.is_in_sglte_coverage,
                FALSE,
                local_nas_set_sys_sel_pref_req,
                remote_nas_set_sys_sel_pref_req);

        if (rc == QMI_NO_ERR)
        {
          rc = qmi_proxy_force_sys_sel_pref_sync( QMI_PROXY_LOCAL_CONN_TYPE,
                  local_nas_set_sys_sel_pref_req,
                  local_nas_set_sys_sel_pref_resp,
                  0x7fffffff );
          if (rc != QMI_NO_ERR)
          {
            QMI_PROXY_ERR_MSG("%s", "Unable to send sys_sel_pref request to local modem");
          }
        }
      }
      qmi_proxy_free((void **)&local_nas_set_sys_sel_pref_req);
      qmi_proxy_free((void **)&local_nas_set_sys_sel_pref_resp);
      qmi_proxy_free((void **)&remote_nas_set_sys_sel_pref_req);
    }
  }

  /* Forward the QMI Service Indication to QMI Proxy Client */
  qmi_proxy_srvc_send_qmi_indication( proxy_srvc_id,
                                      proxy_client_id,
                                      proxy_conn_id,
                                      proxy_conn_type,
                                      msg_id,
                                      decoded_payload,
                                      decoded_payload_len );

  /* Release buffer */
  qmi_proxy_free( (void **)&decoded_payload );

} /* qmi_proxy_voice_srvc_unsol_ind_cb */


/*=========================================================================
  FUNCTION:  qmi_proxy_pbm_srvc_arb_hdlr

===========================================================================*/
/*!
    @brief
    Perform the arbitration for QMI PBM service

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_pbm_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
)
{
  qmi_proxy_txn_entry_list_type local_txn_entry_list, remote_txn_entry_list;
  qmi_proxy_async_state_type async_state = QMI_PROXY_ASYNC_IDLE;
  boolean user_data_valid = FALSE;
  uint32 user_data = 0;
  qmi_proxy_conn_type cs_active_modem, ps_active_modem;
  int rc = QMI_NO_ERR;

  /* Supress compiler warnings for unused variables */
  (void) client;

  /*-----------------------------------------------------------------------*/

  QMI_PROXY_DEBUG_MSG( "%s ....... Msg ID 0x%x\n", __FUNCTION__, srvc_msg_id );

  /* Initialize transaction entry lists */
  qmi_proxy_init_txn_entry_list( &local_txn_entry_list, &remote_txn_entry_list );

  /* Perform service arbitration */
  if( QMI_PROXY_SGLTE_TARGET() )
  {

    QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
    cs_active_modem = qmi_proxy_internal_info.cs_active_modem;
    ps_active_modem = qmi_proxy_internal_info.ps_active_modem;
    QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

    /* Perform service arbitration */
    switch( srvc_msg_id )
    {
      /* Get list of all emergency numbers */
      case QMI_PBM_GET_EMERGENCY_LIST_REQ_V01:
        if ( cs_active_modem == QMI_PROXY_REMOTE_CONN_TYPE || ps_active_modem == QMI_PROXY_REMOTE_CONN_TYPE )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_PBM_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        break;

      default:
        break;
    }

    if ( cs_active_modem == QMI_PROXY_LOCAL_CONN_TYPE || ps_active_modem == QMI_PROXY_LOCAL_CONN_TYPE )
    {
      /* Request applies to local modem processing */
      qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_PBM_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
    }
  }
  else if ( QMI_PROXY_FUSION_TARGET() )
  {
    /* Request applies to remote modem processing */
    qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_PBM_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
  }
  else
  {
    /* Request applies to local modem processing */
    qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_PBM_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
  }

  /* Setup and dispatch transaction */
  rc = qmi_proxy_setup_and_dispatch_txn( proxy_srvc_id,
                                         proxy_client_id,
                                         proxy_conn_id,
                                         srvc_msg_id,
                                         &local_txn_entry_list,
                                         &remote_txn_entry_list,
                                         rx_txn_id,
                                         async_state,
                                         user_data_valid,
                                         user_data );

  return rc;

} /* qmi_proxy_pbm_srvc_arb_hdlr */


/*=========================================================================
  FUNCTION:  qmi_proxy_pbm_srvc_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Handle QMI PBM Service unsolicited callback indications.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_pbm_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
)
{
  qmi_service_id_type proxy_srvc_id;
  qmi_client_id_type proxy_client_id;
  qmi_connection_id_type proxy_conn_id;
  qmi_proxy_conn_type proxy_conn_type;
  unsigned long   msg_id;
  void *decoded_payload = NULL;
  uint32_t decoded_payload_len = 0;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  msg_id = rx_msg->msg_id;
  proxy_srvc_id = rx_msg->proxy_srvc_id;
  proxy_client_id = rx_msg->proxy_client_id;
  proxy_conn_type = rx_msg->proxy_conn_type;
  proxy_conn_id = rx_msg->proxy_conn_id;
  decoded_payload = rx_msg->decoded_payload;
  decoded_payload_len = rx_msg->decoded_payload_len;

  if( (FALSE == QMI_PROXY_SGLTE_TARGET()) || (QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() && QMI_PROXY_REMOTE_CONN_TYPE == proxy_conn_type) ||
      (!QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() && QMI_PROXY_LOCAL_CONN_TYPE == proxy_conn_type) )
  {
    /* Forward the QMI Service Indication to QMI Proxy Client */
    qmi_proxy_srvc_send_qmi_indication( proxy_srvc_id,
                                        proxy_client_id,
                                        proxy_conn_id,
                                        proxy_conn_type,
                                        msg_id,
                                        decoded_payload,
                                        decoded_payload_len );
  }

  /* Release buffer */
  qmi_proxy_free( (void **)&decoded_payload );

} /* qmi_proxy_pbm_srvc_unsol_ind_cb */


/*=========================================================================
  FUNCTION:  qmi_proxy_wms_srvc_arb_hdlr

===========================================================================*/
/*!
    @brief
    Perform the arbitration for QMI WMS service

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_wms_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
)
{
  qmi_proxy_txn_entry_list_type local_txn_entry_list, remote_txn_entry_list;
  wms_raw_send_req_msg_v01 *wms_raw_send_req = NULL;
  wms_send_from_mem_store_req_msg_v01 *wms_send_from_mem_store_req = NULL;
  wms_send_ack_req_msg_v01 *wms_send_ack_req = NULL;
  qmi_proxy_async_state_type async_state = QMI_PROXY_ASYNC_IDLE;
  boolean user_data_valid = FALSE;
  uint32 user_data = 0;
  int rc = QMI_NO_ERR;
  wms_raw_read_req_msg_v01 *wms_raw_read_req = NULL;
  qmi_proxy_svlte_cache_type *local_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_LOCAL_CONN_TYPE ];
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
  /*-----------------------------------------------------------------------*/

  /* Supress compiler warnings for unused variables */
  (void) client;

  QMI_PROXY_DEBUG_MSG( "%s ....... Msg ID 0x%x\n", __FUNCTION__, srvc_msg_id );

  /* Initialize transaction entry lists */
  qmi_proxy_init_txn_entry_list( &local_txn_entry_list, &remote_txn_entry_list );

  /* Perform service arbitration */
  switch( srvc_msg_id )
  {
    /* Reset the control point WMS state */
    case QMI_WMS_RESET_REQ_V01:
    /* Set WMS event reporting for the control point */
    case QMI_WMS_SET_EVENT_REPORT_REQ_V01:
    /* Set the action performed upon WMS message receipt for the specified message routes. It also
       sets the action performed upon WMS receipt of status reports */
    case QMI_WMS_SET_ROUTES_REQ_V01:
    /* Set the SMSC address used when storing or saving SMS messages */
    case QMI_WMS_SET_SMSC_ADDRESS_REQ_V01:
    /* Configures the retry period */
    case QMI_WMS_SET_RETRY_PERIOD_REQ_V01:
    /* Configures the retry interval */
    case QMI_WMS_SET_RETRY_INTERVAL_REQ_V01:
    /* Set whether the client has storage available for new SMS message */
    case QMI_WMS_SET_MEMORY_STATUS_REQ_V01:
    /* Set the GW domain preference */
    case QMI_WMS_SET_DOMAIN_PREF_REQ_V01:
    /* Set client as primary client */
    case QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01:
    /* Set SMS parameters */
    case QMI_WMS_SET_SMS_PARAMETERS_REQ_V01:
      if ( QMI_PROXY_SGLTE_TARGET() )
      {
        /* Request applies to both modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else
      {
        if ( QMI_PROXY_FUSION_TARGET() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }

        if ( QMI_PROXY_SVLTE_TARGET() || QMI_PROXY_MULTIMODE_TARGET() )
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      break;

    case QMI_WMS_RAW_SEND_REQ_V01:
      if ( QMI_PROXY_CSFB_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else if ( QMI_PROXY_SVLTE_TARGET() )
      {
        wms_raw_send_req = ( wms_raw_send_req_msg_v01 * ) decoded_payload;

        QMI_PROXY_DEBUG_MSG("ims_valid = %u, sms_on_ims =%u\n", wms_raw_send_req->sms_on_ims_valid, wms_raw_send_req->sms_on_ims);

        /* SMS over IMS */
        if ( wms_raw_send_req->sms_on_ims_valid && ( wms_raw_send_req->sms_on_ims == 0x01 ) )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        /* SMS */
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else if ( QMI_PROXY_SGLTE_TARGET() )
      {
        wms_raw_send_req = ( wms_raw_send_req_msg_v01 * ) decoded_payload;

        QMI_PROXY_DEBUG_MSG("remote modem cs active = %d\n", QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE());

        if( QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    /* Read a message from the device memory storage */
    case QMI_WMS_RAW_READ_REQ_V01:
      if ( QMI_PROXY_SGLTE_TARGET() )
      {
        QMI_PROXY_DEBUG_MSG("remote modem cs active = %d\n", QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE());

        if( QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else
      {
         if ( QMI_PROXY_FUSION_TARGET() )
         {
           wms_raw_read_req = ( wms_raw_read_req_msg_v01 * ) decoded_payload;

           if ( ( ( wms_raw_read_req->sms_on_ims_valid ) && ( wms_raw_read_req->sms_on_ims == TRUE ) ) ||
                 ( wms_raw_read_req->message_memory_storage_identification.storage_type == WMS_STORAGE_TYPE_UIM_V01 ) )
           {
             /* Remote modem is the master of SIM operations. Request applies to remote modem processing */
             qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
           }
           else
           {
             /* Request applies to local modem processing */
             qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
           }
         }
         else
         {
           /* Request applies to local modem processing */
           qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
         }
      }
       break;

    /* Write a new message given in its raw format */
    case QMI_WMS_RAW_WRITE_REQ_V01:
    /* Modify the metatag of a message in the device storage */
    case QMI_WMS_MODIFY_TAG_REQ_V01:
    /* Delete messages in a specified memory location */
    case QMI_WMS_DELETE_REQ_V01:
    /* Query a list of WMS message indices and meta information within the specified memory storage,
       matching a specified message tag */
    case QMI_WMS_LIST_MESSAGES_REQ_V01:
    /* Query the maximum number of messages that can be stored per memory storage, as well as the number of slots
       currently available */
    case QMI_WMS_GET_STORE_MAX_SIZE_REQ_V01:
      if ( QMI_PROXY_SGLTE_TARGET() )
      {
        if ( QMI_PROXY_SGLTE_LOCAL_MODEM_ONLINE() )
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else
      {
        if ( QMI_PROXY_FUSION_TARGET() )
        {
          /* Remote modem is the master of SIM operations. Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      break;

    /* Sends an ACK to the network for transfer-only routes  */
    case QMI_WMS_SEND_ACK_REQ_V01:
      if ( QMI_PROXY_CSFB_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else if ( QMI_PROXY_SVLTE_TARGET() )
      {
        wms_send_ack_req = ( wms_send_ack_req_msg_v01 * ) decoded_payload;

        /* SMS over IMS */
        if ( wms_send_ack_req->sms_on_ims_valid && ( wms_send_ack_req->sms_on_ims == 0x01 ) )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        /* SMS */
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else if ( QMI_PROXY_SGLTE_TARGET() )
      {
        wms_send_ack_req = ( wms_send_ack_req_msg_v01 * ) decoded_payload;

        QMI_PROXY_DEBUG_MSG("remote modem cs active = %d\n", QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE());

        if( QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    /* Enable o disable the reception of broadcast SMS message */
    case QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01:
    /* Set the broadcast SMS configuration */
    case QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01:
      if ( QMI_PROXY_SGLTE_TARGET() )
      {
        /* Request applies to both modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else
      {
        if ( QMI_PROXY_CSFB_TARGET() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* There is no broadcast service defined in IMS. Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      break;

    /* Query the current broadcast SMS configuration */
    case QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01:
    /* Query SMS parameters */
    case QMI_WMS_GET_SMS_PARAMETERS_REQ_V01:
      if ( QMI_PROXY_SGLTE_TARGET() )
      {
        QMI_PROXY_DEBUG_MSG("remote modem cs active = %d\n", QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE());

        if( QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else
      {
        if ( QMI_PROXY_CSFB_TARGET() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* There is no broadcast service defined in IMS. Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      break;

    /* Send message from memory store */
    case QMI_WMS_SEND_FROM_MEM_STORE_REQ_V01:
      if ( QMI_PROXY_CSFB_TARGET() )
      {
        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else if ( QMI_PROXY_SVLTE_TARGET() )
      {
        wms_send_from_mem_store_req = (wms_send_from_mem_store_req_msg_v01 *) decoded_payload;

        /* SMS over IMS */
        if ( wms_send_from_mem_store_req->sms_on_ims_valid && ( wms_send_from_mem_store_req->sms_on_ims == 0x01 ) )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        /* SMS */
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    /* Register transport layer info events */
    case QMI_WMS_INDICATION_REGISTER_REQ_V01:
    /* Query registered transport layer info events */
    case QMI_WMS_GET_INDICATION_REGISTER_REQ_V01:
      if ( QMI_PROXY_SGLTE_TARGET() )
      {
        /* Request applies to both modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else
      {
        if ( QMI_PROXY_FUSION_TARGET() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      break;

    /* Query transport layer info */
    case QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01:
    /* Query transport NW reg info */
    case QMI_WMS_GET_TRANSPORT_NW_REG_INFO_REQ_V01:
      if ( QMI_PROXY_SGLTE_TARGET() )
      {
        QMI_PROXY_DEBUG_MSG("remote modem cs active = %d\n", QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE());

        if( QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else
      {
        if ( QMI_PROXY_FUSION_TARGET() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      break;

    case QMI_WMS_GET_SERVICE_READY_STATUS_REQ_V01:
      if ( QMI_PROXY_SGLTE_TARGET() )
      {

        /* Request applies to both modem processing */
        if (local_svlte_cache_ptr->wms_get_indication_register_resp.reg_service_ready_events_valid &&
                                     local_svlte_cache_ptr->wms_get_indication_register_resp.reg_service_ready_events)
        {
          user_data_valid = TRUE;
          user_data = QMI_PROXY_LOCAL_CONN_TYPE;
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }

        if (remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_service_ready_events_valid &&
                                     remote_svlte_cache_ptr->wms_get_indication_register_resp.reg_service_ready_events)
        {
          if (user_data_valid)
          {
            user_data = QMI_PROXY_BOTH_CONN_TYPE;
          }
          else
          {
            user_data_valid = TRUE;
            user_data = QMI_PROXY_REMOTE_CONN_TYPE;
          }

          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else
      {
        if ( QMI_PROXY_CSFB_TARGET() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      break;

    default:
      if ( QMI_PROXY_SGLTE_TARGET() )
      {
        QMI_PROXY_DEBUG_MSG("remote modem cs active = %d\n", QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE());

        if( QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else
      {
        if ( QMI_PROXY_CSFB_TARGET() )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
        else
        {
          /* Request applies to local modem processing */
          qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_WMS_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      break;
  }

  /* Setup and dispatch transaction */
  rc = qmi_proxy_setup_and_dispatch_txn( proxy_srvc_id,
                                         proxy_client_id,
                                         proxy_conn_id,
                                         srvc_msg_id,
                                         &local_txn_entry_list,
                                         &remote_txn_entry_list,
                                         rx_txn_id,
                                         async_state,
                                         user_data_valid,
                                         user_data );

  return rc;

} /* qmi_proxy_wms_srvc_arb_hdlr */


/*=========================================================================
  FUNCTION:  qmi_proxy_rx_queue_msg

===========================================================================*/
/*!
    @brief
    Queue unsolicited indication messages.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_rx_queue_msg
(
  qmi_client_type        client_handle,
  unsigned long          msg_id,
  qmi_service_id_type    proxy_srvc_id,
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  void                  *decoded_payload,
  uint32_t               decoded_payload_len
)
{
  QMI_PLATFORM_INIT_SIGNAL_FOR_SEND(0,&qmi_proxy_rx_msg_queue_signal);
  qmi_proxy_rx_queue_msg_locked( client_handle,
                            msg_id,
                            proxy_srvc_id,
                            proxy_client_id,
                            proxy_conn_id,
                            proxy_conn_type,
                            decoded_payload,
                            decoded_payload_len );
  QMI_PLATFORM_FINISH_SIGNAL_SEND(&qmi_proxy_rx_msg_queue_signal);
}

/*=========================================================================
  FUNCTION:  qmi_proxy_rx_queue_msg_locked

===========================================================================*/
/*!
    @brief
    Queue unsolicited indication messages.

    @return
    none
*/
/*=========================================================================*/
static int qmi_proxy_rx_queue_msg_locked
(
  qmi_client_type        client_handle,
  unsigned long          msg_id,
  qmi_service_id_type    proxy_srvc_id,
  qmi_client_id_type     proxy_client_id,
  qmi_connection_id_type proxy_conn_id,
  qmi_proxy_conn_type    proxy_conn_type,
  void                  *decoded_payload,
  uint32_t               decoded_payload_len
)
{

  qmi_proxy_rx_message *rx_msg = NULL;
  int ret = QMI_NO_ERR;

  rx_msg = calloc(1, sizeof(*rx_msg));
  if (rx_msg)
  {
    rx_msg->client_handle = client_handle;
    rx_msg->msg_id = msg_id;
    rx_msg->proxy_srvc_id = proxy_srvc_id;
    rx_msg->proxy_client_id = proxy_client_id;
    rx_msg->proxy_conn_id = proxy_conn_id;
    rx_msg->proxy_conn_type = proxy_conn_type;
    rx_msg->decoded_payload = decoded_payload;
    rx_msg->decoded_payload_len = decoded_payload_len;
    qmi_proxy_queue_push(qmi_proxy_rx_msg_queue, rx_msg);
    QMI_PLATFORM_SEND_SIGNAL_LOCKED(0, &qmi_proxy_rx_msg_queue_signal);
  }
  else
  {
    QMI_PROXY_DEBUG_MSG("%s: Unable to allocate memory indication message\n", __FUNCTION__);
    ret = QMI_INTERNAL_ERR;
  }
  return ret;
}
/*=========================================================================
  FUNCTION:  qmi_proxy_srvc_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Handle QMI Service unsolicited callback indications.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_srvc_unsol_ind_cb
(
  qmi_client_type client_handle,
  unsigned long   msg_id,
  unsigned char   *ind_buf,
  int             ind_buf_len,
  void            *ind_cb_data
)
{
  uint32 unsol_ind_cb_data;
  qmi_service_id_type proxy_srvc_id;
  qmi_client_id_type proxy_client_id;
  qmi_connection_id_type proxy_conn_id;
  qmi_proxy_conn_type proxy_conn_type;
  void *decoded_payload = NULL;
  uint32_t decoded_payload_len = 0;
  unsol_ind_cb_data = (intptr_t) ind_cb_data;
  int rc = QMI_NO_ERR;
  qmi_proxy_rx_message *rx_msg = NULL;

  /* Extract QMI Service ID */
  proxy_srvc_id = QMI_PROXY_UNSOL_IND_CB_USER_DATA_TO_SRVC_ID( unsol_ind_cb_data );

  /* Extract QMI Client ID */
  proxy_client_id = QMI_PROXY_UNSOL_IND_CB_USER_DATA_TO_CLIENT_ID( unsol_ind_cb_data );

  /* Extract QMI Conn ID */
  proxy_conn_id = QMI_PROXY_UNSOL_IND_CB_USER_DATA_TO_CONN_ID( unsol_ind_cb_data );

  /* Extract QMI Proxy Conn Type */
  proxy_conn_type = QMI_PROXY_UNSOL_IND_CB_USER_DATA_TO_CONN_TYPE( unsol_ind_cb_data );

  QMI_PROXY_DEBUG_MSG( "%s ....... %s(%d) Msg ID 0x%x, Client ID %d\n",
                       __FUNCTION__, qmi_proxy_modem_name[ proxy_conn_type ], proxy_conn_type, msg_id, proxy_client_id );

  /* Decode the QMI unsolicted indication */
  rc = qmi_proxy_decode_qmi_srvc_message( proxy_srvc_id,
                                          proxy_client_id,
                                          proxy_conn_type,
                                          proxy_srvc_id,
                                          QMI_IDL_INDICATION,
                                          msg_id,
                                          ind_buf,
                                          ind_buf_len,
                                          &decoded_payload,
                                          &decoded_payload_len );
  if ( rc != QMI_NO_ERR )
  {
    QMI_PROXY_DEBUG_MSG( "Fail to decode QMI %s(%d) Indication 0x%x\n", qmi_proxy_srvc_name[ proxy_srvc_id ], proxy_srvc_id, msg_id );

    return;
  }

  /* Add new entry to message queue */
  qmi_proxy_rx_queue_msg( client_handle,
                          msg_id,
                          proxy_srvc_id,
                          proxy_client_id,
                          proxy_conn_id,
                          proxy_conn_type,
                          decoded_payload,
                          decoded_payload_len );
  return;

}

/*=========================================================================
  FUNCTION:  qmi_proxy_wms_srvc_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Handle QMI WMS Service unsolicited callback indications.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_wms_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
)
{
  uint32 unsol_ind_cb_data;
  unsigned long   msg_id;
  qmi_service_id_type proxy_srvc_id;
  qmi_client_id_type proxy_client_id;
  qmi_connection_id_type proxy_conn_id;
  qmi_proxy_conn_type proxy_conn_type;
  void *decoded_payload = NULL;
  uint32_t decoded_payload_len = 0;
  wms_event_report_ind_msg_v01 *wms_event_report_ind = NULL;
  int rc = QMI_NO_ERR;
  wms_transport_layer_info_ind_msg_v01 *wms_transport_layer_info_ind = NULL;
  wms_service_ready_ind_msg_v01        *wms_service_ready_ind        = NULL;

  /*-----------------------------------------------------------------------*/

  msg_id = rx_msg->msg_id;
  proxy_srvc_id = rx_msg->proxy_srvc_id;
  proxy_client_id = rx_msg->proxy_client_id;
  proxy_conn_type = rx_msg->proxy_conn_type;
  proxy_conn_id = rx_msg->proxy_conn_id;
  decoded_payload = rx_msg->decoded_payload;
  decoded_payload_len = rx_msg->decoded_payload_len;

  if ( QMI_PROXY_FUSION_TARGET() )
  {
    /* MT SMS over IMS */
    if ( ( proxy_conn_type == QMI_PROXY_REMOTE_CONN_TYPE ) && ( msg_id == QMI_WMS_EVENT_REPORT_IND_V01 ) )
    {
      wms_event_report_ind = ( wms_event_report_ind_msg_v01 * ) decoded_payload;
      wms_event_report_ind->sms_on_ims_valid = TRUE;
      wms_event_report_ind->sms_on_ims = 0x01;
    }

    /* Registered on IMS */
    if( msg_id == QMI_WMS_TRANSPORT_LAYER_INFO_IND_V01 )
    {
       wms_transport_layer_info_ind = (wms_transport_layer_info_ind_msg_v01 *) decoded_payload;

       QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.voip_info_mutex);
       qmi_proxy_internal_info.voip_info.is_registered_on_ims = (boolean) wms_transport_layer_info_ind->registered_ind;
       QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.voip_info_mutex );

    }
  }

  if (QMI_PROXY_SGLTE_TARGET())
  {
      if (msg_id == QMI_WMS_SERVICE_READY_IND_V01)
      {
        wms_service_ready_ind = ( wms_service_ready_ind_msg_v01 * ) decoded_payload;
        QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
        qmi_proxy_internal_info.service_ready[proxy_conn_type] = wms_service_ready_ind->ready_status;
        wms_service_ready_ind->ready_status = qmi_proxy_get_cache_service_ready_state();
        QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
      }
  }

  /* Forward the QMI Service Indication to QMI Proxy Client */
  qmi_proxy_srvc_send_qmi_indication( proxy_srvc_id,
                                      proxy_client_id,
                                      proxy_conn_id,
                                      proxy_conn_type,
                                      msg_id,
                                      decoded_payload,
                                      decoded_payload_len );

  /* Release buffer */
  qmi_proxy_free( (void **)&decoded_payload );

} /* qmi_proxy_wms_srvc_unsol_ind_cb */


/*=========================================================================
  FUNCTION:  qmi_proxy_sar_srvc_arb_hdlr

===========================================================================*/
/*!
    @brief
    Perform the arbitration for QMI SAR service

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_sar_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
)
{
  qmi_proxy_txn_entry_list_type local_txn_entry_list, remote_txn_entry_list;
  qmi_proxy_async_state_type async_state = QMI_PROXY_ASYNC_IDLE;
  boolean user_data_valid = FALSE;
  uint32 user_data = 0;
  int rc = QMI_NO_ERR;
  qmi_proxy_svlte_cache_type *remote_svlte_cache_ptr;

  /*-----------------------------------------------------------------------*/

  /* Supress compiler warnings for unused variables */
  (void) client;

  QMI_PROXY_DEBUG_MSG( "%s ....... Msg ID 0x%x\n", __FUNCTION__, srvc_msg_id );

  /* Initialize transaction entry lists */
  qmi_proxy_init_txn_entry_list( &local_txn_entry_list, &remote_txn_entry_list );

  /* Perform the service arbitration */
  switch ( srvc_msg_id )
  {
    /* Set SAR RF state */
    case QMI_SAR_RF_SET_STATE_REQ_MSG_V01:
      if ( QMI_PROXY_CSFB_TARGET() )
      {
        /* Request applies to remote modem processing */
         qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_SAR_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else if ( QMI_PROXY_SVLTE_TARGET() )
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_SAR_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

        /* Request applies to remote modem processing */
        qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_SAR_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else if ( QMI_PROXY_SGLTE_TARGET() )
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_SAR_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

        remote_svlte_cache_ptr = &qmi_proxy_internal_info.svlte_cache[ QMI_PROXY_REMOTE_CONN_TYPE ];
        if ( remote_svlte_cache_ptr->dms_get_oprt_mode_resp.operating_mode == DMS_OP_MODE_ONLINE_V01 )
        {
          /* Request applies to remote modem processing */
          qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_SAR_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
        }
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_SAR_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;

    /* Get SAR RF state */
    case QMI_SAR_RF_GET_STATE_REQ_MSG_V01:
      /* Same state on both modems, just need to read from one modem */
      if ( QMI_PROXY_CSFB_TARGET() )
      {
        /* Request applies to remote modem processing */
         qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_SAR_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      else
      {
        /* Request applies to local modem processing */
        qmi_proxy_add_txn_entry( &local_txn_entry_list, QMI_SAR_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );
      }
      break;
  }

  /* Setup and dispatch transaction */
  rc = qmi_proxy_setup_and_dispatch_txn( proxy_srvc_id,
                                         proxy_client_id,
                                         proxy_conn_id,
                                         srvc_msg_id,
                                         &local_txn_entry_list,
                                         &remote_txn_entry_list,
                                         rx_txn_id,
                                         async_state,
                                         user_data_valid,
                                         user_data );

  return rc;

} /* qmi_proxy_sar_srvc_arb_hdlr */


/*=========================================================================
  FUNCTION:  qmi_proxy_sar_srvc_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Handle QMI SAR Service unsolicited callback indications.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_sar_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
)
{
  qmi_service_id_type proxy_srvc_id;
  qmi_client_id_type proxy_client_id;
  qmi_connection_id_type proxy_conn_id;
  qmi_proxy_conn_type proxy_conn_type;
  unsigned long   msg_id;
  void *decoded_payload = NULL;
  uint32_t decoded_payload_len = 0;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  msg_id = rx_msg->msg_id;
  proxy_srvc_id = rx_msg->proxy_srvc_id;
  proxy_client_id = rx_msg->proxy_client_id;
  proxy_conn_type = rx_msg->proxy_conn_type;
  proxy_conn_id = rx_msg->proxy_conn_id;
  decoded_payload = rx_msg->decoded_payload;
  decoded_payload_len = rx_msg->decoded_payload_len;

  /* Forward the QMI Service Indication to QMI Proxy Client */
  qmi_proxy_srvc_send_qmi_indication( proxy_srvc_id,
                                      proxy_client_id,
                                      proxy_conn_id,
                                      proxy_conn_type,
                                      msg_id,
                                      decoded_payload,
                                      decoded_payload_len );

  /* Release buffer */
  qmi_proxy_free( decoded_payload );

} /* qmi_proxy_sar_srvc_unsol_ind_cb */


/*=========================================================================
  FUNCTION:  qmi_proxy_ims_vt_srvc_arb_hdlr

===========================================================================*/
/*!
    @brief
    Perform the arbitration for QMI IMS VTmservice

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_ims_vt_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
)
{
  qmi_proxy_txn_entry_list_type local_txn_entry_list, remote_txn_entry_list;
  qmi_proxy_async_state_type async_state = QMI_PROXY_ASYNC_IDLE;
  boolean user_data_valid = FALSE;
  uint32 user_data = 0;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  /* Supress compiler warnings for unused variables */
  (void) client;

  QMI_PROXY_DEBUG_MSG( "%s ....... Msg ID 0x%x\n", __FUNCTION__, srvc_msg_id );

  /* Initialize transaction entry lists */
  qmi_proxy_init_txn_entry_list( &local_txn_entry_list, &remote_txn_entry_list );

  /* Request applies to remote modem processing  */
  qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_IMS_VT_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

  /* Setup and dispatch transaction */
  rc = qmi_proxy_setup_and_dispatch_txn( proxy_srvc_id,
                                         proxy_client_id,
                                         proxy_conn_id,
                                         srvc_msg_id,
                                         &local_txn_entry_list,
                                         &remote_txn_entry_list,
                                         rx_txn_id,
                                         async_state,
                                         user_data_valid,
                                         user_data );

  return rc;

} /* qmi_proxy_ims_vt_srvc_arb_hdlr */


/*=========================================================================
  FUNCTION:  qmi_proxy_ims_vt_srvc_arb_hdlr

===========================================================================*/
/*!
    @brief
    Perform the arbitration for QMI IMS PRESENCE service

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static int qmi_proxy_ims_presence_srvc_arb_hdlr
(
  qmi_service_id_type        proxy_srvc_id,
  qmi_client_id_type         proxy_client_id,
  qmi_connection_id_type     proxy_conn_id,
  unsigned long              rx_txn_id,
  unsigned long              srvc_msg_id,
  void                       *decoded_payload,
  uint32_t                   decoded_payload_len,
  qmi_proxy_client_info_type *client
)
{
  qmi_proxy_txn_entry_list_type local_txn_entry_list, remote_txn_entry_list;
  qmi_proxy_async_state_type async_state = QMI_PROXY_ASYNC_IDLE;
  boolean user_data_valid = FALSE;
  uint32 user_data = 0;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  /* Supress compiler warnings for unused variables */
  (void) client;

  QMI_PROXY_DEBUG_MSG( "%s ....... Msg ID 0x%x\n", __FUNCTION__, srvc_msg_id );

  /* Initialize transaction entry lists */
  qmi_proxy_init_txn_entry_list( &local_txn_entry_list, &remote_txn_entry_list );

  /* Request applies to remote modem processing  */
  qmi_proxy_add_txn_entry( &remote_txn_entry_list, QMI_IMS_PRESENCE_SERVICE, srvc_msg_id, decoded_payload, decoded_payload_len );

  /* Setup and dispatch transaction */
  rc = qmi_proxy_setup_and_dispatch_txn( proxy_srvc_id,
                                         proxy_client_id,
                                         proxy_conn_id,
                                         srvc_msg_id,
                                         &local_txn_entry_list,
                                         &remote_txn_entry_list,
                                         rx_txn_id,
                                         async_state,
                                         user_data_valid,
                                         user_data );

  return rc;

} /* qmi_proxy_ims_presence_srvc_arb_hdlr */


/*=========================================================================
  FUNCTION:  qmi_proxy_ims_srvc_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Handle QMI IMS VT Service unsolicited callback indications.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_ims_vt_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
)
{
  qmi_service_id_type proxy_srvc_id;
  qmi_client_id_type proxy_client_id;
  qmi_connection_id_type proxy_conn_id;
  qmi_proxy_conn_type proxy_conn_type;
  void *decoded_payload = NULL;
  unsigned long   msg_id;
  uint32_t decoded_payload_len = 0;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  msg_id = rx_msg->msg_id;
  proxy_srvc_id = rx_msg->proxy_srvc_id;
  proxy_client_id = rx_msg->proxy_client_id;
  proxy_conn_type = rx_msg->proxy_conn_type;
  proxy_conn_id = rx_msg->proxy_conn_id;
  decoded_payload = rx_msg->decoded_payload;
  decoded_payload_len = rx_msg->decoded_payload_len;

  /* Forward the QMI Service Indication to QMI Proxy Client */
  qmi_proxy_srvc_send_qmi_indication( proxy_srvc_id,
                                      proxy_client_id,
                                      proxy_conn_id,
                                      proxy_conn_type,
                                      msg_id,
                                      decoded_payload,
                                      decoded_payload_len );

  /* Release buffer */
  qmi_proxy_free( (void **)&decoded_payload );

} /* qmi_proxy_ims_vt_srvc_unsol_ind_cb */


/*=========================================================================
  FUNCTION:  qmi_proxy_ims_srvc_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Handle QMI IMS Presence Service unsolicited callback indications.

    @return
    none
*/
/*=========================================================================*/
static void qmi_proxy_ims_presence_srvc_unsol_ind_cb
(
  qmi_proxy_rx_message *rx_msg
)
{
  uint32 unsol_ind_cb_data;
  qmi_service_id_type proxy_srvc_id;
  qmi_client_id_type proxy_client_id;
  qmi_connection_id_type proxy_conn_id;
  qmi_proxy_conn_type proxy_conn_type;
  unsigned long   msg_id;
  void *decoded_payload = NULL;
  uint32_t decoded_payload_len = 0;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  msg_id = rx_msg->msg_id;
  proxy_srvc_id = rx_msg->proxy_srvc_id;
  proxy_client_id = rx_msg->proxy_client_id;
  proxy_conn_type = rx_msg->proxy_conn_type;
  proxy_conn_id = rx_msg->proxy_conn_id;
  decoded_payload = rx_msg->decoded_payload;
  decoded_payload_len = rx_msg->decoded_payload_len;

  /* Forward the QMI Service Indication to QMI Proxy Client */
  qmi_proxy_srvc_send_qmi_indication( proxy_srvc_id,
                                      proxy_client_id,
                                      proxy_conn_id,
                                      proxy_conn_type,
                                      msg_id,
                                      decoded_payload,
                                      decoded_payload_len );

  /* Release buffer */
  qmi_proxy_free( (void **)&decoded_payload );

} /* qmi_proxy_ims_presence_srvc_unsol_ind_cb */


/*=========================================================================
  FUNCTION:  qmi_proxy_get_remote_modem_connection_name

===========================================================================*/
/*!
    @brief
    Determine the port name based on the target

    @return
    The connection name to use for remote modem.
*/
/*=========================================================================*/
static const char *qmi_proxy_get_remote_modem_connection_name
(
  void
)
{
  const char *remote_modem_connection_name = QMI_PORT_RMNET_SDIO_0;

  if (QMI_PROXY_SGLTE_TARGET())
  {
    remote_modem_connection_name = QMI_PORT_RMNET_SMUX_0;
  }

  return remote_modem_connection_name;
}

/*=========================================================================
  FUNCTION:  qmi_proxy_get_remote_modem_connection_id

===========================================================================*/
/*!
    @brief
    Determine the connection id name based on the target

    @return
    The connection id to use for remote modem.
*/
/*=========================================================================*/
static int qmi_proxy_get_remote_modem_connection_id
(
  void
)
{
  int remote_modem_connection_id = QMI_CONN_ID_RMNET_SDIO_0;

  if ( QMI_PROXY_SGLTE_TARGET() )
  {
    remote_modem_connection_id = QMI_CONN_ID_RMNET_SMUX_0;
  }

  return remote_modem_connection_id;
}

/*=========================================================================
  FUNCTION:  qmi_proxy_get_local_modem_connection_name

===========================================================================*/
/*!
    @brief
    Determine the port name based on the target

    @return
    The connection name to use for local modem.
*/
/*=========================================================================*/
static const char *qmi_proxy_get_local_modem_connection_name
(
  void
)
{
  const char *local_modem_connection_name = QMI_PORT_RMNET_0;

  if (QMI_PROXY_SGLTE_2_TARGET())
  {
    local_modem_connection_name = QMI_PORT_RMNET_USB_0;
  }

  return local_modem_connection_name;
}

/*=========================================================================
  FUNCTION:  qmi_proxy_get_local_modem_connection_id

===========================================================================*/
/*!
    @brief
    Determine the connection id based on the target

    @return
    The connection id to use for local modem.
*/
/*=========================================================================*/
static int qmi_proxy_get_local_modem_connection_id
(
  void
)
{
  int local_modem_connection_id = QMI_CONN_ID_RMNET_0;

  if ( QMI_PROXY_SGLTE_2_TARGET() )
  {
    local_modem_connection_id = QMI_CONN_ID_RMNET_USB_0;
  }

  return local_modem_connection_id;
}
/*=========================================================================
  FUNCTION:  qmi_proxy_srvc_handle_request

===========================================================================*/
/*!
    @brief
    Handle QMI Service Request

    @return
    0 if success. Otherwise, QMI error code
*/
/*=========================================================================*/
static void *qmi_proxy_srvc_handle_request
(
  qmi_proxy_thread_data_type *data
)
{
  qmi_proxy_thread_data_type *thread_data = NULL;
  qmi_qmux_if_msg_hdr_type *msg_hdr;
  unsigned char *msg;
  int msg_len;
  qmi_proxy_client_info_type *client = NULL;
  unsigned char *rx_msg = NULL;
  int rx_msg_len = 0;
  qmi_service_msg_type rx_msg_type;
  unsigned long rx_txn_id;
  unsigned long srvc_msg_id;
  unsigned long srvc_msg_len = 0;
  qmi_proxy_conn_type conn_type = QMI_PROXY_LOCAL_CONN_TYPE;
  void *decoded_payload = NULL;
  uint32_t decoded_payload_len = 0;
  qmi_proxy_reg_info_type *reg_ptr = NULL;
  int rc = QMI_NO_ERR;

  /*-----------------------------------------------------------------------*/

  /* Extract the input data */
  thread_data =  data;
  if ( thread_data == NULL )
  {
    QMI_PROXY_ERR_MSG( "%s\n", "Invalid parameter in QMI Proxy Service Request Msg" );
    rc = QMI_INTERNAL_ERR;
  }
  else
  {
    msg_hdr = &thread_data->msg_hdr;
    msg = thread_data->msg;
    msg_len = thread_data->msg_len;

    /* Sanity check on parameters */
    if ( ( msg_hdr == NULL ) || ( msg == NULL ) || ( msg_len == 0 ) )
    {
      QMI_PROXY_ERR_MSG( "%s\n", "Invalid parameter in QMI Proxy Service Request Msg" );
      rc = QMI_INTERNAL_ERR;
    }
  }

  if ( rc == QMI_NO_ERR )
  {
    rx_msg = msg;
    rx_msg_len = msg_len;

    /* Read QMI standard txn header */
    if ( qmi_proxy_read_std_txn_hdr( &rx_msg,
                                     &rx_msg_len,
                                     &rx_txn_id,
                                     &rx_msg_type ) < 0 )
    {
      QMI_PROXY_ERR_MSG( "%s\n", "Fail to read QMI std txn header\n" );
      rc = QMI_INTERNAL_ERR;
    }
    /* Should be a request */
    else if ( rx_msg_type != QMI_SERVICE_REQUEST_MSG )
    {
      QMI_PROXY_ERR_MSG( "RX: rx_msg_type (%d) != QMI_SERVICE_REQUEST_MSG\n", rx_msg_type );
      rc = QMI_INTERNAL_ERR;
    }
    /* Read the QMI standard service message header */
    else if ( qmi_proxy_read_std_srvc_msg_hdr( &rx_msg,
                                               &rx_msg_len,
                                               &srvc_msg_id,
                                               &srvc_msg_len ) < 0 )
    {
      QMI_PROXY_ERR_MSG( "%s\n", "Fail to read QMI std srvc msg header" );
      rc = QMI_INTERNAL_ERR;
    }
    /* Make sure that the message length specified in the QMI message
    ** matches the message length that we received from SMD
    */
    else if ( srvc_msg_len != (unsigned long) rx_msg_len )
    {
      QMI_PROXY_ERR_MSG( "RX: srvc_msg_len (%d) != rx_msg_len (%d)\n", (int)srvc_msg_len, (int)rx_msg_len );
      /* send error response */
      qmi_proxy_srvc_err_hdlr_tbl[msg_hdr->qmi_service_id ]( msg_hdr->qmi_client_id,
                                   QMI_CONN_ID_FIRST,
                                   conn_type,
                                   srvc_msg_id,
                                   rx_txn_id,
                                   QMI_ERR_MALFORMED_MSG_V01);

      rc = QMI_INTERNAL_ERR;
    }
    /* Check whether QMI service is supported by QMI Proxy */
    else if ( !QMI_PROXY_SRVC_IS_SUPPORTED( msg_hdr->qmi_service_id ) )
    {
      QMI_PROXY_ERR_MSG( "QMI service = %d not supported by QMI Proxy\n", msg_hdr->qmi_service_id );
      rc = QMI_SERVICE_ERR;
    }
  }

  if ( rc == QMI_NO_ERR )
  {
    QMI_PROXY_DEBUG_MSG( "%s ....... QMI %s(%d), Msg ID 0x%x, RX Msg Len %d\n",
                         __FUNCTION__, qmi_proxy_srvc_name[ msg_hdr->qmi_service_id ], msg_hdr->qmi_service_id,
                         srvc_msg_id, rx_msg_len );

    /* Decode message */
    reg_ptr = &qmi_proxy_reg_info_tbl[ qmi_proxy_cfg ][ msg_hdr->qmi_service_id ];
    if ( reg_ptr->local_srvc_required )
    {
      conn_type = QMI_PROXY_LOCAL_CONN_TYPE;
    }
    else
    {
      conn_type = QMI_PROXY_REMOTE_CONN_TYPE;
    }

    rc = qmi_proxy_decode_qmi_srvc_message( msg_hdr->qmi_service_id,
                                            msg_hdr->qmi_client_id,
                                            conn_type,
                                            msg_hdr->qmi_service_id,
                                            QMI_IDL_REQUEST,
                                            srvc_msg_id,
                                            rx_msg,
                                            rx_msg_len,
                                            &decoded_payload,
                                            &decoded_payload_len );
    if ( rc != QMI_NO_ERR )
    {
      QMI_PROXY_ERR_MSG( "Fail to decode QMI %s(%u) Msg ID 0x%x\n",
                         qmi_proxy_srvc_name[ msg_hdr->qmi_service_id ], msg_hdr->qmi_service_id, srvc_msg_id );

      /* send error response */
      qmi_proxy_srvc_err_hdlr_tbl[msg_hdr->qmi_service_id ]( msg_hdr->qmi_client_id,
                                   QMI_CONN_ID_FIRST,
                                   conn_type,
                                   srvc_msg_id,
                                   rx_txn_id,
                                   QMI_ERR_MALFORMED_MSG_V01);

      rc = QMI_INTERNAL_ERR;
    }
    else
    {
      /* Call Service Arbitration Handler for processing */
      rc = qmi_proxy_srvc_arb_hdlr_tbl[ msg_hdr->qmi_service_id ]( msg_hdr->qmi_service_id,
                                                                   msg_hdr->qmi_client_id,
                                                                   msg_hdr->qmi_conn_id,
                                                                   rx_txn_id,
                                                                   srvc_msg_id,
                                                                   decoded_payload,
                                                                   decoded_payload_len,
                                                                   client );
    }
  }

  /* Free buffer */
  qmi_proxy_free( (void **)&decoded_payload );


  /* Free thread data */
  if ( thread_data != NULL )
  {
    if ( thread_data->msg != NULL )
    {
      qmi_proxy_free( (void **)&thread_data->msg );
    }

    qmi_proxy_free( (void **)&thread_data );
  }

  return NULL;

} /* qmi_proxy_srvc_handle_request */

/*===========================================================================
  FUNCTION  qmi_proxy_process_client_msg
===========================================================================*/
/*!
@brief
  Function to process the message received from clients of the proxy server

@return
   FALSE in case of error
   SUCCESS in case of no error
*/
/*=========================================================================*/
int
qmi_proxy_process_client_msg
(
  int fd
)
{
  int buf_size;
  unsigned char msg_hdr[QMI_QMUX_HDR_SIZE];
  unsigned char msg[QMI_MAX_MSG_SIZE];
  pthread_t tid;
  qmi_proxy_thread_data_type *thread_data = NULL;
  pthread_attr_t thread_attr;
  unsigned char *msg_ptr;
  int                     i_f_byte;
  unsigned short          length, msg_len;
  qmi_client_id_type      client_id;
  unsigned char           control_flags;
  qmi_service_id_type     service_id;
  unsigned char           temp_char;

  msg_ptr = msg;

  if ((buf_size = recv (fd,
                        (void *)&msg_hdr,
                        (size_t)QMI_QMUX_HDR_SIZE,
                        0)) <= 0)

  {
    QMI_PROXY_ERR_MSG ("%s: RX on fd=%d returned error=%d\n",__FUNCTION__, fd, buf_size);
    return FALSE;
  }

  if( buf_size != QMI_QMUX_HDR_SIZE )
  {
    QMI_PROXY_ERR_MSG ("%s: RX recv msg len = %d does not match with qmux header size = %d on fc = %d\n",
                         __FUNCTION__, buf_size, QMI_QMUX_HDR_SIZE, fd);
     return TRUE;
  }

  msg_ptr = msg_hdr;

  /* Read the I/F byte, make sure it is a 1 */
  READ_8_BIT_VAL(msg_ptr,i_f_byte);

  if (i_f_byte != 1)
  {
    QMI_PROXY_ERR_MSG ("%s: Received invalid I/F byte = %d\n",__FUNCTION__, i_f_byte);
    return TRUE;
  }

  /* Read the message length */
  READ_16_BIT_VAL(msg_ptr, length);

  /* Read the control flags */
  READ_8_BIT_VAL(msg_ptr,control_flags);

  /* Read the service_type and client_id */
  READ_8_BIT_VAL(msg_ptr,temp_char);
  service_id = (qmi_service_id_type) temp_char;
  READ_8_BIT_VAL(msg_ptr,temp_char);
  client_id = (qmi_client_id_type) temp_char;

  QMI_PROXY_DEBUG_MSG("QMUX header I/F(%d), length(%d) control flag(%c) service type(%d) client id(%d)\n",
                                      i_f_byte,
                                      length,
                                      control_flags,
                                      (int)service_id,
                                      (int)client_id);

   msg_ptr = msg;
   msg_len = length - QMI_QMUX_HDR_SIZE + 1;
  /* read the message now, as length is decoded */
  if ((buf_size = recv (fd,
                        (void *)msg_ptr,
                        (size_t)msg_len,
                        0)) <= 0)

  {
    QMI_PROXY_ERR_MSG ("%s: RX on fd=%d returned error, buff_size = %d\n",__FUNCTION__, fd, buf_size);
    return FALSE;
  }

  if( buf_size != msg_len )
  {
    QMI_PROXY_DEBUG_MSG ("%s: mismatch between the expected =%d and read = %d message size from fd = %d\n",__FUNCTION__, buf_size, msg_len, fd);
  }

  QMI_PROXY_DEBUG_MSG ("recevied request from client on fd = %d with  msg_len =%d\n",fd, buf_size);

  size_t remaining_bytes = buf_size;

  /* Spawn thread to process QMI Proxy request */
  thread_data =  qmi_proxy_malloc( sizeof( qmi_proxy_thread_data_type ) );
  if ( thread_data == NULL )
  {
      QMI_PROXY_ERR_MSG( "%s\n", "Fail to allocate thread data" );
      return TRUE;
   }
   else
   {
      thread_data->msg_hdr.qmi_client_id  = client_id;
      thread_data->msg_hdr.qmi_service_id  = service_id;
      thread_data->msg_hdr.qmi_conn_id  = QMI_CONN_ID_PROXY;
      thread_data->msg_len = remaining_bytes;
      thread_data->msg = NULL;
      thread_data->fd = fd;

      if ( thread_data->msg_len > 0 )
      {
         thread_data->msg = qmi_proxy_malloc( thread_data->msg_len );
         if ( thread_data->msg == NULL )
         {
            qmi_proxy_free( (void **)&thread_data );
            return TRUE;
          }
          else
          {
            memcpy( thread_data->msg, msg_ptr, thread_data->msg_len );
          }
      }

      pthread_attr_init( &thread_attr );
      pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );

      if( service_id != QMI_PROXY_CTL_SERVICE )
      {
        /* process the messages sequentially instead of spanning multile threads */
        qmi_proxy_srvc_handle_request( thread_data );
      }
      else
      {
        if ( pthread_create( &tid, &thread_attr, qmi_proxy_ctl_handle_request, (void *) thread_data ) != 0 )
        {
          QMI_PROXY_ERR_MSG( "%s\n", "Fail to spawn thread to handle QMI CTL Request" );
        }
      }
      pthread_attr_destroy( &thread_attr );
    }

  return TRUE;
}  /* qmi_proxy_process_client_msg */

/*===========================================================================
  FUNCTION  qmi_proxy_send_response_to_clients
===========================================================================*/
/*!
@brief
  Function to send a message to a particular client process based on the
  file descriptor passed

@return
   NONE

*/
/*=========================================================================*/
void
qmi_proxy_send_response_to_clients
(
  unsigned char *msg,
  int  msg_len,
  qmi_service_id_type srvc_id,
  qmi_client_id_type client_id,
  unsigned char control_flag,
  int           fd
)
{
  int  ret;
  unsigned char  *tmp_msg_ptr;

  msg -= QMI_QMUX_HDR_SIZE;
  msg_len += QMI_QMUX_HDR_SIZE;

  tmp_msg_ptr = msg;

  /* I/F type is 1 for a QMUX message */
  WRITE_8_BIT_VAL (tmp_msg_ptr, 1);

  /* Length is length of message to send which includes the QMUX header, but since
  ** QMI_QMUX_HDR_SIZE includes the I/F byte and the length field in a QMUX
  ** message doesn't include this, we need to subtract 1
  */
  WRITE_16_BIT_VAL (tmp_msg_ptr, (msg_len - 1));

  /* Control flags byte should be set to 0 for control point */
  WRITE_8_BIT_VAL (tmp_msg_ptr, control_flag);

  /* Now put in service type and client ID */
  WRITE_8_BIT_VAL (tmp_msg_ptr, srvc_id);
  WRITE_8_BIT_VAL (tmp_msg_ptr, client_id);

  QMI_PROXY_DEBUG_MSG("QMUX header I/F(%d), length(%d) control flag(%c) service type(%d) client id(%d), fd(%d)\n",
                                       1,
                                       msg_len - 1,
                                       control_flag,
                                       srvc_id,
                                       client_id,
                                       fd);

  if ((ret = send ((fd),
                  (void *) msg,
                  (size_t) msg_len,
                   0)) < 0)
  {
    QMI_PROXY_ERR_MSG ("proxy_server: TX failed on fd=%d, error=%d\n", fd, ret);
  }
  else if (ret != msg_len)
  {
    QMI_PROXY_ERR_MSG ("proxy_server: TX failed on fd=%d, not all bytes sent to clientID\n", fd );
    QMI_PROXY_ERR_MSG ("proxy_server: msg_len=%d, num of bytes sent = %d\n",msg_len, ret);
  }

}/* qmi_proxy_send_response_to_clients */

/*===========================================================================
  FUNCTION  qmi_proxy_qmux_server_init_listener
===========================================================================*/
/*!
@brief
  Function to setup the QMUX listener socket

@return
   NONE

*/
/*=========================================================================*/
static void
qmi_proxy_init_qmux_listener(void)
{
  int len, rc;
  struct sockaddr_un addr;
  int    path_len;

  QMI_PROXY_DEBUG_MSG ("entering function %s \n",__FUNCTION__);

 /* Get the connection listener socket */
  if (( qmi_proxy_qmux_listener_fd = socket (AF_UNIX,SOCK_STREAM,0)) < 0)
  {
    QMI_PROXY_ERR_MSG ("%s: unable to open listener socket, rc = %d\n", __FUNCTION__, qmi_proxy_qmux_listener_fd);
  }

  QMI_PROXY_DEBUG_MSG ("%s: listener_fd = %d \n", __FUNCTION__, qmi_proxy_qmux_listener_fd);

  /* Unlink socket path name just in case.... */
  unlink (QMI_PROXY_QMUX_CONN_SOCKET_PATH);

  /* setup for bind */
  memset (&addr,0, sizeof (struct sockaddr_un));
  path_len = strlen (QMI_PROXY_QMUX_CONN_SOCKET_PATH);
  addr.sun_family = AF_UNIX;
  memcpy (&addr.sun_path[0],QMI_PROXY_QMUX_CONN_SOCKET_PATH,path_len);
  addr.sun_path[path_len] = '\0';

  len = offsetof (struct sockaddr_un, sun_path) + path_len;

  QMI_PROXY_DEBUG_MSG ("%s: addr path = %s, len = %d\n",__FUNCTION__, addr.sun_path, len);

  /* Bind socket to address */
  if ((rc = bind (qmi_proxy_qmux_listener_fd, (struct sockaddr *)&addr, len)) < 0)
  {
    QMI_PROXY_ERR_MSG ("%s: unable to bind to listener socket, rc = %d\n",__FUNCTION__, rc);
  }

  QMI_PROXY_DEBUG_MSG ("%s: binding fd (%d) to socket = %s \n", __FUNCTION__, qmi_proxy_qmux_listener_fd, addr.sun_path);

  /* Make socket a listener */
  if ((rc = listen ( qmi_proxy_qmux_listener_fd,5)) < 0)
  {
    QMI_PROXY_ERR_MSG("%s: unable to listen with listener socket, rc = %d\n",__FUNCTION__, rc);
  }

  /* Set the maxfd and the bit in the master_fd */
  if (qmi_proxy_qmux_listener_fd > qmi_proxy_qmux_max_fd)
  {
    qmi_proxy_qmux_max_fd = qmi_proxy_qmux_listener_fd;
  }
  FD_SET ( qmi_proxy_qmux_listener_fd,&qmi_proxy_qmux_master_fd_set);

  QMI_PROXY_DEBUG_MSG ("%s:  Added listener, maxfd=%d\n",__FUNCTION__, qmi_proxy_qmux_max_fd);

  QMI_PROXY_DEBUG_MSG ("exiting function %s \n",__FUNCTION__);
}/* qmi_proxy_qmux_server_init_listener */


/*===========================================================================
  FUNCTION  qmi_proxy_tether_server_init_listener
===========================================================================*/
/*!
@brief
  Function to setup the TETHER listener socket

@return
   NONE

*/
/*=========================================================================*/
static void
qmi_proxy_init_tether_listener(void)
{
  int len, rc;
  struct sockaddr_un addr;
  int    path_len;

  QMI_PROXY_DEBUG_MSG ("entering function %s \n",__FUNCTION__);

 /* Get the connection listener socket */
  if ((qmi_proxy_tether_listener_fd = socket (AF_UNIX,SOCK_STREAM,0)) < 0)
  {
    QMI_PROXY_ERR_MSG ("%s: unable to open listener socket, rc = %d\n", __FUNCTION__, qmi_proxy_tether_listener_fd);
  }

  QMI_PROXY_DEBUG_MSG ("%s: listener_fd = %d \n", __FUNCTION__, qmi_proxy_tether_listener_fd);

  /* Unlink socket path name just in case.... */
  unlink (QMI_PROXY_TETHER_CONN_SOCKET_PATH);

  /* setup for bind */
  memset (&addr,0, sizeof (struct sockaddr_un));
  path_len = strlen (QMI_PROXY_TETHER_CONN_SOCKET_PATH);
  addr.sun_family = AF_UNIX;
  memcpy (&addr.sun_path[0],QMI_PROXY_TETHER_CONN_SOCKET_PATH,path_len);
  addr.sun_path[path_len] = '\0';

  len = offsetof (struct sockaddr_un, sun_path) + path_len;

  QMI_PROXY_DEBUG_MSG ("%s: addr path = %s, len = %d\n",__FUNCTION__, addr.sun_path,len);

  /* Bind socket to address */
  if ((rc = bind ( qmi_proxy_tether_listener_fd, (struct sockaddr *)&addr, len)) < 0)
  {
    QMI_PROXY_ERR_MSG ("%s: unable to bind to listener socket, rc = %d\n",__FUNCTION__, rc);
  }

  QMI_PROXY_DEBUG_MSG ("%s: binding fd (%d) to socket = %s \n", __FUNCTION__, qmi_proxy_tether_listener_fd, addr.sun_path);

  /* Make socket a listener */
  if ((rc = listen ( qmi_proxy_tether_listener_fd,5)) < 0)
  {
    QMI_PROXY_ERR_MSG("%s: unable to listen with listener socket, rc = %d\n",__FUNCTION__, rc);
  }

  /* Set the maxfd and the bit in the master_fd */
  if ( qmi_proxy_tether_listener_fd > qmi_proxy_tether_max_fd)
  {
    qmi_proxy_tether_max_fd =  qmi_proxy_tether_listener_fd;
  }
  FD_SET ( qmi_proxy_tether_listener_fd,&qmi_proxy_tether_master_fd_set);

  QMI_PROXY_DEBUG_MSG ("%s:  Added listener, maxfd=%d\n",__FUNCTION__, qmi_proxy_tether_max_fd);

  QMI_PROXY_DEBUG_MSG ("exiting function %s \n",__FUNCTION__);
}/* qmi_proxy_tether_server_init_listener */


/*===========================================================================
  FUNCTION  qmi_proxy_init_new_qmux_client
===========================================================================*/
/*!
@brief
  Function to accept the new clients on the QMUX socket

@return
   NONE

*/
/*=========================================================================*/
static void
qmi_proxy_init_new_qmux_client(void)
{
  int client_fd, rc;
  socklen_t len;
  struct stat stats;
  struct sockaddr_un addr;

  memset (&addr,0,sizeof(struct sockaddr_un));
  len = sizeof(struct sockaddr_un);

  QMI_PROXY_DEBUG_MSG ("entering %s function\n",__FUNCTION__);

  if ((client_fd = accept ( qmi_proxy_qmux_listener_fd, (struct sockaddr *)&addr, &len)) < 0)
  {
    QMI_PROXY_ERR_MSG ("%s: unable to accept on listener socket, rc = %d\n",__FUNCTION__, client_fd);
    return;
  }

  len -= offsetof (struct sockaddr_un, sun_path);
  addr.sun_path[len-1] = '\0';


  if (( rc = stat (addr.sun_path, &stats)) < 0)
  {
    QMI_PROXY_ERR_MSG ("%s: unable to stat client socket file \"%s\", rc = %d\n",__FUNCTION__,addr.sun_path,rc);
    close(client_fd);
    return;
  }

  if (S_ISSOCK (stats.st_mode) == 0)
  {
    QMI_PROXY_ERR_MSG ("%s: client socket file not a socket file, rc = %d\n",__FUNCTION__, rc);
    close(client_fd);
    return;
  }

  /* No longer need the temp file */
  unlink (addr.sun_path);

  /* Add the new fd to the master fd set */
  FD_SET (client_fd,&qmi_proxy_qmux_master_fd_set);
  if (client_fd > qmi_proxy_qmux_max_fd)
  {
    qmi_proxy_qmux_max_fd = client_fd;
  }

  QMI_PROXY_DEBUG_MSG ("%s: added new client, fd=%d, max_fd=%d\n",__FUNCTION__, client_fd, qmi_proxy_qmux_max_fd);

}/* qmi_proxy_init_new_qmux_client */


/*===========================================================================
  FUNCTION  qmi_proxy_init_new_tether_client
===========================================================================*/
/*!
@brief
  Function to accept the new clients on the TETHER socket

@return
   NONE

*/
/*=========================================================================*/
static void
qmi_proxy_init_new_tether_client(void)
{
  int client_fd, rc;
  socklen_t len;
  struct stat stats;
  struct sockaddr_un addr;

  memset (&addr,0,sizeof(struct sockaddr_un));
  len = sizeof(struct sockaddr_un);

  QMI_PROXY_DEBUG_MSG ("entering %s function\n",__FUNCTION__);

  if ((client_fd = accept ( qmi_proxy_tether_listener_fd, (struct sockaddr *)&addr, &len)) < 0)
  {
    QMI_PROXY_ERR_MSG ("%s: unable to accept on listener socket, rc = %d\n",__FUNCTION__, client_fd);
    return;
  }

  len -= offsetof (struct sockaddr_un, sun_path);
  addr.sun_path[len-1] = '\0';


  if (( rc = stat (addr.sun_path, &stats)) < 0)
  {
    QMI_PROXY_ERR_MSG ("%s: unable to stat client socket file \"%s\", rc = %d\n",__FUNCTION__, addr.sun_path,rc);
    close(client_fd);
    return;
  }

  if (S_ISSOCK (stats.st_mode) == 0)
  {
    QMI_PROXY_ERR_MSG ("%s: client socket file not a socket file, rc = %d\n",__FUNCTION__, rc);
    close(client_fd);
    return;
  }

  /* No longer need the temp file */
  unlink (addr.sun_path);

  /* Add the new fd to the master fd set */
  FD_SET (client_fd,&qmi_proxy_tether_master_fd_set);
  if (client_fd > qmi_proxy_tether_max_fd)
  {
    qmi_proxy_tether_max_fd = client_fd;
  }

  QMI_PROXY_DEBUG_MSG ("%s: added new client, fd=%d, max_fd=%d\n",__FUNCTION__, client_fd, qmi_proxy_qmux_max_fd);

}/* qmi_proxy_init_new_tether_client */

/*===========================================================================
  FUNCTION  qmi_proxy_create_qmux_socket
===========================================================================*/
/*!
@brief
  Function to create the QMUX socket

@return
   NONE
*/
/*=========================================================================*/
void *qmi_proxy_create_qmux_socket()
{

  /* Initialize file desciptor sets */
  FD_ZERO (&qmi_proxy_qmux_master_fd_set);
  FD_ZERO (&qmi_proxy_qmux_read_fd_set);
  qmi_proxy_qmux_max_fd = 2;
  int i = 0;
    /* Set up listerner socket */
  qmi_proxy_init_qmux_listener ();

  for (;;)
  {
    int num_fds_ready;
    qmi_proxy_qmux_read_fd_set = qmi_proxy_qmux_master_fd_set;

    num_fds_ready = select (qmi_proxy_qmux_max_fd + 1,&qmi_proxy_qmux_read_fd_set,NULL,NULL,NULL);

    if (num_fds_ready < 0)
    {
      QMI_PROXY_ERR_MSG ("%s: select returns err %d, continuing\n",__FUNCTION__, num_fds_ready);
      continue;
    }

    /* Process new client connection request if we get one */
    if (FD_ISSET( qmi_proxy_qmux_listener_fd, &qmi_proxy_qmux_read_fd_set))
    {
      qmi_proxy_init_new_qmux_client();
      FD_CLR ( qmi_proxy_qmux_listener_fd, &qmi_proxy_qmux_read_fd_set);
    }
    else
    {
      int fd;
      /* Loop through all client FD's and process any with messages */
      for (fd =  qmi_proxy_qmux_listener_fd + 1; fd <= qmi_proxy_qmux_max_fd; fd++)
      {
        if (FD_ISSET (fd, &qmi_proxy_qmux_read_fd_set))
        {
           if (qmi_proxy_process_client_msg(fd) == FALSE)
           {
              /* delete fd from the master fd list, calculate the new max value, close fd*/
              FD_CLR (fd, &qmi_proxy_qmux_master_fd_set);

              /* Find new max_fd */
              if (fd == qmi_proxy_qmux_max_fd)
              {
                QMI_PROXY_DEBUG_MSG("%s, recalculating the max fd for qmux socket \n", __FUNCTION__ );

                for (i = qmi_proxy_qmux_listener_fd; i < fd; i++)
                {
                  if (FD_ISSET(i,&qmi_proxy_qmux_master_fd_set))
                  {
                    qmi_proxy_qmux_max_fd = i;
                  }
                }
              }
              close(fd);
           }
           FD_CLR (fd,&qmi_proxy_qmux_read_fd_set);
        } /* if */
      }  /* for */
    } /* else */
  }
} /* qmi_proxy_create_qmux_socket */


/*===========================================================================
  FUNCTION  qmi_proxy_create_tether_socket
===========================================================================*/
/*!
@brief
  Function to create the TETHER socket

@return
   NONE
*/
/*=========================================================================*/
void *qmi_proxy_create_tether_socket()
{

  /* Initialize file desciptor sets */
  FD_ZERO (&qmi_proxy_tether_master_fd_set);
  FD_ZERO (&qmi_proxy_tether_read_fd_set);
  qmi_proxy_tether_max_fd = 2;
  pthread_t tid;
  qmi_proxy_thread_data_type *thread_data = NULL;
  pthread_attr_t thread_attr;
  int i;

  /* Set up listerner socket */
  qmi_proxy_init_tether_listener ();

  QMI_PROXY_DEBUG_MSG ("%s: added listener, fd=%d, max_fd=%d\n",  __FUNCTION__, qmi_proxy_tether_listener_fd, qmi_proxy_tether_max_fd);

  for (;;)
  {
    int num_fds_ready;
    qmi_proxy_tether_read_fd_set = qmi_proxy_tether_master_fd_set;

    num_fds_ready = select (qmi_proxy_tether_max_fd + 1,&qmi_proxy_tether_read_fd_set,NULL,NULL,NULL);

    if (num_fds_ready < 0)
    {
      QMI_PROXY_ERR_MSG ("%s: select returns err %d, continuing\n",__FUNCTION__, num_fds_ready);
      continue;
    }

    /* Process new client connection request if we get one */
    if (FD_ISSET(qmi_proxy_tether_listener_fd, &qmi_proxy_tether_read_fd_set))
    {
      qmi_proxy_init_new_tether_client();
      FD_CLR (qmi_proxy_tether_listener_fd, &qmi_proxy_tether_read_fd_set);
    }
    else
    {
      int fd;
      /* Loop through all client FD's and process any with messages */
      for (fd = qmi_proxy_tether_listener_fd + 1; fd <= qmi_proxy_tether_max_fd; fd++)
      {
        if (FD_ISSET (fd, &qmi_proxy_tether_read_fd_set))
        {
           if (qmi_proxy_process_client_msg(fd) == FALSE)
           {
              QMI_PROXY_ERR_MSG ("%s: socket closed\n",__FUNCTION__);

              /* delete fd from the master fd list, calculate the new max value, close fd*/
              FD_CLR (fd, &qmi_proxy_tether_master_fd_set);

              /* Find new max_fd */
              if (fd == qmi_proxy_tether_max_fd)
              {
                QMI_PROXY_DEBUG_MSG("%s, recalculating the max fd for tether socket \n", __FUNCTION__ );

                for (i = qmi_proxy_tether_listener_fd; i < fd; i++)
                {
                  if (FD_ISSET(i,&qmi_proxy_tether_master_fd_set))
                  {
                    qmi_proxy_tether_max_fd = i;
                  }
                }
              }
              close(fd);

              /* release proxy client connections associated with file descriptor as socket is closed*/
              thread_data =  qmi_proxy_malloc( sizeof( qmi_proxy_thread_data_type ) );

              if ( thread_data != NULL )
              {
                 thread_data->fd = fd;

                 pthread_attr_init( &thread_attr );
                 pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );

                 if ( pthread_create( &tid, &thread_attr, qmi_proxy_release_clients_with_fd, (void *) thread_data ) != 0 )
                 {
                   QMI_PROXY_ERR_MSG( "%s\n", "Fail to spawn thread for releasing proxy clients associated with FD" );
                 }
                 pthread_attr_destroy( &thread_attr );
              }
           }
        }
        FD_CLR (fd,&qmi_proxy_tether_read_fd_set);
      } /* for */
    }  /* else */
  } /* for */
}/* qmi_proxy_create_tether_socket */


/*==============================================================================
  FUNCTION   qmi_proxy_mcc_same_country
==============================================================================*/
/*!
@brief
  Function that compares two PLMN IDs and determines if the MCCs are in
  the same country. MCCs from NA with different values (in range 310-316)
  are considered as MCCs of same country.

  Refer to 3GPP TS23.122 Annex B.

PARAMETERS

  qmi_proxy_plmn_id_s_type * plmn_1    Ptr to the first PLMN ID.
  qmi_proxy_plmn_id_s_type * plmn_2    Ptr to the second PLMN ID.

@return
  boolean    TRUE if MCCs are of same country, FALSE otherwise
*/
/*==============================================================================*/
static boolean qmi_proxy_mcc_same_country
(
  const qmi_proxy_plmn_id_s_type  * const plmn_1,
  const qmi_proxy_plmn_id_s_type  * const plmn_2
)
{
  char plmn1_mcc_digit_1  = plmn_1->mcc[0];
  char plmn1_mcc_digit_2  = plmn_1->mcc[1];
  char plmn1_mcc_digit_3  = plmn_1->mcc[2];

  char plmn2_mcc_digit_1  = plmn_2->mcc[0];
  char plmn2_mcc_digit_2  = plmn_2->mcc[1];
  char plmn2_mcc_digit_3  = plmn_2->mcc[2];

  boolean value = FALSE;

  if ( (plmn1_mcc_digit_1 == plmn2_mcc_digit_1) &&
       (plmn1_mcc_digit_2 == plmn2_mcc_digit_2) &&
       (plmn1_mcc_digit_3 == plmn2_mcc_digit_3) )
  {
      value = TRUE;
  }
  else if ( (plmn1_mcc_digit_1 == plmn2_mcc_digit_1) &&
            (plmn1_mcc_digit_1 == '3') &&
            (plmn1_mcc_digit_2 == plmn2_mcc_digit_2) &&
            (plmn1_mcc_digit_2 == '1') &&
            ((plmn1_mcc_digit_3 >= '0' &&
              plmn1_mcc_digit_3 <= '6') &&
             (plmn2_mcc_digit_3 >= '0' &&
              plmn1_mcc_digit_3 <= '6'))
          )
  {
      value = TRUE;
  }

  return value;
} /* qmi_proxy_mcc_same_country */

/*==============================================================================
FUNCTION   qmi_proxy_is_sglte_plmn
==============================================================================*/
/*!
@brief
  Function determines if a given PLMN ID is one of the configured SGLTE PLMNs.

PARAMETERS

  qmi_proxy_plmn_id_s_type * plmn    Ptr to the PLMN ID.

@return
  boolean    TRUE if the PLMN is an SGLTE PLMN, FALSE otherwise
*/
/*==============================================================================*/
static boolean qmi_proxy_is_sglte_plmn
(
  const qmi_proxy_plmn_id_s_type * const plmn
)
{
  uint32    index = 0;

  for (index = 0; index < QMI_PROXY_MAX_SGLTE_PLMN_ENTRIES; index++ )
  {
      if( plmn && strlen(plmn->mcc) &&
          qmi_proxy_mcc_same_country(plmn, &sglte_plmn_list[index])
        )
      {
         QMI_PROXY_DEBUG_MSG( "is_sglte_plmn: %d mcc: %s mnc:%s \n",
                              TRUE, plmn->mcc, plmn->mnc);
         return TRUE;
      }
  }

  QMI_PROXY_DEBUG_MSG( "is_sglte_plmn: %d mcc: %s mnc:%s \n",
                       FALSE, plmn->mcc, plmn->mnc);
  return FALSE;

} /* qmi_proxy_is_sglte_plmn */


/*===========================================================================
  FUNCTION  qmi_proxy_use_default_sglte_plmn_list
===========================================================================*/
/*!
@brief
  Sets default values for SGLTE PLMNs

@return
   NONE
*/
/*=========================================================================*/
static void qmi_proxy_use_default_sglte_plmn_list
(
  void
)
{

  QMI_PROXY_DEBUG_MSG( "%s\n", "Using default SGLTE PLMN list" );

  memset(sglte_plmn_list, 0, sizeof(sglte_plmn_list));

  strlcpy(sglte_plmn_list[0].mcc,
          DEFAULT_SGLTE_MCC_1,
          QMI_PROXY_MCC_LENGTH);

  strlcpy(sglte_plmn_list[0].mnc,
          DEFAULT_SGLTE_MNC_1,
          QMI_PROXY_MNC_LENGTH);

  strlcpy(sglte_plmn_list[1].mcc,
          DEFAULT_SGLTE_MCC_2,
          QMI_PROXY_MCC_LENGTH);

  strlcpy(sglte_plmn_list[1].mnc,
          DEFAULT_SGLTE_MNC_2,
          QMI_PROXY_MNC_LENGTH);

  strlcpy(sglte_plmn_list[2].mcc,
          DEFAULT_SGLTE_MCC_3,
          QMI_PROXY_MCC_LENGTH);

  strlcpy(sglte_plmn_list[2].mnc,
          DEFAULT_SGLTE_MNC_3,
          QMI_PROXY_MNC_LENGTH);
}

/*===========================================================================
  FUNCTION  qmi_proxy_sglte_remote_is_full_service
===========================================================================*/
/*!
@brief
  Helper function to evaluate if remote_modem has full service when
  in SGLTE configuration when in a SGLTE PLMN.

@return
   TRUE if remote_modem reported full service
   FALSE otherwise
*/
/*=========================================================================*/
static int qmi_proxy_sglte_remote_is_full_service
(
    nas_sys_info_ind_msg_v01 * nas_sys_info_ind
)
{

  uint8_t gsm_srv_status_info_valid;
  nas_service_status_enum_type_v01 gsm_srv_status;

  gsm_srv_status_info_valid = nas_sys_info_ind->gsm_srv_status_info_valid;
  gsm_srv_status = nas_sys_info_ind->gsm_srv_status_info.srv_status;

  return (gsm_srv_status_info_valid ? gsm_srv_status == NAS_SYS_SRV_STATUS_SRV_V01 : -1);

} /* qmi_proxy_sglte_remote_is_full_service */

/*===========================================================================
  FUNCTION  qmi_proxy_sglte_local_is_full_service
===========================================================================*/
/*!
@brief
  Helper function to evaluate if local modem has full service when
  in SGLTE configuration when in a SGLTE PLMN.

@return
   TRUE if local modem reported full service
   FALSE otherwise
*/
/*=========================================================================*/
static boolean qmi_proxy_sglte_local_is_full_service
(
    nas_sys_info_ind_msg_v01 * nas_sys_info_ind
)
{

  uint8_t lte_srv_status_info_valid;
  nas_service_status_enum_type_v01 lte_srv_status;
  uint8_t tdscdma_srv_status_info_valid;
  nas_service_status_enum_type_v01 tdscdma_srv_status;

  lte_srv_status_info_valid = nas_sys_info_ind->lte_srv_status_info_valid;
  lte_srv_status = nas_sys_info_ind->lte_srv_status_info.srv_status;
  tdscdma_srv_status_info_valid = nas_sys_info_ind->tdscdma_srv_status_info_valid;
  tdscdma_srv_status = nas_sys_info_ind->tdscdma_srv_status_info.srv_status;

  return ((lte_srv_status_info_valid &&
           lte_srv_status == NAS_SYS_SRV_STATUS_SRV_V01) ||
          (tdscdma_srv_status_info_valid &&
           tdscdma_srv_status == NAS_SYS_SRV_STATUS_SRV_V01));

} /* qmi_proxy_sglte_local_is_full_service */

/*===========================================================================
  FUNCTION  qmi_proxy_sglte_local_can_register_ps_srv_domain
===========================================================================*/
/*!
@brief
  Helper function to evaluate if local modem is reporting that it can
  register for PS service when in SGLTE configuration when in a SGLTE PLMN.

@return
   TRUE if PLMN with PS service is available
   FALSE otherwise
*/
/*=========================================================================*/
static boolean qmi_proxy_sglte_local_can_register_ps_srv_domain
(
    nas_sys_info_ind_msg_v01 * nas_sys_info_ind
)
{

  uint8_t lte_reg_domain_valid;
  nas_possible_reg_domain_enum_type_v01 lte_reg_domain;
  uint8_t tdscdma_reg_domain_valid;
  nas_possible_reg_domain_enum_type_v01 tdscdma_reg_domain;

  lte_reg_domain_valid = nas_sys_info_ind->lte_reg_domain_valid;
  lte_reg_domain= nas_sys_info_ind->lte_reg_domain;
  tdscdma_reg_domain_valid = nas_sys_info_ind->tdscdma_reg_domain_valid;
  tdscdma_reg_domain = nas_sys_info_ind->tdscdma_reg_domain;

  return ((tdscdma_reg_domain_valid &&
         (tdscdma_reg_domain == NAS_POSSIBLE_REG_DOMAIN_PS_ONLY_V01 ||
          tdscdma_reg_domain == NAS_POSSIBLE_REG_DOMAIN_CS_PS_V01)) ||
         (lte_reg_domain_valid &&
         (lte_reg_domain == NAS_POSSIBLE_REG_DOMAIN_PS_ONLY_V01 ||
          lte_reg_domain == NAS_POSSIBLE_REG_DOMAIN_CS_PS_V01)));

} /* qmi_proxy_sglte_local_can_register_ps_srv_domain */

/*===========================================================================
  FUNCTION  qmi_proxy_sglte_update_srv_domain_timers
===========================================================================*/
/*!
@brief
  Uses reported service status information to set or clear hysteresis
  timers. Hysteresis timers are used to determine when to move PS service
  domain from local to remove modem when both
    1. user_mode_pref is QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA and
    2. UE is operating in an SGLTE PLMN.

@return
   NONE
*/
/*=========================================================================*/
static void qmi_proxy_sglte_update_srv_domain_timers
(
  qmi_proxy_conn_type proxy_conn_type,
  nas_sys_info_ind_msg_v01 * nas_sys_info_ind
)
{
  boolean is_full_service = FALSE;
  uint8_t lte_srv_status_info_valid;
  nas_service_status_enum_type_v01 lte_srv_status;
  uint8_t tdscdma_srv_status_info_valid;
  nas_service_status_enum_type_v01 tdscdma_srv_status;
  uint8_t srv_reg_restriction_valid;
  nas_srv_reg_restriction_enum_v01 srv_reg_restriction;
  uint8_t lte_reg_domain_valid;
  nas_possible_reg_domain_enum_type_v01 lte_reg_domain;
  uint8_t tdscdma_reg_domain_valid;
  nas_possible_reg_domain_enum_type_v01 tdscdma_reg_domain;

  uint8_t gsm_srv_domain_valid;
  nas_service_domain_enum_type_v01 gsm_srv_domain;
  uint8_t gsm_dtm_supported;
  uint8_t gsm_dtm_supported_valid;

  QMI_PROXY_DEBUG_MSG( "%s\n", "Update srv_domain timers" );

  lte_srv_status_info_valid = nas_sys_info_ind->lte_srv_status_info_valid;
  lte_srv_status = nas_sys_info_ind->lte_srv_status_info.srv_status;
  tdscdma_srv_status_info_valid = nas_sys_info_ind->tdscdma_srv_status_info_valid;
  tdscdma_srv_status = nas_sys_info_ind->tdscdma_srv_status_info.srv_status;
  srv_reg_restriction_valid = nas_sys_info_ind->srv_reg_restriction_valid;
  srv_reg_restriction = nas_sys_info_ind->srv_reg_restriction;
  lte_reg_domain_valid = nas_sys_info_ind->lte_reg_domain_valid;
  lte_reg_domain= nas_sys_info_ind->lte_reg_domain;
  tdscdma_reg_domain_valid = nas_sys_info_ind->tdscdma_reg_domain_valid;
  tdscdma_reg_domain = nas_sys_info_ind->tdscdma_reg_domain;
  gsm_srv_domain_valid = nas_sys_info_ind->gsm_sys_info.common_sys_info.srv_domain_valid;
  gsm_srv_domain = nas_sys_info_ind->gsm_sys_info.common_sys_info.srv_domain;
  gsm_dtm_supported_valid = nas_sys_info_ind->gsm_sys_info.gsm_specific_sys_info.dtm_supp_valid;
  gsm_dtm_supported = nas_sys_info_ind->gsm_sys_info.gsm_specific_sys_info.dtm_supp;

  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
  /* Hyseteresis timers used only with SGLTE mode_pref within
  ** SGLTE PLMN */

  if (gsm_dtm_supported_valid)
  {
    QMI_PROXY_DEBUG_MSG("DTM Supported: %d\n", gsm_dtm_supported);
    qmi_proxy_internal_info.is_dtm_supported[proxy_conn_type] = gsm_dtm_supported;
  }

  if (( qmi_proxy_internal_info.sglte_user_mode_pref
          == QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA) &&
      ( qmi_proxy_internal_info.is_in_sglte_coverage ) )
  {
    QMI_PROXY_DEBUG_MSG( "LTE srv_valid: %d srv_status: %d" \
                         "TDSCDMA srv_valid %d srv_status: %d\n",
                         lte_srv_status_info_valid, lte_srv_status,
                         tdscdma_srv_status_info_valid, tdscdma_srv_status);

    QMI_PROXY_DEBUG_MSG( "reg_restriction_valid: %d reg_restriction: %d\n",
                         srv_reg_restriction_valid, srv_reg_restriction);

    QMI_PROXY_DEBUG_MSG( "LTE reg_domain_valid: %d reg_domain: %d" \
                         "TDSCDMA reg_domain_valid %d reg_domain: %d\n",
                         lte_reg_domain_valid, lte_reg_domain,
                         tdscdma_reg_domain_valid, tdscdma_reg_domain);

    QMI_PROXY_DEBUG_MSG( "Hystersis timer thread id %ld\n",
                         qmi_proxy_sglte_hystersis_timer_info.timer_thread_id);
  }
    if (proxy_conn_type == QMI_PROXY_LOCAL_CONN_TYPE &&
        srv_reg_restriction_valid)
    {
      if (srv_reg_restriction == NAS_SRV_REG_RESTRICTION_CAMPED_ONLY_V01)
      {
        QMI_PROXY_DEBUG_MSG( "%s: Local camped only\n", __FUNCTION__);
        qmi_proxy_sglte_sm_queue_event(
                qmi_proxy_internal_info.sglte_sm,
                SGLTE_SM_EVENT_CAMP_ONLY_ACTIVE_ON_LOCAL,
                "SGLTE_SM_EVENT_CAMP_ONLY_ACTIVE_ON_LOCAL",
                NULL);
      }
      else if (srv_reg_restriction == NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01)
      {
        QMI_PROXY_DEBUG_MSG( "%s: Local unrestricted\n", __FUNCTION__);
        qmi_proxy_sglte_sm_queue_event(
                qmi_proxy_internal_info.sglte_sm,
                SGLTE_SM_EVENT_UNRESTRICTED_ACTIVE_ON_LOCAL,
                "SGLTE_SM_EVENT_UNRESTRICTED_ACTIVE_ON_LOCAL",
                NULL);
      }
    }

    if (proxy_conn_type == QMI_PROXY_REMOTE_CONN_TYPE &&
        gsm_srv_domain_valid)
    {
      if (gsm_srv_domain == SYS_SRV_DOMAIN_CS_PS_V01)
      {
        QMI_PROXY_DEBUG_MSG( "%s: Remote CS/PS\n", __FUNCTION__);
        qmi_proxy_sglte_sm_queue_event(
                qmi_proxy_internal_info.sglte_sm,
                SGLTE_SM_EVENT_PS_ACTIVE_ON_REMOTE,
                "SGLTE_SM_EVENT_PS_ACTIVE_ON_REMOTE",
                NULL);
      }
      else if (gsm_srv_domain == SYS_SRV_DOMAIN_CS_ONLY_V01)
      {
        QMI_PROXY_DEBUG_MSG( "%s: Remote CS\n", __FUNCTION__);
        qmi_proxy_sglte_sm_queue_event(
                qmi_proxy_internal_info.sglte_sm,
                SGLTE_SM_EVENT_PS_INACTIVE_ON_REMOTE,
                "SGLTE_SM_EVENT_PS_INACTIVE_ON_REMOTE",
                NULL);
      }

    }

    /* UE has full service, cancel pending PS->G timer */
    if (proxy_conn_type == QMI_PROXY_LOCAL_CONN_TYPE &&
        qmi_proxy_is_full_service(nas_sys_info_ind) == TRUE)
    {
      QMI_PROXY_DEBUG_MSG( "%s\n", "Cancel PS->G timer" );
      qmi_proxy_internal_info.is_local_svc = TRUE;
      qmi_proxy_sglte_sm_queue_event(
              qmi_proxy_internal_info.sglte_sm,
              SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL,
              "SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL",
              NULL);
    }
    /* UE does not have full service, there are no service registration
    ** restrictions, set PS->G timer if it is not running */
    else if( proxy_conn_type == QMI_PROXY_LOCAL_CONN_TYPE &&
            qmi_proxy_is_full_service(nas_sys_info_ind) == FALSE &&
            (srv_reg_restriction_valid &&
             srv_reg_restriction == NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01))
    {
      QMI_PROXY_DEBUG_MSG("%s: No SVC on Local\n", __FUNCTION__);
      qmi_proxy_internal_info.is_local_svc = FALSE;
      qmi_proxy_sglte_sm_queue_event(
              qmi_proxy_internal_info.sglte_sm,
              SGLTE_SM_EVENT_NO_SVC_ON_LOCAL,
              "SGLTE_SM_EVENT_NO_SVC_ON_LOCAL",
              NULL);
    }
    /* UE does not have full service, UE has service registration
    ** restrictions active, and modem reporting available PLMN:
    ** set PS->LT timer if not already running. */
    else if(proxy_conn_type == QMI_PROXY_LOCAL_CONN_TYPE &&
            qmi_proxy_is_full_service(nas_sys_info_ind) == FALSE &&
            (srv_reg_restriction_valid &&
             srv_reg_restriction != NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01) &&
            (qmi_proxy_sglte_local_can_register_ps_srv_domain(nas_sys_info_ind ))
            )
    {
      QMI_PROXY_DEBUG_MSG("%s: Can register ps on local\n", __FUNCTION__);
      qmi_proxy_internal_info.is_local_svc = TRUE;
      qmi_proxy_sglte_sm_queue_event(
              qmi_proxy_internal_info.sglte_sm,
              SGLTE_SM_EVENT_CAMP_ONLY_SVC_ON_LOCAL,
              "SGLTE_SM_EVENT_CAMP_ONLY_SVC_ON_LOCAL",
              NULL);
    }
    /* UE does not have full service, UE has service registration
    ** restrictions active, and modem does not report available PLMN,
    ** cancel PS->LT timer if it is running. */
    else if(proxy_conn_type == QMI_PROXY_LOCAL_CONN_TYPE &&
            qmi_proxy_is_full_service(nas_sys_info_ind) == FALSE &&
            (srv_reg_restriction_valid &&
             srv_reg_restriction != NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01) &&
            !(qmi_proxy_sglte_local_can_register_ps_srv_domain(nas_sys_info_ind ))
            )
    {
      qmi_proxy_internal_info.is_local_svc = FALSE;
      QMI_PROXY_DEBUG_MSG( "%s: NO SVC on Local\n", __FUNCTION__);
      qmi_proxy_sglte_sm_queue_event(
              qmi_proxy_internal_info.sglte_sm,
              SGLTE_SM_EVENT_NO_SVC_ON_LOCAL,
              "SGLTE_SM_EVENT_NO_SVC_ON_LOCAL",
              NULL);
    }
    if (proxy_conn_type == QMI_PROXY_REMOTE_CONN_TYPE &&
        qmi_proxy_is_full_service(nas_sys_info_ind) == TRUE)
    {
      qmi_proxy_internal_info.is_remote_svc = TRUE;
      qmi_proxy_sglte_sm_queue_event(
              qmi_proxy_internal_info.sglte_sm,
              SGLTE_SM_EVENT_FULL_SVC_ON_REMOTE,
              "SGLTE_SM_EVENT_FULL_SVC_ON_REMOTE",
              NULL);
    }
    else if (proxy_conn_type == QMI_PROXY_REMOTE_CONN_TYPE &&
             qmi_proxy_is_full_service(nas_sys_info_ind) == FALSE)
    {
      qmi_proxy_internal_info.is_remote_svc = FALSE;
      qmi_proxy_sglte_sm_queue_event(
              qmi_proxy_internal_info.sglte_sm,
              SGLTE_SM_EVENT_NO_SVC_ON_REMOTE,
              "SGLTE_SM_EVENT_NO_SVC_ON_REMOTE",
              NULL);
    }
  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

} /* qmi_proxy_sglte_update_srv_domain_timers */

/*===========================================================================
  FUNCTION  qmi_proxy_sglte_update_in_sglte_coverage
===========================================================================*/
/*!
@brief
  Update the is_in_sglte_coverage state variable.

@return
   NONE
*/
/*=========================================================================*/
static void qmi_proxy_sglte_update_in_sglte_coverage
(
  void
)
{
  QMI_PLATFORM_MUTEX_LOCK(&qmi_proxy_internal_info.cache_mutex);
  qmi_proxy_sglte_update_in_sglte_coverage_locked();
  QMI_PLATFORM_MUTEX_UNLOCK(&qmi_proxy_internal_info.cache_mutex);
} /* qmi_proxy_sglte_update_in_sglte_coverage */

/*===========================================================================
  FUNCTION  qmi_proxy_sglte_update_in_sglte_coverage_locked
===========================================================================*/
/*!
@brief
  Update the is_in_sglte_coverage state variable. This must be called in the
  context of cache_mutex

@return
   NONE
*/
/*=========================================================================*/
static void qmi_proxy_sglte_update_in_sglte_coverage_locked
(
  void
)
{
  int have_valid_plmn = FALSE;
  int evt_id;
  const char *name;
  int is_local_plmn_valid =
          qmi_proxy_internal_info.is_sglte_plmn_valid[ QMI_PROXY_LOCAL_CONN_TYPE ];
  int is_remote_plmn_valid =
          qmi_proxy_internal_info.is_sglte_plmn_valid[ QMI_PROXY_REMOTE_CONN_TYPE ];

  have_valid_plmn = (is_local_plmn_valid || is_remote_plmn_valid ) ?
          1 : 0;

  if (qmi_proxy_sglte_is_test_mode )
  {
    qmi_proxy_internal_info.is_in_sglte_coverage = TRUE;
    have_valid_plmn = TRUE;
  }
  else if (have_valid_plmn)
  {
    qmi_proxy_internal_info.is_in_sglte_coverage =
            ( qmi_proxy_internal_info.is_sglte_plmn_valid[ QMI_PROXY_LOCAL_CONN_TYPE ] &&
              qmi_proxy_internal_info.is_sglte_plmn[ QMI_PROXY_LOCAL_CONN_TYPE ] ) ||
            (qmi_proxy_internal_info.is_sglte_plmn_valid[ QMI_PROXY_REMOTE_CONN_TYPE ] &&
              qmi_proxy_internal_info.is_sglte_plmn[ QMI_PROXY_REMOTE_CONN_TYPE ] );
  }
  else
  {
    qmi_proxy_internal_info.is_in_sglte_coverage = -1;
  }

  if (have_valid_plmn)
  {
    if (qmi_proxy_internal_info.is_in_sglte_coverage == 1) {
      if (qmi_proxy_internal_info.sglte_user_mode_pref == QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA)
      {
        if (qmi_proxy_internal_info.number_of_calls &&
            QMI_PROXY_SGLTE_LOCAL_MODEM_CS_ACTIVE())
        {
          qmi_proxy_internal_info.pending_event      = SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE;
          qmi_proxy_internal_info.pending_event_name = "SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE";
        }
        else
        {
          evt_id = SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE;
          name = "SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE";
          qmi_proxy_sglte_sm_queue_event(qmi_proxy_internal_info.sglte_sm, evt_id, name, NULL);
        }
      }
      else
      {
        if (qmi_proxy_internal_info.number_of_calls &&
            QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() &&
            QMI_PROXY_SGLTE_LOCAL_MODEM_PS_ACTIVE())
        {
          qmi_proxy_internal_info.pending_event      = SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE;
          qmi_proxy_internal_info.pending_event_name = "SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE";
        }
        else
        {
          evt_id = SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE;
          name = "SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE";
          qmi_proxy_sglte_sm_queue_event(qmi_proxy_internal_info.sglte_sm, evt_id, name, NULL);
        }
      }
    }
    else if(qmi_proxy_internal_info.is_in_sglte_coverage == 0)
    {
      if (qmi_proxy_internal_info.number_of_calls &&
          QMI_PROXY_SGLTE_REMOTE_MODEM_CS_ACTIVE() &&
          QMI_PROXY_SGLTE_LOCAL_MODEM_PS_ACTIVE())
      {
        qmi_proxy_internal_info.pending_event      = SGLTE_SM_EVENT_NON_SGLTE_PLMN;
        qmi_proxy_internal_info.pending_event_name = "SGLTE_SM_EVENT_NON_SGLTE_PLMN";
      }
      else
      {
        evt_id = SGLTE_SM_EVENT_NON_SGLTE_PLMN;
        name = "SGLTE_SM_EVENT_NON_SGLTE_PLMN";
        qmi_proxy_sglte_sm_queue_event(qmi_proxy_internal_info.sglte_sm, evt_id, name, NULL);
      }
    }
  }

} /* qmi_proxy_sglte_update_in_sglte_coverage_locked */

/*===========================================================================
  FUNCTION  qmi_proxy_sglte_invalidate_last_plmns
===========================================================================*/
/*!
@brief
  Invalidate the last known PLMNs for both modems. This will set the
  is_sglte_plmn cache to false.

@return
   NONE
*/
/*=========================================================================*/
static void qmi_proxy_sglte_invalidate_last_plmns
(
  void
)
{
  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
  qmi_proxy_sglte_invalidate_last_plmns_locked();
  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
} /* qmi_proxy_sglte_invalidate_last_plmns */

/*===========================================================================
  FUNCTION  qmi_proxy_sglte_invalidate_last_plmn
===========================================================================*/
/*!
@brief
  Invalidate the last known PLMN. This will set the is_sglte_plmn cache
  for the modem to false.

@return
   NONE
*/
/*=========================================================================*/
static void qmi_proxy_sglte_invalidate_last_plmn
(
  qmi_proxy_conn_type modem_index
)
{
  QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
  qmi_proxy_sglte_invalidate_last_plmn_locked(modem_index);
  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );
} /* qmi_proxy_sglte_invalidate_last_plmn*/


/*===========================================================================
  FUNCTION  qmi_proxy_sglte_invalidate_last_plmn_locked
===========================================================================*/
/*!
@brief
  Invalidate the last known PLMN. This will set the is_sglte_plmn cache
  for the modem to false.

@return
   NONE
*/
/*=========================================================================*/
static void qmi_proxy_sglte_invalidate_last_plmn_locked
(
  qmi_proxy_conn_type modem_index
)
{
  qmi_proxy_internal_info.is_sglte_plmn_valid[modem_index] = FALSE;

  qmi_proxy_sglte_update_in_sglte_coverage_locked();
} /* qmi_proxy_sglte_invalidate_last_plmn_locked */

/*===========================================================================
  FUNCTION  qmi_proxy_sglte_invalidate_last_plmns_locked
===========================================================================*/
/*!
@brief
  Invalidate the last known PLMN. This will set the is_sglte_plmn cache
  for the modem to false.

@return
   NONE
*/
/*=========================================================================*/
static void qmi_proxy_sglte_invalidate_last_plmns_locked
(
  void
)
{
  qmi_proxy_internal_info.is_sglte_plmn_valid[QMI_PROXY_LOCAL_CONN_TYPE] = FALSE;
  qmi_proxy_internal_info.is_sglte_plmn_valid[QMI_PROXY_REMOTE_CONN_TYPE] = FALSE;

  qmi_proxy_sglte_update_in_sglte_coverage_locked();
} /* qmi_proxy_sglte_invalidate_last_plmn_locked */

/*===========================================================================
  FUNCTION  qmi_proxy_is_no_service
===========================================================================*/
/*!
@brief
  Find out whether sys info informs modem is in no service

@return
    TRUE  if modem is in no service on any technology
    FALSE if either modem is in service or if no service information
            is available
*/
/*=========================================================================*/
int qmi_proxy_is_no_service
(
  nas_sys_info_ind_msg_v01 *nas_sys_info_ind
)
{
  boolean ret = FALSE;

  if (nas_sys_info_ind->gsm_srv_status_info_valid ||
        nas_sys_info_ind->wcdma_srv_status_info_valid ||
        nas_sys_info_ind->tdscdma_srv_status_info_valid ||
        nas_sys_info_ind->lte_srv_status_info_valid)
  {
    ret = TRUE;
    if (nas_sys_info_ind->gsm_srv_status_info_valid &&
          (NAS_SYS_SRV_STATUS_NO_SRV_V01 !=
                   nas_sys_info_ind->gsm_srv_status_info.srv_status) &&
          (NAS_SYS_SRV_STATUS_PWR_SAVE_V01 !=
                   nas_sys_info_ind->gsm_srv_status_info.srv_status))
    {
      ret = FALSE;
    }

    if (nas_sys_info_ind->wcdma_srv_status_info_valid &&
          (NAS_SYS_SRV_STATUS_NO_SRV_V01 !=
                   nas_sys_info_ind->wcdma_srv_status_info.srv_status) &&
          (NAS_SYS_SRV_STATUS_PWR_SAVE_V01 !=
                   nas_sys_info_ind->wcdma_srv_status_info.srv_status))
    {
      ret = FALSE;
    }

    if (nas_sys_info_ind->tdscdma_srv_status_info_valid &&
          (NAS_SYS_SRV_STATUS_NO_SRV_V01 !=
                   nas_sys_info_ind->tdscdma_srv_status_info.srv_status) &&
          (NAS_SYS_SRV_STATUS_PWR_SAVE_V01 !=
                   nas_sys_info_ind->tdscdma_srv_status_info.srv_status))
    {
      ret = FALSE;
    }

    if (nas_sys_info_ind->lte_srv_status_info_valid &&
          (NAS_SYS_SRV_STATUS_NO_SRV_V01 !=
                   nas_sys_info_ind->lte_srv_status_info.srv_status) &&
          (NAS_SYS_SRV_STATUS_PWR_SAVE_V01 !=
                   nas_sys_info_ind->lte_srv_status_info.srv_status))
    {
      ret = FALSE;
    }
  }

  return ret;
} /* qmi_proxy_is_no_service */

/*===========================================================================
  FUNCTION  qmi_proxy_is_full_service
===========================================================================*/
/*!
@brief
  Find out whether sys info informs modem is in service

@return
   TRUE if modem is in service on any technology
   FALSE if modem is not in service
   -1 if message doesn't contain service state TLVs
*/
/*=========================================================================*/
int qmi_proxy_is_full_service
(
  nas_sys_info_ind_msg_v01 *nas_sys_info_ind
)
{
  uint8_t cdma_ss_valid, hdr_ss_valid;
  uint8_t gsm_ss_valid, wcdma_ss_valid, lte_ss_valid, tds_ss_valid;
  nas_3gpp2_srv_status_info_type_v01 *cdma_ss, *hdr_ss;
  nas_3gpp_srv_status_info_type_v01 *gsm_ss, *wcdma_ss, *lte_ss, *tds_ss;
  int ret = 0;

  cdma_ss_valid = nas_sys_info_ind->cdma_srv_status_info_valid;
  cdma_ss = &nas_sys_info_ind->cdma_srv_status_info;

  hdr_ss_valid = nas_sys_info_ind->hdr_srv_status_info_valid;
  hdr_ss = &nas_sys_info_ind->hdr_srv_status_info;

  gsm_ss_valid = nas_sys_info_ind->gsm_srv_status_info_valid;
  gsm_ss = &nas_sys_info_ind->gsm_srv_status_info;

  wcdma_ss_valid = nas_sys_info_ind->wcdma_srv_status_info_valid;
  wcdma_ss = &nas_sys_info_ind->wcdma_srv_status_info;

  lte_ss_valid = nas_sys_info_ind->lte_srv_status_info_valid;
  lte_ss = &nas_sys_info_ind->lte_srv_status_info;

  tds_ss_valid = nas_sys_info_ind->tdscdma_srv_status_info_valid;
  tds_ss = &nas_sys_info_ind->tdscdma_srv_status_info;

  if ( cdma_ss_valid  )
  {
    ret |= cdma_ss->srv_status == NAS_SYS_SRV_STATUS_SRV_V01;
  }

  if ( hdr_ss_valid )
  {
    ret |= hdr_ss->srv_status == NAS_SYS_SRV_STATUS_SRV_V01;
  }

  if ( gsm_ss_valid )
  {
    ret |= gsm_ss->srv_status == NAS_SYS_SRV_STATUS_SRV_V01;
  }

  if ( gsm_ss_valid )
  {
    ret |= gsm_ss->srv_status == NAS_SYS_SRV_STATUS_SRV_V01;
  }

  if ( wcdma_ss_valid )
  {
    ret |= wcdma_ss->srv_status == NAS_SYS_SRV_STATUS_SRV_V01;
  }

  if ( lte_ss_valid )
  {
    ret |= lte_ss->srv_status == NAS_SYS_SRV_STATUS_SRV_V01;
  }

  if ( tds_ss_valid )
  {
    ret |= tds_ss->srv_status == NAS_SYS_SRV_STATUS_SRV_V01;
  }

  return ret;
} /* qmi_proxy_is_full_service */

/*===========================================================================
  FUNCTION  qmi_proxy_sglte_update_last_known_plmn
===========================================================================*/
/*!
@brief
  Updates internal cache of last known SGLTE PLMN

@return
   NONE
*/
/*=========================================================================*/
void qmi_proxy_sglte_update_last_known_plmn
(
  nas_sys_info_ind_msg_v01 * nas_sys_info_ind,
  qmi_proxy_conn_type modem_index
)
{
  nas_3gpp_only_sys_info_type_v01 * threegpp_only_sys_info = NULL;
  int is_sglte_plmn;

  QMI_PROXY_DEBUG_MSG( "%s\n", "Update last known PLMN " );

  if (nas_sys_info_ind->wcdma_sys_info_valid &&
      nas_sys_info_ind->wcdma_sys_info.threegpp_specific_sys_info.network_id_valid)
  {
     threegpp_only_sys_info = &(nas_sys_info_ind->wcdma_sys_info.threegpp_specific_sys_info);
     QMI_PROXY_DEBUG_MSG( "%s\n", "WCDMA sys_info valid " );

  }

  if (nas_sys_info_ind->tdscdma_sys_info_valid &&
      nas_sys_info_ind->tdscdma_sys_info.threegpp_specific_sys_info.network_id_valid)
  {
     threegpp_only_sys_info = &(nas_sys_info_ind->tdscdma_sys_info.threegpp_specific_sys_info);
     QMI_PROXY_DEBUG_MSG( "%s\n", "TD-SCDMA sys_info valid " );
  }

  if (nas_sys_info_ind->lte_sys_info_valid &&
      nas_sys_info_ind->lte_sys_info.threegpp_specific_sys_info.network_id_valid)
  {
     threegpp_only_sys_info = &(nas_sys_info_ind->lte_sys_info.threegpp_specific_sys_info);
     QMI_PROXY_DEBUG_MSG( "%s\n", "LTE sys_info valid " );

  }

  if (nas_sys_info_ind->gsm_sys_info_valid &&
      nas_sys_info_ind->gsm_sys_info.threegpp_specific_sys_info.network_id_valid)
  {
     threegpp_only_sys_info = &(nas_sys_info_ind->gsm_sys_info.threegpp_specific_sys_info);
     QMI_PROXY_DEBUG_MSG( "%s\n", "GSM sys_info valid " );

  }

  if( threegpp_only_sys_info )
  {

    QMI_PLATFORM_MUTEX_LOCK( &qmi_proxy_internal_info.cache_mutex );
    memset(qmi_proxy_internal_info.last_sglte_plmn[modem_index].mcc,
           0,
           QMI_PROXY_MCC_LENGTH);
    memset(qmi_proxy_internal_info.last_sglte_plmn[modem_index].mnc,
           0,
           QMI_PROXY_MNC_LENGTH);
    memcpy(qmi_proxy_internal_info.last_sglte_plmn[modem_index].mcc,
           &(threegpp_only_sys_info->network_id.mcc),
           QMI_PROXY_MCC_LENGTH-1); /* length-1 for NULL terminator */
    memcpy(qmi_proxy_internal_info.last_sglte_plmn[modem_index].mnc,
           &(threegpp_only_sys_info->network_id.mnc),
           QMI_PROXY_MNC_LENGTH-1); /* length-1 for NULL terminator */

    is_sglte_plmn = qmi_proxy_is_sglte_plmn(&qmi_proxy_internal_info.last_sglte_plmn[modem_index]);
    qmi_proxy_internal_info.is_sglte_plmn[modem_index] = is_sglte_plmn;
    qmi_proxy_internal_info.is_sglte_plmn_valid[modem_index] = TRUE;
    qmi_proxy_sglte_update_in_sglte_coverage_locked();
    QMI_PLATFORM_MUTEX_UNLOCK( &qmi_proxy_internal_info.cache_mutex );

    QMI_PROXY_DEBUG_MSG( "Updated SGLTE MCC: %s MNC: %s modem: %d\n",
                         qmi_proxy_internal_info.last_sglte_plmn[modem_index].mcc,
                         qmi_proxy_internal_info.last_sglte_plmn[modem_index].mnc,
                         modem_index);
    QMI_PROXY_DEBUG_MSG( "Is in SGLTE coverage: %d\n",
                         qmi_proxy_internal_info.is_in_sglte_coverage);
  }
  else if (qmi_proxy_is_no_service(nas_sys_info_ind))
  {
    qmi_proxy_sglte_invalidate_last_plmn(modem_index);
  }
  else
  {
    QMI_PROXY_DEBUG_MSG( "%s\n", "No valid PLMN present ", __FUNCTION__);
  }
} /* qmi_proxy_sglte_update_last_known_plmn */

/*===========================================================================
FUNCTION qmi_proxy_sglte_ps_to_g_timer_exp_handler
===========================================================================*/
/*!
@brief
Called when SGLTE PS->G hysteresis timer expires.

@return
NONE
*/
/*=========================================================================*/
void qmi_proxy_sglte_ps_to_g_timer_exp_handler
(
  void
)
{

  nas_set_system_selection_preference_req_msg_v01 *local_nas_set_sys_sel_pref_req;
  nas_set_system_selection_preference_req_msg_v01 *remote_nas_set_sys_sel_pref_req;
  nas_set_system_selection_preference_resp_msg_v01 *local_nas_set_sys_sel_pref_resp;
  nas_set_system_selection_preference_resp_msg_v01 *remote_nas_set_sys_sel_pref_resp;
  int rc;

  QMI_PROXY_DEBUG_MSG( "%s\n", "PS->G timer expired" );

  /* Allocate buffers for requests and responses */
  local_nas_set_sys_sel_pref_req =
    qmi_proxy_malloc(sizeof(*local_nas_set_sys_sel_pref_req));
  remote_nas_set_sys_sel_pref_req =
    qmi_proxy_malloc(sizeof(*remote_nas_set_sys_sel_pref_req));
  local_nas_set_sys_sel_pref_resp =
    qmi_proxy_malloc(sizeof(*local_nas_set_sys_sel_pref_resp));
  remote_nas_set_sys_sel_pref_resp =
    qmi_proxy_malloc(sizeof(*remote_nas_set_sys_sel_pref_resp));

  if ( local_nas_set_sys_sel_pref_req && remote_nas_set_sys_sel_pref_req &&
          local_nas_set_sys_sel_pref_resp && remote_nas_set_sys_sel_pref_resp )
  {

    local_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = TRUE;
    local_nas_set_sys_sel_pref_req->srv_reg_restriction =
      NAS_SRV_REG_RESTRICTION_CAMPED_ONLY_V01;
    local_nas_set_sys_sel_pref_req->srv_domain_pref_valid = FALSE;
    local_nas_set_sys_sel_pref_req->srv_domain_pref =
      QMI_SRV_DOMAIN_PREF_PS_DETACH_NO_PREF_CHANGE_V01;

    remote_nas_set_sys_sel_pref_req->srv_domain_pref_valid = TRUE;
    remote_nas_set_sys_sel_pref_req->srv_domain_pref =
      QMI_SRV_DOMAIN_PREF_CS_PS_V01;

    rc = qmi_proxy_force_sys_sel_pref( QMI_PROXY_LOCAL_CONN_TYPE,
                                       local_nas_set_sys_sel_pref_req,
                                       local_nas_set_sys_sel_pref_resp);
    if (rc != QMI_NO_ERR)
    {
      QMI_PROXY_ERR_MSG("%s/n", "Unable to send sys_sel_pref request to local modem");
      qmi_proxy_free((void **)&local_nas_set_sys_sel_pref_resp);
    }

    rc = qmi_proxy_force_sys_sel_pref( QMI_PROXY_REMOTE_CONN_TYPE,
                                       remote_nas_set_sys_sel_pref_req,
                                       remote_nas_set_sys_sel_pref_resp);
    if (rc != QMI_NO_ERR)
    {
      QMI_PROXY_ERR_MSG("%s\n", "Unable to send sys_sel_pref_request to remote modem");
      qmi_proxy_free((void **)&remote_nas_set_sys_sel_pref_resp);
    }

    qmi_proxy_free((void **)&local_nas_set_sys_sel_pref_req);
    qmi_proxy_free((void **)&remote_nas_set_sys_sel_pref_req);
  }
  else
  {
    qmi_proxy_free((void **)&local_nas_set_sys_sel_pref_req);
    qmi_proxy_free((void **)&remote_nas_set_sys_sel_pref_req);
    qmi_proxy_free((void **)&local_nas_set_sys_sel_pref_resp);
    qmi_proxy_free((void **)&remote_nas_set_sys_sel_pref_resp);
    QMI_PROXY_ERR_MSG("%s: allocation failed\n", __FUNCTION__);
  }

} /* qmi_proxy_sglte_ps_to_g_timer_exp_handler */

/*===========================================================================
FUNCTION qmi_proxy_sglte_ps_to_LT_timer_exp_handler
===========================================================================*/
/*!
@brief
Called when SGLTE PS->LT hysteresis timer expires.

@return
NONE
*/
/*=========================================================================*/
void qmi_proxy_sglte_ps_to_lt_timer_exp_handler
(
  void
)
{

  nas_set_system_selection_preference_req_msg_v01 *local_nas_set_sys_sel_pref_req;
  nas_set_system_selection_preference_req_msg_v01 *remote_nas_set_sys_sel_pref_req;
  nas_set_system_selection_preference_resp_msg_v01 *local_nas_set_sys_sel_pref_resp;
  nas_set_system_selection_preference_resp_msg_v01 *remote_nas_set_sys_sel_pref_resp;
  int rc;

  QMI_PROXY_DEBUG_MSG( "%s\n", "PS->LT timer expired" );

  /* Allocate buffers for requests and responses */
  local_nas_set_sys_sel_pref_req =
    qmi_proxy_malloc(sizeof(*local_nas_set_sys_sel_pref_req));
  remote_nas_set_sys_sel_pref_req =
    qmi_proxy_malloc(sizeof(*remote_nas_set_sys_sel_pref_req));
  local_nas_set_sys_sel_pref_resp =
    qmi_proxy_malloc(sizeof(*local_nas_set_sys_sel_pref_resp));
  remote_nas_set_sys_sel_pref_resp =
    qmi_proxy_malloc(sizeof(*remote_nas_set_sys_sel_pref_resp));

  if ( local_nas_set_sys_sel_pref_req && remote_nas_set_sys_sel_pref_req &&
          local_nas_set_sys_sel_pref_resp && remote_nas_set_sys_sel_pref_resp )
  {
    remote_nas_set_sys_sel_pref_req->srv_domain_pref_valid = TRUE;
    remote_nas_set_sys_sel_pref_req->srv_domain_pref =
      QMI_SRV_DOMAIN_PREF_CS_ONLY_V01;

    local_nas_set_sys_sel_pref_req->srv_reg_restriction_valid = TRUE;
    local_nas_set_sys_sel_pref_req->srv_reg_restriction =
      NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01;
    local_nas_set_sys_sel_pref_req->srv_domain_pref_valid = TRUE;
    local_nas_set_sys_sel_pref_req->srv_domain_pref =
      QMI_SRV_DOMAIN_PREF_PS_ONLY_V01;

    rc = qmi_proxy_force_sys_sel_pref( QMI_PROXY_REMOTE_CONN_TYPE,
                                       remote_nas_set_sys_sel_pref_req,
                                       remote_nas_set_sys_sel_pref_resp);
    if (rc != QMI_NO_ERR)
    {
      QMI_PROXY_ERR_MSG("%s\n", "Unable to send sys_sel_pref_request to remote modem");
      qmi_proxy_free((void **)&remote_nas_set_sys_sel_pref_resp);
    }

    rc = qmi_proxy_force_sys_sel_pref( QMI_PROXY_LOCAL_CONN_TYPE,
                                       local_nas_set_sys_sel_pref_req,
                                       local_nas_set_sys_sel_pref_resp);
    if (rc != QMI_NO_ERR)
    {
      QMI_PROXY_ERR_MSG("%s/n", "Unable to send sys_sel_pref request to local modem");
      qmi_proxy_free((void **)&local_nas_set_sys_sel_pref_resp);
    }

    qmi_proxy_free((void **)&local_nas_set_sys_sel_pref_req);
    qmi_proxy_free((void **)&remote_nas_set_sys_sel_pref_req);
  }
  else
  {
    qmi_proxy_free((void **)&local_nas_set_sys_sel_pref_req);
    qmi_proxy_free((void **)&remote_nas_set_sys_sel_pref_req);
    qmi_proxy_free((void **)&local_nas_set_sys_sel_pref_resp);
    qmi_proxy_free((void **)&remote_nas_set_sys_sel_pref_resp);
    QMI_PROXY_ERR_MSG("%s: allocation failed\n", __FUNCTION__);
  }

} /* qmi_proxy_sglte_ps_to_LT_timer_exp_handler */

/*===========================================================================
FUNCTION qmi_proxy_create_timer
===========================================================================*/
/*!
@brief
Create timer for hystersis

@return
NONE
*/
/*=========================================================================*/
static void qmi_proxy_create_timer
(
  int timer_in_seconds,
  timer_expiry_handler_type timer_expiry_handler
)
{
  QMI_PLATFORM_MUTEX_LOCK(&qmi_proxy_timer_mutex);
  signal(SIGUSR1, qcril_proxy_signal_handler_sigusr1);
  if ( !qmi_proxy_sglte_hystersis_timer_info.timer_thread_id )
  {
    memset(&qmi_proxy_sglte_hystersis_helper_info, 0,
           sizeof(qmi_proxy_sglte_hystersis_helper_info));
    qmi_proxy_sglte_hystersis_helper_info.timer_expiry_handler = timer_expiry_handler;
    qmi_proxy_sglte_hystersis_helper_info.hystersis_timer.tv_sec = timer_in_seconds;
    qmi_proxy_sglte_hystersis_helper_info.hystersis_timer.tv_usec = 0;

    pthread_create(&qmi_proxy_sglte_hystersis_timer_info.timer_thread_id,
                   NULL, qmi_proxy_timer_thread_proc, NULL);
    QMI_PLATFORM_INIT_SIGNAL_FOR_WAIT(0, &qmi_proxy_sglte_hystersis_timer_info.signal);
    QMI_PLATFORM_WAIT_FOR_SIGNAL(0, &qmi_proxy_sglte_hystersis_timer_info.signal, 10000);
  }
  else
  {
    QMI_PROXY_DEBUG_MSG("%s: A timer is already running.\n", __FUNCTION__);
  }
  QMI_PLATFORM_MUTEX_UNLOCK(&qmi_proxy_timer_mutex);
}

/*===========================================================================
FUNCTION qmi_proxy_timer_thread_proc
===========================================================================*/
/*!
@brief
timer thread execution

@return
NONE
*/
/*=========================================================================*/
static void * qmi_proxy_timer_thread_proc
(
  void * param
)
{
  int res = 0;
  int canceled = 0;
  struct timeval hysteresis_timer;
  timer_expiry_handler_type cb = NULL;

  /* Supress compiler warnings for unused variables */
  (void) param;

  QMI_PLATFORM_SEND_SIGNAL(0, &qmi_proxy_sglte_hystersis_timer_info.signal);

  QMI_PLATFORM_MUTEX_LOCK(&qmi_proxy_timer_mutex);
  QMI_PROXY_DEBUG_MSG( "Hystersis timer created with duration of %d seconds",
                       qmi_proxy_sglte_hystersis_helper_info.hystersis_timer.tv_sec);
  hysteresis_timer = qmi_proxy_sglte_hystersis_helper_info.hystersis_timer;
  if ( !pthread_equal(qmi_proxy_sglte_hystersis_timer_info.timer_thread_id,
                      pthread_self()) )
  {
    /* Our timer was cancelled before we got to the select */
    canceled = TRUE;
  }
  QMI_PLATFORM_MUTEX_UNLOCK(&qmi_proxy_timer_mutex);

  if ( !canceled )
  {
    res = select(0, NULL, NULL, NULL,
               &hysteresis_timer);
    canceled = !(FALSE == res);
  }

  if( !canceled )
  {
    QMI_PLATFORM_MUTEX_LOCK(&qmi_proxy_timer_mutex);
    if (pthread_equal(qmi_proxy_sglte_hystersis_timer_info.timer_thread_id,
                      pthread_self()) )
    {
      cb = qmi_proxy_sglte_hystersis_helper_info.timer_expiry_handler;
    }
    else
    {
      canceled = TRUE;
    }
    QMI_PLATFORM_MUTEX_UNLOCK(&qmi_proxy_timer_mutex);
    if (cb)
    {
      QMI_PROXY_DEBUG_MSG( "%s\n", "Hystersis timer expired," \
                                   "calling timer callback function" );
      (cb)();
    }
  }

  if (canceled)
  {
    QMI_PROXY_DEBUG_MSG( "%s\n", "Hystersis timer cancelled" );
  }

  QMI_PLATFORM_MUTEX_LOCK(&qmi_proxy_timer_mutex);
  if (pthread_equal(qmi_proxy_sglte_hystersis_timer_info.timer_thread_id,
                    pthread_self()) )
  {
    qmi_proxy_sglte_hystersis_timer_info.timer_thread_id = 0;
    qmi_proxy_sglte_hystersis_helper_info.timer_expiry_handler = NULL;
  }
  QMI_PLATFORM_MUTEX_UNLOCK(&qmi_proxy_timer_mutex);

  return NULL;
}

/*===========================================================================
FUNCTION qmi_proxy_cancel_timer
===========================================================================*/
/*!
@brief
Cancel timer created for hystersis

@return
NONE
*/
/*=========================================================================*/
static void qmi_proxy_cancel_timer
(
  void
)
{
  QMI_PLATFORM_MUTEX_LOCK(&qmi_proxy_timer_mutex);
  //thread id can be either positive or negative
  if( qmi_proxy_sglte_hystersis_timer_info.timer_thread_id !=  0 )
  {
    QMI_PROXY_DEBUG_MSG( "%s\n", "Cancelling Hystersis timer" );
    pthread_kill(qmi_proxy_sglte_hystersis_timer_info.timer_thread_id,
                 SIGUSR1);
    qmi_proxy_sglte_hystersis_timer_info.timer_thread_id = 0;
  }
  QMI_PLATFORM_MUTEX_UNLOCK(&qmi_proxy_timer_mutex);
}

/*===========================================================================
FUNCTION qcril_proxy_signal_handler_sigusr1
===========================================================================*/
/*!
@brief
catch SIGUSR1

@return
NONE
*/
/*=========================================================================*/
static void qcril_proxy_signal_handler_sigusr1
(
  int arg
)
{
  (void) arg;
  return;
}


/*===========================================================================

FUNCTION qmi_proxy_rx_hdlr

===========================================================================*/

/*!
@brief
 indication message handler

@return
NONE
*/

/*=========================================================================*/
static void * qmi_proxy_rx_hdlr
(
  void * param
)
{
  qmi_proxy_rx_message *rx_msg;
  int rc;

  (void) param;

  for(;;)
  {
    QMI_PLATFORM_INIT_SIGNAL_FOR_WAIT(0, &qmi_proxy_rx_msg_queue_signal);
    if (qmi_proxy_queue_is_empty(qmi_proxy_rx_msg_queue)) {
      rc = QMI_PLATFORM_WAIT_FOR_SIGNAL_NO_UNLOCK(0, &qmi_proxy_rx_msg_queue_signal, 0x7fffffff);
      if (rc == QMI_TIMEOUT_ERR)
      {
        QMI_PROXY_DEBUG_MSG("%s: Timeout in receiving events. Retrying\n", __FUNCTION__);
        QMI_PLATFORM_FINISH_SIGNAL_WAIT(&qmi_proxy_rx_msg_queue_signal);
        continue;
      }
    }

    QMI_PROXY_DEBUG_MSG("%s: Dequeuing message\n", __FUNCTION__);
    rx_msg = qmi_proxy_queue_pop(qmi_proxy_rx_msg_queue);
    QMI_PLATFORM_FINISH_SIGNAL_WAIT(&qmi_proxy_rx_msg_queue_signal);
    if (rx_msg)
    {
      if (qmi_proxy_srvc_unsol_ind_cb_tbl[ rx_msg->proxy_srvc_id ])
      {
        qmi_proxy_srvc_unsol_ind_cb_tbl[ rx_msg->proxy_srvc_id ](rx_msg);
      }
      else
      {
        QMI_PROXY_ERR_MSG("%s: Received unregistered service indication", __FUNCTION__);
      }
      free(rx_msg);
    }
    else
    {
      QMI_PROXY_ERR_MSG("%s: Dequeued null message", __FUNCTION__);
    }
  }

  QMI_PROXY_DEBUG_MSG("%s: Thread ending\n", __FUNCTION__);
  return NULL;
}

/*===========================================================================
  FUNCTION  main
===========================================================================*/
/*!
@brief
  Function to create QMUX and TETHER socket,
  process the messages received on the sockets and response to them appropriately

@return
   NONE
*/
/*=========================================================================*/
int main(int argc, char **argv)
{
  int i;
  pthread_attr_t qmux_thread_attr;
  pthread_attr_t rx_thread_attr;

  pthread_attr_init( &qmux_thread_attr );
  pthread_attr_setdetachstate( &qmux_thread_attr, PTHREAD_CREATE_DETACHED );
  pthread_t tid;

  (void) argc;
  (void) argv;

  qmi_proxy_init();

  if ( pthread_create( &tid, &qmux_thread_attr, qmi_proxy_create_qmux_socket, (void *) NULL ) != 0 )
  {
      QMI_PROXY_ERR_MSG( "%s\n", "Fail to spawn thread to handle QMUX Sockett" );
  }
  pthread_attr_destroy( &qmux_thread_attr );

  qmi_proxy_rx_msg_queue = qmi_proxy_queue_new();
  if (qmi_proxy_rx_msg_queue)
  {
    pthread_attr_init( &rx_thread_attr);
    pthread_attr_setdetachstate( &rx_thread_attr, PTHREAD_CREATE_DETACHED );
    QMI_PLATFORM_INIT_SIGNAL_FOR_SEND(0, &qmi_proxy_rx_msg_queue_signal);
    if ( pthread_create( &tid, &rx_thread_attr, qmi_proxy_rx_hdlr, NULL ) != 0 )
    {
      QMI_PROXY_ERR_MSG( "%s\n",
                "Fail to spawn thread to handle unsolicited indication messages" );
    }
    pthread_attr_destroy( &rx_thread_attr );

    QMI_PLATFORM_FINISH_SIGNAL_SEND(&qmi_proxy_rx_msg_queue_signal);
  }
  else
  {
    QMI_PROXY_ERR_MSG("%s: Unable to allocate memory for queue\n", __FUNCTION__);
  }

  qmi_proxy_create_tether_socket();

  return 1;
} /* main */
