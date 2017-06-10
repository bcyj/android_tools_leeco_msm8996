/* asd_module.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "asd_module.h"
#include "asd_port.h"

typedef struct {
  int my_private;
} asd_module_private_t;

/** asd_module_find_port
 *    @data1: mct_port_t object
 *    @data2: identity object to be checked
 *
 *  Check if this port has already been linked by the same session.
 *
 *  Return TRUE if the port has the same session id.
 **/
static boolean asd_module_find_port(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  unsigned int *id = (unsigned int *)data2;

  return asd_port_find_identity(port, *id);
}

/** asd_module_start_session
 *    @module:   stats module itself("stats")
 *    @identity: session identity
 *
 *
 *  Create ports when new session is started
 *  Call submodule start session fn
 *
 *  Return :boolean
 **/
static boolean asd_module_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  boolean rc = FALSE;
  mct_port_t *port = NULL;
  mct_list_t *list = NULL;
  asd_module_private_t *private = NULL;

  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "asd")){
    ALOGE("%s: ASD module name does not match!", __func__);
    return FALSE;
  }

  private = module->module_private;
  if (!private) {
    ALOGE("%s: Private port NULL!", __func__);
    return FALSE;
  }

  MCT_OBJECT_LOCK(module);
  if (MCT_MODULE_NUM_SINKPORTS(module) != 0) {
    list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
             &sessionid, asd_module_find_port);

    /* corresponding session port is already existing */
    if (list != NULL) {
      CDBG("%s: Sink ports exist!", __func__);
      rc = TRUE;
      goto start_done;
    }
  }

  /* Now need to create a new sink port */
  CDBG("%s: Create ASD port!", __func__);
  port = mct_port_create("asd_sink");
  if (port == NULL){
    ALOGE("%s: Failure creating ASD Port!", __func__);
    goto start_done;
  }

  CDBG("%s: Initi ASD port!", __func__);
  if (asd_port_init(port, sessionid) == FALSE) {
    ALOGE("%s: ASD port init failure!", __func__);
    goto port_init_error;
  }

  CDBG("%s: Add ASD port!", __func__);
  if (mct_module_add_port(module, port) == FALSE) {
    ALOGE("%s: Failure adding ASD port!", __func__);
    goto port_add_error;
  }

  rc = TRUE;
  goto start_done;

port_add_error:
  asd_port_deinit(port);
port_init_error:
  mct_port_destroy(port);
  rc = FALSE;
start_done:
  MCT_OBJECT_UNLOCK(module);
  return rc;
}

/** asd_module_stop_session
 *    @module: Stats module
 *    @identity: stream|session identity
 *
 *  Call submodule stop session fn
 *  Return: boolean
 **/
static boolean asd_module_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  mct_list_t *list;
  mct_port_t *port = NULL;

  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "asd"))
    return FALSE;

  MCT_OBJECT_LOCK(module);

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
           &sessionid, asd_module_find_port);
  if (list == NULL) {
    MCT_OBJECT_UNLOCK(module);
    return FALSE;
  }
  port = MCT_PORT_CAST(list->data);

  mct_module_remove_port(module, port);

  asd_port_deinit(port);
  mct_port_destroy(port);

  MCT_OBJECT_UNLOCK(module);
  return TRUE;
}

/** asd_module_set_mod
 *    @module: Stats module
 *    @module_type:
 *    @identity: stream|session identity
 *
 *  ASD module should support this function
 *  Return :void
 **/
void asd_module_set_mod(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  return;
}

/** asd_module_query_mod
*     @module:    ASD module object
*     @query_buf: ASD capability to fill out
*     @sessionid: identity of stream/session
*
*  Since ASD is purely a software module, its capability
*  is independant of the session id.
*
*  This function is used to query module capability
*
*  Return: Boolean
**/
static boolean asd_module_query_mod(mct_module_t *module,
  mct_pipeline_cap_t *query_buf, unsigned int sessionid)
{
  asd_module_private_t *private;

  if (!module || !query_buf || strcmp(MCT_OBJECT_NAME(module), "asd"))
    return FALSE;

  private = (asd_module_private_t *)module->module_private;

  /* TODO: fill out query_buf for ASD related information */

  return TRUE;
}

/** asd_module_remove_port
 *    @data: mct_port_t object
 *    @user_data: mct_module_t module
 **/
static boolean asd_module_remove_port(void *data, void *user_data)
{
  mct_port_t *port     = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;

  if (!port || !module || strcmp(MCT_OBJECT_NAME(port), "asd_sink") ||
      strcmp(MCT_OBJECT_NAME(module), "asd"))
    return FALSE;

  /* 1. remove port from the module
   * 2. port_deinit;
   * 3. mct_port_destroy */
  mct_module_remove_port(module, port);
  asd_port_deinit(port);
  mct_port_destroy(port);

  return TRUE;
}

/** asd_module_deinit
 *  @mod: Stats module object
 *
 *  Function for Stats module to deinit ASD module. This will remove
 *  all ports of this module.
 *
 *  Return nothing
 **/
void asd_module_deinit(mct_module_t *module)
{
  if (!module || strcmp(MCT_OBJECT_NAME(module), "asd"))
    return;

  /* Remove all ports of this module */
  mct_list_traverse(MCT_OBJECT_CHILDREN(module), asd_module_remove_port,
    module);

  mct_list_free_list(MCT_OBJECT_CHILDREN(module));

  free(module->module_private);
  mct_module_destroy(module);

  return;
}

/** asd_module_init:
 *    @name: name of ASD module("asd").
 *    Return: asd module
 *
 *  ASD module initializtion entry point, it only
 *  creates ASD module. Because ASD module is pure
 *  software module, and its ports are per session based, postpone
 *  ports create during link stage.
 *
 *  Return ASD module object
 **/
mct_module_t* asd_module_init(const char *name)
{
  int i;
  mct_module_t *asd;
  asd_module_private_t *private;

  if (strcmp(name, "asd"))
    return NULL;

  asd = mct_module_create("asd");
  if (!asd)
    return NULL;

  private = malloc(sizeof(asd_module_private_t));
  if (private == NULL)
    goto private_error;

  asd->module_private = private;

  mct_module_set_set_mod_func(asd, asd_module_set_mod);
  mct_module_set_query_mod_func(asd, asd_module_query_mod);
  mct_module_set_start_session_func(asd, asd_module_start_session);
  mct_module_set_stop_session_func(asd, asd_module_stop_session);

  return asd;

private_error:
  mct_module_destroy(asd);
  return NULL;
}

/** asd_module_get_port
 *    @module:    ASD module object
 *    @sessionid: per sensor based session id
 *
 **/
mct_port_t *asd_module_get_port(mct_module_t *module, unsigned int sessionid)
{

  mct_list_t *list = NULL;
  if (!module || strcmp(MCT_OBJECT_NAME(module), "asd"))
    return FALSE;

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
           &sessionid, asd_module_find_port);
  if (list == NULL) {
    return NULL;
  }

  return MCT_PORT_CAST(list->data);
}
