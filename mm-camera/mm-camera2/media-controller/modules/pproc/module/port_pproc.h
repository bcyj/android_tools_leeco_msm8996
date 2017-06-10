/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __PORT_PPROC_H__
#define __PORT_PPROC_H__

#include <pthread.h>
#include "mct_port.h"
#include "module_pproc_common.h"

/** _port_pproc_priv_data:
 *    @links_by_identity: list of internal links by identity
 *
 * Topology link creation info object used to pass as event data
 * to create a link.
 **/
typedef struct _port_pproc_priv_data {
  cam_streaming_mode_t streaming_mode;
  mct_list_t *links_by_identity;
  mct_list_t *out_evnt_list;
  boolean return_divert_buffer;
  pthread_mutex_t mutex;
} port_pproc_priv_data_t;

boolean port_pproc_init(mct_port_t *port, mct_port_direction_t direction,
  cam_streaming_mode_t mode);
boolean port_pproc_find_divert_link_by_identity(void *list_data,
  void *user_data);
boolean port_pproc_find_queue_event(void *list_data, void *user_data);

#endif /* __PORT_PPROC_H__ */
