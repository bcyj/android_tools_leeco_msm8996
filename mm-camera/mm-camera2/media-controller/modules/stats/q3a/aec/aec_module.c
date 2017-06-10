/* aec_module.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_pipeline.h"
#include "aec_module.h"
#include "aec_port.h"

/* Every AEC sink port ONLY corresponds to ONE session */

/** aec_module_remove_port:
 *    @data:      mct_port_t object
 *    @user_data: mct_module_t module
 *
 * TODO description
 *
 * TODO Return
 **/
static boolean aec_module_remove_port(void *data, void *user_data)
{
  mct_port_t   *port   = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;

  if (!port || !module || strcmp(MCT_OBJECT_NAME(port), "aec_sink") ||
    strcmp(MCT_OBJECT_NAME(module), "aec")) {
    return FALSE;
  }

  /* 1. remove port from the module
   * 2. port_deinit;
   * 3. mct_port_destroy */
  mct_module_remove_port(module, port);
  aec_port_deinit(port);
  mct_port_destroy(port);

  return TRUE;
}

/** aec_module_find_port:
 *    @data1: mct_port_t object
 *    @data2: identity object to be checked
 *
 *  Check if this port has already been linked by the same session.
 *
 *  Return TRUE if the port has the same session id.
 **/
static boolean aec_module_find_port(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  unsigned int *id = (unsigned int *)data2;

  return aec_port_find_identity(port, *id);
}

/** aec_module_set_mod:
 *    @module:      TODO
 *    @module_type: TODO
 *    @identity:    TODO
 *
 * This function shouldnt be allowed by any
 * modules to overwrite module type
 *
 * TODO Return
 **/
void aec_module_set_mod(mct_module_t *module, unsigned int module_type,
  unsigned int identity)
{
  /* TODO */
  return;
}

/** aec_module_start_session:
 *    @module:    aec module
 *    @sessionid: session identity
 *
 * TODO description
 *
 * TODO Return
 **/
static boolean aec_module_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  boolean    rc = FALSE;
  mct_port_t *port = NULL;
  mct_list_t *list = NULL;

  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "aec")) {
    return FALSE;
  }

  CDBG("%s E sessionid=%d", __func__, sessionid);
  MCT_OBJECT_LOCK(module);
  if (MCT_MODULE_NUM_SINKPORTS(module) != 0) {
    list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
      &sessionid, aec_module_find_port);
    if (list != NULL) {
      rc = TRUE;
      goto start_done;
    }
  }

  /* Now need to create a new sink port */
  port = mct_port_create("aec_sink");
  if (port == NULL) {
    goto start_done;
  }

  if (aec_port_init(port, &sessionid) == FALSE) {
    goto port_init_error;
  }

  if (mct_module_add_port(module, port) == FALSE) {
    goto port_add_error;
  }

  rc = TRUE;
  goto start_done;

port_add_error:
  aec_port_deinit(port);
port_init_error:
  mct_port_destroy(port);
start_done:
  MCT_OBJECT_UNLOCK(module);
  return rc;
}

/** aec_module_stop_session:
 *    @module:    aec module
 *    @sessionid: session identity
 *
 * Deinitialize aec component
 *
 * Return  boolean
 **/
static boolean aec_module_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  mct_list_t *list;
  mct_port_t *port = NULL;

  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "aec")) {
    return FALSE;
  }

  MCT_OBJECT_LOCK(module);
  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &sessionid,
    aec_module_find_port);
  if (list == NULL) {
    MCT_OBJECT_UNLOCK(module);
    return FALSE;
  }

  port = MCT_PORT_CAST(list->data);
  mct_module_remove_port(module, port);

  aec_port_deinit(port);
  mct_port_destroy(port);
  MCT_OBJECT_UNLOCK(module);

  return TRUE;
}

/**aec_module_query_mod:
 *    @module:    aec module
 *    @query_buf: TODO
 *    @sessionid: idenity
 *
 * TODO description
 *
 * TODO Return
 **/
boolean aec_module_query_mod(mct_module_t *module,
  mct_pipeline_cap_t *query_buf, unsigned int sessionid)
{
  query_buf->stats_cap.auto_exposure_lock_supported = TRUE;
  query_buf->stats_cap.exposure_compensation_max = 3;
  query_buf->stats_cap.exposure_compensation_min = -3;
  query_buf->stats_cap.exposure_compensation_step = 1;
  query_buf->stats_cap.max_num_metering_areas = 81;
  query_buf->stats_cap.supported_aec_modes[0] = CAM_AEC_MODE_FRAME_AVERAGE;
  query_buf->stats_cap.supported_aec_modes[1] = CAM_AEC_MODE_CENTER_WEIGHTED;
  query_buf->stats_cap.supported_aec_modes[2] = CAM_AEC_MODE_SPOT_METERING;
  //query_buf->stats_cap.supported_aec_modes[3] = CAM_AEC_MODE_SMART_METERING;
  query_buf->stats_cap.supported_aec_modes[3] = CAM_AEC_MODE_CENTER_WEIGHTED;
  query_buf->stats_cap.supported_aec_modes[4] = CAM_AEC_MODE_SPOT_METERING_ADV;
  query_buf->stats_cap.supported_aec_modes[5] =
    CAM_AEC_MODE_CENTER_WEIGHTED_ADV;
  query_buf->stats_cap.supported_aec_modes_cnt = CAM_AEC_MODE_MAX - 1; //to remove support for smart_metering

  query_buf->stats_cap.supported_flash_modes[0] = CAM_FLASH_MODE_OFF;
  query_buf->stats_cap.supported_flash_modes[1] = CAM_FLASH_MODE_AUTO;
  query_buf->stats_cap.supported_flash_modes[2] = CAM_FLASH_MODE_ON;
  query_buf->stats_cap.supported_flash_modes[3] = CAM_FLASH_MODE_TORCH;
  query_buf->stats_cap.supported_flash_modes_cnt = CAM_FLASH_MODE_MAX;

  query_buf->stats_cap.supported_iso_modes[0] = CAM_ISO_MODE_AUTO;
  query_buf->stats_cap.supported_iso_modes[1] = CAM_ISO_MODE_DEBLUR;
  query_buf->stats_cap.supported_iso_modes[2] = CAM_ISO_MODE_100;
  query_buf->stats_cap.supported_iso_modes[3] = CAM_ISO_MODE_200;
  query_buf->stats_cap.supported_iso_modes[4] = CAM_ISO_MODE_400;
  query_buf->stats_cap.supported_iso_modes[5] = CAM_ISO_MODE_800;
  query_buf->stats_cap.supported_iso_modes[6] = CAM_ISO_MODE_1600;
  query_buf->stats_cap.supported_iso_modes[7] = CAM_ISO_MODE_3200;
  query_buf->stats_cap.supported_iso_modes_cnt = CAM_ISO_MODE_MAX;

  return TRUE;
}

/** aec_module_deinit:
 *    @module: aec module
 *
 * This function deinits aec module. WIll be called by Stats
 * module and triggered by MCT.
 *
 * Return nothing
 **/
void aec_module_deinit(mct_module_t *module)
{
  if (!module || strcmp(MCT_OBJECT_NAME(module), "aec")) {
    return;
  }

  /* Remove all ports of this module */
  mct_list_traverse(MCT_OBJECT_CHILDREN(module), aec_module_remove_port,
    module);

  mct_list_free_list(MCT_OBJECT_CHILDREN(module));

  mct_module_destroy(module);
}

/** aec_module_init:
 *    @name: name of this stats interface module("aec").
 *
 * TODO description
 *
 * Return this stats interface module if succes, otherwise return NULL.
 **/
mct_module_t *aec_module_init(const char *name) {
  int          i;
  mct_module_t *aec;

  if (strcmp(name, "aec")) {
    return NULL;
  }

  aec = mct_module_create("aec");
  if (!aec) {
    return NULL;
  }

  mct_module_set_set_mod_func(aec, aec_module_set_mod);
  mct_module_set_query_mod_func(aec, aec_module_query_mod);
  mct_module_set_start_session_func(aec, aec_module_start_session);
  mct_module_set_stop_session_func(aec, aec_module_stop_session);

  return aec;
}


/** aec_module_get_port:
 *    @module:   TODO
 *    sessionid: TODO
 *
 * TODO description
 *
 * TODO Return
 **/
mct_port_t *aec_module_get_port(mct_module_t *module, unsigned int sessionid)
{
  mct_list_t *list = NULL;

  if (!module || strcmp(MCT_OBJECT_NAME(module), "aec")) {
    return FALSE;
  }

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &sessionid,
    aec_module_find_port);
  if (list == NULL) {
    return NULL;
  }

  return ((mct_port_t *)list->data);
}
