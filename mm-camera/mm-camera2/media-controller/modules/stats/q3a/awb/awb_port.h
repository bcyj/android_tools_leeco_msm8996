/* awb_port.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AWB_PORT_H__
#define __AWB_PORT_H__
#include "modules.h"

#if 0
typedef enum {
  AWB_PORT_STATE_CREATED,
  AWB_PORT_STATE_RESERVED,
  AWB_PORT_STATE_LINKED,
  AWB_PORT_STATE_UNLINKED,
  AWB_PORT_STATE_UNRESERVED
} awb_port_state_t;

/** awb_port_private
 *    @awb_object: session index
 *    @port:       stream index
 *
 * Each awb moduld object should be used ONLY for one Bayer
 * serssin/stream set - use this structure to store session
 * and stream indices information.
 **/
typedef struct _awb_port_private {
  unsigned int      reserved_id;
  awb_port_state_t  state;
  awb_object_t      awb_object;
  q3a_thread_data_t *thread_data;
} awb_port_private_t;
#endif
void    awb_port_deinit(mct_port_t *port);
boolean awb_port_find_identity(mct_port_t *port, unsigned int identity);
boolean awb_port_init(mct_port_t *port, unsigned int *session_id);
#endif /* __AWB_PORT_H__ */
