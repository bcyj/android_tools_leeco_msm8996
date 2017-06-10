/*============================================================================

Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <string.h>

#include "camera_dbg.h"
#include "frameproc.h"

//static frame_proc_t *frameCtrl[MAX_INSTANCES];
//static uint32_t frame_proc_handle_cnt = 0;
static frame_proc_comp_root_t my_frame_proc_struct;
static pthread_t frame_proc_thread;
static pthread_cond_t frame_proc_ready_cond;
static pthread_mutex_t frame_proc_mutex;
static int frame_proc_terminate_fd[2];
static int frame_proc_thread_fd[2];
static int flag =0;  // to make pp single threaded
static void *frame_processing(void *parm);
static void preview_frameprocess( frame_proc_t *frameCtrl);
static void snapshot_frameprocess( frame_proc_t *frameCtrl);
uint32_t handle1;

static uint32_t frame_proc_gen_handle(uint8_t client_idx)
{
  /* pattern: [24 bit count]|[4bits obj idx]|[4 bits client idx] */
  uint32_t handle = ((++my_frame_proc_struct.frame_proc_handle_cnt) << 8) +
    (0xff & client_idx);
  return handle;
}

static int frame_proc_parse_handle(uint32_t handle, uint8_t *client_idx)
{
  int tmp = (handle & 0xff);
  *client_idx = tmp & 0xff;
  CDBG_FRAME_PROC("%s: handle = 0x%x, client_idx = %d", __func__, handle, *client_idx);
  return 0;
}

/*===========================================================================
 * FUNCTION    - get_frame_proc_client_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
static frame_proc_t *get_frame_proc_client_info(uint32_t handle)
{
  uint8_t client_idx;

  frame_proc_parse_handle(handle, &client_idx);
  if (client_idx >= FRAMEPROC_MAX_CLIENT_NUM ||
    my_frame_proc_struct.client[client_idx].handle != handle) {
    CDBG_ERROR("%s: client_idx = %d, local_handle = 0x%x, input_handle = 0x%x",
      __func__, client_idx,
      my_frame_proc_struct.client[client_idx].handle,
      handle);
    return NULL;
  } else
    return &(my_frame_proc_struct.client[client_idx]);
}  /* get_frame_proc_client_info */

/*===========================================================================
* FUNCTION    - frameproc_client_add_obj -
*
* DESCRIPTION:
*==========================================================================*/

static int frameproc_client_add_obj(frame_proc_t *frame_proc_client, int type, void *parm_in, void *
parm_out)
{
  frame_proc_obj_t *frame_proc_obj;
  int frame_proc_obj_id = *((int *)parm_in);

  if(frame_proc_client->obj_idx_mask > 0) {
    CDBG_ERROR("%s: only support one obj",  __func__);
    return -1;
  }
  frame_proc_client->obj_idx_mask |= (1 << frame_proc_obj_id);
  frame_proc_obj = &my_frame_proc_struct.obj[frame_proc_obj_id];
  frame_proc_obj->refcount++;
  return 0;
} /* frameproc_client_add_obj */

/*===========================================================================
 * FUNCTION    - frameproc_get_obj -
 *
 * DESCRIPTION:
 *==========================================================================*/

static frame_proc_obj_t *frameproc_get_obj(frame_proc_comp_root_t *frame_proc_root,
frame_proc_t *frame_proc_client)
{
  frame_proc_obj_t *frame_proc_obj = NULL;

  if(frame_proc_client->obj_idx_mask == 1)
    frame_proc_obj= &frame_proc_root->obj[0];
  else if(frame_proc_client->obj_idx_mask == 2)
    frame_proc_obj= &frame_proc_root->obj[1];
  return frame_proc_obj;
} /* frameproc_get_obj */
/*===========================================================================
* FUNCTION    - is_face_register_enabled -
*
* DESCRIPTION:
*==========================================================================*/
static int is_face_register_enabled(frame_proc_t *frameCtrl)
{
  return(frameCtrl->output.fd_d.fd_enable &&
    (frameCtrl->output.fd_d.fd_mode == FACE_REGISTER));
}
/*===========================================================================
 * FUNCTION    - frame_proc_translate_coordinates -
 *
 * DESCRIPTION: Adjsut Coordinates to new reference coordinates.
 *==========================================================================*/
static int frame_proc_translate_coordinates(uint16_t *x,uint16_t *y,
  struct fd_rect_t *ref_coordinate, struct fd_rect_t *orig_coordinate)
{
  int rc = 0;
  uint32_t zoom_crp_x, zoom_crp_y;
  int validate_x = *x - ref_coordinate->x;
  int validate_y = *y - ref_coordinate->y;
  if (validate_x < 0 || validate_y < 0)
    return -1;
  zoom_crp_x =  *x - ref_coordinate->x;
  zoom_crp_y = *y - ref_coordinate->y;
  *x = orig_coordinate->dx * zoom_crp_x /  ref_coordinate->dx;
  *y = orig_coordinate->dy * zoom_crp_y / ref_coordinate->dy;
  return rc;
}
/*===========================================================================
 * FUNCTION    - frame_proc_adjust_fd_coordinates -
 *
 * DESCRIPTION: Adjsut Face Detection Coordinates based on MDP Zoom.
 *==========================================================================*/
static int frame_proc_adjust_fd_coordinates(frame_proc_t *frameCtrl,
  struct fd_rect_t *v4l2_crp)
{
  struct fd_rect_t zoom_crp, orig_coordinate;
  int i,k,index,num_valid_faces = 0;
  orig_coordinate.x =0;
  orig_coordinate.y = 0;
  orig_coordinate.dx =frameCtrl->input.mctl_info.display_dim.width;
  orig_coordinate.dy =frameCtrl->input.mctl_info.display_dim.height;
  for (i = 0; i < (int)frameCtrl->output.fd_d.num_faces_detected; i++) {
    /*Translate fd_roi.x and fd_roi.y*/
    if (frame_proc_translate_coordinates(
      (uint16_t *)&frameCtrl->output.fd_d.roi[i].face_boundary.x,
      (uint16_t *)&frameCtrl->output.fd_d.roi[i].face_boundary.y,
      v4l2_crp, &orig_coordinate) <0) {
      CDBG_ERROR("%s x and y Coordinates out of bounds",__func__);
      continue;
    }
    zoom_crp.dx = frameCtrl->output.fd_d.roi[i].face_boundary.dx ;
    zoom_crp.dy = frameCtrl->output.fd_d.roi[i].face_boundary.dy;
    if ((frameCtrl->output.fd_d.roi[i].face_boundary.x+ zoom_crp.dx >
      v4l2_crp->x +v4l2_crp->dx )||(frameCtrl->output.fd_d.roi[i].face_boundary.y
      + zoom_crp.dy >v4l2_crp->y +v4l2_crp->dy)) {
      CDBG_ERROR("%s: dx and dy Coordinates out of bounds",__func__);
      continue;
    }
    frameCtrl->output.fd_d.roi[i].face_boundary.dx =
      frameCtrl->input.mctl_info.display_dim.width * zoom_crp.dx / v4l2_crp->dx;
    frameCtrl->output.fd_d.roi[i].face_boundary.dy =
      frameCtrl->input.mctl_info.display_dim.height * zoom_crp.dy / v4l2_crp->dy;
    for (k = 0; k < 12; k++) {
      if (frame_proc_translate_coordinates(
        (uint16_t *)&frameCtrl->output.fd_d.roi[i].fp.facePt[k].x,
        (uint16_t *)&frameCtrl->output.fd_d.roi[i].fp.facePt[k].y,
        v4l2_crp, &orig_coordinate) <0) {
        CDBG_ERROR("Face Part Coordinates out of bounds");
        continue;
      }
    }
    if(i != num_valid_faces)
      memcpy(&(frameCtrl->output.fd_d.roi[num_valid_faces]),
        &(frameCtrl->output.fd_d.roi[i]),
        sizeof(frame_proc_fd_roi_t));
     num_valid_faces++;
  }
  frameCtrl->output.fd_d.num_faces_detected = num_valid_faces;
  return 1;
}
/*===========================================================================
* FUNCTION    - convert_sp_to_mp -
*
* DESCRIPTION:
*==========================================================================*/
static int convert_sp_to_mp(struct msm_pp_frame * frame,frame_proc_dimension_t *dim,
  frame_plane_type type)
{
  CDBG_FRAME_PROC("%s: frame.sp.cbcr_off %d frame.sp.length %d",__func__,
    frame->sp.cbcr_off, frame->sp.length );
  int rc = 0;
  struct msm_pp_frame_sp sp;
  memcpy(&sp,&(frame->sp), sizeof(struct msm_pp_frame_sp));
  switch (type) {
    case  NUM_PLANE_2:
      frame->num_planes = 2;
      /* fill y plane */
      frame->mp[0].vaddr =  sp.vaddr;
      frame->mp[0].fd = sp.fd;
      frame->mp[0].length = dim->width*dim->height;
      frame->mp[0].data_offset = sp.y_off;
      /* fill cbcr plane */
      frame->mp[1].vaddr = sp.vaddr + sp.cbcr_off;
      frame->mp[1].fd = sp.fd;
      frame->mp[1].length = (dim->width*dim->height)/2;
      frame->mp[1].data_offset = 0;
      break;
    default :
      rc = -1;
      break;
  }
  CDBG_ERROR(" After %s:  mp0lenght %d mp1 length %d",__func__, frame->mp[0].length,frame->mp[1].length );
  return rc;
}
/*===========================================================================
* FUNCTION    - frame_proc_init -
*
* DESCRIPTION:
*==========================================================================*/
static int frame_proc_client_init(uint32_t handle, mctl_ops_t *ops, void *p_init_data)
{
  frame_proc_interface_input_t *init_data = p_init_data;
  handle1 = handle;
  frame_proc_t *frameCtrl = get_frame_proc_client_info(handle);
  if (!frameCtrl)
    return -1;
  memcpy(&(frameCtrl->input), init_data, sizeof(frame_proc_interface_input_t));
  frameCtrl->ops = ops;
  CDBG_FRAME_PROC("In %s\n", __func__);
  pthread_mutex_init(&frame_proc_mutex, NULL);
  pthread_cond_init(&frame_proc_ready_cond, NULL);
  frameCtrl->exit_flag = false;
  frameCtrl->output.afd_d.afd_enable = 0;
  frameCtrl->ignore_snap_frame = -1;
  frameCtrl->output.hjr_d.hjr_enable = 0;
  frameCtrl->output.wd_d.denoise_enable = 0;
  frameCtrl->output.fd_d.fd_enable = FALSE;
#ifdef VFE_2X
  if (afd_init(frameCtrl) != 0) {
    CDBG_ERROR("afd init failed");
    free(frameCtrl);
    return -1;
  }
#endif
  frameCtrl->output.share_d.divert_preview = 0;
  frameCtrl->output.share_d.divert_snapshot = 0;
  return pthread_create(&frame_proc_thread, NULL, frame_processing, (void *)&handle1);
}  /* frame_proc_init */

/*===========================================================================
* FUNCTION    - frame_proc_destroy -
*
* DESCRIPTION:
*==========================================================================*/
static int frame_proc_client_destroy(uint32_t handle)
{
  frame_proc_t *frameCtl = get_frame_proc_client_info(handle);
  frame_proc_obj_t *frame_proc_obj;
  int rc = 0;
  pthread_mutex_lock(&my_frame_proc_struct.mutex);
  if (!frameCtl) {
    pthread_mutex_unlock(&my_frame_proc_struct.mutex);
    return -1;
  }

  int frameprocess_exit = 1;
  frame_proc_obj = frameproc_get_obj(&my_frame_proc_struct, frameCtl);
  if (!frame_proc_obj){
    memset(frameCtl, 0, sizeof(frame_proc_t));
    pthread_mutex_unlock(&my_frame_proc_struct.mutex);
    return 0;
  }
#ifdef VFE_2X
  if (afd_exit(frameCtl)!= 0)
    rc = -1;
#endif
#ifdef MM_CAMERA_FD
  if (frameCtl->output.fd_d.fd_enable) {
    if (face_proc_exit(frameCtl) != 0)
      rc = -1;
  }
#endif
  CDBG_FRAME_PROC( "%s :denoise %d", __func__,
    frameCtl->output.wd_d.denoise_enable);
  if (frameCtl->output.wd_d.denoise_enable) {
    if (wavelet_denoise_exit(frameCtl) != 0)
      rc = -1;
  }
  frameCtl->exit_flag = true;
  CDBG_FRAME_PROC("Sending termination to frame_proc_procesing");
  rc = write(frame_proc_terminate_fd[1],&frameprocess_exit,
    sizeof(frameprocess_exit));
  if (rc <0)
    CDBG_ERROR("frameproc termination  : Failed\n");

  if (pthread_join(frame_proc_thread, NULL) != 0)
    CDBG_ERROR("%s frame_proc_thread exit failure!!!\n", __func__);
  else
    CDBG_FRAME_PROC("%s frame_proc_thread exit sucess\n", __func__);
  close(frame_proc_thread_fd[0]);
  close(frame_proc_thread_fd[1]);
  close( frame_proc_terminate_fd[0]);
  close( frame_proc_terminate_fd[1]);
  pthread_cond_destroy(&frame_proc_ready_cond);
  pthread_mutex_destroy(&frame_proc_mutex);
  frame_proc_obj->refcount--;
  if (!frame_proc_obj->refcount)
    memset(frame_proc_obj, 0, sizeof(frame_proc_obj_t));
  memset(frameCtl, 0, sizeof(frame_proc_t));
  pthread_mutex_unlock(&my_frame_proc_struct.mutex);
  return 0;

  /* TODO:
     CALL exit for each modules
   * 1: Free frame_proc_t structure
   * 2. Free frame_proc_process_t structure */
}  /* frame_proc_destroy */

/*===========================================================================
* FUNCTION    - frame_proc_set_params -
*
* DESCRIPTION:
*==========================================================================*/
int frame_proc_client_set_params(uint32_t handle, int type, void *parm_in, void *parm_out)
{
  int rc = -1;
  frame_proc_set_t *frame_proc_set = parm_in;
  frame_proc_obj_t *frame_proc_obj;
  frame_proc_interface_t *frame_proc_intf = parm_out;
  frame_proc_t *frameCtrl = get_frame_proc_client_info(handle);
  if (!frameCtrl) {
    CDBG_ERROR("%s: no frame proc client",  __func__);
    return rc;
  }

  if (type == FRAME_PROC_ADD_OBJ ) {
    frameproc_client_add_obj(frameCtrl, type, parm_in, parm_out);
    return 0;
  }
  frame_proc_obj = frameproc_get_obj(&my_frame_proc_struct, frameCtrl);
  if (!frame_proc_obj){
    CDBG_ERROR("%s: no frame proc obj",  __func__);
    return rc;
  }
  memcpy(&(frameCtrl->input),&(frame_proc_intf->input), sizeof(frame_proc_interface_input_t));
  frame_proc_interface_output_t *output = &(frame_proc_intf->output);
  rc = TRUE;
  switch (frame_proc_set->type) {
  case FRAME_PROC_FACE_DETECT:
    rc = face_proc_set_params(frameCtrl, &(frame_proc_set->d.set_fd));
    memcpy(&(output->fd_d),&(frameCtrl->output.fd_d),
           sizeof(frame_proc_fd_data_t));
    break;
#ifdef VFE_2X
  case FRAME_PROC_AFD:
    rc = afd_set_params(frameCtrl, &(frame_proc_set->d.set_afd));
    memcpy(&(output->afd_d),&(frameCtrl->output.afd_d),
           sizeof(frame_proc_afd_data_t));
    break;
#endif
  case FRAME_PROC_WAVELET_DENOISE:
    rc = wavelet_denoise_set_params(frameCtrl,  &(frame_proc_set->d.set_wd));
    memcpy(&(output->wd_d),&(frameCtrl->output.wd_d),
           sizeof(frame_proc_wd_data_t));
    break;
  case FRAME_PROC_HJR:
    frameCtrl->output.hjr_d.hjr_enable = frame_proc_set->d.set_hjr.hjr_enable;
    memcpy(&(output->hjr_d),&(frameCtrl->output.hjr_d),
           sizeof(frame_proc_hjr_data_t));
    break;
  case FRAME_PROC_HDR:
    rc = hdr_set_params(frameCtrl, &(frame_proc_set->d.set_hdr));
    memcpy(&(output->hdr_d),&(frameCtrl->output.hdr_d),
           sizeof(frame_proc_hdr_data_t));
    break;
  case FRAME_PROC_SHARE:
    switch (frame_proc_set->d.set_share.type) {
      case FRAME_PROC_SET_STREAM:
        frameCtrl->output.share_d.stream_type = frame_proc_set->d.set_share.d.stream_type;
        break;
      case FRAME_PROC_ADJUST_ZOOM:
        if (frameCtrl->output.fd_d.fd_enable) {
          frame_proc_adjust_fd_coordinates(frameCtrl, &frame_proc_set->d.set_share.d.zoom_crp);
          memcpy(&(output->fd_d),&(frameCtrl->output.fd_d), sizeof(frame_proc_fd_data_t));
        }
      break;
    default:
      break;
    }
    break;
  default:
    CDBG_ERROR("Invalid frameproc Set Param Type");
    return -1;
  }

  if (frameCtrl->output.hjr_d.hjr_enable ||
      frameCtrl->output.wd_d.denoise_enable||
      frameCtrl->output.hdr_d.hdr_enable||
      is_face_register_enabled(frameCtrl))
    frameCtrl->output.share_d.divert_snapshot = 1;
  else
    frameCtrl->output.share_d.divert_snapshot = 0;

  if ((frameCtrl->output.fd_d.fd_enable &&
       (frameCtrl->output.fd_d.fd_mode != FACE_REGISTER)) || (frameCtrl->output.afd_d.afd_enable))
    frameCtrl->output.share_d.divert_preview = 1;
  else
    frameCtrl->output.share_d.divert_preview = 0;

  memcpy(&(output->share_d),&(frameCtrl->output.share_d),
         sizeof(frame_proc_share_data_t));
  return rc;
}  /* frame_proc_set_params */


/*===========================================================================
* FUNCTION    - frame_proc_process -
*
* DESCRIPTION:
*==========================================================================*/
//int frame_proc_process(uint32_t handle, frame_proc_interface_t *frame_proc_intf)
int frame_proc_client_process(uint32_t handle, int event, void *data)
{
  int rc = -1;
  frame_proc_interface_t *frame_proc_intf = data;
  int i,chromaShiftIndicatorHori = 0,chromaShiftIndicatorVert = 0;
  frame_proc_t *frameCtrl = get_frame_proc_client_info(handle);
  frame_proc_obj_t *frame_proc_obj =
    frameproc_get_obj(&my_frame_proc_struct, frameCtrl);
  frame_proc_parms frameproc;
  frameproc.handle = handle;
  frameproc.frame_proc_intf = frame_proc_intf;
  if (!frameCtrl || !frame_proc_intf || !frame_proc_obj) {
    CDBG_ERROR("frameCtrl = frame_proc_intf NULL");
    return rc;
  }
  memcpy(&(frameCtrl->input),&(frame_proc_intf->input),
    sizeof(frame_proc_interface_input_t));
  if (frameCtrl->input.mctl_info.main_img_format == FRAME_PROC_H2V2) {
    chromaShiftIndicatorHori = 1;
    chromaShiftIndicatorVert = 1;
  } else if (frameCtrl->input.mctl_info.main_img_format == FRAME_PROC_H2V1) {
    chromaShiftIndicatorHori = 1;
    chromaShiftIndicatorVert = 0;
  } else if (frameCtrl->input.mctl_info.main_img_format == FRAME_PROC_H1V2) {
    chromaShiftIndicatorHori = 0;
    chromaShiftIndicatorVert = 1;
  } else if (frameCtrl->input.mctl_info.main_img_format == FRAME_PROC_H1V1) {
    chromaShiftIndicatorHori = 0;
    chromaShiftIndicatorVert = 0;
  }
  CDBG_ERROR("frameproc opt_mode %d\n", frameCtrl->input.mctl_info.opt_mode);
  switch (frameCtrl->input.mctl_info.opt_mode) {
    case FRAME_PROC_SNAPSHOT:
      /* For MAIN IMG */
      for (i=0; i<frameCtrl->input.mctl_info.num_main_img; i++) {
        if (frameCtrl->input.mctl_info.main_img_frame[i].num_planes >2) {
          CDBG_ERROR("%s: Error Snapshot Frame proc doesnt support num_planes >2",
            __func__);
          return -1;
        }
        if (frameCtrl->input.mctl_info.main_img_frame[i].num_planes == 1) {
          rc = convert_sp_to_mp(&(frameCtrl->input.mctl_info.main_img_frame[i]),
            &(frameCtrl->input.mctl_info.picture_dim),
            NUM_PLANE_2);
          if (rc<0)
            return -1;
        }
        /* Error Checking */
        if ((int)(frameCtrl->input.mctl_info.main_img_frame[i].mp[0].length -
          frameCtrl->input.mctl_info.main_img_frame[i].mp[0].data_offset) !=
          frameCtrl->input.mctl_info.picture_dim.width*
          CEILING16(frameCtrl->input.mctl_info.picture_dim.height)) {
          CDBG_ERROR("Error: Snapshot length for Y buffer doesnt match preview dimensions %d %d",
            frameCtrl->input.mctl_info.main_img_frame[i].mp[0].length,
            frameCtrl->input.mctl_info.picture_dim.width*
            CEILING16(frameCtrl->input.mctl_info.picture_dim.height));
          return -1;
        }
        if ((frameCtrl->input.mctl_info.main_img_frame[i].mp[1].length -
          frameCtrl->input.mctl_info.main_img_frame[i].mp[1].data_offset) != (uint32_t)
          ((frameCtrl->input.mctl_info.picture_dim.width>>chromaShiftIndicatorHori)*
          (CEILING16(frameCtrl->input.mctl_info.picture_dim.height)>>
          chromaShiftIndicatorVert)*2)) {
          CDBG_ERROR("Error: Snapshot length for CbCr buffer doesnt match preview dimensions %d %d",
            frameCtrl->input.mctl_info.main_img_frame[i].mp[1].length,
            ((frameCtrl->input.mctl_info.picture_dim.width>>chromaShiftIndicatorHori)*
            (CEILING16(frameCtrl->input.mctl_info.picture_dim.height)>>
            chromaShiftIndicatorVert))*2);
          return -1;
        }
        for (i=0; i<frameCtrl->input.mctl_info.main_img_frame[i].num_planes; i++) {
          CDBG_FRAME_PROC("Error: frame_proc_process(snapshot)!!! buf=0x%lu fd=%d y_off=%d\n",
            frameCtrl->input.mctl_info.main_img_frame[i].mp[i].vaddr,
            frameCtrl->input.mctl_info.main_img_frame[i].mp[i].fd,
            frameCtrl->input.mctl_info.main_img_frame[i].mp[i].data_offset);
        }
      }
      /* THUMB IMG */
      for (i=0; i<frameCtrl->input.mctl_info.num_thumb_img; i++) {
        if (frameCtrl->input.mctl_info.thumb_img_frame[i].num_planes >2) {
          CDBG_ERROR("%s: Error Snapshot Frame proc doesnt support num_planes >2",
            __func__);
          return -1;
        }
        if (frameCtrl->input.mctl_info.thumb_img_frame[i].num_planes == 1) {
          rc = convert_sp_to_mp(&(frameCtrl->input.mctl_info.thumb_img_frame[i]),
            &(frameCtrl->input.mctl_info.thumbnail_dim),
            NUM_PLANE_2);
          if (rc<0)
            return -1;
        }
      }
      break;
    case FRAME_PROC_PREVIEW:
      if (frameCtrl->input.mctl_info.frame.num_planes == 1) {
        rc = convert_sp_to_mp(&(frameCtrl->input.mctl_info.frame),
          &(frameCtrl->input.mctl_info.display_dim), NUM_PLANE_2);
        if (rc<0)
          return -1;
      }
      if (frameCtrl->input.mctl_info.frame.num_planes != 2) {
        CDBG_ERROR("%s: Preview Frame proc doesn support num_planes >2",__func__);
        return -1;
      }
      if ((int)(frameCtrl->input.mctl_info.frame.mp[0].length -
        frameCtrl->input.mctl_info.frame.mp[0].data_offset) !=
        frameCtrl->input.mctl_info.display_dim.width*frameCtrl->input.mctl_info.display_dim.height) {
        CDBG_ERROR("Preview length for Y buffer doesnt match preview dimensions %d %d",frameCtrl->input.mctl_info.frame.mp[0].length,
          frameCtrl->input.mctl_info.display_dim.width*frameCtrl->input.mctl_info.display_dim.height);
        return -1;
      }
      if ((frameCtrl->input.mctl_info.frame.mp[1].length -
        frameCtrl->input.mctl_info.frame.mp[1].data_offset) !=
        (uint32_t)(frameCtrl->input.mctl_info.display_dim.width*
        frameCtrl->input.mctl_info.display_dim.height)/2) {
        CDBG_ERROR("Preview length for CbCr buffer doesnt match preview dimensions %d %d",frameCtrl->input.mctl_info.frame.mp[1].length,
          (frameCtrl->input.mctl_info.display_dim.width*
          frameCtrl->input.mctl_info.display_dim.height)/2);
        return -1;
      }
      for (i=0; i<frameCtrl->input.mctl_info.frame.num_planes; i++) {
        CDBG_FRAME_PROC("frame_proc_process(preview)!!! buf=0x%lu fd=%d y_off=%d\n",
          frameCtrl->input.mctl_info.frame.mp[i].vaddr,
          frameCtrl->input.mctl_info.frame.mp[i].fd,
          frameCtrl->input.mctl_info.frame.mp[i].data_offset);
      }
      break;
    default:
      CDBG_ERROR("%s: Invalid Operation Mode!!!\n",__func__);
      return rc;
      break;
  }
  if (frameCtrl->output.hjr_d.hjr_enable ||
      frameCtrl->output.afd_d.afd_enable ||
    frameCtrl->output.fd_d.fd_enable ||
    frameCtrl->output.wd_d.denoise_enable ||
    frameCtrl->output.hdr_d.hdr_enable||
    is_face_register_enabled(frameCtrl)) {
    /* TO REMOVE flag */
    pthread_mutex_lock(&frame_proc_mutex);
    flag = 0;
    pthread_mutex_unlock(&frame_proc_mutex);
    rc = write(frame_proc_thread_fd[1],&frameproc, sizeof(frame_proc_parms));
    pthread_mutex_lock(&frame_proc_mutex);
    if (!flag)
      pthread_cond_wait(&frame_proc_ready_cond, &frame_proc_mutex);
    pthread_mutex_unlock(&frame_proc_mutex);
    if (frameCtrl->output.hjr_d.hjr_enable ||
      frameCtrl->output.wd_d.denoise_enable ||
      frameCtrl->output.hdr_d.hdr_enable||
      is_face_register_enabled(frameCtrl))
      frameCtrl->output.share_d.divert_snapshot = 1;
    else
      frameCtrl->output.share_d.divert_snapshot = 0;

    if ((frameCtrl->output.fd_d.fd_enable &&
	(frameCtrl->output.fd_d.fd_mode != FACE_REGISTER)) || (frameCtrl->output.afd_d.afd_enable))
      frameCtrl->output.share_d.divert_preview = 1;
    else
      frameCtrl->output.share_d.divert_preview = 0;

    memcpy(&(frame_proc_intf->output.share_d), &(frameCtrl->output.share_d),
      sizeof(frame_proc_share_data_t));

    CDBG_FRAME_PROC("%s: X",__func__);
  } else {
    CDBG_ERROR("DO NOT DISTURB frameprocESSING IF MODULES are not ENABLED");
  }
  return 0;
}  /* frame_proc_process */

/*===========================================================================
* FUNCTION    - frame_processing -
*
* DESCRIPTION:
*==========================================================================*/
static void *frame_processing(void *parm)
{

  fd_set frame_proc_fds;
  frame_proc_parms frame_proc_parm;
  uint32_t handle = *(uint32_t *)parm;
  frame_proc_t *frameCtrl = get_frame_proc_client_info(handle);
  if (!frameCtrl) {
    CDBG_ERROR(" %s: frameCtrl is NULL .. Exiting",__func__);
    return NULL;
  }
  int rc;
  int max_frame_proc_fds = 0;
  int frameprocess_exit = 0;
  memset(&frame_proc_parm, 0x0, sizeof(frame_proc_parms));
  if (pipe(frame_proc_terminate_fd)< 0) {
    CDBG_ERROR("%s: thread termination pipe creation for frame_proc_terminate_fd failed\n",
      __func__);
    return NULL;
  }
  if (pipe(frame_proc_thread_fd)< 0) {
    CDBG_ERROR("%s: thread termination pipe creation for frame_proc_thread_fd failed\n",
      __func__);
    return NULL;
  }
  CDBG_FRAME_PROC("In %s E\n", __func__);
  if (frameCtrl->exit_flag) {
    CDBG_HIGH("Terminating post processing thread");
    return NULL;
  }
  do {
    FD_ZERO(&frame_proc_fds);
    FD_SET(frame_proc_terminate_fd[0], &frame_proc_fds);
    FD_SET(frame_proc_thread_fd[0], &frame_proc_fds);
    max_frame_proc_fds = MAX(frame_proc_thread_fd[0], frame_proc_terminate_fd[0])+ 1;
    struct timeval frame_proc_timeout;
    frame_proc_timeout.tv_usec = 0;
    frame_proc_timeout.tv_sec = 6;

    rc = select(max_frame_proc_fds, &frame_proc_fds, NULL, NULL, &frame_proc_timeout);
    if (0 == rc) {
      CDBG_FRAME_PROC("...pp thread select timeout...\n");
      continue;
    } else if (rc < 0) {
      CDBG_ERROR("%s: SELECT ERROR %s \n", __func__, strerror(errno));
      usleep(1000 * 10);
      continue;
    }
    if (FD_ISSET(frame_proc_terminate_fd[0], &frame_proc_fds)) {
      CDBG_FRAME_PROC("%s: Received msg from frame_proc_terminate/ exit thread.\n", __func__);
      rc = read(frame_proc_terminate_fd[0],&frameprocess_exit, sizeof(frameprocess_exit));
      if (rc < 0) {
        CDBG_ERROR("%s: Cannot read from frame_proc_exit thread\n", __func__);
        frameprocess_exit = 0;
        continue;
      }
      break;
    }
    if (frameprocess_exit) {
      /*  T DO  should the frame be released */
      break;
    }
    if (FD_ISSET(frame_proc_thread_fd[0], &frame_proc_fds)) {
      rc = read(frame_proc_thread_fd[0],&frame_proc_parm, sizeof(frame_proc_parms));
      CDBG_FRAME_PROC("%s: Received msg from frame_proc_process.\n", __func__);
      if (rc < 0) {
        CDBG_ERROR("%s: Cannot read from frame_proc_process thread\n", __func__);
        continue;
      }
      handle = frame_proc_parm.handle;
      frameCtrl = get_frame_proc_client_info(handle);
      if (!frameCtrl) {
        CDBG_FRAME_PROC("%s: frameCtrl is NULL.. returning", __func__);
        return FALSE;
      }

      switch (frameCtrl->input.mctl_info.opt_mode) {
        case FRAME_PROC_SNAPSHOT:
          snapshot_frameprocess(frameCtrl);
          break;

        case FRAME_PROC_PREVIEW:
          preview_frameprocess(frameCtrl);
          break;

        default:
          CDBG_ERROR("Invalid Operation Mode!!!\n");
          break;
      }
    }
    CDBG("afd_antibanding_type: %d\n",
             frameCtrl->output.afd_d.afd_antibanding_type);
    /* DO MEDIA CONTROLLER CALLBACK */
    memcpy(&(frame_proc_parm.frame_proc_intf->output),&(frameCtrl->output),
      sizeof(frame_proc_interface_output_t));
    pthread_mutex_lock(&frame_proc_mutex);
    flag = 1;
    pthread_cond_signal(&frame_proc_ready_cond);
    pthread_mutex_unlock(&frame_proc_mutex);
  } while (!frameprocess_exit);

  CDBG_FRAME_PROC("In %s X\n", __func__);
  return NULL;
}  /* frame_processing */
/*===========================================================================
* FUNCTION    - preview_frameprocess -
*
* DESCRIPTION:
*==========================================================================*/
static void preview_frameprocess( frame_proc_t *frameCtrl)
{
#ifdef VFE_2X
  /**AFD***/
  if (frameCtrl->output.afd_d.afd_enable) {
    CDBG("preview_frameprocess: frame_proc_afd_execute\n");
    frame_proc_afd_execute(frameCtrl);
    CDBG("preview_frameprocess: frame_proc_afd_execute - x\n");
  }
#endif
  /**FD**/
#ifdef MM_CAMERA_FD
  if (frameCtrl->output.fd_d.fd_enable) {
    face_proc_execute(frameCtrl);
  }
#endif


}  /* preview_frameprocess */
/*===========================================================================
* FUNCTION    - snapshot_frameprocess -
*
* DESCRIPTION:
*==========================================================================*/
static void snapshot_frameprocess( frame_proc_t *frameCtrl)
{
  int process_mode = WAVELET_DENOISE_YCBCR_PLANE;
  frameCtrl->ignore_snap_frame = -1;
  /****HJR*****/
  if (frameCtrl->output.hjr_d.hjr_enable) {
    hjr_execute(frameCtrl);
  }  /* end of HJR*/
  /**** HDR ***/
  if (frameCtrl->output.hdr_d.hdr_enable) {
    CDBG_ERROR("%s Executing HDR",__func__);
    hdr_execute(frameCtrl);
    process_mode = frameCtrl->output.wd_d.process_mode;
    frameCtrl->output.wd_d.process_mode = WAVELET_DENOISE_CBCR_ONLY;
    if (frameCtrl->input.mctl_info.num_main_img == 3)
      frameCtrl->ignore_snap_frame = 0;
  }
  if (frameCtrl->output.wd_d.denoise_enable) {
    wavelet_denoise_execute(frameCtrl);
  }
  frameCtrl->output.wd_d.process_mode = process_mode;
  if (is_face_register_enabled(frameCtrl))
    face_proc_execute(frameCtrl);

}  /* snaphot_frameprocess */

/*============================================================================
 * FUNCTION    - FRAME_PROC_interface_create -
 *
 * DESCRIPTION:
 *===========================================================================*/
uint32_t FRAME_PROC_client_open(module_ops_t *ops)
{
  uint32_t handle = 0;
  int idx;
  frame_proc_t *frame_client = NULL;

  if (!ops) {
    CDBG_ERROR("%s: null ops pointer",  __func__);
    return 0;
  }
  memset(ops,  0,  sizeof(module_ops_t));

  pthread_mutex_lock(&my_frame_proc_struct.mutex);
  for (idx = 0; idx < FRAMEPROC_MAX_CLIENT_NUM; idx++) {
    if (my_frame_proc_struct.client[idx].handle == 0) {
      frame_client = &my_frame_proc_struct.client[idx];
      break;
    }
  }
  /* if not found return null */
  if (!frame_client) {
    pthread_mutex_unlock(&my_frame_proc_struct.mutex);
    return(uint32_t)NULL;
  } else {
    memset(frame_client, 0, sizeof(frame_proc_t));
    frame_client->obj_idx_mask = 0;
    frame_client->client_idx = idx;
    frame_client->my_comp_id = MCTL_COMPID_FRAMEPROC;
    frame_client->handle = frame_proc_gen_handle((uint8_t)idx);
    ops->handle = (uint32_t)frame_client->handle;
    ops->init = frame_proc_client_init;
    ops->set_params = frame_proc_client_set_params;
    ops->get_params = NULL;
    ops->process = frame_proc_client_process;
    ops->abort = NULL;
    ops->destroy= frame_proc_client_destroy;
  }
  pthread_mutex_unlock(&my_frame_proc_struct.mutex);
  CDBG_FRAME_PROC("%s: client_idx = %d, handle = 0x%x",
    __func__, idx, frame_client->handle);
  return frame_client->handle;
}  /*frame_interface_create*/

/*===========================================================================
 * FUNCTION    - FRAME_PROC_comp_create -
 *
 * DESCRIPTION: initialize FRAME_PROC component.
 *==========================================================================*/
int FRAME_PROC_comp_create()
{
  memset(&my_frame_proc_struct, 0, sizeof(my_frame_proc_struct));
  pthread_mutex_init(&my_frame_proc_struct.mutex, NULL);
  CDBG_FRAME_PROC("%s: FRAME_PROC_comp_create success", __func__);

  return 0;
}
/*===========================================================================
 * FUNCTION    - FRAME_PROC_comp_destroy -
 *
 * DESCRIPTION: destroy the FRAME_PROC component
 *==========================================================================*/
int FRAME_PROC_comp_destroy()
{
  pthread_mutex_destroy(&my_frame_proc_struct.mutex);
  memset(&my_frame_proc_struct, 0, sizeof(my_frame_proc_struct));
  CDBG_FRAME_PROC("%s: FRAME_PROC_comp_destroy success", __func__);
  return 0;
}
