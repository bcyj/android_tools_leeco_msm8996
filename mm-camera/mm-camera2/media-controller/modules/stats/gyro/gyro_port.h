/* gyro_port.h
 *
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __GYRO_PORT_H__
#define __GYRO_PORT_H__
#include "mct_port.h"

void gyro_port_deinit(mct_port_t *port);
boolean gyro_port_find_identity(mct_port_t *port, unsigned int identity);
boolean gyro_port_init(mct_port_t *port, unsigned int session_id);
void gyro_port_callback(void *port, unsigned int frame_id);
#endif /* __GYRO_PORT_H__ */
