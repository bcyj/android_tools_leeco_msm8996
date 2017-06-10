/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#ifndef __AFD_H__
#define __AFD_H__

#ifdef _ANDROID_
#include <utils/Log.h>
#endif
#include "stats_proc.h"
#include "camera_dbg.h"

#define CAFD_DELAY 120
#if(AFD_DEBUG_HIGH)
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-AFD"
#endif
#if(AFD_DEBUG_LOW)
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-AFD"
  #define CDBG_AFD(fmt, args...) LOGE(fmt, ##args)
#else
  #define CDBG_AFD(fmt, args...) do{}while(0)
#endif
#undef CDBG_HIGH
#define CDBG_HIGH(fmt, args...) LOGE(fmt, ##args)
#define AFD_NUM_FRAMES 6 /* continous frames for computing row sum data. */
/* (AFD_FRAME_SKIP -1) number of frames are skipped when calculating
 * diff between 2 frames. This is helpful if the enough band rolling does
 * not present between consecutive frames. */
#define AFD_FRAME_SKIP 1
#define AFD_NUM_ROWS 480
#define AFD_Q8     0x00000100
typedef enum {
  AFD_INIT = 0,
  AFD_ACTIVE,
  AFD_INACTIVE,
  AFD_INVALID_STATE,
  AFD_STATE_OFF,
  AFD_STATE_ON,
  AFD_STATE_BUSY,
  AFD_STATE_DONE,
  AFD_STATE_INIT,
} afd_state_type;

typedef struct {
  int trigger;
  int frame_skip_cnt;
  int status;
  int enable;
} conti_afd_t;

typedef struct {
  int frame_index;
  /* This is based on 2 frame diff so the dimension is AFD_NUM_FRAMES - 1 */
  int edge[AFD_NUM_FRAMES - AFD_FRAME_SKIP][8];
  /* this is based on 2 frame diff so the dimension is AFD_NUM_FRAMES - 1 */
  int peak_width[AFD_NUM_FRAMES - AFD_FRAME_SKIP][7];
  int array_size;
  int num_peaks;
  int std_threshold;
  int row_sum[AFD_NUM_FRAMES][AFD_NUM_ROWS];
  /* the diff signal between 2 frames has to be above this value
   * to be considered flicker signal exists */
  int diff_threshold;
  /* if FPS is <30 and >=15, line count has to be smaller than this
   * value to trigger FD. */
  int line_count_threshold;
  /* Duration between any successive FD sessions in terms of
   * number of frame. It should be programmed to a factor of sensor
   * framerate */
  int frame_ct_threshold;
  /* for multiple peak algo, banding distance std(width) in Q8, initialize as -1 */
  int std_width;
  /* row subsampling ratio when calculating row sum */
  int row_subsmpl;
  int afd_done;  /* only run once */
  int frame_skip;
  int flicker;
  int flicker_freq;
  int actual_peaks;
  int conti_afd_delay;
  afd_state_type state;
  camera_antibanding_type afd_mode;
  conti_afd_t    conti;
} afd_t;

void afd_reset(stats_proc_t *sproc, afd_t *afd);
int afd_algo_run(stats_proc_t *sproc, afd_t *afd);
#endif /* __AFD_H__ */
