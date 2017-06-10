/* ============================================================================
  Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
 ============================================================================*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include "camera.h"
#include "camera_dbg.h"
#include "config.h"
#include "config_proc.h"
#include "stereocam.h"
#include <dlfcn.h>

static pthread_t stereocam_analysis_thread_id;
static int stereocam_analysis_exit = 0;
static int is_stereocam_analysis_thread_ready = 0;
pthread_cond_t stereocam_analysis_thread_ready_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t stereocam_analysis_thread_ready_mutex =
  PTHREAD_MUTEX_INITIALIZER;
#if 0
static int st_analysis_terminate_fd[2];

static void *stereocam_analysis (void *data);

int launch_stereocam_analysis_thread(void *data)
{
  stereocam_analysis_exit = 0;
  is_stereocam_analysis_thread_ready = 0;

  return pthread_create(&stereocam_analysis_thread_id, NULL,
                        stereocam_analysis, (void *)data);
} /* launch_stereocam_analysis_thread */

int wait_stereocam_analysis_ready() {
  pthread_mutex_lock(&stereocam_analysis_thread_ready_mutex);

  if (!is_stereocam_analysis_thread_ready)
    pthread_cond_wait(&stereocam_analysis_thread_ready_cond,
                      &stereocam_analysis_thread_ready_mutex);

  pthread_mutex_unlock(&stereocam_analysis_thread_ready_mutex);
  return stereocam_analysis_exit;
} /* wait_stereocam_analysis_ready */

int release_stereocam_analysis_thread(void)
{
  int rc;
  char end = 'y';
  stereocam_analysis_exit = 1;
  rc = write(st_analysis_terminate_fd[1], &end, sizeof(end));

  if (rc <0)
    CDBG_HIGH("camframe termination failed : Failed\n");

  return pthread_join(stereocam_analysis_thread_id, NULL);
} /* release_stereocam_analysis_thread */

void stereocam_analysis_ready_signal(void)
{
  /*
   * Send signal to config thread to indicate that stereocam_analysis
   * thread is ready.
   */
  CDBG("stereocam_analysis() is ready, call pthread_cond_signal\n");

  pthread_mutex_lock(&stereocam_analysis_thread_ready_mutex);
  is_stereocam_analysis_thread_ready = 1;
  pthread_cond_signal(&stereocam_analysis_thread_ready_cond);
  pthread_mutex_unlock(&stereocam_analysis_thread_ready_mutex);

  CDBG("stereocam_analysis() is ready, call pthread_cond_signal done\n");
} /* stereocam_analysis_ready_signal */

/*===========================================================================
 * FUNCTION    - init_lib3d -
 *
 * DESCRIPTION:  Load lib3d library and link necessary procedures.
 *==========================================================================*/
static int init_lib3d(config_ctrl_t *ctrl)
{
  stereo_ctrl_t *stCtrl = (stereo_ctrl_t *)&ctrl->stereoCtrl;
  int rc, i, j;

  stCtrl->lib3d.lib3d_obj = dlopen("libmmstereo.so", RTLD_NOW);
  if (!stCtrl->lib3d.lib3d_obj) {
    CDBG_ERROR("FATAL ERROR: could not dlopen libmmstereo.so: %s", dlerror());
    return FALSE;
  }

  *(void **)&stCtrl->lib3d.s3d_get_version =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_get_version");
  *(void **)&stCtrl->lib3d.s3d_create =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_create");
  *(void **)&stCtrl->lib3d.s3d_config =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_config");
  *(void **)&stCtrl->lib3d.s3d_set_display_angle_of_view =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_set_display_angle_of_view");
  *(void **)&stCtrl->lib3d.s3d_set_display_distance =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_set_display_distance");
  *(void **)&stCtrl->lib3d.s3d_set_acceptable_angular_disparity =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_set_acceptable_angular_disparity");
  *(void **)&stCtrl->lib3d.s3d_set_convergence_lpf_weight =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_set_convergence_lpf_weight");
  *(void **)&stCtrl->lib3d.s3d_set_active_analysis_window =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_set_active_analysis_window");
  *(void **)&stCtrl->lib3d.s3d_get_correction_matrix =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_get_correction_matrix");
  *(void **)&stCtrl->lib3d.s3d_reset_history =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_reset_history");
  *(void **)&stCtrl->lib3d.s3d_run_convergence =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_run_convergence");
  *(void **)&stCtrl->lib3d.s3d_adjust_3d_effect =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_adjust_3d_effect");
  *(void **)&stCtrl->lib3d.s3d_abort =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_abort");
  *(void **)&stCtrl->lib3d.s3d_destroy =
    dlsym(stCtrl->lib3d.lib3d_obj, "s3d_destroy");

  if (!stCtrl->lib3d.s3d_get_version                    ||
    !stCtrl->lib3d.s3d_create                           ||
    !stCtrl->lib3d.s3d_config                           ||
    !stCtrl->lib3d.s3d_set_display_angle_of_view        ||
    !stCtrl->lib3d.s3d_set_display_distance             ||
    !stCtrl->lib3d.s3d_set_acceptable_angular_disparity ||
    !stCtrl->lib3d.s3d_set_convergence_lpf_weight       ||
    !stCtrl->lib3d.s3d_set_active_analysis_window       ||
    !stCtrl->lib3d.s3d_get_correction_matrix            ||
    !stCtrl->lib3d.s3d_reset_history                    ||
    !stCtrl->lib3d.s3d_run_convergence                  ||
    !stCtrl->lib3d.s3d_adjust_3d_effect                 ||
    !stCtrl->lib3d.s3d_abort                            ||
    !stCtrl->lib3d.s3d_destroy) {
    CDBG_ERROR("%s: dlsym ERROR", __func__);
    return FALSE;
  }

  rc = stCtrl->lib3d.s3d_create(&stCtrl->lib3d.s3d_param);
  if (rc != S3D_RET_SUCCESS) {
    CDBG_HIGH("%s: s3d_create failed with rc = %d\n", __func__, rc);
    return FALSE;
  }

  if (!stCtrl->procFrame.non_zoom_upscale) {
    stCtrl->lib3d.s3d_cfg_data.analysis_mono_width =
      stCtrl->procFrame.left_pack_dim.modified_w;
    stCtrl->lib3d.s3d_cfg_data.analysis_mono_height =
      stCtrl->procFrame.left_pack_dim.modified_h;
  } else {
    stCtrl->lib3d.s3d_cfg_data.analysis_mono_width =
      stCtrl->procFrame.left_pack_dim.orig_w;
    stCtrl->lib3d.s3d_cfg_data.analysis_mono_height =
      stCtrl->procFrame.left_pack_dim.orig_h;
  }

  DOUBLE_2_FLOAT(3, 4, stCtrl->sensorOTPMatrix.left_matrix,
    ctrl->sensorCtrl.cali_data_3d.left_p_matrix);
  stCtrl->lib3d.s3d_cfg_data.proj_matrix_left =
    (float *)stCtrl->sensorOTPMatrix.left_matrix;

#ifdef PRINT_STEREO_MATRIX
  CDBG_HIGH("%s: 3/4 Double OTP Left Matrix\n", __func__);
  PRINT_2D_MATRIX(3, 4, ctrl->sensorCtrl.cali_data_3d.left_p_matrix);
  CDBG_HIGH("%s: 3/4 Float OTP Left Matrix\n", __func__);
  PRINT_2D_MATRIX(3, 4, stCtrl->sensorOTPMatrix.left_matrix);
#endif

  DOUBLE_2_FLOAT(3, 4, stCtrl->sensorOTPMatrix.right_matrix,
    ctrl->sensorCtrl.cali_data_3d.right_p_matrix);
  stCtrl->lib3d.s3d_cfg_data.proj_matrix_right =
    (float *)stCtrl->sensorOTPMatrix.right_matrix;

#ifdef PRINT_STEREO_MATRIX
  CDBG_HIGH("%s: 3/4 Double OTP Right Matrix\n", __func__);
  PRINT_2D_MATRIX(3, 4, ctrl->sensorCtrl.cali_data_3d.right_p_matrix);
  CDBG_HIGH("%s: 3/4 Float OTP Right Matrix\n", __func__);
  PRINT_2D_MATRIX(3, 4, stCtrl->sensorOTPMatrix.right_matrix);
#endif

  stCtrl->lib3d.s3d_cfg_data.analysis_img_format =
    stereocam_get_lib3d_format(stCtrl->procFrame.packing);

  rc = stCtrl->lib3d.s3d_config(stCtrl->lib3d.s3d_param,
    &stCtrl->lib3d.s3d_cfg_data);
  if (rc != S3D_RET_SUCCESS) {
    CDBG_HIGH("%s: s3d_config failed with rc = %d\n", __func__, rc);
    return FALSE;
  }

  if (stCtrl->lib3d.display_distance) {
    rc = stCtrl->lib3d.s3d_set_display_distance(stCtrl->lib3d.s3d_param,
        stCtrl->lib3d.display_distance);
    if (rc != S3D_RET_SUCCESS) {
      CDBG_HIGH("%s: s3d_set_display_distance failed with rc = %d\n",
        __func__, rc);
      return FALSE;
    }
  }

  if (stCtrl->lib3d.view_angle) {
    rc = stCtrl->lib3d.s3d_set_display_angle_of_view(stCtrl->lib3d.s3d_param,
        stCtrl->lib3d.view_angle);
    if (rc != S3D_RET_SUCCESS) {
      CDBG_HIGH("%s: s3d_set_display_angle_of_view failed with rc = %d\n",
        __func__, rc);
      return FALSE;
    }
  }

  if (!stereocam_get_correction_matrix(stCtrl, &stCtrl->videoFrame)) {
    CDBG_HIGH("%s: stereocam_get_correction_matrix for VideoFrame failed \n",
      __func__);
    return FALSE;
  }

  stCtrl->convMode = ST_CONV_AUTO;

  return TRUE;
} /* init_lib3d */

/*===========================================================================
 * FUNCTION    - deinit_lib3d -
 *
 * DESCRIPTION:  Close lib3d library and un-link necessary procedures.
 *==========================================================================*/
static void deinit_lib3d(config_ctrl_t *ctrl)
{
  stereo_ctrl_t *stCtrl = (stereo_ctrl_t *)&ctrl->stereoCtrl;
  int rc;

  rc = stCtrl->lib3d.s3d_destroy(stCtrl->lib3d.s3d_param);

  if (rc != S3D_RET_SUCCESS)
    CDBG_HIGH("%s: s3d_destroy failed with rc = %d\n", __func__, rc);

  *(void **)&stCtrl->lib3d.s3d_get_version = NULL;
  *(void **)&stCtrl->lib3d.s3d_create = NULL;
  *(void **)&stCtrl->lib3d.s3d_config = NULL;
  *(void **)&stCtrl->lib3d.s3d_set_display_angle_of_view = NULL;
  *(void **)&stCtrl->lib3d.s3d_set_display_distance = NULL;
  *(void **)&stCtrl->lib3d.s3d_set_acceptable_angular_disparity = NULL;
  *(void **)&stCtrl->lib3d.s3d_set_convergence_lpf_weight = NULL;
  *(void **)&stCtrl->lib3d.s3d_set_active_analysis_window = NULL;
  *(void **)&stCtrl->lib3d.s3d_get_correction_matrix = NULL;
  *(void **)&stCtrl->lib3d.s3d_reset_history = NULL;
  *(void **)&stCtrl->lib3d.s3d_run_convergence = NULL;
  *(void **)&stCtrl->lib3d.s3d_adjust_3d_effect = NULL;
  *(void **)&stCtrl->lib3d.s3d_destroy = NULL;
  *(void **)&stCtrl->lib3d.s3d_abort = NULL;

  dlclose(stCtrl->lib3d.lib3d_obj);

  stCtrl->lib3d.lib3d_obj = NULL;
} /* deinit_lib3d */

/*===========================================================================
 * FUNCTION    - stereocam_analysis -
 *
 * DESCRIPTION: stereocam analysis thread which will talk to VPE & C2D
 *==========================================================================*/
static void *stereocam_analysis(void *data)
{
  fd_set analysis_fds;
  int analysis_nfds = 0;
  int rc;
  config_ctrl_t *cctrl = (config_ctrl_t *)data;
  stereo_ctrl_t *stCtrl = (stereo_ctrl_t *)&cctrl->stereoCtrl;
  struct msm_cam_evt_msg camMsg;
  struct msm_st_frame input_frame;
  struct msm_frame output_frame;
  float right_shift;
  uint32_t *mono_left_w = NULL, *mono_right_w = NULL;
  uint32_t *mono_left_h = NULL, *mono_right_h = NULL;
  uint32_t mono_w_scale = 0, mono_h_scale = 0;

  if (pipe(st_analysis_terminate_fd)< 0) {
    CDBG_ERROR("%s: thread termination pipe creation failed\n", __func__);
    return NULL;
  }

  CDBG("%s: st_analysis_terminate_fds %d %d\n", __func__,
    st_analysis_terminate_fd[0], st_analysis_terminate_fd[1]);

  if (!init_lib3d(cctrl)) {
    CDBG_HIGH("%s: init_lib3d failed\n", __func__);
    return NULL;
  }

  FIND_STEREO_SIZE_FACTOR(stCtrl->procFrame.packing, FALSE,
    mono_w_scale, mono_h_scale);

  if (stCtrl->procFrame.non_zoom_upscale) {
    mono_left_w = &stCtrl->procFrame.left_pack_dim.orig_w;
    mono_left_h = &stCtrl->procFrame.left_pack_dim.orig_h;
    mono_right_w = &stCtrl->procFrame.right_pack_dim.orig_w;
    mono_right_h = &stCtrl->procFrame.right_pack_dim.orig_h;
  } else {
    mono_left_w = &stCtrl->procFrame.left_pack_dim.modified_w;
    mono_left_h = &stCtrl->procFrame.left_pack_dim.modified_h;
    mono_right_w = &stCtrl->procFrame.right_pack_dim.modified_w;
    mono_right_h = &stCtrl->procFrame.right_pack_dim.modified_h;
  }

  stCtrl->right_image_shift = 0;
  stCtrl->left_image_shift = 0;

  stereocam_analysis_ready_signal();
  do {
    struct timeval analysis_timeout;
    analysis_timeout.tv_usec = 0;
    analysis_timeout.tv_sec = 6;

    FD_ZERO(&analysis_fds);
    FD_SET(cctrl->child_fd_set[STEREO_ANALYSIS][PIPE_IN][READ_END],
      &analysis_fds);
    FD_SET(st_analysis_terminate_fd[0], &analysis_fds);

    analysis_nfds =
      MAX(cctrl->child_fd_set[STEREO_ANALYSIS][PIPE_IN][READ_END],
      st_analysis_terminate_fd[0]);

    CDBG("%s: loop started\n", __func__);
    rc = select(analysis_nfds + 1, &analysis_fds, NULL, NULL,
                &analysis_timeout);
    if (rc == 0) {
      CDBG_HIGH("...stereocam_analysis select timeout...\n");
      continue;
    } else if (rc < 0) {
      CDBG_ERROR("%s: SELECT ERROR %s \n", __func__, strerror(errno));
      if (stereocam_analysis_exit != 0) break;
      usleep(1000 * 100);
      continue;
    } else if (rc) {
      if (stereocam_analysis_exit != 0) break;
      if (FD_ISSET(cctrl->child_fd_set[STEREO_ANALYSIS][PIPE_IN][READ_END],
        &analysis_fds)) {
        rc = read(cctrl->child_fd_set[STEREO_ANALYSIS][PIPE_IN][READ_END],
          &camMsg, sizeof(camMsg));
        if (rc < 0)
          CDBG_HIGH("%s: Cannot read from config thread\n", __func__);

        if (camMsg.data == NULL) {
          CDBG_ERROR("%s: Error...Msg Data pointer is NULL\n", __func__);
          break;
        } else {
          CDBG("%s: Msg Data pointer is Good.\n", __func__);
          input_frame = *(struct msm_st_frame *)camMsg.data;
        }

        if (input_frame.type != OUTPUT_TYPE_ST_D)
          CDBG_HIGH("%s: Invalid Frame type = %d!!!\n", __func__,
               input_frame.type);

        CDBG("%s: cropinfo in_w = %d, in_h = %d\n", __func__,
          input_frame.L.stCropInfo.in_w, input_frame.L.stCropInfo.in_h);
        CDBG("%s: cropinfo out_w = %d, out_h = %d\n", __func__,
          input_frame.L.stCropInfo.out_w, input_frame.L.stCropInfo.out_h);

        if ((input_frame.L.stCropInfo.out_w != 0) &&
          (input_frame.L.stCropInfo.out_h != 0)) {
          stCtrl->lib3d.s3d_active_window.x = (*mono_left_w -
            input_frame.L.stCropInfo.in_w) / 2;
          stCtrl->lib3d.s3d_active_window.y = (*mono_left_h -
            input_frame.L.stCropInfo.in_h) / 2;
          stCtrl->lib3d.s3d_active_window.dx = input_frame.L.stCropInfo.in_w;
          stCtrl->lib3d.s3d_active_window.dy = input_frame.L.stCropInfo.in_h;
        } else {
          stCtrl->lib3d.s3d_active_window.x = 0;
          stCtrl->lib3d.s3d_active_window.y = 0;
          stCtrl->lib3d.s3d_active_window.dx = *mono_left_w;
          stCtrl->lib3d.s3d_active_window.dy = *mono_left_h;
        }

        rc = stCtrl->lib3d.s3d_set_active_analysis_window(
          stCtrl->lib3d.s3d_param, &stCtrl->lib3d.s3d_active_window);
        if (rc != S3D_RET_SUCCESS) {
          CDBG_HIGH("%s: s3d_set_active_analysis_window failed with rc = %d\n",
            __func__, rc);
          return FALSE;
        }

        rc = stCtrl->lib3d.s3d_adjust_3d_effect(stCtrl->lib3d.s3d_param,
          stCtrl->lib3d.pop_out_ratio);
        if (rc != S3D_RET_SUCCESS) {
          CDBG_HIGH("%s: s3d_adjust_3d_effect failed with rc = %d\n",
            __func__, rc);
          return FALSE;
        }

        rc = stCtrl->lib3d.s3d_run_convergence(stCtrl->lib3d.s3d_param,
          (uint8_t *)input_frame.buf_info.buffer,
          (*mono_left_w) * mono_w_scale,
          (uint8_t *)(input_frame.buf_info.buffer + (*mono_left_w)),
          (*mono_left_w) * mono_w_scale,
          &stCtrl->lib3d.s3d_state, &right_shift,
          &stCtrl->lib3d.quality_indicator);

        if (rc == S3D_RET_ABORTED) {
          CDBG_HIGH("%s: s3d_run_convergence aborted\n", __func__);
          /* TODO: Chech if reset_history needs to be called. */
        } else if (rc != S3D_RET_SUCCESS) {
          CDBG_HIGH("%s: s3d_run_convergence failed with rc = %d\n",
            __func__, rc);
          return FALSE;
        } else {
          stCtrl->right_image_shift = right_shift;
          stCtrl->left_image_shift = 0;
          CDBG("%s, R_img_shift = %f L_img_shift = %f\n", __func__,
            cctrl->stereoCtrl.right_image_shift,
            cctrl->stereoCtrl.left_image_shift);
        }

        output_frame = input_frame.buf_info;
        rc = write(cctrl->child_fd_set[STEREO_ANALYSIS][PIPE_OUT][WRITE_END],
          &output_frame, sizeof(output_frame));
        if (rc < 0)
          CDBG_HIGH("%s: Config thread wake up failed", __func__);
      }
    }
  } while (!stereocam_analysis_exit);

  deinit_lib3d(cctrl);

  if (st_analysis_terminate_fd[0] >= 0)
    close(st_analysis_terminate_fd[0]);
  if (st_analysis_terminate_fd[1] >= 0)
    close(st_analysis_terminate_fd[1]);

  CDBG("%s: EXIT\n", __func__);
  return NULL;
} /* stereocam_analysis */
#endif
