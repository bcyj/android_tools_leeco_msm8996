/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "module_imglib_common.h"
#include "module_wnr.h"

/* define the following when the meta data are sent from PPROC */
//#define WNR_PUSH_METADATA

static boolean module_wnr_find_identity(void *p_data, void *p_input);
static boolean module_wnr_find_client(void *p_wnr_data, void *p_input);


/** ======================================================================
 * PORT FUNCTIONS
 *=======================================================================*/

/**
 * Function: module_wnr_port_set_caps
 *
 * Description: This method is used to set the capabilities
 *
 * Arguments:
 *   @port: mct port pointer
 *   @caps: mct port capabilities
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_wnr_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  int rc = IMG_SUCCESS;
  return GET_STATUS(rc);
}

/**
 * Function: module_wnr_port_set_parm_event
 *
 * Description: This function is used to set the denoise
 *                    ON event
 *
 * Arguments:
 *   @p_client: wnr client pointer
 *   @event: container event
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/


static boolean module_wnr_port_set_parm_event(wnr_client_t *p_client,
  mct_event_t *event)
{
  boolean ret_val = FALSE;
  mct_event_control_parm_t *event_parm = NULL;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  event_parm = event->u.ctrl_event.control_event_data;

  if (CAM_INTF_PARM_WAVELET_DENOISE == event_parm->type) {
    memcpy(&p_client->cam_denoise_param, event_parm->parm_data,
      sizeof(p_client->cam_denoise_param));

    ret_val = TRUE;
  }

  return ret_val;
}

/**
 * Function: module_wnr_port_stats_aec_update_event
 *
 * Description: This function is used to update the
 *                    AEC metadata coming from PPROC
 *
 * Arguments:
 *   @p_client: wnr client pointer
 *   @event: container event
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_wnr_port_stats_aec_update_event(wnr_client_t *p_client,
  mct_event_t *event)
{
  stats_update_t *stats_update = NULL;
  stats_get_data_t *stats_get_data = NULL;

  if (NULL == event->u.module_event.module_event_data)
    return FALSE;

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  stats_get_data = (stats_get_data_t *)
                     &p_client->session_meta.stats_aec_data.private_data;

  if (stats_update->flag == STATS_UPDATE_AEC) {
    stats_get_data->aec_get.valid_entries = 1;
    stats_get_data->aec_get.real_gain[0] = stats_update->aec_update.real_gain;
    stats_get_data->aec_get.lux_idx = stats_update->aec_update.lux_idx;
    stats_get_data->flag = stats_update->flag;
  }

  return TRUE;
}

/* ================================== */
/* we can handle the following events/metadata here
*  this would require extending the pproc module
*  or we can pull this metadata in wnr_client in start
*  don't forward the events  */
/* ================================== */

/**
 * Function: module_wnr_port_set_chromatix_ptr_event
 *
 * Description: This function is used to update the chromatix
 *                    pointer metadata coming from PPROC
 *
 * Arguments:
 *   @p_client: wnr client pointer
 *   @event: container event
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/

static boolean module_wnr_port_set_chromatix_ptr_event(wnr_client_t *p_client,
  mct_event_t *event)
{
  modulesChromatix_t *chromatix_param;
  if (NULL == event->u.module_event.module_event_data) {
    IDBG_ERROR("%s:%d] Invalid chromatix event data", __func__, __LINE__);
    return FALSE;
 }

 chromatix_param = (modulesChromatix_t *)
     event->u.module_event.module_event_data;

 if (!chromatix_param->chromatixPtr || !chromatix_param->chromatixComPtr) {
   IDBG_ERROR("%s:%d] Invalid chromatix param ptr %p ComPtr %p",
       __func__, __LINE__, chromatix_param->chromatixPtr,
       chromatix_param->chromatixComPtr);
   return FALSE;
 }

 p_client->chromatix_param = *chromatix_param;
 return TRUE;
}

#ifdef WNR_PUSH_METADATA
/**
 * Function: module_wnr_port_isp_awb_update_event
 *
 * Description: This function is used to update the
 *                    AWB metadata coming from PPROC
 *
 * Arguments:
 *   @p_client: wnr client pointer
 *   @event: container event
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_wnr_port_isp_awb_update_event(wnr_client_t *p_client,
  mct_event_t *event)
{
  mct_bus_msg_isp_stats_awb_metadata_t *temp_awb = NULL;
  awb_update_t *awb_update = NULL;
  mct_stream_session_metadata_info *session_meta = NULL;

  if (NULL == event->u.module_event.module_event_data)
    return FALSE;

  temp_awb = (mct_bus_msg_isp_stats_awb_metadata_t *)
               (event->u.module_event.module_event_data);
  if (!temp_awb->is_valid)
   return FALSE;

  awb_update = (awb_update_t *)temp_awb->private_data[0];
  session_meta = &p_client->session_meta;
  memcpy(&session_meta->isp_stats_awb_data.private_data,
    awb_update,sizeof(awb_update_t));

  return TRUE;
}

/**
 * Function: module_wnr_port_isp_gamma_update_event
 *
 * Description: This function is used to update the
 *                    Gamma metadata coming from PPROC
 *
 * Arguments:
 *   @p_client: wnr client pointer
 *   @event: container event
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_wnr_port_isp_gamma_update_event(wnr_client_t *p_client,
  mct_event_t *event)
{
  mct_bus_msg_isp_gamma_t *temp_gama = NULL;
  void *gama = NULL;
  mct_stream_session_metadata_info *session_meta = NULL;

  if (NULL == event->u.module_event.module_event_data)
    return FALSE;

  temp_gama = (mct_bus_msg_isp_gamma_t)
                (event->u.module_event.module_event_data);
 if (!temp_gama->isValid)
  return FALSE;

  gama = &temp_gama->private_data[0];
  session_meta = &p_client->session_meta;
  memcpy(&session_meta->isp_gamma_data.private_data,
    gama,sizeof(awb_update_t));

  return TRUE;
}
#endif

/**
 * Function: module_wnr_acquire_port
 *
 * Description: This function is used to acquire the port
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @port: mct port pointer
 *   @stream_info: stream information
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_wnr_port_acquire(mct_module_t *p_mct_mod,
  mct_port_t *port,
  mct_stream_info_t *stream_info)
{
  int rc = IMG_SUCCESS;
  unsigned int p_identity ;
  mct_list_t *p_temp_list = NULL;
  wnr_client_t *p_client = NULL;
  module_wnr_t *p_mod = NULL;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  p_mod = (module_wnr_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] wnr module NULL", __func__, __LINE__);
    return FALSE;
  }
  p_identity =  stream_info->identity;

  /* check if its sink port*/
  if (MCT_PORT_IS_SINK(port)) {
    /* create wnr client */
    rc = module_wnr_client_create(p_mct_mod, port, p_identity, stream_info);
  } else {
    /* update the internal connection with source port */
    p_temp_list = mct_list_find_custom(p_mod->wnr_client, &p_identity,
      module_wnr_find_client);
    if (NULL != p_temp_list) {
      p_client = p_temp_list->data;
      p_client->p_srcport = port;
      port->port_private = p_client;
      IDBG_MED("%s:%d] found client %p", __func__, __LINE__, p_client);
    } else {
      IDBG_ERROR("%s:%d] cannot find the client", __func__, __LINE__);
      goto error;
    }
  }
  IDBG_MED("%s:%d] port %p port_private %p X", __func__, __LINE__,
    port, port->port_private);
  return GET_STATUS(rc);

error:
  IDBG_MED("%s:%d] Error X", __func__, __LINE__);
  return FALSE;
}

/**
 * Function: module_wnr_port_release_client
 *
 * Description: This method is used to release the client after all the ports are
 *                   destroyed
 *
 * Arguments:
 *   @p_mod: pointer to the WNR module
 *   @port: port of the client
 *   @p_client: client to release
 *   @identity: stream/session id
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_wnr_port_release_client(module_wnr_t *p_mod,
  mct_port_t *port, wnr_client_t *p_client, unsigned int identity)
{
  mct_list_t *p_temp_list = NULL;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  p_temp_list = mct_list_find_custom(p_mod->wnr_client, &identity,
    module_wnr_find_client);
  if (NULL != p_temp_list) {
    IDBG_MED("%s:%d] ", __func__, __LINE__);
    p_mod->wnr_client = mct_list_remove(p_mod->wnr_client,
      p_temp_list->data);
  }
  module_wnr_client_destroy(p_client);
}

/**
 * Function: module_wnr_port_check_caps_unreserve
 *
 * Description: This method is used to unreserve the port
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_wnr_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  int rc = IMG_SUCCESS;
  mct_module_t *p_mct_mod = NULL;
  module_wnr_t *p_mod = NULL;
  wnr_client_t *p_client = NULL;
  uint32_t *p_identity = NULL;

  if (!port) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s:%d] E %d", __func__, __LINE__, MCT_PORT_DIRECTION(port));

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_wnr_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] wnr module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (wnr_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] wnr client NULL", __func__, __LINE__);
    return FALSE;
  }

  /* lock the module */
  pthread_mutex_lock(&p_mod->mutex);

  if (MCT_PORT_IS_SRC(port)) {
    module_wnr_port_release_client(p_mod, port, p_client, identity);
    port->port_private = NULL;
  } else {
    if (NULL == p_client->p_srcport) {
      module_wnr_port_release_client(p_mod, port, p_client, identity);
    }
    port->port_private = NULL;
  }
  pthread_mutex_unlock(&p_mod->mutex);

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return GET_STATUS(rc);

}

/**
 * Function: module_wnr_port_check_caps_reserve
 *
 * Description: This function is used to reserve the port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @peer_caps: pointer to peer capabilities
 *   @stream_info: stream information
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_wnr_port_check_caps_reserve(mct_port_t *port, void *peer_caps,
  void *vstream_info)
{
  boolean rc = FALSE;
  mct_module_t *p_mct_mod = NULL;
  module_wnr_t *p_mod = NULL;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)vstream_info;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  if (!port || !stream_info) {
    CDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    CDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_wnr_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    CDBG_ERROR("%s:%d] wnr module NULL", __func__, __LINE__);
    return FALSE;
  }

  /* lock the module */
  pthread_mutex_lock(&p_mod->mutex);
  if (port->port_private) {
    /* port is already reserved */
    IDBG_MED("%s:%d] Error port is already reserved", __func__, __LINE__);
    pthread_mutex_unlock(&p_mod->mutex);
    return FALSE;
  }
  rc = module_wnr_port_acquire(p_mct_mod, port, stream_info);
  if (FALSE == rc) {
    CDBG_ERROR("%s:%d] Error acquiring port", __func__, __LINE__);
    pthread_mutex_unlock(&p_mod->mutex);
    return FALSE;
  }

  pthread_mutex_unlock(&p_mod->mutex);

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return TRUE;

}

/**
 * Function: module_wnr_port_ext_link
 *
 * Description: This method is called when the user establishes
 *              link.
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_wnr_port_ext_link(unsigned int identity,
  mct_port_t* port, mct_port_t *peer)
{
  int rc = IMG_SUCCESS;
  unsigned int *p_identity = NULL;
  mct_list_t *p_temp_list = NULL;
  mct_module_t *p_mct_mod = NULL;
  module_wnr_t *p_mod = NULL;
  wnr_client_t *p_client = NULL;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (!port || !peer) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s:%d] port %p E", __func__, __LINE__, port);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_wnr_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] wnr module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (wnr_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] invalid client", __func__, __LINE__);
    return FALSE;
  }

  if (MCT_PORT_PEER(port)) {
    IDBG_ERROR("%s:%d] link already established", __func__, __LINE__);
    return FALSE;
  }

  MCT_PORT_PEER(port) = peer;

  /* check if its sink port*/
  if (MCT_PORT_IS_SINK(port)) {
    /* start wnr client in case of dynamic module */

    mod_img_msg_t msg;
    memset(&msg, 0x0, sizeof(mod_img_msg_t));
    msg.port = port;
    msg.type = MOD_IMG_MSG_EXEC_INFO;
    msg.data.exec_info.data = NULL;
    msg.data.exec_info.p_exec = module_wnr_client_buffers_allocate;
    msg.data.exec_info.p_userdata = (void *)p_client;
    module_imglib_send_msg(&p_mod->msg_thread, &msg);

  } else {
    /* do nothing for source port */
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return GET_STATUS(rc);
}

/**
 * Function: module_wnr_port_unlink
 *
 * Description: This method is called when the user disconnects
 *              the link.
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_wnr_port_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  int rc = IMG_SUCCESS;
  mct_list_t *p_temp_list = NULL;
  mct_module_t *p_mct_mod = NULL;
  module_wnr_t *p_mod = NULL;
  wnr_client_t *p_client = NULL;
  uint32_t *p_identity = NULL;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (!port || !peer) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return;
  }

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return;
  }

  p_mod = (module_wnr_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] wnr module NULL", __func__, __LINE__);
    return;
  }

  p_client = (wnr_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] wnr client NULL", __func__, __LINE__);
    return;
  }

  if (MCT_PORT_IS_SINK(port)) {
    /* stop the client in case of dynamic module */
  } else {
    /* do nothing for source port*/
  }

  MCT_PORT_PEER(port) = NULL;

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return;
}


/**
 * Function: module_wnr_forward_port_event
 *
 * Description: This method is used to forward an event
 * depending on the direction.
 *
 * Arguments:
 *   @mct_port: Port that recieved the event
 *   @event: Event recieved
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_wnr_forward_port_event(wnr_client_t *p_client,
  mct_port_t *port, mct_event_t *event)
{
  boolean rc = FALSE;
  mct_port_t *p_adj_port = NULL;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (MCT_PORT_IS_SINK(port)) {
    p_adj_port = p_client->p_srcport;
    if (NULL == p_adj_port) {
       IDBG_HIGH("%s:%d] Invalid port.. Skip forward", __func__, __LINE__);
       return TRUE;
    }
    switch(event->direction) {
      case MCT_EVENT_UPSTREAM : {
        IDBG_ERROR("%s:%d] Error Upstream event on Sink port %d",
          __func__, __LINE__, event->type);
        break;
      }
      case MCT_EVENT_DOWNSTREAM: {
       rc =  mct_port_send_event_to_peer(p_adj_port, event);
       if (rc == FALSE) {
         IDBG_ERROR("%s:%d] Fowarding event %d from sink port failed",
           __func__, __LINE__, event->type);
       }
       break;
     }
     default:
       IDBG_ERROR("%s:%d] Invalid port direction for event %d",
         __func__, __LINE__, event->type);
       break;
    }
  } else if (MCT_PORT_IS_SRC(port)) {
    p_adj_port = p_client->p_sinkport;
    if (NULL == p_adj_port) {
       IDBG_HIGH("%s:%d] Invalid port", __func__, __LINE__);
       return FALSE;
    }
    switch(event->direction) {
      case MCT_EVENT_DOWNSTREAM : {
        IDBG_ERROR("%s:%d] Error Downstream event on Src port %d",
          __func__, __LINE__, event->type);
        break;
      }
      case MCT_EVENT_UPSTREAM: {
       rc =  mct_port_send_event_to_peer(p_adj_port, event);
       if (rc == FALSE) {
         IDBG_ERROR("%s:%d] Fowarding event %d from src port failed",
           __func__, __LINE__, event->type);
       }
       break;
     }
     default:
       IDBG_ERROR("%s:%d] Invalid port direction for event %d",
         __func__, __LINE__, event->type);
       break;
    }
  }
  return rc;
}

/**
 * Function: module_wnr_port_event_func
 *
 * Description: Event handler function for the dummy port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_wnr_port_event_func(mct_port_t *port,
  mct_event_t *event)
{
  int rc = IMG_SUCCESS;
  boolean brc = FALSE;
  mct_module_t *p_mct_mod = NULL;
  module_wnr_t *p_mod = NULL;
  wnr_client_t *p_client;
  boolean fwd_event = TRUE;
  mct_event_t new_event;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (!port || !event) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    rc = IMG_ERR_GENERAL;
    goto wnr_port_event_exit;
  }
  IDBG_LOW("%s:%d] port %p E", __func__, __LINE__, port);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    rc = IMG_ERR_GENERAL;
    goto wnr_port_event_exit;
  }

  p_mod = (module_wnr_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] WNR module NULL", __func__, __LINE__);
    rc = IMG_ERR_GENERAL;
    goto wnr_port_event_exit;
  }

  p_client = (wnr_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] WNR client NULL", __func__, __LINE__);
    rc = IMG_ERR_GENERAL;
    goto wnr_port_event_exit;
  }
  if (p_client->identity != event->identity) {
    IDBG_ERROR("%s:%d] Event and module identities don't match", __func__, __LINE__);
    rc = IMG_ERR_GENERAL;
    goto wnr_port_event_exit;
  }

  IDBG_LOW("%s:%d] type %d", __func__, __LINE__, event->type);
  switch (event->type) {
    case MCT_EVENT_CONTROL_CMD: {
      mct_event_control_t *p_ctrl_event = &event->u.ctrl_event;
      IDBG_MED("%s:%d] Ctrl type %d", __func__, __LINE__, p_ctrl_event->type);
      switch (p_ctrl_event->type) {
        case MCT_EVENT_CONTROL_STREAMON: {
          IDBG_HIGH("%s:%d] WNR STREAMON", __func__, __LINE__);
          mct_stream_info_t *streaminfo = (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
          /* Check if Wavelet is turned on */
          memcpy(&p_client->cam_denoise_param, &streaminfo->pp_config.denoise2d,
          sizeof(p_client->cam_denoise_param));
          pthread_mutex_lock(&p_client->mutex);
          mct_port_t* port = module_wnr_find_port_with_identity(p_mct_mod, MCT_PORT_SINK,
          event->identity,p_client);
          p_client->stream_off = FALSE;
         /* Get AEC Gain Values */
	  new_event.type = MCT_EVENT_MODULE_EVENT;
          new_event.identity = event->identity;
          new_event.direction = MCT_EVENT_UPSTREAM;
          new_event.u.module_event.type = MCT_EVENT_MODULE_PPROC_GET_AEC_UPDATE;
          new_event.u.module_event.module_event_data = (void *)&p_client->stats_get;
	  mct_port_send_event_to_peer(port, &new_event);
          /* Get AWB Gain values */
	  new_event.type = MCT_EVENT_MODULE_EVENT;
          new_event.identity = event->identity;
          new_event.direction = MCT_EVENT_UPSTREAM;
          new_event.u.module_event.module_event_data = (void *)&p_client->stats_get;
	  new_event.u.module_event.type = MCT_EVENT_MODULE_PPROC_GET_AWB_UPDATE;
	  mct_port_send_event_to_peer(port, &new_event);
          pthread_mutex_unlock(&p_client->mutex);
          break;
        }
        case MCT_EVENT_CONTROL_STREAMOFF: {
          module_wnr_t *p_mod = (module_wnr_t *)p_client->p_mod;
          IDBG_MED("%s:%d] WNR STREAMOFF", __func__, __LINE__);
          pthread_mutex_lock(&p_client->mutex);
          p_client->stream_off = TRUE;
          pthread_mutex_unlock(&p_client->mutex);
          img_q_flush(&p_mod->msg_thread.msg_q);

          module_wnr_client_stop(p_client);
          break;
        }
        case MCT_EVENT_CONTROL_SET_PARM:
          brc = module_wnr_port_set_parm_event(p_client, event);
          if (TRUE == brc) fwd_event = TRUE;
          break;
        case MCT_EVENT_CONTROL_PARM_STREAM_BUF: {
          cam_stream_parm_buffer_t *parm_buf = p_ctrl_event->control_event_data;

          /* Set metadata info in client, process will start on buf divert */
          if (parm_buf->type == CAM_STREAM_PARAM_TYPE_DO_REPROCESS) {
            wnr_metadata_info_t metadata_info;

            metadata_info.meta_buf_index = parm_buf->reprocess.meta_buf_index;
            metadata_info.meta_stream_handle =
                parm_buf->reprocess.meta_stream_handle;

            /* We need to protect the client when the meta info is set */
            pthread_mutex_lock(&p_client->mutex);
            rc = module_wnr_client_set_meta_info(p_client, &metadata_info);
            pthread_mutex_unlock(&p_client->mutex);
            if (IMG_ERROR(rc)) {
              IDBG_ERROR("%s:%d] Error set metadata info", __func__, __LINE__);
              goto wnr_port_event_exit;
            }
          }

          break;
        }
        default:
          break;
      }
      break;
    }
    case MCT_EVENT_MODULE_EVENT: {
      IDBG_MED("%s:%d] WNR Enable %d", __func__, __LINE__,
                             p_client->cam_denoise_param.denoise_enable);
      mct_event_module_t *p_mod_event = &event->u.module_event;
      img_component_ops_t *p_comp = &p_client->comp;
      IDBG_MED("%s:%d] Mod type %d", __func__, __LINE__, p_mod_event->type);
      switch (p_mod_event->type) {
        case MCT_EVENT_MODULE_BUF_DIVERT: {
          /* no need to handle MCT_EVENT_MODULE_BUF_DIVERTt here if */
          /*denoise is not enabled, just forward the event */
          if (p_client->cam_denoise_param.denoise_enable) {
            mod_img_msg_t msg;
            isp_buf_divert_t *p_buf_divert =
              (isp_buf_divert_t *)p_mod_event->module_event_data;

            IDBG_ERROR("%s:%d] identity %x", __func__, __LINE__,
              event->identity);

            memset(&msg, 0x0, sizeof(mod_img_msg_t));
            msg.port = port;
            msg.type = MOD_IMG_MSG_DIVERT_BUF;
            msg.data.buf_divert.buf_divert = *p_buf_divert;
            msg.data.buf_divert.identity = p_client->identity;
            msg.data.buf_divert.p_exec = module_wnr_client_divert_exec;
            msg.data.buf_divert.userdata = (void *)p_client;
            module_imglib_send_msg(&p_mod->msg_thread, &msg);

            /* indicate that the buffer is consumed */
            p_buf_divert->is_locked = FALSE;
            p_buf_divert->ack_flag = FALSE;
            fwd_event = FALSE;
          } else {
            /* If wnr is not enabled on buff_divert discard metadata
             * info for that buffer */
            wnr_metadata_info_t discarded_metadata;
            module_wnr_client_get_meta_info(p_client, &discarded_metadata);
          }
          break;
        }

        case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
          mct_stream_info_t *stream_info =
            (mct_stream_info_t *)(event->u.module_event.module_event_data);
          if (!stream_info) {
            IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
          } else {
            p_client->input_stream_info.dim.width = stream_info->dim.width;
            p_client->input_stream_info.dim.height = stream_info->dim.height;
            memcpy(p_client->input_stream_info.buf_planes.plane_info.mp,
              stream_info->buf_planes.plane_info.mp,sizeof(stream_info->buf_planes.plane_info.mp));
            p_client->input_stream_info.fmt = stream_info->fmt;
            IDBG_MED("%s:%d] MCT_EVENT_MODULE_ISP_OUTPUT_DIM %dx%d",
              __func__, __LINE__,
              p_client->input_stream_info.dim.width,
              p_client->input_stream_info.dim.height);
          }
          break;
        }

        case MCT_EVENT_MODULE_STATS_AEC_UPDATE:
          brc = module_wnr_port_stats_aec_update_event(p_client, event);
          if (TRUE == brc) fwd_event = TRUE;
          break;

          /* we can handle the following events/metadata here*/
          /* this would require extending the pproc module */
          /* or we can pull this metadata in wnr_client in start */
          /* don't forward the events  */
        case MCT_EVENT_MODULE_SET_CHROMATIX_PTR:
        case MCT_EVENT_MODULE_SET_LIVESHOT_CHROMATIX_PTR:
          brc = module_wnr_port_set_chromatix_ptr_event(p_client, event);
          if (TRUE == brc) fwd_event = TRUE;
          break;
#ifdef WNR_PUSH_METADATA
        case MCT_EVENT_MODULE_ISP_AWB_UPDATE:
          brc = module_wnr_port_isp_awb_update_event(port, event);
          if (TRUE == brc) fwd_event = TRUE;
          break;
        case MCT_EVENT_MODULE_ISP_GAMMA_UPDATE:
          brc = module_wnr_port_isp_gamma_update_event(port, event);
          if (TRUE == brc) fwd_event = TRUE;
  #else
        case MCT_EVENT_MODULE_ISP_AWB_UPDATE:
          fwd_event = TRUE;
          break;
        case MCT_EVENT_MODULE_ISP_GAMMA_UPDATE:
          fwd_event = TRUE;
          break;
#endif
        default:
          break;
      }
    }
    default:
      break;
  }

  if (fwd_event) {
    boolean brc = module_wnr_forward_port_event(p_client, port, event);
    rc = (brc) ? IMG_SUCCESS : IMG_ERR_GENERAL;
  }

wnr_port_event_exit:
  return GET_STATUS(rc);
}

/**
 * Function: module_wnr_create_port
 *
 * Description: This function is used to create a port and link with the
 *              module
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @dir: port direction (source or sink)
 *
 * Return values:
 *     MCTL port pointer
 *
 * Notes: none
 **/
mct_port_t *module_wnr_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir)
{
  char portname[PORT_NAME_LEN];
  mct_port_t *p_port = NULL;
  int status = IMG_SUCCESS;
  int index = 0;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (!p_mct_mod || (MCT_PORT_UNKNOWN == dir)) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }

  index = (MCT_PORT_SINK == dir) ? p_mct_mod->numsinkports :
    p_mct_mod->numsrcports;
  /*portname <mod_name>_direction_portIndex*/
  snprintf(portname, sizeof(portname), "%s_d%d_i%d",
    MCT_MODULE_NAME(p_mct_mod), dir, index);
  p_port = mct_port_create(portname);
  if (NULL == p_port) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }
  IDBG_MED("%s:%d portname %s", __func__, __LINE__, portname);

  p_port->direction = dir;
  p_port->port_private = NULL;
  p_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;

  /* override the function pointers */
  p_port->check_caps_reserve    = module_wnr_port_check_caps_reserve;
  p_port->check_caps_unreserve  = module_wnr_port_check_caps_unreserve;
  p_port->ext_link              = module_wnr_port_ext_link;
  p_port->un_link               = module_wnr_port_unlink;
  p_port->set_caps              = module_wnr_port_set_caps;
  p_port->event_func            = module_wnr_port_event_func;

   /* add port to the module */
  if (!mct_module_add_port(p_mct_mod, p_port)) {
    IDBG_ERROR("%s: Set parent failed", __func__);
    status = IMG_ERR_GENERAL;
    goto error;
  }

  IDBG_MED("%s:%d ", __func__, __LINE__);
  return p_port;

error:

  IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
  if (p_port) {
    mct_port_destroy(p_port);
  }
  return NULL;
}

/**
 * Function: module_wnr_free_port
 *
 * Description: This function is used to free the wnr ports
 *
 * Arguments:
 *   @data - wnr port
 *   @user_data - wnr module
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static boolean module_wnr_free_port(void *data, void *user_data)
{
  mct_port_t *p_port = (mct_port_t *)data;
  mct_module_t *p_mct_mod = (mct_module_t *)user_data;
  boolean rc = FALSE;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (!p_port || !p_mct_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return TRUE;
  }
  IDBG_MED("%s:%d port %p p_mct_mod %p", __func__, __LINE__, p_port,
    p_mct_mod);

  rc = mct_module_remove_port(p_mct_mod, p_port);
  if (rc == FALSE) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
  }
  mct_port_destroy(p_port);
  return TRUE;
}


/** ======================================================================
 * MODULE FUNCTIONS
 *======================================================================*/

/**
 * Function: module_wnr_find_identity
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @data1: not used
 *   @data2: not used
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_wnr_get_next_from_list(void *data1, void *data2)
{
  return TRUE;
}

/**
 * Function: module_wnr_find_identity
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_data: data in the mct list
 *   @p_input: input data to be seeked
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_wnr_find_identity(void *p_data, void *p_input)
{
  uint32_t *p_identity = (uint32_t *)p_data;
  uint32_t identity = *((uint32_t *)p_input);

  return (*p_identity == identity) ? TRUE : FALSE;
}

/**
 * Function: module_wnr_find_identity
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_data: data in the mct list
 *   @p_input: input data to be seeked
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
mct_port_t* module_wnr_find_port_with_identity(mct_module_t *module,
  mct_port_direction_t dir, uint32_t identity , wnr_client_t * p_client)
{
  mct_port_t *port = NULL;
  mct_list_t *templist;
  switch(dir) {
  case MCT_PORT_SRC:
        if(identity == p_client->identity)
	   port = p_client->p_srcport;
    break;
  case MCT_PORT_SINK:
     if(identity == p_client->identity)
	   port = p_client->p_sinkport;
    break;
  default:
    CDBG_ERROR("%s:%d: failed, bad port_direction=%d", __func__, __LINE__, dir);
    return NULL;
  }
  return port;
}

/**
 * Function: module_wnr_find_client
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_fp_data: wnr client
 *   @p_input: input data
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_wnr_find_client(void *p_wnr_data, void *p_input)
{
  wnr_client_t *p_client = (wnr_client_t *)p_wnr_data;
  uint32_t identity = *((uint32_t *)p_input);

  return (p_client->identity == identity) ? TRUE : FALSE;
}

/**
 * Function: module_wnr_start_session
 *
 * Description: This function is called when a new camera
 *              session is started
 *
 * Arguments:
 *   @module: mct module pointer
 *   @sessionid: session id
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_wnr_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_wnr_t *p_mod;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (!module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_wnr_t *)module->module_private;
  if (!p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  /* create message thread */
  rc = module_imglib_create_msg_thread(&p_mod->msg_thread);

  return GET_STATUS(rc);
}

/**
 * Function: module_wnr_stop_session
 *
 * Description: This function is called when the camera
 *              session is stopped
 *
 * Arguments:
 *   @module: mct module pointer
 *   @sessionid: session id
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_wnr_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_wnr_t *p_mod;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (!module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_wnr_t *)module->module_private;
  if (!p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  /* destroy message thread */
  rc = module_imglib_destroy_msg_thread(&p_mod->msg_thread);
  return GET_STATUS(rc);
}

/**
 * Function: module_wnr_query_mod
 *
 * Description: This function is used to query the wnr module
 * info
 *
 * Arguments:
 *   @module: mct module pointer
 *   @query_buf: pipeline capability
 *   @sessionid: session identity
 *
 * Return values:
 *     success/failure
 *
 * Notes: none
 **/
boolean module_wnr_query_mod(mct_module_t *module, void *buf,
  unsigned int sessionid)
{
  mct_pipeline_cap_t *p_mct_cap = (mct_pipeline_cap_t *)buf;
  mct_pipeline_pp_cap_t *p_cap;

  IDBG_MED("%s:%d: E", __func__, __LINE__);

  if (!p_mct_cap || !module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }
  p_cap = &p_mct_cap->pp_cap;
  p_cap->min_num_pp_bufs += MODULE_WNR_MIN_NUM_PP_BUFS;
#ifdef CAMERA_FEATURE_WNR_SW
  p_cap->is_sw_wnr = TRUE;
#endif
  if (p_mct_cap->sensor_cap.sensor_format != FORMAT_YCBCR)
    p_cap->feature_mask |= CAM_QCOM_FEATURE_DENOISE2D;

 return TRUE;

}

/**
 * Function: module_wnr_set_mod
 *
 * Description: This function is used to query the wnr module
 * info
 *
 * Arguments:
 *   @module: mct module pointer
 *   @query_buf: pipeline capability
 *   @sessionid: session identity
 *
 * Return values:
 *     success/failure
 *
 * Notes: none
 **/
void module_wnr_set_mod(mct_module_t *module, unsigned int module_type,
  unsigned int identity)
{
  if(!module) {
    CDBG_ERROR("%s:%d: failed", __func__, __LINE__);
    return;
  }
  //module->type = (mct_module_type_t) module_type;
}
/** module_wnr_set_parent:
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_wnr_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent)
{
  module_wnr_t *p_mod = NULL;

  p_mod = (module_wnr_t *)p_mct_mod->module_private;
  p_mod->parent_mod = p_parent;
}

/**
 * Function: module_wnr_deinit
 *
 * Description: This function is used to free the WNR module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_wnr_deinit(mct_module_t *p_mct_mod)
{
  module_wnr_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  mct_list_t* p_list;
  int rc = 0;
  int i = 0;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  if (NULL == p_mct_mod) {
    IDBG_ERROR("%s:%d] MCTL module NULL", __func__, __LINE__);
    return;
  }

  p_mod = (module_wnr_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] wnr module NULL", __func__, __LINE__);
    return;
  }

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SINKPORTS(p_mct_mod), p_mct_mod,
      module_wnr_get_next_from_list);
    if (p_list)
      module_wnr_free_port(p_list->data, p_mct_mod);
  } while (p_list);

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SRCPORTS(p_mct_mod), p_mct_mod,
      module_wnr_get_next_from_list);
    if (p_list)
      module_wnr_free_port(p_list->data, p_mct_mod);
  } while (p_list);

  p_core_ops = &p_mod->core_ops;
  IDBG_MED("%s:%d lib_ref_cnt %d", __func__, __LINE__, p_mod->lib_ref_count);
  if (1 == p_mod->lib_ref_count) {
    IMG_COMP_UNLOAD(p_core_ops);
  }

  if (p_mod->lib_ref_count) {
    p_mod->lib_ref_count--;
  }
  p_mod->wnr_client_cnt = 0;
  pthread_mutex_destroy(&p_mod->mutex);
  pthread_mutex_destroy(&p_mod->lib_singleton_mutex);
  pthread_cond_destroy(&p_mod->cond);

  mct_module_destroy(p_mct_mod);
  return;
}

/** module_wnr_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the wnr
 * module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_wnr_init(const char *name)
{
  mct_module_t *p_mct_mod = NULL;
  module_wnr_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  mct_port_t *p_sinkport = NULL, *p_sourceport = NULL;
  int rc = 0;
  int i = 0;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  p_mct_mod = mct_module_create(name);
  if (NULL == p_mct_mod) {
    IDBG_ERROR("%s:%d cannot allocate mct module", __func__, __LINE__);
    return NULL;
  }

  p_mod = malloc(sizeof(module_wnr_t));
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto error;
  }

  p_mct_mod->module_private = (void *)p_mod;
  memset(p_mod, 0, sizeof(module_wnr_t));

  pthread_mutex_init(&p_mod->mutex, NULL);
  pthread_mutex_init(&p_mod->lib_singleton_mutex, NULL);
  pthread_cond_init(&p_mod->cond, NULL);
  p_core_ops = &p_mod->core_ops;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  /* check if the wnr module is present */
  rc = img_core_get_comp(IMG_COMP_DENOISE, "qcom.wavelet", p_core_ops);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
    goto error;
  }
  /* try to load the component */
  if (0 == p_mod->lib_ref_count) {
    rc = IMG_COMP_LOAD(p_core_ops, NULL);
  }
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
    goto error;
  }
  p_mod->lib_ref_count++;
  p_mod->wnr_client = NULL;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  /* create static ports */
  for (i = 0; i < MAX_WNR_STATIC_PORTS; i++) {
    p_sinkport = module_wnr_create_port(p_mct_mod, MCT_PORT_SINK);
    if (NULL == p_sinkport) {
      IDBG_ERROR("%s:%d] create SINK port failed", __func__, __LINE__);
      goto error;
    }
    p_sourceport = module_wnr_create_port(p_mct_mod, MCT_PORT_SRC);
    if (NULL == p_sourceport) {
      IDBG_ERROR("%s:%d] create SINK port failed", __func__, __LINE__);
      goto error;
    }
  }
  p_mct_mod->start_session    = module_wnr_start_session;
  p_mct_mod->stop_session     = module_wnr_stop_session;
  p_mct_mod->query_mod        = module_wnr_query_mod;
  p_mct_mod->set_mod             = module_wnr_set_mod;

  IDBG_MED("%s:%d] %p", __func__, __LINE__, p_mct_mod);
  return p_mct_mod;

error:
  if (p_mod) {
    module_wnr_deinit(p_mct_mod);
  } else if (p_mct_mod) {
    mct_module_destroy(p_mct_mod);
  }
  return NULL;
}


