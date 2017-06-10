/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef CPP_MODULE_EVENTS_H
#define CPP_MODULE_EVENTS_H

#include "modules.h"
#include "chromatix.h"
#include "chromatix_common.h"
#include "mct_queue.h"
#include "mct_list.h"
#include "media_controller.h"
#include "mct_port.h"
#include "mct_object.h"
#include "cam_types.h"
#include "mct_module.h"
#include "mct_pipeline.h"
#include "mct_stream.h"
#include "camera_dbg.h"
#include "cpp_port.h"
#include "cpp_module.h"
#include "cpp_log.h"
#include "eztune_diagnostics.h"

/* cpp_decide_hfr_skip:
 *
 * Decides if a frame needs to be skipped. if @frame_id is 0, its not skipped.
 * @count number of frames are skipped after the 0th frame.
 * The pattern repeats.
 *
 **/
#define cpp_decide_hfr_skip(frame_id, count) \
  ((((frame_id) % ((count)+1)) == 0) ? FALSE : TRUE)

int32_t cpp_module_handle_buf_divert_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_isp_out_dim_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_aec_update_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_chromatix_ptr_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_stream_crop_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_dis_update_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_set_parm_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_set_stream_parm_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_streamon_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_streamoff_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_stream_cfg_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_div_info_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_load_chromatix_event(mct_module_t* module,
  mct_event_t* event);

int32_t cpp_module_handle_set_output_buff_event(mct_module_t* module,
  mct_event_t* event);

#endif
