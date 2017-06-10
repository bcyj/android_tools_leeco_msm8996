/* af_fdprio.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "af_port.h"
#include "af_fdprio.h"
#include "math.h"

#if 0
#undef CDBG
#define CDBG CDBG_ERROR
#endif

#define AF_FDPRIO_INDEX_INVALID         0xFFFF

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a)  (((a) > 0) ? (a) : -(a))
#endif


#define PERCENT_CHANGE(a, b) \
    (((float)MAX(a, b)/(float)MAX(MIN(a, b),1))*100.0-100.0)

#define PERCENT_LOC_CHANGE(a, b, c) (((float)ABS(a - b) / MAX(c,1)) * 100)

#define AF_FDPRIO_NO_FACE_COUNT_MAX        0xFF

/********************************************
     FACE DETECT PRIORITY ALGORYTHM API
********************************************/

/* Internal helper functions */
static void    af_fdprio_init(af_fdprio_t *af_fdprio_data);
static boolean af_fdprio_process_counters(af_fdprio_t *af_fdprio_data);
static boolean af_fdprio_process_counters_int(
  af_fdprio_t *af_fdprio_data);
static boolean af_fdprio_process_fd_roi(af_fdprio_t *af_fdprio_data);
static int32_t af_fdprio_find_biggest_face(mct_face_info_t *face_info);
static boolean af_fdprio_is_tracked_face_present(
  af_fdprio_t *af_fdprio_data);
static boolean af_fdprio_send_default_roi(af_fdprio_t *af_fdprio_data);
static int32_t af_fdprio_get_index_by_id(af_fdprio_t *af_fdprio_data,
  int32_t face_id);
static boolean af_fdprio_is_new_face_big_enough(
  af_fdprio_t *af_fdprio_data,
  int32_t curr_biggest_face_id);
static boolean af_fdprio_send_current_roi(af_fdprio_t *af_fdprio_data);
static void    af_fdprio_begin_new_history(af_fdprio_t *af_fdprio_data);
static void af_fdprio_update_face_info(
  af_fdprio_face_info_t *af_fdprio_data,
  mct_face_data_t *face_info,
  int32_t loc_stability_cnt,
  int32_t size_stability_cnt);
static boolean af_fdprio_check_loc_stability(
  af_fdprio_t *af_fdprio_data,
  af_fdprio_face_info_t *this_face,
  int32_t new_ctr_x,
  int32_t new_ctr_y);
static boolean af_fdprio_check_size_stability(
  af_fdprio_t *af_fdprio_data,
  af_fdprio_face_info_t *this_face,
  int32_t new_roi_top_left_x,
  int32_t new_roi_top_left_y);
static void af_fdprio_check_face_stability(
  af_fdprio_t *af_fdprio_data,
  af_fdprio_face_info_t *this_face,
  mct_face_data_t *cur_update,
  boolean *is_loc_stable,
  boolean *is_size_stable);
static boolean af_fdprio_compare_with_ref_roi(
  af_fdprio_t *af_fdprio_data,
  cam_rect_t *ref_roi,
  cam_rect_t *cur_roi,
  int32_t frame_width);

static void af_fdprio_dbg_print_face_dim(af_fdprio_t *af_fdprio_data);


/** af_fdprio_adjust_stability_count:
 * adjust stability count based on the threshold.
 *
 *  @af: internal AF data structure
 *
 **/
int af_fdprio_adjust_stability_count(af_fdprio_t *af_fdprio_data,
  int input_stability_count)
{
  af_fd_priority_caf_t *fd_tuning =
    &af_fdprio_data->tuning_info->af_algo.fd_prio;
  int adjusted_count = input_stability_count;
  CDBG("%s: Preview-fps: %d Stability count: %d th: %d", __func__,
    af_fdprio_data->preview_fps, input_stability_count,
    fd_tuning->fps_adjustment_th);

  if (af_fdprio_data->preview_fps <= fd_tuning->fps_adjustment_th) {
    adjusted_count /= 2;
    if (af_fdprio_data->preview_fps < (fd_tuning->fps_adjustment_th / 2)) {
      adjusted_count /= 2;
    }
  }

  if (adjusted_count < 1) {
    adjusted_count = 1;
  }

  CDBG("%s: stability_count after adjustment: %d", __func__, adjusted_count);

  return adjusted_count;
} /* af_fdprio_adjust_stability_count */

/** af_fdprio_init
 * Initialize the internal data for the FD priority AF feature
 *
 * @af_fdprio_data: internal data to control the FD priority AF feature
 *
 **/
static void af_fdprio_init(af_fdprio_t *af_fdprio_data)
{
  CDBG("%s:%d Init", __func__, __LINE__);
  memset(af_fdprio_data, 0, sizeof(af_fdprio_t));
  /* We need to initialize noface_cnt to threshold so that we don't
     trigger unnecessary "reset to default" search in the beginning.
     We need to reset it only we detect first face */
  af_fdprio_data->noface_cnt = AF_FDPRIO_NO_FACE_COUNT_MAX;
}

static boolean af_fdprio_process_counters(af_fdprio_t *af_fdprio_data)
{
  boolean rc;

  if(af_fdprio_data->last_processed_frame_id ==
      af_fdprio_data->current_frame_id) {
    CDBG("%s:%d Already processed, returning...", __func__, __LINE__);
    return TRUE;
  }

  rc = af_fdprio_process_counters_int(af_fdprio_data);
  return rc;
}

static boolean af_fdprio_process_counters_int(af_fdprio_t *af_fdprio_data)
{
  af_fd_priority_caf_t *fd_tuning =
    &af_fdprio_data->tuning_info->af_algo.fd_prio;

  CDBG("%s:%d Process counters", __func__, __LINE__);

  if(af_fdprio_data->faces_detected) {
    af_fdprio_data->noface_cnt = 0;
  } else {
    /* increment only once above threshold */
    if(af_fdprio_data->noface_cnt <= fd_tuning->no_face_wait_th) {
      af_fdprio_data->noface_cnt++;
      CDBG("%s:%d No face counter: %d", __func__, __LINE__,
        af_fdprio_data->noface_cnt);
      /* check if if the threshold is reached */
      if(af_fdprio_data->noface_cnt > fd_tuning->no_face_wait_th) {
        CDBG("%s:%d Trigger default ROI AF_START", __func__, __LINE__);
        af_fdprio_send_default_roi(af_fdprio_data);
      }
    }
  }

  af_fdprio_data->last_processed_frame_id =
    af_fdprio_data->current_frame_id;


  return TRUE;
} /* af_port_process_fdaf_counters */


static void af_fdprio_dbg_print_face_dim(af_fdprio_t *af_fdprio_data)
{
  uint32_t i;
  mct_face_info_t *face_info;

  face_info = af_fdprio_data->pface_info;

  CDBG("%s: Face info (count: %d): ", __func__, face_info->face_count);
  for(i = 0; i < face_info->face_count; i++) {
    CDBG("%s: Face-id: %d ROI: x: %d y: %d dx: %d dy: %d", __func__,
      face_info->faces[i].face_id, face_info->faces[i].roi.left,
      face_info->faces[i].roi.top, face_info->faces[i].roi.width,
      face_info->faces[i].roi.height);
  }
}

static void af_fdprio_update_face_info(
  af_fdprio_face_info_t *af_fdprio_data,
  mct_face_data_t *face_info,
  int32_t loc_stability_cnt,
  int32_t size_stability_cnt)
{
  af_fdprio_data->is_valid = TRUE;
  af_fdprio_data->face_id = face_info->face_id;
  af_fdprio_data->ctr_x = face_info->roi.left + face_info->roi.width / 2;
  af_fdprio_data->ctr_y = face_info->roi.top + face_info->roi.height / 2;
  af_fdprio_data->top_left_x = face_info->roi.left;
  af_fdprio_data->top_left_y = face_info->roi.top;
  af_fdprio_data->loc_stability_cnt = loc_stability_cnt;
  af_fdprio_data->size_stability_cnt = size_stability_cnt;
  CDBG("%s: face-id: %d Face-center (%d, %d) top_left_coord: (%d,%d)", __func__,
    af_fdprio_data->face_id, af_fdprio_data->ctr_x, af_fdprio_data->ctr_y,
    af_fdprio_data->top_left_x, af_fdprio_data->top_left_y);
  CDBG("%s: Stability Count - Location: %d Size: %d", __func__,
    af_fdprio_data->loc_stability_cnt, af_fdprio_data->size_stability_cnt);
} /* af_fdprio_update_face_info */

static boolean af_fdprio_compare_with_ref_roi(
  af_fdprio_t *af_fdprio_data,
  cam_rect_t *ref_roi,
  cam_rect_t *cur_roi,
  int32_t frame_width)
{
  boolean roi_x_stable = FALSE, roi_y_stable = TRUE;
  float percent_change = 0.0;
  int32_t ref_ctr_x = ref_roi->left + ref_roi->width / 2;
  int32_t ref_ctr_y = ref_roi->top + ref_roi->height/ 2;
  int32_t cur_ctr_x = cur_roi->left + cur_roi->width / 2;
  int32_t cur_ctr_y = cur_roi->top + cur_roi->height / 2;
  af_fd_priority_caf_t *fd_tuning =
    &af_fdprio_data->tuning_info->af_algo.fd_prio;

  CDBG("%s: Input - Ref ROI x:%d y:%d dx:%d dy:%d"
    "Cur ROI x:%d y:%d dx:%d dy:%d frame-width: %d",
    __func__, ref_roi->left, ref_roi->top, ref_roi->width, ref_roi->height,
    cur_roi->left, cur_roi->top, cur_roi->width, cur_roi->height,
    frame_width);

  /* Check difference in x-axis */
  percent_change = PERCENT_LOC_CHANGE(ref_ctr_x, cur_ctr_x, frame_width);
  CDBG("%s: Percent X-Axis difference: %f Th: %f",
    __func__, percent_change, fd_tuning->pos_change_th);
  roi_x_stable =
    (percent_change > fd_tuning->pos_change_th) ? FALSE : TRUE;

  /* Check difference in Y-axis */
  /* Note: use frame_width here too as it remains fixed for different resolution (4:3, 16:9) */
  percent_change = PERCENT_LOC_CHANGE(ref_ctr_y, cur_ctr_y, frame_width);
  CDBG("%s: Percent Y-Axis difference: %f Threshold: %f", __func__,
    percent_change, fd_tuning->pos_change_th);
  roi_y_stable =
    (percent_change > fd_tuning->pos_change_th) ? FALSE : TRUE;

  return (roi_x_stable & roi_y_stable);
} /* af_fdprio_check_loc_stability */


static boolean af_fdprio_check_loc_stability(
  af_fdprio_t *af_fdprio_data,
  af_fdprio_face_info_t *this_face,
  int32_t new_ctr_x,
  int32_t new_ctr_y)
{
  boolean loc_x_stable = FALSE, loc_y_stable = FALSE;
  float percent_change = 0.0;
  int32_t frame_width = af_fdprio_data->camif_width;
  int32_t last_ctr_x = this_face->ctr_x;
  int32_t last_ctr_y = this_face->ctr_y;
  af_fd_priority_caf_t *fd_tuning =
    &af_fdprio_data->tuning_info->af_algo.fd_prio;
  int stable_count_pos = fd_tuning->stable_count_pos;

  /* Adjust stability count threshold based on current fps */
  stable_count_pos = af_fdprio_adjust_stability_count(af_fdprio_data,
    fd_tuning->stable_count_pos);
  CDBG("%s: Input - Last Ctr (%d, %d) Cur Ctr (%d, %d) frame-width: %d",
    __func__, last_ctr_x, last_ctr_y,
    new_ctr_x, new_ctr_y, frame_width);

  /* Check location stability in x-axis */
  percent_change = PERCENT_LOC_CHANGE(new_ctr_x, last_ctr_x, frame_width);
  CDBG("%s: Percent Loc Change in X-axis: %f Pos-Stable-Th Hi: %f Low: %f",
    __func__, percent_change, fd_tuning->pos_stable_th_hi,
    fd_tuning->pos_stable_th_low);
  /* this percent_change between position of last and current ROI of the face
     * determine whether it is still moving or stable.
     * However we will use two thresholds - THRESHLD_HI and THRESHOLD_LOW.
     * THRESHLD_HI is checked to detect motion. If percent_change is more than
     * this threshold we'll indicate that face is moving.
     * THRESHLD_LO is checked while slowing down. Only after percent_change is
     * lower than this threshold we will assume face is stable.
     * Using two thresholds give better stability and flexibility for tuning */

  if (percent_change > fd_tuning->pos_stable_th_hi) {
    loc_x_stable = FALSE;
  } else if (percent_change < fd_tuning->pos_stable_th_low) {
    loc_x_stable = TRUE;
  } else {
    /* If percent_change is within LO and HI threshold stability is kept as before */
    loc_x_stable =
      (this_face->loc_stability_cnt < stable_count_pos) ? FALSE : TRUE;
  }

  /* Check location stability in Y-axis */
  /* Note: use frame_width here too as it remains fixed for different resolution (4:3, 16:9) */
  percent_change = PERCENT_LOC_CHANGE(new_ctr_y, last_ctr_y, frame_width);
  CDBG("%s: Percent Loc Change in Y-axis: %f Pos-Stable-Th Hi: %f Low: %f",
    __func__, percent_change, fd_tuning->pos_stable_th_hi,
    fd_tuning->pos_stable_th_low);
  if (percent_change > fd_tuning->pos_stable_th_hi) {
    loc_y_stable = FALSE;
  } else if (percent_change < fd_tuning->pos_stable_th_low) {
    loc_y_stable = TRUE;
  } else {
    loc_y_stable =
      (this_face->loc_stability_cnt < stable_count_pos) ? FALSE : TRUE;
  }

  return (loc_x_stable & loc_y_stable);
} /* af_fdprio_check_loc_stability */


static boolean af_fdprio_check_size_stability(
  af_fdprio_t *af_fdprio_data,
  af_fdprio_face_info_t *this_face,
  int32_t new_roi_top_left_x,
  int32_t new_roi_top_left_y)
{
  boolean rc = FALSE;
  float percent_change = 0.0;
  int32_t delta_x = 0, delta_y = 0;
  int32_t new_size_delta = 0;
  af_fd_priority_caf_t *fd_tuning =
    &af_fdprio_data->tuning_info->af_algo.fd_prio;

  CDBG("%s: Top-Left coordinate - Ref ROI: (%d, %d) New ROI: (%d, %d)",
    __func__, this_face->top_left_x, this_face->top_left_y,
    new_roi_top_left_x, new_roi_top_left_y);

  delta_x = new_roi_top_left_x - this_face->top_left_x;
  delta_y = new_roi_top_left_y - this_face->top_left_y;
  new_size_delta = (uint32_t) sqrt(delta_x * delta_x + delta_y * delta_y);

  percent_change = ((float)new_size_delta / af_fdprio_data->camif_width) * 100;
  CDBG("%s: Percent Size Change : %f Threshold: %f", __func__,
    percent_change, fd_tuning->size_change_th);
  rc = (percent_change > fd_tuning->size_change_th) ? FALSE : TRUE;

  return rc;
} /* af_fdprio_check_loc_stability */

static void af_fdprio_check_face_stability(
  af_fdprio_t *af_fdprio_data,
  af_fdprio_face_info_t *this_face,
  mct_face_data_t *cur_update,
  boolean *is_loc_stable,
  boolean *is_size_stable)
{
  int32_t new_roi_top_left_x = 0, new_roi_top_left_y;
  int32_t new_ctr_x = 0, new_ctr_y = 0;
  boolean loc_stability = FALSE, size_stability = FALSE;
  cam_rect_t *new_roi = &cur_update->roi;

  new_ctr_x = new_roi->left + new_roi->width / 2;
  new_ctr_y = new_roi->top + new_roi->height/ 2;

  new_roi_top_left_x = new_roi->left;
  new_roi_top_left_y = new_roi->top;


  /* Location stability check:
     * This is to detect the movement of face in horizontal and vertical plane.
     * For location stability we'll check by how much center of current face changes
     * from previous face in x-axis and y-axis */
  loc_stability = af_fdprio_check_loc_stability(
    af_fdprio_data, this_face, new_ctr_x, new_ctr_y);

  /* Size stability check:
     * This is to detect the movement towards and away from the camera (z-axis)
     * For size stability, we'll calculate a simple delta of top-left coordinates of current
     * and previous faces. Then percentage change of this delta will be compared with
     * our threshold to determine size stability. */
  size_stability = af_fdprio_check_size_stability(
    af_fdprio_data, this_face, new_roi_top_left_x, new_roi_top_left_y);

  CDBG("%s: Stability check: Location: %d Size: %d", __func__,
    loc_stability, size_stability);
  /* Params that need to be output */
  *is_loc_stable = loc_stability;
  *is_size_stable = size_stability;
}


static boolean af_fdprio_process_fd_roi(af_fdprio_t *af_fdprio_data) {
  boolean rc = FALSE;
  mct_face_info_t *face_info;
  int32_t curr_biggest_face_id = -1;
  int32_t curr_biggest_face_index = 0;
  int32_t active_face_index = 0;
  boolean loc_stability = FALSE, size_stability = FALSE;
  uint32_t loc_stability_cnt = 0, size_stability_cnt = 0;
  boolean roi_stability = FALSE;
  int32_t face_id_ret = 0;
  af_fd_priority_caf_t *fd_tuning =
    &af_fdprio_data->tuning_info->af_algo.fd_prio;
  int stable_count_pos = fd_tuning->stable_count_pos;
  int stable_count_size = fd_tuning->stable_count_size;

  /* Adjust stability count threshold based on current fps */
  stable_count_pos = af_fdprio_adjust_stability_count(af_fdprio_data,
    fd_tuning->stable_count_pos);
  stable_count_size = af_fdprio_adjust_stability_count(af_fdprio_data,
    fd_tuning->stable_count_size);

  CDBG("%s:%d Process FD_ROI data", __func__, __LINE__);

  if(!af_fdprio_data->pface_info) {
    CDBG_ERROR("%s: Null pointer passed for face_info", __func__);
    return FALSE;
  }

  face_info = af_fdprio_data->pface_info;
  af_fdprio_data->faces_detected = face_info->face_count;
  CDBG("%s: Number of faces detected: %d", __func__,
    af_fdprio_data->faces_detected);


  af_fdprio_dbg_print_face_dim(af_fdprio_data);

  /* Check if faces are still detected. If no faces are detected for
       number of frames, we'll revert back to default ROI */
  af_fdprio_process_counters_int(af_fdprio_data);

  if(!af_fdprio_data->faces_detected) {
    memset(&af_fdprio_data->active_face, 0, sizeof(af_fdprio_face_info_t));
    CDBG("%s: Face_count: 0", __func__);
    return TRUE;
  }

  /* Find the biggest face */
  curr_biggest_face_id = af_fdprio_find_biggest_face(face_info);
  CDBG("%s: Biggest Face-id: %d", __func__, curr_biggest_face_id);

  curr_biggest_face_index = af_fdprio_get_index_by_id(af_fdprio_data,
    curr_biggest_face_id);

  /* faces[] array is of size MAX_ROI */
  if (curr_biggest_face_index == AF_FDPRIO_INDEX_INVALID || curr_biggest_face_index >= MAX_ROI) {
    curr_biggest_face_index = 0;
  }
  CDBG("%s: Index of current biggest face: %d", __func__,
    curr_biggest_face_index);
  rc = af_fdprio_is_new_face_big_enough(af_fdprio_data,
    curr_biggest_face_id);
  face_id_ret = af_fdprio_get_index_by_id(af_fdprio_data,
    af_fdprio_data->active_face.face_id);

  if (((af_fdprio_data->active_face.face_id !=  curr_biggest_face_id) && (rc))
     ||
    ((AF_FDPRIO_INDEX_INVALID == face_id_ret) && (af_fdprio_data->active_face.face_id != 0))){

    /* Either this is a new face detected for the first time, OR A "larger" face is detected then
        * the previously tracked active face. */
    CDBG("%s:Active face is not big enough. Or this is the only face."
      "Active face id:%d, curr_biggest_face_id: %d",
      __func__, af_fdprio_data->active_face.face_id, curr_biggest_face_id);

    af_fdprio_update_face_info(&af_fdprio_data->active_face,
      &face_info->faces[curr_biggest_face_index], 0, 0);

    CDBG("%s: Set trigger_search for New Active Face", __func__);
    af_fdprio_data->trigger_search = TRUE;
    loc_stability                  = FALSE;
    size_stability                 = FALSE;
    active_face_index              = curr_biggest_face_index;
  } else {
    /* Check face stability and increase the count to compare to tuned value.*/
    CDBG("%s: Check stability of active face (id: %d)", __func__,
      af_fdprio_data->active_face.face_id);
    active_face_index = af_fdprio_get_index_by_id(af_fdprio_data,
      af_fdprio_data->active_face.face_id);

    if (active_face_index == AF_FDPRIO_INDEX_INVALID || active_face_index >= MAX_ROI) {
      active_face_index = 0;
    }
    af_fdprio_check_face_stability(
      af_fdprio_data,
      &af_fdprio_data->active_face,
      &face_info->faces[active_face_index],
      &loc_stability,
      &size_stability);

    if (loc_stability == TRUE) {
      af_fdprio_data->active_face.loc_stability_cnt++;
    }
    if (size_stability == TRUE) {
      af_fdprio_data->active_face.size_stability_cnt++;
    }
    /* update the face struct with active face data in current frame. This is needed to compare
        * the face information of next frame with the current frame face info */
    af_fdprio_update_face_info(&af_fdprio_data->active_face,
      &face_info->faces[active_face_index],
      af_fdprio_data->active_face.loc_stability_cnt,
      af_fdprio_data->active_face.size_stability_cnt);
  }

  /* Also we need to check if current face has moved significantly away from the
     * last configured ROI. Sometime if face moves very slowly we may not detect
     * it has moved and assume it is stable. However after sometime face ROI might
     * have changed significantly even without triggering instability */
  if ((loc_stability == TRUE) &&
    (af_fdprio_data->trigger_search == FALSE)) {
    roi_stability = af_fdprio_compare_with_ref_roi(
      af_fdprio_data,
      &af_fdprio_data->cur_confgd_roi,
      &face_info->faces[active_face_index].roi,
      af_fdprio_data->camif_width);
    if (roi_stability == FALSE) {
      CDBG("%s: Set trigger_search for Diff between Ref and Current ROI!",
        __func__);
      af_fdprio_data->trigger_search = TRUE;
      loc_stability_cnt = 0;
    }
  }

  /* Now check both location and size stability to determine whether we need to refocus
     * or not */
  if ((af_fdprio_data->trigger_search == TRUE) &&
    (af_fdprio_data->active_face.loc_stability_cnt >
    stable_count_pos) &&
    (af_fdprio_data->active_face.size_stability_cnt >
    stable_count_size)) {
    CDBG("%s: We have stable ROI! Send it to AF library!", __func__);
    memcpy(&af_fdprio_data->cur_confgd_roi,
      &face_info->faces[active_face_index].roi,
      sizeof(cam_rect_t));
    af_fdprio_send_current_roi(af_fdprio_data);
    af_fdprio_data->trigger_search = FALSE;
  }
  return TRUE;
}

boolean af_fdprio_process(
  af_fdprio_t *af_fdprio_data,
  af_fdprio_cmd_t cmd)
{
  boolean rc = FALSE;

  if (!af_fdprio_data) {
    CDBG_ERROR("%s: Null pointer passed for af_fdprio_data", __func__);
    return FALSE;
  }

  CDBG("%s: Processing frame %d", __func__, af_fdprio_data->current_frame_id);

  switch(cmd)
  {
  case AF_FDPRIO_CMD_INIT:
    af_fdprio_init(af_fdprio_data);
    rc = TRUE;
    break;
  case AF_FDPRIO_CMD_PROC_COUNTERS:
    rc = af_fdprio_process_counters(af_fdprio_data);
    break;
  case AF_FDPRIO_CMD_PROC_FD_ROI:
    rc = af_fdprio_process_fd_roi(af_fdprio_data);
    break;
  default:
    break;
  }

  return rc;
}

static int32_t af_fdprio_find_biggest_face(mct_face_info_t *face_info) {
  int32_t biggest_face = 0;
  cam_rect_t *biggest_fd_roi;
  int32_t i;

  biggest_fd_roi = &face_info->faces[0].roi;
  biggest_face = face_info->faces[0].face_id;

  for(i = 1; i < face_info->face_count; i++) {
    if(biggest_fd_roi->width < face_info->faces[i].roi.width ||
        biggest_fd_roi->height < face_info->faces[i].roi.height) {
      biggest_fd_roi = &face_info->faces[i].roi;
      biggest_face = face_info->faces[i].face_id;
    }
  }
  CDBG("%s: Biggest Face: ID: %d x: %d y: %d dx: %d dy: %d", __func__,
    biggest_face, biggest_fd_roi->left, biggest_fd_roi->top,
    biggest_fd_roi->width, biggest_fd_roi->height);

  return biggest_face;
}

static boolean af_fdprio_is_tracked_face_present(
  af_fdprio_t *af_fdprio_data)
{
  int32_t i;
  mct_face_info_t *face_info;

  face_info = af_fdprio_data->pface_info;

  for(i = 0; i < face_info->face_count; i++) {
    /* compare with the curr_biggest_face_id field because this is the id */
    /* we have been tracking so far */
    if(face_info->faces[i].face_id == af_fdprio_data->active_face.face_id) {
      return TRUE;
    }
  }

  return FALSE;
}

static int32_t af_fdprio_get_index_by_id(af_fdprio_t *af_fdprio_data,
  int32_t face_id) {
  int32_t i;
  mct_face_info_t *face_info;
  int32_t index = AF_FDPRIO_INDEX_INVALID;
  face_info = af_fdprio_data->pface_info;

  for(i = 0; i < face_info->face_count; i++) {
    if(face_info->faces[i].face_id == face_id) {
      index = i;
      break;
    }
  }

  CDBG("%s: For Face-id: %d Index: %d", __func__, face_id, index);
  return index;
}

static boolean af_fdprio_is_new_face_big_enough(
  af_fdprio_t *af_fdprio_data,
  int32_t curr_biggest_face_id)
{
  boolean rc = TRUE;
  int32_t old_face_idx;
  int32_t new_face_idx;
  mct_face_info_t *face_info;
  int32_t old_face_size = 0, new_face_size = 0;
  cam_rect_t *old_face = NULL;
  cam_rect_t *new_face = NULL;
  float percent_change = 0.0;
  af_fd_priority_caf_t *fd_tuning =
    &af_fdprio_data->tuning_info->af_algo.fd_prio;

  face_info = af_fdprio_data->pface_info;

  old_face_idx = af_fdprio_get_index_by_id(af_fdprio_data,
    af_fdprio_data->active_face.face_id);
  new_face_idx = af_fdprio_get_index_by_id(af_fdprio_data,
    curr_biggest_face_id);

  if(old_face_idx == AF_FDPRIO_INDEX_INVALID || old_face_idx >= MAX_ROI)
    return TRUE;
  if (new_face_idx == AF_FDPRIO_INDEX_INVALID || new_face_idx >= MAX_ROI) {
    return FALSE;
  }

  old_face = &face_info->faces[old_face_idx].roi;
  new_face = &face_info->faces[new_face_idx].roi;

  old_face_size = sqrt((old_face->width * old_face->width) +
    (old_face->height * old_face->height)) / 2;

  new_face_size = sqrt((new_face->width * new_face->width) +
    (new_face->height * new_face->height)) / 2;

  percent_change = PERCENT_LOC_CHANGE(old_face_size, new_face_size,
    af_fdprio_data->camif_width);

  CDBG("%s: Face Size: Old: %d New: %d Th: %f Percent_change: %f", __func__,
    old_face_size, new_face_size, fd_tuning->old_new_size_diff_th,
    percent_change);
  rc = (percent_change > fd_tuning->old_new_size_diff_th) ? TRUE : FALSE;

  return rc;
}


static boolean af_fdprio_send_default_roi(af_fdprio_t *af_fdprio_data) {
  boolean rc = FALSE;
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg == NULL) {
    return FALSE;
  }
  memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));

  af_msg->u.af_set_parm.u.af_roi_info.roi_updated = TRUE;
  af_msg->u.af_set_parm.u.af_roi_info.frm_id = af_fdprio_data->current_frame_id;
  af_msg->u.af_set_parm.u.af_roi_info.num_roi = 0;
  af_msg->u.af_set_parm.u.af_roi_info.type = AF_ROI_TYPE_GENERAL;

  af_msg->u.af_set_parm.u.af_roi_info.roi[0].x = 0;
  af_msg->u.af_set_parm.u.af_roi_info.roi[0].y = 0;
  af_msg->u.af_set_parm.u.af_roi_info.roi[0].dx = 0;
  af_msg->u.af_set_parm.u.af_roi_info.roi[0].dy = 0;

  af_fdprio_data->cur_confgd_roi.left = 0;
  af_fdprio_data->cur_confgd_roi.top = 0;
  af_fdprio_data->cur_confgd_roi.width = 0;
  af_fdprio_data->cur_confgd_roi.height = 0;

  /* Clear our active face info */
  memset(&af_fdprio_data->active_face, 0, sizeof(af_fdprio_face_info_t));
  af_msg->type = MSG_AF_SET;
  af_msg->u.af_set_parm.type = AF_SET_PARAM_ROI;

  CDBG("%s: Sending default ROI", __func__);
  rc = q3a_af_thread_en_q_msg(af_fdprio_data->thread_data, af_msg);

  return rc;
}

static boolean af_fdprio_send_current_roi(af_fdprio_t *af_fdprio_data) {
  boolean rc = FALSE;
  uint8_t roi_pos;
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg == NULL) {
    return FALSE;
  }
  memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));

  af_msg->u.af_set_parm.u.af_roi_info.roi_updated = TRUE;
  af_msg->u.af_set_parm.u.af_roi_info.frm_id = af_fdprio_data->current_frame_id;
  af_msg->u.af_set_parm.u.af_roi_info.num_roi = 1;
  af_msg->u.af_set_parm.u.af_roi_info.type = AF_ROI_TYPE_FACE;

  af_msg->u.af_set_parm.u.af_roi_info.roi[0].x =
    af_fdprio_data->cur_confgd_roi.left;
  af_msg->u.af_set_parm.u.af_roi_info.roi[0].y =
    af_fdprio_data->cur_confgd_roi.top;
  af_msg->u.af_set_parm.u.af_roi_info.roi[0].dx =
    af_fdprio_data->cur_confgd_roi.width;
  af_msg->u.af_set_parm.u.af_roi_info.roi[0].dy =
    af_fdprio_data->cur_confgd_roi.height;

  af_msg->type = MSG_AF_SET;
  af_msg->u.af_set_parm.type = AF_SET_PARAM_ROI;

  CDBG("%s: Sending New Face ROI: x: %d y: %d dx: %d dy: %d", __func__,
    af_fdprio_data->cur_confgd_roi.left, af_fdprio_data->cur_confgd_roi.top,
    af_fdprio_data->cur_confgd_roi.width,
    af_fdprio_data->cur_confgd_roi.height);
  rc = q3a_af_thread_en_q_msg(af_fdprio_data->thread_data, af_msg);

  return rc;
}
