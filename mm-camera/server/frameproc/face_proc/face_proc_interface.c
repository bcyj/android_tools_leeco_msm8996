/**********************************************************************
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
**********************************************************************/
#include <string.h>
#include "face_proc_util.h"

static fd_ctrl_t *fdCtrl[MAX_INSTANCES];
const char FACE_ALBUM[] =  "/data/fdAlbum";
static pthread_mutex_t face_proc_mutex = PTHREAD_MUTEX_INITIALIZER;
/*==========================================================================
 * FUNCTION    - face_proc_init -
 *
 * DESCRIPTION:
 *=========================================================================*/
int face_proc_init(frame_proc_t *frameCtrl, fd_mode_t mode)
{
  CDBG_HIGH("%s E", __func__);
  uint32_t index = frameCtrl->handle & 0xFF;
  if (frameCtrl->fd_init_flag) {
    fdCtrl[index]->mode = mode;
    frameCtrl->output.fd_d.fd_mode = mode;
    return 0;
  }
  fdCtrl[index] = malloc(sizeof(fd_ctrl_t));
  if (!fdCtrl[index])
    return -1;
  fd_ctrl_t *fd = fdCtrl[index];
  memset(fdCtrl[index], 0, sizeof(fd_ctrl_t));
  if (fd_util_init(frameCtrl, fd) != 0) {
    CDBG_ERROR("%s fd_util_init fail",__func__);
    free(fd);
    fd = NULL;
    return 0;
  }
  frameCtrl->fd_init_flag = 1;
  fd->mode = mode;
  frameCtrl->output.fd_d.fd_skip_cnt = 0;
  frameCtrl->output.fd_d.num_faces_detected = 0;
  pthread_mutex_init(&face_proc_mutex,NULL);
  return 0;
}  /* face_dectect_init */
/*===========================================================================
 * FUNCTION    - face_proc_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int face_proc_set_params(frame_proc_t *frameCtrl, frame_proc_set_fd_data_t *data)
{
  int rc = 0;
  FILE *fp;
  CDBG_HIGH("%s E", __func__);

  switch (data->type) {
    case FACE_DETECT_ENABLE:
      frameCtrl->output.fd_d.fd_enable = data->fd_enable;
      if (frameCtrl->output.fd_d.fd_enable) {
#ifdef MM_CAMERA_FD
        if(data->num_fd <= 0 && data->num_fd > MAX_ROI)
           frameCtrl->output.fd_d.config_num_fd = MAX_ROI;
        else
           frameCtrl->output.fd_d.config_num_fd = data->num_fd;
        if (face_proc_init(frameCtrl,data->mode) != 0)
          rc = -1;
#endif
      } else {
#ifdef MM_CAMERA_FD
        if (face_proc_exit(frameCtrl) != 0)
          rc = -1;
#endif
      }
      break;
    case FACE_CLEAR_ALBUM:
      fp = fopen(FACE_ALBUM, "wb");
      if (fp != NULL)
        fclose (fp);
      break;
    default :
      break;
  }
  return rc;
}  /* face_proc_set_params */

/*===========================================================================
 * FUNCTION    - face_proc_execute -
 *
 * DESCRIPTION:
 *==========================================================================*/
int face_proc_execute(frame_proc_t *frameCtrl)
{
  int i,rc = -1;
  uint32_t index = frameCtrl->handle & 0xFF;

  if (index >= MAX_INSTANCES) {
    CDBG_ERROR("%s: Invalid handle!", __func__);
    return -1;
  }

  fd_ctrl_t *fd = fdCtrl[index];
  struct msm_pp_frame * pframe = NULL;
  if (!frameCtrl->fd_init_flag) {
    CDBG_HIGH("%s Face library not initialized", __func__);
    return 0;
  }
  CDBG_HIGH("%s: E", __func__);
  if (!fd)
    return 0;

  if (fd->fd_state == FD_STATE_OFF || fd->fd_state == FD_STATE_BUSY || FD_STATE_DESTROYING == fd->fd_state) {
    CDBG_ERROR("Face Engine is busy. Hence rejecting frame %d ",fd->fd_state);
    return 0;
  }
  switch (frameCtrl->output.fd_d.fd_mode) {
    case FACE_DETECT:
    case FACE_RECOGNIZE:
      pframe = &(frameCtrl->input.mctl_info.frame);
      /* Validate frame */
      pthread_mutex_lock(&face_proc_mutex);
      if (!pframe ||!(pframe->mp[0].vaddr)||!(pframe->mp[1].vaddr)||(fd->current_frame.buffer == NULL)) {
        CDBG_ERROR("FD_proc_start: empty frame!");
        pthread_mutex_unlock(&face_proc_mutex);
        return -1;
      }
      memcpy((uint8_t *)fd->current_frame.buffer,
        (uint8_t *)pframe->mp[0].vaddr+pframe->mp[0].data_offset,
        pframe->mp[0].length);
      rc = fd_util_execute(frameCtrl,fd);
      pthread_mutex_unlock(&face_proc_mutex);
      break;
    case FACE_REGISTER:

      for (i=0; i<frameCtrl->input.mctl_info.num_thumb_img; i++) {
        pframe = &(frameCtrl->input.mctl_info.thumb_img_frame[i]);
        /* Validate thumb frame */
        if (!pframe ||!(pframe->mp[0].vaddr)||!(pframe->mp[1].vaddr)) {
          CDBG_ERROR("FD_proc_start: thumb frame is invalid/empty!");
          return -1;
        }
        memcpy((uint8_t *)fd->current_frame.buffer,
          (uint8_t *)pframe->mp[0].vaddr+pframe->mp[0].data_offset,
          pframe->mp[0].length);
        rc = fd_util_execute(frameCtrl,fd);
      }
      break;
    default:
      break;
  }

  return rc;
}  /* face_proc_execute */

/*===========================================================================
 * FUNCTION    - face_proc_exit -
 *
 * DESCRIPTION:
 *==========================================================================*/
int face_proc_exit(frame_proc_t *frameCtrl)
{
  int rc = 0;
  uint32_t index = frameCtrl->handle & 0xFF;
  CDBG_HIGH("%s E", __func__);

  if (index >= MAX_INSTANCES) {
    CDBG_ERROR("%s: Invalid handle!", __func__);
    return -1;
  }

  fd_ctrl_t *fd = fdCtrl[index];
  if (!frameCtrl->fd_init_flag)
    return rc;
  pthread_mutex_lock(&face_proc_mutex);
  rc = fd_util_exit(frameCtrl, fd);
  if (fdCtrl[index]) {
    free(fdCtrl[index]);
    fdCtrl[index] = NULL;
  }
  frameCtrl->fd_init_flag = 0;
  frameCtrl->output.fd_d.fd_skip_cnt = 0;
  pthread_mutex_unlock(&face_proc_mutex);
  return rc;
}  /* face_proc_exit */
