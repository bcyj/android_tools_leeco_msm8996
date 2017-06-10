/*============================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "c2d_module_events.h"
/** c2d_module_create_c2d_event:
 *
 * Description:
 *  Create c2d event and fill ack key, and processed divert
 *  information.
 *
 **/
c2d_module_event_t *c2d_module_create_c2d_event(c2d_module_ack_key_t ack_key,
  c2d_hardware_params_t *hw_params, isp_buf_divert_t *isp_buf,
  uint32_t identity, uint32_t div_identity, mct_stream_info_t **stream_info)
{
  mct_stream_info_t *output_stream_info = NULL, *input_stream_info = NULL;
  c2d_module_event_t *c2d_event = (c2d_module_event_t*)
    malloc(sizeof(c2d_module_event_t));
  if(!c2d_event) {
    CDBG_ERROR("%s:%d, malloc() failed\n", __func__, __LINE__);
    return NULL;
  }
  //mct_stream_info_t  *streaminfo = (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  memset(c2d_event, 0x00, sizeof(c2d_module_event_t));
  c2d_event->ack_key = ack_key;
  /* by default all events are created valid */
  c2d_event->invalid = FALSE;

  if (hw_params) {
    /* this is hw processing event */
    c2d_event->hw_process_flag = TRUE;
    c2d_event->type = C2D_MODULE_EVENT_PROCESS_BUF;
    c2d_event->u.process_buf_data.proc_identity = identity;

    /* copy isp buf and other data from the mct event */
    memcpy(&(c2d_event->u.process_buf_data.isp_buf_divert),
      (isp_buf_divert_t *)(isp_buf), sizeof(isp_buf_divert_t));
    if (stream_info[0]) {
      if (stream_info[0]->identity == isp_buf->identity)
        input_stream_info = stream_info[0];
      if (stream_info[0]->identity == identity)
        output_stream_info = stream_info[0];
    }
    if (stream_info[1]) {
      if (stream_info[1]->identity == isp_buf->identity)
        input_stream_info = stream_info[1];
      if (stream_info[1]->identity == identity)
        output_stream_info = stream_info[1];
    }
    /* Store input_stream_info pointer in c2d event */
    c2d_event->u.process_buf_data.input_stream_info = input_stream_info;
    /* Store stream_info pointer in c2d event */
    c2d_event->u.process_buf_data.stream_info = output_stream_info;
    /* processed divert related info */
    c2d_event->u.process_buf_data.proc_div_identity =
      div_identity;
    c2d_event->u.process_buf_data.proc_div_required = FALSE;
    if (div_identity != PPROC_INVALID_IDENTITY) {
      c2d_event->u.process_buf_data.proc_div_required = TRUE;
    }
    /* copy the stream hw params in event */
    memcpy(&(c2d_event->u.process_buf_data.hw_params), hw_params,
      sizeof(c2d_hardware_params_t));
  } else {
    /* this is unprocessed divert event */
    if (div_identity == PPROC_INVALID_IDENTITY) {
      CDBG_ERROR("%s:%d] failed invalid unprocess div identity\n", __func__,
        __LINE__);
      free(c2d_event);
      return NULL;
    }
    c2d_event->hw_process_flag = FALSE;
    c2d_event->type = C2D_MODULE_EVENT_DIVERT_BUF;

    /* copy isp buf and other data from the mct event */
    memcpy(&(c2d_event->u.divert_buf_data.isp_buf_divert),
      (isp_buf_divert_t*)(isp_buf), sizeof(isp_buf_divert_t));
    c2d_event->u.divert_buf_data.div_identity = div_identity;
    c2d_event->u.divert_buf_data.isp_buf_divert.identity = identity;
    c2d_event->u.divert_buf_data.isp_buf_divert.pass_through = 1;
  }
  return c2d_event;
}

/*
 * c2d_module_send_buf_divert_event
 *      module: c2d module structure
 *      indentity: current stream identity
 *      isp_buf: buf_divert event structure.
 *
 *      This function creates event and sends events to c2d_thread. The events
 *      are created according tpo the divert_config structure.
 *
 *      Returns 0 at success
 *
 */
int32_t c2d_module_send_buf_divert_event(mct_module_t* module,
  unsigned int identity, isp_buf_divert_t *isp_buf)
{
  uint32_t identity_list[2];
  uint32_t identity_list_size = 0;
  if (!module || !isp_buf) {
    CDBG_ERROR("%s:%d, failed, module=%p, isp_buf=%p\n", __func__, __LINE__,
      module, isp_buf);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  struct timeval tv1, tv2;
  gettimeofday(&tv1, NULL);
  uint32_t frame_id = isp_buf->buffer.sequence;
  int32_t ret = 0;

  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_stream_params_t *linked_stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if (!stream_params || !session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  linked_stream_params = stream_params->linked_stream_params;

  c2d_module_stream_params_t *linked_stream_list[2];
  uint32_t unproc_div_identity=0x00;
  boolean unproc_div_required=FALSE;

  identity_list[0] = PPROC_INVALID_IDENTITY;
  identity_list[1] = PPROC_INVALID_IDENTITY;
  linked_stream_list[0] = NULL;
  linked_stream_list[1] = NULL;

  /* note: unlock these on all return paths */
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));

  /* decide processing requirements based on the stream params */
  if (linked_stream_params) { /* linked stream case */
    PTHREAD_MUTEX_LOCK(&(linked_stream_params->mutex));
    /* if both streams in the pair are off, drop frame */
    if (stream_params->is_stream_on == FALSE &&
        linked_stream_params->is_stream_on == FALSE) {
      CDBG("%s:%d, stream is off, drop frame and piggy-back ACK\n",
        __func__, __LINE__);
      isp_buf->ack_flag = TRUE;
      isp_buf->is_buf_dirty = 1;
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      return 0;
    }
    /* if current stream is on and linked stream is off */
    else if (stream_params->is_stream_on == TRUE &&
        linked_stream_params->is_stream_on == FALSE) {
      /* only one pass required on current identity */
      linked_stream_list[0] = stream_params;
      identity_list[0] = stream_params->identity;
      identity_list_size = 1;
    }
    /* if current stream is off and linked stream is on */
    else if (stream_params->is_stream_on == FALSE &&
        linked_stream_params->is_stream_on == TRUE) {
      /* only one pass required on linked identity */
      linked_stream_list[0] = linked_stream_params;
      identity_list[0] = linked_stream_params->identity;
      identity_list_size = 1;
    }
    /* if both streams are on */
    else if (stream_params->is_stream_on == TRUE &&
        linked_stream_params->is_stream_on == TRUE) {
      /* first pass on current identity and second pass on linked identity */
      linked_stream_list[0] = stream_params;
      linked_stream_list[1] = linked_stream_params;
      identity_list[0] = stream_params->identity;
      identity_list[1] = linked_stream_params->identity;
      identity_list_size = 2;
    }
  } else { /* non-linked stream case */
    /* if stream is off, drop frame */
    if (stream_params->is_stream_on == FALSE) {
      CDBG("%s:%d, stream is off, drop frame and piggy-back ACK\n",
        __func__, __LINE__);
      isp_buf->ack_flag = TRUE;
      isp_buf->is_buf_dirty = 1;
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      return 0;
    }
    linked_stream_list[0] = stream_params;
    identity_list[0] = stream_params->identity;
    identity_list_size = 1;
  }

  /* Ensure if second params exist then it should be linked with first
     params and share the same divert config table */
  if (linked_stream_list[1]) {
    if (linked_stream_list[1]->div_config !=
      linked_stream_list[0]->div_config) {
      CDBG_ERROR("%s:%d] failed invalid divert config table\n", __func__,
        __LINE__);
      isp_buf->ack_flag = TRUE;
      isp_buf->is_buf_dirty = 1;
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      return 0;
    }
  }

  pproc_divert_info_t *divert_info =
    c2d_module_get_divert_info(&identity_list[0], identity_list_size,
    linked_stream_list[0]->div_config);
  if (!divert_info) {
    CDBG_ERROR("%s:%d, c2d_module_get_divert_info() failed\n",
      __func__, __LINE__);
    if (linked_stream_params)
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    return -EFAULT;
  }
  /* decide if processed/unprocessed divert is required.
     Currently, only one kind of divert is supported. */
  if (divert_info->divert_flags & PPROC_DIVERT_UNPROCESSED) {
    unproc_div_required = TRUE;
    unproc_div_identity = divert_info->div_unproc_identity;
  }

  /* create a key for ack with original event identity, this key will be
     put in all corresponding events in queue and used to release the ack */
  c2d_module_ack_key_t key;
  key.identity = isp_buf->identity;
  key.buf_idx = isp_buf->buffer.index;
  key.channel_id = isp_buf->channel_id;
  key.meta_data = isp_buf->meta_data;

  /* Decide the events to be queued to process this buffer */
  int event_idx = 0, num_events = 0;
  /* based on configuration, at max 3 events are queued for one buffer */
  c2d_module_event_t* c2d_event[3];

  /* Step 1. if unprocessed divert is needed, add an event for that */
  if (unproc_div_required == TRUE) {
    c2d_event[event_idx] = c2d_module_create_c2d_event(key, NULL, isp_buf,
      identity, unproc_div_identity, NULL);
    if (!c2d_event[event_idx]) {
      CDBG_ERROR("%s:%d, malloc() failed\n", __func__, __LINE__);
      if (linked_stream_params)
        PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      return -ENOMEM;
    }
    event_idx++;
  }

  int32_t divert_info_num_passes = divert_info->num_passes;
  int32_t i = 0, j = 0;
  int32_t num_passes = 0;
  boolean skip_frame = FALSE;
  mct_stream_info_t *stream_info[2];
  stream_info[0] = NULL;
  if (stream_params) {
    stream_info[0] = stream_params->stream_info;
  }
  stream_info[1] = NULL;
  if (linked_stream_params) {
    stream_info[1] = linked_stream_params->stream_info;
  }

  /* If LPM is enabled, check if C2D processing is needed based
     on crop, rotation , scaling and formats set. If C2D processing
     can be skipped, then ISP need to do buf done. */
  if (session_params->lpm_enable) {
    if ((stream_params->hw_params.crop_info.stream_crop.x ||
      stream_params->hw_params.crop_info.stream_crop.y) ||
      (stream_params->hw_params.rotation == 1)||
      (stream_params->hw_params.rotation == 3) ||
      (stream_params->hw_params.input_info.c2d_plane_fmt !=
         stream_params->hw_params.output_info.c2d_plane_fmt) ||
      (stream_params->hw_params.input_info.height !=
        stream_params->hw_params.output_info.height) ||
      (stream_params->hw_params.input_info.width !=
        stream_params->hw_params.output_info.width)) {
     CDBG("%s Need C2d processing for iden: 0x%x and frame %d",
       __func__,identity,frame_id);
     isp_buf->is_skip_pproc = FALSE;
    }
    stream_params->hfr_skip_info.skip_required = FALSE;
    divert_info_num_passes = identity_list_size;
  }

  /* Step 2. Based on the number of process identities set in divert config,
     generate c2d events accordingly */
  for (i = 0; i < divert_info_num_passes; i++) {
    for (j = 0; j < 2; j++) {
      if (!linked_stream_list[j] ||
        (linked_stream_list[j]->identity != divert_info->proc_identity[i])) {
        continue;
      }
      if (!linked_stream_list[j]->out_dim_initialized)
       continue;
      skip_frame = FALSE;
      /* decide if skip is required for HFR */
      if (linked_stream_list[j]->hfr_skip_info.skip_required) {
        if ((c2d_decide_hfr_skip(frame_id -
          linked_stream_list[j]->hfr_skip_info.frame_offset,
          linked_stream_list[j]->hfr_skip_info.skip_count)) == TRUE) {
          /* Skip this frame */
          CDBG("%s:%d, skipping frame_id=%d for identity=0x%x", __func__,
            __LINE__, frame_id, linked_stream_list[j]->identity);
          CDBG("%s:%d, skip_count=%d, offset=%d", __func__, __LINE__,
            linked_stream_list[j]->hfr_skip_info.skip_count,
            linked_stream_list[j]->hfr_skip_info.frame_offset);
          skip_frame = TRUE;
        }
      }

      if (skip_frame == FALSE) {
        c2d_event[event_idx] = c2d_module_create_c2d_event(key,
          &(linked_stream_list[j]->hw_params), isp_buf,
          linked_stream_list[j]->identity,
          divert_info->div_proc_identity[i],
          &stream_info[0]);
        if(!c2d_event[event_idx]) {
          CDBG_ERROR("%s:%d, malloc() failed\n", __func__, __LINE__);
          if(linked_stream_params)
            PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
          PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
          return -ENOMEM;
        }
        event_idx++;
      }
      num_passes++;
    }
  }

  if (divert_info_num_passes != num_passes) {
    CDBG("%s:%d] divert_info_num_passes %d num_passes %d\n", __func__,
      __LINE__, divert_info_num_passes, num_passes);
  }

  num_events = event_idx;
  /* if no events needs to be queued, do a piggy-back ACK */
  if (num_events == 0) {
    isp_buf->ack_flag = TRUE;
    if (isp_buf->is_skip_pproc) {
      isp_buf->is_buf_dirty = 0;
    } else {
      isp_buf->is_buf_dirty = 1;
    }
    if(linked_stream_params)
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    CDBG_ERROR("%s:%d] buffer event received with no divert config\n",
      __func__, __LINE__);
    return 0;
  }
  /* before queuing any events, first put corresponding ACK in the ack_list */
  c2d_module_put_new_ack_in_list(ctrl, key, 1, num_events, isp_buf);

  /* now enqueue all events one by one in priority queue */
  int rc;
  for (i=0; i<num_events; i++) {
    rc = c2d_module_enq_event(module, c2d_event[i], stream_params->priority);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed, i=%d\n", __func__, __LINE__, i);
      if(linked_stream_params)
        PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      return -EFAULT;
    }
  }
  if (linked_stream_params)
    PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  /* notify the thread about this new events */

  if (num_events) {
    c2d_thread_msg_t msg;
    msg.type = C2D_THREAD_MSG_NEW_EVENT_IN_Q;
    c2d_module_post_msg_to_thread(module, msg);
  }

  gettimeofday(&tv2, NULL);
  CDBG_LOW("%s:%d, downstream event time = %6ld us, ", __func__, __LINE__,
    (tv2.tv_sec - tv1.tv_sec)*1000000L +
    (tv2.tv_usec - tv1.tv_usec));
  return 0;
}

/* c2d_module_handle_buf_divert_event:
 *
 * Description:
 *  Send the MCT_HANDLE_MODULE_BUF_DIVERT event. First put corresponding
 *  acknowledgement in a list which will be sent later. Depending on the
 *  stream's parameters, divert and processing events are added in
 *  c2d's priority queue. c2d_thread will pick up these events one by one in
 *  order and when all events corresponding to the ACK are processed,
 *  the ACK will be removed from list and will be sent upstream.
 *
 **/
int32_t c2d_module_handle_buf_divert_event(mct_module_t* module,
  mct_event_t* event)
{
  c2d_module_ctrl_t           *ctrl = NULL;
  isp_buf_divert_t            *isp_buf = NULL;
  uint32_t                     frame_id = 0;
  c2d_module_stream_params_t  *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_stream_params_t  *linked_stream_params = NULL;
  c2d_module_frame_hold_t     *frame_hold = NULL;
  c2d_module_dis_hold_t       *dis_hold = NULL;

  if (!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }

  ctrl = (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    CDBG_ERROR("%s:%d: failed ctrl %p\n", __func__, __LINE__, ctrl);
    return -EFAULT;
  }

  isp_buf = (isp_buf_divert_t *)(event->u.module_event.module_event_data);
  if (!isp_buf) {
    CDBG_ERROR("%s:%d: isp_buf %p\n", __func__, __LINE__, isp_buf);
    return -EFAULT;
  }

  if (isp_buf->pass_through == 1) {
    int32_t ret;
    /* This buffer divert event simply needs a bypass through. So directly
       send to downstream module. This event is not queued and ref counted.
       The rule is that downstream module is expected to do a piggyback ack.
       This type of unprocess divert is not very clean though. */
    ret = c2d_module_send_event_downstream(ctrl->p_module, event);
    if (ret < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    }
    return ret;
  }

  frame_id = isp_buf->buffer.sequence;
  CDBG("%s:%d received buffer divert for %d and identity 0x%x\n",
    __func__, __LINE__, frame_id,event->identity);


  c2d_module_get_params_for_identity(ctrl, event->identity, &session_params,
     &stream_params);
  if (!session_params || !stream_params) {
    CDBG_ERROR("%s:%d: failed params %p %p\n", __func__, __LINE__,
      session_params, stream_params);
    return -EFAULT;
  }
  linked_stream_params = stream_params->linked_stream_params;
  if (stream_params->stream_info->is_type == IS_TYPE_EIS_2_0 ||
      stream_params->interleaved) {
    if (linked_stream_params &&
        (stream_params->stream_type == CAM_STREAM_TYPE_VIDEO)) {
      CDBG("%s:%d, Set output out dim change %d identity=0x%x",
               __func__, __LINE__,stream_params->linkedstream_out_dim_change,
               stream_params->identity);
      if (stream_params->linkedstream_out_dim_change) {
        mct_stream_info_t  stream_info;
        mct_event_t  linked_event;
        int32_t ret;

        linked_stream_params->hw_params.output_info.width =
            stream_params->hw_params.output_info.width;
        linked_stream_params->hw_params.output_info.height =
            stream_params->hw_params.output_info.height;
        linked_stream_params->hw_params.output_info.stride =
            PAD_TO_32(stream_params->hw_params.output_info.stride);
        linked_stream_params->hw_params.output_info.scanline =
            PAD_TO_32( stream_params->hw_params.output_info.scanline);
        CDBG("%s:%d, Set output dim %dx%d identity=0x%x",
          __func__, __LINE__,stream_params->hw_params.output_info.width,
          stream_params->hw_params.output_info.height,stream_params->identity);

        stream_info.dim.width =
            linked_stream_params->hw_params.output_info.width;
        stream_info.dim.height =
            linked_stream_params->hw_params.output_info.height;
        stream_info.buf_planes.plane_info.mp[0].stride =
            stream_info.dim.width;
        stream_info.buf_planes.plane_info.mp[0].scanline =
            stream_info.dim.height;
        stream_info.buf_planes.plane_info.mp[0].offset = 0;
        stream_info.buf_planes.plane_info.mp[1].offset = 0;
        if(!stream_params->interleaved) {
          stream_info.fmt = linked_stream_params->hw_params.input_info.cam_fmt;
        }
        else {
          /* For camcorder recording, if video dim is greater than
             preview dim, C2D outputs video stream to CPP, so inputs format to CPP
             should be that of video stream. */
          stream_info.fmt = stream_params->hw_params.output_info.cam_fmt;
        }
        stream_info.stream_type = linked_stream_params->stream_type;

        linked_event.u.module_event.type = MCT_EVENT_MODULE_ISP_OUTPUT_DIM;
        linked_event.u.module_event.module_event_data =
            (void *)&stream_info;
        linked_event.type = MCT_EVENT_MODULE_EVENT;
        linked_event.identity = linked_stream_params->identity;
        linked_event.direction = MCT_EVENT_DOWNSTREAM;
        ret = c2d_module_send_event_downstream(module, &linked_event);
        if (ret < 0) {
          CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
            __func__, __LINE__, event->u.module_event.type, linked_event.identity);
          return -EFAULT;
        }
        stream_params->linkedstream_out_dim_change = FALSE;
      }

    }
  }

  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));
  /* Check whether DIS is enabled */
  if (session_params->dis_enable == 0) {
    CDBG("%s:%d send %d for processing\n", __func__, __LINE__,
      isp_buf->buffer.sequence);
    /* DIS is disabled. Send frame for processing */
    c2d_module_send_buf_divert_event(module, event->identity, isp_buf);
  } else if (session_params->dis_hold.is_valid == TRUE) {
    /* DIS is enabled and atleast one DIS crop event is received */
    linked_stream_params = stream_params->linked_stream_params;
    /* Check whether current frame identity belongs to preview or video */
    if ((stream_params->stream_type == CAM_STREAM_TYPE_PREVIEW) ||
      (stream_params->stream_type == CAM_STREAM_TYPE_VIDEO)) {
      /* Check whether there is already one frame on HOLD */
      frame_hold = &session_params->frame_hold;
      if (frame_hold->is_frame_hold == TRUE) {
        /* DIS crop event is not sent for frame on HOLD yet. But next frame
           is received. Send frame on HOLD for processing */
        CDBG("%s:%d dis not received for previous frame -> %d for processing\n",
          __func__, __LINE__, frame_hold->isp_buf.buffer.sequence);
        c2d_module_send_buf_divert_event(module, frame_hold->identity,
          &frame_hold->isp_buf);
        /* Set is_frame_hold flag to FALSE */
        frame_hold->is_frame_hold = FALSE;
      }
      dis_hold = &session_params->dis_hold;
      /* Check whether DIS frame id is valid &&
         Check whether DIS crop event for this frame has already arrived */
      if (dis_hold->is_valid == TRUE && frame_id <= dis_hold->dis_frame_id) {
        CDBG("%s:%d DIS already arrived for %d, send for processing\n",
          __func__, __LINE__, isp_buf->buffer.sequence);
        /* Send current frame for processing */
        c2d_module_send_buf_divert_event(module, event->identity, isp_buf);
      } else if ((stream_params->is_stream_on == TRUE) ||
          (linked_stream_params &&
          (linked_stream_params->is_stream_on == TRUE))) {
        /* DIS frame id is either invalid or DIS crop event for this frame
           has not arrived yet. HOLD this frame */
        CDBG("%s:%d HOLD %d\n", __func__, __LINE__, isp_buf->buffer.sequence);
        frame_hold->is_frame_hold = TRUE;
        frame_hold->identity = event->identity;
        memcpy(&frame_hold->isp_buf, isp_buf, sizeof(isp_buf_divert_t));
      } else {
        /* Send acknowledge to free  the buffer. */
        isp_buf->ack_flag = 1;
        isp_buf->is_buf_dirty = 1;
      }
    } else {
      /* This frame does not belong to preview / video.
         Send this for processing */
      CDBG("%s:%d live snapshot send %d for processing\n", __func__, __LINE__,
        isp_buf->buffer.sequence);
      c2d_module_send_buf_divert_event(module, event->identity, isp_buf);
    }
  } else {
     CDBG("%s:%d DIS crop event not sent yet %d for processing\n",
      __func__, __LINE__, isp_buf->buffer.sequence);
    /* DIS is disabled. Send frame for processing */
    c2d_module_send_buf_divert_event(module, event->identity, isp_buf);

  }
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));

  return 0;

}

/* c2d_module_handle_isp_out_dim_event:
 *
 * Description:
 *
 **/
int32_t c2d_module_handle_isp_out_dim_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  mct_stream_info_t *stream_info =
    (mct_stream_info_t *)(event->u.module_event.module_event_data);
  if(!stream_info) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  CDBG("%s:%d identity=0x%x, dim=%dx%d\n", __func__, __LINE__,
    event->identity, stream_info->dim.width, stream_info->dim.height);
  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  /* update the dimension of the stream */
  stream_params->hw_params.input_info.width = stream_info->dim.width;
  stream_params->hw_params.input_info.height = stream_info->dim.height;
  stream_params->hw_params.input_info.stride =
    stream_info->buf_planes.plane_info.mp[0].stride;
  stream_params->hw_params.input_info.scanline =
    stream_info->buf_planes.plane_info.mp[0].scanline;
  stream_params->out_dim_initialized = TRUE;
  /* format info */
  if ( (stream_info->fmt == CAM_FORMAT_YUV_420_NV12) ||
       (stream_info->fmt == CAM_FORMAT_YUV_420_NV12_VENUS) ) {
    stream_params->hw_params.input_info.c2d_plane_fmt = C2D_PARAM_PLANE_CBCR;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_420_NV21) {
    stream_params->hw_params.input_info.c2d_plane_fmt = C2D_PARAM_PLANE_CRCB;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_422_NV16) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_CBCR422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_422_NV61) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_CRCB422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_420_YV12) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_CRCB420;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_RAW_8BIT_YUYV) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_YCRYCB422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_RAW_8BIT_YVYU) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_YCBYCR422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_RAW_8BIT_UYVY) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_CRYCBY422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_RAW_8BIT_VYUY) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_CBYCRY422;
  } else {
    CDBG_ERROR("%s:%d] Format not supported\n", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    return -EINVAL;
  }
  stream_params->hw_params.input_info.cam_fmt = stream_info->fmt;
  /* init crop info */
  stream_params->hw_params.crop_info.stream_crop.x = 0;
  stream_params->hw_params.crop_info.stream_crop.y = 0;
  stream_params->hw_params.crop_info.stream_crop.dx = stream_info->dim.width;
  stream_params->hw_params.crop_info.stream_crop.dy = stream_info->dim.height;
  stream_params->hw_params.crop_info.is_crop.x = 0;
  stream_params->hw_params.crop_info.is_crop.y = 0;
  stream_params->hw_params.crop_info.is_crop.dx = stream_info->dim.width;
  stream_params->hw_params.crop_info.is_crop.dy = stream_info->dim.height;


  if ((stream_params->stream_info->is_type == IS_TYPE_EIS_2_0 ||
      stream_params->interleaved) &&
      !stream_params->single_module) {
    c2d_module_stream_params_t*  linked_stream_params = NULL;

    linked_stream_params = stream_params->linked_stream_params;


    if (!stream_params->interleaved) {
      stream_params->hw_params.crop_info.stream_crop.x = 0;
      stream_params->hw_params.crop_info.stream_crop.y = 0;
      stream_params->hw_params.crop_info.stream_crop.dx =
        stream_params->stream_info->dim.width;
      stream_params->hw_params.crop_info.stream_crop.dy =
        stream_params->stream_info->dim.height;
      stream_params->hw_params.crop_info.is_crop.x = 0;
      stream_params->hw_params.crop_info.is_crop.y = 0;
      stream_params->hw_params.crop_info.is_crop.dx =
        stream_params->stream_info->dim.width;
      stream_params->hw_params.crop_info.is_crop.dy =
        stream_params->stream_info->dim.height;
    }


    if (linked_stream_params &&
        (stream_params->stream_type == CAM_STREAM_TYPE_VIDEO)) {
      if ((stream_params->hw_params.output_info.width *
          stream_params->hw_params.output_info.height) <
          (linked_stream_params->hw_params.output_info.width *
              linked_stream_params->hw_params.output_info.height)) {
      stream_params->hw_params.output_info.width =
          linked_stream_params->hw_params.output_info.width;
      stream_params->hw_params.output_info.height =
          linked_stream_params->hw_params.output_info.height;
      stream_params->hw_params.output_info.stride =
          linked_stream_params->hw_params.output_info.stride;
      stream_params->hw_params.output_info.scanline =
        linked_stream_params->hw_params.output_info.scanline;
      }
    }

    stream_info->dim.width = stream_params->hw_params.output_info.width;
    stream_info->dim.height = stream_params->hw_params.output_info.height;
    stream_info->buf_planes.plane_info.mp[0].stride =
        stream_info->dim.width;
    stream_info->buf_planes.plane_info.mp[0].scanline =
        stream_info->dim.height;
    stream_info->buf_planes.plane_info.mp[0].offset = 0;
    stream_info->buf_planes.plane_info.mp[1].offset = 0;
    if ( stream_params->interleaved) {
      /* For camcorder recording, if video dim is less than preview dim
         C2D outputs preview stream to CPP (which performs 2 pass to
         give preview and video streams), so input format to CPP
         should be that of preview stream */
      if (linked_stream_params &&
        (stream_params->stream_type == CAM_STREAM_TYPE_VIDEO) &&
        (!stream_params->linkedstream_out_dim_change)) {
        stream_info->fmt = linked_stream_params->c2d_output_lib_params.format;
      }
      else {
        stream_info->fmt = stream_params->c2d_output_lib_params.format;
      }
    }
    CDBG("%s:%d, set format =%d, identity=0x%x",
      __func__, __LINE__,stream_info->fmt,event->identity);
    CDBG("%s:%d,Send CPP input dimensions %dx%d, %dx%d",
      __func__, __LINE__,stream_info->dim.width,
      stream_info->dim.height, stream_info->buf_planes.plane_info.mp[0].stride,
      stream_info->buf_planes.plane_info.mp[0].scanline);
  }
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  rc = c2d_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* c2d_module_handle_stream_crop_event:
 *
 * Description:
 *
 **/
int32_t c2d_module_handle_stream_crop_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  CDBG("%s:%d identity=0x%x", __func__, __LINE__, event->identity);
  mct_bus_msg_stream_crop_t *stream_crop =
    (mct_bus_msg_stream_crop_t *) event->u.module_event.module_event_data;
  if(!stream_crop) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  /* if crop is (0, 0, 0, 0) ignore the event */
  if (stream_crop->x == 0 && stream_crop->y == 0 &&
      stream_crop->crop_out_x == 0 && stream_crop->crop_out_x == 0) {
    //return 0;
      PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
      stream_params->hw_params.crop_info.stream_crop.x = stream_crop->x;
      stream_params->hw_params.crop_info.stream_crop.y = stream_crop->y;
      stream_params->hw_params.crop_info.stream_crop.dx =
        stream_params->hw_params.input_info.width;
      stream_params->hw_params.crop_info.stream_crop.dy =
        stream_params->hw_params.input_info.height;
      stream_crop->crop_out_x =
        stream_params->hw_params.input_info.width;
      stream_crop->crop_out_y =
        stream_params->hw_params.input_info.height;

      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      rc = c2d_module_send_event_downstream(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
          __func__, __LINE__, event->u.module_event.type, event->identity);
        return -EFAULT;
      }
      return 0;

  }

  if ((stream_params->stream_info->is_type == IS_TYPE_EIS_2_0 ||
      stream_params->interleaved) &&
      !stream_params->single_module) {

    if (stream_params->hw_params.input_info.width &&
        stream_params->hw_params.output_info.width) {
      stream_crop->x = (stream_crop->x *
          stream_params->hw_params.output_info.width) /
              stream_params->hw_params.input_info.width;
      stream_crop->y = (stream_crop->y *
          stream_params->hw_params.output_info.height) /
              stream_params->hw_params.input_info.height;
      stream_crop->crop_out_x = (stream_crop->crop_out_x *
          stream_params->hw_params.output_info.width) /
              stream_params->hw_params.input_info.width;
      stream_crop->crop_out_y = (stream_crop->crop_out_y *
          stream_params->hw_params.output_info.height) /
             stream_params->hw_params.input_info.height;
    }
    rc = c2d_module_send_event_downstream(module, event);
    if (rc < 0) {
       CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
        __func__, __LINE__, event->u.module_event.type, event->identity);
       return -EFAULT;
    }
    return 0;
  }

  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->hw_params.crop_info.stream_crop.x = stream_crop->x;
  stream_params->hw_params.crop_info.stream_crop.y = stream_crop->y;
  stream_params->hw_params.crop_info.stream_crop.dx = stream_crop->crop_out_x;
  stream_params->hw_params.crop_info.stream_crop.dy = stream_crop->crop_out_y;
  CDBG("%s:%d stream_crop.x=%d, stream_crop.y=%d, stream_crop.dx=%d,"
           " stream_crop.dy=%d, identity=0x%x", __func__, __LINE__,
           stream_crop->x, stream_crop->y, stream_crop->crop_out_x,
           stream_crop->crop_out_y, event->identity);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  rc = c2d_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* c2d_module_handle_dis_update_event:
 *
 * Description:
 *
 **/
int32_t c2d_module_handle_dis_update_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;

  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  is_update_t *is_update =
    (is_update_t *) event->u.module_event.module_event_data;
  if(!is_update) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_stream_params_t  *linked_stream_params = NULL;
  c2d_module_frame_hold_t     *frame_hold = FALSE;
  c2d_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));
  /* Check whether DIS is enabled, else return without storing */
  if (session_params->dis_enable == 0) {
    CDBG_LOW("%s:%d dis enable %d\n", __func__, __LINE__,
      session_params->dis_enable);
    PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));
    return 0;
  }
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));

  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  /* Update is_crop in linked_stream_params */
  linked_stream_params = stream_params->linked_stream_params;

  stream_params->hw_params.crop_info.is_crop.use_3d = is_update->use_3d;
  memcpy(&stream_params->hw_params.crop_info.is_crop.transform_matrix,
      &is_update->transform_matrix,sizeof(is_update->transform_matrix));
  stream_params->hw_params.crop_info.is_crop.transform_type =
      is_update->transform_type;

  if (((is_update->x  + is_update->width) <=
    stream_params->hw_params.input_info.width) &&
    ((is_update->y + is_update->height) <=
    stream_params->hw_params.input_info.height)) {
    stream_params->hw_params.crop_info.is_crop.x = is_update->x;
    stream_params->hw_params.crop_info.is_crop.y = is_update->y;
    stream_params->hw_params.crop_info.is_crop.dx = is_update->width;
    stream_params->hw_params.crop_info.is_crop.dy = is_update->height;
    CDBG("%s:%d is_crop.x=%d, is_crop.y=%d, is_crop.dx=%d, is_crop.dy=%d,"
      " identity=0x%x", __func__, __LINE__, is_update->x, is_update->y,
      is_update->width, is_update->height, event->identity);
  } else {
    /* Error case. Incorrect DIS parameters */
    linked_stream_params = NULL;
    CDBG_ERROR("%s:%d is_crop.x=%d, is_crop.y=%d, is_crop.dx=%d, is_crop.dy=%d,"
      " identity=0x%x", __func__, __LINE__, is_update->x, is_update->y,
      is_update->width, is_update->height, event->identity);
  }
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));
  /* Update frame id in session_params */
  session_params->dis_hold.is_valid = TRUE;
  session_params->dis_hold.dis_frame_id = is_update->frame_id;
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));

  if (linked_stream_params) {
    PTHREAD_MUTEX_LOCK(&(linked_stream_params->mutex));
    linked_stream_params->hw_params.crop_info.is_crop.use_3d = is_update->use_3d;
    memcpy(&linked_stream_params->hw_params.crop_info.is_crop.transform_matrix,
        &is_update->transform_matrix, sizeof(is_update->transform_matrix));
    linked_stream_params->hw_params.crop_info.is_crop.transform_type =
        is_update->transform_type;
      linked_stream_params->hw_params.crop_info.is_crop.x = is_update->x;
      linked_stream_params->hw_params.crop_info.is_crop.y = is_update->y;
      linked_stream_params->hw_params.crop_info.is_crop.dx = is_update->width;
      linked_stream_params->hw_params.crop_info.is_crop.dy = is_update->height;
    PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
  }

  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));
  frame_hold = &session_params->frame_hold;
  /* Check whether frame is on HOLD &&
     DIS crop event is for frame on HOLD */
  if ((frame_hold->is_frame_hold == TRUE) &&
    (session_params->dis_hold.dis_frame_id >=
    frame_hold->isp_buf.buffer.sequence)) {
    CDBG("%s:%d send %d for processing\n", __func__, __LINE__,
      frame_hold->isp_buf.buffer.sequence);
    /* Send this frame for c2d processing */
    c2d_module_send_buf_divert_event(module, frame_hold->identity,
      &frame_hold->isp_buf);
    /* Update frame hold flag to FALSE */
    frame_hold->is_frame_hold = FALSE;
  }
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));

#if 0
  rc = c2d_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
#endif

  return 0;
}

/* c2d_module_handle_stream_cfg_event:
 *
 * Description:
 *
 **/
int32_t c2d_module_handle_stream_cfg_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  sensor_out_info_t *sensor_out_info =
    (sensor_out_info_t *)(event->u.module_event.module_event_data);
  if (!sensor_out_info) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* apply this to all streams for that session */
  int i;
  for(i=0; i<C2D_MODULE_MAX_STREAMS; i++) {
    if(session_params->stream_params[i]) {
      PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
      session_params->stream_params[i]->hfr_skip_info.frame_offset =
        sensor_out_info->num_frames_skip + 1;
      session_params->stream_params[i]->hfr_skip_info.input_fps =
        sensor_out_info->max_fps;
      c2d_module_update_hfr_skip(session_params->stream_params[i]);
      PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
      CDBG("%s:%d frame_offset=%d, input_fps=%.2f, identity=0x%x",
        __func__, __LINE__,
        session_params->stream_params[i]->hfr_skip_info.frame_offset,
        session_params->stream_params[i]->hfr_skip_info.input_fps,
        session_params->stream_params[i]->identity);
    }
  }
  rc = c2d_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* c2d_module_handle_fps_update_event:
 *
 * Description:
 *
 **/
int32_t c2d_module_handle_fps_update_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  sensor_fps_update_t *fps_update =
    (sensor_fps_update_t *)(event->u.module_event.module_event_data);

  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* apply this to all streams for that session */
  int i;
  for(i=0; i<C2D_MODULE_MAX_STREAMS; i++) {
    if(session_params->stream_params[i]) {
      PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
      session_params->stream_params[i]->hfr_skip_info.input_fps =
        fps_update->max_fps;
      c2d_module_update_hfr_skip(session_params->stream_params[i]);
      PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
      CDBG("%s:%d input_fps=%.2f, identity=0x%x",
        __func__, __LINE__,
        session_params->stream_params[i]->hfr_skip_info.input_fps,
        session_params->stream_params[i]->identity);
    }
  }
  rc = c2d_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/**c2d_module_handle_div_info_event:
 *
 * Description:
 *
 **/
int32_t c2d_module_handle_div_info_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  pproc_divert_config_t *div_cfg =
    (pproc_divert_config_t *)(event->u.module_event.module_event_data);
  if (!div_cfg) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  if (!div_cfg->name) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* check if this config is intended for this module */
  if (strncmp(MCT_OBJECT_NAME(module), div_cfg->name,
       strlen(MCT_OBJECT_NAME(module))) != 0) {
    rc = c2d_module_send_event_downstream(module, event);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
        __func__, __LINE__, event->u.module_event.type, event->identity);
      return -EFAULT;
    }
    return 0;
  }
  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  c2d_module_set_divert_cfg_entry(event->identity, div_cfg->update_mode,
    &div_cfg->divert_info, stream_params->div_config);
  if(div_cfg->divert_info.divert_flags & PPROC_DIVERT_PROCESSED)
    stream_params->hw_params.processed_divert = TRUE;
  CDBG("%s,processed_divert = %d,divert_flag = 0x%x for iden: 0x%x",
    __func__,
    stream_params->hw_params.processed_divert,
    div_cfg->divert_info.divert_flags,
    event->identity);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  return 0;
}

/* c2d_module_set_parm_hfr_mode:
 *
 **/
static int32_t c2d_module_set_parm_hfr_mode(c2d_module_ctrl_t *ctrl,
  uint32_t identity, cam_hfr_mode_t hfr_mode)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* apply this to all streams where hfr skip is required */
  int i;
  for(i=0; i<C2D_MODULE_MAX_STREAMS; i++) {
    if(session_params->stream_params[i]) {
      PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
      if(session_params->stream_params[i]->hfr_skip_info.skip_required) {
        switch(hfr_mode) {
        case CAM_HFR_MODE_OFF:
          session_params->stream_params[i]->hfr_skip_info.skip_count = 0;
          break;
        case CAM_HFR_MODE_60FPS:
          session_params->stream_params[i]->hfr_skip_info.skip_count = 1;
          break;
        case CAM_HFR_MODE_90FPS:
          session_params->stream_params[i]->hfr_skip_info.skip_count = 2;
          break;
        case CAM_HFR_MODE_120FPS:
          session_params->stream_params[i]->hfr_skip_info.skip_count = 3;
          break;
        case CAM_HFR_MODE_150FPS:
          session_params->stream_params[i]->hfr_skip_info.skip_count = 4;
          break;
        default:
          CDBG_ERROR("%s:%d, bad hfr_mode=%d", __func__, __LINE__, hfr_mode);
          PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
          return -EINVAL;
        }
      }
      PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
    }
  }
  return 0;
}

/* c2d_module_set_parm_fps_range:
 *
 **/
static int32_t c2d_module_set_parm_fps_range(c2d_module_ctrl_t *ctrl,
  uint32_t identity, cam_fps_range_t *fps_range)
{
  if ((!ctrl) || (!fps_range)) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* apply this to all streams where hfr skip is required */
  int i;
  session_params->fps_range.max_fps = fps_range->max_fps;
  session_params->fps_range.min_fps = fps_range->min_fps;
  session_params->fps_range.video_max_fps = fps_range->video_max_fps;
  session_params->fps_range.video_min_fps = fps_range->video_min_fps;
  CDBG ("%s:%d, max_fps %f video_max_fps %f", __func__, __LINE__,
    fps_range->max_fps, fps_range->video_max_fps);
  for(i=0; i<C2D_MODULE_MAX_STREAMS; i++) {
    if(session_params->stream_params[i]) {
      PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
      if(session_params->stream_params[i]->hfr_skip_info.skip_required) {
        if(session_params->stream_params[i]->stream_type ==
          CAM_STREAM_TYPE_VIDEO)
        {
          session_params->stream_params[i]->hfr_skip_info.output_fps =
           fps_range->video_max_fps;
        }
        else
        {
          session_params->stream_params[i]->hfr_skip_info.output_fps =
           fps_range->max_fps;
        }
        c2d_module_update_hfr_skip(session_params->stream_params[i]);
      }
      PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
    }
  }
  return 0;
}

static int32_t c2d_module_set_parm_lowpowermode(c2d_module_ctrl_t *ctrl,
  uint32_t identity, uint32_t lpm_enable)
{
  if (!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  int i;
  session_params->lpm_enable = lpm_enable;

  CDBG ("%s:%d,lowpowermode %d", __func__, __LINE__,lpm_enable);

  return 0;
}

/*
 * c2d_module_set_parm_dis:
 *    ctrl: c2d_module control structure
 *    identity: current strean identity
 *    dis_enable: flag used to enable or disabel DIS
 *
 *    This is a event handler of event CAM_INTF_PARM_DIS_ENABLE. This event is
 *    is used to enable/diable DIS
 *
 *    It returns 0 at success.
 **/
static int32_t c2d_module_set_parm_dis(c2d_module_ctrl_t *ctrl,
  uint32_t identity, int32_t dis_enable)
{
  if (!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if (!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));
  /* Update dis_enable flag in session_params */
  session_params->dis_enable = dis_enable;
  CDBG("%s:%d dis_enable %d\n", __func__, __LINE__, dis_enable);
  if (dis_enable == 0) {
    /* Invalidate DIS hold flag */
    session_params->dis_hold.is_valid = FALSE;
  }
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));
  return 0;
}

/* c2d_module_set_parm_flip:
 *    @ctrl: c2d module control struct
 *    @indentity: current indentity
 *    @flip_mask: new flip mode
 *
 *    Set the flip mode sent form application
 *
 *    Returns 0 on succes or EFAULT if some of the parameters
 *      is missing.
 **/
static int32_t c2d_module_set_parm_flip(c2d_module_ctrl_t *ctrl,
  uint32_t identity, int32_t flip_mask)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->hw_params.mirror = flip_mask;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  return 0;
}

/* c2d_module_handle_set_stream_parm_event:
 *
 *  @module: mct module structure for c2d module
 *  @event: incoming event
 *
 *    Handles stream_param events. Such as SET_FLIP_TYPE.
 *
 *    Returns 0 on success.
 **/
int32_t c2d_module_handle_set_stream_parm_event(mct_module_t* module,
  mct_event_t* event)
{
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p", __func__, __LINE__,
                module, event);
    return -EINVAL;
  }

  cam_stream_parm_buffer_t *param =
    (cam_stream_parm_buffer_t *)event->u.ctrl_event.control_event_data;
  int32_t rc;
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  switch(param->type) {
  case CAM_STREAM_PARAM_TYPE_SET_FLIP: {
    int32_t flip_mask = param->flipInfo.flip_mask;

    rc = c2d_module_set_parm_flip(ctrl, event->identity, flip_mask);
    if(rc) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return rc;
    }
    break;
  }
  default:
    rc = c2d_module_send_event_downstream(module, event);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed, control_event_type=%d, identity=0x%x",
        __func__, __LINE__, event->u.ctrl_event.type, event->identity);
      return -EFAULT;
    }
    break;
  }

  return 0;
}


/* c2d_module_handle_set_parm_event:
 *
 * Description:
 *   Handle the set_parm event.
 **/
int32_t c2d_module_handle_set_parm_event(mct_module_t* module,
  mct_event_t* event)
{
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p", __func__, __LINE__,
                module, event);
    return -EINVAL;
  }
  mct_event_control_parm_t *ctrl_parm =
    (mct_event_control_parm_t *) event->u.ctrl_event.control_event_data;
  if(!ctrl_parm) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  int32_t rc;
  switch (ctrl_parm->type) {
  case CAM_INTF_PARM_HFR: {
    if(!(ctrl_parm->parm_data)) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return -EFAULT;
    }
    cam_hfr_mode_t hfr_mode =
      *(cam_hfr_mode_t *)(ctrl_parm->parm_data);
    CDBG("%s:%d, CAM_INTF_PARM_HFR, mode=%d, identity=0x%x",
      __func__, __LINE__, hfr_mode, event->identity);
    rc = c2d_module_set_parm_hfr_mode(ctrl, event->identity, hfr_mode);
    if (rc < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_FPS_RANGE: {
    if(!(ctrl_parm->parm_data)) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return -EFAULT;
    }
    cam_fps_range_t *fps_range = (cam_fps_range_t *)(ctrl_parm->parm_data);
    CDBG("%s:%d, CAM_INTF_PARM_FPS_RANGE,, max_fps=%.2f, identity=0x%x",
      __func__, __LINE__, fps_range->max_fps, event->identity);
    rc = c2d_module_set_parm_fps_range(ctrl, event->identity, fps_range);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_LOW_POWER_ENABLE: {
    if(!(ctrl_parm->parm_data)) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return -EFAULT;
    }
    int32_t lpm_enable =
      *(int32_t *)(ctrl_parm->parm_data);
    rc = c2d_module_set_parm_lowpowermode(ctrl, event->identity, lpm_enable);
    if (rc < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_DIS_ENABLE: {
    if (!(ctrl_parm->parm_data)) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return -EFAULT;
    }
    int32_t dis_enable =
      *(int32_t *)(ctrl_parm->parm_data);
    CDBG("%s:%d, CAM_INTF_PARM_DIS_ENABLE, enable=%d, identity=0x%x",
      __func__, __LINE__, dis_enable, event->identity);
    rc = c2d_module_set_parm_dis(ctrl, event->identity, dis_enable);
    if (rc < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return rc;
    }
    break;
  }
  default:
    break;
  }

  rc = c2d_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* c2d_module_handle_streamon_event:
 *
 **/
int32_t c2d_module_handle_streamon_event(mct_module_t* module,
  mct_event_t* event)
{
  c2d_module_stream_buff_info_t   stream_buff_info;
  c2d_hardware_stream_buff_info_t hw_strm_buff_info;
  mct_stream_info_t              *streaminfo =
    (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  c2d_module_ctrl_t              *ctrl =
    (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  c2d_hardware_cmd_t              cmd;
  boolean                         rc = -EINVAL;
  mct_event_t                     new_event;
  stats_get_data_t                stats_get;

  /* get stream parameters */
  c2d_module_session_params_t* session_params = NULL;
  c2d_module_stream_params_t*  stream_params = NULL;
  c2d_module_stream_params_t*  linked_stream_params = NULL;
  c2d_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  linked_stream_params = stream_params->linked_stream_params;

  memset(&stream_buff_info, 0, sizeof(c2d_module_stream_buff_info_t));
  memset(&hw_strm_buff_info, 0, sizeof(c2d_hardware_stream_buff_info_t));

  /* attach the identity */
  stream_buff_info.identity = event->identity;
  stream_params->stream_info->img_buffer_list = streaminfo->img_buffer_list;
  /* traverse through the mct stream buff list and create c2d module's
     own list of buffer info */
  if (mct_list_traverse(streaminfo->img_buffer_list,
    c2d_module_util_map_buffer_info, &stream_buff_info) == FALSE) {
    CDBG_ERROR("%s:%d, error creating stream buff list\n", __func__,
      __LINE__);
    goto C2D_MODULE_STREAMON_ERROR1;
  }

  /* create and translate to hardware buffer array */
  hw_strm_buff_info.buffer_info = (c2d_hardware_buffer_info_t *)malloc(
    sizeof(c2d_hardware_buffer_info_t) * stream_buff_info.num_buffs);
  if(NULL == hw_strm_buff_info.buffer_info) {
    CDBG_ERROR("%s:%d, error creating hw buff list\n", __func__,
      __LINE__);
    goto C2D_MODULE_STREAMON_ERROR1;
  }

  hw_strm_buff_info.identity = stream_buff_info.identity;
  if (mct_list_traverse(stream_buff_info.buff_list,
    c2d_module_util_create_hw_stream_buff, &hw_strm_buff_info) == FALSE) {
    CDBG_ERROR("%s:%d, error creating stream buff list\n", __func__,
      __LINE__);
    goto C2D_MODULE_STREAMON_ERROR2;
  }

  if(hw_strm_buff_info.num_buffs != stream_buff_info.num_buffs) {
    CDBG_ERROR("%s:%d, error creating stream buff list\n", __func__,
      __LINE__);
    goto C2D_MODULE_STREAMON_ERROR2;
  }

  cmd.type = C2D_HW_CMD_STREAMON;
  cmd.u.stream_buff_list = &hw_strm_buff_info;
  rc = c2d_hardware_process_command(ctrl->c2dhw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    goto C2D_MODULE_STREAMON_ERROR2;
  }
  /* change state to stream ON */
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->is_stream_on = TRUE;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  if ((stream_params->stream_info->is_type == IS_TYPE_EIS_2_0 ||
      stream_params->interleaved) &&
      !stream_params->single_module){
    if (linked_stream_params &&
        (stream_params->stream_type == CAM_STREAM_TYPE_VIDEO)) {
      if (( linked_stream_params->hw_params.output_info.width *
          linked_stream_params->hw_params.output_info.height) <
          (stream_params->hw_params.output_info.width *
              stream_params->hw_params.output_info.height)){
        stream_params->linkedstream_out_dim_change = TRUE;
        CDBG_ERROR("%s:%d, Preview needs to change output identity 0x%x",
          __func__, __LINE__,
          stream_params->identity);
      }
    }
  }
  rc = c2d_module_send_event_downstream(module,event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    goto C2D_MODULE_STREAMON_ERROR2;
  }

  CDBG_HIGH("%s:%d, identity=0x%x, stream-on done", __func__, __LINE__,
    event->identity);

  rc = 0;

C2D_MODULE_STREAMON_ERROR2:
  free(hw_strm_buff_info.buffer_info);

C2D_MODULE_STREAMON_ERROR1:
  mct_list_traverse(stream_buff_info.buff_list,
    c2d_module_util_free_buffer_info, &stream_buff_info);
  mct_list_free_list(stream_buff_info.buff_list);

  return rc;
}

/* c2d_module_handle_streamoff_event:
 *
 **/
int32_t c2d_module_handle_streamoff_event(mct_module_t* module,
  mct_event_t* event)
{
  int rc;
  c2d_module_frame_hold_t *frame_hold = NULL;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n",
      __func__, __LINE__, module, event);
    return -EINVAL;
  }
  mct_stream_info_t *streaminfo =
    (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  uint32_t identity = event->identity;
  CDBG("%s:%d, info: doing stream-off for identity 0x%x",
    __func__, __LINE__, identity);

  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  c2d_module_session_params_t* session_params = NULL;
  c2d_module_stream_params_t*  stream_params = NULL;
  c2d_module_stream_params_t*  linked_stream_params = NULL;
  c2d_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  if ((stream_params->stream_info->is_type == IS_TYPE_EIS_2_0 ||
      stream_params->interleaved) &&
      !stream_params->single_module) {
     c2d_module_stream_params_t*  linked_stream_params = NULL;

     linked_stream_params = stream_params->linked_stream_params;

     if (linked_stream_params &&
         (stream_params->stream_type == CAM_STREAM_TYPE_VIDEO)) {
         mct_stream_info_t  linked_stream_info;
         mct_event_t  linked_event;

         linked_stream_params->hw_params.output_info.width =
             linked_stream_params->stream_info->dim.width;
         linked_stream_params->hw_params.output_info.height =
             linked_stream_params->stream_info->dim.height;
         linked_stream_params->hw_params.output_info.stride =
             linked_stream_params->stream_info->dim.width;
         linked_stream_params->hw_params.output_info.scanline =
             linked_stream_params->stream_info->dim.height;
         if (!stream_params->interleaved) {
           linked_stream_params->hw_params.crop_info.is_crop.dx =
             linked_stream_params->stream_info->dim.width;
           linked_stream_params->hw_params.crop_info.is_crop.dy =
             linked_stream_params->stream_info->dim.height;
           linked_stream_info.fmt =
             linked_stream_params->hw_params.input_info.cam_fmt;
         } else {
           linked_stream_info.fmt =
             linked_stream_params->hw_params.output_info.cam_fmt;
         }
         linked_stream_info.dim.width =
             linked_stream_params->hw_params.output_info.width;
         linked_stream_info.dim.height =
             linked_stream_params->hw_params.output_info.height;
         linked_stream_info.buf_planes.plane_info.mp[0].stride =
             linked_stream_info.dim.width;
         linked_stream_info.buf_planes.plane_info.mp[0].scanline =
             linked_stream_info.dim.height;
         linked_stream_info.buf_planes.plane_info.mp[0].offset = 0;
         linked_stream_info.buf_planes.plane_info.mp[1].offset = 0;
         linked_stream_info.stream_type = linked_stream_params->stream_type;

         linked_event.u.module_event.type = MCT_EVENT_MODULE_ISP_OUTPUT_DIM;
         linked_event.u.module_event.module_event_data =
             (void *)&linked_stream_info;
         linked_event.type = MCT_EVENT_MODULE_EVENT;
         linked_event.identity =linked_stream_params->identity;
         linked_event.direction = MCT_EVENT_DOWNSTREAM;
         rc = c2d_module_send_event_downstream(module, &linked_event);
         if (rc < 0) {
           CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
             __func__, __LINE__, event->u.module_event.type, event->identity);
           return -EFAULT;
         }
     }
   }
  /* change the state of this stream to OFF, this will prevent
     any incoming buffers to be added to the processing queue  */
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->is_stream_on = FALSE;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  linked_stream_params = stream_params->linked_stream_params;

  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));
  /* Check whether there is a frame on HOLD */
  frame_hold = &session_params->frame_hold;
  if (frame_hold->is_frame_hold == TRUE) {
    /* Check whether identity of frame on HOLD matches with identity of
       stream_params / linked_stream_params */
    if ((stream_params->identity == frame_hold->identity) ||
      (linked_stream_params && (linked_stream_params->identity ==
      frame_hold->identity))) {
      CDBG("%s:%d send %d for processing\n", __func__, __LINE__,
        frame_hold->isp_buf.buffer.sequence);
      /* Send frame on HOLD for processing */
      c2d_module_send_buf_divert_event(module, frame_hold->identity,
        &frame_hold->isp_buf);
      /* Set is_frame_hold flag to FALSE */
      frame_hold->is_frame_hold = FALSE;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));


  /* send stream_off to downstream. This blocking call ensures
     downstream modules are streamed off and no acks pending from them */
  rc = c2d_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  CDBG("%s:%d, info: downstream stream-off done.", __func__, __LINE__);

  /* invalidate any remaining entries in queue corresponding to
     this identity. This will also send/update corresponding ACKs */
  CDBG("%s:%d, info: invalidating queue.", __func__, __LINE__);
  rc = c2d_module_invalidate_queue(ctrl, identity);
    if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  /* process hardware command for stream off, this ensures
     hardware is done with this identity */
  c2d_hardware_cmd_t cmd;
  cmd.type = C2D_HW_CMD_STREAMOFF;
  cmd.u.streamoff_identity = streaminfo->identity;
  rc = c2d_hardware_process_command(ctrl->c2dhw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d: hw streamoff failed\n", __func__, __LINE__);
    return -rc;
  }
  CDBG_HIGH("%s:%d, info: stream-off done for identity 0x%x",
    __func__, __LINE__, identity);
  return 0;
}
