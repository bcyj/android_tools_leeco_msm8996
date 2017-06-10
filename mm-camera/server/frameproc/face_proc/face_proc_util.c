/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#include <sys/time.h>
#include "camera_dbg.h"
#include "face_proc_util.h"
#include "faceproc_engine.h"




/*============================================================================
            EXTERNAL VARIABLES DEFINITIONS
============================================================================*/
static pthread_t fd_thread;
static pthread_cond_t fd_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t fd_mutex = PTHREAD_MUTEX_INITIALIZER;
void * face_proc_thread(void *arg);
void fd_hist_process(frame_proc_t *frameCtrl, fd_ctrl_t *fdCtrl);
/*===========================================================================

FUNCTION:  FD_proc_init

RETURN VALUE:
1 - success
0 - failure
============================================================================*/
int fd_util_init(void * Ctrl, fd_ctrl_t  *fdCtrl)
{
  int rc;
  frame_proc_t *frameCtrl = (frame_proc_t *) Ctrl;
  fdCtrl->fd_lib_loaded = true;
  fdCtrl->fd_state = FD_STATE_INIT;
  fdCtrl->frame_ctrl = frameCtrl;
  /* Initialize Omron Engine */
  rc = faceproc_engine_create(fdCtrl);
  if (rc != 0) {
    CDBG_FD("FD Engine create failed");
    fdCtrl->fd_lib_loaded = false;
    fdCtrl->fd_state = FD_STATE_OFF;
    return -1;
  }
  /* Config the Omron engine */
  CDBG_FD("%s Display dim %d %d", __func__, frameCtrl->input.mctl_info.
    display_dim.width, frameCtrl->input.mctl_info.display_dim.height);
  fdCtrl->config.frame_cfg.width    = frameCtrl->input.mctl_info.display_dim.width;
  fdCtrl->config.frame_cfg.height   = frameCtrl->input.mctl_info.display_dim.height;
  fdCtrl->config.frame_cfg.row_incr = frameCtrl->input.mctl_info.display_dim.width;
  fdCtrl->config.frame_cfg.format   = FD_FRAME_FORMAT_8BIT_GRAYSCALE;
  fdCtrl->config.face_cfg.min_face_size = MIN_FACE_SIZE;
  fdCtrl->config.face_cfg.max_face_size = MAX_FACE_SIZE;
  fdCtrl->config.face_cfg.max_num_face_to_detect =
    frameCtrl->output.fd_d.config_num_fd;
  fdCtrl->config.face_cfg.face_orientation_hint = FD_FACE_ORIENTATION_UNKNOWN;
  fdCtrl->config.face_cfg.rotation_range = FACE_ANGLE_TO_DETECT;
  fdCtrl->config.histogram_enable = FD_ENABLE_HISTOGRAM_GEN;
  rc = faceproc_engine_config(fdCtrl, &(fdCtrl->config));
  if (rc != 0) {
    CDBG_FD("Fd engine config failed");
    fdCtrl->fd_state = FD_STATE_OFF;
    return -1;
  }
  /* Spawn local fd thread */
  pthread_mutex_init(&fd_mutex, NULL);
  pthread_cond_init(&fd_cond, NULL);
  rc = pthread_create(&fd_thread, NULL, face_proc_thread, (void*)fdCtrl);
  /* Buffer allocations */
  fdCtrl->current_frame.buffer =
    malloc(fdCtrl->config.frame_cfg.height *fdCtrl->config.frame_cfg.width);
  if (fdCtrl->current_frame.buffer == NULL) {
    CDBG_ERROR("Current buffer Malloc failed");
    rc = -1;
  }
  if (rc!=0) {
    CDBG_FD("%s failed ",__func__);
    fd_util_exit(Ctrl,fdCtrl);
    return -1;
  }
  pthread_mutex_lock(&fd_mutex);
  fdCtrl->current_frame.frame_id = 0;
  fdCtrl->prev_frame_id = 0;
  fdCtrl->fd_state = FD_STATE_ON;
  pthread_mutex_unlock(&fd_mutex);
  fdCtrl->num_faces_total = MAX_ROI;
  return 0;
}  /* init */

/*===========================================================================

FUNCTION:  fd_util_exit

RETURN VALUE:
0 - success
-1 - failure
============================================================================*/
int fd_util_exit(void * Ctrl, fd_ctrl_t  *fdCtrl)
{
  int rc = 0;
  frame_proc_t *frameCtrl = (frame_proc_t *) Ctrl;
  if (!frameCtrl || !fdCtrl) {
    CDBG_FD("Returning as ctrl is NULL");
    return 0;
  }
  if (fdCtrl->fd_state ==FD_STATE_OFF) {
    CDBG_FD("%s: face detection libraries already destroyed",__func__);
    return 0;
  }
  frameCtrl->output.fd_d.fd_enable = 0;

  /*clean the result */
  memset(&(frameCtrl->output.fd_d.roi),0,sizeof(frame_proc_fd_roi_t));
  pthread_mutex_lock(&fd_mutex);
  fdCtrl->fd_state = FD_STATE_DESTROYING;
  pthread_mutex_unlock(&fd_mutex);
  pthread_cond_signal(&fd_cond);
  /* Join fd thread */
  pthread_join(fd_thread, NULL);

  CDBG_FD("FD clean up\n");
  if (fdCtrl->fd_lib_loaded)
    rc = faceproc_engine_destroy(fdCtrl);
  if (rc != 0)
    CDBG_ERROR("%s: FD Engine destroy failed",__func__);
  fdCtrl->fd_lib_loaded = false;
  /* free allocated frame buffer */
  if (fdCtrl->current_frame.buffer) {
    free(fdCtrl->current_frame.buffer);
    fdCtrl->current_frame.buffer = NULL;
  }
  CDBG_FD("FD output cleaned\n");
  pthread_mutex_destroy(&fd_mutex);
  pthread_cond_destroy(&fd_cond);
  fdCtrl->fd_state = FD_STATE_OFF;
  CDBG_FD("%s X\n", __func__);
  return 0;
}

/*===========================================================================

FUNCTION:  face_proc_execute

RETURN VALUE:
1 - success
0 - failure
============================================================================*/
int32_t fd_util_execute(void *Ctrl, fd_ctrl_t *fdCtrl)
{
  int    i,rc;
  frame_proc_t *frameCtrl = (frame_proc_t *)Ctrl;
  pthread_mutex_lock(&fd_mutex);
  if (fdCtrl->fd_state == FD_STATE_BUSY ||
    fdCtrl->fd_state == FD_STATE_OFF ||
    fdCtrl->fd_state == FD_STATE_INIT) {
    CDBG_FD("FD Engine is busy/off");
    pthread_mutex_unlock(&fd_mutex);
    return 0;
  }
  if (!fdCtrl->fd_lib_loaded) {
    CDBG_ERROR("Ignoring FD processing as Library is not loaded");
    return 0;
  }
  fdCtrl->fd_state = FD_STATE_BUSY;
  pthread_mutex_unlock(&fd_mutex);
  switch (fdCtrl->mode) {
    case FACE_DETECT:
    case FACE_RECOGNIZE:
      pthread_cond_signal(&fd_cond);
      if (fdCtrl->current_frame.frame_id != fdCtrl->prev_frame_id) {
        /* New ouput obtained from fd engine*/
        fdCtrl->prev_frame_id = fdCtrl->current_frame.frame_id;
        frameCtrl->output.fd_d.frame_id = fdCtrl->current_frame.frame_id;
        frameCtrl->output.fd_d.output_updated = 1;
        frameCtrl->output.fd_d.type = ROI_TYPE_FACE;
      } else
        frameCtrl->output.fd_d.output_updated = 0;
      break;
    case FACE_REGISTER:

      rc = faceproc_engine_run(fdCtrl);
      if (rc != 0) {
        CDBG_ERROR("Face Proc Image Registration Failed!!!");
        fdCtrl->fd_state = FD_STATE_ON;
        return -1;
      }
      fdCtrl->fd_state = FD_STATE_ON;
      break;
    default:
      break;
  }
  return 0;
}  /* execute */

/*===========================================================================

FUNCTION:  face_proc_thread

RETURN VALUE:
1 - success
0 - failure
============================================================================*/

void * face_proc_thread(void *arg)
{
  int rc;
  fd_ctrl_t *fdCtrl = (fd_ctrl_t *)arg;
  frame_proc_t *frameCtrl = fdCtrl->frame_ctrl;
  for (;;) {
    /* mutex lock */
    pthread_mutex_lock(&fd_mutex);
    /* wait till a new frame comes */
    if (fdCtrl->fd_state != FD_STATE_DESTROYING &&
      fdCtrl->fd_state != FD_STATE_BUSY) {
      CDBG_FD("%s: waiting for frame state =%d\n", __func__,fdCtrl->fd_state);
      pthread_cond_wait(&fd_cond, &fd_mutex);
    }
    if (fdCtrl->fd_state == FD_STATE_DESTROYING) {
      CDBG_FD("fd_thread exits \n");
      pthread_mutex_unlock(&fd_mutex);
      return NULL;
    }
    pthread_mutex_unlock(&fd_mutex);
    CDBG_FD("%s gets new frame, state =%d\n",__func__, fdCtrl->fd_state);
    /* mutex unlock */
    /* pass the new frame to engine */
    rc = faceproc_engine_run(fdCtrl);
    if (rc != 0)
      CDBG_ERROR("Face Proc run Failed!!!");
    else {
      rc = faceproc_engine_get_output(fdCtrl,&(frameCtrl->output.fd_d));
      if ((fdCtrl->config.histogram_enable & FD_ENABLE_HISTOGRAM_GEN) &&
        (frameCtrl->output.fd_d.num_faces_detected >0))
        fd_hist_process(frameCtrl, fdCtrl);
    }
    pthread_mutex_lock(&fd_mutex);
    if (fdCtrl->fd_state != FD_STATE_DESTROYING)
      fdCtrl->fd_state = FD_STATE_ON;
    fdCtrl->current_frame.frame_id++;
    pthread_mutex_unlock(&fd_mutex);
  }
}
/*===========================================================================
FUNCTION      fd_hist_process

DESCRIPTION
  post process the histogram of the region

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void fd_hist_process(frame_proc_t *frameCtrl,fd_ctrl_t *fdCtrl)
{
  uint32_t face_index;
  int y,x,bottom,right;
  frame_proc_fd_roi_t * fd_output;
  for (face_index = 0; face_index < frameCtrl->output.fd_d.num_faces_detected;
    face_index++) {
    fd_output = &(frameCtrl->output.fd_d.roi[face_index]);
    /* Generate histogram if necessary */
    right  = fd_output->face_boundary.x +
      fd_output->face_boundary.dx;
    bottom =fd_output->face_boundary.y +
      fd_output->face_boundary.dy;
    fd_output->histogram.num_samples =
      fd_output->face_boundary.dx *
      fd_output->face_boundary.dy;
    memset(&(fd_output->histogram.bin[0]),
      0, FD_HIST_SIZE*sizeof(uint32_t));
    for (y = fd_output->face_boundary.y;y < bottom; y++) {
      for (x = fd_output->face_boundary.x;x < right; x++) {
        int pixel = fdCtrl->current_frame.
          buffer[y * fdCtrl->config.frame_cfg.row_incr + x];
        fd_output->histogram.bin[pixel]++;
      }
    }
  }
}  /* fd_hist_process */
