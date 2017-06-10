/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "camera_dbg.h"
#include "mct_stream.h"
#include "port_pproc_common.h"
#include "cam_types.h"

/** port_pproc_common_match_identity:
 *    @list_data: private data from list
 *    @user_data: user data to match
 *
 *  Return: TRUE for success and FALSE on failure
 **/
boolean port_pproc_common_match_identity(void *list_data, void *user_data)
{
  unsigned int *port_identity  = (unsigned int *)list_data;
  unsigned int *identity = (unsigned int *)user_data;
  if (!port_identity || !identity) {
    CDBG_ERROR("%s:%d failed port_identity %p, identity %p\n", __func__,
      __LINE__, port_identity, identity);
    return FALSE;
  }

  CDBG("%s:req_iden:%d, list_iden:%d\n", *identity, *port_identity);
  if (*port_identity == *identity)
    return TRUE;

  return FALSE;
}

/** port_pproc_common_send_event_to_peer:
 *    @list_data: port from which event is to be sent out
 *    @user_data: event to be sent
 *
 *  Return: TRUE for success and FALSE on failure
 **/
boolean port_pproc_common_send_event_to_peer(
  void *list_data, void *user_data)
{
  mct_port_t *mct_port = (mct_port_t *)list_data;
  mct_event_t *event = (mct_event_t *)user_data;
  mct_list_t *list_match = NULL;

  list_match = mct_list_find_custom(MCT_PORT_CHILDREN(mct_port), &event->identity,
    port_pproc_common_match_identity);

  if (list_match != NULL)
      return mct_port_send_event_to_peer(mct_port, event);

  return TRUE;
}
