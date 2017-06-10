/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __MODULE_DENOISE_PORT_H__
#define __MODULE_DENOISE_PORT_H__

#include "mct_port.h"

/** MODULE_DENOISE_PORT_DUMMY_NAME:
 *
 * Defines denoise port dummy name
 *
 * Returns denoise port dummy name
 **/
#define MODULE_DENOISE_PORT_DUMMY_NAME (MODULE_DENOISE_NAME"_dummy")

/** MODULE_DENOISE_PORT_SINK_NAME:
 *
 * Defines denoise port sync name
 *
 * Returns denoise port sync name
 **/
#define MODULE_DENOISE_PORT_SINK_NAME (MODULE_DENOISE_NAME"_sink")

/** MODULE_DENOISE_PORT_SRC_NAME:
 *
 * Defines denoise port src name
 *
 * Returns denoise port src name
 **/
#define MODULE_DENOISE_PORT_SRC_NAME (MODULE_DENOISE_NAME"_src")

/** MODULE_DENOISE_VALIDATE_NAME:
 *    @name: name to be validated
 *
 * Checks whether specified name is part of denoise module
 *
 * Returns: TRUE if success
 **/
#define MODULE_DENOISE_VALIDATE_NAME(name) (!strncmp(MCT_OBJECT_NAME(name),\
  MODULE_DENOISE_NAME, sizeof(MODULE_DENOISE_NAME) - 1))

/** module_denoise_port_validate_port_session_id
  *    @data1: mct_port_t object
  *    @data2: identity to be checked
  *
  *  Checks if this port has already been assigned to specified session id
  *
  *  Return TRUE if the port has the same session id
  **/
boolean module_denoise_port_validate_port_session_id(void *data1,
  void *data2);

/** module_denoise_port_check_linked_port_identity
 *    @data1: mct_port_t object
 *    @data2: identity to be checked
 *
 *  Checks if this port has already been assigned to specified identity
 *    and the port is linked
 *
*  Return TRUE if the port has the same identity and is linked
 **/
boolean module_denoise_port_check_linked_port_identity(void *data1,
  void *data2);

/** module_denoise_port_deinit
 *    @port: port to be deinitialized
 *
 * Deinitializes port
 *
 * Returns TRUE in case of success
 **/
void module_denoise_port_deinit(mct_port_t *port);

/** module_denoise_port_init:
 *    @port: port to be initialized
 *    @direction: source / sink
 *    @sessionid: session ID to be associated with this port
 *    @lib_handle: library handle
 *
 *  Port initialization entry point. Becase current module/port is
 *  pure software object, defer this function when session starts.
 **/
boolean module_denoise_port_init(mct_port_t *port,
  mct_port_direction_t direction, uint32_t *sessionid, void* lib_handle);

#endif //__MODULE_DENOISE_PORT_H__
