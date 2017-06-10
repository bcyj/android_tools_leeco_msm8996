
/*============================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "eztune_diagnostics.h"
#include "cpp_module_events.h"

/** cpp_module_create_cpp_event:
 *
 * Description:
 *  Create cpp event and fill ack key, and processed divert
 *  information.
 *
 **/
cpp_module_event_t *cpp_module_create_cpp_event(cpp_module_ack_key_t ack_key,
  cpp_hardware_params_t *hw_params, isp_buf_divert_t *isp_buf,
  uint32_t identity, uint32_t div_identity,boolean duplicate_output)
{
  cpp_module_event_t *cpp_event = (cpp_module_event_t*)
    malloc(sizeof(cpp_module_event_t));
  if(!cpp_event) {
    CDBG_ERROR("%s:%d, malloc() failed\n", __func__, __LINE__);
    return NULL;
  }
  memset(cpp_event, 0x00, sizeof(cpp_module_event_t));
  cpp_event->ack_key = ack_key;
  /* by default all events are created valid */
  cpp_event->invalid = FALSE;

  if (hw_params) {
    /* this is hw processing event */
    cpp_event->hw_process_flag = TRUE;
    cpp_event->type = CPP_MODULE_EVENT_PROCESS_BUF;
    cpp_event->u.process_buf_data.proc_identity = identity;

    /* copy isp buf and other data from the mct event */
    memcpy(&(cpp_event->u.process_buf_data.isp_buf_divert),
      (isp_buf_divert_t *)(isp_buf), sizeof(isp_buf_divert_t));
    /* copy the stream hw params in event */
    memcpy(&(cpp_event->u.process_buf_data.hw_params), hw_params,
      sizeof(cpp_hardware_params_t));
    /* processed divert related info */
    cpp_event->u.process_buf_data.proc_div_identity =
      div_identity;
    cpp_event->u.process_buf_data.hw_params.duplicate_output =
      duplicate_output;
    cpp_event->u.process_buf_data.proc_div_required = FALSE;
    if (div_identity != PPROC_INVALID_IDENTITY) {
      cpp_event->u.process_buf_data.proc_div_required = TRUE;
    }
  } else {
    /* this is unprocessed divert event */
    if (div_identity == PPROC_INVALID_IDENTITY) {
      CDBG_ERROR("%s:%d] failed invalid unprocess div identity\n", __func__,
        __LINE__);
      free(cpp_event);
      return NULL;
    }
    cpp_event->hw_process_flag = FALSE;
    cpp_event->type = CPP_MODULE_EVENT_DIVERT_BUF;

    /* copy isp buf and other data from the mct event */
    memcpy(&(cpp_event->u.divert_buf_data.isp_buf_divert),
      (isp_buf_divert_t*)(isp_buf), sizeof(isp_buf_divert_t));
    cpp_event->u.divert_buf_data.div_identity = div_identity;
    cpp_event->u.divert_buf_data.isp_buf_divert.identity = identity;
    cpp_event->u.divert_buf_data.isp_buf_divert.pass_through = 1;
  }
  return cpp_event;
}

/* cpp_module_send_buf_divert_event:
 *
 **/
static int32_t cpp_module_send_buf_divert_event(mct_module_t* module,
  unsigned int identity, isp_buf_divert_t *isp_buf)
{
  uint32_t                    identity_list[2];
  uint32_t                    identity_list_size = 0;

  if(!module || !isp_buf) {
    CDBG_ERROR("%s:%d, failed, module=%p, isp_buf=%p\n", __func__, __LINE__,
      module, isp_buf);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  struct timeval tv1, tv2;
  gettimeofday(&tv1, NULL);
  uint32_t frame_id = isp_buf->buffer.sequence;
  int32_t ret = 0;

  CDBG("%s:%d frame id %d and identity 0x%x\n",
    __func__, __LINE__, frame_id,identity);
  /* get stream parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_stream_params_t *linked_stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, identity, &session_params,
     &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  linked_stream_params = stream_params->linked_stream_params;

  cpp_module_stream_params_t *linked_stream_list[2];
  uint32_t unproc_div_identity=0x00;
  boolean duplicate_output=FALSE;
  boolean unproc_div_required=FALSE;

  identity_list[0] = PPROC_INVALID_IDENTITY;
  identity_list[1] = PPROC_INVALID_IDENTITY;
  linked_stream_list[0] = NULL;
  linked_stream_list[1] = NULL;

  /* note: unlock these on all return paths */
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));

  /* decide processing requirements based on the stream params */
  if(linked_stream_params) { /* linked stream case */
    PTHREAD_MUTEX_LOCK(&(linked_stream_params->mutex));
    /* if both streams in the pair are off, drop frame */
    if (stream_params->is_stream_on == FALSE &&
        linked_stream_params->is_stream_on == FALSE) {
      CDBG_ERROR("%s:%d, stream is off, drop frame and piggy-back ACK\n",
        __func__, __LINE__);
      isp_buf->ack_flag = TRUE;
      isp_buf->is_buf_dirty = 1;
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
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
    else if(stream_params->is_stream_on == FALSE &&
        linked_stream_params->is_stream_on == TRUE) {
      /* only one pass required on linked identity */
      linked_stream_list[0] = linked_stream_params;
      identity_list[0] = linked_stream_params->identity;
      identity_list_size = 1;
    }
    /* if both streams are on */
    else if(stream_params->is_stream_on == TRUE &&
        linked_stream_params->is_stream_on == TRUE) {
      linked_stream_list[0] = stream_params;
      identity_list[0] = stream_params->identity;
      identity_list_size = 1;
      /* if duplicate outputs are possible, both streams can be satisfied with
         single hw pass */
      if (stream_params->hw_params.duplicate_output == TRUE) {
        duplicate_output = TRUE;
      } else {
        /* first pass on current identity and second pass on linked identity */
        linked_stream_list[1] = linked_stream_params;
        identity_list[1] = linked_stream_params->identity;
        identity_list_size = 2;
      }
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
    cpp_module_get_divert_info(&identity_list[0], identity_list_size,
    linked_stream_list[0]->div_config);
  if(!divert_info) {
    CDBG_ERROR("%s:%d, cpp_module_get_divert_info() failed\n",
      __func__, __LINE__);
    if(linked_stream_params)
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    return -EFAULT;
  }

  /* decide if processed/unprocessed divert is required.
     Currently, only one kind of divert is supported. */
  CDBG("%s:%d, ###CPP divert flag %x",
    __func__, __LINE__, divert_info->divert_flags);
  if(divert_info->divert_flags & PPROC_DIVERT_UNPROCESSED) {
    unproc_div_required = TRUE;
    unproc_div_identity = divert_info->div_unproc_identity;
  }

  /* create a key for ack with original event identity, this key will be
     put in all corresponding events in queue and used to release the ack */
  cpp_module_ack_key_t key;
  key.identity = identity;
  key.buf_idx = isp_buf->buffer.index;
  key.channel_id = isp_buf->channel_id;
  key.frame_id = isp_buf->buffer.sequence;
  key.meta_data = isp_buf->meta_data;

  /* Decide the events to be queued to process this buffer */
  int event_idx = 0, num_events = 0;
  /* based on configuration, at max 3 events are queued for one buffer */
  cpp_module_event_t *cpp_event[3];

  int32_t i = 0, j = 0;
  int32_t num_passes = 0;
  boolean skip_frame = FALSE;

  if(divert_info->num_passes != 0)
    isp_buf->is_cpp_processed = TRUE;

   /* Step 1. if unprocessed divert is needed, add an event for that */
  if(unproc_div_required == TRUE) {
    cpp_event[event_idx] = cpp_module_create_cpp_event(key, NULL, isp_buf,
      identity, unproc_div_identity, duplicate_output);
    if(!cpp_event[event_idx]) {
      CDBG_ERROR("%s:%d, malloc() failed\n", __func__, __LINE__);
      if(linked_stream_params)
        PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      return -ENOMEM;
    }
    event_idx++;
  }

  /* Step 2. Based on the number of process identities set in divert config,
     generate cpp events accordingly */
  for (i = 0; i < divert_info->num_passes; i++) {
    for (j = 0; j < 2; j++) {
      if (!linked_stream_list[j] ||
        (linked_stream_list[j]->identity != divert_info->proc_identity[i])) {
        continue;
      }

      skip_frame = FALSE;
      /* decide if skip is required for HFR */
      if (linked_stream_list[j]->hfr_skip_info.skip_required) {
        if ((cpp_decide_hfr_skip(frame_id -
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
          boolean final_dup_output = duplicate_output;
          if (duplicate_output &&
            linked_stream_list[j]->linked_stream_params &&
            linked_stream_list[j]->linked_stream_params\
            ->hfr_skip_info.skip_required) {
            if ((cpp_decide_hfr_skip(frame_id -
              linked_stream_list[j]->linked_stream_params\
              ->hfr_skip_info.frame_offset,
              linked_stream_list[j]->linked_stream_params\
              ->hfr_skip_info.skip_count)) == TRUE) {
              /* duplication not required as linked stream has to be skipped */
              final_dup_output = FALSE;
              CDBG("%s:%d, disable duplication "
                "skip_count=%d, offset=%d frame_id=%d for identity=0x%x",
                __func__, __LINE__,
                linked_stream_list[j]->linked_stream_params\
                ->hfr_skip_info.skip_count,
                linked_stream_list[j]->linked_stream_params\
                ->hfr_skip_info.frame_offset,
                frame_id, linked_stream_list[j]->identity);
            }
        }

        cpp_event[event_idx] = cpp_module_create_cpp_event(key,
          &(linked_stream_list[j]->hw_params), isp_buf,
          linked_stream_list[j]->identity,
          divert_info->div_proc_identity[i],final_dup_output);
        if(!cpp_event[event_idx]) {
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

  if (divert_info->num_passes != num_passes) {
    CDBG_ERROR("%s:%d] failed error in accessing stream params\n", __func__,
      __LINE__);
    if(linked_stream_params)
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    return -EFAULT;
  }

  num_events = event_idx;
  /* if no events needs to be queued, do a piggy-back ACK */
  if (num_events == 0) {
    isp_buf->ack_flag = TRUE;
    isp_buf->is_buf_dirty = 1;
    if(linked_stream_params)
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    CDBG_ERROR("%s:%d] buffer event received with no divert config\n",
      __func__, __LINE__);
    return 0;
  }
  /* before queuing any events, first put corresponding ACK in the ack_list */
  cpp_module_put_new_ack_in_list(ctrl, key, 1, num_events, isp_buf);

  /* now enqueue all events one by one in priority queue */
  int rc;
  for(i=0; i<num_events; i++) {
    rc = cpp_module_enq_event(module, cpp_event[i], stream_params->priority);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed, i=%d\n", __func__, __LINE__, i);
      if(linked_stream_params)
        PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      return -EFAULT;
    }
  }
  if(linked_stream_params)
    PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  /* notify the thread about this new events */
  cpp_thread_msg_t msg;
  msg.type = CPP_THREAD_MSG_NEW_EVENT_IN_Q;
  cpp_module_post_msg_to_thread(module, msg);

  gettimeofday(&tv2, NULL);
  CDBG_LOW("%s:%d, downstream event time = %6ld us, ", __func__, __LINE__,
    (tv2.tv_sec - tv1.tv_sec)*1000000L +
    (tv2.tv_usec - tv1.tv_usec));
  return 0;
}

/** cpp_module_handle_buf_divert_event:
 *
 * Description:
 *  Handle the MCT_EVENT_MODULE_BUF_DIVERT event. First put corresponding
 *  acknowledgement in a list which will be sent later. Depending on the
 *  stream's parameters, divert and processing events are added in
 *  cpp's priority queue. cpp_thread will pick up these events one by one in
 *  order and when all events corresponding to the ACK are processed,
 *  the ACK will be removed from list and will be sent upstream.
 *
 **/
int32_t cpp_module_handle_buf_divert_event(mct_module_t* module,
  mct_event_t* event)
{
  cpp_module_ctrl_t           *ctrl = NULL;
  isp_buf_divert_t            *isp_buf = NULL;
  uint32_t                     frame_id = 0;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *linked_stream_params = NULL;
  cpp_module_frame_hold_t     *frame_hold = NULL;
  cpp_module_dis_hold_t       *dis_hold = NULL;

  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }

  ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
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
    ret = cpp_module_send_event_downstream(ctrl->p_module, event);
    if (ret < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    }
    return ret;
  }

  frame_id = isp_buf->buffer.sequence;

  CDBG("%s:%d received buffer divert for %d and identity 0x%x\n",
    __func__, __LINE__, frame_id,event->identity);
  cpp_module_get_params_for_identity(ctrl, event->identity, &session_params,
     &stream_params);
  if (!session_params || !stream_params) {
    CDBG_ERROR("%s:%d: failed params %p %p\n", __func__, __LINE__,
      session_params, stream_params);
    return -EFAULT;
  }

  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));
  /* Check whether DIS is enabled */
  if (session_params->dis_enable == 0) {
    CDBG("%s:%d send %d for processing\n", __func__, __LINE__,
      isp_buf->buffer.sequence);
    /* DIS is disabled. Send frame for processing */
    cpp_module_send_buf_divert_event(module, event->identity, isp_buf);
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
        cpp_module_send_buf_divert_event(module, frame_hold->identity,
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
        cpp_module_send_buf_divert_event(module, event->identity, isp_buf);
      } else if(stream_params->is_stream_on ||
          linked_stream_params->is_stream_on){
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
      cpp_module_send_buf_divert_event(module, event->identity, isp_buf);
    }
  } else {
     CDBG("%s:%d DIS crop event not sent yet %d for processing\n",
      __func__, __LINE__, isp_buf->buffer.sequence);
    /* DIS is disabled. Send frame for processing */
    cpp_module_send_buf_divert_event(module, event->identity, isp_buf);

  }
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));

  return 0;
}


/* cpp_module_handle_isp_out_dim_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_isp_out_dim_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
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
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, event->identity,
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
  stream_params->hw_params.input_info.plane_offsets[0] =
      stream_info->buf_planes.plane_info.mp[0].offset;
  stream_params->hw_params.input_info.plane_offsets[1] =
      stream_info->buf_planes.plane_info.mp[1].offset;

  /* format info */
  if (stream_info->fmt == CAM_FORMAT_YUV_420_NV12 ||
    stream_info->fmt == CAM_FORMAT_YUV_420_NV12_VENUS) {
    stream_params->hw_params.input_info.plane_fmt = CPP_PARAM_PLANE_CBCR;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_420_NV21) {
    stream_params->hw_params.input_info.plane_fmt = CPP_PARAM_PLANE_CRCB;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_422_NV16) {
    stream_params->hw_params.input_info.plane_fmt = CPP_PARAM_PLANE_CBCR422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_422_NV61) {
    stream_params->hw_params.input_info.plane_fmt = CPP_PARAM_PLANE_CRCB422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_420_YV12) {
    stream_params->hw_params.input_info.plane_fmt = CPP_PARAM_PLANE_CRCB420;
  } else {
    CDBG_ERROR("%s:%d] Format not supported\n", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    return -EINVAL;
  }
  /* init crop info */
  stream_params->hw_params.crop_info.stream_crop.x = 0;
  stream_params->hw_params.crop_info.stream_crop.y = 0;
  stream_params->hw_params.crop_info.stream_crop.dx = stream_info->dim.width;
  stream_params->hw_params.crop_info.stream_crop.dy = stream_info->dim.height;
  stream_params->hw_params.crop_info.is_crop.x = 0;
  stream_params->hw_params.crop_info.is_crop.y = 0;
  stream_params->hw_params.crop_info.is_crop.dx = stream_info->dim.width;
  stream_params->hw_params.crop_info.is_crop.dy = stream_info->dim.height;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  cpp_module_set_clock_freq(ctrl, stream_params, 1);
  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* cpp_module_handle_aec_update_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_aec_update_event(mct_module_t* module,
  mct_event_t* event)
{
  stats_update_t              *stats_update;
  aec_update_t                *aec_update;
  float                        aec_trigger_input;
  chromatix_parms_type        *chromatix_ptr;
  wavelet_denoise_type        *wavelet_denoise;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  int32_t                      rc;

  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }

  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  CDBG("%s:%d identity=0x%x", __func__, __LINE__, event->identity);

  stats_update =
      (stats_update_t *)event->u.module_event.module_event_data;
  aec_update = &stats_update->aec_update;

  if (stats_update->flag & STATS_UPDATE_AEC) {
    cpp_hardware_params_t        *running_hw_params = NULL;
    cpp_hardware_params_t        *linked_hw_params = NULL;

    /* get stream parameters based on the event identity */
    cpp_module_get_params_for_identity(ctrl, event->identity,
      &session_params, &stream_params);
    if(!stream_params) {
      CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
      return -EFAULT;
    }

    running_hw_params = &stream_params->hw_params;
    if (stream_params->linked_stream_params)
    {
      linked_hw_params = &stream_params->linked_stream_params->hw_params;
      if (stream_params->is_stream_on == FALSE)
      {
        if (stream_params->linked_stream_params->is_stream_on == TRUE)
        {
          //swap
          running_hw_params = &stream_params->linked_stream_params->hw_params;
          linked_hw_params = &stream_params->hw_params;
        }
      }
    }

    /* For WNR update */
    /* 1. AEC update needs to interpolate WNR values and store the output.
       2. For interpolation we also need,
         a. Chromatix ptr from session params.
         b. Session's SET_PARAM (enable/disable). */
    /* 3. Determine the control method to select lux_idx or gain */
    session_params->aec_trigger.gain = aec_update->real_gain;
    session_params->aec_trigger.lux_idx = aec_update->lux_idx;
    running_hw_params->aec_trigger.lux_idx =
      session_params->aec_trigger.lux_idx;
    running_hw_params->aec_trigger.gain =
      session_params->aec_trigger.gain;

    chromatix_ptr = stream_params->module_chromatix.chromatixPtr;

    if (running_hw_params->denoise_enable == TRUE) {
      cpp_hw_params_update_wnr_params(chromatix_ptr,
        running_hw_params, &session_params->aec_trigger);
    }

    /* For ASF update */
    /* 1. AEC update needs to interpolate ASF values and store the output.
       2. For interpolation we also need,
         a. Chromatix ptr from session params.
         b. Session's SET_PARAM (sharpness level). */
      cpp_hw_params_asf_interpolate(ctrl->cpphw, running_hw_params, chromatix_ptr,
        &session_params->aec_trigger);

    /* Check for existence of linked_stream_params.
       If it exist apply the same aec update */
    if (stream_params->linked_stream_params) {
      memcpy(&(linked_hw_params->denoise_info),
        &(running_hw_params->denoise_info),
        sizeof(running_hw_params->denoise_info));
      memcpy(&(linked_hw_params->asf_info),
        &(running_hw_params->asf_info),
        sizeof(running_hw_params->asf_info));
      linked_hw_params->aec_trigger.lux_idx =
        session_params->aec_trigger.lux_idx;
      linked_hw_params->aec_trigger.gain =
        session_params->aec_trigger.gain;

    }
  }

  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* cpp_module_handle_chromatix_ptr_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_chromatix_ptr_event(mct_module_t* module,
  mct_event_t* event)
{
  modulesChromatix_t *chromatix_param;
  int32_t rc;

  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  CDBG("%s:%d, identity=0x%x\n", __func__, __LINE__, event->identity);
  /* get stream parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  chromatix_param =
    (modulesChromatix_t *)event->u.module_event.module_event_data;
  /* Update the chromatix ptr in session params.
     This chromatix ptr will be used by all streams in this session */
  if(stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    uint32_t i;

    for( i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      if (session_params->stream_params[i] ) {
        session_params->stream_params[i]->module_chromatix = *chromatix_param;
      }
    }
  } else {
    stream_params->module_chromatix = *chromatix_param;
  }

  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* cpp_module_handle_stream_crop_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_stream_crop_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  cpp_hardware_params_t *hw_params = NULL;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
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
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  hw_params = &stream_params->hw_params;
  if (((stream_crop->x  + stream_crop->crop_out_x) <=
    (uint32_t)(hw_params->input_info.width)) &&
    ((stream_crop->y + stream_crop->crop_out_y) <=
    (uint32_t)(hw_params->input_info.height))) {
      PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
      stream_params->hw_params.crop_info.stream_crop.x = stream_crop->x;
      stream_params->hw_params.crop_info.stream_crop.y = stream_crop->y;
      stream_params->hw_params.crop_info.stream_crop.dx = stream_crop->crop_out_x;
      stream_params->hw_params.crop_info.stream_crop.dy = stream_crop->crop_out_y;
      stream_params->hw_params.isp_width_map = stream_crop->width_map;
      stream_params->hw_params.isp_height_map = stream_crop->height_map;
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      CDBG("%s:%d stream_crop.x=%d, stream_crop.y=%d, stream_crop.dx=%d,"
        " stream_crop.dy=%d, identity=0x%x,width_map = %d,height_map = %d",
        __func__,__LINE__, stream_crop->x, stream_crop->y,
        stream_crop->crop_out_x, stream_crop->crop_out_y,
        event->identity,stream_crop->width_map,
        stream_crop->height_map);
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  } else {
    CDBG_HIGH("%s:%d frame id %d stream_crop.x=%d, stream_crop.y=%d, stream_crop.dx=%d,"
             " stream_crop.dy=%d, width %d height %d identity=0x%x", __func__, __LINE__,
             stream_crop->frame_id, stream_crop->x, stream_crop->y,
             stream_crop->crop_out_x,stream_crop->crop_out_y, hw_params->input_info.width,
             hw_params->input_info.height, event->identity);
  }

  /* This crop info event cannot be sent out as it is. This need to be a
     new crop event based on whether CPP can handle the requested crop.
     Only residual crop needs to be sent out */
#if 1
  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
#endif
  return 0;
}

/* cpp_module_handle_dis_update_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_dis_update_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
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
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *linked_stream_params = NULL;
  cpp_module_frame_hold_t     *frame_hold = FALSE;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CDBG_ERROR("%s:%d, failed params %p %p\n", __func__, __LINE__,
      session_params, stream_params);
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
   /* Update is_crop in stream_params */
   if (((is_update->x  + is_update->width) <=
     stream_params->hw_params.input_info.width) &&
   ((is_update->y + is_update->height) <=
     stream_params->hw_params.input_info.height)) {
     CDBG("%s:%d frame id %d x %d y %d dx %d dy %d"
      " width %d height %d iden 0x%x\n",
      __func__, __LINE__, is_update->frame_id,
       is_update->x, is_update->y, is_update->width, is_update->height,
       stream_params->hw_params.input_info.width, stream_params->hw_params.input_info.height,
       stream_params->identity);
       stream_params->hw_params.crop_info.is_crop.x = is_update->x;
       stream_params->hw_params.crop_info.is_crop.y = is_update->y;
       stream_params->hw_params.crop_info.is_crop.dx = is_update->width;
       stream_params->hw_params.crop_info.is_crop.dy = is_update->height;
   } else {
     CDBG_HIGH("%s:%d frame id %d x %d y %d dx %d dy %d"
      " width %d height %d iden 0x%x\n",
      __func__, __LINE__, is_update->frame_id,
       is_update->x, is_update->y, is_update->width, is_update->height,
       stream_params->hw_params.input_info.width, stream_params->hw_params.input_info.height,
       stream_params->identity);
     PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
     return 0;
   }

  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));
   /* Update frame id in session_params */
  session_params->dis_hold.is_valid = TRUE;
  session_params->dis_hold.dis_frame_id = is_update->frame_id;
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));

  /* Update is_crop in linked_stream_params */
  linked_stream_params = stream_params->linked_stream_params;
  if (linked_stream_params) {
    PTHREAD_MUTEX_LOCK(&(linked_stream_params->mutex));
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
    /* Send this frame for CPP processing */
    cpp_module_send_buf_divert_event(module, frame_hold->identity,
      &frame_hold->isp_buf);
    /* Update frame hold flag to FALSE */
    frame_hold->is_frame_hold = FALSE;
  }

  CDBG_LOW("%s:%d is_crop.x=%d, is_crop.y=%d, is_crop.dx=%d, is_crop.dy=%d,"
    " identity=0x%x", __func__, __LINE__, is_update->x, is_update->y,
    is_update->width, is_update->height, event->identity);

  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));

  /* TODO: Review where DIS info needs to be sent out. */
#if 0
  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
#endif
  return 0;
}

/* cpp_module_handle_stream_cfg_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_stream_cfg_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
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
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* apply this to all streams for that session */
  int i;
  for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
    if(session_params->stream_params[i]) {
      PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
      session_params->stream_params[i]->hfr_skip_info.frame_offset =
        sensor_out_info->num_frames_skip + 1;
      session_params->stream_params[i]->hfr_skip_info.input_fps =
        sensor_out_info->max_fps;
      session_params->stream_params[i]->hw_params.sensor_dim_info.width =
        sensor_out_info->dim_output.width;
      session_params->stream_params[i]->hw_params.sensor_dim_info.height =
        sensor_out_info->dim_output.height;
      cpp_module_update_hfr_skip(session_params->stream_params[i]);
      PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
      CDBG("%s:%d frame_offset=%d, input_fps=%.2f, identity=0x%x",
        __func__, __LINE__,
        session_params->stream_params[i]->hfr_skip_info.frame_offset,
        session_params->stream_params[i]->hfr_skip_info.input_fps,
        session_params->stream_params[i]->identity);
    }
  }
  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* cpp_module_handle_fps_update_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_fps_update_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  sensor_fps_update_t *fps_update =
    (sensor_fps_update_t *)(event->u.module_event.module_event_data);

  /* get stream parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* apply this to all streams for that session */
  int i;
  for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
    if(session_params->stream_params[i]) {
      PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
      session_params->stream_params[i]->hfr_skip_info.input_fps =
        fps_update->max_fps;
      cpp_module_update_hfr_skip(session_params->stream_params[i]);
      PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
      CDBG("%s:%d input_fps=%.2f, identity=0x%x",
        __func__, __LINE__,
        session_params->stream_params[i]->hfr_skip_info.input_fps,
        session_params->stream_params[i]->identity);
    }
  }
  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* cpp_module_handle_set_output_buff_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_set_output_buff_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t               rc;
  mct_stream_map_buf_t *img_buf;

  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  img_buf = (mct_stream_map_buf_t *)(event->u.module_event.module_event_data);
  if (!img_buf) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* get stream parameters based on the event identity */
  cpp_module_stream_params_t *stream_params;
  cpp_module_session_params_t *session_params;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->hw_params.output_buffer_info.fd = img_buf->buf_planes[0].fd;
  stream_params->hw_params.output_buffer_info.index = img_buf->buf_index;
  stream_params->hw_params.output_buffer_info.native_buff = TRUE;
  stream_params->hw_params.output_buffer_info.offset = 0;
  stream_params->hw_params.output_buffer_info.processed_divert = 0;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  return 0;
}

/**cpp_module_handle_div_info_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_div_info_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
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
       sizeof(MCT_OBJECT_NAME(module))) != 0) {
    rc = cpp_module_send_event_downstream(module, event);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
        __func__, __LINE__, event->u.module_event.type, event->identity);
      return -EFAULT;
    }
    return 0;
  }
  /* get stream parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  cpp_module_set_divert_cfg_entry(event->identity, div_cfg->update_mode,
    &div_cfg->divert_info, stream_params->div_config);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  return 0;
}


/* cpp_module_handle_load_chromatix_event:
 *
 **/
int32_t cpp_module_handle_load_chromatix_event(mct_module_t* module,
  mct_event_t* event)
{
    int32_t rc;
    if(!module || !event) {
      CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
        module, event);
      return -EINVAL;
    }
    cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
    if(!ctrl) {
      CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
      return -EFAULT;
    }

    int  i;
    modulesChromatix_t  *chromatix_param =
        (modulesChromatix_t  *)(event->u.module_event.module_event_data);
    /* get stream parameters */
    cpp_module_session_params_t* session_params = NULL;
    cpp_module_stream_params_t*  stream_params = NULL;

    cpp_module_get_params_for_identity(ctrl, event->identity,
      &session_params, &stream_params);
    if(!session_params) {
      CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
      return -EFAULT;
    }

    if(stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
      uint32_t i;

      for( i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
        if (session_params->stream_params[i] ) {
          session_params->stream_params[i]->module_chromatix.chromatixComPtr =
            chromatix_param->chromatixComPtr;
          session_params->stream_params[i]->module_chromatix.chromatixPtr =
            chromatix_param->chromatixPtr;
        }
      }
    } else {
      stream_params->module_chromatix.chromatixComPtr =
        chromatix_param->chromatixComPtr;
      stream_params->module_chromatix.chromatixPtr =
        chromatix_param->chromatixPtr;
    }
    /* apply this to all streams in session */
    for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
      if(session_params->stream_params[i]) {

        cpp_hw_params_update_wnr_params(chromatix_param->chromatixPtr,
          &session_params->stream_params[i]->hw_params,
          &session_params->aec_trigger);

        cpp_hw_params_asf_interpolate(ctrl->cpphw,
          &session_params->stream_params[i]->hw_params,
          chromatix_param->chromatixPtr,&session_params->aec_trigger);
      }
    }

    return 0;
}


/* cpp_module_set_parm_sharpness:
 *
 **/
static int32_t cpp_module_set_parm_sharpness(cpp_module_ctrl_t *ctrl,
  uint32_t identity, int32_t value)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  chromatix_parms_type        *chromatix_ptr;
  float                        trigger_input;
  int                          i = 0;

  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  if (session_params->sensor_format != FORMAT_BAYER) {
    session_params->hw_params.sharpness_level = 0.0f;
    CDBG_LOW("%s:%d,Sharpness feature disabled,sharpness_level = 0.0f",
      __func__,
      __LINE__);
  } else {
    session_params->hw_params.sharpness_level = cpp_get_sharpness_ratio(value);
  }

  if (session_params->hw_params.sharpness_level ==
    0.0f) {
    if (session_params->hw_params.asf_mode ==
      CPP_PARAM_ASF_DUAL_FILTER) {
      session_params->hw_params.asf_mode =
        CPP_PARAM_ASF_OFF;
    }
  } else {
    if (session_params->hw_params.asf_mode ==
      CPP_PARAM_ASF_OFF) {
      session_params->hw_params.asf_mode =
        CPP_PARAM_ASF_DUAL_FILTER;
    }
  }
  CDBG("%s:%d] value:%d, sharpness_level:%f\n", __func__, __LINE__, value,
    session_params->hw_params.sharpness_level);

  /* apply this to all streams in session */
  for(i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
    if(session_params->stream_params[i]) {
      PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
      session_params->stream_params[i]->hw_params.sharpness_level =
        cpp_get_sharpness_ratio(value);
      if (session_params->stream_params[i]->hw_params.sharpness_level ==
        0.0f) {
        if (session_params->stream_params[i]->hw_params.asf_mode ==
          CPP_PARAM_ASF_DUAL_FILTER) {
          session_params->stream_params[i]->hw_params.asf_mode =
            CPP_PARAM_ASF_OFF;
        }
      } else {
        if (session_params->stream_params[i]->hw_params.asf_mode ==
          CPP_PARAM_ASF_OFF) {
          session_params->stream_params[i]->hw_params.asf_mode =
            CPP_PARAM_ASF_DUAL_FILTER;
        }
      }
      chromatix_ptr = stream_params->module_chromatix.chromatixPtr;
      cpp_hw_params_asf_interpolate(ctrl->cpphw,
        &session_params->stream_params[i]->hw_params, chromatix_ptr,
        &session_params->aec_trigger);
      PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
    }
  }
  return 0;
}



/* cpp_module_set_parm_sceneMode:
 *
 **/
static int32_t cpp_module_set_parm_sceneMode(cpp_module_ctrl_t *ctrl,
  uint32_t identity, int32_t value)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;

  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  if (value != CAM_SCENE_MODE_OFF) {
    stream_params->hw_params.scene_mode_on = 1;
  } else {
    stream_params->hw_params.scene_mode_on = 0;
  }

  session_params->hw_params.scene_mode_on =
    stream_params->hw_params.scene_mode_on;

  return 0;
}


/* cpp_module_set_parm_effect:
 *
 **/
static int32_t cpp_module_set_parm_effect(cpp_module_ctrl_t *ctrl,
  uint32_t identity, int32_t value)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  chromatix_parms_type        *chromatix_ptr;
  float                        trigger_input;
  int                          i;

  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  if (value == CAM_EFFECT_MODE_OFF) {
    if (session_params->hw_params.sharpness_level == 0.0f) {
      session_params->hw_params.asf_mode = CPP_PARAM_ASF_OFF;
    } else {
      session_params->hw_params.asf_mode = CPP_PARAM_ASF_DUAL_FILTER;
    }
  } else {
    switch(value) {
    case CAM_EFFECT_MODE_EMBOSS:
      session_params->hw_params.asf_mode = CPP_PARAM_ASF_EMBOSS;
      break;
    case CAM_EFFECT_MODE_SKETCH:
      session_params->hw_params.asf_mode = CPP_PARAM_ASF_SKETCH;
      break;
    case CAM_EFFECT_MODE_NEON:
      session_params->hw_params.asf_mode = CPP_PARAM_ASF_NEON;
      break;
    default:
      CDBG_ERROR("%s:%d, invalid mode\n", __func__, __LINE__);
      if (session_params->hw_params.sharpness_level == 0.0f) {
        session_params->hw_params.asf_mode = CPP_PARAM_ASF_OFF;
      } else {
        session_params->hw_params.asf_mode = CPP_PARAM_ASF_DUAL_FILTER;
      }
    }
  }
  if (session_params->sensor_format != FORMAT_BAYER) {
    session_params->hw_params.asf_mode = CPP_PARAM_ASF_OFF;
    session_params->hw_params.sharpness_level = 0.0f;
    CDBG_LOW("%s:%d,Sharpness feature disabled",__func__,__LINE__);
  }
  CDBG_LOW("%s:%d] effect:%d\n", __func__, __LINE__,
    session_params->hw_params.asf_mode);
  /* TODO: SET_PARAM will be triggered intially before any streamon etc.,
     and also when ever there is UI change */
  chromatix_ptr = stream_params->module_chromatix.chromatixPtr;
  /* apply this to all streams in session */
  for(i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
    if(session_params->stream_params[i]) {
      PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
      session_params->stream_params[i]->hw_params.asf_mode =
        session_params->hw_params.asf_mode;
      PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
      cpp_hw_params_asf_interpolate(ctrl->cpphw,
        &session_params->stream_params[i]->hw_params, chromatix_ptr,
        &session_params->aec_trigger);
    }
  }
  return 0;
}

/* cpp_module_set_parm_denoise:
 *
 **/
static int32_t cpp_module_set_parm_denoise(cpp_module_ctrl_t *ctrl,
  uint32_t identity, cam_denoise_param_t parm)
{
  int                          i;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  chromatix_parms_type        *chromatix_ptr;
  float                        trigger_input;

  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  if (session_params->sensor_format != FORMAT_BAYER) {
    session_params->hw_params.denoise_enable = 0;
  } else {
    session_params->hw_params.denoise_enable = parm.denoise_enable;
  }
  /* TODO: SET_PARAM will be triggered intially before any streamon etc.,
     and also when ever there is UI change */
  chromatix_ptr = stream_params->module_chromatix.chromatixPtr;
  /* apply this to all streams in session */
  for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
    if(session_params->stream_params[i]) {
      PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
      session_params->stream_params[i]->hw_params.denoise_enable =
        parm.denoise_enable;
      PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));

      if (session_params->hw_params.denoise_enable == TRUE) {
        cpp_hw_params_update_wnr_params(chromatix_ptr,
          &session_params->stream_params[i]->hw_params,
          &session_params->aec_trigger);
      }
    }
  }
  return 0;
}

/* cpp_module_set_parm_fps_range:
 *
 **/
static int32_t cpp_module_set_parm_fps_range(cpp_module_ctrl_t *ctrl,
  uint32_t identity, cam_fps_range_t *fps_range)
{
  if((!ctrl) || (!fps_range)){
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if (!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* apply this to all streams where hfr skip is required */
  int i;
  session_params->fps_range.max_fps = fps_range->max_fps;
  session_params->fps_range.min_fps = fps_range->min_fps;
  session_params->fps_range.video_max_fps = fps_range->video_max_fps;
  session_params->fps_range.video_min_fps = fps_range->video_min_fps;
  CDBG("%s:%d, max_fps %f video_max_fps %f", __func__, __LINE__,
    fps_range->max_fps, fps_range->video_max_fps);
  for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
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
      cpp_module_update_hfr_skip(session_params->stream_params[i]);
      }
    PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
    }
  }
  return 0;
}

/* cpp_module_set_parm_rotation:
 *
 **/
static int32_t cpp_module_set_parm_rotation(cpp_module_ctrl_t *ctrl,
  uint32_t identity, cam_rotation_t rotation)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  CDBG_ERROR("%s:%d,SET UP STREAM ROTATION\n", __func__, __LINE__);
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  if (rotation == ROTATE_0) {
    stream_params->hw_params.rotation = 0;
  } else if (rotation == ROTATE_90) {
    stream_params->hw_params.rotation = 1;
  } else if (rotation == ROTATE_180) {
    stream_params->hw_params.rotation = 2;
  } else if (rotation == ROTATE_270) {
    stream_params->hw_params.rotation = 3;
  }
  cpp_module_set_output_duplication_flag(stream_params);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  return 0;
}


/* cpp_module_set_parm_dis:
 *
 **/
static int32_t cpp_module_set_parm_dis(cpp_module_ctrl_t *ctrl,
  uint32_t identity, int32_t dis_enable)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity, &session_params,
     &stream_params);
  if(!session_params) {
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

/* cpp_module_set_parm_flip:
 *    @ctrl: cpp module control struct
 *    @indentity: current indentity
 *    @flip_mask: new flip mode
 *
 *    Set the flip mode sent form application
 *
 *    Returns 0 on succes or EFAULT if some of the parameters
 *      is missing.
 **/
static int32_t cpp_module_set_parm_flip(cpp_module_ctrl_t *ctrl,
  uint32_t identity, int32_t flip_mask)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->hw_params.mirror = flip_mask;
  cpp_module_set_output_duplication_flag(stream_params);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  return 0;
}

/* cpp_module_handle_set_parm_event:
 *
 * Description:
 *   Handle the set_parm event.
 **/
int32_t cpp_module_handle_set_parm_event(mct_module_t* module,
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
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  int32_t rc;
  switch (ctrl_parm->type) {
  case CAM_INTF_PARM_SHARPNESS: {
    if(!(ctrl_parm->parm_data)) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return -EFAULT;
    }
    int32_t value = *(int32_t*)(ctrl_parm->parm_data);
    CDBG("%s:%d, CAM_INTF_PARM_SHARPNESS, value=%d, identity=0x%x",
      __func__, __LINE__, value, event->identity);
    rc = cpp_module_set_parm_sharpness(ctrl, event->identity, value);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_BESTSHOT_MODE: {
    if(!(ctrl_parm->parm_data)) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return -EFAULT;
    }
    int32_t value = *(int32_t*)(ctrl_parm->parm_data);
    CDBG("%s:%d, CAM_INTF_PARM_BESTSHOT_MODE, value=%d, identity=0x%x",
      __func__, __LINE__, value, event->identity);
    rc = cpp_module_set_parm_sceneMode(ctrl, event->identity, value);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_EFFECT: {
    if(!(ctrl_parm->parm_data)) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return -EFAULT;
    }
    int32_t value = *(int32_t*)(ctrl_parm->parm_data);
    CDBG("%s:%d, CAM_INTF_PARM_EFFECT, value=%d, identity=0x%x",
      __func__, __LINE__, value, event->identity);
    rc = cpp_module_set_parm_effect(ctrl, event->identity, value);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_WAVELET_DENOISE: {
    if(!(ctrl_parm->parm_data)) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return -EFAULT;
    }
    cam_denoise_param_t parm =
      *(cam_denoise_param_t *)(ctrl_parm->parm_data);
    CDBG("%s:%d, CAM_INTF_PARM_WAVELET_DENOISE, enable=%d, identity=0x%x",
      __func__, __LINE__, parm.denoise_enable, event->identity);
    rc = cpp_module_set_parm_denoise(ctrl, event->identity, parm);
    if(rc < 0) {
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
    rc = cpp_module_set_parm_fps_range(ctrl, event->identity, fps_range);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_ROTATION: {
   if(!(ctrl_parm->parm_data)) {
     CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
     return -EFAULT;
   }

   cam_rotation_t rotation =
          *(cam_rotation_t *)(ctrl_parm->parm_data);
   CDBG_ERROR("%s:%d, CAM_INTF_PARM_ROTATION,rotation %d", __func__,
          __LINE__, rotation);
   rc = cpp_module_set_parm_rotation(ctrl, event->identity, rotation);
   if(rc < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return rc;
   }
   break;
  }
  case CAM_INTF_PARM_DIS_ENABLE: {
    if(!(ctrl_parm->parm_data)) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return -EFAULT;
    }
    int32_t dis_enable =
      *(int32_t *)(ctrl_parm->parm_data);
    CDBG("%s:%d, CAM_INTF_PARM_DIS_ENABLE, enable=%d, identity=0x%x",
      __func__, __LINE__, dis_enable, event->identity);
    rc = cpp_module_set_parm_dis(ctrl, event->identity, dis_enable);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_SET_PP_COMMAND: {
    if(!(ctrl_parm->parm_data)) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return -EFAULT;
    }

    int  i;
    tune_cmd_t *ez_tune_cmd = (tune_cmd_t *)ctrl_parm->parm_data;

    /* get stream parameters */
    cpp_module_session_params_t* session_params = NULL;
    cpp_module_stream_params_t*  stream_params = NULL;

    cpp_module_get_params_for_identity(ctrl, event->identity,
      &session_params, &stream_params);
    if(!session_params) {
      CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
      return -EFAULT;
    }
    CDBG_ERROR("%s:%d, [ILIYA] Received CAM_INTF_PARM_SET_PP_COMMAND event\n"
        "module type %d command %d value %d",
        __func__, __LINE__,ez_tune_cmd->module,ez_tune_cmd->type,ez_tune_cmd->value);
    switch (ez_tune_cmd->module) {
    case PP_MODULE_WNR: {
      if(ez_tune_cmd->type == SET_ENABLE) {
        session_params->hw_params.ez_tune_wnr_enable = ez_tune_cmd->value;
        session_params->diag_params.control_wnr.enable = ez_tune_cmd->value;
        /* apply this to all streams in session */
        for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
          if(session_params->stream_params[i]) {
            session_params->stream_params[i]->hw_params.ez_tune_wnr_enable =
                ez_tune_cmd->value;
          }
        }
      }
      else if(ez_tune_cmd->type == SET_CONTROLENABLE) {
        session_params->diag_params.control_wnr.cntrlenable =
          ez_tune_cmd->value;
        /* apply this to all streams in session */
        for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
          if(session_params->stream_params[i]) {
            session_params->stream_params[i]->hw_params.denoise_lock =
                ez_tune_cmd->value;
          }
        }
      }
      break;
    }
    case PP_MODULE_ASF: {
      if(ez_tune_cmd->type == SET_ENABLE) {
        session_params->hw_params.ez_tune_asf_enable = ez_tune_cmd->value;
        session_params->diag_params.control_asf7x7.enable =
          ez_tune_cmd->value;
        /* apply this to all streams in session */
        for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
          if(session_params->stream_params[i]) {
            session_params->stream_params[i]->hw_params.ez_tune_asf_enable =
                ez_tune_cmd->value;
          }
        }
      }
      else if(ez_tune_cmd->type == SET_CONTROLENABLE) {
        session_params->diag_params.control_asf7x7.cntrlenable =
          ez_tune_cmd->value;
        /* apply this to all streams in session */
        for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
          if(session_params->stream_params[i]) {
            session_params->stream_params[i]->hw_params.asf_lock =
                ez_tune_cmd->value;
          }
        }
      }
      break;
    }
    case PP_MODULE_ALL: {
      if(ez_tune_cmd->type == SET_STATUS) {
        session_params->hw_params.diagnostic_enable =
          ez_tune_cmd->value;
        /* apply this to all streams in session */
        for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
          if(session_params->stream_params[i]) {
            session_params->stream_params[i]->hw_params.diagnostic_enable =
                ez_tune_cmd->value;
          }
        }
      }
      break;
    }
    default: {
      break;
    }
  }
    break;
  }
  default:
    break;
  }

  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}


/* cpp_module_handle_set_stream_parm_event:
 *
 *  @module: mct module structure for cpp module
 *  @event: incoming event
 *
 *    Handles stream_param events. Such as SET_FLIP_TYPE.
 *
 *    Returns 0 on success.
 **/
int32_t cpp_module_handle_set_stream_parm_event(mct_module_t* module,
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
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  switch(param->type) {
  case CAM_STREAM_PARAM_TYPE_SET_FLIP: {
    int32_t flip_mask = param->flipInfo.flip_mask;

    rc = cpp_module_set_parm_flip(ctrl, event->identity, flip_mask);
    if(rc) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return rc;
    }
    break;
  }
  default:
    rc = cpp_module_send_event_downstream(module, event);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed, control_event_type=%d, identity=0x%x",
        __func__, __LINE__, event->u.ctrl_event.type, event->identity);
      return -EFAULT;
    }
    break;
  }

  return 0;
}

/* cpp_module_handle_update_buf_info
 *
 *  @ module - structure that holds current module information.
 *  @ event - structure that holds event data.
 *
 *  Event handler that creates buff info list send it to hardware layer for
 *  process. Send downstream event also.
 *
 *  Returns 0 on success.
 *
**/
int32_t cpp_module_handle_update_buf_info(mct_module_t* module,
  mct_event_t* event)
{
  cpp_module_stream_buff_info_t   stream_buff_info;
  cpp_hardware_stream_buff_info_t hw_strm_buff_info;
  mct_stream_map_buf_t *buf_holder =
    (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  cpp_module_ctrl_t              *ctrl =
    (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  cpp_hardware_cmd_t              cmd;
  boolean                         rc = -EINVAL;


  memset(&stream_buff_info, 0, sizeof(cpp_module_stream_buff_info_t));
  memset(&hw_strm_buff_info, 0, sizeof(cpp_hardware_stream_buff_info_t));

  /* attach the identity */
  stream_buff_info.identity = event->identity;
  /* Apend the new buffer to cpp module's  own list of buffer info */
  if (cpp_module_util_map_buffer_info(buf_holder, &stream_buff_info) == FALSE) {
    CDBG_ERROR("%s:%d, error creating stream buff list\n", __func__,
      __LINE__);
    goto CPP_MODULE_BUF_UPDATE_ERROR1;
  }

  /* create and translate to hardware buffer array */
  hw_strm_buff_info.buffer_info = (cpp_hardware_buffer_info_t *)malloc(
    sizeof(cpp_hardware_buffer_info_t) * stream_buff_info.num_buffs);
  if(NULL == hw_strm_buff_info.buffer_info) {
    CDBG_ERROR("%s:%d, error creating hw buff list\n", __func__,
      __LINE__);
    goto CPP_MODULE_BUF_UPDATE_ERROR1;
  }

  hw_strm_buff_info.identity = stream_buff_info.identity;
  if (mct_list_traverse(stream_buff_info.buff_list,
    cpp_module_util_create_hw_stream_buff, &hw_strm_buff_info) == FALSE) {
    CDBG_ERROR("%s:%d, error creating stream buff list\n", __func__,
      __LINE__);
    goto CPP_MODULE_BUF_UPDATE_ERROR2;
  }

  if(hw_strm_buff_info.num_buffs != stream_buff_info.num_buffs) {
    CDBG_ERROR("%s:%d, error creating stream buff list\n", __func__,
      __LINE__);
    goto CPP_MODULE_BUF_UPDATE_ERROR2;
  }

  cmd.type = CPP_HW_CMD_BUF_UPDATE;
  cmd.u.stream_buff_list = &hw_strm_buff_info;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    goto CPP_MODULE_BUF_UPDATE_ERROR2;
  }
  rc = cpp_module_send_event_downstream(module,event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    goto CPP_MODULE_BUF_UPDATE_ERROR2;
  }

  rc = 0;

CPP_MODULE_BUF_UPDATE_ERROR2:
  free(hw_strm_buff_info.buffer_info);
CPP_MODULE_BUF_UPDATE_ERROR1:
  mct_list_traverse(stream_buff_info.buff_list,
    cpp_module_util_free_buffer_info, &stream_buff_info);
  mct_list_free_list(stream_buff_info.buff_list);

  return rc;
}


/* cpp_module_handle_streamon_event:
 *
 **/
int32_t cpp_module_handle_streamon_event(mct_module_t* module,
  mct_event_t* event)
{
  cpp_module_stream_buff_info_t   stream_buff_info;
  cpp_hardware_stream_buff_info_t hw_strm_buff_info;
  mct_stream_info_t              *streaminfo =
    (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  cpp_module_ctrl_t              *ctrl =
    (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  cpp_hardware_cmd_t              cmd;
  boolean                         rc = -EINVAL;
  chromatix_parms_type           *chromatix_ptr;
  mct_event_t                     new_event;
  stats_get_data_t                stats_get;

  /* get stream parameters */
  cpp_module_session_params_t* session_params = NULL;
  cpp_module_stream_params_t*  stream_params = NULL;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  memset(&stream_buff_info, 0, sizeof(cpp_module_stream_buff_info_t));
  memset(&hw_strm_buff_info, 0, sizeof(cpp_hardware_stream_buff_info_t));

  /* attach the identity */
  stream_buff_info.identity = event->identity;
  /* traverse through the mct stream buff list and create cpp module's
     own list of buffer info */
  if (mct_list_traverse(streaminfo->img_buffer_list,
    cpp_module_util_map_buffer_info, &stream_buff_info) == FALSE) {
    CDBG_ERROR("%s:%d, error creating stream buff list\n", __func__,
      __LINE__);
    goto CPP_MODULE_STREAMON_ERROR1;
  }

  /* create and translate to hardware buffer array */
  hw_strm_buff_info.buffer_info = (cpp_hardware_buffer_info_t *)malloc(
    sizeof(cpp_hardware_buffer_info_t) * stream_buff_info.num_buffs);
  if(NULL == hw_strm_buff_info.buffer_info) {
    CDBG_ERROR("%s:%d, error creating hw buff list\n", __func__,
      __LINE__);
    goto CPP_MODULE_STREAMON_ERROR1;
  }

  hw_strm_buff_info.identity = stream_buff_info.identity;
  if (mct_list_traverse(stream_buff_info.buff_list,
    cpp_module_util_create_hw_stream_buff, &hw_strm_buff_info) == FALSE) {
    CDBG_ERROR("%s:%d, error creating stream buff list\n", __func__,
      __LINE__);
    goto CPP_MODULE_STREAMON_ERROR2;
  }

  if(hw_strm_buff_info.num_buffs != stream_buff_info.num_buffs) {
    CDBG_ERROR("%s:%d, error creating stream buff list\n", __func__,
      __LINE__);
    goto CPP_MODULE_STREAMON_ERROR2;
  }

  cpp_module_set_clock_freq(ctrl, stream_params, 1);

  cmd.type = CPP_HW_CMD_STREAMON;
  cmd.u.stream_buff_list = &hw_strm_buff_info;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    goto CPP_MODULE_STREAMON_ERROR2;
  }
  rc = cpp_module_send_event_downstream(module,event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    goto CPP_MODULE_STREAMON_ERROR2;
  }
  /* change state to stream ON */
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->is_stream_on = TRUE;
  stream_params->hw_params.diagnostic_enable =
    session_params->hw_params.diagnostic_enable;
  stream_params->hw_params.scene_mode_on =
    session_params->hw_params.scene_mode_on;
  CDBG_LOW("%s:%d] scene_mode_on:%d", __func__, __LINE__,
    stream_params->hw_params.scene_mode_on);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  CDBG_HIGH("%s:%d, identity=0x%x, stream-on done", __func__, __LINE__,
    event->identity);

  if (stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    /* TODO: Fetch AEC info by generating an event and store triggers in
       session params */
    new_event.type = MCT_EVENT_MODULE_EVENT;
    new_event.identity = streaminfo->identity;
    new_event.direction = MCT_EVENT_UPSTREAM;
    new_event.u.module_event.type = MCT_EVENT_MODULE_PPROC_GET_AEC_UPDATE;
    new_event.u.module_event.module_event_data = (void *)&stats_get;
    rc = cpp_module_send_event_upstream(module, &new_event);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      goto CPP_MODULE_STREAMON_ERROR2;
    }

    session_params->aec_trigger.gain = stats_get.aec_get.real_gain[0];
    session_params->aec_trigger.lux_idx = stats_get.aec_get.lux_idx;

    chromatix_ptr = stream_params->module_chromatix.chromatixPtr;
    if (NULL == chromatix_ptr) {
      CDBG_ERROR("%s:%d] failed chromatix_param is NULL\n", __func__, __LINE__);
      goto CPP_MODULE_STREAMON_ERROR2;
    }

    stream_params->hw_params.denoise_enable =
      session_params->hw_params.denoise_enable;
    stream_params->hw_params.sharpness_level =
      session_params->hw_params.sharpness_level;
    stream_params->hw_params.asf_mode =
      session_params->hw_params.asf_mode;
    stream_params->hw_params.aec_trigger.lux_idx =
      session_params->aec_trigger.lux_idx;
    stream_params->hw_params.aec_trigger.gain =
      session_params->aec_trigger.gain;
    CDBG_LOW("%s:%d] denoise_enable:%d", __func__, __LINE__,
      stream_params->hw_params.denoise_enable);
    CDBG_LOW("%s:%d] sharpness_level:%f", __func__, __LINE__,
      stream_params->hw_params.sharpness_level);
    CDBG_LOW("%s:%d] asf_mode:%d", __func__, __LINE__,
      stream_params->hw_params.asf_mode);

    if (stream_params->hw_params.denoise_enable == TRUE) {
       cpp_hw_params_update_wnr_params(chromatix_ptr,
         &stream_params->hw_params, &session_params->aec_trigger);
    }
    cpp_hw_params_asf_interpolate(ctrl->cpphw,&stream_params->hw_params, chromatix_ptr,
        &session_params->aec_trigger);

    /* Check for existence of linked_stream_params and apply */
    if (stream_params->linked_stream_params) {
      memcpy(stream_params->linked_stream_params->hw_params.denoise_info,
        stream_params->hw_params.denoise_info,
        sizeof(stream_params->hw_params.denoise_info));
      memcpy(&stream_params->linked_stream_params->hw_params.asf_info,
        &stream_params->hw_params.asf_info,
        sizeof(stream_params->hw_params.asf_info));
    }
  }

  rc = 0;

CPP_MODULE_STREAMON_ERROR2:
  free(hw_strm_buff_info.buffer_info);

CPP_MODULE_STREAMON_ERROR1:
  mct_list_traverse(stream_buff_info.buff_list,
    cpp_module_util_free_buffer_info, &stream_buff_info);
  mct_list_free_list(stream_buff_info.buff_list);

  return rc;
}

/* cpp_module_handle_streamoff_event:
 *
 **/
int32_t cpp_module_handle_streamoff_event(mct_module_t* module,
  mct_event_t* event)
{
  int                      rc;
  cpp_module_frame_hold_t *frame_hold = NULL;
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

  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  cpp_module_session_params_t* session_params = NULL;
  cpp_module_stream_params_t*  stream_params = NULL;
  cpp_module_stream_params_t*  linked_stream_params = NULL;
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
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
      cpp_module_send_buf_divert_event(module, frame_hold->identity,
        &frame_hold->isp_buf);
      /* Set is_frame_hold flag to FALSE */
      frame_hold->is_frame_hold = FALSE;
    }
  }

  /* Check whether DIS is enabled AND current stream off belongs to preview
     or video */
  if ((session_params->dis_enable == TRUE) &&
    ((stream_params->stream_type == CAM_STREAM_TYPE_PREVIEW) ||
    (stream_params->stream_type == CAM_STREAM_TYPE_VIDEO))) {
    /* DIS crop event is not sent for camcorder preview. It is sent only after
       camcorder recording is started. while returning from camcorder recording
       to camcorder preview session, DIS crop should not be applied for future
       preview frames. Invalidate DIS crop valid flag so that preview stream
       does not use it */
    session_params->dis_hold.is_valid = FALSE;
    /* Reset DIS crop params in stream_params and linked_stream_params */
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
    stream_params->hw_params.crop_info.is_crop.x = 0;
    stream_params->hw_params.crop_info.is_crop.y = 0;
    stream_params->hw_params.crop_info.is_crop.dx =
      stream_params->hw_params.input_info.width;
    stream_params->hw_params.crop_info.is_crop.dy =
      stream_params->hw_params.input_info.height;
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    if (linked_stream_params) {
      PTHREAD_MUTEX_LOCK(&(linked_stream_params->mutex));
      linked_stream_params->hw_params.crop_info.is_crop.x = 0;
      linked_stream_params->hw_params.crop_info.is_crop.y = 0;
      linked_stream_params->hw_params.crop_info.is_crop.dx =
        linked_stream_params->hw_params.input_info.width;
      linked_stream_params->hw_params.crop_info.is_crop.dy =
        linked_stream_params->hw_params.input_info.height;
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));

  /* send stream_off to downstream. This blocking call ensures
     downstream modules are streamed off and no acks pending from them */
  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  CDBG("%s:%d, info: downstream stream-off done.", __func__, __LINE__);

  /* invalidate any remaining entries in queue corresponding to
     this identity. This will also send/update corresponding ACKs */
  CDBG("%s:%d, info: invalidating queue.", __func__, __LINE__);
  rc = cpp_module_invalidate_queue(ctrl, identity);
    if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  cpp_module_set_clock_freq(ctrl, stream_params, 0);

  /* process hardware command for stream off, this ensures
     hardware is done with this identity */
  cpp_hardware_cmd_t cmd;
  cmd.type = CPP_HW_CMD_STREAMOFF;
  cmd.u.streamoff_data.streamoff_identity = streaminfo->identity;
  cmd.u.streamoff_data.duplicate_identity = 0;
  CDBG("%s:%d] iden:0x%x, linked_params:%p\n",
    __func__, __LINE__, streaminfo->identity, stream_params->linked_stream_params);
  if (stream_params->linked_stream_params) {
    cmd.u.streamoff_data.duplicate_identity =
      stream_params->linked_stream_params->identity;
  }
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d: hw streamoff failed\n", __func__, __LINE__);
    return -rc;
  }
  CDBG_HIGH("%s:%d, info: stream-off done for identity 0x%x",
    __func__, __LINE__, identity);
  return 0;
}
