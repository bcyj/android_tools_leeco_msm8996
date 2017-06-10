/******************************************************************************
  @file    linux_qmi_qmux_if_client.c
  @brief   The QMI QMUX linux client

  DESCRIPTION
  Linux-based QMUX multi protection-domain client

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None

  ---------------------------------------------------------------------------
  Copyright (c) 2009-2012,2014-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <grp.h>
#include "qmi_util.h"
#include "qmi_platform_qmux_if.h"
#include "qmi_qmux_if.h"

#define LINUX_QMI_MAX_CONNECT_TRIES (60)
#define LINUX_QMI_MAX_RETRIES       (5)
#define LINUX_QMI_RETRY_DELAY       (10000)  /* 10 ms */

#define READ_PIPE_FD_INDEX 0
#define WRITE_PIPE_FD_INDEX 1

QMI_PLATFORM_MUTEX_CREATE_AND_INIT_STATIC_MUTEX (linux_qmi_qmux_if_platform_mutex);

QMI_PLATFORM_MUTEX_CREATE_AND_INIT_STATIC_MUTEX (linux_qmi_qmux_if_client_list_mutex);

typedef struct linux_qmi_qmux_if_client_data
{
  struct linux_qmi_qmux_if_client_data  *next;
  int                                   qmux_client_fd;
  qmi_qmux_clnt_id_t                    qmux_client_id;
  unsigned char                         *rx_buf;
  int                                   rx_buf_size;
  pthread_t                             th_id;
  int                                   pipe_fds[2];
  pthread_mutex_t                       client_mutex;
} linux_qmi_qmux_if_client_data_t;

/* List of qmux_if clients */
static linux_qmi_qmux_if_client_data_t  *linux_qmi_qmux_if_client_list = NULL;

static int linux_qmi_qmux_pid = -1;

typedef struct
{
  const char *grp_name;
  const char *bind_sock_path;
  const char *server_sock_path;
} linux_qmi_qmux_if_conn_sock_info_t;

#define LINUX_QMI_QMUX_IF_NUM_CLIENT_CONN_SOCK_ENTRIES (sizeof(linux_qmi_qmux_if_client_conn_socks)/   \
                                                        sizeof(linux_qmi_qmux_if_client_conn_socks[0]))
/* Structure containing information about the client bind and server socket to
   use for a paritcular client process */
static linux_qmi_qmux_if_conn_sock_info_t linux_qmi_qmux_if_client_conn_socks[] =
{
#ifdef FEATURE_QMI_ANDROID
  /* Radio Group Client Process */
  { "radio",      QMI_QMUX_IF_RADIO_CLIENT_SOCKET_PATH,     QMI_QMUX_IF_RADIO_CONN_SOCKET_PATH },

  /* Audio Group Client Process */
  { "audio",      QMI_QMUX_IF_AUDIO_CLIENT_SOCKET_PATH,     QMI_QMUX_IF_AUDIO_CONN_SOCKET_PATH },

  /* Bluetooth Group Client Process */
  { "bluetooth",  QMI_QMUX_IF_BLUETOOTH_CLIENT_SOCKET_PATH, QMI_QMUX_IF_BLUETOOTH_CONN_SOCKET_PATH },

  /* GPS Group Client Process */
  { "gps",        QMI_QMUX_IF_GPS_CLIENT_SOCKET_PATH,       QMI_QMUX_IF_GPS_CONN_SOCKET_PATH },

  /* NFC Group Client Process */
  { "nfc",        QMI_QMUX_IF_NFC_CLIENT_SOCKET_PATH,       QMI_QMUX_IF_NFC_CONN_SOCKET_PATH },
#endif
};

#ifdef FEATURE_QMI_ANDROID
int qmi_log_adb_level;
#endif

/* Forward function declarations */

static int linux_qmi_qmux_if_connect(void);

static int linux_qmi_qmux_if_client_get_client_id
(
  int                 client_fd,
  qmi_qmux_clnt_id_t  *qmux_client_id
);

#if defined(FEATURE_DATA_LOG_QXDM)
  boolean qmi_platform_qxdm_init = FALSE;
#endif

/*===========================================================================
                          LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  linux_qmi_qmux_if_client_get_proc_name
===========================================================================*/
/*!
@brief
  Returns the process name corresponding to the give pid

@return
  QMI_NO_ERR on success
  QMI_INTERNAL_ERR otherwise

*/
/*=========================================================================*/
static int
linux_qmi_qmux_if_client_get_proc_name
(
  int   pid,
  char  *proc_name,
  int   proc_name_size
)
{
  char proc_entry[50];
  char tmp_proc_name[50];
  int fd;
  ssize_t nread;
  int rc = QMI_INTERNAL_ERR;
  char *ptr = NULL;

  if (!proc_name || proc_name_size <= 0)
  {
    QMI_ERR_MSG_0("linux_qmi_qmux_if_client_get_proc_name: bad parameter(s)\n");
    return rc;
  }

  /* Obtain the command used when launching the process */
  snprintf(proc_entry, sizeof(proc_entry), "/proc/%d/cmdline", pid);

  /* Open the proc entry */
  fd = open(proc_entry, O_RDONLY);

  if (fd < 0)
  {
    QMI_ERR_MSG_3("linux_qmi_qmux_if_client_get_proc_name: failed to open proc_entry=%s, errno=[%d:%s]\n",
                  proc_entry,
                  errno,
                  strerror(errno));
    goto bail;
  }

  nread = read(fd, tmp_proc_name, sizeof(tmp_proc_name)-1);

  if (nread <= 0)
  {
    QMI_ERR_MSG_2("linux_qmi_qmux_if_client_get_proc_name: read failed errno=[%d:%s]\n",
                  errno,
                  strerror(errno));
    goto bail;
  }

  tmp_proc_name[nread] = '\0'; /* set NULL at end */

  /* Obtain the last entry if directory names are present */
  if (NULL == (ptr = strrchr(tmp_proc_name, '/')))
  {
#ifdef FEATURE_QMI_ANDROID
    strlcpy(proc_name, tmp_proc_name, (size_t)proc_name_size);
#else
    strncpy(proc_name, tmp_proc_name, proc_name_size-1);
    proc_name[proc_name_size-1] = '\0';
#endif
  }
  else
  {
#ifdef FEATURE_QMI_ANDROID
    strlcpy(proc_name, ptr+1, (size_t)proc_name_size);
#else
    strncpy(proc_name, ptr+1, proc_name_size-1);
    proc_name[proc_name_size-1] = '\0';
#endif
  }

  QMI_DEBUG_MSG_2("linux_qmi_qmux_if_client_get_proc_name: pid=%d, proc_name=%s\n",
                  pid,
                  proc_name);

  rc = QMI_NO_ERR;

bail:
  close(fd);
  return rc;
}

/*===========================================================================
  FUNCTION  linux_qmi_qmux_if_client_get_proc_group_name
===========================================================================*/
/*!
@brief
  Returns the process group name corresponding to the give gid

@return
  QMI_NO_ERR on success
  QMI_INTERNAL_ERR otherwise

*/
/*=========================================================================*/
static int
linux_qmi_qmux_if_client_get_proc_group_name
(
  gid_t  gid,
  char   *grp_name,
  int    grp_name_size
)
{
  struct group *grp = NULL;
  int rc = QMI_INTERNAL_ERR;

  if (!grp_name || grp_name_size <= 0)
  {
    QMI_ERR_MSG_0("linux_qmi_qmux_if_client_get_proc_group_name: bad parameter(s)\n");
    goto bail;
  }

  grp = getgrgid(gid);

  if (!grp)
  {
    QMI_ERR_MSG_2("linux_qmi_qmux_if_client_get_proc_group_name: getgrname() failed errno[%d:%s]\n",
                  errno,
                  strerror(errno));
    goto bail;
  }

  QMI_DEBUG_MSG_2("linux_qmi_qmux_if_client_get_proc_group_name: gid=%d, grp_name=%s\n",
                  (int)gid,
                  grp->gr_name);

#ifdef FEATURE_QMI_ANDROID
    strlcpy(grp_name, grp->gr_name, (size_t)grp_name_size);
#else
    strncpy(grp_name, grp->gr_name, grp_name_size-1);
    grp_name[grp_name_size-1] = '\0';
#endif

  rc = QMI_NO_ERR;

bail:
  return rc;
}

/*===========================================================================
  FUNCTION  linux_qmi_qmux_if_client_get_socket_paths
===========================================================================*/
/*!
@brief
  Returns the client's bind socket path and the server connect socket path
  to use for the given pid

@return
  QMI_NO_ERR on success
  QMI_INTERNAL_ERR otherwise

*/
/*=========================================================================*/
static int
linux_qmi_qmux_if_client_get_socket_paths
(
  int        pid,
  const char **bind_sock_path,
  const char **conn_sock_path
)
{
  char name[50];
  int i, j;
  gid_t  *gids = NULL;
  int num_grps = 0;

  if (!bind_sock_path || !conn_sock_path)
  {
    QMI_ERR_MSG_0("linux_qmi_qmux_if_client_get_socket_paths: bad parameter(s)\n");
    return QMI_INTERNAL_ERR;
  }

  /* Default to the radio sockets */
  *bind_sock_path = QMI_QMUX_IF_CLIENT_SOCKET_PATH;
  *conn_sock_path = QMI_QMUX_IF_CONN_SOCKET_PATH;

  if (QMI_NO_ERR != linux_qmi_qmux_if_client_get_proc_name(pid,
                                                           name,
                                                           sizeof(name)))
  {
    QMI_ERR_MSG_1("linux_qmi_qmux_if_client_get_socket_paths: failed to obtain "
                  "proc name for pid=%d\n",
                  pid);
  }


#ifdef FEATURE_QMI_ANDROID
  /* Obtain the number of supplementary groups */
  num_grps = getgroups(0, NULL);

  if (num_grps < 0)
  {
    QMI_ERR_MSG_1("linux_qmi_qmux_if_client_get_socket_paths: getgroups() failed\n",
                  pid);
    goto bail;
  }

  /* Allocate memory for storing the supplementary gids + effective gid */
  ++num_grps;
  gids = malloc(sizeof(gid_t) * (size_t)num_grps);

  if (!gids)
  {
    QMI_ERR_MSG_3("linux_qmi_qmux_if_client_get_socket_paths: getgroups() failed errno=[%d:%s]\n",
                  pid,
                  errno,
                  strerror(errno));
    goto bail;
  }

  QMI_DEBUG_MSG_1("linux_qmi_qmux_if_client_get_socket_paths: num_grps=%d\n",
                  num_grps);

  gids[0] = getegid();

  if (getgroups(num_grps-1, &gids[1]) < 0)
  {
    QMI_ERR_MSG_3("linux_qmi_qmux_if_client_get_socket_paths: getgroups() failed errno=[%d:%s]\n",
                  pid,
                  errno,
                  strerror(errno));
    goto bail;
  }

  /* For each supplementary group the process belongs to */
  for (i = 0; i < num_grps; ++i)
  {
    if (QMI_NO_ERR != linux_qmi_qmux_if_client_get_proc_group_name(gids[i],
                                                                   name,
                                                                   sizeof(name)))
    {
      QMI_ERR_MSG_1("linux_qmi_qmux_if_client_get_socket_paths: failed to obtain "
                    "proc group name for pid=%d\n",
                    pid);
      continue;
    }

    /* Match the process' supplementary group with our table */
    for (j = 0; j < (int)LINUX_QMI_QMUX_IF_NUM_CLIENT_CONN_SOCK_ENTRIES; ++j)
    {
      size_t len_conn_sock_grp_name = strlen(linux_qmi_qmux_if_client_conn_socks[j].grp_name);
      size_t len_grp_name = strlen(name);

      if (0 == strncmp(linux_qmi_qmux_if_client_conn_socks[j].grp_name,
                       name,
                       MIN(len_conn_sock_grp_name, len_grp_name) + 1))
      {
        *bind_sock_path = linux_qmi_qmux_if_client_conn_socks[j].bind_sock_path;
        *conn_sock_path = linux_qmi_qmux_if_client_conn_socks[j].server_sock_path;
        goto bail;
      }
    }
  }

bail:
  if (NULL != gids)
  {
    free(gids);
  }
#endif
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  linux_qmi_qmux_if_reinit_qmux_conn
===========================================================================*/
/*!
@brief
  Reinitialize the connection with QMUXD and obtain a new QMUX client ID

@return
  QMI_NO_ERR on success
  QMI_INTERNAL_ERR on failure

@note

  - Side Effects
    - None

*/
/*=========================================================================*/
static int
linux_qmi_qmux_if_reinit_qmux_conn
(
  linux_qmi_qmux_if_client_data_t  *client_data,
  fd_set                           *master_fd_set,
  int                              *max_fd,
  qmi_qmux_clnt_id_t               *old_qmux_client_id
)
{
  int rc = QMI_INTERNAL_ERR;
  int ret;
  linux_qmi_qmux_if_client_data_t  *client = NULL, *prev = NULL;
  int client_fd = LINUX_QMI_INVALID_FD;
  qmi_qmux_clnt_id_t  qmux_client_id = QMI_QMUX_INVALID_QMUX_CLIENT_ID;

  if (!client_data || !master_fd_set || !max_fd)
  {
    QMI_ERR_MSG_0("linux_qmi_qmux_if_reinit_qmux_conn: invalid parameter\n");
    return rc;
  }

  /* Locate the client data */
  QMI_PLATFORM_MUTEX_LOCK (&linux_qmi_qmux_if_client_list_mutex);
  QMI_SLL_FIND(client,
               prev,
               linux_qmi_qmux_if_client_list,
               (client == client_data));
  if (client)
  {
    /* Find new max_fd */
    if (client->qmux_client_fd == *max_fd)
    {
      int i;
      for (i = 0; i < client->qmux_client_fd; i++)
      {
        if (FD_ISSET(i,master_fd_set))
        {
          *max_fd = i;
        }
      }
    }

    FD_CLR(client->qmux_client_fd, master_fd_set);

    close(client->qmux_client_fd);
    client->qmux_client_fd = LINUX_QMI_INVALID_FD;

    *old_qmux_client_id = client->qmux_client_id;
    client->qmux_client_id = QMI_QMUX_INVALID_QMUX_CLIENT_ID;
  }
  QMI_PLATFORM_MUTEX_UNLOCK (&linux_qmi_qmux_if_client_list_mutex);

  if (!client)
  {
    QMI_ERR_MSG_1("linux_qmi_qmux_if_reinit_qmux_conn: unable to locate client=0x%x\n",
                  client_data);
    goto bail;
  }

  client_fd = linux_qmi_qmux_if_connect();

  if (LINUX_QMI_INVALID_FD == client_fd)
  {
    QMI_ERR_MSG_0("linux_qmi_qmux_if_reinit_qmux_conn: unable to connect to qmux\n");
    goto bail;
  }

  /* Obtain the client ID from the QMUXD server */
  if (QMI_NO_ERR != linux_qmi_qmux_if_client_get_client_id(client_fd, &qmux_client_id) ||
      QMI_QMUX_INVALID_QMUX_CLIENT_ID == qmux_client_id)
  {
    QMI_ERR_MSG_2 ("qmi_client [%d] fd=%x: failed to obtain qmux_client_id",
                   linux_qmi_qmux_pid,
                   client_fd);
    goto bail;
  }

  QMI_DEBUG_MSG_3 ("qmi_client [%d] assigning new client_id=%x, fd=%x",
                   linux_qmi_qmux_pid,
                   qmux_client_id,
                   client_fd);

  client->qmux_client_fd = client_fd;
  client->qmux_client_id = qmux_client_id;

  FD_SET(client_fd, master_fd_set);
  if (client_fd > *max_fd)
  {
    *max_fd = client_fd;
  }

  rc = QMI_NO_ERR;

bail:
  return rc;
}

/*===========================================================================
  FUNCTION  linux_qmi_qmux_if_reset
===========================================================================*/
/*!
@brief
  Reset the QMUXD connection and send a Modem Out of Service followed by a
  Modem In Service system indication to the QMI QMUX IF layer

@return
  QMI_NO_ERR on success
  QMI_INTERNAL_ERR on failure

@note

  - Side Effects
    - None

*/
/*=========================================================================*/
static int
linux_qmi_qmux_if_reset
(
  linux_qmi_qmux_if_client_data_t  *client_data,
  fd_set                           *master_fd_set,
  int                              *max_fd
)
{
  unsigned char  buf [QMI_QMUX_IF_HDR_SIZE + sizeof (qmi_qmux_if_cmd_rsp_type)];
  qmi_qmux_if_msg_hdr_type  *hdr = NULL;
  qmi_qmux_if_cmd_rsp_type  *cmd = NULL;
  qmi_qmux_clnt_id_t  old_qmux_client_id = QMI_QMUX_INVALID_QMUX_CLIENT_ID;
  int rc = QMI_INTERNAL_ERR;
  int ret;

  union
  {
    unsigned char *uchar_ptr;
    qmi_qmux_if_msg_hdr_type *if_hdr_ptr;
  } tmp;

  if (!client_data || !master_fd_set || !max_fd)
  {
    QMI_ERR_MSG_0("linux_qmi_qmux_if_send_reset_msg: invalid parameters\n");
    return rc;
  }

  ret = QMI_PLATFORM_MUTEX_TRY_LOCK (&client_data->client_mutex);

  if (ret != 0)
  {
    QMI_ERR_MSG_1("linux_qmi_qmux_if_reinit_qmux_conn: failed to acquire mutex ret=%d\n",
                  ret);
    return rc;
  }

  if (QMI_NO_ERR != linux_qmi_qmux_if_reinit_qmux_conn(client_data,
                                                       master_fd_set,
                                                       max_fd,
                                                       &old_qmux_client_id))
  {
    QMI_ERR_MSG_0("linux_qmi_qmux_if_send_reset_msg: QMUX connection reinit failed\n");
    goto bail;
  }

  memset(buf, 0, sizeof(buf));

  tmp.uchar_ptr = buf;

  /* Send an Out of Service message */
  hdr = tmp.if_hdr_ptr;

  /* Set hdr values */
  hdr->msg_id = QMI_QMUX_IF_MODEM_OUT_OF_SERVICE_MSG_ID;
  hdr->qmux_client_id = old_qmux_client_id;
  hdr->qmux_txn_id = QMI_INVALID_TXN_ID;
  hdr->qmi_conn_id = QMI_CONN_ID_INVALID;
  hdr->qmi_service_id = (qmi_service_id_type) 0; /* Not used */
  hdr->qmi_client_id = 0; /* Not used */
  hdr->control_flags = 0; /* Not used */
  hdr->sys_err_code = QMI_NO_ERR;
  hdr->qmi_err_code = QMI_SERVICE_ERR_NONE;

  QMI_DEBUG_MSG_2("linux_qmi_qmux_if_reset_conn: sending OOS message [%d] %x\n",
                  linux_qmi_qmux_pid,
                  old_qmux_client_id);

  qmi_qmux_if_rx_msg (buf, sizeof(buf));

  /* Send an In Service message */
  hdr->msg_id = QMI_QMUX_IF_MODEM_IN_SERVICE_MSG_ID;
  cmd = (qmi_qmux_if_cmd_rsp_type *)(buf + QMI_QMUX_IF_HDR_SIZE);
  cmd->qmi_qmux_if_sub_sys_restart_ind.is_valid = TRUE;
  cmd->qmi_qmux_if_sub_sys_restart_ind.qmux_client_id = client_data->qmux_client_id;

  QMI_DEBUG_MSG_3("linux_qmi_qmux_if_reset_conn: sending IS message [%d] %x, new qmux_client_id=%x\n",
                  linux_qmi_qmux_pid,
                  old_qmux_client_id,
                  client_data->qmux_client_id);

  qmi_qmux_if_rx_msg (buf, sizeof(buf));

  rc = QMI_NO_ERR;

bail:
  QMI_PLATFORM_MUTEX_UNLOCK (&client_data->client_mutex);
  return rc;
}

static void *
linux_qmi_qmux_if_rx_msg
(
  void *param
)
{
  ssize_t buf_size;
  int zero_bytes_count = 0;
  int ret = 0;
  linux_qmi_qmux_if_platform_hdr_type   platform_msg_hdr;
  linux_qmi_qmux_if_client_data_t  *client_data = (linux_qmi_qmux_if_client_data_t *) param;
  int                              client_fd;
  qmi_qmux_clnt_id_t               qmux_client_id;
  linux_qmi_qmux_if_client_data_t  *client = NULL, *prev = NULL;
  fd_set                           select_fd_set;
  fd_set                           master_fd_set;
  int                              max_fd;

  if (!client_data)
  {
    QMI_ERR_MSG_1("linux_qmi_qmux_if_rx_msg: invalid parameter qmi_client [%d]\n",
                  linux_qmi_qmux_pid);
    goto bail;
  }

  client_fd = client_data->qmux_client_fd;
  qmux_client_id = client_data->qmux_client_id;

  FD_ZERO (&select_fd_set);
  FD_ZERO (&master_fd_set);

  FD_SET (client_fd,&master_fd_set);
  FD_SET (client_data->pipe_fds[READ_PIPE_FD_INDEX], &master_fd_set);

  max_fd = (client_data->pipe_fds[READ_PIPE_FD_INDEX] > client_fd) ?
            client_data->pipe_fds[READ_PIPE_FD_INDEX] : client_fd;

  for (;;)
  {
    select_fd_set = master_fd_set;

    if ((ret = select (max_fd + 1, &select_fd_set, NULL, NULL, NULL)) < 0)
    {
      QMI_ERR_MSG_4 ("qmi_client [%d] %x: select errno[%d:%s]\n",
                     linux_qmi_qmux_pid,
                     qmux_client_id,
                     errno,
                     strerror(errno));

      if (-1 == ret && EINTR == errno)
      {
        continue;
      }
      else
      {
        QMI_ERR_MSG_2 ("qmi_client [%d] %x: RX thread exiting\n",
                       linux_qmi_qmux_pid,
                       qmux_client_id);
        return (void*)-1;
      }
    }

    /* Locate the correponding client data */
    QMI_PLATFORM_MUTEX_LOCK (&linux_qmi_qmux_if_client_list_mutex);
    QMI_SLL_FIND(client,
                 prev,
                 linux_qmi_qmux_if_client_list,
                 (client->qmux_client_id == qmux_client_id));
    QMI_PLATFORM_MUTEX_UNLOCK (&linux_qmi_qmux_if_client_list_mutex);

    if (!client)
    {
      QMI_ERR_MSG_2("qmi_client [%d] %x: failed to locate client data",
                    linux_qmi_qmux_pid,
                    qmux_client_id);
      goto bail;
    }

    if (FD_ISSET (client->pipe_fds[READ_PIPE_FD_INDEX], &select_fd_set))
    {
      QMI_DEBUG_MSG_3 ("qmi_client [%d] %x: Read thread %u is terminating\n",
                       linux_qmi_qmux_pid,
                       qmux_client_id,
                       (unsigned int)client->th_id);
      goto bail;
    }

    /* Read in new message header */
    if ((buf_size = recv (client_fd,
                          (void *)&platform_msg_hdr,
                          QMI_QMUX_IF_PLATFORM_SPECIFIC_HDR_SIZE,
                          0)) < 0)
    {
      QMI_ERR_MSG_4 ("qmi_client [%d] %x: recv returns errno[%d:%s]\n",
                     linux_qmi_qmux_pid,
                     qmux_client_id,
                     errno,
                     strerror(errno));
      if (QMI_NO_ERR != linux_qmi_qmux_if_reset(client_data, &master_fd_set, &max_fd))
      {
        goto bail;
      }

      client_fd = client_data->qmux_client_fd;
      qmux_client_id = client_data->qmux_client_id;
      continue;
    }

    if (buf_size == 0)
    {
      /* Limit number of debugging messages regarding 0 length buffers to 5 before we surpress them */
      if (zero_bytes_count < LINUX_QMI_MAX_RETRIES)
      {
        QMI_DEBUG_MSG_4 ("qmi_client [%d] %x: received %d bytes on fd = %d\n",
                         linux_qmi_qmux_pid,
                         client->qmux_client_id,
                         (int)buf_size,
                         client->qmux_client_fd);
        zero_bytes_count++;
        usleep(LINUX_QMI_RETRY_DELAY);
      }
      else
      {
        if (QMI_NO_ERR != linux_qmi_qmux_if_reset(client_data, &master_fd_set, &max_fd))
        {
          goto bail;
        }

        client_fd = client_data->qmux_client_fd;
        qmux_client_id = client_data->qmux_client_id;
        continue;
      }
    }
    else
    {
      /* If we surpressed the 0 length debug messages reset the surpressions */
      if (zero_bytes_count >= LINUX_QMI_MAX_RETRIES)
      {
        QMI_DEBUG_MSG_3 ("qmi_client [%d] %x: Resetting zero_bytes_count message surpression for fd = %d\n",
                         linux_qmi_qmux_pid,
                         client->qmux_client_id,
                         client_fd);
        zero_bytes_count = 0;
      }

      /* Now process header */
      if (buf_size != QMI_QMUX_IF_PLATFORM_SPECIFIC_HDR_SIZE)
      {
        QMI_ERR_MSG_4 ("qmi_client [%d] %x: RX on fd=%d didn't return enough header data, size received=%d\n",
                       linux_qmi_qmux_pid,
                       client->qmux_client_id,
                       client_fd,
                       (int)buf_size);
      }
      else
      {
        size_t remaining_bytes = (size_t) platform_msg_hdr.total_msg_size - QMI_QMUX_IF_PLATFORM_SPECIFIC_HDR_SIZE;

        QMI_DEBUG_MSG_4 ("qmi_client [%d] %x: Received %d bytes on fd = %d\n",
                         linux_qmi_qmux_pid,
                         client->qmux_client_id,
                         platform_msg_hdr.total_msg_size,
                         client_fd);

        /* If message is larger than we can handle, than print error message and read/discard message */
        if (remaining_bytes > (size_t) client->rx_buf_size)
        {
          QMI_ERR_MSG_4 ("qmi_client %x: RX on fd=%d, msg size=%d is too big "
                         "for buffer size = %d, DISCARDING!!...\n",
                         client->qmux_client_id,
                         client_fd,
                         (int)remaining_bytes,
                         client->rx_buf_size);

          /* Discard all of the data in the message */
          while (remaining_bytes > 0 )
          {
            if ((buf_size = recv (client_fd,
                                  (void *)client->rx_buf,
                                  (size_t) MIN (remaining_bytes,(size_t) client->rx_buf_size),
                                  0)) <= 0)
            {
              QMI_ERR_MSG_4 ("qmi_client [%d] %x: RX on fd=%d returned error=%d\n",
                             linux_qmi_qmux_pid,
                             client->qmux_client_id,
                             client_fd,
                             (int)buf_size);
              break;
            }
            else
            {
              remaining_bytes -= (size_t)buf_size;
            }
          }
        }
        else /* Message looks good... read in and process as normal */
        {
          if ((buf_size = recv (client_fd,
                                (void *)client->rx_buf,
                                remaining_bytes,
                                0)) <= 0)
          {
            QMI_ERR_MSG_4 ("qmi_client [%d] %x: RX on fd=%d returned error=%d\n",
                           linux_qmi_qmux_pid,
                           client->qmux_client_id,
                           client_fd,
                           (int)buf_size);
          }
          else if (buf_size != (ssize_t) remaining_bytes)
          {
            QMI_ERR_MSG_3 ("qmi_client [%d] %x: RX on fd=%d didn't return enough msg data\n",
                           linux_qmi_qmux_pid,
                           client->qmux_client_id,
                           client_fd);
            QMI_ERR_MSG_2 (" size rx'd=%d, expected=%d\n", (int)buf_size, (int)remaining_bytes);
          }
          else
          {
            /* Process message */
            qmi_qmux_if_rx_msg (client->rx_buf,(int)remaining_bytes);
          }
        }
      }
    }
  }

bail:
  return 0;
}

static int linux_qmi_qmux_if_client_get_client_id
(
  int                 client_fd,
  qmi_qmux_clnt_id_t  *qmux_client_id
)
{
  int rc = QMI_INTERNAL_ERR;

  if (!qmux_client_id)
  {
    QMI_ERR_MSG_1 ("linux_qmi_qmux_if_client_get_client_id [%d]: bad param",
                   linux_qmi_qmux_pid);
    goto bail;
  }

  /* Receive the qmux_client_id from the QMUXD server */
  if (QMI_QMUX_CLIENT_ID_SIZE != recv (client_fd,
                                       (void *)qmux_client_id,
                                       QMI_QMUX_CLIENT_ID_SIZE,
                                       0))
  {
    QMI_ERR_MSG_3 ("linux_qmi_qmux_if_client_get_client_id [%d]: recv failed. [%d:%s]",
                   linux_qmi_qmux_pid, errno, strerror(errno));
    goto bail;
  }

  QMI_DEBUG_MSG_2 ("linux_qmi_qmux_if_client_get_client_id [%d]: qmux_client_id=%x",
                   linux_qmi_qmux_pid,
                   *qmux_client_id);
  rc = QMI_NO_ERR;

bail:
  return rc;
}

/*===========================================================================
  FUNCTION  linux_qmi_qmux_if_connect
===========================================================================*/
/*!
@brief
  Establish a connection with QMUXD

@param
  None

@note

  - Side Effects
    - None

*/
/*=========================================================================*/
static int
linux_qmi_qmux_if_connect(void)
{
  struct sockaddr_un server_addr, client_addr;
  int  client_fd = LINUX_QMI_INVALID_FD;
  const char *bind_sock_path = NULL;
  const char *conn_sock_path = NULL;
  int  ret_fd = LINUX_QMI_INVALID_FD;
  int  rc, i;
  socklen_t len;
  qmi_platform_sockaddr_type tmp;

  /* Initialize the addr variables */
  memset (&server_addr,0,sizeof (struct sockaddr_un));
  memset (&client_addr,0,sizeof (struct sockaddr_un));

  linux_qmi_qmux_pid = (int) getpid();

  /* Get the connection listener socket */
  if ((client_fd = socket (AF_UNIX,SOCK_STREAM,0)) < 0)
  {
    QMI_ERR_MSG_2 ("qmi_client [%d]: unable to open client socket, rc = %d\n",
                   linux_qmi_qmux_pid,
                   client_fd);
    goto bail;
  }

  if (QMI_NO_ERR != linux_qmi_qmux_if_client_get_socket_paths(linux_qmi_qmux_pid,
                                                              &bind_sock_path,
                                                              &conn_sock_path))
  {
    bind_sock_path = QMI_QMUX_IF_CLIENT_SOCKET_PATH;
    conn_sock_path = QMI_QMUX_IF_CONN_SOCKET_PATH;
  }

  QMI_DEBUG_MSG_2 ("using socket paths bind=%s, connect=%s\n",
                   bind_sock_path,
                   conn_sock_path);

  client_addr.sun_family = AF_UNIX;
  snprintf (client_addr.sun_path,
            sizeof(client_addr.sun_path),
            "%s%7d",
            bind_sock_path,
            gettid());

  len = (socklen_t)(offsetof (struct sockaddr_un, sun_path) + strlen (client_addr.sun_path));


  /* Delete file in case it exists */
  unlink (client_addr.sun_path);


  tmp.sunaddr = &client_addr;

  /* Bind socket to address */
  if ((rc = bind (client_fd, tmp.saddr, sizeof(struct sockaddr_un))) < 0)
  {
    QMI_ERR_MSG_4 ("qmi_client [%d]: unable to bind to client socket, rc = %d errno[%d:%s]\n",
                   linux_qmi_qmux_pid,
                   rc,
                   errno,
                   strerror(errno));
    goto bail;
  }

  /* Allow RW permissions only for user and group */
  if (-1 == chmod(client_addr.sun_path, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP))
  {
    QMI_ERR_MSG_4 ("qmi_client [%d]: unable to chmod bind_sock_path=%s, errno=[%d:%s]\n",
                   linux_qmi_qmux_pid,
                   client_addr.sun_path,
                   errno,
                   strerror(errno));
  }

  server_addr.sun_family = AF_UNIX;

  snprintf (server_addr.sun_path,
            sizeof(server_addr.sun_path),
            "%s",
            conn_sock_path);

  len = (socklen_t)(offsetof (struct sockaddr_un, sun_path) + strlen (server_addr.sun_path));

  tmp.sunaddr = &server_addr;

  /* Connect to the server's connection socket */
  for (i = 0; i < LINUX_QMI_MAX_CONNECT_TRIES; i++)
  {
    if ((rc = connect (client_fd, tmp.saddr, len)) < 0)
    {
      QMI_ERR_MSG_4 ("qmi_client [%d]: unable to connect to server, errno=[%d:%s], attempt=%d\n",
                     linux_qmi_qmux_pid,
                     errno,
                     strerror(errno),
                     i+1);
      sleep (1);
    }
    else
    {
      QMI_DEBUG_MSG_2 ("qmi_client [%d]: successfully connected to server, attempt=%d\n",
                       linux_qmi_qmux_pid,
                       i+1);
      break;
    }
  }

  if (rc < 0)
  {
    QMI_ERR_MSG_2 ("qmi_client [%d]: unable to connect to server after %d tries... giving up\n",
                   linux_qmi_qmux_pid,
                   i);
    goto bail;
  }

  ret_fd = client_fd;

bail:
  if (LINUX_QMI_INVALID_FD == ret_fd &&
      LINUX_QMI_INVALID_FD != client_fd)
  {
    /* Delete file in case it exists */
    unlink (client_addr.sun_path);
    close(client_fd);
  }

  return ret_fd;
}

/*===========================================================================
                          GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

int linux_qmi_qmux_if_client_init
(
  qmi_qmux_clnt_id_t  *qmux_client_id,
  unsigned char       *rx_buf,
  int                 rx_buf_size
)
{
  static int diag_inited = FALSE;
  int ret = QMI_INTERNAL_ERR;
  int i;
  linux_qmi_qmux_if_client_data_t  *client_data = NULL;
  int  client_fd = LINUX_QMI_INVALID_FD;

  QMI_PLATFORM_MUTEX_LOCK (&linux_qmi_qmux_if_platform_mutex);

    /*Initialize Diag services*/
#ifdef FEATURE_DATA_LOG_QXDM
  if (diag_inited == FALSE)
  {
    boolean ret_val = FALSE;
    ret_val = Diag_LSM_Init(NULL);
    qmi_platform_qxdm_init = ret_val;
    if ( !ret_val )
    {
      QMI_ERR_MSG_0 ("Failed on DIAG init\n");
    }
    diag_inited = TRUE;
  }
#endif

#ifdef FEATURE_QMI_ANDROID
  /* If property is not set, we will log only ERROR messages */
  qmi_log_adb_level = QMI_LOG_ADB_LEVEL_ERROR;
  char property[PROPERTY_VALUE_MAX];
  if (property_get(QMI_LOG_ADB_PROP, property, NULL) > 0)
  {
    qmi_log_adb_level = atoi(property);
  }
  if ((qmi_log_adb_level < QMI_LOG_ADB_LEVEL_NONE) || (qmi_log_adb_level > QMI_LOG_ADB_LEVEL_ALL))
  {
    qmi_log_adb_level = QMI_LOG_ADB_LEVEL_ERROR;
  }
#endif

  client_fd = linux_qmi_qmux_if_connect();

  if (LINUX_QMI_INVALID_FD == client_fd)
  {
    goto init_exit;
  }

  /* Obtain the client ID from the QMUXD server */
  if (QMI_NO_ERR != linux_qmi_qmux_if_client_get_client_id(client_fd, qmux_client_id))
  {
    QMI_ERR_MSG_2 ("qmi_client [%d] fd=%x: failed to obtain qmux_client_id",
                   linux_qmi_qmux_pid,
                   client_fd);
    goto init_exit;
  }

  /* Allocate storage for client specific data */
  client_data = malloc (sizeof (linux_qmi_qmux_if_client_data_t));

  /* Make sure memory allocation succeeds */
  if (!client_data)
  {
    QMI_DEBUG_MSG_0 ("linux_qmi_qmux_if_client_init:  Client data malloc failed\n");
    QMI_ERR_MSG_3 ("qmi_client [%d] %x: failed to alloc client data for fd=%d",
                   linux_qmi_qmux_pid,
                   *qmux_client_id,
                   client_fd);
    goto init_exit;
  }

  /* Save client data */
  client_data->qmux_client_fd = client_fd;
  client_data->qmux_client_id = *qmux_client_id;
  client_data->rx_buf         = rx_buf;
  client_data->rx_buf_size    = rx_buf_size;
  client_data->pipe_fds[0]    = LINUX_QMI_INVALID_FD;
  client_data->pipe_fds[1]    = LINUX_QMI_INVALID_FD;
  QMI_PLATFORM_MUTEX_INIT(&client_data->client_mutex);

  QMI_DEBUG_MSG_2 ("qmi_client [%d] %x: qmux_client ID is initialized\n",
                   linux_qmi_qmux_pid,
                   client_data->qmux_client_id);

  /* Spawn receive message thread */
  /* Create pipe that will be used to tell reader thread when to terminate */
  if (pipe (client_data->pipe_fds) < 0)
  {
    QMI_ERR_MSG_4  ("qmi_client [%d] %x: pipe() system call  returns errno[%d:%s]\n",
                    linux_qmi_qmux_pid,
                    client_data->qmux_client_id,
                    errno,
                    strerror(errno));
    goto init_exit;
  }
  else
  {
    QMI_DEBUG_MSG_4  ("qmi_client [%d] %x: pipe() system call, fd[0]=%d, fd[1]=%d\n",
                      linux_qmi_qmux_pid,
                      client_data->qmux_client_id,
                      client_data->pipe_fds[0],
                      client_data->pipe_fds[1]);
  }

  if ((pthread_create (&client_data->th_id,
                       NULL,
                       linux_qmi_qmux_if_rx_msg,
                       (void *)client_data)) != 0)
  {
    QMI_ERR_MSG_2 ("qmi_client [%d] %x:  can't create RX thread",
                   linux_qmi_qmux_pid,
                   client_data->qmux_client_id);
    goto init_exit;
  }

  /* Lock list mutex, add new client data, and then unlock */
  QMI_PLATFORM_MUTEX_LOCK (&linux_qmi_qmux_if_client_list_mutex);
  QMI_SLL_ADD (client_data, linux_qmi_qmux_if_client_list);
  QMI_PLATFORM_MUTEX_UNLOCK (&linux_qmi_qmux_if_client_list_mutex);

  ret = QMI_NO_ERR;

init_exit:
  if (ret != QMI_NO_ERR)
  {
    if (LINUX_QMI_INVALID_FD != client_fd)
    {
      close(client_fd);
    }
    if (client_data)
    {
      if (LINUX_QMI_INVALID_FD != client_data->pipe_fds[0])
      {
        close(client_data->pipe_fds[0]);
      }
      if (LINUX_QMI_INVALID_FD != client_data->pipe_fds[1])
      {
        close(client_data->pipe_fds[1]);
      }
      free(client_data);
    }
  }

  QMI_PLATFORM_MUTEX_UNLOCK (&linux_qmi_qmux_if_platform_mutex);
  return ret;
}

int linux_qmi_qmux_if_client_tx_msg
(
  qmi_qmux_clnt_id_t  qmux_client_id,
  unsigned char       *msg,
  int                 msg_len
)
{
  linux_qmi_qmux_if_platform_hdr_type *p;
  int  ret = QMI_INTERNAL_ERR;
  ssize_t rc;
  linux_qmi_qmux_if_client_data_t  *client = NULL, *prev = NULL;
  int  client_fd = LINUX_QMI_INVALID_FD;

  /* Locate the client data */
  QMI_PLATFORM_MUTEX_LOCK (&linux_qmi_qmux_if_client_list_mutex);
  QMI_SLL_FIND(client,
               prev,
               linux_qmi_qmux_if_client_list,
               (client->qmux_client_id == qmux_client_id));
  if (client)
  {
    client_fd = client->qmux_client_fd;
  }
  QMI_PLATFORM_MUTEX_UNLOCK (&linux_qmi_qmux_if_client_list_mutex);

  /* Make sure that we have a valid entry for the given qmux_client_id */
  if (LINUX_QMI_INVALID_FD == client_fd)
  {
    QMI_DEBUG_MSG_2 ("linux_qmi_qmux_if_client_tx_msg qmi_client [%d] %x: "
                     "unable to locate client data\n",
                     linux_qmi_qmux_pid,
                     qmux_client_id);
    goto bail;
  }

  msg -= QMI_QMUX_IF_PLATFORM_SPECIFIC_HDR_SIZE;
  msg_len += (int)QMI_QMUX_IF_PLATFORM_SPECIFIC_HDR_SIZE;
  p = (linux_qmi_qmux_if_platform_hdr_type *) msg;
  p->total_msg_size = msg_len;
  p->qmux_client_id = qmux_client_id;

  QMI_DEBUG_MSG_4 ("qmi_client [%d] %x: sending %d bytes on fd = %d\n",
                   linux_qmi_qmux_pid,
                   qmux_client_id,
                   msg_len,
                   client_fd);

  if ((rc = send (client_fd,
                  (void *) msg,
                  (size_t) msg_len,
                  MSG_DONTWAIT | MSG_NOSIGNAL)) < 0)
  {
    QMI_ERR_MSG_3 ("qmi_client [%d] %x:  send error = %d\n",
                   linux_qmi_qmux_pid,
                   qmux_client_id,
                   rc);
    goto bail;
  }

  ret = QMI_NO_ERR;

bail:
  return ret;
}


int
linux_qmi_qmux_if_client_release
(
  qmi_qmux_clnt_id_t  qmux_client_id
)
{
  int rc = QMI_NO_ERR;
  linux_qmi_qmux_if_client_data_t  client_data;
  linux_qmi_qmux_if_client_data_t  *client = NULL, *prev = NULL;
  char  buf[1] = {'1'};

  QMI_PLATFORM_MUTEX_LOCK (&linux_qmi_qmux_if_platform_mutex);

  /* Locate the client data */
  QMI_PLATFORM_MUTEX_LOCK (&linux_qmi_qmux_if_client_list_mutex);
  QMI_SLL_FIND(client,
               prev,
               linux_qmi_qmux_if_client_list,
               (client->qmux_client_id == qmux_client_id));
  QMI_PLATFORM_MUTEX_UNLOCK (&linux_qmi_qmux_if_client_list_mutex);

  /* Make sure that we have a valid entry for the given qmux_client_id */
  if (!client)
  {
    QMI_DEBUG_MSG_2 ("qmi_client [%d] %x: release - unable to locate client data\n",
                     linux_qmi_qmux_pid,
                     qmux_client_id);
    rc = QMI_INTERNAL_ERR;
    goto bail;
  }
  else
  {
    QMI_PLATFORM_MUTEX_LOCK (&client->client_mutex);
  }

  QMI_PLATFORM_MUTEX_LOCK (&linux_qmi_qmux_if_client_list_mutex);
  QMI_SLL_FIND_AND_REMOVE(client,
                          prev,
                          linux_qmi_qmux_if_client_list,
                          (client->qmux_client_id == qmux_client_id));
  QMI_PLATFORM_MUTEX_UNLOCK (&linux_qmi_qmux_if_client_list_mutex);


  QMI_DEBUG_MSG_3("qmi_client [%d] %x: Client release, writing pipe to kill read thread, fd = %d\n",
                  linux_qmi_qmux_pid,
                  qmux_client_id,
                  client->pipe_fds[WRITE_PIPE_FD_INDEX]);

  /* write to the pipe is ths indication to the read thread to terminate */
  if (write (client->pipe_fds[WRITE_PIPE_FD_INDEX],buf,sizeof(buf)) < 0)
  {
    QMI_ERR_MSG_4  ("qmi_client %x: write call fails for linux_qmi_qmux_if_th_id=%u returns errno[%d:%s]\n",
                    qmux_client_id,
                    (unsigned int)client->th_id,
                    errno,
                    strerror(errno));
    rc = QMI_INTERNAL_ERR;
  }
  else if (pthread_join (client->th_id, NULL) != 0)
  {
    QMI_ERR_MSG_4  ("qmi_client %x: pthread_join of linux_qmi_qmux_if_th_id=%u returns errno[%d:%s]\n",
                    qmux_client_id,
                    (unsigned int)client->th_id,
                    errno,
                    strerror(errno));
    rc = QMI_INTERNAL_ERR;
  }

  if (close (client->pipe_fds[WRITE_PIPE_FD_INDEX]) != 0)
  {
    QMI_ERR_MSG_4  ("qmi_client %d: write pipe fd close of fd=%d returns errno[%d:%s]\n",
                    qmux_client_id,
                    client->pipe_fds[WRITE_PIPE_FD_INDEX],
                    errno,
                    strerror(errno));
    rc = QMI_INTERNAL_ERR;
  }

  /* Close read end of pipe, as well as sockets fd */
  if (close (client->pipe_fds[READ_PIPE_FD_INDEX]) != 0)
  {
    QMI_ERR_MSG_4  ("qmi_client %x: read pipe fd close of fd=%d returns errno[%d:%s]\n",
                    qmux_client_id,
                    client->pipe_fds[READ_PIPE_FD_INDEX],
                    errno,
                    strerror(errno));
    rc = QMI_INTERNAL_ERR;
  }

  QMI_DEBUG_MSG_3 ("qmi_client [%d] %x: calling release of fd=%d\n",
                   linux_qmi_qmux_pid,
                   qmux_client_id,
                   client->qmux_client_fd);

  if (close (client->qmux_client_fd) != 0)
  {
    QMI_ERR_MSG_4  ("qmi_client %x: close of fd=%d returns errno[%d:%s]\n",
                    linux_qmi_qmux_pid,
                    client->qmux_client_fd,
                    errno,
                    strerror(errno));
    rc = QMI_INTERNAL_ERR;
  }

  QMI_PLATFORM_MUTEX_UNLOCK(&client->client_mutex);
  QMI_PLATFORM_MUTEX_DESTROY(&client->client_mutex);
  free(client);

bail:
  QMI_PLATFORM_MUTEX_UNLOCK (&linux_qmi_qmux_if_platform_mutex);
  return rc;
}

