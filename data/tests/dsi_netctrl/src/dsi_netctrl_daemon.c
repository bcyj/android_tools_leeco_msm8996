/******************************************************************************

                        D S I _ N E T C T R L _ D A E M O N . C

******************************************************************************/

/******************************************************************************

  @file    dsi_netctrl_daemon.c
  @brief   dsi_netctrl test daemon

  DESCRIPTION
  Daemon that that allows making data calls using dsi_netctrl

  ---------------------------------------------------------------------------
  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id: $

when       who        what, where, why
--------   ---        -------------------------------------------------------
10/07/10   jf         created

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <ds_string.h>
#include <errno.h>

#include "qmi_wds_srvc.h"
#include "dsi_netctrl_test.h"
#include "qdp.h"

#define LOG(...) if (debug) printf( __VA_ARGS__ );

#define WRITE_STR(fd, str) write(fd, str, std_strlen(str))
#define DSI_NETCTRL_TEST_RESPONSE_LENGTH 128

/* Start/end data call wait time in seconds */
#define DSI_NETCTRL_TEST_WAIT_TIME 10

#define DSI_NETCTRL_TEST_MAX_CALLS 12
#define DSI_NETCTRL_TEST_MAX_PROFILES 12

typedef struct
{
  char name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH];
  char* profile[QDP_RIL_PARAM_MAX];
  unsigned int cdma_profile_id;
  qdp_profile_pdn_type  cdma_profile_pdn;
  unsigned int umts_profile_id;
  qdp_profile_pdn_type  umts_profile_pdn;
} dsi_net_profile_info;

dsi_net_profile_info dsi_net_profiles[DSI_NETCTRL_TEST_MAX_PROFILES];

typedef struct
{
  dsi_hndl_t handle;
  dsi_test_call_status_e status;
  char name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH];

  pthread_cond_t cond;
  pthread_mutex_t mutx;
} dsi_net_call_info;

/* data for each call */
dsi_net_call_info dsi_net_calls[DSI_NETCTRL_TEST_MAX_CALLS];

/* Flag that keeps the daemon alive until it is told to die */
int alive = 1;

/* Flag that keeps the process from daemonizing. Output logs to stderr */
int debug = 0;

int qdp_inited = 0;

/* QMI client handles */
int qmi_handle = QMI_INVALID_CLIENT_HANDLE;

void dsi_callback(dsi_hndl_t hndl, void* user_data,
                  dsi_net_evt_t event, dsi_evt_payload_t *payload_ptr )
{
  dsi_net_call_info *info = user_data;
  (void)hndl;

  if (!info)
    return;

  LOG("call=%s event=%d payload=%p\n", info->name, event, payload_ptr);

  pthread_mutex_lock(&info->mutx);

  switch (event)
  {
  case DSI_EVT_NET_NO_NET:
    info->status = DSI_TEST_CALL_STATUS_IDLE;
    break;
  case DSI_EVT_NET_IS_CONN:
    info->status = DSI_TEST_CALL_STATUS_CONNECTED;
    break;

  default:
    goto unlock;
  }

  pthread_cond_signal(&info->cond);

unlock:
  pthread_mutex_unlock(&info->mutx);

}

dsi_net_profile_info* find_profile(const char* name)
{
  int i;
  dsi_net_profile_info* ret = NULL;

  for (i = 0; i < DSI_NETCTRL_TEST_MAX_PROFILES; i++)
  {
    if (std_stricmp(name, dsi_net_profiles[i].name) == 0)
    {
      ret = &dsi_net_profiles[i];
      break;
    }
  }

  LOG("profile %s is %p\n", name, ret);
  return ret;
}

void create_profile(int fd, const char* name)
{
  int i;
  int index = -1;

  for (i = 0; i < DSI_NETCTRL_TEST_MAX_PROFILES; i++)
  {
    if (std_strlen(dsi_net_profiles[i].name) == 0)
    {
      index = i;
      break;
    }
  }

  if (index < 0)
  {
    WRITE_STR(fd, "Maximum profiles reached\n");
    return;
  }

  memset(&dsi_net_profiles[index], 0, sizeof(dsi_net_profiles[index]));
  strlcpy(dsi_net_profiles[index].name, name, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
}

void release_profile(int fd, dsi_net_profile_info* profile)
{
  int i;

  if (profile->cdma_profile_id != 0)
  {
    if (qdp_profile_release(profile->cdma_profile_id) != QDP_SUCCESS)
    {
      WRITE_STR(fd, "Failed to release CDMA profile.\n");
    }
  }

  if (profile->umts_profile_id != 0)
  {
    if (qdp_profile_release(profile->umts_profile_id) != QDP_SUCCESS)
    {
      WRITE_STR(fd, "Failed to release UMTS profile.\n");
    }
  }

  for (i = 0; i < QDP_RIL_PARAM_MAX; i++)
  {
    free(profile->profile[i]);
  }

  memset(profile, 0, sizeof(*profile));
}

void set_profile_param(int fd, dsi_net_profile_info* profile, qdp_ril_param_idx_t identifier, const char* val)
{
  if (identifier >= QDP_RIL_PARAM_MAX)
  {
    WRITE_STR(fd, "Invalid identifier\n");
    return;
  }

  /* Delete the old value if the parameter has been set multiple times. */
  free(profile->profile[identifier]);
  profile->profile[identifier] = NULL;

  profile->profile[identifier] = strdup(val);

  if (!profile->profile[identifier])
  {
    WRITE_STR(fd, "Alloc failed.\n");
  }
}

void lookup_profile(int fd, dsi_net_profile_info* profile)
{
  char* result = NULL;
  boolean abort_call = FALSE;
  qdp_error_info_t error_info;

  if( QDP_SUCCESS != qdp_profile_look_up( (const char**) profile->profile,
                                          &profile->umts_profile_id,
                                          &profile->umts_profile_pdn,
                                          &profile->cdma_profile_id,
                                          &profile->cdma_profile_pdn,
                                          &error_info ) )
  {
    /* Don't write to fd to indicate error */
    return;
  }

  if (asprintf(&result,
               "%d %d\n",
               profile->umts_profile_id,
               profile->cdma_profile_id) != -1)
  {
    WRITE_STR(fd, result);
  }

  if (NULL != result) {
      free(result);
  }
}

dsi_net_call_info* find_call(const char* name)
{
  int i;
  dsi_net_call_info* ret = NULL;

  for (i = 0; i < DSI_NETCTRL_TEST_MAX_CALLS; i++)
  {
    if (std_stricmp(name, dsi_net_calls[i].name) == 0)
    {
      ret = &dsi_net_calls[i];
      break;
    }
  }

  return ret;
}

const char* status_to_string(dsi_test_call_status_e status)
{
  const char* str = NULL;

  switch (status)
  {
  case DSI_TEST_CALL_STATUS_IDLE:
    str = "idle";
    break;

  case DSI_TEST_CALL_STATUS_CONNECTING:
    str = "connecting";
    break;

  case DSI_TEST_CALL_STATUS_CONNECTED:
    str = "connected";
    break;

  case DSI_TEST_CALL_STATUS_DISCONNECTING:
    str = "disconnecting";
    break;

  default:
    break;
  }

  return str;
}

void list_calls(int fd)
{
  int i;
  char buf[256];
  int call_exists = 0;

  for (i = 0; i < DSI_NETCTRL_TEST_MAX_CALLS; i++)
  {
    if (dsi_net_calls[i].handle != NULL)
    {
      snprintf(buf, sizeof(buf), "Call: <%s> status: %s\n",
                    dsi_net_calls[i].name,
                    status_to_string(dsi_net_calls[i].status));
      WRITE_STR(fd, buf);
      call_exists = 1;
    }
  }

  if (!call_exists)
  {
    WRITE_STR(fd, "No calls created yet.\n");
  }
}

void get_device(dsi_net_call_info* call, int fd)
{
  /* max length, new line and null terminator */
  char device[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2];

  if (dsi_get_device_name(call->handle, device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1) == DSI_SUCCESS)
  {
    LOG("Device name is \"%s\"\n", device);
    strlcat(device, "\n", DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2);
    WRITE_STR(fd, device);
  }
}

void wds_ind_cb(int wds_hndl,
                qmi_service_id_type sid,
                void* user_data,
                qmi_wds_indication_id_type ind_id,
                qmi_wds_indication_data_type* ind_data)
{
  (void)wds_hndl; (void)sid; (void)user_data; (void)ind_id; (void)ind_data;
}

void get_tech(dsi_net_call_info* call, int fd)
{
  int wds_handle;
  int qmi_err;
  int ret;
  qmi_wds_data_bearer_tech_type bearer_info;
  char device[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1];

  if (dsi_get_device_name(call->handle, device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1) != DSI_SUCCESS)
  {
    /* Error getting the device name.  Cannot complete request. */
    LOG("Could not get device name from dsi\n");
    return;
  }

  wds_handle = qmi_wds_srvc_init_client(device, wds_ind_cb, NULL, &qmi_err);

  if (wds_handle < 0)
  {
    LOG("Could not initalize qmi wds service\n");
    return;
  }

  ret = qmi_wds_get_current_bearer_tech(wds_handle, &bearer_info, &qmi_err);

  if (ret >= 0)
  {
    /* Got the bearer tech.  Interpret the result and write to client file descriptor */
    switch (bearer_info.current_db_nw)
    {
    case QMI_WDS_CDMA_TYPE:
      WRITE_STR(fd, "CDMA");

      switch (bearer_info.rat_mask.cdma_rat_mask)
      {
      case CDMA_1X:
        WRITE_STR(fd, " 1x");

        switch (bearer_info.db_so_mask.so_mask_1x)
        {
        case CDMA_1X_IS95:
          WRITE_STR(fd, " IS95");
          break;

        case CDMA_1X_IS2000:
          WRITE_STR(fd, " IS2000");
          break;

        case CDMA_1X_IS2000_REL_A:
          WRITE_STR(fd, " IS2000_RELA");
          break;

        case CDMA_1X_DONT_CARE:
        default:
          break;
        }

        break;

      case CDMA_EVDO_REV0:
        WRITE_STR(fd, " EVDO REV0");
        break;

      case CDMA_EVDO_REVA:
        WRITE_STR(fd, " EVDO REVA");

        switch (bearer_info.db_so_mask.so_mask_evdo_reva)
        {
        case CDMA_EVDO_REVA_DPA:
          WRITE_STR(fd, " DPA");
          break;

        case CDMA_EVDO_REVA_MFPA:
          WRITE_STR(fd, " MFPA");
          break;

        case CDMA_EVDO_REVA_EMPA:
          WRITE_STR(fd, " EMPA");
          break;

        case CDMA_EVDO_DONT_CARE:
        default:
          break;
        }

        break;

      case CDMA_DONT_CARE:
      default:
        break;
      }

      WRITE_STR(fd, "\n");
      break;

    case QMI_WDS_UMTS_TYPE:
      WRITE_STR(fd, "UMTS");

      switch (bearer_info.rat_mask.umts_rat_mask)
      {
      case UMTS_WCDMA:
        WRITE_STR(fd, " WCDMA");
        break;

      case UMTS_GPRS:
        WRITE_STR(fd, " GPRS");
        break;

      case UMTS_HSDPA:
        WRITE_STR(fd, " HSDPA");
        break;

      case UMTS_HSUPA:
        WRITE_STR(fd, " HSUPA");
        break;

      case UMTS_EDGE:
        WRITE_STR(fd, " EDGE");
        break;

      case UMTS_DONT_CARE:
      default:
        break;
      }

      WRITE_STR(fd, "\n");
      break;


    case QMI_WDS_UNKNOWN_TYPE:
    default:
      break;
    }
  }
  else
  {
    LOG("qmi_wds_get_current_bearer_tech failed with [%d] qmi_err = [%d]\n", ret, qmi_err);
  }

  qmi_wds_srvc_release_client(wds_handle, &qmi_err);
}

void net_up(dsi_net_call_info* call, int fd)
{
  int failed = 0;
  struct timespec  tspec;

  if (call->status != DSI_TEST_CALL_STATUS_IDLE)
  {
    WRITE_STR(fd, "Call up operation failed.  Call not in idle state.\n");
    return;
  }

  /* set the call status before starting asynchronous operation */
  call->status = DSI_TEST_CALL_STATUS_CONNECTING;

  /* start the data call */
  if (dsi_start_data_call(call->handle) != DSI_SUCCESS)
  {
    call->status = DSI_TEST_CALL_STATUS_IDLE;
    WRITE_STR(fd, "Error starting data call.\n");
    return;
  }

  /* wait until the call status changes */
  pthread_mutex_lock(&call->mutx);

  if (call->status == DSI_TEST_CALL_STATUS_CONNECTING)
  {
    clock_gettime(CLOCK_REALTIME, &tspec);
    tspec.tv_sec += DSI_NETCTRL_TEST_WAIT_TIME;
    pthread_cond_timedwait(&call->cond, &call->mutx, &tspec);
  }

  if (call->status != DSI_TEST_CALL_STATUS_CONNECTED)
  {
    failed = 1;
  }

  pthread_mutex_unlock(&call->mutx);

  if (failed)
  {
    WRITE_STR(fd, "Call up operation failed\n");
  }
}

void net_down(dsi_net_call_info* call, int fd)
{
  int failed = 0;
  struct timespec  tspec;

  if (call->status != DSI_TEST_CALL_STATUS_CONNECTED)
  {
    WRITE_STR(fd, "Call down operation failed.  Call not in connected state.\n");
    return;
  }

  /* set the call status before starting asynchronous operation */
  call->status = DSI_TEST_CALL_STATUS_DISCONNECTING;

  /* end the data call */
  if (dsi_stop_data_call(call->handle) != DSI_SUCCESS)
  {
    call->status = DSI_TEST_CALL_STATUS_CONNECTED;
    WRITE_STR(fd, "Error ending data call.\n");
    return;
  }

  /* wait until the call status changes */
  pthread_mutex_lock(&call->mutx);

  if (call->status == DSI_TEST_CALL_STATUS_DISCONNECTING)
  {
    clock_gettime(CLOCK_REALTIME, &tspec);
    tspec.tv_sec += DSI_NETCTRL_TEST_WAIT_TIME;
    pthread_cond_timedwait(&call->cond, &call->mutx, &tspec);
  }

  if (call->status != DSI_TEST_CALL_STATUS_IDLE)
  {
    failed = 1;
  }

  pthread_mutex_unlock(&call->mutx);

  if (failed)
  {
    WRITE_STR(fd, "Call down operation failed\n");
  }
}

void create_call(int fd, const char* name)
{
  int i;
  int index = -1;

  for (i = 0; i < DSI_NETCTRL_TEST_MAX_CALLS; i++)
  {
    if (dsi_net_calls[i].handle == NULL)
    {
      index = i;
      break;
    }
  }

  if (index >= 0)
  {
    dsi_net_calls[index].handle = dsi_get_data_srvc_hndl(dsi_callback, (void*) &dsi_net_calls[index]);
    dsi_net_calls[index].status = DSI_TEST_CALL_STATUS_IDLE;
    strlcpy(dsi_net_calls[index].name, name, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
    pthread_mutex_init(&dsi_net_calls[index].mutx, NULL);
    pthread_cond_init(&dsi_net_calls[index].cond, NULL);
  }
  else
  {
    WRITE_STR(fd, "Maximum number of calls reached\n");
  }
}

void release_call(int fd, dsi_net_call_info* call)
{
  (void)fd;
  dsi_rel_data_srvc_hndl(call->handle);
  pthread_mutex_destroy(&call->mutx);
  pthread_cond_destroy(&call->cond);
  memset(call, 0, sizeof(dsi_net_call_info));
}

void set_param(int fd, dsi_net_call_info* call, int identifier, int num, char* buf)
{
  dsi_call_param_value_t param_info;

  param_info.num_val = num;
  param_info.buf_val = buf;

  if (dsi_set_data_call_param(call->handle, identifier, &param_info) != DSI_SUCCESS)
  {
    WRITE_STR(fd, "Set parameter operation failed!\n");
  }
}

void do_cmd(dsi_netctrl_test_cmd_info *cmd, int fd)
{
  dsi_net_call_info* call = NULL;
  dsi_net_profile_info* profile = NULL;

  switch (cmd->cmd)
  {
  case DSI_NETCTRL_TEST_CMD_STOP_SERVER:
    alive = 0;
    break;

  case DSI_NETCTRL_TEST_CMD_CREATE_CALL:
    call = find_call(cmd->name);
    if (call)
    {
      WRITE_STR(fd, "Create call failed.  Call with that name exists already.\n");
      break;
    }

    create_call(fd, cmd->name);
    break;

  case DSI_NETCTRL_TEST_CMD_RELEASE_CALL:
    call = find_call(cmd->name);
    if (!call)
    {
      WRITE_STR(fd, "Release call failed.  Call not found\n");
      break;
    }

    release_call(fd, call);
    break;

  case DSI_NETCTRL_TEST_CMD_SET_CALL_PARAM:
    call = find_call(cmd->name);
    if (!call)
    {
      WRITE_STR(fd, "Set call parameter failed.  Call not found\n");
      break;
    }

    set_param(fd, call, cmd->call_param.identifier, cmd->call_param.num_val, cmd->call_param.buf_val);
    break;

  case DSI_NETCTRL_TEST_CMD_NET_UP:
    call = find_call(cmd->name);
    if (!call)
    {
      WRITE_STR(fd, "Net up failed.  Call not found\n");
      break;
    }

    net_up(call, fd);
    break;

  case DSI_NETCTRL_TEST_CMD_NET_DOWN:
    call = find_call(cmd->name);
    if (!call)
    {
      WRITE_STR(fd, "Net down failed.  Call not found\n");
      break;
    }

    net_down(call, fd);
    break;

  case DSI_NETCTRL_TEST_CMD_LIST_CALLS:
    list_calls(fd);
    break;

  case DSI_NETCTRL_TEST_CMD_GET_DEVICE:
    call = find_call(cmd->name);
    if (!call)
    {
      /* output is expected for this command.  No output on error */
      break;
    }

    get_device(call, fd);
    break;

  case DSI_NETCTRL_TEST_CMD_GET_TECH:
    call = find_call(cmd->name);
    if (!call)
    {
      /* output is expected for this command.  No output on error */
      break;
    }

    get_tech(call, fd);
    break;

  case DSI_NETCTRL_TEST_CMD_PROFILE_INIT:
    if (qdp_init(cmd->name) != QDP_SUCCESS)
    {
      WRITE_STR(fd, "QDP initialization failed\n");
    }
    else
    {
      qdp_inited = 1;
    }

    break;

  case DSI_NETCTRL_TEST_CMD_PROFILE_DEINIT:
    if (qdp_inited)
    {
      qdp_deinit();
      qdp_inited = 0;
    }
    break;

  case DSI_NETCTRL_TEST_CMD_CREATE_PROFILE:
    profile = find_profile(cmd->name);
    if (profile)
    {
      WRITE_STR(fd, "Create profile failed.  A profile with that name exists already.\n");
      break;
    }

    create_profile(fd, cmd->name);
    break;

  case DSI_NETCTRL_TEST_CMD_RELEASE_PROFILE:
    profile = find_profile(cmd->name);
    if (!profile)
    {
      WRITE_STR(fd, "Release profile failed.  Profile not found.\n");
      break;
    }

    release_profile(fd, profile);
    break;

  case DSI_NETCTRL_TEST_CMD_SET_PROFILE_PARAM:
    profile = find_profile(cmd->name);
    if (!profile)
    {
      WRITE_STR(fd, "Set profile parameter failed. Profile not found.\n");
      break;
    }

    set_profile_param(fd, profile, cmd->profile_param.identifier, cmd->profile_param.val);
    break;

  case DSI_NETCTRL_TEST_CMD_LOOKUP_PROFILE:
    profile = find_profile(cmd->name);
    if (!profile)
    {
      /*Lookup profile failed.  Operation expects output.  No output to indicate error. */
      break;
    }

    lookup_profile(fd, profile);
    break;

  default:
    WRITE_STR(fd, "Test application error: Unrecognized command code.\n");
    break;
  }
}

void cleanup()
{
  int i;

  LOG("cleaning up\n");

  for (i = 0; i < DSI_NETCTRL_TEST_MAX_CALLS; i++)
  {
    if (dsi_net_calls[i].handle != NULL)
    {
      LOG("releasing call %s\n", dsi_net_calls[i].name);
      release_call(STDERR_FILENO, &dsi_net_calls[i]);
    }
  }

  for (i = 0; i < DSI_NETCTRL_TEST_MAX_PROFILES; i++)
  {
    if (std_strlen(dsi_net_profiles[i].name) > 0)
    {
      LOG("releasing profile %s\n", dsi_net_profiles[i].name);
      release_profile(STDERR_FILENO, &dsi_net_profiles[i]);
    }
  }

  if (qdp_inited)
  {
    qdp_deinit();
  }

  LOG("done cleaning up\n");
}

int main(int argc, char** argv)
{
  int socketfd;
  int servlen;
  socklen_t clilen;
  ssize_t len;
  struct sockaddr_un cli_addr, serv_addr;
  struct sockaddr_un addr;
  struct sockaddr_un* __attribute__((__may_alias__)) addr_ptr;

  /* Our process ID and Session ID */
  pid_t pid, sid;

  if (argc == 2 && std_stricmp("debug", argv[1]) == 0)
    debug = 1;

  if (!debug)
  {
    /* Fork off the parent process */
    pid = fork();
    if (pid < 0)
    {
      exit(EXIT_FAILURE);
    }

    /* Exit the parent process. */
    if (pid > 0)
    {
      exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0)
    {
      exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }

  socketfd = socket(AF_UNIX, SOCK_STREAM, 0);

  if (socketfd < 0)
  {
    exit(EXIT_FAILURE);
  }

  /* Unlink in case previous run exited unexpectedly */
  unlink(DSI_NETCTRL_TEST_SOCKET);

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strlcpy(serv_addr.sun_path, DSI_NETCTRL_TEST_SOCKET, sizeof(serv_addr.sun_path));
  servlen = (int)(std_strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family));

  memcpy(&addr, &serv_addr, sizeof(addr));
  addr_ptr = &addr;
  if (bind(socketfd, (struct sockaddr*)addr_ptr, servlen) < 0)
  {
    LOG("bind failed\n");
    exit(EXIT_FAILURE);
  }

  if (listen(socketfd,5) < 0)
  {
    LOG("listen failed\n");
    exit(EXIT_FAILURE);
  }

  clilen = sizeof(struct sockaddr_un);

  /* initilize dsi netctrl module  */
  if (DSI_SUCCESS != dsi_init(DSI_MODE_GENERAL))
  {
    LOG("Could not initialize dsi\n");
    exit(EXIT_FAILURE);
  }

  /* initialize qmi */
  qmi_handle = qmi_init(NULL, NULL);
  if (qmi_handle < 0)
  {
    LOG("Could not initialize qmi\n");
    exit(EXIT_FAILURE);
  }

  memset(dsi_net_calls, 0, sizeof(dsi_net_calls));
  memset(dsi_net_profiles, 0, sizeof(dsi_net_profiles));

  while (alive)
  {
    dsi_netctrl_test_cmd_info cmd;

    memcpy(&addr, &cli_addr, sizeof(addr));
    addr_ptr = &addr;
    int clientfd = accept(socketfd, (struct sockaddr*)addr_ptr, &clilen);

    if (clientfd < 0)
    {
      continue;
    }

    len = read(clientfd, &cmd, sizeof(cmd));

    if (len != sizeof(cmd))
    {
      WRITE_STR(clientfd, "ERROR: invalid data sent by client\n");
    }
    if (!memchr(cmd.name, '\0', sizeof(cmd.name)))
    {
      WRITE_STR(clientfd, "ERROR: invalid string\n");
    }
    else
    {
      do_cmd(&cmd, clientfd);
    }

    close(clientfd);
  }

  cleanup();

  close(socketfd);
  unlink(DSI_NETCTRL_TEST_SOCKET);
  qmi_release(qmi_handle);

  exit(EXIT_SUCCESS);
}

