/* ============================================================================
  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
 ============================================================================*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <linux/msm_kgsl.h>
#include <sys/mman.h>
#include <ctype.h>
#include <time.h>
#include <poll.h>
#include "camera_dbg.h"
#include "mctl_pp.h"
#include "c2d_client_intf.h"

#if 0
#undef CDBG
#define CDBG LOGE
#endif

#define ENABLE_C2D_DUMP 0

static c2d_intf_t *c2d_intf[8];
static uint32_t c2d_open_count = 0;

/*==============================================================================
 * FUNCTION    - c2d_get_ctrl_info -
 *
 * DESCRIPTION:
 *============================================================================*/
static c2d_ctrl_info_t *c2d_get_ctrl_info(uint32_t handle)
{
  int idx  = handle & 0xff;
  if(idx >= 8 || !c2d_intf[idx] || c2d_intf[idx]->handle != handle)
    return NULL;
  else
    return &(c2d_intf[idx]->obj);
} /* c2d_get_ctrl_info */

/*==============================================================================
 * FUNCTION    - c2d_thread_ready_signal -
 *
 * DESCRIPTION:
 *============================================================================*/
static void c2d_thread_ready_signal(c2d_ctrl_info_t *ctrl)
{
  CDBG("%s: E\n", __func__);

  pthread_mutex_lock(&ctrl->thread_data.mutex);
  ctrl->thread_data.is_c2d_thread_ready = TRUE;
  pthread_cond_signal(&ctrl->thread_data.cond_v);
  pthread_mutex_unlock(&ctrl->thread_data.mutex);

  CDBG("%s: X\n", __func__);
} /* c2d_thread_ready_signal */

/*==============================================================================
 * FUNCTION    - c2d_thread_release -
 *
 * DESCRIPTION:
 *============================================================================*/
static void c2d_thread_release(c2d_ctrl_info_t *ctrl)
{
  int rc = -1, i = 0;
  c2d_thread_msg_t msg;

  CDBG("%s: E\n", __func__);

  msg.type = C2D_CMD_EXIT;
  msg.data = NULL;

  do {
    CDBG("%s: Send C2D_CMD_EXIT. Attemp = %d", __func__, i++);
    rc = write(ctrl->thread_data.c2d_in_pipe_fds[1], &msg, sizeof(msg));
  } while (rc <= 0);

  if (pthread_join(ctrl->thread_data.pid, NULL) != 0)
    CDBG("%s: pthread dead already\n", __func__);

  pthread_mutex_destroy(&ctrl->thread_data.mutex);
  pthread_cond_destroy(&ctrl->thread_data.cond_v);

  CDBG("%s: X\n", __func__);
} /* c2d_thread_release */

/*==============================================================================
 * FUNCTION    - c2d_dump_img -
 *
 * DESCRIPTION:
 *============================================================================*/
static void c2d_dump_img(c2d_surface_param_t *surface, int num)
{
  char out_file_name[128];
  snprintf(out_file_name,128, "/data/dump_%d.yuv", num);
  int out_file_fd;
  out_file_fd = open(out_file_name, O_RDWR | O_CREAT, 0777);

  CDBG("%s: file fd = %d \n", __func__, out_file_fd);
  CDBG("%s: vaddr: 0x%x size: %d\n", __func__,
    (int)surface->buf.frame.mp[0].vaddr, (surface->buf.frame.mp[0].length +
    surface->buf.frame.mp[1].length));

  write(out_file_fd, (void *)surface->buf.frame.mp[0].vaddr,
    (surface->buf.frame.mp[0].length + surface->buf.frame.mp[1].length));

  close(out_file_fd);
} /* c2d_dump_img */

/*==============================================================================
 * FUNCTION    - c2d_copy_buffer -
 *
 * DESCRIPTION:
 *============================================================================*/
static int c2d_copy_buffer(c2d_buf_t *src, c2d_buf_t *dst)
{
  int i;
  uint32_t temp_addr;

  if (dst->frame.num_planes == 1) {
    if (src->frame.num_planes == 1) {
      CDBG("%s: dst single planar, src single planar.\n", __func__);
      memcpy((void *)(dst->frame.sp.vaddr + dst->frame.sp.y_off),
        (void *)(src->frame.sp.vaddr + src->frame.sp.y_off),
        src->frame.sp.length);
    } else if (src->frame.num_planes > 1) {
      CDBG("%s: dst single planar, src multi planar.\n", __func__);
      temp_addr = dst->frame.sp.vaddr + dst->frame.sp.y_off;
      for (i = 0; i < src->frame.num_planes; i++) {
        memcpy((void *)(temp_addr),
          (void *)(src->frame.mp[i].vaddr + src->frame.mp[i].data_offset),
          src->frame.mp[i].length);
        temp_addr += src->frame.mp[i].length;
      }
    }
  } else if (dst->frame.num_planes > 1) {
    if (src->frame.num_planes == 1) {
      CDBG("%s: dst multi planar, src single planar.\n", __func__);
      temp_addr = src->frame.sp.vaddr + src->frame.sp.y_off;
      for (i = 0; i < dst->frame.num_planes; i++) {
        memcpy((void *)(dst->frame.mp[i].vaddr + dst->frame.mp[i].data_offset),
          (void *)(temp_addr), dst->frame.mp[i].length);
        temp_addr += dst->frame.mp[i].length;
      }
    } else if (src->frame.num_planes > 1) {
      CDBG("%s: dst multi planar, src multi planar.\n", __func__);
      for (i = 0; i < src->frame.num_planes; i++)
        memcpy((void *)(dst->frame.mp[i].vaddr + dst->frame.mp[i].data_offset),
          (void *)(src->frame.mp[i].vaddr + src->frame.mp[i].data_offset),
          src->frame.mp[i].length);
    }
  } else {
    CDBG_ERROR("%s: dst Invalid planar.\n", __func__);
    goto fail;
  }

  return 0;
fail:
  return -1;
} /* c2d_copy_buffer */

/*==============================================================================
 * FUNCTION    - c2d_skip_check -
 *
 * DESCRIPTION: If C2D skip then return 1 else 0.
 *============================================================================*/
static uint32_t c2d_skip_check(c2d_ctrl_info_t *ctrl)
{
  c2d_surface_param_t *src = &ctrl->current_cmd->params.src;
  c2d_surface_param_t *dst = &ctrl->current_cmd->params.dst;
  c2d_draw_params_t *draw_params =
    &ctrl->current_cmd->params.mode_data.draw_params;

  if ((ctrl->current_cmd->params.mode == C2D_IMAGE_DRAW) &&
    ((src->surfaceDef.width % 32) != 0) &&
    ((dst->surfaceDef.width % 32) != 0)) {

    if ((src->roi_cfg.x == dst->roi_cfg.x) &&
      (src->roi_cfg.y == dst->roi_cfg.y) &&
      (src->roi_cfg.width == dst->roi_cfg.width) &&
      (src->roi_cfg.height == dst->roi_cfg.height) &&
      (src->surfaceDef.format == dst->surfaceDef.format) &&
      (draw_params->target_config == C2D_TARGET_ROTATE_0)) {
      CDBG("%s: memcpy src to dst and skip C2D.", __func__);

      if (c2d_copy_buffer(&src->buf, &dst->buf) < 0) {
        CDBG_HIGH("%s: c2d_copy_buffer failed. Send frame to C2D.\n", __func__);
      } else {
        CDBG("%s: c2d_copy_buffer is success.\n", __func__);
        goto success;
      }
    } else {
      CDBG("%s: src & dst are different. Can't skip C2D.\n", __func__);
    }
  } else {
    CDBG("%s: src or dst is 32 byte aligned. Can't skip C2D.\n", __func__);
  }

  return 0;
success:
  return 1;
} /* c2d_skip_check */

/*==============================================================================
 * FUNCTION    - c2d_processing_thread -
 *
 * DESCRIPTION:
 *============================================================================*/
void *c2d_processing_thread(void *data)
{
  int rc = 0, i, counter = 0;
  c2d_ctrl_info_t *ctrl = (c2d_ctrl_info_t *)data;
  struct pollfd read_fd;
  int timeoutms = 3000;
  c2d_thread_msg_t msg;
  mctl_pp_cmd_t cmd;
  struct timespec before_time, after_time;
  c2d_surface_param_t *src = NULL;
  c2d_surface_param_t *dst = NULL;
  c2d_draw_params_t *draw_params = NULL;

  CDBG("%s: E", __func__);

  c2d_thread_ready_signal(ctrl);

  do {
    read_fd.fd = ctrl->thread_data.c2d_in_pipe_fds[0];
    read_fd.events = POLLIN|POLLRDNORM;

    rc = poll(&read_fd, 1, timeoutms);
    CDBG("%s: Woke Up", __func__);
    if (rc > 0) {
      memset(&msg, 0, sizeof(msg));
      rc = read(ctrl->thread_data.c2d_in_pipe_fds[0], &msg, sizeof(msg));
      if (rc < 0) {
        CDBG("%s: Error in Reading.", __func__);
        continue;
      }

      memset(&cmd, 0, sizeof(cmd));
      cmd.cmd_type = QCAM_MCTL_CMD_DATA;
      cmd.evt_type = MSM_CAM_RESP_MCTL_PP_EVENT;
      cmd.pp_event.event = MCTL_PP_EVENT_CMD_ACK;
      /* Will be used to decide which c2d client instance,
       * this ACK should be routed to.*/
      cmd.pp_event.ack.cookie = ctrl->handle;

      src = &ctrl->current_cmd->params.src;
      dst = &ctrl->current_cmd->params.dst;
      draw_params = &ctrl->current_cmd->params.mode_data.draw_params;

      CDBG("%s: msg.type = %d", __func__, msg.type);
      switch (msg.type) {
        case C2D_CMD_DRAW:
          if (NULL != msg.data) {
            cmd.pp_event.ack.cmd = *((unsigned int *)msg.data);
            free(msg.data);
            msg.data = NULL;
          }
          CDBG("%s: msg.data = %d", __func__, cmd.pp_event.ack.cmd);

          clock_gettime(CLOCK_REALTIME, &before_time);
#ifndef VFE_40
          if (c2d_skip_check(ctrl)) {
            CDBG("%s: src & dst have same config. Skip C2D & do memcpy.\n",
              __func__);
            rc = C2D_STATUS_OK;
          } else {
            CDBG("%s: src id: %d dest id: %d\n", __func__, src->id, dst->id);
            rc = ctrl->c2d_lib->c2dDraw(draw_params->target_id,
              draw_params->target_config, draw_params->target_scissor,
              draw_params->target_mask_id, draw_params->target_color_key,
              &draw_params->draw_obj, 1);

            if (rc != C2D_STATUS_OK)
              CDBG_ERROR("%s: c2dDraw failed.\n", __func__);
            else
              rc = ctrl->c2d_lib->c2dFinish(ctrl->current_cmd->params.dst.id);
          }
#else
          CDBG("%s: src id: %d dest id: %d\n", __func__, src->id, dst->id);
          rc = ctrl->c2d_lib->c2dDraw(draw_params->target_id,
            draw_params->target_config, draw_params->target_scissor,
            draw_params->target_mask_id, draw_params->target_color_key,
            &draw_params->draw_obj, 1);

          if (rc != C2D_STATUS_OK)
            CDBG_ERROR("%s: c2dDraw failed.\n", __func__);
          else
            rc = ctrl->c2d_lib->c2dFinish(ctrl->current_cmd->params.dst.id);
#endif
          clock_gettime(CLOCK_REALTIME, &after_time);

          CDBG("%s: C2D processing time=%d ms for src: %dx%d dst: %dx%d",
            __func__, (uint32_t)(((after_time.tv_sec*1000) +
            (after_time.tv_nsec/1000000)) - ((before_time.tv_sec*1000) +
            (before_time.tv_nsec/1000000))), src->roi_cfg.width,
            src->roi_cfg.height, dst->roi_cfg.width, dst->roi_cfg.height);

          if (rc != C2D_STATUS_OK) {
            CDBG_ERROR("%s: c2d processing failed. rc = %d\n", __func__, rc);
            cmd.pp_event.ack.status = -1;
          } else {
            cmd.pp_event.ack.status = 1;
          }

          if (ENABLE_C2D_DUMP) {
            if (counter < 10) {
              c2d_dump_img(&ctrl->current_cmd->params.src, counter);
              c2d_dump_img(&ctrl->current_cmd->params.dst, counter+10);
            }
          }
          counter++;

          if (ctrl->thread_data.stop_requested) {
            CDBG_HIGH("%s: stop is requested.", __func__);
            pthread_mutex_lock(&ctrl->thread_data.mutex);
            ctrl->thread_data.stop_requested = FALSE;
            pthread_cond_signal(&ctrl->thread_data.cond_v);
            pthread_mutex_unlock(&ctrl->thread_data.mutex);
            CDBG_HIGH("%s: Signaled waiting thread. Skip Event Ack.", __func__);
            continue;
          }

          i = 0;
          do {
            CDBG("%s: Send MCTL_PP_EVENT_CMD_ACK to fd=%d. Attempt = %d",
              __func__, ctrl->thread_data.c2d_out_pipe_fds[1], i++);
            rc = write(ctrl->thread_data.c2d_out_pipe_fds[1], &cmd, sizeof(cmd));
            ctrl->thread_data.sent_ack_to_pp = TRUE;
          } while (rc <= 0);

          CDBG("%s: C2D_CMD_FINISH Done.", __func__);
          break;
        case C2D_CMD_EXIT:
          pthread_mutex_lock(&ctrl->thread_data.mutex);
          ctrl->thread_data.is_c2d_thread_ready = FALSE;
          pthread_mutex_unlock(&ctrl->thread_data.mutex);
          break;
        default:
          CDBG_HIGH("%s: Invalid msg type = %d", __func__, msg.type);
          break;
      }
    } else {
      usleep(10);
      continue;
    }
  } while (ctrl->thread_data.is_c2d_thread_ready);

  CDBG("%s: X", __func__);
  return NULL;
} /* c2d_processing_thread */

/*==============================================================================
 * FUNCTION    - c2d_launch_thread -
 *
 * DESCRIPTION:
 *============================================================================*/
static int c2d_launch_thread(c2d_ctrl_info_t *ctrl)
{
  int rc = 0;

  CDBG("%s: X\n", __func__);

  pthread_mutex_init(&ctrl->thread_data.mutex, NULL);
  pthread_cond_init(&ctrl->thread_data.cond_v, NULL);

  pthread_mutex_lock(&ctrl->thread_data.mutex);

  ctrl->thread_data.stop_requested = FALSE;
  ctrl->thread_data.is_c2d_thread_ready = FALSE;
  rc = pthread_create(&ctrl->thread_data.pid, NULL, c2d_processing_thread,
    (void *)ctrl);
  if (rc < 0) {
    CDBG_ERROR("%s: Cannot launch c2d_processing_thread rc = %d", __func__, rc);
    return rc;
  }
  if (!ctrl->thread_data.is_c2d_thread_ready)
    pthread_cond_wait(&ctrl->thread_data.cond_v, &ctrl->thread_data.mutex);

  pthread_mutex_unlock(&ctrl->thread_data.mutex);

  CDBG("%s: X\n", __func__);
  return rc;
} /* c2d_launch_thread */

/*==============================================================================
 * FUNCTION    - c2d_get_new_gpu_addr -
 *
 * DESCRIPTION: Get gpu address for C2D surface definition.
 *============================================================================*/
static uint32_t c2d_get_new_gpu_addr(c2d_ctrl_info_t *ctrl, int fd,
  uint32_t len, uint32_t offset, uint32_t vAddr)
{
  int rc = C2D_STATUS_OK, flag;
  uint32_t gpuAddr;

#ifdef USE_ION
  flag = KGSL_USER_MEM_TYPE_ION;
#else
  flag = KGSL_USER_MEM_TYPE_PMEM;
#endif

  CDBG("%s: fd=%d, len=%d, offset=%d, vaddr=0x%x\n", __func__, fd, len, offset,
    vAddr);
  rc = ctrl->c2d_lib->c2dMapAddr(fd, (void *)vAddr, len, offset, flag,
    (void **)&gpuAddr);
  if (C2D_STATUS_OK != rc) {
    CDBG_ERROR("%s: c2dMapAddr failed. rc = %d\n", __func__, rc);
    return -1;
  }

  CDBG("%s: gpuAddress = 0x%x\n", __func__, gpuAddr);
  return gpuAddr;
} /* c2d_get_new_gpu_addr */

/*==============================================================================
 * FUNCTION    - c2d_unmap_gpu_addr -
 *
 * DESCRIPTION:  unregister C2D buffer with kernel and deallocate.
 *============================================================================*/
static int c2d_unmap_gpu_addr(c2d_ctrl_info_t *ctrl, uint32_t gAddr)
{
  int rc = C2D_STATUS_OK;

  CDBG("%s: unmap gpuAddr = 0x%x", __func__, gAddr);
  rc = ctrl->c2d_lib->c2dUnMapAddr((void *)gAddr);
  if (C2D_STATUS_OK != rc) {
    CDBG_ERROR("%s: c2dUnMapAddr failed for gpuAddr = 0x%x. rc = %d\n",
      __func__, gAddr, rc);
    return -1;
  }

  return 0;
} /* c2d_unmap_gpu_addr */

/*==============================================================================
 * FUNCTION    - c2d_get_process_mode -
 *
 * DESCRIPTION:
 *============================================================================*/
static c2d_process_mode_t c2d_get_process_mode(uint32_t mode_in)
{
  switch (mode_in) {
    case C2D_CMD_IMAGE_DRAW:
      return C2D_IMAGE_DRAW;
    case C2D_CMD_IMAGE_GEOCORRECT:
      return C2D_IMAGE_GEOCORRECT;
    default:
      return C2D_IMAGE_INVALID;
  }
} /* c2d_get_process_mode */

/*==============================================================================
 * FUNCTION    - c2d_delete_gpu_addr_list -
 *
 * DESCRIPTION:  Delete gpu address list and free allocated memory.
 *============================================================================*/
static void c2d_delete_gpu_addr_list(c2d_ctrl_info_t *ctrl)
{
  struct cam_list *pos1, *pos2;
  struct cam_list *head = &(ctrl->g_list.list);
  c2d_gpu_addr_list_t *entry;

  for (pos1 = head->next, pos2 = pos1->next; pos1 != head; pos1 = pos2,
    pos2 = pos1->next) {
    entry = member_of(pos1, c2d_gpu_addr_list_t, list);
    if (c2d_unmap_gpu_addr(ctrl, entry->data.gpuAddr) < 0)
      CDBG_ERROR("%s: c2d_unmap_gpu_addr failed. Ignore it for now.", __func__);
    cam_list_del_node(pos1);
    free(entry);
  }
} /* c2d_delete_gpu_addr_list */

/*==============================================================================
 * FUNCTION    - c2d_find_gpu_addr_item -
 *
 * DESCRIPTION:  Find pmem buffer and matching gpu address entry into list.
 *============================================================================*/
static uint32_t c2d_find_gpu_addr_item(c2d_ctrl_info_t *ctrl, int fd,
  uint32_t vAddr)
{
  struct cam_list *pos;
  struct cam_list *head = &(ctrl->g_list.list);
  c2d_gpu_addr_list_t *entry;

  CDBG("%s: Entry to find fd = %d, vAddr = 0x%x\n", __func__, fd, vAddr);
  for (pos = head->next; pos != head; pos = pos->next) {
    entry = member_of(pos, c2d_gpu_addr_list_t, list);
    CDBG("%s: Current Entry fd = %d vAddr = 0x%x gpuAddr = 0x%x\n", __func__,
      entry->data.fd, entry->data.vAddr, entry->data.gpuAddr);

    if ((entry->data.fd == fd) && (entry->data.vAddr == vAddr))
      return entry->data.gpuAddr;
  }

  CDBG("%s: entry not found\n", __func__);
  return 0;
} /* c2d_find_gpu_addr_item */

/*==============================================================================
 * FUNCTION    - c2d_add_gpu_addr_item -
 *
 * DESCRIPTION:  Add pmem buffer and matching gpu address entry into list.
 *============================================================================*/
static void c2d_add_gpu_addr_item(c2d_ctrl_info_t *ctrl, int fd, uint32_t vAddr,
  uint32_t gpuAddr)
{
  c2d_gpu_addr_list_t *entry;
  entry = (c2d_gpu_addr_list_t *)malloc(sizeof(c2d_gpu_addr_list_t));
  if (!entry) {
    CDBG_ERROR("%s: malloc error\n", __func__);
    return;
  }

  entry->data.fd = fd;
  entry->data.vAddr = vAddr;
  entry->data.gpuAddr = gpuAddr;

  cam_list_add_tail_node(&(entry->list), &(ctrl->g_list.list));

  CDBG("%s: entry fd = %d, vAddr = 0x%x, gAddr = 0x%x\n", __func__,
    entry->data.fd, entry->data.vAddr, entry->data.gpuAddr);
} /* c2d_add_gpu_addr_item */

/*==============================================================================
 * FUNCTION    - c2d_get_gpu_addr -
 *
 * DESCRIPTION: Get gpu address for C2D surface definition.
 *============================================================================*/
static uint32_t c2d_get_gpu_addr(c2d_ctrl_info_t *ctrl, int fd, uint32_t len,
  uint32_t offset, uint32_t vAddr)
{
  int rc = 0;
  uint32_t gpuAddr;
  gpuAddr = c2d_find_gpu_addr_item(ctrl, fd, vAddr);
  if (gpuAddr) {
    CDBG("%s: found gpu addr for vaddr 0x%x gaddr 0x%x\n", __func__, vAddr,
      gpuAddr);
    return gpuAddr;
  } else {
    gpuAddr = c2d_get_new_gpu_addr(ctrl, fd, len, offset, vAddr);
    CDBG("%s: found new gpu addr for vaddr 0x%x gaddr 0x%x\n", __func__, vAddr,
      gpuAddr);
    c2d_add_gpu_addr_item(ctrl, fd, vAddr, gpuAddr);
    return gpuAddr;
  }

  return FALSE;
} /* c2d_get_gpu_addr */

/*==============================================================================
 * FUNCTION    - c2d_open_lib -
 *
 * DESCRIPTION:  Load C2D library and link necessary procedures.
 *============================================================================*/
static int c2d_open_lib(uint32_t handle)
{
  c2d_ctrl_info_t *ctrl = NULL;

  CDBG("%s: E ", __func__);
  ctrl = c2d_get_ctrl_info(handle);
  if (!ctrl) {
    CDBG_ERROR("%s: Invalid handle", __func__);
    return -EINVAL;
  }

  if (NULL == (ctrl->c2d_lib = (c2d_lib_t *)malloc(sizeof(c2d_lib_t)))) {
    CDBG_ERROR("%s: Malloc Error", __func__);
    return -ENOMEM;
  }

  ctrl->c2d_lib->ptr = NULL;
  ctrl->c2d_lib->ptr = dlopen("libC2D2.so", RTLD_NOW);
  if (!ctrl->c2d_lib->ptr) {
    CDBG_ERROR("%s ERROR: couldn't dlopen libc2d2.so: %s", __func__, dlerror());
    free(ctrl->c2d_lib);
    ctrl->c2d_lib = NULL;
    return -EINVAL;
  }

  *(void **)&ctrl->c2d_lib->c2dCreateSurface =
      dlsym(ctrl->c2d_lib->ptr, "c2dCreateSurface");
  *(void **)&ctrl->c2d_lib->c2dUpdateSurface =
      dlsym(ctrl->c2d_lib->ptr, "c2dUpdateSurface");
  *(void **)&ctrl->c2d_lib->c2dLensCorrection =
      dlsym(ctrl->c2d_lib->ptr, "c2dLensCorrection");
  *(void **)&ctrl->c2d_lib->c2dDraw =
      dlsym(ctrl->c2d_lib->ptr, "c2dDraw");
  *(void **)&ctrl->c2d_lib->c2dFinish =
      dlsym(ctrl->c2d_lib->ptr, "c2dFinish");
  *(void **)&ctrl->c2d_lib->c2dDestroySurface =
      dlsym(ctrl->c2d_lib->ptr, "c2dDestroySurface");
  *(void **)&ctrl->c2d_lib->c2dMapAddr =
      dlsym(ctrl->c2d_lib->ptr, "c2dMapAddr");
  *(void **)&ctrl->c2d_lib->c2dUnMapAddr =
      dlsym(ctrl->c2d_lib->ptr, "c2dUnMapAddr");

  if (!ctrl->c2d_lib->c2dCreateSurface || !ctrl->c2d_lib->c2dUpdateSurface ||
    !ctrl->c2d_lib->c2dFinish || !ctrl->c2d_lib->c2dDestroySurface ||
    !ctrl->c2d_lib->c2dLensCorrection || !ctrl->c2d_lib->c2dDraw ||
    !ctrl->c2d_lib->c2dMapAddr || !ctrl->c2d_lib->c2dUnMapAddr) {
    CDBG_ERROR("%s: ERROR mapping symbols from libc2d2.so", __func__);
    free(ctrl->c2d_lib);
    ctrl->c2d_lib = NULL;
    return -EINVAL;
  }

  CDBG("%s: X ", __func__);
  return 0;
} /* c2d_open_lib */

/*==============================================================================
 * FUNCTION    - c2d_add_cmd_item -
 *
 * DESCRIPTION:  Add c2d cmd to the list
 *============================================================================*/
static void c2d_add_cmd_item(c2d_ctrl_info_t *ctrl, c2d_process_mode_t mode,
  void *param_out)
{
  c2d_cmd_list_t *entry;
  entry = (c2d_cmd_list_t *)malloc(sizeof(c2d_cmd_list_t));
  if (!entry) {
    CDBG_ERROR("%s: malloc error\n", __func__);
    return;
  }

  entry->params.mode = mode;
  entry->params.src = ctrl->persist_params.src;
  entry->params.dst = ctrl->persist_params.dst;
  *(uint32_t *)param_out = (uint32_t)entry;

  cam_list_add_tail_node(&(entry->list), &(ctrl->cmd_list.list));

  CDBG("%s: entry cookie = 0x%x, mode = %s\n", __func__, (uint32_t)entry,
    (entry->params.mode == C2D_IMAGE_DRAW) ? "IMAGE_DRAW" : "IMAGE_GEOCORRECT");
} /* c2d_add_cmd_item */

/*==============================================================================
 * FUNCTION    - c2d_find_first_cmd_item -
 *
 * DESCRIPTION:  Find first cmd item from the cmd list
 *============================================================================*/
static void *c2d_find_first_cmd_item(c2d_ctrl_info_t *ctrl)
{
  struct cam_list *pos;
  struct cam_list *head = &(ctrl->cmd_list.list);
  c2d_cmd_list_t *entry;

  pos = head->next;
  if (pos != head) {
    entry = member_of(pos, c2d_cmd_list_t, list);
    CDBG("%s: Entry = %p\n", __func__, (uint32_t *)entry);
    return (void *)entry;
  }

  CDBG("%s: entry not found\n", __func__);
  return NULL;
} /* c2d_find_first_cmd_item */

/*==============================================================================
 * FUNCTION    - c2d_delete_cmd_item -
 *
 * DESCRIPTION:  Delete one entry from the cmd list and free allocated memory.
 *============================================================================*/
static void c2d_delete_cmd_item(c2d_ctrl_info_t *ctrl, void *entry)
{
  struct cam_list *pos1, *pos2;
  struct cam_list *head = &(ctrl->cmd_list.list);
  c2d_cmd_list_t *cur_entry;

  for (pos1 = head->next, pos2 = pos1->next; pos1 != head; pos1 = pos2,
    pos2 = pos1->next) {
    cur_entry = member_of(pos1, c2d_cmd_list_t, list);
    if ((uint32_t *)cur_entry == (uint32_t *)entry) {
      CDBG("%s: Entries Match, delete the current node", __func__);
      cam_list_del_node(pos1);
      free(entry);
      entry = NULL;
    }
  }
} /* c2d_delete_cmd_item */

/*==============================================================================
 * FUNCTION    - c2d_delete_cmd_list -
 *
 * DESCRIPTION:  Delete entire cmd list and free allocated memory.
 *============================================================================*/
static void c2d_delete_cmd_list(c2d_ctrl_info_t *ctrl)
{
  struct cam_list *pos1, *pos2;
  struct cam_list *head = &(ctrl->cmd_list.list);
  c2d_cmd_list_t *entry;

  for (pos1 = head->next, pos2 = pos1->next; pos1 != head; pos1 = pos2,
    pos2 = pos1->next) {
    entry = member_of(pos1, c2d_cmd_list_t, list);
    cam_list_del_node(pos1);
    free(entry);
  }
} /* c2d_delete_cmd_list */

/*==============================================================================
 * FUNCTION    - c2d_create_default_yuv_surface -
 *
 * DESCRIPTION: create C2D surface with default data.
 *============================================================================*/
static int c2d_create_default_yuv_surface(c2d_ctrl_info_t *ctrl,
  C2D_YUV_SURFACE_DEF *surface, uint32_t *id, C2D_SURFACE_BITS surface_bits,
  C2D_YUV_FORMAT format)
{
  int rc = 0;

  /* Create Source Surface */
  surface->format = format;
  surface->width = 1 * 4;
  surface->height = 1 * 4;
  surface->plane0 = (void*)0xaaaaaaaa;
  surface->phys0 = (void*)0xaaaaaaaa;
  surface->stride0 = 1 * 4;
  surface->plane1 = (void*)0xaaaaaaaa;
  surface->phys1 = (void*)0xaaaaaaaa;
  surface->stride1 = 1 * 4;
  surface->plane2 = (void*)0xaaaaaaaa;
  surface->phys2 = (void*)0xaaaaaaaa;
  surface->stride2 = 1 * 4;

  rc = ctrl->c2d_lib->c2dCreateSurface(id, surface_bits,
    (C2D_SURFACE_TYPE)(C2D_SURFACE_YUV_HOST | C2D_SURFACE_WITH_PHYS |
    C2D_SURFACE_WITH_PHYS_DUMMY), surface);

  if (rc != C2D_STATUS_OK) {
    CDBG_ERROR("%s: c2dCreateSurface failed. rc = %d\n", __func__, rc);
    return -1;
  }

  return 0;
} /* c2d_create_default_yuv_surface */

/*==============================================================================
 * FUNCTION    - c2d_get_target_rotation -
 *
 * DESCRIPTION:
 *============================================================================*/
static uint32_t c2d_get_target_rotation(camera_rotation_type type)
{
  switch (type) {
    case ROT_NONE:
      return C2D_TARGET_ROTATE_0;
    case ROT_CLOCKWISE_90:
      return C2D_TARGET_ROTATE_90;
    case ROT_CLOCKWISE_180:
      return C2D_TARGET_ROTATE_180;
    case ROT_CLOCKWISE_270:
      return C2D_TARGET_ROTATE_270;
    default:
      CDBG_ERROR("%s: Invalid camera_rotation_type = %d", __func__, type);
      return C2D_TARGET_ROTATE_0;
  }
} /* c2d_get_target_rotation */

/*==============================================================================
 * FUNCTION    - c2d_prepare_img_draw -
 *
 * DESCRIPTION:
 *============================================================================*/
static void c2d_prepare_img_draw(c2d_ctrl_info_t *ctrl)
{
  c2d_cmd_params_t *params = &ctrl->current_cmd->params;
  c2d_draw_params_t *draw_params = &params->mode_data.draw_params;
  C2D_OBJECT *draw_obj = &draw_params->draw_obj;

  draw_params->target_config = 0;
  draw_params->target_id = params->dst.id;
  draw_params->target_scissor = NULL;
  draw_params->target_mask_id = 0;
  draw_params->target_color_key = 0;
  draw_params->target_config |=
    c2d_get_target_rotation(ctrl->persist_params.rotation);

  CDBG("%s: dst id = %d target_config = %d", __func__, draw_params->target_id,
    draw_params->target_config);

  draw_obj->config_mask = 0;
  draw_obj->surface_id = params->src.id;
  draw_obj->source_rect.x = params->src.roi_cfg.x << 16;
  draw_obj->source_rect.y = params->src.roi_cfg.y << 16;
  draw_obj->source_rect.width = params->src.roi_cfg.width << 16;
  draw_obj->source_rect.height = params->src.roi_cfg.height << 16;
  draw_obj->config_mask |= C2D_SOURCE_RECT_BIT;

  CDBG("%s: src id = %d config_mask = %d", __func__, draw_obj->surface_id,
    draw_obj->config_mask);
  CDBG("%s: src x %d y %d width %d heigh %d\n", __func__,
    draw_obj->source_rect.x, draw_obj->source_rect.y,
    draw_obj->source_rect.width >> 16, draw_obj->source_rect.height >> 16);
} /* c2d_prepare_img_draw */

/*==============================================================================
 * FUNCTION    - c2d_get_frame_format -
 *
 * DESCRIPTION:
 *============================================================================*/
static C2D_YUV_FORMAT c2d_get_frame_format(cam_format_t cam_fmt)
{
  switch (cam_fmt) {
    case CAMERA_YUV_420_NV12:
      return C2D_COLOR_FORMAT_420_NV12;
    case CAMERA_YUV_420_NV21:
      return C2D_COLOR_FORMAT_420_NV21;
    case CAMERA_YUV_420_YV12:
      return C2D_COLOR_FORMAT_420_YV12;
    case CAMERA_YUV_422_YUYV:
      return C2D_COLOR_FORMAT_422_UYVY;
    default:
      CDBG_ERROR("%s: Invalid cam_fmt = %d", __func__, cam_fmt);
      return C2D_COLOR_FORMAT_420_NV12;
  }
} /* c2d_get_frame_format */

/*==============================================================================
 * FUNCTION    - c2d_set_surface_data -
 *
 * DESCRIPTION:
 *============================================================================*/
static void c2d_set_surface_data(c2d_dimension_cfg_t *dim,
  C2D_SURFACE_BITS type, c2d_surface_param_t* surface)
{
  surface->surfaceDef.format = c2d_get_frame_format(dim->cam_fmt);
  surface->surfaceDef.width = dim->width;
  surface->surfaceDef.height = dim->height;
  surface->surfaceDef.stride0 = dim->plane[0].stride;
  surface->surfaceDef.stride1 = dim->plane[1].stride;
  surface->surface_type = type;

  CDBG("%s: fmt=%d w=%d h=%d type=%d", __func__, surface->surfaceDef.format,
    surface->surfaceDef.width, surface->surfaceDef.height,
    surface->surface_type);
} /* c2d_set_surface_data */

/*==============================================================================
 * FUNCTION    - c2d_set_rotation -
 *
 * DESCRIPTION:
 *============================================================================*/
static void c2d_set_rotation(c2d_ctrl_info_t *ctrl, void *param_in,
  c2d_cmd_params_t *params)
{
  ctrl->persist_params.rotation = *(camera_rotation_type *)param_in;

  if (ctrl->persist_params.rotation == ROT_CLOCKWISE_90 ||
    ctrl->persist_params.rotation == ROT_CLOCKWISE_270) {
    params->dst.surfaceDef.width = ctrl->persist_params.dst.surfaceDef.height;
    params->dst.surfaceDef.height = ctrl->persist_params.dst.surfaceDef.width;
  }
} /* c2d_set_rotation */

/*==============================================================================
 * FUNCTION    - c2d_set_crop_data -
 *
 * DESCRIPTION:
 *============================================================================*/
static void c2d_set_crop_data(struct msm_pp_crop *crop, c2d_cmd_params_t *param)
{
  param->src.roi_cfg.x = crop->src_x;
  param->src.roi_cfg.y = crop->src_y;
  if (!crop->src_w && !crop->src_h) {
    param->src.roi_cfg.width = param->src.surfaceDef.width;
    param->src.roi_cfg.height = param->src.surfaceDef.height;
  } else {
    param->src.roi_cfg.width = crop->src_w;
    param->src.roi_cfg.height = crop->src_h;
  }
  CDBG("%s: src ROI x=%d y=%d w=%d h=%d", __func__, param->src.roi_cfg.x,
    param->src.roi_cfg.y, param->src.roi_cfg.width, param->src.roi_cfg.height);

  param->dst.roi_cfg.x = 0;
  param->dst.roi_cfg.y = 0;
  param->dst.roi_cfg.width = param->dst.surfaceDef.width;
  param->dst.roi_cfg.height = param->dst.surfaceDef.height;
  CDBG("%s: dst ROI x=%d y=%d w=%d h=%d", __func__, param->dst.roi_cfg.x,
    param->dst.roi_cfg.y, param->dst.roi_cfg.width,
    param->dst.roi_cfg.height);
} /* c2d_set_crop_data */

/*==============================================================================
 * FUNCTION    - c2d_prepare_surface -
 *
 * DESCRIPTION:
 *============================================================================*/
static int c2d_prepare_surface(c2d_ctrl_info_t *ctrl,
  c2d_surface_param_t* param)
{
  int rc = 0;
  C2D_YUV_SURFACE_DEF *srf = &param->surfaceDef;

  if (param->buf.frame.num_planes == 1) {
    CDBG("%s: Single Plane Surface", __func__);

    srf->plane0 = (void *)param->buf.frame.mp[0].vaddr;
    param->buf.mp_gAddr[0] = c2d_get_gpu_addr(ctrl, param->buf.frame.mp[0].fd,
      param->buf.frame.mp[0].length, param->buf.frame.mp[0].addr_offset,
      param->buf.frame.mp[0].vaddr);
    if (!param->buf.mp_gAddr[0]) {
      CDBG_ERROR("%s: Single Plane c2d_get_gpu_addr failed", __func__);
      return -1;
    }
    srf->phys0 = (void *)param->buf.mp_gAddr[0];
    srf->stride0 = srf->width * 2;
  } else if (param->buf.frame.num_planes > 1) {
    CDBG("%s: Multi Plane Surface", __func__);

#ifdef USE_ION
    srf->plane0 = (void *)(param->buf.frame.mp[0].vaddr +
      param->buf.frame.mp[0].data_offset);
    param->buf.mp_gAddr[0] = c2d_get_gpu_addr(ctrl, param->buf.frame.mp[0].fd,
      param->buf.frame.mp[0].length + param->buf.frame.mp[1].length,
      param->buf.frame.mp[0].addr_offset, param->buf.frame.mp[0].vaddr);
    if (!param->buf.mp_gAddr[0]) {
      CDBG_ERROR("%s: mp[0] c2d_get_gpu_addr failed", __func__);
      return -1;
    }
    srf->phys0 = (void *)(param->buf.mp_gAddr[0] +
      param->buf.frame.mp[0].data_offset);
    srf->plane1 = (void *)(param->buf.frame.mp[1].vaddr +
      param->buf.frame.mp[1].data_offset);
    param->buf.mp_gAddr[1] = param->buf.mp_gAddr[0] +
      (param->buf.frame.mp[1].vaddr - param->buf.frame.mp[0].vaddr);
    if (!param->buf.mp_gAddr[1]) {
      CDBG_ERROR("%s: mp[1] c2d_get_gpu_addr failed", __func__);
      return -1;
    }
    srf->phys1 = (void *)(param->buf.mp_gAddr[1] +
      param->buf.frame.mp[1].data_offset);
#else
    srf->plane0 = (void *)(param->buf.frame.mp[0].vaddr +
      param->buf.frame.mp[0].data_offset);
    param->buf.mp_gAddr[0] = c2d_get_gpu_addr(ctrl, param->buf.frame.mp[0].fd,
      param->buf.frame.mp[0].length, param->buf.frame.mp[0].addr_offset,
      param->buf.frame.mp[0].vaddr);
    if (!param->buf.mp_gAddr[0]) {
      CDBG_ERROR("%s: mp[0] c2d_get_gpu_addr failed", __func__);
      return -1;
    }
    srf->phys0 = (void *)(param->buf.mp_gAddr[0] +
      param->buf.frame.mp[0].data_offset);
    srf->stride0 = srf->width;

    srf->plane1 = (void *)(param->buf.frame.mp[1].vaddr +
      param->buf.frame.mp[1].data_offset);
    param->buf.mp_gAddr[1] = c2d_get_gpu_addr(ctrl, param->buf.frame.mp[1].fd,
      param->buf.frame.mp[1].length, param->buf.frame.mp[1].addr_offset,
      param->buf.frame.mp[1].vaddr);
    if (!param->buf.mp_gAddr[1]) {
      CDBG_ERROR("%s: mp[1] c2d_get_gpu_addr failed", __func__);
      return -1;
    }
    srf->phys1 = (void *)(param->buf.mp_gAddr[1] +
      param->buf.frame.mp[1].data_offset);
    srf->stride1 = srf->width;

    if (param->buf.frame.num_planes == 3) {
      srf->plane2 = (void *)(param->buf.frame.mp[2].vaddr +
        param->buf.frame.mp[2].data_offset);
      param->buf.mp_gAddr[2] = c2d_get_gpu_addr(ctrl, param->buf.frame.mp[2].fd,
        param->buf.frame.mp[2].length, param->buf.frame.mp[2].addr_offset,
        param->buf.frame.mp[2].vaddr);
      if (!param->buf.mp_gAddr[2]) {
        CDBG_ERROR("%s: mp[2] c2d_get_gpu_addr failed", __func__);
        return -1;
      }
      srf->phys2 = (void *)(param->buf.mp_gAddr[2] +
        param->buf.frame.mp[2].data_offset);
      srf->stride2 = srf->width;
    }
#endif
  } else {
    CDBG_ERROR("%s: Invalid number of planes %d", __func__,
      param->buf.frame.num_planes);
    return -1;
  }

  CDBG("%s: surface_id: %d surface_type: %d\n", __func__, param->id,
    param->surface_type);
  CDBG("%s: plane0: 0x%x phys0: 0x%x\n", __func__, (uint32_t)srf->plane0,
    (uint32_t)srf->phys0);
  CDBG("%s: plane1: 0x%x phys1: 0x%x\n", __func__, (uint32_t)srf->plane1,
    (uint32_t)srf->phys1);
  CDBG("%s: plane2: 0x%x phys2: 0x%x\n", __func__, (uint32_t)srf->plane2,
    (uint32_t)srf->phys2);
  CDBG("%s: stride0: %d stride1: %d stride2: %d\n", __func__, srf->stride0,
    srf->stride1, srf->stride2);

  rc = ctrl->c2d_lib->c2dUpdateSurface(param->id, param->surface_type,
    (C2D_SURFACE_TYPE)(C2D_SURFACE_YUV_HOST | C2D_SURFACE_WITH_PHYS),
    (void *)srf);

  if (rc != C2D_STATUS_OK) {
    CDBG_ERROR("%s: c2dUpdateSurface failed. rc = %d\n", __func__, rc);
    return -1;
  }

  return 0;
} /* c2d_prepare_surface */

/*==============================================================================
 * FUNCTION    - c2d_img_draw -
 *
 * DESCRIPTION:
 *============================================================================*/
static int c2d_img_draw(c2d_ctrl_info_t *ctrl)
{
  int rc = 0, i;
  unsigned int *data = NULL;
  c2d_thread_msg_t msg;

  CDBG("%s: E\n", __func__);

  data = (unsigned int *)malloc(sizeof(*data));
  if (NULL == data) {
    CDBG_ERROR("%s: malloc error\n", __func__);
    return -1;
  }
  *data = C2D_CMD_IMAGE_DRAW;

  msg.type = C2D_CMD_DRAW;
  msg.data = (void *)data;
  msg.len = sizeof(*data);

  if (c2d_prepare_surface(ctrl, &ctrl->current_cmd->params.src) != 0) {
    CDBG_ERROR("%s: src c2d_prepare_surface failed", __func__);
    goto error;
  }
  if (c2d_prepare_surface(ctrl, &ctrl->current_cmd->params.dst) != 0) {
    CDBG_ERROR("%s: dst c2d_prepare_surface failed", __func__);
    goto error;
  }

  c2d_prepare_img_draw(ctrl);

  i = 0;
  do {
    CDBG("%s: Send C2D_CMD_FINISH. Attempt = %d", __func__, i++);
    rc = write(ctrl->thread_data.c2d_in_pipe_fds[1], &msg, sizeof(msg));
  } while (rc <= 0);

  CDBG("%s: X\n", __func__);
  return 0;

error:
  if (NULL != data) {
    free(data);
    data = NULL;
  }
  return -1;
} /* c2d_img_draw */

/*==============================================================================
 * FUNCTION    - c2d_event_start -
 *
 * DESCRIPTION:
 *============================================================================*/
static int c2d_event_start(c2d_ctrl_info_t *ctrl, void *data)
{
  int rc = 0;
  mctl_pp_hw_init_data *init_data = (mctl_pp_hw_init_data *)data;
  C2D_YUV_FORMAT format;

  CDBG("%s: E ", __func__);

  if ((ctrl->state != C2D_STATE_INIT) && (ctrl->state != C2D_STATE_STOP)) {
    CDBG_ERROR("%s: Invalid state = %d", __func__, ctrl->state);
    return -1;
  }

  format = c2d_get_frame_format(init_data->src_format);
  CDBG("%s C2D_SOURCE surface fmt %d ", __func__, format);
  rc = c2d_create_default_yuv_surface(ctrl, &ctrl->persist_params.src.surfaceDef,
    &ctrl->persist_params.src.id, C2D_SOURCE, format);
  if (rc < 0) {
    CDBG_ERROR("%s: Default src surface creation failed", __func__);
    return -1;
  }

  format = c2d_get_frame_format(init_data->dest_format);
  CDBG("%s C2D_TARGET surface fmt %d ", __func__, format);
  rc = c2d_create_default_yuv_surface(ctrl, &ctrl->persist_params.dst.surfaceDef,
    &ctrl->persist_params.dst.id, C2D_TARGET, format);
  if (rc < 0) {
    CDBG_ERROR("%s: Default dst surface creation failed", __func__);
    rc = ctrl->c2d_lib->c2dDestroySurface(ctrl->persist_params.src.id);
    if (rc != C2D_STATUS_OK)
      CDBG_ERROR("%s: Destroying src surface failed. rc = %d", __func__, rc);
    return -1;
  }

  cam_list_init(&(ctrl->g_list.list));
  cam_list_init(&(ctrl->cmd_list.list));
  ctrl->thread_data.sent_ack_to_pp = FALSE;

  ctrl->state = C2D_STATE_START;
  CDBG("%s: ctrl->state = %d", __func__, ctrl->state);

  CDBG("%s: X ", __func__);
  return rc;
} /* c2d_event_start */

/*==============================================================================
 * FUNCTION    - c2d_event_stop -
 *
 * DESCRIPTION:
 *============================================================================*/
static int c2d_event_stop(c2d_ctrl_info_t *ctrl, void *data)
{
  int rc = 0;

  CDBG("%s: E ", __func__);

  if ((ctrl->state != C2D_STATE_START) && (ctrl->state != C2D_STATE_DOING)) {
    CDBG_ERROR("%s: Invalid state = %d", __func__, ctrl->state);
    return -1;
  }

  if (ctrl->state == C2D_STATE_DOING &&
      !ctrl->thread_data.sent_ack_to_pp) {
    CDBG("%s: Finish current operation and then flush the cache", __func__);
    pthread_mutex_lock(&ctrl->thread_data.mutex);
    ctrl->thread_data.stop_requested = TRUE;
    pthread_cond_wait(&ctrl->thread_data.cond_v, &ctrl->thread_data.mutex);
    pthread_mutex_unlock(&ctrl->thread_data.mutex);
    CDBG("%s: Done with current operation", __func__);
  }

  c2d_delete_cmd_list(ctrl);
  c2d_delete_gpu_addr_list(ctrl);

  ctrl->c2d_lib->c2dDestroySurface(ctrl->persist_params.src.id);
  ctrl->c2d_lib->c2dDestroySurface(ctrl->persist_params.dst.id);

  ctrl->state = C2D_STATE_STOP;
  CDBG("%s: ctrl->state = %d", __func__, ctrl->state);

  CDBG("%s: X ", __func__);
  return rc;
} /* c2d_event_stop */

/*==============================================================================
 * FUNCTION    - c2d_event_do_pp -
 *
 * DESCRIPTION:
 *============================================================================*/
static int c2d_event_do_pp(c2d_ctrl_info_t *ctrl, c2d_cmd_list_t *cmd)
{
  int rc = 0;

  CDBG("%s: E ", __func__);

  CDBG("%s: ctrl->state = %d", __func__, ctrl->state);
  switch (ctrl->state) {
    case C2D_STATE_START:
      CDBG("%s: Send the Frame to C2D for processing", __func__);
      break;
    case C2D_STATE_DOING:
      CDBG("%s: C2D is busy. This frame will be dequeued when C2D is not busy.",
        __func__);
      return rc;
    default:
      CDBG_ERROR("%s: Invalid State =%d", __func__, ctrl->state);
      return -1;
  }

  if (!cmd) {
    CDBG_ERROR("%s: cmd is NULL", __func__);
    return -1;
  }

  ctrl->current_cmd = cmd;
  ctrl->state = C2D_STATE_DOING;

  switch(cmd->params.mode) {
    case C2D_IMAGE_DRAW:
      rc = c2d_img_draw(ctrl);
      break;
    case C2D_IMAGE_GEOCORRECT:
      /* ToDo: When S3D is ready */
      break;
    default:
      ctrl->current_cmd = NULL;
      ctrl->state = C2D_STATE_START;
      CDBG_ERROR("%s: Invalid Process Mode = %d", __func__, cmd->params.mode);
      rc = -EINVAL;
      break;
  }

  CDBG("%s: X ", __func__);
  return rc;
} /* c2d_event_do_pp */

/*==============================================================================
 * FUNCTION    - c2d_event_ack -
 *
 * DESCRIPTION:
 *============================================================================*/
static int c2d_event_ack(c2d_ctrl_info_t *ctrl, void *data)
{
  int rc = 0;
  struct msm_mctl_pp_event_info *pp_event = NULL;
  pp_event = (struct msm_mctl_pp_event_info *)data;
  c2d_process_mode_t ack_mode;
  c2d_cmd_list_t *cmd = NULL;

  CDBG("%s: E ", __func__);

  ctrl->thread_data.sent_ack_to_pp = FALSE;
  /* If already stopped, just return back immediately. */
  if (ctrl->state == C2D_STATE_STOP) {
    CDBG_HIGH("%s: C2D already stopped, just return back ", __func__);
    return rc;
  }

  if (ctrl->state != C2D_STATE_DOING) {
    CDBG_ERROR("%s: Invalid State =%d", __func__, ctrl->state);
    return -1;
  }

  ack_mode = c2d_get_process_mode(pp_event->ack.cmd);

  if (ctrl->current_cmd == NULL) {
    CDBG_ERROR("%s: Current CMD is NULL", __func__);
    return -1;
  }

  if (ack_mode == ctrl->current_cmd->params.mode) {
    rc = ctrl->ops.buf_done((void *)ctrl->ops.parent,
      &ctrl->current_cmd->cb_data);
    if (rc < 0)
      CDBG_ERROR("%s: buf done call back failed. rc = %d", __func__, rc);

    c2d_delete_cmd_item(ctrl, (void *)ctrl->current_cmd);
  } else {
    CDBG_ERROR("%s: Error. Ack Mode = %d, Curr = %d", __func__, ack_mode,
      ctrl->current_cmd->params.mode);
  }

  ctrl->state = C2D_STATE_START;

  cmd = c2d_find_first_cmd_item(ctrl);
  if (cmd) {
    rc = c2d_event_do_pp(ctrl, cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: c2d_event_do_pp failed", __func__);
      c2d_delete_cmd_item(ctrl, (void *)ctrl->current_cmd);
      ctrl->current_cmd = NULL;
    }
  }

  CDBG("%s: X ", __func__);
  return rc;
} /* c2d_event_ack */

/*==============================================================================
 * FUNCTION    - c2d_process -
 *
 * DESCRIPTION:
 *============================================================================*/
int c2d_process(uint32_t handle, int event, void *data)
{
  int rc = 0;
  c2d_ctrl_info_t *ctrl = NULL;
  c2d_cmd_list_t *cmd = NULL;

  CDBG("%s: E ", __func__);
  ctrl = c2d_get_ctrl_info(handle);
  if (!ctrl) {
    CDBG_ERROR("%s: Invalid handle", __func__);
    return -EINVAL;
  }

  switch(event) {
    case C2D_EVENT_DO_PP:
      cmd = (c2d_cmd_list_t *)(*(uint32_t *)data);
      rc = c2d_event_do_pp(ctrl, cmd);
      if (rc < 0)
        CDBG_ERROR("%s: c2d_event_do_pp failed", __func__);
      break;
    case C2D_EVENT_ACK:
      rc = c2d_event_ack(ctrl, data);
      if (rc < 0)
        CDBG_ERROR("%s: c2d_event_ack failed", __func__);
      break;
  case C2D_EVENT_START:
      rc = c2d_event_start(ctrl, data);
      if (rc < 0)
        CDBG_ERROR("%s: c2d_event_start failed", __func__);
      break;
    case C2D_EVENT_STOP:
      rc = c2d_event_stop(ctrl, data);
      if (rc < 0)
        CDBG_ERROR("%s: c2d_event_stop failed", __func__);
      break;
    default:
      CDBG_ERROR("%s: Invalid event = %d", __func__, event);
      rc = -EINVAL;
      break;
  }

  CDBG("%s : X ", __func__);
  return rc;
} /* c2d_process */

/*==============================================================================
 * FUNCTION    - c2d_init -
 *
 * DESCRIPTION:
 *============================================================================*/
int c2d_init(uint32_t handle, mctl_ops_t *ops, void *init_data)
{
  int rc = 0;
  c2d_ctrl_info_t *ctrl = NULL;

  CDBG("%s: E ", __func__);
  ctrl = c2d_get_ctrl_info(handle);
  if (!ctrl) {
    CDBG_ERROR("%s: Invalid handle", __func__);
    return -EINVAL;
  }

  if (ctrl->state != C2D_STATE_NULL) {
    CDBG_ERROR("%s: Invalid C2D state = %d", __func__, ctrl->state);
    return -1;
  }

  if(ops)
    ctrl->ops = *ops;
  else
    memset(&ctrl->ops, 0, sizeof(ctrl->ops));

  ctrl->thread_data.c2d_out_pipe_fds[0] = *((int *)init_data);
  ctrl->thread_data.c2d_out_pipe_fds[1] = *(((int *)init_data) + 1);

  CDBG("%s: Received FD[0] = %d FD[1] = %d", __func__,
    ctrl->thread_data.c2d_out_pipe_fds[0], ctrl->thread_data.c2d_out_pipe_fds[1]);

  ctrl->state = C2D_STATE_INIT;
  CDBG("%s: ctrl->state = %d", __func__, ctrl->state);

  CDBG("%s : X", __func__);
  return rc;
} /* c2d_init */

/*==============================================================================
 * FUNCTION    - c2d_release -
 *
 * DESCRIPTION:
 *============================================================================*/
int c2d_release(uint32_t handle)
{
  int idx, rc = 0;
  c2d_ctrl_info_t *ctrl = NULL;

  CDBG("%s: E ", __func__);
  ctrl = c2d_get_ctrl_info(handle);
  if (!ctrl) {
    CDBG_ERROR("%s: Invalid handle", __func__);
    return -EINVAL;
  }

  if ((ctrl->state == C2D_STATE_NULL) || (ctrl->state == C2D_STATE_DOING))
    CDBG_ERROR("%s: Invalid state = %d", __func__, ctrl->state);

  c2d_thread_release(ctrl);

  if (ctrl->thread_data.c2d_in_pipe_fds[0])
    close(ctrl->thread_data.c2d_in_pipe_fds[0]);
  if (ctrl->thread_data.c2d_in_pipe_fds[1])
    close(ctrl->thread_data.c2d_in_pipe_fds[1]);

  idx  = handle & 0xff;
  if (idx >= 8) {
    CDBG("%s: Invalid index %d\n", __func__, idx);
    return -EINVAL;
  }

  c2d_open_count--;
  if (c2d_open_count == 0) {
    CDBG("%s: All c2d instances are closed. Deinit c2d_lib", __func__);
    if(ctrl->c2d_lib->ptr)
      dlclose(ctrl->c2d_lib->ptr);

    free(ctrl->c2d_lib);
  }

  if(c2d_intf[idx])
    free(c2d_intf[idx]);
  c2d_intf[idx] = NULL;

  CDBG("%s : X ", __func__);
  return rc;
} /* c2d_release */

/*==============================================================================
 * FUNCTION    - c2d_abort -
 *
 * DESCRIPTION:
 *============================================================================*/
void c2d_abort(uint32_t handle)
{
  c2d_ctrl_info_t *ctrl = NULL;

  CDBG("%s: E ", __func__);
  ctrl = c2d_get_ctrl_info(handle);
  if (!ctrl) {
    CDBG_ERROR("%s: Invalid handle", __func__);
    return;
  }

  CDBG("%s: X ", __func__);
} /* c2d_abort */

/*==============================================================================
 * FUNCTION    - c2d_get_params -
 *
 * DESCRIPTION:
 *============================================================================*/
int c2d_get_params(uint32_t handle, int type, void *param, int param_len)
{
  int rc = 0;
  c2d_ctrl_info_t *ctrl = NULL;

  CDBG("%s: E ", __func__);
  ctrl = c2d_get_ctrl_info(handle);
  if (!ctrl) {
    CDBG_ERROR("%s: Invalid handle", __func__);
    return -EINVAL;
  }

  CDBG("%s: X ", __func__);
  return rc;
} /* c2d_get_params */

/*==============================================================================
 * FUNCTION    - c2d_set_params -
 *
 * DESCRIPTION:
 *============================================================================*/
int c2d_set_params(uint32_t handle, int type, void *param_in, void *param_out)
{
  int rc = 0;
  c2d_ctrl_info_t *ctrl = NULL;
  c2d_cmd_list_t *cmd_entry;

  CDBG("%s: E ", __func__);
  ctrl = c2d_get_ctrl_info(handle);
  if (!ctrl) {
    CDBG_ERROR("%s: Invalid handle", __func__);
    return -EINVAL;
  }

  CDBG("%s: set param type = %d\n", __func__, type);
  switch(type) {
    case C2D_SET_INPUT_CFG: {
      c2d_set_surface_data((c2d_dimension_cfg_t *)param_in, C2D_SOURCE,
        &ctrl->persist_params.src);
      break;
    }
    case C2D_SET_OUTPUT_CFG: {
      c2d_set_surface_data((c2d_dimension_cfg_t *)param_in, C2D_TARGET,
        &ctrl->persist_params.dst);
      break;
    }
    case C2D_SET_PROCESS_MODE: {
      c2d_add_cmd_item(ctrl, *(c2d_process_mode_t *)param_in, param_out);
      break;
    }
    /* This set param is overridden for param_out */
    case C2D_SET_ROTATION_CFG: {
      cmd_entry = (c2d_cmd_list_t *)(*(uint32_t *)param_out);
      CDBG("%s: C2D_SET_ROTATION_CFG: cmd_entry = %p\n", __func__,
        (uint32_t *)cmd_entry);
      c2d_set_rotation(ctrl, param_in, &cmd_entry->params);
      break;
    }
    /* This set param is overridden for param_out */
    case C2D_SET_INPUT_BUF_CFG: {
      cmd_entry = (c2d_cmd_list_t *)(*(uint32_t *)param_out);
      CDBG("%s: C2D_SET_INPUT_BUF_CFG: cmd_entry = %p\n", __func__,
        (uint32_t *)cmd_entry);
      struct msm_pp_frame *buf = &(cmd_entry->params.src.buf.frame);
      *buf = *(struct msm_pp_frame *)param_in;
      break;
    }
    /* This set param is overridden for param_out */
    case C2D_SET_OUTPUT_BUF_CFG: {
      cmd_entry = (c2d_cmd_list_t *)(*(uint32_t *)param_out);
      CDBG("%s: C2D_SET_OUTPUT_BUF_CFG: cmd_entry = %p\n", __func__,
        (uint32_t *)cmd_entry);
      struct msm_pp_frame *buf = &(cmd_entry->params.dst.buf.frame);
      *buf = *(struct msm_pp_frame *)param_in;
      break;
    }
    /* This set param is overridden for param_out */
    case C2D_SET_CROP_CFG: {
      cmd_entry = (c2d_cmd_list_t *)(*(uint32_t *)param_out);
      CDBG("%s: C2D_SET_CROP_CFG: cmd_entry = %p\n", __func__,
        (uint32_t *)cmd_entry);
      c2d_set_crop_data((struct msm_pp_crop *)param_in, &cmd_entry->params);
      break;
    }
    /* This set param is overridden for param_out */
    case C2D_SET_CB_DATA: {
      cmd_entry = (c2d_cmd_list_t *)(*(uint32_t *)param_out);
      CDBG("%s: C2D_SET_CB_DATA: cmd_entry = %p\n", __func__,
        (uint32_t *)cmd_entry);
      cmd_entry->cb_data = *(pp_frame_data_t *)param_in;
      break;
    }
    default:
      rc = -EINVAL;
      break;
  }

  CDBG("%s: X ", __func__);
  return rc;
} /* c2d_set_params */

/*==============================================================================
 * FUNCTION    - c2d_interface_create -
 *
 * DESCRIPTION:
 *============================================================================*/
uint32_t c2d_interface_create(module_ops_t *c2d_ops)
{
  int idx, rc = 0;
  c2d_intf_t *c2d_intf_obj = NULL;
  static c2d_lib_t *c2d_lib_p = NULL;

  CDBG("%s: E ", __func__);

  for(idx = 0; idx < 8; idx++)
    if(c2d_intf[idx] == NULL)
      break;

  if (idx >= 7) {
    CDBG_ERROR("%s: Error. Maxed out open c2d intf instances ", __func__);
    return (uint32_t)NULL;
  } else {
    if (NULL == (c2d_intf_obj = malloc(sizeof(c2d_intf_t)))) {
      CDBG_ERROR("%s : malloc error ", __func__);
      return (uint32_t)NULL;
    }

    memset(c2d_intf_obj, 0, sizeof(c2d_intf_t));
    c2d_intf[idx] = c2d_intf_obj;
    c2d_intf_obj->handle = ((++c2d_open_count) << 8) + idx;

    c2d_ops->handle = c2d_intf[idx]->handle;
    c2d_ops->init = c2d_init;
    c2d_ops->set_params = c2d_set_params;
    c2d_ops->get_params = c2d_get_params;
    c2d_ops->process = c2d_process;
    c2d_ops->abort = c2d_abort;
    c2d_ops->destroy = c2d_release;

    /* Note: c2d_lib is shared across all instances of c2d.
     *       So init it once and then share the pointer.
     */
    if (c2d_open_count == 1) {
      if (c2d_open_lib(c2d_intf_obj->handle) < 0) {
        CDBG_ERROR("%s: Cannot open libC2D2. \n", __func__);
        goto error1;
      }
      c2d_lib_p = c2d_intf[idx]->obj.c2d_lib;
      CDBG("%s: c2d_lib_p %p", __func__, c2d_lib_p);
    } else {
      if (c2d_lib_p != NULL) {
        c2d_intf[idx]->obj.c2d_lib = c2d_lib_p;
        CDBG("%s: c2d_intf[idx]->obj.c2d_lib %p", __func__,
          c2d_intf[idx]->obj.c2d_lib);
      } else {
        CDBG_ERROR("%s: c2d_lib_p is not valid", __func__);
      }
    }

    rc = pipe(c2d_intf_obj->obj.thread_data.c2d_in_pipe_fds);
    if (rc < 0) {
      CDBG_ERROR("%s: C2D IN PIPE creation failed. rc = %d", __func__, rc);
      goto error2;
    }

    rc = c2d_launch_thread(&c2d_intf_obj->obj);
    if (rc < 0) {
      CDBG_ERROR("%s: c2d_launch_thread failed", __func__);
      goto error3;
    }

    c2d_intf_obj->obj.state = C2D_STATE_NULL;
    c2d_intf_obj->obj.handle = c2d_intf_obj->handle;
    CDBG_HIGH("%s: idx = %d handle = %d", __func__, idx, c2d_intf_obj->obj.handle);
    CDBG("%s: ctrl->state = %d", __func__, c2d_intf_obj->obj.state);

    CDBG("%s: X ", __func__);
    return (uint32_t)c2d_intf_obj->handle;
  }

error3:
  if (c2d_intf_obj->obj.thread_data.c2d_in_pipe_fds[0])
    close(c2d_intf_obj->obj.thread_data.c2d_in_pipe_fds[0]);
  if (c2d_intf_obj->obj.thread_data.c2d_in_pipe_fds[1])
    close(c2d_intf_obj->obj.thread_data.c2d_in_pipe_fds[1]);
error2:
  if(c2d_intf[idx]->obj.c2d_lib->ptr)
    dlclose(c2d_intf[idx]->obj.c2d_lib->ptr);
  free(c2d_intf[idx]->obj.c2d_lib);
  c2d_lib_p = NULL;

error1:
  c2d_open_count--;
  c2d_ops->init = NULL;
  c2d_ops->set_params = NULL;
  c2d_ops->get_params = NULL;
  c2d_ops->process = NULL;
  c2d_ops->abort = NULL;
  c2d_ops->destroy = NULL;
  c2d_ops->handle = -1;
  c2d_intf[idx] = NULL;
  free(c2d_intf_obj);

  return (uint32_t)NULL;
} /* c2d_interface_create */
