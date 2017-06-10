/* asd_port.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ASD_PORT_H__
#define __ASD_PORT_H__

#include "mct_port.h"

#define ASD_PORT_STATE_CREATED        0x1
#define ASD_PORT_STATE_RESERVED       0x2
#define ASD_PORT_STATE_UNRESERVED     0x3
#define ASD_PORT_STATE_LINKED         0x4
#define ASD_PORT_STATE_UNLINKED       0x5
#define ASD_PORT_STATE_STREAMON       0x6
#define ASD_PORT_STATE_STREAMOFF      0x7
#define ASD_PORT_STATE_AECAWB_RUNNING (ASD_PORT_STATE_STREAMON | (0x00000001 << 8))
#define ASD_PORT_STATE_AF_RUNNING     (ASD_PORT_STATE_STREAMON | (0x00000001 << 9))

void    asd_port_deinit(mct_port_t *port);
boolean asd_port_find_identity(mct_port_t *port, unsigned int identity);
boolean asd_port_init(mct_port_t *port, unsigned int identity);
#endif /* __ASD_PORT_H__ */
