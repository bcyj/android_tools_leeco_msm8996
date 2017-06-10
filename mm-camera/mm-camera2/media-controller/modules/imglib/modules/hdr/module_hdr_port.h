/**********************************************************************
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
 * Qualcomm Technologies Proprietary and Confidential.                 *
 **********************************************************************/

#ifndef __MODULE_HDR_PORT_H__
#define __MODULE_HDR_PORT_H__

#include "mct_port.h"

/** MODULE_HDR_PORT_DUMMY_NAME:
 *
 * Defines hdr port dummy name
 *
 * Returns hdr port dummy name
 **/
#define MODULE_HDR_PORT_DUMMY_NAME (MODULE_HDR_NAME"_dummy")

/** MODULE_HDR_PORT_SINK_NAME:
 *
 * Defines hdr port sync name
 *
 * Returns hdr port sync name
 **/
#define MODULE_HDR_PORT_SINK_NAME (MODULE_HDR_NAME"_sink")

/** MODULE_HDR_PORT_SRC_NAME:
 *
 * Defines hdr port src name
 *
 * Returns hdr port src name
 **/
#define MODULE_HDR_PORT_SRC_NAME (MODULE_HDR_NAME"_src")

/** MODULE_HDR_VALIDATE_NAME:
 *    @name: name to be validated
 *
 * Checks whether specified name is part of hdr module
 *
 * Returns: TRUE if success
 **/
#define MODULE_HDR_VALIDATE_NAME(name) (!strncmp(MCT_OBJECT_NAME(name),\
  MODULE_HDR_NAME, sizeof(MODULE_HDR_NAME) - 1))

/** module_hdr_port_validate_port_session_id
 *    @data1: mct_port_t object
 *    @data2: identity to be checked
 *
 *  Checks if this port has already been assigned to specified session id
 *
 *  Return TRUE if the port has the same session id
 **/
boolean module_hdr_port_validate_port_session_id(void *data1, void *data2);

/** module_hdr_port_check_linked_port_identity
 *    @data1: mct_port_t object
 *    @data2: identity to be checked
 *
 *  Checks if this port has already been assigned to specified identity
 *    and the port is linked
 *
 *  Return TRUE if the port has the same identity and is linked
 **/
boolean module_hdr_port_check_linked_port_identity(void *data1, void *data2);

/** module_hdr_port_deinit
 *    @port: port to be deinitialized
 *
 * Deinitializes port
 *
 * Returns TRUE in case of success
 **/
void module_hdr_port_deinit(mct_port_t *port);

/** module_hdr_port_init:
 *    @port: port to be initialized
 *    @direction: source / sink
 *    @sessionid: session ID to be associated with this port
 *    @lib_handle: library handle
 *
 *  Port initialization entry point. Becase current module/port is
 *  pure software object, defer this function when session starts.
 **/
boolean module_hdr_port_init(mct_port_t *port, mct_port_direction_t direction,
  uint32_t *sessionid, void* lib_handle);

#endif //__MODULE_HDR_PORT_H__
