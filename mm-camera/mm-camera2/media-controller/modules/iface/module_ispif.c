/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <assert.h>
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
#include "ispif.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef MODULE_ISPIF_DEBUG
#undef CDBG
#define CDBG ALOGE
#endif

/** module_ispif_process_event_func:
 *    @module: ispif MCTL Module object
 *    @event: MCTL event to module
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implement ispif module process event method - currently empty
 *
 *  Return: FALSE
 **/
static boolean  module_ispif_process_event_func(mct_module_t *module,
  mct_event_t *event)
{
  return FALSE;
}

/** module_ispif_send_event_func:
 *    @module: ispif MCTL Module object
 *    @event: MCTL event to module
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implement ispif module send event method - currently empty
 *
 *  Return: FALSE
 **/
static boolean  module_ispif_send_event_func(mct_module_t *module,
  mct_event_t *event)
{
  return FALSE;
}

/** module_ispif_set_mod_func:
 *    @module: ispif MCTL Module object
 *    @module_type: MCTL module type
 *    @identity: identity
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implement ispif module set mod method - currently empty
 *
 *  Return: None
 **/
static void module_ispif_set_mod_func(mct_module_t *module,
  unsigned int module_type,
  unsigned int identity)
{
  return;
}

/** module_ispif_query_mod_func:
 *    @module: ispif MCTL Module object
 *    @buf: buffer for querried info
 *    @sessionid: session ID
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implement ispif module query mod method - currently empty
 *
 *  Return: FALSE
 **/
static boolean  module_ispif_query_mod_func(mct_module_t *module, void *buf,
  unsigned int sessionid)
{
  return FALSE;
}

/** module_ispif_free_mod_func:
 *    @module: ispif MCTL Module object
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implement ispif module free mod method - currently empty
 *
 *  Return: None
 **/
static void module_ispif_free_mod_func(mct_module_t *module)
{
  return;
}

/** module_ispif_start_session:
 *    @module: ispif MCTL Module object
 *    @sessionid: session ID
 *
 *  This function runs in MCTL thread context.
 *
 *  This function starts ispif module session
 *
 *  Return: TRUE  - Success
 *          FALSE - Session not started
 **/
static boolean module_ispif_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  boolean rc = FALSE;
  ispif_t *ispif;
  int ret = 0;

  /* should not happen */
  assert(module != NULL);
  assert(module->module_private != NULL);

  ispif = module->module_private;

  pthread_mutex_lock(&ispif->mutex);
  ret = ispif_start_session(ispif, sessionid);
  pthread_mutex_unlock(&ispif->mutex);
  rc = (ret == 0)? TRUE : FALSE;

  return rc;
}

/** module_ispif_query_mod_func:
 *    @module: ispif MCTL Module object
 *    @sessionid: session ID
 *
 *  This function runs in MCTL thread context.
 *
 *  This function stops ispif module session
 *
 *  Return: TRUE  - Success
 *          FALSE - Error while stopping session
 **/
static boolean module_ispif_stop_session(
  mct_module_t *module, unsigned int sessionid)
{
  boolean rc = FALSE;
  int ret = 0;
  boolean trigger_dual_isp_restore = FALSE;
  ispif_t *ispif;

  /* should not happen */
  assert(module != NULL);
  assert(module->module_private != NULL);
  ispif = module->module_private;

  pthread_mutex_lock(&ispif->mutex);
  trigger_dual_isp_restore =
    ispif_need_restore_dual_isp_session(ispif, sessionid);
  ret = ispif_stop_session(ispif, sessionid);
  if (trigger_dual_isp_restore) {
  CDBG_HIGH("%s: Session %d closed. Restore dual isp session\n",
    __func__, sessionid);
  ispif_restore_dual_isp_session(ispif);
  }
  pthread_mutex_unlock(&ispif->mutex);
  rc = (ret == 0)? TRUE : FALSE;

  return rc;
}

/** module_ispif_query_mod_func:
 *    @name: name of initialized module
 *
 *  This function runs in MCTL thread context.
 *
 *  This function creates and initializes ispif module
 *
 *  Return: NULL  - Could not initialize module
 *          Otherwise - pointer to initialized module
 **/
mct_module_t *module_iface_init(const char *name)
{
  int rc = 0;
  ispif_caps_t caps;
  mct_module_t *module_ispif = NULL;
  ispif_t *ispif = NULL;

  assert(name != NULL);

  /* Create MCT module for sensor */
  module_ispif = mct_module_create(name);
  if (!module_ispif) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return NULL;
  }

  ispif = (ispif_t *)malloc(sizeof(ispif_t));
  if (!ispif) {
    /* no mem */
    CDBG_ERROR("%s: no mem", __func__);
    goto error;
  }
  memset(ispif, 0,  sizeof(ispif_t));
  rc = ispif_init(ispif);
  if (rc < 0) {
    CDBG_ERROR("%s: cannot create ispif\n", __func__);
    return NULL;
  }
  caps.max_num_sink_ports = ISPIF_SINK_PORTS_NUM;
  caps.max_num_src_ports = ISPIF_SRC_PORTS_NUM;
  ispif->module = module_ispif;
  if (0 != (rc = port_ispif_create_ports(ispif, &caps))) {
    CDBG_ERROR("%s: create sink port error = %d", __func__, rc);
    goto error_cap;
  }
  ispif->module->process_event = module_ispif_process_event_func;
  ispif->module->set_mod = module_ispif_set_mod_func;
  ispif->module->query_mod = module_ispif_query_mod_func;
  ispif->module->start_session = module_ispif_start_session;
  ispif->module->stop_session = module_ispif_stop_session;
  ispif->module->module_private = ispif;

  return ispif->module;

error_cap:
  ispif_destroy (ispif);
error:
  if (ispif) {
    free (ispif);
  }
  mct_module_destroy(module_ispif);
  return NULL;
}

/** module_ispif_query_mod_func:
 *    @module: ispif MCTL module object
 *
 *  This function runs in MCTL thread context.
 *
 *  This function destroys ispif module - not implemented
 *
 *  Return: None
 **/
void module_iface_deinit(mct_module_t *module)
{
  ispif_t *ispif = NULL;
  if (module) {
    ispif = (ispif_t*)module->module_private;
    if (ispif) {
      port_ispif_destroy_ports(ispif);
      mct_module_destroy(ispif->module);
      free(ispif);
      module->module_private = NULL;
    }
  }
  return;
}

