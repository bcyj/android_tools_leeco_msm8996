/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#ifndef __HJR_H__
#define __HJR_H__

#include "camera_dbg.h"
#if(FRAME_PROC_HJR_DEBUG)
  #include <utils/Log.h>
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-HJR"
  #define CDBG_HJR(fmt, args...) LOGE(fmt, ##args)
#else
  #define CDBG_HJR(fmt, args...) do{}while(0)
#endif

#define IS_UINT7(a) ((a)<=127)

#define HJR_ABS(x) (x>0 ? x : -x)
#define HJR_MIN(x,y) (x<y ? x : y)
#define HJR_MAX(x,y) (x>y ? x : y)

int hjr_handle_multi_frames_for_handjitter(void *Ctrl);
#endif /* __HJR_H__ */

