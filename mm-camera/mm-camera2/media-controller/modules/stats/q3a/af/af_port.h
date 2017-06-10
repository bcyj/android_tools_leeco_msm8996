/* af_port.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __PORT_AF_H__
#define __PORT_AF_H__

#include <mct_stream.h>
#include "q3a_thread.h"
#include "modules.h"

#define AF_CAUSE_QUEUE_SIZE (4)

/** af_port_state_transition_type: cause of transition of AF
 *  state
 **/
typedef enum {
  AF_PORT_TRANS_CAUSE_TRIGGER,
  AF_PORT_TRANS_CAUSE_CANCEL,
  AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED,
  AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED,
  AF_PORT_TRANS_CAUSE_MODE_CHANGE,
  AF_PORT_TRANS_CAUSE_SCANNING,
} af_port_state_transition_type;

/** _af_port_state_trans: Information regarding AF state
*   transition
*    @trigger_id:     trigger id sent
*    @cause:          reason of transition
*    @af_state:       current AF state
*
**/
typedef struct _af_port_state_trans {
  int32_t                       trigger_id;
  af_port_state_transition_type cause;
  cam_af_state_t                af_state;
} af_port_state_trans_t;

/** af_port_state_t:
 *
 * Enum with the states of the AF port
 *
 **/
typedef enum {
  AF_PORT_STATE_CREATED,
  AF_PORT_STATE_RESERVED,
  AF_PORT_STATE_LINKED,
  AF_PORT_STATE_UNLINKED,
  AF_PORT_STATE_UNRESERVED
} af_port_state_t;

/** _af_isp_up_event:
 *    @is_isp_ready:           flag to indicate if ISP is ready to
 *                             accept request.
 *    @need_to_send_af_config: true if we need to send AF config after ISP is ON
 *    @config:                 The saved config that will be sent to the ISP
 *
 * Holds data that needs to be sent to the ISP once it indicates it is ready.
 **/
typedef struct _af_isp_up_event {
  boolean     is_isp_ready;
  boolean     need_to_send_af_config;
  af_config_t config;
} af_isp_up_event_t;

void    af_port_deinit(mct_port_t *port);
boolean af_port_find_identity(mct_port_t *port, unsigned int identity);
boolean af_port_init(mct_port_t *port, unsigned int *session_id);

#endif /* __PORT_AF_H__ */
