/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "cac44.h"

#ifdef ENABLE_CAC_LOGGING
  #undef CDBG
  #define CDBG ALOGE
#endif

#define CAC_START_REG 0x02a4

/** cac_reset:
 *
 *    @mod:
 *
 **/
static void cac_reset(isp_cac_mod_t *mod)
{
}

/** cac_init:
 *
 *    @mod_ctrl:
 *    @in_params:
 *    @notify_ops:
 *
 * init cac module
 *
 **/
static int cac_init(void *mod_ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  return 0;
} /* cac_init */

/** cac_config:
 *
 *    @mod:
 *    @pix_setting:
 *    @in_param_size:
 *
 **/
static int cac_config(isp_cac_mod_t *mod,
  isp_hw_pix_setting_params_t *pix_setting, uint32_t in_param_size)
{
  return 0;
} /* cac_config */

/** cac_destroy:
 *
 *    @mod_ctrl:
 *
 **/
static int cac_destroy(void *mod_ctrl)
{
  return 0;
} /* cac_destroy */

/** cac_enable:
 *
 *    @mod:
 *    @enable:
 *    @in_param_size:
 *
 **/
static int cac_enable(isp_cac_mod_t *mod,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  return 0;
} /* cac_enable */

/** cac_trigger_enable:
 *
 *    @mod:
 *    @enable:
 *    @in_param_size:
 *
 * enable abf trigger update feature
 *
 **/
static int cac_trigger_enable(isp_cac_mod_t *mod,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  return 0;
} /* cac_trigger_enable */

/** cac_trigger_update:
 *
 *    @mod:
 *    @trigger_params:
 *    @in_param_size:
 *
 **/
static int cac_trigger_update(isp_cac_mod_t *mod,
  isp_pix_trigger_update_input_t *trigger_params,
  uint32_t in_param_size)
{
  return 0;
} /* cac_trigger_update */

/** cac_set_params:
 *
 *    @mod_ctrl:
 *    @param_id:
 *    @in_params:
 *
 **/
static int cac_set_params(void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  return 0;
} /* cac_set_params */

/** cac_get_params:
 *
 *    @mod_ctrl:
 *    @param_id:
 *    @in_params:
 *    @in_param_size:
 *    @out_params:
 *    @out_param_size:
 *
 **/
static int cac_get_params(void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size,
  void *out_params, uint32_t out_param_size)
{
  return 0;
} /* cac_get_params */

/** cac_do_hw_update:
 *
 *    @cac_mod:
 *
 * update module register to kernel
 *
 **/
static int cac_do_hw_update(isp_cac_mod_t *cac_mod)
{
  return 0;
} /* cac_hw_reg_update */

/** cac_action:
 *
 *    @mod_ctrl:
 *    @action_code
 *    @data:
 *    @data_size:
 *
 * processing the action
 *
 **/
static int cac_action(void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  return 0;
} /* cac_action */

/** cac44_open:
 *
 *    @version:
 *
 * open cac44 mod, create func table
 *
 **/
isp_ops_t *cac44_open(uint32_t version)
{
  isp_cac_mod_t *mod = malloc(sizeof(isp_cac_mod_t));

  CDBG("%s: E", __func__);

  if (!mod) {
    CDBG_ERROR("%s: fail to allocate memory",  __func__);
    return NULL;
  }

  memset(mod,  0,  sizeof(isp_cac_mod_t));

  mod->ops.ctrl = (void *)mod;
  mod->ops.init = cac_init;
  mod->ops.destroy = cac_destroy;
  mod->ops.set_params = cac_set_params;
  mod->ops.get_params = cac_get_params;
  mod->ops.action = cac_action;

  return &mod->ops;
} /* cac44_open */
