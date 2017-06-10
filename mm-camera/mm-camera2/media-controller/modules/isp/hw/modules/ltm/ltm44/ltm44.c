/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "ltm44.h"

#ifdef ENABLE_LTM_LOGGING
  #undef CDBG
  #define CDBG ALOGE
#endif

#define LTM_START_REG 0x02a4

/** ltm_reset:
 *
 *    @mod:
 *
 **/
static void ltm_reset(isp_ltm_mod_t *mod)
{
}

/** ltm_init:
 *
 *    @mod_ctrl:
 *    @in_params:
 *    @notify_ops:
 *
 * init ltm module
 *
 **/
static int ltm_init(void *mod_ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  return 0;
} /* ltm_init */

/** ltm_config:
 *
 *    @mod:
 *    @pix_setting:
 *    @in_param_size:
 *
 **/
static int ltm_config(isp_ltm_mod_t *mod,
  isp_hw_pix_setting_params_t *pix_setting, uint32_t in_param_size)
{
  return 0;
} /* ltm_config */

/** ltm_destroy:
 *
 *    @mod_ctrl:
 *
 **/
static int ltm_destroy(void *mod_ctrl)
{
  return 0;
} /* ltm_destroy */

/** ltm_enable:
 *
 *    @mod:
 *    @enable:
 *    @in_param_size:
 *
 **/
static int ltm_enable(isp_ltm_mod_t *mod,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  return 0;
} /* ltm_enable */

/** ltm_trigger_enable:
 *
 *    @mod:
 *    @enable:
 *    @in_param_size:
 *
 * enable abf trigger update feature
 *
 **/
static int ltm_trigger_enable(isp_ltm_mod_t *mod,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  return 0;
} /* ltm_trigger_enable */

/** ltm_trigger_update:
 *
 *    @mod:
 *    @trigger_params:
 *    @in_param_size:
 *
 **/
static int ltm_trigger_update(isp_ltm_mod_t *mod,
  isp_pix_trigger_update_input_t *trigger_params,
  uint32_t in_param_size)
{
  return 0;
} /* ltm_trigger_update */

/** ltm_set_params:
 *
 *    @mod_ctrl:
 *    @param_id:
 *    @in_params:
 *
 **/
static int ltm_set_params(void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  return 0;
} /* ltm_set_params */

/** ltm_get_params:
 *
 *    @mod_ctrl:
 *    @param_id:
 *    @in_params:
 *    @in_param_size:
 *    @out_params:
 *    @out_param_size:
 *
 **/
static int ltm_get_params(void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size,
  void *out_params, uint32_t out_param_size)
{
  return 0;
} /* ltm_get_params */

/** ltm_do_hw_update:
 *
 *    @ltm_mod:
 *
 * update module register to kernel
 *
 **/
static int ltm_do_hw_update(isp_ltm_mod_t *ltm_mod)
{
  return 0;
} /* ltm_hw_reg_update */

/** ltm_action:
 *
 *    @mod_ctrl:
 *    @action_code
 *    @data:
 *    @data_size:
 *
 * processing the action
 *
 **/
static int ltm_action(void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  return 0;
} /* ltm_action */

/** ltm44_open:
 *
 *    @version:
 *
 * open ltm44 mod, create func table
 *
 **/
isp_ops_t *ltm44_open(uint32_t version)
{
  isp_ltm_mod_t *mod = malloc(sizeof(isp_ltm_mod_t));

  CDBG("%s: E", __func__);

  if (!mod) {
    CDBG_ERROR("%s: fail to allocate memory",  __func__);
    return NULL;
  }

  memset(mod,  0,  sizeof(isp_ltm_mod_t));

  mod->ops.ctrl = (void *)mod;
  mod->ops.init = ltm_init;
  mod->ops.destroy = ltm_destroy;
  mod->ops.set_params = ltm_set_params;
  mod->ops.get_params = ltm_get_params;
  mod->ops.action = ltm_action;

  return &mod->ops;
} /* ltm44_open */
