/*
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_module.h"
#include "gyro_port.h"
#include "dsps_hw_interface.h"
#include "camera_dbg.h"


#if 1
#undef CDBG
#define CDBG ALOGE
#endif


/** gyro_module_find_port
 *    @data1: mct_port_t object
 *    @data2: identity object to be checked
 *
 *  Check if this port has already been linked by the same session.
 *
 *  Return TRUE if the port has the same session id.
 **/
static boolean gyro_module_find_port(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  unsigned int *id = (unsigned int *)data2;

  return gyro_port_find_identity(port, *id);
}


/** gyro_module_remove_port
 *    @data: mct_port_t object
 *    @user_data: mct_module_t module
 **/
static boolean gyro_module_remove_port(void *data, void *user_data)
{
  mct_port_t *port = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;

  if (!port || !module || strcmp(MCT_OBJECT_NAME(port), "gyro_sink") ||
      strcmp(MCT_OBJECT_NAME(module), "gyro"))
    return FALSE;

  mct_module_remove_port(module, port);
  gyro_port_deinit(port);
  mct_port_destroy(port);

  return TRUE;
}


/** gyro_module_start_session
 *    @module: gyro module
 *    @session_id: session id
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean gyro_module_start_session(mct_module_t *module,
  unsigned int session_id)
{
  boolean rc = FALSE;
  mct_port_t *port = NULL;
  mct_list_t *list = NULL;

  CDBG("%s: Enter", __func__);
  if (module == NULL || strcmp(MCT_OBJECT_NAME(module), "gyro")) {
    return rc;
  }

  MCT_OBJECT_LOCK(module);
  if (MCT_MODULE_NUM_SINKPORTS(module) != 0) {
    list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &session_id,
             gyro_module_find_port);
    if (list != NULL) {
      rc = TRUE;
      goto start_done;
    }
  }

  /* Now need to create a new sink port */
  port = mct_port_create("gyro_sink");
  if (port == NULL) {
    goto start_done;
  }

  if (gyro_port_init(port, session_id) == FALSE) {
    goto port_init_error;
  }

  if (mct_module_add_port(module, port) == FALSE) {
    goto port_add_error;
  }

  MCT_OBJECT_REFCOUNT(module) += 1;
  rc = TRUE;
  CDBG("%s: Exit successful", __func__);
  goto start_done;

port_add_error:
  gyro_port_deinit(port);
port_init_error:
  mct_port_destroy(port);
start_done:
  MCT_OBJECT_UNLOCK(module);
  return rc;
}


/** gyro_module_stop_session
 *    @module: Gyro module
 *    @identity: stream/session identity
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean gyro_module_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  mct_list_t *list;
  mct_port_t *port = NULL;

  CDBG("%s: Enter", __func__);
  if (!module || strcmp(MCT_OBJECT_NAME(module), "gyro")) {
    return FALSE;
  }

  MCT_OBJECT_LOCK(module);

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
           &sessionid, gyro_module_find_port);
  if (list == NULL) {
    MCT_OBJECT_UNLOCK(module);
    return FALSE;
  }

  port = MCT_PORT_CAST(list->data);
  mct_module_remove_port(module, port);
  gyro_port_deinit(port);
  mct_port_destroy(port);
  MCT_OBJECT_REFCOUNT(module) -= 1;

  MCT_OBJECT_UNLOCK(module);

  CDBG("%s: Exit successful", __func__);
  return TRUE;
}


mct_port_t *gyro_module_get_port(mct_module_t *module, unsigned int sessionid)
{

  mct_list_t *list = NULL;
  if (!module || strcmp(MCT_OBJECT_NAME(module), "gyro"))
    return NULL;

  list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
           &sessionid, gyro_module_find_port);
  if (list == NULL) {
    CDBG("%s: Exit failure", __func__);
    return NULL;
  }
  CDBG("%s: Exit successful", __func__);
  return (mct_port_t *)list->data;
}


/** gyro_module_init:
 *    @name: name of this interface module.
 *    Return: gyro module
 *
 *  Gyro interface module initializtion entry point, it only
 *  creates gyro module. Because the gyro module is pure
 *  software module, and its ports are per session based,
 *  postpone port creation to start session.
 **/
mct_module_t* gyro_module_init(const char *name)
{
  int i;
  mct_module_t *gyro;

  CDBG("%s: Enter", __func__);
  if (strcmp(name, "gyro"))
    return NULL;

  gyro = mct_module_create("gyro");
  if (gyro == NULL)
    return NULL;

  /* Accept default set_mod function */
  /* Accept default query_mod funcion */
  mct_module_set_start_session_func(gyro, gyro_module_start_session);
  mct_module_set_stop_session_func(gyro, gyro_module_stop_session);

  CDBG("%s: Exit successful", __func__);
  return gyro;

dsps_error:
  mct_module_destroy(gyro);
  return NULL;
}


/** gyro_module_deinit
 *  @mod: gyro module object
 *
 *  Return: NULL
 *
 *  Function for MCT to deinit gyro module.  This will remove all ports of this
 *  module.
 **/
void gyro_module_deinit(mct_module_t *module)
{
  CDBG("%s: Enter", __func__);
  if (!module || strcmp(MCT_OBJECT_NAME(module), "gyro"))
    return;

  /* Remove all ports of this module */
  mct_list_traverse(MCT_OBJECT_CHILDREN(module), gyro_module_remove_port,
    module);
  mct_list_free_list(MCT_OBJECT_CHILDREN(module));

  mct_module_destroy(module);
  CDBG("%s: Exit successful", __func__);
}

