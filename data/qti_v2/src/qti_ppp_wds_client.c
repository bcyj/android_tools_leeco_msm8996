/*=============================================================================

FILE:  qti_ppp_wds_client.c

SERVICES: Implementation of QTI interface with WDS service for PPP
===============================================================================

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

=============================================================================*/

/*=============================================================================
  EDIT HISTORY FOR MODULE

  Please notice that the changes are listed in reverse chronological order.

  when       who        what, where, why
  --------   ---        ------------------------------------------------------
  02/19/14   cp         Initial version

=============================================================================*/


/*=============================================================================
                               INCLUDE FILES
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h>
#include <sys/un.h>
#include "ds_util.h"
#include "ds_string.h"
#include "wireless_data_service_v01.h"
#include "qti_ppp.h"
#include "stringl.h"

/*=============================================================================
                                MACRO DEFINITIONS
==============================================================================*/


/*=============================================================================
                                VARIABLE DEFINITIONS
==============================================================================*/
qmi_client_type         qti_ppp_wds_handle;
static boolean qti_ppp_wds_inited;

/* Server sockets */
unsigned int qti_ppp_wds_sockfd;

/* Client sockets */
unsigned int wds_qti_ppp_sockfd;
/*=============================================================================
                                FUNCTION FORWARD DECLARATION
==============================================================================*/
void qti_ppp_wds_ind_cb
(
 qmi_client_type user_handle,                    /* QMI user handle       */
 unsigned int    msg_id,                         /* Indicator message ID  */
 void           *ind_buf,                        /* Raw indication data   */
 unsigned int    ind_buf_len,                    /* Raw data length       */
 void           *ind_cb_data                     /* User call back handle */
 );

/*=============================================================================
                                FUNCTION DEFINITIONS
==============================================================================*/
int create_socket(unsigned int *sockfd)
{

  if ((*sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == QTI_PPP_FAILURE)
  {
    LOG_MSG_ERROR("Error creating socket, errno: %d", errno, 0, 0);
    return QTI_PPP_FAILURE;
  }

  if(fcntl(*sockfd, F_SETFD, FD_CLOEXEC) < 0)
  {
    LOG_MSG_ERROR("Couldn't set Close on Exec, errno: %d", errno, 0, 0);
  }

  return QTI_PPP_SUCCESS;
}

int create_qti_ppp_wds_socket(void)
{
  int val, rval;
  struct sockaddr_un qti_ppp_wds;
  int len;
  struct timeval rcv_timeo;

  rval = create_socket(&qti_ppp_wds_sockfd);

  if (rval == QTI_PPP_FAILURE)
    return QTI_PPP_FAILURE;

  rcv_timeo.tv_sec = 0;
  rcv_timeo.tv_usec = 100000;
  setsockopt(qti_ppp_wds_sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&rcv_timeo, sizeof(rcv_timeo));
  val = fcntl(qti_ppp_wds_sockfd, F_GETFL, 0);
  fcntl(qti_ppp_wds_sockfd, F_SETFL, val | O_NONBLOCK);

  qti_ppp_wds.sun_family = AF_UNIX;
  strlcpy(qti_ppp_wds.sun_path, QTI_PPP_WDS_UDS_FILE, QTI_PPP_UNIX_PATH_MAX);
  unlink(qti_ppp_wds.sun_path);
  len = strlen(qti_ppp_wds.sun_path) + sizeof(qti_ppp_wds.sun_family);
  if (bind(qti_ppp_wds_sockfd, (struct sockaddr *)&qti_ppp_wds, len) == QTI_PPP_FAILURE)
  {
    LOG_MSG_ERROR("Error binding the socket, errno: %d", errno, 0, 0);
    close(qti_ppp_wds_sockfd);
    return QTI_PPP_FAILURE;
  }
  return QTI_PPP_SUCCESS;
}
/*===========================================================================

FUNCTION QTI_PPP_WDS_MAP_FD_READ()

DESCRIPTION

  This function
  - adds the WDS DUN indication fd to the list of FD on which select call listens
  - maps the read fucntion for the WDS DUN fd

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/

static int qti_ppp_wds_map_fd_read
(
   qti_ppp_nl_sk_fd_set_info_t *fd_set,
   int                     qti_ppp_wds_sockfd,
   qti_ppp_sock_thrd_fd_read_f read_f
)
{
  if( fd_set->num_fd < MAX_NUM_OF_FD )
  {
    FD_SET(qti_ppp_wds_sockfd, &(fd_set->fdset));
/*--------------------------------------------------------------------------
  Add fd to fdmap array and store read handler function ptr
-------------------------------------------------------------------------- */
    fd_set->sk_fds[fd_set->num_fd].sk_fd = qti_ppp_wds_sockfd;
    fd_set->sk_fds[fd_set->num_fd].read_func = read_f;
    LOG_MSG_INFO1("Added read function for fd %d", qti_ppp_wds_sockfd, 0, 0);

/*--------------------------------------------------------------------------
  Increment number of fds stored in fdmap
--------------------------------------------------------------------------*/
    fd_set->num_fd++;
    if(fd_set->max_fd < qti_ppp_wds_sockfd)
    {
      LOG_MSG_INFO1("Updating USB max fd %d", qti_ppp_wds_sockfd, 0, 0);
      fd_set->max_fd = qti_ppp_wds_sockfd;
    }
  }
  else
  {
    LOG_MSG_ERROR("Exceeds maximum num of FD", 0, 0, 0);
    return QTI_PPP_FAILURE;
  }

  return QTI_PPP_SUCCESS;
}
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
                         qti_ppp_sock_thrd_fd_read_f read_f)
{
  qmi_idl_service_object_type                            qti_wds_service_object;
  qmi_client_error_type                                  qmi_error, qmi_err_code = QMI_NO_ERR;
  qmi_cci_os_signal_type                                 qti_wds_os_params;
  int                                                    ret_val = QTI_PPP_SUCCESS;
  wds_get_dun_call_info_req_msg_v01 qcmap_dun_call_info_req_msg;
  wds_get_dun_call_info_resp_msg_v01 qcmap_dun_call_info_resp_msg;
/*---------------------------------------------------------------------------*/

  /* Check for NULL Args. */
  if (fd_set == NULL)
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return QTI_PPP_FAILURE;
  }

  LOG_MSG_INFO1("qti_ppp_wds_init()", 0, 0, 0);

/*-----------------------------------------------------------------------------
  Obtain a WDS service client for QTI PPP
  - get the service object
  - obtain the client
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Get the service object
------------------------------------------------------------------------------*/
  qti_wds_service_object = wds_get_service_object_v01();
  if (qti_wds_service_object == NULL)
  {
    LOG_MSG_ERROR("QTI WDS messenger service object not available",
                   0, 0, 0);
    return QTI_PPP_FAILURE;
  }

/*----------------------------------------------------------------------------
  Obtain a QCMAP messenger client for QTI
------------------------------------------------------------------------------*/
  qmi_error = qmi_client_init_instance(qti_wds_service_object,
                              QMI_CLIENT_INSTANCE_ANY,
                              qti_ppp_wds_ind_cb,
                              NULL,
                              &qti_wds_os_params,
                              QTI_PPP_QMI_MAX_TIMEOUT_MS,
                              &qti_ppp_wds_handle);

  LOG_MSG_INFO1("qmi_client_init_instance: %d", qmi_error, 0, 0);


  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not init qcmap_msgr client %d", qmi_error, 0, 0);
    return QTI_PPP_FAILURE;
  }

  memset(&qcmap_dun_call_info_req_msg, 0, sizeof(qcmap_dun_call_info_req_msg));
  memset(&qcmap_dun_call_info_resp_msg, 0, sizeof(qcmap_dun_call_info_resp_msg));

  qcmap_dun_call_info_req_msg.report_connection_status_valid = TRUE;
  qcmap_dun_call_info_req_msg.report_connection_status = 1;

  qmi_error = qmi_client_send_msg_sync(qti_ppp_wds_handle,
                                       QMI_WDS_GET_DUN_CALL_INFO_REQ_V01,
                                       &qcmap_dun_call_info_req_msg,
                                       sizeof(qcmap_dun_call_info_req_msg),
                                       &qcmap_dun_call_info_resp_msg,
                                       sizeof(qcmap_dun_call_info_resp_msg),
                                       QTI_PPP_QMI_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(qcmap_cm_enable): error %d result %d",
                qmi_error, qcmap_dun_call_info_resp_msg.resp.result,0);
  if (qmi_error != QMI_NO_ERR)
  {
    qmi_client_release(qti_ppp_wds_handle);
    qti_ppp_wds_handle = NULL;
    LOG_MSG_ERROR("Can not perform DUN call indication register %d",
                 qmi_error, 0, 0);
    return QTI_PPP_FAILURE;
  }

   /* Initialize the sockets. */
  /* Create WDS> QTI_PPP client socket */
  if (create_socket(&wds_qti_ppp_sockfd) != QTI_PPP_SUCCESS)
  {
    LOG_MSG_ERROR("qti_ppp_wdsnit::error creating wds_ppp_sockfd socket", 0, 0, 0);
  }

  /* Create qti_ppp -> qcmap server socket */
  if (create_qti_ppp_wds_socket() != QTI_PPP_SUCCESS)
  {
    LOG_MSG_ERROR("Unable to create qti_ppp_wds_socket!", 0,0,0);
    return QTI_PPP_FAILURE;
  }

  ret_val = qti_ppp_wds_map_fd_read(fd_set,qti_ppp_wds_sockfd, read_f);
  if(ret_val == QTI_PPP_FAILURE)
  {
    LOG_MSG_ERROR("Failed to map fd with the read function", 0, 0, 0);
    return QTI_PPP_FAILURE;
  }

  qti_ppp_wds_inited = TRUE;

  return QTI_PPP_SUCCESS;
}

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
 int qti_ppp_wds_exit()
{
  qmi_client_error_type     qmi_error;

/*----------------------------------------------------------------------------*/

  if ( !qti_ppp_wds_inited )
    return QTI_PPP_SUCCESS;

  qmi_error = qmi_client_release(qti_ppp_wds_handle);
  qti_ppp_wds_handle = NULL;

  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not release client wds client handle %d",
                  qmi_error,0,0);
    return QTI_PPP_FAILURE;
  }

  /* Close the sockets. */
  if (qti_ppp_wds_sockfd)
  {
    qti_ppp_clear_fd(&sk_fdset, qti_ppp_wds_sockfd);
    close(qti_ppp_wds_sockfd);
  }

  if (wds_qti_ppp_sockfd)
    close(wds_qti_ppp_sockfd);

  qti_ppp_wds_inited = FALSE;

  return QTI_PPP_SUCCESS;
}


/*===========================================================================
  FUNCTION  qti_ppp_wds_ind_cb
  ===========================================================================*/
/*!
  @brief
  Processes an incoming QMI WDS Indication.

  @return
  void

  @note

  - Dependencies
  - None

  - Side Effects
  - None
 */
/*=========================================================================*/
void qti_ppp_wds_ind_cb
(
 qmi_client_type user_handle,                    /* QMI user handle       */
 unsigned int    msg_id,                         /* Indicator message ID  */
 void           *ind_buf,                        /* Raw indication data   */
 unsigned int    ind_buf_len,                    /* Raw data length       */
 void           *ind_cb_data                     /* User call back handle */
 )
{
  qmi_client_error_type qmi_error = QMI_NO_ERR;
  wds_dun_call_info_ind_msg_v01 dun_call_info_ind;
  qti_ppp_dun_call_status_enum call_status;
  struct sockaddr_un qti_ppp_wds;
  int numBytes=0, len;

  LOG_MSG_INFO1("qti_ppp_wds_ind_cb: event %d .",
                msg_id,
                0, 0);

  switch (msg_id)
  {
    case QMI_WDS_DUN_CALL_INFO_IND_V01:
      qmi_error = qmi_client_message_decode(user_handle,
                                            QMI_IDL_INDICATION,
                                            msg_id,
                                            ind_buf,
                                            ind_buf_len,
                                            &dun_call_info_ind,
                                            sizeof(wds_dun_call_info_ind_msg_v01));
      if (qmi_error == QMI_NO_ERR)
      {
        if ( dun_call_info_ind.modem_connection_status_valid &&
             dun_call_info_ind.modem_connection_status == WDS_CONNECTION_STATUS_DISCONNECTED_V01)
        {
          LOG_MSG_INFO1("qti_ppp_wds_ind_cb: DUN call Disconnected...",0,0,0);
          call_status = QTI_PPP_DUN_CALL_DISCONNECTED_V01;
        }
        if ( dun_call_info_ind.modem_connection_status_valid &&
             dun_call_info_ind.modem_connection_status == WDS_CONNECTION_STATUS_CONNECTED_V01)
        {
          LOG_MSG_INFO1("qti_ppp_wds_ind_cb: DUN call Connected...",0,0,0);
          call_status = QTI_PPP_DUN_CALL_CONNECTED_V01;
        }

        if ( dun_call_info_ind.modem_connection_status_valid )
        {

          qti_ppp_wds.sun_family = AF_UNIX;
          strlcpy(qti_ppp_wds.sun_path, QTI_PPP_WDS_UDS_FILE, QTI_PPP_UNIX_PATH_MAX);
          len = strlen(qti_ppp_wds.sun_path) + sizeof(qti_ppp_wds.sun_family);

          if ((numBytes = sendto(wds_qti_ppp_sockfd, (void *)&call_status, sizeof(qti_ppp_dun_call_status_enum), 0,
              (struct sockaddr *)&qti_ppp_wds, len)) == -1)
          {
            LOG_MSG_ERROR("Send Failed from wds indication context", errno, 0, 0);
            return;
          }
        }
      }
      break;
    default:
      /* Ignore all other indications */
      break;
  }
  return;
}
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
)
{
  int        ret;
  qti_ppp_dun_call_status_enum call_status;
  int       ret_val;

 /*-------------------------------------------------------------------------*/
  ret = read(qti_qpp_qcmap_sockfd, &call_status, sizeof(call_status));
  if (ret <= 0)
  {
    LOG_MSG_ERROR("Failed to read from the dev file %d:%d", qti_qpp_qcmap_sockfd, errno, 0);
    return QTI_PPP_FAILURE;
  }

  if ( call_status == QTI_PPP_DUN_CALL_DISCONNECTED_V01)
  {
    /*---------------------------------------------------------------------
      Call into the USB TTY listener init function which sets up QTI to
      listen to AT Commands coming in from the USB device file for DUN
    ---------------------------------------------------------------------*/
    ret_val = qti_usb_tty_listener_init(&usb_tty_config_info,
                                        &sk_fdset,
                                        qti_usb_tty_recv_msg);
    if (ret_val != QTI_PPP_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to initialize QTI USB TTY listener",0,0,0);
    }
  }
  else if ( call_status == QTI_PPP_DUN_CALL_CONNECTED_V01 )
  {
    LOG_MSG_INFO1(" Ignoring DUN call connected event.", 0, 0, 0);
  }
}

