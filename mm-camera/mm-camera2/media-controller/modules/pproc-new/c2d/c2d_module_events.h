/*============================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef C2D_MODULE_EVENTS_H
#define C2D_MODULE_EVENTS_H

#include "c2d_port.h"
#include "c2d_module.h"
#include "c2d_log.h"

/* c2d_decide_hfr_skip:
 *
 * Decides if a frame needs to be skipped. if @frame_id is 0, its not skipped.
 * @count number of frames are skipped after the 0th frame.
 * The pattern repeats.
 *
 **/
#define c2d_decide_hfr_skip(frame_id, count) \
  ((((frame_id) % ((count)+1)) == 0) ? FALSE : TRUE)

int32_t c2d_module_handle_buf_divert_event(mct_module_t* module,
  mct_event_t* event);

int32_t c2d_module_handle_isp_out_dim_event(mct_module_t* module,
  mct_event_t* event);

int32_t c2d_module_handle_stream_crop_event(mct_module_t* module,
  mct_event_t* event);

int32_t c2d_module_handle_dis_update_event(mct_module_t* module,
  mct_event_t* event);

int32_t c2d_module_handle_set_parm_event(mct_module_t* module,
  mct_event_t* event);

int32_t c2d_module_handle_streamon_event(mct_module_t* module,
  mct_event_t* event);

int32_t c2d_module_handle_streamoff_event(mct_module_t* module,
  mct_event_t* event);

int32_t c2d_module_handle_stream_cfg_event(mct_module_t* module,
  mct_event_t* event);

int32_t c2d_module_handle_div_info_event(mct_module_t* module,
  mct_event_t* event);

int32_t c2d_module_handle_set_stream_parm_event(mct_module_t* module,
  mct_event_t* event);


#endif
