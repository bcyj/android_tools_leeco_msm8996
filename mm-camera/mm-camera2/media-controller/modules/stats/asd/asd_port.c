/* asd_port.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "asd_port.h"
#include "asd_thread.h"
#include "asd.h"
#include "modules.h"
#include "stats_event.h"

#undef LOG_TAG
#define LOG_TAG "ASD"

/** _asd_port_private
 *    @asd_object: session index
 *    @port:       stream index
 *
 * Each asd module object should be used ONLY for one Bayer
 * session/stream set - use this structure to store session
 * and stream indices information.
 **/
typedef struct _asd_port_private {
  unsigned int      reserved_id;
  cam_stream_type_t stream_type;
  unsigned int      state;
  asd_object_t      asd_object;
  asd_thread_data_t *thread_data;
  mct_stream_info_t stream_info;
  char              asd_debug_data_array[ASD_DEBUG_DATA_SIZE];
  uint32_t          asd_debug_data_size;
} asd_port_private_t;

static void asd_port_send_decision_event(mct_port_t *port,
                                         cam_auto_scene_t scene)
{

  CDBG("Send ASD decision to MCT");
  boolean rc = TRUE;
  mct_event_t event;
  mct_bus_msg_t bus_msg;
  mct_bus_msg_asd_decision_t asd_msg;
  asd_msg.scene = scene;

  asd_port_private_t *asd_port_private = (asd_port_private_t *)(port->port_private);

  /* pack the message and post to bus */
  bus_msg.sessionid = (asd_port_private->reserved_id >> 16);
  bus_msg.type = MCT_BUS_MSG_AUTO_SCENE_DECISION;
  bus_msg.msg = (void *)&asd_msg;

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = asd_port_private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);
  rc = MCT_PORT_EVENT_FUNC(port)(port, &event);
}


/** asd_port_send_exif_debug_data:
 * Return nothing
 **/
static void asd_port_send_exif_debug_data(mct_port_t *port)
{
  mct_event_t          event;
  mct_bus_msg_t        bus_msg;
  cam_asd_exif_debug_t *asd_info;
  asd_port_private_t   *private;
  int                  size;

  if (!port) {
    CDBG_ERROR("%s: input error", __func__);
    return;
  }
  private = (asd_port_private_t *)(port->port_private);
  if (private == NULL) {
    return;
  }

  /* Send exif data if data size is valid */
  if (!private->asd_debug_data_size) {
    CDBG("asd_port: Debug data not available");
    return;
  }
  asd_info = (cam_asd_exif_debug_t *) malloc(sizeof(cam_asd_exif_debug_t));
  if (!asd_info) {
    CDBG_ERROR("Failure allocating memory for debug data");
    return;
  }
  bus_msg.sessionid = (private->reserved_id >> 16);
  bus_msg.type = MCT_BUS_MSG_ASD_EXIF_DEBUG_INFO;
  bus_msg.msg = (void *)asd_info;
  size = (int)sizeof(cam_asd_exif_debug_t);
  bus_msg.size = size;
  memset(asd_info, 0, size);
  asd_info->asd_debug_data_size = private->asd_debug_data_size;

  CDBG("%s asd_debug_data_size: %d", __func__, private->asd_debug_data_size);
  memcpy(&(asd_info->asd_private_debug_data[0]), private->asd_debug_data_array,
    private->asd_debug_data_size);
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);
  MCT_PORT_EVENT_FUNC(port)(port, &event);
  if (asd_info) {
    free(asd_info);
  }
}


/** asd_port_callback
 *
 **/
static void asd_port_callback(asd_output_data_t *output, void *p)
{
  mct_port_t         *port    = (mct_port_t *)p;
  asd_port_private_t *private;
  mct_event_t        event;
  stats_update_t     stats_update;

  mct_bus_msg_t bus_msg;
  mct_bus_msg_asd_hdr_status_t asd_msg;
  asd_thread_data_t *thread_data;
  int ui_update_count_new;

  if (!output || !port) {
      return;
  }
  private = (asd_port_private_t *)(port->port_private);
  thread_data = (asd_thread_data_t *)(private->thread_data);

  if (ASD_UPDATE == output->type) {
    stats_update.asd_update.asd_enable = output->asd_enable;
    stats_update.asd_update.histo_backlight_detected =
      output->histo_backlight_detected;
    stats_update.asd_update.landscape_severity = output->landscape_severity;
    stats_update.asd_update.backlight_detected = output->backlight_detected;
    stats_update.asd_update.backlight_luma_target_offset =
      output->backlight_luma_target_offset;
    stats_update.asd_update.snow_or_cloudy_scene_detected =
      output->snow_or_cloudy_scene_detected;
    stats_update.asd_update.snow_or_cloudy_luma_target_offset =
      output->snow_or_cloudy_luma_target_offset;
    stats_update.asd_update.landscape_severity = output->landscape_severity;
    stats_update.asd_update.backlight_scene_severity =
      output->backlight_scene_severity;
    stats_update.asd_update.portrait_severity = output->portrait_severity;
    stats_update.asd_update.asd_soft_focus_dgr = output->soft_focus_dgr;

    stats_update.asd_update.mixed_light = output->mixed_light;

    //copy scene and the array for the stats update message
    stats_update.asd_update.scene = output->scene;

    //is severity really needed?
    uint32_t *severity = &(stats_update.asd_update.severity[0]);
    size_t i;
    for (i = 0; i < S_MAX; i++) {
      severity[i] = output->severity[i];
    }

    stats_update.asd_update.is_hdr_scene = output->is_hdr_scene;
    stats_update.asd_update.hdr_confidence = output->hdr_confidence;

    stats_update.flag       = STATS_UPDATE_ASD;

    /* pack into a mct_event object*/
    event.direction = MCT_EVENT_UPSTREAM;
    event.identity  = private->reserved_id;
    event.type      = MCT_EVENT_MODULE_EVENT;

    event.u.module_event.type              = MCT_EVENT_MODULE_STATS_ASD_UPDATE;
    event.u.module_event.module_event_data = (void *)(&stats_update);

    MCT_PORT_EVENT_FUNC(port)(port, &event);

    if (ASD_UI_UPDATE_FRAME_INTERVAL > 0) {
      ui_update_count_new = (int)thread_data->process_data.frame_count /
        ASD_UI_UPDATE_FRAME_INTERVAL;
    } else {
      ui_update_count_new = (int)thread_data->process_data.frame_count;
    }
    if (ui_update_count_new > (int)thread_data->process_data.ui_update_count) {
      //send a bus update message so that HAL can be notified of
      //the new scene decision
      asd_port_send_decision_event(port,
                                   output->scene);
      thread_data->process_data.ui_update_count = ui_update_count_new;
    }

    /* Save the debug data in private data struct to be sent out later */
    CDBG("%s: Save debug data in port private", __func__);
    private->asd_debug_data_size = output->asd_debug_data_size;
    if (output->asd_debug_data_size) {
      memcpy(private->asd_debug_data_array, output->asd_debug_data_array,
        output->asd_debug_data_size);
    }
  } else if (ASD_SEND_EVENT == output->type) {
   /* This will not be invoked from asd thread.
      Keeping the output->type to keep the interface same as other modules */
  }

  /* Send Bus message */

  /* Update ASD bus message data */
  asd_msg.is_hdr_scene = output->is_hdr_scene;
  asd_msg.hdr_confidence = output->hdr_confidence;

  /* pack the message and post to bus */
  bus_msg.sessionid = (private->reserved_id >> 16);
  bus_msg.type = MCT_BUS_MSG_ASD_HDR_SCENE_STATUS;
  bus_msg.msg = (void *)&asd_msg;

  /* pack into an mct_event object */
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);

  MCT_PORT_EVENT_FUNC(port)(port, &event);

  return;
}

/** asd_port_start_threads
 *    @port:
 **/
static boolean asd_port_init_threads(mct_port_t *port)
{
  boolean     rc = FALSE;
  asd_port_private_t *private = port->port_private;

  private->thread_data = asd_thread_init();
  CDBG("%s private->thread_data: %p", __func__, private->thread_data);
  if (private->thread_data != NULL) {
    rc = TRUE;
    private->thread_data->asd_port = port;
    private->thread_data->asd_obj = &private->asd_object;
    private->thread_data->asd_cb = asd_port_callback;
  } else {
    CDBG_ERROR("%s private->thread_data is NULL", __func__);
  }
  return rc;
}

/** asd_port_start_threads
 *    @port:
 **/
static boolean asd_port_start_threads(mct_port_t *port)
{
  boolean     rc = FALSE;
  asd_port_private_t *private = port->port_private;

  if (private->thread_data != NULL) {
    CDBG("%s: Start asd thread", __func__);
    rc = asd_thread_start(private->thread_data);
    if (rc == FALSE) {
      asd_thread_deinit(private->thread_data);
    }
  }
  return rc;
}

/** asd_port_check_session_id
 *    @d1: session+stream identity
 *    @d2: session+stream identity
 *
 *  To find out if both identities are matching;
 *  Return TRUE if matches.
 **/
static boolean asd_port_check_session_id(void *d1, void *d2)
{
  unsigned int v1, v2;
  v1 = *((unsigned int *)d1);
  v2 = *((unsigned int *)d2);

  return (((v1 & 0xFFFF0000) ==
          (v2 & 0xFFFF0000)) ? TRUE : FALSE);
}

static boolean asd_port_event_stats_data( asd_port_private_t *port_private,
                               mct_event_t *event)
{
  boolean rc = TRUE;
  mct_event_module_t *mod_evt = &(event->u.module_event);
  mct_event_stats_isp_t *stats_event ;
  stats_t  *asd_stats = &(port_private->asd_object.stats);

  stats_event =(mct_event_stats_isp_t *)(mod_evt->module_event_data);
  asd_stats->stats_type_mask = 0; /*clear the mask*/
  if (stats_event) {
    if (stats_event->stats_mask) {
      CDBG("%s: hist received stats of mask: %d", __func__,
           stats_event->stats_mask);
      if (stats_event->stats_mask & (1 << MSM_ISP_STATS_IHIST)) {
        CDBG("%s: IHISTO Stats!", __func__);
        asd_stats->stats_type_mask += STATS_IHISTO;
        memcpy(&asd_stats->yuv_stats.histogram,
               stats_event->stats_data[MSM_ISP_STATS_IHIST].stats_buf,
               sizeof(q3a_ihist_stats_t));
      }

      if (stats_event->stats_mask & (1 << MSM_ISP_STATS_BHIST)) {
        CDBG("%s: BHISTO Stats!", __func__);
        asd_stats->stats_type_mask += STATS_BHISTO;
        memcpy(&asd_stats->bayer_stats.q3a_bhist_stats,
               stats_event->stats_data[MSM_ISP_STATS_BHIST].stats_buf,
               sizeof(q3a_bhist_stats_t));
      }

      asd_thread_msg_t * asd_msg = (asd_thread_msg_t *)
        malloc(sizeof(asd_thread_msg_t));

      if (asd_msg) {
        memset(asd_msg, 0, sizeof(asd_thread_msg_t));
        asd_msg->type = MSG_ASD_STATS;
        asd_msg->u.stats = asd_stats;
        rc = asd_thread_en_q_msg(port_private->thread_data, asd_msg);
      }
    }
  }
  return rc;
}


/** asd_port_proc_downstream_event:
 *
 **/
static boolean asd_port_proc_downstream_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = TRUE;

  asd_port_private_t *private = (asd_port_private_t *)(port->port_private);
  mct_event_module_t *mod_evt = &(event->u.module_event);

  CDBG("%s: Received module event of type: %d", __func__, mod_evt->type);
  switch (mod_evt->type) {
  case MCT_EVENT_MODULE_SET_RELOAD_CHROMATIX:
  case MCT_EVENT_MODULE_SET_CHROMATIX_PTR: {
    modulesChromatix_t *mod_chrom =
      (modulesChromatix_t *)mod_evt->module_event_data;
    asd_thread_msg_t *asd_msg   =
      (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
    if (asd_msg) {
      CDBG("%s: MSG_ASD_SET: Set chromatix for ASD!", __func__);
      memset(asd_msg, 0, sizeof(asd_thread_msg_t));
      asd_msg->type = MSG_ASD_SET;

      asd_msg->u.asd_set_parm.type = ASD_SET_PARAM_INIT_CHROMATIX;
      asd_msg->u.asd_set_parm.u.init_param.chromatix = mod_chrom->chromatixPtr;
      asd_msg->u.asd_set_parm.u.init_param.comm_chromatix = mod_chrom->chromatixComPtr;

      rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
    }
  } /* case MCT_EVENT_MODULE_SET_CHROMATIX_PTR */
    break;

  case MCT_EVENT_MODULE_STATS_DATA: {
    CDBG("%s: Handle Stats Event!", __func__);
    rc = asd_port_event_stats_data(private, event);
  } /* case MCT_EVENT_MODULE_STATS_DATA */
    break;
  case MCT_EVENT_MODULE_START_STOP_STATS_THREADS: {
    uint8_t *start_flag = (uint8_t*)(mod_evt->module_event_data);
    CDBG("%s MCT_EVENT_MODULE_START_STOP_STATS_THREADS start_flag: %d",
      __func__,*start_flag);

    if (*start_flag) {
      if (asd_port_start_threads(port) == FALSE) {
        CDBG("%s: asd thread start failed", __func__);
        rc = FALSE;
      }
    } else {
      if (private->thread_data) {
        asd_thread_stop(private->thread_data);
      }
    }
  }
    break;
  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
    stats_update_t *stats_update =
      (stats_update_t *)mod_evt->module_event_data;

    if (!stats_update || stats_update->flag != STATS_UPDATE_AEC) {
      rc = FALSE;
      break;
    }

    asd_thread_msg_t *asd_msg =
      (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
    if (asd_msg) {
      memset(asd_msg, 0, sizeof(asd_thread_msg_t));
      asd_msg->type = MSG_AEC_DATA;

      asd_msg->u.aec_data.num_of_regions
         = stats_update->aec_update.numRegions;
      asd_msg->u.aec_data.pixels_per_region =
        stats_update->aec_update.pixelsPerRegion;
      asd_msg->u.aec_data.target_luma = stats_update->aec_update.target_luma;
      asd_msg->u.aec_data.comp_luma = stats_update->aec_update.comp_luma;
      asd_msg->u.aec_data.exp_index = stats_update->aec_update.exp_index;
      asd_msg->u.aec_data.exp_tbl_val = stats_update->aec_update.exp_tbl_val;
      asd_msg->u.aec_data.extreme_green_cnt =
        stats_update->aec_update.asd_extreme_green_cnt;
      asd_msg->u.aec_data.extreme_blue_cnt =
        stats_update->aec_update.asd_extreme_blue_cnt;
      asd_msg->u.aec_data.extreme_tot_regions =
        stats_update->aec_update.asd_extreme_tot_regions;
      asd_msg->u.aec_data.lux_idx = stats_update->aec_update.lux_idx;
      memcpy(asd_msg->u.aec_data.SY, stats_update->aec_update.SY,
        sizeof(unsigned int) * ASD_AEC_STATS_NUM_MAX);
      /* Currently not updated by AEC */
  #if 0
      asd_msg->u.aec_data.roi_info.type = stats_update->aec_update.type;
      asd_msg->u.aec_data.roi_info.roi_updated =
        stats_update->aec_update.roi_updated;
      asd_msg->u.aec_data.roi_info.frm_width =
        stats_update->aec_update.frm_width;
      asd_msg->u.aec_data.roi_info.frm_height =
        stats_update->aec_update.frm_height;
      asd_msg->u.aec_data.roi_info.num_roi = stats_update->aec_update.num_roi;
      memcpy(asd_msg->u.aec_data.roi_info.roi, stats_update->aec_update.roi,
         sizeof(stats_roi_t) * MAX_ROI);
  #endif

      rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
    }
  }
  break;

  case MCT_EVENT_MODULE_STATS_AWB_UPDATE: {
    stats_update_t *stats_update =
      (stats_update_t *)mod_evt->module_event_data;

    if (!stats_update || stats_update->flag != STATS_UPDATE_AWB) {
      rc = FALSE;
      break;
    }

    asd_thread_msg_t *asd_msg =
      (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
    if (asd_msg) {
      memset(asd_msg, 0, sizeof(asd_thread_msg_t));
      asd_msg->type = MSG_AWB_DATA;

      memcpy(asd_msg->u.awb_data.stat_sample_decision,
        stats_update->awb_update.sample_decision, sizeof(int) * 64);

      asd_msg->u.awb_data.grey_world_stats =
        stats_update->awb_update.grey_world_stats;

      rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
    }
  }
    break;
  case MCT_EVENT_MODULE_FACE_INFO: {
    asd_thread_msg_t *asd_msg =
      (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
    if (asd_msg) {
      memset(asd_msg, 0, sizeof(asd_thread_msg_t));
      asd_msg->type = MSG_FACE_INFO;
      mct_face_info_t * info = (mct_face_info_t *)mod_evt->module_event_data;
      asd_data_face_info_t * face_info = &(asd_msg->u.face_data);
      size_t i;

      face_info->face_count =
        info->face_count > MAX_ROI ? MAX_ROI : info->face_count;

      for (i = 0; i < face_info->face_count; i++) {
        face_info->faces[i].roi = info->faces[i].roi;
        face_info->faces[i].score = info->faces[i].score;
      }

      rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
    }
  }
    break;
  case MCT_EVENT_MODULE_SOF_NOTIFY: {
    CDBG("%s Received SOF event", __func__);

    asd_thread_msg_t *asd_msg =
      (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
    if (asd_msg) {
      memset(asd_msg, 0, sizeof(asd_thread_msg_t));
      asd_msg->type = MSG_SOF;
      //for now, don't care about timestamp or frame_id
      rc = asd_thread_en_q_msg(private->thread_data, asd_msg);

      /* Send the ASD debug data from here in SoF context */
      CDBG("%s: Send exif info update with debug data", __func__);
      asd_port_send_exif_debug_data(port);
    }
  }
    break;
  case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
    asd_thread_msg_t *asd_msg = NULL;
    if (private->stream_type == CAM_STREAM_TYPE_PREVIEW) {
      asd_msg = (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
      if (asd_msg) {
        memset(asd_msg, 0, sizeof(asd_thread_msg_t));
        asd_msg->type = MSG_ASD_SET;
        asd_set_parameter_t *param = &asd_msg->u.asd_set_parm;
        param->type = ASD_SET_UI_FRAME_DIM;
        param->u.init_param.preview_width = private->stream_info.dim.width;
        param->u.init_param.preview_height = private->stream_info.dim.height;

        rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
      }
    }
    /* send  op mode to ASD algorithm */
    asd_msg = (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
    if (asd_msg) {
      memset(asd_msg, 0, sizeof(asd_thread_msg_t));
      asd_msg->type = MSG_ASD_SET;
      asd_set_parameter_t *param = &asd_msg->u.asd_set_parm;
      param->type = ASD_SET_PARAM_OP_MODE;

      switch (private->stream_type) {
        case CAM_STREAM_TYPE_VIDEO: {
           param->u.init_param.op_mode = Q3A_OPERATION_MODE_CAMCORDER;
        }
          break;

        case CAM_STREAM_TYPE_PREVIEW: {
          param->u.init_param.op_mode = Q3A_OPERATION_MODE_PREVIEW;
        }
          break;

        case CAM_STREAM_TYPE_RAW:
        case CAM_STREAM_TYPE_SNAPSHOT: {
          param->u.init_param.op_mode = Q3A_OPERATION_MODE_SNAPSHOT;
        }
          break;
        default:
          param->u.init_param.op_mode = Q3A_OPERATION_MODE_PREVIEW;
          break;
      } /* switch (private->stream_type) */

      rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
    }
  }
    break;

  case MCT_EVENT_MODULE_MODE_CHANGE: {
    //Stream mode has changed
    private->stream_type = ((stats_mode_change_event_data*)
      (event->u.module_event.module_event_data))->stream_type;
    private->reserved_id = ((stats_mode_change_event_data*)
      (event->u.module_event.module_event_data))->reserved_id;
    CDBG("%s MCT_EVENT_MODULE_MODE_CHANGE event. mode: %d", __func__,
      private->stream_type);
   }
    break;
  default:
    break;
  } /* switch (mod_evt->type) */

  return rc;
}

/** asd_port_proc_downstream_ctrl
 *
 **/
static boolean asd_port_proc_downstream_ctrl(mct_port_t *port, mct_event_t *event)
{
  boolean rc = TRUE;

  asd_port_private_t  *private  = (asd_port_private_t *)(port->port_private);
  mct_event_control_t *mod_ctrl = &(event->u.ctrl_event);

  switch (mod_ctrl->type) {
  case MCT_EVENT_CONTROL_SET_PARM: {
    stats_set_params_type *stat_parm =
      (stats_set_params_type *)mod_ctrl->control_event_data;

    if (stat_parm->param_type == STATS_SET_ASD_PARAM) {
      asd_thread_msg_t *asd_msg =
        (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
      if (asd_msg != NULL ) {
        asd_msg->type = MSG_ASD_SET;
        asd_msg->u.asd_set_parm = stat_parm->u.asd_param;

        rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
      }
    }
    /* If it's common params shared by many modules */
    else if (stat_parm->param_type == STATS_SET_COMMON_PARAM) {
      stats_common_set_parameter_t *common_param =
        &(stat_parm->u.common_param);
      asd_thread_msg_t *asd_msg = NULL;
      boolean asd_enable = FALSE;
      asd_set_parameter_t asd_param;
      if (common_param->type == COMMON_SET_PARAM_BESTSHOT) {
        asd_enable = (common_param->u.bestshot_mode == CAM_SCENE_MODE_AUTO) ?
            TRUE : FALSE;
        CDBG("%s: MSG_ASD_SET: Enable/disable ASD? %d", __func__, asd_enable);
        asd_msg = (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
        if (asd_msg != NULL) {
          asd_msg->type = MSG_ASD_SET;
          asd_param.type = ASD_SET_ENABLE;
          asd_param.u.asd_enable = asd_enable;
          asd_msg->u.asd_set_parm = asd_param;
        }
        rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
      } else if (common_param->type == COMMON_SET_PARAM_STATS_DEBUG_MASK) {
        asd_msg = (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
        if (asd_msg != NULL) {
          asd_msg->type = MSG_ASD_SET;
          asd_msg->u.asd_set_parm.type = ASD_SET_STATS_DEBUG_MASK;
          asd_msg->u.asd_set_parm.u.stats_debug_mask = common_param->u.stats_debug_mask;
          rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
        }
      }
    }
  } /* MCT_EVENT_CONTROL_SET_PARM */
    break;

  default:
    break;
  }

  return rc;
}

/** asd_port_event
 *    @port:  this port from where the event should go
 *    @event: event object to send upstream or downstream
 *
 *  Because ASD module works no more than a sink module,
 *  hence its upstream event will need a little bit processing.
 *
 *  Return TRUE for successful event processing.
 **/
static boolean asd_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = FALSE;
  asd_port_private_t *private;

  /* sanity check */
  if (!port || !event)
    return FALSE;

  private = (asd_port_private_t *)(port->port_private);
  if (!private)
    return FALSE;

  /* sanity check: ensure event is meant for port with same identity*/
  if ((private->reserved_id & 0xFFFF0000) != (event->identity & 0xFFFF0000))
    return FALSE;

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_DOWNSTREAM: {

    switch (event->type) {
    case MCT_EVENT_MODULE_EVENT: {
      rc = asd_port_proc_downstream_event(port, event);
    } /* case MCT_EVENT_MODULE_EVENT */
      break;

    case MCT_EVENT_CONTROL_CMD: {
      rc = asd_port_proc_downstream_ctrl(port,event);
    }
      break;

    default:
      break;
    }
  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
    break;

  /* ASD port's peer should be Stats module's port */
  case MCT_EVENT_UPSTREAM: {
    mct_port_t *peer = MCT_PORT_PEER(port);
    MCT_PORT_EVENT_FUNC(peer)(peer, event);
  }
    break;

  default:
    rc = FALSE;
    break;
  }

  return rc;
}

/** asd_port_set_caps
 *    @port: port object which the caps to be set;
 *    @caps: this port's capability.
 *
 *  Return TRUE if it is valid soruce port.
 *
 *  Function overwrites a ports capability.
 **/
static boolean asd_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  if (strcmp(MCT_PORT_NAME(port), "asd_sink"))
    return FALSE;

  port->caps = *caps;
  return TRUE;
}

/** asd_port_check_caps_reserve
 *    @port: this interface module's port;
 *    @peer_caps: the capability of peer port which wants to match
 *                interterface port;
 *    @stream_info:
 *
 *  Stats modules are pure s/w software modules, and every port can
 *  support one identity. If the identity is different, support
 *  can be provided via create a new port. Regardless source or
 *  sink port, once capabilities are matched,
 *  - If this port has not been used, it can be supported;
 *  - If the requested stream is in existing identity, return
 *    failure
 *  - If the requested stream belongs to a different session, the port
 *  can not be used.
 **/
static boolean asd_port_check_caps_reserve(mct_port_t *port, void *caps,
  mct_stream_info_t *stream_info)
{
  boolean            rc = FALSE;
  mct_port_caps_t    *port_caps;
  asd_port_private_t *private;

  MCT_OBJECT_LOCK(port);
  if (!port || !caps || !stream_info ||
      strcmp(MCT_OBJECT_NAME(port), "asd_sink")) {
    rc = FALSE;
    goto reserve_done;
  }

  port_caps = (mct_port_caps_t *)caps;
  if (port_caps->port_caps_type != MCT_PORT_CAPS_STATS) {
    rc = FALSE;
    goto reserve_done;
  }

  private = (asd_port_private_t *)port->port_private;
  switch (private->state) {
  case ASD_PORT_STATE_LINKED: {
    if ((private->reserved_id & 0xFFFF0000) ==
        (stream_info->identity & 0xFFFF0000))
      rc = TRUE;
  }
    break;

  case ASD_PORT_STATE_CREATED:
  case ASD_PORT_STATE_UNRESERVED: {

    private->reserved_id = stream_info->identity;
    private->stream_type = stream_info->stream_type;
    private->state       = ASD_PORT_STATE_RESERVED;
    private->stream_info = *stream_info;
    rc = TRUE;
  }
    break;

  case ASD_PORT_STATE_RESERVED:
    if ((private->reserved_id & 0xFFFF0000) ==
        (stream_info->identity & 0xFFFF0000))
      rc = TRUE;
    break;

  default:
    rc = FALSE;
    break;
  }

reserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
}

/** asd_port_check_caps_unreserve
 *    @port: this port object to remove the session/stream;
 *    @identity: session+stream identity.
 *
 *    Return FALSE if the identity is not existing.
 *
 *  This function frees the identity from port's children list.
 **/
static boolean asd_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  asd_port_private_t *private;
  boolean rc = FALSE;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "asd_sink"))
    return FALSE;

  private = (asd_port_private_t *)port->port_private;
  if (!private)
    return FALSE;

  MCT_OBJECT_LOCK(port);
  if ((private->state == ASD_PORT_STATE_UNLINKED   ||
       private->state == ASD_PORT_STATE_LINKED   ||
       private->state == ASD_PORT_STATE_RESERVED) &&
      ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000))) {

    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state       = ASD_PORT_STATE_UNRESERVED;
      private->reserved_id = (private->reserved_id & 0xFFFF0000);
    }
    rc                   = TRUE;
  } else {
    rc = FALSE;
  }
  MCT_OBJECT_UNLOCK(port);

  return rc;
}

/** asd_port_ext_link
 *    @identity:  Identity of session/stream
 *    @port: SINK of ASD ports
 *    @peer: For ASD sink- peer is STATS sink port
 *
 *  Set ASD port's external peer port, which is STATS module's
 *  sink port.
 *
 *  Return TRUE on success.
 **/
static boolean asd_port_ext_link(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  boolean rc = FALSE, thread_init = FALSE;
  asd_port_private_t  *private;
  mct_event_t         event;

  if (strcmp(MCT_OBJECT_NAME(port), "asd_sink")){
    return FALSE;
  }

  private = (asd_port_private_t *)port->port_private;
  if (!private) {
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  switch (private->state) {
  case ASD_PORT_STATE_RESERVED:
  case ASD_PORT_STATE_UNLINKED:
    if ((private->reserved_id & 0xFFFF0000) != (identity & 0xFFFF0000)) {
      break;
    }
  /* Fall through */
  case ASD_PORT_STATE_CREATED:
    thread_init = TRUE;
    rc = TRUE;
    break;

  case ASD_PORT_STATE_LINKED:
    if ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {
      rc = TRUE;
      thread_init = FALSE;
    }
    break;

  default:
    break;
  }

  if (rc == TRUE) {
    if (thread_init == TRUE) {
      if (asd_port_init_threads(port) == FALSE) {
        rc = FALSE;
        goto asd_ext_link_done;
      }
    }

    private->state = ASD_PORT_STATE_LINKED;
    MCT_PORT_PEER(port) = peer;
    MCT_OBJECT_REFCOUNT(port) += 1;
  }

asd_ext_link_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
}

/** asd_port_ext_unlink
 *
 *  @identity: Identity of session/stream
 *  @port: asd module's sink port
 *  @peer: peer of stats sink port
 *
 * This funtion unlink the peer ports of stats sink, src ports
 * and its peer submodule's port
 *
 **/
static void asd_port_ext_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  asd_port_private_t *private;

  if (!port || !peer || MCT_PORT_PEER(port) != peer)
    return;

  private = (asd_port_private_t *)port->port_private;
  if (!private)
    return;

  MCT_OBJECT_LOCK(port);
  if (private->state == ASD_PORT_STATE_LINKED &&
    (private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {

    MCT_OBJECT_REFCOUNT(port) -= 1;
    if (!MCT_OBJECT_REFCOUNT(port)) {
      CDBG("%s: asd_data=%p", __func__, private->thread_data);
      private->state = ASD_PORT_STATE_UNLINKED;
      asd_thread_deinit(private->thread_data);
      MCT_PORT_PEER(port) = NULL;
    }
  }
  MCT_OBJECT_UNLOCK(port);

  return;
}

/** asd_port_find_identity
 *
 **/
boolean asd_port_find_identity(mct_port_t *port, unsigned int identity)
{
  asd_port_private_t *private;

  if (!port)
    return FALSE;

  if(strcmp(MCT_OBJECT_NAME(port), "asd_sink"))
      return FALSE;

  private = port->port_private;

  if (private) {
    return ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000) ?
      TRUE : FALSE);
  }

  return FALSE;
}

/** asd_port_deinit
 *    @port: asd sink port
 *
 *  de-initialize one ASD sink port
 *
 *  Return nothing
 **/
void asd_port_deinit(mct_port_t *port)
{
  asd_port_private_t *private;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "asd_sink"))
    return;

  private = port->port_private;
  if (!private)
      return;
  asd_destroy(private->asd_object.asd);
  free(port->port_private);
}

/** asd_port_init
 *    @port: port object to be initialized
 *
 *  Port initialization, use this function to overwrite
 *  default port methods and install capabilities. Stats
 *  module should have ONLY sink port.
 *
 *  Return TRUE on success.
 **/
boolean asd_port_init(mct_port_t *port, unsigned int identity)
{
  mct_port_caps_t    caps;
  asd_port_private_t *private;
  mct_list_t         *list;

  private = calloc(1, sizeof(asd_port_private_t));
  if (private == NULL)
    return FALSE;

  /* initialize ASD object */
  ASD_INITIALIZE_LOCK(&private->asd_object);

  private->asd_object.asd = asd_init(&(private->asd_object.asd_ops));
  if (private->asd_object.asd == NULL) {
    ASD_DESTROY_LOCK(&private->asd_object);
    free(private);
    return FALSE;
  }

  private->reserved_id = identity;
  private->state       = ASD_PORT_STATE_CREATED;

  port->port_private  = private;
  port->direction     = MCT_PORT_SINK;
  caps.port_caps_type = MCT_PORT_CAPS_STATS;
  caps.u.stats.flag   = (MCT_PORT_CAP_STATS_ASD | MCT_PORT_CAP_STATS_HIST |
                         MCT_PORT_CAP_STATS_BHIST);

  mct_port_set_event_func(port, asd_port_event);
  mct_port_set_set_caps_func(port, asd_port_set_caps);
  mct_port_set_ext_link_func(port, asd_port_ext_link);
  mct_port_set_unlink_func(port, asd_port_ext_unlink);
  mct_port_set_check_caps_reserve_func(port, asd_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, asd_port_check_caps_unreserve);

  if (port->set_caps)
    port->set_caps(port, &caps);

  return TRUE;
}
