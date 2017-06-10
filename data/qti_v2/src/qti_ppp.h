/******************************************************************************

                        QTI_PPP.H

******************************************************************************/

/******************************************************************************

  @file    qti_ppp.h
  @brief   Qualcomm Tethering Interface module for PPP.

  DESCRIPTION
  Header file for Qualcomm Tethering Interface.

  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/19/14   cp         Initial version
05/02/14   pm         Added log message macros

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_addr.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include <termios.h>

#include "ds_util.h"
#include "qmi_client.h"

#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#define strlcat g_strlcat
#endif

/*===========================================================================
                              MACRO DEFINITIONS
===========================================================================*/
#define MAX_NUM_OF_FD                       4
#define QTI_PPP_NL_MSG_MAX_LEN                  1024
#define QTI_PPP_SUCCESS                         0
#define QTI_PPP_FAILURE                         (-1)
#define QTI_PPP_QMI_MAX_TIMEOUT_MS            5000 /*Timeout value in miliseconds */
#define QTI_PPP_MAX_FILE_NAME_SIZE              50
#define USB_MAX_TRANSFER_SIZE 2048
#define DUN_TTY_BAUDRATE B115200
#define QTI_PPP_QMI_TIMEOUT_VALUE        90000
#define QTI_PPP_UNIX_PATH_MAX    108
#define MAX_COMMAND_STR_LEN                 200
/* USB TTY Device Init Delay Micro seconds */
#define USB_DEV_INIT_DELAY                  1000000

#define TRUE 1
#define FALSE 0

#define USB_DUN_TTY_PORT "/dev/ttyGS1"
#define SMD_DS_TTY_PORT "/dev/smd8"
#define USB_SERIAL_PORT "/dev/android_serial_device"
#define USB_DUN_TTY_ENABLE_FLAG "/sys/class/android_usb/android0/f_serial/dun_w_softap_enable"
#define USB_DUN_TTY_ACTIVE_FLAG "/sys/class/android_usb/android0/f_serial/dun_w_softap_active"

#define DUN_HASH_CHAR '#'
#define DUN_ASTERISK_CHAR '*'
#define DUN_DIAL_STRING "ATD"
#define DUN_3GPP2_DIAL_STRING "ATDT#777"
#define DUN_3GPP_DIAL_STRING1 "ATDT*98"
#define DUN_3GPP_DIAL_STRING2 "ATDT*99"
#define DUN_CONNECT_STRING "CONNECT"
#define DUN_CONNECT_STRING_LEN 7
#define DUN_CONNECT_STRING_WITH_CR "\r\nCONNECT\r\n"
#define DUN_CONNECT_STRING_WITH_CR_LEN 11
#define DUN_NO_CARRIER_STRING "\r\nNO CARRIER\r\n"
#define DUN_NO_CARRIER_STRING_LEN 14

#define QTI_PPP_WDS_UDS_FILE "/etc/qti_ppp_wds_uds_file"
#define QTI_PPP_LS_FILE "/etc/qti_ppp_ls_uds_file"

/*Address lengths*/
#define INET_ADDRSTRLEN        16
#define INET6_ADDRSTRLEN       46

#ifdef QTI_PPP_DEBUG
#undef LOG_MSG_INFO1
#define LOG_MSG_INFO1(fmtString, x, y, z) \
{ \
  if ( x != 0 && y !=0 && z != 0) \
    printf("\nINFO1:"fmtString"\n", x, y, z); \
  else if ( x != 0 && y != 0) \
    printf("\nINFO1:"fmtString"\n", x, y); \
  else if ( x != 0) \
    printf("\nINFO1:"fmtString"\n", x); \
  else \
    printf("\nINFO1:"fmtString"\n"); \
}
#undef LOG_MSG_INFO3
#define LOG_MSG_INFO3(fmtString, x, y, z) \
{ \
  if ( x != 0 && y !=0 && z != 0) \
    printf("\nINFO3:"fmtString"\n", x, y, z); \
  else if ( x != 0 && y != 0) \
    printf("\nINFO3:"fmtString"\n", x, y); \
  else if ( x != 0) \
    printf("\nINFO3:"fmtString"\n", x); \
  else \
    printf("\nINFO3:"fmtString"\n"); \
}

#undef LOG_MSG_ERROR
#define LOG_MSG_ERROR(fmtString, x, y, z) \
{ \
  if ( x != 0 && y !=0 && z != 0) \
    printf("\nError:"fmtString"\n", x, y, z); \
  else if ( x != 0 && y != 0) \
    printf("\nError:"fmtString"\n", x, y); \
  else if ( x != 0) \
    printf("\nError:"fmtString"\n", x); \
  else \
    printf("\nError:"fmtString"\n"); \
}

#else

/*============================================================
Log Message Macros
=============================================================*/

#define LOG_MSG_INFO1_LEVEL           MSG_LEGACY_MED
#define LOG_MSG_INFO2_LEVEL           MSG_LEGACY_MED
#define LOG_MSG_INFO3_LEVEL           MSG_LEGACY_LOW
#define LOG_MSG_ERROR_LEVEL           MSG_LEGACY_ERROR
#define PRINT_MSG( level, fmtString, x, y, z)                         \
        MSG_SPRINTF_4( MSG_SSID_LINUX_DATA, level, "%s(): " fmtString,      \
                       __FUNCTION__, x, y, z);
#define PRINT_MSG_6( level, fmtString, a, b, c, d, e, f)              \
  MSG_SPRINTF_7( MSG_SSID_LINUX_DATA, level, "%s(): " fmtString,            \
                 __FUNCTION__, a, b, c, d, e, f);

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
#define LOG_MSG_INFO1_6( fmtString, a, b, c, d, e, f)                 \
{                                                                     \
  PRINT_MSG_6 ( LOG_MSG_INFO1_LEVEL, fmtString, a, b, c, d, e, f);    \
}
#define LOG_MSG_ERROR( fmtString, x, y, z)                            \
{                                                                     \
  PRINT_MSG( LOG_MSG_ERROR_LEVEL, fmtString, x, y, z);                \
}
#define LOG_MSG_ERROR_6( fmtString, a, b, c, d, e, f)                 \
{                                                                     \
  PRINT_MSG_6( LOG_MSG_ERROR_LEVEL, fmtString, a, b, c, d, e, f);     \
}

#endif

/*--------------------------------------------------------------------------
   Events that need to propagated to QCMAP from QTI_PPP
---------------------------------------------------------------------------*/
typedef enum
{
  QTI_PPP_LINK_UP_EVENT =1,
  QTI_PPP_LINK_DOWN_EVENT
} qti_qcmap_event_e;

/*--------------------------------------------------------------------------
   QTI PPP configuration variable which maintains the information needed with
   respect to a QTI call
---------------------------------------------------------------------------*/
typedef struct
{
 qmi_client_type         qti_ppp_qcmap_msgr_handle;
 uint32_t                qti_ppp_mobile_ap_handle;
} qti_ppp_conf_t;

/*---------------------------------------------------------------------------
   Function pointer registered with the socket listener
   This function is used for reading from a socket on receipt of an incoming
   netlink event
---------------------------------------------------------------------------*/
typedef int (* qti_ppp_sock_thrd_fd_read_f) (int fd);

/*--------------------------------------------------------------------------
   Stores the mapping of a socket descriptor and its associated read
   function
---------------------------------------------------------------------------*/
typedef struct
{
 int sk_fd;
 qti_ppp_sock_thrd_fd_read_f read_func;
} qti_ppp_nl_sk_fd_map_info_t;

/*--------------------------------------------------------------------------
   Stores the socket information associated with netlink sockets required
   to listen to netlink events
---------------------------------------------------------------------------*/
typedef struct
{
 qti_ppp_nl_sk_fd_map_info_t sk_fds[MAX_NUM_OF_FD];
 fd_set fdset;
 int num_fd;
 int max_fd;
} qti_ppp_nl_sk_fd_set_info_t;

/*--------------------------------------------------------------------------
   Socket descriptor paramters
---------------------------------------------------------------------------*/
typedef struct
{
 int                 sk_fd;       /* socket descriptor */
 struct sockaddr_nl  sk_addr_loc; /*  stores socket parameters */
} qti_ppp_nl_sk_info_t;

/*--------------------------------------------------------------------------
   Stoes the metainfo present in the incoming netlink message
---------------------------------------------------------------------------*/
typedef struct
{
 struct ifinfomsg  metainfo;
} qti_ppp_nl_link_info_t;

/*--------------------------------------------------------------------------
   Netlink message: used to decode the incoming netlink message
---------------------------------------------------------------------------*/
typedef struct
{
 unsigned int type;
 boolean link_event;
 qti_ppp_nl_link_info_t nl_link_info;
} qti_ppp_nl_msg_t;

/*--------------------------------------------------------------------------
  Netlink message structure used to send GET_LINK
---------------------------------------------------------------------------*/
typedef struct
{
  struct nlmsghdr hdr;
  struct rtgenmsg gen;
}nl_ppp_req_type;

/*-------------------------------------------------------------------------
  DUN message handling status
--------------------------------------------------------------------------*/
typedef struct{
  int                  usb_fd;
  int                  is_ppp_active;
}qti_usb_tty_config;

/*-------------------------------------------------------------------------
  DUN message handling line state
--------------------------------------------------------------------------*/
typedef struct{
  int                  line_state_fd;
}qti_usb_line_state_config;

/*-------------------------------------------------------------------------
  USB connection status handling
--------------------------------------------------------------------------*/
typedef struct{
  int                  notify_fd;
  int                  enable_watch_fd;
  int                  active_watch_fd;
}qti_usb_tty_notify_config;

/*-------------------------------------------------------------------------
  DUN message handling status
--------------------------------------------------------------------------*/
typedef struct{
  int                  smd_fd;
}qti_smd_tty_config;

struct qti_ppp_ioctl_smd_write_arg_type {
char* buf;
unsigned int size;
};

/*-------------------------------------------------------------------------
  FD Set.
--------------------------------------------------------------------------*/
extern qti_ppp_nl_sk_fd_set_info_t   sk_fdset;
extern qti_usb_tty_config   usb_tty_config_info;

typedef uint64_t qti_ppp_tech_pref_mask_v01;
#define QTI_PPP_MASK_TECH_PREF_3GPP_V01 ((qti_ppp_tech_pref_mask_v01)0x01ull) /**<  3GPP  */
#define QTI_PPP_MASK_TECH_PREF_3GPP2_V01 ((qti_ppp_tech_pref_mask_v01)0x02ull) /**<  3GPP2  */

typedef enum
{
  QTI_PPP_DUN_CALL_CONNECTED_V01 = 0x1,
  QTI_PPP_DUN_CALL_DISCONNECTED_V01
}qti_ppp_dun_call_status_enum;

/*===========================================================================
                       FUNCTION DECLARATIONS
===========================================================================*/
/*===========================================================================

FUNCTION QTI_PPP_CLEAR_FD()

DESCRIPTION

  This function
  - Removes the fd to the list of FD on which select call listens.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/
int qti_ppp_clear_fd
(
   qti_ppp_nl_sk_fd_set_info_t *fd_set,
   int                     fd
);
/*===========================================================================

FUNCTION QTI_PPP_QCMAP_INIT()

DESCRIPTION

  This function initializes QTI:
  - initializes a QCMAP MSGR client for PPP

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/

int qti_ppp_qcmap_init(qti_ppp_conf_t * qti_ppp_conf);

/*=============================================================================
  FUNCTION QTI_QCMAP_EXIT()

  DESCRIPTION

  This function releases QCMAP client

  DEPENDENCIES
  None.

  RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure

  SIDE EFFECTS
  None
==============================================================================*/
 int qti_ppp_qcmap_exit(void);

/*===========================================================================
FUNCTION QTI_PPP_NL_LISTENER_INIT()

DESCRIPTION

  This function initializes netlink sockets and also performs a query to find
  any netlink events that could happened before netlink socket
  initialization.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None
=========================================================================*/
int qti_ppp_nl_listener_init
(
  unsigned int nl_type,
  unsigned int nl_groups,
  qti_ppp_nl_sk_fd_set_info_t * sk_fdset,
  qti_ppp_sock_thrd_fd_read_f read_f
);

/*===========================================================================
FUNCTION QTI_PPP_NL_RECV_MSG()

DESCRIPTION

  Function to receive incoming messages over the NETLINK routing socket.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None
==========================================================================*/
int qti_ppp_nl_recv_msg(int fd);

/*===========================================================================

FUNCTION QTI_USB_TTY_LISTENER_INIT()

DESCRIPTION

  This function
  - sets up QTI to start listening for AT commands coming on USB interface.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*=========================================================================*/
int qti_usb_tty_listener_init
(
  qti_usb_tty_config  * config_info,
  qti_ppp_nl_sk_fd_set_info_t * fd_set,
  qti_ppp_sock_thrd_fd_read_f read_f
);

/*===========================================================================

FUNCTION QTI_SMD_TTY_LISTENER_INIT()

DESCRIPTION

  This function
  - sets up QTI to start listening for AT commands coming on SMD TTY device.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*=========================================================================*/
int qti_smd_tty_listener_init
(
  qti_smd_tty_config  * smd_tty_config_info,
  qti_ppp_nl_sk_fd_set_info_t * fd_set,
  qti_ppp_sock_thrd_fd_read_f read_f
);
/*===========================================================================

FUNCTION QTI_USB_TTY_RECV_MSG()

DESCRIPTION

  This function
  - receives AT commands from USB interface.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*=========================================================================*/
int qti_usb_tty_recv_msg
(
   int usb_tty_fd
);

/*===========================================================================

FUNCTION QTI_USB_TTY_SEND_MSG()

DESCRIPTION

  This function
  - send AT commands to USB

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/

void qti_usb_tty_send_msg
(
   void      *data,
   uint32_t   len
);

/*===========================================================================

FUNCTION QTI_USB_LINE_STATE_INIT()

DESCRIPTION

  This function
  - Adds fd to get line state notifications from QTI.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/

int qti_usb_line_state_init
(
  qti_usb_line_state_config *line_state_config,
  qti_ppp_nl_sk_fd_set_info_t * fd_set,
  qti_ppp_sock_thrd_fd_read_f read_f
);

/*===========================================================================

FUNCTION QTI_USB_LINE_STATE_RECV_MSG()

DESCRIPTION

  This function
  - receives notifications about line state.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*=========================================================================*/
int qti_usb_line_state_recv_msg
(
   int usb_line_state_fd
);

/*=========================================================================
FUNCTION QTI_SMD_TTY_RECV_MSG()

DESCRIPTION

  This function
  - receives AT commands from SMD TTY Device.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*=========================================================================*/
int qti_smd_tty_recv_msg
(
   int smd_tty_fd
);

/*===========================================================================

FUNCTION QTI_SMD_TTY_SEND_MSG()

DESCRIPTION

  This function
  - send AT commands to Modem

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/

void qti_smd_tty_send_msg
(
   void      *data,
   uint32_t   len
);

/*===========================================================================

FUNCTION PRINT_BUFFER()

DESCRIPTION

  This function
  - prints the QMI packet.

DEPENDENCIES
  None.

RETURN VALUE


SIDE EFFECTS
  None

/*=========================================================================*/
void print_buffer
(
  char *buf,
  int size
);
/*===========================================================================

FUNCTION PRINT_AT_CMD()

DESCRIPTION

  This function
  - prints the AT Command.

DEPENDENCIES
  None.

RETURN VALUE


SIDE EFFECTS
  None

/*=========================================================================*/
void print_at_cmd
(
  char *buf,
  int size
);

/*===========================================================================

FUNCTION enable_mobile_ap()

DESCRIPTION

  This function enables QC Mobile AP
  QTI uses the services of QC Mobile AP

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*==========================================================================*/
int qti_ppp_enable_mobile_ap(void);
/*===========================================================================

FUNCTION disable_mobile_ap()

DESCRIPTION

  This function disables QC Mobile AP

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*==========================================================================*/
int qti_ppp_disable_mobile_ap(void);
/*===========================================================================

FUNCTION QTI_PPP_USB_LINK_UP()

DESCRIPTION

  This function sends a message to QCMAP setup the USB link for PPP interface

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*==========================================================================*/
int qti_ppp_usb_link_up
(
  void
);
/*===========================================================================

FUNCTION QTI_PPP_USB_LINK_DOWN()

DESCRIPTION

  This function sends a message to QCMAP to bring down the PPP link

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*==========================================================================*/
int qti_ppp_usb_link_down
(
  void
);
/*===========================================================================

FUNCTION QTI_PPP_VALIDATE_DUN_PROFILE()

DESCRIPTION

  This function sends a message to QCMAP to validate the DUN profile for SoftAP call.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*==========================================================================*/
int qti_ppp_validate_dun_profile
(
  uint8_t dun_profile_id,
  qti_ppp_tech_pref_mask_v01 tech_pref
);
/*=============================================================================
  FUNCTION QTI_PPP_WDS_INIT()

  DESCRIPTION

  This function initializes QTI interface to WDS for PPP.

  DEPENDENCIES
  None.

  RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure

  SIDE EFFECTS
  None
==============================================================================*/
int qti_ppp_wds_init(qti_ppp_nl_sk_fd_set_info_t * fd_set,
                         qti_ppp_sock_thrd_fd_read_f read_f
);
/*=============================================================================
  FUNCTION QTI_WDS_EXIT()

  DESCRIPTION

  This function releases WDS client

  DEPENDENCIES
  None.

  RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure

  SIDE EFFECTS
  None
==============================================================================*/
 int qti_ppp_wds_exit(void);
/*===========================================================================

FUNCTION QTI_PPP_WDS_RECV_MSG()

DESCRIPTION

  This function
  - receives dun call indications from WDS service.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*=========================================================================*/
int qti_ppp_wds_recv_msg
(
  int qti_qpp_qcmap_sockfd
);
