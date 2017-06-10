/* server_process.c
 *
 * This file contains the server_process helper function implementation.
 * After receiving a command from HAL, the server sends over the command here
 * for further processing.
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <sys/un.h>

#include "server_process.h"
#include "mct_controller.h"
#include "mct_module.h"
#include "cam_intf.h"
#include "camera_dbg.h"
#include <sys/sysinfo.h>

#if 0
#undef CDBG
#define CDBG ALOGE
#endif

static mct_module_init_name_t modules_list[] = {
  {"sensor", module_sensor_init,   module_sensor_deinit, NULL},
  {"iface",  module_iface_init,   module_iface_deinit, NULL},
  {"isp",    module_isp_init,      module_isp_deinit, NULL},
  {"stats",  stats_module_init,    stats_module_deinit, NULL},
  {"pproc",  pproc_module_init,    pproc_module_deinit, NULL},
  {"imglib", module_imglib_init, module_imglib_deinit, NULL},
};

static mct_list_t *modules = NULL;
static mct_list_t *modules_all = NULL;

/** server_process_module_deinit:
 *    @data: MctModule_t from link list
 *
 * This function should be triggered ONLY when
 * server_process_module_init() fails. We shouldn't
 * hit here for all other scenarios.
 **/
static boolean server_process_module_deinit()
{
  mct_module_t *temp = NULL;
  int          i;

  CDBG("CAMERA_DAEMON: %s:%d, deint mods", __func__, __LINE__);
  for (i = 1;
       i < (int)(sizeof(modules_list)/sizeof(mct_module_init_name_t)); i++) {
    if( NULL == modules_list[i].deinit_mod)
      continue;
    CDBG("CAMERA_DAEMON: module name : %s: E\n", modules_list[i].name);
    modules_list[i].deinit_mod(modules_list[i].module);
    CDBG("CAMERA_DAEMON: module name : %s: X\n", modules_list[i].name);
  } /* for */

  mct_list_free_all(modules_all, NULL);
  modules_all = NULL;

  CDBG("CAMERA_DAEMON: %s:%d, deint mods done ", __func__, __LINE__);

  mct_list_free_all(modules, NULL);
  modules = NULL;

  temp = modules_list[0].module;

  if (temp) {
    if ((modules = mct_list_append(modules, temp, NULL, NULL)) == NULL) {
      if (modules) {
        modules_list[0].deinit_mod(temp);
        modules_list[0].module = NULL;
        return FALSE;
      }
    }
  }
  return TRUE;
}

/** server_process_module_init:
 *
 *  Very first thing Imaging Server performs after it starts
 *  to build up module list. One specific module initilization
 *  may not success because the module may not exist on this
 *  platform.
 **/
boolean server_process_module_init(void)
{
  mct_module_t *temp = NULL;
  int          i;

  CDBG("CAMERA_DAEMON: %s:%d, int mods", __func__, __LINE__);
  for (i = 1;
       i < (int)(sizeof(modules_list)/sizeof(mct_module_init_name_t)); i++) {
    if( NULL == modules_list[i].init_mod)
      continue;
    CDBG("CAMERA_DAEMON: module name : %s: E\n", modules_list[i].name);
    temp = modules_list[i].init_mod(modules_list[i].name);
    CDBG("CAMERA_DAEMON: module name : %s: X\n", modules_list[i].name);
    if (temp) {
      modules_list[i].module = temp;
      if ((modules = mct_list_append(modules, temp, NULL, NULL)) == NULL) {
        if (modules) {
          for (i--; i >= 0; i--) {
            modules_list[i].deinit_mod(temp);
            modules_list[i].module = NULL;;
          }

          mct_list_free_all(modules, NULL);
          return FALSE;
        }
        mct_list_append(modules_all, temp, NULL, NULL);
      }
    }
  } /* for */
  CDBG("CAMERA_DAEMON: %s:%d, int mods done ", __func__, __LINE__);

  return TRUE;
}

boolean server_process_module_sensor_init(void)
{
  mct_module_t *temp = NULL;

  CDBG("CAMERA_DAEMON: %s:%d, sensor int mods ", __func__, __LINE__);
    if( NULL == modules_list[0].init_mod)
      return FALSE;

    temp = modules_list[0].init_mod(modules_list[0].name);
    if (temp) {
      modules_list[0].module = temp;
      if ((modules = mct_list_append(modules, temp, NULL, NULL)) == NULL) {
        if (modules) {
            modules_list[0].deinit_mod(temp);
            modules_list[0].module = NULL;
            return FALSE;
        }
      }
    }
  CDBG("CAMERA_DAEMON: %s:%d, sensoer int mods done ", __func__, __LINE__);

  return TRUE;
}
/** server_process_bind_hal_dsocket
 *    @session: new session index
 *    @ds_fd:   domain socket file descriptor with HAL
 *
 *  Return - TRUE if it everything is fine
 **/
static boolean server_process_bind_hal_ds(int session,
  int *ds_fd)
{
  struct sockaddr_un addr;

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;

  snprintf(addr.sun_path,
           UNIX_PATH_MAX, "/data/misc/camera/cam_socket%d", session);

  /* remove the socket path if it already exists, otherwise bind might fail */
  unlink(addr.sun_path);

  *ds_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (*ds_fd == -1)
    return FALSE;

  if (bind(*ds_fd, (struct sockaddr *)&addr,
      sizeof(struct sockaddr_un)) == -1) {
    ALOGE("%s:%d: Error in bind socket_fd=%d %s ", __func__, __LINE__,
      *ds_fd, strerror(errno));
    close(*ds_fd);
    return FALSE;
  }
  return TRUE;
}

/** server_process_hal_event:
 *    @event: v4l2_event from kernel
 *
 * Process any command recevied from HAL through kernel.
 *
 * Return: process result, action server should take.
 **/
serv_proc_ret_t server_process_hal_event(struct v4l2_event *event)
{
  serv_proc_ret_t ret;
  mct_serv_msg_t  serv_msg;
  struct msm_v4l2_event_data *data =
    (struct msm_v4l2_event_data *)(event->u.data);
  struct msm_v4l2_event_data *ret_data =
    (struct msm_v4l2_event_data *)(ret.ret_to_hal.ret_event.u.data);
  struct sysinfo info;
  uint32_t result;
  int32_t enabled_savemem = 0;
  char savemem[92];

  /* by default don't return command ACK to HAL,
   * return ACK only for two cases:
   * 1. NEW SESSION
   * 2. Failure
   *
   * other command will return after they are processed
   * in MCT */
  ret.ret_to_hal.ret       = FALSE;
  ret.ret_to_hal.ret_type  = SERV_RET_TO_HAL_CMDACK;
  ret.ret_to_hal.ret_event = *event;
  ret_data->v4l2_event_type   = event->type;
  ret_data->v4l2_event_id     = event->id;
  ret.result               = RESULT_SUCCESS;

  result = sysinfo(&info);
  property_get("cameradaemon.SaveMemAtBoot", savemem, "0");
  enabled_savemem = atoi(savemem);

  switch (event->id) {
  case MSM_CAMERA_NEW_SESSION: {
    ret.ret_to_hal.ret = TRUE;
    ret.result = RESULT_NEW_SESSION;

    if(enabled_savemem == 1) {
      if (server_process_module_init() == FALSE)
        goto error_return;
    }

    /* new session starts, need to create a MCT:
     * open a pipe first.
     *
     * Note the 3 file descriptors:
     * one domain socket fd and two pipe fds are closed
     * at server side once session close information
     * is recevied by server.
     * */
    int pipe_fd[2];

    if (!pipe(pipe_fd)) {
      ret.new_session_info.mct_msg_rd_fd = pipe_fd[0];
      ret.new_session_info.mct_msg_wt_fd = pipe_fd[1];
    } else {
      goto error_return;
    }

    if (server_process_bind_hal_ds(data->session_id,
          &(ret.new_session_info.hal_ds_fd)) == FALSE) {
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      goto error_return;
    }

    if (mct_controller_new(modules, data->session_id, pipe_fd[1]) == TRUE) {
      ret.new_session    = TRUE;
      ret.new_session_info.session_idx = data->session_id;
      goto process_done;
    } else {
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      ret.ret_to_hal.ret_type       = SERV_RET_TO_HAL_NOTIFY_ERROR;
      ret_data->session_id          = data->session_id;
      goto error_return;
    }
  } /* case MSM_CAMERA_NEW_SESSION */
    break;

  case MSM_CAMERA_DEL_SESSION: {
    ret.result = RESULT_DEL_SESSION;
    ret.ret_to_hal.ret = FALSE;

    if (mct_controller_destroy(data->session_id) == FALSE) {
      if (enabled_savemem == 1)
        server_process_module_deinit();
      goto error_return;
    } else {
      if (enabled_savemem == 1)
        server_process_module_deinit();
      goto process_done;
    }
  } /* case MSM_CAMERA_DEL_SESSION */
    break;

  default:
    serv_msg.msg_type  = SERV_MSG_HAL;
    serv_msg.u.hal_msg = *event;
    break;
  } /* switch (event->type) */

  if (mct_controller_proc_serv_msg(&serv_msg) == FALSE) {
    ret.result = RESULT_FAILURE;
    goto error_return;
  }

process_done:
  ret_data->status = MSM_CAMERA_CMD_SUCESS;
  return ret;

error_return:
  ret_data->status = MSM_CAMERA_ERR_CMD_FAIL;
  return ret;
}

/** server_process_read_dsocket_packet:
 *    @fd:
 *    @packet:
 **/
static boolean server_process_read_ds_packet(const int fd,
  cam_sock_packet_t *packet)
{
  struct msghdr  msgh;
  struct iovec   iov[1];
  struct cmsghdr *cmsghp = NULL;
  char   control[CMSG_SPACE(sizeof(int))];
  int    rcvd_fd = -1;

  memset(&msgh, 0, sizeof(msgh));
  msgh.msg_name       = NULL;
  msgh.msg_namelen    = 0;
  msgh.msg_control    = control;
  msgh.msg_controllen = sizeof(control);

  memset(packet, 0, sizeof(cam_sock_packet_t));
  iov[0].iov_base = packet;
  iov[0].iov_len  = sizeof(cam_sock_packet_t);
  msgh.msg_iov    = iov;
  msgh.msg_iovlen = 1;

  if ((recvmsg(fd, &(msgh), 0)) <= 0)
    return FALSE;

  if (((cmsghp = CMSG_FIRSTHDR(&msgh)) != NULL) &&
      (cmsghp->cmsg_len == CMSG_LEN(sizeof(int)))) {

    if (cmsghp->cmsg_level == SOL_SOCKET &&
        cmsghp->cmsg_type  == SCM_RIGHTS) {
      rcvd_fd = *((int *)CMSG_DATA(cmsghp));
    } else {
      return FALSE;
    }
  }

  packet->payload.buf_map.fd = rcvd_fd;

  return TRUE;
}

/** server_process_hal_ds_packet:
 *    @fd
 *    @session
 *
 *  Return: serv_proc_ret_t
 *          FAILURE - will return to HAL immediately
 **/
serv_proc_ret_t server_process_hal_ds_packet(const int fd,
  const int session)
{
  cam_sock_packet_t  packet;
  mct_serv_msg_t     serv_msg;
  serv_proc_ret_t    ret;
  struct msm_v4l2_event_data *ret_data = (struct msm_v4l2_event_data *)
    &(ret.ret_to_hal.ret_event.u.data[0]);

  memset(&ret, 0, sizeof(serv_proc_ret_t));
  ret.ret_to_hal.ret         = TRUE;
  ret.ret_to_hal.ret_type    = SERV_RET_TO_HAL_NOTIFY;
  ret.ret_to_hal.ret_event.type  = MSM_CAMERA_V4L2_EVENT_TYPE;
  ret_data->v4l2_event_type   = MSM_CAMERA_V4L2_EVENT_TYPE;
  ret.result = RESULT_SUCCESS;

  if (server_process_read_ds_packet(fd, &packet) == FALSE) {
    ret.result        = RESULT_FAILURE;
    goto error_return;
  }

  serv_msg.msg_type = SERV_MSG_DS;
  serv_msg.u.ds_msg.session  = session;

  serv_msg.u.ds_msg.operation = (unsigned int)packet.msg_type;
  if (packet.msg_type == CAM_MAPPING_TYPE_FD_MAPPING) {
    ALOGV("%s: [dbgHang] - Map buffer --->>> enter", __func__);
    serv_msg.u.ds_msg.buf_type  = (unsigned int)packet.payload.buf_map.type;
    serv_msg.u.ds_msg.fd        = packet.payload.buf_map.fd;
    serv_msg.u.ds_msg.stream    = packet.payload.buf_map.stream_id;
    serv_msg.u.ds_msg.size      = packet.payload.buf_map.size;
    serv_msg.u.ds_msg.index     = packet.payload.buf_map.frame_idx;
    serv_msg.u.ds_msg.plane_idx = packet.payload.buf_map.plane_idx;
  } else if (packet.msg_type == CAM_MAPPING_TYPE_FD_UNMAPPING) {
    ALOGV("[%s: dbgHang] - UnMap buffer --->>> enter", __func__);
    serv_msg.u.ds_msg.buf_type  = (int)packet.payload.buf_map.type;
    serv_msg.u.ds_msg.stream    = packet.payload.buf_unmap.stream_id;
    serv_msg.u.ds_msg.index     = packet.payload.buf_unmap.frame_idx;
  }

  if (mct_controller_proc_serv_msg(&serv_msg) == FALSE) {
    ret.result = RESULT_FAILURE;
    goto error_return;
  }
  if (packet.msg_type == CAM_MAPPING_TYPE_FD_MAPPING) {
    ALOGV("%s: [dbgHang] - Map buffer --->>> exit", __func__);
  } else if (packet.msg_type == CAM_MAPPING_TYPE_FD_UNMAPPING) {
    ALOGV("[%s: dbgHang] - UnMap buffer --->>> exit", __func__);
  }

  ret_data->status = MSM_CAMERA_BUF_MAP_SUCESS;
  return ret;

error_return:
  if (ret.result == RESULT_FAILURE) {
    ret.ret_to_hal.ret         = TRUE;
    ret_data->status        = MSM_CAMERA_ERR_MAPPING;
    ret_data->session_id    = session;

    if (packet.msg_type == CAM_MAPPING_TYPE_FD_MAPPING) {
      ALOGE("%s: Buffer map error", __func__);
      ret_data->stream_id = packet.payload.buf_map.stream_id;
      /*ret_data->command   = packet.payload.buf_map.type;*/
    } else if (packet.msg_type == CAM_MAPPING_TYPE_FD_UNMAPPING) {
      ALOGE("%s: Buffer unmap error", __func__);
      ret_data->stream_id = packet.payload.buf_unmap.stream_id;
      /*ret_data->command = packet.payload.buf_unmap.type;*/
    }
  }
  return ret;
}

/** server_process_mct_msg:
 *    @fd: pipe fd media controller uses to send message to HAL
 *    @session: session index
 *
 *  Return: serv_proc_ret_t
 *          FAILURE - will return to HAL immediately
 **/
serv_proc_ret_t server_process_mct_msg(const int fd, const unsigned int session)
{
  int read_len;
  mct_process_ret_t mct_ret;
  serv_proc_ret_t ret;
  struct msm_v4l2_event_data *ret_data = (struct msm_v4l2_event_data *)
    ret.ret_to_hal.ret_event.u.data;

  read_len = read(fd, &mct_ret, sizeof(mct_process_ret_t));
  if (read_len <= 0) {
    ALOGE("%s: ERROR - read len is less than expected: %d", __func__, read_len);
    goto error;
  }

  ret.result           = RESULT_SUCCESS;
  ret.ret_to_hal.ret   = TRUE;

  switch (mct_ret.type) {
  case MCT_PROCESS_RET_SERVER_MSG: {

    if (mct_ret.u.serv_msg_ret.msg.msg_type == SERV_MSG_HAL) {
      /* We just processed a HAL command, need to ACK it */
      struct v4l2_event *msg = &(mct_ret.u.serv_msg_ret.msg.u.hal_msg);
      struct msm_v4l2_event_data *data =
        (struct msm_v4l2_event_data *)(msg->u.data);

      if (data->session_id != session) {
        ALOGE("%s: ERROR - session ID: %d is different than expected: %d", __func__, data->session_id, session);
        goto error;
      }

      ret.ret_to_hal.ret_type  = SERV_RET_TO_HAL_CMDACK;
      ret.ret_to_hal.ret_event = *msg;
      ret_data->v4l2_event_type   = msg->type;
      ret_data->v4l2_event_id     = msg->id;

      ret_data->status         = (mct_ret.u.serv_msg_ret.error == TRUE) ?
        MSM_CAMERA_CMD_SUCESS : MSM_CAMERA_ERR_CMD_FAIL;
    } else if (mct_ret.u.serv_msg_ret.msg.msg_type == SERV_MSG_DS) {
      /* Note we just processed a Domain Socket mapping,
       * need to send an event to HAL */
       mct_serv_ds_msg_t *msg = &(mct_ret.u.serv_msg_ret.msg.u.ds_msg);
      if (msg->session!= session) {
        ALOGE("%s: ERROR - session ID: %d is different than expected: %d", __func__, msg->session, session);
        goto error;
      }

      ret.ret_to_hal.ret_type       = SERV_RET_TO_HAL_NOTIFY;
      ret.ret_to_hal.ret_event.type = MSM_CAMERA_V4L2_EVENT_TYPE;
      ret.ret_to_hal.ret_event.id = MSM_CAMERA_MSM_NOTIFY;
      ret_data->v4l2_event_type   = MSM_CAMERA_V4L2_EVENT_TYPE;
      ret_data->v4l2_event_id     = MSM_CAMERA_MSM_NOTIFY;
      ret_data->command           = CAM_EVENT_TYPE_MAP_UNMAP_DONE;
      ret_data->session_id        = msg->session;
      ret_data->stream_id         = msg->stream;
      ret_data->map_buf_idx       = msg->index;
      ret_data->map_op            = (unsigned int)msg->buf_type;
      ret_data->status            = (mct_ret.u.serv_msg_ret.error == TRUE) ?
        MSM_CAMERA_STATUS_SUCCESS : MSM_CAMERA_STATUS_FAIL;
    } else {
      ALOGE("%s: ERROR - unexpected message type: %d", __func__, mct_ret.u.serv_msg_ret.msg.msg_type);
      goto error;
    }
  }
    break;

  case MCT_PROCESS_RET_BUS_MSG: {

    ret.ret_to_hal.ret_type       = SERV_RET_TO_HAL_NOTIFY;
    ret.ret_to_hal.ret_event.type = MSM_CAMERA_V4L2_EVENT_TYPE;
    ret.ret_to_hal.ret_event.id   = MSM_CAMERA_MSM_NOTIFY;
    ret_data->v4l2_event_type   = MSM_CAMERA_V4L2_EVENT_TYPE;
    ret_data->v4l2_event_id     = MSM_CAMERA_MSM_NOTIFY;

    if (mct_ret.u.bus_msg_ret.msg_type == MCT_BUS_MSG_REPROCESS_STAGE_DONE) {
      ret_data->command = CAM_EVENT_TYPE_REPROCESS_STAGE_DONE;
    }

    if (mct_ret.u.bus_msg_ret.msg_type == MCT_BUS_MSG_SEND_EZTUNE_EVT) {
      ret_data->command = CAM_EVENT_TYPE_INT_TAKE_PIC;
    }

    ret_data->session_id          = mct_ret.u.bus_msg_ret.session;
    ret_data->status              = (mct_ret.u.bus_msg_ret.error == TRUE) ?
      MSM_CAMERA_STATUS_SUCCESS : MSM_CAMERA_STATUS_FAIL;
  }
    break;
  case MCT_PROCESS_DUMP_INFO: {
    ret.ret_to_hal.ret_type       = SERV_RET_TO_KERNEL_NOTIFY_POSSIBLE_FREEZE;
    ret_data->session_id          = mct_ret.u.bus_msg_ret.session;
  }
    break;
  case MCT_PROCESS_RET_ERROR_MSG: {
    ret.ret_to_hal.ret_type       = SERV_RET_TO_HAL_NOTIFY_ERROR;
    ret_data->session_id          = mct_ret.u.bus_msg_ret.session;
  }
    break;
  default:
    break;
  }
  return ret;

error:
  ret.result         = RESULT_FAILURE;
  ret.ret_to_hal.ret = FALSE;
  return ret;
}
