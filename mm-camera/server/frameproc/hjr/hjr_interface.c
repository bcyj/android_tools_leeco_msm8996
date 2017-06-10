/**********************************************************************
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
**********************************************************************/
#include <string.h>
#include "frameproc.h"
#include "hjr.h"
#ifdef _ANDROID_
#include <utils/Log.h>
#endif
#include "camera_dbg.h"

/*===========================================================================
 * FUNCTION    - hjr_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int hjr_set_params(frame_proc_t *frameCtrl, frame_proc_set_hjr_data_t *data)
{
  frameCtrl->output.hjr_d.hjr_enable = data->hjr_enable;
  return 0;
} /* hjr_set_params */
/*===========================================================================
 * FUNCTION    - hjr_execute -
 *
 * DESCRIPTION:
 *==========================================================================*/
int hjr_execute(frame_proc_t *frameCtrl)
{
  CDBG_HIGH("%s: E ",__func__);
  if(frameCtrl->input.statsproc_info.aec_d.hjr_snap_frame_count > 1) {
    if (!hjr_handle_multi_frames_for_handjitter(frameCtrl)) {
      CDBG_ERROR("%s: Multi Frame HJR FAILED!!!\n", __func__);
      return -1;
    } else {
      CDBG_HIGH("%s: Multi Frame HJR DONE\n", __func__);
    }
  } else {
    CDBG_ERROR("Execute Hardware HJR");
  }
  return 0;
} /* hjr_execute */


