/* afd_module.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "afd_module.h"
#include "afd_port.h"

typedef struct {
  int my_private;
} afd_module_private_t;

/** afd_module_find_port
 *    @data1: mct_port_t object
 *    @data2: identity object to be checked
 *
 *  Check if this port has already been linked by the same session.
 *
 *  Return TRUE if the port has the same session id.
 **/
static boolean afd_module_find_port(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  unsigned int *id = (unsigned int *)data2;

  return afd_port_find_identity(port, *id);
}

/** afd_module_start_session
 *    @module:   stats module itself("stats")
 *    @identity: session identity
 *
 *
 *  Create ports when new session is started
 *  Call submodule start session fn
 *
 *  Return :boolean
 **/
static boolean afd_module_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  boolean rc = FALSE;
  mct_port_t *port = NULL;
  mct_port_t *aec_port = NULL, *awb_port = NULL, *af_port = NULL;
  mct_list_t *list = NULL;
  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "afd")){
    ALOGE("%s: AFD module name does not match!", __func__);
    return FALSE;
  }

  MCT_OBJECT_LOCK(module);
  if (MCT_MODULE_NUM_SINKPORTS(module) != 0) {
    list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
             &sessionid, afd_module_find_port);

    /* corresponding session port is already existing */
    if (list != NULL) {
      rc = TRUE;
      goto start_done;
    }
  }

  /* Now need to create a new sink port */
  port = mct_port_create("afd_sink");
  if (port == NULL){
    ALOGE("%s: Failure creating AFD Port!", __func__);
    goto start_done;
  }

  if (afd_port_init(port, sessionid) == FALSE) {
    goto port_init_error;
  }

  if (mct_module_add_port(module, port) == FALSE) {
    goto port_add_error;
  }

  rc = TRUE;
  goto start_done;

port_add_error:
  afd_port_deinit(port);
port_init_error:
  mct_port_destroy(port);
  rc = FALSE;
start_done:
  MCT_OBJECT_UNLOCK(module);
  return rc;
}

/** afd_module_stop_session
 *    @module: Stats module
 *    @identity: stream|session identity
 *
 *  Call submodule stop session fn
 *  Return: boolean
 **/
static boolean afd_module_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  mct_list_t *list;
  mct_port_t *port = NULL;

  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "afd"))
    return FALSE;

  MCT_OBJECT_LOCK(module);

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
           &sessionid, afd_module_find_port);
  if (list == NULL) {
    MCT_OBJECT_UNLOCK(module);
    return FALSE;
  }
  port = MCT_PORT_CAST(list->data);

  mct_module_remove_port(module, port);

  afd_port_deinit(port);
  mct_port_destroy(port);

  MCT_OBJECT_UNLOCK(module);
  return TRUE;
}

/** afd_module_set_mod
 *    @module: Stats module
 *    @module_type:
 *    @identity: stream|session identity
 *
 *  AFD module should support this function
 *  Return :void
 **/
void afd_module_set_mod(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  return;
}

/** afd_module_query_mod
*     @module:    AFD module object
*     @query_buf: AFD capability to fill out
*     @sessionid: identity of stream/session
*
*  Since AFD is purely a software module, its capability
*  is independant of the session id.
*
*  This function is used to query module capability
*
*  Return: Boolean
**/
static boolean afd_module_query_mod(mct_module_t *module,
  mct_pipeline_cap_t *query_buf, unsigned int sessionid)
{
  afd_module_private_t *private;

  if (!module || !query_buf || strcmp(MCT_OBJECT_NAME(module), "afd"))
    return FALSE;

  private = (afd_module_private_t *)module->module_private;

  query_buf->stats_cap.supported_antibandings[0] =  CAM_ANTIBANDING_MODE_OFF;
  query_buf->stats_cap.supported_antibandings[1] =  CAM_ANTIBANDING_MODE_60HZ;
  query_buf->stats_cap.supported_antibandings[2] =  CAM_ANTIBANDING_MODE_50HZ;
  query_buf->stats_cap.supported_antibandings[3] =  CAM_ANTIBANDING_MODE_AUTO;
  query_buf->stats_cap.supported_antibandings_cnt = 4;
  return TRUE;
}

/** afd_module_remove_port
 *    @data: mct_port_t object
 *    @user_data: mct_module_t module
 **/
static boolean afd_module_remove_port(void *data, void *user_data)
{
  mct_port_t *port     = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;

  if (!port || !module || strcmp(MCT_OBJECT_NAME(port), "afd_sink") ||
      strcmp(MCT_OBJECT_NAME(module), "afd"))
    return FALSE;

  /* 1. remove port from the module
   * 2. port_deinit;
   * 3. mct_port_destroy */
  mct_module_remove_port(module, port);
  afd_port_deinit(port);
  mct_port_destroy(port);

  return TRUE;
}

/** afd_module_deinit
 *  @mod: Stats module object
 *
 *  Function for Stats module to deinit AFD module. This will remove
 *  all ports of this module.
 *
 *  Return nothing
 **/
void afd_module_deinit(mct_module_t *module)
{
  if (!module || strcmp(MCT_OBJECT_NAME(module), "afd"))
    return;

  /* Remove all ports of this module */
  mct_list_traverse(MCT_OBJECT_CHILDREN(module), afd_module_remove_port,
    module);

  mct_list_free_list(MCT_OBJECT_CHILDREN(module));

  free(module->module_private);
  mct_module_destroy(module);

  return;
}

/** afd_module_init:
 *    @name: name of AFD module("afd").
 *    Return: afd module
 *
 *  AFD module initializtion entry point, it only
 *  creates AFD module. Because AFD module is pure
 *  software module, and its ports are per session based, postpone
 *  ports create during link stage.
 *
 *  Return AFD module object
 **/
mct_module_t* afd_module_init(const char *name)
{
  int i;
  mct_module_t *afd;
  afd_module_private_t *private;
  if (strcmp(name, "afd"))
    return NULL;

  afd = mct_module_create("afd");
  if (!afd)
    return NULL;

  private = malloc(sizeof(afd_module_private_t));
  if (private == NULL)
    goto private_error;

  afd->module_private = private;

  mct_module_set_set_mod_func(afd, afd_module_set_mod);
  mct_module_set_query_mod_func(afd, afd_module_query_mod);
  mct_module_set_start_session_func(afd, afd_module_start_session);
  mct_module_set_stop_session_func(afd, afd_module_stop_session);
  return afd;

private_error:
  mct_module_destroy(afd);
  return NULL;
}

/** afd_module_get_port
 *    @module:    AFD module object
 *    @sessionid: per sensor based session id
 *
 **/
mct_port_t *afd_module_get_port(mct_module_t *module, unsigned int sessionid)
{

  mct_list_t *list = NULL;
  if (!module || strcmp(MCT_OBJECT_NAME(module), "afd"))
    return FALSE;

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
           &sessionid, afd_module_find_port);
  if (list == NULL) {
    return NULL;
  }

  return MCT_PORT_CAST(list->data);
}
