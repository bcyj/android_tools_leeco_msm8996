/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __PORT_PPROC_COMMON_H__
#define __PORT_PPROC_COMMON_H__

#include "mct_list.h"
#include "mct_port.h"

/** _port_pproc_common_link:
 *    @link_index: Link index.
 *    @sink_port:  Sink port for link.
 *    @src_port:   Source port for link.
 *
 * Single link object.
 **/
typedef struct _port_pproc_common_link {
  uint32_t    link_index;
  mct_port_t *sink_port;
  mct_port_t *src_port;
  uint32_t    identity;
} port_pproc_common_link_t;

/** _port_pproc_common_divert_link:
 *    @identity:       sessionid/streamid associated with this
 *                     link.
 *    @tplgy_lnks:     list of port_pproc_common_link_t which
 *                     are all siblinks internally linked to any
 *                     external port_pproc.
 *    @tplgy_lnks_cnt: list of port_pproc_common_link_t which
 *                     are all siblinks internally linked to any
 *                     external port_pproc.
 *    @out_evnt_list:   On successful creation of a link, an
 *                     event queue is created.
 *    @stream_info:    StreamInfo corresponding to this
 *                     identity.
 *
 * Topology link object.
 **/
typedef struct _port_pproc_common_divert_link {
  uint32_t    identity;
  mct_list_t *tplgy_lnks;
  uint32_t    tplgy_lnks_cnt;
  mct_list_t *out_evnt_list;
  void       *stream_info;
  boolean     streaming_on;
} port_pproc_common_divert_link_t;

/** _port_pproc_common_link_create_info:
 *    @stream_info: mct_stream_info_t associated with stream
 *                  being reseved.
 *    @link:        On successful creation of link, this object
 *                  representing src and sink ports of link is
 *                  returned.
 *
 * Topology link creation info object used to pass as event data
 * to create a link.
 **/
typedef struct _port_pproc_common_link_create_info {
  void                     *peer;
  void                     *stream_info;
  port_pproc_common_link_t *link;
} port_pproc_common_link_create_info_t;

boolean port_pproc_common_match_identity(void *list_data, void *user_data);

boolean port_pproc_common_send_event_to_peer(
  void *list_data, void *user_data);

#endif /* __PORT_PPROC_COMMON_H__ */
