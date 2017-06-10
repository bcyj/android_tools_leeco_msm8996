/*============================================================================
   Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential                         .
                                                                 .
   This file implements the media/module/master controller logic in the                                                              .
   mm-camera server. The functionalities of the media controller are:

   1. maintain a list of object to represent a possible camera session
   2. construct, active and deactive a camera session
   3. control the data and command flow between kernel and user space
      moduels

============================================================================*/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <dlfcn.h>
#include "camera_dbg.h"
#include "config_proc.h"
#include "mctl_divert.h"
#include "mctl.h"
#include "mctl_pp.h"
#include "mctl_stats.h"
#include "mctl_ez.h"

#if 0
#undef CDBG
#define CDBG LOGE
#endif

/* global structure containes all the camera information*/
static struct msm_camera_info g_camera_info;
static struct msm_cam_config_dev_info g_config_info;
static mctl_comp_lib_t my_shared_stats_proc_lib;
static mctl_comp_lib_t my_shared_frame_proc_lib;

static int mctl_camera_plugin_init_callbacks(mctl_config_ctrl_t *ctrl);
static void mctl_store_curr_target(mctl_config_ctrl_t *ctrl);

static mctl_ctrl_state_enum_t mctl_get_state(
  mctl_config_ctrl_t* p_cfg_ctrl)
{
  CDBG("%s: mctl state = %d", __func__, p_cfg_ctrl->mctl_state);
  return p_cfg_ctrl->mctl_state;
}

static void mctl_set_state(mctl_config_ctrl_t* p_cfg_ctrl,
  mctl_ctrl_state_enum_t state)
{
  CDBG("%s: mctl state change from %d to %d", __func__, p_cfg_ctrl->mctl_state,
    state);
  p_cfg_ctrl->mctl_state = state;
}

static int mctl_query_camsensor_info(mctl_config_ctrl_t* p_cfg_ctrl,
  struct msm_camsensor_info *psinfo)
{
  int rc = 0;

  if (p_cfg_ctrl->camfd <= 0)
    CDBG_ERROR("%s: invalid fd = %d",  __func__, p_cfg_ctrl->camfd);

  if ((rc=ioctl(p_cfg_ctrl->camfd, MSM_CAM_IOCTL_GET_SENSOR_INFO, psinfo)) < 0)
    CDBG_ERROR("%s: MSM_CAM_IOCTL_GET_SENSOR_INFO(%d) failed %d!\n", __func__,
      p_cfg_ctrl->camfd, rc);

  return rc;
}

static int mctl_rdi_cam_pause_and_resume(mctl_config_ctrl_t* p_cfg_ctrl,
                                         int high_clk)
{
  int rc = 0;
  uint8_t use_diff_clock = 0;
  /* if this mctl is PIX camera and RDI camera is alraedy running
   * we need to check if the RDI clock needs
   * to be bump up to PIX clock */
  if (p_cfg_ctrl->video_ctrl.use_pix_interface == 0)
    return rc;
  rc = p_cfg_ctrl->plugin_client_ops.diff_clk_for_rdi_camera(
           p_cfg_ctrl->camera_plugin_ops->handle,
           p_cfg_ctrl->plugin_client_ops.client_handle,
           &use_diff_clock);
  if (rc < 0) {
    CDBG_ERROR("%s: error, rc = %d, bump up clk = %d",
                 __func__,  rc, high_clk);
    return rc;
  }
  if (use_diff_clock == 0) {
    /* no need to change RDI clock just return */
    return rc;
  }
  /* lock ISPIF, RDI streaming pause and resume with VFE PIX clk */
  return rc;
}
/* destroy interface in a media controller object */
static void mctl_interface_destroy(mctl_config_ctrl_t* p_cfg_ctrl)
{
  module_ops_t *mod_ops = NULL;
  int high_clk = 0;

  if (!p_cfg_ctrl) {
    CDBG_ERROR("%s: invalid input", __func__);
    return;
  }
  CDBG("%s: Enter", __func__);
  /* if this mctl is PIX camera and RDI camera is already running
   * we need to check if the VFE clock needs
   * to be reduced to RDI's necessary clock */
  mctl_rdi_cam_pause_and_resume(p_cfg_ctrl, high_clk);

  mod_ops = &p_cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF];
  if ((uint32_t)NULL != mod_ops->handle) {
    if (mod_ops->destroy) {
      mod_ops->destroy(mod_ops->handle);
      mod_ops->destroy = NULL;
    }
    mod_ops->handle = 0;
  }

  mod_ops = &p_cfg_ctrl->comp_ops[MCTL_COMPID_SENSOR];
  if ((uint32_t)NULL != mod_ops->handle) {
    if (mod_ops->destroy) {
      mod_ops->destroy(mod_ops->handle);
      mod_ops->destroy = NULL;
    }
    mod_ops->handle = 0;
  }

  mod_ops = &p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE];
  if ((uint32_t)NULL != mod_ops->handle) {
    if (mod_ops->destroy) {
      mod_ops->destroy(mod_ops->handle);
      mod_ops->destroy = NULL;
    }
    mod_ops->handle = 0;
  }

  mod_ops = &p_cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF];
  if ((uint32_t)NULL != mod_ops->handle) {
    if (mod_ops->destroy) {
      mod_ops->destroy(mod_ops->handle);
      mod_ops->destroy = NULL;
    }
    mod_ops->handle = 0;
  }

  mod_ops = &p_cfg_ctrl->comp_ops[MCTL_COMPID_AXI];
  if ((uint32_t)NULL != mod_ops->handle) {
    if (mod_ops->destroy) {
      mod_ops->destroy(mod_ops->handle);
      mod_ops->destroy = NULL;
    }
    mod_ops->handle = 0;
  }

  mod_ops = &p_cfg_ctrl->comp_ops[MCTL_COMPID_ACTUATOR];
  if ((uint32_t)NULL != mod_ops->handle) {
    if (mod_ops->destroy) {
      mod_ops->destroy(mod_ops->handle);
      mod_ops->destroy = NULL;
    }
    mod_ops->handle = 0;
  }

  mod_ops = &p_cfg_ctrl->comp_ops[MCTL_COMPID_EEPROM];
  if ((uint32_t)NULL != mod_ops->handle) {
    if (mod_ops->destroy) {
      mod_ops->destroy(mod_ops->handle);
      mod_ops->destroy = NULL;
    }
    mod_ops->handle = 0;
  }

  mod_ops = &p_cfg_ctrl->comp_ops[MCTL_COMPID_FRAMEPROC];
  if ((uint32_t)NULL != mod_ops->handle){
    if (mod_ops->destroy) {
      mod_ops->destroy(mod_ops->handle);
      mod_ops->destroy = NULL;
    }
    mod_ops->handle = 0;
  }

  mod_ops = &p_cfg_ctrl->comp_ops[MCTL_COMPID_FLASHLED];
  if ((uint32_t)NULL != mod_ops->handle) {
    if (mod_ops->destroy) {
      mod_ops->destroy(mod_ops->handle);
      mod_ops->destroy = NULL;
    }
    mod_ops->handle = 0;
  }

  mod_ops = &p_cfg_ctrl->comp_ops[MCTL_COMPID_FLASHSTROBE];
  if ((uint32_t)NULL != mod_ops->handle) {
    if (mod_ops->destroy) {
      mod_ops->destroy(mod_ops->handle);
      mod_ops->destroy = NULL;
    }
    mod_ops->handle = 0;
  }

  if (p_cfg_ctrl->chromatix_ptr) {
    mod_ops = &p_cfg_ctrl->comp_ops[MCTL_COMPID_STATSPROC];
    if ((uint32_t)NULL != mod_ops->handle) {
      if (mod_ops->destroy) {
        mod_ops->destroy(mod_ops->handle);
        mod_ops->destroy = NULL;
      }
      mod_ops->handle = 0;
    }
  }

  mod_ops = &p_cfg_ctrl->comp_ops[MCTL_COMPID_CSI];
  if ((uint32_t)NULL != mod_ops->handle) {
    if (mod_ops->destroy) {
      mod_ops->destroy(mod_ops->handle);
      mod_ops->destroy = NULL;
    }
    mod_ops->handle = 0;
  }

  comp_ext_client_info_t *p_ext = NULL, cl_ext;
  if (p_cfg_ctrl->p_client_ops) {
    cl_ext.fd = p_cfg_ctrl->camfd;
    p_ext = &cl_ext;
  }
  qcamsvr_release_res(p_cfg_ctrl->cfg_arg.vnode_id, MCTL_COMPID_AXI,
    p_ext);
  qcamsvr_release_res(p_cfg_ctrl->cfg_arg.vnode_id, MCTL_COMPID_VFE,
    p_ext);
}

void mctl_init_stats_proc_info(mctl_config_ctrl_t *ctrl,
  stats_proc_interface_input_t *input)
{
  int rc;
  sensor_get_t sensor_get;
  input->chromatix = ctrl->chromatix_ptr;
  sensor_get.data.aec_info.op_mode = SENSOR_MODE_PREVIEW;
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
          ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
          SENSOR_GET_SENSOR_MODE_AEC_INFO,
          &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    return;
  }
  input->sensor_info.preview_linesPerFrame =
    sensor_get.data.aec_info.lines_per_frame;
  input->sensor_info.pixel_clock =
    sensor_get.data.aec_info.pclk;
  input->sensor_info.pixel_clock_per_line =
    sensor_get.data.aec_info.pixels_per_line;
  input->sensor_info.max_preview_fps = input->sensor_info.preview_fps =
    sensor_get.data.aec_info.max_fps;
  input->sensor_info.current_fps = sensor_get.data.aec_info.max_fps;

  sensor_get.data.aec_info.op_mode = SENSOR_MODE_SNAPSHOT;
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
          ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
          SENSOR_GET_SENSOR_MODE_AEC_INFO,
          &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    return;
  }
  input->sensor_info.snap_linesPerFrame =
    sensor_get.data.aec_info.lines_per_frame;
  input->sensor_info.snapshot_fps =
    sensor_get.data.aec_info.max_fps;

  sensor_get.data.aec_info.op_mode = SENSOR_MODE_SNAPSHOT;
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
          ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
          SENSOR_GET_SENSOR_MAX_AEC_INFO,
          &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    return;
  }
  input->sensor_info.snap_max_line_cnt =
    sensor_get.data.aec_info.max_linecount;
  input->sensor_info.max_gain =
    sensor_get.data.aec_info.max_gain;
  CDBG_HIGH("%s: snap_max_line_cnt =%d", __func__, input->sensor_info.snap_max_line_cnt);

  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
    ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
    SENSOR_GET_LENS_INFO, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    return;
  }
  input->actr_info.focal_length = sensor_get.data.lens_info.focal_length;
  input->actr_info.af_f_pix = sensor_get.data.lens_info.pix_size;
  input->actr_info.af_f_num = sensor_get.data.lens_info.f_number;
  input->actr_info.af_total_f_dist = sensor_get.data.lens_info.total_f_dist;
  input->actr_info.hor_view_angle= sensor_get.data.lens_info.hor_view_angle;
  input->actr_info.ver_view_angle= sensor_get.data.lens_info.ver_view_angle;

  input->mctl_info.numRegions = MCTL_AEC_NUM_REGIONS;
  input->isp_info.digital_gain_adj = MCTL_DEFAULT_DIG_GAIN_ADJ;
  ctrl->stats_proc_ctrl.digital_gain = MCTL_DEFAULT_DIG_GAIN;
  input->mctl_info.vfe_stats_out =
    &ctrl->stats_proc_ctrl.vfe_stats_out;
  ctrl->stats_proc_ctrl.vfe_stats_out.numRegions =
    MCTL_AEC_NUM_REGIONS;
}
void mctl_init_frame_proc_info(mctl_config_ctrl_t *ctrl,
                      frame_proc_interface_input_t *input)
{
  CDBG("%s E ", __func__);
  input->chromatix = ctrl->chromatix_ptr;
  /* Reset the post processing variable in kernel */
  mctl_divert_set_key(ctrl, FP_RESET);
}

static int mctl_load_frame_proc_lib(void)
{
  memset(&my_shared_frame_proc_lib, 0, sizeof(my_shared_frame_proc_lib));
  my_shared_frame_proc_lib.ptr = dlopen("libmmcamera_frameproc.so", RTLD_NOW);

  if (!my_shared_frame_proc_lib.ptr) {
     CDBG_ERROR("%s Error opening frame_proc library: %s", __func__, dlerror());
    return -EPERM;
  }

  *(void **)&(my_shared_frame_proc_lib.comp_create) =
    dlsym(my_shared_frame_proc_lib.ptr, "FRAME_PROC_comp_create");

  *(void **)&(my_shared_frame_proc_lib.comp_destroy) =
    dlsym(my_shared_frame_proc_lib.ptr, "FRAME_PROC_comp_destroy");

  *(void **)&(my_shared_frame_proc_lib.client_open) =
    dlsym(my_shared_frame_proc_lib.ptr, "FRAME_PROC_client_open");

  return ERROR_NONE;
}

static int mctl_load_stats_proc_lib(void)
{
  memset(&my_shared_stats_proc_lib, 0, sizeof(my_shared_stats_proc_lib));
  my_shared_stats_proc_lib.ptr = dlopen("libmmcamera_statsproc31.so", RTLD_NOW);
  if (!my_shared_stats_proc_lib.ptr) {
    CDBG_ERROR("%s Error opening stats_proc library: %s", __func__, dlerror());
    return -EPERM;
  }

  *(void **)&(my_shared_stats_proc_lib.comp_create) =
    dlsym(my_shared_stats_proc_lib.ptr, "STATSPROC_comp_create");

  *(void **)&(my_shared_stats_proc_lib.comp_destroy) =
    dlsym(my_shared_stats_proc_lib.ptr, "STATSPROC_comp_destroy");

  *(void **)&(my_shared_stats_proc_lib.client_open) =
    dlsym(my_shared_stats_proc_lib.ptr, "STATSPROC_client_open");

  CDBG("%s: errno %d %p %p %p", __func__, errno,
    my_shared_stats_proc_lib.comp_create,
    my_shared_stats_proc_lib.comp_destroy,
    my_shared_stats_proc_lib.client_open);
  return ERROR_NONE;
}

/* initialize a media contoller itself - opposite to mctl_release() */
static int mctl_init_comps(mctl_config_ctrl_t* p_cfg_ctrl, uint32_t comp_mask)
{
  int rc = 0;
  sensor_get_t sensor_get ;

  CDBG("%s E Mask value = %d ", __func__, comp_mask);
  if(comp_mask & (1 << MCTL_COMPID_SENSOR)) {
    p_cfg_ctrl->sensor_op_mode = SENSOR_MODE_INVALID;

    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
      p_cfg_ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
      SENSOR_GET_CHROMATIX_PTR, &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
        CDBG_ERROR("%s: sensor_get_params failed: rc = %d!\n", __func__, rc);
        goto error;
    }
    p_cfg_ctrl->chromatix_ptr = sensor_get.data.chromatix_ptr;

    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
      p_cfg_ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
      SENSOR_GET_LENS_INFO, &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
      CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
      goto error;
    }
    p_cfg_ctrl->sensorCtrl.lens_info.focal_length =
      sensor_get.data.lens_info.focal_length;
    p_cfg_ctrl->sensorCtrl.lens_info.pix_size =
      sensor_get.data.lens_info.pix_size;
    p_cfg_ctrl->sensorCtrl.lens_info.f_number =
      sensor_get.data.lens_info.f_number;
    p_cfg_ctrl->sensorCtrl.lens_info.total_f_dist =
      sensor_get.data.lens_info.total_f_dist;
    p_cfg_ctrl->sensorCtrl.lens_info.hor_view_angle=
      sensor_get.data.lens_info.hor_view_angle;
    p_cfg_ctrl->sensorCtrl.lens_info.ver_view_angle=
      sensor_get.data.lens_info.ver_view_angle;
  }

  /*Need to turn on VFE clocks (done as part of AXI init)
    before ISPIF can be reset*/
  if(comp_mask & (1 << MCTL_COMPID_AXI)) {
      rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_AXI].init(
                     p_cfg_ctrl->comp_ops[MCTL_COMPID_AXI].handle,
                     &p_cfg_ctrl->ops, (void *)&p_cfg_ctrl->concurrent_enabled);
    if (rc < 0) {
      CDBG_ERROR("%s: axi_interface_init failed: handle = 0x%x, rc = %d!\n",
                 __func__, p_cfg_ctrl->comp_ops[MCTL_COMPID_AXI].handle, rc);
      goto error;
    }
  }

  if(comp_mask & (1 << MCTL_COMPID_CSI)) {
    /* setup CSI */
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_CSI].init(
      p_cfg_ctrl->comp_ops[MCTL_COMPID_CSI].handle,
      &p_cfg_ctrl->ops, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: csi_interface_init failed: handle = 0x%x, rc = %d!\n",
        __func__, p_cfg_ctrl->comp_ops[MCTL_COMPID_CSI].handle,
        rc);
      goto error;
    }

    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
      p_cfg_ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
      SENSOR_GET_CSI_PARAMS, &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
      CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
      goto error;
    }

    csi_set_t csi_set;
    csi_set.data.csi_params = sensor_get.data.sensor_csi_params;
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_CSI].set_params(
      p_cfg_ctrl->comp_ops[MCTL_COMPID_CSI].handle,
      CSI_SET_DATA, &csi_set, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: csi_set_params failed %d\n", __func__, rc);
      goto error;
    }

    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_CSI].process(
      p_cfg_ctrl->comp_ops[MCTL_COMPID_CSI].handle,
      CSI_PROCESS_INIT, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: csi_process_init failed %d\n", __func__, rc);
      goto error;
    }
  }

  if (comp_mask & (1 << MCTL_COMPID_ISPIF)) {
    /* setup ISPIF */
    int obj_idx = 0;
    ispif_set_t ispif_set;
    ispif_get_t ispif_get;
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].init(
                     p_cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
                     &p_cfg_ctrl->ops, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: ispif_interface_init failed: handle = 0x%x, rc = %d!\n",
                 __func__, p_cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
                 rc);
      goto error;
    }

    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].set_params(
                     p_cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
                     ISPIF_PARM_ADD_OBJ_ID, (void *)&obj_idx, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s ISPIF_PARM_ADD_OBJ_ID failed %d ", __func__, rc);
      goto error;
    }

  }

  if(comp_mask & (1 << MCTL_COMPID_VFE)) {
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].init(
                     p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                     &p_cfg_ctrl->ops, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: vfe_interface_init failed: handle = 0x%x, rc = %d!\n",
                 __func__, p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle, rc);
      goto error;
    } else {
      int obj_idx = p_cfg_ctrl->video_ctrl.default_vfe_idx;
      uint32_t hw_version = 0;
      rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
                       p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_PARM_HW_VERSION, (void *)&hw_version, NULL);
      if (rc < 0) {
        CDBG_ERROR("%s VFE_PARM_HW_VERSION failed %d ", __func__, rc);
        goto error;
      }
      rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
                       p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_PARM_ADD_OBJ_ID, (void *)&obj_idx, NULL);
      if (rc < 0) {
        CDBG_ERROR("%s VFE_PARM_ADD_OBJ_ID failed %d ", __func__, rc);
        goto error;
      }

      if (p_cfg_ctrl->chromatix_ptr) {
        rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
                         p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                         VFE_SET_CHROMATIX_PARM,
                         (void *)p_cfg_ctrl->chromatix_ptr, NULL);
        if (rc != 0) {
          CDBG_ERROR("%s: vfe_set_params failed: rc = %d!\n", __func__, rc);
          goto error;
        }
      }
      /* Hard code stats version
         0 ==> Legacy stats
         4 ==> Bayer stats
      */
#ifdef VFE_40
      p_cfg_ctrl->vfe_ver.main = 4;
#else
      p_cfg_ctrl->vfe_ver.main = 0;
#endif
      rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
                       p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_SET_STATS_VERSION,
                       (void *)&p_cfg_ctrl->vfe_ver, NULL);
      if (rc != 0) {
        CDBG_ERROR("%s: VFE_SET_STATS_VERSION failed: rc = %d!\n", __func__, rc);
        goto error;
      }
      rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_MODULE_INIT,
                       &p_cfg_ctrl->sensorCtrl.sensor_output.output_format);
      if (rc < 0) {
        CDBG_ERROR("%s VFE_MODULE_INIT failed %d ", __func__, rc);
        goto error;
      }
    }
  }

  CDBG("%s Checking CAMIF needed? %d ", __func__, comp_mask);
  if(comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].init(
                     p_cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
                     &p_cfg_ctrl->ops, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: camif_interface_init failed: handle = 0x%x, rc = %d!\n",
                 __func__, p_cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].handle, rc);
      goto error;
    } else {
      int type;
      camif_input_t camif_set;
      type = CAMIF_PARAMS_HW_VERSION;
      camif_set.d.vfe_version = 0; /* hard coded for now. need to pass */
      rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].set_params(
                       p_cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].handle, type,
                       (void *)&camif_set, NULL);
      if (rc < 0) {
        CDBG_ERROR("%s CAMIF_PARAMS_HW_VERSION failed %d ", __func__, rc);
        goto error;
      }
      camif_set.obj_idx = 0;
      rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].set_params(
                       p_cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
                       CAMIF_PARAMS_ADD_OBJ_ID, (void *)&camif_set, NULL);
      if (rc < 0) {
        CDBG_ERROR("%s CAMIF_PARAMS_ADD_OBJ_ID failed %d ", __func__, rc);
        goto error;
      }
      rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
        p_cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].handle, CAMIF_MODULE_INIT, NULL);
      if (rc < 0) {
        CDBG_ERROR("%s CAMIF CAMIF_MODULE_INIT failed %d ", __func__, rc);
        goto error;
      }
    }
  }

  if(comp_mask & (1 << MCTL_COMPID_AXI)) {
    axi_set_t axi_set;
    axi_set.type = AXI_PARM_HW_VERSION;
    axi_set.data.vfe_version = 0; /* hard coded for now. need to pass */
    axi_set.data.current_target = p_cfg_ctrl->current_target;
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_AXI].set_params(
                     p_cfg_ctrl->comp_ops[MCTL_COMPID_AXI].handle,
                     axi_set.type, (void *)&axi_set, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s AXI_PARM_HW_VERSION failed %d ", __func__, rc);
      goto error;
    }
    axi_set.type = AXI_PARM_STATS_VERSION;
#ifdef VFE_40
    axi_set.data.stats_version = 4; /* bayer stats = 4 */
#else
    axi_set.data.stats_version = 0;
#endif
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_AXI].set_params(
                     p_cfg_ctrl->comp_ops[MCTL_COMPID_AXI].handle,
                     axi_set.type, (void *)&axi_set, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s AXI_PARM_STATS_VERSION failed %d ", __func__, rc);
      goto error;
    }
    axi_set.type = AXI_PARM_ADD_OBJ_ID;
    axi_set.data.axi_obj_idx = 0; /* hard coded for now. need to pass */
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_AXI].set_params(
                   p_cfg_ctrl->comp_ops[MCTL_COMPID_AXI].handle,
                     axi_set.type, (void *)&axi_set, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s AXI_PARM_ADD_OBJ_ID failed %d ", __func__, rc);
      goto error;
    }
  }

  if(comp_mask & (1 << MCTL_COMPID_ACTUATOR)){
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_ACTUATOR].init(
                     p_cfg_ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle,
                     &p_cfg_ctrl->ops, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: Actuator Intf init failed: handle = 0x%x, rc = %d!\n",
                 __func__, p_cfg_ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle,
                 rc);
      goto error;
    }
  }

  if(comp_mask & (1 << MCTL_COMPID_FRAMEPROC)) {
    int obj_idx = 0;
    mctl_init_frame_proc_info(p_cfg_ctrl,
         &(p_cfg_ctrl->frame_proc_ctrl.intf.input));
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].init(
                     p_cfg_ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle,
                     &p_cfg_ctrl->ops,
                     &(p_cfg_ctrl->frame_proc_ctrl.intf.input));
    if (rc < 0) {
      CDBG_ERROR("%s Error initializing frame proc interface", __func__);
      goto error;
    }
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].set_params(
                     p_cfg_ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle,
                     FRAME_PROC_ADD_OBJ,
                     &obj_idx, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s Error initializing frame proc interface2", __func__);
      goto error;
    }
  }

  if(comp_mask & (1 << MCTL_COMPID_FLASHLED)) {
    flash_init_data_t led_init_data;

    led_init_data.fd = p_cfg_ctrl->camfd;
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_FLASHLED].init(
                     p_cfg_ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
                     &p_cfg_ctrl->ops, &led_init_data);
    if (rc < 0) {
      CDBG("%s: flash_led_interface_init failed: rc = %d!\n", __func__, rc);
      goto error;
    }
  }

  if(comp_mask & (1 << MCTL_COMPID_FLASHSTROBE)) {
    flash_init_data_t strobe_init_data;
    strobe_init_data.fd = p_cfg_ctrl->camfd;
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_FLASHSTROBE].init(
                     p_cfg_ctrl->comp_ops[MCTL_COMPID_FLASHSTROBE].handle,
                     &p_cfg_ctrl->ops, &strobe_init_data);
    if (rc < 0) {
      CDBG("%s: flash_strobe_interface_init failed: rc = %d!\n", __func__, rc);
      goto error;
    }
  }

  if(comp_mask & (1 << MCTL_COMPID_STATSPROC)) {
    if (p_cfg_ctrl->chromatix_ptr) {
      mctl_init_stats_proc_info(p_cfg_ctrl,
           &(p_cfg_ctrl->stats_proc_ctrl.intf.input));

      if (p_cfg_ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle) {
        /* Initialize afCtrl */
        actuator_get_t actuator_info;
        rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_ACTUATOR].get_params(
                         p_cfg_ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle,
                         ACTUATOR_GET_INFO, (void *)&actuator_info,
                         sizeof(actuator_info));
        if (rc < 0) {
          CDBG_ERROR("%s: actuator_get failed %d\n", __func__, rc);
          goto error;
        }
        p_cfg_ctrl->afCtrl.af_enable = actuator_info.data.af_support;
        p_cfg_ctrl->afCtrl.parm_focusrect.minimum_value = AUTO;
        p_cfg_ctrl->afCtrl.parm_focusrect.maximum_value = AVERAGE;
        p_cfg_ctrl->afCtrl.parm_focusrect.default_value = AUTO;
        p_cfg_ctrl->afCtrl.parm_focusrect.current_value = AUTO;
        p_cfg_ctrl->afCtrl.parm_focusrect.step_value    = 1;

        if (p_cfg_ctrl->afCtrl.af_enable) {
          actuator_set_t actuator_set_info;
          rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_ACTUATOR].set_params(
            p_cfg_ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle,
            ACTUATOR_DEF_FOCUS, NULL, NULL);
          CDBG("set DEF focus \n");
          if (rc < 0) {
            CDBG_ERROR("%s: actuator_set failed %d\n", __func__, rc);
            goto error;
          }
          rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_ACTUATOR].get_params(
            p_cfg_ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle,
            ACTUATOR_GET_AF_TUNE_PTR,
            (void *)&actuator_info, sizeof(actuator_info));
          if (rc < 0) {
            CDBG_ERROR("%s: actuator_get failed %d\n", __func__, rc);
            goto error;
          }
          p_cfg_ctrl->af_tune_ptr = actuator_info.data.af_tune_ptr;
        }
      }
      /* associate mctl's ops table with stats_proc */
      p_cfg_ctrl->stats_proc_ctrl.intf.input.af_tune_ptr =
        p_cfg_ctrl->af_tune_ptr;
      p_cfg_ctrl->stats_proc_ctrl.intf.input.ops = p_cfg_ctrl->ops;
      CDBG("%s: stats_proc init", __func__);
      rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_STATSPROC].init(
                       p_cfg_ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
                       &p_cfg_ctrl->ops,
                       &(p_cfg_ctrl->stats_proc_ctrl.intf.input));
      if (rc < 0) {
        CDBG_ERROR("%s Error initializing stats proc interface", __func__);
        goto error;
      }
    }
  }

  if (comp_mask & (1 << MCTL_COMPID_EEPROM)) {
    eeprom_get_t eeprom_get;
    eeprom_set_t eeprom_set;
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_EEPROM].init(
      p_cfg_ctrl->comp_ops[MCTL_COMPID_EEPROM].handle,
      &p_cfg_ctrl->ops, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s Error initializing eeprom interface", __func__);
      goto error;
    }

    eeprom_get.type = EEPROM_GET_SUPPORT;
    rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_EEPROM].get_params(
      p_cfg_ctrl->comp_ops[MCTL_COMPID_EEPROM].handle,
      EEPROM_GET_SUPPORT, &eeprom_get, sizeof(eeprom_get_t));
      if (rc < 0) {
        CDBG_ERROR("%s: eeprom_get_params failed: rc = %d!\n", __func__, rc);
        goto error;
      }

    p_cfg_ctrl->is_eeprom = 0;
    if(eeprom_get.data.is_eeprom_supported) {
      p_cfg_ctrl->is_eeprom = 1;
      if (p_cfg_ctrl->chromatix_ptr) {
        eeprom_set.data.info.chromatixPtr =
        p_cfg_ctrl->chromatix_ptr;
        rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_EEPROM].set_params(
          p_cfg_ctrl->comp_ops[MCTL_COMPID_EEPROM].handle, EEPROM_SET_CHROMATIX,
          &eeprom_set, NULL);
        if (rc != 0) {
          CDBG_ERROR("%s: eeprom_set_params failed: rc = %d!\n", __func__, rc);
          goto error;
        }
      }

      if (p_cfg_ctrl->af_tune_ptr) {
        eeprom_set.data.info.aftune_ptr =
        p_cfg_ctrl->af_tune_ptr;
        rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_EEPROM].set_params(
          p_cfg_ctrl->comp_ops[MCTL_COMPID_EEPROM].handle, EEPROM_SET_FOCUSPTR,
          &eeprom_set, NULL);
        if (rc != 0) {
          CDBG_ERROR("%s: eeprom_set_params failed: rc = %d!\n", __func__, rc);
          goto error;
        }

        rc = p_cfg_ctrl->comp_ops[MCTL_COMPID_EEPROM].set_params(
          p_cfg_ctrl->comp_ops[MCTL_COMPID_EEPROM].handle, EEPROM_SET_DOCALIB,
          &eeprom_set, NULL);
        if (rc != 0) {
          CDBG_ERROR("%s: eeprom_set_params failed: rc = %d!\n", __func__, rc);
          goto error;
        }
      }
    }
  }
  mctl_set_state(p_cfg_ctrl, MCTL_CTRL_STATE_SERVICE_READY);
  return rc;
error:
  mctl_set_state(p_cfg_ctrl, MCTL_CTRL_STATE_COMP_INIT_ERR);
  mctl_interface_destroy (p_cfg_ctrl);
  rc = -1;
  CDBG_ERROR("%s Error initializing mctl... ", __func__);
  return rc;
}

static int mctl_open_comps(mctl_config_ctrl_t* p_cfg_ctrl, uint32_t *comp_mask)
{
  uint32_t tmp_handle = 0;
  int i, rc;
  comp_res_req_info_t res_req_info;
  char sdev_name[32];
  comp_ext_client_info_t *p_ext = NULL, cl_ext;
  if (p_cfg_ctrl->p_client_ops) {
    cl_ext.fd = p_cfg_ctrl->camfd;
    p_ext = &cl_ext;
  }

  p_cfg_ctrl->comp_mask = 0;

  /* Always reserve AXI resource. */
  res_req_info.comp_id = MCTL_COMPID_AXI;
  rc = qcamsvr_reserve_res(p_cfg_ctrl->cfg_arg.vnode_id, &res_req_info, p_ext);
  if (rc == -ENXIO) {
    /*Return value of -ENXIO means the resoucre doesnt have a subdev.
    For older targets with no AXI subdev, please dont stop camera here.
    Go ahead with the normal flow.*/
    res_req_info.sdev_revision = -1;
  } else if (rc < 0) {
    CDBG_ERROR("%s: reserve resource failed for comp id %d ", __func__,
      res_req_info.comp_id);
    goto ERROR;
  }
  if ((1 << MCTL_COMPID_AXI) & (*comp_mask)) {
    tmp_handle = AXI_client_open(&p_cfg_ctrl->comp_ops[MCTL_COMPID_AXI],
                                 res_req_info.sdev_revision);
    if(!tmp_handle || tmp_handle != p_cfg_ctrl->comp_ops[MCTL_COMPID_AXI].handle) {
      CDBG_ERROR("%s Error creating axi interface ", __func__);
      goto ERROR;
    }
  }

  /* Always reserve VFE resource */
  res_req_info.comp_id = MCTL_COMPID_VFE;
  rc = qcamsvr_reserve_res(p_cfg_ctrl->cfg_arg.vnode_id, &res_req_info, p_ext);
  if (rc < 0) {
    CDBG_ERROR("%s: reserve resoucre failed for comp id %d ", __func__,
      res_req_info.comp_id);
    goto ERROR;
  }
  if ((1 << MCTL_COMPID_VFE) & (*comp_mask)) {
    tmp_handle = VFE_client_open(&p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE],
                                  res_req_info.sdev_revision);

    if(!tmp_handle || tmp_handle != p_cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle) {
      CDBG_ERROR("%s Error creating vfe interface ", __func__);
      goto ERROR;
    }
  }

  for(i = 0; i < 32; i++) {
    if((1 << i) & (*comp_mask)) {
      switch (i) {
      case MCTL_COMPID_CAMIF:
        tmp_handle = CAMIF_client_open(&p_cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF]);
        if(!tmp_handle || tmp_handle != p_cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].handle) {
          CDBG_ERROR("%s Error creating camif interface ", __func__);
          goto ERROR;
        }
        break;
      case MCTL_COMPID_STATSPROC:
        CDBG("%s: %p", __func__, my_shared_stats_proc_lib.client_open);
        tmp_handle = 0;
        if (my_shared_stats_proc_lib.client_open) {
        tmp_handle = (my_shared_stats_proc_lib.client_open)(&p_cfg_ctrl->comp_ops[MCTL_COMPID_STATSPROC]);
        }
        if(!tmp_handle || tmp_handle != p_cfg_ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle) {
          CDBG_ERROR("%s Error creating statsproc interface %d", __func__, tmp_handle);
          goto ERROR;
        }
        break;
      case MCTL_COMPID_FRAMEPROC:
        /* Load and init post processing library */
        tmp_handle = (my_shared_frame_proc_lib.client_open)
          (&p_cfg_ctrl->comp_ops[MCTL_COMPID_FRAMEPROC]);
        if(!tmp_handle ||
          tmp_handle != p_cfg_ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle) {
          CDBG_ERROR("%s Error creating frameproc interface ", __func__);
          goto ERROR;
        }
        break;
      case MCTL_COMPID_SENSOR:
        /* sensor opened in mctl_init */
        break;
      case MCTL_COMPID_FLASHLED:
        tmp_handle = flash_led_client_open(&p_cfg_ctrl->comp_ops[MCTL_COMPID_FLASHLED]);
        if(!tmp_handle || tmp_handle != p_cfg_ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle) {
        CDBG_ERROR("%s Error creating sensor interface ", __func__);
        goto ERROR;
        }
        break;
      case MCTL_COMPID_FLASHSTROBE:
        tmp_handle = flash_strobe_client_open(&p_cfg_ctrl->comp_ops[MCTL_COMPID_FLASHSTROBE]);
        if(!tmp_handle || tmp_handle != p_cfg_ctrl->comp_ops[MCTL_COMPID_FLASHSTROBE].handle) {
        CDBG_ERROR("%s Error creating sensor interface ", __func__);
        goto ERROR;
        }
        break;
      case MCTL_COMPID_ACTUATOR:
        tmp_handle = ACTUATOR_client_open(
                       &p_cfg_ctrl->comp_ops[MCTL_COMPID_ACTUATOR]);
        if(!tmp_handle || tmp_handle != p_cfg_ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle) {
          CDBG_ERROR("%s Error creating actuator interface ", __func__);
          goto ERROR;
        }
        break;
      case MCTL_COMPID_EEPROM:
        tmp_handle = eeprom_client_open(&p_cfg_ctrl->comp_ops[MCTL_COMPID_EEPROM]);
        if (!tmp_handle || tmp_handle !=
          p_cfg_ctrl->comp_ops[MCTL_COMPID_EEPROM].handle) {
          CDBG_ERROR("%s Error creating eeprom interface ", __func__);
          goto ERROR;
        }
        break;
      case MCTL_COMPID_ISPIF:
        tmp_handle = ISPIF_client_open(
                       &p_cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF]);
        if (!tmp_handle || tmp_handle !=
          p_cfg_ctrl->comp_ops[MCTL_COMPID_ISPIF].handle) {
          CDBG_ERROR("%s Error creating ispif interface ", __func__);
          goto ERROR;
        }
        break;
      case MCTL_COMPID_CSI:
        tmp_handle = CSI_client_open(
                       &p_cfg_ctrl->comp_ops[MCTL_COMPID_CSI]);
        if (!tmp_handle || tmp_handle !=
          p_cfg_ctrl->comp_ops[MCTL_COMPID_CSI].handle) {
          CDBG_ERROR("%s Error creating CSI interface ", __func__);
          goto ERROR;
        }
        break;
      default:
        break;
      }

      p_cfg_ctrl->comp_mask |= (1 <<i);
    }
  }
  return 0;
ERROR:
  mctl_interface_destroy(p_cfg_ctrl);
  return -1;
}

static int mctl_deinit_camera_plugin(m_ctrl_t* ctrl)
{
  int rc = 0;

  if(!ctrl) {
    CDBG_ERROR("%s: null MCTL handle passed in",  __func__);
    return -1;
  }
  ctrl->p_cfg_ctrl->plugin_client_ops.client_destroy (
    ctrl->p_cfg_ctrl->camera_plugin_ops->handle,
    ctrl->p_cfg_ctrl->plugin_client_ops.client_handle);
  ctrl->p_cfg_ctrl->camera_plugin_ops = NULL;
  memset(&ctrl->p_cfg_ctrl->plugin_client_ops, 0,
    sizeof(ctrl->p_cfg_ctrl->plugin_client_ops));
  CDBG("%s: done", __func__);
  return rc;
}

int mctl_deinit(m_ctrl_t* pme)
{
  module_ops_t *s_comp_ops = NULL;
  int pipeline_idx;

  if (!pme || !pme->p_cfg_ctrl) {
    CDBG_ERROR("%s: Invalid Input", __func__);
    return -EINVAL;
  }
  pipeline_idx = pme->p_cfg_ctrl->video_ctrl.def_pp_idx;
  mctl_pp_release(&pme->p_cfg_ctrl->mctl_pp_ctrl[pipeline_idx]);
  mctl_pp_put_free_pipeline(pme->p_cfg_ctrl, pipeline_idx);

  s_comp_ops = &pme->p_cfg_ctrl->comp_ops[MCTL_COMPID_SENSOR];
  if (s_comp_ops->destroy) {
    if (s_comp_ops->destroy(s_comp_ops->handle) < 0)
      CDBG_ERROR("%s: s_comp_ops->destroy failed: handle = 0x%x\n", __func__,
        s_comp_ops->handle);
    else
      s_comp_ops->destroy = NULL;
  }

  if (pme->p_cfg_ctrl->camfd > 0) {
    close(pme->p_cfg_ctrl->camfd);
    pme->p_cfg_ctrl->camfd = 0;
  }
  /* deinit zoom */
  zoom_ctrl_deinit(&pme->p_cfg_ctrl->zoomCtrl);
  mctl_deinit_camera_plugin(pme);
  return 0;
}

int mctl_init(m_ctrl_t* pme)
{
  int rc = 0;
  char config_device[MAX_DEV_NAME_LEN];
  uint32_t tmp_handle = 0;
  sensor_get_t sensor_get;
  module_ops_t *s_comp_ops = NULL;
  mctl_config_ctrl_t *p_cfg_ctrl = NULL;

  if (!pme || !pme->p_cfg_ctrl) {
    CDBG_ERROR("%s: Invalid Input", __func__);
    return -EINVAL;
  }

  p_cfg_ctrl = pme->p_cfg_ctrl;

#ifdef _ANDROID_
  snprintf(config_device, MAX_DEV_NAME_LEN, "/dev/msm_camera/%s",
    p_cfg_ctrl->cfg_arg.config_name);
#else
  snprintf(config_device, MAX_DEV_NAME_LEN, "/dev/%s",
    p_cfg_ctrl->cfg_arg.config_name);
#endif

  CDBG("%s: dev name is %s\n", __func__, config_device);
  p_cfg_ctrl->camfd = open(config_device, O_RDWR);
  if (p_cfg_ctrl->camfd < 0) {
    CDBG_ERROR("%s: open %s failed: %s!\n", __func__,
      pme->cfg_arg.config_name, strerror(errno));
    return -ENODEV;
  }
  mctl_store_curr_target(pme->p_cfg_ctrl);
  /* move the state to initted */
  mctl_set_state(pme->p_cfg_ctrl, MCTL_CTRL_STATE_INITED);
  return rc;
}

int mctl_init_sensor(mctl_config_ctrl_t *p_cfg_ctrl)
{
  int rc = 0;
  int pipeline_idx = 0;
  char config_device[MAX_DEV_NAME_LEN];
  uint32_t tmp_handle = 0;
  sensor_get_t sensor_get;
  module_ops_t *s_comp_ops = NULL;
  uint8_t num_vfes = 0;
  mctl_ctrl_state_enum_t state = mctl_get_state(p_cfg_ctrl);

  if(state != MCTL_CTRL_STATE_INITED) {
    CDBG("%s: sensor already started, nop", __func__);
    return rc;
  }
#ifdef USE_ION
  p_cfg_ctrl->ion_dev_fd = open("/dev/ion", O_RDONLY);
  if (p_cfg_ctrl->ion_dev_fd < 0) {
    CDBG_ERROR("%s: Ion dev open failed. error: %s", __func__, strerror(errno));
    rc = -ENODEV;
    goto ion_open_failed;
  }
#endif

  mctl_stats_init_ops(p_cfg_ctrl);
  s_comp_ops = &p_cfg_ctrl->comp_ops[MCTL_COMPID_SENSOR];
  if(!s_comp_ops->handle) {
    sensor_init_data_t sensor_init_data;
    tmp_handle = sensor_client_open(s_comp_ops);
    if(!tmp_handle || (tmp_handle != s_comp_ops->handle)) {
      CDBG_ERROR("%s Error creating sensor interface ", __func__);
      goto sensor_client_open_failed;
    }

    p_cfg_ctrl->comp_mask = (1 << MCTL_COMPID_SENSOR);
    sensor_init_data.fd = p_cfg_ctrl->camfd;

    if (s_comp_ops->init) {
      rc = s_comp_ops->init(s_comp_ops->handle, &p_cfg_ctrl->ops,
        &sensor_init_data);
      if (rc < 0) {
        CDBG_ERROR("%s: sensor_interface_init failed: handle = 0x%x, rc = %d",
          __func__, s_comp_ops->handle, rc);
        goto sensor_client_destroy;
      }
    }
  }
  rc = s_comp_ops->get_params(s_comp_ops->handle,
    SENSOR_GET_OUTPUT_CFG, &sensor_get, sizeof(sensor_get));
  if (rc < 0)
    CDBG_HIGH("%s: SENSOR_GET_OUTPUT_CFG failed", __func__);

  p_cfg_ctrl->sensorCtrl.sensor_output = sensor_get.data.sensor_output;
  rc = s_comp_ops->get_params(s_comp_ops->handle,
    SENSOR_GET_SENSOR_INFO, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: cannot get camera sensor info, rc = %d", __func__, rc);
    goto sensor_client_destroy;
  }

  rc = get_num_res(p_cfg_ctrl->cfg_arg.vnode_id,
                   MCTL_COMPID_VFE, &num_vfes);
  if (rc < 0) {
    CDBG_ERROR("%s: query VFE resource error, rc = %d",
               __func__,  rc);
    return rc;
  }

  p_cfg_ctrl->cam_sensor_info = sensor_get.sinfo;
  /* here we know if pix interface is needed through plugin */
  rc = p_cfg_ctrl->plugin_client_ops.grant_pix_interface(
         p_cfg_ctrl->camera_plugin_ops->handle,
         p_cfg_ctrl->plugin_client_ops.client_handle,
         &p_cfg_ctrl->video_ctrl.use_pix_interface,
         &p_cfg_ctrl->concurrent_enabled,
         &p_cfg_ctrl->video_ctrl.default_vfe_idx,
         &p_cfg_ctrl->cam_sensor_info, num_vfes);
  if (rc < 0 || !p_cfg_ctrl->camera_plugin_ops->handle) {
    CDBG_ERROR("%s: grant_pix_interface error, rc = %d, "
      "client_handle = 0x%x", __func__, rc,
      p_cfg_ctrl->plugin_client_ops.client_handle);
  }
  p_cfg_ctrl->config_intf = create_config_intf(
    p_cfg_ctrl->cam_sensor_info.ispif_supported);
  if (p_cfg_ctrl->config_intf == NULL) {
    CDBG_ERROR("%s: create_config_intf failed.", __func__);
    goto sensor_client_destroy;
  }

  if (p_cfg_ctrl->cam_sensor_info.actuator_enabled)
    p_cfg_ctrl->afCtrl.af_enable = 1;
  else
    p_cfg_ctrl->afCtrl.af_enable = 0;
  CDBG("%s af_enable = %d\n", __func__, p_cfg_ctrl->afCtrl.af_enable);

  effects_init (&p_cfg_ctrl->effectCtrl);

  snprintf(p_cfg_ctrl->video_ctrl.mctl_pp_dev_name, MAX_DEV_NAME_LEN, "/dev/%s",
    p_cfg_ctrl->mctl_ctrl->cfg_arg.mctl_node_name);

  p_cfg_ctrl->video_ctrl.def_pp_idx = -1;
  rc = mctl_pp_get_free_pipeline(p_cfg_ctrl, &pipeline_idx);
  if ((rc < 0) || (pipeline_idx < 0)) {
    CDBG_ERROR("%s: Cannot get free pipeline rc = %d", __func__, rc);
    goto config_intf_destroy;
  }

  rc = mctl_pp_launch(&p_cfg_ctrl->mctl_pp_ctrl[pipeline_idx], p_cfg_ctrl,
    MCTL_PP_INPUT_FROM_KERNEL);
  if (rc < 0) {
    CDBG_ERROR("%s: mctl_pp_launch failed. rc = %d", __func__, rc);
    /* Mark this pipeline as unused. */
    mctl_pp_put_free_pipeline(p_cfg_ctrl, pipeline_idx);
    goto config_intf_destroy;
  }
  p_cfg_ctrl->video_ctrl.def_pp_idx = pipeline_idx;

  /* init zoom control with default value 4096*4 */
  rc = zoom_init_ctrl(&p_cfg_ctrl->zoomCtrl);
  if(rc < 0) {
    CDBG_ERROR("%s: nzoom_init_ctrl err, rc = %d",  __func__,  rc);
    goto config_intf_destroy;
  }

  /* init bestshot control */
  bestshot_init(&p_cfg_ctrl->bestshotCtrl);

  /*init hdr control*/
  hdr_init(&p_cfg_ctrl->hdrCtrl,
    p_cfg_ctrl->mctl_ctrl->cfg_arg.mctl_node_name);

  /*init liveshot control*/
  liveshot_init(&p_cfg_ctrl->liveshotCtrl,
    p_cfg_ctrl->mctl_ctrl->cfg_arg.mctl_node_name);

  p_cfg_ctrl->ui_sharp_ctrl_factor = CAMERA_DEF_SHARPNESS;
  p_cfg_ctrl->hfr_mode = CAMERA_HFR_MODE_OFF;
  mctl_set_state(p_cfg_ctrl, MCTL_CTRL_STATE_INITED_SENSOR);
  rc = mctl_open_and_init_comps(p_cfg_ctrl);
  if (rc== 0){
    int high_clk = 1;
    CDBG("%s: End, rc = %d", __func__, rc);
    /* if this mctl is PIX camera and RDI camera is alraedy running
     * we need to check if the RDI clock needs
     * to be bump up to PIX clock */
    rc = mctl_rdi_cam_pause_and_resume(p_cfg_ctrl,high_clk);
    return rc;
  }
config_intf_destroy:
  destroy_config_intf(p_cfg_ctrl->config_intf);
  p_cfg_ctrl->config_intf = NULL;
sensor_client_destroy:
  if (s_comp_ops->destroy)
    if (s_comp_ops->destroy(s_comp_ops->handle) < 0)
      CDBG_ERROR("%s: s_comp_ops->destroy failed: handle = 0x%x\n", __func__,
        s_comp_ops->handle);
sensor_client_open_failed:
#ifdef USE_ION
  if (p_cfg_ctrl->ion_dev_fd > 0) {
    close(p_cfg_ctrl->ion_dev_fd);
    p_cfg_ctrl->ion_dev_fd = 0;
  }
ion_open_failed:
#endif
  return rc;
}

/* Following function is a superset of mctl_deinit. It undos the tasks
 * performed after successful mctl thread creation as well as mctl_deinit.
 * It runs in mctl thread context.
 */
int mctl_release(m_ctrl_t* pme)
{
  int rc = 0;

  if (!pme) {
    CDBG_ERROR("%s: pme is NULL", __func__);
    return -1;
  }

  if (pme->p_cfg_ctrl->comp_ops[MCTL_COMPID_SENSOR].handle) {
    pme->p_cfg_ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
      pme->p_cfg_ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
      SENSOR_SET_STOP_STREAM, NULL, NULL);
  }

  mctl_interface_destroy(pme->p_cfg_ctrl);

  if (pme->p_cfg_ctrl->chromatix_ptr)
    mctl_stats_deinit_ops(pme->p_cfg_ctrl);

  if (mctl_deinit(pme) < 0)
    CDBG_HIGH("%s: mctl_deinit failed", __func__);

#ifdef USE_ION
  if (pme->p_cfg_ctrl->ion_dev_fd > 0) {
    close(pme->p_cfg_ctrl->ion_dev_fd);
    pme->p_cfg_ctrl->ion_dev_fd = 0;
  }
#endif

  if (pme->p_cfg_ctrl->config_intf) {
    destroy_config_intf(pme->p_cfg_ctrl->config_intf);
    pme->p_cfg_ctrl->config_intf = NULL;
  }

  return rc;
}

/* this function delete a media controller from memory */
void mctl_delete (m_ctrl_t* pme)
{
#ifdef FEATURE_GYRO
  dsps_set_data_t dsps_set_data;
#endif
  if (pme) {
#ifdef FEATURE_GYRO
    if (pme->p_cfg_ctrl && (NULL == pme->p_cfg_ctrl->p_client_ops)) {
      dsps_set_data.msg_type = DSPS_DISABLE_REQ;
      dsps_set_data.sensor_feature.mode = 0;
      dsps_proc_set_params(&dsps_set_data);
      dsps_proc_deinit();
    }
#endif
    /* for a stop process on this object - ignore the return value*/
    pthread_cond_destroy(&pme->cam_mctl_thread_ready_cond);
    pthread_mutex_destroy(&pme->cam_mctl_thread_ready_mutex);

    /* reclaim the memory*/
    if (pme->p_cfg_ctrl)
      free(pme->p_cfg_ctrl);
    free(pme);
  }
  return;
}

/* This function creates a media controller object with the parm passed in */
m_ctrl_t* mctl_create(struct config_thread_arguments* config_arg)
{
  m_ctrl_t* pme = NULL;
  mctl_config_ctrl_t* p_cfg_ctrl = NULL;

#ifdef FEATURE_GYRO
  dsps_set_data_t dsps_set_data;
#endif
  if (!config_arg) {
    CDBG_ERROR("%s: Invalid args", __func__);
    return NULL;
  }

  /* allocate the memory for this object*/
  p_cfg_ctrl = (mctl_config_ctrl_t*) malloc(sizeof (mctl_config_ctrl_t));
  if (!p_cfg_ctrl) {
    CDBG_ERROR("%s Could not allocate memory for mctl_config_ctrl_t", __func__);
    return NULL;
  }
  memset(p_cfg_ctrl, 0, sizeof(mctl_config_ctrl_t));

  /* allocate the memory and poplulate the object*/
  pme = (m_ctrl_t*) malloc(sizeof(m_ctrl_t));
  if (!pme) {
    CDBG_ERROR("%s Could not allocate memory for m_ctrl_t", __func__);
    free(p_cfg_ctrl);
    p_cfg_ctrl = NULL;
    return NULL;
  }
  p_cfg_ctrl->mctl_ctrl = pme;
  mctl_set_state(p_cfg_ctrl, MCTL_CTRL_STATE_NULL);
  /* make sure no one can use it unless it is inited and started */

  pme->cfg_arg = *config_arg;
  pme->p_cfg_ctrl = p_cfg_ctrl;
  p_cfg_ctrl->cfg_arg = *config_arg;
  pme->cam_mctl_thread_status = MCTL_THREAD_OPEN_REQUESTED;
  pthread_cond_init (&pme->cam_mctl_thread_ready_cond, NULL);
  pthread_mutex_init (&pme->cam_mctl_thread_ready_mutex, NULL);

end:
#ifdef FEATURE_GYRO
  if (NULL == pme->p_cfg_ctrl->p_client_ops) {
    if (dsps_proc_init() == 0) {
      dsps_set_data.msg_type = DSPS_ENABLE_REQ;
      dsps_set_data.sensor_feature.mode = ENABLE_ALL;
      dsps_proc_set_params(&dsps_set_data);
    }
  }
#endif
  return pme;
}

static int wait_cam_mctl_thread_ready(m_ctrl_t* pme)
{
  int rc = 0;

  if (!pme) {
    CDBG_ERROR("%s: NULL pme", __func__);
    rc = -EINVAL;
    return rc;
  }

  pthread_mutex_lock(&pme->cam_mctl_thread_ready_mutex);
  if (MCTL_THREAD_OPEN_REQUESTED == pme->cam_mctl_thread_status)
    pthread_cond_wait(&pme->cam_mctl_thread_ready_cond,
      &pme->cam_mctl_thread_ready_mutex);

  if (MCTL_THREAD_OPEN_FAILED == pme->cam_mctl_thread_status) {
    pthread_mutex_unlock(&pme->cam_mctl_thread_ready_mutex);
    CDBG_ERROR("%s: mctl thread opened but failed during init", __func__);
    rc = pthread_join(pme->cam_mctl_thread_id, NULL);
    return -rc;
  }
  pthread_mutex_unlock(&pme->cam_mctl_thread_ready_mutex);

  return rc;
} /* wait_cam_mctl_thread_ready */

void cam_mctl_thread_ready_signal(m_ctrl_t* pme, mctl_thread_status status)
{
  if (!pme) {
    CDBG_ERROR("%s: NULL pme", __func__);
    return;
  }
  /*
   * Send signal to calling thread to indicate that this thread is
   * ready or failed to open.
   */
  CDBG("cam_mctl_thread() is ready, call pthread_cond_signal\n");

  pthread_mutex_lock(&pme->cam_mctl_thread_ready_mutex);
  pme->cam_mctl_thread_status = status;
  pthread_cond_signal(&pme->cam_mctl_thread_ready_cond);
  pthread_mutex_unlock(&pme->cam_mctl_thread_ready_mutex);

  CDBG(" call pthread_cond_signal done\n");
} /* cam_mctl_thread_ready_signal */

static void *cam_mctl_thread(void *data)
{
  int read_len;
  int rc, timeoutms, cam_fd = -1, nfds = 0;
  int pipe_readfd, pipe_writefd, server_fd;
  int ez_pipe_readfd, ez_client_fd;
  int ez_prev_pipe_readfd, ez_prev_client_fd;

  uint8_t *ctrl_cmd_buffer;
  uint8_t stats_event_data[MCTL_CONFIG_MAX_LEN];

  struct msm_stats_event_ctrl confCmd;
  struct v4l2_event_subscription sub;
  struct pollfd fds[MCTL_POLL_SLOT_MAX]; /* fd_set fds; */
  struct v4l2_event v4l2_event;
  struct config_thread_arguments *arg = NULL;
  mctl_config_ctrl_t *p_cfg_ctrl = NULL;
  m_ctrl_t *pme = (m_ctrl_t *)data;
  pme->p_cfg_ctrl->p_client_ops = pme->cfg_arg.p_client_ops;

  CDBG("%s: enter!\n", __func__);

  if (!pme) {
    CDBG_ERROR("%s: NULL pme", __func__);
    rc = -EINVAL;
    goto thread_creation_failed;
  }
  arg = &(pme->cfg_arg);
  p_cfg_ctrl = pme->p_cfg_ctrl;

  rc = mctl_init(pme);
  if (rc < 0) {
    CDBG_ERROR("%s: mctl_init() error", __func__);
    goto thread_creation_failed;
  }

  pipe_readfd = arg->read_fd;
  pipe_writefd = arg->write_fd;
  server_fd = arg->server_fd;
  ez_pipe_readfd = arg->ez_read_fd;
  ez_client_fd = -1;
  ez_prev_pipe_readfd = arg->ez_prev_read_fd;
  ez_prev_client_fd = -1;

  CDBG("%s fds are %d, %d \n", __func__, pipe_readfd, pipe_writefd);
  cam_fd = pme->p_cfg_ctrl->camfd;

  ctrl_cmd_buffer = malloc(MAX_SERVER_PAYLOAD_LENGTH);
  if (ctrl_cmd_buffer == NULL) {
      CDBG_ERROR("%s: Could not allocate payload buffer\n", __func__);
      goto thread_creation_failed;
  }

  sub.type = V4L2_EVENT_ALL;
  rc = ioctl(cam_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_SUBSCRIBE_EVENT failed : %s\n", __func__,
      strerror(errno));
    goto subscribe_failed;
  }

  if (create_camfd_receive_socket(
        &pme->p_cfg_ctrl->video_ctrl.socket_info, arg->vnode_id) < 0) {
    CDBG_ERROR("%s: Failed to create_camfd_receive_socket\n", __func__);
    goto socket_creation_failed;
  }

  memset(&p_cfg_ctrl->video_ctrl.user_buf_info, 0,
         sizeof(p_cfg_ctrl->video_ctrl.user_buf_info));
  memset(&p_cfg_ctrl->video_ctrl.mctl_buf_info, 0,
         sizeof(p_cfg_ctrl->video_ctrl.mctl_buf_info));

  /* Notify caller that this thread is ready */
  cam_mctl_thread_ready_signal(pme, MCTL_THREAD_RUNNING);

  p_cfg_ctrl->state = CAMERA_STATE_IDLE;
  timeoutms = 6000;
  p_cfg_ctrl->eztune_preview_flag = 0;

  do {
    CDBG("%s ...... entering config duty loop ...... \n", __func__);
    fds[MCTL_POLL_SLOT_CONFIG_FD].fd = cam_fd;
    fds[MCTL_POLL_SLOT_CONFIG_FD].events = POLLPRI;
    fds[MCTL_POLL_SLOT_DAEMON_PIPE].fd = pipe_readfd;
    fds[MCTL_POLL_SLOT_DAEMON_PIPE].events = POLLPRI | POLLIN;
    fds[MCTL_POLL_SLOT_EZTUNE_PIPE_READ_FD].fd = ez_pipe_readfd;
    fds[MCTL_POLL_SLOT_EZTUNE_PIPE_READ_FD].events = POLLIN;
    fds[MCTL_POLL_SLOT_EZTUNE_CLIENT_FD].fd = ez_client_fd;
    fds[MCTL_POLL_SLOT_EZTUNE_CLIENT_FD].events = POLLIN;
    fds[MCTL_POLL_SLOT_EZTUNE_PREV_PIPE_FD].fd = ez_prev_pipe_readfd;
    fds[MCTL_POLL_SLOT_EZTUNE_PREV_PIPE_FD].events = POLLIN;
    fds[MCTL_POLL_SLOT_EZTUNE_PREV_CLIENT_FD].fd = ez_prev_client_fd;
    fds[MCTL_POLL_SLOT_EZTUNE_PREV_CLIENT_FD].events = POLLIN;
    fds[MCTL_POLL_SLOT_DOMAIN_SOCKET_FD].fd = p_cfg_ctrl->video_ctrl.socket_info.socket_fd;
    fds[MCTL_POLL_SLOT_DOMAIN_SOCKET_FD].events = POLLIN|POLLRDNORM;
    if (p_cfg_ctrl->pp_node.fd > 0) {
      fds[MCTL_POLL_SLOT_PP_NODE_FD].fd = p_cfg_ctrl->pp_node.fd;
      fds[MCTL_POLL_SLOT_PP_NODE_FD].events = POLLIN|POLLRDNORM|POLLPRI;
    } else {
      /* Dont poll */
      fds[MCTL_POLL_SLOT_PP_NODE_FD].fd = -1;
    }
    rc = poll(fds, MCTL_POLL_SLOT_MAX, timeoutms);

    if (rc == 0) {
      usleep(1000 * 100);
      continue;
    } else if (rc < 0) {
      CDBG("SELECT ERROR %s \n", strerror(errno));
      usleep(1000 * 100);
      continue;
    } else {
      /* evt/msg from qcam server */
      if (fds[MCTL_POLL_SLOT_DAEMON_PIPE].revents & POLLPRI ||
          fds[MCTL_POLL_SLOT_DAEMON_PIPE].revents & POLLIN) {
        struct msm_ctrl_cmd *ctrl;
        struct msm_isp_event_ctrl event_data;

        CDBG("%s MCTL_POLL_SLOT_DAEMON_PIPE \n", __func__);

        /* read event control struct from pipe */
        read_len = read(pipe_readfd, &event_data,
                     sizeof(struct msm_isp_event_ctrl));
        if (read_len <= 0) {
          CDBG_HIGH("%s Error reading isp event from pipe ", __func__);
          continue;
        }
        memset(ctrl_cmd_buffer, 0, MAX_SERVER_PAYLOAD_LENGTH);
        event_data.isp_data.ctrl.value = (void *)ctrl_cmd_buffer;
        if (event_data.isp_data.ctrl.length > 0) {
          CDBG("%s Received ctrl cmd from pipe type %d \n", __func__,
            event_data.isp_data.ctrl.type);
          read_len = read(pipe_readfd, event_data.isp_data.ctrl.value,
            event_data.isp_data.ctrl.length);
          if (read_len <= 0) {
            CDBG_HIGH("%s Error reading ctrlcmd from pipe ", __func__);
            continue;
          }
        }
        ctrl = &(event_data.isp_data.ctrl);
        ctrl->resp_fd = server_fd;
        CDBG("%s Received ctrl command %d\n", __func__, ctrl->type);
        if (ctrl->type == MSM_V4L2_CLOSE) {
          config_shutdown_pp(pme->p_cfg_ctrl);
          rc = mctl_send_ctrl_cmd_done(pme->p_cfg_ctrl, NULL, TRUE);
          if (rc < 0) {
            CDBG_ERROR("%s: sending ctrl_cmd_done failed rc = %d\n",
              __func__, rc);
          }
          break;
        } else {
          if (mctl_proc_v4l2_request(pme, ctrl) < 0)
            CDBG_HIGH("%s: mctl_proc_v4l2_request failed", __func__);
        }
      }

      /* evt/msg from config node */
      if (fds[MCTL_POLL_SLOT_CONFIG_FD].revents & POLLPRI) {
        /* event available */
        struct msm_isp_event_ctrl event_data;

        CDBG("%s: MCTL_POLL_SLOT_CONFIG_FD", __func__);

        event_data.isp_data.isp_msg.data = (void *)stats_event_data;
        *((uint32_t *)v4l2_event.u.data) = (uint32_t)&event_data;

        rc = ioctl(cam_fd, VIDIOC_DQEVENT, &v4l2_event);
        if (rc >= 0) {
          CDBG("%s: VIDIOC_DQEVENT type = 0x%x\n", __func__, v4l2_event.type);
          if (v4l2_event.type ==
            V4L2_EVENT_PRIVATE_START + MSM_CAM_RESP_DIV_FRAME_EVT_MSG) {
            CDBG("%s: VIDIOC_DQEVENT type = 0x%x, mctl_divert_frame\n",
                 __func__, v4l2_event.type);
            mctl_divert_frame(p_cfg_ctrl,
              (void *)&(event_data.isp_data.div_frame));
          } else if(v4l2_event.type ==
            V4L2_EVENT_PRIVATE_START + MSM_CAM_RESP_MCTL_PP_EVENT) {
            CDBG("%s: MSM_CAM_RESP_MCTL_PP_EVENT", __func__);
            mctl_pp_proc_event(p_cfg_ctrl,
              (void *)&(event_data.isp_data.pp_event_info));
          } else if (v4l2_event.type ==
            V4L2_EVENT_PRIVATE_START + MSM_CAM_RESP_STAT_EVT_MSG) {
            struct msm_cam_evt_msg *isp_adsp = &(event_data.isp_data.isp_msg);
            CDBG("%s: got isp command = 0x%x\n", __func__, isp_adsp->type);
            CDBG("%s: config received msgId%d\n", __func__, isp_adsp->msg_id);
            mctl_proc_event_message (pme, isp_adsp);
          } else {
            CDBG_HIGH("%s: Error: should not be here", __func__);
          }
        } else {
          CDBG_HIGH("%s: VIDIOC_DQEVENT type failed\n", __func__);
        }
      }

      /* evt/msg from eztune pipe */
      if ((fds[MCTL_POLL_SLOT_EZTUNE_PIPE_READ_FD].revents & POLLIN)
           == POLLIN) {
        CDBG("%s: received events from eztune pipe", __func__);

        read(ez_pipe_readfd, &ez_client_fd, sizeof(int));
        CDBG("__debug: %s server_connect ez_client_fd = %d\n",
          __func__, ez_client_fd);
        if (ez_client_fd > 0)
        mctl_eztune_server_connect(pme, ez_client_fd);
        CDBG("__debug: %s server_connect done\n", __func__);
      }

      /* evt/msg from eztune client */
      if ((fds[MCTL_POLL_SLOT_EZTUNE_CLIENT_FD].revents & POLLIN) == POLLIN) {
        if (ez_client_fd > 0) {
          CDBG("%s: received events from eztune client", __func__);
          mctl_eztune_read_and_proc_cmd(EZ_MCTL_SOCKET_CMD);
        }
      }

      /* evt/msg from eztune prev pipe */
      if ((fds[MCTL_POLL_SLOT_EZTUNE_PREV_PIPE_FD].revents & POLLIN)
           == POLLIN) {
        CDBG("%s: received events from eztune prev pipe", __func__);

        read(ez_prev_pipe_readfd, &ez_prev_client_fd, sizeof(int));
        CDBG("__debug: %s prev server_connect ez_prev_client_fd = %d\n",
          __func__, ez_prev_client_fd);
        if (ez_prev_client_fd > 0)
          mctl_eztune_prev_server_connect(pme, ez_prev_client_fd);
        CDBG("__debug: %s prev server_connect done\n", __func__);
      }

      /* evt/msg from eztune prev client */
      if ((fds[MCTL_POLL_SLOT_EZTUNE_PREV_CLIENT_FD].revents & POLLIN)
           == POLLIN) {
        if (ez_prev_client_fd > 0) {
          CDBG("%s: received events from eztune prev client", __func__);
          mctl_eztune_read_and_proc_cmd(EZ_MCTL_PREV_SOCKET_CMD);
        }
      }

      /* Events on user created socket */
      if ((fds[MCTL_POLL_SLOT_DOMAIN_SOCKET_FD].revents & POLLIN) ||
          (fds[MCTL_POLL_SLOT_DOMAIN_SOCKET_FD].revents & POLLRDNORM)) {
        CDBG_HIGH("%s: Woke up Socket fd\n", __func__);
        if (mctl_divert_socket_recvmsg(fds[MCTL_POLL_SLOT_DOMAIN_SOCKET_FD].fd,
          p_cfg_ctrl) <= 0) {
          CDBG_ERROR(" %s: recvmsg failed", __func__);
          continue;
        }

        mctl_divert_socket_get_buf_data(p_cfg_ctrl);
      }

      if (p_cfg_ctrl->pp_node.fd > 0) {
        /* Events on mctl pp node */
        if ((fds[MCTL_POLL_SLOT_PP_NODE_FD].revents & POLLPRI) ||
           ((fds[MCTL_POLL_SLOT_PP_NODE_FD].revents & POLLIN)
             && (fds[MCTL_POLL_SLOT_PP_NODE_FD].revents & POLLRDNORM))) {
          mctl_pp_node_proc_evt(&p_cfg_ctrl->pp_node,
            &fds[MCTL_POLL_SLOT_PP_NODE_FD]);
        }
      }
    }
  } while(TRUE);

  /* close the domain socket */
  if (p_cfg_ctrl->video_ctrl.socket_info.socket_fd > 0)
    close_camfd_receive_socket(&pme->p_cfg_ctrl->video_ctrl.socket_info,
                               pme->p_cfg_ctrl->cfg_arg.vnode_id);

  CDBG("%s: CAMERA_EXIT\n", __func__);
  if (ioctl(cam_fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub) < 0)
    CDBG_ERROR("%s: config node VIDIOC_UNSUBSCRIBE_EVENT failed %s", __func__,
      strerror(errno));
  if (ctrl_cmd_buffer)
    free(ctrl_cmd_buffer);
  mctl_release(pme);
  return NULL;

socket_creation_failed:
  if (ioctl(cam_fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub) < 0)
    CDBG_ERROR("%s: config node VIDIOC_UNSUBSCRIBE_EVENT failed %s", __func__,
      strerror(errno));

subscribe_failed:
  free(ctrl_cmd_buffer);

thread_creation_failed:
  if (mctl_deinit(pme) < 0)
    CDBG_HIGH("%s: mctl_deinit failed", __func__);

  cam_mctl_thread_ready_signal(pme, MCTL_THREAD_OPEN_FAILED);
  return NULL;
}

void *create_v4l2_conf_thread(struct config_thread_arguments* arg)
{
  int rc = 0;
  m_ctrl_t* pme = NULL;

  /*creat a mctl object and use it to launch the thread*/
  if (arg) {
    pme = mctl_create(arg);
    if (!pme) {
      CDBG_ERROR("%s: mctl_create() error", __func__);
      goto end;
    }

    rc = pthread_create(&pme->cam_mctl_thread_id, NULL, cam_mctl_thread, pme);
    /* if thread is created fine, wait for it to be ready*/
    if (0 == rc) {
      rc = wait_cam_mctl_thread_ready(pme);
      if (rc < 0) {
        CDBG_ERROR("%s: cam_mctl_thread opened but failed to init", __func__);
        goto thread_create_failed;
      } else {
        goto end;
      }
    } else {
      CDBG_ERROR("%s: cam_mctl_thread opened failed", __func__);
      goto thread_create_failed;
    }
  } else {
    CDBG_ERROR("%s: error: no config thread arguments", __func__);
    goto end;
  }
thread_create_failed:
  mctl_delete(pme);
  pme = NULL;
end:
  return (void *)pme;
}

int mctl_init_camera_plugin(void* pme,
      camera_plugin_ops_t *camera_plugin_ops)
{
  int rc = 0;
  m_ctrl_t* ctrl = pme;

  if(!ctrl) {
    CDBG_ERROR("%s: null MCTL handle passed in",  __func__);
    return -1;
  }
  ctrl->p_cfg_ctrl->camera_plugin_ops = camera_plugin_ops;
  if (!ctrl->p_cfg_ctrl->camera_plugin_ops->client_open) {
    CDBG_ERROR("%s: fatal error, no client_open function",  __func__);
    return -1;
  }
  rc = ctrl->p_cfg_ctrl->camera_plugin_ops->client_open(
         ctrl->p_cfg_ctrl->camera_plugin_ops->handle,
         &ctrl->p_cfg_ctrl->plugin_client_ops);
  if( rc != 0) {
    CDBG_ERROR("%s: cannot open camera plugin client, rc = %d",
      __func__,  rc);
    return -1;
  }
  rc = mctl_camera_plugin_init_callbacks(ctrl->p_cfg_ctrl);
  if( rc != 0) {
    CDBG_ERROR("%s: mctl_camera_plugin_init err, rc = %d",
      __func__,  rc);
    return -1;
  }

  CDBG("%s: done, rc = %d", __func__, rc);
  return rc;
}

int destroy_v4l2_cam_conf_thread(void *handle)
{
  int rc = 0;
  m_ctrl_t *pme;

  pme = handle;

  if (pme == NULL) {
    CDBG_HIGH("%s: pme is already NULL", __func__);
    return 0;
  }

  rc = pthread_join(pme->cam_mctl_thread_id, NULL);
  mctl_delete(pme);

  return rc;
} /* release_v4l2_cam_conf_thread */

int mctl_load_comps()
{
  int rc = 0;

  CDBG("%s: E", __func__);

  if ((rc = AXI_comp_create())) {
    CDBG_ERROR("%s: AXI_comp_create() error = %d", __func__, rc);
    goto AXI_comp_create_failed;
  }
  if ((rc = sensor_comp_create())) {
    CDBG_ERROR("%s: sensor_comp_create() error = %d", __func__, rc);
    goto sensor_comp_create_failed;
  }
  if ((rc = flash_led_comp_create())) {
    CDBG_ERROR("%s: flash_led_comp_create() error = %d", __func__, rc);
    goto flash_led_comp_create_failed;
  }
  if ((rc = flash_strobe_comp_create())) {
    CDBG_ERROR("%s: flash_strobe_comp_create() error = %d", __func__, rc);
    goto flash_strobe_comp_create_failed;
  }
  if ((rc = CAMIF_comp_create())) {
    CDBG_ERROR("%s: CAMIF_comp_create() error = %d", __func__, rc);
    goto CAMIF_comp_create_failed;
  }
  if ((rc = VFE_comp_create())) {
    CDBG_ERROR("%s: VFE_comp_create() error = %d", __func__, rc);
    goto VFE_comp_create_failed;
  }
  if ((rc = ACTUATOR_comp_create())) {
    CDBG_ERROR("%s: ACTUATOR_comp_create() error = %d", __func__, rc);
    goto ACTUATOR_comp_create_failed;
  }

  if ((rc = eeprom_comp_create())) {
    CDBG_ERROR("%s: eeprom_comp_create() error = %d", __func__, rc);
    goto eeprom_comp_create_failed;
  }

  if ((rc = mctl_load_stats_proc_lib())) {
    CDBG_ERROR("%s: mctl_load_stats_proc_lib() error = %d", __func__, rc);
    goto stats_proc_lib_load_failed;
  } else {
    rc = (my_shared_stats_proc_lib.comp_create)();
    if (rc) {
      CDBG_ERROR("%s: STATSPROC_comp_create error = %d", __func__, rc);
      goto stats_proc_lib_create_failed;
    }
  }
  if ((rc = mctl_load_frame_proc_lib())) {
    CDBG_ERROR("%s: mctl_load_frame_proc_lib() error = %d", __func__, rc);
    goto frame_proc_lib_load_failed;
  } else {
    rc = (my_shared_frame_proc_lib.comp_create)();
    if (rc) {
      CDBG_ERROR("%s: FRAME_PROC_comp_create error = %d", __func__, rc);
      goto frame_proc_lib_create_failed;
    }
  }

  if ((rc = ISPIF_comp_create())) {
    CDBG_ERROR("%s: ISPIF_comp_create() error = %d", __func__, rc);
    goto ispif_fail;
  }

  CDBG("%s: X", __func__);
  return rc;

ispif_fail:
frame_proc_lib_create_failed:
  if (my_shared_frame_proc_lib.ptr)
    dlclose(my_shared_frame_proc_lib.ptr);
  memset(&my_shared_frame_proc_lib, 0, sizeof(my_shared_frame_proc_lib));
frame_proc_lib_load_failed:
  if (!(my_shared_stats_proc_lib.comp_destroy)())
    CDBG_HIGH("%s: my_shared_stats_proc_lib.ptr.comp_destroy failed", __func__);
stats_proc_lib_create_failed:
  if (my_shared_stats_proc_lib.ptr)
    dlclose(my_shared_stats_proc_lib.ptr);
  memset(&my_shared_stats_proc_lib, 0, sizeof(my_shared_stats_proc_lib));
stats_proc_lib_load_failed:
  if(!eeprom_comp_destroy())
    CDBG_HIGH("%s: eeprom_comp_destroy() failed", __func__);
eeprom_comp_create_failed:
  if(!ACTUATOR_comp_destroy())
    CDBG_HIGH("%s: ACTUATOR_comp_destroy() failed", __func__);
ACTUATOR_comp_create_failed:
  if(!VFE_comp_destroy())
    CDBG_HIGH("%s: VFE_comp_destroy() failed", __func__);
VFE_comp_create_failed:
  if(!CAMIF_comp_destroy())
    CDBG_HIGH("%s: CAMIF_comp_destroy() failed", __func__);
CAMIF_comp_create_failed:
  if(!flash_strobe_comp_destroy())
    CDBG_HIGH("%s: flash_strobe_comp_destroy() failed", __func__);
flash_strobe_comp_create_failed:
  if(!flash_led_comp_destroy())
    CDBG_HIGH("%s: flash_led_comp_destroy() failed", __func__);
flash_led_comp_create_failed:
  if(!sensor_comp_destroy())
    CDBG_HIGH("%s: sensor_comp_destroy() failed", __func__);
sensor_comp_create_failed:
  if(!AXI_comp_destroy())
    CDBG_HIGH("%s: AXI_comp_destroy() failed", __func__);
AXI_comp_create_failed:
  return rc;
}

int mctl_unload_comps()
{
  int rc = 0;
  CDBG("%s: E", __func__);

  /* Even if destroy for one component fails, continue to destroy rest. */
  if(!(rc = AXI_comp_destroy()))
    CDBG_ERROR("%s: AXI_comp_destroy() error = %d", __func__, rc);

  if(!(rc = sensor_comp_destroy()))
    CDBG_ERROR("%s: sensor_comp_destroy() error = %d", __func__, rc);

  if(!(rc = flash_led_comp_destroy()))
    CDBG_ERROR("%s: flash_led_comp_destroy() error = %d", __func__, rc);

  if(!(rc = flash_strobe_comp_destroy()))
    CDBG_ERROR("%s: flash_strobe_comp_destroy() error = %d", __func__, rc);

  if(!(rc = CAMIF_comp_destroy()))
    CDBG_ERROR("%s: CAMIF_comp_destroy() error = %d", __func__, rc);

  if(!(rc = VFE_comp_destroy()))
    CDBG_ERROR("%s: VFE_comp_destroy() error = %d", __func__, rc);

  if(!(rc = ACTUATOR_comp_destroy()))
    CDBG_ERROR("%s: ACTUATOR_comp_destroy() error = %d", __func__, rc);

  if(!(rc = eeprom_comp_destroy()))
    CDBG_ERROR("%s: eeprom_comp_destroy() error = %d", __func__, rc);

  if(!(rc = ISPIF_comp_destroy()))
    CDBG_ERROR("%s: ISPIF_comp_destroy() error = %d", __func__, rc);

  my_shared_stats_proc_lib.comp_destroy();
  dlclose(my_shared_stats_proc_lib.ptr);
  memset(&my_shared_stats_proc_lib,  0,  sizeof(my_shared_stats_proc_lib));

  my_shared_frame_proc_lib.comp_destroy();
  dlclose(my_shared_frame_proc_lib.ptr);
  memset(&my_shared_frame_proc_lib,  0,  sizeof(my_shared_frame_proc_lib));

  CDBG("%s: X", __func__);
  return rc;
}

int mctl_open_and_init_comps(mctl_config_ctrl_t *ctrl)
{
  uint32_t comp_mask = 0;
  int rc = 0;

  memset(&(ctrl->stats_proc_ctrl),
         0, sizeof(stats_proc_ctrl_t));
  memset(&(ctrl->frame_proc_ctrl),
           0, sizeof(frame_proc_ctrl_t));
  CDBG("%s actuator %d, led %d, strobe %d\n", __func__,
    ctrl->cam_sensor_info.actuator_enabled,
    ctrl->cam_sensor_info.flash_enabled,
    ctrl->cam_sensor_info.strobe_flash_enabled);

  if (ctrl->cam_sensor_info.actuator_enabled)
    comp_mask = (1 << MCTL_COMPID_ACTUATOR);

  if (ctrl->cam_sensor_info.ispif_supported)
    comp_mask |= (1 << MCTL_COMPID_ISPIF);

  if (ctrl->video_ctrl.use_pix_interface) {
    switch(ctrl->sensorCtrl.sensor_output.output_format) {
    case SENSOR_BAYER:
      CDBG("mctl_open_and_init_comps: BAYER\n");
      comp_mask |= (1 << MCTL_COMPID_AXI) | (1 << MCTL_COMPID_CAMIF) |
        (1 << MCTL_COMPID_VFE) |
        (1 << MCTL_COMPID_STATSPROC) | (1 << MCTL_COMPID_FRAMEPROC) |
        (1 << MCTL_COMPID_SENSOR) | (1 << MCTL_COMPID_FLASHLED) |
        (1 << MCTL_COMPID_FLASHSTROBE) | (1 << MCTL_COMPID_EEPROM) |
        (1 << MCTL_COMPID_CSI);
      break;
    case SENSOR_YCBCR:
      CDBG("mctl_open_and_init_comps: YUV\n");
      comp_mask |= (1 << MCTL_COMPID_AXI) | (1 << MCTL_COMPID_CAMIF) |
        (1 << MCTL_COMPID_VFE) |
        (1 << MCTL_COMPID_FRAMEPROC) | (1 << MCTL_COMPID_SENSOR) |
        (1 << MCTL_COMPID_FLASHLED) | (1 << MCTL_COMPID_FLASHSTROBE) |
        (1 << MCTL_COMPID_CSI);
      break;
    default:
      /* need more logic for select VFE/camif */
      if(ctrl->how_to_sel_vfe == MCTL_YUV_SENSOR_VFE_HW_SEL_RULE_FIRST_IN_FIRST_SERVE) {
        break;
      } else if(ctrl->how_to_sel_vfe == MCTL_YUV_SENSOR_VFE_HW_SEL_ALWAYS_RDI) {
        comp_mask |= (1 << MCTL_COMPID_SENSOR) | (1 << MCTL_COMPID_AXI) |
          (1 << MCTL_COMPID_CSI);
      }
      break;
    }
  } else {
    /* this is RDI only camera */
    comp_mask |= ((1<< MCTL_COMPID_SENSOR) | (1 << MCTL_COMPID_AXI)|
    (1 << MCTL_COMPID_FLASHLED) | (1 << MCTL_COMPID_FLASHSTROBE) |
    (1 << MCTL_COMPID_CSI));
  }

  if((rc = mctl_open_comps(ctrl, &comp_mask)) != 0) {
    CDBG_ERROR("%s: mctl_open_comps failed, rc = %d", __func__, rc);
    mctl_set_state(ctrl, MCTL_CTRL_STATE_COMP_INIT_ERR);
    goto end;
  }
  if((rc = mctl_init_comps(ctrl, comp_mask)) != 0) {
    CDBG_ERROR("%s: mctl_init_comp failed, rc = %d", __func__, rc);
    mctl_set_state(ctrl, MCTL_CTRL_STATE_COMP_INIT_ERR);
    goto end;
  }
  mctl_set_state(ctrl, MCTL_CTRL_STATE_SERVICE_READY);
end:
  return rc;
}

/*===========================================================================
 * FUNCTION    - mctl_send_ctrl_cmd_done -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_send_ctrl_cmd_done(mctl_config_ctrl_t *ctrl,
  struct msm_ctrl_cmd *ctrlCmd, int client_only)
{
  int rc = TRUE;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  CDBG("%s: client ops %p %p \n", __func__, ctrl->p_client_ops,
   (ctrl->p_client_ops) ? ctrl->p_client_ops->ctrl_cmd_done : NULL);
  if (ctrl->p_client_ops && ctrl->p_client_ops->ctrl_cmd_done) {
    struct msm_ctrl_cmd *p_nativecmd = NULL;
    uint8_t *data_ptr = (ctrlCmd) ? (uint8_t *)ctrlCmd->value : NULL;
    p_nativecmd = (struct msm_ctrl_cmd *)data_ptr;
    CDBG("%s: sending CTRLCMD ACK to client %p", __func__, p_nativecmd);
    ctrl->p_client_ops->ctrl_cmd_done(ctrl->p_client_ops->handle,
      p_nativecmd);
  } else if (!client_only) {
    CDBG("%s: sending MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE to kernel! "
      "length = %d, status = %d\n", __func__,
      ctrlCmd->length, ctrlCmd->status);
    v4l2_ioctl.ioctl_ptr = ctrlCmd;
    if (ioctl(ctrlCmd->resp_fd, MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE,
      &v4l2_ioctl) < 0) {
      CDBG_ERROR("(%d)IOCTL MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE type %d failed\n",
        __LINE__, ctrlCmd->type);
      rc = FALSE;
    }
  }
  return rc;
}/*mctl_send_ctrl_cmd_done*/

static int mctl_camera_plugin_process_dis (void *user_data,
  camera_plugin_process_dis_t *data_dis)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = user_data;
  return rc;
}
static int mctl_camera_plugin_process_zoom (void *user_data,
  camera_plugin_process_zoom_t *data_zoom)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = user_data;

  switch(data_zoom->type) {
  case CAM_OEM_PLUGIN_PROC_ZOOM_SET_ZOOM_TABLE:
    rc = zoom_ctrl_save_plugin_table(&ctrl->zoomCtrl,
                                data_zoom->zoom_table.num_entry,
                                data_zoom->zoom_table.camera_zoom_table);
    break;
  default:
    rc = -1;
    break;
  }
  return rc;
}
static int mctl_camera_plugin_process_vfe (void *user_data,
  camera_plugin_process_vfe_t *data_vfe,
  int32_t immediate_update)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = user_data;
  vfe_plugin_module_params_t vfe_input;

  if(data_vfe->type == CAM_OEM_PLUGIN_PROC_MOD_UPDATE) {
    vfe_input.type = data_vfe->reg_update.entry->module_type;
    vfe_input.mod_ptr = data_vfe->reg_update.entry->reg_update_data;
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
           ctrl->comp_ops[MCTL_COMPID_VFE].handle,
           VFE_PLUGIN_MOD_UPDATE,
           &vfe_input);
    if (rc < 0) {
      CDBG_ERROR("%s VFE_PLUGIN_MOD_UPDATE failed %d ", __func__, rc);
      goto error;
    }
  } else if (data_vfe->type == CAM_OEM_PLUGIN_PROC_REG_UPDATE) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
           ctrl->comp_ops[MCTL_COMPID_VFE].handle,
           VFE_PLUGIN_REG_UPDATE,
           NULL);
    if (rc < 0) {
      CDBG_ERROR("%s VFE_PLUGIN_REG_UPDATE failed %d ", __func__, rc);
      goto error;
    }
  } else
    CDBG_HIGH("%s: Invalid process type %d\n", __func__, data_vfe->type);

error:
  return rc;
}

static int mctl_camera_plugin_private_event(void *user_data,
                                            uint32_t data_length,
                                            uint32_t trans_id, void *data)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = user_data;
  mm_camera_event_t *cam_event;
  struct v4l2_event_and_payload v4l2_ev;
  v4l2_ev.payload_length = data_length;
  v4l2_ev.transaction_id = trans_id;
  v4l2_ev.payload = data;
  v4l2_ev.evt.type = V4L2_EVENT_PRIVATE_START + MSM_CAM_APP_NOTIFY_EVENT;
  cam_event = (mm_camera_event_t *)v4l2_ev.evt.u.data;
  cam_event->event_type = MM_CAMERA_EVT_TYPE_PRIVATE_EVT;
  cam_event->e.pri_evt.data_length = data_length;
  cam_event->e.pri_evt.trans_id = trans_id;
  rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_V4L2_EVT_NOTIFY, &v4l2_ev);
  if (rc < 0) {
      CDBG("%s: ERROR MSM_CAM_IOCTL_V4L2_EVT_NOTIFY, event = 0x%x, rc = %d\n",
      __func__, v4l2_ev.evt.type, rc);
  }
  return rc;
}

static int mctl_camera_plugin_process_sensor (void *user_data,
  camera_plugin_ioctl_data_t *sensor_proc_data_in, void *result)
{
  int rc = 0;
  sensor_set_t sensor_set_data;
  struct sensor_oem_setting oem_setting;
  mctl_config_ctrl_t *ctrl = user_data;

  oem_setting.type = sensor_proc_data_in->type;
  oem_setting.data = sensor_proc_data_in->data;
  sensor_set_data.type = SENSOR_SET_OEM;
  sensor_set_data.data.oem_setting = &oem_setting;
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
                           ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
                           SENSOR_SET_OEM,
                           (void *)&sensor_set_data, result);
  return rc;
}

static int mctl_camera_plugin_init_callbacks(mctl_config_ctrl_t *ctrl)
{
  int rc = 0;
  camera_plugin_mctl_process_ops_t *ops =
    &ctrl->mctl_ops_for_plugin;
  camera_plugin_ops_t *camera_plugin_ops = ctrl->camera_plugin_ops;
  camera_plugin_client_ops_t *plugin_client_ops = &ctrl->plugin_client_ops;

  ops->user_data = (void *)ctrl;
  ops->process_dis = mctl_camera_plugin_process_dis;
  ops->process_zoom = mctl_camera_plugin_process_zoom;
  ops->process_vfe = mctl_camera_plugin_process_vfe;
  ops->send_private_event = mctl_camera_plugin_private_event;
  ops->process_sensor = mctl_camera_plugin_process_sensor;

  rc = plugin_client_ops->client_init (camera_plugin_ops->handle,
                                       plugin_client_ops->client_handle,
                                       ops);
  return rc;
}

void mctl_timing_notify_to_cam_plugin(mctl_config_ctrl_t *ctrl,
  uint32_t timing, void *data)
{
  int rc = 0;

  rc = ctrl->plugin_client_ops.isp_timing_notify_to_plugin(
    ctrl->camera_plugin_ops->handle,
    ctrl->plugin_client_ops.client_handle, timing, data);
  if (rc < 0)
    CDBG_ERROR("%s: timing_enum = %d, error, rc = %d",
      __func__,  timing, rc);
}

int mctl_find_resolution_image_mode (mctl_config_ctrl_t *ctrl,
	  uint32_t image_mode, uint8_t user_buf, camera_resolution_t *res)
{
  int i;
  cam_stream_info_def_t *buf_ptr;

  if (user_buf)
    buf_ptr = &ctrl->video_ctrl.strm_info.user[0];
  else
    buf_ptr = &ctrl->video_ctrl.strm_info.mctl[0];
  for (i = 0; i < MSM_V4L2_EXT_CAPTURE_MODE_MAX; i++) {
    if (image_mode == buf_ptr[i].image_mode) {
      res->format =  buf_ptr[i].format;
      res->width =  buf_ptr[i].width;
      res->height =  buf_ptr[i].height;
      res->inst_handle = buf_ptr[i].inst_handle;
      return 0;
    }
  }
  return -1;
}

static void mctl_store_curr_target(mctl_config_ctrl_t *ctrl)
{
  int id = 0;
  FILE *fp;
  if ((fp = fopen("/sys/devices/soc0/soc_id", "r")) != NULL) {
    fscanf(fp, "%d", &id);
    fclose(fp);
  }
  /* Compare the read soc id against the ids assosciated
   * with targets as per the file
   * kernel/arch/arm/mach-msm/socinfo.c */
  switch (id) {
    /* 8930 IDs */
    case 116:
    case 117:
    case 118:
    case 119:
    /* 8930AA IDs */
    case 142:
    case 143:
    case 144:
    case 160:
    /* 8930AB IDs */
    case 154:
    case 155:
    case 156:
    case 157:
      ctrl->current_target = TARGET_MSM8930;
      break;
    default:
      ctrl->current_target = TARGET_DEFAULT;
      break;
  }
  CDBG_HIGH("%s Current target is %d \n", __func__, ctrl->current_target);
}
