/* is_port.h
 *
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __IS_PORT_H__
#define __IS_PORT_H__
#include "mct_port.h"
#include "is_thread.h"


typedef enum {
  IS_PORT_STATE_CREATED = 1,
  IS_PORT_STATE_RESERVED,
  IS_PORT_STATE_LINKED,
  IS_PORT_STATE_UNLINKED,
  IS_PORT_STATE_UNRESERVED
} is_port_state_t;


/** _is_port_private_t:
 *    @video_reserved_id: session id + video stream id
 *    @state: IS port state
 *    @is_mode: IS method (DIS, gyro-assisted DIS, EIS 1.0)
 *    @RSCS_stats_ready: Indicates whether row and column sum stats has arrived
 *                       for the current frame (composite stats assumed)
 *    @rs_cs_data: RS/CS stats
 *    @dis_context: DIS context
 *    @eis_context: EIS 1.0 context
 *    @is_output: DIS/EIS output/results
 *    @vfe_width: VFE width (image width + margin)
 *    @vfe_height: VFE height (image height + margin)
 *    @width: image width
 *    @height: image height
 *    @num_row_sum: number of row sums
 *    @num_col_sum: number of column sums
 *    @sensor_mount_angle: sensor mount angle (0, 90, 180, 270)
 *    @camera_position: camera position (front or back)
 *    @is_enabled: Indicates whether IS is enabled
 *    @is_inited: Indicates whether IS has been initialized
 *    @video_stream_on: Indicates whether video stream (stream with type
 *                      CAM_STREAM_TYPE_VIDEO) is on.
 *    @sns_lib_offset_set: Indicates whether time offset of gyro sensor library
 *                         has been set.
 *    @stream_type: Indicates camera or camcorder mode
 *    @thread_data: IS thread data
 *
 * This structure represents the IS port's internal variables.
 **/
typedef struct _is_port_private {
  unsigned int video_reserved_id;
  unsigned int reserved_id;
  is_port_state_t state;
  int RSCS_stats_ready;
  is_output_type is_output;
  unsigned int video_stream_on;
  cam_stream_type_t stream_type;
  is_info_t is_info;
  is_process_output_t is_process_output;
  is_set_parameters_func set_parameters;
  is_process_func process;
  is_callback_func callback;
  is_thread_data_t *thread_data;
} is_port_private_t;


void is_port_deinit(mct_port_t *port);
boolean is_port_find_identity(mct_port_t *port, unsigned int identity);
boolean is_port_init(mct_port_t *port, unsigned int session_id);
#endif /* __IS_PORT_H__ */
