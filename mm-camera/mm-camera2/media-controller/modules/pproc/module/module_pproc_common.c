/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <dlfcn.h>
#include <sys/time.h>
#include "camera_dbg.h"
#include "chromatix.h"
#include "chromatix_common.h"
#include "mct_port.h"
#include "mct_list.h"
#include "mct_module.h"
#include "mct_pipeline.h"
#include "mct_stream.h"
#include "module_pproc_common.h"
#include "port_pproc_common.h"
#include "module_cpp.h"

//#define CPP_ENABLE_LOG
#ifdef CPP_ENABLE_LOG
#undef CDBG
#define CDBG ALOGE
#endif

#define CPP_MAX_SHARPNESS         6
#define CPP_MIN_SHARPNESS         0

#define ASF_F_KERNEL_MAX          16
#define ASF_LUT_MAX               24
#define ASF_LUT3_MAX              12

#undef CPP_DUMP_FRAME
//#define CPP_DUMP_FRAME

/* TODO: This definitely needs to go */
boolean port_cpp_init(mct_port_t *port, mct_port_direction_t direction);
int32_t module_cpp_frame_done(uint32_t frame_skip, void *mod);
int32_t module_cpp_create_frame(void * data,
  pproc_interface_frame_divert_t *divert_frame);

#define LINEAR_INTERPOLATE(factor, reference_i, reference_iplus1) \
  ((factor * reference_iplus1) + ((1 - factor) * reference_i))

int32_t pproc_common_load_library(pproc_interface_t *pproc_iface,
  const char *name)
{
  char lib_name[BUFF_SIZE_255] = {0};
  char open_lib_str[BUFF_SIZE_255] = {0};
  void *(*pproc_lib_init)(void) = NULL;

  CDBG("%s:%d E\n", __func__, __LINE__);
  sprintf(lib_name, "libmmcamera_%s.so", name);
  CDBG("%s lib_name %s\n", __func__, lib_name);
  pproc_iface->lib_handle = dlopen(lib_name, RTLD_NOW);
  if (!pproc_iface->lib_handle) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }

  sprintf(open_lib_str, "pproc_library_init");
  *(void **)&pproc_lib_init = dlsym(pproc_iface->lib_handle, open_lib_str);
  if (!pproc_lib_init) {
    CDBG_ERROR("%s:%d pproc_library_init load failed\n", __func__, __LINE__);
    goto ERROR;
  }

  pproc_iface->lib_params = (pproc_interface_lib_params_t *)pproc_lib_init();
  if (!pproc_iface->lib_params || !pproc_iface->lib_params->func_tbl ||
      !pproc_iface->lib_params->func_tbl->open ||
      !pproc_iface->lib_params->func_tbl->process ||
      !pproc_iface->lib_params->func_tbl->close) {
    CDBG_ERROR("%s:%d pproc_library_init failed\n", __func__, __LINE__);
    goto ERROR;
  }
  CDBG("%s:%d X\n", __func__, __LINE__);
  return PPROC_SUCCESS;

ERROR:
  dlclose(pproc_iface->lib_handle);
  return PPROC_FAILURE;
}

int32_t pproc_common_unload_library(pproc_interface_t *pproc_iface)
{
  CDBG("%s:%d E\n", __func__, __LINE__);
  if (!pproc_iface) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  if (pproc_iface->lib_handle) {
    dlclose(pproc_iface->lib_handle);
    pproc_iface->lib_handle = NULL;
    pproc_iface->lib_params = NULL;
  }
  CDBG("%s:%d X\n", __func__, __LINE__);
  return PPROC_SUCCESS;
}

boolean module_pproc_traverse_port(void *list_data, void *user_data)
{
  mct_list_t *identity_holder;
  mct_port_t *port = (mct_port_t *)list_data;
  uint32_t *identity = (uint32_t *)user_data;

  identity_holder = mct_list_find_custom(MCT_PORT_CHILDREN(port),
    identity, port_pproc_common_match_identity);
  if (identity_holder != NULL) {
    uint32_t *list_identity = (uint32_t *)identity_holder->data;
    if (*list_identity == *identity) {
      return TRUE;
    }
  }
  return FALSE;
}

/** module_pproc_common_find_port_using_identity:
 *    @module: pproc submodule
 *    @identity: sessionid/streamid
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function finds a port using identity.
 **/
mct_port_t *module_pproc_common_find_port_using_identity(mct_module_t *module,
  uint32_t *identity)
{
  mct_list_t *identity_holder;

  CDBG("%s:%d %d", __func__, __LINE__, *identity);
  identity_holder = mct_list_find_custom(module->sinkports, identity,
    module_pproc_traverse_port);
  if (identity_holder != NULL) {
    return (mct_port_t *)identity_holder->data;
  }
  CDBG_ERROR("%s:Sink port not found to send event", __func__);
  return NULL;
}

boolean module_pproc_common_find_identity(void *data1, void *data2)
{
  module_pproc_common_frame_params_t *frame_params  =
    (module_pproc_common_frame_params_t *)data1;
  uint32_t *identity = (uint32_t *)data2;
  if (!frame_params || !identity) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }
  if (frame_params->identity == *identity) {
    return TRUE;
  }
  return FALSE;
}

static void module_pproc_common_update_default_asf_params(
  module_pproc_common_session_params_t *session_params,
  module_pproc_common_port_private_t *port_private)
{
  uint32_t i = 0;
  struct cpp_asf_info *asf_info = &port_private->asf_info;

  CDBG("%s:%d fill default params\n", __func__, __LINE__);
  if (session_params->asf_mode == ASF_DUAL_FILTER) {
    asf_info->sobel_h_coeff[0] = 0;
    asf_info->sobel_h_coeff[1] = 0;
    asf_info->sobel_h_coeff[2] = -0.010;
    asf_info->sobel_h_coeff[3] = -0.010;
    asf_info->sobel_h_coeff[4] = 0;
    asf_info->sobel_h_coeff[5] = -0.010;
    asf_info->sobel_h_coeff[6] = -0.030;
    asf_info->sobel_h_coeff[7] = -0.030;
    asf_info->sobel_h_coeff[8] = -0.010;
    asf_info->sobel_h_coeff[9] = -0.030;
    asf_info->sobel_h_coeff[10] = 0;
    asf_info->sobel_h_coeff[11] = 0.070;
    asf_info->sobel_h_coeff[12] = -0.010;
    asf_info->sobel_h_coeff[13] = -0.030;
    asf_info->sobel_h_coeff[14] = 0.070;
    asf_info->sobel_h_coeff[15] = 0.240;

    asf_info->sobel_v_coeff[0] = 0;
    asf_info->sobel_v_coeff[1] = 0;
    asf_info->sobel_v_coeff[2] = -0.010;
    asf_info->sobel_v_coeff[3] = -0.010;
    asf_info->sobel_v_coeff[4] = 0;
    asf_info->sobel_v_coeff[5] = -0.010;
    asf_info->sobel_v_coeff[6] = -0.030;
    asf_info->sobel_v_coeff[7] = -0.030;
    asf_info->sobel_v_coeff[8] = -0.010;
    asf_info->sobel_v_coeff[9] = -0.030;
    asf_info->sobel_v_coeff[10] = 0;
    asf_info->sobel_v_coeff[11] = 0.070;
    asf_info->sobel_v_coeff[12] = -0.010;
    asf_info->sobel_v_coeff[13] = -0.030;
    asf_info->sobel_v_coeff[14] = 0.07;
    asf_info->sobel_v_coeff[15] = 0.24;

    asf_info->hpf_h_coeff[0] = 0;
    asf_info->hpf_h_coeff[1] = 0;
    asf_info->hpf_h_coeff[2] = -0.010;
    asf_info->hpf_h_coeff[3] = -0.010;
    asf_info->hpf_h_coeff[4] = 0;
    asf_info->hpf_h_coeff[5] = -0.010;
    asf_info->hpf_h_coeff[6] = -0.030;
    asf_info->hpf_h_coeff[7] = -0.030;
    asf_info->hpf_h_coeff[8] = -0.010;
    asf_info->hpf_h_coeff[9] = -0.030;
    asf_info->hpf_h_coeff[10] = 0;
    asf_info->hpf_h_coeff[11] = 0.070;
    asf_info->hpf_h_coeff[12] = -0.010;
    asf_info->hpf_h_coeff[13] = -0.030;
    asf_info->hpf_h_coeff[14] = 0.070;
    asf_info->hpf_h_coeff[15] = 0.240;

    asf_info->hpf_v_coeff[0] = 0;
    asf_info->hpf_v_coeff[1] = 0;
    asf_info->hpf_v_coeff[2] = -0.010;
    asf_info->hpf_v_coeff[3] = -0.010;
    asf_info->hpf_v_coeff[4] = 0;
    asf_info->hpf_v_coeff[5] = -0.010;
    asf_info->hpf_v_coeff[6] = -0.030;
    asf_info->hpf_v_coeff[7] = -0.030;
    asf_info->hpf_v_coeff[8] = -0.010;
    asf_info->hpf_v_coeff[9] = -0.030;
    asf_info->hpf_v_coeff[10] = 0;
    asf_info->hpf_v_coeff[11] = 0.070;
    asf_info->hpf_v_coeff[12] = -0.010;
    asf_info->hpf_v_coeff[13] = -0.030;
    asf_info->hpf_v_coeff[14] = 0.07;
    asf_info->hpf_v_coeff[15] = 0.24;

    asf_info->lpf_coeff[0] = 0;
    asf_info->lpf_coeff[1] = 0;
    asf_info->lpf_coeff[2] = 0;
    asf_info->lpf_coeff[3] = 0;
    asf_info->lpf_coeff[4] = 0;
    asf_info->lpf_coeff[6] = 0;
    asf_info->lpf_coeff[7] = 0;
    asf_info->lpf_coeff[8] = 0;
    asf_info->lpf_coeff[9] = 0;
    asf_info->lpf_coeff[10] = 0;
    asf_info->lpf_coeff[11] = 0;
    asf_info->lpf_coeff[12] = 0;
    asf_info->lpf_coeff[13] = 0;
    asf_info->lpf_coeff[14] = 0;
    asf_info->lpf_coeff[15] = 1.0;

    asf_info->lut1[0] =  0.3502;
    asf_info->lut1[1] =  0.6286;
    asf_info->lut1[2] =  1.0791;
    asf_info->lut1[3] =  1.2971;
    asf_info->lut1[4] =  1.3446;
    asf_info->lut1[5] =  1.3497;
    asf_info->lut1[6] =  1.3500;
    asf_info->lut1[7] =  1.3500;
    asf_info->lut1[8] =  1.3500;
    asf_info->lut1[9] =  1.3500;
    asf_info->lut1[10] = 1.3500;
    asf_info->lut1[11] = 1.3500;
    asf_info->lut1[12] = 1.3500;
    asf_info->lut1[13] = 1.3500;
    asf_info->lut1[14] = 1.3500;
    asf_info->lut1[15] = 1.3500;
    asf_info->lut1[16] = 1.3500;
    asf_info->lut1[17] = 1.3500;
    asf_info->lut1[18] = 1.3500;
    asf_info->lut1[19] = 1.3500;
    asf_info->lut1[20] = 1.3500;
    asf_info->lut1[21] = 1.3500;
    asf_info->lut1[22] = 1.3500;
    asf_info->lut1[23] = 1.3500;

    asf_info->lut2[0] =  0.3502;
    asf_info->lut2[1] =  0.6286;
    asf_info->lut2[2] =  1.0791;
    asf_info->lut2[3] =  1.2971;
    asf_info->lut2[4] =  1.3446;
    asf_info->lut2[5] =  1.3497;
    asf_info->lut2[6] =  1.3500;
    asf_info->lut2[7] =  1.3500;
    asf_info->lut2[8] =  1.3500;
    asf_info->lut2[9] =  1.3500;
    asf_info->lut2[10] = 1.3500;
    asf_info->lut2[11] = 1.3500;
    asf_info->lut2[12] = 1.3500;
    asf_info->lut2[13] = 1.3500;
    asf_info->lut2[14] = 1.3500;
    asf_info->lut2[15] = 1.3500;
    asf_info->lut2[16] = 1.3500;
    asf_info->lut2[17] = 1.3500;
    asf_info->lut2[18] = 1.3500;
    asf_info->lut2[19] = 1.3500;
    asf_info->lut2[20] = 1.3500;
    asf_info->lut2[21] = 1.3500;
    asf_info->lut2[22] = 1.3500;
    asf_info->lut2[23] = 1.3500;

    for (i = 0; i < 24; i++) {
      asf_info->lut1[i] *= session_params->sharpness;
      asf_info->lut2[i] *= session_params->sharpness;
    }

    asf_info->lut3[0] = 1;
    asf_info->lut3[1] = 1;
    asf_info->lut3[2] = 1;
    asf_info->lut3[3] = 1;
    asf_info->lut3[4] = 1;
    asf_info->lut3[5] = 1;
    asf_info->lut3[6] = 1;
    asf_info->lut3[7] = 1;
    asf_info->lut3[8] = 1;
    asf_info->lut3[9] = 1;
    asf_info->lut3[10] = 1;
    asf_info->lut3[11] = 1;

    asf_info->sp = 0;
    asf_info->clamp_h_ul = 14;
    asf_info->clamp_h_ll = -14;
    asf_info->clamp_v_ul = 14;
    asf_info->clamp_v_ll = -14;
    asf_info->clamp_scale_max = 1;
    asf_info->clamp_scale_min = 1;
    asf_info->clamp_offset_max = 6;
    asf_info->clamp_offset_min = 6;
  } else if (session_params->asf_mode == ASF_EMBOSS ||
      session_params->asf_mode == ASF_SKETCH ||
      session_params->asf_mode == ASF_NEON) {
    asf_info->sp_eff_en = 1;
    asf_info->sp = 0;
    asf_info->nz_flag = 0x1A90;
    if (session_params->asf_mode == ASF_EMBOSS) {
      asf_info->neg_abs_y1 = 0;
      asf_info->hpf_v_coeff[0] = -0.25;
      asf_info->hpf_v_coeff[10] = 0.25;
      asf_info->lpf_coeff[5] = 0.125;
      for (i = 0; i < 24; i++) {
        asf_info->lut2[i] = 1.0;
      }
      asf_info->clamp_h_ul = 128;
      asf_info->clamp_h_ll = 128;
      asf_info->clamp_v_ul = 255;
      asf_info->clamp_v_ll = -255;
    } else if (session_params->asf_mode == ASF_SKETCH) {
      asf_info->neg_abs_y1 = 1;
      asf_info->sobel_h_coeff[0] = -0.25;
      asf_info->sobel_h_coeff[10] = 0.25;
      asf_info->lpf_coeff[5] = 0.25;
      asf_info->clamp_h_ul = 255;
      asf_info->clamp_h_ll = -255;
      asf_info->clamp_v_ul = 192;
      asf_info->clamp_v_ll = 192;
    } else if (session_params->asf_mode == ASF_NEON) {
      asf_info->neg_abs_y1 = 0;
      asf_info->sobel_h_coeff[0] = 0.25;
      asf_info->sobel_h_coeff[1] = 0.25;
      asf_info->sobel_h_coeff[4] = 0.25;
      asf_info->sobel_h_coeff[6] = -0.25;
      asf_info->sobel_h_coeff[9] = -0.25;
      asf_info->sobel_h_coeff[10] = -0.25;
      asf_info->clamp_h_ul = 255;
      asf_info->clamp_h_ll = -255;
      asf_info->clamp_v_ul = 0;
      asf_info->clamp_v_ll = 0;
    }
  }
}

static module_pproc_asf_region_t module_pproc_common_asf_find_region(
  module_pproc_aec_trigger_params_t *aec_trigger_params)
{
  if (aec_trigger_params->aec_trigger_input <=
      aec_trigger_params->brightlight_trigger_end) {
    return PPROC_ASF_BRIGHT_LIGHT;
  } else if ((aec_trigger_params->aec_trigger_input >
              aec_trigger_params->brightlight_trigger_end) &&
             (aec_trigger_params->aec_trigger_input <
              aec_trigger_params->brightlight_trigger_start)) {
    return PPROC_ASF_BRIGHT_LIGHT_INTERPOLATE;
  } else if ((aec_trigger_params->aec_trigger_input >=
              aec_trigger_params->brightlight_trigger_start) &&
             (aec_trigger_params->aec_trigger_input <=
              aec_trigger_params->lowlight_trigger_start)) {
    return PPROC_ASF_NORMAL_LIGHT;
  } else if ((aec_trigger_params->aec_trigger_input >
              aec_trigger_params->lowlight_trigger_start) &&
             (aec_trigger_params->aec_trigger_input <
              aec_trigger_params->lowlight_trigger_end)) {
    return PPROC_ASF_LOW_LIGHT_INTERPOLATE;
  } else if (aec_trigger_params->aec_trigger_input >=
             aec_trigger_params->lowlight_trigger_end) {
    return PPROC_ASF_LOW_LIGHT;
  } else {
    CDBG_ERROR("%s:%d code flow won't reach here\n", __func__, __LINE__);
    return PPROC_ASF_MAX_LIGHT;
  }
  return PPROC_ASF_MAX_LIGHT;
}

static void module_pproc_common_update_asf_kernel_coeff(
  chromatix_asf_7_7_type *asf_7_7, struct cpp_asf_info *asf_info,
  ASF_7x7_light_type type)
{
  uint32_t i = 0;

  /* Update en_dyna_clamp */
  asf_info->dyna_clamp_en = asf_7_7->en_dyna_clamp[type];

  for (i = 0; i < ASF_F_KERNEL_MAX; i++) {
    /* Update F1 kernel */
    asf_info->sobel_h_coeff[i] = asf_7_7->f1[type][i];
    /* Update F2 kernel */
    asf_info->sobel_v_coeff[i] = asf_7_7->f2[type][i];
    /* Update F3 kernel */
    asf_info->hpf_h_coeff[i] = asf_7_7->f3[type][i];
    /* Update F4 kernel */
    asf_info->hpf_v_coeff[i] = asf_7_7->f4[type][i];
    /* Update F5 kernel */
    asf_info->lpf_coeff[i] = asf_7_7->f5[i];
  }
}

static boolean module_pproc_common_fill_asf_kernel(
  chromatix_asf_7_7_type *asf_7_7, struct cpp_asf_info *asf_info,
  module_pproc_asf_region_t asf_region)
{
  boolean ret = TRUE;
  uint32_t i = 0;
  switch (asf_region) {
  case PPROC_ASF_LOW_LIGHT:
  case PPROC_ASF_LOW_LIGHT_INTERPOLATE:
    module_pproc_common_update_asf_kernel_coeff(asf_7_7, asf_info,
      ASF_7x7_LOW_LIGHT);
    break;
  case PPROC_ASF_NORMAL_LIGHT:
    module_pproc_common_update_asf_kernel_coeff(asf_7_7, asf_info,
      ASF_7x7_NORMAL_LIGHT);
    break;
  case PPROC_ASF_BRIGHT_LIGHT:
  case PPROC_ASF_BRIGHT_LIGHT_INTERPOLATE:
    module_pproc_common_update_asf_kernel_coeff(asf_7_7, asf_info,
      ASF_7x7_BRIGHT_LIGHT);
    break;
  default:
    CDBG_ERROR("%s:%d invalid asf_region %d\n", __func__, __LINE__, asf_region);
    ret = FALSE;
    break;
  }
  return ret;
}

static float module_pproc_common_calculate_interpolate_factor(
  float trigger_start, float trigger_end, float trigger_input)
{
  float factor = 0.0f;
  if (trigger_end - trigger_start) {
    factor = (trigger_input - trigger_start) / (trigger_end - trigger_start);
    CDBG("%s:%d factor %f = (trigger_input %f - trigger_start %f) / (trigger_end %f - trigger_start %f)\n",
      __func__, __LINE__, factor, trigger_input, trigger_start, trigger_end,
      trigger_start);
  } else {
    CDBG_ERROR("%s:%d trigger start and end has same values %f %f\n", __func__,
      __LINE__, trigger_start, trigger_end);
    factor = (trigger_input - trigger_start);
    CDBG("%s:%d factor %f = (trigger_input %f - trigger_start %f)\n",
      __func__, __LINE__, factor, trigger_input, trigger_start);
  }
  return factor;
}

static boolean module_pproc_common_interpolate_lut_params(
  module_pproc_common_session_params_t *session_params,
  chromatix_asf_7_7_type *asf_7_7, struct cpp_asf_info *asf_info,
  ASF_7x7_light_type type1, ASF_7x7_light_type type2, float interpolate_factor)
{
  uint32_t i = 0;
  for (i = 0; i < ASF_LUT_MAX; i++) {
    /* Interpolate LUT1 */
    asf_info->lut1[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_7_7->lut1[type1][i], asf_7_7->lut1[type2][i]);
    CDBG("%s:%d lut1[%d] %f = ((factor %f * reference_iplus1 %f) + ((1 - factor) %f * reference_i %f))\n",
      __func__, __LINE__, i, asf_info->lut1[i], interpolate_factor,
      asf_7_7->lut1[type2][i], 1 - interpolate_factor, asf_7_7->lut1[type1][i]);
    /* Update sharpness ratio */
    asf_info->lut1[i] *= session_params->sharpness;
    CDBG("%s:%d asf_info->lut1[%d] %f *= session_params->sharpness %f\n",
      __func__, __LINE__, i, asf_info->lut1[i], session_params->sharpness);
    /* Interpolate LUT2 */
    asf_info->lut2[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_7_7->lut2[type1][i], asf_7_7->lut2[type2][i]);
    CDBG("%s:%d lut2[%d] %f = ((factor %f * reference_iplus1 %f) + ((1 - factor) %f * reference_i %f))\n",
      __func__, __LINE__, i, asf_info->lut2[i], interpolate_factor,
      asf_7_7->lut2[type2][i], 1 - interpolate_factor, asf_7_7->lut2[type1][i]);
    /* Update sharpness ratio */
    asf_info->lut2[i] *= session_params->sharpness;
    CDBG("%s:%d asf_info->lut2[%d] %f *= session_params->sharpness %f\n",
      __func__, __LINE__, i, asf_info->lut2[i], session_params->sharpness);
  }
  for (i = 0; i < ASF_LUT3_MAX; i++) {
    /* Interpolate LUT3 */
    asf_info->lut3[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_7_7->lut3[type1][i], asf_7_7->lut3[type2][i]);
    CDBG("%s:%d lut3[%d] %f = ((factor %f * reference_iplus1 %f) + ((1 - factor) %f * reference_i %f))\n",
      __func__, __LINE__, i, asf_info->lut3[i], interpolate_factor,
      asf_7_7->lut3[type2][i], 1 - interpolate_factor, asf_7_7->lut3[type1][i]);
  }
  /* Interpolate sp */
  asf_info->sp = LINEAR_INTERPOLATE(interpolate_factor, asf_7_7->sp[type1],
    asf_7_7->sp[type2]);
  asf_info->sp = asf_info->sp / 100.0f;
  /* Interpolate clamp */
  asf_info->clamp_h_ul = LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->reg_hh[type1], asf_7_7->reg_hh[type2]);
  asf_info->clamp_h_ll = LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->reg_hl[type1], asf_7_7->reg_hl[type2]);
  asf_info->clamp_v_ul = LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->reg_vh[type1], asf_7_7->reg_vh[type2]);
  asf_info->clamp_v_ll = LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->reg_vl[type1], asf_7_7->reg_vl[type2]);
  asf_info->clamp_scale_max = LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->smax[type1], asf_7_7->smax[type2]);
  asf_info->clamp_scale_min = LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->smin[type1], asf_7_7->smin[type2]);
  asf_info->clamp_offset_max = LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->omax[type1], asf_7_7->omax[type2]);
  asf_info->clamp_offset_min = LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->omin[type1], asf_7_7->omin[type2]);

  return TRUE;
}

static void module_pproc_common_fill_noninterpolate_params(
  module_pproc_common_session_params_t *session_params,
  chromatix_asf_7_7_type *asf_7_7, struct cpp_asf_info *asf_info,
  module_pproc_asf_region_t asf_region)
{
  uint32_t i = 0;
  for (i = 0; i < ASF_LUT_MAX; i++) {
    asf_info->lut1[i] = asf_7_7->lut1[asf_region][i] *
      session_params->sharpness;
    asf_info->lut2[i] = asf_7_7->lut2[asf_region][i] *
      session_params->sharpness;
  }
  for (i = 0; i < ASF_LUT3_MAX; i++) {
    asf_info->lut3[i] = asf_7_7->lut3[asf_region][i];
  }
  asf_info->sp = asf_7_7->sp[asf_region] ;
  asf_info->sp = asf_info->sp / 100.0f;
  asf_info->clamp_h_ul = asf_7_7->reg_hh[asf_region];
  asf_info->clamp_h_ll = asf_7_7->reg_hl[asf_region];
  asf_info->clamp_v_ul = asf_7_7->reg_vh[asf_region];
  asf_info->clamp_v_ll = asf_7_7->reg_vl[asf_region];
  asf_info->clamp_scale_max = asf_7_7->smax[asf_region];
  asf_info->clamp_scale_min = asf_7_7->smin[asf_region];
  asf_info->clamp_offset_max = asf_7_7->omax[asf_region];
  asf_info->clamp_offset_min = asf_7_7->omin[asf_region];
}

static boolean module_pproc_common_fill_lut_params(
  module_pproc_common_session_params_t *session_params,
  chromatix_asf_7_7_type *asf_7_7, struct cpp_asf_info *asf_info,
  module_pproc_aec_trigger_params_t *aec_trigger_params,
  module_pproc_asf_region_t asf_region)
{
  boolean ret = TRUE;
  uint32_t i = 0;
  float interpolate_factor = 0.0f;
  switch (asf_region) {
  case PPROC_ASF_LOW_LIGHT:
    module_pproc_common_fill_noninterpolate_params(session_params, asf_7_7,
      asf_info, ASF_7x7_LOW_LIGHT);
    break;
  case PPROC_ASF_LOW_LIGHT_INTERPOLATE:
    interpolate_factor = module_pproc_common_calculate_interpolate_factor(
      aec_trigger_params->lowlight_trigger_start,
      aec_trigger_params->lowlight_trigger_end,
      aec_trigger_params->aec_trigger_input);
    if (interpolate_factor == 0.0f) {
      CDBG_ERROR("%s:%d interpolate_factor zero\n", __func__, __LINE__);
      ret = FALSE;
      break;
    }
    ret = module_pproc_common_interpolate_lut_params(session_params,
      asf_7_7, asf_info, ASF_7x7_NORMAL_LIGHT, ASF_7x7_LOW_LIGHT,
      interpolate_factor);
    if (ret == FALSE) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    }
    break;
  case PPROC_ASF_NORMAL_LIGHT:
    module_pproc_common_fill_noninterpolate_params(session_params, asf_7_7,
      asf_info, ASF_7x7_NORMAL_LIGHT);
    break;
  case PPROC_ASF_BRIGHT_LIGHT:
    module_pproc_common_fill_noninterpolate_params(session_params, asf_7_7,
      asf_info, ASF_7x7_BRIGHT_LIGHT);
    break;
  case PPROC_ASF_BRIGHT_LIGHT_INTERPOLATE:
    interpolate_factor = module_pproc_common_calculate_interpolate_factor(
      aec_trigger_params->brightlight_trigger_start,
      aec_trigger_params->brightlight_trigger_end,
      aec_trigger_params->aec_trigger_input);
    if (interpolate_factor == 0.0f) {
      CDBG_ERROR("%s:%d interpolate_factor zero\n", __func__, __LINE__);
      ret = FALSE;
      break;
    }
    ret = module_pproc_common_interpolate_lut_params(session_params,
      asf_7_7, asf_info, ASF_7x7_NORMAL_LIGHT, ASF_7x7_BRIGHT_LIGHT,
      interpolate_factor);
    if (ret == FALSE) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    }
    break;
  default:
    CDBG_ERROR("%s:%d invalid asf_region %d\n", __func__, __LINE__, asf_region);
    ret = FALSE;
    break;
  }
  return ret;
}

static boolean module_pproc_common_update_asf_params(
  module_pproc_common_session_params_t *session_params,
  module_pproc_common_port_private_t *port_private,
  module_pproc_common_frame_params_t *frame_params,
  module_pproc_aec_trigger_params_t *aec_trigger_params)
{
  boolean ret = TRUE;
  uint32_t i = 0;
  chromatix_parms_type   *chromatix_ptr = NULL;
  chromatix_ASF_7x7_type *chromatix_ASF_7x7 = NULL;
  chromatix_asf_7_7_type *asf_7_7 = NULL;
  struct cpp_asf_info    *asf_info = NULL;
  module_pproc_asf_region_t asf_region = PPROC_ASF_MAX_LIGHT;

  chromatix_ptr = port_private->chromatix.chromatixPtr;
  chromatix_ASF_7x7 = &chromatix_ptr->chromatix_VFE.chromatix_ASF_7x7;
  asf_7_7 = &chromatix_ASF_7x7->asf_7_7;
  asf_info = &port_private->asf_info;

  /* Fill ASF parameters in frame_params */
  if ((!chromatix_ASF_7x7->asf_7_7.asf_en) ||
     (aec_trigger_params->aec_trigger_input == 0.0f)) {
    /* Use default */
    module_pproc_common_update_default_asf_params(session_params, port_private);
    return TRUE;
  }
  if (session_params->asf_mode == ASF_DUAL_FILTER) {
    asf_region = module_pproc_common_asf_find_region(aec_trigger_params);
    if (asf_region >= PPROC_ASF_MAX_LIGHT) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }
    CDBG("%s:%d asf region %d\n", __func__, __LINE__, asf_region);
    /* Update F kernel and sp */
    ret = module_pproc_common_fill_asf_kernel(asf_7_7, asf_info, asf_region);
    if (ret == FALSE) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }
    /* Interpolate LUT params */
    ret = module_pproc_common_fill_lut_params(session_params, asf_7_7,
      asf_info, aec_trigger_params, asf_region);
    if (ret == FALSE) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }
    asf_info->sp_eff_en = 0;
  } else if (session_params->asf_mode == ASF_EMBOSS ||
      session_params->asf_mode == ASF_SKETCH ||
      session_params->asf_mode == ASF_NEON) {
    asf_info->sp_eff_en = 1;
    asf_info->sp = 0;
    asf_info->nz_flag = 0x1A90;
    if (session_params->asf_mode == ASF_EMBOSS) {
      asf_info->neg_abs_y1 = 0;
      asf_info->hpf_v_coeff[0] = -0.25;
      asf_info->hpf_v_coeff[10] = 0.25;
      asf_info->lpf_coeff[5] = 0.125;
      for (i = 0; i < 24; i++) {
        asf_info->lut2[i] = 1.0;
      }
      asf_info->clamp_h_ul = 128;
      asf_info->clamp_h_ll = 128;
      asf_info->clamp_v_ul = 255;
      asf_info->clamp_v_ll = -255;
    } else if (session_params->asf_mode == ASF_SKETCH) {
      asf_info->neg_abs_y1 = 1;
      asf_info->sobel_h_coeff[0] = -0.25;
      asf_info->sobel_h_coeff[10] = 0.25;
      asf_info->lpf_coeff[5] = 0.25;
      asf_info->clamp_h_ul = 255;
      asf_info->clamp_h_ll = -255;
      asf_info->clamp_v_ul = 192;
      asf_info->clamp_v_ll = 192;
    } else if (session_params->asf_mode == ASF_NEON) {
      asf_info->neg_abs_y1 = 0;
      asf_info->sobel_h_coeff[0] = 0.25;
      asf_info->sobel_h_coeff[1] = 0.25;
      asf_info->sobel_h_coeff[4] = 0.25;
      asf_info->sobel_h_coeff[6] = -0.25;
      asf_info->sobel_h_coeff[9] = -0.25;
      asf_info->sobel_h_coeff[10] = -0.25;
      asf_info->clamp_h_ul = 255;
      asf_info->clamp_h_ll = -255;
      asf_info->clamp_v_ul = 0;
      asf_info->clamp_v_ll = 0;
    }
  }
  return TRUE;
}

static boolean module_pproc_common_asf_interpolate(
  module_pproc_common_session_params_t *session_params,
  module_pproc_common_port_private_t *port_private,
  module_pproc_common_frame_params_t *frame_params)
{
  chromatix_parms_type   *chromatix_ptr = NULL;
  chromatix_ASF_7x7_type *chromatix_ASF_7x7 = NULL;
  module_pproc_aec_trigger_params_t aec_trigger_params;

  session_params->asf_mode = ASF_DUAL_FILTER;

  chromatix_ptr = port_private->chromatix.chromatixPtr;
  if (!chromatix_ptr) {
    CDBG_ERROR("%s:%d chromatix NULL\n", __func__, __LINE__);
    /* Use default */
    module_pproc_common_update_default_asf_params(session_params, port_private);
    return TRUE;
  }

  chromatix_ASF_7x7 = &chromatix_ptr->chromatix_VFE.chromatix_ASF_7x7;

  /* determine the control method */
  if (chromatix_ASF_7x7->control_asf_7x7 == 0) {
    /* Lux index based */
    aec_trigger_params.lowlight_trigger_start =
      chromatix_ASF_7x7->asf_7x7_lowlight_trigger.lux_index_start;
    aec_trigger_params.lowlight_trigger_end =
      chromatix_ASF_7x7->asf_7x7_lowlight_trigger.lux_index_end;
    aec_trigger_params.brightlight_trigger_start =
      chromatix_ASF_7x7->asf_7x7_brightlight_trigger.lux_index_start;
    aec_trigger_params.brightlight_trigger_end =
      chromatix_ASF_7x7->asf_7x7_brightlight_trigger.lux_index_end;
    aec_trigger_params.aec_trigger_input = port_private->aec_trigger_lux_idx;
  } else if (chromatix_ASF_7x7->control_asf_7x7 == 1) {
    /* Gain based */
    aec_trigger_params.lowlight_trigger_start =
      chromatix_ASF_7x7->asf_7x7_lowlight_trigger.gain_start;
    aec_trigger_params.lowlight_trigger_end =
      chromatix_ASF_7x7->asf_7x7_lowlight_trigger.gain_end;
    aec_trigger_params.brightlight_trigger_start =
      chromatix_ASF_7x7->asf_7x7_brightlight_trigger.gain_start;
    aec_trigger_params.brightlight_trigger_end =
      chromatix_ASF_7x7->asf_7x7_brightlight_trigger.gain_end;
    aec_trigger_params.aec_trigger_input = port_private->aec_trigger_real_gain;
  } else {
    /* Error in chromatix */
    CDBG_ERROR("%s:%d error in chromatix control type\n", __func__, __LINE__);
    return FALSE;
  }
  CDBG("%s:%d low start, end %f %f, bright start, end %f %f, trigger %f\n",
    __func__, __LINE__, aec_trigger_params.lowlight_trigger_start,
    aec_trigger_params.lowlight_trigger_end,
    aec_trigger_params.brightlight_trigger_start,
    aec_trigger_params.brightlight_trigger_end,
    aec_trigger_params.aec_trigger_input);

  module_pproc_common_update_asf_params(session_params, port_private,
    frame_params, &aec_trigger_params);

  return TRUE;
}

/** module_cpp_common_process_control_event:
 *    
 *  Return: 0 for success and negative error on failure
 *
 *  
 *  
 **/

static boolean module_pproc_common_event_control_set_parm(mct_module_t *module,
   mct_port_t *port, void *control_data, uint32_t identity)
{
  boolean                      ret = TRUE;
  mct_list_t                  *p_list = NULL;
  mct_event_control_parm_t    *event_control =
    (mct_event_control_parm_t *)control_data;
  module_pproc_common_ctrl_t  *mod_priv =
    (module_pproc_common_ctrl_t *)module->module_private;
  module_pproc_common_session_params_t *session_params = NULL;
  uint32_t session_id = (identity >> 16) & 0xFFFF;

  if (!port || !control_data || !mod_priv) {
    CDBG_ERROR("%s:%d failed port %p, control_data %p mod_priv %p\n",
      __func__, __LINE__, port, control_data, mod_priv);
    ret = FALSE;
    goto ERROR;
  }

  p_list = mct_list_find_custom(mod_priv->session_params, &session_id,
    module_pproc_common_match_mod_params_by_session);
  if (!p_list) {
    CDBG_ERROR("%s:%d] Error in finding feature params\n", __func__,
      __LINE__);
    return FALSE;
  }

  session_params =
    (module_pproc_common_session_params_t *)p_list->data;

  switch (event_control->type) {
  case CAM_INTF_PARM_SHARPNESS: {
    int32_t *sharpness = (int32_t *)event_control->parm_data;

   session_params->sharpness = (double)(*sharpness) /
     (CPP_MAX_SHARPNESS - CPP_MIN_SHARPNESS);

    CDBG("%s:%d sharpness %d ratio %f\n", __func__, __LINE__,
      *sharpness, session_params->sharpness);
    break;
  }
  case CAM_INTF_PARM_WAVELET_DENOISE: {
    session_params->denoise_params =
      *((cam_denoise_param_t *)event_control->parm_data);
    break;
  }
  default:
    break;
  }
ERROR:
  return ret;

}

void module_pproc_common_interpolate_wnr_params(float interpolation_factor,
  module_pproc_chromatix_denoise_params_t *chrmatix_denoise_params,
  ReferenceNoiseProfile_type *ref_noise_profile_i,
  ReferenceNoiseProfile_type *ref_noise_profile_iplus1)
{
  uint32_t k, j, offset;

  for (k = 0; k < PPROC_WDN_FILTER_LEVEL; k++) {
    for (j = 0; j < PPROC_MAX_PLANES; j++) {
      offset = k + 4 + j * 8;
      chrmatix_denoise_params->noise_profile[j][k] =
        LINEAR_INTERPOLATE(interpolation_factor,
        ref_noise_profile_i->referenceNoiseProfileData[offset],
        ref_noise_profile_iplus1->referenceNoiseProfileData[offset]);
    }

    /* Weight factor */
    chrmatix_denoise_params->weight[0][k] =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_weight_y[k],
      ref_noise_profile_iplus1->denoise_weight_y[k]);
    chrmatix_denoise_params->weight[1][k] =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_weight_chroma[k],
      ref_noise_profile_iplus1->denoise_weight_chroma[k]);
    chrmatix_denoise_params->weight[2][k] =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_weight_chroma[k],
      ref_noise_profile_iplus1->denoise_weight_chroma[k]);

    /* edge softness factor */
    chrmatix_denoise_params->edge_softness[0][k] =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_edge_softness_y[k],
      ref_noise_profile_iplus1->denoise_edge_softness_y[k]);
    chrmatix_denoise_params->edge_softness[1][k] =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_edge_softness_chroma[k],
      ref_noise_profile_iplus1->denoise_edge_softness_chroma[k]);
    chrmatix_denoise_params->edge_softness[2][k] =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_edge_softness_chroma[k],
      ref_noise_profile_iplus1->denoise_edge_softness_chroma[k]);

    /* denoise ratio */
    chrmatix_denoise_params->denoise_ratio[0][k] =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_scale_y[k],
      ref_noise_profile_iplus1->denoise_scale_y[k]);
    chrmatix_denoise_params->denoise_ratio[1][k] =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_scale_chroma[k],
      ref_noise_profile_iplus1->denoise_scale_chroma[k]);
    chrmatix_denoise_params->denoise_ratio[2][k] =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_scale_chroma[k],
      ref_noise_profile_iplus1->denoise_scale_chroma[k]);
  }
}

void module_pproc_common_init_wnr_params(
  module_pproc_common_frame_params_t *frame_params)
{
  uint32_t                                 i, j, k;
  module_pproc_chromatix_denoise_params_t *chrmatix_denoise_params;
  uint32_t                                 profile = 8;

  if (!frame_params) {
    CDBG_ERROR("%s:%d frame params error\n", __func__, __LINE__);
    return;
  }

  chrmatix_denoise_params = &frame_params->chrmatix_denoise_params;

  for (k = 0; k < PPROC_WDN_FILTER_LEVEL; k++) {
    for (j = 0; j < PPROC_MAX_PLANES; j++) {
      chrmatix_denoise_params->noise_profile[j][k] = profile;
      chrmatix_denoise_params->weight[j][k] = 0;
      chrmatix_denoise_params->edge_softness[j][k] = 12.75;
      chrmatix_denoise_params->denoise_ratio[j][k] = 12.75;
    }
    /* Update profile value for next level */
    profile /= 2;
  }

  return;
}

boolean module_pproc_common_update_wnr_params(
  module_pproc_common_port_private_t *port_priv,
  module_pproc_common_frame_params_t *frame_params, float trigger_input)
{
  chromatix_parms_type                    *chromatix_ptr;
  wavelet_denoise_type                    *wavelet_denoise;
  uint32_t                                 i, j, k;
  float                                    trigger_i, trigger_iplus1;
  float                                    interpolation_factor;
  module_pproc_chromatix_denoise_params_t *chrmatix_denoise_params;
  ReferenceNoiseProfile_type              *ref_noise_profile_i;
  ReferenceNoiseProfile_type              *ref_noise_profile_iplus1;
  uint32_t                                 offset;
  float                                    numerator, denominator;

  if (!frame_params) {
    CDBG_ERROR("%s:%d frame params error\n", __func__, __LINE__);
    return FALSE;
  }

  chromatix_ptr = port_priv->chromatix.chromatixPtr;
  if (!chromatix_ptr) {
    return TRUE;
  }
  wavelet_denoise =
        &chromatix_ptr->chromatix_VFE.chromatix_wavelet.wavelet_denoise_HW_420;
  chrmatix_denoise_params = &frame_params->chrmatix_denoise_params;

  if (wavelet_denoise->noise_profile[0].trigger_value >= trigger_input) {
    module_pproc_common_interpolate_wnr_params(0, chrmatix_denoise_params,
      &wavelet_denoise->noise_profile[0], &wavelet_denoise->noise_profile[0]);
    return TRUE;
  }

  /* Find the range in the availble grey patches */
  for (i = 0; i < NUM_GRAY_PATCHES - 1; i++) {
      trigger_i = wavelet_denoise->noise_profile[i].trigger_value;
      trigger_iplus1 = wavelet_denoise->noise_profile[i+1].trigger_value;
      if ((trigger_input > trigger_i) && (trigger_input <= trigger_iplus1)) {
        /* Interpolate all the values using i & iplus1 */
        numerator = (trigger_input - trigger_i);
        denominator = (trigger_iplus1 - trigger_i);
        if (denominator == 0.0f) {
          return TRUE;
        }
        interpolation_factor = numerator / denominator;
        ref_noise_profile_i = &wavelet_denoise->noise_profile[i];
        ref_noise_profile_iplus1 = &wavelet_denoise->noise_profile[i+1];

        module_pproc_common_interpolate_wnr_params(interpolation_factor,
          chrmatix_denoise_params, ref_noise_profile_i,
          ref_noise_profile_iplus1);
        return TRUE;
      } /* else iterate */
  }

  if (i == (NUM_GRAY_PATCHES - 1)) {
    module_pproc_common_interpolate_wnr_params(0, chrmatix_denoise_params,
      &wavelet_denoise->noise_profile[NUM_GRAY_PATCHES - 1],
      &wavelet_denoise->noise_profile[NUM_GRAY_PATCHES - 1]);
    return TRUE;
  }

  return FALSE;
}

static boolean module_pproc_common_wnr_interpolate(
  module_pproc_common_port_private_t *port_priv,
  module_pproc_common_frame_params_t *frame_params)
{
  chromatix_parms_type *chromatix_ptr;
  wavelet_denoise_type *wavelet_denoise;
  /* determine the control method */
  chromatix_ptr = port_priv->chromatix.chromatixPtr;
  if (!chromatix_ptr) {
    return TRUE;
  }
  wavelet_denoise =
    &chromatix_ptr->chromatix_VFE.chromatix_wavelet.wavelet_denoise_HW_420;
  if (wavelet_denoise->control_denoise == 0) {
    /* Lux index based */
    if (port_priv->aec_trigger_lux_idx != 0.0f) {
      module_pproc_common_update_wnr_params(port_priv, frame_params,
        port_priv->aec_trigger_lux_idx);
    }
  } else if (wavelet_denoise->control_denoise == 1) {
    /* Gain based */
    if (port_priv->aec_trigger_real_gain != 0.0f) {
      module_pproc_common_update_wnr_params(port_priv, frame_params,
        port_priv->aec_trigger_real_gain);
    }
  } else {
    /* Error in chromatix */
    CDBG_ERROR("%s:%d error in chromatix control type\n", __func__,
      __LINE__);
    return FALSE;
  }
  return 0;
}
#ifdef CPP_DUMP_FRAME
static boolean module_cpp_dump_frame(void *vaddr, uint32_t size, uint32_t ext)
{
  int out_file_fd;
  char out_fname[32];
  snprintf(out_fname, sizeof(out_fname), "%s_%d.yuv", "/data/cpp_input", ext);
  out_file_fd = open(out_fname, O_RDWR | O_CREAT, 0777);
  if (out_file_fd < 0) {
    ALOGE("Cannot open file\n");
  }
  write(out_file_fd, vaddr, 18051072);
  close(out_file_fd);
  return 0;
}
#endif

static boolean module_pproc_common_find_stream(void *data1, void *data2)
{
  mct_stream_t *stream = (void *)data1;
  uint32_t *input_stream_id = (uint32_t *)data2;
  if (!stream || !input_stream_id) {
    CDBG_ERROR("%s:%d failed stream %p input_stream_id %p", __func__, __LINE__,
      stream, input_stream_id);
  }
  CDBG("request stream id %x incoming ide %x", *input_stream_id,
    stream->streaminfo.identity);
  if ((stream->streaminfo.identity & 0xFFFF) == *input_stream_id) {
    return TRUE;
  }
  return FALSE;
}

static boolean module_pproc_common_find_input_buffer(void *data1, void *data2)
{
  mct_stream_map_buf_t *buf_holder = (mct_stream_map_buf_t *)data1;
  int *buf_index = (int *)data2;
  if (!buf_holder || !buf_index) {
    CDBG_ERROR("%s:%d failed buf_holder %p buf_index %p\n", __func__, __LINE__,
      buf_holder, buf_index);
    return FALSE;
  }
  CDBG("request buf index %d incoming buf index %d\n", *buf_index,
    buf_holder->buf_index);
  if (buf_holder->buf_index == *buf_index) {
    return TRUE;
  }
  return FALSE;
}

static void *module_pproc_common_get_input_buffer(mct_module_t *module,
  mct_stream_info_t *stream_info, cam_stream_parm_buffer_t *parm_buf,
  uint32_t identity)
{
  uint32_t session_id = identity >> 16;
  uint32_t stream_id = identity & 0xFFFF;
  mct_pipeline_t *pipeline = NULL;
  mct_stream_t *stream = NULL;
  mct_list_t *stream_list = NULL, *buf_list = NULL;
  /* Validate input params */
  if (!module || !stream_info) {
    CDBG_ERROR("%s:%d failed module %p stream_info %p\n", __func__, __LINE__,
      module, stream_info);
    return NULL;
  }
  /* Get pproc module's parent - stream */
  stream_list = mct_list_find_custom(MCT_MODULE_PARENT(module), &stream_id,
    module_pproc_common_find_stream);
  if (!stream_list) {
    CDBG_ERROR("%s:%d failed stream_list %p\n", __func__, __LINE__, stream_list);
    return NULL;
  }
  if (!stream_list->data) {
    CDBG_ERROR("%s:%d failed stream_list->data %p", __func__, __LINE__,
      stream_list->data);
    return FALSE;
  }
  stream = (mct_stream_t *)stream_list->data;
  /* Get stream's parent - pipeline */
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  if (!pipeline) {
    CDBG_ERROR("%s:%d failed pipeline %p\n", __func__, __LINE__, pipeline);
    return NULL;
  }
  stream_list = mct_list_find_custom(MCT_PIPELINE_CHILDREN(pipeline),
    &stream_info->reprocess_config.online.input_stream_id,
    module_pproc_common_find_stream);
  if (!stream_list) {
    CDBG_ERROR("%s:%d failed stream_list %p", __func__, __LINE__, stream_list);
    return FALSE;
  }
  if (!stream_list->data) {
    CDBG_ERROR("%s:%d failed stream_list->data %p", __func__, __LINE__,
      stream_list->data);
    return FALSE;
  }
  stream = (mct_stream_t *)stream_list->data;
  buf_list = mct_list_find_custom(stream->streaminfo.img_buffer_list,
    &parm_buf->reprocess.buf_index, module_pproc_common_find_input_buffer);
  if (!buf_list) {
    CDBG_ERROR("%s:%d failed buf_list %p", __func__, __LINE__, buf_list);
    return FALSE;
  }
  if (!buf_list->data) {
    CDBG_ERROR("%s:%d failed buf_list->data %p", __func__, __LINE__,
      buf_list->data);
    return FALSE;
  }
  return buf_list->data;
}

/** module_pproc_proccess_offline_stream:
 *    @mct_module_t: pproc submodule
 *    @data: pointer to cam_stream_parm_buffer_t
 *    @identity: session id - stream id pair
 *    @port: port on which this event is triggered
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function handles offline frame processing and blocks
 *  untils CPP thread processes the frame
 **/
static boolean module_pproc_proccess_offline_stream(mct_module_t *module,
  void *data, uint32_t identity, mct_port_t *port)
{
  boolean rc = TRUE;
  cam_stream_parm_buffer_t *parm_buf = (cam_stream_parm_buffer_t *)data;
  mct_list_t *p_list = NULL;
  module_pproc_common_port_private_t *port_private = NULL;
  module_pproc_common_frame_params_t *frame_params = NULL;
  module_pproc_common_session_params_t *session_params = NULL;
  mct_stream_info_t *streaminfo = NULL;
  module_pproc_common_ctrl_t *pproc_ctrl = NULL;
  pproc_interface_frame_divert_t new_frame;
  isp_buf_divert_t buff_divert;
  mct_module_t *pproc_module = NULL;
  mct_stream_map_buf_t *buf_holder = NULL;
#ifdef CPP_DUMP_FRAME
  static uint32_t cpp_input_dump_count = 0;
#endif
  stats_get_data_t *stats_get = NULL;
  cam_metadata_info_t *bus_msg = NULL;
  uint32_t session_id = identity >> 16;
  mct_stream_session_metadata_info *metadata = NULL;
  cam_crop_data_t *crop_data = NULL;
  uint8_t num_of_streams = 0, index = 0;

  if (!parm_buf || parm_buf->type != CAM_STREAM_PARAM_TYPE_DO_REPROCESS) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }
  pproc_ctrl = (module_pproc_common_ctrl_t *)module->module_private;
  port_private = (module_pproc_common_port_private_t *)port->port_private;
  p_list = mct_list_find_custom(port_private->frame_params, &identity,
    module_pproc_common_find_identity);
  frame_params = (module_pproc_common_frame_params_t *)p_list->data;

  p_list = mct_list_find_custom(pproc_ctrl->session_params, &session_id,
    module_pproc_common_match_mod_params_by_session);
  if (!p_list) {
    CDBG_ERROR("%s:%d] Error in finding feature params\n", __func__,
      __LINE__);
    return FALSE;
  }

  session_params = (module_pproc_common_session_params_t *)p_list->data;
  if (!session_params) {
    CDBG_ERROR("%s:%d failed session_params NULL\n", __func__, __LINE__);
    return FALSE;
  }

  streaminfo = frame_params->stream_info;
  pproc_module = module_pproc_common_get_container(module->module_private);
  if (!pproc_module) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }
  rc = pproc_common_buff_mgr_attach_identity(pproc_ctrl->buff_mgr_client,
    streaminfo->identity, streaminfo->img_buffer_list);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return rc;
  }

  buf_holder = module_pproc_common_get_input_buffer(pproc_module, streaminfo,
    parm_buf, identity);
  if (!buf_holder) {
    CDBG_ERROR("%s:%d failed buf_holder %p\n", __func__, __LINE__, buf_holder);
    return FALSE;
  }
  CDBG("%s:%d get buf size %d buf type %d\n", __func__, __LINE__,
    buf_holder->buf_size, buf_holder->buf_type);
  /* isp_divert_buffer is same as buff_divert */
  if (sizeof(isp_buf_divert_t) != sizeof(pproc_intf_buff_data_t)) {
    CDBG("%s: Alert isp divert structure is having new members", __func__);
  }

  bus_msg = (cam_metadata_info_t *)mct_module_get_buffer_ptr(
    parm_buf->reprocess.meta_buf_index, pproc_module, identity >> 16,
    parm_buf->reprocess.meta_stream_handle);
  if (!bus_msg) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }
  metadata = (mct_stream_session_metadata_info *)bus_msg->private_metadata;
  CDBG("%s:%d c %p com c %p\n", __func__, __LINE__,
    metadata->sensor_data.chromatix_ptr,
    metadata->sensor_data.common_chromatix_ptr);
  stats_get = (stats_get_data_t *)&metadata->stats_aec_data.private_data;
  if (!stats_get) {
    CDBG_ERROR("%s:%d stats_get NULL\n", __func__, __LINE__);
  } else {
    CDBG("%s:%d gain %f lux index %f\n", __func__, __LINE__,
      stats_get->aec_get.real_gain[0], stats_get->aec_get.lux_idx);
    port_private->chromatix.chromatixPtr =
      metadata->sensor_data.chromatix_ptr;
    port_private->chromatix.chromatixComPtr =
      metadata->sensor_data.common_chromatix_ptr;
    port_private->aec_trigger_lux_idx = stats_get->aec_get.lux_idx;
    port_private->aec_trigger_real_gain = stats_get->aec_get.real_gain[0];
    CDBG("%s:%d aec trigger lux idx %f real gain %f\n", __func__, __LINE__,
      port_private->aec_trigger_lux_idx, port_private->aec_trigger_real_gain);
    module_pproc_common_wnr_interpolate(port_private, frame_params);
    module_pproc_common_asf_interpolate(session_params, port_private,
      frame_params);
  }

  memset(&buff_divert, 0, sizeof(isp_buf_divert_t));
  buff_divert.fd = buf_holder->buf_planes[0].fd;
  buff_divert.native_buf = TRUE;
  buff_divert.identity = identity;
  new_frame.isp_divert_buffer.native_buf = TRUE;
  new_frame.isp_divert_buffer.vaddr = buf_holder->buf_planes[0].buf;
  new_frame.isp_divert_buffer.fd = buf_holder->buf_planes[0].fd;
  new_frame.isp_divert_buffer.v4l2_buffer_obj.index = buf_holder->buf_index;
  new_frame.isp_divert_buffer.v4l2_buffer_obj.sequence =
    parm_buf->reprocess.frame_idx;
  new_frame.isp_divert_buffer.is_locked = TRUE;
  new_frame.isp_divert_buffer.ack_flag = FALSE;
  new_frame.isp_divert_buffer.identity = identity;
  new_frame.out_buff_idx = -1;
  /* used by divert frame to put back in v4l2 buffer */
  new_frame.frame_params.frame_id = parm_buf->reprocess.frame_idx;
  new_frame.mct_event_identity = identity;

  /* Update src width and src height */
  frame_params->src_width = streaminfo->dim.width;
  frame_params->src_height = streaminfo->dim.height;

  crop_data = &bus_msg->crop_data;
  /* Validate input crop parameters */
  if (bus_msg->is_crop_valid == TRUE) {
    num_of_streams = crop_data->num_of_streams;
    for (index = 0; index < num_of_streams; index++) {
      if (crop_data->crop_info->stream_id ==
          streaminfo->reprocess_config.online.input_stream_id) {
        break;
      }
    }
    if ((index < num_of_streams) &&
        (crop_data->crop_info[index].crop.width > 0) &&
        (crop_data->crop_info[index].crop.width <= streaminfo->dim.width) &&
        (crop_data->crop_info[index].crop.height > 0) &&
        (crop_data->crop_info[index].crop.height <= streaminfo->dim.height)) {
      CDBG("%s:%d\n", __func__, __LINE__);
      /* Update stream crop with input crop parameters present in stream info */
      frame_params->stream_crop.x = crop_data->crop_info[index].crop.left;
      frame_params->stream_crop.y = crop_data->crop_info[index].crop.top;
      frame_params->stream_crop.crop_out_x =
        crop_data->crop_info[index].crop.width;
      frame_params->stream_crop.crop_out_y =
        crop_data->crop_info[index].crop.height;
    } else {
      CDBG("%s:%d\n", __func__, __LINE__);
      /* Validate stream crop from dimension info */
      frame_params->stream_crop.x = 0;
      frame_params->stream_crop.y = 0;
      frame_params->stream_crop.crop_out_x = streaminfo->dim.width;
      frame_params->stream_crop.crop_out_y = streaminfo->dim.height;
    }
  } else {
    CDBG("%s:%d\n", __func__, __LINE__);
    /* Validate stream crop from dimension info */
    frame_params->stream_crop.x = 0;
    frame_params->stream_crop.y = 0;
    frame_params->stream_crop.crop_out_x = streaminfo->dim.width;
    frame_params->stream_crop.crop_out_y = streaminfo->dim.height;
  }
  CDBG("%s:%d crop %d %d %d %d\n", __func__, __LINE__,
    frame_params->stream_crop.x, frame_params->stream_crop.y,
    frame_params->stream_crop.crop_out_x, frame_params->stream_crop.crop_out_y);
  /* Initialize DIS / EIS crop params */
  frame_params->is_crop.x = 0;
  frame_params->is_crop.y = 0;
  frame_params->is_crop.width = streaminfo->dim.width;
  frame_params->is_crop.height = streaminfo->dim.height;
  frame_params->src_stride =
    streaminfo->buf_planes.plane_info.mp[0].stride;
  frame_params->src_scanline =
    streaminfo->buf_planes.plane_info.mp[0].scanline;

#ifdef CPP_DUMP_FRAME
  module_cpp_dump_frame(new_frame.isp_divert_buffer.vaddr, buf_holder->buf_size,
    cpp_input_dump_count++);
#endif

  rc = module_cpp_fill_input_params(module, &new_frame.frame_params,
    &buff_divert, frame_params, identity, port);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d Get buffer failed.\n", __func__, __LINE__);
    return rc;
  }
  CDBG_ERROR("%s:%d offline input ide %x buf id %d frame id %d\n", __func__,
    __LINE__, identity, buf_holder->buf_index, new_frame.frame_params.frame_id);
  /* Create frame info based on input frame config */
  /* Queue to driver to process frame */
  pproc_ctrl->pproc_iface->lib_params->func_tbl->process
    (pproc_ctrl->pproc_iface->lib_params->lib_private_data,
     PPROC_IFACE_FRAME_DIVERT, (void *)&new_frame);
  return rc;
}

/** module_pproc_common_process_control_event:
 *    @mct_module_t: pproc submodule
 *    @mct_event_control_t: control event to be processed
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function handles all control events originally
 *  generated by mct and sends those events downstream /
 *  upstream
 **/
boolean module_pproc_common_process_control_event(mct_module_t *module,
  mct_event_control_t *event, uint32_t identity, mct_port_t *port)
{
  module_pproc_common_ctrl_t *pproc_ctrl = NULL;
  boolean ret = TRUE;
  mct_event_control_t *event_ctrl = NULL;

  if (!module || !module->module_private || !event) {
    CDBG_ERROR("%s: parameter is null\n", __func__);
    return FALSE;
  }

  CDBG("%s:%d event id %d\n", __func__, __LINE__, event->type);

  pproc_ctrl =
    (module_pproc_common_ctrl_t *)module->module_private;

  switch (event->type) {
  case MCT_EVENT_CONTROL_STREAMON: {
    module_pproc_common_session_params_t *session_params;
    module_pproc_common_port_private_t *port_priv =
      (module_pproc_common_port_private_t *)port->port_private;
    module_pproc_common_frame_params_t *frame_params = NULL;
    uint32_t session_id = (identity >> 16) & 0xFFFF;
    mct_list_t *p_list = NULL;
    mct_stream_info_t *streaminfo =
      (mct_stream_info_t *)event->control_event_data;
    mct_event_t new_event;
    stats_get_data_t stats_get;
    mct_port_t *mct_port = NULL;

    CDBG("%s:buff_client=%p, streaminfo:%p, identity:%d, img_list:%p\n",
      __func__, pproc_ctrl->buff_mgr_client, streaminfo, streaminfo->identity,
      streaminfo->img_buffer_list);
    CDBG("%s:Camcorder_CPP:width:%d, height:%d\n", __func__,
      streaminfo->dim.width, streaminfo->dim.height);

    p_list = mct_list_find_custom(pproc_ctrl->session_params, &session_id,
      module_pproc_common_match_mod_params_by_session);
    if (!p_list) {
      CDBG_ERROR("%s:%d] Error in finding feature params\n", __func__,
        __LINE__);
      return FALSE;
    }

    session_params = (module_pproc_common_session_params_t *)p_list->data;
    if (!session_params) {
      CDBG_ERROR("%s:%d failed session_params NULL\n", __func__, __LINE__);
      return FALSE;
    }

    p_list = mct_list_find_custom(port_priv->frame_params,
      &streaminfo->identity, module_pproc_common_find_identity);
    if (!p_list) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }

    frame_params = (module_pproc_common_frame_params_t *)p_list->data;
    if (streaminfo->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
      ret = pproc_common_buff_mgr_attach_identity(pproc_ctrl->buff_mgr_client,
         streaminfo->identity, streaminfo->img_buffer_list);
      new_event.type = MCT_EVENT_MODULE_EVENT;
      new_event.identity = streaminfo->identity;
      new_event.direction = MCT_EVENT_UPSTREAM;
      new_event.u.module_event.type = MCT_EVENT_MODULE_PPROC_GET_AEC_UPDATE;
      new_event.u.module_event.module_event_data = (void *)&stats_get;
      mct_port = module_pproc_common_find_port_using_identity(module,
        &streaminfo->identity);
      if (!mct_port) {
        CDBG("%s:Error getting sink port\n", __func__);
        return FALSE;
      }

      mct_port_send_event_to_peer(mct_port, &new_event);
      if (ret == FALSE) {
        CDBG_ERROR("%s %d failed\n", __func__, __LINE__);
        return FALSE;
      }

      CDBG("%s:%d gain %f lux index %f\n", __func__, __LINE__,
        stats_get.aec_get.real_gain[0], stats_get.aec_get.lux_idx);
      port_priv->aec_trigger_lux_idx = stats_get.aec_get.lux_idx;
      port_priv->aec_trigger_real_gain = stats_get.aec_get.real_gain[0];
      CDBG("%s:%d aec trigger lux idx %f real gain %f\n", __func__, __LINE__,
        port_priv->aec_trigger_lux_idx, port_priv->aec_trigger_real_gain);
      module_pproc_common_wnr_interpolate(port_priv, frame_params);
      module_pproc_common_asf_interpolate(session_params, port_priv,
        frame_params);
    } else {
      CDBG("%s:%d module_pproc_common_update_default_asf_params\n", __func__,
        __LINE__);
      module_pproc_common_update_default_asf_params(session_params, port_priv);
    }
    break;
  }
  case MCT_EVENT_CONTROL_STREAMOFF: {
    mct_stream_info_t *streaminfo =
      (mct_stream_info_t *)event->control_event_data;

    CDBG("%s: MCT_EVENT_CONTROL_STREAMOFF \n", __func__);
    pproc_ctrl->pproc_iface->lib_params->func_tbl->process
      (pproc_ctrl->pproc_iface->lib_params->lib_private_data,
       PPROC_IFACE_STOP_STREAM, NULL);
    ret = pproc_common_buff_mgr_detach_identity(pproc_ctrl->buff_mgr_client,
      streaminfo->identity);
    break;
  }

  case MCT_EVENT_CONTROL_SET_PARM: {
    module_pproc_common_event_control_set_parm(module,
      port, event->control_event_data, identity);
   break;
  }

  case MCT_EVENT_CONTROL_PARM_STREAM_BUF:
    ret = module_pproc_proccess_offline_stream(module,
      event->control_event_data, identity, port);
    break;
  default:
    break;
  }
  return ret;
}

void print_buffer(isp_buf_divert_t *buff_divert)
{
  CDBG("%s:index:%d\n", __func__, buff_divert->buffer.index);
  CDBG("%s:native_buf:%d\n", __func__, buff_divert->native_buf);
  CDBG("%s:vaddr:%p\n", __func__, buff_divert->vaddr);
  CDBG("%s:fd:%d\n", __func__, buff_divert->fd);

  CDBG("%s:type:%d\n", __func__, buff_divert->buffer.type);
  CDBG("%s:bytesused:%d\n", __func__, buff_divert->buffer.bytesused);
  CDBG("%s:flags:%d\n", __func__, buff_divert->buffer.flags);
  CDBG("%s:field:%d\n", __func__, buff_divert->buffer.field);

  CDBG("%s:sequence:%d\n", __func__, buff_divert->buffer.sequence);
  CDBG("%s:memory:%d\n", __func__, buff_divert->buffer.memory);
  CDBG("%s:userptr[0]:%ld\n", __func__, buff_divert->buffer.m.planes[0].m.userptr);
  CDBG("%s:userptr[1]:%ld\n", __func__, buff_divert->buffer.m.planes[1].m.userptr);
  CDBG("%s:plane[0].bytesused:%d\n", __func__, buff_divert->buffer.m.planes[0].bytesused);
  CDBG("%s:plane[1].bytesused:%d\n", __func__, buff_divert->buffer.m.planes[1].bytesused);
  CDBG("%s:plane[0].length:%d\n", __func__, buff_divert->buffer.m.planes[0].length);
  CDBG("%s:plane[1].length:%d\n", __func__, buff_divert->buffer.m.planes[1].length);
}

/** module_pproc_common_process_module_event:
 *    @module: mct module handle
 *    @event: module event to be processed
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function handles all events and sends those events
 *  downstream / upstream
 **/
boolean module_pproc_common_process_module_event(mct_module_t *module,
  mct_event_module_t *event, uint32_t event_identity, mct_port_t *port)
{
  module_pproc_common_ctrl_t           *module_ctrl = NULL;
  module_pproc_common_session_params_t *session_params;

  CDBG("%s:%d module event id %d\n", __func__, __LINE__, event->type);

  if (!event || !module) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }
  module_ctrl = (module_pproc_common_ctrl_t *)module->module_private;

  /* handle module event here */
  switch (event->type) {
  case MCT_EVENT_MODULE_SET_CHROMATIX_PTR: {
    module_pproc_common_port_private_t *port_private = NULL;
    mct_list_t *list_frame_params = NULL;
    module_pproc_common_frame_params_t *frame_params = NULL;
    modulesChromatix_t *chromatix_param =
      (modulesChromatix_t *)event->module_event_data;
    chromatix_parms_type *chromatix_ptr;
    wavelet_denoise_type *wavelet_denoise;

    if (!chromatix_param) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }
    port_private = (module_pproc_common_port_private_t *)port->port_private;
    list_frame_params = mct_list_find_custom(port_private->frame_params,
      &event_identity, module_pproc_common_find_identity);
    if (!list_frame_params) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }
    frame_params =
      (module_pproc_common_frame_params_t *)list_frame_params->data;
    if (!frame_params) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }

    port_private->chromatix = *chromatix_param;
    break;
  }
  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
    stats_update_t *stats_update =
      (stats_update_t *)event->module_event_data;
    aec_update_t   *aec_update = &stats_update->aec_update;
    float           aec_trigger_input;
    uint32_t        session_id = (event_identity >> 16) & 0xFFFF;
    module_pproc_common_frame_params_t *frame_params = NULL;
    module_pproc_common_port_private_t *port_priv =
      (module_pproc_common_port_private_t *)port->port_private;
    chromatix_parms_type *chromatix_ptr;
    wavelet_denoise_type *wavelet_denoise;
    mct_list_t *p_list = NULL;

    p_list = mct_list_find_custom(module_ctrl->session_params, &session_id,
      module_pproc_common_match_mod_params_by_session);
    if (!p_list) {
      CDBG_ERROR("%s:%d] Error in finding feature params\n", __func__,
        __LINE__);
      return FALSE;
    }

    session_params = (module_pproc_common_session_params_t *)p_list->data;
    if (!session_params) {
      CDBG_ERROR("%s:%d failed session_params NULL\n", __func__, __LINE__);
      return FALSE;
    }

    mct_list_t *list_frame_params = mct_list_find_custom(
      port_priv->frame_params, &event_identity,
      module_pproc_common_find_identity);

    if (!list_frame_params) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }

    frame_params =
      (module_pproc_common_frame_params_t *)list_frame_params->data;
    if (stats_update->flag & STATS_UPDATE_AEC) {
      port_priv->aec_trigger_lux_idx = aec_update->lux_idx;
      port_priv->aec_trigger_real_gain = aec_update->real_gain;
      CDBG("%s:%d aec trigger lux idx %f real gain %f\n", __func__, __LINE__,
        port_priv->aec_trigger_lux_idx, port_priv->aec_trigger_real_gain);
      module_pproc_common_wnr_interpolate(port_priv, frame_params);
      module_pproc_common_asf_interpolate(session_params, port_priv,
        frame_params);
    }
    break;
  }
  case MCT_EVENT_MODULE_SOF:
    break;
  case MCT_EVENT_MODULE_BUF_DIVERT: {
    pproc_interface_frame_divert_t new_frame;
    isp_buf_divert_t *buff_divert =
      (isp_buf_divert_t *)event->module_event_data;
    module_pproc_common_port_private_t *port_private = NULL;
    mct_list_t *list_frame_params = NULL;
    module_pproc_common_frame_params_t *port_frame_params = NULL;

    CDBG("%s:Divert buffer event\n", __func__);
    //print_buffer(buff_divert);

    /* isp_divert_buffer is same as buff_divert */
    if (sizeof(isp_buf_divert_t) != sizeof(pproc_intf_buff_data_t)) {
      CDBG("%s: Alert isp divert structure is having new members", __func__);
    }

    CDBG("%s:%d buf type %d\n", __func__, __LINE__, buff_divert->buffer.type);
    buff_divert->is_locked = 1;
    new_frame.isp_divert_buffer.native_buf = buff_divert->native_buf;
    new_frame.isp_divert_buffer.vaddr = buff_divert->vaddr;
    new_frame.isp_divert_buffer.fd = buff_divert->fd;
    new_frame.isp_divert_buffer.v4l2_buffer_obj = buff_divert->buffer;
    new_frame.isp_divert_buffer.is_locked = buff_divert->is_locked;
    new_frame.isp_divert_buffer.ack_flag = buff_divert->ack_flag;
    new_frame.isp_divert_buffer.identity = buff_divert->identity;
    new_frame.out_buff_idx = -1;
    /* used by divert frame to put back in v4l2 buffer */
    new_frame.frame_params.frame_id = buff_divert->buffer.sequence;
    new_frame.mct_event_identity = event_identity;

    CDBG("%s: Camcorder_CPP: frmid:%d, inidx:%d, outidx:%d,buffiden:%d, eveniden:%d\n",
      __func__, new_frame.frame_params.frame_id,
      new_frame.isp_divert_buffer.v4l2_buffer_obj.index, new_frame.out_buff_idx,
      new_frame.isp_divert_buffer.identity, new_frame.mct_event_identity);

    port_private = (module_pproc_common_port_private_t *)port->port_private;
    list_frame_params = mct_list_find_custom(
      port_private->frame_params, &event_identity,
      module_pproc_common_find_identity);
    if (!list_frame_params) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      break;
    }
    port_frame_params =
      (module_pproc_common_frame_params_t *)list_frame_params->data;
    if (!port_frame_params) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      break;
    }
    if (module_cpp_fill_input_params(module,
      &new_frame.frame_params, buff_divert, port_frame_params, event_identity,
      port) == FALSE) {
      CDBG_ERROR("%s:%d] error cannot process this frame\n", __func__,
        __LINE__);
      buff_divert->is_locked = 0;
      buff_divert->is_buf_dirty = 1;
    } else {
      /* Create frame info based on input frame config */
      /* Queue to driver to process frame */
      module_ctrl->pproc_iface->lib_params->func_tbl->process
        (module_ctrl->pproc_iface->lib_params->lib_private_data,
         PPROC_IFACE_FRAME_DIVERT, (void *)&new_frame);
    }
    break;
  }
  case MCT_EVENT_MODULE_FRAME_IND:
   /* process frame */
   module_ctrl->pproc_iface->lib_params->func_tbl->process
     (module_ctrl->pproc_iface->lib_params->lib_private_data,
       PPROC_IFACE_PROCESS_FRAME, event->module_event_data);
   break;
  case MCT_EVENT_MODULE_FRAME_DONE:
    CDBG("%s:Should not come here\n", __func__);
    module_ctrl->pproc_iface->lib_params->func_tbl->process
      (module_ctrl->pproc_iface->lib_params->lib_private_data,
        PPROC_IFACE_FRAME_DONE, event->module_event_data);
    break;
  case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
    mct_stream_info_t *stream_info = NULL;
    module_pproc_common_port_private_t *port_private = NULL;
    mct_list_t *list_frame_params = NULL;
    module_pproc_common_frame_params_t *frame_params = NULL;
    stream_info = (mct_stream_info_t *)event->module_event_data;
    if (!stream_info) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      break;
    }
    CDBG("%s:%d ide %d width %d height %d\n", __func__, __LINE__,
      event_identity, stream_info->dim.width, stream_info->dim.height);
    port_private = (module_pproc_common_port_private_t *)port->port_private;
    list_frame_params = mct_list_find_custom(port_private->frame_params,
      &event_identity, module_pproc_common_find_identity);
    if (!list_frame_params) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      break;
    }
    frame_params =
      (module_pproc_common_frame_params_t *)list_frame_params->data;
    if (!frame_params) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      break;
    }
    /* Update src width and src height */
    frame_params->src_width = stream_info->dim.width;
    frame_params->src_height = stream_info->dim.height;
    /* Initialize isp stream crop params */
    frame_params->stream_crop.x = 0;
    frame_params->stream_crop.y = 0;
    frame_params->stream_crop.crop_out_x = stream_info->dim.width;
    frame_params->stream_crop.crop_out_y = stream_info->dim.height;
    /* Initialize DIS / EIS crop params */
    frame_params->is_crop.x = 0;
    frame_params->is_crop.y = 0;
    frame_params->is_crop.width = stream_info->dim.width;
    frame_params->is_crop.height = stream_info->dim.height;
    frame_params->src_stride =
      stream_info->buf_planes.plane_info.mp[0].stride;
    frame_params->src_scanline =
      stream_info->buf_planes.plane_info.mp[0].scanline;
    CDBG("%s:%d frame params width %d height %d\n", __func__, __LINE__,
      frame_params->src_width, frame_params->src_height);
    break;
  }
  case MCT_EVENT_MODULE_STREAM_CROP: {
    mct_bus_msg_stream_crop_t *stream_crop = NULL;
    module_pproc_common_port_private_t *port_private = NULL;
    mct_list_t *list_frame_params = NULL;
    module_pproc_common_frame_params_t *frame_params = NULL;
    stream_crop = (mct_bus_msg_stream_crop_t *)event->module_event_data;
    if (!stream_crop) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      break;
    }
    port_private = (module_pproc_common_port_private_t *)port->port_private;
    list_frame_params = mct_list_find_custom(port_private->frame_params,
      &event_identity, module_pproc_common_find_identity);
    if (!list_frame_params) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      break;
    }
    frame_params =
      (module_pproc_common_frame_params_t *)list_frame_params->data;
    if (!frame_params) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      break;
    }
    /* Update src width and src height */
    frame_params->stream_crop = *stream_crop;
    CDBG("%s:%d isp zoom x %d y %d w %d h %d\n", __func__, __LINE__,
      frame_params->stream_crop.x, frame_params->stream_crop.y,
      frame_params->stream_crop.crop_out_x,
      frame_params->stream_crop.crop_out_y);
    break;
  }
  case MCT_EVENT_MODULE_STATS_DIS_UPDATE: {
    is_update_t *is_crop = NULL;
    module_pproc_common_port_private_t *port_private = NULL;
    mct_list_t *list_frame_params = NULL;
    module_pproc_common_frame_params_t *frame_params = NULL;
    is_crop = (is_update_t *)event->module_event_data;
    if (!is_crop) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      break;
    }
    port_private = (module_pproc_common_port_private_t *)port->port_private;
    list_frame_params = mct_list_find_custom(port_private->frame_params,
      &event_identity, module_pproc_common_find_identity);
    if (!list_frame_params) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      break;
    }
    frame_params =
      (module_pproc_common_frame_params_t *)list_frame_params->data;
    if (!frame_params) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      break;
    }
    CDBG("%s:%d dis / eis zoom x %d y %d w %d h %d\n", __func__, __LINE__,
      frame_params->is_crop.x, frame_params->is_crop.y,
      frame_params->is_crop.width, frame_params->is_crop.height);
    /* Sanity check */
    if ((is_crop->x + frame_params->dst_width > frame_params->src_width) ||
        (is_crop->y + frame_params->dst_height > frame_params->src_height)) {
      CDBG_ERROR("%s:%d invalid params crop x*y %d*%d src w*h %d*%d dst w*h \
%d*%d\n", __func__, __LINE__, is_crop->x, is_crop->y, frame_params->src_width,
        frame_params->src_height, frame_params->dst_width,
        frame_params->dst_height);
        return FALSE;
    }
    /* Update src width and src height */
    frame_params->is_crop = *is_crop;
    break;
  }
  default:
    break;
  }
  return TRUE;
}

/** module_pproc_common_process_event:
 *    @module: mct module handle
 *    @event: event to be processed
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function handles all events and sends those events
 *  downstream / upstream.
 **/
boolean module_pproc_common_process_event(mct_module_t *module,
  mct_event_t *event)
{
  module_pproc_common_ctrl_t *module_ctrl = NULL;

  CDBG("%s:%d module event id %d\n", __func__, __LINE__, event->type);

  if (!event || !module) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  module_ctrl = (module_pproc_common_ctrl_t *)module->module_private;

  if (MCT_EVENT_CONTROL_CMD == event->type) {
    module_pproc_common_process_control_event(module, &event->u.ctrl_event,
      event->identity, NULL);
  } else if (MCT_EVENT_MODULE_EVENT == event->type) {
    module_pproc_common_process_module_event(module, &event->u.module_event,
      event->identity, NULL);
  }

  return TRUE;
}

/** module_pproc_common_process_event:
 *    @module: mct module handle
 *    @event: event to be processed
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function handles all events and sends those events
 *  downstream / upstream.
 **/
boolean module_pproc_common_private_event(mct_module_t *module,
  mct_port_t *port, void *data)
{
  module_pproc_common_ctrl_t *module_ctrl = NULL;
  mct_event_t *event = (mct_event_t *)data;

  CDBG("%s:%d module event id %d\n", __func__, __LINE__, event->type);

  if (!event || !module) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  module_ctrl = (module_pproc_common_ctrl_t *)module->module_private;

  if (MCT_EVENT_CONTROL_CMD == event->type) {
    module_pproc_common_process_control_event(module, &event->u.ctrl_event,
      event->identity, port);
  } else if (MCT_EVENT_MODULE_EVENT == event->type) {
    module_pproc_common_process_module_event(module, &event->u.module_event,
      event->identity, port);
  }

  return TRUE;
}

/** module_pproc_common_set_mod:
 *    @module: pproc submodule
 *    @module_type: SINK, SOURCE, INDEXABLE
 *    @identity: identity to which module type is linked.
 *
 *  This function links the module type particular to a session.
 *
 *  Return: TRUE if session is started sucessfully. FALSE if
 *  error is encountered.
 *
 *  This function is called in mediacontroller context.
 **/
void module_pproc_common_set_mod(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  CDBG("%s:%d Enter\n", __func__, __LINE__);
  module->type = (mct_module_type_t)module_type;
  mct_module_set_process_event_func(module,
    module_pproc_common_process_event);
  CDBG("%s:%d Exit\n", __func__, __LINE__);
  return;
}

/** module_pproc_common_query_mod:
 *    @module: mct module pointer for cpp
 *    @query_buf: pointer to module_pproc_common_query_caps_t
 *            struct
 *    @sessionid: session id
 *
 *  Return: 0 for success and negative error for failure
 *
 *  This function handles query module events to return
 *  information requested by mct any stream is created
 **/
boolean module_pproc_common_query_mod(mct_module_t *module, void *query_buf,
  unsigned int sessionid)
{
  int32_t idx = 0, rc = PPROC_SUCCESS;
  mct_pipeline_pp_cap_t            *pproc_caps = NULL;

  CDBG("%s:%d E\n", __func__, __LINE__);
  if (!query_buf || !module) {
    CDBG_ERROR("%s:%d failed query_buf %p module %p\n", __func__, __LINE__,
      query_buf, module);
    return FALSE;
  }

  pproc_caps = (mct_pipeline_pp_cap_t *)query_buf;
  pproc_caps->height_padding = CAM_PAD_NONE;
  pproc_caps->plane_padding = CAM_PAD_NONE;
  pproc_caps->width_padding = CAM_PAD_NONE;
  pproc_caps->min_num_pp_bufs = 2;
  pproc_caps->min_required_pp_mask = CAM_QCOM_FEATURE_SHARPNESS;
  pproc_caps->feature_mask |= (CAM_QCOM_FEATURE_DENOISE2D |
    CAM_QCOM_FEATURE_CROP | CAM_QCOM_FEATURE_CPP | CAM_QCOM_FEATURE_FLIP);
  CDBG("%s:%d hp %d wp %d pp %d min pp buf %d mask %x, %x\n", __func__,
    __LINE__, pproc_caps->height_padding, pproc_caps->width_padding,
    pproc_caps->plane_padding, pproc_caps->min_num_pp_bufs,
    pproc_caps->min_required_pp_mask, pproc_caps->feature_mask);
  CDBG("%s:%d X\n", __func__, __LINE__);
  return rc;
}

/** module_pproc_destroy_port:
 *    @data: pointer to list data(mct_port_t)
 *    @user_data: pointer to module (mct_module_t)
 *
 *  This function removes port out of module and destroys it
 *
 *  This function executes in callers' context
 *
 *  Return:
 *  Success: TRUE
 *  Failure: FALSE
 **/
boolean module_pproc_destroy_port(void *data, void *user_data)
{
  boolean       ret = FALSE;
  mct_port_t   *port = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;
  if (!port || !module) {
    CDBG_ERROR("%s:%d failed, port %p module %p\n", __func__, __LINE__, port,
      module);
    /* return TRUE since we need to continue traversing */
    return TRUE;
  }
  ret = mct_module_remove_port(module, port);
  if (ret == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
  }

  if (port->port_private) {
    free(port->port_private);
  }

  mct_port_destroy(port);
  return TRUE;
}

/** module_pproc_create_port:
 *    @module: pointer to module (mct_module_t)
 *    @name: port name
 *    @direction: port's direction (MCT_PORT_SINK /
 *              MCT_PORT_SRC)
 *
 *  This function creates mct port and adds it to module
 *
 *  This function executes in callers' context
 *
 *  Return:
 *  Success: TRUE
 *  Failure: FALSE
 **/
static boolean module_pproc_create_port(mct_module_t *module,
  const char *name, mct_port_direction_t direction,
  cam_streaming_mode_t mode)
{
  int32_t                      rc = 0;
  mct_port_t                  *port = NULL;
  module_pproc_common_port_private_t *port_private = NULL;

  port = mct_port_create(name);
  if (!port) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  if (port_cpp_init(port, direction) == FALSE ||
      mct_module_add_port(module, port) == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR1;
  }

  port_private = malloc(sizeof(module_pproc_common_port_private_t));
  if (!port_private) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR2;
  }
  memset(port_private, 0, sizeof(module_pproc_common_port_private_t));

  port_private->streaming_mode = mode;
  port->port_private = (void *)port_private;

  return TRUE;

ERROR2:
  mct_module_remove_port(module, port);
ERROR1:
  mct_port_destroy(port);
  return FALSE;
}

/** module_pproc_common_create_default_ports:
 *    @mod: cpp module
 *
 *  By default, pproc submodule creates one sink port and one
 *  source port. It supports dynamically added new ports through
 *  module_cpp_request_new_port.
 *
 *  This function executes in callers' context
 *
 *  Return Success if all ports are created else Failure
 **/
boolean module_pproc_common_create_default_ports(mct_module_t *mod)
{
  int32_t rc = 0;
  char    name[20];

  /* Create sink port 0 */
  snprintf(name, sizeof(name), "sink0");
  rc = module_pproc_create_port(mod, name, MCT_PORT_SINK,
    CAM_STREAMING_MODE_CONTINUOUS);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR1;
  }

  /* Create sink port 1 */
  snprintf(name, sizeof(name), "sink1");
  rc = module_pproc_create_port(mod, name, MCT_PORT_SINK,
    CAM_STREAMING_MODE_BURST);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR1;
  }

  /* Create src port 0 */
  snprintf(name, sizeof(name), "src0");
  rc = module_pproc_create_port(mod, name, MCT_PORT_SRC,
    CAM_STREAMING_MODE_CONTINUOUS);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR2;
  }

  /* Create src port 1 */
  snprintf(name, sizeof(name), "src1");
  rc = module_pproc_create_port(mod, name, MCT_PORT_SRC,
    CAM_STREAMING_MODE_BURST);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR2;
  }

  return TRUE;

ERROR2:
  mct_list_traverse(mod->srcports, module_pproc_destroy_port, mod);
ERROR1:
  mct_list_traverse(mod->sinkports, module_pproc_destroy_port, mod);
  return FALSE;
}

/** module_pproc_common_start_session:
 *    @module: pproc submodule
 *    @sessionid: ID of session to be started.
 *
 *  This function is called when sessionis started.
 *
 *  Return: TRUE if session is started sucessfully. FALSE if
 *  error is encountered.
 *
 *  This function is called in mediacontroller context.
 **/
boolean module_pproc_common_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  module_pproc_common_ctrl_t            *pproc_ctrl = NULL;
  module_pproc_common_session_params_t *session_params;
  boolean                     ret = FALSE;
  pproc_client_callback_t     client_callback;

  if (!module || !module->module_private) {
    CDBG_ERROR("%s: module is null\n", __func__);
    return FALSE;
  }
  pproc_ctrl =
    (module_pproc_common_ctrl_t *)module->module_private;
  CDBG("%s: sessionid = 0x%x\n", __func__, sessionid);
  /* Create a buffer manager client interface */

  session_params =
    malloc(sizeof(module_pproc_common_session_params_t));
  if (!session_params) {
    CDBG_ERROR("%s:%d feature params malloc failed\n", __func__, __LINE__);
    return FALSE;
  }

  memset(session_params, 0,
    sizeof(module_pproc_common_session_params_t));
  session_params->sessionid = sessionid;

  pproc_ctrl->session_params = mct_list_append(pproc_ctrl->session_params,
    session_params, NULL, NULL);

  if (!pproc_ctrl->pproc_iface->ref_count) {
    pproc_ctrl->buff_mgr_client = pproc_common_buff_mgr_get_client();
    if (!pproc_ctrl->buff_mgr_client) {
      CDBG_ERROR("%s: Error getting buffer manager client\n", __func__);
      return FALSE;
    }

    /* open library */
    ret = pproc_common_load_library(pproc_ctrl->pproc_iface, "cpp");
    if (ret < 0) {
      CDBG("%s:%d pproc_common_load_library failed rc %d\n",
        __func__, __LINE__, ret);
      return ret;
    }

    ret = pproc_ctrl->pproc_iface->lib_params->func_tbl->open(
      &pproc_ctrl->pproc_iface->lib_params->lib_private_data);
    if (ret < 0) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }

    client_callback.create_frame = module_cpp_create_frame;
    client_callback.framedone_ack = module_cpp_frame_done;
    client_callback.data = (void *)module;

    pproc_ctrl->pproc_iface->lib_params->func_tbl->process(
      pproc_ctrl->pproc_iface->lib_params->lib_private_data,
      PPROC_IFACE_SET_CALLBACK, (void *)&client_callback);
    if (ret < 0) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }
  }
  pproc_ctrl->pproc_iface->ref_count++;

  CDBG_ERROR("%s: Exit sessionid = 0x%x\n", __func__, sessionid);
  return TRUE;
}

boolean module_pproc_common_match_mod_params_by_session(void *list_data,
  void *user_data)
{
  uint32_t *sessionid = (uint32_t *)user_data;
  module_pproc_common_session_params_t *session_params =
    (module_pproc_common_session_params_t *)list_data;

  if (!sessionid || !session_params) {
    CDBG_ERROR("%s:%d] Invalid Input params\n", __func__, __LINE__);
    return FALSE;
  }

  if (*sessionid == session_params->sessionid) {
    CDBG("%s: sessionid:%d match in list_data:%p\n", __func__,
      *sessionid, list_data);
    return TRUE;
  }

  return FALSE;
}

/** module_pproc_common_stop_session:
 *    @module: pproc submodule
 *    @sessionid: ID of session to be stopped.
 *
 *  Return: TRUE if session is closed sucessfully. FALSE if
 *  error is encountered.
 *
 *  This function is called when session is to be stopped.
 **/
boolean module_pproc_common_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  module_pproc_common_ctrl_t *pproc_ctrl = NULL;
  module_pproc_common_session_params_t *session_params;
  mct_list_t *list_node;

  if (!module || !module->module_private) {
    CDBG_ERROR("%s: module is null\n", __func__);
    return FALSE;
  }

  pproc_ctrl = (module_pproc_common_ctrl_t *)module->module_private;

  list_node = mct_list_find_custom(pproc_ctrl->session_params, &sessionid,
    module_pproc_common_match_mod_params_by_session);
  if (!list_node) {
    CDBG_ERROR("%s:%d] Error in finding feature params\n", __func__,
      __LINE__);
  } else {
    session_params =
      (module_pproc_common_session_params_t *)list_node->data;
    pproc_ctrl->session_params = mct_list_remove(pproc_ctrl->session_params,
      session_params);
    free(session_params);
  }

  if (!pproc_ctrl->pproc_iface->ref_count) {
    CDBG_ERROR("%s: mismatch refcount\n", __func__);
    return FALSE;
  }

  pproc_ctrl->pproc_iface->ref_count--;
  if (!pproc_ctrl->pproc_iface->ref_count) {
    pproc_common_buff_mgr_put_client(pproc_ctrl->buff_mgr_client);
    pproc_ctrl->buff_mgr_client = NULL;
    pproc_ctrl->pproc_iface->lib_params->func_tbl->close(
      pproc_ctrl->pproc_iface->lib_params->lib_private_data);
    pproc_common_unload_library(pproc_ctrl->pproc_iface);
  }

  CDBG("%s: sessionid = 0x%x\n", __func__, sessionid);
  return TRUE;
}

/** module_pproc_common_request_new_port:
 *    @stream_info: pointer to stream info
 *    @direction: source / sink
 *    @module: pointer to pproc submodule
 *
 *  This function creates mct_port_t dynamically.
 *
 *  This function executes in callers' context
 *
 *  Return Success else Failure if port cannot be created.
 **/
mct_port_t *module_pproc_common_request_new_port(void *stream_info,
  mct_port_direction_t direction, mct_module_t *module,
  void *peer_caps)
{
  return NULL;
}

/** module_pproc_common_create_submod:
 *    @name: pproc submodule name
 *
 *  This function creates mct_module_t for pproc submodule,
 *  creates port, fills capabilities and add them to the
 *  submodule
 *
 *  This function executes in callers' context
 *
 *  Return:
 *  Success - mct_module_t pointer corresponding to pproc
 *  submodule Failure - NULL in case of error / submod library
 *  is not found
 **/
mct_module_t *module_pproc_common_create_submod(const char *name)
{
  mct_module_t               *module = NULL;
  int32_t                     rc = 0;
  module_pproc_common_ctrl_t *mod_ctrl = NULL;
  pproc_interface_t          *pproc_iface = NULL;

  /* Create interface structure for pproc submodule library */
  pproc_iface = malloc(sizeof(pproc_interface_t));
  if (!pproc_iface) {
    CDBG_ERROR("%s:%d malloc failed\n", __func__, __LINE__);
    return NULL;
  }

  memset(pproc_iface, 0, sizeof(pproc_interface_t));

  /* check if cpp library is loaded successfully */
  if (pproc_common_load_library(pproc_iface, "cpp") < 0) {
    CDBG_ERROR("%s:%d submodule not supported\n", __func__, __LINE__);
    goto ERROR1;
  }

  /* OPEN call success: If supported, subdev is opened successfully.
     When no subdev is supported then appropriate sw library is opened
     successfully */
  rc = pproc_iface->lib_params->func_tbl->open(
     &pproc_iface->lib_params->lib_private_data);
  if (rc < 0) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR1;
  }

  /* create mct module for pproc submodule */
  module = mct_module_create(name);
  if (!module) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR2;
  }

  /* create module private data */
  mod_ctrl = malloc(sizeof(module_pproc_common_ctrl_t));
  if (!mod_ctrl) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR3;
  }
  memset(mod_ctrl, 0, sizeof(module_pproc_common_ctrl_t));

  pthread_mutex_init(&mod_ctrl->mutex, NULL);
  mod_ctrl->pproc_iface = pproc_iface;
  mod_ctrl->pproc_event_func = module_pproc_common_private_event;
  module->module_private = (void *)mod_ctrl;

  /* Fill function table of pproc submodule */
  mct_module_set_start_session_func(module, module_pproc_common_start_session);
  mct_module_set_stop_session_func(module, module_pproc_common_stop_session);
  mct_module_set_set_mod_func(module, module_pproc_common_set_mod);
  mct_module_set_query_mod_func(module, module_pproc_common_query_mod);
  mct_module_set_request_new_port_func(module,
    module_pproc_common_request_new_port);

  /* Create default ports for pproc submodule */
  if (module_pproc_common_create_default_ports(module) == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR4;
  }

  pproc_iface->lib_params->func_tbl->process(
    pproc_iface->lib_params->lib_private_data, PPROC_IFACE_INIT, NULL);

  /* unload the library for now */
  pproc_iface->lib_params->func_tbl->close(
    pproc_iface->lib_params->lib_private_data);
  pproc_common_unload_library(pproc_iface);
  CDBG("%s:%d X\n", __func__, __LINE__);
  return module;

ERROR4:
  free(mod_ctrl);
  mct_list_free_all(module->sinkports, NULL);
  mct_list_free_all(module->srcports, NULL);
ERROR3:
  mct_module_destroy(module);
ERROR2:
  pproc_iface->lib_params->func_tbl->close(
    pproc_iface->lib_params->lib_private_data);
ERROR1:
  pproc_common_unload_library(pproc_iface);
  free(pproc_iface);
  return NULL;
}
