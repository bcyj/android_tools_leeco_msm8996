/* stats_port.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __STATS_PORT_H__
#define __STATS_PORT_H__

boolean stats_port_init(mct_port_t *port, unsigned int identity, mct_list_t *sub_ports);
void    stats_port_deinit(mct_port_t *port);
boolean stats_port_check_port(mct_port_t *port, unsigned int identity);

#endif /* __STATS_PORT_H__ */
