/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "afd.h"
#include "stats_proc.h"
#include "camera_dbg.h"

/* Set to QQVGA size for minimum acceptable computation */
#define AFD_ZOOM_FACTOR Q12 /* Q12 is default zoom factor which is unity */
#define SAMPLE_RATIO 3
static int frame_count = 0;

/*===========================================================================
 * FUNCTION    - afd_reset -
 *
 * DESCRIPTION
 *===========================================================================*/
void afd_reset(stats_proc_t *sproc, afd_t *afd)
{
  CDBG_AFD("%s", __func__);
  afd->afd_done = 0;
  afd->state = AFD_STATE_OFF;
  sproc->share.afd_status = AFD_OFF;
  afd->frame_index = 0;
  afd->frame_skip  = 0;
} /* afd_reset */

/*===========================================================================
 * FUNCTION    - afd_multiple_peaks -
 *
 * DESCRIPTION: For multiple banding case, identifies banding locations,
 * distance between bandings
 *==========================================================================*/
static void afd_multiple_peaks(stats_proc_t *sproc, afd_t *afd)
{
  int i, j, k, counter = 0, loop, ratio, edge_pos, number_smooth;
  int max_peak = 0, max_peaks = 0, max_num_peaks;  /* in Q8 */
  /* at VGA resolution, we need 9 times smoothing,
   * smaller resolution needs smaller smoothing iterations */
  int number_taps = 9, peak_dist, temp, new_edge_pos;
  int size = afd->array_size;
  int *sum1, *sum2;
  int peak_counts[AFD_NUM_FRAMES - AFD_FRAME_SKIP];
  int frame_rate = sproc->input.sensor_info.preview_fps;

  CDBG_AFD("%s: frame_rate %d", __func__, frame_rate);
  if (!frame_rate) {
    CDBG_ERROR("frame_rate 0");
    return;
  }
  /* Determines the computation time */
  number_smooth = (10 - 480 / afd->array_size) / 2;
  /* this is for without zoom, Q8 */
  max_num_peaks =  ((int)(120 * Q8 * Q8) / frame_rate);

  if (max_num_peaks < 256)
    return;

  /* 4 is for 60 Hz, 3.3 is for 50Hz, so can cover worst case,
   * 2 is to ensure not to miss a peak */
  peak_dist = (int32_t) (AFD_ZOOM_FACTOR * size * Q8 / (max_num_peaks * Q12));
  max_num_peaks = max_num_peaks / Q8;  /* change to Q0 */

  if ((sum1 = (int32_t *) malloc (sizeof (int) * size)) == NULL) {
    CDBG_ERROR("sum1 == NULL");
    return;
  }
  memset(sum1, 0, sizeof (int) * size);

  if ((sum2 = (int32_t *) malloc (sizeof (int) * size)) == NULL) {
    free (sum1);
    CDBG_ERROR("sum2 == NULL");
    return;
  }
  memset(sum2, 0, sizeof (int) * size);
  for (loop = 0; loop < AFD_NUM_FRAMES - AFD_FRAME_SKIP; loop++) {
    peak_counts[loop] = 0;
    for (i=0; i < size; i ++) {
      sum2[i] = afd->row_sum[loop][i] -
        afd->row_sum[loop + AFD_FRAME_SKIP][i];
      if (sum2[i] < 0)
        sum2[i] = 0;
    }
    /* 9 tap filter take care of borders first */
    temp = number_taps/2;

    for (i = 0; i < temp; i++)
      sum1[i] = sum2[i];

    for (i = size - temp; i < size; i ++)
      sum1[i] = sum2[i];

    for (k = 0 ; k < number_smooth; k++) {
      for (i = temp; i < size-temp; i ++) {
        /* sum1 will be the smoothed sum2 */
        sum1[i] = 0;
        for (j = i - temp; j <= i + temp; j ++)
          sum1[i] += sum2[j];
        sum1[i] = (sum1[i] + temp)/number_taps;
      }
      for (i = temp; i < size-temp; i ++)
        sum2[i] = sum1[i];
    }

    edge_pos = -1;
    counter = -1;
    for (i = 0; i < size; i ++)
      if (sum1[i] >= max_peak)
        max_peak = sum1[i];

      /* diff signal has to be above this value to be
         * considered flicker exists */
      /* threshold is programmable based on zoom factor */
    if (max_peak >= ((afd->diff_threshold << 12) / AFD_ZOOM_FACTOR)) {
      /* max_peak *= 0.1 */
      max_peak *= 25;
      max_peak >>= 8;

      /* sum1 is the smoothed diff, sum2 is derivative of smoothed diff */
      for (i = 1; i < size; i++)
        sum2[i] = sum1[i-1]-sum1[i];

      for (i = 0; i < size-1; i++) {
        if (counter < max_num_peaks) {
          if (sum2[i] <= 0 && sum2[i+1] >= 0 && sum1[i] > max_peak) {
            if (sum2[i+1]!=0 && sum2[i]!=0) {
              /* use shift instead of Q8 to reduce latency */
              ratio = (abs(sum2[i]) << 8) / abs(sum2[i+1]);
              temp = (ratio)/(Q8+ratio);
              new_edge_pos = temp + i;
            } else if (sum2[i+1] == 0)
              new_edge_pos = i + 1;
            else
              new_edge_pos = i;

            if (edge_pos == -1) {
              counter=counter+1;
              /* decimal precision */
              edge_pos = new_edge_pos;
              afd->edge[loop][counter]=edge_pos;
            } else {
              temp = new_edge_pos - edge_pos + 1;
              if (temp > peak_dist) {
                counter = counter + 1;
                afd->peak_width[loop][counter-1] = temp;
              }
              edge_pos = new_edge_pos;
              afd->edge[loop][counter]=edge_pos;
            }
          }/* sum2(i) <= 0 && sum2(i+1) >= 0 && sum1(i) > max_peak) */
        } else
          i = size - 1;
      } /* i */
    }
    peak_counts[loop] = counter + 1;
  } /* loop */
  for (i = 0; i < AFD_NUM_FRAMES - AFD_FRAME_SKIP; i ++)
    if (peak_counts[i] > max_peaks)
      max_peaks = peak_counts[i];

    /* add 1 because it starts from 0 */
  afd->num_peaks = max_peaks;
  free (sum1);
  free (sum2);
  CDBG_HIGH("%s max_peaks %d", __func__, max_peaks);
} /* afd_multiple_peaks */

/*==========================================================================
 * FUNCTION    - afd_detect_flicker -
 *
 * DESCRIPTION: Flicker is detected based on banding locations,
 * banding distances computed by afd_multiple_peaks ().
 *=========================================================================*/
static int afd_detect_flicker(stats_proc_t *sproc, afd_t *afd)
{
  float avg_width = 0, std_width = 0;
  int i, j, *width_1d, max_width = 0, min_width = 1000;
  int current_width, max_index = 0, min_index = 0;

  if (!afd->num_peaks)
    return 0;

  width_1d = (int32_t *)malloc(sizeof(int32_t) *
    (AFD_NUM_FRAMES - AFD_FRAME_SKIP) * (afd->num_peaks - 1));

  if (width_1d == NULL)
    return 0;
  afd->actual_peaks = 0;
  memset(width_1d, 0, sizeof(int32_t) *
    (AFD_NUM_FRAMES - AFD_FRAME_SKIP) * (afd->num_peaks - 1));
  for (j = 0; j < AFD_NUM_FRAMES - AFD_FRAME_SKIP; j++) {
    /* width[] has one less entries than edge[] */
    for (i = 0; i < afd->num_peaks - 1 ; i++) {
      current_width =  afd->peak_width[j][i];

      /* we only count non-zero values */
      if (current_width) {
        width_1d[afd->actual_peaks] = current_width;
        avg_width += afd->peak_width[j][i];

        if (current_width < min_width) {
          min_width = current_width;
          min_index = afd->actual_peaks;
        }
        if (current_width > max_width) {
          max_width = current_width;
          max_index = afd->actual_peaks;
        }
        afd->actual_peaks++;
      }
    }
  } /* j */
  CDBG_HIGH("%s actual_peaks %d", __func__, afd->actual_peaks);
  /* we need at least certain number of data pts to be statistically
   * significant */
  if (afd->actual_peaks < 6) {
    free(width_1d);
    return 0;
  }
  /* remove max and min value */
  avg_width = avg_width - max_width - min_width;
  avg_width = avg_width / (afd->actual_peaks - 2);

  if (avg_width == 0) {
    CDBG_AFD("%s: avg_width = %f", __func__, avg_width);
    /* making avg_wdith to 1 if it is zero. */
    avg_width = 1;
  }

  for (i = 0; i < afd->actual_peaks ; i++)
    if ((i!= max_index) && (i != min_index))
      std_width += ((width_1d[i] - avg_width) * (width_1d[i] - avg_width));

  std_width = std_width / (afd->actual_peaks - 2);
  std_width =(float) pow((double)std_width, 0.5);
  std_width = std_width / avg_width;

  afd->std_width = (int)(std_width * Q8);
  free(width_1d);
  if (afd->std_width < afd->std_threshold)
    return 1;
  else
    return 0;
} /* afd_detect_flicker */

/*==========================================================================
 * FUNCTION    - afd_config -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void afd_config(stats_proc_t *sproc, afd_t *afd, int reset_status)
{
  int i, j, col_subsmpl = 1;
  chromatix_auto_flicker_detection_data_type *afd_cptr =
    &(sproc->input.chromatix->auto_flicker_detection_data);

  CDBG_AFD("%s reset_status %d", __func__, reset_status);
    afd->row_subsmpl = SAMPLE_RATIO;

    col_subsmpl = 1;

  /* Number of consecutive frames for computing row sum data */
  afd->num_peaks   = 0;
  afd->array_size  = sproc->input.mctl_info.rgnVnum / afd->row_subsmpl;
  afd->std_threshold  = afd_cptr->afd_std_threshold * Q8;

  /* for QQVGA col = 160 threshold is 90 */
  afd->diff_threshold = (float) (afd_cptr->afd_diff_threshold);

  /* half of the VF frame length. i.e., half the max. exposure line cnt */
  if (sproc->share.aec_ext.max_line_cnt)
    afd->line_count_threshold = sproc->share.aec_ext.max_line_cnt / 2;
  else
    afd->line_count_threshold = 300;

  afd->frame_ct_threshold  = afd_cptr->afd_frame_ct_threshold;
  if (reset_status) {
    sproc->share.afd_status = AFD_OFF;
    afd->frame_index = 0;
    afd->state = AFD_STATE_ON;
  }
  for (i=0; i < 8; i++)
    for (j= 0; j < (AFD_NUM_FRAMES - AFD_FRAME_SKIP); j++)
      afd->edge[j][i] = 0;

  for (i=0; i < 7; i++)
    for (j= 0; j < AFD_NUM_FRAMES - AFD_FRAME_SKIP; j++)
      afd->peak_width[j][i] = 0;

  for (i=0; i < AFD_NUM_FRAMES; i++)
    for (j= 0; j < afd->array_size; j++)
      afd->row_sum[i][j] = 0;

  afd->std_width = -1;
} /* afd_config */

/*===========================================================================
 * FUNCTION    - afd_detect_bands -
 *
 * DESCRIPTION: AFD entry routine
 *==========================================================================*/
static void afd_detect_bands(stats_proc_t *sproc, afd_t *afd)
{
  afd->flicker = 0;
  afd_multiple_peaks(sproc, afd);
  afd->flicker = afd_detect_flicker(sproc, afd);
  CDBG_HIGH("%s: flicker %d", __func__, afd->flicker);
} /* afd_detect_bands */

/*===========================================================================
 * FUNCTION    - afd_needed -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int afd_needed(stats_proc_t *sproc, afd_t *afd)
{
  int need_fd = 0;
  /* when there is flicker but none of the antibanding table worked
   * or flicker_freq = 0 means did not detect flicker
   * we need to re run AFD */
  if (afd->afd_done) {
    CDBG_AFD("%s: %d \n", __func__,need_fd);
    return need_fd;
  }
  if (afd->frame_index < afd->frame_ct_threshold) {
    afd->frame_index++;
    CDBG_AFD("%s: threshold afd_needed:%d \n", __func__,need_fd);
    return need_fd;
  }
  afd->frame_index = 0;   /* reset frame counter */
  /* AFD is required if sensor integration time is
   * longer than one flicker cycle and shorter than the threshold. */
  CDBG_AFD("frame_rate %d, max_preview_fps %d", sproc->input.sensor_info.
    preview_fps, sproc->input.sensor_info.max_preview_fps);
  if ((sproc->input.sensor_info.preview_fps >= sproc->input.sensor_info.
    max_preview_fps || (sproc->share.aec_ext.cur_line_cnt < (uint32_t)afd->
    line_count_threshold && sproc->input.sensor_info.preview_fps >=
    sproc->input.sensor_info.max_preview_fps / 2)) &&
    sproc->share.aec_ext.cur_line_cnt > sproc->share.aec_ext.band_50hz_gap) {
    need_fd = 1;
  }else{
    if(sproc->share.aec_ext.cur_line_cnt < sproc->share.aec_ext.band_50hz_gap)
      CDBG_AFD("Background is too bright to trigger AFD");
  }
  CDBG_AFD("%s: %d", __func__, need_fd);
  return need_fd;
} /* afd_needed */

/*===========================================================================
 * FUNCTION    - afd_algo_run -
 *
 * DESCRIPTION:
 *==========================================================================*/
int afd_algo_run(stats_proc_t *sproc, afd_t *afd)
{
  int line_cnt, cnt, cnt2, temp;
  uint32_t preview_fps,preview_linesPerFrame;
  float preview_exposuretime;
   CDBG_AFD("%s :E",__func__);
  if (afd->state ==  AFD_STATE_DONE) {
    CDBG_AFD("%s :DONE with AFD",__func__);
    return -1;
  }
  if(afd->state == AFD_STATE_INIT){
    CDBG_AFD("state is init");
    /* Ensure that AEC has settled */
    if (sproc->share.aec_ext.aec_settled == FALSE) {
      CDBG_AFD("%s: AEC is not settled. Retry... \n", __func__);
      return -1;
    } else {
      afd->state = AFD_STATE_ON;
      afd->conti.status = FALSE;
      CDBG_AFD("%s: AEC is settled. \n", __func__);
      afd_config(sproc, afd, 0);
      if (afd_needed(sproc, afd)) {
        CDBG_AFD("%s: Start AFD, with antibanding table, afd_status %d",
          __func__, sproc->share.afd_status);
      } else{
          CDBG_AFD("%s: AFD is not needed \n", __func__);
          return 0;
        }
    }
  }
  preview_fps = sproc->input.sensor_info.preview_fps;
  preview_linesPerFrame = sproc->input.sensor_info.preview_linesPerFrame;
  preview_exposuretime = sproc->share.aec_ext.cur_line_cnt /
    ((float)preview_fps / AFD_Q8 * preview_linesPerFrame);
 if (sproc->share.afd_monitor == 0){
   if ((afd->flicker_freq == 60) && (preview_exposuretime < (1/120.0))) {
       afd->frame_index = 0;
       CDBG_AFD("Ignore 60Hz AFD,Preview exposure time %f is less than 8ms",
         preview_exposuretime);
       return 0;
   }
  if ((afd->flicker_freq == 50) && (preview_exposuretime < (1/100))) {
    afd->frame_index = 0;
    CDBG_AFD("Ignore 50Hz AFD,Preview exposure time %f is less than 10ms",
      preview_exposuretime);
    return 0;
   }
 }
  CDBG_AFD("%s: afd->conti.status %d %d",
    __func__, afd->conti.status, afd->flicker_freq);
  if (!sproc->share.af_ext.active ||
    sproc->share.af_ext.cont_af_enabled) {
    if (afd->conti.status) {
      if (afd->flicker_freq == 60) {
        sproc->share.afd_status = AFD_60HZ_EXPOSURE_TABLE;
        sproc->share.afd_atb = CAMERA_ANTIBANDING_60HZ;
      } else if (afd->flicker_freq == 50) {
        sproc->share.afd_status = AFD_50HZ_EXPOSURE_TABLE;
        sproc->share.afd_atb = CAMERA_ANTIBANDING_50HZ;
      } else if (afd->flicker_freq == 0) {
        sproc->share.afd_atb = CAMERA_ANTIBANDING_OFF;
        sproc->share.afd_status = AFD_REGULAR_EXPOSURE_TABLE;
      }
      afd->frame_index = -3;
      afd->conti.status = FALSE;
    }

    CDBG_AFD("%s: afd_status = %d \n", __func__, sproc->share.afd_status);
    switch (sproc->share.afd_status) {
      case AFD_OFF: /* for the beginning of FD, we need AEC to be settled */
            sproc->share.afd_status = AFD_REGULAR_EXPOSURE_TABLE;
            afd->frame_index = -3;
            sproc->share.afd_atb = CAMERA_ANTIBANDING_OFF;
        break;
      case AFD_REGULAR_EXPOSURE_TABLE:
      case AFD_60HZ_EXPOSURE_TABLE:
      case AFD_50HZ_EXPOSURE_TABLE: {
          CDBG_AFD("%s: Collect Stats Data frame index %d",
            __func__, afd->frame_index);
          if (afd->frame_index < 0)/* skip frame after changing exp tbl */
            afd->frame_index++;
          else if (afd->frame_skip == 0 && afd->frame_index == 0)
            afd->frame_skip = 1;
          else if (afd->frame_index < AFD_NUM_FRAMES) {
            afd->frame_skip = 0;
            temp = SAMPLE_RATIO ;
            line_cnt = sproc->input.mctl_info.rgnVnum / temp;

#ifdef LOG_STATS
            static int frame_count = 0;
            char file_name[128];
            sprintf(file_name, "/data/RS_%d.log", frame_count);
            FILE *file_fd = NULL;
            file_fd = fopen(file_name, "wt");
            frame_count++;
#endif

            for (cnt = 0; cnt < line_cnt; ++cnt) {
              afd->row_sum[afd->frame_index][cnt] = sproc->input.mctl_info.
                vfe_stats_out->vfe_stats_struct.rs_op.row_sum[(int)temp * cnt];
#ifdef LOG_STATS
              fprintf(file_fd, "%d\n",
                afd->row_sum[afd->frame_index][cnt]);
#endif
            }

#ifdef LOG_STATS
            fclose(file_fd);
#endif
            temp = 0;
            if (afd->frame_index > 0)
              for (cnt2 = 0; cnt2 < line_cnt; ++cnt2)
                if (afd->row_sum [afd->frame_index][cnt] > afd->row_sum 
                  [afd->frame_index - AFD_FRAME_SKIP][cnt])
                  ++temp;

            if (temp != (line_cnt + 1))
              afd->frame_index++;

          } else { /* need to analyze data */
            /* this will reset frame counter, 0 means not to reset status */
            afd->frame_index = AFD_NUM_FRAMES;
            afd_detect_bands(sproc, afd);
            if (afd->flicker) {
              if(sproc->share.afd_exec_once == 1){
                CDBG_AFD("Retry once for false positive");
                afd_config(sproc, afd, 0);
                afd->conti_afd_delay = CAFD_DELAY;
                sproc->share.afd_exec_once = 0;
                afd->frame_index = -3;
                break;
              }
              sproc->share.afd_monitor = 1;
              if (sproc->share.afd_status == AFD_REGULAR_EXPOSURE_TABLE) {
                /* 1 - just analyzed first N frames of data using the
                 * regular exposure table. load antibanding table for 60hz */
                sproc->share.afd_status = AFD_60HZ_EXPOSURE_TABLE;
                sproc->share.afd_atb = CAMERA_ANTIBANDING_60HZ;
                afd->frame_index = -3;
                CDBG_HIGH("%s: std_width %d, load 60hz table afd_status %d",
                  __func__, afd->std_width, sproc->share.afd_status);
              } else if (sproc->share.afd_status == AFD_60HZ_EXPOSURE_TABLE) {
                /* 2 means analysed data using 60 hz antibanding table
                           * load antibanding table for 50hz */
                sproc->share.afd_status = AFD_50HZ_EXPOSURE_TABLE;
                sproc->share.afd_atb = CAMERA_ANTIBANDING_50HZ;
                afd->frame_index = -3;
                CDBG_HIGH("%s: std_width=%d, load 50hz banding table type %d",
                  __func__, afd->std_width, sproc->share.afd_status);
              } else if (sproc->share.afd_status == AFD_50HZ_EXPOSURE_TABLE) {
                /* (frameCtrl->output.afd_d.afd_status == 3 )
                 * 3 means analysed data using 50 hz antibanding table,
                 * at this point, both table are used with no effect
                 * load regular exposure table.
                 * this means some of the assumption are not satisfied,
                 * re-run immediately if the next AFD session is far in time. */
                sproc->share.afd_status = AFD_OFF;
                afd->frame_index = -3;
                CDBG_HIGH("%s: std_width=%d, re-run AFD, afd type %d",
                  __func__, afd->std_width, sproc->share.afd_status);
                afd->afd_done = 1;
                afd->state = AFD_STATE_DONE;
                afd->conti.status = TRUE;
                afd->conti_afd_delay = CAFD_DELAY/2;
                afd->flicker_freq = 0;
                CDBG_AFD("%s: afd is DONE.\n", __func__);
              }
            } else { /* no flicker, detected light freq */
              if (sproc->share.afd_status == AFD_60HZ_EXPOSURE_TABLE)
                afd->flicker_freq = 60;
              else if (sproc->share.afd_status == AFD_50HZ_EXPOSURE_TABLE)
                afd->flicker_freq = 50;
              else if (sproc->share.afd_status == AFD_REGULAR_EXPOSURE_TABLE)
                afd->flicker_freq = 0; /* no flicker */

              afd->frame_index = 0;
              CDBG_HIGH("%s: AFD success, std_width %d, flicker_freq %d",
                __func__, afd->std_width, afd->flicker_freq);
              /* Send FRAME_PROC_PREV DONE to kernel */
              afd->afd_done = 1;
              afd->state = AFD_STATE_DONE;
              afd->conti.status = TRUE;
              sproc->share.afd_exec_once = 1;
              sproc->share.afd_monitor = 0;
              afd->conti_afd_delay = CAFD_DELAY;
              CDBG_AFD("%s: afd is DONE.", __func__);
            }
            /* we reset everything after each afd_detect_bands */
            afd_config(sproc, afd, 0);
          } /* end else need to analyze data */
          if (!afd->afd_done)
            afd->state = AFD_STATE_ON;
        }
        break;
      default:
        break;
    } /* switch */
  } else { /* reset AFD process */
    afd->frame_index = -3;
    CDBG_HIGH("%s: things changed during AFD, reset AFD type %d",
      __func__, sproc->share.afd_status);
  }
  return 0;
} /* afd_algo_run */
