/**********************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "afd.h"
#include "frameproc.h"


/*===========================================================================
 * FUNCTION    - afd_reset -
 *
 * DESCRIPTION
 *===========================================================================*/
void afd_reset(void *Ctrl, afd_t *afdCtrl)
{
  frame_proc_t *frameCtrl = (frame_proc_t *) Ctrl;
  CDBG_AFD("%s", __func__);
  afdCtrl->afd_done = 0;
  frameCtrl->output.afd_d.afd_state = AFD_STATE_OFF;
  frameCtrl->output.afd_d.afd_status = AFD_OFF;
  frameCtrl->output.afd_d.afd_antibanding_type =
    CAMERA_ANTIBANDING_OFF;
  afdCtrl->afd_needed = 0;
  afdCtrl->frame_index = 0;
  afdCtrl->flicker_freq = 0;
}

/*===========================================================================

FUNCTION: afd_multiple_peaks

DESCRIPTION:
  For multiple banding case, identifies banding locations,
  distance between bandings

INPUT: frame_rate - Q8 number
       zoom_factor - Q12 number

DEPENDENCIES:

SIDE EFFECTS:

RETURN VALUE:
  None

============================================================================*/
static void afd_multiple_peaks(afd_t *afdCtrl, int32_t frame_rate, int32_t zoom_factor)
{
  int32_t i, j, k;

  int32_t counter = 0, loop;
  int32_t ratio;
  int32_t edge_pos;
  int32_t max_peak = 0;
  int32_t max_peaks = 0;
  int32_t max_num_peaks;  /* in Q8 */
  int32_t number_smooth;

  /* at VGA resolution, we need 9 times smoothing,
   * smaller resolution needs smaller smoothing iterations
   */
  int32_t number_taps = 9;
  int32_t peak_dist;
  int32_t temp;
  int32_t tmp, new_edge_pos;
  int32_t size = afdCtrl->array_size;
  int32_t *sum1, *sum2;
  int32_t peak_counts[AFD_NUM_FRAMES - AFD_FRAME_SKIP];

  CDBG_AFD("Start afd_multiple_peaks, \
    frame_rate = %d, zoom_factor = %d\n", frame_rate, zoom_factor);

  if (!frame_rate) {
    CDBG_AFD("frame_rate = 0\n");
    return;
  }

  /* Determines the computation time */
  number_smooth = (10 - 480 / afdCtrl->array_size) / 2;

  /* this is for without zoom, Q8 */
  max_num_peaks =  ((uint32_t)(120 * Q8 * Q8) / frame_rate);

  if (max_num_peaks < 256) {
    return;
  }

  /* 4 is for 60 Hz, 3.3 is for 50Hz, so can cover worst case,
   * 2 is to ensure not to miss a peak
   */
  peak_dist = (int32_t) (zoom_factor * size * Q8 / (max_num_peaks * Q12));

  max_num_peaks = max_num_peaks / Q8;  /* change to Q0 */

  if ((sum1 = (int32_t *) malloc (sizeof (int) * size)) == NULL) {
    CDBG_AFD("sum1 == NULL\n");
    return;
  }
  memset(sum1, 0, sizeof (int) * size);
  CDBG_AFD("after malloc sum1 = %p\n", sum1);

  if ((sum2 = (int32_t *) malloc (sizeof (int) * size)) == NULL) {
    free (sum1);
    CDBG_AFD("sum2 == NULL\n");
    return;
  }
  memset(sum2, 0, sizeof (int) * size);
  for (loop = 0; loop < AFD_NUM_FRAMES - AFD_FRAME_SKIP; loop++) {
    peak_counts[loop] = 0;
    for (i=0; i < size; i ++) {
      sum2[i] = afdCtrl->row_sum[loop][i] -
        afdCtrl->row_sum[loop + AFD_FRAME_SKIP][i];
      if (sum2[i] < 0) {
        sum2[i] = 0;
      }
    }

    /* 9 tap filter */
    /* take care of borders first */
    temp = number_taps/2;

    for (i = 0; i < temp; i++) {
      sum1[i] = sum2[i];
    }

    for (i = size - temp; i < size; i ++) {
      sum1[i] = sum2[i];
    }

    for (k = 0 ; k < number_smooth; k++) {
      for (i = temp; i < size-temp; i ++) {
        /* sum1 will be the smoothed sum2 */
        sum1[i] = 0;
        for (j = i - temp; j <= i + temp; j ++) {
          sum1[i] += sum2[j];
        }
        sum1[i] = (sum1[i] + temp)/number_taps;
      }

      for (i = temp; i < size-temp; i ++) {
        sum2[i] = sum1[i];
      }
    }

    edge_pos = -1;
    counter = -1;
    for (i = 0; i < size; i ++) {
      if (sum1[i] >= max_peak) {
        max_peak = sum1[i];
      }
    }

    /* diff signal has to be above this value to be
     * considered flicker exists
     */
    /* threshold is programmable based on zoom factor */
    if (max_peak >= ((afdCtrl->diff_threshold << 12)/zoom_factor)) {
      /* max_peak *= 0.1 */
      max_peak *= 25;
      max_peak >>= 8;

      /* sum1 is the smoothed diff,
       * sum2 is derivative of smoothed diff
       */
      for (i = 1; i < size; i++) {
        sum2[i] = sum1[i-1]-sum1[i];
      }

      for (i = 0; i < size-1; i++) {
        if (counter < max_num_peaks) {
          if (sum2[i] <= 0 && sum2[i+1] >= 0 && sum1[i] > max_peak) {
            if (sum2[i+1]!=0 && sum2[i]!=0) {
              /* use shift instead of Q8 to reduce latency */
              ratio = (abs(sum2[i]) << 8) / abs(sum2[i+1]);
              tmp = (ratio)/(Q8+ratio);
              new_edge_pos = tmp + i;
            } else if (sum2[i+1] == 0) {
              new_edge_pos = i + 1;
            } else {
              new_edge_pos = i;
            }

            if (edge_pos == -1) {
              counter=counter+1;
              /* decimal precision */
              edge_pos = new_edge_pos;
              afdCtrl->edge[loop][counter]=edge_pos;
            } else {
              tmp = new_edge_pos - edge_pos + 1;
              if (tmp > peak_dist) {
                counter = counter + 1;
                afdCtrl->peak_width[loop][counter-1] = tmp;
              }
              edge_pos = new_edge_pos;
              afdCtrl->edge[loop][counter]=edge_pos;
            }
          }/* sum2(i) <= 0 && sum2(i+1) >= 0 && sum1(i) > max_peak) */
        } else
          i = size - 1;
      } /* i */
    }
    peak_counts[loop] = counter + 1;
  } /* loop */

  for (i = 0; i < AFD_NUM_FRAMES - AFD_FRAME_SKIP; i ++) {
    if (peak_counts[i] > max_peaks) {
      max_peaks = peak_counts[i];
    }
  }

  /* add 1 because it starts from 0 */
  afdCtrl->num_peaks = max_peaks;

  CDBG_AFD("before free sum1 = %p\n", sum1);


  free (sum1);
  free (sum2);


  CDBG_AFD("End afd_multiple_peaks\n");
} /* afd_multiple_peaks */

/*===========================================================================

FUNCTION:       afd_multiple_peaks_decision

DESCRIPTION:
  Flicker is detected based on banding locations, banding distances
  computed by afd_multiple_peaks ().

DEPENDENCIES:

SIDE EFFECTS:

RETURN VALUE:
 1 - Flicker
 0 - No Flicker

============================================================================*/
static int32_t afd_multiple_peaks_decision(afd_t *afdCtrl)
{
  float avg_width = 0;
  float std_width = 0;
  int32_t i, j;
  int32_t flicker = 0;
  int32_t actual_peaks = 0;
  int32_t max_width = 0, min_width = 1000;
  int32_t current_width, max_index = 0, min_index = 0;
  int32_t *width_1d;

  CDBG_AFD("start afd_multiple_peaks_decision\n");

  if (!afdCtrl->num_peaks) {
    return flicker;
  }

  width_1d = (int32_t *)malloc(sizeof(int32_t) *
    (AFD_NUM_FRAMES - AFD_FRAME_SKIP) * (afdCtrl->num_peaks - 1));

  if (width_1d == NULL) {
    return flicker;
  }
  memset(width_1d, 0, sizeof(int32_t) *
    (AFD_NUM_FRAMES - AFD_FRAME_SKIP) * (afdCtrl->num_peaks - 1));
  for (j = 0; j < AFD_NUM_FRAMES - AFD_FRAME_SKIP; j++) {
    /* width[] has one less entries than edge[] */
    for (i = 0; i < afdCtrl->num_peaks - 1 ; i++) {
      current_width =  afdCtrl->peak_width[j][i];

      /* we only count non-zero values */
      if (current_width) {

        width_1d[actual_peaks] = current_width;
        avg_width += afdCtrl->peak_width[j][i];

        if (current_width < min_width) {
          min_width = current_width;
          min_index = actual_peaks;
        }

        if (current_width > max_width) {
          max_width = current_width;
          max_index = actual_peaks;
        }

        actual_peaks++;
      }
    }
  } /* j */

  afdCtrl->actual_peaks = actual_peaks;

  CDBG_AFD("actual_peaks = %d\n", actual_peaks);

  /* we need at least certain number of data pts to be statistically
   * significant
   */
/*  if (actual_peaks < 6) {
    free(width_1d);
    return flicker;
  } */

  /* remove max and min value */
  avg_width = avg_width - max_width - min_width;
  avg_width = avg_width / (actual_peaks - 2);

  if (avg_width == 0) {
    CDBG_AFD("avg_width = %f\n", avg_width);
    /* making avg_wdith to 1 if it is zero. */
    avg_width = 1;
  }

  for (i = 0; i < actual_peaks ; i++) {
    if ((i!= max_index) && (i != min_index)) {
      std_width += ((width_1d[i] - avg_width) * (width_1d[i] - avg_width));
    }
  }

  std_width = std_width / (actual_peaks - 2);
  std_width =(float) pow((double)std_width, 0.5);
  std_width = std_width / avg_width;

  afdCtrl->std_width = (int)(std_width * Q8);

  if (afdCtrl->std_width < afdCtrl->std_threshold) {
    flicker = 1;
  }

  free(width_1d);

  CDBG_AFD("end afd_multiple_peaks_decision\n");

  return flicker;
} /* afd_multiple_peaks_decision */
/*===========================================================================

FUNCTION: afd_single_peak

DESCRIPTION:
  Identifies banding location for single banding case.
  High zoom factor / smaller viewfinder size may result in a single banding.

INPUT:  frame_rate  - Q8 number
        zoom_factor - Q12 number

DEPENDENCIES:

SIDE EFFECTS:

RETURN VALUE:
  None

============================================================================*/
static void afd_single_peak(afd_t *afdCtrl,int32_t frame_rate, int32_t zoom_factor)
{
  int32_t i, j;
  int32_t max_peak = 0;
  int32_t search_range = 40;
  int32_t number_smooth = 3;
  int32_t number_taps = 9;
  int32_t temp;
  int32_t max_index = 0;
  int32_t start_pos, end_pos, loop;
  int32_t size;
  int32_t *sum1, *sum2;

  CDBG_AFD("start afd_single_peak, frame_rate = %d, zoom_factor = %d\n",
    frame_rate, zoom_factor);

  size = afdCtrl->array_size;

  search_range = afdCtrl->single_peak_mode_search_range * Q12 / zoom_factor;

  /* SUM_ARRAY_SIZE = size / EVERY_N_ROWS;
   * for now SUM_ARRAY_SIZE=size */
  sum1 = (int32_t *) malloc (sizeof (int32_t ) * size);
  if (sum1 == NULL) {
    CDBG_AFD("sum1 == 0\n");
    return;
  }
  sum2 = (int32_t *) malloc (sizeof (int32_t ) * size);

  if (sum2 == NULL) {
    CDBG_AFD("sum2 == 0\n");
    free (sum1);
    return;
  }

  for (loop = 0; loop < AFD_NUM_FRAMES - AFD_FRAME_SKIP;loop ++) {
    for (i=0; i < size; i++) {
      /* sum2[i] = sum1[i] - sum2[i];
       * here sum2 is the diff */
      sum2[i] = afdCtrl->row_sum[loop][i] -
        afdCtrl->row_sum[loop + AFD_FRAME_SKIP][i];

      if (sum2[i] < 0) {
        sum2[i] = 0;
      }
    }
    /* 9 tap filter */
    /* take care of borders first */
    temp = number_taps / 2;
    for (i = 0; i < temp; i++) {
      sum1[i] = sum2[i];
    }

    for (i = size-temp; i < size; i++) {
      sum1[i] = sum2[i];
    }

    for (j = 0 ; j < number_smooth; j++) {
      for (i = temp; i < size-temp; i++) {
        /* sum1 will be the smoothed sum2 */
        sum1[i] = 0;
        for (j = i - temp; j <= i + temp; j++) {
          sum1[i] += sum2[j];
        }
        sum1[i] = (sum1[i] + temp)/ number_taps;
      }

      for (i = temp; i < size-temp; i ++) {
        sum2[i] = sum1[i];
      }
    } /* NUM_SMOOTH */
    /* at this point both sum1 and sum2 contain smoothed data */
    /* if previous frame did not find anything, use the
     * whole search range */
    if (loop == 0 || afdCtrl->edge[loop-1][0] == 0) {
      /* ensure enough room to search both ends */
      start_pos = search_range;
      end_pos = size - search_range;
    } else {
      start_pos = afdCtrl->edge[loop-1][0] - search_range;

      if (start_pos < 2) {
        start_pos = 2;
      }

      end_pos = start_pos + 2 * search_range;

      if (end_pos > size - 1) {
        end_pos = size-1;
      }
    }
    max_peak = 0;

    for (i = start_pos; i < end_pos; i ++) {
      if (sum1[i] >= max_peak) {
        max_peak = sum1[i];
        max_index = i; /* find the highest peak location. */
      }
    }

    /* only if diff signal is above certain value,
     * we consider flicker exists. */
    if (max_peak > (afdCtrl->diff_threshold / 2)) {
      afdCtrl->edge[loop][0] = max_index;
    }
  } /* end for loop */

  free (sum1);
  free (sum2);

  CDBG_AFD("end afd_single_peak\n");
} /* afd_single_peak */
/*===========================================================================

FUNCTION: afd_single_peak_decision

DESCRIPTION:
  For single frame mode, flicker exists if for continous N frames
  the banding shift is in the same direction. This routine calculates
  banding shift from frame to frame based on the banding location computed by
  afd_single_peak ().

DEPENDENCIES:

SIDE EFFECTS:

RETURN VALUE:
  1 - Flicker
  0 - No Flicker

============================================================================*/
static int32_t afd_single_peak_decision(afd_t *afdCtrl)
{
  int32_t *edge_shift; /* debug purpose */
  int32_t i;
  int32_t positive_ct = 0, negative_ct = 0;
  int32_t ratio_p = 0, ratio_n = 0;
  int32_t flicker = 0;
  int32_t valid_peak_cts = 0;

  CDBG_AFD("start afd_single_peak_decision\n");

  /* dimension for edge is AFD_NUM_FRAMES-AFD_FRAME_SKIP,
   * so the dimension for shift is AFD_NUM_FRAMES-AFD_FRAME_SKIP-1 */
  edge_shift = (int32_t *) malloc (sizeof (int32_t) *
    (AFD_NUM_FRAMES - AFD_FRAME_SKIP - 1));
  if (edge_shift == NULL) {
    CDBG_AFD("edge_shift == NULL\n");
    return flicker;
  }

  for (i = 0; i < (AFD_NUM_FRAMES - AFD_FRAME_SKIP - 1); i++) {
    /* both edge has to be non-zero to be valid */
    if (afdCtrl->edge[i][0] && afdCtrl->edge[i+1][0]) {
      valid_peak_cts++;
      edge_shift[i] = afdCtrl->edge[i][0] - afdCtrl->edge[i+1][0];
      if (edge_shift[i] > 0) {
        positive_ct++;
      } else if (edge_shift[i] < 0) {
        negative_ct++;
      }
    }
  }
  free (edge_shift);
  /* Needs to have all 4 banding shifts, this means we need every
   * frame to have a peak in the desired location  */
  if (valid_peak_cts < AFD_NUM_FRAMES - 2) {
    return flicker;
  }
  if (valid_peak_cts) {
    ratio_p = positive_ct * 100 / valid_peak_cts;
    ratio_n = negative_ct * 100 / valid_peak_cts;
  }
  afdCtrl->ratio_p = ratio_p;
  afdCtrl->ratio_n = ratio_n;

  if (ratio_p > afdCtrl->percent_thresh ||
    ratio_n > afdCtrl->percent_thresh)
    flicker = 1;

  CDBG_AFD("end afd_single_peak_decision\n");

  return flicker;
} /* afd_single_peak_decision */

/*===========================================================================
FUNCTION:  afd_zoomfactor
============================================================================*/
static int32_t afd_zoomfactor(frame_proc_t *frameCtrl)
{
  uint32_t rc;

  rc = MIN((uint32_t)MIN(frameCtrl->input.isp_info.afd_zoom_xScale,
    frameCtrl->input.isp_info.afd_zoom_yScale),
    frameCtrl->input.mctl_info.crop_factor);

  if (rc < Q12)
    rc = Q12;

  CDBG_AFD("afd_crop_factor = %d\n", rc);
  return rc;
}

/*===========================================================================
FUNCTION: afd_config
============================================================================*/
static void afd_config(afd_t *afdCtrl, int32_t reset_status, uint32_t dx,
  uint32_t dy, int32_t zoom_factor, stats_proc_aec_info_t *aec_output,
  frame_proc_t * frameCtrl)
{
  int32_t i, j;
  chromatix_auto_flicker_detection_data_type *chromatix_afd_ptr;
  chromatix_parms_type *chromatix_ptr = frameCtrl->input.chromatix;

  CDBG_AFD("start afd_config, reset_status = %d, zoom_factor = %d\n",
    reset_status, zoom_factor);

  chromatix_afd_ptr = &chromatix_ptr->auto_flicker_detection_data;
  /* Set to QQVGA size for minimum acceptable computation */
  afdCtrl->afd_max_dx = 160;
  afdCtrl->afd_max_dy = 120;

  if (dx > afdCtrl->afd_max_dx)
    afdCtrl->afd_row_subsample = dx / afdCtrl->afd_max_dx;
  else
    afdCtrl->afd_row_subsample = 1;

  if (dy > afdCtrl->afd_max_dy)
    afdCtrl->afd_col_subsample = dy / afdCtrl->afd_max_dy;
  else
    afdCtrl->afd_col_subsample = 1;

  /* Number of consecutive frames for computing row sum data */
  afdCtrl->row_sum_frame_cnt = AFD_NUM_FRAMES;
  afdCtrl->num_peaks   = 0;
  afdCtrl->array_size  = dy / afdCtrl->afd_row_subsample;
  afdCtrl->percent_thresh = chromatix_afd_ptr->afd_percent_threshold;

  afdCtrl->std_threshold  = chromatix_afd_ptr->afd_std_threshold * Q8;

  /* for QQVGA col=160 threshold is 90,
   * other resolution and subsampling rate needs to be
   * adjusted accordingly */
  afdCtrl->diff_threshold =
    (float) (chromatix_afd_ptr->afd_diff_threshold * dy) /
    (float) (afdCtrl->afd_col_subsample * afdCtrl->afd_max_dx);

  /* half of the VF frame length. i.e., half the max. exposure line cnt */
  if (aec_output->max_line_cnt)
    afdCtrl->line_count_threshold = aec_output->max_line_cnt / 2;
  else
    afdCtrl->line_count_threshold = 300;
  afdCtrl->line_count_threshold = 6000;

  afdCtrl->frame_ct_threshold  = chromatix_afd_ptr->afd_frame_ct_threshold;

  afdCtrl->single_peak_mode_search_range =
    (20 * zoom_factor * afdCtrl->array_size / afdCtrl->afd_max_dy) >> 12;

  if (!afdCtrl->single_peak_mode_search_range)
    afdCtrl->single_peak_mode_search_range = 1;

  if (reset_status) {
    frameCtrl->output.afd_d.afd_status = AFD_OFF;
    afdCtrl->afd_needed = 0;
    afdCtrl->frame_index = 0;
    afdCtrl->flicker_freq = 0;
    frameCtrl->output.afd_d.afd_state = AFD_STATE_ON;
  }

  for (i=0; i < 8; i++)
    for (j= 0; j < (AFD_NUM_FRAMES - AFD_FRAME_SKIP); j++)
      afdCtrl->edge[j][i] = 0;

  for (i=0; i < 7; i++)
    for (j= 0; j < AFD_NUM_FRAMES - AFD_FRAME_SKIP; j++)
      afdCtrl->peak_width[j][i] = 0;

  for (i=0; i < AFD_NUM_FRAMES; i++)
    for (j= 0; j < afdCtrl->array_size; j++)
      afdCtrl->row_sum[i][j] = 0;

  afdCtrl->std_width = -1;
  afdCtrl->ratio_p = -1;
  afdCtrl->ratio_n = -1;
  afdCtrl->multiple_peak_algo = -1;
  afdCtrl->actual_peaks = 0;

  CDBG_AFD("end afd_config\n");
} /* afd_config */

/*===========================================================================
 * FUNCTION    - afd_process -
 *
 * DESCRIPTION: AFD entry routine
 *==========================================================================*/
static int32_t afd_process(afd_t *afdCtrl, int32_t frame_rate, int32_t zoom_factor)
{
  int32_t flicker = 0;
  int32_t zoom_limit;

  zoom_limit = (3 * Q12 * 30 * Q8 / (frame_rate * 2));  /*1.5 * 30/frame_rate */

  CDBG_AFD("start afd_process, zoom_factor = %d, zoom_limit = %d\n",
    zoom_factor, zoom_limit);

  if (zoom_factor <= zoom_limit) {

    afdCtrl->multiple_peak_algo = 1;
    afd_multiple_peaks(afdCtrl,frame_rate, zoom_factor);
    flicker = afd_multiple_peaks_decision(afdCtrl);
  } else if ((zoom_factor > zoom_limit) &&
    (zoom_factor <= (((float) (120 * Q8) / frame_rate) * Q12))) {

    afdCtrl->multiple_peak_algo = 0;
    afd_single_peak (afdCtrl, frame_rate, zoom_factor);
    flicker = afd_single_peak_decision(afdCtrl);
  }

  CDBG_AFD("end afd_process %d\n", flicker);

  return flicker;
} /* afd_process */

/*===========================================================================
 * FUNCTION    - afd_needed
 *==========================================================================*/
static int32_t afd_needed(frame_proc_t *frameCtrl, afd_t *afdCtrl,
  int32_t line_count, int32_t frame_rate,
  int32_t flicker_50Hz_line_gap)
{
  int32_t need_fd = 0;
  CDBG_AFD("%s: IN \n", __func__);
  /* when there is flicker but none of the antibanding table worked
   * or afdCtrl->flicker_freq = 0 means did not detect flicker
   * we need to re run AFD */
  if (afdCtrl->afd_done) {
    CDBG_AFD("%s: afd_output.afd_done. afd_needed:%d \n", __func__,need_fd);
    return need_fd;
  }

  if (afdCtrl->frame_index < afdCtrl->frame_ct_threshold) {
    afdCtrl->frame_index++;
    afdCtrl->afd_needed = need_fd;
    CDBG_AFD("%s: threshold afd_needed:%d \n", __func__,need_fd);
    return need_fd;
  }
  /* reset frame counter */
  afdCtrl->frame_index = 0;

  /* AFD is required if sensor integration time is
   * longer than one flicker cycle and shorter than the threshold. */
  if (((frame_rate >=
    frameCtrl->input.sensor_info.max_preview_fps) ||
    ((line_count < afdCtrl->line_count_threshold) &&
    (frame_rate >= frameCtrl->input.sensor_info.max_preview_fps / 2))) &&
    (line_count > flicker_50Hz_line_gap)) {
    need_fd = 1;
  }
  afdCtrl->afd_needed = need_fd;

  CDBG_AFD("%s: OUT afd_needed:%d \n", __func__,need_fd);

  return need_fd;
} /* afd_needed */

/*===========================================================================
 * FUNCTION    - afd_execute -
 *
 * DESCRIPTION:
 *==========================================================================*/
int afd_execute(void *Ctrl, afd_t *afdCtrl)
{
  uint32_t afd_line_loop_cnt;
  uint32_t afd_pix_loop_cnt;
  uint32_t afd_temp1, afd_temp2;
  uint16_t camera_preview_dx;
  uint16_t camera_preview_dy;
  uint8_t *afd_row_addr;
  int8_t rc = -1;
  frame_proc_t *frameCtrl = (frame_proc_t *) Ctrl;
  struct msm_pp_frame * pframe = &(frameCtrl->input.mctl_info.frame);

  stats_proc_aec_info_t *aec_output = &(frameCtrl->input.statsproc_info.aec_d);
  struct msm_pp_frame frame;

  camera_preview_dx = frameCtrl->input.mctl_info.display_dim.height;
  camera_preview_dy = frameCtrl->input.mctl_info.display_dim.width;
  if (frameCtrl->output.afd_d.afd_state ==  AFD_STATE_DONE) {
    CDBG_ERROR("%s :DONE with AFD, So dont divert frame",__func__);
    return -1;
  }
  CDBG_AFD("%s: camera_preview_dx=%d camera_preview_dy=%d\n",
    __func__, camera_preview_dx, camera_preview_dy);
  CDBG_AFD("%s: IN: frameCtrl->output.afd_d.afd_status = %d \n",
    __func__, frameCtrl->output.afd_d.afd_status);

  if (frameCtrl->input.statsproc_info.af_d.is_af_active <= 0) {
    /* TODO: Add zoom condition
     *  && (afd_start_zoom_val == camera_parm_zoom.current_value)) */
    CDBG_AFD("%s: frameCtrl->output.afd_d.afd_status = %d \n",
      __func__, frameCtrl->output.afd_d.afd_status);
    switch (frameCtrl->output.afd_d.afd_status) {
      case AFD_OFF: {
          /* for the beginning of FD, we need AEC to be settled */
          if (aec_output->aec_settled == FALSE) {
            CDBG_AFD("FAILED to set AEC_ANTIBANDING\n");
            CDBG_AFD("%s: AEC is not settled. Retry... \n", __func__);
            return -1;
          } else {
            CDBG_AFD("%s: AEC is settled. \n", __func__);
            afd_config(afdCtrl, 0, camera_preview_dx, camera_preview_dy,
              afd_zoomfactor(frameCtrl), aec_output, frameCtrl);

            if (afd_needed(frameCtrl, afdCtrl, aec_output->cur_line_cnt,
              frameCtrl->input.sensor_info.preview_fps, aec_output->band_50hz_gap)) {

              CDBG_AFD("%s: Start AFD, no antibanding table, "
                "frameCtrl->output.afd_d.afd_status = %d",
                __func__, frameCtrl->output.afd_d.afd_status);
              /* at this point frame_index is reset to 0 by
               * camera_afd_needed() load regular exposure table */
              frameCtrl->output.afd_d.afd_status = AFD_REGULAR_EXPOSURE_TABLE;
              afdCtrl->frame_index = -3;
              frameCtrl->output.afd_d.afd_antibanding_type =
                CAMERA_ANTIBANDING_OFF;
            } else {
              CDBG_AFD("%s: AFD is not needed \n", __func__);
            }
          }
        }
        break;

      case AFD_REGULAR_EXPOSURE_TABLE:
        /* we need to collect 6 frame data or analyze 6 frame data*/
      case AFD_60HZ_EXPOSURE_TABLE:
      case AFD_50HZ_EXPOSURE_TABLE: {
          /* Requested for frames for frameprocessing earlier, pFrame should now
     * points to a valid frame */
          if (!pframe || !(pframe->mp[0].vaddr) ||
            pframe->mp[0].vaddr == pframe->mp[1].vaddr) {
            if (!pframe) {
              CDBG_ERROR("%s: pframe NULL %p\n", __func__, pframe);
              return -1;
            }
            if (!(pframe->mp[0].vaddr))
              CDBG_ERROR("%s: pframe->mp[0].vaddr NULL %p\n", __func__, pframe);
            return -1;
          }
          frame = *pframe;

          CDBG_AFD("FRAME_PROC_PREV is enabled \n");
          CDBG_AFD("%s: Frame details: buffer=0x%lu y_off=%lu cbcr_off=%lu\n",
            __func__, frame.mp[0].vaddr, frame.mp[0].vaddr, frame.mp[1].vaddr);
          if (frame.mp[0].vaddr == 0 || frame.mp[1].vaddr == 0) {
            CDBG_AFD("%s: Failed: Frame details are not valid.\n", __func__);
            return -1;
          }
          CDBG_AFD("%s: Collect frame\n", __func__);
          if (afdCtrl->frame_index < 0) {
            /* skip a frame after changing exposure table */
            afdCtrl->frame_index++;
          } else if (afdCtrl->frame_index < afdCtrl->row_sum_frame_cnt) {
            CDBG_AFD("%s: start afd row sum frame index = %d",
              __func__, afdCtrl->frame_index);
            afd_line_loop_cnt =
              camera_preview_dx / afdCtrl->afd_row_subsample;

            afd_pix_loop_cnt =
              camera_preview_dy / afdCtrl->afd_col_subsample;

            afd_temp1 = afdCtrl->afd_row_subsample *
              camera_preview_dx;

            /* collect rowsum data for this frame */
            for (afdCtrl->row_sum_line_cnt = 0;
              afdCtrl->row_sum_line_cnt < afd_line_loop_cnt;
              ++afdCtrl->row_sum_line_cnt) {

              afdCtrl->row_sum_pixel_cnt = 0;
              afdCtrl->row_sum[afdCtrl->
                frame_index][afdCtrl->row_sum_line_cnt] = 0;

              afd_row_addr = (((uint8_t *)(frame.mp[0].vaddr)) +
                (afdCtrl->row_sum_line_cnt * afd_temp1));
              afd_temp2 = 0;

              while (afdCtrl->row_sum_pixel_cnt < afd_pix_loop_cnt) {
                afd_temp2 +=
                  *(afd_row_addr +
                  (afdCtrl->row_sum_pixel_cnt * afdCtrl->afd_col_subsample));

                afdCtrl->row_sum_pixel_cnt += 1;
              }

              afdCtrl->row_sum [afdCtrl->frame_index][afdCtrl->
                row_sum_line_cnt] = afd_temp2;
            }
            afd_temp1 = 0;
            if (afdCtrl->frame_index > 0) {
              for (afd_temp2 = 0; afd_temp2 < afd_line_loop_cnt;
                ++afd_temp2) {
                if (afdCtrl->row_sum [afdCtrl->frame_index][afdCtrl->
                  row_sum_line_cnt] > afdCtrl->row_sum [afdCtrl->frame_index -
                  AFD_FRAME_SKIP][afdCtrl->row_sum_line_cnt]) {
                  ++afd_temp1;
                }
              }
            }
            if (afd_temp1 != (afd_line_loop_cnt + 1)) {
              afdCtrl->frame_index++;
            }
          } else { /* need to analyze data */
            /* this will reset frame counter, 0 means not to reset status */
            /* move this to after each afd_process */
            /*  afdCtrl->array_size = camera_preview_dy; */
            /* keep the frame ct so we won't try to collect more row_sum */
            afdCtrl->frame_index = afdCtrl->row_sum_frame_cnt;
            /* 1 - flicker present */
            if (afd_process(afdCtrl, frameCtrl->input.sensor_info.preview_fps,
              afd_zoomfactor(frameCtrl)) == 1) {
              if (frameCtrl->output.afd_d.afd_status == AFD_REGULAR_EXPOSURE_TABLE) {
                /* 1 - just analyzed first N frames of data using the
                 * regular exposure table. load antibanding table for 60hz */
                frameCtrl->output.afd_d.afd_status = AFD_60HZ_EXPOSURE_TABLE;
                afdCtrl->frame_index = -3;
                frameCtrl->output.afd_d.afd_antibanding_type =
                  CAMERA_ANTIBANDING_60HZ;
                CDBG_AFD("%s: std_width=%d,  load 60hz banding table"
                  "frameCtrl->output.afd_d.afd_status = %d",
                  __func__, afdCtrl->std_width, frameCtrl->output.afd_d.afd_status);
              } else if (frameCtrl->output.afd_d.afd_status == AFD_60HZ_EXPOSURE_TABLE) {
                /* 2 means analysed data using 60 hz antibanding table
                           * load antibanding table for 50hz */
                frameCtrl->output.afd_d.afd_status = AFD_50HZ_EXPOSURE_TABLE;
                afdCtrl->frame_index = -3;
                frameCtrl->output.afd_d.afd_antibanding_type =
                  CAMERA_ANTIBANDING_50HZ;
                CDBG_AFD("%s: std_width=%d,  load 50hz banding table afd_antibanding_type = %d",
                  __func__, afdCtrl->std_width, frameCtrl->output.afd_d.afd_status);
              } else if (frameCtrl->output.afd_d.afd_status == AFD_50HZ_EXPOSURE_TABLE) {
                /* (frameCtrl->output.afd_d.afd_status == 3 )
                 * 3 means analysed data using 50 hz antibanding table,
                 * at this point, both table are used with no effect
                 * load regular exposure table.
                 * this means some of the assumption are not satisfied,
                 * re-run immediately if the next AFD session is far in time. */
                afdCtrl->flicker_freq = -1;
                frameCtrl->output.afd_d.afd_status = AFD_OFF;
                afdCtrl->frame_index = -3;
                frameCtrl->output.afd_d.afd_antibanding_type =
                  CAMERA_ANTIBANDING_OFF;
                CDBG_AFD("%s: std_width=%d,  re-run AFD, afd_antibanding_type = %d",
                  __func__, afdCtrl->std_width, frameCtrl->output.afd_d.afd_status);
                afdCtrl->afd_done = 1;
                frameCtrl->output.afd_d.afd_state = AFD_STATE_DONE;
                CDBG_ERROR("%s:%d: afd is DONE.\n", __func__, __LINE__);
              }
            } else { /* afd_process () */
              /* no flicker, detected light freq */
              if (frameCtrl->output.afd_d.afd_status == AFD_60HZ_EXPOSURE_TABLE) {
                afdCtrl->flicker_freq = 60;
              } else if (frameCtrl->output.afd_d.afd_status == AFD_50HZ_EXPOSURE_TABLE) {
                afdCtrl->flicker_freq = 50;
              } else if (frameCtrl->output.afd_d.afd_status == AFD_REGULAR_EXPOSURE_TABLE) {
                afdCtrl->flicker_freq = 0; /* no flicker */
              }

              frameCtrl->output.afd_d.afd_status = AFD_OFF;
              afdCtrl->frame_index = 0;
              CDBG_AFD("%s: AFD success, std_width = %d, \
            afdCtrl->flicker_freq = %d", __func__, afdCtrl->std_width,
                afdCtrl->flicker_freq);
              /* Send FRAME_PROC_PREV DONE to kernel */
              afdCtrl->afd_done = 1;
              frameCtrl->output.afd_d.afd_state = AFD_STATE_DONE;
              CDBG_HIGH("%s:%d: afd is DONE.\n", __func__, __LINE__);
            }

            /* we reset everything after each afd_process */
            afd_config(afdCtrl, 0, camera_preview_dx, camera_preview_dy,
              afd_zoomfactor(frameCtrl), aec_output, frameCtrl);

          } /* end else need to analyze data */

          if (!afdCtrl->afd_done) {
            frameCtrl->output.afd_d.afd_state = AFD_STATE_ON;
          }
        }
        break;

      default:
        break;
    } /* switch */
  } else {
    /* reset AFD process */
    CDBG_AFD("%s: things changed during AFD, reset the process, "
      "afd_antibanding_type = %d\n",
      __func__, frameCtrl->output.afd_d.afd_status);

    afd_config(afdCtrl, 1, camera_preview_dx, camera_preview_dy,
      afd_zoomfactor(frameCtrl), aec_output, frameCtrl);
  }
  return 0;
}


