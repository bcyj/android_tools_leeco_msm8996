/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "camera_dbg.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "isp_def.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_hw.h"
#include "isp.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "q3a_stats_hw.h"
#include "aec/aec.h"
#include "isp_log.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef PIPELINE_DEBUG
#undef CDBG
#define CDBG ALOGE
#endif
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

extern int isp_pipeline32_init(isp_hw_pix_dep_t *dep);
extern int isp_pipeline40_init(isp_hw_pix_dep_t *dep);

static int isp_pipeline_module_hw_update(isp_pipeline_t *pix,
  int is_bayer_input);
static int isp_pix_pipeline_module_config(isp_pipeline_t *pix,
  int is_bayer_input);

/** isp_hw_create_pipeline_dep:
 *    @pix: pointer to pipeline
 *
 *  Initialize pipeline based on ISP Version
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_hw_create_pipeline_dep(isp_pipeline_t *pix)
{
  switch (GET_ISP_MAIN_VERSION(pix->isp_version)) {
  case ISP_VERSION_40:
    return isp_pipeline40_init(&pix->dep);
    break;
  case ISP_VERSION_32:
    return isp_pipeline32_init(&pix->dep);
    break;
  default:
    CDBG_ERROR("%s: not supported isp_version = %d\n", __func__,
      pix->isp_version);
    return -1;
  }
  return 0;
}

/** isp_pipeline_set_stream_config:
 *    @pipeline: Pointer to Pipeline
 *    @in_params: input hw config
 *    @in_params_size:
 *
 *  Configure pipeline stream outputs from input params
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_stream_config(isp_pipeline_t *pipeline,
  isp_hwif_output_cfg_t *in_params, uint32_t in_params_size)
{
  int rc = 0;
  isp_hwif_output_cfg_t *pix_outputs;

  pix_outputs = pipeline->pix_params.cfg_and_3a_params.cfg.outputs;
  pipeline->pix_params.cfg_and_3a_params.cfg.ion_fd = in_params->ion_fd;

  if (in_params->need_uv_subsample) {
    /* decide whether we can perform uv subsampling */
    rc = pipeline->mod_ops[ISP_MOD_SCALER]->get_params(
           pipeline->mod_ops[ISP_MOD_SCALER]->ctrl,
           ISP_PIX_GET_UV_SUBSAMPLE_SUPPORTED,
           (void *)&pipeline->pix_params.cfg_and_3a_params.cfg,
           sizeof(isp_hw_pix_setting_params_t),
           (void *)in_params,
           sizeof(isp_hwif_output_cfg_t));
    if (rc < 0) {
      CDBG_ERROR("%s: ISP_PIX_GET_UV_SUBSAMPLE_SUPPORTED error in scaler, rc = %d\n",
        __func__, rc);
      return rc;
    }
  }

  switch (in_params->axi_path) {
  case CAMIF_RAW:
  case IDEAL_RAW:
    pipeline->pix_params.cfg_and_3a_params.cfg.raw_output = *in_params;
    break;
  case PIX_ENCODER:
    /* camif raw and idea raw share the same pix output cfg slot */
    pix_outputs[ISP_PIX_PATH_ENCODER] = *in_params;
  break;
  case PIX_VIEWFINDER:
    pix_outputs[ISP_PIX_PATH_VIEWFINDER] = *in_params;
    break;
  default:
    CDBG_ERROR("%s: not supported axi_path %d\n",
               __func__, in_params->axi_path);
    return -ERANGE;
  }
  return 0;
}

/** isp_pipeline_set_stream_unconfig:
 *    @pipeline: Pointer to pipeline
 *    @in_params: HW Stream uncfg params
 *    @in_params_size:
 *
 *  Unconfigure and reset the pipeline stream outputs
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_stream_unconfig(isp_pipeline_t *pipeline,
  isp_hw_stream_uncfg_t *in_params, uint32_t in_params_size)
{
  int i, k;
  isp_hwif_output_cfg_t *pix_outputs;

  pix_outputs = pipeline->pix_params.cfg_and_3a_params.cfg.outputs;
  for (i = 0; i < in_params->num_streams; i++) {
    for (k = 0; k < ISP_PIX_PATH_MAX; k++) {
      if (pix_outputs[k].stream_param.width > 0 &&
          pix_outputs[k].stream_param.session_id == in_params->session_id &&
          pix_outputs[k].stream_param.stream_id == in_params->stream_ids[i]) {
        /* zero out the stream cfg memory. */
        memset(&pix_outputs[k], 0, sizeof(isp_hwif_output_cfg_t));
        break;
      }
    }
  }
  return 0;
}

/** isp_pipeline_stats_config_update:
 *    @pipeline: Pointer to pipeline
 *    @stats_config: new config
 *    @in_params_size
 *
 *  New stats window config received. Copy it in pipeline settings and run
 *  config update on stats modules
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_stats_config_update(isp_pipeline_t *pipeline,
  stats_config_t *stats_config, uint32_t in_params_size)
{
  int i, rc = 0;
  isp_hw_pix_setting_params_t *pix_settings =
    &pipeline->pix_params.cfg_and_3a_params.cfg;
  boolean new_config = FALSE;

  if (pipeline->pix_params.cur_module_mask & (1 << ISP_MOD_STATS)) {
    if (sizeof(stats_config_t) != in_params_size) {
      CDBG_ERROR("%s: size mismatch\n", __func__);
      return -1;
    }

    if(stats_config->af_config != NULL) {
      pix_settings->stats_cfg.af_config = *(stats_config->af_config);
      new_config = TRUE;
    }
    if(stats_config->awb_config != NULL) {
      pix_settings->stats_cfg.awb_config = *(stats_config->awb_config);
      new_config = TRUE;
    }
    if(stats_config->aec_config != NULL) {
      pix_settings->stats_cfg.aec_config = *(stats_config->aec_config);
      new_config = TRUE;
    }
    if (new_config) {
      pix_settings->stats_cfg.stats_mask = stats_config->stats_mask;
      rc = pipeline->mod_ops[ISP_MOD_STATS]->set_params(
             pipeline->mod_ops[ISP_MOD_STATS]->ctrl, ISP_HW_MOD_SET_CONFIG_UPDATE,
             pix_settings, sizeof(isp_hw_pix_setting_params_t));
    }
  }

  return rc;
}

/** isp_pripeline_set_stats_fullsize:
 *  @ctrl : Pointer to pileline
 *
 *
 *  Return 0 on Success
 *
 **/
int isp_pipeline_set_stats_fullsize(void *ctrl, boolean enable)
{
  int rc = 0;
  isp_pipeline_t *pipeline = ctrl;
  pipeline->pix_params.cfg_and_3a_params.cfg.do_fullsize_cfg = enable;
  rc = pipeline->mod_ops[ISP_MOD_STATS]->set_params(
    pipeline->mod_ops[ISP_MOD_STATS]->ctrl,
    ISP_HW_MOD_SET_STATS_FULLSIZE_CFG,
    (void *)&pipeline->pix_params.cfg_and_3a_params.cfg,
    sizeof(isp_hw_pix_setting_params_t));
  return rc;
}

/** isp_pipeline_awb_trigger_update:
 *    @pipeline: Pointer to Pipeline
 *    @stats_update: new update
 *    @in_params_size
 *
 *  New AWB update received. Run trigger update on all modules
 *
 *
 *  Return 0 on Success, negative on ERROR
**/
static int isp_pipeline_awb_trigger_update(isp_pipeline_t *pipeline,
  stats_update_t *stats_update, uint32_t in_params_size)
{
  int rc = 0;
  uint8_t is_bayer_input = isp_pipeline_util_is_bayer_fmt(pipeline);

  if (sizeof(stats_update_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  if ((stats_update->flag & STATS_UPDATE_AWB) && is_bayer_input) {
    pipeline->pix_params.cfg_and_3a_params.trigger_input.stats_update.awb_update =
      stats_update->awb_update;
   rc = isp_pipeline_util_trigger_update(pipeline);
  }
  return rc;
}

/** isp_pipeline_aec_trigger_update:
 *    @pipeline: Pointer to Pipeline
 *    @dig_gain: new gain
 *    @in_params_size
 *
 *  New AEC gain received. Run trigger update on all modules
 *
 *
 *  Return 0 on Success, negative on ERROR
**/
static int isp_pipeline_aec_trigger_update(isp_pipeline_t *pipeline,
  float *dig_gain, uint32_t in_params_size)
{
  int rc = 0;
  uint8_t is_bayer_input = isp_pipeline_util_is_bayer_fmt(pipeline);

  if (sizeof(float) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  if (is_bayer_input) {
    pipeline->pix_params.cfg_and_3a_params.trigger_input.digital_gain =
      *dig_gain;
    rc = isp_pipeline_util_trigger_update(pipeline);
  }

  return rc;
}

/** isp_pipeline_set_flash_mode:
 *    @pipeline: Pointer to pipeline
 *    @flash_mode: New flash mode
 *    @in_params_size:
 *
 *  New flash mode received. Run trigger update on all modules
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_flash_mode(isp_pipeline_t *pipeline,
  cam_flash_mode_t *flash_mode, uint32_t in_params_size)
{
  int rc = 0;
  uint8_t is_bayer_input = isp_pipeline_util_is_bayer_fmt(pipeline);

  if (sizeof(cam_flash_mode_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  if (is_bayer_input) {
    pipeline->pix_params.cfg_and_3a_params.trigger_input.flash_mode =
      *flash_mode;
    rc = isp_pipeline_util_trigger_update(pipeline);
  }

  return rc;
}

/** isp_pipeline_lens_position_trigger_update:
 *    @pipeline: Pointer to pipeline
 *    @lens_position_update: New lens position
 *    @in_params_size:
 *
 *  New lens position received. Run trigger update on all modules
 *
 *
 *  Return 0 on success, negative on ERROR
 **/
static int isp_pipeline_sensor_lens_position_trigger_update(
  isp_pipeline_t *pipeline, lens_position_update_isp_t *lens_position_update,
  uint32_t in_params_size)
{
  int rc = 0;
  uint8_t is_bayer_input = isp_pipeline_util_is_bayer_fmt(pipeline);

  isp_hw_pix_trigger_update_params_t *trigger_input =
    &pipeline->pix_params.cfg_and_3a_params.trigger_input;
  if (sizeof(lens_position_update_isp_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  if (is_bayer_input) {
    trigger_input->lens_position_current_step =
      lens_position_update->current_step;
    rc = isp_pipeline_util_trigger_update(pipeline);
  }

  return rc;
}

/** isp_pipeline_set_camif_cfg:
 *    @pix: Pointer to pipeline
 *    @in_params: camif config params
 *    @in_params_size:
 *
 *  Set camif config
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_camif_cfg(isp_pipeline_t *pix,
  isp_pix_camif_cfg_t *in_params, uint32_t in_params_size)
{
  int rc = 0;
  isp_pix_camif_cfg_t *camif_cfg = NULL;

  camif_cfg = &pix->pix_params.cfg_and_3a_params.cfg.camif_cfg;
  pix->pix_params.cfg_and_3a_params.cfg.camif_cfg = *in_params;

  /* fill in camif info for rolloff algo, according to sensor info*/
  pix->pix_params.cfg_and_3a_params.cfg.sensor_rolloff_config =
    camif_cfg->sensor_out_info.sensor_rolloff_config;

  /* fill in demosaic crop info if needed. not needed for vfe40*/
  pix->pix_params.cfg_and_3a_params.cfg.demosaic_output.first_pixel =
    camif_cfg->sensor_out_info.request_crop.first_pixel;
  pix->pix_params.cfg_and_3a_params.cfg.demosaic_output.last_pixel =
    camif_cfg->sensor_out_info.request_crop.last_pixel;
  pix->pix_params.cfg_and_3a_params.cfg.demosaic_output.first_line =
    camif_cfg->sensor_out_info.request_crop.first_line;
  pix->pix_params.cfg_and_3a_params.cfg.demosaic_output.last_line =
    camif_cfg->sensor_out_info.request_crop.last_line;

  return 0;
}


/** isp_pipeline_set_effect
 *    @pipeline: isp pipeline
 *
 *  Common pipeline set effect routine.
 *
 *
 *  Return 0 on SUccess, negative on ERROR
 **/
static int isp_pipeline_set_effect(isp_pipeline_t *pipeline)
{
  int i, num, rc = 0;
  uint16_t *module_ids = NULL;
   uint8_t is_bayer_input = isp_pipeline_util_is_bayer_fmt(pipeline);

   if(!is_bayer_input){
     return 0;
  }
  isp_pix_params_t *params = &pipeline->pix_params;
  isp_pix_trigger_update_input_t *trigger_update_params =
    &params->cfg_and_3a_params;
  isp_stats_udpate_t *stats_update =
    &trigger_update_params->trigger_input.stats_update;

  module_ids = pipeline->dep.mod_trigger_update_order_bayer;
  num = pipeline->dep.num_mod_trigger_update_order_bayer;

  ISP_DBG(ISP_MOD_COM,"%s: effect_mask = 0x%x, contrast = %d, "
    "saturation = %f, special effect = %d\n", __func__,
    params->cfg_and_3a_params.cfg.effects.effect_type_mask,
    params->cfg_and_3a_params.cfg.effects.contrast,
    params->cfg_and_3a_params.cfg.effects.saturation,
    params->cfg_and_3a_params.cfg.effects.spl_effect);
  for (i = 0; i < num; i++) {
    if (((1 << module_ids[i]) & params->cur_module_mask) &&
        pipeline->mod_ops[module_ids[i]] && (module_ids[i]!= ISP_MOD_STATS)) {

      rc = pipeline->mod_ops[module_ids[i]]->set_params(
        pipeline->mod_ops[module_ids[i]]->ctrl,
        ISP_HW_MOD_SET_EFFECT,
        (void *)&pipeline->pix_params.cfg_and_3a_params.cfg,
        sizeof(isp_hw_pix_setting_params_t));
      rc = pipeline->mod_ops[module_ids[i]]->set_params(
        pipeline->mod_ops[module_ids[i]]->ctrl,
        ISP_HW_MOD_SET_TRIGGER_UPDATE,
        (void *)&pipeline->pix_params.cfg_and_3a_params,
        sizeof(isp_pix_trigger_update_input_t));

      if (rc < 0) {
        CDBG_ERROR("%s: module %d config failed\n", __func__, i);
        return rc;
      }
    }
  }
  return 0;
}

/** isp_pipeline_set_saved_params
 *    @pipeline: isp pipeline
 *    @saved_params: isp saved params
 *
 *  Sets saved params to pipeline.
 *
 *
 *  Return void
 **/
static void isp_pipeline_set_saved_params(isp_pipeline_t *pipeline,
  isp_saved_params_t* saved_params, uint32_t in_params_size)
{
  int i = 0;
  pipeline->pix_params.cfg_and_3a_params.cfg.effects.spl_effect =
    saved_params->effect;
  pipeline->pix_params.cfg_and_3a_params.cfg.vhdr_enable =
    saved_params->vhdr_enable;
  pipeline->pix_params.cfg_and_3a_params.cfg.effects.contrast =
    saved_params->contrast;
  pipeline->pix_params.cfg_and_3a_params.cfg.effects.saturation =
    (float)saved_params->saturation / 10.0f;
  pipeline->pix_params.cfg_and_3a_params.cfg.sce_factor =
    saved_params->sce_factor;
  pipeline->pix_params.cfg_and_3a_params.cfg.sharpness =
    saved_params->sharpness;
  pipeline->pix_params.cfg_and_3a_params.trigger_input.stats_update.awb_update =
    saved_params->awb_stats_update;
  pipeline->pix_params.cfg_and_3a_params.trigger_input.stats_update.aec_update =
    saved_params->aec_stats_update;
  pipeline->pix_params.cfg_and_3a_params.cfg.flash_params =
    saved_params->flash_params;
  pipeline->pix_params.cfg_and_3a_params.trigger_input.digital_gain =
    saved_params->dig_gain;
  /* save the ihist data in pipeline*/
  for (i = 0; i<256; i++) {
    pipeline->pix_params.cfg_and_3a_params.trigger_input.stats_update.
      ihist_params.isp_ihist_data[i] = saved_params->ihist_stats.histogram[i];
  }
  ISP_DBG(ISP_MOD_COM,"%s: spl_effect = %d, contrast = %d, satuation = %f, sce_factor = %d\n",
    __func__,
    pipeline->pix_params.cfg_and_3a_params.cfg.effects.spl_effect,
    pipeline->pix_params.cfg_and_3a_params.cfg.effects.contrast,
    pipeline->pix_params.cfg_and_3a_params.cfg.effects.saturation,
    pipeline->pix_params.cfg_and_3a_params.cfg.sce_factor);
}

/** isp_pipeline_save_aec_params
 *    @pipeline: isp pipeline
 *    @saved_params: isp saved params
 *
 *  Set saved aec params & update to pipeline.
 *
 *
 *  Return void
 **/
static void isp_pipeline_save_aec_params(isp_pipeline_t *pipeline,
  stats_update_t* saved_params, uint32_t in_params_size)
{
  pipeline->pix_params.cfg_and_3a_params.trigger_input.stats_update.aec_update =
    ((stats_update_t *) saved_params)->aec_update;
}

/** isp_pipeline_save_asd_params
 *    @pipeline: isp pipeline
 *    @saved_params: isp saved params
 *
 *  Set saved asd params to pipeline.
 *
 *
 *  Return void
 **/
static void isp_pipeline_save_asd_params(isp_pipeline_t *pipeline,
  stats_update_t* saved_params, uint32_t in_params_size)
{
  pipeline->pix_params.cfg_and_3a_params.trigger_input.stats_update.asd_update =
    ((stats_update_t *) saved_params)->asd_update;
}

/** isp_pipeline_set_sce
 *    @isp_hw: isp_hw root
 *    @sce_factor: isp saved params
 *    @in_params_size parameter size
 *
 *  Set sce factor to pipeline.
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_sce(isp_pipeline_t *pipeline, int32_t *sce_factor,
  uint32_t in_params_size)
{
  int rc = 0;
  uint8_t is_bayer_input = isp_pipeline_util_is_bayer_fmt(pipeline);

  if(!is_bayer_input){
    return 0;
  }
  if (pipeline->pix_params.cur_module_mask & (1 << ISP_MOD_SCE)) {
    rc = pipeline->mod_ops[ISP_MOD_SCE]->set_params(
      pipeline->mod_ops[ISP_MOD_SCE]->ctrl,
      ISP_HW_MOD_SET_SCE_FACTOR,
      (void *)sce_factor,
      sizeof(*sce_factor));
    if(rc < 0)
      goto END;

    rc = pipeline->mod_ops[ISP_MOD_SCE]->set_params(
      pipeline->mod_ops[ISP_MOD_SCE]->ctrl,
      ISP_HW_MOD_SET_TRIGGER_UPDATE,
      (void *)&pipeline->pix_params.cfg_and_3a_params,
      sizeof(pipeline->pix_params.cfg_and_3a_params));
  }
END:
  return rc;
}

/** isp_pipeline_set_vhdr
 *    @isp_hw: isp_hw root
 *    @sce_factor: isp saved params
 *    @in_params_size parameter size
 *
 *  Set vhdr enable to pipeline.
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_vhdr(isp_pipeline_t *pipeline,
  uint32_t *vhdr_enable, uint32_t in_params_size)
{
  pipeline->pix_params.cfg_and_3a_params.cfg.vhdr_enable = *vhdr_enable;
  return 0;
}

/** isp_pipeline_set_tintless
 *
 *    @pipeline:
 *    @tintless_data:
 *    @in_params_size:
 *
 *  Set tintless data info to pipeline.
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_tintless(isp_pipeline_t *pipeline,
  isp_tintless_data_t *tintless_data, uint32_t in_params_size)
{
  pipeline->pix_params.cfg_and_3a_params.cfg.tintless_data = tintless_data;
  return 0;
}

static int isp_pipeline_map_eatune_mod_to_isp(
        vfemodule_t vfe_mod, isp_hw_module_id_t *isp_mod)
{
   switch(vfe_mod) {
   case VFE_MODULE_LINEARIZATION: {
     *isp_mod = ISP_MOD_LINEARIZATION;
   }
     break;

   case VFE_MODULE_COLORCORRECTION: {
     *isp_mod = ISP_MOD_COLOR_CORRECT;
   }
     break;

   case VFE_MODULE_COLORCONVERSION: {
     *isp_mod = ISP_MOD_COLOR_CONV;
   }
     break;

   case VFE_MODULE_GAMMA: {
     *isp_mod = ISP_MOD_GAMMA;
   }
     break;

   case VFE_MODULE_BLACKLEVEL: {
     *isp_mod = ISP_MOD_LINEARIZATION;
   }
     break;

   case VFE_MODULE_ASF5X5: {
     *isp_mod = ISP_MOD_ASF;
   }
     break;

   case VFE_MODULE_LUMAADAPTATION: {
     *isp_mod = ISP_MOD_LA;
   }
     break;

   case VFE_MODULE_ROLLOFF: {
     *isp_mod = ISP_MOD_ROLLOFF;
   }
     break;

   case VFE_MODULE_BPC: {
     *isp_mod = ISP_MOD_BPC;
   }
     break;

   case VFE_MODULE_BCC: {
     *isp_mod = ISP_MOD_BCC;
   }
     break;

   case VFE_MODULE_CHROMASUPPRESSION: {
     *isp_mod = ISP_MOD_CHROMA_SUPPRESS;
   }
     break;

   case VFE_MODULE_MCE: {
     *isp_mod = ISP_MOD_MCE;
   }
     break;

   case VFE_MODULE_SCE: {
     *isp_mod = ISP_MOD_SCE;
   }
     break;

   case VFE_MODULE_DEMOSAIC: {
     *isp_mod = ISP_MOD_DEMOSAIC;
   }
     break;

   case VFE_MODULE_DEMUX: {
     *isp_mod = ISP_MOD_DEMUX;
   }
     break;

   case VFE_MODULE_CLFILTER: {
     *isp_mod = ISP_MOD_CLF;
   }
     break;

  case VFE_MODULE_WB: {
     *isp_mod = ISP_MOD_WB;
   }
     break;

   case VFE_MODULE_ABF: {
     *isp_mod = ISP_MOD_ABF;
   }
     break;

   case VFE_MODULE_ALL:
   default: {
     return -1;
   }
  }
   return 0;
}

static int isp_pipeline_set_mod_trigger(isp_pipeline_t *pipeline,
  isp_mod_trigger_t *tgr_enable, uint32_t in_params_size)
{
   isp_hw_module_id_t isp_mod;
   int i, id, num, rc = 0;
   isp_mod_set_enable_t enable;
   vfemodule_t vfe_eztune_mod = tgr_enable->module;
   rc = isp_pipeline_map_eatune_mod_to_isp(vfe_eztune_mod,
         &isp_mod);
   if(rc < 0) {
      CDBG_ERROR("%s: Unable to map eztune module to ISP\n", __func__);
      return rc;
   }
   if (GET_ISP_MAIN_VERSION(pipeline->isp_version) == ISP_VERSION_32) {
     if (isp_mod == ISP_MOD_COLOR_XFORM) {
       CDBG_ERROR("%s: This module is not supported in VFE32 module no = %d\n",
       __func__,isp_mod);
     return 0;
     }
   } else if(GET_ISP_MAIN_VERSION(pipeline->isp_version) == ISP_VERSION_40) {
     if ((isp_mod == ISP_MOD_ASF) || (isp_mod == ISP_MOD_CHROMA_SS) ||
        (isp_mod == ISP_MOD_CLF)) {
        CDBG_ERROR("%s: This module is not supported in VFE40 module no = %d\n",
        __func__, isp_mod);
        return 0;
     }
   }
   enable.enable = tgr_enable->enable;
   rc = pipeline->mod_ops[isp_mod]->set_params(pipeline->mod_ops[isp_mod]->ctrl,
         ISP_HW_MOD_SET_TRIGGER_ENABLE, &enable, sizeof(enable));
       if (rc < 0 && rc != -EAGAIN) {
         CDBG_ERROR("%s: error, trigger_enable, module id = %d, rc = %d\n",
           __func__, isp_mod, rc);
         return rc;
       }
   return rc;
}

static int isp_pipeline_set_mod_enable(isp_pipeline_t *pipeline,
  isp_mod_trigger_t *tgr_enable, uint32_t in_params_size)
{
   isp_hw_module_id_t isp_mod;
   int  rc = 0;
   vfemodule_t vfe_eztune_mod = tgr_enable->module;
   rc = isp_pipeline_map_eatune_mod_to_isp(vfe_eztune_mod,
         &isp_mod);
   if(rc < 0) {
      CDBG_ERROR("%s: Unable to map eztune module to ISP\n", __func__);
      return rc;
   }
   if (tgr_enable->enable) {
     pipeline->pix_params.user_module_mask |= (1 << isp_mod);
   }
   else {
     pipeline->pix_params.user_module_mask &= ~(1 << isp_mod);
   }

   //ensure usermodule mask do not set anything not supported in the config
   pipeline->pix_params.user_module_mask &= pipeline->pix_params.max_module_mask;

   return rc;
}

/** isp_pipeline_set_flash_params:
 *
 *  @pipeline: isp pipeline handle
 *  @flash_params: handle to isp_flash_params_t
 *  @in_params_size: flash_params size
 *
 *  Save flash params in pipeline
 *
 *  Return void
 **/
static void isp_pipeline_set_flash_params(isp_pipeline_t *pipeline,
  isp_flash_params_t *flash_params, uint32_t in_params_size)
{
  if (!pipeline || !flash_params) {
    CDBG_ERROR("%s:%d failed: %p %p\n", __func__, __LINE__, pipeline,
      flash_params);
    return;
  }
  pipeline->pix_params.cfg_and_3a_params.cfg.flash_params = *flash_params;
}

/** isp_pipeline_set_bracketing_data:
 *    @pipeline: Pointer to pipeline
 *    @bracketing_data: bracketing data
 *    @in_params_size:
 *
 *  Set the bracketing data to pipeline
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_bracketing_data(isp_pipeline_t *pipeline,
  mct_bracket_ctrl_t *bracketing_data, uint32_t in_params_size)
{
  int i, num, rc = 0;
  uint16_t *module_ids = NULL;
  isp_pix_params_t *params = &pipeline->pix_params;

  uint8_t is_bayer_input = isp_pipeline_util_is_bayer_fmt(pipeline);

  if(!is_bayer_input)
    return 0;

  pipeline->pix_params.cfg_and_3a_params.cfg.bracketing_data = *bracketing_data;

  module_ids = pipeline->dep.mod_trigger_update_order_bayer;
  num = pipeline->dep.num_mod_trigger_update_order_bayer;

  for (i = 0; i < num; i++) {
    if (((1 << module_ids[i]) & params->cur_module_mask) &&
        pipeline->mod_ops[module_ids[i]] && (module_ids[i]!= ISP_MOD_STATS)) {

      rc = pipeline->mod_ops[module_ids[i]]->set_params(
        pipeline->mod_ops[module_ids[i]]->ctrl,
        ISP_HW_MOD_SET_BRACKETING_DATA,
        (void *)&pipeline->pix_params.cfg_and_3a_params.cfg,
        sizeof(isp_hw_pix_setting_params_t));

      rc = pipeline->mod_ops[module_ids[i]]->set_params(
        pipeline->mod_ops[module_ids[i]]->ctrl,
        ISP_HW_MOD_SET_TRIGGER_UPDATE,
        (void *)&pipeline->pix_params.cfg_and_3a_params,
        sizeof(isp_pix_trigger_update_input_t));

      if (rc < 0) {
        CDBG_ERROR("%s: module %d config failed\n", __func__, i);
        return rc;
      }
    }
  }
  return 0;
}

/** isp_pipeline_set_bestshot:
 *    @pipeline: Pointer to pipeline
 *    @bestshot: best shot mode
 *    @in_params_size:
 *
 *  Set the best shot mode to pipeline
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_bestshot(isp_pipeline_t *pipeline,
  int32_t *bestshot, uint32_t in_params_size)
{
  int i, num, rc = 0;
  uint16_t *module_ids = NULL;
  isp_pix_params_t *params = &pipeline->pix_params;
  isp_pix_trigger_update_input_t *trigger_update_params =
    &params->cfg_and_3a_params;
  isp_stats_udpate_t *stats_update =
    &trigger_update_params->trigger_input.stats_update;

  uint8_t is_bayer_input = isp_pipeline_util_is_bayer_fmt(pipeline);

  if(!is_bayer_input)
    return 0;

  if ((pipeline->pix_params.cfg_and_3a_params.cfg.bestshot_mode !=
      CAM_SCENE_MODE_OFF) &&
      (pipeline->pix_params.cfg_and_3a_params.cfg.effects.spl_effect !=
          CAM_EFFECT_MODE_OFF)) {
    CDBG_ERROR("%s: Error: Turn off spl_effect before set Bestshot\n",
      __func__);
    CDBG_ERROR("%s: spl_effect/best_shot: %d/%d\n",
        __func__,
        pipeline->pix_params.cfg_and_3a_params.cfg.effects.spl_effect,
        *bestshot);
    return -1;
  }

  pipeline->pix_params.cfg_and_3a_params.cfg.bestshot_mode = *bestshot;

  module_ids = pipeline->dep.mod_trigger_update_order_bayer;
  num = pipeline->dep.num_mod_trigger_update_order_bayer;

  for (i = 0; i < num; i++) {
    if (((1 << module_ids[i]) & params->cur_module_mask) &&
        pipeline->mod_ops[module_ids[i]] && (module_ids[i]!= ISP_MOD_STATS)) {

      rc = pipeline->mod_ops[module_ids[i]]->set_params(
        pipeline->mod_ops[module_ids[i]]->ctrl,
        ISP_HW_MOD_SET_BESTSHOT,
        (void *)&pipeline->pix_params.cfg_and_3a_params.cfg,
        sizeof(isp_hw_pix_setting_params_t));

      rc = pipeline->mod_ops[module_ids[i]]->set_params(
        pipeline->mod_ops[module_ids[i]]->ctrl,
        ISP_HW_MOD_SET_TRIGGER_UPDATE,
        (void *)&pipeline->pix_params.cfg_and_3a_params,
        sizeof(isp_pix_trigger_update_input_t));

      if (rc < 0) {
        CDBG_ERROR("%s: module %d config failed\n", __func__, i);
        return rc;
      }
    }
  }
  return 0;
}

/** isp_pipeline_set_spl_effect
 *    @pipeline:      isp pipeline
 *    @contrast:      contrast
 *    @in_params_size: size of parameters
 *
 *  Sets special effect to pipeline.
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_spl_effect(isp_pipeline_t *pipeline,
 int32_t *effect, uint32_t in_params_size)
{
  int rc = 0;

  pipeline->pix_params.cfg_and_3a_params.cfg.effects.spl_effect = *effect;
  pipeline->pix_params.cfg_and_3a_params.cfg.effects.effect_type_mask |=
    (1 <<ISP_EFFECT_SPECIAL);

  if ((pipeline->pix_params.cfg_and_3a_params.cfg.bestshot_mode !=
      CAM_SCENE_MODE_OFF) &&
      (*effect != CAM_EFFECT_MODE_OFF)) {
    CDBG_ERROR("%s: Error: Turn off Bestshot before set spl_effects\n",
      __func__);
    CDBG_ERROR("%s: spl_effect/best_shot: %d/%d\n",
        __func__,
        *effect,
        pipeline->pix_params.cfg_and_3a_params.cfg.bestshot_mode);
    return -1;
  }

  pipeline->pix_params.cfg_and_3a_params.cfg.effects.spl_effect = *effect;
  pipeline->pix_params.cfg_and_3a_params.cfg.effects.effect_type_mask |=
    (1 <<ISP_EFFECT_SPECIAL);

  ISP_DBG(ISP_MOD_COM,"%s:set contrast = %d\n", __func__,
    pipeline->pix_params.cfg_and_3a_params.cfg.effects.contrast);

  rc = isp_pipeline_set_effect(pipeline);
  return rc;
}

/** isp_pipeline_set_contrast
 *    @pipeline:      isp pipeline
 *    @contrast:      contrast
 *    @in_params_size size of parameters
 *
 *  Sets contrast to pipeline.
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_contrast(isp_pipeline_t *pipeline,
  int32_t *contrast, uint32_t in_params_size)
{
  int rc = 0;

  pipeline->pix_params.cfg_and_3a_params.cfg.effects.contrast = *contrast;
  pipeline->pix_params.cfg_and_3a_params.cfg.effects.effect_type_mask |=
    (1 <<ISP_EFFECT_CONTRAST);

  ISP_DBG(ISP_MOD_COM,"%s:set contrast = %d\n", __func__,
    pipeline->pix_params.cfg_and_3a_params.cfg.effects.contrast);

  rc = isp_pipeline_set_effect(pipeline);
  return rc;
}

/** isp_pipeline_set_sharpness
 *    @pipeline:      isp pipeline
 *    @sharpness_factor:   sharpness
 *    @in_params_size size of parameters
 *
 *  Sets sharpness to pipeline.
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_sharpness(isp_pipeline_t *pipeline,
                             int32_t *sharpness_factor,
                             uint32_t in_params_size)
{
  int rc = 0;
  uint8_t is_bayer_input = isp_pipeline_util_is_bayer_fmt(pipeline);

  if (!is_bayer_input) {
    return rc;
  }
  if (pipeline->pix_params.cur_module_mask & (1 << ISP_MOD_ASF)) {
    rc = pipeline->mod_ops[ISP_MOD_ASF]->set_params(
      pipeline->mod_ops[ISP_MOD_ASF]->ctrl,
      ISP_HW_MOD_SET_SHARPNESS_FACTOR,
      (void *)sharpness_factor,
      sizeof(*sharpness_factor));
    if(rc < 0)
      goto END;

    rc = pipeline->mod_ops[ISP_MOD_ASF]->set_params(
      pipeline->mod_ops[ISP_MOD_ASF]->ctrl,
      ISP_HW_MOD_SET_TRIGGER_UPDATE,
      (void *)&pipeline->pix_params.cfg_and_3a_params,
      sizeof(pipeline->pix_params.cfg_and_3a_params));
  }
END:
  return rc;
}

/** isp_pipeline_set_saturation
 *    @pipeline:      isp pipeline
 *    @saturation:    saturation
 *    @in_params_size size of parameters
 *
 *  Sets saturation to pipeline.
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_saturation(isp_pipeline_t *pipeline,
  isp_saturation_setting_t *saturation, uint32_t in_params_size)
{
  int rc = 0;
  float float_saturation;
  int32_t saturation_value;

  saturation_value = saturation->saturation;
  float_saturation = ((float)(saturation_value)) / 10.0f;

  pipeline->pix_params.cfg_and_3a_params.cfg.effects.saturation =
    float_saturation;
  pipeline->pix_params.cfg_and_3a_params.trigger_input.is_init_setting =
    saturation->is_init_setting;
  pipeline->pix_params.cfg_and_3a_params.cfg.effects.effect_type_mask |=
    (1 << ISP_EFFECT_SATURATION);

  rc = isp_pipeline_set_effect(pipeline);
  return rc;
}

/** isp_pipeline_ihist_la_trigger_update:
 *    @pipeline: Pointer to Pipeline
 *    @q3a_ihist_stats: ihist stats
 *    @in_params_size
 *
 *  New IHIST stats, Update LA module
 *
 *
 *  Return 0 on Success, negative on ERROR
**/
static int isp_pipeline_ihist_la_trigger_update(isp_pipeline_t *pipeline,
  q3a_ihist_stats_t *q3a_ihist_stats, uint32_t in_param_size)
{
  int rc = 0;
  int i;
  isp_pix_trigger_update_input_t *trigger =
    &pipeline->pix_params.cfg_and_3a_params;
  isp_ihist_params_t *isp_ihist =
    &trigger->trigger_input.stats_update.ihist_params;

  if (q3a_ihist_stats == NULL) {
    CDBG_ERROR("%s:ihist stats NULL , q3a_ihist_stats = %p\n",
      __func__, q3a_ihist_stats);
    return 0;
  }
  /* save the ihist data in pipeline*/
  for (i = 0; i<256; i++) {
    isp_ihist->isp_ihist_data[i] = q3a_ihist_stats->histogram[i];
  }

  if (((1 << ISP_MOD_LA) & pipeline->pix_params.cur_module_mask) &&
    pipeline->mod_ops[ISP_MOD_LA]) {
    /* enable modules*/
    rc = pipeline->mod_ops[ISP_MOD_LA]->set_params(
      pipeline->mod_ops[ISP_MOD_LA]->ctrl, ISP_HW_MOD_SET_LA_HIST_UPDATE,
      (void *)trigger, sizeof(isp_pix_trigger_update_input_t));

    if (rc < 0) {
      CDBG_ERROR("%s: trigger update Luma Adaptation failed, rc = %d\n",
        __func__, rc);
      return rc;
    }
  }

  return rc;
}

/** isp_pipeline_set_uv_subsample:
 *    @pipeline: Pointer to pipeline
 *    @enable: enable flag
 *
 *  Set UV Subsampling Enable on pipeline
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_uv_subsample(isp_pipeline_t *pipeline,
  uint32_t *enable)
{
  return pipeline->dep.util_set_uv_subsample(
    (void *)pipeline, (*enable) ? TRUE : FALSE);
}

/** isp_pipeline_set_crop_factor:
 *    @pipeline: Pointer to pipeline
 *    @crop_factor: hw crop factor
 *
 *  Set crop factor to pipeline
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_crop_factor(isp_pipeline_t *pipeline,
  isp_hw_set_crop_factor_t *crop_factor)
{
  int rc = 0;

  pipeline->pix_params.cfg_and_3a_params.cfg.crop_factor =
    crop_factor->crop_factor;
  if (pipeline->num_active_streams == 0) {
    memset(&crop_factor->hw_zoom_parm, 0, sizeof(crop_factor->hw_zoom_parm));
    return 0; /* pipeline is idle so just save the value */
  }

  rc = pipeline->dep.do_zoom((void *)pipeline, crop_factor);
  return rc;
}

/** isp_pipeline_set_params:
 *    @ctrl: Pointer to pipeline
 *    @param_id: event id indicating what value is set
 *    @in_params: input event params
 *    @in_param_size: size of input params
 *
 *  Set value for parameter given by param id and pass input params
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
int isp_pipeline_set_params (void *ctrl, uint32_t params_id,
  void *in_params, uint32_t in_params_size)
{
  isp_pipeline_t *pipeline = ctrl;
  int rc = 0;

  switch (params_id) {
  case ISP_PIX_SET_RECORDING_HINT: {
    pipeline->pix_params.cfg_and_3a_params.cfg.recording_hint =
      *((int *)in_params);
  }
    break;

  case ISP_PIX_SET_PARAM_CROP_FACTOR: {
    rc = isp_pipeline_set_crop_factor(pipeline,
           (isp_hw_set_crop_factor_t *)in_params);
  }
    break;

  case ISP_PIX_SET_UV_SUBSAMPLE: {
    /*enable extra chroma subsample */
    rc = isp_pipeline_set_uv_subsample(pipeline, (uint32_t *)in_params);
  }
    break;

  case ISP_PIX_SET_ION_FD: {
    pipeline->pix_params.cfg_and_3a_params.cfg.ion_fd =
      *((int *)in_params);
  }
    break;

  case ISP_PIX_SET_CAMIF_CFG: {
    rc = isp_pipeline_set_camif_cfg(pipeline,
      (isp_pix_camif_cfg_t *)in_params, in_params_size);
  }
    break;

  case ISP_PIX_SET_AF_ROLLOFF_PARAMS: {
    af_rolloff_info_t *af_rolloff_info = (af_rolloff_info_t *) in_params;
    af_rolloff_info_t *af_rolloff =
      &pipeline->pix_params.cfg_and_3a_params.cfg.af_rolloff_info;
    *af_rolloff = *af_rolloff_info;
  }
    break;

  case ISP_PIX_SET_CHROMATIX: {
    modulesChromatix_t *chromatix = in_params;
    pipeline->pix_params.cfg_and_3a_params.cfg.chromatix_ptrs = *chromatix;
  }
    break;

  case ISP_PIX_SET_RELOAD_CHROMATIX: {
    modulesChromatix_t *chromatix = in_params;
    pipeline->pix_params.cfg_and_3a_params.cfg.chromatix_ptrs = *chromatix;
    uint8_t is_bayer_input = isp_pipeline_util_is_bayer_fmt(pipeline);
    /* config pix moduled*/
    rc = isp_pix_pipeline_module_config(pipeline, is_bayer_input);
    if (rc < 0) {
      CDBG_ERROR("%s:isp_pix_pipeline_module_config error = %d\n", __func__, rc);
    }
  }
    break;

  case ISP_PIX_SET_BRACKETING_DATA: {
    rc = isp_pipeline_set_bracketing_data(pipeline, in_params, in_params_size);
  }
    break;

  case ISP_PIX_SET_BESTSHOT: {
    /* int maps to camera_bestshot_mode_type */
    int *bestshot = (int *)in_params;
    pipeline->pix_params.cfg_and_3a_params.cfg.bestshot_mode =
      (cam_scene_mode_type)(*bestshot);
    rc = isp_pipeline_set_bestshot(pipeline, (int32_t *)in_params,
           in_params_size);
  }
    break;

  case ISP_PIX_SET_CONTRAST: {
    /* isp_effects_params_t */
    rc = isp_pipeline_set_contrast(pipeline, (int32_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_PIX_SET_SATURATION: {
    /* int32_t */
    rc = isp_pipeline_set_saturation(pipeline,
      (isp_saturation_setting_t *)in_params, in_params_size);
  }
    break;

  case ISP_PIX_SET_EFFECT: {
    /* isp_effects_params_t */
    rc = isp_pipeline_set_spl_effect(pipeline, (int32_t *)in_params,
           in_params_size);
  }
    break;

  case ISP_PIX_SET_STATS_CFG_UPDATE: {
    /* isp_set_af_params_t */
    rc = isp_pipeline_stats_config_update(pipeline, (stats_config_t *)in_params,
           in_params_size);
  }
    break;

  case ISP_PIX_SET_STATS_CFG:{
    isp_stats_config_t *stats_cfg = (isp_stats_config_t *)in_params;
    pipeline->pix_params.cfg_and_3a_params.cfg.stats_cfg = *stats_cfg;
  }
    break;

  case ISP_PIX_SAVE_AEC_PARAMS: {
    isp_pipeline_save_aec_params(pipeline, (stats_update_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_PIX_SAVE_ASD_PARAMS: {
    isp_pipeline_save_asd_params(pipeline, (stats_update_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_PIX_SET_SAVED_PARAMS: {
    isp_pipeline_set_saved_params(pipeline, (isp_saved_params_t *)in_params,
      in_params_size);
  }
    break;

  case ISP_PIX_SET_STREAM_CFG: {
    rc = isp_pipeline_set_stream_config(pipeline,
           (isp_hwif_output_cfg_t *)in_params, in_params_size);
  }
    break;

  case ISP_PIX_SET_STREAM_UNCFG: {
    rc = isp_pipeline_set_stream_unconfig(pipeline,
           (isp_hw_stream_uncfg_t *)in_params, in_params_size);
  }
    break;

  case ISP_PIX_SET_SENSOR_LENS_POSITION_TRIGGER_UPDATE: {
    rc = isp_pipeline_sensor_lens_position_trigger_update(pipeline,
           (lens_position_update_isp_t *)in_params, in_params_size);
  }
    break;

  case ISP_PIX_SET_AWB_TRIGGER_UPDATE: {
     /* update AWB*/
    rc = isp_pipeline_awb_trigger_update(pipeline, (stats_update_t *)in_params,
           in_params_size);
  }
     break;

  case ISP_PIX_SET_AEC_TRIGGER_UPDATE: {
     /* update AWB*/
    rc = isp_pipeline_aec_trigger_update(pipeline, (float *)in_params,
           in_params_size);
  }
     break;

  case ISP_PIX_SET_IHIST_LA_TRIGGER_UPDATE:{
    /*trigger update LA by ihist stats*/
    rc = isp_pipeline_ihist_la_trigger_update(pipeline,
           (q3a_ihist_stats_t *)in_params, in_params_size);
  }
    break;

  case ISP_PIX_SET_WB_MODE: {
    /* int maps to config3a_wb_type_t */
  }
    break;

  case ISP_PIX_SET_SHARPNESS: {
    /* isp_sharpness_info_t */
   rc = isp_pipeline_set_sharpness(pipeline, (int32_t *)in_params, in_params_size);
  }
    break;

  case ISP_PIX_SET_FLASH_MODE: {
    /* cam_flash_mode_t */
    rc = isp_pipeline_set_flash_mode(pipeline, (cam_flash_mode_t *)in_params,
           in_params_size);
  }
    break;

  case ISP_PIX_CAMIF_SYNC_TIMER_CFG: {
    /* camif_strobe_info_t */
  }
    break;

  case ISP_PIX_SET_SCE: {
    rc = isp_pipeline_set_sce(pipeline, (int32_t *)in_params, in_params_size);
  }
    break;

  case ISP_PIX_SET_VHDR: {
    rc = isp_pipeline_set_vhdr(pipeline, (uint32_t *)in_params, in_params_size);
  }
    break;

  case ISP_PIX_SET_TINTLESS: {
    rc = isp_pipeline_set_tintless(pipeline, (isp_tintless_data_t *)in_params, in_params_size);
  }
    break;

  case ISP_PIX_SET_MOD_TRIGGER: {
    rc = isp_pipeline_set_mod_trigger(pipeline, (isp_mod_trigger_t *)in_params,
           in_params_size);
  }
    break;

  case ISP_PIX_SET_MOD_ENABLE: {
    rc = isp_pipeline_set_mod_enable(pipeline, (isp_mod_trigger_t *)in_params,
           in_params_size);
  }
    break;

  case ISP_PIX_SET_FLASH_PARAMS: {
    isp_pipeline_set_flash_params(pipeline, (isp_flash_params_t *)in_params,
      in_params_size);
  }
    break;

  default:
    break;
  }

  return rc;
}

/** isp_pipeline_get_cs_rs_config:
 *    @pipeline: Pointer to pipeline
 *    @cs_rs_config: output cs_rs config
 *
 *  Get CS & RS Config from module
 *
 *
 *  Return 0 on SUccess, negative on ERROR
 **/
static int isp_pipeline_get_cs_rs_config(isp_pipeline_t *pipeline,
  isp_cs_rs_config_t *cs_rs_config)
{
  int rc = 0;
  if (pipeline->dep.max_mod_mask_continuous_bayer & (1 << ISP_MOD_STATS)) {
    /* stats configured */
    rc = pipeline->mod_ops[ISP_MOD_STATS]->get_params(
           pipeline->mod_ops[ISP_MOD_STATS]->ctrl,
           ISP_HW_MOD_GET_CS_RS_CONFIG,
           NULL, 0,
           cs_rs_config,
           sizeof(isp_cs_rs_config_t));
    return rc;
  } else {
    CDBG_ERROR("%s: no stats configured(mazx_mask = 0x%x. Error\n",
      __func__, pipeline->dep.max_mod_mask_continuous_bayer);
    return -1;
  }
}

/** isp_pipeline_get_roi_map:
 *    @pipeline: Pointer to pipeline
 *    @out_params: output roi map
 *    @out_params_size:
 *
 *  Get ROI Map
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_get_roi_map(isp_pipeline_t *pipeline,
  void *out_params, uint32_t out_params_size)
{
  int rc = 0;
  isp_hw_zoom_param_entry_t *zoom_entrys =
    (isp_hw_zoom_param_entry_t *)out_params;

  rc = pipeline->dep.get_roi_map((void *)pipeline, zoom_entrys);

  if (pipeline->mod_ops[ISP_MOD_COLOR_XFORM] == NULL) {
     CDBG("%s: No ColorXform. Nothing to be done\n", __func__);
   } else {
  /* After updating zoom, update colorxform s0, s1 & s2 values */
    rc = pipeline->mod_ops[ISP_MOD_COLOR_XFORM]->set_params(
           pipeline->mod_ops[ISP_MOD_COLOR_XFORM]->ctrl,
           ISP_HW_MOD_SET_TRIGGER_UPDATE,
           &pipeline->pix_params.cfg_and_3a_params.cfg,
           sizeof(pipeline->pix_params.cfg_and_3a_params.cfg));
    if (rc < 0) {
      CDBG_ERROR("%s: ColorXform trigger update error, rc = %d\n",
        __func__, rc);
    }
  }
  return rc;
}

/** isp_pipeline_get_rolloff_grid_info:
 *    @pipeline: pointer to Pipeline
 *    @out_params: output rolloff grid info
 *    @out_params_size:
 *
 *  Get Rolloff Grid info from rolloff module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_get_rolloff_grid_info(isp_pipeline_t *pipeline,
  void *out_params, uint32_t out_params_size)
{
  int rc = 0;

  if (out_params_size != sizeof(uint32_t)) {
    CDBG_ERROR("%s: size mismatch, recv = %d, need = %d\n",
      __func__, out_params_size, sizeof(uint32_t));
    return -1;
  }

  rc = pipeline->mod_ops[ISP_MOD_ROLLOFF]->get_params(
    pipeline->mod_ops[ISP_MOD_ROLLOFF]->ctrl,
    ISP_HW_MOD_GET_ROLLOFF_GRID_INFO, NULL, 0,
    out_params, sizeof(uint32_t));
  if (rc < 0) {
    CDBG_ERROR("%s: ISP_PIX_GET_ROLLOFF_GRID_INFO error, rc = %d\n",
      __func__, rc);
  }
  return rc;
}

/** isp_pipeline_get_rolloff_table:
 *    @pipeline: pointer to Pipeline
 *    @out_params: output rolloff table pointer
 *    @out_params_size:
 *
 *  Get Rolloff Table from rolloff module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_get_rolloff_table(isp_pipeline_t *pipeline,
  void *out_params, uint32_t out_params_size)
{
  int rc = 0;

  if (out_params_size != sizeof(mct_event_stats_isp_rolloff_t)) {
    CDBG_ERROR("%s: size mismatch, recv = %d, need = %d\n",
      __func__, out_params_size, sizeof(mct_event_stats_isp_rolloff_t));
    return -1;
  }

  rc = pipeline->mod_ops[ISP_MOD_ROLLOFF]->get_params(
    pipeline->mod_ops[ISP_MOD_ROLLOFF]->ctrl,
    ISP_HW_MOD_GET_ROLLOFF_TABLE, NULL, 0,
    out_params, sizeof(mct_event_stats_isp_rolloff_t));
  if (rc < 0) {
    CDBG_ERROR("%s: ISP_HW_MOD_GET_ROLLOFF_TABLE error, rc = %d\n",
      __func__, rc);
  }
  return rc;
}

/** isp_pipeline_get_cds_trigg_val:
 *    @pipeline: pointer to Pipeline
 *    @out_params: output la gamma table pointer
 *    @out_params_size:
 *
 *  Get trigger points for uv subsampling hystersis from
 *  chromatix.
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_get_cds_trigg_val(isp_pipeline_t *pipeline,
  void *in_params, uint32_t in_params_size, void *out_params,
  uint32_t out_params_size)
{
  int rc = 0;
  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  if (out_params_size != sizeof(isp_uv_subsample_t)) {
    CDBG_ERROR("%s: size mismatch, recv = %d, need = %d\n",
      __func__, out_params_size, sizeof(isp_uv_subsample_t));
    return -1;
  }
  pipeline->dep.util_get_param(in_params, in_params_size,
    ISP_PIPELINE_GET_CDS_TRIGGER_VAL, out_params, out_params_size, NULL);
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d\n", __func__, rc);
  return rc;
}

/** isp_pipeline_get_vfe_diag_info:
 *    @pipeline: pointer to Pipeline
 *    @out_params: output la gamma table pointer
 *    @out_params_size:
 *
 *  Get reg config from each module and update the struct
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_get_vfe_diag_info(isp_pipeline_t *pipeline,
   void *in_params, uint32_t in_params_size, void *out_params,
   uint32_t out_params_size)
{
  int rc = 0;
  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  if (out_params_size != sizeof(vfe_diagnostics_t)) {
    CDBG_ERROR("%s: size mismatch, recv = %d, need = %d\n",
      __func__, out_params_size, sizeof(vfe_diagnostics_t));
    return -1;
  }
  pipeline->dep.util_get_param(in_params, in_params_size,
    ISP_HW_MOD_GET_VFE_DIAG_INFO_USER, out_params, out_params_size, pipeline);
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d\n", __func__, rc);
  return rc;
}

/** isp_pipeline_get_current_rolloff_data:
 *    @pipeline: pointer to Pipeline
 *    @out_params: output la gamma table pointer
 *    @out_params_size:
 *
 *  Get reg config from each module and update the struct
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_get_current_rolloff_data(isp_pipeline_t *pipeline,
   void *in_params, uint32_t in_params_size, void *out_params,
   uint32_t out_params_size)
{
  int rc = 0;
  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  if (out_params_size != sizeof(tintless_mesh_rolloff_array_t)) {
    CDBG_ERROR("%s: size mismatch, recv = %d, need = %d\n",
      __func__, out_params_size, sizeof(tintless_mesh_rolloff_array_t));
    return -1;
  }
  pipeline->dep.util_get_param(in_params, in_params_size,
    ISP_HW_MOD_GET_TINTLESS_RO, out_params, out_params_size, pipeline);
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d\n", __func__, rc);
  return rc;
}

/** isp_pipeline_get_la_gamma_tbls:
 *    @pipeline: pointer to Pipeline
 *    @out_params: output la gamma table pointer
 *    @out_params_size:
 *
 *  Get la Gamma Table from module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_get_la_gamma_tbls(isp_pipeline_t *pipeline,
  void *out_params, uint32_t out_params_size)
{
  int rc = 0;

  if (out_params_size != sizeof(mct_isp_table_t)) {
    CDBG_ERROR("%s: size mismatch, recv = %d, need = %d\n",
      __func__, out_params_size, sizeof(mct_isp_table_t));
    return -1;
  }

  rc = pipeline->mod_ops[ISP_MOD_LA]->get_params(
    pipeline->mod_ops[ISP_MOD_LA]->ctrl, ISP_HW_MOD_GET_TBLS,
    NULL, 0,
    out_params, sizeof(mct_isp_table_t));
  if (rc < 0) {
    CDBG_ERROR("%s: ISP_PIX_GET_TBLS LA error, rc = %d\n",
      __func__, rc);
  }

  rc = pipeline->mod_ops[ISP_MOD_GAMMA]->get_params(
    pipeline->mod_ops[ISP_MOD_GAMMA]->ctrl, ISP_HW_MOD_GET_TBLS,
    NULL, 0,
    out_params, sizeof(mct_isp_table_t));
  if (rc < 0) {
    CDBG_ERROR("%s: ISP_PIX_GET_TBLS GAMMA error, rc = %d\n",
      __func__, rc);
  }

  return rc;
}

/** isp_pipeline_get_fov_crop:
 *    @pipeline: pointer to Pipeline
 *    @out_params: output fov crop info
 *    @out_params_size:
 *
 *  Get FOV Crop info from fov module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_get_fov_crop(isp_pipeline_t *pipeline,
  void *out_params, uint32_t out_params_size)
{
  int rc = 0;

  if (out_params_size != sizeof(isp_hw_zoom_param_t)) {
    CDBG_ERROR("%s: size mismatch, recv = %d, need = %d\n",
      __func__, out_params_size, sizeof(isp_hw_zoom_param_t));
    return -1;
  }

  rc = pipeline->mod_ops[ISP_MOD_FOV]->get_params(
    pipeline->mod_ops[ISP_MOD_FOV]->ctrl, ISP_HW_MOD_GET_FOV,
    (void *)&pipeline->pix_params.cfg_and_3a_params.cfg,
    sizeof(isp_hw_pix_setting_params_t), out_params,
    sizeof(isp_hw_zoom_param_t));
  if (rc < 0) {
    CDBG_ERROR("%s: ISP_PIX_GET_FOV error, rc = %d\n",
      __func__, rc);
  }
  return rc;
}

/** isp_pipeline_get_table_size:
 *    @param_id: event id indicating what param to get
 *    @out_params: output params
 *    @out_param_size: size of output params
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int isp_pipeline_get_table_size(isp_pipeline_t *pipeline,
  uint32_t param_id, void *out_params, uint32_t out_params_size)
{
  int rc = 0;
  uint32_t module_id;

  if (out_params_size != sizeof(isp_hw_read_info)) {
    CDBG_ERROR("%s: size mismatch, recv = %d, need = %d\n",
      __func__, out_params_size, sizeof(isp_hw_read_info));
    return -1;
  }

  switch (param_id) {
  case ISP_PIX_GET_ROLLOFF_TABLE_SIZE:
    module_id = ISP_MOD_ROLLOFF;
    break;
  case ISP_PIX_GET_GAMMA_TABLE_SIZE:
    module_id = ISP_MOD_GAMMA;
    break;
  case ISP_PIX_GET_LINEARIZATION_TABLE_SIZE:
    module_id = ISP_MOD_LINEARIZATION;
    break;
  case ISP_PIX_GET_LA_TABLE_SIZE:
    module_id = ISP_MOD_LA;
    break;
  default:
     break;
  }

  rc = pipeline->mod_ops[module_id]->get_params(
    pipeline->mod_ops[module_id]->ctrl, ISP_HW_MOD_GET_TABLE_SIZE,
    NULL, 0, out_params, sizeof(uint32_t));
  if (rc < 0) {
    CDBG_ERROR("%s: ISP_HW_MOD_GET_TABLE_SIZE error, module_id = %d, rc = %d\n",
      __func__, module_id, rc);
  }
  return rc;
}

/** isp_pipeline_get_params:
 *    @ctrl: Pointer to pipeline
 *    @param_id: event id indicating what param to get
 *    @in_params: input params
 *    @in_param_size: Size of Input Params
 *    @out_params: output params
 *    @out_param_size: size of output params
 *
 *  Get value of parameter given by param id
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
int isp_pipeline_get_params (void *ctrl, uint32_t params_id, void *in_params,
      uint32_t in_params_size, void *out_params, uint32_t out_params_size)
{
  isp_pipeline_t *pipeline = ctrl;
  int rc = 0;

  switch (params_id) {
  case ISP_PIX_GET_CS_RS_CONFIG:
    rc = isp_pipeline_get_cs_rs_config(pipeline,
      (isp_cs_rs_config_t *)out_params);
    break;
  case ISP_PIX_GET_ROLLOFF_GRID_INFO:
    rc = isp_pipeline_get_rolloff_grid_info(pipeline, out_params, out_params_size);
    break;
  case ISP_PIX_GET_LA_GAMMA_TBLS:
    rc = isp_pipeline_get_la_gamma_tbls(pipeline, out_params, out_params_size);
    break;
  case ISP_PIX_GET_FOV:
    rc = isp_pipeline_get_fov_crop(pipeline, out_params, out_params_size);
    break;

  case ISP_PIX_GET_ROI_MAP:
    rc = isp_pipeline_get_roi_map(pipeline, out_params, out_params_size);
    break;

  case ISP_PIX_GET_ROLLOFF_TABLE:
    rc = isp_pipeline_get_rolloff_table(pipeline, out_params, out_params_size);
    break;

  case ISP_PIX_GET_ROLLOFF_TABLE_SIZE:
  case ISP_PIX_GET_GAMMA_TABLE_SIZE:
  case ISP_PIX_GET_LINEARIZATION_TABLE_SIZE:
  case ISP_PIX_GET_LA_TABLE_SIZE:
    rc = isp_pipeline_get_table_size(pipeline, params_id, out_params, out_params_size);
    break;

  case ISP_PIX_GET_DMI_DUMP:
    rc = pipeline->dep.read_dmi_tbl(pipeline, in_params, out_params);
    break;

  case ISP_PIX_GET_CDS_TRIGGER_VAL: {
    rc = isp_pipeline_get_cds_trigg_val(pipeline, in_params, in_params_size,
           out_params, out_params_size);
  }
    break;

  case ISP_PIX_GET_VFE_DIAG_INFO: {
    rc = isp_pipeline_get_vfe_diag_info(pipeline, in_params, in_params_size,
           out_params, out_params_size);
  }
    break;
  case ISP_PIX_GET_TINTLESS_RO: {
    rc = isp_pipeline_get_current_rolloff_data(pipeline, in_params, in_params_size,
           out_params, out_params_size);
  }
    break;

  default:
    break;
  }

  return rc;
}

/** isp_pipeline_start_common:
 *    @pipeline: pointer to pipeline
 *
 *  Common functions for any pipeline start. HW update module and start modules.
 *  Config stats module and start stats
 *
 *
 *  Return 0 on Success, negative on ERROR
**/
static int isp_pipeline_start_common(isp_pipeline_t *pipeline)
{
  int rc = 0;
  uint32_t data_offset = 0;
  int fd = pipeline->fd;
  isp_pix_params_t *params = &pipeline->pix_params;
  isp_hw_pix_setting_params_t *cfg = &params->cfg_and_3a_params.cfg;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[5];
  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);

  rc = isp_pipeline_module_hw_update(pipeline,
         pipeline->pix_params.cfg_and_3a_params.cfg.camif_cfg.is_bayer_sensor);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_pipeline_module_hw_update error = %d\n", __func__, rc);
    goto error;
  }
  rc = pipeline->dep.module_start(pipeline);
  if (rc < 0) {
    CDBG_ERROR("%s: module_start error = %d\n", __func__, rc);
    goto error;
  }
  /* start stats */
  rc = isp_pipeline_util_stats_buf_cfg(pipeline, 1); /* config */
  if (rc < 0) {
    CDBG_ERROR("%s: isp_pipeline_util_stats_buf_cfg error = %d\n", __func__, rc);
    goto error;
  }
  rc = isp_pipeline_util_stats_start(pipeline, 1); /* start */
  if (rc < 0) {
    CDBG_ERROR("%s: isp_pipeline_util_stats_start error = %d\n", __func__, rc);
    goto error;
  }
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d", __func__, rc);

  return rc;
error:
  CDBG_ERROR("%s: Error X, rc = %d", __func__, rc);

  /* TODO: VFE module reset */
    return rc;
}

/** isp_hw_pix_stats_enable_and_config:
 *    @pix: pointer to pipeline
 *    @enb: enable flag
 *
 *  Enable and config stats modules
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_hw_pix_stats_enable_and_config(isp_pipeline_t *pix, uint8_t enb)
{
  int rc = 0;
  isp_mod_set_enable_t enable;
  uint32_t mod_mask = pix->pix_params.cur_module_mask;
  void *ctrl = pix->mod_ops[ISP_MOD_STATS]->ctrl;

  if (!(mod_mask & (1 << ISP_MOD_STATS)) || (pix->mod_ops[ISP_MOD_STATS] == NULL))
    return 0;

  /*enable stats*/
  enable.enable = enb;
  rc = pix->mod_ops[ISP_MOD_STATS]->set_params(ctrl,
    ISP_HW_MOD_SET_MOD_ENABLE, &enable, sizeof(isp_mod_set_enable_t));
  if (rc < 0) {
    CDBG_ERROR("%s: cannot enable STATS modules\n", __func__);
    return rc;
  }

  /*config stats*/
  rc = pix->mod_ops[ISP_MOD_STATS]->set_params(ctrl,
     ISP_HW_MOD_SET_MOD_CONFIG, &pix->pix_params.cfg_and_3a_params.cfg,
     sizeof(pix->pix_params.cfg_and_3a_params.cfg));
  if (rc < 0) {
    CDBG_ERROR("%s: cannot config stats module\n", __func__);
    return rc;
  }

  /*write the configuration into HW*/
  if (GET_ISP_MAIN_VERSION(pix->isp_version) !=
     ISP_VERSION_32) {
    if (enable.enable) {
      rc = pix->mod_ops[ISP_MOD_STATS]->action(ctrl,
        ISP_HW_MOD_ACTION_HW_UPDATE, NULL, 0);
      if (rc < 0) {
        CDBG_ERROR("%s: cannot write to STATS hw registers\n", __func__);
        return rc;
      }
    }
  }
  return rc;
}

/** isp_pix_generate_module_enable_mask:
 *    @pix: pointer to pipeline
 *    @is_bayer_input: flag if sensor input is bayer
 *    @first_start: flag for first start
 *
 *  Generate mask for all modules enabled
 *
 *
 *  Return void
 **/
static void isp_pix_generate_module_enable_mask(isp_pipeline_t *pix,
  int is_bayer_input, int *first_start)
{
  isp_pix_params_t *params = &pix->pix_params;
  isp_hw_pix_setting_params_t *cfg = &params->cfg_and_3a_params.cfg;

  *first_start = (pix->pix_params.max_module_mask == 0)? 1: 0;

  /* maximum allowed module mask */
  if (is_bayer_input) {
    if (cfg->streaming_mode == CAM_STREAMING_MODE_CONTINUOUS){
      pix->pix_params.max_module_mask = pix->dep.max_mod_mask_continuous_bayer;
      pix->pix_params.cur_stats_mask = pix->dep.max_supported_stats;
    } else {
      pix->pix_params.max_module_mask = pix->dep.max_mod_mask_burst_bayer;
      pix->pix_params.cur_stats_mask &= ~pix->dep.max_supported_stats;
    }
  } else {
    if (cfg->streaming_mode == CAM_STREAMING_MODE_CONTINUOUS)
      pix->pix_params.max_module_mask = pix->dep.max_mod_mask_continuous_yuv;
    else
      pix->pix_params.max_module_mask = pix->dep.max_mod_mask_burst_yuv;
  }
  /* if not first start, we carry on the previous module enable mask
     if the first start, we use the default max config*/
  if (*first_start) {
    pix->pix_params.cur_module_mask = pix->pix_params.max_module_mask;
    pix->pix_params.user_module_mask = pix->pix_params.max_module_mask;
  } else {
    /* This is stop and then start case, we use the user_module_mask
     * to do bit &  to find out the mask for the new pix module configuration.*/
    pix->pix_params.cur_module_mask =
      (pix->pix_params.user_module_mask &
       pix->pix_params.max_module_mask);
  }
  ISP_DBG(ISP_MOD_COM,"%s: is_bayer = %d, streaming_mode = %d, cur_mask = 0x%x, user_maks = 0x%x",
  __func__, is_bayer_input, cfg->streaming_mode,
  pix->pix_params.cur_module_mask,
  pix->pix_params.user_module_mask);
}

/** isp_pix_pipeline_module_enable:
 *    @pix: Pointer to pipeline
 *
 *  Enable all modules except stats
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pix_pipeline_module_enable(isp_pipeline_t *pix)
{
  int i, id, num, rc = 0;
  isp_mod_set_enable_t enable;

  /*enable pix modules one by one except for stats module*/
  for (i = 0; i < ISP_MOD_MAX_NUM; i++) {
    if (i == ISP_MOD_STATS) {
       /*enable and config stats module seperately*/
       continue;
    }

    if (pix->mod_ops[i]) {
      enable.enable = (pix->pix_params.cur_module_mask & (1 << i)) ? 1 : 0;
      /* enable modules*/
      enable.fast_aec_mode = pix->fast_aec_mode;
      rc = pix->mod_ops[i]->set_params(pix->mod_ops[i]->ctrl,
         ISP_HW_MOD_SET_MOD_ENABLE, &enable, sizeof(enable));
      if (rc < 0 && rc != -EAGAIN) {
        CDBG_ERROR("%s: error, cannot enable module id = %d, rc = %d\n",
          __func__, i, rc);
        return rc;
      }
    }
  }

  return rc;
}

/** isp_pix_pipeline_module_trigger_enable:
 *    @pix: pointer to pipeline
 *
 *  Enable trigger on all modules except stats
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pix_pipeline_module_trigger_enable(isp_pipeline_t *pix)
{
  int i, id, num, rc = 0;
  isp_mod_set_enable_t enable;

  /*enable pix modules one by one except for stats module*/
  for (i = 0; i < ISP_MOD_MAX_NUM; i++) {
     ISP_DBG(ISP_MOD_COM,"%s: i = %d\n", __func__, i);
    if (i == ISP_MOD_STATS) {
       /*enable and config stats module seperately*/
       continue;
    }
    if (pix->mod_ops[i]) {
      enable.enable = (pix->pix_params.cur_module_mask & (1 << i)) ? 1 : 0;
      /* trigrer enable to get a updated lighting status*/
      if (!enable.enable) {
        ISP_DBG(ISP_MOD_COM,"%s: mod %d not enabled, skip trigger enable\n",
          __func__, i);
        continue;
      }

      ISP_DBG(ISP_MOD_COM,"%s: module %d trigger_enable = %d\n", __func__, i, enable.enable);
      rc = pix->mod_ops[i]->set_params(pix->mod_ops[i]->ctrl,
        ISP_HW_MOD_SET_TRIGGER_ENABLE, &enable, sizeof(enable));
      if (rc < 0 && rc != -EAGAIN) {
        CDBG_ERROR("%s: error, trigger_enable, module id = %d, rc = %d\n",
          __func__, i, rc);
        return rc;
      }
    }
  }
  return rc;
}

/** isp_pix_pipeline_module_config:
 *    @pix: pointer to pipeline
 *    @is_bayer_input: Is input from sensor bayer
 *
 *  Configure the pipeline modules
 *
 *
 *  Return 0 on SUccess, negative on ERROR
 **/
static int isp_pix_pipeline_module_config(isp_pipeline_t *pix,
  int is_bayer_input)
{
  int i, id, num, rc = 0;
  uint16_t *module_ids = NULL;
  isp_pix_params_t *params = &pix->pix_params;
  isp_hw_pix_setting_params_t *cfg = &params->cfg_and_3a_params.cfg;

  if (is_bayer_input) {
    module_ids = pix->dep.mod_cfg_order_bayer;
    num = pix->dep.num_mod_cfg_order_bayer;
  } else {
    module_ids = pix->dep.mod_cfg_order_yuv;
    num = pix->dep.num_mod_cfg_order_yuv;
  }

  for (i = 0; i < num; i++) {
    id = module_ids[i];
    ISP_DBG(ISP_MOD_COM,"%s: id = %d, cur_mask = 0x%x, match = 0x%x",
      __func__, id, pix->pix_params.cur_module_mask,
    ((1 << id) & pix->pix_params.cur_module_mask));
    if (((1 << id) & pix->pix_params.cur_module_mask) &&
        pix->mod_ops[id]) {
      rc = pix->mod_ops[id]->set_params(
             pix->mod_ops[id]->ctrl,
             ISP_HW_MOD_SET_MOD_CONFIG,
             cfg,
             sizeof(isp_hw_pix_setting_params_t));
      if (rc < 0) {
        CDBG_ERROR("%s: module %d config failed\n", __func__, id);
        return rc;
      }
    }
  }

  return rc;
}

/** isp_pipeline_module_trigger_update:
 *    @pipeline: pointer to pipeline
 *    @is_bayer_input: is input from sensor bayer
 *
 *  trigger update all modules
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_module_trigger_update(isp_pipeline_t *pipeline,
  int is_bayer_input)
{
  int rc = 0;
  if (is_bayer_input)
    rc = isp_pipeline_util_trigger_update(pipeline);
  return rc;
}

/** isp_pipeline_module_hw_update:
 *    @pix: pointer to pipeline
 *    @is_bayer_input: is input from sensor bayer
 *
 *  Run HW Update on all modules at SOF
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_module_hw_update(isp_pipeline_t *pix,
  int is_bayer_input)
{
  int i, num, rc = 0;
  uint16_t *module_ids = NULL;
  isp_pix_params_t *params = &pix->pix_params;
  isp_hw_pix_setting_params_t *cfg = &params->cfg_and_3a_params.cfg;
  uint8_t hfr_mode = pix->pix_params.cfg_and_3a_params.cfg.camif_cfg.hfr_mode;
  isp_mod_set_enable_t enable;

  memset(&enable, 0, sizeof(isp_mod_set_enable_t));

  if(hfr_mode == CAM_HFR_MODE_120FPS) {
    if(pix->hfr_update_mod_mask == pix->hfr_update_batch1)
      pix->hfr_update_mod_mask = pix->hfr_update_batch2;
    else
      pix->hfr_update_mod_mask = pix->hfr_update_batch1;
  } else
    pix->hfr_update_mod_mask = 0xFFFFFFFF;

  if (is_bayer_input) {
    module_ids = pix->dep.mod_cfg_order_bayer;
    num = pix->dep.num_mod_cfg_order_bayer;
  } else {
    module_ids = pix->dep.mod_cfg_order_yuv;
    num = pix->dep.num_mod_cfg_order_yuv;
  }

  /* Enable/Disable the modules as the per the user control */
  if (pix->pix_params.cur_module_mask != pix->pix_params.user_module_mask) {
    pix->pix_params.cur_module_mask = (pix->pix_params.user_module_mask &
      pix->pix_params.max_module_mask);
    /* operation reconfig */
    if(pix->dep.module_reconf_module) {
      rc = pix->dep.module_reconf_module(pix);
       if (rc < 0) {
         CDBG_ERROR("%s: module reconfiguration error = %d\n", __func__, rc);
         return rc;
      }
     if (cfg->streaming_mode == CAM_STREAMING_MODE_CONTINUOUS)
       for (i = 0; i < num; i++) {
         if (pix->mod_ops[i]) {
           /* Based on the reconfig operation (enable/disable of individual
            * modules), notify and update private data flags of those modules. */
           enable.enable = ((1 << i) & pix->pix_params.cur_module_mask)? 1 : 0;
           if (pix->mod_ops[i] && pix->mod_ops[i]->set_params) {
             rc = pix->mod_ops[i]->set_params(pix->mod_ops[i]->ctrl,
               ISP_HW_MOD_SET_MOD_ENABLE, &enable, sizeof(enable));
           }
           /* MCE and CS modules needs to be handles seperately
            as they have separate controls to turn on/off */
           if (i == ISP_MOD_CHROMA_SUPPRESS || i == ISP_MOD_MCE) {
             if (!enable.enable) {
               rc = pix->mod_ops[i]->action(
                 pix->mod_ops[i]->ctrl, ISP_HW_MOD_ACTION_HW_UPDATE,
                 NULL, 0);
             }
           }
           /* ASF module needs to be reconfigured */
           if (i == ISP_MOD_ASF) {
             if (enable.enable) {
               if (pix->mod_ops[i]) {
                 rc = pix->mod_ops[i]->set_params(pix->mod_ops[i]->ctrl,
                   ISP_HW_MOD_SET_MOD_CONFIG, cfg,
                   sizeof(isp_hw_pix_setting_params_t));
                 if (rc < 0)
                   CDBG_ERROR("%s: module ASF config failed\n", __func__);
               }
             }
           }
         }
       }
    }
  }
  for (i = 0; i < num; i++) {
    if (((1 << module_ids[i]) & pix->pix_params.cur_module_mask) &&
      pix->mod_ops[module_ids[i]]) {
      if ((1 << module_ids[i]) & pix->hfr_update_mod_mask) {
        rc = pix->mod_ops[module_ids[i]]->action(
          pix->mod_ops[module_ids[i]]->ctrl, ISP_HW_MOD_ACTION_HW_UPDATE,
          NULL, 0);
        if (rc < 0) {
          CDBG_ERROR("%s: module %d hw register write failed\n", __func__, i);
          return rc;
        }
      }
    }
  }

  /* copy pix params which are active after the last trigger update to the
   * current one, which will be active for the next frame from now. */
  memcpy(&pix->cur_pix_params, &pix->pix_params, sizeof(pix->pix_params));

  return rc;
}

/** isp_pipeline_set_saved_sce_factor:
 *    @pipeline: pointer to pipeline
 *
 *  Set saved sce factor to pipeline
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_saved_sce_factor(isp_pipeline_t *pipeline)
{
  int rc = 0;
  isp_pix_params_t *params = &pipeline->pix_params;

  rc = isp_pipeline_set_sce(pipeline,
    &params->cfg_and_3a_params.cfg.sce_factor,
    sizeof(params->cfg_and_3a_params.cfg.sce_factor));
  if (rc < 0) {
    CDBG_ERROR("%s: isp_pipeline_set_sce error = %d, sce_factor = %d\n",
      __func__, rc, params->cfg_and_3a_params.cfg.sce_factor);
  }
  return rc;
}
/** isp_pipeline_set_saved_sharpness_factor:
 *    @pipeline: pointer to pipeline
 *
 *  Set saved sce factor to pipeline
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_set_saved_sharpness_factor(isp_pipeline_t *pipeline)
{
  int rc = 0;
  isp_pix_params_t *params = &pipeline->pix_params;

  rc = isp_pipeline_set_sharpness(pipeline,
    &params->cfg_and_3a_params.cfg.sharpness,
    sizeof(params->cfg_and_3a_params.cfg.sharpness));
  if (rc < 0) {
    CDBG_ERROR("%s: isp_pipeline_set_sce error = %d, sce_factor = %d\n",
      __func__, rc, params->cfg_and_3a_params.cfg.sce_factor);
  }
  return rc;
}
/** isp_pipeline_start:
 *    @pix: pointer to pipeline
 *
 *  Perform all tasks required to start pipeline
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_pipeline_start(isp_pipeline_t *pix)
{
  int rc = 0, i, is_initial_start = 0;
  isp_mod_set_enable_t enable;
  uint8_t is_bayer_input = isp_pipeline_util_is_bayer_fmt(pix);

  ISP_DBG(ISP_MOD_COM,"%s: E, is_bayer = %d", __func__, is_bayer_input);

  /*For HFR mode divide modules into batches. Batch 1,2 can be configured here.
    We can use only one variable and toggle to update in batches, but extra
    variables are added to fine tune batches and come to combination of batches.
    */
   if (pix->pix_params.cfg_and_3a_params.cfg.camif_cfg.hfr_mode ==
       CAM_HFR_MODE_120FPS) {
      pix->hfr_update_batch1 = (1 << ISP_MOD_LA);
      pix->hfr_update_batch2 = ~(pix->hfr_update_batch1);
      pix->hfr_update_mod_mask = pix->hfr_update_batch1;
   } else
     pix->hfr_update_mod_mask = 0xFFFFFFFF;

  /* check streaming mode for current streams*/
  pix->pix_params.cfg_and_3a_params.cfg.streaming_mode =
    isp_util_get_streaming_mode(pix);

  /* generate enable mask to decide which module will be on*/
  isp_pix_generate_module_enable_mask(pix, is_bayer_input, &is_initial_start);

  /*enable pix modules one by one except for stats module*/
  rc = isp_pix_pipeline_module_enable(pix);
  if (rc < 0) {
    CDBG_ERROR("%s:  isp_pix_pipeline_module_enable error = %d\n", __func__, rc);
    goto end;
  }

  /*trigger_enable pix modules one by one except for stats module*/
  rc = isp_pix_pipeline_module_trigger_enable(pix);
  if (rc < 0) {
    CDBG_ERROR("%s:  isp_pix_pipeline_module_enable error = %d\n", __func__, rc);
    goto end;
  }

  /* config pix moduled*/
  rc = isp_pix_pipeline_module_config(pix, is_bayer_input);
  if (rc < 0) {
    CDBG_ERROR("%s:isp_pix_pipeline_module_config error = %d\n", __func__, rc);
    goto end;
  }

  if(is_bayer_input) {
    rc = isp_pipeline_set_effect(pix);
    if (rc < 0) {
      CDBG_ERROR("%s:  isp_pipeline_set_effect error = %d\n", __func__, rc);
      goto end;
  }
 }

  rc = isp_pipeline_set_saved_sce_factor(pix);
  if (rc < 0) {
    CDBG_ERROR("%s:  isp_pipeline_set_saved_sce_factor error = %d\n",
      __func__, rc);
    goto end;
  }

  rc = isp_pipeline_set_saved_sharpness_factor(pix);
  if (rc < 0) {
    CDBG_ERROR("%s:  isp_pipeline_set_saved_sharpness_factor error = %d\n",
      __func__, rc);
    goto end;
  }

  if (is_bayer_input)
    rc = isp_pipeline_util_trigger_start(pix);
  if (rc < 0) {
    CDBG_ERROR("%s: pipeline trigger update error = %d\n", __func__, rc);
    goto end;
  }

  /* stats is only used in continuous mode now */
  if (pix->pix_params.cfg_and_3a_params.cfg.streaming_mode ==
      CAM_STREAMING_MODE_CONTINUOUS) {
    /* enable stats, AF is not enabled at this time.
     * AF is triggered by user land action */
    rc = isp_hw_pix_stats_enable_and_config(pix, 1);
    if (rc < 0) {
      CDBG_ERROR("%s: isp_hw_pix_stats_enable_and_config error = %d\n",
        __func__, rc);
      goto end;
    }
  }

  /* operation config */
  rc = pix->dep.operation_config(pix, is_bayer_input);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_hw_pix_operation_config error = %d\n", __func__, rc);
    goto end;
  }
end:
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d", __func__, rc);
  return rc;
}

/** isp_pix_pipeline_trigger_update_aec:
 *    @pix: pointer to pipeline
 *
 **/
static int isp_pix_pipeline_trigger_update_aec(isp_pipeline_t *pix)
{
  /* Note: Since AWB is dependent on AEC, trigger update for AWB always
   *       comes after trigger update of AEC. So move all the vfe module
   *       triggers to AWB.
   */
  return 0;
}


/* ============================================================
 * function name: isp_pipeline_stop
 * description:
 * ============================================================*/
static int isp_pipeline_stop(isp_pipeline_t *pipeline)
{
  int rc = 0;
  isp_pix_params_t *params = &pipeline->pix_params;
  isp_hw_pix_setting_params_t *cfg = &params->cfg_and_3a_params.cfg;

  rc = isp_pipeline_util_stats_start(pipeline, 0); /* stop */
  rc = isp_pipeline_util_stats_buf_cfg(pipeline, 0); /* unconfig */
  isp_pipeline_util_reset(pipeline);

  isp_hw_pix_trigger_update_params_t *trigger_input;
  trigger_input = &params->cfg_and_3a_params.trigger_input;

  cfg->effects.effect_type_mask = 0; /* bit of isp_effect_type_t */
  cfg->effects.contrast = 0;
  cfg->effects.hue = 0;
  cfg->effects.saturation = 0;
  cfg->effects.spl_effect = CAM_EFFECT_MODE_OFF;
  cfg->bestshot_mode = CAM_SCENE_MODE_OFF;
  params->isp_mode = ISP_MODE_2D;
  cfg->flash_params.flash_type = CAMERA_FLASH_LED; //Assume system has LED flash unless set otherwise
  trigger_input->stats_update.awb_update.color_temp = 4000;
  cfg->wb_mode = CAM_WB_MODE_AUTO;
  trigger_input->flash_mode = CAM_FLASH_MODE_OFF;
  cfg->crop_factor = Q12; /* set initial value to 4096 (1x zoom) */
  trigger_input->digital_gain = 1;
  memset(&cfg->af_rolloff_info, 0, sizeof(cfg->af_rolloff_info));


  return rc;
}

/* ============================================================
 * function name: isp_pipeline_af_start
 * description:
 * ============================================================*/
static int isp_pipeline_af_start(isp_pipeline_t *pipeline, void *data)
{
  int rc = 0;
  isp_pix_params_t *params = &pipeline->pix_params;
  isp_hw_pix_setting_params_t *cfg = &params->cfg_and_3a_params.cfg;
  /* TODO: need implementation */
  CDBG_ERROR("%s: need implementation\n", __func__);
  return rc;
}

/* ============================================================
 * function name: isp_pipeline_af_stop
 * description:
 * ============================================================*/
static int isp_pipeline_af_stop(isp_pipeline_t *pipeline, void *data)
{
  int rc = 0;
  isp_pix_params_t *params = &pipeline->pix_params;
  isp_hw_pix_setting_params_t *cfg = &params->cfg_and_3a_params.cfg;
  /* TODO: need implementation */
  CDBG_ERROR("%s: need implementation\n", __func__);
  return rc;
}

/* ============================================================
 * function name: isp_pix_get_streaming_mode
 * description:
 * ============================================================*/
static cam_streaming_mode_t isp_pix_get_streaming_mode(isp_pipeline_t *pix)
{
  int i;
  isp_hwif_output_cfg_t *outputs;
  cam_streaming_mode_t streaming_mode = CAM_STREAMING_MODE_BURST;

  outputs = pix->pix_params.cfg_and_3a_params.cfg.outputs;
   for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
     if (outputs[i].stream_param.width > 0 &&
         outputs[i].stream_param.streaming_mode ==
         CAM_STREAMING_MODE_CONTINUOUS) {
       streaming_mode = CAM_STREAMING_MODE_CONTINUOUS;
       break;
     }
   }
   return streaming_mode;
}

/* ============================================================
 * function name: isp_pipeline_ideal_raw_dump_start
 * description:
 * ============================================================*/
static int isp_pipeline_ideal_raw_dump_start(isp_pipeline_t *pipeline)
{
  /* TODO: ideal raw need implementation */
  int rc = 0;
  uint8_t is_bayer_input = isp_pipeline_util_is_bayer_fmt(pipeline);
  isp_pix_params_t *params = &pipeline->pix_params;
  isp_hw_pix_setting_params_t *cfg = &params->cfg_and_3a_params.cfg;
  CDBG_ERROR("%s: Configuring modules required for Idea Raw\n", __func__);

  pipeline->pix_params.cur_module_mask = ((1 << ISP_MOD_LINEARIZATION) |
  (1 << ISP_MOD_ROLLOFF) |
  (1 << ISP_MOD_DEMUX) |
  (1 << ISP_MOD_DEMOSAIC) |
  (1 << ISP_MOD_BPC) |
  (1 << ISP_MOD_ABF));

    /* config pix moduled*/
  rc = isp_pix_pipeline_module_config(pipeline, is_bayer_input);
  if (rc < 0) {
    CDBG_ERROR("%s:isp_pix_pipeline_module_config error = %d\n", __func__, rc);
  }

  /* hw update for piox modules */
  rc = isp_pipeline_module_hw_update(pipeline,
         is_bayer_input);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_pipeline_module_hw_update error = %d\n", __func__, rc);
  }
  return 0;

}

/* ============================================================
 * function name: isp_pipeline_start_stream
 * description:
 * ============================================================*/
static int isp_pipeline_start_stream(
   isp_pipeline_t *pipeline,
   start_stop_stream_t *action_data,
   uint32_t action_data_size)
{
  int rc = 0;

  /* only config pipeline once when it first starts */
  if (pipeline->num_active_streams == 0) {
    enum msm_vfe_axi_stream_src axi_path = VFE_AXI_SRC_MAX;
    pipeline->fast_aec_mode = action_data->fast_aec_mode;
    /* we need to consider camif dump or idea raw dump use cases */
    axi_path = isp_pipeline_util_get_stream_path(pipeline,
      action_data->session_id, action_data->stream_ids[0]);

    switch (axi_path) {
    case CAMIF_RAW:
      break;
    case IDEAL_RAW:
      /* both CAMIF&RDI RAW dont need pipeline.
         config idea raw */
      rc = isp_pipeline_ideal_raw_dump_start(pipeline);
      break;
    case PIX_ENCODER:
    case PIX_VIEWFINDER:
      /* fall through the pix stream start sequence */
      rc = isp_pipeline_start(pipeline);
      break;
    default:
      CDBG_ERROR("%s: not supported axi_path = %d\n", __func__, axi_path);
      return -1;
    }
    if (rc == 0) {
      rc = isp_pipeline_start_common(pipeline);
      if (rc < 0) {
        CDBG_ERROR("%s: start_common error, rc = %d\n", __func__, rc);
      }
    }
  }

  /* if pipeline already started only update ref count */
  if (rc == 0)
    pipeline->num_active_streams += action_data->num_streams;

  return rc;
}

/* ============================================================
 * function name: isp_pipeline_stop_stream
 * description:
 * ============================================================*/
static int isp_pipeline_stop_stream(
   isp_pipeline_t *pipeline,
   start_stop_stream_t *action_data,
   uint32_t action_data_size)
{
  int rc = 0;

  /* TODO: do we need to do anything here? */
  pipeline->num_active_streams -= action_data->num_streams;
  if (pipeline->num_active_streams == 0) {
    /* unconfig stats buf */
    rc = isp_pipeline_stop(pipeline);
  }
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d, active_streams = %d\n",
       __func__, rc, pipeline->num_active_streams);
  return rc;
}

/* ============================================================
 * function name: isp_pipeline_action
 * description:
 * ============================================================*/
int isp_pipeline_action (
  void *ctrl, uint32_t action_code,
  void *action_data, uint32_t action_data_size)
{
  isp_pipeline_t *pix = ctrl;
  int rc = 0;

  switch (action_code) {
  case ISP_PIX_ACTION_CODE_STREAM_START:
    isp_pipeline_util_dump_stream_dim(pix);
    rc = isp_pipeline_start_stream(pix,
                              (start_stop_stream_t *)action_data,
                              action_data_size);
    break;
  case ISP_PIX_ACTION_CODE_STREAM_STOP:
    rc = isp_pipeline_stop_stream(pix,
                              (start_stop_stream_t *)action_data,
                              action_data_size);
    //rc = isp_pipeline_stop(pix);
    break;
  case ISP_PIX_ACTION_CODE_AF_START:
    rc = isp_pipeline_af_start(pix, NULL);
    break;
  case ISP_PIX_ACTION_CODE_AF_STOP:
    rc = isp_pipeline_af_stop(pix, NULL);
    break;
  case ISP_PIX_ACTION_CODE_SYNC_TIMER_START:
    rc = -1;
    CDBG_ERROR("%s: sync timer has not been enabeld yet", __func__);
    //rc = isp_pix_camif_timer_config(pix);
    break;
  case ISP_PIX_ACTION_CODE_HW_UPDATE:
    rc = isp_pipeline_module_hw_update(pix,
         pix->pix_params.cfg_and_3a_params.cfg.camif_cfg.is_bayer_sensor);
    if (rc < 0) {
      ISP_DBG(ISP_MOD_COM,"%s: module hw register update error = %d\n", __func__, rc);
      return rc;
    }
    break;
  case ISP_PIX_ACTION_CODE_STATS_PARSE:
    if (pix->num_active_streams == 0) {
      /* got delayed stats. Drop it */
      ISP_DBG(ISP_MOD_COM,"%s: received dropped stats\n", __func__);
      break;
    }
    rc = pix->mod_ops[ISP_MOD_STATS]->action(
      pix->mod_ops[ISP_MOD_STATS]->ctrl,
      ISP_HW_MOD_ACTION_STATS_PARSE,
      action_data, action_data_size);
    if (rc < 0)
      CDBG_ERROR("%s: stats parsing error = %d\n",
                 __func__, rc);
    else {
      isp_hwif_output_cfg_t *output = pix->pix_params.cfg_and_3a_params.cfg.outputs;
      isp_pipeline_stats_parse_t *tmp_event = action_data;
      if (output[ISP_PIX_PATH_VIEWFINDER].stream_param.width != 0)
        tmp_event->session_id = output[ISP_PIX_PATH_VIEWFINDER].stream_param.session_id;
      else
         tmp_event->session_id = output[ISP_PIX_PATH_ENCODER].stream_param.session_id;
    }

    isp_pipeline_stats_parse_t *tmp_event = action_data;
    q3a_bg_stats_t *bg_stats =
      tmp_event->parsed_stats_event->stats_data[MSM_ISP_STATS_BG].stats_buf;
    isp_zoom_roi_params_t *roi_info =
      &(pix->pix_params.cfg_and_3a_params.cfg.saved_zoom_roi);

    if (bg_stats) {
      bg_stats->bg_region_height = roi_info->rgnHeight + 1;
      bg_stats->bg_region_width = roi_info->rgnWidth + 1;
      bg_stats->region_pixel_cnt =
        (roi_info->rgnWidth + 1) * (roi_info->rgnHeight + 1);
      bg_stats->rMax = roi_info->rMax;
      bg_stats->bMax = roi_info->bMax;
      bg_stats->grMax = roi_info->grMax;
      bg_stats->gbMax = roi_info->gbMax;
    }

    break;

  default:
    break;
  }
  return rc;
}

/* ============================================================
 * function name: isp_pipeline_destroy
 * description:
 * ============================================================*/
int isp_pipeline_destroy (void *ctrl)
{
  isp_pipeline_t *pix = ctrl;
  int i, rc = 0;
  isp_pix_params_t *params = &pix->pix_params;

  if (pix->pix_state >= ISP_PIX_STATE_INIT) {
    for (i = 0; i < ISP_MOD_MAX_NUM; i++) {
      if (pix->mod_ops[i]) {
        pix->mod_ops[i]->destroy(pix->mod_ops[i]->ctrl);
          pix->mod_ops[i]= NULL;
      }
    }
  }
  if (pix->dep.destroy)
    pix->dep.destroy(&pix->dep);
  free (pix);
  return 0;
}

/*===========================================================================
 * FUNCTION    - isp_pix_params_init -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This fucnction initializes all the default vfe params.
 *==========================================================================*/
static int isp_pipeline_params_init(isp_pipeline_t *pix)
{
  int rc = 0;
  isp_pix_params_t *params = &pix->pix_params;
  isp_hw_pix_setting_params_t *cfg = &params->cfg_and_3a_params.cfg;
  isp_hw_pix_trigger_update_params_t *trigger_input;

  trigger_input = &params->cfg_and_3a_params.trigger_input;
  rc = isp_hw_create_pipeline_dep(pix);
  if (rc < 0) {
    return rc;
  }
  cfg->effects.effect_type_mask = 0; /* bit of isp_effect_type_t */
  cfg->effects.contrast = 0;
  cfg->effects.hue = 0;
  cfg->effects.saturation = 0;
  cfg->effects.spl_effect = CAM_EFFECT_MODE_OFF;
  cfg->bestshot_mode = CAM_SCENE_MODE_OFF;
  params->isp_mode = ISP_MODE_2D;
  //Assume system has LED flash unless set otherwise
  cfg->flash_params.flash_type = CAMERA_FLASH_LED;
  trigger_input->stats_update.awb_update.color_temp = 4000;
  cfg->wb_mode = CAM_WB_MODE_AUTO;
  cfg->flash_params.sensitivity_led_hi = 1;
  cfg->flash_params.sensitivity_led_low = 1;
  trigger_input->flash_mode = CAM_FLASH_MODE_OFF;
  cfg->crop_factor = Q12; /* set initial value to 4096 (1x zoom) */
  trigger_input->digital_gain = 1;
  memset(&cfg->af_rolloff_info, 0, sizeof(cfg->af_rolloff_info));

  return 0;
}

/* ============================================================
 * function name: isp_pipeline_init
 * description:
 * ============================================================*/
int isp_pipeline_init (
   void *ctrl, /* pix ptr */
   void *in_params, /* int num of pix outputs */
   void *parent)
{
  isp_pipeline_t *pix = ctrl;
  int i, rc = 0;
  isp_hw_mod_init_params_t mod_init_params;
  isp_hw_t *isp_hw = parent;

  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  pix->parent = parent;
  memset(&mod_init_params,  0,  sizeof(mod_init_params));
  mod_init_params.fd = pix->fd;
  mod_init_params.max_stats_mask = pix->dep.max_supported_stats;
  mod_init_params.isp_version = pix->isp_version;
  mod_init_params.buf_mgr = pix->buf_mgr;
  /* we save HW dev_idx as handle*/
  mod_init_params.dev_idx = pix->mod_notify_ops.handle;
  mod_init_params.max_scaler_out_width =
    isp_hw->init_params.cap.isp_info.max_scaler_out_width;
  mod_init_params.max_scaler_out_height =
    isp_hw->init_params.cap.isp_info.max_scaler_out_height;
  for (i = 0; i < ISP_MOD_MAX_NUM; i++) {
    pix->mod_ops[i] =
      isp_hw_module_open(pix->isp_version, (isp_hw_module_id_t)i);

    if (pix->mod_ops[i]) {
      /* init module */
      rc = pix->mod_ops[i]->init(pix->mod_ops[i]->ctrl,
        (void *)&mod_init_params, &pix->mod_notify_ops);
      if (rc < 0) {
        /* module init error */
        CDBG_ERROR("%s: pix = %p, pix module init error = %d",
               __func__, pix, rc);
        break;
      }
    }
  }
  if (rc == 0)
    pix->pix_state = ISP_PIX_STATE_INIT;
  return rc;
}

/* ============================================================
 * function name: isp_pipeline_mod_notify
 * description:
 * ============================================================*/
int isp_pipeline_mod_notify (void *parent, uint32_t handle, uint32_t type,
  void *notify_data, uint32_t notify_data_size)
{
  int rc = 0;
  isp_pipeline_t *pipeline = parent;
  isp_hw_t *isp_hw = pipeline->parent;

  ISP_DBG(ISP_MOD_COM,"%s: type = %d\n", __func__, type);
  switch (type) {
  case ISP_HW_MOD_NOTIFY_FETCH_SCALER_OUTPUT:
    rc = pipeline->mod_ops[ISP_MOD_SCALER]->get_params(
             pipeline->mod_ops[ISP_MOD_SCALER]->ctrl,
             ISP_PIX_GET_SCALER_OUTPUT,
             NULL, 0,
             notify_data,
             sizeof(isp_pixel_window_info_t)*ISP_PIX_PATH_MAX);
    break;
  case ISP_HW_MOD_NOTIFY_FETCH_SCALER_CROP_REQUEST:
    rc = pipeline->mod_ops[ISP_MOD_SCALER]->get_params(
             pipeline->mod_ops[ISP_MOD_SCALER]->ctrl,
             ISP_PIX_GET_SCALER_CROP_REQUEST,
             NULL, 0,
             notify_data,
             sizeof(uint32_t)*ISP_PIX_PATH_MAX);
    break;
  case ISP_HW_MOD_NOTIFY_FETCH_FOVCROP_OUTPUT:
    rc = pipeline->mod_ops[ISP_MOD_FOV]->get_params(
             pipeline->mod_ops[ISP_MOD_FOV]->ctrl,
             ISP_PIX_GET_FOV_OUTPUT,
             NULL, 0,
             notify_data,
             sizeof(isp_pixel_line_info_t)*ISP_PIX_PATH_MAX);
    break;
  case ISP_HW_MOD_NOTIFY_MOD_ENABLE_FLAG:
    rc = pipeline->dep.module_enable_notify(pipeline,
           (isp_mod_enable_motify_t *)notify_data);
    break;

  case ISP_HW_MOD_NOTIFY_BE_CONFIG: {
    rc = isp_hw->notify_ops->notify(isp_hw->notify_ops->parent,
           isp_hw->notify_ops->handle, ISP_HW_NOTIFY_STATS_BE_CONFIG,
           notify_data, notify_data_size);
  }
    break;

  case ISP_HW_MOD_NOTIFY_ROLL_CONFIG: {
    rc = isp_hw->notify_ops->notify(isp_hw->notify_ops->parent,
           isp_hw->notify_ops->handle, ISP_HW_NOTIFY_ROLLOFF_CONFIG,
           notify_data, notify_data_size);
  }
    break;

  case ISP_HW_MOD_NOTIFY_BG_PCA_STATS_CONFIG: {
    rc = isp_hw->notify_ops->notify(isp_hw->notify_ops->parent,
           isp_hw->notify_ops->handle, ISP_HW_NOTIFY_BG_PCA_STATS_CONFIG,
           notify_data, notify_data_size);
  }
    break;

  case ISP_HW_MOD_NOTIFY_GET_ROLLOFF_TABLE: {
    rc = isp_hw->notify_ops->notify(isp_hw->notify_ops->parent,
           isp_hw->notify_ops->handle, ISP_HW_NOTIFY_ROLLOFF_GET,
           notify_data, notify_data_size);
  }
    break;

  default:
    rc = -1;
    break;
  }
  return rc;
}

/* ============================================================
 * function name:isp_hw_create_pipeline
 * description:
 * ============================================================*/
void *isp_hw_create_pipeline(
  int fd,
  uint32_t isp_version,
  int dev_idx,
  void *buf_mgr)
{
  int i;
  int rc = 0;

  isp_pipeline_t *pipeline = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  pipeline = malloc(sizeof(isp_pipeline_t));
  if (!pipeline) {
    /* no mem */
    ISP_DBG(ISP_MOD_COM,"%s: error, no mem", __func__);
    return NULL;
  }
  memset(pipeline, 0, sizeof(isp_pipeline_t));
  pipeline->fd = fd;
  pipeline->buf_mgr = buf_mgr;
  pipeline->isp_version = isp_version;
  pipeline->pix_state = ISP_PIX_STATE_INVALID;
  pipeline->parent = NULL;
  pipeline->mod_notify_ops.parent = (void *)pipeline;
  pipeline->mod_notify_ops.handle = dev_idx;
  pipeline->mod_notify_ops.notify = isp_pipeline_mod_notify;
  pipeline->pix_state = ISP_PIX_STATE_INVALID;
  rc = isp_pipeline_params_init(pipeline);
  if (rc < 0) {
    free (pipeline);
    CDBG_ERROR("%s: error = %d\n", __func__, rc);
    return NULL;
  } else
    return (void *)pipeline;
}
