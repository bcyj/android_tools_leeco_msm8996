/******************************************************************************

                        QMI_IP.H

******************************************************************************/

/******************************************************************************

  @file    qmi_ip.C
  @brief   Qualcomm mapping interface over IP HEADER

  DESCRIPTION


  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
08/30/13   tw         Initial version

******************************************************************************/
/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#ifndef __QMI_IP_H__
#define __QMI_IP_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>

#include <openssl/rsa.h>       /* SSLeay stuff */
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "qmi_ip_netlink.h"
#include "qmi_ip_cmdq.h"
#include "qmi_client.h"
#include "network_access_service_v01.h"
#include "wireless_data_service_v01.h"
#include "qualcomm_mobile_access_point_msgr_v01.h"
#include "qmi_client_instance_defs.h"
#include "qmi_wds_srvc.h"
#include "qmi_nas_srvc.h"
#include "dsi_netctrl.h"
#include "ds_util.h"
#include "common_v01.h"

/*===========================================================================
                              VARIABLE DECLARATIONS
===========================================================================*/
#define COUNTRY_CODE    "US"
#define ORG_NAME        "Qualcomm"
#define COMMON_NAME     "Qualcomm"

#define DEFAULT_PSWD "PASSWORD"
#define PASSWORD_FILE "/etc/odu.user"
#define MAX_PSWD_SIZE 16  //Max password length + 1

#define LINK_UP_STR '1'
#define LINK_DOWN_STR '0'

#define MAX_BUF_LEN 256
#define IP_FAMILY_4 "IP"
#define IP_FAMILY_6 "IPV4"
#define IP_FAMILY_4_6 "IPV4V6"

#define XML_PATH   "/etc/IPACM_cfg.xml"
#define QMI_IP_STA_FILE "/etc/qmi_ip_sta"
#define ODU_NET_CARRIER "/sys/class/net/odu0/carrier"
/* Make these what you want for cert & key files */

#define QMI_TIMEOUT 90000
#define MAX_QMI_EMB_INSTANCES 7
#define SSL_ACCEPT_DELAY 10

#define BRIDGE_MODE 0
#define ROUTER_MODE 1

#define QMI_IP_SUCCESS 0
#define QMI_IP_ERROR   -1

#define ODU_INTERFACE 2

/* Used for eMBMS calls state */
#define UP        1
#define DOWN      0

#define  DSI_NET_IS_CONN    1
#define  DSI_NET_NO_NET     0

/* Header Defines */
#define QMI_TLV_HDR_SIZE         (3)
#define QMI_HEADER_SIZE          13
#define QMI_ERROR_SIZE            7
#define QMI_ERROR_TLV             4

/* Total message size numbers */
#define QMI_MAX_STD_MSG_SIZE         (512)
#define QMI_MAX_MSG_SIZE             (2048)

/* Service Type */
#define QMI_IP_ODU_SERVICE_TYPE             0x00
#define QMI_WDS_SERVICE_TYPE                0x01
#define QMI_NAS_SERVICE_TYPE                0x03

/* WDS Message ID's */
#define QMI_WDS_START_NETWORK_INTERFACE     0x0020
#define QMI_WDS_GET_PKT_SRVC_STATUS_IND     0x0022
#define QMI_WDS_EMBMS_TMGI_ACTIVATE         0x0065
#define QMI_WDS_EMBMS_TMGI_DEACTIVATE       0x0066
#define QMI_WDS_EMBMS_TMGI_LIST_QUERY       0x0067
#define QMI_WDS_EMBMS_TMGI_LIST_IND         0x0068
#define QMI_WDS_EMBMS_TMGI_ACT_DEACT        0x0088

//WDS TLV ID's
#define QMI_WDS_EMBMS_TECH                                -30590
#define QMI_WDS_NW_IF_XTENDED_TECH_PREF_REQ_TLV_ID         0x34


/* NAS Messages ID's */
#define QMI_NAS_INDICATION_REGISTER_REQ     0x0003
#define QMI_NAS_GET_SYS_INFO                0x004D
#define QMI_NAS_CONFIG_EMBMS                0x0062
#define QMI_NAS_GET_EMBMS_STATUS            0x0063
#define QMI_NAS_GET_EMBMS_SIG               0x006F
#define QMI_NAS_GET_EMBMS_SIG_EXT           0x0081

/* ODU Message ID's */
#define QMI_IP_ODU_SET_MODE                 0x005A
#define QMI_IP_ODU_GET_MODE                 0x005B
#define QMI_IP_ODU_SET_PSWD                 0x005C
#define QMI_IP_ODU_PSWD_RESET               0x005D

#define QMI_IP_ODU_MODE_TLV                 0x10

//ODU TLV ID's
#define QMI_IP_ODU_MODE_TLV_ID              0x10
#define QMI_IP_ODU_PSWD_TLV_ID              0x10

#define QMI_IP_RESULT_CODE                  0x02

#define QMI_IND_TX_ID                       0x0000
#define QMI_EMPTY                           0x00
#define QMI_REQ_CTRL_FLAGS                  0x00
#define QMI_RESP_CTRL_FLAGS                 0x01
#define QMI_IND_CTRL_FLAGS                  0x11

/* EMBMS indication registration TLVs */
#define QMI_WDS_REPORT_EMBMS_TMGI_LIST_TLV_ID    0x10

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

/* EMBMS TMGI Activate */
#define QMI_WDS_EMBMS_ACTIVATE_REQ_TMGI_TLV_ID          0x01
#define QMI_WDS_EMBMS_ACTIVATE_REQ_TRANX_ID_TLV_ID      0x10
#define QMI_WDS_EMBMS_ACTIVATE_REQ_PREEMPT_PRI_TLV_ID   0x11
#define QMI_WDS_EMBMS_ACTIVATE_REQ_EARFCNLIST_TLV_ID    0x12

/* EMBMS TMGI Deactivate */
#define QMI_WDS_EMBMS_DEACTIVATE_REQ_TLV_ID             0x01
#define QMI_WDS_EMBMS_DEACTIVATE_REQ_TRANX_ID_TLV_ID    0x10

/* EMBMS TMGI Activate-Deactivate */
#define QMI_WDS_EMBMS_ACT_DEACT_REQ_ACTIVATE_TMGI_TLV_ID   0x01
#define QMI_WDS_EMBMS_ACT_DEACT_REQ_DEACTIVATE_TMGI_TLV_ID 0x02
#define QMI_WDS_EMBMS_ACT_DEACT_REQ_TRANX_ID_TLV_ID        0x10
#define QMI_WDS_EMBMS_ACT_DEACT_REQ_PREEMPT_PRI_TLV_ID     0x11
#define QMI_WDS_EMBMS_ACT_DEACT_REQ_EARFCNLIST_TLV_ID      0x12

/* EMBMS TMGI List Query request */
#define QMI_WDS_EMBMS_LIST_QUERY_REQ_LIST_TYPE_TLV_ID       0x01
#define QMI_WDS_EMBMS_LIST_QUERY_REQ_TRANX_ID_TLV_ID        0x10

/* EMBMS TMGI List Query response */
#define QMI_WDS_EMBMS_LIST_QUERY_RESP_TMGI_LIST_TLV_ID      0x10

#define CHK_ERR(err,s) if ((err)==-1) { perror(s); }
#define CHK_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); }

/* Utility macros for reading/writing various quantities from buffers.  Note
** that buffer pointers are changed by calling these.  Also note that these
** will only work for little-endian processors.
*/
#define SIZE_8_BIT_VAL   (1)
#define SIZE_16_BIT_VAL  (2)
#define SIZE_32_BIT_VAL  (4)

/* Write the QMI header.  Length value(2nd) does not include the I/F value */
#define WRITE_QMI_HEADER(buf, len, serv, msg_id, ctrl_flags, tx_id, msg_len) \
do { \
	 WRITE_8_BIT_VAL(buf, QMI_EMPTY); \
	 WRITE_16_BIT_VAL(buf, (unsigned long) (len - SIZE_8_BIT_VAL)); \
	 WRITE_8_BIT_VAL(buf, QMI_EMPTY); \
	 WRITE_8_BIT_VAL(buf, (unsigned long) serv); \
	 WRITE_8_BIT_VAL(buf, QMI_EMPTY); \
	 WRITE_8_BIT_VAL(buf, (unsigned long) ctrl_flags); \
	 WRITE_16_BIT_VAL(buf, (unsigned long) tx_id); \
	 WRITE_16_BIT_VAL(buf, (unsigned long) msg_id); \
	 WRITE_16_BIT_VAL(buf, (unsigned long) msg_len); \
    } while(0)

#define WRITE_8_BIT_VAL(buf,val) \
 do { unsigned char *b_ptr = buf; \
      unsigned long val_copy = (unsigned long) val; \
      unsigned char *v_ptr = (unsigned char *) &val_copy; \
      int unlikely_cntr; \
      for (unlikely_cntr=0; unlikely_cntr<SIZE_8_BIT_VAL; unlikely_cntr++) {*b_ptr++ = *v_ptr++;} \
      buf = b_ptr; \
    } while (0)

#define WRITE_16_BIT_VAL(buf,val) \
do { unsigned char *b_ptr = buf; \
      unsigned long val_copy = (unsigned long) val; \
      unsigned char *v_ptr = (unsigned char *) &val_copy; \
      int unlikely_cntr; \
      for (unlikely_cntr=0; unlikely_cntr<SIZE_16_BIT_VAL; unlikely_cntr++) {*b_ptr++ = *v_ptr++;} \
      buf = b_ptr; \
    } while (0)

#define WRITE_32_BIT_VAL(buf,val) \
do { unsigned char *b_ptr = buf; \
      unsigned long val_copy = (unsigned long) val; \
      unsigned char *v_ptr = (unsigned char *) &val_copy; \
      int unlikely_cntr; \
      for (unlikely_cntr=0; unlikely_cntr<SIZE_32_BIT_VAL; unlikely_cntr++) {*b_ptr++ = *v_ptr++;} \
      buf = b_ptr; \
    } while (0)


#define READ_8_BIT_VAL(buf,dest) \
do { unsigned char *b_ptr = buf; \
     unsigned char *d_ptr = (unsigned char *) &dest; \
     int unlikely_cntr; \
     dest = 0; \
     for (unlikely_cntr=0; unlikely_cntr<SIZE_8_BIT_VAL; unlikely_cntr++) {*d_ptr++ = *b_ptr++;} \
     buf = b_ptr; \
   } while (0)

#define READ_16_BIT_VAL(buf,dest) \
do { unsigned char *b_ptr = buf; \
     unsigned char *d_ptr = (unsigned char *) &dest; \
     int unlikely_cntr; \
     dest = 0; \
     for (unlikely_cntr=0; unlikely_cntr<SIZE_16_BIT_VAL; unlikely_cntr++) {*d_ptr++ = *b_ptr++;} \
     buf = b_ptr; \
   } while (0)

#define READ_32_BIT_VAL(buf,dest) \
do { unsigned char *b_ptr = buf; \
     unsigned char *d_ptr = (unsigned char *) &dest; \
     int unlikely_cntr; \
     dest = 0; \
     for (unlikely_cntr=0; unlikely_cntr<SIZE_32_BIT_VAL; unlikely_cntr++) {*d_ptr++ = *b_ptr++;} \
     buf = b_ptr; \
   } while (0)

/*
Log Message Macros
*/
#define LOG_MSG_INFO1_LEVEL           MSG_LEGACY_MED
#define LOG_MSG_INFO2_LEVEL           MSG_LEGACY_MED
#define LOG_MSG_INFO3_LEVEL           MSG_LEGACY_LOW
#define LOG_MSG_ERROR_LEVEL           MSG_LEGACY_ERROR
#define PRINT_MSG( level, fmtString, x, y, z)                         \
        MSG_SPRINTF_4( MSG_SSID_LINUX_DATA, level, "%s(): " fmtString,      \
                       __FUNCTION__, x, y, z);
#define LOG_MSG_INFO1( fmtString, x, y, z)                            \
{                                                                     \
  PRINT_MSG( LOG_MSG_INFO1_LEVEL, fmtString, x, y, z);                \
}
#define LOG_MSG_INFO2( fmtString, x, y, z)                            \
{                                                                     \
  PRINT_MSG( LOG_MSG_INFO2_LEVEL, fmtString, x, y, z);                \
}
#define LOG_MSG_INFO3( fmtString, x, y, z)                            \
{                                                                     \
  PRINT_MSG( LOG_MSG_INFO3_LEVEL, fmtString, x, y, z);                \
}
#define LOG_MSG_ERROR( fmtString, x, y, z)                            \
{                                                                     \
  PRINT_MSG( LOG_MSG_ERROR_LEVEL, fmtString, x, y, z);                \
}

/*--------------------------------------------------------------------------
   Events that need to propagated from QTI
---------------------------------------------------------------------------*/
typedef enum
{
  QMI_IP_INIT = 0,
  QMI_IP_LINK_UP_WAIT,
  QMI_IP_LINK_UP,
  QMI_IP_LINK_DOWN_WAIT,
  QMI_IP_LINK_DOWN
} qmi_ip_nl_state_e;

/*---------------------------------------------------------------------------
   Struct to store WDS handle for tethered autoconnect
---------------------------------------------------------------------------*/
typedef struct
{
 qmi_client_type         qmi_ip_v4_wds_handle;
 qmi_client_type         qmi_ip_v6_wds_handle;
} qmi_ip_ac_conf;

/*---------------------------------------------------------------------------
   Struct to store WDS/QCMAP handles
---------------------------------------------------------------------------*/
typedef struct
{
 qmi_ip_nl_state_e       state;
 qmi_client_type         qmi_ip_v4_wds_handle;
 qmi_client_type         qmi_ip_v6_wds_handle;
 uint32                  qmi_ip_v4_wds_call_handle;
 uint32                  qmi_ip_v6_wds_call_handle;
 qmi_client_type         qmi_ip_qcmap_msgr_handle;
 uint32_t                qmi_ip_mobile_ap_handle;
 qmi_ip_ac_conf          autoconnect_conf;
} qmi_ip_conf_t;

/*---------------------------------------------------------------------------
   Call info for a DSI Call
---------------------------------------------------------------------------*/
typedef struct {
  dsi_hndl_t handle;
  const char* tech;
  const char* family;
  int profile;
} dsi_call_info_t;

int netmgr_status;
int embms_call_state;

extern int multicast_server_finished;
extern int acceptTCPConnection;
extern int heartbeat_check;
extern int heartbeat_response;
extern int shut_down;
extern int sslConnecting;
extern int ssl_active;

extern SSL*                ssl;
extern int                 qmi_handle;
extern int                 wds_clnt_hndl;
extern qmi_client_type     qmi_nas_handle;
extern dsi_call_info_t     dsi_net_hndl;

extern int                 device_mode;

extern pthread_cond_t      cond;
extern pthread_mutex_t     mutex;

extern qmi_ip_conf_t       qmi_ip_conf;

void* multicast_listener(void);

extern int qmi_ip_wds_init(qmi_ip_conf_t * conf, SSL* s,
                           qmi_client_qmux_instance_type rmnet_instance);

static void qmi_ip_nas_sys_info_cb
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  void                           *resp_cb_data,
  qmi_client_error_type          transp_err
);

#endif
