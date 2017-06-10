/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __ISPIF_UTIL_H__
#define __ISPIF_UTIL_H__


/** ispif_util_add_stream:
 *    @ispif: ispif instance
 *    @session: session instance
 *    @stream_id: stream ID
 *    @stream_info: MCTL stream data
 *
 *  This function runs in MCTL thread context.
 *
 *  This function adds a stream to ispif module
 *
 *  Return: NULL - Error
 *          Otherwise - Pointer to added stream
 **/
ispif_stream_t *ispif_util_add_stream(ispif_t *ispif, ispif_session_t *session,
  uint32_t stream_id, mct_stream_info_t *stream_info);

/** ispif_util_del_stream:
 *    @ispif: ispif instance
 *    @stream: stream  to be deleted
 *
 *  This function runs in MCTL thread context.
 *
 *  This function deletes a stream from ispif module
 *
 *  Return: 0 - Success
 *         -1 - Stream used by some port
 **/
int ispif_util_del_stream(ispif_t *ispif,ispif_stream_t *stream);

/** ispif_util_find_stream:
 *    @ispif: ispif pointer
 *    @session_id: Session id
 *    @stream_id: Stream ID
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a stream with certain ID in session with given id
 *
 *  Return: NULL - Stream not found
 *          Otherwise - pointer to stream found
 **/
ispif_stream_t *ispif_util_find_stream(ispif_t *ispif, uint32_t session_id,
  uint32_t stream_id);

/** ispif_util_find_stream_in_session:
 *    @session: Session to find stream in
 *    @stream_id: Stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a stream with certain ID in session
 *
 *  Return: NULL - Stream not found
 *          Otherwise - pointer to stream found
 **/
ispif_stream_t *ispif_util_find_stream_in_session(ispif_session_t *session,
  uint32_t stream_id);

/** ispif_util_get_session_by_id:
 *    @ispif: ispif pointer
 *    @session_id: Session ID
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds session with given ID
 *
 *  Return: NULL - Session not found
 *          Otherwise - pointer to session
 **/
ispif_session_t *ispif_util_get_session_by_id(ispif_t *ispif,
  uint32_t session_id);

/** ispif_util_find_stream_in_sink_port:
 *    @ispif: ispif instance
 *    @ispif_sink_port: ispif sink port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function checks if stream is linked to given sink port.
 *
 *  Return: TRUE  - Success
 *          FALSE - Stream is not linked to this port
 **/
boolean ispif_util_find_stream_in_sink_port(ispif_t *ispif,
  ispif_port_t *ispif_sink_port, ispif_stream_t *stream);

/** ispif_util_find_stream_in_src_port:
 *    @ispif: ispif isntance
 *    @ispif_src_port: ispif source port
 *    @session_id: Session ID
 *    @stream_id: Stream ID
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a stream linked to ispif source port by ID
 *
 *  Return: NULL - Stream not found
 *          Otherwise - pointer to stream found
 **/
ispif_stream_t *ispif_util_find_stream_in_src_port( ispif_t *ispif,
  ispif_port_t *ispif_src_port, uint32_t session_id, uint32_t stream_id);

/** ispif_util_add_stream_to_sink_port:
 *    @ispif: ispif isntance
 *    @ispif_sink_port: ispif sinc port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function adds a stream to sink potr
 *
 *  Return: 0 - Success
 *         -1 - Port is connected to its max number of streams
 **/
int ispif_util_add_stream_to_sink_port(ispif_t *ispif,
  ispif_port_t *ispif_sink_port, ispif_stream_t *stream);

/** ispif_util_del_stream_from_sink_port:
 *    @ispif: ispif isntance
 *    @ispif_sink_port: ispif sinc port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function deletes a stream from sink port
 *
 *  Return: 0 - Success
 *         -1 - Stream not found
 **/
int ispif_util_del_stream_from_sink_port( ispif_t *ispif,
  ispif_port_t *ispif_sink_port, ispif_stream_t *stream);

/** ispif_util_add_stream_to_src_port:
 *    @ispif: ispif instance
 *    @ispif_src_port: ispif source port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function ads a stream to source port
 *
 *  Return: 0 - Success
 *         -1 - Port is connected to its max number of streams
 **/
int ispif_util_add_stream_to_src_port(ispif_t *ispif,
  ispif_port_t *ispif_src_port, ispif_stream_t *stream);

/** ispif_util_del_stream_from_src_port:
 *    @ispif: ispif instance
 *    @ispif_src_port: ispif source port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function deletes a stream from source port
 *
 *  Return: 0 - Success
 *         -1 - Stream not found
 **/
int ispif_util_del_stream_from_src_port(ispif_t *ispif,
  ispif_port_t *ispif_src_port, ispif_stream_t *stream);

/** ispif_util_get_match_src_port:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif pointer
 *    @stream: ispif pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds matching source port for a stream
 *
 *  Return: NULL - Port not found
 *          Otherwise - Pointer to matching port
 **/
ispif_port_t *ispif_util_get_match_src_port(ispif_t *ispif,
  ispif_port_t *ispif_sink_port, ispif_stream_t *stream);

/** ispif_util_find_sink_port:
 *    @ispif: ispif instance
 *    @sensor_cap: ispif pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a sink port matching to a sensor source port with given
 *  capabilities
 *
 *  Return: NULL - Port not fouund
 *          Otherwise - Pointer to ispif sink port
 **/
ispif_port_t *ispif_util_find_sink_port(ispif_t *ispif,
  sensor_src_port_cap_t *sensor_cap);

/** ispif_util_config_src_port:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function confugures a source port
 *
 *  Return: 0 - Success
 *         -1 - Downstream config event failed
 *        -10 - Not enough memory
 **/
int ispif_util_config_src_port(ispif_t *ispif,
  ispif_port_t *ispif_sink_port, ispif_stream_t *stream);

/** ispif_util_find_primary_cid:
 *    @sensor_cfg: Sensor configuration
 *    @sensor_cap: Sensor source port capabilities
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds primary CID for sensor source port by format
 *
 *  Return: Array index of the primary CID
 **/
uint32_t ispif_util_find_primary_cid(sensor_out_info_t *sensor_cfg,
  sensor_src_port_cap_t *sensor_cap);

/** ispif_util_dump_sensor_cfg:
 *    @sink_port: Sink port connected to sensor
 *
 *  This function runs in MCTL thread context.
 *
 *  This function dumps sensor configuration by port.
 *
 *  Return: None
 **/
void ispif_util_dump_sensor_cfg(ispif_sink_port_t *sink_port);

/** ispif_util_choose_isp_interface:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @session: ispif session
 *    @stream: ispif stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function determines ISP output interface for given stream
 *
 *  Return: None
 **/
void ispif_util_choose_isp_interface(ispif_t *ispif,
  ispif_port_t *ispif_sink_port, ispif_session_t *session,
  ispif_stream_t *stream);

/** ispif_util_reserve_isp_resource:
 *    @ispif: ispif instance
 *    @ispif_sink_port: ispif sink port
 *
 *  This function runs in MCTL thread context.
 *
 *  This function reserves an ISP resource
 *
 *  Return: 0 - Success
 *         -1 - Couldn't reserve resource
 **/
int ispif_util_reserve_isp_resource(ispif_t *ispif,
  ispif_port_t *ispif_sink_port);

/** ispif_util_release_isp_resource:
 *    @ispif: ispif pointer
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function releases ISP resources associated with a stream
 *
 *  Return: None
 **/
void ispif_util_release_isp_resource(ispif_t *ispif, ispif_stream_t *stream);

/** ispif_util_find_isp_intf_type:
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function determines ISP interface type associates with a stream
 *  by output mask
 *
 *  Return: msm_ispif_intftype enumeration of interface found
 *          INTF_MAX - invalid mask
 **/
enum msm_ispif_intftype ispif_util_find_isp_intf_type(ispif_stream_t *stream,
  uint32_t used_output_mask, uint32_t vfe_mask);
/** ispif_util_set_bundle:
 *    @ispif: ispif instance
 *    @ispif_sink_port: ispif sink port
 *    @session_id: session id
 *    @stream_id: stream id
 *    @bundle_param: HAL bundle params
 *
 *  This function runs in MCTL thread context.
 *
 *  This function sets ispif bundling mask according to bundle configuration
 *  by output mask
 *
 *  Return: 0 - Success
 *         -1 - CAnnot find session or stream
 **/
int ispif_util_set_bundle(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t session_id, uint32_t stream_id, cam_bundle_config_t *bundle_param);

/** ispif_util_get_stream_ids_by_mask:
 *    @session: session
 *    @num_streams: number of active stream
 *    @stream_ids: array with stream IDs
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds stream by mask
 *
 *  Return: 0 - Success
 **/
int ispif_util_get_stream_ids_by_mask(ispif_session_t *session,
  uint32_t stream_idx_mask, int *num_streams, uint32_t *stream_ids);

/** ispif_util_dump_sensor_src_cap:
 *    @sensor_cap: Sensor source port capabilities
 *
 *  This function runs in MCTL thread context.
 *
 *  This function dumps sensor source port capabilities
 *
 *  Return: None
 **/
void ispif_util_dump_sensor_src_cap(sensor_src_port_cap_t *sensor_cap);

/** ispif_util_has_pix_resource:
 *    @sink_port: ispif sink port
 *    @stream_info: stream info data
 *
 *  This function runs in MCTL thread context.
 *
 *  This function checks if PIX interface is needed and available
 *
 *  Return: TRUE  - There is PIX interface available or it is not needed for
 *                  this stream
 *          FALSE - There is no available PIX interface
 **/
boolean ispif_util_has_pix_resource(ispif_sink_port_t *sink_port,
  mct_stream_info_t *stream_info);

/** ispif_util_pip_switching_cap_op_pixclk:
 *    @ispif: ispif instance
 *    @ispif_sink_port: ispif sink port
 *    @sensor_cfg: sensor configuration
 *
 *  This function runs in MCTL thread context.
 *
 *  This function switches the pixel clock when in PIP
 *
 *  Return: 0 - Success
 *         -1 - Stream not found
 **/
int ispif_util_pip_switching_cap_op_pixclk(ispif_t *ispif,
  ispif_port_t *ispif_sink_port, sensor_out_info_t *sensor_cfg);

/** ispif_util_dual_vfe_to_pip_switching:
 *    @ispif: ispif instance
 *    @ispif_sink_port: ispif sink port
 *    @sensor_cfg: sensor configuration
 *
 *  This function runs in MCTL thread context.
 *
 *  This function switches from dual ISP mode to PIP
 *
 *  Return: None
 **/
void ispif_util_dual_vfe_to_pip_switching(ispif_t *ispif,
  sensor_src_port_cap_t *sensor_port_cap, ispif_stream_t *starving_stream);

#endif /* __ISPIF_UTIL_H__ */
