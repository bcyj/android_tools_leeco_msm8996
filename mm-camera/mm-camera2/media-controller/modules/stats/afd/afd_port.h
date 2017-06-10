/* afd_port.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AFD_PORT_H__
#define __AFD_PORT_H__

#include "mct_port.h"
#include "afd_module.h"
#include "afd_thread.h"

#define AFD_PORT_STATE_CREATED        0x1
#define AFD_PORT_STATE_RESERVED       0x2
#define AFD_PORT_STATE_UNRESERVED     0x3
#define AFD_PORT_STATE_LINKED         0x4
#define AFD_PORT_STATE_UNLINKED       0x5
#define AFD_PORT_STATE_STREAMON       0x6
#define AFD_PORT_STATE_STREAMOFF      0x7
#define AFD_PORT_STATE_AECAWB_RUNNING (AFD_PORT_STATE_STREAMON | (0x00000001 << 8))
#define AFD_PORT_STATE_AF_RUNNING     (AFD_PORT_STATE_STREAMON | (0x00000001 << 9))

/** afd_port_private_t
 *    @sub_ports: stats module's sub-modules' sink-ports,
 *                used for internal link function
 **/
typedef struct _afd_port_private {
  unsigned int reserved_id;
  unsigned int stream_type;
  unsigned int state;

  afd_module_object_t afd_object;
  afd_thread_data_t   *thread_data;
} afd_port_private_t;

void    afd_port_deinit(mct_port_t *port);
boolean afd_port_find_identity(mct_port_t *port, unsigned int identity);
boolean afd_port_init(mct_port_t *port, unsigned int identity);
#endif /* __AFD_PORT_H__ */
