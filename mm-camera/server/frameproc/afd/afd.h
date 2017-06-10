/**********************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#ifndef __AFD_H__
#define __AFD_H__
#if 0
#if(FRAME_PROC_AFD_DEBUG)
  #include <utils/Log.h>
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-AFD"
  #define CDBG_AFD(fmt, args...) LOGE(fmt, ##args)
#else
  #define CDBG_AFD(fmt, args...) do{}while(0)
#endif
#endif
#define CDBG_AFD(fmt, args...) LOGE(fmt, ##args)
#define CDBG_ERROR(fmt, args...) LOGE(fmt, ##args)
/*============================================================================
                        CONSTANTS
============================================================================*/
/* Programmable parameter: Number of continous frames used for computing
 * row sum data.
 */
#define AFD_NUM_FRAMES 6

/* Programmable parameter:
 * (AFD_FRAME_SKIP -1) number of frames are skipped when calculating
 * diff between 2 frames. This is helpful if the enough band rolling does
 * not present between consecutive frames.
 */
#define AFD_FRAME_SKIP 1

/* Max. Rows, currently we support upto VGA (620 x 480) size */
#define AFD_NUM_ROWS 480

/*============================================================================
                        EXTERNAL ABSTRACT DATA TYPES
============================================================================*/

typedef struct
{
  int32_t frame_index;

  /* Number of consecutive frames for computing row sum data */
  uint8_t row_sum_frame_cnt;

  /* This is based on 2 frame diff so the dimension is AFD_NUM_FRAMES - 1 */
  int32_t edge[AFD_NUM_FRAMES - AFD_FRAME_SKIP][8];

  /* this is based on 2 frame diff so the dimension is AFD_NUM_FRAMES - 1 */
  int32_t peak_width[AFD_NUM_FRAMES - AFD_FRAME_SKIP][7];
  int32_t array_size;
  int32_t num_peaks;

  /* Q8 for multiple peak mode */
  int32_t std_threshold;

  /* 0 to 100 for single peak mode */
  int32_t percent_thresh;

  /* half of the total search range in pixels without zoom. */
  int32_t single_peak_mode_search_range;
  int32_t row_sum[AFD_NUM_FRAMES][AFD_NUM_ROWS];

  /* the diff signal between 2 frames has to be above this value
   * to be considered flicker signal exists
   */
  int32_t diff_threshold;

  /* if FPS is <30 and >=15, line count has to be smaller than this
   * value to trigger FD.
   */
  int32_t line_count_threshold;

  /* Duration between any successive FD sessions in terms of
   * number of frame. It should be programmed to a factor of sensor
   * framerate
   */
  int32_t frame_ct_threshold;

  /* 0 means nothing has been done,
   * 1 means using normal table,
   * 2 means using 60Hz antibanding table,
   * 3 means 50 Hz table,
   * 4 means FD is done
   */
  int32_t status;

  /* -1 means not sure,
   * 0 means no flicker; 60, or 50.
   */
  int32_t flicker_freq;


  /* for multiple peak algo, banding distance std(width) in Q8, initialize as -1 */
  int32_t std_width;

  /* for single band algo, positive shift banding percent.
   * 80 means 80% , initialize as -1 */
  int32_t ratio_p;

  /* for single band algo, negative shift banding percent.
   * 80 means 80% , initialize as -1
  */
  int32_t ratio_n;

  /* indicate what algo to use: 0 means single peak algo,
   * 1 means multiple peak algo, intialize as -1.
  */
  int32_t multiple_peak_algo;
  int32_t afd_needed;

  int32_t actual_peaks;

  /* row subsampling ratio when calculating row sum */
  int8_t afd_row_subsample;
  /* col subsampling ratio when calculating row sum */
  int8_t afd_col_subsample;

  /* Max. effecive row sum window for minimum acceptable computation */
  uint32_t afd_max_dx;
  uint32_t afd_max_dy;

  int32_t afd_done;  /* only run once */
  uint32_t row_sum_line_cnt;
  uint32_t row_sum_pixel_cnt;



} afd_t;

void afd_reset(void *Ctrl, afd_t *afdCtrl);
int afd_execute(void *Ctrl, afd_t *afdCtrl);
#endif /* __AFD_H__ */
