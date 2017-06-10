/* q3a_module.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 **/
#include "q3a_module.h"
#include "q3a_port.h"

#if Q3A_MODULE_DEBUG
#undef CDBG
#define CDBG CDBG_ERROR
#endif

/** q3a_module_private_t
 *    @aec_module: pointer to the aec module
 *    @awb_module: pointer to the awb module
 *    @af_module:  pointer to the af module
 *
 *  Private data for the q3a module.
 **/
typedef struct {
  mct_module_t *aec_module;
  mct_module_t *awb_module;
  mct_module_t *af_module;
} q3a_module_private_t;

/** q3a_module_find_port
 *    @data1: mct_port_t object
 *    @data2: identity object to be checked
 *
 *  Check if this port has already been linked by the same session.
 *
 *  Return TRUE if the port has the same session id. Otherwise return FALSE.
 **/
static boolean q3a_module_find_port(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  unsigned int *id = (unsigned int *)data2;

  return q3a_port_find_identity(port, *id);
} /* q3a_module_find_port */

/** q3a_module_start_session
 *  @module:   stats module itself("stats")
 *  @identity: session identity
 *
 *  Create ports when new session is started
 *  Call submodule start session fn
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean q3a_module_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  boolean              rc = FALSE;
  mct_port_t           *port = NULL;
  mct_port_t           *aec_port = NULL;
  mct_port_t           *awb_port = NULL;
  mct_port_t           *af_port  = NULL;
  mct_list_t           *list = NULL;
  q3a_module_private_t *private;

  CDBG("%s: E module=%p", __func__, module);
  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "q3a")) {
    CDBG_ERROR("%s: Q3A module name does not match or module is NULL!",
      __func__);
    return FALSE;
  }

  private = module->module_private;
  if (!private) {
    CDBG_ERROR("%s: Private port is NULL!", __func__);
    return FALSE;
  }

  MCT_OBJECT_LOCK(module);

  if (MCT_MODULE_NUM_SINKPORTS(module) != 0) {
    list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
      &sessionid, q3a_module_find_port);
    if (list != NULL) {
      CDBG_ERROR("%s: Sink ports exist!", __func__);
      rc = TRUE;
      goto start_done;
    }
  }

  /* Now need to create a new sink port */
  port = mct_port_create("q3a_sink");
  if (port == NULL) {
    CDBG_ERROR("%s: Failure creating Q3A Port!", __func__);
    goto start_done;
  }

  rc = private->aec_module->start_session(private->aec_module, sessionid);
  if (rc == FALSE) {
    CDBG_ERROR("%s: Failure starting AEC session!", __func__);
    goto aec_start_fail;
  }

  rc = private->awb_module->start_session(private->awb_module, sessionid);
  if (rc == FALSE) {
    CDBG_ERROR("%s: Failure starting AWB session!", __func__);
    goto awb_start_fail;
  }

  rc = private->af_module->start_session(private->af_module, sessionid);
  if (rc == FALSE) {
    CDBG_ERROR("%s: Failure starting AF session!", __func__);
    goto af_start_fail;
  }

  aec_port = aec_module_get_port(private->aec_module, sessionid);
  awb_port = awb_module_get_port(private->awb_module, sessionid);
  af_port  = af_module_get_port(private->af_module, sessionid);

  CDBG("%s: AEC %p, AWB %p, AF %p", __func__, aec_port, awb_port, af_port);
  if (!aec_port || !awb_port || !af_port) {
    CDBG_ERROR("%s: One of the ports is NULL!", __func__);
    goto sub_mod_port_error;
  }

  if (q3a_port_init(port, aec_port, awb_port, af_port,
    sessionid) == FALSE) {
    CDBG_ERROR("%s: Port init failed", __func__);
    goto sub_mod_port_error;
  }

  if (mct_module_add_port(module, port) == FALSE) {
    CDBG_ERROR("%s: Failure adding q3a port!", __func__);
    goto port_add_error;
  }

  rc = TRUE;
  goto start_done;

port_add_error:
  q3a_port_deinit(port);
sub_mod_port_error:
  private->af_module->stop_session(private->af_module, sessionid);
af_start_fail:
  private->awb_module->stop_session(private->awb_module, sessionid);
awb_start_fail:
  private->aec_module->stop_session(private->aec_module, sessionid);
aec_start_fail:
  mct_port_destroy(port);
start_done:
  MCT_OBJECT_UNLOCK(module);
  return rc;
} /* q3a_module_start_session */

/** q3a_module_stop_session
 *    @module:   Stats module
 *    @identity: stream|session identity
 *
 *  Call submodule stop session function.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean q3a_module_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  q3a_module_private_t *private;
  mct_list_t           *list;
  mct_port_t           *port = NULL;

  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "q3a")) {
    return FALSE;
  }

  private = module->module_private;
  if (!private) {
    return FALSE;
  }

  MCT_OBJECT_LOCK(module);

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
    &sessionid, q3a_module_find_port);
  if (list == NULL) {
    MCT_OBJECT_UNLOCK(module);
    CDBG("%s: X", __func__);
    return FALSE;
  }

  port = MCT_PORT_CAST(list->data);
  mct_module_remove_port(module, port);

  q3a_port_deinit(port);

  private->af_module->stop_session(private->af_module, sessionid);
  private->awb_module->stop_session(private->awb_module, sessionid);
  private->aec_module->stop_session(private->aec_module, sessionid);

  mct_port_destroy(port);

  MCT_OBJECT_UNLOCK(module);
  return TRUE;
} /* q3a_module_stop_session */

/** q3a_module_set_mod
 *    @module:      Stats module
 *    @module_type: the type of the module
 *    @identity:    stream|session identity
 *
 *  Stats module should support this function
 *
 *  Return void
 **/
void q3a_module_set_mod(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  return;
} /* q3a_module_set_mod */

/** q3a_module_query_mod
*     @module:    Should be Stats module
*     @query_buf: the buffer with the submodule capabilities
*     @sessionid: identity of stream/session
*
*  Since stats port is purely a software module, its capability
*  is based on the submodule as a whole and independent of
*  the session id.
*  This function is used to query submodule capabilities.
*
*  Return TRUE on success, FALSE on failure.
**/
static boolean q3a_module_query_mod(mct_module_t *module,
  mct_pipeline_cap_t *query_buf, unsigned int sessionid)
{
  q3a_module_private_t *private;

  if (!module || !query_buf || strcmp(MCT_OBJECT_NAME(module), "q3a")) {
    return FALSE;
  }

  private = (q3a_module_private_t *)module->module_private;

  if (private->aec_module->query_mod(private->aec_module,
    query_buf, sessionid) == FALSE) {
    return FALSE;
  }

  if (private->awb_module->query_mod(private->awb_module,
    query_buf, sessionid) == FALSE) {
    return FALSE;
  }

  if (private->af_module->query_mod(private->af_module,
    query_buf, sessionid) == FALSE) {
    return FALSE;
  }

  return TRUE;
} /* q3a_module_query_mod */

/** q3a_module_remove_port
 *    @data:      mct_port_t object
 *    @user_data: mct_module_t module
 *
 *  Removes the port.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean q3a_module_remove_port(void *data, void *user_data)
{
  boolean      rc;
  mct_port_t   *port   = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;

  if (!port || !module || strcmp(MCT_OBJECT_NAME(port), "q3a_sink") ||
    strcmp(MCT_OBJECT_NAME(module), "q3a")) {
    return FALSE;
  }

  rc = mct_module_remove_port(module, port);
  q3a_port_deinit(port);
  mct_port_destroy(port);

  return rc;
} /* q3a_module_remove_port */

/** q3a_module_deinit
 *  @module: Stats module object
 *
 *  Function for MCT to deinit Stats module. This will remove
 *  all the ports of this module.
 *
 *  Return void.
 **/
void q3a_module_deinit(mct_module_t *module)
{
  boolean              rc;
  q3a_module_private_t *private;

  if (!module || strcmp(MCT_OBJECT_NAME(module), "q3a")) {
    return;
  }

  private = module->module_private;
  if (!private) {
    return;
  }

  /* Remove all ports of this module */
  rc = mct_list_traverse(MCT_OBJECT_CHILDREN(module), q3a_module_remove_port,
    module);

  if (rc == FALSE) {
    CDBG_ERROR("%s: Failure traversing list of ports to remove!", __func__);
  }

  mct_list_free_list(MCT_OBJECT_CHILDREN(module));

  awb_module_deinit(private->awb_module);
  aec_module_deinit(private->aec_module);
  af_module_deinit(private->af_module);

  free(module->module_private);
  mct_module_destroy(module);

  return;
} /* q3a_module_deinit */

/** q3a_module_init:
 *    @name:  name of this stats interface module("stats_stats ").
 *
 *  Stats interface module initialization entry point, it only
 *  creates stats module. Because all stats modules are pure
 *  software module, and its ports are per session based, postpone
 *  ports create during link stage.
 *
 *  Return stats module or NULL on failure.
 **/
mct_module_t *q3a_module_init(const char *name)
{
  int                  i;
  mct_module_t         *q3a;
  q3a_module_private_t *private;

  if (strcmp(name, "q3a")) {
    return NULL;
  }

  q3a = mct_module_create("q3a");
  if (!q3a) {
    return NULL;
  }

  private = malloc(sizeof(q3a_module_private_t));
  if (private == NULL) {
    goto private_error;
  }

  private->aec_module = aec_module_init("aec");
  if (private->aec_module == NULL) {
    goto aec_module_error;
  }

  private->awb_module = awb_module_init("awb");
  if (private->awb_module == NULL) {
    goto awb_module_error;
  }

  private->af_module = af_module_init("af");
  if (private->af_module == NULL) {
    goto af_module_error;
  }

  q3a->module_private = private;

  mct_module_set_set_mod_func(q3a, q3a_module_set_mod);
  mct_module_set_query_mod_func(q3a, q3a_module_query_mod);
  mct_module_set_start_session_func(q3a, q3a_module_start_session);
  mct_module_set_stop_session_func(q3a, q3a_module_stop_session);

  return q3a;

af_module_error:
  awb_module_deinit(private->awb_module);
awb_module_error:
  aec_module_deinit(private->aec_module);
aec_module_error:
  free(private);
private_error:
  mct_module_destroy(q3a);
  return NULL;
} /* q3a_module_init */

/** q3a_module_get_port:
 *    @module:    Should be Stats module
 *    @sessionid: identity of stream/session
 *
 *  This function will get the port of the module.
 *
 *  Return the port of the module or NULL on failure.
 **/
mct_port_t *q3a_module_get_port(mct_module_t *module, unsigned int sessionid)
{
  mct_list_t *list = NULL;

  if (!module || strcmp(MCT_OBJECT_NAME(module), "q3a")) {
    return FALSE;
  }

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
    &sessionid, q3a_module_find_port);
  if (list == NULL) {
    return NULL;
  }

  return ((mct_port_t *)list->data);
} /* q3a_module_get_port */
