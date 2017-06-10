/* Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential. */

#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/media.h>
#include <media/msm_camera.h>
#ifdef _ANDROID_
#include <cutils/log.h>
#endif
#include <dlfcn.h>
#include <poll.h>
#include <media/msm_gestures.h>

#include "camera_dbg.h"
#include "camera.h"
#include "cam_mmap.h"
#ifdef FEATURE_GYRO
#include "sensor1.h"
#endif
#include "tgtcommon.h"
#include "intf_comm_data.h"
#include "mctl.h"
#include "dsps_hw_interface.h"
#include "camera_plugin_intf.h"
#include "camera_mctl_interface.h"

#define QSRV_CONFIG_MAX_LEN         512

#define CAMERA_INFO_DEBUG 0

#if 0
#undef CDBG
#define CDBG LOGE
#endif

typedef struct {
  void *handle;
  int command_pipe_fds[2];
  int mctl_id;
  struct cam_list list;
} mm_mctl_inst_t;

static mm_mctl_inst_t my_mctl_list;

typedef struct {
  void* ptr;
  int (*gesture_service_create)(camera_mctl_client_t *p_client,
    camera_mctl_observer_t *p_observer);
  int (*gesture_service_destroy)();
  int (*gesture_service_send_data)(struct msm_ctrl_cmd *p_ctrl);
} gesture_lib_info_t;

typedef struct {
  camera_mctl_client_t cam_mctl;
  camera_mctl_observer_t observer;
  gesture_lib_info_t gesture_lib;
} gesture_info_t;

typedef struct {
  uint8_t init_done;
  pthread_mutex_t mutex;
  int num_comps;
  comp_res_entries_t *res[COMP_MAX_HWS];
} comp_res_t;

static comp_res_t qserver_res_mgr;

typedef struct {
  void* ptr;
  camera_plugin_ops_t plugin_ops;
  void (*camera_plugin_create_fn) (camera_plugin_ops_t *camera_ops);
} mm_camera_plugin_t;
typedef struct {
  //mm_mctl_inst_t my_mctl_list;
  mm_camera_plugin_t camera_plugin;
} mm_daemon_obj_t;

static mm_daemon_obj_t daemon_obj;
/*===========================================================================
 * FUNCTION    - qcamsvr_camera_available -
 *
 * DESCRIPTION: check if camera resource is available
 *==========================================================================*/
int qcamsvr_camera_available(int resource_id)
{
  int cam_avail = (&my_mctl_list.list == my_mctl_list.list.next)
    && (&my_mctl_list.list == my_mctl_list.list.prev);
  CDBG_HIGH("%s:%d] cam_avail %d", __func__, __LINE__, cam_avail);
  return cam_avail;
}/*qcamsvr_camera_available*/

/*===========================================================================
 * FUNCTION    - qcamsvr_load_gesture_lib -
 *
 * DESCRIPTION:   load gesture library
 *==========================================================================*/
static int qcamsvr_load_gesture_lib(gesture_lib_info_t* pme)
{
  pme->ptr = dlopen("libmmgesture_services.so", RTLD_NOW);
  if (!pme->ptr) {
    CDBG_ERROR("%s Error opening gesture library", __func__);
    return -1;
  }

  *(void **)&(pme->gesture_service_create) =
  dlsym(pme->ptr, "gesture_service_create");
  if (!pme->gesture_service_create) {
    CDBG_ERROR("%s Error linking gesture_service_create", __func__);
    return -1;
  }

  *(void **)&(pme->gesture_service_destroy) =
  dlsym(pme->ptr, "gesture_service_destroy");
  if (!pme->gesture_service_destroy) {
    CDBG_ERROR("%s Error linking gesture_service_destroy", __func__);
    return -1;
  }

  *(void **)&(pme->gesture_service_send_data) =
  dlsym(pme->ptr, "gesture_service_send_data");
  if (!pme->gesture_service_send_data) {
    CDBG_ERROR("%s Error linking gesture_service_send_data", __func__);
    return -1;
  }

  return 0;
}

/*===========================================================================
 * FUNCTION    - qcamsvr_load_camera_plugin_lib -
 *
 * DESCRIPTION:   load gesture library
 *==========================================================================*/
static int qcamsvr_load_camera_plugin_lib(mm_daemon_obj_t* pme)
{

  pme->camera_plugin.ptr = dlopen("libmmcamera_plugin.oem.so", RTLD_NOW);
  if (pme->camera_plugin.ptr == NULL) {
    /* there is no oem plugin, load qualcomm's default plugin */
    CDBG("%s: no oem plugin libmmcamera_plugin.oem.so, load deafult plugin",
         __func__);
    pme->camera_plugin.ptr = dlopen("libmmcamera_plugin.so", RTLD_NOW);
  }
  if (!pme->camera_plugin.ptr) {
    CDBG_ERROR("%s Error opening libmmcamera_plugin.so", __func__);
    return -1;
  }
  CDBG("%s: camera plugin 'libmmcamera_plugin.so' opened", __func__);
  *(void **)&(pme->camera_plugin.camera_plugin_create_fn) =
    dlsym(pme->camera_plugin.ptr, "camera_plugin_create_func");
  if (!pme->camera_plugin.camera_plugin_create_fn) {
    CDBG_ERROR("%s Error linking camera_plugin_create_func", __func__);
    dlclose(pme->camera_plugin.ptr);
    memset(&pme->camera_plugin, 0, sizeof(pme->camera_plugin));
    return -1;
  }
  (pme->camera_plugin.camera_plugin_create_fn) (
    &pme->camera_plugin.plugin_ops);
  if (!pme->camera_plugin.plugin_ops.handle) {
    CDBG_ERROR("%s: camera_plugin_create_fn error, plugin.so closed",
      __func__);
    dlclose(pme->camera_plugin.ptr);
    memset(&pme->camera_plugin, 0, sizeof(pme->camera_plugin));
    return -1;
  }
  CDBG("%s: End, camera_plugin_handle = %p", __func__,
       pme->camera_plugin.plugin_ops.handle);
  return 0;
}

static int get_mctl_node_info(int server_fd,
  struct msm_mctl_node_info *mctl_node_info)
{
  int i=0;
  char mctl_node_name[MAX_DEV_NAME_LEN];
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;

  /* now ask server  node for mctl node names*/
  v4l2_ioctl.ioctl_ptr = mctl_node_info;
  if (ioctl(server_fd, MSM_CAM_V4L2_IOCTL_GET_MCTL_INFO,
    (void *)&v4l2_ioctl) < 0) {
    CDBG_ERROR("%s: MSM_CAM_V4L2_IOCTL_GET_MCTL_INFO error (%s)", __func__,
      strerror(errno));
    return -EINVAL;
  }

  CDBG("number of mctl nodes = %d\n", mctl_node_info->num_mctl_nodes);
  for (i = 0; i < mctl_node_info->num_mctl_nodes; i++) {
    if (mctl_node_info->mctl_node_name[i]== NULL)
      continue;
    snprintf(mctl_node_name, MAX_DEV_NAME_LEN, "%s",
      mctl_node_info->mctl_node_name[i]);
    CDBG("mctl device vid node %d name = %s\n", i, mctl_node_name);
  }

  return 0;
}

/*===========================================================================
 * FUNCTION     - eztune_setup_server -
 *
 * DESCRIPTION:
 * ==========================================================================*/
int eztune_setup_server (const char *const server_ip_str,
  const char *const server_port_str)
{
  int socket_inet = -1;
  struct sockaddr_in addr_inet;
  int port = atoi(server_port_str);
  int result;
  int option;
  int socket_flag;
  memset(&addr_inet, 0, sizeof(addr_inet));
  addr_inet.sin_family = AF_INET;
  addr_inet.sin_port= htons(port);
  addr_inet.sin_addr.s_addr = inet_addr(server_ip_str);

  if (addr_inet.sin_addr.s_addr == INADDR_NONE) {
    CDBG_ERROR("%s invalid address.\n", __func__);
    return -1;
  }

  /* Create an AF_INET stream socket to receive incoming connection ON */
  socket_inet = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_inet < 0) {
    CDBG_ERROR("%s socket failed\n", __func__);
    return -1;
  }
  CDBG("socket_inet = %d\n", socket_inet);

  socket_flag = fcntl(socket_inet, F_GETFL, 0);
  fcntl(socket_inet, F_SETFL, socket_flag | O_NONBLOCK);
  /* reuse in case it is in timeout */
  option = 1;
  result = setsockopt(socket_inet, SOL_SOCKET, SO_REUSEADDR,
    &option, sizeof(option));
  if (result < 0) {
    CDBG("%s result %d\n", __func__, result);
    perror("eztune setsockopt failed");
    close(socket_inet);
    return -1;
  }

  result = bind(socket_inet, (const struct sockaddr *)&addr_inet,
    sizeof(addr_inet));
  if (result < 0) {
    CDBG("%s result %d.\n", __func__, result);
    perror("eztune socket bind failed");
    close(socket_inet);
    return -1;
  }
  result = listen(socket_inet, 1);
  if (result != 0) {
    CDBG("%s result %d.\n", __func__, result);
    perror("eztune socket listen failed");
    close(socket_inet);
    return -1;
  }

  return socket_inet;
}

static mm_mctl_inst_t *qcamsvr_find_mctl_inst(int mctl_id)
{
  struct cam_list *head;
  struct cam_list *pos;
  mm_mctl_inst_t *node;

  head = &my_mctl_list.list;
  pos = head->next;
  while(pos != head) {
    node = member_of(pos, mm_mctl_inst_t, list);
    pos = pos->next;
    CDBG("%s: node->mctl_id = %d",  __func__,  node->mctl_id);
    if(node->mctl_id == mctl_id)
      return node;
  }
  CDBG_ERROR("%s: no match for mctl_id = %d",  __func__,  mctl_id);
  return NULL;
}

static int init_resource_table()
{
  int num_media_devices = 0;
  int num_entities = 0;
  int dev_fd = 0;
  char dev_name[32];
  int rc = 0;
  struct media_device_info mdev_info;
  comp_res_entries_t *temp_res_comp;

  qserver_res_mgr.num_comps = 0;
  pthread_mutex_init(&qserver_res_mgr.mutex, NULL);

  while (1) {
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices++);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd < 0) {
      CDBG("Done discovering media devices\n");
      break;
    }
    rc = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (rc < 0) {
      CDBG_ERROR("Error: ioctl media_dev failed: %s\n", strerror(errno));
      close(dev_fd);
      break;
    }
    if(strncmp(mdev_info.model, QCAMERA_SERVER_NAME, sizeof(mdev_info.model) != 0)) {
      close(dev_fd);
      continue;
    }

    num_entities = 1;
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities++;
      rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0) {
        CDBG("Done enumerating media entities\n");
        rc = 0;
        break;
      }
      if(entity.type == MEDIA_ENT_T_V4L2_SUBDEV) {
        temp_res_comp =
          (comp_res_entries_t*) malloc(sizeof(comp_res_entries_t));
        if (temp_res_comp == NULL) {
          CDBG_ERROR("%s: could not allocate resoucre memory\n", __func__);
          rc = -ENOMEM;
          break;
        }
        temp_res_comp->subdev_revision = entity.revision;
        temp_res_comp->num_reservations = 0;
        temp_res_comp->def.mctl_id = -1;
        switch (entity.group_id) {
        case VFE_DEV:
          temp_res_comp->comp_id = MCTL_COMPID_VFE;
          temp_res_comp->max_reservations = 2;
          temp_res_comp->u.vfe.num_used = 0;
          break;
        case AXI_DEV:
          temp_res_comp->comp_id = MCTL_COMPID_AXI;
          temp_res_comp->max_reservations = 2;
          temp_res_comp->u.axi.num_used_pix = 0;
          temp_res_comp->u.axi.num_used_rdi = 0;
          break;
        case VPE_DEV:
          temp_res_comp->comp_id = MCTL_COMPID_VPE;
          temp_res_comp->max_reservations = 1;
          break;
        case ISPIF_DEV:
          temp_res_comp->comp_id = MCTL_COMPID_ISPIF;
          temp_res_comp->max_reservations = 5;
          break;
        default:
          free(temp_res_comp);
          temp_res_comp = NULL;
          CDBG("%s: entity.group_id = %d, Invalid entity for this media device\n",
               __func__, entity.group_id);
          break;
        }
        if (temp_res_comp != NULL) {
          qserver_res_mgr.res[qserver_res_mgr.num_comps] = temp_res_comp;
          qserver_res_mgr.num_comps++;
        }
      }
    }
    close(dev_fd);
  }

  if (qserver_res_mgr.num_comps == 0)
    CDBG_HIGH("%s: Resource table is empty\n", __func__);

  qserver_res_mgr.init_done = 1;
  return rc;
}

static int destroy_resource_table()
{
  int i = 0;
  for (i = 0; i < qserver_res_mgr.num_comps; i++) {
    if (qserver_res_mgr.res[i])
      free(qserver_res_mgr.res[i]);
  }
  qserver_res_mgr.num_comps = 0;
  pthread_mutex_destroy(&qserver_res_mgr.mutex);
  qserver_res_mgr.init_done = 0;
  return 0;
}
/*==============================================================================
 * FUNCTION    - qcamsvr_send_ctrl_cmd_done -
 *
 * DESCRIPTION: Sends the Success or Failure V4L2 ctrl cmd done to server node.
 *============================================================================*/
static void qcamsvr_send_ctrl_cmd_done(int server_fd,
  struct msm_camera_v4l2_ioctl_t *v4l2_ioctl)
{
  if (ioctl(server_fd, MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE, v4l2_ioctl) < 0)
    CDBG_ERROR("%s: MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE error (%s)",
      __func__, strerror(errno));
} /* qcamsvr_send_ctrl_cmd_done */

/*==============================================================================
 * FUNCTION    - qcamsvr_process_server_node_event -
 *
 * DESCRIPTION: De-queues the events from server node and  dispatches to
 *              respective mctl thread depending on config node name.
 *============================================================================*/
static int qcamsvr_process_server_node_event(
  struct config_thread_arguments *config_arg,
  struct msm_mctl_node_info *mctl_node_info,
  gesture_info_t *p_gesture_info, uint8_t *ctrl_cmd_buffer)
{
  int rc = 0;
  char config_dev_name[MAX_DEV_NAME_LEN];
  struct v4l2_event v4l2_evt;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct msm_ctrl_cmd *ctrl;
  mm_mctl_inst_t *tmp_mctl_struct;

  rc = ioctl(config_arg->server_fd, VIDIOC_DQEVENT, &v4l2_evt);
  if (rc >= 0) {
    struct msm_isp_event_ctrl event_data;
    event_data.isp_data.ctrl.queue_idx = v4l2_evt.u.data[0];
    event_data.isp_data.ctrl.value = (void *)ctrl_cmd_buffer;

    CDBG("%s: VIDIOC_DQEVENT type = 0x%x\n", __func__, v4l2_evt.type);

    v4l2_ioctl.ioctl_ptr = &event_data;
    rc = ioctl(config_arg->server_fd, MSM_CAM_V4L2_IOCTL_GET_EVENT_PAYLOAD,
      &v4l2_ioctl);
    if (v4l2_evt.type == V4L2_EVENT_PRIVATE_START + MSM_GES_RESP_V4L2
      && rc >= 0) {
      struct msm_ctrl_cmd *ctrl;
      ctrl = &(event_data.isp_data.ctrl);
      int status = CAMERA_SUCCESS;
      CDBG("###Recieved gesture event %d %p", ctrl->type,
        p_gesture_info->gesture_lib.ptr);
      if (!p_gesture_info->gesture_lib.ptr) {
        CDBG_ERROR("gesture error loading library");
        status = CAMERA_ERR_GENERAL;
      } else if (ctrl->type == MSM_V4L2_GES_OPEN) {
        p_gesture_info->cam_mctl.svr_ops.launch_mctl_thread =
          create_v4l2_conf_thread;
        p_gesture_info->cam_mctl.svr_ops.release_mctl_thread =
          destroy_v4l2_cam_conf_thread;
        p_gesture_info->cam_mctl.svr_ops.camera_available =
          qcamsvr_camera_available;
        p_gesture_info->cam_mctl.svr_ops.init_camera_plugin =
          mctl_init_camera_plugin;
        p_gesture_info->cam_mctl.svr_ops.server_fd = config_arg->server_fd;
        p_gesture_info->cam_mctl.svr_ops.p_plugin_ops =
          &daemon_obj.camera_plugin.plugin_ops;
        status = p_gesture_info->gesture_lib.gesture_service_create(
          &p_gesture_info->cam_mctl, &p_gesture_info->observer);
        if (status != CAMERA_SUCCESS) {
          CDBG_ERROR("gesture_service_create failed");
        }
      } else if (ctrl->type == MSM_V4L2_GES_CLOSE) {
        status = p_gesture_info->gesture_lib.gesture_service_destroy();
        if (status != CAMERA_SUCCESS) {
          CDBG_ERROR("gesture_service_destroy failed");
        }
      }
      if ((status == CAMERA_SUCCESS) &&
        (ctrl->type != MSM_V4L2_GES_CLOSE)) {
        status = p_gesture_info->gesture_lib.gesture_service_send_data(ctrl);
        if (status != CAMERA_SUCCESS) {
          CDBG_ERROR("gesture_service_send_data failed");
        }
      } else {
        if (ctrl->type == MSM_V4L2_GES_CLOSE) {
          ctrl->status = CAM_CTRL_SUCCESS;
        } else {
          CDBG_ERROR("gesture send failure message");
          ctrl->status = CAM_CTRL_FAILED;
        }
        v4l2_ioctl.ioctl_ptr = ctrl;
        qcamsvr_send_ctrl_cmd_done(config_arg->server_fd, &v4l2_ioctl);
      }
    } else if (v4l2_evt.type == V4L2_EVENT_PRIVATE_START + MSM_CAM_RESP_V4L2 &&
      rc >= 0) {
      ctrl = &(event_data.isp_data.ctrl);
      CDBG("%s: got control command = %d\n", __func__, ctrl->type);

      if (ctrl->type == MSM_V4L2_OPEN) {
        CDBG("MSM_V4L2_OPEN is received\n");
        snprintf(config_arg->config_name, MAX_DEV_NAME_LEN, "%s",
          (char *)event_data.isp_data.ctrl.value);
        CDBG("%s: OPEN %s\n", __func__, config_arg->config_name);

        tmp_mctl_struct = malloc(sizeof(mm_mctl_inst_t));
        if (!tmp_mctl_struct) {
          CDBG_ERROR("%s: cannot malloc mctl struct",  __func__);
          ctrl->status = CAM_CTRL_FAILED;
          v4l2_ioctl.ioctl_ptr = ctrl;
          goto error;
        }

        memset(tmp_mctl_struct, 0, sizeof(mm_mctl_inst_t));
        tmp_mctl_struct->mctl_id = ctrl->vnode_id;

        snprintf(config_arg->mctl_node_name, MAX_DEV_NAME_LEN, "%s",
          mctl_node_info->mctl_node_name[ctrl->vnode_id]);
        CDBG("%s: MCTL Node = %s ", __func__, config_arg->mctl_node_name);

        /*initialize and setup command pipe*/
        if (pipe(tmp_mctl_struct->command_pipe_fds) < 0) {
          CDBG_ERROR("%s: pipe error \n", __func__);
          ctrl->status = CAM_CTRL_FAILED;
          v4l2_ioctl.ioctl_ptr = ctrl;
          goto error_config_pipe;
        }
        CDBG("%s: pipe fds are %d %d \n", __func__,
          tmp_mctl_struct->command_pipe_fds[0],
          tmp_mctl_struct->command_pipe_fds[1]);

        config_arg->read_fd = tmp_mctl_struct->command_pipe_fds[0];
        config_arg->write_fd = tmp_mctl_struct->command_pipe_fds[1];
        config_arg->vnode_id = ctrl->vnode_id;

        if ((tmp_mctl_struct->handle =
          create_v4l2_conf_thread(config_arg)) == NULL) {
          CDBG_ERROR("%s: create_v4l2_conf_thread failed", __func__);
          ctrl->status = CAM_CTRL_FAILED;
          v4l2_ioctl.ioctl_ptr = ctrl;
          goto error_config_thread_creation;
        }
        rc = mctl_init_camera_plugin(tmp_mctl_struct->handle,
          &daemon_obj.camera_plugin.plugin_ops);
        if(rc != 0) {
          CDBG_ERROR("%s: mctl_init_camera_plugin err = %d", __func__, rc);
          ctrl->status = CAM_CTRL_FAILED;
          v4l2_ioctl.ioctl_ptr = ctrl;
          goto error_config_thread_creation;
        }
        cam_list_add_tail_node(&tmp_mctl_struct->list, &my_mctl_list.list);
        ctrl->status = CAM_CTRL_SUCCESS;
        v4l2_ioctl.ioctl_ptr = ctrl;
        qcamsvr_send_ctrl_cmd_done(config_arg->server_fd, &v4l2_ioctl);
      } else if (ctrl->type == MSM_V4L2_CLOSE) {
        CDBG("MSM_V4L2_CLOSE is received\n");

        if(!(tmp_mctl_struct = qcamsvr_find_mctl_inst(ctrl->vnode_id))) {
          CDBG_ERROR("%s: MSM_V4L2_CLOSE - cannot find mctl, mctl_id = %d",
            __func__, ctrl->vnode_id);
          ctrl->status = CAM_CTRL_FAILED;
          v4l2_ioctl.ioctl_ptr = ctrl;
          goto error;
        }
        snprintf(config_dev_name, MAX_DEV_NAME_LEN, "%s",
          (char *)event_data.isp_data.ctrl.value);
        CDBG("%s: CLOSE %s\n", __func__, config_dev_name);

        if (tmp_mctl_struct->command_pipe_fds[1]) {
          write(tmp_mctl_struct->command_pipe_fds[1], &event_data,
            sizeof(struct msm_isp_event_ctrl));
          if (ctrl->length > 0) {
            write(tmp_mctl_struct->command_pipe_fds[1], ctrl->value,
              ctrl->length);
          }
        }

        if (destroy_v4l2_cam_conf_thread(tmp_mctl_struct->handle) < 0) {
          CDBG_ERROR("%s: destroy_v4l2_cam_conf_thread failed", __func__);
          ctrl->status = CAM_CTRL_FAILED;
          v4l2_ioctl.ioctl_ptr = ctrl;
          goto error_destroy_config_thread;
        }
        if (tmp_mctl_struct->command_pipe_fds[0]) {
          close(tmp_mctl_struct->command_pipe_fds[0]);
          tmp_mctl_struct->command_pipe_fds[0] = 0;
        }
        if (tmp_mctl_struct->command_pipe_fds[1]) {
          close(tmp_mctl_struct->command_pipe_fds[1]);
          tmp_mctl_struct->command_pipe_fds[1] = 0;
        }
        cam_list_del_node(&tmp_mctl_struct->list);
        if (tmp_mctl_struct) {
          free(tmp_mctl_struct);
          tmp_mctl_struct = NULL;
        }
        ctrl->status = CAM_CTRL_SUCCESS;
        v4l2_ioctl.ioctl_ptr = ctrl;
        qcamsvr_send_ctrl_cmd_done(config_arg->server_fd, &v4l2_ioctl);
      } else {
       /* send command through pipe and wait for config to return */
        CDBG("%s: camer_id = %d. Got event, send to config thread. type: %d\n",
        __func__, ctrl->vnode_id, ctrl->type);
        if (!(tmp_mctl_struct = qcamsvr_find_mctl_inst(ctrl->vnode_id))) {
          CDBG_ERROR("%s: MSM_V4L2_* - cannot find mctl, id = %d", __func__,
            ctrl->vnode_id);
          ctrl->status = CAM_CTRL_FAILED;
          v4l2_ioctl.ioctl_ptr = ctrl;
          goto error;
        }

        if (tmp_mctl_struct->command_pipe_fds[1]) {
          write(tmp_mctl_struct->command_pipe_fds[1], &event_data,
            sizeof(struct msm_isp_event_ctrl));
          if (ctrl->length > 0) {
            write(tmp_mctl_struct->command_pipe_fds[1], ctrl->value,
              ctrl->length);
          }
        }
      }
    } else {
      CDBG_HIGH("%s: Ignore events with null payload\n", __func__);
    }
  } else {
    CDBG_HIGH("%s: VIDIOC_DQEVENT type failed\n", __func__);
  }

  CDBG("%s: X", __func__);
  return rc;

error_destroy_config_thread:
  cam_list_del_node(&tmp_mctl_struct->list);
error_config_thread_creation:
  if (tmp_mctl_struct->command_pipe_fds[0]) {
    close(tmp_mctl_struct->command_pipe_fds[0]);
    tmp_mctl_struct->command_pipe_fds[0] = 0;
  }
  if (tmp_mctl_struct->command_pipe_fds[1]) {
    close(tmp_mctl_struct->command_pipe_fds[1]);
    tmp_mctl_struct->command_pipe_fds[1] = 0;
  }
error_config_pipe:
  if (tmp_mctl_struct) {
    free(tmp_mctl_struct);
    tmp_mctl_struct = NULL;
  }
error:
  if (CAM_CTRL_FAILED == ctrl->status)
    qcamsvr_send_ctrl_cmd_done(config_arg->server_fd, &v4l2_ioctl);

  return rc;
} /* qcamsvr_process_server_node_event */

/*===========================================================================
 * FUNCTION    - qcamsvr_start -
 *
 * DESCRIPTION: main daemon thread. Polls for events from server node
 *    dispatches to respective mctl thread depending on config
 *    node name.
 *==========================================================================*/
int qcamsvr_start(void)
{
  int c = 0,i, rc;
  int server_fd = -1, video_fd = -1;
  int cam_conf_exit = 1;
  char server_dev_name[MAX_DEV_NAME_LEN];
  struct pollfd fds[3];
  int timeoutms;
  struct v4l2_event_subscription sub;
  int ez_cmd_pipe[2] = {-1, -1};
  int ez_prev_cmd_pipe[2] = {-1, -1};
  int poll_count;
#ifdef EZTUNE_ENABLE
  int ez_server_socket_id;
  struct sockaddr_in addr_client_inet;
  socklen_t addr_client_len = sizeof(struct sockaddr_in);
  int ez_prev_server_socket_id;
#endif
  struct msm_mctl_node_info mctl_node_info;
  struct config_thread_arguments config_arg;
  gesture_lib_info_t gesture_lib;
  camera_mctl_client_t cam_mctl;
  camera_mctl_observer_t observer;
  gesture_info_t gesture_info;
  mode_t old_mode;
  uint8_t *ctrl_cmd_buffer;

  memset(&daemon_obj,  0,  sizeof(daemon_obj));
  memset(&gesture_info, 0x0, sizeof(gesture_info_t));
  memset(&config_arg, 0x0, sizeof(struct config_thread_arguments));

  memset(&my_mctl_list, 0, sizeof(mm_mctl_inst_t));
  cam_list_init(&my_mctl_list.list);
  memset(&mctl_node_info, 0x0, sizeof(struct msm_mctl_node_info));

  memset(&qserver_res_mgr, 0, sizeof(qserver_res_mgr));
  for (i=0; i < MSM_MAX_CAMERA_SENSORS; i++) {
    mctl_node_info.mctl_node_name[i] = (char*)malloc((size_t)MAX_DEV_NAME_LEN);
    if(mctl_node_info.mctl_node_name[i] == NULL) {
      goto error_init;
    }
  }

  CDBG("\nCamera Daemon starting\n");

  old_mode = umask(S_IRWXO);
  CDBG_ERROR("%s: old_mode = %x\n", __func__, old_mode);

  /*try to open the server node, currently "video_msm"*/
  snprintf(server_dev_name, MAX_DEV_NAME_LEN, MSM_CAMERA_SERVER);
  do {
    server_fd = open(server_dev_name, O_RDWR);
    if (server_fd < 0)
      CDBG_ERROR("qcamera_server_init open driver failed");
    usleep(1000 * 100);
  } while (server_fd < 0);
  /*setup the global resource table*/
  if (init_resource_table() < 0) {
    CDBG_ERROR("%s: Could not set up resoucre table\n", __func__);
    goto error_mctl_load_comps;
  }
  /* load all necessary components */
  if (mctl_load_comps()) {
    CDBG_ERROR("%s: mctl_load_comps failed", __func__);
    goto error_mctl_load_comps;
  }

  /* Load gesture library*/
  rc = qcamsvr_load_gesture_lib(&gesture_info.gesture_lib);
  if (rc != 0) {
    CDBG_ERROR("Cannot init Gesture library\n");
    if (gesture_info.gesture_lib.ptr) {
      dlclose(gesture_info.gesture_lib.ptr);
      gesture_info.gesture_lib.ptr = NULL;
    }
  }

  /*camera plugin is mandatory*/
  rc = qcamsvr_load_camera_plugin_lib(&daemon_obj);
  if (rc != 0) {
    CDBG_ERROR("%s: fatal error: Cannot "
      "init camera_plugin_lib\n", __func__);
    return -1;
  }

#ifdef EZTUNE_ENABLE
  ez_server_socket_id = eztune_setup_server("127.0.0.1", "55555");
  if (ez_server_socket_id < 0) {
    CDBG_ERROR("__debug:%s eztune_setup_server failed\n", __func__);
    goto error_eztune_server;
  } else {
    if (pipe(ez_cmd_pipe) < 0) {
      CDBG_ERROR("ez pipe error \n");
      goto error_eztune_cmd_pipe;
    }
  }

  ez_prev_server_socket_id = eztune_setup_server("127.0.0.1", "55556");
  if (ez_prev_server_socket_id < 0) {
    CDBG_ERROR("__debug:%s prev eztune_setup_server failed\n", __func__);
    goto error_eztune_prev_server;
  } else {
    if(pipe(ez_prev_cmd_pipe) < 0) {
      CDBG_ERROR("ez prev pipe error \n");
      goto error_eztune_prev_cmd_pipe;
    }
  }
#endif

#ifdef FEATURE_GYRO
  if (sensor1_init() != SENSOR1_SUCCESS) {
    CDBG_ERROR("Cannot init sensor1\n");
    goto error_dsps_config;
  }
#endif

  CDBG("%s: GETTING MCTL INFO ", __func__);
  if (get_mctl_node_info(server_fd, &mctl_node_info)) {
    CDBG_ERROR("%s: get_mctl_node_info failed", __func__);
    goto error_dsps_config;
  }

  ctrl_cmd_buffer = malloc(MAX_SERVER_PAYLOAD_LENGTH);
  if (ctrl_cmd_buffer == NULL) {
      CDBG_ERROR("%s: Could not allocate payload buffer\n", __func__);
      goto error_dsps_config;
  }
  memset(&sub, 0, sizeof(struct v4l2_event_subscription));
  sub.type = V4L2_EVENT_ALL;
  sub.id = 0;
  rc = ioctl(server_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_SUBSCRIBE_EVENT failed : %s\n", __func__,
      strerror(errno));
    goto error_subscribe;
  }

  config_arg.server_fd = server_fd;
  config_arg.ez_read_fd = ez_cmd_pipe[0];
  config_arg.ez_write_fd = ez_cmd_pipe[1];
  config_arg.ez_prev_read_fd = ez_prev_cmd_pipe[0];
  config_arg.ez_prev_write_fd = ez_prev_cmd_pipe[1];

  /* put logic to poll on command queue and on receiving
   * open command start a config thread
   */
  timeoutms = -1;
  do {
    CDBG("%s ...... entering config duty loop ...... \n", __func__);
    fds[0].fd = server_fd;
    fds[0].events = POLLPRI;
    poll_count = 1;
#ifdef EZTUNE_ENABLE
    fds[1].fd = ez_server_socket_id;
    fds[1].events = POLLIN;
    fds[2].fd = ez_prev_server_socket_id;
    fds[2].events = POLLIN;
    poll_count = 3;
#endif
    rc = poll(fds, poll_count, timeoutms);
    if (rc == 0) {
      CDBG_HIGH("%s: poll timed out. sleep 100ms",__func__);
      usleep(1000 * 100);
      continue;
    } else if (rc < 0) {
      CDBG_HIGH("%s: poll ERROR: %s. sleep 100ms", __func__, strerror(errno));
      usleep(1000 * 100);
      continue;
    } else {
      if (fds[0].revents & POLLPRI) { /* Server Node Wake Up */
        CDBG("%s: Received event at server\n", __func__);
        memset(ctrl_cmd_buffer, 0, MAX_SERVER_PAYLOAD_LENGTH);
        rc = qcamsvr_process_server_node_event(&config_arg, &mctl_node_info,
          &gesture_info, ctrl_cmd_buffer);
        if (rc < 0)
          CDBG_ERROR("%s: qcamsvr_process_server_node_event failed", __func__);
      }

#ifdef EZTUNE_ENABLE
      if ((fds[1].revents & POLLIN) == POLLIN) { /* EzTune Server */
        int client_socket_id;

        CDBG("%s: listening on server_socket_inet=%d", __func__,
          ez_server_socket_id);
        client_socket_id = accept(ez_server_socket_id,
          (struct sockaddr *)&addr_client_inet, &addr_client_len);
        CDBG("%s: accepted connection client_socket_id=%d\n", __func__,
          client_socket_id);
        if (client_socket_id > 0) {
          CDBG("%s: writing to pipe ez_cmd_pipe_id=%d\n", __func__,
            ez_cmd_pipe[1]);
          write(ez_cmd_pipe[1], &client_socket_id, sizeof(int));
        }
      }

      if ((fds[2].revents & POLLIN) == POLLIN) { /* EzTune Prev Server */
        int client_socket_id;

        CDBG("%s: listening on prev server_socket_inet=%d\n", __func__,
          ez_prev_server_socket_id);
        client_socket_id = accept(ez_prev_server_socket_id,
          (struct sockaddr *)&addr_client_inet, &addr_client_len);
        CDBG("%s: accepted connection prev client_socket_id=%d\n", __func__,
          client_socket_id);
        if (client_socket_id > 0) {
          CDBG("%s: writing to pipe ez_prev_cmd_pipe_id=%d\n", __func__,
            ez_prev_cmd_pipe[1]);
          write(ez_prev_cmd_pipe[1], &client_socket_id, sizeof(int));
        }
      }
#endif
    } /* Else for Poll rc */
  } while (1);

  /* Clean up and exit. */
  CDBG("Camera Daemon stopped!\n");

  sub.type = V4L2_EVENT_ALL;
  rc = ioctl(server_fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
  if (rc < 0)
    CDBG_ERROR("%s: ioctl VIDIOC_UNSUBSCRIBE_EVENT failed : %s\n", __func__,
      strerror(errno));

error_subscribe:
  free(ctrl_cmd_buffer);
error_dsps_config:
#ifdef EZTUNE_ENABLE
  if (ez_prev_cmd_pipe[0]) {
    close(ez_prev_cmd_pipe[0]);
    ez_prev_cmd_pipe[0] = 0;
  }
  if (ez_prev_cmd_pipe[1]) {
    close(ez_prev_cmd_pipe[1]);
    ez_prev_cmd_pipe[1] = 0;
  }
error_eztune_prev_cmd_pipe:
  if (ez_prev_server_socket_id) {
    close(ez_prev_server_socket_id);
    ez_prev_server_socket_id = 0;
  }
error_eztune_prev_server:
  if (ez_cmd_pipe[0]) {
    close(ez_cmd_pipe[0]);
    ez_cmd_pipe[0] = 0;
  }
  if (ez_cmd_pipe[1]) {
    close(ez_cmd_pipe[1]);
    ez_cmd_pipe[1] = 0;
  }
error_eztune_cmd_pipe:
  if (ez_server_socket_id) {
    close(ez_server_socket_id);
    ez_server_socket_id = 0;
  }
error_eztune_server:
#endif
  mctl_unload_comps();

  if (gesture_info.gesture_lib.ptr) {
    CDBG("Close gesture library\n");
    dlclose(gesture_info.gesture_lib.ptr);
    gesture_info.gesture_lib.ptr = NULL;
  }

error_mctl_load_comps:
  destroy_resource_table();
  if (server_fd) {
    close(server_fd);
    server_fd = 0;
  }
error_init:
  for (i=0; i < MSM_MAX_CAMERA_SENSORS; i++)
    if(mctl_node_info.mctl_node_name[i] != NULL)
      free((void*)mctl_node_info.mctl_node_name[i]);

  return 0;
}

int get_num_res(uint32_t mctl_id,
                comp_id_t comp_id, uint8_t *num_res)
{
  int i;
  int cnt = 0;

  pthread_mutex_lock(&qserver_res_mgr.mutex);
  for (i = 0; i < qserver_res_mgr.num_comps; i++) {
    if ((qserver_res_mgr.res[i]->comp_id == comp_id))
      cnt++;
  }
  *num_res = (uint8_t)cnt;
  pthread_mutex_unlock(&qserver_res_mgr.mutex);
  return 0;
}

static int check_res_existence(comp_res_req_info_t *res_req_info)
{
  int i;
  for (i = 0; i < qserver_res_mgr.num_comps; i++) {
    if ((qserver_res_mgr.res[i]->comp_id == res_req_info->comp_id))
      break;
  }
  if (i == qserver_res_mgr.num_comps) {
    CDBG_HIGH("%s: Availability check failed for comp id %d\n",
      __func__, res_req_info->comp_id);
    return -1;
  }
  return 0;
}

static comp_res_entries_t *check_res_availability
  (comp_res_req_info_t *res_req_info)
{
  int i;
  for (i = 0; i < qserver_res_mgr.num_comps; i++) {
    if ((qserver_res_mgr.res[i]->comp_id == res_req_info->comp_id)
        && (qserver_res_mgr.res[i]->num_reservations <
            qserver_res_mgr.res[i]->max_reservations))
      break;
  }
  if (i == qserver_res_mgr.num_comps) {
    CDBG_HIGH("%s: Availability check failed for comp id %d\n",
      __func__, res_req_info->comp_id);
    return NULL;
  }
  return qserver_res_mgr.res[i];
}

/* reserve resources. One compid at a time */
int qcamsvr_reserve_res(uint32_t mctl_id,
  comp_res_req_info_t *res_req_info, comp_ext_client_info_t* p_ext)
{

  mm_mctl_inst_t *tmp_mctl_struct;
  m_ctrl_t *tmp_pme = NULL;
  int rc, fd;
  comp_res_entries_t *comp_entry;
  struct msm_mctl_set_sdev_data set_data;

  if (NULL == p_ext) {
    if (!(tmp_mctl_struct = qcamsvr_find_mctl_inst(mctl_id))) {
      CDBG_ERROR("%s: cannot find mctl, id = %d", __func__,
        mctl_id);
      goto error;
    }
    tmp_pme = (m_ctrl_t *)tmp_mctl_struct->handle;
    if (!tmp_pme) {
      CDBG_ERROR("%s: pme not valid", __func__);
      goto error;
    }
    fd = tmp_pme->p_cfg_ctrl->camfd;
  } else {
    fd = p_ext->fd;
  }

  pthread_mutex_lock(&qserver_res_mgr.mutex);

  /*first check if the requested resoucre even exists. This is
    done becasue targets older than 8960 dont have a separate
    axi subdev in the kernel*/
  rc = check_res_existence(res_req_info);
  if (rc < 0) {
    pthread_mutex_unlock(&qserver_res_mgr.mutex);
    return -ENXIO;
  }
  /*then check if the stored resources are capable of
    fulfilling the request*/
  comp_entry = check_res_availability(res_req_info);
  if (comp_entry == NULL) {
    CDBG_HIGH("%s: Could not find available resource for comp id %d\n",
      __func__, res_req_info->comp_id);
    pthread_mutex_unlock(&qserver_res_mgr.mutex);
    return -EBUSY;
  }

  /* reserve the resource if the capability check succeeds*/
  comp_entry->num_reservations++;
  set_data.revision = comp_entry->subdev_revision;
  switch (res_req_info->comp_id) {
  case MCTL_COMPID_VFE:
    set_data.sdev_type = VFE_DEV;
    break;
  case MCTL_COMPID_AXI:
    set_data.sdev_type = AXI_DEV;
    break;
  case MCTL_COMPID_VPE:
    set_data.sdev_type = VPE_DEV;
    break;
  case MCTL_COMPID_ISPIF:
    set_data.sdev_type = ISPIF_DEV;
    break;
  default:
    CDBG_HIGH("%s: Incorrect comp id\n", __func__);
    break;
  }

  rc = ioctl(fd, MSM_CAM_IOCTL_SET_MCTL_SDEV, &set_data);
  if (rc < 0) {
    CDBG_ERROR("%s: reserve ioctl failed; error=%d\n", __func__, rc);
    pthread_mutex_unlock(&qserver_res_mgr.mutex);
    goto error;
  }
  res_req_info->sdev_revision = comp_entry->subdev_revision;
  pthread_mutex_unlock(&qserver_res_mgr.mutex);
  return 0;
error:
  return -EINVAL;
}

/* release resource. One compid at a time. */
void qcamsvr_release_res(uint32_t mctl_id,
  comp_id_t res_comp_id,  comp_ext_client_info_t* p_ext)
{
  mm_mctl_inst_t *tmp_mctl_struct;
  int i, rc, fd;
  m_ctrl_t *tmp_pme = NULL;
  struct msm_mctl_set_sdev_data set_data;

  if (NULL == p_ext) {
    if (!(tmp_mctl_struct = qcamsvr_find_mctl_inst(mctl_id))) {
      CDBG_ERROR("%s: cannot find mctl, id = %d", __func__,
        mctl_id);
      return;
    }
    tmp_pme = (m_ctrl_t *)tmp_mctl_struct->handle;
    if (!tmp_pme) {
      CDBG_ERROR("%s: pme not valid", __func__);
      return;
    }
    fd = tmp_pme->p_cfg_ctrl->camfd;
  } else {
    fd = p_ext->fd;
  }
  pthread_mutex_lock(&qserver_res_mgr.mutex);
  switch (res_comp_id) {
  case MCTL_COMPID_VFE:
    set_data.sdev_type = VFE_DEV;
    break;
  case MCTL_COMPID_AXI:
    set_data.sdev_type = AXI_DEV;
    break;
  case MCTL_COMPID_VPE:
    set_data.sdev_type = VPE_DEV;
    break;
  case MCTL_COMPID_ISPIF:
    set_data.sdev_type = ISPIF_DEV;
    break;
  default:
    CDBG_HIGH("%s: Incorrect comp id\n", __func__);
    break;
  }

  rc = ioctl(fd, MSM_CAM_IOCTL_UNSET_MCTL_SDEV, &set_data);
  if (rc < 0) {
    CDBG_ERROR("%s: release ioctl failed; error=%d\n", __func__, rc);
  }
  for (i = 0; i < qserver_res_mgr.num_comps; i++) {
    if ((qserver_res_mgr.res[i]->comp_id == res_comp_id)
        && (qserver_res_mgr.res[i]->num_reservations > 0)) {
      qserver_res_mgr.res[i]->num_reservations--;
      break;
    }
  }
  if (i == qserver_res_mgr.num_comps)
    CDBG("%s: Could not find available resource for comp id %d\n",
      __func__, res_comp_id);
  pthread_mutex_unlock(&qserver_res_mgr.mutex);
  return;
}


