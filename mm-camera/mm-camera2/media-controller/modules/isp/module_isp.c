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
#include "isp_ops.h"
#include "isp_hw.h"
#include "isp.h"
#include "isp_log.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#if MODULE_ISP_DEBUG
#undef CDBG
#define CDBG ALOGE
#endif

extern int port_isp_create_ports(isp_t *isp);

/** module_isp_process_event_func
*    @module: mct module instance
*    @event : mct event
*
*  This method processes module events: not used
*
*  Returns 0 always
**/
static boolean  module_isp_process_event_func(mct_module_t *module,
  mct_event_t *event)
{
  ISP_DBG(ISP_MOD_COM,"%s: E, event type %d direction %d\n", __func__,
       event->type, event->direction);

  ISP_DBG(ISP_MOD_COM,"%s: Xn", __func__);
  return 0;
}

/** module_isp_send_event_func
*    @module: mct module instance
*    @event : pointer to mct event
*
*  This method is to send events: not used
*
*  Returns 0 always
**/
static boolean  module_isp_send_event_func(mct_module_t *module,
  mct_event_t *event)
{
  ISP_DBG(ISP_MOD_COM,"%s: E, event type %d direction %d\n", __func__,
       event->type, event->direction);

  ISP_DBG(ISP_MOD_COM,"%s: X\n", __func__);
  return 0;
}

/** module_isp_set_mod_func
*    @module: mct module instance
*    @module_type : type of module represented by struct
*                 mct_module_type_t
*    @identity : contains session id and steram id
*
*  This method is to set information about a module: not used
*
*  Returns nothing
**/
static void module_isp_set_mod_func(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  ISP_DBG(ISP_MOD_COM,"%s: E, module_type %d\n", __func__, module_type);

  ISP_DBG(ISP_MOD_COM,"%s: X\n", __func__);
  return;
}

/** module_isp_query_mod_func
*    @module: mct module instance
*    @query_buf : query capabilities data
*    @sessionid : current session id
*
*  This method populates isp capabilities such as max zoom and
*  supported color effects
*
*  This function executes in Imaging Server context
*
*  Returns TRUE in case of success
**/
static boolean  module_isp_query_mod_func(mct_module_t *module,
  void *query_buf, unsigned int sessionid)
{
  int32_t rc = 0;
  mct_pipeline_isp_cap_t *isp_cap = NULL;
  mct_pipeline_cap_t *cap_buf = (mct_pipeline_cap_t *)query_buf;
  isp_t *isp;
  int num;

  ISP_DBG(ISP_MOD_COM,"%s: E, sessionid %d\n", __func__, sessionid);
  if (!query_buf || !module) {
    CDBG_ERROR("%s:%d failed query_buf %p s_module %p\n", __func__, __LINE__,
        query_buf, module);
    return FALSE;
  }

  isp_cap = &cap_buf->isp_cap;
  isp_cap->zoom_ratio_tbl_cnt = MAX_ZOOMS_CNT;

  isp = (isp_t*)module->module_private;
  num = isp_cap->zoom_ratio_tbl_cnt;
  rc = isp_zoom_get_ratio_table(isp->data.zoom, &num,
      isp_cap->zoom_ratio_tbl);
  isp_cap->zoom_ratio_tbl_cnt = (uint8_t)num;

  /* populate supported color efects*/

  isp_cap->supported_effects_cnt = 9;
  isp_cap->supported_effects[0] = CAM_EFFECT_MODE_OFF;
  isp_cap->supported_effects[1] = CAM_EFFECT_MODE_MONO;
  isp_cap->supported_effects[2] = CAM_EFFECT_MODE_NEGATIVE;
  isp_cap->supported_effects[3] = CAM_EFFECT_MODE_SOLARIZE;
  isp_cap->supported_effects[4] = CAM_EFFECT_MODE_SEPIA;
  isp_cap->supported_effects[5] = CAM_EFFECT_MODE_POSTERIZE;
  isp_cap->supported_effects[6] = CAM_EFFECT_MODE_WHITEBOARD;
  isp_cap->supported_effects[7] = CAM_EFFECT_MODE_BLACKBOARD;
  isp_cap->supported_effects[8] = CAM_EFFECT_MODE_AQUA;
  isp_cap->low_power_mode_supported =
    (isp->data.sd_info.sd_info[0].cap.hw_ver == ISP_MSM8909);
  isp_cap->use_pix_for_SOC = isp->res_mgr->vfe_info[0].use_pix_for_SOC;
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d\n", __func__, rc);
  return rc;
}

/** module_isp_start_session
*    @module: mct module instance
*    @sessionid : session id to be started
*
*  Starts specified session
*
*  This function executes in Imaging Server context
*
*  Returns TRUE in case of success
**/
static boolean module_isp_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  boolean rc = FALSE;
  int ret = 0;
  isp_t *isp = module->module_private;

  ISP_DBG(ISP_MOD_COM,"%s: E, module->module_private= %p, sessionid %d \n", __func__,
       module->module_private, sessionid);
  pthread_mutex_lock(&isp->mutex);
  ret = isp_start_session(isp, sessionid);
  pthread_mutex_unlock(&isp->mutex);
  rc = (ret == 0)? TRUE : FALSE;
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d\n", __func__, rc);
  return rc;
}

/** module_isp_stop_session:
 *    @module: mct module instance
 *    @sessionid: session id to be stopped
 *
 * Stops specified session
 *
 * This function executes in Imaging Server context
 *
 * Returns TRUE in case of success
 **/
static boolean module_isp_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  boolean rc = FALSE;
  int ret = 0;
  isp_t *isp = module->module_private;

  ISP_DBG(ISP_MOD_COM,"%s: E, module->module_private= %p, sessionid %d \n", __func__,
       module->module_private,sessionid);
  pthread_mutex_lock(&isp->mutex);
  ret = isp_stop_session(isp, sessionid);
  pthread_mutex_unlock(&isp->mutex);
  rc = (ret == 0)? TRUE : FALSE;

  ISP_DBG(ISP_MOD_COM,"%s: X rc = %d", __func__, rc);
  return rc;
}

/** module_isp_init:
 *    @name: name of this ISP interface module ("ISP")
 *
 * Initializes new instance of ISP module
 *
 * This function executes in Imaging Server context
 *
 * Returns new instance on success or NULL on fail
 **/
mct_module_t *module_isp_init(const char *name)
{
  int rc = 0;
  mct_module_t *module_isp = NULL;
  isp_t         *isp = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: E, name= %s\n", __func__, name);

  /* Create MCT module for ISP */
  module_isp = mct_module_create(name);
  if (!module_isp) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return NULL;
  }
  /* initialize isp ops table */
  rc = isp_create(&isp);
  if (rc != 0 || NULL == isp) {
    CDBG_ERROR("%s: isp_open error = %d\n", __func__, rc);
    goto error;
  }
  isp->module = module_isp;

  /* create isp sink ports and src ports */
  if (0 != (rc = port_isp_create_ports(isp))) {
    CDBG_ERROR("%s: create sink port error = %d", __func__, rc);
    goto error_ports;
  }

  /* overload function ptrs and save mct_module ptr in isp base */
  module_isp->process_event = module_isp_process_event_func;
  module_isp->set_mod = module_isp_set_mod_func;
  module_isp->query_mod = module_isp_query_mod_func;
  module_isp->start_session = module_isp_start_session;
  module_isp->stop_session = module_isp_stop_session;
  isp->module->module_private = isp;

  ISP_DBG(ISP_MOD_COM,"%s: X, isp->module %x\n", __func__, (unsigned int)isp->module);
  return isp->module;

error_ports:
  isp_destroy(isp);
error:
  mct_module_destroy(module_isp);
  return NULL;
}

/** module_isp_deinit:
 *    @mod: mct module instance
 *
 * Deinitializes new instance of ISP module
 *
 * This function executes in Imaging Server context
 *
 * Returns nothing
 **/
void module_isp_deinit(mct_module_t *mod)
{
  isp_t *isp = NULL;
  if (mod) {
      ISP_DBG(ISP_MOD_COM,"%s: E, mod->module_private= %p\n", __func__,
           mod->module_private);
      isp = (isp_t*)mod->module_private;
      if (isp){
        port_isp_destroy_ports(isp);
        mct_module_destroy(isp->module);
        isp_destroy(isp);
        mod->module_private = NULL;
      }
  }
  ISP_DBG(ISP_MOD_COM,"%s: X\n", __func__);
}
