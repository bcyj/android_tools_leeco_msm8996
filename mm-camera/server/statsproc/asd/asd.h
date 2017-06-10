/**********************************************************************
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
**********************************************************************/
#ifndef __ASD_H__
#define __ASD_H__

#ifdef _ANDROID_
#include <utils/Log.h>
#endif
#include "camera_dbg.h"

#undef CDBG_HIGH
#undef CDBG_LOW

#if(ASD_DEBUG_HIGH)
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-ASD"
  #define CDBG_HIGH(fmt, args...) LOGE(fmt, ##args)
#else
  #define CDBG_HIGH(fmt, args...) do{}while(0)
#endif

#if(ASD_DEBUG_LOW)
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-ASD"
  #define CDBG_LOW(fmt, args...) LOGE(fmt, ##args)
#else
  #define CDBG_LOW(fmt, args...) do{}while(0)
#endif

typedef struct {
  uint32_t   low;
  uint32_t   high;
  float low_pct;
  float high_pct;
} asd_backlight_threshold_t;

typedef struct {
  int dark;
  int mid;
  int bright;
} asd_mode_t;

typedef struct {
  int asd_enable;
  int mixed_light;

  int no_backlight_cnt;
  int portrait_face_delay_cnt;

  asd_mode_t  gw_mode; /*grey world */
  asd_mode_t  ww_mode; /* white world */
  asd_backlight_threshold_t bl_thld;

  int histo_backlight_detected;
  uint32_t histo_backlight_scene_severity;
} asd_t;

void asd_init_data(stats_proc_t *sproc, asd_t *asd);
void asd_histogram_backlight_detect(stats_proc_t *sproc, asd_t *asd);
void asd_backlight_and_snowscene_detect(stats_proc_t *sproc, asd_t *asd);
void asd_landscape_detect(stats_proc_t *sproc, asd_t *asd);
void asd_portrait_detect(stats_proc_t *sproc, asd_t *asd);
#endif /*__ASD_H__*/
