/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __PPROC_PORT_H__
#define __PPROC_PORT_H__

#include "mct_stream.h"
#include "chromatix_metadata.h"

#define PPROC_MAX_STREAM_PER_PORT 2
#define PPROC_MAX_SUBMODS 8

mct_port_t *pproc_port_init(const char *name);
void pproc_port_deinit(mct_port_t *port);

boolean pproc_port_check_identity_in_port(void *data1, void *data2);
boolean pproc_port_check_port(mct_port_t *port, unsigned int identity);
mct_port_t *pproc_port_get_reserved_port(mct_module_t *module,
  unsigned int identity);
mct_stream_info_t *pproc_port_get_attached_stream_info(mct_port_t *port,
  unsigned int identity);
boolean pproc_port_send_divert_config_event(mct_module_t *submodule,
  uint32_t identity, pproc_cfg_update_t update_mode, mct_port_t *int_link,
  pproc_divert_info_t *divert_info);

mct_port_t *pproc_port_resrv_port_on_module(mct_module_t *submod,
  mct_stream_info_t *stream_info, mct_port_direction_t direction,
  mct_port_t *pproc_port);
#endif /* __PPROC_PORT_H__ */
