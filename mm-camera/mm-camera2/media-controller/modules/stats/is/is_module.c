/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_module.h"
#include "is_port.h"
#include "camera_dbg.h"


#if 0
#undef CDBG
#define CDBG ALOGE
#endif


/** is_module_find_port
 *    @data1: mct_port_t object
 *    @data2: identity object to be checked
 *
 *  Check if this port has already been linked by the same session.
 *
 *  Return TRUE if the port has the same session id.
 **/
static boolean is_module_find_port(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  unsigned int *id = (unsigned int *)data2;

  return is_port_find_identity(port, *id);
}


/** is_module_remove_port
 *    @data: mct_port_t object
 *    @user_data: mct_module_t module
 **/
static boolean is_module_remove_port(void *data, void *user_data)
{
  mct_port_t *port = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;

  if (!port || !module || strcmp(MCT_OBJECT_NAME(port), "is_sink") ||
      strcmp(MCT_OBJECT_NAME(module), "is"))
    return FALSE;

  mct_module_remove_port(module, port);
  is_port_deinit(port);
  mct_port_destroy(port);

  return TRUE;
}


/** is_module_start_session
 *    @module: IS module
 **/
static boolean is_module_start_session(mct_module_t *module,
  unsigned int session_id)
{
  boolean rc = FALSE;
  mct_port_t *port = NULL;
  mct_list_t *list = NULL;

  CDBG("%s: Enter", __func__);
  if (module == NULL || strcmp(MCT_OBJECT_NAME(module), "is"))
    return rc;

  MCT_OBJECT_LOCK(module);
  if (MCT_MODULE_NUM_SINKPORTS(module) != 0) {
    list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &session_id,
             is_module_find_port);
    if (list != NULL) {
      rc = TRUE;
      goto start_done;
    }
  }

  /* Now need to create a new sink port */
  port = mct_port_create("is_sink");
  if (port == NULL)
    goto start_done;

  if (is_port_init(port, session_id) == FALSE)
    goto port_init_error;

  if (mct_module_add_port(module, port) == FALSE)
    goto port_add_error;

  rc = TRUE;
  CDBG("%s: Exit successful", __func__);
  goto start_done;

port_add_error:
  is_port_deinit(port);
port_init_error:
  mct_port_destroy(port);
start_done:
  MCT_OBJECT_UNLOCK(module);
  return rc;
}


/** is_module_stop_session
 *    @module: IS module
 *    @identity: stream/session identity
 **/
static boolean is_module_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  mct_list_t *list;
  mct_port_t *port = NULL;

  CDBG("%s: Enter", __func__);
  if (!module || strcmp(MCT_OBJECT_NAME(module), "is"))
    return FALSE;

  MCT_OBJECT_LOCK(module);

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
           &sessionid, is_module_find_port);
  if (list == NULL) {
    MCT_OBJECT_UNLOCK(module);
    return FALSE;
  }

  port = MCT_PORT_CAST(list->data);
  mct_module_remove_port(module, port);
  is_port_deinit(port);
  mct_port_destroy(port);

  MCT_OBJECT_UNLOCK(module);

  CDBG("%s: Exit successful", __func__);
  return TRUE;
}


mct_port_t *is_module_get_port(mct_module_t *module, unsigned int sessionid)
{

  mct_list_t *list = NULL;
  if (!module || strcmp(MCT_OBJECT_NAME(module), "is"))
    return NULL;

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
           &sessionid, is_module_find_port);
  if (list == NULL) {
    CDBG("%s: Exit failure", __func__);
    return NULL;
  }
  CDBG("%s: Exit successful", __func__);
  return (mct_port_t *)list->data;
}


/** is_module_init:
 *    @name: name of this interface module.
 *    Return: IS module
 *
 *  IS interface module initializtion entry point, it only
 *  creates IS module. Because the IS module is pure
 *  software module, and its ports are per session based,
 *  postpone port creation to start session.
 **/
mct_module_t* is_module_init(const char *name)
{
  int i;
  mct_module_t *is;

  CDBG("%s: Enter", __func__);
  if (strcmp(name, "is"))
    return NULL;

  is = mct_module_create("is");
  if (is == NULL)
    return NULL;

  /* Accept default set_mod function */
  /* Accept default query_mod funcion */
  mct_module_set_start_session_func(is, is_module_start_session);
  mct_module_set_stop_session_func(is, is_module_stop_session);

  return is;
}


/** is_module_deinit
 *  @mod: is_module object
 *
 *  Return: NULL
 *
 *  Function for MCT to deinit is module.  This will remove all ports of this
 *  module.
 **/
void is_module_deinit(mct_module_t *module)
{
  if (!module || strcmp(MCT_OBJECT_NAME(module), "is"))
    return;

  /* Remove all ports of this module */
  mct_list_traverse(MCT_OBJECT_CHILDREN(module), is_module_remove_port,
    module);
  mct_list_free_list(MCT_OBJECT_CHILDREN(module));
  mct_module_destroy(module);
}

