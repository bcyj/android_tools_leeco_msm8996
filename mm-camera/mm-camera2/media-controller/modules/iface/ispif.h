/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISPIF_CORE_H__
#define __ISPIF_CORE_H__

#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "isp_def.h"
#include "isp_event.h"

#define ERROR_CODE_BASE                  0x1000
#define ERROR_CODE_ISP_RESOURCE_STARVING (ERROR_CODE_BASE+1)

#define ISP_PIX_MAX_MASK  ((1 << ISP_INTF_PIX) | (1 << (16 + ISP_INTF_PIX)))
#define ISP_RDI0_MAX_MASK ((1 << ISP_INTF_RDI0) | (1 << (16 + ISP_INTF_RDI0)))
#define ISP_RDI1_MAX_MASK ((1 << ISP_INTF_RDI1) | (1 << (16 + ISP_INTF_RDI1)))
#define ISP_RDI2_MAX_MASK ((1 << ISP_INTF_RDI2) | (1 << (16 + ISP_INTF_RDI2)))


#define UNPACK_STREAM_ID(identity) (identity & 0x0000FFFF)
#define UNPACK_SESSION_ID(identity) ((identity & 0xFFFF0000) >> 16)

#define ISPIF_SINK_PORTS_NUM 8  /* ispif supports max 8 ports */
#define ISPIF_SRC_PORTS_NUM  8  /* ispif supports max 8 ports */

/** ispif_stream_state_t
 *
 *    @ISPIF_STREAM_NOTUSED: not used
 *    @ISPIF_STREAM_CREATED: created
 *    @ISPIF_STREAM_ASSOCIATED_WITH_SINK_PORT: linked with sink port
 *    @ISPIF_STREAM_ASSOCIATED_WITH_SRC_PORT: linked with src port
 *    @ISPIF_STREAM_CONFIGURED: stream configured
 *    @ISPIF_STREAM_STARTED: stream started
 *    @ISPIF_STREAM_MAX: number of states
 *
 * ispif stream states enumeration
 **/
typedef enum {
  ISPIF_STREAM_NOTUSED,
  ISPIF_STREAM_CREATED,
  ISPIF_STREAM_ASSOCIATED_WITH_SINK_PORT,
  ISPIF_STREAM_ASSOCIATED_WITH_SRC_PORT,
  ISPIF_STREAM_CONFIGURED,
  ISPIF_STREAM_STARTED,
  ISPIF_STREAM_MAX
} ispif_stream_state_t;

/** ispif_stream_t
 *
 *    @session: session to which stream belongs
 *    @session_id: session id
 *    @stream_id: stream id
 *    @stream_info: stream parameters from MCTL
 *    @sink_port: sink port
 *    @src_port: src port
 *    @msm_vfe_axi_stream_src axi_path: which interface is stream using for AXI
 *    @state: stream state
 *    @link_cnt: link count for unreserve
 *    @use_pix: TRUE - use pix interface
 *    @used_output_mask:  hi 16 bits - VFE1, lo 16 bits VFE0
 *    @stream_idx: index of stream in stream array
 *    @num_meta: number meta channels in stream
 *    @meta_info[MAX_META]: meta channels info array
 *    @meta_use_output_mask: meta channels output mask
 *    @split_info: describes the output dimensions in split output case
 *
 * ispif stream data structure
 **/
typedef struct {
  void *session;
  uint32_t session_id;
  uint32_t stream_id;
  mct_stream_info_t stream_info;
  mct_port_t *sink_port;
  mct_port_t *src_port;
  enum msm_vfe_axi_stream_src axi_path;
  ispif_stream_state_t state;
  int link_cnt;
  uint8_t use_pix;
  uint32_t used_output_mask;
  int stream_idx;
  uint32_t num_meta;
  sensor_meta_t meta_info[MAX_META];
  uint32_t meta_use_output_mask;
  ispif_out_info_t split_info;
} ispif_stream_t;

/** ispif_session_t
 *
 *    @ispif: ispif instance
 *    @streams[ISP_MAX_STREAMS]: streams in the session
 *    @session_id: session id
 *    @vfe_mask: which vfe associated
 *    @camif_cnt: PIX interfaces used from the session
 *    @rdi_cnt: RDI interfaces used from the session
 *    @session_idx: session index in ispif session array
 *    @num_stream: number of streams
 *    @hal_bundling_mask: HAL stream bundling mask
 *    @streamon_bundling_mask: stream on bundling mask
 *    @streamoff_bundling_mask: stream off bundling mask
 *    @dual_single_isp_switching: we are switched from dual to single isp
 *    @high_op_pix_clk: isp pixel clock
 *    @need_resume: when dual vfe session switch to single vfe, this session
 *                need resume
 *    @trigger_dual_isp_restore: if this is set to TRUE,
 *                             when this session is closed we need to find the
 *                             dual isp session and ask mct to restore it.
 *    @active_count: number of started streams
 *
 * ispif session data structure
 **/
typedef struct {
  void *ispif;
  ispif_stream_t streams[ISP_MAX_STREAMS];
  uint32_t session_id;
  uint32_t vfe_mask;
  uint8_t camif_cnt;
  uint8_t rdi_cnt;
  uint8_t num_meta;
  uint8_t session_idx;
  int num_stream;
  uint32_t hal_bundling_mask;
  uint32_t streamon_bundling_mask;
  uint32_t streamoff_bundling_mask;
  boolean dual_single_isp_switching;
  uint32_t high_op_pix_clk;
  boolean need_resume;
  boolean trigger_dual_isp_restore;
  int active_count;
  uint8_t fast_aec_mode;
  boolean streamoff_error;
} ispif_session_t;

/** isp_output_resources_t:
  *
  *  @cfg_output_mask: mask containing configured isp resources
  *                    hi 16 bits - VFE1, lo 16 bits VFE0
  *  @used_output_mask: mask containing used ispif resources
  *                     hi 16 bits - VFE1, lo 16 bits VFE0
  *  @rdi_only_session_mask: mask containing RDI resources
  *                          shift by session idx, vfe0 - lo 16 bits
  *  @pix_session_mask: mask containing PIX resources
  *                     shift by session idx, vfe0 - lo 16 bits
  *
  * isp resources data structure
  **/
typedef struct {
  uint32_t cfg_output_mask;
  uint32_t used_output_mask;
  uint32_t rdi_only_session_mask;
  uint32_t pix_session_mask;
} isp_output_resources_t;

/** ispif_src_port_t:
  *
  *  @ispif_port: MCTL base module of ispif port.
  *  @streams: streams linked to port
  *  @caps: ispif source port capabilities
  *  @num_streams: number of streams
  *
  * ispif source port data structure
  **/
typedef struct {
  mct_port_t *ispif_port;
  ispif_stream_t *streams[ISP_MAX_STREAMS];
  ispif_src_port_caps_t caps;
  int num_streams;
} ispif_src_port_t;

/** ispif_sink_port_t:
  *
  *  @ispif_port: MCTL base module of ispif port.
  *  @streams: streams linked to port
  *  @sensor_cfg: sensor configuration
  *  @sensor_cap: matching sensor source port capabilities
  *               this cap matches to Sensor's src cap
  *  @num_streams: number of streams
  *
  * ispif sink port data structure
  **/
typedef struct {
  mct_port_t *ispif_port;
  ispif_stream_t *streams[ISP_MAX_STREAMS];
  sensor_out_info_t sensor_cfg;
  sensor_src_port_cap_t sensor_cap;
  int num_streams;
} ispif_sink_port_t;

/** ispif_port_state_t:
  *
  *  @ISPIF_PORT_STATE_CREATED: port is created
  *  @ISPIF_PORT_STATE_RESERVED: port is reserved
  *  @ISPIF_PORT_STATE_START_PENDING: port is ready to start
  *  @ISPIF_PORT_STATE_ACTIVE: port is active
  *  @ISPIF_PORT_STATE_MAX:
  *
  * ispif port states enumeration
  **/
typedef enum {
  ISPIF_PORT_STATE_CREATED,
  ISPIF_PORT_STATE_RESERVED,
  ISPIF_PORT_STATE_START_PENDING,
  ISPIF_PORT_STATE_ACTIVE,
  ISPIF_PORT_STATE_MAX
} ispif_port_state_t;

/** ispif_port_t:
  *
  *  @port: MCTL base module of ispif port.
  *  @ispif: ispif instance to which port belongs
  *  @state: port state
  *  @session_id: session which opened the port
  *               port cannot be shared between sessions.
  *  @num_active_streams: number of currently active streams
  *  @src_port: source port data
  *  @sink_port: sink port data
  *
  * ispif port data structure
  **/
typedef struct {
  mct_port_t *port;
  void *ispif;
  ispif_port_state_t state;
  uint32_t session_id;
  int num_active_streams;
  union {
    ispif_src_port_t src_port;
    ispif_sink_port_t sink_port;
  } u;
} ispif_port_t;

/** ispif_t:
  *
  *  @module: MCTL base module of ispif.
  *  @mutex: mutex lock of ispif module
  *  @dev_name: FS name of ispif subdevice
  *  @fd: file descriptor of ispif subdevice
  *  @num_active_streams: number of currently active streams
  *  @meta_info: sensor meta stream info
  *  @meta_identity: sensor meta stream identity
  *  @cfg_cmd: ispif hw configuration data
  *  @meta_pending: flag for pending meta channel configuration
  *
  * ispif module instance data structure
  **/
typedef struct {
  mct_module_t *module;
  pthread_mutex_t mutex;
  char dev_name[32];
  ispif_session_t sessions[ISP_MAX_SESSIONS];
  int fd;
  int num_active_streams;
  sensor_meta_data_t meta_info;
  uint32_t meta_identity;
  struct ispif_cfg_data cfg_cmd;
  boolean meta_pending;
} ispif_t;

/** ispif_caps_t:
  *
  *  @max_num_src_ports: max number of source ports
  *  @max_num_sink_ports: max number of sink ports
  *
  * ispif module capabilities structure
  **/
typedef struct {
  int max_num_src_ports;
  int max_num_sink_ports;
} ispif_caps_t;

/** ispif_init:
 *    @ispif: ispif pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This function initializes ispif module.
 *
 *  Return:  0 - Success
 *          -1 - error during discovering subdevices
 **/
int ispif_init(ispif_t *ispif);

/** ispif_destroy:
 *    @ispif: ispif pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This stub function will contain code to destroy ispif module.
 *
 *  Return:  None
 **/
void ispif_destroy (ispif_t *ispif);

/** ispif_start_session:
 *    @ispif: ispif pointer
 *    @session_id session id to be started
 *
 *  This function runs in MCTL thread context.
 *
 *  This function starts an ispif session
 *
 *  Return:  0 - Success
 *          -1 - Session not found
 **/
int ispif_start_session(ispif_t *ispif, uint32_t session_id);

/** ispif_stop_session:
 *
 *    @ispif: ispif pointer
 *    @session_id: id of session to be stopped
 *
 *  This function runs in MCTL thread context.
 *
 * This functions stops a session
 *
 *  Return:  0 - Success
 *          -1 - Session not found
 **/
int ispif_stop_session(ispif_t *ispif, uint32_t session_id);

/** ispif_reserve_sink_port:
 *
 *    @ispif: ispif pointer
 *    @ispif_port: port to be reserved
 *    @sensor_port_cap: sersor src port capabilities
 *    @stream_info: stream info
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function tries to reserve a sink port in ispif module.
 *
 *  Return:  0 - Success
 *     -EAGAIN - Has another port matching the sensor cap.
 *      other negative value - error while reserving port
 **/
int ispif_reserve_sink_port(ispif_t *ispif, ispif_port_t *ispif_port,
  sensor_src_port_cap_t *sensor_port_cap, mct_stream_info_t *stream_info,
  unsigned int session_id, unsigned int stream_id);

/** ispif_reserve_src_port:
 *    @ispif: ispif pointer
 *    @src_port: src port to be reserved
 *    @stream_info: stream info
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function reserves an ispif sink port
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream or port is in invalid state
 *     -EAGAIN - The given src port cannot be used to reserve
 **/
int ispif_reserve_src_port(ispif_t *ispif, ispif_port_t *src_port,
  mct_stream_info_t *stream_info, unsigned int session_id,
  unsigned int stream_id);

/** ispif_unreserve_sink_port:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: sink port to be unreserved
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function unreserves an ispif port.
 *
 *  Return:  0 - Success
 *          -1 - Stream not found
 **/
int ispif_unreserve_sink_port(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t session_id, uint32_t stream_id);

/** ispif_unreserve_src_port:
 *    @ispif: ispif pointer
 *    @ispif_src_port: source port to be unreserved
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function unreserves an ispif source port
 *
 *  Return:  0 - Success
 *          -1 - Stream not found
 **/
int ispif_unreserve_src_port(ispif_t *ispif, ispif_port_t *ispif_src_port,
  unsigned int session_id, unsigned int stream_id);

/** ispif_streamon:
 *
 *    @ispif: pointer to ispif instance
 *    @ispif_sink_port: ispif sink port
 *    @session_id: session id
 *    @stream_id: stream id
 *    @event: MCTL control event
 *
 *  This function runs in MCTL thread context.
 *
 * This function start ispif streaming
 *
 *  Return:  0 - Success
 *          -1 - cannot find session/stream or
 *               start streaming is not successful
 **/
int ispif_streamon(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event);

/** ispif_streamoff:
 *
 *    @ispif: pointer to ispif instance
 *    @ispif_sink_port: ispif sink port
 *    @session_id: session id
 *    @stream_id: stream id
 *    @event: MCTL control event
 *
 *  This function runs in MCTL thread context.
 *
 * This function stops ispif streaming
 *
 *  Return:  0 - Success
 *          -1 - Invalid stream/session ID or
 *               unsuccessful stopping of stream
 **/
int ispif_streamoff(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event);

/** ispif_unlink_sink_port:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @peer_port: peer to which sink port is connected
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function unlinks ispif sink port
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream
 **/
int ispif_unlink_sink_port(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id);

/** ispif_unlink_src_port:
 *    @ispif: ispif pointer
 *    @ispif_src_port: ispif source port
 *    @peer_port: port to which ispif source port is connected
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function unlinks ispif source port
 *
 *  Return:  0 - Success
 *          -1 - cannot find stream
 **/
int ispif_unlink_src_port(ispif_t *ispif, ispif_port_t *ispif_src_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id);

/** ispif_link_sink_port:
 *
 *    @ispif: pointer to ispif instance
 *    @ispif_port: pointer to ispif sink port
 *    @peer_port: pointer to which sink port will link
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This pointer links ispif sink port with corresponding source port of other
 *  module.
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream or
 *               stream not in given sink port or
 *               peer is not matching
 **/
int ispif_link_sink_port(ispif_t *ispif, ispif_port_t *ispif_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id);

/** ispif_link_sink_port:
 *    @ispif: pointer to ispif instance
 *    @ispif_port: pointer to ispif sink port
 *    @peer_port: pointer to which sink port will link
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This pointer links ispif sink port with corresponding source port of other
 *  module.
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream or
 *               stream not in given sink port or
 *               peer is not matching
 **/
int ispif_link_sink_port(ispif_t *ispif, ispif_port_t *ispif_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id);

/** ispif_link_src_port:
 *    @ispif: pointer to ispif instance
 *    @ispif_port: pointer to ispif source port
 *    @peer_port: pointer to which sink port will link
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This pointer links ispif source port with corresponding sink port of other
 *  module.
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream or stream not reserved at source port
 **/
int ispif_link_src_port(ispif_t *ispif, ispif_port_t *ispif_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id);

/** ispif_set_split_info:
 *    @ispif: ispif pointer
 *    @session_id: session id
 *    @stream_id: stream id
 *    @split_info: structure with split info (dual vfe mode)
 *
 *  This function runs in MCTL thread context.
 *
 *  This function sets image split info to a stream
 *
 *  Return:  0 - Success
 *          -1 - Invalid session or stream ID
 **/
int ispif_set_split_info(ispif_t *ispif, uint32_t session_id,
  uint32_t stream_id, ispif_out_info_t *split_info);

/** ispif_meta_channel_config:
 *    @ispif: ispif pointer
 *    @session_id: session id
 *    @stream_id: stream id
 *    @ispif_sink_port: ispif sink port
 *
 *  This function runs in MCTL thread context.
 *
 *  This function configures the meta channel
 *
 *  Return:  0 - Success
 *          -1 - cannot find session or RDI reserve error
 **/
int ispif_meta_channel_config(ispif_t *ispif, uint32_t session_id,
  uint32_t stream_id, ispif_port_t *ispif_sink_port);

/** ispif_sink_port_config:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @stream_idi: stream id
 *    @session_id: session id
 *    @sensor_cfg: semsor configuration
 *
 *  This function configures ispif sink port
 *
 *  Return: 0 for success and negative error on failure
 **/
int ispif_sink_port_config(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t stream_id, uint32_t session_id, sensor_out_info_t *sensor_cfg);

/** ispif_set_hal_param:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @stream_idi: stream id
 *    @session_id: session id
 *    @event: MCTL control event
 *
 *  This function runs in MCTL thread context.
 *
 *  This stub function sets hal parameter to ispif module
 *
 *  Return: 0
 **/
int ispif_set_hal_param(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event);

/** ispif_set_hal_stream_param:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @stream_idi: stream id
 *    @session_id: session id
 *    @event: MCTL control event
 *
 *  This function runs in MCTL thread context.
 *
 *  This function sets hal stream parameter to ispif module
 *
 *  Return:  0 - No error
 *          -1 - Cannot find session or stream
 **/
int ispif_set_hal_stream_param(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event);

/** ispif_dual_isp_pip_switch:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @sensor_cfg: sensor configuration
 *
 *  This function runs in MCTL thread context.
 *
 *  This function switches from dual isp mode to PIP mode
 *
 *  Return:  0 - No error
 *          -1 - Cannot find the stream to switch
 **/
int ispif_dual_isp_pip_switch(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  sensor_out_info_t *sensor_cfg);
int ispif_set_fast_aec_mode(ispif_t *ispif, uint32_t session_id, void *data);
/** ispif_resume_pending_session:
 *    @ispif: pointer to ispif instance
 *
 *  This function runs in MCTL thread context.
 *
 *  This function resumes a stopped pending session
 *
 *  Returns: None
 **/
void ispif_resume_pending_session(ispif_t *ispif);

/** ispif_need_restore_dual_isp_session:
 *    @ispif: pointer to ispif instance
 *    @session_id: session id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function checks if a session needs to be restored in dual isp mode
 *
 *  Returns: True  - if session needs to be restored in dual isp mode
 *           False - otherwise
 **/
boolean ispif_need_restore_dual_isp_session(ispif_t *ispif, uint32_t session_id);

/** ispif_restore_dual_isp_session:
 *    @ispif: pointer to ispif instance
 *
 *  This function runs in MCTL thread context.
 *
 *  This function restores dual isp session
 *
 *  Returns: None
 **/
void ispif_restore_dual_isp_session(ispif_t *ispif);

/** port_ispif_add_ports:
 *    @ispif: ispif instance
 *    @caps: ispif module capabilities
 *
 *  This function runs in MCTL thread context.
 *
 *  This function create all ports to ispif module
 *
 *  Return: 0 - Success
 *         -1 - Error while adding ports
 **/
int port_ispif_create_ports(ispif_t *ispif, ispif_caps_t *caps);
void port_ispif_destroy_ports(ispif_t *ispif);
#endif /* __ISPIF_CORE_H__ */

