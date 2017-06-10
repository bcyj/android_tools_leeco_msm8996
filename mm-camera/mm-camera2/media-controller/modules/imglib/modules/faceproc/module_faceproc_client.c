/***************************************************************************
* Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/

#include <cutils/properties.h>
#include <linux/media.h>
#include "mct_module.h"
#include "module_faceproc.h"
#include "mct_stream.h"
#include "modules.h"
#include "fd_chromatix.h"
#include <sys/syscall.h>
#include <sys/prctl.h>

//#define FD_RESULT_DEBUG
//#define FD_USE_PROP

#define IMG_NUM_ZOOM_ENTRIES 79
#define IMG_NUM_ZOOM_MAX_ENTRIES 182

#define IMG_IS_FACE_SCALING_NEEDED(t) \
  (((t).h_scale > 1.0) || ((t).v_scale > 1.0) || \
   ((t).h_offset > 0) || ((t).v_offset > 0))

#define SWAP(T,x,y){T *p = &(x), *q = &(y); \
  T z = *p; *p = *q; *q = z; }
/** g_zoom_table_def
 *
 *  Zoom table in Q12 format
 **/
static const uint32_t g_zoom_table_def[IMG_NUM_ZOOM_MAX_ENTRIES] = {
  4096, 4191, 4289, 4389, 4492,
  4597, 4705, 4815, 4927, 5042,
  5160, 5281, 5404, 5531, 5660,
  5792, 5928, 6066, 6208, 6353,
  6501, 6653, 6809, 6968, 7131,
  7298, 7468, 7643, 7822, 8004,
  8192, 8383, 8579, 8779, 8985,
  9195, 9410, 9630, 9855, 10085,
  10321, 10562, 10809, 11062, 11320,
  11585, 11856, 12133, 12416, 12706,
  13003, 13307, 13619, 13937, 14263,
  14596, 14937, 15286, 15644, 16009,
  16384, 16766, 17158, 17559, 17970,
  18390, 18820, 19260, 19710, 20171,
  20642, 21125, 21618, 22124, 22641,
  23170, 23712, 24266, 24833, 25413,
  26007, 26615, 27238, 27874, 28526,
  29192, 29875, 30573, 31288, 32019,
  32768, 33533, 34317, 35119, 35940,
  36780, 37640, 38520, 39420, 40342,
  41285, 42250, 43237, 44248, 45282,
  46340, 47424, 48532, 49666, 50827,
  52015, 53231, 54476, 55749, 57052,
  58385, 59750, 61147, 62576, 64039,
  65536, 67067, 68635, 70239, 71881,
  73561, 75281, 77040, 78841, 80684,
  82570, 84500, 86475, 88496, 90565,
  92681, 94848, 97065, 99334, 101655,
  104031, 106463, 108952, 111498, 114104,
  116771, 119501, 122294, 125152, 128078,
  131072, 134135, 137270, 140479, 143763,
  147123, 150562, 154081, 157682, 161368,
  165140, 169000, 172950, 176993, 181130,
  185363, 189696, 194130, 198668, 203311,
  208063, 212927, 217904, 222997, 228209,
  233543, 239002, 244589, 250305, 256156,
  262144, 999999
};

/** g_detection_chromatix:
 *
 *  Chromatix for Face detection
 **/
static fd_chromatix_t g_detection_chromatix = {
  #include "fd_chromatix_detect_qc.h"
};

/** g_recognition_chromatix:
 *
 *  Chromatix for Face recognition
 **/
static fd_chromatix_t g_recognition_chromatix = {
  #include "fd_chromatix_recog_qc.h"
};

/**
 * Function: module_faceproc_client_get_buf
 *
 * Description: get the buffer for face detection
 *
 * Arguments:
 *   @p_client - Faceproc client pointer
 *
 * Return values:
 *   buffer index
 *
 * Notes: none
 **/
inline int module_faceproc_client_get_buf(faceproc_client_t *p_client)
{
  uint32_t idx = p_client->buf_idx;
  p_client->buf_idx = (p_client->buf_idx + 1) %
    p_client->buffer_info.fd_buf_count;
  return idx;
}

 /**
 * Function: module_faceproc_client_update_face_info
 *
 * Description: Update face information from face detection result
 *
 * Arguments:
 *   @faces - Face detection info to be filled
 *   @roi - Face proc information output of the algorithm
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
int module_faceproc_client_update_face_info(cam_face_detection_info_t *faces,
  faceproc_info_t *roi)
{
  cam_face_detection_data_t faces_data;
  uint32_t i = 0;

  if (!faces || !roi) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  faces->face_id = roi->unique_id;
  faces->score = (roi->fd_confidence/10);

  faces->face_boundary.top = roi->face_boundary.y;
  faces->face_boundary.left = roi->face_boundary.x;
  faces->face_boundary.width = roi->face_boundary.dx;
  faces->face_boundary.height = roi->face_boundary.dy;
#if(FACE_PART_DETECT)
  faces->smile_degree = roi->sm.smile_degree;
  faces->smile_confidence = roi->sm.confidence;
  faces->face_recognised = roi->is_face_recognised;
  faces->gaze_angle = roi->gaze_angle;
  faces->left_right_gaze = roi->left_right_gaze;
  faces->top_bottom_gaze = roi->top_bottom_gaze;
  faces->blink_detected = roi->blink_detected;
  faces->left_blink = (roi->left_blink/10);
  faces->right_blink = (roi->right_blink/10);

  faces->left_eye_center.x = roi->fp.face_pt[FACE_PART_LEFT_EYE].x;
  faces->left_eye_center.y = roi->fp.face_pt[FACE_PART_LEFT_EYE].y;
  faces->right_eye_center.x = roi->fp.face_pt[FACE_PART_RIGHT_EYE].x;
  faces->right_eye_center.y = roi->fp.face_pt[FACE_PART_RIGHT_EYE].y;

  faces->mouth_center.x = roi->fp.face_pt[FACE_PART_MOUTH].x;
  faces->mouth_center.y = roi->fp.face_pt[FACE_PART_MOUTH].y;
  faces->roll_dir       = roi->fp.direction_roll;
  faces->updown_dir     = roi->fp.direction_up_down;
#endif

  return IMG_SUCCESS;
}

 /**
 * Function: module_faceproc_client_scale_face_info
 *
 * Description: Scale face information from face detection result
 *
 * Arguments:
 *   @faces - Face detection info to be filled
 *   @trans_info - Transition info to be applied
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
int module_faceproc_client_scale_face_info(cam_face_detection_info_t *faces,
  img_trans_info_t *trans_info)
{
  cam_face_detection_data_t faces_data;
  uint32_t i = 0;

  if (!faces || !trans_info) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  if (IMG_F_EQUAL(trans_info->h_scale, 0.0)
    || (IMG_F_EQUAL(trans_info->v_scale, 0.0))) {
    IDBG_ERROR("%s:%d] Invalid scale factors", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_MED("%s:%d] %f %f %d %d", __func__, __LINE__,
    trans_info->h_scale,
    trans_info->v_scale,
    trans_info->h_offset,
    trans_info->v_offset);

  faces->face_boundary.top =
    IMG_TRANSLATE(faces->face_boundary.top,
      trans_info->v_scale,
      trans_info->v_offset);

  faces->face_boundary.left =
    IMG_TRANSLATE(faces->face_boundary.left,
      trans_info->h_scale,
      trans_info->h_offset);

  faces->face_boundary.width =
      faces->face_boundary.width * trans_info->h_scale;

  faces->face_boundary.height =
      faces->face_boundary.height * trans_info->v_scale;

#if(FACE_PART_DETECT)
  faces->left_eye_center.x =
    IMG_TRANSLATE(faces->left_eye_center.x,
      trans_info->h_scale,
      trans_info->h_offset);

  faces->left_eye_center.y =
    IMG_TRANSLATE(faces->left_eye_center.y,
      trans_info->v_scale,
      trans_info->v_offset);

  faces->right_eye_center.x =
    IMG_TRANSLATE(faces->right_eye_center.x,
      trans_info->h_scale,
      trans_info->h_offset);

  faces->right_eye_center.y =
    IMG_TRANSLATE(faces->right_eye_center.y,
      trans_info->v_scale,
      trans_info->v_offset);

  faces->mouth_center.x =
    IMG_TRANSLATE(faces->mouth_center.x,
      trans_info->h_scale,
      trans_info->h_offset);

  faces->mouth_center.y =
    IMG_TRANSLATE(faces->mouth_center.y,
      trans_info->v_scale,
      trans_info->v_offset);
#endif

  return IMG_SUCCESS;
}

/**
 * Function: module_faceproc_client_send_faceinfo
 *
 * Description: send the face info to metadata stream
 *
 * Arguments:
 *   @p_client - Faceproc client pointer
 *   p_result - Face detection result
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_faceproc_client_send_faceinfo(faceproc_client_t *p_client,
  faceproc_result_t *p_result)
{
  cam_face_detection_data_t faces_data;
  img_trans_info_t curr_trans_info;
  uint32_t i = 0;

  if ((IMG_F_EQUAL(p_result->trans_info.h_scale, 0.0)
    || (IMG_F_EQUAL(p_result->trans_info.v_scale, 0.0)))
    &&  (p_result->num_faces_detected != 0)) {
    IDBG_ERROR("%s:%d] Invalid scale factors", __func__, __LINE__);
    return;
  }
  IDBG_MED("%s:%d] %f %f %d %d out scale %f %f", __func__, __LINE__,
    p_result->trans_info.h_scale,
    p_result->trans_info.v_scale,
    p_result->trans_info.h_offset,
    p_result->trans_info.v_offset,
    p_client->out_trans_info.h_scale,
    p_client->out_trans_info.v_scale);
  IDBG_MED("%s:%d] Num face detected %d", __func__, __LINE__,
    p_result->num_faces_detected);

  memset(&faces_data, 0x0, sizeof(cam_face_detection_data_t));

  /* populate the values */
  faces_data.num_faces_detected = p_result->num_faces_detected;
  faces_data.frame_id = p_result->frame_id;

  for (i = 0; i < p_result->num_faces_detected; i++) {
    module_faceproc_client_update_face_info(&faces_data.faces[i],
      &p_result->roi[i]);
    if (IMG_IS_FACE_SCALING_NEEDED(p_result->trans_info)) {
      module_faceproc_client_scale_face_info(&faces_data.faces[i],
        &p_result->trans_info);
    }
  }

  /* Apply additional correction based on output preview size */
  if (IMG_IS_FACE_SCALING_NEEDED(p_client->out_trans_info)) {
    for (i = 0; i < p_result->num_faces_detected; i++) {
      module_faceproc_client_scale_face_info(&faces_data.faces[i],
        &p_client->out_trans_info);
      IDBG_LOW("[FD_ZOOM_DBG] FD_ImgCord (%d %d %d %d)",
        faces_data.faces[i].face_boundary.left,
        faces_data.faces[i].face_boundary.top,
        faces_data.faces[i].face_boundary.width,
        faces_data.faces[i].face_boundary.height);
    }
  }

  faces_data.fd_frame_dim.width  = p_client->stream_info->dim.width;
  faces_data.fd_frame_dim.height = p_client->stream_info->dim.height;

  /* create bus message ToDo: move to another function */
  mct_bus_msg_t bus_msg;
  mct_module_t *p_mct_mod =
    MCT_MODULE_CAST(MCT_PORT_PARENT(p_client->p_sinkport)->data);
  bus_msg.type = MCT_BUS_MSG_FACE_INFO;
  bus_msg.msg = (void *)&faces_data;
  bus_msg.sessionid = IMGLIB_SESSIONID(p_client->identity);
  IDBG_MED("%s:%d] session id %d mct_mod %p", __func__, __LINE__,
    bus_msg.sessionid, p_mct_mod);
  mct_module_post_bus_msg(p_mct_mod, &bus_msg);

}

/**
 * Function: module_faceproc_sort_results
 *
 * Description: Sort the face detection results depending on
 * largest face in in descending order
 *
 * Arguments:
 *   @p_client - Faceproc client pointer
 *   @p_result - Face detection result
 *   @frame_id - frame id
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/

int module_faceproc_sort_results(faceproc_result_t *p_result,
  int start, int end)
{
  if(end > start+1){
    int width = p_result->roi[start].face_boundary.dx *
      p_result->trans_info.h_scale;
    int height = p_result->roi[start].face_boundary.dy *
      p_result->trans_info.v_scale;
    int pivot = width * height;
    int left = start+1, right = end;
    IDBG_MED("%s:%d] width=%d, height %d pivot=%d ", __func__, __LINE__,
      width, height, pivot);
    while(left < right) {
      int lwidth = p_result->roi[left].face_boundary.dx *
        p_result->trans_info.h_scale;
      int lheight = p_result->roi[left].face_boundary.dy *
        p_result->trans_info.v_scale;
      IDBG_MED("%s:%d] lwidth=%d, lheight %d pivot=%d ", __func__, __LINE__,
        lwidth, height, pivot);
      if((lwidth * lheight) >= pivot)
        left++;
      else
        SWAP(faceproc_info_t, p_result->roi[left], p_result->roi[--right]);

    }
    SWAP(faceproc_info_t, p_result->roi[--left], p_result->roi[start]);
    module_faceproc_sort_results(p_result, start, left);
    module_faceproc_sort_results(p_result, right, end);
  }
  return 1;
 }

/**
 * Function: module_faceproc_client_check_boundary
 *
 * Description: validates the boundary of the FD cordiantes
 *
 * Arguments:
 *   @p_client - Faceproc client pointer
 *   @p_fd_rect - Face detection input cordinates
 *   @p_img_rect - Display window
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: none
 **/
inline boolean module_faceproc_client_check_boundary(
  faceproc_client_t *p_client,
  fd_rect_t *p_fd_rect,
  img_rect_t *p_img_rect)
{
  if ((p_fd_rect->x < p_img_rect->pos.x) ||
    (p_fd_rect->y < p_img_rect->pos.y) ||
    ((p_fd_rect->x + p_fd_rect->dx) >
    (p_img_rect->pos.x + p_img_rect->size.width)) ||
    ((p_fd_rect->y + p_fd_rect->dy) >
    (p_img_rect->pos.y + p_img_rect->size.height))) {
    /* out of boundary */
    IMG_PRINT_RECT(p_img_rect);
    FD_PRINT_RECT(p_fd_rect);
    return FALSE;
  }
  IMG_PRINT_RECT(p_img_rect);
  FD_PRINT_RECT(p_fd_rect);
  return TRUE;
}


/**
 * Function: module_faceproc_client_filter_roi
 *
 * Description: filter the ROIs which falls outside the display
 *              window
 *
 * Arguments:
 *   @p_client - Faceproc client pointer
 *   @p_in_result - Face detection input result
 *   @p_out_result - Face detection output result
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_faceproc_client_filter_roi(faceproc_client_t *p_client,
  faceproc_result_t *p_in_result, faceproc_result_t *p_out_result)
{
  uint32_t i = 0;
  for (i = 0; i < p_in_result->num_faces_detected; i++) {
    if (module_faceproc_client_check_boundary(p_client,
      &p_in_result->roi[i].face_boundary, &p_client->crop_info)) {
      p_out_result->trans_info = p_in_result->trans_info;
      p_out_result->roi[i] = p_in_result->roi[i];
      p_out_result->num_faces_detected++;
    }
  }
  p_out_result->frame_id = p_in_result->frame_id;
}

/**
 * Function: module_faceproc_client_send_roi_event
 *
 * Description: send the face info to upstream
 *
 * Arguments:
 *   @p_client - Faceproc client pointer
 *   @p_result - Face detection result
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_faceproc_client_send_roi_event(faceproc_client_t *p_client,
  faceproc_result_t *p_result)
{
  mct_event_t mct_event;
  mct_face_info_t face_info;
  uint32_t i = 0;
  img_rect_t in_region;
  img_rect_t out_region;
  img_size_t preview_dim;
  int status = IMG_SUCCESS;
  double zoom_ratio;

  memset(&mct_event, 0x0, sizeof(mct_event_t));
  memset(&face_info, 0x0, sizeof(mct_face_info_t));

  if (p_client->zoom_val > IMG_NUM_ZOOM_ENTRIES)
    return;

  /* populate the values */
  face_info.frame_id = p_result->frame_id;
  for (i = 0; i < p_result->num_faces_detected; i++) {
    face_info.face_count++;
    face_info.faces[i].score = (p_result->roi[i].fd_confidence/10);
    face_info.faces[i].face_id = p_result->roi[i].unique_id;

    in_region.pos.x = p_result->roi[i].face_boundary.x;
    in_region.pos.y = p_result->roi[i].face_boundary.y;
    in_region.size.width = p_result->roi[i].face_boundary.dx;
    in_region.size.height = p_result->roi[i].face_boundary.dy;
    preview_dim.width = p_client->stream_info->dim.width;
    preview_dim.height = p_client->stream_info->dim.height;
    zoom_ratio = (double)g_zoom_table_def[p_client->zoom_val]/
      (double)g_zoom_table_def[0];
    IDBG_MED("%s:%d] zoom ratio %f", __func__, __LINE__, zoom_ratio);

#ifdef USE_INTERNAL_ZOOM
    status = img_translate_cordinates_zoom(p_client->main_dim,
      preview_dim, &in_region, &out_region, zoom_ratio,
      g_zoom_table_def, IMG_NUM_ZOOM_ENTRIES);
    if (IMG_ERROR(status))
      continue;
#else
    out_region.pos.x =
      IMG_TRANSLATE2(in_region.pos.x,
      p_client->camif_trans_info.h_scale,
      p_client->camif_trans_info.h_offset);
    out_region.pos.y =
      IMG_TRANSLATE2(in_region.pos.y,
      p_client->camif_trans_info.v_scale,
      p_client->camif_trans_info.v_offset);
    out_region.size.width =
      IMG_TRANSLATE(in_region.size.width,
      p_client->camif_trans_info.h_scale,
      0);
    out_region.size.height =
      IMG_TRANSLATE(in_region.size.height,
      p_client->camif_trans_info.v_scale,
      0);
#endif

    face_info.orig_faces[i].roi.top = in_region.pos.y;
    face_info.orig_faces[i].roi.left = in_region.pos.x;
    face_info.orig_faces[i].roi.width = in_region.size.width;
    face_info.orig_faces[i].roi.height = in_region.size.height;
    face_info.faces[i].roi.top = out_region.pos.y;
    face_info.faces[i].roi.left = out_region.pos.x;
    face_info.faces[i].roi.width = out_region.size.width;
    face_info.faces[i].roi.height = out_region.size.height;
  }

  /* create MCT event and send */
  if (p_client->prev_face_count || face_info.face_count) {
    mct_event.u.module_event.type = MCT_EVENT_MODULE_FACE_INFO;
    mct_event.u.module_event.module_event_data = (void *)&face_info;
    mct_event.type = MCT_EVENT_MODULE_EVENT;
    mct_event.identity = p_client->identity;
    mct_event.direction = MCT_EVENT_UPSTREAM;
    mct_port_send_event_to_peer(p_client->port, &mct_event);
  }
  p_client->prev_face_count = face_info.face_count;
}

/**
 * Function: module_faceproc_client_debug
 *
 * Description: print the faceproc result
 *
 * Arguments:
 *   p_result - Face detection result
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_faceproc_client_debug(faceproc_result_t *p_result)
{
  uint32_t i = 0;
  IDBG_HIGH("Faceproc result num_faces_detected %d",
    p_result->num_faces_detected);

  for (i = 0; i < p_result->num_faces_detected; i++) {
    IDBG_HIGH("Faceproc face[%d] blink_detected %d", i,
      p_result->roi[i].blink_detected);
    IDBG_HIGH("Faceproc face[%d] face_boundary (%d %d %d %d)", i,
      p_result->roi[i].face_boundary.x,
      p_result->roi[i].face_boundary.y,
      p_result->roi[i].face_boundary.dx,
      p_result->roi[i].face_boundary.dy);
    IDBG_HIGH("Faceproc face[%d] fd_confidence %d", i,
      p_result->roi[i].fd_confidence);
    IDBG_HIGH("Faceproc face[%d] fp.direction_left_right %d", i,
      p_result->roi[i].fp.direction_left_right);
    IDBG_HIGH("Faceproc face[%d] fp.direction_up_down %d", i,
      p_result->roi[i].fp.direction_up_down);
    IDBG_HIGH("Faceproc face[%d] fp.direction_roll %d", i,
      p_result->roi[i].fp.direction_roll);
    IDBG_HIGH("Faceproc face[%d] gaze_angle %d", i,
      p_result->roi[i].gaze_angle);
    IDBG_HIGH("Faceproc face[%d] is_face_recognised %d", i,
      p_result->roi[i].is_face_recognised);
    IDBG_HIGH("Faceproc face[%d] left_blink %d", i,
      p_result->roi[i].left_blink);
    IDBG_HIGH("Faceproc face[%d] right_blink %d", i,
      p_result->roi[i].right_blink);
    IDBG_HIGH("Faceproc face[%d] left_right_gaze %d", i,
      p_result->roi[i].left_right_gaze);
    IDBG_HIGH("Faceproc face[%d] top_bottom_gaze %d", i,
      p_result->roi[i].top_bottom_gaze);
    IDBG_HIGH("Faceproc face[%d] sm.confidence %d", i,
      p_result->roi[i].sm.confidence);
    IDBG_HIGH("Faceproc face[%d] sm.smile_degree %d", i,
      p_result->roi[i].sm.smile_degree);
    IDBG_HIGH("Faceproc face[%d] Left Eye (%d %d)", i,
      p_result->roi[i].fp.face_pt[FACE_PART_LEFT_EYE].x,
      p_result->roi[i].fp.face_pt[FACE_PART_LEFT_EYE].y);
    IDBG_HIGH("Faceproc face[%d] Right Eye (%d %d)", i,
      p_result->roi[i].fp.face_pt[FACE_PART_RIGHT_EYE].x,
      p_result->roi[i].fp.face_pt[FACE_PART_RIGHT_EYE].y);
    IDBG_HIGH("Faceproc face[%d] Mouth (%d %d)", i,
      p_result->roi[i].fp.face_pt[FACE_PART_MOUTH].x,
      p_result->roi[i].fp.face_pt[FACE_PART_MOUTH].y);
  }
}

/**
 * Function: module_faceproc_client_event_handler
 *
 * Description: event handler for FaceProc client
 *
 * Arguments:
 *   p_appdata - FaceProc test object
 *   p_event - pointer to the event
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int module_faceproc_client_event_handler(void* p_appdata,
  img_event_t *p_event)
{
  faceproc_client_t *p_client = (faceproc_client_t *)p_appdata;
  img_component_ops_t *p_comp = &p_client->comp;
  int rc = IMG_SUCCESS;
  img_frame_t *p_frame = NULL;

  if ((NULL == p_event) || (NULL == p_appdata)) {
    IDBG_ERROR("%s:%d] invalid event", __func__, __LINE__);
    return IMG_SUCCESS;
  }
  IDBG_LOW("%s:%d] type %d", __func__, __LINE__, p_event->type);

  switch (p_event->type) {
  case QIMG_EVT_FACE_PROC: {
    /* get the result*/
    int index = 0;
    pthread_mutex_lock(&p_client->result_mutex);
    rc = IMG_COMP_GET_PARAM(p_comp, QWD_FACEPROC_RESULT,
      (void *)&p_client->result[p_client->fd_active_index]);
    if (rc == IMG_SUCCESS) {
      index = p_client->fd_active_index;
      p_client->fd_active_index = (p_client->fd_active_index + 1) % 2;
    }
    pthread_mutex_unlock(&p_client->result_mutex);

    /* Signal module that face register is done */
    if (p_client->mode == FACE_REGISTER)
      pthread_cond_signal(&p_client->cond);

#ifdef FD_RESULT_DEBUG
    IDBG_HIGH("%s:%d] index %d", __func__, __LINE__, index);
    module_faceproc_client_debug(&p_client->result[index]);
#endif
    break;
  }
  case QIMG_EVT_BUF_DONE:
    /*send the buffer back*/
    rc = IMG_COMP_DQ_BUF(p_comp, &p_frame);
    IDBG_MED("%s:%d] buffer idx %d", __func__, __LINE__, p_frame->idx);
    break;
  case QIMG_EVT_ERROR:
    rc = IMG_ERR_GENERAL;

    /*send the buffer back*/
    IMG_COMP_DQ_BUF(p_comp, &p_frame);
    if (p_client->mode == FACE_REGISTER)
      pthread_cond_signal(&p_client->cond);
    break;
  default:
    break;
  }
  return rc;
}

/**
 * Function: module_faceproc_client_destroy
 *
 * Description: This function is used to destroy the faceproc client
 *
 * Arguments:
 *   @p_client: faceproc client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
void module_faceproc_client_destroy(faceproc_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = NULL;

  if (NULL == p_client) {
    return;
  }

  p_comp = &p_client->comp;
  IDBG_MED("%s:%d] state %d", __func__, __LINE__, p_client->state);

  if (IMGLIB_STATE_STARTED == p_client->state) {
    module_faceproc_client_stop(p_client);
  }

  if (IMGLIB_STATE_INIT == p_client->state) {
    rc = IMG_COMP_DEINIT(p_comp);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] deinit failed %d", __func__, __LINE__, rc);
    }
    p_client->state = IMGLIB_STATE_IDLE;
  }

  if (IMGLIB_STATE_IDLE == p_client->state) {
    pthread_mutex_destroy(&p_client->mutex);
    pthread_cond_destroy(&p_client->cond);
    if (p_client->buffer_info.p_buffer) {
      free(p_client->buffer_info.p_buffer);
      p_client->buffer_info.p_buffer = NULL;
      p_client->buffer_info.buf_count = 0;
    }
    free(p_client);
    p_client = NULL;
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);
}

/**
 * Function: module_faceproc_client_exec
 *
 * Description: This function is used to execute the faceproc
 *              client
 *
 * Arguments:
 *   @p_client: faceproc client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_faceproc_client_exec(faceproc_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;
  mct_stream_info_t *stream_info;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  stream_info = p_client->stream_info;

#ifdef FD_USE_INTERNAL_MEM
  if ((uint32_t)stream_info->dim.width > MAX_FD_WIDTH ||
    (uint32_t)stream_info->dim.height > MAX_FD_HEIGHT) {
    p_client->config.frame_cfg.max_width =
      (uint32_t)stream_info->dim.width;
    p_client->config.frame_cfg.max_height =
      (uint32_t)stream_info->dim.height;
  } else {
    p_client->config.frame_cfg.max_width = MAX_FD_WIDTH;
    p_client->config.frame_cfg.max_height = MAX_FD_HEIGHT;
  }
#else
  p_client->config.frame_cfg.max_width =
    (uint32_t)stream_info->dim.width;
  p_client->config.frame_cfg.max_height =
    (uint32_t)stream_info->dim.height;
#endif

  IDBG_HIGH("%s:%d] %dx%d", __func__, __LINE__,
    p_client->config.frame_cfg.max_width,
    p_client->config.frame_cfg.max_height);

  /* Initialize output transition info */
  p_client->out_trans_info.h_scale  = 1.0;
  p_client->out_trans_info.v_scale  = 1.0;
  p_client->out_trans_info.h_offset = 0;
  p_client->out_trans_info.v_offset = 0;

#ifdef FD_USE_PROP
  if (p_client->config.fd_feature_mask & FACE_PROP_MIN_FACE_SIZE) {
    int face_size = (p_client->config.fd_feature_mask & FACE_PROP_MIN_FACE_SIZE)
        >> FACE_PROP_MIN_FACE_SIZE_SHIFT;
    switch (face_size) {
    case 1:
      p_client->p_fd_chromatix->min_face_size = 25;
      break;
    case 2:
      p_client->p_fd_chromatix->min_face_size = 100;
      break;
    case 3:
      p_client->p_fd_chromatix->min_face_size = 200;
      break;
    }
    p_client->p_fd_chromatix->min_face_adj_type = FD_FACE_ADJ_FIXED;
  }
#endif

  rc = IMG_COMP_SET_CB(p_comp, module_faceproc_client_event_handler);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QWD_FACEPROC_MODE, (void *)&p_client->mode);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QWD_FACEPROC_CFG, (void *)&p_client->config);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QWD_FACEPROC_CHROMATIX,
    (void *)p_client->p_fd_chromatix);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_START(p_comp, NULL);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] create failed %d", __func__, __LINE__, rc);
    return rc;
  }
  IDBG_MED("%s:%d] X rc %d", __func__, __LINE__, rc);
  return rc;
}

/**
 * Function: module_faceproc_client_thread_func
 *
 * Description: This function represents the event thread for
 *             the faceproc modules
 *
 * Arguments:
 *   @data: faceproc client
 *
 * Return values:
 *     NULL
 *
 * Notes: none
 **/
void *module_faceproc_client_thread_func(void *data)
{
  int rc = IMG_SUCCESS;
  faceproc_client_t *p_client = (faceproc_client_t *)data;

  IDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "faceproc_thread", 0, 0, 0);
  /*signal the base class*/
  pthread_mutex_lock(&p_client->mutex);
  pthread_cond_signal(&p_client->cond);
  pthread_mutex_unlock(&p_client->mutex);

  rc = module_faceproc_client_exec(p_client);
  if (IMG_ERROR(rc)) {
    p_client->status = rc;
    return NULL;
  }

  /* event handler at module level?? */
  return NULL;
}

/**
 * Function: module_faceproc_client_create
 *
 * Description: This function is used to create the faceproc client
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @p_port: mct port pointer
 *   @identity: identity of the stream
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_faceproc_client_create(mct_module_t *p_mct_mod, mct_port_t *p_port,
  uint32_t identity, mct_stream_info_t *stream_info)
{
  int rc = IMG_SUCCESS;
  faceproc_client_t *p_client = NULL;
  img_component_ops_t *p_comp = NULL;
  img_core_ops_t *p_core_ops = NULL;
  module_faceproc_t *p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  mct_list_t *p_temp_list = NULL;
  char value[32];
  int fd_feature_mask = 0;

  p_core_ops = &p_mod->core_ops;

  IDBG_MED("%s:%d]", __func__, __LINE__);
  p_client = (faceproc_client_t *)malloc(sizeof(faceproc_client_t));
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] client alloc failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  /* initialize the variables */
  memset(p_client, 0x0, sizeof(faceproc_client_t));
  p_comp = &p_client->comp;
  pthread_mutex_init(&p_client->mutex, NULL);
  pthread_cond_init(&p_client->cond, NULL);
  p_client->threadid = -1;
  p_client->state = IMGLIB_STATE_IDLE;
  p_client->sync = MOD_FACEPROC_SYNC;
  p_client->stream_info = stream_info;

  p_client->buffer_info.p_buffer = malloc(MAX_NUM_FRAMES *
    sizeof(mod_img_buffer_t));
  if (NULL == p_client->buffer_info.p_buffer) {
    IDBG_ERROR("%s:%d] p_client->buffer_info.p_buffer alloc failed",
      __func__, __LINE__);
    rc = IMG_ERR_NO_MEMORY;
    goto error;
  }
  memset(p_client->buffer_info.p_buffer, 0x0, MAX_NUM_FRAMES *
    sizeof(mod_img_buffer_t));
  p_client->buffer_info.total_count = MAX_NUM_FRAMES;
  p_client->buffer_info.fd_buf_count = MAX_NUM_FD_FRAMES;

  rc = IMG_COMP_CREATE(p_core_ops, p_comp);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] create failed %d", __func__, __LINE__, rc);
    goto error;
  }

  rc = IMG_COMP_INIT(p_comp, p_client, NULL);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] init failed %d", __func__, __LINE__, rc);
    goto error;
  }
  p_client->state = IMGLIB_STATE_INIT;

  p_client->p_sinkport = p_port;
  p_client->identity = identity;
  p_port->port_private = p_client;
  p_client->port = p_port;
  p_client->buf_idx = 0;
  p_client->p_mod = p_mod;

  /* Todo: If module is set as source we will do face registration */
  if (p_mod->module_type == MCT_MODULE_FLAG_SOURCE) {
    p_client->mode = FACE_REGISTER;
    p_client->p_fd_chromatix = &g_recognition_chromatix;
  } else {
    p_client->mode = FACE_DETECT_OFF;
    p_client->p_fd_chromatix = &g_detection_chromatix;
  }
  p_client->frame_skip_cnt = p_client->p_fd_chromatix->frame_skip;


  // Get the featurue mask from property
#ifdef FD_USE_PROP
  property_get("debug.sf.fdfeature", value, "0");
  fd_feature_mask = atoi(value);
  if (fd_feature_mask > 0) {
    p_client->config.fd_feature_mask = fd_feature_mask;
    p_mod->active = FALSE;
  } else {
    p_client->config.fd_feature_mask = FACE_PROP_DEFAULT;
  }
#else
  p_client->config.fd_feature_mask = FACE_PROP_DEFAULT;
#endif
  IDBG_ERROR("%s:%d] Face proc feature mask %x %x", __func__, __LINE__,
    p_client->config.fd_feature_mask, fd_feature_mask);


  IDBG_MED("%s:%d] port %p client %p X", __func__, __LINE__, p_port, p_client);
  return rc;

error:
  if (p_client) {
    module_faceproc_client_destroy(p_client);
    p_client = NULL;
  }
  return rc;
}

/**
 * Function: module_faceproc_client_map_buffers
 *
 * Description: This function is used to map the buffers when
 * the stream is started
 *
 * Arguments:
 *   @p_client: faceproc client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_faceproc_client_map_buffers(faceproc_client_t *p_client)
{
  mct_stream_info_t *stream_info;
  mod_img_buffer_info_t *p_buf_info;
  uint32_t i = 0;

  if (!(p_client && p_client->stream_info))
    return IMG_ERR_INVALID_INPUT;

  if (p_client->state == IMGLIB_STATE_INIT ||
      p_client->state == IMGLIB_STATE_IDLE) {
     IDBG_ERROR("%s:%d] client not started", __func__, __LINE__);
     return IMG_SUCCESS;
  }

  stream_info = p_client->stream_info;
  if ((uint32_t)stream_info->dim.width >
    p_client->config.frame_cfg.max_width ||
    (uint32_t)stream_info->dim.height >
    p_client->config.frame_cfg.max_height) {
    IDBG_MED("%s:%d] Exceeded max size %dx%d", __func__, __LINE__,
      stream_info->dim.width, stream_info->dim.width);
    return IMG_ERR_INVALID_INPUT;
  }

  p_buf_info = &p_client->buffer_info;
  p_buf_info->buf_count = 0;

  if (p_client->mode == FACE_REGISTER) {
    mct_list_traverse(stream_info->img_buffer_list, mod_imglib_map_fr_buffer,
      p_buf_info);
  } else {
    mct_list_traverse(stream_info->img_buffer_list, mod_imglib_map_fd_buffer,
      p_buf_info);
  }

  for (i = 0; i < p_client->buffer_info.fd_buf_count; i++) {
    p_buf_info->p_buffer[i].frame.frame_cnt = 1;
    p_buf_info->p_buffer[i].frame.idx =
      p_buf_info->p_buffer[i].map_buf.buf_index;
    p_buf_info->p_buffer[i].frame.info.width = stream_info->dim.width;
    p_buf_info->p_buffer[i].frame.info.height = stream_info->dim.height;
    p_buf_info->p_buffer[i].frame.frame[0].plane_cnt = 1;
    p_buf_info->p_buffer[i].frame.frame[0].plane[0].width =
      stream_info->dim.width;
    p_buf_info->p_buffer[i].frame.frame[0].plane[0].height =
      stream_info->dim.height;
    p_buf_info->p_buffer[i].frame.frame[0].plane[0].plane_type = PLANE_Y;

    // width/height are equal to stride/scanline since
    // library doesn't support stride and scanline
    p_buf_info->p_buffer[i].frame.frame[0].plane[0].stride =
      p_buf_info->p_buffer[i].frame.frame[0].plane[0].width;
    p_buf_info->p_buffer[i].frame.frame[0].plane[0].scanline =
      p_buf_info->p_buffer[i].frame.frame[0].plane[0].height;
    p_buf_info->p_buffer[i].frame.frame[0].plane[0].length =
      p_buf_info->p_buffer[i].frame.frame[0].plane[0].stride *
      p_buf_info->p_buffer[i].frame.frame[0].plane[0].scanline;

    if (p_client->mode == FACE_REGISTER) {
      int  p_cnt, plane_cnt = p_buf_info->p_buffer[i].frame.frame[0].plane_cnt;
      for (p_cnt = 0; p_cnt < plane_cnt; p_cnt++)
        p_buf_info->p_buffer[i].frame.frame[0].plane[p_cnt].addr =
            p_buf_info->p_buffer[i].map_buf.buf_planes[p_cnt].buf;
    } else {
      p_buf_info->p_buffer[i].frame.frame[0].plane[0].addr =
        malloc(p_buf_info->p_buffer[i].frame.frame[0].plane[0].length);
      if (NULL == p_buf_info->p_buffer[i].frame.frame[0].plane[0].addr) {
        IDBG_ERROR("%s:%d] buffer create failed", __func__, __LINE__);
        return IMG_ERR_NO_MEMORY;
      }
    }
  }
  IDBG_MED("%s:%d] dim %dx%d", __func__, __LINE__,
    stream_info->dim.width, stream_info->dim.height);
  return IMG_SUCCESS;
}

/**
 * Function: module_faceproc_client_unmap_buffers
 *
 * Description: This function is used to unmap the buffers when
 * the stream is started
 *
 * Arguments:
 *   @p_client: faceproc client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_faceproc_client_unmap_buffers(faceproc_client_t *p_client)
{
  mod_img_buffer_info_t *p_buf_info;
  uint32_t i = 0;
  img_sub_frame_t *p_sub_frame;

  /* When face registration is used buffers are not allocated */
  if (p_client->mode == FACE_REGISTER) {
    p_client->buffer_info.buf_count = 0;
    return IMG_SUCCESS;
  }

  p_buf_info = &p_client->buffer_info;
  for (i = 0; i < p_client->buffer_info.fd_buf_count; i++) {
    p_sub_frame = &p_buf_info->p_buffer[i].frame.frame[0];
    if (p_sub_frame->plane[0].addr) {
      free(p_sub_frame->plane[0].addr);
    }
  }
  p_client->buffer_info.buf_count = 0;
  return IMG_SUCCESS;
}

/**
 * Function: module_faceproc_client_set_scale_ratio
 *
 * Description: Set face detection scale ratio.
 *
 * Arguments:
 *   @p_client: faceproc client
 *   @stream_crop: Stream crop event structure
 *
 * Return values:
 *     imaging error values
 *
 * Notes: Calculate just scale ratio which we assume that will be applied
 *  to get preview resolution.
 **/
int module_faceproc_client_set_scale_ratio(faceproc_client_t *p_client,
  mct_bus_msg_stream_crop_t *stream_crop)
{
  int rc = IMG_SUCCESS;

  if (!(p_client && p_client->stream_info) || !stream_crop)
    return IMG_ERR_INVALID_INPUT;

  /* Hack to work around ISP bug */
  if (p_client->zoom_val > 0) {
    p_client->crop_info.pos.x = stream_crop->x;
    p_client->crop_info.pos.y = stream_crop->y;
    p_client->crop_info.size.width = stream_crop->crop_out_x;
    p_client->crop_info.size.height= stream_crop->crop_out_y;
  } else {
    p_client->crop_info.pos.x = 0;
    p_client->crop_info.pos.y = 0;
    p_client->crop_info.size.width = p_client->stream_info->dim.width;
    p_client->crop_info.size.height = p_client->stream_info->dim.height;
  }

  if (p_client->stream_info->dim.width > p_client->crop_info.size.width) {
    p_client->out_trans_info.h_scale =
      (float)p_client->stream_info->dim.width /
      (float)p_client->crop_info.size.width;
    p_client->out_trans_info.h_offset = p_client->crop_info.pos.x;

  } else {
    p_client->out_trans_info.h_scale = 1.0;
    p_client->out_trans_info.h_offset = 0;
  }

  if (p_client->stream_info->dim.height > p_client->crop_info.size.height) {
    p_client->out_trans_info.v_scale =
      (float)p_client->stream_info->dim.height /
      (float)p_client->crop_info.size.height;
    p_client->out_trans_info.v_offset = p_client->crop_info.pos.y;
  } else {
    p_client->out_trans_info.v_scale = 1.0;
    p_client->out_trans_info.v_offset = 0;
  }

  p_client->camif_trans_info.h_scale =
    (float)stream_crop->width_map /
    (float)p_client->stream_info->dim.width;
  p_client->camif_trans_info.v_scale =
    (float)stream_crop->height_map /
    (float)p_client->stream_info->dim.height;
  p_client->camif_trans_info.h_offset =
    stream_crop->x_map;
  p_client->camif_trans_info.v_offset =
    stream_crop->y_map;
  IDBG_MED("%s:%d] [FD_CAMIF] Map(%d %d %d %d)",
    __func__, __LINE__,
    stream_crop->width_map,
    stream_crop->height_map,
    stream_crop->x_map,
    stream_crop->y_map);
  IDBG_MED("%s:%d] [FD_CAMIF] Scale(%f %f %d %d)",
    __func__, __LINE__,
    p_client->camif_trans_info.h_scale,
    p_client->camif_trans_info.v_scale,
    p_client->camif_trans_info.h_offset,
    p_client->camif_trans_info.v_offset);

  return rc;
}


/**
 * Function: module_faceproc_client_start
 *
 * Description: This function is used to start the faceproc
 *              client
 *
 * Arguments:
 *   @p_client: faceproc client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_faceproc_client_start(faceproc_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  module_faceproc_t *p_mod = (module_faceproc_t *)(p_client->p_mod);

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  if (p_client->state != IMGLIB_STATE_INIT) {
    IDBG_ERROR("%s:%d] invalid state %d",
      __func__, __LINE__, rc);
    return IMG_ERR_INVALID_OPERATION;
  }

  /* create the client thread */
  if (FALSE == p_client->sync) {
    p_client->is_ready = FALSE;
    rc = pthread_create(&p_client->threadid, NULL,
       module_faceproc_client_thread_func,
      (void *)p_client);
    pthread_setname_np(p_client->threadid, "CAM_fd_client");
    if (rc < 0) {
      IDBG_ERROR("%s:%d] pthread creation failed %d",
        __func__, __LINE__, rc);
      rc = IMG_ERR_GENERAL;
      goto error;
    }

    if (FALSE == p_client->is_ready) {
      IDBG_MED("%s: before wait", __func__);
      pthread_cond_wait(&p_client->cond, &p_client->mutex);
    }
    IDBG_MED("%s: after wait", __func__);
    if (IMG_ERROR(p_client->status)) {
      rc = p_client->status;
      IDBG_ERROR("%s:%d] create failed %d", __func__, __LINE__, rc);
      goto error;
    }
    p_client->state = IMGLIB_STATE_STARTED;
  } else {
    rc = module_faceproc_client_exec(p_client);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] create failed %d", __func__, __LINE__, rc);
      goto error;
    }
    p_client->state = IMGLIB_STATE_STARTED;
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);

  return rc;

error:
  IDBG_MED("%s:%d] error %d X", __func__, __LINE__, rc);
  return rc;
}

/**
 * Function: module_faceproc_client_stop
 *
 * Description: This function is used to stop the faceproc
 *              client
 *
 * Arguments:
 *   @p_client: faceproc client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_faceproc_client_stop(faceproc_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;
  module_faceproc_t *p_mod = (module_faceproc_t *)(p_client->p_mod);
  mct_event_t mct_event;
  mct_face_info_t face_info;

  rc = IMG_COMP_ABORT(p_comp, NULL);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] create failed %d", __func__, __LINE__, rc);
    return rc;
  }

  /* reset the face info passed to 3A before closing the FD module.
     other wise 3A holds the old face info and uses in video mode too
     where FD is disabled */
  memset(&mct_event, 0x0, sizeof(mct_event_t));
  memset(&face_info, 0x0, sizeof(mct_face_info_t));
  mct_event.u.module_event.type = MCT_EVENT_MODULE_FACE_INFO;
  mct_event.u.module_event.module_event_data = (void *)&face_info;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = p_client->identity;
  mct_event.direction = MCT_EVENT_UPSTREAM;
  mct_port_send_event_to_peer(p_client->port, &mct_event);

  p_client->state = IMGLIB_STATE_INIT;
  return rc;
}

/**
 * Function: module_faceproc_client_send_info
 *
 * Description: This function is for get the active result and
 *    send it to the metadata and stats module
 *
 * Arguments:
 *   @p_client: faceproc client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static void module_faceproc_client_send_info(faceproc_client_t *p_client)
{
  int index;
  faceproc_result_t out_result;
  faceproc_result_t *p_out_result;
  img_rect_t *p_crop;

  memset(&out_result, 0x0, sizeof(faceproc_result_t));
  pthread_mutex_lock(&p_client->result_mutex);
  index = (p_client->fd_active_index == 0) ? 1 : 0;
  IDBG_MED("%s:%d] active index %d face_cnt %d",
    __func__, __LINE__, index,
    p_client->result[index].num_faces_detected);

  /* filter the cordinates */
  p_crop = &p_client->crop_info;
  if (IMG_RECT_IS_VALID(p_crop, p_client->stream_info->dim.width,
    p_client->stream_info->dim.height)) {
    p_out_result = &out_result;
    module_faceproc_client_filter_roi(p_client, &p_client->result[index],
      p_out_result);
    IDBG_MED("%s:%d] face_cnt %d", __func__, __LINE__,
      p_out_result->num_faces_detected);
  } else {
    p_out_result = &p_client->result[index];
    IDBG_MED("%s:%d] face_cnt %d", __func__, __LINE__,
      p_out_result->num_faces_detected);
  }

  if (p_client->p_fd_chromatix->stab_enable) {
    module_faceproc_faces_stabilization(p_client, p_out_result);
  }

  module_faceproc_client_send_faceinfo(p_client, p_out_result);

  module_faceproc_client_send_roi_event(p_client, p_out_result);
  pthread_mutex_unlock(&p_client->result_mutex);
}

/**
 * Function: module_faceproc_client_handle_buffer
 *
 * Description: This function is for handling the buffers
 *            sent from the peer modules
 *
 * Arguments:
 *   @p_client: faceproc client
 *   @buf_idx: index of the buffer to be processed
 *   @frame_id: frame id
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_faceproc_client_handle_buffer(faceproc_client_t *p_client,
  uint32_t buf_idx, uint32_t frame_id)
{
  img_frame_t *p_frame;
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;
  int img_idx;
  mct_stream_map_buf_t *p_map_buf;
  int i;
  uint8_t *src;
  uint8_t *dst;

  /* process the frame only in IMGLIB_STATE_PROCESSING state */
  pthread_mutex_lock(&p_client->mutex);
  if (TRUE != p_client->active) {
    pthread_mutex_unlock(&p_client->mutex);
    return IMG_SUCCESS;
  }

  if (IMGLIB_STATE_PROCESSING != p_client->state) {
    pthread_mutex_unlock(&p_client->mutex);
    return IMG_SUCCESS;
  }

  IDBG_MED("%s:%d] count %d", __func__, __LINE__, p_client->current_count);

  if (0 != p_client->current_count) {
    IDBG_LOW("%s:%d] Skip frame", __func__, __LINE__);
    p_client->current_count =
      (p_client->current_count + 1)%(p_client->frame_skip_cnt+1);
    /* Don't send the FD info for each frame to save
       power consumed by GPU to draw the FD rectangles */
    //module_faceproc_client_send_info(p_client);
    pthread_mutex_unlock(&p_client->mutex);
    return IMG_SUCCESS;
  }
  p_client->current_count =
    (p_client->current_count + 1)%(p_client->frame_skip_cnt+1);

  /* send to metadata buffer */
  module_faceproc_client_send_info(p_client);

  img_idx = module_faceproc_client_get_buf(p_client);
  IDBG_MED("%s:%d] img_idx %d", __func__, __LINE__, img_idx);

  if ((buf_idx >= p_client->buffer_info.buf_count)
    || (img_idx >= (int)p_client->buffer_info.fd_buf_count)) {
    IDBG_ERROR("%s:%d] invalid buffer index %d img_idx %d",
      __func__, __LINE__, buf_idx, img_idx);
    pthread_mutex_unlock(&p_client->mutex);
    return IMG_ERR_OUT_OF_BOUNDS;
  }

  p_frame = &p_client->buffer_info.p_buffer[img_idx].frame;
  p_map_buf = &p_client->buffer_info.p_buffer[buf_idx].map_buf;
  IDBG_MED("%s:%d] buffer %d %p %p", __func__, __LINE__, buf_idx,
    p_frame->frame[0].plane[0].addr,
    p_map_buf->buf_planes[0].buf);

  // remove stride and scanline during buffer copy since
  // library doesn't support stride and scanline
  dst = (uint8_t *)p_frame->frame[0].plane[0].addr;
  src = (uint8_t *)p_map_buf->buf_planes[0].buf;
  for (i = 0; i < p_frame->frame[0].plane[0].height; i++) {
    memcpy(dst, src, p_frame->frame[0].plane[0].width);
    dst += p_frame->frame[0].plane[0].width;
    src += p_map_buf->buf_planes[0].stride;
  }

  p_frame->frame_id = frame_id;

  rc = IMG_COMP_Q_BUF(p_comp, p_frame, IMG_IN);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] cannot enqueue %d", __func__, __LINE__, rc);
  }
  pthread_mutex_unlock(&p_client->mutex);

  return rc;
}

/**
 * Function: module_faceproc_client_process_buffers
 *
 * Description: This function is processing the buffers
 *
 * Arguments:
 *   @p_client: faceproc client
 *   @buf_idx: index of the buffer to be processed
 *   @frame_id: frame id
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_faceproc_client_process_buffers(faceproc_client_t *p_client,
    uint32_t frame_id)
{
  img_frame_t *p_frame;
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;
  uint32_t f_cnt;

  for (f_cnt = 0; f_cnt < p_client->buffer_info.buf_count; f_cnt++) {
    p_frame = &p_client->buffer_info.p_buffer[f_cnt].frame;

    p_frame->frame_id = frame_id;

    rc = IMG_COMP_Q_BUF(p_comp, p_frame, IMG_IN);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] buffer enqueue error %d", __func__, __LINE__, rc);
      break;
    }
    pthread_mutex_lock(&p_client->mutex);
    p_client->state = IMGLIB_STATE_PROCESSING;
    rc = img_wait_for_completion(&p_client->cond, &p_client->mutex, 10000);
    p_client->state = IMGLIB_STATE_STARTED;
    pthread_mutex_unlock(&p_client->mutex);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] buffer Wait timeout %d", __func__, __LINE__, rc);
      break;
    }
  }
  return rc;
}

/**
 * Function: module_faceproc_client_set_mode
 *
 * Description: Set faceproc library mode
 *
 * Arguments:
 *   @p_client: faceproc client
 *   @mode: New faceproc mode
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_faceproc_client_set_mode(faceproc_client_t *p_client,
  faceproc_mode_t mode)
{
  boolean start_processing;
  img_component_ops_t *p_comp;
  int status = IMG_SUCCESS;
  module_faceproc_t *p_mod = NULL;

  if (NULL == p_client)
    return IMG_ERR_INVALID_INPUT;

  if (p_client->mode == mode)
    return IMG_SUCCESS;

  p_mod = (module_faceproc_t *)p_client->p_mod;
  p_comp = &p_client->comp;

  pthread_mutex_lock(&p_client->mutex);
  status = IMG_COMP_SET_PARAM(p_comp, QWD_FACEPROC_MODE, (void *)&mode);

  if (IMG_SUCCESS == status) {
    /* Preserve the mode */
    p_client->mode = mode;

    /* Start processing only if img lib is not in init state */
    if (IMGLIB_STATE_INIT != p_client->state) {
      p_client->state = (FACE_DETECT_OFF == mode) ? IMGLIB_STATE_STARTED :
        IMGLIB_STATE_PROCESSING;
    }
  }
  pthread_mutex_unlock(&p_client->mutex);

  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] Setting Face param Failed", __func__, __LINE__);
    goto out;
  }

out:
  return status;
}
/**
 * Function: module_faceproc_client_handle_ctrl_parm
 *
 * Description: This function is used to handle the ctrl
 *             commands passed from the MCTL
 *
 * Arguments:
 *   @p_ctrl_event: pointer to mctl ctrl events
 *   @p_client: faceproc client
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
int module_faceproc_client_handle_ctrl_parm(faceproc_client_t *p_client,
  mct_event_control_parm_t *param)
{
  int status = IMG_SUCCESS;

  if (NULL == param)
    return status;

  IDBG_MED("%s:%d] param %d", __func__, __LINE__, param->type);
  switch(param->type) {
  case CAM_INTF_PARM_FD: {
    cam_fd_set_parm_t *p_fd_set_parm = (cam_fd_set_parm_t *)param->parm_data;
    faceproc_mode_t new_mode = FACE_DETECT_OFF;

    if (NULL == p_fd_set_parm) {
      IDBG_ERROR("%s:%d] NULL invalid data", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }

    /* Set Face Proc library mode */
    if (CAM_FACE_PROCESS_MASK_RECOGNITION & p_fd_set_parm->fd_mode)
      new_mode = FACE_RECOGNIZE;
    else if (CAM_FACE_PROCESS_MASK_DETECTION & p_fd_set_parm->fd_mode)
      new_mode = FACE_DETECT;

    /* Change the Face proc mode if required */
    module_faceproc_client_set_mode(p_client, new_mode);

    break;
  }
  case CAM_INTF_PARM_ZOOM: {
    int *p_zoom_val = param->parm_data;
    if (NULL == p_zoom_val) {
      IDBG_ERROR("%s:%d] NULL invalid data", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_client->zoom_val = *p_zoom_val;
    IDBG_MED("%s:%d] zoom_val %d", __func__, __LINE__, p_client->zoom_val);
    break;
  }
  default:
    break;
  }
  return status;
}
