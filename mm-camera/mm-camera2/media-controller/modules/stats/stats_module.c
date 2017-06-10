/* stats_module.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "stats_module.h"
#include "stats_port.h"
#include "q3a_module.h"
#ifdef FEATURE_GYRO
#include "gyro_module.h"
#endif
#include "is_module.h"
#include "mct_controller.h"
#include "afd_module.h"
#include "asd_module.h"
#include "camera_dbg.h"

/** stats_module_query_t
 *    @query:       the structure to hold the capability of stats moduel
 *    @sessionid:   the identity of session
 *
 *  This structure is used to store stats module's capability
 *
 **/
typedef struct {
  mct_pipeline_cap_t *query;
  unsigned int       sessionid;
} stats_module_query_t;

/** stats_module_sub_ports_t
 *    @session:      the identity of session
 *    @sub_ports:    the sub modules list of stats module
 *
 *  This structure is used to store the port info of all sub modules'
 *
 **/
typedef struct {
  unsigned int session;
  mct_list_t   *sub_ports;
} stats_module_sub_ports_t;

static mct_module_init_name_t stats_mods_list[] = {
  { "q3a",  q3a_module_init,  q3a_module_deinit },
  { "afd",  afd_module_init,  afd_module_deinit },
  { "asd",  asd_module_init,  asd_module_deinit },
  { "is",   is_module_init,   is_module_deinit},
#ifdef FEATURE_GYRO
  { "gyro", gyro_module_init, gyro_module_deinit},
#endif
};

/** stats_module_check_port
 *    @data1: mct_port_t object
 *    @data2: identity to be compared
 *
 *  compared if the two identity(session_ID + stream_ID) are on the same
 *  session.
 *
 *  Return TRUE if the two identities are from the same session.
 **/
static boolean stats_module_check_port(void *data1, void *data2)
{
  mct_port_t   *port = (mct_port_t *)data1;
  unsigned int *id   = (unsigned int *)data2;

  CDBG("%s: E port=%p, name =%s, id=%d", __func__,
    port, MCT_OBJECT_NAME(port), *id );
  return (stats_port_check_port(port, *id));
}

/** stats_module_sub_mod_start_session
 *    @data:      sub module of stats module
 *    @user_data: session_ID
 *
 *  Send request_new_port to all sub-modules, form a list of all
 *  sub-modules' sink ports for stats module to use.
 *
 *  Return TRUE is all sub-modules can create new ports dynamically.
 **/
static boolean stats_module_sub_mod_start_session(void *data, void *user_data)
{
  mct_module_t  *module  = (mct_module_t *)data;
  unsigned int  *session = (unsigned int *)user_data;
  boolean rc = FALSE;

  CDBG("%s, %s start session\n", __func__, MCT_OBJECT_NAME(module));
  if (module->start_session) {
    rc = module->start_session(module, *session);
  }

  return rc;
}

/** stats_module_sub_mod_stop_session
 *    @data:      sub module of stats module
 *    @user_data: session_ID
 *
 *  Send request to stop session to submodule
 *
 *  Return TRUE if the sub module stopped successfully
 *
 **/
static boolean stats_module_sub_mod_stop_session(void *data, void *user_data)
{
  mct_module_t  *module  = (mct_module_t *)data;
  unsigned int  *session = (unsigned int *)user_data;
  boolean rc = FALSE;

  CDBG("%s, %s stop session\n", __func__, MCT_OBJECT_NAME(module));
  if (module->stop_session) {
    rc = module->stop_session(module, *session);
  }

  return rc;
}

/** stats_module_deinit_submod:
 *    @data:     sub module of stats module
 *    @userdata: Usr data (NULL)
 *
 *
 *  Call deinit of each sub module.
 *  Return: TRUE
 **/
boolean stats_module_deinit_submod(void *data, void *userdata)
{
  int i;
  mct_module_t *sub_module = (mct_module_t *)data;

  if (sub_module == NULL) {
    CDBG_ERROR("%s: submodule null", __func__);
  }
  /* Iterate through sub module list and call deinit */
  for (i = 0; i < (int)(sizeof(stats_mods_list) /
    sizeof(mct_module_init_name_t)); i++) {
    if ((sub_module != NULL) && (MCT_MODULE_NAME(sub_module) != NULL) &&
      (!strcmp(MCT_MODULE_NAME(sub_module), stats_mods_list[i].name))) {
      stats_mods_list[i].deinit_mod(sub_module);
      break;
    }
  }

  return TRUE;
}

/** stats_module_get_sub_ports
 *    @data:      sub module of stats module
 *    @user_data: stats_module_sub_ports_t object
 *
 *  Find the port for given stats mobule, and attach it to the port list
 *  For any given session, every sub-module must have ONLY one port corresponds
 *  to it.
 *
 *  Return boolean to indicate success or failure
 **/
static boolean stats_module_get_sub_ports(void *data, void *user_data)
{
  boolean rc = TRUE;
  mct_list_t   *list;

  stats_module_sub_ports_t *sub    = (stats_module_sub_ports_t *)user_data;
  mct_module_t             *module = (mct_module_t *)data;
  mct_port_t               *port   = NULL;

  if (!strcmp(MCT_OBJECT_NAME(module), "q3a")) {
    port = q3a_module_get_port(module, sub->session);
  } else if (!strcmp(MCT_OBJECT_NAME(module), "afd")) {
    port = afd_module_get_port(module, sub->session);
  } else if (!strcmp(MCT_OBJECT_NAME(module), "asd")) {
    port = asd_module_get_port(module, sub->session);
  }
#if FEATURE_GYRO
  if (!strcmp(MCT_OBJECT_NAME(module), "gyro")) {
    port = gyro_module_get_port(module, sub->session);
    CDBG("%s: gyro port %p", __func__, port);
  }
#endif
  if (!strcmp(MCT_OBJECT_NAME(module), "is")) {
    port = is_module_get_port(module, sub->session);
    CDBG("%s: is port %p", __func__, port);
  }

  /* TODO: add other sub modules */

  if (port != NULL) {
    list = mct_list_append(sub->sub_ports, port, NULL, NULL);
    if (list != NULL) {
      sub->sub_ports = list;
    } else {
      rc = FALSE;
    }
  } else {
    rc = FALSE;
  }
  return rc;
}

/** stats_module_start_session
 *    @module:   Stats module
 *    @identity: stream|session identity
 *
 *  Create ports when new session is started, call submodule start session
 *  function.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean stats_module_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  boolean    rc = TRUE;
  mct_port_t *port = NULL;
  mct_list_t *list = NULL;

  stats_module_sub_ports_t sub;

  CDBG("%s: E, sessionid=0x%x, module=%p", __func__, sessionid, module);
  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "stats")) {
    return NULL;
  }

  MCT_OBJECT_LOCK(module);

  sessionid = sessionid << 16;
  if (MCT_MODULE_NUM_SINKPORTS(module) != 0) {
    if (mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
      &sessionid, stats_module_check_port) != NULL) {

      rc = TRUE;
      goto start_done;
    }
  }

  /* Now need to create a new sink port */
  port = mct_port_create("stats_sink");
  if (port == NULL) {
    CDBG_ERROR("%s: Failure creating Stats sink port!", __func__);
    rc = FALSE;
    goto start_done;
  }

  /* send start new session to all sub-modules */
  if (mct_list_traverse((mct_list_t *)module->module_private,
    stats_module_sub_mod_start_session, &sessionid) == FALSE) {

    CDBG_ERROR("%s: Failure starting session in sub-modules!", __func__);
    goto sub_mod_session_error;
  }

  sub.session   = sessionid;
  sub.sub_ports = NULL;

  if (mct_list_traverse((mct_list_t *)module->module_private,
    stats_module_get_sub_ports, &sub) == FALSE) {
    CDBG_ERROR("%s: Failure getting sub-ports!", __func__);
    goto sub_mod_port_error;
  }

  if (stats_port_init(port, sessionid , sub.sub_ports) == FALSE) {
    CDBG_ERROR("%s: Failure initializing stats sub-ports!", __func__);
    goto sub_mod_port_error;
  }

  if (mct_module_add_port(module, port) == FALSE) {
    CDBG_ERROR("%s: Failure adding port!", __func__);
    goto port_add_error;
  }

  rc = TRUE;
  goto start_done;

port_add_error:
  stats_port_deinit(port);
sub_mod_port_error:
  mct_list_free_list(sub.sub_ports);
sub_mod_session_error:
  mct_list_traverse((mct_list_t *)module->module_private,
    stats_module_sub_mod_stop_session, &sessionid);
  mct_port_destroy(port);
  rc = FALSE;
start_done:
  MCT_OBJECT_UNLOCK(module);

  CDBG("%s: X rc =%d", __func__, rc);
  return rc;
}

/** stats_module_stop_session
 *    @module:   Stats module
 *    @identity: stream|session identity
 *
 *  Call submodule stop session function.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean stats_module_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  mct_list_t *list;
  mct_port_t *port;
  CDBG("%s: E, module=%p", __func__, module);
  sessionid = sessionid << 16;

  CDBG("%s: E, sessionid = %d", __func__, sessionid);
  if (!module || strcmp(MCT_OBJECT_NAME(module), "stats")) {
    return FALSE;
  }

  MCT_OBJECT_LOCK(module);

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &sessionid,
    stats_module_check_port);
  if (list == NULL) {
    MCT_OBJECT_UNLOCK(module);
    CDBG("%s: X, early", __func__);
    return FALSE;
  }
  port = MCT_PORT_CAST(list->data);

  CDBG("%s: list =%p, remove port =%p name=%s from module=%p, name=%s",
    __func__, list, port, MCT_OBJECT_NAME( port), module,
    MCT_OBJECT_NAME(module));
  mct_module_remove_port(module, port);

  CDBG("%s: 1 port =%p name=%s ", __func__,
    port, MCT_OBJECT_NAME( port) );
  stats_port_deinit(port);

  CDBG("%s: 2 port =%p name=%s ", __func__,
    port, MCT_OBJECT_NAME( port) );
  mct_list_traverse((mct_list_t *)module->module_private,
    stats_module_sub_mod_stop_session, &sessionid);

  CDBG("%s: 3 port =%p name=%s ", __func__,
    port, MCT_OBJECT_NAME( port) );
  mct_port_destroy(port);

  MCT_OBJECT_UNLOCK(module);
  CDBG("%s: X", __func__);
  return TRUE;
}

/** stats_module_set_mod
 *    @module:      Stats module
 *    @module_type: The type of module
 *    @identity:    stream|session identity
 *
 *  Stats module should support this function.
 *
 *  Return void.
 **/
void stats_module_set_mod(mct_module_t *module, unsigned int module_type,
  unsigned int identity)
{
  return;
}

/** stats_module_query_submod
 *    @data:     Sub modules  eg q3a
 *    @userdata: stats_module_query_t ptr
 *
 *  This function will query all the sub modules and is mainly used to query
 *  capabilities.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean stats_module_query_submod(void *data, void *userdata)
{
  mct_module_t         *sub_module  = (mct_module_t *)data;
  stats_module_query_t *stats_query = (stats_module_query_t *)userdata;

  /* Call submodule query fn */
  if (sub_module->query_mod(sub_module, stats_query->query,
    stats_query->sessionid) == FALSE) {

    return FALSE;
  }

  return TRUE;
}

/** stats_module_query_mod
*     @module:     stats module itself ("stats")
*     @query_buf:  media controller's query information buffer
*     @sessionid:  session and stream identity
*
*  stats proc is a pure software module, its capability
*  is based on the sub-modules and independent of the session id.
*  This function is used to query submodule capability by simply
*  pass the query buffer to sub-modules so that the can be filled
*  out accordingly.
*
*  Return boolean to indicate success or failure
**/
static boolean stats_module_query_mod(mct_module_t *module,
  mct_pipeline_cap_t *query_buf, unsigned int sessionid)
{
  stats_module_query_t stats_query;

  if (!module || !query_buf || strcmp(MCT_OBJECT_NAME(module), "stats")) {
    return FALSE;
  }

  stats_query.sessionid = sessionid;
  stats_query.query     = query_buf;

  /* send query sub module */
  mct_list_traverse((mct_list_t *)(module->module_private),
    stats_module_query_submod, &stats_query);

  return TRUE;
}

/** stats_module_deinit
 *    @module: Stats module object
 *
 *  Return: NULL
 *
 *  Function for MCT to deinit Stats module. This will remove
 *  all the ports of this module.
 **/
void stats_module_deinit(mct_module_t *module)
{
  if (!module || strcmp(MCT_OBJECT_NAME(module), "stats")) {
    return;
  }

  mct_list_free_all((mct_list_t *)module->module_private,
    stats_module_deinit_submod);

  mct_module_destroy(module);

  return;
}

/** stats_module_init:
 *    @name: name of this stats interface module("stats").
 *
 *  Stats interface module initialization entry point, it only
 *  creates stats module. Because all stats sub-modules are pure
 *  software module, and its ports are per session based, ports
 *  creation is postponed during session start.
 *
 *  Return stats module if success, NULL on failure.
 **/
mct_module_t* stats_module_init(const char *name)
{
  int i;
  int stats_list_size;
  mct_list_t   *sub_mod_list = NULL;
  mct_module_t *stats, *mod;
  mct_port_t   *port;

  CDBG("%s: E", __func__);
  if (strcmp(name, "stats")) {
    CDBG_ERROR("%s: Invalid stats module name!", __func__);
    return NULL;
  }

  stats = mct_module_create("stats");
  if (!stats) {
    CDBG_ERROR("%s: Failure creating stats module!", __func__);
    return NULL;
  }

  MCT_OBJECT_LOCK(stats);
  /* initialize stats sub-modules */
  stats_list_size = (sizeof(stats_mods_list) / sizeof(mct_module_init_name_t));
  for (i = 0; i < stats_list_size; i++) {
    mod = stats_mods_list[i].init_mod(stats_mods_list[i].name);
    if (mod == NULL) {
      CDBG_ERROR("%s: Sub-module NULL. Skip initializing thi one!", __func__);
      continue;
    }
    sub_mod_list = mct_list_append(sub_mod_list, mod, NULL, NULL);
    if (sub_mod_list == NULL) {
      CDBG_ERROR("%s: Sub module list NULL!", __func__);
      MCT_OBJECT_UNLOCK(stats);
      goto sub_mod_error;
    }
  } /* for */
  MCT_OBJECT_UNLOCK(stats);

  stats->module_private = sub_mod_list;

  mct_module_set_set_mod_func(stats, stats_module_set_mod);
  mct_module_set_query_mod_func(stats, stats_module_query_mod);
  mct_module_set_start_session_func(stats, stats_module_start_session);
  mct_module_set_stop_session_func(stats, stats_module_stop_session);


  CDBG("%s: E Module=%p", __func__, stats);
  return stats;

sub_mod_error:
  mct_list_free_all(sub_mod_list, stats_module_deinit_submod);
  mct_module_destroy(stats);
  return NULL;
}
